#ifndef PORT_IO_H
#define PORT_IO_H

#include <stdint.h>

static inline void port_write_byte(uint16_t port, uint8_t data)
{
    asm volatile (
        "out %b0, %w1"
        :
        : "a"(data), "Nd"(port)
        : "memory"
    );
}

static inline uint8_t port_read_byte(uint16_t port)
{
    uint8_t data;

    asm volatile (
        "in %w1, %b0"
        : "=a"(data)
        : "Nd"(port)
        : "memory"
    );

    return data;
}

#endif