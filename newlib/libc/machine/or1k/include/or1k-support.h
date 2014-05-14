/* OR1K support defines
  
   Copyright (C) 2011, ORSoC AB
   Copyright (C) 2008, 2010 Embecosm Limited
   Copyright (C) 2014, Authors
  
   Contributor Julius Baxter  <julius.baxter@orsoc.se>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
   Contributor Stefan Wallentowitz <stefan.wallentowitz@tum.de>
  
   This file is part of Newlib.
  
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

/* This machine configuration matches the Or1ksim configuration file in this
   directory. */

#ifndef OR1K_NEWLIB_SUPPORT__H
#define OR1K_NEWLIB_SUPPORT__H

#include "or1k-support-defs.h"
#include <_ansi.h>
#include <stdint.h>

/*! External symbols from each board's object file */
extern unsigned long _board_clk_freq;

extern unsigned long _board_uart_base;
extern unsigned long _board_uart_baud;
extern unsigned long _board_uart_irq;

/*!
 * \defgroup or1k_macros OR1K macros
 * @{
 */

/*!
 * Check if board has UART
 *
 * Check if board has UART by testing for base address
 */
#define BOARD_HAS_UART (_board_uart_base)

/*!
 * Access byte-sized memory mapped register
 *
 * Used to access a byte-sized memory mapped register. It avoids usage errors
 * when not defining register addresses volatile and handles casting correctly.
 *
 * Example for both read and write:
 *
 * \code
 *   uint8_t status = REG8(IPBLOCK_STATUS_REG_ADDR);
 *   REG8(IPBLOCK_ENABLE) = 1;
 * \endcode
 *
 * \param add Register address
 */
#define REG8(add) *((volatile unsigned char *) (add))

/*!
 * Access halfword-sized memory mapped register
 *
 * Used to access a 16 byte-sized memory mapped register. It avoids usage errors
 * when not defining register addresses volatile and handles casting correctly.
 *
 * See REG8() for an example.
 *
 * \param add Register address
 */
#define REG16(add) *((volatile unsigned short *) (add))

/*!
 * Access word-sized memory mapped register
 *
 * Used to access a word-sized memory mapped register. It avoids usage errors
 * when not defining register addresses volatile and handles casting correctly.
 *
 * See REG8() for an example.
 *
 * \param add Register address
 */
#define REG32(add) *((volatile unsigned long *) (add))

/*!
 * @}
 */

/*!
 * \defgroup or1k_interrupts OR1K interrupt control
 *
 * Interrupt control function prototypes
 *
 * @{
 */

/*! Function pointer to interrupt handler functions */
typedef void (*or1k_interrupt_handler_fptr)(uint32_t data);

/*!
 * Add interrupt handler for interrupt line
 *
 * Registers a callback function for a certain interrupt line.
 *
 * \param line Interrupt line/id to register a handler for
 * \param handler Handler to register
 * \param data Data value passed to the handler
 */
void or1k_interrupt_handler_add(int line, or1k_interrupt_handler_fptr handler,
                                uint32_t data);

/*!
 * Enable interrupts from a given line
 *
 * Unmask the given interrupt line. It is also important to enable interrupts
 * in general, e.g., using or1k_interrupts_enable().
 *
 * \param line Interrupt line to enable
 */
void or1k_interrupt_enable(int line);

/*!
 * Disable interrupts from a given line
 *
 * Mask given interrupt line. It can be unmasked using or1k_interrupt_enable().
 *
 * \param line Interrupt line to disable
 */
void or1k_interrupt_disable(int line);

/*!
 * Disable interrupts
 *
 * This disables the interrupt exception. This is sufficient to disable all
 * interrupts. It does not change the mask register (which is modified using
 * or1k_interrupt_enable() and or1k_interrupt_disable()).
 *
 * The interrupt exception can be enabled using or1k_interrupts_enable().
 *
 * Finally, the status of the interrupt exception enable flag is returned by
 * this function. That allows to call this function even if interrupts are
 * already disabled. To restore the value of the interrupt exception enable
 * flag, use the or1k_interrupts_restore() function. That way you avoid to
 * accidentally enable interrupts. Example:
 *
 * \code
 *   void f() {
 *     uint32_t interrupt_status = or1k_interrupts_disable();
 *     // do something
 *     or1k_interrupts_restore(status);
 *   }
 * \endcode
 *
 * This code will preserve the original status of the interrupt enable flag.
 *
 * \return Interrupt exception enable flag before call
 */
uint32_t or1k_interrupts_disable(void);

/*!
 * Enable interrupt exception
 *
 * Enable the interrupt exception. Beside the interrupt exception, it is also
 * necessary to enable the individual interrupt lines using
 * or1k_interrupt_enable().
 *
 * You should avoid using this function together with or1k_interrupts_disable()
 * to guard atomic blocks as it unconditionally enables the interrupt
 * exception (see documentation of or1k_interrupts_disable()).
 */
void or1k_interrupts_enable(void);

/*!
 * Restore interrupt exception enable flag
 *
 * This function restores the given status to the processor.
 * or1k_interrupts_restore(0) is identical to or1k_interrupts_disable() and
 * or1k_interrupts_restore(SPR_SR_IEE) is identical to or1k_interrupts_enable().
 *
 * It is for example used to guard an atomic block and restore the original
 * status of the interrupt exception enable flag as returned by
 * or1k_interrupts_disable(). See the documentation of or1k_interrupts_disable()
 * for a usage example.
 *
 * \param status Status of the flag to restore
 */
void or1k_interrupts_restore(uint32_t status);

/*!
 * @}
 */

/*!
 * \defgroup or1k_exception Exception handling
 * @{
 */

/*! Function pointer to an exception handler function */
typedef void (*or1k_exception_handler_fptr)(void);

/*!
 * Register exception handler
 *
 * Register an exception handler for the given exception id. This handler is
 * in the following then called when the exception occurs. You can thereby
 * individually handle those exceptions.
 *
 * \param id Exception id
 * \param handler Handler callback
 */
void or1k_exception_handler_add(int id, or1k_exception_handler_fptr handler);

/*!
 * @}
 */

/*!
 * \defgroup or1k_spr SPR access
 */

/*!
 * Move value to special purpose register
 *
 * Move data value to a special purpose register
 *
 * \param spr SPR identifier, see spr-defs.h for macros
 * \param value value to move to SPR
 */
void  or1k_mtspr (unsigned long int  spr,
                  unsigned long int  value);

/*!
 * Copy value from special purpose register
 *
 * Copy a data value from the given special purpose register.
 *
 * \param spr SPR identifier, see spr-defs.h for macros
 * \return SPR data value
 */
unsigned long int  or1k_mfspr (unsigned long int  spr);

/*!
 * @}
 */

/*!
 * \defgroup or1k_util Miscellaneous utility functions
 *
 * @{
 */

/*!
 * Report value to simulator
 *
 * Uses the built-in simulator functionality.
 *
 * \param value Value to report
 */
void or1k_report (unsigned long int value);

/*!
 * Get (pseudo) random number
 *
 * This should return pseudo-random numbers, based on a Galois LFSR.
 *
 * \return (Pseudo) Random number
 */
unsigned long int or1k_rand(void);

/*!
 * @}
 */

/*!
 * \defgroup or1k_cache Cache control
 *
 * @{
 */

/*!
 * Enable instruction cache
 */
void or1k_icache_enable(void);

/*!
 * Disable instruction cache
 */
void or1k_icache_disable(void);

/*!
 * Flush instruction cache
 *
 * Invalidate instruction cache entry
 *
 * \param entry Entry to invalidate
 */
void or1k_icache_flush(unsigned long entry);

/*!
 * Enable data cache
 */
void or1k_dcache_enable(void);

/*!
 * Disable data cache
 */
void or1k_dcache_disable(void);

/*!
 * Flush data cache
 *
 * Invalidate data cache entry
 *
 * \param entry Entry to invalidate
 */
void or1k_dcache_flush(unsigned long entry);

/*!
 * @}
 */

/*!
 * \defgroup or1k_mmu MMU control
 * @{
 */

/*!
 * Enable instruction MMU
 */
void or1k_immu_enable(void);

/*!
 * Disable instruction MMU
 */
void or1k_immu_disable(void);

/*!
 * Enable data MMU
 */
void or1k_dmmu_enable(void);

/*!
 * Disable data MMU
 */
void or1k_dmmu_disable(void);

/*!
 * @}
 */

/*!
 * \defgroup or1k_timer Timer control
 *
 * The tick timer can be used for time measurement, operating system scheduling
 * etc. By default it is initialized to continuously count the ticks of a
 * certain period after calling or1k_timer_init(). The period can later be
 * changed using or1k_timer_set_period().
 *
 * The timer is controlled using or1k_timer_enable(), or1k_timer_disable(),
 * or1k_timer_restore(), or1k_timer_pause(). After initialization it is required
 * to enable the timer the first time using or1k_timer_enable().
 * or1k_timer_disable() only disables the tick timer interrupts, it does not
 * disable the timer counting. If you plan to use a pair of or1k_timer_disable()
 * and or1k_timer_enable() to protect sections of your code against interrupts
 * you should use or1k_timer_disable() and or1k_timer_restore(), as it may be
 * possible that the timer interrupt was not enabled before disabling it,
 * enable would then start it unconditionally. or1k_timer_pause() pauses the
 * counting.
 *
 * In the default mode you can get the tick value using or1k_timer_get_ticks()
 * and reset this value using or1k_timer_reset_ticks().
 *
 * Example for using the default mode:
 *
 * \code
 *   int main() {
 *     uint32_t ticks = 0;
 *     uint32_t timerstate;
 *     or1k_timer_init(100);
 *     or1k_timer_enable();
 *     while (1) {
 *       while (ticks == or1k_timer_get_ticks()) { }
 *       timerstate = or1k_timer_disable();
 *       // do something atomar
 *       or1k_timer_restore(timerstate);
 *       if (ticks == 100) {
 *         printf("A second elapsed\n");
 *         or1k_timer_reset_ticks();
 *         ticks = 0;
 *       }
 *     }
 *   }
 * \endcode
 *
 * It is possible to change the mode of the tick timer using
 * or1k_timer_set_mode(). Allowed values are the correct bit pattern (including
 * the bit positions) for the TTMR register, it is recommended to use the macros
 * defined in spr-defs.h. For example, implementing an operating system with
 * scheduling decisions of varying duration favors the implementation of single
 * run tick timer. Here, each quantum is started before leaving the operating
 * system kernel. The counter can be restarted with or1k_timer_reset().
 * Example:
 *
 * \code
 *   void tick_handler(void) {
 *     // Make schedule decision
 *     // and set new thread
 *     or1k_timer_reset();
 *     // End of exception, new thread will run
 *   }
 *
 *   int main() {
 *     // Configure operating system and start threads..
 *
 *     // Configure timer
 *     or1k_timer_init(50);
 *     or1k_timer_set_handler(&tick_handler);
 *     or1k_timer_set_mode(SPR_TTMR_SR);
 *     or1k_timer_enable();
 *
 *     // Schedule first thread and die..
 *   }
 *  \endcode
 *
 * @{
 */

/*!
 * Initialize tick timer
 *
 * This initializes the tick timer in default mode (see \ref or1k_timer for
 * details).
 *
 * \param hz Initial period of the tick timer
 * \return 0 if successful, -1 if timer not present
 */
int or1k_timer_init(unsigned int hz);

/*!
 * Set period of timer
 *
 * Set the period of the timer to a value in Hz. The frequency from the board
 * support package is used to determine the match value.
 */
void or1k_timer_set_period(uint32_t hz);

/*!
 * Replace the timer interrupt handler
 *
 * By default the tick timer is used to handle timer ticks. The user can replace
 * this with an own handler for example when implementing an operating system.
 *
 * \param handler The callback function pointer to the handler
 */
void or1k_timer_set_handler(void (*handler)(void));

/*!
 * Set timer mode
 *
 * The timer has different modes (see architecture manual). The default is to
 * automatically restart counting (SPR_TTMR_RT), others are single run
 * (SPR_TTMR_SR) and continuous run (SPR_TTMR_CR).
 *
 * \param mode a valid mode (use definitions from spr-defs.h as it is important
 *             that those are also at the correct position in the bit field!)
 */
void or1k_timer_set_mode(uint32_t mode);

/*!
 * Enable timer interrupt
 *
 * Enable the timer interrupt exception, independent of the status before.
 * If you want to enable the timer conditionally, for example to implement a
 * non-interruptible sequence of code, you should use or1k_timer_restore(). See
 * the description of or1k_timer_disable() for more details.
 *
 * The enable will also restore the mode if the timer was paused previously.
 */
void or1k_timer_enable(void);

/*!
 * Disable timer interrupt
 *
 * This disables the timer interrupt exception and returns the state of the
 * interrupt exception enable flag before the call. This can be used with
 * or1k_timer_restore() to implement sequences of code that are not allowed to
 * be interrupted. Using or1k_timer_enable() will unconditionally enable the
 * interrupt independent of the state before calling or1k_timer_disable().
 * For an example see \ref or1k_timer.
 *
 * \return Status of timer interrupt before call
 */
uint32_t or1k_timer_disable(void);

/*!
 * Restore timer interrupt exception flag
 *
 * Restores the timer interrupt exception flag as returned by
 * or1k_timer_disable(). See the description of or1k_timer_disable() and \ref
 * or1k_timer for details and an example.
 *
 * \param sr_tee Status of timer interrupt
 */
void or1k_timer_restore(uint32_t sr_tee);

/*!
 * Pause timer counter
 *
 * Pauses the counter of the tick timer. The counter will hold its current value
 * and it can be started again with or1k_timer_enable() which will restore the
 * configured mode.
 */
void or1k_timer_pause(void);

/*!
 * Reset timer counter
 */
void or1k_timer_reset(void);

/*!
 * Get timer ticks
 *
 * Get the global ticks of the default configuration. This will increment the
 * tick counter according to the preconfigured period.
 *
 * \return Current value of ticks
 */
unsigned long or1k_timer_get_ticks(void);

/*!
 * Reset timer ticks
 *
 * Resets the timer ticks in default configuration to 0.
 */
void or1k_timer_reset_ticks(void);

/*
 * @}
 */

#endif	/* OR1K_NEWLIB_SUPPORT__H */
