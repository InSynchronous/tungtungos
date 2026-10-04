#include "ttoskernel.h"

extern "C" void program_start();
volatile unsigned char *vga = (volatile unsigned char *)0xB8000;
unsigned char vga_color = 0x0F;

extern "C" void kernel_start() {
    const char *string = "TTOS VERSION 0.1a: LOADING PROGRAM";

    ttos::VGA_Clear();
    ttos::VGA_Print(string, 0, 0);

    program_start();
}

namespace ttos {
namespace {
// Internal functions.
unsigned char inb(unsigned short port) {
    unsigned char value;

    // read port
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));

    return value;
}

void outb(unsigned short port, unsigned char value) {
    asm volatile("outb %0, %1" : "=a"(value) : "Nd"(port));
}

static const char keyboard_map[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6', '7',  '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r', 't',  'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',

    'd', 'f', 'g',  'h',  'j',  'k', 'l', ';', '\'', '`', 0,   '\\',
    'z', 'x', 'c',  'v',  'b',  'n', 'm', ',', '.',  '/', 0,   '*',
    0,   ' ', 0,    0,    0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,    0,    0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,    0,    0,    0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,    0,    0,    0,   0,   0,   0,    0,   0,   0};

} // namespace

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

char Keyboard_Read() {
    while (!(inb(0x64) & 1))
        ;

    unsigned char scan_code = inb(0x60);

    if (scan_code & 0x80)
        return 0;

    return keyboard_map[scan_code];
}

} // namespace ttos
