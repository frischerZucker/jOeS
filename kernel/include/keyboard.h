#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

// Keycodes. I use a german keyboard layout (QWERTZ), so the keycodes are the main symbols on a german keyboard.
typedef enum key_codes
{
    // Default for unknown keys.
    KEY_UNKNOWN,
    // First row.
    KEY_ESC,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    // Second row.
    KEY_CIRCUMFLEX, // The roof (^).
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_ß,
    KEY_BACK_TICK,
    KEY_BACKSPACE,
    // Third row.
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Z,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_Ü,
    KEY_PLUS,
    KEY_ENTER,
    // Fourth row.
    KEY_CAPS_LOCK,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_Ö,
    KEY_Ä,
    KEY_HASHTAG,
    // Fifth row.
    KEY_LSHIFT,
    KEY_LESS_THEN,
    KEY_Y,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_MINUS,
    KEY_RSHIFT,
    KEY_LSTRG,
    KEY_LSUPER,
    KEY_ALT,
    KEY_SPACE,
    KEY_ALTGR,
    KEY_RSUPER,
    KEY_MENU,
    KEY_RSTRG,
    // Arrow keys.
    KEY_UP,
    KEY_LEFT,
    KEY_DOWN,
    KEY_RIGHT,
    // Random block above the arrow keys.
    KEY_PRINT_SCREEN,
    KEY_SCROLL_LOCK,
    KEY_PAUSE,
    KEY_INSERT,
    KEY_POS1,
    KEY_PAGE_UP,
    KEY_DEL,
    KEY_END,
    KEY_PAGE_DOWN,
    // Keypad.
    KEY_NUM_LOCK,
    KEY_KEYPAD_SLASH,
    KEY_KEYPAD_ASTERISK,
    KEY_KEYPAD_MINUS,
    KEY_KEYPAD_7,
    KEY_KEYPAD_8,
    KEY_KEYPAD_9,
    KEY_KEYPAD_PLUS,
    KEY_KEYPAD_4,
    KEY_KEYPAD_5,
    KEY_KEYPAD_6,
    KEY_KEYPAD_1,
    KEY_KEYPAD_2,
    KEY_KEYPAD_3,
    KEY_KEYPAD_ENTER,
    KEY_KEYPAD_0,
    KEY_KEYPAD_DELETE,
    // Multimedia keys.
    KEY_MULTIMEDIA_PREVIOUS_TRACK,
    KEY_MULTIMEDIA_NEXT_TRACK,
    KEY_MULTIMEDIA_MUTE,
    KEY_MULTIMEDIA_CALCULATOR,
    KEY_MULTIMEDIA_PLAY,
    KEY_MULTIMEDIA_STOP,
    KEY_MULTIMEDIA_VOLUME_DOWN,
    KEY_MULTIMEDIA_VOLUME_UP,
    KEY_MULTIMEDIA_WWW_HOME,
    KEY_MULTIMEDIA_WWW_SEARCH,
    KEY_MULTIMEDIA_FAVORITES,
    KEY_MULTIMEDIA_WWW_REFRESH,
    KEY_MULTIMEDIA_WWW_STOP,
    KEY_MULTIMEDIA_WWW_FORWARD,
    KEY_MULTIMEDIA_WWW_BACK,
    KEY_MULTIMEDIA_MY_COMPUTER,
    KEY_MULTIMEDIA_EMAIL,
    KEY_MULTIMEDIA_MEDIA_SELECT
} key_code_t;

typedef enum key_event_types
{
    KEY_EVENT_TYPE_UNDEFINED,
    KEY_EVENT_TYPE_PRESSED,
    KEY_EVENT_TYPE_RELEASED
} key_event_type_t;

struct key_event
{
    uint8_t scancode;
    bool pressed;
    bool shift;
    bool caps_lock;
    bool alt;
    bool alt_gr;
};

#endif // KEYBOARD_H