#ifndef _BCPP_HEADER
#define _BCPP_HEADER

// Common/miscellaneous definitions for BCPP

#ifdef DEBUG
#define TRACE(p) fprintf p;
#else
#define TRACE(p) /*nothing*/
#endif

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

// strings.cpp
extern char *NewString (const char *src);
extern char *NewSubstring (const char *src, size_t len);

// tabs.cpp
extern void ExpandTabs (char* &pString,
    int tabLen,
    int deleteChars,
    Boolean quoteChars,
    CharState &curState, char * &lineState, Boolean &codeOnLine);
extern char* TabSpacing (int mode, int len, int spaceIndent);

#endif // _BCPP_HEADER
