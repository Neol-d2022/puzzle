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

unsigned char *FindPlate(AVL_TREE *plates, unsigned char *key)
{
    return (unsigned char *)AVL_Retrieve(plates, key);
}

int AddPlate(AVL_TREE *plates, unsigned char *p, unsigned int np)
{
    unsigned char *q = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE + sizeof(unsigned int));
    unsigned char *r;

    memcpy(q, p, PUZZLE_SIZE * PUZZLE_SIZE);
    if (AVL_Insert(plates, q) == 0)
    {
        free(q);
        r = FindPlate(plates, p);
        if (*(unsigned int *)(r + PUZZLE_SIZE * PUZZLE_SIZE) > np)
        {
            *(unsigned int *)(r + PUZZLE_SIZE * PUZZLE_SIZE) = np;
            return 0;
        }
        else
            return 1;
    }
    else
        return 0;
}