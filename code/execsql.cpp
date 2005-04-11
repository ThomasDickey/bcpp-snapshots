//******************************************************************************
// Copyright 1996-2003,2005 by Thomas E. Dickey                                *
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
// $Id: execsql.cpp,v 1.16 2005/04/11 00:19:30 tom Exp $
// EXEC SQL parsing & indention

#include <ctype.h>
#include <string.h>

#include "bcpp.h"
#include "cmdline.h"        // StrUpr()

// skip to the beginning of the next word, inclusive of the starting position.
int
SqlStruct::NextWord(int start, OutputStruct *pOut)
{
    int n = start;
    bool reset = False;

    TRACE(("next:%s\n", pOut -> pCode+start));
    while (pOut -> pCode[n] != NULLC
      &&  (pOut -> pCFlag[n] != Normal || !isName(pOut -> pCode[n])))
    {
        if ((pOut -> pCFlag[n] == Normal && !isName(pOut -> pCode[n]))
          || pOut -> pCFlag[n] == DQuoted
          || pOut -> pCFlag[n] == DQuoted)
            reset = True;
        n++;
    }

    // Maintain the level-counter as the maximum number of words we could
    // have matched.  Punctuation resets it, since the SQL-keywords don't
    // bypass normal C/C++ syntax.
    if (reset)
    {
        matched[level = 0] = NULLC;
        if (state == 3
         && pOut -> pCFlag[n-1] == Normal
         && pOut -> pCode[n-1] == SEMICOLON)
        {
            //TRACE(("FIXME2:%s\n", pOut->pCode + n - 1))
            state = 0;
        }
    }

    return n;
}

int
SqlStruct::SkipWord(int start, OutputStruct *pOut)
{
    int n = start;

    TRACE(("skip:%s\n", pOut -> pCode+start));
    // skip the current word
    while (pOut -> pCode[n] != NULLC
      &&  (ispunct(pOut -> pCFlag[n]) && isName(pOut -> pCode[n])))
        n++;
    return n;
}

// return true if we've found a keyword which shouldn't be indented
bool
SqlStruct::SqlVerb(const char *code)
{
    if (!emptyString(code))
    {
        const char *table[] = {
            "ADD",
            "AND",
            "APPEND",
            "AS",
            "BEGIN",
            "BETWEEN",
            "BODY",
            "BY",
            "CANCEL",
            "CHANGE",
            "CLOSE",
            "COMMIT",
            "CONNECT",
            "CONTAIN",
            "CONTAINS",
            "COUNT",
            "CREATE",
            "CURRENT",
            "CURRVAL",
            "CURSOR",
            "DECLARE",
            "DELETE",
            "DISABLE",
            "DO",
            "DROP",
            "ELSE",
            "ELSIF",
            "ENABLE",
            "END",
            "ERASE",
            "EXCEPTION",
            "EXECUTE",
            "EXISTS",
            "FETCH",
            "FOR",
            "FROM",
            "FUNCTION",
            "GRANT",
            "GROUP",
            "HAVING",
            "IF",
            "IN",
            "INCLUDING",
            "INCREMENT",
            "INDEX",
            "INSERT",
            "INTO",
            "IS",
            "LAST",
            "LIKE",
            "MAX",
            "MIN",
            "MOD",
            "MODIFY",
            "NEW",
            "NEXT",
            "NEXTVAL",
            "NOT",
            "NULL",
            "NUMBER",
            "OF",
            "ON",
            "ONLY",
            "OPEN",
            "OR",
            "POSITION",
            "RAISE",
            "RANGE",
            "RAW",
            "READ",
            "RECOVER",
            "REM",
            "RENAME",
            "REPLACE",
            "RESUME",
            "RETURN",
            "REVERSE",
            "REVOKE",
            "ROLLBACK",
            "ROW",
            "ROWID",
            "SELECT",
            "SEQUENCE",
            "SET",
            "SORT",
            "SQL",
            "START",
            "STOP",
            "TABLE",
            "THEN",
            "THIS",
            "TO",
            "TRIGGER",
            "TRUE",
            "UNDER",
            "UNION",
            "UNIQUE",
            "UNTIL",
            "UPDATE",
            "USE",
            "USING",
            "VALIDATE",
            "VALUES",
            "VIEW",
            "WHEN",
            "WHENEVER",
            "WHERE",
            "WHILE",
            "WITH",
        };
        for (size_t n = 0; n < TABLESIZE(table); n++)
        {
            if (CompareKeyword(code, table[n]))
            {
                return True;
            }
        }
    }
    return False;
}

void
SqlStruct::IndentSQL(OutputStruct *pOut)
{
    // (Yes, lex/yacc would be more powerful ;-)
    const struct {
        const char *name;
        int code;
    } state_keys[] = {
        { "EXEC",      '+' },
        { "SQL",       'S' },
        { "EXECUTE",   'x' },
        { "BEGIN",     'b' },
        { "END-EXEC",  'E' },   // match this before "END"
        { "END",       'e' },
        { "DECLARE",   'd' },
        { "SECTION",   's' },
    };
    const struct {
        const char *text;
        int code;
    } state_strings[] = {
        { "+Sbds",  1 },        // begin declaration
        { "+Seds",  0 },        // end declaration
        { "+Sxb",   2 },        // begin statement
        { "E",      0 },        // end statement
    };

    int old_state = state;
    char* pUprString = NULL;

    // First, look for SQL keywords to see when we've entered a block or
    // a statement.  Ignore preprocessor-lines.
    if (pOut -> pType == Code
     && pOut -> pCode != NULL
     && pOut -> pCFlag != NULL)
    {
        pUprString = new char[strlen (pOut -> pCode) + 1];
        if (pUprString == NULL)
            return;

        StrUpr (strcpy(pUprString, pOut->pCode));
        TRACE(("HERE:%s\n", pUprString));
        TRACE(("FLAG:%s\n", pOut->pCFlag));

        for (int n = NextWord(0, pOut);
            pOut -> pCode[n] != NULLC;
                n = NextWord(n, pOut))
        {
            bool found = False;

            for (size_t m = 0; m < TABLESIZE(state_keys); m++)
            {
                if (CompareKeyword(pUprString+n, state_keys[m].name))
                {
                    found = True;
                    matched[level++] = state_keys[m].code;
                    matched[level] = NULLC;
                    n += strlen(state_keys[m].name); // fix for END-EXEC
                    for (size_t j = 0; j < TABLESIZE(state_strings); j++)
                    {
                        if (!strcmp(state_strings[j].text, matched))
                        {
                            state = state_strings[j].code;
                            matched[level = 0] = NULLC;
                            break;
                        }
                    }
                    break;
                }
            }
            if (!found)
            {
                if (state == 0
                 && level >= 2
                 && !strncmp(matched, "+S", 2))
                {
                    state = 3;
                    //TRACE(("FIXME:%s\n", pOut->pCode + n))
                }
                matched[level = 0] = NULLC;
            }
            TRACE(("TEST:%d:%d:%s:%s\n", state, level, matched, pUprString+n));
            n = SkipWord(n, pOut);
            if (pOut -> pCode[n] == NULLC)
            {
                break;
            }
        }
    }

    if (pOut -> pType != PreP)
    {
        if (state != 0
         && old_state != 0)
        {
            pOut -> indentHangs = 1;
            if ((state == 2 || state == 3) && !SqlVerb(pUprString))
                pOut -> indentHangs = 2;
            //TRACE(("FIXME-HANG:%d\n", pOut -> indentHangs))
        }
        else
        if (state == 0
         && old_state == 3)
        {
            pOut -> indentHangs = 1;
            if (!SqlVerb(pUprString))
                pOut -> indentHangs = 2;
            //TRACE(("FIXME-HANG2:%d\n", pOut -> indentHangs))
        }
    }

    delete[] pUprString;
}
