#ifndef DESICION_H
#define DESICION_H

struct DESICISON_STRUCT;

typedef struct DESICISON_STRUCT
{
    unsigned char *p;
    struct DESICISON_STRUCT *parent;
    unsigned int h; //distance to goal
    unsigned int nparents;
    unsigned int ref;
} DESICISON;

void InitD(DESICISON *d);
void DeInitD(DESICISON *d);

#endif
