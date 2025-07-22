#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <limine.h>

uint8_t framebuffer_clear(struct limine_framebuffer *framebuffer, uint32_t color);

uint8_t framebuffer_draw_char(struct limine_framebuffer *framebuffer, char c, uint16_t start_x, uint16_t start_y, uint32_t color);

#endif