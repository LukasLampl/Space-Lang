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

SyntaxReport is_runnable(TOKEN **tokens, size_t startPos, int withBlock);
SyntaxReport is_class_object_access(TOKEN **tokens, size_t startPos);
SyntaxReport is_runnable_expression(TOKEN **tokens, size_t startPos);
SyntaxReport is_runnable_term(TOKEN **tokens, size_t startPos);
SyntaxReport is_assignable_instruction(TOKEN **tokens, size_t startPos);
SyntaxReport is_return_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_break_statement(TOKEN **tokens, size_t currentTokenPosition);
SyntaxReport is_for_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_if(TOKEN **tokens, size_t startPos);
SyntaxReport is_if_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_else_if_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_else_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_do_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_while_statement(TOKEN **tokens, size_t startPos);
SyntaxReport is_while_condition(TOKEN **tokens, size_t startPos);
SyntaxReport is_class_instance(TOKEN **tokens, size_t startPos);
SyntaxReport is_variable(TOKEN **tokens, size_t startPos);
SyntaxReport is_conditional_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport is_chained_condition(TOKEN **tokens, size_t startPos);
SyntaxReport is_condition(TOKEN **tokens, size_t startPos);
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
SyntaxReport is_term(TOKEN **tokens, size_t currentTokenPosition, int inFunctionCall);
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
SyntaxReport is_identifier(TOKEN **tokens, size_t tokenPos, int pointingIdentifier);
SyntaxReport is_pointer_identifier(TOKEN **tokens, size_t startPos);
int is_keyword(char *value);

int is_letter(const char character);
int is_number(const char character);
int is_rational_operator(const char *sequence);
int is_arithmetic_operator(TOKEN *token);
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

int Check_syntax(TOKEN **tokens, size_t tokenArrayLength) {
    tokenLength = tokenArrayLength;

    if (tokens == NULL || tokenArrayLength < 1) {
        printf("NO TOKENS");
        return -1;
    }

    clock_t start, end;

    if (PARSER_DISPLAY_USED_TIME == 1) {
        start = clock();
    }

    if (PARSER_DEBUG_MODE == 1) {
        (void)printf("\n\n\n>>>>>>>>>>>>>>>>>>>>    SYNTAX ANALYZER    <<<<<<<<<<<<<<<<<<<<\n\n");
    }

    for (int i = 0; i < 1; i++) {
        SyntaxReport isProgramValid = is_runnable(tokens, 0, 0);

        if (PARSER_DEBUG_MODE == 1) {
            if (isProgramValid.errorType != _NONE_) {
                printf("There's something wrong with the input!\n");
                return 0;
            } else {
                printf("Syntaxanalysis has been executed successfully! (Toks: %i)\n", isProgramValid.tokensToSkip);
            }
        }
    }

    if (PARSER_DEBUG_MODE == 1) {
        (void)printf("\n>>>>>    Tokens successfully analyzed    <<<<<\n");
    }

    if (PARSER_DISPLAY_USED_TIME == 1) {
        end = clock();
        printf("\nCPU time used for SYNTAX ANALYSIS: %f seconds\n", ((double) (end - start)) / CLOCKS_PER_SEC);
    }

    return 1;
}

//If error get's spotted enter the mode, so it can continue search for syntax errors
//without terminating
void enter_panic_mode(TOKEN **tokens, size_t currenTokenPosition) {

}

SyntaxReport is_runnable(TOKEN **tokens, size_t startPos, int withBlock) {
    int jumper = 0;

    if (withBlock == 1) {
        if ((*tokens)[startPos].type != _OP_RIGHT_BRACE_) {
            return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_RUNNABLE_);
        }
        
        jumper++;
    }

    while (startPos + jumper < tokenLength && (*tokens)[startPos + jumper].type != __EOF__) {
        if (withBlock == 1 && (*tokens)[startPos + jumper].type == _OP_LEFT_BRACE_) {
            break;
        } else if (withBlock == 2 &&
            ((*tokens)[startPos + jumper].type == _KW_IS_
            || (*tokens)[startPos + jumper].type == _OP_LEFT_BRACE_)) {
            break;
        }
        
        SyntaxReport reports[19];
        reports[0] = is_variable(tokens, startPos + jumper);
        reports[1] = is_runnable_expression(tokens, startPos + jumper);
        reports[2] = is_class_instance(tokens, startPos + jumper);
        reports[3] = is_function_call(tokens, startPos + jumper, _PARAM_FUNCTION_CALL_);
        reports[4] = is_function(tokens, startPos + jumper);
        reports[5] = is_if(tokens, startPos + jumper);
        reports[6] = is_while_statement(tokens, startPos + jumper);
        reports[7] = is_do_statement(tokens, startPos + jumper);
        reports[8] = is_try_statement(tokens, startPos + jumper);
        reports[9] = is_for_statement(tokens, startPos + jumper);
        reports[10] = is_check_statement(tokens, startPos + jumper);
        reports[11] = is_include(tokens, startPos + jumper);
        reports[12] = is_class(tokens, startPos + jumper);
        reports[13] = is_enumeration(tokens, startPos + jumper);
        reports[14] = is_export(tokens, startPos + jumper);
        reports[15] = is_return_statement(tokens, startPos + jumper);
        reports[16] = is_break_statement(tokens, startPos + jumper);
        reports[17] = is_class_object_access(tokens, startPos + jumper);
        reports[18] = is_assignable_instruction(tokens, startPos + jumper);

        int foundMatch = 0;
        int reportsArraySize = (sizeof(reports) / sizeof(reports[0]));
        
        for (int i = 0; i < reportsArraySize; i++) {
            if (reports[i].errorType == _NONE_) {
                jumper += reports[i].tokensToSkip;
                foundMatch = 1;
                i = reportsArraySize;
                continue;
            }
        }

        if (foundMatch == 0) {
            printf("-RUNNABLE ERROR:");
            printf(" %s%s\n", (*tokens)[startPos + jumper - 1].value, (*tokens)[startPos + jumper].value);
            jumper++;
            break;
        }
    }
    
    if (withBlock == 1) {
        if ((*tokens)[startPos + jumper].type != _OP_LEFT_BRACE_) {
            return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_RUNNABLE_);
        }

        jumper++;
    }
    
    return create_syntax_report(NULL, jumper, _NONE_);
}

SyntaxReport is_class_object_access(TOKEN **tokens, size_t startPos) {
    SyntaxReport isFunctionCall = is_function_call(tokens, startPos + 2, _PARAM_FUNCTION_CALL_);
    SyntaxReport isIdentifier = is_identifier(tokens, startPos, 0);

    //Layout: <IDENTIFIER> -> <FUNCTION_CALL>;
    if (isIdentifier.errorType == _NONE_
        && (*tokens)[startPos + 1].type == _OP_CLASS_ACCESSOR_
        && isFunctionCall.errorType == _NONE_
        && (*tokens)[startPos + isFunctionCall.tokensToSkip + 1].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, isFunctionCall.tokensToSkip + 2, _NONE_);
    }

    //Layout: <IDENTIFIER>.<IDENTIFIER>;
    if (isIdentifier.errorType == _NONE_
        && (*tokens)[startPos + 1].type == _OP_DOT_
        && (*tokens)[startPos + 2].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, 3, _NONE_);
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_CLASS_OBJECT_ACCESS_);
}

/*
Pupose: Checking if a given Token array is matching an expression in a runnable
Return Type: SyntaxReport => ErrorType = _NONE_ on success else _NOT_AN_EXPRESSION_
Params: TOKEN **tokens => Token array;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_runnable_expression(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].value == NULL) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    SyntaxReport expressionReport = is_expression(tokens, startPos);

    if (expressionReport.errorType == _NONE_
        && (*tokens)[startPos + expressionReport.tokensToSkip].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, expressionReport.tokensToSkip + 1, _NONE_);
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_EXPRESSION_);
}

/*
Pupose: Checking if a given Token array is matching an assignment in a runnable
Return Type: SyntaxReport => ErrorType = _NONE_ on success else _NOT_AN_ASSIGNABLE_INSTRUCTION_
Params: TOKEN **tokens => Token array;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_assignable_instruction(TOKEN **tokens, size_t startPos) {
    SyntaxReport isIdentifier = is_identifier(tokens, startPos, 0);

    if (isIdentifier.errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_ASSIGNABLE_INSTRUCTION_);
    }

    char *value = (*tokens)[startPos + isIdentifier.tokensToSkip].value;

    int isAssignmentOperator = is_assignment_operator(value);
    int isIncrementOperator = is_increment_operator(value);
    int isDecrementOperator = is_decrement_operator(value);

    if (isIncrementOperator > 0 || isDecrementOperator > 0) {
        int skip = isIncrementOperator + isDecrementOperator;
        return create_syntax_report(NULL, isIdentifier.tokensToSkip + skip + 1, _NONE_);
    } else if (isAssignmentOperator > 0) {
        SyntaxReport termReport = is_term(tokens, startPos + isIdentifier.tokensToSkip + isAssignmentOperator, 0);
        
        if ((*tokens)[startPos + isIdentifier.tokensToSkip + termReport.tokensToSkip + 1].type == _OP_SEMICOLON_) {
            return create_syntax_report(NULL, isIdentifier.tokensToSkip + termReport.tokensToSkip + 2, _NONE_);
        }
    }
    
    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_ASSIGNABLE_INSTRUCTION_);
}

/*
Purpose: Check if an TOKEN array is writte accordingly to the RETURN rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success, _NOT_A_RETURN_STATEMENT_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking the array
*/
SyntaxReport is_return_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_RETURN_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_RETURN_STATEMENT_);
    }

    SyntaxReport isTerm = is_term(tokens, startPos + 1, 0);
    SyntaxReport isFunctionCall = is_function_call(tokens, startPos + 1, _PARAM_FUNCTION_CALL_);
    SyntaxReport isConditionalAssignment = is_conditional_assignment(tokens, startPos + 1);
    SyntaxReport isIdentifier = is_identifier(tokens, startPos + 1, 0);

    if (isTerm.errorType == _NONE_
        && (*tokens)[startPos + isTerm.tokensToSkip + 1].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, isTerm.tokensToSkip + 2, _NONE_);
    }

    if (isFunctionCall.errorType == _NONE_
        && (*tokens)[startPos + isFunctionCall.tokensToSkip].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, isFunctionCall.tokensToSkip + 1, _NONE_);
    }

    if (isConditionalAssignment.errorType == _NONE_
        && (*tokens)[startPos + isConditionalAssignment.tokensToSkip].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, isConditionalAssignment.tokensToSkip + 1, _NONE_);
    }

    if (isIdentifier.errorType == _NONE_
        && (*tokens)[startPos + isIdentifier.tokensToSkip].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, isIdentifier.tokensToSkip, _NONE_);
    }

    if ((int)is_string(&(*tokens)[startPos + 1]) == 1
        && (*tokens)[startPos + 2].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, 3, _NONE_);
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_RETURN_STATEMENT_);
}

/*
Purpose: Check if an TOKEN array is writte accordingly to the FOR rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success, _NOT_A_FOR_STATEMENT_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking the array
*/
SyntaxReport is_for_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_FOR_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FOR_STATEMENT_);
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FOR_STATEMENT_);
    }

    SyntaxReport variableReport = is_variable(tokens, startPos + 2);

    if (variableReport.errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FOR_STATEMENT_);
    }

    if ((*tokens)[startPos + variableReport.tokensToSkip + 1].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FOR_STATEMENT_);
    }

    SyntaxReport conditionReport = is_chained_condition(tokens, startPos + variableReport.tokensToSkip + 2);

    if (conditionReport.errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FOR_STATEMENT_);
    }

    if ((*tokens)[startPos + variableReport.tokensToSkip + conditionReport.tokensToSkip + 2].type != _OP_SEMICOLON_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FOR_STATEMENT_);
    }

    SyntaxReport termReport = is_term(tokens, startPos + variableReport.tokensToSkip + conditionReport.tokensToSkip + 1, 1);
    SyntaxReport expressionReport = is_expression(tokens, startPos + variableReport.tokensToSkip + conditionReport.tokensToSkip + 1);

    if (termReport.errorType != _NONE_ && expressionReport.errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FOR_STATEMENT_);
    }

    int exprJump = termReport.tokensToSkip + expressionReport.tokensToSkip;
    
    if ((*tokens)[startPos + variableReport.tokensToSkip + conditionReport.tokensToSkip + exprJump + 3].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FOR_STATEMENT_);
    }
    
    SyntaxReport runnableReport = is_runnable(tokens, startPos + variableReport.tokensToSkip + conditionReport.tokensToSkip + exprJump + 4, 1);

    if (runnableReport.errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_FOR_STATEMENT_);
    }

    return create_syntax_report(NULL, variableReport.tokensToSkip + conditionReport.tokensToSkip + exprJump + runnableReport.tokensToSkip + 4, _NONE_);
}

/*
Purpose: Check if an TOKEN array is writte accordingly to the IF rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success, _NOT_AN_IF_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking the array
*/
SyntaxReport is_if(TOKEN **tokens, size_t startPos) {
    SyntaxReport ifStatementReport = is_if_statement(tokens, startPos);

    if (ifStatementReport.errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IF_);
    }

    int jumper = ifStatementReport.tokensToSkip;

    while ((*tokens)[startPos + jumper].type != __EOF__ && jumper < tokenLength) {
        SyntaxReport eleseIfReport = is_else_if_statement(tokens, startPos + jumper);

        if (eleseIfReport.errorType == _NONE_) {
            jumper += eleseIfReport.tokensToSkip;
        } else {
            break;
        }
    }

    SyntaxReport elseReport = is_else_statement(tokens, startPos + jumper);
    
    if (elseReport.errorType == _NONE_) {
        return create_syntax_report(NULL, jumper + elseReport.tokensToSkip, _NONE_);
    } else {
        return create_syntax_report(NULL, jumper, _NONE_);
    }
}

/*
Purpose: Check if an TOKEN array is writte accordingly to the IF STATEMENT rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success, _NOT_AN_IF_STATEMENT_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking the array
*/
SyntaxReport is_if_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_IF_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IF_STATEMENT_);
    }

    if ((*tokens)[startPos + 1].type == _OP_RIGHT_BRACKET_) {
        SyntaxReport conditionReport = is_chained_condition(tokens, startPos + 2);

        if (conditionReport.errorType != _NONE_) {
            return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IF_STATEMENT_);
        }

        if ((*tokens)[startPos + conditionReport.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
            return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IF_STATEMENT_);
        }

        SyntaxReport runnabelReport = is_runnable(tokens, startPos + conditionReport.tokensToSkip + 3, 1);

        if (runnabelReport.errorType == _NONE_) {
            return create_syntax_report(NULL, conditionReport.tokensToSkip + runnabelReport.tokensToSkip + 3, _NONE_);
        }
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IF_STATEMENT_);
}

/*
Purpose: Check if an TOKEN array is writte accordingly to the ELSE_IF rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success, _NOT_AN_ELSE_IF_STATEMENT_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking the array
*/
SyntaxReport is_else_if_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_ELSE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_ELSE_IF_STATEMENT_);
    }

    SyntaxReport ifReport = is_if_statement(tokens, startPos + 1);
    
    if (ifReport.errorType == _NONE_) {
        return create_syntax_report(NULL, ifReport.tokensToSkip + 1, 0);
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_ELSE_IF_STATEMENT_);
}

/*
Purpose: Check if an TOKEN array is writte accordingly to the ELSE rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success, _NOT_AN_ELSE_STATEMENT_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking the array
*/
SyntaxReport is_else_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_ELSE_) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_ELSE_STATEMENT_);
    }

    SyntaxReport runnableReport = is_runnable(tokens, startPos + 1, 1);

    if (runnableReport.errorType == _NONE_) {
        return create_syntax_report(NULL, runnableReport.tokensToSkip + 1, _NONE_);
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_ELSE_STATEMENT_);
}

/*
Purpose: Check if an TOKEN array is writte accordingly to the DO rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success, _NOT_A_DO_STATEMENT_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking the array
*/
SyntaxReport is_do_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type == _KW_DO_) {
        SyntaxReport runnableReport = is_runnable(tokens, startPos + 1, 1);

        if (runnableReport.errorType == _NONE_) {
            SyntaxReport whileConditionReport = is_while_condition(tokens, startPos + runnableReport.tokensToSkip + 1);

            if (whileConditionReport.errorType == _NONE_) {
                if ((*tokens)[startPos + runnableReport.tokensToSkip + whileConditionReport.tokensToSkip + 1].type == _OP_SEMICOLON_) {
                    return create_syntax_report(NULL, runnableReport.tokensToSkip + whileConditionReport.tokensToSkip + 2, _NONE_);
                }
            }
        }
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_DO_STATEMENT_);
}

/*
Purpose: Check if an TOKEN array is writte accordingly to the WHILE rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success, _NOT_A_WHILE_STATEMENT_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking the array
*/
SyntaxReport is_while_statement(TOKEN **tokens, size_t startPos) {
    SyntaxReport whileConditionReport = is_while_condition(tokens, startPos);

    if (whileConditionReport.errorType == _NONE_) {
        SyntaxReport runnableReport = is_runnable(tokens, startPos + whileConditionReport.tokensToSkip, 1);
        
        if (runnableReport.errorType == _NONE_) {
            return create_syntax_report(NULL, runnableReport.tokensToSkip + whileConditionReport.tokensToSkip, _NONE_);
        }
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_WHILE_STATEMENT_);
}

/*
Purpose: Check if an TOKEN array is writte accordingly to the WHILE_CONDITION rule
Return Type: SyntaxReport => ErrorType = _NONE_ on success, _NOT_A_WHILE_CONDITION_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking the array
*/
SyntaxReport is_while_condition(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type == _KW_WHILE_) {
        if ((*tokens)[startPos + 1].type == _OP_RIGHT_BRACKET_) {
            SyntaxReport chainedConditionReport = is_chained_condition(tokens, startPos + 2);
            
            if (chainedConditionReport.errorType == _NONE_
                && (*tokens)[startPos + chainedConditionReport.tokensToSkip + 2].type == _OP_LEFT_BRACKET_) {
                return create_syntax_report(NULL, chainedConditionReport.tokensToSkip + 3, _NONE_);
            } else if (((*tokens)[startPos + 2].type == _KW_TRUE_
                || (*tokens)[startPos + 2].type == _KW_FALSE_)
                && (*tokens)[startPos + 3].type == _OP_LEFT_BRACKET_) {
                return create_syntax_report(NULL, 4, _NONE_);
            }
        }
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_WHILE_CONDITION_);
}

/*
Purpose: Check if a given TOKEN array match the class instance rule
Return Type: SyntaxReport => _NOT_A_CLASS_INSTANCE_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_class_instance(TOKEN **tokens, size_t startPos) {
    if (is_identifier(tokens, startPos, 1).errorType == _NONE_
        && is_identifier(tokens, startPos + 1, 1).errorType == _NONE_
        && (*tokens)[startPos + 2].type == _OP_EQUALS_
        && (*tokens)[startPos + 3].type == _KW_NEW_) {

            SyntaxReport functionCallReport = is_function_call(tokens, startPos + 4, _PARAM_FUNCTION_CALL_);
            int endIndicator = (int)is_end_indicator(&(*tokens)[startPos + functionCallReport.tokensToSkip + 3]);

            if (functionCallReport.errorType == _NONE_ && endIndicator == 1) {
                return create_syntax_report(NULL, functionCallReport.tokensToSkip + 4, _NONE_);
            }
 
            if (is_identifier(tokens, startPos + 4, 1).errorType == _NONE_) {
                SyntaxReport arrayElementReport = is_array_element(tokens, startPos + 5);
                endIndicator = (int)is_end_indicator(&(*tokens)[startPos + arrayElementReport.tokensToSkip + 4]);

                if (arrayElementReport.errorType == _NONE_ && endIndicator == 1) {
                    return create_syntax_report(NULL, arrayElementReport.tokensToSkip + 6, _NONE_);
                }
            }
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_CLASS_INSTANCE_);
}

/*
Purpose: Check if a given TOKEN array match the variable rule
Return Type: SyntaxReport => _NOT_A_VARIABLE_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_variable(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type == _KW_VAR_) {
        if (is_identifier(tokens, startPos + 1, 1).errorType == _NONE_
            || is_pointer(&(*tokens)[startPos + 1]).errorType == _NONE_) {

            SyntaxReport normalVar = is_normal_var(tokens, startPos + 1);
            SyntaxReport parameteredVar = is_parametered_var(tokens, startPos + 1);
            SyntaxReport arrayVar = is_array_var(tokens, startPos + 1);
            SyntaxReport conditionalVar = is_conditional_assignment(tokens, startPos + 3);

            if (normalVar.errorType == _NONE_) {
                return create_syntax_report(NULL, normalVar.tokensToSkip + 1, _NONE_);
            } else if (parameteredVar.errorType == _NONE_) {
                return create_syntax_report(NULL, parameteredVar.tokensToSkip + 1, _NONE_);
            } else if (arrayVar.errorType == _NONE_) {
                return create_syntax_report(NULL, arrayVar.tokensToSkip + 2, _NONE_);
            } else if (conditionalVar.errorType == _NONE_) {
                if ((*tokens)[startPos + 2].type != _OP_EQUALS_) {
                    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_CONDITIONAL_ASSIGNMENT_);
                }

                return create_syntax_report(NULL, conditionalVar.tokensToSkip + 3, _NONE_);
            }
        }
    }

    //Constant coverage
    if ((*tokens)[startPos].type == _KW_CONST_) {
        SyntaxReport isIdentifier = is_identifier(tokens, startPos + 1, 1);

        if (isIdentifier.errorType == _NONE_) {
            SyntaxReport assignmentReport = is_assignment(tokens, startPos + isIdentifier.tokensToSkip + 1);

            if (assignmentReport.errorType == _NONE_) {
                if ((*tokens)[startPos + assignmentReport.tokensToSkip + 1].type == _OP_SEMICOLON_) {
                    return create_syntax_report(NULL, assignmentReport.tokensToSkip + 2, _NONE_);
                }
            }
        }
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_VARIABLE_);
}

/*
Purpose: Check if a given TOKEN array is a conditional assignment
Return Type: SyntaxReport => _NOT_A_CONDITIONAL_ASSIGNMENT_ on error
Params: TOKEN **tokens => Tokens to be scanned;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_conditional_assignment(TOKEN **tokens, size_t startPos) {
    SyntaxReport conditionReport = is_condition(tokens, startPos);
    
    if (conditionReport.errorType == _NONE_) {
        if ((*tokens)[startPos + conditionReport.tokensToSkip].type == _OP_QUESTION_MARK_) {
            SyntaxReport leftTerm = is_term(tokens, startPos + conditionReport.tokensToSkip + 1, 0);
            SyntaxReport rightTerm = is_term(tokens, startPos + conditionReport.tokensToSkip + leftTerm.tokensToSkip + 2, 0);
            TOKEN colonToken = (*tokens)[startPos + conditionReport.tokensToSkip + leftTerm.tokensToSkip + 1];

            if (leftTerm.errorType == _NONE_
                && rightTerm.errorType == _NONE_
                && colonToken.type == _OP_COLON_) {
                //+4, because 1 equals, 1 Questionmark, 1 Colon, 1 end indicator
                return create_syntax_report(NULL, conditionReport.tokensToSkip + leftTerm.tokensToSkip + rightTerm.tokensToSkip + 3, _NONE_);
            }
        }
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_CONDITIONAL_ASSIGNMENT_);
}

/*
Purpose: Check if a TOKEN array is a chained condition, meaning: <condition> "or"/"and" <condition>
Return Type: SyntaxReport => ErrorType = _NONE_ on success else ErrorType = _NOT_A_CHAINED_CONDITION_
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Postion from where to start checking
*/
SyntaxReport is_chained_condition(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int hasToBeLogicOperator = 0;

    while ((*tokens)[startPos + jumper].type != _OP_LEFT_BRACKET_
        && (*tokens)[startPos + jumper].type != _OP_SEMICOLON_
        && (*tokens)[startPos + jumper].type != __EOF__
        && startPos + jumper < tokenLength) {
        switch (hasToBeLogicOperator) {
        case 0: {
            SyntaxReport conditionReport = is_condition(tokens, startPos + jumper);

            if (conditionReport.errorType == _NONE_) {
                jumper += conditionReport.tokensToSkip;
                hasToBeLogicOperator = 1;
                break;
            }

            return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_CHAINED_CONDITION_);
        }
        case 1:
            if ((*tokens)[startPos + jumper].type == _KW_AND_
                || (*tokens)[startPos + jumper].type == _KW_OR_) {
                hasToBeLogicOperator = 0;
                jumper++;
                break;
            }
            
            return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_CHAINED_CONDITION_);
        }
    }

    if (hasToBeLogicOperator == 0) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_CHAINED_CONDITION_);
    }

    return create_syntax_report(NULL, jumper, _NONE_);
}

/*
Purpose: Check if a given TOKEN array matches a condition
Return Type: SyntaxReport => _NONE_ on condition; _NOT_A_CONDITION_ on error
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_condition(TOKEN **tokens, size_t startPos) {
    SyntaxReport leftTermReport = is_term(tokens, startPos, 1);
    SyntaxReport rightTermReport = is_term(tokens, startPos + leftTermReport.tokensToSkip + 1, 1);

    if (leftTermReport.errorType == _NONE_
        && (int)is_rational_operator((*tokens)[startPos + leftTermReport.tokensToSkip].value) == 1
        && rightTermReport.errorType == _NONE_) {
        return create_syntax_report(NULL, leftTermReport.tokensToSkip + rightTermReport.tokensToSkip + 1, _NONE_);
    }

    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_CONDITION_);
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
        int stop = 0;

        while (((*tokens)[startPos + jumper].type != __EOF__
            || startPos + jumper < tokenLength) && stop == 0) {

            switch ((*tokens)[startPos + jumper].type) {
            case _OP_RIGHT_BRACE_:
                openBraces++;
                jumper++;
                continue;
            case _OP_LEFT_BRACE_:
                openBraces--;

                if (openBraces == 0) {
                    stop = 1;
                }

                jumper++;
                shouldBeComma = 1;
                continue;
            case _OP_SEMICOLON_:
                break;
            default:
                break;
            }

            switch (shouldBeComma) {
            case 0: {
                shouldBeComma = 1;
                SyntaxReport isTerm = is_term(tokens, startPos + jumper, 0);

                if (isTerm.errorType != _NONE_) {
                    if (is_numeral_identifier(&(*tokens)[startPos + jumper]).errorType == _NONE_
                        && (int)is_arithmetic_operator(&(*tokens)[startPos + jumper + 1]) == 0) {
                        break;
                    }
                    
                    if (is_pointer_pointing_to_value(&(*tokens)[startPos + jumper]).errorType == _NONE_
                        || is_pointer(&(*tokens)[startPos + jumper]).errorType == _NONE_
                        || is_reference(&(*tokens)[startPos + jumper]).errorType == _NONE_) {
                        break;
                    }

                    if (is_string(&(*tokens)[startPos + jumper]) == 1) {
                        break;
                    }
                    
                    return create_syntax_report(&(*tokens)[startPos], 0, _NOT_A_VAR_BLOCK_ASSIGNMENT_);
                } else {
                    jumper += isTerm.tokensToSkip;
                    continue;
                }
            }
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
    SyntaxReport assignmentReport = is_assignment(tokens, startPos + 1);
    
    //Here: var IDEN = IDEN;
    if (assignmentReport.errorType == _NONE_
        && (*tokens)[startPos + assignmentReport.tokensToSkip].type == _OP_SEMICOLON_) {
        return create_syntax_report(NULL, assignmentReport.tokensToSkip + 1, _NONE_);

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

    while (((*tokens)[start + jumper].type != __EOF__
        || (*tokens)[start + jumper].type != _OP_EQUALS_
        || (*tokens)[start + jumper].type != _OP_SEMICOLON_)
        && start + jumper < tokenLength) {
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
    SyntaxReport isIdentifier = is_identifier(tokens, startPos, 0);
    SyntaxReport isAssignment = is_assignment(tokens, startPos + isIdentifier.tokensToSkip);

    if (isIdentifier.errorType == _NONE_) {
        if (isAssignment.errorType == _NONE_) {
            return create_syntax_report(NULL, isAssignment.tokensToSkip + isIdentifier.tokensToSkip - 1, _NONE_);
        }

        if (((int)is_decrement_operator((*tokens)[startPos + isIdentifier.tokensToSkip].value) == 1
        || (int)is_increment_operator((*tokens)[startPos + isIdentifier.tokensToSkip].value) == 1)
        && (*tokens)[startPos + isIdentifier.tokensToSkip + 1].type == _OP_SEMICOLON_) {
            return create_syntax_report(NULL, isIdentifier.tokensToSkip + 2, _NONE_);
        }
    }
    
    SyntaxReport termReport = is_term(tokens, startPos, 0);

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
        && (int)is_string(&(*tokens)[startPos + 1]) == 1
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

    SyntaxReport isIdentifier = is_identifier(tokens, currentTokenPos + 2, 0);
    
    if (((*tokens)[currentTokenPos].type != _KW_CHECK_
        || (*tokens)[currentTokenPos + 1].type != _OP_RIGHT_BRACKET_
        || isIdentifier.errorType != _NONE_
        || (*tokens)[currentTokenPos + isIdentifier.tokensToSkip + 2].type != _OP_LEFT_BRACKET_)) {
        return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_CHECK_STATEMENT_);
    }

    if ((*tokens)[currentTokenPos + isIdentifier.tokensToSkip + 3].type == _OP_RIGHT_BRACE_) {
        SyntaxReport isTokenSkip = is_is_statement(tokens, currentTokenPos + isIdentifier.tokensToSkip + 4);
        
        if (isTokenSkip.errorType == _NONE_) {
            if ((*tokens)[currentTokenPos + isTokenSkip.tokensToSkip + isIdentifier.tokensToSkip + 4].type == _OP_LEFT_BRACE_) {
                return create_syntax_report(NULL, isTokenSkip.tokensToSkip + isIdentifier.tokensToSkip + 5, _NONE_);
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
        && (*tokens)[startPos + jumper].type != __EOF__
        && startPos + jumper < tokenLength) {
        switch (isRunnableTurn) {
        case 0:
            if ((*tokens)[startPos + jumper].type == _KW_IS_
                && (is_identifier(tokens, startPos + jumper + 1, 0).errorType == _NONE_
                || is_numeral_identifier(&(*tokens)[startPos + jumper + 1]).errorType == _NONE_
                || is_bool((*tokens)[startPos + jumper + 1].value) == 1
                || is_string(&(*tokens)[startPos + jumper + 1]) == 1)
                && (*tokens)[startPos + jumper + 2].type == _OP_COLON_) {
                isRunnableTurn = 1;
                jumper += 3;
                continue;
            } else {
                return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IS_STATEMENT_);
            }
        case 1: {
            SyntaxReport runnableReport = is_runnable(tokens, startPos + jumper, 2);

            if (runnableReport.errorType == _NONE_ && runnableReport.tokensToSkip > 1) {
                isRunnableTurn = 0;
                jumper += runnableReport.tokensToSkip;
                continue;
            } else {
                return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IS_STATEMENT_);
            }
        }
        }
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

    SyntaxReport functionCallReport = is_function_call(tokens, currentTokenPosition + 1, _PARAM_CLASS_);
    
    if (functionCallReport.errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_CLASS_);
    }
    
    int withTokensToSkip = is_with_statement(tokens, currentTokenPosition + functionCallReport.tokensToSkip).tokensToSkip;

    if ((*tokens)[currentTokenPosition + functionCallReport.tokensToSkip + withTokensToSkip].type != _OP_CLASS_CREATOR_
        || (*tokens)[currentTokenPosition + functionCallReport.tokensToSkip + withTokensToSkip + 1].type != _OP_RIGHT_BRACE_) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_CLASS_);
    }
    
    SyntaxReport runnableReport = is_runnable(tokens, currentTokenPosition + withTokensToSkip + functionCallReport.tokensToSkip + 1, 1);

    return create_syntax_report(NULL, functionCallReport.tokensToSkip + withTokensToSkip + runnableReport.tokensToSkip + 1, _NONE_);
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
    if ((*tokens)[currentTokenPosition].type == _OP_EQUALS_
        || is_rational_operator((*tokens)[currentTokenPosition].value) == 1) {
        SyntaxReport termReport = is_term(tokens, currentTokenPosition + 1, 0);

        if (termReport.errorType == _NONE_
            && (int)is_end_indicator(&(*tokens)[currentTokenPosition + termReport.tokensToSkip + 1]) == 1) {
            return create_syntax_report(NULL, termReport.tokensToSkip + 2, _NONE_);
        }
        
        int isString = (int)is_string(&(*tokens)[currentTokenPosition + 1]);
        int isBool = (int)is_bool((*tokens)[currentTokenPosition + 1].value);
        SyntaxReport isIdentifier = is_identifier(tokens, currentTokenPosition + 1, 0);
        SyntaxReport isFunctionCall = is_function_call(tokens, currentTokenPosition + 1, _PARAM_FUNCTION_CALL_);
        SyntaxReport isTerm = is_term(tokens, currentTokenPosition, 0);
        SyntaxReport isClassObjectAccess = is_class_object_access(tokens, currentTokenPosition + 1);

        if ((isIdentifier.errorType == _NONE_
            || (*tokens)[currentTokenPosition + 1].type == _KW_NULL_
            || isFunctionCall.errorType == _NONE_ || isTerm.errorType == _NONE_)
            && isClassObjectAccess.errorType != _NONE_) {
            //Only one return can be true, so adding all only gives the skips of the correct tokenLength to skip
            int skip = isIdentifier.tokensToSkip + isFunctionCall.tokensToSkip + isTerm.tokensToSkip;

            if ((int)is_end_indicator(&(*tokens)[currentTokenPosition + skip + 1]) == 1) {
                return create_syntax_report(NULL, skip + 1, _NONE_);
            }
        }  else if (isClassObjectAccess.errorType == _NONE_) {
            return create_syntax_report(NULL, isClassObjectAccess.tokensToSkip + 1, _NONE_);
        } else if (isBool == 1 || isString == 1) {
            if ((int)is_end_indicator(&(*tokens)[currentTokenPosition + 2]) == 1) {
                return create_syntax_report(NULL, 3, _NONE_);
            }
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
        size_t currentTokenPosition => Position from here to start checking;
        int inFunctionCall => Boolean if the term is an function call or not 
*/
SyntaxReport is_term(TOKEN **tokens, size_t currentTokenPosition, int inFunctionCall) {
    if ((*tokens)[currentTokenPosition].type == __EOF__) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_TERM_);
    }

    SyntaxReport isIdentifier = is_identifier(tokens, currentTokenPosition, 0);
    SyntaxReport isNumeralIdentifier = is_numeral_identifier(&(*tokens)[currentTokenPosition]);

    if ((isIdentifier.errorType == _NONE_ || isNumeralIdentifier.errorType == _NONE_)) {
        int skip = isIdentifier.tokensToSkip + isNumeralIdentifier.tokensToSkip;

        if ((int)is_end_indicator(&(*tokens)[currentTokenPosition + skip]) == 1
            && (*tokens)[currentTokenPosition + skip].type != _OP_RIGHT_BRACKET_) {
            return create_syntax_report(NULL, skip, _NONE_);
        }
    }

    if ((*tokens)[currentTokenPosition].type == _KW_TRUE_
        || (*tokens)[currentTokenPosition].type == _KW_FALSE_) {
        if ((int)is_end_indicator(&(*tokens)[currentTokenPosition + 1]) == 1) {
            return create_syntax_report(NULL, 1, _NONE_);
        }
    }

    SyntaxReport functionCallReport = is_function_call(tokens, currentTokenPosition, _PARAM_FUNCTION_CALL_);

    if (functionCallReport.errorType == _NONE_
        && (int)is_end_indicator(&(*tokens)[currentTokenPosition + functionCallReport.tokensToSkip]) == 1) {
        return create_syntax_report(NULL, functionCallReport.tokensToSkip, _NONE_);
    }

    SyntaxReport simpleTermReport = is_simple_term(tokens, currentTokenPosition, inFunctionCall);
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

    while ((*tokens)[startPosition + jumpTokensForward].type != __EOF__
        && startPosition + jumpTokensForward < tokenLength) {
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
            SyntaxReport isFuncCall = is_function_call(tokens, startPosition + jumpTokensForward, _PARAM_FUNCTION_CALL_);

            if (isFuncCall.errorType == _NONE_) {
                jumpTokensForward += isFuncCall.tokensToSkip - 1;
                hasToBeArithmeticOperator = 1;
                continue;
            }

            if (is_pointer_pointing_to_value(&currentToken).errorType == _NONE_
                || is_numeral_identifier(&currentToken).errorType == _NONE_
                || currentToken.type == _KW_TRUE_
                || currentToken.type == _KW_FALSE_) {
                jumpTokensForward++;
                hasToBeArithmeticOperator = 1;
                continue;
            }

            SyntaxReport isIdentifier = is_identifier(tokens, startPosition + jumpTokensForward, 0);

            if (isIdentifier.errorType == _NONE_) {
                jumpTokensForward += isIdentifier.tokensToSkip;
                hasToBeArithmeticOperator = 1;
                continue;
            }

            return create_syntax_report(&currentToken, 0, _NOT_A_SIMPLE_TERM_);
        }
        case 1:
            if ((int)is_arithmetic_operator(&currentToken) == 0) {
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
Purpose: Check if a given TOKEN matches an "end of statement" indicator ("=", ";", "]", "}", "?", ")", "," ...)
Return Type: int => 1 = is end indicator; 0 = is not an end indicator
Params: const TOKEN *token -> Token to be checked
*/
const char endIndicators[][4] = {"=", ";", "]", "}", ")", "?", ",",
                                "<", ">", "<=", ">=", "!=", "==", ":",
                                "and", "or", "-=", "+=", "*=", "/="};

int is_end_indicator(const TOKEN *token) {
    if (token->type == __EOF__) {
        return 0;
    }

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

    SyntaxReport isRunnable = is_runnable(tokens, currentTokenPosition + 1, 1);
    
    if (isRunnable.errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_TRY_);
    }

    int catchStatementSkips = is_catch_statement(tokens, currentTokenPosition + isRunnable.tokensToSkip + 1).tokensToSkip;

    if (catchStatementSkips == 0) {
        return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_TRY_);
    }

    return create_syntax_report(NULL, catchStatementSkips + isRunnable.tokensToSkip + 1, _NONE_);
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

    if (is_identifier(tokens, startPosition + 2, 1).errorType != _NONE_
        || (*tokens)[startPosition + 3].type != _OP_LEFT_BRACKET_) {
        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_CATCH_);
    }

    SyntaxReport isRunnable = is_runnable(tokens, startPosition + 4, 1);
    
    if (isRunnable.errorType != _NONE_) {
        return create_syntax_report(&(*tokens)[startPosition], 0, _NOT_A_CATCH_);
    }

    return create_syntax_report(NULL, isRunnable.tokensToSkip + 4, _NONE_);
}

/*
Purpose: Check if the following tokens starting from currentTokenPosition match the INCLUDE rule
Return Type: SyntaxReport => Contains errors, tokensToSkip, token itself when error
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t currentTokenPosition => Position of the current token
*/
SyntaxReport is_include(TOKEN **tokens, size_t currentTokenPosition) {
    if ((*tokens)[currentTokenPosition].type != _KW_INCLUDE_
        || (int)is_string(&(*tokens)[currentTokenPosition + 1]) != 1
        || (*tokens)[currentTokenPosition + 2].type != _OP_SEMICOLON_) {
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
        || is_identifier(tokens, currentTokenPosition + 1, 1).errorType != _NONE_
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

    while ((*tokens)[startPosition + tokensToSkip].type != _OP_LEFT_BRACE_
        && (*tokens)[startPosition + tokensToSkip].type != __EOF__
        && startPosition + tokensToSkip < tokenLength) {
        switch (commaAwaited) {
        case 0:
            //Layout here: <IDENTIFIER>
            if (is_identifier(tokens, startPosition + tokensToSkip, 1).errorType == _NONE_) {
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
    if ((*tokens)[currentTokenPosition].type == _KW_BREAK_
        && (*tokens)[currentTokenPosition + 1].type == _OP_SEMICOLON_) {
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
    int modifier = 0;

    switch ((*tokens)[currentTokenPosition].type) {
    case _KW_GLOBAL_:
    case _KW_SECURE_:
    case _KW_PRIVATE_:
        index++;
        modifier = 1;
        break;
    default:
        break;
    }

    if ((*tokens)[index].type == _KW_FUNCTION_) {
        int skipTokens = is_function_call(tokens, index + 1, _PARAM_FUNCTION_).tokensToSkip;

        if (skipTokens == 0) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_);
        }

        SyntaxReport isRunnable = is_runnable(tokens, index + skipTokens, 1);

        if (isRunnable.errorType != _NONE_) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_);
        } else {
            return create_syntax_report(NULL, skipTokens + isRunnable.tokensToSkip + modifier, _NONE_);
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
    
    for (int i = 0; i + currentTokenPosition < tokenLength; i++) {
        TOKEN currentToken = (*tokens)[currentTokenPosition + i];

        if (currentToken.type == __EOF__) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
        }
        
        if (i == 0 && is_identifier(tokens, currentTokenPosition + i, 1).errorType != _NONE_) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
        } else if (i == 1 && currentToken.type != _OP_RIGHT_BRACKET_) {
            return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
        } else if (i > 1 && workedDownParameters == 0) {
            SyntaxReport isParam = is_parameter(tokens, currentTokenPosition + i, parameterUsage, _OP_LEFT_BRACKET_);

            if (isParam.errorType != _NONE_) {
                return create_syntax_report(&(*tokens)[currentTokenPosition], 0, _NOT_A_FUNCTION_CALL_);
            }
            
            checkedTokens += isParam.tokensToSkip;
            i += isParam.tokensToSkip - 1;
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

    while ((*tokens)[i].type != crucialType && (*tokens)[i].type != __EOF__ && i < tokenLength) {
        switch (isCurrentlyComma) {
        case 0:
            switch (usage) {
            case _PARAM_FUNCTION_CALL_: {
                SyntaxReport isSimpleTerm = is_simple_term(tokens, i, 1);

                if (isSimpleTerm.errorType == _NONE_) {
                    i += isSimpleTerm.tokensToSkip;
                    isCurrentlyComma = 1;
                    continue;
                } else if (is_atom(&(*tokens)[i]).errorType == _NONE_
                    && (*tokens)[i + 1].type != _OP_EQUALS_
                    && (*tokens)[i + 1].type != _OP_RIGHT_BRACKET_) {
                    break;
                } else if (is_pointer_pointing_to_value(&(*tokens)[i]).errorType == _NONE_
                    || is_reference(&(*tokens)[i]).errorType == _NONE_
                    || is_pointer(&(*tokens)[i]).errorType == _NONE_) {
                    break;
                } else {
                    SyntaxReport isFuncCall = is_function_call(tokens, i, _PARAM_FUNCTION_CALL_);
                    
                    if (isFuncCall.errorType == _NONE_) {
                        i += isFuncCall.tokensToSkip - 2;
                        break;
                    }
                    
                    return create_syntax_report(&(*tokens)[currentTokenPos], 0, _NOT_A_PARAMETER_);
                }
            }
            case _PARAM_WITH_STATEMENT_:
            case _PARAM_CLASS_:
            case _PARAM_FUNCTION_: {
                SyntaxReport isFunctionParamInit = is_function_parameter_initializer(tokens, i);

                if (isFunctionParamInit.errorType == _NONE_) {
                    i += isFunctionParamInit.tokensToSkip;
                    break;
                }

                if (is_identifier(tokens, i, 1).errorType == _NONE_) {
                    break;
                } else if (is_pointer(&(*tokens)[i]).errorType == _NONE_) {
                    break;
                }

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
    if (is_identifier(tokens, startPos, 1).errorType == _NONE_
        && (*tokens)[startPos + 1].type == _OP_EQUALS_) {
        
        SyntaxReport isIdentifier = is_identifier(tokens, startPos + 2, 0);
        SyntaxReport isNumeralIdentifier = is_numeral_identifier(&(*tokens)[startPos + 2]);

        if (isIdentifier.errorType == _NONE_ || isNumeralIdentifier.errorType == _NONE_) {
            return create_syntax_report(NULL, 2, _NONE_);
        }
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
    }  else if ((int)is_string(token) == 1) {
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
Params: TOKEN **tokens => Tokens to be checked;
        size_t tokenPos => Token Position;
        int pointingIdentifier => If the check is for a normal or poiting identifier
        {If the last flag is active, it means the check is already checking for a nested
        idetifier something like: "this.test". The flag is to prevent unwanted recursions}
*/
SyntaxReport is_identifier(TOKEN **tokens, size_t tokenPos, int pointingIdentifier) {
    TOKEN *token = &(*tokens)[tokenPos];

    if (token == NULL || token->value == NULL || tokenPos > tokenLength) {
        (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
    }

    if ((int)is_keyword(token->value) == 1) {
        return create_syntax_report(token, 0, _NOT_AN_IDENTIFIER_);
    }

    for (size_t i = 0; i < token->size; i++) {
        char currentCharacter = (*token).value[i];

        if (currentCharacter == '\0') {
            break;
        } else if ((int)is_letter(currentCharacter) == 1) {
            continue;
        } else if ((int)is_number(currentCharacter) == 1 && i > 0) {
            continue;
        } else if ((int)is_underscore(currentCharacter) == 1) {
            continue;
        }

        return create_syntax_report(&(*tokens)[tokenPos], 0, _NOT_AN_IDENTIFIER_);
    }

    if ((*tokens)[tokenPos + 1].type == _OP_DOT_ && pointingIdentifier == 0) {
        SyntaxReport pointingReport = is_pointer_identifier(tokens, tokenPos);

        if (pointingReport.errorType == _NONE_) {
            return create_syntax_report(NULL, pointingReport.tokensToSkip, _NONE_);
        } else {
            return create_syntax_report(&(*tokens)[tokenPos], 0, _NOT_AN_IDENTIFIER_);
        }
    }

    return create_syntax_report(NULL, 1, _NONE_);
}

/*
Purpose: Check whether a given value is written according to the POINTER IDENTIFIER rule
Return Type: SyntaxReport => Contains how many tokens to skip and if the token has an error, it gets delivered with the report
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport is_pointer_identifier(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int hasToBeDot = 0;

    while ((*tokens)[startPos + jumper].type != __EOF__
        && startPos + jumper < tokenLength) {
        if (is_end_indicator(&(*tokens)[startPos + jumper]) == 1) {
            break;
        }

        switch (hasToBeDot) {
        case 0: {
            SyntaxReport functionCallReport = is_function_call(tokens, startPos + jumper, _PARAM_FUNCTION_CALL_);

            if (functionCallReport.errorType == _NONE_) {
                jumper += functionCallReport.tokensToSkip - 1;
                hasToBeDot = 1;
                continue;
            }
            
            if (is_identifier(tokens, startPos + jumper, 1).errorType == _NONE_) {
                hasToBeDot = 1;
                break;
            }

            return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IDENTIFIER_);
        }
        case 1:
            if ((*tokens)[startPos + jumper].type != _OP_DOT_) {
                return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IDENTIFIER_);
            }

            hasToBeDot = 0;
            break;
        }

        jumper++;
    }

    //hasToBeDot == 0 because it indicates, that the pointing identifier ends with '.'
    if (hasToBeDot == 0) {
        return create_syntax_report(&(*tokens)[startPos], 0, _NOT_AN_IDENTIFIER_);
    }

    return create_syntax_report(NULL, jumper, _NONE_);
}

/*
Purpose: Check if a given value is a keyword or not
Return Type: int => 1 = is keyword; 0 = not a keyword
Params: char *value => Value to be checked
*/
int is_keyword(char *value) {
    for (int i = 0; i < sizeof(KeywordLookupTable) / sizeof(KeywordLookupTable[0]); i++) {
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
const char rationalOperators[][3] = {"==", "<=", ">=", "!=", "<", ">"};

int is_rational_operator(const char *sequence) {
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
Params: TOKEN *token => Token to be checked
*/
int is_arithmetic_operator(TOKEN *token) {
    //Could be double operators like += or -= or *= ect.
    if (strlen(token->value) != 1) {
        return 0;
    }

    switch (token->value[0]) {
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
    int lengthOfAssignmentOperators = (sizeof(assignmentOperator) / sizeof(assignmentOperator[0]));

    for (int i = 0; i < lengthOfAssignmentOperators; i++) {
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

void throw_error(TOKEN *token, SyntaxErrorType error) {}