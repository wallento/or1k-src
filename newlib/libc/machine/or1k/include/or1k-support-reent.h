/* OR1K support reentrancy

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

#ifndef __OR1K_REENT_H__
#define __OR1K_REENT_H__

#include "or1k-support.h"
#include <stdint.h>

/*!
 * Reentrancy data structure
 *
 * The or1k support library has a certain state on each core. This is captured
 * in this reentrancy data structure.
 */
struct _or1k_support_reent {
    /*! Pointer to the table of interrupt handlers */
    or1k_interrupt_handler_fptr **or1k_interrupt_handler_table;
    /*! Pointer to the interrupt handler data table */
    void ***or1k_interrupt_handler_data_ptr_table;
    /*! Pointer to the table of exception handlers */
    or1k_exception_handler_fptr **or1k_exception_handler_table;

    /*! Tick timer variable */
    uint32_t or1k_timer_ticks;
    /*! Tick timer mode */
    uint32_t or1k_timer_mode;
};

extern struct _or1k_support_reent *_or1k_support_getreent(void);

#endif
