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

/**
 * The subprogram {@code SPACE.src.syntaxAnalyzer} was created
 * to provide a basic syntax analyzer for the SPACE-Lang. The
 * process of the analysis happens on a "work-down" basis, which
 * means, that all tokens are invoked as a RUNNABLE, then classified
 * and processed accordingly.
 * 
 * For example:
 * - Code:
 * var a[] = {1, 2}
 * 
 * - Syntax analysis steps:
 * RUNNABLE (main) -> VARIABLE -> ARRAY VARIABLE -> INIT ARRAY VARIABLE
 * -> INIT 1 DIMENSIOT
 * 
 * @version 1.0     06.06.2024
 * @author Lukas Nian En Lampl
*/

/**
 * Defines the used booleans, 1 for true and 0 for false
*/
#define true 1
#define false 0

/**
 * Structure for storing Syntax reports.
 * The structure contains a token field, which is the place for
 * the error token. TokensToSkip says how many tokens to skip
 * until the next operation can be performed. The boolean errorOccured
 * is a flag that is set to true, when an error occured. The expectedToken
 * string is the field for defining what token was expected instead of
 * the error token.
*/
typedef struct SyntaxReport {
    TOKEN *token;
    int tokensToSkip;
    int errorOccured;
    char *expectedToken;
} SyntaxReport;

/**
 * Enum for all possible parameter types
*/
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
SyntaxReport SA_is_conditional_assignment(TOKEN **tokens, size_t startPos, int inDepth);
SyntaxReport SA_is_chained_condition(TOKEN **tokens, size_t startPos, int inParam);
SyntaxReport SA_is_condition(TOKEN **tokens, size_t startPos, int inParam);
SyntaxReport SA_is_array_variable(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_array_assignment(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_array_assignment_element(TOKEN **tokens, size_t startPos, int inDepth);
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
int SA_skip_visibility_modifier(TOKEN *token);
SyntaxReport SA_is_function_return_type_specifier(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_function_call(TOKEN **tokens, size_t startPos, int inFunction);
SyntaxReport SA_is_parameter(TOKEN **tokens, size_t startPos, enum ParameterType type);
SyntaxReport SA_is_array_dimension_definition(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_var_type_definition(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_simple_term(TOKEN **tokens, size_t startPos, int inParameter);
SyntaxReport SA_is_term_expression(TOKEN **tokens, size_t startPos);
int SA_predict_term_expression(TOKEN **tokens, size_t startPos);
int SA_predict_class_object_access(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_identifier(TOKEN **tokens, size_t startPos);
int SA_predict_array_access(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_array_access(TOKEN **tokens, size_t startPos);
SyntaxReport SA_is_array_identifier(TOKEN **tokens, size_t startPos);
int SA_is_root_identifier(TOKEN *token);
SyntaxReport SA_is_numeral_identifier(TOKEN *token);

int SA_is_string(TOKEN *token);
int SA_is_reference(TOKEN *token);
int SA_is_pointer(const TOKEN *token);
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

/**
 * @brief Checks the token sequence received by the
 * lexer and checks for syntax errors that might have been
 * introduced by the programmer.
 * 
 * @param tokens    Pointer the the tokens array from the lexer
 * @param tokenArrayLength  Length of the token array
 * @param **source  Pointer to the actual source code
 * @param sourceSize    Length of the source code
 * @param *sourceName    Name of the file that is currently checked
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

    if (SYNTAX_ANALYZER_DISPLAY_USED_TIME == true) {
        start = clock();
    }

    if (SYNTAX_ANALYZER_DEBUG_MODE == true) {
        (void)printf("\n\n\n>>>>>>>>>>>>>>>>>>>>    SYNTAX ANALYZER    <<<<<<<<<<<<<<<<<<<<\n\n");
    }

    //Start of the check sequence
    (void)SA_is_runnable(tokens, 0, false);

    if (SYNTAX_ANALYZER_DEBUG_MODE == true) {
        (void)printf("\n>>>>>    Tokens successfully analyzed    <<<<<\n");
    }

    if (SYNTAX_ANALYZER_DISPLAY_USED_TIME == true) {
        end = clock();
        (void)printf("\nCPU time used for SYNTAX ANALYSIS: %f seconds\n", ((double) (end - start)) / CLOCKS_PER_SEC);   
    }

    return FILE_CONTAINS_ERRORS == false ? 0 : 1;
}

int panicModeOpenBraces = 0;
int panicModeLastStartPos = 0;

/**
 * @brief Enters the syntax analyzer into a "panic mode" and thus skips
 * all tokens, till a recover might be possible.
 * 
 * @param **tokens  Pointer to the tokens array
 * @param startPos  Position from where to start searching for recovering position
 * @param runnableWithBlock Is the panic mode happening in a block-runnable
*/
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
                (void)printf("Estimated line: %li (%s)\n", (*tokens)[startPos].line + 1, FILE_NAME);
            } else if (panicModeLastStartPos == 1) {
                (void)printf("SYNTAX ERROR: Missing 1 closing brace \"}\".\n");
                (void)printf("Estimated line: %li (%s)\n", (*tokens)[startPos].line + 1, FILE_NAME);
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
        } else if ((int)is_keyword(currentToken) == true) {
            switch (currentToken->type) {
            case _KW_NEW_:
            case _KW_WHILE_:
                continue;
            default:
                return i - startPos;
            }
        }
    }

    return 1;
}

/**
 * @brief Checks if the passed tokens are written accordingly to the runnable
 * rule
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
 * @param withBlock Flag for whether the runnable is in a block or not (e.g. function)
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
                    return isKWBasedRunnable;
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
                return isNKWBasedRunnable;
            } else {
                if (withBlock == true) {
                    return isNKWBasedRunnable;
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

/**
 * @brief Predicts the programmers target based on expression prediction.
 *
 * @note Examples:
 * @note ```
 * @note print("Text");
 * @note a = a + 4;
 * @note b = getData();
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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

/**
 * @brief Checks if a class instance is set to null.
 *
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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
                    return isArrayIdentifier;
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

/**
 * @brief If no expression could be predicted the runnable function
 * call is executed as a "replacement". It is also called if it is
 * an intended function call.
 *
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_runnable_function_call(TOKEN **tokens, size_t startPos) {
    SyntaxReport isIdentifier = SA_is_identifier(tokens, startPos);
    
    if (isIdentifier.errorOccured == true) {
        return isIdentifier;
    }

    if ((*tokens)[startPos + isIdentifier.tokensToSkip - 1].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip - 1], 0, true, ")");
    }
    
    if ((*tokens)[startPos + isIdentifier.tokensToSkip].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isIdentifier.tokensToSkip], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, isIdentifier.tokensToSkip + 1, false, NULL);
}

/**
 * @brief Tries to predict if the current token array sequence is a class sequence
 * or not.
 * 
 * @returns `True (1)` if it is a class instance, else `false (0)`
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
int SA_predict_class_instance(TOKEN **tokens, size_t startPos) {
    int jumper = 0;

    while (startPos + jumper < MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        switch ((*tokens)[startPos + jumper].type) {
        case _KW_NEW_:
            return true;
        case _OP_SEMICOLON_:
        case _OP_RIGHT_BRACE_:
            return false;
        default: break;
        }

        jumper++;
    }

    return false;
}

/**
 * @brief Tries to predict if the current token sequence is an expression or not.
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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

/**
 * @brief Predicts the intended operation by the programmer and invokes
 * the predicted function.
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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

/**
 * @brief Checks if a given token sequence matches the class object access rule.
 *
 * @note Example:
 * @note ```
 * @note iden->iden2;
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_class_object_access(TOKEN **tokens, size_t startPos, int independentCall) {
    SyntaxReport leftIdentifier = SA_is_identifier(tokens, startPos);

    if (leftIdentifier.errorOccured == true) {
        return leftIdentifier;
    }

    if ((*tokens)[startPos + leftIdentifier.tokensToSkip].type != _OP_CLASS_ACCESSOR_) {
        return SA_create_syntax_report(&(*tokens)[startPos + leftIdentifier.tokensToSkip], 0, true, "->");
    }

    SyntaxReport rightIdentifier = SA_is_identifier(tokens, startPos + leftIdentifier.tokensToSkip + 1);

    if (rightIdentifier.errorOccured == true) {
        return rightIdentifier;
    }

    if (independentCall == true) {
        if ((*tokens)[startPos + leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 1].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos + leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 1], 0, true, ";");
        }

        return SA_create_syntax_report(NULL, leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 2, false, NULL);
    }

    return SA_create_syntax_report(NULL, leftIdentifier.tokensToSkip + rightIdentifier.tokensToSkip + 1, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the return statement.
 *
 * @note Examples:
 * @note ```
 * @note return null;
 * @note return 2 + a * b;
 * @note return "Hello world!";
 * @note return new Object();
 * @note return a < -1 ? b : c;
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_return_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_RETURN_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "return");
    }

    if ((*tokens)[startPos + 1].type != _KW_NEW_) {
        SyntaxReport isCondAssignment = SA_is_conditional_assignment(tokens, startPos + 1, true);
        
        if (isCondAssignment.errorOccured == false) {
            return isCondAssignment;
        }

        SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + 1, false);

        if (isSimpleTerm.errorOccured == true) {
            return isSimpleTerm;
        }

        if ((*tokens)[startPos + isSimpleTerm.tokensToSkip + 1].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos + isSimpleTerm.tokensToSkip + 1], 0, true, ";");
        }

        return SA_create_syntax_report(NULL, isSimpleTerm.tokensToSkip + 2, false, NULL);
    }

    SyntaxReport isReturnClassInstance = SA_is_return_class_instance(tokens, startPos + 1);

    if (isReturnClassInstance.errorOccured == true) {
        return isReturnClassInstance;
    }

    return SA_create_syntax_report(NULL, isReturnClassInstance.tokensToSkip + 1, false, NULL);
}

/**
 * @brief Checks if a given token sequence is a class instance creation that is
 * directly returned.
 *
 * @note Example:
 * @note ```
 * @note return new Object();
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_return_class_instance(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_NEW_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "new");
    }

    SyntaxReport isFunctionCall = SA_is_function_call(tokens, startPos + 1, false);

    if (isFunctionCall.errorOccured == true) {
        return isFunctionCall;
    }

    if ((*tokens)[startPos + isFunctionCall.tokensToSkip + 1].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isFunctionCall.tokensToSkip + 1], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, isFunctionCall.tokensToSkip + 2, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the continue statement.
 *
 * @note Example:
 * @note ```
 * @note continue;
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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

/**
 * @brief Checks if a given token sequence matches the break statement.
 *
 * @note Example:
 * @note ```
 * @note break;
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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

/**
 * @brief Checks if a given token sequence matches the for statement.
 *
 * @note Example:
 * @note ```
 * @note for (var i = 0; i < 10; i++) {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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
        return isVar;
    }

    SyntaxReport isChainedCond = SA_is_chained_condition(tokens, startPos + isVar.tokensToSkip + 2, true);

    if (isChainedCond.errorOccured == true) {
        return isChainedCond;
    }
    
    if ((*tokens)[startPos + isVar.tokensToSkip + isChainedCond.tokensToSkip + 2].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isVar.tokensToSkip + isChainedCond.tokensToSkip + 2], 0, true, ";");
    }

    int totalSkip = isVar.tokensToSkip + isChainedCond.tokensToSkip + 3;
    SyntaxReport isExpression = SA_is_expression(tokens, startPos + totalSkip, false);

    if (isExpression.errorOccured == true) {
        return isExpression;
    }

    totalSkip += isExpression.tokensToSkip;

    if ((*tokens)[startPos + totalSkip].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + totalSkip], 0, true, ")");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + totalSkip + 1, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    return SA_create_syntax_report(NULL, totalSkip + isRunnable.tokensToSkip + 1, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches an expression.
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_expression(TOKEN **tokens, size_t startPos, int inRunnable) {
    SyntaxReport isIdentifier = SA_is_identifier(tokens, startPos);
    int skip = 0;

    if (isIdentifier.errorOccured == true) {
        SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos, true);

        if (isSimpleTerm.errorOccured == true) {
            return isSimpleTerm;
        }

        skip += isSimpleTerm.tokensToSkip;
    }

    TOKEN *crucialToken = &(*tokens)[startPos + isIdentifier.tokensToSkip];
    skip += isIdentifier.tokensToSkip;

    if ((int)predict_is_conditional_assignment_type(tokens, startPos + skip, MAX_TOKEN_LENGTH) == true) {
        SyntaxReport isConditionAssignment = SA_is_conditional_assignment(tokens, startPos + skip, false);

        if (isConditionAssignment.errorOccured == true) {
            return isConditionAssignment;
        }

        skip += isConditionAssignment.tokensToSkip - 1;
    } else if (crucialToken->type == _OP_ADD_ONE_
        || crucialToken->type == _OP_SUBTRACT_ONE_) {
        int jumper = 0;

        while (startPos + skip + jumper < MAX_TOKEN_LENGTH) {
            if ((*tokens)[startPos + skip + jumper].type != _OP_ADD_ONE_
                && (*tokens)[startPos + skip + jumper].type != _OP_SUBTRACT_ONE_) {
                break;
            }

            jumper++;
        }

        skip += jumper;
    } else if ((int)SA_is_assignment_operator(crucialToken->value) == true
        || crucialToken->type == _OP_EQUALS_) {
        SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + skip + 1, false);

        if (isSimpleTerm.errorOccured == true) {
            return isSimpleTerm;
        }

        skip += isSimpleTerm.tokensToSkip + 1;
    } else {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "++\", \"--\", \"-=\", \"+=\", \"*=\", \"/=\" or \"=");
    }

    if (inRunnable == true) {
        if ((*tokens)[startPos + skip].type == _OP_SEMICOLON_) {
            return SA_create_syntax_report(NULL, + skip + 1, false, NULL);
        }

        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ";");
    }
    
    return SA_create_syntax_report(NULL, skip, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the if statement.
 *
 * @note Examples:
 * @note ```
 * @note if (a > 10) {}
 * @note if (object.getName() == "Thread") {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_if_statement(TOKEN **tokens, size_t startPos) { 
    int skip = 0;
    
    if ((*tokens)[startPos].type != _KW_IF_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "if");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    skip++;

    SyntaxReport isChainedCond = SA_is_chained_condition(tokens, startPos + ++skip, true);

    if (isChainedCond.errorOccured == true) {
        return isChainedCond;
    }

    skip += isChainedCond.tokensToSkip;

    if ((*tokens)[startPos + skip].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ")");
    }
    
    skip++;
    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + skip, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    skip += isRunnable.tokensToSkip;
    return SA_create_syntax_report(NULL, skip, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the else-if statement.
 *
 * @note Examples:
 * @note ```
 * @note else if (a > 60) {}
 * @note else if (b == 5 and c == 2 * 4) {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_else_if_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_ELSE_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "else");
    }

    SyntaxReport isIfStatement = SA_is_if_statement(tokens, startPos + 1);

    if (isIfStatement.errorOccured == true) {
        return isIfStatement;
    }

    return SA_create_syntax_report(NULL, isIfStatement.tokensToSkip + 1, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the else statement.
 *
 * @note Example:
 * @note ```
 * @note else {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_else_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_ELSE_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "else");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + 1, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + 1, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the do statement.
 *
 * @note Example:
 * @note ```
 * @note do {} while (a == b);
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_do_statment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_DO_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "do");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + 1, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    SyntaxReport isWhileCond = SA_is_while_condition(tokens, startPos + isRunnable.tokensToSkip + 1);

    if (isWhileCond.errorOccured == true) {
        return isWhileCond;
    }

    if ((*tokens)[startPos + isWhileCond.tokensToSkip + isRunnable.tokensToSkip + 1].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isWhileCond.tokensToSkip + isRunnable.tokensToSkip + 1], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + isWhileCond.tokensToSkip + 2, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the while statement.
 *
 * @note Example:
 * @note ```
 * @note while (true) {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_while_statement(TOKEN **tokens, size_t startPos) {
    SyntaxReport isWhileCond = SA_is_while_condition(tokens, startPos);

    if (isWhileCond.errorOccured == true) {
        return isWhileCond;
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + isWhileCond.tokensToSkip, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    return SA_create_syntax_report(NULL, isWhileCond.tokensToSkip + isRunnable.tokensToSkip, false, NULL);
}

/**
 * @brief Checks if a given token sequence is a while condition.
 *
 * @note Examples:
 * @note ```
 * @note while (true)
 * @note while (a < 2)
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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
        return isChainedCondition;
    }

    if ((*tokens)[startPos + isChainedCondition.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isChainedCondition.tokensToSkip + 2], 0, true, ")");
    }

    return SA_create_syntax_report(NULL, isChainedCondition.tokensToSkip + 3, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the check statement.
 *
 * @note Example:
 * @note ```
 * @note check (a) {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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
        return isIdentifier;
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
            return isIsStatement;
        }

        jumper += isIsStatement.tokensToSkip;
    }
   
    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the is statement.
 *
 * @note Example:
 * @note ```
 * @note is true:
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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
        SyntaxReport idenReport = SA_is_identifier(tokens, startPos + 1);

        if (idenReport.errorOccured == true) {
            return idenReport;
        }

        skip = idenReport.tokensToSkip;
    }

    if ((*tokens)[startPos + skip + 1].type != _OP_COLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip + 1], 0, true, ":");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + skip + 2, 2);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + skip + 2, false, NULL);
}

/**
 * @brief Checks if a token sequence fulfills the variable definition rule.
 *
 * @note All variable definitions also support the type option and visibility.
 * @note Examples:
 * @note ```
 * @note var a = 10;
 * @note var b;
 * @note var c[];
 * @note var d[] = {1, 2, 3, 4};
 * @note var obj = new Object();
 * @note var obj = new Object(param1, param2, *ptr);
 * @note const a = 10;
 * @note const b;
 * @note const d[] = {1, 2, 3, 4};
 * @note const obj = new Object();
 * @note const obj = new Object(param1, param2, *ptr);
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_variable(TOKEN **tokens, size_t startPos) {
    int skip = SA_skip_visibility_modifier(&(*tokens)[startPos]);
    TOKEN *varTok = &(*tokens)[startPos + skip];

    if (varTok->type == _KW_VAR_
        || varTok->type == _KW_CONST_) {
        skip++;

        if ((*tokens)[startPos + skip].type == _OP_COLON_) {
            SyntaxReport varTypeRep = SA_is_var_type_definition(tokens, startPos + skip);
            
            if (varTypeRep.errorOccured == true) {
                return varTypeRep;
            }

            skip += varTypeRep.tokensToSkip;
        }

        if ((int)SA_is_root_identifier(&(*tokens)[startPos + skip]) == false) {
            return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "<IDENTIFIER>");
        }

        skip++;
        TOKEN *crucialToken = &(*tokens)[startPos + skip];
        SyntaxReport report = {NULL, -1};

        if (crucialToken->type == _OP_RIGHT_EDGE_BRACKET_) {
            report = SA_is_array_variable(tokens, startPos + skip);
        } else if (crucialToken->type == _OP_SEMICOLON_
            && varTok->type != _KW_CONST_) {
            return SA_create_syntax_report(NULL, skip + 1, false, NULL);
        } else if (crucialToken->type == _OP_EQUALS_) {
            if ((int)predict_is_conditional_assignment_type(tokens, startPos + skip, MAX_TOKEN_LENGTH) == true) {
                report = SA_is_conditional_assignment(tokens, startPos + skip, false);
            } else if ((*tokens)[startPos + skip + 1].type == _KW_NEW_) {
                report = SA_is_class_instance(tokens, startPos + skip);
            } else {
                report = SA_is_assignment(tokens, startPos + skip);
            }
        } else {
            return SA_create_syntax_report(crucialToken, 0, true, "[\", \";\", \",\", \"=\" or \"<IDENTIFIER>");
        }

        if (report.errorOccured == true) {
            return report;
        }

        skip += report.tokensToSkip;
        return SA_create_syntax_report(NULL, skip, false, NULL);
    }
    
    return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "const\" or \"var");
}

/**
 * @brief Handles class instance variables.
 *
 * @note Examples:
 * @note ```
 * @note = new Object();
 * @note = new Object(param1, param2, *ptr);
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_class_instance(TOKEN **tokens, size_t startPos) {
    int skip = 0;
   
    if ((*tokens)[startPos + skip].type != _OP_EQUALS_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "=");
    }

    skip++;

    if ((*tokens)[startPos + skip].type != _KW_NEW_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "new");
    }

    skip++;

    if ((int)SA_is_root_identifier(&(*tokens)[startPos + skip]) == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "<IDENTIFIER>");    
    }

    skip++;

    if ((*tokens)[startPos + skip].type == _OP_RIGHT_BRACKET_) {
        SyntaxReport isParam = SA_is_parameter(tokens, startPos + skip + 1, _PARAM_FUNCTION_CALL_);

        if (isParam.errorOccured == true) {
            return isParam;
        }

        skip += isParam.tokensToSkip + 1;

        if ((*tokens)[startPos + skip].type != _OP_LEFT_BRACKET_) {
            return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ")");
        }

        skip++;
    } else if ((*tokens)[startPos + skip].type == _OP_RIGHT_EDGE_BRACKET_) {
        SyntaxReport isArray = SA_is_array_element(tokens, startPos + skip);

        if (isArray.errorOccured == true) {
            return isArray;
        }

        skip += isArray.tokensToSkip;
    } else {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "(\", \"[");
    }

    if ((*tokens)[startPos + skip].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ";");
    }
    
    return SA_create_syntax_report(NULL, skip + 1, false, NULL);
}

/**
 * @brief Handles normal variable assignments.
 *
 * @note Examples:
 * @note ```
 * @note = b;
 * @note = 10;
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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

/**
 * @brief Handles conditional assignments.
 *
 * @note Examples:
 * @note ```
 * @note a == true ? 2 : 3;
 * @note a == true ? b == true ? 2 : 4 : 5
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
 * @param inDepth   Flag whether the conditional assignment needs an "="
*/
SyntaxReport SA_is_conditional_assignment(TOKEN **tokens, size_t startPos, int inDepth) {
    int skip = inDepth == false ? 1 : 0;
    
    if ((*tokens)[startPos].type != _OP_EQUALS_
        && inDepth == false) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "=");
    }

    SyntaxReport isChainedCondition = SA_is_chained_condition(tokens, startPos + skip, false);

    if (isChainedCondition.errorOccured == true) {
        return isChainedCondition;
    }

    skip += isChainedCondition.tokensToSkip;

    if ((*tokens)[startPos + skip].type != _OP_QUESTION_MARK_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "?");
    }

    skip++;
    SyntaxReport leftVal = {NULL, -1};

    if ((int)predict_is_conditional_assignment_type(tokens, startPos + skip, MAX_TOKEN_LENGTH) == true) {
        leftVal = SA_is_conditional_assignment(tokens, startPos + skip, true);
    } else {
        leftVal = SA_is_simple_term(tokens, startPos + skip, true);
    }

    if (leftVal.errorOccured == true) {
        return leftVal;
    }

    skip += leftVal.tokensToSkip;

    if ((*tokens)[startPos + skip].type != _OP_COLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ":");
    }

    skip++;
    SyntaxReport rightVal = {NULL, -1};

    if ((int)predict_is_conditional_assignment_type(tokens, startPos + skip, MAX_TOKEN_LENGTH) == true) {
        rightVal = SA_is_conditional_assignment(tokens, startPos + skip, true);
    } else {
        rightVal = SA_is_simple_term(tokens, startPos + skip, true);
    }

    if (rightVal.errorOccured == true) {
        return rightVal;
    }

    skip += rightVal.tokensToSkip;

    if (inDepth == false) {
        if ((*tokens)[startPos + skip].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ";");
        }

        skip++;
    }

    return SA_create_syntax_report(NULL, skip, false, NULL);
}

/**
 * @brief Checks if a token sequence is a chained condition or not.
 *
 * @note Examples:
 * @note ```
 * @note a == true and b == 2 or c == d
 * @note (a == true and b == 2) or c == d
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
 * @param inParam   Flag if the chained condition is in a parameter or not
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
        } else if ((int)is_end_indicator(currentToken) == true
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

/**
 * @brief Checks if a given token sequence is a simple condition.
 *
 * @note Examples:
 * @note ```
 * @note a <= 2
 * @note b == true
 * @note c != 3
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
 * @param inParam   Flag if the condition is in a parameter
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

/**
 * @brief Handles array variables.
 *
 * @note Examples:
 * @note ```
 * @note [] = {};
 * @note [2] = {1, 2};
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_array_variable(TOKEN **tokens, size_t startPos) {
    int skip = 0;
    SyntaxReport isArrayElement = SA_is_array_element(tokens, startPos);

    if (isArrayElement.errorOccured == true) {
        return isArrayElement;
    }
    
    skip += isArrayElement.tokensToSkip;

    if ((*tokens)[startPos + skip].type == _OP_EQUALS_) {
        SyntaxReport rep = {NULL, -1};
        
        if ((*tokens)[startPos + skip + 1].type == _OP_RIGHT_BRACE_) {
            rep = SA_is_array_assignment(tokens, startPos + skip);
        } else {
            rep = SA_is_identifier(tokens, startPos + skip + 1);
            rep.tokensToSkip++;
        }

        if (rep.errorOccured == true) {
            return rep;
        }

        skip += rep.tokensToSkip;
    }

    if ((*tokens)[startPos + skip].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ";");
    }

    return SA_create_syntax_report(NULL, skip + 1, false, NULL);
}

/**
 * @brief Handles basic array assignments.
 *
 * @note Examples:
 * @note ```
 * @note = {1, 2}
 * @note = {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_array_assignment(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_EQUALS_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "=");
    }

    if ((*tokens)[startPos + 1].type == _KW_NULL_) {
        return SA_create_syntax_report(NULL, 2, false, NULL);
    }

    SyntaxReport arrayAssignmentElemReport = SA_is_array_assignment_element(tokens, startPos + 2, false);

    if (arrayAssignmentElemReport.errorOccured == true) {
        return arrayAssignmentElemReport;
    }
    
    return SA_create_syntax_report(NULL, arrayAssignmentElemReport.tokensToSkip + 1, false, NULL);
}

/**
 * @brief Handles single elements `{}` individually with recursion.
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_array_assignment_element(TOKEN **tokens, size_t startPos, int inDepth) {
    int jumper = 0;
    int hasToBeComma = false;

    while (startPos + jumper < MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken->type == _OP_RIGHT_BRACE_) {
            SyntaxReport arrayAssignment = SA_is_array_assignment_element(tokens, startPos + jumper + 1, true);
            
            if (arrayAssignment.errorOccured == true) {
                return arrayAssignment;
            }
            
            jumper += arrayAssignment.tokensToSkip;
            continue;
        } else if (currentToken->type == _OP_LEFT_BRACE_) {
            if (inDepth == true) {
                return SA_create_syntax_report(NULL, jumper, false, NULL);
            }

            jumper++;
            continue;
        } else if ((int)is_end_indicator(currentToken) == true
            && currentToken->type != _OP_COMMA_) {
            jumper++;
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
    } else if (jumper <= 1) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<ARRAY_ASSIGNMENT>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches an array element.
 *
 * @note Examples:
 * @note ```
 * @note [2]
 * @note [a + b * 3]
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_array_element(TOKEN **tokens, size_t startPos) {
    int jumper = 0;

    while (startPos + jumper < MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if ((int)is_end_indicator(currentToken)
            && currentToken->type != _OP_LEFT_EDGE_BRACKET_) {
            break;
        }

        if (currentToken->type != _OP_RIGHT_EDGE_BRACKET_) {
            return SA_create_syntax_report(currentToken, 0, true, "[");
        }
        
        if ((*tokens)[startPos + jumper + 1].type != _OP_LEFT_EDGE_BRACKET_) {
            SyntaxReport isSimpleTerm = SA_is_simple_term(tokens, startPos + jumper + 1, false);

            if (isSimpleTerm.errorOccured == true) {
                return isSimpleTerm;
            }

            jumper += isSimpleTerm.tokensToSkip + 1;

            if ((*tokens)[startPos + jumper].type != _OP_LEFT_EDGE_BRACKET_) {
                return SA_create_syntax_report(currentToken, 0, true, "]");
            }
        } else {
            jumper++;
        }

        jumper++;
    }

    if (jumper == 0) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<ARRAY_ELEMENT>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the constructor statement.
 *
 * @note Examples:
 * @note ```
 * @note this::constructor() {}
 * @note this::constructor(obj) {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_class_constructor(TOKEN **tokens, size_t startPos) {
    int skip = 0;
    
    if ((*tokens)[startPos].type != _KW_THIS_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "this");
    }

    if ((*tokens)[startPos + 1].type != _OP_COLON_
        && (*tokens)[startPos + 2].type != _OP_COLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "::");
    }

    skip += 3;

    if ((*tokens)[startPos + skip].type != _KW_CONSTRUCTOR_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 3], 0, true, "constructor");
    }

    skip++;

    if ((*tokens)[startPos + skip].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 3], 0, true, "(");
    }

    skip++;
    SyntaxReport paramReport = SA_is_parameter(tokens, startPos + skip, _PARAM_FUNCTION_);

    if (paramReport.errorOccured == true) {
        return paramReport;
    }

    skip += paramReport.tokensToSkip;

    if ((*tokens)[startPos + skip].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ")");
    }

    skip++;

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + skip, 1);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    skip += isRunnable.tokensToSkip;
    return SA_create_syntax_report(NULL, skip, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the class statment.
 *
 * @note Examples:
 * @note ```
 * @note class Apple => {}
 * @note class Form extends Line => {}
 * @note class Window with EventListener => {}
 * @note class Dice extends Cube with RollEvent => {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_class(TOKEN **tokens, size_t startPos) {
    int skip = 0;
    skip += SA_skip_visibility_modifier(&(*tokens)[startPos]);

    if ((*tokens)[startPos + skip].type != _KW_CLASS_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "class");
    }

    skip++;
    int rootIden = SA_is_root_identifier(&(*tokens)[startPos + skip]);

    if (rootIden == 0) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "<IDENTIFIER>");
    }

    skip++;

    if ((*tokens)[startPos + skip].type == _KW_EXTENDS_) {
        int rootIden = SA_is_root_identifier(&(*tokens)[startPos + skip + 1]);

        if (rootIden == 0) {
            return SA_create_syntax_report(&(*tokens)[startPos + skip + 1], 0, true, "<IDENTIFIER>");
        }

        skip += 2;
    }

    if ((*tokens)[startPos + skip].type == _KW_WITH_) {
        SyntaxReport isWith = SA_is_with_statement(tokens, startPos + skip);

        if (isWith.errorOccured == true) {
            return isWith;
        }
        
        skip += isWith.tokensToSkip;
    }

    if ((*tokens)[startPos + skip].type != _OP_CLASS_CREATOR_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "=>");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + skip + 1, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    skip += isRunnable.tokensToSkip;
    return SA_create_syntax_report(NULL, skip + 1, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the with statment.
 *
 * @note Examples:
 * @note ```
 * @note with ActionListener
 * @note with Math
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_with_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_WITH_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "with");
    }
    
    SyntaxReport isParameter = SA_is_parameter(tokens, startPos + 1, _PARAM_CLASS_);

    if (isParameter.errorOccured == true) {
        return isParameter;
    }
    
    return SA_create_syntax_report(NULL, isParameter.tokensToSkip + 1, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the try statement.
 *
 * @note Examples:
 * @note ```
 * @note try {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_try_statement(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _KW_TRY_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "try");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + 1, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    return SA_create_syntax_report(NULL, isRunnable.tokensToSkip + 1, false, NULL);
}

/**
 * @brief Checks if a given token sequence matches the catch statement.
 *
 * @note Examples:
 * @note ```
 * @note catch (Exception e) {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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

/**
 * @brief Checks if a given token sequence matches the export statement.
 *
 * @note Examples:
 * @note ```
 * @note export "package";
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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

/**
 * @brief Checks if a given token sequence matches the include statement.
 *
 * @note Examples:
 * @note ```
 * @note include "package";
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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

/**
 * @brief Checks if a tokens sequence starting from `startingPos` is written
 * accordingly to the enum syntax.
 * 
 * @note Example:
 * @note ```
 * @note enum test {
 * @note    a,
 * @note    b : 10,
 * @note    c
 * @note }
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
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
        return isEnumerator;
    }

    if ((*tokens)[startPos + isEnumerator.tokensToSkip + 3].type != _OP_LEFT_BRACE_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isEnumerator.tokensToSkip + 3], 0, true, "}");
    }
    
    return SA_create_syntax_report(NULL, isEnumerator.tokensToSkip + 4, false, NULL);
}

/**
 * @brief Checks if the token sequence initializes enumerators in an enum.
 * 
 * @note Examples:
 * @note ```
 * @note a
 * @note b : 10
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_enumerator(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int hasToBeComma = false;

    while (startPos + jumper < MAX_TOKEN_LENGTH
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

    if (hasToBeComma == false && jumper > 0) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/**
 * @brief Checks whether the input tokens at the position `startPos` are aligned
 * to a function definition.  
 *   
 * @note Examples:  
 * @note ```
 * @note function add(num1, num2) {}
 * @note function:int add(num1, num2) {}
 * @note function:int add(num1:int, num2:int) {}
 * @note function:int add(*num1, (*num2)) {}
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_function(TOKEN **tokens, size_t startPos) {
    int skip = 0;
    skip += SA_skip_visibility_modifier(&(*tokens)[startPos]);

    if ((*tokens)[startPos + skip].type != _KW_FUNCTION_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "function");
    }

    if ((*tokens)[startPos + skip + 1].type == _OP_COLON_) {
        SyntaxReport isTypeDefinition = SA_is_var_type_definition(tokens, startPos + skip + 1);

        if (isTypeDefinition.errorOccured == true) {
            return isTypeDefinition;
        }

        skip += isTypeDefinition.tokensToSkip;
    }

    SyntaxReport isFunctionCall = SA_is_function_call(tokens, startPos + skip + 1, true);

    if (isFunctionCall.errorOccured == true) {
        return isFunctionCall;
    }
    
    skip += isFunctionCall.tokensToSkip;
    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + skip + 1, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    skip += isRunnable.tokensToSkip;
    return SA_create_syntax_report(NULL, skip + 1, false, NULL);
}

/**
 * @brief Checks if a token is a visibility token or not.
 * 
 * @note Here is a list of visibility modifiers:
 * @note - global
 * @note - secure
 * @note - private
 * 
 * @returns `True (1)`, when the token is a visibility modifier, else
 * `false (0)`
*/
int SA_skip_visibility_modifier(TOKEN *token) {
    switch (token->type) {
    case _KW_GLOBAL_: case _KW_SECURE_: case _KW_PRIVATE_:
        return 1;
    default:
        return 0;
    }
}

/**
 * @brief Checks if a tokens sequence starting from `startingPos` is written
 * accordingly to the function call syntax.
 * 
 * @note Examples:
 * @note ```
 * @note add(num1, num2);
 * @note add(&num1, *num2);
 * @note add(2 + 3, 4 * 5 + a);
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_function_call(TOKEN **tokens, size_t startPos, int inFunction) {
    int isRootIdentifier = (int)SA_is_root_identifier(&(*tokens)[startPos]);

    if (isRootIdentifier == false) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }
    
    SyntaxReport isParameter = SA_is_parameter(tokens, startPos + 2, inFunction == true ? _PARAM_FUNCTION_ : _PARAM_FUNCTION_CALL_);

    if (isParameter.errorOccured == true) {
        return SA_create_syntax_report(isParameter.token, 0, true, isParameter.expectedToken);
    }

    if ((*tokens)[startPos + isParameter.tokensToSkip + 2].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + isParameter.tokensToSkip + 2], 0, true, ")");
    }
    
    return SA_create_syntax_report(NULL, isRootIdentifier + isParameter.tokensToSkip + 2, false, NULL);
}

/**
 * @brief Checks if a tokens sequence starting from `startingPos` is a
 * parameter or not.
 * 
 * @note Examples:
 * @note ```
 * @note num1, num2
 * @note &num1, *num2
 * @note 2 + 3, 4 * 5 + a
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
 * @param type  Type of the param (function call, function, class)
*/
SyntaxReport SA_is_parameter(TOKEN **tokens, size_t startPos, enum ParameterType type) {
    int jumper = 0;
    unsigned char hasToBeComma = false;

    while (startPos + jumper < MAX_TOKEN_LENGTH
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
            case _PARAM_FUNCTION_CALL_: {
                SyntaxReport rep = {NULL, -1};

                if ((int)predict_is_conditional_assignment_type(tokens, startPos + jumper, MAX_TOKEN_LENGTH) == true) {
                    rep = SA_is_conditional_assignment(tokens, startPos + jumper, true);
                } else {
                    rep = SA_is_simple_term(tokens, startPos + jumper, true);
                }

                if (rep.errorOccured == true) {
                    return rep;
                }

                jumper += rep.tokensToSkip;
                break;
            }
            case _PARAM_CLASS_:
            case _PARAM_FUNCTION_:
                if (currentToken->value[0] == '*') {
                    if ((int)SA_is_pointer(currentToken) == false) {
                        return SA_create_syntax_report(currentToken, 0, true, "<POINTER>");
                    }
                } else if ((int)SA_is_letter(currentToken->value[0]) == true) {
                    if ((int)SA_is_root_identifier(currentToken) == false) {
                        return SA_create_syntax_report(currentToken, 0, true, "<IDENTIFIER>");
                    }

                    if ((*tokens)[startPos + jumper + 1].type == _OP_RIGHT_EDGE_BRACKET_) {
                        SyntaxReport isDimDef = SA_is_array_dimension_definition(tokens, startPos + jumper + 1);
                        
                        if (isDimDef.errorOccured == true) {
                            return isDimDef;
                        }
                        
                        jumper += isDimDef.tokensToSkip;
                    }
                }

                jumper++;

                if ((*tokens)[startPos + jumper].type == _OP_COLON_) {
                    SyntaxReport isTypeDefinition = SA_is_var_type_definition(tokens, startPos + jumper);

                    if (isTypeDefinition.errorOccured == true) {
                        return isTypeDefinition;
                    }

                    jumper += isTypeDefinition.tokensToSkip;
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
        return SA_create_syntax_report(&(*tokens)[startPos + jumper - 1], 0, true, ")");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/**
 * @brief Checks if the following tokens are a valid dimension definition.
 *
 * @note Examples:
 * @note ```
 * @note []
 * @note [][][][]
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_array_dimension_definition(TOKEN **tokens, size_t startPos) {
    int skip = 0;

    while (startPos + skip < MAX_TOKEN_LENGTH) {
        if ((*tokens)[startPos + skip].type != _OP_RIGHT_EDGE_BRACKET_) {
            break;
        }
        
        if ((*tokens)[startPos + skip + 1].type != _OP_LEFT_EDGE_BRACKET_) {
            return SA_create_syntax_report(&(*tokens)[startPos + skip + 1], 0, true, "]");
        }

        skip += 2;
    }

    return SA_create_syntax_report(NULL, skip, false, NULL);
}

/**
 * @brief Checks if a sequence is a variable type definition (cast).
 * 
 * @note Examples:
 * @note ```
 * @note :int
 * @note :char
 * @note :double[]
 * @note :Object[][]
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_var_type_definition(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_COLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, ":");
    }

    if ((int)is_primitive((*tokens)[startPos + 1].type) == false
        && (int)SA_is_root_identifier(&(*tokens)[startPos + 1]) == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "<IDENTIFIER>\" or \"<PRIMITIVE>");
    }

    int jumper = 2;

    while ((int)SA_is_root_identifier(&(*tokens)[startPos + jumper]) == false
        && (int)is_end_indicator(&(*tokens)[startPos + jumper]) == false) {
        if ((*tokens)[startPos + jumper].type != _OP_RIGHT_EDGE_BRACKET_) {
            return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "[");
        }

        if ((*tokens)[startPos + jumper + 1].type != _OP_LEFT_EDGE_BRACKET_) {
            return SA_create_syntax_report(&(*tokens)[startPos + jumper + 1], 0, true, "]");
        }

        jumper += 2;
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/**
 * @brief Checks if a tokens sequence starting from `startingPos` is a simple
 * term or not.
 * 
 * @note Examples:
 * @note ```
 * @note 1 + 2
 * @note 3 * 4
 * @note a + b * c
 * @note "Hello" + "World"
 * @note getInteger() + 2
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
 * @param inParameter   Flag for whether the simple term is called from a
 *                      parameter or not
*/
SyntaxReport SA_is_simple_term(TOKEN **tokens, size_t startPos, int inParameter) {
    int jumper = 0;
    unsigned char hasToBeArithmeticOperator = false;

    while (startPos + jumper < MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken->type == _OP_RIGHT_BRACKET_) {
            SyntaxReport isTerm = SA_is_simple_term(tokens, startPos + jumper + 1, true);

            if (isTerm.errorOccured == true) {
                return isTerm;
            }

            jumper += isTerm.tokensToSkip + 2;

            if ((int)SA_predict_array_access(tokens, startPos + jumper) == true) {
                SyntaxReport isArrayAccess = SA_is_array_access(tokens, startPos + jumper);

                if (isArrayAccess.errorOccured == false) {
                    jumper += isArrayAccess.tokensToSkip;
                } else {
                    return isArrayAccess;
                }
            }
            
            hasToBeArithmeticOperator = true;
            continue;
        } else if (currentToken->type == _OP_LEFT_BRACKET_) {
            if (inParameter == true) {
                break;
            } else if (hasToBeArithmeticOperator == false) { //If an operator is before the Bracket
                return SA_create_syntax_report(&(*tokens)[startPos + jumper - 1], 0, true, ")");
            }

            jumper++;
            continue;
        } else if (currentToken->type == _OP_NOT_
            && hasToBeArithmeticOperator == false) {
            jumper++;
            continue;
        } else if ((int)is_end_indicator(currentToken) == true) {
            break;
        }

        switch (hasToBeArithmeticOperator) {
        case false:
            hasToBeArithmeticOperator = true;

            if (currentToken->value[0] == '\"'
                || currentToken->value[0] == '\'') {
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
                } else if ((int)SA_is_bool((*tokens)[startPos].value) == true
                    || currentToken->type == _KW_NULL_) {
                    jumper++;
                    continue;
                } else {
                    isIdentifier = SA_is_identifier(tokens, startPos + jumper);
                }
            } else if ((int)is_digit(currentToken->value[0]) == true) {
                isIdentifier = SA_is_numeral_identifier(currentToken);
            } else if ((int)SA_predict_term_expression(tokens, startPos + jumper)) {
                isIdentifier = SA_is_term_expression(tokens, startPos + jumper);
            } else {
                if ((int)SA_is_pointer(currentToken) == true
                    || (int)SA_is_reference(currentToken) == true) {
                    if ((int)SA_predict_array_access(tokens, startPos + jumper + 1) == true) {
                        SyntaxReport isArrayAccess = SA_is_array_access(tokens, startPos + jumper + 1);

                        if (isArrayAccess.errorOccured == false) {
                            jumper += isArrayAccess.tokensToSkip;
                        } else {
                            return isArrayAccess;
                        }
                    }

                    jumper++;
                    continue;
                }

                return SA_create_syntax_report(currentToken, 0, true, "<POINTER>\" or \"<REFERENCE>");
            }

            if (isIdentifier.errorOccured == false) {
                jumper += isIdentifier.tokensToSkip;
            } else {
                return isIdentifier;
            }

            break;
        case true:
            if ((int)SA_is_arithmetic_operator(currentToken) != true) {
                return SA_create_syntax_report(currentToken, 0, true, "+\", \"-\", \"*\", \"%\" or \"/");
            }

            jumper++;
            hasToBeArithmeticOperator = false;
            break;
        }
    }

    if (hasToBeArithmeticOperator == false) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFIER>");
    } else if (jumper == 0) {
        return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "<IDENTIFER>\", \"<FUNCTION_CALL>\" or \"<CLASS_OBJECT_ACCESS>");
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/**
* @brief Handles operations like `p++`, `++p`, `p--`, `--p` or `p++++`.
 *
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_term_expression(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    unsigned char identifiers = 0;

    while (startPos + jumper < MAX_TOKEN_LENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];
        int rootIden = SA_is_root_identifier(currentToken);
        identifiers += rootIden;

        if (identifiers > 1) {
            return SA_create_syntax_report(currentToken, 0, true, "++\" or \"--");
        } else if (currentToken->type != _OP_ADD_ONE_
            && currentToken->type != _OP_SUBTRACT_ONE_
            && rootIden == false) {
            break;
        }

        if (tokens == 0) {
            return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "<IDENTIFIER>");
        }

        jumper++;
    }

    return SA_create_syntax_report(&(*tokens)[startPos], jumper, false, NULL);
}

/**
 * @brief Predicts if the next operation is a term expression.
 * 
 * @note As a term expression counts everything like this:
 * @note `p++`, `++p`, `p--`, `--p` or `p++++`.
 * 
 * @returns `True (1)` if the next operation is a term expression, else
 * `false (0)`
*/
int SA_predict_term_expression(TOKEN **tokens, size_t startPos) {
    for (int i = 0; i < MAX_TOKEN_LENGTH; i++) {
        if ((*tokens)[startPos + i].type == _OP_ADD_ONE_
            || (*tokens)[startPos + i].type == _OP_SUBTRACT_ONE_) {
            return true;
        } else if ((int)is_end_indicator(&(*tokens)[startPos + i]) == true) {
            return false;
        }
    }

    return false;
}

/**
 * @brief Checks if the next tokens reference to a class access or not.
 * 
 * @returns `True` (1), when a class access is detected, else `false (0)`
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
int SA_predict_class_object_access(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    int openBrackets = 0;

    while (startPos + jumper < MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        switch ((*tokens)[startPos + jumper].type) {
        case _OP_CLASS_ACCESSOR_:
            if (openBrackets != 0) {
                return false;
            }

            return true;
        case _OP_SEMICOLON_:
            return false;
        case _OP_RIGHT_BRACKET_:
            openBrackets++;
            break;
        case _OP_LEFT_BRACKET_:
            openBrackets--;
            break;
        default: break;
        }

        jumper++;
    }

    return false;
}

/**
 * @brief Checks if a token sequence is an IDENTIFIER or not.
 * 
 * @note An identifier is a token sequence that is seperated with a '.'.
 * @note  
 * @note Examples:
 * @note ```
 * @note token.name.getString().getBytes()[5]
 * @note this.size
 * @note getDefaultValue().getInteger()
 * @note ```
 *
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_identifier(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    unsigned char hasToBeDot = false;

    while (startPos + jumper < MAX_TOKEN_LENGTH
        && (*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if ((int)is_end_indicator(currentToken) == true
            || (int)SA_is_arithmetic_operator(currentToken) == true
            || currentToken->type == _OP_ADD_ONE_
            || currentToken->type == _OP_SUBTRACT_ONE_) {
            break;
        }

        switch (hasToBeDot) {
        case false: {
            hasToBeDot = true;
            int isRootIdentifier = SA_is_root_identifier(currentToken);

            if (isRootIdentifier == true) {
                currentToken = &(*tokens)[startPos + jumper + 1];

                if (currentToken->type == _OP_RIGHT_EDGE_BRACKET_) {
                    SyntaxReport isArrayIdentifier = SA_is_array_identifier(tokens, startPos + jumper);

                    if (isArrayIdentifier.errorOccured == false) {
                        jumper += isArrayIdentifier.tokensToSkip + isRootIdentifier;
                        continue;
                    } else {
                        return isArrayIdentifier;
                    }
                } else if (currentToken->type == _OP_RIGHT_BRACKET_) {
                    SyntaxReport isFunctionCall = SA_is_function_call(tokens, startPos + jumper, false);

                    if (isFunctionCall.errorOccured == false) {
                        jumper += isFunctionCall.tokensToSkip;

                        if ((int)SA_predict_array_access(tokens, startPos + jumper) == true) {
                            SyntaxReport isArrayAccess = SA_is_array_access(tokens, startPos + jumper);

                            if (isArrayAccess.errorOccured == false) {
                                jumper += isArrayAccess.tokensToSkip;
                                continue;
                            } else {
                                return isArrayAccess;
                            }
                        }

                        continue;
                    } else {
                        return isFunctionCall;
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

/**
 * @brief Predicts whether the next tokens are an array access or not.
 * 
 * @returns `True (1)` if there will be an array access, else
 * `false (0)`
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start predicting
*/
int SA_predict_array_access(TOKEN **tokens, size_t startPos) {
    for (int i = startPos; i < MAX_TOKEN_LENGTH; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (currentToken->type == _OP_RIGHT_EDGE_BRACKET_) {
            return true;
        } else {
            break;
        }
    }

    return false;
}

/**
 * @brief Checks if an input matches the array acces sequence or not.
 *
 * @note Examples:
 * @note ```
 * @note [0]
 * @note [index + offset]
 * @note ```
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_array_access(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type != _OP_RIGHT_EDGE_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "[");
    }

    int jumper = 0;

    while ((*tokens)[startPos + jumper].type == _OP_RIGHT_EDGE_BRACKET_
        && startPos + jumper < MAX_TOKEN_LENGTH) {
        SyntaxReport term = SA_is_simple_term(tokens, startPos + (++jumper), false);

        if (term.errorOccured == true) {
            return term;
        }

        jumper += term.tokensToSkip;

        if ((*tokens)[startPos + jumper].type != _OP_LEFT_EDGE_BRACKET_) {
            return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "]");
        }

        jumper++;
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
    int rootIden = SA_is_root_identifier(&(*tokens)[startPos]);

    if (rootIden == 0) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "<IDENTIFIER>");
    }
    
    SyntaxReport isArrayAccess = SA_is_array_access(tokens, startPos + 1);

    if (isArrayAccess.errorOccured == true) {
        return isArrayAccess;
    }
    
    return SA_create_syntax_report(NULL, isArrayAccess.tokensToSkip, 0, NULL);
}

/*
Purpose: Checks if a token matches the basic IDENTIFIER made out of letters, digits, underscores
Return Type: int => true = is root identifier; false = not a root identifier
Params: TOKEN *token => Token to be checked
*/
int SA_is_root_identifier(TOKEN *token) {
    if (token == NULL) {
        return false;
    } else if ((int)is_keyword(token) == true
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

    unsigned char dots = 0;

    for (int i = 0; i < token->size; i++) {
        char currentCharacter = token->value[i];

        if (currentCharacter == '\0') {
            break;
        } else if (currentCharacter == '.') {
            if (dots >= 1) { //Two dots in one number
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
    int size = (sizeof(rationalOperators) / sizeof(rationalOperators[0]));

    for (int i = 0; i < size; i++) {
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
    char assignmentOperator[][3] = {"+=", "-=", "*=", "/=", "++", "--"};
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

/**
 * Creates a SyntaxReport structure containing all data to throw an error.
 * The SyntaxReport contains the error token, how many tokens to skip, if
 * a run was successful and the expected token.
 * 
 * @param *token    Pointer to the TOKEN array
 * @param tokensToSkip  Number of tokens to skip after the check
 * @param errorOccured  Flag for whether an error occured or not
 * @param *expectedToken    The token that is expected instead of the error token
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
    int errorWindow = 32;

    if (SOURCE_CODE == NULL) {
        (void)printf("Source code pointer = NULL!");
        return;
    }

    (void)printf("SYNTAX ERROR: An error occured on line %li (%s).\n", (errorToken->line + 1), FILE_NAME);
    (void)printf("-------------------------------------------------------\n");

    int errorLine = errorToken->line + 1;
    int printPosition = errorToken->type == __EOF__ ? SOURCE_LENGTH : errorLine;

    for (int i = printPosition, step = 0; i > 0; i--, step++) {
        if ((*SOURCE_CODE)[i] == '\n' || step == errorWindow) {
            break;
        }

        printPosition = i - 1;
    }
    
    char buffer[32];
    int tokPos = ((errorToken->tokenStart + 1) - printPosition);
    int blankLength = (int)snprintf(buffer, 32, "%i : %i | ", errorLine, tokPos);
    (void)printf("%s", buffer);

    for (int i = printPosition, step = 0; i < SOURCE_LENGTH; i++, step++) {
        if ((*SOURCE_CODE)[i] == '\n' || step == errorWindow) {
            printPosition = i + 1;
            continue;
        } else if ((*SOURCE_CODE)[i] == '\0') {
            break;
        }

        (void)printf("%c", (*SOURCE_CODE)[i]);
        
        if ((*SOURCE_CODE)[i + 1] == '\n' || i + 1 == SOURCE_LENGTH) {
            (void)printf("\n");
            break;
        }
    }

    blankLength = blankLength > 32 ? 32 : blankLength;

    for (int i = 0; i < blankLength; i++) {
        (void)printf(" ");
    }

    int counter = 0;

    for (int i = printPosition; i < SOURCE_LENGTH && i - printPosition < 32; i++) {
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