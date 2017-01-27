#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "avl.h"

//#define PUZZLE_SIZE 4
//#define DB_MAX (PUZZLE_SIZE * PUZZLE_SIZE * PUZZLE_SIZE)
//#define DB_DROP_FACTOR 0.9

unsigned int DB;
unsigned int PUZZLE_SIZE = 3;

typedef struct
{
    unsigned char *s;
} PLATE;

typedef struct DESICISON_STRUCT
{
    PLATE *p;
    struct DESICISON_STRUCT *parent;
    unsigned int h; //distance to goal
    unsigned int nparents;
    unsigned int preference;
    unsigned int ldis;
} DESICISON;

typedef struct
{
    DESICISON **base;
    unsigned int n;
    unsigned int max;
} DBank;

/*typedef struct
{
    DESICISON **base;
    unsigned int n;
    unsigned int max;
} PQ;*/

/*typedef struct
{
    PLATE *base;
    unsigned int n;
    unsigned int max;
} PLATES;*/

int cmpPlate(void *a, void *b)
{
    PLATE *c, *d;

    c = (PLATE *)a;
    d = (PLATE *)b;

    return memcmp(c->s, d->s, PUZZLE_SIZE * PUZZLE_SIZE);
}

// Notice: REVERSED!!
int cmpPQ(void *a, void *b)
{
    DESICISON *c, *d;

    c = (DESICISON *)a;
    d = (DESICISON *)b;

    if (c->h + c->nparents - c->preference > d->h + d->nparents - d->preference)
        return -1;
    else if (c->h + c->nparents - c->preference < d->h + d->nparents - d->preference)
        return 1;
    else if (c->h + c->nparents > d->h + d->nparents)
        return -1;
    else if (c->h + c->nparents < d->h + d->nparents)
        return 1;
    else if (c->h > d->h)
        return -1;
    else if (c->h < d->h)
        return 1;
    else if (c->nparents > d->nparents)
        return -1;
    else if (c->nparents < d->nparents)
        return 1;
    else
        return cmpPlate(c->p, d->p);
}

AVL_TREE *CreatePQ(void)
{
    /*PQ *ret;
    DESICISON **base;

    ret = (PQ *)malloc(sizeof(*ret));
    base = (DESICISON **)malloc(sizeof(*base) * 8);
    ret->base = base;
    ret->n = 0;
    ret->max = 8;
    return ret;*/
    return AVL_Create(cmpPQ, 0);
}

AVL_TREE *CreatePlates(void)
{
    /*PLATES *ret;
    PLATE *base;

    ret = (PLATES *)malloc(sizeof(*ret));
    base = (PLATE *)malloc(sizeof(*base) * 8);
    ret->base = base;
    ret->n = 0;
    ret->max = 8;
    return ret;*/
    return AVL_Create(cmpPlate, 0);
}

void InitD(DESICISON *d)
{
    PLATE *p;

    p = (PLATE *)malloc(sizeof(*p));
    //memset(p, 0, sizeof(*p));
    p->s = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    memset(p->s, 0, PUZZLE_SIZE * PUZZLE_SIZE);
    d->p = p;
    d->parent = 0;
    d->nparents = 0;
    d->preference = 0;
    d->ldis = 0;
}

void DeInitD(DESICISON *d)
{
    free(d->p);
    //if (d->parent)
    //{
    //    DeInitD(d->parent);
    //    free(d->parent);
    //}
}

//void DestroyPQ(PQ *pq)
void DestroyPQ(AVL_TREE *pq)
{
    //free(pq->base);
    AVL_Destroy(pq);
}

void DestroyPlates(AVL_TREE *plates)
{
    //free(plates->base);
    AVL_Destroy(plates);
}

void AddPlate(AVL_TREE *plates, PLATE *p)
{
    /*if (plates->max == plates->n)
    {
        plates->base = (PLATE *)realloc(plates->base, sizeof(*(plates->base)) * ((plates->max) << 1));
        plates->max <<= 1;
    }

    memcpy(plates->base + plates->n, p, sizeof(*p));
    plates->n += 1;

    qsort(plates->base, plates->n, sizeof(*(plates->base)), cmpPlate);*/
    AVL_Insert(plates, p);
}

PLATE *FindPlate(AVL_TREE *plates, PLATE *key)
{
    //return (PLATE *)bsearch(key, plates->base, plates->n, sizeof(*(plates->base)), cmpPlate);
    return AVL_Retrieve(plates, key);
}

//copy to queue
void EnqueuePQ(AVL_TREE *pq, DESICISON *d)
{
    /*if (pq->max == pq->n)
    {
        pq->base = (DESICISON **)realloc(pq->base, sizeof(*(pq->base)) * ((pq->max) << 1));
        pq->max <<= 1;
    }

    (pq->base)[pq->n] = d;
    pq->n += 1;

    qsort(pq->base, pq->n, sizeof(*(pq->base)), cmpPQ);
    return (DESICISON *)bsearch(d, pq->base, pq->n, sizeof(*(pq->base)), cmpPQ);*/
    AVL_Insert(pq, d);
}

int DequeuePQ(AVL_TREE *pq, DESICISON **d)
{
    NODE *n;

    if (!(pq->root))
        return 1;

    n = pq->root;
    while (n->right)
    {
        n = n->right;
    }

    *d = n->dataPtr;
    AVL_Delete(pq, n->dataPtr);

    return 0;
}

void D1ToD2(unsigned int i, unsigned int *row, unsigned int *col)
{
    *row = i / PUZZLE_SIZE;
    *col = i % PUZZLE_SIZE;
}

void GetMax(unsigned int *max, unsigned int *min)
{
    unsigned int t;

    if (*min > *max)
    {
        t = *max;
        *max = *min;
        *min = t;
    }
}

unsigned int D2Diff(unsigned int i, unsigned int j)
{
    unsigned int x[2], y[2], r;

    D1ToD2(i, x, x + 1);
    D1ToD2(j, y, y + 1);

    r = 0;
    GetMax(x, y);
    r += x[0] - y[0];
    GetMax(x + 1, y + 1);
    r += x[1] - y[1];

    return r;
}

unsigned int CalcDis(PLATE *current, const PLATE *goal)
{
    unsigned int i, j, k;

    k = 0;
    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if ((current->s)[i] == 0)
            continue;
        for (j = 0; j < PUZZLE_SIZE * PUZZLE_SIZE; j += 1)
        {
            if ((current->s)[i] == (goal->s)[j])
            {
                k += D2Diff(i, j);
                break;
            }
        }
        if (j >= PUZZLE_SIZE * PUZZLE_SIZE)
            return -1;
    }

    return k;
}

unsigned int FindInPlate(const PLATE *p, unsigned int chr)
{
    unsigned int i;

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
        if ((p->s)[i] == chr)
            break;
    return i;
}

unsigned int CalcLDis(PLATE *current, const PLATE *goal, unsigned int index)
{
    unsigned int x[2], k, l, m;

    D1ToD2(index, x, x + 1);
    k = 0;
    if(x[0] == 0) {
        l = FindInPlate(current, (goal->s)[index + PUZZLE_SIZE]);
        m = D2Diff(index + PUZZLE_SIZE, l);
        k += m;
    }
    else if(x[0] == PUZZLE_SIZE - 1) {
        l = FindInPlate(current, (goal->s)[index - PUZZLE_SIZE]);
        m = D2Diff(index - PUZZLE_SIZE, l);
        k += m;
    }
    else {
        l = FindInPlate(current, (goal->s)[index + PUZZLE_SIZE]);
        m = D2Diff(index + PUZZLE_SIZE, l);
        k += m;
        l = FindInPlate(current, (goal->s)[index - PUZZLE_SIZE]);
        m = D2Diff(index - PUZZLE_SIZE, l);
        k += m;
    }
    if(x[1] == 0) {
        l = FindInPlate(current, (goal->s)[index + 1]);
        m = D2Diff(index + 1, l);
        k += m;
    }
    else if(x[1] == PUZZLE_SIZE - 1) {
        l = FindInPlate(current, (goal->s)[index - 1]);
        m = D2Diff(index - 1, l);
        k += m;
    }
    else {
        l = FindInPlate(current, (goal->s)[index + 1]);
        m = D2Diff(index + 1, l);
        k += m;
        l = FindInPlate(current, (goal->s)[index - 1]);
        m = D2Diff(index - 1, l);
        k += m;
    }

    return k;
}

void TrimNewline(char *buf)
{
    char *p;

    p = strchr(buf, '\n');
    if (p)
        *p = '\0';
    p = strchr(buf, '\r');
    if (p)
        *p = '\0';
}

void tokenize(const char *str, int sepChr, char ***arr, size_t *size, int ignore)
{
    const char *last, *sep, *end;
    char **a, *subs;
    size_t n, s;

    end = str + strlen(str);
    last = str - 1;
    n = 0;
    sep = str;
    a = (char **)malloc(0);
    while ((sep = strchr(sep, sepChr)))
    {
        if (((size_t)sep == (size_t)(last + 1)) && ignore)
        {
            last = sep;
            sep += 1;
            continue;
        }
        else
        {
            s = (size_t)sep - (size_t)(last + 1);
            subs = (char *)malloc(s + 1);
            subs[s] = '\0';
            memcpy(subs, last + 1, s);
            a = (char **)realloc(a, sizeof(*a) * (n + 1));
            a[n] = subs;
            n += 1;
        }
        last = sep;
        sep += 1;
    }

    if (((size_t)end != (size_t)(last + 1)) || !ignore)
    {
        s = (size_t)end - (size_t)(last + 1);
        subs = (char *)malloc(s + 1);
        subs[s] = '\0';
        memcpy(subs, last + 1, s);
        a = (char **)realloc(a, sizeof(*a) * (n + 1));
        a[n] = subs;
        n += 1;
    }

    *size = n;
    *arr = a;
}

void freeToks(char **toks, size_t n)
{
    size_t i;
    for (i = 0; i < n; i += 1)
        free(toks[i]);
    free(toks);
}

int GetPlate(PLATE *p, FILE *input)
{
    char buf[256], **toks;
    size_t ntoks;
    unsigned int i, j, in;
    p->s = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    for (i = 0; i < PUZZLE_SIZE; i += 1)
    {
        if (!fgets(buf, sizeof(buf), input))
            return 1;
        TrimNewline(buf);

        tokenize(buf, ' ', &toks, &ntoks, 1);
        if (ntoks != PUZZLE_SIZE)
            return 2;

        for (j = 0; j < PUZZLE_SIZE; j += 1)
        {
            sscanf(toks[j], "%u", &in);
            if (in > 0xFF)
            {
                freeToks(toks, ntoks);
                return 3;
            }
            (p->s)[i * PUZZLE_SIZE + j] = (unsigned char)in;
        }

        freeToks(toks, ntoks);
    }
    return 0;
}

void swap(unsigned char *a, unsigned char *b)
{
    unsigned char c = *a;
    *a = *b;
    *b = c;
}

void printPlate(const PLATE *p, FILE *f)
{
    unsigned int i, j;

    for (j = 0; j < PUZZLE_SIZE; j += 1)
    {
        for (i = 0; i < PUZZLE_SIZE - 1; i += 1)
        {
            if ((p->s)[j * PUZZLE_SIZE + i])
                fprintf(f, "%3u ", (unsigned int)((p->s)[j * PUZZLE_SIZE + i]));
            else
                fprintf(f, "    ");
        }

        if ((p->s)[j * PUZZLE_SIZE + PUZZLE_SIZE - 1])
            fprintf(f, "%3u\n", (unsigned int)((p->s)[j * PUZZLE_SIZE + PUZZLE_SIZE - 1]));
        else
            fprintf(f, "   \n");
    }
}

typedef struct
{
    DESICISON **base;
    unsigned int n;
    unsigned int max;
} STACK;

STACK *CreateStack(void)
{
    STACK *ret;
    DESICISON **base;

    ret = (STACK *)malloc(sizeof(*ret));
    base = (DESICISON **)malloc(sizeof(*base) * 8);
    ret->base = base;
    ret->n = 0;
    ret->max = 8;
    return ret;
}

void StackAdd(STACK *stack, DESICISON *p)
{
    if (stack->max == stack->n)
    {
        stack->base = (DESICISON **)realloc(stack->base, sizeof(*(stack->base)) * ((stack->max) << 1));
        stack->max <<= 1;
    }

    memcpy(stack->base + stack->n, &p, sizeof(p));
    stack->n += 1;
}

DBank *CreateDecisionBank(void)
{
    DBank *ret;
    DESICISON **base;

    ret = (DBank *)malloc(sizeof(*ret));
    base = (DESICISON **)malloc(sizeof(*base) * 8);
    ret->base = base;
    ret->n = 0;
    ret->max = 8;
    return ret;
}

DESICISON *DecisionBankAdd(DBank *dbank)
{
    DESICISON *ret;

    if (dbank->max == dbank->n)
    {
        dbank->base = (DESICISON **)realloc(dbank->base, sizeof(*(dbank->base)) * ((dbank->max) << 1));
        dbank->max <<= 1;
    }

    ret = (dbank->base)[dbank->n] = (DESICISON *)malloc(sizeof(*((dbank->base)[dbank->n])));
    dbank->n += 1;
    InitD(ret);
    return ret;
}

void DecisionBankDestory(DBank *dbank)
{
    unsigned int i;

    for (i = 0; i < dbank->n; i += 1)
    {
        DeInitD((dbank->base)[i]);
        free((dbank->base)[i]);
    }

    free(dbank->base);
}

typedef struct {
    PLATE *goal;        //RO
    DBank *dbank;       //RW
    AVL_TREE *pq;       //RW
    AVL_TREE *plates;   //RW
    int *exiting;       //RW
    DESICISON *dOutput; //WO (OUTPUT)
    unsigned int *min;  //RW
    unsigned int *max;  //RW
    pthread_rwlock_t *pqLock;
    pthread_rwlock_t *platesLock;
    pthread_rwlock_t *exitLock;
    pthread_mutex_t *outputLock;
    int debug;
    int interact;
    pthread_t firstThread;
} WORKSPACE;

void* doWork(void *arg) {
    char buf[256];
    PLATE *plate;
    DESICISON *d, *next;
    DBank *dbank;
    AVL_TREE *pq;
    AVL_TREE *plates;
    WORKSPACE *w;
    PLATE *findResult;
    size_t s;
    pthread_t self = pthread_self();
    unsigned int i, rc[2], idle;
    int dequeued, r, exiting;

    w = (WORKSPACE*)arg;
    dbank = w->dbank;
    pq = w->pq;
    plates = w->plates;

    idle = 0;
    plate = (PLATE*)malloc(sizeof(*plate));
    plate->s = (unsigned char*)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    while (1)
    {
        while(pthread_rwlock_tryrdlock(w->exitLock));
        exiting = *(w->exiting);
        pthread_rwlock_unlock(w->exitLock);
        if(exiting) {
            if(w->debug) {
                while(pthread_mutex_trylock(w->outputLock));
                printf("(%i) Exiting (Flag)\n\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }
            break;
        }

        while(pthread_rwlock_trywrlock(w->pqLock));
        dequeued = DequeuePQ(pq, &d);
        pthread_rwlock_unlock(w->pqLock);
        if(dequeued) {
            idle += 1;
            if(idle >= 3) break;
            else {
                sleep(1);
                continue;
            }
        }
        else idle = 0;
        
        if(self == w->firstThread && w->interact) {
            while(pthread_rwlock_tryrdlock(w->platesLock));
            while(pthread_rwlock_tryrdlock(w->pqLock));
            while(pthread_mutex_trylock(w->outputLock));

            if(w->debug) {
                printPlate(d->p, stdout);
                printf("(%i) Dequeued(h=%u l=%u p=%u n=%u).\n\n", (int)pthread_self(), d->h, d->ldis, d->preference, d->nparents);
                fgets(buf, sizeof(buf), stdin);
            }

            if (*(w->min) > d->h)
                *(w->min) = d->h;
            if (d->h > *(w->max))
                *(w->max) = d->h;

            if(w->debug) {
                s = printf("minD=%u maxD=%u Qsize=%u Psize=%u dis=%u ldis=%u pref=%u steps=%u\n", *(w->min), *(w->max), pq->count, plates->count, d->h, d->ldis, d->preference, d->nparents);
            }
            else {
                s = printf("minD=%u maxD=%u Qsize=%u Psize=%u dis=%u ldis=%u pref=%u steps=%u ", *(w->min), *(w->max), pq->count, plates->count, d->h, d->ldis, d->preference, d->nparents);
                memset(buf, '\b', s);
                buf[s] = '\0';
                printf("%s", buf);
            }
            
            pthread_mutex_unlock(w->outputLock);
            pthread_rwlock_unlock(w->pqLock);
            pthread_rwlock_unlock(w->platesLock);
        }

        if (d->h == 0)
        {
            pthread_rwlock_wrlock(w->exitLock);
            if(*(w->exiting) == 0) {
                *(w->exiting) = 1;
                w->dOutput = d;
            }
            pthread_rwlock_unlock(w->exitLock);
            if(w->debug) {
                while(pthread_mutex_trylock(w->outputLock));
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
            while(pthread_mutex_trylock(w->outputLock));
            printf("(%i) i=%u r=%u c=%u f=%u\n", (int)pthread_self(), i, rc[0], rc[1], r);
            pthread_mutex_unlock(w->outputLock);
        }

        //if (d->h + d->nparents < lastH + lastN)
        //{
        //    DB = (unsigned int)(DB * DB_DROP_FACTOR);
        //    if (!DB)
        //        DB = 1;
        //}
        //else if (d->h + d->nparents > lastH + lastN)
        //{
        //    if (DB < DB_MAX - 2)
        //        DB += 2;
        //    else
        //        DB = DB_MAX - 1;
        //}
        //else if (DB < DB_MAX - 1)
        //    DB += 1;

        // Up
        if (r & 0x8)
        {
            memcpy(plate->s, d->p->s, PUZZLE_SIZE * PUZZLE_SIZE);
            swap(plate->s + rc[0] * PUZZLE_SIZE + rc[1], plate->s + (rc[0] - 1) * PUZZLE_SIZE + rc[1]);
            if (w->debug) {
                while(pthread_mutex_trylock(w->outputLock));
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated U.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            while(pthread_rwlock_tryrdlock(w->platesLock));
            findResult = FindPlate(plates, plate);
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult)
            {
                while(pthread_rwlock_trywrlock(w->platesLock));
                AddPlate(plates, plate);
                next = DecisionBankAdd(dbank);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate; plate = next->p; next->p = findResult;
                next->h = CalcDis(next->p, w->goal);
                next->ldis = CalcLDis(next->p, w->goal, i - PUZZLE_SIZE);
                next->parent = d;
                next->nparents = d->nparents + 1;
                if(next->h + next->ldis < d->h + d->ldis) next->preference = d->preference + d->h + d->ldis - next->h - next->ldis;
                else if(d->preference > next->h + next->ldis - d->h - d->ldis + 1) next->preference = d->preference + d->h + d->ldis - next->h - next->ldis - 1;
                while(pthread_rwlock_trywrlock(w->pqLock));
                EnqueuePQ(pq, next);
                pthread_rwlock_unlock(w->pqLock);
                if (w->debug)
                {
                    while(pthread_mutex_trylock(w->outputLock));
                    printf("(%i) Accepted (h=%u l=%u p=%u n=%u).\n\n", (int)pthread_self(), next->h, next->ldis, next->preference, next->nparents);
                    if(w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
            else
            {
                if (w->debug)
                {
                    while(pthread_mutex_trylock(w->outputLock));
                    printf("(%i) Rejected.\n\n", (int)pthread_self());
                    if(w->interact && w->firstThread == self)
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
            if (w->debug) {
                while(pthread_mutex_trylock(w->outputLock));
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated D.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            while(pthread_rwlock_tryrdlock(w->platesLock));
            findResult = FindPlate(plates, plate);
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult)
            {
                while(pthread_rwlock_trywrlock(w->platesLock));
                AddPlate(plates, plate);
                next = DecisionBankAdd(dbank);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate; plate = next->p; next->p = findResult;
                next->h = CalcDis(next->p, w->goal);
                next->ldis = CalcLDis(next->p, w->goal, i + PUZZLE_SIZE);
                next->parent = d;
                next->nparents = d->nparents + 1;
                if(next->h + next->ldis < d->h + d->ldis) next->preference = d->preference + d->h + d->ldis - next->h - next->ldis;
                else if(d->preference > next->h + next->ldis - d->h - d->ldis + 1) next->preference = d->preference + d->h + d->ldis - next->h - next->ldis - 1;
                while(pthread_rwlock_trywrlock(w->pqLock));
                EnqueuePQ(pq, next);
                pthread_rwlock_unlock(w->pqLock);
                if (w->debug)
                {
                    while(pthread_mutex_trylock(w->outputLock));
                    printf("(%i) Accepted (h=%u l=%u p=%u n=%u).\n\n", (int)pthread_self(), next->h, next->ldis, next->preference, next->nparents);
                    if(w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
            else
            {
                if (w->debug)
                {
                    while(pthread_mutex_trylock(w->outputLock));
                    printf("(%i) Rejected.\n\n", (int)pthread_self());
                    if(w->interact && w->firstThread == self)
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
            if (w->debug) {
                while(pthread_mutex_trylock(w->outputLock));
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated L.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            while(pthread_rwlock_tryrdlock(w->platesLock));
            findResult = FindPlate(plates, plate);
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult)
            {
                while(pthread_rwlock_trywrlock(w->platesLock));
                AddPlate(plates, plate);
                next = DecisionBankAdd(dbank);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate; plate = next->p; next->p = findResult;
                next->h = CalcDis(next->p, w->goal);
                next->ldis = CalcLDis(next->p, w->goal, i - 1);
                next->parent = d;
                next->nparents = d->nparents + 1;
                if(next->h + next->ldis < d->h + d->ldis) next->preference = d->preference + d->h + d->ldis - next->h - next->ldis;
                else if(d->preference > next->h + next->ldis - d->h - d->ldis + 1) next->preference = d->preference + d->h + d->ldis - next->h - next->ldis - 1;
                while(pthread_rwlock_trywrlock(w->pqLock));
                EnqueuePQ(pq, next);
                pthread_rwlock_unlock(w->pqLock);
                if (w->debug)
                {
                    while(pthread_mutex_trylock(w->outputLock));
                    printf("(%i) Accepted (h=%u l=%u p=%u n=%u).\n\n", (int)pthread_self(), next->h, next->ldis, next->preference, next->nparents);
                    if(w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
            else
            {
                if (w->debug)
                {
                    while(pthread_mutex_trylock(w->outputLock));
                    printf("(%i) Rejected.\n\n", (int)pthread_self());
                    if(w->interact && w->firstThread == self)
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
            if (w->debug) {
                while(pthread_mutex_trylock(w->outputLock));
                printPlate(plate, stdout);
                printf("(%i) Next Desicion generated R.\n", (int)pthread_self());
                pthread_mutex_unlock(w->outputLock);
            }

            while(pthread_rwlock_tryrdlock(w->platesLock));
            findResult = FindPlate(plates, plate);
            pthread_rwlock_unlock(w->platesLock);

            if (!findResult)
            {
                while(pthread_rwlock_trywrlock(w->platesLock));
                AddPlate(plates, plate);
                next = DecisionBankAdd(dbank);
                pthread_rwlock_unlock(w->platesLock);
                findResult = plate; plate = next->p; next->p = findResult;
                next->h = CalcDis(next->p, w->goal);
                next->ldis = CalcLDis(next->p, w->goal, i + 1);
                next->parent = d;
                next->nparents = d->nparents + 1;
                if(next->h + next->ldis < d->h + d->ldis) next->preference = d->preference + d->h + d->ldis - next->h - next->ldis;
                else if(d->preference > next->h + next->ldis - d->h - d->ldis + 1) next->preference = d->preference + d->h + d->ldis - next->h - next->ldis - 1;
                while(pthread_rwlock_trywrlock(w->pqLock));
                EnqueuePQ(pq, next);
                pthread_rwlock_unlock(w->pqLock);
                if (w->debug)
                {
                    while(pthread_mutex_trylock(w->outputLock));
                    printf("(%i) Accepted (h=%u l=%u p=%u n=%u).\n\n", (int)pthread_self(), next->h, next->ldis, next->preference, next->nparents);
                    if(w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
            else
            {
                if (w->debug)
                {
                    while(pthread_mutex_trylock(w->outputLock));
                    printf("(%i) Rejected.\n\n", (int)pthread_self());
                    if(w->interact && w->firstThread == self)
                        fgets(buf, sizeof(buf), stdin);
                    pthread_mutex_unlock(w->outputLock);
                }
            }
        }
    }

    free(plate->s);
    free(plate);
    return 0;
}

int main(int argc, char **argv)
{
    char buf[256];
    WORKSPACE w;
    pthread_attr_t attr;
    pthread_rwlock_t pqLock;
    pthread_rwlock_t platesLock;
    pthread_rwlock_t exitLock;
    pthread_mutex_t outputLock;
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
    //unsigned int lastH, lastN;
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

        if (strcmp("noninteract", argv[argci]) == 0)
        {
            interact = 0;
        }
    }

    printf("SIZE = %u\n", PUZZLE_SIZE);
    printf("THREADS = %u\n", nthreads);
    printf("Input puzzle for GOAL:\n");
    if ((r = GetPlate(&goal, stdin)))
        return r;
    printPlate(&goal, stdout);
    printf("Input puzzle for CURRENT:\n");
    if ((r = GetPlate(&input, stdin)))
        return r;
    printPlate(&input, stdout);
    printf("\n");

    t = clock();
    d = DecisionBankAdd(dbank);
    memcpy(d->p->s, input.s, PUZZLE_SIZE * PUZZLE_SIZE);
    d->h = CalcDis(d->p, &goal);
    d->ldis = CalcLDis(d->p, &goal, FindInPlate(d->p, 0));
    AddPlate(plates, d->p);
    EnqueuePQ(pq, d);

    max = min = d->h;
    //lastH = d->h;
    //lastN = 0;
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
    w.pq =pq;
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
    for(i = 0; i < nthreads; i += 1) {
        pthread_create(th + i, &attr, doWork, &w);
        if(i == 0) w.firstThread = th[i];
        //sleep(1);
    }
    //printf("All thread are runnung...\n");
    for(i = 0; i < nthreads; i += 1) {
        pthread_join(th[i], NULL);
        //
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
