/* or1k-support - OR1K CPU support functions
  
   Copyright (C) 2011, ORSoC AB
   Copyright (C) 2014, Authors
  
   Contributor Julius Baxter  <julius.baxter@orsoc.se>
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

#include "or1k-support.h"
#include "spr-defs.h"
#include <stdint.h>

/* Board-specific CPU clk HZ value */
extern unsigned long _board_clk_freq;

/* Tick timer variable */
volatile unsigned long or1k_timer_ticks;

/* Tick rate storage */
unsigned long or1k_timer_period;
uint32_t or1k_timer_mode;

void
or1k_interrupts_enable(void)
{
    uint32_t sr = or1k_mfspr(SPR_SR);
    sr |= SPR_SR_IEE;
    or1k_mtspr(SPR_SR, sr);
}

uint32_t
or1k_interrupts_disable(void)
{
    uint32_t oldsr, newsr;
    oldsr= or1k_mfspr(SPR_SR);
    newsr = oldsr & ~SPR_SR_IEE;
    or1k_mtspr(SPR_SR, newsr);
    return oldsr & SPR_SR_IEE;
}

void
or1k_interrupts_restore(uint32_t sr_iee)
{
    or1k_mtspr(SPR_SR, or1k_mfspr(SPR_SR) | (sr_iee & SPR_SR_IEE));
}

/* --------------------------------------------------------------------------*/
/*!Report a 32-bit value

   Uses the built-in simulator functionality.

   @param[in] value  Value to report.                                        */
/* --------------------------------------------------------------------------*/
void
or1k_report (unsigned long int  value)
{
  __asm__ __volatile__ ("l.addi\tr3,%0,0\n\t"
                        "l.nop %1": : "r" (value), "K" (NOP_REPORT));

}       /* report () */

/* --------------------------------------------------------------------------*/
/*!Write a SPR

   @todo Surely the SPR should be a short int, since it is only 16-bits. Left
         as is for now due to large amount of user code that might need
         changing.

   @param[in] spr    The SPR to write
   @param[in] value  The value to write to the SPR                           */
/* --------------------------------------------------------------------------*/
void
or1k_mtspr (unsigned long int  spr,
       unsigned long int  value)
{       
  __asm__ __volatile__ ("l.mtspr\t\t%0,%1,0": : "r" (spr), "r" (value));

}       /* mtspr () */

/* --------------------------------------------------------------------------*/
/*!Read a SPR

   @todo Surely the SPR should be a short int, since it is only 16-bits. Left
         as is for now due to large amount of user code that might need
         changing.

   @param[in] spr    The SPR to write

   @return  The value read from the SPR                                      */
/* --------------------------------------------------------------------------*/
unsigned long int
or1k_mfspr (unsigned long spr)
{       
  unsigned long value;

  __asm__ __volatile__ ("l.mfspr\t\t%0,%1,0" : "=r" (value) : "r" (spr));

  return value;

}       /* mfspr () */

/* --------------------------------------------------------------------------*/
/*!Pseudo-random number generator

   This should return pseudo-random numbers, based on a Galois LFSR

   @return The next pseudo-random number                                     */
/* --------------------------------------------------------------------------*/
unsigned long int
or1k_rand ()
{
  static unsigned long int lfsr = 1;
  static int period = 0;
  /* taps: 32 31 29 1; characteristic polynomial: x^32 + x^31 + x^29 + x + 1 */
  lfsr = (lfsr >> 1) ^ (unsigned long int)((0 - (lfsr & 1u)) & 0xd0000001u); 
  ++period;
  return lfsr;
}


/* --------------------------------------------------------------------------*/
/*!Tick timer interrupt handler

   Increment timer ticks counter, reload TTMR
                                                                             */
/* --------------------------------------------------------------------------*/
void 
or1k_timer_interrupt_handler(void)
{
  or1k_timer_ticks++;
  uint32_t ttmr = (or1k_mfspr(SPR_TTMR) & SPR_TTMR_PERIOD);
  or1k_mtspr(SPR_TTMR, ttmr | SPR_TTMR_IE | SPR_TTMR_RT);
}

/* --------------------------------------------------------------------------*/
/*!Enable tick timer    

   Install handler, calculate TTMR period, reset tick counter

   @param[in] hz     Rate at which to trigger timer ticks                    */
/* --------------------------------------------------------------------------*/
int
or1k_timer_init(unsigned int hz)
{
  uint32_t upr = or1k_mfspr(SPR_UPR);
  if ((upr & SPR_UPR_TTP) == 0) {
      return -1;
  }

  /* Set this, for easy access when reloading */
  uint32_t period = (_board_clk_freq/hz) & SPR_TTMR_PERIOD;
  or1k_timer_period = period;
  or1k_mtspr(SPR_TTMR, period);

  /* Reset timer tick counter */
  or1k_timer_ticks = 0;
  
  /* Install handler */
  or1k_exception_handler_add(0x5,or1k_timer_interrupt_handler);
  or1k_timer_mode = SPR_TTMR_RT;

  /* Reset counter register */
  or1k_mtspr(SPR_TTCR, 0);

  return 0;
}

void
or1k_timer_set_period(uint32_t hz)
{
    uint32_t period = (_board_clk_freq/hz) & SPR_TTMR_PERIOD;
    uint32_t ttmr = or1k_mfspr(SPR_TTMR);
    ttmr = (ttmr & ~SPR_TTMR_PERIOD) | period;
    or1k_mtspr(SPR_TTMR, ttmr);
    or1k_timer_period = period;
}

void
or1k_timer_set_handler(void (*handler)(void))
{
    or1k_exception_handler_add(0x5, handler);
}

void
or1k_timer_set_mode(uint32_t mode)
{
    // Store mode in variable
    or1k_timer_mode = mode;

    uint32_t ttmr = or1k_mfspr(SPR_TTMR);
    // If the timer is currently running, we also change the mode
    if ((ttmr & SPR_TTMR_M) != 0) {
        ttmr = (ttmr & ~SPR_TTMR_M) | mode;
    }
}

/* --------------------------------------------------------------------------*/
/*!Enable tick timer    

   Enable timer interrupt, install handler, load TTMR
                                                                             */
/* --------------------------------------------------------------------------*/
void 
or1k_timer_enable(void)
{
    uint32_t ttmr = or1k_mfspr(SPR_TTMR);
    ttmr |= SPR_TTMR_IE | or1k_timer_mode;
    or1k_mtspr(SPR_TTMR, ttmr);
    or1k_mtspr(SPR_SR, SPR_SR_TEE | or1k_mfspr(SPR_SR));
}

/* --------------------------------------------------------------------------*/
/*!Disable tick timer    

   Disable timer interrupt in SR
                                                                             */
/* --------------------------------------------------------------------------*/
uint32_t
or1k_timer_disable(void)
{
    uint32_t sr = or1k_mfspr(SPR_SR);
    or1k_mtspr(SPR_SR, ~SPR_SR_TEE & sr);
    return (sr & SPR_SR_TEE);
}

void
or1k_timer_restore(uint32_t sr_tee)
{
    or1k_mtspr(SPR_SR, or1k_mfspr(SPR_SR) | (sr_tee & SPR_SR_TEE));
}

void
or1k_timer_pause(void)
{
    or1k_mtspr(SPR_TTMR, or1k_mfspr(SPR_TTMR) & ~SPR_TTMR_M);
}

void
or1k_timer_reset(void)
{
    or1k_mtspr(SPR_TTMR, or1k_mfspr(SPR_TTMR) & ~SPR_TTMR_IP);
    or1k_mtspr(SPR_TTCR, 0);
}

/* --------------------------------------------------------------------------*/
/*!Get tick timer    

   Return value of tick timer
                                                                             */
/* --------------------------------------------------------------------------*/
unsigned long
or1k_timer_get_ticks(void)
{
  return or1k_timer_ticks;
}

/* --------------------------------------------------------------------------*/
/*!Reset tick timer    

   Clear value of tick timer
                                                                             */
/* --------------------------------------------------------------------------*/
void
or1k_timer_reset_ticks(void)
{
  or1k_timer_ticks = 0;
}
