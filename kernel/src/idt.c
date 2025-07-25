#include "idt.h"

#include "string.h"

struct idt_gate_descriptor idt[IDT_ENTRIES];

/*
    Creates a Gate Descriptor with the given values.

    For information about the descriptors structure visit take a look at:
        https://wiki.osdev.org/Interrupt_Descriptor_Table#Structure_on_x86-64

    @param target Pointer to the IDT descriptor.
    @param offset 64 bit pointer to the ISRs entry point.
    @param segment_selector Must point to a valid code segment in our GDT.
    @param ist Interrupt Stack Table ???
    @param attributes Gate Type and CPU Privilege Levels
*/
void idt_create_entry(struct idt_gate_descriptor *target, uint64_t offset, uint16_t segment_selector, uint8_t ist, uint8_t attributes)
{
    target->offset_low = offset & 0xffff;
    target->offset_mid = (offset >> 16) & 0xffff;
    target->offset_high = (offset >> 32) & 0xffffffff;

    target->segment_selector = segment_selector;

    target->ist = ist & 0x7;

    target->type_attributes = attributes;

    target->reserved = 0;
}

/*
    Initializes a Interrupt Descriptor Table (IDT).

    Right now it creates an empty IDT.
*/
void idt_init(void)
{
    memset(idt, 0, (sizeof(struct idt_gate_descriptor) * IDT_ENTRIES) - 1);

    // Add entries here.
}

/*
    Loads a IDT into the CPU.

    Builds a pointer to the IDT and loads it into the CPU.

    @param target IDT to load.
*/
void idt_install(struct idt_gate_descriptor *target)
{
    struct idt_ptr idtr;
    idtr.size = (sizeof(struct idt_gate_descriptor) * IDT_ENTRIES) - 1;
    idtr.offset = (uint64_t)target;

    asm(
        "LIDT %0"
        :
        : "m"(idtr)
        : "memory");
}