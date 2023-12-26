/////////////////////////////////////////////////////////////
///////////////////////    LICENSE    ///////////////////////
/////////////////////////////////////////////////////////////
/*
The SPACE-Language compiler compiles an input file into a runnable program.
Copyright (C) 2023  Lukas Nian En Lampl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

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
#include "../headers/modules.h"
#include "../headers/errors.h"
#include "../headers/stack.h"

void init_stack(struct Stack *stack) {
    (void)memset(stack->storage, 0, sizeof(stack->storage));
    stack->stackPointer = 0;
}

void stack_push(struct Stack *stack, struct StackElement element) {
    if (stack->stackPointer >= STACK_SIZE) {
        (void)STACK_OVERFLOW_EXCEPTION();
    }

    stack->storage[stack->stackPointer++] = element;
}

struct StackElement stack_pop(struct Stack *stack) {
    if (stack->stackPointer - 1 < 0) {
        (void)STACK_UNDERFLOW_EXCEPTION();
    }

    return stack->storage[--stack->stackPointer];
}

struct StackElement stack_peek(struct Stack *stack) {
    return stack->storage[stack->stackPointer];
}