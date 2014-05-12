
// TODO: set memory base here
unsigned long __attribute__((weak)) _board_mem_base = 0x0;

// TODO: set memory size here
unsigned long __attribute__((weak)) _board_mem_size = 0x0;

// TODO: set board clock frequency here
unsigned long __attribute__((weak)) _board_clk_freq = 0x0;

// TODO: UART configuration
unsigned long __attribute__((weak)) _board_uart_base = 0x0;
unsigned long __attribute__((weak)) _board_uart_baud = 0x0;
unsigned long __attribute__((weak)) _board_uart_IRQ = 0x0;

// TODO: Board exit function, default: loop
void __attribute__((weak)) _board_exit(void) {
    while (1) {}
}

// TODO: Board initialization
void __attribute__((weak)) _board_init(void) {
    return;
}
