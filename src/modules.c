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
#include <stdlib.h>
#include <string.h>
#include "../headers/modules.h"

/*
Purpose: Check if a character is a space character
Return Type: int => 1 = is whitespace char; 0 = is not a whitespace char
Params: const char character => Character to be checked
*/
int is_space(const char character) {
	switch (character) {
	case '\n':
		return 2;
	case ' ':
	case '\r':
	case '\v':
	case '\t':
	case '\f':
		return 1;
	default:
		return 0;
	}
}

/*
Purpose: Checks if a string is empty or not
Return Type: int => 1 = is empty; 0 = not empty
Params: const char *string => String to be checked
*/
int is_empty_string(const char* string) {
	if (string == NULL) {
		return 1;
	} else if ((int)strlen(string) < 1) {
		return 1;
	} else if (string[0] == '\0') {
		return 1;
	}

	return 0;
}

/*
Purpose: Check if a character is a digit
Return Type: int => 1 = is digit; 0 = not a digit
Params: char character => Character to be checked
*/
int is_digit(char character) {
	if (character >= '0' && character <= '9') {
		return 1;
	}

	return 0;
}

/*
Purpose: Figure out if the input is an operator or not
Return Type: int => 1 = true; 0 = false;
Params: char input => Compare the current character to the operators in the OPERATORS array
*/
int check_for_operator(char input) {
	switch (input) {
	case '.': case ',': case '+': case ';': case '-': case '/': case '*':
	case '^': case '!': case '=': case '<': case '>': case '(': case ')':
	case '[': case ']': case '{': case '}': case ':': case '?': case '&':
	case '%': case '|':
		return 1;
	default:
		return 0;
	}
}

int is_primitive(TOKENTYPES type) {
	switch (type) {
	case _KW_INT_:
	case _KW_SHORT_:
	case _KW_LONG_:
	case _KW_DOUBLE_:
	case _KW_FLOAT_:
	case _KW_CHAR_:
	case _KW_BOOLEAN_:
	case _KW_VOID_:
		return 1;
	default:
		return 0;
	}
}

const TOKENTYPES endIndicators[] = {
_OP_EQUALS_, _OP_SEMICOLON_, _OP_LEFT_EDGE_BRACKET_, _OP_SMALLER_CONDITION_,
_OP_GREATER_CONDITION_, _OP_SMALLER_OR_EQUAL_CONDITION_, _OP_GREATER_OR_EQUAL_CONDITION_,
_OP_NOT_EQUALS_CONDITION_, _OP_EQUALS_CONDITION_, _OP_COLON_, _KW_AND_, _KW_OR_,
_OP_MINUS_EQUALS_, _OP_PLUS_EQUALS_, _OP_MULTIPLY_EQUALS_, _OP_DIVIDE_EQUALS_,
_OP_LEFT_BRACKET_, _OP_COMMA_, _OP_CLASS_CREATOR_,
_OP_LEFT_BRACE_, _OP_QUESTION_MARK_, _OP_CLASS_ACCESSOR_, _OP_RIGHT_BRACE_};

int is_end_indicator(const TOKEN *token) {
	if (token->type == __EOF__) {
		return 1;
	}

	unsigned char length = (sizeof(endIndicators) / sizeof(endIndicators[0]));

	for (unsigned char i = 0; i < length; i++) {
		if (token->type == endIndicators[i]) {
			return 1;
		}
	}

	return 0;
}

TOKENTYPES KeywordLookupTable[] = {
	_KW_WHILE_, _KW_IF_, _KW_FUNCTION_, _KW_VAR_, _KW_BREAK_, _KW_BREAK_, _KW_RETURN_,
	_KW_DO_, _KW_CLASS_, _KW_WITH_, _KW_NEW_, _KW_TRUE_, _KW_FALSE_, _KW_NULL_, _KW_ENUM_,
	_KW_CHECK_, _KW_IS_, _KW_TRY_, _KW_CATCH_, _KW_CONTINUE_, _KW_CONST_, _KW_INCLUDE_,
	_KW_AND_, _KW_OR_, _KW_GLOBAL_, _KW_SECURE_, _KW_PRIVATE_, _KW_EXPORT_, _KW_FOR_,
	_KW_THIS_, _KW_ELSE_, _KW_INT_, _KW_DOUBLE_, _KW_FLOAT_, _KW_CHAR_,
	_KW_SHORT_, _KW_LONG_, _KW_CONSTRUCTOR_, _KW_EXTENDS_
};

int is_keyword(TOKEN *token) {
	for (int i = 0; i < sizeof(KeywordLookupTable) / sizeof(KeywordLookupTable[0]); i++) {
		if (token->type == KeywordLookupTable[i]) {
			return 1;
		}
	}

	return 0;
}

/**
 * @brief Predicts if the next token sequence might be a conditional assignment.
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
int predict_is_conditional_assignment_type(TOKEN **tokens, size_t startPos, int maxToks) {
	for (int i = startPos; i < maxToks; i++) {
		switch ((*tokens)[i].type) {
		case _OP_QUESTION_MARK_:
			return 1;
		case _OP_SEMICOLON_:
		case _OP_COLON_:
		case _KW_NEW_:
		case _OP_COMMA_:
			return 0;
		default: break;
		}
	}

	return 0;
}

int abs(int x) {
	return x < 0 ? -1 * x : x;
}