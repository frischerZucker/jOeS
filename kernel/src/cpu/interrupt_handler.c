#include "cpu/interrupt_handler.h"

#include "stdio.h"

#include "cpu/hcf.h"
#include "drivers/keyboard.h"
#include "drivers/pic.h"
#include "drivers/ps2_keyboard.h"
#include "logging.h"

/*!
    @brief Handles CPU exceptions and interrupts.

    This function is called by the assembly stubs for each interrupt vector.
    - For exceptions without error codes (e.g. invalid opcode),
      prints a predefined error message and halts the system.
    - For exceptions with error codes (e.g. page fault),
      prints the description and the error code and halts.
    - For external interrupts 0 to 15 (from the PIC), prints the IRQs number
      and sends an End-Of-Interrupt command.
    - For unknown / unhandled interrupts, prints a generic message 
      including the interrupt vector and error code, then halts.

    @param stack Pointer to the interrupt stack frame pushed by the CPU.
*/
void interrupt_handler(struct interrupt_stack_frame *stack)
{
    switch (stack->interrupt_vector)
    {
    // Exceptions that don't have an error code.
    case INT_DIVIDE_ERR:
    case INT_DEBUG_EXCEPTION:
    case INT_NMI_INT:
    case INT_BREAKPOINT:
    case INT_OVERFLOW:
    case INT_BOUND_RANGE_EXCEEDED:
    case INT_INVALID_OPCODE:
    case INT_NO_MATH_COPROCESSOR:
    case INT_COPROCESSOR_SEG_OVERRUN:
    case INT_RESERVED0:
    case INT_FPU_FLOATING_POINT_ERR:
    case INT_MACHINE_CHECK:
    case INT_SIMD_FLOATING_POINT_EXCEPTION:
    case INT_VIRTUALIZATION_EXCEPTION:
    case INT_RESERVED1:
    case INT_RESERVED2:
    case INT_RESERVED3:
    case INT_RESERVED4:
    case INT_RESERVED5:
    case INT_RESERVED6:
    case INT_RESERVED7:
    case INT_RESERVED8:
    case INT_RESERVED9:
    case INT_RESERVED10:
        LOG_ERROR(interrupt_descriptions[stack->interrupt_vector]);
        hcf();
        break;
    // Exceptions that have an error code.
    case INT_DOUBLE_FAULT:
    case INT_INVALID_TSS:
    case INT_SEGMENT_NOT_PRESENT:
    case INT_STACK_SEGMENT_FAULT:
    case INT_GENERAL_PROTECTION_FAULT:
    case INT_PAGE_FAULT:
    case INT_ALIGNMENT_CHECK:
    case INT_CONTROL_PROTECTION_EXCEPTION:
        LOG_ERROR(interrupt_descriptions[stack->interrupt_vector], stack->error_code);
        hcf();
        break;
    // IRQs from the PIC.
    case INT_EXT_INT0: // PIT
        /*
            Retrieve key events and print corresponding ASCII characters if possible.
        */
        struct key_event_t key_event;
        while (kbd_get_key_event_from_buffer(&key_event) != KBD_ERROR_KEY_EVENT_BUFFER_EMPTY)
        {
            printf("%s", kbd_key_event_to_ascii(&key_event));
        }
        pic_send_eoi(0);
        break;
    case INT_EXT_INT1:
        ps2_kbd_irq_callback();
        pic_send_eoi(1);
        break;
    case INT_EXT_INT2:
        LOG_DEBUG("EXT2\n");
        pic_send_eoi(2);
        break;
    case INT_EXT_INT3:
        LOG_DEBUG("EXT3\n");
        pic_send_eoi(3);
        break;
    case INT_EXT_INT4:
        LOG_DEBUG("EXT4\n");
        pic_send_eoi(4);
        break;
    case INT_EXT_INT5:
        LOG_DEBUG("EXT5\n");
        pic_send_eoi(5);
        break;
    case INT_EXT_INT6:
        LOG_DEBUG("EXT6\n");
        pic_send_eoi(6);
        break;
    case INT_EXT_INT7:
        LOG_DEBUG("EXT7\n");
        pic_send_eoi(7);
        break;
    case INT_EXT_INT8:
        LOG_DEBUG("EXT8\n");
        pic_send_eoi(8);
        break;
    case INT_EXT_INT9:
        LOG_DEBUG("EXT9\n");
        pic_send_eoi(9);
        break;
    case INT_EXT_INT10:
        LOG_DEBUG("EXT10\n");
        pic_send_eoi(10);
        break;
    case INT_EXT_INT11:
        LOG_DEBUG("EXT11\n");
        pic_send_eoi(11);
        break;
    case INT_EXT_INT12:
        LOG_DEBUG("EXT12\n");
        pic_send_eoi(12);
        break;
    case INT_EXT_INT13:
        LOG_DEBUG("EXT13\n");
        pic_send_eoi(13);
        break;
    case INT_EXT_INT14:
        LOG_DEBUG("EXT14\n");
        pic_send_eoi(14);
        break;
    case INT_EXT_INT15:
        LOG_DEBUG("EXT15\n");
        pic_send_eoi(15);
        break;

    // Catches Exceptions / Interrupts that aren't handled.
    default:
        LOG_ERROR(interrupt_descriptions[INT_DESCRIPTION_UNKNOWN_EXCEPTION], stack->interrupt_vector, stack->error_code);
        hcf();
        break;
    }
}