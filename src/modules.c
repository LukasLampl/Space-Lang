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
#include <string.h>
#include "../headers/modules.h"

// Operators
char OPERATORS[] = {
    '.', ',', '+', ';', '-', '/', '*', '^', '!', '=', '<', '>', '(',
    ')', '[', ']', '{', '}', ':', '?', '$', '&', ':', '%'
};

/*
Purpose: Check if a character is a space character
Return Type: int => 1 = is whitespace char; 0 = is not a whitespace char
Params: const char character => Character to be checked
*/
int is_space(const char character) {
    switch (character) {
    case '\n':
        return 2;
    case ' ':
    case '\r':
    case '\v':
    case '\t':
    case '\f':
        return 1;
    default:
        return 0;
    }
}

/*
Purpose: Checks if a string is empty or not
Return Type: int => 1 = is empty; 0 = not empty
Params: const char *string => String to be checked
*/
int is_empty_string(const char* string) {
    if (string == NULL) {
        return 1;
    } else if ((int)strlen(string) <= 1) {
        return 1;
    } else if (string[0] == '\0') {
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

int is_primitive(TOKENTYPES type) {
    switch (type) {
    case _KW_INT_:
    case _KW_SHORT_:
    case _KW_LONG_:
    case _KW_DOUBLE_:
    case _KW_FLOAT_:
    case _KW_CHAR_:
    case _KW_STRING_:
    case _KW_BOOLEAN_:
        return 1;
    default:
        return 0;
    }
}