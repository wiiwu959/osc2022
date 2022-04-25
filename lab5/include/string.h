#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

int strlen(char* str);
int strcmp(char *a, char *b);
int strncmp(char *a, char *b, size_t n);

int atoi(char* str);

void *memset (void *dest, int val, size_t len);

#endif  /*_STRING_H */

