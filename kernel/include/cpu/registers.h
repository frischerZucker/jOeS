#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

#include "logging.h"

static inline uint64_t read_cr2()
{
    uint64_t cr2;

    asm(
        "mov %%cr2, %0"
        : "=r"(cr2)
    );

    return cr2;    
}

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
    LOG_DEBUG("1");
    asm(
        "mov %0, %%cr3"
        :
        :"r"(value)
    );
    LOG_DEBUG("2");
}

#endif // REGISTERS_H