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
    unsigned char *plate;
    DESICISON *d, *next, *children[4];
    DBank *dbank;
    AVL_TREE *pq, *pq2, *pq3;
    AVL_TREE *plates;
    WORKSPACE *w;
    unsigned char *findResult;
    size_t s;
    pthread_t self = pthread_self();
    unsigned int i, rc[2], idle, h, nparents, updater, total, bad, chId;
    int dequeued, r, exiting;

    w = (WORKSPACE *)arg;
    dbank = w->dbank;
    pq = w->pq;
    pq2 = w->pq2;
    pq3 = w->pq3;
    plates = w->plates;

    updater = idle = 0;
    plate = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
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
        bad = total = 0;
        if (rc[0] == 0)
        {
            r |= 0x4; //Down
            total += 1;
        }
        else if (rc[0] == PUZZLE_SIZE - 1)
        {
            r |= 0x8; //Up
            total += 1;
        }
        else
        {
            r |= 0xC; //Up & Down
            total += 2;
        }

        if (rc[1] == 0)
        {
            r |= 0x1; //Right
            total += 1;
        }
        else if (rc[1] == PUZZLE_SIZE - 1)
        {
            r |= 0x2; //Left
            total += 1;
        }
        else
        {
            r |= 0x3; //Right & Left
            total += 2;
        }
        if (w->debug)
        {
            while (pthread_mutex_trylock(w->outputLock))
                ;
            printf("(%i) i=%u x=[%u,%u] f=%u\n", (int)pthread_self(), i, rc[0], rc[1], r);
            pthread_mutex_unlock(w->outputLock);
        }
        chId = 0;

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

            h = (w->CalcDis)(plate, w->goal, buffers);
            nparents = d->nparents + 1;
            if (h + nparents > d->h + d->nparents)
                bad += 1;

            while (pthread_rwlock_tryrdlock(w->platesLock))
                ;
            findResult = FindPlate(plates, plate);
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult)
            {
                while (pthread_rwlock_trywrlock(w->platesLock))
                    ;
                next = DecisionBankAdd(dbank);
                AddPlate(plates, plate);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->h = h;
                next->parent = d;
                next->nparents = nparents;
                d->ref += 1;
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

            h = (w->CalcDis)(plate, w->goal, buffers);
            nparents = d->nparents + 1;
            if (h + nparents > d->h + d->nparents)
                bad += 1;

            while (pthread_rwlock_tryrdlock(w->platesLock))
                ;
            findResult = FindPlate(plates, plate);
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult)
            {
                while (pthread_rwlock_trywrlock(w->platesLock))
                    ;
                next = DecisionBankAdd(dbank);
                AddPlate(plates, plate);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->h = h;
                next->parent = d;
                next->nparents = nparents;
                d->ref += 1;
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

            h = (w->CalcDis)(plate, w->goal, buffers);
            nparents = d->nparents + 1;
            if (h + nparents > d->h + d->nparents)
                bad += 1;

            while (pthread_rwlock_tryrdlock(w->platesLock))
                ;
            findResult = FindPlate(plates, plate);
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult)
            {
                while (pthread_rwlock_trywrlock(w->platesLock))
                    ;
                next = DecisionBankAdd(dbank);
                AddPlate(plates, plate);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->h = h;
                next->parent = d;
                next->nparents = nparents;
                d->ref += 1;
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

            h = (w->CalcDis)(plate, w->goal, buffers);
            nparents = d->nparents + 1;
            if (h + nparents > d->h + d->nparents)
                bad += 1;

            while (pthread_rwlock_tryrdlock(w->platesLock))
                ;
            findResult = FindPlate(plates, plate);
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult)
            {
                while (pthread_rwlock_trywrlock(w->platesLock))
                    ;
                next = DecisionBankAdd(dbank);
                AddPlate(plates, plate);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate;
                plate = next->p;
                next->p = findResult;
                next->h = h;
                next->parent = d;
                next->nparents = nparents;
                d->ref += 1;
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
        }

        while (pthread_rwlock_trywrlock(w->pq3Lock))
            ;
        EnqueuePQ(pq3, d);
        pthread_rwlock_unlock(w->pq3Lock);
    }

    free(plate);
    free(buffers[0]);
    free(buffers[1]);
    return 0;
}
