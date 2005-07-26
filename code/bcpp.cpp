// C(++) Beautifier V1.61 Unix/MS-DOS update !
// -----------------------------------------
// $Id: bcpp.cpp,v 1.115 2005/07/25 20:53:34 tom Exp $
//
// Program was written by Steven De Toni 1994 (CBC, ACBC).
// Modified/revised by Thomas E. Dickey 1996-2002,2003.
//
// Steven's original notes follow:
// ----------------------------------------------------------------------------
// This program attempts to alter C, C++ code so that it fits to a
// format that the user wants.
// This program is the result of a project that needed to be written
// for a module that I was doing at the Waikato Polytech.
// Course     : Advanced Certificate In Business Computing (ACBC)
// Module     : PR-300.
// Assignment : Project 1, approx 60 hours.
//
// The program reads a configuration file that has the user's settings.
// Input, and output files are via the command line.
//
// If compiling under DOS, use Large memory model, any other
// type of system (i.e., Unix, Amiga), use default.
// I have tried to use all standard Unix functions for I/O to keep
// the Unix programmers happy.
//
// NOTE:
// This code has never been tested under any other system other than MS-DOS.
// All program code is public domain, you can use any part of this
// code on the condition that my name be placed somewhere within documentation,
// or program. However, all rights are reserved as to the program logic, that
// is to say, you can change it, give it to people, but my name must still
// exist within the credits of the old, or newly altered program (Programmers Ethics).
//
// ###################################################################################
//
// Program Update : 14/11/94 V1.5
//
// Removed a couple more bugs, optimized code (not that you notice it),
// added the following features:
//     - Able to decode lines fully (uses recursion)
//     - Able to handle unlimited depth handling for single indent code.
//     - Added command line parameter processing (able use i/o redirection)
//     - Added variable internal buffer length (used for repositioning of braces)
//
// ###################################################################################
//
// Program Update : 21/11/94 V1.55
//
// Compiling done using GNU G++, brilliant compiler ...
// Dos debugger (debug32) sucked totally compared to Borlands programming
// environment, ended up using a series of printf statements in debugging !
//
// Program functions perfectly under Sun SPARC station running SunOs V??,
// compiled using GNU G++ V2.6.?. Version 1.05 of the GNU running under
// DOS functions brilliantly, except for input redirection fails. I think this
// could be due to fseek() failing to move back, and forward within the input
// stream. Under Unix, this isn't a problem as pipes/redirections are handled
// properly !
//
// Removed/fixed segmentation violations picked up by G++ within code!
//
//     - Removed non-existing enum constant out of fseek() (I didn't know)
//     - Wrote my own string upper case routine as standard string.h
//       didn't have it (again I didn't know).
//     - Fix a couple of string constants, and few other odds and ends.
//
// ###################################################################################
//
// Program Update : 24/11/94 V1.6 <Final>
//
// Final version update of this program.
//
//      - Added a time feature to show long processing of the input
//        data stream took ... just because I could.
//
//      - Fixed tab space size so that code lines are calculated at correct
//        lengths according to indent spacing.
//
//      - Implemented a backup feature for the input file, and changed the
//        selection of input/output file to the Unix standard.
//
//      - fix the following decode problems
//        if (strcmp (pAString, "A String") == 0) // test for "A String"
//                                                ^
//                                                |
//  Not recognized as a comment because of the quotes chars!
//
//      - struct a { int b, c, d; } as;
//        The above now gets decoded properly.
//
//      - able to indent multiple depthed switch like structures properly.
//
// ###################################################################################
//
// Program Update : 9/1/95 V1.61 <User Patch>
//
//      - Able to set indentation of comments with no code the same as normal code,
//        this results in the comments lining up with the code.
//
//      This option was brought to you by a Canadian user Justin Slootsky.
//
// ###################################################################################
//
// Program Update : 19/2/95 V1.62 <Error Update>
//
// Fixed error that was caused when C style comment was placed in the wrong
// placed which caused program output to be wrong
//
// if (someCode) /* C Style Comment
//
// <may end up like the follow ing code !>
//
//         /* C Style Comment
// if (someCode)
//
// Fixed output error is multi-if statements were nested together, and have
// have code within braces after them.
//
// if (someCode) { someCode; }
// if (someCode) { someCode; }
//
// <ended up line>
//
// if (someCode) { someCode; }
//    if (someCode) { someCode; }
//
// ###################################################################################
//
// Program Update : 10/3/95 V1.629 <PreProcessor Update>
//
// Fixed problems with preprocessor statements. All preprocessor statements
// are place at column 1.
//
// ###################################################################################
//
// Program Update : 4/5/95 V1.7 <C comment fix>
//
// Fixed removing of spaces in c style multiline comments
// Remove bug with c style comment that has code before it (uses recursion as a fix)
// Added location of program to use bcpp.cfg configuration without specifying
// it with options (i.e. uses PATH variable to locate).
// ###################################################################################
//
// Program Update : 27/11/95 V1.75 <Single if, while, do, statements fixed>
//
// Fixed the indenting of single statements so they don't become corrupt, and
// line up with previous code.
// Also some simple logic errors where found by users and fixed.
//
// ###################################################################################
//
// Program Update : 06/12/95 V1.8 (Bug was created when ported to Unix and
// trying to prevent segmentation errors, this has been fixed. The problem
// caused incorrect indenting of code if a 'if' statement was within a case
// structure.


#include <stdlib.h>            // getenv()
#include <time.h>              // time()
#include <string.h>            // strlen(), strstr(), strchr(), strcpy(), strcmp()
#include <ctype.h>             // character-types

#include "cmdline.h"           // ProcessCommandLine()
#include "bcpp.h"

// ----------------------------------------------------------------------------

static const char *cppc_begin = "//";
static const char *ccom_begin = "/*";
static const char *ccom_end = "*/";

const IndentwordStruct pIndentWords[] = {
    { "if",         oneLine },
    { "while",      oneLine },
    { "for",        oneLine },
    { "else",       oneLine },
    { "case",       multiLine },
    { "default",    multiLine },
    { "public",     multiLine },
    { "protected",  multiLine },
    { "private",    multiLine },
    { "do",         blockLine },
    { "switch",     blockLine },
    { "while",      blockLine },
};

#if defined(DEBUG) || defined(DEBUG2)
int   totalTokens;            // token count, for debugging
#endif

// ----------------------------------------------------------------------------

static inline const char *endOf(const char *s)
{
   return (s + strlen(s));
}

static inline char *endOf(char *s)
{
   return (s + strlen(s));
}

static inline char lastChar(const char *s)
{
   return ((s != NULL) && (*s != NULLC)) ? *(endOf(s)-1) : static_cast<char>(NULLC);
}

static Boolean IsStartOfComment(char *pLineData, char *pLineState)
{
    if (pLineState[0] == Comment)
    {
        if (!strncmp(pLineData, ccom_begin, 2))
            return True;
    }
    return False;
}

static Boolean IsEndOfComment(char *pLineData, char *pLineState)
{
    while (*pLineState++ == Comment)
    {
        if (!strncmp(pLineData++, ccom_end, 2))
            return True;
    }
    return False;
}

static Boolean IsLeadingCommentFragment(char *pLineData, char *pLineState)
{
    if (IsStartOfComment(pLineData, pLineState)
     && !IsEndOfComment(pLineData+2, pLineState+2))
        return True;
    return False;
}

// Check if we've just extracted a comment fragment, i.e., a C comment
// beginning on the current line that doesn't end there.  We'll have to defer
// the comment til after the code is flushed out, otherwise we end up
// commenting it out.
static Boolean ExtractedCCmtFragment(char *pLineData, InputStruct* pItem)
{
    if (*SkipBlanks(pLineData)
     && IsLeadingCommentFragment(pItem->pData, pItem->pState))
    {
        pItem->comWcode = False;
        pItem->offset = 0;
        return True;
    }
    return False;
}

static inline void ShiftLeft(char *s, int len)
{
    if (len > 0)
    {
        char *t = s + len;
        while ((*s = *t) != '\0')
        {
            ++s;
            ++t;
        }
    }
}

int LookupKeyword(const char *tst)
{
    size_t n;
    if (!emptyString(tst))
    {
        for (n = 0; n < TABLESIZE(pIndentWords); n++)
            if (CompareKeyword(tst, pIndentWords[n].name))
                return n;
    }
    return -1;
}

// Return true if the given data is a blockLine.
static bool beginBlockLine(OutputStruct* pItem)
{
    bool result = False;
    int findWord = LookupKeyword(pItem -> pCode);
    if (findWord >= 0)
    {
        if (pIndentWords[findWord].code == blockLine)
            result = True;
    }
    return result;
}

// Return true if the given data is a multiLine.
static bool beginMultiLine(OutputStruct* pItem)
{
    bool result = False;
    int findWord = LookupKeyword(pItem -> pCode);
    if (findWord >= 0)
    {
        if (pIndentWords[findWord].code == multiLine)
            result = True;
    }
    return result;
}

// Return true if the current line is a blockLine preceding L_CURL.
static bool beginBlockLine(QueueList* pLines)
{
    return beginBlockLine(reinterpret_cast<OutputStruct *>(pLines -> peek(1)));
}

// ----------------------------------------------------------------------------
// Function removes leading, trailing, both leading/trailing characters
// that are less than or equal to a space character (includes spaces, tabs etc)
//
// Parameters:
// pLineData : Pointer to the start location of the string that's going to be processed
// mode      : Bit values that define the removal of characters from the string...
//             1 = remove spaces from left
//             2 = remove spaces from right
//             3 = remove spaces from left, and right
//
// Returns: the number of spaces removed from the left.
//
static int StripSpacingLeftRight (char* pLineData, char* pLineState, int mode = 3)
{
    int n;
    int result = 0;

    if (mode & 1)
    {
        for (n = -1; pLineState[n+1] == Blank; n++)
            ;

        if (n >= 0)
        {
            ShiftLeft(pLineData,  n+1);
            ShiftLeft(pLineState, n+1);
            result = n+1;
        }
    }

    if (mode & 2)
    {
        for (n = strlen(pLineData); n > 0 && pLineState[n-1] == Blank; n--)
        {
            pLineData[n-1] = NULLC;
            pLineState[n-1] = NullC;
        }
    }
    return result;
}


// ----------------------------------------------------------------------------
// Function returns a Boolean value that shows where code is contained within
// a string, given its parse-state.
//
// Parameters:
// pLineState : Pointer to a string to process.
//
// Return Values:
// Boolean   : False = line has no code
//             True  = line has some sort of code
//
static Boolean TestLineHasCode (char* pLineState)
{
    if (pLineState != NULL)
    {
        while (*pLineState != NullC)
        {
            if (ispunct(*pLineState))
                return True;
            pLineState++;
        }
    }
    return False;
}


static inline void TerminateLine(char *pData, char *pState, size_t n)
{
    pData[n] = NULLC;
    pState[n] = NullC;
}

/*
 * Check if the indicated comment is the last item on the line.  If it is not,
 * it is not safe to move to the end of the line, or to a different line, since
 * we are not sure of the context.
 */
static bool isFinalComment(int first, int last, char *pData, char *pState)
{
    if (pState[first] == Ignore)
    {
        return True;
    }
    last += 2;              // count the "*/"
    int limit = strlen(pState);
    if (last >= limit)
    {
        return True;
    }
    while (last < limit)
    {
        if (pData[last] != ESCAPE
         && pData[last] != SPACE)
            return False;
        ++last;
    }
    return True;
}

static bool isContinuation(size_t &len, char *pData, char *pState)
{
    if (pData != 0 && pState != 0)
    {
        len = strlen(pState);
        if (len != 0
         && pData[--len] == ESCAPE
         && pState[len] != Comment
         && pState[len] != Ignore)
            return True;
    }
    return False;
}

static bool isContinuation(InputStruct *pItem)
{
    size_t len;
    return isContinuation(len, pItem->pData, pItem->pState);
}

static bool isContinuation(OutputStruct *pItem)
{
    size_t len;
    return isContinuation(len, pItem->pCode, pItem->pCFlag);
}

static void TrimContinuation(char *pData, char *pState)
{
    size_t len;
    if (isContinuation(len, pData, pState))
    {
        while (len > 1
         && isspace(pState[len-1])
         && isspace(pState[len-2]))
        {
            len--;
            pData[len-1] = SPACE;
            pData[len]  = ESCAPE; pData[len+1]  = NULLC;
            pState[len] = Normal; pState[len+1] = NullC;
        }
        if (len > 0
         && isspace(pState[len-1]))
        {
            pData[len-1] = SPACE;
        }
    }
}

// check if the given data is a preprocessor-line
static inline bool isPreproLine(OutputStruct* pOut)
{
    return ((pOut != 0)
         && (pOut->pCode != 0)
         && (pOut->pType == PreP));
}

// check if the given data begins with a right curly-brace
static inline bool BeginsCurly(OutputStruct* pOut)
{
    bool result = False;
    for (int n = 0; pOut->pCode[n] != NULLC; ++n)
    {
        if (pOut->pCFlag[n] != Blank)
        {
            if (pOut->pCFlag[n] == Normal
             && pOut->pCode[n] == R_CURL)
                result = True;
            break;
        }
    }
    return result;
}

// ----------------------------------------------------------------------------
// This function is used within function DecodeLine(), it creates a new
// InputStructure and stores what is contained in pLineData string in
// the newly created structure.
//
// Parameters:
// offset     : offset within original line's text of this component
// pLineData  : Pointer to the string to store within the InputStructure.
// dataType   : Type of data that is to be stored within the InputStructure
//              see DataTypes enum.
// removeSpace : true when we're to remove leading/trailing blanks
//
// Return Values:
// InputStruct* : Returns a pointer to the newly constructed InputStructure,
//                returns a NULL value if unable to allocate memory.
//
static InputStruct* ExtractCode (int      offset,
                          char*    pLineData,
                          char*    pLineState,
                          DataTypes dataType = Code,
                          Boolean  removeSpace = True)
{
    char* pNewCode = 0;
    char* pNewState = 0;
    InputStruct* pItem = 0;

    if ((pNewCode = NewString(pLineData)) != 0)
    {
        if ((pNewState =  NewString(pLineState)) != 0)
        {
            // strip spacing in new string before storing
            if (removeSpace != False)
            {
                offset += StripSpacingLeftRight (pNewCode, pNewState);
                if (dataType == Code
                 || dataType == PreP)
                    TrimContinuation(pNewCode, pNewState);
            }
            if ((pItem = new InputStruct(dataType, offset)) != 0)
            {
                pItem -> pData    = pNewCode;
                pItem -> pState   = pNewState;
                return pItem;
            }
            delete pNewState;
        }
        delete pNewCode;
    }

    return 0;
}


// ----------------------------------------------------------------------------
// Extracting comments from the input line is a little more complicated than
// the code fragments, since we'll strip the comments out of the input line
// after extracting them.
//
static InputStruct* ExtractCCmt (int&     offset,
                          int      start,
                          int      end,
                          char*    pLineData,
                          char*    pLineState,
                          DataTypes dataType = CCom)
{
    InputStruct* pItem = 0;
    char endData = NULLC;
    char endState = NULLC;
    size_t len = end >= 0 ? (end - start + 2) : strlen(pLineData);
    size_t last = start + len;

    if (end >= 0)
    {
        endData  = pLineData[last];
        endState = pLineState[last];
        TerminateLine(pLineData, pLineState, last);
    }

    pItem = ExtractCode(
        offset + start,
        pLineData + start,
        pLineState + start,
        dataType,
        False);

    if (end >= 0)
    {
        pLineData[last]  = endData;
        pLineState[last] = endState;
        ShiftLeft (pLineData +start, len);
        ShiftLeft (pLineState+start, len);
    }
    else
    {
        TerminateLine(pLineData, pLineState, start);
    }

    if (pItem != 0)
    {
        pItem -> comWcode = TestLineHasCode (pLineState); // Comment without code ?
    }

    offset += len;
    TRACE(("Updated offset to %d\n", offset));
    return pItem;
}

// ----------------------------------------------------------------------------
// This Function is used to de-allocate memory in a InputStructure.
// A destructor wasn't used because other objects may also own the
// same memory.
//
// Parameters:
// pDelStruct : Pointer to a dynamically allocated InputStructure within
//              string data allocated.
//
static inline void CleanInputStruct (InputStruct* pDelStruct)
{
    if (pDelStruct != NULL)
    {
        delete[] pDelStruct -> pState;
        delete[] pDelStruct -> pData;
        delete pDelStruct;
    }
}


// ----------------------------------------------------------------------------
// Function is used within function DecodeLine() to de-allocate memory
// that it is currently using. This function is called upon a memory
// allocation failure.
//
// Parameters:
// PDelQueue : Pointer to a QueueList object which in general will contain
//             InputStructures.
//
static int DecodeLineCleanUp (QueueList* pDelQueue)
{
    // Don't implement destructor as other objects may be using the same
    // memory when using structure in output line processing (simple garbage collection)
    while (pDelQueue->status() > 0)
        CleanInputStruct ( reinterpret_cast<InputStruct*>(pDelQueue -> takeNext()) );
    return -1;
}

static int FindStartofComment(char *pLineState, CharState code = Comment)
{
    int it = -1;
    int n;
    for (n = 0; pLineState[n] != NullC; n++)
    {
        if (pLineState[n] == code)
        {
            it = n;
            break;
        }
    }
    return it;
}

static int FindEndofComment(char *pLineState)
{
    int it = -1;
    int n;
    for (n = 0; pLineState[n] == Comment; n++)
    {
        if (pLineState[n+1] == Comment
         && pLineState[n+2] != Comment)
        {
            it = n;
            break;
        }
    }
    return it;
}

// find punctuation delimiting code, e.g., curly braces or semicolon
static int FindPunctuation(char *pLineData, char *pLineState, char punct)
{
    int it = -1;
    int n;
    for (n = 0; pLineData[n] != NULLC; n++)
    {
        if (pLineState[n] == Normal
         && pLineData[n] == punct)
        {
            it = n;
            break;
        }
    }
    return it;
}

// ----------------------------------------------------------------------------
// When splitting a line (e.g., to move an open brace), check to see if the
// right fragment has a backslash escaping the newline.  If so, append one to
// the left fragment.
//
// pItem      : pointer to structure that we may append continuation to.
// pLineData  : Pointer to a line of a users input file (string).
// pLineState : Pointer to a state of a users input line (string).
//
static void splitContinuation(InputStruct *pItem, char *pLineData, char *pLineState, bool force)
{
    size_t len;

    if (force && !strcmp(pLineData, pItem->pData))
        force = False;

    if ((force || isContinuation(len, pLineData, pLineState))
     && !isContinuation(len, pItem->pData, pItem->pState))
    {
        char *s = new char[len + 4];
        strcpy(s, pItem->pData);
        strcat(s, " \\");
        delete[] pItem->pData;
        pItem->pData = s;

        s = new char[len + 4];
        strcpy(s, pItem->pState);
        s[++len] = Blank;
        s[++len] = Normal;
        s[++len] = NullC;
        delete[] pItem->pState;
        pItem->pState = s;
    }
}

// ----------------------------------------------------------------------------
// This function is a single pass decoder for a line of input code that
// is read from the user's input file. The function stores each part of a line,
// be it a comment (with its attributes), code, open brace, close brace, or
// blank line as a InputStructure, each InputStructure is stored within
// a Queue Object.
//
// Parameters:
// offset     : offset within original line's text of this component
// pLineData  : Pointer to a line of a users input file (string).
// pLineState : Pointer to a state of a users input line (string).
// QueueList* : Pointer to a QueueList object will contains all of
//              a lines basic elements. If this object doesn't contain
//              any elements, then it suggests there was a processing
//              problem.
//
// Return Values:
// int        : returns a error code.
//              -1 : Memory allocation failure
//               0 : No Worries
//
static int DecodeLine (bool afterSlash, int offset, char* pLineData, char *pLineState, QueueList* pInputQueue)
{
    int         SChar = -1;
    int         EChar = -1;
    size_t      commentLen = 0;

    // @@@@@@ C Comment processing, if over multiple lines @@@@@@
    if (*pLineState == Comment && !IsStartOfComment(pLineData, pLineState))
    {

        //#### Test to see if end terminating C comment has arrived !
        EChar = FindEndofComment(pLineState);

        if (EChar >= 0)
        {
            InputStruct* pItem = ExtractCCmt(offset, 0, EChar, pLineData, pLineState, CCom);

            if (pItem == NULL)
                return DecodeLineCleanUp (pInputQueue);

            TRACE_INPUT(pItem)
            pInputQueue->putLast (pItem);
        }
        else //##### Place output as comment without code (C comment terminator not found)
        {
            InputStruct* pTemp = ExtractCode (offset, pLineData, pLineState, CCom, False); // don't remove spaces !

            //#### Test if memory allocated
            if (pTemp == NULL)
                return DecodeLineCleanUp (pInputQueue);

            TRACE_INPUT(pTemp)
            pInputQueue->putLast (pTemp);

            return 0;
        }


    }// if multi-line C style comments


    // N.B Place this function here as to sure not to corrupt relative pointer
    // settings that may be used within pLinedata, and become altered through
    // using this routine.
    offset += StripSpacingLeftRight (pLineData, pLineState);

    //@@@@@@ Extract /* comment */ C type comments on one line
    SChar = FindStartofComment (pLineState);  // find start of C Comment
    if (SChar >= 0)
    {
        //##### Check if there is a ending C terminator comment string
        EChar = FindEndofComment(pLineState+SChar) + SChar;

        //##### If negative then comments are on multiple lines !
        if (EChar < 0)
        {
            InputStruct* pItem = ExtractCCmt(offset, SChar, -1, pLineData, pLineState, CCom);

            if (pItem == NULL)
                return DecodeLineCleanUp (pInputQueue);

            // make multi-line C style comments totally separate
            // from code to avoid some likely errors occurring if they
            // are shifted due to being over written by code.

            // apply recursion so that comment is last item placed
            // in queue !
            if (DecodeLine (afterSlash, offset, pLineData, pLineState, pInputQueue) != 0)
            {
                // problems !
                delete[] pItem -> pData;
                delete pItem;
                return -1;
            }

            TRACE_INPUT(pItem)
            pInputQueue->putLast (pItem);

            return 0; // no need to continue processing
        }
        else if (!isFinalComment(SChar, EChar, pLineData, pLineState))
        {
            InputStruct* pItem = ExtractCode(offset, pLineData, pLineState);
            TRACE_INPUT(pItem)
            pInputQueue->putLast (pItem);

            return 0; // no need to continue processing
        }
        else if (!isContinuation(commentLen, pLineData, pLineState))
        {
            InputStruct* pItem = ExtractCCmt(offset, SChar, EChar, pLineData, pLineState, CCom);

            if (pItem == NULL)
                return DecodeLineCleanUp (pInputQueue);

            if (ExtractedCCmtFragment(pLineData, pItem))
            {
                if (DecodeLine (afterSlash, offset, pLineData, pLineState, pInputQueue) != 0)
                    return DecodeLineCleanUp(pInputQueue);
                TRACE_INPUT(pItem)
                pInputQueue->putLast (pItem);
                return 0;
            }
            else
            {
                TRACE_INPUT(pItem)
                pInputQueue->putLast (pItem);
            }

        } //##### else

    }//##### If "/*" C comments present

    //##### Remove blank spacing from left & right of string
    offset += StripSpacingLeftRight (pLineData, pLineState);

    //@@@@@@ C++ Comment Processing !
    SChar = FindStartofComment (pLineState, Ignore);
    if (SChar >= 0)
    {
        int myoff = offset;
        InputStruct* pItem = ExtractCCmt(myoff, SChar, -1, pLineData, pLineState, CppCom);

        if (pItem == NULL)
            return DecodeLineCleanUp (pInputQueue);

        if (ExtractedCCmtFragment(pLineData, pItem))
        {
            if (DecodeLine (afterSlash, offset, pLineData, pLineState, pInputQueue) != 0)
                return DecodeLineCleanUp(pInputQueue);
            TRACE_INPUT(pItem)
            pInputQueue->putLast (pItem);
            return 0;
        }
        else
        {
            TRACE_INPUT(pItem)
            pInputQueue->putLast (pItem);
        }
    }

    //##### Remove blank spacing from left & right of string
    offset += StripSpacingLeftRight (pLineData, pLineState);

    //@@@@@@ #define (preprocessor extraction)
    if (pLineState[0] == POUNDC)
    {
        //#### create new queue structure !
        InputStruct* pItem = ExtractCode(offset, pLineData, pLineState, PreP);

        //#### Test if memory allocated
        if (pItem == NULL)
            return DecodeLineCleanUp (pInputQueue);

        TRACE_INPUT(pItem)
        pInputQueue->putLast (pItem);

        return 0; // no worries !
    }

    //################# Actual Code Extraction #################

    offset += StripSpacingLeftRight (pLineData, pLineState);

    //@@@@@@ Test what's left in line for L_CURL, and R_CURL braces

    SChar = FindPunctuation(pLineData, pLineState, L_CURL);
    EChar = FindPunctuation(pLineData, pLineState, R_CURL);

    Boolean testEnumType = False;
    if ( ((SChar >= 0) && (EChar >= 0)) && (SChar < EChar))
    {
        // test to see if there are multiple open/ close braces in enum
        // selective range
        // i.e. { if ( a == b ) { b = c } else { d = e } }
        int OBrace2 = FindPunctuation(pLineData+SChar+1, pLineState+SChar+1, L_CURL);

        if ( (OBrace2 < 0) || ((OBrace2 > EChar) && (OBrace2 >= 0)) )
           testEnumType = True;
    }

    //##### If condition correct, then make rest of line just code ! (e.g enum)
    // if no items in input queue, and no multiple open, close braces in
    // line then .... extract as enum.
    if ( (testEnumType != False) && (pInputQueue -> status () <= 0) )
    {
        //store code as enum type if follow-up condition is true
        EChar++;

        switch (pLineData[EChar]) // advance another char?
        {
            case SEMICOLON:
            case ',':
                EChar++;
                break;
            default:
                break;
        }

        char saveData  = pLineData[EChar];  pLineData[EChar]  = NULLC;
        char saveState = pLineState[EChar]; pLineState[EChar] = NULLC;

        InputStruct* pTemp = ExtractCode(offset, pLineData, pLineState);
        if (pTemp == NULL)
            return DecodeLineCleanUp (pInputQueue);

        pLineData[EChar]   = saveData;
        pLineState[EChar]  = saveState;
        splitContinuation(pTemp, pLineData, pLineState, afterSlash);

        TRACE_INPUT(pTemp)
        pInputQueue->putLast (pTemp);

        offset += EChar;
        ShiftLeft (pLineData,  EChar);
        ShiftLeft (pLineState, EChar);

        // restart decoding line !
        return DecodeLine (afterSlash, offset, pLineData, pLineState, pInputQueue);
        // end of recursive call !

    } // if L_CURL and R_CURL exit on same line

    //##### Determine extraction precedence !
    if ((SChar >= 0) && (EChar >= 0))
    {
        if (SChar > EChar)
            SChar = -1;
        else
            EChar = -1;
    }

    //##### Place whatever is before the open brace L_CURL, or R_CURL as code
    if ((SChar >= 0) || (EChar >= 0))
    {
        char saveCode;
        char saveFlag;
        int toSave = SChar >= 0 ? SChar : EChar;

        saveCode = pLineData[toSave];  pLineData[toSave]  = NULLC;
        saveFlag = pLineState[toSave]; pLineState[toSave] = NULLC;

        //#### Store leading code if any
        if (TestLineHasCode (pLineState) != False)
        {
           char* pTemp = NewString(pLineData);
           if (pTemp == NULL)
                return DecodeLineCleanUp (pInputQueue);

           //#### strip spacing is handled within extractCode routine.  This
           //#### means that pointers that are calculated before stripSpacing
           //#### function remain valid.

           InputStruct* pLeadCode = ExtractCode (offset, pTemp, pLineState);

           if (pLeadCode == NULL)
                return DecodeLineCleanUp (pInputQueue);

           pLineData[toSave]  = saveCode;
           pLineState[toSave] = saveFlag;
           splitContinuation(pLeadCode, pLineData+toSave+1, pLineState+toSave+1, afterSlash);

           TRACE_INPUT(pLeadCode)
           pInputQueue->putLast (pLeadCode);
           delete[] pTemp;
        }

        //##### Update main string
        offset += toSave;
        pLineData[toSave]  = saveCode; ShiftLeft (pLineData,  toSave);
        pLineState[toSave] = saveFlag; ShiftLeft (pLineState, toSave);
        TrimContinuation(pLineData, pLineState);

        // extract open/closing brace from code, and place brace as separate
        // line from code. And create new structure for code
        InputStruct* pTemp = 0;
        int          extractMode;

        if (pLineData[0] == L_CURL)
            extractMode = 1; // remove open brace
        else
            extractMode = 2; // remove close brace

        do
        {
            switch (extractMode)
            {
                case (1):    // remove open brace
                {
                    saveCode     = pLineData[1];  pLineData[1]  = NULLC;
                    saveFlag     = pLineState[1]; pLineState[1] = NULLC;

                    pTemp        = ExtractCode (offset, pLineData, pLineState, OBrace);//##### Define data type before storing

                    offset += 1;
                    pLineData[1] = saveCode;  ShiftLeft (pLineData,  1);
                    pLineState[1] = saveFlag; ShiftLeft (pLineState, 1);

                    splitContinuation(pTemp, pLineData, pLineState, afterSlash);
                    extractMode  = 3;            // apply recursive extraction

                    break;
                }

                case (2):   // remove close brace
                {
                    // test the type of close brace extraction !
                    // check for following code ...
                    // struct { int a, b, } aStructure;
                    //@@@@@@ Test what's left in line for L_CURL, and R_CURL braces

                    // start one after first char !
                    SChar = FindPunctuation(pLineData+1, pLineState+1, L_CURL);
                    EChar = FindPunctuation(pLineData+1, pLineState+1, R_CURL);

                    if ((SChar >= 0) || (EChar >= 0))
                    {
                        int mark;
                        if (pLineData[1] == SEMICOLON)     // if true, extract after char
                            mark = 2;
                        else
                            mark = 1;

                        saveCode = pLineData[mark]; pLineData[mark] = NULLC;
                        saveFlag = pLineState[mark]; pLineState[mark] = NULLC;

                        pTemp = ExtractCode (offset, pLineData, pLineState, CBrace);

                        offset += mark;
                        pLineData[mark] = saveCode;  ShiftLeft (pLineData,  mark);
                        pLineState[mark] = saveFlag; ShiftLeft (pLineState, mark);

                        splitContinuation(pTemp, pLineData, pLineState, afterSlash);
                        extractMode       = 3;       // apply recursive extraction
                    }
                    else // rest of data is considered as code !
                    {
                        pTemp     = ExtractCode (offset, pLineData, pLineState, CBrace);
                        splitContinuation(pTemp, pLineData, pLineState, afterSlash);
                        pLineState = NULL;      // leave processing !
                    }
                    break;
                }

                case (3):   // remove what is left on line as code.
                {
                    return DecodeLine (afterSlash, offset, pLineData, pLineState, pInputQueue);
                    // end of recursive call !
                }
            }// switch;

            //#### Test if memory allocated
            if (pTemp == NULL)
                return DecodeLineCleanUp (pInputQueue);

            TRACE_INPUT(pTemp)
            pInputQueue->putLast (pTemp); // store Item

        } while ((TestLineHasCode (pLineState) != False) && (pTemp != NULL));

    }
    else //##### Line contains either code, or spacing
    {
        //##### If nothing in string, and nothing stored in queue, then blank line
        if ((pLineData[0] == NULLC) && ((pInputQueue->status()) <= 0))
        {
            //##### implement blank space
            InputStruct* pTemp = ExtractCode (offset, pLineData, pLineState, ELine);

            if (pTemp == NULL)
                return DecodeLineCleanUp (pInputQueue);

            TRACE_INPUT(pTemp)
            pInputQueue->putLast (pTemp);
        }
        //##### If line has more than spacing/tabs then code
        else if (TestLineHasCode (pLineState) != False
         && strcmp(pLineData, "\\") != 0)
        {
            // implement blank space
            InputStruct* pTemp = ExtractCode (offset, pLineData, pLineState);

            if (pTemp == NULL)
                return DecodeLineCleanUp (pInputQueue);

            TRACE_INPUT(pTemp)
            pInputQueue->putLast (pTemp);
        }
    }
    return 0;  // no worries
}

// If the comment (fragment) doesn't begin a comment, we may be continuing
// a multi-line comment.  Adjust its indention to line up with the beginning
// to avoid a hanging-indent appearance.
static void dontHangComment(InputStruct *pIn, OutputStruct *pOut, QueueList* pLines)
{
    if (pIn -> dataType == CCom
     && strncmp(pOut -> pComment, ccom_begin, 2) != 0)
    {
        const char *text = SkipBlanks(pOut -> pComment);
        int count = pLines -> status();
        int length = text - pOut -> pComment;

        // If the comment text begins with an '*', increase the indention by
        // one unless it follows a comment-line that didn't begin with '*'.
        if (count > 0)
        {
            bool star = (*text == '*');

            while (count > 0)
            {
                OutputStruct *pTemp = reinterpret_cast<OutputStruct *>(pLines -> peek(count--));
                if (pTemp == 0
                 || pTemp -> pComment == 0)
                    continue;
                text = SkipBlanks(pTemp -> pComment);
                if (!strncmp(text, ccom_begin, 2))
                {
                    if (!star)
                    {
                        if (length > pTemp -> offset)
                            length = pTemp -> offset;
                    }
                    break;
                }
                if (*text != '*')
                    star = False;
            }
            ShiftLeft(pOut -> pComment, length);
            if (star)
                pOut -> indentSpace += 1;
        }
    }
}

// if nothing in queue, or next item isn't code then some sort of error
static bool inputIsCode(InputStruct *pItem)
{
    if (pItem != NULL)
    {
        switch (pItem -> dataType)
        {
            case Code:
            case OBrace:
            case CBrace:
            case PreP:
                return True;
            case CCom:
            case CppCom:
            case ELine:
            case NoType:
                break;
        }
    }

    warning ("\n#### ERROR ! Error In Line Construction !");
    warning ("\nExpected Some Sort Of Code ! Data Type Found = ");
    if (pItem == NULL)
        warning ("NULL");
    else
        warning ("%d", pItem -> dataType);

    return False;      // ##### incorrect dataType expected!
}

// ----------------------------------------------------------------------------
// Determine the type of preprocessor-control:
//  0 = unknown (leave it alone!)
//  1 = if-nesting
//  2 = if-unnesting
//  3 = nest/unnest
//  4 = other
static int typeOfPreP(InputStruct *pItem)
{
    static const struct {
        const char *keyword;
        int code;
    } table[] = {
        { "define",    4 },
        { "elif",      3 },
        { "else",      3 },
        { "endif",     2 },
        { "error",     4 },
        { "if",        1 },
        { "ifdef",     1 },
        { "ifndef",    1 },
        { "include",   4 },
        { "line",      4 },
        { "pragma",    4 },
        { "undef",     4 }
    };

    const char *s = pItem -> pData;

    // FIXME: we should be using the "state", just in case there's quotes
    if (*s == POUNDC)
    {
        s = SkipBlanks(s+1);
        for (size_t i = 0; i < TABLESIZE(table); i++)
        {
            if (CompareKeyword(s, table[i].keyword))
                return table[i].code;
        }
    }
    return 0;
}

// Returns the combination of brace-indent and preprocessor-indent
static int combinedIndent(int indentStack, int prepStack, Config userS)
{
    if (prepStack > userS.tabSpaceSize)
        return indentStack + prepStack - userS.tabSpaceSize;
    return indentStack;
}

// ----------------------------------------------------------------------------
// Analyze an OutputStruct to see if it began as a continued quoted-string.
bool ContinuedQuote(OutputStruct *pOut)
{
    if (pOut -> pCode != 0
     && pOut -> pCFlag != 0)
    {
        if (pOut -> pCFlag[0] == SQuoted
         || pOut -> pCFlag[0] == DQuoted)
        {
            return (pOut -> pCFlag[0] != pOut -> pCode[0]);
        }
    }
    return False;
}

// ----------------------------------------------------------------------------
// Check if the given fragment ends a statement (i.e., there's a semicolon that
// isn't quoted, and not within parentheses
static bool EndsStatement(OutputStruct *pOut)
{
    int nested = 0;

    for (int n = 0; pOut -> pCode[n] != NULLC; n++)
    {
        if (pOut -> pCFlag[n] == Normal)
        {
            if (pOut -> pCode[n] == L_PAREN)
                nested++;
            else
            if (pOut -> pCode[n] == R_PAREN)
                nested--;
            else
            if (nested == 0
             && pOut -> pCode[n] == SEMICOLON)
                return True;
        }
    }
    return False;
}

// ----------------------------------------------------------------------------
// Check for an "else" or "else if" that doesn't finish the statement on the
// given fragment.
static bool BeginsElseClause(OutputStruct *pOut)
{
    if (CompareKeyword(pOut -> pCode, "else"))
    {
        return !EndsStatement(pOut);
    }
    return False;
}

// ----------------------------------------------------------------------------
// Compute next curly-brace level after the given line
static void computeBraces(OutputStruct *pOut, int& level)
{
    TRACE_OUTPUT(pOut);
    if (pOut->pCode != 0
     && pOut->pCFlag != 0)
    {
        for (int n = 0; pOut->pCFlag[n] != NullC; ++n)
        {
            if (pOut->pCFlag[n] == Normal)
            {
                if (pOut->pCode[n] == L_CURL)
                    ++level;
                else if (pOut->pCode[n] == R_CURL)
                    --level;
            }
        }
    }
    else if (pOut->pBrace != 0
          && pOut->pBFlag != 0)
    {
        for (int n = 0; pOut->pBFlag[n] != NullC; ++n)
        {
            if (pOut->pBFlag[n] == Normal)
            {
                if (pOut->pBrace[n] == L_CURL)
                    ++level;
                else if (pOut->pBrace[n] == R_CURL)
                    --level;
            }
        }
    }

    // this could happen in a syntax-bending macro
    if (level < 0)
        level = 0;
}

// Check if the given output line was a preprocessor line that ended with
// a backslash.
static bool outputWasContinuedPreP (OutputStruct * pOut)
{
    if (pOut != 0 && pOut -> pType == PreP && isContinuation(pOut))
    {
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
// Function takes a QueueList object that contains InputStructure items, and
// uses these items to reconstruct a compressed version of a output line of
// code, comment, or both.
//
// Parameters:
// indentStack : Variable used to show how many spaces/tabs to indent when
//               creating a new OutputStructure.
// pInputQueue : Pointer to the InputStructure queue object.
// pOutputQueue: Pointer to the OutputStructure queue object.
// userS       : Structure that contains the users config settings.
//
// Return Values:
// int           : Return values of ...
//         0 = No problems
//        -1 = Memory allocation failure
//        -2 = Line construction error, unexpected type found.
static int ConstructLine (
    bool &indentPreP,
    int &prepStack,             // level of preprocessor-stack
    int& bracesLevel,           // curly-brace level for normal line
    int& preproLevel,           // curly-brace level for preprocesor line
    int& indentStack,
    bool& pendingElse,
    HangStruct& hang_state,
    SqlStruct& sql_state,
    QueueList* pInputQueue,
    QueueList* pOutputQueue,
    const Config& userS)
{
    InputStruct* pTestType = NULL;
    char *pendingComment = NULL;

    TRACE(("ConstructLine indentStack=%d\n", indentStack));

    while ( pInputQueue->status() > 0 )
    {
        int tokenIndent = indentStack;
        pTestType = reinterpret_cast<InputStruct*>(pInputQueue -> takeNext());

        OutputStruct* pOut = new OutputStruct(pTestType);

        if (pOut == NULL)
            return -1;

        // Special logic to make controls for MCCONFIG look "correct"
        if (pTestType -> dataType == CppCom)
        {
            const char *tst = SkipBlanks(pTestType -> pData + 2);
            static const char *keys[] = {
                "MCCONFIG{{",
                "MCCONFIG}}"
            };
            if (CompareKeyword(tst, keys[0]))
                indentPreP = True;
            if (CompareKeyword(tst, keys[1]))
                indentPreP = False;
        }

        int theType = pTestType -> dataType;
        for (int p = pOutputQueue -> status(); p > 0; --p)
        {
            OutputStruct *pq = reinterpret_cast<OutputStruct*>(pOutputQueue -> peek(p));
            if (pq != 0 && pq -> pCFlag != 0)
            {
                if (outputWasContinuedPreP (pq))
                {
                    theType = PreP;
                }
                break;
            }
        }

        switch (theType)
        {
            //@@@@@@@ Processing of C type comments /* comment */
            case (CCom):
            //@@@@@@@ Processing of C++ type comments // comment
            case (CppCom):
            {
                if (pTestType -> comWcode == True)  //##### If true then comment has code
                {
                    InputStruct *pNextItem = reinterpret_cast<InputStruct*>(pInputQueue -> peek(1));

                    if (pNextItem == 0)
                    {
                        // comment after nothing?
                        pOut -> pComment = pTestType -> pData;
                        dontHangComment(pTestType, pOut, pOutputQueue);
                        break;
                    }
                    if (!inputIsCode(pNextItem))
                        return -2;      // ##### incorrect dataType expected!

                    // if pData length overwrites comments then place comments on new line
                    if ( (indentStack + static_cast<int>(strlen (pNextItem -> pData))) > (userS.posOfCommentsWC) )
                    {
                        /*
                         * Check if this is a comment or fragment which we will
                         * delay after the code on the current line.  If so,
                         * indent it to align with code.
                         */
                        if ((pTestType->dataType == CCom
                          && !strncmp(pTestType->pData, ccom_begin, 2))
                         || (pTestType->dataType == CppCom))
                        {
                            pOut -> filler = userS.posOfCommentsWC;
                        } else {
                            pOut -> filler = indentStack + 1;
                        }
                        pOut -> pComment = pTestType -> pData;
                        TRACE(("@%d, Split Comment = %s:%d\n", __LINE__, pOut->pComment, pOut->thisToken));
                    }
                    else
                    {
                        pendingComment = pTestType -> pData;
                        TRACE(("@%d, Pending Comment = %s:%d\n", __LINE__, pendingComment, pOut->thisToken));
                        delete[] pTestType -> pState;
                        delete pTestType;
                        delete pOut;
                        continue;
                    }
                }
                else
                {
                    // JZAS Start
                    if (userS.leaveCommentsNC != False)
                    {
                        pOut -> indentSpace   = combinedIndent(indentStack, prepStack, userS);
                        if ((pOut -> offset >= userS.posOfCommentsWC)
                         && (pOut -> indentSpace < userS.posOfCommentsWC))
                        {
                            pOut -> indentSpace = 0;
                            pOut -> filler = userS.posOfCommentsWC;
                        }
                    }
                    else
                        pOut -> indentSpace   = userS.posOfCommentsNC;
                    // JZAS End

                    pOut -> pComment          = pTestType -> pData;
                    dontHangComment(pTestType, pOut, pOutputQueue);

                    TRACE(("@%d, Set Comment = %s:%d indent %d\n", __LINE__, pOut->pComment, pOut->thisToken, pOut->indentSpace));

                }// else a comment without code !
                break;

            }// case

            // @@@@@@ Processing of code (i.e k = 1; enum show {one, two};)
            case (Code):
            {
                pOut -> pCode  = pTestType -> pData;
                pOut -> pCFlag = pTestType -> pState;
                if (!ContinuedQuote(pOut))
                {
                    pOut -> indentSpace = combinedIndent(indentStack, prepStack, userS);

                    // Special case: align "else" and "if" if they're on successive lines
                    if (pendingElse
                     && CompareKeyword(pOut -> pCode, "if"))
                        pOut -> splitElseIf = True;
                    pendingElse = BeginsElseClause(pOut);
                }
                TRACE(("@%d, Set Code   = %s:%d indent %d\n", __LINE__, pOut->pCode, pOut->thisToken, pOut->indentSpace));

                break;
            }

            // @@@@@@ Processing of open brackets "{ k = 1;"
            case (OBrace):
            // @@@@@@ Processing of closed brackets "} k = 1;"
            case (CBrace):
            {
                pendingElse = False;

                // indent back before adding brace, some error checking
                if ((pTestType -> dataType == CBrace) && !userS.braceIndent)
                {
                    indentStack -= userS.tabSpaceSize;
                    if (indentStack < 0)
                        indentStack = 0;
                    tokenIndent = indentStack;
                }

                pOut -> indentSpace     = combinedIndent(indentStack, prepStack, userS);
                pOut -> pBrace          = pTestType -> pData;
                pOut -> pBFlag          = pTestType -> pState;
                TRACE(("@%d, Set pBrace = %s:%d indent %d\n", __LINE__, pOut->pBrace, pOut->thisToken, pOut->indentSpace));

                // ##### advance to the right !
                if (pTestType -> dataType == OBrace)
                    indentStack += userS.tabSpaceSize;

                // indent back before adding brace, some error checking
                if ((pTestType -> dataType == CBrace) &&  userS.braceIndent)
                {
                    indentStack -= userS.tabSpaceSize;
                    if (indentStack < 0)
                        indentStack = 0;
                    tokenIndent = indentStack;
                }
                break;
            }

            // @@@@@@ Blank Line spacing
            case (ELine):
            {
                delete[] pTestType -> pData;
                break;
            }

            // @@@@@@ Preprocessor Line !
            case (PreP):
            {
                pOut -> pType  = PreP;
                pOut -> pCode  = pTestType -> pData;
                pOut -> pCFlag = pTestType -> pState;
                if (userS.indentPreP)
                {
                    int resetPreproLevel = 0;

                    switch (typeOfPreP(pTestType))
                    {
                        case 0:
                            pOut -> indentSpace = indentStack + prepStack;
                            resetPreproLevel = preproLevel;
                            break;
                        case 1:
                            pOut -> indentSpace = indentStack + prepStack;
                            if (indentPreP != False)
                                prepStack += userS.tabSpaceSize;
                            break;
                        case 2:
                            if (indentPreP != False)
                            {
                                prepStack -= userS.tabSpaceSize;
                                if (prepStack < 0)
                                    prepStack = 0;
                            }
                            pOut -> indentSpace = indentStack + prepStack;
                            break;
                        case 3:
                            pOut -> indentSpace = indentStack + prepStack;
                            if (prepStack >= userS.tabSpaceSize)
                                pOut -> indentSpace -= userS.tabSpaceSize;
                            break;
                        case 4:
                            pOut -> indentSpace = indentStack + prepStack;
                            break;
                    }

                    // If this was a "#" line and not a continuation, reset
                    // the curly-brace level seen for indenting.
                    preproLevel = resetPreproLevel;
                }
                else
                    pOut -> indentSpace = 0;
                break;
            }

        } // switch

        if (pOut -> pCode == 0
         && pOut -> pBrace == 0)
        {
            delete[] pTestType -> pState;
        }
        else if (pendingComment != NULL)
        {
            TRACE(("@%d, Use Pending Comment = %s:%d\n", __LINE__, pendingComment, pOut->thisToken));
            pOut -> pComment = pendingComment;
            pOut -> filler = (userS.posOfCommentsWC - (tokenIndent + strlen (pTestType -> pData)));
            pendingComment = NULL;
        }

        hang_state.IndentHanging(pOut);

        if (userS.indent_sql)
            sql_state.IndentSQL(pOut);

        // set the braces level from a previous call to this function
        pOut->bracesLevel = bracesLevel;
        pOut->preproLevel = preproLevel;

        // compute the braces-level for the next line
        if (isPreproLine(pOut))
        {
            computeBraces(pOut, preproLevel);
            if (beginMultiLine(pOut))
                preproLevel += 1;
        }
        else
        {
            computeBraces(pOut, bracesLevel);
            preproLevel = 0;
        }

        pOutputQueue -> putLast (pOut);

        delete pTestType; // ##### Remove structure from memory, not its data
                          // ##### (i.e., char* pData), this is stored
                          // ##### in the output queue.

        if (indentStack < 0)
            indentStack = 0;

    } // while there are items to construct !

    return 0;

}

#if 0
// purge an Indent-stack
static void freeIndentStack(StackList* pImode)
{
    while ((pImode -> pop()) != 0)
        TRACE(("freeing...\n"));
}

// copy one Indent-stack to another.  The 'dst' stack is empty.
// FIXME: this just copies pointers -- must revise to copy data
static void copyIndentStack(StackList* src, StackList* dst)
{
    int n = 1;
    ANYOBJECT* temp;
    while ((temp = src -> peek(n++)) != 0)
    {
        //TRACE(("copying %d...\n", ((IndentStruct *)temp) -> pos));
        TRACE(("copying %d...\n", n-1));
        dst -> push(temp);
    }
}
#endif

// no extra indent immediately after any brace
static void resetSingleIndent(StackList* pIMode)
{
    int n = 1;
    IndentStruct* pIndentItem;

    while ((pIndentItem = reinterpret_cast<IndentStruct*>(pIMode -> peek(n++))) != 0)
    {
        TRACE(("...reset single-indent (%d)\n", pIndentItem->singleIndentLen));
        pIndentItem->singleIndentLen = 0;
        pIndentItem->attrib = noIndent;
    }
}

// returns index to the next OutputStruct iff it's an open brace (skipping
// comments), or zero.
static int peekIndexOBrace(QueueList* pLines, int first)
{
    OutputStruct* pTemp;
    while ((pTemp = reinterpret_cast<OutputStruct*>(pLines -> peek (first))) != 0)
    {
        if (pTemp -> pCode != 0)
            break;
        if ((pTemp -> pBrace != 0) && *(pTemp -> pBrace) == L_CURL)
            return first;
        first++;
    }
    return 0;
}

// Check for a chain of single-indents
static bool chainedSingleIndent(StackList* pIMode)
{
    int count;
    for (count = 1; count <= 2; count++)
    {
        IndentStruct* pIndentItem = reinterpret_cast<IndentStruct*>(pIMode -> peek(count));
        if (pIndentItem == 0
         || pIndentItem -> attrib != oneLine
         || pIndentItem -> singleIndentLen == 0)
            return False;
    }
    return True;
}

// If we've had a chain of single indents before a L_CURL, we have to shift
// the block to match the indent of the last indented code, because the
// preceding logic was indenting solely on the basis of curly braces.
static void shiftToMatchSingleIndent(QueueList* pLines, int indention, int first)
{
    int baseIn = (reinterpret_cast<OutputStruct*>(pLines -> peek (first))) -> indentSpace;
    int adjust = indention - baseIn;
    int state = 0;

    if (adjust > 0)
    {
        TRACE(("shiftToMatchSingleIndent, base %d adj %d\n", baseIn, adjust));
        for (int i = first; i <= pLines -> status() ; i++)
        {
            OutputStruct* pAlterLine  = reinterpret_cast<OutputStruct*>(pLines -> peek (i));
            if (pAlterLine == 0)
                break;

            // If there's an "else" immediately after the block-else, shift
            // it also.
            if (state == 1)
            {
                if (pAlterLine -> pCode == 0
                 || !BeginsElseClause(pAlterLine))
                    break;
                state = 2;
            }
            else
            if (pAlterLine -> pCode != 0)
            {
                if (pAlterLine -> pType == PreP)
                    continue;
            }
            else
            if (pAlterLine -> pBrace == 0)
            {
                continue;
            }

            if (pAlterLine -> indentSpace < baseIn)
                break;

            TRACE(("...shift %2d %2d :%s\n",
                pAlterLine -> indentSpace,
                pAlterLine -> indentSpace + adjust,
                pAlterLine -> pCode
                    ? pAlterLine -> pCode
                    : pAlterLine -> pBrace));

            pAlterLine -> indentSpace += adjust;

            if (pAlterLine -> indentSpace <= baseIn + adjust
             && pAlterLine -> pBrace != 0
             && pAlterLine -> pBrace[0] == R_CURL)
            {
                if (state == 0)
                    state = 1;
                else
                    break;
            }
        }
    }
}

// check if an output-struct contains code, so we can distinguish it from
// blank lines or comments
static inline bool OutputContainsCode(OutputStruct *pOut)
{
    return (pOut -> pCode != NULL || pOut -> pBrace != NULL);
}

// ----------------------------------------------------------------------------
// Function is used to indent single indented code such is found in if, while,
// else statements. Also handles case like statements within switches'.
//
// Parameters:
// pLines     : Pointer to the output queue.
// pIMode     : Pointer to indent type stack.
// userS      : User configuration (i.e indent spacing, position of comments)
//
// Return Values:
// QueueList*    : Pointer to the output queue (may have been reconstructed),
//                 returns NULL if failed to allocate memory
//
static QueueList* IndentNonBraceCode (QueueList* pLines, StackList* pIMode, const Config& userS, bool top)
{
    // if there are items to check !
    if ((pLines != NULL) && (pLines -> status () <= 0))
        return pLines;

    // If there are indent items to process !
    if (pIMode -> status() <= 0)
        return pLines;

    OutputStruct* pAlterLine  = reinterpret_cast<OutputStruct*>(pLines -> peek (1));

    if (pAlterLine -> pType == PreP)
       return pLines;

    IndentStruct* pIndentItem = reinterpret_cast<IndentStruct*>(pIMode -> pop());

    if ( ((pAlterLine -> pCode != NULL)     || ((pAlterLine -> pBrace != NULL) && (pIndentItem -> attrib == multiLine)) ) ||
         ((userS.leaveCommentsNC != False)  && ((pAlterLine -> pCode == NULL)  && (pAlterLine -> pComment != NULL))) )
    {
        bool adjusted = False;

        TRACE_OUTPUT(pAlterLine);
        switch (pIndentItem -> attrib)
        {
            case (blockLine):
            case (noIndent):
                break;

            // single indent
            case (oneLine):
            {
                int indentAmount;

                // Test for continued statements, suppressing indent until
                // it's complete.
                if (ContinuedQuote(pAlterLine))
                    indentAmount = 0;
                else
                if (pAlterLine -> indentHangs != 0)
                    indentAmount = 0;
                else
                    indentAmount = userS.tabSpaceSize;

                // Single line indentation calculation
                pAlterLine -> indentSpace += indentAmount;
                TRACE(("@%d, total indent %d (%d)\n", __LINE__, pAlterLine->indentSpace, indentAmount));
                break;
            }

            // indent of a case statement !
            case (multiLine):
            {
                // determine how many case-like items are stored within
                // list to determine how much to indent !
                int pTest;

                pAlterLine -> indentSpace += (userS.tabSpaceSize * (pIMode -> status()));

                // test if not another case, or default, if so, don't indent !
                pTest = LookupKeyword(pAlterLine -> pCode);
                if (pTest >= 0 && pIndentWords[pTest].code != multiLine)
                    pTest = -1;

                // check for closing braces to end case indention
                if ((pTest < 0) && (pAlterLine -> pBrace != NULL))
                {
                    if ((*(pAlterLine -> pBrace) == R_CURL) && (pAlterLine -> indentSpace == pIndentItem -> pos))
                    {
                        delete pIndentItem;
                        pIndentItem = NULL;
                    }
                }

                // indent as per normal !
                if ((pIndentItem != NULL) && (pTest < 0))
                {
                    pIMode -> push (pIndentItem); // ok to indent next item !
                    if (OutputContainsCode(pAlterLine))
                    {
                        pAlterLine -> indentSpace += userS.tabSpaceSize;
                        adjusted = True;
                    }
                    else if (pAlterLine -> pComment != NULL)
                    {
                        if (pAlterLine -> filler == 0)
                            pAlterLine -> indentSpace += userS.tabSpaceSize;
                    }
                    TRACE_OUTPUT(pAlterLine)
                }
                else if (pIndentItem != NULL)
                {
                    // if end single indent keyword found, check to see
                    // whether it is the correct one before removing it !
                    if ((pTest >= 0) && (pIndentItem -> pos+userS.tabSpaceSize < pAlterLine -> indentSpace))
                    {
                        pIMode -> push (pIndentItem); // ok to indent next item !
                        if (OutputContainsCode(pAlterLine))
                        {
                            pAlterLine -> indentSpace += userS.tabSpaceSize;
                            adjusted = True;
                        }
                        TRACE_OUTPUT(pAlterLine)
                    }
                    else
                    {
                        delete pIndentItem;
                        pIndentItem = NULL;
                    }
                }
                break;
            }

        }// switch

        // test if code has started to overwrite comments, and
        // not a case, or default statement ... if so, adjust queue !
        if ( ((pAlterLine -> pComment != NULL) && (pIndentItem != NULL)) &&
             ((pAlterLine -> pCode    != NULL) || (pAlterLine -> pBrace != NULL)) )
        {
            // alter filler size for comment spacing !
            pAlterLine -> filler -= userS.tabSpaceSize;

            // if less than 0, then code is overwriting comments !
            if (pAlterLine -> filler < 0)
            {
                // reconstruct queue !
                QueueList*    pNewQueue = new QueueList();
                OutputStruct* pNewItem  = new OutputStruct(pAlterLine);
                pAlterLine              = reinterpret_cast<OutputStruct*>(pLines -> takeNext());

                if (pNewItem == NULL)
                {
                    delete pNewQueue;
                    delete pIMode;
                    delete pLines;
                    return NULL;// out of memory
                }

                // load new structure
                pNewItem   -> filler      = userS.posOfCommentsWC;
                pNewItem   -> pComment    = pAlterLine -> pComment;
                pAlterLine -> filler      = 0; // set this to zero as not to create filler
                                               // spaces at line output time.

                if (adjusted) // we'll add this back later
                    pAlterLine -> indentSpace -= userS.tabSpaceSize;
                pAlterLine -> pComment    = NULL;

                // reconstruct queue !
                pNewQueue -> putLast (pNewItem);
                pNewQueue -> putLast (pAlterLine);

                // copy existing lines from old queue, into the newly created queue !
                while (pLines -> status () > 0)
                      pNewQueue -> putLast ( pLines -> takeNext() );

                delete pLines;
                pLines = pNewQueue; // reassign new queue object

            } // if overwriting comments
        }// if comments exist on same line as code

        // Remove single line indentation from memory, if current line
        // does contain a if, else, while ... type keyword
        if (pIndentItem == NULL)
            ;
        else
        if (pIndentItem -> attrib == oneLine)
        {
            int block = 0;

            // recursive function call !
            if (pIMode -> status() > 0)
                pLines = IndentNonBraceCode (pLines, pIMode, userS, False);

            TRACE(("#%d, brace=%p: %d\n", pAlterLine->thisToken, pAlterLine->pBrace, pIndentItem->attrib));
            TRACE(("@%d, push indent %d\n", __LINE__, pIndentItem -> singleIndentLen));
            pIMode -> push (pIndentItem);
            pIndentItem = NULL;

            if (top
             && (chainedSingleIndent(pIMode) || beginBlockLine(pLines))
             && (block = peekIndexOBrace(pLines, 2)) != 0)
            {
                shiftToMatchSingleIndent(pLines, pAlterLine->indentSpace, block);
            }
        }
        //FIXME (leak): delete pIndentItem;

    } // if code to process
    else if (pIndentItem != NULL)
    {
        TRACE(("#%d, brace=%p: %d\n", pAlterLine->thisToken, pAlterLine->pBrace, pIndentItem->attrib));
        // no indentation yet, maybe only blank line, or comment in case
        pIMode -> push (pIndentItem);

        // no extra indent immediately after any brace
        if (pAlterLine->pBrace != 0)
            resetSingleIndent(pIMode);
    }

    return pLines;
}


// ----------------------------------------------------------------------------
// Function allocates indent structures used to indent code that don't lie
// within braces, but should still be indented.
//
// Parameters:
// pIMode     : Pointer to a indent stack. Contains indent structures used to
//              indent code without braces
// pLines     : Pointer to output queue, stores semi-finished output code.
// userS      : User settings.
//
// Return Values:
// QueueList*    : Pointer to the output queue (may have been reconstructed),
//                 returns NULL if failed to allocate memory
//
static QueueList* IndentNonBraces (StackList* pIMode, QueueList* pLines, const Config& userS)
{
    const int minLimit = 2;             // used in searching output queue
                                        // for open braces
    // indent Items contained !
    if (pIMode -> status () > 0)
    {
        char*         pBraceOnNewLn = (reinterpret_cast<OutputStruct*>(pLines -> peek (1))) -> pBrace;
        char*         pBraceOnCurLn = (reinterpret_cast<OutputStruct*>(pLines -> peek (1))) -> pCode;
        IndentStruct* pTestBrace = reinterpret_cast<IndentStruct*>(pIMode -> pop());

        if ( (pBraceOnNewLn != NULL) &&
            ((pBraceOnNewLn[0] == L_CURL) && (pTestBrace -> attrib == oneLine)) )
        {
            delete pTestBrace;
        }
        else if (lastChar(pBraceOnCurLn) == L_CURL && (pTestBrace -> attrib == oneLine))
        {
            delete pTestBrace;
        }
        else
            pIMode -> push (pTestBrace);
    }

    //#### Indent code if code available, in a case statement
    if (pIMode -> status () > 0)
        pLines = IndentNonBraceCode (pLines, pIMode, userS, True);

    if (pLines -> status () < minLimit)
        return pLines;

    OutputStruct *pOut = reinterpret_cast<OutputStruct*>(pLines -> peek (1));

    // Cancel the indent applied by "else" to "if", and abandon the indent
    // that would be computed in this function for the code under "if".
    if (pOut -> splitElseIf)
    {
        pOut -> indentSpace -= userS.tabSpaceSize;
        return pLines;
    }

    // determine if current line has a single line keyword ! (if, else, while, for, do)
    const char*   pTestCode = pOut -> pCode;
    if (pTestCode != NULL)
    {
        int     findWord = LookupKeyword(pTestCode);

        // if keyword found, check if next line not a brace or, comment !

        // Test if code not NULL, and No Hidden Open Braces
        // FIXME: punctuation need not be at end of line
        if (findWord >= 0)
        {
            if (pIndentWords[findWord].code == multiLine)
            {
                const char *pTmp = SkipBlanks(pTestCode
                                 + strlen(pIndentWords[findWord].name));
                if (*pTmp != '\0'
                 && *pTmp != ':'
                 && lastChar(pTestCode) != ':')
                    findWord = -1;
            }
            switch (lastChar(pTestCode))
            {
                case L_CURL:
                case SEMICOLON:
                case R_CURL:
                    findWord = -1;
                    break;
                default:
                    break;
            }
        }

        // Test if open brace not located on next line
        if (findWord >= 0)
        {
            pTestCode = (reinterpret_cast<OutputStruct*>(pLines -> peek (minLimit))) -> pBrace;

            if ((pTestCode != NULL) && (pTestCode[0] == L_CURL))
                findWord = -1;    // Don't process line as a single indentation !
        }

        if (findWord >= 0)
        // create new structure !
        {
            IndentStruct* pIndent = new IndentStruct();

            // #### memory allocation error
            if (pIndent == NULL)
            {
                delete pLines;
                delete pIMode;
                return NULL;
            }

            // do indent mode for (if, while, for, else)
            if (pIndentWords[findWord].code == oneLine)
            {
                pIndent -> attrib = oneLine; // single indent !

                // determine how much to indent the next line of code !
                pIndent -> singleIndentLen = userS.tabSpaceSize;
                TRACE(("#%d: set single-indent to %d\n",
                      (reinterpret_cast<OutputStruct *>(pLines->peek(1)))->thisToken,
                      pIndent->singleIndentLen));
            }
            else // it's a case or other block-statement !
            {
                pIndent -> attrib = pIndentWords[findWord].code;
                pIndent -> pos    = ((reinterpret_cast<OutputStruct*>(pLines -> peek (1))) -> indentSpace) - userS.tabSpaceSize;
                TRACE(("#%d: set multi-indent %d, pos = %d\n",
                      (reinterpret_cast<OutputStruct *>(pLines->peek(1)))->thisToken,
                      pIndent->attrib,
                      pIndent->pos));
            }

            // place item on stack !
            pIMode -> push (pIndent);
        }
        else
        {
            // update pIMode indent queue, throw out single indents if
            // not needed (i.e multi line single if conditions)
            IndentStruct* pThrowOut = NULL;

            // Test code for single indentation, if semi-colon exits
            // within code, remove item from indent stack!
            pTestCode = (reinterpret_cast<OutputStruct*>(pLines -> peek (1))) -> pCode ;

            while (pIMode -> status () > 0)
            {
                pThrowOut = reinterpret_cast<IndentStruct*>(pIMode -> pop());

                if (pThrowOut -> attrib == multiLine)
                {
                    pIMode -> push (pThrowOut);
                    break;
                }    // Test single code indents for a semicolon !
                else if (lastChar(pTestCode) == SEMICOLON)
                    delete pThrowOut; // throw out the single indent item
                else
                {
                    pIMode -> push (pThrowOut); // Place item back on stack!
                    break; // Leave loop!
                }
            }
        }
    }
    return pLines;
}


// ----------------------------------------------------------------------------
// Check for a keyword which can precede a left curly-brace.
static bool KeyBeforeBrace (const char *word, int length)
{
    switch (length)
    {
    case 2:
        return (strncmp(word, "do", length)) ? False : True;
    case 4:
        return (strncmp(word, "else", length)
             && strncmp(word, "enum", length)) ? False : True;
    case 5:
        return (strncmp(word, "while", length)) ? False : True;
    }
    return False;
}

// ----------------------------------------------------------------------------
// Function reformats open braces to be on the same lines as the
// code that it's assigned (if possible).
//
// Parameters:
// pLines     : Pointer to a OutputStructure queue object
// userS      : Users configuration settings.
//
// Return Values:
// QueueList* : Returns a pointer to a newly constructed OutputStructure
//              queue, or the value of pLines if no work is needed.
//              The input pLines is freed unless it is the return-value.
//
static QueueList* ReformatBraces (QueueList* pLines, int first, const Config& userS)
{
    int           queueNum      = pLines -> status (); // get queue number

    int           findBrace;    // position in queue where first brace line is located
    int           findCode ;    // position in queue where next code line is located

    OutputStruct* pBraceLine    = NULL;
    OutputStruct* pCodeLine     = NULL;

    TRACE(("ReformatBraces(%d:%d)\n", first, queueNum));

    // Can't process less than two items (i.e. move brace from one line to next line to make one line )
    if (queueNum < 2 || first > queueNum)
    {
        return pLines;
    }

    // search forward through queue to find the first appearance of a brace !
    findBrace = first;
    while (findBrace <= queueNum)
    {
        pBraceLine = reinterpret_cast<OutputStruct*>(pLines -> peek (findBrace));

        if ((pBraceLine -> pBrace != NULL) && (pBraceLine -> pBrace[0] == L_CURL))
            break;              // leave queue search !
        else
            findBrace++;
    }

    if (findBrace > queueNum)   // open brace not found in queue !
    {
        return pLines;
    }

    // find out if there is a place to place the brace in the code that
    // is currently stored !
    for (findCode = findBrace-1; findCode >= first; findCode--)
    {
        if ((reinterpret_cast<OutputStruct*>(pLines -> peek (findCode))) -> pCode != NULL)
            break;              // found a code Line
    }

    if (findCode >= first)      // o.k found a line that has code !
    {
        OutputStruct* pNewItem   = NULL;
        char*         pNewMem    = NULL;

        // we're here to join braces, but must check if this instance must
        // remain split:
        bool splitBraces = False;
        if (pBraceLine->bracesLevel == 0
         && userS.topBraceLoc != False)
        {
            splitBraces = True;
        }
        else
        {
            int lastword = -1;
            int wordsize = 0;
            char lastchar = NullC;

            pCodeLine = reinterpret_cast<OutputStruct*>(pLines -> peek(findCode));
            if (pCodeLine->pCode != 0
             && pCodeLine->pCFlag != 0)
            {
                for (int n = 0; pCodeLine->pCode[n] != NullC; ++n)
                {
                    if (pCodeLine->pCFlag[n] == PreProc)
                    {
                        lastchar = NullC;
                        lastword = -1;
                        break;
                    }
                    else if (pCodeLine->pCFlag[n] == Normal)
                    {
                        lastchar = pCodeLine->pCode[n];
                        if (n > 0 && !isName(pCodeLine->pCode[n-1]))
                        {
                            lastword = -1;
                            wordsize = 0;
                        }
                        if (!isName(pCodeLine->pCode[n]))
                        {
                            lastword = -1;
                            wordsize = 0;
                        }
                        else if (lastword < 0 && isName(pCodeLine->pCode[n]))
                        {
                            lastword = n;
                            wordsize = 0;
                        }
                        if (lastword >= 0)
                            ++wordsize;
                    }
                    else if (pCodeLine->pCFlag[n] == DQuoted
                          || pCodeLine->pCFlag[n] == SQuoted)
                    {
                        lastchar = NullC;
                        lastword = -1;
                        wordsize = 0;
                    }
                }
                // we can join a left-curly after a right-paren, equals, or "else"
                if (lastchar != R_PAREN
                 && lastchar != '='
                 && (lastword < 0
                  || KeyBeforeBrace(pCodeLine->pCode + lastword, wordsize) == False))
                    splitBraces = True;
            }
        }

        // place top-level open braces on same line as code
        if (splitBraces || userS.braceLoc == True)
        {
            TRACE(("...leave brace, restart\n"));

            return ReformatBraces (pLines, findBrace + 1, userS);
        }

        QueueList* pNewLines = new QueueList();
        if (pNewLines == NULL)
        {
            return NULL;        // out of memory
        }

        // load newQueue with lines up to code line found !
        for (int loadNew = 1; loadNew < findCode; loadNew++)
            pNewLines -> putLast (pLines -> takeNext());

        // take code line that is going to be altered !
        pCodeLine = reinterpret_cast<OutputStruct*>(pLines -> takeNext ());

        // if code has comments, then it's placed on a new line !
        if (pCodeLine -> pComment != NULL)
        {
            // len of indent + code + space + brace
            int overWrite = pCodeLine -> indentSpace + strlen (pCodeLine -> pCode) + 1 + strlen (pBraceLine -> pBrace);
            if (overWrite >= userS.posOfCommentsWC) // if true then place comment on new line !
            {
                pNewItem = new OutputStruct(pCodeLine);
                if (pNewItem == NULL)
                    return NULL;

                pNewItem  -> filler      = userS.posOfCommentsWC;
                pNewItem  -> pComment    = pCodeLine -> pComment;
                pCodeLine -> pComment    = NULL;// make this NULL as not to be delete when
                                                // object destructor is called.
                pNewLines -> putLast (pNewItem);
            }
        }

        // place brace code onto new output structure !
        pNewItem = new OutputStruct(pCodeLine);
        // code + space + brace + nullc
        pNewMem  = new char [strlen (pCodeLine -> pCode) + strlen (pBraceLine -> pBrace) + 1 + 1];

        if ((pNewItem == NULL) || (pNewMem == NULL))
        {
            delete pCodeLine;
            delete pBraceLine;
            delete pLines;
            delete pNewLines;
            return NULL;        // out of memory
        }

        // concatenate code + space + brace // ### CHECK IT
        pNewMem = strcpy (pNewMem, pCodeLine -> pCode); // copy code
        strcpy (endOf(pNewMem), " ");                   // copy space
        strcpy (endOf(pNewMem), pBraceLine -> pBrace);  // copy brace

        // place attributes into queue
        pNewItem -> indentSpace = pCodeLine -> indentSpace;
        pNewItem -> pCode       = pNewMem;

        // Add comments to new code line if they exist
        if (pCodeLine -> pComment != NULL)
        {
            pNewItem  -> pComment = pCodeLine -> pComment;
            pCodeLine -> pComment = NULL;// make this NULL as not to be delete when
                                         // object destructor is called.

            // calculate filler spacing!
            pNewItem  -> filler   = userS.posOfCommentsWC - (pCodeLine -> indentSpace + strlen (pNewItem -> pCode));
        }

        // store newly constructed output structure
        pNewLines -> putLast (pNewItem);

        int start = pNewLines->status();

        // process brace Line !, create new output structure for brace comment
        if (pBraceLine -> pComment != NULL)
        {
            pNewItem = new OutputStruct(pBraceLine);

            if (pNewItem == NULL)
            {
                delete pCodeLine;
                delete pBraceLine;
                delete pLines;
                delete pNewLines;
                return NULL;// out of memory
            }

            // load comment
            pNewItem   -> pComment = pBraceLine -> pComment;
            pBraceLine -> pComment = NULL;

            // positioning comment, use filler - not indentSpace - as this
            // will become screw when using tabs ... fillers use spaces!
            pNewItem   -> filler  = userS.posOfCommentsWC;

            pNewLines  -> putLast (pNewItem);
        }

        delete pCodeLine;

        // copy existing lines from old queue, into the newly created queue !

        // copy all objects from pLines up to pBraceLine
        pCodeLine = reinterpret_cast<OutputStruct*>(pLines -> takeNext()); // read ahead rule
        while (pCodeLine != pBraceLine)
        {
              pNewLines -> putLast (pCodeLine);
              pCodeLine = reinterpret_cast<OutputStruct*>(pLines -> takeNext());
        }

        delete pCodeLine; // remove brace lines (in disguise)

        // code what's left in pLines queue to pNewLines !
        while ((pLines -> status ()) > 0 )
        {
              pNewLines -> putLast (pLines -> takeNext ());
        }

        // remove old queue object from memory, return newly constructed one
        delete pLines;

        // Test if there are any more open brace lines in pNewLine queue.
        // Do a recursive call to check.
        pLines = ReformatBraces (pNewLines, start, userS);
    }

    return pLines;
}


// ----------------------------------------------------------------------------
// Function reformats the spacing between functions, structures, unions, classes.
//
// Parameters:
// pOutFile : Pointer to the output FILE structure.
// pLines   : Pointer to the OutputStructure queue object.
// userS    : Users configuration settings.
// FuncVar  : Defines what type of mode the function is operating in.
// inBraces : Set to true if we're within curly-braces.
// pendingBlank : is used to control consecutive blank lines
//
// Return Values:
// QueueList*   : Pointer to the OutputStructure (sometimes altered)
// FuncVar      : Defines what mode function is currently in
//                0 = don't delete blank lines
//                1 = output blank lines
//                2 = delete blank OutputStructures in queue until code is reached.
//
static void FunctionSpacing (QueueList* pLines, const Config& userS, int& FuncVar, int &pendingBlank, bool& inBraces )
{
    inBraces = False;
    if (pLines -> status () > 0) // if there are items in the queue !
    {
        OutputStruct* pTestLine =  reinterpret_cast<OutputStruct*>(pLines -> peek (1));

        inBraces = (pTestLine -> indentSpace > 0) ? True : False;

        // check if end of function, structure, class has been reached !
        if ( ((FuncVar == 0) && (inBraces == False)) &&
             (pTestLine -> pBrace != NULL) )
        {
             if (pTestLine -> pBrace[0] == R_CURL
              && pTestLine -> pBrace[1] == NULLC)
             {
                FuncVar = 1; // add function spacing !
                return;
             }
        }

        if (FuncVar == 1)
        {
            // go into blank line output mode between functions!
            pendingBlank = userS.numOfLineFunc;
            FuncVar = 2;
        }

        if ( (FuncVar == 2) &&
             (((pTestLine -> pCode != NULL ) || (pTestLine -> pBrace != NULL)) || (pTestLine -> pComment != NULL)) )
              FuncVar = 0;
        else if (FuncVar == 2)
        {
            OutputStruct* dump = reinterpret_cast<OutputStruct*>(pLines -> takeNext()); // dump line from queue!
            delete dump;
        }
    }
}

// ----------------------------------------------------------------------------
// Putting an inter-function space before "#endif" looks ugly.  This is used
// to test for that condition.
static bool PreProcessorEndif(OutputStruct *pOut)
{
    if (pOut -> pType == PreP)
    {
        return CompareKeyword(SkipBlanks(pOut -> pCode + 1), "endif");
    }
    return False;
}

// ----------------------------------------------------------------------------
// Check if the first statement on the input-queue begins a preprocessor
// command, is a continuation of one, or is neither.
static int beginningPrePro (QueueList *pInputQueue, int Current)
{
    int n;
    int result = 0;
    InputStruct *pNextItem;

    if ((n = pInputQueue -> status()) != 0)
    {
        pNextItem = reinterpret_cast<InputStruct*>(pInputQueue -> peek(n));

        switch (pNextItem -> dataType)
        {
            case PreP:
                result = isContinuation(pNextItem) ? 1 : 0;
                break;
            case Code:
            case CBrace:
            case OBrace:
                result = Current ? (isContinuation(pNextItem) ? Current+1 : 0) : 0;
                break;
            default:
                result = 0;
                break;
        }
        if (Current && !result)
        {
            TRACE(("FIXME:PEEK(%d)%s\n", Current, isContinuation(pNextItem) ? " CONT" : ""));
            TRACE_INPUT(pNextItem)
        }
    }
    return result;
}

// We may convert leading whitespace in a comment back to tabs
static Boolean adjustLeadingSpaces(int fillMode, char *&notes, int &leading)
{
    if ((fillMode & 1) != 0 && notes != NULL && *notes == SPACE)
    {
        while (*notes == SPACE)
        {
            notes++;
            leading++;
        }
        return True;
    }
    return False;
}

// ----------------------------------------------------------------------------
// Function is used to expand OutputStructures contained within a queue to the
// user's output file.  Function also reformats braces, function spacing,
// braces indenting.
//
// Parameters:
// pOutFile  : Pointer to the users output FILE structure/handle
// pLines    : Pointer to the OutputStructures queue object
// FuncVar   : See FunctionSpacing()
// userS     : Users configuration settings.
// stopLimit : Defines how many OutputStructures remain within the Queue not
//             processed.
// pendingBlank : is used to control consecutive blank lines
//
// Return Values:
// FuncVar   : See FunctionSpacing()
// QueueList*: Pointer to the Output queue object (sometimes modified!)
//
// returns NULL if memory allocation failed
//
static QueueList* OutputToOutFile (FILE* pOutFile, QueueList* pLines, StackList* pIMode, int& FuncVar, const Config& userS, int stopLimit, int &pendingBlank)
{
    OutputStruct* pOut         = NULL;
    char*         pIndentation = NULL;
    char*         pFiller      = NULL;
    int           fillMode     = 2; // we can always use spaces
    bool          inBraces;

    // determine fill mode
    if (userS.useTabs == True)
        fillMode |= 1;          // set bit 0, tabs

    while (pLines -> status() > stopLimit) // stopLimit is used to search backward for L_CURL
    {
        // process function spacing !!!!!
        int testProcessing = pLines -> status();

        FunctionSpacing (pLines, userS, FuncVar, pendingBlank, inBraces );

        if (pLines -> status () < testProcessing) // line removed, test next line in buffer
             continue;

        // check indentation on case statements etc
        pLines = IndentNonBraces (pIMode, pLines, userS);
        if (pLines == NULL)
             return NULL;               //#### Memory Allocation Failure

        // reformat open braces if user option set !
        if (userS.topBraceLoc == False  // place open braces on same line as code
         || userS.braceLoc == False)    // place open braces on same line as code
        {
            pLines = ReformatBraces (pLines, 1, userS);
            if (pLines == NULL)
               return NULL;
        }

        pOut = reinterpret_cast<OutputStruct*>(pLines -> takeNext());

        TRACE_OUTPUT(pOut);

        // expand pOut structure to print to the output file
        if (!emptyString(pOut -> pCode)
         || !emptyString(pOut -> pBrace)
         || !emptyString(pOut -> pComment))
        {
            int mark;
            int in_code  = ((pOut -> pCode  != 0) ? strlen(pOut -> pCode) : 0)
                         + ((pOut -> pBrace != 0) ? strlen(pOut -> pBrace) : 0);
            int leading  = pOut -> indentSpace + (pOut -> indentHangs * userS.tabSpaceSize); // FIXME: indentHangs should use separate param
            char *notes  = pOut -> pComment;

            if (emptyString(notes))
                notes = NULL;

            // Check for a trailing C-comment fragment.  That must go before
            // any code!
            if (notes != NULL
             && (!emptyString(pOut -> pCode)
              || !emptyString(pOut -> pBrace))
             && strncmp(notes, ccom_begin, 2)
             && strncmp(notes, cppc_begin, 2))
             {
                adjustLeadingSpaces(fillMode, notes, leading);
                pIndentation = TabSpacing (fillMode,  0, leading, userS.tabSpaceSize);
                fprintf (pOutFile, "%s", pIndentation);
                delete[] pIndentation;

                fprintf (pOutFile, "%s\n", notes);
                notes = NULL;
            }

            // Just in case we only had a comment fragment, check again...
            if (!emptyString(pOut -> pCode)
             || !emptyString(pOut -> pBrace)
             || !emptyString(notes))
            {
                if (!adjustLeadingSpaces(fillMode, notes, leading)
                  && (in_code == 0)
                  && !emptyString(notes))
                {
                    if (pOut -> filler > leading)
                        leading = 0;
                }

                // compute the end-column of the code before filler, to use in
                // adjusting tab conversion.
                mark = leading + in_code;

                if (notes != 0)
                if (pOut -> filler > userS.posOfCommentsWC - mark)
                    pOut -> filler = userS.posOfCommentsWC - mark;
                if (pOut -> filler < 0)
                    pOut -> filler = 0;

                // 2-9-2 BTM - re-adjust location of braces & any comments that might follow them
                if ( pOut -> pBrace && userS.braceIndent2)
                {
                    leading += userS.tabSpaceSize;
                    if ( pOut->filler > userS.tabSpaceSize )
                    {
                        pOut->filler -= userS.tabSpaceSize;
                    }
                }

                if (ContinuedQuote(pOut))
                    pIndentation = NULL;
                else
                {
                    if (isPreproLine(pOut))
                    {
                        int next = pOut->preproLevel;
                        if (next > 0 && BeginsCurly(pOut))
                            --next;
                        leading += next * userS.tabSpaceSize;
                    }

                    pIndentation = TabSpacing (fillMode,  0, leading, userS.tabSpaceSize);
                }
                pFiller = TabSpacing (fillMode, mark, pOut -> filler, userS.tabSpaceSize);

                if (pendingBlank != 0)
                {
                    if (PreProcessorEndif(pOut))
                    {
                        pendingBlank = 0;
                        FuncVar = 0;
                    }
                    else
                    {
                        while (pendingBlank > 0)
                        {
                            fputc (LF, pOutFile); // output line feed!
                            pendingBlank--;
                        }
                    }
                }

                // Output data
                if (pIndentation != NULL)
                {
                    fprintf (pOutFile, "%s", pIndentation);
                    delete[] pIndentation;
                }

                if (pOut -> pCode != NULL)
                    fprintf (pOutFile, "%s", pOut -> pCode);

                if (pOut -> pBrace != NULL)
                    fprintf (pOutFile, "%s", pOut -> pBrace);

                if (pFiller != NULL)
                {
                    fprintf (pOutFile, "%s", pFiller);
                    delete[] pFiller;
                }

                if (notes != NULL)
                {
                    size_t len = strlen(notes);
                    fprintf (pOutFile, "%s", notes);
                    if (len > 0 && notes[len-1] == ESCAPE)
                        fprintf (pOutFile, " ");
                }

                fputc (LF, pOutFile); // output line feed!
            }
        }
        else
            pendingBlank = 1;

        // free memory
        delete pOut;
    }

    return pLines;
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Function will backspace the desired characters length according to
// numeric size.
static void backSpaceIt (unsigned long int num)
{
    unsigned long int size = 1;
    while (num >= size)
    {
        printf ("\b");
        size = size * 10;
    }
    printf ("\b"); // remove the trail zero, or space !
}

// Parameters:
// mode      : 1 = set new time, 2 = compare current time with now time
static unsigned long int GetStartEndTime (int mode)
{
    static time_t newTime;

    switch (mode)
    {
        case (1):
             newTime = time (NULL);
             return 0;
        case (2):
             return (time (NULL) - newTime);
    }

    return 0;
}

// ----------------------------------------------------------------------------
// Function is used to bundle all of the input, and output line processing
// functions together to create a final output file.
//
// Parameters:
// pInFile    : Pointer to the user's input FILE structure/handle.
// pOutFile   : Pointer to the user's output FILE structure/handle.
// userS      : User's configuration settings.
//
// Return Values:
// int        : Returns a value indicating whether there were any problems
//              in processing the input/output files.
//               0 = no worries.
//              -1 = memory allocation failure, or line construction failure
//
static int ProcessFile (FILE* pInFile, FILE* pOutFile, const Config& userS)
{
    const    char* errorMsg = "\n\n#### ERROR ! Memory Allocation Failed\n";
    const    unsigned long lineStep  = 10;     // line number update period (show every 10 lines)

    unsigned long int   lineNo       = 0;
    int                 EndOfFile    = 0;      // Var used by readline() to show eof has been reached
    char*               pData        = 0;

    int                 pendingBlank = 0;      // var used to control blank lines
    int                 indentStack  = 0;      // var used for brace spacing
    int                 indentStack2 = 0;      // save/restore "indentStack" for preprocessor lines

    QueueList*          pOutputQueue = new QueueList();
    StackList*          pIMode       = new StackList();
    StackList*          pIMode2      = new StackList();
    QueueList*          pInputQueue  = new QueueList();

    int                 FuncVar      = 0;      // variable used in processing function spacing !
    CharState           curState     = Blank;
    char*               lineState    = NULL;
    Boolean             codeOnLine   = False;
    bool                indentPreP   = False;
    bool                pendingElse  = False;
    int                 prepStack    = 0;
    int                 bracesLevel  = 0;
    int                 preproLevel  = 0;
    int                 in_prepro    = 0;
    HangStruct          hang_state;
    HtmlStruct          html_state;
    SqlStruct           sql_state;
    size_t              beforeSize;
    bool                beforeSlash  = False;
    bool                afterSlash;

    // Check memory allocated !
    if (((pOutputQueue == NULL) || (pIMode == NULL)) || (pInputQueue == NULL))
    {
           delete pOutputQueue;
           delete pInputQueue;
           delete pIMode;

        warning ("%s", errorMsg);
        return -1;
    }

    if (userS.output != False)
    {
        verbose ("\nFeed Me, Feed Me Code ...\n");
        verbose ("Number Of Lines Processed :  ");
    }

    GetStartEndTime (1);    // lets time the operation !

    while (! EndOfFile)
    {
        if (pData != 0)
            delete[] pData;

        pData = ReadLine (pInFile, EndOfFile);

        if (lineState != 0)
        {
            delete[] lineState;
            lineState = NULL;
        }

        if (pData != NULL)
        {
            lineNo++;
            if ( (lineNo % lineStep == 0) && (userS.output != False) )
            {
                if (lineNo > 0)
                    backSpaceIt (lineNo - lineStep); // reposition cursor ! Don't used gotoxy() for Unix compatibility

                printf ("%lu ", lineNo);
            }

            if (html_state.Active(pData))
            {
                if (EndOfFile)
                    break;
                // flush queue ...
                pOutputQueue = OutputToOutFile (
                        pOutFile,
                        pOutputQueue,
                        pIMode,
                        FuncVar,
                        userS,
                        0,
                        pendingBlank);
                fprintf(pOutFile, "%s\n", pData);
                continue;
            }

            ExpandTabs (pData,
                userS.tabSpaceSize,
                userS.deleteHighChars,
                userS.quoteChars,
                curState, lineState, codeOnLine);
            if (pData == NULL)
            {
                warning ("%s", errorMsg);
                delete pIMode;
                delete pInputQueue;
                delete pOutputQueue;
                return -1;
            }

            afterSlash = beforeSlash;
            beforeSlash = isContinuation(beforeSize, pData, lineState);

            if (DecodeLine (afterSlash, 0, pData, lineState, pInputQueue) == 0) // if there are input items to process
            {
                int old_prepro = in_prepro;
                bool restoreit = False;

                if ((in_prepro = beginningPrePro(pInputQueue, in_prepro)) != 0)
                {
                    if (in_prepro == 1)
                    {
                        TRACE(("FIXME: save indentStack: %d (%d)\n", in_prepro, indentStack));
                        //copyIndentStack(pIMode, pIMode2);
                        indentStack2 = indentStack;
                    }
                    else if (in_prepro == 2)
                    {
                        TRACE(("FIXME: increase indentStack\n"));
                        indentStack += userS.tabSpaceSize;
                    }
                }
                else if (old_prepro)
                {
                    restoreit = True;
                    if (old_prepro == 1)
                        indentStack += userS.tabSpaceSize;
                }

                int errorCode = ConstructLine (
                        indentPreP,
                        prepStack,
                        bracesLevel,
                        preproLevel,
                        indentStack,
                        pendingElse,
                        hang_state,
                        sql_state,
                        pInputQueue,
                        pOutputQueue,
                        userS);

                switch (errorCode)
                {
                    case (0)  : break;
                    case (-1) :
                    {
                        warning ("%s", errorMsg);
                        delete pIMode;
                        delete pInputQueue;
                        delete pOutputQueue;
                        return errorCode;
                    }

                    case (-2): // Construct line failed !
                    {
                        // output final line position
                        warning ("\nLast Line Read %ld", lineNo);
                        delete pIMode;
                        delete pInputQueue;
                        delete pOutputQueue;
                        return errorCode;
                    }

                    default:
                    {
                        warning ("\nSomething Weird %d\n", errorCode);
                        return errorCode;
                    }

                }

                pOutputQueue = OutputToOutFile (
                            pOutFile,
                            pOutputQueue,
                            pIMode,
                            FuncVar,
                            userS,
                            restoreit ? 0 : userS.queueBuffer,
                            pendingBlank );

                if (pOutputQueue == NULL)
                {
                    warning ("%s", errorMsg);
                    delete pIMode;
                    delete pInputQueue;
                    return -1; // memory allocation error !
                }

                if (restoreit)
                {
                    TRACE(("FIXME: restore indentStack (%d) to %d\n", indentStack, indentStack2));
                    //copyIndentStack(pIMode2, pIMode);
                    //freeIndentStack(pIMode2);
                    indentStack = indentStack2;
                }

            }
        } // if there's data available

    }// while data

    // flush queue ...
    pOutputQueue = OutputToOutFile (
            pOutFile,
            pOutputQueue,
            pIMode,
            FuncVar,
            userS,
            0,
            pendingBlank);

    // output final line position
    if (userS.output != False)
    {
        if ((lineNo > 0) && (lineNo > lineStep))
           backSpaceIt (lineNo - (lineNo % lineStep)); // reposition cursor

        printf ("%lu ", lineNo);
    }

    delete pIMode2;
    delete pIMode;
    delete pOutputQueue;
    delete pInputQueue;
    delete[] pData;
    delete[] lineState;

    if (userS.output != False)
    {
        unsigned long int t = GetStartEndTime (2);
        int    hours = (t / 60) / 60,
               mins  = (t / 60),
               secs  = (t % 60);
        verbose ("(In %d Hours %d Minutes %d Seconds)", hours, mins, secs);
    }

    return 0;
}

// ----------------------------------------------------------------------------
// locates programs configuration file via the PATH command.
// Should work for MS-DOS, and Unix environments. Amiga dos
// may fail because PATH is not the name of their path variable.
// pCfgName = Name of configuration file
// pCfgFile = reference to FILE structure pointer.
static void FindConfigFile (const char* pCfgName, FILE*& pCfgFile)
{
    const char* sepCharList = ";,:"; // dos, amigaDos, unix
    char* pSPath      = getenv ("PATH");
    char* pEPath      = NULL;
    char* pNameMem    = NULL;
    char  sepChar     = NULLC;
    const char* pathSepChar;
    char  backUp;
    int   count       = 0;

    // test to see if file is in current directory first !
    if ((pCfgFile = fopen(pCfgName, "r")) != NULL)
        return;

    // environment variable not found, lord knows what it is !
    if (pSPath == NULL)
       return;

    if ((pNameMem = new char[strlen (pSPath) + strlen (pCfgName)+2]) == NULL)
       return;

    // best guess in separating parameters !
    while (sepCharList[count] != NULLC)
    {
        pEPath   = endOf(pSPath);
        while ((*pEPath != sepCharList[count]) && (pEPath > pSPath))
              pEPath--;
        if (*pEPath == sepCharList[count])
        {
            sepChar = sepCharList[count];
            break; // leave loop
        }
        count++;
    }

    pEPath = pSPath;
    do
    {
          while ((*pEPath != sepChar) && (*pEPath != NULLC))
                pEPath++;

          backUp = *pEPath;
          *pEPath = NULLC;
          strcpy (pNameMem, pSPath);
          if (sepChar == SEMICOLON)
              pathSepChar = "\\"; // dumb dos's backwards path system !
          else
              pathSepChar = "/"; // everyone else uses this method

          // try to prevent segmentation errors !
          if (strlen (pNameMem) > 0)
             if (lastChar(pNameMem) != pathSepChar[0])
                 strcpy (endOf(pNameMem), pathSepChar);

          strcpy (endOf(pNameMem), pCfgName);
          *pEPath = backUp;
          if (*pEPath != NULLC)
          {
              pEPath++;
              pSPath = pEPath;
          }

          pCfgFile = fopen(pNameMem, "r");

    } while ((*pEPath != NULLC) && (pCfgFile == NULL));

    delete[] pNameMem;
}

// ----------------------------------------------------------------------------
// Front-end to the program, it reads in the configuration file, checks if there
// were any errors, and starts processing of the files.
//
// Parameters:
// argc       : command line parameter count
// argv[]     : array of pointers to command line parameters
//
// Return Values:
// int        : A non zero value indicates processing problem.
//
static int LoadnRun (int argc, char* argv[])
{
    const char* pNoFile    = "Couldn't Open, or Create File";
    bool  renamed          = False;
    char* pConfig          = NULL;
    char* pInFile          = NULL;
    char* pOutFile         = NULL;
    FILE* pInputFile       = NULL;
    FILE* pOutputFile      = NULL;
    FILE* pConfigFile      = NULL;

    int   errorNum         = 0;
    int   errorCode        = 0;

    Config settings        = {2,      // numOfLineFunc
                              4,      // tabSpaceSize
                              False,  // useTabs
                              50,     // posOfCommentsWC
                              0,      // posOfCommentsNC
                              False,  // leaveCommentsNC
                              False,  // quoteChars
                              3,      // deleteHighChars
                              True,   // topBraceLoc
                              True,   // braceLoc
                              True,   // output
                              10,     // queueBuffer
                              False,  // backUp
                              False,  // indentPreP
                              False,  // indent_sql
                              False,  // braceIndent
                              False}; // braceIndent2

/* ************************************************************************************
    // set defaults
    settings.numOfLineFunc    = 2;    // number of lines between functions
    settings.tabSpaceSize     = 4;    // number of spaces a tab takes up
    settings.useTabs          = False;// use tabs to indents rather than spaces
    settings.posOfCommentsWC  = 50;   // position of comments on line with code
    settings.posOfCommentsNC  = 0;    // position of comments on line
    settings.leaveCommentsNC  = False;// True = don't change the indentation of comments with code.
    settings.quoteChars       = False;// use tabs to indents rather than spaces
    settings.deleteHighChars  = 3;    // 0  = no check         , 1 = delete high chars,
                                      // 3  = delete high chars, but not graphics
    settings.topBraceLoc      = True; // Start top-level open braces on new line
    settings.braceLoc         = True; // Start open braces on new line
    settings.output           = True; // Set this true for normal program output
    settings.queueBuffer      = 10;   // Set the number if lines to store in memory at a time !
    settings.backUp           = False;// backup the original file, have output file become input file name !
************************************************************************************ */

    // Function processes command line parameters
    // FIRST read of the command line will search for the -fnc option to
    // read the configuration file, default is bcpp.cfg at current directory
    if (ProcessCommandLine (argc, argv, settings, pInFile, pOutFile, pConfig) != 0)
       return -1; // problems

    // *********************************************************************
    // Find default path and default configuration file name
    if (pConfig == NULL)
        FindConfigFile ("bcpp.cfg", pConfigFile);
    else
        pConfigFile = fopen(pConfig, "r");

    if (pConfigFile == NULL)
    {
        warning ("\nCouldn't Open Config File: %s\n", pConfig);
        warning ("Read Docs For Configuration Settings\n");
    }
    else
    {
        // LOAD CONFIG FILE !
        errorNum = SetConfig (pConfigFile, settings);

        // If output is via stdout, then turn out program output if it's
        // set within config file !
        if (pOutputFile == stdout)
           settings.output = False;

        if (settings.output != False)
           warning ("\n%d Error(s) In Config File.\n\n", errorNum);
    }

    // *********************************************************************

    // SECOND read of the command line will overwrite settings that may have
    // been changed by the previous command.  Lots of processing to overcome
    // this process, but hey it's a easy solution !

    pInFile = pOutFile = NULL;  // reset these so they can re-assigned again !
    if (ProcessCommandLine (argc, argv, settings, pInFile, pOutFile, pConfig) != 0)
       return -1; // problems

    // *********************************************************************

    // backup original filename!
    if ( ((settings.backUp != False) && (pInFile != NULL)) &&
          (pOutFile == NULL)) // Test if user wants an output file !
    {
        if (BackupFile (pInFile, pOutFile) != 0)
           return -1;
        renamed = True;
    }
    // **************************************************************

    // assign I/O streams
    if (pInFile == NULL)
        pInputFile = stdin;
    else
        pInputFile = fopen(pInFile, "r");

    if (pOutFile == NULL)
    {
        pOutputFile     = stdout;
        settings.output = False; // if using standard out, don't corrupt output
    }
    else
        pOutputFile = fopen(pOutFile, "wb");

    // Check user defined I/O streams
    if (pInputFile == NULL)
    {
        warning ("%s %s\n", pNoFile, pInFile);
        errorCode = -1;
    }

    if (pOutputFile == NULL)
    {
        warning ("%s %s\n", pNoFile, pOutFile);
        errorCode = -1;
    }

    if ((settings.output != False) && (errorCode == 0))
        errorNum = ShowConfig(settings);

    if (pConfigFile != NULL)
       fclose (pConfigFile);

    // #### Lets do some code crunching !
    if ((errorNum == 0) && (errorCode == 0))
        errorCode = ProcessFile (pInputFile, pOutputFile, settings);

    if (settings.output != False)
        verbose ("\nCleaning Up Dinner ... ");

    if (pInputFile != NULL)
        fclose (pInputFile);

    if (pOutputFile != NULL)
        fclose (pOutputFile);

    if (renamed)
    {
        RestoreIfUnchanged(pInFile, pOutFile);
        delete[] pInFile;
    }

    if (settings.output != False)
        verbose ("Done !\n");

    return errorCode;
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int main (int argc, char* argv[])
{
    return LoadnRun (argc, argv);
}
// The End :-).
