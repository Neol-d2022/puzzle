#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avl.h"

//#define PUZZLE_SIZE 4
//#define DB_MAX (PUZZLE_SIZE * PUZZLE_SIZE * PUZZLE_SIZE)
//#define DB_DROP_FACTOR 0.9

unsigned int DB;
#define LEVELMAX 100
unsigned int LEVEL = LEVELMAX >> 1;
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
    //DESICISON **e, **f;

    /*e = (DESICISON **)a;
    f = (DESICISON **)b;*/

    c = (DESICISON *)a;
    d = (DESICISON *)b;

    //if (c->h * (DB_MAX - DB) + c->nparents * DB > d->h * (DB_MAX - DB) + d->nparents * DB)
    //    return -1;
    //else if (c->h * (DB_MAX - DB) + c->nparents * DB < d->h * (DB_MAX - DB) + d->nparents * DB)
    //    return 1;
    if (c->h * (LEVELMAX - LEVEL) + c->nparents * LEVEL > d->h * (LEVELMAX - LEVEL) + d->nparents * LEVEL)
        return -1;
    else if (c->h * (LEVELMAX - LEVEL) + c->nparents * LEVEL < d->h * (LEVELMAX - LEVEL) + d->nparents * LEVEL)
        return 1;
    else if (c->h > d->h)
        return -1;
    else if (c->h < d->h)
        return 1;
    else if (c->nparents > d->nparents)
        return -1;
    else if (c->nparents < d->nparents)
        return -1;
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

unsigned int FindInPlate(const PLATE *p, unsigned int chr)
{
    unsigned int i;

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
        if ((p->s)[i] == chr)
            break;
    return i;
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

int DecisionBankRemoveLast(DBank *dbank)
{
    if (dbank->n > 0)
    {
        dbank->n -= 1;
        DeInitD((dbank->base)[dbank->n]);
        free((dbank->base)[dbank->n]);
        return 0;
    }
    else
        return 1;
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

int main(int argc, char **argv)
{
    char buf[256];
    PLATE goal;
    PLATE input;
    DESICISON *d, *next;
    DBank *dbank = CreateDecisionBank();
    AVL_TREE *pq = CreatePQ();
    AVL_TREE *plates = CreatePlates();
    STACK *stack = CreateStack();
    DESICISON *cur;
    size_t s;
    int r;
    unsigned int i, rc[2], c, min, max;
    //unsigned int lastH, lastN;
    int debug = 0, interact = 1, argci;

    for (argci = 1; argci < argc; argci += 1)
    {
        if (strcmp("debug", argv[argci]) == 0)
        {
            debug = 1;
        }

        if (strcmp("level", argv[argci]) == 0 && (argci + 1 < argc))
        {
            sscanf(argv[argci + 1], "%u", &LEVEL);
            if (LEVEL > LEVELMAX)
                LEVEL = LEVELMAX;
            argci += 1;
        }

        if (strcmp("size", argv[argci]) == 0 && (argci + 1 < argc))
        {
            sscanf(argv[argci + 1], "%u", &PUZZLE_SIZE);
            if (PUZZLE_SIZE == 0)
                PUZZLE_SIZE = 2;
            argci += 1;
        }

        if (strcmp("noninteract", argv[argci]) == 0)
        {
            interact = 0;
        }
    }

    printf("SIZE = %u\n", PUZZLE_SIZE);
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

    //DB = (DB_MAX >> 2) - 1;
    d = DecisionBankAdd(dbank);
    memcpy(d->p->s, input.s, PUZZLE_SIZE * PUZZLE_SIZE);
    d->h = CalcDis(d->p, &goal);
    AddPlate(plates, d->p);
    EnqueuePQ(pq, d);

    max = min = d->h;
    //lastH = d->h;
    //lastN = 0;
    c = 1;
    while (!DequeuePQ(pq, &d))
    {
        if (debug)
        {
            printPlate(d->p, stdout);
            printf("Dequeued\n\n");
        }
        else
        {
            if (min > d->h)
                min = d->h;
            if (d->h > max)
                max = d->h;

            s = printf("mindis=%u maxdis=%u dis=%u tested=%u Qsize=%u Psize=%u steps=%u        ", min, max, d->h, c, pq->count, plates->count, d->nparents);
            memset(buf, '\b', s);
            buf[s] = '\0';
            printf("%s", buf);
        }

        if (d->h == 0)
        {
            r |= 0x10;
            printf("\n");
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
        if (debug)
        {
            printf("i=%u r=%u c=%u f=%u\n", i, rc[0], rc[1], r);
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
            next = DecisionBankAdd(dbank);
            memcpy(next->p->s, d->p->s, PUZZLE_SIZE * PUZZLE_SIZE);
            swap((next->p)->s + rc[0] * PUZZLE_SIZE + rc[1], (next->p)->s + (rc[0] - 1) * PUZZLE_SIZE + rc[1]);
            if (debug)
                printPlate(next->p, stdout);
            if (debug)
                printf("Next Desicion generated U.\n");
            if (!FindPlate(plates, next->p))
            {
                next->h = CalcDis(next->p, &goal);
                AddPlate(plates, next->p);
                next->parent = d;
                next->nparents = d->nparents + 1;
                EnqueuePQ(pq, next);
                if (debug)
                {
                    printf("Accepted (h = %4u).\n\n", next->h);
                    if (interact)
                        fgets(buf, sizeof(buf), stdin);
                }
            }
            else
            {
                DecisionBankRemoveLast(dbank);
                if (debug)
                {
                    printf("Rejected.\n\n");
                    if (interact)
                        fgets(buf, sizeof(buf), stdin);
                }
            }
        }

        // Down
        if (r & 0x4)
        {
            next = DecisionBankAdd(dbank);
            memcpy(next->p->s, d->p->s, PUZZLE_SIZE * PUZZLE_SIZE);
            swap((next->p)->s + rc[0] * PUZZLE_SIZE + rc[1], (next->p)->s + (rc[0] + 1) * PUZZLE_SIZE + rc[1]);
            if (debug)
                printPlate(next->p, stdout);
            if (debug)
                printf("Next Desicion generated D.\n");
            if (!FindPlate(plates, next->p))
            {
                next->h = CalcDis(next->p, &goal);
                AddPlate(plates, next->p);
                next->parent = d;
                next->nparents = d->nparents + 1;
                EnqueuePQ(pq, next);
                if (debug)
                {
                    printf("Accepted (h = %4u).\n\n", next->h);
                    if (interact)
                        fgets(buf, sizeof(buf), stdin);
                }
            }
            else
            {
                DecisionBankRemoveLast(dbank);
                if (debug)
                {
                    printf("Rejected.\n\n");
                    if (interact)
                        fgets(buf, sizeof(buf), stdin);
                }
            }
        }

        // Left
        if (r & 0x2)
        {
            next = DecisionBankAdd(dbank);
            memcpy(next->p->s, d->p->s, PUZZLE_SIZE * PUZZLE_SIZE);
            swap((next->p)->s + rc[0] * PUZZLE_SIZE + rc[1], (next->p)->s + rc[0] * PUZZLE_SIZE + (rc[1] - 1));
            if (debug)
                printPlate(next->p, stdout);
            if (debug)
                printf("Next Desicion generated L.\n");
            if (!FindPlate(plates, next->p))
            {
                next->h = CalcDis(next->p, &goal);
                AddPlate(plates, next->p);
                next->parent = d;
                next->nparents = d->nparents + 1;
                EnqueuePQ(pq, next);
                if (debug)
                {
                    printf("Accepted (h = %4u).\n\n", next->h);
                    if (interact)
                        fgets(buf, sizeof(buf), stdin);
                }
            }
            else
            {
                DecisionBankRemoveLast(dbank);
                if (debug)
                {
                    printf("Rejected.\n\n");
                    if (interact)
                        fgets(buf, sizeof(buf), stdin);
                }
            }
        }

        // Right
        if (r & 0x1)
        {
            next = DecisionBankAdd(dbank);
            memcpy(next->p->s, d->p->s, PUZZLE_SIZE * PUZZLE_SIZE);
            swap((next->p)->s + rc[0] * PUZZLE_SIZE + rc[1], (next->p)->s + rc[0] * PUZZLE_SIZE + (rc[1] + 1));
            if (debug)
                printPlate(next->p, stdout);
            if (debug)
                printf("Next Desicion generated R.\n");
            if (!FindPlate(plates, next->p))
            {
                next->h = CalcDis(next->p, &goal);
                AddPlate(plates, next->p);
                next->parent = d;
                next->nparents = d->nparents + 1;
                EnqueuePQ(pq, next);
                if (debug)
                {
                    printf("Accepted (h = %4u).\n\n", next->h);
                    if (interact)
                        fgets(buf, sizeof(buf), stdin);
                }
            }
            else
            {
                DecisionBankRemoveLast(dbank);
                if (debug)
                {
                    printf("Rejected.\n\n");
                    if (interact)
                        fgets(buf, sizeof(buf), stdin);
                }
            }
        }

        //lastH = d->h;
        //lastN = d->nparents;
        c += 1;
    }

    if (r & 0x10)
    {
        if (debug)
            printPlate(d->p, stdout);
        printf("Solution found.\n\n");
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