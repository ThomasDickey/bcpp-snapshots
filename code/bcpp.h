#ifndef _BCPP_HEADER
#define _BCPP_HEADER

// Common/miscellaneous definitions for BCPP

#ifdef DEBUG
#define TRACE(p) fprintf p;
#else
#define TRACE(p) /*nothing*/
#endif

enum CharState { Normal, Comment, DQuoted, SQuoted, Ignore  };

// tabs.cpp
extern char* ExpandTabs (char* pString, int tabLen, CharState &leftState, CharState &rightState);
extern char* TabSpacing (int mode, int len, int spaceIndent);

#endif // _BCPP_HEADER
