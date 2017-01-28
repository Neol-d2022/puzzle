#ifndef DESICION_H
#define DESICION_H

struct DESICISON_STRUCT;

#include "plate.h"

typedef struct DESICISON_STRUCT
{
    PLATE *p;
    struct DESICISON_STRUCT *parent;
    unsigned int h; //distance to goal
    unsigned int nparents;
} DESICISON;

void InitD(DESICISON *d);
void DeInitD(DESICISON *d);

#endif
