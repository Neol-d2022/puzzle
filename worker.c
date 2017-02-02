#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "worker.h"
#include "main.h"
#include "util.h"
#include "core.h"
#include "plates.h"

static int NewMove(unsigned char *buffers, char *buf, unsigned char **plate, DESICISON *d, WORKSPACE *w, DESICISON **children, unsigned int *chId, pthread_t self, unsigned int i, unsigned int j, unsigned int dir)
{
    unsigned char *findResult;
    DESICISON *next;
    unsigned int h, nparents, b;

    swap((*plate) + i, (*plate) + j);
    if (w->debug)
    {
        while (pthread_mutex_trylock(w->outputLock))
            ;
        printPlate(*plate, stdout);
        printf("(%u) Next Desicion generated %u.\n", (unsigned int)self, dir);
        pthread_mutex_unlock(w->outputLock);
    }

    h = (w->CalcDis)(*plate, w->goal, buffers, &b);
    nparents = d->nparents + 1;

    while (pthread_rwlock_tryrdlock(w->platesLock))
        ;
    findResult = FindPlate(w->plates, *plate);
    if (findResult)
        if (*(unsigned int *)(findResult + PUZZLE_SIZE * PUZZLE_SIZE) <= nparents)
            findResult = 0;
    pthread_rwlock_unlock(w->platesLock);

    if (findResult == 0)
    {
        while (pthread_rwlock_trywrlock(w->dbankLock))
            ;
        next = DecisionBankAdd(w->dbank);
        pthread_rwlock_unlock(w->dbankLock);
        memcpy(next->p, *plate, PUZZLE_SIZE * PUZZLE_SIZE);
        next->h = h;
        next->b = b;
        AddParent(next, d, dir);
        children[*chId] = next;
        (*chId) += 1;
        if (w->debug)
        {
            while (pthread_mutex_trylock(w->outputLock))
                ;
            printf("(%u) Accepted (h=%u n=%u).\n\n", (unsigned int)self, next->h, next->nparents);
            if (w->interact && w->firstThread == self)
                fgets(buf, sizeof(buf), stdin);
            pthread_mutex_unlock(w->outputLock);
        }
        return 0;
    }
    else
    {
        if (w->debug)
        {
            while (pthread_mutex_trylock(w->outputLock))
                ;
            printf("(%u) Rejected (h=%u n=%u).\n\n", (unsigned int)self, h, nparents);
            if (w->interact && w->firstThread == self)
                fgets(buf, sizeof(buf), stdin);
            pthread_mutex_unlock(w->outputLock);
        }
        return 1;
    }
}

static void MakeMoves(unsigned char *buffers, char *buf, unsigned char **plate, DESICISON *d, WORKSPACE *w, DESICISON **children, unsigned int *chId, pthread_t self, unsigned int i, unsigned int level, unsigned int lastDir)
{
    unsigned char *o;
    DESICISON *_d;
    unsigned int rc[2], r;

    if (level == 0)
        return;
    o = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);

    D1ToD2(i, rc, rc + 1);
    r = 0;
    if (rc[0] == 0)
    {
        r |= 0x4; //Down
    }
    else if (rc[0] == PUZZLE_SIZE - 1)
    {
        r |= 0x8; //Up
    }
    else
    {
        r |= 0xC; //Up & Down
    }

    if (rc[1] == 0)
    {
        r |= 0x1; //Right
    }
    else if (rc[1] == PUZZLE_SIZE - 1)
    {
        r |= 0x2; //Left
    }
    else
    {
        r |= 0x3; //Right & Left
    }
    if (w->debug)
    {
        while (pthread_mutex_trylock(w->outputLock))
            ;
        printf("(%u) i=%u x=[%u,%u] f=%u l=%u\n", (unsigned int)self, i, rc[0], rc[1], r, level);
        pthread_mutex_unlock(w->outputLock);
    }

    memcpy(o, *plate, PUZZLE_SIZE * PUZZLE_SIZE);
    // Up
    if (r & 0x8)
    {
        if (NewMove(buffers, buf, plate, d, w, children, chId, self, i, i - PUZZLE_SIZE, 0x8) == 0)
        {
            if (lastDir != 0x4)
            {
                _d = children[(*chId) - 1];
                if (_d->h + d->nparents > d->h + d->nparents)
                {
                    if (level > 2)
                        MakeMoves(buffers, buf, plate, _d, w, children, chId, self, i - PUZZLE_SIZE, level - 2, 0x8);
                }
                else
                    MakeMoves(buffers, buf, plate, _d, w, children, chId, self, i - PUZZLE_SIZE, level - 1, 0x8);
            }
        }
        memcpy(*plate, o, PUZZLE_SIZE * PUZZLE_SIZE);
    }

    // Down
    if (r & 0x4)
    {
        if (NewMove(buffers, buf, plate, d, w, children, chId, self, i, i + PUZZLE_SIZE, 0x4) == 0)
        {
            if (lastDir != 0x8)
            {
                _d = children[(*chId) - 1];
                if (_d->h + d->nparents > d->h + d->nparents)
                {
                    if (level > 2)
                        MakeMoves(buffers, buf, plate, _d, w, children, chId, self, i + PUZZLE_SIZE, level - 2, 0x4);
                }
                else
                    MakeMoves(buffers, buf, plate, _d, w, children, chId, self, i + PUZZLE_SIZE, level - 1, 0x4);
            }
        }
        memcpy(*plate, o, PUZZLE_SIZE * PUZZLE_SIZE);
    }

    // Left
    if (r & 0x2)
    {
        if (NewMove(buffers, buf, plate, d, w, children, chId, self, i, i - 1, 0x2) == 0)
        {
            if (lastDir != 0x1)
            {
                _d = children[(*chId) - 1];
                if (_d->h + d->nparents > d->h + d->nparents)
                {
                    if (level > 2)
                        MakeMoves(buffers, buf, plate, _d, w, children, chId, self, i - 1, level - 2, 0x2);
                }
                else
                    MakeMoves(buffers, buf, plate, _d, w, children, chId, self, i - 1, level - 1, 0x2);
            }
        }
        memcpy(*plate, o, PUZZLE_SIZE * PUZZLE_SIZE);
    }

    // Right
    if (r & 0x1)
    {
        if (NewMove(buffers, buf, plate, d, w, children, chId, self, i, i + 1, 0x1) == 0)
        {
            if (lastDir != 0x2)
            {
                _d = children[(*chId) - 1];
                if (_d->h + d->nparents > d->h + d->nparents)
                {
                    if (level > 2)
                        MakeMoves(buffers, buf, plate, _d, w, children, chId, self, i + 1, level - 2, 0x1);
                }
                else
                    MakeMoves(buffers, buf, plate, _d, w, children, chId, self, i + 1, level - 1, 0x1);
            }
        }
        memcpy(*plate, o, PUZZLE_SIZE * PUZZLE_SIZE);
    }

    free(o);
}

void *doWork(void *arg)
{
    char buf[256];
    unsigned char *buffers;
    DESICISON *d, **children;
    DBank *dbank;
    AVL_TREE *pq, *pq2, *pq3;
    WORKSPACE *w;
    unsigned char *plate, *o;
    size_t s;
    pthread_t self = pthread_self();
    unsigned int i, idle, updater, chId;
    int dequeued, exiting;

    w = (WORKSPACE *)arg;
    dbank = w->dbank;
    pq = w->pq;
    pq2 = w->pq2;
    pq3 = w->pq3;

    updater = idle = 0;
    plate = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    o = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    buffers = (unsigned char *)malloc(GetBufSize());
    children = (DESICISON **)malloc(sizeof(*children) * (4 * (unsigned int)pow(3, (PUZZLE_SIZE >> 1) + 1)));
    while (1)
    {
        while (pthread_rwlock_tryrdlock(w->exitLock))
            ;
        exiting = *(w->exiting);
        pthread_rwlock_unlock(w->exitLock);
        if (exiting)
        {
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printf("(%u) Exiting (Flag)\n\n", (unsigned int)self);
                pthread_mutex_unlock(w->outputLock);
            }
            break;
        }

        while (pthread_rwlock_trywrlock(w->pqLock))
            ;
        dequeued = DequeuePQ(pq, &d);
        pthread_rwlock_unlock(w->pqLock);
        if (dequeued)
        {
            idle += 1;
            if (idle >= 2)
                break;
            else
            {
                sleep(1);
                continue;
            }
        }
        else
            idle = 0;

        if (w->debug)
        {
            while (pthread_mutex_trylock(w->outputLock))
                ;
            printPlate(d->p, stdout);
            printf("(%u) Dequeued(h=%u n=%u).\n\n", (unsigned int)self, d->h, d->nparents);
            if (w->interact)
                fgets(buf, sizeof(buf), stdin);
            pthread_mutex_unlock(w->outputLock);
        }

        if (self == w->firstThread && w->interact)
        {
            if (*(w->min) > d->h)
                *(w->min) = d->h;
            if (d->h + d->npmax > *(w->max))
                *(w->max) = d->h + d->npmax;

            {
                while (pthread_rwlock_tryrdlock(w->thresLock))
                    ;
                while (pthread_rwlock_tryrdlock(w->pqLock))
                    ;
                while (pthread_rwlock_tryrdlock(w->pq2Lock))
                    ;
                while (pthread_rwlock_tryrdlock(w->dbankLock))
                    ;
                while (pthread_mutex_trylock(w->outputLock))
                    ;

                if (w->debug)
                {
                    s = printf("minD=%u maxH=%u c=%u thres=%u err=%u Q1=%u Q2=%u Dsize=%u dis=%u steps=%u\n", *(w->min), *(w->max), w->tests, w->thres, d->h + d->nparents, pq->count, pq2->count, dbank->count, d->h, d->nparents);
                }
                else
                {
                    s = printf("minD=%u maxH=%u c=%u thres=%u err=%u Q1=%u Q2=%u Dsize=%u dis=%u steps=%u ", *(w->min), *(w->max), w->tests, w->thres, d->h + d->nparents, pq->count, pq2->count, dbank->count, d->h, d->nparents);
                    memset(buf, '\b', s);
                    buf[s] = '\0';
                    printf("%s", buf);
                }

                pthread_mutex_unlock(w->outputLock);
                pthread_rwlock_unlock(w->dbankLock);
                pthread_rwlock_unlock(w->pq2Lock);
                pthread_rwlock_unlock(w->pqLock);
                pthread_rwlock_unlock(w->thresLock);
            }
            updater += 1;
        }

        if (d->h == 0)
        {
            pthread_rwlock_wrlock(w->exitLock);
            if (*(w->exiting) == 0)
            {
                *(w->exiting) = 1;
                w->dOutput = d;
            }
            pthread_rwlock_unlock(w->exitLock);
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printf("(%u) Exiting (Found Solution)\n\n", (unsigned int)self);
                pthread_mutex_unlock(w->outputLock);
            }
            break;
        }

        chId = 0;
        i = FindInPlate(d->p, 0);
        memcpy(plate, d->p, PUZZLE_SIZE * PUZZLE_SIZE);
        MakeMoves(buffers, buf, &plate, d, w, children, &chId, self, i, (PUZZLE_SIZE >> 1) + 1, 0);

        for (i = 0; i < chId; i += 1)
        {
            while (pthread_rwlock_trywrlock(w->platesLock))
                ;
            if (AddPlate(w->plates, (children[i])->p, (children[i])->nparents) == 0)
            {
                pthread_rwlock_unlock(w->platesLock);
                while (pthread_rwlock_tryrdlock(w->thresLock))
                    ;
                if ((children[i])->h + (children[i])->nparents <= w->thres)
                {
                    pthread_rwlock_unlock(w->thresLock);
                    while (pthread_rwlock_trywrlock(w->pqLock))
                        ;
                    EnqueuePQ(pq, children[i]);
                    pthread_rwlock_unlock(w->pqLock);
                }
                else
                {
                    pthread_rwlock_unlock(w->thresLock);
                    while (pthread_rwlock_trywrlock(w->pq2Lock))
                        ;
                    EnqueuePQ(pq2, children[i]);
                    pthread_rwlock_unlock(w->pq2Lock);
                }
            }
            else
            {
                pthread_rwlock_unlock(w->platesLock);
                while (pthread_rwlock_trywrlock(w->dbankLock))
                    ;
                AVL_Delete(dbank, children[i]);
                DeInitD(children[i]);
                pthread_rwlock_unlock(w->dbankLock);
            }
        }
        while (pthread_rwlock_trywrlock(w->pq3Lock))
            ;
        EnqueuePQ(pq3, d);
        pthread_rwlock_unlock(w->pq3Lock);

        if (self == w->firstThread)
        {
            while (pthread_rwlock_tryrdlock(w->pq3Lock))
                ;
            if (pq3->count >= 0xFFF * PUZZLE_SIZE)
            {
                w->tests += (w->pq3)->count;
                pthread_rwlock_unlock(w->pq3Lock);
                while (pthread_rwlock_trywrlock(w->pq3Lock))
                    ;
                while (pthread_rwlock_trywrlock(w->dbankLock))
                    ;
                DecisionBankClearUp(dbank, w->pq3);
                pthread_rwlock_unlock(w->dbankLock);
            }
            pthread_rwlock_unlock(w->pq3Lock);
        }
    }

    free(plate);
    free(o);
    free(buffers);
    free(children);
    return 0;
}
