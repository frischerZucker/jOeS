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

typedef enum ps2_kbd_states
{
    PS2_KBD_STATE_UNINITIALIZED,
    PS2_KBD_STATE_NORMAL,
    PS2_KBD_STATE_PREFIX_E0,
    PS2_KBD_STATE_PREFIX_E02A,
    PS2_KBD_STATE_PREFIX_E02AE0,
    PS2_KBD_STATE_PREFIX_E0B7,
    PS2_KBD_STATE_PREFIX_E0B7E0,
    PS2_KBD_STATE_PREFIX_E1,
    PS2_KBD_STATE_PREFIX_E11D,
    PS2_KBD_STATE_PREFIX_E11D45,
    PS2_KBD_STATE_PREFIX_E11D45E1,
    PS2_KBD_STATE_PREFIX_E11D45E19D,
    PS2_KBD_STATE_INVALID
} ps2_kbd_state_t;

static ps2_kbd_state_t ps2_kbd_state = PS2_KBD_STATE_UNINITIALIZED;

static uint8_t ps2_kbd_port = 0;

/*
    Disable warnings for overwriting already initialized values during array initialization.
    First, I initialize the entire array with KEY_UNKNOWN and overwrite certain values with real key codes.
    This overwriting would generate many unnecessary warnings, so I disable them for this initialization.
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
// Look up table to convert scan codes (scan code set 1) to key codes.
static key_code_t mapping_scancode_set_1_to_keycode[256] =
{
    [0x00 ... 0xff] = KEY_UNKNOWN,

    [0x01] = KEY_ESC,
    [0x02] = KEY_1,
    [0x03] = KEY_2,
    [0x04] = KEY_3,
    [0x05] = KEY_4,
    [0x06] = KEY_5,
    [0x07] = KEY_6,
    [0x08] = KEY_7,
    [0x09] = KEY_8,
    [0x0a] = KEY_9,
    [0x0b] = KEY_0,
    [0x0c] = KEY_ß,
    [0x0d] = KEY_BACK_TICK,
    [0x0e] = KEY_BACKSPACE,
    [0x0f] = KEY_TAB,
    [0x10] = KEY_Q,
    [0x11] = KEY_W,
    [0x12] = KEY_E,
    [0x13] = KEY_R,
    [0x14] = KEY_T,
    [0x15] = KEY_Z,
    [0x16] = KEY_U,
    [0x17] = KEY_I,
    [0x18] = KEY_O,
    [0x19] = KEY_P,
    [0x1a] = KEY_Ü,
    [0x1b] = KEY_PLUS,
    [0x1c] = KEY_ENTER,
    [0x1d] = KEY_LSTRG,
    [0x1e] = KEY_A,
    [0x1f] = KEY_S,
    [0x20] = KEY_D,
    [0x21] = KEY_F,
    [0x22] = KEY_G,
    [0x23] = KEY_H,
    [0x24] = KEY_J,
    [0x25] = KEY_K,
    [0x26] = KEY_L,
    [0x27] = KEY_Ö,
    [0x28] = KEY_Ä,
    [0x29] = KEY_HASHTAG,
    [0x2a] = KEY_LSHIFT,
    [0x2b] = KEY_LESS_THEN,
    [0x2c] = KEY_Y,
    [0x2d] = KEY_X,
    [0x2e] = KEY_C,
    [0x2f] = KEY_V,
    [0x30] = KEY_B,
    [0x31] = KEY_N,
    [0x32] = KEY_M,
    [0x33] = KEY_COMMA,
    [0x34] = KEY_PERIOD,
    [0x35] = KEY_MINUS,
    [0x36] = KEY_RSHIFT,
    [0x37] = KEY_KEYPAD_ASTERISK,
    [0x38] = KEY_ALT,
    [0x39] = KEY_SPACE,
    [0x3a] = KEY_CAPS_LOCK,
    [0x3b] = KEY_F1,
    [0x3c] = KEY_F2,
    [0x3d] = KEY_F3,
    [0x3e] = KEY_F4,
    [0x3f] = KEY_F5,
    [0x40] = KEY_F6,
    [0x41] = KEY_F7,
    [0x42] = KEY_F8,
    [0x43] = KEY_F9,
    [0x44] = KEY_F10,
    [0x45] = KEY_NUM_LOCK,
    [0x46] = KEY_SCROLL_LOCK,
    [0x47] = KEY_KEYPAD_7,
    [0x48] = KEY_KEYPAD_8,
    [0x49] = KEY_KEYPAD_9,
    [0x4a] = KEY_KEYPAD_MINUS,
    [0x4b] = KEY_KEYPAD_4,
    [0x4c] = KEY_KEYPAD_5,
    [0x4d] = KEY_KEYPAD_6,
    [0x4e] = KEY_KEYPAD_PLUS,
    [0x4f] = KEY_KEYPAD_1,
    [0x50] = KEY_KEYPAD_2,
    [0x51] = KEY_KEYPAD_3,
    [0x52] = KEY_KEYPAD_0,
    [0x53] = KEY_KEYPAD_DELETE,
    [0x57] = KEY_F11,
    [0x58] = KEY_F12,
    [0x90] = KEY_MULTIMEDIA_PREVIOUS_TRACK,
    [0x99] = KEY_MULTIMEDIA_NEXT_TRACK,
    [0x9c] = KEY_KEYPAD_ENTER,
    [0x9d] = KEY_RSTRG,
    [0xa0] = KEY_MULTIMEDIA_MUTE,
    [0xa1] = KEY_MULTIMEDIA_CALCULATOR,
    [0xa2] = KEY_MULTIMEDIA_PLAY,
    [0xa4] = KEY_MULTIMEDIA_STOP,
    [0xae] = KEY_MULTIMEDIA_VOLUME_DOWN,
    [0xb0] = KEY_MULTIMEDIA_VOLUME_UP,
    [0xb2] = KEY_MULTIMEDIA_WWW_HOME,
    [0xb5] = KEY_KEYPAD_SLASH,
    [0xb8] = KEY_ALTGR,
    [0xc7] = KEY_POS1,
    [0xc8] = KEY_UP,
    [0xc9] = KEY_PAGE_UP,
    [0xcb] = KEY_LEFT,
    [0xcd] = KEY_RIGHT,
    [0xcf] = KEY_END,
    [0xd0] = KEY_DOWN,
    [0xd1] = KEY_PAGE_DOWN,
    [0xd2] = KEY_INSERT,
    [0xd3] = KEY_DEL,
    [0xdb] = KEY_LSUPER,
    [0xdc] = KEY_RSUPER,
    [0xdd] = KEY_MENU,
    [0xde] = KEY_POWER,
    [0xdf] = KEY_SLEEP,
    [0xe3] = KEY_WAKE,
    [0xe5] = KEY_MULTIMEDIA_WWW_SEARCH,
    [0xe6] = KEY_MULTIMEDIA_FAVORITES,
    [0xe7] = KEY_MULTIMEDIA_WWW_REFRESH,
    [0xe8] = KEY_MULTIMEDIA_WWW_STOP,
    [0xe9] = KEY_MULTIMEDIA_WWW_FORWARD,
    [0xea] = KEY_MULTIMEDIA_WWW_BACK,
    [0xeb] = KEY_MULTIMEDIA_MY_COMPUTER,
    [0xec] = KEY_MULTIMEDIA_EMAIL,
    [0xed] = KEY_MULTIMEDIA_MEDIA_SELECT,
    [0xb7] = KEY_PRINT_SCREEN,
    [0xaa] = KEY_PRINT_SCREEN,
    [0xc5] = KEY_PAUSE
};
// Enable "-Woverride-init" again.
#pragma GCC diagnostic pop 

/*
    Translates scan codes to key codes.

    Handles translation from raw scan codes to key codes.
    For now only scan code set 1 is implemented.

    @param scancode Scan code to translate.
    @param state State the driver was in when the scan code was received.
    @param scancode_set Scan code set the scan code is from.
    @returns A key code, KEY_UNKNOWN if an unknown scan code set was requested.
*/
static inline key_code_t ps2_kbd_scancode_to_keycode(uint8_t scancode, ps2_kbd_state_t state, ps2_kbd_scancode_set_t scancode_set)
{
    switch (scancode_set)
    {
    case PS2_KBD_SCANCODE_SET_1:
        // Remove pressed / released bit, so it can be used to distinguish keys with prefixed scan codes.
        scancode = scancode & ~(1 << 7);
        // Set the 7th bit if the scan code had a prefix. This ensures there are no collisions between keys.
        if (state != PS2_KBD_STATE_NORMAL) 
        {
            scancode = scancode | (1 << 7);
        }
        // Get the correct key code for the scan code.
        return mapping_scancode_set_1_to_keycode[scancode];
        break;
    
    default:
        printf("PS/2 KBD: Can't convert scancode. Scancode set %d is not implemented.\n", scancode_set);
        return KEY_UNKNOWN;
        break;
    }
}

/*
    Sets the modifier flags for a key event.

    Sets and clears modifier flags (shift, caps lock, alt, altgr, strg, left super and right super) for a key event.

    TODO: 
    If any of both strg or shift keys is released, it's modifier flag is cleared.
    In case both keys are pressed simoultanously, this shouldn't happen until the last one is released.

    @param key_event Key event for which the modifiers should be set.
*/
static inline void ps2_kbd_set_modifiers(struct key_event_t *key_event)
{
    // Stores the modifiers of the last run. This is needed to toggle caps lock.
    static uint16_t modifiers_old = 0;
    static uint8_t num_shift_keys_pressed = 0;
    static uint8_t num_strg_keys_pressed = 0;

    uint16_t mask = 0;

    // Select the correct mask and go to the right code section to apply the mask.
    switch (key_event->keycode)
    {
    case KEY_CAPS_LOCK:
        mask = KBD_MODIFIER_MASK_CAPS_LOCK;
        goto APPLY_TOGGLED_MODIFIERS_MASK;
        break;

    case KEY_NUM_LOCK:
        mask = KBD_MODIFIER_MASK_NUM_LOCK;
        goto APPLY_TOGGLED_MODIFIERS_MASK;
        break;
    
    case KEY_SCROLL_LOCK:
        mask = KBD_MODIFIER_MASK_SCROLL_LOCK;
        goto APPLY_TOGGLED_MODIFIERS_MASK;
        break;

    case KEY_LSHIFT:
    case KEY_RSHIFT:
        /*
            Count how many Shift keys are pressed. 
            Set the flag everytime a Shift key is pressed. 
            Only clear the flag when no Shift key is pressed.
            This is needed beacause there are two Shift keys and the flag should be set as long as any of them is pressed.
        */
        if (key_event->pressed == KEY_EVENT_TYPE_PRESSED)
        {
            num_shift_keys_pressed = num_shift_keys_pressed + 1;
            mask = KBD_MODIFIER_MASK_SHIFT;
        }
        else
        {
            num_shift_keys_pressed = num_shift_keys_pressed - 1;
            if (num_shift_keys_pressed == 0)
            {
                mask = KBD_MODIFIER_MASK_SHIFT;
            } 
        }     
        goto APPLY_PRESSED_RELEASED_MODIFIERS_MASK;
        break;
    
    case KEY_LSTRG:
    case KEY_RSTRG:
        // Do the same thing as with Shift, as there are also two Strg keys.
        if (key_event->pressed == KEY_EVENT_TYPE_PRESSED)
        {
            num_strg_keys_pressed = num_strg_keys_pressed + 1;
            mask = KBD_MODIFIER_MASK_STRG;
        }
        else
        {
            num_strg_keys_pressed = num_strg_keys_pressed - 1;
            if (num_strg_keys_pressed == 0)
            {
                mask = KBD_MODIFIER_MASK_STRG;
            } 
        }     
        goto APPLY_PRESSED_RELEASED_MODIFIERS_MASK;
        break;
    
    case KEY_ALT:
        mask = KBD_MODIFIER_MASK_ALT;
        goto APPLY_PRESSED_RELEASED_MODIFIERS_MASK;
        break;
    
    case KEY_ALTGR:
        mask = KBD_MODIFIER_MASK_ALTGR;
        goto APPLY_PRESSED_RELEASED_MODIFIERS_MASK;
        break;

    case KEY_LSUPER:
        mask = KBD_MODIFIER_MASK_LSUPER;
        goto APPLY_PRESSED_RELEASED_MODIFIERS_MASK;
        break;
    
    case KEY_RSUPER:
        mask = KBD_MODIFIER_MASK_RSUPER;
        goto APPLY_PRESSED_RELEASED_MODIFIERS_MASK;
        break;

    default:
        // Key event does not correspond to a modifier key, so no flags need to be set / cleared.
        key_event->modifiers = modifiers_old;
        return;
        break;
    }

    // Toggle the masked modifier flag everytime the key is pressed.
    APPLY_TOGGLED_MODIFIERS_MASK:
    if (key_event->pressed == KEY_EVENT_TYPE_PRESSED)
    {
        modifiers_old = modifiers_old ^ mask;
    }

    key_event->modifiers = modifiers_old;

    return;

    // Sets the masked modifier flag when the key is pressed, clears it when the key is released.
    APPLY_PRESSED_RELEASED_MODIFIERS_MASK:    
    if (key_event->pressed == KEY_EVENT_TYPE_PRESSED)
    {
        modifiers_old = modifiers_old | mask;
    }
    else
    {
        modifiers_old = modifiers_old & ~mask;
    }

    key_event->modifiers = modifiers_old;

    return;
}

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
    Receives scancodes (scancode set 1) from the keyboard and translates them to key events.

    This function acts as a finite state machine.
    The incoming byte is read and depending on the current state actions are performed.

    These include:
    - recognizing prefixes (0xe0, 0xe1, ...) and switching states accordingly
    - recognizing complete scancodes, translating them to key events, appending them to a ring buffer & switching back to "normal" state
    - switching to an "invalid" state whenever unexpected bytes are received
    - switching back from "invalid" to "normal" state, when the last expected byte of scancodes with more then 2 bytes is received

    It should be called from the interrupt handler's EXT_INT_1 case, as this interrupt is generated for every incoming byte.

    TODO:
    - Append keyevents to some sort of buffer
*/
void ps2_kbd_irq_callback(void)
{
    uint8_t scancode_raw = 0;
    ps2_receive_byte(&scancode_raw);

    // Logic for switching states.
    switch (ps2_kbd_state)
    {
    case PS2_KBD_STATE_NORMAL:
        if (scancode_raw == 0xe0)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E0;
            return;
        }
        else if (scancode_raw == 0xe1)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E1;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E0:
        if (scancode_raw == 0x2a)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E02A;
            return;
        }
        else if (scancode_raw == 0xb7)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E0B7;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E02A:
        if (scancode_raw == 0xe0)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E02AE0;
            return;
        }
        else
        {
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E02AE0:
        if (scancode_raw != 0x37)
        {
            ps2_kbd_state = PS2_KBD_STATE_NORMAL;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E0B7:
        if (scancode_raw == 0xe0)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E02AE0;
            return;
        }
        else
        {
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E0B7E0:
        if (scancode_raw != 0xaa)
        {
            ps2_kbd_state = PS2_KBD_STATE_NORMAL;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E1:
        if (scancode_raw == 0x1d)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E11D;
            return;
        }
        else
        {
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E11D:
        if (scancode_raw == 0x45)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E11D45;
            return;
        }
        else
        {
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E11D45:
        if (scancode_raw == 0xe1)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E11D45E1;
            return;
        }
        else
        {
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E11D45E1:
        if (scancode_raw == 0x9d)
        {
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E11D45E19D;
            return;
        }
        else
        {
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E11D45E19D:
        if (scancode_raw != 0xc5)
        {
            ps2_kbd_state = PS2_KBD_STATE_NORMAL;
            return;
        }

        break;

    case PS2_KBD_STATE_INVALID:
        if (scancode_raw == 0x37 || scancode_raw == 0xaa || scancode_raw == 0xc5)
        {
            ps2_kbd_state = PS2_KBD_STATE_NORMAL;
        }
        return;

        break;

    default:
        printf("PS/2 KBD: This state should not be reached.\n");
        break;
    }

    /*
        This part of the code is only reached, when a full scancode was received.
        It translates the raw scancode to a key event.
    */

    struct key_event_t key_event = {0};
    // If the scancodes 7th bit is set the key was released, otherwise it was pressed.
    if ((scancode_raw & (1 << 7)) == 0)
    {
        key_event.pressed = KEY_EVENT_TYPE_PRESSED;
    }
    else
    {
        key_event.pressed = KEY_EVENT_TYPE_RELEASED;
    }

    key_event.keycode = ps2_kbd_scancode_to_keycode(scancode_raw, ps2_kbd_state, PS2_KBD_SCANCODE_SET_1);

    // Set or clear modifier flags.
    ps2_kbd_set_modifiers(&key_event);

    printf("PS/2 KBD: Key: %d, Key event type: %d, Modifiers: %d.\n", key_event.keycode, key_event.pressed, key_event.modifiers);
    // A full scan code was received, so we return back to the "normal" state.
    ps2_kbd_state = PS2_KBD_STATE_NORMAL;
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
    ps2_kbd_port = port;

    // Abort if the driver is already initialized.
    if (ps2_kbd_state != PS2_KBD_STATE_UNINITIALIZED)
    {
        printf("PS/2 KBD: Driver is already initialized at port %d!\n", port);
        return PS2_KBD_ERROR_ALREADY_INITIALIZED;
    }

    printf("PS/2 KBD: Initializing keyboard driver for port %d.\n", port);

    // Disable scanning so that the keyboard cannot disturb the initialization.
    ps2_send_byte(ps2_kbd_port, PS2_KBD_DISABLE_SCANNING);

    // Enable scancode set 1.
    if (ps2_kbd_set_scancode_set(ps2_kbd_port, PS2_KBD_SCANCODE_SET_1) != PS2_KBD_OK)
    {
        printf("PS/2 KBD: Couldn't set the scancode set!");
        return PS2_KBD_ERROR_SET_SCANCODE_SET_FAILED;
    }

    // Enable the interrupt used by the keyboard.
    pic_enable_irq(1);

    // Enable scanning to be able to get keyboard input.
    ps2_send_byte(ps2_kbd_port, PS2_KBD_ENABLE_SCANNING);

    ps2_kbd_state = PS2_KBD_STATE_NORMAL;

    return PS2_KBD_OK;
}