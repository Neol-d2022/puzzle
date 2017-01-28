
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

static void _traverseF(void *dataPtr, void *n) {
    DeInitD(dataPtr);
    free(dataPtr);
}

void DecisionBankDestory(DBank *dbank)
{
    AVL_Traverse(dbank, 0, _traverseF);
    AVL_Destroy(dbank);
}
