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

void ttos::VGA_SetColor(unsigned char color) { vga_color = color; }

void ttos::VGA_Write(const char character, int x, int y) {
    int offset = (y * 80 + x) * 2;

    vga[offset] = character;
    vga[offset + 1] = vga_color;
}

void ttos::VGA_Print(const char *string, int x, int y) {
    for (int i = 0; string[i] != '\0'; i++) {
        VGA_Write(string[i], x + i, y);
    }
}

void ttos::VGA_Clear() {
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            VGA_Write(' ', x, y);
        }
    }
}

void ttos::CPU_halt() {
    while (1) {
        asm volatile("hlt");
    }
}
