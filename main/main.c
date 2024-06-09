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
#include "../headers/errors.h"

int main() {
    (void)printf("SPACE-Language compiler [Version 0.0.1 - Alpha]\n");
    (void)printf("Copyright (C) 2024 Lukas Nian En Lampl\n");
    (void)printf("_________________________________________________\n\n");
    
    /////////////////////////////////////////
    //////////     INPUT READER    //////////
    /////////////////////////////////////////
    char *path = "../SPACE/prgm.txt";
    char *fileName = "prgm.txt";

    struct InputReaderResults inputReaderResults = ProcessInput(path);
    int *arrayOfIndividualTokenSizes = inputReaderResults.arrayOfIndividualTokenSizes;
    char *buffer = inputReaderResults.buffer;
    size_t fileLength = inputReaderResults.fileLength;
    size_t requiredTokenNumber = inputReaderResults.requiredTokenNumber;

    //////////////////////////////////
    //////////     LEXER    //////////
    //////////////////////////////////
    printf("Tokenize\n");
    TOKEN *tokens = Tokenize(&buffer, &arrayOfIndividualTokenSizes, fileLength, requiredTokenNumber, fileName);
    (void)FREE_TOKEN_LENGTHS(inputReaderResults.arrayOfIndividualTokenSizes);

    ////////////////////////////////////////
    /////     CHECK SYNTAX FUNCTION     ////
    ////////////////////////////////////////

    //0 = no errors, 1 = with errors
    /*int containsSyntaxErrors = (int)CheckInput(&tokens, requiredTokenNumber, &buffer, fileLength, fileName);

    /////////////////////////////////////////
    ///////     GENERATE PARSETREE     //////
    /////////////////////////////////////////
    if (containsSyntaxErrors == 0) {
        (int)Generate_Parsetree(&tokens, requiredTokenNumber);
    }*/
    (int)Generate_Parsetree(&tokens, requiredTokenNumber);
    (void)FREE_MEMORY();
    (void)printf("\n>>>>> %s has been successfully compiled. <<<<<\n", path);
}