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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "../headers/modules.h"
#include "../headers/errors.h"
#include "../headers/hashmap.h"
#include "../headers/list.h"
#include "../headers/parsetree.h"
#include "../headers/semantic.h"

/**
 * <p>
 * Defines the used booleans, 1 for true and 0 for false.
 * </p>
*/
#define true 1
#define false 0

/**
 * <p>
 * A collection of all errors that can be thrown by the semantic analyzer.
 * </p>
 */
enum ErrorType {
	NONE,												//Node error was found
	ALREADY_DEFINED_EXCEPTION,							//The var, class or function was defined prior
	NOT_DEFINED_EXCEPTION,								//The var, class or function wasn't defined before the invokation
	CONSTRUCTOR_NOT_DEFINED_EXCEPTION,					//The constructor doesn't exist
	CONSTRUCTOR_ALREADY_DEFINED_EXCEPTION,				//The constructor has been defined previously
	TYPE_MISMATCH_EXCEPTION,							//The return types or operation types do not match (e.g. char != int or Object != String)
	STATEMENT_MISPLACEMENT_EXCEPTION,					//The statement was placed in a scope, that is not suitable for the statement
	WRONG_ACCESSOR_EXCEPTION,							//The accessor is not used correcty ('->' for class accesses and '.' for member accesses)
	WRONG_ARGUMENT_EXCPEPTION,							//The amount of arguments is not equal or the types do not match
	MODIFIER_EXCEPTION,									//When the code tries to access functions or vars, that are 'hidden'
	NO_SUCH_ARRAY_DIMENSION_EXCEPTION,					//When an array reaches negative dimensions
	WRONG_LVAL_EXCEPTION,								//If the left hand side is not valid
	WRONG_RVAL_EXCEPTION,								//If the right hand side is not valid
	NON_BOOLEAN_CHECK_EXCEPTION,						//The condition does not end in a boolean
	NON_COMPARABLE_CHECK_EXCEPTION,

	ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION			//If an illegal arightmetic operation was made or an operation is misplaced
};

/**
 * <p>
 * The types of how a function call should be handled.
 * </p>
 * 
 * <p>
 * FNC_CALL is the least strict in type matching, while the constructor checking
 * also checks the types for equality.
 * </p>
 */
enum FunctionCallType {
	FNC_CALL,
	CONSTRUCTOR_CALL,
	CONSTRUCTOR_CHECK_CALL
};

/**
 * <p>
 * All different statuses a report can reach.
 * </p>
 */
enum ErrorStatus {
	SUCCESS,
	ERROR,
	NA
};

enum ExternalStatus {
	EXTERNAL_FOUND,
	NOT_AN_EXTERNAL
};

struct MemberAccessList {
	size_t size;
	Node **nodes;
};

struct ParamTransferObject {
	size_t params;
	SemanticEntry **entries;
};

struct ErrorContainer {
	char *description;
	char *explanation;
	char *suggestion;
};

struct SemanticReport {
	enum ErrorStatus status;
	struct VarDec dec;
	struct Node *errorNode;
	enum ErrorType errorType;
	struct ErrorContainer container;
};

struct SemanticEntryReport {
	int success;
	int errorOccured;
	SemanticEntry *entry;
};

struct varTypeLookup {
	char name[12];
	enum VarType type;
};

struct varTypeLookup TYPE_LOOKUP[] = {
	{"int", INTEGER}, {"double", DOUBLE}, {"float", FLOAT},
	{"short", SHORT}, {"long", LONG}, {"char", CHAR},
	{"boolean", BOOLEAN}, {"String", STRING}, {"void", VOID}
};

void SA_init_globals();
void SA_manage_runnable(Node *root, SemanticTable *table);
void SA_add_parameters_to_runnable_table(SemanticTable *scopeTable, struct ParamTransferObject *params);

struct SemanticReport SA_evaluate_function_call(Node *topNode, SemanticEntry *functionEntry, SemanticTable *callScopeTable, enum FunctionCallType fnccType);
void SA_add_class_to_table(SemanticTable *table, Node *classNode);
void SA_add_function_to_table(SemanticTable *table, Node *functionNode);
void SA_add_normal_variable_to_table(SemanticTable *table, Node *varNode);
void SA_add_conditional_variable_to_table(SemanticTable *table, Node *varNode);
void SA_add_instance_variable_to_table(SemanticTable *table, Node *varNode);
void SA_add_array_variable_to_table(SemanticTable *table, Node *varNode);
void SA_add_constructor_to_table(SemanticTable *table, Node *constructorNode);
void SA_add_enum_to_table(SemanticTable *table, Node *enumNode);
void SA_add_enumerators_to_enum_table(SemanticTable *enumTable, Node *topNode, struct VarDec type);
void SA_add_include_to_table(SemanticTable *table, Node *includeNode);
void SA_add_try_statement(SemanticTable *table, Node *tryNode, Node *parentNode, int index);
void SA_add_catch_statement(SemanticTable *table, Node *catchNode, Node *parentNode, int index);
void SA_add_while_or_do_to_table(SemanticTable *table, Node *whileDoNode);
void SA_add_if_to_table(SemanticTable *table, Node *ifNode);
void SA_add_else_if_to_table(SemanticTable *table, Node *elseIfNode, Node *parentNode, int index);
void SA_add_else_to_table(SemanticTable *table, Node *elseNode, Node *parentNode, int index);
void SA_check_break_or_continue_to_table(SemanticTable *table, Node *breakOrContinueNode);
void SA_add_return_to_table(SemanticTable *table, Node *returnNode);
void SA_add_for_to_table(SemanticTable *table, Node *forNode);
void SA_add_check_to_table(SemanticTable *table, Node *checkNode);
void SA_check_assignments(SemanticTable *table, Node *node);

struct SemanticReport SA_handle_return_in_constructor(Node *returnNode);
struct SemanticReport SA_handle_return_in_function(SemanticTable *functionTable, SemanticTable *table, Node *returnNode);
int SA_is_function_already_defined(struct ParamTransferObject *params, SemanticTable *table, Node *functionNode);
void SA_handle_check_statement_runnable(Node *runnableNode, SemanticTable *checkTable);
void SA_create_checkable_error_message(struct SemanticReport memberAccessReport, Node *checkableNode);
int SA_validate_checkable(struct SemanticReport memberAccessReport);
struct SemanticReport SA_evaluate_not_operator(Node *topNode, SemanticTable *table);
struct SemanticReport SA_set_scope_table_of_member_access(struct VarDec retType, Node *cachedNode, SemanticTable **currentScope, struct SemanticEntryReport foundEntry);
int SA_count_set_array_var_dimensions(Node *arrayVar);
int SA_count_total_array_dimensions(Node *arrayNode);
int SA_is_break_or_continue_placement_valid(SemanticTable *table);
char *SA_get_string(char bufferString[]);
struct SemanticReport SA_evaluate_chained_condition(SemanticTable *table, Node *rootNode);
int SA_is_obj_already_defined(char *key, SemanticTable *scopeTable);
SemanticTable *SA_get_next_table_of_type(SemanticTable *currentTable, enum ScopeType type);
struct SemanticReport SA_contains_constructor_of_type(SemanticTable *classTable, struct Node *paramHolder, enum FunctionCallType fnccType);
int SA_get_node_param_count(struct Node *paramHolder);
Node *SA_get_most_left_node_from_member_access(Node *node);

struct SemanticReport SA_validate_increment_and_decrement(Node *node, SemanticTable *table);
struct SemanticReport SA_evaluate_instance_creation(SemanticTable *table, Node *node);
struct SemanticReport SA_evaluate_assignment(struct VarDec expectedType, Node *topNode, SemanticTable *table);
struct SemanticReport SA_evaluate_conditional_assignment(struct VarDec expectedType, Node *topNode, SemanticTable *table);
struct SemanticReport SA_evaluate_array_creation(struct VarDec expectedType, Node *topNode, SemanticTable *table);
struct SemanticReport SA_evaluate_array_assignment(struct VarDec expectedType, Node *topNode, SemanticTable *table);
struct SemanticReport SA_evaluate_simple_term(struct VarDec expectedType, Node *topNode, SemanticTable *table);
struct SemanticReport SA_evaluate_term_side(struct VarDec expectedType, Node *node, SemanticTable *table);

struct SemanticReport SA_is_term_valid(struct VarDec type1, struct VarDec type2, Node *operatorNode, Node *rightNode, Node *leftNode);
SemanticTable *SA_create_new_scope_table(Node *root, enum ScopeType scope, SemanticTable *parent, struct ParamTransferObject *params, size_t line, size_t position);
int SA_is_node_arithmetic_operator(Node *node);
int SA_is_node_bit_operator(Node *node);
int SA_are_VarTypes_equal(struct VarDec type1, struct VarDec type2, int strict);
int SA_are_strict_VarTypes_equal(struct VarDec type1, struct VarDec type2);
int SA_are_non_strict_VarTypes_equal(struct VarDec type1, struct VarDec type2);

struct ParamTransferObject *SA_get_params(Node *topNode, enum ScopeType stdType, SemanticTable *table);
struct SemanticReport SA_evaluate_member_access(Node *topNode, SemanticTable *table);
enum ExternalStatus SA_handle_external_references(Node *node, SemanticTable *currentScope, struct SemanticEntryReport entry);
struct SemanticReport SA_check_restricted_member_access(Node *node, SemanticTable *table, SemanticTable *topScope);
struct SemanticReport SA_check_non_restricted_member_access(Node *node, SemanticTable *table, SemanticTable *topScope);
struct SemanticReport SA_handle_inherited_functions_and_vars(SemanticTable **currentScope, SemanticTable *table, Node *cacheNode, struct SemanticReport *resMemRep, struct SemanticEntryReport *entry);
struct SemanticReport SA_evaluate_potential_this_keyword(Node *node, Node **cacheNode, SemanticTable **currentScope, SemanticTable *table, struct VarDec *retType);
struct SemanticReport SA_handle_array_accesses(struct VarDec *currentType, struct Node *arrayAccStart, SemanticTable *table);
struct SemanticReport SA_execute_identifier_analysis(Node *currentNode, SemanticTable *callScopeTable, struct VarDec *currentNodeType, SemanticEntry *currentEntryParam, enum FunctionCallType fnccType);
struct SemanticReport SA_execute_function_call_precheck(SemanticTable *ref, Node *topNode,  enum FunctionCallType fnccType);
struct SemanticReport SA_evaluate_modifier(SemanticTable *currentScope, enum Visibility vis, Node *node, SemanticTable *topTable, int checkAccessOnly);

struct SemanticReport SA_execute_access_type_checking(Node *cacheNode, SemanticTable *currentScope, SemanticTable *topScope);
SemanticTable *SA_get_next_table_with_declaration(char *key, SemanticTable *table);
struct SemanticEntryReport SA_get_entry_if_available(char *NodeAsKey, SemanticTable *table);
struct VarDec SA_convert_identifier_to_VarType(Node *node);
struct VarDec SA_get_VarType(Node *node, int constant, SemanticTable *table);
int SA_set_VarType_type(Node *node, struct VarDec *cust);
enum Visibility SA_get_visibility(Node *visibilityNode);
char *SA_get_VarType_string(struct VarDec type);
char *SA_get_ScopeType_string(enum ScopeType type);
SemanticEntry *SA_get_param_entry_if_available(char *key, SemanticTable *table);

struct SemanticEntryReport SA_create_semantic_entry_report(SemanticEntry *entry, int success, int errorOccured);
struct SemanticReport SA_create_semantic_report(struct VarDec type, enum ErrorStatus status, Node *errorNode, enum ErrorType errorType, struct ErrorContainer container);
SemanticEntry *SA_create_semantic_entry(char *name, struct VarDec varType, enum Visibility visibility, enum ScopeType internalType, void *ptr, size_t line, size_t position);
ExternalEntry *SA_create_external_entry(char *fileName, Node *node, enum ExternalType type);
SemanticTable *SA_create_semantic_table(int paramCount, int symbolTableSize, SemanticTable *parent, enum ScopeType type, size_t line, size_t position);

void FREE_TABLE(SemanticTable *rootTable);

struct SemanticReport SA_create_expected_got_report(struct VarDec expected, struct VarDec got, Node *errorNode);
struct SemanticReport SA_create_already_defined_exception_report(char *collissionName, SemanticTable *currentTable, Node *node);
char *SA_convert_ScopeType_to_string(enum ScopeType type);

void THROW_ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION(struct SemanticReport rep);

void THROW_NON_COMPARABLE_CHECK_EXCEPTION(struct SemanticReport rep);
void THROW_NON_BOOLEAN_CHECK_EXCEPTION(struct SemanticReport rep);
void THROW_WRONG_RVAL_EXCEPTION(Node *node, char *description);
void THROW_WRONG_LVAL_EXCEPTION(Node *node, char *description);
void THROW_NO_SUCH_ARRAY_DIMENSION_EXCEPTION(struct SemanticReport rep);
void THROW_MODIFIER_EXCEPTION(struct SemanticReport rep);
void THROW_WRONG_ARGUMENT_EXCEPTION(struct SemanticReport rep);
void THROW_WRONG_ACCESSOR_EXEPTION(struct SemanticReport rep);
void THROW_STATEMENT_MISPLACEMENT_EXEPTION(struct SemanticReport rep);
void THROW_TYPE_MISMATCH_EXCEPTION(struct SemanticReport rep);
void THROW_CONSTRUCTOR_NOT_DEFINED_EXCEPTION(Node *node);
void THROW_NOT_DEFINED_EXCEPTION(Node *node);
void THROW_ALREADY_DEFINED_EXCEPTION(struct SemanticReport rep);
void THROW_CONSTRUCTOR_ALREADY_DEFINED_EXCEPTION(Node *node);
void THROW_MEMORY_RESERVATION_EXCEPTION(char *problemPosition);
void THROW_EXCEPTION(char *message, struct SemanticReport rep);
void THROW_ASSIGNED_EXCEPTION(struct SemanticReport rep);

extern char *FILE_NAME;
extern char **BUFFER;
extern size_t BUFFER_LENGTH;

struct VarDec nullDec = {null, 0, NULL, false};
struct VarDec externalDec = {EXTERNAL_RET, 0, NULL};
struct ErrorContainer nullCont = {NULL, NULL, NULL};
struct SemanticReport nullRep;

/**
 * <p>
 * This list holds all member accesses or class accesses that
 * are in an external file. Ready to checked by the linker.
 * </p>
 */
struct List *LIST_OF_EXTERNAL_ACCESSES = NULL;
struct List *LIST_OF_SYMBOL_TABLES = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DEBUG_print_from_top_node(Node *topNode, int depth, int pos) {
	if (topNode == NULL || topNode->value == NULL) {
		return;
	}

	for (int i = 0; i < depth; ++i) {
		if (i + 1 == depth) {
			(void)printf("+-- ");
		} else {
			(void)printf("|   ");
		}
	}

	if (pos == 0) {
		(void)printf("C: %s -> %i\n", topNode->value, topNode->type);
	} else if (pos == 1) {
		(void)printf("L: %s -> %i\n", topNode->value, topNode->type);
	} else {
		(void)printf("R: %s -> %i\n", topNode->value, topNode->type);
	}

	for (int i = 0; i < topNode->detailsCount; i++) {
		if (topNode->details[i] != NULL) {
			for (int n = 0; n < depth + 1; n++) {
				if (n + 1 == depth + 1) {
					(void)printf("+-- ");
				} else {
					(void)printf("|   ");
				}
			}

			(void)printf("(%s) detail: %s -> %i\n", topNode->value, topNode->details[i]->value, topNode->details[i]->type);
			(void)DEBUG_print_from_top_node(topNode->details[i]->leftNode, depth + 2, 1);
			(void)DEBUG_print_from_top_node(topNode->details[i]->rightNode, depth + 2, 2);

			for (int n = 0; n < topNode->details[i]->detailsCount; n++) {
				(void)DEBUG_print_from_top_node(topNode->details[i]->details[n], depth + 2, 0);
			}
		} else {
			(void)printf("(%s) detail: NULL -> NULL\n", topNode->value);
		}
	}

	(void)DEBUG_print_from_top_node(topNode->leftNode, depth + 1, 1);
	(void)DEBUG_print_from_top_node(topNode->rightNode, depth + 1, 2);
}

void DEBUG_print_list(struct List *list, int flag) {
	if (list == NULL) {
		return;
	}

	printf("List@[%p]\n", (void*)list);
	printf("List Capacity: %li\n", list->size);
	printf("\n");

	if (flag == 0) {
		return;
	}

	printf("%-11s|%-23s|\n", "Index", "VALUES");
	printf("-----------+-----------------------+\n");

	for (int i = 0; i < list->size; i++) {
		if (list->entries[i] == NULL) {
			printf("%11i|%-23s|\n", i, "(null)");
			continue;
		}

		ExternalEntry *e = (ExternalEntry*)list->entries[i];
		printf("%11i|%-23s|\n", i, e->node->value);
	}

	return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CheckSemantic(Node *root) {
	(void)SA_init_globals();

	SemanticTable *mainTable = SA_create_new_scope_table(root, MAIN, NULL, NULL, 0, 0);
	(void)SA_manage_runnable(root, mainTable);
	(void)FREE_TABLE(mainTable);
	printf(TEXT_COLOR_YELLOW "Total Externals: %li\n" TEXT_COLOR_RESET, LIST_OF_EXTERNAL_ACCESSES->load);
	DEBUG_print_list(LIST_OF_EXTERNAL_ACCESSES, true);
	return 1;
}

void SA_init_globals() {
	nullRep = SA_create_semantic_report(nullDec, SUCCESS, NULL, NONE, nullCont);
	LIST_OF_EXTERNAL_ACCESSES = CreateNewList(16);
}

void SA_manage_runnable(Node *root, SemanticTable *table) {
	printf("Main instructions count: %li\n", root->detailsCount);
	
	for (int i = 0; i < root->detailsCount; i++) {
		Node *currentNode = root->details[i];

		switch (currentNode->type) {
		case _VAR_NODE_:
		case _CONST_NODE_:
			(void)SA_add_normal_variable_to_table(table, currentNode);
			break;
		case _CONDITIONAL_VAR_NODE_:
		case _CONDITIONAL_CONST_NODE_:
			(void)SA_add_conditional_variable_to_table(table, currentNode);
			break;
		case _FUNCTION_NODE_:
			(void)SA_add_function_to_table(table, currentNode);
			break;
		case _CLASS_NODE_:
			(void)SA_add_class_to_table(table, currentNode);
			break;
		case _VAR_CLASS_INSTANCE_NODE_:
		case _CONST_CLASS_INSTANCE_NODE_:
			(void)SA_add_instance_variable_to_table(table, currentNode);
			break;
		case _ARRAY_VAR_NODE_:
		case _ARRAY_CONST_NODE_:
			(void)SA_add_array_variable_to_table(table, currentNode);
			break;
		case _CLASS_CONSTRUCTOR_NODE_:
			(void)SA_add_constructor_to_table(table, currentNode);
			break;
		case _ENUM_NODE_:
			(void)SA_add_enum_to_table(table, currentNode);
			break;
		case _INCLUDE_NODE_:
			(void)SA_add_include_to_table(table, currentNode);
			break;
		case _TRY_NODE_:
			(void)SA_add_try_statement(table, currentNode, root, i);
			break;
		case _CATCH_NODE_:
			(void)SA_add_catch_statement(table, currentNode, root, i);
			break;
		case _WHILE_STMT_NODE_:
		case _DO_STMT_NODE_:
			(void)SA_add_while_or_do_to_table(table, currentNode);
			break;
		case _IF_STMT_NODE_:
			(void)SA_add_if_to_table(table, currentNode);
			break;
		case _ELSE_IF_STMT_NODE_:
			(void)SA_add_else_if_to_table(table, currentNode, root, i);
			break;
		case _ELSE_STMT_NODE_:
			(void)SA_add_else_to_table(table, currentNode, root, i);
			break;
		case _CONTINUE_STMT_NODE_:
		case _BREAK_STMT_NODE_:
			(void)SA_check_break_or_continue_to_table(table, currentNode);
			break;
		case _RETURN_STMT_NODE_:
			(void)SA_add_return_to_table(table, currentNode);
			break;
		case _FOR_STMT_NODE_:
			(void)SA_add_for_to_table(table, currentNode);
			break;
		case _CHECK_STMT_NODE_:
			(void)SA_add_check_to_table(table, currentNode);
			break;
		case _PLUS_EQUALS_NODE_:
		case _MINUS_EQUALS_NODE_:
    	case _EQUALS_NODE_:
		case _MULTIPLY_EQUALS_NODE_:
		case _DIVIDE_EQUALS_NODE_:
		default:
			(void)SA_check_assignments(table, currentNode);
			break;
		}
	}
}

/**
 * <p>
 * Adds all parameters that are included in the ParameterTransferObject
 * into the parameter table of the local SemanticTable.
 * </p>
 * 
 * @param *params       Transfer object to add
 * @param *scopeTable   The current scope table
 */
void SA_add_parameters_to_runnable_table(SemanticTable *scopeTable, struct ParamTransferObject *params) {
	if (params == NULL) {
		return;
	}
	
	for (int i = 0; i < params->params; i++) {
		SemanticEntry *entry = params->entries[i];
		(void)L_add_item(scopeTable->paramList, entry);
	}

	(void)free(params->entries);
	(void)free(params);
}

void SA_add_class_to_table(SemanticTable *table, Node *classNode) {
	if (table->type != MAIN) {
		char *msg = "Classes have to be in the outermost scope.";
		char *exp = "If a class is not defined in the outermost scope, it is not reachable anymore.";
		char *sugg = "Maybe move the \"class\" to the outermost scope.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, classNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	char *name = classNode->value;
	enum Visibility vis = SA_get_visibility(classNode->leftNode);
	struct ParamTransferObject *params = SA_get_params(classNode, EXT_CLASS_OR_INTERFACE, table);
	Node *runnableNode = classNode->rightNode;

	if ((int)SA_is_obj_already_defined(name, table) == true) {
		struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(name, table, classNode);
		(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
		return;
	}

	SemanticTable *scopeTable = SA_create_new_scope_table(runnableNode, CLASS, table, params, classNode->line, classNode->position);
	scopeTable->name = name;
	
	SemanticEntry *referenceEntry = SA_create_semantic_entry(name, nullDec, vis, CLASS, scopeTable, classNode->line, classNode->position);
	(void)HM_add_entry(name, referenceEntry, table->symbolTable);
	(void)SA_manage_runnable(runnableNode, scopeTable);
}

void SA_add_function_to_table(SemanticTable *table, Node *functionNode) {
	if (table->type != MAIN && table->type != CLASS) {
		char *msg = "Functions are only allowed in classes and the outermost scope.";
		char *exp = "A function can't be defined anywhere, since it is a code snippet to run when called. If defined in another function for instance, the function is not reachable anymore.";
		char *sugg = "Maybe move the \"function\" to a MAIN or CLASS scope.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, functionNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	char *name = functionNode->value;
	enum Visibility vis = SA_get_visibility(functionNode->leftNode);
	struct SemanticReport modifierReport = SA_evaluate_modifier(table, vis, functionNode, table, false);

	if (modifierReport.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(modifierReport);
	}
	
	struct VarDec type = SA_get_VarType(functionNode->details == NULL ? NULL : functionNode->details[0], false, table);
	int paramsCount = functionNode->detailsCount - 1; //-1 because of the runnable
	struct ParamTransferObject *params = SA_get_params(functionNode, VARIABLE, table);

	if ((int)SA_is_obj_already_defined(name, table) == true) {
		if ((int)SA_is_function_already_defined(params, table, functionNode) == true) {
			struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(name, table, functionNode);
			(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
			return;
		}
	}

	Node *runnableNode = functionNode->details[paramsCount];
	SemanticTable *scopeTable = SA_create_new_scope_table(runnableNode, FUNCTION, table, params, functionNode->line, functionNode->position);
	scopeTable->name = name;

	SemanticEntry *referenceEntry = SA_create_semantic_entry(name, type, vis, FUNCTION, scopeTable, functionNode->line, functionNode->position);
	(void)HM_add_entry(name, referenceEntry, table->symbolTable);
	(void)SA_manage_runnable(runnableNode, scopeTable);
}

/**
 * <p>
 * Adds a variable as an entry into the current
 * Semantic table.
 * </p>
 * 
 * @param *table    Table to add the variable to
 * @param *varNode  AST-Node that defines a variable
 */
void SA_add_normal_variable_to_table(SemanticTable *table, Node *varNode) {
	if (table->type == ENUM) {
		char *msg = "Vars are not allowed within enums.";
		char *exp = "There's no possibility to define something in an enum, except for enumerators.";
		char *sugg = "Remove the \"var\" from the enum.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, varNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	char *name = varNode->value;
	enum Visibility vis = SA_get_visibility(varNode->leftNode);
	struct SemanticReport modifierReport = SA_evaluate_modifier(table, vis, varNode, table, false);

	if (modifierReport.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(modifierReport);
	}
	
	int constant = varNode->type == _VAR_NODE_ ? false : true;
	struct VarDec type = SA_get_VarType(varNode->details == NULL ? NULL : varNode->details[0], constant, table);
	SemanticTable *actualTable = table->type == TRY ? table->parent : table;
	//printf("Type: %i, %i, %s, %i\n", type.type, type.dimension, type.typeName == NULL ? "(null)" : type.typeName, type.constant);
	if ((int)SA_is_obj_already_defined(name, actualTable) == true) {
		struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(name, table, varNode);
		(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
		return;
	}

	if (varNode->rightNode != NULL) {
		if (type.dimension != 0) {
			struct VarDec expected = {type.type, 0, type.typeName, constant};
			struct SemanticReport rep = SA_create_expected_got_report(expected, type, varNode->details[0]);
			(void)THROW_TYPE_MISMATCH_EXCEPTION(rep);
		}

		struct SemanticReport assignmentRep = SA_evaluate_assignment(type, varNode->rightNode, actualTable);

		if (assignmentRep.status == ERROR) {
			(void)THROW_ASSIGNED_EXCEPTION(assignmentRep);
		}
	}

	SemanticEntry *entry = SA_create_semantic_entry(name, type, vis, VARIABLE, NULL, varNode->line, varNode->position);
	(void)HM_add_entry(name, entry, table->symbolTable);
}

/**
 * <p>
 * Adds a conditional variable as an entry into the current
 * Semantic table.
 * </p>
 * 
 * @param *table    Table to add the variable to
 * @param *varNode  AST-Node that defines a variable
 */
void SA_add_conditional_variable_to_table(SemanticTable *table, Node *varNode) {
	if (table->type == ENUM) {
		char *msg = "Vars are not allowed within enums.";
		char *exp = "There's no possibility to define something in an enum, except for enumerators.";
		char *sugg = "Remove the \"var\" from the enum.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, varNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	char *name = varNode->value;
	enum Visibility vis = SA_get_visibility(varNode->leftNode);
	struct SemanticReport modifierReport = SA_evaluate_modifier(table, vis, varNode, table, false);

	if (modifierReport.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(modifierReport);
	}

	int constant = varNode->type == _CONDITIONAL_VAR_NODE_ ? false : true;
	struct VarDec type = SA_get_VarType(varNode->details == NULL ? NULL : varNode->details[0], constant, table);
	SemanticTable *actualTable = table->type == TRY ? table->parent : table;

	if ((int)SA_is_obj_already_defined(name, actualTable) == true) {
		struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(name, table, varNode);
		(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
		return;
	}

	if (varNode->rightNode != NULL) {
		struct SemanticReport assignmentRep = SA_evaluate_conditional_assignment(type, varNode->rightNode, actualTable);

		if (assignmentRep.status == ERROR) {
			(void)THROW_ASSIGNED_EXCEPTION(assignmentRep);
		}
	}

	SemanticEntry *entry = SA_create_semantic_entry(name, type, vis, VARIABLE, NULL, varNode->line, varNode->position);
	(void)HM_add_entry(name, entry, table->symbolTable);
}

void SA_add_instance_variable_to_table(SemanticTable *table, Node *varNode) {
	char *name = varNode->value;
	enum Visibility vis = SA_get_visibility(varNode->leftNode);
	struct SemanticReport modifierReport = SA_evaluate_modifier(table, vis, varNode, table, false);

	if (modifierReport.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(modifierReport);
	}
	
	int constant = varNode->type == _VAR_CLASS_INSTANCE_NODE_ ? false : true;
	struct VarDec type = {CLASS_REF, 0, varNode->rightNode->value, constant};
	SemanticTable *actualTable = table->type == TRY ? table->parent : table;

	if ((int)SA_is_obj_already_defined(name, actualTable) == true) {
		struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(name, table, varNode);
		(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
		return;
	}

	if (varNode->rightNode != NULL) {
		struct SemanticReport instanceRep = SA_evaluate_instance_creation(table, varNode->rightNode);

		if (instanceRep.status == ERROR) {
			(void)THROW_ASSIGNED_EXCEPTION(instanceRep);
		}
	}

	SemanticEntry *entry = SA_create_semantic_entry(name, type, vis, CLASS_INSTANCE, NULL, varNode->line, varNode->position);
	(void)HM_add_entry(name, entry, table->symbolTable);
}

void SA_add_array_variable_to_table(SemanticTable *table, Node *varNode) {
	if (table->type == ENUM) {
		char *msg = "Vars are not allowed within enums.";
		char *exp = "There's no possibility to define something in an enum, except for enumerators.";
		char *sugg = "Remove the \"var\" from the enum.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, varNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	char *name = varNode->value;
	enum Visibility vis = SA_get_visibility(varNode->leftNode);
	struct SemanticReport modifierReport = SA_evaluate_modifier(table, vis, varNode, table, false);

	if (modifierReport.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(modifierReport);
	}

	int constant = varNode->type == _ARRAY_VAR_NODE_ ? false : true;
	int setDimensions = (int)SA_count_set_array_var_dimensions(varNode);
	struct VarDec type = SA_get_VarType(varNode->details == NULL ? NULL : varNode->details[0], constant, table);

	if (type.dimension != 0) {
		char *msg = "Setting array var type is not allowed when an array is defined.";
		char *exp = "Since the var is an array, the type specifier must only provide the \"deepest\" element type.";
		char *sugg = "Remove the \"[]\" from the type specifier.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, varNode->details[0], WRONG_ARGUMENT_EXCPEPTION, errCont);
		(void)THROW_ASSIGNED_EXCEPTION(rep);
	}

	type.dimension = setDimensions;
	SemanticTable *actualTable = table->type == TRY ? table->parent : table;

	if ((int)SA_is_obj_already_defined(name, actualTable) == true) {
		struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(name, table, varNode);
		(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
		return;
	}

	if (varNode->rightNode != NULL) {
		struct SemanticReport rep = nullRep;

		if (varNode->rightNode->type == _ARRAY_CREATION_NODE_) {
			rep = SA_evaluate_array_creation(type, varNode->rightNode, actualTable);
		} else {
			rep = SA_evaluate_array_assignment(type, varNode->rightNode, actualTable);
		}

		if (rep.status == ERROR) {
			(void)THROW_ASSIGNED_EXCEPTION(rep);
		}
	}

	SemanticEntry *entry = SA_create_semantic_entry(name, type, vis, VARIABLE, NULL, varNode->line, varNode->position);
	(void)HM_add_entry(name, entry, table->symbolTable);
}

void SA_add_constructor_to_table(SemanticTable *table, Node *constructorNode) {
	if (table->type != CLASS) {
		char *msg = "Constructors are only allowed in classes.";
		char *exp = "A function can't have a constructor for instance, since the function has a fixed set of params, while a class don't.";
		char *sugg = "Maybe remove the constructor or move it into a class.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, constructorNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	struct SemanticReport hasConstructor = SA_contains_constructor_of_type(table, constructorNode, CONSTRUCTOR_CALL);

	if ((int)hasConstructor.status == SUCCESS) {
		(void)THROW_CONSTRUCTOR_ALREADY_DEFINED_EXCEPTION(constructorNode);
		return;
	}

	char *name = SA_get_string("Constructor");
	struct Node *runnableNode = constructorNode->rightNode;
	struct VarDec constructDec = {CONSTRUCTOR_PARAM, 0, NULL};
	struct ParamTransferObject *params = SA_get_params(constructorNode, CONSTRUCTOR_PARAM, table);
	SemanticTable *scopeTable = SA_create_new_scope_table(constructorNode, CONSTRUCTOR, table, params, constructorNode->line, constructorNode->position);
	SemanticEntry *entry = SA_create_semantic_entry(name, constructDec, GLOBAL, CONSTRUCTOR, scopeTable, constructorNode->line, constructorNode->position);
	(void)L_add_item(table->paramList, entry);
	(void)SA_manage_runnable(runnableNode, scopeTable);
}

void SA_add_enum_to_table(SemanticTable *table, Node *enumNode) {
	if (table->type != MAIN) {
		char *msg = "Enums have to be in the outermost scope.";
		char *exp = "Enums are only allowed in the outer scope";
		char *sugg = "Move the enum to the outermost scope";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, enumNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	char *name = enumNode->value;
	enum Visibility vis = table->type == MAIN ? P_GLOBAL : GLOBAL;

	if ((int)SA_is_obj_already_defined(name, table) == true) {
		struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(name, table, enumNode);
		(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
		return;
	}

	struct VarDec enumDec = {ENUM_REF, 1, name, false};
	SemanticTable *scopeTable = SA_create_new_scope_table(enumNode, ENUM, table, NULL, enumNode->line, enumNode->position);
	(void)SA_add_enumerators_to_enum_table(scopeTable, enumNode, enumDec);
	SemanticEntry *entry = SA_create_semantic_entry(name, enumDec, vis, ENUM, scopeTable, enumNode->line, enumNode->position);
	(void)HM_add_entry(name, entry, table->symbolTable);
}

void SA_add_enumerators_to_enum_table(SemanticTable *enumTable, struct Node *topNode, struct VarDec type) {
	struct VarDec enumDec = {type.type, 0, type.typeName, type.constant};
	struct HashMap *valueMap = (struct HashMap*)CreateNewHashMap(4);

	for (int i = 0; i < topNode->detailsCount; i++) {
		struct Node *enumerator = topNode->details[i];

		if (enumerator == NULL) {
			continue;
		}

		char *name = enumerator->value;
		char *assignedValue = enumerator->rightNode->value;

		if ((int)HM_contains_key(name, enumTable->symbolTable) == true
			|| (int)HM_contains_key(assignedValue, valueMap) == true) {
			struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(name, enumTable, enumerator);
		(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
		return;
		}

		struct SemanticEntry *entry = SA_create_semantic_entry(name, enumDec, P_GLOBAL, ENUMERATOR, NULL, enumerator->line, enumerator->position);
		(void)HM_add_entry(name, entry, enumTable->symbolTable);
		(void)HM_add_entry(assignedValue, NULL, valueMap);
	}

	(void)HM_free(valueMap);
}

void SA_add_include_to_table(SemanticTable *table, struct Node *includeNode) {
	if (table->type != MAIN) {
		char *msg = "Includes have to be in the outermost scope.";
		char *exp = "External files and libraries must be included before their usage in the head of the file.";
		char *sugg = "Maybe move the include to the head of the file.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, includeNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	struct Node *actualInclude = NULL;
	struct Node *cacheNode = includeNode;

	while (cacheNode != NULL) {
		actualInclude = cacheNode->leftNode;
		cacheNode = cacheNode->rightNode;
	}

	char *name = actualInclude->value;
	struct SemanticEntry *entry = SA_create_semantic_entry(name, nullDec, P_GLOBAL, EXTERNAL, NULL, includeNode->line, includeNode->position);
	
	if ((int)SA_is_obj_already_defined(name, table) == true) {
		struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(name, table, includeNode);
		(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
		return;
	}
	
	(void)HM_add_entry(name, entry, table->symbolTable);
	ExternalEntry *externalEntry = SA_create_external_entry(FILE_NAME, includeNode, DECLARATION_CHECK);
	(void)L_add_item(LIST_OF_EXTERNAL_ACCESSES, externalEntry);
}

/**
 * <p>
 * Evaluates a try statement for correctness.
 * </p>
 * 
 * @param *table        The parent table
 * @param *tryNode      The node with the try runnable
 * @param *parentNode   Node abover the catchNode (evaluates if the statement is before a "catch" statement)
 * @param index         Index at the parent node (evaluates if the statement is before a "catch" statement)
 */
void SA_add_try_statement(SemanticTable *table, Node *tryNode, Node *parentNode, int index) {
	if (table->type == ENUM) {
		char *msg = "Try statements are not allowed in enums.";
		char *exp = "There's no possibility to run something in enums.";
		char *sugg = "Remove the \"try\" from the enum.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, tryNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	struct Node *estimatedCatchNode = parentNode->details[index + 1];

	if (estimatedCatchNode == NULL
		|| estimatedCatchNode->type != _CATCH_NODE_) {
		char *msg = "Try statements have to have a catch statement.";
		char *exp = "Can't try anything, when the error is not caught afterwards.";
		char *sugg = "Maybe add a catch statement after the try statement.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, tryNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
	}

	char *name = SA_get_string("try");
	SemanticTable *tempTable = SA_create_new_scope_table(NULL, TRY, table, NULL, tryNode->line, tryNode->position);
	struct SemanticEntry *tryEntry = SA_create_semantic_entry(name, nullDec, P_GLOBAL, WHILE, tempTable, tryNode->line, tryNode->position);
	(void)HM_add_entry(name, tryEntry, table->symbolTable);
	(void)SA_manage_runnable(tryNode, tempTable);
}

/**
 * <p>
 * Evaluates a catch statement for correctness.
 * </p>
 * 
 * @param *table        The parent table
 * @param *catchNode    The node with the catch runnable and error node
 * @param *parentNode   Node abover the catchNode (evaluates if the statement is after a "try" statement)
 * @param index         Index at the parent node (evaluates if the statement is after a "try" statement)
 */
void SA_add_catch_statement(SemanticTable *table, Node *catchNode, Node *parentNode, int index) {
	if (table->type == ENUM) {
		char *msg = "Catch statements are not allowed in enums.";
		char *exp = "There's no possibility to run something in enums.";
		char *sugg = "Remove the \"catch\" from the enum.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, catchNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	struct Node *estimatedTryNode = parentNode->details[index - 1];

	if (estimatedTryNode == NULL
		|| estimatedTryNode->type != _TRY_NODE_) {
		char *msg = "Catch statements have to be placed after a try statement.";
		char *exp = "Can't catch anything when nothing is tried before.";
		char *sugg = "Maybe add a try statement before.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, catchNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	char *name = SA_get_string("catch");
	SemanticTable *tempTable = SA_create_new_scope_table(catchNode->rightNode, CATCH, table, NULL, catchNode->line, catchNode->position);
	Node *errorHandleNode = catchNode->leftNode;
	struct VarDec errorType = {CLASS_REF, 0, errorHandleNode->leftNode->value, true};
	struct SemanticEntry *param = SA_create_semantic_entry(errorHandleNode->value, errorType, P_GLOBAL, VARIABLE, NULL, errorHandleNode->line, errorHandleNode->position);
	(void)L_add_item(tempTable->paramList, param);
	struct SemanticEntry *catchEntry = SA_create_semantic_entry(name, nullDec, P_GLOBAL, WHILE, tempTable, catchNode->line, catchNode->position);
	(void)HM_add_entry(name, catchEntry, table->symbolTable);
	(void)SA_manage_runnable(catchNode->rightNode, tempTable);
}

void SA_add_while_or_do_to_table(SemanticTable *table, Node *whileDoNode) {
	if (table->type == ENUM) {
		char *msg = "While and Do statements are not allowed in enums.";
		char *exp = "There's no possibility to run something in enums.";
		char *sugg = "Remove the \"do\" from the enum.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, whileDoNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	struct SemanticReport conditionRep = SA_evaluate_chained_condition(table, whileDoNode->leftNode);

	if (conditionRep.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(conditionRep);
		return;
	}

	char *name = whileDoNode->type == _WHILE_STMT_NODE_ ? SA_get_string("while") : SA_get_string("do");
	enum ScopeType type = whileDoNode->type == _WHILE_STMT_NODE_ ? WHILE : DO;
	SemanticTable *whileTable = SA_create_new_scope_table(whileDoNode->rightNode, type, table, NULL, whileDoNode->line, whileDoNode->position);
	struct SemanticEntry *whileEntry = SA_create_semantic_entry(name, nullDec, P_GLOBAL, type, whileTable, whileDoNode->line, whileDoNode->position);
	(void)HM_add_entry(name, whileEntry, table->symbolTable);
	(void)SA_manage_runnable(whileDoNode->rightNode, whileTable);
}

void SA_add_if_to_table(SemanticTable *table, Node *ifNode) {
	if (table->type == ENUM) {
		char *msg = "If statements are not allowed in enums.";
		char *exp = "There's no possibility to check something in enums.";
		char *sugg = "Remove the \"if\" from the enum.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, ifNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
	}

	struct SemanticReport conditionRep = SA_evaluate_chained_condition(table, ifNode->leftNode);

	if (conditionRep.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(conditionRep);
		return;
	}

	char *name = SA_get_string("if");
	SemanticTable *whileTable = SA_create_new_scope_table(ifNode->rightNode, IF, table, NULL, ifNode->line, ifNode->position);
	struct SemanticEntry *ifEntry = SA_create_semantic_entry(name, nullDec, P_GLOBAL, IF, whileTable, ifNode->line, ifNode->position);
	(void)HM_add_entry(name, ifEntry, table->symbolTable);
	(void)SA_manage_runnable(ifNode->rightNode, whileTable);
}

void SA_add_else_if_to_table(SemanticTable *table, Node *elseIfNode, Node *parentNode, int index) {
	struct Node *estimatedIfNode = parentNode->details[index - 1];

	if (estimatedIfNode->type != _IF_STMT_NODE_
		&& estimatedIfNode->type != _ELSE_IF_STMT_NODE_) {
		char *msg = "Else-if statements are only allowed after an if and else-if statement.";
		char *exp = "Else-if statements have to be placed after wither an if or else-if statement, since only then conditions can be checked.";
		char *sugg = "Maybe remove the else-if or move it under an if/else-if statement.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, elseIfNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	struct SemanticReport conditionRep = SA_evaluate_chained_condition(table, elseIfNode->leftNode);

	if (conditionRep.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(conditionRep);
		return;
	}

	char *name = SA_get_string("else_if");
	SemanticTable *whileTable = SA_create_new_scope_table(elseIfNode->rightNode, ELSE_IF, table, NULL, elseIfNode->line, elseIfNode->position);
	struct SemanticEntry *elseIfEntry = SA_create_semantic_entry(name, nullDec, P_GLOBAL, ELSE_IF, whileTable, elseIfNode->line, elseIfNode->position);
	(void)HM_add_entry(name, elseIfEntry, table->symbolTable);
	(void)SA_manage_runnable(elseIfNode->rightNode, whileTable);
}

void SA_add_else_to_table(SemanticTable *table, Node *elseNode, Node *parentNode, int index) {
	struct Node *estimatedIfOrElseIfNode = parentNode->details[index - 1];

	if (estimatedIfOrElseIfNode->type != _IF_STMT_NODE_
		&& estimatedIfOrElseIfNode->type != _ELSE_IF_STMT_NODE_) {
		char *msg = "Else statements are only allowed after an if and else-if statement.";
		char *exp = "Else statements can only be placed after an if or else-if statement to execute a non catched condition.";
		char *sugg = "Maybe remove the else-statement or place an if/else-if statement before.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, elseNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	char *name = SA_get_string("else");
	SemanticTable *whileTable = SA_create_new_scope_table(elseNode->rightNode, ELSE, table, NULL, elseNode->line, elseNode->position);
	struct SemanticEntry *elseEntry = SA_create_semantic_entry(name, nullDec, P_GLOBAL, ELSE, whileTable, elseNode->line, elseNode->position);
	(void)HM_add_entry(name, elseEntry, table->symbolTable);
	(void)SA_manage_runnable(elseNode->rightNode, whileTable);
}

void SA_check_break_or_continue_to_table(SemanticTable *table, Node *breakOrContinueNode) {
	if ((int)SA_is_break_or_continue_placement_valid(table) == false) {
		char *msg = NULL;
		char *exp = NULL;
		char *sugg;

		if (breakOrContinueNode->type == _BREAK_STMT_NODE_) {
			msg = "Breaks are only allowed within a loop scope.\0";
			exp = "Breaks have to be placed in a loop scope, since for instance a function or class can't be \"breaked\".\0";
			sugg = "Remove the \"break\".\0";
		} else {
			msg = "Continues are only allowed within a loop scope.\0";
			exp = "Continues have to be placed in a loop scope, since for instance a function or class can't be \"continued\".\0";
			sugg = "Remove the \"continue\".\0";
		}
		
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, breakOrContinueNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
	}
}

void SA_add_return_to_table(SemanticTable *table, Node *returnNode) {
	SemanticTable *potentialFunctionTable = SA_get_next_table_of_type(table, FUNCTION);
	struct SemanticReport rep = nullRep;

	if (potentialFunctionTable->type == FUNCTION) {
		rep = SA_handle_return_in_function(potentialFunctionTable, table, returnNode);
	} else {
		potentialFunctionTable = SA_get_next_table_of_type(table, CONSTRUCTOR);

		if (potentialFunctionTable->type != CONSTRUCTOR) {
			char *msg = "Returns statements are only allowed within a scope of a function or constructor.";
			char *exp = "If a return statement is not within a function scope, nothing can be returned as a \"result\".";
			char *sugg = "Maybe wrap the \"return\" into a function or remove the \"return\" statement.";
			struct ErrorContainer errCont = {msg, exp, sugg};
			struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, returnNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
			(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		}

		rep = SA_handle_return_in_constructor(returnNode);
	}

	if (rep.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(rep);
		return;
	}

	char *name = SA_get_string("return");
	(void)HM_add_entry(name, NULL, table->symbolTable);
}

void SA_add_for_to_table(SemanticTable *table, Node *forNode) {
	if (table->type == CLASS || table->type == ENUM) {
		char *msg = "For loops are not allowed in classes or enums.";
		char *exp = "There is no possible way for a function to execute the loop.";
		char *sugg = "Maybe move the \"for\" into a function, constructor or remove the \"for\".";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, forNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	char *name = SA_get_string("for");
	SemanticTable *forTable = SA_create_new_scope_table(forNode->rightNode, FOR, table, NULL, forNode->line, forNode->position);
	struct SemanticEntry *forEntry = SA_create_semantic_entry(name, nullDec, P_GLOBAL, FOR, forTable, forNode->line, forNode->position);
	(void)HM_add_entry(name, forEntry, table->symbolTable);

	(void)SA_add_normal_variable_to_table(forTable, forNode->leftNode);
	struct SemanticReport conditionRep = SA_evaluate_chained_condition(forTable, forNode->details[0]);

	if (conditionRep.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(conditionRep);
	}
	
	(void)SA_check_assignments(forTable, forNode->details[1]);
	(void)SA_manage_runnable(forNode->rightNode, forTable);
}

void SA_add_check_to_table(SemanticTable *table, Node *checkNode) {
	if (table->type == CLASS || table->type == ENUM) {
		char *msg = "Check statements are not allowed in classes or enums.";
		char *exp = "There is no possible way for a function to execute the check statement.";
		char *sugg = "Maybe move the \"check\" into a function, constructor or remove the \"check\".";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, checkNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		return;
	}

	Node *checkableNode = checkNode->leftNode;
	struct SemanticReport memberAccessReport = SA_evaluate_member_access(checkableNode, table);

	if (memberAccessReport.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(memberAccessReport);
		return;
	}

	if ((int)SA_validate_checkable(memberAccessReport) == false) {
		(void)SA_create_checkable_error_message(memberAccessReport, checkableNode);
		return;
	}

	char *name = SA_get_string("check");
	SemanticTable *checkTable = SA_create_new_scope_table(checkNode, CHECK, table, NULL, checkNode->line, checkNode->position);
	struct SemanticEntry *checkEntry = SA_create_semantic_entry(name, nullDec, P_GLOBAL, CHECK, checkTable, checkNode->line, checkNode->position);
	(void)HM_add_entry(name, checkEntry, table->symbolTable);
	(void)SA_handle_check_statement_runnable(checkNode->rightNode, checkTable);
}

struct SemanticReport SA_handle_return_in_constructor(Node *returnNode) {
	if (returnNode->leftNode != NULL) {
		char *msg = "A constructor returns an instance of the object itself and thus can't return something else.";
		char *exp = "Returning something from a constructor cannot be handled, since the constructor returns an instance of the object itself.";
		char *sugg = "Maybe remove the rVal.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, returnNode, WRONG_RVAL_EXCEPTION, errCont);
		return rep;
	}

	return nullRep;
}

struct SemanticReport SA_handle_return_in_function(SemanticTable *functionTable, SemanticTable *table, Node *returnNode) {
	struct HashMapEntry *hashFunctionEntry = (struct HashMapEntry*)HM_get_entry(functionTable->name, functionTable->parent->symbolTable);
	struct SemanticEntry *functionEntry = (struct SemanticEntry*)(hashFunctionEntry->value);
	struct VarDec awaitedType = functionEntry->dec;
	struct SemanticReport rep = nullRep;

	if (awaitedType.type == VOID) {
		if (returnNode->leftNode != NULL) {
			char *msg = "A function returning \"void\" cannot return something.";
			char *exp = "Returning something in a \"void\" returning function is prohibited.";
			char *sugg = "Maybe modify the rVal.";
			struct ErrorContainer errCont = {msg, exp, sugg};
			struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, returnNode, WRONG_RVAL_EXCEPTION, errCont);
			return rep;
		}

		return nullRep;
	} else if (returnNode->leftNode == NULL) {
		char *msg = "A function returning \"non-void\" cannot return nothing.";
		char *exp = "Returning \"void\" in a \"non-void\" returning function is prohibited.";
		char *sugg = "Maybe modify the rVal.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, returnNode, WRONG_RVAL_EXCEPTION, errCont);
		return rep;
	}

	if (returnNode->leftNode->type == _INHERITED_CLASS_NODE_) {
		rep = SA_evaluate_instance_creation(table, returnNode->leftNode);
	} else if (returnNode->leftNode->type == _ARRAY_ASSIGNMENT_NODE_) {
		rep = SA_evaluate_array_assignment(awaitedType, returnNode->leftNode, table);

		//If a array is returned, but a non-array was awaited, a NO_SUCH_ARRAY_DIM... is thrown
		if (rep.status == ERROR && rep.errorType == NO_SUCH_ARRAY_DIMENSION_EXCEPTION) {
			int foundDim = (int)SA_count_total_array_dimensions(returnNode->leftNode);
			struct VarDec gotType = {awaitedType.type, foundDim, NULL, false};
			rep = SA_create_expected_got_report(awaitedType, gotType, rep.errorNode);
		}
	} else if (returnNode->leftNode->type == _CONDITIONAL_ASSIGNMENT_NODE_) {
		rep = SA_evaluate_conditional_assignment(awaitedType, returnNode->leftNode, table);
	} else {
		rep = SA_evaluate_simple_term(awaitedType, returnNode->leftNode, table);
	}

	if ((int)SA_are_VarTypes_equal(awaitedType, rep.dec, false) == false) {
		return SA_create_expected_got_report(awaitedType, rep.dec, returnNode);
	}

	return rep;
}

/**
 * <p>
 * Searches for all function with the same name and then checks for equality by
 * checking the parameters, if all types match up, the function was defined prior
 * and thus an error is thrown.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - If the function is already defined
 * <li>false - If the function is not defined
 * </ul>
 * 
 * @param *params			Parameters of the function node
 * @param *table			Table in which the function is defined in
 * @param *functionNode		Function node to check for duplication
 */
int SA_is_function_already_defined(struct ParamTransferObject *params, SemanticTable *table, Node *functionNode) {
	SemanticTable *tableToCheck = SA_get_next_table_of_type(table, CLASS);

	do {
		struct HashMapEntry *entryOfFunctions = (struct HashMapEntry*)HM_get_entry(functionNode->value, tableToCheck->symbolTable);
		struct HashMapEntry *tempEntry = entryOfFunctions;

		while (tempEntry != NULL) {
			SemanticEntry *functionEntry = (SemanticEntry*)tempEntry->value;
			SemanticTable *functionTable = (SemanticTable*)functionEntry->reference;
			int equalityCounter = 0;

			//Filter all entries, that are not functions and do not have the same parameter count
			if (functionEntry->internalType != FUNCTION) {
				return true;
			} else if (params->params != functionTable->paramList->load) {
				tempEntry = tempEntry->linkedEntry;
				continue;
			}

			for (int i = 0, c = 0; i < functionNode->detailsCount; i++) {
				Node *currentParamNode = functionNode->details[i];

				if (currentParamNode == NULL || currentParamNode->type == _RUNNABLE_NODE_) {
					continue;
				}

				SemanticEntry *paramToCompare = (SemanticEntry*)L_get_item(functionTable->paramList, c++);
				struct VarDec currentParamType = SA_get_VarType(currentParamNode->details[0], false, table);
				struct VarDec paramToCompareType = paramToCompare->dec;
				equalityCounter += (int)SA_are_VarTypes_equal(currentParamType, paramToCompareType, true);
			}

			if (equalityCounter == params->params) {
				return true;
			}

			tempEntry = tempEntry->linkedEntry;
		}

		tableToCheck = SA_get_next_table_of_type(tableToCheck, MAIN);
	} while (tableToCheck->type != MAIN);

	return false;
}

struct VarDec validCheckables[] = {
	{INTEGER, 0, NULL, true}, {INTEGER, 0, NULL, false},
	{LONG, 0, NULL, true}, {LONG, 0, NULL, false},
	{SHORT, 0, NULL, true}, {SHORT, 0, NULL, false},
	{CHAR, 0, NULL, true}, {CHAR, 0, NULL, false},
	{FLOAT, 0, NULL, true}, {FLOAT, 0, NULL, false},
	{DOUBLE, 0, NULL, true}, {DOUBLE, 0, NULL, false},
	{BOOLEAN, 0, NULL, true}, {BOOLEAN, 0, NULL, false}
};

void SA_handle_check_statement_runnable(Node *runnableNode, SemanticTable *checkTable) {
	for (int i = 0; i < runnableNode->detailsCount; i++) {
		Node *detailNode = runnableNode->details[i];

		if (detailNode == NULL) {
			continue;
		} else if (detailNode->type != _IS_STMT_NODE_) {
			char *msg = "Only the \"is\" statement is allowed within the scope of a \"check\" statement.";
			char *exp = "A check statement has to have different comparables, which can only be achieved through \"is\" statements.";
			char *sugg = "Maybe change the current statement to an \"is\" statement or remove the current statement.";
			struct ErrorContainer errCont = {msg, exp, sugg};
			struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, detailNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
			(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
			return;
		} else if ((int)HM_contains_key(detailNode->leftNode->value, checkTable->symbolTable) == true) {
			struct SemanticReport alreadyDefRep = SA_create_already_defined_exception_report(detailNode->leftNode->value, checkTable, detailNode->leftNode);
			(void)THROW_ALREADY_DEFINED_EXCEPTION(alreadyDefRep);
			return;
			continue;
		}

		SemanticTable *isTable = SA_create_new_scope_table(detailNode, IS, checkTable, NULL, detailNode->line, detailNode->position);
		struct SemanticEntry *isEntry = SA_create_semantic_entry(detailNode->leftNode->value, nullDec, P_GLOBAL, IS, isTable, detailNode->line, detailNode->position);
		(void)HM_add_entry(detailNode->leftNode->value, isEntry, checkTable->symbolTable);
		(void)SA_manage_runnable(detailNode->rightNode, isTable);
	}
}

void SA_create_checkable_error_message(struct SemanticReport memberAccessReport, Node *checkableNode) {
	char *type_str = SA_get_VarType_string(memberAccessReport.dec);
	size_t length = (size_t)strlen(type_str);
	char *sugg = (char*)calloc(length + 55 + 1, sizeof(char));

	if (sugg == NULL) {
		(void)THROW_MEMORY_RESERVATION_EXCEPTION("CHECKABLE_ERR_CREATION");
		struct SemanticReport panicRep = SA_create_semantic_report(nullDec, ERROR, checkableNode, TYPE_MISMATCH_EXCEPTION, nullCont);
		(void)THROW_ASSIGNED_EXCEPTION(panicRep);
		return;
	}

	(void)snprintf(sugg, (length + 55 + 1) * sizeof(char), "Maybe convert the \"%s\" to a number, boolean or character.", type_str);
	char *msg = "Check statements can only compare numbers, booleans and characters.";
	struct ErrorContainer errCont = {msg, NULL, sugg};
	struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, checkableNode, NON_COMPARABLE_CHECK_EXCEPTION, errCont);
	(void)free(type_str);
	(void)THROW_ASSIGNED_EXCEPTION(rep);
}

int SA_validate_checkable(struct SemanticReport memberAccessReport) {
	int checkablesLen = sizeof(validCheckables) / sizeof(validCheckables[0]);

	for (int i = 0; i < checkablesLen; i++) {
		if ((int)SA_are_VarTypes_equal(validCheckables[i], memberAccessReport.dec, true) == true) {
			return true;
		}
	}

	return false;
}

void SA_check_assignments(SemanticTable *table, Node *node) {
	if (table->type == ENUM || table->type == CLASS) {
		char *msg = "Assignments are only allowed in callables.";
		char *exp = "Is an assignment is not in a callable, it can't be invoked.";
		char *sugg = "Maybe move the assignment into a callable like a function, constructor or MAIN.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
	}

	if (node->type == _SIMPLE_INC_DEC_ASS_NODE_) {
		struct SemanticReport incDecRep = SA_validate_increment_and_decrement(node, table);

		if (incDecRep.status == ERROR) {
			(void)THROW_ASSIGNED_EXCEPTION(incDecRep);
		}

		return;
	}

	Node *lValNode = node->leftNode;
	Node *rValNode = node->rightNode;
	struct SemanticReport lValReport = SA_evaluate_member_access(lValNode, table);
	struct VarDec awaitedDec = lValReport.dec;

	if (lValReport.status == ERROR) {
		if ((int)SA_is_node_arithmetic_operator(lValNode) == true) {
			char *msg = "Arithmetic operations on the left hand side are not allowed.";
			char *exp = "Can't assign a term to a specified value.";
			char *sugg = "Maybe remove the arithmetic operator or change the lVal to \"<IDENTIFIER>\" or \"<MEMBER_ACCESS>\".";
			struct ErrorContainer errCont = {msg, exp, sugg};
			struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, lValNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
			(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
			return;
		} else {
			(void)THROW_ASSIGNED_EXCEPTION(lValReport);
			return;
		}
	} else if (awaitedDec.constant == true) {
		char *desc = "Can't modify a constant value.";
		(void)THROW_WRONG_LVAL_EXCEPTION(lValNode, desc);
		return;
	}
	
	struct SemanticReport rValReport = SA_evaluate_simple_term(awaitedDec, rValNode, table);
	struct VarDec gotDec = rValReport.dec;

	if (rValReport.status == ERROR) {
		(void)THROW_ASSIGNED_EXCEPTION(rValReport);
		return;
	} else if ((int)SA_are_VarTypes_equal(awaitedDec, gotDec, false) == false) {
		struct SemanticReport expGotRep =  SA_create_expected_got_report(awaitedDec, gotDec, rValNode);
		(void)THROW_TYPE_MISMATCH_EXCEPTION(expGotRep);
		return;
	}
}

struct SemanticReport SA_create_already_defined_exception_report(char *collissionName, SemanticTable *currentTable, Node *node) {
	SemanticTable *nextTableWithDec = SA_get_next_table_with_declaration(collissionName, currentTable);
	struct SemanticEntryReport entry = SA_get_entry_if_available(collissionName, nextTableWithDec);

	if (entry.entry == NULL) {
		return nullRep;
	}

	char *targetString = (char*)SA_convert_ScopeType_to_string(entry.entry->internalType);
	size_t length = (size_t)strlen(targetString);
	
	char *msg = (char*)calloc(length + 128 + 1, sizeof(char));
	(void)snprintf(msg, length + 128 + 1, "The identifier \"%s\" was defined previously as \"%s\" on line "
					TEXT_COLOR_BLUE TEXT_UNDERLINE "%li:%li"
					TEXT_COLOR_RESET TEXT_COLOR_RED ".", collissionName, targetString, entry.entry->line + 1, entry.entry->position);
	char *exp = "Can't have two identical identifiers below each other, since it is unclear which to choose.";
	char *sugg = "Maybe rename the identifier.";
	struct ErrorContainer errCont = {msg, exp, sugg};
	return SA_create_semantic_report(nullDec, ERROR, node, ALREADY_DEFINED_EXCEPTION, errCont);
}

/**
 * <p>
 * Returns the according string from the given ScopeType.
 * </p>
 * 
 * <p>
 * On error it'll return "undefinied".
 * </p>
 * 
 * @returns The converted string
 * 
 * @param type	ScopeType to convert
 */
char *SA_convert_ScopeType_to_string(enum ScopeType type) {
	switch (type) {
	case VARIABLE:
		return "variable";
	case FUNCTION:
		return "function";
	case CLASS:
		return "class";
	case ENUM:
		return "enum";
	default:
		return "undefined";
	}
}

/**
 * <p>
 * Counts the dimension that were set by the programmer.
 * </p>
 * 
 * @returns The number of set dimensions
 * 
 * @param *arrayVar		Variable with the array
 */
int SA_count_set_array_var_dimensions(Node *arrayVar) {
	int dims = 0;

	for (int i = 0; i < arrayVar->detailsCount; i++) {
		Node *detailNode = arrayVar->details[i];

		if (detailNode == NULL) {
			continue;
		}

		dims += detailNode->type == _ARRAY_DIM_NODE_ ? 1 : 0;
	}

	return dims;
}

/**
 * <p>
 * Counts the dimensions of an array assignment.
 * </p>
 * 
 * @returns The number of set dimensions
 * 
 * @param *arrayVar		Variable with the array
 */
int SA_count_total_array_dimensions(Node *arrayNode) {
	int dims = 1;

	for (int i = 0; i < arrayNode->detailsCount; i++) {
		Node *curNode = arrayNode->details[i];

		if (curNode == NULL) {
			continue;
		} else if (curNode->type == _ARRAY_ASSIGNMENT_NODE_) {
			dims += SA_count_total_array_dimensions(curNode);
		};
	}

	return dims;
}

/**
 * <p>
 * Checks the placement of the break or continue statement.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - If the statement is placed correctly
 * <li>false - If the statement is place incorrectly
 * </ul>
 * 
 * @param *table    The innerscope from which to work upwards, till a loop is detected
 */
int SA_is_break_or_continue_placement_valid(SemanticTable *table) {
	SemanticTable *temp = table;
	int metLoop = false;

	while (temp != NULL) {
		switch (temp->type) {
		case FOR: case WHILE: case DO:
		case IS:
			metLoop = true;
			break;
		case IF: case ELSE_IF: case ELSE:
		case TRY: case CATCH:
			break;
		default:
			temp = NULL;
			break;
		}

		temp = temp == NULL ? NULL : temp->parent;
	}

	return metLoop;
}

/**
 * <p>
 * Evaluates a chained condition for correctness.
 * </p>
 * 
 * <p>
 * It goes down the chained condition tree recursively by checkinig for the 'and' and 'or'
 * keyword. If thse are found another recursion happens, else the resulting terms are
 * checked. There's no need to check the rational operator, due to syntax analysis.
 * </p>
 * 
 * @returns A SeamnticReport with possible errors.
 * 
 * @param *table        Table in which the condition is called from
 * @param *rootNode     The root of the condition tree
 */
struct SemanticReport SA_evaluate_chained_condition(SemanticTable *table, Node *rootNode) {
	if (rootNode->type == _OR_NODE_
		|| rootNode->type == _AND_NODE_) {
		struct SemanticReport leftCond = SA_evaluate_chained_condition(table, rootNode->leftNode);
		struct SemanticReport rightCond = SA_evaluate_chained_condition(table, rootNode->rightNode);

		if (leftCond.status == ERROR) {
			return leftCond;
		} else if (rightCond.status == ERROR) {
			return rightCond;
		}
	} else if (rootNode->type == _FUNCTION_CALL_NODE_
		|| rootNode->type == _MEM_CLASS_ACC_NODE_) {
		struct SemanticReport memAccessRep = SA_evaluate_member_access(rootNode, table);

		if (memAccessRep.status == ERROR) {
			return memAccessRep;
		} else if (memAccessRep.dec.type == CLASS_REF) {
			char *msg = "Cannot check against non-boolean type.";
			char *exp = "It is not possible to check an non-boolean since there is no way of evaluating the correctness.";
			char *sugg = "Maybe add a rtionl operator like \"==\" or \"<=\" ... to the condition.";
			struct ErrorContainer errCont = {msg, exp, sugg};
			return SA_create_semantic_report(nullDec, ERROR, rootNode, NON_BOOLEAN_CHECK_EXCEPTION, errCont);
		}
	} else {
		struct VarDec cust = {CUSTOM, 0, NULL};
		struct SemanticReport lVal = SA_evaluate_simple_term(cust, rootNode->leftNode, table);
		struct SemanticReport rVal = SA_evaluate_simple_term(cust, rootNode->rightNode, table);

		if (lVal.status == ERROR) {
			return lVal;
		} else if (rVal.status == ERROR) {
			return rVal;
		}
	}

	return nullRep;
}

struct SemanticReport SA_evaluate_instance_creation(SemanticTable *table, Node *node) {
	struct VarDec dec = nullDec;

	if (node->type == _INHERITED_CLASS_NODE_) {
		SemanticTable *topTable = SA_get_next_table_of_type(table, MAIN);
		struct SemanticEntryReport classEntry = SA_get_entry_if_available(node->value, topTable);

		if (classEntry.entry == NULL) {
			return SA_create_semantic_report(nullDec, ERROR, node, NOT_DEFINED_EXCEPTION, nullCont);
		} else if (classEntry.entry->internalType == EXTERNAL) {
			ExternalEntry *externalEntry = SA_create_external_entry(FILE_NAME, node, CORRECTNESS_CHECK);
			(void)L_add_item(LIST_OF_EXTERNAL_ACCESSES, externalEntry);
		}
		
		dec.type = CLASS_REF;
		dec.typeName = classEntry.entry->name;
		
		if (node->detailsCount == 0) { //Empty constructor
			return SA_create_semantic_report(dec, SUCCESS, NULL, NONE, nullCont);
		}

		SemanticTable *classTable = (SemanticTable*)classEntry.entry->reference;
		struct SemanticReport containsConstructor = SA_contains_constructor_of_type(classTable, node, CONSTRUCTOR_CHECK_CALL);

		if (containsConstructor.status == NA) {
			return SA_create_semantic_report(nullDec, ERROR, node, CONSTRUCTOR_NOT_DEFINED_EXCEPTION, nullCont);
		}
	} else {
		char *msg = "Inherited var assignment is not a inheritance of a class.";
		char *exp = "A var, that awaits an inherited class, cannot be assigned to a non-class statement";
		char *sugg = "Maybe remove the instance creation or inherit from an existing class.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		return SA_create_semantic_report(nullDec, ERROR, node, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
	}

	return SA_create_semantic_report(dec, SUCCESS, NULL, NONE, nullCont);
}

char *SA_get_string(char bufferString[]) {
	int length = (int)strlen(bufferString);
	char *string = (char*)calloc(length + 1, sizeof(char));

	if (string == NULL) {
		(void)THROW_MEMORY_RESERVATION_EXCEPTION("toString");
		return NULL;
	}

	for (int i = 0; i < length; i++) {
		string[i] = bufferString[i];
	}

	string[length] = '\0';
	return string;
}

/**
 * <p>
 * Checks if a constructor with the exact same types is already defined or not.
 * </p>
 * 
 * <p>
 * The types have to be different for the constructor to be recognized as "different".
 * 
 * Example:
 * 
 * ```
 * this::constructor(param1, param2) {}
 * this::constructor(number1, number2) {}
 * => ERROR, because params are of equal types
 * 
 * this::constructor(param1:int, param2:char) {}
 * this::constructor(param1:int, param2:double) {}
 * => ALLOWED, due to different types
 * ```
 * </p>
 * 
 * @returns A SemanticReport with possible errors and if the class contains another constructor
 * of the same type or not.
 * 
 * @param *classTable       The table in the current class with ass other constructors
 * @param *paramHolder      The new constructor node, that holds the params
 * @param fncctype          Determines the "strictness", for declarations it is stricter than checks
 */
struct SemanticReport SA_contains_constructor_of_type(SemanticTable *classTable, struct Node *paramHolder, enum FunctionCallType fncctype) {
	if (classTable == NULL || paramHolder == NULL) {
		return SA_create_semantic_report(nullDec, NA, NULL, NONE, nullCont);
	}

	int actualNodeParamCount = (int)SA_get_node_param_count(paramHolder);

	for (int i = 0; i < classTable->paramList->load; i++) {
		SemanticEntry *entry = (SemanticEntry*)L_get_item(classTable->paramList, i);

		if (entry == NULL) {
			continue;
		} else if (entry->dec.type != CONSTRUCTOR_PARAM) {
			continue;
		}
		
		SemanticTable *entryTable = (SemanticTable*)entry->reference;

		if (entryTable == NULL) {
			continue;
		} else if (entryTable->paramList->load != actualNodeParamCount) {
			continue;
		}
		
		struct SemanticReport fncCallRep = SA_evaluate_function_call(paramHolder, entry, classTable, fncctype);
		
		//Checks if another constructor is already defined with the same parameter types,
		//on error there is, else not
		if (fncCallRep.status == ERROR) {
			continue;
		} else {
			fncCallRep.status = SUCCESS;
			return fncCallRep;
		}
	}

	return SA_create_semantic_report(nullDec, NA, NULL, NONE, nullCont);
}

/**
 * <p>
 * Gets the parameter count of the provided node.
 * RUNNABLES and NULL nodes are excluded in the count.
 * </p>
 * 
 * @returns The number of parameters
 * 
 * @param *paramHolder  The node that holds the parameters
 */
int SA_get_node_param_count(struct Node *paramHolder) {
	int actualNodeParamCount = 0;

	for (int i = 0; i < paramHolder->detailsCount; i++) {
		if (paramHolder->details[i] == NULL) {
			continue;
		} if (paramHolder->details[i]->type == _RUNNABLE_NODE_) {
			continue;
		}

		actualNodeParamCount++;
	}

	return actualNodeParamCount;
}

/**
 * <p>
 * This function evaluates a simple term with the help of recursion.
 * </p>
 * 
 * <p>
 * First the topNode is checked for an arithmetic operator, if it is
 * not an arithmetic operator the one node is evaluated. If it is an
 * arithmetic operator the simple_term function is invoked again, until
 * the top node is not an arithmetic operator anymore.
 * </p>
 * 
 * <p><strong>Note:</strong>
 * This function also evaluates the optional typesafety!
 * </p>
 * 
 * @returns A report with possible errors
 * 
 * @param expectedType      Type to check for typesafety
 * @param *topNode          Current top node to check
 * @param *table            Current scope table
 */
struct SemanticReport SA_evaluate_simple_term(struct VarDec expectedType, Node *topNode, SemanticTable *table) {
	int isTopNodeArithmeticOperator = (int)SA_is_node_arithmetic_operator(topNode);
	int isTopNodeBitOperator = (int)SA_is_node_bit_operator(topNode);
	
	if (isTopNodeArithmeticOperator == true || isTopNodeBitOperator == true) {
		struct SemanticReport leftTerm = SA_evaluate_simple_term(expectedType, topNode->leftNode, table);
		struct SemanticReport rightTerm = SA_evaluate_simple_term(expectedType, topNode->rightNode, table);
		
		if (leftTerm.status == ERROR) {
			return leftTerm;
		} else if (rightTerm.status == ERROR) {
			return rightTerm;
		}

		if (isTopNodeBitOperator == true) {
			if (leftTerm.dec.type != INTEGER && leftTerm.dec.type != LONG
				&& leftTerm.dec.type != SHORT && leftTerm.dec.type != CHAR) {
				struct VarDec actualExpected = {INTEGER, 0, NULL, false};
				return SA_create_expected_got_report(actualExpected, leftTerm.dec, topNode->leftNode);
			} else if (rightTerm.dec.type != INTEGER && rightTerm.dec.type != LONG
				&& rightTerm.dec.type != SHORT && rightTerm.dec.type != CHAR) {
				struct VarDec actualExpected = {INTEGER, 0, NULL, false};
				return SA_create_expected_got_report(actualExpected, rightTerm.dec, topNode->rightNode);
			}
		}

		struct SemanticReport validationReport = SA_is_term_valid(leftTerm.dec, rightTerm.dec, topNode, topNode->rightNode, topNode->leftNode);
		struct SemanticReport successReport = SA_create_semantic_report(expectedType, SUCCESS, NULL, NONE, nullCont);
		return validationReport.status == ERROR ? validationReport : successReport;
	} else {
		return SA_evaluate_term_side(expectedType, topNode, table);
	}
}

/**
 * <p>
 * Checks if a term operation is valid, considering various arithmetic and type errors.
 * </p>
 * 
 * @returns A semantic report with possible errors
 * 
 * @param type1			Type of the left operand
 * @param type2			Type of the right operand
 * @param operatorNode	Operator node
 * @param rightNode		Right operand node
 * @param leftNode		Left operand node
 */
struct SemanticReport SA_is_term_valid(struct VarDec type1, struct VarDec type2, Node *operatorNode, Node *rightNode, Node *leftNode) {
	int isOperatorPlus = (int)strcmp(operatorNode->value, "+");
	
	if (type1.dimension > 1 || type2.dimension > 1) {
		if (isOperatorPlus == 0) {
			char *msg = "Cannot concatenate arrays with multiple dimensions.";
			char *exp = "Handling multidimensional concatenation might lead to misassumptions and thus it is prohibited.";
			char *sugg = "Maybe access the array down to the first dimension and then add.";
			struct ErrorContainer errCont = {msg, exp, sugg};
			return SA_create_semantic_report(nullDec, ERROR, operatorNode, ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION, errCont);
		}
	}
	
	if (type1.dimension > 0 || type2.dimension > 0) {
		if (isOperatorPlus != 0) {
			char *msg = "Cannot perform any arithmetic operation on arrays.";
			char *exp = "Can't perform an arithmetic operation on an array, can't subtract an array from another for example.";
			char *sugg = "Maybe access the array down to the first dimension and then perform the operation.";
			struct ErrorContainer errCont = {msg, exp, sugg};
			return SA_create_semantic_report(nullDec, ERROR, operatorNode, ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION, errCont);
		}
	}
	
	if (type1.type == CLASS_REF || type2.type == CLASS_REF) {
		char *msg = "Unable to perform an arithmetic operation on a class.";
		char *exp = "It is not possible to perform an arithmetic operation on a class.";
		char *sugg = "Maybe change the lVal and rVal to a value and not a class.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		return SA_create_semantic_report(nullDec, ERROR, operatorNode, ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION, errCont);
	}

	if (operatorNode->type == _DIVIDE_NODE_ && (int)strcmp(rightNode->value, "0") == 0) {
		char *msg = "Can't divide by 0.";
		char *exp = "Dividing by 0 is undefined.";
		char *sugg = "Maybe change 0 division to a division, where the divisor is not \"0\".";
		struct ErrorContainer errCont = {msg, exp, sugg};
		return SA_create_semantic_report(nullDec, ERROR, operatorNode, ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION, errCont);
	}

	if (leftNode->type == _NULL_NODE_ || rightNode->type == _NULL_NODE_) {
		char *msg = "Can't use \"null\" to perform any arithmetic operations.";
		char *exp = "It is not possible to calculate using \"null\", since \"null\" is effectively nothing.";
		char *sugg = "Maybe remove the \"null\" out of the equation.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		return SA_create_semantic_report(nullDec, ERROR, operatorNode, ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION, errCont);
	}

	return nullRep;
}

/**
 * <p>
 * The function checks if a term side makes sense.
 * </p>
 * 
 * <p><strong>Allowed Objects:</strong>
 * <ul>
 * <li>Number
 * <li>Member access
 * <li>Class access
 * <li>Identifier
 * <li>Function call
 * </ul>
 * </p>
 * 
 * @returns A report with possible errors and the identified type
 * 
 * @param expectedType      Type to which the term should stick
 * @param *node             Node to check (root of subtree)
 * @param *table            Table in which the term is called
 */
struct SemanticReport SA_evaluate_term_side(struct VarDec expectedType, Node *node, SemanticTable *table) {
	struct VarDec predictedType = {CUSTOM, 0, NULL};
	struct SemanticReport tempRep = nullRep;
	int useReport = false;
	
	switch (node->type) {
	case _NUMBER_NODE_:
	case _FLOAT_NODE_:
		predictedType = SA_convert_identifier_to_VarType(node);
		break;
	case _NULL_NODE_:
		predictedType = nullDec;
		break;
	case _STRING_NODE_:
	case _CHAR_ARRAY_NODE_:
		if ((int)strlen(node->value) > 3) { //3 for 1 letter + 2 quotationmarks
			predictedType.type = STRING;
		} else {
			predictedType.type = CHAR;
		}

		break;
	case _MEM_CLASS_ACC_NODE_:
	case _IDEN_NODE_:
	case _FUNCTION_CALL_NODE_: {
		tempRep = SA_evaluate_member_access(node, table);
		printf("(T): %i, %i, %s, %i\n", tempRep.dec.type, tempRep.dec.dimension, tempRep.dec.typeName == NULL ? "(null)" : tempRep.dec.typeName, tempRep.dec.constant);
		useReport = true;
		node = node->type == _MEM_CLASS_ACC_NODE_ ? SA_get_most_left_node_from_member_access(node) : node;
		break;
	}
	case _BOOL_NODE_:
		predictedType.type = BOOLEAN;
		break;
	case _CONDITIONAL_ASSIGNMENT_NODE_: {
		tempRep = SA_evaluate_conditional_assignment(expectedType, node, table);
		useReport = true;
		break;
	}
	case _SIMPLE_INC_DEC_ASS_NODE_: {
		tempRep = SA_validate_increment_and_decrement(node, table);
		useReport = true;
		break;
	}
	case _NOT_NODE_: {
		tempRep = SA_evaluate_not_operator(node, table);
		useReport = true;
		break;
	}
	default: break;
	}

	if (useReport == true) {
		if (tempRep.status == ERROR) {
			return tempRep;
		}

		predictedType = tempRep.dec;
	}

	printf("EXP: %i | %i | %i | %s\n", expectedType.type, expectedType.dimension, expectedType.constant, expectedType.typeName == NULL ? "null" : expectedType.typeName);
	printf("PRE: %i | %i | %i | %s\n", predictedType.type, predictedType.dimension, predictedType.constant, predictedType.typeName == NULL ? "null" : predictedType.typeName);
	if ((int)SA_are_VarTypes_equal(expectedType, predictedType, false) == false) {
		return SA_create_expected_got_report(expectedType, predictedType, node);
	}

	return SA_create_semantic_report(predictedType, SUCCESS, NULL, NONE, nullCont);
}

Node *SA_get_most_left_node_from_member_access(Node *node) {
	Node *cache = node;

	while (cache->rightNode != NULL) {
		cache = cache->rightNode;
	}

	return cache->leftNode;
}

/**
 * <p>
 * A collection of all possible types that can be
 * inverted using the logical '!' operator.
 * </p>
 */
struct VarDec awaitedNotTypeList[] = {
	{BOOLEAN, 0, NULL, false}, {BOOLEAN, 0, NULL, true},
};

/**
 * <p>
 * Validates the use of unary operators.
 * </p>
 * 
 * @return A report containing the analysis result, like error occured, success and resulting type
 * 
 * @param *topNode      Start node of the unary operation
 * @param *table        Table in which the expression was written in (current scope)
 */
struct SemanticReport SA_evaluate_not_operator(Node *topNode, SemanticTable *table) {
	Node *cache = topNode;
	struct SemanticReport rep = nullRep;
	int length = sizeof(awaitedNotTypeList) / sizeof(awaitedNotTypeList[0]);

	while (cache->rightNode != NULL) {
		if (cache->type != _NOT_NODE_) {
			break;
		}

		cache = cache->rightNode;
	}

	for (int i = 0; i < length; i++) {
		rep = SA_evaluate_simple_term(awaitedNotTypeList[i], cache, table);

		if (rep.status == ERROR) {
			continue;
		}

		break;
	}

	return rep;
}

/**
 * <p>
 * Evaluates a member access as well as a class access.
 * </p>
 * 
 * <p>
 * Due to the structure of the trees the accesses are divided into two groups,
 * the first includes access with '.' or '->', while the other describes
 * itself without any access operator (objects are in local scope).
 * </p>
 * 
 * @return A report containing the analysis result, like error occured, success and resulting type
 * 
 * @param *topNode      Start node of the member/class access tree
 * @param *table        Table in which the expression was written in (current scope)
 */
struct SemanticReport SA_evaluate_member_access(Node *topNode, SemanticTable *table) {
	SemanticTable *topScope = NULL;
	struct SemanticReport rep;

	if (topNode->type == _MEM_CLASS_ACC_NODE_) {
		topScope = SA_get_next_table_with_declaration(topNode->leftNode->value, table);
		printf("TABLE: %s (search: %s)\n", table->name, topNode->leftNode->value);
		rep = SA_check_non_restricted_member_access(topNode, table, topScope);
	} else {
		topScope = SA_get_next_table_with_declaration(topNode->value, table);
		rep = SA_check_restricted_member_access(topNode, table, topScope);
	}

	printf(">>>> >>>> >>>> EXIT! (%i)\n", rep.dec.type);
	return rep.status == ERROR ? rep : SA_create_semantic_report(rep.dec, SUCCESS, NULL, NONE, nullCont);
}

/**
 * <p>
 * Checks a member access tree with multiple accesses.
 * </p>
 * 
 * <p>
 * The function only goes down the tree, actual checking occures
 * here: #SA_check_restricted_member_access(Node *node, SemanticTable *table, SemanticTable *topScope).
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * this->a
 * test()
 * Math->add()
 * List->toList().getItem()
 * ```
 * </p>
 * 
 * @returns A SemanticReport with the analyzed type and flags for error an success
 * 
 * @param *node     Node to check
 * @param *table    The table from the scope, at which the member access occured
 * @param *topScope The current top scope in the process
 */
struct SemanticReport SA_check_non_restricted_member_access(Node *node, SemanticTable *table,
															SemanticTable *topScope) {
	SemanticTable *currentScope = topScope;
	Node *cacheNode = node;
	struct VarDec retType = {CUSTOM, 0, NULL};
	struct SemanticReport potentialThisKeywordReport = SA_evaluate_potential_this_keyword(node, &cacheNode, &currentScope, table, &retType);

	if (potentialThisKeywordReport.status == ERROR) {
		return potentialThisKeywordReport;
	}

	while (cacheNode != NULL) {
		struct SemanticEntryReport entry = SA_get_entry_if_available(cacheNode->leftNode->value, currentScope);
		struct SemanticReport resMemRep = SA_check_restricted_member_access(cacheNode->leftNode, table, currentScope);

		if (resMemRep.status == ERROR) {
			if (retType.type != CLASS_REF) {
				return resMemRep;
			}
			
			struct SemanticReport inheritRep = SA_handle_inherited_functions_and_vars(&currentScope, table, cacheNode, &resMemRep, &entry);

			if (inheritRep.status == ERROR) {
				return resMemRep;
			}
		}

		if (SA_handle_external_references(node, currentScope, entry) == EXTERNAL_FOUND) {
			return SA_create_semantic_report(externalDec, SUCCESS, NULL, NONE, nullCont);
		}

		struct SemanticReport checkRes = SA_execute_access_type_checking(cacheNode, currentScope, table);

		if (checkRes.status == ERROR) {
			return checkRes;
		} else if (cacheNode->type == _CLASS_ACCESS_NODE_
			&& (entry.entry->visibility != GLOBAL
			&& entry.entry->visibility != P_GLOBAL)) {
			char *msg = "Can't access non-global modified vars from outside.";
			char *exp = "Accessing effectively \"hidden\" variables or functions is not possible from outside the class.";
			char *sugg = "Maybe change the modifier to \"global\".";
			struct ErrorContainer errCont = {msg, exp, sugg};
			return SA_create_semantic_report(nullDec, ERROR, cacheNode->leftNode, MODIFIER_EXCEPTION, errCont);
		}

		retType = resMemRep.dec;
		struct SemanticReport scopeReport = SA_set_scope_table_of_member_access(retType, cacheNode, &currentScope, entry);

		if (scopeReport.status == ERROR) {
			return scopeReport;
		}

		cacheNode = cacheNode->rightNode;
	}

	return SA_create_semantic_report(retType, SUCCESS, NULL, NONE, nullCont);
}

/**
 * <p>
 * Checks the current entry for an external reference.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>EXTERNAL_FOUND - When an external is detected
 * <li>NOT_AN_EXTERNAL - When it is not an external reference
 * </ul>
 * 
 * @param *node				Node tht contains the reference
 * @param *currentScope		SemanticTable from which to start searching or the external references (upwards)
 * @param entry				Entry with the found identifier of the memberaccess tree
 */
enum ExternalStatus SA_handle_external_references(Node *node, SemanticTable *currentScope, struct SemanticEntryReport entry) {
	if (entry.entry->internalType == EXTERNAL) {
		ExternalEntry *externalEntry = SA_create_external_entry(FILE_NAME, node, CORRECTNESS_CHECK);
		(void)L_add_item(LIST_OF_EXTERNAL_ACCESSES, externalEntry);
		return EXTERNAL_FOUND;
	} else if (entry.entry->internalType == CLASS_INSTANCE
		|| entry.entry->internalType == FUNCTION) {
		struct SemanticEntryReport refEntry = SA_get_entry_if_available(entry.entry->dec.typeName, currentScope);

		if (refEntry.entry != NULL) {
			if (refEntry.entry->internalType != EXTERNAL) {
				return NOT_AN_EXTERNAL;
			}

			if (entry.entry->internalType == FUNCTION
				&& node->type == _MEM_CLASS_ACC_NODE_) {
				node->leftNode->value = entry.entry->dec.typeName;
			}

			ExternalEntry *externalEntry = SA_create_external_entry(FILE_NAME, node, CORRECTNESS_CHECK);
			(void)L_add_item(LIST_OF_EXTERNAL_ACCESSES, externalEntry);
			return EXTERNAL_FOUND;
		}
	}

	return NOT_AN_EXTERNAL;
}

struct SemanticReport SA_handle_inherited_functions_and_vars(SemanticTable **currentScope, SemanticTable *table, Node *cacheNode, struct SemanticReport *resMemRep, struct SemanticEntryReport *entry) {
	if ((*currentScope)->type == CLASS) {
		SemanticTable *mainTable = SA_get_next_table_of_type((*currentScope), MAIN);

		for (int i = 0; i < (*currentScope)->paramList->load; i++) {
			SemanticEntry *classToSearch = (SemanticEntry*)L_get_item((*currentScope)->paramList, i);

			if (classToSearch == NULL) {
				continue;
			} else if (classToSearch->internalType != EXT_CLASS_OR_INTERFACE) {
				continue;
			}
			
			struct SemanticEntryReport classEntry = SA_get_entry_if_available(classToSearch->name, mainTable);

			if (classEntry.entry == NULL) {
				return *resMemRep;
			} else if (classEntry.entry->internalType == EXTERNAL) {
				return SA_create_semantic_report(externalDec, SUCCESS, NULL, NONE, nullCont);
			}

			*currentScope = (SemanticTable*)classEntry.entry->reference;
			*entry = SA_get_entry_if_available(cacheNode->leftNode->value, *currentScope);
			*resMemRep = SA_check_restricted_member_access(cacheNode->leftNode, table, *currentScope);

			if ((*currentScope)->type == CLASS) {
				if (SA_get_entry_if_available(cacheNode->leftNode->value, *currentScope).entry != NULL) {
					return SA_create_semantic_report(nullDec, SUCCESS, NULL, NONE, nullCont);
				}

				return SA_handle_inherited_functions_and_vars(currentScope, table, cacheNode, resMemRep, entry);
			} else {
				return SA_create_semantic_report(nullDec, SUCCESS, NULL, NONE, nullCont);
			}
		}
	}

	return *resMemRep;
}

struct SemanticReport SA_evaluate_potential_this_keyword(Node *node, Node **cacheNode, SemanticTable **currentScope, SemanticTable *table, struct VarDec *retType) {
	if (node->leftNode->type == _THIS_NODE_) {
		*currentScope = SA_get_next_table_of_type(table, CLASS);

		if (currentScope == NULL || (*currentScope)->type != CLASS) {
			char *msg = "The \"this\" keyword can only be used in classes.";
			char *exp = "The \"this\" keyword references on the instance of the class in which it is called from and thus must be in a class.";
			char *sugg = "Maybe remove the \"this\" keyword.";
			struct ErrorContainer errCont = {msg, exp, sugg};
			struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node->leftNode, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
			return rep;
		}

		*cacheNode = node->rightNode;
		struct VarDec recRet = {CLASS_REF, 0, (*currentScope)->name, false};
		*retType = recRet;
	}

	return nullRep;
}

/**
 * <p>
 * Sets the correct scope table, if an class access is executed.
 * </p>
 * 
 * <p>
 * If a class access is found, then the current scope is set to MAIN and the class is
 * searched, on failure a NOT_DEFINED_EXCEPTION is returned. If found, but the entry is
 * marked as EXTERNAL, it returns success for later checking. Else it sets the new scope
 * to the class table.
 * </p>
 * 
 * <p><strong>Important:</strong>
 * THE CURRENT_SCOPE TABLE IS SET USING POINTERS!
 * </p>
 * 
 * @returns A SemanticReport with potential errors
 * 
 * @param retType			The estimated return type
 * @param *cachedNode		The current accessor node
 * @param **currentScope	Table, in which the next member is searched in
 * @param foundEntry		The entry that was found without considering the class access (default, when no class was detected)
 */
struct SemanticReport SA_set_scope_table_of_member_access(struct VarDec retType, Node *cachedNode, SemanticTable **currentScope, struct SemanticEntryReport foundEntry) {
	if (retType.type == CLASS_REF || retType.type == ENUM_REF) {
		(*currentScope) = SA_get_next_table_of_type(*currentScope, MAIN);
		struct SemanticEntryReport entry = SA_get_entry_if_available(retType.typeName, *currentScope);

		if (entry.entry == NULL) {
			return SA_create_semantic_report(nullDec, ERROR, cachedNode, NOT_DEFINED_EXCEPTION, nullCont);
		} else if (entry.entry->internalType == EXTERNAL) {
			return SA_create_semantic_report(externalDec, SUCCESS, NULL, NONE, nullCont);
		}

		(*currentScope) = (SemanticTable*)entry.entry->reference;
	} else {
		(*currentScope) = foundEntry.entry->reference;
	}

	return nullRep;
}

/**
 * <p>
 * Checks a member access, with only one identifier.
 * </p>
 * 
 * <p>
 * Examples:
 * ```
 * a
 * test()
 * add()
 * list()[0]
 * ```
 * </p>
 * 
 * @returns A SemanticReport with the analyzed type and flags for error an success
 * 
 * @param *node     Node to check
 * @param *table    The table from the scope, at which the member access occured
 * @param *topScope The current top scope in the process
 * 
 * <p>
 * The topScope is defined as the table at which the call occures, with the
 * importance of previously analyzed member access identifiers. That means, if
 * the source contains `add()` as an example, the top scope would be the table
 * at which the function call was called. If we change the example to `Math->add()`,
 * then the top scope would be the table, where the class is defined in.
 * </p>
 */
struct SemanticReport SA_check_restricted_member_access(Node *node, SemanticTable *table,
															SemanticTable *topScope) {
	struct VarDec retType = {CUSTOM, 0, NULL};
	struct SemanticEntryReport entry = SA_get_entry_if_available(node->value, topScope);

	if (entry.entry == NULL) {
		return SA_create_semantic_report(nullDec, ERROR, node, NOT_DEFINED_EXCEPTION, nullCont);
	}

	retType = entry.entry->dec;
	
	if (node->type == _FUNCTION_CALL_NODE_) {
		struct SemanticReport rep = SA_evaluate_function_call(node, entry.entry, table, FNC_CALL);

		if (rep.status == ERROR) {
			return rep;
		}

		retType = rep.dec;
	}
	
	struct SemanticReport arrayRep = SA_handle_array_accesses(&retType, node, topScope);

	if (arrayRep.status == ERROR) {
		return arrayRep;
	}

	return SA_create_semantic_report(retType, SUCCESS, NULL, NONE, nullCont);
}

/**
 * <p>
 * Evaluates a function call for correctness.
 * </p>
 * 
 * <p>
 * A function call can contain another member access, term or function call.
 * The return types are matched with the params of the function.
 * </p>
 * 
 * @return A report with possible error
 * 
 * @param *topNode          Top node, that holds the function call details
 * @param *functionEntry    Entry of the function in the nearest scope table
 * @param *callScopeTable   Table from which the function call was called
 */
struct SemanticReport SA_evaluate_function_call(Node *topNode, SemanticEntry *functionEntry,
												SemanticTable *callScopeTable, enum FunctionCallType fnccType) {
	if (functionEntry == NULL || topNode == NULL || callScopeTable == NULL) {
		return nullRep;
	}
	
	SemanticTable *ref = (SemanticTable*)functionEntry->reference;

	if (ref == NULL) {
		return SA_create_semantic_report(nullDec, ERROR, topNode, NOT_DEFINED_EXCEPTION, nullCont);
	}

	struct SemanticReport preCheck = SA_execute_function_call_precheck(ref, topNode, fnccType);
	
	if (preCheck.status == ERROR) {
		return preCheck;
	} else if (fnccType == FNC_CALL) {
		struct SemanticReport modCheck = SA_evaluate_modifier(ref, functionEntry->visibility, topNode, callScopeTable, true);

		if (modCheck.status == ERROR) {
			return modCheck;
		}
	}

	int actualParams = (int)SA_get_node_param_count(topNode);
	int strictCheck = fnccType == FNC_CALL ? false : true;

	for (int i = 0; i < actualParams; i++) {
		Node *currentNode = topNode->details[i];
		SemanticEntry *currentEntryParam = (SemanticEntry*)L_get_item(ref->paramList, i);
		
		struct VarDec currentNodeType = {CUSTOM, 0, NULL};
		struct SemanticReport idenRep = SA_execute_identifier_analysis(currentNode, ref, &currentNodeType, currentEntryParam, fnccType);

		if (idenRep.status == ERROR) {
			return idenRep;
		}

		if ((int)SA_are_VarTypes_equal(currentEntryParam->dec, currentNodeType, strictCheck) == false) {
			struct Node *errorNode = currentNode->type == _MEM_CLASS_ACC_NODE_ ? currentNode->leftNode : currentNode;
			return SA_create_expected_got_report(currentEntryParam->dec, currentNodeType, errorNode);
		}
	}

	return SA_create_semantic_report(functionEntry->dec, SUCCESS, NULL, NONE, nullCont);
}

struct SemanticReport SA_validate_increment_and_decrement(Node *node, SemanticTable *table) {
	struct SemanticReport memAccRep = SA_evaluate_member_access(node->details[0], table);

	if (memAccRep.dec.type == CLASS_REF) {
		char *msg = "Can't increment or decrement classes.";
		char *exp = "A class is not a number and thus can not be incremented or decremented.";
		char *sugg = "Maybe remove the increment/decrement annotations.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, WRONG_ARGUMENT_EXCPEPTION, errCont);
		return rep;
	} else if (memAccRep.dec.type == STRING) {
		char *msg = "Can't increment or decrement Strings.";
		char *exp = "A string is a collection of characters, that can only be changed char by char.";
		char *sugg = "Maybe remove the increment/decrement annotations.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, WRONG_ARGUMENT_EXCPEPTION, errCont);
		return rep;
	} else if (memAccRep.dec.type == VOID) {
		char *msg = "Can't increment or decrement void.";
		char *exp = "Incrementing or decrementing \"void\" is essentially \"void\".";
		char *sugg = "Maybe remove the increment/decrement annotations.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, WRONG_ARGUMENT_EXCPEPTION, errCont);
		return rep;
	} else if (memAccRep.dec.dimension > 0) {
		char *msg = "Can't increment or decrement an array.";
		char *exp = "Incrementing or decrementing an array is not possible.";
		char *sugg = "Maybe increment or decrement the individual entries in the array.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, WRONG_ARGUMENT_EXCPEPTION, errCont);
		return rep;
	} else if (memAccRep.dec.constant == true) {
		char *msg = "Can't increment or decrement a constant.";
		char *exp = "Incrementing or decrementing a fixed value is not possible.";
		char *sugg = "Maybe remove the \"const\" of the variable you're trying to increment/decrement.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, WRONG_ARGUMENT_EXCPEPTION, errCont);
		return rep;
	}

	return memAccRep;
}

/**
 * <p>
 * Creates an expected-got-exception template message and fills it
 * out with the provided information.
 * </p>
 * 
 * @returns A SemanticReport with the message and errorNode and errorType
 * 
 * @param expected      The expected type
 * @param got           The got type instead of the expected type
 * @param *errorNode    Pointer to the error node
 */
struct SemanticReport SA_create_expected_got_report(struct VarDec expected, struct VarDec got, Node *errorNode) {
	char *expected_str = SA_get_VarType_string(expected);
	char *got_str = SA_get_VarType_string(got);
	size_t length = (size_t)strlen(expected_str) + (size_t)strlen(got_str);
	char *buffer = (char*)calloc(length + 32 + 1, sizeof(char));
	char *sugg = (char*)calloc(length + 26 + 1, sizeof(char));

	if (buffer == NULL || sugg == NULL) {
		(void)THROW_MEMORY_RESERVATION_EXCEPTION("EXP_GOT_ERR_CREATION");
		return SA_create_semantic_report(nullDec, ERROR, errorNode, TYPE_MISMATCH_EXCEPTION, nullCont);
	}

	(void)snprintf(buffer, (length + 32 + 1) * sizeof(char), "Expected \"%s\", but got \"%s\" instead.", expected_str, got_str);
	(void)snprintf(sugg, (length + 26 + 1) * sizeof(char), "Maybe change the \"%s\" to \"%s\".", got_str, expected_str);
	char *exp = "Typesafety is active, so types have to either match strictly or be converted to the according type.";
	struct ErrorContainer errCont = {buffer, exp, sugg};
	(void)free(expected_str);
	(void)free(got_str);
	return SA_create_semantic_report(nullDec, ERROR, errorNode, TYPE_MISMATCH_EXCEPTION, errCont);
}

/**
 * <p>
 * Gets the VarType of an identifier or function call parameter. The evaluated
 * type is then written into the provided type pointer.
 * </p>
 * 
 * @returns Report with errors.
 * 
 * @param *currentNode          Node to check
 * @param *callScopeTable       Table from which the function call was called
 * @param *currentNodeType      Pointer to a modifyable VarType
 * @param *currentEntryParam    The entry of the current parameter in the function
 */
struct SemanticReport SA_execute_identifier_analysis(Node *currentNode, SemanticTable *callScopeTable, struct VarDec *currentNodeType,
									SemanticEntry *currentEntryParam, enum FunctionCallType fnccType) {
	switch (fnccType) {
	case FNC_CALL: {
		struct SemanticReport rep;

		if (currentNode->type == _MEM_CLASS_ACC_NODE_
			|| currentNode->type == _FUNCTION_CALL_NODE_) {
			rep = SA_evaluate_member_access(currentNode, callScopeTable);
		} else {
			rep = SA_evaluate_simple_term(currentEntryParam->dec, currentNode, callScopeTable);
		}

		if (rep.status == ERROR) {
			return rep;
		}

		(*currentNodeType) = rep.dec;
		return nullRep;
	}
	case CONSTRUCTOR_CALL:
	case CONSTRUCTOR_CHECK_CALL: {
		struct VarDec dec = {CUSTOM, 0, NULL};
		
		if (currentNode->details != NULL && currentNode->detailsCount > 0) {
			dec = SA_get_VarType(currentNode->details[0], false, callScopeTable);
		}

		if (dec.type == CUSTOM && fnccType == CONSTRUCTOR_CHECK_CALL) {
			struct SemanticReport termRep = SA_evaluate_simple_term(currentEntryParam->dec, currentNode, callScopeTable);
			dec = currentEntryParam->dec;

			if (termRep.status == ERROR) {
				return termRep;
			}
		}
		
		(*currentNodeType) = dec;
		return nullRep;
	}
	default: return nullRep;
	}

	return nullRep;
}

struct SemanticReport SA_handle_array_accesses(struct VarDec *currentType, struct Node *arrayAccStart, SemanticTable *table) {
	if (arrayAccStart->leftNode == NULL) {
		return nullRep;
	} else if (arrayAccStart->leftNode->type != _ARRAY_ACCESS_NODE_) {
		return nullRep;
	}

	struct Node *cache = arrayAccStart->leftNode != NULL ? arrayAccStart->leftNode : NULL;

	while (cache != NULL) {
		if (cache->leftNode != NULL) {
			struct VarDec expected = {INTEGER, 0, NULL};
			struct SemanticReport termRep = SA_evaluate_simple_term(expected, cache->leftNode, table);

			if (termRep.status == ERROR) {
				return termRep;
			}
		}

		cache = cache->rightNode;
		currentType->dimension--;
	}

	if (currentType->dimension < 0) {
		char *msg = "Negative arrays are not allowed.";
		char *exp = "There's no negative dimension in the SPACE-Lang.";
		char *sugg = "Maybe remove array accesses, that access deeper than allowed.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		return SA_create_semantic_report(nullDec, ERROR, arrayAccStart, NO_SUCH_ARRAY_DIMENSION_EXCEPTION, errCont);
	}

	return nullRep;
}

/**
 * <p>
 * Checks a function call for those properties:
 * 
 * <ul>
 * <li>If the parameter count is equal as the definitions
 * <li>If the function call is a constructor call with no params
 * </ul>
 * </p>
 * 
 * @returns SemanticReport with potentil errors
 * 
 * @param *ref		Reference to the function definition table
 * @param *topNode	Node that holds the function call
 * @param fnccType	Whether the function call is a function call or constructor creation call
 * 
 * @throws TypeMismatchException - If the function call is marked as constructor, but the entry says that it's a function and vice versa
 * @throws WrongArgumentException - If the argument count does not match the definition
 */
struct SemanticReport SA_execute_function_call_precheck(SemanticTable *ref, Node *topNode, enum FunctionCallType fnccType) {
	if (ref == NULL) {
		return nullRep;
	} else if (topNode->detailsCount != ref->paramList->load) {
		char *msg = "The argument count is not equal to the definition.";
		char *exp = "A function cannot take more or less arguments than its definition.";
		char *sugg = "Maybe add or remove overlapping parameters.";
		struct ErrorContainer errCont = {msg, exp, sugg};
		return SA_create_semantic_report(nullDec, ERROR, topNode, WRONG_ARGUMENT_EXCPEPTION, errCont);
	} else if (fnccType == CONSTRUCTOR_CHECK_CALL) {
		return nullRep;
	} else if ((topNode->type == _FUNCTION_CALL_NODE_ && ref->type != FUNCTION)
		|| (topNode->type == _CLASS_CONSTRUCTOR_NODE_ && ref->type != CONSTRUCTOR)) {
		struct VarDec exp = {E_FUNCTION_CALL, 0, NULL};
		struct VarDec got = {E_NON_FUNCTION_CALL, 0, NULL};
		return SA_create_expected_got_report(exp, got, topNode);
	}

	return nullRep;
}

/**
 * <p>
 * Evaluates if a member access is valid or not, by checking
 * the modifier of the accessed object.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - No error occured
 * <li>false - Error occured
 * </ul>
 * 
 * @param *currentScope     The table of the modifier scope
 * @param vis               Assigned modifier to the object that tried to accessed
 * @param *node             Node at which the assignment happens
 * @param *topTable         The outer table of the call
 * @param checkAccessOnly	A flag, that decides what to check, either the access or declaration
 * 
 * <p><strong>Note:</strong>
 * By currentScope the table of the current scope is meant, that means if
 * the expression would be `Book->getPage().number`, the current scope is
 * the `Book`, while the topTable remains in the MAIN routine.
 * </p>
 */
struct SemanticReport SA_evaluate_modifier(SemanticTable *currentScope, enum Visibility vis, Node *node, SemanticTable *topTable, int checkAccessOnly) {
	if (checkAccessOnly == false) {
		if (topTable->type != CLASS) {
			if (vis != P_GLOBAL) {
				char *msg = "Modifiers outside of classes are not allowed.";
				char *exp = "Modifiers cannot effectively be used outside of classes.";
				char *sugg = "Maybe remove the modifier.";
				struct ErrorContainer errCont = {msg, exp, sugg};
				return SA_create_semantic_report(nullDec, ERROR, node, STATEMENT_MISPLACEMENT_EXCEPTION, errCont);
			} else {
				return nullRep;
			}
		}
	} else {
		SemanticTable *nextClassTable = SA_get_next_table_of_type(currentScope, CLASS);
		SemanticTable *nextTopClassTable = SA_get_next_table_of_type(topTable, CLASS);

		if (nextClassTable == NULL || nextTopClassTable->name == NULL
			|| nextClassTable->name == NULL) {
			return nullRep;
		} else if ((int)strcmp(nextTopClassTable->name, nextClassTable->name) == 0
			&& nextClassTable->type != MAIN) {
			return nullRep;
		} else if (vis == PRIVATE || vis == SECURE) {
			char *msg = "Tried to access \"hidden\" declaration.";
			char *exp = "It is not possible to access a variable or class that is \"hidden\" externally.";
			char *sugg = "Maybe change the modifier to \"global\".";
			struct ErrorContainer errCont = {msg, exp, sugg};
			return SA_create_semantic_report(nullDec, ERROR, node, MODIFIER_EXCEPTION, errCont);
		}
	}

	return nullRep;
}

/**
 * <p>
 * This function is specially designed to check the acces operator
 * used in a member/class access.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>-1 - Error occured (break)
 * <li>0 - Exit condition reached (break)
 * <li>1 - Nothing to worry about (continue)
 * </ul>
 * 
 * @param *cacheNode        Operator node
 * @param *currentScope     The SemanticTable of the current scope
 * @param *topScope         The SemanticTable when entering the member access checking
 * 
 */
struct SemanticReport SA_execute_access_type_checking(Node *cacheNode, SemanticTable *currentScope,
									SemanticTable *topScope) {
	if (cacheNode == NULL) {
		return nullRep;
	}
	
	if (cacheNode->type == _CLASS_ACCESS_NODE_) {
		if (currentScope->type != CLASS) {
			char *msg = "Used \"->\" for non-class access instead of \".\".";
			char *exp = "If you want to access a class externally, you have to use \"->\".";
			char *sugg = "Maybe replace the \".\" with a \"->\".";
			struct ErrorContainer errCont = {msg, exp, sugg};
			return SA_create_semantic_report(nullDec, ERROR, cacheNode, WRONG_ACCESSOR_EXCEPTION, errCont);
		}
	} else if (cacheNode->type == _MEMBER_ACCESS_NODE_) {
		if (currentScope->type != ENUM
			&& currentScope->type != CLASS) {
			SemanticTable *nextClassTableFromCall = (SemanticTable*)SA_get_next_table_of_type(topScope, CLASS);

			if (nextClassTableFromCall->type != CLASS
				|| (int)strcmp(currentScope->name, nextClassTableFromCall->name) != 0) {
				char *msg = "Used \".\" for class access instead of \"->\".";
				char *exp = "If you want to access a class externally, you have to use \"->\".";
				char *sugg = "Maybe replace the \".\" with a \"->\".";
				struct ErrorContainer errCont = {msg, exp, sugg};
				return SA_create_semantic_report(nullDec, ERROR, cacheNode, WRONG_ACCESSOR_EXCEPTION, errCont);
			}
		}
	}

	return nullRep;
}

/**
 * <p>
 * Get the SemanticTable with the provided declaration.
 * </p>
 * 
 * <p><strong>Note:</strong>
 * Since classes and variables are only allowed to be used, if declared
 * prior, the function only has to check the parent tables until it reaches
 * the MAIN table.
 * </p>
 * 
 * @return Table that contains the declaration, if non was found it returns NULL
 * 
 * @param *key     	Key to search / identifier / function call to search
 * @param *table    The table in which the searching starts (current scope).
 */
SemanticTable *SA_get_next_table_with_declaration(char *key, SemanticTable *table) {
	SemanticTable *temp = table;

	while (temp != NULL) {
		if ((int)HM_contains_key(key, temp->symbolTable) == false
			&& SA_get_param_entry_if_available(key, temp) == NULL) {
			temp = temp->parent;
		} else {
			break;
		}
	}

	return temp;
}

/**
 * <p>
 * Returns an entry of the table, of the topNode->value (key) is found
 * int the table.
 * </p>
 * 
 * @returns A SemanticEntryReport with the found entry. If nothing was found
 * NULL is set as entry.entry
 * 
 * @param *NodeAsKey    The key to search
 * @param *table        Table in which to search in
 */
struct SemanticEntryReport SA_get_entry_if_available(char *NodeAsKey, SemanticTable *table) {
	if (NodeAsKey == NULL || table == NULL) {
		return SA_create_semantic_entry_report(NULL, false, true);
	}
	
	SemanticEntry *entry = SA_get_param_entry_if_available(NodeAsKey, table);
	
	if ((int)HM_contains_key(NodeAsKey, table->symbolTable) == true) {
		entry = HM_get_entry(NodeAsKey, table->symbolTable)->value;
	}

	if (entry == NULL) {
		return SA_create_semantic_entry_report(NULL, false, true);
	}

	return SA_create_semantic_entry_report(entry, true, false);
}

/**
 * <p>
 * Returns a table with the provided type.
 * </p>
 * 
 * @returns The table with the provided type
 * 
 * @param *currentTable     The table from which to go up from
 * @param type              Type to search
 */
SemanticTable *SA_get_next_table_of_type(SemanticTable *currentTable, enum ScopeType type) {
	SemanticTable *temp = currentTable;

	while (temp != NULL) {
		if (type == temp->type
			|| temp->type == MAIN) {
			break;
		}

		temp = temp->parent;
	}

	return temp;
}

/**
 * <p>
 * Evaluates if an assignment (simple assignment for vars) is correct.
 * </p>
 * 
 * @returns A SemanticReport with potential errors and return type
 * 
 * @param expectedType      The type expected by the statement that is assigned
 * @param *topNode          The root node of the assignment operation
 * @param *table            Table in which the assignment occured
 * 
 * @throws All exceptions from #SA_evaluate_member_access
 */
struct SemanticReport SA_evaluate_assignment(struct VarDec expectedType, Node *topNode, SemanticTable *table) {
	if (expectedType.type == ENUM_REF) {
		SemanticTable *mainTable = SA_get_next_table_of_type(table, MAIN);
		struct SemanticEntryReport possibleEnumEntry = SA_get_entry_if_available(expectedType.typeName, mainTable);

		if (possibleEnumEntry.entry == NULL
			|| possibleEnumEntry.entry->internalType != ENUM) {
			char *msg = "Can't access an enum, which hasn't been defined.";
			char *exp = "You have to define the desired enum before usage.";
			char *sugg = "Maybe create an enum with the according values.";
			struct ErrorContainer errCont = {msg, exp, sugg};
			return SA_create_semantic_report(nullDec, ERROR, topNode->leftNode, NOT_DEFINED_EXCEPTION, errCont);
		}

		struct SemanticReport rep = SA_evaluate_member_access(topNode, mainTable);

		if (rep.status == ERROR) {
			return rep;
		} else if ((int)SA_are_VarTypes_equal(expectedType, rep.dec, true) == false) {
			return SA_create_expected_got_report(expectedType, rep.dec, rep.errorNode);
		}

		return rep;
	}

	struct SemanticReport rep = SA_evaluate_simple_term(expectedType, topNode, table);

	if (rep.dec.type == null && expectedType.type == CLASS_REF) {
		return nullRep;
	}

	return rep;
}

/**
 * <p>
 * Checks if a condition assignment is logically correct.
 * </p>
 * 
 * <p>
 * Examples for conditional assignments:
 * 
 * ```
 * a = b == true ? 2 : 1;
 * text = input == null ? "(null)" : input;
 * ```
 * </p>
 * 
 * @returns SemanticReport with potential errors
 * 
 * @param expectedType		Type that is expected to result out of the `true` and `false` statements
 * @param *topNode			Node that holds the conditional assignment
 * @param *table			Table from which the condition assignment comes from
 * 
 * @throws All exceptions from #SA_evaluate_simple_term
 */
struct SemanticReport SA_evaluate_conditional_assignment(struct VarDec expectedType, Node *topNode, SemanticTable *table) {
	struct SemanticReport condRep = SA_evaluate_chained_condition(table, topNode->leftNode);

	if (condRep.status == ERROR) {
		return condRep;
	}
	
	if (topNode->details != NULL && topNode->detailsCount >= 2) {
		struct SemanticReport trueRep = SA_evaluate_simple_term(expectedType, topNode->details[0], table);
		struct SemanticReport falseRep = SA_evaluate_simple_term(expectedType, topNode->details[1], table);

		if (trueRep.status == ERROR) {
			return trueRep;
		} else if (falseRep.status == ERROR) {
			return falseRep;
		}
	}

	return SA_create_semantic_report(expectedType, SUCCESS, NULL, NONE, nullCont);
}

/**
 * <p>
 * Checks if an array creation is correct or not.
 * </p>
 * 
 * <p>
 * An array creaation is defined as follows:
 * 
 * ```
 * var:int number[][] = new int[2][4];
 * ```
 * </p>
 * 
 * @returns SemanticReport with potential errors
 * 
 * @param expectedType		Type that is expeced after the creation
 * @param *topNode			Node with the array creation
 * @param *table			Table in which the creation happens
 * 
 * @throws TypeMismatchException - When the resulting array is not equal to the awaited array
 */
struct SemanticReport SA_evaluate_array_creation(struct VarDec expectedType, Node *topNode, SemanticTable *table) {
	struct VarDec definedDec = nullDec;
	
	if ((int)SA_set_VarType_type(topNode, &definedDec) == false) {
		definedDec.type = CLASS_REF;
		definedDec.typeName = topNode->value;
	}

	for (int i = 0; i < topNode->detailsCount; i++) {
		Node *detailNode = topNode->details[i];
		definedDec.dimension += detailNode != NULL ? 1 : 0;
	}
	
	if ((int)SA_are_VarTypes_equal(expectedType, definedDec, true) == false) {
		return SA_create_expected_got_report(expectedType, definedDec, topNode);
	}

	return nullRep;
}

/**
 * <p>
 * Checks if an array assignment is valid or not.
 * </p>
 * 
 * <p>
 * The function matches the result with a given expected type
 * if they're not equal an error is thrown.
 * </p>
 * 
 * @returns A SemanticReport with possible errors
 * 
 * @param expectedType	Type that is expected to be the result at the end
 * @param *topNode		Node with the assignment
 * @param *table		Table from which the assignment is directed from
 * 
 * @throws <b>NoSuchArrayDimensionException</b> - When the array acces goes below 0 from the expected type
 * @throws All exceptions from #SA_evaluate_simple_term
 */
struct SemanticReport SA_evaluate_array_assignment(struct VarDec expectedType, Node *topNode, SemanticTable *table) {
	if (topNode->type == _ARRAY_ASSIGNMENT_NODE_) {
		struct VarDec cpyType = expectedType;
		cpyType.dimension -= 1;

		if (cpyType.dimension < 0) {
			char *msg = "Negative arrays are not allowed.";
			char *exp = "There's no negative dimension in the SPACE-Lang.";
			char *sugg = "Maybe remove array accesses, that access deeper than allowed.";
			struct ErrorContainer errCont = {msg, exp, sugg};
			return SA_create_semantic_report(nullDec, ERROR, topNode, NO_SUCH_ARRAY_DIMENSION_EXCEPTION, errCont);
		}

		for (int i = 0; i < topNode->detailsCount; i++) {
			Node *currentNode = topNode->details[i];

			if (currentNode->type == _ARRAY_ASSIGNMENT_NODE_) {
				struct SemanticReport innerRep = SA_evaluate_array_assignment(cpyType, currentNode, table);

				if (innerRep.status == ERROR) {
					return innerRep;
				}
			} else {
				struct SemanticReport termRep = SA_evaluate_simple_term(cpyType, currentNode, table);

				if (termRep.status == ERROR) {
					return termRep;
				}
			}
		}
	} else {
		struct SemanticReport termRep = SA_evaluate_simple_term(expectedType, topNode, table);

		if (termRep.status == ERROR) {
			return termRep;
		}
	}

	return SA_create_semantic_report(expectedType, SUCCESS, NULL, NONE, nullCont);
}

/**
 * <p>
 * This function creates a new SemanticTable for the current scope.
 * </p>
 * 
 * @returns The created SemanticTable
 * 
 * @param *root         The root node of the new scope (usually a `RUNNABLE`)
 * @param scope         The ScopeType of the table (e.g. "MAIN", "CLASS", "FUNCTION", ...)
 * @param *parent       The parent of the table
 * @param *params       Parameters for the table (e.g. functions, constructors ect.)
 * @param line          Line at which the table is created
 * @param position      Character position at which the table is created
 */
SemanticTable *SA_create_new_scope_table(Node *root, enum ScopeType scope,
											SemanticTable *parent, struct ParamTransferObject *params,
											size_t line, size_t position) {
	int paramCount = params == NULL ? 0 : params->params;
	int rootParamCount = root == NULL ? 0 : root->detailsCount;
	SemanticTable *table = SA_create_semantic_table(paramCount, rootParamCount, NULL, scope, line, position);
	table->name = root->value;
	table->parent = parent;
	(void)SA_add_parameters_to_runnable_table(table, params);
	return table;
}

/**
 * <p>
 * Checks if a node is an arithmetic operator or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Node is an arithmetic operator
 * <li>false - Node is not an arithmetic operator
 * </ul>
 * 
 * @param *node     Pointer to the node to check
 */
int SA_is_node_arithmetic_operator(Node *node) {
	switch (node->type) {
	case _PLUS_NODE_: case _MINUS_NODE_: case _MULTIPLY_NODE_: case _MODULO_NODE_:
	case _DIVIDE_NODE_:
		return true;
	default: return false;
	}
}

/**
 * <p>
 * Checks if a node is a bit operator or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Node is a bit operator
 * <li>false - Node is not a bit operator
 * </ul>
 * 
 * @param *node     Pointer to the node to check
 */
int SA_is_node_bit_operator(Node *node) {
	switch (node->type) {
	case _LEFT_BITSHIFT_NODE_: case _RIGHT_BITSHIFT_NODE_: case _XOR_NODE_:
	case _LOGICAL_AND_NODE_: case _LOGICAL_OR_NODE_:
		return true;
	default: return false;
	}
}

/**
 * <p>
 * Ensures that both provided types are equal.
 * </p>
 * 
 * <p>
 * The `strict` flag sets the standard. For strict, the
 * types have to match by 100%, for non-strict the
 * types only have to match the format (FLOAT and DOUBLE).
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Types are equal
 * <li>false - Types are not equal
 * </ul>
 * 
 * @param type1     The first type to check against the second
 * @param type2     The second type to check agains the first
 * @param strict    Flag to enable the strict mode
 */
int SA_are_VarTypes_equal(struct VarDec type1, struct VarDec type2, int strict) {
	return strict == true ?
		(int)SA_are_strict_VarTypes_equal(type1, type2) :
		(int)SA_are_non_strict_VarTypes_equal(type1, type2);
}

/**
 * <p>
 * Checks if two VarTypes are equal, but on a higher standard
 * basis. By that both types have to be the same as the other,
 * unlike the `#...non_strict_var_types...`.
 * </p>
 * 
 * <p><strong>Usage:</strong>
 * This is used to evaluate type equality in a constructor
 * definition, to prevent multiple constructors with equal
 * parameters.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Types are equal
 * <li>false - Types are not equal
 * </ul>
 * 
 * @param type1     The first type to check against the second
 * @param type2     The second type to check agains the first
 */
int SA_are_strict_VarTypes_equal(struct VarDec type1, struct VarDec type2) {
	if ((type1.type == CLASS_REF && type2.type == CLASS_REF)
		|| (type1.type == ENUM_REF && type2.type == ENUM_REF)) {
		if ((int)strcmp(type1.typeName, type2.typeName) == 0
			&& type1.dimension == type2.dimension) {
			return true;
		} else {
			return false;
		}
	}

	if (type1.type == EXTERNAL_RET || type2.type == EXTERNAL_RET) {
		return true;
	}

	return type1.type == type2.type
			&& type1.dimension == type2.dimension ? true : false;
}

/**
 * <p>
 * Checks if two VarTypes are equal, but on a lower standard
 * basis. By that FLOAT's and DOUBLE's are handled equally for
 * instance and are assigned later.
 * </p>
 * 
 * <p><strong>Usage:</strong>
 * This is used to evaluate type equality in a function call.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Types are equal
 * <li>false - Types are not equal
 * </ul>
 * 
 * @param type1     The first type to check against the second
 * @param type2     The second type to check agains the first
 */
int SA_are_non_strict_VarTypes_equal(struct VarDec type1, struct VarDec type2) {
	if (((type1.type == DOUBLE || type1.type == FLOAT)
		&& (type2.type == DOUBLE || type2.type == FLOAT))
		&& type1.dimension == type2.dimension) {
		return true;
	}
	
	if (type1.type == CUSTOM) {
		if (type1.dimension == type2.dimension) {
			return true;
		}

		return false;
	}

	if ((type1.type == CLASS_REF && type2.type == CLASS_REF)
		|| (type1.type == ENUM_REF && type2.type == ENUM_REF)) {
		if ((int)strcmp(type1.typeName, type2.typeName) == 0
			&& type1.dimension == type2.dimension) {
			return true;
		} else {
			return false;
		}
	}

	if (type1.type == EXTERNAL_RET || type2.type == EXTERNAL_RET) {
		return true;
	}

	return (type1.type == type2.type
			&& type1.dimension == type2.dimension) ? true : false;
}

/**
 * <p>
 * Checks if an object is already defined or not.
 * </p>
 * 
 * <p>
 * The checking first occures on the lowest scope
 * going to the highest scope and then stops the search.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Object is already defined
 * <li>false - Object is not defined
 * </ul>
 * 
 * @param *key          Object name to search
 * @param *scopeTable   Current table in the current scope
 */
int SA_is_obj_already_defined(char *key, SemanticTable *scopeTable) {
	SemanticTable *temp = scopeTable;

	while (temp != NULL) {
		if ((int)HM_contains_key(key, temp->symbolTable) == true
			|| SA_get_param_entry_if_available(key, temp) != NULL) {
			return true;
		}

		temp = temp->parent;
	}

	return false;
}

/**
 * <p>
 * Returns the params of a provided node.
 * </p>
 * 
 * <p>
 * The params are always in the node->details pointer.
 * </p>
 * 
 * @returns An object, that contains the parameters
 * 
 * @param *topNode      Node to get the params from
 * @param stdType       The standard type of the params
 * @param *table		Current scope table
 */
struct ParamTransferObject *SA_get_params(Node *topNode, enum ScopeType stdType, SemanticTable *table) {
	struct ParamTransferObject *obj = (struct ParamTransferObject*)calloc(1, sizeof(struct ParamTransferObject));

	if (obj == NULL) {
		(void)THROW_MEMORY_RESERVATION_EXCEPTION("Param_Transfer_Object");
	}

	obj->entries = (SemanticEntry**)calloc(topNode->detailsCount, sizeof(SemanticEntry));

	if (obj->entries == NULL) {
		(void)THROW_MEMORY_RESERVATION_EXCEPTION("Parameter_Entries");
	}

	int actualParams = 0;

	for (int i = 0; i < topNode->detailsCount; i++) {
		Node *innerNode = topNode->details[i];

		if (innerNode == NULL) {
			continue;
		} else if (innerNode->type == _RUNNABLE_NODE_
			|| innerNode->type == _VAR_TYPE_NODE_) {
			continue;
		}
		
		Node *typeNode = innerNode->detailsCount > 0 ? innerNode->details[0] : NULL;
		struct VarDec type = SA_get_VarType(typeNode, false, table);
		SemanticEntry *entry = SA_create_semantic_entry(innerNode->value, type, P_GLOBAL, stdType, NULL, innerNode->line, innerNode->position);
		obj->entries[actualParams++] = entry;
	}

	obj->params = actualParams;
	return obj;
}

/**
 * <p>
 * Get an entry in the param list of the provided table,
 * by the provided key.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>SemanticEntry - The found entry in the table
 * <li>NULL - When no entry was found
 * </ul>
 * 
 * @param *key      Key to search in the param list
 * @param *table    The table to search in
 */
SemanticEntry *SA_get_param_entry_if_available(char *key, SemanticTable *table) {
	if (table == NULL) {
		return NULL;
	} else if (table->paramList == NULL) {
		return NULL;
	}

	for (int i = 0; i < table->paramList->load; i++) {
		SemanticEntry *entry = (SemanticEntry*)L_get_item(table->paramList, i);

		if ((int)strcmp(entry->name, key) == 0) {
			return entry;
		}
	}

	return NULL;
}

/**
 * <p>
 * Returns the VarType of the provided identifier.
 * </p>
 * 
 * @returns The converted VarDec
 * 
 * @param *node     Node to convert
 */
struct VarDec SA_convert_identifier_to_VarType(Node *node) {
	struct VarDec cust = {CUSTOM, 0, NULL};

	switch (node->type) {
	case _FLOAT_NODE_:
		cust.type = DOUBLE;
		break;
	case _NUMBER_NODE_:
		cust.type = INTEGER;
		break;
	default:
		break;
	}
	
	cust.dimension = node->leftNode == NULL ? 0 : (int)atoi(node->leftNode->value);
	return cust;
}

/**
 * <p>
 * Converts a node to the according VarType.
 * </p>
 * 
 * <p>
 * The VarType is received by the details.
 * </p>
 * 
 * @returns The converted VarType
 * 
 * @param *topNode  Node to convert
 * @param constant  Flag for whether the type is a constant
 * @param table		Table from which the next class or enum is checked
 */
struct VarDec SA_get_VarType(Node *node, int constant, SemanticTable *table) {
	struct VarDec cust = {CUSTOM, 0, NULL, constant};

	if (node == NULL) {
		return cust;
	}

	int setType = (int)SA_set_VarType_type(node, &cust);

	if (node->value != NULL && setType == false) {
		SemanticTable *occuranceTable = (SemanticTable*)SA_get_next_table_with_declaration(node->value, table);
		struct SemanticEntryReport entry = SA_get_entry_if_available(node->value, occuranceTable);

		if (entry.entry == NULL
			|| entry.entry->internalType == CLASS) {
			cust.type = CLASS_REF;
		} else if (entry.entry->internalType == ENUM) {
			cust.type = ENUM_REF;
		}

		cust.typeName = node->value;
		cust.dimension = node->leftNode == NULL ? 0 : (int)atoi(node->leftNode->value);
	}

	return cust;
}

/**
 * <p>
 * Uses a matching table to set primitive types to the VarType.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - If the type is a primitive
 * <li>false - If the type is not a primitive
 * </ul>
 * 
 * @param *node		Node from which to determine the type
 * @param *cust		Pointer to the VarType that should be modified
 */
int SA_set_VarType_type(Node *node, struct VarDec *cust) {
	int length = sizeof(TYPE_LOOKUP) / sizeof(TYPE_LOOKUP[0]);

	for (int i = 0; i < length; i++) {
		char *occurance = (char*)strstr(node->value, TYPE_LOOKUP[i].name);
		int pos = occurance - node->value;

		if (pos == 0) {
			(*cust).type = TYPE_LOOKUP[i].type;
			(*cust).dimension = node->leftNode == NULL ? 0 : (int)atoi(node->leftNode->value);
			return true;
		}
	}

	return false;
}

/**
 * <p>
 * Converts a modifier string into a visiblity type.
 * </p>
 * 
 * @returns The converted visibility type
 * 
 * @param *visibilityNode   Node to convert
 */
enum Visibility SA_get_visibility(Node *visibilityNode) {
	if (visibilityNode == NULL) {
		return P_GLOBAL;
	} else if (visibilityNode->type != _MODIFIER_NODE_) {
		printf("MODIFIER NODE IS INCORRECT!\n\n");
		exit(EXIT_FAILURE);
	}

	if ((int)strcmp("global", visibilityNode->value) == 0) {
		return GLOBAL;
	} else if ((int)strcmp("secure", visibilityNode->value) == 0) {
		return SECURE;
	} else if ((int)strcmp("private", visibilityNode->value) == 0) {
		return PRIVATE;
	}

	return P_GLOBAL;
}

/**
 * <p>
 * Creates a semantic report structure with the provided
 * information.
 * </p>
 * 
 * @returns A SemanticReport with the information
 * 
 * @param type          Report return type
 * @param status        Error status
 * @param *errorNode    Node that caused the error
 * @param errorType     Type of the error
 * @param container		Holds the error msg, suggestion and explanation
 */
struct SemanticReport SA_create_semantic_report(struct VarDec type, enum ErrorStatus status, Node *errorNode, enum ErrorType errorType,
												struct ErrorContainer container) {
	struct SemanticReport rep;
	rep.dec = type;
	printf(">>>> >>>> >>>> >>>> ERROR OCC: %i %i\n", status, errorType);
	rep.status = status;
	rep.errorNode = errorNode;
	rep.errorType = errorType;
	rep.container = container;
	return rep;
}

/**
 * <p>
 * Creates an entry report.
 * </p>
 * 
 * @returns A SemanticEntryReport with the provided information
 * 
 * @param *entry        Entry to report
 * @param success       Flag for success
 * @param errorOccured  Flag for errors
 */
struct SemanticEntryReport SA_create_semantic_entry_report(SemanticEntry *entry, int success, int errorOccured) {
	struct SemanticEntryReport rep;
	rep.entry = entry;
	rep.success = success;
	rep.errorOccured = errorOccured;
	return rep;
}

/**
 * <p>
 * Creates an entry for the symboltable.
 * </p>
 * 
 * @returns A SemanticEntry with the provided information
 * 
 * @param *name         Name of the entry
 * @param varType       Return type / type of the entry
 * @param visibility    Visibility of the entry
 * @param internalType  Scope type
 * @param *ptr          Pointer to a reference table (optional)
 */
SemanticEntry *SA_create_semantic_entry(char *name, struct VarDec varType,
												enum Visibility visibility, enum ScopeType internalType,
												void *ptr, size_t line, size_t position) {
	SemanticEntry *entry = (SemanticEntry*)calloc(1, sizeof(SemanticEntry));

	if (entry == NULL) {
		(void)THROW_MEMORY_RESERVATION_EXCEPTION("SemanticEntry_Creation_Function");
		return NULL;
	}

	entry->name = name;
	entry->reference = ptr;
	entry->dec = varType;
	entry->visibility = visibility;
	entry->internalType = internalType;
	entry->line = line;
	entry->position = position;
	return entry;
}

/**
 * <p>
 * Creates an external entry using the ExternalEntry template.
 * </p>
 * 
 * @returns An ExternalEntry with the set values
 * 
 * @param *fileName		Name of the file, in which the external was created
 * @param *node			Node that is considered to be an external
 * @param type			Type of the external (CHECK or DECLARATION)
 */
ExternalEntry *SA_create_external_entry(char *fileName, Node *node, enum ExternalType type) {
	ExternalEntry *entry = (ExternalEntry*)calloc(1, sizeof(ExternalEntry));

	if (entry == NULL) {
		(void)THROW_MEMORY_RESERVATION_EXCEPTION("ExternalEntry_Creation_Function");
		return NULL;
	}

	entry->fileName = fileName;
	entry->node = node;
	entry->type = type;
	return entry;
}

/**
 * <p>
 * Creates a semantic table and fills it with the provided information.
 * </p>
 * 
 * @returns A SemanticTable with the provided information
 * 
 * @param paramCount        How many parameters the table has
 * @param symbolTableSize   How many objects will be in the table (dynamic)
 * @param *parent           Pointer to the parent table
 * @param type              Type of the semantic table
 */
SemanticTable *SA_create_semantic_table(int paramCount, int symbolTableSize, SemanticTable *parent,
												enum ScopeType type, size_t line, size_t position) {
	SemanticTable *table = (SemanticTable*)calloc(1, sizeof(SemanticTable));

	if (table == NULL) {
		(void)THROW_MEMORY_RESERVATION_EXCEPTION("SemanticTable_Creation_Function");
		return NULL;
	}

	table->paramList = CreateNewList(paramCount);
	table->symbolTable = CreateNewHashMap(symbolTableSize > 0 ? symbolTableSize : 1);
	table->parent = parent;
	table->type = type;
	table->line = line;
	table->position = position;
	return table;
}

/**
 * UNDER CONSTRUCTION!!!!
 */
void FREE_TABLE(SemanticTable *rootTable) {
	for (int i = 0; i < rootTable->paramList->load; i++) {
		(void)free(rootTable->paramList->entries[i]);
	}

	(void)FREE_LIST(rootTable->paramList);

	for (int i = 0; i < rootTable->symbolTable->capacity; i++) {
		if (rootTable->symbolTable->entries[i] == NULL) {
			continue;
		} else if (rootTable->symbolTable->entries[i]->value == NULL) {
			continue;
		}

		SemanticEntry *entry = (SemanticEntry*)rootTable->symbolTable->entries[i]->value;

		if (entry->reference != NULL) {
			(void)FREE_TABLE((SemanticTable*)entry->reference);
		}
	}

	(void)HM_free(rootTable->symbolTable);
}

void THROW_ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("ArithmeticOperationMisplacementException", rep);
}

void THROW_NON_COMPARABLE_CHECK_EXCEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("NonComparableCheckException", rep);
	
	if (rep.container.suggestion != NULL) {
		(void)free(rep.container.suggestion);
	}
}

void THROW_NON_BOOLEAN_CHECK_EXCEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("NonBooleanCheckException", rep);
}

void THROW_WRONG_RVAL_EXCEPTION(Node *node, char *description) {
	struct ErrorContainer errCont = {description, NULL, NULL};
	struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, WRONG_RVAL_EXCEPTION, errCont);
	(void)THROW_EXCEPTION("NotAValidRValException", rep);
}

void THROW_WRONG_LVAL_EXCEPTION(Node *node, char *description) {
	struct ErrorContainer errCont = {description, NULL, NULL};
	struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, WRONG_LVAL_EXCEPTION, errCont);
	(void)THROW_EXCEPTION("NotAValidLValException", rep);
}

void THROW_NO_SUCH_ARRAY_DIMENSION_EXCEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("NoSuchArrayDimensionException", rep);
}

void THROW_MODIFIER_EXCEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("ModifierException", rep);
}

void THROW_WRONG_ARGUMENT_EXCEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("WrongArgumentException", rep);
}

void THROW_WRONG_ACCESSOR_EXEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("WrongAccessorException", rep);
}

void THROW_STATEMENT_MISPLACEMENT_EXEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("StatementMisplacementException", rep);
}

void THROW_TYPE_MISMATCH_EXCEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("TypeMismatchException", rep);

	if (rep.container.description != NULL) {
		(void)free(rep.container.description);
	}

	if (rep.container.suggestion != NULL) {
		(void)free(rep.container.suggestion);
	}
}

void THROW_CONSTRUCTOR_NOT_DEFINED_EXCEPTION(Node *node) {
	struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, CONSTRUCTOR_NOT_DEFINED_EXCEPTION, nullCont);
	(void)THROW_EXCEPTION("ConstructorNotDefinedException", rep);
}

void THROW_NOT_DEFINED_EXCEPTION(Node *node) {
	struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, NOT_DEFINED_EXCEPTION, nullCont);
	(void)THROW_EXCEPTION("NotDefinedException", rep);
}

void THROW_CONSTRUCTOR_ALREADY_DEFINED_EXCEPTION(Node *node) {
	struct SemanticReport rep = SA_create_semantic_report(nullDec, ERROR, node, CONSTRUCTOR_NOT_DEFINED_EXCEPTION, nullCont);
	(void)THROW_EXCEPTION("ConstructorAlreadyDefinedException", rep);
}

void THROW_ALREADY_DEFINED_EXCEPTION(struct SemanticReport rep) {
	(void)THROW_EXCEPTION("AlreadyDefinedException", rep);

	if (rep.container.description != NULL) {
		(void)free(rep.container.description);
	}
}

void THROW_MEMORY_RESERVATION_EXCEPTION(char *problemPosition) {
	printf(TEXT_COLOR_RED "MemoryReservationException: at %s\n", problemPosition);
	printf("Error was thrown while semantic analysis.\n");
	printf("This error is an internal issue, please recompile.\n" TEXT_COLOR_RESET);
	exit(EXIT_FAILURE);
}

/**
 * <p>
 * Throws an standard exception with the provided message and node details.
 * </p>
 * 
 * @param *message  Message / Exception to write
 * @param rep       Report to print
 */
void THROW_EXCEPTION(char *message, struct SemanticReport rep) {
	int charsInLine = 0;
	int errorCharsAwayFromNL = 0;
	struct Node *node = rep.errorNode;

	for (int i = node->position; i > 0; i--, errorCharsAwayFromNL++) {
		if ((*BUFFER)[i - 1] == '\n' || (*BUFFER)[i - 1] == '\0') {
			break;
		}
	}

	for (int i = node->position - charsInLine; (*BUFFER)[i] != '\n' && (*BUFFER)[i] != '\0'; i++, charsInLine++);
	charsInLine += errorCharsAwayFromNL;

	(void)printf(TEXT_COLOR_RED);
	(void)printf("%s: at line ", message);
	(void)printf(TEXT_UNDERLINE);
	(void)printf(TEXT_COLOR_BLUE);
	(void)printf("%li:%i", node->line + 1, errorCharsAwayFromNL);
	(void)printf(TEXT_COLOR_RESET);
	(void)printf(TEXT_COLOR_RED);
	(void)printf(" from \"%s\"\n", FILE_NAME);
	
	if (rep.errorNode == NULL) {
		(void)printf(TEXT_COLOR_RESET);
		return;
	}

	char firstFoldMeta[32];
	int minSkip = (int)snprintf(firstFoldMeta, 32, "    at: ");
	(void)printf(firstFoldMeta);
	(void)printf(TEXT_COLOR_GRAY);

	for (int i = 0; i < charsInLine; i++) {
		int pos = node->position - errorCharsAwayFromNL + i;
		(void)printf("%c", (*BUFFER)[pos]);
	}

	(void)printf("\n");
	(void)printf(TEXT_COLOR_RED);
	int WSSP = node->position - errorCharsAwayFromNL;
	int WSLen = node->position + minSkip;

	for (int i = WSSP; i < WSLen; i++) {
		(void)printf(" ");
	}

	(void)printf(TEXT_COLOR_YELLOW);

	for (int i = 0; i < (int)strlen(node->value) && i < 1000; i++) {
		(void)printf("^");
	}

	(void)printf("\n" TEXT_COLOR_RED);
	struct ErrorContainer container = rep.container;

	if (container.description != NULL) {
		(void)printf("    Error: %s\n", container.description);
	}
	
	if (container.explanation != NULL) {
		(void)printf("    Explanation: %s\n", container.explanation);
	}

	if (container.suggestion != NULL) {
		(void)printf("    Suggestion: %s\n", container.suggestion);
	}

	(void)printf(TEXT_COLOR_RESET);
}

/**
 * <p>
 * Takes a SemanticReport and Throws the according error.
 * </p>
 */
void THROW_ASSIGNED_EXCEPTION(struct SemanticReport rep) {
	switch (rep.errorType) {
	case ALREADY_DEFINED_EXCEPTION:
		(void)THROW_ALREADY_DEFINED_EXCEPTION(rep);
		break;
	case CONSTRUCTOR_NOT_DEFINED_EXCEPTION:
		(void)THROW_CONSTRUCTOR_NOT_DEFINED_EXCEPTION(rep.errorNode);
		break;
	case NOT_DEFINED_EXCEPTION:
		(void)THROW_NOT_DEFINED_EXCEPTION(rep.errorNode);
		break;
	case TYPE_MISMATCH_EXCEPTION:
		(void)THROW_TYPE_MISMATCH_EXCEPTION(rep);
		break;
	case STATEMENT_MISPLACEMENT_EXCEPTION:
		(void)THROW_STATEMENT_MISPLACEMENT_EXEPTION(rep);
		break;
	case WRONG_ACCESSOR_EXCEPTION:
		(void)THROW_WRONG_ACCESSOR_EXEPTION(rep);
		break;
	case WRONG_ARGUMENT_EXCPEPTION:
		(void)THROW_WRONG_ARGUMENT_EXCEPTION(rep);
		break;
	case MODIFIER_EXCEPTION:
		(void)THROW_MODIFIER_EXCEPTION(rep);
		break;
	case NO_SUCH_ARRAY_DIMENSION_EXCEPTION:
		(void)THROW_NO_SUCH_ARRAY_DIMENSION_EXCEPTION(rep);
		break;
	case NON_BOOLEAN_CHECK_EXCEPTION:
		(void)THROW_NON_BOOLEAN_CHECK_EXCEPTION(rep);
		break;
	case NON_COMPARABLE_CHECK_EXCEPTION:
		(void)THROW_NON_COMPARABLE_CHECK_EXCEPTION(rep);
		break;
	case ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION:
		(void)THROW_ARITHMETIC_OPERATION_MISPLACEMENT_EXCEPTION(rep);
		break;
	default:
		(void)THROW_EXCEPTION("Exception", rep);
		break;
	}
}

/**
 * <p>
 * The lookup structure of the VarType to String function.
 * </p>
 */
struct VarTypeString {
	enum VarType type;
	char *string;
};

/**
 * <p>
 * Collection of all primitive VarTypes, that can be matched.
 * </p>
 */
struct VarTypeString VarTypeStringLookup[] = {
	{INTEGER, "INTEGER"}, {DOUBLE, "DOUBLE"}, {FLOAT, "FLOAT"}, {STRING, "STRING"}, {LONG, "LONG"},
	{SHORT, "SHORT"}, {BOOLEAN, "BOOLEAN"}, {CHAR, "CHAR"}, {CUSTOM, "CUSTOM"}, {VOID, "VOID"},
	{null, "null"}, {EXTERNAL_RET, "EXT"}, {E_FUNCTION_CALL ,"<FUNCTION_CALL>"},
	{E_NON_FUNCTION_CALL, "<NON_FUNCTION_CALL>"}
};

/**
 * <p>
 * Converts a VarType into the string version.
 * </p>
 * 
 * @returns The converted VarType
 * 
 * @param type  Type to convert
 */
char *SA_get_VarType_string(struct VarDec type) {
	int size = 8;
	size += (int)abs(type.dimension) * 2;
	size += (type.typeName != NULL) ? (int)strlen(type.typeName) : 0;
	char *buffer = NULL;

	if (type.type != CLASS_REF && type.type != ENUM_REF) {
		int lookupSize = sizeof(VarTypeStringLookup) / sizeof(VarTypeStringLookup[0]);

		for (int i = 0; i < lookupSize; i++) {
			if (type.type == VarTypeStringLookup[i].type) {
				buffer = VarTypeStringLookup[i].string;
			}
		}
	} else {
		buffer = type.typeName;
	}

	char *string = (char*)calloc(size + 1, sizeof(char));
	(void)strncpy(string, buffer, size);

	if (type.dimension < 0) {
		(void)strcat(string, "-");
	}

	for (int i = 0; i < abs(type.dimension); i++) {
		(void)strcat(string, "[]");
	}

	return string;
}

/**
 * <p>
 * Get the string of the scope type.
 * This function basically "stringyfies" the scope type.
 * </p>
 * 
 * @returns The converted string
 * 
 * @param type  Scope type to convert
 */
char *SA_get_ScopeType_string(enum ScopeType type) {
	switch (type) {
	case VARIABLE:
		return "VARIABLE";
	case FUNCTION_CALL:
		return "FUNCTION_CALL";
	case CLASS:
		return "CLASS";
	case IF:
		return "IF";
	default: return "<REST>";
	}
}