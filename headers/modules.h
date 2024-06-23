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

#ifndef SPACE_MODULES_H_
#define SPACE_MODULES_H_

#include "../headers/Token.h"

// 1 = true; 0 = false
#define LEXER_DEBUG_MODE 1
#define LEXER_DISPLAY_USED_TIME 1

#define SYNTAX_ANALYZER_DEBUG_MODE 0
#define SYNTAX_ANALYZER_DISPLAY_USED_TIME 1

#define PARSETREE_GENERATOR_DEBUG_MODE 1
#define PARSETREE_GENERATOR_DISPLAY_USED_TIME 1

//////////////////////////////////////
//////////     FUNCTIONS     /////////
//////////////////////////////////////

int check_for_operator(char input);
int is_space(char character);
int is_empty_string(const char* string);
int is_digit(char character);
int is_primitive(TOKENTYPES type);
int is_end_indicator(const TOKEN *token);
int is_keyword(TOKEN *token);
int predict_is_conditional_assignment_type(TOKEN **tokens, size_t startPos, int maxToks);

//Input reader
struct InputReaderResults {
    char *buffer;
    int *arrayOfIndividualTokenSizes;
    int requiredTokenNumber;
    size_t fileLength;
};

struct InputReaderResults ProcessInput(char *path);

//Lexer
TOKEN *Tokenize(int **arrayOfIndividualTokenSizes);

//Parse
struct Node *GenerateParsetree(TOKEN **tokens);

//int Check_syntax(TOKEN **tokens, size_t tokenArrayLength, char **buffer, size_t bufferSize);
int CheckInput(TOKEN **tokens);
int CheckSemantic(struct Node *root);

#endif
