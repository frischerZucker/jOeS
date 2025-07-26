; Interrupt Service Routines
;
; Defines the Interrupt Service Routines that are placed into the IDT.

extern interrupt_handler

global _isr0

; Division by zero.
_isr0:
    cli             ; Disable interrupts.
    push $0         ; Interrupt 0 has no error code, so we push a dummy error code.
    push $0         ; Push the interrupt vector (id of the interrupt).
    call isr_common ; Call the rest of our ISR.
    
; Part that is needed for every ISR.
;
; Saves the current CPU state and calls our C interrupt handler, passing it a pointer to the CPU's status on the stack.
;
; Call this from _isr* after pushing error code and interrupt vector.
isr_common:
    push rax                ; Save registers.
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rsp
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp            ; Move the stack pointer to the register that's used for the first parameter of C functions.
    call interrupt_handler  ; Call our C interrupt handler.

    pop r15                 ; Restore registers.
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rsp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    add rsp, $16            ; Remove error code and interrupt vector from the stack.
    
    sti                     ; Reenable interrupts.
    iretq                   ; Do a 64-bit interrupt return.