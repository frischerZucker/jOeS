/*!
    @file idt.h

    @brief Interrupt Descriptor Table (IDT) setup for x86_64 systems.
    
    Defines structures and functions for initializing and installing the IDT, which maps hardware and software interrupts to handler routines.
    Assumes a 256-entry table with support for both interrupt and trap gates.
    Includes macros for gate type and attribute configuration.

    @author frischerZucker
 */

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256

#define IDT_GATE_TYPE_INT_GATE 0xe
#define IDT_GATE_TYPE_TRAP_GATE 0xf

#define IDT_ATTRIBUTES(dpl, gate_type) (0x80 | ((dpl & 0x3) << 5) | (gate_type & 0xf))

struct idt_gate_descriptor
{
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

extern struct idt_gate_descriptor idt[IDT_ENTRIES];

/*!
    @brief Initializes an Interrupt Descriptor Table (IDT).

    Initialized an Interrupt Descriptor Table (IDT) with 256 interrupt gates pointing to the assembly stubs for interrupt handling.
*/
void idt_init(void);

/*!
    @brief Loads an IDT into the CPU.

    Builds a pointer to the IDT and loads it into the CPU.

    @param target IDT to load.
*/
void idt_install(struct idt_gate_descriptor *target);

#endif // IDT_H