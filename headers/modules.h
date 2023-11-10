#ifndef SPACE_MODULES_H_
#define SPACE_MODULES_H_

#include "../headers/Token.h"
#include "../headers/stack.h"

// 1 = true; 0 = false
#define LEXER_DEBUG_MODE 1
#define LEXER_DISPLAY_USED_TIME 1

#define PARSER_DEBUG_MODE 0
#define PARSER_DISPLAY_GRAMMAR_PROCESSING 0
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

void check(TOKEN **tokens);

#endif  // SPACE_MODULES_H_
