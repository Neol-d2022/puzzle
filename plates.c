#include <string.h>

#include "avl.h"
#include "main.h"

int cmpPlate(void *a, void *b)
{
    return memcmp(a, b, PUZZLE_SIZE * PUZZLE_SIZE);
}

AVL_TREE *CreatePlates(void)
{
    return AVL_Create(cmpPlate, 0);
}

void DestroyPlates(AVL_TREE *plates)
{
    AVL_Destroy(plates);
}

void AddPlate(AVL_TREE *plates, unsigned char *p)
{
    AVL_Insert(plates, p);
}

unsigned char *FindPlate(AVL_TREE *plates, unsigned char *key)
{
    return (unsigned char*)AVL_Retrieve(plates, key);
}
