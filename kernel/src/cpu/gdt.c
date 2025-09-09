#include <gdt.h>

struct gdt_descriptor gdt[GDT_NUM_ENTRIES];

extern void gdt_load_segments(void);

/*
    Creates a Segment Descriptor entry with the given values.

    For information about the descriptors structure visit take a look at:
        https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor

    @param target Pointer to the GDT descriptor.
    @param base 32-bit base adress of the segment.
    @param limit 20-bit limit of the segment.
    @param access Access byte (present, descriptor privilege level, type).
    @param flags Flags including granularity and operand-size bits.
    @returns void
*/
void gdt_create_segment_descriptor(struct gdt_descriptor *target, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    target->base_low = base & 0xFFFF;
    target->base_mid = (base >> 16) & 0xFF;
    target->base_high = (base >> 24) & 0xFF;

    target->limit_low = limit & 0xFFFF;
    target->limit_high_flags = (limit >> 16) & 0x0F;

    target->access = access;

    target->limit_high_flags = target->limit_high_flags | ((flags & 0xF) << 4);
}

/*
    Initializes a Global Descriptor Table (GDT) for a flat memory model.

    Sets up the following descriptors:
    - Null descriptor
    - Kernel code segment
    - Kernel data segment
    - User code segment
    - User data segment

    Each non-null segment covers the full 4 GiB address space with 4 KiB granularity.

    @returns void
*/
void gdt_init(void)
{
    // Null Descriptor
    gdt_create_segment_descriptor(&gdt[0], 0x0, 0x0, 0x0, 0x0);
    // Kernel Mode Code Segment
    gdt_create_segment_descriptor(&gdt[1], 0x0, 0xfffff, 0x9a, 0xa);
    // Kernel Mode Data Segment
    gdt_create_segment_descriptor(&gdt[2], 0x0, 0xfffff, 0x92, 0xc);
    // User Mode Code Segment
    gdt_create_segment_descriptor(&gdt[3], 0x0, 0xfffff, 0xfa, 0xa);
    // User Mode Data Segment
    gdt_create_segment_descriptor(&gdt[4], 0x0, 0xfffff, 0xf2, 0xc);
}

/*
    Loads a GDT into the CPU and reloads segment registers.

    Tells the CPU where it finds the GDT to load and loads it into the registers.

    @param target GDT to load.
    @returns void
*/
void gdt_install(struct gdt_descriptor *target)
{
    struct gdt_ptr gdtr;
    // Size of the GDT -1 (in bytes)
    gdtr.limit = (sizeof(struct gdt_descriptor) * GDT_NUM_ENTRIES) - 1;
    // Pointer to the GDT
    gdtr.base = (uint64_t)target;

    // Load the gdtr register. -> Tells the CPU where our GDT is.
    asm(
        "LGDT %0"
        :
        : "m"(gdtr)
        : "memory");

    // Reload all segment registers.
    gdt_load_segments();
}