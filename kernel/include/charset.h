/*!
    @file charset.h

    @brief Provides bitmaps for printable ASCII characters.

    @author frischerZucker
 */

#ifndef CHARSET_H
#define CHARSET_H

#include <stdint.h>

#define CHARACTER_WIDTH 10
#define CHARACTER_HEIGHT 10

#define TAB_WIDTH 4

/*!
    @brief Returns a characters offset in the charset.
*/
#define CHARSET_INDEX(c) (c - 32)

/*!
    Returns a characters bitmap from the charset.
*/
#define CHARSET(c) (charset[CHARSET_INDEX(c)])

/*!
    Bitmaps of all printable ASCII characters.
*/
extern uint8_t charset[95][CHARACTER_WIDTH * CHARACTER_HEIGHT];

#endif // CHARSET_H