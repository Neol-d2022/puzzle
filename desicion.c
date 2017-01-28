#include <stdlib.h>
#include <string.h>

#include "desicion.h"
#include "main.h"

void InitD(DESICISON *d)
{
    d->p = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    d->parent = 0;
    d->nparents = 0;
    d->ref = 0;
}

void DeInitD(DESICISON *d)
{
    if (d->parent)
        d->parent->ref -= 1;
    free(d->p);
}
