#include "ttoskernel.h"
int doMath(const char *expression) {
    unsigned int pointer = 0;
    char c = expression[pointer];

    char num1[20];
    num1[0] = '\0';

    char num2[20];
    num2[0] = '\0';

    enum seleted_tung {
        tung_tung_sa_add,
        tung_tung_sa_subtract,
        tung_tung_sa_multiply,
        tung_tung_sa_divide, // we yes we hitting up #DE flag for /0
    };

    seleted_tung selection = tung_tung_sa_add;

    char *selected = num1;
    unsigned int selectedPointer = 0;

    while (expression[pointer] != '\0') {
        c = expression[pointer++];

        if (c == ' ') {
            continue;
        }

        // Operator
        if (c == '+') {
            selection = tung_tung_sa_add;
            selected = num2;
            selectedPointer = 0;
            continue;
        }

        if (c == '-') {
            selection = tung_tung_sa_subtract;
            selected = num2;
            selectedPointer = 0;
            continue;
        }

        if (c == 'x') {
            selection = tung_tung_sa_multiply;
            selected = num2;
            selectedPointer = 0;
            continue;
        }

        if (c == '/') {
            selection = tung_tung_sa_divide;
            selected = num2;
            selectedPointer = 0;
            continue;
        }

        // Number
        if (c >= '0' && c <= '9') {
            if (selectedPointer < 19) {
                selected[selectedPointer++] = c;
                selected[selectedPointer] = '\0';
            }
        }
    }

    int n1 = 0;
    int n2 = 0;

    for (unsigned int i = 0; num1[i] != '\0'; i++) {
        n1 = n1 * 10 + (num1[i] - '0');
    }

    for (unsigned int i = 0; num2[i] != '\0'; i++) {
        n2 = n2 * 10 + (num2[i] - '0');
    }

    switch (selection) {
    case tung_tung_sa_add:
        return n1 + n2;

    case tung_tung_sa_subtract:
        return n1 - n2;

    case tung_tung_sa_multiply:
        return n1 * n2;

    case tung_tung_sa_divide:
        if (n2 == 0)
            ; // GG rip ur cpu idiot
        return n1 / n2;
    }

    return 0;
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
