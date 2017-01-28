#ifndef INPUT_H
#define INPUT_H

void TrimNewline(char *buf);
void tokenize(const char *str, int sepChr, char ***arr, size_t *size, int ignore);
void freeToks(char **toks, size_t n);
int GetPlate(unsigned char *p, FILE *input);

#endif
