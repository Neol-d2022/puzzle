#include "core.h"
#include "util.h"
#include "main.h"

unsigned int CalcDis(unsigned char *current, const unsigned char *goal, unsigned int **buffers)
{
    unsigned int *p = buffers[0], *d = buffers[1];
    unsigned int i, j, k, l, x[2];

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if (current[i] == 0)
        {
            p[i] = 0;
            d[i] = 32;
            continue;
        }
        for (j = 0; j < PUZZLE_SIZE * PUZZLE_SIZE; j += 1)
        {
            if (current[i] == goal[j])
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
                    if (i / PUZZLE_SIZE > j / PUZZLE_SIZE)
                    {
                        d[i] = 8; //Up
                    }
                    else
                    {
                        d[i] = 4; //Down
                    }
                }
                else
                    d[i] = 16;
                break;
            }
        }
        if (j >= PUZZLE_SIZE * PUZZLE_SIZE)
            return -1;
    }

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if (p[i] == 0 && d[i] != 32)
            continue;
        D1ToD2(i, x, x + 1);
        l = 0;
        for (j = 0; j < 3; j += 1)
        {
            if (x[0] - 1 + j >= PUZZLE_SIZE)
                continue;
            for (k = 0; k < 3; k += 1)
            {
                if (x[1] - 1 + k >= PUZZLE_SIZE)
                    continue;
                if (j != 1 && k != 1)
                    continue;
                if (j == 1 && k == 1)
                    continue;

                l += 4;
                if (p[((x[0] - 1 + j) * PUZZLE_SIZE) + (x[1] - 1 + k)] == 0)
                    l += 1;
            }
        }
        l = (l >> 2) - (l & 0x4);
        if (l < 2)
        {
            p[i] += (2 - l) * 2;
            if(d[i] == 32) p[i] -= 2;
        }
    }

    l = 0;
    for (i = 0; i < PUZZLE_SIZE; i += 1)
    {
        for (j = 0; j < PUZZLE_SIZE; j += 1)
        {
            for (k = j + 1; k < PUZZLE_SIZE; k += 1)
            {
                if (d[(i * PUZZLE_SIZE) + j] == 1 && d[(i * PUZZLE_SIZE) + k] == 2)
                {
                    l += 2;
                    d[(i * PUZZLE_SIZE) + j] = 16;
                    d[(i * PUZZLE_SIZE) + k] = 16;
                }
                if (d[(i * PUZZLE_SIZE) + j] == 1 && d[(i * PUZZLE_SIZE) + k] == 0 && k - j < p[(i * PUZZLE_SIZE) + j])
                {
                    l += 2;
                    d[(i * PUZZLE_SIZE) + j] = 16;
                    d[(i * PUZZLE_SIZE) + k] = 16;
                }
                if (d[(i * PUZZLE_SIZE) + j] == 0 && d[(i * PUZZLE_SIZE) + k] == 2 && k - j < p[(i * PUZZLE_SIZE) + k])
                {
                    l += 2;
                    d[(i * PUZZLE_SIZE) + j] = 16;
                    d[(i * PUZZLE_SIZE) + k] = 16;
                }
                if (d[(j * PUZZLE_SIZE) + i] == 4 && d[(k * PUZZLE_SIZE) + i] == 8)
                {
                    l += 2;
                    d[(j * PUZZLE_SIZE) + i] = 16;
                    d[(k * PUZZLE_SIZE) + i] = 16;
                }
                if (d[(j * PUZZLE_SIZE) + i] == 4 && d[(k * PUZZLE_SIZE) + i] == 0 && k - j < p[(j * PUZZLE_SIZE) + i])
                {
                    l += 2;
                    d[(j * PUZZLE_SIZE) + i] = 16;
                    d[(k * PUZZLE_SIZE) + i] = 16;
                }
                if (d[(j * PUZZLE_SIZE) + i] == 0 && d[(k * PUZZLE_SIZE) + i] == 8 && k - j < p[(k * PUZZLE_SIZE) + i])
                {
                    l += 2;
                    d[(j * PUZZLE_SIZE) + i] = 16;
                    d[(k * PUZZLE_SIZE) + i] = 16;
                }

                if (p[(i * PUZZLE_SIZE) + j] == 0)
                    d[(i * PUZZLE_SIZE) + j] = 0;
                if (p[(i * PUZZLE_SIZE) + k] == 0)
                    d[(i * PUZZLE_SIZE) + k] = 0;
                if (p[(j * PUZZLE_SIZE) + i] == 0)
                    d[(j * PUZZLE_SIZE) + i] = 0;
                if (p[(k * PUZZLE_SIZE) + i] == 0)
                    d[(k * PUZZLE_SIZE) + i] = 0;
            }
        }
    }

    k = 0;
    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        k += p[i];
    }
    k += l;
    return k;
}
