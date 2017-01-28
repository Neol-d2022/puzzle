#include "core.h"
#include "util.h"
#include "main.h"

unsigned int CalcDis(PLATE *current, const PLATE *goal, unsigned int **buffers)
{
    unsigned int *p = buffers[0], *r = buffers[1];
    unsigned int i, j, k, m, n, x[2];

    for (i = 0; i < PUZZLE_SIZE * PUZZLE_SIZE; i += 1)
    {
        if ((current->s)[i] == 0)
        {
            p[i] = 0;
            continue;
        }
        for (j = 0; j < PUZZLE_SIZE * PUZZLE_SIZE; j += 1)
        {
            if ((current->s)[i] == (goal->s)[j])
            {
                p[i] = D2Diff(i, j);
                break;
            }
        }
        if (j >= PUZZLE_SIZE * PUZZLE_SIZE)
            return -1;
    }

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
