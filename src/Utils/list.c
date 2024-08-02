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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../../headers/list.h"
#include "../../headers/modules.h"

/** 
 * The subprogram {@code SPACE/src/list.c} was created
 * to provide a list and its basic functionalities.
 * 
 * The main feature of the list is dynamic resizing.
 * 
 * @version 1.0     24.06.2024
 * @author Lukas Nian En Lampl
*/

/**
 * <p>
 * This defines the resizing factor.
 * The formula is basically: $ newSize = \floor (oldSize \cdot FACTOR) $
 * </p>
 */
const float FACTOR = 2.0;

void L_resize_list(struct List *list);

/**
 * <p>
 * Creates a new list and returns a pointer to the list.
 * </p>
 * 
 * @returns A pointer to the list
 * 
 * @param initialCapacity   The initialCapacity of the list (always at least 16)
 */
struct List *CreateNewList(int initialCapacity) {
	initialCapacity = initialCapacity < 16 ? 16 : initialCapacity;
	struct List *list = (struct List*)calloc(1, sizeof(struct List));

	if (list == NULL) {
		(void)printf("ERROR on reserving list!\n");
		return NULL;
	}

	list->entries = (void**)calloc(initialCapacity, sizeof(void*));
	list->size = initialCapacity;
	list->load = 0;
	return list;
}

/**
 * <p>
 * Adds an item to the provided list.
 * </p>
 * 
 * <p>
 * If the load of the list is equal to its size, the list is resized.
 * </p>
 * 
 * @param *list     List to which to add the item
 * @param *ptr      Pointer to add into the list
 */
void L_add_item(struct List *list, void *ptr) {
	if (list == NULL
		|| list->entries == NULL) {
		return;
	}

	if (list->load >= list->size) {
		(void)L_resize_list(list);
	}

	list->entries[list->load++] = ptr;
}

/**
 * <p>
 * Resizes a provided list.
 * </p>
 * 
 * <p>
 * The new size is equal to this equation:
 * $ newSize = \floor (oldSize * FACTOR) $
 * </p>
 * 
 * @param *list     List to resize
 */
void L_resize_list(struct List *list) {
	size_t newSize = (size_t)(FACTOR * list->size);
	list->entries = (void**)realloc(list->entries, sizeof(void*) * newSize);

	if (list->entries == NULL) {
		printf("ERROR ON RESIZING THE LIST!\n");
		return;
	}

	(void)memset(list->entries + list->size, 0, list->size);
	list->size = newSize;
}

/**
 * <p>
 * Returns a item at a specific position.
 * </p>
 * 
 * @returns
 * <ul>
 * <li> The item at the given position
 * <li> NULL, when n < 0 or n >= load or list = NULL
 * </ul>
 * 
 * @param *list     List from which to get the item
 * @param n         Index of the item
 */
void *L_get_item(struct List *list, int n) {
	if (list == NULL || list->entries == NULL
		|| n >= list->load || n < 0) {
		return NULL;
	}

	return list->entries[n];
}

void L_print_list(struct List *list, int flag) {
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

		printf("%11i|%-23p|\n", i, (void*)list->entries[i]);
	}

	return;
}

void FREE_LIST(struct List *list) {
	list->load = 0;
	list->size = 0;
	(void)free(list->entries);
	(void)free(list);
}