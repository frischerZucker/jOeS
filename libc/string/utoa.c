#include "string.h"

#include <stdint.h>

/*
    Converts an unsigned integer to a string in a given base.

    @param number Number to convert (has to fit in an unsigned long).
    @param str Char array where the result will be stored.
    @param base Base used for the conversion.
*/
void utoa(unsigned long number, char *str, int base)
{
    int temp;
    size_t i = 0;

    // The normal conversion doesn't work for 0, so we directly return the string "0\0".
    if (number == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    // Continous division by the target base. The remainder is our digit.
    while (number != 0)
    {
        temp = number % base;
        
        str[i] = (temp > 9) ? 'a' + temp - 10 : '0' + temp;
        number = number / base;
        i = i + 1;
    }
    
    str[i] = '\0';

    // The algorithm built the string in reverse order, so we have to reverse it.
    strrev(str);
}