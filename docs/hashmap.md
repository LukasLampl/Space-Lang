# SPACE Language - [HashMap documentation](../src/hashmap.c) #

by Lukas Lampl  (15.06.2024)

----------------------------

### Content table ###
- [Information](#information)
- [API](#api)
   1. [Creating a HashMap](#1-getting-a-hashmap)
   2. [Adding an entry](#2-adding-an-entry)
      - [Hashing details](#deeper-explaination)
   3. [Getting an entry](#3-getting-an-entry)
   4. [Removing an entry](#4-removing-an-entry)
   5. [Contains an entry](#5-contains-an-entry)
   6. [Finalize and flush](#6-finalize-and-flush)

### Information ###
The HashMap is essentially a structure containing its size, capacity, information, and a pointer to an entry array. Each entry consists of a key (char*) and a value (void*).

This HashMap implementation is by far not the fastest available, but more than enough for the SPACE language.

Running the HashMap on an Intel i7-7700 H @ 2.80 GHz, you'll get these results for 100'000'000 entries:

- **Put-time**: 1.548355 μs / entry
- **Get-time**: 0.736480 μs / entry
- **Total Put-time**: 154.835500 seconds
- **Total Get-time**: 73.648000 seconds
- **Total Entries**: 21'641'585
- **Total Collisions**: 78'050'943
- **Resizings**: 30 (150 initial capacity)

### API ###
#### 1. Getting a HashMap ####
To create a HashMap you have to call:

```C
struct HashMap *CreateNewHashMap(int initCapacity);
```

Now you'll get a HashMap with the initial capacity of the next nearest prime number in correlation to the initCapacity. Since a prime can only be divided by 1 and itself, the data will be spread more evenly.

#### 2. Adding an entry ####
If you want to add an entry into the created HashMap you just have to call:

```C
void HM_add_entry(char *key, char *value, struct HashMap *map);
```

This function first checks if the load of the map would exceed a certain factor. If the factor is exceeded the map is resized. To get *n* the function calculates $n = \frac{load}{capacity}$.

##### Deeper explanation #####
After the size check the *key* is hashed using an unsecure, but fast hashing algorithm, where $h(x) = floor(m \cdot (kA \mod 1)) \mod c$. The *k* describes the integer resulting from the string transformation. If the resulting position/bucket is already in use, a LinkedList is created, that holds the key-value pairs.

String transformation:

$$
t(i)=\sum_{i=0}^{n-1}k_i+(t_{i-1}*31)
$$

The *n* describes the string length, while *k* is the character at the position *i*. *t* is initialized to 0 for i = -1. The final transformation / last transformation is the number we need to process further ($t(n - 1)$). The string transformation is mainly done to get one integer out of a *n* long string and thus provide a simple hashing algorithm.

After the transformation, the rest is applied. Important to note is that $A = \frac{\sqrt{5}-1}{2} \approx 0.618033988749894$ and *c* is the capacity of the HashMap. (A could also be any other real number)

**Security**: As mentioned above the hashing algorithm is not suitable for storing data securely and is just for serving its purpose of being simple and fast.

#### 3. Getting an entry ####
If you want an entry from a HashMap you can call this function:

```C
struct HashMapEntry *HM_get_entry(char *key, struct HashMap *map);
```

It uses a key to find the correct entry and finally returns it. The searching process is the same as in the [add entry](#2-adding-an-entry) function.

#### 4. Removing an entry ####
To remove an entry you can use:

```C
void HM_remove_entry(struct HashMapEntry *entry, struct HashMap *map);
```

It's essentially [getting an entry](#3-getting-an-entry), but instead of returning it, it removes it from the list. After the removal, the LinkedList is reconnected.

#### 5. Contains an entry ####
If you'd like to check whether an entry
is already present in the table, use:

```C
int HM_contains_entry(struct HashMapEntry *entry, struct HashMap *map);
int HM_contains_key(char *key, struct HashMap *map);
```

It's essentially [getting an entry](#3-getting-an-entry), but instead of returning the entry it returns an integer, that expresses a boolean.

- true = 1
- false = 0

#### 6. Finalize and flush ####
If you don't need the list anymore, you can call

```C
void HM_free(struct HashMap *map);
void HM_clear(struct HashMap *map);
```

For either completely freeing the list or just clearing the content.

**Remember**: After the call, the content is not "reachable" anymore and thus "lost forever".

**Important**: Don't forget to call the `HM_free();` at the end of the application or else it ends in a memory leak!
