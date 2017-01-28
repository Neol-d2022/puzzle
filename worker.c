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
    unsigned int *buffers[2];
    PLATE *plate;
    DESICISON *d, *next;
    DBank *dbank;
    AVL_TREE *pq;
    AVL_TREE *plates;
    WORKSPACE *w;
    PLATE *findResult;
    size_t s;
    pthread_t self = pthread_self();
    unsigned int i, rc[2], idle, h, nparents, updater;
    int dequeued, r, exiting, plateBetter;

    w = (WORKSPACE *)arg;
    dbank = w->dbank;
    pq = w->pq;
    plates = w->plates;

    updater = idle = 0;
    plate = (PLATE *)malloc(sizeof(*plate));
    plate->s = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    buffers[0] = (unsigned int *)malloc(sizeof(**buffers) * PUZZLE_SIZE * PUZZLE_SIZE);
    buffers[1] = (unsigned int *)malloc(sizeof(**buffers) * PUZZLE_SIZE * PUZZLE_SIZE);
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
            if (idle >= 3)
                break;
            else
            {
                sleep(1);
                continue;
            }
        }
        else
            idle = 0;

        if (self == w->firstThread && w->interact)
        {
            if (*(w->min) > d->h)
                *(w->min) = d->h;
            if (d->h > *(w->max))
                *(w->max) = d->h;

            if (w->debug)
            {
                printPlate(d->p, stdout);
                printf("(%i) Dequeued(h=%u n=%u).\n\n", (int)pthread_self(), d->h, d->nparents);
                fgets(buf, sizeof(buf), stdin);
            }

            while (pthread_rwlock_tryrdlock(w->platesLock))
                ;
            if ((updater & 0x3FFF) == 0x3FFF)
            {
                while (pthread_rwlock_tryrdlock(w->pqLock))
                    ;
                while (pthread_mutex_trylock(w->outputLock))
                    ;

                if (w->debug)
                {
                    s = printf("minD=%u maxD=%u err=%u Qsize=%u Psize=%u dis=%u steps=%u\n", *(w->min), *(w->max), d->h + d->nparents, pq->count, plates->count, d->h, d->nparents);
                }
                else
                {
                    s = printf("minD=%u maxD=%u err=%u Qsize=%u Psize=%u dis=%u steps=%u ", *(w->min), *(w->max), d->h + d->nparents, pq->count, plates->count, d->h, d->nparents);
                    memset(buf, '\b', s);
                    buf[s] = '\0';
                    printf("%s", buf);
                }

                pthread_mutex_unlock(w->outputLock);
                pthread_rwlock_unlock(w->pqLock);
            }
            pthread_rwlock_unlock(w->platesLock);
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

        // Up
        if (r & 0x8)
        {
            memcpy(plate->s, d->p->s, PUZZLE_SIZE * PUZZLE_SIZE);
            swap(plate->s + rc[0] * PUZZLE_SIZE + rc[1], plate->s + (rc[0] - 1) * PUZZLE_SIZE + rc[1]);
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated U.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            h = CalcDis(plate, w->goal, buffers);
            nparents = d->nparents + 1;

            plateBetter = 0;
            while (pthread_rwlock_tryrdlock(w->platesLock))
                ;
            findResult = FindPlate(plates, plate);
            if (findResult)
                if (nparents + h < findResult->parent->nparents + findResult->parent->h)
                    plateBetter = 1;
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult || plateBetter)
            {
                while (pthread_rwlock_trywrlock(w->platesLock))
                    ;
                next = DecisionBankAdd(dbank);
                if (plateBetter)
                    findResult->parent = next;
                else
                    AddPlate(plates, plate);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->p->parent = next;
                next->h = h;
                next->parent = d;
                next->nparents = nparents;
                while (pthread_rwlock_trywrlock(w->pqLock))
                    ;
                EnqueuePQ(pq, next);
                pthread_rwlock_unlock(w->pqLock);
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
                    printf("(%i) Rejected.\n\n", (int)pthread_self());
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
        }

        // Down
        if (r & 0x4)
        {
            memcpy(plate->s, d->p->s, PUZZLE_SIZE * PUZZLE_SIZE);
            swap(plate->s + rc[0] * PUZZLE_SIZE + rc[1], plate->s + (rc[0] + 1) * PUZZLE_SIZE + rc[1]);
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated D.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            h = CalcDis(plate, w->goal, buffers);
            nparents = d->nparents + 1;

            plateBetter = 0;
            while (pthread_rwlock_tryrdlock(w->platesLock))
                ;
            findResult = FindPlate(plates, plate);
            if (findResult)
                if (nparents + h < findResult->parent->nparents + findResult->parent->h)
                    plateBetter = 1;
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult || plateBetter)
            {
                while (pthread_rwlock_trywrlock(w->platesLock))
                    ;
                next = DecisionBankAdd(dbank);
                if (plateBetter)
                    findResult->parent = next;
                else
                    AddPlate(plates, plate);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->p->parent = next;
                next->h = h;
                next->parent = d;
                next->nparents = nparents;
                while (pthread_rwlock_trywrlock(w->pqLock))
                    ;
                EnqueuePQ(pq, next);
                pthread_rwlock_unlock(w->pqLock);
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
                    printf("(%i) Rejected.\n\n", (int)pthread_self());
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
        }

        // Left
        if (r & 0x2)
        {
            memcpy(plate->s, d->p->s, PUZZLE_SIZE * PUZZLE_SIZE);
            swap(plate->s + rc[0] * PUZZLE_SIZE + rc[1], plate->s + rc[0] * PUZZLE_SIZE + (rc[1] - 1));
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated L.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            h = CalcDis(plate, w->goal, buffers);
            nparents = d->nparents + 1;

            plateBetter = 0;
            while (pthread_rwlock_tryrdlock(w->platesLock))
                ;
            findResult = FindPlate(plates, plate);
            if (findResult)
                if (nparents + h < findResult->parent->nparents + findResult->parent->h)
                    plateBetter = 1;
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult || plateBetter)
            {
                while (pthread_rwlock_trywrlock(w->platesLock))
                    ;
                next = DecisionBankAdd(dbank);
                if (plateBetter)
                    findResult->parent = next;
                else
                    AddPlate(plates, plate);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->p->parent = next;
                next->h = h;
                next->parent = d;
                next->nparents = nparents;
                while (pthread_rwlock_trywrlock(w->pqLock))
                    ;
                EnqueuePQ(pq, next);
                pthread_rwlock_unlock(w->pqLock);
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
                    printf("(%i) Rejected.\n\n", (int)pthread_self());
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
        }

        // Right
        if (r & 0x1)
        {
            memcpy(plate->s, d->p->s, PUZZLE_SIZE * PUZZLE_SIZE);
            swap(plate->s + rc[0] * PUZZLE_SIZE + rc[1], plate->s + rc[0] * PUZZLE_SIZE + (rc[1] + 1));
            if (w->debug)
            {
                while (pthread_mutex_trylock(w->outputLock))
                    ;
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated R.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            h = CalcDis(plate, w->goal, buffers);
            nparents = d->nparents + 1;

            plateBetter = 0;
            while (pthread_rwlock_tryrdlock(w->platesLock))
                ;
            findResult = FindPlate(plates, plate);
            if (findResult)
                if (nparents + h < findResult->parent->nparents + findResult->parent->h)
                    plateBetter = 1;
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult || plateBetter)
            {
                while (pthread_rwlock_trywrlock(w->platesLock))
                    ;
                next = DecisionBankAdd(dbank);
                if (plateBetter)
                    findResult->parent = next;
                else
                    AddPlate(plates, plate);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->p->parent = next;
                next->h = h;
                next->parent = d;
                next->nparents = nparents;
                while (pthread_rwlock_trywrlock(w->pqLock))
                    ;
                EnqueuePQ(pq, next);
                pthread_rwlock_unlock(w->pqLock);
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
                    printf("(%i) Rejected.\n\n", (int)pthread_self());
                    if (w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
        }
    }

    free(plate->s);
    free(plate);
    free(buffers[0]);
    free(buffers[1]);
    return 0;
}
