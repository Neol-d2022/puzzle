#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>

#include "dbank.h"
#include "pq.h"
#include "desicion.h"
#include "avl.h"
#include "core.h"

typedef struct
{
    unsigned char *goal;        //RO
    DBank *dbank;       //RW
    AVL_TREE *plates;   //RW
    AVL_TREE *pq;       //RW
    AVL_TREE *pq2;      //RW
    AVL_TREE *pq3;      //RW
    int *exiting;       //RW
    DESICISON *dOutput; //WO (OUTPUT)
    unsigned int *min;  //RW
    unsigned int *max;  //RW
    pthread_rwlock_t *pqLock;
    pthread_rwlock_t *pq2Lock;
    pthread_rwlock_t *pq3Lock;
    pthread_rwlock_t *exitLock;
    pthread_rwlock_t *thresLock;
    pthread_rwlock_t *platesLock;
    pthread_mutex_t *outputLock;
    pthread_rwlock_t *dbankLock;
    CalcDisF CalcDis;
    int debug;
    int interact;
    unsigned int thres;
    unsigned int tests;
    pthread_t firstThread;
} WORKSPACE;

void *doWork(void *arg);

#endif
