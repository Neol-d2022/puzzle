#include "stack.h"
#include "stdlib.h"
#include "string.h"

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
