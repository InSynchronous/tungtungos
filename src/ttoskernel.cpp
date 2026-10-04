#include "ttoskernel.h"

extern "C" void program_start();
volatile unsigned char *vga = (volatile unsigned char *)0xB8000;

extern "C" void kernel_start() {
    const char *string = "TTOS VERSION 0.1a    LOADING PROGRAM";

    ttos::VGA_Clear();
    ttos::VGA_Print(string);

    program_start();
}

void ttos::VGA_Print(const char *string) {
    for (int i = 0; string[i] != '\0'; i++) {
        vga[i * 2] = string[i];
        vga[i * 2 + 1] = 0x0F;
    }
}

void ttos::VGA_Clear() {
    volatile unsigned char *vga = (volatile unsigned char *)0xB8000;

    for (int i = 0; i < 80 * 25; i++) {
        vga[i * 2] = ' ';
        vga[i * 2 + 1] = 0x0F;
    }
}

void ttos::halt() {
    while (1) {
        asm volatile("hlt");
    }
}
