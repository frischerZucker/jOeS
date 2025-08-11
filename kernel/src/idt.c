#include "idt.h"

#include "string.h"

struct idt_gate_descriptor idt[IDT_ENTRIES];

extern void _isr0x00(), _isr0x01(), _isr0x02(), _isr0x03(), _isr0x04(), _isr0x05(), _isr0x06(), _isr0x07(), _isr0x08(), _isr0x09(), _isr0x0a(), _isr0x0b(), _isr0x0c(), _isr0x0d(), _isr0x0e(), _isr0x0f(), _isr0x10(), _isr0x11(), _isr0x12(), _isr0x13(), _isr0x14(), _isr0x15(), _isr0x16(), _isr0x17(), _isr0x18(), _isr0x19(), _isr0x1a(), _isr0x1b(), _isr0x1c(), _isr0x1d(), _isr0x1e(), _isr0x1f(), _isr0x20(), _isr0x21(), _isr0x22(), _isr0x23(), _isr0x24(), _isr0x25(), _isr0x26(), _isr0x27(), _isr0x28(), _isr0x29(), _isr0x2a(), _isr0x2b(), _isr0x2c(), _isr0x2d(), _isr0x2e(), _isr0x2f(), _isr0x30(), _isr0x31(), _isr0x32(), _isr0x33(), _isr0x34(), _isr0x35(), _isr0x36(), _isr0x37(), _isr0x38(), _isr0x39(), _isr0x3a(), _isr0x3b(), _isr0x3c(), _isr0x3d(), _isr0x3e(), _isr0x3f(), _isr0x40(), _isr0x41(), _isr0x42(), _isr0x43(), _isr0x44(), _isr0x45(), _isr0x46(), _isr0x47(), _isr0x48(), _isr0x49(), _isr0x4a(), _isr0x4b(), _isr0x4c(), _isr0x4d(), _isr0x4e(), _isr0x4f(), _isr0x50(), _isr0x51(), _isr0x52(), _isr0x53(), _isr0x54(), _isr0x55(), _isr0x56(), _isr0x57(), _isr0x58(), _isr0x59(), _isr0x5a(), _isr0x5b(), _isr0x5c(), _isr0x5d(), _isr0x5e(), _isr0x5f(), _isr0x60(), _isr0x61(), _isr0x62(), _isr0x63(), _isr0x64(), _isr0x65(), _isr0x66(), _isr0x67(), _isr0x68(), _isr0x69(), _isr0x6a(), _isr0x6b(), _isr0x6c(), _isr0x6d(), _isr0x6e(), _isr0x6f(), _isr0x70(), _isr0x71(), _isr0x72(), _isr0x73(), _isr0x74(), _isr0x75(), _isr0x76(), _isr0x77(), _isr0x78(), _isr0x79(), _isr0x7a(), _isr0x7b(), _isr0x7c(), _isr0x7d(), _isr0x7e(), _isr0x7f(), _isr0x80(), _isr0x81(), _isr0x82(), _isr0x83(), _isr0x84(), _isr0x85(), _isr0x86(), _isr0x87(), _isr0x88(), _isr0x89(), _isr0x8a(), _isr0x8b(), _isr0x8c(), _isr0x8d(), _isr0x8e(), _isr0x8f(), _isr0x90(), _isr0x91(), _isr0x92(), _isr0x93(), _isr0x94(), _isr0x95(), _isr0x96(), _isr0x97(), _isr0x98(), _isr0x99(), _isr0x9a(), _isr0x9b(), _isr0x9c(), _isr0x9d(), _isr0x9e(), _isr0x9f(), _isr0xa0(), _isr0xa1(), _isr0xa2(), _isr0xa3(), _isr0xa4(), _isr0xa5(), _isr0xa6(), _isr0xa7(), _isr0xa8(), _isr0xa9(), _isr0xaa(), _isr0xab(), _isr0xac(), _isr0xad(), _isr0xae(), _isr0xaf(), _isr0xb0(), _isr0xb1(), _isr0xb2(), _isr0xb3(), _isr0xb4(), _isr0xb5(), _isr0xb6(), _isr0xb7(), _isr0xb8(), _isr0xb9(), _isr0xba(), _isr0xbb(), _isr0xbc(), _isr0xbd(), _isr0xbe(), _isr0xbf(), _isr0xc0(), _isr0xc1(), _isr0xc2(), _isr0xc3(), _isr0xc4(), _isr0xc5(), _isr0xc6(), _isr0xc7(), _isr0xc8(), _isr0xc9(), _isr0xca(), _isr0xcb(), _isr0xcc(), _isr0xcd(), _isr0xce(), _isr0xcf(), _isr0xd0(), _isr0xd1(), _isr0xd2(), _isr0xd3(), _isr0xd4(), _isr0xd5(), _isr0xd6(), _isr0xd7(), _isr0xd8(), _isr0xd9(), _isr0xda(), _isr0xdb(), _isr0xdc(), _isr0xdd(), _isr0xde(), _isr0xdf(), _isr0xe0(), _isr0xe1(), _isr0xe2(), _isr0xe3(), _isr0xe4(), _isr0xe5(), _isr0xe6(), _isr0xe7(), _isr0xe8(), _isr0xe9(), _isr0xea(), _isr0xeb(), _isr0xec(), _isr0xed(), _isr0xee(), _isr0xef(), _isr0xf0(), _isr0xf1(), _isr0xf2(), _isr0xf3(), _isr0xf4(), _isr0xf5(), _isr0xf6(), _isr0xf7(), _isr0xf8(), _isr0xf9(), _isr0xfa(), _isr0xfb(), _isr0xfc(), _isr0xfd(), _isr0xfe(), _isr0xff();

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

    // target->type_attributes = attributes;

    target->type_attributes = 0b1110 | ((attributes & 0b11) << 5) |(1 << 7);

    target->reserved = 0;
}

/*
    Initializes a Interrupt Descriptor Table (IDT).

    Right now it creates an empty IDT.
*/
void idt_init(void)
{
    memset(idt, 0, (sizeof(struct idt_gate_descriptor) * IDT_ENTRIES));

    // Add entries here.
    // for (size_t i = 0; i < IDT_ENTRIES; i++)
    // {
    //     idt_create_entry(&idt[i], _isr0, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    // }
    // idt_create_entry(&idt[3], _isr3, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[0], _isr0x00, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[1], _isr0x01, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[2], _isr0x02, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[3], _isr0x03, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[4], _isr0x04, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[5], _isr0x05, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[6], _isr0x06, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[7], _isr0x07, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[8], _isr0x08, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[9], _isr0x09, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[10], _isr0x0a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[11], _isr0x0b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[12], _isr0x0c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[13], _isr0x0d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[14], _isr0x0e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[15], _isr0x0f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[16], _isr0x10, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[17], _isr0x11, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[18], _isr0x12, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[19], _isr0x13, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[20], _isr0x14, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[21], _isr0x15, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[22], _isr0x16, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[23], _isr0x17, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[24], _isr0x18, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[25], _isr0x19, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[26], _isr0x1a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[27], _isr0x1b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[28], _isr0x1c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[29], _isr0x1d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[30], _isr0x1e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[31], _isr0x1f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[32], _isr0x20, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[33], _isr0x21, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[34], _isr0x22, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[35], _isr0x23, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[36], _isr0x24, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[37], _isr0x25, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[38], _isr0x26, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[39], _isr0x27, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[40], _isr0x28, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[41], _isr0x29, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[42], _isr0x2a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[43], _isr0x2b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[44], _isr0x2c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[45], _isr0x2d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[46], _isr0x2e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[47], _isr0x2f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[48], _isr0x30, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[49], _isr0x31, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[50], _isr0x32, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[51], _isr0x33, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[52], _isr0x34, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[53], _isr0x35, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[54], _isr0x36, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[55], _isr0x37, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[56], _isr0x38, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[57], _isr0x39, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[58], _isr0x3a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[59], _isr0x3b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[60], _isr0x3c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[61], _isr0x3d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[62], _isr0x3e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[63], _isr0x3f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[64], _isr0x40, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[65], _isr0x41, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[66], _isr0x42, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[67], _isr0x43, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[68], _isr0x44, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[69], _isr0x45, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[70], _isr0x46, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[71], _isr0x47, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[72], _isr0x48, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[73], _isr0x49, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[74], _isr0x4a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[75], _isr0x4b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[76], _isr0x4c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[77], _isr0x4d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[78], _isr0x4e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[79], _isr0x4f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[80], _isr0x50, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[81], _isr0x51, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[82], _isr0x52, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[83], _isr0x53, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[84], _isr0x54, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[85], _isr0x55, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[86], _isr0x56, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[87], _isr0x57, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[88], _isr0x58, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[89], _isr0x59, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[90], _isr0x5a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[91], _isr0x5b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[92], _isr0x5c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[93], _isr0x5d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[94], _isr0x5e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[95], _isr0x5f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[96], _isr0x60, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[97], _isr0x61, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[98], _isr0x62, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[99], _isr0x63, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[100], _isr0x64, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[101], _isr0x65, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[102], _isr0x66, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[103], _isr0x67, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[104], _isr0x68, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[105], _isr0x69, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[106], _isr0x6a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[107], _isr0x6b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[108], _isr0x6c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[109], _isr0x6d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[110], _isr0x6e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[111], _isr0x6f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[112], _isr0x70, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[113], _isr0x71, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[114], _isr0x72, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[115], _isr0x73, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[116], _isr0x74, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[117], _isr0x75, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[118], _isr0x76, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[119], _isr0x77, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[120], _isr0x78, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[121], _isr0x79, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[122], _isr0x7a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[123], _isr0x7b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[124], _isr0x7c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[125], _isr0x7d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[126], _isr0x7e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[127], _isr0x7f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[128], _isr0x80, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[129], _isr0x81, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[130], _isr0x82, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[131], _isr0x83, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[132], _isr0x84, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[133], _isr0x85, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[134], _isr0x86, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[135], _isr0x87, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[136], _isr0x88, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[137], _isr0x89, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[138], _isr0x8a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[139], _isr0x8b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[140], _isr0x8c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[141], _isr0x8d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[142], _isr0x8e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[143], _isr0x8f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[144], _isr0x90, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[145], _isr0x91, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[146], _isr0x92, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[147], _isr0x93, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[148], _isr0x94, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[149], _isr0x95, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[150], _isr0x96, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[151], _isr0x97, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[152], _isr0x98, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[153], _isr0x99, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[154], _isr0x9a, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[155], _isr0x9b, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[156], _isr0x9c, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[157], _isr0x9d, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[158], _isr0x9e, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[159], _isr0x9f, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[160], _isr0xa0, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[161], _isr0xa1, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[162], _isr0xa2, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[163], _isr0xa3, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[164], _isr0xa4, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[165], _isr0xa5, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[166], _isr0xa6, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[167], _isr0xa7, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[168], _isr0xa8, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[169], _isr0xa9, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[170], _isr0xaa, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[171], _isr0xab, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[172], _isr0xac, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[173], _isr0xad, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[174], _isr0xae, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[175], _isr0xaf, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[176], _isr0xb0, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[177], _isr0xb1, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[178], _isr0xb2, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[179], _isr0xb3, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[180], _isr0xb4, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[181], _isr0xb5, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[182], _isr0xb6, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[183], _isr0xb7, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[184], _isr0xb8, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[185], _isr0xb9, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[186], _isr0xba, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[187], _isr0xbb, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[188], _isr0xbc, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[189], _isr0xbd, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[190], _isr0xbe, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[191], _isr0xbf, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[192], _isr0xc0, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[193], _isr0xc1, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[194], _isr0xc2, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[195], _isr0xc3, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[196], _isr0xc4, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[197], _isr0xc5, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[198], _isr0xc6, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[199], _isr0xc7, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[200], _isr0xc8, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[201], _isr0xc9, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[202], _isr0xca, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[203], _isr0xcb, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[204], _isr0xcc, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[205], _isr0xcd, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[206], _isr0xce, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[207], _isr0xcf, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[208], _isr0xd0, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[209], _isr0xd1, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[210], _isr0xd2, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[211], _isr0xd3, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[212], _isr0xd4, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[213], _isr0xd5, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[214], _isr0xd6, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[215], _isr0xd7, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[216], _isr0xd8, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[217], _isr0xd9, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[218], _isr0xda, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[219], _isr0xdb, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[220], _isr0xdc, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[221], _isr0xdd, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[222], _isr0xde, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[223], _isr0xdf, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[224], _isr0xe0, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[225], _isr0xe1, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[226], _isr0xe2, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[227], _isr0xe3, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[228], _isr0xe4, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[229], _isr0xe5, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[230], _isr0xe6, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[231], _isr0xe7, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[232], _isr0xe8, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[233], _isr0xe9, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[234], _isr0xea, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[235], _isr0xeb, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[236], _isr0xec, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[237], _isr0xed, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[238], _isr0xee, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[239], _isr0xef, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[240], _isr0xf0, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[241], _isr0xf1, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[242], _isr0xf2, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[243], _isr0xf3, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[244], _isr0xf4, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[245], _isr0xf5, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[246], _isr0xf6, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[247], _isr0xf7, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[248], _isr0xf8, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[249], _isr0xf9, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[250], _isr0xfa, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[251], _isr0xfb, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[252], _isr0xfc, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[253], _isr0xfd, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[254], _isr0xfe, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
    idt_create_entry(&idt[255], _isr0xff, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));
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
        "lidt %0"
        :
        : "m"(idtr)
        : "memory");
    asm("cli");
}