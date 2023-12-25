#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../headers/modules.h"
#include "../headers/errors.h"

// Cache
TOKEN *TokenCache = NULL;
char *BufferCache = NULL;

/*
Purpose: Initialize the token array, so it can be freed at an error
Return Type: void
Params: TOKEN **tokens -> Pointer to the token array from the lexer
*/
void _init_error_token_cache_(TOKEN **tokens) {
    TokenCache = (*tokens);
}

/*
Purpose: Initialize the input buffer, so it can be freed at an error
Return Type: void
Params: char **buffer -> Pointer to the buffer
*/
void _init_error_buffer_cache_(char **buffer) {
    BufferCache = (*buffer);
}

/*
Purpose: Throw an IO exception
Return Type: void
Params: char *source -> Which part of the compiler has Problems; char *file -> File,
        that are affected by the error (file name) 
*/
void IO_FILE_EXCEPTION(char *Source, char *file) {
    printf("\nIOException at %s file: %s\n", file, Source);
    printf("File: NULL => Can't processes NULL!");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the buffer is out of bounds
Return Type: void
Params: char *Step -> Part of the compiler, which has a buffer overflow
*/
void IO_BUFFER_EXCEPTION(char *Step) {
    printf("BufferException: Buffer out of bounds at %s.\n", Step);

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when memory couldn't be reserved
Return Type: void
Params: void
*/
void IO_BUFFER_RESERVATION_EXCEPTION() {
    printf("An error occured while trying to allocate memory.\n");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if a file couldn't be closed
Return Type: void
Params: void
*/
void IO_FILE_CLOSING_EXCEPTION() {
    printf("Unable to close the file.");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the lexer detects an unexpected symbol
Return Type: void
Params: char **input -> Source code; int pos -> position of error;
        int maxBackPos -> Maximum backup bounds; int line -> Line at which
        the error was found
*/
void LEXER_UNEXPECTED_SYMBOL_EXCEPTION(char **input, int pos, int maxBackPos, int line) {
    char *errormsg = "unexpected symbol has been found in the input.";
    printf("\n%s\n", errormsg);
    printf("At line: %i : position: %i of the input\n", line + 1, pos);

    int back = 0, forward = 0, backSpace = 0, forSpace = 0;

    for (int i = pos; (*input)[i] != '\0'; i--) {
        if ((*input)[i] == ';' || (*input)[i] == '}') {
            back--;
            break;
        }

        if ((*input)[i] == ' ' && backSpace == 0) {
            backSpace = back;
        }

        if (is_space((*input)[i]) && (*input)[i] != ' ') {
            back--;
            break;
        }

        if ((*input)[i] == ' ' && (*input)[i - 1] == ' ') {
            back--;
            break;
        }

        back++;
    }

    for (int i = pos; (*input)[i] != '\0'; i++) {
        if ((*input)[i] == ';' || (*input)[i] == '}') {
            forward++;
            break;
        }

        if ((*input)[i] == ' ' && forSpace == 0) {
            forSpace = forward;
        }

        if (isspace((*input)[i]) && (*input)[i] != ' ') {
            break;
        }

        if ((*input)[i] == ' ' && (*input)[i + 1] == ' ') {
            break;
        }

        forward++;
    }

    if (forward + back + 2 < strlen(errormsg)) {
        for (int i = 0; i < strlen(errormsg); i++) {
            printf("-");
        }
    } else {
        for (int i = 0; i < (forward + back + 2); i++) {
            printf("-");
        }
    }

    printf("\n %-3i|\t", (line + 1));

    for (int i = back; i > 0; i--) {
        printf("%c", (*input)[pos - i]);
    }

    printf("%c", (*input)[pos]);

    for (int i = 0; i < forward; i++) {
        printf("%c", (*input)[pos + i]);
    }

    printf("\n %3c|\t", ' ');

    for (int i = 0; i < (back - backSpace); i++) {
        printf(" ");
    }

    for (int i = 0; i < (forSpace + backSpace + 1); i++) {
        if (((pos + backSpace) - i) == pos) {
            printf("^");
        } else {
            printf("=");
        }
    }

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the passed input from the input reader is NULL
Return Type: void
Params: void
*/
void LEXER_NULL_TOKEN_EXCEPTION() {
    printf("An fatal error occured while trying to assign the file content into tokens.\n");
    printf("More data than tokens are available.\n");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if there is a pointer without correct definition
Returny Type: void
Params: void
*/
void LEXER_UNFINISHED_POINTER_EXCEPTION() {
    printf("Unfinished or invalid pointer declaration");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when a token vlaue has a NULL pointer
Returny Type: void
Params: void
*/
void LEXER_NULL_TOKEN_VALUE_EXCEPTION() {
    printf("Token with value NULL detected => Cannot process NULL.\n");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when a passed token is a NULL pointer reference
Returny Type: void
Params: void
*/
void LEXER_TOKEN_ERROR_EXCEPTION() {
    printf("NULL token found => Cannot process NULL Token.");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the tokens couldn't be transmitted to the parse section
Return Type: void
Params: void
*/
void PARSER_TOKEN_TRANSMISSION_EXCEPTION() {
    printf("An fatal error occured while transmitting the tokens to the parsing section.\n");
    printf("Tokens = NULL, NULL can't be processed.");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when the needed space for the rules couldn't be reserved
Return Type: void
Params: void
*/
void PARSER_RULE_RESERVATION_EXCEPTION() {
    printf("An error occured while reservating memory for the Grammar rule.\n");
    printf("*Pointer NULL, NULL can't be processed.");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when the grammar file is corrupted
Return Type: void
Params: void
*/
void PARSER_RULE_FILE_CORRUPTION_EXCEPTION() {
    printf("The parser rule file is corrupted and can't be processed anymore.\n");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the rules coulnd't be transmitted to the parse section
Return Type: void
Params: void
*/
void PARSER_RULE_TRANSMISSION_EXCEPTION() {
    printf("An fatal error occured while transmitting the rules to the parsing section.\n");
    printf("GrammarRules = NULL, NULL can't be processed.");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the code tries to access memory out of the stack
Return Type: void
Params: void
*/
void STACK_OVERFLOW_EXCEPTION() {
    printf("Too much data was pushed into the stack, can't process more than STACK_SIZE\n");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when the code tries to access memory out of the stack
Return Type: void
Params: void
*/
void STACK_UNDERFLOW_EXCEPTION() {
    printf("Can't access to data at position NULL in the stack.\n");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

void LEXER_UNFINISHED_STRING_EXCEPTION() {
    printf("Unfinished string at end of file.");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if a token doesn't match with the expected token
Return Type: void
Params: char *value -> Problem token; char *awaited -> Expected token
*/
void SYNTAX_MISMATCH_EXCEPTION(char *value, char *awaited) {
    printf("Terminated compile process due to rule mismatch!\n");
    printf("Problem: \"%s\", awaited \"%s\"\n", value, awaited);

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

void SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION() {
    printf("Terminated compile process due to token NULL, NULL can't be processed!\n");

    if (FREE_MEMORY() == 1) {
        exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Frees the reserved memory on error throw
Return Type: int -> 1 = successfully freed; < or > 1 = error occured, terminate
Params: void
*/
int FREE_MEMORY() {
    int free = 0;

    free += (int)FREE_BUFFER(BufferCache);
    free += (int)FREE_TOKENS(TokenCache);

    if (free == 2) {
        printf("\nProgram exited successful\n");
        return 1;
    }

    printf("\nProgram exited with errors\n");
    return 0;
}