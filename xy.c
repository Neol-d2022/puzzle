#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "main.h"
#include "avl.h"

static unsigned int max(unsigned int a, unsigned int b)
{
    return (a > b) ? a : b;
}

static unsigned int min(unsigned int a, unsigned int b)
{
    return (a < b) ? a : b;
}

static int cmpUChr(const void *a, const void *b)
{
    unsigned char c, d;

    c = *((unsigned char *)a);
    d = *((unsigned char *)b);

    return (int)c - (int)d;
}

typedef struct
{
    unsigned char *rc;
    unsigned int h;
    unsigned int cost;
} XY_OBJ;

static int cmpPtrV(void *a, void *b)
{
    size_t c, d;

    c = (size_t)a;
    d = (size_t)b;

    if (c > d)
        return 1;
    else if (c < d)
        return -1;
    else
        return 0;
}

static int cmpXYObj(void *a, void *b)
{
    XY_OBJ *c, *d;
    int r;

    c = (XY_OBJ *)a;
    d = (XY_OBJ *)b;

    if((r = cmpPtrV(a, b)) == 0) return 0;
    else if (c->h + c->cost > d->h + d->cost)
        return -1;
    else if (c->h + c->cost < d->h + d->cost)
        return 1;
    else if (c->h > d->h)
        return -1;
    else if (c->h < d->h)
        return 1;
    else if (c->cost > d->cost)
        return -1;
    else if (c->cost < d->cost)
        return 1;
    else return r;
}

static unsigned int _CalcDis_xymn(unsigned char *current, const unsigned char *goal)
{
    unsigned int i, j, k, l;

    l = 0;
    for (i = 0; i < PUZZLE_SIZE; i += 1)
    {
        for (j = 0; j < PUZZLE_SIZE; j += 1)
        {
            if (current[(i * PUZZLE_SIZE + j)] == 0)
                continue;
            for (k = 0; k < PUZZLE_SIZE; k += 1)
            {
                if (bsearch(current + (i * PUZZLE_SIZE + j), (goal + k * PUZZLE_SIZE), PUZZLE_SIZE, sizeof(*goal), cmpUChr) != 0)
                {
                    l += max(i, k) - min(i, k);
                    break;
                }
            }
        }
    }

    return l;
}
#define DEBUGP// printf("DEBUG %u\n", __LINE__);
static unsigned int FindInXY(unsigned char *current, unsigned char *key)
{
    unsigned int i;

    for (i = 0; i < PUZZLE_SIZE; i += 1) 
        if (bsearch(key, current + (i * PUZZLE_SIZE), PUZZLE_SIZE, sizeof(*current), cmpUChr) != 0)
            return i;

    return i;
}

static void _releaser(void *dataPtr)
{
    XY_OBJ *a;

    a = (XY_OBJ *)dataPtr;
    free(a->rc);
    free(a);
}

unsigned int CalcDis_xymn(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis)
{
    unsigned char *a = (unsigned char *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE * (3 * sizeof(unsigned int));
    unsigned char *b = (unsigned char *)(buffers) + (PUZZLE_SIZE * PUZZLE_SIZE * (3 * sizeof(unsigned int) + 1));
    unsigned char *c = (unsigned char *)(buffers) + (PUZZLE_SIZE * PUZZLE_SIZE * (3 * sizeof(unsigned int) + 2));
    unsigned char *d = (unsigned char *)(buffers) + (PUZZLE_SIZE * PUZZLE_SIZE * (3 * sizeof(unsigned int) + 3));
    unsigned char *_inits[] = {a, b};
    unsigned char *_goals[] = {c, d};
    unsigned int _result[2];
    AVL_TREE *q;
    XY_OBJ *e, *h;
    unsigned char *f;
    NODE *g;
    unsigned int i, k, l, m;
    unsigned char chr;

    if (blankDis)
        *blankDis = 0;

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        a[i] = current[i];DEBUGP
        b[i] = current[(i % PUZZLE_SIZE) * PUZZLE_SIZE + (i / PUZZLE_SIZE)];DEBUGP
        c[i] = goal[i];DEBUGP
        d[i] = goal[(i % PUZZLE_SIZE) * PUZZLE_SIZE + (i / PUZZLE_SIZE)];DEBUGP
    }

    for (i = 0; i < PUZZLE_SIZE; i += 1)
    {
        qsort(c + i * PUZZLE_SIZE, PUZZLE_SIZE, sizeof(*c), cmpUChr);DEBUGP
        qsort(d + i * PUZZLE_SIZE, PUZZLE_SIZE, sizeof(*d), cmpUChr);DEBUGP
    }

    for (l = 0; l < 2; l += 1)
    {
        q = AVL_Create(cmpXYObj, _releaser);DEBUGP
        e = (XY_OBJ *)malloc(sizeof(*e));DEBUGP
        f = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
        memcpy(f, _inits[l], PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
        for (i = 0; i < PUZZLE_SIZE; i += 1)
            qsort(f + i * PUZZLE_SIZE, PUZZLE_SIZE, sizeof(*f), cmpUChr);
        e->rc = f;DEBUGP
        e->cost = 0;DEBUGP
        e->h = _CalcDis_xymn(f, _goals[l]);DEBUGP
        AVL_Insert(q, e);DEBUGP
        do
        {DEBUGP
            if (!(q->root))
                break;
            g = q->root;DEBUGP
            while (g->right)
            {
                g = g->right;DEBUGP
            }
            e = g->dataPtr;DEBUGP

            if (e->h == 0)
                break;;

            chr = 0;DEBUGP
            k = FindInXY(e->rc, &chr);DEBUGP
            if (k == 0)
            {DEBUGP
                for (i = 0; i < PUZZLE_SIZE; i += 1)
                {DEBUGP
                    h = (XY_OBJ *)malloc(sizeof(*h));DEBUGP
                    f = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
                    memcpy(f, e->rc, PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
                    swap(f + (k * PUZZLE_SIZE), f + ((k + 1) * PUZZLE_SIZE + i));DEBUGP
                    for (m = 0; m < PUZZLE_SIZE; m += 1)
                        qsort(f + m * PUZZLE_SIZE, PUZZLE_SIZE, sizeof(*f), cmpUChr);
                    h->rc = f;DEBUGP
                    h->cost = e->cost + 1;DEBUGP
                    h->h = _CalcDis_xymn(f, _goals[l]);DEBUGP
                    AVL_Insert(q, h);DEBUGP
                }
            }
            else if (k == PUZZLE_SIZE - 1)
            {DEBUGP
                for (i = 0; i < PUZZLE_SIZE; i += 1)
                {DEBUGP
                    h = (XY_OBJ *)malloc(sizeof(*h));DEBUGP
                    f = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
                    memcpy(f, e->rc, PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
                    swap(f + (k * PUZZLE_SIZE), f + ((k - 1) * PUZZLE_SIZE + i));DEBUGP
                    for (m = 0; m < PUZZLE_SIZE; m += 1)
                        qsort(f + m * PUZZLE_SIZE, PUZZLE_SIZE, sizeof(*f), cmpUChr);
                    h->rc = f;DEBUGP
                    h->cost = e->cost + 1;DEBUGP
                    h->h = _CalcDis_xymn(f, _goals[l]);DEBUGP
                    AVL_Insert(q, h);DEBUGP
                }
            }
            else
            {DEBUGP
                for (i = 0; i < PUZZLE_SIZE; i += 1)
                {DEBUGP
                    h = (XY_OBJ *)malloc(sizeof(*h));DEBUGP
                    f = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
                    memcpy(f, e->rc, PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
                    swap(f + (k * PUZZLE_SIZE), f + ((k + 1) * PUZZLE_SIZE + i));DEBUGP
                    for (m = 0; m < PUZZLE_SIZE; m += 1)
                        qsort(f + m * PUZZLE_SIZE, PUZZLE_SIZE, sizeof(*f), cmpUChr);
                    h->rc = f;DEBUGP
                    h->cost = e->cost + 1;DEBUGP
                    h->h = _CalcDis_xymn(f, _goals[l]);DEBUGP
                    AVL_Insert(q, h);DEBUGP
                }
                for (i = 0; i < PUZZLE_SIZE; i += 1)
                {DEBUGP
                    h = (XY_OBJ *)malloc(sizeof(*h));DEBUGP
                    f = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
                    memcpy(f, e->rc, PUZZLE_SIZE * PUZZLE_SIZE);DEBUGP
                    swap(f + (k * PUZZLE_SIZE), f + ((k - 1) * PUZZLE_SIZE + i));DEBUGP
                    for (m = 0; m < PUZZLE_SIZE; m += 1)
                        qsort(f + m * PUZZLE_SIZE, PUZZLE_SIZE, sizeof(*f), cmpUChr);
                    h->rc = f;DEBUGP
                    h->cost = e->cost + 1;DEBUGP
                    h->h = _CalcDis_xymn(f, _goals[l]);DEBUGP
                    AVL_Insert(q, h);DEBUGP
                }
            }
DEBUGP
            AVL_Delete(q, e);DEBUGP
        } while (1);
        if (e->h == 0)
            _result[l] = e->cost;
        else
            _result[l] = (unsigned int)-1;
        AVL_Destroy(q);DEBUGP
    }
DEBUGP
    return _result[0] + _result[1];
}