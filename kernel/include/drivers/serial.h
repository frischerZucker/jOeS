/*!
    @file serial.h

    @brief Serial port driver.
    
    Provides functions initializing, testing, sending, and receiving data via serial ports.
    Supports configurable baud rate, parity, stop bits, and character length.
    Includes basic error handling and loopback testing for reliability.

    @author frischerZucker
 */

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

/*!
    @brief Error codes used by the driver.
*/
typedef enum serial_error_codes
{
    SERIAL_OK = 0,
    SERIAL_ERROR_BR_OUT_OF_BOUNDS,
    SERIAL_LOOPBACK_FAILED
} serial_error_codes_t;

/*!
    @brief Initializes a serial port.

    Initializes and tests a serial port with the given baud rate and mode.

    @param port Serial port to initialize.
    @param baud_rate Baud rate the serial port should run at.
    @param mode Mode the serial port should use (stop bits, parity, character length).
    @returns SERIAL_ERROR_BR_OUT_OF_BOUNDS if baud rate is too low / high,
             or SERIAL_LOOPBACK_FAILED if loopback test fails, otherwise SERIAL_OK.
*/
serial_error_codes_t serial_init(uint16_t port, uint32_t baud_rate, uint8_t mode);

/*!
    @brief Checks if a serial port is working.

    Sets the serial port to loopback mode, sends a byte and checks if it receives the same byte.

    @param port Serial port to test.
    @returns SERIAL_OK if the sent byte is received, otherwise SERIAL_LOOPBACK_FAILED.
*/
serial_error_codes_t serial_test(uint16_t port);

/*!
    @brief Sends a byte via the serial port.

    Waits until the transmit buffer is empty and sends a byte.

    @param port Serial port.
    @param data Byte to send.
*/
void serial_send_byte(uint16_t port, uint8_t data);

/*!
    @brief Reads a byte from the serial port.

    Waits until there is data in the receive buffer and reads a byte.

    @param port Serial port.
    @returns A byte read from the serial port.
*/
uint8_t serial_read_byte(uint16_t port);

/*!
    @brief Sends a string via the serial port.

    @param port Serial port.
    @param data Pointer to the string to be sent.
*/
void serial_print(uint16_t port, char *data);

/*!
    @brief Sends a string via the serial port adding a new line character ('\n') at the end.

    @param port Serial port.
    @param data Pointer to the string to be sent.
*/
void serial_print_line(uint16_t port, char *data);

/*!
    @brief Reads a line from the serial port.

    Reads bytes from the serial port until a new line character ('\n') is received or the buffers length is reached.
    
    @param port Serial Port.
    @param dest Pointer to the location the string should be written to.
    @param buffer_size Size of the buffer pointed to by dest.
*/
void serial_read_line(uint16_t port, char *dest, size_t buffer_size);

#endif // SERIAL_H