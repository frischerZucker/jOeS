#include <terminal.h>

#include <charset.h>
#include <framebuffer.h>

static struct terminal terminal;

void terminal_init(struct limine_framebuffer *framebuffer, uint8_t char_w, uint8_t char_h)
{
    terminal.cursor_x = 0;
    terminal.cursor_y = 0;

    terminal.char_w = char_w;
    terminal.char_h = char_h;

    terminal.fg_color = 0xffffff;

    terminal.framebuffer = framebuffer;
}

uint8_t terminal_set_color(uint32_t color)
{
    terminal.fg_color = color;

    return 0;
}

uint8_t terminal_put_char(uint8_t c)
{
    switch (c)
    {
    case ' ': // space -> move the cursor to the right and draw nothing
        terminal.cursor_x = terminal.cursor_x + 1;
        break;
    
    case '\n': // line break -> move the cursor all the way to the left and one position down, draw nothing
        terminal.cursor_x = 0;
        terminal.cursor_y = terminal.cursor_y + 1;
        break;
    
    case '\t': // tab -> move the cursore to the right until it reaches a multiple of the defined tab width, draw nothing
        do
        {
            terminal.cursor_x = terminal.cursor_x + 1;
        } while (terminal.cursor_x % TAB_WIDTH != 0);
        break;

    default: // draw the character, move the cursor one position to the right afterwards
        // c is not printable -> exit with error code
        if (c < 32 || c > 126)
        {
            return -1;
        }
        
        framebuffer_draw_char(terminal.framebuffer, c, terminal.cursor_x*terminal.char_w, terminal.cursor_y*terminal.char_h, terminal.fg_color);
        terminal.cursor_x = terminal.cursor_x + 1;
        break;
    }

    // goes to the beginning of the next line if the cursor moved out of the screen
    if ((terminal.cursor_x + 1)*terminal.char_w > terminal.framebuffer->width)
    {
        terminal.cursor_x = 0;
        terminal.cursor_y = terminal.cursor_y + 1;
    }

    return 0;
}