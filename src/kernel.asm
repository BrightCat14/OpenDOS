section .note.GNU-stack noalloc noexec nowrite progbits
bits 32
section .text

; multi boot header
align 4
dd 0x1BADB002
dd 0x00
dd -(0x1BADB002 + 0x00)

global start
global enable_interrupts

extern k_main
extern load_gdt

start:
    cli               
    mov esp, 0x90000   
    call k_main   
.hang:
    hlt              
    jmp .hang

enable_interrupts:
    sti
    ret
