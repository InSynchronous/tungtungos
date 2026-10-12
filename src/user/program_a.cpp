extern "C" void _start() {
    for (;;) {
        asm volatile("hlt");
    }
}
