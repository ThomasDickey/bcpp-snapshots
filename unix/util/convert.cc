// Program MS-DOS -> UNIX, UNIX -> MS-DOS text converter.
// Written by Steven De Toni for Unix people.

#include <stdio.h>      // FILE struct, fgetc(), ftell(), fseek()
#include <string.h>     // strlen()

enum ConfigChars {NULLC = '\0', TAB = 9, LF = 10, CR = 13, SPACE = ' ', SEMICOLON = ';'};

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
         EndOfFile = testChar;

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

int main (int argc , char* argv[])
{

    if (argc < 2)
    {
        printf ("Text Converter By Steven De Toni October 1994\n");
        printf ("\nExpected Input & Output Files\n");
        printf ("Usage : %s <infile> > [outfile]\n", argv[0]);
        printf ("\nN.B Uses Standard Output\n");
        return -1;
    }

    int   Eof      = 0;
    char* pOutput  = NULL;
    char* pTakeOut = NULL;
    FILE* pInFile  = fopen (argv[1], "rb");

    if (pInFile == NULL)
    {
        printf ("### Couldn't Open %s File", argv[1]);
        return -1;
    }

    while (Eof == 0)
    {
        pOutput = ReadLine (pInFile, Eof);

        // remove carriage returns
        if (pOutput != NULL)
        {
            pTakeOut = pOutput + (strlen (pOutput));
            do
            {
                pTakeOut--;
            } while ((*pTakeOut == LF) || (*pTakeOut == CR));

            // finsh up
            pTakeOut[1] = NULLC;

            printf ("%s\n", pOutput);
            delete pOutput;
        }
    }

    fclose (pInFile);
    return 0;
}


