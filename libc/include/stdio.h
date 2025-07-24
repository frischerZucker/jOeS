#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>

// Maximum characters needed to represent an int.
#define INT_MAX_CHARS 12

#define STATE_NORMAL 0
#define STATE_FORMAT 1

#define FORMAT_SPECIFIER '%'
#define FORMAT_STRING 's'   
#define FORMAT_CHAR 'c'
#define FORMAT_INT1 'd'
#define FORMAT_INT2 'i'

size_t printf(char *str, ...);

#endif // STDIO_H