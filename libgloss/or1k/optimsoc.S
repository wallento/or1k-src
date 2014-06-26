#include "or1k-asm.h"

#define LOAD_SYMBOL_2_GPR(gpr,symbol)  \
	.global symbol ;                   \
	l.movhi gpr, hi(symbol) ;          \
	l.ori   gpr, gpr, lo(symbol)

/* Peripheral information - Set base to 0 if not present*/
.weak _board_mem_base
.weak _board_mem_size
.weak _board_clk_freq

_board_mem_base:	.long	0
_board_mem_size:	.long	0

_board_clk_freq:	.long	0

/* Peripheral information - Set base to 0 if not present*/
.weak _board_uart_base
.weak _board_uart_baud
.weak _board_uart_IRQ

_board_uart_base:	.long	0
_board_uart_baud:	.long	0
_board_uart_IRQ:	.long	0


.extern _optimsoc_compute_tile_memsize
.extern _optimsoc_system_clock

.weak _board_exit
_board_exit:
	l.nop 0x1
.Lboard_exit_loop:
	OR1K_DELAYED_NOP(l.j .Lboard_exit_loop)

.global _board_init_early
_board_init_early:
	LOAD_SYMBOL_2_GPR(r1,_optimsoc_compute_tile_memsize)
	l.lwz	r1,0(r1)
	LOAD_SYMBOL_2_GPR(r2,_board_mem_base)
	LOAD_SYMBOL_2_GPR(r3,_board_mem_size)
	l.sw	0(r2),r0
	l.sw	0(r3),r1
	LOAD_SYMBOL_2_GPR(r1,_optimsoc_system_clock)
	l.lwz	r1,0(r1)
	LOAD_SYMBOL_2_GPR(r2,_board_clk_freq)
	l.sw	0(r2),r1
	OR1K_DELAYED_NOP(l.jr r9)

.weak _board_init
_board_init:
	OR1K_DELAYED_NOP(l.jr r9)
