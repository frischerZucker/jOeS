#include "drivers/keyboard.h"

#include "drivers/keyboard_layouts.h"

struct key_event_t kbd_key_event_buffer[KBD_KEY_EVENT_BUFFER_SIZE] = {0};
uint8_t kbd_key_event_buffer_capacity = KBD_KEY_EVENT_BUFFER_SIZE;
uint8_t kbd_key_event_buffer_size = 0;
uint8_t kbd_key_event_buffer_front = 0;

/*!
    @brief Appends a key event to the key event buffer.

    Appends a key event to the key event ring buffer. If the buffer is full, old values are overwritten.

    @param key_event Pointer to the key event to append.
*/
void kbd_append_key_event_to_buffer(struct key_event_t *key_event)
{    
    uint8_t rear = (kbd_key_event_buffer_front + kbd_key_event_buffer_size) % KBD_KEY_EVENT_BUFFER_SIZE;

    kbd_key_event_buffer[rear] = *key_event;

    if (kbd_key_event_buffer_size < kbd_key_event_buffer_capacity)
    {
        kbd_key_event_buffer_size = kbd_key_event_buffer_size + 1;
    }
    else
    {
        kbd_key_event_buffer_front = (kbd_key_event_buffer_front + 1) % KBD_KEY_EVENT_BUFFER_SIZE;
    }
}

/*!
    @brief Retrieves the next key event from the key event buffer.

    Tries to read a key event from the key event ring buffer.
    On success the key event is read an removed from the buffer.
    If the buffer is empty, the function returns an error code and does not modify *dest.

    @param dest Pointer to where the key event will be stored.
    @returns KBD_KEY_EVENT_BUFFER_OK on success, KBD_KEY_EVENT_BUFFER_EMPTY if the buffer is empty.
*/
kbd_error_codes_t kbd_get_key_event_from_buffer(struct key_event_t *dest)
{
    if (kbd_key_event_buffer_size <= 0)
    {
        return KBD_ERROR_KEY_EVENT_BUFFER_EMPTY;
    }

    *dest = kbd_key_event_buffer[kbd_key_event_buffer_front];

    kbd_key_event_buffer_front = (kbd_key_event_buffer_front + 1) % KBD_KEY_EVENT_BUFFER_SIZE;
    kbd_key_event_buffer_size = kbd_key_event_buffer_size - 1;

    return KBD_ERROR_KEY_EVENT_BUFFER_OK;
}

/*!
    @brief Converts a key event into its corresponding ASCII character(s).

    Translates a key_event_t into a character string based on current modifier
    flags and the german keyboard layout. Only key press events produce output. 
    Key releases are ignored. Returns an empty string if the key was released
    or no valid character mapping exists.

    @param key_event Pointer to the key event to convert.
    @returns Pointer to a character string (may be empty).
*/
char * kbd_key_event_to_ascii(struct key_event_t *key_event)
{
    // Only return characters when a key was pressed.
    if (key_event->pressed == KEY_EVENT_TYPE_RELEASED)
    {
        return "";
    }
    
    if (key_event->modifiers & KBD_MODIFIER_MASK_ALTGR)
    {
        return kbd_layout_german.altgr[key_event->keycode];
    }
    
    if (key_event->modifiers & KBD_MODIFIER_MASK_CAPS_LOCK)
    {
        // If Caps Lock is on, use normal table when shift is pressed and shift table when not.
        if (key_event->modifiers & KBD_MODIFIER_MASK_SHIFT)
        {
            return kbd_layout_german.normal[key_event->keycode];
        }
        else
        {
            return kbd_layout_german.shift[key_event->keycode];
        }
    }
    else
    {
        // If Caps Lock is off, use shift table when shift is pressed and the normal one when not.
        if (key_event->modifiers & KBD_MODIFIER_MASK_SHIFT)
        {
            return kbd_layout_german.shift[key_event->keycode];
        }
        else
        {
            return kbd_layout_german.normal[key_event->keycode];
        }
    }
    
    return "";
}