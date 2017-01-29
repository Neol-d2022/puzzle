#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "util.h"
#include "main.h"

static unsigned int max(unsigned int a, unsigned int b)
{
    return (a > b) ? a : b;
}

unsigned int CalcDis_slow(unsigned char *current, const unsigned char *goal, unsigned int **buffers)
{
    unsigned int *p = buffers[0], *d = buffers[1];
    unsigned char *c;
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
            if (d[i] == 32)
                p[i] -= 2;
        }
    }

    l = 0;
    c = (unsigned char *)malloc(PUZZLE_SIZE);
    for (i = 0; i < PUZZLE_SIZE; i += 1)
    {
    calcRow:
        memset(c, 0, PUZZLE_SIZE);
        for (j = 0; j < PUZZLE_SIZE; j += 1)
        {
            for (k = j + 1; k < PUZZLE_SIZE; k += 1)
            {
                if ((d[(i * PUZZLE_SIZE) + j] == 1 && d[(i * PUZZLE_SIZE) + k] == 2) ||
                    (d[(i * PUZZLE_SIZE) + j] == 1 && d[(i * PUZZLE_SIZE) + k] == 0) ||
                    (d[(i * PUZZLE_SIZE) + j] == 0 && d[(i * PUZZLE_SIZE) + k] == 2))
                {
                    if (k - j <= max(p[(i * PUZZLE_SIZE) + j], p[(i * PUZZLE_SIZE) + k]))
                    {
                        c[j] += 1;
                        c[k] += 1;
                    }
                }
            }
        }
        k = 0;
        for (j = 1; j < PUZZLE_SIZE; j += 1)
        {
            if (c[k] < c[j])
                k = j;
        }
        if (c[k])
        {
            d[(i * PUZZLE_SIZE) + k] = 16;
            p[(i * PUZZLE_SIZE) + k] += 2;
            goto calcRow;
        }
        for (j = 0; j < PUZZLE_SIZE; j += 1)
        {
            if (p[(i * PUZZLE_SIZE) + j] == 0)
                d[(i * PUZZLE_SIZE) + j] = 0;
        }
    calcCol:
        memset(c, 0, PUZZLE_SIZE);
        for (j = 0; j < PUZZLE_SIZE; j += 1)
        {
            for (k = j + 1; k < PUZZLE_SIZE; k += 1)
            {
                if ((d[(j * PUZZLE_SIZE) + i] == 4 && d[(k * PUZZLE_SIZE) + i] == 8) ||
                    (d[(j * PUZZLE_SIZE) + i] == 4 && d[(k * PUZZLE_SIZE) + i] == 0) ||
                    (d[(j * PUZZLE_SIZE) + i] == 0 && d[(k * PUZZLE_SIZE) + i] == 8))
                {

                    if (k - j <= max(p[(j * PUZZLE_SIZE) + i], p[(k * PUZZLE_SIZE) + i]))
                    {
                        c[j] += 1;
                        c[k] += 1;
                    }
                }
            }
        }
        k = 0;
        for (j = 1; j < PUZZLE_SIZE; j += 1)
        {
            if (c[k] < c[j])
                k = j;
        }
        if (c[k])
        {
            d[(k * PUZZLE_SIZE) + i] = 16;
            p[(k * PUZZLE_SIZE) + i] += 2;
            goto calcCol;
        }
        for (j = 0; j < PUZZLE_SIZE; j += 1)
        {
            if (p[(j * PUZZLE_SIZE) + i] == 0)
                d[(j * PUZZLE_SIZE) + i] = 0;
        }
    }
    free(c);

    k = 0;
    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        k += p[i];
    }
    return k;
}

unsigned int CalcDis_fast(unsigned char *current, const unsigned char *goal, unsigned int **buffers)
{
    unsigned int *p = buffers[0], *r = buffers[1];
    unsigned int i, j, k, m, n, x[2];

    CalcDis_slow(current, goal, buffers);

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        D1ToD2(i, x, x + 1);
        n = 0;
        for (j = 0; j < 3; j += 1)
        {
            if (x[0] - 1 + j >= PUZZLE_SIZE)
                continue;
            for (k = 0; k < 3; k += 1)
            {
                if (j != 1 && k != 1)
                    continue;
                if (x[1] - 1 + k >= PUZZLE_SIZE)
                    continue;
                m = p[(x[0] - 1 + j) * PUZZLE_SIZE + (x[1] - 1 + k)];
                if (m > n)
                    n = m;
            }
        }
        r[i] = n;
    }

    k = 0;
    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        k += r[i];
    }

    return k;
}
