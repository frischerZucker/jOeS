/*!
    @file port_io.h

    @brief Low-level I/O port access for x86 systems.
    
    Provides inline functions for reading and writing bytes to I/O ports using x86 assembly instructions.

    @author frischerZucker
 */

#ifndef PORT_IO_H
#define PORT_IO_H

#include <stdint.h>

/*!
    @brief Writes a byte to a port.
    
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

/*!
    @brief Reads a byte from a port.

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