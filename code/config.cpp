#ifndef _CONFIG_CODE
#define _CONFIG_CODE

// Program C(++) beautifier Written By Steven De Toni ACBC 11 11/94
//
// This program module contains routines to read data from a text file a 
// line at a time, and able to read parameters from a configuration file.

#include "config.h"         // Prototypes, struct, and enum declarations ...
#include <stdlib.h>         // atol(),
#include <string.h>         // strlen(), strstr(), strcpy(), strcmp(), strpbrk()
#include <stdio.h>          // NULL constant, printf(), FILE, ftell(), fseek(), fprintf(), stderr
#include "cmdline.h"        // StrUpr()

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Allocates memory for line in file, and places that the data in it.
// pInFile = the file handle to use when reading the file !
// EndOfFile variable is ued to test if the end of the file has been reached.
//           When  this is true, the variable is changed to -1
//
// A string is returned with the contents the current line in the file,
// memory is allocated via the ReadLine routine, and should be deleted
// when not needed !
char* ReadLine (FILE *pInFile, int& EndOfFile)
{
    long int startOfLine, endOfLine;

    int lineLen     = 0;
    int testChar;

    startOfLine = ftell (pInFile);

    // find length
    testChar = fgetc(pInFile);

    while ( (testChar != LF) && (testChar > 0) ) // while not at end of file, or line feed
    {
        lineLen++;
        testChar = fgetc (pInFile);
    }

    // chack if at endoffile !
    if (testChar < 0)
    {
         lineLen++;
         EndOfFile = testChar;
    }

    // allocate buffer memory!
    char* pLineBuffer = new char [lineLen+1];

    if (pLineBuffer == NULL)
        return NULL;
    else
        pLineBuffer[lineLen] = NULLC;

    // reset position in file!
    endOfLine = ftell (pInFile);
    fseek (pInFile, startOfLine, 0);

    // place data into buffer!
    int counter = lineLen;
    lineLen = 0;

    testChar = fgetc(pInFile);
    while (counter > 0)
    {
        pLineBuffer[lineLen] = testChar;
        lineLen++;
        counter--;
        testChar = fgetc(pInFile);
    }

    fseek (pInFile, endOfLine, 0);

    return pLineBuffer;
}


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
char* FindConfigWords (char* pConfigLine, ConfigWords& type)
{
   char* pWordLoc = NULL;

   // check is there is a comment in the line, if so then any chars
   // after a ";" will be ignored !
   pWordLoc = strstr (pConfigLine, pConfigWord[0]); // search for a comment !
   if (pWordLoc != NULL)
   {
        *pWordLoc = NULLC;
   }

   if (type > ANYT)
   {
         pWordLoc = strstr (pConfigLine, pConfigWord[type]);

         if  (pWordLoc != NULL)  // if word found
         {
                // advance to next word !
                pWordLoc += strlen (pConfigWord[type]);
                return pWordLoc;
         }
   }

   for (int typeCount = (int) FSPC; typeCount < ((int)OFF)+1; typeCount++)
   {

         pWordLoc = strstr (pConfigLine, pConfigWord[typeCount]);

         if (pWordLoc != NULL)
         {
                type = (ConfigWords) typeCount;
                // advance to next word !
                pWordLoc += strlen (pConfigWord[typeCount]);
                return pWordLoc;
         }
   }

   type   = ANYT; // not a keyword !
   return NULL;
}


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
void ErrorMessage (int lineNo, int errorCode, int& errorCount, const char* pMessage)
{

    switch (errorCode)
    {
        case (1):
        {
            fprintf (stderr, "Syntax Error After Key Word ");
            break;
        }

        case (2):
        {
            fprintf (stderr, "Range Error  !");
            break;
        }

        case (3):
        {
            fprintf (stderr, "Expected Numeric Data !");
            break;
        }

        case (4):
        {
            fprintf (stderr, "Can't Decipher");
            break;
        }

    }

    if (pMessage != NULL)
       fprintf (stderr, "%s", pMessage);

    fprintf (stderr, " At Line %d\n", lineNo);

    errorCount++;
}


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
void ConfigAssignment (ConfigWords type, int assignType, int& errorCount, int& configError, char* pPosInLine, int& variable)
{

    switch (assignType)
    {
        case (1):
        {
            // check if key words are there
            pPosInLine = FindConfigWords (pPosInLine, type);

            switch (type)
            {
                case (YES):  // YES
                case (ON) :  // ON
                {
                    variable = True;
                    break;
                }

                case (NO) :  // NO
                case (OFF):  // OFF
                {
                    variable = False;
                    break;
                }

                default:
                        ErrorMessage (errorCount, 1, configError, pConfigWord[EQUAL]);

            } // switch

            break;
        }

        case (2):
        {
            // covert whats left in the string to an INTEGER !
            if (strpbrk(pPosInLine, "0123456789") != NULL)
                variable = atoi (pPosInLine);
            else
                ErrorMessage (errorCount, 3, configError);

            // check range of lines numbers between functions!
            if ( (variable < 0) || (variable > 500) )
                   ErrorMessage (errorCount, 2, configError, " Valid Range = 0 - 500");

            break;
        }

    } // switch (assignType)

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
// int          : Returns the number of errors encounted when reading the
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

             // ############################################################
             case (FSPC): // Function_Spacing = (%d)
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 2, lineCount, configError, pPosInLine, userSettings.numOfLineFunc);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[FSPC]);

                break;
             }// case (FSPC)

             // ############################################################
             case (UTAB): // use_tabs = {on, off, yes, no}
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                   ConfigAssignment (ANYT, 1, lineCount, configError, pPosInLine, (int&) userSettings.useTabs);
                else
                   ErrorMessage (lineCount, 1, configError, pConfigWord[UTAB]);

                break;
             }// case (UTAB)

             // ############################################################
             case (ISPC): // Indent_Spacing = (%d)
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 2, lineCount, configError, pPosInLine, userSettings.tabSpaceSize);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[ISPC]);

                break;
             }// case (ISPC)

             // ############################################################
             case (NAQTOOCT): // NONASCII_QUOTES_TO_OCTAL = {on, off, yes, no}
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 1, lineCount, configError, pPosInLine, (int&) userSettings.quoteChars);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[NAQTOOCT]);

                break;
             }// case (NAQTOOCT)

             // ############################################################
             case (COMWC): // Comments_With_Code = (%d)
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 2, lineCount, configError, pPosInLine, userSettings.posOfCommentsWC);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[COMWC]);

                break;
             }// case (COMWC)

             // ############################################################
             case (COMNC): // Comments_With_NoCode = (%d)
             {
                type = EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 2, lineCount, configError, pPosInLine, userSettings.posOfCommentsNC);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[COMNC]);

                break;
             }// case (COMNC)
             
             //JZAS Start
             // ############################################################
             case (LCNC):     // leave_comments_nocode = {on, off, yes, no}
             {
                type = EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 1, lineCount, configError, pPosInLine, (int&) userSettings.leaveCommentsNC);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[BUF]);

                break;
             }// case (LCNC)
             //JZAS End


             // ############################################################
             case (LGRAPHC): // LEAVE_GRAPHIC_CHARS = {on, off, yes, no}
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                {
                    int test = False;
                    ConfigAssignment (ANYT, 1, lineCount, configError, pPosInLine, test);

                    if (test == True)
                        userSettings.deleteHighChars = 3; //   set bit 0, 1
                    else
                        userSettings.deleteHighChars = 0; // unset bit 0, 1
                }
                else
                   ErrorMessage (lineCount, 1, configError, pConfigWord[LGRAPHC]);

                break;
             }// case (LGRAPHC)

             // ############################################################
             case (ASCIIO): // ASCII_CHARS_ONLY = {on, off, yes, no}
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                {
                    int test = False;
                    ConfigAssignment (ANYT, 1, lineCount, configError, pPosInLine, test);

                    if (test == True)
                        userSettings.deleteHighChars = 1;   //   set bit 0
                    else
                        userSettings.deleteHighChars = 0;   // unset bit 0
                }
                else
                   ErrorMessage (lineCount, 1, configError, pConfigWord[ASCIIO]);

                break;
             }// case (ASCIIO)

             // ############################################################
             case (PBNLINE): // Braces On Newline = {on, off, yes, no}
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 1, lineCount, configError, pPosInLine, (int&) userSettings.braceLoc);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[PBNLINE]);

                break;
             }// case (PBNLINE)

             // ############################################################
             case (PROGO): // program_output = {on, off, yes, no}
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 1, lineCount, configError, pPosInLine, (int&) userSettings.output);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[PROGO]);

                break;
             }// case (PROGO)

             // ############################################################
             case (QBUF): // queue_buffer = (%d)
             {
                type =  EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 2, lineCount, configError, pPosInLine, userSettings.queueBuffer);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[QBUF]);

                // minimum needed is a 2 line buffer !
                if (userSettings.queueBuffer < 2)
                      userSettings.queueBuffer = 2;

                break;
             }// case (QBUF)

             // ############################################################
             case (BUF):     // backup_file = {on, off, yes, no}
             {
                type = EQUAL;
                pPosInLine = FindConfigWords (pPosInLine, type);

                if (type == EQUAL) // expect a "="
                    ConfigAssignment (ANYT, 1, lineCount, configError, pPosInLine, (int&) userSettings.backUp);
                else
                    ErrorMessage (lineCount, 1, configError, pConfigWord[BUF]);

                break;
             }// case (BF)

             case (EQUAL):
             case (YES):
             case (ON):
             case (NO):
             case (OFF):
                    ErrorMessage (lineCount, 4, configError);
        }// switch

        delete pLineOfConfig;
  }// while

  return configError;
}

#endif
