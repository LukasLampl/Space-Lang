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

#ifndef SPACE_SEMANTIC_ANALYZER_H_
#define SPACE_SEMANTIC_ANALYZER_H_

enum Visibility {
    P_GLOBAL,
    GLOBAL,
    SECURE,
    PRIVATE
};

enum VarType {
    INTEGER, LONG, SHORT, DOUBLE, FLOAT, CHAR, BOOLEAN, STRING, VOID,
    null,
    CUSTOM, CLASS_REF, ENUM_REF, CONSTRUCTOR_PARAM, EXTERNAL_RET,
    E_FUNCTION_CALL, E_NON_FUNCTION_CALL, CLASS_DEF, INTERFACE_DEF
};

enum ScopeType {
    MAIN, FUNCTION, CLASS, IF, ELSE, ELSE_IF, CHECK, IS, FOR, WHILE, DO,
    VARIABLE, CLASS_INSTANCE, FUNCTION_CALL, CONSTRUCTOR, ENUM, ENUMERATOR,
    EXTERNAL, TRY, CATCH, FINALLY,
    EXT_CLASS_OR_INTERFACE, CLASS_INHERIT,
    CLASS_INTERFACE, INTERFACE
};

enum ExternalType {
    DECLARATION_CHECK,
    CORRECTNESS_CHECK
};

struct VarDec {
    enum VarType type;
    int dimension;
    char *typeName;
    int constant;
};

typedef struct SemanticEntry {
    char *name;
    struct VarDec dec;
    enum Visibility visibility;
    enum ScopeType internalType;
    void *reference;
    size_t line;
    size_t position;
} SemanticEntry;

typedef struct SemanticTable {
    struct List *paramList;
    struct HashMap *symbolTable;
    struct SemanticTable *parent;
    enum ScopeType type;
    char *name;
    size_t line;
    size_t position;
} SemanticTable;

typedef struct ExternalEntry {
    Node *node;
    char *fileName;
    enum ExternalType type;
} ExternalEntry;

#endif