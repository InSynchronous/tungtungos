bits 32

global enter_user_mode
extern program_start

USER_CS equ 0x1B
USER_DS equ 0x23
TSS_SEL equ 0x28
TSS equ 0x5000

section .bss

alignb 4096
kernel_stack:
    resb 4096

kernel_stack_top:

alignb 4096
user_stack:
    resb 4096

user_stack_top:


section .text

enter_user_mode:
    cli

    ; load ring 3 data segments
    mov ax, USER_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push USER_DS ; ss
    push user_stack_top

    pushfd
    pop eax
    or eax, 0x200 ; inturupts
    push eax

    push USER_CS ; cs
    push program_start ; eip

    iret
