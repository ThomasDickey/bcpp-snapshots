#ifndef _CONFIG_HEADER
#define _CONFIG_HEADER

// Program C(++) beautifier Written By Steven De Toni ACBC 11 10/94
//
// This file contains prototypes, constants, enum declarations for the 
// source file config?.cpp. Prototypes declared can read data from a 
// text file a line at a time, and able to read parameters from a 
// configuration file.

#include <stdio.h>          // FILE Structure

static const char  pConfigWord[][29] = {
                                  ";"                         , // 0

                                  "FUNCTION_SPACING"          , // 1

                                  "USE_TABS"                  , // 2
                                  "INDENT_SPACING"            , // 3
                                  "NONASCII_QUOTES_TO_OCTAL"  , // 4

                                  "COMMENTS_WITH_CODE"        , // 5
                                  "COMMENTS_WITH_NOCODE"      , // 6
                                  //JZAS Start
				                  "LEAVE_COMMENTS_NOCODE"     , // 7
                                  // JZAS End

                                  "LEAVE_GRAPHIC_CHARS"       , // 8
                                  "ASCII_CHARS_ONLY"          , // 9
                                  "PLACE_BRACE_ON_NEW_LINE"   , // 10
                                  "PROGRAM_OUTPUT"            , // 11
                                  "QUEUE_BUFFER"              , // 12
                                  "BACKUP_FILE"               , // 13

                                  "="                         , // 14
                                  "YES"                       , // 15
                                  "ON"                        , // 16
                                  "NO"                        , // 17
                                  "OFF"                         // 18
                                };

enum Boolean     {False = 0, True = -1};

enum ConfigWords {ANYT = 0, FSPC, UTAB, ISPC, NAQTOOCT, COMWC, COMNC, LCNC,
                  LGRAPHC, ASCIIO, PBNLINE, PROGO, QBUF, BUF,
                  EQUAL, YES, ON, NO, OFF};

// Commonly-used characters that are awkward to represent
enum ConfigChars {NULLC = '\0',
                  TAB = 9,
                  LF = 10,
                  CR = 13,
                  SPACE = ' ',
                  SEMICOLON = ';',
                  L_CURL = '{',
                  R_CURL = '}',
                  DQUOTE = '"',
                  SQUOTE = '\'',
                  ESCAPE = '\\'};

// This structure is used to store the users settings that are read from a
// configuration file.
struct Config
{
  int     numOfLineFunc  ;  // number of lines between functions
  int     tabSpaceSize   ;  // number of spaces a tab takes up {4}
  Boolean useTabs        ;  // True = use tabs in indentingm, False = use spaces
  int     posOfCommentsWC;  // position of comments on line with code
  int     posOfCommentsNC;  // position of comments on line
  // JZAS Start
  Boolean leaveCommentsNC;  // True = don't change the indentation of comments with code.
  // JZAS End
  Boolean quoteChars     ;  // change non-ascii chars in quotes to octal notation
  int     deleteHighChars;  // 0  = no check         , 1 = delete high chars,
                            // 2  = dont delete graphics chars
  Boolean braceLoc       ;  // True = place on new line, False = at end of code
  Boolean output         ;  // Set this True for normal program output
  int     queueBuffer    ;  // Set the number if lines to store in memory at a time !
  Boolean backUp         ;  // backup the original file, have output file become input file name !
};


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Allocates memory for line in file, and places that the data in it.
// pInFile = the file handle to use when reading the file !
// EndOfFile variable is ued to test if the end of the file has been reached.
//           When  this is true, the variable is changed to -1
//
// A string is returned with the contents the current line in the file,
// memory is allocated via the ReadLine routine, and should be deleted
// when not needed !
char* ReadLine (FILE *pInFile, int& EndOfFile);


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\
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
char* FindConfigWords (char* pConfigLine, ConfigWords& type);


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// This function is used to generate a generic error message.
//
// Parameters:
// LineNo       : Line number where the error occured
// errorCode    : Error type to output to the user
// errorCount   : This variable is increment when this function is used
// pMessage     : Use by programmer to add additional information about the error
//
//
// Return Values:
// errorCount   : This variable is used to show how many errors have occured!
//
void ErrorMessage (int lineNo, int errorCode, int& errorCount, const char* pMessage = NULL);


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// ConfigAssignment function is used to assigned Boolean, or unsigned integer
// values from 0 - 500 to variables that are passed to it.
//
// Parameters:
// type       : This variable defines the keyword next to expect within config line.
// assignType : The variable is used to define what type of assignment to use,
//              (i.e 1 = Boolean, 2 = Integer)
// errorCount : Variable used to define how many error have occured. If any errors
//              encounted within the function, then this var will be incremented.
// PosInLine  : Defines a pointer to the starting location to read in data for
//              assignment from config data line (string).
// variable   : This defines the variables that's going to be altered, be boolean, or
//              integer.
//
// Return Values:
// errorCount : If any error occur within variable assignment, the a error
//              message is displayed, and this vaiable is incremented.
// variable   : If no errors have occured, then this variable will contain the value
//              that was set by the user !
//
void ConfigAssignment (ConfigWords type, int assignType, int& errorCount, int& configError, char* pPosInLine, int& variable);


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// This function is used to load the users configuration from a file.
//
// Parameters:
// pConfigFile  : Pointer to a FILE structure/handle that contains the
//                configuration data.
// userSettings : Config structure that will contain the user settings.
//
// Return Values:
// int          : Returns the number of errors encounted when reading the
//                configuration file.
// userSettings : This variable is altered to the user settings read from the
//                config file.
//
int SetConfig (FILE* pConfigFile, Config& userSettings);

#endif
