#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>
#include <stdint.h>

#include <limine.h>

struct terminal
{
    uint8_t cursor_x;
    uint8_t cursor_y;

    uint8_t char_w;
    uint8_t char_h;

    uint32_t fg_color;

    struct limine_framebuffer *framebuffer;
};

void terminal_init(struct limine_framebuffer *framebuffer, uint8_t char_w, uint8_t char_h);

uint8_t terminal_set_color(uint32_t color);

uint8_t terminal_put_char(uint8_t c);
uint8_t terminal_write_string(char *str, size_t len);

#endif