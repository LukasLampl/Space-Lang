echo off
SET PROFILE_MODE=0

IF %PROFILE_MODE% == 0 (
    gcc -Wall -Werror -Wpedantic main/input.c src/lexer.c src/syntaxAnalyzer.c src/parsetreeGenerator.c src/errorHandler.c src/modules.c src/hashmap.c src/semanticAnalyzer.c main/main.c -o space.exe
)
IF %PROFILE_MODE% == 1 (
    gcc -Wall -Werror -Wpedantic -pg main/input.c src/lexer.c src/syntaxAnalyzer.c src/parsetreeGenerator.c src/errorHandler.c src/modules.c src/hashmap.c src/semanticAnalyzer.c main/main.c -o space.exe
)

space.exe

IF %PROFILE_MODE% == 1 (
    gprof space.exe gmon.out > times.txt
    del gmon.out
)

del space.exe