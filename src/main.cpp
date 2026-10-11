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

static inline unsigned int syscall0(unsigned int number) {
    unsigned int result;

    asm volatile("int $0x80" : "=a"(result) : "a"(number) : "memory");

    return result;
}

static inline unsigned int syscall1(unsigned int number, unsigned int arg1) {
    unsigned int result;

    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(number), "b"(arg1)
                 : "memory");

    return result;
}

static inline unsigned int syscall2(unsigned int number, unsigned int arg1,
                                    unsigned int arg2) {
    unsigned int result;

    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(number), "b"(arg1), "c"(arg2)
                 : "memory");

    return result;
}

static inline unsigned int syscall3(unsigned int number, unsigned int arg1,
                                    unsigned int arg2, unsigned int arg3) {
    unsigned int result;

    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(number), "b"(arg1), "c"(arg2), "d"(arg3)
                 : "memory");

    return result;
}

// temu libc
void print_char(char c) { syscall1(1, (unsigned char)c); }
void clear_screen() { syscall0(3); }
char get_char() { return (char)syscall0(4); }
void print_str(const char *string, unsigned int x, unsigned int y) {
    syscall3(2, (unsigned int)string, x, y);
}
void print_char_at(char c, unsigned int x, unsigned int y) {
    syscall3(8, (unsigned char)c, x, y);
}
void clear_to_end_of_line(unsigned int x, unsigned int y) { syscall2(7, x, y); }

static inline unsigned int syscall_test(unsigned int value) {
    unsigned int result;

    asm volatile("int $0x80" : "=a"(result) : "a"(5), "b"(value) : "memory");

    return result;
}

extern "C" void program_a_start() {
    unsigned int n = 0;
    char buffer[20];

    print_str("A (by 2s): ", 0, 1);

    while (1) {
        int_to_string(n, buffer);
        print_str(buffer, 11, 1);

        n += 2;

        // shitty msleep
        for (volatile unsigned int i = 0; i < 3000000; i++)
            ;
    }
}

extern "C" void program_b_start() {
    unsigned int n = 0;
    char buffer[20];

    print_str("B (by 3s): ", 0, 3);

    while (1) {
        int_to_string(n, buffer);
        print_str(buffer, 11, 3);

        n += 3;

        // shitty msleep
        for (volatile unsigned int i = 0; i < 3000000; i++)
            ;
    }
}

extern "C" void program_d_start() {
    while (1) {
        ;
    }
}

extern "C" void program_c_start() {
    char buffer[100];
    char result_str[100];
    unsigned int pointer = 0;

    buffer[0] = '\0';
    result_str[0] = '\0';

    clear_screen();

    print_str("TTOS VGA SPACING TEST", 0, 0);
    print_str("22222222222222222222", 0, 2);
    print_str("ABCDEFGHIJKLMNOPQRST", 0, 3);
    print_str("Input:", 0, 5);
    print_str("Result:", 0, 7);
    print_str("Type an expression and press Enter.", 0, 9);

    while (1) {
        char c = get_char();

        if (c == 0)
            continue;
        char hexbuf[4];
        hexbuf[0] = "0123456789ABCDEF"[((unsigned char)c) >> 4];
        hexbuf[1] = "0123456789ABCDEF"[c & 0xF];
        hexbuf[2] = ' ';
        hexbuf[3] = 0;
        static unsigned int dbg = 0;
        print_str(hexbuf, (dbg++ % 25) * 3, 12);

        if (c == '\b') {
            if (pointer > 0) {
                --pointer;
                buffer[pointer] = '\0';
                print_char_at(' ', 7 + pointer, 5);
            }
        } else if (c == '\n') {
            int result = doMath(buffer);
            int_to_string(result, result_str);

            clear_to_end_of_line(8, 7);
            print_str(result_str, 8, 7);

            pointer = 0;
            buffer[0] = '\0';
            clear_to_end_of_line(7, 5);
        } else if (pointer < sizeof(buffer) - 1) {
            buffer[pointer] = c;
            print_char_at(c, 7 + pointer, 5);
            ++pointer;
            buffer[pointer] = '\0';
        }
    }
}
