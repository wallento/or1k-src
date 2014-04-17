/* crt0.S. C design runtime startup file.

   Copyright (C) 2004, Jacob Bower
   Copyright (C) 2010, Embecosm Limited <info@embecosm.com>
   Copyright (C) 2011, ORSoC AB
   Copyright (C) 2014, Authors

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
   Contributor Julius Baxter  <julius.baxter@orsoc.se>
   Contributor Stefan Wallentowitz <stefan.wallentowitz@tum.de>

   This file is part of Newlib.

   The original work by Jacob Bower is provided as-is without any kind of
   warranty. Use it at your own risk!

   All subsequent work is bound by version 3 of the GPL as follows.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License along
   with this program.  If not, see <http://www.gnu.org/licenses/>.            */
/* -------------------------------------------------------------------------- */
/* This program is commented throughout in a fashion suitable for processing
   with Doxygen.                                                              */
/* -------------------------------------------------------------------------- */

#include "or1k-asm.h"
#include "spr-defs.h"


/* -------------------------------------------------------------------------- */
// Stack definitions
/* -------------------------------------------------------------------------- */

// Stacks
// Memory layout:
//  +--------------------+ <- board_mem_base+board_mem_size/exception_stack_top
//  | exception stack(s) |
//  +--------------------+ <- stack_top
//  |     stack(s)       |
//  +--------------------+ <- stack_bottom
//  |      heap          |
//  +--------------------+
//  | text, data, bss..  |
//  +--------------------+

// Reserved stack size
#define STACK_SIZE 65536

// Reserved stack size for exceptions (can usually be smaller than normal stack)
#define EXCEPTION_STACK_SIZE 8192

// Size of redzone + size of space required to store state
// This value must match that in the support library or1k_exception_handler
// function
#define EXCEPTION_STACK_FRAME (128+128)

	.section .data
	.global stack_top     /* points to the next address after the stack */
	.global stack_bottom  /* points to the last address in the stack */
	.global stack_size    /* reserved stack size */
	.global exception_stack_top
	.global exception_stack_bottom
	.global exception_stack_size

stack_top:		.space	4,0
stack_bottom:	.space	4,0
stack_size:		.word	STACK_SIZE	/*!< Reserved stack space in bytes. */
exception_stack_top:	.space	4,0
exception_stack_bottom:	.space	4,0
exception_stack_size:	.word	EXCEPTION_STACK_SIZE

/* -------------------------------------------------------------------------- */
/*!Macro to load a symbol's address into a register.         
  
  @param[in] gpr  General purpose register to load address into.       
  @param[in] symbol Name of symbol to load.                                   */
/* -------------------------------------------------------------------------- */
#define LOAD_SYMBOL_2_GPR(gpr,symbol)  \
	.global symbol ;               \
	l.movhi gpr, hi(symbol) ;      \
	l.ori   gpr, gpr, lo(symbol)

/* -------------------------------------------------------------------------- */
/*!Macro to handle exceptions.

  Load NPC into r3, EPCR into r4
                                                                              */
/* -------------------------------------------------------------------------- */
#define CALL_EXCEPTION_HANDLER            		   \
    /* store current stack pointer to address 4 */ \
	l.sw    0x4(r0), r1;                           \
    /* Load top of the exception stack */          \
	l.movhi r1,hi(exception_stack_top);            \
	l.ori   r1,r1,lo(exception_stack_top);         \
	l.lwz   r1, 0(r1);                             \
    /* Reserve red zone and context space */       \
	l.addi  r1, r1, -EXCEPTION_STACK_FRAME;        \
    /* Store GPR3 in context */                    \
	l.sw    0x4(r1), r3;             			   \
    /* Load back software's stack pointer */       \
	l.lwz   r3, 0x4(r0);                           \
    /* Store this in the context */                \
	l.sw    0x78(r1), r3;                          \
    /* Store GPR4 in the context */                \
	l.sw    0x8(r1), r4;             			   \
    /* Copy the current program counter as first */\
    /* argument for the exception handler. This */ \
    /* is then used to determine the exception. */ \
	l.mfspr r3,r0,SPR_NPC;          		       \
    OR1K_DELAYED(					               \
    /* Copy program counter of exception as */     \
    /* second argument to the exception handler */ \
	OR1K_INST(l.mfspr r4,r0,SPR_EPCR_BASE),		   \
    /* Jump to exception handler. This will rfe */ \
	OR1K_INST(l.j 	or1k_exception_handler)		   \
    )

/* -------------------------------------------------------------------------- */
/*!Exception vectors                                                          */
/* -------------------------------------------------------------------------- */
	.section .vectors,"ax"

	/* 0x100: RESET exception */
        .org	0x100 	
__reset:
	l.movhi r0, 0
	l.movhi r1, 0
	l.movhi r2, 0
	l.movhi r3, 0
	l.movhi r4, 0
	l.movhi r5, 0
	l.movhi r6, 0
	l.movhi r7, 0
	l.movhi r8, 0
	l.movhi r9, 0
	l.movhi r10, 0
	l.movhi r11, 0
	l.movhi r12, 0
	l.movhi r13, 0
	l.movhi r14, 0
	l.movhi r15, 0
	l.movhi r16, 0
	l.movhi r17, 0
	l.movhi r18, 0
	l.movhi r19, 0
	l.movhi r20, 0
	l.movhi r21, 0
	l.movhi r22, 0
	l.movhi r23, 0
	l.movhi r24, 0
	l.movhi r25, 0
	l.movhi r26, 0
	l.movhi r27, 0
	l.movhi r28, 0
	l.movhi r29, 0
	l.movhi r30, 0
	l.movhi r31, 0

	/* Clear status register, set supervisor mode */
	l.ori r1, r0, SPR_SR_SM
	l.mtspr r0, r1, SPR_SR
	/* Clear timer mode register*/
	l.mtspr r0, r0, SPR_TTMR
	/* Jump to program initialisation code */
	LOAD_SYMBOL_2_GPR(r4, _start)
	OR1K_DELAYED_NOP(OR1K_INST(l.jr    r4))

	.org	0x200
	CALL_EXCEPTION_HANDLER

	/* 0x300: Data Page Fault exception */
	.org	0x300
	CALL_EXCEPTION_HANDLER
	
	/* 0x400: Insn Page Fault exception */
	.org	0x400
	CALL_EXCEPTION_HANDLER

	/* 0x500: Timer exception */
	.org	0x500
	CALL_EXCEPTION_HANDLER

	/* 0x600: Aligment exception */
	.org	0x600
	CALL_EXCEPTION_HANDLER
	
	/* 0x700: Illegal insn exception */
	.org	0x700
	CALL_EXCEPTION_HANDLER

	/* 0x800: External interrupt exception */
	.org	0x800
	CALL_EXCEPTION_HANDLER

	/* 0x900: DTLB miss exception */
	.org	0x900
	CALL_EXCEPTION_HANDLER
	
	/* 0xa00: ITLB miss exception */
	.org	0xa00
	CALL_EXCEPTION_HANDLER
	
	/* 0xb00: Range exception */
	.org	0xb00
	CALL_EXCEPTION_HANDLER
	
	/* 0xc00: Syscall exception */
	.org	0xc00
	CALL_EXCEPTION_HANDLER
	
	/* 0xd00: floating point exception */
	.org	0xd00
	CALL_EXCEPTION_HANDLER
	
	/* 0xe00: Trap exception */
	.org	0xe00
	CALL_EXCEPTION_HANDLER
	
	/* 0xf00: Reserved exceptions */
	.org	0xf00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1000
	CALL_EXCEPTION_HANDLER
	
	.org	0x1100
	CALL_EXCEPTION_HANDLER
	
	.org	0x1200
	CALL_EXCEPTION_HANDLER
	
	.org	0x1300
	CALL_EXCEPTION_HANDLER
	
	.org	0x1400
	CALL_EXCEPTION_HANDLER
	
	.org	0x1500
	CALL_EXCEPTION_HANDLER
	
	.org	0x1600
	CALL_EXCEPTION_HANDLER
	
	.org	0x1700
	CALL_EXCEPTION_HANDLER
	
	.org	0x1800
	CALL_EXCEPTION_HANDLER
	
	.org	0x1900
	CALL_EXCEPTION_HANDLER
	
	.org	0x1a00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1b00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1c00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1d00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1e00
	CALL_EXCEPTION_HANDLER
	
	.org	0x1f00
	CALL_EXCEPTION_HANDLER

	/* Pad to the end */
	.org	0x1ffc
	l.nop

/* -------------------------------------------------------------------------- */
/*!Main entry point

TODO: What is meant with the following? What is patched?
   >>We initialise the stack and frame pointer first, before we set up the
   caches, since otherwise we'll need to disable the instruction cache when
   patching the bus error vector code.<<

  This is the initialization code of the library. It performs these steps:

   * Initialize the stacks:
     Two stacks are configured: The system stack is used by the software and
     the exception stack is used when an exception occurs. We added this as
     this should be flexible with respect to the usage of virtual memory.

   * Activate the caches:
     If available the caches are initiliazed and activated.

   * Clear BSS:
     The BSS are essentially the uninitialized C variables. They are set to 0
     by default. This is performed by this function.

   * Initialize the impure data structure:
     Similarly, we need two library contexts, one for the normal software and
     one that is used during exceptions. The impure data structure holds
     the context information of the library. The called C function will setup
     both data structures. There is furthermore a pointer to the currently
     active impure data structure, which is initially set to the normal one.

   * Initialize constructors:
     Call the static and global constructors

   * Set up destructors to call from exit

   * Jump to main

   * Call exit after main returns

   * Loop forever
*/

/* -------------------------------------------------------------------------- */
	.section .text
	.global	_start
	.type	_start,@function

	/* Following externs from board-specific object passed at link time */
	.extern _board_mem_base
	.extern _board_mem_size
	.extern _board_uart_base
	
_start:
	/* Initialise stack and frame pointer (set to same value) */
	LOAD_SYMBOL_2_GPR(r1,_board_mem_base)
	l.lwz	r1,0(r1)
	LOAD_SYMBOL_2_GPR(r2,_board_mem_size)
	l.lwz	r2,0(r2)
	l.add	r1,r1,r2

	/* Store exception stack top address */
	LOAD_SYMBOL_2_GPR(r3,exception_stack_top)
	l.sw    0(r3),r1

	/* Store exception stack bottom address */
	// calculate bottom address
	// r3 = *exception stack size
	LOAD_SYMBOL_2_GPR(r3,exception_stack_size)
	// r3 = exception stack size
	l.lwz   r3,0(r3)
	// r4 = exception stack top - exception stack size = exception stack bottom
	l.sub   r4,r1,r3
	// r5 = *exception stack bottom
	LOAD_SYMBOL_2_GPR(r5,exception_stack_bottom)
	// store
	l.sw    0(r5),r4

	// Move stack pointer accordingly
	l.or    r1,r0,r4
	l.or	r2,r1,r1

	/* Store stack top address */
	LOAD_SYMBOL_2_GPR(r3,stack_top)
	l.sw	0(r3),r1

	/* Store stack bottom address */
	// calculate bottom address
	// r3 = stack size
	LOAD_SYMBOL_2_GPR(r3,stack_size)
	l.lwz   r3,0(r3)
	// r4 = stack top - stack size = stack bottom
	// -> stack bottom
	l.sub   r4,r1,r3
	// r5 = *exception stack bottom
	LOAD_SYMBOL_2_GPR(r5,stack_bottom)
	// store to variable
	l.sw	0(r5),r4

	/* Initialise cache */
	/* TODO - potentially make this optional for simulation targets to save
	time during startup */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal 	or1k_cache_init))

	/* Clear BSS */
.L10:
	LOAD_SYMBOL_2_GPR(r3,__bss_start)
	LOAD_SYMBOL_2_GPR(r4,end)

.L1:    l.sw    (0)(r3),r0
        l.sfltu r3,r4
        OR1K_DELAYED(
        OR1K_INST(l.addi  r3,r3,4),
        OR1K_INST(l.bf    .L1)
        )

	/* Reinitialize the reentrancy structure */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal	__impure_init))

	/* Call global and static constructors */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal	_init))
	
	/* Set up destructors to be called from exit if main ever returns */
	l.movhi	r3,hi(_fini)
        OR1K_DELAYED(
	OR1K_INST(l.ori	r3,r3,lo(_fini)),
	OR1K_INST(l.jal	atexit)
        )

	/* Check if UART is to be initialised */
	l.movhi	r4,hi(_board_uart_base)
	l.ori	r4,r4,lo(_board_uart_base)
	l.lwz	r4,0(r4)
	l.sfne	r4,r0		/* Is base set? If not, no UART */
	l.bnf	.L2
	l.or	r3,r0,r0
	OR1K_DELAYED_NOP(OR1K_INST(l.jal	__uart_init))

.L2:	
	/* Jump to main program entry point (argc = argv = envp = 0) */
	l.or	r3,r0,r0
	l.or	r4,r0,r0
        OR1K_DELAYED(
	OR1K_INST(l.or	r5,r0,r0),
	OR1K_INST(l.jal   main)
        )

	/* If program exits, call exit routine */
        OR1K_DELAYED(
	OR1K_INST(l.addi  r3,r11,0),
	OR1K_INST(l.jal   exit)
        )

	/* Loop forever */
.L3:	OR1K_DELAYED_NOP(OR1K_INST(l.j	.L3))
	
	.size	_start, .-_start
