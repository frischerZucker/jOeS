#include <stddef.h>

#include <charset.h>

#include <framebuffer.h>

#define COLOR_BLACK 0x000000
#define COLOR_WHITE 0xffffff

/*
    Fills the entire framebuffer with a solid color.

    Iterates over every pixel and writes the given 32-bit color to it.

    @param framebuffer Pointer to the limine_framebuffer to clear.
    @param color 24-bit RGB color value to fill the screen with.
*/
void framebuffer_clear(struct limine_framebuffer *framebuffer, uint32_t color)
{
    uint32_t *fb_ptr = framebuffer->address;

    size_t x, y;
    for (y = 0; y < framebuffer->height; y++)
    {
        for (x = 0; x < framebuffer->width; x++)
        {
            fb_ptr[y * (framebuffer->pitch / (framebuffer->bpp / 8)) + x] = color;
        }
    }
}

/*
    Draws a character on the framebuffer into the framebuffer at the given coordinates.

    Renders a character by scanning a doubled glyph cell. For each pixel in that cell, checks the bitmap from CHARSET(c). 
    If the bit is set, writes the color to the framebuffer at the computed (x,y) position.

    @param framebuffer Pointer to the limine_framebuffer to draw on.
    @param c ASCII character to render.
    @param start_x X-coordinate of the glyph's top-left corner.
    @param start_y Y-coordinate of the glyph's top left corner.
    @param color 24-bit RGB color value.
*/
void framebuffer_draw_char(struct limine_framebuffer *framebuffer, char c, uint16_t start_x, uint16_t start_y, uint32_t color)
{
    uint32_t *fb_ptr = framebuffer->address;

    size_t x, y;
    for (y = 0; y < CHAR_HEIGHT * 2; y++)
    {
        for (x = 0; x < CHAR_WIDTH * 2; x++)
        {
            // if the pixels bit in the bitmap is set, a pixel is drawn on the framebuffer
            if (CHARSET(c)[(x / 2) + (y / 2) * CHAR_WIDTH] == 1)
            {
                fb_ptr[(start_y + y) * (framebuffer->pitch / (framebuffer->bpp / 8)) + start_x + x] = color;
            }
        }
    }
}