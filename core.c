#include "core.h"
#include "util.h"
#include "main.h"

unsigned int CalcDis(PLATE *current, const PLATE *goal, unsigned int **buffers)
{
    unsigned int *p = buffers[0], *d = buffers[1];
    unsigned int i, j, k, x[2];

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if ((current->s)[i] == 0)
        {
            p[i] = 0;
            d[i] = 16;
            continue;
        }
        for (j = 0; j < PUZZLE_SIZE * PUZZLE_SIZE; j += 1)
        {
            if ((current->s)[i] == (goal->s)[j])
            {
                p[i] = D2Diff(i, j);
                if (p[i] == 0)
                    d[i] = 0;
                else if (i / PUZZLE_SIZE == j / PUZZLE_SIZE)
                {
                    if (i % PUZZLE_SIZE < j % PUZZLE_SIZE)
                    {
                        d[i] = 1; //Right
                    }
                    else
                    {
                        d[i] = 2; //Left
                    }
                }
                else if (i % PUZZLE_SIZE == j % PUZZLE_SIZE)
                {
                    if (i / PUZZLE_SIZE < j / PUZZLE_SIZE)
                    {
                        d[i] = 8; //Up
                    }
                    else
                    {
                        d[i] = 4; //Down
                    }
                }
                break;
            }
        }
        if (j >= PUZZLE_SIZE * PUZZLE_SIZE)
            return -1;
    }

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if ((current->s)[i] == 0)
            continue;
        D1ToD2(i, x, x + 1);
        if (x[0] == 0)
        {
            if (d[i] == 4)
            {
                if (d[i + PUZZLE_SIZE] == 8)
                    p[i] += 1;
                else if (d[i + PUZZLE_SIZE] == 0)
                    p[i] += 2;
            }
        }
        else if (x[0] == PUZZLE_SIZE - 1)
        {
            if (d[i] == 8)
            {
                if (d[i - PUZZLE_SIZE] == 4)
                    p[i] += 1;
                else if (d[i - PUZZLE_SIZE] == 0)
                    p[i] += 2;
            }
        }
        else
        {
            if (d[i] == 4)
            {
                if (d[i + PUZZLE_SIZE] == 8)
                    p[i] += 1;
                else if (d[i + PUZZLE_SIZE] == 0)
                    p[i] += 2;
            }
            if (d[i] == 8)
            {
                if (d[i - PUZZLE_SIZE] == 4)
                    p[i] += 1;
                else if (d[i + PUZZLE_SIZE] == 0)
                    p[i] += 2;
            }
        }
        if (x[1] == 0)
        {
            if (d[i] == 1)
            {
                if (d[i + 1] == 2)
                    p[i] += 1;
                else if (d[i + 1] == 0)
                    p[i] += 2;
            }
        }
        else if (x[1] == PUZZLE_SIZE - 1)
        {
            if (d[i] == 2)
            {
                if (d[i - 1] == 1)
                    p[i] += 1;
                else if (d[i - 1] == 0)
                    p[i] += 2;
            }
        }
        else
        {
            if (d[i] == 1)
            {
                if (d[i + 1] == 2)
                    p[i] += 1;
                else if (d[i + 1] == 0)
                    p[i] += 2;
            }
            if (d[i] == 2)
            {
                if (d[i - 1] == 1)
                    p[i] += 1;
                else if (d[i - 1] == 0)
                    p[i] += 2;
            }
        }
    }

    k = 0;
    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        k += p[i];
    }
    return k;
}
