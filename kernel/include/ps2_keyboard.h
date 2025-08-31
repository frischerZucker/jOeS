#ifndef PS2_KEYBOARD
#define PS2_KEYBOARD

#include <stdbool.h>
#include <stdint.h>

typedef enum ps2_kbd_error_codes
{
    PS2_KBD_OK = 0,
    PS2_KBD_ERROR_ALREADY_INITIALIZED,
    PS2_KBD_ERROR_SET_SCANCODE_SET_FAILED,
    PS2_KBD_ERROR_TOO_MANY_RESENDS
} ps2_kbd_error_codes_t;

struct key_event
{
    uint8_t scancode;
    bool shift;
    bool caps_lock;
    bool alt;
    bool alt_gr;
};

ps2_kbd_error_codes_t ps2_kbd_init(uint8_t port);

void ps2_kbd_irq_callback();

#endif // PS2_KEYBOARD