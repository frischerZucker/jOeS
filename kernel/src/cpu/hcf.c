#include "hcf.h"

/*
    Halt-And-Catch-Fire function.

    Stops all action and does nothing.
*/
void hcf(void)
{
    while (1)
    {
        asm("hlt");
    }
}