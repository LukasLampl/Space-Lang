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

//int Check_syntax(TOKEN **tokens, size_t tokenArrayLength, char **buffer, size_t bufferSize);
int CheckInput(TOKEN **tokens, size_t tokenArrayLength, char **source, size_t sourceSize);

#endif  // SPACE_MODULES_H_
