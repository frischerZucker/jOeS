#ifndef CHARSET_H
#define CHARSET_H

#include <stdint.h>

#define CHAR_WIDTH 10
#define CHAR_HEIGHT 10

#define CHARSET_INDEX(c) (c-32)

#define CHARSET(c) (charset[CHARSET_INDEX(c)])

extern uint8_t charset[95][CHAR_WIDTH*CHAR_HEIGHT];

#endif