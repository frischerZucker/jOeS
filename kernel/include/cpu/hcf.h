/*!
    @file hcf.h

    @brief Defines a Halt-And-Catch-Fire function.

    @author frischerZucker
*/

#ifndef HCF_H
#define HCF_H

/*!
    @brief Halt-And-Catch-Fire function.

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