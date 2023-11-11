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
    _KW_FOR_,                        _KW_THIS_,                       _INTEGER_,
    _CHARACTER_ARRAY_,               _OPERATOR_,                      _OP_PLUS_,
    _OP_MINUS_,                      _OP_MULTIPLY_,                   _OP_DIVIDE_,
    _OP_MODULU_,                     _OP_DOT_,                        _OP_COMMA_,
    _OP_LEFT_BRACKET_,               _OP_RIGHT_BRACKET_,              _OP_LEFT_BRACE_,
    _OP_RIGHT_BRACE_,                _OP_LEFT_EDGE_BRACKET_,          _OP_RIGHT_EDGE_BRACKET_,
    _OP_GREATER_CONDITION_,          _OP_SMALLER_CONDITION_,          _OP_NOT_,
    _OP_NOT_CONDITION_,              _OP_NOT_EQUALS_CONDITION_,       _OP_EQUALS_,
    _OP_EQUALS_CONDITION_,           _OP_GREATER_OR_EQUAL_CONDITION_, _OP_SMALLER_OR_EQUAL_CONDITION_,
    _OP_CLASS_ACCESSOR,              _OP_OVERWRITE_,                  _OP_SEMICOLON_,
    _OP_PLUS_EQUALS_,                _OP_MINUS_EQUALS_,               _OP_DIVIDE_EQUALS_,
    _OP_MULTIPLY_EQUALS_,            _OP_ADD_ONE_,                    _OP_SUBTRACT_ONE_,
    _OP_AND_,                        _NUMBER_,                        _STRING_,
    _FLOAT_,                         _UNDEF_
} TOKENTYPES;

///////////////////
//     Token     //
///////////////////

typedef struct TOKEN {
    TOKENTYPES type;
    char *value;
    size_t size;
} TOKEN;

#endif  // SPACE_TOKEN_H_
