#ifndef _CMDLINE_CODE
#define _CMDLINE_CODE

// Program C(++) Beautifier Written By Steven De Toni ACBC 11 12/94
//
// This module contains methods to parse the command line.

#include <stdlib.h>   // atoi()
#include <stdio.h>    // printf(), getc(), fprintf(), stderr
#include <string.h>   // strcmp()

#include "cmdline.h"  // prototypes

// Function converts a lower case string into upper case, any special 
// characters remain the same (i.e "$", "%" ...)
void StrUpr (char* pUpCase)
{
     while (*pUpCase != '\0')
     {
          if ((*pUpCase >= 'a') && (*pUpCase <= 'z'))
                  *pUpCase = (*pUpCase & 223);
          pUpCase++;
     }
}

// This function displays brief command line help to the user.
// Parameters:
// char* argv[]     : Pointer to command line parameter pointer array
//
void PrintProgramHelp (char* argv[])
{
    int userKey;

    printf ("\n C(++) Beautifier     V1.8\n");
    printf ("---------------------------\n");
    printf ("Program Was Written By Steven De Toni, December 1995\n");
    printf ("Modified/revised by Thomas E. Dickey <dickey@clark.net> 1996\n");
    printf ("All Parts Of This Program Are Freely Distributable.\n\n");

    printf ("Usage : %s [Parameters] [Input File Name] [Output File Name] \n\n", argv[0]);
    printf ("                    Available Command Line Parameters\n");
    printf ("                    ---------------------------------\n\n");
    printf ("[-bcl] [-bnl] [-cc <num>] [-f <num>] [-fi <string>] [-fnc <string>]\n");
    printf ("[-fo <string>] [-h] [-i <num>] [-lg] [-na] [-nb] [-nc] [-nlcnc] [-no]\n");
    printf ("[-nq] [-qb] [-s] [-t] [-ya] [-yb] [-ylcnc] [-yo] [-yq]\n");
    printf ("[<string>] [<string>]\n");
    printf ("\n[] = Optional <> = Parameter Requirement\n");
    printf ("\n* Support For I/O Redirection Is Provided *\n");

    fprintf (stderr, "\n More Detail Y/N ? ");  // make sure this is displayed !
    userKey = getc(stdin);
    userKey = (userKey & 223);                  // change key to upper case!

    if (userKey == 'Y')
    {
        printf ("-bcl          : -bcl         : Open Braces On Code Line\n");
        printf ("-bnl          : -bnl         : Open Braces On New Line\n");
        printf ("-cc  <num>    : -cc  50      : Comments With Code\n");
        printf ("-f   <num>    : -f   2       : Function Line Spacing\n");
        printf ("-fi <string>  : -fi  in.cpp  : Input File Name\n");
        printf ("-fnc <string> : -fnc c.cfg   : Load Custom Configuration File\n");
        printf ("-fo <string>  : -fo  out.cpp : Output File Name\n");
        printf ("-h    or   -? : -h   or  -?  : This Help\n");
        printf ("-i   <num>    : -i   4       : Indent Space Length\n");
        printf ("-lg           : -lg          : Leave Graphic Chars\n");
        printf ("-na           : -na          : Leave Non-ASCII Chars As Normal\n");
        printf ("-nb           : -nb          : Don't Backup Input File\n");
        printf ("-nc  <num>    : -nc  0       : Comments With No Code\n");
        printf ("-nlcnc        : -nlcnc       : Turn Off Leave Comments With NoCode\n");
        printf ("-no           : -no          : Turn Off Program Output (Unless Errors Occur)\n");
        printf ("-nq           : -nq          : Turn Off Non-ASCII Chars In Quotes To Octal\n");
        printf ("-qb  <num>    : -qb  10      : Define Internal Queue Buffer\n");
        printf ("-s            : -s           : Use Spaces In Indenting\n");
        printf ("-t            : -t           : Use Tabs In Indenting\n");
        printf ("-ya           : -ya          : Remove Non-ASCII Chars\n");
        printf ("-yb           : -yb          : Backup Input File With .bac Extension\n");
        printf ("-ylcnc        : -ylcnc       : Turn On Leave Comments With NoCode\n");
        printf ("-yo           : -yo          : Turn On Program Output\n");
        printf ("-yq           : -yq          : Turn On Non-ASCII Chars In Quotes To Octal\n");
        printf ("<string>      :      in.cpp  : Input File Name\n");
        printf ("<string>      :      out.cpp : Output File Name\n");
    }
}

// integer assignment
int intAssign (int& cmdCount, int argc, char* argv[] )
{
    if (cmdCount+1 <= (argc-1))
    {
        cmdCount++;
        return atoi (argv[cmdCount]);
    }
    else
    {
        fprintf (stderr, "Expected Another Integer Parameter For Command Directive %s\n", argv[cmdCount]);
        PrintProgramHelp (argv);
        cmdCount = -1;
        return cmdCount;
    }
}

// string assignment
char* strAssign (int& cmdCount, int argc, char* argv[])
{
    if (cmdCount+1 <= (argc-1))
    {
        cmdCount++;
        return argv[cmdCount];
    }
    else
    {
        fprintf (stderr, "Expected Another String Parameter For Command Directive %s\n", argv[cmdCount]);
        PrintProgramHelp (argv);
        cmdCount = -1;
        return NULL;
    }
}

// Function processes command line parameters
int ProcessCommandLine (int argc, char* argv[], Config& settings,
                        char*& pInFile, char*& pOutFile, char*& pConfig)
{

    int   cmdCount = 0;
    char* cmdRead;

    while (cmdCount < argc-1)
    {
        // command line error
        if (cmdCount < 0)
           return cmdCount;

        // next command to process!
        cmdCount++;
        cmdRead = argv[cmdCount];

        // this is a command directive
        if (cmdRead[0] == '-')
        {
            // upcase the command parameter
            StrUpr (cmdRead);

            cmdRead++;

            // Test for "No Output Wanted" = false
            if (strcmp ("NO", cmdRead) == 0)
            {
                settings.output = False;
                continue;
            }

            // Test for "Comments With No Code Position"
			if (strcmp ("NC", cmdRead) == 0)
            {
                settings.posOfCommentsNC = intAssign (cmdCount, argc, argv);
                continue;
            }

            // Test for "Leave Non-ASCII Chars As Normal"
            if (strcmp ("NA", cmdRead) == 0)
            {
                settings.deleteHighChars = 0;
                continue;
            }

            // Test for "Backup File" = no !
            if (strcmp ("NB", cmdRead) == 0)
            {
                settings.backUp = False;
                continue;
            }
            
            // Test for "Leave Comments No Code" = False 
            if (strcmp ("NLCNC", cmdRead) == 0)
            {
                settings.leaveCommentsNC = False;
                continue;
            }
            
            // Test for "Don't Convert Non-ASCII Chars In Quotes To Octal"
            if (strcmp ("NQ", cmdRead) == 0)
            {
                settings.quoteChars = False;
                continue;
            }

            // Test for "Leave Graphic Chars"
            if (strcmp ("LG", cmdRead) == 0)
            {
                settings.deleteHighChars = 3;
                continue;
            }

            // Test for "Comments With Code Position"
            if (strcmp ("CC", cmdRead) == 0)
            {
                settings.posOfCommentsWC = intAssign (cmdCount, argc, argv);
                continue;
            }

            // Test for  "File Name To Read New Configuration File"
            if (strcmp ("FNC", cmdRead) == 0)
            {
                pConfig = strAssign (cmdCount, argc, argv);
                continue;
            }

            // Test for "Input File Name"
            if (strcmp ("FI", cmdRead) == 0)
            {
                pInFile = strAssign (cmdCount, argc, argv);
                continue;
            }

            // Test for "Output File Name"
            if (strcmp ("FO", cmdRead) == 0)
            {
                pOutFile = strAssign (cmdCount, argc, argv);
                continue;
            }

            // Test for "Function Spacing"
            if (strcmp ("F", cmdRead) == 0)
            {
                settings.numOfLineFunc = intAssign (cmdCount, argc, argv);
                continue;
            }

            // Test for "Use Tabs In Indenting"
            if (strcmp ("T", cmdRead) == 0)
            {
                settings.useTabs = True;
                continue;
            }

            // Test for "Use Spaces In Indenting"
            if (strcmp ("S", cmdRead) == 0)
            {
                settings.useTabs = False;
                continue;
            }

            // Test for "Indent Spacing Length"
            if (strcmp ("I", cmdRead) == 0)
            {
                settings.tabSpaceSize = intAssign (cmdCount, argc, argv);
                continue;
            }

            // Test for "Open Brace On Code Line"
            if (strcmp ("BCL", cmdRead) == 0)
            {
                settings.braceLoc = False;
                continue;
            }

            // Test for "Open Brace On New Line"
            if (strcmp ("BNL", cmdRead) == 0)
            {
                settings.braceLoc = True;
                continue;
            }

            // Test for "Queue Buffer"
            if (strcmp ("QB", cmdRead) == 0)
            {
                settings.queueBuffer = intAssign (cmdCount, argc, argv);

                if (settings.queueBuffer < 2)
                   settings.queueBuffer = 2;

                continue;
            }

            // Test for "Remove Non-ASCII Chars"
            if (strcmp ("YA", cmdRead) == 0)
            {
                settings.deleteHighChars = 1;
                continue;
            }

            // Test for "Backup File" = yes
            if (strcmp ("YB", cmdRead) == 0)
            {
                settings.backUp = True;
                continue;
            }
            
            // Test for "Leave Comments No Code" = True 
            if (strcmp ("YLCNC", cmdRead) == 0)
            {
                settings.leaveCommentsNC = True;
                continue;
            }

            // Test for  "Output Wanted" = True
            if (strcmp ("YO", cmdRead) == 0)
            {
                settings.output = True;
                continue;
            }

            // Test for "Convert Non-ASCII Chars In Quotes To Octal"
            if (strcmp ("YQ", cmdRead) == 0)
            {
                settings.quoteChars = True;
                continue;
            }

            // ### display help ###
            if( (strcmp ("?", cmdRead) == 0) ||
                (strcmp ("H", cmdRead) == 0) )
            {
                printf ("*** Displaying Brief Help ***\n");
                PrintProgramHelp (argv);
                return -1;
            }

            fprintf (stderr, "Unknown Command Directive %s \n", cmdRead);
            PrintProgramHelp (argv);
            return -1;
        }
        else if (pInFile == NULL)              // Test for "Input File Name"
                pInFile  = argv [cmdCount];
             else if (pOutFile == NULL)        // Test for "Output File Name"
                pOutFile = argv [cmdCount];
             else
             {
                   fprintf (stderr, "Command Line Error : Expected Command Directive, Not %s\n", argv[cmdCount]);
                   PrintProgramHelp (argv);
                   return -1;
             }
    }

    return 0;
}

#endif //_CMDLINE_CODE
