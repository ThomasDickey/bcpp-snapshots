// C(++) Beautifer V1.61 Unix/MS-DOS update !
// -----------------------------------------
//
// Program was written by Steven De Toni 1994 (CBC, ACBC).
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
// Removed a couple more bugs, optimised code (not that you notice it),
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
//  Not recognized as a comment becuase of the quotes chars!
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
//      This option was brought to you by a Candian user Justin Slootsky.
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
// Added location of program to use bcpp.cfg configuration without specifing
// it with options (i.e. uses PATH variable to locate).
// ###################################################################################
//
// Program Update : 27/11/95 V1.75 <Single if, while, do, statements fixed>
//
// Fixed the indentin of single statements so they dont become corrupt, and
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

#include "anyobj.h"            // Use ANYOBJECT base class
#include "baseq.h"             // QueueList class to store Output structures
#include "stacklis.h"          // StackList class to store indentStruct
#include "cmdline.h"           // ProcessCommandLine()
#include "config.h"            // SetConfig()

#include "bcpp.h"

// ----------------------------------------------------------------------------

enum indentAttr { noIndent=0, oneLine=1, multiLine=2, blockLine=3 };

const struct {
    char *name;
    indentAttr code;
} pIndentWords[] = {
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

enum  DataTypes { CCom = 1,   CppCom = 2, Code  = 3,
                  OBrace = 4, CBrace = 5, PreP = 6, ELine = 7
                };

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// This structure is used to store line data that is de-constructed from the
// user's input file.
class InputStruct : public ANYOBJECT
{
    public:

        DataTypes dataType;   // i.e "/*" or "*/"                  (1)
                              //     "//"                          (2)
                              //     Code (a = 5; , if (a == b) .. (3)
                              //     "{"                           (4)
                              //     "}"                           (5)
                              //     #define                       (6)
                              //     SPACES (nothing, blank line)  (7)

        Boolean comWcode;
                              //       -1 : True,  comment with code (for comment dataType)
                              //        0 : False, comment with no Code

        char* pData;          // pointer to queue data !
        char* pState;         // pointer to corresponding parse-state

        inline InputStruct (DataTypes theType)
        {
            dataType = theType;
            comWcode = False;
            pState = 0;       // only non-null for code
        }
};

#ifdef DEBUG
int   totalTokens;            // token count, for debugging
#endif

// ----------------------------------------------------------------------------
// The output structure is used to hold an entire output line. The structure is
// expanded with it's real tabs/spaces within the output function of the program.
class OutputStruct : public ANYOBJECT
{
    private:
    public:
           DataTypes pType;
#ifdef DEBUG
           int   thisToken;    // current token number
#endif
           int   indentSpace;  // num of spaces
           char* pCode;
           char* pState;
           char* pBrace;       // "}" or "{"
           int   filler;       // num of spaces
           char* pComment;

           // Constructor
           // Automate initalisation
           inline  OutputStruct  (DataTypes theType)
           {
                pType          = theType;
#ifdef DEBUG
                thisToken      = totalTokens++;
#endif
                pCode = pState = pBrace = pComment = NULL;
                indentSpace    = filler   = 0;
           }

           // Destructor
           // Automate destruction
           inline ~OutputStruct (void)
           {
                delete pCode;
                delete pState;
                delete pBrace;
                delete pComment;
           }
};

// This structure is used to hold indent data on non-brace code.
// This includes case statements, single line if's, while's, for statements...
class IndentStruct : public ANYOBJECT
{
    public:
           // attribute values ...
           // value 1 = indent code one position, until a ';' is found !
           //       2 = end on close brace, and at a position "pos"
           indentAttr    attrib;
           int           pos;

           // Indent double the amount for multiline single if, while ...
           // statements.
           int           singleIndentLen;
           Boolean       firstPass; // user to determine if more than a single line
                                    // if, while... statement.

    // constructor
    inline IndentStruct (void)
    {
        attrib          = noIndent;
        pos             = 0;
        singleIndentLen = 0; // number of spaces to indent !
        firstPass       = False;
    }
};

inline char *endOf(char *s)
{
   return (s + strlen(s));
}

inline char lastChar(char *s)
{
   return ((s != NULL) && (*s != NULLC)) ? *(endOf(s)-1) : (char)NULLC;
}

inline bool isName(char c)
{
   return isalnum(c) || (c == '_') || (c == '$');
}

static Boolean IsStartOfComment(char *pLineData, char *pLineState)
{
    if (pLineState[0] == Comment)
    {
        if (!strncmp(pLineData, "/*", 2))
            return True;
    }
    return False;
}

inline bool emptyString(char *s)
{
    return s == 0 || *s == 0;
}

inline void ShiftLeft(char *s, int len)
{
    strcpy (s, s + len);
}

inline const char *SkipBlanks(const char *s)
{
    while (isspace(*s))
        s++;
    return s;
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

int LookupKeyword(char *tst)
{
    size_t n;
    if (!emptyString(tst)) {
        for (n = 0; n < sizeof(pIndentWords)/sizeof(pIndentWords[0]); n++)
            if (CompareKeyword(tst, pIndentWords[n].name))
                return n;
    }
    return -1;
}

#ifdef DEBUG
static char *traceDataType(DataTypes theType)
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

static void traceInput(int line, InputStruct *pIn)
{
    TRACE((stderr, "@%d, %s%s:%s\n",
        line,
        traceDataType(pIn->dataType),
        pIn->comWcode ? " comWcode" : "",
        pIn->pData))
}

static void traceOutput(int line, OutputStruct *pOut)
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
#define TRACE_INPUT(pOut)  traceInput(__LINE__, pOut);
#define TRACE_OUTPUT(pOut) traceOutput(__LINE__, pOut);
#else
#define TRACE_INPUT(pOut)  /* nothing */
#define TRACE_OUTPUT(pOut) /* nothing */
#endif

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
void StripSpacingLeftRight (char* pLineData, char* pLineState, int mode = 3)
{
    int n;

    if (mode & 1)
    {
        for (n = -1; pLineState[n+1] == Blank; n++)
            ;

        if (n >= 0)
        {
            ShiftLeft(pLineData,  n+1);
            ShiftLeft(pLineState, n+1);
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
}


// ----------------------------------------------------------------------------
// Function returns a Boolean value that shows where code is contained within
// a string. Any chars within a string above space are considered code.
//
// Parameters:
// pLineData : Pointer to a string to process.
//
// Return Values:
// Boolean   : False = line has no code
//             True  = line has some sort of code
//
Boolean TestLineHasCode (char* pLineData)
{
    // save on segmentation error !
    if (pLineData == NULL)
       return False;

    unsigned char* pTest = (unsigned char*) pLineData;
    int      len         = strlen (pLineData);

    while ((*pTest < 33) && (*pTest > 0))
        pTest++;

    if ((*pTest != 0) && (len > 0)) // if not null ending char, and char has length
        return True;
    else
        return False;
}


// ----------------------------------------------------------------------------
// This function is used within function DecodeLine(), it creates a new
// InputStructure and stores whats is contained in pLineData string in
// the newly created structure.
//
// Parameters:
// pLineData  : Pointer to the string to store within the InputStructure.
// dataType   : Type of data that is to be stored within the InputStructure
//              see DataTypes enum.
//
// Return Values:
// InputStruct* : Returns a pointer to the newly constructed InputStructure,
//                returns a NULL value is unable to allocate memory.
//
InputStruct* ExtractCode (char*    pLineData,
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
            if ((pItem = new InputStruct(dataType)) != 0)
            {
                // strip spacing in new string before storing
                if (removeSpace != False)
                    StripSpacingLeftRight (pNewCode, pNewState);
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
// This Function is used to de-allocate memory in a InputStructure.
// A destructor wasn't used becuase other objects may also own the
// same memory.
//
// Parameters:
// pDelStruct : Pointer to a dynamically allocated InputStructure within
//              string data allocated.
//
inline void CleanInputStruct (InputStruct* pDelStruct)
{
    if (pDelStruct != NULL)
    {
        delete pDelStruct -> pState;
        delete pDelStruct -> pData;
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
int DecodeLineCleanUp (QueueList* pDelQueue)
{
    // Don't implement destructor as other objects may be using the same
    // memory when using structure in output line processing (simple garbage collection)
    while (pDelQueue->status() > 0)
        CleanInputStruct ( ((InputStruct*)pDelQueue -> takeNext()) );
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

inline void TerminateLine(char *pData, char *pState, size_t n)
{
    pData[n] = NULLC;
    pState[n] = NullC;
}

// ----------------------------------------------------------------------------
// This function is a single pass decoder for a line of input code that
// is read from the user's input file. The function stores each part of a line,
// be it a comment (with is attributes), code, open brace, close brace, or
// blank line as a InputStructure, each InputStructure is stored within
// a Queue Object.
//
// Parameters:
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
int DecodeLine (char* pLineData, char *pLineState, QueueList* pInputQueue)
{
    int         SChar = -1;
    int         EChar = -1;

    // @@@@@@ C Comment processing, if over multiple lines @@@@@@
    if (*pLineState == Comment && !IsStartOfComment(pLineData, pLineState))
    {

        //#### Test to see if end terminating C comment has arrived !
        EChar = FindEndofComment(pLineState);

        if (EChar >= 0)
        {
            size_t len = EChar + 2;
            char* pNewComment =  NewSubstring(pLineData, len);

            //#### Test if memory allocated
            if (pNewComment == NULL)
                return DecodeLineCleanUp (pInputQueue);

            ShiftLeft (pLineData, len); //##### Shift left string from current pos
            ShiftLeft (pLineState, len);

            //#### create new queue structure !
            InputStruct* pItem = new InputStruct(CCom);

            //#### Test if memory allocated
            if (pItem == NULL)
                return DecodeLineCleanUp (pInputQueue);

            pItem -> pData    = pNewComment;

            TRACE_INPUT(pItem)
            pInputQueue->putLast (pItem);
        }
        else //##### Place output as comment without code (C comment terminator not found)
        {
            InputStruct* pTemp = ExtractCode (pLineData, pLineState, CCom, False); // don't remove spaces !

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
    StripSpacingLeftRight (pLineData, pLineState);

    //@@@@@@ Extract /* comment */ C type comments on one line
    SChar = FindStartofComment (pLineState);  // find start of C Comment
    if (SChar >= 0)
    {
        //##### Check if there is a ending C terminator comment string
        EChar = FindEndofComment(pLineState+SChar) + SChar;

        //##### If negative then comments are on multiple lines !
        if (EChar < 0)
        {
            char* pNewComment = NewString(pLineData+SChar);

            //#### Test if memory allocated
            if (pNewComment == NULL)
                return DecodeLineCleanUp (pInputQueue);

            //##### Make it NULL so that comment is removed, from Line
            TerminateLine(pLineData, pLineState, SChar);

            InputStruct* pItem = new InputStruct(CCom);

            //#### Test if memory allocated
            if (pItem == NULL)
                return DecodeLineCleanUp (pInputQueue);

            pItem -> pData    = pNewComment;

            // make multi-line C style comments totally separate
            // from code to avoid some likely errors occurring if they
            // are shifted due to being over written by code.

            // apply recursion so that comment is last item placed
            // in queue !
            if (DecodeLine (pLineData, pLineState, pInputQueue) != 0)
            {
                // problems !
                delete pItem -> pData;
                delete pItem;
                return -1;
            }

            TRACE_INPUT(pItem)
            pInputQueue->putLast (pItem);

            return 0; // no need to continue processing
        }
        else
        {
            size_t len = EChar - SChar + 2;
            char* pNewComment =  NewSubstring(pLineData+SChar, len);

            //#### Test if memory allocated
            if (pNewComment == NULL)
                return DecodeLineCleanUp (pInputQueue);

            ShiftLeft (pLineData +SChar, len);
            ShiftLeft (pLineState+SChar, len);

            InputStruct* pItem = new InputStruct(CCom);
            //#### Test if memory allocated
            if (pItem == NULL)
                return DecodeLineCleanUp (pInputQueue);

            pItem -> pData    = pNewComment;
            pItem -> comWcode = TestLineHasCode (pLineData); // Comment without code ?
            TRACE((stderr, "@%d: set attrib to %d\n", __LINE__, pItem->comWcode))

            TRACE_INPUT(pItem)
            pInputQueue->putLast (pItem);

        } //##### else

    }//##### If "/*" C comments pressent

    //##### Remove blank spacing from left & right of string
    StripSpacingLeftRight (pLineData, pLineState);

    //@@@@@@ C++ Comment Processing !
    SChar = FindStartofComment (pLineState, Ignore);
    if (SChar >= 0)
    {
        char* pNewComment = NewString(pLineData+SChar);

        //#### Test if memory allocated
        if (pNewComment == NULL)
            return DecodeLineCleanUp (pInputQueue);

        //##### Terminate original string !
        TerminateLine(pLineData, pLineState, SChar);

        //#### create new queue structure !
        InputStruct* pItem = new InputStruct(CppCom);
        //#### Test if memory allocated
        if (pItem == NULL)
            return DecodeLineCleanUp (pInputQueue);

        pItem -> pData    = pNewComment;
        pItem -> comWcode = TestLineHasCode (pLineData); // Comment without code ?
        TRACE((stderr, "@%d: set comWcode to %d\n", __LINE__, pItem->comWcode))

        TRACE_INPUT(pItem)
        pInputQueue->putLast (pItem);
    }

    //##### Remove blank spacing from left & right of string
    StripSpacingLeftRight (pLineData, pLineState);

    //@@@@@@ #define (preprocessor extraction)
    if (pLineState[0] == POUNDC)
    {
        //#### create new queue structure !
        InputStruct* pItem = new InputStruct(PreP);

        //#### Test if memory allocated
        if (pItem == NULL)
            return DecodeLineCleanUp (pInputQueue);

        pItem -> pData    = NewString(pLineData);

        TRACE_INPUT(pItem)
        pInputQueue->putLast (pItem);

        return 0; // no worries !
    }

    //################# Actual Code Extraction #################

    StripSpacingLeftRight (pLineData, pLineState);

    //@@@@@@ Test what's left in line for L_CURL, and R_CURL braces

    SChar = FindPunctuation(pLineData, pLineState, L_CURL);
    EChar = FindPunctuation(pLineData, pLineState, R_CURL);

    Boolean testEnumType = False;
    if ( ((SChar >= 0) && (EChar >= 0)) && (SChar < EChar))
    {
        // test to see if there are multiple open/ close braces in enum
        // selective range
        // i.e. { if ( a == b ) { b = c } else { d = e } }
        int OBrace = FindPunctuation(pLineData+SChar+1, pLineState+SChar+1, L_CURL);

        if ( (OBrace < 0) || ((OBrace > EChar) && (OBrace >= 0)) )
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

        char BackUp = pLineData[EChar];
        pLineData[EChar] = NULLC;

        InputStruct* pTemp = ExtractCode(pLineData, pLineState);
        if (pTemp == NULL)
            return DecodeLineCleanUp (pInputQueue);

        TRACE_INPUT(pTemp)
        pInputQueue->putLast (pTemp);

        pLineData[EChar]   = BackUp;
        ShiftLeft (pLineData,  EChar);
        ShiftLeft (pLineState, EChar);

        // restart decoding line !
        return DecodeLine (pLineData, pLineState, pInputQueue);
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
        char backUp;
        int toSave = SChar >= 0 ? SChar : EChar;

        backUp = pLineData[toSave];
        pLineData[toSave] = NULLC;

        //#### Store leading code if any
        if (TestLineHasCode (pLineData) != False)
        {
           char* pTemp = NewString(pLineData);
           if (pTemp == NULL)
                return DecodeLineCleanUp (pInputQueue);

           //#### strip spacing is handled within extractCode routine. This
           //#### means that pointers that are calculated before stripSpacing function
           //#### remain valid.

           InputStruct* pLeadCode = ExtractCode (pTemp, pLineState);

           if (pLeadCode == NULL)
                return DecodeLineCleanUp (pInputQueue);

            TRACE_INPUT(pLeadCode)
           pInputQueue->putLast (pLeadCode);
           delete pTemp;
        }

        //##### Update main string
        pLineData[toSave] = backUp;
        ShiftLeft (pLineData,  toSave);
        ShiftLeft (pLineState, toSave);

        // extract open/closing brace from code, and place brace as seperate
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
                    backUp       = pLineData[1]; // contain a char, or NULLC char
                    pLineData[1] = NULLC;
                    pTemp        = ExtractCode (pLineData, pLineState, OBrace);//##### Define data type before storing
                    //#### update string
                    pLineData[1] = backUp;

                    ShiftLeft (pLineData,  1);
                    ShiftLeft (pLineState, 1);
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

                        backUp          = pLineData[mark];
                        pLineData[mark] = NULLC;

                        pTemp = ExtractCode (pLineData, pLineState, CBrace);
                        // #### update string;
                        pLineData[mark] = backUp;
                        ShiftLeft (pLineData,  mark);
                        ShiftLeft (pLineState, mark);

                        extractMode       = 3;       // apply recursive extraction
                    }
                    else // rest of data is considered as code !
                    {
                        pTemp     = ExtractCode (pLineData, pLineState, CBrace);
                        pLineData = NULL;            // leave processing !
                    }
                    break;
                }

                case (3):   // remove what is left on line as code.
                {
                    return DecodeLine (pLineData, pLineState, pInputQueue);
                    // end of recursive call !
                }
            }// switch;

            //#### Test if memory allocated
            if (pTemp == NULL)
                return DecodeLineCleanUp (pInputQueue);

            TRACE_INPUT(pTemp)
            pInputQueue->putLast (pTemp); // store Item

        } while ((TestLineHasCode (pLineData) != False) && (pTemp != NULL));

    }

    else //##### Line contains either code, or spacing
    {
        //##### If nothing in string, and nothing stored in queue, then blank line
        if ((pLineData[0] == NULLC) && ((pInputQueue->status()) <= 0))
        {
            //##### implement blank space
            InputStruct* pTemp = ExtractCode (pLineData, pLineState, ELine);

            if (pTemp == NULL)
                return DecodeLineCleanUp (pInputQueue);

            TRACE_INPUT(pTemp)
            pInputQueue->putLast (pTemp);
        }
        //##### If line has more than spacing/tabs then code
        else if (TestLineHasCode (pLineData) != False)
        {
            // implement blank space
            InputStruct* pTemp = ExtractCode (pLineData, pLineState);

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
void dontHangComment(InputStruct *pIn, OutputStruct *pOut, QueueList* pLines)
{
    if (pIn -> dataType == CCom
     && pOut -> indentSpace != 0
     && strstr(pOut -> pComment, "/*") == 0)
    {
        char *text = pOut -> pComment;
        int n;

        // If the comment text has as much leading whitespace as the indention,
        // remove that amount from the indention, to better retain the original
        // appearance of the comments.
        for (n = 0; n < pOut -> indentSpace && text[n] == SPACE; n++)
            ;
        if (n >= pOut -> indentSpace)
            pOut -> indentSpace = 0;
        else if (n != 0)
            pOut -> indentSpace -= n;

        // If the comment text begins with an '*', increase the indention by
        // one unless it follows a comment-line that didn't begin with '*'.
        if (text[n] == '*'
         && (n = pLines -> status()) > 0)
        {
            OutputStruct *pTemp = (OutputStruct*) pLines -> peek (n);
            if (pTemp != 0
             && pTemp -> pComment != 0)
                text = pTemp -> pComment;
            while (*text == SPACE)
                text++;
            if (*text == '*'
             || !strncmp(text, "/*", 2))
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
                break;
        }
    }

    fprintf (stderr, "\n#### ERROR ! Error In Line Construction !");
    fprintf (stderr, "\nExpected Some Sort Of Code ! Data Type Found = ");
    if (pItem == NULL)
        fprintf (stderr, "NULL");
    else
        fprintf (stderr, "%d", pItem -> dataType);

    return False;      // ##### incorrect dataType expected!
}

// ----------------------------------------------------------------------------
// Determine the type of preprocessor-control:
//  0 = unknown (leave it alone!)
//  1 = if-nesting
//  2 = if-unnesting
//  3 = nest/unnest
//  4 = other
int typeOfPreP(InputStruct *pItem)
{
    static const struct {
        char *keyword;
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
        for (size_t i = 0; i < sizeof(table)/sizeof(table[0]); i++) {
            if (CompareKeyword(s, table[i].keyword))
                return table[i].code;
        }
    }
    return 0;
}

// Returns the combination of brace-indent and preprocessor-indent
int combinedIndent(int indentStack, int prepStack, Config userS)
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
     && pOut -> pState != 0)
    {
        if (pOut -> pState[0] == SQuoted
         || pOut -> pState[0] == DQuoted)
        {
            return (pOut -> pState[0] != pOut -> pCode[0]);
        }
    }
    return False;
}

// ----------------------------------------------------------------------------
// Function takes a QueueList object that contains InputStructure items, and
// uses these items to reconstruct a compressed version of a output line of
// code, comment, or both.
//
// Parameters:
// indentStack : Variable used to show how many spaces/tabs to indent when
//              creating a new OutputStructure.
// pInputQueue : Pointer to the InputStructure queue object.
// pOutputQueue: Pointer to the OutputStructure queue object.
// userS       : Structure that contains the users config settings.
//
// Return Values:
// int           : Return values of ...
//         0 = No problems
//        -1 = Memory allocation failure
//        -2 = Line construction error, unexpected type found.
int ConstructLine (
    bool &indentPreP,
    int &prepStack,
    int& indentStack,
    bool& pendingElse,
    QueueList* pInputQueue,
    QueueList* pOutputQueue,
    Config userS)
{
    InputStruct* pTestType = NULL;
    char *pendingComment = NULL;

    while ( pInputQueue->status() > 0 )
    {
        int tokenIndent = indentStack;
        pTestType = (InputStruct*) pInputQueue -> takeNext();

        OutputStruct* pOut = new OutputStruct(pTestType -> dataType);

        if (pOut == NULL)
            return -1;

        // Special logic to make comments for MCCONFIG look "correct"
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

        switch (pTestType -> dataType)
        {
            //@@@@@@@ Processing of C type comments /* comment */
            case (CCom):
            //@@@@@@@ Processing of C++ type comments // comment
            case (CppCom):
            {
                if (pTestType -> comWcode == True)  //##### If true then comment has code
                {
                    InputStruct *pNextItem = (InputStruct*) pInputQueue -> peek(1);

                    // ##### Type Checking !!!!!
                    if (!inputIsCode(pNextItem))
                        return -2;      // ##### incorrect dataType expected!

                    // if pData length overwrites comments then place comments on newline
                    if ( (indentStack + (int)strlen (pNextItem -> pData)) > (userS.posOfCommentsWC) )
                    {
                        pOut -> filler = userS.posOfCommentsWC;
                        pOut -> pComment = pTestType -> pData;
                        TRACE((stderr, "@%d, Split Comment = %s:%d\n", __LINE__, pOut->pComment, pOut->thisToken))
                    }
                    else
                    {
                        pendingComment = pTestType -> pData;
                        TRACE((stderr, "@%d, Pending Comment = %s:%d\n", __LINE__, pendingComment, pOut->thisToken))
                        delete pTestType;
                        delete pOut;
                        continue;
                    }
                }
                else
                {
                    // JZAS Start
                    if (userS.leaveCommentsNC != False)
                        pOut -> indentSpace   = combinedIndent(indentStack, prepStack, userS);
                    else
                        pOut -> indentSpace   = userS.posOfCommentsNC;
                    // JZAS End

                    pOut -> pComment          = pTestType -> pData;
                    dontHangComment(pTestType, pOut, pOutputQueue);

                    TRACE((stderr, "@%d, Set Comment = %s:%d indent %d\n", __LINE__, pOut->pComment, pOut->thisToken, pOut->indentSpace))

                }// else a comment without code !
                break;

            }// case

            // @@@@@@ Processing of code (i.e k = 1; enum show {one, two};)
            case (Code):
            {
                pOut -> pCode = pTestType -> pData;
                pOut -> pState = pTestType -> pState;
                if (!ContinuedQuote(pOut))
                {
                    pOut -> indentSpace = combinedIndent(indentStack, prepStack, userS);

                    // Special case: align "else" and "if" if they're on successive lines
                    if (pendingElse
                     && CompareKeyword(pOut -> pCode, "if"))
                        pOut -> indentSpace -= userS.tabSpaceSize;
                    pendingElse = !strcmp(pOut -> pCode, "else");
                }
                TRACE((stderr, "@%d, Set Code   = %s:%d indent %d\n", __LINE__, pOut->pCode, pOut->thisToken, pOut->indentSpace))

                break;
            }

            // @@@@@@ Processing of open brackets "{ k = 1;"
            case (OBrace):
            // @@@@@@ Processing of closed brackets "} k = 1;"
            case (CBrace):
            {
                pendingElse = False;

                // indent back before adding brace, some error checking
                if (pTestType -> dataType == CBrace)
                {
                    indentStack -= userS.tabSpaceSize;
                    if (indentStack < 0)
                        indentStack = 0;
                    tokenIndent = indentStack;
                }

                pOut -> indentSpace     = combinedIndent(indentStack, prepStack, userS);
                pOut -> pBrace          = pTestType -> pData;
                TRACE((stderr, "@%d, Set pBrace = %s:%d indent %d\n", __LINE__, pOut->pBrace, pOut->thisToken, pOut->indentSpace))

                // ##### advance to the right !
                if (pTestType -> dataType == OBrace)
                    indentStack += userS.tabSpaceSize;

                break;
            }

            // @@@@@@ Blank Line spacing
            case (ELine):
            {
                delete pTestType -> pData;
                break;
            }

            // @@@@@@ Preprocessor Line !
            case (PreP):
            {
                pOut -> pCode       = pTestType -> pData;
                if (userS.indentPreP) {
                    switch (typeOfPreP(pTestType))
                    {
                        case 0:
                            pOut -> indentSpace = 0;
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
                }
                else
                    pOut -> indentSpace = 0;
                break;
            }

        } // switch

        if (pendingComment != NULL)
        {
            pOut -> pComment = pendingComment;
            pOut -> filler = (userS.posOfCommentsWC - (tokenIndent + strlen (pTestType -> pData)));
            pendingComment = NULL;
        }
        if (pOut -> pCode == 0)
            delete pTestType -> pState;

        pOutputQueue -> putLast (pOut);

        delete pTestType; // ##### Remove structure from memory, not its data
                          // ##### (i.e., char* pData), this is stored
                          // ##### in the output queue.

        if (indentStack < 0)
            indentStack = 0;

    } // while there are items to construct !

    return 0;

}


// no extra indent immediately after any brace
void resetSingleIndent(StackList* pIMode)
{
    int n = 1;
    IndentStruct* pIndentItem;

    while ((pIndentItem = (IndentStruct*) pIMode -> peek(n++)) != 0)
    {
        TRACE((stderr, "...reset single-indent (%d)\n", pIndentItem->singleIndentLen))
        pIndentItem->singleIndentLen = 0;
        pIndentItem->firstPass = False;
        pIndentItem->attrib = noIndent;
    }
}

// returns index to the next OutputStruct iff it's an open brace (skipping
// comments), or zero.
int peekIndexOBrace(QueueList* pLines, int first)
{
    OutputStruct* pTemp;
    while ((pTemp = (OutputStruct*) pLines -> peek (first)) != 0)
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
bool chainedSingleIndent(StackList* pIMode)
{
    int count;
    for (count = 1; count <= 2; count++)
    {
        IndentStruct* pIndentItem = (IndentStruct*) pIMode -> peek(count);
        if (pIndentItem == 0
         || pIndentItem -> attrib != oneLine
         || pIndentItem -> singleIndentLen == 0)
            return False;
    }
    return True;
}

// Return true if the current line is a blockLine preceding L_CURL.
bool beginBlockLine(QueueList* pLines)
{
    OutputStruct *pItem = (OutputStruct *) pLines -> peek(1);
    int findWord = LookupKeyword(pItem -> pCode);
    if (findWord >= 0)
    {
        if (pIndentWords[findWord].code == blockLine)
            return True;
    }
    return False;
}

// If we've had a chain of single indents before a L_CURL, we have to shift
// the block to match the indent of the last indented code, because the
// preceding logic was indenting solely on the basis of curly braces.
void shiftToMatchSingleIndent(QueueList* pLines, int indention, int first)
{
    int baseIn = ((OutputStruct*) pLines -> peek (first)) -> indentSpace;
    int adjust = indention - baseIn;

    if (adjust > 0)
    {
        TRACE((stderr, "shiftToMatchSingleIndent, base %d adj %d\n", baseIn, adjust))
        for (int i = first; i <= pLines -> status() ; i++)
        {
            OutputStruct* pAlterLine  = (OutputStruct*) pLines -> peek (i);
            if (pAlterLine == 0)
                break;

            if (pAlterLine -> pCode != 0)
            {
                if (pAlterLine -> pType == PreP)
                    continue;
            }
            else if (pAlterLine -> pBrace == 0)
            {
                continue;
            }

            if (pAlterLine -> indentSpace < baseIn)
                break;

            TRACE((stderr, "...shift %2d %2d :%s\n",
                pAlterLine -> indentSpace,
                pAlterLine -> indentSpace + adjust,
                pAlterLine -> pCode
                    ? pAlterLine -> pCode
                    : pAlterLine -> pBrace))

            pAlterLine -> indentSpace += adjust;

            if (pAlterLine -> indentSpace <= baseIn + adjust
             && pAlterLine -> pBrace != 0
             && pAlterLine -> pBrace[0] == R_CURL)
                break;
        }
    }
}

// check if an output-struct contains code, so we can distinguish it from
// blank lines or comments
inline bool OutputContainsCode(OutputStruct *pOut)
{
    return (pOut -> pCode != NULL || pOut -> pBrace != NULL);
}

// Check a comment-only line to see if it immediately follows a line of code
// with inline comment.  If so, "hang" it by indenting to the same level as
// other comments.
bool adjustHangingComment(QueueList *pLines)
{
// pNewItem   -> filler      = userS.posOfCommentsWC;
    TRACE((stderr, "adjustHangingComment:%d\n", pLines -> status()))
    TRACE_OUTPUT((OutputStruct*) pLines -> peek (1))
    TRACE_OUTPUT((OutputStruct*) pLines -> peek (2))
    TRACE_OUTPUT((OutputStruct*) pLines -> peek (3))
    return False;
}

// ----------------------------------------------------------------------------
// Function is used to indent single indented code such is found in if, while,
// else statements. Also handles case like statements within switchs'.
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
QueueList* IndentNonBraceCode (QueueList* pLines, StackList* pIMode, Config& userS, bool top)
{
    // if there are items to check !
    if ((pLines != NULL) && (pLines -> status () <= 0))
        return pLines;

    // If there are indent items to process !
    if (pIMode -> status() <= 0)
        return pLines;

    OutputStruct* pAlterLine  = (OutputStruct*) pLines -> peek (1);

    if (pAlterLine -> pType == PreP)
       return pLines;

    IndentStruct* pIndentItem = (IndentStruct*) pIMode -> pop();

    if ( ((pAlterLine -> pCode != NULL)     || ((pAlterLine -> pBrace != NULL) && (pIndentItem -> attrib == multiLine)) ) ||
         ((userS.leaveCommentsNC != False)  && ((pAlterLine -> pCode == NULL)  && (pAlterLine -> pComment != NULL))) )
    {
        bool adjusted = False;

        TRACE((stderr, "#%d, attrib=%d\n", pAlterLine->thisToken, pIndentItem->attrib))
        switch (pIndentItem -> attrib)
        {
            case (blockLine):
            case (noIndent):
                break;

            // single indent
            case (oneLine):
            {
                int indentAmount;

                // Test if current code has a ';' char at end of line,
                // if so then do full indent, else do 3 char indent!
                if (ContinuedQuote(pAlterLine))
                    indentAmount = 0;
                else
                if ((pAlterLine -> pCode != NULL) &&
                    lastChar(pAlterLine -> pCode) != SEMICOLON)
                {
                    pIndentItem -> firstPass    = True;
                    indentAmount = pIndentItem -> singleIndentLen;
                    TRACE((stderr, "#%d, use single-indent %d\n", pAlterLine->thisToken, pIndentItem->singleIndentLen))
                }
                else
                if (pIndentItem -> firstPass != False && top)
                    indentAmount = (userS.tabSpaceSize << 1);
                else
                    indentAmount = userS.tabSpaceSize;

                // Single line indentation calculation
                pAlterLine -> indentSpace += indentAmount;
                TRACE((stderr, "@%d, total indent %d (%d)\n", __LINE__, pAlterLine->indentSpace, indentAmount))
                break;
            }

            // indent of a case statement !
            case (multiLine):
            {
                // determine how many case-like items are stored within
                // list to determine how much to indent !
                int pTest;

                pAlterLine -> indentSpace += (userS.tabSpaceSize * (pIMode -> status()));

                // test if not another case, or default, if so, dont indent !
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
                        // FIXME: test for hanging comment
                        if (!adjustHangingComment(pLines)
                         && pAlterLine -> filler == 0)
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
                OutputStruct* pNewItem  = new OutputStruct(pAlterLine -> pType);
                pAlterLine              = (OutputStruct*) pLines -> takeNext();

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
        // does contain a if, else, while ... type keyowrd
        if (pIndentItem == NULL)
            ;
        else
        if (pIndentItem -> attrib == oneLine)
        {
            int block = 0;
 
            // recursive function call !
            if (pIMode -> status() > 0)
                pLines = IndentNonBraceCode (pLines, pIMode, userS, False);

            TRACE((stderr, "#%d, brace=%p: %d\n", pAlterLine->thisToken, pAlterLine->pBrace, pIndentItem->attrib))
            TRACE((stderr, "@%d, push indent %d\n", __LINE__, pIndentItem -> singleIndentLen))
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
        TRACE((stderr, "#%d, brace=%p: %d\n", pAlterLine->thisToken, pAlterLine->pBrace, pIndentItem->attrib))
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
QueueList* IndentNonBraces (StackList* pIMode, QueueList* pLines, Config userS)
{
    const int minLimit = 2;             // used in searching output queue
                                        // for open braces
    // indent Items contained !
    if (pIMode -> status () > 0)
    {
        char*         pBraceOnNewLn = ((OutputStruct*) pLines -> peek (1)) -> pBrace;
        char*         pBraceOnCurLn = ((OutputStruct*) pLines -> peek (1)) -> pCode;
        IndentStruct* pTestBrace = (IndentStruct*) pIMode -> pop();

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

    // determine if current line has a single line keyword ! (if, else, while, for, do)
    char*   pTestCode = ((OutputStruct*) pLines -> peek (1)) -> pCode;
    if (pTestCode != NULL)
    {
        int     findWord = LookupKeyword(pTestCode);

        // if keyword found, check if next line not a brace or, comment !

        // Test if code not NULL, and No Hidden Open Braces
        // FIXME: punctuation need not be at end of line
        switch (lastChar(pTestCode))
        {
            case L_CURL:
            case SEMICOLON:
            case R_CURL:
                pTestCode = NULL;
                break;
            default:
                break;
        }

        // Test if open brace not located on next line
        if ((pTestCode != NULL) && (findWord >= 0))
        {
            pTestCode = ((OutputStruct*) pLines -> peek (minLimit)) -> pBrace;

            if ((pTestCode != NULL) && (pTestCode[0] == L_CURL))
                pTestCode = NULL;    // Dont process line as a single indentation !
            else
                pTestCode = "E";     // make sure pointer is not null !
        }

        if ((findWord >= 0) && (pTestCode != NULL))
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
                pTestCode = ((OutputStruct*) pLines -> peek (1)) -> pCode;

                pIndent -> attrib = oneLine; // single indent !

                // determine how much to indent the next line of code !
                pIndent -> singleIndentLen = userS.tabSpaceSize;
                TRACE((stderr, "#%d: set single-indent to %d\n", ((OutputStruct *)pLines->peek(1))->thisToken, pIndent->singleIndentLen))
            }
            else // it's a case or other block-statement !
            {
                pIndent -> attrib = pIndentWords[findWord].code;
                pIndent -> pos    = (((OutputStruct*) pLines -> peek (1)) -> indentSpace) - userS.tabSpaceSize;
                TRACE((stderr, "#%d: set multi-indent %d, pos = %d\n", ((OutputStruct *)pLines->peek(1))->thisToken, pIndent -> attrib, pIndent -> pos))
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
            pTestCode = ((OutputStruct*) pLines -> peek (1)) -> pCode ;

            while (pIMode -> status () > 0)
            {
                pThrowOut = (IndentStruct*) pIMode -> pop();

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
// Function reformats open braces to be on the same lines as the
// code that it's assigned (if possible).
//
// Parameters:
// pLines     : Pointer to a OutputStructure queue object
// userS      : Users configuration settings.
//
// Return Values:
// QueueList* : Returns a pointer to a newly constructed OutputStructure
//              queue.
//
QueueList* ReformatBraces (QueueList* pLines, Config userS)
{
    QueueList*    pNewLines     = new QueueList();

    int           queueNum      = pLines -> status (); // get queue number
    int           findBrace;    // position in queue where first brace line is located
    int           findCode ;    // position in queue where next code line is located

    OutputStruct* pBraceLine    = NULL;
    OutputStruct* pCodeLine     = NULL;

    if (pNewLines == NULL)
    {
        delete pLines;
        return NULL;     // out of memory
    }

    // Cant process less than two items (i.e. move brace from one line to next line to make one line )
    if (queueNum  < 2)
    {
        delete pNewLines; // free object that wasn't used !
        return pLines;
    }

    // search forward through queue to find the first appearance of a brace !
    findBrace = 1;
    while (findBrace <= queueNum)
    {
        pBraceLine = (OutputStruct*) pLines -> peek (findBrace);

        if ((pBraceLine -> pBrace != NULL) && (pBraceLine -> pBrace[0] == L_CURL))
            break;                 // leave queue search !
        else
            findBrace++;
    }

    if (findBrace > queueNum)      // open brace not found in queue !
    {
        delete pNewLines;
        return pLines;
    }

    // find out if there is a place to place the brace in the code that
    // is currently stored !
    for (findCode = findBrace-1; findCode >= 1; findCode--)
    {
        if (((OutputStruct*) pLines -> peek (findCode)) -> pCode != NULL)
                             break; // found a code Line !
    }

    if (findCode > 0)   // o.k found a line that has code !
    {
        OutputStruct* pNewItem   = NULL;
        char*         pNewMem    = NULL;

        // load newQueue with lines up to code line found !
        for (int loadNew = 1; loadNew < findCode; loadNew++)
            pNewLines -> putLast (pLines -> takeNext());

        // take code line that is going to be altered !
        pCodeLine  = (OutputStruct*) pLines -> takeNext ();

        // calculate new brace position in queue
        findBrace                = findBrace - (queueNum - pLines -> status ());

        // get a pointer to the brace line, this is because of code lines that may still
        // exist within queue.
        pBraceLine = (OutputStruct*) pLines -> peek (findBrace);

        // if code has comments, then it's placed on a new line !
        if (pCodeLine -> pComment != NULL)
        {
            // len of indent + code + space + brace
            int overWrite = pCodeLine -> indentSpace + strlen (pCodeLine -> pCode) + 1 + strlen (pBraceLine -> pBrace);
            if (overWrite >= userS.posOfCommentsWC) // if true then place comment on new line !
            {
                pNewItem                = new OutputStruct(pCodeLine -> pType);
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
        pNewItem = new OutputStruct(pCodeLine -> pType);
        // code + space + brace + nullc
        pNewMem  = new char [strlen (pCodeLine -> pCode) + strlen (pBraceLine -> pBrace) + 1 + 1];

        if ((pNewItem == NULL) || (pNewMem == NULL))
        {
            delete pCodeLine;
            delete pBraceLine;
            delete pLines;
            delete pNewLines;
            return NULL;// out of memory
        }

        // concatenate code + space + brace // ### CHECK IT
        pNewMem = strcpy  (pNewMem, pCodeLine -> pCode);          // copy code
        strcpy (endOf(pNewMem), " ");                 // copy space
        strcpy (endOf(pNewMem), pBraceLine -> pBrace);// copy brace

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

        // process brace Line !, create new output structure for brace comment
        if (pBraceLine -> pComment != NULL)
        {
            pNewItem = new OutputStruct(pBraceLine -> pType);

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

            // positioning comment, use filler no indentSpace as this
            // will become screw when using tabs ... fillers use spaces!
            pNewItem   -> filler  = userS.posOfCommentsWC;

            pNewLines  -> putLast (pNewItem);
        }

        delete pCodeLine;

        // copy existing lines from old queue, into the newly created queue !

        // copy one all objects on pLines up to pBraceLine
        pCodeLine = (OutputStruct*) pLines -> takeNext(); // read ahead rule
        while (pCodeLine != pBraceLine)
        {
              pNewLines -> putLast (pCodeLine);
              pCodeLine = (OutputStruct*) pLines -> takeNext();
        }

        delete pCodeLine; // remove brace lines (indisguise)

        // code whats left in pLines queue to pNewLines !
        while ((pLines -> status ()) > 0 )
              pNewLines -> putLast (pLines -> takeNext ());;

        // remove old queue object from memory, return newly constructed one
        delete pLines;

        // Test if there are any more L_CURL open brace lines in pNewLine queue,
        // do a recursive call to check ! :-)
        pLines = ReformatBraces (pNewLines, userS);
        return pLines;
    }
    else
        delete pNewLines;   // couldn't find a code line in queue buffer!

    return pLines;
}


// ----------------------------------------------------------------------------
// Function reformats the spacing between functions, structures, unions, classes.
//
// Parameters:
// pOutFile : Pointer to the ouput FILE structure.
// pLines   : Pointer to the OuputStructure queue object.
// userS    : Users configuration settings.
// FuncVar  : Defines what type of mode the function is operating in.
//
// Return Values:
// QueueList*   : Pointer to the OuputStructure (sometimes altered)
// FuncVar      : Defines what mode function is currently in
//                0 = dont delete blank lines
//                1 = output blank lines
//                2 = delete blank OutputStructures in queue until code is reached.
//
void FunctionSpacing (QueueList* pLines, const Config& userS, int& FuncVar, int &pendingBlank )
{
    if (pLines -> status () > 0) // if there are items in the queue !
    {
        OutputStruct* pTestLine      =  (OutputStruct*) pLines -> peek (1);

        // check if end of function, structure, class has been reached !
        if ( ((FuncVar == 0) && (pTestLine -> indentSpace <= 0 )) &&
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
            OutputStruct* dump = (OutputStruct*) pLines -> takeNext(); // dump line from queue!
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
// Function is used to expand OutputStructures contained within a queue to the
// users output file. Function also reformats braces, function spacing,
// braces indenting.
//
// Parameters:
// pOutFile  : Pointer to the users output FILE structure/handle
// pLines    : Pointer to the OutputStructures queue object
// FuncVar   : See FunctionSpacing()
// userS     : Users configuration settings.
// stopLimit : Defines how many OutputStructures remain within the Queue not
//             processed.
//
// Return Values:
// FuncVar   : See FunctionSpacing()
// QueueList*: Pointer to the Output queue object (sometimes modified!)
//
// returns NULL if memory allocation failed
//
QueueList* OutputToOutFile (FILE* pOutFile, QueueList* pLines, StackList* pIMode, int& FuncVar, const Config& userS, int stopLimit, int &pendingBlank)
{
    OutputStruct* pOut         = NULL;
    char*         pIndentation = NULL;
    char*         pFiller      = NULL;
    int           fillMode     = 2; // we can always use spaces

    // determine fill mode
    if (userS.useTabs == True)
        fillMode |= 1;          // set bit 0, tabs

    while (pLines -> status() > stopLimit) // stopLimit is used to search backward for L_CURL
    {
        // process function spacing !!!!!
        int testProcessing = pLines -> status();

        FunctionSpacing (pLines, userS, FuncVar, pendingBlank );

        if (pLines -> status () < testProcessing) // line removed, test next line in buffer
             continue;

        // check indentation on case statements etc
        pLines = IndentNonBraces (pIMode, pLines, userS);
        if (pLines == NULL)
             return NULL;               //#### Memory Allocation Failure

        // reformat open braces if user option set !
        if (userS. braceLoc == False) // place open braces on same line as code
        {
            pLines = ReformatBraces (pLines, userS);
            if (pLines == NULL)
               return NULL;
        }

        pOut = (OutputStruct*) pLines -> takeNext();

        // ######## debug
        //gotoxy (1,1);
        //printf ("%d Indent    %d  Filler   \n", pOut -> indentSpace, pOut -> filler);
        TRACE_OUTPUT(pOut)

        // expand pOut structure to print to the output file
        if (!emptyString(pOut -> pCode)
         || !emptyString(pOut -> pBrace)
         || !emptyString(pOut -> pComment))
        {
            int nn = (pOut -> pCode == 0 && pOut -> pBrace == 0) ? fillMode : 2;
            int leading  = pOut -> indentSpace;
            char *notes  = pOut -> pComment;

            // convert leading whitespace in a comment back to tabs
            if (nn & 1 && notes != NULL && *notes == SPACE)
            {
                while (*notes == SPACE)
                {
                    notes++;
                    leading++;
                }
            }
            else
            if (emptyString(pOut -> pCode)
              && emptyString(pOut -> pBrace)
              && !emptyString(pOut -> pComment))
            {
                if (pOut -> filler > leading)
                    leading = 0;
            }
            pIndentation = TabSpacing (fillMode,  leading, userS.tabSpaceSize);
            pFiller      = TabSpacing (nn, pOut -> filler, userS.tabSpaceSize);

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
                delete pIndentation;
            }

            if (pOut -> pCode != NULL)
                fprintf (pOutFile, "%s", pOut -> pCode);

            if (pOut -> pBrace != NULL)
                fprintf (pOutFile, "%s", pOut -> pBrace);

            if (pFiller != NULL)
            {
                fprintf (pOutFile, "%s", pFiller);
                delete pFiller;
            }

            if (notes != NULL)
                fprintf (pOutFile, "%s", notes);

            fputc (LF, pOutFile); // output line feed!
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
void backSpaceIt (unsigned long int num)
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
unsigned long int GetStartEndTime (int mode)
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
int ProcessFile (FILE* pInFile, FILE* pOutFile, const Config& userS)
{
    const    char* errorMsg = "\n\n#### ERROR ! Memory Allocation Failed\n";
    const    int        lineStep     = 10;     // line number update period (show every 10 lines)

    unsigned long int   lineNo       = 0;
    int                 EndOfFile    = 0;      // Var used by readline() to show eof has been reached
    char*               pData        = 0;

    int                 pendingBlank = 0;      // var used to control blank lines
    int                 indentStack  = 0;      // var used for brace spacing

    QueueList*          pOutputQueue = new QueueList();
    StackList*          pIMode       = new StackList();
    QueueList*          pInputQueue  = new QueueList();

    int                 FuncVar      = 0;      // variable used in processing function spacing !
    CharState           curState     = Blank;
    char*               lineState    = NULL;
    Boolean             codeOnLine   = False;
    bool                indentPreP   = False;
    bool                pendingElse  = False;
    int                 prepStack    = 0;

    // Check memory allocated !
    if (((pOutputQueue == NULL) || (pIMode == NULL)) || (pInputQueue == NULL))
    {
           delete pOutputQueue;
           delete pInputQueue;
           delete pIMode;

        printf ("%s", errorMsg);
        return -1;
    }

    if (userS.output != False)
    {
        printf ("\nFeed Me, Feed Me Code ...\n");
        printf ("Number Of Lines Processed :  ");
    }

    GetStartEndTime (1);    // lets time the operation !

    while (! EndOfFile)
    {
        if (pData != 0)
            delete pData;

        pData = ReadLine (pInFile, EndOfFile);

        if (lineState != 0)
        {
            delete lineState;
            lineState = NULL;
        }

        if (pData != NULL)
        {
            lineNo++;
            if ( (lineNo % lineStep == 0) && (userS.output != False) )
            {
                    if (lineNo > 0)
                        backSpaceIt (lineNo - lineStep); // reposition cursor ! Don't used gotoxy() for Unix compatibility

                    printf ("%ld ", lineNo);
            }

            ExpandTabs (pData,
                userS.tabSpaceSize,
                userS.deleteHighChars,
                userS.quoteChars,
                curState, lineState, codeOnLine);
            if (pData == NULL)
            {
                printf ("%s", errorMsg);
                delete pIMode;
                delete pInputQueue;
                delete pOutputQueue;
                return -1;
            }

            if (DecodeLine (pData, lineState, pInputQueue) == 0) // if there are input items to process
            {
                    int errorCode = ConstructLine (
                            indentPreP,
                            prepStack,
                            indentStack,
                            pendingElse,
                            pInputQueue,
                            pOutputQueue,
                            userS);

                    switch (errorCode)
                    {
                        case (0)  : break;
                        case (-1) :
                        {
                            fprintf (stderr, "%s", errorMsg);
                            delete pIMode;
                            delete pInputQueue;
                            delete pOutputQueue;
                            return errorCode;
                        }

                        case (-2): // Construct line failed !
                        {
                            // output final line position
                            fprintf (stderr, "\nLast Line Read %ld", lineNo);
                            delete pIMode;
                            delete pInputQueue;
                            delete pOutputQueue;
                            return errorCode;
                        }

                        default:
                        {
                            fprintf (stderr, "\nSomething Weird %d\n", errorCode);
                            return errorCode;
                        }

                    }

                    pOutputQueue = OutputToOutFile (pOutFile, pOutputQueue,
                                                    pIMode  , FuncVar,
                                                    userS   , userS.queueBuffer,
                                                    pendingBlank );
                    if (pOutputQueue == NULL)
                    {
                        fprintf (stderr, "%s", errorMsg);
                        delete pIMode;
                        delete pInputQueue;
                        return -1; // memory allocation error !
                    }
            }
        } // if there's data available

    }// while data

    // flush queue ...
    pOutputQueue = OutputToOutFile (pOutFile, pOutputQueue,
                                    pIMode,   FuncVar,
                                    userS,    0,
                                    pendingBlank);

    // output final line position
    if (userS.output != False)
    {
        if ((lineNo > 0) && (lineNo > lineStep))
           backSpaceIt (lineNo - (lineNo % lineStep)); // reposition cursor

        printf ("%ld ", lineNo);
    }

    delete pIMode;
    delete pOutputQueue;
    delete pInputQueue;
    delete pData;
    delete lineState;

    if (userS.output != False)
    {
        unsigned long int t = GetStartEndTime (2);
        int    hours = (t / 60) / 60,
               mins  = (t / 60),
               secs  = (t % 60);
        printf ("(In %d Hours %d Minutes %d Seconds)", hours, mins, secs);
    }

    return 0;
}

// Function creates a backup of pInFile, by opening a
// new file with a ".bac" extenstion and copying the original
// data into it.
//
int BackupFile (char*& pInFile, char*& pOutFile)
{
    const char* pBackUp    = ".bac";

    pOutFile               = pInFile;
    pInFile                = new char[strlen(pOutFile)+5];
    char*    pLook         = NULL;
    FILE*    pInputFile    = NULL;
    FILE*    pOutputFile   = NULL;

    if (pInFile == NULL)
       return -1;

    strcpy (pInFile, pOutFile);

    // change input filename !
    pLook = endOf(pInFile)-1;
    while ( ((*pLook != '.')  && (pLook > pInFile)) &&
            ((*pLook != '\\') && (*pLook != '/')) )
                     pLook--;

    if  ((pLook <= pInFile) || ((*pLook == '\\') || (*pLook == '/')))
        strcpy (endOf(pInFile) - 1, pBackUp);
    else
        strcpy (pLook, pBackUp);

    // implement custom backup routine as one is NOT provided as standard !
    pInputFile  = fopen(pInFile,  "wb");
    pOutputFile = fopen(pOutFile, "rb");

    if ((pInputFile == NULL) || (pOutputFile == NULL))
    {
        fclose (pInputFile);
        fclose (pOutputFile);
        return -1;
    }

    int inChar = fgetc (pOutputFile);

    while (inChar >= 0)
    {
        fputc (inChar, pInputFile);
        inChar = fgetc (pOutputFile);
    }

    fclose (pInputFile);
    fclose (pOutputFile);

    return 0;
}


// ----------------------------------------------------------------------------
// locates programs configuration file via the PATH command.
// Should work for MS-DOS, and Unix enviroments. Amiga dos
// may fail because PATH is not the name of their path variable.
// pCfgName = Name of configuration file
// pCfgFile = reference to FILE struture pointer.
void FindConfigFile (char* pCfgName, FILE*& pCfgFile)
{
    char* sepCharList = ";,:"; // dos, amigaDos, unix
    char* pSPath      = getenv ("PATH");
    char* pEPath      = NULL;
    char* pNameMem    = NULL;
    char  sepChar     = NULLC;
    char* pathSepChar;
    char  backUp;
    int   count       = 0;

    // test to see if file is in current directory first !
    if ((pCfgFile = fopen(pCfgName, "rb")) != NULL)
        return;

    // environment variable not found, lord knows what it is !
    if (pSPath == NULL)
       return;

    if ((pNameMem = new char[strlen (pSPath) + strlen (pCfgName)+2]) == NULL)
       return;

    // best guess in seperating parameters !
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

          pCfgFile = fopen(pNameMem, "rb");

    } while ((*pEPath != NULLC) && (pCfgFile == NULL));

    delete pNameMem;
}

// ----------------------------------------------------------------------------
// Front-end to the program, it reads in the configuration file, checks if there
// were any errors, and starts processing of the files.
//
// Parameters:
// argc       : comand line parameter count
// argv[]     : array of pointers to command line parameters
//
// Return Values:
// int        : A non zero value indicates processing problem.
//
int LoadnRun (int argc, char* argv[])
{
    const char* choices[2] = {"Yes", "No"};
    const char* pNoFile    = "Couldn't Open, or Create File";
    char* pConfig          = NULL;
    char* pInFile          = NULL;
    char* pOutFile         = NULL;
    FILE* pInputFile       = NULL;
    FILE* pOutputFile      = NULL;
    FILE* pConfigFile      = NULL;

    int   errorNum         = 0;
    int   errorCode        = 0;

    Config settings        = {2, 4, False, 50, 0, False, False, 3, True, True, 10, False};

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
        pConfigFile = fopen(pConfig, "rb");

    if (pConfigFile == NULL)
    {
        fprintf (stderr, "\nCouldn't Open Config File: %s\n", pConfig);
        fprintf (stderr, "Read Docs For Configuration Settings\n");
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
           fprintf (stderr, "\n%d Error(s) In Config File.\n\n", errorNum);
    }

    // *********************************************************************

    // SECOND read of the command line will overwrite settings that may have
    // been changed by the previous command.  Lots of processing to over come
    // this process, but hey it's a easy solution !

    pInFile = pOutFile = NULL;  // reset these so they can re-assigned again !
    if (ProcessCommandLine (argc, argv, settings, pInFile, pOutFile, pConfig) != 0)
       return -1; // problems

    // *********************************************************************

    // backup original filename!
    if ( ((settings.backUp != False) && (pInFile != NULL)) &&
          (pOutFile == NULL)) // Test if user wants an output file !
    {
        if (settings.output != False)
           printf ("Please Wait, Backing Up Data ... ");

        if (BackupFile (pInFile, pOutFile) != 0)
           return -1;

        if (settings.output != False)
           printf ("Done!\n\n");
    }
    // **************************************************************

    // assign I/O streams
    if (pInFile == NULL)
        pInputFile = stdin;
    else
        pInputFile = fopen(pInFile, "rb");

    if (pOutFile == NULL)
    {
        pOutputFile     = stdout;
        settings.output = False; // if using standed out, don't currupt output
    }
    else
        pOutputFile = fopen(pOutFile, "wb");

    // Check user defined I/O streams
    if (pInputFile == NULL)
    {
        fprintf (stderr, "%s %s\n", pNoFile, pInFile);
        errorCode = -1;
    }

    if (pOutputFile == NULL)
    {
        fprintf (stderr, "%s %s\n", pNoFile, pOutFile);
        errorCode = -1;
    }

    // **************************************************************

    if ((settings.output != False) && (errorCode == 0))
    {
        printf ("Function Line Spacing              : %d\n", settings.numOfLineFunc);
        printf ("Use Tabs In Indenting              : %s\n", choices[settings.useTabs+1]);
        printf ("Indent Spacing Length              : %d\n", settings.tabSpaceSize);
        printf ("Comments With Code                 : %d\n", settings.posOfCommentsWC);
        if (settings.leaveCommentsNC != False)
            printf ("Comments With No Code              : Indented According To Code\n");
        else
            printf ("Comments With No Code              : %d\n", settings.posOfCommentsNC);
        printf ("Remove Non-ASCII Chars             : ");

        switch (settings.deleteHighChars)
        {
            case (0):
                 printf ("No\n");
                 break;
            case (1):
                 printf ("Yes\n");
                 break;
            case (3):
                 printf ("Yes But Not Graphic Chars\n");
                 break;
            default:
                    fprintf (stderr, "#### ERROR : Unexpected Value %d", settings.deleteHighChars);
                    errorNum++;
        }

        printf ("Non-Ascii Chars In Quotes To Octal : %s\n", choices[settings.quoteChars+1]);
        printf ("Open Braces On New Line            : %s\n", choices[settings.braceLoc+1]);
        printf ("Program Output                     : %s\n", choices[settings.output+1]);
        printf ("Internal Queue Buffer Size         : %d\n", settings.queueBuffer);

        if (errorNum > 0)
        {
            fprintf (stderr, "\nDo You Wish To Continue To Process %s File [Y/N] ?", pInFile);

            int userKey = 0;
            do
            {
                userKey = getc(stdin);
                userKey = (userKey & 223); // change key to upper case!
                if (userKey == 'Y')
                   errorNum = 0;

            } while ((userKey != 'Y') && (userKey != 'N'));
        }
    }// display user settings !

    if (pConfigFile != NULL)
       fclose (pConfigFile);

    // #### Lets do some code crunching !
    if ((errorNum == 0) && (errorCode == 0))
       errorCode = ProcessFile (pInputFile, pOutputFile, settings);

    if (settings.output != False)
          printf ("\nCleaning Up Dinner ... ");

    if (pInputFile != NULL)
          fclose (pInputFile);

    if (pOutputFile != NULL)
          fclose (pOutputFile);

    if (settings.output != False)
          printf ("Done !\n");

    return errorCode;
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int main (int argc, char* argv[])
{
    return LoadnRun (argc, argv);
}
// The End :-).
