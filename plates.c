#include <string.h>
#include <stdlib.h>

#include "avl.h"
#include "main.h"

int cmpPlate(void *a, void *b)
{
    return memcmp(a, b, PUZZLE_SIZE * PUZZLE_SIZE);
}

static void freePlate(void *dataPtr)
{
    free(dataPtr);
}

AVL_TREE *CreatePlates(void)
{
    return AVL_Create(cmpPlate, freePlate);
}

void DestroyPlates(AVL_TREE *plates)
{
    AVL_Destroy(plates);
}

void AddPlate(AVL_TREE *plates, unsigned char *p)
{
    unsigned char *q = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    memcpy(q, p, PUZZLE_SIZE * PUZZLE_SIZE);
    if (AVL_Insert(plates, q) == 0)
        free(q);
}

unsigned char *FindPlate(AVL_TREE *plates, unsigned char *key)
{
    return (unsigned char *)AVL_Retrieve(plates, key);
}
