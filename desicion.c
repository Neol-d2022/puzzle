#include <stdlib.h>
#include <string.h>

#include "desicion.h"
#include "plate.h"
#include "main.h"

void InitD(DESICISON *d)
{
    PLATE *p;

    p = (PLATE *)malloc(sizeof(*p));
    p->s = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    memset(p->s, 0, PUZZLE_SIZE * PUZZLE_SIZE);
    p->parent = d;
    d->p = p;
    d->parent = 0;
    d->nparents = 0;
}

void DeInitD(DESICISON *d)
{
    free(d->p);
}
