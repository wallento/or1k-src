/* or1k-reent.c OpenRISC support library reentrancy support.

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

#include "or1k-support.h"
#include "or1k-support-reent.h"
#include <stdlib.h>
#include <string.h>

#ifdef OR1K_MULTICORE
#define NUMCORES or1k_numcores()
#define GET_OR1K_REENT _or1k_support_reent_ptr[or1k_coreid()]
#else
#define NUMCORES 1
#define GET_OR1K_REENT _or1k_support_reent_ptr[0]
#endif

struct _or1k_support_reent _or1k_support_reent_core0;
const struct _or1k_support_reent *_or1k_support_reent_core0_ptr = &_or1k_support_reent_core0;

extern or1k_interrupt_handler_fptr *or1k_interrupt_handler_table;
extern void **or1k_interrupt_handler_data_ptr_table;
extern or1k_exception_handler_fptr *or1k_exception_handler_table;

struct _or1k_support_reent **_or1k_support_reent_ptr;

void _or1k_support_reent_init_core0() {
    // First, we temporarily set the _or1k_support_reent_ptr to point to the
    // core 0 data structure, as the others are stopped..

    _or1k_support_reent_ptr = &_or1k_support_reent_core0_ptr;

    // Set core 0 handler tables
    _or1k_support_reent_core0.or1k_interrupt_handler_table = &or1k_interrupt_handler_table;
    _or1k_support_reent_core0.or1k_interrupt_handler_data_ptr_table = &or1k_interrupt_handler_data_ptr_table;
    _or1k_support_reent_core0.or1k_exception_handler_table = &or1k_exception_handler_table;
}

void _or1k_support_reent_init() {
    int c;

    // TODO: move this to newlib?
    _or1k_support_reent_ptr = _sbrk_r(_impure_ptr,
                                      sizeof(struct _or1k_support_reent*) * NUMCORES);

    _or1k_support_reent_ptr[0] = &_or1k_support_reent_core0;

    // Allocate on other cores and copy
    for (c = 1; c < NUMCORES; c++) {
        _or1k_support_reent_ptr[c] = _sbrk_r(_impure_ptr,
                                             sizeof(struct _or1k_support_reent));

        memset(_or1k_support_reent_ptr[c], 0,sizeof(struct _or1k_support_reent));

        _or1k_support_reent_ptr[c]->or1k_interrupt_handler_table = _sbrk_r(_impure_ptr, 4 * 32);
        _or1k_support_reent_ptr[c]->or1k_interrupt_handler_data_ptr_table = _sbrk_r(_impure_ptr, 4 * 32);
        _or1k_support_reent_ptr[c]->or1k_exception_handler_table = _sbrk_r(_impure_ptr, 4 * 29);

        memcpy(_or1k_support_reent_ptr[c]->or1k_interrupt_handler_table,
                _or1k_support_reent_ptr[0]->or1k_interrupt_handler_table, 4*32);
        memcpy(_or1k_support_reent_ptr[c]->or1k_interrupt_handler_data_ptr_table,
                _or1k_support_reent_ptr[0]->or1k_interrupt_handler_data_ptr_table, 4*32);
        memcpy(_or1k_support_reent_ptr[c]->or1k_exception_handler_table,
                _or1k_support_reent_ptr[0]->or1k_exception_handler_table, 4*29);
    }
}

struct _or1k_support_reent *_or1k_support_getreent(void) {
    return GET_OR1K_REENT;
}
