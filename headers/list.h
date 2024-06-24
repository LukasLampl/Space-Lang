/////////////////////////////////////////////////////////////
///////////////////////    LICENSE    ///////////////////////
/////////////////////////////////////////////////////////////
/*
The SPACE-Language compiler compiles an input file into a runnable program.
Copyright (C) 2024  Lukas Nian En Lampl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SPACE_LIST_H_
#define SPACE_LIST_H_

struct List {
    size_t size;
    void **entries;
    size_t load;
};

struct List *CreateNewList(int initialCapacity);
void L_add_item(struct List *list, void *ptr);
void *L_get_item(struct List *list, int n);
void L_print_list(struct List *list, int flag);
void FREE_LIST(struct List *list);

#endif