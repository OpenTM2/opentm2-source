// Copyright (c) 2013, International Business Machines
// Corporation and others.  All rights reserved.

// ***************************************************************************
//
//    AUTHOR: Markus Conrad (Base of program received from Gerhard Queck)
//
//    Short description: This program reads a definition file containing
//                       // Text:   and // Help:  statements.
//                       The program creates the text and help file and the
//                       definition file is renumbered. If the program
//                       detects errors then it adds !! Error and
//                       !! Warning statements to the definition file.
//                       
//                       GQ 2010/11/24
//                       Changed message output to string table rather than
//                       OS/2 message file source
// ***************************************************************************
#include <eqf.h>

#define MAXTEXTCOL     200
#define TRUE 1
#define FALSE 0

#define DEFAULT_MESSAGE_HELP_TITLE_ENG     "Help for Message: EQF"
#define DEFAULT_MESSAGE_HELP_TITLE_ITA     "Aiuto per Messaggio: EQF"
#define DEFAULT_MESSAGE_HELP_TITLE_FRA     "Aide : Message EQF"
#define DEFAULT_MESSAGE_HELP_TITLE_GER     "Hilfe fr Nachricht: EQF"
#define DEFAULT_MESSAGE_HELP_TITLE_JAP     "ƒwƒ‹ƒv: EQF"
#define DEFAULT_MESSAGE_TITLE_ENG          ":h1 res=65010.Messages\n"
#define DEFAULT_MESSAGE_TITLE_ITA          ":h1 res=65010.Messaggi\n"
#define DEFAULT_MESSAGE_TITLE_FRA          ":h1 res=65010.Messages\n"
#define DEFAULT_MESSAGE_TITLE_GER          ":h1 res=65010.Nachrichten\n"
#define DEFAULT_MESSAGE_TITLE_JAP          ":h1 res=65010.Messages\n"
#define TEXT_NO_MESSAGE_TEXT           "// Text:       !! No message text defined !!"
#define WARNING_NO_MESSAGE_TEXT        "//* !! Warning: No message text defined !!"
#define WARNING_NO_HELP_TITLE          "//* !! Warning: No help title defined !!"
#define HELPTITLE_NO_HELP_TITLE        "// HelpTitle:  OpenTM2 - Help"
#define NO_HELP_TEXT                   "!! No help text defined !!"
#define NO_HELP_TITLE                  "!! No help title defined !!"
#define WARNING_NO_HELP_TEXT           "//* !! Warning: No help text defined !!"
#define HELP_NO_HELP_TEXT              "// Help:       !! No help text defined !!"


typedef enum _STATE
{
   TEXT_OF_MSG, HELP_OF_MSG, HELPTITLE_OF_MSG, SEV_OF_MSG, TYPE_OF_MSG,
   CAUSE_OF_MSG, ADDINFO_OF_MSG, NO_STATE
} STATE, *PSTATE;

FILE *Input           = NULL;
FILE *OutputMsgFile   = NULL;
FILE *OutputHelpFile  = NULL;
FILE *OutputHelpIDFile= NULL;
FILE *hBakFile = NULL;
   BOOL fOK = TRUE;
   BOOL fText;
   BOOL fHelp;
   BOOL fHelpTitle;
   BOOL fDefineActive;
   BOOL fFree;                           // indicator for not used messages
   int usCurrent = 0;
   int usLine = 0;
   int usNoOfFields;
   int usDosRc;
   DOSVALUE usBytesWritten;
   char InBuf[512];
   char OutBuf[512];
   char TempBuf[1024];
   char chDefine[512];
   char chMsg[512];
   char chNumber[512];
   char szTextLine1[512];
   char *pChar;
   ULONG ulNewPointer;
   BOOL fWarning = 0;
   BOOL fError   = 0;

   CHAR szLanguage[20];
   CHAR szDefaultMessageHelpTitle[30];
   CHAR szDefaultMessageTitle[30];

    HFILE                hFile;                     // File handle
    DOSVALUE             usAction;                  // Action taken
    FILESTATUS           stStatus;                  // File status information
PSZ pszInFile;

VOID HandleNoText( VOID );

VOID WriteMessageText( FILE *OutputMsgFile, char *pszText );

int main(int argc, char* argv[])
{
  BOOL   fDBCS = FALSE;                // no DBCS
  BOOL   fLFPending = FALSE;           // is an additional LF necessary ??
   STATE CurrentState = NO_STATE;
   int rc = FALSE;

   argv++;
   argc--;

   fOK = TRUE;
   fText = FALSE;
   fHelp = FALSE;
   fHelpTitle = FALSE;
   fDefineActive = FALSE;
   szTextLine1[0] = '\0';
   pszInFile = argv[0];

   if ( argc != 5)
   {
      printf("Error: Incorrect number of parameters\n");
      printf("Invokation:\n");
      printf("EQFMSG InFile  OutMsgFile  OutHelpFile  OutHelpIDFile Language\n");
      printf("       |       |           |            |             + Which language\n");
      printf("       |       |           |            + Will contain the help IDs\n");
      printf("       |       |           + Will contain the help text\n");
      printf("       |       + Will contain the message text\n");
      printf("       + Must contains #define, // Text: and // Help: statements\n");
      fOK = FALSE;
   } /* endif */

// --------------------------------------------------------

   if ( fOK )
   {
     if ( !_stricmp( argv[4], "FRENCH" ) )
     {
       strcpy( szDefaultMessageHelpTitle, DEFAULT_MESSAGE_HELP_TITLE_FRA );
     }
     else if ( !_stricmp( argv[4], "GERMAN" ) )
     {
       strcpy( szDefaultMessageHelpTitle, DEFAULT_MESSAGE_HELP_TITLE_GER );
     }
     else if ( !_stricmp( argv[4], "ITALIAN" ) )
     {
       strcpy( szDefaultMessageHelpTitle, DEFAULT_MESSAGE_HELP_TITLE_ITA );
     }
     else if ( !_stricmp( argv[4], "JAPANESE" ) )
     {
       strcpy( szDefaultMessageHelpTitle, DEFAULT_MESSAGE_HELP_TITLE_JAP );
       fDBCS = TRUE;
     }
     else
     {
       strcpy( szDefaultMessageHelpTitle, DEFAULT_MESSAGE_HELP_TITLE_ENG );
     } /* endif */
   } /* endif */


   // -------  Open the message file  ------------------------

   if ( fOK )
   {
      OutputMsgFile = fopen( argv[1], "w" );
      if ( OutputMsgFile == NULL )
      {
         printf("Error: File %s could not be opened in WRITE mode\n",
                argv[1] );
         fOK = FALSE;
      }
      else
      {
         fputs( "/* file generated by EQFMSG V0.0.4 - the message file compiler */\n",
                OutputMsgFile );
         fputs( "STRINGTABLE\nBEGIN\n", OutputMsgFile );
      } /* endif */
   } /* endif */

// -------  Open the Help file    -------------------------

   if ( fOK )
   {
      OutputHelpFile = fopen( argv[2], "w" );
      if ( OutputHelpFile == NULL )
      {
         printf("Error: File %s could not be opened in WRITE mode\n",
                argv[2]);
         fOK = FALSE;
      }
      else
      {
         fputs( ".* This file was generated by EQFMSG the message file compiler\n",
                OutputHelpFile );
         if ( !_stricmp( argv[4], "FRENCH" ) )
         {
           strcpy( szDefaultMessageTitle, DEFAULT_MESSAGE_TITLE_FRA );
         }
         else
         {
           if ( !_stricmp( argv[4], "GERMAN" ) )
           {
             strcpy( szDefaultMessageTitle, DEFAULT_MESSAGE_TITLE_GER );
           }
           else
           {
             if ( !_stricmp( argv[4], "ITALIAN" ) )
             {
               strcpy( szDefaultMessageTitle, DEFAULT_MESSAGE_TITLE_ITA );
             }
             else
             {
               strcpy( szDefaultMessageTitle, DEFAULT_MESSAGE_TITLE_ENG );
             } /* endif */
           } /* endif */
         } /* endif */
         fputs ( szDefaultMessageTitle, OutputHelpFile );
      } /* endif */
   } /* endif */

// -------  Open the Help ID file -------------------------

   if ( fOK )
   {
      OutputHelpIDFile = fopen( argv[3], "w" );
      if ( OutputHelpIDFile == NULL )
      {
         printf("Error: File %s could not be opened in WRITE mode\n",
                argv[3]);
         fOK = FALSE;
      }
      else
      {
         // Write any output to the top of  OutputHelpIDFile if required.
      } /* endif */
   } /* endif */

   // -------  Open the message source file as input file ---------------

   if ( fOK )
   {
      Input = fopen( argv[0], "r" );
      if ( Input == NULL )
      {
         printf("Error: Inputfile %s could not be opened for READ\n", argv[0] );
         fOK = FALSE;
      } /* endif */
   } /* endif */

   // --------------------------------------------------------

   if ( fOK )
   {
      while ( !feof( Input ) )
      {
       fgets( InBuf, 512, Input );
       if ( !feof( Input ) )
       {
         if ( InBuf[0] != '\0' )
         {
            InBuf[ strlen(InBuf) - 1 ] = '\0';
         } /* endif */
         usLine++;
         if ( _strnicmp( InBuf, "#define", 7 ) == 0 )
         {
            usNoOfFields = sscanf( InBuf, "%s %s %s",
                                   chDefine, chMsg, chNumber );

            // Check whether the define constant is larger then 31 Bytes
            if ( strlen( chMsg ) > 31  )
            {
             printf( "%s(%d): Warning: Define constant exceeds 31 characters\n",
                      argv[0], usLine );
			fWarning = TRUE;
            } /* endif */

            if ( fDefineActive )
            {
              fprintf( OutputHelpIDFile, "    %-31s, id_msg_subtable , %-31s,\n", chMsg, chMsg );
              usCurrent = atoi(chNumber);

              // Initialize text line 1 to NULL
              szTextLine1[0] = '\0';
            }
            else
            {
               if ( usCurrent != 0 )
               {
                  HandleNoText();
				  
				  // complete current message string
				  if ( fText ) fputs( "\"\n", OutputMsgFile );

               } /* endif */

               usCurrent = atoi(chNumber);

               if  ( (!strncmp(chMsg,"FREE",4))  &&
                     (!strcmp(chMsg+10,"xxxxxxxxxxxxxxxxxxxxx")) )
               {
                  fprintf( OutputMsgFile, "\n%d \"", usCurrent );
                  fFree = TRUE;
               }
               else
               {
                  fprintf( OutputMsgFile, "\n%d \"", usCurrent );
                  fprintf( OutputHelpFile, ".*\n.*\n:h2 res=%d.", usCurrent );
                  fFree = FALSE;
               }
               fText = FALSE;
               fHelp = FALSE;
               fHelpTitle = FALSE;
               fprintf( OutputHelpIDFile, "    %-31s, id_msg_subtable , %-31s,\n", chMsg, chMsg );

               // Initialize text line 1 to NULL
               szTextLine1[0] = '\0';
            } /* endif */
            fDefineActive = TRUE;
         }
         else if ( strncmp( InBuf, "//*", 3) == 0)
         {
         }
         else if ( strncmp( InBuf, "//", 2) == 0)
         {
            fDefineActive = FALSE;
            if ( strlen( InBuf ) > MAXTEXTCOL )
            {
               printf( "%s(%d): Warning: Line is too long and has to be truncated.\n",
                        argv[0], usLine );
				fWarning = TRUE;
            } /* endif */
            if ( usCurrent == 0 )
            {
               /* ignore everything before the first define statement         */
            }
            else if ( _strnicmp( InBuf, "// Text:       ", 15) == 0)
            {
              if (fText)
              {
                printf( "%s(%d): Error: Only one // Text:  statement is allowed!\n",
                         argv[0], usLine );
                fError = TRUE;
              }
              else
              {
                CurrentState = TEXT_OF_MSG;

                // Save text line 1
                strncpy( szTextLine1, InBuf+15, MAXTEXTCOL );
                szTextLine1[MAXTEXTCOL] = '\0';

                // Put the text line to the message file
                strncpy( OutBuf, InBuf+15, MAXTEXTCOL );
                OutBuf[MAXTEXTCOL] = '\0';
                WriteMessageText( OutputMsgFile, OutBuf );
                fText = TRUE;
              } /* endif */
            }
            else if ( _strnicmp( InBuf, "// HelpTitle:  ", 15) == 0)
            {
              if (fHelpTitle)
              {
                printf( "%s(%d): Error: Only one  // HelpTitle:  statement is allowed.!\n",
                         argv[0], usLine );
                fError = TRUE;
              }
              else
              {
                CurrentState = HELPTITLE_OF_MSG;

                strncpy( OutBuf, InBuf+15, MAXTEXTCOL);
                OutBuf[MAXTEXTCOL] = '\0';

                if ( !fFree )
                {
                  fprintf( OutputHelpFile, "%s%04d\n",
                           szDefaultMessageHelpTitle, usCurrent );
                } /* endif */

                fHelpTitle = TRUE;
              } /* endif */
            }
            else if ( _strnicmp( InBuf, "// Help:       ", 15) == 0)
            {
              if (fHelp)
              {
                printf( "%s(%d): Error: Only one  // HelpTitle:  statement is allowed.!\n",
                         argv[0], usLine );
                fError = TRUE;
              }
              else
              {
                CurrentState = HELP_OF_MSG;
                if ( !fHelpTitle )
                {
                  if ( !fFree )
                  {
                    fprintf( OutputHelpFile, szDefaultMessageHelpTitle );
                  } /* endif */
                  fHelpTitle = TRUE;
                } /* endif */

                strncpy( OutBuf, InBuf+15, MAXTEXTCOL );
                OutBuf[MAXTEXTCOL] = '\0';
                if ( !fFree )
                {
                  fprintf( OutputHelpFile, "%s\n", OutBuf );
                } /* endif */
                fHelp = TRUE;

                // If help text "No help ......." then renew the warning
                if ( _strnicmp( InBuf, HELP_NO_HELP_TEXT, 41) == 0)
                {
                  if ( !fFree )
                  {
//                    fprintf( OutputHelpFile, ":p.%s\n", szTextLine1 );
                  } /* endif */
                } /* endif */
              } /* endif */
            }
            else if ( _strnicmp( InBuf, "// Type:       ", 15) == 0)
            {
               CurrentState = TYPE_OF_MSG;
            }
            else if ( _strnicmp( InBuf, "// Severity:   ", 15) == 0)
            {
               CurrentState = SEV_OF_MSG;
            }
            else if ( _strnicmp( InBuf, "// Cause:      ", 15) == 0)
            {
               CurrentState = CAUSE_OF_MSG;
            }
            else if ( _strnicmp( InBuf, "// Add.Info:   ", 15) == 0)
            {
               CurrentState = ADDINFO_OF_MSG;
            }
            else if ( _strnicmp( InBuf, "//             ", 15) == 0)
            {
               switch ( CurrentState )
               {
                  case TEXT_OF_MSG :
                     strncpy( OutBuf, InBuf+15, MAXTEXTCOL );
                     OutBuf[MAXTEXTCOL] = '\0';
                     WriteMessageText( OutputMsgFile, " " );
                     WriteMessageText( OutputMsgFile, OutBuf );
                     break;
                  case HELP_OF_MSG :
                     strncpy( OutBuf, InBuf+15, MAXTEXTCOL );
                     OutBuf[MAXTEXTCOL] = '\0';
                     if ( !fFree )
                     {
                       fprintf( OutputHelpFile, "%s\n", OutBuf );
                     } /* endif */
                     break;
                  default :
                     break;
               } /* endswitch */
            }
            else
            {
               printf( "%s(%d): Error: Line contains a syntax error and was not interpreted.\n",
                       argv[0], usLine );
               fError = TRUE;
            } /* endif */
         }
         else
         {
            fDefineActive = FALSE;

            // Do only write non blank lines into the H file with error message
            if ( InBuf[0] != '\0' )
            {
              printf( "%s(%d): Error: Line contains a syntax error and was not interpreted.\n",
                      argv[0], usLine );
              fError = TRUE;
            }
         } /* endif */
       }
       else
       {
         // End of input file has been reached: Complete not terminated define statements
         if ( !fDefineActive && usCurrent != 0 )
         {
           HandleNoText();
         } /* endif */
       } /* endif */

      } /* endwhile */
   } /* endif */

   if ( Input != NULL )          fclose( Input );
   if ( OutputMsgFile != NULL )  
   {
		if ( fText ) fputs( "\"\n", OutputMsgFile );
		fputs( "\nEND\n", OutputMsgFile );
    	fclose( OutputMsgFile );
   }
   if ( OutputHelpFile != NULL )
   {
     fputs( ".* End of output generated by EQFMSG\n", OutputHelpFile );
     fclose( OutputHelpFile );
   } /* endif */
   if ( OutputHelpIDFile != NULL )
   {
     fputs( "    0,                               NULL,             0\n", OutputHelpIDFile );
     fputs( "};\n\n", OutputHelpIDFile );
     fclose( OutputHelpIDFile );
   } /* endif */

// --------------------------------------------------------

   if ( fOK )
   {
      if (  fError )
        printf("Errors in file %s. Normal termination of program EQFMSG\n", argv[0] );
      else if (  fWarning )
        printf("Warnings in file %s. Normal termination of program EQFMSG\n", argv[0] );
      else
        printf("Normal termination of program EQFMSG\n" );
   }
   else
   {
      printf("EQFMSG ended due to previously reported error\n" );
   } /* endif */
   return rc;
}

VOID HandleNoText()
 {
  if ( !fText )
  {
     fputs( "\"!! No message text defined !!", OutputMsgFile );
     printf( "%s(%d): Warning: No message text defined.\n", pszInFile, usLine );
     fWarning = TRUE;
  } /* endif */
  
  if ( !fHelpTitle )
  {
    if ( !fFree )
    {
      fprintf( OutputHelpFile, szDefaultMessageHelpTitle );
    } /* endif */
  } /* endif */
  if ( !fHelp )
  {
     if ( !fFree )
     {
       fprintf( OutputHelpFile, ":p." );
       fprintf( OutputHelpFile, NO_HELP_TEXT );
       fprintf( OutputHelpFile, "\n" );
       fprintf( OutputHelpFile, ":p.%s\n", szTextLine1 );
     } /* endif */
     printf( "%s(%d): Warning: No help text defined.\n", pszInFile, usLine );
     fWarning = TRUE;
  } /* endif */
 } /* end of function HandleNoText */

// write given text to the message file (string table resource) and do any
// required pre-processing
VOID WriteMessageText( FILE *OutputMsgFile, char *pszInText )
{
  char *pszOutText = TempBuf;
  while( *pszInText != '\0' )
  {
    if ( *pszInText == '\"' )
	{
	  // duplicate double quotes
	  *pszOutText++ = *pszInText++;
	  *pszOutText++ = '\"' ;
	}
	else if ( (*pszInText == '\\') && (pszInText[1] == 'n') )
	{
	  // convert to stringtable linebreak \012
	  *pszOutText++ = *pszInText++;
	  pszInText++;
	  *pszOutText++ = '0';
	  *pszOutText++ = '1';
	  *pszOutText++ = '2';
	}
	else
    {
	  // no change required, copy as-is
	  *pszOutText++ = *pszInText++;
	}
  } /* endwhile */  
  *pszOutText = '\0';
  
  // write text to output file
  fputs( TempBuf, OutputMsgFile );
}

