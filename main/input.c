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
int is_correct_pointer(char **buffer, size_t currentBufferCharPos, const size_t maxSize);
int skip_buffer_comment(char **buffer, size_t currentPos, size_t bufferLength, char crucialChar);

/*
Purpose: Read in the source files to compile, then read in the grammar file and tokenize the whole input
Return Type: int
Params: NULL
*/
int main() {
    const char *pathToInputFile = "../SPACE/prgm.txt";
    (void)printf("SPACE-Language compiler [Version 0.0.1 - Alpha]\n");
    (void)printf("Copyright (C) 2023 Lukas Nian En Lampl\n");
    (void)printf("_________________________________________________\n\n");

    // File to read
    FILE *filePointer = (FILE *)fopen((char *)pathToInputFile, "r");

    (void)check_file_pointer(filePointer, (char *)pathToInputFile);
    
    (void)fseek(filePointer, 0L, SEEK_END);
    const size_t fileLength = (size_t)ftell(filePointer);
    
    (void)check_file_length(&fileLength, (char*)pathToInputFile);
    
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
        (void)IO_FILE_EXCEPTION(pathToSourceFile, "input");
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
            if ((*buffer)[i] == '/'
                && ((*buffer)[i + 1] == '/'
                || (*buffer)[i + 1] == '*')) {
                i += (int)skip_buffer_comment(buffer, i, *bufferLength, (*buffer)[i + 1]);
                continue;
            }

            int isWhitespace = (int)is_space((*buffer)[i]);
            int isOperator = 0;
            
            if (isWhitespace == 0) {
                if ((int)check_for_operator((*buffer)[i]) == 1) {
                    if ((*buffer)[i] == '&'
                        || (int)is_correct_pointer(buffer, i, *bufferLength) == 1) {
                        isOperator = 0;
                    } else {
                        isOperator = 1;
                    }
                }
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
Purpose: Gives back the number of chars to skip as they are in a comment
Return Type: int => Number of chars to skip
Params: char **buffer => Buffer to be checked;
        size_t currentPos => Position from where to start skipping;
        size_t bufferLength => Maximum length of the input file;
        char crucialChar => Character that determines the type of comment
*/
int skip_buffer_comment(char **buffer, size_t currentPos, size_t bufferLength, char crucialChar) {
    int skip = 1;

    while (currentPos + skip < bufferLength) {
        if (crucialChar == '/') {
            if ((*buffer)[currentPos + skip] == '\n') {
                break;
            }
        } else if (crucialChar == '*') {
            if ((*buffer)[currentPos + skip] == '*'
                && (*buffer)[currentPos + skip + 1] == '/') {
                skip++;
                break;
            }
        } else {
            printf("Something went wrong in the comment definition.");
        }

        skip++;
    }

    return skip;
}

/*
Purpose: Check if a pointer is defined correctly
Return Type: int => 1 = is correct defined pointer; 0 = not correct defined pointer;
Params: char **buffer => Buffer to be checked;
        size_T currentBufferCharPos => Position of the current character in the buffer;
        const size_t maxSize => File size
*/
int is_correct_pointer(char **buffer, size_t currentBufferCharPos, const size_t maxSize) {
    if (buffer != NULL) {
        int skips = 0;
        
        while (currentBufferCharPos + skips < maxSize) {
            if ((*buffer)[currentBufferCharPos + skips] == '*') {
                skips++;
            } else {
                break;
            }
        }
        
        if ((int)is_space((*buffer)[currentBufferCharPos + skips]) == 1
            || (int)is_digit((*buffer)[currentBufferCharPos + skips]) == 1
            || (int)check_for_operator((*buffer)[currentBufferCharPos + skips]) == 1) {
            return 0;
        }

        return 1;
    }

    return 0;
}

/*
Purpose: Skips everything, till it scans a comment, operator, whitespace, string ect.
Return Type: int => How many chars to skip
Params: size_t currentBufferCharacterPosition => Position of the current character in the buffer;
        size_t bufferLength => Length of the buffer;
        char **buffer => Buffer, in which the content of the source file can be found;
        int **arrayOfIndividualTokens => Array in which the size of every token is stored;
        size_t tokenNumber => Current token number that is processed
*/
int add_identifiers(size_t currentBufferCharacterPosition, size_t bufferLength, char **buffer, int **arrayOfIndividualTokenSizes, size_t currentTokenNumber) {
    int identifierLength = 0;
    int isInReferenceToPointer = 0;

    while (currentBufferCharacterPosition + identifierLength < bufferLength) {
        if ((int)is_space((*buffer)[currentBufferCharacterPosition + identifierLength]) == 1) {
            break;
        }

        if ((int)check_for_operator((*buffer)[currentBufferCharacterPosition + identifierLength]) == 1) {
            if ((*buffer)[currentBufferCharacterPosition + identifierLength] == '&') {
                if ((*buffer)[currentBufferCharacterPosition + identifierLength + 1] == '(') {
                    isInReferenceToPointer = 1;
                }
                
                identifierLength++;
                continue;
            } else if ((*buffer)[currentBufferCharacterPosition + identifierLength] == '.') {
                if ((int)is_digit((*buffer)[currentBufferCharacterPosition + identifierLength - 1]) == 1
                    && (int)is_digit((*buffer)[currentBufferCharacterPosition + identifierLength + 1]) == 1) {
                    identifierLength++;
                    continue;
                } else {
                    break;
                }
            } else if ((*buffer)[currentBufferCharacterPosition + identifierLength] == '*') {
                /*if ((int)is_space((*buffer)[currentBufferCharacterPosition + identifierLength + 1]) == 0
                    && (int)is_digit((*buffer)[currentBufferCharacterPosition + identifierLength + 1]) == 0) {
                    identifierLength++;
                    continue;
                } else*/ if (isInReferenceToPointer == 1) {
                    identifierLength++;
                    continue;
                }
            } else if ((*buffer)[currentBufferCharacterPosition + identifierLength] == ')'
                || (*buffer)[currentBufferCharacterPosition + identifierLength] == '(') {
                if (isInReferenceToPointer == 1) {
                    isInReferenceToPointer = 0;
                    identifierLength++;
                    continue;
                } else {
                    break;
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
            && ((*buffer)[currentBufferCharacterPosition + lengthOfString] != '\"'
            || (*buffer)[currentBufferCharacterPosition + lengthOfString - 1] == '\\')) {
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

    if ((currentInputChar == '<' || currentInputChar == '>')
        && NextInputChar == '=') {
        return 1;
    }

    if (currentInputChar == '-' && NextInputChar == '>') {
        return 1;
    } else if (currentInputChar == '=' && NextInputChar == '>') {
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
