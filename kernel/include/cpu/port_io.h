#ifndef PORT_IO_H
#define PORT_IO_H

#include <stdint.h>

/*
    Writes a byte to a port.

    @param port Port to write data to.
    @param data Data to write to port.
*/
static inline void port_write_byte(uint16_t port, uint8_t data)
{
    asm volatile (
        "out %b0, %w1"
        :
        : "a"(data), "Nd"(port)
        : "memory"
    );
}

/*
    Reads a byte from a port.

    @param port Port to read data from.
    @returns A byte read from the port.
*/
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

#endif // PORT_IO_H