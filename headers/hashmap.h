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

#ifndef SPACE_HASH_MAP_H_
#define SPACE_HASH_MAP_H_

struct HashMapEntry {
    void *value;
    char *key;
    struct HashMapEntry *linkedEntry;
};

/**
 * <p>
 * Defines the HashMap structure.
 * </p>
 */
struct HashMap {
    /**
     * <p>
     * This defines the current maximum capacity of the HashMap.
     * </p>
     */
    int capacity;

    /**
     * <p>
     * Holds the number of items in the HashMap.
     * </p>
     */
    int load;

    /**
     * <p>
     * Pointer to the HashMapEntry array.
     * </p>
     */
    struct HashMapEntry **entries;

    int resizes;
    int collissions;
};

struct HashMap *CreateNewHashMap(int initCapacity);

//Internal functions
void print_map(struct HashMap *map, int withList);
void HM_add_entry(char *key, char *value, struct HashMap *map);
struct HashMapEntry *HM_get_entry(char *key, struct HashMap *map);
void HM_remove_entry(struct HashMapEntry *entry, struct HashMap *map);
int HM_contains_entry(struct HashMapEntry *entry, struct HashMap *map);
int HM_contains_key(char *key, struct HashMap *map);
void HM_free(struct HashMap *map);
void HM_clear(struct HashMap *map);

#endif