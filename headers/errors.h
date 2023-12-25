#ifndef SPACE_ERRORS_H_
#define SPACE_ERRORS_H_

#include "../headers/modules.h"

//////////////////////////////////////////////////////////////
///////////////////     ERROR HANDLING     ///////////////////
//////////////////////////////////////////////////////////////

int FREE_MEMORY();

void _init_error_token_cache_(TOKEN **tokens);
void _init_error_buffer_cache_(char **buffer);

void IO_FILE_EXCEPTION(char *Source, char *file);
void IO_BUFFER_EXCEPTION(char *Step);
void IO_BUFFER_RESERVATION_EXCEPTION();
void IO_FILE_CLOSING_EXCEPTION();

void LEXER_UNEXPECTED_SYMBOL_EXCEPTION(char **input, int pos, int maxBackPos, int line);
void LEXER_NULL_TOKEN_EXCEPTION();
void LEXER_UNFINISHED_POINTER_EXCEPTION();
void LEXER_UNFINISHED_STRING_EXCEPTION();
void LEXER_NULL_TOKEN_VALUE_EXCEPTION();
void LEXER_TOKEN_ERROR_EXCEPTION();

void PARSER_TOKEN_TRANSMISSION_EXCEPTION();

void STACK_OVERFLOW_EXCEPTION();
void STACK_UNDERFLOW_EXCEPTION();

void SYNTAX_MISMATCH_EXCEPTION(char *value, char *awaited);

void SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();

int FREE_BUFFER(char *buffer);
int FREE_TOKENS(TOKEN *tokens);

#endif  // SPACE_ERRORS_H_
