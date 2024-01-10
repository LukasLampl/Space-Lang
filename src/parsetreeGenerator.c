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

void append_node_to_root_node(struct Node *node);
NodeReport create_simple_term_node(TOKEN **tokens, size_t startPos, size_t boundaries);
struct Node *create_node(char *value, TOKENTYPES type);
NodeReport create_node_report(struct Node *topNode, int tokensToSkip);
void print_from_top_node(struct Node *topNode, int depth, int pos);
int FREE_NODES();
void FREE_NODE(struct Node *node);

struct RootNode RootNode;

/*PRECEDENCE TABLE (I = <IDENTIFIER>)
+-----+-----+-----+-----+-----+-----+
|     |  +  |  -  |  *  |  /  |  I  |
+-----+-----+-----+-----+-----+-----+
|  +  |  =  |  >  |  <  |  <  |  <  |
+-----+-----+-----+-----+-----+-----+
|  -  |  >  |  = |  <  |  <  |  <  |
+-----+-----+-----+-----+-----+-----+
|  *  |  >  |  >  |  =  |  >  |  <  |
+-----+-----+-----+-----+-----+-----+
|  /  |  >  |  >  |  >  |  =  |  <  |
+-----+-----+-----+-----+-----+-----+
|  I  |  >  |  >  |  >  |  >  |  x  |
+-----+-----+-----+-----+-----+-----+
*/

int Generate_Parsetree(TOKEN **tokens, size_t TokenLength) {
    (void)printf("\n\n\n>>>>>>>>>>>>>>>>>>>>    PARSETREE    <<<<<<<<<<<<<<<<<<<<\n\n");

    if (tokens == NULL || TokenLength == 0) {
        (void)PARSER_TOKEN_TRANSMISSION_EXCEPTION();
    }

    RootNode.nodeCount = 0;
    printf("TOKENLENGTH: %i\n", TokenLength);
    for (size_t i = 0; i < TokenLength; i++) {
        NodeReport report = create_simple_term_node(tokens, i, TokenLength);

        printf("\n");
        print_from_top_node(report.node, 0, 0);
        i += report.tokensToSkip;
    }

    (void)printf("\n\n\n>>>>>    Tokens converted to tree    <<<<<\n\n");

    return 1;
}

void print_from_top_node(struct Node *topNode, int depth, int pos) {
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

    print_from_top_node(topNode->leftNode, depth + 1, 1);
    print_from_top_node(topNode->rightNode, depth + 1, 2);
}

/*
struct Node *create_variable_node(TOKEN **tokens, size_t startPos) {
    struct Node varNode;
}*/

NodeReport create_simple_term_node(TOKEN **tokens, size_t startPos, size_t boundaries) {
    struct Node *topNode = (struct Node*)malloc(sizeof(struct Node));
    topNode->value = "simple term";
    topNode->type = _UNDEF_;

    struct storageItem {
        struct Node *cacheNode;
        size_t itemStartPos;
        size_t itemEndPos;
    };

    struct storageItem *storage = (struct storageItem*)calloc(boundaries, sizeof(struct storageItem));
    int storagePointer = 0;
    struct Node *cache = NULL;
    size_t startPosOfMultiplyOrDivide = UNINITIALZED;
    size_t endPosOfMultiplyOrDivide = UNINITIALZED;
    int wasPlusOrMinus = true;

    for (size_t i = startPos; i < startPos + boundaries; i++) {
        TOKEN *currentToken = &(*tokens)[i];
        
        if (currentToken->type == _OP_MULTIPLY_
            || currentToken->type == _OP_DIVIDE_) {
            struct Node *cacheNode = create_node(currentToken->value, currentToken->type);

            if (wasPlusOrMinus == true) {
                startPosOfMultiplyOrDivide = i - 1;
                endPosOfMultiplyOrDivide = i + 1;
                cacheNode->leftNode = create_node((*tokens)[i - 1].value, (*tokens)[i - 1].type);
                cacheNode->rightNode = create_node((*tokens)[i + 1].value, (*tokens)[i + 1].type);
            } else {
                endPosOfMultiplyOrDivide = i + 1;
                cacheNode->leftNode = create_node((*tokens)[i + 1].value, (*tokens)[i + 1].type);
                cacheNode->rightNode = cache;
            }

            wasPlusOrMinus = false;
            cache = cacheNode;
            i++;
        } else if (currentToken->type == _OP_PLUS_
            || currentToken->type == _OP_MINUS_) {
            if (cache != NULL) {
                struct storageItem item;
                item.cacheNode = cache;
                item.itemStartPos = startPosOfMultiplyOrDivide;
                item.itemEndPos = endPosOfMultiplyOrDivide;
                storage[storagePointer++] = item;
            }

            cache = NULL;
            wasPlusOrMinus = true;
        }

        if (i + 1 >= startPos + boundaries
            && cache != NULL) {
            struct storageItem item;
            item.cacheNode = cache;
            item.itemStartPos = startPosOfMultiplyOrDivide;
            item.itemEndPos = endPosOfMultiplyOrDivide;
            storage[storagePointer++] = item;
        }
    }

    for (size_t i = startPos; i < startPos + boundaries; i++) {
        if ((*tokens)[i].type == _OP_PLUS_
            || (*tokens)[i].type == _OP_MINUS_) {
            int mover = 1;
            //If value is greater than UNINITIALIZED, than the next Op is "*" or "/"
            //The value is also the position of the Operator
            int prevOpIsMultiplyOrDivide = UNINITIALZED;
            int nextOpIsMultiplyOrDivide = UNINITIALZED;

            while (i - mover > 0) {
                if ((*tokens)[i - mover].type == _OP_PLUS_
                    || (*tokens)[i - mover].type == _OP_MINUS_) {
                    break;
                } else if ((*tokens)[i - mover].type == _OP_MULTIPLY_
                    || (*tokens)[i - mover].type == _OP_DIVIDE_) {
                    prevOpIsMultiplyOrDivide = i - mover;
                    break;
                }

                mover++;
            }

            mover = 1;

            while (mover < boundaries) {
                if ((*tokens)[i + mover].type == _OP_PLUS_
                    || (*tokens)[i + mover].type == _OP_MINUS_) {
                    break;
                } else if ((*tokens)[i + mover].type == _OP_MULTIPLY_
                    || (*tokens)[i + mover].type == _OP_DIVIDE_) {
                    nextOpIsMultiplyOrDivide = i + mover;
                    break;
                }

                mover++;
            }

            if (nextOpIsMultiplyOrDivide > UNINITIALZED
                && prevOpIsMultiplyOrDivide > UNINITIALZED) {
                struct storageItem itemContainingPrevNode;
                int prevNodeStorageIndex = UNINITIALZED;
                struct storageItem itemContainingNextNode;
                int nextNodeStorageIndex = UNINITIALZED;

                for (int i = 0; i < storagePointer; i++) {
                    if (storage[i].itemStartPos <= prevOpIsMultiplyOrDivide
                        && storage[i].itemEndPos >= prevOpIsMultiplyOrDivide) {
                        itemContainingPrevNode = storage[i];
                        prevNodeStorageIndex = i;
                    }

                    if (storage[i].itemStartPos <= nextOpIsMultiplyOrDivide
                        && storage[i].itemEndPos >= nextOpIsMultiplyOrDivide) {
                        itemContainingNextNode = storage[i];
                        nextNodeStorageIndex = i;
                    }
                }

                struct Node *newNode = create_node((*tokens)[i].value, (*tokens)[i].type);
                newNode->leftNode = itemContainingPrevNode.cacheNode;
                newNode->rightNode = itemContainingNextNode.cacheNode;
                
                struct storageItem newItem;
                newItem.cacheNode = newNode;
                newItem.itemStartPos = itemContainingPrevNode.itemStartPos;
                newItem.itemEndPos = itemContainingNextNode.itemEndPos;

                struct storageItem nullItem;
                nullItem.cacheNode = NULL;
                nullItem.itemEndPos = -1;
                nullItem.itemStartPos = -1;

                storage[prevNodeStorageIndex] = newItem;
                storage[nextNodeStorageIndex] = nullItem;
            }
        } else {
            continue;
        }
    }

    cache = NULL;

    for (size_t i = startPos; i < startPos + boundaries; i++) {
        for (int n = 0; n < boundaries; n++) {
            if (storage[n].itemStartPos <= i
                && storage[n].itemEndPos >= i) {
                cache = storage[n].cacheNode;
                i += storage[n].itemEndPos;
                break;
            }
        }
        
        TOKEN *currentToken = &(*tokens)[i];

        if (currentToken->type == _OP_PLUS_
            || currentToken->type == _OP_MINUS_) {
            int isNextOpMultiplyOrDivide = UNINITIALZED;
            int jumper = 1;

            while (i + jumper < startPos + boundaries) {
                if ((*tokens)[i + jumper].type == _OP_PLUS_
                    || (*tokens)[i + jumper].type == _OP_MINUS_) {
                    break;
                } else if ((*tokens)[i + jumper].type == _OP_MULTIPLY_
                    || (*tokens)[i + jumper].type == _OP_DIVIDE_) {
                    isNextOpMultiplyOrDivide = i + jumper;
                    break;
                }

                jumper++;
            }

            struct Node *node = create_node(currentToken->value, currentToken->type);

            if (isNextOpMultiplyOrDivide > UNINITIALZED) {
                struct Node *storageNode = NULL;

                for (int n = 0; n < boundaries; n++) {
                    if (storage[n].itemStartPos <= isNextOpMultiplyOrDivide
                        && storage[n].itemEndPos >= isNextOpMultiplyOrDivide) {
                        storageNode = storage[n].cacheNode;
                    }
                }
                
                if (storageNode == NULL) {
                    printf("NODE ERROR\n");
                    exit(EXIT_FAILURE);
                }

                node->leftNode = cache;
                node->rightNode = storageNode;
            } else {
                node->rightNode = create_node((*tokens)[i + 1].value, (*tokens)[i + 1].type);
                node->leftNode = cache;
            }

            cache = node;
            i++;
        }
    }

    topNode = cache;

    (void)free(storage);
    return create_node_report(topNode, boundaries);
}

struct Node *create_node(char *value, TOKENTYPES type) {
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

NodeReport create_node_report(struct Node *topNode, int tokensToSkip) {
    NodeReport report;
    report.node = topNode;
    report.tokensToSkip = tokensToSkip;
    return report;
}

void append_node_to_root_node(struct Node *node) {
    RootNode.nodeCount++;
    RootNode.nodes = realloc(RootNode.nodes, sizeof(struct Node) * (RootNode.nodeCount + 1));

    if (RootNode.nodes == NULL) {
        (void)PARSE_TREE_NODE_RESERVATION_EXCEPTION();
    }

    RootNode.nodes[RootNode.nodeCount] = node;
}

int FREE_NODES() {
    for (size_t i = 0; i < RootNode.nodeCount; i++) {
        FREE_NODE(RootNode.nodes[i]);
        free(RootNode.nodes[i]);
    }

    return 1;
}

void FREE_NODE(struct Node *node) {
    if (node != NULL) {
        FREE_NODE(node->leftNode);
        FREE_NODE(node->rightNode);
        free(node);
    }
}