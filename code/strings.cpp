// strings.cpp

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "bcpp.h"

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
