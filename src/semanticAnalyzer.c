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
#include "../headers/hashmap.h"
#include "../headers/parsetree.h"
#include "../headers/semantic.h"

/**
 * <p>
 * Defines the used booleans, 1 for true and 0 for false.
 * </p>
*/
#define true 1
#define false 0

struct MemberAccessList {
    size_t size;
    Node **nodes;
};

struct ParamTransferObject {
    size_t params;
    SemanticEntry **entries;
};

struct SemanticReport {
    int success;
    int errorOccured;
    enum VarType type;
};

struct SemanticEntryReport {
    int success;
    int errorOccured;
    SemanticEntry *entry;
};

struct varTypeLookup {
    char name[12];
    enum VarType type;
};

struct varTypeLookup TYPE_LOOKUP[] = {
    {"int", INTEGER}, {"double", DOUBLE}, {"float", FLOAT},
    {"short", SHORT}, {"long", LONG}, {"char", CHAR},
    {"boolean", BOOLEAN}, {"String", STRING}
};

void SA_manage_runnable(SemanticTable *parent, Node *root, SemanticTable *table);
void SA_add_parameters_to_runnable_table(SemanticTable *scopeTable, struct ParamTransferObject *params);

struct SemanticReport SA_evaluate_function_call(Node *topNode, SemanticEntry *functionEntry, SemanticTable *callScopeTable);
void SA_add_class_to_table(SemanticTable *table, Node *classNode);
void SA_add_function_to_table(SemanticTable *table, Node *functionNode);
void SA_add_normal_variable_to_table(SemanticTable *table, Node *varNode);
int SA_is_obj_already_defined(char *key, SemanticTable *scopeTable);
SemanticTable *SA_get_next_table_of_type(SemanticTable *currentTable, enum ScopeType type);

int SA_evaluate_assignment(enum VarType expectedType, Node *topNode, SemanticTable *table);
int SA_evaluate_simple_term(enum VarType expectedType, Node *topNode, SemanticTable *table);
int SA_evaluate_term_side(enum VarType expectedType, Node *node, SemanticTable *table);

SemanticTable *SA_create_new_scope_table(int paramCount, Node *root, enum ScopeType scope, SemanticTable *parent, struct ParamTransferObject *params);
int SA_is_node_a_number(Node *node);
int SA_is_node_arithmetic_operator(Node *node);
int SA_are_VarTypes_equal(enum VarType type1, enum VarType type2);

struct ParamTransferObject *SA_get_params(Node *topNode, enum VarType stdType);
struct SemanticReport SA_evaluate_member_access(Node *topNode, SemanticTable *table);
struct SemanticReport SA_check_restricted_member_access(Node *node, SemanticTable *table, SemanticTable *topScope);
struct SemanticReport SA_check_non_restricted_member_access(Node *node, SemanticTable *table, SemanticTable *topScope);

int SA_execute_access_type_checking(Node *cacheNode, SemanticTable *currentScope, SemanticTable *topScope);
SemanticTable *SA_get_next_table_with_declaration(Node *node, SemanticTable *table);
struct SemanticEntryReport SA_get_entry_if_available(Node *topNode, SemanticTable *table);
enum VarType SA_get_identifier_var_Type(Node *node);
enum VarType SA_get_var_type(Node *node);
enum Visibility SA_get_visibility(Node *visibilityNode);
char *SA_get_VarType_string(enum VarType type);
char *SA_get_ScopeType_string(enum ScopeType type);
SemanticEntry *SA_get_param_entry_if_available(char *key, SemanticTable *table);

struct SemanticEntryReport SA_create_semantic_entry_report(SemanticEntry *entry, int success, int errorOccured);
struct SemanticReport SA_create_semantic_report(enum VarType type, int success, int errorOccured);
SemanticEntry *SA_create_semantic_entry(char *name, char *value, enum VarType varType, enum Visibility visibility, enum ScopeType internalType, void *ptr);
SemanticTable *SA_create_semantic_table(int paramCount, int symbolTableSize, SemanticTable *parent, enum ScopeType type);

void FREE_TABLE(SemanticTable *rootTable);

void THROW_WRONG_ARGUMENT_EXCEPTION(Node *node);
void THROW_WRONG_ACCESSOR_EXEPTION(Node *node);
void THROW_STATEMENT_MISPLACEMENT_EXEPTION(Node *node);
void THROW_TYPE_MISMATCH_EXCEPTION(Node *node, char *expected, char *got);
void THROW_NOT_DEFINED_EXCEPTION(Node *node);
void THROW_ALREADY_DEFINED_EXCEPTION(Node *node);
void THROW_MEMORY_RESERVATION_EXCEPTION(char *problemPosition);
void THROW_EXCEPTION(char *message, Node *node);

int CheckSemantic(Node *root) {
    SemanticTable *mainTable = SA_create_new_scope_table(0, root, MAIN, NULL, NULL);
    (void)SA_manage_runnable(NULL, root, mainTable);
    (void)FREE_TABLE(mainTable);
    return 1;
}

void SA_manage_runnable(SemanticTable *parent, Node *root, SemanticTable *table) {
    printf("Main instructions count: %li\n", root->detailsCount);
    
    for (int i = 0; i < root->detailsCount; i++) {
        Node *currentNode = root->details[i];

        switch (currentNode->type) {
        case _VAR_NODE_:
            (void)SA_add_normal_variable_to_table(table, currentNode);
            break;
        case _FUNCTION_NODE_:
            (void)SA_add_function_to_table(table, currentNode);
            break;
        case _CLASS_NODE_:
            (void)SA_add_class_to_table(table, currentNode);
            break;
        default: continue;
        }
    }
    
    /*printf(">>>> SYMBOL TABLE <<<<\n");
    HM_print_map(table->symbolTable, true);*/

    /*char *search = "add";
    struct HashMapEntry *entry = HM_get_entry(search, mainTable->symbolTable);
    printf("\n\nEntry found: %s\n\n", entry == NULL ? "(null)" : "availabe");
    SemanticEntry *sentry = entry->value;
    printf("(%s) Name: %s, Value: %s, Vis: %i, Type: %i, Internal Type: %i, Reference: %p\n", search, sentry->name, sentry->value, sentry->visibility, sentry->type, sentry->internalType, (void*)sentry->reference);
    */
}

/**
 * <p>
 * Adds all parameters that are included in the ParameterTransferObject
 * into the parameter table of the local SemanticTable.
 * </p>
 * 
 * @param *params       Transfer object to add
 * @param *scopeTable   The current scope table
 */
void SA_add_parameters_to_runnable_table(SemanticTable *scopeTable, struct ParamTransferObject *params) {
    if (params == NULL) {
        return;
    }
    
    for (int i = 0; i < params->params; i++) {
        SemanticEntry *entry = params->entries[i];
        scopeTable->paramEntries[i] = entry;
    }

    /*printf(">>>> PARAMS TABLE <<<<\n");
    HM_print_map(map, true);*/
    (void)free(params->entries);
    (void)free(params);
}

struct SemanticReport SA_evaluate_function_call(Node *topNode, SemanticEntry *functionEntry,
                                                SemanticTable *callScopeTable) {
    if (functionEntry == NULL || topNode == NULL) {
        return SA_create_semantic_report(null, true, false);
    }
    
    SemanticTable *ref = (SemanticTable*)functionEntry->reference;

    if (ref == NULL) {
        return SA_create_semantic_report(null, true, false);
    } else if (topNode->detailsCount != ref->paramSize) {
        (void)THROW_WRONG_ARGUMENT_EXCEPTION(topNode);
        return SA_create_semantic_report(null, true, false);
    } else if (topNode->type != _FUNCTION_CALL_NODE_
        || ref->type != FUNCTION) {
        char *expected = SA_get_VarType_string(ref->type);
        char *got = SA_get_VarType_string(topNode->type);
        (void)THROW_TYPE_MISMATCH_EXCEPTION(topNode, expected, got);
        return SA_create_semantic_report(null, true, false);
    }

    for (int i = 0; i < topNode->detailsCount; i++) {
        Node *currentNode = topNode->details[i];
        enum VarType currentNodeType = CUSTOM;
        SemanticEntry *currentEntryParam = ref->paramEntries[i];

        if (currentNode->type == _MEM_CLASS_ACC_NODE_) {
            struct SemanticReport memAccRep = SA_evaluate_member_access(currentNode, callScopeTable);
            currentNodeType = memAccRep.type;
        } else if (currentNode->type == _FUNCTION_CALL_NODE_) {
            struct SemanticEntryReport tempEntry = SA_get_entry_if_available(currentNode, callScopeTable);
            struct SemanticReport fncCallRep = SA_evaluate_function_call(currentNode, tempEntry.entry, callScopeTable);
            currentNodeType = fncCallRep.type;
        } else {
            int success = SA_evaluate_simple_term(currentEntryParam->type, currentNode, callScopeTable);
            currentNodeType = success == true ? currentEntryParam->type : SA_get_var_type(currentNode);

            if (success == true) {
                currentNodeType = currentEntryParam->type;
            } else {
                currentNodeType = SA_get_var_type(currentNode);
            }
        }

        if ((int)SA_are_VarTypes_equal(currentEntryParam->type, currentNodeType) == false) {
            char *expected = SA_get_VarType_string(currentEntryParam->type);
            char *got = SA_get_VarType_string(currentNodeType);
            struct Node *errorNode = currentNode->type == _MEM_CLASS_ACC_NODE_ ? currentNode->leftNode : currentNode;
            (void)THROW_TYPE_MISMATCH_EXCEPTION(errorNode, expected, got);
            return SA_create_semantic_report(null, true, false);
        }
    }

    return SA_create_semantic_report(functionEntry->type, false, true);
}

void SA_add_class_to_table(SemanticTable *table, Node *classNode) {
    if (table->type != MAIN) {
        (void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(classNode);
        return;
    }

    char *name = classNode->value;
    enum Visibility vis = SA_get_visibility(classNode->leftNode);

    int paramsCount = classNode->detailsCount;
    struct ParamTransferObject *params = SA_get_params(classNode, EXT_CLASS_OR_INTERFACE);
    Node *runnableNode = classNode->rightNode;

    SemanticTable *scopeTable = SA_create_new_scope_table(paramsCount, runnableNode, CLASS, table, params);
    scopeTable->name = name;
    
    SemanticEntry *referenceEntry = SA_create_semantic_entry(name, NULL, null, vis, CLASS, scopeTable);
    (void)HM_add_entry(name, referenceEntry, table->symbolTable);
    (void)SA_manage_runnable(table, runnableNode, scopeTable);
}

void SA_add_function_to_table(SemanticTable *table, Node *functionNode) {
    if (table->type != MAIN && table->type != CLASS) {
        (void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(functionNode);
        return;
    }

    char *name = functionNode->value;
    enum Visibility vis = SA_get_visibility(functionNode->leftNode);
    enum VarType type = SA_get_var_type(functionNode->rightNode);
    int paramsCount = functionNode->detailsCount - 1; //-1 because of the runnable
    struct ParamTransferObject *params = SA_get_params(functionNode, VARIABLE);
    Node *runnableNode = functionNode->details[paramsCount];

    SemanticTable *scopeTable = SA_create_new_scope_table(paramsCount, runnableNode, FUNCTION, table, params);
    scopeTable->name = name;

    SemanticEntry *referenceEntry = SA_create_semantic_entry(name, NULL, type, vis, FUNCTION, scopeTable);
    (void)HM_add_entry(name, referenceEntry, table->symbolTable);
    (void)SA_manage_runnable(table, runnableNode, scopeTable);
}

/**
 * <p>
 * Adds a variable as an entry into the current
 * Semantic table.
 * </p>
 * 
 * @param *table    Table to add the variable to
 * @param *varNode  AST-Node that defines a variable
 */
void SA_add_normal_variable_to_table(SemanticTable *table, Node *varNode) {
    char *name = varNode->value;
    enum Visibility vis = SA_get_visibility(varNode->leftNode);
    Node *typeNode = varNode->detailsCount > 0 ? varNode->details[0] : NULL;
    enum VarType type = SA_get_var_type(typeNode);
    char *value = varNode->rightNode == NULL ? "(null)" : varNode->rightNode->value;

    if ((int)SA_is_obj_already_defined(name, table) == true) {
        (void)THROW_ALREADY_DEFINED_EXCEPTION(varNode);
    } else if ((int)SA_evaluate_assignment(type, varNode->rightNode, table) == false) {
        return;
    }

    SemanticEntry *entry = SA_create_semantic_entry(name, value, type, vis, VARIABLE, NULL);
    (void)HM_add_entry(name, entry, table->symbolTable);
}

/**
 * <p>
 * This function evaluates a simple term with the help of recursion.
 * </p>
 * 
 * <p>
 * First the topNode is checked for an arithmetic operator, if it is
 * not an arithmetic operator the one node is evaluated. If it is an
 * arithmetic operator the simple_term function is invoked again, until
 * the top node is not an arithmetic operator anymore.
 * </p>
 * 
 * <p><strong>Note:</strong>
 * This function also evaluates the optional typesafety!
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Simple term is valid
 * <li>false - Simple term is not valid
 * </ul>
 * 
 * @param expectedType      Type to check for typesafety
 * @param *topNode          Current top node to check
 * @param *table            Current scope table
 */
int SA_evaluate_simple_term(enum VarType expectedType, Node *topNode, SemanticTable *table) {
    int isTopNodeArithmeticOperator = (int)SA_is_node_arithmetic_operator(topNode);
    
    if (isTopNodeArithmeticOperator == true) {
        int leftTerm = (int)SA_evaluate_simple_term(expectedType, topNode->leftNode, table);
        int rightTerm = (int)SA_evaluate_simple_term(expectedType, topNode->rightNode, table);
        return leftTerm == false || rightTerm == false ? false : true;
    } else {
        return (int)SA_evaluate_term_side(expectedType, topNode, table);
    }
}

int SA_evaluate_term_side(enum VarType expectedType, Node *node, SemanticTable *table) {
    enum VarType predictedType = CUSTOM;

    if ((int)SA_is_node_a_number(node) == true) {
        predictedType = SA_get_identifier_var_Type(node);
    } else if (node->type == _NULL_NODE_) {
        predictedType = null; 
    } else if (node->type == _MEM_CLASS_ACC_NODE_
        || node->type == _IDEN_NODE_
        || node->type == _FUNCTION_CALL_NODE_) {
        struct SemanticReport rep = SA_evaluate_member_access(node, table);

        if (rep.errorOccured == true) {
            return false;
        }

        predictedType = rep.type;
        node = node->type == _MEM_CLASS_ACC_NODE_ ? node->leftNode : node;
    }

    if ((int)SA_are_VarTypes_equal(expectedType, predictedType) == false) {
        char *expected = SA_get_VarType_string(expectedType);
        char *got = SA_get_VarType_string(predictedType);
        (void)THROW_TYPE_MISMATCH_EXCEPTION(node, expected, got);
        return false;
    }

    return true;
}

/**
 * <p>
 * Evaluates a member acces as well as a class access.
 * </p>
 * 
 * @return A report containing the analysis result, like error occured, success and resulting type
 * 
 * @param *topNode      Start node of the member/class access tree
 * @param *table        Table in which the expression was written in (current scope)
 */
struct SemanticReport SA_evaluate_member_access(Node *topNode, SemanticTable *table) {
    SemanticTable *topScope = NULL;
    int restrictedMode = false;

    if (topNode->type == _MEM_CLASS_ACC_NODE_) {
        topScope = SA_get_next_table_with_declaration(topNode->leftNode, table);
    } else {
        topScope = SA_get_next_table_with_declaration(topNode, table);
        restrictedMode = true;
    }

    struct SemanticReport rep;

    if (restrictedMode == true) {
        rep = SA_check_restricted_member_access(topNode, table, topScope);
    } else {
        rep = SA_check_non_restricted_member_access(topNode, table, topScope);
    }

    if (rep.errorOccured == true) {
        return SA_create_semantic_report(null, false, true);
    }

    printf(">>>> >>>> >>>> EXIT! (%i)\n", rep.type);
    return SA_create_semantic_report(rep.type, true, false);
}

struct SemanticReport SA_check_non_restricted_member_access(Node *node, SemanticTable *table,
                                                            SemanticTable *topScope) {
    SemanticTable *currentScope = topScope;
    Node *cacheNode = node;
    enum VarType retType = CUSTOM;

    while (cacheNode != NULL) {
        struct SemanticEntryReport entry = SA_get_entry_if_available(cacheNode->leftNode, currentScope);
        
        if (entry.entry == NULL) {
            (void)THROW_NOT_DEFINED_EXCEPTION(cacheNode->leftNode);
            return SA_create_semantic_report(null, false, true);
        }

        retType = entry.entry->type;
        
        if (cacheNode->leftNode->type == _FUNCTION_CALL_NODE_) {
            if (entry.entry->internalType != FUNCTION) {
                (void)THROW_NOT_DEFINED_EXCEPTION(cacheNode->leftNode);
            }
            
            struct SemanticReport rep = SA_evaluate_function_call(cacheNode->leftNode, entry.entry, table);
            retType = rep.type;
        }

        currentScope = (struct SemanticTable*)entry.entry->reference;
        int checkRes = (int)SA_execute_access_type_checking(cacheNode, currentScope, topScope);

        if (checkRes == -1) {
            return SA_create_semantic_report(null, false, true);
        } else if (checkRes == 0) {
            break;
        }

        cacheNode = cacheNode->rightNode;
    }

    return SA_create_semantic_report(retType, true, false);
}

struct SemanticReport SA_check_restricted_member_access(Node *node, SemanticTable *table,
                                                            SemanticTable *topScope) {
    SemanticTable *currentScope = topScope;
    Node *cacheNode = node;
    enum VarType retType = CUSTOM;
    struct SemanticEntryReport entry = SA_get_entry_if_available(cacheNode, currentScope);

    if (entry.entry == NULL) {
        (void)THROW_NOT_DEFINED_EXCEPTION(cacheNode);
        return SA_create_semantic_report(null, false, true);
    }
    
    retType = entry.entry->type;
        
    if (cacheNode->type == _FUNCTION_CALL_NODE_) {
        if (entry.entry->internalType != FUNCTION) {
            (void)THROW_NOT_DEFINED_EXCEPTION(cacheNode);
        }
        
        struct SemanticReport rep = SA_evaluate_function_call(cacheNode, entry.entry, table);
        retType = rep.type;
    }

    currentScope = (SemanticTable*)entry.entry->reference;
    int checkRes = (int)SA_execute_access_type_checking(cacheNode, currentScope, topScope);

    if (checkRes == -1) {
        return SA_create_semantic_report(null, false, true);
    }

    return SA_create_semantic_report(retType, true, false);
}

/**
 * <p>
 * This function is specially designed to check the acces operator
 * used in a member/class access.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>-1 - Error occured (break)
 * <li>0 - Exit condition reached (break)
 * <li>1 - Nothing to worry about (continue)
 * </ul>
 * 
 * @param *cacheNode        Operator node
 * @param *currentScope     The SemanticTable of the current scope
 * @param *topScope         The SemanticTable when entering the member access checking
 * 
 */
int SA_execute_access_type_checking(Node *cacheNode, SemanticTable *currentScope,
                                    SemanticTable *topScope) {
    if (cacheNode->rightNode != NULL) {
        if (cacheNode->rightNode->type == _CLASS_ACCESS_NODE_) {
            if (currentScope->type != CLASS) {
                (void)THROW_WRONG_ACCESSOR_EXEPTION(cacheNode->rightNode);
                return -1;
            }
        } else if (cacheNode->rightNode->type == _MEMBER_ACCESS_NODE_) {
            if (topScope->type != CLASS
                || (int)strcmp(topScope->name, topScope->name) != 0) {
                (void)THROW_WRONG_ACCESSOR_EXEPTION(cacheNode->rightNode);
                return -1;
            }
        } else {
            return 0;
        }
    }

    return 1;
}

/**
 * <p>
 * Get the SemanticTable with the provided declaration.
 * </p>
 * 
 * <p><strong>Note:</strong>
 * Since classes and variables are only allowed to be used, if declared
 * prior, the function only has to check the parent tables until it reaches
 * the MAIN table.
 * </p>
 * 
 * @return Table that contains the declaration, if non was found it returns NULL
 * 
 * @param *node     Node to search / identifier / function call to search
 * @param *table    The table in which the searching starts (current scope).
 */
SemanticTable *SA_get_next_table_with_declaration(Node *node, SemanticTable *table) {
    SemanticTable *temp = table;

    while ((int)HM_contains_key(node->value, temp->symbolTable) == false
        && SA_get_param_entry_if_available(node->value, temp) == NULL) {
        temp = temp->parent;
    }

    return temp;
}

struct SemanticEntryReport SA_get_entry_if_available(Node *topNode, SemanticTable *table) {
    if (topNode == NULL) {
        return SA_create_semantic_entry_report(NULL, false, true);
    }
    
    SemanticEntry *entry = SA_get_param_entry_if_available(topNode->value, table);
    
    if ((int)HM_contains_key(topNode->value, table->symbolTable) == true) {
        entry = HM_get_entry(topNode->value, table->symbolTable)->value;
    }

    if (entry == NULL) {
        return SA_create_semantic_entry_report(NULL, false, true);
    }

    return SA_create_semantic_entry_report(entry, true, false);
}

SemanticTable *SA_get_next_table_of_type(SemanticTable *currentTable, enum ScopeType type) {
    SemanticTable *temp = currentTable;

    while (temp->type != MAIN) {
        if (type == CLASS && temp->type == CLASS) {
            break;
        }

        temp = temp->parent;
    }

    return temp;
}

int SA_evaluate_assignment(enum VarType expectedType, Node *topNode, SemanticTable *table) {
    return (int)SA_evaluate_simple_term(expectedType, topNode, table);
}

SemanticTable *SA_create_new_scope_table(int paramCount, Node *root, enum ScopeType scope,
                                                SemanticTable *parent, struct ParamTransferObject *params) {
    SemanticTable *table = SA_create_semantic_table(paramCount, root->detailsCount, NULL, scope);
    table->parent = parent;
    (void)SA_add_parameters_to_runnable_table(table, params);
    return table;
}

int SA_is_node_a_number(Node *node) {
    switch (node->type) {
    case _NUMBER_NODE_: case _FLOAT_NODE_:
        return true;
    default: return false;
    }
}

int SA_is_node_arithmetic_operator(Node *node) {
    switch (node->type) {
    case _PLUS_NODE_: case _MINUS_NODE_: case _MULTIPLY_NODE_: case _MODULO_NODE_:
    case _DIVIDE_NODE_:
        return true;
    default: return false;
    }
}

int SA_are_VarTypes_equal(enum VarType type1, enum VarType type2) {
    if (type1 == DOUBLE || type1 == FLOAT) {
        if (type2 == DOUBLE ||type2 == FLOAT) {
            return true;
        }

        return false;
    }
    
    if (type1 == CUSTOM) {
        return true;
    }

    return type1 == type2 ? true : false;
}

/**
 * <p>
 * Checks if an object is already defined or not.
 * </p>
 * 
 * <p>
 * The checking first occures on the lowest scope
 * going to the highest scope and then stops the search.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Object is already defined
 * <li>false - Object is not defined
 * </ul>
 * 
 * @param *key          Object name to search
 * @param *scopeTable   Current table in the current scope
 */
int SA_is_obj_already_defined(char *key, SemanticTable *scopeTable) {
    SemanticTable *temp = scopeTable;

    while (temp != NULL) {
        if ((int)HM_contains_key(key, temp->symbolTable) == true
            || SA_get_param_entry_if_available(key, temp) != NULL) {
            return true;
        }

        temp = temp->parent;
    }

    return false;
}

struct ParamTransferObject *SA_get_params(Node *topNode, enum VarType stdType) {
    struct ParamTransferObject *obj = (struct ParamTransferObject*)calloc(1, sizeof(struct ParamTransferObject));

    if (obj == NULL) {
        (void)THROW_MEMORY_RESERVATION_EXCEPTION("Param_Transfer_Object");
    }

    obj->entries = (SemanticEntry**)calloc(topNode->detailsCount, sizeof(SemanticEntry));

    if (obj->entries == NULL) {
        (void)THROW_MEMORY_RESERVATION_EXCEPTION("Parameter_Entries");
    }

    int actualParams = 0;

    for (int i = 0; i < topNode->detailsCount; i++) {
        Node *innerNode = topNode->details[i];

        if (innerNode->type == _RUNNABLE_NODE_) {
            continue;
        }

        Node *typeNode = innerNode->detailsCount > 0 ? innerNode->details[0] : NULL;
        enum VarType type = SA_get_var_type(typeNode);
        SemanticEntry *entry = SA_create_semantic_entry(innerNode->value, "null", type, GLOBAL, stdType, NULL);
        obj->entries[actualParams++] = entry;
    }

    obj->params = actualParams;
    return obj;
}

SemanticEntry *SA_get_param_entry_if_available(char *key, SemanticTable *table) {
    for (int i = 0; i < table->paramSize; i++) {
        if ((int)strcmp(table->paramEntries[i]->name, key) == 0) {
            return table->paramEntries[i];
        }
    }

    return NULL;
}

enum VarType SA_get_identifier_var_Type(Node *node) {
    switch (node->type) {
    case _FLOAT_NODE_:
        return DOUBLE;
    case _NUMBER_NODE_:
        return INTEGER;
    default:
        return CUSTOM;
    }
}

/**
 * <p>
 * Converts a node to the according VarType.
 * </p>
 * 
 * <p>
 * The VarType is received by the details.
 * </p>
 * 
 * @returns The converted VarType
 * 
 * @param *topNode  Node to convert
 */
enum VarType SA_get_var_type(Node *node) {
    if (node == NULL) {
        return CUSTOM;
    }

    int length = sizeof(TYPE_LOOKUP) / sizeof(TYPE_LOOKUP[0]);

    for (int i = 0; i < length; i++) {
        if ((int)strcmp(TYPE_LOOKUP[i].name, node->value) == 0) {
            return TYPE_LOOKUP[i].type;
        }
    }

    return CUSTOM;
}

/**
 * <p>
 * Converts a modifier string into a visiblity type.
 * </p>
 * 
 * @returns The converted visibility type
 * 
 * @param *visibilityNode   Node to convert
 */
enum Visibility SA_get_visibility(Node *visibilityNode) {
    if (visibilityNode == NULL) {
        return GLOBAL;
    } else if (visibilityNode->type != _MODIFIER_NODE_) {
        printf("MODIFIER NODE IS INCORRECT!\n\n");
        exit(EXIT_FAILURE);
    }

    if ((int)strcmp("global", visibilityNode->value) == 0) {
        return GLOBAL;
    } else if ((int)strcmp("secure", visibilityNode->value) == 0) {
        return SECURE;
    } else if ((int)strcmp("private", visibilityNode->value) == 0) {
        return PRIVATE;
    }

    return GLOBAL;
}

struct SemanticReport SA_create_semantic_report(enum VarType type, int success, int errorOccured) {
    struct SemanticReport rep;
    rep.type = type;
    rep.success = success;
    rep.errorOccured = errorOccured;
    return rep;
}

struct SemanticEntryReport SA_create_semantic_entry_report(SemanticEntry *entry, int success, int errorOccured) {
    struct SemanticEntryReport rep;
    rep.entry = entry;
    rep.success = success;
    rep.errorOccured = errorOccured;
    return rep;
}

/**
 * <p>
 * Creates an entry for the symboltable.
 * </p>
 * 
 * @returns A SemanticEntry with the provided information
 * 
 * @param *name         Name of the entry
 * @param *value        Value of the entry
 * @param varType       Return type / type of the entry
 * @param visibility    Visibility of the entry
 * @param *ptr          Pointer to a reference table (optional)
 */
SemanticEntry *SA_create_semantic_entry(char *name, char *value, enum VarType varType,
                                                enum Visibility visibility, enum ScopeType internalType,
                                                void *ptr) {
    SemanticEntry *entry = (SemanticEntry*)calloc(1, sizeof(SemanticEntry));
    entry->name = name;
    entry->reference = ptr;
    entry->type = varType;
    entry->visibility = visibility;
    entry->value = value;
    entry->internalType = internalType;
    return entry;
}

/**
 * <p>
 * Creates a semantic table and fills it with the provided information.
 * </p>
 * 
 * @returns A SemanticTable with the provided information
 * 
 * @param paramCount        How many parameters the table has
 * @param symbolTableSize   How many objects will be in the table (dynamic)
 * @param *parent           Pointer to the parent table
 * @param type              Type of the semantic table
 */
SemanticTable *SA_create_semantic_table(int paramCount, int symbolTableSize, SemanticTable *parent,
                                                enum ScopeType type) {
    SemanticTable *table = (SemanticTable*)calloc(1, sizeof(SemanticTable));

    if (table == NULL) {
        printf("Error on semantic table reservation!\n");
        return NULL;
    }

    if (paramCount > 0) {
        table->paramEntries = (SemanticEntry**)calloc(paramCount, sizeof(SemanticEntry));
        table->paramSize = paramCount;
    }

    table->symbolTable = CreateNewHashMap(symbolTableSize > 0 ? symbolTableSize : 1);
    table->parent = parent;
    table->type = type;
    return table;
}

/**
 * UNDER CONSTRUCTION!!!!
 */
void FREE_TABLE(SemanticTable *rootTable) {
    for (int i = 0; i < rootTable->paramSize; i++) {
        (void)free(rootTable->paramEntries[i]);
    }

    (void)free(rootTable->paramEntries);
    (void)HM_free(rootTable->symbolTable);
}

void THROW_WRONG_ARGUMENT_EXCEPTION(Node *node) {
    (void)THROW_EXCEPTION("WrongArgumentException", node);
}

void THROW_WRONG_ACCESSOR_EXEPTION(Node *node) {
    (void)THROW_EXCEPTION("WrongAccessorException", node);
}

void THROW_STATEMENT_MISPLACEMENT_EXEPTION(Node *node) {
    (void)THROW_EXCEPTION("StatementMisplacementException", node);
}

void THROW_TYPE_MISMATCH_EXCEPTION(Node *node, char *expected, char *got) {
    (void)THROW_EXCEPTION("TypeMismatchException", node);
    printf("Expected: %s, got %s\n", expected, got);
}

void THROW_NOT_DEFINED_EXCEPTION(Node *node) {
    (void)THROW_EXCEPTION("NotDefinedException", node);
}

void THROW_ALREADY_DEFINED_EXCEPTION(Node *node) {
    (void)THROW_EXCEPTION("AlreadyDefinedException", node);
}

void THROW_MEMORY_RESERVATION_EXCEPTION(char *problemPosition) {
    printf("MemoryReservationException: at %s\n", problemPosition);
    printf("Error was thrown while semantic analysis.\n");
    printf("This error is an internal issue, please recompile.\n");
    exit(EXIT_FAILURE);
}

/**
 * <p>
 * Throws an standard exception with the provided message and node details.
 * </p>
 * 
 * @param *message  Message / Exception to write
 * @param *node     Error node
 */
void THROW_EXCEPTION(char *message, Node *node) {
    printf("%s: at line %li from \"%s\"\n", message, node->line + 1, node->value);
}

char *SA_get_VarType_string(enum VarType type) {
    switch (type) {
    case INTEGER:
        return "INTEGER";
    case DOUBLE:
        return "DOUBLE";
    case FLOAT:
        return "FLOAT";
    case STRING:
        return "STRING";
    case LONG:
        return "LONG";
    case SHORT:
        return "SHORT";
    case BOOLEAN:
        return "BOOLEAN";
    case CHAR:
        return "CHAR";
    case CUSTOM:
        return "CUSTOM";
    case null:
        return "null";
    default: return "<REST>";
    }
}

char *SA_get_ScopeType_string(enum ScopeType type) {
    switch (type) {
    case VARIABLE:
        return "VARIABLE";
    case FUNCTION_CALL:
        return "FUNCTION_CALL";
    case CLASS:
        return "CLASS";
    case IF:
        return "IF";
    default: return "<REST>";
    }
}