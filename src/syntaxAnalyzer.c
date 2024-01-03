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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../headers/Token.h"
#include "../headers/errors.h"

#define true 1
#define false 0

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
    {"is", _KW_IS_},              {"try", _KW_TRY_},         {"catch", _KW_CATCH_},
    {"continue", _KW_CONTINUE_},  {"const", _KW_CONST_},     {"include", _KW_INCLUDE_},
    {"and", _KW_AND_},            {"or", _KW_OR_},           {"global", _KW_GLOBAL_},
    {"secure", _KW_SECURE_},      {"private", _KW_PRIVATE_}, {"export", _KW_EXPORT_},
    {"for", _KW_FOR_},            {"this", _KW_THIS_},       {"this", _KW_THIS_}
};

typedef struct SyntaxReport {
    TOKEN *token;
    int tokensToSkip;
    int errorOccured;
    char *expectedToken;
} SyntaxReport;

enum ParameterType {
    _PARAM_FUNCTION_CALL_, _PARAM_FUNCTION_, _PARAM_CLASS_
};

SyntaxReport is_runnable(TOKEN **tokens, size_t startPos, int withBlock);
SyntaxReport is_non_keyword_based_runnable(TOKEN **tokens, size_t startPos);
int predict_expression(TOKEN **tokens, size_t startPos);
SyntaxReport is_keyword_based_runnable(TOKEN **tokens, size_t startPos);
SyntaxReport is_class_object_access(TOKEN **tokens, size_t startPos, int independentCall);
SyntaxReport is_return_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_continue_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_break_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_for_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_expression(TOKEN **tokens, size_t startPos, int inRunnable);
SyntaxReport is_if(TOKEN **tokens, size_t startPos);
SyntaxReport is_if_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_else_if_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_else_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_do_statment(TOKEN **tokens, size_t startPos);
SyntaxReport is_while_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_while_condition(TOKEN **tokens, size_t startPos);
SyntaxReport is_class_instance(TOKEN **tokens, size_t startPos);
SyntaxReport is_check_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_is_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_variable(TOKEN **tokens, size_t startPos);
SyntaxReport is_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport is_conditional_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport is_chained_condition(TOKEN **tokens, size_t startPos, int inParam);
SyntaxReport is_condition(TOKEN **tokens, size_t startPos, int inParam);
int predict_is_conditional_variable_type(TOKEN **tokens, size_t startPos);
SyntaxReport is_multiple_variable_definition(TOKEN **tokens, size_t startPos);
SyntaxReport is_multiple_variable_definition_identifier(TOKEN **tokens, size_t startPos);
SyntaxReport is_array_variable(TOKEN **tokens, size_t startPos);
SyntaxReport is_array_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport is_array_element(TOKEN **tokens, size_t startPos);
SyntaxReport is_class_constructor(TOKEN **tokens, size_t startPos);
SyntaxReport is_class(TOKEN **tokens, size_t startPos);
SyntaxReport is_with_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_try_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_catch_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_export(TOKEN **tokens, size_t startPos);
SyntaxReport is_include(TOKEN **tokens, size_t startPos);
SyntaxReport is_enum(TOKEN **tokens, size_t startPos);
SyntaxReport is_enumerator(TOKEN **tokens, size_t startPos);
SyntaxReport is_function(TOKEN **tokens, size_t startPos);
SyntaxReport is_function_call(TOKEN **tokens, size_t startPos, int inFunction);
SyntaxReport is_parameter(TOKEN **tokens, size_t startPos, enum ParameterType type);
SyntaxReport is_simple_term(TOKEN **tokens, size_t startPos, int inParameter);
int predict_class_object_access(TOKEN **tokens, size_t startPos);
SyntaxReport is_identifier(TOKEN **tokens, size_t startPos);
SyntaxReport is_array_identifier(TOKEN **tokens, size_t startPos);
int is_root_identifier(TOKEN *token);
SyntaxReport is_numeral_identifier(TOKEN *token);

int is_end_indicator(const TOKEN *token);
int is_string(TOKEN *token);
int is_reference(TOKEN *token);
int is_pointer(const TOKEN *token);
int is_keyword(char *value);
int is_letter(const char character);
int is_number(const char character);
int is_rational_operator(const char *sequence);
int is_arithmetic_operator(const TOKEN *token);
int is_assignment_operator(const char *sequence);
int is_underscore(const char character);
int is_bool(const char *sequence);
int is_modifier(const char *sequence);
int is_logic_operator(const char *sequence);

SyntaxReport create_syntax_report(TOKEN *token, int tokensToSkip, int errorOccured, char *expextedToken);
void throw_error(TOKEN *errorToken, char *expectedToken);

size_t maxTokenLength = 0;
char **sourceCode = NULL;
size_t sourceLength = 0;
int FILE_CONTAINS_ERRORS = 0;

int CheckInput(TOKEN **tokens, size_t tokenArrayLength, char **source, size_t sourceSize) {
    maxTokenLength = tokenArrayLength;
    sourceCode = source;
    sourceLength = sourceSize;

    if (tokens == NULL || tokenArrayLength < 1) {
        (void)PARSER_TOKEN_TRANSMISSION_EXCEPTION();
        return -1;
    }

    clock_t start, end;

    if (PARSER_DISPLAY_USED_TIME == true) {
        start = clock();
    }

    if (PARSER_DEBUG_MODE == true) {
        (void)printf("\n\n\n>>>>>>>>>>>>>>>>>>>>    SYNTAX ANALYZER    <<<<<<<<<<<<<<<<<<<<\n\n");
    }

    printf("is_runnable: %i\n", is_runnable(tokens, 0, false).tokensToSkip);
    
    if (PARSER_DEBUG_MODE == true) {
        (void)printf("\n>>>>>    Tokens successfully analyzed    <<<<<\n");
    }

    if (PARSER_DISPLAY_USED_TIME == true) {
        end = clock();
        (void)printf("\nCPU time used for SYNTAX ANALYSIS: %f seconds\n", ((double) (end - start)) / CLOCKS_PER_SEC);  // NOLINT
    }

    return 1;
}

/*
Purpose: Checks if the current TOKEN array matches the RUNNABLE rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int independentCall => Flag if the object access was called by term or indepentently
*/
SyntaxReport is_runnable(TOKEN **tokens, size_t startPos, int withBlock) {
    int jumper = 0;
    
    if (withBlock == true) {
        if ((*tokens)[startPos].type != _OP_RIGHT_BRACE_) {
            return create_syntax_report(&(*tokens)[startPos], 0, true, "{");
        } else {
            jumper++;
        }
    }

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        if ((*tokens)[startPos + jumper].type == _OP_LEFT_BRACE_
            && withBlock == true) {
            break;
        }

        if (((*tokens)[startPos + jumper].type == _KW_IS_
            || (*tokens)[startPos + jumper].type == _OP_LEFT_BRACE_)
            && withBlock == 2) {
            break;
        }
        
        SyntaxReport isKWBasedRunnable = is_keyword_based_runnable(tokens, startPos + jumper);

        if (isKWBasedRunnable.errorOccured == true) {
            (void)throw_error(isKWBasedRunnable.token, isKWBasedRunnable.expectedToken);
            return create_syntax_report(isKWBasedRunnable.token, 0, true, isKWBasedRunnable.expectedToken);
        } else if (isKWBasedRunnable.errorOccured == false
            && isKWBasedRunnable.tokensToSkip > 0) {
            jumper += isKWBasedRunnable.tokensToSkip;
            continue;
        }
        
        SyntaxReport isNKWBasedRunnable = is_non_keyword_based_runnable(tokens, startPos + jumper);

        if (isNKWBasedRunnable.errorOccured == true) {
            (void)throw_error(isNKWBasedRunnable.token, isNKWBasedRunnable.expectedToken);
            return create_syntax_report(isNKWBasedRunnable.token, 0, true, isNKWBasedRunnable.expectedToken);
        } else if (isNKWBasedRunnable.errorOccured == false
            && isNKWBasedRunnable.tokensToSkip > 0) {
            jumper += isNKWBasedRunnable.tokensToSkip;
        } else {
            return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<ERROR>");
        }
    }

    if (withBlock == true) {
        if  ((*tokens)[startPos + jumper].type != _OP_LEFT_BRACE_) {
            return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "}");
        } else {
            jumper++;
        }
    }

    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Predicts what the token array wants to achieve, then tries it
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int independentCall => Flag if the object access was called by term or indepentently
*/
SyntaxReport is_non_keyword_based_runnable(TOKEN **tokens, size_t startPos) {
    if ((int)predict_expression(tokens, startPos) == true) {
        return is_expression(tokens, startPos, true);
    } else if ((int)predict_class_object_access(tokens, startPos) == true) {
        return is_class_object_access(tokens, startPos, true);
    } else {
        SyntaxReport isIdentifier = is_identifier(tokens, startPos);

        if (isIdentifier.errorOccured == true) {
            return create_syntax_report(isIdentifier.token, 0, true, isIdentifier.expectedToken);
        }

        if ((*tokens)[startPos + isIdentifier.tokensToSkip - 1].type != _OP_LEFT_BRACKET_) {
            return create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip - 1], 0, true, ")");
        }
        
        if ((*tokens)[startPos + isIdentifier.tokensToSkip].type != _OP_SEMICOLON_) {
            return create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip], 0, true, ";");
        }

        return create_syntax_report(NULL, isIdentifier.tokensToSkip + 1, false, NULL);
    }
}

/*
Purpose: Tries to predict if the current token array is an expression or not
Return Type: int => true = follwing array is expression; false following array is not an expression
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
*/
int predict_expression(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int facedSemicolon = false;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if ((int)is_assignment_operator(currentToken->value) == true
            || currentToken->type == _OP_EQUALS_
            || currentToken->type == _OP_ADD_ONE_
            || currentToken->type == _OP_SUBTRACT_ONE_) {
            if (facedSemicolon == true) {
                return false;
            } else {
                return true;
            }
        }

        if (currentToken->type == _OP_SEMICOLON_) {
            facedSemicolon = true;
        }

        jumper++;
    }

    return false;
}

/*
Purpose: Executes a function, that matches with the keyword
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int independentCall => Flag if the object access was called by term or indepentently
*/
SyntaxReport is_keyword_based_runnable(TOKEN **tokens, size_t startPos) {
    switch ((*tokens)[startPos].type) {
    case _KW_GLOBAL_:
    case _KW_SECURE_:
    case _KW_PRIVATE_:
        if ((*tokens)[startPos + 1].type == _KW_VAR_
            || (*tokens)[startPos + 1].type == _KW_CONST_) {
            return is_variable(tokens, startPos);
        } else if ((*tokens)[startPos + 1].type == _KW_FUNCTION_) {
            return is_function(tokens, startPos);
        } else if ((*tokens)[startPos + 1].type == _KW_CLASS_) {
            return is_class(tokens, startPos);
        } else {
            return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "var\" or \"const\" or \"function");
        }

        break;
    case _KW_VAR_:
    case _KW_CONST_:
        return is_variable(tokens, startPos);
    case _KW_FUNCTION_:
        return is_function(tokens, startPos);
    case _KW_CLASS_:
        return is_class(tokens, startPos);
    case _KW_IF_:
        return is_if(tokens, startPos);
    case _KW_WHILE_:
        return is_while_statement(tokens, startPos);
    case _KW_DO_:
        return is_do_statment(tokens, startPos);
    case _KW_FOR_:
        return is_for_statement(tokens, startPos);
    case _KW_TRY_:
        return is_try_statement(tokens, startPos);
    case _KW_CHECK_:
        return is_check_statement(tokens, startPos);
    case _KW_INCLUDE_:
        return is_include(tokens, startPos);
    case _KW_EXPORT_:
        return is_export(tokens, startPos);
    case _KW_ENUM_:
        return is_enum(tokens, startPos);
    case _KW_THIS_:
        if ((*tokens)[startPos + 1].type == _OP_COLON_) {
            return is_class_constructor(tokens, startPos);
        }

        break;
    case _KW_BREAK_:
        return is_break_statement(tokens, startPos);
    case _KW_RETURN_:
        return is_return_statement(tokens, startPos);
    case _KW_CONTINUE_:
        return is_continue_statement(tokens, startPos);
    default:
        break;
    }

    return create_syntax_report(NULL, 0, false, "N/A");
}

/*
Purpose: Checks if the current TOKEN array matches the CLASS_OBJECT_ACCESS rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int independentCall => Flag if the object access was called by term or indepentently
*/
SyntaxReport is_class_object_access(TOKEN **tokens, size_t startPos, int independentCall) {
    SyntaxReport leftIdentifier = is_identifier(tokens, startPos);

    if (leftIdentifier.errorOccured == true) {
        return create_syntax_report(leftIdentifier.token, 0, true, leftIdentifier.expectedToken);
    }

    if ((*tokens)[startPos + leftIdentifier.tokensToSkip].type != _OP_CLASS_ACCESSOR_) {
        return create_syntax_report(&(*tokens)[startPos + leftIdentifier.tokensToSkip], 0, true, "->");
    }

    SyntaxReport rightIdentifier = is_identifier(tokens, startPos + leftIdentifier.tokensToSkip + 1);

    if (rightIdentifier.errorOccured == true) {
        return create_syntax_report(rightIdentifier.token, 0, true, rightIdentifier.expectedToken);
    }

    if (independentCall == true) {
        if ((*tokens)[startPos + leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 1].type != _OP_SEMICOLON_) {
            return create_syntax_report(&(*tokens)[startPos + leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 1], 0, true, ";");
        }

        return create_syntax_report(NULL, leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 2, false, NULL);
    }

    return create_syntax_report(NULL, leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the RETURN rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_return_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_RETURN_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "return");
    }

    SyntaxReport isSimpleTerm = is_simple_term(tokens, startPos + 1, false);

    if (isSimpleTerm.errorOccured == true) {
        return create_syntax_report(isSimpleTerm.token, 0, true, isSimpleTerm.expectedToken);
    }

    if ((*tokens)[startPos + isSimpleTerm.tokensToSkip + 1].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + isSimpleTerm.tokensToSkip + 1], 0, true, ";");
    }

    return create_syntax_report(NULL, isSimpleTerm.tokensToSkip + 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the CONTINUE rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_continue_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_CONTINUE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "continue");
    }

    if ((*tokens)[startPos + 1].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, ";");
    }

    return create_syntax_report(NULL, 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the BREAK rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_break_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_BREAK_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "break");
    }

    if ((*tokens)[startPos + 1].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, ";");
    }

    return create_syntax_report(NULL, 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the FOR rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_for_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_FOR_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "for");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }
    
    SyntaxReport isVar = is_variable(tokens, startPos + 2);

    if (isVar.errorOccured == true) {
        return create_syntax_report(isVar.token, 0, true, isVar.expectedToken);
    }

    SyntaxReport isChainedCond = is_chained_condition(tokens, startPos + isVar.tokensToSkip + 2, true);

    if (isChainedCond.errorOccured == true) {
        return create_syntax_report(isChainedCond.token, 0, true, isChainedCond.expectedToken);
    }
    
    if ((*tokens)[startPos + isVar.tokensToSkip + isChainedCond.tokensToSkip + 2].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + isVar.tokensToSkip + isChainedCond.tokensToSkip + 2], 0, true, ";");
    }

    int totalSkip = isVar.tokensToSkip + isChainedCond.tokensToSkip + 3;
    SyntaxReport isExpression = is_expression(tokens, startPos + totalSkip, false);

    if (isExpression.errorOccured == true) {
        return create_syntax_report(isExpression.token, 0, true, isExpression.expectedToken);
    }

    totalSkip += isExpression.tokensToSkip;

    if ((*tokens)[startPos + totalSkip].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + totalSkip], 0, true, ")");
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + totalSkip + 1, true);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return create_syntax_report(NULL, totalSkip + isRunnable.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches an expression
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_expression(TOKEN **tokens, size_t startPos, int inRunnable) {
    SyntaxReport isIdentifier = is_identifier(tokens, startPos);

    if (isIdentifier.errorOccured == true) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, isIdentifier.expectedToken);
    }

    TOKEN *crucialToken = &(*tokens)[startPos + isIdentifier.tokensToSkip];
    int skip = 0;

    if (crucialToken->type == _OP_ADD_ONE_
        || crucialToken->type == _OP_SUBTRACT_ONE_) {
        skip = 1;
    } else if ((int)is_assignment_operator(crucialToken->value) == true
        || crucialToken->type == _OP_EQUALS_) {
        SyntaxReport isSimpleTerm = is_simple_term(tokens, startPos + isIdentifier.tokensToSkip + 1, true);

        if (isSimpleTerm.errorOccured == true) {
            return create_syntax_report(isSimpleTerm.token, 0, true, isSimpleTerm.expectedToken);
        }

        skip = isSimpleTerm.tokensToSkip + 1;
    } else {
        return create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip], 0, true, "++\" or \"--\" or \"-=\" or \"+=\" or \"*=\" or \"/=\" or \"=");
    }

    if (inRunnable == true) {
        if ((*tokens)[startPos + isIdentifier.tokensToSkip + skip].type == _OP_SEMICOLON_) {
            return create_syntax_report(NULL, isIdentifier.tokensToSkip + skip + 1, false, NULL);
        }

        return create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip + skip], 0, true, ";");
    }
    
    return create_syntax_report(NULL, isIdentifier.tokensToSkip + skip, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the IF rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_if(TOKEN **tokens, size_t startPos) {
    SyntaxReport isIfStatement = is_if_statement(tokens, startPos);

    if (isIfStatement.errorOccured == true) {
        return create_syntax_report(isIfStatement.token, 0, true, isIfStatement.expectedToken);
    }

    TOKEN *crucialToken = &(*tokens)[startPos + isIfStatement.tokensToSkip];
    int skip = 0;

    if (crucialToken->type == _KW_ELSE_) {
        crucialToken = &(*tokens)[startPos + isIfStatement.tokensToSkip + 1];
        
        if (crucialToken->type == _KW_IF_) {
            SyntaxReport isElseIf = is_else_if_statement(tokens, startPos + isIfStatement.tokensToSkip);

            if (isElseIf.errorOccured == true) {
                return create_syntax_report(isElseIf.token, 0, true, isElseIf.expectedToken);
            }

            skip = isElseIf.tokensToSkip;
        }

        SyntaxReport isElse;

        if ((*tokens)[startPos + skip + isIfStatement.tokensToSkip].type != _KW_ELSE_) {
            isElse = is_else_statement(tokens, startPos + isIfStatement.tokensToSkip);
            skip--;
        } else {
            isElse = is_else_statement(tokens, startPos + skip + isIfStatement.tokensToSkip);
        }

        if (isElse.errorOccured == true) {
            return create_syntax_report(isElse.token, 0, true, isElse.expectedToken);
        }

        skip += isElse.tokensToSkip;
    }
    
    return create_syntax_report(NULL, skip + isIfStatement.tokensToSkip, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the IF_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_if_statement(TOKEN **tokens, size_t startPos) { 
    if ((*tokens)[startPos].type != _KW_IF_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "if");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    SyntaxReport isChainedCond = is_chained_condition(tokens, startPos + 2, true);

    if (isChainedCond.errorOccured == true) {
        return create_syntax_report(isChainedCond.token, 0, true, isChainedCond.expectedToken);
    }

    if ((*tokens)[startPos + isChainedCond.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + isChainedCond.tokensToSkip + 2], 0, true, ")");
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + isChainedCond.tokensToSkip + 3, true);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return create_syntax_report(NULL, isRunnable.tokensToSkip + isChainedCond.tokensToSkip + 3, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the ELSE_IF_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_else_if_statement(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int hasToBeElse = true;
    int exitOnPurpose = false;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (hasToBeElse == false && currentToken->type != _KW_IF_) {
            exitOnPurpose = true;
            jumper--;
            break;
        } else if (hasToBeElse == true && currentToken->type != _KW_ELSE_) {
            break;
        }

        switch (hasToBeElse) {
        case false: {
            SyntaxReport isIfStatement = is_if_statement(tokens, startPos + jumper);

            if (isIfStatement.errorOccured == true) {
                return create_syntax_report(isIfStatement.token, 0, true, isIfStatement.expectedToken);
            }

            jumper += isIfStatement.tokensToSkip;
            hasToBeElse = true;
            break;
        }
        case true:
            if (currentToken->type != _KW_ELSE_) {
                return create_syntax_report(currentToken, 0, true, "else");
            }

            jumper++;
            hasToBeElse = false;
            break;
        }
    }

    if (hasToBeElse == false && exitOnPurpose != true) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<ELSE_IF>");
    }

    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the ELSE_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_else_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_ELSE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "else");
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + 1, true);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return create_syntax_report(NULL, isRunnable.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the DO_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_do_statment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_DO_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "do");
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + 1, true);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    SyntaxReport isWhileCond = is_while_condition(tokens, startPos + isRunnable.tokensToSkip + 1);

    if (isWhileCond.errorOccured == true) {
        return create_syntax_report(isWhileCond.token, 0, true, isWhileCond.expectedToken);
    }

    return create_syntax_report(NULL, isRunnable.tokensToSkip + isWhileCond.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the WHILE_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_while_statement(TOKEN **tokens, size_t startPos) {
    SyntaxReport isWhileCond = is_while_condition(tokens, startPos);

    if (isWhileCond.errorOccured == true) {
        return create_syntax_report(isWhileCond.token, 0, true, isWhileCond.expectedToken);
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + isWhileCond.tokensToSkip, true);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return create_syntax_report(NULL, isWhileCond.tokensToSkip + isRunnable.tokensToSkip, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the WHILE_CONDITION rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_while_condition(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_WHILE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "while");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    SyntaxReport isChainedCondition = is_chained_condition(tokens, startPos + 2, true);

    if (isChainedCondition.errorOccured == true) {
        return create_syntax_report(isChainedCondition.token, 0, true, isChainedCondition.expectedToken);
    }

    if ((*tokens)[startPos + isChainedCondition.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + isChainedCondition.tokensToSkip + 2], 0, true, ")");
    }

    return create_syntax_report(NULL, isChainedCondition.tokensToSkip + 3, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the CLASS_INSTANCE rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_class_instance(TOKEN **tokens, size_t startPos) {
    if ((int)is_root_identifier(&(*tokens)[startPos]) == false) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "<IDENTIFIER>");
    }

    if ((int)is_root_identifier(&(*tokens)[startPos + 1]) == false) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "<IDENTIFIER>");
    }

    int skip = 2;

    if ((*tokens)[startPos + 2].type == _OP_RIGHT_EDGE_BRACKET_) {
        SyntaxReport isArrayElement = is_array_element(tokens, startPos + 2);

        if (isArrayElement.errorOccured == true) {
            return create_syntax_report(isArrayElement.token, 0, true, isArrayElement.expectedToken);
        } else {
            skip += isArrayElement.tokensToSkip;
        }
    }

    if ((*tokens)[startPos + skip].type != _OP_EQUALS_) {
        return create_syntax_report(&(*tokens)[startPos + 2], 0, true, "=");
    }
    
    if ((*tokens)[startPos + skip + 1].type != _KW_NEW_) {
        return create_syntax_report(&(*tokens)[startPos + 3], 0, true, "new");
    }

    if ((int)is_root_identifier(&(*tokens)[startPos + skip + 2]) == false) {
        return create_syntax_report(&(*tokens)[startPos + 4], 0, true, "<IDENTIFIER>");
    }
    
    if ((*tokens)[startPos + skip + 3].type == _OP_RIGHT_EDGE_BRACKET_) {
        SyntaxReport isArrayElement = is_array_element(tokens, startPos + skip + 3);

        if (isArrayElement.errorOccured == true) {
            return create_syntax_report(isArrayElement.token, 0, true, isArrayElement.expectedToken);
        } else {
            skip += isArrayElement.tokensToSkip + 3;
        }
    } else if ((*tokens)[startPos + skip + 3].type == _OP_RIGHT_BRACKET_) {
        SyntaxReport isFunctionCall = is_function_call(tokens, startPos + skip + 2, _PARAM_FUNCTION_CALL_);

        if (isFunctionCall.errorOccured == true) {
            return create_syntax_report(isFunctionCall.token, 0, true, isFunctionCall.expectedToken);
        } else {
            skip += isFunctionCall.tokensToSkip + 2;
        }
    } else {
        return create_syntax_report(&(*tokens)[startPos + skip + 3], 0, true, "<ARRAY>\" or \"<FUNCTION_CALL>");
    }
    
    if ((*tokens)[startPos + skip].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + skip], 0, true, ";");
    }
    
    return create_syntax_report(NULL, skip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the CHECK_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_check_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_CHECK_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "check");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    SyntaxReport isIdentifier = is_identifier(tokens, startPos + 2);

    if (isIdentifier.errorOccured == true) {
        return create_syntax_report(isIdentifier.token, 0, true, isIdentifier.expectedToken);
    }

    if ((*tokens)[startPos + isIdentifier.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip + 2], 0, true, ")");
    }

    if ((*tokens)[startPos + isIdentifier.tokensToSkip + 3].type != _OP_RIGHT_BRACE_) {
        return create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip + 3], 0, true, "{");
    }

    int jumper = isIdentifier.tokensToSkip + 4;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        if ((*tokens)[startPos + jumper].type == _OP_LEFT_BRACE_) {
            jumper++;
            break;
        }
        
        SyntaxReport isIsStatement = is_is_statement(tokens, startPos + jumper);

        if (isIsStatement.errorOccured == true) {
            return create_syntax_report(isIsStatement.token, 0, true, isIsStatement.expectedToken);
        }

        jumper += isIsStatement.tokensToSkip;
    }
   
    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the IS_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_is_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_IS_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "is");
    }

    TOKEN *crucialToken = &(*tokens)[startPos + 1];
    int skip = 0;

    if (crucialToken->value[0] == '\"') {
        if ((int)is_string(crucialToken) == true) {
            skip = 1;
        } else {
            return create_syntax_report(crucialToken, 0, true, "<STRING>");
        }
    } else if ((int)is_digit(crucialToken->value[0]) == true) {
        SyntaxReport isNumeralIdentifier = is_numeral_identifier(crucialToken);

        if (isNumeralIdentifier.errorOccured == true) {
            return create_syntax_report(crucialToken, 0, true, "<NUMBER>");
        }

        skip = isNumeralIdentifier.tokensToSkip;
    } else if ((int)is_letter(crucialToken->value[0]) == true) {
        //THIS OPTION IS ONLY FOR CONSTANTS
        if ((int)is_root_identifier(crucialToken) == false) {
            return create_syntax_report(crucialToken, 0, true, "<CONSTANT>");
        }

        skip = 1;
    }

    if ((*tokens)[startPos + skip + 1].type != _OP_COLON_) {
        return create_syntax_report(&(*tokens)[startPos + skip + 1], 0, true, ":");
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + skip + 2, 2);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return create_syntax_report(NULL, isRunnable.tokensToSkip + skip + 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the VARIABLE rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_variable(TOKEN **tokens, size_t startPos) {
    int modifier = 0;

    switch ((*tokens)[startPos].type) {
    case _KW_GLOBAL_:
    case _KW_SECURE_:
    case _KW_PRIVATE_:
        modifier = 1;
        break;
    default:
        break;
    }

    if ((*tokens)[startPos + modifier].type == _KW_VAR_) {
        if ((int)is_root_identifier(&(*tokens)[startPos + modifier + 1]) == false) {
            return create_syntax_report(&(*tokens)[startPos + modifier + 1], 0, true, "<IDENTIFIER>");
        }

        TOKEN *crucialToken = &(*tokens)[startPos + 2];
        int skip = 0;

        if (crucialToken->type == _OP_RIGHT_EDGE_BRACKET_) {
            SyntaxReport isArrayVar = is_array_variable(tokens, startPos + modifier  + 2);

            if (isArrayVar.errorOccured == true) {
                return create_syntax_report(isArrayVar.token, 0, true, isArrayVar.expectedToken);
            } else {
                skip = isArrayVar.tokensToSkip;
            }
        } else if (crucialToken->type == _OP_SEMICOLON_) {
            return create_syntax_report(NULL, 3 + modifier, false, NULL);
        } else if (crucialToken->type == _OP_COMMA_) {
            SyntaxReport isMultipleVarDef = is_multiple_variable_definition(tokens, startPos + modifier + 1);

            if (isMultipleVarDef.errorOccured == true) {
                return create_syntax_report(isMultipleVarDef.token, 0, true, isMultipleVarDef.expectedToken);
            } else {
                //-1, because the identifier get's checked again in the is_multiple... function
                skip = isMultipleVarDef.tokensToSkip - 1;
            }
        } else if (crucialToken->type == _OP_EQUALS_) {
            if ((int)predict_is_conditional_variable_type(tokens, startPos + modifier + 2) == true) {
                SyntaxReport isCondAssignment = is_conditional_assignment(tokens, startPos + modifier + 2);

                if (isCondAssignment.errorOccured == true) {
                    return create_syntax_report(isCondAssignment.token, 0, true, isCondAssignment.expectedToken);
                } else {
                    skip = isCondAssignment.tokensToSkip;
                }
            } else {
                SyntaxReport isAssignment = is_assignment(tokens, startPos + modifier + 2);

                if (isAssignment.errorOccured == true) {
                    return create_syntax_report(isAssignment.token, 0, true, isAssignment.expectedToken);
                } else {
                    skip = isAssignment.tokensToSkip;
                }
            }
        } else {
            return create_syntax_report(crucialToken, 0, true, "[\" or \";\" or \",\" or \"=\" or \"<IDENTIFIER>");
        }

        return create_syntax_report(NULL, skip + modifier + 2, false, NULL);
    } else if ((*tokens)[startPos + modifier].type == _KW_CONST_) {
        if ((int)is_root_identifier(&(*tokens)[startPos + modifier + 1]) == true) {
            SyntaxReport isAssignment = is_assignment(tokens, startPos + modifier + 2);

            if (isAssignment.errorOccured == true) {
                return create_syntax_report(isAssignment.token, 0, true, isAssignment.expectedToken);
            } else {
                return create_syntax_report(NULL, isAssignment.tokensToSkip + modifier + 2, false, NULL);
            }
        } else {
            return create_syntax_report(&(*tokens)[startPos + modifier + 1], 0, true, "<IDENTIFIER>");
        }
    }
    
    return create_syntax_report(&(*tokens)[startPos], 0, true, "const\" or \"var");
}

/*
Purpose: Checks if the current TOKEN array is an ASSIGNMENT
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_assignment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_EQUALS_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "=");
    }

    SyntaxReport isTerm = is_simple_term(tokens, startPos + 1, false);

    if (isTerm.errorOccured == true) {
        return create_syntax_report(isTerm.token, 0, true, isTerm.expectedToken);
    }

    if ((*tokens)[startPos + isTerm.tokensToSkip + 1].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + isTerm.tokensToSkip + 1], 0, true, ";");
    }

    return create_syntax_report(NULL, isTerm.tokensToSkip + 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is a CONDITIONAL_ASSIGNMENT
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_conditional_assignment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_EQUALS_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "=");
    }

    SyntaxReport isChainedCondition = is_chained_condition(tokens, startPos + 1, false);

    if (isChainedCondition.errorOccured == true) {
        return create_syntax_report(isChainedCondition.token, 0, true, isChainedCondition.expectedToken);
    }

    if ((*tokens)[startPos + isChainedCondition.tokensToSkip + 1].type != _OP_QUESTION_MARK_) {
        return create_syntax_report(&(*tokens)[startPos + isChainedCondition.tokensToSkip + 1], 0, true, "?");
    }

    SyntaxReport leftTerm = is_simple_term(tokens, startPos + isChainedCondition.tokensToSkip + 2, false);

    if (leftTerm.errorOccured == true) {
        return create_syntax_report(leftTerm.token, 0, true, leftTerm.expectedToken);
    }

    int totalSkip = isChainedCondition.tokensToSkip + leftTerm.tokensToSkip;

    if ((*tokens)[startPos + totalSkip + 2].type != _OP_COLON_) {
        return create_syntax_report(&(*tokens)[startPos + totalSkip + 2], 0, true, ":");
    }

    SyntaxReport rightTerm = is_simple_term(tokens, startPos + totalSkip + 3, false);

    if (rightTerm.errorOccured == true) {
        return create_syntax_report(rightTerm.token, 0, true, rightTerm.expectedToken);
    }

    totalSkip += rightTerm.tokensToSkip;

    if ((*tokens)[startPos + totalSkip + 3].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + totalSkip + 3], 0, true, ";");
    }

    return create_syntax_report(NULL, totalSkip + 4, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is a chained condition (condition with "and" and "or")
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int inParam => Flag if chainedCondition is in parameter
*/
SyntaxReport is_chained_condition(TOKEN **tokens, size_t startPos, int inParam) {
    int jumper = 0;
    int openBrackets = 1;
    int hasToBeLogicOperator = false;
    
    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];
        
        if (currentToken->type == _OP_RIGHT_BRACKET_) {
            openBrackets++;
            jumper++;
            continue;
        } else if (currentToken->type == _OP_LEFT_BRACKET_) {
            openBrackets--;

            if (openBrackets == 0) {
                break;
            }

            jumper++;
            continue;
        } else if ((int)is_end_indicator(currentToken) == true
            && currentToken->type != _KW_AND_
            && currentToken->type != _KW_OR_) {
            break;
        }

        switch (hasToBeLogicOperator) {
        case false:
            hasToBeLogicOperator = true;

            SyntaxReport isCondition = is_condition(tokens, startPos + jumper, inParam);

            if (isCondition.errorOccured == true) {
                return create_syntax_report(isCondition.token, 0, true, isCondition.expectedToken);
            }

            jumper += isCondition.tokensToSkip;
            break;
        case true:
            if (currentToken->type != _KW_AND_
                && currentToken->type != _KW_OR_) {
                return create_syntax_report(currentToken, 0, true, "and\" or \"or");
            }

            hasToBeLogicOperator = false;
            jumper++;
            break;
        }
    }

    if (jumper == 0) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<CHAINED_CONDITION>");
    } else if (hasToBeLogicOperator == false) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    }

    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is a condition
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int inParam => Flag if the condition is delivered as parameter
*/
SyntaxReport is_condition(TOKEN **tokens, size_t startPos, int inParam) {
    if ((int)is_bool((*tokens)[startPos].value) == false) {
        SyntaxReport leftTerm = is_simple_term(tokens, startPos, false);

        if (leftTerm.errorOccured == true) {
            return create_syntax_report(leftTerm.token, 0, true, leftTerm.expectedToken);
        }

        char *sequence = (*tokens)[startPos + leftTerm.tokensToSkip].value;

        if ((int)is_rational_operator(sequence) == false) {
            return create_syntax_report(&(*tokens)[startPos + leftTerm.tokensToSkip], 0, true, "==\" or \"<=\" or \">=\" or \"!=\" or \"<\" or \">");
        }

        SyntaxReport rightTerm = is_simple_term(tokens, startPos + leftTerm.tokensToSkip + 1, inParam);

        if (rightTerm.errorOccured == true) {
            return create_syntax_report(rightTerm.token, 0, true, rightTerm.expectedToken);
        }

        return create_syntax_report(NULL, leftTerm.tokensToSkip + rightTerm.tokensToSkip + 1, false, NULL);
    }

    return create_syntax_report(NULL, 1, false, NULL);
}

/*
Purpose: Predict whether the next var option is a conditional assignment or not
Return Type: int => true = is conditional assignment; false = not a conditional assignment
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
int predict_is_conditional_variable_type(TOKEN **tokens, size_t startPos) {
    int facedSemicolon = false;

    for (int i = startPos; i < maxTokenLength; i++) {
        if ((*tokens)[i].type == _OP_QUESTION_MARK_) {
            if (facedSemicolon == false) {
                return true;
            } else {
                return false;
            }
        }

        if ((*tokens)[i].type == _OP_SEMICOLON_) {
            facedSemicolon = true;
        }
    }

    return false;
}

/*
Purpose: Checks if the current TOKEN array is an array variable
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_multiple_variable_definition(TOKEN **tokens, size_t startPos) {
    SyntaxReport isMultiVarIdentifer = is_multiple_variable_definition_identifier(tokens, startPos);

    if (isMultiVarIdentifer.errorOccured == true) {
        return create_syntax_report(isMultiVarIdentifer.token, 0, true, isMultiVarIdentifer.expectedToken);
    }

    if ((*tokens)[startPos + isMultiVarIdentifer.tokensToSkip].type == _OP_EQUALS_) {
        SyntaxReport isSimpleTerm = is_simple_term(tokens, startPos + isMultiVarIdentifer.tokensToSkip + 1, false);
        int totalSkip = startPos + isSimpleTerm.tokensToSkip + isMultiVarIdentifer.tokensToSkip;

        if (isSimpleTerm.errorOccured == true) {
            return create_syntax_report(&(*tokens)[totalSkip], 0, true, isSimpleTerm.expectedToken);
        }

        if ((*tokens)[totalSkip + 1].type != _OP_SEMICOLON_) {
            return create_syntax_report(&(*tokens)[totalSkip + 1], 0, true, ";");
        }

        return create_syntax_report(NULL, totalSkip + 1, false, NULL);
    } else {
        if ((*tokens)[startPos + isMultiVarIdentifer.tokensToSkip].type != _OP_SEMICOLON_) {
            return create_syntax_report(&(*tokens)[startPos + isMultiVarIdentifer.tokensToSkip], 0, true, ";");
        }
    }

    return create_syntax_report(NULL, isMultiVarIdentifer.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is a multiple variable identifier
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
Layout: <IDENTIIFER>, <IDENTIFIER>, [...]
*/
SyntaxReport is_multiple_variable_definition_identifier(TOKEN **tokens, size_t startPos) {
    int hasToBeComma = false;
    int jumper = 0;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];
        
        if ((int)is_end_indicator(currentToken) == true
            && currentToken->type != _OP_COMMA_) {
            break;
        }

        switch (hasToBeComma) {
        case false:
            hasToBeComma = true;

            if ((int)is_root_identifier(currentToken) == false) {
                return create_syntax_report(currentToken, 0, true, "<IDENTIFIER>");
            }

            jumper++;
            break;
        case true:
            if (currentToken->type != _OP_COMMA_) {
                return create_syntax_report(currentToken, 0, true, ",");
            }

            jumper++;
            hasToBeComma = false;
            break;
        }
    }

    if (jumper <= 1) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<MULTIPLE_DEFINITION>");
    } else if (hasToBeComma == false) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    }

    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is an array variable
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_array_variable(TOKEN **tokens, size_t startPos) {
    SyntaxReport isArrayElement = is_array_element(tokens, startPos);

    if (isArrayElement.errorOccured == true) {
        return create_syntax_report(isArrayElement.token, 0, true, isArrayElement.expectedToken);
    }
    
    if ((*tokens)[startPos + isArrayElement.tokensToSkip].type == _OP_EQUALS_) {
        SyntaxReport isArrayAssignment = is_array_assignment(tokens, startPos + isArrayElement.tokensToSkip);
        
        if (isArrayAssignment.errorOccured == true) {
            return create_syntax_report(isArrayAssignment.token, 0, true, isArrayAssignment.expectedToken);
        }
        
        if ((*tokens)[startPos + isArrayElement.tokensToSkip + isArrayAssignment.tokensToSkip].type != _OP_SEMICOLON_) {
            return create_syntax_report(&(*tokens)[startPos + isArrayElement.tokensToSkip + isArrayAssignment.tokensToSkip], 0, true, ";");
        }
        
        return create_syntax_report(NULL, isArrayElement.tokensToSkip + isArrayAssignment.tokensToSkip + 1, false, NULL);
    }

    if ((*tokens)[startPos + isArrayElement.tokensToSkip].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + isArrayElement.tokensToSkip], 0, true, ";");
    }

    return create_syntax_report(NULL, isArrayElement.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is an array assignment
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_array_assignment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_EQUALS_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "=");
    }

    int jumper = 1;
    int openBraces = 0;
    int hasToBeComma = false;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken->type == _OP_RIGHT_BRACE_) {
            openBraces++;
            jumper++;
            continue;
        } else if (currentToken->type == _OP_LEFT_BRACE_) {
            openBraces--;
            jumper++;
            continue;
        } else if ((int)is_end_indicator(currentToken) == true
            && currentToken->type != _OP_COMMA_) {
            break;
        }

        switch (hasToBeComma) {
        case false: {
            SyntaxReport isSimpleTerm = is_simple_term(tokens, startPos + jumper, false);

            if (isSimpleTerm.errorOccured == true
                || isSimpleTerm.tokensToSkip == 0) {
                return create_syntax_report(isSimpleTerm.token, 0, true, isSimpleTerm.expectedToken);
            }
            
            jumper += isSimpleTerm.tokensToSkip;
            hasToBeComma = true;
            break;
        }
        case true:
            if (currentToken->type != _OP_COMMA_) {
                return create_syntax_report(currentToken, 0, true, ",");
            }

            jumper++;
            hasToBeComma = false;
           break;
        }
    }

    if (hasToBeComma == false) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    } else if (openBraces != 0) {
        if (openBraces > 0) {
            return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "}");
        } else {
            return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "{");
        }
    } else if (jumper <= 1) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<ARRAY_ASSIGNMENT>");
    }

    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the ARRAY_ELEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_array_element(TOKEN **tokens, size_t startPos) {
    int hasToBeClosingEdgeBracket = false;
    int jumper = 0;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if ((int)is_end_indicator(currentToken)
            && currentToken->type != _OP_LEFT_EDGE_BRACKET_) {
            break;
        }

        switch (hasToBeClosingEdgeBracket) {
        case false:
            hasToBeClosingEdgeBracket = true;

            if (currentToken->type != _OP_RIGHT_EDGE_BRACKET_) {
                return create_syntax_report(currentToken, 0, true, "[");
            }
            
            if ((*tokens)[startPos + jumper + 1].type != _OP_LEFT_EDGE_BRACKET_) {
                SyntaxReport isSimpleTerm = is_simple_term(tokens, startPos + jumper + 1, false);

                if (isSimpleTerm.errorOccured == true) {
                    return create_syntax_report(isSimpleTerm.token, 0, true, isSimpleTerm.expectedToken);
                }

                jumper += isSimpleTerm.tokensToSkip + 1;
            } else {
                jumper++;
            }

            break;
        case true:
            if (currentToken->type != _OP_LEFT_EDGE_BRACKET_) {
                return create_syntax_report(currentToken, 0, true, "]");
            }

            jumper++;
            hasToBeClosingEdgeBracket = false;
            break;
        }
    }

    if (hasToBeClosingEdgeBracket == true) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "]");
    } else if (jumper == 0) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<ARRAY_ELEMENT>");
    }

    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the CLASS_CONSTRUCTOR rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_class_constructor(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_THIS_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "this");
    }

    if ((*tokens)[startPos + 1].type != _OP_COLON_
        && (*tokens)[startPos + 2].type != _OP_COLON_) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "::");
    }

    if ((*tokens)[startPos + 3].type != _KW_CONSTRUCTOR_) {
        return create_syntax_report(&(*tokens)[startPos + 3], 0, true, "constructor");
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + 4, 1);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return create_syntax_report(NULL, isRunnable.tokensToSkip + 4, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the CLASS rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_class(TOKEN **tokens, size_t startPos) {
    int modifier = 0;

    switch ((*tokens)[startPos].type) {
    case _KW_GLOBAL_:
    case _KW_SECURE_:
    case _KW_PRIVATE_:
        modifier = 1;
        break;
    default:
        break;
    }

    if ((*tokens)[startPos + modifier].type != _KW_CLASS_) {
        return create_syntax_report(&(*tokens)[startPos + modifier], 0, true, "class");
    }

    SyntaxReport isFunctionCall = is_function_call(tokens, startPos + modifier + 1, true);

    if (isFunctionCall.errorOccured == true) {
        return create_syntax_report(isFunctionCall.token, 0, true, isFunctionCall.expectedToken);
    }

    int additionalWithSkip = 0;

    if ((*tokens)[startPos + isFunctionCall.tokensToSkip + modifier + 1].type == _KW_WITH_) {
        SyntaxReport isWith = is_with_statement(tokens, startPos + isFunctionCall.tokensToSkip + modifier + 1);

        if (isWith.errorOccured == true) {
            return create_syntax_report(isWith.token, 0, true, isWith.expectedToken);
        } else {
            additionalWithSkip = isWith.tokensToSkip;
        }
    }

    int totalSkip = isFunctionCall.tokensToSkip + modifier + additionalWithSkip + 1;

    if ((*tokens)[startPos + totalSkip].type != _OP_CLASS_CREATOR_) {
        return create_syntax_report(&(*tokens)[startPos + totalSkip], 0, true, "->");
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + totalSkip + 1, true);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return create_syntax_report(NULL, totalSkip + isRunnable.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the WITH rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_with_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_WITH_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "with");
    }
    
    SyntaxReport isParameter = is_parameter(tokens, startPos + 1, _PARAM_CLASS_);

    if (isParameter.errorOccured == true) {
        return create_syntax_report(isParameter.token, 0, true, isParameter.expectedToken);
    }
    
    return create_syntax_report(NULL, isParameter.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Check if a TOKEN array is written accordingly to the TRY rule
Return Type: SyntaxReport => ErrorOccured = true, if error appears, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_try_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_TRY_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "try");
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + 1, true);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    SyntaxReport isCatchStatement = is_catch_statement(tokens, startPos + isRunnable.tokensToSkip + 1);

    if (isCatchStatement.errorOccured == true) {
        return create_syntax_report(isCatchStatement.token, startPos + isRunnable.tokensToSkip + 1, true, isCatchStatement.expectedToken);
    }

    return create_syntax_report(NULL, isRunnable.tokensToSkip + isCatchStatement.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Check if a TOKEN array is written accordingly to the CATCH rule
Return Type: SyntaxReport => ErrorOccured = true, if error appears, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_catch_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_CATCH_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "catch");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    if ((int)is_root_identifier(&(*tokens)[startPos + 2]) == false) {
        return create_syntax_report(&(*tokens)[startPos + 2], 0, true, "<IDENTIFIER>");
    }

    if ((int)is_root_identifier(&(*tokens)[startPos + 3]) == false) {
        return create_syntax_report(&(*tokens)[startPos + 3], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 4].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + 4], 0, true, ")");
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + 5, true);

    return create_syntax_report(NULL, isRunnable.tokensToSkip + 5, false, isRunnable.expectedToken);
}

/*
Purpose: Check if a TOKEN array is written accordingly to the EXPORT rule
Return Type: SyntaxReport => ErrorOccured = true, if error appears, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_export(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_EXPORT_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "export");
    }

    if ((int)is_string(&(*tokens)[startPos + 1]) == false) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "<STRING>");
    }

    if ((*tokens)[startPos + 2].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + 2], 0, true, ";");
    }

    return create_syntax_report(NULL, 3, false, NULL);
}
/*
Purpose: Check if a TOKEN array is written accordingly to the INCLUDE rule
Return Type: SyntaxReport => ErrorOccured = true, if error appears, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_include(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_INCLUDE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "include");
    }

    if ((int)is_string(&(*tokens)[startPos + 1]) == false) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "<STRING>");
    }

    if ((*tokens)[startPos + 2].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos + 2], 0, true, ";");
    }

    return create_syntax_report(NULL, 3, false, NULL);
}

/*
Purpose: Checks if a given TOKEN array is an ENUM or not
Return Type: SyntaxReport => ErrorOccured = true, when something is not written
                            accordingly to the rule, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_enum(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_ENUM_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "enum");
    }

    if ((int)is_root_identifier(&(*tokens)[startPos + 1]) == false) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 2].type != _OP_RIGHT_BRACE_) {
        return create_syntax_report(&(*tokens)[startPos + 2], 0, true, "{");
    }

    SyntaxReport isEnumerator = is_enumerator(tokens, startPos + 3);
    
    if (isEnumerator.errorOccured == true) {
        return create_syntax_report(isEnumerator.token, 0, true, isEnumerator.expectedToken);
    }

    if ((*tokens)[startPos + isEnumerator.tokensToSkip + 3].type != _OP_LEFT_BRACE_) {
        return create_syntax_report(&(*tokens)[startPos + isEnumerator.tokensToSkip + 3], 0, true, "}");
    }
    
    return create_syntax_report(NULL, isEnumerator.tokensToSkip + 4, false, NULL);
}

/*
Purpose: Checks if a given TOKEN array is an enumerator or not
Return Type: SyntaxReport => ErrorOccured = true, when something is not written
                            accordingly to the rule, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_enumerator(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int hasToBeComma = false;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__
        && (*tokens)[startPos + jumper].type != _OP_LEFT_BRACE_) {
        switch (hasToBeComma) {
        case false:
            hasToBeComma = true;

            if ((int)is_letter((*tokens)[startPos + jumper].value[0]) == true) {
                int isRootIdentifier = (int)is_root_identifier(&(*tokens)[startPos + jumper]);
                
                if ((*tokens)[startPos + jumper + 1].type == _OP_COLON_) {
                    SyntaxReport isNumeralIdentifier = is_numeral_identifier(&(*tokens)[startPos + jumper + 2]);

                    if (isNumeralIdentifier.errorOccured == false) {
                        jumper += 3;
                    } else {
                        return create_syntax_report(&(*tokens)[startPos + jumper + 2], 0, true, "<NUMBER>");
                    }
                } else {
                    jumper += isRootIdentifier;
                }
            } else {
                return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
            }

            break;
        case true:
            if ((*tokens)[startPos + jumper].type != _OP_COMMA_) {
                return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, ",");
            }

            jumper++;
            hasToBeComma = false;
            break;
        }
    }

    if (hasToBeComma == false) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    }

    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Check if a TOKEN array is matching the FUNCTION rule
Return Type: SyntaxReport => ErrorOccured = false if everything works fine, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_function(TOKEN **tokens, size_t startPos) {
    int skip = 0;

    switch ((*tokens)[startPos].type) {
    case _KW_GLOBAL_:
    case _KW_SECURE_:
    case _KW_PRIVATE_:
        skip = 1;
        break;
    default:
        break;
    }

    if ((*tokens)[startPos + skip].type != _KW_FUNCTION_) {
        return create_syntax_report(&(*tokens)[startPos + skip], 0, true, "function");
    }

    SyntaxReport isFunctionCall = is_function_call(tokens, startPos + skip + 1, true);

    if (isFunctionCall.errorOccured == true) {
        return create_syntax_report(isFunctionCall.token, 0, true, isFunctionCall.expectedToken);
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPos + skip + isFunctionCall.tokensToSkip + 1, true);

    if (isRunnable.errorOccured == true) {
        return create_syntax_report(&(*tokens)[startPos + isFunctionCall.tokensToSkip + isRunnable.tokensToSkip + skip + 1], 0, true, isRunnable.expectedToken);
    }

    return create_syntax_report(NULL, skip + isFunctionCall.tokensToSkip + isRunnable.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if a TOKEN array is written accordingly to the FUNCTION CALL rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking;
        int inFunctionCall => Flag if the functioncall is independent or from a function
*/
SyntaxReport is_function_call(TOKEN **tokens, size_t startPos, int inFunction) {
    int isRootIdentifier = (int)is_root_identifier(&(*tokens)[startPos]);

    if (isRootIdentifier == false) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }
    
    SyntaxReport isParameter;

    if (inFunction == true) {
        isParameter = is_parameter(tokens, startPos + 2, _PARAM_FUNCTION_);
    } else {
        isParameter = is_parameter(tokens, startPos + 2, _PARAM_FUNCTION_CALL_);
    }
    
    if (isParameter.errorOccured == true) {
        return create_syntax_report(isParameter.token, 0, true, isParameter.expectedToken);
    }

    if ((*tokens)[startPos + isParameter.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + isParameter.tokensToSkip + 2], 0, true, ")");
    }
    
    return create_syntax_report(NULL, isRootIdentifier + isParameter.tokensToSkip + 2, false, NULL);
}

/*
Purpose: Check if a Token array is a parameter or not
Return Type: SyntaxReport => true if it is a parameter, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        enum ParameterType type => Determines the type of the parameter
*/
SyntaxReport is_parameter(TOKEN **tokens, size_t startPos, enum ParameterType type) {
    int jumper = 0;
    int hasToBeComma = false;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken == NULL) {
            (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
        } else if ((int)is_end_indicator(currentToken) == true
            && currentToken->type != _OP_COMMA_) {
            break;
        }

        switch (hasToBeComma) {
        case false:
            hasToBeComma = true;

            switch (type) {
            case _PARAM_FUNCTION_CALL_:
                //Layout: *<POINTER>
                if (currentToken->value[0] == '*') {
                    if ((int)is_pointer(currentToken) == true) {
                        jumper++;
                    } else {
                        return create_syntax_report(currentToken, 0, true, "<POINTER>");
                    }
                //Layout: &<IDENTIFIER> or &(*<IDENTIFIER>)
                } else if (currentToken->value[0] == '&') {
                    if ((int)is_reference(currentToken) == true) {
                        jumper++;
                    } else {
                        return create_syntax_report(currentToken, 0, true, "<REFERENCE>");
                    }
                //Layout: <IDENTIFIER> or <NUMBER> or <SIMPLE_TERM>
                } else {
                    SyntaxReport isSimpleTerm = is_simple_term(tokens, startPos + jumper, 1);

                    if (isSimpleTerm.errorOccured == false) {
                        jumper += isSimpleTerm.tokensToSkip;
                    } else {
                        return create_syntax_report(currentToken, 0, true, isSimpleTerm.expectedToken);
                    }
                }

                break;
            case _PARAM_CLASS_:
            case _PARAM_FUNCTION_:
                if (currentToken->value[0] == '*') {
                    if ((int)is_pointer(currentToken) == true) {
                        jumper++;
                    } else {
                        return create_syntax_report(currentToken, 0, true, "<POINTER>");
                    }
                } else if ((int)is_letter(currentToken->value[0]) == true) {
                    if ((int)is_root_identifier(currentToken) == true) {
                        jumper++;
                    } else {
                        return create_syntax_report(currentToken, 0, true, "<IDENTIFIER>");
                    }
                }

                break;
            }

            break;
        case true:
            if (currentToken->type != _OP_COMMA_) {
                return create_syntax_report(currentToken, 0, true, ",");
            }

            jumper++;
            hasToBeComma = false;
            break;
        }
    }

    if (hasToBeComma == false
        && (*tokens)[startPos + jumper - 1].type != _OP_RIGHT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, ")");
    }

    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Check if a TOKEN array is written accordingly to the simple term rule
Return Type: SyntaxReport => Contains how many tokens to skip if TOKEN array match
                            the SIMPLE_TERM rule, else errorOccured = true
Params: TOKEN **tokens => TOKEN array to be checked;
        size_t startPos => Position from where to start checking;
        int inParameter => Determines if the simple term is called from a parameter
*/
SyntaxReport is_simple_term(TOKEN **tokens, size_t startPos, int inParameter) {
    int openBrackets = 0;
    int jumper = 0;
    int hasToBeArithmeticOperator = false;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken == NULL) {
            (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
        } else if (currentToken->type == _OP_RIGHT_BRACKET_) {
            openBrackets++;
            jumper++;
            continue;
        } else if (currentToken->type == _OP_LEFT_BRACKET_) {
            if (inParameter == true && openBrackets <= 0) {
                break;
            }

            openBrackets--;
            jumper++;
            continue;
        } else if ((int)is_end_indicator(currentToken) == true) {
            break;
        }

        switch (hasToBeArithmeticOperator) {
        case false:
            hasToBeArithmeticOperator = true;

            if (currentToken->value[0] == '\"') {
                if ((int)is_string(currentToken) == true) {
                    jumper++;
                    continue;
                } else {
                    return create_syntax_report(currentToken, 0, true, "<STRING>");
                }
            }

            SyntaxReport isIdentifier;

            if ((int)is_letter(currentToken->value[0]) == true) {
                if ((int)predict_class_object_access(tokens, startPos + jumper) == true) {
                    isIdentifier = is_class_object_access(tokens, startPos + jumper, false);
                } else if ((*tokens)[startPos + jumper + 1].type == _OP_RIGHT_BRACKET_) {
                    isIdentifier = is_function_call(tokens, startPos + jumper, false);
                } else if ((int)is_bool((*tokens)[startPos].value) == false) {
                    isIdentifier = is_identifier(tokens, startPos + jumper);
                } else {
                    jumper++;
                    continue;
                }
            } else if ((int)is_digit(currentToken->value[0]) == true) {
                isIdentifier = is_numeral_identifier(currentToken);
            }

            if (isIdentifier.errorOccured == false) {
                jumper += isIdentifier.tokensToSkip;
            } else {
                return create_syntax_report(currentToken, 0, true, isIdentifier.expectedToken);
            }

            break;
        case true:
            if ((int)is_arithmetic_operator(currentToken) != true) {
                return create_syntax_report(currentToken, 0, true, "+\" or \"-\" or \"*\" or \"/");
            }

            jumper++;
            hasToBeArithmeticOperator = false;
            break;
        }
    }

    if (hasToBeArithmeticOperator == false) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    } else if (openBrackets != 0) {
        if (openBrackets > 0) {
            return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, ")");
        } else {
            return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "(");
        }
    } else if (jumper == 0) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFER>\" or \"<FUNCTION_CALL>\" or \"<CLASS_OBJECT_ACCESS>");
    }

    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Check if the next tokens are an evidence for an object access out of a class
Return Type: int => true = is object access; false = not an object access
Params: TOKEN **tokens => Tokens to be checked for evidence;
        size_t startPos => Position from where to start checking
*/
int predict_class_object_access(TOKEN **tokens, size_t startPos) {
    int facedSemicolon = false;
    int jumper = 0;
    int openBrackets = 0;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken->type == _OP_CLASS_ACCESSOR_) {
            if (facedSemicolon == true || openBrackets != 0) {
                return false;
            } else {
                return true;
            }
        } else if (currentToken->type == _OP_SEMICOLON_) {
            facedSemicolon = true;
        } else if (currentToken->type == _OP_RIGHT_BRACKET_) {
            openBrackets++;
        } else if (currentToken->type == _OP_LEFT_BRACKET_) {
            openBrackets--;
        }

        jumper++;
    }

    return false;
}

/*
Purpose: Checks if a given TOKEN array is matching the possibilities of an IDENTIFIER
Return Type: SyntaxReport => Contains how many tokens to skip if TOKEN array match
                            the IDENTIFIER rule, else errorOccured = true
Params: TOKEN **tokens => TOKEN array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_identifier(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int hasToBeDot = false;

    while (startPos + jumper < maxTokenLength
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if ((int)is_end_indicator(currentToken) == true
            || (int)is_arithmetic_operator(currentToken) == true) {
            break;
        }

        switch (hasToBeDot) {
        case false: {
            hasToBeDot = true;
            int isRootIdentifier = is_root_identifier(currentToken);

            if (isRootIdentifier == true) {
                if ((*tokens)[startPos + jumper + 1].type == _OP_RIGHT_EDGE_BRACKET_) {
                    SyntaxReport isArrayIdentifier = is_array_identifier(tokens, startPos + jumper + 1);

                    if (isArrayIdentifier.errorOccured == false) {
                        jumper += isArrayIdentifier.tokensToSkip + isRootIdentifier;
                        continue;
                    } else {
                        return create_syntax_report(isArrayIdentifier.token, 0, true, isArrayIdentifier.expectedToken);
                    }
                } else if ((*tokens)[startPos + jumper + 1].type == _OP_RIGHT_BRACKET_) {
                    SyntaxReport isFunctionCall = is_function_call(tokens, startPos + jumper, false);

                    if (isFunctionCall.errorOccured == false) {
                        jumper += isFunctionCall.tokensToSkip;
                        continue;
                    } else {
                        return create_syntax_report(isFunctionCall.token, 0, true, isFunctionCall.expectedToken);
                    }
                }

                jumper += isRootIdentifier;
            }

            continue;
        }
        case true:
            if (currentToken->type != _OP_DOT_) {
                return create_syntax_report(currentToken, 0, true, ".");
            }

            jumper++;
            hasToBeDot = false;
            continue;
        }
    }

    if (hasToBeDot == false) {
        return create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    }
    
    return create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if a given TOKEN array is an array identifier
Return Type: SyntaxReport => Contains how many tokens to skip if TOKEN array match
                            the ARRAY_IDENTIFIER rule, else errorOccured = true
Params: TOKEN **tokens => TOKEN array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_array_identifier(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_RIGHT_EDGE_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos], 0, true, "[");
    }
    
    SyntaxReport isSimpleTerm = is_simple_term(tokens, startPos + 1, 0);

    if (isSimpleTerm.errorOccured == true) {
        return create_syntax_report(&(*tokens)[startPos + 1], 0, true, isSimpleTerm.expectedToken);
    }

    if ((*tokens)[startPos + isSimpleTerm.tokensToSkip + 1].type != _OP_LEFT_EDGE_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos + isSimpleTerm.tokensToSkip + 1], 0, true, "]");
    }

    return create_syntax_report(NULL, isSimpleTerm.tokensToSkip + 2, 0, NULL);
}

/*
Purpose: Checks if a token matches the basic IDENTIFIER made out of letters, digits, underscores
Return Type: int => true = is root identifier; false = not a root identifier
Params: TOKEN *token => Token to be checked
*/
int is_root_identifier(TOKEN *token) {
    if (token == NULL) {
        return false;
    }

    if ((int)is_keyword(token->value) == true
        && token->type != _KW_THIS_) {
        return false;
    }

    for (int i = 0; i < token->size; i++) {
        char currentCharacter = token->value[i];

        if (currentCharacter == '\0') {
            break;
        } else if ((int)is_letter(currentCharacter) == true) {
            continue;
        } else if ((int)is_underscore(currentCharacter) == true) {
            continue;
        } else if ((int)is_digit(currentCharacter) == true && i != 0) {
            continue;
        }

        return false;
    }

    return true;
}

/*
Purpose: Check if a given TOKEN is a number or float
Return Type: SyntaxReport => ErrorOccured = true on error, else ErrorOccured = false
Params: TOKEN *token => Token to be checked
*/
SyntaxReport is_numeral_identifier(TOKEN *token) {
    if (token == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    int dots = 0;

    for (int i = 0; i < token->size; i++) {
        char currentCharacter = token->value[i];

        if (currentCharacter == '\0') {
            break;
        } else if (currentCharacter == '.') {
            if (dots >= 1) {
                return create_syntax_report(token, 0, true, "<NUMBER>");
            }

            dots++;
            continue;
        } else if ((int)is_digit(currentCharacter) == true) {
            continue;
        }

        return create_syntax_report(token, 0, true, "<DIGIT>\" or \".");
    }

    return create_syntax_report(NULL, 1, false, NULL);
}

/*
Purpose: Check if a given TOKEN matches an "end of statement" indicator ("=", ";", "]", "}", "?", ")", "," ...)
Return Type: int => true = is end indicator; false = is not an end indicator
Params: const TOKEN *token -> Token to be checked
*/
const TOKENTYPES endIndicators[] = {
_OP_EQUALS_, _OP_SEMICOLON_, _OP_LEFT_EDGE_BRACKET_, _OP_SMALLER_CONDITION_,
_OP_GREATER_CONDITION_, _OP_SMALLER_OR_EQUAL_CONDITION_, _OP_GREATER_OR_EQUAL_CONDITION_,
_OP_NOT_EQUALS_CONDITION_, _OP_EQUALS_CONDITION_, _OP_COLON_, _KW_AND_, _KW_OR_,
_OP_MINUS_EQUALS_, _OP_PLUS_EQUALS_, _OP_MULTIPLY_EQUALS_, _OP_DIVIDE_EQUALS_,
_OP_ADD_ONE_, _OP_SUBTRACT_ONE_, _OP_LEFT_BRACKET_, _OP_COMMA_, _OP_CLASS_CREATOR_,
_OP_LEFT_BRACE_, _OP_QUESTION_MARK_, _OP_CLASS_ACCESSOR_};

int is_end_indicator(const TOKEN *token) {
    if (token->type == __EOF__) {
        return true;
    }

    for (int i = 0; i < (sizeof(endIndicators) / sizeof(endIndicators[0])); i++) {
        if (token->type == endIndicators[i]) {
            return true;
        }
    }

    return false;
}

/*
Purpose: Checks if a token is a string or not
Return Type: int => true = is string; false = not a string
Params: TOKEN *token => Token to be checked
*/
int is_string(TOKEN *token) {
    return (token->type == _STRING_ || token->type == _CHARACTER_ARRAY_) ?
        true : false;
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to the REFERENCE rule
Return int => true = is reference; false = not a reference
Params: TOKEN *token => Token to be checked
*/
int is_reference(TOKEN *token) {
    if (token->type == _REFERENCE_ || token->type == _REFERENCE_ON_POINTER_) {
        return true;
    }

    return false;
}

/*
Purpose: Check if the given TOKEN is a POINTER or not
Return Type: int => true = is pointer; false = not a pointer
Params: const TOKEN *token => Token to be checked
*/
int is_pointer(const TOKEN *token) {
    return token->type == _POINTER_ ? true : false;
}

/*
Purpose: Check if a given value is a keyword or not
Return Type: int => true = is keyword; false = not a keyword
Params: char *value => Value to be checked
*/
int is_keyword(char *value) {
    for (int i = 0; i < sizeof(KeywordLookupTable) / sizeof(KeywordLookupTable[0]); i++) {
        if (strcmp(value, KeywordLookupTable[i].kwName) == 0) {
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////
////////////////  BASE FUNCTIONS  ////////////////
//////////////////////////////////////////////////

/*
Purpose: Check whether a given character is a letter or not
Return Type: int => true = is letter; false = not a letter
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
            return true;
        default:
            return false;
    }
}

/*
Purpose: Check whether a given character is a number or not
Return Type: int => true = is a number; false = not a number
Params: const char character => Character to be checked
*/
int is_number(const char character) {
    switch (character) {
        case '0':   case '1':   case '2':   case '3':   case '4':   case '5':
        case '6':   case '7':   case '8':   case '9':
            return true;
        default:
            return false;
    }
}

/*
Purpose: Check whether a given sequence is a rational operator or not
Return Type: int => true = is a rational operator; false = not a rational operator
Params: const char *sequence => Sequence to be checked
*/
const char rationalOperators[][3] = {"==", "<=", ">=", "!=", "<", ">"};

int is_rational_operator(const char *sequence) {
    for (int i = 0; i < (sizeof(rationalOperators) / sizeof(rationalOperators[0])); i++) {
        if (strcmp(sequence, rationalOperators[i]) == 0) {
            return true;
        }
    }

    return false;
}

/*
Purpose: Check whether a given character is an arithmetic operator or not
Return Type: int => true = is an arithmetic operator; false = not an arithmetic operator
Params: const TOKEN *token => Token to be checked
*/
int is_arithmetic_operator(const TOKEN *token) {
    //Could be double operators like += or -= or *= ect.
    // 1 Operator takes size = 2; 2 take size = 3;
    if (token->size != 2) {
        return false;
    }

    switch (token->value[0]) {
    case '+':   case '-':   case '/':   case '*':   case '%':
        return true;
    default:
        return false;
    }
}

/*
Purpose: Check whether a given sequence is an assignment operator or not
Return Type: int => true = is an assignment operator; false = not an assignment operator
Params: const char *sequence => Sequence to be checked
*/
int is_assignment_operator(const char *sequence) {
    char assignmentOperator[][3] = {"+=", "-=", "*=", "/="};
    int lengthOfAssignmentOperators = (sizeof(assignmentOperator) / sizeof(assignmentOperator[0]));

    for (int i = 0; i < lengthOfAssignmentOperators; i++) {
        if (strcmp(sequence, assignmentOperator[i]) == 0) {
            return true;
        }
    }

    return false;
}

/*
Purpose: Check whether a given character is an underscore or not
Return Type: int => true = is an underscore; false = not an underscore
Params: const char character => Character to be checked
*/
int is_underscore(const char character) {
    return character == '_' ? true : false;
}

/*
Purpose: Check whether a given sequence is a bool or not
Return Type: int => true = is a bool; false = not a bool
Params: const char *sequence => Sequence to be checked
*/
int is_bool(const char *sequence) {
    return (strcmp(sequence, "true") == 0 || strcmp(sequence, "false") == 0) ? true : false;
}

/*
Purpose: Check whether a given sequence is a modifier or not
Return Type: int => true = is a modifier; false = not a modifier
Params: const char *sequence => Sequence to be checked
*/
int is_modifier(const char *sequence) {
    return (strcmp(sequence, "global") == 0
            || strcmp(sequence, "local") == 0
            || strcmp(sequence, "secure") == 0) ? true : false;
}

/*
Purpose: Check whether a given sequence is a logic operator or not
Return Type: int => true = is a logic operator; false = not a logic operator
Params: const char *sequence => Sequence to be checked
*/
int is_logic_operator(const char *sequence) {
    return (strcmp(sequence, "and") == 0
            || strcmp(sequence, "or") == 0
            || strcmp(sequence, "!") == 0) ? true : false;
}

/*
Purpose: Creates a SyntaxReport based on the given Params
Return Type: SyntaxReport => Containing all important information for an error
Params: TOKEN *token => Error token;
        int tokensToSkip => How many tokens should be skipped till the next step;
        int errorOccured => Was there an error or not;
        char *expectedToken => Token that was expected, when an error occures
*/
SyntaxReport create_syntax_report(TOKEN *token, int tokensToSkip, int errorOccured, char *expectedToken) {
    SyntaxReport report;
    report.token = token;
    report.tokensToSkip = tokensToSkip;
    report.errorOccured = errorOccured;
    report.expectedToken = expectedToken;

    if (errorOccured == true) {
        FILE_CONTAINS_ERRORS = 1;
    }

    return report;
}

void throw_error(TOKEN *errorToken, char *expectedToken) {
    if (sourceCode == NULL) {
        (void)printf("Source code pointer = NULL!");
        return;
    }

    (void)printf("An error occured on line %i.\n", (errorToken->line + 1));
    (void)printf("----------------------------------\n");

    int printPosition = errorToken->tokenStart;

    for (int i = errorToken->tokenStart; i > 0; i--) {
        if ((*sourceCode)[i] == '\n') {
            break;
        }

        printPosition = i - 1;
    }

    char buffer[32];
    int tokPos = ((errorToken->tokenStart + 1) - printPosition);
    int blankLength = (int)snprintf(buffer, 32, "%i : %i | ", (errorToken->line + 1), tokPos);

    (void)printf("%s", buffer);

    for (int i = printPosition; i < sourceLength; i++) {
        (void)printf("%c", (*sourceCode)[i]);
        
        if ((*sourceCode)[i + 1] == '\n' || i + 1 == sourceLength) {
            (void)printf("\n");
            break;
        }
    }

    for (int i = 0; i < blankLength; i++) {
        (void)printf(" ");
    }

    for (int i = printPosition; i < sourceLength; i++) {
        if (i < errorToken->tokenStart
            || i >= errorToken->tokenStart + (errorToken->size - 1)) {
            (void)printf(" ");
        } else {
            (void)printf("^");
        }
    }

    (void)printf("\n\nUnexpected token \"%s\", maybe replace with \"%s\".\n", errorToken->value, expectedToken);
    (void)printf("----------------------------------\n\n");
}