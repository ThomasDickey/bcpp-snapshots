// EXEC SQL parsing & indention

#include <ctype.h>
#include <string.h>

#include "config.h"
#include "bcpp.h"
#include "cmdline.h"        // StrUpr()

// skip to the beginning of the next word, inclusive of the starting position.
static int NextWord(int start, OutputStruct *pOut, int& level)
{
    int n = start;
    bool reset = False;

    TRACE((stderr, "next:%s\n", pOut -> pCode+start))
    while (pOut -> pCode[n] != NULLC
      &&  (pOut -> pState[n] != Normal || !isName(pOut -> pCode[n])))
    {
        if ((pOut -> pState[n] == Normal && !isName(pOut -> pCode[n]))
          || pOut -> pState[n] == DQuoted
          || pOut -> pState[n] == DQuoted)
            reset = True;
        n++;
    }

    // Maintain the level-counter as the maximum number of words we could
    // have matched.  Punctuation resets it, since the SQL-keywords don't
    // bypass normal C/C++ syntax.
    if (reset)
        level = 0;

    return n;
}

static int SkipWord(int start, OutputStruct *pOut)
{
    int n = start;

    TRACE((stderr, "skip:%s\n", pOut -> pCode+start))
    // skip the current word
    while (pOut -> pCode[n] != NULLC
      &&  (ispunct(pOut -> pState[n]) && isName(pOut -> pCode[n])))
        n++;
    return n;
}

// return true if we've found a keyword which shouldn't be indented
static bool SqlVerb(const char *code)
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

void IndentSQL(OutputStruct *pOut, const Config& userS, int& sql_state)
{
    static int level;
    static char matched[80];

    // (Yes, lex/yacc would be more powerful ;-)
    const struct {
        char *name;
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
        char *text;
        int code;
    } state_strings[] = {
        { "+Sbds",  1 },        // begin declaration
        { "+Seds",  0 },        // end declaration
        { "+Sxb",   2 },        // begin statement
        { "E",      0 },        // end statement
    };

    int old_state = sql_state;
    char* pUprString = NULL;

    // First, look for SQL keywords to see when we've entered a block or
    // a statement.  Ignore preprocessor-lines.
    if (pOut -> pType == Code
     && pOut -> pCode != NULL
     && pOut -> pState != NULL)
    {
        pUprString = new char[strlen (pOut -> pCode) + 1];
        if (pUprString == NULL)
            return;

        StrUpr (strcpy(pUprString, pOut->pCode));
        TRACE((stderr, "HERE:%s\n", pUprString))
        TRACE((stderr, "FLAG:%s\n", pOut->pState))

        for (int n = NextWord(0, pOut, level);
            pOut -> pCode[n] != NULLC;
                n = NextWord(n, pOut, level))
        {
            for (size_t m = 0; m < TABLESIZE(state_keys); m++)
            {
                if (CompareKeyword(pUprString+n, state_keys[m].name))
                {
                    matched[level++] = state_keys[m].code;
                    matched[level] = NULLC;
                    n += strlen(state_keys[m].name); // fix for END-EXEC
                    for (size_t j = 0; j < TABLESIZE(state_strings); j++)
                    {
                        if (!strcmp(state_strings[j].text, matched))
                        {
                            sql_state = state_strings[j].code;
                            matched[level = 0] = NULLC;
                            break;
                        }
                    }
                    break;
                }
            }
            TRACE((stderr, "TEST:%d:%s:%s\n", level, matched, pUprString+n))
            n = SkipWord(n, pOut);
            if (pOut -> pCode[n] == NULLC)
            {
                break;
            }
        }
    }

    if (sql_state != 0
     && old_state != 0
     && pOut -> pType != PreP)
    {
        pOut -> indentSpace += userS.tabSpaceSize;
        if (sql_state == 2 && !SqlVerb(pUprString))
            pOut -> indentSpace += userS.tabSpaceSize;
    }
    old_state = sql_state;
    delete pUprString;
}
