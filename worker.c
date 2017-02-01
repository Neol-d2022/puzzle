#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "worker.h"
#include "main.h"
#include "util.h"
#include "core.h"

void *doWork(void *arg)
{
    char buf[256];
    unsigned char *buffers;
    DESICISON *d, *next, *children[4];
    DBank *dbank;
    AVL_TREE *pq, *pq2, *pq3;
    WORKSPACE *w;
    unsigned char *findResult, *plate;
    size_t s;
    pthread_t self = pthread_self();
    unsigned int i, b, rc[2], idle, h, nparents, updater, chId, lastMove;
    int dequeued, r, exiting;

    w = (WORKSPACE *)arg;
    dbank = w->dbank;
    pq = w->pq;
    pq2 = w->pq2;
    pq3 = w->pq3;

    updater = idle = 0;
    plate = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    buffers = (unsigned char *)malloc(GetBufSize());
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
                printf("(%i) Exiting (Flag)\n\n", (int)pthread_self());
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
            printf("(%i) Dequeued(h=%u n=%u).\n\n", (int)pthread_self(), d->h, d->nparents);
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
                    s = printf("minD=%u maxH=%u thres=%u err=%u Q1=%u Q2=%u Dsize=%u dis=%u steps=%u\n", *(w->min), *(w->max), w->thres, d->h + d->nparents, pq->count, pq2->count, dbank->count, d->h, d->nparents);
                }
                else
                {
                    s = printf("minD=%u maxH=%u thres=%u err=%u Q1=%u Q2=%u Dsize=%u dis=%u steps=%u ", *(w->min), *(w->max), w->thres, d->h + d->nparents, pq->count, pq2->count, dbank->count, d->h, d->nparents);
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
                printf("(%i) Exiting (Found Solution)\n\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }
            break;
        }

        i = FindInPlate(d->p, 0);
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
            printf("(%i) i=%u x=[%u,%u] f=%u\n", (int)pthread_self(), i, rc[0], rc[1], r);
            pthread_mutex_unlock(w->outputLock);
        }
        chId = 0;
        if (d->nparents)
            lastMove = ((d->parent)[d->nparents - 1]);
        else
            lastMove = 0;

        // Up
        if (r & 0x8)
        {
            memcpy(plate, d->p, PUZZLE_SIZE * PUZZLE_SIZE);
            swap(plate + rc[0] * PUZZLE_SIZE + rc[1], plate + (rc[0] - 1) * PUZZLE_SIZE + rc[1]);
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated U.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            h = (w->CalcDis)(plate, w->goal, buffers, &b);
            nparents = d->nparents + 1;

            if (lastMove != 0x4)
            {
                while (pthread_rwlock_trywrlock(w->dbankLock))
                    ;
                next = DecisionBankAdd(dbank);
                pthread_rwlock_unlock(w->dbankLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->h = h;
                next->b = b;
                AddParent(next, d, 0x8);
                children[chId] = next;
                chId += 1;
                if (w->debug)
                {
                    while (pthread_mutex_trylock(w->outputLock))
                        ;
                    printf("(%i) Accepted (h=%u n=%u).\n\n", (int)pthread_self(), next->h, next->nparents);
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
            else
            {
                if (w->debug)
                {
                    while (pthread_mutex_trylock(w->outputLock))
                        ;
                    printf("(%i) Rejected (h=%u n=%u).\n\n", (int)pthread_self(), h, nparents);
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
        }

        // Down
        if (r & 0x4)
        {
            memcpy(plate, d->p, PUZZLE_SIZE * PUZZLE_SIZE);
            swap(plate + rc[0] * PUZZLE_SIZE + rc[1], plate + (rc[0] + 1) * PUZZLE_SIZE + rc[1]);
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated D.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            h = (w->CalcDis)(plate, w->goal, buffers, &b);
            nparents = d->nparents + 1;

            if (lastMove != 0x8)
            {
                while (pthread_rwlock_trywrlock(w->dbankLock))
                    ;
                next = DecisionBankAdd(dbank);
                pthread_rwlock_unlock(w->dbankLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->h = h;
                next->b = b;
                AddParent(next, d, 0x4);
                children[chId] = next;
                chId += 1;
                if (w->debug)
                {
                    while (pthread_mutex_trylock(w->outputLock))
                        ;
                    printf("(%i) Accepted (h=%u n=%u).\n\n", (int)pthread_self(), next->h, next->nparents);
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
            else
            {
                if (w->debug)
                {
                    while (pthread_mutex_trylock(w->outputLock))
                        ;
                    printf("(%i) Rejected (h=%u n=%u).\n\n", (int)pthread_self(), h, nparents);
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
        }

        // Left
        if (r & 0x2)
        {
            memcpy(plate, d->p, PUZZLE_SIZE * PUZZLE_SIZE);
            swap(plate + rc[0] * PUZZLE_SIZE + rc[1], plate + rc[0] * PUZZLE_SIZE + (rc[1] - 1));
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated L.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            h = (w->CalcDis)(plate, w->goal, buffers, &b);
            nparents = d->nparents + 1;

            if (lastMove != 0x1)
            {
                while (pthread_rwlock_trywrlock(w->dbankLock))
                    ;
                next = DecisionBankAdd(dbank);
                pthread_rwlock_unlock(w->dbankLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->h = h;
                next->b = b;
                AddParent(next, d, 0x2);
                children[chId] = next;
                chId += 1;
                if (w->debug)
                {
                    while (pthread_mutex_trylock(w->outputLock))
                        ;
                    printf("(%i) Accepted (h=%u n=%u).\n\n", (int)pthread_self(), next->h, next->nparents);
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
            else
            {
                if (w->debug)
                {
                    while (pthread_mutex_trylock(w->outputLock))
                        ;
                    printf("(%i) Rejected (h=%u n=%u).\n\n", (int)pthread_self(), h, nparents);
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
        }

        // Right
        if (r & 0x1)
        {
            memcpy(plate, d->p, PUZZLE_SIZE * PUZZLE_SIZE);
            swap(plate + rc[0] * PUZZLE_SIZE + rc[1], plate + rc[0] * PUZZLE_SIZE + (rc[1] + 1));
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated R.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            h = (w->CalcDis)(plate, w->goal, buffers, &b);
            nparents = d->nparents + 1;

            if (lastMove != 0x2)
            {
                while (pthread_rwlock_trywrlock(w->dbankLock))
                    ;
                next = DecisionBankAdd(dbank);
                pthread_rwlock_unlock(w->dbankLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->h = h;
                next->b = b;
                AddParent(next, d, 0x1);
                children[chId] = next;
                chId += 1;
                if (w->debug)
                {
                    while (pthread_mutex_trylock(w->outputLock))
                        ;
                    printf("(%i) Accepted (h=%u n=%u).\n\n", (int)pthread_self(), next->h, next->nparents);
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
            else
            {
                if (w->debug)
                {
                    while (pthread_mutex_trylock(w->outputLock))
                        ;
                    printf("(%i) Rejected (h=%u n=%u).\n\n", (int)pthread_self(), h, nparents);
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
        }

        for (i = 0; i < chId; i += 1)
        {
            while (pthread_rwlock_tryrdlock(w->thresLock))
                ;
            if ((children[i])->h + (children[i])->nparents <= w->thres)
            {
                while (pthread_rwlock_trywrlock(w->pqLock))
                    ;
                EnqueuePQ(pq, children[i]);
                pthread_rwlock_unlock(w->pqLock);
            }
            else
            {
                while (pthread_rwlock_trywrlock(w->pq2Lock))
                    ;
                EnqueuePQ(pq2, children[i]);
                pthread_rwlock_unlock(w->pq2Lock);
            }
            pthread_rwlock_unlock(w->thresLock);
        }

        while (pthread_rwlock_trywrlock(w->pq3Lock))
            ;
        EnqueuePQ(pq3, d);
        pthread_rwlock_unlock(w->pq3Lock);

        if (self == w->firstThread)
        {
            while (pthread_rwlock_tryrdlock(w->pq3Lock))
                ;
            if (pq3->count >= 0xFFFF * PUZZLE_SIZE)
            {
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
    free(buffers);
    return 0;
}
