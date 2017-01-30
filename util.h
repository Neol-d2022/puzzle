#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

void D1ToD2(unsigned int i, unsigned int *row, unsigned int *col);
void GetMax(unsigned int *max, unsigned int *min);
unsigned int D2Diff(unsigned int i, unsigned int j);
unsigned int FindInPlate(const unsigned char *p, unsigned int chr);
void swap(unsigned char *a, unsigned char *b);
void printPlate(const unsigned char *p, FILE *f);
void move(unsigned char *p, unsigned int dChr, unsigned int *x);

#endif
