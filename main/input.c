#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../headers/modules.h"
#include "../headers/errors.h"

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////     Input     ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void check_file_pointer(const FILE *fptr, char *directory);
void check_file_length(const size_t *length, char *directory);
void reserve_buffer(const size_t *length, char **buffer);
int get_minimum_token_number(char **buffer, int **tokenLengths, const size_t *length);
void reserve_token_lengths(const size_t *length, int **tokenLengthsArray);
int check_double_operator(char currentInput, char NextInputChar);

/*
Purpose: Read in the source files to compile, then read in the grammar file and tokenize the whole input
Return type: int
Params: NULL
*/
int main() {
    const char *directory = "../SPACE/prgm.txt";

    // File to read
    FILE *fptr = (FILE *)fopen((char *)directory, "r");

    (void)check_file_pointer(fptr, (char *)directory);

    (void)fseek(fptr, 0L, SEEK_END);
    const size_t length = (size_t)ftell(fptr);

    (void)check_file_length(&length, (char *)directory);

    // Character buffer for all input symbols
    char *buffer = NULL;
    int *tokenLenghtsArray = NULL;
    (void)reserve_buffer(&length, &buffer);
    (void)reserve_token_lengths(&length, &tokenLenghtsArray);
    (void)_init_error_buffer_cache_(&buffer);

    // Go back to the start of the file
    (void)rewind(fptr);

    // Read the contents of the file into the buffer
    (void)fread(buffer, sizeof(char), length, fptr);

    // Get Grammar ready
    (void)Process_Grammar();

    int requiredTokenLength = (int)get_minimum_token_number(&buffer, &tokenLenghtsArray, &length);
    (void)Tokenize(&buffer, &tokenLenghtsArray, &length, requiredTokenLength);

    (void)FREE_MEMORY();
    (void)printf("\n>>>>> %s has been successfully compiled. <<<<<\n", directory);

    if (fclose(fptr) == EOF) {
        (void)IO_FILE_CLOSING_EXCEPTION();
    }

    return 0;
}

/*
Purpose: Checks whether the file pointer is NULL or ot and adds a terminator character
Return type: void
Params: FILE *fptr => File Pointer; char *directory => Absolute or relative path to the file
*/
void check_file_pointer(const FILE *fptr, char *directory) {
    if (fptr == NULL) {
        char source[64] = {'\0'};
        (void)strncpy(directory, source, sizeof(source) - 1);
        (void)IO_FILE_EXCEPTION((char *)source, "input");
    }
}

/*
Purpos: Checks if the file contains something or not
Return type: void
Params: const long *length => length of the file; char *directory => Absolute or relative path to the file
*/
void check_file_length(const size_t *length, char *directory) {
    if (*length == 0) {
        char source[64] = {'\0'};
        (void)strncpy(directory, source, sizeof(source) - 1);
        (void)IO_FILE_EXCEPTION((char *)source, "input");
    }
}

/*
Purpose: Reserves a part of the memory based on the file size for the buffer
Return type: void
Params: const long *length => length of the file; char **buffer => the buffer to which the memory is allocated to
*/
void reserve_buffer(const size_t *length, char **buffer) {
    if (*length > 0) {
        *buffer = (char*)calloc(*length, sizeof(char));

        if (*buffer == NULL) {
            (void)IO_BUFFER_RESERVATION_EXCEPTION();
        }
    }
}

/*
Purpose: Reserves a part of the memory for the individual token lengths
Return type: void
Params: const long *length => length of the file; int **tokenLengths => Array to store the predicted token size
*/
void reserve_token_lengths(const size_t *length, int **tokenLengthsArray) {
    if (*length > 0) {
        *tokenLengthsArray = (int*)calloc(*length, sizeof(int));

        if (*tokenLengthsArray == NULL) {
            (void)IO_BUFFER_RESERVATION_EXCEPTION();
        }
    }
}

/*
Purpose: Determine how much Tokens are required for the file to be processed
Return type: int => how much tokens
Params: char **buffer => pointer to the file; int **tokenLengths => Array for the individual token lengths; const size_t *length => length of the buffer
*/
int get_minimum_token_number(char **buffer, int **tokenLengths, const size_t *length) {
    int tokenNumber = 0;
    int comment = 0;

    if (*buffer != NULL && *tokenLengths != NULL) {
        int tokenLength = 1;

        for (size_t i = 0; i < *length; i++) {
            // If input is a comment
            if ((*buffer)[i] == '#') {
                int jumpForward = 0;
                
                while (i + jumpForward < *length && (*buffer)[i + jumpForward] != '#') {
                    jumpForward++;
                }

                comment++;
                i += jumpForward;
                continue;
            }

            int isWhitespace = is_space((*buffer)[i]);
            int isOperator = isWhitespace == 0 ? (int)check_for_operator((*buffer)[i]) : 0;

            // If input is start of a string
            if ((*buffer)[i] == '\"') {
                tokenLength = 1;

                // Skip the whole string till the end
                while (i + tokenLength < *length && (*buffer)[i + tokenLength] != '\"') {
                    tokenLength++;
                }

                i += tokenLength;

                // "+2", 1 for the end quote and 1 for termination character
                (*tokenLengths)[tokenNumber] = tokenLength + 2;

                tokenNumber++;
                tokenLength = 1;
                continue;
            }

            if (isOperator) {
                if (i + 1 < *length && (int)check_double_operator((*buffer)[i], (*buffer)[i + 1])) {
                    (*tokenLengths)[tokenNumber] = 3;
                    tokenNumber ++;
                    i++;
                    continue;
                }

                (*tokenLengths)[tokenNumber] = 2;
                tokenNumber ++;
                continue;
            }

            if (!isWhitespace && !isOperator) {
                tokenLength = 1;

                while (i + tokenLength < *length && !(int)is_space((*buffer)[i + tokenLength])
                    && (!(int)check_for_operator((*buffer)[i + tokenLength])
                    || (((*buffer)[i + tokenLength] == '.') && (isdigit((*buffer)[i + tokenLength - 1]) 
                    || isdigit((*buffer)[i + tokenLength + 1]))))) {
                    tokenLength ++;
                }

                (*tokenLengths)[tokenNumber] = tokenLength + 1;
                
                i += tokenLength - 1;
                tokenLength = 1;
                tokenNumber ++;
                continue;
            }
        }
    }

    printf("Num: %i\n", tokenNumber - comment);
    return tokenNumber - comment;
}

/*
Purpose: Check if the input is a double operator like "==" or "+="
Return type: int => 1 = true; 0 = false;
Params: char currentInput => the current character of the input; char NextChar => following character of the input
*/
int check_double_operator(char currentInput, char NextInputChar) {

    if ((currentInput == '+' || currentInput == '-' || currentInput == '/'
        || currentInput == '*') && NextInputChar == '=') {
        return 1;
    }

    if ((currentInput == '+' && NextInputChar == '+')
        || (currentInput == '-' && NextInputChar == '-')
        || (currentInput == '=' && NextInputChar == '=')) {
        return 1;
    }

    if (currentInput == '-' && NextInputChar == '>') {
        return 1;
    }

    return 0;
}

/*
Purpose: Check if a character is a space character
Return Type: int => 1 = is whitespace char; 0 = is not a whitespace char
Params: char * character => Character to be checked
*/
int is_space(char character) {
    if (character == ' ' || character == '\n' || character == '\r'
        || character == '\t' || character == '\v') {
        return 1;
    }

    return 0;
}

/*
Purpose: Free the buffer
Return type: int => 1 = freed the buffer
Params: char *buffer => Buffer to be freed
*/
int FREE_BUFFER(char *buffer) {
    (void)free(buffer);
    return 1;
}

/*
Purpose: Free the token lengths array
Return type: int => 1 = freed the array
Params: int *lengths => the token lengths array to be freed
*/
int FREE_TOKEN_LENGTHS(int *lengths) {
    (void)free(lengths);
    return 1;
}
