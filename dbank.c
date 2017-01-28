
#include <stdlib.h>

#include "dbank.h"

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
