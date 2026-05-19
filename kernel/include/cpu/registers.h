#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

static inline uint64_t read_cr3()
{
    uint64_t cr3;

    asm(
        "mov %%cr3, %0"
        : "=r"(cr3)
    );

    return cr3;    
}

static inline void set_cr3(uint64_t value)
{
    asm(
        "mov %0, %%cr3"
        :
        :"r"(value)
    );
}

#endif // REGISTERS_H