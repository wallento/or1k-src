/* or1k-impure.c. Handling of re-entrancy data structure.

   Copyright (C) 2011, Embecosm Limited <info@embecosm.com>
   Copyright (C) 2014, Authors
 
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
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
   with this program.  If not, see <http:#www.gnu.org/licenses/>.             */
/* -------------------------------------------------------------------------- */
/* This program is commented throughout in a fashion suitable for processing
   with Doxygen.                                                              */
/* -------------------------------------------------------------------------- */


#include <reent.h>

/* As an exception handler may also use the library, it is better to use
 * a different re-entrancy data structure for the exceptions.
 * This data structure is configured here and as part of the exception
 * handler (or1k_exception_handler) temporarily replaces the software's
 * impure data pointer.
 *
 * During initialization, the libraries standard _impure_data and the exception
 * impure data (_exception_impure_data) are initialized. Afterwards,
 * the current value _current_impure_ptr is set to _impure_ptr.
 *
 * At runtime __getreent is called to return the current reentrancy pointer,
 * which is stored in _current_impure_ptr.
 *
 * In the or1k_exception_handler the _current_impure_ptr is set to point to
 * _exception_impure_ptr. After the exception handler returned, it is set back
 * to _impure_ptr.
 */

/* Link in the external impure_data structure */
extern struct _reent *__ATTRIBUTE_IMPURE_PTR__ _impure_ptr;

/* Create exception impure data structure */
static struct _reent _exception_impure_data = _REENT_INIT (_exception_impure_data);

/* Link to the exception impure data structure */
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _exception_impure_ptr = &_exception_impure_data;

/* Link to the currently used data structure. */
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _current_impure_ptr;

void
__impure_init (void)
{
    // Initialize both impure data structures
    _REENT_INIT_PTR (_impure_ptr);
    _REENT_INIT_PTR (_exception_impure_ptr);

    // Set current to standard impure pointer
    _current_impure_ptr = _impure_ptr;
}	/* __impure_init () */

struct _reent * __getreent(void) {
    // Return the current one. This is set initially above, and during runtime
    // at or1k_exception_handler
    return _current_impure_ptr;
}
