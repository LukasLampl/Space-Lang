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

typedef enum ParameterUse {
    _PARAM_FUNCTION_, _PARAM_FUNCTION_CALL_, _PARAM_VARIABLE_, _PARAM_WITH_STATEMENT_, _PARAM_CLASS_
} ParameterUse;

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

int is_runnable(TOKEN **tokens, size_t blockStartPosition, int withBlock);
SyntaxReport is_variable(TOKEN **tokens, size_t startPos);
SyntaxReport is_normal_var(TOKEN **tokens, size_t startPos);
SyntaxReport is_parametered_var(TOKEN **tokens, size_t startPos);
SyntaxReport is_array_var(TOKEN **tokens, size_t startPos);
SyntaxReport is_var_block_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport is_var_array(TOKEN **tokens, size_t start);
SyntaxReport is_array_element(TOKEN **tokens, size_t startPos);
SyntaxReport is_expression(TOKEN **tokens, size_t startPos);
SyntaxReport is_export(TOKEN **tokens, size_t startPos);
SyntaxReport is_check_statement(TOKEN **tokens, size_t currentTokenPos);
SyntaxReport is_is_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_class(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_with_statement(TOKEN **tokens, size_t currentTokenPos);
SyntaxReport is_assignment(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_term(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_simple_term(TOKEN **tokens, size_t startPosition, int inFunctionCall);
int is_end_indicator(const TOKEN *token);
SyntaxReport is_try_statement(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_catch_statement(TOKEN **tokens, size_t startPosition);
SyntaxReport is_include(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_enumeration(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport are_enumerators(TOKEN **tokens, size_t startPosition);
SyntaxReport is_function(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_function_call(TOKEN **tokens, size_t currentTokenPosition, ParameterUse parameterUsage);
SyntaxReport is_parameter(TOKEN **tokens, size_t currentTokenPos, ParameterUse usage, TOKENTYPES crucialType);
SyntaxReport is_function_parameter_initializer(TOKEN **token, size_t startPos);
SyntaxReport is_pointer_pointing_to_value(TOKEN *token);
SyntaxReport is_pointer(TOKEN *token);
SyntaxReport is_reference(TOKEN *token);
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
void throw_error(TOKEN *token, SyntaxErrorType error);

size_t tokenLength = 0;

void check(TOKEN **tokens, size_t tokenArrayLength) {
    tokenLength = tokenArrayLength;

    clock_t start, end;
    start = clock();

    printf("is_var: %i\n", is_variable(tokens, 0).tokensToSkip);

    end = clock();
    printf("Time used at syntax analysis: %f\n", ((double) (end - start)) / CLOCKS_PER_SEC);
}

//If error get's spotted enter the mode, so it can continue search for syntax errors
//without terminating
void enter_panic_mode(TOKEN **tokens, size_t currenTokenPosition) {

}

int is_runnable(TOKEN **tokens, size_t blockStartPosition, int withBlock) {
    switch (withBlock) {
    case 1:
        if ((*tokens)[blockStartPosition].type != _OP_RIGHT_BRACE_) {
            return 0;
        }

        if ((*tokens)[blockStartPosition + 1].type != _OP_LEFT_BRACE_) {
            return 0;
        }

        break;
    case 0:
        return 0;
        break;
    }

    return 2;
}

SyntaxReport is_variable(TOKEN **tokens, size_t startPos) {
    int skipper = 0;

    switch ((*tokens)[startPos].type) {
    case _KW_CONST_:
        skipper = 1;
        break;
    default:
        break;
    }

    if ((*tokens)[startPos + skipper].type == _KW_VAR_) {
        if (is_identifier(&(*tokens)[startPos + skipper + 1]).errorType == _NONE_
            || is_pointer(&(*tokens)[startPos + skipper + 1]).errorType == _NONE_) {

            SyntaxReport normalVar = is_normal_var(tokens, startPos + skipper + 1);
            SyntaxReport parameteredVar = is_parametered_var(tokens, startPos + skipper + 1);
            SyntaxReport arrayVar = is_array_var(tokens, startPos + skipper + 1);

            if (normalVar.errorType == _NONE_) {
                return create_syntax_report(NULL, normalVar.tokensToSkip + skipper + 1, _NONE_);
            } else if (parameteredVar.errorType == _NONE_) {
                return create_syntax_report(NULL, parameteredVar.tokensToSkip + skipper + 1, _NONE_);
            } else if (arrayVar.errorType == _NONE_) {
                return create_syntax_report(NULL, arrayVar.tokensToSkip + skipper + 1, _NONE_);
            }
        }
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_VARIABLE_);
}

/*
Purpose: Check if a given token array matches the array VARIABLE definition
Return Type: SyntaxReport => ErrorType = _NONE_ on arrayVar
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start
Layout: "var IDEN[][...] = IDEN;" OR "var IDEN[][...];"
*/
SyntaxReport is_array_var(TOKEN **tokens, size_t startPos) {
    SyntaxReport varArrayReport = is_var_array(tokens, startPos + 1);

    //Here: var IDEN[][...] = IDEN;
    if (varArrayReport.errorType == _NONE_) {
        SyntaxReport assignmentReport = is_assignment(tokens, startPos + varArrayReport.tokensToSkip + 1);

        if (assignmentReport.errorType == _NONE_
            && (*tokens)[startPos + varArrayReport.tokensToSkip + assignmentReport.tokensToSkip].type == _OP_SEMICOLON_) {
            return create_syntax_report(NULL, varArrayReport.tokensToSkip + assignmentReport.tokensToSkip + 1, _NONE_);
        
        //Here: var IDEN[][...];
        } else if ((*tokens)[startPos + varArrayReport.tokensToSkip + 1].type == _OP_SEMICOLON_) {
            return create_syntax_report(NULL, varArrayReport.tokensToSkip + 2, _NONE_);
        }

        SyntaxReport blockAssignReport = is_var_block_assignment(tokens, startPos + varArrayReport.tokensToSkip + 1);

        if (blockAssignReport.errorType == _NONE_
            && (*tokens)[startPos + varArrayReport.tokensToSkip + blockAssignReport.tokensToSkip + 1].type == _OP_SEMICOLON_) {
            return create_syntax_report(NULL, varArrayReport.tokensToSkip + blockAssignReport.tokensToSkip + 1, _NONE_);
        }
    }

    return create_syntax_report(NULL, 0, _NOT_AN_ARRAY_VAR_);
}

SyntaxReport is_var_block_assignment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type == _OP_EQUALS_) {
        int openBraces = 0;
        int shouldBeComma = 0;
        int jumper = 1;

        while ((*tokens)[startPos + jumper].type != __EOF__
            || startPos + jumper < tokenLength) {

            switch ((*tokens)[startPos + jumper].type) {
            case _OP_RIGHT_BRACE_:
                openBraces++;
                jumper++;
                continue;
            case _OP_LEFT_BRACE_:
                openBraces--;
                jumper++;
                shouldBeComma = 1;
                continue;
            default:
                break;
            }

            if ((*tokens)[startPos + jumper].type == _OP_SEMICOLON_) {
                break;
            }

            switch (shouldBeComma) {
            case 0:
                shouldBeComma = 1;

                if (is_identifier(&(*tokens)[startPos + jumper]).errorType != _NONE_) {

                    if (is_numeral_identifier(&(*tokens)[startPos + jumper]).errorType == _NONE_
                        && (int)is_arithmetic_operator((*tokens)[startPos + jumper + 1].value[0]) == 0) {
                        break;
                    }
                    
                    if (is_pointer_pointing_to_value(&(*tokens)[startPos + jumper]).errorType == _NONE_
                        || is_pointer(&(*tokens)[startPos + jumper]).errorType == _NONE_
                        || is_reference(&(*tokens)[startPos + jumper]).errorType == _NONE_) {
                        break;
                    }

                    SyntaxReport termReport = is_term(tokens, startPos + jumper);
                    
                    if (termReport.errorType == _NONE_) {
                        jumper += termReport.tokensToSkip;
                        continue;
                    }
                    
                    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_VAR_BLOCK_ASSIGNMENT_);
                }

                break;
            case 1:
                if ((*tokens)[startPos + jumper].type != _OP_COMMA_) {
                    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_VAR_BLOCK_ASSIGNMENT_);
                }
                
                shouldBeComma = 0;
                break;
            }

            jumper++;
        }

        if ((*tokens)[startPos + jumper].type == _OP_SEMICOLON_) {
            return create_syntax_report(NULL, jumper, _NONE_);
        }
    }
    
    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_VAR_BLOCK_ASSIGNMENT_);
}

/*
Purpose: Check if a given token array matches the parametered VARIABLE definition
Return Type: SyntaxReport => ErrorType = _NONE_ on parameteredVar
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start
Layout: "var IDEN, IDEN, ... = IDEN;" OR "var IDEN, IDEN, ...;"
*/
SyntaxReport is_parametered_var(TOKEN **tokens, size_t startPos) {
    SyntaxReport paramReport = is_parameter(tokens, startPos, _PARAM_VARIABLE_, _OP_EQUALS_);
    
    //Here: var IDEN, IDEN, ... = IDEN;
    if (paramReport.errorType == _NONE_) {
        SyntaxReport paramAssignReport = is_assignment(tokens, startPos + paramReport.tokensToSkip);
        if (paramAssignReport.errorType == _NONE_
            && (*tokens)[startPos + paramAssignReport.tokensToSkip + paramReport.tokensToSkip - 1].type == _OP_SEMICOLON_) {
            return create_syntax_report(NULL, paramAssignReport.tokensToSkip + paramReport.tokensToSkip, _NONE_);
        }
    }

    paramReport = is_parameter(tokens, startPos, _PARAM_VARIABLE_, _OP_SEMICOLON_);

    //Here: var IDEN, IDEN, ...;
    if ((*tokens)[startPos + paramReport.tokensToSkip].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, paramReport.tokensToSkip + 1, _NONE_);
    }

    return create_syntax_report(NULL, 0, _NOT_A_PARAMETERED_VAR_);
}

/*
Purpose: Check if a given token array matches the normal VARIABLE definition
Return Type: SyntaxReport => ErrorType = _NONE_ on normalVar
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start
Layout: "var IDEN = IDEN;" OR "var IDEN;"
*/
SyntaxReport is_normal_var(TOKEN **tokens, size_t startPos) {
    SyntaxReport assignmentReport = is_assignment(tokens, startPos + 2);

    //Here: var IDEN = IDEN;
    if (assignmentReport.errorType == _NONE_
        && (*tokens)[startPos + assignmentReport.tokensToSkip + 2].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, assignmentReport.tokensToSkip + 3, _NONE_);

    //Here: "var IDEN;"
    } else if ((*tokens)[startPos + 2].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, 3, _NONE_);
    }

    return create_syntax_report(NULL, 0, _NOT_A_NORMAL_VAR_);
}

/*
Purpose: Check if a given token array match the ARRAY_ELEMENT rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success
Params: TOKEN **tokens => Token array to be scanned;
        size_t start => Position from where to start scanning
*/
SyntaxReport is_var_array(TOKEN **tokens, size_t start) {
    int jumper = 0;

    while ((*tokens)[start + jumper].type != __EOF__
        || (*tokens)[start + jumper].type != _OP_EQUALS_
        || (*tokens)[start + jumper].type != _OP_SEMICOLON_) {
        SyntaxReport rep = is_array_element(tokens, start + jumper);

        if (rep.errorType == _NONE_) {
            jumper += rep.tokensToSkip;
        } else {
            break;
        }
    }
    
    if ((*tokens)[start + jumper].type == _OP_EQUALS_
        || (*tokens)[start + jumper].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, jumper, _NONE_);
    } else {
        return create_syntax_report(&(*tokens)[start], 0, _NOT_AN_ARRAY_VAR_);
    }

}

/*
Purpose: Check if a token sequence matches the ARRAY_ELEMENT rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success and _NOT_AN_ARRAY_ELEMENT_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_array_element(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type == _OP_RIGHT_EDGE_BRACKET_) {
        SyntaxReport expressionReport = is_expression(tokens, startPos + 1);

        if (expressionReport.errorType == _NONE_) {
            if ((*tokens)[startPos + expressionReport.tokensToSkip + 1].type == _OP_LEFT_EDGE_BRACKET_) {
                return create_syntax_report(NULL, 2 + expressionReport.tokensToSkip, _NONE_);
            }
        } else if ((*tokens)[startPos + 1].type == _OP_LEFT_EDGE_BRACKET_) {
            return create_syntax_report(NULL, 2, _NONE_);
        }
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_ARRAY_ELEMENT_);
}

/*
Purpose: Check if a token sequence matches the EXPRESSION rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success and _NOT_AN_EXPRESSION_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_expression(TOKEN **tokens, size_t startPos) {
    SyntaxReport assignmentReport = is_assignment(tokens, startPos + 1);

    if (is_identifier(&(*tokens)[startPos]).errorType == _NONE_
        && assignmentReport.errorType == _NONE_
        && (int)is_end_indicator(&(*tokens)[startPos + assignmentReport.tokensToSkip + 1]) == 1) {
        return create_syntax_report(NULL, assignmentReport.tokensToSkip + 1, _NONE_);
    }

    if (is_identifier(&(*tokens)[startPos]).errorType == _NONE_
        && ((int)is_decrement_operator((*tokens)[startPos + 1].value) == 1
        || (int)is_increment_operator((*tokens)[startPos].value) == 1)
        && (int)is_end_indicator(&(*tokens)[startPos + assignmentReport.tokensToSkip + 2]) == 1) {
        return create_syntax_report(NULL, 2, _NONE_);
    }
    
    SyntaxReport termReport = is_term(tokens, startPos);

    if (termReport.errorType == _NONE_
        && (int)is_end_indicator(&(*tokens)[startPos + termReport.tokensToSkip]) == 1) {
        return create_syntax_report(NULL, termReport.tokensToSkip, _NONE_);
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_EXPRESSION_);
}

/*
Purpose: Check if a given token array at a specific position is defined accordingly to the EXPORT rule
Return Type: SyntaxReport => ErrorType = _NOT_AN_EXPORT_ on error, else _NONE_
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start from validating
*/
SyntaxReport is_export(TOKEN **tokens, size_t startPos) {
    if (startPos + 3 < tokenLength
        && (*tokens)[startPos].type == _KW_EXPORT_
        && is_identifier(&(*tokens)[startPos + 1]).errorType == _NONE_
        && (*tokens)[startPos + 2].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, 3, _NONE_);
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_EXPORT_);
}

/*
Purpose: Check if a given token array contains a check statement at a specific position
Return Type: SyntaxReport => Returns _NONE_ as errortype when erverything works fine
Params: TOKEN **tokens => Token array to be scanned
        size_t currentTokenPos => Position from where to start checking
*/
SyntaxReport is_check_statement(TOKEN **tokens, size_t currentTokenPos) {
    if (currentTokenPos + 4 > tokenLength) {
        return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_CHECK_STATEMENT_);
    }
    
    if (((*tokens)[currentTokenPos].type != _KW_CHECK_
        || (*tokens)[currentTokenPos + 1].type != _OP_RIGHT_BRACKET_
        || is_identifier(&(*tokens)[currentTokenPos + 2]).errorType != _NONE_
        || (*tokens)[currentTokenPos + 3].type != _OP_LEFT_BRACKET_)) {
        return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_CHECK_STATEMENT_);
    }

    if ((*tokens)[currentTokenPos + 4].type == _OP_RIGHT_BRACE_) {
        SyntaxReport isTokenSkip = is_is_statement(tokens, currentTokenPos + 5);

        if (isTokenSkip.errorType == _NONE_) {
            if ((*tokens)[currentTokenPos + isTokenSkip.tokensToSkip + 5].type == _OP_LEFT_BRACE_) {
                return create_syntax_report(NULL, isTokenSkip.tokensToSkip + 6, _NONE_);
            }
        }
    }

    return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_CHECK_STATEMENT_);
}

/*
Purpose: Check if a given token array at a given position is according to the IS_STATEMENT rule
Return Type: SyntaxReport => Contains Token on error, but return error _NONE_ on success
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking;
*/
SyntaxReport is_is_statement(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int isRunnableTurn = 0;

    while ((*tokens)[startPos + jumper].type != _OP_LEFT_BRACE_
        && (*tokens)[startPos + jumper].type != __EOF__) {
        switch (isRunnableTurn) {
        case 0:
            if (startPos + jumper + 2 > tokenLength) {
                return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IS_STATEMENT_);
            }

            if ((*tokens)[startPos + jumper].type == _KW_IS_
                && (is_identifier(&(*tokens)[startPos + jumper + 1]).errorType == _NONE_
                || is_numeral_identifier(&(*tokens)[startPos + jumper + 1]).errorType == _NONE_)
                && (*tokens)[startPos + jumper + 2].type == _OP_COLON_) {
                isRunnableTurn = 1;
                jumper += 2;
                continue;
            } else {
                return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IS_STATEMENT_);
            }
        case 1:
            if ((int)is_runnable(tokens, startPos + jumper, 0) == 0) {
                isRunnableTurn = 0;
                jumper += (int)is_runnable(tokens, startPos + jumper, 0) + 1;
                continue;
            } else {
                return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IS_STATEMENT_);
            }
        }
    }

    if ((*tokens)[startPos + jumper].type == __EOF__) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IS_STATEMENT_);
    }
    
    return create_syntax_report(NULL, jumper, _NONE_);
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

    int functionCallTokensToSkip = is_function_call(tokens, currentTokenPosition + 1, _PARAM_CLASS_).tokensToSkip;
    
    if (functionCallTokensToSkip == 0) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_CLASS_);
    }

    int withTokensToSkip = is_with_statement(tokens, currentTokenPosition + functionCallTokensToSkip + 1).tokensToSkip;

    if ((*tokens)[currentTokenPosition + functionCallTokensToSkip + withTokensToSkip + 1].type != _OP_CLASS_CREATOR_
        || (*tokens)[currentTokenPosition + functionCallTokensToSkip + withTokensToSkip + 2].type != _OP_RIGHT_BRACE_) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_CLASS_);
    }

    int runnableTokensToSkip = is_runnable(tokens, currentTokenPosition + withTokensToSkip + functionCallTokensToSkip + 2, 1);

    return create_syntax_report(NULL, functionCallTokensToSkip + withTokensToSkip + runnableTokensToSkip + 2, _NONE_);
}

/*
Purpose: Check whether a class contains a with statement or not
Return Type: SyntaxReport => Contains tokensToSkip; returns _NOT_A_WITH_STATEMENT if the class doesn't contain
             a with statement
Params: TOKEN **tokens => Tokens to be scanned;
        size_t currentTokenPos => Start position of the scan
*/
SyntaxReport is_with_statement(TOKEN **tokens, size_t currentTokenPos) {
    if ((*tokens)[currentTokenPos].type == _KW_WITH_) {
        SyntaxReport isParam = is_parameter(tokens, currentTokenPos + 1, _PARAM_WITH_STATEMENT_, _OP_CLASS_CREATOR_);

        if (isParam.errorType == _NONE_) {
            return create_syntax_report(NULL, isParam.tokensToSkip + 1, _NONE_);
        } else {
            return create_syntax_report(&(*tokens)[currentTokenPos + 1], 0, _NOT_A_WITH_STATEMENT_);
        }
    }
    
    return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_WITH_STATEMENT_);
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to an ASSIGNMENT rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Tokens to check;
        size_t currentTokenPosition => Position from here to start checking
*/
SyntaxReport is_assignment(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type == _OP_EQUALS_) {
        SyntaxReport termReport = is_term(tokens, currentTokenPosition + 1);

        if (termReport.errorType == _NONE_
            && (int)is_end_indicator(&(*tokens)[currentTokenPosition + termReport.tokensToSkip + 1]) == 1) {
            return create_syntax_report(NULL, termReport.tokensToSkip + 2, _NONE_);
        } else if (((int)is_string(&(*tokens)[currentTokenPosition + 1]) == 1
            || is_identifier(&(*tokens)[currentTokenPosition + 1]).errorType == _NONE_
            || (int)is_bool((*tokens)[currentTokenPosition + 1].value) == 1
            || (*tokens)[currentTokenPosition + 1].type == _KW_NULL_
            || is_function_call(tokens, currentTokenPosition + 1, _PARAM_FUNCTION_CALL_).errorType == _NONE_
            || is_term(tokens, currentTokenPosition).errorType == _NONE_)
            && (int)is_end_indicator(&(*tokens)[currentTokenPosition + 2]) == 1) {
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
    if ((is_identifier(&(*tokens)[currentTokenPosition]).errorType == _NONE_
        || is_numeral_identifier(&(*tokens)[currentTokenPosition]).errorType == _NONE_)) {
        if ((int)is_end_indicator(&(*tokens)[currentTokenPosition + 1]) == 1) {
            return create_syntax_report(NULL, 1, _NONE_);
        }
    }

    SyntaxReport functionCallReport = is_function_call(tokens, currentTokenPosition, _PARAM_FUNCTION_CALL_);

    if (functionCallReport.errorType == _NONE_
        && (int)is_end_indicator(&(*tokens)[currentTokenPosition + functionCallReport.tokensToSkip]) == 1) {
        return create_syntax_report(NULL, functionCallReport.tokensToSkip, _NONE_);
    }

    SyntaxReport simpleTermReport = is_simple_term(tokens, currentTokenPosition, 0);

    if (simpleTermReport.errorType == _NONE_
        && (int)is_end_indicator(&(*tokens)[currentTokenPosition + simpleTermReport.tokensToSkip]) == 1) {
        return create_syntax_report(NULL, simpleTermReport.tokensToSkip, _NONE_);
    }

    return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_TERM_);
}

/*
Purpose: Check if a given array of TOKENS at a specific position is a simple term or not
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Token array;
        size_t startPosition => Position from where to start checking
*/
SyntaxReport is_simple_term(TOKEN **tokens, size_t startPosition, int inFunctionCall) {
    int openBrackets = 0;
    int hasToBeArithmeticOperator = 0;
    size_t jumpTokensForward = 0;

    while ((*tokens)[startPosition + jumpTokensForward].type != __EOF__) {
        TOKEN currentToken = (*tokens)[startPosition + jumpTokensForward];

        if (currentToken.type == _OP_RIGHT_BRACKET_) {
            openBrackets++;
            jumpTokensForward++;
            continue;
        } else if (currentToken.type == _OP_LEFT_BRACKET_) {
            if (inFunctionCall == 1 && openBrackets <= 0) {
                break;
            }

            openBrackets--;
            jumpTokensForward++;
            continue;
        } else if ((int)is_end_indicator(&currentToken) == 1) {
            break;
        }

        switch (hasToBeArithmeticOperator) {
        case 0: {
            SyntaxReport functionCallReport = is_function_call(tokens, startPosition + jumpTokensForward, _PARAM_FUNCTION_CALL_);

            if (functionCallReport.errorType == _NONE_) {
                jumpTokensForward += functionCallReport.tokensToSkip - 1;
                hasToBeArithmeticOperator = 1;
                continue;
            }

            if (is_identifier(&currentToken).errorType == _NONE_
                || is_pointer_pointing_to_value(&currentToken).errorType == _NONE_
                || is_numeral_identifier(&currentToken).errorType == _NONE_) {
                jumpTokensForward++;
                hasToBeArithmeticOperator = 1;
                continue;
            } else {
                return create_syntax_report(&currentToken, 0, _NOT_A_SIMPLE_TERM_);
            }
        }
        case 1:
            if ((int)is_arithmetic_operator(currentToken.value[0]) == 0) {
                return create_syntax_report(&currentToken, 0, _NOT_A_SIMPLE_TERM_);
            } else {
                jumpTokensForward++;
                hasToBeArithmeticOperator = 0;
            }
            break;
        }
    }

    if (openBrackets != 0 || jumpTokensForward == 0) {
        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_SIMPLE_TERM_);
    }

    return create_syntax_report(NULL, jumpTokensForward, _NONE_);
}

/*
Purpose: Check if a given TOKEN matches an "end of statement" indicator ("=", ";", "]", "}", "?", ")", ",")
Return Type: int => 1 = is end indicator; 0 = is not an end indicator
Params: const TOKEN *token -> Token to be checked
*/
int is_end_indicator(const TOKEN *token) {
    if (token->type == __EOF__) {
        return 0;
    }
    
    char endIndicators[][2] = {"=", ";", "]", "}", ")", "?", ","};

    for (int i = 0; i < (sizeof(endIndicators) / sizeof(endIndicators[0])); i++) {
        if (strcmp(token->value, endIndicators[i]) == 0) {
            return 1;
        }
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

    int skipTokensFromRunnable = (int)is_runnable(tokens, currentTokenPosition + 1, 1);
    
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
        || (*tokens)[startPosition + 3].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_CATCH_);
    }

    int skipTokensFromRunnable = (int)is_runnable(tokens, startPosition + 5, 1);
    
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
        int skipTokens = is_function_call(tokens, index + 1, _PARAM_FUNCTION_).tokensToSkip;
        index++;

        if (skipTokens == 0) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_);
        }

        int runnableTokensToSkip = (int)is_runnable(tokens, index + skipTokens, 1);

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
        ParameterUse parameterUsage => Determines, if the function call is in a function or called seperately
*/
SyntaxReport is_function_call(TOKEN **tokens, size_t currentTokenPosition, ParameterUse parameterUsage) {
    int workedDownParameters = 0;
    size_t checkedTokens = 0;
    
    for (int i = currentTokenPosition; (*tokens)[i].type != __EOF__; i++) {
        TOKEN currentToken = (*tokens)[i];

        if (currentToken.type == __EOF__) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
        }
        
        if (i == currentTokenPosition && is_identifier(&currentToken).errorType != _NONE_) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
        } else if (i == currentTokenPosition + 1 && currentToken.type != _OP_RIGHT_BRACKET_) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
        } else if (i > currentTokenPosition + 1 && workedDownParameters == 0) {
            SyntaxReport paramReport = is_parameter(tokens, i, parameterUsage, _OP_LEFT_BRACKET_);

            if (paramReport.errorType != _NONE_) {
                return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
            }
            
            checkedTokens += paramReport.tokensToSkip;
            i += paramReport.tokensToSkip - 1;
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
        ParameterUse usage => For what purpose is the parameter;
        TOKENTYPES crucialType => Which type determines the end of a parameter
*/
SyntaxReport is_parameter(TOKEN **tokens, size_t currentTokenPos, ParameterUse usage, TOKENTYPES crucialType) {
    size_t i = currentTokenPos;
    int isCurrentlyComma = 0;

    while ((*tokens)[i].type != crucialType && (*tokens)[i].type != __EOF__) {
        switch (isCurrentlyComma) {
        case 0:
            switch (usage) {
            case _PARAM_WITH_STATEMENT_:
            case _PARAM_FUNCTION_CALL_: {
                SyntaxReport termReport = is_simple_term(tokens, i, 1);

                if (termReport.errorType == _NONE_) {
                    i += termReport.tokensToSkip;
                    isCurrentlyComma = 1;
                    continue;
                } else if (is_atom(&(*tokens)[i]).errorType == _NONE_
                    && (*tokens)[i + 1].type != _OP_EQUALS_
                    && (*tokens)[i + 1].type != _OP_RIGHT_BRACKET_) {
                    break;
                } else if (is_pointer_pointing_to_value(&(*tokens)[i]).errorType == _NONE_) {
                    break;
                } else if (is_reference(&(*tokens)[i]).errorType == _NONE_) {
                    break;
                } else {
                    SyntaxReport funcCallReport = is_function_call(tokens, i, _PARAM_FUNCTION_CALL_);
                    
                    if (funcCallReport.errorType == _NONE_) {
                        i += funcCallReport.tokensToSkip - 2;
                        break;
                    } else {
                        return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_PARAMETER_);
                    }
                }
            }
            case _PARAM_CLASS_:
            case _PARAM_FUNCTION_:
                if (is_pointer(&(*tokens)[i]).errorType == _NONE_) {
                    break;
                } else if (is_function_parameter_initializer(tokens, i).errorType == _NONE_) {
                    i += 2;
                    break;
                } else {
                    return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_PARAMETER_);
                }

            case _PARAM_VARIABLE_:
                if (is_pointer(&(*tokens)[i]).errorType == _NONE_) {
                    break;
                } else if ((*tokens)[i + 1].type == _OP_EQUALS_) {
                    break;
                } else {
                    return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_PARAMETER_);
                }
            }

            isCurrentlyComma = 1;
            break;
        case 1:
            //Prevents that the user ends the parameter with a comma instead of IDENTFIER / ATOM
            if ((*tokens)[i].type != _OP_COMMA_
                || ((*tokens)[i].type == _OP_COMMA_ && (*tokens)[i + 1].type == _OP_LEFT_BRACKET_)) {
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
Purpose: Check if a token array is accordingly layed out like the a function parameter initializer
Return Type: SyntaxReport => ErrorType = _NONE_ on success, else _NOT_A_FUNCTION_PARAMETER_INITIALIZER_
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_function_parameter_initializer(TOKEN **tokens, size_t startPos) {
    if (is_identifier(&(*tokens)[startPos]).errorType == _NONE_
        && (*tokens)[startPos + 1].type == _OP_EQUALS_
        && (is_identifier(&(*tokens)[startPos + 2]).errorType == _NONE_
        || is_numeral_identifier(&(*tokens)[startPos + 2]).errorType == _NONE_)) {
        return create_syntax_report(NULL, 2, _NONE_);
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FUNCTION_PARAMETER_INITIALIZER_);
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to the POINTER_TO_VALUE rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN *token => Token to be check after POINTER_POINTING_ON_VALUE rule
*/
SyntaxReport is_pointer_pointing_to_value(TOKEN *token) {
    if (token->type == _POINTER_) {
        int counter = 0;

        for (int i = 0; i < token->size; i++) {
            if (token->value[i] == '*') {
                counter++;
            }

            if (counter > 1) {
                return create_syntax_report(token, 0, _NOT_A_POINTER_POINTING_ON_VALUE);
            }
        }

        return create_syntax_report(NULL, 1, _NONE_);
    }
    
    return create_syntax_report(token, 0, _NOT_A_POINTER_POINTING_ON_VALUE);
}

/*
Purpose: Check if the given TOKEN is a POINTER or not
Return Type: SyntaxReport => Contains errors, tokensToSkip (How many pointers point onto the pointer),
            token itself when error
Params: TOKEN *token => Token to be checked after POINTER rule
        [Layout: "*pointer"]
*/
SyntaxReport is_pointer(TOKEN *token) {
    return token->type == _POINTER_ ?
    create_syntax_report(NULL, 1, _NONE_) :
    create_syntax_report(token, 0, _NOT_A_POINTER_);
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to the REFERENCE rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN *token => Token to be checked after REFERENCE rule
        [Layout: "&reference" or "&(*pointerReference)"]
*/
SyntaxReport is_reference(TOKEN *token) {
    //Layout here: &<ATOM>
    if (token->type == _REFERENCE_ || token->type == _REFERENCE_ON_POINTER_) {
        return create_syntax_report(NULL, 1, _NONE_);
    }

    return create_syntax_report(token, 0, _NOT_A_REFERENCE_);
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
    } else if (is_identifier(token).errorType == _NONE_) {
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
Return Type: SyntaxReport => Reports an error if necessary, returns _NONE_ if everything is ok
Params: TOKEN *token => Token to be checked
*/
SyntaxReport is_numeral_identifier(TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }
    
    int points = 0;

    for (int i = 0; i < token->size; i++) {
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
        } else if (currentChar == '\0') {
            break;
        }

        return create_syntax_report(token, 0, _NOT_A_FLOAT_);
    }

    return create_syntax_report(NULL, 1, _NONE_);
}

/*
Purpose: Check whether a given value is written according to the IDENTIFIER rule
Return Type: SyntaxReport => Contains how many tokens to skip and if the token has an error, it gets delivered with the report
Params: TOKEN *token => Token to be checked
*/
SyntaxReport is_identifier(TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    if ((int)is_keyword(token->value) == 1) {
        return create_syntax_report(token, 0, _NOT_AN_IDENTIFIER_);
    }

    for (size_t i = 0; i < token->size; i++) {
        char currentCharacter = (*token).value[i];

        if ((int)is_letter(currentCharacter) == 1) {
            continue;
        } else if ((int)is_number(currentCharacter) == 1 && i > 0) {
            continue;
        } else if ((int)is_underscore(currentCharacter) == 1) {
            continue;
        } else if (currentCharacter == '\0') {
            break;
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

    if (errorType != _NONE_) {
        throw_error(tokenToAssign, errorType);
    }

    return report;
}

void throw_error(TOKEN *token, SyntaxErrorType error) {
    switch (error) {
    case _NOT_AN_IDENTIFIER_:
    break;
    case _NOT_A_FLOAT_:
    break;
    case _NOT_AN_ATOM_:
    break;
    case _NOT_A_REFERENCE_:
    break;
    case _NOT_A_POINTER_:
    break;
    case _NOT_A_PARAMETER_:
    break;
    case _NOT_A_POINTER_POINTING_ON_VALUE:
    break;
    case _NOT_A_FUNCTION_CALL_:
    break;
    case _NOT_A_FUNCTION_:
    break;
    case _NOT_A_BREAK_:
    break;
    case _NOT_AN_ENUMERATOR_:
    break;
    case _NOT_AN_ENUM_:
    break;
    case _NOT_AN_INCLUDE_:
    break;
    case _NOT_A_CATCH_:
    break;
    case _NOT_A_TRY_:
    break;
    case _NOT_A_SIMPLE_TERM_:
    break;
    case _NOT_A_TERM_:
    break;
    case _NOT_AN_ASSIGNMENT_:
    break;
    case _NOT_A_CLASS_:
    break;
    default:
    break;
    }
}