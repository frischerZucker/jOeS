#include "keyboard.h"

#include "keyboard_layouts.h"

/*
    Converts a key event into its corresponding ASCI character(s).

    Translates a key_event_t into a character string based on current modifier
    flags and the german keyboard layout. Only key press events produce output. 
    Key releases are ignored. Returns an empty string if the key was released
    or no valid character mapping exists.

    @param key_event Pointer to the key event to convert.
    @returns Pointer to a character string (may be empty).
*/
char * key_event_to_ascii(struct key_event_t *key_event)
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