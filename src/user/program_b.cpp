#include "stdlib.h"
using namespace std;

extern "C" void _start() {
    const char frames[] = {'|', '/', '-', '\\'};
    uint32_t frame = 0;
    uint32_t ticks = 0;

    print_str("[B] ANIMATION", 2, 6);
    print_str("Worker:", 2, 7);
    print_str("Cycles:", 2, 8);

    while (1) {
        print_char_at(frames[frame], 10, 7);
        frame = (frame + 1) % 4;

        clear_to_end_of_line(10, 8);

        uint32_t n = ticks++;
        char digits[10];
        uint32_t count = 0;

        do {
            digits[count++] = '0' + n % 10;
            n /= 10;
        } while (n && count < 10);

        while (count) {
            print_char_at(digits[--count], 10 + (ticks % 10), 8);
        }

        for (volatile uint32_t i = 0; i < 20'000'000; i++) {
        }
    }
}
