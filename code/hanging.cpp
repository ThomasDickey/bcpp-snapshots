// $Id: hanging.cpp,v 1.8 1996/12/10 01:27:02 tom Exp $
// Compute hanging-indent for most multiline statements.

#include <stdlib.h>
#include <ctype.h>

#include "config.h"
#include "bcpp.h"

// Determine if the current OutputStruct should be indented for hanging
// indent of a multi-line statement.  Generally, we indent all lines after
// the first for a statement, except when other conditions prevail:
//
//      a.  we encounter curly-braces (which have their own rules)
//      b.  we encounter a keyword that has its own indention rules

void
HangStruct::ScanState(const char *code, const char *state)
{
    if (code != 0)
    {
        for (int n = 0; code[n] != NULLC; n++)
        {
            if (state[n] == Normal)
            {
                int c = code[n];

                if (isName(c)
                 && (n == 0 || !isName(code[n-1])))
                {
                    int findWord = LookupKeyword(code + n);
                    do_aggreg = False;
                    if (findWord >= 0)
                    {
                        indent = 0;
                        until_parn = 1;
                        n += strlen(pIndentWords[findWord].name) - 1;
                        if (pIndentWords[findWord].code == oneLine)
                            stmt_level++;
                        else
                            stmt_level = 0;
                    }
                    else
                    {
                        if (CompareKeyword(code+n, "enum"))
                            until_curl = 1;

                        if (parn_level == 0)
                            until_parn = 0;
                        indent = 1;
                        while (isName(code[n]))
                            n++;
                        n--;
                    }
                }
                else if (!isspace(code[n]))
                {
                    if (do_aggreg && code[n] != L_CURL)
                        do_aggreg = False;

                    switch (code[n])
                    {
                        case '=':
                            if (parn_level == 0)
                                do_aggreg = True;
                            break;
                        case L_CURL:
                            curl_level++;
                            indent = 0;
                            stmt_level = 0;
                            until_parn = 0;
                            if (do_aggreg)
                                in_aggreg = curl_level;
                            break;
                        case R_CURL:
                            curl_level--;
                            indent = 0;
                            stmt_level = 0;
                            until_curl = 0;
                            break;
                        case ':':
                            // "::" means something different entirely
                            if (code[n+1] == ':')
                            {
                                n++;
                            }
                            else
                            {
                                indent = 0;
                                stmt_level = 0;
                                until_parn = 0;
                            }
                            break;
                        case SEMICOLON:
                            if (parn_level == 0)
                            {
                                indent = 0;
                                stmt_level = 0;
                                until_parn = 0;
                                until_curl = 0;
                                if (in_aggreg > curl_level)
                                    in_aggreg = 0;
                            }
                            break;
                        case '(':
                            parn_level++;
                            indent = 1;
                            break;
                        case ')':
                            parn_level--;
                            if (until_parn && !parn_level)
                                indent = 0;
                            else
                                indent = 1;
                            break;
                        default:
                            indent = 1;
                            break;
                    }
                }
            }
        }
    }
}

void
HangStruct::IndentHanging(OutputStruct *pOut)
{
#ifdef DEBUG2
    traceOutput(__LINE__, pOut);
    TRACE((stderr, "state:%d/%d, parn:%d/%d, curl:%d, agg:%d/%d\n",
        indent,
        stmt_level,
        until_parn,
        parn_level,
        curl_level,
        do_aggreg,
        in_aggreg))
#endif

    if (indent != 0
     && curl_level != 0
     && (until_parn == 0 || parn_level != 0)
     && until_curl == 0
     && in_aggreg < curl_level
     && !emptyString(pOut -> pCode)
     && !emptyString(pOut -> pCFlag)
     && (pOut -> pCFlag[0] == Normal || !ContinuedQuote(pOut)))
    {
        pOut -> indentHangs = indent;
        if (stmt_level && !until_parn)
            pOut -> indentHangs += stmt_level;
        TRACE((stderr, "HANG:%d\n", pOut -> indentHangs))
    }

    if (pOut -> pType != PreP)
    {
        ScanState(pOut -> pCode,  pOut -> pCFlag);
        ScanState(pOut -> pBrace, pOut -> pBFlag);
    }
}
