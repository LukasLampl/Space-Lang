#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "../headers/modules.h"
#include "../headers/errors.h"
#include "../headers/Token.h"
#include "../headers/syntaxChecker.h"
#include "../headers/Grammar.h"

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////     GRAMMAR LEXER     ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

//Functions//
GRAMMAR_REP get_representation(const struct GrammarToken *token);
GRAMMAR_REP getGlobalRules(const struct GrammarToken *token);

void get_grammar_file(char **GrammarBuffer, Rule** rules);
void tokenize_grammar(const size_t length, char **GrammarBuffer, Rule **rules);
void print_tokens(Rule **printRules);

void reserve_tokens(Rule **ruleArray);
void reserve_rules(Rule **rules);
void print_used_time(float cpu_time_used);
void close_up_token(struct GrammarToken *currentToken, const int currentTokenSymbol);
void resize_tokens(size_t currentLength, struct GrammarToken **tokens);
void check_for_embeded_representation(struct GrammarToken *token);

int skip_grammar_comment(const char *GrammarBuffer, const int currentIndex, const size_t Length);
int skip_grammar_whitespaces(char **GarmmarBuffer, const int currentIndex);
void increase_used_token(struct RuleOption *currenOption, const int currentTokenIndex);
void increase_rule_option(Rule *currentRule, const int currentOptionIndex);
int set_rule_representation(Rule *rule, struct GrammarToken *token, const int currentSymbol);
int process_group(struct GrammarToken *tokens, int currentToken, char *input, int currentSymbol);
void set_choice_in_token(struct RuleOption *currentOption, const int currentToken);

int rule_Reserved = 0;
int grammar_Reserved = 0;
int tokens_Reserved = 0;

/*
Purpose: Entry point of the grammar lexer
Return type: void
Params: void
*/
void Process_Grammar() {
    Rule *rules = NULL;
    char *GrammarBuffer = NULL;

    (void)reserve_rules(&rules);
    (void)_init_error_rules_cache_(&rules);

    (void)reserve_tokens(&rules);
    (void)_init_error_grammar_buffer_cache_(&GrammarBuffer);

    rule_Reserved = 1;
    tokens_Reserved = 1;

    //CLOCK FOR DEBUG PURPOSES ONLY!!!
    clock_t start, end;

    if (GRAMMAR_LEXER_DISPLAY_USED_TIME == 1) {
        start = (clock_t)clock();
    }

    (void)get_grammar_file(&GrammarBuffer, &rules); //Tokenize the space.grammar file

    (void)init_syntax_checker(rules);
    (void)Transmit_Grammar_To_Parsetree_Generator(&rules);

    //END CLOCK
    if (GRAMMAR_LEXER_DISPLAY_USED_TIME == 1) {
        end = (clock_t)clock();
    }

    if (GRAMMAR_LEXER_DISPLAY_GRAMMAR_PROCESSING == 1) {
        (void)print_tokens(&rules);
    }

    if (GRAMMAR_LEXER_DISPLAY_USED_TIME == 1) {
        (void)print_used_time(((double) (end - start)) / CLOCKS_PER_SEC);
    }

}

/*
Purpose: Reserve and empty memory for the rules array
Return type: void
Params: Rule **rules => rules array
*/
void reserve_rules(Rule **rules) { //calloc the needed memory for rules
    (*rules) = (Rule*)calloc(RULES_LENGTH, sizeof(Rule));

    if ((*rules) == NULL) {
        (void)PARSER_RULE_RESERVATION_EXCEPTION(); //On error throw an error
    }
}

/*
Purpose: Reserve and empty memory for all tokens in the rules array
Return type: void
Params: Rule **rulesArray => Array in which the tokens are in
*/
void reserve_tokens(Rule **ruleArray) {
    for (int i = 0; i < RULES_LENGTH; i++) {
        for (int n = 0; n < GRAMMAR_RULE_OPTION_LENGTH; n++) {
            (*ruleArray)[i].options[n].tokens = (struct GrammarToken*)calloc(GRAMMAR_TOKEN_LENGTH, sizeof(struct GrammarToken));

            if ((*ruleArray)[i].options[n].tokens == NULL) {
                (void)PARSER_RULE_RESERVATION_EXCEPTION();
            }
        }
    }
}

void print_used_time(float cpu_time_used) {
    (void)printf("\nCPU time used for GRAMMAR LEXING: %f seconds\n", cpu_time_used);
}

/*
Purpose: Get the rules from the space.grammar file in form of a string
Return type: void
Params: char **GrammarBuffer => Buffer in which the file is stored as a string;
        Rule **rules => Empty array, that is later on filled with the rules
        from the file
*/
void get_grammar_file(char **GrammarBuffer, Rule **rules) {
    const char *directory = "../SPACE/definitions/space.grammar"; //File directory, where the space.grammar file is stored

    //File to read
    FILE *fptr = (FILE *)fopen(directory, "r");

    if (fptr == NULL) {
        (void)IO_FILE_EXCEPTION((char *)directory, "GrammarLexer");
    }

    (void)fseek(fptr, 0L, SEEK_END);
    const long length = (long)ftell(fptr);

    if (length <= 0) {
        (void)IO_FILE_EXCEPTION((char *)directory, "GrammarLexer");
    }

    //Character buffer for all input symbols
    *GrammarBuffer = (char *)calloc(sizeof(char), length);

    if (*GrammarBuffer == NULL) {
        (void)IO_BUFFER_RESERVATION_EXCEPTION();
    }

    // Go back to the start of the file
    (void)rewind(fptr);

    // Read the contents of the file into the GrammarBuffer
    (void)fread(*GrammarBuffer, sizeof(char), length, fptr);
    grammar_Reserved = 1;

    (void)tokenize_grammar(length, GrammarBuffer, rules);

    if (fclose(fptr) == EOF) {
        (void)IO_FILE_CLOSING_EXCEPTION();
    }

}

/*
Purpose: Tokenize the GrammarFile to Rule, RuleOption and RuleTokens
Return type: void
Params: const size_t length => length of the GrammarFile;
        char **GrammarBuffer => Buffer in which the Grammar File is stored a a string;
        Rule **rules => Empty array, that is filled with the rules based on the
        file
*/
void tokenize_grammar(const size_t length, char **GrammarBuffer, Rule **rules) {
    if (*GrammarBuffer == NULL) {
        return;
    }

    int currentRuleIndex = 0;
    int currentTokenIndex = 0;
    int currentSymbolIndex = 0;
    int currentRuleOptionIndex = 0;

    int currentIncremental = 1;
    int currentTokenInGroup = 0;

    for (int i = 0; i < length; i++) {
        struct GrammarToken *currentToken = &(*rules)[currentRuleIndex].options[currentRuleOptionIndex].tokens[currentTokenIndex];

        if (currentTokenIndex + 1 >= ((GRAMMAR_TOKEN_LENGTH * currentIncremental) - 1)) {
            (void)resize_tokens(((GRAMMAR_TOKEN_LENGTH * currentIncremental) - 1), &(*rules)[currentRuleIndex].options[currentRuleOptionIndex].tokens);
            currentIncremental ++;
        }

        if ((*GrammarBuffer)[i] == '#') { //On comment skip the comment by increasing i by the length of the comment
            i += (int)skip_grammar_comment(*GrammarBuffer, i, length);
            continue;
        }

        if ((*GrammarBuffer)[i] == '$') {
            
            (void)close_up_token(currentToken, currentSymbolIndex);
            (void)check_for_embeded_representation(currentToken);

            if ((*GrammarBuffer)[i + 1] == '_') {
                currentSymbolIndex = 0;
                currentTokenIndex = 0;
                currentRuleOptionIndex = 0;
                currentIncremental = 1;
                currentRuleIndex ++;

                i++;
                continue;
            } else {
                currentSymbolIndex = 0;
                currentTokenIndex = 0;
                currentIncremental = 1;
                currentRuleOptionIndex ++;
                (void)increase_rule_option(*rules, currentRuleOptionIndex);
                continue;
            }
        }
        
        if (!isascii((*GrammarBuffer)[i])) { //Check whether the grammarfile has characters, that are not valid or not
            (void)PARSER_RULE_FILE_CORRUPTION_EXCEPTION();
        }

        if ((int)isspace((*GrammarBuffer)[i]) != 0) { //Filtering all whitespaces out of the grammer definition
            (void)close_up_token(currentToken, currentSymbolIndex);
            i += (int)skip_grammar_whitespaces(GrammarBuffer, i);
            continue;
        }
        
        if ((*GrammarBuffer)[i] == '/' && ((*GrammarBuffer)[i - 1] != '"' && (*GrammarBuffer)[i + 1] != '"')) { //Set all tokens.group within a group to true (modules.h)
            (void)close_up_token(currentToken, currentSymbolIndex);
            (void)set_choice_in_token((*rules)[currentRuleIndex].options, currentTokenIndex);

            if (!(int)isspace(currentToken->value[0])) {
                (void)check_for_embeded_representation(currentToken);
                currentTokenIndex ++;
            }
            
            currentSymbolIndex = 0;
            continue;
        }

        if ((*GrammarBuffer)[i] == '(' && (*rules)[currentRuleIndex].rep != _BRACKET_ 
        && currentTokenInGroup == 0) { //Start of a "Group" in the grammar

            currentTokenInGroup = 1;
            currentToken->Group = 1;
            continue;

        } else if ((*GrammarBuffer)[i] != ')' && currentTokenInGroup == 1) {

            currentToken->Group = 1;

        } else if ((*GrammarBuffer)[i] == ')'
            && (*rules)[currentRuleIndex].rep != _BRACKET_
            && currentTokenInGroup == 1) { //End the Group option

            currentToken->Group = 2;
            currentTokenInGroup = 0;
            continue;
        }

        if ((*GrammarBuffer)[i] == '-' && (*GrammarBuffer)[i + 1] == '>') { //Skip to the next token, if the input is "->"
            (void)close_up_token(currentToken, currentSymbolIndex);
            (void)increase_used_token(&(*rules)[currentRuleIndex].options[currentRuleOptionIndex], currentTokenIndex);

            if (!(int)isspace(currentToken->value[0])) {
                (void)check_for_embeded_representation(currentToken);
                currentTokenIndex ++;
            }

            currentSymbolIndex = 0;
            i++;
            continue;
        }
    	
        if ((*GrammarBuffer)[i] == '"') { //Check if the input is a Quote or not to set the token.exactspelling to true
            if ((*GrammarBuffer)[i - 1] == '\\') { //If the input before is '/' then set the current token value to '"'
                char *quote = "\"";
                (void)strcpy(currentToken->value, quote);
                i++;
                continue;
            }

            currentToken->ExactSpelling = 1;
            continue;
        }

        if ((*GrammarBuffer)[i] == '*' && (*rules)[currentRuleIndex].rep != _ARITHMETIC_OPERATOR_) { //Sets the repeatable option in the token
            currentToken->repeatable = 1;
            continue;
        }
        
        if ((*GrammarBuffer)[i] == ':') { //Skip all definition of next option indicators
            if ((*GrammarBuffer)[i + 1] == ':' && (*GrammarBuffer)[i + 2] == '=') {
                (void)close_up_token(currentToken, currentSymbolIndex);
                i += (int)set_rule_representation(&(*rules)[currentRuleIndex], currentToken, currentSymbolIndex);
                (void)check_for_embeded_representation(currentToken);

                currentTokenIndex = 0;
                currentSymbolIndex = 0;
                currentRuleOptionIndex = 0;
                continue;
            } else if ((*GrammarBuffer)[i + 1] == ':' && (*GrammarBuffer)[i + 2] != '=') {
                (void)close_up_token(currentToken, currentSymbolIndex);
                i++;
                continue;
            }
        }
        
        currentToken->value[currentSymbolIndex++] = (*GrammarBuffer)[i];
        (*rules)[currentRuleIndex].options[currentRuleOptionIndex].usedTokens = currentTokenIndex;
    }

}

void print_tokens(Rule **printRules) {
    for (int i = 0; i < RULES_LENGTH; i++) {
        for (int n = 0; n < ((*printRules)[i].usedOptions + 1); n++) {
            for (int j = 0; j < ((*printRules)[i].options[n].usedTokens + 1); j++) {

                (void)printf("Rule: %3i | Option: %3i | Token: %3i | choice: %2i | REP: %i | Value: %s\n", 
                i, n, j, (*printRules)[i].options[n].tokens[j].choice, (*printRules)[i].rep, (*printRules)[i].options[n].tokens[j].value);

            }
        }
    }
}

/*
Purpose: Skip comments in the GrammarBuffer
Return type: int => how many characters have to be skipped
Params: const char *GrammarBuffer => Buffer that holds the whole grammar;
        const int currentIndex => position of the current character in the buffer;
        const size_t Length => length of the GrammarBuffer 
*/
int skip_grammar_comment(const char *GrammarBuffer, const int currentIndex, const size_t Length) {
    int skipChars = 1;

    while (skipChars + currentIndex < Length && GrammarBuffer[currentIndex + skipChars] != '#') {
        skipChars ++;
    }

    return skipChars;
}

/*
Purpose: Skips the whitespaces in the input based on the current input pointer position
Return type: int => number of how many characters have to be skipped
Params: char **GrammarBuffer => string with all rules; const int currentIndex => position
        of the buffer pointer position (current character position in the buffer)
*/
int skip_grammar_whitespaces(char **GrammarBuffer, const int currentIndex) {
    int skipChars = 0;

    while ((int)isspace((*GrammarBuffer)[(currentIndex + skipChars) + 1]) != 0) {
        skipChars ++;
    }

    return skipChars;
}

/*
Purpose: Scale the GrammarTokens array if needed
Return type: void
Params: struct RuleOption *currentOption => option in which the token array is included;
        const int currentTokenIndex => current array length
*/
void increase_used_token(struct RuleOption *currentOption, const int currentTokenIndex) {
    currentOption->usedTokens = currentTokenIndex + 1;
}

/*
Purpose: Adds the escape sequence to the token ('\0')
Return type: void
Params: struct *GrammarToken => Token in which the escape character should be placed in;
        const int currentTokenSymbol => position of the last character
*/
void close_up_token(struct GrammarToken *currentToken, const int currentTokenSymbol) {
    currentToken->value[currentTokenSymbol + 1] = '\0';
}

/*
Purpose: Sets the usedOption variable to the currentRuleOptionIndex
Return type: void
Params: Rule *currentRule => Rule in which the option is contained;
        const int currentOptionIndex => Number of how many options are there
*/
void increase_rule_option(Rule *currentRule, const int currentOptionIndex) {
    currentRule->usedOptions = currentOptionIndex;
}

/*
Purpose: Set the representation of the current rule
Return type: int => characters to skip
Params: Rule *rule => rule in which the representation should be set in;
        struct GrammarToken *token => representation indicator Token;
        const int currentSymbol => Index of how many memory has to be reseted to 0
*/
int set_rule_representation(Rule *rule, struct GrammarToken *token, const int currentSymbol) {
    if (token != NULL) {
        rule->rep = (GRAMMAR_REP)get_representation(token);
        (void)memset(token->value, 0, currentSymbol);
        return 2;
    }

    rule->rep = _UNDEFINED_;
    return 2;
}

/*
Purpose: Set representation for possible embedded rules
Return type: void
Params: struct GrammarToken *token => token to be scanned for representation
*/
void check_for_embeded_representation(struct GrammarToken *token) {
    if (token != NULL) {
        token->rep = (GRAMMAR_REP)get_representation(token);
    }
}

/*
Purpose: Sets whether the token is one of an optional token
Return type: void
Params: struct RuleOption *currentOption => current option of the rule
        that gets processed; const int currentToken => current Token of the
        current Option
*/
void set_choice_in_token(struct RuleOption *currentOption, const int currentToken) {
    if (currentOption->usedTokens <= currentToken + 1) {
        currentOption->tokens[currentToken].choice = 1;
        currentOption->tokens[currentToken + 1].choice = 2;
    } else {
        currentOption->tokens[currentToken].choice = 2;
    }
}

/*
Purpose: Get all possible rule representation
Return type: GRAMMAR_REP => representation of the according rule based on the input token
Params: const struct GrammarToken *token => token to be checked
*/
GRAMMAR_REP get_representation(const struct GrammarToken *token) {
    
    struct symbol {
        char *value;
        GRAMMAR_REP type;
    };

    struct symbol lookup[] = {
        {"VARIABLE", _VARIABLE_},       {"CLASS_CALL", _CLASS_CALL_},
        {"FUNCTION", _FUNCTION_},       {"IDENTIFIER", _GR_IDENTIFIER_},
        {"IF", _IF_},                   {"WHILE", _WHILE_},
        {"DO", _DO_},                   {"TRY", _TRY_},
        {"FOR", _FOR_},                 {"CHECK", _CHECK_},
        {"INCLUDE", _INCLUDE_},         {"CLASS", _CLASS_},
        {"ENUM", _ENUM_},               {"EXPORT", _EXPORT_},
        {"STRING", _GR_STRING_}
    };
    
    for (int i = 0; i < (sizeof(lookup) / sizeof(lookup[0])); i++) {
        if ((char *)strstr(token->value, lookup[i].value) != NULL) {
            return lookup[i].type;
        }
    }
    
    return getGlobalRules(token);
}

/*
Purpose: Check whether the rule is a global rule or not
Return type: GRAMMAR_REP => which global rule gets represented
Params: const struct GrammarToken *token => token to be checked
*/
GRAMMAR_REP getGlobalRules(const struct GrammarToken *token) {

    struct symbol {
        char *value;
        GRAMMAR_REP type;
    };

    struct symbol lookup[] = {
        {"DIGIT", _DIGIT_},                             {"LETTER", _LETTER_},
        {"BRACKET", _BRACKET_},                         {"BRACE", _BRACE_},
        {"SQUARE_BRACKET", _SQUARE_BRACKET_},           {"RATIONAL_OPERATORS", _RATIONAL_OPERATORS_},
        {"ARITHMETIC_OPERATOR", _ARITHMETIC_OPERATOR_}, {"ASSIGNMENT_OPERATORS", _ASSIGNMENT_OPERATORS_},
        {"INCREMENT_OPERATOR", _INCREMENT_OPERATOR_},   {"DECREMENT_OPERATOR", _DECREMENT_OPERATOR_},
        {"UNDERSCORE", _UNDERSCORE_},                   {"BOOL", _BOOL_},
        {"MODIFIER", _MODIFIER_},                       {"QUOTE", _QUOTE_},
        {"LOGIC_OPERATOR", _LOGIC_OPERATOR_},           {"EXPRESSION", _EXPRESSION_},
        {"ATOM", _ATOM_},                               {"TERM", _TERM_},
        {"STATEMENT", _STATEMENT_},                     {"RUNABLE", _RUNABLE_},
        {"FUNCTION_CALL", _FUNCTION_CALL_},             {"ASSIGNMENT", _ASSIGNMENT_},
        {"ARRAY_ELEMENT", _ARRAY_ELEMENT_}
    };
    
    for (int i = 0; i < (sizeof(lookup) / sizeof(lookup[0])); i++) {
        if ((char *)strstr(token->value, lookup[i].value) != NULL) {
            return lookup[i].type;
        }
    }
    
    return _UNDEFINED_;
}

/*
Purpose: Free the GrammarBuffer
Return type: int => 1 = Freed sucessfully; 0 = Error at freeing the memory
Params: char *GrammarBuffer => GrammarBuffer that holds the Grammar in form of a string
*/
int FREE_GRAMMAR_BUFFER(char *GrammarBuffer) {
    if (grammar_Reserved == 1) {
        (void)free(GrammarBuffer);
        grammar_Reserved = 0;
    }

    return 1;
}

/*
Purpose: Resize the token count in a rule option, if the current token count is to small
Return type: void
Params: size_t currentLength => Current token length; struct GrammarToken **tokens => Array
        of the option tokens;
*/
void resize_tokens(size_t currentLength, struct GrammarToken **tokens) {
    if (*tokens != NULL && currentLength > 0) {
        *tokens = realloc(*tokens, sizeof(struct GrammarToken) * (currentLength * 2));

        if (*tokens == NULL) {
            (void)PARSER_RULE_RESERVATION_EXCEPTION();
        } else {
            (void)memset(*tokens + currentLength, 0, sizeof(struct GrammarToken) * currentLength);
        }
    }
}

/*
Purpose: Free the grammar rules one by one
Return type: int => 1 = freed successfully; 0 = Error occured
Params: Rule *rules => Rules array to be freed 
*/
int FREE_GRAMMAR_RULES(Rule *rules) {
    if (rule_Reserved == 1) {
        if (tokens_Reserved == 1) {
            for (int i = 0; i < RULES_LENGTH; i++) {
                for (int n = 0; n < GRAMMAR_RULE_OPTION_LENGTH; n++) {
                    (void)free(rules[i].options[n].tokens);
                }
            }

        }

        (void)free(rules);
        rule_Reserved = 0;
    }

    return 1;
}