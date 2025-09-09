; Reloads the segment registers.
;
; Call this after loading the GDT with the LGDT instruction.
;
; no parameters, no return value

global gdt_load_segments        ; make gdt_load_segments visible in other files

gdt_load_segments:
    mov ax, 0x10                ; data segment seletor offset
    mov ds, ax                  ; reload DS
    mov es, ax                  ; reload ES
    mov fs, ax                  ; reload FS
    mov gs, ax                  ; reload GS
    mov ss, ax                  ; reload SS
    
    push 0x08                   ; push code segment selector offset
    lea rax, [rel .reload_cs]   ; load adress of .reload_cs into RAX
    push rax                    ; push return RIP for RETFQ
    retfq                       ; far return: 

.reload_cs:
    ret                         ; return to where gdt_load_segments was called