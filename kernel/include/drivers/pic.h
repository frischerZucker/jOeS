#ifndef PIC_H
#define PIC_H

#include <stdint.h>

// Adresses of the two PICs ports.
#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2 0xa0
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

void pic_init(int offset_pic1, int offset_pic2);
void pic_disable();

void pic_enable_irq(uint8_t irq);
void pic_disable_irq(uint8_t irq);

void pic_send_eoi(uint8_t irq);

#endif // PIC_H