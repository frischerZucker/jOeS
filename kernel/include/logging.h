/*!
    @file logging.h

    @brief Logging module with backend abstraction.
    
    Provides functions, macros and type definitions for logging.
    Supports multiple logging levels (DEBUG, INFO, WARNING, ERROR) and allows selecting a backend (e.g. terminal, serial) via a simple interface.

    Logging macros automatically prepend the logging level and, except for INFO, also include the originating file name.
    Messages below the configured minimum level (LOGGING_MIN_LEVEL) are compiled out to reduce overhead.

    Backends must implement a write function of the form:
        void BACKEND_NAME_log_write(uint8_t c, void *context)

    @author frischerZucker
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <stdint.h>

/*!
    @brief Supported logging levels.
*/
typedef enum {
    LOGGING_LEVEL_DEBUG,
    LOGGING_LEVEL_INFO,
    LOGGING_LEVEL_WARNING,
    LOGGING_LEVEL_ERROR
} logging_level_t;

/*!
    @brief Structure containing information about a logging backend.
*/
struct logging_backend {
    void (*write)(uint8_t, void *);
    void *context;
    logging_level_t level;
};

/*!
    @brief Logging macros for different logging levels.

    Logging macros that call the logging function with the correct logging level and also pass the file name to it.
    Macros below the defined minimum level (LOGGING_MIN_LEVEL) are replaced by ((void)0) to remove them from the code.

    @param msg Format string to log.
    @param ... Format arguments.
*/
#if(LOGGING_MIN_LEVEL == 0)
    #define LOG_DEBUG(msg, ...) logging_log_msg(LOGGING_LEVEL_DEBUG, __FILE__, msg, ##__VA_ARGS__)
    #define LOG_INFO(msg, ...) logging_log_msg(LOGGING_LEVEL_INFO, __FILE__, msg, ##__VA_ARGS__)
    #define LOG_WARNING(msg, ...) logging_log_msg(LOGGING_LEVEL_WARNING, __FILE__, msg, ##__VA_ARGS__)
    #define LOG_ERROR(msg, ...) logging_log_msg(LOGGING_LEVEL_ERROR, __FILE__, msg, ##__VA_ARGS__)
#elif(LOGGING_MIN_LEVEL == 1)
    #define LOG_DEBUG(msg, ...) ((void)0)
    #define LOG_INFO(msg, ...) logging_log_msg(LOGGING_LEVEL_INFO, __FILE__, msg, ##__VA_ARGS__)
    #define LOG_WARNING(msg, ...) logging_log_msg(LOGGING_LEVEL_WARNING, __FILE__, msg, ##__VA_ARGS__)
    #define LOG_ERROR(msg, ...) logging_log_msg(LOGGING_LEVEL_ERROR, __FILE__, msg, ##__VA_ARGS__)
#elif(LOGGING_MIN_LEVEL == 2)
    #define LOG_DEBUG(msg, ...) ((void)0)
    #define LOG_INFO(msg, ...) ((void)0)
    #define LOG_WARNING(msg, ...) logging_log_msg(LOGGING_LEVEL_WARNING, __FILE__, msg, ##__VA_ARGS__)
    #define LOG_ERROR(msg, ...) logging_log_msg(LOGGING_LEVEL_ERROR, __FILE__, msg, ##__VA_ARGS__)
#elif(LOGGING_MIN_LEVEL == 3)
    #define LOG_DEBUG(msg, ...) ((void)0)
    #define LOG_INFO(msg, ...) ((void)0)
    #define LOG_WARNING(msg, ...) ((void)0)
    #define LOG_ERROR(msg, ...) logging_log_msg(LOGGING_LEVEL_ERROR, __FILE__, msg, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(msg, ...) logging_log_msg(LOGGING_LEVEL_DEBUG, __FILE__, msg, ##__VA_ARGS__)
    #define LOG_INFO(msg, ...) logging_log_msg(LOGGING_LEVEL_INFO, __FILE__, msg, ##__VA_ARGS__)
    #define LOG_WARNING(msg, ...) logging_log_msg(LOGGING_LEVEL_WARNING, __FILE__, msg, ##__VA_ARGS__)
    #define LOG_ERROR(msg, ...) logging_log_msg(LOGGING_LEVEL_ERROR, __FILE__, msg, ##__VA_ARGS__)
#endif

/*!
    @brief Select a backend for logging.

    Select a backend that is used for logging. (e.g terminal, serial)
    The backend needs to implement a logging interface of the form:
        void BACKEND_NAME_log_write(uint8_t c, void *context)

    @param log_func Logging interface that should be used.
    @param context Additional parameters required by the logging interface.
*/
void logging_set_backend(void (*log_func)(uint8_t, void *), void *context);
/*!
    @brief Select a logging level.

    Select a logging level to suppress the logging of messages below that level.

    @param l Level.
*/
void logging_set_level(logging_level_t l);

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
void logging_log_msg(const logging_level_t level, const char *file, const char * msg, ...);

#endif // LOGGING_H