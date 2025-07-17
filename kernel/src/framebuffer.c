#include <stddef.h>

#include <charset.h>

#include <framebuffer.h>

#define COLOR_BLACK 0x000000
#define COLOR_WHITE 0xffffff

/*
    Fills the framebuffer with a solid color.

    Fills the entire framebuffer with the given color.

    args:
        struct limine_framebuffer *framebuffer -> framebuffer to fill
        uint32_t color -> color to fill it with
    
    returns
        0 if success
*/
uint8_t framebuffer_clear(struct limine_framebuffer *framebuffer, uint32_t color)
{
    uint32_t *fb_ptr = framebuffer->address;

    for (size_t y = 0; y < framebuffer->height; y++)
    {
        for (size_t x = 0; x < framebuffer->width; x++)
        {
            fb_ptr[y * (framebuffer->pitch / (framebuffer->bpp / 8)) + x] = color;
        }
    }

    return 0;
}

/*
    Draws a character on the framebuffer.

    Draws a 20x20 pixel character in the given color at the given coordinates on the framebuffer.

    args:
        struct limine_framebuffer *framebuffer -> framebuffer to draw on
        char c -> character to draw
        uint16_t start_x -> x coordinate of the upper left corner of where c should be printed
        uint16_t start_y -> y coordinate of the upper left corner of where c should be printed
        uint32_t color -> color of the character
    
    returns:
        0 if success
*/
uint8_t framebuffer_draw_char(struct limine_framebuffer *framebuffer, char c, uint16_t start_x, uint16_t start_y, uint32_t color)
{
    uint32_t *fb_ptr = framebuffer->address;

    for (uint16_t y = 0; y < CHAR_HEIGHT*2; y++)
    {
        for (uint16_t x = 0; x < CHAR_WIDTH*2; x++)
        {
            // if the current pixels bit in the bitmap equals '1', a pixel is drawn on the framebuffer
            if (CHARSET(c)[(x / 2) + (y / 2)*CHAR_WIDTH] == 1)
            {
                fb_ptr[(start_y + y) * (framebuffer->pitch / (framebuffer->bpp / 8)) + start_x + x] = color;
            }            
        }
    }

    return 0;
}