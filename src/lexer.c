/////////////////////////////////////////////////////////////
///////////////////////    LICENSE    ///////////////////////
/////////////////////////////////////////////////////////////
/*
The SPACE-Language compiler compiles an input file into a runnable program.
Copyright (C) 2024  Lukas Nian En Lampl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "../headers/modules.h"
#include "../headers/errors.h"
#include "../headers/Token.h"

/** 
 * The subprogram {@code SPACE/src/lexer.c} was created
 * to provide a lexical analysis module for the SPACE language.
 * The lexer can handle around 1.3 million tokens in 0.313 seconds
 * on an intel i7 - 7700 HQ (Around 5 million characters).
 * 
 * The lexer works by checking character by character and reads an lookahead
 * character (character that follows after the current character) if necessary.
 * 
 * A token is defined as a token, when an operator is detected, a string starts,
 * a whitespace (-sequence) starts or the EOF is reached.
 * 
 * For ressource friendlyness, the sizes of the tokens are precalculated using the
 * {@code SPACE/main/input.c} module.
 * 
 * @see SPACE/main/input.c
 * 
 * @version 1.0     13.06.2024
 * @author Lukas Nian En Lampl
*/

void LX_set_token_value_to_awaited_size(TOKEN **tokens, int **tokenLengthsArray);
void LX_resize_tokens_value(TOKEN *token, size_t oldSize);
int LX_eof_token_clearance_check(TOKEN *token, size_t lineNumber);
int LX_token_clearance_check(TOKEN *token, size_t lineNumber);
void LX_set_line_number(TOKEN *token, size_t lineNumber);
int LX_is_reference_on_pointer(TOKEN *token, char **buffer, size_t currentSymbolIndex);

int LX_skip_comment(char **input, const size_t currentIndex, size_t *lineNumber);
int LX_write_string_in_token(TOKEN *token, char **input, const size_t currentInputIndex, const char crucialCharacter, size_t *lineNumber);
int LX_skip_whitespaces(char **input, size_t currentInputIndex, size_t *lineNumber);
void LX_put_type_float_in_token(TOKEN *token, const size_t symbolIndex);
void LX_write_class_accessor_or_creator_in_token(TOKEN *token, char crucialChar, size_t lineNumber);
int LX_write_pointer_in_token(TOKEN *token, char **buffer, size_t currentBufferCharPos);
void LX_write_reference_in_token(TOKEN *token);
void LX_write_double_operator_in_token(TOKEN *token, char currentChar, char nextChar);
void LX_write_default_operator_in_token(TOKEN *token, char currentChar, size_t lineNumber);
void LX_set_keyword_type_to_token(TOKEN *token);
TOKENTYPES LX_get_keyword_type(const char *value);
int LX_check_for_number(TOKEN *token);
void LX_set_EOF_token(TOKEN *token);

int LX_check_for_operator(char input);
int LX_check_for_double_operator(char currentChar, char nextChar);
void LX_print_result(TOKEN *tokens, size_t currenTokenIndex);
void LX_print_cpu_time(float cpu_time_used);

TOKENTYPES LX_fill_operator_type(char *value);
TOKENTYPES LX_fill_condition_type(char *value);

/**
 * <p>
 * A structure for a keyword lookup table.
 * </p>
 * <p>
 * The kwName holds the string, that indicates the keyword.
 * The kwValue holds the corresponding TOKENTYPES
 * </p>
 */
struct kwLookup {
	char kwName[12];
	TOKENTYPES kwValue;
};

struct symbol {
	char symbol;
	TOKENTYPES rep;
};

struct sequence {
	char seq[3];
	TOKENTYPES rep;
};

struct symbol OPERATOR_LOOKUP[] = {
	{'%', _OP_MODULU_},             {'!', _OP_NOT_},                {'(', _OP_RIGHT_BRACKET_},
	{')', _OP_LEFT_BRACKET_},       {'{', _OP_RIGHT_BRACE_},        {'}', _OP_LEFT_BRACE_},
	{'[', _OP_RIGHT_EDGE_BRACKET_}, {']', _OP_LEFT_EDGE_BRACKET_},  {'$', _OP_OVERWRITE_},
	{'.', _OP_DOT_},                {',', _OP_COMMA_},              {';', _OP_SEMICOLON_},
	{'+', _OP_PLUS_},               {'-', _OP_MINUS_},              {'/', _OP_DIVIDE_},
	{'*', _OP_MULTIPLY_},           {'=', _OP_EQUALS_},             {':', _OP_COLON_},
	{'?', _OP_QUESTION_MARK_}
};

struct sequence DOUBLE_OPERATOR_LOOKUP[] = {
	{"-=", _OP_MINUS_EQUALS_},               {"--", _OP_SUBTRACT_ONE_},
	{"+=", _OP_PLUS_EQUALS_},                {"++", _OP_ADD_ONE_},
	{"/=", _OP_DIVIDE_EQUALS_},              {"*=", _OP_MULTIPLY_EQUALS_},
	{"!=", _OP_NOT_EQUALS_CONDITION_},       {"==", _OP_EQUALS_CONDITION_},
	{"<", _OP_SMALLER_CONDITION_},           {">", _OP_GREATER_CONDITION_},
	{">=", _OP_GREATER_OR_EQUAL_CONDITION_}, {"<=", _OP_SMALLER_OR_EQUAL_CONDITION_},
};

struct kwLookup KEYWORD_LOOKUP[] = {
	{"while", _KW_WHILE_},         {"if", _KW_IF_},           {"function", _KW_FUNCTION_},
	{"var", _KW_VAR_},             {"break", _KW_BREAK_},     {"return", _KW_RETURN_},
	{"do", _KW_DO_},               {"class", _KW_CLASS_},     {"with", _KW_WITH_},
	{"new", _KW_NEW_},             {"true", _KW_TRUE_},       {"false", _KW_FALSE_},
	{"null", _KW_NULL_},           {"enum", _KW_ENUM_},       {"check", _KW_CHECK_},
	{"is", _KW_IS_},               {"try", _KW_TRY_},         {"catch", _KW_CATCH_},
	{"continue", _KW_CONTINUE_},   {"const", _KW_CONST_},     {"include", _KW_INCLUDE_},
	{"and", _KW_AND_},             {"or", _KW_OR_},           {"global", _KW_GLOBAL_},
	{"secure", _KW_SECURE_},       {"private", _KW_PRIVATE_}, {"export", _KW_EXPORT_},
	{"for", _KW_FOR_},             {"this", _KW_THIS_},       {"else", _KW_ELSE_},
	{"int", _KW_INT_},             {"double", _KW_DOUBLE_},   {"float", _KW_FLOAT_},
	{"char", _KW_CHAR_},           {"extends", _KW_EXTENDS_}, {"short", _KW_SHORT_},
	{"long", _KW_LONG_},           {"void", _KW_VOID_},       {"constructor", _KW_CONSTRUCTOR_}
};

/**
 * <p>
 * This variable is invoked on the Tokenize() function and is set
 * to the buffer length.
 * </p>
 * 
 * <p><strong>Usage:</strong>
 * This is used to provide a global boundary checking
 * variable and thus preventing out of bounds accesses in parts
 * that access the buffer directly.
 * </p>
 */
extern size_t BUFFER_LENGTH;

/**
 * <p>
 * This holds the maximum length of the precalculated tokens.
 * </p>
 * 
 * <p><strong>Usage:</strong>
 * Provides a variable, that can be used globally for ensuring
 * that an access into the TOKEN array is not out of bounds.
 * </p>
 */
extern size_t TOKEN_LENGTH;
size_t maxTokensLength = 0;

/**
 * <p>
 * A flag whether the token array is already / was already reserved
 * or not. (Important for freeing memory afterwards)
 * </p>
 */
int tokensreserved = 0;

/**
 * <p>
 * The future token array.
 * </p>
 */
TOKEN *TOKENS = NULL;
extern char **BUFFER;
extern char *FILE_NAME;

/**
 * <p>
 * The function start the lexing process by first initializing the
 * tokens array and finally lexing the input.
 * </p>
 * <p>
 * A token is a own token, if an operator start, a string starts or
 * a whitespace was found. The lexer only uses 1 character at a time,
 * but for identifiying double operators like '++' or '+=' etc. another
 * character is loaded.
 * </p>
 * @returns The final token array with all tokens
 * 
 * @param **arrayOfIndividualTokenSizes     Sizes of the indiviual tokens
 */
TOKEN* Tokenize(int **arrayOfIndividualTokenSizes) {
	// TOKEN defined in modules.h
	TOKENS = (struct TOKEN*)calloc((TOKEN_LENGTH + 2), sizeof(struct TOKEN));
	maxTokensLength = TOKEN_LENGTH + 1;
	char **input = BUFFER;

	// When the TOKEN array couldn't be allocated, then throw an IO_BUFFER_RESERVATION_EXCEPTION (errors.h)
	if (TOKENS == NULL) {
		(void)IO_BUFFER_RESERVATION_EXCEPTION();
	}

	(void)LX_set_token_value_to_awaited_size(&TOKENS, arrayOfIndividualTokenSizes);
	tokensreserved = 1;
	
	// Set a pointer on the token array to free it, when the program crashes or ends
	(void)_init_error_token_cache_(&TOKENS);
	// Set StoragePointer and Index to 0 for new counting
	size_t storageIndex = 0;
	size_t storagePointer = 0;

	// CLOCK FOR DEBUG PURPOSES ONLY!!
	clock_t start, end;

	if (LEXER_DISPLAY_USED_TIME == 1) {
		start = (clock_t)clock();
	}

	size_t lineNumber = 0;

	for (size_t i = 0; i < BUFFER_LENGTH; i++) {
		// When the input character at index i is a hashtag, then skip the input till the next hashtag
		if ((*input)[i] == '/'
			&& ((*input)[i + 1] == '/' || (*input)[i + 1] == '*')) {
			i += (int)LX_skip_comment(input, i, &lineNumber);
			continue;
		}

		if (storagePointer > maxTokensLength) {
			(void)LEXER_NULL_TOKEN_EXCEPTION();
		}

		// Check if the Size is bigger than the expected, if true, then increase the size of the token value
		if (storageIndex > TOKENS[storagePointer].size) {
			(void)LX_set_keyword_type_to_token(&TOKENS[storagePointer]);
			(void)LX_resize_tokens_value(&TOKENS[storagePointer], TOKENS[storagePointer].size);
		} else if (storageIndex == 0) {
			TOKENS[storagePointer].tokenStart = i;
		}

		// Checks if input is a whitespace (if isspace() returns a non-zero number the integer is set to 1 else to 0)
		int isWhiteSpace = (int)is_space((*input)[i]);
		int isOperator = isWhiteSpace != 1 ? (int)check_for_operator((*input)[i]) : 0; //Checks if input at i is an operator from above
		// Check if the input character at index i is the beginning of an string or character array
		if ((*input)[i] == '"' || (*input)[i] == '\'') {
			storagePointer += (int)LX_token_clearance_check(&TOKENS[storagePointer], lineNumber);
			i += (int)LX_write_string_in_token(&TOKENS[storagePointer], input, i, (*input)[i], &lineNumber);
			(void)LX_set_line_number(&TOKENS[storagePointer], lineNumber);
			storagePointer++;
			storageIndex = 0;
			continue;
		}

		if (i + 1 >= BUFFER_LENGTH) {
			(void)LX_set_keyword_type_to_token(&TOKENS[storagePointer]);
			(void)LX_set_line_number(&TOKENS[storagePointer], lineNumber);
		}

		// If the input character at index i is a whitespace, then filter the whitespace character
		if (isWhiteSpace > 0) {
			(void)LX_set_keyword_type_to_token(&TOKENS[storagePointer]);

			// If the current token is already filled or not, if then add "\0" to close the string  
			if ((int)LX_token_clearance_check(&TOKENS[storagePointer], lineNumber)) { 
				if (TOKENS[storagePointer].size > storageIndex) {
					TOKENS[storagePointer].value[storageIndex] = '\0';
				} else {
					TOKENS[storagePointer].value[storageIndex - 1] = '\0';
				}

				storagePointer++;
			}

			i += (int)LX_skip_whitespaces(input, i, &lineNumber);
			storageIndex = 0;
			continue;

		// Execute if input at i is an operator
		} else if (isOperator) {
			// Check if the TOKEN could be a FLOAT or not
			if ((*input)[i] == '.' 
				&& ((int)is_digit((*input)[i - 1])
				&& (int)is_digit((*input)[i + 1]))) {
				(void)LX_put_type_float_in_token(&TOKENS[storagePointer], storageIndex);
				storageIndex++;
				continue;
			} else if ((*input)[i] == '*') {
				if ((int)is_space((*input)[i + 1]) == 0
					&& (int)is_digit((*input)[i + 1]) == 0) {
					int ptrRet = (int)LX_write_pointer_in_token(&TOKENS[storagePointer], input, i);

					if (ptrRet > 0) {
						i += ptrRet - 1;
						storageIndex = ptrRet - 1;
						(void)LX_set_line_number(&TOKENS[storagePointer], lineNumber);
						storageIndex++;
					}

					continue;
				}
			} else if ((*input)[i] == '-' && (int)is_digit((*input)[i + 1]) == 1) {
				storagePointer += (int)LX_token_clearance_check(&TOKENS[storagePointer], lineNumber);
				storageIndex = 0;
				TOKENS[storagePointer].value[storageIndex++] = (*input)[i];
				TOKENS[storagePointer].type = _NUMBER_;
				continue;
			}

			// Check if the current token is used or not, and if it increases storagePointer by 1
			(void)LX_set_keyword_type_to_token(&TOKENS[storagePointer]);
			storagePointer += (int)LX_token_clearance_check(&TOKENS[storagePointer], lineNumber); 
			// Check whether the input could be an ELEMENT ACCESSOR or not
			if (((*input)[i] == '-' || (*input)[i] == '=') && (*input)[i + 1] == '>') {
				(void)LX_write_class_accessor_or_creator_in_token(&TOKENS[storagePointer], (*input)[i], lineNumber);
				TOKENS[storagePointer].tokenStart = i;
				storagePointer++;
				storageIndex = 0;
				i++;
				continue;
			} else if ((*input)[i] == '&') {
				if ((*input)[i + 1] == '(' && (*input)[i + 2] == '*') {
					i += (int)LX_is_reference_on_pointer(&TOKENS[storagePointer], input, i);
					(void)LX_set_line_number(&TOKENS[storagePointer], lineNumber);
					TOKENS[storagePointer].tokenStart = i;
					storageIndex = 0;
					storagePointer++;
					continue;
				} else {
					(void)LX_write_reference_in_token(&TOKENS[storagePointer]);
					(void)LX_set_line_number(&TOKENS[storagePointer], lineNumber);
					TOKENS[storagePointer].tokenStart = i;
					storageIndex++;
					continue;
				}

			// Figure out whether the input is a double operator like "++" or "--" or not
			} else if ((int)LX_check_for_double_operator((*input)[i], (*input)[i + 1])) {
				TOKENS[storagePointer].tokenStart = i;
				(void)LX_write_double_operator_in_token(&TOKENS[storagePointer], (*input)[i], (*input)[i + 1]);   
				(void)LX_set_line_number(&TOKENS[storagePointer], lineNumber);
				TOKENS[storagePointer].tokenStart = ++i;
				storagePointer++;
				storageIndex = 0;
				continue;
			}
			//If non if the above is approved, the input gets processed as a 'normal' Operator
			TOKENS[storagePointer].tokenStart = i;
			(void)LX_write_default_operator_in_token(&TOKENS[storagePointer], (*input)[i], lineNumber);
			storagePointer++;
			storageIndex = 0;
			continue;
		} else {
			TOKEN *token = &TOKENS[storagePointer];

			if (token->size > storageIndex + 1) {
				// Sets the rest as IDENTIFIER. Adding the current input to the current token value
				token->value[storageIndex++] = (*input)[i];
				token->line = lineNumber;
				(void)LX_check_for_number(token);

				if (token->type != _FLOAT_
					&& token->type != _NUMBER_
					&& token->type != _REFERENCE_
					&& token->type != _POINTER_) {
					token->type = _IDENTIFIER_;
				}
			}
		}
	}
	
	/////////////////////////
	///     EOF TOKEN     ///
	/////////////////////////
	storagePointer += (int)LX_eof_token_clearance_check(&(TOKENS[storagePointer]), lineNumber);
	(void)LX_set_EOF_token(&TOKENS[storagePointer]);
	maxTokensLength = storagePointer > maxTokensLength ? storagePointer : maxTokensLength;
	storagePointer--;

	// END CLOCK AND PRINT RESULT
	if (LEXER_DISPLAY_USED_TIME == 1) {
		end = (clock_t)clock();
	}

	if (LEXER_DEBUG_MODE == 1) {
		(void)LX_print_result(TOKENS, storagePointer);
	}

	if (LEXER_DISPLAY_USED_TIME == 1) {
		(void)printf("Finished with %li tokens and %li lines in total.\n", storagePointer + 1, lineNumber + 1);
		(void)LX_print_cpu_time(((double) (end - start)) / CLOCKS_PER_SEC);
	}

	return TOKENS;
}

/**
 * <p>
 * Checks whether the provided token (last token) is used or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li> 1 - Token is indeed not used
 * <li> 0 - Token is already used
 * </ul>
 * 
 * @param *token        Token to check
 * @param lineNumber    Variable that holds the current line number
 */
int LX_eof_token_clearance_check(TOKEN *token, size_t lineNumber) {
	if (token != NULL && token->value != NULL) {
		if (token->type > _LII_ || token->size == 0 || token->type == __EOF__) {
			return 0;
		} else {
			if (token->value[0] != 0) {
				return 1;
			}
		}
	}

	return 0;
}

/**
 * <p>
 * Writes a possible reference from a pointer into the provided
 * token. (e.g. `&(*ptr)`)
 * </p>
 * 
 * @returns Characters to skip
 * 
 * @param *token                Token to write into
 * @param **buffer              Input (source file)
 * @param currentSymbolIndex    Current index at the buffer
 */
int LX_is_reference_on_pointer(TOKEN *token, char **buffer, size_t currentSymbolIndex) {
	if ((*buffer)[currentSymbolIndex + 1] != '(') {
		return 0;
	}

	int symbolsToSkip = 0;

	while ((*buffer)[currentSymbolIndex + symbolsToSkip + 1] != ')'
		&& (int)is_space((*buffer)[currentSymbolIndex + symbolsToSkip + 1]) == 0
		&& currentSymbolIndex + symbolsToSkip + 1 < BUFFER_LENGTH) {
		if (token->size > symbolsToSkip + 2) {
			token->value[symbolsToSkip + 2] = (*buffer)[currentSymbolIndex + symbolsToSkip + 2];
		}

		symbolsToSkip++;
	}

	if ((*buffer)[currentSymbolIndex + symbolsToSkip + 1] != ')') {
		token->value = "";
		return 0;
	}

	if (token->size >= symbolsToSkip + 2) {
		token->value[0] = '&';
		token->value[1] = '(';
		token->value[symbolsToSkip + 1] = ')';
		token->value[symbolsToSkip + 2] = '\0';
		token->type = _REFERENCE_ON_POINTER_;
	}

	return symbolsToSkip + 1;
}

/**
 * <p>
 * Writes a pointer into the provided token.
 * </p>
 * 
 * @param *token                Token to write into
 * @param currentBufferCharPos  Index of the current buffer symbol
 * @param **buffer              Source code buffer
 */
int LX_write_pointer_in_token(TOKEN *token, char **buffer, size_t currentBufferCharPos) {
	if (token != NULL) {
		int pointers = 0;

		for (size_t i = 0; i + currentBufferCharPos < BUFFER_LENGTH; i++) {
			if ((*buffer)[currentBufferCharPos + i] == '*') {
				pointers++;
				continue;
			}

			if ((int)is_space((*buffer)[currentBufferCharPos + i]) == 1
				|| (int)is_digit((*buffer)[currentBufferCharPos + i]) == 1) {
				LEXER_UNFINISHED_POINTER_EXCEPTION();
			} else if ((int)check_for_operator((*buffer)[currentBufferCharPos + i]) == 1) {
				return 0;
			} else {
				break;
			}
		}

		while (token->size < pointers + 1) {
			(void)LX_resize_tokens_value(token, token->size);
		}

		for (int i = 0; i < pointers; i++) {
			token->value[i] = '*';
		}

		token->value[pointers + 1] = '\0';
		token->type = _POINTER_;
		return pointers;
	}

	return 0;
}

/**
 * <p>
 * Writes a reference operator into the provided token.
 * </p>
 * 
 * @param *token    Token to write into
 */
void LX_write_reference_in_token(TOKEN *token) {
	if (token != NULL) {
		token->type = _REFERENCE_;

		if (token->size > 1) {
			token->value[0] = '&';
			token->value[1] = '\0';
		}
	}
}

/**
 * <p>
 * Sets the current line number to the provided token.
 * </p>
 * 
 * @param *token        Token to set the line to
 * @param lineNumber    Line number to set
 */
void LX_set_line_number(TOKEN *token, size_t lineNumber) {
	token->line = lineNumber;
}

/**
 * <p>
 * Allocates the memory to the predicted size from the
 * input module.
 * </p>
 * 
 * <p>
 * Calloc is used, in order to have 0 values only.
 * </p>
 * 
 * @param **tokens              Pointer to the token array to calloc
 * @param **tokenLengthsArray   Sizes of the individual tokens
 */
void LX_set_token_value_to_awaited_size(TOKEN **tokens, int **tokenLengthsArray) {
	if (*tokens != NULL && tokenLengthsArray != NULL) {
		for (int i = 0; i < maxTokensLength; i++) {
			// Calloc as much space as predicted; Tokens at i has the value length of tokenLengths at i
			(*tokens)[i].value = (char*)calloc((*tokenLengthsArray)[i], sizeof(char));
			(*tokens)[i].size = (*tokenLengthsArray)[i];

			// If the allocation of the memory should fail an error gets called
			if ((*tokens)[i].value == NULL) {
				(void)IO_BUFFER_RESERVATION_EXCEPTION();
			}
		}
	}
}

/**
 * <p>
 * Resizes a tokens value to the 125% of its old size.
 * </p>
 * 
 * <p>
 * The occurance of wrong determined token sizes is rare, but
 * for the case it increments by 25%, in order to provide more
 * space if needed without reserving each by each. This
 * results in better performance but in less ressource friendlyness.
 * </p>
 * 
 * @param *token    Token to resize
 * @param oldSize   Size of the token before the resize
 */
void LX_resize_tokens_value(TOKEN *token, size_t oldSize) {
	int newSize = (int)(oldSize * 1.25);
	char *newValue = (char*)realloc(token->value, sizeof(char) * newSize);

	if (token->value == NULL) {
		(void)IO_BUFFER_RESERVATION_EXCEPTION();
	}

	token->value = newValue;
	token->size = newSize;

	// Set the new allocated memory to '0'
	if (token->value != NULL) {
		(void)memset(token->value + oldSize, 0, sizeof(char) * (oldSize * 0.25));
	}
}

/**
 * <p>
 * Checks whether the provided token is used or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li> 1 - Token is indeed not used
 * <li> 0 - Token is already used
 * </ul>
 * 
 * @param *token        Token to check
 * @param lineNumber    Variable that holds the current line number
 */
int LX_token_clearance_check(TOKEN *token, size_t lineNumber) {
	if (token != NULL && token->value != NULL) {
		if (token->value[0] != 0) {
			token->line = lineNumber;
			return 1;
		}
	}

	return 0;
}

/**
 * <p>
 * Skips a comment.
 * </p>
 * 
 * <p>
 * Based on the "crucial" part the skip is defined. By crucial a double slash
 * or a slash-star is meant. For the first option the comment
 * goes till the next line. For the slash-start the skip happens, until the counterpart
 * was found.
 * </p>
 * 
 * @returns How many characters can be skipped
 * 
 * @param **input       Pointer to the input
 * @param currentIndex  Index of the current char in the buffer
 * @param *lineNumber   Pointer to the lineNumber
 */
int LX_skip_comment(char **input, const size_t currentIndex, size_t *lineNumber) {
	char crucialChar = (*input)[currentIndex + 1];
	// The index of how much characters has to be skipped
	int jumpForward = 1;
	// Figure out where the next '#' is and stops at that or if the whole thing is out of bounds
	while ((jumpForward + currentIndex) < BUFFER_LENGTH) {
		//Check for '\n'
		if ((int)is_space((*input)[currentIndex + jumpForward]) == 2) {
			(*lineNumber)++;
		}

		if (crucialChar == '*'
			&& (*input)[currentIndex + jumpForward] == '*'
			&& (*input)[currentIndex + jumpForward + 1] == '/') {
			jumpForward++;
			break;
		} else if (crucialChar == '/'
			&& (*input)[currentIndex + jumpForward] == '\n') {
			break;
		}

		jumpForward++;
	}

	return jumpForward;
}

/**
 * <p>
 * Put a string into a provided token.
 * </p>
 * 
 * @returns How many chars can be skipped
 * 
 * @param *token                Token to write into
 * @param **input               Input (Source code)
 * @param currentInputIndex     Index of the input character
 * @param crucialCharacter      Type of the string ("'" or '"')
 * @param *lineNumber           Current line number
 */
int LX_write_string_in_token(TOKEN *token, char **input, const size_t currentInputIndex, const char crucialCharacter, size_t *lineNumber) {
	int jumpForward = 1;

	if (input != NULL && token != NULL && token->value != NULL) {
		int currentIncremental = 1;
		int currentTokenSize = (((token->size * currentIncremental) - 1));

		// write the current character into the current token value
		// while the input is not the crucial character again the input gets set into the current token value
		while (((*input)[currentInputIndex + jumpForward] != crucialCharacter)
			&& (currentInputIndex + jumpForward) < BUFFER_LENGTH) {
			// If the string size is bigger than size, resize the token
			if (jumpForward + 1 >= currentTokenSize) {
				(void)LX_resize_tokens_value(token, currentTokenSize);
				currentIncremental++;
				currentTokenSize = (((token->size * currentIncremental) - 1));
			}

			if (token->size > jumpForward) {
				token->value[jumpForward] = (*input)[currentInputIndex + jumpForward];
			}

			if ((int)is_space((*input)[currentInputIndex + jumpForward]) == 2) {
				(*lineNumber)++;
			}

			jumpForward++;
		}

		if ((*input)[currentInputIndex + jumpForward] != crucialCharacter) {
			(void)LEXER_UNFINISHED_STRING_EXCEPTION(input, currentInputIndex, *lineNumber);
		}

		if (crucialCharacter == '"') {
			token->type = _STRING_;
		} else if (crucialCharacter == '\'') {
			token->type = _CHARACTER_ARRAY_;
		}

		token->value[0] = crucialCharacter;

		if (token->size > jumpForward) {
			token->value[jumpForward] = crucialCharacter;
		} else {
			token->value[token->size - 1] = crucialCharacter;
		}

		// End the whole token with the '\0' character
		if (token->size >= jumpForward + 1) {
			token->value[jumpForward + 1] = '\0';
		} else {
			token->value[token->size - 1] = crucialCharacter;
		}
	}

	return jumpForward;
}

/**
 * <p>
 * Skips all whitespace characters to the next possible token.
 * </p>
 * 
 * @returns Characters that can be skipped
 * 
 * @param **input               Source code with the whitespaces
 * @param currentInputIndex     Index of the current character in the buffer
 * @param *lineNumber           Current line number
 */
int LX_skip_whitespaces(char **input, size_t currentInputIndex, size_t *lineNumber) {
	int jumpForward = 0;

	while ((currentInputIndex + jumpForward) < BUFFER_LENGTH) {
		int whitespaceChar = (int)is_space((*input)[(currentInputIndex + jumpForward)]);
		
		if (whitespaceChar == 0) {
			break;
		} else if (whitespaceChar == 2) {
			(*lineNumber)++;
		}

		jumpForward++;
		continue;
	}

	// return the value of how much the input index has to skip until there's another non whitespace character
	return jumpForward - 1;
}

/**
 * <p>
 * Puts the _FLOAT_ type into the token and adds the '.'.
 * </p>
 * 
 * @param *token        Token to add the type and dot to
 * @param symbolIndex   Position of the dot in the buffer
 */
void LX_put_type_float_in_token(TOKEN *token, const size_t symbolIndex) {
	if (token != NULL && token->value != NULL) {
		token->type = _FLOAT_;

		if (token->size > symbolIndex) {
			token->value[symbolIndex] = '.';
		}
	}
}

/**
 * <p>
 * Writes a class accessor or class creator operator into the provided token.
 * </p>
 * 
 * <p>
 * If the size of the token should be to small, the token is resized.
 * </p>
 * 
 * @param *token        Token to write the operation into
 * @param cruacialChar  Character that determines the operator type
 * @param lineNumber    Current line number
 */
void LX_write_class_accessor_or_creator_in_token(TOKEN *token, char crucialChar, size_t lineNumber) {
	if (token != NULL && token->value != NULL) {
		char src[3];
		src[0] = crucialChar;
		src[1] = '>';
		src[2] = '\0';
		
		// Check for possible buffer overflow (important when changes occour)
		if (strlen(src) <= token->size) {
			(void)strncpy(token->value, src, token->size);
		} else {
			int length = (int)strlen(src) * sizeof(char) + sizeof(char);
			token->value = (char*)realloc(token->value, length);

			if (token->value == NULL) {
				(void)IO_BUFFER_RESERVATION_EXCEPTION();
			}

			(void)strncpy(token->value, src, token->size);
			token->size = length;
		}
		
		token->line = lineNumber;

		switch (crucialChar) {
		case '-':
			token->type = _OP_CLASS_ACCESSOR_;
			break;
		case '=':
			token->type = _OP_CLASS_CREATOR_;
			break;
		}
	}
}

/**
 * <p>
 * Writes a double operator into the provided token.
 * </p>
 * 
 * <p>
 * As a double operator any operation that contains two
 * normal operators is count. (e.g. "+=", "--", "/=", ...)
 * </p>
 * 
 * @param *token        Token to write into
 * @param currentChar   First char of the double operator
 * @param nextChar      Second char of the double operator
 */
void LX_write_double_operator_in_token(TOKEN *token, char currentChar, char nextChar) {
	if (token != NULL && token->value != NULL) {
		token->value[0] = currentChar;
		token->value[1] = nextChar;
		token->value[2] = '\0';

		token->type = (TOKENTYPES)LX_fill_condition_type(token->value);
	}
}

/**
 * <p>
 * This function writes a single operator into the provided token.
 * </p>
 * 
 * <p>
 * A normal operator is defined as the standard operators that are
 * not followed by another operator. (e.g. "+", "/", "*", ...)
 * </p>
 * 
 * @param *token        Token to write into
 * @param currentChar   Character / operator to write
 * @param lineNumber    Line of the token
 */
void LX_write_default_operator_in_token(TOKEN *token, char currentChar, size_t lineNumber) {
	if (token != NULL && token->value != NULL) {
		token->value[0] = currentChar;
		token->value[1] = '\0';
		(void)LX_set_line_number(token, lineNumber);

		token->type = (TOKENTYPES)LX_fill_operator_type(token->value);
	}
}

/**
 * <p>
 * Writes the SPACE EOF token into the provided token.
 * </p>
 * 
 * <p>
 * The EOF token indicates the end of the source file.
 * </p>
 * 
 * @param *token    Token to write the EOF indicator into
 */
void LX_set_EOF_token(TOKEN *token) {
	if (token != NULL) {
		char *src = "$EOF$\0";
		token->value = (char*)calloc(sizeof(char), 7);

		if (token->value == NULL) {
			printf("ERROR ON ALLOCATIONG MEMORY FOR EOF TOKEN!\n");
		}

		(void)strncpy(token->value, src, 6 * sizeof(char));
		
		token->value[6] = '\0';
		token->type = __EOF__;
		token->size = 6;
		token->line = -1;
		token->tokenStart = -1;
	}
}

/**
 * <p>
 * Sets the according keyword to the token.
 * </p>
 * 
 * @param *token    Token to set the keyword in
 */
void LX_set_keyword_type_to_token(TOKEN *token) {
	if (token == NULL) {
		(void)LEXER_TOKEN_ERROR_EXCEPTION();
	}

	if (token != NULL && token->value != NULL) {
		if (token->type != _IDENTIFIER_) {
			return;
		}
		
		TOKENTYPES type = (TOKENTYPES)LX_get_keyword_type(token->value);
		token->type = type;
	}
}

/**
 * <p>
 * Checks if a token is a number or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Token is a number
 * <li>false - Token in not a number
 * </ul>
 * 
 * @param *token    Token to check
 */
int LX_check_for_number(TOKEN *token) {
	if (token == NULL) {
		(void)LEXER_TOKEN_ERROR_EXCEPTION();
	}

	if ((int)is_digit(token->value[0]) && token->type != _FLOAT_) {
		token->type = _NUMBER_;
		return 1;
	}

	return 0;
}

/**
 * <p>
 * Returns the according operator type, that matches the input.
 * </p>
 * 
 * <p>
 * If the input would be "*" for instance, the returned result
 * would look like this: _OP_MULTIPLY_.
 * </p>
 * 
 * @returns The converted TOKENTYPES
 * 
 * @param *value    Value to convert
 */
TOKENTYPES LX_fill_operator_type(char *value) {
	int length = (sizeof(OPERATOR_LOOKUP) / sizeof(OPERATOR_LOOKUP[0]));

	for (int i = 0; i < length; i++) {
		// If match is found check if it is a double operator
		if ((char*)strchr(value, OPERATOR_LOOKUP[i].symbol) != NULL) {
			switch (OPERATOR_LOOKUP[i].rep) {
			case _OP_EQUALS_:
			case _OP_PLUS_:
			case _OP_NOT_:
			case _OP_MINUS_:
			case _OP_DIVIDE_:
			case _OP_MULTIPLY_: {
				TOKENTYPES possibleCondition = (TOKENTYPES)LX_fill_condition_type(value);

				if (possibleCondition != _IDENTIFIER_) {
					return possibleCondition;
				}
			}
			default:
				return OPERATOR_LOOKUP[i].rep;
			}
		}
	}

	// When none of the above character could be detected, then the fill_condition_type() is called
	TOKENTYPES alternativeReturnType = LX_fill_condition_type(value);

	if (alternativeReturnType == _UNDEF_) {
		(void)LEXER_NULL_TOKEN_VALUE_EXCEPTION();
	}

	return alternativeReturnType;
}

/**
 * <p>
 * Returns the according double operator type, that matches the input.
 * </p>
 * 
 * <p>
 * If the input would be "*=" for instance, the returned result
 * would look like this: _OP_MULTIPLY_EQUALS_.
 * </p>
 * 
 * @returns The converted TOKENTYPES
 * 
 * @param *value    Value to convert
 */
TOKENTYPES LX_fill_condition_type(char *value) {
	if (value == NULL) {
		return _UNDEF_;
	}

	int length = (sizeof(DOUBLE_OPERATOR_LOOKUP) / sizeof(DOUBLE_OPERATOR_LOOKUP[0]));

	for (int i = 0; i < length; i++) {
		// Compare the input with the lookup
		if ((char*)strstr(value, DOUBLE_OPERATOR_LOOKUP[i].seq) != NULL) {
			return DOUBLE_OPERATOR_LOOKUP[i].rep;
		}
	}

	return _IDENTIFIER_;
}

/**
 * <p>
 * Frees the allocated memory.
 * </p>
 * 
 * <p>
 * If the tokens are not initialized before the function
 * just returns.
 * </p>
 * 
 * @returns 1, when the function has run
 * 
 * @param *tokens   Token to free
 */
int FREE_TOKENS(TOKEN *tokens) {
	if (tokensreserved == 1 && tokens != NULL) {
		for (int i = 0; i < maxTokensLength; i++) {
			if (tokens[i].value != NULL) {
				(void)free(tokens[i].value);
				tokens[i].value = NULL;
			}
		}

		(void)free(tokens);
		tokens = NULL;
		tokensreserved = 0;
	}

	return 1;
}

/**
 * <p>
 * Checks if the input at the currentSymbolIndex and
 * currentSymbolIndex + 1 is a double operator or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - If the input is a double operator
 * <li>false - If the input is not a double operator
 * </ul>
 * 
 * @param currentChar   First char of the possible double operator
 * @param nextChar      Second char of the possible double operator
 */
int LX_check_for_double_operator(char currentChar, char nextChar) {
	if ((currentChar == '+' && nextChar == '+')
		|| (currentChar == '-' && nextChar == '-')) {
		return 1;
	} else if ((currentChar == '-' || currentChar == '+'
		|| currentChar == '*' || currentChar == '/')
		&& nextChar == '=') {
		return 1;
	} else if ((currentChar = '<' || currentChar == '>')
		&& nextChar == '=') {
		return 1;
	} else if (currentChar == '=' && nextChar == '=') {
		return 1;
	} else {
		return 0;
	}
}

/**
 * <p><strong>DEBUG ONLY!</strong>
 * This function prints the details of the lexer for the DEBUG_MODE = 1.
 * </p>
 * 
 * @param *tokens               Tokens to print
 * @param currentTokenIndex     Size of the token array
 */
void LX_print_result(TOKEN *tokens, size_t currenTokenIndex) {
	if (tokens != NULL) {
		(void)printf("\n>>>>>>>>>>>>>>>>>>>>    LEXER    <<<<<<<<<<<<<<<<<<<<\n\n");

		for (size_t i = 0; i < currenTokenIndex + 2; i++) {
			if (tokens[i].value == NULL) {
				printf("Token: (NULL)\n");
				continue;
			}

			(void)printf("Token: %3lu | Type: %-2d | Size: %3li | Line: %3li | Start of TOKEN: %3li -> Token: %s\n", i, (int)tokens[i].type, tokens[i].size, tokens[i].line, tokens[i].tokenStart, tokens[i].value);
		}

		(void)printf("\n>>>>>    Buffer successfully lexed    <<<<<\n");
	}
}

/**
 * <p>
 * Returns the according keyword type, that matches the input.
 * </p>
 * 
 * <p>
 * If the input would be "var" for instance, the returned result
 * would look like this: _KW_VAR_.
 * </p>
 * 
 * @returns The converted TOKENTYPES
 * 
 * @param *value    Value to convert
 */
TOKENTYPES LX_get_keyword_type(const char *value) {
	if (value == NULL || (int)is_empty_string(value) == 1) {
		return _UNDEF_;
	}

	int length = (sizeof(KEYWORD_LOOKUP) / sizeof(KEYWORD_LOOKUP[0]));
	
	for (int i = 0; i < length; i++) {
		if ((int)strcmp(value, KEYWORD_LOOKUP[i].kwName) == 0) {
			return KEYWORD_LOOKUP[i].kwValue;
		}
	}

	return _IDENTIFIER_;
}

/**
 * <p><strong>DEBUG ONLY!</strong>
 * Prints out the totaly used time in seconds.
 * </p>
 * 
 * @param cpu_time_used     CPU time that was used to run the lexer module
 */
void LX_print_cpu_time(float cpu_time_used) {
	(void)printf("\nCPU time used for LEXING: %f seconds\n", cpu_time_used);
}