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

struct varTypeLookup {
    char name[12];
    enum VarType type;
};

struct varTypeLookup TYPE_LOOKUP[] = {
    {"int", INTEGER}, {"double", DOUBLE}, {"float", FLOAT},
    {"short", SHORT}, {"long", LONG}, {"char", CHAR},
    {"boolean", BOOLEAN}, {"String", STRING}
};


void SA_add_variable_to_table(struct SemanticTable *table, struct Node *varNode);
int SA_is_obj_already_defined(char *key, struct SemanticTable *scopeTable);
enum VarType SA_get_var_type(struct Node *topNode);
enum Visibility SA_get_visibility(struct Node *visibilityNode);
struct SemanticEntry *SA_create_semantic_entry(char *name, char *value, enum VarType varType, enum Visibility visibility, void *ptr);
struct SemanticTable *SA_create_semantic_table(int paramCount, int symbolTableSize, struct SemanticTable *parent, enum ScopeType type);

void THROW_ALREADY_DEFINED_EXCEPTION(struct Node *node);

int CheckSemantic(struct Node *root) {
    struct SemanticTable *mainTable = SA_create_semantic_table(0, root->detailsCount, NULL, MAIN);
    
    printf("Main instructions count: %li\n", root->detailsCount);
    
    for (int i = 0; i < root->detailsCount; i++) {
        switch (root->details[i]->type) {
        case _VAR_NODE_:
            (void)SA_add_variable_to_table(mainTable, root->details[i]);
            break;
        default: continue;
        }
    }
    
    HM_print_map(mainTable->symbolTable, true);
    struct HashMapEntry *entry = HM_get_entry("z", mainTable->symbolTable);
    struct SemanticEntry *sentry = entry->value;
    printf("z: %s, %s, %i, %i\n", sentry->name, sentry->value, sentry->visibility, sentry->type);
    return 1;
}

void SA_add_variable_to_table(struct SemanticTable *table, struct Node *varNode) {
    char *name = varNode->value;
    char *value = varNode->rightNode == NULL ? "(null)" : varNode->rightNode->value;
    enum Visibility vis = SA_get_visibility(varNode->leftNode);
    enum VarType type = SA_get_var_type(varNode);

    struct SemanticEntry *entry = SA_create_semantic_entry(name, value, type, vis, NULL);
    
    if ((int)SA_is_obj_already_defined(name, table) == true) {
        (void)THROW_ALREADY_DEFINED_EXCEPTION(varNode);
    }
    
    (void)HM_add_entry(name, entry, table->symbolTable);
}

int SA_is_obj_already_defined(char *key, struct SemanticTable *scopeTable) {
    struct SemanticTable *temp = scopeTable;

    while (temp != NULL) {
        if ((int)HM_contains_key(key, temp->symbolTable) == true
            || (int)HM_contains_key(key, temp->paramTable) == true) {
            return true;
        }

        temp = temp->parent;
    }

    return false;
}

enum VarType SA_get_var_type(struct Node *topNode) {
    if (topNode->details == NULL) {
        return CUSTOM;
    }

    struct Node *typeNode = topNode->details[0];
    int length = sizeof(TYPE_LOOKUP) / sizeof(TYPE_LOOKUP[0]);

    for (int i = 0; i < length; i++) {
        if ((int)strcmp(TYPE_LOOKUP[i].name, typeNode->value) == 0) {
            return TYPE_LOOKUP[i].type;
        }
    }

    return CUSTOM;
}

enum Visibility SA_get_visibility(struct Node *visibilityNode) {
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

struct SemanticEntry *SA_create_semantic_entry(char *name, char *value, enum VarType varType,
                                                enum Visibility visibility, void *ptr) {
    struct SemanticEntry *entry = (struct SemanticEntry*)calloc(1, sizeof(struct SemanticEntry));
    entry->name = name;
    entry->reference = ptr;
    entry->type = varType;
    entry->visibility = visibility;
    entry->value = value;
    return entry;
}

struct SemanticTable *SA_create_semantic_table(int paramCount, int symbolTableSize, struct SemanticTable *parent,
                                                enum ScopeType type) {
    struct SemanticTable *table = (struct SemanticTable*)calloc(1, sizeof(struct SemanticTable));

    if (table == NULL) {
        printf("Error on semantic table reservation!\n");
        return NULL;
    }

    if (paramCount > 0) {
        table->paramTable = CreateNewHashMap(paramCount);
    }

    table->symbolTable = CreateNewHashMap(symbolTableSize > 0 ? symbolTableSize : 1);
    table->parent = parent;
    return table;
}

void THROW_ALREADY_DEFINED_EXCEPTION(struct Node *node) {
    printf("AlreadyDefinedException: at %li (%s)\n", node->line, node->value);

    exit(EXIT_FAILURE);
}
