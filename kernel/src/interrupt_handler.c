#include "interrupt_handler.h"

#include "stdio.h"

#include "hcf.h"

/*
    Handles CPU exceptions and interrupts.

    This function is called by the assembly stubs for each interrupt vector.
    - For exceptions without error codes (e.g. invalid opcode),
      prints a predefined error message and halts the system.
    - For exceptions with error codes (e.g. page fault),
      prints the description and the error code and halts.
    - For unknown / unhandled interrupts, prints a generic message 
      including the interrupt vector and error code, then halts.

    @param stack pointer to the interrupt stack frame pushed by the CPU.
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
        printf(interrupt_descriptions[stack->interrupt_vector]);
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
        printf(interrupt_descriptions[stack->interrupt_vector], stack->error_code);
        hcf();
        break;
    // Catches Exceptions / Interrupts that aren't handled.
    default:
        printf(interrupt_descriptions[INT_DESCRIPTION_UNKNOWN_EXCEPTION], stack->interrupt_vector, stack->error_code);
        hcf();
        break;
    }
}