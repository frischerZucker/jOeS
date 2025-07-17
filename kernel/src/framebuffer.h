#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <limine.h>

uint8_t framebuffer_clear(struct limine_framebuffer *framebuffer, uint32_t color);

#endif