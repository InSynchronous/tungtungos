bits 32
KERNEL_DS equ 0x10

global keyboard_isr
global divide_error_isr
global general_protection_isr
global page_fault_isr
global syscall_isr
global timer_isr
global restore_context

extern keyboard_handler
extern kernel_panic
extern syscall_handler
extern timer_handler
extern scheduler

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
    ; CPU has already pushed the privilege-transition frame:
    ; EIP, CS, EFLAGS, user ESP, user SS (in reverse stack order).

    pusha

    xor eax, eax
    mov ax, ds
    push eax

    xor eax, eax
    mov ax, es
    push eax

    xor eax, eax
    mov ax, fs
    push eax

    xor eax, eax
    mov ax, gs
    push eax

    ; kernel
    mov ax, KERNEL_DS
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; update clock
    call timer_handler

    ; end irq
    mov al, 0x20
    out 0x20, al

    ; scheduler(saved_esp) returns the next tasks esp.
    push esp
    call scheduler
    add esp, 4

    mov esp, eax ; esp = eax = new sp

restore_context:
    ; reverse order save segments
    ; ax = lower 16 eax = 32 and rax=64 bit
    ; ttos is a 32 bit os
    pop eax
    mov gs, ax ; lower 16 eax

    pop eax
    mov fs, ax

    pop eax
    mov es, ax
    
    pop eax
    mov ds, ax

    popa

    iretd ; should restore eip cs eflags, esp, ss in rnig 3 hopefuly

