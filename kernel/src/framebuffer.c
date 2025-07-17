#include <stddef.h>

#include <framebuffer.h>

#define COLOR_BLACK 0x000000
#define COLOR_WHITE 0xffffff

/*
    Fills the framebuffer with the given color.
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