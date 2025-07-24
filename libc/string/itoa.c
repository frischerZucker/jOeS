#include "string.h"

#include <stdint.h>

void itoa(int number, char *str, int base)
{
    int temp;
    size_t i = 0;
    uint8_t sign = 0;

    if (number == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    if (number < 0)
    {
        sign = 1;
    }

    while (number != 0)
    {
        temp = number % base;
        if (sign)
        {
            temp = temp * -1;
        }
        
        str[i] = (temp > 9) ? 'a' + temp - 10 : '0' + temp;
        number = number / base;
        i = i + 1;
    }
    
    if (sign)
    {
        str[i] = '-';
        i = i + 1;
    }
    
    str[i] = '\0';

    strrev(str);
}