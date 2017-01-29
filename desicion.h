#ifndef DESICION_H
#define DESICION_H

struct DESICISON_STRUCT;

typedef struct DESICISON_STRUCT
{
    unsigned char *p;
    unsigned char *parent;
    unsigned int h; //distance to goal
    unsigned int nparents;
    unsigned int npmax;
} DESICISON;

void InitD(DESICISON *d);
void DeInitD(DESICISON *d);
void AddParent(DESICISON *next, const DESICISON *d, unsigned char c);

#endif
