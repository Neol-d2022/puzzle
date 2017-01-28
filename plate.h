#ifndef PLATE_H
#define PLATE_H

struct DESICISON_STRUCT;
typedef struct
{
    unsigned char *s;
    struct DESICISON_STRUCT *parent;
} PLATE;

#include "desicion.h"

#endif
