// Debug/trace functions for BCPP

#include "config.h"
#include "bcpp.h"

#ifdef DEBUG
char *traceDataType(DataTypes theType)
{
    char *it;
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
    TRACE((stderr, "@%d, %s%s:%s\n",
        line,
        traceDataType(pIn->dataType),
        pIn->comWcode ? " comWcode" : "",
        pIn->pData))
}

void traceOutput(int line, OutputStruct *pOut)
{
    TRACE((stderr, "@%d, indent %d, fill %d, OUT #%d:%s:%s:%s:\n",
        line,
        pOut->indentSpace,
        pOut->filler,
        pOut->thisToken,
        pOut->pCode ? "code" : "",
        pOut->pBrace ? "brace" : "",
        pOut->pComment ? "comment" : ""))
    if (pOut->pCode)    TRACE((stderr, "----- code:%s\n", pOut->pCode))
    if (pOut->pState)   TRACE((stderr, "---- state:%s\n", pOut->pState))
    if (pOut->pBrace)   TRACE((stderr, "---- brace:%s\n", pOut->pBrace))
    if (pOut->pComment) TRACE((stderr, "-- comment:%s\n", pOut->pComment))
}
#endif
