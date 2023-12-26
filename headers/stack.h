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