#include <stdio.h>

#include "main.h"
#include "plate.h"

void D1ToD2(unsigned int i, unsigned int *row, unsigned int *col)
{
    *row = i / PUZZLE_SIZE;
    *col = i % PUZZLE_SIZE;
}

void GetMax(unsigned int *max, unsigned int *min)
{
    unsigned int t;

    if (*min > *max)
    {
        t = *max;
        *max = *min;
        *min = t;
    }
}

unsigned int D2Diff(unsigned int i, unsigned int j)
{
    unsigned int x[2], y[2], r;

    D1ToD2(i, x, x + 1);
    D1ToD2(j, y, y + 1);

    r = 0;
    GetMax(x, y);
    r += x[0] - y[0];
    GetMax(x + 1, y + 1);
    r += x[1] - y[1];

    return r;
}

unsigned int FindInPlate(const PLATE *p, unsigned int chr)
{
    unsigned int i;

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
        if ((p->s)[i] == chr)
            break;
    return i;
}

void swap(unsigned char *a, unsigned char *b)
{
    unsigned char c = *a;
    *a = *b;
    *b = c;
}

void printPlate(const PLATE *p, FILE *f)
{
    unsigned int i, j;

    for (j = 0; j < PUZZLE_SIZE; j += 1)
    {
        for (i = 0; i < PUZZLE_SIZE - 1; i += 1)
        {
            if ((p->s)[j * PUZZLE_SIZE + i])
                fprintf(f, "%3u ", (unsigned int)((p->s)[j * PUZZLE_SIZE + i]));
            else
                fprintf(f, "    ");
        }

        if ((p->s)[j * PUZZLE_SIZE + PUZZLE_SIZE - 1])
            fprintf(f, "%3u\n", (unsigned int)((p->s)[j * PUZZLE_SIZE + PUZZLE_SIZE - 1]));
        else
            fprintf(f, "   \n");
    }
}
