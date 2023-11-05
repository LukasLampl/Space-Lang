#ifndef SPACE_GRAMMAR_H_
#define SPACE_GRAMMAR_H_

#include "../headers/Token.h"

#define RULES_LENGTH 39
#define GRAMMAR_TOKEN_VALUE_LENGTH 64
#define GRAMMAR_TOKEN_LENGTH 32
#define GRAMMAR_RULE_OPTION_LENGTH 8

//////////////////////////////////////
///////////     GRAMMAR     //////////
//////////////////////////////////////

typedef enum GRAMMAR_REP {
    _UNDEFINED_ = 0,        _GR_IDENTIFIER_,        _EXPRESSION_,
    _VARIABLE_,             _CLASS_CALL_,           _FUNCTION_,
    _IF_,                   _WHILE_,                _TRY_,
    _FOR_,                  _CHECK_,                _INCLUDE_,
    _CLASS_,                _ENUM_,                 _EXPORT_,
    _RUNABLE_,              _TERM_,                 _DO_,
    _DIGIT_,                _LETTER_,               _BRACKET_,
    _BRACE_,                _SQUARE_BRACKET_,       _RATIONAL_OPERATORS_,
    _ARITHMETIC_OPERATOR_,  _ASSIGNMENT_OPERATORS_, _INCREMENT_OPERATOR_,
    _DECREMENT_OPERATOR_,   _UNDERSCORE_,           _BOOL_,
    _MODIFIER_,             _QUOTE_,                _LOGIC_OPERATOR_,
    _ATOM_,                 _STATEMENT_,            _ASSIGNMENT_,
    _ARRAY_ELEMENT_,        _FUNCTION_CALL_,        _GR_STRING_
} GRAMMAR_REP;

//////////////////////////
//     Grammar Rule     //
//////////////////////////

struct GrammarToken {
    char value[GRAMMAR_TOKEN_VALUE_LENGTH];
    GRAMMAR_REP rep;
    int ExactSpelling;
    int repeatable;

    // 0 = no Group; 1 = in Group; 2 = End of current Group. NOTE: ALL OBJECTS IN A GROUP
    // ARE AUTOMATICALLY MARKED AS REPEATABLE!
    int Group;

    // 0 = no choice; 1 = in choice section; 2 = End of current choice section
    int choice;
};

struct RuleOption {
    struct GrammarToken *tokens;
    size_t usedTokens;
};

typedef struct Rule {
    struct RuleOption options[GRAMMAR_RULE_OPTION_LENGTH];
    size_t usedOptions;

    GRAMMAR_REP rep;
} Rule;

struct RuleArray {
    Rule rules[5];
};

#endif  // SPACE_GRAMMAR_H_
