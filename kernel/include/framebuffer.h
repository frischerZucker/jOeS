#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <limine.h>

void framebuffer_clear(struct limine_framebuffer *framebuffer, uint32_t color);

void framebuffer_draw_char(struct limine_framebuffer *framebuffer, char c, uint16_t start_x, uint16_t start_y, uint32_t color);

#endif