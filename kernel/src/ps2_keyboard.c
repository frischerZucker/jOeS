#include "ps2_keyboard.h"

#include "stdio.h"

#include "pic.h"
#include "ps2.h"

#define PS2_KBD_ENABLE_SCANNING 0xf4
#define PS2_KBD_DISABLE_SCANNING 0xf5
#define PS2_KBD_GET_SET_SCANCODE_SET 0xf0

typedef enum ps2_kbd_scancode_set
{
    PS2_KBD_GET_CURRENT_SCANCODE_SET = 0,
    PS2_KBD_SCANCODE_SET_1 = 1,
    PS2_KBD_SCANCODE_SET_2 = 2,
    PS2_KBD_SCANCODE_SET_3 = 3
} ps2_kbd_scancode_set_t;

static bool ps2_kbd_driver_initialized = false;

/*
    Selects a scancode set.

    Selects what scancode set the keyboard should use.
    After three failed attempts an error is returned.

    @param port Specifies at which PS/2 port the device to identify is connected.
    @param scancode_set Scancode set that should be selected.
    @returns PS2_KBD_ERROR_TOO_MANY_RESENDS after three failed attempts, otherwise PS2_KBD_OK.
*/
static ps2_kbd_error_codes_t ps2_kbd_set_scancode_set(uint8_t port, ps2_kbd_scancode_set_t scancode_set)
{
    uint8_t response = 0;
    uint8_t tries = 0;

    // Tries to set the scancode set up to three times.
    do
    {
        ps2_send_byte(port, PS2_KBD_GET_SET_SCANCODE_SET);
        ps2_send_byte(port, scancode_set);

        ps2_receive_byte(&response);

        // Break from the resend loop if the command was acknowledged.
        if (response == 0xfa)
        {
            break;
        }

        tries = tries + 1;
    } while (response == 0xfe && tries < 3);
    
    if (tries >= 3)
    {
        return PS2_KBD_ERROR_TOO_MANY_RESENDS;
    }
    
    printf("PS/2 KBD: Enabled scancode set %d.", scancode_set);

    return PS2_KBD_OK;
}

/*
    Initializes the keyboard.

    Selects scancode set 1 and enables the IRQ that is used for keyboard interrupts.

    @param port Specifies at which PS/2 port the device to identify is connected.
    @returns PS2_KBD_ERROR_ALREADY_INITIALIZED if the driver was initialized before,
             PS2_KBD_ERROR_SET_SCANCODE_SET_FAILED if selecting the scancode set failed,
             otherwise PS2_KBD_OK.
*/
ps2_kbd_error_codes_t ps2_kbd_init(uint8_t port)
{
    if (ps2_kbd_driver_initialized)
    {
        printf("PS/2 KBD: Driver is already initialized at port %d!\n", port);
        return PS2_KBD_ERROR_ALREADY_INITIALIZED;
    }

    printf("PS/2 KBD: Initializing keyboard driver for port %d.\n", port);

    // Disable scanning so that the keyboard cannot disturb the initialization.
    ps2_send_byte(port, PS2_KBD_DISABLE_SCANNING);

    // Enable scancode set 1.
    if (ps2_kbd_set_scancode_set(port, PS2_KBD_SCANCODE_SET_1) != PS2_KBD_OK)
    {
        printf("PS/2 KBD: Couldn't set the scancode set!");
        return PS2_KBD_ERROR_SET_SCANCODE_SET_FAILED;
    }
    
    // Enable the interrupt used by the keyboard.
    pic_enable_irq(1);

    // Enable scanning to be able to get keyboard input.
    ps2_send_byte(port, PS2_KBD_ENABLE_SCANNING);

    ps2_kbd_driver_initialized = true;

    return PS2_KBD_OK;
}