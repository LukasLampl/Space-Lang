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
<b>3.2.4</b> After executing the <code>compile.bat</code> file you'll find a file named <code>space.exe</code>
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
