#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "avl.h"
#include "worker.h"
#include "stack.h"
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
    pthread_rwlock_t platesLock;
    pthread_rwlock_t exitLock;
    pthread_mutex_t outputLock;
    unsigned int *buffers[2];
    PLATE goal;
    PLATE input;
    pthread_t *th;
    DESICISON *d;
    DBank *dbank = CreateDecisionBank();
    AVL_TREE *pq = CreatePQ();
    AVL_TREE *plates = CreatePlates();
    STACK *stack = CreateStack();
    DESICISON *cur;
    clock_t t;
    unsigned int i, min, max, nthreads = 1;
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
            if (LEVEL > 16)
                LEVEL = 16;
            argci += 1;
        }

        if (strcmp("noninteract", argv[argci]) == 0)
        {
            interact = 0;
        }
    }

    printf("SIZE = %u\n", PUZZLE_SIZE);
    printf("THREADS = %u\n", nthreads);
    printf("LEVEL = %u\n", LEVEL);
    printf("Input puzzle for GOAL:\n");
    if ((r = GetPlate(&goal, stdin)))
        return r;
    printPlate(&goal, stdout);
    printf("Input puzzle for CURRENT:\n");
    if ((r = GetPlate(&input, stdin)))
        return r;
    printPlate(&input, stdout);
    printf("\n");

    buffers[0] = (unsigned int *)malloc(sizeof(**buffers) * PUZZLE_SIZE * PUZZLE_SIZE);
    buffers[1] = (unsigned int *)malloc(sizeof(**buffers) * PUZZLE_SIZE * PUZZLE_SIZE);
    t = clock();
    d = DecisionBankAdd(dbank);
    memcpy(d->p->s, input.s, PUZZLE_SIZE * PUZZLE_SIZE);
    d->h = CalcDis(d->p, &goal, buffers);
    AddPlate(plates, d->p);
    EnqueuePQ(pq, d);
    free(buffers[0]);
    free(buffers[1]);

    max = min = d->h;
    pthread_rwlock_init(&pqLock, 0);
    pthread_rwlock_init(&platesLock, 0);
    pthread_rwlock_init(&exitLock, 0);
    pthread_mutex_init(&outputLock, 0);
    w.pqLock = &pqLock;
    w.platesLock = &platesLock;
    w.exitLock = &exitLock;
    w.outputLock = &outputLock;
    printf("Lock initialized.\n");
    w.goal = &goal;
    w.dbank = dbank;
    w.pq = pq;
    w.plates = plates;
    w.exiting = &exiting;
    w.min = &min;
    w.max = &max;
    w.debug = debug;
    w.interact = interact;
    w.dOutput = 0;
    printf("Workspace set up.\n");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    th = (pthread_t *)malloc(sizeof(*th) * nthreads);
    printf("Creating thread...\n");
    for (i = 0; i < nthreads; i += 1)
    {
        pthread_create(th + i, &attr, doWork, &w);
        if (i == 0)
            w.firstThread = th[i];
    }
    for (i = 0; i < nthreads; i += 1)
    {
        pthread_join(th[i], NULL);
    }
    free(th);
    printf("\nAll threads finished.\n");
    pthread_attr_destroy(&attr);
    pthread_rwlock_destroy(&pqLock);
    pthread_rwlock_destroy(&platesLock);
    pthread_rwlock_destroy(&exitLock);
    pthread_mutex_destroy(&outputLock);

    if (w.dOutput)
    {
        d = w.dOutput;
        if (debug)
            printPlate(d->p, stdout);
        printf("Solution found (In %.3lf seconds).\n\n", (clock() - t) / (double)CLOCKS_PER_SEC);
        cur = d;
        while (cur)
        {
            StackAdd(stack, cur);
            cur = cur->parent;
        }
        if (interact == 0)
        {
            printf("%u steps\n\n", stack->n);
            for (i = stack->n - 1; i < stack->n; i -= 1)
            {
                printPlate((*((stack->base) + i))->p, stdout);
                printf("\n");
            }
        }
        else
        {
            printf("%u steps\n\n", stack->n);
            for (i = stack->n - 1; i < stack->n; i -= 1)
            {
                printPlate((*((stack->base) + i))->p, stdout);
                if (interact)
                    fgets(buf, sizeof(buf), stdin);
            }
        }
    }
    else
    {
        printf("Solution not found.\n");
    }
    //printf("free(stack->base).\n");
    free(stack->base);
    //printf("DestroyPlates(plates).\n");
    DestroyPlates(plates);
    //printf("DestroyPQ(pq).\n");
    DestroyPQ(pq);
    //printf("DecisionBankDestory(dbank).\n");
    DecisionBankDestory(dbank);
    return 0;
}
