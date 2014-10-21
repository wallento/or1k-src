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

/* -------------------------------------------------------------------------- */
/* Coding convention:
   Assembly is hard to read per se, so please follow the following coding
   conventions to keep it consistent and ease reading:
    * internal jump labels start with L, no identation
    * assemble lines have one tab identation
    * attributes (.section, .global, ..) are indented with one tab
    * code is structured using tabs, i.e., use 'l.sw\t0(r1),r1' with a single
      tab. libgloss assumes 8 space tab width, so that might look unstructured
      with tab widths below 6. Nevertheless don't use spaces or two tabs.
    * no space after comma
    * use the defined macros if possible as they reduce errors
    * use OR1K_INST with OR1K_DELAYED(_NOP)
    * OR1K_DELAYED is multiline for better readability, the inner parts are
      indented with another tab.
    * COMMENT! Try to accompy every line with a meaningful comment. If possible
      use pseudo code to describe the code. Also mention intentions and not only
      the obvious things..                                                    */
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
	.global stack_top	/* points to the next address after the stack */
	.global stack_bottom	/* points to the last address in the stack */
	.global stack_size	/* reserved stack size */
	.global exception_stack_top
	.global exception_stack_bottom
	.global exception_stack_size

stack_top:	.space 4,0
stack_bottom: .space 4,0
stack_size:   .word  STACK_SIZE /*!< Reserved stack space in bytes. */
exception_stack_top:    .space 4,0
exception_stack_bottom: .space 4,0
exception_stack_size:   .word  EXCEPTION_STACK_SIZE

#ifdef OR1K_MULTICORE
// Pointers to arrays of pointers, configured in _or1k_multicore_init
	.section .data
	.global stack_core
	.global exception_stack_core
stack_core:		.space 4, 0
exception_stack_core:	.space 4, 0
#endif

/* -------------------------------------------------------------------------- */
/*!Macro to load a symbol's address into a register.

  @param[in] gpr  General purpose register to load address into.
  @param[in] symbol Name of symbol to load.                                   */
/* -------------------------------------------------------------------------- */
#define LOAD_SYMBOL_2_GPR(gpr,symbol)  \
	.global symbol ;                   \
	l.movhi	gpr,hi(symbol) ;           \
	l.ori	gpr,gpr,lo(symbol)

#define SHADOW_REG(x) (SPR_GPR_BASE + 32 + x)

/* -------------------------------------------------------------------------- */
/*!Macro to handle exceptions.

  Load NPC into r3, EPCR into r4
                                                                              */
/* -------------------------------------------------------------------------- */
#ifndef OR1K_MULTICORE
#define CALL_EXCEPTION_HANDLER                     \
	/* store current stack pointer to address 4 */ \
	l.sw	0x4(r0), r1;                           \
	/* Load top of the exception stack */          \
	l.movhi	r1,hi(exception_stack_top);            \
	l.ori	r1,r1,lo(exception_stack_top);         \
	l.lwz	r1,0(r1);                              \
	/* Reserve red zone and context space */       \
	l.addi	r1,r1,-EXCEPTION_STACK_FRAME;          \
	/* Store GPR3 in context */                    \
	l.sw	0x4(r1),r3;                            \
	/* Load back software's stack pointer */       \
	l.lwz	r3,0x4(r0);                            \
	/* Store this in the context */                \
	l.sw	0x78(r1),r3;                           \
	/* Store GPR4 in the context */                \
	l.sw	0x8(r1),r4;                            \
	/* Copy the current program counter as first */\
	/* argument for the exception handler. This */ \
	/* is then used to determine the exception. */ \
	l.mfspr	r3,r0,SPR_NPC;                         \
	OR1K_DELAYED(                                  \
	/* Copy program counter of exception as */     \
	/* second argument to the exception handler */ \
		OR1K_INST(l.mfspr r4,r0,SPR_EPCR_BASE),    \
	/* Jump to exception handler. This will rfe */ \
		OR1K_INST(l.j or1k_exception_handler)      \
	)
#else
#define CALL_EXCEPTION_HANDLER					\
	/* store current stack pointer to SPR_ISR0 */		\
	l.mtspr	r0, r1, SHADOW_REG(1);				\
	/* store current GPR3 as we need it */			\
	l.mtspr	r0, r3, SHADOW_REG(2);				\
	/* Load pointer to exception stack array */		\
	l.movhi	r1, hi(exception_stack_core);			\
	l.ori	r1, r1, lo(exception_stack_core);		\
	l.lwz	r1, 0(r1);					\
	/* Get core id */					\
	l.mfspr	r3, r0, SPR_COREID;				\
	/* Calculate offset in array */				\
	l.slli	r3, r3, 2;					\
	/* Load value from array to stack pointer */		\
	l.add	r1, r1, r3;					\
	l.lwz	r1, 0(r1);					\
	/* Reserve red zone and context space */		\
	l.addi	r1, r1, -EXCEPTION_STACK_FRAME;			\
	/* Load back software's stack pointer */		\
	l.mfspr	r3, r0, SHADOW_REG(1);				\
	/* Store this in the context */				\
	l.sw	0x78(r1), r3;					\
	/* Load back GPR3 */					\
	l.mfspr	r3, r0, SHADOW_REG(2);				\
	/* Store this in the context */				\
	l.sw	0x4(r1), r3;					\
	/* Store GPR4 in the context */				\
	l.sw	0x8(r1), r4;					\
	/* Copy the current program counter as first */		\
	/* argument for the exception handler. This */		\
	/* is then used to determine the exception. */		\
	l.mfspr	r3, r0, SPR_NPC;				\
	OR1K_DELAYED(						\
	/* Copy program counter of exception as */		\
	/* second argument to the exception handler */		\
		OR1K_INST(l.mfspr r4, r0, SPR_EPCR_BASE),	\
	/* Jump to exception handler. This will rfe */		\
		OR1K_INST(l.j or1k_exception_handler)		\
	)
#endif

/* -------------------------------------------------------------------------- */
/*!Exception vectors                                                          */
/* -------------------------------------------------------------------------- */
	.section .vectors,"ax"

	/* 0x100: RESET exception */
	.org 0x100
__reset:
	l.movhi	r0,0
#ifdef OR1K_MULTICORE
	// This is a trick that relies on the fact, that all cores start at the same
	// time and they are similarily fast
	l.sw	0x4(r0), r0
	// Similarly, we use address 8 to signal how many cores have exit'ed
	l.sw	0x8(r0), r0
#endif
	l.movhi	r1,0
	l.movhi	r2,0
	l.movhi	r3,0
	l.movhi	r4,0
	l.movhi	r5,0
	l.movhi	r6,0
	l.movhi	r7,0
	l.movhi	r8,0
	l.movhi	r9,0
	l.movhi	r10,0
	l.movhi	r11,0
	l.movhi	r12,0
	l.movhi	r13,0
	l.movhi	r14,0
	l.movhi	r15,0
	l.movhi	r16,0
	l.movhi	r17,0
	l.movhi	r18,0
	l.movhi	r19,0
	l.movhi	r20,0
	l.movhi	r21,0
	l.movhi	r22,0
	l.movhi	r23,0
	l.movhi	r24,0
	l.movhi	r25,0
	l.movhi	r26,0
	l.movhi	r27,0
	l.movhi	r28,0
	l.movhi	r29,0
	l.movhi	r30,0
	l.movhi	r31,0

	/* Clear status register, set supervisor mode */
	l.ori	r1,r0,SPR_SR_SM
	l.mtspr	r0,r1,SPR_SR
	/* Clear timer mode register*/
	l.mtspr	r0,r0,SPR_TTMR
	/* Jump to program initialisation code */
	LOAD_SYMBOL_2_GPR(r4, _start)
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r4))

	.org 0x200
	CALL_EXCEPTION_HANDLER

	/* 0x300: Data Page Fault exception */
	.org 0x300
	CALL_EXCEPTION_HANDLER

	/* 0x400: Insn Page Fault exception */
	.org 0x400
	CALL_EXCEPTION_HANDLER

	/* 0x500: Timer exception */
	.org 0x500
	CALL_EXCEPTION_HANDLER

	/* 0x600: Aligment exception */
	.org 0x600
	CALL_EXCEPTION_HANDLER

	/* 0x700: Illegal insn exception */
	.org 0x700
	CALL_EXCEPTION_HANDLER

	/* 0x800: External interrupt exception */
	.org 0x800
	CALL_EXCEPTION_HANDLER

	/* 0x900: DTLB miss exception */
	.org 0x900
	CALL_EXCEPTION_HANDLER

	/* 0xa00: ITLB miss exception */
	.org 0xa00
	CALL_EXCEPTION_HANDLER

	/* 0xb00: Range exception */
	.org 0xb00
	CALL_EXCEPTION_HANDLER

	/* 0xc00: Syscall exception */
	.org 0xc00
	CALL_EXCEPTION_HANDLER

	/* 0xd00: Floating point exception */
	.org 0xd00
	CALL_EXCEPTION_HANDLER

	/* 0xe00: Trap exception */
	.org 0xe00
	CALL_EXCEPTION_HANDLER

	/* 0xf00: Reserved exceptions */
	.org 0xf00
	CALL_EXCEPTION_HANDLER

	.org 0x1000
	CALL_EXCEPTION_HANDLER

	.org 0x1100
	CALL_EXCEPTION_HANDLER

	.org 0x1200
	CALL_EXCEPTION_HANDLER

	.org 0x1300
	CALL_EXCEPTION_HANDLER

	.org 0x1400
	CALL_EXCEPTION_HANDLER

	.org 0x1500
	CALL_EXCEPTION_HANDLER

	.org 0x1600
	CALL_EXCEPTION_HANDLER

	.org 0x1700
	CALL_EXCEPTION_HANDLER

	.org 0x1800
	CALL_EXCEPTION_HANDLER

	.org 0x1900
	CALL_EXCEPTION_HANDLER

	.org 0x1a00
	CALL_EXCEPTION_HANDLER

	.org 0x1b00
	CALL_EXCEPTION_HANDLER

	.org 0x1c00
	CALL_EXCEPTION_HANDLER

	.org 0x1d00
	CALL_EXCEPTION_HANDLER

	.org 0x1e00
	CALL_EXCEPTION_HANDLER

	.org 0x1f00
	CALL_EXCEPTION_HANDLER

	/* Pad to the end */
	.org 0x1ffc
	l.nop

/* -------------------------------------------------------------------------- */
/*!Main entry point

  This is the initialization code of the library. It performs these steps:

   * Call early board initialization:
     Before anything happened, the board support may do some very early
     initialization. This is at maximum some very basic stuff that would
     otherwise prevent the following code from functioning. Other initialization
     of peripherals etc. is done later (before calling main).
     See the description below and README.board for details.

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

   * Initialize or1k support library reentrant data structures

   * Initialize constructors:
     Call the static and global constructors

   * Set up destructors to call from exit
     The library will call the function set via atexit() during exit(). We set
     it to call the _fini function which performs destruction.

   * Call board initialization:
     The board initialization can perform board specific initializations such as
     configuring peripherals etc.

   * Jump to main
     Call main with argc = 0 and *argv[] = 0

   * Call exit after main returns
     Now we call exit()

   * Loop forever
     We are dead.
*/

/* -------------------------------------------------------------------------- */
	.section .text
	.global  _start
	.type    _start,@function

	/* Following externs from board-specific object passed at link time */
	.extern _board_mem_base
	.extern _board_mem_size
	.extern _board_uart_base

	/* The early board initialization may for example read the memory size and
	   set the mem_base and mem_size or do some preliminary board
	   initialization. As we do not have a stack at this time, the function may
	   not use the stack (and therefore be a or call a C function. But it can
	   safely use all registers.

	   We define a default implementation, which allows board files in C. As
	   described above, this can only be used in assembly (board_*.S) as at
	   the early stage not stack is available. A board that needs early
	   initialization can overwrite the function with .global _board_init_early.

	   Recommendation: Only use when you really need it! */
	.weak _board_init_early
_board_init_early:
	OR1K_DELAYED_NOP(OR1K_INST(l.jr r9))

	/* The board initialization is then called after the C library and UART are
	   initialized. It can then be used to configure UART or other devices
	   before the actual main function is called. */
	.extern _board_init

_start:
#ifdef OR1K_MULTICORE
	// It is good to initialize and enable the caches before we do anything,
	// otherwise the cores will continuously access the bus during the wait time
	// for the boot barrier (0x4).
	// Fortunately or1k_cache_init does not need a stack!
	/* Initialise cache */
	/* TODO - potentially make this optional for simulation targets to save
	time during startup */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal     or1k_cache_init))

	// All but core 0 wait here
	l.mfspr	r1, r0, SPR_COREID
	l.sfeq	r1, r0
	OR1K_DELAYED(
		OR1K_INST(l.or r1, r0, r0),
		OR1K_INST(l.bf .Lcore0)
	)
.Lspin:
	l.sfeq r1, r0
	OR1K_DELAYED(
		OR1K_INST(l.lwz r1, 0x4(r0)),
		OR1K_INST(l.bf  .Lspin)
	)

	LOAD_SYMBOL_2_GPR(r2, stack_core)
	l.lwz	r2, 0(r2)
	l.mfspr	r1, r0, SPR_COREID
	l.slli	r1, r1, 2
	l.add	r2, r2, r1
	l.lwz	r1, 0(r2)
	l.or	r2, r1, r1

	OR1K_DELAYED_NOP(OR1K_INST(l.j .Linit_done));
.Lcore0:
#endif
	/* Call early board initialization */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal _board_init_early))

	/* Initialise stack and frame pointer (set to same value) */
	LOAD_SYMBOL_2_GPR(r1,_board_mem_base)
	l.lwz	r1,0(r1)
	LOAD_SYMBOL_2_GPR(r2,_board_mem_size)
	l.lwz	r2,0(r2)
	l.add	r1,r1,r2

	/* Store exception stack top address */
	LOAD_SYMBOL_2_GPR(r3,exception_stack_top)
	l.sw	0(r3),r1

	/* Store exception stack bottom address */
	// calculate bottom address
	// r3 = *exception stack size
	LOAD_SYMBOL_2_GPR(r3,exception_stack_size)
	// r3 = exception stack size
	l.lwz	r3,0(r3)
#ifdef OR1K_MULTICORE
	l.mfspr	r4, r0, SPR_NUMCORES
	l.mul	r3, r4, r3
#endif
	// r4 = exception stack top - exception stack size = exception stack bottom
	l.sub	r4,r1,r3
	// r5 = *exception stack bottom
	LOAD_SYMBOL_2_GPR(r5,exception_stack_bottom)
	// store
	l.sw	0(r5),r4

	// Move stack pointer accordingly
	l.or	r1,r0,r4
	l.or	r2,r1,r1

	/* Store stack top address */
	LOAD_SYMBOL_2_GPR(r3,stack_top)
	l.sw	0(r3),r1

	/* Store stack bottom address */
	// calculate bottom address
	// r3 = stack size
	LOAD_SYMBOL_2_GPR(r3,stack_size)
	l.lwz	r3,0(r3)
#ifdef OR1K_MULTICORE
	l.mfspr	r4, r0, SPR_NUMCORES
	l.mul	r3, r4, r3
#endif
	// r4 = stack top - stack size = stack bottom
	// -> stack bottom
	l.sub	r4,r1,r3
	// r5 = *exception stack bottom
	LOAD_SYMBOL_2_GPR(r5,stack_bottom)
	// store to variable
	l.sw	0(r5),r4

#ifndef OR1K_MULTICORE
	/* Initialise cache */
	/* TODO - potentially make this optional for simulation targets to save
	time during startup */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal or1k_cache_init))
#endif

	/* Clear BSS */
.Lclear_bss:
	LOAD_SYMBOL_2_GPR(r3,__bss_start)
	LOAD_SYMBOL_2_GPR(r4,end)

.Lclear_bss_loop:
	l.sw	(0)(r3),r0
	l.sfltu r3,r4
	OR1K_DELAYED(
		OR1K_INST(l.addi r3,r3,4),
		OR1K_INST(l.bf .Lclear_bss_loop)
	)

#ifdef OR1K_MULTICORE
	OR1K_DELAYED_NOP(OR1K_INST(l.jal _or1k_support_reent_init_core0))
#endif

	/* Reinitialize the reentrancy structure */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal __impure_init))

#ifdef OR1K_MULTICORE
	OR1K_DELAYED_NOP(OR1K_INST(l.jal _or1k_multicore_init))

	/* Reinitialize the or1k support library reentrancy structure */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal _or1k_support_reent_init))
#endif

	/* Call global and static constructors */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal _init))

	/* Set up destructors to be called from exit if main ever returns */
	l.movhi	r3,hi(_fini)
	OR1K_DELAYED(
		OR1K_INST(l.ori r3,r3,lo(_fini)),
		OR1K_INST(l.jal atexit)
	)

	/* Check if UART is to be initialised */
	LOAD_SYMBOL_2_GPR(r4,_board_uart_base)
	l.lwz	r4,0(r4)
	/* Is base set? If not, no UART */
	l.sfne	r4,r0
	l.bnf	.Lskip_uart
	l.or	r3,r0,r0
	OR1K_DELAYED_NOP(OR1K_INST(l.jal __uart_init))

.Lskip_uart:
	/* Board initialization */
	OR1K_DELAYED_NOP(OR1K_INST(l.jal _board_init))

#ifdef OR1K_MULTICORE
	// Start other cores
	l.ori	r3, r0, 1
	l.sw	0x4(r0), r3
#endif

.Linit_done:
	/* Jump to main program entry point (argc = argv = envp = 0) */
	l.or	r3,r0,r0
	l.or	r4,r0,r0
	OR1K_DELAYED(
		OR1K_INST(l.or r5,r0,r0),
		OR1K_INST(l.jal main)
	)

#ifdef OR1K_MULTICORE
.incrementexit:
	l.lwa	r3, 0x8(r0)
	l.addi	r3, r3, 1
	l.swa	0x8(r0), r3
	OR1K_DELAYED_NOP(OR1K_INST(l.bnf .incrementexit));
	l.mfspr	r4, r0, SPR_NUMCORES
	l.sfeq	r3, r4
	OR1K_DELAYED_NOP(OR1K_INST(l.bf .exitcorelast));
	OR1K_DELAYED_NOP(OR1K_INST(l.j _exit));
.exitcorelast:
#endif
	/* If program exits, call exit routine */
	OR1K_DELAYED(
		OR1K_INST(l.addi r3,r11,0),
		OR1K_INST(l.jal exit)
	)

	/* Loop forever */
.Lloop_forever:
	OR1K_DELAYED_NOP(OR1K_INST(l.j .Lloop_forever))

	.size _start,.-_start
