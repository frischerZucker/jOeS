#include <terminal.h>

#include <charset.h>
#include <framebuffer.h>

static struct terminal terminal;

/*!
    @brief Initializes the terminal structure.

    Sets the cursor position to the top-left corner, stores the character size and the framebuffer for drawing characters.

    @param framebuffer Pointer to the framebuffer to render text to.
    @param char_w Width of each character in pixels.
    @param char_h Height of each character in pixels.
*/
void terminal_init(struct limine_framebuffer *framebuffer, uint8_t char_w, uint8_t char_h)
{
    terminal.cursor_x = 0;
    terminal.cursor_y = 0;

    terminal.char_w = char_w;
    terminal.char_h = char_h;

    terminal.fg_color = 0xffffff;

    terminal.framebuffer = framebuffer;
}

/*!
    @brief Sets the foreground color used to render text.

    @param color 24-bit RGB color value.
*/
void terminal_set_color(uint32_t color)
{
    terminal.fg_color = color;
}

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
void terminal_put_char(uint8_t c)
{
    switch (c)
    {
    case ' ': // space -> move the cursor right and draw nothing
        terminal.cursor_x = terminal.cursor_x + 1;
        break;

    case '\n': // line break -> move the cursor to the beginning of the next line
        terminal.cursor_x = 0;
        terminal.cursor_y = terminal.cursor_y + 1;
        break;

    case '\t': // tab -> move the cursore right until it reaches a multiple of the defined tab width, draw nothing
        do
        {
            terminal.cursor_x = terminal.cursor_x + 1;
        } while (terminal.cursor_x % TAB_WIDTH != 0);
        break;

    default: // draw the character, move the cursor one position right afterwards
        // c is not printable -> exit with error code
        if (c < 32 || c > 126)
        {
            return;
        }

        framebuffer_draw_char(terminal.framebuffer, c, terminal.cursor_x * terminal.char_w, terminal.cursor_y * terminal.char_h, terminal.fg_color);
        terminal.cursor_x = terminal.cursor_x + 1;
        break;
    }

    // goes to the beginning of the next line if the cursor moved out of the screen
    if ((uint64_t)((terminal.cursor_x + 1) * terminal.char_w) > terminal.framebuffer->width)
    {
        terminal.cursor_x = 0;
        terminal.cursor_y = terminal.cursor_y + 1;
    }

    // Clear the screen when its end is reached.
    if ((uint64_t)((terminal.cursor_y + 1) * terminal.char_h) > terminal.framebuffer->height)
    {
        terminal.cursor_x = 0;
        terminal.cursor_y = 0;
        framebuffer_clear(terminal.framebuffer, 0);
    }
}

/*!
    @brief Writes a string of characters to the terminal.

    Iterates through the input string and prints it by calling \ref terminal_put_char() for every character.

    @param str Pointer to the character array to write.
    @param len Length of the string in characters.
*/
void terminal_write_string(char *str, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        terminal_put_char(str[i]);
    }
}

/*
    Disable warnings for unused parameters.
    context is not used, but needs to be there so that the function can be used for logging.
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
/*!
    @brief Logging interface implementation for the terminal.

    @param c Character to log.
    @param context Additional parameters (not used for terminal).
*/
void terminal_log_write(uint8_t c, void *context)
{
    terminal_put_char(c);
}
// Enable "-Wunused-parameter" again.
#pragma GCC diagnostic pop 