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
    null, EXT_CLASS_OR_INTERFACE,
    CUSTOM, CLASS_REF, CONSTRUCTOR_PARAM, ENUM_INT, EXTERNAL_RET
};

enum ScopeType {
    MAIN, FUNCTION, CLASS, IF, CHECK, IS, FOR, WHILE, DO,
    VARIABLE, FUNCTION_CALL, CONSTRUCTOR, ENUM, ENUMERATOR,
    EXTERNAL
};

struct VarDec {
    enum VarType type;
    int dimension;
    char *classType;
    int constant;
};

typedef struct SemanticEntry {
    char *name;
    char *value;
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

#endif