#ifndef CORE_H
#define CORE_H

typedef unsigned int (*CalcDisF)(unsigned char *current, const unsigned char *goal, unsigned char *buffers, unsigned int *blankDis);
unsigned int GetBufSize();
CalcDisF GetCalcFunc();
const char *GetCalcFuncStr();

#endif
