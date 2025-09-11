/*!
    @file ps2_keyboard.h

    @brief PS/2 keyboard driver.
    
    Provides initialization and interrupt handling for PS/2 keyboards.
    The driver works according to the priciple of a finite state machine.
    It translates incoming scancodes into key events and manages driver state transitions.
    \image html ps2_keyboard_fsm.png
    
    @author frischerZucker
 */

#ifndef PS2_KEYBOARD
#define PS2_KEYBOARD

#include "drivers/keyboard.h"

/*!
    @brief Error codes used by the driver.
*/
typedef enum ps2_kbd_error_codes
{
    PS2_KBD_OK = 0,
    PS2_KBD_ERROR_ALREADY_INITIALIZED,
    PS2_KBD_ERROR_SET_SCANCODE_SET_FAILED,
    PS2_KBD_ERROR_TOO_MANY_RESENDS
} ps2_kbd_error_codes_t;

/*!
    @brief Initializes the keyboard.

    Selects scancode set 1 and enables scanning.

    @param port Specifies at which PS/2 port the device to identify is connected.
    @returns PS2_KBD_ERROR_ALREADY_INITIALIZED if the driver was initialized before,
             PS2_KBD_ERROR_SET_SCANCODE_SET_FAILED if selecting the scancode set failed,
             otherwise PS2_KBD_OK.
*/
ps2_kbd_error_codes_t ps2_kbd_init(uint8_t port);

/*!
    @brief Receives scancodes (scancode set 1) from the keyboard and translates them to key events.
    
    The incoming byte is read and depending on the current state actions are performed:
    - recognizing prefixes (0xe0, 0xe1, ...) and switching states accordingly
    - recognizing complete scancodes, translating them to key events, appending them to a ring buffer & switching back to "normal" state
    - switching to an "invalid" state whenever unexpected bytes are received
    - switching back from "invalid" to "normal" state, when the last expected byte of scancodes with more then 2 bytes is received

    It should be called from the interrupt handler's EXT_INT_1 case, as this interrupt is generated for every incoming byte.
*/
void ps2_kbd_irq_callback(void);

#endif // PS2_KEYBOARD