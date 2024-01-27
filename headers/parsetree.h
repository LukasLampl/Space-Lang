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

#ifndef SPACE_PARSETREE_H_
#define SPACE_PARSETREE_H_

#include "../headers/Token.h"

enum NodeType {
    _IDEN_NODE_ = 0, _STRING_NODE_, _NUMBER_NODE_, _FLOAT_NODE_, _BOOL_NODE_,
    _NULL_NODE_, _CLASS_ACCESS_NODE_, _POINTER_NODE_, _REFERENCE_NODE_,
    _FUNCTION_NODE_, _MODIFIER_NODE_, _RET_TYPE_NODE_, _NAME_NODE_,
    _MULTIPLY_NODE_, _DIVIDE_NODE_, _PLUS_NODE_, _MINUS_NODE_, _MODULO_NODE_,
    _VAR_NODE_, _FUNCTION_CALL_NODE_, _TERM_NODE_, _RUNNABLE_NODE_, _ENUM_NODE_,
    _ENUMERATOR_NODE_, _VALUE_NODE_, _INCLUDE_NODE_
};

struct Node {
    enum NodeType type;
    char *value;

    struct Node **details;
    size_t detailsCount;

    struct Node *leftNode;
    struct Node *rightNode;
};

struct RootNode {
    size_t nodeCount;
    struct Node **nodes;
};

#endif