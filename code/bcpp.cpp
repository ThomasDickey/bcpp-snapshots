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
// type of system (i.e Unix, Amiga), use default.
// I have tryed to use all standard Unix functions for I/O to keep
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
//  Not recoginised as a comment becuase of the quotes chars!
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

#include "bcpp.h"
#include "anyobj.h"            // Use ANYOBJECT base class
#include "baseq.h"             // QueueList class to store Output structures
#include "stacklis.h"          // StackList class to store indentStruct
#include "cmdline.h"           // ProcessCommandLine()
#include "config.h"            // SetConfig()

// ----------------------------------------------------------------------------
const int IndentWordLen  = 9;   // number of indent words in pIndentWords
const int MultiIndent    = 4;   // pos in pIndentWords where multi-line indent starts

const char pIndentWords [IndentWordLen][10] =
{
    "if", "while", "for", "else",                       // single line indentation
    "case", "default", "public", "protected", "private" // multi-line indentation
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

    DataTypes dataType;       // i.e "/*" or "*/"                  (1)
                              //     "//"                          (2)
                              //     Code (a = 5; , if (a == b) .. (3)
                              //     "{"                           (4)
                              //     "}"                           (5)
                              //     #define                       (6)
                              //     SPACES (nothing, blank line)  (7)


    Boolean attrib;
                              //       -1 : True,  comment with code (for comment dataType)
                              //        0 : False, comment with no Code

    char* pData;              // pointer to queue data !
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
#ifdef DEBUG
           int   thisToken;    // current token number
#endif
           int   indentSpace;  // num of spaces
           char* pCode;
           char* pBrace;       // "}" or "{"
           int   filler;       // num of spaces
           char* pComment;

           // Constructor
           // Automate initalisation
           inline  OutputStruct  (void)
           {
#ifdef DEBUG
                thisToken      = totalTokens++;
#endif
                pCode = pBrace = pComment = NULL;
                indentSpace    = filler   = 0;
           }

           // Destructor
           // Automate destruction
           inline ~OutputStruct (void)
           {
                delete pCode;
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
           //       2 = end on close brace, and at a position
           unsigned char attrib;
           int           pos;

           // Indent double the amount for multiline single if, while ...
           // statements.
           int           singleIndentLen;
           Boolean       firstPass; // user to determine if more than a single line
                                    // if, while... statement.

    // constructor
    inline IndentStruct (void)
    {
        attrib = pos = 0;

        singleIndentLen = 0; // number of spaces to indent !
        firstPass       = False;
    }
};

inline char lastChar(const char *s)
{
   return (*s != NULLC) ? *(s + strlen(s) - 1) : (char)NULLC;
}

inline bool isName(char c)
{
   return isalnum(c) || (c == '_') || (c == '$');
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

const char *MatchKeyword(const char *tst, int Icount)
{
   if (tst != 0)
   {
      do
      {
         if (CompareKeyword(tst, pIndentWords[Icount]))
            return tst;
         Icount++;
      } while (Icount < IndentWordLen);
   }
   return 0;
}

// ----------------------------------------------------------------------------
// Function returns a True/False value according if a word/character is
// contained within quotes within a string.
// examples: hello "world"        : world lies within quotes (returns True)
//           hello 'world'        : same as above
//           "hello" world        : world no long lies within quotes (returns false)
//
// Parameters:
// pLeftLoc      : Pointer in the string of the left most starting position to
//                 start searching.
// pRightLoc     : Pointer in the string of the right most starting position to
//                 start searching.
// pStartOfString: Pointer to the actual start location of the string that is
//                 going to be searched.
//
// Return Values :
// Boolean       : Returns True if sub-string is within quotes, else returns
//                 False.
//
Boolean CheckCharsWithinQuotes (char* pLeftLoc, char* pRightLoc, char* pStartOfString)
{
    char TQuote   = NULLC;
    
    while (  (pLeftLoc >= pStartOfString)
       &&  ((*pLeftLoc != DQUOTE) && (*pLeftLoc != SQUOTE)) )
              pLeftLoc--;

    while ( (*pRightLoc != NULLC)
       &&  ((*pRightLoc != DQUOTE) && (*pRightLoc != SQUOTE)) )
              pRightLoc++;

    if  ((*pLeftLoc == DQUOTE) && (*pRightLoc == DQUOTE))
        TQuote = DQUOTE;
    else if ((*pLeftLoc == SQUOTE) && (*pRightLoc == SQUOTE))
        TQuote = SQUOTE;

    if  (TQuote != NULLC)
    {
        // scan from left to right testing for multiple "quotes !"
        char* pTLeft  = pStartOfString;
        char* pTRight = pStartOfString;

        for (;;)
        {
            pTLeft = strchr (pTLeft, TQuote);
            if (pTLeft == NULL)
               return False;
            else if (*(pTLeft-1) == ESCAPE)   // a constant !
                 {
                    pTLeft++;
                    continue;               // continue next search !
                 }

            char* pBackup = pTLeft;
            do
            {
                pTRight = strchr (pTLeft+1, TQuote);
                if (pTRight == NULL)
                   return False;
                else if (*(pTRight-1) == ESCAPE)
                   pTLeft = pTRight;

            } while (*(pTRight-1) == ESCAPE);

            pTLeft = pBackup;

            if ((pLeftLoc >= pTLeft) && (pRightLoc <= pTRight))
               return True;

            pTLeft = pTRight+1;
        }
    }

    return False;
}


// ----------------------------------------------------------------------------
// Function is used to test if characters found on a line arn't just
// C++ comments in disguise. This is used to test C comments.
// example // /* C comment*/              : returns True
//         /* C comment*/ // C++ comment  : returns False
//
// Parameters:
// pLeftLoc       : Pointer to the start of the sub-string to start searching
//                  left for a C++ comment
// pStartOfString : Pointer to the actual start location of the whole string
//                  that is going to be searched.
//
// Return Values:
// Boolean      : Returns True if sub-string is within a C++ comment,
//                else returns False
//
Boolean CheckCharsNotCppComments (char* pLeftLoc, char* pStartOfString)
{
    char* pCppCommentLoc = strstr (pStartOfString, "//");

    if ((pCppCommentLoc != NULL) && (pCppCommentLoc < pLeftLoc))
        return True;
    else
        return False;
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
// Return Values:
// char*     : Returns a pointer to the newly reformated string.
//
char* StripSpacingLeftRight (char* pLineData, int mode = 3)
{
             int   counter   = strlen (pLineData);
    unsigned char* pLeftPos  = (unsigned char*) pLineData;
    unsigned char* pRightPos = (unsigned char*) pLineData + counter;


    // find left start of code !
    // over chars that are greater than 32 !

    if ((mode & 1) == 1)
    {
        while ((*pLeftPos <= 32) && (counter > 0))
        {
            pLeftPos++;
            counter--;
        }
    }

    // find right start of code !
    // over chars that are greater than 32 !
    if ((mode & 2) == 2)
    {
        while ((*pRightPos <= 32)  && (pRightPos > pLeftPos))
              pRightPos--;
    }

    // copy that part of the code to the start of the string
    if (counter == 0)
       pRightPos[0] = NULLC;  // create null string
    else
       pRightPos[1] = NULLC;  // create a shortened string

    pLineData = strcpy (pLineData, (char*) pLeftPos);

    return pLineData;
}


// ----------------------------------------------------------------------------
// Function takes a unsigned char and converts it to a C type string that
// contains the char's value, but in octal (i.e "\000" = null char).
//
// Parameters:
// value     : The value that wishes to be converted
//
// Return Values:
// char*     : Returns a pointer to the string that was converted.
// Memory is allocated via the new command, and once string has been used,
// memory should be returned to the system.
//
char* ConvertCharToOctal (unsigned char value)
{
    const char octalVals[] = "01234567";

    char* pOctalValue = new char[5]; // \000 digits plus null terminator

    if (pOctalValue != NULL)
    {

        for (int pos = 3; pos >= 1; pos--)
        {
            pOctalValue[pos] = octalVals[(value & 7)];
            value >>= 3; // left shift to next three bits
        }
        pOctalValue[0] = ESCAPE;
        pOctalValue[4] = NULLC;

    }

    return pOctalValue;
}


// ----------------------------------------------------------------------------
// This function will strip any non-printable characters from a string,
// any characters between quotes are converted to octal character notation,
// if quoteChar parameter set.
//
// Parameters:
// pLineData  : Pointer to the string to process
// mode       : The type of characters to strip, and move over ...
//          1 : Remove non-printing chars (i.e control chars, non-ASCII chars).
//          3 : Same as above cept leave graphic chars alone.
// quoteChars : Boolean char used to change non-ascii chars that lie within
//            : quotes to character/octal notation if set to True.
//
// Return Values:
// char*      : Returns a pointer to the newly altered string (if any chars removed).
//
char* StripNonPrintables (char* pLineData, int mode, Boolean quoteChars)
{
    unsigned char* pCheckByte = (unsigned char*) pLineData;

    while (*pCheckByte != NULLC)
    {
    Boolean removeChar = False;

        // type cast unsigned chars to ints to stop compiler wingeing.
        switch (mode)
        {
            case (1):
            {
                // remove chars below a space, but not if char is a TAB.
                // Remove chars if greater than 127 (non ascii ... IBM)
                if ( ((( (int) *pCheckByte > 0)   && ((int) *pCheckByte <= 31) ) && ((int) *pCheckByte != 9)) ||
                     (( (int) *pCheckByte >= 127) && ( (int) *pCheckByte <= 255)) )
                   removeChar = True;
                break;
            }
            case (3):
            {
                if ( ((( (int) *pCheckByte > 0)   && ((int) *pCheckByte <= 31) ) && ((int) *pCheckByte != 9)) ||
                     (( (int) *pCheckByte >= 127) && ( (int) *pCheckByte <= 175)) )
                {
                   // check for non graph chars
                   if (( (int) *pCheckByte >= 224) && ( (int) *pCheckByte <= 255))
                       removeChar = False; // graphic char
                   else
                       removeChar = True;  // a non-graphic char !
                }
                break;
            }
        } // switch

        if (removeChar != False) // remove char from string
        {
            Boolean shuffleChars = True;

            // test to see if char lies within quotes, if true then covert it to
            // octal !
            // Do test if not CPP comment !!!
            if ((CheckCharsWithinQuotes   ((char*)pCheckByte, (char*)pCheckByte, pLineData) != False) &&
                (quoteChars == True))
            {
                char* pStartQuote = strchr (pLineData, DQUOTE); // try find first type of quote in string

                if (pStartQuote == NULL)   // find other type of quote if other doesn't exist
                   pStartQuote = strchr (pLineData, SQUOTE);

                // char isn't within a cpp comment !
                if (CheckCharsNotCppComments (pStartQuote, pLineData) == False)
                {

                    char* pOctalVal  = ConvertCharToOctal (*pCheckByte);
                    char* pTempStore = new char[strlen(pLineData)+strlen(pOctalVal)+1]; // include length of octal, plus null terminator

                    // memory allocation failed
                    if ((pTempStore == NULL) || (pOctalVal == NULL))
                    {
                        delete pOctalVal;
                        delete pTempStore;
                        delete pLineData;
                        return NULL;
                    }

                    *pCheckByte     = NULLC; // terminate offending char
                    // concatinate orignal string, octal string, and remaining original string!
                    strcpy (pTempStore, pLineData);   // copy first part of string
                    strcpy (pTempStore + strlen(pTempStore), pOctalVal);
                    strcpy (pTempStore + strlen(pTempStore), (char*)pCheckByte+1);

                    delete pLineData;
                    delete pOctalVal;
                    pLineData       = pTempStore;
                    pCheckByte      = (unsigned char*) pLineData; // rescan line !

                    shuffleChars    = False;                      // bypass removing char !
                } // if chars not within a cpp comment !
            }

            // remove char from string, so long it's not within a quote !
            if ((shuffleChars != False) &&
                (CheckCharsWithinQuotes   ((char*)pCheckByte, (char*)pCheckByte, pLineData) == False))
            {

                // shuffle next char over offending non-printg char !
                unsigned char* pShuff = pCheckByte;

                while (*pShuff != NULLC)
                {
                      pShuff[0] = pShuff[1];
                      pShuff++;
                }
                pCheckByte--;   //recheck copied char over original!
            }

        } // remove/alter char from/in string !

        pCheckByte++;
    }// while

    return pLineData;
}


// ----------------------------------------------------------------------------
// Function returns a Boolean value that shows where code is contained within
// a string. Any chars within a string above space are consided code.
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
InputStruct* ExtractCode (char*    pLineData, DataTypes dataType = Code,
                          Boolean  removeSpace = True)
{
    char* pNewCode =  new char[strlen (pLineData)+1];

    // ############## test memory #############
    if (pNewCode == NULL)
        return NULL;

    pNewCode = strcpy (pNewCode, pLineData);
    
    // create new queue structure !
    InputStruct* pItem = new InputStruct();
    if (pItem != NULL)
    {
            // strip spacing in new string before storing
            if (removeSpace != False)
                pItem -> pData    = StripSpacingLeftRight (pNewCode);
            else
                pItem -> pData = pNewCode;
            pItem -> dataType = dataType;
            pItem -> attrib   = False;     // no applicable
    }
    else
        delete pNewCode;

    return pItem;

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
// pLineData : Pointer to a string will contain a line of a users input file.
//
void DecodeLineCleanUp (QueueList* pDelQueue, char* pLineData)
{
    // Don't implement destructor as other objects may be using the same
    // memory when using structure in output line processing (simple garbage collection)
    while (pDelQueue->status() > 0)
        CleanInputStruct ( ((InputStruct*)pDelQueue -> takeNext()) );

    delete pLineData;
}

static char *FindPunctuation(char *pLineData, char punct)
{
    char *pSChar = pLineData-1;

    for (;;)
    {
        pSChar = strchr (pSChar+1, punct);

        if (pSChar != NULL)
        {
           if (CheckCharsWithinQuotes (pSChar, pSChar, pLineData) == False)
               break;
           // else continue and try and find next one after comment !
        }
        else
            break;
    }
    return pSChar;
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
// CComments  : This variable is used to test if the current line is
//              a C type comment (i.e multi-line comment). True value
//              indicated Currently in C comment extraction mode, else
//              in normal extraction mode.
// QueueList* : Pointer to a QueueList object will contains all of
//              a lines basic elements. If this object doesn't contain
//              any elements, then it sugguests there was a processing
//              problem.
//
// Return Values:
// int        : returns a error code.
//              -1 : Memory allocation failure
//               0 : No Worries
//
int DecodeLine (char* pLineData, Boolean& CComments, QueueList* pInputQueue)
{
    char*      pSChar = NULL; // used to find the starting location of certain elements in a string
    char*      pEChar = NULL; // used as a terminator of an element in a string !


    // remove unwanted space chars ????
    // pLineData = StripSpacingLeftRight (pLineData);

    // @@@@@@ C Comment processing, if over multiple lines @@@@@@
    if (CComments == True)
    {

        //#### Test to see if end terminating C comment has arrived !
        pSChar = strstr (pLineData, "*/");

        if (pSChar != NULL)
        {
            pSChar[1] = NULLC;   //#### make temp string !
            char* pNewComment =  new char[strlen (pLineData) + 2];

            //#### Test if memory allocated
            if (pNewComment == NULL)
            {
                DecodeLineCleanUp (pInputQueue, pLineData);
                return -1;
            }

            pNewComment        = strcpy (pNewComment, pLineData);
            int len            = strlen (pNewComment);
            pNewComment[len]   = '/';
            pNewComment[len+1] = NULLC;

            pSChar += 2;                //##### Advance two mem locations to the right
            strcpy (pLineData, pSChar); //##### Shift left string from current pos

            //#### create new queue structure !
            InputStruct* pItem = new InputStruct();

            //#### Test if memory allocated
            if (pItem != NULL)
            {
                pItem -> pData    = pNewComment;
                pItem -> dataType = CCom;      // comment
                pItem -> attrib   = False;     // comment without code, even if it has some !

                pInputQueue->putLast (pItem);
            }
            else
            {
                DecodeLineCleanUp (pInputQueue, pLineData);
                return -1;
            }

            CComments = False;                 //##### No more C like comments

        }
        else //##### Place output as comment without code (C comment terminator not found)
        {
            InputStruct* pTemp = ExtractCode (pLineData, CCom, False); // don't remove spaces !

            //#### Test if memory allocated
            if (pTemp != NULL)
            {
                pInputQueue->putLast (pTemp);
                delete pLineData;
            }
            else
            {
                DecodeLineCleanUp (pInputQueue, pLineData);
                return -1;
            }

            return 0;
        }


    }// if multi-line C style comments

    
    // N.B Place this function here as to sure not to corrupt relative pointer
    // settings that may be used within pLinedata, and become altered through 
    // using this routine.     
    pLineData = StripSpacingLeftRight (pLineData); 
    
    //@@@@@@ Extract /* comment */ C type comments on one line
    pSChar = strstr (pLineData, "/*");  // find start of C Comment
    if (pSChar != NULL)
    {
        //##### Check if "/*" isn't within quotes
        if ((CheckCharsWithinQuotes (pSChar, pSChar+1, pLineData) == False) &&
            (CheckCharsNotCppComments (pSChar, pLineData) == False))
        {
            //##### Check if there is a ending "*/" C terminator comment string
            pEChar = strstr (pLineData, "*/");

            //##### If NULL then comments are on multiple lines !
            if (pEChar == NULL)
            {
                char* pNewComment = new char[strlen (pSChar)+1];

                //#### Test if memory allocated
                if (pNewComment == NULL)
                {
                    DecodeLineCleanUp (pInputQueue, pLineData);
                    return -1;
                }

                pNewComment = strcpy (pNewComment, pSChar);
                *pSChar  = NULLC; //##### Make it NULL so that comment is removed, from Line

                InputStruct* pItem = new InputStruct();

                //#### Test if memory allocated
                if (pItem != NULL)
                {
                    pItem -> pData    = pNewComment;
                    pItem -> dataType = CCom;       // Comment
                    pItem -> attrib   = False;
                    // make multi-line C style comments totally seperate
                    // from code to avoid some likely errors occuring if they
                    // are shifted due to being over written by code.

                    // apply recursion so that comment is last item placed
                    // in queue !
                    if (DecodeLine (pLineData, CComments, pInputQueue) != 0)
                    {
                        // problems !
                        delete pItem -> pData;
                        delete pItem;
                        return -1;
                    }
                    else
                        pInputQueue->putLast (pItem);

                    //##### Set multi-line comment variable on
                    CComments = True;

                    return 0; // no need to continue processing
                }
                else
                {
                    DecodeLineCleanUp (pInputQueue, pLineData);
                    return -1;
                }
            }
            else
            {
                pEChar[1] = NULLC; //##### make temp string !
                char* pNewComment =  new char[strlen (pSChar) + 2];

                //#### Test if memory allocated
                if (pNewComment == NULL)
                {
                    DecodeLineCleanUp (pInputQueue, pLineData);
                    return -1;
                }

                pNewComment        = strcpy (pNewComment, pSChar);
                int len            = strlen (pNewComment);
                pNewComment[len]   = '/';
                pNewComment[len+1] = NULLC;

                pEChar += 2;      //##### Advance two memory locs to the right
                strcpy (pSChar, pEChar);

                InputStruct* pItem = new InputStruct();
                //#### Test if memory allocated
                if (pItem != NULL)
                {
                    pItem -> pData    = pNewComment;
                    pItem -> dataType = CCom;                        // Comment
                    pItem -> attrib   = TestLineHasCode (pLineData); // Comment without code ?
                    TRACE((stderr, "@%d: set attrib to %d\n", __LINE__, pItem->attrib))

                    pInputQueue->putLast (pItem);

                }
                else
                {
                    DecodeLineCleanUp (pInputQueue, pLineData);
                    return -1;
                }

            } //##### else

        } //##### If comment not within quotes

    }//##### If "/*" C comments pressent
    
    //##### Remove blank spacing from left & right of string
    pLineData = StripSpacingLeftRight (pLineData);

    //@@@@@@ C++ Comment Processing !
    pSChar = strstr (pLineData, "//");
    if (pSChar != NULL)
    {
        //##### Check if "//" arn't within quotes, if false then it's a comment
        if (CheckCharsWithinQuotes (pSChar, pSChar+1, pLineData) == False)
        {
            char* pNewComment =  new char[strlen (pSChar) + 1];

            //#### Test if memory allocated
            if (pNewComment == NULL)
            {
                DecodeLineCleanUp (pInputQueue, pLineData);
                return -1;
            }

            // copy comment into allocated memory
            strcpy (pNewComment, pSChar);
            *pSChar  = NULLC;                   //##### Terminate original string !

            //#### create new queue structure !
            InputStruct* pItem = new InputStruct();
            //#### Test if memory allocated
            if (pItem != NULL)
            {
                pItem -> pData    = pNewComment;
                pItem -> dataType = CppCom;                      // Comment
                pItem -> attrib   = TestLineHasCode (pLineData); // Comment without code ?
                TRACE((stderr, "@%d: set attrib to %d\n", __LINE__, pItem->attrib))

                pInputQueue->putLast (pItem);

            }
            else
            {
                DecodeLineCleanUp (pInputQueue, pLineData);
                return -1;
            }
        }
    }

    //@@@@@@ #define (preprocessor extraction)
    if (pLineData[0] == '#')
    {
        //#### create new queue structure !
        InputStruct* pItem = new InputStruct();

        //#### Test if memory allocated
        if (pItem != NULL)
        {
            pItem -> pData    = pLineData;
            pItem -> dataType = PreP;                            // preprocessor
            pInputQueue->putLast (pItem);

        }
        else
        {
            DecodeLineCleanUp (pInputQueue, pLineData);
            return -1;
        }

        return 0; // no worries !
    }

    //################# Actual Code Extraction #################

    pLineData = StripSpacingLeftRight (pLineData);
    
    //@@@@@@ Test what's left in line for L_CURL, and R_CURL braces

    pSChar = FindPunctuation(pLineData, L_CURL);
    pEChar = FindPunctuation(pLineData, R_CURL);

    //##### check if braces aren't within quotes, if so .. then ignore!

    Boolean testEnumType = False;
    if ( ((pSChar != NULL) && (pEChar != NULL)) && (pSChar < pEChar))
    {
        // test to see if there are multiple open/ close braces in enum
        // selective range
        // i.e. { if ( a == b ) { b = c } else { d = e } }
        char* pOBrace = FindPunctuation(pSChar+1, L_CURL);

        if ( (pOBrace == NULL) || ((pOBrace > pEChar) && (pOBrace != NULL)) )
           testEnumType = True;
    }

    //##### If condition correct, then make rest of line just code ! (e.g enum)
    // if no items in input queue, and no multiple open, close braces in
    // line then .... extract as enum.
    if ( (testEnumType != False) && (pInputQueue -> status () <= 0) )
    {
        //store code as enum type if follow if condition is true
        pEChar++;

        if (*pEChar == SEMICOLON) // advance another char
                pEChar++;
            
        char BackUp = *pEChar;
        *pEChar     = NULLC;

        InputStruct* pTemp = ExtractCode(pLineData);
        if (pTemp == NULL)
        {
            DecodeLineCleanUp (pInputQueue, pLineData);
            return -1;
        }

        pInputQueue->putLast (pTemp);

        *pEChar   = BackUp;
        strcpy (pLineData, pEChar);

        // restart decoding line !
        return DecodeLine (pLineData, CComments, pInputQueue);
        // end of recursive call !

    } // if L_CURL and R_CURL exit on same line

    //##### Determine extraction precedence !
    if ((pSChar != NULL) && (pEChar != NULL))
    {
        if (pSChar > pEChar)
            pSChar = NULL;
        else
            pEChar = NULL;
    }

    //##### Place whatever is before the open brace L_CURL, or R_CURL as code
    if ((pSChar != NULL) || (pEChar != NULL))
    {
        char backUp;

        if (pSChar != NULL)
        {
            backUp         = *pSChar;
            *pSChar        = NULLC;
        }
        else
        {
            backUp = *pEChar;
            *pEChar        = NULLC;
        }

        //#### Store leading code if any
        if (TestLineHasCode (pLineData) != False)
        {
           char* pTemp = new char [strlen (pLineData) +1];
           if (pTemp == NULL)
           {
                DecodeLineCleanUp (pInputQueue, pLineData);
                return -1;
           }

           //#### strip spacing is handled within extractCode routine. This
           //#### means that pointers that are calculated before stripSpacing function
           //#### remain valid.

           pTemp = strcpy (pTemp, pLineData);
           InputStruct* pLeadCode = ExtractCode (pTemp);

           if (pLeadCode == NULL)
           {
                DecodeLineCleanUp (pInputQueue, pLineData);
                return -1;
           }

           pInputQueue->putLast (pLeadCode);
           delete pTemp;
        }

        //##### Update main string
        if (pSChar != NULL)
        {
           *pSChar        = backUp;
           pLineData      = strcpy (pLineData, pSChar);
        }
        else
        {
           *pEChar        = backUp;
           pLineData      = strcpy (pLineData, pEChar);
        }

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
                    pTemp        = ExtractCode (pLineData, OBrace);//##### Define data type before storing
                    //#### update string
                    pLineData[1] = backUp;

                    strcpy (pLineData, pLineData+1);
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
                    pSChar = FindPunctuation(pLineData+1, L_CURL);
                    pEChar = FindPunctuation(pLineData+1, R_CURL);

                    if ((pSChar != NULL) || (pEChar != NULL))
                    {
                        if (pLineData[1] == SEMICOLON)     // if true, extract after char
                            pEChar        = pLineData+2;
                        else
                            pEChar        = pLineData+1;

                        backUp            = *pEChar;
                        *pEChar           = NULLC;

                        pTemp = ExtractCode (pLineData, CBrace);
                        // #### update string;
                        *pEChar           = backUp;
                        strcpy (pLineData, pEChar);

                        extractMode       = 3;       // apply recursive extraction
                    }
                    else // rest of data is considered as code !
                    {
                        pTemp     = ExtractCode (pLineData, CBrace);
                        pLineData = NULL;            // leave processing !
                    }
                    break;
                }

                case (3):   // remove what is left on line as code.
                {
                    return DecodeLine (pLineData, CComments, pInputQueue);
                    // end of recursive call !
                }
            }// switch;

            //#### Test if memory allocated
            if (pTemp == NULL)
            {
                DecodeLineCleanUp (pInputQueue, pLineData);
                return -1;
            }

            pInputQueue->putLast (pTemp); // store Item

        } while ((TestLineHasCode (pLineData) != False) && (pTemp != NULL));

    }

    else //##### Line contains either code, or spacing
    {
        //##### If nothing in string, and nothing stored in queue, then blank line
        if ((pLineData[0] == NULLC) && ((pInputQueue->status()) <= 0))
        {
            //##### implement blank space
            InputStruct* pTemp = ExtractCode (pLineData, ELine);

            if (pTemp != NULL)
                pInputQueue->putLast (pTemp);
            else
            {
                DecodeLineCleanUp (pInputQueue, pLineData);
                return -1;
            }
        }
        //##### If line has more than spacing/tabs then code
        else if (TestLineHasCode (pLineData) != False)
             {
                  // implement blank space
                  InputStruct* pTemp = ExtractCode (pLineData);

                  if (pTemp != NULL)
                     pInputQueue->putLast (pTemp);
                  else
                  {
                      DecodeLineCleanUp (pInputQueue, pLineData);
                      return -1;
                  }
             }
    }
    
    //##### Final cleanup before returning queue with line elements
    delete pLineData;
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

        // If the comment text begins with an '*', increase the indention by
        // one unless it follows a comment-line that didn't begin with '*'.
        if (*text == '*'
         && (n = pLines -> status()) > 0)
        {
            OutputStruct *pTemp = (OutputStruct*) pLines -> peek (n);
            if (pTemp != 0
             && pTemp -> pComment != 0)
                text = pTemp -> pComment;
            if (*text == '*'
             || !strncmp(text, "/*", 2))
                pOut -> indentSpace += 1;
        }
    }
}

// ----------------------------------------------------------------------------
// Function takes a QueueList object that contains InputStructure items, and
// uses these items to reconstuct a compressed version of a output line of
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
int ConstructLine (int& indentStack, QueueList* pInputQueue, QueueList* pOutputQueue, Config userS)
{
    InputStruct* pTestType = NULL;

    while ( pInputQueue->status() > 0 )
    {
        pTestType = (InputStruct*) pInputQueue -> takeNext();

        switch (pTestType -> dataType)
        {
            //@@@@@@@ Processing of C type comments /* comment */
            case (CCom):
            //@@@@@@@ Processing of C++ type comments // comment
            case (CppCom):
            {
                if (pTestType -> attrib == True)  //##### If true then comment has code
                {
                    OutputStruct* pOut = new OutputStruct;

                    // ##### Memory allocation test
                    if (pOut != NULL)
                    {
                        // ##### store current comment in output structure
                        pOut -> pComment      = pTestType -> pData;
                        delete pTestType;     //#### free memory

                        // ##### getnext Item
                        pTestType = (InputStruct*) pInputQueue -> takeNext();

                        // ##### Type Checking !!!!!
                        // if nothing in queue, or next item isn't code then some sort of error
                        if ((pTestType == NULL) || ((pTestType -> dataType <= CppCom) || (pTestType -> dataType >= ELine)))
                        {
                            fprintf (stderr, "\n#### ERROR ! Error In Line Construction !");
                            fprintf (stderr, "\nExpected Some Sort Of Code ! Data Type Found = ");
                            if (pTestType == NULL)
                                fprintf (stderr, "NULL");
                            else
                                fprintf (stderr, "%d", pTestType -> dataType);
                            return -2;      // ##### incorrect dataType expected!
                        }
                        else // place code in line structure, calculate indenting !
                        {

                            // if pData length overwrites comments then place comments on newline
                            if ( (indentStack + strlen (pTestType -> pData)) > (userS.posOfCommentsWC) )
                            {
                                pOut -> filler = userS.posOfCommentsWC;
                                pOutputQueue -> putLast (pOut);

                                // create new output line structure
                                pOut = new OutputStruct();
                                if (pOut == NULL)
                                   return -1; //##### Memory Allocation error
                            }

                            switch (pTestType -> dataType)
                            {
                                case (Code):
                                case (PreP):
                                {
                                    pOut -> indentSpace = indentStack;
                                    pOut -> pCode  = pTestType -> pData;

                                    // if comment wasn't placed on a newline due to code overwriting comment
                                    if (pOut -> pComment != NULL)
                                        pOut -> filler = (userS.posOfCommentsWC - (indentStack + strlen (pTestType -> pData)));

                                    break;
                                }

                                case (OBrace):
                                case (CBrace):
                                {

                                    // indent back before adding brace, some error checking
                                    if ((pTestType -> dataType == CBrace) && (indentStack > 0))
                                    {
                                        indentStack -= userS.tabSpaceSize;

                                        // if comment wasn't placed on a newline due to code overwriting comment
                                        if (pOut -> pComment != NULL)
                                               pOut -> filler = (userS.posOfCommentsWC - (indentStack + strlen (pTestType -> pData)));
                                    }

                                    // load input data into the approiate place in output structure
                                    pOut -> indentSpace = indentStack;
                                    pOut -> pBrace = pTestType -> pData;
                                    TRACE((stderr, "Set pBrace = %s:%d at %d\n", pOut->pBrace, pOut->thisToken, __LINE__))

                                    // adjust indentation if next output line is a open Brace
                                    if (pTestType -> dataType == OBrace)
                                    {
                                          // if comment wasn't placed on a newline due to code overwriting comment
                                          if (pOut -> pComment != NULL)
                                                     pOut -> filler = (userS.posOfCommentsWC - (indentStack + strlen (pTestType -> pData)));

                                          indentStack += userS.tabSpaceSize;
                                    }

                                    break;
                                }

                                case (ELine):
                                case (CppCom):
                                case (CCom):
                                    // not affected by indentStack
                                    break;

                            }// switch


                            pOutputQueue -> putLast (pOut);
                        }// else

                    }
                    else
                        return -1; //##### Memory allocation error
                }
                else
                {
                    OutputStruct* pOut = new OutputStruct();

                    // ##### Memory allocation test
                    if (pOut != NULL)
                    {
                        // JZAS Start
                        if (userS.leaveCommentsNC != False)
                            pOut -> indentSpace   = indentStack;
                        else
                              pOut -> indentSpace   = userS.posOfCommentsNC;
                        // JZAS End
                        
                        pOut -> pComment          = pTestType -> pData;
                        dontHangComment(pTestType, pOut, pOutputQueue);

                        TRACE((stderr, "Set Comment = %s:%d indent %d at %d\n", pOut->pComment, pOut->thisToken, pOut->indentSpace, __LINE__))
                        pOutputQueue -> putLast (pOut);
                    }
                    else
                        return -1; //##### Memory allocation error

                }// else a comment without code !

                break;
            }// case

            // @@@@@@ Processing of code (i.e k = 1; enum show {one, two};)
            case (Code):
            {
                OutputStruct* pOut = new OutputStruct();
                if (pOut != NULL)
                {
                    pOut -> indentSpace     = indentStack;
                    pOut -> pCode           = pTestType -> pData;
                    TRACE((stderr, "Set Code   = %s:%d indent %d at %d\n", pOut->pCode, pOut->thisToken, pOut->indentSpace, __LINE__))
                    pOutputQueue -> putLast (pOut);
                }
                else
                    return -1; //##### Memory allocation error

                break;
            }

            // @@@@@@ Processing of open brackets "{ k = 1;"
            case (OBrace):
            // @@@@@@ Processing of closed brackets "} k = 1;"
            case (CBrace):
            {
                OutputStruct* pOut = new OutputStruct();
                if (pOut != NULL)
                {
                    // indent back before adding brace, some error checking
                    if ((pTestType -> dataType == CBrace) && (indentStack > 0))
                        indentStack -= userS.tabSpaceSize;

                    pOut -> indentSpace     = indentStack;
                    pOut -> pBrace          = pTestType -> pData;
                    TRACE((stderr, "Set pBrace = %s:%d indent %d at %d\n", pOut->pBrace, pOut->thisToken, pOut->indentSpace, __LINE__))
                    pOutputQueue -> putLast (pOut);

                    // ##### advance to the right !
                    if (pTestType -> dataType == OBrace)
                        indentStack += userS.tabSpaceSize;
                }
                else
                    return -1; //##### Memory allocation error

                break;
            }

            // @@@@@@ Blank Line spacing
            case (ELine):
            {
                OutputStruct* pOut = new OutputStruct();
                if (pOut != NULL)
                    pOutputQueue -> putLast (pOut);      //#### Place NULL line in output queue
                else
                    return 0; //##### Memory allocation error

                break;
            }
            // @@@@@@ Preprocessor Line !
            case (PreP):
            {
                OutputStruct* pOut = new OutputStruct();
                if (pOut != NULL)
                {
                    pOut -> pCode       = pTestType -> pData;
                    pOut -> indentSpace = 0;
                    pOutputQueue -> putLast (pOut);      //#### Place NULL line in output queue
                }
                else
                    return 0; //##### Memory allocation error

                break;
            }

        } // switch

        delete pTestType; // ##### Remove structure from memory, not it's data
                          // ##### it contained (i.e char* pData), this is stored
                          // ##### in the output queue.

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
        pIndentItem->attrib = 0;
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
         || pIndentItem -> attrib != 1
         || pIndentItem -> singleIndentLen == 0)
            return False;
    }
    return True;
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
                // A preprocessor command
                if (pAlterLine -> pCode[0] == '#')
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
    // A preprocessor command
    if ((pAlterLine -> pCode != NULL) && (pAlterLine -> pCode[0] == '#'))
       return pLines;

    IndentStruct* pIndentItem = (IndentStruct*) pIMode -> pop();

    if ( ((pAlterLine -> pCode != NULL)     || ((pAlterLine -> pBrace != NULL) && (pIndentItem -> attrib == 2)) ) ||
         ((userS.leaveCommentsNC != False)  && ((pAlterLine -> pCode == NULL)  && (pAlterLine -> pComment != NULL))) )
    {
        TRACE((stderr, "#%d, attrib=%d\n", pAlterLine->thisToken, pIndentItem->attrib))
        switch (pIndentItem -> attrib)
        {
            // single indent
            case (1):
            {
                int indentAmount;

                // Test if current code has a ';' char at end of line,
                // if so then do full indent, else do 3 char indent!
                if ((pAlterLine -> pCode != NULL) &&
                    lastChar(pAlterLine -> pCode) != SEMICOLON)
                {
                    pIndentItem -> firstPass    = True;
                    indentAmount = pIndentItem -> singleIndentLen;
                    TRACE((stderr, "#%d, use single-indent %d\n", pAlterLine->thisToken, pIndentItem->singleIndentLen))
                }
                else if (pIndentItem -> firstPass != False)
                         indentAmount = (userS.tabSpaceSize << 1);
                     else
                         indentAmount = userS.tabSpaceSize;

                // Single line indentation calculation
                pAlterLine -> indentSpace += indentAmount;
                TRACE((stderr, "total indent %d (%d)\n", pAlterLine->indentSpace, indentAmount))
                break;
            }

            // indent of a case statement !
            case (2):
            {
                // determine how many case like items are stored within
                // list to determine how much to indent !
                const char* pTest = NULL;

                pAlterLine -> indentSpace += (userS.tabSpaceSize * (pIMode -> status()));

                // test if not another case, or default, if so, dont indent !
                pTest = MatchKeyword(pAlterLine -> pCode, MultiIndent);

                // check for closing braces to end case indention
                if ((pTest == NULL) && (pAlterLine -> pBrace != NULL))
                {
                    if ((*(pAlterLine -> pBrace) == R_CURL) && (pAlterLine -> indentSpace == pIndentItem -> pos))
                    {
                        delete pIndentItem;
                        pIndentItem = NULL;
                    }
                }

                // indent as per normal !
                if ((pIndentItem != NULL) && (pTest == NULL))
                {
                    pIMode -> push (pIndentItem); // ok to indent next item !
                    pAlterLine -> indentSpace += userS.tabSpaceSize;
                    TRACE((stderr, "@%d, indent %d\n", __LINE__, pAlterLine->indentSpace))
                }
                else if (pIndentItem != NULL)
                {
                    // if end single indent keyword found, check to see
                    // whether it is the correct one before removing it !
                    if ((pTest != NULL) && (pIndentItem -> pos+userS.tabSpaceSize < pAlterLine -> indentSpace))
                    {
                        pIMode -> push (pIndentItem); // ok to indent next item !
                        pAlterLine -> indentSpace += userS.tabSpaceSize;
                        TRACE((stderr, "@%d, indent %d\n", __LINE__, pAlterLine -> indentSpace))
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
                OutputStruct* pNewItem  = new OutputStruct();
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
                pAlterLine -> pComment    = NULL;

                // reconstruct queue !
                pNewQueue -> putLast (pNewItem);
                pNewQueue -> putLast (pAlterLine);

                // copy existing lines from old queue, into the newly created queue !
                while (pLines -> status () > 0)
                      pNewQueue -> putLast ( pLines -> takeNext() );

                delete pLines;
                pLines = pNewQueue; // reassign new queue object

            } // if over writting comments
        }// if comments exist on same line as code

        // Remove single line indentation from memory, if current line
        // does contain a if, else, while ... type keyowrd
        if ((pIndentItem != NULL) && (pIndentItem -> attrib == 1))
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
             && chainedSingleIndent(pIMode)
             && (block = peekIndexOBrace(pLines, 2)) != 0)
            {
                shiftToMatchSingleIndent(pLines, pAlterLine->indentSpace, block);
            }
        }

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
              ((pBraceOnNewLn[0] == L_CURL) && (pTestBrace -> attrib == 1)) )
         {
            delete pTestBrace;
         }
         else if ( (pBraceOnCurLn != NULL) &&
                   ((pBraceOnCurLn[strlen(pBraceOnCurLn)-1] == L_CURL) && (pTestBrace -> attrib == 1)) )
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
    if ( ((OutputStruct*) pLines -> peek (1)) -> pCode     != NULL)
    {
        // test the existance of a key word !
        int     findWord = 0;
        char*   pTestCode = ((OutputStruct*) pLines -> peek (1)) -> pCode;

        if (pTestCode != 0)
        {
           while (findWord < IndentWordLen)
           {
               if (CompareKeyword(pTestCode, pIndentWords[findWord]))
                  break;
               findWord++;
           }
        }

        // if keyword found, check if next line not a brace or, comment !

        // Test if code not NULL, and No Hidden Open Braces
        // FIXME: punctuation need not be at end of line
        // FIXME: pTestCode isn't reset if the previous scan failed
        if ( (pTestCode != NULL) && 
              (((pTestCode [strlen (pTestCode) - 1] == L_CURL)   ||
                (pTestCode [strlen (pTestCode) - 1] == SEMICOLON))  ||
                (pTestCode [strlen (pTestCode) - 1] == R_CURL)) )
                           pTestCode = NULL;

        // Test if open brace not located on next line
        if ((pTestCode != NULL) && (findWord < IndentWordLen))
        {
                pTestCode = ((OutputStruct*) pLines -> peek (minLimit)) -> pBrace;

                if ((pTestCode != NULL) && (pTestCode[0] == L_CURL))
                    pTestCode = NULL;    // Dont process line as a single indentation !
                else
                    pTestCode = "E";     // make sure pointer is not null !
        }

        if ((findWord < IndentWordLen) && (pTestCode != NULL))
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
            if ((findWord >= 0) && (findWord < MultiIndent))
            {
                pTestCode = ((OutputStruct*) pLines -> peek (1)) -> pCode;

                pIndent -> attrib = 1; // single indent !
                
                // determine how much to indent the next line of code !
                TRACE((stderr, "#%d: change single-indent from %d\n", ((OutputStruct *)pLines->peek(1))->thisToken, pIndent->singleIndentLen))
                pIndent -> singleIndentLen = strlen (pIndentWords[findWord]);

                // test if there is a space after statement.
                pTestCode += pIndent -> singleIndentLen;
                while (*pTestCode == SPACE)
                {
                    pTestCode++;
                    pIndent -> singleIndentLen++;
                }                                
                TRACE((stderr, "... to %d\n", pIndent->singleIndentLen))
            }
            else // it's a case statement !
            {
                pIndent -> attrib = 2; // multiple indent !
                pIndent -> pos    = (((OutputStruct*) pLines -> peek (1)) -> indentSpace) - userS.tabSpaceSize;
                TRACE((stderr, "#%d: set multi-indent\n", ((OutputStruct *)pLines->peek(1))->thisToken))
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

            for (;;)
            {
                if (pIMode -> status () > 0)
                    pThrowOut = (IndentStruct*) pIMode -> pop();
                else
                    break;            // leave loop

                if (pThrowOut -> attrib >= 2)
                {
                    pIMode -> push (pThrowOut);
                    break;
                }    // Test single code indents for a semicolon !
                else if (pTestCode [strlen(pTestCode)-1] == SEMICOLON)
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
                pNewItem                = new OutputStruct();
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
        pNewItem = new OutputStruct();
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
        strcpy (pNewMem + strlen (pNewMem), " ");                 // copy space
        strcpy (pNewMem + strlen (pNewMem), pBraceLine -> pBrace);// copy brace

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
            pNewItem = new OutputStruct();

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
        // delete pBraceLine; // dont implement this, use queue object to delete it

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
QueueList* FunctionSpacing (FILE* pOutFile, QueueList* pLines, const Config& userS, int& FuncVar )
{
    if (pLines -> status () > 0) // if there are items in the queue !
    {
        OutputStruct* pTestLine      =  (OutputStruct*) pLines -> peek (1);
        
        // check is end of function, structure, class has been reached !
        if ( ((FuncVar == 0) && (pTestLine -> indentSpace <= 0 )) &&
             (pTestLine -> pBrace != NULL) )
        {
             if (pTestLine -> pBrace[0] == R_CURL)
             {
                FuncVar = 1; // add function spacing !
                return pLines;
             }
        }

        if (FuncVar == 1)
        {
            // go into blank line output mode between functions!
            for (int outBlank = 0; outBlank < userS.numOfLineFunc; outBlank++)
                fputc (LF, pOutFile); // output line feed!
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

    return pLines;
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
QueueList* OutputToOutFile (FILE* pOutFile, QueueList* pLines, StackList* pIMode, int& FuncVar, const Config& userS, int stopLimit)
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

        pLines = FunctionSpacing (pOutFile, pLines, userS, FuncVar );

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
        TRACE((stderr, "%3d|%2d Indent  %2d Fill >%s\n",
            pOut->thisToken,
            pOut->indentSpace, pOut->filler,
            pOut->pCode
                ? pOut->pCode
                : (pOut->pBrace
                    ? pOut->pBrace
                    : (pOut->pComment
                        ? pOut->pComment
                        : "?"))))

        // expand pOut structure to print to the output file
        pIndentation = TabSpacing (fillMode, pOut -> indentSpace, userS.tabSpaceSize );
        pFiller      = TabSpacing (2, pOut -> filler, userS.tabSpaceSize);

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

        if (pOut -> pComment != NULL)
                fprintf (pOutFile, "%s", pOut -> pComment);

        fputc (LF, pOutFile); // output line feed!

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

    Boolean             CComments    = False;  // var used to test existance of multiline C Comments
    int                 indentStack  = 0;      // var used for brace spacing

    QueueList*          pOutputQueue = new QueueList();
    StackList*          pIMode       = new StackList();
    QueueList*          pInputQueue  = new QueueList();

    int                 FuncVar      = 0;      // variable used in processing function spacing !
    CharState           leftState    = Normal;
    CharState           rightState   = Normal;

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
        pData = ReadLine (pInFile, EndOfFile);

        if (pData != NULL)
        {
            lineNo++;
            if ( (lineNo % lineStep == 0) && (userS.output != False) )
            {
                    if (lineNo > 0)
                        backSpaceIt (lineNo - lineStep); // reposition cursor ! Don't used gotoxy() for Unix compatibility

                    printf ("%ld ", lineNo);
            }

            if (userS.deleteHighChars != 0)
                    pData = StripNonPrintables (pData, userS.deleteHighChars, userS.quoteChars);
                    
               pData = ExpandTabs (pData, userS.tabSpaceSize, leftState, rightState);
               if (pData == NULL)
               {
                    printf ("%s", errorMsg);
                    delete pIMode;
                    delete pInputQueue;
                    delete pOutputQueue;
                    return -1;
               }
               
            if (DecodeLine (pData, CComments, pInputQueue) == 0) // if there are input items to process
            {
                    int errorCode = ConstructLine (indentStack, pInputQueue, pOutputQueue, userS);

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
                                                    userS   , userS.queueBuffer );
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
                                    userS,    0);

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
    pLook = pInFile + (strlen (pInFile)-1);
    while ( ((*pLook != '.')  && (pLook > pInFile)) &&
            ((*pLook != '\\') && (*pLook != '/')) )
                     pLook--;

    if  ((pLook <= pInFile) || ((*pLook == '\\') || (*pLook == '/')))
        strcpy (pInFile + (strlen (pInFile) - 1), pBackUp);
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
        pEPath   = pSPath + strlen (pSPath);
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
             if (pNameMem[strlen(pNameMem)-1] != pathSepChar[0])
                 strcpy (pNameMem+strlen (pNameMem), pathSepChar);

          strcpy (pNameMem+strlen (pNameMem), pCfgName);
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
