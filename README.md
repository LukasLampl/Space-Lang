This repository is still in it's early stage, so it may be that you encounter issues or unexpected errors.
In my testings everything works fine, when I apply everything as intended.

Every file can contain code or notations that may or may not be removed or changed.

<h1>To use on Windows:</h1>
<b>1.</b> Download the code and put it into the desired directory<br>
<b>2.</b> Open a terminal and head into the directory in which you have saved the repo<br>
<b>3.</b> Now run a compiler for C (I use gcc) and type: <code>gcc main/input.c src/lexer.c src/modules.c src/errorhandler.c src/stack.c src/parsetreeGenerator.c src/syntaxAnalyzer.c -o space.exe</code><br>
<b>4.</b> Now run the compiled <code>space.exe</code> file
<br><br>
<h4>Note:</h4>
To change the input head into the <code>prgm.txt</code> file and change the code to the desired code (It has to follow the rule scopes)
