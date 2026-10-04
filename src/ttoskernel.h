#pragma once

/*
TTOS Kernel. Userspace begins in `main.cpp`
*/

extern "C" void kernel_start();

namespace ttos {
void VGA_Clear();
void VGA_Write(const char character, int x, int y);
void VGA_Print(const char *string, int x, int y);
void halt();
} // namespace ttos
