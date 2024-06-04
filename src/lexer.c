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

//--------- ALL DEFINITIONS IN "modules.h", ERRORS in "erros.h" TOKEN definitions in "TOKEN.h"! ---------//

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////     Lexer     ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

// Functions
void LX_set_token_value_to_awaited_size(TOKEN **tokens, int **tokenLengthsArray);
void LX_resize_tokens_value(TOKEN *token, size_t oldSize);
int LX_eof_token_clearance_check(TOKEN *token, size_t lineNumber);
int LX_token_clearance_check(TOKEN *token, size_t lineNumber);
void LX_set_line_number(TOKEN *token, size_t lineNumber);
int LX_is_reference_on_pointer(TOKEN *token, char **buffer, size_t currentSymbolIndex);

int LX_skip_comment(char **input, const size_t currentIndex, size_t *lineNumber);
int LX_write_string_in_token(TOKEN *token, char **input, const size_t currentInputIndex, const char crucial_character, size_t *lineNumber, const char **fileName);
int LX_skip_whitespaces(char **input, int maxLength, size_t currentInputIndex, size_t *lineNumber);
void LX_put_type_float_in_token(TOKEN *token, const size_t symbolIndex);
void LX_write_class_accessor_or_creator_in_token(TOKEN *token, char crucialChar, size_t lineNumber);
int LX_write_pointer_in_token(TOKEN *token, size_t currentSymbolIndex, char **buffer, size_t currentBufferCharPos);
void LX_write_reference_in_token(TOKEN *token);
int LX_write_double_operator_in_token(TOKEN *token, char currentChar, char nextChar);
int LX_write_default_operator_in_token(TOKEN *token, char currentChar, size_t lineNumber);
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

size_t maxlength = 0;
size_t maxTokensLength = 0;
int tokensreserved = 0;

// Keywords [CHANGES ALSO HAVE TO BE APPLIED IN THE SYNTAX ANALYZER]
struct kwLookup {
    char kwName[12];
    TOKENTYPES kwValue;
};

struct kwLookup KeywordTable[] = {
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
    {"char", _KW_CHAR_},           {"String", _KW_STRING_},   {"short", _KW_SHORT_},
    {"long", _KW_LONG_},           {"extends", _KW_EXTENDS_}, {"constructor", _KW_CONSTRUCTOR_}
};

/*
Purpose: Tokenize the passed input
Return Type: TOKEN ** => Lexed tokens
Params: char **buffer => Input to tokenize;
        int **arrayOfIndividualTokenSizes => Length of the individual tokens;
        const size_t fileLength => input length;
        const size_t requiredTokenLength => Required tokens to tokenize the whole input;
        const char *fileName => Name of the file that gets processed

*/
TOKEN *TOKENS = NULL;

TOKEN* Tokenize(char **input, int **arrayOfIndividualTokenSizes, const size_t fileLength, const size_t requiredTokenLength, const char *fileName) {
    // TOKEN defined in modules.h
    TOKENS = (struct TOKEN*)calloc((requiredTokenLength + 2), sizeof(struct TOKEN));
    maxlength = fileLength;
    maxTokensLength = requiredTokenLength + 1;

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

    for (size_t i = 0; i < fileLength; i++) {
        // When the input character at index i is a hashtag, then skip the input till the next hashtag
        if ((*input)[i] == '/'
            && ((*input)[i + 1] == '/' || (*input)[i + 1] == '*')) {
            i += (int)LX_skip_comment(input, i, &lineNumber);
            continue;
        }

        if (storagePointer > requiredTokenLength) {
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
            i += (int)LX_write_string_in_token(&TOKENS[storagePointer], input, i, (*input)[i], &lineNumber, &fileName);
            (void)LX_set_line_number(&TOKENS[storagePointer], lineNumber);
            storagePointer++;
            storageIndex = 0;
            continue;
        }

        if (i + 1 >= fileLength) {
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

            i += (int)LX_skip_whitespaces(input, fileLength, i, &lineNumber);
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
                    int ptrRet = (int)LX_write_pointer_in_token(&TOKENS[storagePointer], storageIndex, input, i);

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
                i += (int)LX_write_double_operator_in_token(&TOKENS[storagePointer], (*input)[i], (*input)[i + 1]);   
                (void)LX_set_line_number(&TOKENS[storagePointer], lineNumber);
                TOKENS[storagePointer].tokenStart = i;
                storagePointer++;
                storageIndex = 0;
                continue;
            }
            //If non if the above is approved, the input gets processed as a 'normal' Operator
            TOKENS[storagePointer].tokenStart = i;
            storagePointer += (int)LX_write_default_operator_in_token(&TOKENS[storagePointer], (*input)[i], lineNumber);
            storageIndex = 0;
            continue;
        } else {
            if (TOKENS[storagePointer].size > storageIndex + 1) {
                // Sets the rest as IDENTIFIER. Adding the current input to the current token value
                TOKENS[storagePointer].value[storageIndex++] = (*input)[i];
                TOKENS[storagePointer].line = lineNumber;
                (void)LX_check_for_number(&TOKENS[storagePointer]);

                if (TOKENS[storagePointer].type != _FLOAT_
                    && TOKENS[storagePointer].type != _NUMBER_
                    && TOKENS[storagePointer].type != _REFERENCE_
                    && TOKENS[storagePointer].type != _POINTER_) {
                    TOKENS[storagePointer].type = _IDENTIFIER_;
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

/*
Purpose: Check if the last token is used or not
Return Type: int => 1 = true; 0 = false;
Params: TOKEN *token => Token to check its value;
        size_t *lineNumber => The line number of the token;
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

/*
Purpose: Write a possible reference from a pointer into the token
Return Type: int => > 0 = how many chars to skip; 0 = something went wrong
Params: TOKEN *token => Token to be written in;
        char **buffer => Buffer with the file content;
        size_t currentSymbolIndex => Position of the current character in the buffer
*/
int LX_is_reference_on_pointer(TOKEN *token, char **buffer, size_t currentSymbolIndex) {
    if ((*buffer)[currentSymbolIndex + 1] != '(') {
        return 0;
    }

    int symbolsToSkip = 0;

    while ((*buffer)[currentSymbolIndex + symbolsToSkip + 1] != ')'
        && (int)is_space((*buffer)[currentSymbolIndex + symbolsToSkip + 1]) == 0
        && currentSymbolIndex + symbolsToSkip + 1 < maxlength) {
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

/*
Purpose: Write the pointer operator into the token and sets the type, so the later process is easier to handle
Return Type: void
Params: TOKEN *token => Token to be set as pointer;
        size_t currentSymbolIndex => Position of the current character pointer in the value field of the token
*/
int LX_write_pointer_in_token(TOKEN *token, size_t currentSymbolIndex, char **buffer, size_t currentBufferCharPos) {
    if (token != NULL) {
        int pointers = 0;

        for (size_t i = 0; i + currentBufferCharPos < maxlength; i++) {
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

/*
Purpose: Write the reference operator into the token and sets the type, so the later process is easier to handle
Return Type: void
Params: TOKEN *token => Token to be set as reference
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

/*
Purpose: Set the line number and token number of the given token
Return Type: void
Params: TOKEN *token => Token to which the numbers are getting set;
        size_t lineNumber => Current line;
*/
void LX_set_line_number(TOKEN *token, size_t lineNumber) {
    token->line = lineNumber;
}

/*
Purpose: Allocate memory for the individual tokens by the tokenLengths to minimize memory usage
Return Type: void
Params: TOKEN **tokens => Pointer to the clear token array; int **tokenLengths => Length of the individual tokens
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

/*
Purpose: Resize the value of a token
Return Type: void
Params: TOKEN *token => Token to resize its value; size_t oldSize => current size / length of the value memeber
*/
void LX_resize_tokens_value(TOKEN *token, size_t oldSize) {
    char *newValue = (char*)realloc(token->value, sizeof(char) * (oldSize * 2));

    if (token->value == NULL) {
        (void)IO_BUFFER_RESERVATION_EXCEPTION();
    }

    token->value = newValue;
    token->size = oldSize * 2;

    // Set the new allocated memory to '0'
    if (token->value != NULL) {
        (void)memset(token->value + oldSize, 0, sizeof(char) * oldSize);
    }
}

/*
Purpose: Check if the current token is used or not
Return Type: int => 1 = true; 0 = false;
Params: TOKEN *token => Token to check its value;
        size_t *lineNumber => The line number of the token;
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

/*
Purpose: Skip the input until a second '#' appears
Return Type: int => How many characters got skipped
Params: char **input => The whole input;
        const size_t currentIndex => Position from where to start to search for another '#';
        size_t *lineNumber => The current line number of the file
*/
int LX_skip_comment(char **input, const size_t currentIndex, size_t *lineNumber) {
    char crucialChar = (*input)[currentIndex + 1];
    // The index of how much characters has to be skipped
    int jumpForward = 1;
    // Figure out where the next '#' is and stops at that or if the whole thing is out of bounds
    while ((jumpForward + currentIndex) < maxlength) {
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

/*
Purpose: Put a string into the current token
Return Type: int => How many chars to skip
Params: TOKEN *token => Current Token to write in; const char *input => Source code;
        char **input => Content of the source file;
        const size_t currentInputIndex => Index at where to start writing the string;
        const char crucial_character => Character: ' or ";
        size_t lineNumber => Current line number;
        const char **fileName => Name of the currently processed file
*/
int LX_write_string_in_token(TOKEN *token, char **input, const size_t currentInputIndex, const char crucial_character, size_t *lineNumber, const char **fileName) {
    int jumpForward = 1;

    if (input != NULL && token != NULL && token->value != NULL) {
        int currentIncremental = 1;

        // write the current character into the current token value
        // while the input is not the crucial character again the input gets set into the current token value
        while (((*input)[currentInputIndex + jumpForward] != crucial_character)
            && (currentInputIndex + jumpForward) < maxlength) {
            // If the string size is bigger than size, resize the token
            if (jumpForward + 1 >= (((token->size * currentIncremental) - 1))) {
                (void)LX_resize_tokens_value(token, ((token->size * currentIncremental) - 1));
                currentIncremental++;
            }

            if (token->size > jumpForward) {
                token->value[jumpForward] = (*input)[currentInputIndex + jumpForward];
            }

            if ((int)is_space((*input)[currentInputIndex + jumpForward]) == 2) {
                (*lineNumber)++;
            }

            jumpForward++;
        }

        if ((*input)[currentInputIndex + jumpForward] != crucial_character) {
            (void)LEXER_UNFINISHED_STRING_EXCEPTION(input, currentInputIndex, *lineNumber, fileName);
        }

        // Set the current tokentype to _STRING_ and add the quotes
        if (crucial_character == '"') {
            token->type = _STRING_;
            token->value[0] = crucial_character;

            if (token->size > jumpForward) {
                token->value[jumpForward] = crucial_character;
            } else {
                token->value[token->size - 1] = crucial_character;
            }

        // Same as the _STRING_ here, just for the _CHARACTER_ARRAY_
        } else if (crucial_character == '\'') {
            token->type = _CHARACTER_ARRAY_;
            token->value[0] = crucial_character;

            if (token->size > jumpForward) {
                token->value[jumpForward] = crucial_character;
            } else {
                token->value[token->size - 1] = crucial_character;
            }
        }

        // End the whole token with the '\0' character
        if (token->size >= jumpForward + 1) {
            token->value[jumpForward + 1] = '\0';
        } else {
            token->value[token->size - 1] = crucial_character;
        }
    }

    return jumpForward;
}

/*
Purpose: Skip all whitespaces to the next possible token
Return Type: int => Characters to skip over
Params: char **input => Input to be processed; int maxLength => Length of the buffer;
        size_t currentInputIndex => Current character position;
        size_t *lineNumber => Current line number to be increased if nesseccarry
*/
int LX_skip_whitespaces(char **input, int maxLength, size_t currentInputIndex, size_t *lineNumber) {
    int jumpForward = 0;

    while ((currentInputIndex + jumpForward) < maxLength) {
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

/*
Purpose: Put the type _FLOAT_ into the token type
Return Type: void
Params: TOKEN *token => Token to set its type to _FLOAT_; const size_t symbolIndex => current token value symbol position
*/
void LX_put_type_float_in_token(TOKEN *token, const size_t symbolIndex) {
    if (token != NULL && token->value != NULL) {
        token->type = _FLOAT_;

        if (token->size > symbolIndex) {
            token->value[symbolIndex] = '.';
        }
    }
}

/*
Purpose: Write a class accessor or class creator symbol to the current token
Return Type: void
Params: TOKEN *token => Token to which the symbol should be written to;
        char crucialChar => Character, that determines which of the accsessor or creator gets written;
        size_t lineNumber => Line number on which the token can be found
*/
void LX_write_class_accessor_or_creator_in_token(TOKEN *token, char crucialChar, size_t lineNumber) {
    if (token != NULL && token->value != NULL) {
        char src[3];
        src[0] = crucialChar;
        src[1] = '>';
        src[2] = '\0';
        
        // Check for possible buffer overflow (important when changes occour)
        if (strlen(src) <= token->size) {
            (void)strcpy(token->value, src);
        } else {
            int length = (int)strlen(src) * sizeof(char) + sizeof(char);
            token->value = (char*)realloc(token->value, length);

            if (token->value == NULL) {
                (void)IO_BUFFER_RESERVATION_EXCEPTION();
            }

            (void)strcpy(token->value, src);
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

/*
Purpose: Write the double operator into the current token
Return Type: int
Params: TOKEN *token => Current token;
        const char currentChar => Current input char;
        const char nextChar => Next char in the input
*/
int LX_write_double_operator_in_token(TOKEN *token, char currentChar, char nextChar) {
    if (token != NULL && token->value != NULL) {
        token->value[0] = currentChar;
        token->value[1] = nextChar;
        token->value[2] = '\0';

        token->type = (TOKENTYPES)LX_fill_condition_type(token->value);
    }

    return 1;
}

/*
Purpose: Write the operator into the current token
Return Type: int
Params: TOKEN *token => Current token; 
        char currentChar => Operator character;
        size_t lineNumber => Line number of the current token;
*/
int LX_write_default_operator_in_token(TOKEN *token, char currentChar, size_t lineNumber) {
    if (token != NULL && token->value != NULL) {
        token->value[0] = currentChar;
        token->value[1] = '\0';
        (void)LX_set_line_number(token, lineNumber);

        token->type = (TOKENTYPES)LX_fill_operator_type(token->value);
    }

    return 1;
}

/*
Purpose: Writes the EOF token into the current token
Return Type: void
Params: TOKEN *token => Current token which should be the EOF token
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

/*
Purpose: Set the keyword type to the current token
Return Type: void
Params: TOKEN *token => Token to set its token type
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

/*
Purpose: Check whether the input is a digit or not
Return Type: int => 1 = true; 0 = false;
Params: TOKEN *token => Token to check its value
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

/*
Purpose: Fill in the single character operators if they are an operator
Return Type: TOKENTYPES => Type of the operator (f.e. _OP_NOT_ for '!')
Params: char *value => Character array to check for
*/
TOKENTYPES LX_fill_operator_type(char *value) {
    struct symbol {
        char symbol;
        TOKENTYPES rep;
    };
    
    struct symbol lookup[] = {
        {'%', _OP_MODULU_},             {'!', _OP_NOT_},                {'(', _OP_RIGHT_BRACKET_},
        {')', _OP_LEFT_BRACKET_},       {'{', _OP_RIGHT_BRACE_},        {'}', _OP_LEFT_BRACE_},
        {'[', _OP_RIGHT_EDGE_BRACKET_}, {']', _OP_LEFT_EDGE_BRACKET_},  {'$', _OP_OVERWRITE_},
        {'.', _OP_DOT_},                {',', _OP_COMMA_},              {';', _OP_SEMICOLON_},
        {'+', _OP_PLUS_},               {'-', _OP_MINUS_},              {'/', _OP_DIVIDE_},
        {'*', _OP_MULTIPLY_},           {'=', _OP_EQUALS_},             {':', _OP_COLON_},
        {'?', _OP_QUESTION_MARK_}
    };

    for (int i = 0; i < (sizeof(lookup) / sizeof(lookup[0])); i++) {
        // If match is found check if it is a double operator
        if ((char*)strchr(value, lookup[i].symbol) != NULL) {
            if (lookup[i].rep == _OP_EQUALS_ || lookup[i].rep == _OP_NOT_
                || lookup[i].rep == _OP_PLUS_ || lookup[i].rep == _OP_MINUS_
                || lookup[i].rep == _OP_DIVIDE_ || lookup[i].rep == _OP_MULTIPLY_) {

                TOKENTYPES possibleCondition = (TOKENTYPES)LX_fill_condition_type(value);

                if (possibleCondition != _IDENTIFIER_) {
                    return possibleCondition;
                }
            }

            return lookup[i].rep;
        }
    }

    // When none of the above character could be detected, then the fill_condition_type() is called
    TOKENTYPES alternativeReturnType = LX_fill_condition_type(value);

    if (alternativeReturnType == _UNDEF_) {
        (void)LEXER_NULL_TOKEN_VALUE_EXCEPTION();
    }

    return alternativeReturnType;
}

/*
Purpose: Check if the input is a double operator and if, fill in the type of it
Return Type: TOKENTYPES => Type of the double operator (f.e. _OP_EQUALS_CONDITION_ for '==')
Params: char *value => Character array to check for double operators
*/
TOKENTYPES LX_fill_condition_type(char *value) {
    if (value == NULL) {
        return _UNDEF_;
    }
    
    struct sequence {
        char seq[3];
        TOKENTYPES rep;
    };

    struct sequence lookup[] = {
        {"-=", _OP_MINUS_EQUALS_},               {"--", _OP_SUBTRACT_ONE_},
        {"+=", _OP_PLUS_EQUALS_},                {"++", _OP_ADD_ONE_},
        {"/=", _OP_DIVIDE_EQUALS_},              {"*=", _OP_MULTIPLY_EQUALS_},
        {"!=", _OP_NOT_EQUALS_CONDITION_},       {"==", _OP_EQUALS_CONDITION_},
        {"<", _OP_SMALLER_CONDITION_},           {">", _OP_GREATER_CONDITION_},
        {">=", _OP_GREATER_OR_EQUAL_CONDITION_}, {"<=", _OP_SMALLER_OR_EQUAL_CONDITION_},
    };

    for (int i = 0; i < (sizeof(lookup) / sizeof(lookup[0])); i++) {
        // Compare the input with the lookup
        if ((char*)strstr(value, lookup[i].seq) != NULL) {
            return lookup[i].rep;
        }
    }

    return _IDENTIFIER_;
}

/*
Purpose: Free the allocated token array
Return Type: int => 1 = sucessfully freed
Params: TOKEN *tokens => Array to be freed
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

/*
Purpose: Check if the input at the position currentSymbolIndex is a double operator or not
Return Type: int => 1 = true; 0 = false;
Params: char currentChar => Current input char;
        char nextChar => Following char after currentChar
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

/*
Purpose: Prints out the values of the token array
Return Type: void
Params: TOKEN *tokens => Token array to be printed; int currentTokenIndex => size of the token array
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

/*
Purpose: Returns the keyword type based on the passed value
Return Type: TOKENTYPES => Keyword in TOKENTYPES enum
Params: char *value => Value to be scanned for keyword
*/
TOKENTYPES LX_get_keyword_type(const char *value) {
    if (value == NULL || (int)is_empty_string(value) == 1) {
        return _UNDEF_;
    }
    
    for (int i = 0; i < (sizeof(KeywordTable) / sizeof(KeywordTable[0])); i++) {
        if ((int)strcmp(value, KeywordTable[i].kwName) == 0) {
            return KeywordTable[i].kwValue;
        }
    }

    return _IDENTIFIER_;
}

/*
Purpose: Print the used CPU time
Return Type: void
Params: float cpu_time_used => Time to be printed
*/
void LX_print_cpu_time(float cpu_time_used) {
    (void)printf("\nCPU time used for LEXING: %f seconds\n", cpu_time_used);
}