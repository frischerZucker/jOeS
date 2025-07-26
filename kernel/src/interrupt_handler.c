#include "interrupt_handler.h"

#include "stdio.h"

void interrupt_handler(struct interrupt_stack_frame *stack)
{
    printf("ERROR: DIV0\n", 12);

    for(;;);
}