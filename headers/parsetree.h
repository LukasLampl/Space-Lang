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

#ifndef SPACE_PARSETREE_GENERATOR_H_
#define SPACE_PARSETREE_GENERATOR_H_

#include "../headers/Token.h"

enum NodeType {
    _NULL_ = -1, _IDEN_NODE_ = 0,
    
    _STRING_NODE_, _NUMBER_NODE_, _FLOAT_NODE_, _BOOL_NODE_, _NULL_NODE_, _CHAR_ARRAY_NODE_, _THIS_NODE_,

    _MULTIPLY_NODE_, _DIVIDE_NODE_, _PLUS_NODE_, _MINUS_NODE_, _MODULO_NODE_, _NOT_NODE_,

    _MODIFIER_NODE_, _RET_TYPE_NODE_, _NAME_NODE_, _VALUE_NODE_, _FUNCTION_CALL_NODE_,
    _TERM_NODE_, _RUNNABLE_NODE_, _ENUMERATOR_NODE_, _INTERFACE_NODE_,
    _CLASS_CONSTRUCTOR_NODE_, _ARRAY_NODE_, _MEMBER_ACCESS_NODE_, _CONDITIONAL_ASSIGNMENT_NODE_,
    _ARRAY_ASSIGNMENT_NODE_, _ARRAY_DIM_NODE_, _ARRAY_CREATION_NODE_,

    _FUNCTION_NODE_, _VAR_NODE_, _ENUM_NODE_, _INCLUDE_NODE_, _EXPORT_NODE_, _CATCH_NODE_,
    _TRY_NODE_, _CLASS_NODE_, _ARRAY_VAR_NODE_, _ARRAY_CONST_NODE_, _CONST_NODE_, _CONDITIONAL_VAR_NODE_,
    _CONDITIONAL_CONST_NODE_, _WHILE_STMT_NODE_, _DO_STMT_NODE_, _CONST_CLASS_INSTANCE_NODE_,
    _VAR_CLASS_INSTANCE_NODE_, _INHERITED_CLASS_NODE_, _RETURN_STMT_NODE_, _BREAK_STMT_NODE_,
    _CONTINUE_STMT_NODE_, _CHECK_STMT_NODE_, _IS_STMT_NODE_, _FOR_STMT_NODE_, _IF_STMT_NODE_,
    _ELSE_IF_STMT_NODE_, _ELSE_STMT_NODE_, _VAR_TYPE_NODE_,

    _EQUALS_CONDITION_NODE_, _NOT_EQUALS_CONDITION_NODE_, _GREATER_CONDITION_NODE_,
    _SMALLER_CONDITION_NODE_, _GREATER_OR_EQUAL_CONDITION_NODE_, _SMALLER_OR_EQUAL_CONDITION_NODE_,
    _AND_NODE_, _OR_NODE_,

    _INCREMENT_ONE_NODE_, _DECREMENT_ONE_NODE_, _PLUS_EQUALS_NODE_, _MINUS_EQUALS_NODE_,
    _EQUALS_NODE_, _MULTIPLY_EQUALS_NODE_, _DIVIDE_EQUALS_NODE_, _SIMPLE_INC_DEC_ASS_NODE_,

    _VAR_DIM_NODE_, _INHERITANCE_NODE_, _PARAM_NODE_, _ARRAY_ACCESS_NODE_,
    _CLASS_ACCESS_NODE_, _POINTER_NODE_, _REFERENCE_NODE_, _MEM_CLASS_ACC_NODE_
};

/**
 * <p>
 * The Node is a crucial part of the parsetree generator.
 * A Node holds the line, position, type, value, details
 * (center nodes), left nodes and right nodes.
 * </p>
 * 
 * <p>
 * Nodes are used to reduce the total amout of tokens that are
 * used and only hold the most important data, which
 * then is processed further in the semantic analyzer.
 * </p>
 */
typedef struct Node {
    /**
     * <p>
     * Type of the node
     * </p>
     */
    enum NodeType type;

    /**
     * <p>
     * Value that the node holds (source name)
     * </p>
     */
    char *value;

    /**
     * <p>
     * Line at which the Node can be found in the source code
     * </p>
     */
    size_t line;

    /**
     * <p>
     * Position from the start in chars, at which the Node can be found
     * </p>
     */
    size_t position;

    /**
     * <p>
     * Node array that is in the center, this is useful for
     * parameters and type specifiers for instance.
     * </p>
     */
    struct Node **details;

    /**
     * <p>
     * Holds the size of the details array
     * </p>
     */
    size_t detailsCount;

    /**
     * <p>
     * Left Node appended to the parent Node (pointer)
     * </p>
     */
    struct Node *leftNode;
    
    /**
     * <p>
     * Right Node appended to the parent Node (pointer)
     * </p>
     */
    struct Node *rightNode;
} Node;

#endif