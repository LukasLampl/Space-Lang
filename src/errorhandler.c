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
#include <ctype.h>
#include <string.h>
#include "../headers/modules.h"
#include "../headers/errors.h"

#define true 1
#define false 0

extern char *FILE_NAME;

// Cache
TOKEN *TokenCache = NULL;
char *BufferCache = NULL;
int *arrayOfIndividualTokenSizesCache = NULL;
struct Node *rootNode = NULL;

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
Purpose: Initialize the sizes of the tokens, so it can be freed at an error
Return Type: void
Params: int **arrayOfIndividualTokenSizes => Sizes of the individual tokens
*/
void _init_error_token_size_cache_(int **arrayOfIndividualTokenSizes) {
    arrayOfIndividualTokenSizesCache = (*arrayOfIndividualTokenSizes);
}

void _init_error_tree_cache_(struct Node **root) {
    rootNode = (*root);
}
/*
Purpose: Throw an IO exception
Return Type: void
Params: char *source -> Which part of the compiler has Problems; char *file -> File,
        that are affected by the error (file name) 
*/
void IO_FILE_EXCEPTION(char *Source, char *file) {
    (void)printf("\nIOException at %s file: %s\n", file, Source);
    (void)printf("File: NULL => Can't processes NULL!");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the buffer is out of bounds
Return Type: void
Params: char *Step -> Part of the compiler, which has a buffer overflow
*/
void IO_BUFFER_EXCEPTION(char *Step) {
    (void)printf("BufferException: Buffer out of bounds at %s.\n", Step);

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when memory couldn't be reserved
Return Type: void
Params: void
*/
void IO_BUFFER_RESERVATION_EXCEPTION() {
    (void)printf("An error occured while trying to allocate memory.\n");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if a file couldn't be closed
Return Type: void
Params: void
*/
void IO_FILE_CLOSING_EXCEPTION() {
    (void)printf("Unable to close the file.");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
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
    char *errormsg = "Unexpected symbol has been found in the input.";
    (void)printf("\n%s\n", errormsg);
    (void)printf("At line: %i : position: %i of the input\n", line + 1, pos);

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

        if (is_space((*input)[i]) > 0 && (*input)[i] != ' ') {
            break;
        }

        if ((*input)[i] == ' ' && (*input)[i + 1] == ' ') {
            break;
        }

        forward++;
    }

    if (forward + back + 2 < strlen(errormsg)) {
        for (int i = 0; i < strlen(errormsg); i++) {
            (void)printf("-");
        }
    } else {
        for (int i = 0; i < (forward + back + 2); i++) {
            (void)printf("-");
        }
    }

    (void)printf("\n %-3i|\t", (line + 1));

    for (int i = back; i > 0; i--) {
        (void)printf("%c", (*input)[pos - i]);
    }

    (void)printf("%c", (*input)[pos]);

    for (int i = 0; i < forward; i++) {
        (void)printf("%c", (*input)[pos + i]);
    }

    (void)printf("\n %3c|\t", ' ');

    for (int i = 0; i < (back - backSpace); i++) {
        (void)printf(" ");
    }

    for (int i = 0; i < (forSpace + backSpace + 1); i++) {
        if (((pos + backSpace) - i) == pos) {
            (void)printf("^");
        } else {
            (void)printf("=");
        }
    }

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the passed input from the input reader is NULL
Return Type: void
Params: void
*/
void LEXER_NULL_TOKEN_EXCEPTION() {
    (void)printf("An fatal error occured while trying to assign the file content into tokens.\n");
    (void)printf("More data than tokens are available.\n");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if there is a pointer without correct definition
Returny Type: void
Params: void
*/
void LEXER_UNFINISHED_POINTER_EXCEPTION() {
    (void)printf("Unfinished or invalid pointer declaration");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when a token vlaue has a NULL pointer
Returny Type: void
Params: void
*/
void LEXER_NULL_TOKEN_VALUE_EXCEPTION() {
    (void)printf("Token with value NULL detected => Cannot process NULL.\n");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when a passed token is a NULL pointer reference
Returny Type: void
Params: void
*/
void LEXER_TOKEN_ERROR_EXCEPTION() {
    (void)printf("NULL token found => Cannot process NULL Token.");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the tokens couldn't be transmitted to the parse section
Return Type: void
Params: void
*/
void PARSER_TOKEN_TRANSMISSION_EXCEPTION() {
    (void)printf("An fatal error occured while transmitting the tokens to the parsing section.\n");
    (void)printf("Tokens = NULL, NULL can't be processed.");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when the needed space for the rules couldn't be reserved
Return Type: void
Params: void
*/
void PARSER_RULE_RESERVATION_EXCEPTION() {
    (void)printf("An error occured while reservating memory for the Grammar rule.\n");
    (void)printf("*Pointer NULL, NULL can't be processed.");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when the grammar file is corrupted
Return Type: void
Params: void
*/
void PARSER_RULE_FILE_CORRUPTION_EXCEPTION() {
    (void)printf("The parser rule file is corrupted and can't be processed anymore.\n");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the rules coulnd't be transmitted to the parse section
Return Type: void
Params: void
*/
void PARSER_RULE_TRANSMISSION_EXCEPTION() {
    (void)printf("An fatal error occured while transmitting the rules to the parsing section.\n");
    (void)printf("GrammarRules = NULL, NULL can't be processed.");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if the code tries to access memory out of the List
Return Type: void
Params: void
*/
void LIST_OVERFLOW_EXCEPTION() {
    (void)printf("Too much data was pushed into the list, can't process more than LIST_SIZE\n");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when the code tries to access memory out of the List
Return Type: void
Params: void
*/
void LIST_UNDERFLOW_EXCEPTION() {
    (void)printf("Can't access to data at position NULL in the list.\n");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, when there is a string, that is not finished
Return Type: void
Params: char **input => Source code;
        size_t errorPos => Position from where the string starts;
        size_t lineNumber => Line number of the string start;
*/
void LEXER_UNFINISHED_STRING_EXCEPTION(char **input, size_t errorPos, size_t lineNumber) {
    (void)printf("Unfinished string at end of file. (%s)\n", FILE_NAME);
    (void)printf("-----------------------------------------------------\n");

    char buffer[32];
    int charPosition = 0;
    int printPosition = errorPos;

    for (int i = errorPos; i > 0; i--) {
        if ((*input)[i] == '\n' || (i - 1) <= 0) {
            printPosition = i - 1;
            charPosition++;
            break;
        }

        charPosition++;
    }

    int msgLength = (int)snprintf(buffer, 32, "%li : %i | ", (lineNumber + 1), charPosition);
    (void)printf("%s", buffer);

    for (int i = printPosition; (*input)[i] != '\0'; i++) {
        (void)printf("%c", (*input)[i]);

        if ((*input)[i + 1] == '\n' || (*input)[i + 1] == '\0') {
            (void)printf("\n");
            break;
        }
    }

    for (int i = 0; i < msgLength; i++) {
        (void)printf(" ");
    }

    for (int i = printPosition; (*input)[i] != '\0'; i++) {
        if (i >= errorPos) {
            (void)printf("^");
        } else {
            (void)printf("~");
        }

        if ((*input)[i + 1] == '\n' || (*input)[i + 1] == '\0') {
            (void)printf("\n");
            break;
        }
    }

    (void)printf("-----------------------------------------------------\n");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if a token doesn't match with the expected token
Return Type: void
Params: char *value => Problem token;
        char *awaited => Expected token
*/
void SYNTAX_MISMATCH_EXCEPTION(char *value, char *awaited) {
    (void)printf("Terminated compile process due to rule mismatch!\n");
    (void)printf("Problem: \"%s\", awaited \"%s\"\n", value, awaited);

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Throw an error, if a token is a NULL pointer
Return Type: void
Params: void
*/
void SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION() {
    (void)printf("Terminated compile process due to token NULL, NULL can't be processed!\n");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

void PARSE_TREE_NODE_RESERVATION_EXCEPTION() {
    (void)printf("Terminated parsetree generation due to memory reservation exception!\n");

    if ((int)FREE_MEMORY() == true) {
        (void)exit(EXIT_SUCCESS);
    }
}

/*
Purpose: Frees the reserved memory on error throw
Return Type: int => true = successfully freed; false = error occured, terminate
Params: void
*/
int FREE_MEMORY() {
    int free = 0;

    free += (int)FREE_BUFFER(BufferCache);
    free += (int)FREE_TOKENS(TokenCache);
    free += (int)FREE_TOKEN_LENGTHS(arrayOfIndividualTokenSizesCache);
    free += (int)FREE_NODE(rootNode);

    if (free == 4) {
        (void)printf("\n\n\nProgram exited successful\n");
        return true;
    }

    (void)printf("\nProgram exited with errors\n");
    return false;
}