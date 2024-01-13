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

struct storage {
    struct storageItem *items;
    size_t storageSize;
};

enum processDirection {
    LEFT = 0,
    RIGHT
};


void append_node_to_root_node(struct Node *node);
NodeReport create_simple_term_node(TOKEN **tokens, size_t startPos, size_t boundaries);
void manage_brackets(struct storage *listOfSubTrees, TOKEN **tokens, size_t startPos, size_t boundaries);
struct Node *manage_remaining_plus_and_minus(struct storage *listOfSubTrees, TOKEN **tokens, size_t startPos, size_t boundaries);
void manage_surrounded_plus_or_minus(struct storage *listOfSubTrees, TOKEN **tokens, size_t startPos, size_t boundaries);
void manage_multiplication_or_division_or_modulo_in_simple_term(struct storage *listOfSubTrees, TOKEN **tokens, size_t startPos, size_t boundaries);
struct storageItem create_null_storage_item();
char *get_token_identifier_value_by_position(size_t position, TOKEN **tokens, enum processDirection direction);
int is_operator(const TOKEN *token);
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
        NodeReport report = create_simple_term_node(tokens, i, TokenLength);

        printf("\nPRINT:\n");
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
NodeReport create_simple_term_node(TOKEN **tokens, size_t startPos, size_t boundaries) {
    struct Node *topNode = (struct Node*)malloc(sizeof(struct Node));
    topNode->value = "NULL";
    topNode->type = _UNDEF_;

    struct storageItem *items = (struct storageItem*)calloc(boundaries, sizeof(struct storageItem));
    struct storage *listOfSubTrees = (struct storage*)malloc(sizeof(struct storage));

    if (items == NULL || listOfSubTrees == NULL) {
        printf("POINTER ERROR\n");
        exit(EXIT_FAILURE);
    }

    listOfSubTrees->items = items;
    listOfSubTrees->storageSize = 0;

    (void)manage_brackets(listOfSubTrees, tokens, startPos, boundaries);
    (void)manage_multiplication_or_division_or_modulo_in_simple_term(listOfSubTrees, tokens, startPos, boundaries);

    /*
    Manipulation of the list by surrounded pluses or minuses.
    > If the term is: 2 * 3 + 4 / 5
    > 1) The list would contain follwing items: (1) 2 * 3 AND (2) 4 / 5
    > 2) The function searches for '+' and '-', which are surrounded by '/', '*' or '%'
    > 3) The function replacec the list with following item: (1) 2 * 3 + 4 / 5
    */
    (void)manage_surrounded_plus_or_minus(listOfSubTrees, tokens, startPos, boundaries);
    topNode = manage_remaining_plus_and_minus(listOfSubTrees, tokens, startPos, boundaries);

    (void)free(listOfSubTrees);
    (void)free(items);
    return create_node_report(topNode, boundaries);
}

void manage_brackets(struct storage *listOfSubTrees, TOKEN **tokens, size_t startPos, size_t boundaries) {
    for (size_t i = startPos; i < startPos + boundaries; i++) {
        TOKEN *currentToken = &(*tokens)[i];

        if (currentToken->type == _OP_RIGHT_BRACKET_) {
            size_t bound = 0;
            int openBrackets = 0;

            while (i + bound < startPos + boundaries) {
                if ((*tokens)[i + bound].type == _OP_LEFT_BRACKET_) {
                    openBrackets--;

                    if (openBrackets == 0) {
                        break;
                    }
                } else if ((*tokens)[i + bound].type == _OP_RIGHT_BRACKET_) {
                    openBrackets++;
                }

                bound++;
            }
            
            NodeReport bracketReport = create_simple_term_node(tokens, i + 1, bound - 1);
            listOfSubTrees->items[listOfSubTrees->storageSize].itemStartPos = i;
            listOfSubTrees->items[listOfSubTrees->storageSize].itemEndPos = i + bound;
            listOfSubTrees->items[listOfSubTrees->storageSize++].cacheNode = bracketReport.node;
            i += bracketReport.tokensToSkip;
        }
    }
}

struct Node *manage_remaining_plus_and_minus(struct storage *listOfSubTrees, TOKEN **tokens, size_t startPos, size_t boundaries) {
    struct Node *cache = NULL;

    for (size_t i = startPos; i < startPos + boundaries; i++) {
        if (i == startPos) {
            for (int n = 0; n < boundaries; n++) {
                if (listOfSubTrees->items[n].itemStartPos <= i
                    && listOfSubTrees->items[n].itemEndPos >= i) {
                    cache = listOfSubTrees->items[n].cacheNode;
                    i += listOfSubTrees->items[n].itemEndPos;
                    break;
                }
            }
        }
        
        TOKEN *currentToken = &(*tokens)[i];
        size_t skip = 0;

        if (currentToken->type == _OP_PLUS_
            || currentToken->type == _OP_MINUS_) {
            int isNextOpMultiplyOrDivide = UNINITIALZED;
            int jumper = 1;

            while (i + jumper < startPos + boundaries) {
                if ((*tokens)[i + jumper].type == _OP_PLUS_
                    || (*tokens)[i + jumper].type == _OP_MINUS_) {
                    break;
                } else if ((*tokens)[i + jumper].type == _OP_MULTIPLY_
                    || (*tokens)[i + jumper].type == _OP_DIVIDE_
                    || (*tokens)[i + jumper].type == _OP_MODULU_) {
                    isNextOpMultiplyOrDivide = i + jumper - 1;
                    break;
                }

                jumper++;
            }

            struct Node *node = create_node(currentToken->value, currentToken->type);

            if (isNextOpMultiplyOrDivide > UNINITIALZED) {
                struct Node *storageNode = NULL;

                for (int n = 0; n < boundaries; n++) {
                    if (listOfSubTrees->items[n].itemStartPos <= isNextOpMultiplyOrDivide
                        && listOfSubTrees->items[n].itemEndPos >= isNextOpMultiplyOrDivide) {
                        storageNode = listOfSubTrees->items[n].cacheNode;
                        skip = listOfSubTrees->items[n].itemEndPos - listOfSubTrees->items[n].itemStartPos;
                    }
                }
                
                if (storageNode == NULL) {
                    printf("NODE ERROR\n");
                    exit(EXIT_FAILURE);
                }

                node->rightNode = storageNode;
            } else {
                char *rightNodeValue = get_token_identifier_value_by_position(i + 1, tokens, RIGHT);
                node->rightNode = create_node(rightNodeValue, _IDENTIFIER_);
            }

            if (cache != NULL) {
                node->leftNode = cache;
            } else {
                char *leftValue = get_token_identifier_value_by_position(i - 1, tokens, LEFT);
                node->leftNode = create_node(leftValue, _IDENTIFIER_);
                i += skip;
            }

            cache = node;
            i++;
        }
    }

    return cache;
}

//CAREFUL THE STORAGE *LISTOFSUBTREES GETS MANIPULATED!!!!
void manage_surrounded_plus_or_minus(struct storage *listOfSubTrees, TOKEN **tokens, size_t startPos, size_t boundaries) {
    for (size_t i = startPos; i < startPos + boundaries; i++) {
        if ((*tokens)[i].type == _OP_PLUS_
            || (*tokens)[i].type == _OP_MINUS_) {
            int skipIteration = false;

            for (size_t n = 0; n < listOfSubTrees->storageSize; n++) {
                if (listOfSubTrees->items[n].itemStartPos <= i
                    && listOfSubTrees->items[n].itemEndPos >= i) {
                    i = listOfSubTrees->items[n].itemEndPos;
                    skipIteration = true;
                }
            }

            if (skipIteration == true) {
                continue;
            }

            int mover = 1;
            //If value is greater than UNINITIALIZED, than the next Op is "*" or "/"
            //The value is also the position of the Operator
            int prevOpIsMultiplyOrDivide = UNINITIALZED;
            int nextOpIsMultiplyOrDivide = UNINITIALZED;

            while (i - mover > startPos) {
                if ((*tokens)[i - mover].type == _OP_PLUS_
                    || (*tokens)[i - mover].type == _OP_MINUS_) {
                    break;
                } else if ((*tokens)[i - mover].type == _OP_MULTIPLY_
                    || (*tokens)[i - mover].type == _OP_DIVIDE_
                    || (*tokens)[i - mover].type == _OP_MODULU_) {
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
                    || (*tokens)[i + mover].type == _OP_DIVIDE_
                    || (*tokens)[i + mover].type == _OP_MODULU_) {
                    nextOpIsMultiplyOrDivide = i + mover;
                    break;
                }

                mover++;
            }

            if (nextOpIsMultiplyOrDivide > UNINITIALZED
                && prevOpIsMultiplyOrDivide > UNINITIALZED) {
                struct storageItem *itemContainingPrevNode = NULL;
                int prevNodeStorageIndex = UNINITIALZED;

                struct storageItem *itemContainingNextNode = NULL;
                int nextNodeStorageIndex = UNINITIALZED;

                for (int i = 0; i < listOfSubTrees->storageSize; i++) {
                    if ((*listOfSubTrees).items[i].itemStartPos <= prevOpIsMultiplyOrDivide
                        && (*listOfSubTrees).items[i].itemEndPos >= prevOpIsMultiplyOrDivide) {
                        itemContainingPrevNode = &(*listOfSubTrees).items[i];
                        prevNodeStorageIndex = i;
                    }

                    if ((*listOfSubTrees).items[i].itemStartPos <= nextOpIsMultiplyOrDivide
                        && (*listOfSubTrees).items[i].itemEndPos >= nextOpIsMultiplyOrDivide) {
                        itemContainingNextNode = &(*listOfSubTrees).items[i];
                        nextNodeStorageIndex = i;
                    }
                }

                if (itemContainingNextNode == NULL || itemContainingPrevNode == NULL) {
                    printf("PREV OR NEXT NODE == NULL PTR*");
                    exit(EXIT_FAILURE);
                }

                struct Node *newNode = create_node((*tokens)[i].value, (*tokens)[i].type);
                newNode->leftNode = itemContainingPrevNode->cacheNode;
                newNode->rightNode = itemContainingNextNode->cacheNode;
                
                struct storageItem newItem;
                newItem.cacheNode = newNode;
                newItem.itemStartPos = itemContainingPrevNode->itemStartPos;
                newItem.itemEndPos = itemContainingNextNode->itemEndPos;

                (*listOfSubTrees).items[prevNodeStorageIndex] = newItem;
                (*listOfSubTrees).items[nextNodeStorageIndex] = create_null_storage_item();
            }
        } else {
            continue;
        }
    }
}

void manage_multiplication_or_division_or_modulo_in_simple_term(struct storage *listOfSubTrees, TOKEN **tokens, size_t startPos, size_t boundaries) {
    struct Node *cache = NULL;
    size_t startPosOfMultiplyOrDivide = UNINITIALZED;
    size_t endPosOfMultiplyOrDivide = UNINITIALZED;
    int wasPlusOrMinus = true;

    for (size_t i = startPos; i < startPos + boundaries; i++) {
        TOKEN *currentToken = &(*tokens)[i];
        
        if (currentToken->type == _OP_MULTIPLY_
            || currentToken->type == _OP_DIVIDE_
            || currentToken->type == _OP_MODULU_) {
            struct Node *cacheNode = create_node(currentToken->value, currentToken->type);

            if (wasPlusOrMinus == true) {
                startPosOfMultiplyOrDivide = i - 1;
                endPosOfMultiplyOrDivide = i + 1;

                char *leftNodeValue = get_token_identifier_value_by_position(i - 1, tokens, LEFT);
                char *rightNodeValue = get_token_identifier_value_by_position(i + 1, tokens, RIGHT);
                int replacedRight = false;
                int replacedLeft = false;

                for (int n = 0; n < listOfSubTrees->storageSize; n++) {
                    if ((listOfSubTrees->items[n].itemStartPos <= i - 1
                        && listOfSubTrees->items[n].itemEndPos >= i - 1)) {
                        cacheNode->leftNode = listOfSubTrees->items[n].cacheNode;
                        endPosOfMultiplyOrDivide = listOfSubTrees->items[n].itemEndPos;
                        i = listOfSubTrees->items[n].itemEndPos;
                        listOfSubTrees->items[n] = create_null_storage_item();
                        replacedLeft = true;
                    }
                }

                for (int n = 0; n < listOfSubTrees->storageSize; n++) {
                    if ((listOfSubTrees->items[n].itemStartPos <= i + 1
                        && listOfSubTrees->items[n].itemEndPos >= i + 1)) {
                        cacheNode->rightNode = listOfSubTrees->items[n].cacheNode;
                        endPosOfMultiplyOrDivide = listOfSubTrees->items[n].itemEndPos;
                        i = listOfSubTrees->items[n].itemEndPos;
                        listOfSubTrees->items[n] = create_null_storage_item();
                        replacedRight = true;
                    }
                }
                
                if (replacedLeft == false) {
                    cacheNode->leftNode = create_node(leftNodeValue, _IDENTIFIER_);
                }

                if (replacedRight == false) {
                    cacheNode->rightNode = create_node(rightNodeValue, _IDENTIFIER_);
                }
            } else {
                char *rightNodeValue = get_token_identifier_value_by_position(i + 1, tokens, RIGHT);
                int replacedRight = false;

                for (int n = 0; n < listOfSubTrees->storageSize; n++) {
                    if ((listOfSubTrees->items[n].itemStartPos <= i + 1
                        && listOfSubTrees->items[n].itemEndPos >= i + 1)) {
                        cacheNode->rightNode = listOfSubTrees->items[n].cacheNode;
                        endPosOfMultiplyOrDivide = listOfSubTrees->items[n].itemEndPos;
                        i = listOfSubTrees->items[n].itemEndPos;
                        listOfSubTrees->items[n] = create_null_storage_item();
                        replacedRight = true;
                    }
                }

                if (replacedRight == false) {
                    cacheNode->rightNode = create_node(rightNodeValue, _IDENTIFIER_);
                    endPosOfMultiplyOrDivide = i + 1;
                }

                cacheNode->leftNode = cache;
            }

            wasPlusOrMinus = false;
            cache = cacheNode;
        } else if (currentToken->type == _OP_PLUS_
            || currentToken->type == _OP_MINUS_) {
            if (cache != NULL) {
                struct storageItem item;
                item.cacheNode = cache;
                item.itemStartPos = startPosOfMultiplyOrDivide;
                item.itemEndPos = endPosOfMultiplyOrDivide;
                (*listOfSubTrees).items[listOfSubTrees->storageSize++] = item;
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
            (*listOfSubTrees).items[listOfSubTrees->storageSize++] = item;
        }
    }
}

struct storageItem create_null_storage_item() {
    struct storageItem nullItem;
    nullItem.cacheNode = NULL;
    nullItem.itemEndPos = -1;
    nullItem.itemStartPos = -1;

    return nullItem;
}

/*
Purpose: Create a node based on the params
Return Type: struct Node * => Pointer to the Node
Params: char *value => Value of the node;
        TOKENTYPES type => Type of the Node
*/
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

/*
Purpose: Get the value of all tokens that are in an IDENTIFIER
Return Type: char * => Pointer to the value
Params: size_t position => Position from where to start adding the values;
        TOKEN **tokens => Token array to be used;
        enum process direction => In which direction the IDENTIFIER is estimated to be
*/
char *get_token_identifier_value_by_position(size_t position, TOKEN **tokens, enum processDirection direction) {
    size_t requiredSize = 1;
    int mover = 0;

    if (direction == LEFT) {
        do {
            if ((int)is_operator(&(*tokens)[position - mover]) == true) {
                break;
            }
            
            requiredSize += (*tokens)[position - mover].size - 1;
            mover++;
        } while (mover <= position);
    } else if (direction == RIGHT && (*tokens)[position + 1].type != __EOF__) {
        do {
            if ((int)is_operator(&(*tokens)[position + mover]) == true) {
                break;
            }
            
            requiredSize += (*tokens)[position + mover].size - 1;
            mover++;
        } while ((*tokens)[position + mover].type != __EOF__);
    }
    
    if (requiredSize == 0) {
        printf("ERROR ON SIZING!\n");
        exit(EXIT_FAILURE);
    }

    char *value = (char*)calloc(requiredSize + 1, sizeof(char));

    if (value == NULL) {
        printf("ERROR OCCURED\n");
        exit(EXIT_FAILURE);
    }

    value[requiredSize] = '\0';
    int valuePointer = 0;

    if (mover == 0) {
        strncpy(value, (*tokens)[position].value, (*tokens)[position].size);
        return value;
    }

    if (direction == 0) {
        for (size_t i = position - mover + 1; i < position + 1; i++) {
            TOKEN *currentToken = &(*tokens)[i];
            
            for (size_t n = 0; n < currentToken->size - 1; n++) {
                if (valuePointer + 1 >= requiredSize) {
                    break;
                }

                value[valuePointer++] = currentToken->value[n];
            }
        }
    } else {
        for (size_t i = position; i < position + mover; i++) {
            TOKEN *currentToken = &(*tokens)[i];
            
            for (size_t n = 0; n < currentToken->size - 1; n++) {
                if (valuePointer + 1 >= requiredSize) {
                    break;
                }

                value[valuePointer++] = currentToken->value[n];
            }
        }
    }

    return value;
}

/*
Purpose: Check if a given TOKEN is an operator or not
Return Type: int => true = is operator; false = not an operator
Params: const TOKEN *token => Token to be checked
*/
const TOKENTYPES operators[] = {
_OP_EQUALS_, _OP_PLUS_, _OP_MINUS_, _OP_MULTIPLY_, _OP_DIVIDE_,
_OP_MODULU_, _OP_SEMICOLON_, _OP_LEFT_BRACKET_, _OP_RIGHT_BRACKET_};

int is_operator(const TOKEN *token) {
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
NodeReport create_node_report(struct Node *topNode, int tokensToSkip) {
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
void append_node_to_root_node(struct Node *node) {
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