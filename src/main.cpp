#include "ttoskernel.h"

int doMath(const char *expression) {
    unsigned int pointer = 0;
    char c = expression[pointer];
    int result = 67;

    while (c != '\0') {
        c = expression[pointer++];
    }

    return result;
}

void int_to_string(int value, char *buffer) {
    char temp[12];
    int i = 0;
    bool negative = false;

    if (value < 0) {
        negative = true;
        value = -value;
    }

    do {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    if (negative)
        temp[i++] = '-';

    int j = 0;
    while (i > 0)
        buffer[j++] = temp[--i];

    buffer[j] = '\0';
}

extern "C" void program_start() {
    char buffer[100];
    char result_str[100];

    ttos::VGA_Clear();
    ttos::VGA_Print("SCAN CODE: ", 0, 0);

    unsigned int pointer = 0;

    buffer[0] = '\0';
    result_str[0] = '\0';

    while (1) {
        ttos::VGA_Clear();

        ttos::VGA_Print("Tung Tung Calculator: ", 0, 0);
        ttos::VGA_Print(buffer, sizeof("Tung Tung Calculator: ") - 1, 0);

        ttos::VGA_Print("Result: ", 0, 5);
        ttos::VGA_Print(result_str, sizeof("Result: ") - 1, 5);

        char c = ttos::Keyboard_Read();

        if (c != 0) {
            if (c == '\b') {
                if (pointer > 0) {
                    pointer--;
                    buffer[pointer] = '\0';
                }
            } else if (c == '\n') {
                int result = doMath(buffer);
                int_to_string(result, result_str);
            } else {
                buffer[pointer++] = c;
                buffer[pointer] = '\0';
            }
        }
    }
}
