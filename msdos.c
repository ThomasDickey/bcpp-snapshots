/******************************************************************************
 * Copyright 1995 by Thomas E. Dickey.  All Rights Reserved.                  *
 *                                                                            *
 * You may freely copy or redistribute this software, so long as there is no  *
 * profit made from its use, sale trade or reproduction. You may not change   *
 * this copyright notice, and it must be included in any copy made.           *
 ******************************************************************************/
#ifndef NO_IDENT
static char *Id = "$Id: msdos.c,v 6.0 1995/03/13 00:23:48 dickey Rel $";
#endif

/*
 * MSDOS (and some OS/2) support functions.
 */

#include "conflict.h"
#include <ctype.h>

char *
fleaf (char *name)
{
	char	*s;

	while ((s = strpbrk(name, ":/\\")) != 0) {
		name = s+1;
	}
	return name;
}

char *
ftype (char *name)
{
	char	*leaf = fleaf(name);
	char	*type = strrchr(leaf, '.');
	if (type == 0)
		type = leaf + strlen(leaf);
	return type;
}

void	blip(int c)
{
	putc(c, stderr);
}

int
have_drive (char *name)
{
	return (isalpha(name[0]) && name[1] == ':');
}

int
same_drive (char *a, char *b)
{
	return have_drive(a) && have_drive(b) && (*a == *b);
}

int
set_directory (char *name)
{
	return set_drive(name) && (chdir(name) >= 0);
}

#if SYS_MSDOS
#include <dos.h>		/* ...for _getdrive/_setdrive */

int
set_drive (char *name)
{
	if (have_drive(name)) {
		unsigned want = name[0] - 'A';
		unsigned have;
		bdos(0x0e, want, 0); 			/* set drive */
		have = (bdos(0x19, 0, 0) & 0xff);	/* get drive */
		if (want != have)
			return FALSE;
	}
	return TRUE;
}
#endif	/* SYS_MSDOS */

#ifndef HAVE_GETOPT
int	optind;
char	*optarg;

	/*
	 * This version of 'getopt()' uses either '/' or '-' for the switch
	 * character to look more like a native dos or os/2 application.
	 */
int	getopt(int argc, char **argv, char *opts)
{
	int	code = EOF;
	char	*s, *t;

	if (optind == 0) {
		optind = 1;
		optarg = "";
	}
	if (optind < argc) {
		s = argv[optind];
		if (*s == '/' || *s == '-') {
			optind++;
			code = '?';
			if (*++s != EOS) {
				t = strchr(opts, *s++);
				if (t != 0 && *t != ':') {
					code = *t++;
					if (*t == ':') {
						if (*s != EOS)
							optarg = s;
						else if (optind < argc)
							optarg = argv[optind++];
					} else if (*s != EOS) {
						optind--;
						while ((s[-1] = s[0]) != EOS)
							s++;
					}
				}
			}
		}
	}
	return code;
}
#endif

#if	!HAVE_REALPATH
char	*my_realpath(char *given, char *actual)
{
	return (_fullpath(actual, given, MAXPATHLEN));
}
#endif
