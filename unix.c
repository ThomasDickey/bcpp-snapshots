/******************************************************************************
 * Copyright 1995 by Thomas E. Dickey.  All Rights Reserved.                  *
 *                                                                            *
 * You may freely copy or redistribute this software, so long as there is no  *
 * profit made from its use, sale trade or reproduction. You may not change   *
 * this copyright notice, and it must be included in any copy made.           *
 ******************************************************************************/
#ifndef NO_IDENT
static char *Id = "$Id: unix.c,v 5.6 1995/03/15 00:12:22 tom Exp $";
#endif

#include "conflict.h"

void	blip(c)
	int	c;
{
	static	char	text[] = "?";
	text[0] = (char)c;
	write(fileno(stderr), text, 1);
}

char *	fleaf(name)
	char	*name;
{
	char	*leaf = strrchr(name, PATHNAME_SEP);
	if (leaf != 0)
		leaf++;
	else
		leaf = name;
	return leaf;
}

char *
ftype (name)
	char	*name;
{
	char	*leaf = fleaf(name);
	char	*type = strrchr(leaf, '.');
	if (type == 0)
		type = leaf + strlen(leaf);
	return type;
}

#if !USE_INODE && !HAVE_REALPATH
char	*my_realpath(given, actual)
	char	*given;
	char	*actual;
{
	char	current[MAXPATHLEN];
	char	*extra = 0;
	char	*result = 0;

	if (getwd(current) != 0) {
		struct	stat	sb;

		if (stat(strcpy(actual, given), &sb) >= 0) {
			if ((sb.st_mode & S_IFMT) != S_IFDIR) {
				extra = fleaf(given);
				if (extra == given) {
					(void) strcpy(actual, current);
				} else {
					actual[(int)(extra-given)] = EOS;
				} 	
			}
			if (!strcmp(actual, current)) {
				result = actual;
			} else if (chdir(actual) >= 0) {
				if (getwd(actual) != 0) {
					result = actual;
				}
				(void)chdir(current);
			}
			if (result != 0 && extra != 0) {
				char *s = actual + strlen(actual);
				*s++ = PATHNAME_SEP;
				(void)strcpy(s, extra);
			}
		}
	}
	return result;
}
#endif

#if !HAVE_STRDUP
char	*my_strdup(s)
	char	*s;
{
	if (s != 0) {
		char	*t = malloc(strlen(s)+1);
		if (t != 0) {
			(void)strcpy(t, s);
		}
		s = t;
	}
	return s;
}
#endif
