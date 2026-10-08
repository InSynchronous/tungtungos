#include "ttoskernel.h"

extern "C" void program_start();
volatile unsigned char *vga = (volatile unsigned char *)0xB8000;
unsigned char vga_color = 0x0F;

extern "C" void kernel_start() {
    const char *string = "TTOS VERSION 0.1a: LOADING PROGRAM";

    ttos::VGA_Clear();
    ttos::VGA_Print(string, 0, 0);

    ttos::Keyboard_Init();

    ttos::Paging_Init();

    ttos::IDT_Init();
    ttos::PIC_Init();

    // don't enable interrupts yet
    // asm volatile("sti");
    enter_user_mode();
}

extern "C" void kernel_panic(KernelFault fault) {
    ttos::VGA_SetColor(0x4F); // white text red background

    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            ttos::VGA_Write(' ', x, y);
        }
    }

    ttos::VGA_Print("TTOS KERNEL PANIC", 30, 10);
    ttos::VGA_Print("A fatal error has occurred.", 27, 12);
    switch (fault) {
    case KernelFault::DivideByZero:
        ttos::VGA_Print("Division by zero (#DE)", 27, 13);
        break;

    case KernelFault::GeneralProtection:
        ttos::VGA_Print("General Protection Fault (#GP)", 27, 13);
        break;

    case KernelFault::PageFault:
        unsigned int fault_address;

        asm volatile("mov %%cr2, %0" : "=r"(fault_address));

        ttos::VGA_Print("Page Fault (#PF)", 27, 13);
        ttos::VGA_Print("Fault address:", 27, 14);
        ttos::VGA_PrintHex(fault_address, 42, 14);

        break;
    }

    ttos::VGA_Print("The system has been halted.", 27, 16);
    ttos::CPU_halt();
}

extern "C" unsigned int syscall_handler() {
    unsigned int syscall_number;

    asm volatile("mov %%eax, %0" : "=r"(syscall_number));

    if (syscall_number == 1) {
        unsigned int character;

        asm volatile("mov %%ebx, %0" : "=r"(character));

        ttos::VGA_Write((char)character, 0, 20);
        return 0;
    }

    if (syscall_number == 2) {
        const char *string;
        unsigned int x;
        unsigned int y;

        asm volatile("mov %%ebx, %0" : "=r"(string));
        asm volatile("mov %%ecx, %0" : "=r"(x));
        asm volatile("mov %%edx, %0" : "=r"(y));

        ttos::VGA_Print(string, x, y);
        return 0;
    }

    if (syscall_number == 3) {
        ttos::VGA_Clear();
        return 0;
    }

    if (syscall_number == 4) {
        return (char)ttos::Keyboard_Read();
    }

    if (syscall_number == 5) {
        unsigned int value;

        asm volatile("mov %%ebx, %0" : "=r"(value));

        return value + 100;
    }

    return 0;
}

volatile char keyboard_buffer[128];
volatile unsigned int keyboard_read_pos = 0;
volatile unsigned int keyboard_write_pos = 0;

extern "C" void keyboard_handler() {
    unsigned char scan_code = ttos::inb(0x60);

    if (scan_code < 128) {
        char c = ttos::keyboard_map[scan_code];

        if (c != 0) {
            unsigned int next = (keyboard_write_pos + 1) % 128;

            if (next != keyboard_read_pos) {
                keyboard_buffer[keyboard_write_pos] = c;
                keyboard_write_pos = next;
            }
        }
    }

    // EOI
    ttos::outb(0x20, 0x20);
}

namespace ttos {
unsigned char inb(unsigned short port) {
    unsigned char value;

    // read port
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));

    return value;
}

void outb(unsigned short port, unsigned char value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void VGA_SetColor(unsigned char color) { vga_color = color; }

void VGA_Write(const char character, int x, int y) {
    int offset = (y * 80 + x) * 2;

    vga[offset] = character;
    vga[offset + 1] = vga_color;
}

void VGA_Print(const char *string, int x, int y) {
    for (int i = 0; string[i] != '\0'; i++) {
        VGA_Write(string[i], x + i, y);
    }
}

void VGA_PrintHex(unsigned int value, int x, int y) {
    const char *hex = "0123456789ABCDEF";

    VGA_Write('0', x, y);
    VGA_Write('x', x + 1, y);

    for (int i = 0; i < 8; i++) {
        VGA_Write(hex[(value >> ((7 - i) * 4)) & 0xF], x + 2 + i, y);
    }
}

void VGA_Clear() {
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            VGA_Write(' ', x, y);
        }
    }
}

void CPU_halt() {
    while (1) {
        asm volatile("hlt");
    }
}

void Keyboard_Init() {
    while (inb(0x64) & 0x01)
        inb(0x60);
}

char Keyboard_Read() {
    if (keyboard_read_pos == keyboard_write_pos)
        return 0;

    char c = keyboard_buffer[keyboard_read_pos];

    keyboard_read_pos = (keyboard_read_pos + 1) % 128;

    return c;
}

void PIT_Init(unsigned int frequency) {
    unsigned int divisor = 1193182 / frequency;

    // init
    outb(0x43, 0x36);
    // send our divisor low byte
    outb(0x40, divisor & 0xFF);
    // send our divisor high byte
    outb(0x40, (divisor >> 8) & 0xFF);
}

// IDT system!
struct IDTEntry idt[256];
struct IDTPointer idt_ptr;

void IDT_SetGate(unsigned char vector, unsigned int handler) {
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = 0x08; // gdt code
    idt[vector].zero = 0;
    idt[vector].type_attr = 0x8E;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}

void IDT_SetUserGate(unsigned char vector, unsigned int handler) {
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = 0x08;
    idt[vector].zero = 0;
    idt[vector].type_attr = 0xEE; // Present, DPL=3, 32-bit interrupt gate
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}

void IDT_Load() {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (unsigned int)&idt; // ignore lsp warning; 32 bit code

    asm volatile("lidt %0" : : "m"(idt_ptr));
}

void IDT_Init() {
    for (int i = 0; i < 256; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }

    IDT_SetGate(0x21, (unsigned int)keyboard_isr);
    IDT_SetGate(0x00, (unsigned int)divide_error_isr);
    IDT_SetGate(0x0D, (unsigned int)general_protection_isr);
    IDT_SetGate(0x0E, (unsigned int)page_fault_isr);
    IDT_SetUserGate(0x80, (unsigned int)syscall_isr);

    IDT_Load();
}

// Default PIC is overlapping with actual CPU exception IRQs (#GP #PF #DE)
// KB -> PIC -> IDT -> Function
void PIC_Init() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // Vector offsets
    outb(0x21, 0x20); // remap irq0 to 0x20
    outb(0xA1, 0x28); // remap irq8 to 0x28

    // Wiring
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // mask all
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    // UNMASK keyboard
    // todo: more stuff; kernel panics
    outb(0x21, 0xFD);
}

// Paging
alignas(4096) unsigned int page_directory[1024];
alignas(4096) unsigned int kernel_page_table[1024];
alignas(4096) unsigned int user_page_table[1024];

void Paging_Init() {
    // clear everything
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
        kernel_page_table[i] = 0;
        user_page_table[i] = 0;
    }

    // kernel: 0x00000000 - 0x003FFFFF
    // keep NULL unmapped
    for (unsigned int i = 1; i < 1024; i++) {
        unsigned int address = i * 0x1000;

        // Present + Writable, Supervisor-only
        kernel_page_table[i] = address | 0x3;
    }

    // current Ring 3 program lives around 0x8000.
    for (unsigned int i = 8; i < 32; i++) {
        unsigned int address = i * 0x1000;

        // Present + Writable + User
        kernel_page_table[i] = address | 0x7;
    }

    // page directory 0
    page_directory[0] = ((unsigned int)kernel_page_table) | 0x7;

    // 0x400000 - 0x7FFFFF unmapped for now.
    page_directory[1] = 0;

    asm volatile("mov %0, %%cr3" : : "r"(page_directory) : "memory");

    unsigned int cr0;

    asm volatile("mov %%cr0, %0" : "=r"(cr0));

    cr0 |= 0x80000000; // CR0.PG

    asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}
} // namespace ttos
