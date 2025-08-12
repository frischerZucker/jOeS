#include "interrupt_handler.h"

#include "stdio.h"

#include "hcf.h"

void interrupt_handler(struct interrupt_stack_frame *stack)
{
    switch (stack->interrupt_vector)
    {
    case 0:
        printf("Divide Error %d %d\n", stack->interrupt_vector, stack->error_code);
        hcf();
        break;
    
    case 3:
        printf("Breakpoint %d %d\n", stack->interrupt_vector, stack->error_code);
        hcf();
        break;
    
    default:
        printf("An unknown Exception / Interrupt occured: int=%d, errno=%d\n", stack->interrupt_vector, stack->error_code);
        hcf();
        break;
    }
}