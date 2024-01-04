# SPACE - Language #
This project is a compiler for my own programming language.
The program basically reads in an input file and converts it into tokens, then a parsetree and finally in an intermediate language.

# Table of contents #
1. Requirements
2. Installation & running the compiler
   - Running the compiler on Windows
   - Running the compiler on Linux
3. Error messages
4. Commands
5. Program examples
6. Contact
</ol>

# 1. Requirements #
To compile the source code into an useable format, you'll have to use a C compiler.
I used the `gcc` compiler throughout the whole project.
If you want to run the program with the Batch file, you'll need a Terminal and the gcc compiler, or else the Batchfile won't work.

# 2. Installation & running the compiler #
## Running the compiler on Windows ##
**1.** Download the code and put it into the desired directory  
**2.** Now you have two options:

### Option 1: ###
**2.1.1.** Open the terminal and head into the directory at which you have saved the repository.  
**2.1.2.** Now compile the code with a C compiler (Here: gcc) type: `gcc main/input.c src/lexer.c src/stack.c src/syntaxAnalyzer.c src/parsetreeGenerator.c src/errorHandler.c src/modules.c -o space.exe`  
**2.1.3.** Now run the compiled `space.exe` file with `space.exe` or `./space.exe`  

### Option 2: ###
**2.2.1** Open the Explorer and head to the repository directory  
**2.2.2** In the directory, you'll find a file named `compile.bat`  
**2.2.3** Run the `compile.bat` file  
**2.2.4** After executing the `compile.bat` file, you'll find a file named `space.exe`  
**2.2.5** Run the `space.exe` file in the terminal  

## Running the compiler on Linux ##
**1.** Download the code and put it into the desired directory  
**2.** Open a terminal and head into the directory in which you have saved the repository  
**3.** Now compile the code with a C compiler (Here: gcc) type: `gcc main/input.c src/lexer.c src/stack.c src/syntaxAnalyzer.c src/parsetreeGenerator.c src/errorHandler.c src/modules.c -o space.bin`  
**4.** Now run the executable by typing `space.bin`  

> [!NOTE]
> To change the input, head into the `prgm.txt` file and change the code to the desired code (It has to follow the grammar rules)  
>
> This repository is still in it's early stage, so it may be that you encounter issues or unexpected errors.  
>
> If rules are applied correctly everything works as intended!  
>
> All files may be changed based on bugs, errors and notations.  

# 3. Error messages #
When you run the compiler on an invalid input you'll get an error message like this:
```
SYNTAX ERROR: An error occured at line 1.
----------------------------------
1 : 19 | for (var i = 0; i = 10; i++) {
                           ^

Unexpected token "=", maybe replace with "==", "<=", ">=", "!=", "<" or ">".
----------------------------------
```

> [!TIP]
> All error messages start with the line (here: `line 1`) on which the error occured, followed by the exact position (here: `1 : 19` => Error at line 1 and position 19) and a replacement suggestion.

<details>
<summary>List of confusing error messages</summary>
 
In the following list you'll find suggestions that might be unclear like `<IDENTIFIER>`

| Error message  | meaning |
| -------------  | ------- |
| `<ARRAY_ASSIGNMENT>` | replace with assignment to array, like: `= {2, 1}` |
| `<ARRAY_ELEMENT>` | replace with array indicator, like: `[]` or `[5]` |
| `<CHAINED_CONDITION>` | replace with condition or condition with `and` or `or` |
| `<CLASS_OBJECT_ACCESS>` | replace with a accessor to a class, like: `MyClass->myVar;` |
| `<DIGIT>` | replace with a single digit, like: `1` or `0` |
| `<EXPRESSION>` | replace with an expression, like: `a = 10;` |
| `<FUNCTION_CALL>` | replace with a function call, like: `test(param1, param2)` |
| `<IDENTIFIER>` | replace with a valid IDENTIFIER |
| `<MULTIPLE_DEFINITION>` | replace with multiple var definition, like: `var a,b` |
| `<NUMBER>` | replace with a valid number, like: `3` or `3.141` |
| `<POINTER>` | replace with valid pointer, like: `*ptr` |
| `<REFERENCE>` | replace with valid reference, like `&ref` or `&(*ptr)` |
| `<STRING>` | replace with a string, like: `"String"` |
</details>

# 4. Commands #
> [!IMPORTANT]
> This section appears later in the project.

# 5. Program examples #
If you don't want to stick to the initialized input, head into the `prgm.txt` file. In the `prgm.txt` file, you'll find a sample program. Now you can change the sample to whatever you want and try it!

> [!CAUTION]
> If you modify the `prgm.txt` file, please follow the defined grammar rules from the `space.grammar` file, which you'll find in the directory `../definitions`. If you don't follow the rules, you'll might get incorrect results.  
>
> More examples in form of an example can be found in the `GrammarExamples.txt`!

<details>
<summary>How to change the input (Example)</summary>

## Modifying the input ##  
If I'd like to change the input to a class named "Calculator" for example, I can edit the content of the `prgm.txt` file to the following code:  

```
class Calculator() => {}
```

To add a function `add(number1, number2)`, I'll just add the function with its visibility or modificator (default: `global`):  

```
global function add(number1, number2) {}
```

And the last step is to merge both together:  

```
class Calculator() => {
  global function add(number1, number2) {
    return number1 + number2;
  }
}
```

There are also some critical applications, in which a return type is forced. Let's say I'd define the function `add(number1, number2)` again, but I want to make sure an `int` gets returned. For that I can do the following thing:

```
class Calculator() => {
  global function:int add(number1, number2) {
    return number1 + number2;
  }
}
```
</details>

# 6. Contact: #  
**E-Mail:** lampl.lukas@outlook.com
