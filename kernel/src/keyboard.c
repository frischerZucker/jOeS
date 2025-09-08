#include "keyboard.h"

#include <stddef.h>

/*
    Look up table for converting key events to ASCII characters for a german keyboard layout (QWERTZ).
    For now only ASCII characters are supported, as for other characters there is no charset.
    Umlaute "ä", "ö" and "ü" are replaced with "ae", "oe" and "ue". "ß" is replaced with "ss".
*/
// Disable warnings for overwriting already initialized values during array initialization.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
struct keyboard_layout_t kbd_layout_german = 
{
    .normal = {
        [0 ... 255] = "",

        // Zahlenreihe
        [KEY_CIRCUMFLEX] = "^",   // Dead key in Realität
        [KEY_1] = "1",
        [KEY_2] = "2",
        [KEY_3] = "3",
        [KEY_4] = "4",
        [KEY_5] = "5",
        [KEY_6] = "6",
        [KEY_7] = "7",
        [KEY_8] = "8",
        [KEY_9] = "9",
        [KEY_0] = "0",
        [KEY_ß] = "ss",
        [KEY_BACK_TICK] = "'",    // Akzent
        [KEY_BACKSPACE] = "\b",

        // QWERTZ-Reihe
        [KEY_TAB] = "\t",
        [KEY_Q] = "q",
        [KEY_W] = "w",
        [KEY_E] = "e",
        [KEY_R] = "r",
        [KEY_T] = "t",
        [KEY_Z] = "z",
        [KEY_U] = "u",
        [KEY_I] = "i",
        [KEY_O] = "o",
        [KEY_P] = "p",
        [KEY_Ü] = "ue",
        [KEY_PLUS] = "+",
        [KEY_ENTER] = "\n",

        // ASDF-Reihe
        [KEY_A] = "a",
        [KEY_S] = "s",
        [KEY_D] = "d",
        [KEY_F] = "f",
        [KEY_G] = "g",
        [KEY_H] = "h",
        [KEY_J] = "j",
        [KEY_K] = "k",
        [KEY_L] = "l",
        [KEY_Ö] = "oe",
        [KEY_Ä] = "ae",
        [KEY_HASHTAG] = "#",

        // YXCVBNM-Reihe
        [KEY_LESS_THEN] = "<",
        [KEY_Y] = "y",
        [KEY_X] = "x",
        [KEY_C] = "c",
        [KEY_V] = "v",
        [KEY_B] = "b",
        [KEY_N] = "n",
        [KEY_M] = "m",
        [KEY_COMMA] = ",",
        [KEY_PERIOD] = ".",
        [KEY_MINUS] = "-",

        // Space
        [KEY_SPACE] = " ",
    
        // Keypad (nur normale Ziffern/Symbole)
        [KEY_KEYPAD_SLASH] = "/",
        [KEY_KEYPAD_ASTERISK] = "*",
        [KEY_KEYPAD_MINUS] = "-",
        [KEY_KEYPAD_7] = "7",
        [KEY_KEYPAD_8] = "8",
        [KEY_KEYPAD_9] = "9",
        [KEY_KEYPAD_PLUS] = "+",
        [KEY_KEYPAD_4] = "4",
        [KEY_KEYPAD_5] = "5",
        [KEY_KEYPAD_6] = "6",
        [KEY_KEYPAD_1] = "1",
        [KEY_KEYPAD_2] = "2",
        [KEY_KEYPAD_3] = "3",
        [KEY_KEYPAD_0] = "0",
        [KEY_KEYPAD_DELETE] = ".",
        [KEY_KEYPAD_ENTER] = "\n"
    },

    .shift = {
        [0 ... 255] = "",

        // Zahlenreihe mit Shift
        [KEY_1] = "!",
        [KEY_2] = "\"",
        [KEY_4] = "$",
        [KEY_5] = "%",
        [KEY_6] = "&",
        [KEY_7] = "/",
        [KEY_8] = "(",
        [KEY_9] = ")",
        [KEY_0] = "=",
        [KEY_ß] = "?",
        [KEY_BACK_TICK] = "`",    // Shift + ´  → `
        
        // QWERTZ-Reihe mit Shift
        [KEY_Q] = "Q",
        [KEY_W] = "W",
        [KEY_E] = "E",
        [KEY_R] = "R",
        [KEY_T] = "T",
        [KEY_Z] = "Z",
        [KEY_U] = "U",
        [KEY_I] = "I",
        [KEY_O] = "O",
        [KEY_P] = "P",
        [KEY_Ü] = "UE",
        [KEY_PLUS] = "*",

        // ASDF-Reihe mit Shift
        [KEY_A] = "A",
        [KEY_S] = "S",
        [KEY_D] = "D",
        [KEY_F] = "F",
        [KEY_G] = "G",
        [KEY_H] = "H",
        [KEY_J] = "J",
        [KEY_K] = "K",
        [KEY_L] = "L",
        [KEY_Ö] = "OE",
        [KEY_Ä] = "AE",
        [KEY_HASHTAG] = "\'",

        // YXCVBNM-Reihe mit Shift
        [KEY_LESS_THEN] = ">",    // Shift + <  → >
        [KEY_Y] = "Y",
        [KEY_X] = "X",
        [KEY_C] = "C",
        [KEY_V] = "V",
        [KEY_B] = "B",
        [KEY_N] = "N",
        [KEY_M] = "M",
        [KEY_COMMA] = ";",
        [KEY_PERIOD] = ":",
        [KEY_MINUS] = "_",

        // Space bleibt gleich
        [KEY_SPACE] = " ",

        // Keypad (bei Shift meist gleich)
        [KEY_KEYPAD_SLASH] = "/",
        [KEY_KEYPAD_ASTERISK] = "*",
        [KEY_KEYPAD_MINUS] = "-",
        [KEY_KEYPAD_7] = "7",
        [KEY_KEYPAD_8] = "8",
        [KEY_KEYPAD_9] = "9",
        [KEY_KEYPAD_PLUS] = "+",
        [KEY_KEYPAD_4] = "4",
        [KEY_KEYPAD_5] = "5",
        [KEY_KEYPAD_6] = "6",
        [KEY_KEYPAD_1] = "1",
        [KEY_KEYPAD_2] = "2",
        [KEY_KEYPAD_3] = "3",
        [KEY_KEYPAD_0] = "0",
        [KEY_KEYPAD_DELETE] = "."
    },

    .altgr = {
        [0 ... 255] = "",
    
        // Zahlenreihe mit AltGr
        [KEY_Q] = "@",      // AltGr + Q → @

        // Zeichen rechts oben
        [KEY_7] = "{",      // AltGr + 7 → {
        [KEY_8] = "[",      // AltGr + 8 → [
        [KEY_9] = "]",      // AltGr + 9 → ]
        [KEY_0] = "}",      // AltGr + 0 → }
        [KEY_ß] = "\\",     // AltGr + ß → Backslash

        // QWERTZ-Reihe
        [KEY_PLUS] = "~",   // AltGr + + → ~

        // ASDF-Reihe
        [KEY_HASHTAG] = "\'", // AltGr + # → "
        
        // YXCVBNM-Reihe
        [KEY_LESS_THEN] = "|", // AltGr + < → |
    },

    .ctrl = {
        [0 ... 255] = ""
    }
};
// Enable "-Woverride-init" again.
#pragma GCC diagnostic pop 

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