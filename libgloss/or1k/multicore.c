/* multicore.c Multicore support.

   Copyright (C) 2014, Authors

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

#include <stdlib.h>
#include <stdint.h>
#include <or1k-support.h>

extern uint32_t *stack_core;
extern uint32_t *exception_stack_core;

extern uint32_t stack_top;
extern uint32_t stack_size;
extern uint32_t exception_stack_top;
extern uint32_t exception_stack_size;

extern void _or1k_init_reent_multicore(void);

void _or1k_multicore_init(void) {
    int c;
    _or1k_init_reent_multicore();

    // Initialize stacks
    stack_core = _sbrk_r(_impure_ptr, 4 * or1k_numcores());
    exception_stack_core = _sbrk_r(_impure_ptr, 4 * or1k_numcores());

    stack_core[0] = stack_top;
    exception_stack_core[0] = exception_stack_top;

    for (c = 1; c < or1k_numcores(); c++) {
        stack_core[c] = stack_core[c-1] - stack_size;
        exception_stack_core[c] = exception_stack_core[c-1] -
                                  exception_stack_size;
    }

}
