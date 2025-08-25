#include "stdio.h"

#include <stdarg.h>
#include <stdint.h>

#include "string.h"
#include "terminal.h"

/*
    Writes a string to the terminal and substitutes format specifiers (%...) with variables.
    
    Supported format specifiers are:
    - %d signed integer
    - %i signed integer
    - %c char
    - %s string.
    - %% Prints a '%'.

    @param str String to print.
    @param ... Optional variables to substitute format specifiers with.
    @returns Number of characers printed, -1 if an undefined format specifier was hit.
*/
size_t printf(char *str, ...)
{
    va_list list;
    va_start(list, str);

    size_t written = 0; // Counts printed characters.
    size_t len = strlen(str);

    uint8_t state = 0;

    for (size_t i = 0; i < len; i++)
    {
        switch (state)
        {
        // Normal mode: Switch to format mode if a format specifier is found. Otherwise print the character.
        case STATE_NORMAL:
            if (str[i] == FORMAT_SPECIFIER)
            {
                state = STATE_FORMAT;
            }
            else
            {
                terminal_put_char(str[i]);
                written = written + 1;
            }
            break;
        // Format mode: Print something depending on the character after the format specifier and change back to normal mode.
        case STATE_FORMAT:
            switch (str[i])
            {
            // Print a character.
            case FORMAT_CHAR:
                terminal_put_char((char)va_arg(list, int));
                written = written + 1;
                state = STATE_NORMAL;
                break;

            // Print a string.
            case FORMAT_STRING:
                char *s = va_arg(list, char *);
                size_t s_len = strlen(s);
                terminal_write_string(s, s_len);
                written = written + s_len;
                state = STATE_NORMAL;
                break;
            
            // Print the format specifier.
            case FORMAT_SPECIFIER:
                terminal_put_char(FORMAT_SPECIFIER);
                written = written + 1;
                state = STATE_NORMAL;
                break;

            // Convert an integer to a string and print it.
            case FORMAT_INT1:
            case FORMAT_INT2:
                int number = va_arg(list, int);
                char temp[INT_MAX_CHARS];
                
                itoa(number, temp, 10);

                size_t temp_len = strlen(temp);

                terminal_write_string(temp, temp_len);
                written = written + temp_len;

                state = STATE_NORMAL;
                break;

            default:
                return -1;
                break;
            }

        default:
            break;
        }
    }

    va_end(list);

    return written;
}