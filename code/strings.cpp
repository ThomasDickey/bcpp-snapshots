//******************************************************************************
// Copyright 1996-2002,2005 by Thomas E. Dickey                                *
// All Rights Reserved.                                                        *
//                                                                             *
// Permission to use, copy, modify, and distribute this software and its       *
// documentation for any purpose and without fee is hereby granted, provided   *
// that the above copyright notice appear in all copies and that both that     *
// copyright notice and this permission notice appear in supporting            *
// documentation, and that the name of the above listed copyright holder(s)    *
// not be used in advertising or publicity pertaining to distribution of the   *
// software without specific, written prior permission. THE ABOVE LISTED       *
// COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,   *
// INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO      *
// EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY SPECIAL, *
// INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM  *
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE  *
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR   *
// PERFORMANCE OF THIS SOFTWARE.                                               *
//******************************************************************************
// $Id: strings.cpp,v 1.12 2005/04/11 00:20:30 tom Exp $
// strings.cpp

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "bcpp.h"

bool isName(char c)
{
   return isalnum(c) || (c == '_') || (c == '$');
}

bool CompareKeyword(const char *tst, const char *ref)
{
   int n;
   for (n = 0; ref[n] != NULLC; n++)
      if (tst[n] != ref[n])
         return False;
   TRACE(("Match (\"%s\",\"%s\")\n", tst, ref));
   return !isName(tst[n]);
}

char *NewString (const char *src)
{
    char* dst =  new char[strlen (src)+1];
    if (dst != 0)
        strcpy(dst, src);
    return dst;
}

char *NewSubstring (const char *src, size_t len)
{
    char* dst =  new char[len + 1];
    if (dst != 0)
    {
        strncpy(dst, src, len);
        dst[len] = NULLC;
    }
    return dst;
}

const char *SkipBlanks(const char *s)
{
    while (isspace(*s))
        s++;
    return s;
}
