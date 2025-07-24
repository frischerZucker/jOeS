#include "string.h"

#include <stdint.h>

/*
    Converts an integer to a string in a given base.

    @param number Integer to convert.
    @param str Char array where the result will be stored.
    @param base Base used for the conversion.
*/
void itoa(int number, char *str, int base)
{
    int temp;
    size_t i = 0;
    uint8_t sign = 0;

    // The normal conversion doesn't work for 0, so we directly return the string "0\0".
    if (number == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    // Remember the sign so we can add a '-' to the string if needed.
    if (number < 0)
    {
        sign = 1;
    }

    // Continous division by the target base. The remainder is our digit.
    while (number != 0)
    {
        temp = number % base;
        // temp should always be positive, otherwise (temp > 9) won't work.
        if (sign)
        {
            temp = temp * -1;
        }
        
        str[i] = (temp > 9) ? 'a' + temp - 10 : '0' + temp;
        number = number / base;
        i = i + 1;
    }
    // Add a '-' if the number was negative.
    if (sign)
    {
        str[i] = '-';
        i = i + 1;
    }
    
    str[i] = '\0';

    // The algorithm built the string in reverse order, so we have to reverse it.
    strrev(str);
}