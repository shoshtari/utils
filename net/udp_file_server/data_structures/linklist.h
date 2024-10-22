
#ifndef LINKLIST_INC
#define LINKLIST_INC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rwmutex.h"
#include "semaphore.h"

typedef struct linklist_entry {
    void *data;
    struct linklist_entry *next, *before;
} linklist_entry;

typedef struct linklist {
    linklist_entry *root, *end;
    int size;
    int valueSize;
    RWMutex *mutex;

} linklist;

linklist *create_linklist(int valueSize);
void rinsert_linklist(linklist *list, void *value);
void* lpop_linklist(linklist* list) ;
// void* rpop_linklist(linklist* list);
void linklist_remove(linklist* list, linklist_entry* node);
void destroy_linklist(linklist *list);
void block_get(linklist *list);

#endif
