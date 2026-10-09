#pragma once
#include <cstdint>
#define uint32_t unsigned int
#define uint16_t short int

/*
TTOS Kernel. Userspace begins in `main.cpp`
*/

extern "C" void kernel_start();

extern "C" void enter_user_mode();

// implemented in assembly; inturrupt subroutine
// needs to be asm in order to push all the registers to the stack since we get
// inturrupted by something (keyboard press)
// pusha and popa are the instructions of choice

extern "C" void keyboard_isr();
extern "C" void keyboard_handler(); // called by isr

extern "C" void divide_error_isr();
extern "C" void general_protection_isr();
extern "C" void page_fault_isr();

extern "C" void timer_isr();
extern "C" void timer_handler();

// User ISR
extern "C" void syscall_isr();
extern "C" uint32_t syscall_handler(unsigned int syscall_number,
                                    unsigned int arg1, unsigned int arg2,
                                    unsigned int arg3);

enum class KernelFault : uint32_t {
    DivideByZero = 0,
    GeneralProtection = 1,
    PageFault = 2
};

extern "C" void kernel_panic(KernelFault fault); // called by isr

extern "C" void TSS_Init();

namespace ttos {

struct IDTEntry {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_high;
} __attribute__((packed));

struct IDTPointer {
    unsigned short limit;
    uint32_t base;
} __attribute__((packed));

static const char keyboard_map[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6', '7',  '8', '9', '0',
    '-', '+', '\b', '\t', 'q',  'w', 'e', 'r', 't',  'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',

    'd', 'f', 'g',  'h',  'j',  'k', 'l', ';', '\'', '`', 0,   '\\',
    'z', 'x', 'c',  'v',  'b',  'n', 'm', ',', '.',  '/', 0,   '*',
    0,   ' ', 0,    0,    0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,    0,    0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,    0,    0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,    0,    0,    0,   0,   0,   0,    0,   0,   0};

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char value);

struct TSS {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
};
static_assert(sizeof(TSS) == 104, "TSS layout is flawed! TSS won't work");

/*
 * @brief Initializes the Programmable Interval Timer
 *
 * @param frequency The target frequency for the PIT
 */
void PIT_Init(uint32_t frequency);

/**
 * @brief Reads a byte from the PS/2 keyboard.
 *
 * @return The keyboard scan code.
 */
char Keyboard_Read();

void Keyboard_Init();

/* *
 * @brief Set's the color of the VGA cursor in the text buffer
 *
 * @param color The color to set the cursor to
 */
void VGA_SetColor(unsigned char color);

/**
 * @brief Writes a single character to the VGA text buffer.
 *
 * @param character The character to display.
 * @param x The horizontal position, from 0 to 79.
 * @param y The vertical position, from 0 to 24.
 */
void VGA_Write(const char character, int x, int y);

/**
 * @brief Prints a null-terminated string to the VGA text buffer. Automatically
 * wraps text by character.
 *
 * @param string The string to print.
 * @param x The horizontal starting position, from 0 to 79.
 * @param y The vertical starting position, from 0 to 24.
 */
void VGA_Print(const char *string, int x, int y);

/**
 * @brief Prints an uint32_teger in hexadecimal format to the VGA display.
 *
 * The value is displayed as an 8-digit hexadecimal number prefixed with
 * "0x".
 *
 * @param value The uint32_teger to print.
 * @param x The horizontal VGA character position.
 * @param y The vertical VGA character position.
 */
void VGA_PrintHex(uint32_t value, int x, int y);

/**
 * @brief Clears the entire VGA text screen.
 */
void VGA_Clear();

/**
 * @brief Halts the CPU indefinitely.
 */
void CPU_halt();

/**
 * @brief Sets an entry in the Interrupt Descriptor Table.
 *
 * @param vector Interrupt vector number (0-255).
 * @param handler Address of the interrupt handler.
 */
void IDT_SetGate(unsigned char vector, uint32_t handler);

void IDT_SetUserGate(unsigned char vector, uint32_t handler);

/**
 * @brief Initializes and loads the Interrupt Descriptor Table.
 */
void IDT_Init();

/**
 * @brief Loads the Interrupt Descriptor Table into the CPU.
 */
void IDT_Load();

/**
 * @brief Initializes the programmable interrupt contrroller
 */
void PIC_Init();

/**
 * @brief Initializes the page tables & directory. enables cr0.pg
 */
void Paging_Init();

} // namespace ttos
