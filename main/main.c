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
#include "../headers/modules.h"
#include "../headers/hashmap.h"
#include "../headers/errors.h"

#include <time.h>
#include <stdlib.h>

char *FILE_NAME = NULL;
char **BUFFER = NULL;
size_t BUFFER_LENGTH = 0;
size_t TOKEN_LENGTH = 0;

int main() {
    (void)printf("SPACE-Language compiler [Version 0.0.1 - Alpha]\n");
    (void)printf("Copyright (C) 2024 Lukas Nian En Lampl\n");
    (void)printf("_________________________________________________\n\n");
    
    /////////////////////////////////////////
    //////////     INPUT READER    //////////
    /////////////////////////////////////////
    char *path = "../SPACE/prgm.txt";
    FILE_NAME = "prgm.txt";

    struct InputReaderResults inputReaderResults = ProcessInput(path);
    int *arrayOfIndividualTokenSizes = inputReaderResults.arrayOfIndividualTokenSizes;
    BUFFER = &inputReaderResults.buffer;
    BUFFER_LENGTH = inputReaderResults.fileLength;
    TOKEN_LENGTH = inputReaderResults.requiredTokenNumber;

    //////////////////////////////////
    //////////     LEXER    //////////
    //////////////////////////////////
    printf("Tokenize\n");
    TOKEN *tokens = Tokenize(&arrayOfIndividualTokenSizes);
    (void)FREE_TOKEN_LENGTHS(inputReaderResults.arrayOfIndividualTokenSizes);

    ////////////////////////////////////////
    /////     CHECK SYNTAX FUNCTION     ////
    ////////////////////////////////////////

    //0 = no errors, 1 = with errors
    int containsSyntaxErrors = (int)CheckInput(&tokens);

    /////////////////////////////////////////
    ///////     GENERATE PARSETREE     //////
    /////////////////////////////////////////
    if (containsSyntaxErrors != 0) {
        return -1;
    }

    struct Node *root = GenerateParsetree(&tokens);

    int containsSemanticErrors = (int)CheckSemantic(root);

    if (containsSemanticErrors != 0) {
        return -1;
    }

    (void)FREE_MEMORY();
    (void)printf("\n>>>>> %s has been successfully compiled. <<<<<\n", path);
}