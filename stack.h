#ifndef STACK_H
#define STACK_H

#include "desicion.h"

typedef struct
{
    DESICISON **base;
    unsigned int n;
    unsigned int max;
} STACK;

STACK *CreateStack(void);
void StackAdd(STACK *stack, DESICISON *p);

#endif
