//******************************************************************************
// Copyright 1996,1997,1999 by Thomas E. Dickey <dickey@clark.net>             *
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
// $Id: debug.cpp,v 1.11 1999/01/01 17:03:09 tom Exp $
// Debug/trace functions for BCPP

#include <stdlib.h>
#include <stdarg.h>

#include "bcpp.h"

void trace(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

#if defined(DEBUG) || defined(DEBUG2)
const char *traceDataType(DataTypes theType)
{
    const char *it;
    switch (theType)
    {
        case CCom:   it = "CCom";   break;
        case CppCom: it = "CppCom"; break;
        case Code:   it = "Code";   break;
        case OBrace: it = "OBrace"; break;
        case CBrace: it = "CBrace"; break;
        case PreP:   it = "PreP";   break;
        default:
        case ELine:  it = "ELine";  break;
    }
    return it;
}

void traceInput(int line, InputStruct *pIn)
{
    TRACE(("@%d, %s%s (col:%d)\n",
        line,
        traceDataType(pIn->dataType),
        pIn->comWcode ? " comWcode" : "",
        pIn->offset))
    if (pIn->pData  != 0) TRACE(("---- data:%s\n", pIn->pData))
    if (pIn->pState != 0) TRACE(("---- flag:%s\n", pIn->pState))
}

void traceOutput(int line, OutputStruct *pOut)
{
    TRACE(("@%d, indent %d(%d), fill %d, OUT #%d:%s:%s:%s:\n",
        line,
        pOut->indentSpace,
        pOut->indentHangs,
        pOut->filler,
        pOut->thisToken,
        pOut->pCode ? "code" : "",
        pOut->pBrace ? "brace" : "",
        pOut->pComment ? "comment" : ""))
    if (pOut->pCode)    TRACE(("----- code:%s\n", pOut->pCode))
    if (pOut->pCFlag)   TRACE(("---- state:%s\n", pOut->pCFlag))
    if (pOut->pBrace)   TRACE(("---- brace:%s\n", pOut->pBrace))
    if (pOut->pBFlag)   TRACE(("---- state:%s\n", pOut->pBFlag))
    if (pOut->pComment) TRACE(("-- comment:%s\n", pOut->pComment))
}
#endif
