/******************************************************************************
 * Copyright 1995 by Thomas E. Dickey.  All Rights Reserved.                  *
 *                                                                            *
 * You may freely copy or redistribute this software, so long as there is no  *
 * profit made from its use, sale trade or reproduction. You may not change   *
 * this copyright notice, and it must be included in any copy made.           *
 ******************************************************************************/
#ifndef NO_IDENT
static char *Id = "$Id: os2.c,v 6.0 1995/03/12 02:02:26 dickey Rel $";
#endif

#include "conflict.h"

#if SYS_OS2
#include <dos.h>		/* ...for _getdrive/_setdrive */

int
set_drive (char *name)
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
#endif	/* SYS_OS2 */
