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
    - Translate key, state & key event type to key events
    -> I could use a LUT with 256 entries for (key+state)->key event translation.
    -> Bit 7 is only used for deciding between pressed / released
    -> I could set it if the state was a prefix state, to distinguish keys
    - Append keyevents to some sort of buffer
*/
void ps2_kbd_irq_callback(void)
{
    uint8_t scancode_raw = 0;
    ps2_receive_byte(&scancode_raw);

    printf("PS/2 KBD: Scancode = %x. State = %x\n", scancode_raw, ps2_kbd_state);

    switch (ps2_kbd_state)
    {
    case PS2_KBD_STATE_NORMAL:
        if (scancode_raw == 0xe0)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E0);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E0;
            return;
        }
        else if (scancode_raw == 0xe1)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E1);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E1;
            return;
        }

        break;
        
    case PS2_KBD_STATE_PREFIX_E0:
        if (scancode_raw == 0x2a)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E02A);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E02A;
            return;
        }
        else if (scancode_raw == 0xb7)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E0B7);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E0B7;
            return;
        }

        break;
    
    case PS2_KBD_STATE_PREFIX_E02A:
        if (scancode_raw == 0xe0)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E02AE0);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E02AE0;
            return;
        }
        else
        {
            printf("PS/2 KBD: Invalid byte. Switching to state %d.\n", PS2_KBD_STATE_INVALID);
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;
        
    case PS2_KBD_STATE_PREFIX_E02AE0:
        if (scancode_raw != 0x37)
        {
            printf("PS/2 KBD: Invalid byte. Switching to state %d.\n", PS2_KBD_STATE_NORMAL);
            ps2_kbd_state = PS2_KBD_STATE_NORMAL;
            return;
        }

        break;
    
    case PS2_KBD_STATE_PREFIX_E0B7:
        if (scancode_raw == 0xe0)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E02AE0);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E02AE0;
            return;
        }
        else
        {
            printf("PS/2 KBD: Invalid byte. Switching to state %d.\n", PS2_KBD_STATE_INVALID);
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;
        
    case PS2_KBD_STATE_PREFIX_E0B7E0:
        if (scancode_raw != 0xaa)
        {
            printf("PS/2 KBD: Invalid byte. Switching to state %d.\n", PS2_KBD_STATE_NORMAL);
            ps2_kbd_state = PS2_KBD_STATE_NORMAL;
            return;
        }

        break;  

    case PS2_KBD_STATE_PREFIX_E1:
        if (scancode_raw == 0x1d)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E11D);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E11D;
            return;
        }
        else
        {
            printf("PS/2 KBD: Invalid byte. Switching to state %d.\n", PS2_KBD_STATE_INVALID);
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E11D:
        if (scancode_raw == 0x45)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E11D45);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E11D45;
            return;
        }
        else
        {
            printf("PS/2 KBD: Invalid byte. Switching to state %d.\n", PS2_KBD_STATE_INVALID);
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E11D45:
        if (scancode_raw == 0xe1)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E11D45E1);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E11D45E1;
            return;
        }
        else
        {
            printf("PS/2 KBD: Invalid byte. Switching to state %d.\n", PS2_KBD_STATE_INVALID);
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E11D45E1:
        if (scancode_raw == 0x9d)
        {
            printf("PS/2 KBD: Switching to state %d.\n", PS2_KBD_STATE_PREFIX_E11D45E19D);
            ps2_kbd_state = PS2_KBD_STATE_PREFIX_E11D45E19D;
            return;
        }
        else
        {
            printf("PS/2 KBD: Invalid byte. Switching to state %d.\n", PS2_KBD_STATE_INVALID);
            ps2_kbd_state = PS2_KBD_STATE_INVALID;
            return;
        }

        break;

    case PS2_KBD_STATE_PREFIX_E11D45E19D:
        if (scancode_raw != 0xc5)
        {
            printf("PS/2 KBD: Invalid byte. Switching to state %d.\n", PS2_KBD_STATE_NORMAL);
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

    printf("PS/2 KBD: Key: %d, State: %d, Key event type: %d.\n", scancode_raw & ~(1 << 7), ps2_kbd_state, (scancode_raw & (1 << 7)) > 1);
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
    // Abort if the driver is already initialized.
    if (ps2_kbd_state != PS2_KBD_STATE_UNINITIALIZED)
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

    ps2_kbd_state = PS2_KBD_STATE_NORMAL;

    return PS2_KBD_OK;
}