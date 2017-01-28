#include "pq.h"
#include "main.h"
#include "plates.h"

// Notice: REVERSED!!
static int cmpPQ(void *a, void *b)
{
    DESICISON *c, *d;

    c = (DESICISON *)a;
    d = (DESICISON *)b;

    if (c->h + (c->nparents << LEVEL) + (1 << LEVEL) - 1 > d->h + (d->nparents << LEVEL) + (1 << LEVEL) - 1)
        return -1;
    else if (c->h + (c->nparents << LEVEL) + (1 << LEVEL) - 1 < d->h + (d->nparents << LEVEL) + (1 << LEVEL) - 1)
        return 1;
    else if (c->h > d->h)
        return -1;
    else if (c->h < d->h)
        return 1;
    else if (c->nparents > d->nparents)
        return -1;
    else if (c->nparents < d->nparents)
        return 1;
    else
        return cmpPlate(c->p, d->p);
}

AVL_TREE *CreatePQ(void)
{
    return AVL_Create(cmpPQ, 0);
}

void DestroyPQ(AVL_TREE *pq)
{
    AVL_Destroy(pq);
}

//copy to queue
void EnqueuePQ(AVL_TREE *pq, DESICISON *d)
{
    AVL_Insert(pq, d);
}

int DequeuePQ(AVL_TREE *pq, DESICISON **d)
{
    NODE *n;

    if (!(pq->root))
        return 1;

    n = pq->root;
    while (n->right)
    {
        n = n->right;
    }

    *d = n->dataPtr;
    AVL_Delete(pq, n->dataPtr);

    return 0;
}
