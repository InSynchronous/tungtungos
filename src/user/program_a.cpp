#include "stdlib.h"
using namespace std;
void print_number(uint32_t n, uint32_t x, uint32_t y) {
    char buffer[11];
    uint32_t i = 0;

    do {
        buffer[i++] = '0' + n % 10;
        n /= 10;
    } while (n);

    while (i > 0) {
        print_char_at(buffer[--i], x++, y);
    }
}

extern "C" void _start() {
    for (;;) {
        uint32_t counter = 0;

        print_str("[A] SYSTEM MONITOR", 2, 2);
        print_str("Iterations:", 2, 3);
        print_str("State: RUNNING", 2, 4);

        while (1) {
            clear_to_end_of_line(14, 3);
            print_number(counter++, 14, 3);

            for (volatile uint32_t i = 0; i < 1500000; i++) {
            }
        }
    }
}
