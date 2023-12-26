/////////////////////////////////////////////////////////////
///////////////////////    LICENSE    ///////////////////////
/////////////////////////////////////////////////////////////
/*
The SPACE-Language compiler compiles an input file into a runnable program.
Copyright (C) 2023  Lukas Nian En Lampl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SPACE_MODULES_H_
#define SPACE_MODULES_H_

#include "../headers/Token.h"
#include "../headers/stack.h"

// 1 = true; 0 = false
#define LEXER_DEBUG_MODE 1
#define LEXER_DISPLAY_USED_TIME 1

#define PARSER_DEBUG_MODE 1
#define PARSER_DISPLAY_USED_TIME 1

#define GRAMMAR_LEXER_DISPLAY_GRAMMAR_PROCESSING 0
#define GRAMMAR_LEXER_DISPLAY_USED_TIME 1

//////////////////////////////////////
//////////     FUNCTIONS     /////////
//////////////////////////////////////

int check_for_operator(char input);
int is_space(char character);
int is_digit(char character);

// Lexing the input
int FREE_TOKEN_LENGTHS(int *arrayOfIndividualTokenSizes);
void Tokenize(char **buffer, int **arrayOfIndividualTokenSizes, const size_t *fileLength, const size_t requiredTokenLength);

// Parse
int Generate_Parsetree(TOKEN **tokens, size_t TokenLength);

int Check_syntax(TOKEN **tokens, size_t tokenArrayLength);

//////////////////////////////////////
//////////   SYNTAX REPORT   /////////
//////////////////////////////////////
typedef enum SyntaxErrorType {
    _NONE_ = 0,              _NOT_AN_IDENTIFIER_,      _NOT_A_FLOAT_,              _NOT_AN_ATOM_,
    _NOT_A_REFERENCE_,       _NOT_A_POINTER_,          _NOT_A_PARAMETER_,          _NOT_A_POINTER_POINTING_ON_VALUE,
    _NOT_A_FUNCTION_CALL_,   _NOT_A_FUNCTION_,         _NOT_A_BREAK_,              _NOT_AN_ENUMERATOR_,
    _NOT_AN_ENUM_,           _NOT_AN_INCLUDE_,         _NOT_A_CATCH_,              _NOT_A_TRY_,
    _NOT_A_SIMPLE_TERM_,     _NOT_A_TERM_,             _NOT_AN_ASSIGNMENT_,        _NOT_A_CLASS_,
    _NOT_A_WITH_STATEMENT_,  _NOT_A_CHECK_STATEMENT_,  _NOT_AN_IS_STATEMENT_,      _NOT_AN_EXPORT_,
    _NOT_AN_EXPRESSION_,     _NOT_AN_ARRAY_ELEMENT_,   _NOT_A_VARIABLE_,           _NOT_A_FUNCTION_PARAMETER_INITIALIZER_,
    _NOT_AN_ARRAY_VAR_,      _NOT_A_NORMAL_VAR_,       _NOT_A_CONDITION_,          _NOT_A_VAR_BLOCK_ASSIGNMENT_,
    _NOT_A_CLASS_INSTANCE_,  _NOT_A_WHILE_CONDITION_,  _NOT_A_CHAINED_CONDITION_,  _NOT_A_PARAMETERED_VAR_,
    _NOT_A_WHILE_STATEMENT_, _NOT_A_DO_STATEMENT_,     _NOT_AN_ELSE_STATEMENT_,    _NOT_A_CONDITIONAL_ASSIGNMENT_,
    _NOT_AN_IF_STATEMENT_,   _NOT_AN_IF_,              _NOT_A_FOR_STATEMENT_,      _NOT_AN_ELSE_IF_STATEMENT_,
    _NOT_A_RUNNABLE_,        _NOT_A_RETURN_STATEMENT_, _NOT_A_CLASS_OBJECT_ACCESS_, _NOT_AN_ASSIGNABLE_INSTRUCTION_
} SyntaxErrorType;

typedef struct SyntaxReport {
    TOKEN *token;
    SyntaxErrorType errorType;
    int tokensToSkip;
} SyntaxReport;

#endif  // SPACE_MODULES_H_
