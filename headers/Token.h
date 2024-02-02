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

#ifndef SPACE_TOKEN_H_
#define SPACE_TOKEN_H_

//////////////////////////////////////
///////////     TOKENS     ///////////
//////////////////////////////////////

/////////////////////////
//     Token Types     //
/////////////////////////

typedef enum TOKENTYPES {
    __EOF__ = 0,                     ___PROGRAM___,                   _IDENTIFIER_,
    _KW_WHILE_,                      _KW_IF_,                         _KW_FUNCTION_,
    _KW_VAR_,                        _KW_BREAK_,                      _KW_RETURN_,
    _KW_DO_,                         _KW_CLASS_,                      _KW_WITH_,
    _KW_NEW_,                        _KW_TRUE_,                       _KW_FALSE_,
    _KW_NULL_,                       _KW_ENUM_,                       _KW_CHECK_,
    _KW_IS_,                         _KW_TRY_,                        _KW_CATCH_,
    _KW_CONTINUE_,                   _KW_CONST_,                      _KW_INCLUDE_,
    _KW_AND_,                        _KW_OR_,                         _KW_GLOBAL_,
    _KW_SECURE_,                     _KW_PRIVATE_,                    _KW_EXPORT_,
    _KW_FOR_,                        _KW_THIS_,                       _KW_ELSE_,
    _KW_CONSTRUCTOR_,                _KW_INT_,                        _KW_DOUBLE_,
    _KW_FLOAT_,                      _KW_CHAR_,                       _KW_STRING_,
    _KW_BOOLEAN_,                    _KW_SHORT_,                      _KW_LONG_,
    _INTEGER_,
    _CHARACTER_ARRAY_,               _OPERATOR_,                      _OP_PLUS_,
    _OP_MINUS_,                      _OP_MULTIPLY_,                   _OP_DIVIDE_,
    _OP_MODULU_,                     _OP_DOT_,                        _OP_COMMA_,
    _OP_LEFT_BRACKET_,               _OP_RIGHT_BRACKET_,              _OP_LEFT_BRACE_,
    _OP_RIGHT_BRACE_,                _OP_LEFT_EDGE_BRACKET_,          _OP_RIGHT_EDGE_BRACKET_,
    _OP_GREATER_CONDITION_,          _OP_SMALLER_CONDITION_,          _OP_NOT_,
    _OP_NOT_EQUALS_CONDITION_,       _OP_EQUALS_,
    _OP_EQUALS_CONDITION_,           _OP_GREATER_OR_EQUAL_CONDITION_, _OP_SMALLER_OR_EQUAL_CONDITION_,
    _OP_CLASS_ACCESSOR_,              _OP_OVERWRITE_,                  _OP_SEMICOLON_,
    _OP_PLUS_EQUALS_,                _OP_MINUS_EQUALS_,               _OP_DIVIDE_EQUALS_,
    _OP_MULTIPLY_EQUALS_,            _OP_ADD_ONE_,                    _OP_SUBTRACT_ONE_,
    _OP_COLON_,                      _OP_CLASS_CREATOR_,              _OP_QUESTION_MARK_,
    _NUMBER_,                        _STRING_,                        _FLOAT_,
    _POINTER_,                       _REFERENCE_,                     _REFERENCE_ON_POINTER_,
    _UNDEF_,                         _LII_,

    //THE FOLLOWING DECLARATIONS ARE FOR THE PARSETREE AND
    //AFTER PARSETREE GENERATION
    _TERM_FUNCTION_CALL_
} TOKENTYPES;

///////////////////
//     Token     //
///////////////////

typedef struct TOKEN {
    TOKENTYPES type;
    char *value;
    size_t size;
    size_t line;
    size_t tokenStart;
} TOKEN;

#endif  // SPACE_TOKEN_H_
