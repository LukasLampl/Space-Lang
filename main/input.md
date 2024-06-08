# SPACE Language - [Input module documentation](../main/input.c) #

by Lukas Lampl  (08.06.2024)

----------------------------
### Content table ##
**1.** Brief description  
**2.** Precise description  
**3.** Example

### 1. Brief Description ###
The file `input.c` is responsible for processing the raw input and precalculate the needed tokens as well as their individual sizes.

### 2. Precise Description ###
The step of calculating the length and sizes of the tokens is crucial for better ressource management and faster size determination of individual tokens, which increases performance too.

The first step of the input reader is to read in the provided file. The content of the file is written into a buffer. Followed by that the content is checked character by character for possible tokens.  

*Example:*
```JS
var a = 10;
```

The prediction module might say, that 'var', 'a', '=', '10' and ';' are individual tokens and based on the sizes are indentified. Important to notice is that all indentified sizes are at least _N + 1_ to keep space for the termination character ('\0'). In the end the module returns a _InputReaderResult_ structure, containing all the predicted sizes and token length.

### 3. Example ###
> [!NOTE]  
> Here the reading in the content part is skipped.

Let's say we have a function, that adds two numbers and returns them:

```JS
function add(num1, num2) {
    return num1 + num2;
}
```

Now every character is checked individually and if a whitespace, operator, string or comment occures the token number is increased and the size calculated. Basically like this:

```JS
function    <- predicted token, due to whitespace
add         <- predicted token, due to operator
(           <- predicted token, due to operator end
num1        <- predicted token, due to operator
,           <- predicted token, due to operator end
num2        <- predicted token, due to operator
) ...
```

Whilst seperating / predicting the tokens the size of each token is tried to match. Or easier, it counts how many characters are in a token and adds 1 for the termination character. After that there are an array of sizes and an integer with the predicted token length. For our example it would look like this:

```
NUMBER OF TOKENS: 14
SIZES: {9, 4, 2, 5, 2, 5, 2, 2, 7, 5, 2, 5, 2, 2}
```