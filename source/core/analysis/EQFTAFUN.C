//+----------------------------------------------------------------------------+
//|EQFTAFUN.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:   G. Queck                                                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: This module contains the common functions used for text        |
//|             analysis.                                                      |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|   TAGiveCntrl                                                              |
//|   TAReadBlock                                                              |
//|   UtlBufWrite                                                              |
//|   UtlBufOutOpen                                                            |
//|   UtlBufOutCLose                                                           |
//|   SetSegDate                                                               |
//|   DelSegFiles                                                              |
//|   WriteSegment                                                             |
//|   TARestartList                                                            |
//|   TALockFiles                                                              |
//|   TAUnlockFiles                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//|   UtlDispatch                                                              |
//|   UtlRead                                                                  |
//|   UtlError                                                                 |
//|   UtlWrite                                                                 |
//|   UtlAlloc                                                                 |
//|   UtlOpen                                                                  |
//|   UtlClose                                                                 |
//|   UtlGetFnameFromPath                                                      |
//|   OpenProperties                                                           |
//|   MakePropPtrFromHnd                                                       |
//|   SetPropAccess                                                            |
//|   SavePropertiesResetPropAccess                                            |
//|   CloseProperties                                                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|           UtlBufOutFlush                                                   |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#include <eqf.h>                  // General Translation Manager include file

//#include "OtmProposal.h"

#include "eqftai.h"               // Private include file for Text Analysis
#include "eqftmtag.h"             // TM SGML tag definitions
#include "eqfserno.h"             // TM driver level

#define TM_MATCH    3  // an exact match is found and added to document

#ifdef _DEBUG
  #define SESSIONLOG
#endif

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    TAGiveContrl                                              |
//+----------------------------------------------------------------------------+
//|Function call:    TAGiveContrl(hwnd, pInd)                                  |
//+----------------------------------------------------------------------------+
//|Description:      Dispatching messages from the PM Queue; check for         |
//|                  termination requests from Application system menu         |
//|                  (window memory is freed). In case of termination request  |
//|                  set the terminationflag.                                  |
//+----------------------------------------------------------------------------+
//|Parameters:       HWND         hwnd   window handle                         |
//|                  PTAINSTDATA  pInd   pointer to instance data              |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:      TRUE if no close request                                  |
//|                  FALSE else                                                |
//+----------------------------------------------------------------------------+
//|Side effects:     flag to end the process might be set                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                  BEGIN                                                     |
//|                     CALL UtlDispatch                                       |
//|                     IF parent not still exists THEN                        |
//|                        set terminationflag                                 |
//|                     ENDIF                                                  |
//|                     return (NOTterminationflag)                            |
//|                  END                                                       |
//+----------------------------------------------------------------------------+

BOOL TAGiveCntrl (HWND hwnd, PTAINSTDATA pInD)
{
   BOOL fOK = TRUE;

   if ( hwnd != HWND_FUNCIF )
   {
     UtlDispatch();   // should be called only once, but as often as possible

     /*----------- requests to terminate ---------*/
     if (!ACCESSWNDIDA( hwnd, PTAINSTDATA))  // parent request
        {
        pInD->fTerminate = TRUE;
        }

     fOK = pInD->fTerminate ? FALSE : TRUE;
   } /* endif */

   return ( fOK );
} /* end of TAGiveCntrl */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    TAReadBlock                                               |
//+----------------------------------------------------------------------------+
//|Function call:    TAReadBlock(pTAInput)                                     |
//+----------------------------------------------------------------------------+
//|Description:      Move remaining data in front of the buffer, fill the      |
//|                  buffer with more data from the file;                      |
//|                  if end of data, set the flag fAll.                        |
//+----------------------------------------------------------------------------+
//|Parameters:       PTAINPUT  pTAInput  contains the variables and file handle|
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:      TRUE if no error occured                                  |
//|                  else FALSE                                                |
//+----------------------------------------------------------------------------+
//|Side effects:     flag to end the process might be set                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                  BEGIN                                                     |
//|                     move the data that has not been segmented              |
//|                           in front of the input buffer                     |
//|                     IF the text is not yet complete THEN                   |
//|                        read more data from file into buffer                |
//|                        IF read failed THEN                                 |
//|                           fOK = FALSE                                      |
//|                           display error message                            |
//|                           set fKill                                        |
//|                        ELSE                                                |
//|                           IF end of input reached THEN                     |
//|                              set fAll                                      |
//|                           ENDIF                                            |
//|                        ENDIF                                               |
//|                     ENDIF                                                  |
//|                  END                                                       |
//+----------------------------------------------------------------------------+

BOOL TAReadBlock (PTAINPUT pTAInput)
   {
   BOOL fOK= TRUE;                    // processing flag
   PTAINSTDATA pInD;                  // pointer to instance data
   USHORT usReturn;
   ULONG  ulFreeBufferCharW;        // free buffer size for data from file
   ULONG  ulReadCharW = 0;          // charWs read from input file
   ULONG  ulBytesRead;              // bytes from input file
   CHAR_W c;                       // work character
   USHORT usI;                     // loop index


   pInD = pTAInput -> pInD ;  // store pointer to instance data temp.
   if (pInD->fFirstRead )
   {
     PSZ pszPrefix = UNICODEFILEPREFIX;
     int iLen = strlen(pszPrefix);
     ULONG   ulBytesRead;
     CHAR    ch[10];
     PSZ     pData = &ch[0];
     ULONG ulTemp;

     // assure that if file starts with Unicode FILE prefix, it is
     // recognized as a Unicode file
     // currently in HTML, Preseg already writes an UTF16 file if
     // a meta tag with charset info is detected and interpreted
     pInD->usCharSetInFile = pInD->pLoadedTable->usCharacterSet;
     UtlReadL( pInD->hSource, pData, 8, &ulBytesRead, FALSE );
     if ( memcmp( pData, pszPrefix, iLen ) == 0 )
     {
       pInD->usCharSetInFile = TAGCHARSET_UNICODE;
     }
     else
     {
       // if we are dealing with an TAGCHARSET_ANSI file, we know,
       // that all the old userexits have converted the file into
       // ASCII during presegmentation.
       // NEW: 020918: TAGCHARSET_ANSI is always active!!
       // independent of whether a user exit exists or not
       // If user exit exists and makes the conversion, set charset ASCII in markup table
       //old:if ((pInD->usCharSetInFile == TAGCHARSET_ANSI) &&
       //old:    (pInD->pfnEQFPreSeg || pInD->pfnEQFPreSeg2) )
       //
       //{
       //  pInD->usCharSetInFile = TAGCHARSET_ASCII;
       //}
     }
     // position back to start of file
     UtlChgFilePtr( pInD->hSource, 0L, FILE_BEGIN, &ulTemp, FALSE );
   }
   if (pInD->pRest)  // prepare buffer, copy rest to start
   {
      pInD->uCurTextPos = (USHORT)(pInD->ulFilled - (pInD->pRest - pInD->szTextBuffer));
      memmove( (PBYTE)pInD->szTextBuffer,
               (PBYTE)pInD->pRest,
               pInD->uCurTextPos * sizeof(CHAR_W));
      pInD->szTextBuffer[pInD->uCurTextPos] = EOS;
   }
   else   // buffer empty or completely tokenized
   {
      pInD->uCurTextPos = 0;
   }

   // calculate free length for data in char_w's
   ulFreeBufferCharW = BUFFERSIZE - pInD->uCurTextPos - 1;

   if (!pInD->fAll)   /* the input text is not yet complete       */
   {               /* read more data from file into buffer     */

     switch (pInD->usCharSetInFile  )
     {
       case TAGCHARSET_ANSI:
         {
		   PSZ_W pBufferW = NULL;
           usReturn = UtlReadL( pInD->hSource, pInD->szASCIIBuffer ,
                               ulFreeBufferCharW, &ulBytesRead, FALSE );
           if (usReturn == NO_ERROR)
           {
			   if (ulBytesRead != 0)
			   {
				 LONG  lBytesLeft = 0;
				 LONG  lRc = 0;
				 pInD->szASCIIBuffer[ulBytesRead] = EOS;
				 pBufferW = &(pInD->szTextBuffer[pInD->uCurTextPos] );
				 ulReadCharW = UtlDirectAnsi2UnicodeBuf((PSZ)pInD->szASCIIBuffer,
				                       (PSZ_W)pBufferW,
				                       ulBytesRead,
				                       pInD->TolstControl.ulAnsiCP,
				                       TRUE, &lRc, &lBytesLeft);
                 usReturn = (USHORT)lRc;
                 if (lBytesLeft)
                 {
					 ULONG ulCurrentPos = 0;
					 UtlChgFilePtr( pInD->hSource, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
					 ulCurrentPos -= lBytesLeft;
					 UtlChgFilePtr( pInD->hSource, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);
					 ulBytesRead       -= lBytesLeft;
					 ulFreeBufferCharW -= lBytesLeft;
					 *(pInD->szASCIIBuffer + ulBytesRead) = '\0';
			     } /* endif */
			   } /* endif */
		   } /* endif */
		   if (ulBytesRead != ulFreeBufferCharW)
		   {
			  pInD->fAll = TRUE;   // fAll is the EOF indicator
	       } /* endif */
         }
         break;

       case TAGCHARSET_UNICODE:
         {
             ULONG  ulUnicodeBytesRead;
             PSZ    pData = (PSZ)&(pInD->szTextBuffer[pInD->uCurTextPos]);
             ULONG  ulFreeBufferBytes = ulFreeBufferCharW * sizeof(CHAR_W);
             // read data into conversion buffer
             usReturn = UtlReadL( pInD->hSource, &(pInD->szTextBuffer[pInD->uCurTextPos]),
                                 ulFreeBufferBytes, &ulUnicodeBytesRead, FALSE );
             if ( !usReturn )
             {
               pInD->fAll = (ulFreeBufferBytes != ulUnicodeBytesRead);
             }
             // if first read skip any unicode text prefix
             if ( (usReturn == NO_ERROR) && pInD->fFirstRead )
             {
               PSZ pszPrefix = UNICODEFILEPREFIX;
               int iLen = strlen(pszPrefix);
               if ( memcmp( pData, pszPrefix, iLen ) == 0 )
               {
                 // skip prefix ...
                 pData += iLen;
                 ulUnicodeBytesRead -= iLen;
                 memmove( &(pInD->szTextBuffer[pInD->uCurTextPos]), pData, ulUnicodeBytesRead );
               } /* endif */
               pInD->fFirstRead = FALSE;
             } /* endif */

             ulReadCharW = ulUnicodeBytesRead / sizeof(CHAR_W);
         }
         break;

       case TAGCHARSET_UTF8 :
         {

           usReturn = UtlReadL( pInD->hSource, pInD->szASCIIBuffer,  ulFreeBufferCharW, &ulBytesRead, FALSE );

            // if first read skip any unicode text prefix
            if ( (usReturn == NO_ERROR) && pInD->fFirstRead )
            {
              PSZ pszPrefix = UTF8FILEPREFIX;
              int iLen = strlen(pszPrefix);
              if ( memcmp( pInD->szASCIIBuffer, pszPrefix, iLen ) == 0 )
              {
                // skip prefix ...
                ulFreeBufferCharW -= iLen;
                ulBytesRead -= iLen;
                memmove( pInD->szASCIIBuffer, pInD->szASCIIBuffer + iLen, ulBytesRead );
              } /* endif */
              pInD->fFirstRead = FALSE;
            } /* endif */

           if ( !usReturn )
           {
             int iChars, iRC = 0;                 
             int iTry = 5;


             PSZ_W pBufferW;
             pInD->szASCIIBuffer[ulBytesRead] = EOS;
             if (ulBytesRead != ulFreeBufferCharW )    // end of input reached
             {
               pInD->fAll = TRUE;  // fAll is the EOF indicator
             } /* endf */

             pBufferW = &(pInD->szTextBuffer[pInD->uCurTextPos] );

             do
             {
                iChars = MultiByteToWideChar( CP_UTF8, 0, pInD->szASCIIBuffer, ulBytesRead, pBufferW, ulFreeBufferCharW );
                if ( iChars == 0 )
                {
                  iRC = GetLastError();
                } /* endif */

                if ( (iRC == ERROR_NO_UNICODE_TRANSLATION) && !pInD->fAll  )
                {
                  // undo last character and retry
                  ULONG ulCurrentPos = 0;
                  UtlChgFilePtr( pInD->hSource, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
                  ulCurrentPos--;
                  UtlChgFilePtr( pInD->hSource, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);
                  ulBytesRead--;
                  iTry--;
                } /* endif */
             } while ( (iRC == ERROR_NO_UNICODE_TRANSLATION) && (iTry > 0) && ulBytesRead && !pInD->fAll );
             ulReadCharW = (ULONG)iChars;
           } /* endif */
         }
         break;

       case TAGCHARSET_ASCII:
       default:
         {

           usReturn = UtlReadL( pInD->hSource, pInD->szASCIIBuffer ,
                               ulFreeBufferCharW, &ulBytesRead, FALSE );
           if ( !usReturn )
           {
             PSZ_W pBufferW;
             pInD->szASCIIBuffer[ulBytesRead] = EOS;
             if (ulBytesRead != ulFreeBufferCharW )    // end of input reached
             {
               pInD->fAll = TRUE;  // fAll is the EOF indicator
             } /* endf */

             pBufferW = &(pInD->szTextBuffer[pInD->uCurTextPos] );

             // ensure, that always complete DBCS characters are in the buffer...
             if (!pInD->fAll && ulBytesRead &&
                  IsDBCSLeadByteEx( pInD->TolstControl.ulOemCP, (BYTE)pInD->szASCIIBuffer[ulBytesRead-1]) )
             {
                 int iTry = 5;

                 while (  iTry > 0  && ulBytesRead)
                 {
                      // undo the last character read...
                      // reposition file pointer
                     ULONG ulCurrentPos = 0;
                     UtlChgFilePtr( pInD->hSource, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
                     ulCurrentPos--;
                     UtlChgFilePtr( pInD->hSource, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);

                     // adjust counters
                     ulBytesRead--;
                     // return is number of wide chars written to pszBuffer
                     ulReadCharW =ASCII2UnicodeBuf( pInD->szASCIIBuffer, pBufferW,
                                                    ulBytesRead, pInD->TolstControl.ulOemCP  );

                     if ( ulReadCharW && pBufferW[ulReadCharW-1]   == 0)
                     {
                          // try again, we probably found a 2nd DBCSbyte which might be in the range of a 1st byte, too
                          iTry--;
                     }
                     else
                     {
                          // leave loop
                          iTry = 0;
                      } /* endif */
                  } /* endwhile */
              }
              else
              {
                    ulReadCharW =ASCII2UnicodeBuf( pInD->szASCIIBuffer, pBufferW,
                                                   ulBytesRead, pInD->TolstControl.ulOemCP  );
              } /* endif */
           }
         }
         break;
     } /* endswitch */

      if (usReturn)   // read failed - display error message
      {
         PSZ pszDoc;
         fOK = FALSE; // error happened during read of file
         // ask user whether to continue
         pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
#ifdef SESSIONLOG
         UtlLogWriteString( "TAReadBlock: Error reading document %s", pszDoc );
#endif
         usReturn = UtlError ( ERROR_TA_SOURCEFILE,
                               MB_YESNO,
                               1,
                               &pszDoc,
                               EQF_QUERY );

         pTAInput->fKill  = (usReturn != MBID_YES);  //  stop
      }
      else
      {
        if ( pInD->fAll)    // end of input reached
       {
           c = pInD->szTextBuffer[pInD->uCurTextPos + ulReadCharW - 1];
           if ( c == 26  )  // EOF character
           {
              if ( ulReadCharW != 0 )
              {
                ulReadCharW -=1;  // discard EOF at end of file
              }
              else if ( pInD->uCurTextPos != 0 )
              {
                pInD->uCurTextPos -=1;  // discard EOF at end of file
              } /* endif */
            }
         }
        /**************************************************************/
        /* change EOS characters within text block to BLANK           */
        /**************************************************************/
        for ( usI = pInD->uCurTextPos;
              usI < pInD->uCurTextPos+ulReadCharW;
              usI++ )
        {
          if ( pInD->szTextBuffer[usI] == EOS )
          {
            pInD->szTextBuffer[usI] = BLANK;
          } /* endif */
        } /* endfor */

        // append string terminator at end of textblock
        pInD->szTextBuffer[pInD->uCurTextPos + ulReadCharW] = EOS;
        pInD->ulFilled = pInD->uCurTextPos + ulReadCharW;
      } /* endif */
   } /* endif */
   pInD->fFirstRead = FALSE;

   return (fOK);
 } /* end of TAReadBlock */


// count translatable and not translated segments in a STARGET document
static USHORT TACountTranslatableSegments
(
  PSZ    pszFolObjName,                // folder object name
  PSZ    pszDocShortName,              // document short name
  PULONG pulTranslatableSegments,      // ptr to buffer receiving number of translatable segments
  PULONG pulTranslatedSegments,        // ptr to buffer receiving number of translated segments
  PULONG pulNotTranslatedSegments      // ptr to buffer receiving number of not translatable segments
)
{
   USHORT      usRC = NO_ERROR;
   PTBDOCUMENT pDoc = NULL;
   ULONG       ulSegNum = 1;
   PTBSEGMENT  pSeg = NULL;
   CHAR   szTargetDoc[MAX_LONGFILESPEC];

   // setup name of segmented target document
   UtlMakeEQFPath( szTargetDoc, pszFolObjName[0], DIRSEGTARGETDOC_PATH, UtlGetFnameFromPath( pszFolObjName ) );
   strcat( szTargetDoc, "\\" );
   strcat( szTargetDoc, pszDocShortName );

   // return ASAP if there there is no segmented target document
   if ( !UtlFileExist( szTargetDoc ) )
   {
     return( 0 );
   } /* endif */

   // initalize result fields
   if ( pulTranslatableSegments != NULL ) *pulTranslatableSegments = 0;
   if ( pulTranslatedSegments != NULL ) *pulTranslatedSegments = 0;
   if ( pulNotTranslatedSegments != NULL ) *pulNotTranslatedSegments = 0;

   // allocate buffer areas
   if ( !usRC ) if ( !UtlAlloc( (PVOID *)&pDoc, 0L, (LONG) sizeof(TBDOCUMENT), ERROR_STORAGE ) )usRC = ERROR_NOT_ENOUGH_MEMORY;
   if ( !usRC ) usRC = TALoadTagTable( DEFAULT_QFTAG_TABLE, (PLOADEDTABLE *)&pDoc->pQFTagTable, TRUE, TRUE );

   // load segmented file
   if ( !usRC )
   {
     usRC = EQFBFileReadExW( szTargetDoc, pDoc, 0L );
   } /* endif */

   while ( !usRC && (ulSegNum < pDoc->ulMaxSeg) )
   {
     pSeg = EQFBGetSegW( pDoc, ulSegNum );
     if ( pSeg && !pSeg->SegFlags.Joined )
     {
       if ( pSeg->qStatus != QF_NOP )
       {
         if ( pulTranslatableSegments != NULL ) *pulTranslatableSegments += 1;
         if ( pSeg->qStatus != QF_XLATED )
         {
           if ( pulNotTranslatedSegments != NULL ) *pulNotTranslatedSegments += 1;
         }
         else
         {
           if ( pulTranslatedSegments != NULL ) *pulTranslatedSegments += 1;
         } /* endif not translated segment */
       } /* endif translatable segment */
     } /* endif */
     ulSegNum++;
   } /* endwhile */

   // cleanup
   TAFreeDoc((PVOID *) &pDoc);

   return( usRC == NO_ERROR );
} /* end of function TACountTranslatableSegments */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    SetSegDate                                                |
//+----------------------------------------------------------------------------+
//|Function call:    SetSegDate(pTAInput, ulSegDate)                           |
//+----------------------------------------------------------------------------+
//|Description:      Set the date of analysis, reset or set the date of        |
//|                  translation depending on status.                          |
//+----------------------------------------------------------------------------+
//|Parameters:       PTAINPUT  pTAInput     pointer to input data store        |
//|                  ULONG     ulSegDate    segmentation date                  |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:      TRUE if operation has been successful                     |
//|                  else FALSE                                                |
//+----------------------------------------------------------------------------+
//|Side effects:     flag to end the process might be set                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                  BEGIN                                                     |
//|                     update properties                                      |
//|                     set fOK accordingly                                    |
//|                     IF fOK THEN                                            |
//|                        set document touched stamp in document properties   |
//|                            with elapsed time                               |
//|                        IF document is complete translated THEN             |
//|                           set translation date                             |
//|                        ELSE                                                |
//|                           reset translation date                           |
//|                        ENDIF                                               |
//|                        save document properties                            |
//|                        set fOK accordingly                                 |
//|                     ENDIF                                                  |
//|                     IF (NOT fOK) THEN                                      |
//|                        display error message                               |
//|                        set terminationflag                                 |
//|                     ENDIF                                                  |
//|                     close instance properties in any case                  |
//|                     return fOK                                             |
//|                  END                                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+

BOOL SetSegDate
(
   PTAINPUT  pTAInput,                // pointer to input data area
   ULONG ulSegDate                    // segmentation date
)
{
   PTAINSTDATA     pInD;
   HPROP           hPropDocument;            // handle to document properties
   PPROPDOCUMENT   pPropDocument = NULL;     // pointer to document properties
   ULONG           ulErrorInfo;              // error indicator from PRHA
   BOOL            fOK = TRUE;

   // store pointer to instance data temp.
   pInD = pTAInput->pInD ;

   // update properties
   if( (hPropDocument = OpenProperties( pInD->pszCurSourceFile,
                                         pTAInput->szFolder,
                                         PROP_ACCESS_READ,
                                         &ulErrorInfo)) == NULL)
   {
      fOK = FALSE;
   } /* endif */

   if (fOK)
   {
      pPropDocument = (PPROPDOCUMENT) MakePropPtrFromHnd( hPropDocument );
      fOK = SetPropAccess( hPropDocument, PROP_ACCESS_WRITE);
   } /* endif */

   if (fOK)
   {
      //set document touched stamp in document properties with elapsed time
      pPropDocument->ulTouched = 0L;
      pPropDocument->ulXLated = 0L;
      pPropDocument->ulSeg = ulSegDate;
      pPropDocument->usCopied   = (USHORT)pInD->ulSegsReplaced;
      pPropDocument->usModified = 0;
      pPropDocument->usScratch  = 0;

      // if doc is complete translated, set translation date else reset it
      if ( pTAInput->fInsertTMMatches )
      {
         // set touched date if something inserted
         if ( pInD->fTMInserted )
         {
            pPropDocument->ulTouched = ulSegDate;
         } /* endif */

         // set translated date if completly translated
         if ( !pInD->fNotComplete )
         {
            pPropDocument->ulXLated = ulSegDate;
         } /* endif */

      } /* endif */

      // store match count results in document properties
      pPropDocument->ulNoProps    = pInD->ulNoProps;
      pPropDocument->ulFuzzy      = pInD->ulFuzzy;
      pPropDocument->ulExactExact = pInD->ulExactExact;
      pPropDocument->ulExactOne   = pInD->ulExactOne;
      pPropDocument->ulExactMore  = pInD->ulExactMore;
      pPropDocument->ulTotal      = pInD->ulTotal;
      pPropDocument->ulRepl       = pInD->ulRepl;
      pPropDocument->ulFuzzyRepl  = pInD->ulFuzzyRepl;
      pPropDocument->ulSegNoProps    = pInD->ulSegNoProps;
      pPropDocument->ulSegFuzzy      = pInD->ulSegFuzzy;
      pPropDocument->ulSegExactExact = pInD->ulSegExactExact;
      pPropDocument->ulSegExactOne   = pInD->ulSegExactOne;
      pPropDocument->ulSegExactMore  = pInD->ulSegExactMore;
      pPropDocument->ulSegTotal      = pInD->ulSegTotal;
      pPropDocument->ulSegRepl       = pInD->ulSegRepl;
      pPropDocument->ulSegFuzzyRepl  = pInD->ulSegFuzzyRepl;
      pPropDocument->ulSegMachineMatch = pInD->ulSegMachineMatch;
      pPropDocument->ulMachineMatch = pInD->ulMachineMatch;

      memcpy( &pPropDocument->Total,      &pInD->Total,    sizeof(COUNTSUMS) );
      memcpy( &pPropDocument->ExactExact, &pInD->ExactExact, sizeof(COUNTSUMS) );
      memcpy( &pPropDocument->ExactOne,   &pInD->Exact,    sizeof(COUNTSUMS) );
      memcpy( &pPropDocument->Fuzzy1,     &pInD->Fuzzy1,   sizeof(COUNTSUMS) );
      memcpy( &pPropDocument->Fuzzy2,     &pInD->Fuzzy2,   sizeof(COUNTSUMS) );
      memcpy( &pPropDocument->Fuzzy3,     &pInD->Fuzzy3,   sizeof(COUNTSUMS) );
      memcpy( &pPropDocument->NoProps,    &pInD->NoProps,  sizeof(COUNTSUMS) );
      memcpy( &pPropDocument->MTProps,    &pInD->MTProps,  sizeof(COUNTSUMS) );
      memcpy( &pPropDocument->Repl,       &pInD->Repl,     sizeof(COUNTSUMS) );
      memcpy( &pPropDocument->ExactMore,  &pInD->ExactMore, sizeof(COUNTSUMS) );

      // fill fuzzy level fields
      pPropDocument->lLargeFuzzLevel  = (LONG)UtlQueryULong( QL_LARGELOOKUPFUZZLEVEL );
      pPropDocument->lMediumFuzzLevel = (LONG)UtlQueryULong( QL_MEDIUMLOOKUPFUZZLEVEL );
      pPropDocument->lSmallFuzzLevel  = (LONG)UtlQueryULong( QL_SMALLLOOKUPFUZZLEVEL );

      // GQ 2016/10/28: count number of translatable segments and not-translated segments when these values have not been set already
      if ( (ulSegDate != 0) && ((pInD->ulTotalSegs == 0) || (pPropDocument->ulNotTranslated == 0)) )
      {
        ULONG ulTranslatableSegs = 0;
        TACountTranslatableSegments( pTAInput->szFolder, pInD->pszCurSourceFile, &ulTranslatableSegs, &(pInD->ulSegsReplaced), &(pInD->ulSegsNotReplaced) );
      } /* endif */


      /************************************************************** 17@20A  */
      /* evaluate translation complete rate                           */
      /****************************************************************/
      if ( pInD->ulTotalSegs == 0L )                            /* 1@KIT1270C */
      {
        /**************************************************************/
        /* No segments processed by text analysis itself ==>          */
        /*    no completian rate can be supplied                      */
        /**************************************************************/
        pPropDocument->usComplete = 0;
        pPropDocument->ulNotTranslated = pInD->ulTotal;
      }
//    else if ( (pTAInput->fInsertTMMatches ) &&                /* 4-13-15 */
      else if ( (pInD->ulSegsReplaced == 0L) &&                 /* 5@KIT1270A */
                (pInD->ulSegsNotReplaced == 0L))
      {
        pPropDocument->usComplete = 100;
        pPropDocument->ulNotTranslated = 0;
      }
      else if ( pInD->ulSegsReplaced == 0L )
      {
        pPropDocument->usComplete = 0;
        pPropDocument->ulNotTranslated = pInD->ulTotal;
      }
      else if ( pInD->ulSegsNotReplaced == 0L )
      {
        pPropDocument->usComplete = 100;
        pPropDocument->ulNotTranslated = 0L;
      }
      else
      {
        pPropDocument->usComplete = (USHORT)
                    ( (pInD->ulSegsReplaced * 100L) /
                      (pInD->ulSegsReplaced + pInD->ulSegsNotReplaced ) );
        /******************************************************************/
        /* check that we set compl.rate to 1% if transl. started work.    */
        /******************************************************************/
        if ( !pPropDocument->usComplete && pInD->ulSegsReplaced )
        {
          pPropDocument->usComplete = 1;
        } /* endif */
        pPropDocument->ulNotTranslated = pInD->ulTotal - pInD->ulReplacedWords;
      } /* endif */

      // set version of TM used for the analysis
      sprintf( pPropDocument->szAnalysisTMVersion, "%d.%d.%d.%d", EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE, EQF_DRIVER_SUBRELEASE, EQF_DRIVER_BUILD );

      //save document properties
      fOK = ! SaveProperties( hPropDocument, &ulErrorInfo );

      //reset write access to document properties
      ResetPropAccess( hPropDocument, PROP_ACCESS_WRITE);
   } /* endif */


   if ( !fOK )
   {
      if ( ulErrorInfo != Err_NoStorage )
      {
         PSZ pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
         UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1,
                   &pszDoc,
                   EQF_ERROR);
      } /* endif */
      pInD->usError = TAERROR;
      pInD->fTerminate = TRUE;                  // request stop proc.
   } /* endif */


   // close instance(document) properties  in any case
   if ( hPropDocument )
   {
      if ( CloseProperties( hPropDocument, PROP_FILE, &ulErrorInfo) )
      {
         pInD->fTerminate = TRUE;                  // request stop proc.
      } /* endif */
   } /* endif */
   return ( fOK );
} /* end of SetSegDate */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    DelSegFiles                                               |
//+----------------------------------------------------------------------------+
//|Function call:    DelSegFiles(pTAInput)                                     |
//+----------------------------------------------------------------------------+
//|Description:      Close and delete segmented source and target file,        |
//|                  close source file.                                        |
//+----------------------------------------------------------------------------+
//|Parameters:       PTAINPUT  pTAInput    pointer to input data store         |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                  BEGIN                                                     |
//|                     close source file                                      |
//|                     IF path to segmented source is set THEN                |
//|                        close and delete segmented source                   |
//|                        and target file                                     |
//|                     ENDIF                                                  |
//|                  END                                                       |
//+----------------------------------------------------------------------------+

VOID DelSegFiles(PTAINPUT pTAInput)
{
   CHAR   szFileName[MAX_EQF_PATH];       // space for file entries
   PTAINSTDATA   pInD;                 // pointer to instance data for TA

  pInD = pTAInput->pInD;

  TACLOSE ( pInD->hSource );               // close sourcefile

  if (*(pTAInput->szSEGSOURCE_Path) )
  {
    // close and delete segmented source file
    if ( pInD->CBSegSource )
    {
      UtlBufClose( pInD->CBSegSource, FALSE );
    } /* endif */
    TADELETE( szFileName,
              pTAInput->szSEGSOURCE_Path,pInD->pszCurSourceFile);

    // close and delete segmented target
    if ( pInD->CBSegTarget )
    {
      UtlBufClose( pInD->CBSegTarget, FALSE );
    } /* endif */
    TADELETE( szFileName,
              pTAInput->szSEGTARGET_Path,pInD->pszCurSourceFile);

    /******************************************************************/
    /* Delete any temporary file                                      */
    /******************************************************************/
    // if source file different from temp file delete temp file,
    // otherwise user might get problems ....
    sprintf( szFileName,                     // create full file name
             PATHCATFILE,
             pTAInput->szSOURCE_Path,
             pInD->pszCurSourceFile);
    if ( strcmpi(pTAInput->szTempName, szFileName) )
    {
       UtlDelete(pTAInput->szTempName, 0L, FALSE); //delete temp file
    } /* endif */


  } /* endif */

} /* end of DelSegFiles */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    WriteSegment                                              |
//+----------------------------------------------------------------------------+
//|Function call:    WriteSegment(pTAInput, usSegLength, pszSegment,           |
//|                               XlateState, pBuffer, usSegNum, fFoundTM)     |
//+----------------------------------------------------------------------------+
//|Description:      Fill buffer with segment start tag, segment and segment   |
//|                  end tag.                                                  |
//+----------------------------------------------------------------------------+
//|Parameters:       PTAINPUT    pTAInput        pointer to input data area    |
//|                  USHORT      usSegLength     length of data                |
//|                  PSZ         pszSegment      data to output                |
//|                  XLATESTATE  XlateState      translation state of the      |
//|                                              segment                       |
//|                  PBUFCB    pBuffer         pointer to I/O control block  |
//|                  USHORT      usSegNum        logical segment number        |
//|                  BOOL        fFoundTM        exact match was found and     |
//|                                              written into document         |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:      TRUE if the operation has been succesful                  |
//|                  else FALSE                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                  BEGIN                                                     |
//|                     fill buffers with source segment                       |
//|                     set fOK accordingly                                    |
//|                     return fOK                                             |
//|                  END                                                       |
//+----------------------------------------------------------------------------+

BOOL WriteSegmentW( PTAINPUT       pTAInput,
                   USHORT         usSegLength,
                   PSZ_W          pszSegment,
                   XLATESTATE     XlateState,
                   PBUFCB         pBuffer,
                   ULONG          ulSegNum,
                   BOOL           fFoundTM)
{
   BOOL fOK= TRUE;                     // processing flag
   ULONG ulLength;                    // length of current segment
   PTAINSTDATA pInD;                   // pointer to instance data

   pInD = pTAInput->pInD;

   if (!fFoundTM)
   {
      // fill buffer with segment start
      ulLength = swprintf( pInD->szSGMLSegBuffer, pTAInput->TATagW[XlateState].chSTag,
                          ulSegNum);
   }
   else
   {
      // fill buffer with segment start and flag for already translated
      ulLength = swprintf( pInD->szSGMLSegBuffer, pTAInput->TATagW[XlateState].chSTag,
                          ulSegNum, TM_MATCH);
   } /* endif */

   memcpy( (PBYTE)(pInD->szSGMLSegBuffer + ulLength), (PBYTE)pszSegment,
            usSegLength * sizeof(CHAR_W) );
   ulLength += usSegLength;

   ulLength += swprintf( pInD->szSGMLSegBuffer + ulLength,
                        pTAInput->TATagW[XlateState].chETag, pInD->ulSegNum);

   // write unicode prefix to output file if at begin of file
   if ( ulSegNum == 1 )
   {
     fOK = (UtlBufWrite( pBuffer, UNICODEFILEPREFIX, (USHORT)strlen(UNICODEFILEPREFIX), TRUE ) == NO_ERROR );
   } /* endif */

   if ( fOK )
   {
     fOK = (UtlBufWrite( pBuffer, (PSZ)pInD->szSGMLSegBuffer,
                         (ulLength * sizeof(CHAR_W)), TRUE ) == NO_ERROR );
   } /* endif */

   pInD->fFirstWrite = FALSE;

   return (fOK);
} /* end of WriteSegment */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    WriteTMSegment                                            |
//+----------------------------------------------------------------------------+
//|Function call:    WriteTMSegment(pTAInput, usSegLength, pszSegment,         |
//|                               XlateState, pBuffer, usSegNum, fFoundTM)     |
//+----------------------------------------------------------------------------+
//|Description:      Fill buffer with segment data in Translation Memory       |
//|                  export format.                                            |
//+----------------------------------------------------------------------------+
//|Parameters:       PTAINPUT    pTAInput        pointer to input data area    |
//|                  USHORT      usSegLength     length of data                |
//|                  PSZ         pszSegment      data to output                |
//|                  XLATESTATE  XlateState      translation state of the      |
//|                                              segment                       |
//|                  PBUFCB    pBuffer         pointer to I/O control block  |
//|                  USHORT      usSegNum        logical segment number        |
//|                  BOOL        fFoundTM        exact match was found and     |
//|                                              written into document         |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:      TRUE if the operation has been succesful                  |
//|                  else FALSE                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                  BEGIN                                                     |
//|                     fill buffers with source segment                       |
//|                     set fOK accordingly                                    |
//|                     return fOK                                             |
//|                  END                                                       |
//+----------------------------------------------------------------------------+

BOOL WriteTMSegment
(
  PTAINPUT       pTAInput,
  ULONG          ulSegLength,
  PSZ_W          pszSegment,
  PBUFCB         pBuffer,
  ULONG          ulSegNum,
  SHORT          sFormat
)
{
   BOOL fOK= TRUE;                     // processing flag
   ULONG ulLength;                     // length of current segment
   ULONG ulControlLength;              // length of control data
   PTAINSTDATA pInD;                   // pointer to instance data
   PSZ         pSegData = NULL;
   PSZ_W       pszSegTempBuffer = NULL;
   ULONG       ulSegBufLen = 0;

   pInD = pTAInput->pInD ;  // store pointer to instance data temp.

   ulSegBufLen = 4 * MAX_SEGMENT_SIZE;
   fOK = UtlAlloc( (PVOID *) &pSegData, 0L, (LONG)ulSegBufLen, ERROR_STORAGE );
   if ( fOK ) fOK = UtlAlloc( (PVOID *) &pszSegTempBuffer, 0L, (LONG)(sizeof(CHAR_W)*(MAX_SEGMENT_SIZE+2)), ERROR_STORAGE );

   // prepare control data
   {
      PSZ pTemp = (stricmp( pTAInput->pInD->pszCurSourceFile, pTAInput->apszLongNames[pTAInput->pInD->usCurNumSourceFile-1]) == 0) ?
                  "": pTAInput->apszLongNames[pTAInput->pInD->usCurNumSourceFile-1];

      ulControlLength = sprintf( pTAInput->chControlInfo,
                        "%06lu%s%c%s%016lu%s%s%s%s%s%s%s%s%s%s%s%s",
                        //|    | | | |     | | | | | | | | | +----- file name
                        //|    | | | |     | | | | | | | | +------- X15
                        //|    | | | |     | | | | | | | +--------- tag table
                        //|    | | | |     | | | | | | +----------- X15
                        //|    | | | |     | | | | | +------------- author
                        //|    | | | |     | | | | +--------------- X15
                        //|    | | | |     | | | +----------------- target lng
                        //|    | | | |     | | +------------------- X15
                        //|    | | | |     | +--------------------- source lng
                        //|    | | | |     +----------------------- X15
                        //|    | | | +----------------------------- time
                        //|    | | +------------------------------- X15
                        //|    | +--------------------------------- mt flag
                        //|    +----------------------------------- X15
                        //+---------------------------------------- seg id
                        ulSegNum,                     // SegmentId
                        X15_STR,
                        '1',
                        X15_STR,
                        0L,                     // TargetTime
                        X15_STR,
                        pTAInput->pInD->szDocSourceLang,
                        X15_STR,
                        pTAInput->pInD->szDocTargetLang,
                        X15_STR,
                        "",                     // Author Name
                        X15_STR,
                        pTAInput->pInD->szDocFormat,
                        X15_STR,
                        pTAInput->pInD->pszCurSourceFile,
                        X15_STR,
                        pTemp );
   }


   // prepare segment data
#ifdef FORCE_CRLF_IN_MEMORY_OUTPUT
   if ( fOK )
   {
      PSZ_W pszSource = pszSegment;
      PSZ_W pszTarget = pszSegTempBuffer;

      while( (ulSegLength != 0) && *pszSource )
      {
        if ( *pszSource == L'\r' )
        {
          // ignore carriage return
          pszSource++;
          ulSegLength--;
        }
        else if ( *pszSource == L'\n' )
        {
          // convert to CRLF
          pszSource++;
          ulSegLength--;
          *pszTarget++ = L'\r';
          *pszTarget++ = L'\n';
        }
        else
        {
          // copy as-is
          *pszTarget++ = *pszSource++;
          ulSegLength--;
        } /* endif */
      } /* endwhile */
      *pszTarget = 0;
      ulSegLength = wcslen( pszSegTempBuffer );
   } /* endif */
#else
   if ( fOK )
   {
     wcscpy( pszSegTempBuffer, pszSegment );
   } /* endif */
#endif

   if ( fOK )
   {
     if ( sFormat == WRITETMSEGMENT_UTF16 )
     {
        CHAR_W chTag [ 80 ];                 // space to create tag

        // write segment start data                                        
        ulLength = swprintf( chTag, L"%s%10.10ld\r\n", MEM_SEGMENT_BEGIN_TAGW, ulSegNum );
        fOK = (UtlBufWriteW( pBuffer, chTag, ulLength*sizeof(CHAR_W) , TRUE ) == NO_ERROR );

        // Write control start tag
        if ( fOK )
        {
          fOK = (UtlBufWriteW( pBuffer, MEM_CONTROL_BEGIN_TAGW, UTF16strlenBYTE(MEM_CONTROL_BEGIN_TAGW) , TRUE ) == NO_ERROR );
        } /* endif */

        // Write control data                                             
        if ( fOK )
        {
          ASCII2Unicode( pTAInput->chControlInfo, pTAInput->chControlInfoW, 0 );
          fOK = (UtlBufWriteW( pBuffer, pTAInput->chControlInfoW, ulControlLength * sizeof(CHAR_W), TRUE ) == NO_ERROR );
        } /* endif */

        // Write control end tag and source start tag                      
        if ( fOK )
        {
          wcscpy( chTag, MEM_CONTROL_END_TAGW );
          wcscat( chTag, MEM_SOURCE_BEGIN_TAGW );
          ulLength = UTF16strlenBYTE( chTag );
          fOK = (UtlBufWriteW( pBuffer, chTag, ulLength, TRUE ) == NO_ERROR );
        } /* endif */

        // Write source segment                                            
        if ( fOK ) 
        {
          fOK = (UtlBufWriteW( pBuffer, pszSegTempBuffer, ulSegLength*sizeof(CHAR_W), TRUE ) == NO_ERROR );
        }

        // write end source segment tag, empty target segment data and end segment tag
        if ( fOK )
        {
          wcscpy( chTag, MEM_SOURCE_END_TAGW );
          wcscat( chTag, MEM_TARGET_BEGIN_TAGW );
          wcscat( chTag, MEM_TARGET_END_TAGW );
          wcscat( chTag, MEM_SEGMENT_END_TAGW );
          ulLength = UTF16strlenBYTE( chTag );
          fOK = (UtlBufWriteW( pBuffer, chTag, ulLength, TRUE ) == NO_ERROR );
        } /* endif */
     }
     else if ( sFormat == WRITETMSEGMENT_UTF8 )
     {
        CHAR chTag [ 80 ];                 // space to create tag

        // write segment start data                                        
        ulLength = sprintf( chTag, "%s%10.10ld\r\n", MEM_SEGMENT_BEGIN_TAG, ulSegNum );
        fOK = (UtlBufWrite( pBuffer, chTag, ulLength, TRUE ) == NO_ERROR );

        // Write control start tag
        if ( fOK )
        {
          fOK = (UtlBufWrite( pBuffer, MEM_CONTROL_BEGIN_TAG, strlen(MEM_CONTROL_BEGIN_TAG) , TRUE ) == NO_ERROR );
        } /* endif */

        // Write control data                                             
        if ( fOK )
        {
          LONG lRc = 0;
          ASCII2Unicode( pTAInput->chControlInfo, pTAInput->chControlInfoW, 0 );
	        ulLength = Unicode2UTF8BufEx( pTAInput->chControlInfoW, pTAInput->chControlInfo, UTF16strlenCHAR(pTAInput->chControlInfoW),
	                                      sizeof(pTAInput->chControlInfo), FALSE, &lRc);
          fOK = (UtlBufWrite( pBuffer, pTAInput->chControlInfo, ulLength, TRUE ) == NO_ERROR );
        } /* endif */

        // Write control end tag and source start tag                      
        if ( fOK )
        {
          strcpy( chTag, MEM_CONTROL_END_TAG );
          strcat( chTag, MEM_SOURCE_BEGIN_TAG );
          ulLength = strlen( chTag );
          fOK = (UtlBufWrite( pBuffer, chTag, ulLength, TRUE ) == NO_ERROR );
        } /* endif */

        // Write source segment                                            
        if ( fOK ) 
        {
          LONG lRc = 0;
	        ulLength = Unicode2UTF8BufEx( pszSegTempBuffer, pSegData, UTF16strlenCHAR(pszSegTempBuffer), 
	                                      ulSegBufLen, FALSE, &lRc);

          fOK = (UtlBufWrite( pBuffer, pSegData, ulLength, TRUE ) == NO_ERROR );
        }

        // write end source segment tag, empty target segment data and end segment tag
        if ( fOK )
        {
          strcpy( chTag, MEM_SOURCE_END_TAG );
          strcat( chTag, MEM_TARGET_BEGIN_TAG );
          strcat( chTag, MEM_TARGET_END_TAG );
          strcat( chTag, MEM_SEGMENT_END_TAG );
          ulLength = strlen( chTag );
          fOK = (UtlBufWrite( pBuffer, chTag, ulLength, TRUE ) == NO_ERROR );
        } /* endif */
     }
     else
     {
        CHAR chTag [ 80 ];                 // space to create tag

        if ( sFormat == WRITETMSEGMENT_ANSI )
        {
          if ( pInD)
          {
            // use CP of doc source language (ASCII CP required for Ansi!!!)
            UtlDirectUnicode2Ansi(pszSegTempBuffer, pSegData, pInD->TolstControl.ulAnsiCP);
          }
          else
          {
            UtlDirectUnicode2Ansi(pszSegTempBuffer, pSegData, 0L);      // use system pref.lang
          } /* endif */
        }
        else
        {
          if ( pInD)
          {
            // use CP of doc source language
            Unicode2ASCII(pszSegTempBuffer, pSegData, pInD->TolstControl.ulOemCP);
          }
          else
          {
            Unicode2ASCII(pszSegTempBuffer, pSegData, 0L);      // use system pref.lang
          } /* endif */
        } /* endif */

        if (fOK)
        {
          /*******************************************************************/
          /* Write segment start data                                        */
          /*******************************************************************/
          ulLength = sprintf( chTag, "%s%10.10ld\r\n", MEM_SEGMENT_BEGIN_TAG, ulSegNum );
          fOK = (UtlBufWrite( pBuffer, chTag, ulLength, TRUE ) == NO_ERROR );

          /*******************************************************************/
          /* Write control start tag                                         */
          /*******************************************************************/
          if ( fOK )
          {
            fOK = (UtlBufWrite( pBuffer, MEM_CONTROL_BEGIN_TAG,
                                strlen(MEM_CONTROL_BEGIN_TAG) , TRUE ) == NO_ERROR );
          } /* endif */

          /*******************************************************************/
          /* Write control data                                              */
          /*******************************************************************/
          if ( fOK )
          {
            fOK = (UtlBufWrite( pBuffer, pTAInput->chControlInfo, ulControlLength, TRUE ) == NO_ERROR );
          } /* endif */

          /*******************************************************************/
          /* Write control end tag and source start tag                      */
          /*******************************************************************/
          if ( fOK )
          {
            strcpy( chTag, MEM_CONTROL_END_TAG );
            strcat( chTag, MEM_SOURCE_BEGIN_TAG );
            ulLength = strlen( chTag );
            fOK = (UtlBufWrite( pBuffer, chTag, ulLength, TRUE ) == NO_ERROR );
          } /* endif */

          /*******************************************************************/
          /* Write source segment                                            */
          /*******************************************************************/
          if ( fOK )                                      // fill in segment
          {
            fOK = (UtlBufWrite( pBuffer, pSegData, ulSegLength, TRUE ) == NO_ERROR );
          }

          /*******************************************************************/
          /* write end source segment tag, empty target segment data and     */
          /* end segment tag                                                 */
          /*******************************************************************/
          if ( fOK )
          {
            strcpy( chTag, MEM_SOURCE_END_TAG );
            strcat( chTag, MEM_TARGET_BEGIN_TAG );
            strcat( chTag, MEM_TARGET_END_TAG );
            strcat( chTag, MEM_SEGMENT_END_TAG );
            ulLength = strlen( chTag );
            fOK = (UtlBufWrite( pBuffer, chTag, ulLength, TRUE ) == NO_ERROR );
          } /* endif */
        }
     } /* endif */
   } /* endif */

   if (pSegData) UtlAlloc((PVOID *)&pSegData, 0L, 0L, NOMSG);
   if (pszSegTempBuffer) UtlAlloc((PVOID *)&pszSegTempBuffer, 0L, 0L, NOMSG);

   return (fOK);
} /* end of WriteTMSegment */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    TARestartList                                             |
//+----------------------------------------------------------------------------+
//|Function call:    TARestartList( pTAInput)                                  |
//+----------------------------------------------------------------------------+
//|Description:      Scan the file list; for all files deletes them from the   |
//|                  right window, only for the files not in error set them    |
//|                  again back in the "to be processed" window.               |
//+----------------------------------------------------------------------------+
//|Parameters:       PTAINPUT pTAInput  pointer to the instance data area      |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:      TRUE                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                  BEGIN                                                     |
//|                     set pointer to first file name                         |
//|                     FOR all file names DO                                  |
//|                        delete founded files from right list                |
//|                        IF file status ok                                   |
//|                           insert in right list                             |
//|                        ENDIF                                               |
//|                     ENDFOR                                                 |
//|                     return TRUE                                            |
//|                  END                                                       |
//+----------------------------------------------------------------------------+

BOOL TARestartList
(
   PTAINPUT pTAInput
)
{
   PTAINSTDATA pInD;                   // pointer to instance data
   PSZ         pszCurrFile;            // pointer to current file name
   USHORT      usCount;                // file name counter
   SHORT       sItem;                  // item number
   PUSHORT     pusTemp;                // pointer to file status

   pInD = pTAInput->pInD;
   // pointer to first file name
   pszCurrFile = (PSZ) pTAInput + pTAInput->stSourcefiles.ulOffset;
   for (usCount = 0; usCount < pTAInput->stSourcefiles.usNumber; usCount++)
   {

       // delete founded files from right list
       sItem = SEARCHITEMHWND( pTAInput->hwndDone,
                        pTAInput->apszLongNames[usCount] );

       if (sItem != LIT_NONE)
          DELETEITEMHWND( pTAInput->hwndDone, sItem );

       // if file status ok, insert in left list
       pusTemp = (pInD->pusFileStatus + usCount);

       if ( *pusTemp )
         INSERTITEMHWND( pTAInput->hwndToDo,
                          pTAInput->apszLongNames[usCount] );

       // point to next file name, skipping null
       pszCurrFile += strlen(pszCurrFile) + 1;
   } /* endfor */

   return (TRUE);
} /* end of TARestartList */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    TALockFiles                                               |
//+----------------------------------------------------------------------------+
//|Function call:    TALockFiles(pTAInput)                                     |
//+----------------------------------------------------------------------------+
//|Description:      Scan the list of files and try to lock them to disable    |
//|                  access from other resources.                              |
//+----------------------------------------------------------------------------+
//|Parameters:       PTAINPUT  pTAInput   pointer to input structure           |
//+----------------------------------------------------------------------------+
//|Returncode type:  BOOL                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:      TRUE if operation has been successful                     |
//|                  else FALSE                                                |
//+----------------------------------------------------------------------------+
//|Side effects:     flag to terminate the process might be set                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                  BEGIN                                                     |
//|                     set pointer to first file name                         |
//|                     FOR all file names DO                                  |
//|                        build object name                                   |
//|                        IF an error occures THEN                            |
//|                           display an error message                         |
//|                           fOK = FALSE                                      |
//|                           set fKill                                        |
//|                        ELSE                                                |
//|                           object name was accepted                         |
//|                        ENDIF                                               |
//|                     ENDFOR                                                 |
//|                     return fOK                                             |
//|                  END                                                       |
//+----------------------------------------------------------------------------+

BOOL TALockFiles
(
    PTAINPUT  pTAInput                         // pointer to document name
)
{
   BOOL  fOK = TRUE;                           // success indicator
   SHORT sRc;                                  // return code
   PSZ   pData;                                // pointer to error data
   USHORT      usCount;                        // number of files
   CHAR  szFileName[ MAX_PATH144 ];            // complete file name
   PSZ         pszCurrFile;                    // pointer to current file name
   SHORT       sItem;                          // index of listbox item

   // allocate array for lock flags
   UtlAlloc( (PVOID *)&(pTAInput->pfDocLocked), 0L,
             max( 20L, (sizeof(BOOL) * pTAInput->stSourcefiles.usNumber)),
             NOMSG );

   // lock folder
   sRc = QUERYSYMBOL( pTAInput->szFolder );
   if ( sRc != -1 )
   {
     PSZ pszParm = pTAInput->szFolder;
     UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm, EQF_ERROR, pTAInput->hwndErrMsg );
     fOK = FALSE;
     pTAInput->fKill = TRUE;                // request stop
   }
   else
   {
     SETSYMBOL( pTAInput->szFolder );
     pTAInput->fFolderLocked = TRUE;
   } /* endif */

   // lock documents
   if ( fOK )
   {
    // pointer to first file name
    pszCurrFile = (PSZ) pTAInput + pTAInput->stSourcefiles.ulOffset;
    for (usCount = 0;
          ( fOK && (usCount < pTAInput->stSourcefiles.usNumber));
          usCount++)
    {
        // build object name (folder concatenated with file )
        sprintf( szFileName, "%s\\%s", pTAInput->szFolder, pszCurrFile);
        // check if symbol already exists, i.e. file is used
        sRc = QUERYSYMBOL( szFileName );
        if ( sRc != -1 )
        {
            pData = pTAInput->apszLongNames[usCount];
            if ( pTAInput->fBatch )
            {
              UtlErrorHwnd( ERROR_DOC_INUSE, MB_CANCEL,
                            1, &pData, EQF_ERROR, pTAInput->hwndErrMsg );
              fOK = FALSE;
              pTAInput->fKill = TRUE;                // request stop
            }
            else
            {
              if ( UtlErrorHwnd( ERROR_DOC_INUSE, MB_YESNO,
                                1, &pData, EQF_QUERY, pTAInput->hwndErrMsg ) == MBID_YES )
              {
                /**********************************************************/
                /* user wants to skip locked file                         */
                /**********************************************************/
                // exclude file from further processing
                pTAInput->pInD->pusFileStatus[usCount] = FALSE;

                // delete file from todo listbox
                sItem = SEARCHITEMHWND( pTAInput->hwndToDo, pData );
                DELETEITEMHWND( pTAInput->hwndToDo, sItem );

                // set error condition to indicate that not all files have been
                // processed
                pTAInput->pInD->usError = TA_PREV_ERROR;
              }
              else
              {
                /**********************************************************/
                /* user wants to stop analysis                            */
                /**********************************************************/
                fOK = FALSE;
                pTAInput->fKill = TRUE;                // request stop
              } /* endif */
            } /* endif */
        }
        else
        {
            SETSYMBOL( szFileName );
            pTAInput->pfDocLocked[usCount] = TRUE;
            pTAInput->pInD->pusFileStatus[usCount] = TRUE;
        } /* endif */

        /*************************************************************/
        /* Point to next file name, skipping null                    */
        /*************************************************************/
        pszCurrFile += strlen(pszCurrFile) + 1;
    } /* endfor */
   } /* endif */

   /********************************************************************/
   /* Lock term lists ...                                              */
   /********************************************************************/
   if ( fOK )
   {
     if ( pTAInput->szNTLname[0] != '\0' )
     {
       if ( QUERYSYMBOL( pTAInput->szNTLname ) != -1 )
       {
         CHAR  szListName[MAX_FNAME];
         PSZ   pszParm;

         Utlstrccpy( szListName,
                     UtlGetFnameFromPath( pTAInput->szNTLname ), DOT );
         pszParm = szListName;
         UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszParm, EQF_INFO );
         fOK = FALSE;
       }
       else
       {
         SETSYMBOL( pTAInput->szNTLname );
         pTAInput->fNTLLocked = TRUE;
       } /* endif */
     } /* endif */
   } /* endif */

   if ( fOK )
   {
     if ( pTAInput->szFTLname[0] != '\0' )
     {
       if ( QUERYSYMBOL( pTAInput->szFTLname ) != -1 )
       {
         CHAR  szListName[MAX_FNAME];
         PSZ   pszParm;

         Utlstrccpy( szListName,
                     UtlGetFnameFromPath( pTAInput->szFTLname ), DOT );
         pszParm = szListName;
         UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszParm, EQF_INFO );
         fOK = FALSE;
       }
       else
       {
         SETSYMBOL( pTAInput->szFTLname );
         pTAInput->fFTLLocked = TRUE;
       } /* endif */
     } /* endif */
   } /* endif */

   if ( fOK )
   {
     if ( pTAInput->szExclusionList[0] != '\0' )
     {
       if ( QUERYSYMBOL( pTAInput->szExclusionList ) != -1 )
       {
         CHAR  szListName[MAX_FNAME];
         PSZ   pszParm;

         Utlstrccpy( szListName,
                     UtlGetFnameFromPath( pTAInput->szExclusionList ), DOT );
         pszParm = szListName;
         UtlError( ERROR_LST_IN_USE, MB_CANCEL, 1, &pszParm, EQF_INFO );
         fOK = FALSE;
       }
       else
       {
         SETSYMBOL( pTAInput->szExclusionList );
         pTAInput->fExcLocked = TRUE;
       } /* endif */
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Lock exclusion dictionary                                       */
   /*******************************************************************/
   if ( fOK )
   {
     if ( pTAInput->szExclDictname[0] != '\0' )
     {
       /***************************************************************/
       /* Build object name for dictionary                            */
       /***************************************************************/
       OBJNAME     szObject;
       UtlMakeEQFPath( szObject, NULC, SYSTEM_PATH, NULL );
       strcat( szObject, BACKSLASH_STR );
       strcat( szObject, pTAInput->szExclDictname );
       strcat( szObject, EXT_OF_DICTPROP );

       /***************************************************************/
       /* Check and lock dictionary                                   */
       /***************************************************************/
       if ( QUERYSYMBOL( szObject ) != -1 )
       {
         PSZ pszParm = pTAInput->szExclDictname;
         UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszParm, EQF_INFO );
         fOK = FALSE;
       }
       else
       {
         SETSYMBOL( szObject );
         pTAInput->fExcDicLocked = TRUE;
       } /* endif */
     } /* endif */
   } /* endif */

   return fOK;
} /* end of TALockFiles */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    TAUnlockFiles                                             |
//+----------------------------------------------------------------------------+
//|Function call:    TAUnlockFiles(pTAInput)                                   |
//+----------------------------------------------------------------------------+
//|Description:      Scan the list of files and unlock them to unable access   |
//|                  for other resources.                                      |
//+----------------------------------------------------------------------------+
//|Parameters:       PTAINPUT  pTAInput   pointer to input structure           |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                  BEGIN                                                     |
//|                     set pointer to first file name                         |
//|                     FOR all file names DO                                  |
//|                        build object name                                   |
//|                        remove the symbol from the object manager           |
//|                            don't care about errors                         |
//|                     ENDFOR                                                 |
//|                  END                                                       |
//+----------------------------------------------------------------------------+

VOID TAUnlockFiles
(
    PTAINPUT pTAInput                          // pointer to input struct
)
{
   PSZ         pszCurrFile;                    // pointer to current file name
   CHAR  szFileName[ MAX_PATH144 ];            // complete file name
   USHORT      usCount;                        // number of files

   // unlock folder
   if ( pTAInput->fFolderLocked )
   {
      REMOVESYMBOL( pTAInput->szFolder );
   } /* endif */

   // pointer to first file name
   pszCurrFile = (PSZ) pTAInput + pTAInput->stSourcefiles.ulOffset;
   if ( pTAInput->pfDocLocked != NULL )
   {
     for (usCount = 0; usCount < pTAInput->stSourcefiles.usNumber; usCount++)
     {
       if ( pTAInput->pfDocLocked[usCount] )
       {
         // build object name (folder concatenated with file )
         sprintf( szFileName, "%s\\%s", pTAInput->szFolder, pszCurrFile);
         REMOVESYMBOL( szFileName );
       } /* endif */

       // point to next file name, skipping null
       pszCurrFile += strlen(pszCurrFile) + 1;
     } /* endfor */
     UtlAlloc((PVOID *) &(pTAInput->pfDocLocked), 0L, 0L, NOMSG );
   } /* endif */

   /********************************************************************/
   /* Unlock term lists ...                                            */
   /********************************************************************/
   if ( pTAInput->fNTLLocked )
   {
     REMOVESYMBOL( pTAInput->szNTLname );
   } /* endif */
   if ( pTAInput->fFTLLocked )
   {
     REMOVESYMBOL( pTAInput->szFTLname );
   } /* endif */
   if ( pTAInput->fExcLocked )
   {
     REMOVESYMBOL( pTAInput->szExclusionList );
   } /* endif */

   /*******************************************************************/
   /* Unlock exclusion dictionary                                     */
   /*******************************************************************/
   if ( pTAInput->fExcDicLocked )
   {
     OBJNAME     szObject;
     UtlMakeEQFPath( szObject, NULC, SYSTEM_PATH, NULL );
     strcat( szObject, BACKSLASH_STR );
     strcat( szObject, pTAInput->szExclDictname );
     strcat( szObject, EXT_OF_DICTPROP );
     REMOVESYMBOL( szObject );
   } /* endif */

} /* end of TAUnlockFiles */

