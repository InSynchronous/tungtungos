#include "ttoskernel.h"

extern "C" void program_start();
volatile unsigned char *vga = (volatile unsigned char *)0xB8000;
volatile unsigned char scan_debug[64];
volatile uint32_t scan_debug_count = 0;
unsigned char vga_color = 0x0F;
uint32_t pit_ticks = 0; // relative time
ttos::TSS *tss = reinterpret_cast<ttos::TSS *>(0x5000);

#define STACK_SIZE 4096
#define MAX_TASKS 4

// TODO: infinite taskss and better memory alloc
static uint32_t task_sizes[MAX_TASKS] = {
    0x9000 + STACK_SIZE * 1, // task 0
    0x9000 + STACK_SIZE * 2, // task 1
    0x9000 + STACK_SIZE * 3, // task 2
    0x9000 + STACK_SIZE * 4, // task 3
};

extern "C" {

ttos::TCB *current_tcb;
ttos::TCB tasks[MAX_TASKS];
volatile uint32_t task_index = 0; // for now index = process id
}

extern "C" void kernel_start() {
    ttos::VGA_Clear();
    ttos::VGA_Print("TTOS VERSION 0.1a: LOADING PROGRAM", 0, 0);

    ttos::Keyboard_Init();
    ttos::Paging_Init();
    ttos::IDT_Init();
    ttos::PIC_Init();
    TSS_Init();

    Tasks_Init();
    ttos::PIT_Init(100);

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
        uint32_t fault_address;

        asm volatile("mov %%cr2, %0" : "=r"(fault_address));

        ttos::VGA_Print("Page Fault (#PF)", 27, 13);
        ttos::VGA_Print("Fault address:", 27, 14);
        ttos::VGA_PrintHex(fault_address, 42, 14);

        break;
    }

    ttos::VGA_Print("The system has been halted.", 27, 16);
    ttos::CPU_halt();
}

extern "C" uint32_t syscall_handler(unsigned int syscall_number, uint32_t arg1,
                                    unsigned int arg2, uint32_t arg3) {
    switch (syscall_number) {
    case 1:
        ttos::VGA_Write((char)arg1, 0, 20);
        return 0;

    case 2:
        ttos::VGA_Print((const char *)arg1, (int)arg2, (int)arg3);
        return 0;

    case 3:
        ttos::VGA_Clear();
        return 0;

    case 4:
        return (unsigned char)ttos::Keyboard_Read();

    case 5:
        return arg1 + 100;

    case 7:
        if (arg1 < 80 && arg2 < 25) {
            for (uint32_t x = arg1; x < 80; x++)
                ttos::VGA_Write(' ', x, arg2);
        }
        return 0;
    case 8:
        ttos::VGA_Write((char)arg1, (int)arg2, (int)arg3);
        return 0;
    }

    return 0;
}
volatile char keyboard_buffer[128];
volatile uint32_t keyboard_read_pos = 0;
volatile uint32_t keyboard_write_pos = 0;

extern "C" void keyboard_handler() {
    unsigned char scan_code = ttos::inb(0x60);

    if (scan_code >= 128)
        return;

    char c = ttos::keyboard_map[scan_code];
    if (c == 0)
        return;

    static uint32_t x = 0;
    x = (x + 1) % 80;

    uint32_t next = (keyboard_write_pos + 1) % 128;

    if (next != keyboard_read_pos) {
        keyboard_buffer[keyboard_write_pos] = c;
        keyboard_write_pos = next;
    }
}
extern "C" void timer_handler() {
    /*
    ttos::VGA_Print("timer:", 5, 10);
    ttos::VGA_PrintHex(pit_ticks, 5, 11);
    */
    pit_ticks++;
}

extern "C" void TSS_Init() {
    volatile unsigned char *p = reinterpret_cast<volatile unsigned char *>(tss);

    for (uint32_t i = 0; i < sizeof(ttos::TSS); i++) {
        p[i] = 0;
    }

    tss->ss0 = 0x10;
    tss->esp0 = 0x90000;
    tss->iomap_base = sizeof(ttos::TSS);

    uint16_t selector = 0x28;
    asm volatile("ltr %0" : : "rm"(selector));
}

extern "C" uint32_t scheduler(uint32_t saved_esp) {
    // save esp
    current_tcb->saved_esp = saved_esp;

    // round robin impl
    for (uint32_t i = 1; i <= MAX_TASKS; ++i) {
        uint32_t next = (task_index + i) % MAX_TASKS;

        if (tasks[next].state != TASK_READY)
            continue;

        task_index = next;
        current_tcb = &tasks[next];

        tss->esp0 = tasks[next].kernel_stack_top;

        return tasks[next].saved_esp;
    }

    // if no other processes, resume
    ttos::VGA_SetColor(0x4F);
    ttos::VGA_Write('X', 5, 2);
    // ttos::VGA_SetColor(0x4F);

    tss->esp0 = current_tcb->kernel_stack_top;
    return current_tcb->saved_esp;
}

extern "C" uint32_t make_initial_context(uint32_t kernel_stack_top,
                                         uint32_t user_stack_top,
                                         uint32_t entry) {
    uint32_t *sp = reinterpret_cast<uint32_t *>(kernel_stack_top);
    // push everything to stack. stack memory sp goes DOWN!
    // lwk spent 50 minutes wondering why sp++ didn't work

    // the iret frame aka the thing iret does
    *--sp = 0x23;           // User SS
    *--sp = user_stack_top; // User ESP
    *--sp = 0x202;          // EFLAGS: reserved bit + IF
    *--sp = 0x1B;           // User CS
    *--sp = entry;          // User EIP

    // normal registers
    *--sp = 0; // EAX
    *--sp = 0; // ECX
    *--sp = 0; // EDX
    *--sp = 0; // EBX
    *--sp = 0; // original ESP placeholder
    *--sp = 0; // EBP
    *--sp = 0; // ESI
    *--sp = 0; // EDI

    // segment registers
    *--sp = 0x23; // DS
    *--sp = 0x23; // ES
    *--sp = 0x23; // FS
    *--sp = 0x23; // GS

    return reinterpret_cast<uint32_t>(sp);
}

extern "C" void Tasks_Init() {
    for (uint32_t i = 0; i < MAX_TASKS; ++i) {
        uint32_t base = task_sizes[i];

        tasks[i].kernel_stack_top = base + STACK_SIZE;
        tasks[i].user_stack_top = base + STACK_SIZE * 2;

        // for now everyone starts at the same entry
        // todo: implement a safer fix
        tasks[i].entry = reinterpret_cast<uint32_t>(program_start);

        tasks[i].saved_esp = make_initial_context(
            tasks[i].kernel_stack_top, tasks[i].user_stack_top, tasks[i].entry);

        tasks[i].state = TASK_READY;
    }

    task_index = 0;
    current_tcb = &tasks[0];
    tss->esp0 = tasks[0].kernel_stack_top;
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

void VGA_PrintHex(uint32_t value, int x, int y) {
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
    uint32_t flags;

    asm volatile("pushfl\n"
                 "popl %0\n"
                 "cli\n"
                 : "=r"(flags)
                 :
                 : "memory");

    if (keyboard_read_pos == keyboard_write_pos) {
        asm volatile("pushl %0\n"
                     "popfl\n"
                     :
                     : "r"(flags)
                     : "memory");
        return 0;
    }

    char c = keyboard_buffer[keyboard_read_pos];
    keyboard_read_pos = (keyboard_read_pos + 1) % 128;

    asm volatile("pushl %0\n"
                 "popfl\n"
                 :
                 : "r"(flags)
                 : "memory");

    return c;
}

void PIT_Init(uint32_t frequency) {
    uint32_t divisor = 1193182 / frequency;

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

void IDT_SetGate(unsigned char vector, uint32_t handler) {
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = 0x08; // gdt code
    idt[vector].zero = 0;
    idt[vector].type_attr = 0x8E;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}

void IDT_SetUserGate(unsigned char vector, uint32_t handler) {
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = 0x08;
    idt[vector].zero = 0;
    idt[vector].type_attr = 0xEE; // Present, DPL=3, 32-bit interrupt gate
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}

void IDT_Load() {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt; // ignore lsp warning; 32 bit code

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

    IDT_SetGate(0x21, (uint32_t)keyboard_isr);
    IDT_SetGate(0x00, (uint32_t)divide_error_isr);
    IDT_SetGate(0x0D, (uint32_t)general_protection_isr);
    IDT_SetGate(0x0E, (uint32_t)page_fault_isr);
    IDT_SetGate(0x20, (uint32_t)timer_isr);
    IDT_SetUserGate(0x80, (uint32_t)syscall_isr);

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
alignas(4096) uint32_t page_directory[1024];
alignas(4096) uint32_t kernel_page_table[1024];
alignas(4096) uint32_t user_page_table[1024];

void Paging_Init() {
    // clear everything
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
        kernel_page_table[i] = 0;
        user_page_table[i] = 0;
    }

    // kernel: 0x00000000 - 0x003FFFFF
    // keep NULL unmapped
    for (uint32_t i = 1; i < 1024; i++) {
        uint32_t address = i * 0x1000;

        // Present + Writable, Supervisor-only
        kernel_page_table[i] = address | 0x3;
    }

    // current Ring 3 program lives around 0x8000.
    for (uint32_t i = 8; i < 32; i++) {
        uint32_t address = i * 0x1000;

        // Present + Writable + User
        kernel_page_table[i] = address | 0x7;
    }

    // page directory 0
    page_directory[0] = ((uint32_t)kernel_page_table) | 0x7;

    // 0x400000 - 0x7FFFFF unmapped for now.
    page_directory[1] = 0;

    asm volatile("mov %0, %%cr3" : : "r"(page_directory) : "memory");

    uint32_t cr0;

    asm volatile("mov %%cr0, %0" : "=r"(cr0));

    cr0 |= 0x80000000; // CR0.PG

    asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}
} // namespace ttos
