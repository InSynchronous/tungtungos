bits 32
KERNEL_DS equ 0x10

global enter_user_mode
extern timer_handler
extern scheduler
extern current_tcb
extern restore_context
extern selected_directory

USER_CS equ 0x1B
USER_DS equ 0x23
TSS_SEL equ 0x28
TSS equ 0x5000

enter_user_mode:
    cli
    mov eax, [current_tcb]
    mov esp, [eax]
    jmp restore_context
