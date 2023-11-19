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

// Keywords
struct lookup {
    char kwName[9];
    TOKENTYPES kwValue;
};

struct lookup KeywordLookupTable[] = {
    {"while", _KW_WHILE_},         {"if", _KW_IF_},           {"function", _KW_FUNCTION_},
    {"var", _KW_VAR_},             {"break", _KW_BREAK_},     {"return", _KW_RETURN_},
    {"do", _KW_DO_},               {"class", _KW_CLASS_},     {"with", _KW_WITH_},
    {"new", _KW_NEW_},             {"true", _KW_TRUE_},       {"false", _KW_FALSE_},
    {"null", _KW_NULL_},           {"enum", _KW_ENUM_},       {"check", _KW_CHECK_},
    {"is", _KW_IS_,},              {"try", _KW_TRY_},         {"catch", _KW_CATCH_},
    {"continue", _KW_CONTINUE_,},  {"const", _KW_CONST_},     {"include", _KW_INCLUDE_},
    {"and", _KW_AND_,},            {"or", _KW_OR_},           {"global", _KW_GLOBAL_},
    {"secure", _KW_SECURE_,},      {"private", _KW_PRIVATE_}, {"export", _KW_EXPORT_},
    {"for", _KW_FOR_,},            {"this", _KW_THIS_},       {"this", _KW_THIS_}
};

void enter_panic_mode(TOKEN **tokens, size_t currentTokenPosition);

int is_runnable(TOKEN **tokens, size_t blockStartPosition);
SyntaxReport is_class(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_assignment(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_term(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_simple_term(TOKEN **tokens, size_t startPosition);
int is_end_indicator(const TOKEN *token);
SyntaxReport is_try_statement(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_catch_statement(TOKEN **tokens, size_t startPosition);
SyntaxReport is_include(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_enumeration(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport are_enumerators(TOKEN **tokens, size_t startPosition);
SyntaxReport is_function(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_function_call(TOKEN **tokens, size_t currentTokenPosition, int inFunction);
SyntaxReport is_parameter(TOKEN **tokens, size_t currentTokenPos, int inFunction);
SyntaxReport is_pointer_pointing_to_value(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_pointer(TOKEN **tokens, size_t startPosition);
SyntaxReport is_reference(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_atom(TOKEN *token);
int is_string(TOKEN *token);
SyntaxReport is_numeral_identifier(TOKEN *token);
SyntaxReport is_identifier(TOKEN *token);
int is_keyword(char *value);

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
SyntaxReport create_syntax_report(TOKEN *tokenToAssign, int tokensToSkip, SyntaxErrorType errorType);

size_t tokenLength = 0;

void check(TOKEN **tokens, size_t tokenArrayLength) {
    tokenLength = tokenArrayLength;
    //char *seq = "!";
    clock_t start, end;
    start = clock();;
    printf("is_param: %i\n", is_function(tokens, 0).tokensToSkip);

    end = clock();
    printf("Time used at syntax analysis: %f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
}

//If error get's spotted enter the mode, so it can continue search for syntax errors
//without terminating
void enter_panic_mode(TOKEN **tokens, size_t currenTokenPosition) {

}

int is_runnable(TOKEN **tokens, size_t blockStartPosition) {
    if ((*tokens)[blockStartPosition].type != _OP_RIGHT_BRACE_) {
        return 0;
    }

    if ((*tokens)[blockStartPosition + 1].type != _OP_LEFT_BRACE_) {
        return 0;
    }

    return 2;
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to an CLASS rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Tokens to check;
        size_t currentTokenPosition => Position from here to start checking
*/
SyntaxReport is_class(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type != _KW_CLASS_) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_CLASS_);
    }

    int functionCallTokensToSkip = is_function_call(tokens, currentTokenPosition + 1, 1).tokensToSkip;
    
    if (functionCallTokensToSkip == 0) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_CLASS_);
    }

    if ((*tokens)[currentTokenPosition + functionCallTokensToSkip + 1].type != _OP_CLASS_CREATOR_
        || (*tokens)[currentTokenPosition + functionCallTokensToSkip + 2].type != _OP_RIGHT_BRACE_) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_CLASS_);
    }

    int runnableTokensToSkip = is_runnable(tokens, currentTokenPosition + functionCallTokensToSkip + 2);

    return create_syntax_report(NULL, functionCallTokensToSkip + runnableTokensToSkip + 3, _NONE_);
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to an ASSIGNMENT rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Tokens to check;
        size_t currentTokenPosition => Position from here to start checking
*/
SyntaxReport is_assignment(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type == _OP_EQUALS_) {
        if (is_term(tokens, currentTokenPosition + 1).errorType == _NONE_
        || (int)is_string(&(*tokens)[currentTokenPosition]) == 1
        || is_identifier(&(*tokens)[currentTokenPosition]).errorType == _NONE_
        || (int)is_bool((*tokens)[currentTokenPosition].value) == 1) {
            return create_syntax_report(NULL, 2, _NONE_);
        } else {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_AN_ASSIGNMENT_);
        }
    }

    return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_AN_ASSIGNMENT_);
}

/*
Purpose: Check if a given TOKEN array contains a term at a specific position
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Tokens to check;
        size_t currentTokenPosition => Position from here to start checking
*/
SyntaxReport is_term(TOKEN **tokens, size_t currentTokenPosition) {
    if (is_identifier(&(*tokens)[currentTokenPosition]).errorType == _NONE_
        && (int)is_end_indicator(&(*tokens)[currentTokenPosition + 1]) == 1) {
        return create_syntax_report(NULL, 1, _NONE_);
    }

    int functionCallTokensToSkip = is_function_call(tokens, currentTokenPosition, 0).tokensToSkip;

    if (functionCallTokensToSkip != 0
        && (int)is_end_indicator(&(*tokens)[currentTokenPosition + functionCallTokensToSkip]) == 1) {
        return create_syntax_report(NULL, functionCallTokensToSkip, _NONE_);
    }

    int simpleTermTokensToSkip = is_simple_term(tokens, currentTokenPosition).tokensToSkip;

    if (simpleTermTokensToSkip != 0
        && (int)is_end_indicator(&(*tokens)[currentTokenPosition + functionCallTokensToSkip]) == 1) {
        return create_syntax_report(NULL, simpleTermTokensToSkip, _NONE_);
    }

    return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_TERM_);
}

/*
Purpose: Check if a given array of TOKENS at a specific position is a simple term or not
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Token array; size_t startPosition => Position from where to start checking
*/
SyntaxReport is_simple_term(TOKEN **tokens, size_t startPosition) {
    if ((*tokens)[startPosition].type == _OP_RIGHT_BRACKET_
        || is_identifier(&(*tokens)[startPosition]).errorType == _NONE_) {
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
                        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_SIMPLE_TERM_);
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
                    return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_SIMPLE_TERM_);
                }
            }

            //Layout check: <IDENTIFIER> [ARITHMETIC_OPERATOR] <IDENTIFER>
            if ((int)is_arithmetic_operator((*tokens)[startPosition + counter].value[0]) == 1) {
                if (is_identifier(&(*tokens)[startPosition + counter - 1]).errorType == _NONE_
                    || is_numeral_identifier(&(*tokens)[startPosition + counter - 1]).errorType == _NONE_
                    || (*tokens)[startPosition + counter - 1].type == _OP_LEFT_BRACKET_) {

                    if (is_identifier(&(*tokens)[startPosition + counter + 1]).errorType == _NONE_
                        || is_numeral_identifier(&(*tokens)[startPosition + counter + 1]).errorType == _NONE_
                        || (*tokens)[startPosition + counter + 1].type == _OP_RIGHT_BRACKET_) {
                        
                        counter++;
                        continue;
                    } else {
                        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_SIMPLE_TERM_);;
                    }
                } else {
                    return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_SIMPLE_TERM_);;
                }
            }

            counter++;
        }

        if (openBrackets != 0 || (*tokens)[startPosition + counter].type == __EOF__) {
            return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_SIMPLE_TERM_);
        }

        return create_syntax_report(NULL, counter, _NONE_);
    }

    return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_SIMPLE_TERM_);
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
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
SyntaxReport is_try_statement(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type != _KW_TRY_) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_TRY_);
    }

    int skipTokensFromRunnable = (int)is_runnable(tokens, currentTokenPosition + 1);
    
    if (skipTokensFromRunnable == -1) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_TRY_);
    }

    int catchStatementSkips = is_catch_statement(tokens, currentTokenPosition + skipTokensFromRunnable + 1).tokensToSkip;

    if (catchStatementSkips == 0) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_TRY_);
    }

    return create_syntax_report(NULL, catchStatementSkips + skipTokensFromRunnable + 1, _NONE_);
}

/*
Purpose: Check if the following tokens starting from startPosition match the CATCH rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPosition => Position of the current token
*/
SyntaxReport is_catch_statement(TOKEN **tokens, size_t startPosition) {
    if ((*tokens)[startPosition].type != _KW_CATCH_
        || (*tokens)[startPosition + 1].type != _OP_RIGHT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_CATCH_);
    }

    if (is_identifier(&(*tokens)[startPosition + 2]).errorType != _NONE_
        || is_identifier(&(*tokens)[startPosition + 3]).errorType != _NONE_
        || (*tokens)[startPosition + 4].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_CATCH_);
    }

    int skipTokensFromRunnable = (int)is_runnable(tokens, startPosition + 5);
    
    if (skipTokensFromRunnable == -1) {
        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_CATCH_);
    }

    return create_syntax_report(NULL, skipTokensFromRunnable + 5, _NONE_);
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the INCLUDE rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
SyntaxReport is_include(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type != _KW_INCLUDE_
        && (int)is_string(&(*tokens)[currentTokenPosition + 1]) != 1
        && (*tokens)[currentTokenPosition + 2].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_AN_INCLUDE_);
    }

    return create_syntax_report(NULL, 3, _NONE_);
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the ENUM rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
SyntaxReport is_enumeration(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type != _KW_ENUM_
        || is_identifier(&(*tokens)[currentTokenPosition + 1]).errorType != _NONE_
        || (*tokens)[currentTokenPosition + 2].type != _OP_RIGHT_BRACE_) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_AN_ENUM_);
    }

    int tokensToSkip = are_enumerators(tokens, currentTokenPosition + 3).tokensToSkip;

    if (tokensToSkip > 0) {
        if ((*tokens)[currentTokenPosition + tokensToSkip + 4].type != _OP_SEMICOLON_) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_AN_ENUM_);
        }
    }

    return create_syntax_report(NULL, currentTokenPosition + tokensToSkip + 5, _NONE_);
}

/*
Purpose: Check if the contained enumaerators match the ENUMERATOR rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => TOKEN array to be checked;
        size_t startPosition => Start position of the checking procedure
*/
SyntaxReport are_enumerators(TOKEN **tokens, size_t startPosition) {
    size_t tokensToSkip = 0;
    int commaAwaited = 0;

    while ((*tokens)[startPosition + tokensToSkip].type != _OP_LEFT_BRACE_ && (*tokens)[startPosition + tokensToSkip].type != __EOF__) {
        switch (commaAwaited) {
        case 0:
            //Layout here: <IDENTIFIER>
            if (is_identifier(&(*tokens)[startPosition + tokensToSkip]).errorType == _NONE_) {
                //Layout here: <IDENTIFIER> : [NUMBER]
                if (startPosition + tokensToSkip + 2 < tokenLength - 1
                    && (*tokens)[startPosition + tokensToSkip + 1].type == _OP_COLON_
                    && (*tokens)[startPosition + tokensToSkip + 2].type == _NUMBER_) {
                    
                    tokensToSkip += 2;
                    commaAwaited = 1;
                }

                tokensToSkip ++;
                commaAwaited = 1;
                break;
            } else {
                return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_AN_ENUMERATOR_);
            }
        case 1:
            if ((*tokens)[startPosition + tokensToSkip].type == _OP_COMMA_
                && (*tokens)[startPosition + tokensToSkip + 1].type != _OP_LEFT_BRACE_) {
                tokensToSkip ++;
                commaAwaited = 0;
                break;
            } else {
                return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_AN_ENUMERATOR_);
            }
        }
    }

    return create_syntax_report(NULL, tokensToSkip, _NONE_);
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the BREAK rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
SyntaxReport is_break_statement(TOKEN **tokens, size_t currentTokenPosition) {
    //Layout: break;
    if ((*tokens)[currentTokenPosition].type == _KW_BREAK_ && (*tokens)[currentTokenPosition + 1].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, 2, _NONE_);
    }

    return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_BREAK_);
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the FUNCTION rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
SyntaxReport is_function(TOKEN **tokens, size_t currentTokenPosition) {
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
        int skipTokens = is_function_call(tokens, index + 1, 1).tokensToSkip;
        index++;

        if (skipTokens == 0) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_);
        }

        int runnableTokensToSkip = (int)is_runnable(tokens, index + skipTokens);

        if (runnableTokensToSkip == -1) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_);
        } else {
            return create_syntax_report(NULL, index + skipTokens + runnableTokensToSkip, _NONE_);
        }
    }

    return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_);
}

/*
Purpose: Check if a function call is going by the rule FUNCTION_CALL
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Tokens array pointer with the function call;
        size_t currentTokenPos => Position from where to start checking;
        int inFunction => Determines, if the function call is in a function or called seperately
*/
SyntaxReport is_function_call(TOKEN **tokens, size_t currentTokenPosition, int inFunction) {
    int workedDownParameters = 0;
    size_t checkedTokens = 0;

    for (int i = currentTokenPosition; (*tokens)[i].type != __EOF__; i++) {
        TOKEN currentToken = (*tokens)[i];

        if (i - currentTokenPosition < 2 && currentToken.type == __EOF__) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
        }
        
        if (i == currentTokenPosition && is_identifier(&currentToken).errorType != _NONE_) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
        } else if (i == currentTokenPosition + 1 && currentToken.type != _OP_RIGHT_BRACKET_) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
        } else if (i > currentTokenPosition + 1 && workedDownParameters == 0) {
            int tokensToSkip = is_parameter(tokens, i, inFunction).tokensToSkip;
            if (tokensToSkip == 0) {
                return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
            }
            
            checkedTokens += tokensToSkip - 1;
            i += tokensToSkip - 1;
            workedDownParameters = 1;
            continue;
        } else if (workedDownParameters == 1) {
            if (currentToken.type == _OP_LEFT_BRACKET_) {
                break;
            } else {
                return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
            }
        }

        checkedTokens++;
    }
    
    //+2 because the function checks 2 tokens at the start without adding them to the checkedTokens variable
    return create_syntax_report(NULL, checkedTokens + 2, _NONE_);
}

/*
Purpose: Check if the parameters in a fuctioncall are valid or not
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Tokens array pointer with the parameters to be checked;
        size_t currentTokenPos => Position from where to start checking;
        int inFunction => Is the parameter checker caled as a function parameter or functioncall parameter?
*/
SyntaxReport is_parameter(TOKEN **tokens, size_t currentTokenPos, int inFunction) {
    size_t i = currentTokenPos;
    int isCurrentlyComma = 0;

    while ((*tokens)[i].type != _OP_LEFT_BRACKET_ && (*tokens)[i].type != __EOF__) {
        switch (isCurrentlyComma) {
        case 0:
            if (is_atom(&(*tokens)[i]).errorType != _NONE_) {
                switch (inFunction) {
                case 0:
                    if (is_pointer_pointing_to_value(tokens, i).errorType == _NONE_) {
                        i++;
                        break;
                    } else if (is_reference(tokens, i).errorType == _NONE_) {
                        i += is_reference(tokens, i).tokensToSkip;
                        break;
                    } else {
                        return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_PARAMETER_);
                    }
                case 1:
                    if (is_pointer(tokens, i).errorType == _NONE_) {
                        i += is_pointer(tokens, i).tokensToSkip;
                        break;
                    } else {
                        return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_PARAMETER_);
                    }

                    if ((*tokens)[i].type == _REFERENCE_) {
                        return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_PARAMETER_);
                    }
                } 
            }

            isCurrentlyComma = 1;
            break;
        case 1:
            //Prevents that the user ends the parameter with a comma instead of IDENTFIER / ATOM
            if ((*tokens)[i].type != _OP_COMMA_
                || ((*tokens)[i].type == _OP_COMMA_ && (*tokens)[i + 1].type == _OP_LEFT_BRACKET_)) {
                printf("MCR\n");
                return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_PARAMETER_);
            }

            isCurrentlyComma = 0;
            break;
        }

        i++;
    }

    return create_syntax_report(NULL, i - currentTokenPos, _NONE_);
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to the POINTER_TO_VALUE rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Token array to be checked;
        size_t currentTokenPosition => Position of the token that should be checked
        [The currentTokePosition can either be the "*" or the ATOM of "*ATOM"]
*/
SyntaxReport is_pointer_pointing_to_value(TOKEN **tokens, size_t currentTokenPosition) {
    //Layout here: *<ATOM>
    if ((*tokens)[currentTokenPosition].type == _OP_MULTIPLY_
        && is_identifier(&(*tokens)[currentTokenPosition + 1]).errorType == _NONE_) {
        return create_syntax_report(NULL, 1, _NONE_);
    } else if ((*tokens)[currentTokenPosition - 1].type == _OP_MULTIPLY_
        && is_identifier(&(*tokens)[currentTokenPosition]).errorType == _NONE_) {
        return create_syntax_report(NULL, 1, _NONE_);
    }
    
    return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_POINTER_POINTING_ON_VALUE);
}

/*
Purpose: Check if the given TOKEN array cantains a pointer starting from a given position
Return Type: int =>  SyntaxReport => Contains errors, tokensToSkip (How many pointers point onto the pointer),
            token itself when error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPosition => Position from where to start checking
*/
SyntaxReport is_pointer(TOKEN **tokens, size_t startPosition) {
    int pointersPointingOnPointer = 0;

    while ((*tokens)[startPosition + pointersPointingOnPointer].type == _OP_MULTIPLY_
        && (startPosition + pointersPointingOnPointer) < tokenLength) {
        pointersPointingOnPointer++;
    }

    if (is_identifier(&(*tokens)[startPosition + pointersPointingOnPointer]).errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_POINTER_);
    }

    return create_syntax_report(NULL, pointersPointingOnPointer, _NONE_);
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to the REFERENCE rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Token array to be checked;
        size_t currentTokenPosition => Position of the token that should be checked
        [THE currentTokenPosition can be the '&' and the last character ')' of the "&(example)"]
*/
SyntaxReport is_reference(TOKEN **tokens, size_t currentTokenPosition) {
    //Layout here: &<ATOM>
    if ((*tokens)[currentTokenPosition].type == _REFERENCE_
        && is_identifier(&(*tokens)[currentTokenPosition + 1]).errorType == _NONE_
        && (*tokens)[currentTokenPosition + 2].type != _OP_MULTIPLY_) {
        return create_syntax_report(NULL, 1, _NONE_);
    } else if ((*tokens)[currentTokenPosition - 1].type == _REFERENCE_
        && is_identifier(&(*tokens)[currentTokenPosition]).errorType == _NONE_
        && (*tokens)[currentTokenPosition + 1].type != _OP_MULTIPLY_) {
        return create_syntax_report(NULL, 1, _NONE_);

        //Layout here: &(*<ATOM>)
    } else if (tokenLength - 1 >= currentTokenPosition + 4
        && (*tokens)[currentTokenPosition].type == _REFERENCE_
        && (*tokens)[currentTokenPosition + 1].type == _OP_RIGHT_BRACKET_
        && (*tokens)[currentTokenPosition + 2].type == _OP_MULTIPLY_
        && is_identifier(&(*tokens)[currentTokenPosition + 3]).errorType == _NONE_
        && (*tokens)[currentTokenPosition + 4].type == _OP_LEFT_BRACKET_) {
        return create_syntax_report(NULL, 4, _NONE_);
    } else if (currentTokenPosition - 4 >= 0
        && (*tokens)[currentTokenPosition].type == _OP_LEFT_BRACKET_
        && is_identifier(&(*tokens)[currentTokenPosition - 1]).errorType == _NONE_
        && (*tokens)[currentTokenPosition - 2].type == _OP_MULTIPLY_
        && (*tokens)[currentTokenPosition - 3].type == _OP_RIGHT_BRACKET_
        && (*tokens)[currentTokenPosition - 4].type == _REFERENCE_) {
        return create_syntax_report(NULL, 4, _NONE_);
    }

    return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_REFERENCE_);
}

/*
Purpose: Check whether a given value is written according to the ATOM rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN *token => Token to be checked
*/
SyntaxReport is_atom(TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    if (token->type == __EOF__) {
        return create_syntax_report(token, 0, _NOT_AN_ATOM_);
    }

    if ((SyntaxErrorType)is_identifier(token).tokensToSkip == 1) {
        return create_syntax_report(NULL, 1, _NONE_);
    } else if ((int)is_string(token) == 1) {
        return create_syntax_report(NULL, 1, _NONE_);
    } else if (token->type == _OP_LEFT_BRACKET_ || token->type == _OP_RIGHT_BRACKET_) {
        return create_syntax_report(NULL, 1, _NONE_);
    }

    return create_syntax_report(token, 0, _NOT_AN_ATOM_);
}

/*
Purpose: Check whether a given value is written according to the STRING rule
Return Type: int => 1 = is a string; 0 = is not a string
Params: TOKEN *token => Token to be checked
*/
int is_string(TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    return (*token).type == _STRING_ ? 1 : 0;
}

/*
Purpose: Check whether a given value is a number or float
Return Type: SyntaxReport => Contains error, tokensToSkip and the token itself
Params: TOKEN *token => Token to be checked
*/
SyntaxReport is_numeral_identifier(TOKEN *token) {
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
                return create_syntax_report(token, 0, _NOT_A_FLOAT_);
            }
        }

        return create_syntax_report(token, 0, _NOT_A_FLOAT_);
    }

    return create_syntax_report(NULL, 1, _NONE_);
}

/*
Purpose: Check whether a given value is written according to the IDENTIFIER rule
Return Type: SyntaxReport => Contains how many tokens to skip and if the token has an error
Params: TOKEN *token => Token to be checked
*/
SyntaxReport is_identifier(TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    if ((int)is_keyword(token->value)) {
        return create_syntax_report(token, 0, _NOT_AN_IDENTIFIER_);
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
        
        return create_syntax_report(token, 0, _NOT_AN_IDENTIFIER_);
    }

    return create_syntax_report(NULL, 1, _NONE_);
}

/*
Purpose: Check if a given value is a keyword or not
Return Type: int => 1 = is keyword; 0 = not a keyword
Params: char *value => Value to be checked
*/
int is_keyword(char *value) {
    for (int i = 0; i < (sizeof(KeywordLookupTable) / sizeof(KeywordLookupTable[0])); i++) {
        if (strcmp(value, KeywordLookupTable[i].kwName) == 0) {
            return 1;
        }
    }

    return 0;
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

/*
Purpose: Create an SyntaxReport based on the parameters
Return Type: SyntaxReport => Created SyntaxReport
Params: TOKEN *tokenToAssing => token to be shamed when error occures;
        int tokensToSkip => how many tokens to skip to next rule (non error);
        SyntaxErrorType errorType => what type of error got found
*/
SyntaxReport create_syntax_report(TOKEN *tokenToAssign, int tokensToSkip, SyntaxErrorType errorType) {    
    SyntaxReport report;
    report.token = tokenToAssign;
    report.errorType = errorType;
    report.tokensToSkip = tokensToSkip;

    return report;
}