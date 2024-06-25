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
 * @version 1.0     24.06.2024
 * @author Lukas Nian En Lampl
*/

/**
 * <p>
 * Defines the used booleans, 1 for true and 0 for false
 * </p>
*/
#define true 1
#define false 0

/**
 * <p>
 * Structure for storing Syntax reports.
 * The structure contains a token field, which is the place for
 * the error token. TokensToSkip says how many tokens to skip
 * until the next operation can be performed. The boolean errorOccured
 * is a flag that is set to true, when an error occured. The expectedToken
 * string is the field for defining what token was expected instead of
 * the error token.
 * </p>
*/
typedef struct SyntaxReport {
    TOKEN *token;
    int tokensToSkip;
    int errorOccured;
    char *expectedToken;
} SyntaxReport;

/**
 * <p>
 * Enum for all possible parameter types
 * </p>
*/
enum ParameterType {
    _PARAM_FUNCTION_CALL_, _PARAM_FUNCTION_, _PARAM_CLASS_
};

int SA_enter_panic_mode(TOKEN **tokens, size_t startPos, int runnableWithBlock);
SyntaxReport SA_is_runnable(TOKEN **tokens, size_t startPos, int withBlock);
int SA_handle_runnable_rep(SyntaxReport report, TOKEN **tokens, size_t startPos, int *jumper, int withBlock);

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
int SA_is_logic_operator_bracket(TOKEN **tokens, size_t startPos);
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
SyntaxReport SA_is_var_type_definition(TOKEN **tokens, size_t startPos, int functionDefinition);
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

/**
 * This flag is turned true, if an error occured
 */
int FILE_CONTAINS_ERRORS = 0;

/**
 * Holds the total length of all tokens (init during ".CheckInput()")
 */
size_t MAX_TOKEN_LENGTH = 0;

/**
 * Holds the source code
 */
extern char **BUFFER;

/**
 * Defines the length of the source code
 */
extern size_t BUFFER_LENGTH;

/**
 * Contains the name of the source file for error dumping
 */
extern char *FILE_NAME;

/**
 * Length of the tokens
 */
extern size_t TOKEN_LENGTH;

/**
 * <p>
 * Checks the token sequence received by the
 * lexer and checks for syntax errors that might have been
 * introduced by the programmer.
 * </p>
 * 
 * @param tokens    Pointer the the tokens array from the lexer
*/
int CheckInput(TOKEN **tokens) {
    if (tokens == NULL || TOKEN_LENGTH < 1) {
        (void)PARSER_TOKEN_TRANSMISSION_EXCEPTION();
        return -1;
    }

    MAX_TOKEN_LENGTH = TOKEN_LENGTH;
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
 * <p>
 * Enters the syntax analyzer into a "panic mode" and thus skips
 * all tokens, till a recover might be possible.
 * </p>
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
 * <p>
 * Checks if the passed tokens are written accordingly to the runnable
 * rule
 * </p>
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

    while (startPos + jumper <  MAX_TOKEN_LENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];
        
        if (currentToken->type == __EOF__) {
            break;
        } else if (currentToken->type == _OP_LEFT_BRACE_ && withBlock == true) {
            break;
        }

        if ((currentToken->type == _KW_IS_
            || currentToken->type == _OP_LEFT_BRACE_)
            && withBlock == 2) {
            break;
        }

        SyntaxReport isKWBasedRunnable = SA_is_keyword_based_runnable(tokens, startPos + jumper);
        int KWRet = (int)SA_handle_runnable_rep(isKWBasedRunnable, tokens, startPos, &jumper, withBlock);
        
        if (KWRet == -1) {
            return isKWBasedRunnable;
        } else if (KWRet == 1) {
            continue;
        }
        
        SyntaxReport isNKWBasedRunnable = SA_is_non_keyword_based_runnable(tokens, startPos + jumper);
        int NKWRet = (int)SA_handle_runnable_rep(isNKWBasedRunnable, tokens, startPos, &jumper, withBlock);

        if (NKWRet == -1) {
            return isNKWBasedRunnable;
        } else if (NKWRet == 1) {
            continue;
        } else {
            TOKEN *errorTok = &(*tokens)[startPos + jumper];
            return SA_create_syntax_report(errorTok, 0, true, "<ERROR>");
        }
    }

    if (withBlock == true) {
        TOKEN *finalTok = &(*tokens)[startPos + jumper];

        if  (finalTok->type != _OP_LEFT_BRACE_) {
            return SA_create_syntax_report(finalTok, 0, true, "}");
        } else {
            jumper++;
        }
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/**
 * <p>
 * Checks if the panic mode should be activated or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>-1 - Return report
 * <li>0 - Continue the loop
 * <li>1 - Non-success pass to next check
 * </ul>
 * 
 * @param report    Report to check
 * @param **tokens  Pointer to the tokens array
 * @param startPos  Position at where to runnable starts
 * @param *jumper   Pointer to the jumper / counter variable
 * @param withBlock Flag whether the code is in a block or not
 */
int SA_handle_runnable_rep(SyntaxReport report, TOKEN **tokens, size_t startPos, int *jumper, int withBlock) {
    if (report.errorOccured == true) {
        (void)SA_throw_error(report.token, report.expectedToken);
        int skip = (int)SA_enter_panic_mode(tokens, startPos + (*jumper), withBlock);

        if (skip > 0) {
            (*jumper) += skip;
            return -1;
        } else {
            if (withBlock == true) {
                return -1;
            } else {
                (*jumper)++;
                return 0;
            }
        }
    } else if (report.errorOccured == false
        && report.tokensToSkip > 0) {
        (*jumper) += report.tokensToSkip;
    }

    return 1;
}

/**
 * <p>
 * Predicts the programmers target based on expression prediction.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * print("Text");
 * a = a + 4;
 * b = getData();
 * ```
 * </p>
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_non_keyword_based_runnable(TOKEN **tokens, size_t startPos) {
    SyntaxReport rep = {NULL, -1};
    int mode = -1;
    
    if ((int)SA_predict_expression(tokens, startPos) == true) {
        mode = 0;
        rep = SA_is_expression(tokens, startPos, true);
    } else if ((int)SA_predict_class_object_access(tokens, startPos) == true) {
        return SA_is_class_object_access(tokens, startPos, true);
    } else {
        mode = 1;
        rep = SA_is_runnable_function_call(tokens, startPos);
    }

    rep = (rep.errorOccured == true && mode == 0) ? SA_is_null_assigned_class_instance(tokens, startPos) : rep;

    if (rep.errorOccured == false) {
        return rep;
    }

    char *buffer = (char*)calloc(128, sizeof(char));
    snprintf(buffer, 128, "<EXPRESSION>\" or \"<CLASS_INSTANCE>\" at \"%s", rep.expectedToken);
    return SA_create_syntax_report(rep.token, 0, true, buffer);
}

/**
 * <p>
 * Checks if a class instance is set to null.
 * </p>
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_null_assigned_class_instance(TOKEN **tokens, size_t startPos) {
    int isRootIdentifier = (int)SA_is_root_identifier(&(*tokens)[startPos]);
    int skip = 1;
    
    if (isRootIdentifier == false) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "<IDENTIFIER>");
    }

    if ((*tokens)[startPos + 1].type == _OP_RIGHT_EDGE_BRACKET_) {
        SyntaxReport isArrayAccess = SA_is_array_access(tokens, startPos + 1);

        if (isArrayAccess.errorOccured == false) {
            skip += isArrayAccess.tokensToSkip;
        } else {
            return isArrayAccess;
        }
    }

    isRootIdentifier = (int)SA_is_root_identifier(&(*tokens)[startPos + skip]);
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
 * <p>
 * If no expression could be predicted the runnable function
 * call is executed as a "replacement". It is also called if it is
 * an intended function call.
 * </p>
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
 * <p>
 * Tries to predict if the current token array sequence is a class sequence
 * or not.
 * </p>
 * 
 * @returns `True (1)` if it is a class instance, else `false (0)`
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
int SA_predict_class_instance(TOKEN **tokens, size_t startPos) {
    int jumper = 0;

    while (startPos + jumper < MAX_TOKEN_LENGTH) {
        switch ((*tokens)[startPos + jumper].type) {
        case _KW_NEW_:
            return true;
        case _OP_SEMICOLON_: case _OP_RIGHT_BRACE_: case __EOF__:
            return false;
        default:
            jumper++;
            break;
        }
    }

    return false;
}

/**
 * <p>
 * Tries to predict if the current token sequence is an expression or not.
 * </p>
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
int SA_predict_expression(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    
    while (startPos + jumper <  MAX_TOKEN_LENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        switch (currentToken->type) {
        case _OP_EQUALS_:
        case _OP_ADD_ONE_:
        case _OP_SUBTRACT_ONE_:
            return true;
        case __EOF__:
        case _OP_SEMICOLON_:
        case _OP_RIGHT_BRACE_:
            return false;
        default:
            if ((int)SA_is_assignment_operator(currentToken->value) == true) {
                return true;
            }

            jumper++;
            break;
        }
    }

    return false;
}

/**
 * <p>
 * Predicts the intended operation by the programmer and invokes
 * the predicted function.
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the class object access rule.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * iden->iden2;
 * ```
 * </p>
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_class_object_access(TOKEN **tokens, size_t startPos, int independentCall) {
    SyntaxReport leftIdentifier = SA_is_identifier(tokens, startPos);
    int skip = leftIdentifier.tokensToSkip;

    if (leftIdentifier.errorOccured == true) {
        return leftIdentifier;
    }

    if ((*tokens)[startPos + skip].type != _OP_CLASS_ACCESSOR_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, "->");
    }

    SyntaxReport rightIdentifier = SA_is_identifier(tokens, startPos + skip + 1);
    skip += rightIdentifier.tokensToSkip + 1;

    if (rightIdentifier.errorOccured == true) {
        return rightIdentifier;
    }

    if (independentCall == true) {
        if ((*tokens)[startPos + skip].type != _OP_SEMICOLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ";");
        }

        return SA_create_syntax_report(NULL, startPos + ++skip, false, NULL);
    }

    return SA_create_syntax_report(NULL, skip, false, NULL);
}

/**
 * <p>
 * Checks if a given token sequence matches the return statement.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * return null;
 * return 2 + a * b;
 * return "Hello world!";
 * return new Object();
 * return a < -1 ? b : c;
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence is a class instance creation that is
 * directly returned.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * return new Object();
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the continue statement.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * continue;
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the break statement.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * break;
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the for statement.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * for (var i = 0; i < 10; i++) {}
 * ```
 * </p>
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_for_statement(TOKEN **tokens, size_t startPos) {
    int skip = 0;
    
    if ((*tokens)[startPos].type != _KW_FOR_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "for");
    }

    if ((*tokens)[startPos + ++skip].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }
    
    skip++;
    SyntaxReport isVar = SA_is_variable(tokens, startPos + skip);
    skip += isVar.tokensToSkip;

    if (isVar.errorOccured == true) {
        return isVar;
    }

    SyntaxReport isChainedCond = SA_is_chained_condition(tokens, startPos + skip, true);
    skip += isChainedCond.tokensToSkip;

    if (isChainedCond.errorOccured == true) {
        return isChainedCond;
    }
    
    if ((*tokens)[startPos + skip].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ";");
    }

    skip++;
    SyntaxReport isExpression = SA_is_expression(tokens, startPos + skip, false);
    skip += isExpression.tokensToSkip;

    if (isExpression.errorOccured == true) {
        return isExpression;
    }

    if ((*tokens)[startPos + skip].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ")");
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + skip + 1, true);
    skip += isRunnable.tokensToSkip + 1;

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    return SA_create_syntax_report(NULL, skip, false, NULL);
}

/**
 * <p>
 * Checks if a given token sequence matches an expression.
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the if statement.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * if (a > 10) {}
 * if (object.getName() == "Thread") {}
 * ```
 * </p>
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_if_statement(TOKEN **tokens, size_t startPos) { 
    int skip = 1;
    
    if ((*tokens)[startPos].type != _KW_IF_) {
        return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "if");
    }

    if ((*tokens)[startPos + 1].type != _OP_RIGHT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + 1], 0, true, "(");
    }

    SyntaxReport isChainedCond = SA_is_chained_condition(tokens, startPos + ++skip, true);

    if (isChainedCond.errorOccured == true) {
        return isChainedCond;
    }

    skip += isChainedCond.tokensToSkip;

    if ((*tokens)[startPos + skip].type != _OP_LEFT_BRACKET_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ")");
    }
    
    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + ++skip, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    skip += isRunnable.tokensToSkip;
    return SA_create_syntax_report(NULL, skip, false, NULL);
}

/**
 * <p>
 * Checks if a given token sequence matches the else-if statement.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * else if (a > 60) {}
 * else if (b == 5 and c == 2 * 4) {}
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the else statement.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * else {}
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the do statement.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * do {} while (a == b);
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the while statement.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * while (true) {}
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence is a while condition.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * while (true)
 * while (a < 2)
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the check statement.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * check (a) {}
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the is statement.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * is true:
 * ```
 * </p>
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

    if (crucialToken->value[0] == '\"'
        || crucialToken->value[0] == '\'') {
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
 * <p>
 * Checks if a token sequence fulfills the variable definition rule.
 * </p>
 * 
 * <p>
 * All variable definitions also support the type option and visibility.
 * Examples:
 * ```
 * var a = 10;
 * var b;
 * var c[];
 * var d[] = {1, 2, 3, 4};
 * var obj = new Object();
 * var obj = new Object(param1, param2, *ptr);
 * const a = 10;
 * const b;
 * const d[] = {1, 2, 3, 4};
 * const obj = new Object();
 * const obj = new Object(param1, param2, *ptr);
 * ```
 * </p>
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens  Pointer to the TOKEN array
 * @param startPos  Position from where to start checking
*/
SyntaxReport SA_is_variable(TOKEN **tokens, size_t startPos) {
    int skip = (int)SA_skip_visibility_modifier(&(*tokens)[startPos]);
    TOKEN *varTok = &(*tokens)[startPos + skip];

    if (varTok->type == _KW_VAR_
        || varTok->type == _KW_CONST_) {
        skip++;

        if ((*tokens)[startPos + skip].type == _OP_COLON_) {
            SyntaxReport varTypeRep = SA_is_var_type_definition(tokens, startPos + skip, false);
            
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
 * <p>
 * Handles class instance variables.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * = new Object();
 * = new Object(param1, param2, *ptr);
 * ```
 * </p>
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

    SyntaxReport classPath = SA_is_identifier(tokens, startPos + skip);

    if (classPath.errorOccured == true) {
        return classPath;
    }

    skip += classPath.tokensToSkip;

    if ((*tokens)[startPos + skip].type != _OP_SEMICOLON_) {
        return SA_create_syntax_report(&(*tokens)[startPos + skip], 0, true, ";");
    }
    
    return SA_create_syntax_report(NULL, skip + 1, false, NULL);
}

/**
 * <p>
 * Handles normal variable assignments.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * = b;
 * = 10;
 * ```
 * </p>
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
 * <p>
 * Handles conditional assignments.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * a == true ? 2 : 3;
 * a == true ? b == true ? 2 : 4 : 5
 * ```
 * </p>
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
 * <p>
 * Checks if a token sequence is a chained condition or not.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * a == true and b == 2 or c == d
 * (a == true and b == 2) or c == d
 * ```
 * </p>
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
        
        if (currentToken->type == _OP_RIGHT_BRACKET_
            && (int)SA_is_logic_operator_bracket(tokens, startPos + jumper) == true) {
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
 * <p>
 * Checks if a given token sequence is a simple condition.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * a <= 2
 * b == true
 * c != 3
 * ```
 * </p>
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
 * <p>
 * Handles array variables.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * [] = {};
 * [2] = {1, 2};
 * ```
 * </p>
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
        
        switch ((*tokens)[startPos + skip + 1].type) {
        case _OP_RIGHT_BRACE_:
            rep = SA_is_array_assignment(tokens, startPos + skip);
            break;
        case _STRING_:
        case _CHARACTER_ARRAY_:
            rep = SA_create_syntax_report(NULL, 2, false, NULL);
            break;
        case _KW_NULL_:
            rep = SA_create_syntax_report(NULL, 2, false, NULL);
            break;
        default: 
            rep = SA_is_identifier(tokens, startPos + skip + 1);
            rep.tokensToSkip++;
            break;
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
 * <p>
 * Handles basic array assignments.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * = {1, 2}
 * = {}
 * ```
 * </p>
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
 * <p>
 * Handles single elements `{}` individually with recursion.
 * </p>
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
 * <p>
 * Checks if a given token sequence matches an array element.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * [2]
 * [a + b * 3]
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the constructor statement.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * this::constructor() {}
 * this::constructor(obj) {}
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the class statment.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * class Apple => {}
 * class Form extends Line => {}
 * class Window with EventListener => {}
 * class Dice extends Cube with RollEvent => {}
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the with statment.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * with ActionListener
 * with Math
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the try statement.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * try {}
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the catch statement.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * catch (Exception e) {}
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the export statement.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * export "package";
 * ```
 * </p>
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
 * <p>
 * Checks if a given token sequence matches the include statement.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * include "package";
 * ```
 * </p>
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
 * <p>
 * Checks if a tokens sequence starting from `startingPos` is written
 * accordingly to the enum syntax.
 * </p>
 * 
 * <p>
 * Example:
 * ```
 * enum test {
 *    a,
 *    b : 10,
 *    c
 * }
 * ```
 * </p>
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
 * <p>
 * Checks if the token sequence initializes enumerators in an enum.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * a
 * b : 10
 * ```
 * </p>
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
 * <p>
 * Checks whether the input tokens at the position `startPos` are aligned
 * to a definition.  
 * </p>
 * 
 * <p>
 * Examples:  
 * ```
 * function add(num1, num2) {}
 * function:int add(num1, num2) {}
 * function:int add(num1:int, num2:int) {}
 * function:int add(*num1, (*num2)) {}
 * ```
 * </p>
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

    SyntaxReport isFunctionCall = SA_is_function_call(tokens, startPos + skip + 1, true);

    if (isFunctionCall.errorOccured == true) {
        return isFunctionCall;
    }
    
    skip += isFunctionCall.tokensToSkip;

    if ((*tokens)[startPos + skip + 1].type == _OP_CLASS_ACCESSOR_) {
        SyntaxReport isTypeDefinition = SA_is_var_type_definition(tokens, startPos + skip + 1, true);

        if (isTypeDefinition.errorOccured == true) {
            return isTypeDefinition;
        }

        skip += isTypeDefinition.tokensToSkip;
    }

    SyntaxReport isRunnable = SA_is_runnable(tokens, startPos + skip + 1, true);

    if (isRunnable.errorOccured == true) {
        return isRunnable;
    }

    skip += isRunnable.tokensToSkip;
    return SA_create_syntax_report(NULL, skip + 1, false, NULL);
}

/**
 * <p>
 * Checks if a token is a visibility token or not.
 * </p>
 * 
 * <p>
 * Here is a list of visibility modifiers:
 * <ul>
 * <li>global
 * <li>secure
 * <li>private
 * </ul></p>
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
 * <p>
 * Checks if a tokens sequence starting from `startingPos` is written
 * accordingly to the function call syntax.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * add(num1, num2);
 * add(&num1, *num2);
 * add(2 + 3, 4 * 5 + a);
 * ```
 * </p>
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
 * <p>
 * Checks if a tokens sequence starting from `startingPos` is a
 * parameter or not.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * num1, num2
 * &num1, *num2
 * 2 + 3, 4 * 5 + a
 * ```
 * </p>
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
                    SyntaxReport isTypeDefinition = SA_is_var_type_definition(tokens, startPos + jumper, false);

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
 * <p>
 * Checks if the following tokens are a valid dimension definition.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * []
 * [][][][]
 * ```
 * </p>
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
 * <p>
 * Checks if a sequence is a variable type definition (cast).
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * :int
 * :char
 * :double[]
 * :Object[][]
 * ```
 * </p>
 * 
 * @returns SyntaxReport, that expresses an error or contains the tokens to skip on success
 * 
 * @param **tokens              Pointer to the TOKEN array
 * @param startPos              Position from where to start checking
 * @param functionDefinition    Flag for whether the VarType declaration is in a function or not
*/
SyntaxReport SA_is_var_type_definition(TOKEN **tokens, size_t startPos, int functionDefinition) {
    if (functionDefinition == false) {
        if ((*tokens)[startPos].type != _OP_COLON_) {
            return SA_create_syntax_report(&(*tokens)[startPos], 0, true, ":");
        }
    } else {
        if ((*tokens)[startPos].type != _OP_CLASS_ACCESSOR_) {
            return SA_create_syntax_report(&(*tokens)[startPos], 0, true, "->");
        }
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
 * <p>
 * Checks if a tokens sequence starting from `startingPos` is a simple
 * term or not.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * 1 + 2
 * 3 * 4
 * a + b * c
 * "Hello" + "World"
 * getInteger() + 2
 * ```
 * </p>
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

            if ((int)SA_predict_term_expression(tokens, startPos + jumper)) {
                isIdentifier = SA_is_term_expression(tokens, startPos + jumper);
            } else if ((int)SA_is_letter(currentToken->value[0]) == true) {
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
 * <p>
 * Handles operations like `p++`, `++p`, `p--`, `--p` or `p++++`.
 * </p>
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
 * <p>
 * Checks if a token sequence is an IDENTIFIER or not.
 * </p>
 * 
 * <p>
 * An identifier is a token sequence that is seperated with a '.'.
 * 
 * Examples:
 * ```
 * token.name.getString().getBytes()[5]
 * this.size
 * getDefaultValue().getInteger()
 * ```
 * </p>
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
                    }
                    
                    return isArrayIdentifier;
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
 * <p>
 * Checks if an input matches the array acces sequence or not.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * [0]
 * [index + offset]
 * ```
 * </p>
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
            return SA_create_syntax_report(&(*tokens)[startPos + jumper], 0, true, "]");
        }

        jumper++;
    }

    return SA_create_syntax_report(NULL, jumper, false, NULL);
}

/**
 * <p>
 * Checks if a token sequence is an array identifier.
 * </p>
 * 
 * <p>
 * An array identifier is a normal identifier with an array
 * access.
 * </p>
 * 
 * @returns SyntaxReport with the number of tokens to skip
 * and error token, if an error occured.
 * 
 * @param **tokens      Pointer to the token array
 * @param startPos      Position from where to start checking
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

/**
 * <p>
 * Checks if a given token is a root identifier.
 * </p>
 * 
 * <p>
 * By "root identifier" a normal identifier with
 * underscores, numbers and letters is meant.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - token is a root identifier
 * <li>flase - token is not a root identifier
 * 
 * @param *token    Token to check
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

/**
 * <p>
 * Checks if a given token is a number or float / double.
 * </p>
 * 
 * @returns SyntaxReport with the number of tokens to skip
 * and error token, if an error occured.
 * 
 * @param *token    Token to check
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

//////////////////////////////////////////////////
///////////////  PREDICTION UNITS  ///////////////
//////////////////////////////////////////////////
/**
 * Starting from here you'll find all prediction
 * units of the SPACE syntax analyzer.
 * A "prediction unit" is referred as a function, that
 * predicts the next rule that should apply.
 */

/**
 * <p>
 * Predicts if the next token sequence, which is surrounded by brackets
 * contains any logic operator or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - contains logic operator
 * <li>false - does not contain any logic operator
 * </ul>
 * 
 * @param **tokens  Pointer to the token array
 * @param startPos  Position from where to start checking
 */
int SA_is_logic_operator_bracket(TOKEN **tokens, size_t startPos) {
    int openBrackets = 0;
    
    for (int i = startPos; i < MAX_TOKEN_LENGTH; i++) {
        switch ((*tokens)[i].type) {
        case _KW_AND_:
        case _KW_OR_:
            return true;
        case _OP_LEFT_BRACKET_:
            openBrackets--;

            if (openBrackets <= 0) {
                return false;
            }

            break;
        case _OP_RIGHT_BRACKET_:
            openBrackets++;
            break;
        case _OP_SEMICOLON_:
        case _OP_RIGHT_BRACE_:
            return false;
        default: break;
        }
    }

    return false;
}

/**
 * <p>
 * Predicts if the next operation is a term expression.
 * </p>
 * 
 * <p>
 * As a term expression counts everything like this:
 * `p++`, `++p`, `p--`, `--p` or `p++++`.
 * </p>
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
 * <p>
 * Checks if the next tokens reference to a class access or not.
 * </p>
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
        case _OP_EQUALS_:
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
 * <p>
 * Predicts whether the next tokens are an array access or not.
 * </p>
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

//////////////////////////////////////////////////
////////////////  BASE FUNCTIONS  ////////////////
//////////////////////////////////////////////////
/**
 * Everything below this section is the start
 * of base function implementation, which means the
 * absolute / atomic parts.
 */


/**
 * <p>
 * Checks if a token is a string.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - token is a string
 * <li>false - token is not a string
 * </ul>
 * 
 * @param *token    Pointer to the token to check
 */
int SA_is_string(TOKEN *token) {
    return (token->type == _STRING_ || token->type == _CHARACTER_ARRAY_) ?
        true : false;
}

/**
 * <p>
 * Checks if a token is a reference.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - token is a reference
 * <li>false - token is not a reference
 * </ul>
 * 
 * @param *token    Pointer to the token to check
 */
int SA_is_reference(TOKEN *token) {
    if (token->type == _REFERENCE_ || token->type == _REFERENCE_ON_POINTER_) {
        return true;
    }

    return false;
}

/**
 * <p>
 * Checks if a token is a pointer.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - token is a pointer
 * <li>false - token is not a pointer
 * </ul>
 * 
 * @param *token    Pointer to the token to check
 */
int SA_is_pointer(const TOKEN *token) {
    return token->type == _POINTER_ ? true : false;
}

/**
 * <p>
 * Checks if a character is a letter.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - character is a letter
 * <li>false - character is not a letter
 * </ul>
 * 
 * @param character     Character to check
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

/**
 * <p>
 * Checks if a character is a digit.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - character is a number
 * <li>false - character is not a number
 * </ul>
 * 
 * @param character     Character to check
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

/**
 * <p>
 * The list of all rational operators of the SPACE language
 * (at first version).
 * </p>
 */
const char rationalOperators[][3] = {"==", "<=", ">=", "!=", "<", ">"};

/**
 * <p>
 * Checks if a character sequence is indicating a rational operator.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - is a rational operator
 * <li>false - is not a rational operator
 * </ul>
 * 
 * @param *sequence     Sequence to check
 */
int SA_is_rational_operator(const char *sequence) {
    int size = (sizeof(rationalOperators) / sizeof(rationalOperators[0]));

    for (int i = 0; i < size; i++) {
        if ((int)strcmp(sequence, rationalOperators[i]) == 0) {
            return true;
        }
    }

    return false;
}

/**
 * <p>
 * Checks if a token is an arithmetic operator.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - token is an arithmetic operator
 * <li>false - token is not an arithmetic operator
 * </ul>
 * 
 * @param *token    Token to check
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

/**
 * <p>
 * Checks if a character sequence matches an assignment operator.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - is an assignment operator
 * <li>false - is not an assignment operator
 * </ul>
 * 
 * @param *sequence     Sequence to check
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

/**
 * <p>
 * Checks if a character is an underscore or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - is an underscore
 * <li>false - is not an underscore
 * </ul>
 * 
 * @param character     Character to check
 */
int SA_is_underscore(const char character) {
    return character == '_' ? true : false;
}

/**
 * <p>
 * Checks if a character sequence matches a boolean.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - is a boolean
 * <li>false - is not a boolean
 * </ul>
 * 
 * @param *sequence     Sequence to check
 */
int SA_is_bool(const char *sequence) {
    return ((int)strcmp(sequence, "true") == 0
        || (int)strcmp(sequence, "false") == 0) ? true : false;
}

/**
 * <p>
 * Checks if a character sequence matches a modifier.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - is modifier
 * <li>false - is not a modifier
 * </ul>
 * 
 * @param *sequence     Sequence to check
 */
int SA_is_modifier(const char *sequence) {
    return ((int)strcmp(sequence, "global") == 0
            || (int)strcmp(sequence, "local") == 0
            || (int)strcmp(sequence, "secure") == 0) ? true : false;
}

/**
 * <p>
 * Checks if a character sequence matches a logic operator.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - is logic operator
 * <li>false - is not a logic operator
 * </ul>
 * 
 * @param *sequence     Sequence to evaluate
 */
int SA_is_logic_operator(const char *sequence) {
    return ((int)strcmp(sequence, "and") == 0
            || (int)strcmp(sequence, "or") == 0
            || (int)strcmp(sequence, "!") == 0) ? true : false;
}

/**
 * <p>
 * Creates a SyntaxReport structure containing all data to throw an error.
 * The SyntaxReport contains the error token, how many tokens to skip, if
 * a run was successful and the expected token.
 * </p>
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

    if (BUFFER == NULL) {
        (void)printf("Source code pointer = NULL!");
        return;
    }

    int errorCharsAwayFromNL = 0;
    size_t errorLine = errorToken->line + 1;

    for (int i = errorToken->tokenStart; i > 0; i--, errorCharsAwayFromNL++) {
        if ((*BUFFER)[i - 1] == '\n' || (*BUFFER)[i - 1] == '\0') {
            break;
        }
    }

    (void)printf(TEXT_COLOR_RED);
    (void)printf("SYNTAX ERROR occured on line ");
    (void)printf(TEXT_COLOR_BLUE);
    (void)printf(TEXT_UNDERLINE);
    (void)printf("%li:%i", errorLine, errorCharsAwayFromNL);
    (void)printf(TEXT_COLOR_RESET);
    (void)printf(TEXT_COLOR_RED);
    (void)printf(" in \"%s\"\n", FILE_NAME);
    
    char buffer[32];
    int tokPos = ((errorToken->tokenStart + 1) - errorCharsAwayFromNL);
    int blankLength = (int)snprintf(buffer, 32, "%li : %i | ", errorLine, tokPos);
    (void)printf("%s", buffer);
    (void)printf(TEXT_COLOR_GRAY);

    for (int i = errorToken->tokenStart - errorCharsAwayFromNL; i < BUFFER_LENGTH; i++) {
        if ((*BUFFER)[i] == '\n' || (*BUFFER)[i] == '\0') {
            break;
        }

        (void)printf("%c", (*BUFFER)[i]);
    }

    (void)printf("\n");
    (void)printf(TEXT_COLOR_YELLOW);

    for (int i = 0; i < blankLength + errorCharsAwayFromNL; i++) {
        (void)printf(" ");
    }

    for (int i = 0; i < errorToken->size - 1; i++) {
        (void)printf("^");
    }

    (void)printf(TEXT_COLOR_RED);
    (void)printf("\n");
    (void)printf("    Unexpected token \"%s\",\n", errorToken->value);
    (void)printf("    maybe replace with \"%s\".\n", expectedToken);
    (void)printf("\n\n");
    (void)printf(TEXT_COLOR_RESET);
}