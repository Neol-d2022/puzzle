#ifndef DBANK_H
#define DBANK_H

#include "desicion.h"
#include "avl.h"

typedef AVL_TREE DBank;

DBank *CreateDecisionBank(void);
DESICISON *DecisionBankAdd(DBank *dbank);
void DecisionBankDestory(DBank *dbank);

#endif
