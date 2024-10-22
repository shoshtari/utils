#include "rwmutex.h"

RWMutex* create_rwmutex() {
    RWMutex* ans = malloc(sizeof(RWMutex));
    if (ans == NULL) {
        printf("Memory allocation failedn");
        return NULL;
    }

    ans->readCount = 0;

    if (pthread_mutex_init(&ans->masterLock, NULL) != 0) {
        printf("n mutex init has failedn");
        free(ans);
        return NULL;
    }

    if (pthread_mutex_init(&ans->readCountLock, NULL) != 0) {
        printf("n mutex init has failedn");
        pthread_mutex_destroy(&ans->masterLock);
        free(ans);
        return NULL;
    }

    return ans;
}

void lock_rwmutex(RWMutex* mutex) {
    pthread_mutex_lock(&mutex->masterLock);
}

void rlock_rwmutex(RWMutex* mutex) {
    pthread_mutex_lock(&mutex->readCountLock);
    mutex->readCount++;
    if (mutex->readCount == 1) {
        pthread_mutex_lock(&mutex->masterLock);
    }
    pthread_mutex_unlock(&mutex->readCountLock);
}

void unlock_rwmutex(RWMutex* mutex) {
    pthread_mutex_unlock(&mutex->masterLock);
}

void runlock_rwmutex(RWMutex* mutex) {
    pthread_mutex_lock(&mutex->readCountLock);
    mutex->readCount--;
    if (mutex->readCount == 0) {
        pthread_mutex_unlock(&mutex->masterLock);
    }
    pthread_mutex_unlock(&mutex->readCountLock);
}

void destroy_rwmutex(RWMutex* mutex) {
    pthread_mutex_destroy(&mutex->masterLock);
    pthread_mutex_destroy(&mutex->readCountLock);
    free(mutex);
}

// int x = 0;
// int running = 1;
// RWMutex* mutex;

// void* inc(void* arg) {
//     while (running) {
//         lock_rwmutex(mutex);
//         for (int i = 0; i < 100; i++) {
//             x++;
//         }
//         unlock_rwmutex(mutex);
//     }
//     return NULL;
// }

// void* observe(void* arg) {
//     while (running) {
//         rlock_rwmutex(mutex);
//         printf("%d\n", x);
//         runlock_rwmutex(mutex);
//         usleep(100);
//     }
//     return NULL;
// }

// int main() {
//     pthread_t tid[4];

//     mutex = create_rwmutex();

//     pthread_create(&tid[0], NULL, inc, NULL);
//     pthread_create(&tid[1], NULL, observe, NULL);
//     pthread_create(&tid[2], NULL, observe, NULL);
//     pthread_create(&tid[3], NULL, observe, NULL);

//     usleep(100000);
//     running = 0;

//     pthread_join(tid[0], NULL);
//     pthread_join(tid[1], NULL);
//     pthread_join(tid[2], NULL);
//     pthread_join(tid[3], NULL);

//     destroy_rwmutex(mutex);
//     return 0;
// }
