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

static int  until_parn;     // suppress hang until right-parenthesis
static int  parn_level;     // parentheses-level

static int  until_curl;     // suppress hang until R_CURL
static int  curl_level;     // curly-brace-level

static int  in_aggreg;      // in aggregate, curly-brace-level
static bool do_aggreg = False;

static void ScanState(const char *code, const char *state, int& hang_state)
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
                        hang_state = 0;
                        until_parn = 1;
                        n += strlen(pIndentWords[findWord].name) - 1;
                    }
                    else
                    {
                        if (CompareKeyword(code+n, "enum"))
                            until_curl = 1;

                        if (parn_level == 0)
                            until_parn = 0;
                        hang_state = 1;
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
                            hang_state = 0;
                            until_parn = 0;
                            if (do_aggreg)
                                in_aggreg = curl_level;
                            break;
                        case R_CURL:
                            curl_level--;
                            hang_state = 0;
                            until_curl = 0;
                            break;
                        case ':':
                            hang_state = 0;
                            until_parn = 0;
                            break;
                        case SEMICOLON:
                            if (parn_level == 0)
                            {
                                hang_state = 0;
                                until_parn = 0;
                                until_curl = 0;
                                if (in_aggreg > curl_level)
                                    in_aggreg = 0;
                            }
                            break;
                        case '(':
                            parn_level++;
                            hang_state = 1;
                            break;
                        case ')':
                            parn_level--;
                            if (until_parn && !parn_level)
                                hang_state = 0;
                            else
                                hang_state = 1;
                            break;
                        default:
                            hang_state = 1;
                            break;
                    }
                }
            }
        }
    }
}

void IndentHanging(OutputStruct *pOut, int& hang_state)
{
#ifdef DEBUG2
    traceOutput(__LINE__, pOut);
    TRACE((stderr, "state:%d, parn:%d/%d, curl:%d, agg:%d/%d\n", hang_state, until_parn, parn_level, curl_level, do_aggreg, in_aggreg))
#endif

    if (hang_state != 0
     && curl_level != 0
     && until_parn == 0
     && until_curl == 0
     && in_aggreg < curl_level
     && !emptyString(pOut -> pCode)
     && !emptyString(pOut -> pCFlag)
     && (pOut -> pCFlag[0] == Normal || !ContinuedQuote(pOut)))
    {
        TRACE((stderr, "HERE\n"))
        pOut -> indentHangs = 1;
    }

    if (pOut -> pType != PreP)
    {
        ScanState(pOut -> pCode,  pOut -> pCFlag, hang_state);
        ScanState(pOut -> pBrace, pOut -> pBFlag, hang_state);
    }
}
