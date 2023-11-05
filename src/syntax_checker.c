#include "stdio.h"
#include "ctype.h"
#include "string.h"
#include "stdlib.h"
#include "../headers/syntaxChecker.h"
#include "../headers/Grammar.h"
#include "../headers/Token.h"
#include "../headers/errors.h"

Rule *SyntaxRules = NULL;

// Stack
struct Stack GrammarStack;

/*
Purpose: Init all Rules and Tokens
Return type: void
Params: Rule *InvokeRules => Pointer to the rule array;
*/
void init_syntax_checker(Rule *InvokeRules) {
    if (InvokeRules == NULL) {
        (void)PARSER_RULE_TRANSMISSION_EXCEPTION();
    }

    SyntaxRules = InvokeRules;
    init_stack(&GrammarStack);
}

GRAMMAR_REP get_representation_based_on_name(const char *sequence) {
    struct symbol {
        char name[21];
        GRAMMAR_REP rep;
    };

    struct symbol lookup[] = {
        {"IDENTIFIER", _GR_IDENTIFIER_},                  {"EXPRESSION", _EXPRESSION_},
        {"VARIABLE", _VARIABLE_},                         {"CLASS_CALL", _CLASS_CALL_},
        {"FUNCTION", _FUNCTION_},                         {"IF", _IF_},
        {"WHILE", _WHILE_},                               {"TRY", _TRY_},
        {"FOR", _FOR_},                                   {"CHECK", _CHECK_},
        {"INCLUDE", _INCLUDE_},                           {"CLASS", _CLASS_},
        {"ENUM", _ENUM_},                                 {"EXPORT", _EXPORT_},
        {"RUNABLE", _RUNABLE_},                           {"TERM", _TERM_},
        {"DO", _DO_},                                     {"DIGIT", _DIGIT_},
        {"LETTER", _LETTER_},                             {"BRACKET", _BRACKET_},
        {"BRACE", _BRACE_},                               {"SQUARE_BRACKET", _SQUARE_BRACKET_},
        {"RATIONAL_OPERATORS", _RATIONAL_OPERATORS_},     {"ARITHMETIC", _ARITHMETIC_OPERATOR_},
        {"ASSIGNMENT_OPERATORS", _ASSIGNMENT_OPERATORS_}, {"INCREMENT_OPERATOR", _INCREMENT_OPERATOR_},
        {"DECREMENT", _DECREMENT_OPERATOR_},              {"UNDERSCORE", _UNDERSCORE_},
        {"BOOL", _BOOL_},                                 {"MODIFIER", _MODIFIER_},
        {"QUOTE", _QUOTE_},                               {"LOGIC_OPERATOR", _LOGIC_OPERATOR_},
        {"ATOM", _ATOM_},                                 {"STATEMENT", _STATEMENT_},
        {"ASSIGNMENT", _ASSIGNMENT_},                     {"ARRAY_ELEMENT", _ARRAY_ELEMENT_},
        {"FUNCTION", _FUNCTION_},                         {"STRING", _GR_STRING_}
    };

    for (int i = 0; i < (sizeof(lookup) / sizeof(lookup[0])); i++) {
        if (strstr(sequence, lookup[i].name) != NULL) {
            return lookup[i].rep;
        }
    }

    return _UNDEFINED_;
}

/*
Purpose: Check if multiple characters are representetive to the representation
Return type: int => 1 = true (to the representation; e.g. sequence: "true" & rep: _BOOL_); 0 = false; -1 = Error / Unknown
Params: const char *sequence => sequence to be checked; GRAMMAR_REP representation => what the sequence should
        represent
*/
int check_sequence_after_representation(char *sequence, GRAMMAR_REP representation) {
    if (sequence != 0 && representation != _UNDEFINED_) {
        Rule *rule = get_rule(representation);

        if (rule->rep != _UNDEFINED_) {
            for (int i = 0; i < rule->usedOptions + 1; i++) {
                for (int n = 0; n < rule->options[i].usedTokens + 1; n++) {
                    if (strcmp(rule->options[i].tokens[n].value, sequence) == 0) {
                        return 1;
                    }
                }
            }
        } else {
            return -1;
        }
    }

    return 0;
}

/*
Purpose: Get the rule by passing which representation the rule has
Return type: Rule => The rule which has the same representation as the param
Params: GRAMMAR_REP rep => representation to search for
*/
Rule *get_rule(GRAMMAR_REP rep) {

    if (SyntaxRules == NULL) {
        PARSER_RULE_TRANSMISSION_EXCEPTION();
    }

    for (int i = 0; i < RULES_LENGTH; i++) {
        if (SyntaxRules[i].rep == rep) {
            return &SyntaxRules[i];
        }
    }

    return &SyntaxRules[0];
}

int check_for_rule(Rule *rule, const int ruleOption, const int currentRuleTokenIndex, TOKEN **tokens, size_t currentTokenIndex, size_t tokenLength, int canBeUsedMultipleTimes) {
    //struct StackElement stackElement = stack_peek(&GrammarStack);
    int verifiedCharacters = 0;

    for (int i = ruleOption; i < rule->usedOptions + 1; i++) {
        for (int j = currentRuleTokenIndex; j < (*rule).options[i].usedTokens + 1; j++) {
            struct GrammarToken currentRuleToken = (*rule).options[i].tokens[j];
            printf("cur: %s | %i | %s\n", currentRuleToken.value, get_representation_based_on_name(currentRuleToken.value), (*tokens)[currentTokenIndex + verifiedCharacters].value);
            if (currentRuleToken.ExactSpelling == 1) {
                if (((int)check_sequence_after_representation((*tokens)[currentTokenIndex + verifiedCharacters].value, get_representation_based_on_name(currentRuleToken.value)) == 1
                    || (int)strcmp(currentRuleToken.value, (*tokens)[currentTokenIndex + verifiedCharacters].value) == 0)
                    && canBeUsedMultipleTimes == 0) {
                    i = rule->usedOptions;
                    verifiedCharacters++;
                    continue;
                }

                if (canBeUsedMultipleTimes == 1) {
                    printf("\n%s\n", currentRuleToken.value);
                    int correctchars = check_rule_token_after_sequence((*tokens)[currentTokenIndex].value, rule);
                    verifiedCharacters += correctchars;

                    if (correctchars > 0) {
                        j = (*rule).options[i].usedTokens;
                        i = rule->usedOptions;
                        continue;
                    }
                }

                if (i < (rule->usedOptions + 1) && j >= ((*rule).options[i].usedTokens + 1)) {
                    printf("INCREMENT OPTION\n");
                    j = 0;
                    i ++;
                    verifiedCharacters = 0;
                    continue;
                }
                
                if (j < ((*rule).options[i].usedTokens + 1)) {
                    printf("IncTok\n");
                    continue;
                }
                
                (void)SYNTAX_MISMATCH_EXCEPTION((*tokens)[currentTokenIndex + verifiedCharacters].value, currentRuleToken.value);
            }

            if (currentRuleToken.value[0] == '[' || currentRuleToken.value[0] == '<') {
                
                struct StackElement elem = {
                    rule, i, j
                };

                printf("deb\n\n");
                (void)stack_push(&GrammarStack, elem);

                Rule *ruleToCheckFor = (Rule*)get_rule((GRAMMAR_REP)get_representation_based_on_name(currentRuleToken.value));
                verifiedCharacters += (int)check_for_rule(ruleToCheckFor, 0, 0, tokens, currentTokenIndex + verifiedCharacters, tokenLength, currentRuleToken.repeatable);
            }
        }
    }

    char *tokenValue = (*tokens)[currentTokenIndex].value;

    if (verifiedCharacters == (sizeof(tokenValue) / sizeof(tokenValue[0]))) {
        return verifiedCharacters;
    } else {
        printf("\nERROR\n");
        return 0;
    }
}

int check_rule_token_after_sequence(char *sequence, Rule *rule) {
    int validatedChars = 0;
    
    for (int i = 0; i < (sizeof(sequence) / sizeof(sequence[0])) - 1; i++) {
        for (int j = 0; j < rule->usedOptions + 1; j++) {
            for (int n = 0; n < (*rule).options[j].usedTokens + 1; n++) {
                struct GrammarToken currentGrammarToken = (*rule).options[j].tokens[n];

                if (currentGrammarToken.ExactSpelling == 1) {
                    printf("veryf: %c | %s\n", sequence[i], currentGrammarToken.value);
                    if (sequence[i] == currentGrammarToken.value[0]) {
                        printf("validated: %c\n", sequence[i]);
                        n = (*rule).options[j].usedTokens;
                        j = rule->usedOptions;
                        validatedChars ++;
                        continue;
                    }
                }
            }
        }
    }
    printf("val: %i\n", validatedChars);

    return validatedChars;
}