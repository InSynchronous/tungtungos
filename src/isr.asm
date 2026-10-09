bits 32

global keyboard_isr
global divide_error_isr
global general_protection_isr
global page_fault_isr
global syscall_isr
global timer_isr

extern keyboard_handler
extern kernel_panic
extern syscall_handler
extern timer_handler

keyboard_isr:
    pusha

    call keyboard_handler
    mov al, 0x20
    out 0x20, al
    popa
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

    mov ebp, esp

    push dword [ebp + 20] ; arg3: EDX
    push dword [ebp + 24] ; arg2: ECX
    push dword [ebp + 16] ; arg1: EBX
    push dword [ebp + 28] ; number: EAX

    call syscall_handler
    add esp, 16

    ; replace saved EAX with syscall result
    mov [esp + 28], eax

    popa
    iretd

timer_isr:
    pusha

    call timer_handler

    popa

    mov al, 0x20
    out 0x20, al

    iretd
