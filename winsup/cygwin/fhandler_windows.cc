/* fhandler_windows.cc: code to access windows message queues.

   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2009, 2011, 2012
   Red Hat, Inc.

   Written by Sergey S. Okhapkin (sos@prospect.com.ru).
   Feedback and testing by Andy Piper (andyp@parallax.co.uk).

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <wingdi.h>
#include <winuser.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "sigproc.h"
#include "thread.h"


/*
The following unix-style calls are supported:

	open ("/dev/windows", flags, mode=0)
		- create a unix fd for message queue.

	read (fd, buf, len)
		- return next message from queue. buf must point to MSG
		  structure, len must be >= sizeof (MSG). If read is set to
		  non-blocking and the queue is empty, read call returns -1
		  immediately with errno set to EAGAIN, otherwise it blocks
		  untill the message will be received.

	write (fd, buf, len)
		- send a message pointed by buf. len argument ignored.

	ioctl (fd, command, *param)
		- control read()/write() behavior.
		ioctl (fd, WINDOWS_POST, NULL): write() will PostMessage();
		ioctl (fd, WINDOWS_SEND, NULL): write() will SendMessage();
		ioctl (fd, WINDOWS_HWND, &hWnd): read() messages for
			hWnd window.

	select () call marks read fd when any message posted to queue.
*/

fhandler_windows::fhandler_windows ()
  : fhandler_base (), hWnd_ (NULL), method_ (WINDOWS_POST)
{
}

int
fhandler_windows::open (int flags, mode_t)
{
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  close_on_exec (true);
  set_open_status ();
  return 1;
}

ssize_t __stdcall
fhandler_windows::write (const void *buf, size_t)
{
  MSG *ptr = (MSG *) buf;

  if (method_ == WINDOWS_POST)
    {
      if (!PostMessageW (ptr->hwnd, ptr->message, ptr->wParam, ptr->lParam))
	{
	  __seterrno ();
	  return -1;
	}
    }
  else if (!SendNotifyMessageW (ptr->hwnd, ptr->message, ptr->wParam,
				ptr->lParam))
    {
      __seterrno ();
      return -1;
    }
  return sizeof (MSG);
}

void __reg3
fhandler_windows::read (void *buf, size_t& len)
{
  MSG *ptr = (MSG *) buf;

  if (len < sizeof (MSG))
    {
      set_errno (EINVAL);
      len = (size_t) -1;
      return;
    }

  HANDLE w4[3] = { get_handle (), };
  set_signal_arrived here (w4[1]);
  DWORD cnt = 2;
  if ((w4[cnt] = pthread::get_cancel_event ()) != NULL)
    ++cnt;
  for (;;)
    {
      switch (MsgWaitForMultipleObjectsEx (cnt, w4,
					   is_nonblocking () ? 0 : INFINITE,
					   QS_ALLINPUT | QS_ALLPOSTMESSAGE,
					   MWMO_INPUTAVAILABLE))
	{
	case WAIT_OBJECT_0:
	  if (!PeekMessageW (ptr, hWnd_, 0, 0, PM_REMOVE))
	    {
	      len = (size_t) -1;
	      __seterrno ();
	    }
	  else if (ptr->message == WM_QUIT)
	    len = 0;
	  else
	    len = sizeof (MSG);
	  break;
	case WAIT_OBJECT_0 + 1:
	  if (_my_tls.call_signal_handler ())
	    continue;
	  len = (size_t) -1;
	  set_errno (EINTR);
	  break;
	case WAIT_OBJECT_0 + 2:
	  pthread::static_cancel_self ();
	  break;
	case WAIT_TIMEOUT:
	  len = (size_t) -1;
	  set_errno (EAGAIN);
	  break;
	default:
	  len = (size_t) -1;
	  __seterrno ();
	  break;
	}
      break;
    }
}

int
fhandler_windows::ioctl (unsigned int cmd, void *val)
{
  switch (cmd)
    {
    case WINDOWS_POST:
    case WINDOWS_SEND:
      method_ = cmd;
      break;
    case WINDOWS_HWND:
      if (val == NULL)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      hWnd_ = * ((HWND *) val);
      break;
    default:
      return fhandler_base::ioctl (cmd, val);
    }
  return 0;
}

void
fhandler_windows::set_close_on_exec (bool val)
{
  if (get_handle ())
    fhandler_base::set_close_on_exec (val);
  else
    fhandler_base::close_on_exec (val);
  void *h = hWnd_;
  if (h)
    set_no_inheritance (h, val);
}

void
fhandler_windows::fixup_after_fork (HANDLE parent)
{
  if (get_handle ())
    fhandler_base::fixup_after_fork (parent);
  void *h = hWnd_;
  if (h)
    fork_fixup (parent, h, "hWnd_");
}
