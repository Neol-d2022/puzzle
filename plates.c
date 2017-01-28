#include <string.h>

#include "avl.h"
#include "plate.h"
#include "main.h"

int cmpPlate(void *a, void *b)
{
    PLATE *c, *d;

    c = (PLATE *)a;
    d = (PLATE *)b;

    return memcmp(c->s, d->s, PUZZLE_SIZE * PUZZLE_SIZE);
}

AVL_TREE *CreatePlates(void)
{
    return AVL_Create(cmpPlate, 0);
}

void DestroyPlates(AVL_TREE *plates)
{
    //free(plates->base);
    AVL_Destroy(plates);
}

void AddPlate(AVL_TREE *plates, PLATE *p)
{
    AVL_Insert(plates, p);
}

PLATE *FindPlate(AVL_TREE *plates, PLATE *key)
{
    return AVL_Retrieve(plates, key);
}
