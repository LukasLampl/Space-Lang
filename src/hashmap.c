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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../headers/hashmap.h"
#include "../headers/modules.h"
#include "../headers/errors.h"

/** 
 * The subprogram {@code SPACE/src/hashmap.c} was created
 * to provide a hashmap and its basic functionalities.
 * 
 * The following benchmark wes done on an Intel i7-7700 HQ @ 2.80 GHz.
 * The benchmark measures the time taken for inserting / putting / adding
 * and reading 100'000'000 tokens.
 * 
 * Put-time:                1.548355 μs / entry
 * Get-time:                0.736480 μs / entry
 * Total Put-time:          154.835500 seconds
 * Total Get-time:          73.648000 seconds
 * Total Entries:           21'641'585
 * Total Collisions:        78'050'943
 * Resizings:               30 (150 initial capacity)
 * 
 * @see SPACE/docs/hashmap.md
 * 
 * @version 1.0     15.06.2024
 * @author Lukas Nian En Lampl
*/

#define false 0
#define true 1

/**
 * <p>
 * Defines the 2^31 prime number, which is
 * also the maximum size of the HashMap.
 * </p>
 */
static const int MAX_PRIME = 2147483647;

/**
 * <p>
 * This defines the maximum load factor
 * (n / cap), before the HashMap is resized to
 * the next prime number.
 * </p>
 */
static const float MAXIMUM_LOAD_FACTOR = 0.66f;

/**
 * <p>
 * This defines the scale factor of the HashMap when a
 * resize happens. (newCap = oldCap * SCALE_FACTOR)
 * </p>
 */
static const float SCALE_FACTOR = 1.6f;

///// PROTOTYPES /////

struct HashMapEntry *HM_create_new_entry(char *key, char *value);
void HM_add_internal_entry(struct HashMapEntry *entry, struct HashMap *map);
void HM_add_entry_to_linked_list(struct HashMapEntry *entry, struct HashMap *map, int index);
int HM_get_position_based_on_hash(char *key, int capacity);
void HM_handle_load(struct HashMap *map);
void HM_resize_hashmap(struct HashMap *map, int newCapacity);
int HM_get_next_prime_number(int currentPrime);
int HM_is_prime(int num);
void HM_free_row(struct HashMap *map, int index);
void HM_free_entry(struct HashMapEntry *entry, int freeList);

struct HashMap *CreateNewHashMap(int initCapacity) {
    struct HashMap *map = (struct HashMap*)calloc(1, sizeof(struct HashMap));
    int primeCap = (int)HM_get_next_prime_number(initCapacity);
    map->capacity = primeCap;
    map->entries = (struct HashMapEntry**)calloc(primeCap, sizeof(struct HashMapEntry));

    if (map->entries == NULL) {
        printf("Hashmap allocation failed!\n");
        HM_free(map);
        exit(0);
    }

    return map;
}

void print_map(struct HashMap *map, int withList) {
    if (map == NULL) {
        return;
    }

    printf("HashMap@[%p]\n", (void*)map);
    printf("Map Capacity: %i\n", map->capacity);
    printf("Map Collision: %i\n", map->collissions);
    printf("Map Resizes: %i\n", map->resizes);
    printf("\n");

    if (withList == false) {
        return;
    }

    printf("           |%-23s|%-24s|%-24s|\n", "KEYS", "VALUES", "LINKS");
    printf("-----------+-----------------------+------------------------+------------------------+\n");

    for (int i = 0; i < map->capacity; i++) {
        if (map->entries[i] == NULL) {
            printf("Entry %5i|%-23s|%-24p|%-24s|\n", i, "(null)", NULL, "(0)");
            continue;
        }

        char *key = map->entries[i]->key;
        void *value = map->entries[i]->value;

        char linksString[256] = "";
        int links = 0;
        struct HashMapEntry *temp = map->entries[i]->linkedEntry;

        while (temp != NULL) {
            if ((int)strlen(linksString) == 0) {
                (void)strncpy(linksString, key, 256);
            }

            int size = sizeof(linksString) - strlen(linksString) - 1;
            (void)strncat(linksString, "->", size);
            (void)strncat(linksString, temp->key, size);
            
            temp = temp->linkedEntry;
            links++;
            
            if (strlen(linksString) >= 256 - 1) {
                printf("Warning: linksString buffer full!\n");
                break;
            }
        }

        printf("Entry %5i|%-23s|%-24p|%-24i| %s\n", i, key == NULL ? "(null)" : key, value == NULL ? "(null)" : value, links, linksString);
    }

    return;
}

/**
 * <p>
 * Adds an entry into the HashMap.
 * </p>
 * 
 * <p>
 * The key and value is converted into a HashMapEntry (allocated)
 * and then added.
 * </p>
 * 
 * @param *key      Key of the entry
 * @param *value    Value of the entry
 * @param *map      HashMap to add the entry to
 */
void HM_add_entry(char *key, char *value, struct HashMap *map) {
    struct HashMapEntry *entry = HM_create_new_entry(key, value);
    (void)HM_add_internal_entry(entry, map);
}

/**
 * <p>
 * Adds a HashMapEntry to the given HashMap.
 * </p>
 * 
 * <p>
 * If after the adding the load factor (load / capacity)
 * exceeds 0.75 the HashMap is resized.
 * </p>
 * 
 * <p><strong>Collisions:</strong>
 * If a collision occures, a linked list is set up, to which
 * the samples are added.
 * </p>
 * 
 * @param *entry    Entry to add
 * @param *map      Map to which to add the entry to
 * @param newEntry  Flag for determining if the entry is new or not
 */
void HM_add_internal_entry(struct HashMapEntry *entry, struct HashMap *map) {
    if (entry == NULL || map == NULL) {
        printf("No map or entry to add!\n");
        return;
    }

    map->load++;
    (void)HM_handle_load(map);

    int hashPos = (int)HM_get_position_based_on_hash(entry->key, map->capacity);
    (void)HM_add_entry_to_linked_list(entry, map, hashPos);
    return;
}

/**
 * <p>
 * Adds an entry to the according bucket.
 * </p>
 * 
 * <p>
 * If the bucket is already in use, a linked list is made,
 * else the bucket is set to the entry.
 * </p>
 * 
 * @param *entry    Entry to store
 * @param *map      Map in which to store the entry in
 * @param index     Bucket index in the HashMap
 */
void HM_add_entry_to_linked_list(struct HashMapEntry *entry, struct HashMap *map, int index) {
    if (entry == NULL || map == NULL) {
        return;
    }

    if (map->entries[index] == NULL) {
        map->entries[index] = entry;
    } else {
        struct HashMapEntry *temp = map->entries[index];

        while (temp->linkedEntry != NULL) {
            temp = temp->linkedEntry;
        }

        temp->linkedEntry = entry;
        map->collissions++;
    }
}

/**
 * <p>
 * Creates a new HashMapEntry.
 * </p>
 * 
 * <p>
 * The new entry is allocated from the heap and finally
 * returned.
 * </p>
 * 
 * @param *key      Key of the entry
 * @param *value    Value of the entry
 */
struct HashMapEntry *HM_create_new_entry(char *key, char *value) {
    struct HashMapEntry *entry = (struct HashMapEntry*)calloc(1, sizeof(struct HashMapEntry));
    
    if (entry == NULL) {
        printf("Couldn't allocate space for entry!\n");
        return NULL;
    }
    
    size_t keyLength = (size_t)strlen(key);
    size_t valueLength = (size_t)strlen(value);

    entry->key = (char*)calloc(keyLength + 1, sizeof(char));
    entry->value = (char*)calloc(valueLength + 1, sizeof(char));

    if (entry->key == NULL || entry->value == NULL) {
        printf("Couldn't allocate space for entry key or value!\n");
        return NULL;
    }

    (void)strncpy(entry->key, key, keyLength);
    (void)strncpy(entry->value, value, valueLength);
    return entry;
}

/**
 * <p>
 * Resizes the provided HashMap to the next prime
 * number, when the load (n / cap) > 0.75.
 * </p>
 * 
 * @param *map  Map to check and resize
 */
void HM_handle_load(struct HashMap *map) {
    double n = (double)map->load / (double)map->capacity;
    
    if (n > MAXIMUM_LOAD_FACTOR) {
        int minSize = (int)((double)map->capacity * SCALE_FACTOR);
        int primeCap = (int)HM_get_next_prime_number(minSize);
        (void)HM_resize_hashmap(map, primeCap);
    }
}

/**
 * <p>
 * Gets a HashMapEntry out of the HashMap using the key.
 * </p>
 * 
 * <p><strong>On error:</strong>
 * If no entry was found NULL is returned.
 * </p>
 * 
 * @param *key  Key to the HashMapEntry
 * @param *map  Map from which to get the HashMapEntry from
 */
struct HashMapEntry *HM_get_entry(char *key, struct HashMap *map) {
    if (key == NULL || map == NULL) {
        printf("No map or key to search!\n");
        return NULL;
    }
    
    int hashPos = (int)HM_get_position_based_on_hash(key, map->capacity);
    struct HashMapEntry *temp = map->entries[hashPos];
    
    while (temp != NULL) {
        if ((int)strcmp(temp->key, key) == 0) {
            return temp;
        }

        temp = temp->linkedEntry;
    }

    return NULL;
}

/**
 * <p>
 * Checks if an entry is already in the provided HashMap.
 * </p>
 * 
 * @param *entry    Entry to search
 * @param *map      Map in which to search
 */
int HM_contains_entry(struct HashMapEntry *entry, struct HashMap *map) {
    return HM_get_entry(entry->key, map) == NULL ? false : true;
}

/**
 * <p>
 * Checks if a key is already in the provided HashMap.
 * </p>
 * 
 * @param *key      Key to search
 * @param *map      Map in which to search
 */
int HM_contains_key(char *key, struct HashMap *map) {
    return HM_get_entry(key, map) == NULL ? false : true;
}

/**
 * <p>
 * Removes an entry from the provided HashMap.
 * </p>
 * 
 * <p><strong>On error:</strong>
 * If no entry was found, nothing is removed.
 * </p>
 * 
 * @param *entry    Entry to remove
 * @param *map      Map from which the entry should be removed
 */
void HM_remove_entry(struct HashMapEntry *entry, struct HashMap *map) {
    int hashPos = (int)HM_get_position_based_on_hash(entry->key, map->capacity);
    struct HashMapEntry *prevEntry = map->entries[hashPos];
    struct HashMapEntry *temp = map->entries[hashPos];

    while (temp != NULL) {
        if ((int)strcmp(temp->key, entry->key) == 0) {
            prevEntry->linkedEntry = temp->linkedEntry;
            (void)HM_free_entry(temp, false);
            map->load--;
            break;
        }

        prevEntry = temp;
        temp = temp->linkedEntry;
    }
}

/**
 * <p>
 * This resizes a given HashMap to the provided capacity.
 * </p>
 * 
 * <p>
 * First the old elements are stored, then the entry pointer
 * is reallocated and all entries rehashed. After adding all
 * entries the old entry pointer is freed for avoiding memory leaks.
 * </p>
 * 
 * @param *map          HashMap to resize
 * @param newCapacity   New capacity of the HashMap
 */
void HM_resize_hashmap(struct HashMap *map, int newCapacity) {
    int oldCapacity = map->capacity;
    struct HashMapEntry **entries = map->entries;
    map->entries = (struct HashMapEntry**)calloc(newCapacity, sizeof(struct HashMapEntry));

    if (map->entries == NULL) {
        printf("Hashmap allocation failed!\n");
        HM_free(map);
        return;
    }
    
    map->collissions = 0;
    map->resizes++;
    map->capacity = newCapacity;
    map->load = 0;

    //Re-init old entries
    for (int i = 0; i < oldCapacity; i++) {
        struct HashMapEntry *temp = entries[i];

        while (temp != NULL) {
            struct HashMapEntry *cache = temp->linkedEntry;
            temp->linkedEntry = NULL;
            (void)HM_add_internal_entry(temp, map);
            temp = cache;
        }
    }

    if (entries != NULL) {
        (void)free(entries);
        entries = NULL;
    }
}

/**
 * <p>
 * This function calculates a hash based on multiplaication.
 * </p>
 * 
 * <p>
 * The formula is as follows:
 * ´´´
 * h(x) = floor(m * (kA mod 1))
 * ´´´
 * 
 * First the key is converted into an integer
 * which then is 'k'. The A describes a fixed
 * real number ((sqrt(5) - 1) / 2), while the
 * mod is replaced by `frac`. The last step ensures
 * that the position is within the HashMap capacity.
 * </p> 
 * 
 * @returns Position of the HashMapEntry
 * 
 * @param *key      Key to hash
 * @param capacity  Capacity of the HashMap
 */
int HM_get_position_based_on_hash(char *key, int capacity) {
    if (key == NULL) {
        return 0;
    }

    double A = 0.618033988749894;
    unsigned int hash = 0;

    for (int i = 0; key[i] != '\0'; i++) {
        hash = (hash * 31) + key[i];
    }

    double frac = (hash * A) - (unsigned int)(hash * A);
    return (int)((capacity - 1) * frac);
}

/**
 * <p>
 * Gets the next prime number in correlation to the given
 * number.
 * </p>
 * 
 * @returns The next prime number
 * 
 * @param currentPrime  The current number / prime to increase to the next prime
 */
int HM_get_next_prime_number(int currentPrime) {
    while ((int)HM_is_prime(++currentPrime) == false) {
        if (currentPrime <= 0) {
            printf("INTEGER overflow in finding prime occured!\n");
            return MAX_PRIME;
        }
    }

    return currentPrime;
}

/**
 * <p>
 * Check if a given number is a prime or not.
 * </p>
 * 
 * @returns
 * <ul>
 * <li>true - Number is a prime
 * <li>false - Number is not a prime
 * </ul>
 * 
 * @param num   Number to check
 */
int HM_is_prime(int num) {
    if (num <= 1) {
        return false;
    } else if (num <= 3) {
        return true;
    }

    if (num % 2 == 0 || num % 3 == 0) {
        return false;
    }

    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) {
            return false;
        }
    }

    return true;
}

/**
 * <p>
 * Frees the whole HashMap and its content.
 * </p>
 * 
 * @param *map  Pointer to the HashMap to free
 */
void HM_free(struct HashMap *map) {
    if (map == NULL) {
        return;
    }

    if (map->entries != NULL) {
        (void)HM_clear(map);
        (void)free(map->entries);
        map->entries = NULL;
    }

    (void)free(map);
}

/**
 * <p>
 * Clears the whole HashMap.
 * </p>
 * 
 * @param *map  Pointer to the map to clear
 */
void HM_clear(struct HashMap *map) {
    if (map == NULL || map->entries == NULL) {
        printf("No map to clear!\n");
        return;
    }

    for (int i = 0; i < map->capacity; i++) {
        if (map->entries[i] == NULL) {
            continue;
        }

        (void)HM_free_entry(map->entries[i], true);
    }
}

void HM_free_entry(struct HashMapEntry *entry, int freeList) {
    if (entry == NULL) {
        return;
    }

    if (entry->key != NULL) {
        (void)free(entry->key);
        entry->key = NULL;
    }

    if (entry->value != NULL) {
        (void)free(entry->value);
        entry->value = NULL;
    }

    if (freeList == true && entry->linkedEntry != NULL) {
        (void)HM_free_entry(entry->linkedEntry, freeList);
        entry->linkedEntry = NULL;
    }

    (void)free(entry);
    entry = NULL;
}