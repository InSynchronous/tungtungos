bits 32
KERNEL_DS equ 0x10

global enter_user_mode
extern program_start
extern timer_handler
extern scheduler
extern current_tcb
extern restore_context

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
    mov eax, [current_tcb]
    mov esp, [eax]             ; first field must be saved_esp
    jmp restore_context
