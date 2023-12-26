<h1>SPACE - Language</h1><br>
This project is a compiler for my own programming language.<br>
The program basically reads in an input file and converts it into tokens, then a parsetree and finally in an intermediate language.<br>

<h1>Table of contents</h1><br>
<ol>
  <li>Requiresments</li>
  <li>Running the compiler on Windows</li>
  <li>Running the compiler on Linux</li>
  <li>Notes</li>
</ol>

<br>

<h1>Requirements</h1><br>
To compile the source code into an useable format you'll have to use a C compiler.
I used the gcc compiler throughout the whole project.<br>
If you want to run the program with the Batchfile, you'll need a Terminal and the gcc compiler or else the Batchfile won't work.

<br><br>

<h1>To run on Windows:</h1>
<b>1.</b> Download the code and put it into the desired directory<br>
<b>2.</b> Now you have two options:<br><br>
<b><h3>Option 1:</h3></b><br>
<b>3.1.1.</b> Open the terminal and head into the directory at which you have saved the repository.<br>
<b>3.1.2.</b> Now compile the code with a C compiler (Here: gcc) type: <code>gcc main/input.c src/lexer.c src/stack.c src/syntaxAnalyzer.c src/parsetreeGenerator.c src/errorHandler.c src/modules.c -o space.exe</code><br>
<b>3.1.3.</b> Now run the compiled <code>space.exe</code> file with <code>space.exe</code> or <code>./space.exe</code>

<br><br>

<b><h3>Option 2:</h3></b><br>
<b>3.2.1</b> Open the Explorer and head to the repository dircetory<br>
<b>3.2.2</b> In the directory you'll find a file named <code>compile.bat</code><br>
<b>3.2.3</b> Run the <code>compile.bat</code> file<br>
<b>3.2.4</b> After executing the <code>compile.bat</code> file you'll find a file named <code>space.exe</code><br>
<b>3.2.5</b> Run the <code>space.exe</code> file

<br><br>

<h1>To run on Linux:</h1>
<b>1.</b> Download the code and put it into the desired directory<br>
<b>2.</b> Open a terminal and head into the directory in which you have saved the repository<br>
<b>3.</b> Now compile the code with a C compiler (Here: gcc) type: <code>gcc main/input.c src/lexer.c src/stack.c src/syntaxAnalyzer.c src/parsetreeGenerator.c src/errorHandler.c src/modules.c -o space.exe</code><br>
<b>4.</b> Now run the executable by typing <code>./space</code>

<br><br>

<h4>Note:</h4>
To change the input head into the <code>prgm.txt</code> file and change the code to the desired code (It has to follow the grammar rules)<br>
This repository is still in it's early stage, so it may be that you encounter issues or unexpected errors.<br>
If rules are applied correctly everything works as intended!<br><br>
All files may be changed based on bugs, errors and notations.
