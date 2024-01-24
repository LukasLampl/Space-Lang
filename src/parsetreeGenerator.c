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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../headers/modules.h"
#include "../headers/errors.h"
#include "../headers/parsetree.h"
#include "../headers/Token.h"

#define true 1
#define false 0

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////     PARSE TREE GENERATOR     ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

const int UNINITIALZED = -1;

typedef struct NodeReport {
    struct Node *node;
    size_t tokensToSkip;
} NodeReport;

struct idenValRet {
    char *value;
    size_t movedTokens;
};

enum processDirection {
    LEFT = 0,
    RIGHT
};


void PG_append_node_to_root_node(struct Node *node);
NodeReport PG_create_variable_tree(TOKEN **tokens, size_t startPos);
size_t PG_get_size_till_next_semicolon(TOKEN **tokens, size_t startPos);
NodeReport PG_create_function_call_tree(TOKEN **tokens, size_t startPos);
int PG_get_bound_of_single_param(TOKEN **tokens, size_t startPos);
NodeReport PG_create_simple_term_node(TOKEN **tokens, size_t startPos, size_t boundaries);
size_t go_backwards_till_operator(TOKEN **tokens, size_t startPos);
int PG_determine_bounds_for_capsulated_term(TOKEN **tokens, size_t startPos);
int PG_is_next_operator_multiply_divide_or_modulo_by_direction(TOKEN **tokens, size_t startPos, enum processDirection direction);
struct idenValRet PG_get_identifier_by_index(TOKEN **tokens, size_t startPos, enum processDirection direction);
int PG_is_function_call(TOKEN **tokens, size_t startPos);
int predict_argument_count(TOKEN **tokens, size_t startPos);
int PG_is_operator(const TOKEN *token);
struct Node *PG_create_node(char *value, enum NodeType type);
NodeReport PG_create_node_report(struct Node *topNode, int tokensToSkip);
void PG_print_from_top_node(struct Node *topNode, int depth, int pos);
int FREE_NODES();
void FREE_NODE(struct Node *node);

//TREE ROOT
struct RootNode RootNode;
size_t TOKENLENGTH = 0;

int Generate_Parsetree(TOKEN **tokens, size_t TokenLength) {
    TOKENLENGTH = TokenLength;

    (void)printf("\n\n\n>>>>>>>>>>>>>>>>>>>>    PARSETREE    <<<<<<<<<<<<<<<<<<<<\n\n");

    if (tokens == NULL || TokenLength == 0) {
        (void)PARSER_TOKEN_TRANSMISSION_EXCEPTION();
    }

    RootNode.nodeCount = 0;
    printf("TOKENLENGTH: %i\n", TokenLength);

    for (size_t i = 0; i < TokenLength; i++) {
        /*TOKEN *currentToken = &(*tokens)[i];
        NodeReport report = {NULL, UNINITIALZED};

        switch (currentToken->type) {
        case _KW_VAR_:
            report = PG_create_variable_tree(tokens, i);
            break;
        default:
            break;
        }

        if (report.node != NULL) {
            printf("\nPRINT:\n");
            PG_print_from_top_node(report.node, 0, 0);
            i += report.tokensToSkip;
        }*/

        NodeReport termRep = PG_create_function_call_tree(tokens, i);
        PG_print_from_top_node(termRep.node, 0, 0);
        i += termRep.tokensToSkip;
    }

    (void)printf("\n\n\n>>>>>    Tokens converted to tree    <<<<<\n\n");

    return 1;
}

NodeReport PG_create_variable_tree(TOKEN **tokens, size_t startPos) {
    struct Node *varNode = PG_create_node("var", _VAR_NODE_);

    struct idenValRet nameRet = PG_get_identifier_by_index(tokens, startPos + 1, RIGHT);
    char *varName = nameRet.value;
    struct Node *nameNode = PG_create_node(varName, _IDEN_NODE_);
    size_t checkPosition = startPos + nameRet.movedTokens + 1;

    if ((*tokens)[checkPosition].type == _OP_EQUALS_) {
        size_t bounds = (size_t)PG_get_size_till_next_semicolon(tokens, checkPosition + 1);
        NodeReport termReport = PG_create_simple_term_node(tokens, checkPosition + 1, bounds);
        struct Node *valueNode = termReport.node;
        varNode->rightNode = valueNode;
    }

    varNode->leftNode = nameNode;
    return PG_create_node_report(varNode, 1);
}

NodeReport PG_create_function_call_tree(TOKEN **tokens, size_t startPos) {
    struct idenValRet nameRet = PG_get_identifier_by_index(tokens, startPos, RIGHT);
    struct Node *functionCallNode = PG_create_node(nameRet.value, _FUNCTION_CALL_NODE_);

    int argumentSize = (int)predict_argument_count(tokens, startPos);
    functionCallNode->details = (struct Node**)malloc(sizeof(struct Node*) * argumentSize);
    functionCallNode->detailsCount = argumentSize;

    if (functionCallNode->details == NULL) {
        printf("DETAILS ERROR!\n");
        return PG_create_node_report(NULL, 0);
    }
    
    for (int i = 0; i < functionCallNode->detailsCount; i++) {
        functionCallNode->details[i] = NULL;
    }

    int detailsPointer = 0;
    int skip = 0;

    for (size_t i = startPos + nameRet.movedTokens; i < TOKENLENGTH; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (currentToken->type == _OP_COMMA_
            || currentToken->type == _OP_RIGHT_BRACKET_) {
            if (detailsPointer == argumentSize) {
                printf("SIZE!\n");
                skip = i - startPos;
                break;
            }

            int bounds = (int)PG_get_bound_of_single_param(tokens, i + 1);
            NodeReport termReport = PG_create_simple_term_node(tokens, i + 1, bounds);
            functionCallNode->details[detailsPointer++] = termReport.node;
            i += termReport.tokensToSkip;
        } else if (currentToken->type == _OP_LEFT_BRACKET_) {
            skip = i - startPos;
            break;
        }
    }

    printf("FUNC CALL:\n");
    PG_print_from_top_node(functionCallNode, 0, 0);

    return PG_create_node_report(functionCallNode, skip);
}

int PG_get_bound_of_single_param(TOKEN **tokens, size_t startPos) {
    int bound = 0;
    int openBrackets = 0;

    for (size_t i = startPos; i < TOKENLENGTH; i++) {
        TOKEN *token = &(*tokens)[i];

        if (token->type == _OP_COMMA_
            && openBrackets == 0) {
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

int predict_argument_count(TOKEN **tokens, size_t startPos) {
    int count = 1;
    int openBrackets = 0;

    for (size_t i = startPos; i < TOKENLENGTH; i++) {
        TOKEN *token = &(*tokens)[i];
        
        if (token->type == _OP_COMMA_
            && openBrackets == 1) {
            count++;
        } else if (token->type == _OP_RIGHT_BRACKET_) {
            openBrackets++;
        } else if (token->type == _OP_LEFT_BRACKET_) {
            openBrackets--;

            if (openBrackets <= 0) {
                break;
            }
        }
    }

    return count;
}

size_t PG_get_size_till_next_semicolon(TOKEN **tokens, size_t startPos) {
    size_t size = 0;

    while ((*tokens)[startPos + size].type != _OP_SEMICOLON_) {
        size++;
    }

    return size;
}

void PG_print_from_top_node(struct Node *topNode, int depth, int pos) {
    if (topNode == NULL || topNode->value == NULL) {
        return;
    }

    for (int i = 0; i < depth; ++i) {
        printf("  ");
    }

    if (pos == 0) {
        printf("C: %s\n", topNode->value);
    } else if (pos == 1) {
        printf("L: %s\n", topNode->value);
    } else {
        printf("R: %s\n", topNode->value);
    }

    for (int i = 0; i < topNode->detailsCount; i++) {
        if (topNode->details[i] != NULL) {
            printf("(%s) detail: %s\n", topNode->value, topNode->details[i]->value);
            PG_print_from_top_node(topNode->details[i]->leftNode, depth + 2, 1);
            PG_print_from_top_node(topNode->details[i]->rightNode, depth + 2, 2);

            for (int n = 0; n < topNode->details[i]->detailsCount; n++) {
                PG_print_from_top_node(topNode->details[i]->details[n], depth + 2, 0);
            }
        } else {
            printf("(%s) detail: NULL\n", topNode->value);
        }
    }

    PG_print_from_top_node(topNode->leftNode, depth + 1, 1);
    PG_print_from_top_node(topNode->rightNode, depth + 1, 2);
}

/*
struct Node *create_variable_node(TOKEN **tokens, size_t startPos) {
    struct Node varNode;
}*/

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
*/
NodeReport PG_create_simple_term_node(TOKEN **tokens, size_t startPos, size_t boundaries) {
    struct Node *cache = NULL;
    struct Node *temp = NULL;
    int waitingToEndPlusOrMinus = false;

    for (size_t i = startPos; i < startPos + boundaries; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (currentToken->type == __EOF__) {
            break;
        } else if (boundaries == 1) {
            cache = PG_create_node(currentToken->value, _IDENTIFIER_);
            break;
        }

        switch (currentToken->type) {
        case _OP_RIGHT_BRACKET_: {
            int isFunctionCall = (int)PG_is_function_call(tokens, i);
            struct Node *target = waitingToEndPlusOrMinus == true ? temp : cache;

            if (isFunctionCall > 0) {
                size_t tokensBack = (size_t)go_backwards_till_operator(tokens, i - 1);
                NodeReport functionCallReport = PG_create_function_call_tree(tokens, i - tokensBack - 1);
                i += functionCallReport.tokensToSkip - 1;

                if (target != NULL) {
                    if (target->leftNode == NULL) {
                        target->leftNode = functionCallReport.node;
                    } else {
                        target->rightNode = functionCallReport.node;
                    }
                } else {
                    cache = functionCallReport.node;
                }

                continue;
            }

            size_t bounds = (size_t)PG_determine_bounds_for_capsulated_term(tokens, i);
            NodeReport report = PG_create_simple_term_node(tokens, i + 1, bounds - 1);
            i += bounds;

            if (target != NULL) {
                if (target->leftNode == NULL) {
                    target->leftNode = report.node;
                } else {
                    target->rightNode = report.node;
                }
            } else {
                cache = report.node;
            }
            
            break;
        }
        case _OP_PLUS_:
        case _OP_MINUS_: {
            if (waitingToEndPlusOrMinus == true
                && temp != NULL) {
                cache->rightNode = temp;
                temp = NULL;
            }

            struct Node *node = PG_create_node(currentToken->value, currentToken->type);

            if (cache == NULL) {
                int multOrDivOrModAtRight = (int)PG_is_next_operator_multiply_divide_or_modulo_by_direction(tokens, i + 1, RIGHT);
                struct idenValRet lvalRet = PG_get_identifier_by_index(tokens, i - 1, LEFT);
                
                char *lval = lvalRet.value;
                struct Node *leftNode = PG_create_node(lval, _IDEN_NODE_);

                if (multOrDivOrModAtRight == false) {
                    struct idenValRet rvalRet = PG_get_identifier_by_index(tokens, i + 1, RIGHT);
                    char *rval = rvalRet.value;
                    struct Node *rightNode = PG_create_node(rval, _IDEN_NODE_);
                    node->rightNode = rightNode;
                } else {
                    waitingToEndPlusOrMinus = true;
                }

                node->leftNode = leftNode;
            } else {
                int multOrDivOrModAtRight = (int)PG_is_next_operator_multiply_divide_or_modulo_by_direction(tokens, i + 1, RIGHT);

                if (multOrDivOrModAtRight == false) {
                    struct idenValRet rvalRet = PG_get_identifier_by_index(tokens, i + 1, RIGHT);
                    char *rval = rvalRet.value;
                    struct Node *rightNode = PG_create_node(rval, _IDEN_NODE_);
                    node->rightNode = rightNode;
                    node->leftNode = cache;
                } else {
                    node->leftNode = cache;
                    waitingToEndPlusOrMinus = true;
                }
            }

            cache = node;
            break;
        }
        case _OP_DIVIDE_:
        case _OP_MULTIPLY_:
        case _OP_MODULU_: {
            struct Node *node = PG_create_node(currentToken->value, currentToken->type);

            if (cache == NULL) {
                struct idenValRet lvalRet = PG_get_identifier_by_index(tokens, i - 1, LEFT);
                struct idenValRet rvalRet = PG_get_identifier_by_index(tokens, i + 1, RIGHT);
                char *lval = lvalRet.value;
                char *rval = rvalRet.value;
                struct Node *leftNode = PG_create_node(lval, _IDEN_NODE_);
                struct Node *rightNode = PG_create_node(rval, _IDEN_NODE_);

                node->leftNode = leftNode;
                node->rightNode = rightNode;
            } else {
                struct idenValRet rvalRet = PG_get_identifier_by_index(tokens, i + 1, RIGHT);
                char *rval = rvalRet.value;
                struct Node *rightNode = PG_create_node(rval, _IDEN_NODE_);
                node->rightNode = rightNode;

                if (waitingToEndPlusOrMinus == false) {
                    node->leftNode = cache;
                } else {
                    if (temp == NULL) {
                        struct idenValRet lvalRet = PG_get_identifier_by_index(tokens, i - 1, LEFT);
                        char *lval = lvalRet.value;
                        struct Node *leftNode = PG_create_node(lval, _IDEN_NODE_);
                        node->leftNode = leftNode;
                    } else {
                        node->leftNode = temp;
                    }
                }
            }

            if (waitingToEndPlusOrMinus == false) {
                cache = node;
            } else {
                temp = node;
            }

            break;
        }
        default:
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
                struct idenValRet valRet = PG_get_identifier_by_index(tokens, startPos, RIGHT);
                char *value = valRet.value;
                struct Node *node = PG_create_node(value, _IDEN_NODE_);
                cache = node;
            }
        }
    }

    return PG_create_node_report(cache, boundaries);
}

size_t go_backwards_till_operator(TOKEN **tokens, size_t startPos) {
    for (size_t i = 0; startPos - i > 0; i++) {
        if (PG_is_operator(&(*tokens)[startPos - i]) == true) {
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
        size_t startPos => Position from where to start checking;
        enum processDirection => Direction to which should be checked
*/
int PG_is_next_operator_multiply_divide_or_modulo_by_direction(TOKEN **tokens, size_t startPos, enum processDirection direction) {
    int jumper = 0;
    
    if (direction == RIGHT) {
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
    } else {
        while (startPos - jumper > 0) {
            TOKEN *token = &(*tokens)[startPos - jumper];

            if (token->type == _OP_PLUS_
                || token->type == _OP_MINUS_
                || token->type == _OP_LEFT_BRACKET_
                || token->type == _OP_RIGHT_BRACKET_) {
                return false;
            } else if (token->type == _OP_MULTIPLY_
                || token->type == _OP_DIVIDE_
                || token->type == _OP_MODULU_) {
                return true;
            }

            jumper++;
        }
    }

    return false;
}

/*
Purpose: Get the value of an identifier list like "Math.sqrt()"
Return Type: struct idenValRet => Contains the value of the identifier and how many tokens to skip
Params: TOKEN **tokens => Token array in which the identifiers are located;
        size_t startPos => Position from where to start to get the values;
        enum processDirection => In which direction should be processed;
*/
struct idenValRet PG_get_identifier_by_index(TOKEN **tokens, size_t startPos, enum processDirection direction) {
    size_t idenStartPos = startPos;
    size_t idenEndPos = startPos;

    if ((*tokens)[startPos + 1].type == _OP_LEFT_BRACKET_
        && (*tokens)[startPos - 1].type == _OP_RIGHT_BRACKET_) {
        idenStartPos = startPos;
        idenEndPos = startPos + 1;
    } else if (direction == RIGHT) {
        while ((*tokens)[idenEndPos].type != __EOF__) {
            TOKEN *currentToken = &(*tokens)[idenEndPos];

            if ((int)PG_is_operator(currentToken) == true) {
                int isFunctionCall = (int)PG_is_function_call(tokens, idenEndPos);

                if (isFunctionCall == false) {
                    break;
                } else {
                    break;
                }

                idenEndPos += isFunctionCall + 1;
            }

            idenEndPos++;
        }
    } else {
        idenEndPos = startPos + 1;

        while (idenStartPos > 0) {
            TOKEN *currentToken = &(*tokens)[idenStartPos];

            if ((int)PG_is_operator(currentToken) == true) {
                int isFunctionCall = (int)PG_is_function_call(tokens, idenStartPos);
                
                if (isFunctionCall == false) {
                    idenStartPos++;
                    break;
                } else {
                    break;
                }

                idenStartPos -= isFunctionCall + 1;
            }

            idenStartPos--;
        }
    }
    
    char *cache = NULL;
    size_t cacheSize = 0;

    for (size_t i = idenStartPos; i < idenEndPos; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (cache == NULL) {
            //The currentToken->size already includes a +1 for the '\0'
            cache = (char*)malloc(sizeof(char) * currentToken->size);

            if (cache == NULL) {
                (void)printf("ERROR! (CACHE MALLOC)\n");
            }
            //currentToken->value already has '\0'
            (void)strncpy(cache, currentToken->value, currentToken->size);
            cacheSize = currentToken->size;
        } else {
            cache = (char*)realloc(cache, sizeof(char) * (cacheSize + currentToken->size - 1));

            if (cache == NULL) {
                (void)printf("ERROR! (CACHE REALLOC)\n");
            }

            (void)strncat(cache, currentToken->value, currentToken->size);
            cacheSize += currentToken->size;
        }
    }

    struct idenValRet ret = {cache, idenEndPos - idenStartPos};
    return ret;
}

int PG_is_function_call(TOKEN **tokens, size_t startPos) {
    if ((*tokens)[startPos].type == _OP_RIGHT_BRACKET_
        && (*tokens)[startPos + 1].type == _OP_LEFT_BRACKET_) {
        return true;
    } else if ((*tokens)[startPos].type == _OP_LEFT_BRACKET_
        && (*tokens)[startPos - 1].type == _OP_RIGHT_BRACKET_) {
        return true;
    }

    int mover = 0;
    int openBrackets = 0;

    if ((*tokens)[startPos].type == _OP_LEFT_BRACKET_) {
        while (startPos - mover > 0) {
            if ((*tokens)[startPos - mover].type == _OP_LEFT_BRACKET_) {
                openBrackets--;
            } else if ((*tokens)[startPos - mover].type == _OP_RIGHT_BRACKET_) {
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

/*
Purpose: Create a node based on the params
Return Type: struct Node * => Pointer to the Node
Params: char *value => Value of the node;
        enum NodeType type => Type of the Node
*/
struct Node *PG_create_node(char *value, enum NodeType type) {
    struct Node *node = (struct Node*)malloc(sizeof(struct Node));

    if (node == NULL) {
        (void)PARSE_TREE_NODE_RESERVATION_EXCEPTION();
    }

    node->type = type;
    node->value = value;
    node->leftNode = NULL;
    node->rightNode = NULL;
    node->details = NULL;
    node->detailsCount = 0;
    return node;
}

/*
Purpose: Check if a given TOKEN is an operator or not
Return Type: int => true = is operator; false = not an operator
Params: const TOKEN *token => Token to be checked
*/
const TOKENTYPES operators[] = {
_OP_PLUS_, _OP_MINUS_, _OP_MULTIPLY_, _OP_DIVIDE_, _OP_MODULU_,
_OP_LEFT_BRACKET_, _OP_RIGHT_BRACKET_, _OP_EQUALS_, _OP_SEMICOLON_,
_OP_COMMA_};

int PG_is_operator(const TOKEN *token) {
    if (token->type == __EOF__) {
        return true;
    }

    for (int i = 0; i < (sizeof(operators) / sizeof(operators[0])); i++) {
        if (token->type == operators[i]) {
            return true;
        }
    }

    return false;
}

/*
Purpose: Creates a NodeReport containing the topNode and how many tokens got processed
Return Type: NodeReport => Created NodeReport
Params: struct Node *topNode => TopNode;
        int tokensToSkip => How many tokens should be skipped
*/
NodeReport PG_create_node_report(struct Node *topNode, int tokensToSkip) {
    NodeReport report;
    report.node = topNode;
    report.tokensToSkip = tokensToSkip;
    return report;
}

/*
Purpose: Appends a Node to the RootNode
Return Type: void
Params: struct Node *node => Node to be appended
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
        FREE_NODE(RootNode.nodes[i]);
        free(RootNode.nodes[i]->value);
        free(RootNode.nodes[i]);
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
        free(node->value);

        for (size_t i = 0; i < node->detailsCount; i++) {
            FREE_NODE(node->details[i]);
            free(node->details[i]);
        }

        free(node->details);

        free(node);
    }
}