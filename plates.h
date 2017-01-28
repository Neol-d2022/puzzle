#ifndef PLATES_H
#define PLATES_H

#include "avl.h"

int cmpPlate(void *a, void *b);
AVL_TREE *CreatePlates(void);
void DestroyPlates(AVL_TREE *plates);
void AddPlate(AVL_TREE *plates, unsigned char *p);
unsigned char *FindPlate(AVL_TREE *plates, unsigned char *key);

#endif
