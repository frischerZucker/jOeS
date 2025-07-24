#include "string.h"

void strrev(char *str)
{
    size_t i = 0;
    size_t j = strlen(str) - 1;
    char temp;

    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;

        i = i + 1;
        j = j - 1;
    } 
}