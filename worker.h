#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>

#include "dbank.h"
#include "pq.h"
#include "desicion.h"
#include "avl.h"
#include "plates.h"

typedef struct
{
    unsigned char *goal;        //RO
    DBank *dbank;       //RW
    AVL_TREE *pq;       //RW
    AVL_TREE *pq2;      //RW
    AVL_TREE *pq3;      //RW
    AVL_TREE *plates;   //RW
    int *exiting;       //RW
    DESICISON *dOutput; //WO (OUTPUT)
    unsigned int *min;  //RW
    unsigned int *max;  //RW
    pthread_rwlock_t *pqLock;
    pthread_rwlock_t *pq2Lock;
    pthread_rwlock_t *pq3Lock;
    pthread_rwlock_t *platesLock;
    pthread_rwlock_t *exitLock;
    pthread_mutex_t *outputLock;
    int debug;
    int interact;
    unsigned int thres;
    pthread_t firstThread;
} WORKSPACE;

void *doWork(void *arg);

#endif
