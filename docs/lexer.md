# SPACE Language - [Lexer module documentation](../src/lexer.c) #

by Lukas Lampl  (08.06.2024)

----------------------------
### Content table ##
**1.** Brief description  
**2.** Precise description  
**3.** Example

### 1. Brief Description ###
The file `lexer.c` is responsible for processing the raw input like the `input.c`, but creating tokens while doing that.

### 2. Precise Description ###
The predicted amount of tokens and their sizes is used to allocate enough memory. Now like in the `input.c` file the characters are read character by character and seperated based on whitespace characters, comments, strings and operators. For the best processing the algorithm always has a look ahead of at least 1 character.

A token speartion has to follow these rules:

| Current character | Next character   | Example           |
|-------------------|------------------|-------------------|
| LETTER            | QUOTE            | a"                |
| LETTER            | WHITESPACE       | a b               |
| LETTER            | OPERATOR         | a +               |
| LETTER            | LETTER = KEYWORD | var               |
| OPERATOR          | OPERATOR         | +- (this not: +=) |
| OPERATOR          | LETTER           | +b                |
| OPERATOR          | QUOTE            | *"                |
| LETTER            | QUOTE            | "string"          |
| NUMBER            | WHITESPACE       | 3.14159 +         |
| NUMBER            | OPERATOR         | 2.5-              |

### 3. Example ###
> [!NOTE]  
> For simplicity I stick to the previous example in the [input module](../main/input.md#3-example).

Let's say we have a function, that adds two numbers and returns them:

```JS
function add(num1, num2) {
    return num1 + num2;
}
```

Now all predicted tokens are allocated from memory:

```
NUMBER OF TOKENS: 14
SIZES: {9, 4, 2, 5, 2, 5, 2, 2, 7, 5, 2, 5, 2, 2}

TOKEN 1: {Size: 9}
TOKEN 2: {Size: 4}
TOKEN 3: {Size: 2}
TOKEN 4: {Size: 5}
...
```

After the allocation the tokens are filled up by the rules above. As the final result we get this:

```
TOKEN 1 | Value: "function" | Size: 9 | Line: 1 | Pos: 0
TOKEN 2 | "add"             | Size: 4 | Line: 1 | Pos: 10
TOKEN 3 | "("               | Size: 2 | Line: 1 | Pos: 11
TOKEN 4 | "num1"            | Size: 5 | Line: 1 | Pos: 12
...
```

By that the lexing process is finsihed and the tokens are returned.