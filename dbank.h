#ifndef DBANK_H
#define DBANK_H

#include "desicion.h"

typedef struct
{
    DESICISON **base;
    unsigned int n;
    unsigned int max;
} DBank;

DBank *CreateDecisionBank(void);
DESICISON *DecisionBankAdd(DBank *dbank);
void DecisionBankDestory(DBank *dbank);

#endif
