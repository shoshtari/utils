#include "hashmap.h"

#include "linklist.h"

uint32_t hash(void *data, int size, uint32_t cardinality) {
    const uint32_t base = 31;
    const uint32_t prime = 1e9 + 9;
    uint32_t hash_value = 0;

    unsigned char *byte_data = (unsigned char *)data;

    for (int i = 0; i < size; i++) {
        hash_value = (hash_value * base + byte_data[i]) % prime;
    }

    return hash_value % cardinality;
}

hashmap *create_hashsmap(int hash_cardinality, int keysize, int valuesize, int (*compare)(void *, void *)) {
    hashmap *ans = malloc(sizeof(hashmap));
    ans->data = malloc(sizeof(linklist *) * hash_cardinality);
    memset(ans->data, 0, sizeof(linklist *) * hash_cardinality);
    ans->size = 0;
    ans->hash_cardinality = hash_cardinality;
    ans->keysize = keysize;
    ans->valuesize = valuesize;
    ans->compare = compare;
    return ans;
}

void hashmap_set(hashmap *map, void *key, void *value) {
    uint32_t h = hash(key, map->keysize, map->hash_cardinality);
    if (map->data[h] == NULL) {
        map->data[h] = create_linklist(sizeof(hashmap_entry));
    }

    int k2 = *(int *)key;
    hashmap_entry *entry = malloc(sizeof(hashmap_entry));

    entry->key = calloc(1, map->keysize);  // it is important to use calloc, since
                                           // it also set buffer alignments
    memcpy(entry->key, key, map->keysize);

    entry->value = malloc(map->valuesize);
    memcpy(entry->value, value, map->valuesize);

    linklist *list = map->data[h];
    linklist_entry *iterator = list->root;

    for (int i = 0; i < list->size; i++) {
        hashmap_entry *cur_data = iterator->data;

        int k1 = *(int *)cur_data->key;
        if (!map->compare(key, cur_data->key)) {
            int k1 = *(int *)cur_data->key;
            int k2 = *(int *)key;

            free(cur_data->value);
            cur_data->value = entry->value;

            free(entry->key);
            free(entry);
            return;
        }
        iterator = iterator->next;
    }
    map->size++;
    rinsert_linklist(list, entry);

    free(entry);
}

void *hashmap_get(hashmap *map, void *key) {
    uint32_t h = hash(key, map->keysize, map->hash_cardinality);

    if (map->data[h] == NULL) {
        map->data[h] = create_linklist(sizeof(hashmap_entry));
        perror("hash not found to get");
        return NULL;
    }

    linklist *list = map->data[h];

    linklist_entry *iterator = list->root;
    for (int i = 0; i < list->size; i++) {
        hashmap_entry *cur_data = iterator->data;

        if (!map->compare(key, cur_data->key)) {
            return cur_data->value;
        }
        iterator = iterator->next;
    }

    perror("key not found to get");
    return NULL;
}

void hashmap_del(hashmap *map, void *key) {
    uint32_t h = hash(key, map->keysize, map->hash_cardinality);
    if (map->data[h] == NULL) {
        perror("hash not found to delete");
        return;
    }

    linklist *list = map->data[h];

    linklist_entry *iterator = list->root;
    for (int i = 0; i < list->size; i++) {
        hashmap_entry *cur_data = iterator->data;

        if (!map->compare(key, cur_data->key)) {
            linklist_remove(list, iterator);
            map->size--;
            free(cur_data->key);
            free(cur_data->value);
            free(cur_data);

            return;
        }
        iterator = iterator->next;
    }

    perror("key not found to delete");
    return;
}

void destroy_hashmap(hashmap *map) {
    for (int i = 0; i < map->hash_cardinality; i++) {
        if (map->data[i] == NULL) {
            continue;
        }
        linklist_entry *cur = (linklist_entry *)map->data[i]->root;
        while (cur != NULL) {
            hashmap_entry *entry = cur->data;
            free(entry->key);
            free(entry->value);
            cur = cur->next;
        }
        destroy_linklist(map->data[i]);
    }
    free(map->data);
    free(map);
}

int cmp(void *a, void *b) {
    return 0;
}
// int main() {
//     hashmap *map = create_hashsmap(10, sizeof(int), sizeof(int), cmp);

//     int n = 100;
//     for (int i = 0; i < n; i++) {
//         int k = i, v = i * i;
//         hashmap_set(map, &k, &v);
//     }

//     for (int i = 0; i < n; i += 2) {
//         int k = i;
//         hashmap_del(map, &k);
//     }
//     for (int i = 1; i < n; i += 2) {
//         int k = i;

//         printf("%d %d\n", k, *(int *)hashmap_get(map, &k));
//     }

//     destroy_hashmap(map);
// }