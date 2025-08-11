; Interrupt Service Routines
;
; Defines the Interrupt Service Routines that are placed into the IDT.

extern interrupt_handler

; Macro for interrupts that push no error code on their own.
%macro isr_no_err 1
global _isr%+%1
_isr%+%1:
    push qword %1   ; No error code was pushed by the interrupt, so we push one ourself.
    push qword %1   ; Interrupt vector.
    jmp isr_common  ; Go to the part both type of ISRs share using jmp, so no return address is pushed to the stack.
%endmacro

; Macro for interrupts that push an error code.
%macro isr_err 1
global _isr%+%1
_isr%+%1:
    ; An Error code was automaticaly pushed, so we don't need to do it.
    push qword %1   ; Interrupt vector.
    jmp isr_common  ; Go to the part both type of ISRs share using jmp, so no return address is pushed to the stack.
%endmacro

isr_no_err 0x00 ; Divide Error.
isr_no_err 0x01 ; Debug Exception.
isr_no_err 0x02 ; NMI Interrupt.
isr_no_err 0x03 ; Breakpoint.
isr_no_err 0x04 ; Overflow.
isr_no_err 0x05 ; BOUND Range Exceeded.
isr_no_err 0x06 ; Invalid Opcode (Undefined Opcode).
isr_no_err 0x07 ; Device Not Available (No Math Coprocessor).
isr_err 0x08    ; Double Fault.
isr_no_err 0x09 ; Coprocessor Segment Overrun (reserved).
isr_err 0x0a    ; Invalid TSS.
isr_err 0x0b    ; Segment Not Present.
isr_err 0x0c    ; Stack-Segment Fault.
isr_err 0x0d    ; General Protection.
isr_err 0x0e    ; Page Fault.
isr_no_err 0x0f ; Reserved. Don't use this.
isr_no_err 0x10 ; x87 Floating-Point Error (Math Fault).
isr_err 0x11    ; Alignment Check.
isr_no_err 0x12 ; Machine Check.
isr_no_err 0x13 ; SIMD Floating-Point Exception.
isr_no_err 0x14 ; Virtualization Exception.
isr_err 0x15    ; Control Protection Exception.
; Reserved for futute use as CPU exception vectors.
isr_no_err 0x16
isr_no_err 0x17
isr_no_err 0x18
isr_no_err 0x19
isr_no_err 0x1a
isr_no_err 0x1b
isr_no_err 0x1c
isr_no_err 0x1d
isr_no_err 0x1e
isr_no_err 0x1f
; External Interrupts.
isr_no_err 0x20
isr_no_err 0x21
isr_no_err 0x22
isr_no_err 0x23
isr_no_err 0x24
isr_no_err 0x25
isr_no_err 0x26
isr_no_err 0x27
isr_no_err 0x28
isr_no_err 0x29
isr_no_err 0x2a
isr_no_err 0x2b
isr_no_err 0x2c
isr_no_err 0x2d
isr_no_err 0x2e
isr_no_err 0x2f
isr_no_err 0x30
isr_no_err 0x31
isr_no_err 0x32
isr_no_err 0x33
isr_no_err 0x34
isr_no_err 0x35
isr_no_err 0x36
isr_no_err 0x37
isr_no_err 0x38
isr_no_err 0x39
isr_no_err 0x3a
isr_no_err 0x3b
isr_no_err 0x3c
isr_no_err 0x3d
isr_no_err 0x3e
isr_no_err 0x3f
isr_no_err 0x40
isr_no_err 0x41
isr_no_err 0x42
isr_no_err 0x43
isr_no_err 0x44
isr_no_err 0x45
isr_no_err 0x46
isr_no_err 0x47
isr_no_err 0x48
isr_no_err 0x49
isr_no_err 0x4a
isr_no_err 0x4b
isr_no_err 0x4c
isr_no_err 0x4d
isr_no_err 0x4e
isr_no_err 0x4f
isr_no_err 0x50
isr_no_err 0x51
isr_no_err 0x52
isr_no_err 0x53
isr_no_err 0x54
isr_no_err 0x55
isr_no_err 0x56
isr_no_err 0x57
isr_no_err 0x58
isr_no_err 0x59
isr_no_err 0x5a
isr_no_err 0x5b
isr_no_err 0x5c
isr_no_err 0x5d
isr_no_err 0x5e
isr_no_err 0x5f
isr_no_err 0x60
isr_no_err 0x61
isr_no_err 0x62
isr_no_err 0x63
isr_no_err 0x64
isr_no_err 0x65
isr_no_err 0x66
isr_no_err 0x67
isr_no_err 0x68
isr_no_err 0x69
isr_no_err 0x6a
isr_no_err 0x6b
isr_no_err 0x6c
isr_no_err 0x6d
isr_no_err 0x6e
isr_no_err 0x6f
isr_no_err 0x70
isr_no_err 0x71
isr_no_err 0x72
isr_no_err 0x73
isr_no_err 0x74
isr_no_err 0x75
isr_no_err 0x76
isr_no_err 0x77
isr_no_err 0x78
isr_no_err 0x79
isr_no_err 0x7a
isr_no_err 0x7b
isr_no_err 0x7c
isr_no_err 0x7d
isr_no_err 0x7e
isr_no_err 0x7f
isr_no_err 0x80
isr_no_err 0x81
isr_no_err 0x82
isr_no_err 0x83
isr_no_err 0x84
isr_no_err 0x85
isr_no_err 0x86
isr_no_err 0x87
isr_no_err 0x88
isr_no_err 0x89
isr_no_err 0x8a
isr_no_err 0x8b
isr_no_err 0x8c
isr_no_err 0x8d
isr_no_err 0x8e
isr_no_err 0x8f
isr_no_err 0x90
isr_no_err 0x91
isr_no_err 0x92
isr_no_err 0x93
isr_no_err 0x94
isr_no_err 0x95
isr_no_err 0x96
isr_no_err 0x97
isr_no_err 0x98
isr_no_err 0x99
isr_no_err 0x9a
isr_no_err 0x9b
isr_no_err 0x9c
isr_no_err 0x9d
isr_no_err 0x9e
isr_no_err 0x9f
isr_no_err 0xa0
isr_no_err 0xa1
isr_no_err 0xa2
isr_no_err 0xa3
isr_no_err 0xa4
isr_no_err 0xa5
isr_no_err 0xa6
isr_no_err 0xa7
isr_no_err 0xa8
isr_no_err 0xa9
isr_no_err 0xaa
isr_no_err 0xab
isr_no_err 0xac
isr_no_err 0xad
isr_no_err 0xae
isr_no_err 0xaf
isr_no_err 0xb0
isr_no_err 0xb1
isr_no_err 0xb2
isr_no_err 0xb3
isr_no_err 0xb4
isr_no_err 0xb5
isr_no_err 0xb6
isr_no_err 0xb7
isr_no_err 0xb8
isr_no_err 0xb9
isr_no_err 0xba
isr_no_err 0xbb
isr_no_err 0xbc
isr_no_err 0xbd
isr_no_err 0xbe
isr_no_err 0xbf
isr_no_err 0xc0
isr_no_err 0xc1
isr_no_err 0xc2
isr_no_err 0xc3
isr_no_err 0xc4
isr_no_err 0xc5
isr_no_err 0xc6
isr_no_err 0xc7
isr_no_err 0xc8
isr_no_err 0xc9
isr_no_err 0xca
isr_no_err 0xcb
isr_no_err 0xcc
isr_no_err 0xcd
isr_no_err 0xce
isr_no_err 0xcf
isr_no_err 0xd0
isr_no_err 0xd1
isr_no_err 0xd2
isr_no_err 0xd3
isr_no_err 0xd4
isr_no_err 0xd5
isr_no_err 0xd6
isr_no_err 0xd7
isr_no_err 0xd8
isr_no_err 0xd9
isr_no_err 0xda
isr_no_err 0xdb
isr_no_err 0xdc
isr_no_err 0xdd
isr_no_err 0xde
isr_no_err 0xdf
isr_no_err 0xe0
isr_no_err 0xe1
isr_no_err 0xe2
isr_no_err 0xe3
isr_no_err 0xe4
isr_no_err 0xe5
isr_no_err 0xe6
isr_no_err 0xe7
isr_no_err 0xe8
isr_no_err 0xe9
isr_no_err 0xea
isr_no_err 0xeb
isr_no_err 0xec
isr_no_err 0xed
isr_no_err 0xee
isr_no_err 0xef
isr_no_err 0xf0
isr_no_err 0xf1
isr_no_err 0xf2
isr_no_err 0xf3
isr_no_err 0xf4
isr_no_err 0xf5
isr_no_err 0xf6
isr_no_err 0xf7
isr_no_err 0xf8
isr_no_err 0xf9
isr_no_err 0xfa
isr_no_err 0xfb
isr_no_err 0xfc
isr_no_err 0xfd
isr_no_err 0xfe
isr_no_err 0xff


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
    ; push rsp
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
    ; pop rsp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    add rsp, 8            ; Remove error code and interrupt vector from the stack.

    iretq                   ; Do a 64-bit interrupt return.