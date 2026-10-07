[org 0x7c00] ; start here
bits 16

start:
    cli ; end interrupts
    mov [boot_drive], dl
    ; Need to set up registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; enable a20 (physical line that enables more ram)
    in al, 0x92 ; save input 0x92 to al
    or al, 00000010b ; magic
    out 0x92, al; // write al to 0x92

    ; Load kernel
    xor ax, ax
    mov es, ax

    mov ah, 0x02       ; BIOS read
    mov al, 8          ; 6 sector
    mov ch, 0
    mov cl, 2          ; sector 2
    mov dh, 0
    mov dl, [boot_drive]

    mov bx, 0x8000

    int 0x13

    jc disk_error

    ; Load GDT, a
    lgdt [gdt_descriptor]

    ; pull cr0, operate on it, and write it back
    ; enables protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp 0x08:kernel

bits 32
kernel:
    mov ax, 0x10 ; gdt entry 2
    
    ; point everything to axa aka gdt entry 2
    ; these are segment registers, so it makes everything use the correct gdt
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; set sp
    mov esp, 0x90000

    ; use vga memory
    mov edi, 0xB8000

    mov byte [edi], 'H'
    mov byte [edi + 1], 0x0F

    mov byte [edi + 2], 'i'
    mov byte [edi + 3], 0x0F

    jmp 0x8000


disk_error:
    cli

.hang:
    hlt
    jmp .hang

boot_drive db 0

; Its purpose is to define what memory points to what
gdt_start:

gdt_null:
    dq 0x0000000000000000

gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:


gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Make it so that the rest is 0s and then the boot signature
times 510 - ($ - $$) db 0
dw 0xAA55 
