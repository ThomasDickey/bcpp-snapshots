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
static const char Id[] = "$Id: unix.c,v 6.2 2004/06/19 14:38:08 tom Exp $";
#endif

#include "conflict.h"

void
blip(int c)
{
    static char text[] = "?";
    text[0] = (char) c;
    write(fileno(stderr), text, 1);
}

char *
fleaf(char *name)
{
    char *leaf = strrchr(name, PATHNAME_SEP);
    if (leaf != 0)
	leaf++;
    else
	leaf = name;
    return leaf;
}

char *
ftype(char *name)
{
    char *leaf = fleaf(name);
    char *type = strrchr(leaf, '.');
    if (type == 0)
	type = leaf + strlen(leaf);
    return type;
}

#if !USE_INODE && !HAVE_REALPATH
char *
my_realpath(char *given, char *actual)
{
    char current[MAXPATHLEN];
    char *extra = 0;
    char *result = 0;

    if (getwd(current) != 0) {
	struct stat sb;

	if (stat(strcpy(actual, given), &sb) >= 0) {
	    if ((sb.st_mode & S_IFMT) != S_IFDIR) {
		extra = fleaf(given);
		if (extra == given) {
		    (void) strcpy(actual, current);
		} else {
		    actual[(int) (extra - given)] = EOS;
		}
	    }
	    if (!strcmp(actual, current)) {
		result = actual;
	    } else if (chdir(actual) >= 0) {
		if (getwd(actual) != 0) {
		    result = actual;
		}
		(void) chdir(current);
	    }
	    if (result != 0 && extra != 0) {
		char *s = actual + strlen(actual);
		*s++ = PATHNAME_SEP;
		(void) strcpy(s, extra);
	    }
	}
    }
    return result;
}
#endif

#if !HAVE_STRDUP
char *
my_strdup(char *s)
{
    if (s != 0) {
	char *t = malloc(strlen(s) + 1);
	if (t != 0) {
	    (void) strcpy(t, s);
	}
	s = t;
    }
    return s;
}
#endif
