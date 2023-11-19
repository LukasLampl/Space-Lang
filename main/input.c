#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../headers/modules.h"
#include "../headers/errors.h"

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////     Input     ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

void check_file_pointer(const FILE *fptr, char *pathToSourceFile);
void check_file_length(const size_t *length, char *pathToSourceFile);
void reserve_buffer(const size_t *fileLength, char **buffer);
int get_minimum_token_number(char **buffer, int **arrayOfIndividualTokenSizes, const size_t *bufferLength);
int add_identifiers(size_t currentBufferCharacterPosition, size_t bufferLength, char **buffer, int **arrayOfIndividualTokenSizes, size_t currentTokenNumber);
int set_operator_size(char **buffer, size_t bufferLength, size_t currentBufferCharacterPosition, int **arrayOfIndividualTokenSizes, size_t currentTokenNumber);
int skip_string(char **buffer, size_t bufferLength, size_t currentBufferCharacterPosition, int **arrayOfIndividualTokenSizes, size_t currentTokenNumber);
void reserve_token_lengths(const size_t *fileLength, int **arrayOfIndividualTokenSizes);
int check_double_operator(char currentInputChar, char NextInputChar);

/*
Purpose: Read in the source files to compile, then read in the grammar file and tokenize the whole input
Return Type: int
Params: NULL
*/
int main() {
    const char *pathToInputFile = "../SPACE/prgm.txt";

    // File to read
    FILE *filePointer = (FILE *)fopen((char *)pathToInputFile, "r");

    (void)check_file_pointer(filePointer, (char *)pathToInputFile);

    (void)fseek(filePointer, 0L, SEEK_END);
    const size_t fileLength = (size_t)ftell(filePointer);

    (void)check_file_length(&fileLength, (char *)pathToInputFile);

    // Character buffer for all input symbols
    char *buffer = NULL;
    int *arrayOfIndividualTokenSizes = NULL;
    (void)reserve_buffer(&fileLength, &buffer);
    (void)reserve_token_lengths(&fileLength, &arrayOfIndividualTokenSizes);
    (void)_init_error_buffer_cache_(&buffer);

    // Go back to the start of the file
    (void)rewind(filePointer);

    // Read the contents of the file into the buffer
    (void)fread(buffer, sizeof(char), fileLength, filePointer);

    int requiredTokenLength = (int)get_minimum_token_number(&buffer, &arrayOfIndividualTokenSizes, &fileLength);
    (void)Tokenize(&buffer, &arrayOfIndividualTokenSizes, &fileLength, requiredTokenLength);
    (void)FREE_MEMORY();
    (void)printf("\n>>>>> %s has been successfully compiled. <<<<<\n", pathToInputFile);

    if (fclose(filePointer) == EOF) {
        (void)IO_FILE_CLOSING_EXCEPTION();
    }

    return 0;
}

/*
Purpose: Checks whether the file pointer is NULL or ot and adds a terminator character
Return Type: void
Params: FILE *fptr => File Pointer; 
        char *pathToSourceFile => Absolute or relative path to the file
*/
void check_file_pointer(const FILE *fptr, char *pathToSourceFile) {
    if (fptr == NULL) {
        char sourceFile[64] = {'\0'};
        (void)strncpy(pathToSourceFile, sourceFile, 63);
        (void)IO_FILE_EXCEPTION((char *)sourceFile, "input");
    }
}

/*
Purpos: Checks if the file contains something or not
Return Type: void
Params: const long *length => Length of the file; 
        char *pathToSourceFile => Absolute or relative path to the file
*/
void check_file_length(const size_t *length, char *pathToSourceFile) {
    if (*length == 0) {
        char sourceFile[64] = {'\0'};
        (void)strncpy(pathToSourceFile, sourceFile, 63);
        (void)IO_FILE_EXCEPTION((char *)sourceFile, "input");
    }
}

/*
Purpose: Reserves a part of the memory based on the file size for the buffer
Return Type: void
Params: const long *fileLength => Length of the file; 
        char **buffer => The buffer to which the memory should be allocated
*/
void reserve_buffer(const size_t *fileLength, char **buffer) {
    if (*fileLength > 0) {
        *buffer = (char*)calloc(*fileLength, sizeof(char));

        if (*buffer == NULL) {
            (void)IO_BUFFER_RESERVATION_EXCEPTION();
        }
    }
}

/*
Purpose: Reserves a part of the memory for the individual token lengths
Return Type: void
Params: const long *fileLength => Length of the file;
        int **arrayOfIndividualTokenSizes => Array to store the predicted token size
*/
void reserve_token_lengths(const size_t *fileLength, int **arrayOfIndividualTokenSizes) {
    if (*fileLength > 0) {
        *arrayOfIndividualTokenSizes = (int*)calloc(*fileLength, sizeof(int));

        if (*arrayOfIndividualTokenSizes == NULL) {
            (void)IO_BUFFER_RESERVATION_EXCEPTION();
        }
    }
}

/*
Purpose: Determine how much Tokens are required for the file to be processed
Return Type: int => how much tokens
Params: char **buffer => Pointer to the buffer, which holds the source files content;
        int **arrayOfIndividualTokenSizes => Array for the individual token lengths;
        const size_t *bufferLength => Length of the buffer
*/
int get_minimum_token_number(char **buffer, int **arrayOfIndividualTokenSizes, const size_t *bufferLength) {
    int tokenNumber = 0;
    int comment = 0;

    if (*buffer != NULL && *arrayOfIndividualTokenSizes != NULL) {
        for (size_t i = 0; i < *bufferLength; i++) {
            // If input is a comment
            if ((*buffer)[i] == '#') {
                int jumpForward = 0;
                
                while (i + jumpForward < *bufferLength && (*buffer)[i + jumpForward] != '#') {
                    jumpForward++;
                }

                comment++;
                i += jumpForward;
                continue;
            }

            int isWhitespace = is_space((*buffer)[i]);
            int isOperator = 0;
            
            if (isWhitespace == 0 && ((int)check_for_operator((*buffer)[i]) == 1 && (*buffer)[i] != '&')) {
                isOperator = 1;
            }
            
            // If input is start of a string
            if ((*buffer)[i] == '\"') {
                i += (int)skip_string(buffer, *bufferLength, i, arrayOfIndividualTokenSizes, tokenNumber);
                tokenNumber++;
                continue;
            }
            
            if (isOperator) {
                i += (int)set_operator_size(buffer, *bufferLength, i, arrayOfIndividualTokenSizes, tokenNumber);
                tokenNumber ++;
                continue;
            }
            
            if (!isWhitespace && !isOperator) {
                i += (int)add_identifiers(i, *bufferLength, buffer, arrayOfIndividualTokenSizes, tokenNumber);
                tokenNumber ++;
                continue;
            }
        }
    }

    printf("Num: %i\n", tokenNumber - comment);
    return tokenNumber - comment;
}

/*
Purpose: Skips everything, till it scans a comment, operator, whitespace, string ect.
Return Type: int => How many chars to skip
Params: size_t currentBufferCharacterPasition => Position of the current character in the buffer;
        size_t bufferLength => Length of the buffer;
        char **buffer => Buffer, in which the content of the source file can be found;
        int **arrayOfIndividualTokens => Array in which the size of every token is stored;
        size_t tokenNumber => Current token number that is processed
*/
int add_identifiers(size_t currentBufferCharacterPosition, size_t bufferLength, char **buffer, int **arrayOfIndividualTokenSizes, size_t currentTokenNumber) {
    int identifierLength = 1;

    while (currentBufferCharacterPosition + identifierLength < bufferLength) {
        if ((int)is_space((*buffer)[currentBufferCharacterPosition + identifierLength]) == 1) {
            break;
        }

        if ((int)check_for_operator((*buffer)[currentBufferCharacterPosition + identifierLength]) == 1) {
            if ((*buffer)[currentBufferCharacterPosition + identifierLength] == '&') {
                identifierLength++;
                continue;
            } else if ((*buffer)[currentBufferCharacterPosition + identifierLength] == '.') {
                if (is_digit((*buffer)[currentBufferCharacterPosition + identifierLength - 1]) == 1
                    && is_digit((*buffer)[currentBufferCharacterPosition + identifierLength + 1]) == 1) {
                    identifierLength++;
                    continue;
                }
            } else {
                break;
            }
        }

        if ((*buffer)[currentBufferCharacterPosition + identifierLength + 1] == '#') {
            break;
        }

        identifierLength++;
    }

    (*arrayOfIndividualTokenSizes)[currentTokenNumber] = identifierLength + 1;
    
    return identifierLength - 1;
}

/*
Purpose: Sets the size of the current integer in arrayOfIndividualTokens to the given operator size
Return Type: int => 1 = double Operator; 0 = normal Operator
Params: char **buffer => Buffer, in which the content of the source file can be found;
        size_t bufferLength => Length of the buffer;
        size_t currentBufferCharacterPasition => Position of the current character in the buffer;
        int **arrayOfIndividualTokens => Array in which the size of every token is stored;
        size_t tokenNumber => Current token number that is processed
*/
int set_operator_size(char **buffer, size_t bufferLength, size_t currentBufferCharacterPosition, int **arrayOfIndividualTokenSizes, size_t currentTokenNumber) {
    if (currentBufferCharacterPosition + 1 < bufferLength) {
        char currentCharacter = (*buffer)[currentBufferCharacterPosition];
        char followingCharacter = (*buffer)[currentBufferCharacterPosition + 1];

        if ((int)check_double_operator(currentCharacter, followingCharacter)) {
            (*arrayOfIndividualTokenSizes)[currentTokenNumber] = 3;
            return 1;
        }
    }

    (*arrayOfIndividualTokenSizes)[currentTokenNumber] = 2;
    return 0;
}

/*
Purpose: Skips a given string till the end and returns how many characters to skip
Return Type: int => Count of how many characters to skip
Params: const char **buffer => Buffer, that holds the source file content;
        const size_t bufferLength => Length of the buffer;
        const size_t currentBufferCharacterPosition => Poosition of the buffer reader or the position of the current character
        int **arrayOfIndividualTokenSizes => Array with all the  sizes of every individual token;
        const size_t currentTokenNumber => Position of the current token
*/
int skip_string(char **buffer, size_t bufferLength, size_t currentBufferCharacterPosition, int **arrayOfIndividualTokenSizes, size_t currentTokenNumber) {
    size_t lengthOfString = 1;

    // Skip the whole string till the end
    while (currentTokenNumber + lengthOfString < bufferLength 
            && (*buffer)[currentBufferCharacterPosition + lengthOfString] != '\"') {
        lengthOfString++;
    }

    // "+2", 1 for the end quote and 1 for termination character
    (*arrayOfIndividualTokenSizes)[currentTokenNumber] = lengthOfString + 2;
    return lengthOfString;
}

/*
Purpose: Check if the input is a double operator like "==" or "+="
Return Type: int => 1 = true; 0 = false;
Params: char currentInput => The current character of the input;
        char NextChar => Following character of the input
*/
int check_double_operator(char currentInputChar, char NextInputChar) {

    if ((currentInputChar == '+' || currentInputChar == '-' || currentInputChar == '/'
        || currentInputChar == '*') && NextInputChar == '=') {
        return 1;
    }

    if ((currentInputChar == '+' && NextInputChar == '+')
        || (currentInputChar == '-' && NextInputChar == '-')
        || (currentInputChar == '=' && NextInputChar == '=')) {
        return 1;
    }

    if (currentInputChar == '-' && NextInputChar == '>') {
        return 1;
    }

    return 0;
}

/*
Purpose: Free the buffer
Return Type: int => 1 = freed the buffer
Params: char *buffer => Buffer to be freed
*/
int FREE_BUFFER(char *buffer) {
    (void)free(buffer);
    return 1;
}

/*
Purpose: Free the token lengths array
Return Type: int => 1 = freed the array
Params: int *arrayOfIndividualTokenSizes => The array of individual token lengths to be freed
*/
int FREE_TOKEN_LENGTHS(int *arrayOfIndividualTokenSizes) {
    (void)free(arrayOfIndividualTokenSizes);
    return 1;
}
