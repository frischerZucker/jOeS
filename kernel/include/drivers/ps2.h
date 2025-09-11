/*!
    @file ps2.h

    @brief Driver for the I8042 PS/2 Controller.
    
    Provides functions to initialize the I8042 PS/2 Controller and to communicate with connected PS/2 devices.
    This includes identifiying and resetting devices, as well as sending and receiving data.

    @author frischerZucker
 */

#ifndef PS2_H
#define PS2_H

#include <stdbool.h>
#include <stdint.h>

// Use this as port for ps2_send_byte().
#define PS2_PORT_1 1
#define PS2_PORT_2 2

/*!
    @brief Error codes used by the driver.
*/
typedef enum ps2_error_codes
{
    PS2_OK = 0,
    PS2_ERROR_CONTROLLER_TEST_FAILED,
    PS2_ERROR_PORT_TEST_FAILED,
    PS2_ERROR_NO_WORKING_PORTS,
    PS2_ERROR_DEVICE_RESET_FAILED,
    PS2_ERROR_TIMEOUT,
    PS2_ERROR_NO_ACK
} ps2_error_codes_t;

/*!
    @brief Contains information about what is connected to the PS/2 ports.
*/
struct ps2_device_status
{
    bool port_1_populated;
    int32_t device_type_port_1;
    bool port_2_populated;
    int32_t device_type_port_2;
};

extern struct ps2_device_status ps2_ports;

/**
    @brief Initializes the PS/2 controller.

    Initializes the PS/2 controller and checks if a second port exists.
    Performs a controller test as well as interface tests for existing ports.
    Enables working ports, resets and identifies connected devices.
    If a driver for identified devices exists, tries to initialize its driver.

    TODO:
    - I just assume that a PS/2 controller exists. Maybe I should add code to check if this is true.

    @returns PS2_OK if everything works, PS2_ERROR_NO_WORKING_PORTS if both ports fail the interface tests,
             PS2_ERROR_CONTROLLER_TEST_FAILED if the controller test failed or PS2_ERROR_TIMEOUT if there
             was a timeout.
*/
ps2_error_codes_t ps2_init_controller();

/*!
    @brief Resets a device connected to a PS/2 port.

    Sends a reset command (0xff) to a PS/2 device and validates its response.

    @param port Specifies at which PS/2 port the device to reset is connected.

    @returns PS2_OK if the devices response is ok, PS2_ERROR_DEVICE_RESET_FAILED if not
             or PS2_ERROR_TIMEOUT if there was a timeout.
*/
ps2_error_codes_t ps2_reset_device(uint8_t port);

/*!
    @brief Detects the type of a device connected to a PS/2 port.

    Detects the type of a device connected to a PS/2 port by sending an identify command.
    Side effect: Disables scanning for the device.

    @param port Specifies at which PS/2 port the device to identify is connected.
    @param device_type Pointer to a variable where the devices id is stored.
    @returns PS2_ERROR_NO_ACK or PS2_ERROR_TIMEOUT if something went wrong, otherwise PS2_OK.
*/
ps2_error_codes_t ps2_identify_device(uint8_t port, int32_t *device_type);

/*!
    @brief Sends a byte to a PS/2 device with timeout and response handling.

    Attempts to send data up to PS2_MAX_RESENDS times.
    Sends the data using ps2_raw_send_byte() and checks the devices response. 
    If it wasn't ACK (0xfa) (or 0xee in case of an echo command), starts a new 
    attempt to send it. If all attempts went wrong, an error is returned.

    @param port Specifies to which PS/2 port the data should be sent.
    @param data Data to send to the port.
    @returns PS2_OK if the byte was successfully sent, PS2_ERROR_NO_ACK if all attempts went wrong.
*/
ps2_error_codes_t ps2_send_byte(uint8_t port, uint8_t data);

/*!
    @brief Receives a byte from the PS/2 port with timeout handling.

    Waits until the controller's output buffer contains data to read.
    If the buffer stays empty for more the PS2_TIMEOUT iterations, assumes an error, writes 0 to *dest and abborts.
    If data is available before the timeout, reads one byte from the PS/2 port into *dest.

    @param dest Pointer to a variable where the received byte is stored.
    @returns PS2_OK if a byte was successfully read, PS2_ERROR_TIMEOUT if the operation timed out.
*/
ps2_error_codes_t ps2_receive_byte(uint8_t *dest);

#endif // PS2_H