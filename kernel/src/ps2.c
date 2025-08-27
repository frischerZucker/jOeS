#include "ps2.h"

#include <stdbool.h>
#include "stdio.h"

#include "port_io.h"
#include "serial.h"

// Adresses of the IO ports used by the PS/2 controller.
#define PS2_DATA 0x60
#define PS2_COMMAND 0x64
#define PS2_STATUS 0x64

// Use this as port for sending command data via ps2_send_byte().
#define PS2_PORT_CMD 0

// Command to send to a device to reset it.
#define PS2_RESET_DEVICE 0xff

// For how many iterations sending and receiving data via the PS/2 port is tried until we give up.
#define PS2_TIMEOUT 100000

enum ps2_commands
{
    PS2_CMD_READ_BYTE_0 = 0x20,
    PS2_CMD_WRITE_BYTE_0 = 0x60,
    PS2_CMD_DISABLE_PORT_2 = 0xa7,
    PS2_CMD_ENABLE_PORT_2 = 0xa8,
    PS2_CMD_TEST_PORT_2 = 0xa9,
    PS2_CMD_TEST_CONTROLLER = 0xaa,
    PS2_CMD_TEST_PORT_1 = 0xab,
    PS2_CMD_DIAGNOSTIC_DUMP = 0xac,
    PS2_CMD_DISABLE_PORT_1 = 0xad,
    PS2_CMD_ENABLE_PORT_1 = 0xae,
    PS2_CMD_READ_CONTROLLER_INPUT_PORT = 0xc0,
    PS2_CMD_COPY_INPUT_PORT_BITS_0_TO_3_TO_STATUS_BITS_4_TO_7 = 0xc1, // I feel like I have to find a shorter name for this..
    PS2_CMD_COPY_INPUT_PORT_BITS_4_TO_7_TO_STATUS_BITS_4_TO_7 = 0xc2,
    PS2_CMD_READ_CONTROLLER_OUTPU_PORT = 0xd0,
    PS2_CMD_WRITE_NEXT_BYTE_TO_OUTPUT_BUFFER = 0xd1,
    PS2_CMD_WRITE_NEXT_BYTE_TO_PORT_1_OUTPUT_BUFFER = 0xd2,
    PS2_CMD_WRITE_NEXT_BYTE_TO_PORT_2_OUTPUT_BUFFER = 0xd3,
    PS2_CMD_SEND_NEXT_BYTE_TO_PORT_2 = 0xd4,
    PS2_CMD_PULSE_ALL_LINES = 0xf0 // Pulse all output lines low for 6 ms.
};
#define PS2_CMD_READ_BYTE_N(n) (PS2_CMD_READ_BYTE_0 + n)
#define PS2_CMD_READ_CONFIG_BYTE PS2_CMD_READ_BYTE_0
#define PS2_CMD_WRITE_BYTE_N(n) (PS2_CMD_WRITE_BYTE_0 + n)
#define PS2_CMD_WRITE_CONFIG_BYTE PS2_CMD_WRITE_BYTE_0
#define PS2_CMD_PULSE_LINE_N(n) (PS2_CMD_PULSE_ALL_LINES | (0x0f & ~(1 << n)))
#define PS2_CMD_PULSE_RESET PS2_PULSE_LINE_N(0) // Pulse reset line low for 6 ms.

enum ps2_device_response
{
    PS2_DEV_ACK = 0xfa,
    PS2_DEV_SELFTEST_PASSED = 0xaa,
    PS2_DEV_ECHO = 0xee,
    PS2_DEV_SELFTEST_FAILED_1 = 0xfc,
    PS2_DEV_SELFTEST_FAILED_2 = 0xfd,
};

// Possible response bytes after sending command PS2_TEST_PORT_*.
enum ps2_port_test_response
{
    PS2_PORT_TEST_PASSED = 0x00,
    PS2_CLK_STUCK_LOW = 0x01,
    PS2_CLK_STUCK_HIGH = 0x02,
    PS2_DATA_STUCK_LOW = 0x03,
    PS2_DATA_STUCK_HIGH = 0x04
};
// Possible response bytes after sending command PS2_TEST_CONTROLLER.
enum ps2_controller_test_response
{
    PS2_CONTROLLER_TEST_PASSED = 0x55,
    PS2_CONTROLLER_TEST_FAILED = 0xfc
};

struct ps2_config_byte_bitfield
{
    bool port_1_int_enabled : 1; // 1 = enabled, 0 = disabled
    bool port_2_int_enabled : 1; // 1 = enabled, 0 = disabled
    bool system_flag : 1;
    bool should_be_zero : 1;
    bool port_1_clk_enabled : 1;         // 0 = enabled, 1 = disabled
    bool port_2_clk_enabled : 1;         // 0 = enabled, 1 = disabled
    bool port_1_translation_enabled : 1; // 1 = enabled, 0 = disabled
    bool must_be_zero : 1;
};
// Combines config byte bitfield and uint8_t so that it can be used with port_read/write_byte().
union ps2_config_byte
{
    struct ps2_config_byte_bitfield bits;
    uint8_t byte;
};

static bool ps2_port_2_supported = false;
static bool ps2_port_1_works = false;
static bool ps2_port_2_works = false;

/*
    Checks if the controllers output buffer (incoming data) is full.

    @returns True if buffer is full, otherwise false.
*/
static inline bool output_buffer_full()
{
    return port_read_byte(PS2_STATUS) & 1;
}

/*
    Checks if the controllers input buffer (data to be sent) is full.

    @returns True if buffer is full, otherwise false.
*/
static inline bool input_buffer_full()
{
    return port_read_byte(PS2_STATUS) & (1 << 1);
}

/*
    Sends a command to the PS/2 controller.

    @param command Command to send to the controller.
*/
static void ps2_send_command(uint8_t command)
{
    while (input_buffer_full())
        ;

    port_write_byte(PS2_COMMAND, command);
}

/*
    Flushes the controllers output buffer and discards its content.
*/
static void ps2_flush_output_buffer(void)
{
    while (output_buffer_full())
    {
        port_read_byte(PS2_DATA); // Discard data that was read.
    }
}

/*
    Sends a byte to a PS/2 port with timeout handling.

    Waits until the controller's input buffer is empty, so that new data can be sent.
    If the buffer stays full for more the PS2_TIMEOUT iterations, assumes an error and abborts.

    @param port Specifies to which PS/2 port the data should be sent.
    @param data Data to send to the port.
    @returns PS2_OK if the byte was successfully sent, PS2_ERROR_TIMEOUT if the operation timed out.
*/
uint8_t ps2_send_byte(uint8_t port, uint8_t data)
{
    unsigned int timeout = PS2_TIMEOUT;

    // Make the controller send the data to the second PS/2 port, if needed.
    if (port == PS2_PORT_2)
    {
        ps2_send_command(PS2_CMD_SEND_NEXT_BYTE_TO_PORT_2);
    }

    // Wait until data can be sent.
    while (input_buffer_full())
    {
        timeout = timeout - 1;
        if (timeout == 0) // Waiting took too long, so I assume something went wrong and we can give up.
        {
            return PS2_ERROR_TIMEOUT;
        }
    }

    port_write_byte(PS2_DATA, data);
    
    return PS2_OK;
}

/*
    Receives a byte from the PS/2 port with timeout handling.

    Waits until the controller's output buffer contains data to read.
    If the buffer stays empty for more the PS2_TIMEOUT iterations, assumes an error, writes 0 to *dest and abborts.
    If data is available before the timeout, reads one byte from the PS/2 port into *dest.

    @param dest Pointer to a variable where the received byte is stored.
    @returns PS2_OK if a byte was successfully read, PS2_ERROR_TIMEOUT if the operation timed out.
*/
uint8_t ps2_receive_byte(uint8_t *dest)
{
    unsigned int timeout = PS2_TIMEOUT;

    // Wait until there is data to read. 
    while (output_buffer_full() == 0)
    {
        timeout = timeout - 1;
        if (timeout == 0) // Waiting for data took too long, so I assume something went wrong and we can give up.
        {
            *dest = 0; // Returns a defined value after a timeout.
            return PS2_ERROR_TIMEOUT;
        }
    }

    *dest = port_read_byte(PS2_DATA);

    return PS2_OK;
}

/*
    Resets a device connected to a PS/2 port.

    Sends a reset command (0xff) to a PS/2 device and validates its response.

    @param port Specifies at which PS/2 port the device to reset is connected.

    @returns PS2_OK if the devices response is ok, PS2_ERROR_DEVICE_RESET_FAILED if not
             or PS2_ERROR_TIMEOUT if there was a timeout.
*/
uint8_t ps2_reset_device(uint8_t port)
{   
    ps2_send_byte(port, PS2_RESET_DEVICE);

    uint8_t response_1 = 0, response_2 = 0;
    
    if (ps2_receive_byte(&response_1) == PS2_ERROR_TIMEOUT)
    {
        printf("PS/2: Timeout during device reset!");
        return PS2_ERROR_TIMEOUT;
    }
    if (ps2_receive_byte(&response_2) == PS2_ERROR_TIMEOUT)
    {
        printf("PS/2: Timeout during device reset!");
        return PS2_ERROR_TIMEOUT;
    }

    // The response to a reset should be a "Command Acknowledged" and a "Selftest Passed", otherwise something went wrong.
    if ((response_1 == PS2_DEV_ACK && response_2 == PS2_DEV_SELFTEST_PASSED) || (response_1 == PS2_DEV_SELFTEST_PASSED && response_2 == PS2_DEV_ACK))
    {
        return PS2_OK;
    }
    else
    {
        return PS2_ERROR_DEVICE_RESET_FAILED;
    }
}

/*
    Initializes the PS/2 controller.

    Initializes the PS/2 controller and checks if a second port exists.
    Performs a controller test as well as interface tests for existing ports.
    Enables working ports and resets connected devices.

    TODO:
    - I just assume that a PS/2 controller exists. Maybe I should add code to check if this is true.
    - Identify connected devices and save this information somewhere.

    @returns PS2_OK if everything works, PS2_ERROR_NO_WORKING_PORTS if both ports fail the interface tests,
             PS2_ERROR_CONTROLLER_TEST_FAILED if the controller test failed or PS2_ERROR_TIMEOUT if there
             was a timeout.
*/
uint8_t ps2_init_controller(void)
{
    uint8_t response = 0; // Used for receiving data via PS/2 when I don't really know where to put it.

    // Read data from the output buffer so that nothing unexpected (e.g. some key presses) are stuck in there.
    ps2_flush_output_buffer();
    
    // Disable PS/2 ports, so that connected devices cannot mess up the initialization by sending data in the wrong moment.
    ps2_send_command(PS2_CMD_DISABLE_PORT_1);
    ps2_send_command(PS2_CMD_DISABLE_PORT_2);

    // Do it again. Just to be sure.
    ps2_flush_output_buffer();
    
    union ps2_config_byte config_byte;
    // Read the config byte.
    ps2_send_command(PS2_CMD_READ_CONFIG_BYTE);
    // config_byte.byte = ps2_receive_byte();
    if (ps2_receive_byte(&config_byte.byte) == PS2_ERROR_TIMEOUT)
    {
        printf("PS/2: Timeout while waiting for config byte!\n");
        return PS2_ERROR_TIMEOUT;
    }
    // Disable IRQs clocks of both PS/2 ports, disable translation for port 1.
    config_byte.bits.port_1_int_enabled = 0;
    config_byte.bits.port_2_int_enabled = 0;
    config_byte.bits.port_1_clk_enabled = 0;
    config_byte.bits.port_2_clk_enabled = 1;
    config_byte.bits.port_1_translation_enabled = 0;
    // Send back the modified config byte.
    ps2_send_command(PS2_CMD_WRITE_CONFIG_BYTE);
    ps2_send_byte(PS2_PORT_CMD, config_byte.byte);

    // Perform controller self test.
    ps2_send_command(PS2_CMD_TEST_CONTROLLER);
    if (ps2_receive_byte(&response) == PS2_ERROR_TIMEOUT)
    {
        printf("PS/2: Timeout during controller self test!\n");
        return PS2_ERROR_TIMEOUT;
    }
    if (response != PS2_CONTROLLER_TEST_PASSED)
    {
        printf("PS/2: Controller self test failed! %d\n", response);
        return PS2_ERROR_CONTROLLER_TEST_FAILED;
    }

    // Send back the modified config byte again, in case the self test reset the controller.
    ps2_send_command(PS2_CMD_WRITE_CONFIG_BYTE);
    ps2_send_byte(PS2_PORT_CMD, config_byte.byte);

    // Check if the second PS/2 port exists.
    ps2_send_command(PS2_CMD_ENABLE_PORT_2);
    ps2_send_command(PS2_CMD_READ_CONFIG_BYTE);
    if (ps2_receive_byte(&response) == PS2_ERROR_TIMEOUT)
    {
        printf("PS/2: Timeout while checking if port 2 exists!\n");
        return PS2_ERROR_TIMEOUT;
    }
    if ((response & (1 << 1)) == 0)
    {
        // Make sure port 2 is disabled again.
        ps2_send_command(PS2_CMD_DISABLE_PORT_2);
        ps2_send_command(PS2_CMD_WRITE_CONFIG_BYTE);
        ps2_send_byte(PS2_PORT_CMD, config_byte.byte);

        ps2_port_2_supported = true;
        printf("PS/2: Second port is supported.\n");
    }
    else
    {
        ps2_port_2_supported = false;
        printf("PS/2: Second port is not supported.\n");
    }

    // Perform interface tests for port 1.
    ps2_send_command(PS2_CMD_TEST_PORT_1);
    if (ps2_receive_byte(&response) == PS2_ERROR_TIMEOUT)
    {
        printf("PS/2: Timeout during port 1 interface test!\n");
        return PS2_ERROR_TIMEOUT;
    }
    if (response == PS2_PORT_TEST_PASSED)
    {
        ps2_port_1_works = true;
        printf("PS/2: Port 1 passed the interface tests.\n");
    }
    else
    {
        printf("PS/2: Port 1 didn't pass the interface tests. :(\n");
    }

    // Perform interface tests for port 2 if it is supported.
    if (ps2_port_2_supported)
    {
        ps2_send_command(PS2_CMD_TEST_PORT_1);
        if (ps2_receive_byte(&response) == PS2_ERROR_TIMEOUT)
        {
            printf("PS/2: Timeout during port 2 interface test!\n");
            return PS2_ERROR_TIMEOUT;
        }
        if (response == PS2_PORT_TEST_PASSED)
        {
            ps2_port_2_works = true;
            printf("PS/2: Port 2 passed the interface tests.\n");
        }
        else
        {
            printf("PS/2: Port 2 didn't pass the interface tests. :(\n");
        }
    }

    // Check if there are working PS/2 ports. If there are none an error code is returned.
    if (ps2_port_1_works == false && ps2_port_2_works == false)
    {
        printf("PS/2: There are no working PS/2 ports!\n");
        return PS2_ERROR_NO_WORKING_PORTS;
    }

    // Enable working PS/2 ports.
    if (ps2_port_1_works)
    {
        ps2_send_command(PS2_CMD_ENABLE_PORT_1);
        config_byte.bits.port_1_int_enabled = 1;
        config_byte.bits.port_1_clk_enabled = 0;

        printf("PS/2: Enabled port 1.\n");
    }
    if (ps2_port_2_works)
    {
        ps2_send_command(PS2_CMD_ENABLE_PORT_2);
        config_byte.bits.port_2_int_enabled = 1;
        config_byte.bits.port_2_clk_enabled = 0;
        printf("PS/2: Enabled port 2.\n");
    }
    ps2_send_command(PS2_CMD_WRITE_CONFIG_BYTE);
    ps2_send_byte(PS2_PORT_CMD, config_byte.byte);

    // Reset connected devices.
    if (ps2_port_1_works)
    {
        if (ps2_reset_device(PS2_PORT_1) != PS2_OK)
        {
            printf("PS/2: Reset of device at port 1 failed!\n");
        }
    }
    if (ps2_port_2_works)
    {
        if (ps2_reset_device(PS2_PORT_2) != PS2_OK)
        {
            printf("PS/2: Reset of device at port 2 failed!\n");
        }
    }

    printf("PS/2: Controller initialized.\n");
    return 0;
}