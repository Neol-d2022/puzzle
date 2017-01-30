#include <stdlib.h>

#include "dbank.h"

static int cmpDBank(void *a, void *b)
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

DBank *CreateDecisionBank(void)
{
    DBank *ret;

    ret = AVL_Create(cmpDBank, 0);
    return ret;
}

DESICISON *DecisionBankAdd(DBank *dbank)
{
    DESICISON *ret;

    ret = (DESICISON *)malloc(sizeof(*ret));
    InitD(ret);
    AVL_Insert(dbank, ret);
    return ret;
}

static void _traverseF(void *dataPtr, void *n)
{
    if (n)
        *(int *)n = 0;
    DeInitD(dataPtr);
    free(dataPtr);
}

void DecisionBankDestory(DBank *dbank)
{
    AVL_Traverse(dbank, 0, _traverseF);
    AVL_Destroy(dbank);
}

typedef struct
{
    DESICISON **base;
    unsigned int i;
} _WP_R;

static void _traverseC(void *dataPtr, void *n)
{
    _WP_R *w = (_WP_R *)n;
    (w->base)[w->i] = (DESICISON *)dataPtr;
    (w->i) += 1;
}

void DecisionBankClearUp(DBank *dbank, AVL_TREE *pq)
{
    _WP_R w;
    DESICISON **base;
    unsigned int i, n;

    base = (DESICISON **)malloc(sizeof(*base) * pq->count);
    w.base = base;
    w.i = 0;
    AVL_Traverse(pq, &w, _traverseC);

    n = pq->count;
    for (i = 0; i < n; i += 1)
    {
        AVL_Delete(pq, base[i]);
        AVL_Delete(dbank, base[i]);
        DeInitD(base[i]);
        free(base[i]);
    }

    free(base);
}
