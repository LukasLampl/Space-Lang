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

struct storageItem {
    struct Node *cacheNode;
    size_t itemStartPos;
    size_t itemEndPos;
};

struct identifierValueReturn {
    size_t movedTokens;
    char *value;
};

enum processDirection {
    LEFT = 0,
    RIGHT
};


void PG_append_node_to_root_node(struct Node *node);
NodeReport PG_create_simple_term_node(TOKEN **tokens, size_t startPos, size_t boundaries);
int PG_determine_bounds_for_capsulated_term(TOKEN **tokens, size_t startPos);
int PG_is_next_operator_multiply_divide_or_modulo_by_direction(TOKEN **tokens, size_t startPos, enum processDirection direction);
char *PG_get_identifier_by_index(TOKEN **tokens, size_t startPos, enum processDirection direction);
int PG_is_function_call(TOKEN **tokens, size_t startPos);
int PG_is_operator(const TOKEN *token);
struct Node *PG_create_node(char *value, TOKENTYPES type);
NodeReport PG_create_node_report(struct Node *topNode, int tokensToSkip);
void PG_print_from_top_node(struct Node *topNode, int depth, int pos);
int FREE_NODES();
void FREE_NODE(struct Node *node);

//TREE ROOT
struct RootNode RootNode;
size_t tokenLenght = 0;

int Generate_Parsetree(TOKEN **tokens, size_t TokenLength) {
    tokenLenght = TokenLength;

    (void)printf("\n\n\n>>>>>>>>>>>>>>>>>>>>    PARSETREE    <<<<<<<<<<<<<<<<<<<<\n\n");

    if (tokens == NULL || TokenLength == 0) {
        (void)PARSER_TOKEN_TRANSMISSION_EXCEPTION();
    }

    RootNode.nodeCount = 0;
    printf("TOKENLENGTH: %i\n", TokenLength);

    for (size_t i = 0; i < TokenLength; i++) {
        NodeReport report = PG_create_simple_term_node(tokens, i, TokenLength);

        printf("\nPRINT:\n");
        PG_print_from_top_node(report.node, 0, 0);
        i += report.tokensToSkip;
    }

    (void)printf("\n\n\n>>>>>    Tokens converted to tree    <<<<<\n\n");

    return 1;
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
        }

        switch (currentToken->type) {
        case _OP_RIGHT_BRACKET_: {
            int isFunctionCall = (int)PG_is_function_call(tokens, i);

            if (isFunctionCall > 0) {
                i += isFunctionCall;
                continue;
            }

            size_t bounds = PG_determine_bounds_for_capsulated_term(tokens, i);
            NodeReport report = PG_create_simple_term_node(tokens, i + 1, bounds - 1);
            i += bounds;

            struct Node *target = waitingToEndPlusOrMinus == true ? temp : cache;

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
                char *lval = (char*)PG_get_identifier_by_index(tokens, i - 1, LEFT);
                struct Node *leftNode = PG_create_node(lval, _IDENTIFIER_);

                if (multOrDivOrModAtRight == false) {
                    char *rval = (char*)PG_get_identifier_by_index(tokens, i + 1, RIGHT);
                    struct Node *rightNode = PG_create_node(rval, _IDENTIFIER_);
                    node->rightNode = rightNode;
                } else {
                    waitingToEndPlusOrMinus = true;
                }

                node->leftNode = leftNode;
            } else {
                int multOrDivOrModAtRight = (int)PG_is_next_operator_multiply_divide_or_modulo_by_direction(tokens, i + 1, RIGHT);

                if (multOrDivOrModAtRight == false) {
                    char *rval = (char*)PG_get_identifier_by_index(tokens, i + 1, RIGHT);
                    struct Node *rightNode = PG_create_node(rval, _IDENTIFIER_);
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
                char *lval = (char*)PG_get_identifier_by_index(tokens, i - 1, LEFT);
                char *rval = (char*)PG_get_identifier_by_index(tokens, i + 1, RIGHT);
                struct Node *leftNode = PG_create_node(lval, _IDENTIFIER_);
                struct Node *rightNode = PG_create_node(rval, _IDENTIFIER_);

                node->leftNode = leftNode;
                node->rightNode = rightNode;
            } else {
                char *rval = (char*)PG_get_identifier_by_index(tokens, i + 1, RIGHT);
                struct Node *rightNode = PG_create_node(rval, _IDENTIFIER_);
                node->rightNode = rightNode;

                if (waitingToEndPlusOrMinus == false) {
                    node->leftNode = cache;
                } else {
                    if (temp == NULL) {
                        char *lval = (char*)PG_get_identifier_by_index(tokens, i - 1, LEFT);
                        struct Node *leftNode = PG_create_node(lval, _IDENTIFIER_);
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
                char *value = PG_get_identifier_by_index(tokens, startPos, RIGHT);
                struct Node *node = PG_create_node(value, _IDENTIFIER_);
                cache = node;
            }
        }
    }

    return PG_create_node_report(cache, boundaries);
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
                || token->type == _OP_RIGHT_BRACKET_) {
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
Return Type: char * => Pointer to summed up string
Params: TOKEN **tokens => Token array in which the identifiers are located;
        size_t startPos => Position from where to start to get the values;
        enum processDirection => In which direction should be processed
*/
char *PG_get_identifier_by_index(TOKEN **tokens, size_t startPos, enum processDirection direction) {
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
            cache = (char*)malloc(sizeof(char) * currentToken->size);

            if (cache == NULL) {
                (void)printf("ERROR!\n");
            }
            //currentToken->value already has '\0'
            (void)strncpy(cache, currentToken->value, currentToken->size);
            cacheSize = currentToken->size;
        } else {
            cache = (char*)realloc(cache, sizeof(char) * (cacheSize + currentToken->size - 1));

            if (cache == NULL) {
                (void)printf("ERROR!\n");
            }

            (void)strncat(cache, currentToken->value, currentToken->size);
            cacheSize += currentToken->size;
        }
    }

    return cache;
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
        TOKENTYPES type => Type of the Node
*/
struct Node *PG_create_node(char *value, TOKENTYPES type) {
    struct Node *node = (struct Node*)malloc(sizeof(struct Node));

    if (node == NULL) {
        (void)PARSE_TREE_NODE_RESERVATION_EXCEPTION();
    }

    node->type = type;
    node->value = value;
    node->leftNode = NULL;
    node->rightNode = NULL;
    return node;
}

/*
Purpose: Check if a given TOKEN is an operator or not
Return Type: int => true = is operator; false = not an operator
Params: const TOKEN *token => Token to be checked
*/
const TOKENTYPES operators[] = {
_OP_PLUS_, _OP_MINUS_, _OP_MULTIPLY_, _OP_DIVIDE_, _OP_MODULU_,
_OP_LEFT_BRACKET_, _OP_RIGHT_BRACKET_};

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
        free(node);
    }
}