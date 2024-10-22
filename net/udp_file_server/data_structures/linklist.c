#include "linklist.h"

linklist *create_linklist(int valueSize) {
    linklist *ans = malloc(sizeof(linklist));
    ans->valueSize = valueSize;
    ans->size = 0;
    ans->root = NULL;
    ans->end = NULL;
    ans->mutex = create_rwmutex();
    return ans;
}

void rinsert_linklist(linklist *list, void *value) {
    lock_rwmutex(list->mutex);

    linklist_entry *newnode = malloc(sizeof(linklist_entry));
    newnode->data = malloc(list->valueSize);
    memcpy(newnode->data, value, list->valueSize);
    newnode->next = NULL;
    newnode->before = list->end;

    list->size++;
    if (list->end == NULL) {
        list->root = newnode;
    } else {
        list->end->next = newnode;
    }
    list->end = newnode;

    unlock_rwmutex(list->mutex);
}

void *rpop_linklist(linklist *list) {
    lock_rwmutex(list->mutex);

    if (list->size == 0) {
        perror("list is empty");
    }
	linklist_entry* oldend;
	oldend = list->end;
    void *ans = oldend->data;

    list->end = oldend->before;
	list->end->next = NULL;

	free(oldend);
    list->size--;

	if (list->size == 0){
		list->root = NULL;
	}


    unlock_rwmutex(list->mutex);
    return ans;
}

void *lpop_linklist(linklist *list) {
    lock_rwmutex(list->mutex);

    if (list->size == 0 || list->root == NULL) {
        perror("list is empty");
        return NULL;
    }

    linklist_entry *oldroot = list->root;
    void *ans = oldroot->data;

    list->root = list->root->next;
    list->size--;

    free(oldroot);

    if (list->root != NULL) {
        list->root->before = NULL;
    }
	if (list->size == 0){
		list->end = NULL;
	}

    unlock_rwmutex(list->mutex);

    return ans;
}

void linklist_remove(linklist *list, linklist_entry *node) {
    lock_rwmutex(list->mutex);

    if (node == NULL) {
        perror("node is null");
        unlock_rwmutex(list->mutex);
        return;
    }

    if (node == list->root) {
        list->root = list->root->next;
        if (list->root != NULL) {
            list->root->before = NULL;
        }
    } else if (node == list->end) {
        list->end = list->end->before;
        if (list->end != NULL) {
            list->end->next = NULL;
        }
    } else {
        node->before->next = node->next;
        node->next->before = node->before;
    }
    list->size--;
    free(node);
    unlock_rwmutex(list->mutex);
}

void destroy_linklist(linklist *list) {
    lock_rwmutex(list->mutex);

    linklist_entry *cur = list->root, *tmp = NULL;
    for (int i = 0; i < list->size; i++) {
        tmp = cur->next;
        free(cur->data);
        free(cur);
        cur = tmp;
    }

    unlock_rwmutex(list->mutex);
    destroy_rwmutex(list->mutex);

    free(list);
}

// int main() {
//     linklist* list = create_linklist(sizeof(int));
//     int n = 12;
//     for (int i = 0; i < n; i++) {
//         insert_linklist(list, &i);
//     }
//     for (int i = 0; i < n / 2; i++) {
//         if (i % 2 == 0) {
//             free(pop_linklist(list));
//         } else {
//             free(rpop_linklist(list));
//         }
//     }

//     linklist_entry* cur = list->root;
//     while (cur != NULL) {
//         printf("%d\n", *(int*)cur->data);
//         cur = cur->next;
//     }
//     destroy_linklist(list);
// }
