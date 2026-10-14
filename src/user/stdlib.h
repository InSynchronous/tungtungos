using uint32_t = unsigned int;
namespace std {
namespace {
static inline uint32_t syscall0(uint32_t number) {
    uint32_t result;

    asm volatile("int $0x80" : "=a"(result) : "a"(number) : "memory");

    return result;
}

static inline uint32_t syscall1(uint32_t number, uint32_t arg1) {
    uint32_t result;

    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(number), "b"(arg1)
                 : "memory");

    return result;
}

static inline uint32_t syscall2(uint32_t number, uint32_t arg1, uint32_t arg2) {
    uint32_t result;

    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(number), "b"(arg1), "c"(arg2)
                 : "memory");

    return result;
}

static inline uint32_t syscall3(uint32_t number, uint32_t arg1, uint32_t arg2,
                                uint32_t arg3) {
    uint32_t result;

    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(number), "b"(arg1), "c"(arg2), "d"(arg3)
                 : "memory");

    return result;
}
} // namespace

void print_char(char c);
void clear_screen();
char get_char();
void print_str(const char *string, uint32_t x, uint32_t y);
void print_char_at(char c, uint32_t x, uint32_t y);
void clear_to_end_of_line(uint32_t x, uint32_t y);

} // namespace std
