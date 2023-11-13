#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../headers/Token.h"
#include "../headers/errors.h"

/*-------   QUICK NOTE: EVERYTHING HERE IS IN DEPENDENCY OF THE GRAMMAR! -------*/

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////   SYNTAX ANALYSIS   /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

int is_runnable(TOKEN **tokens, size_t blockStartPosition);
int is_term(TOKEN **tokens, size_t currentTokenPosition);
int is_simple_term(TOKEN **tokens, size_t startPosition);
int is_end_indicator(const TOKEN *token);
int is_try_statement(TOKEN **tokens, size_t currentTokenPosition);
int is_catch_statement(TOKEN **tokens, size_t startPosition);
int is_include(TOKEN **tokens, size_t currentTokenPosition);
int is_enumeration(TOKEN **tokens, size_t currentTokenPosition);
int are_enumerators(TOKEN **tokens, size_t startPosition);
int is_function(TOKEN **tokens, size_t currentTokenPosition);
int is_function_call(TOKEN **tokens, size_t currentTokenPosition, int inFunction);
int is_parameter(TOKEN **tokens, size_t currentTokenPos, int inFunction);
int is_pointer(TOKEN **tokens, size_t currentTokenPosition);
int is_reference(TOKEN **tokens, size_t currentTokenPosition);
int is_atom(const TOKEN *token);
int is_string(const TOKEN *token);
int is_numeral_identifier(const TOKEN *token);
int is_identifier(const TOKEN *token);

int is_letter(const char character);
int is_number(const char character);
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
    clock_t start, end;
    start = clock();;

    printf("is_s_term: %i\n", is_simple_term(tokens, 0));

    end = clock();
    printf("Time used at syntax analysis: %f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
}

int is_runnable(TOKEN **tokens, size_t blockStartPosition) {
    if ((*tokens)[blockStartPosition].type != _OP_RIGHT_BRACE_) {
        return 0;
    }

    if ((*tokens)[blockStartPosition + 1].type != _OP_LEFT_BRACE_) {
        return 0;
    }

    return 1;
}

/*
Purpose: Check if a given TOKEN array contains a term at a specific position
Return Type: int => > 0 = how many tokens to skip; 0 = not a term
Params: TOKEN **tokens => Tokens to check;
        size_t currentTokenPosition => Position from here to start checking
*/
int is_term(TOKEN **tokens, size_t currentTokenPosition) {
    if (is_identifier(&(*tokens)[currentTokenPosition])
        && (int)is_end_indicator(&(*tokens)[currentTokenPosition + 1]) == 1) {
        return 1;
    }

    int functionCallTokensToSkip = (int)is_function_call(tokens, currentTokenPosition, 0);

    if (functionCallTokensToSkip != 0
        && (int)is_end_indicator(&(*tokens)[currentTokenPosition + functionCallTokensToSkip]) == 1) {
        return functionCallTokensToSkip;
    }

    int simpleTermTokensToSkip = (int)is_simple_term(tokens, currentTokenPosition);

    if (simpleTermTokensToSkip != 0
        && (int)is_end_indicator(&(*tokens)[currentTokenPosition + functionCallTokensToSkip]) == 1) {
        return simpleTermTokensToSkip;
    }

    return 0;
}

/*
Purpose: Check if a given array of TOKENS at a specific position is a simple term or not
Return Type: int => > 0 = how many tokens got checked; 0 = not a simple term
Params: TOKEN **tokens => Token array; size_t startPosition => Position from where to start checking
*/
int is_simple_term(TOKEN **tokens, size_t startPosition) {
    if ((*tokens)[startPosition].type == _OP_RIGHT_BRACKET_
        || (int)is_identifier(&(*tokens)[startPosition]) == 1) {
        int openBrackets = (*tokens)[startPosition].type == _OP_RIGHT_BRACKET_ ? 1 : 0;
        int counter = openBrackets == 1 ? 1 : 0;

        while ((int)is_end_indicator(&(*tokens)[startPosition + counter]) != 1
            || (*tokens)[startPosition + counter].type == _OP_LEFT_BRACKET_) {
            int endIndicator = (int)is_end_indicator(&(*tokens)[startPosition + counter]);
            
            if (endIndicator == 1) {
                if ((*tokens)[startPosition + counter].type == _OP_LEFT_BRACKET_) {
                    if ((*tokens)[startPosition + counter + 1].type != _OP_RIGHT_BRACKET_) {
                        openBrackets--;
                        counter++;
                        continue;
                    } else {
                        return 0;
                    }
                } else {
                    break;
                }
            }

            if ((*tokens)[startPosition + counter].type == _OP_RIGHT_BRACKET_) {
                if ((*tokens)[startPosition + counter + 1].type != _OP_LEFT_BRACKET_) {
                    openBrackets++;
                    counter++;
                    continue;
                } else {
                    return 0;
                }
            }

            //Layout check: <IDENTIFIER> [ARITHMETIC_OPERATOR] <IDENTIFER>
            if ((int)is_arithmetic_operator((*tokens)[startPosition + counter].value[0]) == 1) {
                if ((int)is_identifier(&(*tokens)[startPosition + counter - 1]) == 1
                    || (int)is_numeral_identifier(&(*tokens)[startPosition + counter - 1]) == 1
                    || (*tokens)[startPosition + counter - 1].type == _OP_LEFT_BRACKET_) {

                    if ((int)is_identifier(&(*tokens)[startPosition + counter + 1]) == 1
                        || (int)is_numeral_identifier(&(*tokens)[startPosition + counter + 1]) == 1
                        || (*tokens)[startPosition + counter + 1].type == _OP_RIGHT_BRACKET_) {
                        
                        counter++;
                        continue;
                    } else {
                        return 0;
                    }
                } else {
                    return 0;
                }
            }

            counter++;
        }

        if (openBrackets != 0 || (*tokens)[startPosition + counter].type == __EOF__) {
            return 0;
        }

        return counter;
    }

    return 0;
}

/*
Purpose: Check if a given TOKEN matches an "end of statement" indicator ("=", ";", "]", "}", "?", ")")
Return Type: int => 1 = is end indicator; 0 = is not an end indicator
Params: const TOKEN *token -> Token to be checked
*/
int is_end_indicator(const TOKEN *token) {
    char endIndicators[][2] = {"=", ";", "]", "}", ")", "?"};

    for (int i = 0; i < (sizeof(endIndicators) / sizeof(endIndicators[0])); i++) {
        if (strcmp(token->value, endIndicators[i]) == 0) {
            return 1;
        }
    }

    if (token->type == __EOF__) {
        return 1;
    }

    return 0;
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the TRY rule
Return Type: int => > 0 = is try statement / tokens to skip; 0 = is not a try statement
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
int is_try_statement(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type != _KW_TRY_) {
        return 0;
    }

    int skipTokensFromRunnable = (int)is_runnable(tokens, currentTokenPosition + 1);
    
    if (skipTokensFromRunnable == -1) {
        return 0;
    }

    int catchStatementSkips = (int)is_catch_statement(tokens, currentTokenPosition + skipTokensFromRunnable);

    if (catchStatementSkips != 1) {
        return 0;
    }

    return catchStatementSkips + skipTokensFromRunnable;
}

/*
Purpose: Check if the following tokens starting from startPosition match the CATCH rule
Return Type: int => > 0 = how many tokens to skip; 0 = is not a catch statement
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPosition => Position of the current token
*/
int is_catch_statement(TOKEN **tokens, size_t startPosition) {
    if ((*tokens)[startPosition].type != _KW_CATCH_) {
        printf("iddus %s %s\n", (*tokens)[startPosition - 1].value, (*tokens)[startPosition].value);
        return 0;
    }

    if ((*tokens)[startPosition + 1].type != _OP_RIGHT_BRACKET_) {
        return 0;
    }

    if (is_identifier(&(*tokens)[startPosition + 2]) != 1 || is_identifier(&(*tokens)[startPosition + 3]) != 1) {
        return 0;
    }

    int skipTokensFromRunnable = (int)is_runnable(tokens, startPosition + 4);
    
    if (skipTokensFromRunnable == -1) {
        return 0;
    }

    return skipTokensFromRunnable + 4;
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the INCLUDE rule
Return Type: int => 2 = is inclusion / tokens to skip; 0 = is not an inclusion
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
int is_include(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type != _KW_INCLUDE_) {
        return 0;
    }

    if ((int)is_string(&(*tokens)[currentTokenPosition + 1]) != 1) {
        return 0;
    }

    if ((*tokens)[currentTokenPosition + 2].type != _OP_SEMICOLON_) {
        return 0;
    }

    return 2;
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the ENUM rule
Return Type: int => > 0 is enumeration; 0 = is not an enumeration
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
int is_enumeration(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type != _KW_ENUM_) {
        return 0;
    }

    if ((int)is_identifier(&(*tokens)[currentTokenPosition + 1]) != 1) {
        return 0;
    }

    if ((*tokens)[currentTokenPosition + 2].type != _OP_RIGHT_BRACE_) {
        return 0;
    }

    int tokensToSkip = (int)are_enumerators(tokens, currentTokenPosition + 3);

    if (tokensToSkip > 0) {
        if ((*tokens)[currentTokenPosition + tokensToSkip + 4].type != _OP_SEMICOLON_) {
            return 0;
        }
    }

    return currentTokenPosition + tokensToSkip + 4;
}

/*
Purpose: Check if the contained enumaerators match the ENUMERATOR rule
Return Type: int => > 0 = how many token got checked; 0 = is not an enumerator
Params: TOKEN **tokens => TOKEN array to be checked;
        size_t startPosition => Start position of the checking procedure
*/
int are_enumerators(TOKEN **tokens, size_t startPosition) {
    size_t tokensToSkip = 0;
    int commaAwaited = 0;

    while ((*tokens)[startPosition + tokensToSkip].type != _OP_LEFT_BRACE_ && (*tokens)[startPosition + tokensToSkip].type != __EOF__) {
        switch (commaAwaited) {
        case 0:
            //Layout here: <IDENTIFIER>
            if ((int)is_identifier(&(*tokens)[startPosition + tokensToSkip]) == 1) {
                //Layout here: <IDENTIFIER> : [NUMBER]
                if ((*tokens)[startPosition + tokensToSkip + 1].type == _OP_COLON_
                    && ((*tokens)[startPosition + tokensToSkip + 2].type == _NUMBER_
                    || (*tokens)[startPosition + tokensToSkip + 3].type == _FLOAT_)) {
                    
                    tokensToSkip += 2;
                    commaAwaited = 1;
                }

                tokensToSkip ++;
                commaAwaited = 1;
                break;
            } else {
                return 0;
            }
        case 1:
            if ((*tokens)[startPosition + tokensToSkip].type == _OP_COMMA_ && (*tokens)[startPosition + tokensToSkip + 1].type != _OP_LEFT_BRACE_) {
                tokensToSkip ++;
                commaAwaited = 0;
                break;
            } else {
                return 0;
            }
        }
    }

    return tokensToSkip;
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the BREAK rule
Return Type: int => 2 = is break statement / tokens to skip; 0 = is not a break statement
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
int is_break_statement(TOKEN **tokens, size_t currentTokenPosition) {
    //Layout: break;
    if ((*tokens)[currentTokenPosition].type == _KW_BREAK_ && (*tokens)[currentTokenPosition + 1].type == _OP_SEMICOLON_) {
        return 2;
    }

    return 0;
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the FUNCTION rule
Return Type: int => 1 = is function; 0 = is not a function
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
int is_function(TOKEN **tokens, size_t currentTokenPosition) {
    int index = currentTokenPosition;
    
    switch ((*tokens)[currentTokenPosition].type) {
    case _KW_GLOBAL_:
    case _KW_SECURE_:
    case _KW_PRIVATE_:
        index++;
        break;
    default:
        break;
    }

    if ((*tokens)[index].type == _KW_FUNCTION_) {
        int skipTokens = (int)is_function_call(tokens, index + 1, 1);

        if (skipTokens == 0) {
            return 0;
        }

        if ((int)is_runnable(tokens, index + skipTokens) == -1) {
            return 0;
        } else {
            return 1;
        }
    }

    return 0;
}

/*
Purpose: Check if a function call is going by the rule FUNCTION_CALL
Return Type: int => Number of how many tokens got checked
Params: TOKEN **tokens => Tokens array pointer with the function call;
        size_t currentTokenPos => Position from where to start checking;
        int inFunction => Determines, if the function call is in a function or called seperately
*/
int is_function_call(TOKEN **tokens, size_t currentTokenPosition, int inFunction) {
    int workedDownParameters = 0;
    size_t checkedTokens = 0;

    for (int i = currentTokenPosition; (*tokens)[i].type != __EOF__; i++) {
        TOKEN currentToken = (*tokens)[i];

        if (i - currentTokenPosition < 2 && currentToken.type == __EOF__) {
            return 0;
        }
        
        if (i == currentTokenPosition && (int)is_identifier(&currentToken) != 1) {
            return 0;
        } else if (i == currentTokenPosition + 1 && currentToken.type != _OP_RIGHT_BRACKET_) {
            return 0;
        } else if (i > currentTokenPosition + 1 && workedDownParameters == 0) {
            int tokensToSkip = (int)is_parameter(tokens, i, inFunction);
            
            if (tokensToSkip == -1) {
                return 0;
            }
            
            checkedTokens += tokensToSkip;
            i += tokensToSkip;
            workedDownParameters = 1;
            continue;
        } else if (i > currentTokenPosition + 1 && workedDownParameters == 1 && inFunction == 0) {
            if (currentToken.type != _OP_SEMICOLON_) {
                return 0;
            } else {
                checkedTokens ++;
                break;
            }
        }

        checkedTokens ++;
    }

    return checkedTokens;
}

/*
Purpose: Check if the parameters in a fuctioncall are valid or not
Return Type: int => Number of how many tokens got checked; -1 = ERROR
Params: TOKEN **tokens => Tokens array pointer with the parameters to be checked;
        size_t currentTokenPos => Position from where to start checking;
        int inFunction => Is the parameter checker caled as a function parameter or functioncall parameter?
*/
int is_parameter(TOKEN **tokens, size_t currentTokenPos, int inFunction) {
    size_t i = currentTokenPos;
    int isCurrentlyComma = 0;

    while ((*tokens)[i].type != _OP_LEFT_BRACKET_ && (*tokens)[i].type != __EOF__) {
        if ((*tokens)[i + 1].type == _OP_LEFT_BRACKET_ && ((*tokens)[i + 2].type != _OP_SEMICOLON_ && inFunction == 0)) {
            return -1;
        }

        switch (isCurrentlyComma) {
        case 0:
            if ((int)is_atom(&(*tokens)[i]) == 0) {
                switch (inFunction) {
                case 0:
                    if (is_pointer(tokens, i) == 1) {
                        i++;
                        break;
                    } else if (is_reference(tokens, i) > 0) {
                        i += is_reference(tokens, i);
                        break;
                    } else {
                        return -1;
                    }
                case 1:
                    if ((int)is_pointer(tokens, i) == 1) {
                        i++;
                        break;
                    } else {
                        return -1;
                    }

                    if ((*tokens)[i].type == _OP_AND_) {
                        return -1;
                    }
                } 
                
            }

            isCurrentlyComma = 1;
            break;
        case 1:
            //Prevents that the user ends the parameter with a comma instead of IDENTFIER / ATOM
            if ((*tokens)[i].type != _OP_COMMA_ || ((*tokens)[i].type == _OP_COMMA_ && (*tokens)[i + 1].type == _OP_LEFT_BRACKET_)) {
                return -1;
            }

            isCurrentlyComma = 0;
            break;
        }

        i++;
    }

    //Checking for missing semicolon, if the functioncall should be at the end of the source code
    if ((*tokens)[i].type == __EOF__) {
        return 0;
    }

    return i - currentTokenPos;
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to the POINTER rule
Return Type: int => 1 = is pointer; 0 = not a pointer
Params: TOKEN **tokens => Token array to be checked; size_t currentTokenPosition => Position of the token that should be checked
*/
int is_pointer(TOKEN **tokens, size_t currentTokenPosition) {
    //Layout here: *<IDENTIFIER>
    if ((*tokens)[currentTokenPosition].type == _OP_MULTIPLY_ && (*tokens)[currentTokenPosition + 1].type == _IDENTIFIER_
        && (*tokens)[currentTokenPosition - 1].type != _IDENTIFIER_) {
        return 1;
    }
    
    return 0;
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to the REFERENCE rule
Return Type: int => > 1 = is reference; 0 = not a reference
Params: TOKEN **tokens => Token array to be checked; size_t currentTokenPosition => Position of the token that should be checked
*/
int is_reference(TOKEN **tokens, size_t currentTokenPosition) {
    //Layout here: &<IDENTIFIER>
    if ((*tokens)[currentTokenPosition].type == _OP_AND_ && (*tokens)[currentTokenPosition + 1].type == _IDENTIFIER_) {
        return 1;
    //Layout here: &(*<IDENTIFIER>)
    } else if ((*tokens)[currentTokenPosition].type == _OP_AND_ && (*tokens)[currentTokenPosition + 1].type == _OP_RIGHT_BRACKET_
        && (*tokens)[currentTokenPosition + 2].type == _OP_MULTIPLY_ && (*tokens)[currentTokenPosition + 3].type == _IDENTIFIER_
        && (*tokens)[currentTokenPosition + 4].type == _OP_LEFT_BRACKET_) {
        return 4;
    }

    return 0;
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

    return (*token).type == _STRING_ ? 1 : 0;
}

/*
Purpose: Check whether a given value is a number or float
Return Type: int => 1 = is a number or float; 0 = is not a number or a float
Params: const TOKEN *token => Token to be checked
*/
int is_numeral_identifier(const TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }
    
    int points = 0;

    for (int i = 0; i < (size_t)strlen(token->value); i++) {
        char currentChar = (*token).value[i];
        
        if ((int)is_digit(currentChar) == 1) {
            continue;
        } else if (currentChar == '.') {
            if (points == 0) {
                points ++;
                continue;
            } else {
                return 0;
            }
        }

        return 0;
    }

    return 1;
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