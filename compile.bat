echo off
SET PROFILE_MODE=0

IF %PROFILE_MODE% == 0 (
    gcc -Wall -Werror -Wpedantic main/input.c src/Lexer/lexer.c src/Parser/syntaxAnalyzer.c src/Parser/parsetreeGenerator.c src/errorHandler.c src/Utils/modules.c src/Utils/hashmap.c src/Utils/list.c src/SemanticAnalysis/semanticAnalyzer.c main/main.c -o space.exe
)
IF %PROFILE_MODE% == 1 (
    gcc -Wall -Werror -Wpedantic -pg main/input.c src/Lexer/lexer.c src/Parser/syntaxAnalyzer.c src/Parser/parsetreeGenerator.c src/errorHandler.c src/Utils/modules.c src/Utils/hashmap.c src/Utils/list.c src/SemanticAnalysis/semanticAnalyzer.c main/main.c -o space.exe
)

space.exe

IF %PROFILE_MODE% == 1 (
    gprof space.exe gmon.out > times.txt
    del gmon.out
)

del space.exe