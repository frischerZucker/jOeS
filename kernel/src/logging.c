#include <stdarg.h>

#include "stdio.h"
#include "string.h"

#include "logging.h"

#define MAX_FORMAT_STRING_LENGTH 256

typedef enum {
    FMT_STATE_NORMAL,
    FMT_STATE_FORMAT
} fmt_state_t;

typedef enum {
    FMT_SPECIFIER_SPECIFIER = '%',
    FMT_SPECIFIER_STRING = 's',   
    FMT_SPECIFIER_CHAR = 'c',
    FMT_SPECIFIER_INT1 = 'd',
    FMT_SPECIFIER_INT2 = 'i',
    FMT_SPECIFIER_UINT = 'u',
    FMT_SPECIFIER_OCTAL = 'o',
    FMT_SPECIFIER_HEX = 'x',
    FMT_SPECIFIER_POINTER = 'p',
    FMT_SPECIFIER_NO_OUTPUT = 'n',
    FMT_SPECIFIER_PREFIX_PLUS = '+', // Prefix positive numbers with '+'.
    FMT_SPECIFIER_PREFIX_SPACE = ' ' // Prefix positive numbers with ' '.
} fmt_specifier_t;

#define NO_PREFIX '\0' // Value for pos_number_prefix if no prefix should be printed.

static struct logging_backend backend = { NULL, NULL, LOGGING_LEVEL_DEBUG};

/*!
    @brief Write a string to the logging backend.

    Writes a string character for character to the logging backend.

    @param str String to write.
    @param len Length of the string.
*/
static void log_str(const char *str, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        backend.write(str[i], backend.context);
    }
}

/*!
    @brief Writes a format string to the logging backend.

    It's copied from stdio.h/printf() and modified to fit here.

    @param str Format string to write to the backend.
    @param list Variadic arguments list containing the format arguments.
*/
static void log_formated_str(const char *str, va_list list)
{
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
        case FMT_STATE_NORMAL:
            if (str[i] == FMT_SPECIFIER_SPECIFIER)
            {
                state = FMT_STATE_FORMAT;
                pos_number_prefix = NO_PREFIX;
            }
            else
            {
                backend.write(str[i], backend.context);
            }
            break;
        // Format mode: Print something depending on the character after the format specifier and change back to normal mode.
        case FMT_STATE_FORMAT:
            switch (str[i])
            {
            // Print a character.
            case FMT_SPECIFIER_CHAR:
                backend.write((char)va_arg(list, int), backend.context);
                state = FMT_STATE_NORMAL;
                break;

            // Print a string.
            case FMT_SPECIFIER_STRING:
                char *s = va_arg(list, char *);
                size_t s_len = strlen(s);
                log_str(s, s_len);
                state = FMT_STATE_NORMAL;
                break;
            
            // Print the format specifier.
            case FMT_SPECIFIER_SPECIFIER:
                backend.write(FMT_SPECIFIER_SPECIFIER, backend.context);
                state = FMT_STATE_NORMAL;
                break;

            // Convert an integer to a string and print it.
            case FMT_SPECIFIER_INT1:
            case FMT_SPECIFIER_INT2:
                format_number = va_arg(list, int);
                
                itoa(format_number, format_string, 10);

                format_string_len = strlen(format_string);

                // Add a trailing ' ' or '+' to positive numbers if needed.
                if (pos_number_prefix != '\0' && format_number >= 0)
                {
                    backend.write(pos_number_prefix, backend.context);
                }

                log_str(format_string, format_string_len);

                state = FMT_STATE_NORMAL;
                break;
            
            // Convert an unsigned integer to a string and print it.
            case FMT_SPECIFIER_UINT:
                unsigned_format_number = va_arg(list, unsigned int);
            
                utoa(unsigned_format_number, format_string, 10);

                format_string_len = strlen(format_string);

                backend.write(pos_number_prefix, backend.context); // Add a trailing ' ' or '+' if needed.
                log_str(format_string, format_string_len);

                state = FMT_STATE_NORMAL;
                break;
            
            // Convert an unsigned integer to a string in base 8 and print it.
            case FMT_SPECIFIER_OCTAL:
                unsigned_format_number = va_arg(list, unsigned int);
            
                utoa(unsigned_format_number, format_string, 8);

                format_string_len = strlen(format_string);

                log_str(format_string, format_string_len);

                state = FMT_STATE_NORMAL;
                break;
            
            // Convert an unsigned integer to a string in base 16 and print it.
            case FMT_SPECIFIER_HEX:
                unsigned_format_number = va_arg(list, unsigned int);
            
                utoa(unsigned_format_number, format_string, 16);

                format_string_len = strlen(format_string);

                log_str(format_string, format_string_len);

                state = FMT_STATE_NORMAL;
                break;
            
            // Print a pointer as a base 16 integer.
            case FMT_SPECIFIER_POINTER:
                void *format_pointer = va_arg(list, void *);
                
                utoa((unsigned long)format_pointer, format_string, 16);

                format_string_len = strlen(format_string);

                backend.write('0', backend.context);
                backend.write('x', backend.context);
                log_str(format_string, format_string_len);

                state = FMT_STATE_NORMAL;
                break;
            
            // Prefix positive numbers with '+'.
            case FMT_SPECIFIER_PREFIX_PLUS:
                pos_number_prefix = '+';
                break;

            // Prefix positive numbers with ' '.
            case FMT_SPECIFIER_PREFIX_SPACE:
                pos_number_prefix = ' ';
                break;
                
            default:
                LOG_ERROR("Unknown format specifier: %c", str[i]);
                return;
                break;
            }

        default:
            break;
        }
    }
}

/*!
    @brief Select a backend for logging.

    Select a backend that is used for logging. (e.g terminal, serial)
    The backend needs to implement a logging interface of the form:
        void BACKEND_NAME_log_write(uint8_t c, void *context)

    @param log_func Logging interface that should be used.
    @param context Additional parameters required by the logging interface.
*/
void logging_set_backend(void (*log_func)(uint8_t, void *), void *context)
{
    backend.write = log_func;
    backend.context = context;
}

/*!
    @brief Select a logging level.

    Select a logging level to suppress the logging of messages below that level.

    @param l Level.
*/
void logging_set_level(logging_level_t l)
{
    backend.level = l;
}

/*!
    @brief Logs a formated message to the current logging backend.

    Logs a message appending the logging level in front of it.
    Only messages that have a logging level greater or equal then the selected level of the backend are logged.
    At levels other then LOGGING_LEVEL_INFO the file where the message originates from is added between the level and the message.

    @param level Logging level of the message.
    @param file String containing the name of file the message originates from.
    @param msg Format string to log.
    @param ... Format arguments.
*/
void logging_log_msg(const logging_level_t level, const char *file, const char *msg, ...)
{
    if (level < backend.level)
    {
        return;
    }

    va_list list;
    va_start(list, msg);

    switch (level)
    {
    case LOGGING_LEVEL_DEBUG:
        log_str("[DEBUG] ", 8);
        break;

    case LOGGING_LEVEL_INFO:
        log_str("[INFO] ", 8);
        break;

    case LOGGING_LEVEL_WARNING:
        log_str("[WARNING] ", 10);
        break;

    case LOGGING_LEVEL_ERROR:
        log_str("[ERROR] ", 8);
        break;

    default:
        break;
    }

    if (level != LOGGING_LEVEL_INFO)
    {
        log_str(file, strlen(file));
        backend.write(':', backend.context);
        backend.write(' ', backend.context);
    }

    log_formated_str(msg, list);

    backend.write('\n', backend.context);

    va_end(list);
}