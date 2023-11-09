#ifndef SPACE_STACK_H_
#define SPACE_STACK_H_

#define STACK_SIZE 64

struct StackElement {
    struct Rule *rule;
    int optionNumber;
    int tokenNumber;
};

struct Stack {
    int stackPointer;
    struct StackElement storage[STACK_SIZE];
};

void init_stack(struct Stack *stack);

void stack_push(struct Stack *stack, struct StackElement element);
struct StackElement stack_pop(struct Stack *stack);
struct StackElement stack_peek(struct Stack *stack);

#endif  // SPACE_STACK_H_