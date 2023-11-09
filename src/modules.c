#include <stdio.h>
#include <stdlib.h>
#include "../headers/modules.h"

// Operators
char OPERATORS[] = {
    '.', ',', '+', ';', '-', '/', '*', '^', '!', '=', '<', '>', '(',
    ')', '[', ']', '{', '}', ':', '?', '$'
};

/*
Purpose: Check if a character is a space character
Return Type: int => 1 = is whitespace char; 0 = is not a whitespace char
Params: char character => Character to be checked
*/
int is_space(char character) {
    if (character == ' ' || character == '\n' || character == '\r'
        || character == '\t' || character == '\v') {
        return 1;
    }

    return 0;
}

/*
Purpose: Check if a character is a digit
Return Type: int => 1 = is digit; 0 = not a digit
Params: char character => Character to be checked
*/
int is_digit(char character) {
    if (character >= '0' && character <= '9') {
        return 1;
    }

    return 0;
}

/*
Purpose: Figure out if the input is an operator or not
Return Type: int => 1 = true; 0 = false;
Params: char input => Compare the current character to the operators in the OPERATORS array
*/
int check_for_operator(char input) {
    for (int i = 0; i < (sizeof(OPERATORS) / sizeof(OPERATORS[0])); i++) {
        if (input == OPERATORS[i]) {
            return 1;
        }
    }

    return 0;
}