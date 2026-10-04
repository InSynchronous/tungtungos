#pragma once

/*
TTOS Kernel. Userspace begins in `main.cpp`
*/

extern "C" void kernel_start();

namespace ttos {
void VGA_Clear();
void VGA_Print(const char *string);
void halt();
} // namespace ttos
