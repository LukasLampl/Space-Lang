#ifndef SPACE_MODULES_H_
#define SPACE_MODULES_H_

#include "../headers/Token.h"
#include "../headers/Grammar.h"
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

int is_space(char character); //input.c

// Lexing the input
int FREE_TOKEN_LENGTHS(int *lengths);
void Tokenize(char **buffer, int **tokenLengthsArray, const size_t *length, const size_t requiredTokenLength);

// Tokenize Grammar
void Process_Grammar();

// Parse
void Transmit_Grammar_To_Parsetree_Generator(Rule **rules);
int Generate_Parsetree(TOKEN **tokens, size_t TokenLength);

#endif  // SPACE_MODULES_H_
