#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "input.h"
#include "main.h"

void TrimNewline(char *buf)
{
    char *p;

    p = strchr(buf, '\n');
    if (p)
        *p = '\0';
    p = strchr(buf, '\r');
    if (p)
        *p = '\0';
}

void tokenize(const char *str, int sepChr, char ***arr, size_t *size, int ignore)
{
    const char *last, *sep, *end;
    char **a, *subs;
    size_t n, s;

    end = str + strlen(str);
    last = str - 1;
    n = 0;
    sep = str;
    a = (char **)malloc(0);
    while ((sep = strchr(sep, sepChr)))
    {
        if (((size_t)sep == (size_t)(last + 1)) && ignore)
        {
            last = sep;
            sep += 1;
            continue;
        }
        else
        {
            s = (size_t)sep - (size_t)(last + 1);
            subs = (char *)malloc(s + 1);
            subs[s] = '\0';
            memcpy(subs, last + 1, s);
            a = (char **)realloc(a, sizeof(*a) * (n + 1));
            a[n] = subs;
            n += 1;
        }
        last = sep;
        sep += 1;
    }

    if (((size_t)end != (size_t)(last + 1)) || !ignore)
    {
        s = (size_t)end - (size_t)(last + 1);
        subs = (char *)malloc(s + 1);
        subs[s] = '\0';
        memcpy(subs, last + 1, s);
        a = (char **)realloc(a, sizeof(*a) * (n + 1));
        a[n] = subs;
        n += 1;
    }

    *size = n;
    *arr = a;
}

void freeToks(char **toks, size_t n)
{
    size_t i;
    for (i = 0; i < n; i += 1)
        free(toks[i]);
    free(toks);
}

int GetPlate(unsigned char *p, FILE *input)
{
    char buf[256], **toks;
    unsigned char *v;
    size_t ntoks;
    unsigned int i, j, in;
    v = (unsigned char *)malloc(PUZZLE_SIZE * PUZZLE_SIZE);
    memset(v, PUZZLE_SIZE * PUZZLE_SIZE, PUZZLE_SIZE * PUZZLE_SIZE);
    for (i = 0; i < PUZZLE_SIZE; i += 1)
    {
        if (!fgets(buf, sizeof(buf), input))
        {
            free(v);
            return 1;
        }

        TrimNewline(buf);

        tokenize(buf, ' ', &toks, &ntoks, 1);
        if (ntoks != PUZZLE_SIZE)
        {
            freeToks(toks, ntoks);
            free(v);
            return 2;
        }

        for (j = 0; j < PUZZLE_SIZE; j += 1)
        {
            if (sscanf(toks[j], "%u", &in) != 1)
            {
                freeToks(toks, ntoks);
                free(v);
                return 3;
            }
            if (in >= PUZZLE_SIZE * PUZZLE_SIZE)
            {
                freeToks(toks, ntoks);
                free(v);
                return 3;
            }
            if (v[in] != PUZZLE_SIZE * PUZZLE_SIZE)
            {
                freeToks(toks, ntoks);
                free(v);
                return 3;
            }
            v[in] = i * PUZZLE_SIZE + j;
            p[i * PUZZLE_SIZE + j] = (unsigned char)in;
        }

        freeToks(toks, ntoks);
    }
    free(v);
    return 0;
}
