#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../headers/modules.h"
#include "../headers/errors.h"
#include "../headers/Grammar.h"
#include "../headers/stack.h"
#include "../headers/parsetree.h"
#include "../headers/Token.h"
#include "../headers/syntaxChecker.h"

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////     PARSER     ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

Rule **GrammarRules = NULL;

GRAMMAR_REP predict_rule(TOKEN **tokens, int position);

// Parsetree Generation process
int Generate_Parsetree(TOKEN **tokens, size_t TokenLength) {

    if ((*tokens) == NULL) {
        (void)PARSER_TOKEN_TRANSMISSION_EXCEPTION();
    }

    if (*GrammarRules == NULL) {
        (void)PARSER_RULE_TRANSMISSION_EXCEPTION();
    }

    /*Rule *currentRule = get_rule(_VARIABLE_);
    int currentOptionIndex = 0;
    int currentTokenIndex = 0;
    
    for (size_t i = 0; i < TokenLength; i++) {
        printf("\nawaited: %s\n", (*currentRule).options[currentOptionIndex].tokens[currentTokenIndex].value);

        if (currentRule == NULL || tokens == NULL) {
            printf("An error occured at the parsetree geneartion: currentRule or tokens = NULL\n");
            printf("NULL can't be processed!\n");
            break;
        }

        struct GrammarToken currentToken = (*currentRule).options[currentOptionIndex].tokens[currentTokenIndex];

        if (currentToken.ExactSpelling == 1) {
            printf("Exact");
            if (strcmp(currentToken.value, (*tokens)[i].value) == 0) {
                printf(":true -> %s\n", currentToken.value);
                currentTokenIndex++;
                continue;
            } else {
                SYNTAX_MISMATCH_EXCEPTION((*tokens)[i].value, currentToken.value);
            }
        }

        if (currentToken.value[0] == '<') {
            if (check_for_rule(currentRule, 0, 0, tokens, i, TokenLength, 0)) {
                printf("Recursed\n");
                currentTokenIndex++;
                continue;
            }
        } else if (currentToken.value[0] == '[') {
            if (check_sequence_after_representation((*tokens)[i].value, get_representation_based_on_name(currentToken.value)) == 1) {
                currentTokenIndex++;
                continue;
            } else {
                SYNTAX_MISMATCH_EXCEPTION((*tokens)[i].value, currentToken.value);
            }
        }
    }*/

    return 1;
}

GRAMMAR_REP predict_rule(TOKEN **tokens, int position) {
    if (*tokens != NULL) {
        switch ((*tokens)[position].type) {
        case _KW_VAR_:      return _VARIABLE_;
        case _KW_WHILE_:    return _WHILE_;
        case _KW_FOR_:      return _FOR_;
        case _KW_FUNCTION_: return _FUNCTION_;
        case _KW_CONST_:    return _VARIABLE_;
        case _KW_ENUM_:     return _ENUM_;
        case _KW_CLASS_:    return _CLASS_;
        case _KW_DO_:       return _DO_;
        case _KW_TRY_:      return _TRY_;
        case _KW_CHECK_:    return _CHECK_;
        case _KW_INCLUDE_:  return _INCLUDE_;
        case _KW_EXPORT_:   return _EXPORT_;
        default:            break;
        }
    }

    return _UNDEFINED_;
}

/*
Purpose: Get a pointer to the rules array to use it in this section
Return type: void
Params: Rule **rules => Pointer to the rules array
*/
void Transmit_Grammar_To_Parsetree_Generator(Rule **rules) {
    if (rules != NULL) {
        GrammarRules = rules;
    }
}