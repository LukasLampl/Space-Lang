#include "stdio.h"
#include "ctype.h"
#include "string.h"
#include "stdlib.h"
#include "../headers/Token.h"
#include "../headers/errors.h"

/*-------   QUICK NOTE: EVERYTHING HERE IS IN DEPENDENCY OF THE GRAMMAR! -------*/

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////   SYNTAX ANALYSIS   /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

int is_function_call(TOKEN **tokens, size_t currentTokenPosition);
int work_down_function_call_parameters(TOKEN **tokens, size_t currentTokenPos);
int is_atom(const TOKEN *token);
int is_string(const TOKEN *token);
int is_identifier(const TOKEN *token);

int is_letter(const char character);
int is_number(const char character);
int is_bracket(const char character);
int is_brace(const char character);
int is_square_bracket(const char character);
int is_rational_operator(const char *sequence);
int is_arithmetic_operator(const char character);
int is_assignment_operator(const char *sequence);
int is_increment_operator(const char *sequence);
int is_decrement_operator(const char *sequence);
int is_underscore(const char character);
int is_bool(const char *sequence);
int is_modifier(const char *sequence);
int is_quote(const char character);
int is_logic_operator(const char *sequence);


void check(TOKEN **tokens) {
    //char *seq = "!";

    printf("is_func: %i\n", is_function_call(tokens, 0));
}

/*
Purpose: Check if a function call is going by the rule FUNCTION_CALL
Return Type: int => Number of how many tokens got checked
Params: TOKEN **tokens => Tokens array pointer with the function call;
        size_t currentTokenPos => Position from where to start checking
*/
int is_function_call(TOKEN **tokens, size_t currentTokenPosition) {
    int workedDownParameters = 0;
    size_t checkedTokens = 0;

    for (int i = currentTokenPosition; (*tokens)[i].type != __EOF__; i++) {
        TOKEN currentToken = (*tokens)[i];

        if (i - currentTokenPosition < 2 && currentToken.type == __EOF__) {
            return 0;
        }

        if (i == 0 && (int)is_identifier(&currentToken) != 1) {
            return 0;
        } else if (i == 1 && currentToken.type != _OP_RIGHT_BRACKET_) {
            return 0;
        } else if (i > 1 && workedDownParameters == 0) {
            int tokensToSkip = (int)work_down_function_call_parameters(tokens, i);
            checkedTokens += tokensToSkip;
            i += tokensToSkip;
            workedDownParameters = 1;
            continue;
        } else if (i > 1 && workedDownParameters == 1) {
            if (currentToken.type != _OP_SEMICOLON_) {
                return 0;
            } else {
                break;
            }
        }

        checkedTokens ++;
    }

    return checkedTokens;
}

/*
Purpose: Check if the parameters in a fuctioncall are valid or not
Return Type: int => Number of how many tokens got checked
Params: TOKEN **tokens => Tokens array pointer with the parameters to be checked;
        size_t currentTokenPos => Position from where to start checking
*/
int work_down_function_call_parameters(TOKEN **tokens, size_t currentTokenPos) {
    size_t i = currentTokenPos;

    while ((*tokens)[i].type != _OP_LEFT_BRACKET_ && (*tokens)[i].type != __EOF__) {
        if ((*tokens)[i + 1].type == _OP_LEFT_BRACKET_ && (*tokens)[i + 2].type != _OP_SEMICOLON_) {
            return 0;
        }

        if (i % 2 == 0) {
            if ((int)is_atom(&(*tokens)[i]) != 1) {
                return 0;
            }
        } else {
            if ((*tokens)[i].type != _OP_COMMA_ 
                && ((*tokens)[i].type == _OP_COMMA_ && (*tokens)[i + 1].type == _OP_LEFT_BRACKET_)) {
                return 0;
            }
        }

        i++;
    }

    if ((*tokens)[i].type == __EOF__) {
        return 0;
    }

    return i - currentTokenPos;
}

/*
Purpose: Check whether a given value is written according to the ATOM rule
Return Type: int => 1 = is an ATOM; 0 = is not an ATOM
Params: const TOKEN *token => Token to be checked
*/
int is_atom(const TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    if ((int)is_identifier(token) == 1) {
        return 1;
    } else if ((int)is_string(token) == 1) {
        return 1;
    } else if (token->type == _OP_LEFT_BRACKET_ || token->type == _OP_RIGHT_BRACKET_) {
        return 1;
    }

    return 0;
}

/*
Purpose: Check whether a given value is written according to the STRING rule
Return Type: int => 1 = is a string; 0 = is not a string
Params: const TOKEN *token => Token to be checked
*/
int is_string(const TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    size_t tokenValueLength = (size_t)strlen(token->value);

    if ((*token).value[0] == '"' && (*token).value[tokenValueLength - 1] == '"') {
        return 1;
    }

    return 0;
}

/*
Purpose: Check whether a given value is written according to the IDENTIFIER rule
Return Type: int => 1 = is an identifier; 0 = is not an identifier
Params: const TOKEN *token => Token to be checked
*/
int is_identifier(const TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    size_t tokenValueLength = (size_t)strlen(token->value);

    for (size_t i = 0; i < tokenValueLength; i++) {
        char currentCharacter = (*token).value[i];

        if ((int)is_letter(currentCharacter) == 1) {
            continue;
        } else if ((int)is_number(currentCharacter) == 1 && i > 0) {
            continue;
        } else if ((int)is_underscore(currentCharacter) == 1) {
            continue;
        }

        return 0;
    }

    return 1;
}

//////////////////////////////////////////////////
////////////////  BASE FUNCTIONS  ////////////////
//////////////////////////////////////////////////

/*
Purpose: Check whether a given character is a letter or not
Return Type: int => 1 = is letter; 0 = not a letter
Params: const char character => Character to be checked
*/
int is_letter(const char character) {
    switch (character) {
        case 'A':   case 'B':   case 'C':   case 'D':   case 'E':   case 'F':
        case 'G':   case 'H':   case 'I':   case 'J':   case 'K':   case 'L':
        case 'M':   case 'N':   case 'O':   case 'P':   case 'Q':   case 'R':
        case 'S':   case 'T':   case 'U':   case 'V':   case 'W':   case 'X':
        case 'Y':   case 'Z':   case 'a':   case 'b':   case 'c':   case 'd':   
        case 'e':   case 'f':   case 'g':   case 'h':   case 'i':   case 'j':
        case 'k':   case 'l':   case 'm':   case 'n':   case 'o':   case 'p':
        case 'q':   case 'r':   case 's':   case 't':   case 'u':   case 'v':
        case 'w':   case 'x':   case 'y':   case 'z':
            return 1;
        default:
            return 0;
    }
}

/*
Purpose: Check whether a given character is a number or not
Return Type: int => 1 = is a number; 0 = not a number
Params: const char character => Character to be checked
*/
int is_number(const char character) {
    switch (character) {
        case '0':   case '1':   case '2':   case '3':   case '4':   case '5':
        case '6':   case '7':   case '8':   case '9':
            return 1;
        default:
            return 0;
    }
}

/*
Purpose: Check whether a given character is a bracket or not
Return Type: int => 1 = is a bracket; 0 = not a bracket
Params: const char character => Character to be checked
*/
int is_bracket(const char character) {
    return (character == '(' || character == ')') ? 1 : 0;
}

/*
Purpose: Check whether a given character is a brace or not
Return Type: int => 1 = is a brace; 0 = not a brace
Params: const char character => Character to be checked
*/
int is_brace(const char character) {
    return (character == '{' || character == '}') ? 1 : 0;
}

/*
Purpose: Check whether a given character is a square bracket or not
Return Type: int => 1 = is a square bracket; 0 = not a square bracket
Params: const char character => Character to be checked
*/
int is_square_bracket(const char character) {
    return (character == '[' || character == ']') ? 1 : 0;
}

/*
Purpose: Check whether a given sequence is a rational operator or not
Return Type: int => 1 = is a rational operator; 0 = not a rational operator
Params: const char *sequence => Sequence to be checked
*/
int is_rational_operator(const char *sequence) {
    char rationalOperators[][3] = {"==", "<=", ">=", "!=", "<", ">"};

    for (int i = 0; i < (sizeof(rationalOperators) / sizeof(rationalOperators[0])); i++) {
        if (strcmp(sequence, rationalOperators[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

/*
Purpose: Check whether a given character is an arithmetic operator or not
Return Type: int => 1 = is an arithmetic operator; 0 = not an arithmetic operator
Params: const char character => Character to be checked
*/
int is_arithmetic_operator(const char character) {
    switch (character) {
    case '+':   case '-':   case '/':   case '*':   case '%':
        return 1;
    default:
        return 0;
    }
}

/*
Purpose: Check whether a given sequence is an assignment operator or not
Return Type: int => 1 = is an assignment operator; 0 = not an assignment operator
Params: const char *sequence => Sequence to be checked
*/
int is_assignment_operator(const char *sequence) {
    char assignmentOperator[][3] = {"+=", "-=", "*=", "/="};

    for (int i = 0; i < (sizeof(assignmentOperator) / sizeof(assignmentOperator[0])); i++) {
        if (strcmp(sequence, assignmentOperator[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

/*
Purpose: Check whether a given sequence is an increment operator or not
Return Type: int => 1 = is an increment operator; 0 = not an increment operator
Params: const char *sequence => Sequence to be checked
*/
int is_increment_operator(const char *sequence) {
    return strcmp(sequence, "++") == 0 ? 1 : 0;
}

/*
Purpose: Check whether a given sequence is a decrement operator or not
Return Type: int => 1 = is a decrement operator; 0 = not a decrement operator
Params: const char *sequence => Sequence to be checked
*/
int is_decrement_operator(const char *sequence) {
    return strcmp(sequence, "--") == 0 ? 1 : 0;
}

/*
Purpose: Check whether a given character is an underscore or not
Return Type: int => 1 = is an underscore; 0 = not an underscore
Params: const char character => Character to be checked
*/
int is_underscore(const char character) {
    return character == '_' ? 1 : 0;
}

/*
Purpose: Check whether a given sequence is a bool or not
Return Type: int => 1 = is a bool; 0 = not a bool
Params: const char *sequence => Sequence to be checked
*/
int is_bool(const char *sequence) {
    return (strcmp(sequence, "true") == 0 || strcmp(sequence, "false") == 0) ? 1 : 0;
}

/*
Purpose: Check whether a given sequence is a modifier or not
Return Type: int => 1 = is a modifier; 0 = not a modifier
Params: const char *sequence => Sequence to be checked
*/
int is_modifier(const char *sequence) {
    return (strcmp(sequence, "global") == 0
            || strcmp(sequence, "local") == 0
            || strcmp(sequence, "secure") == 0) ? 1 : 0;
}

/*
Purpose: Check whether a given character is a quote or not
Return Type: int => 1 = is a quote; 0 = not a quote
Params: const char character => Character to be checked
*/
int is_quote(const char character) {
    return character == '"' ? 1 : 0;
}

/*
Purpose: Check whether a given sequence is a logic operator or not
Return Type: int => 1 = is a logic operator; 0 = not a logic operator
Params: const char *sequence => Sequence to be checked
*/
int is_logic_operator(const char *sequence) {
    return (strcmp(sequence, "and") == 0
            || strcmp(sequence, "or") == 0
            || strcmp(sequence, "!") == 0) ? 1 : 0;
}