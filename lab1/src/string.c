#include <string.h>

// https://code.woboq.org/userspace/glibc/string/strcmp.c.html
int strcmp(char *a, char *b) {
    const unsigned char *s1 = (const unsigned char *) a;
    const unsigned char *s2 = (const unsigned char *) b;
    unsigned char c1, c2;
    do {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);
    return c1 - c2;
}