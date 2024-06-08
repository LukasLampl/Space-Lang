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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "../headers/modules.h"
#include "../headers/errors.h"
#include "../headers/parsetree.h"
#include "../headers/Token.h"

/**
 * The subprogram {@code SPACE.src.parsetreeGenerator} was created
 * to provide the parsetree generator for the SPACE Language. The
 * procedure is equal to the procedure used in the
 * {@code SPACE.src.syntaxAnalyzer}.
 * 
 * @see SPACE.src.parseTreeGenerator.md
 * 
 * @version 1.0     06.06.2024
 * @author Lukas Nian En Lampl
*/

/**
 * Defines the used booleans, 1 for true and 0 for false
*/
#define true 1
#define false 0

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////     PARSE TREE GENERATOR     ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

const int UNINITIALZED = -1;

/**
 * Defines a NodeReport, the basic unit of the parsetree generator.
 * A NodeReport contains the top node (referred as the `.node`)
 * and how many tokens got processed and thus can be skipped.
*/
typedef struct NodeReport {
    struct Node *node;
    size_t tokensToSkip;
} NodeReport;

/**
 * The idenValRet is a structure dedicated for handling identifiers.
 * It holds the value of the identifier (a variable name for instance)
 * and also how many tokens were processed and thus can be skipped.
*/
struct idenValRet {
    char *value;
    size_t movedTokens;
};

/**
 * The VAR_TYPE enum provides all variable types that can be processed
 * by the parsetree generator. Before processing the variable type is
 * searched in {@code #get_var_type()} then based on the enumerator
 * the individual process is invoked.
 * 
 * @note Here is a overview to the different types (examples):
 * @note - UDEF => No type could be identified
 * @note - NORMAL_VAR => `var a = 10;`
 * @note - ARRAY_VAR => `var arr[];` or `var arr[] = {1, 2, 3}`
 * @note - COND_VAR => `var a = b <= 2 ? 1 : 2;`
 * @note - INSTANCE_VAR => `var obj = new Object();`
*/
enum VAR_TYPE {
    UNDEF,
    NORMAL_VAR,
    ARRAY_VAR,
    COND_VAR,
    INSTANCE_VAR
};

/**
 * Enum for identifying the runnable type.
 * - InBlock is used, if the runnable is in a block statment.
 * - SwitchStatement, when the runnable is in a switch statment.
 * - IsStatement, when the runnable is in a is statement.
*/
enum RUNNABLE_TYPE {
    InBlock,
    SwitchStatement,
    IsStatement
};

enum processDirection {
    LEFT,
    STAY,
    RIGHT
};

///// FUNCTIONS PROTOTYPES /////

void PG_print_cpu_time(float cpu_time_used);
void PG_append_node_to_root_node(struct Node *node);
NodeReport PG_create_runnable_tree(TOKEN **tokens, size_t startPos, enum RUNNABLE_TYPE type);
NodeReport PG_get_report_based_on_token(TOKEN **tokens, size_t startPos, enum RUNNABLE_TYPE type);
int PG_predict_function_call(TOKEN **tokens, size_t startPos);
NodeReport PG_create_else_statement_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_else_if_statement_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_if_statement_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_for_statement_tree(TOKEN **tokens, size_t startPos);
int PG_predict_assignment(TOKEN **tokens, size_t startPos);
int PG_get_term_bounds(TOKEN **tokens, size_t startPos);
enum NodeType PG_get_nodeType_of_operator(TOKENTYPES type);
NodeReport PG_create_simple_assignment_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_is_statement_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_check_statement_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_abort_operation_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_return_statement_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_do_statement_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_while_statement_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_variable_tree(TOKEN **tokens, size_t startPos);
enum VAR_TYPE get_var_type(TOKEN **tokens, size_t startPos);
NodeReport PG_create_varType_tree(TOKEN *typeToken);
NodeReport PG_create_class_instance_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_instance_var_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_condition_assignment_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_conditional_var_tree(TOKEN **tokens, size_t startPos);
int PG_get_cond_assignment_bounds(TOKEN **tokens, size_t startPos);
NodeReport PG_create_array_var_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_array_init_tree(TOKEN **tokens, size_t startPos, int dim);
int PG_predict_array_init_count(TOKEN **tokens, size_t startPos);
int PG_get_array_element_size(TOKEN **tokens, size_t startPos);
int PG_add_dimensions_to_var_node(struct Node *node, TOKEN **tokens, size_t startPos, int offset);
int PG_get_dimension_count(TOKEN **tokens, size_t startPos);
NodeReport PG_create_normal_var_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_condition_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_conditional_assignment_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_chained_condition_tree(TOKEN **tokens, size_t startPos);
int PG_get_condition_iden_length(TOKEN **tokens, size_t startPos);
int PG_is_condition_operator(TOKENTYPES type);
NodeReport PG_create_class_constructor_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_class_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_try_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_catch_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_export_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_include_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_enum_tree(TOKEN **tokens, size_t startPos);
int PG_predict_enumerator_count(TOKEN **tokens, size_t starPos);
NodeReport PG_create_function_tree(TOKEN **tokens, size_t startPos);
size_t PG_get_size_till_next_semicolon(TOKEN **tokens, size_t startPos);
NodeReport PG_create_array_access_tree(TOKEN **tokens, size_t startPos);
NodeReport PG_create_function_call_tree(TOKEN **tokens, size_t startPos);
size_t PG_add_params_to_node(struct Node *node, TOKEN **tokens, size_t startPos, int addStart, enum NodeType stdType);
int PG_get_bound_of_single_param(TOKEN **tokens, size_t startPos);
enum NodeType PG_get_node_type_by_value(char **value);
NodeReport PG_create_simple_term_node(TOKEN **tokens, size_t startPos, size_t boundaries);
NodeReport PG_assign_processed_node_to_node(TOKEN **tokens, size_t startPos);
size_t PG_go_backwards_till_operator(TOKEN **tokens, size_t startPos);
int PG_determine_bounds_for_capsulated_term(TOKEN **tokens, size_t startPos);
int PG_is_next_operator_multiply_divide_or_modulo(TOKEN **tokens, size_t startPos);
int PG_is_next_iden_a_member_access(TOKEN **tokens, size_t startPos);
NodeReport PG_create_member_access_tree(TOKEN **tokens, size_t startPos);
struct idenValRet PG_get_identifier_by_index(TOKEN **tokens, size_t startPos);
int PG_is_function_call(TOKEN **tokens, size_t startPos);
NodeReport PG_get_member_access_side_node_tree(TOKEN **tokens, size_t startPos, enum processDirection direction);
int PG_propagate_back_till_iden(TOKEN **tokens, size_t startPos);
int PG_propagate_offset_by_direction(TOKEN **tokens, size_t startPos, enum processDirection direction);
int PG_back_shift_array_access(TOKEN **tokens, size_t startPos);
int PG_predict_argument_count(TOKEN **tokens, size_t startPos, int withPredefinedBrackets);
int PG_is_operator(const TOKEN *token);
int PG_add_varType_definition(TOKEN **tokens, size_t startPos, struct Node *parentNode);
struct Node *PG_create_modifier_node(TOKEN *token, int *skip);
struct Node *PG_create_node(char *value, enum NodeType type, int line, int pos);
NodeReport PG_create_node_report(struct Node *topNode, int tokensToSkip);
void PG_allocate_node_details(struct Node *node, size_t size, int resize);
void PG_print_from_top_node(struct Node *topNode, int depth, int pos);
int FREE_NODES();
void FREE_NODE(struct Node *node);

/**
 * This variable is the root of the total parsetree.
*/
struct RootNode RootNode;

/**
 * Holds the size of the token array.
*/
size_t TOKENLENGTH = 0;

/**
 * @brief This is the entrypoint of the parsetree.
 * 
 * @param **tokens  Pointer to the token array
 * @param TokenLength   Length of the token array
*/
int Generate_Parsetree(TOKEN **tokens, size_t TokenLength) {
    TOKENLENGTH = TokenLength;

    (void)printf("\n\n\n>>>>>>>>>>>>>>>>>>>>    PARSETREE    <<<<<<<<<<<<<<<<<<<<\n\n");

    if (tokens == NULL || TokenLength == 0) {
        (void)PARSER_TOKEN_TRANSMISSION_EXCEPTION();
    }

    RootNode.nodeCount = 0;
    printf("TOKENLENGTH: %li\n", TokenLength);

    // CLOCK FOR DEBUG PURPOSES ONLY!!
    clock_t start, end;

    if (PARSETREE_GENERATOR_DISPLAY_USED_TIME == 1) {
        start = (clock_t)clock();
    }

    //Tree generation process
    NodeReport runnable = PG_create_runnable_tree(tokens, 0, false);

    if (PARSETREE_GENERATOR_DISPLAY_USED_TIME == 1) {
        end = (clock_t)clock();            
    }
    
    if (PARSETREE_GENERATOR_DEBUG_MODE == 1) {
        if (runnable.node == NULL) {
            printf("Something went wrong in the parsetree generation step.");
        } else {
            (void)PG_print_from_top_node(runnable.node, 0, 0);
        }
    }

    if (PARSETREE_GENERATOR_DISPLAY_USED_TIME == 1) {
        (void)PG_print_cpu_time(((double) (end - start)) / CLOCKS_PER_SEC);
    }

    if (runnable.node == NULL) {
        printf("Something went wrong (PG)!\n");
    }

    (void)printf("\n\n\n>>>>>    Tokens converted to tree    <<<<<\n\n");

    return 1;
}

/*
Purpose: Print the used CPU time
Return Type: void
Params: float cpu_time_used => Time to be printed
*/
void PG_print_cpu_time(float cpu_time_used) {
    (void)printf("\nCPU time used for PARSETREE GENERATION: %f seconds\n", cpu_time_used);
}

/*
Purpose: Generate a subtree for a function definition
Return Type: NodeReport => Contains the root node of the subtree and the number of tokens to skip
Params: TOKEN **tokens => Pointer to the array of tokens;
        size_t startPos => The position from where to start constructing
_______________________________
Layout:

 [RUNNABLE]
     |
[STATEMENT]
[EXPRESSION]

The [RUNNABLE] is created as a fully independent node,
whose [STATEMENT] and [EXPRESSION] con be found in
´´´node->details[position]´´´.

[STATEMENT]: Statements in the source
[EXPRESSION]: Expressions to run
_______________________________
*/
NodeReport PG_create_runnable_tree(TOKEN **tokens, size_t startPos, enum RUNNABLE_TYPE type) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *parentNode = PG_create_node("RUNNABLE", _RUNNABLE_NODE_, token->line, token->tokenStart);
    int argumentCount = 0;
    size_t jumper = 0;
    
    while (startPos + jumper < TOKENLENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken->type == _OP_LEFT_BRACE_
            && (type == InBlock || type == SwitchStatement
            || type == IsStatement)) {
            jumper++;
            break;
        } else if (currentToken->type == __EOF__) {
            break;
        } else if (currentToken->type == _KW_IS_
            && type == IsStatement) {
            jumper++;
            break;
        }

        NodeReport report = PG_get_report_based_on_token(tokens, startPos + jumper, type);

        if (report.node != NULL) {
            if (argumentCount == 0) {
                (void)PG_allocate_node_details(parentNode, 1, false);
            } else {
                struct Node **temp = (struct Node**)realloc(parentNode->details, sizeof(struct Node) * argumentCount + 1);

                if (temp == NULL) {
                    free(parentNode->details);
                    FREE_MEMORY();
                    printf("Something went wrong (runnable)!\n");
                    exit(EXIT_FAILURE);
                }

                parentNode->details = temp;
                parentNode->detailsCount = argumentCount + 1;
            }

            parentNode->details[argumentCount++] = report.node;
            jumper += report.tokensToSkip;
        } else {
            jumper++;
        }
    }

    return PG_create_node_report(parentNode, jumper);
}

/*
Purpose: Get the correct NodeReport based on a token
Return Type: NodeReport => NodeReport the called function returns
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position where the crucial token is located
        and from where to start constructing subtrees
*/
NodeReport PG_get_report_based_on_token(TOKEN **tokens, size_t startPos, enum RUNNABLE_TYPE type) {
    switch ((*tokens)[startPos].type) {
    case _KW_VAR_:
    case _KW_CONST_:
        return PG_create_variable_tree(tokens, startPos);
    case _KW_INCLUDE_:
        return PG_create_include_tree(tokens, startPos);
    case _KW_EXPORT_:
        return PG_create_export_tree(tokens, startPos);
    case _KW_FOR_:
        return PG_create_for_statement_tree(tokens, startPos);
    case _KW_ENUM_:
        return PG_create_enum_tree(tokens, startPos);
    case _KW_FUNCTION_:
        return PG_create_function_tree(tokens, startPos);
    case _KW_CATCH_:
        return PG_create_catch_tree(tokens, startPos);
    case _KW_TRY_:
        return PG_create_try_tree(tokens, startPos);
    case _KW_CLASS_:
        return PG_create_class_tree(tokens, startPos);
    case _KW_WHILE_:
        return PG_create_while_statement_tree(tokens, startPos);
    case _KW_DO_:
        return PG_create_do_statement_tree(tokens, startPos);
    case _KW_CHECK_:
        return PG_create_check_statement_tree(tokens, startPos);
    case _KW_IS_:
        if (type == SwitchStatement) {
            return PG_create_is_statement_tree(tokens, startPos);
        }
        break;
    case _KW_IF_:
        return PG_create_if_statement_tree(tokens, startPos);
    case _KW_ELSE_:
        if ((*tokens)[startPos + 1].type == _KW_IF_) {
            return PG_create_else_if_statement_tree(tokens, startPos);
        } else {
            return PG_create_else_statement_tree(tokens, startPos);
        }
    case _KW_CONTINUE_:
    case _KW_BREAK_:
        return PG_create_abort_operation_tree(tokens, startPos);
    case _KW_RETURN_:
        return PG_create_return_statement_tree(tokens, startPos);
    case _KW_GLOBAL_:
    case _KW_SECURE_:
    case _KW_PRIVATE_:
        if ((*tokens)[startPos + 1].type == _KW_FUNCTION_) {
            return PG_create_function_tree(tokens, startPos);
        } else if ((*tokens)[startPos + 1].type == _KW_CLASS_) {
            return PG_create_class_tree(tokens, startPos);
        } else if ((*tokens)[startPos + 1].type == _KW_VAR_
            || (*tokens)[startPos + 1].type == _KW_CONST_) {
            return PG_create_variable_tree(tokens, startPos);
        }
        break;
    default:
        if ((*tokens)[startPos].type == _KW_THIS_
            && (*tokens)[startPos + 3].type == _KW_CONSTRUCTOR_) {
            return PG_create_class_constructor_tree(tokens, startPos);
        } else if ((*tokens)[startPos].type == _OP_SEMICOLON_) {
            break;
        }

        if ((int)PG_predict_assignment(tokens, startPos) == true) {
            return PG_create_simple_assignment_tree(tokens, startPos);
        }

        int fncCallBounds = (int)PG_predict_function_call(tokens, startPos);

        if (fncCallBounds > 0) {
            return PG_create_simple_term_node(tokens, startPos, fncCallBounds);
        }

        break;
    }

    return PG_create_node_report(NULL, UNINITIALZED);
}

int PG_predict_function_call(TOKEN **tokens, size_t startPos) {
    int counter = 0;

    while (startPos + counter < TOKENLENGTH) {
        TOKEN *token = &(*tokens)[startPos + counter];

        if (token->type == _OP_SEMICOLON_) {
            break;
        }

        counter++;
    }

    return counter;
}

/*
Purpose: Creates a subtree for an else statement
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
[ELSE_STMT]
     \
  [RUNNABLE]

The [ELSE_STMT] contains the runnable at
```node->rightNode```.

[ELSE_STMT]: Indicator for the else statment
[RUNNABLE]: Runnable of the else statement
*/
NodeReport PG_create_else_statement_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *node = PG_create_node(token->value, _ELSE_STMT_NODE_, token->line, token->tokenStart);
    int skip = 2;
    
    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + skip, InBlock);
    node->rightNode = runnableReport.node;
    skip += runnableReport.tokensToSkip;

    return PG_create_node_report(node, skip);
}

/*
Purpose: Creates a subtree for an else-if statement
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
    [EIF_STMT]
   /          \
[COND]     [RUNNABLE]

The [EIF_STMT] contains the condition at
```node->leftNode``` and the runnable at
```node->rightNode```.

[EIF_STMT]: Indicator for the else-if statment
[COND]: Condition that has to be met to run the runnable
[RUNNABLE]: Runnable of the else-if statement
*/
NodeReport PG_create_else_if_statement_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *node = PG_create_node(token->value, _ELSE_IF_STMT_NODE_, token->line, token->tokenStart);
    int skip = 0;

    NodeReport chainedCondReport = PG_create_chained_condition_tree(tokens, startPos + 3);
    node->leftNode = chainedCondReport.node;
    skip += chainedCondReport.tokensToSkip + 4;

    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + skip, InBlock);
    node->rightNode = runnableReport.node;
    skip += runnableReport.tokensToSkip;

    return PG_create_node_report(node, skip);
}

/*
Purpose: Creates a subtree for an if statement
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
    [IF_STMT]
   /         \
[COND]     [RUNNABLE]

The [IF_STMT] contains the condition at
```node->leftNode``` and the runnable at
```node->rightNode```.

[IF_STMT]: Indicator for the if statment
[COND]: Condition that has to be met to run the runnable
[RUNNABLE]: Runnable of the if statement
*/
NodeReport PG_create_if_statement_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *node = PG_create_node(token->value, _IF_STMT_NODE_, token->line, token->tokenStart);
    int skip = 0;

    NodeReport chainedCondReport = PG_create_chained_condition_tree(tokens, startPos + 2);
    node->leftNode = chainedCondReport.node;
    skip += chainedCondReport.tokensToSkip + 3;

    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + skip, InBlock);
    node->rightNode = runnableReport.node;
    skip += runnableReport.tokensToSkip;

    return PG_create_node_report(node, skip);
}

/*
Purpose: Creates a subtree for a for statement
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
   [FOR_STMT]
   /    |   \
[VAR] [COND] [RUNNABLE]
     [ACTION]

The [FOR_STMT] contains the var to count for at
```node->leftNode``` and the conditions at
```node->details[0]``` and action to fulfill (incrementing)
as well at ```node->details[1]```.

[FOR_STMT]: Indicator for the for statment
[VAR]: Var to use as "counter" or iterator
[COND]: Condition that has to be met to run the loop
[ACTION]: Actions that occur at every iteration, like incrementing
[RUNNABLE]: Runnable in the the loop itself
*/
NodeReport PG_create_for_statement_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *topNode = PG_create_node("FOR", _FOR_STMT_NODE_, token->line, token->tokenStart);
    (void)PG_allocate_node_details(topNode, 2, false);
    int skip = 0;
    /*
    > for (var i = 0; i < 10; i++) {}
    >      ^
    > (*tokens)[startPos + 2]
    */

    NodeReport varReport = PG_create_variable_tree(tokens, startPos + 2);
    topNode->leftNode = varReport.node;
    skip += varReport.tokensToSkip + 3; //+1 for ';'

    NodeReport chainedReport = PG_create_chained_condition_tree(tokens, startPos + skip);
    topNode->details[0] = chainedReport.node;
    skip += chainedReport.tokensToSkip;
    
    NodeReport expressionReport = PG_create_simple_assignment_tree(tokens, startPos + skip);
    topNode->details[1] = expressionReport.node;
    skip += expressionReport.tokensToSkip + 2;

    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + skip, InBlock);
    topNode->rightNode = runnableReport.node;
    skip += runnableReport.tokensToSkip;

    return PG_create_node_report(topNode, skip);
}

/*
Purpose: Check whether an assignment follows or not
Return Type: int => true = is assignment; false = not an assignment
Params: TOKEN **tokens => Point to tokens array with tokens to check;
        size_t startPos => Position from where to start checking
*/
int PG_predict_assignment(TOKEN **tokens, size_t startPos) {
    for (int i = startPos; i < TOKENLENGTH; i++) {
        if ((*tokens)[i].type == _OP_SEMICOLON_) {
            break;
        } else if ((*tokens)[i].type == _OP_EQUALS_
            || (*tokens)[i].type == _OP_PLUS_EQUALS_
            || (*tokens)[i].type == _OP_MINUS_EQUALS_
            || (*tokens)[i].type == _OP_ADD_ONE_
            || (*tokens)[i].type == _OP_SUBTRACT_ONE_
            || (*tokens)[i].type == _OP_MULTIPLY_EQUALS_
            ||(*tokens)[i].type == _OP_DIVIDE_EQUALS_) {
            return true;
        }
    }

    return false;
}

/*
Purpose: Convert TOKENTYPES (Operators only) to NodeType
Return Type: enum NodeType => Converted NodeType
Params: TOKENTYPES type => Type to convert
*/
enum NodeType PG_get_nodeType_of_operator(TOKENTYPES type) {
    switch (type) {
    case _OP_SUBTRACT_ONE_:
        return _DECREMENT_ONE_NODE_;
    case _OP_ADD_ONE_:
        return _INCREMENT_ONE_NODE_;
    case _OP_PLUS_EQUALS_:
        return _PLUS_EQUALS_NODE_;
    case _OP_MINUS_EQUALS_:
        return _MINUS_EQUALS_NODE_;
    case _OP_MULTIPLY_EQUALS_:
        return _MULTIPLY_EQUALS_NODE_;
    case _OP_DIVIDE_EQUALS_:
        return _DIVIDE_EQUALS_NODE_;
    case _OP_EQUALS_:
        return _EQUALS_NODE_;
    default:
        return _NULL_;
    }
}

/*
Purpose: Get the bounds of a simple term
Return Type: int => Boundaries
Params: TOKEN **tokens => Pointer to tokens on which the bound is predicted from;
        size_t startPos => Position from where to start counting
*/
int PG_get_term_bounds(TOKEN **tokens, size_t startPos) {
    for (int i = startPos; i < TOKENLENGTH; i++) {
        switch ((*tokens)[i].type) {
            case _OP_SEMICOLON_:
            case _OP_EQUALS_:
            case _OP_PLUS_EQUALS_:
            case _OP_MINUS_EQUALS_:
            case _OP_MULTIPLY_EQUALS_:
            case _OP_DIVIDE_EQUALS_:
            case _OP_LEFT_EDGE_BRACKET_:
            case _OP_LEFT_BRACE_:
            case _OP_RIGHT_EDGE_BRACKET_:
                return i - startPos;
            default: continue;
        }
    }

    return -1;
}

/*
Purpose: Creates a subtree for an assignment
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
  [ASS_TYPE]
   /     \
[VAR]   [VAL]

The [ASS_TYPE] contains the var to change at
```node->leftNode``` and the value to assign at
```node-rightNode```.

[ASS_TYPE]: Determines the assignment type
[VAL]: Value to assign
*/
NodeReport PG_create_simple_assignment_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos + 1];
    struct Node *operatorNode = NULL;
    token = &(*tokens)[startPos];
    int skip = 0;

    //Array assignment handling
    if ((*tokens)[startPos + 1].type == _OP_RIGHT_EDGE_BRACKET_) {
        operatorNode = PG_create_node((*tokens)[startPos + skip].value, PG_get_nodeType_of_operator((*tokens)[startPos + skip].type), token->line, token->tokenStart);
        operatorNode->leftNode = PG_create_node(token->value, _IDEN_NODE_, token->line, token->tokenStart);
        int dimCount = PG_get_dimension_count(tokens, startPos + 1);
        (void)PG_allocate_node_details(operatorNode->leftNode, dimCount, false);
        skip += (int)PG_add_dimensions_to_var_node(operatorNode->leftNode, tokens, startPos + skip, 0);
    //Member acces handling
    } else {
        int bounds = PG_get_term_bounds(tokens, startPos);
        NodeReport termReport = PG_create_simple_term_node(tokens, startPos, bounds);
        skip += termReport.tokensToSkip;
        operatorNode = PG_create_node((*tokens)[startPos + skip].value, PG_get_nodeType_of_operator((*tokens)[startPos + skip].type), token->line, token->tokenStart);
        operatorNode->leftNode = termReport.node;
    }

    skip++;
    NodeReport rep = {NULL, UNINITIALZED};
    
    if (get_var_type(tokens, startPos + skip) == COND_VAR) {
        rep = PG_create_condition_assignment_tree(tokens, startPos + skip);
    } else {
        int bounds = (int)PG_get_term_bounds(tokens, startPos + skip);
        rep = PG_create_simple_term_node(tokens, startPos + skip, bounds);
    }

    operatorNode->rightNode = rep.node;
    skip += rep.tokensToSkip;

    return PG_create_node_report(operatorNode, skip);
}

/*
Purpose: Creates a subtree for a is statement
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
[IS_STMT]
    |
[RUNNABLE]

The indicator node [IS_STMT] has the value to check and the
```node->details[i]``` is filled with the runnable block.

[IS_STMT]: Indicator for the is statement with value to check against
[RUNNABLE]: Runnable in the is statement section
*/
NodeReport PG_create_is_statement_tree(TOKEN **tokens, size_t startPos) {
    /*
    > is 3:
    >    ^
    > (*tokens)[startPos + 2]
    */
    
    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + 3, IsStatement);
    struct Node *topNode = runnableReport.node;
    topNode->value = (*tokens)[startPos + 1].value;
    topNode->type = _IS_STMT_NODE_;

    return PG_create_node_report(topNode, runnableReport.tokensToSkip + 1);
}

/*
Purpose: Creates a subtree for a check statement
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
[CHECK_STMT]
     |
   [CASE]

The indicator node [CHECK_STMT] has the value to check at
´´´node->leftNode´´´ and the cases in ```node->details[i]```.

[CHECK_STMT]: Indicator for the check statement with var to check
[CASE]: Cases
*/
NodeReport PG_create_check_statement_tree(TOKEN **tokens, size_t startPos) {
    /*
    > check (a) {}
    >        ^
    > (*tokens)[startPos + 2]
    */

    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + 4, SwitchStatement);
    struct Node *topNode = runnableReport.node;
    topNode->value = (*tokens)[startPos + 2].value;
    topNode->type = _CHECK_STMT_NODE_;
    
    return PG_create_node_report(topNode, runnableReport.tokensToSkip + 4);
}

/*
Purpose: Creates a Node for a abort operation (BREAK, CONTINUE)
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
*/
NodeReport PG_create_abort_operation_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *topNode = NULL;

    if ((*tokens)[startPos].type == _KW_CONTINUE_) {
        topNode = PG_create_node("CONTINUE", _CONTINUE_STMT_NODE_, token->line, token->tokenStart);
    } else if ((*tokens)[startPos].type == _KW_BREAK_) {
        topNode = PG_create_node("BREAK", _BREAK_STMT_NODE_, token->line, token->tokenStart);
    }

    return PG_create_node_report(topNode, 2);
}

/*
Purpose: Creates a subtree for a return statement
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
    [RET_STMT]
    /
[RET]

The indicator node [RET_STMT] has the return value at
´´´node->leftNode´´´.

[RET_STMT]: Indicator for the return statement
[RET]: Return value
*/
NodeReport PG_create_return_statement_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *topNode = PG_create_node("RETURN_STATMENT", _RETURN_STMT_NODE_, token->line, token->tokenStart);
    int skip = 0;
    
    if ((*tokens)[startPos + 1].type == _KW_NEW_) {
        NodeReport classInstanceReport = PG_create_class_instance_tree(tokens, startPos + 1);
        topNode->leftNode = classInstanceReport.node;
        skip += classInstanceReport.tokensToSkip + 1;
    } else if (get_var_type(tokens, startPos + 1) == COND_VAR) {
        NodeReport condReport = PG_create_condition_assignment_tree(tokens, startPos + 1);
        topNode->leftNode = condReport.node;
        skip += condReport.tokensToSkip;
    } else {
        int bounds = (int)PG_get_term_bounds(tokens, startPos + 1);
        NodeReport termReport = PG_create_simple_term_node(tokens, startPos + 1, bounds);
        topNode->leftNode = termReport.node;
        skip += termReport.tokensToSkip + 1;
    }

    return PG_create_node_report(topNode, skip);
}

/*
Purpose: Creates a subtree for a do statement
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
    [DO_STMT]
    /       \
[COND]   [RUNNABLE]

The indicator node [DO_STMT] has the conditions at
´´´node->leftNode´´´ and the runnable block in
´´´node->rightNode´´´.

[DO_STMT]: Indicator for the do statement
[COND]: Chained condition to be fulfilled
[RUNNABLE]: Block in the while statment
*/
NodeReport PG_create_do_statement_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *topNode = PG_create_node("DO_STMT", _DO_STMT_NODE_, token->line, token->tokenStart);
    int skip = 2;

    /*
    > do { }
    >     ^
    > (*tokens)[startPos + skip]
    */

    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + skip, InBlock);
    topNode->rightNode = runnableReport.node;
    skip += runnableReport.tokensToSkip + 2;

    /*
    > do {} while (a == 2);
    >              ^
    >  (*tokens)[startPos + skip + 2]
    */

    NodeReport chainedCondReport = PG_create_chained_condition_tree(tokens, startPos + skip);
    topNode->leftNode = chainedCondReport.node;
    skip += chainedCondReport.tokensToSkip + 1; //Skip the ';'
    return PG_create_node_report(topNode, skip);
}

/*
Purpose: Creates a subtree for a while statement
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Point to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:
 
   [WHILE_STMT]
    /       \
[COND]   [RUNNABLE]

The indicator node [WHILE_STMT] has the conditions at
´´´node->leftNode´´´ and the runnable block in
´´´node->rightNode´´´.

[WHILE_STMT]: Indicator for the while statement
[COND]: Chained condition to be fulfilled
[RUNNABLE]: Block in the while statment
*/
NodeReport PG_create_while_statement_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *topNode = PG_create_node("WHILE_STMT", _WHILE_STMT_NODE_, token->line, token->tokenStart);
    int skip = 2;

    /*
    > while (a == 2) {}
    >        ^
    >   (*tokens)[startPos + skip]
    */

    NodeReport chainedCondReport = PG_create_chained_condition_tree(tokens, startPos + skip);
    topNode->leftNode = chainedCondReport.node;
    skip += chainedCondReport.tokensToSkip + 1; //Skip the '{'

    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + skip, InBlock);
    topNode->rightNode = runnableReport.node;
    skip += runnableReport.tokensToSkip;

    return PG_create_node_report(topNode, skip);
}

/*
Purpose: Returns a tree from the created var calls
Return Type: NodeReport => Contains the topNode as well as how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start constructing
*/
NodeReport PG_create_variable_tree(TOKEN **tokens, size_t startPos) {
    enum VAR_TYPE type = get_var_type(tokens, startPos);
    NodeReport report = {NULL, UNINITIALZED};

    if (type == NORMAL_VAR) {
        report = PG_create_normal_var_tree(tokens, startPos);
    } else if (type == ARRAY_VAR) {
        report = PG_create_array_var_tree(tokens, startPos);
    } else if (type == COND_VAR) {
        report = PG_create_conditional_var_tree(tokens, startPos);
    } else if (type == INSTANCE_VAR) {
        report = PG_create_instance_var_tree(tokens, startPos);
    }

    return report;
}

/*
Purpose: Determines the variable type
Return Type: enum varType => Type of the variable
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start checking
*/
enum VAR_TYPE get_var_type(TOKEN **tokens, size_t startPos) {
    int colonBefore = false;
    int equalsPassed = false;
    int colonSkip = 0;

    for (size_t i = startPos; i < TOKENLENGTH; i++) {
        if (colonBefore == true) {
            if (colonSkip > 0 && (*tokens)[i].type == _IDENTIFIER_) {
                colonBefore = false;
            }

            colonSkip++;
            continue;
        }

        switch ((*tokens)[i].type) {
        case _OP_COLON_:
            colonBefore = true;
            continue;
        case _OP_EQUALS_:
            equalsPassed = true;
            continue;
        case _KW_NEW_:
            return INSTANCE_VAR;
        case _OP_SEMICOLON_:
            return NORMAL_VAR;
        case _OP_RIGHT_EDGE_BRACKET_:
            if (equalsPassed == true) {
                return NORMAL_VAR;
            } else {
                return ARRAY_VAR;
            }
        case _OP_QUESTION_MARK_:
            return COND_VAR;
        default:
            break;
        }
    }

    return UNDEF;
}

/*
Purpose: Create a subtree for a class instance definition
Return Type: NodeReport => Contains the topNode and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

[INSTANCE]
    |
  [VAL]
_______________________________
*/
NodeReport PG_create_class_instance_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos + 1];
    struct Node *topNode = PG_create_node(token->value, _INHERITED_CLASS_NODE_, token->line, token->tokenStart);
    int bounds = (int)PG_predict_argument_count(tokens, startPos + 2, false);
    (void)PG_allocate_node_details(topNode, bounds, false);
    int skip = (int)PG_add_params_to_node(topNode, tokens, startPos + 3, 0, _NULL_);

    return PG_create_node_report(topNode, skip + 4);
}

/**
 * @brief Creates a subtree for a class instance variable.
 * 
 * @returns A NodeReport with the root of the created subtree and
 * how many tokens were processed.
 * 
 * @param **tokens  Pointer to the token array
 * @param startPos  Position from where to start the tree construction
 * 
 * @note The Layout of the created tree is as followed:
 * @note _______________________________
 * @note Layout:
 * @note
 * @note  [INSTANCE]
 * @note     /   \
 * @note  [MOD] [VAL]
 * @note _______________________________
*/
NodeReport PG_create_instance_var_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    int skip = 0;
    struct Node *topNode = PG_create_node(NULL, _UNDEF_, token->line, token->tokenStart);
    topNode->leftNode = PG_create_modifier_node(token, &skip);

    if ((*tokens)[startPos + skip].type == _KW_CONST_) {
        topNode->type = _CONST_CLASS_INSTANCE_NODE_;
    } else {
        topNode->type = _VAR_CLASS_INSTANCE_NODE_;
    }

    skip++;
    
    if ((*tokens)[startPos + skip].type == _OP_COLON_) {
        skip += (int)PG_add_varType_definition(tokens, startPos + skip + 1, topNode) + 1;
    }
    
    topNode->value = (*tokens)[startPos + skip].value;
    skip += 3; //Skip the name, "=" and "new"
    token = &(*tokens)[startPos + skip];

    struct Node *inheritNode = PG_create_node(token->value, _INHERITED_CLASS_NODE_, token->line, token->tokenStart);
    topNode->rightNode = inheritNode;
    skip += 2;

    int paramOffset = topNode->detailsCount;
    int bounds = 0;

    if ((*tokens)[startPos + skip].type == _IDENTIFIER_
        && (*tokens)[startPos + skip + 1].type == _OP_LEFT_BRACKET_) {
        bounds = 1;
    } else {
        (int)PG_predict_argument_count(tokens, startPos + skip, false);
    }
    
    (void)PG_allocate_node_details(inheritNode, paramOffset + bounds, true);
    skip += (int)PG_add_params_to_node(inheritNode, tokens, startPos + skip, paramOffset, _NULL_);
    return PG_create_node_report(topNode, skip + 2);
}

NodeReport PG_create_condition_assignment_tree(TOKEN **tokens, size_t startPos) {
    NodeReport conditionReport = PG_create_chained_condition_tree(tokens, startPos);
    int skip = 0;
    TOKEN *token = &(*tokens)[startPos];

    struct Node *topNode = PG_create_node("?", _CONDITIONAL_ASSIGNMENT_NODE_, token->line, token->tokenStart);
    topNode->leftNode = conditionReport.node;
    skip += conditionReport.tokensToSkip;

    (void)PG_allocate_node_details(topNode, 2, false);

    /*
    > var a = b == true ? 2 : 1;
    >                     ^
    >         (*tokens)[startPos + skip]
    */

    NodeReport trueValue = {NULL, UNINITIALZED};
    
    if ((int)predict_is_conditional_assignment_type(tokens, startPos + skip, TOKENLENGTH) == true) {
        trueValue = PG_create_condition_assignment_tree(tokens, startPos + skip);
    } else {
        int bounds = PG_get_cond_assignment_bounds(tokens, startPos + skip);
        trueValue = PG_create_simple_term_node(tokens, startPos + skip, bounds);
        trueValue.tokensToSkip++;
    }

    topNode->details[0] = trueValue.node;
    topNode->details[0]->type = _TRUE_VALUE_NODE_;
    skip += trueValue.tokensToSkip;

    NodeReport falseValue = {NULL, UNINITIALZED};
    
    if ((int)predict_is_conditional_assignment_type(tokens, startPos + skip, TOKENLENGTH) == true) {
        falseValue = PG_create_condition_assignment_tree(tokens, startPos + skip);
    } else {
        int bounds = PG_get_cond_assignment_bounds(tokens, startPos + skip);
        falseValue = PG_create_simple_term_node(tokens, startPos + skip, bounds);
        falseValue.tokensToSkip++;
    }

    topNode->details[1] = falseValue.node;
    topNode->details[1]->type = _FALSE_VALUE_NODE_;
    skip += falseValue.tokensToSkip;

    return PG_create_node_report(topNode, skip);
}

/*
Purpose: Create a subtree for a conditional variable definition
Return Type: NodeReport => Contains the topNode and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

     [COND_VAR]
   /      |     \
[MOD]    [T]    [?]
              /   |
         [COND] [VAL]

[COND_VAR] is the root, appended is the modifier
at ´´´node->leftNode´´´ and the condition at
´´´node->rightNode´´´. The [?] is the assignment holder,
to it's leftNode is the condition and to the details the
assigning value if cond is true and false.

[COND_VAR]: Conditional var
[MOD]: Modifier
[?]: Conditional assignment indicator
[COND]: Condition
[VAL]: Values for true and false
[T]: Type of variable (optional)
_______________________________
*/
NodeReport PG_create_conditional_var_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *topNode = PG_create_node(NULL, _CONDITIONAL_VAR_NODE_, token->line, token->tokenStart);
    int skip = 1;
    topNode->leftNode = PG_create_modifier_node(token, &skip);

    if ((*tokens)[startPos + skip].type == _OP_COLON_) {
        skip += PG_add_varType_definition(tokens, startPos + skip + 1, topNode);
        skip++;
    }

    struct idenValRet varName = PG_get_identifier_by_index(tokens, startPos + skip);
    topNode->value = varName.value;
    skip += 2;

    NodeReport conditionReport = PG_create_condition_assignment_tree(tokens, startPos + skip);
    topNode->rightNode = conditionReport.node;
    skip += conditionReport.tokensToSkip;

    return PG_create_node_report(topNode, skip);
}

/*
Purpose: Get the size of an conditional assignment statement
Return Type: int => Size of the statement
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start Counting
*/
int PG_get_cond_assignment_bounds(TOKEN **tokens, size_t startPos) {
    int skip = 0;

    while (startPos + skip < TOKENLENGTH) {
        if ((*tokens)[startPos + skip].type == _OP_SEMICOLON_
            || (*tokens)[startPos + skip].type == _OP_COLON_) {
            break;
        }

        skip++;
    }

    return skip;
}

/*
Purpose: Create a subtree for an array variable definition
Return Type: NodeReport => Contains the topNode and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

     [ARR_VAR]
   /     |     \
[MOD] [DIMEN] [VAL]
        [T]

To the [ARR_VAR] appended are the modifier
([MOD]: ´´´node->leftNode´´´), the value
([VAL]: ´´´´node->rightNode´´´) and the
definitions ([NAMES]: ´´´´node->details[pos]´´´).

[ARR_VAR]: Contains variable name
[MOD]: Contains the var modifier
[VAL]: Value of the variables
[DIMEN]: Dimension of the array var
[T]: Type of the variable (optional)
_______________________________
*/
NodeReport PG_create_array_var_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *topNode = PG_create_node(NULL, _ARRAY_VAR_NODE_, token->line, token->tokenStart);
    int skip = 1;
    topNode->leftNode = PG_create_modifier_node(token, &skip);

    if ((*tokens)[startPos + skip].type == _OP_COLON_) {
        skip += (int)PG_add_varType_definition(tokens, startPos + skip + 1, topNode) + 1;
    }

    struct idenValRet varName = PG_get_identifier_by_index(tokens, startPos + skip);
    topNode->value = varName.value;
    skip++;

    //DIMENSION HANDLING
    int dimCount = PG_get_dimension_count(tokens, startPos + skip);
    (void)PG_allocate_node_details(topNode, dimCount + 1, true);
    skip += (int)PG_add_dimensions_to_var_node(topNode, tokens, startPos + skip, 1);

    if ((*tokens)[startPos + skip].type == _OP_EQUALS_) {
        NodeReport arrayInit = PG_create_array_init_tree(tokens, startPos + skip, 0);
        skip += arrayInit.tokensToSkip + 1;
        topNode->rightNode = arrayInit.node;
    }

    return PG_create_node_report(topNode, skip);
}

/*
Purpose: Creates a subtree for an array initialization
Return Type: NodeReport => Topnode and How many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start constructing;
        int dim => Dimension of the init
_______________________________
Layout:

[ARRAY_INIT]
     |
[ARRAY_INIT]
[ARRAY_INIT]
_______________________________
*/
NodeReport PG_create_array_init_tree(TOKEN **tokens, size_t startPos, int dim) {
    unsigned int size = sizeof(char) * sizeof(int);
    char *name = (char*)calloc(dim + 2, size);

    if (name == NULL) {
        FREE_MEMORY();
        printf("NAME ERROR!\n");
        exit(EXIT_FAILURE);
    }

    //Automatic '\0' added
    (void)snprintf(name, size, "d:%i", dim);
    struct Node *topNode = PG_create_node(name, _ARRAY_ASSINGMENT_NODE_, (*tokens)[startPos].line, (*tokens)[startPos].tokenStart);
    int jumper = 2; //Skip "=" and first "{"
    int detailsPointer = 0;
    int argCount = (int)PG_predict_array_init_count(tokens, startPos);
    (void)PG_allocate_node_details(topNode, argCount, false);

    while (startPos + jumper < TOKENLENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];
        
        if (currentToken->type == _OP_RIGHT_BRACE_) {
            NodeReport arrInitReport = PG_create_array_init_tree(tokens, startPos + jumper, dim + 1);
            jumper += arrInitReport.tokensToSkip;
            topNode->details[detailsPointer++] = arrInitReport.node;
        } else if (currentToken->type == _OP_LEFT_BRACE_) {
            break;
        } else if (currentToken->type == _OP_COMMA_) {
            if (detailsPointer == 0) {
                TOKEN *prevToken = &(*tokens)[startPos + jumper - 1];
                enum NodeType prevType = PG_get_node_type_by_value(&prevToken->value);
                topNode->details[detailsPointer++] = PG_create_node(prevToken->value, prevType, prevToken->line, prevToken->tokenStart);
            }

            if ((*tokens)[startPos + jumper + 1].type != _OP_RIGHT_BRACE_) {
                TOKEN *nextToken = &(*tokens)[startPos + jumper + 1];
                enum NodeType nextType = PG_get_node_type_by_value(&nextToken->value);
                topNode->details[detailsPointer++] = PG_create_node(nextToken->value, nextType, nextToken->line, nextToken->tokenStart);
            }
        }

        jumper++;
    }

    if (detailsPointer == 0) {
        TOKEN *token = &(*tokens)[startPos + 1];
        enum NodeType type = PG_get_node_type_by_value(&token->value);
        topNode->details[detailsPointer] = PG_create_node(token->value, type, token->line, token->tokenStart);
    }

    return PG_create_node_report(topNode, jumper);
}

/*
Purpose: Predicts how many params an array init dimension has
Return Type: int => Number of predicted params
Params: TOKEN **tokens => Pointer to tokens;
        size_t startPos => Position from where to start counting
*/
int PG_predict_array_init_count(TOKEN **tokens, size_t startPos) {
    int openBraces = 0;
    int jumper = 0;
    int count = 1;

    while (startPos + jumper < TOKENLENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken->type == _OP_RIGHT_BRACE_) {
            openBraces++;
        } else if (currentToken->type == _OP_COMMA_) {
            if (openBraces == 1) {
                count++;
            } else if (openBraces == 0) {
                break;
            }
        } else if (currentToken->type == _OP_LEFT_BRACE_) {
            openBraces--;
        } else if (currentToken->type == _OP_SEMICOLON_) {
            break;
        }

        jumper++;
    }

    return count;
}

/*
Purpose: Set the individual dimensions into the ´´´node->details[pos]´´´ field
Return Type: int => Tokens to skip
Params: struct Node *node => Node to set the dimensions in;
        TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position from where the dimensions start
*/
int PG_add_dimensions_to_var_node(struct Node *node, TOKEN **tokens, size_t startPos, int offset) {
    size_t jumper = 0;
    size_t currentDetail = 0 + offset;

    while (startPos + jumper < TOKENLENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + jumper];

        if (currentToken->type == _OP_RIGHT_EDGE_BRACKET_) {
            int bounds = PG_get_array_element_size(tokens, startPos + jumper);
            
            if (bounds > 0) {
                NodeReport termReport = PG_create_simple_term_node(tokens, startPos + jumper + 1, bounds);
                node->details[currentDetail++] = termReport.node;
                jumper += termReport.tokensToSkip;
            } else {
                node->details[currentDetail++] = PG_create_node("_", _NUMBER_NODE_, currentToken->line, currentToken->tokenStart);
                jumper++;
            }
        } else if (currentToken->type == _OP_EQUALS_
            || currentToken->type == _OP_SEMICOLON_) {
            break;
        }

        jumper++;
    }

    return jumper;
}

/*
Purpose: Get the size of an array element
Return Type: int => Size of the array element
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position from where to start getting the size
*/
int PG_get_array_element_size(TOKEN **tokens, size_t startPos) {
    int bounds = 0;

    while ((*tokens)[startPos + bounds].type != _OP_LEFT_EDGE_BRACKET_) {
        bounds++;
    }

    return bounds - 1;
}

/*
Purpose: Get the dimension count of an array var
Return Type: int => Dimension count
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start counting
*/
int PG_get_dimension_count(TOKEN **tokens, size_t startPos) {
    int counter = 0;

    for (size_t i = startPos; i < TOKENLENGTH; i++) {
        if ((*tokens)[i].type == _OP_RIGHT_EDGE_BRACKET_) {
            counter++;
        } else if ((*tokens)[i].type == _OP_EQUALS_
            || (*tokens)[i].type == _OP_SEMICOLON_) {
            break;
        }
    }

    return counter;
}

/*
Purpose: Create a subtree for a variable definition
Return Type: NodeReport => Contains the topNode and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

     [VAR]
   /      \
[MOD]    [VAL]

To the [VAR] appended are the modifier
([MOD]: ´´´node->leftNode´´´) and the value
([VAL]: ´´´´node->rightNode´´´).

[VAR]: Contains the variable name and primitive type if assigned
[MOD]: Contains the var modifier
[VAL]: Value of the variable
_______________________________
*/
NodeReport PG_create_normal_var_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct Node *varNode = PG_create_node(NULL, _VAR_NODE_, token->line, token->tokenStart);
    int skip = 0;
    
    varNode->leftNode = PG_create_modifier_node(token, &skip);

    //Determine var type
    enum NodeType type = (*tokens)[startPos + skip].type == _KW_VAR_ ? _VAR_NODE_ : _CONST_NODE_;
    varNode->type = type;
    skip++;

    if ((*tokens)[startPos + skip].type == _OP_COLON_) {
        skip += (int)PG_add_varType_definition(tokens, startPos + skip + 1, varNode) + 1;
    }

    struct idenValRet nameRet = PG_get_identifier_by_index(tokens, startPos + skip);
    varNode->value = nameRet.value;
    skip += nameRet.movedTokens;
    
    if ((*tokens)[startPos + skip].type == _OP_EQUALS_) {
        size_t bounds = (size_t)PG_get_size_till_next_semicolon(tokens, startPos + skip + 1);
        NodeReport termReport = PG_create_simple_term_node(tokens, startPos + skip + 1, bounds);
        struct Node *valueNode = termReport.node;
        varNode->rightNode = valueNode;
        skip += termReport.tokensToSkip;
    }

    return PG_create_node_report(varNode, skip + 1);
}

/*
Purpose: Generate a subtree for a condition
Return Type: NodeReport => Contains top of the subtree and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position from where to start constructing
_______________________________
PRECEDENCE of CONDITION operators:

+-----+-----+----+---+---+
|     | AND | OR | ( | ) |
+-----+-----+----+---+---+
| AND |  =  | =  | ( | ) |
+-----+-----+----+---+---+
| OR  |  =  | =  | ( | ) |
+-----+-----+----+---+---+
|  (  |  (  | (  | = | = |
+-----+-----+----+---+---+
|  )  |  )  | )  | = | = |
+-----+-----+----+---+---+
_______________________________
Layout:

   [CHCOND]
   /     \
[COND] [COND]

The conditions can be found in ´´´node->leftNode´´´ and
´´´node->rightNode´´´.

[COND]: Condition
[CHCOND]: Chained condition holding multiple [COND]s
_______________________________
*/
NodeReport PG_create_chained_condition_tree(TOKEN **tokens, size_t startPos) {
    struct Node *cache = NULL;
    size_t lastCondStart = UNINITIALZED;
    size_t skip = startPos;

    while (skip < TOKENLENGTH) {
        TOKEN *currentToken = &(*tokens)[skip];

        if (currentToken->type == _OP_LEFT_BRACKET_
            || currentToken->type == _OP_QUESTION_MARK_
            || currentToken->type == _OP_SEMICOLON_) {
            break;
        }

        switch (currentToken->type) {
        case _OP_RIGHT_BRACKET_: {
            NodeReport report = PG_create_chained_condition_tree(tokens, skip + 1);
            skip += report.tokensToSkip;

            if (cache == NULL) {
                cache = report.node;
            } else {
                if (cache->leftNode == NULL) {
                    cache->leftNode = report.node;
                } else {
                    cache->rightNode = report.node;
                }
            }

            break;
        }
        case _KW_OR_:
        case _KW_AND_: {
            enum NodeType type = currentToken->type == _KW_AND_ ? _AND_NODE_ : _OR_NODE_;
            struct Node *node = PG_create_node(currentToken->value, type, currentToken->line, currentToken->tokenStart);

            NodeReport rightReport = {NULL, UNINITIALZED};

            if ((*tokens)[skip + 1].type == _OP_RIGHT_BRACKET_) {
                rightReport = PG_create_chained_condition_tree(tokens, skip + 2);
            } else {
                rightReport = PG_create_condition_tree(tokens, skip + 1);
            }

            skip += rightReport.tokensToSkip;
            node->rightNode = rightReport.node;

            if (cache == NULL) {
                NodeReport leftReport = {NULL, UNINITIALZED};
                leftReport = PG_create_condition_tree(tokens, lastCondStart);
                node->leftNode = leftReport.node;
            } else {
                node->leftNode = cache;
            }

            cache = node;
            lastCondStart = UNINITIALZED;
            break;
        }
        default:
            lastCondStart = lastCondStart == UNINITIALZED ? skip : lastCondStart;
            break;
        }

        skip++;
    }

    if (cache == NULL) {
        NodeReport condRep = PG_create_condition_tree(tokens, startPos);
        cache = condRep.node;
    }

    return PG_create_node_report(cache, skip - startPos + 1);
}

/*
Purpose: Creates a condition subtree
Return Type: NodeReport => Contains the subtree and how many tokens to skip
Params: TOKEN **tokens => Point to the tokens array;
        size_t startPos => Position from where to start constructing;
*/
NodeReport PG_create_condition_tree(TOKEN **tokens, size_t startPos) {
    size_t skip = 0;

    while (startPos + skip < TOKENLENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + skip];

        if ((int)PG_is_condition_operator(currentToken->type) == true) {
            struct Node *conditionNode = PG_create_node(currentToken->value, PG_get_node_type_by_value(&currentToken->value), currentToken->line, currentToken->tokenStart);
            int leftBounds = (int)PG_get_condition_iden_length(tokens, startPos);
            int rightBounds = (int)PG_get_condition_iden_length(tokens, startPos + skip + 1);

            NodeReport rightTermReport = PG_create_simple_term_node(tokens, startPos + skip + 1, rightBounds);
            NodeReport leftTermReport = PG_create_simple_term_node(tokens, startPos + skip - leftBounds, leftBounds);

            conditionNode->leftNode = leftTermReport.node;
            conditionNode->rightNode = rightTermReport.node;

            skip += rightTermReport.tokensToSkip;
            return PG_create_node_report(conditionNode, skip);
        } else if ((currentToken->type == _KW_TRUE_
            || currentToken->type == _KW_FALSE_)
            && (int)PG_is_condition_operator((*tokens)[startPos + skip + 1].type) == false) {
                struct Node *boolNode = PG_create_node(currentToken->value, _BOOL_NODE_, currentToken->line, currentToken->tokenStart);
                return PG_create_node_report(boolNode, 1);
            }

        skip++;
    }

    return PG_create_node_report(NULL, 0);
}

int PG_get_condition_iden_length(TOKEN **tokens, size_t startPos) {
    int counter = 0;

    while (startPos + counter < TOKENLENGTH) {
        if ((int)PG_is_condition_operator((*tokens)[startPos + counter].type) == true
            || (*tokens)[startPos + counter].type == _OP_LEFT_BRACKET_
            || (*tokens)[startPos + counter].type == _OP_RIGHT_BRACKET_) {
            break;
        }

        counter++;
    }
    
    return counter;
}

int PG_is_condition_operator(TOKENTYPES type) {
    switch (type) {
    case _OP_EQUALS_CONDITION_:
    case _OP_GREATER_CONDITION_:
    case _OP_SMALLER_CONDITION_:
    case _OP_GREATER_OR_EQUAL_CONDITION_:
    case _OP_SMALLER_OR_EQUAL_CONDITION_:
    case _OP_NOT_EQUALS_CONDITION_:
    case _KW_AND_:
    case _KW_OR_:
    case _OP_QUESTION_MARK_:
        return true;
    default:
        return false;
    }
}

/*
Purpose: Create a subtree for a class constructor definition
Return Type: NodeReport => Contains top of the subtree and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

[CONSTRUCTOR]
     |
 [RUNNABLE]

The [CONSTRUCTOR] node only contains a [RUNNABLE]
in the ´´´node->details[position]´´´ field.
_______________________________
*/
NodeReport PG_create_class_constructor_tree(TOKEN **tokens, size_t startPos) {
    int skip = 5;
    TOKEN *token = &(*tokens)[startPos + skip];
    struct Node *topNode = PG_create_node("CONSTRUCTOR", _CLASS_CONSTRUCTOR_NODE_, token->line, token->tokenStart);
    //Start (from ´´´this´´´ to ´´´constructor´´´) is SYNTAXANALYSIS
    //not tree generation.
    /*
    > this::constructor(param) {}
    >                  ^
    >         (*tokens)[startPos + 5]
    */
    if ((*tokens)[startPos + skip].type == _OP_LEFT_BRACKET_) {
        skip += 2;
    } else {
        int arguments = (int)PG_predict_argument_count(tokens, startPos + skip, false);
        (void)PG_allocate_node_details(topNode, arguments, false);
        skip += PG_add_params_to_node(topNode, tokens, startPos + skip, 0, _PARAM_NODE_) + 1;
    }
    
    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + skip, InBlock);
    topNode->rightNode = runnableReport.node;
    skip += runnableReport.tokensToSkip;

    return PG_create_node_report(topNode, skip);
}

/*
Purpose: Create a subtree for a class definition
Return Type: NodeReport => Contains top of the subtree and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

       [CLASS]
    /     |     \
[MOD]  [INHRTS] [RUNNABLE]
       [INTFCS]

The [CLASS] node contains the class name
and appended to the node are the [MOD] for
the modifier (´´´´node->leftNode´´´) and
[PARAMS] for the params in the
´´´node->details[position]´´´ field. In
addition to that is the [INTFCS] node.

[MOD]: Modifier of the class
[PARAMS]: Parameters of the class
[RUNNABLE]: Runnable of the class
[INTFCS]: Interfaces bound with the class
_______________________________
*/
NodeReport PG_create_class_tree(TOKEN **tokens, size_t startPos) {
    int skip = 0;
    struct Node *modNode = NULL;
    TOKEN *token = &(*tokens)[startPos];
    //public class obj => {}
    modNode = PG_create_modifier_node(token, &skip);

    skip++;
    token = &(*tokens)[startPos + skip];
    struct Node *classNode = PG_create_node(token->value, _CLASS_NODE_, token->line, token->tokenStart);
    classNode->leftNode = modNode;
    skip++;

    if ((*tokens)[startPos + skip].type == _KW_EXTENDS_) {
        (void)PG_allocate_node_details(classNode, 1, false);
        TOKEN *inTok = &(*tokens)[startPos + skip + 1];
        classNode->details[0] = PG_create_node(inTok->value, _INHERITANCE_NODE_, inTok->line, inTok->tokenStart);
        skip += 2;
    }

    if ((*tokens)[startPos + skip].type == _KW_WITH_) {
        int offset = classNode->detailsCount;
        int arguments = 0;
        
        if ((*tokens)[startPos + skip + 1].type == _IDENTIFIER_
            && (*tokens)[startPos + skip + 2].type == _OP_CLASS_CREATOR_) {
            arguments = 1;
        } else {
            arguments += (int)PG_predict_argument_count(tokens, startPos + skip, false);
        }

        (void)PG_allocate_node_details(classNode, offset + arguments, true);
        skip += PG_add_params_to_node(classNode, tokens, startPos + skip + 1, offset, _INTERFACE_NODE_);
        skip++;
    }

    skip += 2;
    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + skip, InBlock);
    classNode->rightNode = runnableReport.node;

    return PG_create_node_report(classNode, skip + runnableReport.tokensToSkip);
}

/*
Purpose: Create a subtree for a try statement
Return Type: NodeReport => Contains top of the subtree and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

 [TRY]
    |
[RUNNABLE]

In the [TRY] node contained are the runnable
statements ´´´node->details[position]´´´.
_______________________________
*/
NodeReport PG_create_try_tree(TOKEN **tokens, size_t startPos) {
    /*
    > try {}
    >     ^
    > (*tokens)[startPos + 1]
    */
    //Rest is checked by the SYNTAXANALYZER for correctness
    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + 1, InBlock);
    struct Node *tryNode = runnableReport.node;
    tryNode->type = _TRY_NODE_;
    tryNode->value = "TRY";

    return PG_create_node_report(tryNode, runnableReport.tokensToSkip + 1);
}

/*
Purpose: Create a subtree for a catch statement
Return Type: NodeReport => Contains top of the subtree and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

 [CATCH]
    |
[RUNNABLE]

In the [CATCH] node contained are the runnable
statements ´´´node->details[position]´´´.
_______________________________
*/
NodeReport PG_create_catch_tree(TOKEN **tokens, size_t startPos) {
    /*
    > catch (Exception e) {}
    >                     ^
    >           (*tokens)[startPos + 5]
    */
    //Rest is checked by the SYNTAXANALYZER for correctness
    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + 5, InBlock);
    struct Node *catchNode = runnableReport.node;
    catchNode->type = _CATCH_NODE_;
    catchNode->value = "CATCH";

    return PG_create_node_report(catchNode, runnableReport.tokensToSkip + 5);
}

/*
Purpose: Generate a subtree for an inclusion
Return Type: NodeReport => Contains topNode and tokensToSkip
Params: TOKEN **tokens => Pointer to token array;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

[EXPORT]

In the [EXPORT] node is a indicator and the exported
file can be found in ´´´node->value´´´.
_______________________________
*/
NodeReport PG_create_export_tree(TOKEN **tokens, size_t startPos) {
    //Here: export "name";
    TOKEN *token = &(*tokens)[startPos + 1];
    struct Node *topNode = PG_create_node(token->value, _EXPORT_NODE_, token->line, token->tokenStart);
    return PG_create_node_report(topNode, 3);
}

/*
Purpose: Generate a subtree for an inclusion
Return Type: NodeReport => Contains topNode and tokensToSkip
Params: TOKEN **tokens => Pointer to token array;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

[INCLUDE]

In the [INCLUDE] node is a indicator and the included
file can be found in ´´´node->value´´´.
_______________________________
*/
NodeReport PG_create_include_tree(TOKEN **tokens, size_t startPos) {
    //Here: include "../file.spc";
    TOKEN *token = &(*tokens)[startPos + 1];
    struct Node *topNode = PG_create_node(token->value, _INCLUDE_NODE_, token->line, token->tokenStart);
    return PG_create_node_report(topNode, 3);
}

/*
Purpose: Generate a subtree for an enum
Return Type: NodeReport => Contains topNode and how many tokens to skip
Params: TOKEN **tokens => Pointer to the array of tokens;
        size_t startPos => Position from where to start constructing
_______________________________
Layout:

   [ENUM]
     |
[ENUMERATOR]
           \
         [VALUE]

To the [ENUM] node appended are the [ENUMERATOR] nodes,
int the ´´´node->details[position]´´´ field. The
´´´enumerator->rightNode´´´ field contains the value of
the enumerator (If not set it assigns it automatically).

[ENUM]: Contains the enum's name
[ENUMERATOR]: A single enumerator element
[VALUE]: Value of a enumerator element
_______________________________
*/
NodeReport PG_create_enum_tree(TOKEN **tokens, size_t startPos) {
    /*
    > enum exampleEnum = {...}
    >      ^^^^^^^^^^^
    > (*tokens)[startPos + 1]
    */
    TOKEN *token = &(*tokens)[startPos + 1];
    struct Node *enumNode = PG_create_node(token->value, _ENUM_NODE_, token->line, token->tokenStart);
    int argumentCount = (int)PG_predict_enumerator_count(tokens, startPos + 2);
    (void)PG_allocate_node_details(enumNode, argumentCount, false);
    
    argumentCount = 0;
    size_t skip = 2;
    int currentEnumeratorValue = 0;

    while (startPos + skip < TOKENLENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + skip];

        if (currentToken->type == _OP_LEFT_BRACE_) {
            skip++;
            break;
        }

        if (argumentCount > enumNode->detailsCount) {
            FREE_MEMORY();
            printf("SIZE (enum) %li!\n", enumNode->detailsCount);
            exit(EXIT_FAILURE);
        }

        token = &(*tokens)[startPos + skip + 1];
        struct Node *enumeratorNode = PG_create_node(token->value, _ENUMERATOR_NODE_, token->line, token->tokenStart);

        if (currentToken->type == _OP_COMMA_
            || currentToken->type == _OP_RIGHT_BRACE_) {
            /*
            > Looking for: enumerator : [NUMBER]
            >                         ^
            */
            token = &(*tokens)[startPos + skip + 3];

            if ((*tokens)[startPos + skip + 2].type == _OP_COLON_) {
                currentEnumeratorValue = (int)atoi(token->value);
                skip++;
            }

            //Size for long
            char *value = (char*)calloc(24, sizeof(char));

            if (value == NULL) {
                FREE_MEMORY();
                printf("VALUE = NULL! (enum)\n");
                exit(EXIT_FAILURE);
            }

            (void)snprintf(value, 24, "%d", currentEnumeratorValue++);
            enumeratorNode->rightNode = PG_create_node(value, _VALUE_NODE_, token->line, token->tokenStart);
            enumNode->details[argumentCount++] = enumeratorNode;
            skip++;
        }

        skip++;
    }
    
    return PG_create_node_report(enumNode, skip);
}

/*
Purpose: Predict how many enumerators an enum has
Return Type: int => Number of predicted enumerators
Params: TOKEN **tokens => Pointer to the token array;
        size_t startPos => Position from where to start counting enumerators
*/
int PG_predict_enumerator_count(TOKEN **tokens, size_t starPos) {
    int enumCount = 1;
    int jumper = 0;

    while (starPos + jumper < TOKENLENGTH) {
        TOKEN *currentToken = &(*tokens)[starPos + jumper];

        if (currentToken->type == _OP_LEFT_BRACE_) {
            break;
        } else if (currentToken->type == _OP_COMMA_) {
            enumCount++;
        }

        jumper++;
    }

    return enumCount;
}

/*
Purpose: Generate a subtree for a function definition
Return Type: NodeReport => Contains the root node of the subtree and the number of tokens to skip
Params: TOKEN **tokens => Pointer to the array of tokens;
        size_t startPos => The position from where to start constructing
_______________________________
Layout:

     [FUNCTION]
   /      |     \
[MOD]  [PARAMS]  [RET]
      [RUNNABLE]

To the [FUNCTION] node appended are the
function name ´´´node->leftNode´´´ and
the parameters ´´´node->rightNode´´´.
In addition to that the modifier and
return type.

[FUNCTION]: Node the holds the function and name
[PARAMS]: Parameters, that get invoked
[MOD]: Modifier
[RET]: Return type
[RUNNABLE]: All processes, that happen in the function
_______________________________
*/
NodeReport PG_create_function_tree(TOKEN **tokens, size_t startPos) {
    int skip = 0;

    struct Node *modNode = NULL;
    struct Node *retTypeNode = NULL;
    TOKEN *token = &(*tokens)[startPos];

    /*
    > global function:int add(number1, number2)
    > ^^^^^^         ^^^^ ^^^ ^^^^^^^^^^^^^^^^
    > [POS1]        [POS2] |       [POS4]
    >                    [POS3]
    >
    > [POS1]: (*tokens)[startPos] (skip gets increased by 1)
    > [POS2]: (*tokens)[startPos + skip + 2] (skip gets increased by 2)
    > [POS3]: (*tokens)[startPos + skip + 1]
    > [POS4]: (*tokens)[startPos + skip + 2]
    */

    modNode = PG_create_modifier_node(token, &skip);

    if ((*tokens)[startPos + skip + 1].type == _OP_COLON_) {
        token = &(*tokens)[startPos + skip + 2];
        retTypeNode = PG_create_node(token->value, _RET_TYPE_NODE_, token->line, token->tokenStart);
        skip += 2;
    }

    //No NULL check needed, if leftNode or rightNode == NULL nothing changes
    token = &(*tokens)[startPos + skip + 1];
    struct Node *functionNode = PG_create_node(token->value, _FUNCTION_NODE_, token->line, token->tokenStart);
    functionNode->leftNode = modNode;
    functionNode->rightNode = retTypeNode;

    int argumentCount = (int)PG_predict_argument_count(tokens, startPos + skip + 2, true);
    (void)PG_allocate_node_details(functionNode, argumentCount + 1, false);
    skip += (size_t)PG_add_params_to_node(functionNode, tokens, startPos + skip + 3, 0, _NULL_) + 4;
    skip += argumentCount == 0 ? 1 : 0;

    NodeReport runnableReport = PG_create_runnable_tree(tokens, startPos + skip + 1, InBlock);
    functionNode->details[argumentCount] = runnableReport.node;

    return PG_create_node_report(functionNode, skip + runnableReport.tokensToSkip + 1);
}

/*
Purpose: Generate a subtree for a function call
Return Type: NodeReport => Contains the root node of the subtree and the number of tokens to skip
Params: TOKEN **tokens => Pointer to the array of tokens;
        size_t startPos => The index of the first token of the function call
*/
NodeReport PG_create_function_call_tree(TOKEN **tokens, size_t startPos) {
    TOKEN *token = &(*tokens)[startPos];
    struct idenValRet nameRet = PG_get_identifier_by_index(tokens, startPos);
    struct Node *functionCallNode = PG_create_node(nameRet.value, _FUNCTION_CALL_NODE_, token->line, token->tokenStart);
    int argumentSize = (int)PG_predict_argument_count(tokens, startPos + nameRet.movedTokens, true);
    (void)PG_allocate_node_details(functionCallNode, argumentSize, false);
    size_t paramSize = (size_t)PG_add_params_to_node(functionCallNode, tokens, startPos + nameRet.movedTokens + 1, 0, _NULL_);
    paramSize = paramSize > 0 ? paramSize - 1 : paramSize;
    return PG_create_node_report(functionCallNode, paramSize + 3);
}

/*
Purpose: Adds the parameters to the function call node
Return Type: size_t => The number of tokens to skip
Params: struct Node *node => The root node of the function call subtree;
        TOKEN **tokens => Pointer to the array of tokens;
        size_t startPos => The index of the first token of the function call;
        int addStart => Index at which to start writing;
        enum NodeType stdType => StandartType;
NOTICE: THE PARENTNODE HAS TO HAVE ALLOCATED SPACE OR ELSE THE PROGRAM
        WILL CRASH!!!
_______________________________
Layout:

[FUNCTION_CALL]
       |
    [PARAM]
    [PARAM]       

To the [FUNCTION_CALL] node appended are the
params. The params are appended at ´´´node->details[pos]´´´.

[FUNCTION_CALL]: Node the holds the function call name
[PARAM]: Parameter, that gets invoked
_______________________________
*/
size_t PG_add_params_to_node(struct Node *node, TOKEN **tokens, size_t startPos, int addStart, enum NodeType stdType) {
    int detailsPointer = addStart;
    int skip = 0;

    for (size_t i = startPos; i < TOKENLENGTH; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (currentToken->type != _OP_LEFT_BRACKET_
            && currentToken->type != _OP_RIGHT_BRACE_
            && currentToken->type != _OP_CLASS_CREATOR_
            && currentToken->type != _KW_WITH_) {
            //Check if the param is going to be out of the allocated space
            if (detailsPointer == node->detailsCount) {
                skip = i - startPos;
                break;
            }

            NodeReport report = {NULL, -1};

            if ((int)predict_is_conditional_assignment_type(tokens, i, TOKENLENGTH) == true) {
                report = PG_create_condition_assignment_tree(tokens, i);
            } else {
                int bounds = (int)PG_get_bound_of_single_param(tokens, i);
                report = PG_create_simple_term_node(tokens, i, bounds);
            }

            node->details[detailsPointer] = report.node;
            node->details[detailsPointer++]->type = stdType == _NULL_ ? report.node->type : stdType;
            i += report.tokensToSkip;
        } else {
            skip = i - startPos;
            break;
        }
    }

    return skip;
}

/*
Purpose: Get the boundaries of a single parameter
Return Type: int => Boundaries of the single parameter
Params: TOKEN **tokens => Pointer to the array of tokens;
        size_t startPos => The index of the first token of the single parameter
*/
int PG_get_bound_of_single_param(TOKEN **tokens, size_t startPos) {
    int bound = 0;
    int openBrackets = 0;

    for (size_t i = startPos; i < TOKENLENGTH; i++) {
        TOKEN *token = &(*tokens)[i];

        if ((token->type == _OP_COMMA_ || token->type == _OP_CLASS_CREATOR_)
            && openBrackets <= 0) {
            break;
        } else if (token->type == _OP_RIGHT_BRACKET_) {
            openBrackets++;
        } else if (token->type == _OP_LEFT_BRACKET_) {
            openBrackets--;

            if (openBrackets < 0) {
                break;
            }
        }

        bound++;
    }

    return bound;
}

/*
Purpose: Predict the number of arguments of a function call
Return Type: int => Number of arguments of the function call
Params: TOKEN **tokens => Pointer to the array of tokens;
        size_t startPos => The index of the first token of the function call;
        int withPredefinedBrackets => Is "openBrackets" awaited to be 1
*/
int PG_predict_argument_count(TOKEN **tokens, size_t startPos, int withPredefinedBrackets) {
    if ((*tokens)[startPos].type == _OP_RIGHT_BRACKET_
        && (*tokens)[startPos + 1].type == _OP_LEFT_BRACKET_) {
        return 0;
    } else if ((*tokens)[startPos].type == _OP_RIGHT_BRACKET_
        && ((*tokens)[startPos + 1].type == _IDENTIFIER_
        || (*tokens)[startPos + 1].type == _NUMBER_)
        && (*tokens)[startPos + 2].type == _OP_LEFT_BRACKET_) {
        return 1;
    }

    int count = 1;
    int openBrackets = 0;

    for (size_t i = startPos; i < TOKENLENGTH; i++) {
        TOKEN *token = &(*tokens)[i];
        
        if (token->type == _OP_COMMA_) {
            if (withPredefinedBrackets == true
                && openBrackets != 1) {
                continue;
            }

            count++;
        } else if (token->type == _OP_RIGHT_BRACKET_) {
            openBrackets++;
        } else if (token->type == _OP_LEFT_BRACKET_) {
            openBrackets--;

            if (openBrackets <= 0) {
                break;
            }
        } else if (token->type == _OP_RIGHT_BRACE_
            || token->type == _OP_CLASS_CREATOR_) {
            break;
        }
    }

    return count;
}

/*
Purpose: Get the token count to skip till the next semicolon
Return Type: size_t => Number of tokens to skip till the next semicolon
Params: TOKEN **tokens => Pointer to the array of tokens;
        size_t startPos => The index of the first token of the subtree
*/
size_t PG_get_size_till_next_semicolon(TOKEN **tokens, size_t startPos) {
    size_t size = 0;

    while ((*tokens)[startPos + size].type != _OP_SEMICOLON_) {
        size++;
    }

    return size;
}

/*
Purpose: Allocate space for the details of the node
Return Type: void
Params: struct Node *node => Pointer to the node;
        size_t size => The size of the details to allocate,
        int resize => Determines if the call is a resize or not
*/
void PG_allocate_node_details(struct Node *node, size_t size, int resize) {
    if (node == NULL) {
        FREE_MEMORY();
        printf("NODE NULL\n");
        exit(EXIT_FAILURE);
    }

    if (resize == false) {
        struct Node **temp = (struct Node**)calloc(size, sizeof(struct Node*));
        
        if (temp == NULL) {
            free(node->details);
            FREE_MEMORY();
            printf("DETAILS ERROR!\n");
            exit(EXIT_FAILURE);
        }
        
        node->details = temp;
        node->detailsCount = size;
    } else {
        struct Node **temp = (struct Node**)realloc(node->details, sizeof(struct Node) * size);

        if (temp == NULL) {
            free(node->details);
            FREE_MEMORY();
            printf("DETAILS ERROR!\n");
            exit(EXIT_FAILURE);
        }

        node->details = temp;
        node->detailsCount = size;

        for (int i = node->detailsCount; i < size; i++) {
            node->details[i] = NULL;
        }
    }
}

/*
Purpose: Print out a tree base on the nodes
Return Type: void
Params: struct Node *topNode => Pointer to the root node of the tree;
        int depth => The depth of the tree;
        int pos => The position of the node (0 = Center, 1 = Left, 2 = Right)
*/
void PG_print_from_top_node(struct Node *topNode, int depth, int pos) {
    if (topNode == NULL || topNode->value == NULL) {
        return;
    }

    for (int i = 0; i < depth; ++i) {
        printf("  ");
    }

    if (pos == 0) {
        printf("C: %s -> %i\n", topNode->value, topNode->type);
    } else if (pos == 1) {
        printf("L: %s -> %i\n", topNode->value, topNode->type);
    } else {
        printf("R: %s -> %i\n", topNode->value, topNode->type);
    }

    for (int i = 0; i < topNode->detailsCount; i++) {
        if (topNode->details[i] != NULL) {
            for (int i = 0; i < depth + 1; ++i) {
                printf("  ");
            }

            printf("(%s) detail: %s -> %i\n", topNode->value, topNode->details[i]->value, topNode->details[i]->type);
            PG_print_from_top_node(topNode->details[i]->leftNode, depth + 2, 1);
            PG_print_from_top_node(topNode->details[i]->rightNode, depth + 2, 2);

            for (int n = 0; n < topNode->details[i]->detailsCount; n++) {
                PG_print_from_top_node(topNode->details[i]->details[n], depth + 2, 0);
            }
        } else {
            printf("(%s) detail: NULL -> NULL\n", topNode->value);
        }
    }

    PG_print_from_top_node(topNode->leftNode, depth + 1, 1);
    PG_print_from_top_node(topNode->rightNode, depth + 1, 2);
}

/*
Purpose: Get the node type by the value of the node
Return Type: enum NodeType => The node type of the node
Params: char **value => Pointer to the value of the node
*/
enum NodeType PG_get_node_type_by_value(char **value) {
    if ((*value)[0] == '"') {
        return _STRING_NODE_;
    } else if ((int)is_digit((*value)[0]) == true) {
        for (size_t i = 0; (*value)[i] != '\0'; i++) {
            if ((*value)[i] == '.') {
                return _FLOAT_NODE_;
            }
        }

        return _NUMBER_NODE_;
    } else if ((*value)[0] == '*') {
        if ((*value)[1] == '\0') {
            return _MULTIPLY_NODE_;
        }
        
        return _POINTER_NODE_;
    } else if ((*value)[0] == '&') {
        return _REFERENCE_NODE_;
    } else if ((*value)[0] == '+') {
        return _PLUS_NODE_;
    } else if ((*value)[0] == '-') {
        return _MINUS_NODE_;
    } else if ((*value)[0] == '/') {
        return _DIVIDE_NODE_;
    } else if ((*value)[0] == '%') {
        return _MODULO_NODE_;
    } else if ((int)strcmp((*value), "true") == 0
        || (int)strcmp((*value), "false") == 0) {
        return _BOOL_NODE_;
    } else if ((int)strcmp((*value), "null") == 0) {
        return _NULL_NODE_;
    } else if ((int)strcmp((*value), "==") == 0) {
        return _EQUALS_CONDITION_NODE_;
    } else if ((int)strcmp((*value), "!=") == 0) {
        return _NOT_EQUALS_CONDITION_NODE_;
    } else if ((int)strcmp((*value), "<=") == 0) {
        return _SMALLER_OR_EQUAL_CONDITION_NODE_;
    } else if ((int)strcmp((*value), ">=") == 0) {
        return _GREATER_OR_EQUAL_CONDITION_NODE_;
    } else if ((int)strcmp((*value), "<") == 0) {
        return _SMALLER_CONDITION_NODE_;
    } else if ((int)strcmp((*value), ">") == 0) {
        return _GREATER_CONDITION_NODE_;
    } else {
        for (size_t i = 0; (*value)[i] != '\0'; i++) {
            if ((*value)[i] == '-'
                && (*value)[i + 1] == '>') {
                return _CLASS_ACCESS_NODE_;
            } else if ((*value)[i] == '[') {
                return _ARRAY_NODE_;
            }
        }
    }

    return _IDEN_NODE_;
}

/*
Purpose: Creates a subTree for a simple term
Return Type: NodeReport => Report containing the topNode (start operation) and how many tokens got processed
Params: TOKEN **tokens => Tokens array to be processed;
        size_t startPos => Position from where to start processing;
        size_t boundaries => How many tokens should be checked
_______________________________
PRECEDENCE of term OPERATORS:
+---+---+---+---+---+---+---+---+
|   | * | / | % | + | - | ( | ) |
+---+---+---+---+---+---+---+---+
| * | = | = | = | * | * | ( | ) |
+---+---+---+---+---+---+---+---+
| / | = | = | = | / | / | ( | ) |
+---+---+---+---+---+---+---+---+
| % | = | = | = | % | % | ( | ) |
+---+---+---+---+---+---+---+---+
| + | * | / | % | = | = | ( | ) |
+---+---+---+---+---+---+---+---+
| - | * | / | % | = | = | ( | ) |
+---+---+---+---+---+---+---+---+
| ( | ( | ( | ( | ( | ( | = | = |
+---+---+---+---+---+---+---+---+
| ) | ) | ) | ) | ) | ) | = | = |
+---+---+---+---+---+---+---+---+
_______________________________
Layout:

     [OP]
   /      \
[IDEN]  [IDEN]

After [OP] the [IDEN] is either on the ´´´node->leftNode´´´ or
´´´node->rightNode´´´.

[OP]: All operators like: '+', '-', '*', '/', '%'
[IDEN]: Can be a number, function call, a string or an identifier
_______________________________
*/

NodeReport PG_create_simple_term_node(TOKEN **tokens, size_t startPos, size_t boundaries) {
    struct Node *cache = NULL;
    struct Node *temp = NULL;
    size_t lastIdenPos = UNINITIALZED;
    int waitingToEndPlusOrMinus = false;
    int length = startPos + boundaries;

    for (size_t i = startPos; i < length; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (currentToken->type == __EOF__) {
            break;
        } else if (boundaries == 1) {
            enum NodeType nodeType = PG_get_node_type_by_value(&currentToken->value);
            cache = PG_create_node(currentToken->value, nodeType, currentToken->line, currentToken->tokenStart);
            break;
        }

        switch (currentToken->type) {
        case _OP_RIGHT_BRACKET_: {
            struct Node *target = waitingToEndPlusOrMinus == true ? temp : cache;

            if ((int)PG_is_function_call(tokens, startPos) > 0) {
                break;
            }

            int bounds = (int)PG_determine_bounds_for_capsulated_term(tokens, startPos);
            NodeReport ret = PG_create_simple_term_node(tokens, startPos + 1, bounds);

            if (target != NULL) {
                if (target->leftNode == NULL) {
                    target->leftNode = ret.node;
                } else {
                    target->rightNode = ret.node;
                }
            } else {
                cache = ret.node;
            }

            i += ret.tokensToSkip;
            continue;
        }
        case _OP_PLUS_:
        case _OP_MINUS_: {
            if (waitingToEndPlusOrMinus == true && temp != NULL) {
                cache->rightNode = temp;
                temp = NULL;
            }

            struct Node *node = PG_create_node(currentToken->value, PG_get_node_type_by_value(&currentToken->value), currentToken->line, currentToken->tokenStart);
            
            if (cache == NULL) {
                node->leftNode = PG_create_member_access_tree(tokens, lastIdenPos).node;
            } else {
                node->leftNode = cache;
            }

            int multOrDivOrModAtRight = (int)PG_is_next_operator_multiply_divide_or_modulo(tokens, i + 1);

            if (multOrDivOrModAtRight == false) {
                NodeReport assignRep = PG_assign_processed_node_to_node(tokens, i);
                node->rightNode = assignRep.node;
                i += assignRep.tokensToSkip;
            } else {
                waitingToEndPlusOrMinus = true;
            }

            lastIdenPos = UNINITIALZED;
            cache = node;
            break;
        }
        case _OP_DIVIDE_:
        case _OP_MULTIPLY_:
        case _OP_MODULU_: {
            struct Node *node = PG_create_node(currentToken->value, PG_get_node_type_by_value(&currentToken->value), currentToken->line, currentToken->tokenStart);
            NodeReport rightAssignRep = PG_assign_processed_node_to_node(tokens, i);
            node->rightNode = rightAssignRep.node;
            i += rightAssignRep.tokensToSkip;

            if (cache == NULL) {
                NodeReport leftAssignRep = PG_assign_processed_node_to_node(tokens, lastIdenPos - 1);
                node->leftNode = leftAssignRep.node;
            }

            if (waitingToEndPlusOrMinus == false) {
                if (node->leftNode == NULL) {
                    node->leftNode = cache;
                }

                cache = node;
            } else {
                if (temp == NULL) {
                    NodeReport leftAssignRep = PG_assign_processed_node_to_node(tokens, lastIdenPos - 1);
                    node->leftNode = leftAssignRep.node;
                } else {
                    node->leftNode = temp;
                }
                
                temp = node;
            }

            lastIdenPos = UNINITIALZED;
            break;
        }
        default:
            lastIdenPos = lastIdenPos == UNINITIALZED ? i : lastIdenPos;
            break;
        }

        if (i + 1 == startPos + boundaries) {
            if (waitingToEndPlusOrMinus == true
                && temp != NULL) {
                if (cache != NULL) {
                    if (cache->leftNode == NULL) {
                        cache->leftNode = temp;
                    } else {
                        cache->rightNode = temp;
                    }
                } else {
                    cache = temp;
                }

                temp = NULL;
            } else if (cache == NULL) {
                struct Node *node = NULL;
                NodeReport assignRep = PG_assign_processed_node_to_node(tokens, startPos - 1);
                node = assignRep.node;
                i += assignRep.tokensToSkip;
                cache = node;
            }
        }
    }

    return PG_create_node_report(cache, boundaries);
}

/*
Purpose: Assign the correct simple term node to the given node
Return Type: NodeReport => Contains the top node and how many tokens to skip
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start processing
*/
NodeReport PG_assign_processed_node_to_node(TOKEN **tokens, size_t startPos) {
    NodeReport report = {NULL, UNINITIALZED};

    if ((*tokens)[startPos + 1].type == _OP_RIGHT_BRACKET_) {
        size_t bounds = (size_t)PG_determine_bounds_for_capsulated_term(tokens, startPos + 1);
        report = PG_create_simple_term_node(tokens, startPos + 2, bounds);
    } else {
        report = PG_create_member_access_tree(tokens, startPos + 1);
    }
    
    if (report.node != NULL && report.tokensToSkip != UNINITIALZED) {
        return report;
    }

    return PG_create_node_report(NULL, report.tokensToSkip);
}

NodeReport PG_create_array_access_tree(TOKEN **tokens, size_t startPos) {
    struct Node *topNode = NULL;
    struct Node *prevDimNode = NULL;
    int skip = 0;
    
    while ((*tokens)[startPos + skip].type == _OP_RIGHT_EDGE_BRACKET_
        && startPos + skip < TOKENLENGTH) {
        TOKEN *tok = &(*tokens)[startPos + skip];
        struct Node *arrAccNode = PG_create_node("ARR_ACC", _ARRAY_ACCESS_NODE_, tok->line, tok->tokenStart);
        
        int termBounds = (int)PG_get_term_bounds(tokens, startPos + skip + 1);
        NodeReport termRep = PG_create_simple_term_node(tokens, startPos + skip + 1, termBounds);
        arrAccNode->leftNode = termRep.node;

        if (topNode == NULL) {
            topNode = arrAccNode;
        } else {
            (void)PG_allocate_node_details(prevDimNode, 1, false);
            prevDimNode->details[0] = arrAccNode;
        }

        prevDimNode = arrAccNode;
        skip += termRep.tokensToSkip + 2;
    }

    return PG_create_node_report(topNode, skip);
}

/*
Purpose: Get the index of the last operator in the token array
Return Type: size_t => How many tokens to go back till the operator
Params: TOKEN **tokens => Tokens to checked for the operator;
        size_t startPos => Position from where to start checking
*/
size_t PG_go_backwards_till_operator(TOKEN **tokens, size_t startPos) {
    for (size_t i = 0; startPos - i > 0; i++) {
        if ((int)PG_is_operator(&(*tokens)[startPos - i]) == true) {
            return i - 1;
        } else if (startPos - (i + 1) == 0) {
            return i + 1;
        }
    }

    return 0;
}

/*
Purpose: Determine the bounds of a capsulated term
Return Type: int => Size of the capsulated term
Params: TOKEN **tokens => Tokens to checked for the capsulated term;
        size_t startPos => Position from where to start checking
*/
int PG_determine_bounds_for_capsulated_term(TOKEN **tokens, size_t startPos) {
    size_t bounds = 0;
    int openBrackets = 0;

    while ((*tokens)[startPos + bounds].type != __EOF__) {
        if ((*tokens)[startPos + bounds].type == _OP_LEFT_BRACKET_) {
            openBrackets--;

            if (openBrackets == 0) {
                break;
            }
        } else if ((*tokens)[startPos + bounds].type == _OP_RIGHT_BRACKET_) {
            openBrackets++;
        }

        bounds++;
    }

    return bounds;
}

/*
Purpose: Check if the next Operator in a term is a Multiply, Modulo or Divide operator
Return Type: int => true = the next OP is '*', '/' or '%'; false = is not a '*', '/' or '%'
Params: TOKEN **tokens => Token array for next operator determination;
        size_t startPos => Position from where to start checking
*/
int PG_is_next_operator_multiply_divide_or_modulo(TOKEN **tokens, size_t startPos) {
    int jumper = 0;
    
    while ((*tokens)[startPos + jumper].type != __EOF__) {
        TOKEN *token = &(*tokens)[startPos + jumper];
        if (token->type == _OP_PLUS_
            || token->type == _OP_MINUS_
            || token->type == _OP_LEFT_BRACKET_
            || token->type == _OP_RIGHT_BRACKET_
            || token->type == _OP_COMMA_) {
            return false;
        } else if (token->type == _OP_MULTIPLY_
            || token->type == _OP_DIVIDE_
            || token->type == _OP_MODULU_) {
            return true;
        }

        jumper++;
    }

    return false;
}

/*
Purpose: Checks if the identifier at the position is a member access
Return Type: int => true = is member access; false => not a member access
Params: TOKEN **tokens => Pointer to the tokens;
        size_t startPos => Position from where to start checking
*/
int PG_is_next_iden_a_member_access(TOKEN **tokens, size_t startPos) {
    int openEdgeBrackets = 0;

    for (size_t i = startPos; i < TOKENLENGTH; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (currentToken->type == _OP_LEFT_EDGE_BRACKET_) {
            openEdgeBrackets--;
        } else if (currentToken->type == _OP_RIGHT_EDGE_BRACKET_) {
            openEdgeBrackets++;
        }

        if (openEdgeBrackets != 0) {
            continue;
        }

        if ((int)PG_is_operator(currentToken) == true) {
            if (currentToken->type == _OP_DOT_
                || currentToken->type == _OP_CLASS_ACCESSOR_) {
                return true;
            } else if (currentToken->type == _OP_RIGHT_BRACKET_
                || currentToken->type == _OP_LEFT_BRACKET_) {
                continue;
            } else {
                return false;
            }
        }
    }

    return false;
}

/*
Purpose: Creates a subtree for member accesses
Return Type: NodeReport => Contains the topNode and how many tokens got consumed
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position from where to start constructing
*/
NodeReport PG_create_member_access_tree(TOKEN **tokens, size_t startPos) {
    struct Node *cache = NULL;
    size_t skip = 0;

    while (startPos + skip < TOKENLENGTH) {
        TOKEN *currentToken = &(*tokens)[startPos + skip];

        if (currentToken->type == _OP_DOT_
            || currentToken->type == _OP_CLASS_ACCESSOR_) {
            enum NodeType type = currentToken->type == _OP_DOT_ ? _MEMBER_ACCESS_NODE_ : _CLASS_ACCESS_NODE_;
            struct Node *topNode = PG_create_node(currentToken->value, type, currentToken->line, currentToken->tokenStart);

            if (cache == NULL) {
                NodeReport lVal = PG_get_member_access_side_node_tree(tokens, startPos + skip, LEFT);
                topNode->leftNode = lVal.node;
            }

            NodeReport rVal = PG_get_member_access_side_node_tree(tokens, startPos + skip, RIGHT);
            topNode->rightNode = rVal.node;
            topNode->leftNode = cache == NULL ? topNode->leftNode : cache;
            skip += rVal.tokensToSkip;
            cache = topNode;
            continue;
        } else if ((int)PG_is_operator(currentToken) == true) {
            if (cache == NULL
                && (currentToken->type == _OP_LEFT_BRACKET_
                || currentToken->type == _OP_RIGHT_BRACKET_
                || currentToken->type == _OP_LEFT_EDGE_BRACKET_
                || currentToken->type == _OP_RIGHT_EDGE_BRACKET_)) {
                    skip++;
                    continue;
                }

            break;
        }

        skip++;
    }

    if (cache == NULL) {
        NodeReport rVal = PG_get_member_access_side_node_tree(tokens, startPos, STAY);
        cache = rVal.node;
    }
    
    return PG_create_node_report(cache, skip);
}

NodeReport PG_get_member_access_side_node_tree(TOKEN **tokens, size_t startPos, enum processDirection direction) {
    int offset = (int)PG_propagate_offset_by_direction(tokens, startPos, direction);
    int internalSkip = startPos + offset;
    struct Node *topNode = NULL;

    if ((int)PG_is_function_call(tokens, internalSkip) == true) {
        int nextIden = direction == LEFT ? internalSkip - (int)PG_propagate_back_till_iden(tokens, internalSkip) : internalSkip;
        NodeReport functionCallReport = PG_create_function_call_tree(tokens, nextIden);
        topNode = functionCallReport.node;
        internalSkip = direction == LEFT ? nextIden + functionCallReport.tokensToSkip : internalSkip + functionCallReport.tokensToSkip;
    } else {
        TOKEN *token = &(*tokens)[internalSkip];
        struct idenValRet valRet = PG_get_identifier_by_index(tokens, internalSkip);
        topNode = PG_create_node(valRet.value, PG_get_node_type_by_value(&valRet.value), token->line, token->tokenStart);
        internalSkip += valRet.movedTokens;
    }

    if ((*tokens)[internalSkip].type == _OP_RIGHT_EDGE_BRACKET_) {
        NodeReport arrayRep = PG_create_array_access_tree(tokens, internalSkip);
        internalSkip += arrayRep.tokensToSkip;
        topNode->leftNode = arrayRep.node;
    }

    if ((*tokens)[internalSkip].type == _OP_COLON_) {
        (void)PG_allocate_node_details(topNode, 1, false);
        internalSkip += (int)PG_add_params_to_node(topNode, tokens, internalSkip + 1, 0, _NULL_);
    }

    int actualSkip = internalSkip - startPos;
    return PG_create_node_report(topNode, actualSkip);
}

int PG_propagate_back_till_iden(TOKEN **tokens, size_t startPos) {
    int back = 0;
    int openBrackets = 0;

    for (int i = startPos; i >= 0; i--) {
        if ((*tokens)[i].type == _OP_LEFT_BRACKET_) {
            openBrackets--;
        } else if ((*tokens)[i].type == _OP_RIGHT_BRACKET_) {
            openBrackets++;
        }

        if (openBrackets != 0) {
            continue;
        }

        if ((*tokens)[i].type == _IDENTIFIER_) {
            back = startPos - i;
        }
    }

    return back;
}

int PG_propagate_offset_by_direction(TOKEN **tokens, size_t startPos, enum processDirection direction) {
    switch (direction) {
    case LEFT:
        return -1 * (PG_back_shift_array_access(tokens, startPos)) - 1;
    case RIGHT:
        return 1;
    case STAY:
    default:
        return 0;
    }
}

int PG_back_shift_array_access(TOKEN **tokens, size_t startPos) {
    int back = 0;

    for (int i = startPos; i >= 0; i--) {
        if (i <= 0) {
            return 0;
        }

        switch ((*tokens)[i].type) {
            case _OP_SEMICOLON_:
            case _OP_EQUALS_:
            case _OP_PLUS_EQUALS_:
            case _OP_MINUS_EQUALS_:
            case _OP_MULTIPLY_EQUALS_:
            case _OP_DIVIDE_EQUALS_:
                i = 0;
                break;
            case _OP_RIGHT_EDGE_BRACKET_:
                if (((*tokens)[i - 1].type == _OP_LEFT_BRACKET_
                    || (*tokens)[i - 1].type == _IDENTIFIER_)) {
                    back = startPos - i;
                    i = 0;
                    break;
                }
            default: continue;
        }
    }

    return back;
}

/*
Purpose: Get the value of an identifier list like "Math.sqrt()"
Return Type: struct idenValRet => Contains the value of the identifier and how many tokens to skip
Params: TOKEN **tokens => Token array in which the identifiers are located;
        size_t startPos => Position from where to start to get the values
*/
struct idenValRet PG_get_identifier_by_index(TOKEN **tokens, size_t startPos) {
    size_t idenStartPos = startPos;
    size_t idenEndPos = startPos;
    int nullParam = false;

    if ((*tokens)[startPos + 1].type == _OP_LEFT_BRACKET_
        && (*tokens)[startPos - 1].type == _OP_RIGHT_BRACKET_) {
        idenEndPos = startPos + 1;
        nullParam = true;
    }

    while ((*tokens)[idenEndPos].type != __EOF__ && nullParam == false) {
        TOKEN *currentToken = &(*tokens)[idenEndPos];

        if ((int)PG_is_operator(currentToken) == true) {
            break;
        }

        idenEndPos++;
    }
    
    char *cache = NULL;
    size_t cacheSize = 0;

    for (size_t i = idenStartPos; i < idenEndPos; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (cache == NULL) {
            //The currentToken->size already includes a +1 for the '\0'
            cache = (char*)calloc(currentToken->size, sizeof(char));

            if (cache == NULL) {
                free(cache);
                FREE_MEMORY();
                (void)printf("ERROR! (CACHE CALLOC)\n");
            }
            //currentToken->value already has '\0'
            (void)strncpy(cache, currentToken->value, currentToken->size);
            cacheSize = currentToken->size;
        } else {
            char *temp = (char*)realloc(cache, sizeof(char) * (cacheSize + currentToken->size - 1));

            if (temp == NULL) {
                free(cache);
                FREE_MEMORY();
                (void)printf("ERROR! (CACHE REALLOC)\n");
            }

            cache = temp;
            (void)strncat(cache, currentToken->value, currentToken->size);
            cacheSize += currentToken->size;
        }
    }

    struct idenValRet ret = {cache, idenEndPos - idenStartPos};
    return ret;
}

/*
Purpose: Predicts whether the following tokens are a function call or not
Return Type: int => true = is function call; false = not a function call
Params: TOKEN **tokens => Pointer to the tokens array;
        size_t startPos => Position from where to start checking
*/
int PG_is_function_call(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type == _OP_RIGHT_BRACKET_
        && (*tokens)[startPos + 1].type == _OP_LEFT_BRACKET_) {
        return true;
    } else if ((*tokens)[startPos].type == _OP_LEFT_BRACKET_
        && (*tokens)[startPos - 1].type == _OP_RIGHT_BRACKET_) {
        return true;
    } else if ((*tokens)[startPos].type == _IDENTIFIER_
        && (*tokens)[startPos + 1].type == _OP_RIGHT_BRACKET_) {
        return true;
    }

    int mover = 0;
    int openBrackets = 0;
    int openEdgeBrackets = 0;

    if ((*tokens)[startPos].type == _OP_LEFT_BRACKET_
        || (*tokens)[startPos].type == _OP_LEFT_EDGE_BRACKET_) {
        while (startPos - mover > 0) {
            TOKEN *tok = &(*tokens)[startPos - mover];

            if (tok->type == _OP_LEFT_EDGE_BRACKET_) {
                openEdgeBrackets--;
            } else if (tok->type == _OP_RIGHT_EDGE_BRACKET_) {
                openEdgeBrackets++;
            }
            
            if (openEdgeBrackets != 0) {
                continue;
            }
            
            if (tok->type == _OP_LEFT_BRACKET_) {
                openBrackets--;
            } else if (tok->type == _OP_RIGHT_BRACKET_) {
                openBrackets++;

                if (openBrackets == 0
                    && (*tokens)[startPos - mover - 1].type == _IDENTIFIER_) {
                    return mover;
                }
            }

            mover++;
        }
    } else if ((*tokens)[startPos].type == _OP_RIGHT_BRACKET_) {
        while ((*tokens)[startPos + mover].type != __EOF__) {
            if ((*tokens)[startPos + mover].type == _OP_LEFT_BRACKET_) {
                openBrackets--;

                if (openBrackets == 0
                    && (*tokens)[startPos - 1].type == _IDENTIFIER_) {
                    return mover;
                }
            } else if ((*tokens)[startPos + mover].type == _OP_RIGHT_BRACKET_) {
                openBrackets++;
            }

            mover++;
        }
    }

    return false;
}

/**
 * @brief Adds a variable type if defined.
 * 
 * @note The startPos starts after the ":"!!!
 * 
 * @param **tokens  Pointer to the tokens array
 * @param startPos  Position from where to start addding the type definition
 * @param *parentNode   Pointer to the node above the type def (e.g. variable)
*/
int PG_add_varType_definition(TOKEN **tokens, size_t startPos, struct Node *parentNode) {
    int skip = 1;
    int dimensions = 0;
    
    TOKEN *nameTok = &(*tokens)[startPos];
    struct Node *nameOfType = PG_create_node(nameTok->value, _VAR_TYPE_NODE_, nameTok->line, nameTok->tokenStart);
    
    while ((*tokens)[startPos + skip].type == _OP_RIGHT_EDGE_BRACKET_
        && (*tokens)[startPos + skip + 1].type == _OP_LEFT_EDGE_BRACKET_
        && startPos + skip < TOKENLENGTH) {
        skip += 2;
        dimensions++;
    }

    if (dimensions > 0) {
        char *buffer = (char*)calloc(16, sizeof(char));
        int ret = (int)snprintf(buffer, 16 * sizeof(char), "d:%i", dimensions);

        if (ret <= 16 && ret > 0) {
            nameOfType->leftNode = PG_create_node(buffer, _VAR_DIM_NODE_, nameTok->line, nameTok->tokenStart);
        } else {
            (void)PARSE_TREE_NODE_RESERVATION_EXCEPTION();
        }
    }

    (void)PG_allocate_node_details(parentNode, 1, false);
    parentNode->details[0] = nameOfType;
    return skip;
}

/**
 * @brief Create a pointer to a node, when the token is a modifier.
 * 
 * @note As an modifier counts:
 * @note - private
 * @note - secure
 * @note - global
 * 
 * @param *token    Token to check
 * @param *skip     Pointer to the skipper of the current function
*/
struct Node *PG_create_modifier_node(TOKEN *token, int *skip) {
    if (token->type == _KW_PRIVATE_
        || token->type == _KW_SECURE_
        || token->type == _KW_GLOBAL_) {
        (*skip)++;
        return PG_create_node(token->value, _MODIFIER_NODE_, token->line, token->tokenStart);
    }

    return NULL;
}

/**
 * @brief Creates a Node structure on invokation with all the provided data.
 * 
 * @returns A Node with all the provided data
 * 
 * @param *value    Value of the node
 * @param type  Type of the Node (e.g. _VAR_NODE_ or _FUNCTION_CALL_NODE_, etc.)
 * @param line  Line of the token
 * @param pos   Position of the token
*/
struct Node *PG_create_node(char *value, enum NodeType type, int line, int pos) {
    struct Node *node = (struct Node*)calloc(1, sizeof(struct Node));

    if (node == NULL) {
        (void)PARSE_TREE_NODE_RESERVATION_EXCEPTION();
    }

    node->line = line;
    node->position = pos;
    node->type = type;
    node->value = value;
    node->leftNode = NULL;
    node->rightNode = NULL;
    node->details = NULL;
    node->detailsCount = 0;
    return node;
}

/**
 * This is an array containing all "mark worthy" operators.
*/
const TOKENTYPES operators[] = {
_OP_PLUS_, _OP_MINUS_, _OP_MULTIPLY_, _OP_DIVIDE_, _OP_MODULU_,
_OP_LEFT_BRACKET_, _OP_RIGHT_BRACKET_, _OP_EQUALS_, _OP_SEMICOLON_,
_OP_COMMA_, _OP_RIGHT_BRACE_, _OP_DOT_, _OP_RIGHT_EDGE_BRACKET_,
_OP_LEFT_EDGE_BRACKET_, _OP_COLON_, _OP_PLUS_EQUALS_, _OP_MINUS_EQUALS_,
_OP_MULTIPLY_EQUALS_, _OP_DIVIDE_EQUALS_, _OP_CLASS_ACCESSOR_};

/**
 * @brief Check if a given token is an operator or not.
 * 
 * @returns `True (1)` if the token is an operator, else `False (0)`
 * 
 * @param *token    Token to check
*/
int PG_is_operator(const TOKEN *token) {
    if (token->type == __EOF__) {
        return true;
    }

    for (int i = 0; i < (sizeof(operators) / sizeof(operators[0])); i++) {
        if (token->type == operators[i]
            || (int)PG_is_condition_operator(token->type) == true) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Creates a NodeReport structure, which contains the root node of the
 * subtree as well as the token count tht got processed and can now be skipped.
 * 
 * @returns Created NodeReport with all the provided data
 * 
 * @param *topNode  Root node of the subtree
 * @param tokensToSkip  Number of tokens, that can be skipped
*/
NodeReport PG_create_node_report(struct Node *topNode, int tokensToSkip) {
    NodeReport report;
    report.node = topNode;
    report.tokensToSkip = tokensToSkip;
    return report;
}

/**
 * @brief Appends a Node to the RootNode.
 * 
 * @param *node Pointer to the node to append
 * 
 * @throws PARSE_TREE_NODE_RESERVATION_EXCEPTION    When the nodes of the RootNode is NULL
*/
void PG_append_node_to_root_node(struct Node *node) {
    RootNode.nodeCount++;
    RootNode.nodes = realloc(RootNode.nodes, sizeof(struct Node) * (RootNode.nodeCount + 1));

    if (RootNode.nodes == NULL) {
        (void)PARSE_TREE_NODE_RESERVATION_EXCEPTION();
    }

    RootNode.nodes[RootNode.nodeCount] = node;
}

/*
Purpose: Free all nodes from the RootNode
ReturnType: int => Return 1 on finish
Params: void
*/
int FREE_NODES() {
    for (size_t i = 0; i < RootNode.nodeCount; i++) {
        (void)FREE_NODE(RootNode.nodes[i]);
        free(RootNode.nodes[i]->value);
        free(RootNode.nodes[i]);
        RootNode.nodes[i] = NULL;
    }

    if (RootNode.nodes != NULL) {
        free(RootNode.nodes);
    }

    return 1;
}

/*
Purpose: Free all nodes from the parent Node
ReturnType: void
Params: struct Node *node => Node to be freed
*/
void FREE_NODE(struct Node *node) {
    if (node != NULL) {
        FREE_NODE(node->leftNode);
        FREE_NODE(node->rightNode);

        for (size_t i = 0; i < node->detailsCount; i++) {
            FREE_NODE(node->details[i]);
            free(node->details[i]);
        }

        free(node->value);
        free(node->details);
        free(node);
        node = NULL;
    }
}