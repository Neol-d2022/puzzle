#ifndef PQ_H
#define PQ_H

#include "avl.h"
#include "desicion.h"

AVL_TREE *CreatePQ(void);
void DestroyPQ(AVL_TREE *pq);
void EnqueuePQ(AVL_TREE *pq, DESICISON *d);
int DequeuePQ(AVL_TREE *pq, DESICISON **d);

#endif
