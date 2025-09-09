#ifndef SERIAL_H
#define SERIAL_H

#include <stddef.h>
#include <stdint.h>

// Adresses of serial ports.
#define COM1 0x3f8
#define COM2 0x2f8

// Settings that can be used for initializing a serial port.
#define SERIAL_DATA_BITS_8 0b11
#define SERIAL_DATA_BITS_7 0b10
#define SERIAL_DATA_BITS_6 0b01
#define SERIAL_DATA_BITS_5 0b00

#define SERIAL_STOP_BITS_1 0
#define SERIAL_STOP_BITS_2 1

#define SERIAL_PARITY_NONE 0b000
#define SERIAL_PARITY_ODD 0b001
#define SERIAL_PARITY_EVEN 0b011
#define SERIAL_PARITY_MARK 0b101
#define SERIAL_PARITY_SPACE 0b111

#define SERIAL_CLEAR_TX_FIFO (1 << 2)
#define SERIAL_CLEAR_RX_FIFO (1 << 1)
#define SERIAL_ENABLE_FIFOS 1

// Status codes that can be returned by serial functions.
typedef enum serial_error_codes
{
    SERIAL_OK = 0,
    SERIAL_ERROR_BR_OUT_OF_BOUNDS,
    SERIAL_LOOPBACK_FAILED
} serial_error_codes_t;

serial_error_codes_t serial_init(uint16_t port, uint32_t baud_rate, uint8_t mode);

serial_error_codes_t serial_test(uint16_t port);

void serial_send_byte(uint16_t port, uint8_t data);
uint8_t serial_read_byte(uint16_t port);

void serial_print(uint16_t port, char *data);
void serial_print_line(uint16_t port, char *data);
void serial_read_line(uint16_t port, char *dest, size_t buffer_size);

#endif // SERIAL_H