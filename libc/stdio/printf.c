#include "stdio.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "string.h"
#include "terminal.h"

#define MAX_FORMAT_STRING_LENGTH 256

#define STATE_NORMAL 0
#define STATE_FORMAT 1

#define FORMAT_SPECIFIER '%'
#define FORMAT_STRING 's'   
#define FORMAT_CHAR 'c'
#define FORMAT_INT1 'd'
#define FORMAT_INT2 'i'
#define FORMAT_UINT 'u'
#define FORMAT_OCTAL 'o'
#define FORMAT_HEX 'x'
#define FORMAT_POINTER 'p'
#define FORMAT_NO_OUTPUT 'n'
#define FORMAT_PREFIX_PLUS '+' // Prefix positive numbers with '+'.
#define FORMAT_PREFIX_SPACE ' ' // Prefix positive numbers with ' '.

#define NO_PREFIX '\0' // Value for pos_number_prefix if no prefix should be printed.

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

    char pos_number_prefix = NO_PREFIX; // Prefix to add to positive numbers.

    char format_string[MAX_FORMAT_STRING_LENGTH];
    size_t format_string_len = 0;

    int format_number;
    unsigned int unsigned_format_number;

    for (size_t i = 0; i < len; i++)
    {
        switch (state)
        {
        // Normal mode: Switch to format mode if a format specifier is found. Otherwise print the character.
        case STATE_NORMAL:
            if (str[i] == FORMAT_SPECIFIER)
            {
                state = STATE_FORMAT;
                pos_number_prefix = NO_PREFIX;
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
                format_number = va_arg(list, int);
                
                itoa(format_number, format_string, 10);

                format_string_len = strlen(format_string);

                // Add a trailing ' ' or '+' to positive numbers if needed.
                if (pos_number_prefix != '\0' && format_number >= 0)
                {
                    terminal_put_char(pos_number_prefix);
                    written = written + 1;
                }

                terminal_write_string(format_string, format_string_len);
                written = written + format_string_len;

                state = STATE_NORMAL;
                break;
            
            // Convert an unsigned integer to a string and print it.
            case FORMAT_UINT:
                unsigned_format_number = va_arg(list, unsigned int);
            
                utoa(unsigned_format_number, format_string, 10);

                format_string_len = strlen(format_string);

                terminal_put_char(pos_number_prefix); // Add a trailing ' ' or '+' if needed.
                terminal_write_string(format_string, format_string_len);
                written = written + format_string_len + 1;

                state = STATE_NORMAL;
                break;
            
            // Convert an unsigned integer to a string in base 8 and print it.
            case FORMAT_OCTAL:
                unsigned_format_number = va_arg(list, unsigned int);
            
                utoa(unsigned_format_number, format_string, 8);

                format_string_len = strlen(format_string);

                terminal_write_string(format_string, format_string_len);
                written = written + format_string_len;

                state = STATE_NORMAL;
                break;
            
            // Convert an unsigned integer to a string in base 16 and print it.
            case FORMAT_HEX:
                unsigned_format_number = va_arg(list, unsigned int);
            
                utoa(unsigned_format_number, format_string, 16);

                format_string_len = strlen(format_string);

                terminal_write_string(format_string, format_string_len);
                written = written + format_string_len;

                state = STATE_NORMAL;
                break;
            
            // Print a pointer as a base 16 integer.
            case FORMAT_POINTER:
                void *format_pointer = va_arg(list, void *);
                
                utoa((unsigned long)format_pointer, format_string, 16);

                format_string_len = strlen(format_string);

                terminal_write_string("0x", 2);
                terminal_write_string(format_string, format_string_len);
                written = written + format_string_len;

                state = STATE_NORMAL;
                break;
            
            case FORMAT_NO_OUTPUT:
                int *output_pointer = va_arg(list, int *);

                *output_pointer = written;
                state = STATE_NORMAL;
                break;

            // Prefix positive numbers with '+'.
            case FORMAT_PREFIX_PLUS:
                pos_number_prefix = '+';
                break;

            // Prefix positive numbers with ' '.
            case FORMAT_PREFIX_SPACE:
                pos_number_prefix = ' ';
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