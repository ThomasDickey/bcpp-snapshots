/******************************************************************************
 * Copyright 1995,2004 by Thomas E. Dickey.  All Rights Reserved.             *
 *                                                                            *
 * Permission to use, copy, modify, and distribute this software and its      *
 * documentation for any purpose and without fee is hereby granted, provided  *
 * that the above copyright notice appear in all copies and that both that    *
 * copyright notice and this permission notice appear in supporting           *
 * documentation, and that the name of the above listed copyright holder(s)   *
 * not be used in advertising or publicity pertaining to distribution of the  *
 * software without specific, written prior permission.                       *
 *                                                                            *
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD   *
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND  *
 * FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE  *
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES          *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN      *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR *
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.                *
 ******************************************************************************/

#ifndef NO_IDENT
static char *Id = "$Id: os2.c,v 6.2 2004/06/19 14:37:37 tom Exp $";
#endif

#include "conflict.h"

#if SYS_OS2
#include <dos.h>		/* ...for _getdrive/_setdrive */

int
set_drive(char *name)
{
    if (have_drive(name)) {
	unsigned want = name[0] - 'A' + 1;
	unsigned have;
	unsigned total;
	_dos_setdrive(want, &total);
	_dos_getdrive(&have);
	if (want != have)
	    return FALSE;
    }
    return TRUE;
}
#endif /* SYS_OS2 */
