#ifndef _CONFIG_HEADER
#define _CONFIG_HEADER

// Program C(++) beautifier Written By Steven De Toni ACBC 11 10/94
//
// This file contains prototypes, constants, enum declarations for the 
// source file config?.cpp. Prototypes declared can read data from a 
// text file a line at a time, and able to read parameters from a 
// configuration file.

#include <stdio.h>          // FILE Structure

enum Boolean     {False = 0, True = -1};

// Commonly-used characters that are awkward to represent
enum ConfigChars {NULLC = '\0',
                  TAB = '\t',
                  LF = '\n',
                  CR = '\r',
                  SPACE = ' ',
                  SEMICOLON = ';',
                  POUNDC = '#',
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
  Boolean useTabs        ;  // True = use tabs in indenting, False = use spaces
  int     posOfCommentsWC;  // position of comments on line with code
  int     posOfCommentsNC;  // position of comments on line
  // JZAS Start
  Boolean leaveCommentsNC;  // True = don't change the indentation of comments with code.
  // JZAS End
  Boolean quoteChars     ;  // change non-ASCII chars in quotes to octal notation
  int     deleteHighChars;  // 0  = no check         , 1 = delete high chars,
                            // 2  = don't delete graphics chars
  Boolean braceLoc       ;  // True = place on new line, False = at end of code
  Boolean output         ;  // Set this True for normal program output
  int     queueBuffer    ;  // Set the number if lines to store in memory at a time !
  Boolean backUp         ;  // backup the original file, have output file become input file name !
  Boolean indentPreP     ;  // indent preprocessor controls to match code
  Boolean indent_sql     ;  // indent embedded SQL statements
};


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Allocates memory for line in file, and places that the data in it.
// pInFile = the file handle to use when reading the file !
// EndOfFile variable is used to test if the end of the file has been reached.
//           When  this is true, the variable is changed to -1
//
// A string is returned with the contents the current line in the file,
// memory is allocated via the ReadLine routine, and should be deleted
// when not needed !
char* ReadLine (FILE *pInFile, int& EndOfFile);


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// This function is used to load the users configuration from a file.
//
// Parameters:
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
extern int SetConfig (FILE* pConfigFile, Config& userSettings);

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// This function is used to show the users configuration
//
// Parameters:
// userSettings : Config structure that contains the user settings.
//
// Return Values:
// int          : Returns the number of errors encountered when reading the
//                configuration file.
//
extern int ShowConfig(Config& userSettings);

#endif
