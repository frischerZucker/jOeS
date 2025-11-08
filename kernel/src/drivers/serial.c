#include "drivers/serial.h"

#include "stdio.h"
#include "string.h"

#include "cpu/port_io.h"
#include "logging.h"

// Register offsets.
#define SERIAL_INT_ENABLE_REG(p) (p + 1)
#define SERIAL_FIFO_CONTROL_REG(p) (p + 2)
#define SERIAL_LINE_CONTROL_REG(p) (p + 3)
#define SERIAL_MODEM_CONTROL_REG(p) (p + 4)
#define SERIAL_LINE_STATUS_REG(p) (p + 5)
#define SERIAL_MODEM_STATUS_REG(p) (p + 6)
#define SERIAL_SCRATCH_REG(p) (p + 7)
#define SERIAL_BR_LOW_REG(p) (p)
#define SERIAL_BR_HIGH_REG(p) (p + 1)
#define SERIAL_INT_ID_REG(p) (p + 2)

#define SERIAL_LOOPBACK_MODE 0b00011011
#define SERIAL_NORMAL_MODE 0b00001011

#define SERIAL_BAUD_RATE_MAX 115200

#define SERIAL_TEST_BYTE 0x69

/*!
    @brief Sends a byte via the serial port.

    Waits until the transmit buffer is empty and sends a byte.

    @param port Serial port.
    @param data Byte to send.
*/
void serial_send_byte(uint16_t port, uint8_t data)
{
    // Wait for the TX buffer to be empty, so that new data can be written into it.
    while ((port_read_byte(SERIAL_LINE_STATUS_REG(port)) & (1 << 5)) == 0);    

    port_write_byte(port, data);
}

/*!
    @brief Reads a byte from the serial port.

    Waits until there is data in the receive buffer and reads a byte.

    @param port Serial port.
    @returns A byte read from the serial port.
*/
uint8_t serial_read_byte(uint16_t port)
{
    // Wait until there is data that can be read.
    while ((port_read_byte(SERIAL_LINE_STATUS_REG(port)) & 1) == 0);
    
    return port_read_byte(port);
}

/*!
    @brief Sends a string via the serial port.

    @param port Serial port.
    @param data Pointer to the string to be sent.
*/
void serial_print(uint16_t port, char *data)
{
    size_t len = strlen(data);
    for (size_t i = 0; i < len; i++)
    {
        serial_send_byte(port, data[i]);
    }
}

/*!
    @brief Sends a string via the serial port adding a new line character ('\n') at the end.

    @param port Serial port.
    @param data Pointer to the string to be sent.
*/
void serial_print_line(uint16_t port, char *data)
{
    serial_print(port, data);
    serial_send_byte(port, '\n');
}

/*!
    @brief Reads a line from the serial port.

    Reads bytes from the serial port until a new line character ('\n') is received or the buffers length is reached.
    
    @param port Serial Port.
    @param dest Pointer to the location the string should be written to.
    @param buffer_size Size of the buffer pointed to by dest.
*/
void serial_read_line(uint16_t port, char *dest, size_t buffer_size)
{
    size_t i = 0;
    
    // Read bytes until ether a new line character ('\n') is received or the buffers length is reached.
    dest[i] = serial_read_byte(port);
    i = i + 1;
    while (i < buffer_size - 1)
    {
        dest[i] = serial_read_byte(port);
        
        if (dest[i] == '\n') break;

        i = i + 1;
    }
    
    dest[i + 1] = '\0';
}

/*!
    @brief Initializes a serial port.

    Initializes and tests a serial port with the given baud rate and mode.

    @param port Serial port to initialize.
    @param baud_rate Baud rate the serial port should run at.
    @param mode Mode the serial port should use (stop bits, parity, character length).
    @returns SERIAL_ERROR_BR_OUT_OF_BOUNDS if baud rate is too low / high,
             or SERIAL_LOOPBACK_FAILED if loopback test fails, otherwise SERIAL_OK.
*/
serial_error_codes_t serial_init(uint16_t port, uint32_t baud_rate, uint8_t mode)
{
    LOG_INFO("Initializing serial driver...");

    // Check if the baud rate is ok, return if isn't..
    if (baud_rate > SERIAL_BAUD_RATE_MAX || baud_rate < 1)
    {
        LOG_ERROR("Baud rate out of bounds! (br=%d)", baud_rate);
        return SERIAL_ERROR_BR_OUT_OF_BOUNDS;
    }
    
    // Disable all serial interrupts.
    port_write_byte(SERIAL_INT_ENABLE_REG(port), 0);

    // Setup the baud rate.
    uint16_t divisor = SERIAL_BAUD_RATE_MAX / baud_rate;
    port_write_byte(SERIAL_LINE_CONTROL_REG(port), (1 << 7)); // Set the DLAB bit.
    port_write_byte(SERIAL_BR_LOW_REG(port), divisor & 0x00ff);
    port_write_byte(SERIAL_BR_HIGH_REG(port), ((divisor & 0xff00) >> 8));

    // Set the correct mode.
    port_write_byte(SERIAL_LINE_CONTROL_REG(port), mode);

    // Clear and enable both FIFOs.
    port_write_byte(SERIAL_FIFO_CONTROL_REG(port), SERIAL_CLEAR_TX_FIFO | SERIAL_CLEAR_RX_FIFO | SERIAL_ENABLE_FIFOS);

    if (serial_test(port) == SERIAL_LOOPBACK_FAILED)
    {
        return SERIAL_LOOPBACK_FAILED;
    }

    return SERIAL_OK;
}

/*!
    @brief Checks if a serial port is working.

    Sets the serial port to loopback mode, sends a byte and checks if it receives the same byte.

    @param port Serial port to test.
    @returns SERIAL_OK if the sent byte is received, otherwise SERIAL_LOOPBACK_FAILED.
*/
serial_error_codes_t serial_test(uint16_t port)
{
    // Set serial port to loopback mode.
    port_write_byte(SERIAL_MODEM_CONTROL_REG(port), SERIAL_LOOPBACK_MODE);

    port_write_byte(port, SERIAL_TEST_BYTE);

    // Read data and check if it is what we send earlier.
    if (port_read_byte(port) == SERIAL_TEST_BYTE)
    {
        LOG_INFO("Loopback OK.");

        // Set serial port back to normal mode.
        port_write_byte(SERIAL_MODEM_CONTROL_REG(port), SERIAL_NORMAL_MODE);

        return SERIAL_OK;
    }
    else
    {
        LOG_ERROR("Loopback failed!");

        // Set serial port back to normal mode.
        port_write_byte(SERIAL_MODEM_CONTROL_REG(port), SERIAL_NORMAL_MODE);

        return SERIAL_LOOPBACK_FAILED;
    }
}

/*!
    @brief Logging interface implementation for logging via serial.

    @param c Character to log.
    @param context Pointer to a variable that specifies the serial port to use.
*/
void serial_log_write(uint8_t c, void *context)
{
    int port = *(int *)context;

    serial_send_byte(port, c);
}