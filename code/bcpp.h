#ifndef _BCPP_HEADER
#define _BCPP_HEADER

#include "anyobj.h"            // Use ANYOBJECT base class
#include "baseq.h"             // QueueList class to store Output structures
#include "stacklis.h"          // StackList class to store indentStruct

// Common/miscellaneous definitions for BCPP

#define TABLESIZE(n) (sizeof(n)/sizeof(n[0]))

#ifdef DEBUG
#define TRACE(p) fprintf p;
#else
#define TRACE(p) /*nothing*/
#endif

//-----------------------------------------------------------------------------
// Character-state, chosen to allow storing as a null-terminated char-string
enum CharState {
    NullC   = 0,
    Blank   = ' ',      // whitespace not in other categories
    PreProc = '#',      // preprocessor (first chunk, with '#')
    Normal  = '.',      // code
    DQuoted = '"',      // "string"
    SQuoted = '\'',     // 'c'
    Comment = 'c',      // C comment
    Ignore  = 'C'       // C++ comment
    };

// ----------------------------------------------------------------------------
enum  DataTypes { CCom = 1,   CppCom = 2, Code  = 3,
                  OBrace = 4, CBrace = 5, PreP = 6, ELine = 7
                };

// ----------------------------------------------------------------------------
enum indentAttr { noIndent=0, oneLine=1, multiLine=2, blockLine=3 };

// ----------------------------------------------------------------------------
typedef struct {
    const char *name;
    indentAttr code;
} IndentwordStruct;

extern const IndentwordStruct pIndentWords[];

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

#if defined(DEBUG) || defined(DEBUG2)
extern int   totalTokens;            // token count, for debugging
#endif

// ----------------------------------------------------------------------------
// The output structure is used to hold an entire output line. The structure is
// expanded with its real tabs/spaces within the output function of the program.
class OutputStruct : public ANYOBJECT
{
    private:
    public:
           DataTypes pType;
#if defined(DEBUG) || defined(DEBUG2)
           int   thisToken;     // current token number
#endif
           int   indentSpace;   // num of spaces
           int   indentHangs;   // num of indents for continuation
           char* pCode;
           char* pCFlag;        // state-flags for pCode
           char* pBrace;        // "}" or "{", with possible code-fragment
           char* pBFlag;        // state-flags for pBrace
           int   filler;        // num of spaces
           char* pComment;

           // Constructor
           // Automate initalisation
           inline  OutputStruct  (DataTypes theType)
           {
                pType          = theType;
#if defined(DEBUG) || defined(DEBUG2)
                thisToken      = totalTokens++;
#endif
                pCode = pCFlag = pBrace = pBFlag = pComment = NULL;
                indentSpace = indentHangs = filler = 0;
           }

           // Destructor
           // Automate destruction
           inline ~OutputStruct (void)
           {
                delete pCode;
                delete pCFlag;
                delete pBrace;
                delete pBFlag;
                delete pComment;
           }
};

// ----------------------------------------------------------------------------
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

    // constructor
    inline IndentStruct (void)
    {
        attrib          = noIndent;
        pos             = 0;
        singleIndentLen = 0; // number of spaces to indent !
    }
};

//-----------------------------------------------------------------------------
// debug.cpp
#ifdef DEBUG
extern char *traceDataType(DataTypes theType);
extern void traceInput(int line, InputStruct *pIn);
extern void traceOutput(int line, OutputStruct *pOut);
#define TRACE_INPUT(pOut)  traceInput(__LINE__, pOut);
#define TRACE_OUTPUT(pOut) traceOutput(__LINE__, pOut);
#else
#define TRACE_INPUT(pOut)  /* nothing */
#define TRACE_OUTPUT(pOut) /* nothing */
#endif

inline bool emptyString(const char *s)
{
    return s == 0 || *s == 0;
}

//-----------------------------------------------------------------------------
// exec_sql.cpp
extern void IndentSQL (OutputStruct *pOut, int& state);

// hanging.cpp
extern void IndentHanging (OutputStruct *pOut, int& state);

// FIXME
extern int LookupKeyword(const char *tst);
extern bool ContinuedQuote(OutputStruct *pOut);

// strings.cpp
extern bool isName(char c);
extern bool CompareKeyword(const char *tst, const char *ref);
extern char *NewString (const char *src);
extern char *NewSubstring (const char *src, size_t len);
extern const char *SkipBlanks(const char *s);

// tabs.cpp
extern void ExpandTabs (char* &pString,
    int tabLen,
    int deleteChars,
    Boolean quoteChars,
    CharState &curState, char * &lineState, Boolean &codeOnLine);
extern char* TabSpacing (int mode, int len, int spaceIndent);

#endif // _BCPP_HEADER
