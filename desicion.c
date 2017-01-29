#include <stdlib.h>
#include <string.h>

#include "desicion.h"
#include "main.h"

void InitD(DESICISON *d)
{
    d->p = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    d->parent = (unsigned char *)malloc(8);
    d->nparents = 0;
    d->npmax = 8;
}

void DeInitD(DESICISON *d)
{
    free(d->p);
    free(d->parent);
}

static void _copyParent(DESICISON *next, const DESICISON *d) {
    next->parent = (unsigned char *)realloc(next->parent, d->npmax);
    memcpy(next->parent, d->parent, d->nparents);
    next->npmax = d->npmax;
    next->nparents = d->nparents;
}

static void _addParent(DESICISON *d, unsigned char c) {
    if(d->nparents == d->npmax) {
        d->parent = (unsigned char*)realloc(d->parent, d->npmax << 1);
        d->npmax <<= 1;
    }

    (d->parent)[d->nparents] = c;
    d->nparents += 1;
}

void AddParent(DESICISON *next, const DESICISON *d, unsigned char c) {
    _copyParent(next, d);
    _addParent(next, c);
}
