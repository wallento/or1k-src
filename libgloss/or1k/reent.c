/* reent.c. Handling of re-entrancy data structure.

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
   with this program.  If not, see <http:#www.gnu.org/licenses/>.             */

#include <stdlib.h>
#include <sys/reent.h>

#ifdef OR1K_MULTICORE
#include <or1k-support.h>

#define CURRENT_IMPURE_PTR _current_impure_ptr_cores[or1k_coreid()]
#define IMPURE_PTR _impure_ptr_cores[or1k_coreid()]
#define EXCEPTION_IMPURE_PTR _exception_impure_ptr_cores[or1k_coreid()]
#else
#define CURRENT_IMPURE_PTR _current_impure_ptr
#define IMPURE_PTR _impure_ptr
#define EXCEPTION_IMPURE_PTR _exception_impure_ptr
#endif

extern struct _reent *__ATTRIBUTE_IMPURE_PTR__ _current_impure_ptr;
extern struct _reent *__ATTRIBUTE_IMPURE_PTR__ _impure_ptr;
extern struct _reent *__ATTRIBUTE_IMPURE_PTR__ _exception_impure_ptr;

#ifdef OR1K_MULTICORE
struct _reent **__ATTRIBUTE_IMPURE_PTR__ _impure_ptr_cores;
struct _reent **__ATTRIBUTE_IMPURE_PTR__ _exception_impure_ptr_cores;
struct _reent **__ATTRIBUTE_IMPURE_PTR__ _current_impure_ptr_cores;

void _or1k_init_reent_multicore(void) {
    int c;
    // Only core 0 calls this!
    size_t vector_size = sizeof(struct _reent*) * or1k_numcores();

    // Allocate arrays
    struct _reent **iv, **ev, **cv;
    iv = _sbrk_r(_impure_ptr, vector_size);
    ev = _sbrk_r(_impure_ptr, vector_size);
    cv = _sbrk_r(_impure_ptr, vector_size);

    // Now set arrays productive
    _impure_ptr_cores = iv;
    _exception_impure_ptr_cores = ev;
    _current_impure_ptr_cores = cv;

    // Core 0 array entries
    _impure_ptr_cores[0] = _impure_ptr;
    _exception_impure_ptr_cores[0] = _exception_impure_ptr;
    _current_impure_ptr_cores[0] = _impure_ptr;

    // Other cores' array entries
    for (c = 1; c < or1k_numcores(); c++) {
        _impure_ptr_cores[c] = _sbrk_r(_impure_ptr, sizeof(struct _reent));
        _REENT_INIT_PTR (_impure_ptr_cores[c] );
        _exception_impure_ptr_cores[c] = _sbrk_r(_impure_ptr, sizeof(struct _reent));
        _REENT_INIT_PTR (_exception_impure_ptr_cores[c] );
        _current_impure_ptr_cores[c] = _impure_ptr_cores[c];
    }
}
#endif

struct _reent *_or1k_getreent(void) {
    return CURRENT_IMPURE_PTR;
}

void _or1k_setreent_standard(void) {
    CURRENT_IMPURE_PTR = IMPURE_PTR;
}

void _or1k_setreent_exception(void) {
    CURRENT_IMPURE_PTR = EXCEPTION_IMPURE_PTR;
}
