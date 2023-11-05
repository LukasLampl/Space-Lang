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