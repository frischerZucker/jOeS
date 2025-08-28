#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *str);

// Non-standard functions
void strrev(char * str);
void itoa(long number, char *str, int base);
void utoa(unsigned long number, char *str, int base);

#endif // STRING_H