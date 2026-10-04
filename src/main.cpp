#include "ttoskernel.h"

extern "C" void program_start() {
    ttos::VGA_Clear();
    ttos::VGA_SetColor(0x0A);
    ttos::VGA_Print("SUCCESS", 10, 5);

    ttos::VGA_SetColor(0x0C);
    ttos::VGA_Print("ERROR", 10, 6);

    ttos::VGA_SetColor(0x0E);
    ttos::VGA_Print("WARNING", 10, 7);
    ttos::CPU_halt();
}
