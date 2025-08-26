#ifndef PS2_H
#define PS2_H

#include <stdint.h>

// Use this as port for ps2_send_byte().
#define PS2_PORT_1 1
#define PS2_PORT_2 2

enum ps2_error_codes
{
    PS2_OK = 0,
    PS2_ERROR_CONTROLLER_TEST_FAILED,
    PS2_ERROR_PORT_TEST_FAILED,
    PS2_ERROR_NO_WORKING_PORTS,
    PS2_ERROR_DEVICE_RESET_FAILED,
    PS2_ERROR_TIMEOUT
};

uint8_t ps2_init_controller();

uint8_t ps2_reset_device(uint8_t port);
void ps2_send_byte(uint8_t port, uint8_t data);
uint8_t ps2_receive_byte(void);

#endif