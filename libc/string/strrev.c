#include "string.h"

/*
    Reverses a string in place.

    @param str String to reverse.
*/
void strrev(char *str)
{
    size_t i = 0;
    size_t j = strlen(str) - 1;
    char temp;

    // Swap a value from the front with one from the back until the indices meet in the middle.
    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        // Move the indices closer together.
        i = i + 1;
        j = j - 1;
    } 
}