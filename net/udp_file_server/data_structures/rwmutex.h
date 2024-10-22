#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef RWMUTEX_INC
#define RWMUTEX_INC

typedef struct RWMutex {
    int readCount;
    pthread_mutex_t readCountLock;
    pthread_mutex_t masterLock;
} RWMutex;

RWMutex* create_rwmutex();

void lock_rwmutex(RWMutex* mutex);

void rlock_rwmutex(RWMutex* mutex);

void unlock_rwmutex(RWMutex* mutex);
void runlock_rwmutex(RWMutex* mutex);
void destroy_rwmutex(RWMutex* mutex);

#endif