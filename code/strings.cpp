// strings.cpp

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "config.h"
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
         return false;
   TRACE((stderr, "Match (%s,%s)\n", tst, ref))
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
        strncpy(dst, src, len)[len] = NULLC;
    }
    return dst;
}

const char *SkipBlanks(const char *s)
{
    while (isspace(*s))
        s++;
    return s;
}
