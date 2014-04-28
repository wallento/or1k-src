/* malloc-lock.c. Lock malloc.

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

#include "include/or1k-support.h"
#include <reent.h>
#include <spr-defs.h>

/* Lock calls from different threads (addresses of reent structure),
   but allows recursive calls from the same thread. We do not only lock, but
   makes the malloc itself atomic. This is necessary as it would otherwise
   lead to a deadlock to interrupt while in malloc and then call it from
   an exception. But as we want the exceptions to be flexible to use
   all library calls and especially memory management, this is necessary. */

volatile uint32_t _malloc_lock_own; /*!< Current lock owner */
volatile uint32_t _malloc_lock_cnt; /*!< Current lock count (recursive lock!) */
volatile uint32_t _malloc_lock_restore_timer; /*!< TEE flag at lock time */
volatile uint32_t _malloc_lock_restore_interrupts; /*!< IEE flag at lock time */

/**
 * Recursive lock of the malloc
 */
void __malloc_lock(struct _reent *ptr) {
    uint32_t timerenable, interruptenable;
    uint32_t id, old_owner;

    // Disable timer and interrupt exception, store TEE and IEE flag temporarily
    // to restore them later on unlock
    timerenable = or1k_timer_disable();
    interruptenable = or1k_interrupts_disable();

    // Now we cannot be disturbed by an interrupt or timer exception

    id = (uint32_t) ptr; // Our id is the address of the reentrancy data

    do {
        do {
            // Read current lock owner
            old_owner = or1k_sync_ll(&_malloc_lock_own);
            // .. and spin while it is locked and we are not the owner
        } while (old_owner && (old_owner != id));
    } while (!or1k_sync_sc(&_malloc_lock_own, id));

    // Store the TEE and IEE flags for later restore
    _malloc_lock_restore_timer = timerenable;
    _malloc_lock_restore_interrupts = interruptenable;

    // Increment counter. The lock may be accessed recursively
    _malloc_lock_cnt++;

    return;
}

void __malloc_unlock(struct _reent *ptr) {
    // Decrement counter. The lock may be unlocked recursively
    _malloc_lock_cnt--;

    // If this was the last recursive unlock call
    if(_malloc_lock_cnt == 0){
        // reset owner
        _malloc_lock_own = 0;
        // Restore IEE flag
         or1k_interrupts_restore(_malloc_lock_restore_interrupts);
         // Restore TEE flag
         or1k_timer_restore(_malloc_lock_restore_timer);
    }

    return;
}
