#include "stdlib.h"

void std::print_char(char c) { syscall1(1, (unsigned char)c); }
void std::clear_screen() { syscall0(3); }
char std::get_char() { return (char)syscall0(4); }
void std::print_str(const char *string, uint32_t x, uint32_t y) {
    syscall3(2, (uint32_t)string, x, y);
}
void std::print_char_at(char c, uint32_t x, uint32_t y) {
    syscall3(8, (unsigned char)c, x, y);
}
void std::clear_to_end_of_line(uint32_t x, uint32_t y) { syscall2(7, x, y); }
