#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core.h"
#include "util.h"
#include "main.h"
#include "avl.h"

static int cmpUint(void *a, void *b)
{
    unsigned int c, d;

    c = *((unsigned int *)a);
    d = *((unsigned int *)b);

    if (c > d)
        return 1;
    else if (c < d)
        return -1;
    else
        return 0;
}

static int CheckConflict(unsigned int *d, unsigned int i, unsigned int lastD)
{
    unsigned int j = d[i];
    int r;

    if (j & 32)
    {
        if (j & 15)
        {
            if (j & lastD)
                return 0;
            else
            {
                d[i] |= lastD;
                return 1;
            }
        }
        else
        {
            d[i] |= lastD;
            return 0;
        }
    }
    if (j == 0)
        return 1;
    d[i] = 0;
    if (j & 1)
    {
        r = CheckConflict(d, i + 1, 1);
        if (r == 0)
        {
            d[i] = j;
            return 0;
        }
    }
    if (j & 2)
    {
        r = CheckConflict(d, i - 1, 2);
        if (r == 0)
        {
            d[i] = j;
            return 0;
        }
    }
    if (j & 4)
    {
        r = CheckConflict(d, i + PUZZLE_SIZE, 4);
        if (r == 0)
        {
            d[i] = j;
            return 0;
        }
    }
    if (j & 8)
    {
        r = CheckConflict(d, i - PUZZLE_SIZE, 8);
        if (r == 0)
        {
            d[i] = j;
            return 0;
        }
    }
    d[i] = j;
    return 1;
}

static void _traverse(void *dataPtr, void *paramInOut)
{
    if (paramInOut)
        *((int *)paramInOut) = 0;
    *((unsigned int *)dataPtr) = PUZZLE_SIZE * PUZZLE_SIZE;
}

unsigned int CalcDis_slow(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis)
{
    unsigned int *p = (unsigned int *)buffers;
    unsigned int *d = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE;
    unsigned int *s = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE * 2;
    AVL_TREE *t;
    unsigned int i, j, k, l, x[2], g;
    int r;

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if (current[i] == 0)
        {
            p[i] = 0;
            d[i] = 32;
            s[i] = FindInPlate(goal, 0);
            if (blankDis)
                *blankDis = s[i];
            continue;
        }
        for (j = 0; j < PUZZLE_SIZE * PUZZLE_SIZE; j += 1)
        {
            if (current[i] == goal[j])
            {
                s[i] = j;
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

    g = 0;
    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if (d[i] == 32)
            continue;
        if (s[i] == PUZZLE_SIZE * PUZZLE_SIZE)
            continue;
        if (s[i] == i)
            continue;
        if (d[s[i]] == 32)
            continue;

        j = i;
        t = AVL_Create(cmpUint, 0);
        while ((r = AVL_Insert(t, s + j)) != 0)
            j = s[j];
        AVL_Traverse(t, 0, _traverse);
        g += 1;
        AVL_Destroy(t);
    }

    j = PUZZLE_SIZE * PUZZLE_SIZE;
    l = FindInPlate(current, 0);
    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if (p[i] == 0 || i == l)
            continue;
        D1ToD2(i, x, x + 1);
        if (x[0] == 0)
        {
            if (d[i] & 4)
            {
                k = D2Diff(l, (x[0] + 1) * PUZZLE_SIZE + x[1]);
                if (k < j)
                    j = k;
            }
        }
        else if (x[0] == PUZZLE_SIZE - 1)
        {
            if (d[i] & 8)
            {
                k = D2Diff(l, (x[0] - 1) * PUZZLE_SIZE + x[1]);
                if (k < j)
                    j = k;
            }
        }
        else
        {
            if (d[i] & 4)
            {
                k = D2Diff(l, (x[0] + 1) * PUZZLE_SIZE + x[1]);
                if (k < j)
                    j = k;
            }
            if (d[i] & 8)
            {
                k = D2Diff(l, (x[0] - 1) * PUZZLE_SIZE + x[1]);
                if (k < j)
                    j = k;
            }
        }
        if (x[1] == 0)
        {
            if (d[i] & 1)
            {
                k = D2Diff(l, x[0] * PUZZLE_SIZE + (x[1] + 1));
                if (k < j)
                    j = k;
            }
        }
        else if (x[1] == PUZZLE_SIZE - 1)
        {
            if (d[i] & 2)
            {
                k = D2Diff(l, x[0] * PUZZLE_SIZE + (x[1] - 1));
                if (k < j)
                    j = k;
            }
        }
        else
        {
            if (d[i] & 1)
            {
                k = D2Diff(l, x[0] * PUZZLE_SIZE + (x[1] + 1));
                if (k < j)
                    j = k;
            }
            if (d[i] & 2)
            {
                k = D2Diff(l, x[0] * PUZZLE_SIZE + (x[1] - 1));
                if (k < j)
                    j = k;
            }
        }
    }
    if (j == PUZZLE_SIZE * PUZZLE_SIZE)
        j = 0;

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if (p[i] == 0 || i == l)
            continue;
        if (CheckConflict(d, i, 0))
        {
            g += 1;
            break;
        }
    }

    k = 0;
    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        k += p[i];
    }
    return k + j + g;
}

unsigned int CalcDis_fast(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis)
{
    unsigned int *p = (unsigned int *)buffers;
    unsigned int *r = (unsigned int *)(buffers) + PUZZLE_SIZE * PUZZLE_SIZE;
    unsigned int i, j, k, m, n, x[2];

    CalcDis_slow(current, goal, buffers, blankDis);

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
    return PUZZLE_SIZE * PUZZLE_SIZE * 3 * sizeof(unsigned int);
}

CalcDisF GetCalcFunc()
{
    if (LEVEL == 0)
        return CalcDis_fast;
    else if (LEVEL == 1)
        return CalcDis_slow;
    else
        return (CalcDisF)0;
}
