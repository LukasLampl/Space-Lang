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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../headers/Token.h"
#include "../headers/errors.h"

#define true 1
#define false 0

typedef struct SyntaxReport {
    TOKEN *token;
    int tokensToSkip;
    int errorOccured;
    char *expectedToken;
} SyntaxReport;

enum ParameterType {
    _PARAM_FUNCTION_CALL_, _PARAM_FUNCTION_, _PARAM_CLASS_
};

int SA_enter_panic_mode(TOKEN **tokens, size_t startPos, int runnableWithBlock);
SyntaxReport SA_is_runnable(TOKEN **tokens, size_t startPos, int withBlock);
SyntaxReport SA_is_non_keyword_based_runnable(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_null_assigned_class_instance(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_runnable_function_call(TOKEN **tokens, size_t startPos);
int SA_predict_expression(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_keyword_based_runnable(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_class_object_access(TOKEN **tokens, size_t startPos, int independentCall);
SyntaxReport SA_is_return_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_return_class_instance(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_continue_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_break_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_for_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_expression(TOKEN **tokens, size_t startPos, int inRunnable);
SyntaxReport SA_is_if_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_else_if_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_else_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_do_statment(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_while_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_while_condition(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_check_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_is_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_variable(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_class_instance(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_conditional_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_simple_conditional_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_chained_condition(TOKEN **tokens, size_t startPos, int inParam);
SyntaxReport SA_is_condition(TOKEN **tokens, size_t startPos, int inParam);
int SA_predict_is_conditional_variable_type(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_multiple_variable_definition(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_multiple_variable_definition_identifier(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_array_variable(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_array_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_array_element(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_class_constructor(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_class(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_with_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_try_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_catch_statement(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_export(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_include(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_enum(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_enumerator(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_function(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_function_return_type_specifier(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_function_call(TOKEN **tokens, size_t startPos, int inFunction);
SyntaxReport SA_is_parameter(TOKEN **tokens, size_t startPos, enum ParameterType type);
SyntaxReport SA_is_simple_term(TOKEN **tokens, size_t startPos, int inParameter);
int SA_predict_class_object_access(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_identifier(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_array_identifier(TOKEN **tokens, size_t startPos);
int SA_is_root_identifier(TOKEN *token);
SyntaxReport SA_is_numeral_identifier(TOKEN *token);

int SA_is_end_indicator(const TOKEN *token);
int SA_is_string(TOKEN *token);
int SA_is_reference(TOKEN *token);
int SA_is_pointer(const TOKEN *token);
int SA_is_keyword(TOKEN *token);
int SA_is_letter(const char character);
int SA_is_number(const char character);
int SA_is_rational_operator(const char *sequence);
int SA_is_arithmetic_operator(const TOKEN *token);
int SA_is_assignment_operator(const char *sequence);
int SA_is_underscore(const char character);
int SA_is_bool(const char *sequence);
int SA_is_modifier(const char *sequence);
int SA_is_logic_operator(const char *sequence);

SyntaxReport SA_create_syntax_report(TOKEN *token, int tokensToSkip, int errorOccured, char *expextedToken);
void SA_throw_error(TOKEN *errorToken, char *expectedToken);

int FILE_CONTAINS_ERRORS = 0;
size_t MAX_TOKEN_LENGTH = 0;
char **SOURCE_CODE = NULL;
size_t SOURCE_LENGTH = 0;
char *FILE_NAME = NULL;

/*
Purpose: Checks all TOKENS and returns errors
Return Type: int => 1 = Syntax is correct; 0 = Syntax is incorrect
Params: TOKEN **tokens => TOKEN array in which the error/s occured;
        size_t tokenArrayLength => Size of the TOKEN array
        char **source => Sourcec code from the user;
        size_t sourceSize => Length of the source code;
        char *sourceName => Filename of the source code
*/
int CheckInput(TOKEN **tokens, size_t tokenArrayLength, char **source, size_t sourceSize, char *sourceName) {
    MAX_TOKEN_LENGTH = tokenArrayLength;
    SOURCE_CODE = source;
    SOURCE_LENGTH = sourceSize;
    FILE_NAME = sourceName;

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

    //Actual check (everything is a runnable, just without or with blocks e.g. braces)
    for (int i = 0; i < 1; i++) {
        (void)SA_is_runnable(tokens, 0, false);
    }

    if (PARSER_DEBUG_MODE == true) {
        (void)printf("\n>>>>>    Tokens successfully analyzed    <<<<<\n");
    }

    if (PARSER_DISPLAY_USED_TIME == true) {
        end = clock();
        (void)printf("\nCPU time used for SYNTAX ANALYSIS: %f seconds\n", ((double) (end - start)) / CLOCKS_PER_SEC);   
    }

    return FILE_CONTAINS_ERRORS == false ? 0 : 1;
}

/*
Purpose: Enters the panic mode and skips as many TOKENS as needed to recover
Return Type: int => How many tokens got skipped
Params: TOKEN **tokens => TOKEN array in which the error/s occured;
        size_t startPos => Position from where to start skipping
        int runnableWithBlock => If the error occured in a runnable with block or not
*/
int panicModeOpenBraces = 0;
int panicModeLastStartPos = 0;

int SA_enter_panic_mode(TOKEN **tokens, size_t startPos, int runnableWithBlock) {
    for (size_t i = panicModeLastStartPos; i < startPos; i++) {
        if ((*tokens)[i].type == _OP_LEFT_BRACE_) {
            panicModeOpenBraces--;
        } else if ((*tokens)[i].type == _OP_RIGHT_BRACE_) {
            panicModeOpenBraces++;
        }
    }

    panicModeLastStartPos = startPos;

    for (size_t i = startPos + 1; i < MAX_TOKEN_LENGTH + 1; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (currentToken->type == __EOF__) {
            if (panicModeOpenBraces > 1) {
                (void)printf("SYNTAX ERROR: Missing %i closing braces \"}\".\n", panicModeOpenBraces);
                (void)printf("Estimated line: %i (%s)\n", (*tokens)[startPos].line + 1, FILE_NAME);
            } else if (panicModeLastStartPos == 1) {
                (void)printf("SYNTAX ERROR: Missing 1 closing brace \"}\".\n");
                (void)printf("Estimated line: %i (%s)\n", (*tokens)[startPos].line + 1, FILE_NAME);
            }

            return i - startPos;
        }

        if (runnableWithBlock == true) {
            if (currentToken->type == _OP_LEFT_BRACE_) {
                panicModeOpenBraces --;

                if (panicModeOpenBraces == 0) {
                    return i - startPos;
                }
            } else if (currentToken->type == _OP_RIGHT_BRACE_) {
                panicModeOpenBraces++;
            }
        } else if ((int)SA_is_keyword(currentToken) == true) {
            switch (currentToken->type) {
            case _KW_NEW_:
            case _KW_WHILE_:
                continue;
            default:
                return i - startPos;
            }
        }
    }

    return -1;
}

/*
Purpose: Checks if the current TOKEN array matches the RUNNABLE rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int independentCall => Flag if the object access was called by term or indepentently
*/
SyntaxReport SA_is_runnable(TOKEN **tokens, size_t startPos, int withBlock) {
    int jumper = 0;
    
    if (withBlock == true) {
        if ((*tokens)[startPos].type != _OP_RIGHT_BRACE_) {
            return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "{");
        } else {
            jumper++;
        }
    }

    while (startPos + jumper <  MAX_TOKEN_LENGTH
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
        
        SyntaxReport isKWBasedRunnable = SA_is_keyword_based_runnable(tokens, startPos + jumper);

        if (isKWBasedRunnable.errorOccured == true) {
            (void)SA_throw_error(isKWBasedRunnable.token, isKWBasedRunnable.expectedToken);
            int skip = (int)SA_enter_panic_mode(tokens, startPos + jumper, withBlock);
            
            if (skip > 0) {
                jumper += skip;
                continue;
            } else {
                if (withBlock == true) {
                    return SA_create_syntax_report(isKWBasedRunnable.token, 0, true, isKWBasedRunnable.expectedToken);
                } else {
                    jumper++;
                    continue;
                }
            }
        } else if (isKWBasedRunnable.errorOccured == false
            && isKWBasedRunnable.tokensToSkip > 0) {
            jumper += isKWBasedRunnable.tokensToSkip;
            continue;
        }
        
        SyntaxReport isNKWBasedRunnable = SA_is_non_keyword_based_runnable(tokens, startPos + jumper);

        if (isNKWBasedRunnable.errorOccured == true) {
            (void)SA_throw_error(isNKWBasedRunnable.token, isNKWBasedRunnable.expectedToken);
            int skip = (int)SA_enter_panic_mode(tokens, startPos + jumper, withBlock);

            if (skip > 0) {
                jumper += skip;
                return SA_create_syntax_report(isKWBasedRunnable.token, skip, false, isKWBasedRunnable.expectedToken);
            } else {
                if (withBlock == true) {
                    return SA_create_syntax_report(isKWBasedRunnable.token, 0, true, isKWBasedRunnable.expectedToken);
                } else {
                    jumper++;
                    continue;
                }
            }
        } else if (isNKWBasedRunnable.errorOccured == false
            && isNKWBasedRunnable.tokensToSkip > 0) {
            jumper += isNKWBasedRunnable.tokensToSkip;
        } else {
            return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<ERROR>");
        }
    }

    if (withBlock == true) {
        if  ((*tokens)[startPos + jumper].type != _OP_LEFT_BRACE_) {
            return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "}");
        } else {
            jumper++;
        }
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Predicts what the token array wants to achieve, then tries it
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int independentCall => Flag if the object access was called by term or indepentently
*/
SyntaxReport SA_is_non_keyword_based_runnable(TOKEN **tokens, size_t startPos) {
    if ((int)SA_predict_expression(tokens, startPos) == true) {
        SyntaxReport isExpression = SA_is_expression(tokens, startPos, true);

        if (isExpression.errorOccured == false) {
            return isExpression;
        }
        
        SyntaxReport isNullAssignementToClassInstance = SA_is_null_assigned_class_instance(tokens, startPos);
        
        if (isNullAssignementToClassInstance.errorOccured == false) {
            return isNullAssignementToClassInstance;
        }
    } else if ((int)SA_predict_class_object_access(tokens, startPos) == true) {
        return SA_is_class_object_access(tokens, startPos, true);
    } else {
        SyntaxReport isRunnableFunctionCall = SA_is_runnable_function_call(tokens, startPos);
        
        if (isRunnableFunctionCall.errorOccured == false) {
            return isRunnableFunctionCall;
        }
    }

    return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "<EXPRESSION>\" or \"<CLASS_OBJECT_ACCESS>");
}

/*
Purpose: Check if the following tokens are a null assignment to a class instance
Return Type: SyntaxReport => errorOccured = false on success + holding how many tokens to skip
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
*/
SyntaxReport SA_is_null_assigned_class_instance(TOKEN **tokens, size_t startPos) {
    int isRootIdentifier = SA_is_root_identifier(&(*tokens)[startPos]);
    int skip = 1;
    
    if (isRootIdentifier == false) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 1].type == _OP_RIGHT_EDGE_BRACKET_) {
        while (startPos + skip < MAX_TOKEN_LENGTH) {
            if ((*tokens)[startPos + skip].type == _OP_RIGHT_EDGE_BRACKET_) {
                SyntaxReport isArrayIdentifier = SA_is_array_identifier(tokens, startPos + skip);

                if (isArrayIdentifier.errorOccured == false) {
                    skip += isArrayIdentifier.tokensToSkip;
                    continue;
                } else {
                    return SA_create_syntax_report(isArrayIdentifier.token, 0, true, isArrayIdentifier.expectedToken);
                }
            }

            break;
        }
    }

    isRootIdentifier = SA_is_root_identifier(&(*tokens)[startPos + skip]);
    skip += isRootIdentifier;

    if (isRootIdentifier == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + skip].type != _OP_EQUALS_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "=");
    }

    if ((*tokens)[startPos + skip + 1].type != _KW_NULL_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip + 1], 0, true, "null");
    }

    if ((*tokens)[startPos + skip + 2].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip + 2], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, skip + 3, false, NULL);
}

/*
Purpose: Tries if the non keyword runnable is a function call or not
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int independentCall => Flag if the object access was called by term or indepentently
*/
SyntaxReport SA_is_runnable_function_call(TOKEN **tokens, size_t startPos) {
    SyntaxReport isIdentifier = SA_is_identifier(tokens, startPos);

    if (isIdentifier.errorOccured == true) {
        return SA_create_syntax_report(isIdentifier.token, 0, true, isIdentifier.expectedToken);
    }

    if ((*tokens)[startPos + isIdentifier.tokensToSkip - 1].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip - 1], 0, true, ")");
    }
    
    if ((*tokens)[startPos + isIdentifier.tokensToSkip].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, isIdentifier.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Tries to predict if the current token array is a class instance or not
Return Type: int => true = following array is class instance; false following array is not a class instance
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
*/
int SA_predict_class_instance(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int facedSemicolon = false;

    while (startPos + jumper < MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken->type == _KW_NEW_) {
            if (facedSemicolon == true) {
                return false;
            } else {
                return true;
            }
        }

        if (currentToken->type == _OP_SEMICOLON_) {
            facedSemicolon = true;
        } else if (currentToken->type == _OP_RIGHT_BRACE_) {
            return false;
        }

        jumper++;
    }

    return false;
}

/*
Purpose: Tries to predict if the current token array is an expression or not
Return Type: int => true = following array is expression; false following array is not an expression
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
*/
int SA_predict_expression(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    
    while (startPos + jumper <  MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if ((int)SA_is_assignment_operator(currentToken->value) == true
            || currentToken->type == _OP_EQUALS_
            || currentToken->type == _OP_ADD_ONE_
            || currentToken->type == _OP_SUBTRACT_ONE_) {
            return true;
        }

        if (currentToken->type == _OP_SEMICOLON_
            || currentToken->type == _OP_RIGHT_BRACE_) {
            return false;
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
SyntaxReport SA_is_keyword_based_runnable(TOKEN **tokens, size_t startPos) {
    switch ((*tokens)[startPos].type) {
    case _KW_GLOBAL_:
    case _KW_SECURE_:
    case _KW_PRIVATE_:
        if ((*tokens)[startPos + 1].type == _KW_VAR_
            || (*tokens)[startPos + 1].type == _KW_CONST_) {
            return SA_is_variable(tokens, startPos);
        } else if ((*tokens)[startPos + 1].type == _KW_FUNCTION_) {
            return SA_is_function(tokens, startPos);
        } else if ((*tokens)[startPos + 1].type == _KW_CLASS_) {
            return SA_is_class(tokens, startPos);
        } else {
            return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "var\", \"const\" or \"function");
        }

        break;
    case _KW_VAR_:
    case _KW_CONST_:
        return SA_is_variable(tokens, startPos);
    case _KW_FUNCTION_:
        return SA_is_function(tokens, startPos);
    case _KW_CLASS_:
        return SA_is_class(tokens, startPos);
    case _KW_IF_:
        return SA_is_if_statement(tokens, startPos);
    case _KW_ELSE_:
        if ((*tokens)[startPos + 1].type == _KW_IF_) {
            return SA_is_else_if_statement(tokens, startPos);
        } else {
            return SA_is_else_statement(tokens, startPos);
        }
    case _KW_WHILE_:
        return SA_is_while_statement(tokens, startPos);
    case _KW_DO_:
        return SA_is_do_statment(tokens, startPos);
    case _KW_FOR_:
        return SA_is_for_statement(tokens, startPos);
    case _KW_TRY_:
        return SA_is_try_statement(tokens, startPos);
    case _KW_CATCH_:
        return SA_is_catch_statement(tokens, startPos);
    case _KW_CHECK_:
        return SA_is_check_statement(tokens, startPos);
    case _KW_INCLUDE_:
        return SA_is_include(tokens, startPos);
    case _KW_EXPORT_:
        return SA_is_export(tokens, startPos);
    case _KW_ENUM_:
        return SA_is_enum(tokens, startPos);
    case _KW_THIS_:
        if ((*tokens)[startPos + 1].type == _OP_COLON_) {
            return SA_is_class_constructor(tokens, startPos);
        }

        break;
    case _KW_BREAK_:
        return SA_is_break_statement(tokens, startPos);
    case _KW_RETURN_:
        return SA_is_return_statement(tokens, startPos);
    case _KW_CONTINUE_:
        return SA_is_continue_statement(tokens, startPos);
    default:
        break;
    }

    return SA_create_syntax_report(NULL, 0, false, "N/A");
}

/*
Purpose: Checks if the current TOKEN array matches the CLASS_OBJECT_ACCESS rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int independentCall => Flag if the object access was called by term or indepentently
*/
SyntaxReport SA_is_class_object_access(TOKEN **tokens, size_t startPos, int independentCall) {
    SyntaxReport leftIdentifier = SA_is_identifier(tokens, startPos);

    if (leftIdentifier.errorOccured == true) {
        return SA_create_syntax_report(leftIdentifier.token, 0, true, leftIdentifier.expectedToken);
    }

    if ((*tokens)[startPos + leftIdentifier.tokensToSkip].type != _OP_CLASS_ACCESSOR_) {
        return SA_create_syntax_report(&(*tokens)[startPos + leftIdentifier.tokensToSkip], 0, true, "->");
    }

    SyntaxReport rightIdentifier = SA_is_identifier(tokens, startPos + leftIdentifier.tokensToSkip + 1);

    if (rightIdentifier.errorOccured == true) {
        return SA_create_syntax_report(rightIdentifier.token, 0, true, rightIdentifier.expectedToken);
    }

    if (independentCall == true) {
        if ((*tokens)[startPos + leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 1].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos + leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 1], 0, true, ";");
        }

        return SA_create_syntax_report(NULL, leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 2, false, NULL);
    }

    return SA_create_syntax_report(NULL, leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the RETURN rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_return_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_RETURN_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "return");
    }

    if ((*tokens)[startPos + 1].type != _KW_NEW_) {
        SyntaxReport isCondAssignment = SA_is_simple_conditional_assignment(tokens, startPos + 1);
        
        if (isCondAssignment.errorOccured == false) {
            return SA_create_syntax_report(NULL, isCondAssignment.tokensToSkip + 1, false, NULL);
        }

        SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + 1, false);

        if (isSimpleTerm.errorOccured == true) {
            return SA_create_syntax_report(isSimpleTerm.token, 0, true, isSimpleTerm.expectedToken);
        }

        if ((*tokens)[startPos + isSimpleTerm.tokensToSkip + 1].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos + isSimpleTerm.tokensToSkip + 1], 0, true, ";");
        }

        return SA_create_syntax_report(NULL, isSimpleTerm.tokensToSkip + 2, false, NULL);
    }

    SyntaxReport isReturnClassInstance = SA_is_return_class_instance(tokens, startPos + 1);

    if (isReturnClassInstance.errorOccured == true) {
        return SA_create_syntax_report(isReturnClassInstance.token, 0, true, isReturnClassInstance.expectedToken);
    }

    return SA_create_syntax_report(NULL, isReturnClassInstance.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is a class instance created as return value
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_return_class_instance(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_NEW_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "new");
    }

    SyntaxReport isFunctionCall = SA_is_function_call(tokens, startPos + 1, false);

    if (isFunctionCall.errorOccured == true) {
        return SA_create_syntax_report(isFunctionCall.token, 0, true, isFunctionCall.expectedToken);
    }

    if ((*tokens)[startPos + isFunctionCall.tokensToSkip + 1].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isFunctionCall.tokensToSkip + 1], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, isFunctionCall.tokensToSkip + 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the CONTINUE rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_continue_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_CONTINUE_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "continue");
    }

    if ((*tokens)[startPos + 1].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the BREAK rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_break_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_BREAK_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "break");
    }

    if ((*tokens)[startPos + 1].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the FOR rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_for_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_FOR_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "for");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }
    
    SyntaxReport isVar = SA_is_variable(tokens, startPos + 2);

    if (isVar.errorOccured == true) {
        return SA_create_syntax_report(isVar.token, 0, true, isVar.expectedToken);
    }

    SyntaxReport isChainedCond = SA_is_chained_condition(tokens, startPos + isVar.tokensToSkip + 2, true);

    if (isChainedCond.errorOccured == true) {
        return SA_create_syntax_report(isChainedCond.token, 0, true, isChainedCond.expectedToken);
    }
    
    if ((*tokens)[startPos + isVar.tokensToSkip + isChainedCond.tokensToSkip + 2].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isVar.tokensToSkip + isChainedCond.tokensToSkip + 2], 0, true, ";");
    }

    int totalSkip = isVar.tokensToSkip + isChainedCond.tokensToSkip + 3;
    SyntaxReport isExpression = SA_is_expression(tokens, startPos + totalSkip, false);

    if (isExpression.errorOccured == true) {
        return SA_create_syntax_report(isExpression.token, 0, true, isExpression.expectedToken);
    }

    totalSkip += isExpression.tokensToSkip;

    if ((*tokens)[startPos + totalSkip].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + totalSkip], 0, true, ")");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + totalSkip + 1, true);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return SA_create_syntax_report(NULL, totalSkip + isRunnable.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches an expression
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_expression(TOKEN **tokens, size_t startPos, int inRunnable) {
    SyntaxReport isIdentifier = SA_is_identifier(tokens, startPos);

    if (isIdentifier.errorOccured == true) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, isIdentifier.expectedToken);
    }

    TOKEN *crucialToken = &(*tokens)[startPos + isIdentifier.tokensToSkip];
    int skip = 0;

    if ((int)SA_predict_is_conditional_variable_type(tokens, startPos + isIdentifier.tokensToSkip) == true) {
        SyntaxReport isConditionAssignment = SA_is_conditional_assignment(tokens, startPos + isIdentifier.tokensToSkip);

        if (isConditionAssignment.errorOccured == true) {
            return SA_create_syntax_report(isConditionAssignment.token, 0, true, isConditionAssignment.expectedToken);
        }

        skip = isConditionAssignment.tokensToSkip - 1;
    } else if (crucialToken->type == _OP_ADD_ONE_
        || crucialToken->type == _OP_SUBTRACT_ONE_) {
        skip = 1;
    } else if ((int)SA_is_assignment_operator(crucialToken->value) == true
        || crucialToken->type == _OP_EQUALS_) {
        SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + isIdentifier.tokensToSkip + 1, true);

        if (isSimpleTerm.errorOccured == true) {
            return SA_create_syntax_report(isSimpleTerm.token, 0, true, isSimpleTerm.expectedToken);
        }

        skip = isSimpleTerm.tokensToSkip + 1;
    } else {
        return SA_create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip], 0, true, "++\", \"--\", \"-=\", \"+=\", \"*=\", \"/=\" or \"=");
    }
    
    if (inRunnable == true) {
        if ((*tokens)[startPos + isIdentifier.tokensToSkip + skip].type == _OP_SEMICOLON_) {
            return SA_create_syntax_report(NULL, isIdentifier.tokensToSkip + skip + 1, false, NULL);
        }

        return SA_create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip + skip], 0, true, ";");
    }
    
    return SA_create_syntax_report(NULL, isIdentifier.tokensToSkip + skip, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the IF_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_if_statement(TOKEN **tokens, size_t startPos) { 
    if ((*tokens)[startPos].type != _KW_IF_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "if");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    SyntaxReport isChainedCond = SA_is_chained_condition(tokens, startPos + 2, true);

    if (isChainedCond.errorOccured == true) {
        return SA_create_syntax_report(isChainedCond.token, 0, true, isChainedCond.expectedToken);
    }

    if ((*tokens)[startPos + isChainedCond.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isChainedCond.tokensToSkip + 2], 0, true, ")");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + isChainedCond.tokensToSkip + 3, true);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + isChainedCond.tokensToSkip + 3, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the ELSE_IF_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_else_if_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_ELSE_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "else");
    }

    SyntaxReport isIfStatement = SA_is_if_statement(tokens, startPos + 1);

    if (isIfStatement.errorOccured == true) {
        return SA_create_syntax_report(isIfStatement.token, 0, true, isIfStatement.expectedToken);
    }

    return SA_create_syntax_report(NULL, isIfStatement.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the ELSE_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_else_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_ELSE_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "else");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + 1, true);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the DO_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_do_statment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_DO_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "do");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + 1, true);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    SyntaxReport isWhileCond = SA_is_while_condition(tokens, startPos + isRunnable.tokensToSkip + 1);

    if (isWhileCond.errorOccured == true) {
        return SA_create_syntax_report(isWhileCond.token, 0, true, isWhileCond.expectedToken);
    }

    if ((*tokens)[startPos + isWhileCond.tokensToSkip + isRunnable.tokensToSkip + 1].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isWhileCond.tokensToSkip + isRunnable.tokensToSkip + 1], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + isWhileCond.tokensToSkip + 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the WHILE_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_while_statement(TOKEN **tokens, size_t startPos) {
    SyntaxReport isWhileCond = SA_is_while_condition(tokens, startPos);

    if (isWhileCond.errorOccured == true) {
        return SA_create_syntax_report(isWhileCond.token, 0, true, isWhileCond.expectedToken);
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + isWhileCond.tokensToSkip, true);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return SA_create_syntax_report(NULL, isWhileCond.tokensToSkip + isRunnable.tokensToSkip, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the WHILE_CONDITION rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_while_condition(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_WHILE_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "while");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    SyntaxReport isChainedCondition = SA_is_chained_condition(tokens, startPos + 2, true);

    if (isChainedCondition.errorOccured == true) {
        return SA_create_syntax_report(isChainedCondition.token, 0, true, isChainedCondition.expectedToken);
    }

    if ((*tokens)[startPos + isChainedCondition.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isChainedCondition.tokensToSkip + 2], 0, true, ")");
    }

    return SA_create_syntax_report(NULL, isChainedCondition.tokensToSkip + 3, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the CHECK_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_check_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_CHECK_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "check");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    SyntaxReport isIdentifier = SA_is_identifier(tokens, startPos + 2);

    if (isIdentifier.errorOccured == true) {
        return SA_create_syntax_report(isIdentifier.token, 0, true, isIdentifier.expectedToken);
    }

    if ((*tokens)[startPos + isIdentifier.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip + 2], 0, true, ")");
    }

    if ((*tokens)[startPos + isIdentifier.tokensToSkip + 3].type != _OP_RIGHT_BRACE_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip + 3], 0, true, "{");
    }

    int jumper = isIdentifier.tokensToSkip + 4;

    while (startPos + jumper <  MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        if ((*tokens)[startPos + jumper].type == _OP_LEFT_BRACE_) {
            jumper++;
            break;
        }
        
        SyntaxReport isIsStatement = SA_is_is_statement(tokens, startPos + jumper);

        if (isIsStatement.errorOccured == true) {
            return SA_create_syntax_report(isIsStatement.token, 0, true, isIsStatement.expectedToken);
        }

        jumper += isIsStatement.tokensToSkip;
    }
   
    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the IS_STATEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_is_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_IS_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "is");
    }

    TOKEN *crucialToken = &(*tokens)[startPos + 1];
    int skip = 0;

    if (crucialToken->value[0] == '\"') {
        if ((int)SA_is_string(crucialToken) == true) {
            skip = 1;
        } else {
            return SA_create_syntax_report(crucialToken, 0, true, "<STRING>");
        }
    } else if ((int)is_digit(crucialToken->value[0]) == true) {
        SyntaxReport isNumeralIdentifier = SA_is_numeral_identifier(crucialToken);

        if (isNumeralIdentifier.errorOccured == true) {
            return SA_create_syntax_report(crucialToken, 0, true, "<NUMBER>");
        }

        skip = isNumeralIdentifier.tokensToSkip;
    } else if ((int)SA_is_letter(crucialToken->value[0]) == true) {
        //THIS OPTION IS ONLY FOR CONSTANTS
        if ((int)SA_is_root_identifier(crucialToken) == false) {
            return SA_create_syntax_report(crucialToken, 0, true, "<CONSTANT>");
        }

        skip = 1;
    }

    if ((*tokens)[startPos + skip + 1].type != _OP_COLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip + 1], 0, true, ":");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + skip + 2, 2);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + skip + 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the VARIABLE rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_variable(TOKEN **tokens, size_t startPos) {
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
        if ((int)SA_is_root_identifier(&(*tokens)[startPos + modifier + 1]) == false) {
            return SA_create_syntax_report(&(*tokens)[startPos + modifier + 1], 0, true, "<IDENTIFIER>");
        }

        TOKEN *crucialToken = &(*tokens)[startPos + modifier + 2];
        int skip = 0;

        if (crucialToken->type == _OP_RIGHT_EDGE_BRACKET_) {
            SyntaxReport isArrayVar = SA_is_array_variable(tokens, startPos + modifier + 2);

            if (isArrayVar.errorOccured == true) {
                return SA_create_syntax_report(isArrayVar.token, 0, true, isArrayVar.expectedToken);
            } else {
                skip = isArrayVar.tokensToSkip;
            }
        } else if (crucialToken->type == _OP_SEMICOLON_) {
            return SA_create_syntax_report(NULL, 3 + modifier, false, NULL);
        } else if (crucialToken->type == _OP_COMMA_) {
            SyntaxReport isMultipleVarDef = SA_is_multiple_variable_definition(tokens, startPos + modifier + 1);

            if (isMultipleVarDef.errorOccured == true) {
                return SA_create_syntax_report(isMultipleVarDef.token, 0, true, isMultipleVarDef.expectedToken);
            } else {
                //-1, because the identifier get's checked again in the is_multiple... function
                skip = isMultipleVarDef.tokensToSkip - 1;
            }
        } else if (crucialToken->type == _OP_EQUALS_) {
            if ((int)SA_predict_is_conditional_variable_type(tokens, startPos + modifier + 2) == true) {
                SyntaxReport isCondAssignment = SA_is_conditional_assignment(tokens, startPos + modifier + 2);

                if (isCondAssignment.errorOccured == true) {
                    return SA_create_syntax_report(isCondAssignment.token, 0, true, isCondAssignment.expectedToken);
                } else {
                    skip = isCondAssignment.tokensToSkip;
                }
            } else if ((*tokens)[startPos + modifier + 3].type == _KW_NEW_) {
                SyntaxReport isClassInstance = SA_is_class_instance(tokens, startPos + modifier + 4);

                if (isClassInstance.errorOccured == true) {
                    return SA_create_syntax_report(isClassInstance.token, 0, true, isClassInstance.expectedToken);
                } else {
                    skip = isClassInstance.tokensToSkip + 3;
                }
            } else {
                SyntaxReport isAssignment = SA_is_assignment(tokens, startPos + modifier + 2);

                if (isAssignment.errorOccured == true) {
                    return SA_create_syntax_report(isAssignment.token, 0, true, isAssignment.expectedToken);
                } else {
                    skip = isAssignment.tokensToSkip;
                }
            }
        } else {
            return SA_create_syntax_report(crucialToken, 0, true, "[\", \";\", \",\", \"=\" or \"<IDENTIFIER>");
        }
        
        if ((*tokens)[startPos + skip + modifier + 1].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos + skip + modifier + 1], 0, true, ";");
        }

        return SA_create_syntax_report(NULL, skip + modifier + 2, false, NULL);
    } else if ((*tokens)[startPos + modifier].type == _KW_CONST_) {
        if ((int)SA_is_root_identifier(&(*tokens)[startPos + modifier + 1]) == true) {
            if ((*tokens)[startPos + modifier + 3].type != _KW_NEW_) {
                SyntaxReport isAssignment = SA_is_assignment(tokens, startPos + modifier + 2);

                if (isAssignment.errorOccured == true) {
                    return SA_create_syntax_report(isAssignment.token, 0, true, isAssignment.expectedToken);
                } else {
                    if ((*tokens)[startPos + modifier + isAssignment.tokensToSkip + 1].type != _OP_SEMICOLON_) {
                        return SA_create_syntax_report(&(*tokens)[startPos + modifier + isAssignment.tokensToSkip + 1], 0, true, ";");
                    } else {
                        return SA_create_syntax_report(NULL, isAssignment.tokensToSkip + modifier + 2, false, NULL);
                    }
                }
            } else {
                SyntaxReport isClassInstance = SA_is_class_instance(tokens, startPos + modifier + 4);

                if (isClassInstance.errorOccured == true) {
                    return SA_create_syntax_report(isClassInstance.token, 0, true, isClassInstance.expectedToken);
                } else {
                    if ((*tokens)[startPos + modifier + isClassInstance.tokensToSkip + 4].type != _OP_SEMICOLON_) {
                        return SA_create_syntax_report(&(*tokens)[startPos + modifier + isClassInstance.tokensToSkip + 4], 0, true, ";");
                    } else {
                        return SA_create_syntax_report(NULL, isClassInstance.tokensToSkip + modifier + 5, false, NULL);
                    }
                }
            }
        } else {
            return SA_create_syntax_report(&(*tokens)[startPos + modifier + 1], 0, true, "<IDENTIFIER>");
        }
    }
    
    return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "const\" or \"var");
}

/*
Purpose: Checks if the current TOKEN array is a class instance
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_class_instance(TOKEN **tokens, size_t startPos) {
    if ((int)SA_is_root_identifier(&(*tokens)[startPos]) == false) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "<IDENTIFIER>");    
    }

    int skip = 1;

    if ((*tokens)[startPos + skip].type == _OP_RIGHT_BRACKET_) {
        SyntaxReport isParam = SA_is_parameter(tokens, startPos + skip + 1, _PARAM_FUNCTION_CALL_);

        if (isParam.errorOccured == true) {
            return SA_create_syntax_report(isParam.token, 0, true, isParam.expectedToken);
        }

        skip += isParam.tokensToSkip + 1;

        if ((*tokens)[startPos + skip].type != _OP_LEFT_BRACKET_) {
            return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ")");
        }

        skip++;
    } else if ((*tokens)[startPos + skip].type == _OP_RIGHT_EDGE_BRACKET_) {
        while (startPos + skip < MAX_TOKEN_LENGTH) {
            SyntaxReport isArray = SA_is_array_element(tokens, startPos + skip);

            if (isArray.errorOccured == true) {
                if ((*tokens)[startPos + skip].type != _OP_SEMICOLON_) {
                    return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "<ARRAY_ELEMENT>");
                }

                break;
            }

            skip += isArray.tokensToSkip;
        }
    } else {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "(\", \"[");
    }
    
    return SA_create_syntax_report(NULL, skip, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is an ASSIGNMENT
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_assignment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_EQUALS_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "=");
    }

    SyntaxReport isTerm = SA_is_simple_term(tokens, startPos + 1, false);

    if (isTerm.errorOccured == true) {
        return SA_create_syntax_report(isTerm.token, 0, true, isTerm.expectedToken);
    }

    if ((*tokens)[startPos + isTerm.tokensToSkip + 1].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isTerm.tokensToSkip + 1], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, isTerm.tokensToSkip + 2, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is a CONDITIONAL_ASSIGNMENT
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_conditional_assignment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_EQUALS_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "=");
    }

    SyntaxReport isChainedCondition = SA_is_chained_condition(tokens, startPos + 1, false);

    if (isChainedCondition.errorOccured == true) {
        return SA_create_syntax_report(isChainedCondition.token, 0, true, isChainedCondition.expectedToken);
    }

    if ((*tokens)[startPos + isChainedCondition.tokensToSkip + 1].type != _OP_QUESTION_MARK_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isChainedCondition.tokensToSkip + 1], 0, true, "?");
    }

    SyntaxReport leftTerm = SA_is_simple_term(tokens, startPos + isChainedCondition.tokensToSkip + 2, false);

    if (leftTerm.errorOccured == true) {
        return SA_create_syntax_report(leftTerm.token, 0, true, leftTerm.expectedToken);
    }

    int totalSkip = isChainedCondition.tokensToSkip + leftTerm.tokensToSkip;

    if ((*tokens)[startPos + totalSkip + 2].type != _OP_COLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + totalSkip + 2], 0, true, ":");
    }

    SyntaxReport rightTerm = SA_is_simple_term(tokens, startPos + totalSkip + 3, false);

    if (rightTerm.errorOccured == true) {
        return SA_create_syntax_report(rightTerm.token, 0, true, rightTerm.expectedToken);
    }

    totalSkip += rightTerm.tokensToSkip;

    if ((*tokens)[startPos + totalSkip + 3].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + totalSkip + 3], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, totalSkip + 4, false, NULL);
}

SyntaxReport SA_is_simple_conditional_assignment(TOKEN **tokens, size_t startPos) {
    SyntaxReport isChainedCondition = SA_is_chained_condition(tokens, startPos, false);

    if (isChainedCondition.errorOccured == true) {
        return SA_create_syntax_report(isChainedCondition.token, 0, true, isChainedCondition.expectedToken);
    }

    if ((*tokens)[startPos + isChainedCondition.tokensToSkip].type != _OP_QUESTION_MARK_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isChainedCondition.tokensToSkip], 0, true, "?");
    }

    SyntaxReport leftTerm = SA_is_simple_term(tokens, startPos + isChainedCondition.tokensToSkip + 1, false);

    if (leftTerm.errorOccured == true) {
        return SA_create_syntax_report(leftTerm.token, 0, true, leftTerm.expectedToken);
    }

    int totalSkip = isChainedCondition.tokensToSkip + leftTerm.tokensToSkip;

    if ((*tokens)[startPos + totalSkip + 1].type != _OP_COLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + totalSkip + 1], 0, true, ":");
    }

    SyntaxReport rightTerm = SA_is_simple_term(tokens, startPos + totalSkip + 2, false);

    if (rightTerm.errorOccured == true) {
        return SA_create_syntax_report(rightTerm.token, 0, true, rightTerm.expectedToken);
    }

    totalSkip += rightTerm.tokensToSkip;

    if ((*tokens)[startPos + totalSkip + 2].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + totalSkip + 2], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, totalSkip + 3, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is a chained condition (condition with "and" and "or")
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int inParam => Flag if chainedCondition is in parameter
*/
SyntaxReport SA_is_chained_condition(TOKEN **tokens, size_t startPos, int inParam) {
    int jumper = 0;
    int openBrackets = 1;
    int hasToBeLogicOperator = false;
    
    while (startPos + jumper <  MAX_TOKEN_LENGTH
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
        } else if ((int)SA_is_end_indicator(currentToken) == true
            && currentToken->type != _KW_AND_
            && currentToken->type != _KW_OR_) {
            break;
        }

        switch (hasToBeLogicOperator) {
        case false:
            hasToBeLogicOperator = true;

            SyntaxReport isCondition = SA_is_condition(tokens, startPos + jumper, inParam);

            if (isCondition.errorOccured == true) {
                return SA_create_syntax_report(isCondition.token, 0, true, isCondition.expectedToken);
            }

            jumper += isCondition.tokensToSkip;
            break;
        case true:
            if (currentToken->type != _KW_AND_
                && currentToken->type != _KW_OR_) {
                return SA_create_syntax_report(currentToken, 0, true, "and\" or \"or");
            }

            hasToBeLogicOperator = false;
            jumper++;
            break;
        }
    }

    if (jumper == 0) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<CHAINED_CONDITION>");
    } else if (hasToBeLogicOperator == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is a condition
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        int inParam => Flag if the condition is delivered as parameter
*/
SyntaxReport SA_is_condition(TOKEN **tokens, size_t startPos, int inParam) {
    if ((int)SA_is_bool((*tokens)[startPos].value) == false) {
        SyntaxReport leftTerm = SA_is_simple_term(tokens, startPos, false);

        if (leftTerm.errorOccured == true) {
            return SA_create_syntax_report(leftTerm.token, 0, true, leftTerm.expectedToken);
        }

        char *sequence = (*tokens)[startPos + leftTerm.tokensToSkip].value;

        if ((int)SA_is_rational_operator(sequence) == false) {
            return SA_create_syntax_report(&(*tokens)[startPos + leftTerm.tokensToSkip], 0, true, "==\", \"<=\", \">=\", \"!=\", \"<\" or \">");
        }

        SyntaxReport rightTerm = SA_is_simple_term(tokens, startPos + leftTerm.tokensToSkip + 1, inParam);

        if (rightTerm.errorOccured == true) {
            return SA_create_syntax_report(rightTerm.token, 0, true, rightTerm.expectedToken);
        }

        return SA_create_syntax_report(NULL, leftTerm.tokensToSkip + rightTerm.tokensToSkip + 1, false, NULL);
    }

    return SA_create_syntax_report(NULL, 1, false, NULL);
}

/*
Purpose: Predict whether the next var option is a conditional assignment or not
Return Type: int => true = is conditional assignment; false = not a conditional assignment
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
int SA_predict_is_conditional_variable_type(TOKEN **tokens, size_t startPos) {
    int facedSemicolon = false;

    for (int i = startPos; i <  MAX_TOKEN_LENGTH; i++) {
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
SyntaxReport SA_is_multiple_variable_definition(TOKEN **tokens, size_t startPos) {
    SyntaxReport isMultiVarIdentifer = SA_is_multiple_variable_definition_identifier(tokens, startPos);

    if (isMultiVarIdentifer.errorOccured == true) {
        return SA_create_syntax_report(isMultiVarIdentifer.token, 0, true, isMultiVarIdentifer.expectedToken);
    }

    if ((*tokens)[startPos + isMultiVarIdentifer.tokensToSkip].type == _OP_EQUALS_) {
        SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + isMultiVarIdentifer.tokensToSkip + 1, false);
        int totalSkip = startPos + isSimpleTerm.tokensToSkip + isMultiVarIdentifer.tokensToSkip;

        if (isSimpleTerm.errorOccured == true) {
            return SA_create_syntax_report(&(*tokens)[totalSkip], 0, true, isSimpleTerm.expectedToken);
        }

        if ((*tokens)[totalSkip + 1].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[totalSkip + 1], 0, true, ";");
        }

        return SA_create_syntax_report(NULL, totalSkip + 1, false, NULL);
    } else {
        if ((*tokens)[startPos + isMultiVarIdentifer.tokensToSkip].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos + isMultiVarIdentifer.tokensToSkip], 0, true, ";");
        }
    }

    return SA_create_syntax_report(NULL, isMultiVarIdentifer.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is a multiple variable identifier
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
Layout: <IDENTIIFER>, <IDENTIFIER>, [...]
*/
SyntaxReport SA_is_multiple_variable_definition_identifier(TOKEN **tokens, size_t startPos) {
    int hasToBeComma = false;
    int jumper = 0;

    while (startPos + jumper <  MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];
        
        if ((int)SA_is_end_indicator(currentToken) == true
            && currentToken->type != _OP_COMMA_) {
            break;
        }

        switch (hasToBeComma) {
        case false:
            hasToBeComma = true;

            if ((int)SA_is_root_identifier(currentToken) == false) {
                return SA_create_syntax_report(currentToken, 0, true, "<IDENTIFIER>");
            }

            jumper++;
            break;
        case true:
            if (currentToken->type != _OP_COMMA_) {
                return SA_create_syntax_report(currentToken, 0, true, ",");
            }

            jumper++;
            hasToBeComma = false;
            break;
        }
    }

    if (jumper <= 1) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<MULTIPLE_DEFINITION>");
    } else if (hasToBeComma == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is an array variable
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_array_variable(TOKEN **tokens, size_t startPos) {
    SyntaxReport isArrayElement = SA_is_array_element(tokens, startPos);

    if (isArrayElement.errorOccured == true) {
        return SA_create_syntax_report(isArrayElement.token, 0, true, isArrayElement.expectedToken);
    }
    
    if ((*tokens)[startPos + isArrayElement.tokensToSkip].type == _OP_EQUALS_) {
        SyntaxReport isArrayAssignment = SA_is_array_assignment(tokens, startPos + isArrayElement.tokensToSkip);
        
        if (isArrayAssignment.errorOccured == true) {
            return SA_create_syntax_report(isArrayAssignment.token, 0, true, isArrayAssignment.expectedToken);
        }
        
        if ((*tokens)[startPos + isArrayElement.tokensToSkip + isArrayAssignment.tokensToSkip].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos + isArrayElement.tokensToSkip + isArrayAssignment.tokensToSkip], 0, true, ";");
        }
        
        return SA_create_syntax_report(NULL, isArrayElement.tokensToSkip + isArrayAssignment.tokensToSkip + 1, false, NULL);
    }

    if ((*tokens)[startPos + isArrayElement.tokensToSkip].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isArrayElement.tokensToSkip], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, isArrayElement.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array is an array assignment
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_array_assignment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_EQUALS_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "=");
    }

    int jumper = 1;
    int openBraces = 0;
    int hasToBeComma = false;

    while (startPos + jumper <  MAX_TOKEN_LENGTH
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
        } else if ((int)SA_is_end_indicator(currentToken) == true
            && currentToken->type != _OP_COMMA_) {
            break;
        }

        switch (hasToBeComma) {
        case false: {
            SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + jumper, false);

            if (isSimpleTerm.errorOccured == true
                || isSimpleTerm.tokensToSkip == 0) {
                return SA_create_syntax_report(isSimpleTerm.token, 0, true, isSimpleTerm.expectedToken);
            }
            
            jumper += isSimpleTerm.tokensToSkip;
            hasToBeComma = true;
            break;
        }
        case true:
            if (currentToken->type != _OP_COMMA_) {
                return SA_create_syntax_report(currentToken, 0, true, ",");
            }

            jumper++;
            hasToBeComma = false;
           break;
        }
    }

    if (hasToBeComma == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    } else if (openBraces != 0) {
        if (openBraces > 0) {
            return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "}");
        } else {
            return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "{");
        }
    } else if (jumper <= 1) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<ARRAY_ASSIGNMENT>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the ARRAY_ELEMENT rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_array_element(TOKEN **tokens, size_t startPos) {
    int hasToBeClosingEdgeBracket = false;
    int jumper = 0;

    while (startPos + jumper <  MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if ((int)SA_is_end_indicator(currentToken)
            && currentToken->type != _OP_LEFT_EDGE_BRACKET_) {
            break;
        }

        switch (hasToBeClosingEdgeBracket) {
        case false:
            hasToBeClosingEdgeBracket = true;

            if (currentToken->type != _OP_RIGHT_EDGE_BRACKET_) {
                return SA_create_syntax_report(currentToken, 0, true, "[");
            }
            
            if ((*tokens)[startPos + jumper + 1].type != _OP_LEFT_EDGE_BRACKET_) {
                SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + jumper + 1, false);

                if (isSimpleTerm.errorOccured == true) {
                    return SA_create_syntax_report(isSimpleTerm.token, 0, true, isSimpleTerm.expectedToken);
                }

                jumper += isSimpleTerm.tokensToSkip + 1;
            } else {
                jumper++;
            }

            break;
        case true:
            if (currentToken->type != _OP_LEFT_EDGE_BRACKET_) {
                return SA_create_syntax_report(currentToken, 0, true, "]");
            }

            jumper++;
            hasToBeClosingEdgeBracket = false;
            break;
        }
    }

    if (hasToBeClosingEdgeBracket == true) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "]");
    } else if (jumper == 0) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<ARRAY_ELEMENT>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the CLASS_CONSTRUCTOR rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_class_constructor(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_THIS_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "this");
    }

    if ((*tokens)[startPos + 1].type != _OP_COLON_
        && (*tokens)[startPos + 2].type != _OP_COLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "::");
    }

    if ((*tokens)[startPos + 3].type != _KW_CONSTRUCTOR_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 3], 0, true, "constructor");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + 4, 1);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + 4, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the CLASS rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_class(TOKEN **tokens, size_t startPos) {
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
        return SA_create_syntax_report(&(*tokens)[startPos + modifier], 0, true, "class");
    }

    SyntaxReport isFunctionCall = SA_is_function_call(tokens, startPos + modifier + 1, true);

    if (isFunctionCall.errorOccured == true) {
        return SA_create_syntax_report(isFunctionCall.token, 0, true, isFunctionCall.expectedToken);
    }

    int additionalWithSkip = 0;

    if ((*tokens)[startPos + isFunctionCall.tokensToSkip + modifier + 1].type == _KW_WITH_) {
        SyntaxReport isWith = SA_is_with_statement(tokens, startPos + isFunctionCall.tokensToSkip + modifier + 1);

        if (isWith.errorOccured == true) {
            return SA_create_syntax_report(isWith.token, 0, true, isWith.expectedToken);
        } else {
            additionalWithSkip = isWith.tokensToSkip;
        }
    }

    int totalSkip = isFunctionCall.tokensToSkip + modifier + additionalWithSkip + 1;

    if ((*tokens)[startPos + totalSkip].type != _OP_CLASS_CREATOR_) {
        return SA_create_syntax_report(&(*tokens)[startPos + totalSkip], 0, true, "=>");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + totalSkip + 1, true);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return SA_create_syntax_report(NULL, totalSkip + isRunnable.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if the current TOKEN array matches the WITH rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_with_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_WITH_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "with");
    }
    
    SyntaxReport isParameter = SA_is_parameter(tokens, startPos + 1, _PARAM_CLASS_);

    if (isParameter.errorOccured == true) {
        return SA_create_syntax_report(isParameter.token, 0, true, isParameter.expectedToken);
    }
    
    return SA_create_syntax_report(NULL, isParameter.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Check if a TOKEN array is written accordingly to the TRY rule
Return Type: SyntaxReport => ErrorOccured = true, if error appears, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_try_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_TRY_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "try");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + 1, true);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(isRunnable.token, 0, true, isRunnable.expectedToken);
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Check if a TOKEN array is written accordingly to the CATCH rule
Return Type: SyntaxReport => ErrorOccured = true, if error appears, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_catch_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_CATCH_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "catch");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    if ((int)SA_is_root_identifier(&(*tokens)[startPos + 2]) == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + 2], 0, true, "<IDENTIFIER>");
    }

    if ((int)SA_is_root_identifier(&(*tokens)[startPos + 3]) == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + 3], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 4].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 4], 0, true, ")");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + 5, true);

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + 5, false, isRunnable.expectedToken);
}

/*
Purpose: Check if a TOKEN array is written accordingly to the EXPORT rule
Return Type: SyntaxReport => ErrorOccured = true, if error appears, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_export(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_EXPORT_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "export");
    }

    if ((int)SA_is_string(&(*tokens)[startPos + 1]) == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "<STRING>");
    }

    if ((*tokens)[startPos + 2].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 2], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, 3, false, NULL);
}
/*
Purpose: Check if a TOKEN array is written accordingly to the INCLUDE rule
Return Type: SyntaxReport => ErrorOccured = true, if error appears, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_include(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_INCLUDE_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "include");
    }

    if ((int)SA_is_string(&(*tokens)[startPos + 1]) == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "<STRING>");
    }

    if ((*tokens)[startPos + 2].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 2], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, 3, false, NULL);
}

/*
Purpose: Checks if a given TOKEN array is an ENUM or not
Return Type: SyntaxReport => ErrorOccured = true, when something is not written
                            accordingly to the rule, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_enum(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_ENUM_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "enum");
    }

    if ((int)SA_is_root_identifier(&(*tokens)[startPos + 1]) == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 2].type != _OP_RIGHT_BRACE_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 2], 0, true, "{");
    }

    SyntaxReport isEnumerator = SA_is_enumerator(tokens, startPos + 3);
    
    if (isEnumerator.errorOccured == true) {
        return SA_create_syntax_report(isEnumerator.token, 0, true, isEnumerator.expectedToken);
    }

    if ((*tokens)[startPos + isEnumerator.tokensToSkip + 3].type != _OP_LEFT_BRACE_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isEnumerator.tokensToSkip + 3], 0, true, "}");
    }
    
    return SA_create_syntax_report(NULL, isEnumerator.tokensToSkip + 4, false, NULL);
}

/*
Purpose: Checks if a given TOKEN array is an enumerator or not
Return Type: SyntaxReport => ErrorOccured = true, when something is not written
                            accordingly to the rule, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_enumerator(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int hasToBeComma = false;

    while (startPos + jumper <  MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__
        && (*tokens)[startPos + jumper].type != _OP_LEFT_BRACE_) {
        switch (hasToBeComma) {
        case false:
            hasToBeComma = true;

            if ((int)SA_is_letter((*tokens)[startPos + jumper].value[0]) == true) {
                int isRootIdentifier = (int)SA_is_root_identifier(&(*tokens)[startPos + jumper]);
                
                if ((*tokens)[startPos + jumper + 1].type == _OP_COLON_) {
                    SyntaxReport isNumeralIdentifier = SA_is_numeral_identifier(&(*tokens)[startPos + jumper + 2]);

                    if (isNumeralIdentifier.errorOccured == false) {
                        jumper += 3;
                    } else {
                        return SA_create_syntax_report(&(*tokens)[startPos + jumper + 2], 0, true, "<NUMBER>");
                    }
                } else {
                    jumper += isRootIdentifier;
                }
            } else {
                return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
            }

            break;
        case true:
            if ((*tokens)[startPos + jumper].type != _OP_COMMA_) {
                return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, ",");
            }

            jumper++;
            hasToBeComma = false;
            break;
        }
    }

    if (hasToBeComma == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Check if a TOKEN array is matching the FUNCTION rule
Return Type: SyntaxReport => ErrorOccured = false if everything works fine, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_function(TOKEN **tokens, size_t startPos) {
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
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "function");
    }

    if ((*tokens)[startPos + skip + 1].type == _OP_COLON_) {
        SyntaxReport isReturnSpecifier = SA_is_function_return_type_specifier(tokens, startPos + skip + 1);

        if (isReturnSpecifier.errorOccured == true) {
            return SA_create_syntax_report(isReturnSpecifier.token, 0, true, isReturnSpecifier.expectedToken);
        }

        skip += isReturnSpecifier.tokensToSkip;
    }

    SyntaxReport isFunctionCall = SA_is_function_call(tokens, startPos + skip + 1, true);

    if (isFunctionCall.errorOccured == true) {
        return SA_create_syntax_report(isFunctionCall.token, 0, true, isFunctionCall.expectedToken);
    }
    
    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + skip + isFunctionCall.tokensToSkip + 1, true);

    if (isRunnable.errorOccured == true) {
        return SA_create_syntax_report(&(*tokens)[startPos + isFunctionCall.tokensToSkip + isRunnable.tokensToSkip + skip + 1], 0, true, isRunnable.expectedToken);
    }

    return SA_create_syntax_report(NULL, skip + isFunctionCall.tokensToSkip + isRunnable.tokensToSkip + 1, false, NULL);
}

/*
Purpose: Checks if a TOKEN array is a return type definition for a function
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking;
        int inFunctionCall => Flag if the functioncall is independent or from a function
*/
SyntaxReport SA_is_function_return_type_specifier(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_COLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, ":");
    }

    if ((int)SA_is_root_identifier(&(*tokens)[startPos + 1]) == false
        && (*tokens)[startPos + 1].type != _KW_INT_
        && (*tokens)[startPos + 1].type != _KW_CHAR_
        && (*tokens)[startPos + 1].type != _KW_BOOLEAN_
        && (*tokens)[startPos + 1].type != _KW_FLOAT_
        && (*tokens)[startPos + 1].type != _KW_DOUBLE_
        && (*tokens)[startPos + 1].type != _KW_STRING_
        && (*tokens)[startPos + 1].type != _KW_SHORT_
        && (*tokens)[startPos + 1].type != _KW_LONG_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 2].type == _OP_RIGHT_EDGE_BRACKET_) {
        if ((*tokens)[startPos + 3].type != _OP_LEFT_EDGE_BRACKET_) {
            return SA_create_syntax_report(&(*tokens)[startPos + 3], 0, true, "]");
        }

        return SA_create_syntax_report(NULL, 4, false, NULL);
    }

    return SA_create_syntax_report(NULL, 2, false, NULL);
}

/*
Purpose: Checks if a TOKEN array is written accordingly to the FUNCTION CALL rule
Return Type: SyntaxReport => ErrorOccured = true on error, else false
Params: TOKEN **tokens => Token array to be checked;
        size_t startPos => Position from where to start checking;
        int inFunctionCall => Flag if the functioncall is independent or from a function
*/
SyntaxReport SA_is_function_call(TOKEN **tokens, size_t startPos, int inFunction) {
    int isRootIdentifier = (int)SA_is_root_identifier(&(*tokens)[startPos]);

    if (isRootIdentifier == false) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }
    
    SyntaxReport isParameter;

    if (inFunction == true) {
        isParameter = SA_is_parameter(tokens, startPos + 2, _PARAM_FUNCTION_);
    } else {
        isParameter = SA_is_parameter(tokens, startPos + 2, _PARAM_FUNCTION_CALL_);
    }
    
    if (isParameter.errorOccured == true) {
        return SA_create_syntax_report(isParameter.token, 0, true, isParameter.expectedToken);
    }

    if ((*tokens)[startPos + isParameter.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isParameter.tokensToSkip + 2], 0, true, ")");
    }
    
    return SA_create_syntax_report(NULL, isRootIdentifier + isParameter.tokensToSkip + 2, false, NULL);
}

/*
Purpose: Check if a Token array is a parameter or not
Return Type: SyntaxReport => true if it is a parameter, else false
Params: TOKEN **tokens => Tokens to be checked;
        size_t startPos => Position from where to start checking;
        enum ParameterType type => Determines the type of the parameter
*/
SyntaxReport SA_is_parameter(TOKEN **tokens, size_t startPos, enum ParameterType type) {
    int jumper = 0;
    int hasToBeComma = false;

    while (startPos + jumper <  MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken == NULL) {
            (void)SYNTAX_ANALYSIS_TOKEN_NULL_EXCEPTION();
        } else if ((int)SA_is_end_indicator(currentToken) == true
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
                    if ((int)SA_is_pointer(currentToken) == true) {
                        jumper++;
                    } else {
                        return SA_create_syntax_report(currentToken, 0, true, "<POINTER>");
                    }
                //Layout: &<IDENTIFIER> or &(*<IDENTIFIER>)
                } else if (currentToken->value[0] == '&') {
                    if ((int)SA_is_reference(currentToken) == true) {
                        jumper++;
                    } else {
                        return SA_create_syntax_report(currentToken, 0, true, "<REFERENCE>");
                    }
                //Layout: <IDENTIFIER> or <NUMBER> or <SIMPLE_TERM>
                } else {
                    SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + jumper, 1);

                    if (isSimpleTerm.errorOccured == false) {
                        jumper += isSimpleTerm.tokensToSkip;
                    } else {
                        return SA_create_syntax_report(currentToken, 0, true, isSimpleTerm.expectedToken);
                    }
                }

                break;
            case _PARAM_CLASS_:
            case _PARAM_FUNCTION_:
                if (currentToken->value[0] == '*') {
                    if ((int)SA_is_pointer(currentToken) == true) {
                        jumper++;
                    } else {
                        return SA_create_syntax_report(currentToken, 0, true, "<POINTER>");
                    }
                } else if ((int)SA_is_letter(currentToken->value[0]) == true) {
                    if ((int)SA_is_root_identifier(currentToken) == true) {
                        jumper++;
                    } else {
                        return SA_create_syntax_report(currentToken, 0, true, "<IDENTIFIER>");
                    }
                }

                break;
            }

            break;
        case true:
            if (currentToken->type != _OP_COMMA_) {
                return SA_create_syntax_report(currentToken, 0, true, ",");
            }

            jumper++;
            hasToBeComma = false;
            break;
        }
    }

    if (hasToBeComma == false
        && (*tokens)[startPos + jumper - 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, ")");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Check if a TOKEN array is written accordingly to the simple term rule
Return Type: SyntaxReport => Contains how many tokens to skip if TOKEN array match
                            the SIMPLE_TERM rule, else errorOccured = true
Params: TOKEN **tokens => TOKEN array to be checked;
        size_t startPos => Position from where to start checking;
        int inParameter => Determines if the simple term is called from a parameter
*/
SyntaxReport SA_is_simple_term(TOKEN **tokens, size_t startPos, int inParameter) {
    int openBrackets = 0;
    int jumper = 0;
    int hasToBeArithmeticOperator = false;

    while (startPos + jumper <  MAX_TOKEN_LENGTH
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
            //If an operator is before the Bracket
            } else if (hasToBeArithmeticOperator == false) {
                return SA_create_syntax_report(&(*tokens)[startPos + jumper - 1], 0, true, ")");
            }

            openBrackets--;
            jumper++;
            continue;
        } else if (currentToken->type == _OP_NOT_
            && hasToBeArithmeticOperator == false) {
            jumper++;
            continue;
        } else if ((int)SA_is_end_indicator(currentToken) == true) {
            break;
        }

        switch (hasToBeArithmeticOperator) {
        case false:
            hasToBeArithmeticOperator = true;

            if (currentToken->value[0] == '\"') {
                if ((int)SA_is_string(currentToken) == true) {
                    jumper++;
                    continue;
                } else {
                    return SA_create_syntax_report(currentToken, 0, true, "<STRING>");
                }
            }

            SyntaxReport isIdentifier = SA_create_syntax_report(NULL, 0, true, "[LETTER]\", \"[DIGIT]\", \"[FUNCTION_CALL]\" or \"[CLASS_OBJECT_ACCESS]");

            if ((int)SA_is_letter(currentToken->value[0]) == true) {
                if ((int)SA_predict_class_object_access(tokens, startPos + jumper) == true) {
                    isIdentifier = SA_is_class_object_access(tokens, startPos + jumper, false);
                } else if ((*tokens)[startPos + jumper + 1].type == _OP_RIGHT_BRACKET_) {
                    isIdentifier = SA_is_function_call(tokens, startPos + jumper, false);
                } else if ((int)SA_is_bool((*tokens)[startPos].value) == true) {
                    jumper++;
                    continue;
                } else if (currentToken->type == _KW_NULL_) {
                    jumper++;
                    continue;
                } else {
                    isIdentifier = SA_is_identifier(tokens, startPos + jumper);
                }
            } else if ((int)is_digit(currentToken->value[0]) == true) {
                isIdentifier = SA_is_numeral_identifier(currentToken);
            }

            if (isIdentifier.errorOccured == false) {
                jumper += isIdentifier.tokensToSkip;
            } else {
                return SA_create_syntax_report(currentToken, 0, true, isIdentifier.expectedToken);
            }

            break;
        case true:
            if ((int)SA_is_arithmetic_operator(currentToken) != true
                && currentToken->type != _OP_MODULU_) {
                return SA_create_syntax_report(currentToken, 0, true, "+\", \"-\", \"*\", \"%\" or \"/");
            }

            jumper++;
            hasToBeArithmeticOperator = false;
            break;
        }
    }

    if (hasToBeArithmeticOperator == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    } else if (openBrackets != 0) {
        if (openBrackets > 0) {
            return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, ")");
        } else {
            return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "(");
        }
    } else if (jumper == 0) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFER>\", \"<FUNCTION_CALL>\" or \"<CLASS_OBJECT_ACCESS>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Check if the next tokens are an evidence for an object access out of a class
Return Type: int => true = is object access; false = not an object access
Params: TOKEN **tokens => Tokens to be checked for evidence;
        size_t startPos => Position from where to start checking
*/
int SA_predict_class_object_access(TOKEN **tokens, size_t startPos) {
    int facedSemicolon = false;
    int jumper = 0;
    int openBrackets = 0;

    while (startPos + jumper <  MAX_TOKEN_LENGTH
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
SyntaxReport SA_is_identifier(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int hasToBeDot = false;

    while (startPos + jumper < MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if ((int)SA_is_end_indicator(currentToken) == true
            || (int)SA_is_arithmetic_operator(currentToken) == true) {
            break;
        }

        switch (hasToBeDot) {
        case false: {
            hasToBeDot = true;
            int isRootIdentifier = SA_is_root_identifier(currentToken);

            if (isRootIdentifier == true) {
                if ((*tokens)[startPos + jumper + 1].type == _OP_RIGHT_EDGE_BRACKET_) {
                    SyntaxReport isArrayIdentifier = SA_is_array_identifier(tokens, startPos + jumper + 1);

                    if (isArrayIdentifier.errorOccured == false) {
                        jumper += isArrayIdentifier.tokensToSkip + isRootIdentifier;
                        continue;
                    } else {
                        return SA_create_syntax_report(isArrayIdentifier.token, 0, true, isArrayIdentifier.expectedToken);
                    }
                } else if ((*tokens)[startPos + jumper + 1].type == _OP_RIGHT_BRACKET_) {
                    SyntaxReport isFunctionCall = SA_is_function_call(tokens, startPos + jumper, false);

                    if (isFunctionCall.errorOccured == false) {
                        jumper += isFunctionCall.tokensToSkip;
                        continue;
                    } else {
                        return SA_create_syntax_report(isFunctionCall.token, 0, true, isFunctionCall.expectedToken);
                    }
                }

                jumper += isRootIdentifier;
            } else {
                return SA_create_syntax_report(currentToken, 0, true, "<IDENTIFIER>");
            }

            continue;
        }
        case true:
            if (currentToken->type != _OP_DOT_) {
                return SA_create_syntax_report(currentToken, 0, true, ".");
            }

            jumper++;
            hasToBeDot = false;
            continue;
        }
    }

    if (hasToBeDot == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    }
    
    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/*
Purpose: Checks if a given TOKEN array is an array identifier
Return Type: SyntaxReport => Contains how many tokens to skip if TOKEN array match
                            the ARRAY_IDENTIFIER rule, else errorOccured = true
Params: TOKEN **tokens => TOKEN array to be checked;
        size_t startPos => Position from where to start checking
*/
SyntaxReport SA_is_array_identifier(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_RIGHT_EDGE_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "[");
    }
    
    int jumper = 0;

    while ((*tokens)[startPos + jumper].type == _OP_RIGHT_EDGE_BRACKET_) {
        SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + jumper + 1, 0);

        if (isSimpleTerm.errorOccured == true) {
            return SA_create_syntax_report(&(*tokens)[startPos + jumper + 1], 0, true, isSimpleTerm.expectedToken);
        }

        if ((*tokens)[startPos + isSimpleTerm.tokensToSkip + jumper + 1].type != _OP_LEFT_EDGE_BRACKET_) {
            return SA_create_syntax_report(&(*tokens)[startPos + isSimpleTerm.tokensToSkip + jumper + 1], 0, true, "]");
        }

        jumper += isSimpleTerm.tokensToSkip + 2;
    }
    
    return SA_create_syntax_report(NULL, jumper, 0, NULL);
}

/*
Purpose: Checks if a token matches the basic IDENTIFIER made out of letters, digits, underscores
Return Type: int => true = is root identifier; false = not a root identifier
Params: TOKEN *token => Token to be checked
*/
int SA_is_root_identifier(TOKEN *token) {
    if (token == NULL) {
        return false;
    }

    if ((int)SA_is_keyword(token) == true
        && token->type != _KW_THIS_) {
        return false;
    }

    for (int i = 0; i < token->size; i++) {
        char currentCharacter = token->value[i];

        if (currentCharacter == '\0') {
            break;
        } else if ((int)SA_is_letter(currentCharacter) == true) {
            continue;
        } else if ((int)SA_is_underscore(currentCharacter) == true) {
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
SyntaxReport SA_is_numeral_identifier(TOKEN *token) {
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
                return SA_create_syntax_report(token, 0, true, "<NUMBER>");
            }

            dots++;
            continue;
        } else if ((int)is_digit(currentCharacter) == true) {
            continue;
        }

        return SA_create_syntax_report(token, 0, true, "<DIGIT>\" or \".");
    }

    return SA_create_syntax_report(NULL, 1, false, NULL);
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

int SA_is_end_indicator(const TOKEN *token) {
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
int SA_is_string(TOKEN *token) {
    return (token->type == _STRING_ || token->type == _CHARACTER_ARRAY_) ?
        true : false;
}

/*
Purpose: Check if a given TOKEN array at a specific position is equivalent to the REFERENCE rule
Return int => true = is reference; false = not a reference
Params: TOKEN *token => Token to be checked
*/
int SA_is_reference(TOKEN *token) {
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
int SA_is_pointer(const TOKEN *token) {
    return token->type == _POINTER_ ? true : false;
}

/*
Purpose: Check if a given TOKEN is a keyword or not
Return Type: int => true = is keyword; false = not a keyword
Params: TOKEN *token => TOKEN to be checked
*/
TOKENTYPES KeywordLookupTable[] = {
    _KW_WHILE_, _KW_IF_, _KW_FUNCTION_, _KW_VAR_, _KW_BREAK_, _KW_BREAK_, _KW_RETURN_,
    _KW_DO_, _KW_CLASS_, _KW_WITH_, _KW_NEW_, _KW_TRUE_, _KW_FALSE_, _KW_NULL_, _KW_ENUM_,
    _KW_CHECK_, _KW_IS_, _KW_TRY_, _KW_CATCH_, _KW_CONTINUE_, _KW_CONST_, _KW_INCLUDE_,
    _KW_AND_, _KW_OR_, _KW_GLOBAL_, _KW_SECURE_, _KW_PRIVATE_, _KW_EXPORT_, _KW_FOR_,
    _KW_THIS_, _KW_ELSE_, _KW_INT_, _KW_DOUBLE_, _KW_FLOAT_, _KW_CHAR_, _KW_STRING_,
    _KW_SHORT_, _KW_LONG_, _KW_CONSTRUCTOR_
};

int SA_is_keyword(TOKEN *token) {
    for (int i = 0; i < sizeof(KeywordLookupTable) / sizeof(KeywordLookupTable[0]); i++) {
        if (token->type == KeywordLookupTable[i]) {
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
int SA_is_letter(const char character) {
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
int SA_is_number(const char character) {
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

int SA_is_rational_operator(const char *sequence) {
    for (int i = 0; i < (sizeof(rationalOperators) / sizeof(rationalOperators[0])); i++) {
        if ((int)strcmp(sequence, rationalOperators[i]) == 0) {
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
int SA_is_arithmetic_operator(const TOKEN *token) {
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
int SA_is_assignment_operator(const char *sequence) {
    char assignmentOperator[][3] = {"+=", "-=", "*=", "/="};
    int lengthOfAssignmentOperators = (sizeof(assignmentOperator) / sizeof(assignmentOperator[0]));

    for (int i = 0; i < lengthOfAssignmentOperators; i++) {
        if ((int)strcmp(sequence, assignmentOperator[i]) == 0) {
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
int SA_is_underscore(const char character) {
    return character == '_' ? true : false;
}

/*
Purpose: Check whether a given sequence is a bool or not
Return Type: int => true = is a bool; false = not a bool
Params: const char *sequence => Sequence to be checked
*/
int SA_is_bool(const char *sequence) {
    return ((int)strcmp(sequence, "true") == 0
        || (int)strcmp(sequence, "false") == 0) ? true : false;
}

/*
Purpose: Check whether a given sequence is a modifier or not
Return Type: int => true = is a modifier; false = not a modifier
Params: const char *sequence => Sequence to be checked
*/
int SA_is_modifier(const char *sequence) {
    return ((int)strcmp(sequence, "global") == 0
            || (int)strcmp(sequence, "local") == 0
            || (int)strcmp(sequence, "secure") == 0) ? true : false;
}

/*
Purpose: Check whether a given sequence is a logic operator or not
Return Type: int => true = is a logic operator; false = not a logic operator
Params: const char *sequence => Sequence to be checked
*/
int SA_is_logic_operator(const char *sequence) {
    return ((int)strcmp(sequence, "and") == 0
            || (int)strcmp(sequence, "or") == 0
            || (int)strcmp(sequence, "!") == 0) ? true : false;
}

/*
Purpose: Creates a SyntaxReport based on the given Params
Return Type: SyntaxReport => Containing all important information for an error
Params: TOKEN *token => Error token;
        int tokensToSkip => How many tokens should be skipped till the next step;
        int errorOccured => Was there an error or not;
        char *expectedToken => Token that was expected, when an error occures
*/
SyntaxReport SA_create_syntax_report(TOKEN *token, int tokensToSkip, int errorOccured, char *expectedToken) {
    SyntaxReport report;
    report.token = token;
    report.tokensToSkip = tokensToSkip;
    report.errorOccured = errorOccured;
    report.expectedToken = expectedToken;
    return report;
}

/*
Purpose: Throw an error
Return Type: void
Params: TOKEN *errorToken => Token that caused the issue;
        char *expectedToken => String that contain TOKEN suggestions
*/
void SA_throw_error(TOKEN *errorToken, char *expectedToken) {
    FILE_CONTAINS_ERRORS = true;

    if (SOURCE_CODE == NULL) {
        (void)printf("Source code pointer = NULL!");
        return;
    }

    (void)printf("SYNTAX ERROR: An error occured at line %i (%s).\n", (errorToken->line + 1), FILE_NAME);
    (void)printf("-------------------------------------------------------\n");

    int printPosition = errorToken->type == __EOF__ ? SOURCE_LENGTH : errorToken->tokenStart;

    for (int i = printPosition; i > 0; i--) {
        if ((*SOURCE_CODE)[i] == '\n') {
            break;
        }

        printPosition = i - 1;
    }
    
    char buffer[32];
    int tokPos = ((errorToken->tokenStart + 1) - printPosition);
    int blankLength = (int)snprintf(buffer, 32, "%i : %i | ", (errorToken->line + 1), tokPos);

    (void)printf("%s", buffer);

    for (int i = printPosition; i < SOURCE_LENGTH; i++) {
        if ((*SOURCE_CODE)[i] == '\n') {
            printPosition = i + 1;
            continue;
        }

        (void)printf("%c", (*SOURCE_CODE)[i]);
        
        if ((*SOURCE_CODE)[i + 1] == '\n' || i + 1 == SOURCE_LENGTH) {
            (void)printf("\n");
            break;
        }
    }

    for (int i = 0; i < blankLength; i++) {
        (void)printf(" ");
    }

    int counter = 0;

    for (int i = printPosition; i < SOURCE_LENGTH; i++) {
        if (i >= errorToken->tokenStart
            && counter < (errorToken->size - 1)) {
            (void)printf("^");
            counter++;
        } else {
            (void)printf(" ");
        }
    }

    (void)printf("\n\nUnexpected token \"%s\", maybe replace with \"%s\".\n", errorToken->value, expectedToken);
    (void)printf("-------------------------------------------------------\n\n");
}