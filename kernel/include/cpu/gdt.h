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

void gdt_create_segment_descriptor(struct gdt_descriptor *target, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

void gdt_init(void);

void gdt_install(struct gdt_descriptor *target);

#endif // GDT_H