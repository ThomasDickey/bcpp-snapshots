// Tab conversion & first-pass scanning

#include <ctype.h>
#include <string.h>            // strlen(), strstr(), strchr(), strcpy(), strcmp()

#include "bcpp.h"
#include "config.h"

// ----------------------------------------------------------------------------
// Compute the number of characters in an escape
static int skipEscape(char *String)
{
    int it = 1;
    int n = 1;

    if (isdigit(String[n]))
    {
        while (n <= 3)
            if (isdigit(String[n++]))
                it = n;
            else
                break;
    }
    else if (String[n] == 'x')
    {
        while (n <= 3)
            if (isxdigit(String[++n]))
                it = n;
            else
                break;
    }
    else if (String[n] == NULLC)
        it = 0;
    return it;
}

// ----------------------------------------------------------------------------
#ifdef DEBUG
static const char *showCharState(CharState theState)
{
    const char *it;
    switch (theState)
    {
        default:
        case Normal:    it = "Normal";  break;
        case Comment:   it = "Comment"; break;
        case Ignore:    it = "Ignore";  break;
        case DQuoted:   it = "DQuoted"; break;
        case SQuoted:   it = "SQuoted"; break;
    }
    return it;
}
#endif

// ----------------------------------------------------------------------------
// Compute the state after this character is processed
static void nextCharState(char * &String, CharState &theState, int &skip)
{
    if (skip-- <= 0)
    {
        skip = 0;
        if (theState == Normal)
        {
            switch (String[0])
            {
                case ESCAPE:
                    skip = skipEscape(String);
                    break;
                case DQUOTE:
                    theState = DQuoted;
                    break;
                case SQUOTE:
                    theState = SQuoted;
                    break;
                case '/':
                    switch (String[1])
                    {
                    case '*':
                        theState = Comment;
                        skip = 1;
                        break;
                    case '/':
                        theState = Ignore;
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
            }
        }
        else if (theState == Comment)
        {
            if (String[0] == '*'
             && String[1] == '/')
            {
                theState = Normal;
                skip = 1;
            }
        }
        else if (theState == SQuoted)
        {
            switch (String[0])
            {
                case SQUOTE:
                    theState = Normal;
                    break;
                case ESCAPE:
                    skip = skipEscape(String);
                    break;
                default:
                    break;
            }
        }
        else if (theState == DQuoted)
        {
            switch (String[0])
            {
                case DQUOTE:
                    theState = Normal;
                    break;
                case ESCAPE:
                    skip = skipEscape(String);
                    break;
                default:
                    break;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// Function expands tabs to spaces; the number of spaces to expand to is
// dependent upon the tabSpaceSize parameter within user settings, and
// tab column positions.
//
// Parameters:
//      pString     : Pointer to the string to process !
//      tabLen      : How much a tab is worth in spaces.
//      leftState   : character-state at left-end of string
//      rightState  : character-state at right-end of string
//
// Return Values:
//      char*       : Pointer to newly constructed string, return NULL
//                    if memory allocation problems.
//
char* ExpandTabs (char* pString, int tabLen, CharState &leftState, CharState &rightState)
{
    int   col = 0;
    int   skip = 0;
    size_t last = 0;
    char* pSTab = pString;
    bool  expand = True;

    //TRACE((stderr, " ExpandTabs(%s)\n", pString))
    leftState = rightState;                     // push the old state down
    while (*pSTab != NULLC)
    {
        col++;

        if (skip || !isspace(*pSTab))
            last = col + skip;

        if (*pSTab == TAB                       // calculate tab positions !
         && expand
         && skip == 0
         && rightState != SQuoted
         && rightState != DQuoted)
        {
            int tabAmount = 0;

            // tab is first character !!!!
            if (col == 1)
                tabAmount = tabLen;
            else
                tabAmount = (((col / tabLen)+1) * tabLen) - col + 1;

            //TRACE((stderr, "amount:%d, col:%d\n", tabAmount, col))
            if (tabAmount > 0)
            {
                // create newString, remove tab !
                char* pNewString = new char[strlen (pString) + tabAmount + 1];

                if (pNewString == NULL)
                {
                    delete pString;
                    return NULL;
                }

                // copy first part
                strcpy (pNewString, pString);

                // add spaces
                char *pAddSpc = pNewString + col - 1;
                while (tabAmount-- > 0)
                    *pAddSpc++ = SPACE;

                // add original trailing spaces
                strcpy (pAddSpc, pSTab+1);
                delete pString;                 // remove old string from memory
                pString = pNewString;
                pSTab   = pString + col - 1;    // point to the first blank
                //TRACE((stderr, "...%d:%s\n", col, pString))
            }
            else
                *pSTab = SPACE;

        }
        // SCCS ID contains a tab that we don't want to touch
        else if (*pSTab == '@' && !strncmp(pSTab+1, "(#)", 3))
        {
            expand = False;
        }

        nextCharState(pSTab, rightState, skip);
        pSTab++;
    }

    if (rightState == Ignore)
        rightState = Normal;

    if (skip == 0
     && (rightState == DQuoted
      || rightState == SQuoted))
        rightState = Normal;    // recover from syntax error

    if (last < strlen(pString))
        pString[last] = NULLC;      // trim trailing blanks

    //TRACE((stderr, " Expanded  (%s)\n", pString))
    //TRACE((stderr, "FIXME %d/%d %s/%s\n", last, strlen(pString), showCharState(leftState), showCharState(rightState)))

    return pString;
}

// ----------------------------------------------------------------------------
// This function is used to allocate memory for indentation within function
// OutputToOutFile(). Once the memory needed is allocated, it fills the memory
// with spaces, or tabs depending upon the fill mode.
//
// Parameters:
// Mode         : Defines the fill mode of the memory that it allocate
//             1 = tabs only
//             2 = spaces only
//             3 = both
// len       : Number of bytes needed to be allocated
// spaceIndent:Number of memory locations a tab character take up
//
// Return Values:
// char*     : Returns a pointer to the memory/string that was allocated
//
char* TabSpacing (int mode, int len, int spaceIndent)
{
    char* pOutTab = NULL;
    char* pOutSpc = NULL;

    if ((mode & 1) == 1)
    {
        int numOfTabs = 0;

        // bypass exception error
        if (spaceIndent > 0)
           numOfTabs = len / spaceIndent;

        pOutTab = new char[numOfTabs+1];
        if (pOutTab != NULL)
        {
            for (int fillTabs = 0; fillTabs < numOfTabs; fillTabs++)
                    pOutTab[fillTabs] = TAB;
            pOutTab[numOfTabs] = NULLC;
        }
        else
            return NULL; // memory allocation failed

        // If not in both tab, and space concatenation.
        if ((mode & 2) == 0)
              return pOutTab;
    }//bit 0 set !

    if ((mode & 2) == 2)
    {
        if (pOutTab == NULL) //##### normal space allocation !
        {
            pOutSpc = new char[len+1];
            if (pOutSpc != NULL)
            {
                for (int fillSpcs = 0; fillSpcs < len; fillSpcs++)
                    pOutSpc[fillSpcs] = SPACE;
                pOutSpc[len] = NULLC;
                return pOutSpc;   //##### return end product
            }
            else
                    return NULL; // memory allocation failed
        }
        else  // else a mix of spaces & tabs
        {
            int numOfSpcs = 0;

            if (spaceIndent > 0)
               numOfSpcs = len % spaceIndent;

            pOutSpc = new char[numOfSpcs+1];
            if (pOutSpc != NULL)
            {
                for (int fillSpcs = 0; fillSpcs < numOfSpcs; fillSpcs++)
                    pOutSpc[fillSpcs] = SPACE;
                pOutSpc[numOfSpcs] = NULLC;
            }
            else
                return NULL; // memory allocation failed
        }
    }// bit 1 set

    //##### Concatinate tabs & spaces
    if ( ((mode & 1) == 1) && ((mode & 2) == 2) )
    {
        char* pConCat = new char[(strlen (pOutTab) + strlen (pOutSpc) + 1)];
        // #### Check memory allocation
        if (pConCat == NULL)
        {
            delete pOutTab;
            delete pOutSpc;
            return NULL;
        }
        strcpy (pConCat, pOutTab);
        strcpy (pConCat + strlen (pConCat), pOutSpc);
        delete pOutTab;
        delete pOutSpc;
        return pConCat;
    }

    return NULL; //##### illegal mode passed !
}
