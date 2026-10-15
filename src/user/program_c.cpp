#include "stdlib.h"
using namespace std;

void print_number(uint32_t n, uint32_t x, uint32_t y) {
    char digits[10];
    uint32_t count = 0;

    do {
        digits[count++] = '0' + n % 10;
        n /= 10;
    } while (n && count < 10);

    clear_to_end_of_line(x, y);

    for (uint32_t i = 0; i < count; i++) {
        print_char_at(digits[count - 1 - i], x + i, y);
    }
}

bool is_prime(uint32_t n) {
    if (n < 2)
        return false;

    if (n == 2)
        return true;

    if (n % 2 == 0)
        return false;

    for (uint32_t i = 3; i <= n / i; i += 2) {
        if (n % i == 0)
            return false;
    }

    return true;
}

extern "C" void _start() {
    uint32_t candidate = 2;
    uint32_t prime_count = 0;
    uint32_t last_prime = 0;
    uint32_t display_tick = 0;

    const uint32_t DISPLAY_INTERVAL = 10'000;

    print_str("[C] PRIME NUMBER WORKER", 2, 10);
    print_str("Latest prime:", 2, 11);
    print_str("Primes found:", 2, 12);

    while (1) {
        if (is_prime(candidate)) {
            last_prime = candidate;
            prime_count++;
        }

        if (++display_tick >= DISPLAY_INTERVAL) {
            display_tick = 0;
            print_number(last_prime, 16, 11);
            print_number(prime_count, 16, 12);
        }

        candidate++;
    }
}
