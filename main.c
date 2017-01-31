#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "avl.h"
#include "worker.h"
#include "input.h"
#include "util.h"
#include "core.h"

unsigned int LEVEL = 0;
unsigned int PUZZLE_SIZE = 3;

int main(int argc, char **argv)
{
    char buf[256];
    WORKSPACE w;
    pthread_attr_t attr;
    pthread_rwlock_t pqLock;
    pthread_rwlock_t pq2Lock;
    pthread_rwlock_t pq3Lock;
    pthread_rwlock_t exitLock;
    pthread_rwlock_t thresLock;
    pthread_mutex_t outputLock;
    pthread_mutex_t dbankLock;
    unsigned char *buffers;
    unsigned char *goal;
    unsigned char *input;
    pthread_t *th;
    DESICISON *d;
    DBank *dbank = CreateDecisionBank();
    AVL_TREE *pq = CreatePQ();
    AVL_TREE *pq2 = CreatePQ();
    AVL_TREE *pq3 = CreatePQ();
    clock_t t;
    unsigned int i, min, max, nthreads = 1, x[2], h, e, b;
    int r, debug = 0, interact = 1, argci, exiting = 0;

    for (argci = 1; argci < argc; argci += 1)
    {
        if (strcmp("debug", argv[argci]) == 0)
        {
            debug = 1;
        }

        if (strcmp("size", argv[argci]) == 0 && (argci + 1 < argc))
        {
            sscanf(argv[argci + 1], "%u", &PUZZLE_SIZE);
            if (PUZZLE_SIZE == 0)
                PUZZLE_SIZE = 2;
            argci += 1;
        }

        if (strcmp("thread", argv[argci]) == 0 && (argci + 1 < argc))
        {
            sscanf(argv[argci + 1], "%u", &nthreads);
            argci += 1;
        }

        if (strcmp("level", argv[argci]) == 0 && (argci + 1 < argc))
        {
            sscanf(argv[argci + 1], "%u", &LEVEL);
            if (LEVEL > 4)
                LEVEL = 0;
            argci += 1;
        }

        if (strcmp("fast", argv[argci]) == 0)
        {
            LEVEL = 0;
        }

        if (strcmp("mhtd", argv[argci]) == 0)
        {
            LEVEL = 1;
        }

        if (strcmp("mhtl", argv[argci]) == 0)
        {
            LEVEL = 2;
        }

        if (strcmp("swap", argv[argci]) == 0)
        {
            LEVEL = 3;
        }

        if (strcmp("dijk", argv[argci]) == 0)
        {
            LEVEL = 4;
        }

        if (strcmp("noninteract", argv[argci]) == 0)
        {
            interact = 0;
        }
    }

    goal = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    input = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    printf("SIZE = %ux%u\n", PUZZLE_SIZE, PUZZLE_SIZE);
    printf("THREADS = %u\n", nthreads);
    printf("LEVEL = %s\n", GetCalcFuncStr());
    printf("Input puzzle for GOAL:\n");
    if ((r = GetPlate(goal, stdin)))
        return r;
    printPlate(goal, stdout);
    printf("Input puzzle for CURRENT:\n");
    if ((r = GetPlate(input, stdin)))
        return r;
    printPlate(input, stdout);
    printf("\n");

    buffers = (unsigned char *)malloc(GetBufSize());
    t = clock();
    d = DecisionBankAdd(dbank);
    memcpy(d->p, input, PUZZLE_SIZE * PUZZLE_SIZE);
    w.CalcDis = GetCalcFunc();
    d->h = (w.CalcDis)(d->p, goal, buffers, &(d->b));
    EnqueuePQ(pq, d);

    max = min = d->h;
    pthread_rwlock_init(&pqLock, 0);
    pthread_rwlock_init(&pq2Lock, 0);
    pthread_rwlock_init(&pq3Lock, 0);
    pthread_rwlock_init(&exitLock, 0);
    pthread_rwlock_init(&thresLock, 0);
    pthread_mutex_init(&outputLock, 0);
    pthread_mutex_init(&dbankLock, 0);
    w.pqLock = &pqLock;
    w.pq2Lock = &pq2Lock;
    w.pq3Lock = &pq3Lock;
    w.exitLock = &exitLock;
    w.thresLock = &thresLock;
    w.outputLock = &outputLock;
    w.dbankLock = &dbankLock;
    printf("Lock initialized.\n");
    w.goal = goal;
    w.dbank = dbank;
    w.pq = pq;
    w.pq2 = pq2;
    w.pq3 = pq3;
    w.exiting = &exiting;
    w.min = &min;
    w.max = &max;
    w.debug = debug;
    w.interact = interact;
    w.dOutput = 0;
    w.thres = d->h;
    printf("Workspace set up.\n");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    th = (pthread_t *)malloc(sizeof(*th) * nthreads);
    printf("Creating thread...\n");
restart:
    pthread_create(th, &attr, doWork, &w);
    w.firstThread = th[0];
    sleep(1);
    for (i = 1; i < nthreads; i += 1)
    {
        pthread_create(th + i, &attr, doWork, &w);
    }
    for (i = 0; i < nthreads; i += 1)
    {
        pthread_join(th[i], NULL);
    }

    if (w.dOutput)
    {
        d = w.dOutput;
        if (debug)
            printPlate(d->p, stdout);
        printf("\nSolution found (In %.3lf seconds).\n\n", (clock() - t) / (double)CLOCKS_PER_SEC);
        D1ToD2(FindInPlate(input, 0), x, x + 1);
        if (interact == 0)
        {
            printf("%u steps\n\n", (w.dOutput)->nparents);
            printPlate(input, stdout);
            h = (w.CalcDis)(input, goal, buffers, &b);
            e = (w.dOutput)->nparents + h;
            printf("(%u/%u) h = %u, e = %u, b = %u\n", 0, (w.dOutput)->nparents, h, e, b);
            printf("\n");
            for (i = 0; i < (w.dOutput)->nparents; i += 1)
            {
                move(input, ((w.dOutput)->parent)[i], x);
                printPlate(input, stdout);
                h = (w.CalcDis)(input, goal, buffers, &b);
                e = (w.dOutput)->nparents + h - i - 1;
                printf("(%u/%u) h = %u, e = %u, b = %u\n", i, (w.dOutput)->nparents, h, e, b);
                printf("\n");
            }
        }
        else
        {
            printf("%u steps\n\n", (w.dOutput)->nparents);
            printPlate(input, stdout);
            h = (w.CalcDis)(input, goal, buffers, &b);
            e = (w.dOutput)->nparents + h;
            printf("(%u/%u) h = %u, e = %u, b = %u\n", 0, (w.dOutput)->nparents, h, e, b);
            fgets(buf, sizeof(buf), stdin);
            for (i = 0; i < (w.dOutput)->nparents; i += 1)
            {
                move(input, ((w.dOutput)->parent)[i], x);
                printPlate(input, stdout);
                h = (w.CalcDis)(input, goal, buffers, &b);
                e = (w.dOutput)->nparents + h - i - 1;
                printf("(%u/%u) h = %u, e = %u, b = %u\n", i, (w.dOutput)->nparents, h, e, b);
                fgets(buf, sizeof(buf), stdin);
            }
        }
    }
    else
    {
        w.thres += PUZZLE_SIZE;

        w.pq = pq2;
        w.pq2 = pq;
        pq = w.pq;
        pq2 = w.pq2;

        DecisionBankClearUp(dbank, w.pq3);
        DestroyPQ(w.pq3);
        pq3 = w.pq3 = CreatePQ();
        goto restart;
    }

    printf("\nAll threads finished.\n");
    free(th);
    pthread_attr_destroy(&attr);
    pthread_rwlock_destroy(&pqLock);
    pthread_rwlock_destroy(&pq2Lock);
    pthread_rwlock_destroy(&pq3Lock);
    pthread_rwlock_destroy(&exitLock);
    pthread_rwlock_destroy(&thresLock);
    pthread_mutex_destroy(&outputLock);
    pthread_mutex_destroy(&dbankLock);
    //printf("DestroyPQ(pq).\n");
    DestroyPQ(pq);
    DestroyPQ(pq2);
    DestroyPQ(pq3);
    //printf("DecisionBankDestory(dbank).\n");
    DecisionBankDestory(dbank);
    free(buffers);
    return 0;
}
