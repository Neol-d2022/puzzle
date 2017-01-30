#ifndef CORE_H
#define CORE_H

typedef unsigned int (*CalcDisF)(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis);
unsigned int CalcDis_slow(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis);
unsigned int CalcDis_fast(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis);
unsigned int GetBufSize();
CalcDisF GetCalcFunc();

#endif
