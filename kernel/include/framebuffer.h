/*!
    @file framebuffer.h

    @brief Basic interface for drawing to a framebuffer.
    
    Provides functions to draw characters on a framebuffer and clear it by filling it with a solid color.

    @author frischerZucker
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <limine.h>

/*!
    @brief Fills the entire framebuffer with a solid color.

    Iterates over every pixel and sets it to the color.

    @param framebuffer Pointer to the limine_framebuffer to clear.
    @param color 24-bit RGB color value to fill the screen with.
*/
void framebuffer_clear(struct limine_framebuffer *framebuffer, uint32_t color);

/*!
    @brief Draws a character on the framebuffer into the framebuffer at the given coordinates.

    Renders a character by scanning a doubled glyph cell. For each pixel in that cell, checks the bitmap from CHARSET(c). 
    If the bit is set, writes the color to the framebuffer at the computed (x,y) position.

    @param framebuffer Pointer to the limine_framebuffer to draw on.
    @param c ASCII character to render.
    @param start_x X-coordinate of the glyph's top-left corner.
    @param start_y Y-coordinate of the glyph's top left corner.
    @param color 24-bit RGB color value.
*/
void framebuffer_draw_char(struct limine_framebuffer *framebuffer, char c, uint16_t start_x, uint16_t start_y, uint32_t color);

#endif // FRAMEBUFFER_H