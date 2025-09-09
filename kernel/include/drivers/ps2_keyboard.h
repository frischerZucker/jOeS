#ifndef PS2_KEYBOARD
#define PS2_KEYBOARD

#include "drivers/keyboard.h"

typedef enum ps2_kbd_error_codes
{
    PS2_KBD_OK = 0,
    PS2_KBD_ERROR_ALREADY_INITIALIZED,
    PS2_KBD_ERROR_SET_SCANCODE_SET_FAILED,
    PS2_KBD_ERROR_TOO_MANY_RESENDS
} ps2_kbd_error_codes_t;

ps2_kbd_error_codes_t ps2_kbd_init(uint8_t port);

void ps2_kbd_irq_callback(void);

#endif // PS2_KEYBOARD