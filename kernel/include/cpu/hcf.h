#ifndef HCF_H
#define HCF_H

/*
    Halt-And-Catch-Fire function.

    Stops all action and does nothing.
*/
static inline void hcf(void)
{
    while (1)
    {
        asm("hlt");
    }
}

#endif // HCF_H