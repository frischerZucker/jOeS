/*!
    @file pic.h

    @brief Driver for the 8259 Programmable Interrupt Controller (PIC).
    
    Provides functions to initialize and control the 8259 PIC.
    Assumes the standard x86 cascading setup, where PIC2 is connected to IRQ2 of PIC1.
    Supports IRQ remapping, masking/unmasking of interrupts, and sending End-Of-Interrupt (EOI) signals.
    
    @author frischerZucker
 */

#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/*!
    @brief Initializes both PICs.

    Initializes both PICs in cascade mode with PIC2 at IRQ2 of PIC1.
    Also remaps their IRQs to the given offsets.

    @param offset_pic1 Offset to which the IRQs from PIC1 are remapped.
    @param offset_pic2 Offset to which the IRQs from PIC2 are remapped.
*/
void pic_init(int offset_pic1, int offset_pic2);

/*!
    @brief Disable both PICs.
    
    @warning This function is not yet implemented.
*/
void pic_disable();

/*!
    @brief Enables an IRQ.

    Enables an IRQ by clearing its bit in the Interrupt Mask Register of its PIC.

    @param irq IRQ to enable.
*/
void pic_enable_irq(uint8_t irq);

/*!
    @brief Disables an IRQ.

    Disables an IRQ by setting its bit in the Interrupt Mask Register of its PIC.

    @param irq IRQ to disable.
*/
void pic_disable_irq(uint8_t irq);

/*!
    @brief Sends an End-Of-Interrupt message to the PICs.

    If the IRQ came from PIC2 (irq > 7), an EOI message is sent to both PIC1 and PIC2.
    Otherwise it is sent only to PIC1.

    @param irq IRQ number.
*/
void pic_send_eoi(uint8_t irq);

#endif // PIC_H