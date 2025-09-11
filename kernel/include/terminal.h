/*!
    @file terminal.h

    @brief Terminal interface for framebuffer-based text output.
    
    Provides basic terminal functionality for rendering text to a framebuffer,
    including cursor management, character rendering, and color control.

    @author frischerZucker
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>
#include <stdint.h>

#include <limine.h>

/*!
    @brief Contains information about the terminal. 
*/
struct terminal
{
    uint8_t cursor_x;
    uint8_t cursor_y;

    uint8_t char_w;
    uint8_t char_h;

    uint32_t fg_color;

    struct limine_framebuffer *framebuffer;
};

/*!
    @brief Error codes used by the terminal module.
*/
typedef enum terminal_error_codes
{
    TERMINAL_OK = 0,
    TERMINAL_ERROR_UNHANDLED_CHARACTER,
} terminal_error_codes_t;

/*!
    @brief Initializes the terminal structure.

    Sets the cursor position to the top-left corner, stores the character size and the framebuffer for drawing characters.

    @param framebuffer Pointer to the framebuffer to render text to.
    @param char_w Width of each character in pixels.
    @param char_h Height of each character in pixels.
*/
void terminal_init(struct limine_framebuffer *framebuffer, uint8_t char_w, uint8_t char_h);

/*!
    @brief Sets the foreground color used to render text.

    @param color 24-bit RGB color value.
*/
void terminal_set_color(uint32_t color);

/*!
    @brief Draws a single character on the screen and updates the cursor position.

    Handles whitespace and basic control characters:
    - Space (' ') moves the cursor right without drawing.
    - Tab ('\t) advances the dursor to the next tab stop.
    - NewLine ('\n) moves the cursor to the beginning of the next line.
    - Printable characters (32 - 126) are drawn to the framebuffer using \ref framebuffer_draw_char().

    Returns an error, if the character is not handled by this function.
    Automatically moves to the next line if the cursor exceeds the screen width.

    @param c ASCII character to draw.
    @returns TERMINAL_OK on success, TERMINAL_ERROR_UNHANDLED_CHARACTER if character is not handled by this function.
*/
terminal_error_codes_t terminal_put_char(uint8_t c);

/*!
    @brief Writes a string of characters to the terminal.

    Iterates through the input string and prints it by calling \ref terminal_put_char() for every character.

    @param str Pointer to the character array to write.
    @param len Length of the string in characters.
*/
void terminal_write_string(char *str, size_t len);

#endif // TERMINAL_H