/*!
    @file gdt.h

    @brief Global Descriptor Table (GDT) setup for x86_64 systems.
    
    Defines structures and functions for initializing and installing a GDT for a flat memory model.
    Assumes a standard five-entry layout with segments for:
    - null
    - kernel code
    - kernel data
    - user code
    - user data
    Each segment spans the full 4 GiB address space with 4 kiB granularity.

    @author frischerZucker
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_NUM_ENTRIES 5

struct gdt_descriptor
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_high_flags;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

extern struct gdt_descriptor gdt[GDT_NUM_ENTRIES];

/*!
    @brief Initializes a Global Descriptor Table (GDT) for a flat memory model.

    Sets up the following descriptors:
    - Null descriptor
    - Kernel code segment
    - Kernel data segment
    - User code segment
    - User data segment

    Each non-null segment covers the full 4 GiB address space with 4 KiB granularity.

    @returns void
*/
void gdt_init(void);

/*!
    @brief Loads a GDT into the CPU and reloads segment registers.

    Tells the CPU where it finds the GDT to load and loads it into the registers.

    @param target GDT to load.
    @returns void
*/
void gdt_install(struct gdt_descriptor *target);

#endif // GDT_H