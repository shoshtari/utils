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

hashmap *create_hashsmap(int hash_cardinality, int keysize, int valuesize) {
    hashmap *ans = malloc(sizeof(hashmap));
    ans->data = malloc(sizeof(linklist *) * hash_cardinality);
    memset(ans->data, 0, sizeof(linklist *) * hash_cardinality);
    ans->size = 0;
    ans->hash_cardinality = hash_cardinality;
    ans->keysize = keysize;
    ans->valuesize = valuesize;

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
        if (!memcmp(key, cur_data->key, map->keysize)) {
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

        if (!memcmp(key, cur_data->key, map->keysize)) {
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

        if (!memcmp(key, cur_data->key, map->keysize)) {
            linklist_remove(list, iterator);
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

/*int main() {*/
/**/
/*  hashmap *map = create_hashsmap(10, sizeof(int), sizeof(int));*/
/**/
/*  int key, value;*/
/*  int n = 100;*/
/*  for (int i = 0; i < n; i++) {*/
/*    int key = i, value = i * i;*/
/*    hashmap_set(map, &key, &value);*/
/*  }*/
/*  for (int i = 0; i < n; i++) {*/
/*    int key = i;*/
/*    value = *(int*) hashmap_get(map, &key);*/
/*	printf("%d %d\n", key, value);*/
/*  }*/
/**/
/*  destroy_hashmap(map);*/
/**/
/*  return 0;*/
/*}*/
