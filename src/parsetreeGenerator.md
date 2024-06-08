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
The file `parsetreeGenerator.c` processes the already checked tokens from the syntax analyzer and creates a parsetree.

The parsetree is essential for further processing, since all scopes and values are in a more "controllable" structure.

The parsetree generator takes the checked tokens and predicts what the code is trying to achieve and builds a tree based on the prediction.

### 2. Tree Layouts ###
#### 2.1. Member access tree ###
A memeber access tree is responsible for representing identifiers, function calls within them and array accesses. The memeber access is preditable by the '.' and '->' operator. Recursion on the value branches is allowed.

```
   [OPERATOR]
   /        \
[lVal]    [rVal]

[OPERATOR]  := Whether the '.' or '->' was used
[lVal]      := Left value of the tree
[rVal]      := Right value of the tree
```

#### 2.2. Variable tree ####
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

##### 2.2.2. Array Variable #####
As an array variable tree counts every variable, that has a '[' after its name.

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

##### 2.2.3. Condition Variable #####
A condition variable is a variable that is assgined with the use of an decision tree, typically indicated with the '?' operator.

```
     [COND_VAR]
    /     |    \
[MOD]    [T]   [?]
              /   \
          [tVal] [fVal]

[COND_VAR]  := Variable name and type
[MOD]       := Visibility modifier (global, secure or private)
[T]         := Optional type (":int", ":char", ":Object", ...)
[?]         := Condition for assignment ("a == true ? 1 : 2")
[tVal]      := True branch
[fVal]      := False branch
```

##### 2.2.4. Instance Variable #####
Every variable that holds an Object as its value is an instance variable. Those are identified by the 'new' keyword.

```
     [INS_VAR]
    /    |    \
[MOD]   [T]   [Val]

[INS_VAR]   := Variable name and type
[MOD]       := Visibility modifier (global, secure or private)
[T]         := Optional type (":int", ":char", ":Object", ...)
[VAL]       := Value of the variable
```

#### 2.3. Chained Condition tree ####
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

#### 2.4. Term tree ####
In SPACE every expression containing either a '+', '-', '*' or '/' counts as an simple term. Recursion is totally allowed. Since mulitpication and division has a higher precedence than plus and minus, there is a predence table too:

|       | **+** | **-** | **\*** | **/** | **(** | **)** |
|:-----:|:-----:|:-----:|:------:|:-----:|:-----:|:-----:|
| **+** |   =   |   =   |   *    |   /   |   (   |   )   |
| **-** |   =   |   =   |   *    |   /   |   (   |   )   |
|**\*** |   *   |   *   |   =    |   =   |   (   |   )   |
| **/** |   /   |   /   |   =    |   =   |   (   |   )   |
| **(** |   (   |   (   |   (    |   (   |   =   |   =   |
| **)** |   )   |   )   |   )    |   )   |   =   |   =   |

```
   [OPERATOR]
   /        \
[lVal]    [rVal]

[OPERATOR]  := The operator between the term
[lVal]      := Left Value
[rVal]      := Right Value
```

#### 2.5. Fuction tree ####
A function is a code block, that executes a smaller code bock and can be called whenever wanted. It is identified using the 'function' keyword.

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

#### 2.6. Enum tree ####
An enum is typically a structure, that defines constants with help of names and assigns a value to them.

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

#### 2.7. Include / Export tree ####
The include statement is to define the externally used packages, while the export statement is responsible for making a package available to the outside world.

```
[INCLUDE]

[INCLUDE]  := Include with the package / path name
```

```
[EXPORT]

[EXPORT]   := Export with the package / path name
```

#### 2.8. Try-catch tree ####
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

#### 2.9. Class tree ####
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
Constructors are useful, when a class has unitialized values, that have to be initialized before further processing.

```
[CONSTRUCTOR]
      |     \
   [PARAM]  [R]

[CONSTRUCTOR]   := Indicator for the constructor statment
[PARAM]         := Params of the constructor
[R]             := Runnable in the constructor
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
   |
  [IS]
   |
  [R]

[CHECK]     := Value to check
[IS]        := One of multiple branch options
[R]         := Runnable in the is-statement
```

### 2.13. While statement tree ###
While loops can be used for infinite loops or conditional loops.

```
   [WHILE_STMT]
    /       \
[COND]   [RUNNABLE]

[WHILE_STMT]    := Indicator for the while statement
[COND]          := Chained condition to be fulfilled
[RUNNABLE]      := Block in the while statment
```

### 2.14. Do statement tree ###
Do loops can be used for infinite loops or conditional loops, but it executes the code in it at least once.

```
    [DO_STMT]
    /       \
[COND]   [RUNNABLE]

[DO_STMT]       := Indicator for the do statement
[COND]          := Chained condition to be fulfilled
[RUNNABLE]      := Block in the do statment
```

### 2.15. If / else if / else statement tree ###
If, else if and else can be used for executing code blocks based on met conditions.

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

### 2.16. for statement tree ###
For loops are an essential part in the language, since it provides direct codition handling and incrementing the counter.

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

### 2.17. break / continue statement tree ###
Break and continue can either be used to break out of a control flow statment or skipping the code below it.

```
[BREAK]

[CONTINUE]

[BREAK]     := Indicates a break
[CONTINUE]  := Indicates a continue
```

### 2.18. Return statement tree ###
The return statment returns the result of the function, like a number, string, object etc.

```
    [RET_STMT]
    /
[RET]

[RET_STMT]  := Indicator for the return statement
[RET]       := Return value
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