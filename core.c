#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core.h"
#include "util.h"
#include "main.h"
#include "avl.h"
#include "dijkstra.h"

static unsigned int max(unsigned int a, unsigned int b)
{
    return (a > b) ? a : b;
}

unsigned int CalcDis_swap(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis)
{
    unsigned int *g = (unsigned int *)buffers;
    unsigned int *s = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE * 2;
    unsigned int i, j, k;

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if (current[i] == 0)
        {
            s[i] = FindInPlate(goal, 0);
            if (blankDis)
                *blankDis = D2Diff(s[i], i);
            continue;
        }
        for (j = 0; j < PUZZLE_SIZE * PUZZLE_SIZE; j += 1)
        {
            if (current[i] == goal[j])
            {
                s[i] = j;
                break;
            }
        }
        if (j >= PUZZLE_SIZE * PUZZLE_SIZE)
            return -1;
    }

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
        g[i] = i;

    k = 0;
    while (memcmp(g, s, PUZZLE_SIZE * PUZZLE_SIZE * sizeof(*g)))
    {
        for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
        {
            if (s[i] != i)
            {
                j = s[s[i]];
                s[s[i]] = s[i];
                s[i] = j;
                k += 1;
                break;
            }
        }
    }

    return k;
}

unsigned int CalcDis_mhtd(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis)
{
    unsigned int *p = (unsigned int *)buffers;
    unsigned int *d = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE;
    unsigned int i, j, k;

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if (current[i] == 0)
        {
            p[i] = 0;
            d[i] = 32;
            if (blankDis)
                *blankDis = D2Diff(FindInPlate(goal, 0), i);
            continue;
        }
        for (j = 0; j < PUZZLE_SIZE * PUZZLE_SIZE; j += 1)
        {
            if (current[i] == goal[j])
            {
                p[i] = D2Diff(i, j);

                d[i] = 0;
                if (p[i])
                {
                    if (i % PUZZLE_SIZE < j % PUZZLE_SIZE)
                    {
                        d[i] |= 1; //Right
                    }
                    else if (i % PUZZLE_SIZE > j % PUZZLE_SIZE)
                    {
                        d[i] |= 2; //Left
                    }

                    if (i / PUZZLE_SIZE > j / PUZZLE_SIZE)
                    {
                        d[i] |= 8; //Up
                    }
                    else if (i / PUZZLE_SIZE < j / PUZZLE_SIZE)
                    {
                        d[i] |= 4; //Down
                    }
                }
                break;
            }
        }
        if (j >= PUZZLE_SIZE * PUZZLE_SIZE)
            return -1;
    }

    k = 0;
    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        k += p[i];
    }
    return k;
}

unsigned int CalcDis_mhtl(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis)
{
    unsigned int *p = (unsigned int *)buffers;
    unsigned int *d = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE;
    unsigned int i, j, k, g, r;

    r = CalcDis_mhtd(current, goal, buffers, blankDis);

    g = 0;
    for (i = 0; i < PUZZLE_SIZE; i += 1)
    {
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
                        g += 2;
                        break;
                    }
                }
            }
        }
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
                        g += 2;
                        break;
                    }
                }
            }
        }
    }

    return r + g;
}

unsigned int CalcDis_djst(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis)
{
    unsigned int *p = (unsigned int *)buffers;
    unsigned int *d = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE;
    unsigned int *s = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE * 2;
    unsigned int *a = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE * 3;
    unsigned int i, j, k, l, x[2], r;

    r = CalcDis_mhtl(current, goal, buffers, blankDis);

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        for (j = 0; j < PUZZLE_SIZE * PUZZLE_SIZE; j += 1)
        {
            if (current[i] == goal[j])
            {
                s[i] = j;
                break;
            }
        }
    }

    l = FindInPlate(current, 0);

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
        for (j = 0; j < 4; j += 1)
            a[i * 4 + j] = PUZZLE_SIZE * PUZZLE_SIZE;

    a[l * 4 + 0] = a[l * 4 + 1] = a[l * 4 + 2] = a[l * 4 + 3] = 0;

    D1ToD2(l, x, x + 1);
    if (x[0] == 0)
        UpdateShortest(d, a, l + PUZZLE_SIZE);
    else if (x[0] == PUZZLE_SIZE - 1)
        UpdateShortest(d, a, l - PUZZLE_SIZE);
    else
    {
        UpdateShortest(d, a, l + PUZZLE_SIZE);
        UpdateShortest(d, a, l - PUZZLE_SIZE);
    }
    if (x[1] == 0)
        UpdateShortest(d, a, l + 1);
    else if (x[1] == PUZZLE_SIZE - 1)
        UpdateShortest(d, a, l - 1);
    else
    {
        UpdateShortest(d, a, l + 1);
        UpdateShortest(d, a, l - 1);
    }

    printf("\nP = \n");
    for (j = 0; j < PUZZLE_SIZE; j += 1)
    {
        for (k = 0; k < PUZZLE_SIZE - 1; k += 1)
            printf("%2u ", p[j * PUZZLE_SIZE + k]);
        printf("%2u\n", p[j * PUZZLE_SIZE + k]);
    }
    printf("D = \n");
    for (j = 0; j < PUZZLE_SIZE; j += 1)
    {
        for (k = 0; k < PUZZLE_SIZE - 1; k += 1)
            printf("%2u ", d[j * PUZZLE_SIZE + k]);
        printf("%2u\n", d[j * PUZZLE_SIZE + k]);
    }
    printf("S = \n");
    for (j = 0; j < PUZZLE_SIZE; j += 1)
    {
        for (k = 0; k < PUZZLE_SIZE - 1; k += 1)
            printf("%2u ", s[j * PUZZLE_SIZE + k]);
        printf("%2u\n", s[j * PUZZLE_SIZE + k]);
    }
    printf("h = %u\nA = \n", r);
    for (j = 0; j < PUZZLE_SIZE; j += 1)
    {
        for (k = 0; k < PUZZLE_SIZE - 1; k += 1)
            printf("%2u ", (p[(j * PUZZLE_SIZE) + k]) ? minA(a + ((j * PUZZLE_SIZE) + k) * 4, 0) : 0);
        printf("%2u\n", (p[(j * PUZZLE_SIZE) + k]) ? minA(a + ((j * PUZZLE_SIZE) + k) * 4, 0) : 0);
    }

    return r;
}

unsigned int CalcDis_fast(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis)
{
    unsigned int *p = (unsigned int *)buffers;
    unsigned int *r = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE;
    unsigned int i, j, k, m, n, x[2];

    CalcDis_mhtl(current, goal, buffers, blankDis);

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

unsigned int GetBufSize()
{
    return PUZZLE_SIZE * PUZZLE_SIZE * 7 * sizeof(unsigned int);
}

CalcDisF GetCalcFunc()
{
    if (LEVEL == 0)
        return CalcDis_fast;
    else if (LEVEL == 1)
        return CalcDis_mhtd;
    else if (LEVEL == 2)
        return CalcDis_mhtl;
    else if (LEVEL == 3)
        return CalcDis_swap;
    else if (LEVEL == 4)
        return CalcDis_djst;
    else
        return (CalcDisF)0;
}

const char *GetCalcFuncStr()
{
    if (LEVEL == 0)
        return "(fast) FAST";
    else if (LEVEL == 1)
        return "(mthd) Manhattan Distance";
    else if (LEVEL == 2)
        return "(mthl) Manhattan Dis. w/ Linear conflict";
    else if (LEVEL == 3)
        return "(swap) Min N-Swaps";
    else if (LEVEL == 4)
        return "(dijk) Dijkstra (In development)";
    else
        return "UNKNOWN";
}
