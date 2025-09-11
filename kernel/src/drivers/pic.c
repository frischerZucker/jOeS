#include "drivers/pic.h"

#include "cpu/port_io.h"

// Adresses of the two PICs ports.
#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2 0xa0
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_ICW1_INIT (1 << 4)
#define PIC_ICW1_ICW4_NEEDED 1
#define PIC_ICW1_CASCADE_MODE (0 << 1)

#define PIC_ICW4_MODE_8086 1
#define PIC_ICW4_MODE_8080 0

#define PIC_CASCADE_IRQ 2

#define PIC_EOI (1 << 5)    // End-Of-Interrupt command.

/*!
    @brief Sends an End-Of-Interrupt message to the PICs.

    If the IRQ came from PIC2 (irq > 7), an EOI message is sent to both PIC1 and PIC2.
    Otherwise it is sent only to PIC1.

    @param irq IRQ number.
*/
void pic_send_eoi(uint8_t irq)
{
    if (irq > 7)    // If irq > 7 it came from PIC2, so EOI has to be sent to both PICs.
    {
        port_write_byte(PIC2_COMMAND, PIC_EOI);
    }
    port_write_byte(PIC1_COMMAND, PIC_EOI);
}

/*!
    @brief Enables an IRQ.

    Enables an IRQ by clearing its bit in the Interrupt Mask Register of its PIC.

    @param irq IRQ to enable.
*/
void pic_enable_irq(uint8_t irq)
{
    uint16_t port = PIC1_DATA;

    // If irq > 7 it came from PIC2.
    if (irq > 7)
    {
        irq = irq - 8;      // Maps PIC2s IRQs to its bits (8->0, ..., 15->7).
        port = PIC2_DATA;   // Changes the port to write data to to PIC2.
    }
    
    uint8_t mask = port_read_byte(port) & ~(1 << irq);  // Clear the IRQs bit.
    port_write_byte(port, mask);
}

/*!
    @brief Disables an IRQ.

    Disables an IRQ by setting its bit in the Interrupt Mask Register of its PIC.

    @param irq IRQ to disable.
*/
void pic_disable_irq(uint8_t irq)
{
    uint16_t port = PIC1_DATA;

    // If irq > 7 it came from PIC2.
    if (irq > 7)
    {
        irq = irq - 8;      // Maps PIC2s IRQs to its bits (8->0, ..., 15->7).
        port = PIC2_DATA;   // Changes the port to write data to to PIC2.
    }
    
    uint8_t mask = port_read_byte(port) | (1 << irq);  // Set the IRQs bit.
    port_write_byte(port, mask); 
}

/*!
    @brief Initializes both PICs.

    Initializes both PICs in cascade mode with PIC2 at IRQ2 of PIC1.
    Also remaps their IRQs to the given offsets.

    @param offset_pic1 Offset to which the IRQs from PIC1 are remapped.
    @param offset_pic2 Offset to which the IRQs from PIC2 are remapped.
*/
void pic_init(int offset_pic1, int offset_pic2)
{
    // Start initialization routine. Tell both PICs that they are used in cascade mode and ICW4 will be present.
    port_write_byte(PIC1_COMMAND, PIC_ICW1_INIT | PIC_ICW1_CASCADE_MODE | PIC_ICW1_ICW4_NEEDED);
    port_write_byte(PIC2_COMMAND, PIC_ICW1_INIT | PIC_ICW1_CASCADE_MODE | PIC_ICW1_ICW4_NEEDED);

    // Tell them which interrupt vectors they should use.
    port_write_byte(PIC1_DATA, offset_pic1);
    port_write_byte(PIC2_DATA, offset_pic2);

    // Cascade at IRQ2.
    port_write_byte(PIC1_DATA, 1 << PIC_CASCADE_IRQ);
    port_write_byte(PIC2_DATA, PIC_CASCADE_IRQ);

    // Choose 8086 mode.
    port_write_byte(PIC1_DATA, PIC_ICW4_MODE_8086);
    port_write_byte(PIC1_DATA, PIC_ICW4_MODE_8086);

    // Mask all interrupts
    port_write_byte(PIC1_DATA, 255);
    port_write_byte(PIC2_DATA, 255);
}