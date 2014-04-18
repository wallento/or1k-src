/* fstat.c Implementation of the _fstat syscall for newlib

   Copyright (C) 2004, Jacob Bower
   Copyright (C) 2010, Embecosm Limited <info@embecosm.com>
 
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
  
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

#include <errno.h>
#include <reent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "or1k-support.h"


/* -------------------------------------------------------------------------- */
/*!Status of an open file when using a UART

   We only know about stdin, stdout and stderr, and treat these as character
   special files. All other files are erroneous.

   @return  0 on success, with the struct stat appropriately updated. -1 on
            failure, with an error code in the global variable errno.         */
/* -------------------------------------------------------------------------- */
int
_fstat_r (struct _reent *reent,
          int           file,
          struct stat   *st)
{
  if ((BOARD_HAS_UART && (STDIN_FILENO  == file)) ||
      (STDERR_FILENO == file) ||
      (STDOUT_FILENO == file))
    {
      st->st_mode = S_IFCHR;
      return  0;
    }
  else
    {
      reent->_errno = EBADF;
      return  -1;
    }
}	/* _fstat () */
