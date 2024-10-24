#ifndef HASHMAP_INC
#define HASHMAP_INC

#include <stdint.h>
#include <string.h>

#include "linklist.h"

typedef struct hashmap_entry {
    void *key, *value;
} hashmap_entry;

typedef struct hashmap {
    linklist **data;
    int size, hash_cardinality, keysize, valuesize;

    int (*compare)(void *, void *);
    uint32_t (*hash)(void *buffer, int hash_cardinality);
} hashmap;

hashmap *create_hashsmap(int hash_cardinality, int keysize, int valuesize, int (*compare)(void *, void *), uint32_t(*hash)(void* buffer, int hash_cardinality));
void hashmap_set(hashmap *map, void *key, void *value);
void *hashmap_get(hashmap *map, void *key);
void hashmap_del(hashmap *map, void *key);
void destroy_hashmap(hashmap *map);

#endif
