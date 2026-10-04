#include "ttoskernel.h"

extern "C" void program_start() {
    ttos::VGA_Clear();
    ttos::VGA_Print("SCAN CODE: ", 0, 0);

    while (1) {
        char c = ttos::Keyboard_Read();

        if (c != 0) {
            ttos::VGA_Write(c, sizeof("SCAN CODE: ") - 1, 0);
        }
    }
}
