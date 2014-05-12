#include "or1k-asm.h"

/*
 * Define symbols to be used during startup - file is linked at compile time
 *
 */
.weak _board_mem_base
.weak _board_mem_size
.weak _board_clk_freq

// TODO: set memory base here
_board_mem_base:	.long	0x0
// TODO: set memory size here
_board_mem_size:	.long	0x0

// TODO: set board clock frequency here
_board_clk_freq:	.long	50000000

// TODO: UART configuration
.weak _board_uart_base
.weak _board_uart_baud
.weak _board_uart_IRQ
_board_uart_base:	.long	0x0
_board_uart_baud:	.long	0
_board_uart_IRQ:	.long	0

// TODO: Board exit function, default: loop
.weak _board_exit
_board_exit:
        OR1K_DELAYED_NOP(l.j _board_exit)

// TODO: Early initialization (if really needed!)
.global _board_init_early
_board_init_early:
    OR1K_DELAYED_NOP(l.jr r9)

// TODO: Board initialization
.weak _board_init
_board_init:
    OR1K_DELAYED_NOP(l.jr r9)
