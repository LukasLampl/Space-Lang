/////////////////////////////////////////////////////////////
///////////////////////    LICENSE    ///////////////////////
/////////////////////////////////////////////////////////////
/*
The SPACE-Language compiler compiles an input file into a runnable program.
Copyright (C) 2023  Lukas Nian En Lampl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../headers/modules.h"
#include "../headers/errors.h"
#include "../headers/stack.h"
#include "../headers/parsetree.h"
#include "../headers/Token.h"

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////     PARSE TREE GENERATOR     ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

int Generate_Parsetree(TOKEN **tokens, size_t TokenLength) {
    (void)printf("\n\n\n>>>>>>>>>>>>>>>>>>>>    PARSETREE    <<<<<<<<<<<<<<<<<<<<\n\n");

    return 1;
}