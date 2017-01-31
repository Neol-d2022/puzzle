#include "main.h"
#include "util.h"

unsigned int minA(unsigned int *a, unsigned int ud)
{
    unsigned int n = (unsigned int)-1;

    switch (ud)
    {
    case 1:
        if (n > a[0])
            n = a[0];
        if (n > a[1])
            n = a[1];
        if (n > a[3])
            n = a[3];
        break;
    case 2:
        if (n > a[0])
            n = a[0];
        if (n > a[1])
            n = a[1];
        if (n > a[2])
            n = a[2];
        break;
    case 4:
        if (n > a[1])
            n = a[1];
        if (n > a[2])
            n = a[2];
        if (n > a[3])
            n = a[3];
        break;
    case 8:
        if (n > a[0])
            n = a[0];
        if (n > a[2])
            n = a[2];
        if (n > a[3])
            n = a[3];
        break;
    default:
        if (n > a[0])
            n = a[0];
        if (n > a[1])
            n = a[1];
        if (n > a[2])
            n = a[2];
        if (n > a[3])
            n = a[3];
        break;
    }

    return n;
}

void UpdateShortest(unsigned int *d, unsigned int *a, unsigned int i)
{
    unsigned int *k = a + i * 4;
    unsigned int j = d[i];
    unsigned int r, u = 0;
    ;
    unsigned int x[2];

    if (j & 32)
    {
        return;
    }
    D1ToD2(i, x, x + 1);
    if (x[0] == 0)
    {
        r = minA(a + ((i + PUZZLE_SIZE) * 4), 4);
        if (j & 4)
        {
            if (k[1] > r + 1 && r != (unsigned int)-1)
            {
                k[1] = r + 1;
                u = 1;
            }
        }
    }
    else if (x[0] == PUZZLE_SIZE - 1)
    {
        r = minA(a + ((i - PUZZLE_SIZE) * 4), 8);
        if (j & 8)
        {
            if (k[0] > r + 1 && r != (unsigned int)-1)
            {
                k[0] = r + 1;
                u = 1;
            }
        }
    }
    else
    {
        r = minA(a + ((i + PUZZLE_SIZE) * 4), 4);
        if (j & 4)
        {
            if (k[1] > r + 1 && r != (unsigned int)-1)
            {
                k[1] = r + 1;
                u = 1;
            }
        }
        r = minA(a + ((i - PUZZLE_SIZE) * 4), 8);
        if (j & 8)
        {
            if (k[0] > r + 1 && r != (unsigned int)-1)
            {
                k[0] = r + 1;
                u = 1;
            }
        }
    }

    if (x[1] == 0)
    {
        r = minA(a + ((i + 1) * 4), 1);
        if (j & 1)
        {
            if (k[3] > r + 1 && r != (unsigned int)-1)
            {
                k[3] = r + 1;
                u = 1;
            }
        }
    }
    else if (x[1] == PUZZLE_SIZE - 1)
    {
        r = minA(a + ((i - 1) * 4), 2);
        if (j & 2)
        {
            if (k[2] > r + 1 && r != (unsigned int)-1)
            {
                k[2] = r + 1;
                u = 1;
            }
        }
    }
    else
    {
        r = minA(a + ((i + 1) * 4), 1);
        if (j & 1)
        {
            if (k[3] > r + 1 && r != (unsigned int)-1)
            {
                k[3] = r + 1;
                u = 1;
            }
        }
        r = minA(a + ((i - 1) * 4), 2);
        if (j & 2)
        {
            if (k[2] > r + 1 && r != (unsigned int)-1)
            {
                k[2] = r + 1;
                u = 1;
            }
        }
    }

    if (u)
    {
        if (x[0] == 0)
            UpdateShortest(d, a, i + PUZZLE_SIZE);
        else if (x[0] == PUZZLE_SIZE - 1)
            UpdateShortest(d, a, i - PUZZLE_SIZE);
        else
        {
            UpdateShortest(d, a, i + PUZZLE_SIZE);
            UpdateShortest(d, a, i - PUZZLE_SIZE);
        }
        if (x[1] == 0)
            UpdateShortest(d, a, i + 1);
        else if (x[1] == PUZZLE_SIZE - 1)
            UpdateShortest(d, a, i - 1);
        else
        {
            UpdateShortest(d, a, i + 1);
            UpdateShortest(d, a, i - 1);
        }
    }
}