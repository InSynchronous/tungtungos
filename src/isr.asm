bits 32

global keyboard_isr
global divide_error_isr
global general_protection_isr
global page_fault_isr
global syscall_isr

extern keyboard_handler
extern kernel_panic
extern syscall_handler

keyboard_isr:
    pusha

    call keyboard_handler

    popa

    mov al, 0x20
    out 0x20, al

    iretd

divide_error_isr:
    pusha

    push 0
    call kernel_panic

    popa

    mov al, 0x20
    out 0x20, al

    iretd

general_protection_isr:
    pusha

    push 1
    call kernel_panic

    popa

    mov al, 0x20
    out 0x20, al

    iretd

page_fault_isr:
    pusha

    push 2
    call kernel_panic

    popa

    mov al, 0x20
    out 0x20, al

    iretd

syscall_isr:
    pusha

    call syscall_handler

    mov [esp + 28], eax

    popa
    iretd
