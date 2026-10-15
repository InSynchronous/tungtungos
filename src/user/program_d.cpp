#include "stdlib.h"
using namespace std;

/*
TungTung Calculator
*/

int doMath(const char *expression);
void int_to_string(int value, char *buffer);

extern "C" void _start() {
    char buffer[100];
    char result_str[100];
    unsigned int pointer = 0;

    const uint32_t INPUT_X = 9;
    const uint32_t RESULT_X = 10;
    const unsigned int MAX_LEN = 78 - INPUT_X; // dont run off the edge

    buffer[0] = '\0';
    result_str[0] = '\0';

    print_str("[A] CALCULATOR", 2, 14);
    print_str("Input:", 2, 15);
    print_str("Result:", 2, 16);
    print_str("Type an expression and press Enter.", 2, 17);

    while (1) {
        char c = get_char();

        if (c == 0)
            continue;

        if (c == '\b') {
            if (pointer > 0) {
                --pointer;
                buffer[pointer] = '\0';
                print_char_at(' ', INPUT_X + pointer, 15);
            }
        } else if (c == '\n' || c == '\r') {
            int result = doMath(buffer);
            int_to_string(result, result_str);

            clear_to_end_of_line(RESULT_X, 16);
            print_str(result_str, RESULT_X, 16);

            pointer = 0;
            buffer[0] = '\0';
            clear_to_end_of_line(INPUT_X, 15);
        } else if (pointer < MAX_LEN && pointer < sizeof(buffer) - 1) {
            buffer[pointer] = c;
            print_char_at(c, INPUT_X + pointer, 15);
            ++pointer;
            buffer[pointer] = '\0';
        }
    }
}

int doMath(const char *expression) {
    unsigned int pointer = 0;
    char c;

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
