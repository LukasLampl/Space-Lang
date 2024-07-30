# SPACE Language - [Parsetree generator module documentation](../src/parsetreeGenerator.c.c) #

by Lukas Lampl  (08.06.2024)

----------------------------
### Content table ##
1. Brief description
2. Tree Layouts  
   1. [Member access tree](#21-member-access-tree)  
   2. [Variable tree](#22-variable-tree)  
      1. [Normal Variable](#221-normal-variable)  
      2. [Array Variable](#222-array-variable)  
      3. [Condition Variable](#223-condition-variable)  
      4. [Instance Variable](#224-instance-variable)  
   3. [Chained Condition tree](#23-chained-condition-tree)  
   4. [Term tree](#24-term-tree)  
   5. [Function tree](#25-fuction-tree)
   6. [Enum tree](#26-enum-tree)
   7. [Include / Export tree](#27-include--export-tree)
   8. [Try / Catch tree](#28-try-catch-tree)
   9. [Class tree](#29-class-tree)
   10. [Class constructor tree](#210-class-constructor-tree)
   11. [Array element tree](#211-array-element-tree)
   12. [Check / Is statement](#212-check--is-statement-tree)
   13. [While statement tree](#213-while-statement-tree)
   14. [Do statement tree](#214-do-statement-tree)
   15. [If / else if / else statement tree](#215-if--else-if--else-statement-tree)
   16. [For statement tree](#216-for-statement-tree)
   17. [break / continue statement tree](#217-break--continue-statement-tree)
   18. [Return statement tree](#218-return-statement-tree)
   19. [Runnable tree](#219-runnable-tree)

----------------------------

### 1. Brief Description ###
The parsetree generation process is the third step or in other compilers the second step for code evaluation. In that step the previously checked tokens are converted into a tree structure, that represents a hierachy in order to check the code for semantic errors.

The syntax analyzer module checks for syntax errors and throws an error, if detected. After that the validated tokens are fed to this module. Like in the syntax analyzer the parsetree generation process in the SPACE compiler is based on prediction (often by lookahead).

Let's say I have two variables:
```JS
var a = 10;
var arr[] = {1, 2};
```

Now for the prediction we just have to look for dissimilarities and the first is the '[', which only occures at array creation. That's basically the whole system of the parsetree generator and how it predicts the next rule.

The conversion to a tree is an important step, since all scopes are getting visible and tokens, that are unnecessary are thrown out.

### 2. Tree Layouts ###
### 2.1. Member access tree ###
A member access tree is responsible for representing identifiers, function calls within them and array accesses. The memeber access is preditable by the '.' and '->' operator. Recursion on the value branches is allowed.

```
   [OPERATOR]
   /        \
[lVal]    [rVal]

[OPERATOR]  := Whether the '.' or '->' was used
[lVal]      := Left value of the tree
[rVal]      := Right value of the tree
```

Examples:
```JS
this.book.getPages()[0]
image.getPixel(x, y)
myVariable
```

### 2.2. Variable tree ###
##### 2.2.1. Normal Variable #####
The term normal variable is defined as a variable that is directly assigned to a value or terminated without a value.

```
     [VAR]
    /  |  \
[MOD] [T] [VAL]

[VAR]       := Variable name and type
[MOD]       := Visibility modifier (global, secure or private)
[T]         := Optional type (":int", ":char", ":Object", ...)
[VAL]       := Value of the variable
```

Examples:
```JS
var a = 10;
var b = "String";
var:double c = 3.14159;
```

##### 2.2.2. Array Variable #####
Storing data in a nomal variable might be complex, if you have a lot of data that has an order. For this case there are arrays, or array variables. Those are defined as variables, that have an '[' after the name.

```
     [ARR_VAR]
    /    |    \
[MOD]   [T]   [VAL]
      [DIMEN]

[ARR_VAR]   := Variable name and type
[MOD]       := Visibility modifier (global, secure or private)
[T]         := Optional type (":int", ":char", ":Object", ...)
[VAL]       := Value of the variable
[DIMEN]     := Array dimensions
```

Examples:
```JS
var arr[] = {1, 2, 3};
var data[][];
```

##### 2.2.3. Condition Variable #####
Sometimes it is easier and simpler to assign a condition to a variable that returns a value. For this SPACE supports condition assignments, that can be identified by the '?'.

```
     [COND_VAR]
   /      |     \
[MOD]    [T]    [?]
              /   |
         [COND] [VAL]

[COND_VAR]  := Conditional var
[MOD]       := Modifier
[?]         := Conditional assignment indicator
[COND]      := Condition
[VAL]       := Values for true and false
[T]         := Type of variable (optional)
```

Examples:
```JS
var offset = a > 2 ? 1 : 0;
var bool = b < 0 ? true : false;
var pointer = position == 0 ? 0 : offset == 1 ? 16 : 1;
```

##### 2.2.4. Instance Variable #####
In SPACE an instance variable is an object / class instance. That means every variable that contains the "new" keyword counts as an instance. Those can be useful for isolating data and functions.

```
     [INS_VAR]
    /    |    \
[MOD]   [T]   [Val]

[INS_VAR]   := Variable name and type
[MOD]       := Visibility modifier (global, secure or private)
[T]         := Optional type (":int", ":char", ":Object", ...)
[VAL]       := Value of the variable
```

Examples:
```JS
var object = new Object();
var cube = new _3DShape(_CUBE_)
```

### 2.3. Chained Condition tree ###
A normal condition can always be represented using true and false. A chained condition is essentially a condition bound with an 'and' or 'or' operator.

*All operators have their own precedence:*

|         | **AND** | **OR** | **(** | **)** |
|:-------:|:-------:|:------:|:-----:|:-----:|
| **AND** |  =      | =      | (     | )     |
| **OR**  |  =      | =      | (     | )     |
|  **(**  |  (      | (      | =     | =     |
|  **)**  |  )      | )      | =     | =     |

*Condition:*
```
   [OPERATOR]
   /        \
[lVal]    [rVal]

[OPERATOR]  := The operator between the condition ("==", "<", "!=", ...)
[lVal]      := Left value
[rVal]      := Right value
```

*Chained Condition:*
```
   [OPERATOR]
   /        \
[lCond]   [rCond]

[OPERATOR]  := The operator between the conditions ("and", "or")
[lCond]     := Left condition
[rCond]     := Right condition
```

### 2.4. Term tree ###
In SPACE every expression containing either a '+', '-', '*' or '/' counts as an simple term. Recursion is totally allowed. Since mulitpication and division has a higher precedence than plus and minus, there is a predence table too:

|         | **+** | **-** | **\*** | **/** | **%** | **(** | **)** | **>>** | **<<** | **\|** | **&** | **^** |
|:-------:|:-----:|:-----:|:------:|:-----:|:-----:|:-----:|:-----:|:------:|:------:|:------:|:-----:|:-----:|
| **+**   |   =   |   =   |   *    |   /   |   %   |   (   |   )   |   =    |   =    |   =    |   =   |   =   |
| **-**   |   =   |   =   |   *    |   /   |   %   |   (   |   )   |   =    |   =    |   =    |   =   |   =   |
|**\***   |   *   |   *   |   =    |   =   |   =   |   (   |   )   |   *    |   *    |   *    |   *   |   *   |
| **/**   |   /   |   /   |   =    |   =   |   =   |   (   |   )   |   /    |   /    |   /    |   /   |   /   |
| **%**   |   %   |   %   |   =    |   =   |   =   |   (   |   )   |   %    |   %    |   %    |   %   |   %   |
| **(**   |   (   |   (   |   (    |   (   |   (   |   =   |   =   |   (    |   (    |   (    |   (   |   (   |
| **)**   |   )   |   )   |   )    |   )   |   )   |   =   |   =   |   )    |   )    |   )    |   )   |   )   |
| **>>**  |   =   |   =   |   *    |   /   |   %   |   (   |   )   |   =    |   =    |   =    |   =   |   =   |
| **<<**  |   =   |   =   |   *    |   /   |   %   |   (   |   )   |   =    |   =    |   =    |   =   |   =   |
| **\|**  |   =   |   =   |   *    |   /   |   %   |   (   |   )   |   =    |   =    |   =    |   =   |   =   |
| **&**   |   =   |   =   |   *    |   /   |   %   |   (   |   )   |   =    |   =    |   =    |   =   |   =   |
| **^**   |   =   |   =   |   *    |   /   |   %   |   (   |   )   |   =    |   =    |   =    |   =   |   =   |

```
   [OPERATOR]
   /        \
[lVal]    [rVal]

[OPERATOR]  := The operator between the term
[lVal]      := Left Value
[rVal]      := Right Value
```

Example:
```JS
2 + 3 * 4
((a + b) * c) / d
3.14159 * r * r * h
```

### 2.5. Fuction tree ###
A function comes in handy, when a code section should be either abstracted or made adaptive. The advantage is, that it can be called whenever needed and then executes the block code inside of it.

```
     [FNC]
    /  |  \
[MOD] [R] [T]
      [P]

[FNC]       := Function name
[MOD]       := Visibility modifier (global, secure or private)
[T]         := Optional type (":int", ":char", ":Object", ...)
[R]         := Runnable block of the function
[P]         := Params of the function
```

Example:
```JS
function pow(num, exp) {}
```

### 2.6. Enum tree ###
Imagine you're developing a piece of software, that indicates the direction of a conveyor belt. the easiest and most intuitive way would be to say "0 = Left, 1 = Stay, 2 = Right", even though you might find it readable, others don't have to and that's where the enums add their functionality.

```
  [ENUM]
    |
 [ENUMER]
     \
    [VAL]

[ENUM]      := Enum's name
[ENUMER]    := A single enumerator element
[VAL]       := Value of the enumerator element
```

Example:
```JS
enum DIRECTION {
   LEFT = 0,
   STAY,
   RIGHT
}
```

### 2.7. Include / Export tree ###
The include statement is to define the externally used packages, while the export statement is responsible for making a package available to the outside world.

```
[INCLUDE]

[INCLUDE]  := Include with the package / path name
```

```
[EXPORT]

[EXPORT]   := Export with the package / path name
```

### 2.8. Try-catch tree ###
The try catch is essentially useful for code blocks that might throw an error.

```
[TRY]
  |
 [R]

[TRY]       := Try statement indicator
[R]         := Runnable of the try statement
```

```
[CATCH]
   |
  [R]

[Catch]     := catch statement indicator
[R]         := Runnable of the catch statement
```

### 2.9. Class tree ###
Classes are an important part, allowing OOP in SPACE. Those have the ability to inherit and implement interfaces.

```
      [CLASS]
    /    |    \
[MOD] [INHRI] [R]
      [INTER]

[CLASS]     := Contains the class name
[MOD]       := Visibility modifier (global, secure or private)
[INHRI]     := Class from which is inherited from
[INTER]     := Implemented interfaces in the class
[R]         := Runnable of the class
```

### 2.10. Class constructor tree ####
Think about a book. Every book has some pages. Those pages have text on them. Now think about this question: Have you ever seen a book without any pages? Well, the answer should be no (or else it would be concerning :P) and you can represent the concept with constructors. Let's say you have a class _Book_ and it contains the function _.getPage()_, how would it return somethin, if no page or textwas initialized? That's what the constructor does.

```
[CONSTRUCTOR]
      |     \
   [PARAM]  [R]

[CONSTRUCTOR]   := Indicator for the constructor statment
[PARAM]         := Params of the constructor
[R]             := Runnable in the constructor
```

Example:
```JS
this::constructor(text) {
   this.text = text;
}
```

### 2.11. Array element tree ###
Array elements occur at different positions in the code, but often after function calls or array accesses.

```
[PARENT]
    |
[ARR_ACC]
        \
        [DIM]

[PARENT]    := Parent node
[ARR_ACC]   := Array access indicator
[DIM]       := Dimension to access
```

### 2.12. Check / Is statement tree ###
The check-is statement is like the classical switch-case statement.

```
  [CHECK]
  /     \
[V]     [IS]
        /  \
      [V]  [R]

[CHECK]     := Value to check
[IS]        := One of multiple branch options
[R]         := Runnable in the is-statement
[V]         := Value to check (member access tree)
```

### 2.13. While statement tree ###
The while loop is like the mother of the other loops. It is really simple and the programmer has to implement an exit condition by himself. It is very practical in situations, where reapeated code is written.

```
   [WHILE_STMT]
    /       \
[COND]   [RUNNABLE]

[WHILE_STMT]    := Indicator for the while statement
[COND]          := Chained condition to be fulfilled
[RUNNABLE]      := Block in the while statment
```

Example:
```JS
while (a < 10) {}
```

### 2.14. Do statement tree ###
Do loops are also like the while loop. They function the same way, but one key difference is, that the do statement executes the block code inside of it at least once, even if the condition is not met.

```
    [DO_STMT]
    /       \
[COND]   [RUNNABLE]

[DO_STMT]       := Indicator for the do statement
[COND]          := Chained condition to be fulfilled
[RUNNABLE]      := Block in the do statment
```

Example:
```JS
do {} while (input.isFinished == false);
```

### 2.15. If / else if / else statement tree ###
If, else if and else statements are a crucial part, because they allow code executions based on met conditions. Imagine you have a log in form and only want to redirect the user, if the field is filled, else you mark it red. Easily done with if and else statement!

*If - Statement:*
```
    [IF_STMT]
   /         \
[COND]     [RUNNABLE]

[IF_STMT]   := Indicator for the if statment
[COND]      := Condition that has to be met to run the runnable
[RUNNABLE]  := Runnable of the if statement
```

*Else - if - Statement:*
```
 
    [EIF_STMT]
   /          \
[COND]     [RUNNABLE]

[EIF_STMT]  := Indicator for the else-if statment
[COND]      := Condition that has to be met to run the runnable
[RUNNABLE]  := Runnable of the else-if statement
```

*Else - Statement:*
```
[ELSE_STMT]
          \
       [RUNNABLE]

[ELSE_STMT]     := Indicator for the else statment
[RUNNABLE]      := Runnable of the else statement
```

Example:
```JS
if (username.length == 0) {
} else {
}
```

### 2.16. for statement tree ###
The for loop itself is basically a while loop, just with a direct counter, which makes it to a one-liner. The functionality is the same.

```
   [FOR_STMT]
   /    |   \
[VAR] [COND] [RUNNABLE]
     [ACTION]

[FOR_STMT]  := Indicator for the for statment
[VAR]       := Var to use as "counter" or iterator
[COND]      := Condition that has to be met to run the loop
[ACTION]    := Actions that occur at every iteration, like incrementing
[RUNNABLE]  := Runnable in the the loop itself
```

Example:
```JS
for (var i = 0; i < 10; i += 2) {}
```

### 2.17. break / continue statement tree ###
The break and continue statements are as useful as all the other statements, since they allow to exit / skip a loop without a whole iteration through it.

```
[BREAK]

[CONTINUE]

[BREAK]     := Indicates a break
[CONTINUE]  := Indicates a continue
```

Examples:
```JS
break;
continue;
```

### 2.18. Return statement tree ###
Imagine you have a function that calculates a high complex formula and now you want the result, it would be really hard and bad if you'd use a global variable for that purpose. The better solution to that is the return statement. That statement can be used to return a value, string, boolean, etc.

```
    [RET_STMT]
    /
[RET]

[RET_STMT]  := Indicator for the return statement
[RET]       := Return value
```

Examples:
```JS
return a * b + c;
return 0;
return true;
return "MyString";
```

### 2.19. Runnable tree ###
The runnable is one of the main "features" that allow blockwise code. A runnable is essentially a collection of expressions and control flow statements.

```
[RUNNABLE]
    |
 [BLOCK]

[RUNNABLE]  := RUNNABLE within a block or as "main" runnable
[BLOCK]     := Code and expressions within the runnable
```