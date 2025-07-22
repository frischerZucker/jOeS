#include <string.h>

/*
    Determines the lenght of a string.

    Iterates over the string until a '\0' is found to get the strings length by counting the characters before '\0'.

    @param str String
    @returns Length of str.
*/
size_t strlen(const char *str)
{
    size_t i = 0;
    while (str[i] != '\0')
    {
        i = i + 1;
    }

    return i;
}