// $Id: config.cpp,v 1.23 2005/07/25 20:54:07 tom Exp $
// Program C(++) beautifier Written By Steven De Toni ACBC 11 11/94
//
// This program module contains routines to read data from a text file a 
// line at a time, and able to read parameters from a configuration file.

#include <stdlib.h>         // atol(),
#include <string.h>         // strlen(), strstr(), strcpy(), strcmp(), strpbrk()
#include <stdio.h>          // NULL constant, printf(), FILE, ftell(), fseek(), fprintf(), stderr

#include "bcpp.h"
#include "cmdline.h"        // StrUpr()

enum ConfigWords {ANYT = 0, FSPC, UTAB, ISPC, IPRO, ISQL,
                  NAQTOOCT, COMWC, COMNC, LCNC,
                  LGRAPHC, ASCIIO, BI, BI2, PTBNLINE, PBNLINE, PROGO, QBUF, BUF,
                  EQUAL, YES, ON, NO, OFF};

static const struct { ConfigWords code; const char *name; }
    ConfigData[] = {
    { ANYT,     ";" },
    { FSPC,     "FUNCTION_SPACING" },
    { UTAB,     "USE_TABS" },
    { ISPC,     "INDENT_SPACING" },
    { IPRO,     "INDENT_PREPROCESSOR" },
    { ISQL,     "INDENT_EXEC_SQL" },
    { NAQTOOCT, "NONASCII_QUOTES_TO_OCTAL" },
    { COMWC,    "COMMENTS_WITH_CODE" },
    { COMNC,    "COMMENTS_WITH_NOCODE" },
    { LCNC,     "LEAVE_COMMENTS_NOCODE" },
    { LGRAPHC,  "LEAVE_GRAPHIC_CHARS" },
    { ASCIIO,   "ASCII_CHARS_ONLY" },
    { BI,       "BRACE_INDENT" },
    { BI2,      "INDENT_BOTH_BRACES" },
    { PTBNLINE, "PLACE_TOP_BRACE_ON_NEW_LINE" },
    { PBNLINE,  "PLACE_BRACE_ON_NEW_LINE" },
    { PROGO,    "PROGRAM_OUTPUT" },
    { QBUF,     "QUEUE_BUFFER" },
    { BUF,      "BACKUP_FILE" },
    { EQUAL,    "=" },
    { YES,      "YES" },
    { ON,       "ON" },
    { NO,       "NO" },
    { OFF,      "OFF" }
    };

static const size_t SizeofData = TABLESIZE(ConfigData);

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Allocates memory for line in file, and places that the data in it.
// pInFile = the file handle to use when reading the file !
// EndOfFile variable is used to test if the end of the file has been reached.
//           When  this is true, the variable is changed to -1
//
// A string is returned with the contents the current line in the file,
// memory is allocated via the ReadLine routine, and should be deleted
// when not needed !
char* ReadLine (FILE *pInFile, int& EndOfFile)
{
    const int nominal = 80;
    int used = nominal;
    int need = 0;
    int ch;
    char* pLineBuffer = new char [used];

    for(;;) {
        ch = fgetc(pInFile);
        if (ch < 0) {
            EndOfFile = ch;
            break;
        } else if (ch == LF) {
            break;
        }
        if (need + 2 > used) {
            used = ((need + 2) * 3) / 2;
            char *temp = new char [used];
            for (int n = 0; n < need; n++)
                temp[n] = pLineBuffer[n];
            delete[] pLineBuffer;
            pLineBuffer = temp;
        }
        pLineBuffer[need++] = ch;
    }
    pLineBuffer[need] = 0;
    return pLineBuffer;
}


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Lookup keyword in ConfigData[]
static const char *ConfigWordOf(ConfigWords code)
{
    for (size_t i = 0; i < SizeofData; i++)
        if (ConfigData[i].code == code)
            return ConfigData[i].name;
    return 0;
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// This function is used to generate a generic error message.
//
// Parameters:
// LineNo       : Line number where the error occurred
// errorCode    : Error type to output to the user
// errorCount   : This variable is increment when this function is used
// pMessage     : Use by programmer to add additional information about the error
//
//
// Return Values:
// errorCount   : This variable is used to show how many errors have occurred!
//
static void ErrorMessage (int lineNo, int errorCode, int& errorCount, const char* pMessage = NULL)
{

    switch (errorCode)
    {
        case (1):
        {
            warning ("Syntax Error After Key Word ");
            break;
        }

        case (2):
        {
            warning ("Range Error  !");
            break;
        }

        case (3):
        {
            warning ("Expected Numeric Data !");
            break;
        }

        case (4):
        {
            warning ("Can't Decipher");
            break;
        }

    }

    if (pMessage != NULL)
       warning ("%s", pMessage);

    warning (" At Line %d\n", lineNo);

    errorCount++;
}


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Functions finds keywords within a line of data.
//
// Parameters:
// Type :
//        The parameter is used to define the type of keyword to find within
//        a configuration line.
//
//        See ConfigWords enum for values, Use ANYT, or a value of 0 to search
//        for any valid keywords within the line.
//
// pConfigLine :
//        This parameter is a pointer to a string that contains the data that
//        is going to be searched.
//
// Return Values:
//
// Type : Returns the keyword value expected, or keyword value found if
//        searching for any.
//
// Char*: Returns a pointer in the string to the next starting location
//        AFTER the keyword found. Or returns NULL if no keyword found!
//
static char* FindConfigWords (char* pConfigLine, ConfigWords& type)
{
   char* pWordLoc = NULL;

   // check is there is a comment in the line, if so then any chars
   // after a ";" will be ignored !
   pWordLoc = strstr (pConfigLine, ConfigWordOf(ANYT)); // search for a comment !
   if (pWordLoc != NULL)
   {
        *pWordLoc = NULLC;
   }

   if (type > ANYT)
   {
         pWordLoc = strstr (pConfigLine, ConfigWordOf(type));

         if  (pWordLoc != NULL)  // if word found
         {
                // advance to next word !
                pWordLoc += strlen (ConfigWordOf(type));
                return pWordLoc;
         }
   }

   for (size_t typeCount = 1; typeCount < SizeofData; typeCount++)
   {
         const char *name = ConfigData[typeCount].name;
         pWordLoc = strstr (pConfigLine, name);

         if (pWordLoc != NULL)
         {
                type = ConfigData[typeCount].code;
                // advance to next word !
                pWordLoc += strlen (name);
                return pWordLoc;
         }
   }

   type   = ANYT; // not a keyword !
   return NULL;
}


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// ConfigAssignment function is used to assigned Boolean, or unsigned integer
// values from 0 - 5000 to variables that are passed to it.
//
// Parameters:
// errorCount : Variable used to define how many errors have occurred. If any errors
//              encountered within the function, then this var will be incremented.
// PosInLine  : Defines a pointer to the starting location to read in data for
//              assignment from config data line (string).
// variable   : This defines the variables that's going to be altered, be boolean, or
//              integer.
//
// Return Values:
// errorCount : If any error occur within variable assignment, the a error
//              message is displayed, and this variable is incremented.
// variable   : If no errors have occurred, then this variable will contain the value
//              that was set by the user !
//
static void ConfigAssignment (int& errorCount, int& configError, char* pPosInLine, int& variable)
{
    // convert what's left in the string to an INTEGER !
    if (strpbrk(pPosInLine, "0123456789") != NULL)
        variable = atoi (pPosInLine);
    else
        ErrorMessage (errorCount, 3, configError);

    // check range of lines numbers between functions!
    if ( (variable < 0) || (variable > 5000) )
        ErrorMessage (errorCount, 2, configError, " Valid Range = 0 - 5000");
}

static void ConfigAssignment (int& errorCount, int& configError, char* pPosInLine, Boolean& variable)
{
    ConfigWords type = ANYT;

    // check if key words are there
    pPosInLine = FindConfigWords (pPosInLine, type);

    switch (type)
    {
        case (YES):  // YES
        case (ON) :  // ON
            variable = True;
            break;

        case (NO) :  // NO
        case (OFF):  // OFF
            variable = False;
            break;

        default:
            ErrorMessage (errorCount, 1, configError, ConfigWordOf(EQUAL));

    } // switch
}

#define DecodeIt(value) \
    { \
        ConfigWords tesType = EQUAL; \
        pPosInLine = FindConfigWords (pPosInLine, tesType); \
    \
        if (tesType != EQUAL) { \
            ErrorMessage (lineCount, 1, configError, ConfigWordOf(type)); \
            break; \
        } else \
            ConfigAssignment (lineCount, configError, pPosInLine, value); \
    }

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// This function is used to load the users configuration from a file.
//
// Parameters:       2
// pConfigFile  : Pointer to a FILE structure/handle that contains the
//                configuration data.
// userSettings : Config structure that will contain the user settings.
//
// Return Values:
// int          : Returns the number of errors encountered when reading the
//                configuration file.
// userSettings : This variable is altered to the user settings read from the
//                config file.
//
int SetConfig (FILE* pConfigFile, Config& userSettings)
{

  int         noMoreConfig  = 0   ;
  char*       pLineOfConfig = NULL;
  char*       pPosInLine    = NULL;
  ConfigWords type                ;
  int         lineCount     = 0   ;
  int         configError   = 0   ;
  Boolean     test;

  while (! noMoreConfig)
  {

        pLineOfConfig = ReadLine (pConfigFile, noMoreConfig);

        lineCount++;

        // upcase all characters in string !
        StrUpr (pLineOfConfig);

        type = ANYT;
        pPosInLine = FindConfigWords (pLineOfConfig, type);

        switch (type)
        {
             case (FSPC):   // FUNCTION_SPACING = (%d)
                DecodeIt (userSettings.numOfLineFunc);
                break;

             case (UTAB):   // USE_TABS = {on, off, yes, no}
                DecodeIt (userSettings.useTabs);
                break;

             case (ISPC):   // INDENT_SPACING = (%d)
                DecodeIt (userSettings.tabSpaceSize);
                break;

             case (IPRO):   // INDENT_PREPROCESSOR = (%d)
                DecodeIt (userSettings.indentPreP);
                break;

             case (ISQL):   // INDENT_EXEC_SQL = (%d)
                DecodeIt (userSettings.indent_sql);
                break;

             case (NAQTOOCT): // NONASCII_QUOTES_TO_OCTAL = {on, off, yes, no}
                DecodeIt (userSettings.quoteChars);
                break;

             case (COMWC):  // COMMENTS_WITH_CODE = (%d)
                DecodeIt (userSettings.posOfCommentsWC);
                break;

             case (COMNC):  // COMMENTS_WITH_NOCODE = (%d)
                DecodeIt (userSettings.posOfCommentsNC);
                break;
             
             case (LCNC):   // LEAVE_COMMENTS_NOCODE = {on, off, yes, no}
                DecodeIt (userSettings.leaveCommentsNC);
                break;

             case (LGRAPHC): // LEAVE_GRAPHIC_CHARS = {on, off, yes, no}
                test = False;
                DecodeIt (test);
                if (test == True)
                    userSettings.deleteHighChars = 3; //   set bit 0, 1
                else
                    userSettings.deleteHighChars = 0; // unset bit 0, 1
                break;

             case (ASCIIO): // ASCII_CHARS_ONLY = {on, off, yes, no}
                test = False;
                DecodeIt (test);
                if (test == True)
                    userSettings.deleteHighChars = 1;   //   set bit 0
                else
                    userSettings.deleteHighChars = 0;   // unset bit 0
                break;

             case (BI): // BRACE_INDENT = {on, off, yes, no}
                DecodeIt (userSettings.braceIndent);
                break;

             case (BI2): // INDENT_BOTH_BRACES = {on, off, yes, no}
                DecodeIt (userSettings.braceIndent2);
                break;

             case (PTBNLINE): // PLACE_TOP_BRACE_ON_NEW_LINE = {on, off, yes, no}
                DecodeIt (userSettings.topBraceLoc);
                break;

             case (PBNLINE): // PLACE_BRACE_ON_NEW_LINE = {on, off, yes, no}
                DecodeIt (userSettings.braceLoc);
                break;

             case (PROGO): // PROGRAM_OUTPUT = {on, off, yes, no}
                DecodeIt (userSettings.output);
                break;

             case (QBUF): // queue_buffer = (%d)
                DecodeIt (userSettings.queueBuffer);
                if (userSettings.queueBuffer < 2)
                      userSettings.queueBuffer = 2;
                break;

             case (BUF):     // backup_file = {on, off, yes, no}
                DecodeIt (userSettings.backUp);
                break;

             case (ANYT):
                break;

             default:
                warning("Text:%s\n", pLineOfConfig);
                ErrorMessage (lineCount, 4, configError);
                break;
        }// switch

        delete[] pLineOfConfig;
  }// while

  return configError;
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// This function is used to show the users configuration.
//
// Parameters:
// userSettings : Config structure that contains the user settings.
//
// Return Values:
// int          : Returns the number of errors encountered when reading the
//                configuration file.
//
int ShowConfig (Config& userSettings)
{
    const char* choices[2] = {"Yes", "No"};
    int errorNum = 0;

    verbose ("Function Line Spacing              : %d\n", userSettings.numOfLineFunc);
    verbose ("Use Tabs In Indenting              : %s\n", choices[userSettings.useTabs+1]);
    verbose ("Indent Spacing Length              : %d\n", userSettings.tabSpaceSize);
    verbose ("Comments With Code                 : %d\n", userSettings.posOfCommentsWC);
    if (userSettings.leaveCommentsNC != False)
        verbose ("Comments With No Code              : Indented According To Code\n");
    else
        verbose ("Comments With No Code              : %d\n", userSettings.posOfCommentsNC);
    verbose ("Remove Non-ASCII Chars             : ");

    switch (userSettings.deleteHighChars)
    {
        case (0):
            verbose ("No\n");
            break;
        case (1):
            verbose ("Yes\n");
            break;
        case (3):
            verbose ("Yes But Not Graphic Chars\n");
            break;
        default:
            warning ("#### ERROR : Unexpected Value %d", userSettings.deleteHighChars);
            errorNum++;
    }

    verbose ("Non-ASCII Chars In Quotes To Octal : %s\n", choices[userSettings.quoteChars+1]);
    verbose ("Top-level Open Braces On New Line  : %s\n", choices[userSettings.topBraceLoc+1]);
    verbose ("Open Braces On New Line            : %s\n", choices[userSettings.braceLoc+1]);
    verbose ("Program Output                     : %s\n", choices[userSettings.output+1]);
    verbose ("Internal Queue Buffer Size         : %d\n", userSettings.queueBuffer);

    if (errorNum > 0
    && prompt("Do You Wish To Continue To Process Files "))
        errorNum = 0;

    return errorNum;
}
