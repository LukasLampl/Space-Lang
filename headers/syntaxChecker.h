#ifndef SPACE_SYNTAXCHECKER_H_
#define SPACE_SYNTAXCHECKER_H_

#include "../headers/Token.h"
#include "../headers/Grammar.h"

void init_syntax_checker(Rule *InvokeRules);
int check_build_rules_after_grammar(struct GrammarToken *GrammarToken, TOKEN *token);

int check_sequence_after_representation(char *sequence, GRAMMAR_REP representation);
GRAMMAR_REP get_representation_based_on_name(const char *sequence);

int recurse_down_to_base(const TOKEN *token, struct Rule *expectedRule, int currentOption, int currentToken);
int check_for_rule(Rule *rule, const int ruleOption, const int currentRuleTokenIndex, TOKEN **tokens, size_t currentTokenIndex, size_t tokenLength, int canBeUsedMultipleTimes);
int check_optional_tokens(Rule *rule, const int optionIndex, TOKEN **tokens, size_t currentTokenIndex, size_t tokenLength);
int check_rule_token_after_sequence(char *sequence, Rule *rule);

Rule *get_rule(GRAMMAR_REP rep);

#endif  // SPACE_SYNTAXCHECKER_H_
