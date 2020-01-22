/*! \file
	Description: EQF Translation Module

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_BASE
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_LIST             // list handler functions
#include <eqf.h>                  // General Translation Manager include file


/**********************************************************************/
/* The next include is necessary due to inconsistent or               */
/* not existing interfaces ...                                        */
/**********************************************************************/
#include "EQFDDE.H"               // Batch mode definitions
#include <eqftai.h>               // private text analysis include file
#include "EQFHLOG.h"            // counting report
#include "eqfrpt00.h"
#include "eqfdoc00.h"             // private document handler include file
#include "eqfserno.h"

#include "EQFTPI.H"               // Translation Processor priv. include file

#include "core\utilities\LogWriter.h"

// activate the following define to log avents related to saving the segment
// #define SEGMENTSAVE_LOG

#ifdef SEGMENTSAVE_LOG
  LogWriter *pSaveSegLog = NULL;
#endif

void WriteSaveSegLog( PSZ pszLogText )
{
#ifdef SEGMENTSAVE_LOG
  if ( pSaveSegLog == NULL )
  {
    pSaveSegLog = new LogWriter(); 
    if ( pSaveSegLog != NULL )
    {
      pSaveSegLog->open( "EditorSaveSegLog", LogWriter::OM_UTF16 | LogWriter::OM_APPEND ); 
      pSaveSegLog->setForcedWrite( TRUE );
    }
  } /* endif */
  if ( pSaveSegLog != NULL )
  {
    pSaveSegLog->write( pszLogText );
  }
#endif
  return;
}

#define MSGBOXDATALEN     30        // data len of segment to be displ.

#define CHECKPOS_XLATESEG 0         // same position
#define CHECKPOS_MOVE   1           // user movement
#define CHECKPOS_XLATED 2           // already translated
#define CHECKPOS_FIRST  3           // fist call to tm
#define CHECKPOS_POSTEDIT 4         // post edit mode was active



#define  ORIGINAL_PROP  0           // number of proposal which is source

#ifdef TRANTIME
  static  ULONG  ulTime = 0L;
  static  ULONG  ulSave = 0L;
  static  ULONG  ulStart;
  static  ULONG  ulEnd;
  static  ULONG  ulBegin;
  static  ULONG  ulDelta = 0L;
  FILE    *fStream;
#endif

static USHORT EQFBTCheckPos( PTBDOCUMENT );     // check position of new tag
static BOOL EQFBTInitTrans( PTBDOCUMENT, PULONG );   // init translation environment

static VOID EQFBDelSeg ( PTBDOCUMENT, USHORT ); // delete segment from TM

static BOOL EQFBJoinSegData ( PTBDOCUMENT,USHORT,USHORT); // join segment data

static BOOL EQFBSplitSegData ( PTBDOCUMENT, USHORT, USHORT,
                               USHORT, USHORT ); // split seg data

static PTBSEGMENT EQFBFindNextAutoSource( PTBDOCUMENT,
                                         PUSHORT ); // find next source special

static BOOL EQFBIsTRNoteInNOP(PTBDOCUMENT, PTBSEGMENT );
static VOID EQFBDictLookEdit ( PTBDOCUMENT, BOOL );

static void EQFBPasteInSeg ( PTBDOCUMENT, PSZ_W );

static CHAR_W chSeg1[MSGBOXDATALEN + 1];  // start of segment 1
static CHAR_W chSeg2[MSGBOXDATALEN + 1];  // start of segment 2

// list of MT fields to be copied to MTLOG and memory proposal (zero terminated strings followed by zero)
#define MTFIELDLIST L"MT\0TM-MatchType\0MT-ServiceID\0MT-MetricName\0MT-MetricValue\0PE-EditDistanceChars\0PE-EditDistanceWords\0TM-DomainClass\0TM-DomainSubClass\0\0"

// copy specific fields from proposal meta data area
VOID AddData_CopyFieldsFromList( PSZ_W pszTarget, PSZ_W pszSource, PSZ_W pszFieldList );

//#define SHRINKLF_SEGDATA  "@\n"        // shrinked data with LF
//#define SHRINKED_SEGDATA  "@"          // shrinked data


static TBSEGMENT ShrinkSeg=            // shrinkSegment
    {
      NULL,                            // pointer to segment data
      2,                               // length of segment data
      QF_NOP,                          // status of segment  (XLATED, TOBE,..)
      0,                               // segment number
      NULL,                            // Browser Protection Elements Table
      0x00,                                      // segment flags
      0,0,0,0,0,0               // OrgLen, Cntflag, Src-Tgt-Mod-Words, ShrinkLen
    };

// table for determing Character/Whitespace
// Codepage dependent - should be replaced by a generic CodePage supporting
// isalnum C-function
//  1 : isalnum,  0: not alnum

CHAR chDictLookup[256] =
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
     0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0 };


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBDictLook - activate the dictionary lookup
//------------------------------------------------------------------------------
// Function call:     EQFBDictLook( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       This function will start the dictionary lookup
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       -  pointer to document instance area
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:      primitive function but necessary to allow calling logic
//                    within browser
//------------------------------------------------------------------------------
// Function flow:      call EQFDICTLOOKEDIT function
//
//------------------------------------------------------------------------------
VOID
EQFBDictLook
(
   PTBDOCUMENT pDoc          // pointer to document instance
)
{
   if ( pDoc->hwndRichEdit )
   {
     /*****************************************************************/
     /* ensure that workseg is up-to-date                             */
     /*****************************************************************/
     if (pDoc->ulWorkSeg)
       EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );

     EQFBGetSelBlockRTF( pDoc );
   } /* endif */
  /********************************************************************/
  /* invoke the Dictionary LookEdit function with FALSE               */
  /********************************************************************/
  EQFBDictLookEdit( pDoc, FALSE );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBAddAbbrev (PTBDOCUMENT)
//------------------------------------------------------------------------------
// Function call:     EQFBAddAbbrev( pDoc )
//------------------------------------------------------------------------------
// Description:       add an abbreviation to the morphological dictionary
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDOc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID
EQFBAddAbbrev
(
  PTBDOCUMENT   pDoc
)
{
   PDOCUMENT_IDA  pIdaDoc;                       // pointer to document struct.

   if ( pDoc->hwndRichEdit )
   {
     /*****************************************************************/
     /* ensure that workseg is up-to-date                             */
     /*****************************************************************/
     if (pDoc->ulWorkSeg)
       EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );

     EQFBGetSelBlockRTF( pDoc );
   } /* endif */
   pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
   EQFBAddAbbrevFunc(  pDoc, pIdaDoc->sSrcLanguage );

} /* end of function EQFBAddAbbrev */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBEditAbbrev (PTBDOCUMENT)
//------------------------------------------------------------------------------
// Function call:     EQFBEditAbbrev( pDoc )
//------------------------------------------------------------------------------
// Description:       call list handler function to edit an abbreviation
//                    dictionary
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------
VOID EQFBEditAbbrev
(
  PTBDOCUMENT   pDoc
)
{
   PDOCUMENT_IDA  pIdaDoc;                       // pointer to document struct.

   pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
   LstEditAbbrev(  pIdaDoc->szDocSourceLang );

} /* end of function EQFBEditAbbrev */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBEditAddenda (PTBDOCUMENT)
//------------------------------------------------------------------------------
// Function call:     EQFBEditAddenda( pDoc )
//------------------------------------------------------------------------------
// Description:       call list handler function to edit an addenda
//                    dictionary
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDoc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------
VOID EQFBEditAddenda
(
  PTBDOCUMENT   pDoc
)
{
   PDOCUMENT_IDA  pIdaDoc;                       // pointer to document struct.

   pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
   LstEditAddenda( pIdaDoc->szDocTargetLang );

   {
     ULONG ulSegNum = 1;
     PTBSEGMENT pSeg = EQFBGetSegW(pDoc, ulSegNum);

     while (pSeg && (ulSegNum < pDoc->ulMaxSeg) )
     {
       if (pSeg->pusHLType )
       {
         UtlAlloc((PVOID *)&(pSeg->pusHLType) ,0L ,0L , NOMSG);
       } /* endif */
       pSeg->SegFlags.Spellchecked = FALSE;
       ulSegNum++;
       pSeg = EQFBGetSegW(pDoc, ulSegNum);
     } /* endwhile */
     pDoc->Redraw |= REDRAW_ALL;
     if (pDoc->fAutoSpellCheck && pDoc->pvSpellData)
     {	// force that thread recalcs pusHLType of screen
		PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
		pSpellData->TBFirstLine.ulSegNum = 0;
		pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs
	 } /* endif */

   }
} /* end of function EQFBEditAddenda */

/**********************************************************************/
/* do the real abbreviation processing with the passed language id    */
/**********************************************************************/
VOID
EQFBAddAbbrevFunc
(
  PTBDOCUMENT   pDoc,
  SHORT         sLanguageId
)
{
   PTBSEGMENT  pSeg;                            // pointer to segment
   PSZ_W       pData;                           // pointer to data
   PSZ_W       pTemp;                           // pointer to data
   USHORT      usStart = 0;                     // start of string
   USHORT      usEnd = 0;                       // end of string
   PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;      // block structure
   BOOL        fFound = FALSE;
   ULONG       ulOemCP = 0L;

   CHAR   chLang[ MAX_LANG_LENGTH ];
   PSZ    pszLanguage = chLang; // ptr to language string by id..

   MorphGetLanguageString( sLanguageId, pszLanguage );
   ulOemCP = GetLangOEMCP( pszLanguage);

   EQFBCurSegFromCursor( pDoc );                 // get cursor position

   /*******************************************************************/
   /* check if cursor is in marked area                               */
   /*  - if so use marked area, else use cursor word                  */
   /*******************************************************************/
   pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
   if ( pSeg )
   {
       if ( (pstBlock->pDoc == pDoc)
              && (pDoc->TBCursor.ulSegNum == pstBlock->ulSegNum)
              && (pDoc->TBCursor.usSegOffset >= pstBlock->usStart)
              && (pDoc->TBCursor.usSegOffset <= pstBlock->usEnd+1))
       {
          usStart = pstBlock->usStart;
          usEnd = pstBlock->usEnd + 1;
          fFound = TRUE;
          /***************************************************************/
          /* check if ending character is a '.' - add this character to  */
          /* our word in such a case                                     */
          /***************************************************************/
          if ( pSeg->pDataW[usEnd] == DOT )
          {
            usEnd++;
          } /* endif */
       }
       else
       {
           fFound = EQFBFindWord(pSeg->pDataW,
                                  sLanguageId,
                                  pDoc->TBCursor.usSegOffset,
                                  &usStart,
                                  &usEnd, ulOemCP, FALSE);

           /*****************************************************************/
           /* check if we have a tag at the begin of the word...            */
           /*****************************************************************/
           if ( fFound )
           {
             USHORT usLen = usStart;
             USHORT usTypeChar;
             DISPSTYLE  DispStyle;      // temporary display style
           // store dispstyle and set it to protected if unprotected to get tags coloured
             DispStyle =  pDoc->DispStyle;
             pDoc->DispStyle = DISP_PROTECTED;

             while (usLen < usEnd)
             {
               usTypeChar = EQFBCharType(pDoc,pSeg,usLen);
               if (usTypeChar == PROTECTED_CHAR)
               {
                 usLen ++;
               }
               else
               {
                 break;                          // leave the loop...
               } /* endif */
             } /* endwhile */

             /// set the new starting point ..
             usStart = usLen;
             usLen = usEnd;
             while (usLen > usStart)
             {
               usTypeChar = EQFBCharType(pDoc,pSeg,usLen);
               if (usTypeChar == PROTECTED_CHAR)
               {
                 usLen --;
               }
               else
               {
                  break;                          // leave the loop...
               } /* endif */
             } /* endwhile */
             usEnd = usLen;

             pDoc->DispStyle = DispStyle;      // reset protect style
           } /* endif */

           if ( fFound &&  !IsDBCS_CP(pDoc->ulOemCodePage))
           {
             usEnd++;
             /***************************************************************/
             /* check if ending character is a '.' - add this character to  */
             /* our word in such a case                                     */
             /***************************************************************/
             if ( pSeg->pDataW[usEnd] == DOT )
             {
               usEnd++;
             } /* endif */
             fFound = TRUE;
           } /* endif */
       } /* endif */
   } /* endif */

   if (fFound && (usStart < usEnd) )
   {
      if ( usEnd - usStart >= MAX_DICTLENGTH )
      {
                                                  // error message
         UtlError( TB_DICTENTRYTOOLONG, MB_CANCEL, 0, NULL, EQF_WARNING);
      }
      else
      {
        USHORT  usResult;              // result from message box

        if ( UtlAlloc( (PVOID *) &pData, 0L, MAX_DICTLENGTH, ERROR_STORAGE) )
        {
          memcpy( pData,pSeg->pDataW+usStart, (usEnd-usStart) * sizeof(CHAR_W) );
          *(pData+usEnd - usStart) = EOS ;
          pTemp = pData;
          while ( *pTemp )
          {
            if ( *pTemp == LF )
            {
               *pTemp = BLANK;
            } /* endif */
            pTemp++;
          } /* endwhile */
          /**********************************************************/
          /* add the selection to the morphological dictionary      */
          /* if requested...                                        */
          /**********************************************************/
          {
            usResult = UtlErrorW( TB_ADDABBREVIATION, MB_YESNO,
                                 1, &pData, EQF_QUERY, TRUE);
          }
          if ( usResult == MBID_YES )
          {
            /********************************************************/
            /*  no additional checking is done, because in some     */
            /*  languages abbreviations do not end in an '.'        */
            /* Due to special request, abbreviations within TM      */
            /* should always end in a DOT - we add it therefore if  */
            /* not available    (KWT0399)                           */
            /********************************************************/
            USHORT usRC;
            USHORT usLen;

            UtlStripBlanksW( pData ); // always remove leeding and trailing blanks!
            usLen = (USHORT)UTF16strlenCHAR( pData );
            if ( pData[usLen-1] != DOT )
            {
              pData[usLen] = DOT;
              pData[usLen+1] = EOS;
            } /* endif */

            usRC = MorphAddToAbbrev( sLanguageId, pData, ulOemCP );
            if ( usRC != MORPH_OK )
            {
              USHORT usMsg;
              BOOL   fMsgDisplayed = FALSE;

              /******************************************************/
              /* Handle errors returned by Morph functions          */
              /******************************************************/
              switch ( usRC )
              {
                case MORPH_NO_MEMORY :
                  usMsg = ERROR_STORAGE;
                  break;

                case MORPH_INV_TERM :
                  usMsg = TB_INVABBREVIATION;
                  UtlErrorW( usMsg, MB_CANCEL, 1, &pData, EQF_ERROR, TRUE );
                  fMsgDisplayed = TRUE;
                  break;

                case MORPH_BUFFER_OVERFLOW :
                case MORPH_INV_PARMS  :
                case MORPH_INV_LANG_ID :
                  usMsg = ERROR_INTERNAL;
                  break;

                case MORPH_FUNC_NOT_SUPPORTED :
                  usMsg = ERROR_NOABBR_SUPPORT;
                  break;

                default :
                  usMsg = EQFRS_NOMORPH_DICT;
                  break;
              } /* endswitch */
              if ( !fMsgDisplayed )
              {
                UtlError( usMsg, MB_CANCEL, 1, &pszLanguage, EQF_ERROR );
              } /* endif */
            } /* endif */
          } /* endif */

          UtlAlloc( (PVOID *) &pData, 0L, 0L, NOMSG);
        } /* endif */
      } /* endif */
   }
   else
   {
                                                   // empty string
      UtlError( TB_DICTENTRYEMPTY, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */

} /* end of function EQFBAddAbbrevFunc */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBFuncEditTerm (PTBDOCUMENT)
//------------------------------------------------------------------------------
// Function call:     EQFBFuncEditTerm(pDOc)
//------------------------------------------------------------------------------
// Description:       edit a term in the dictionary
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   pDOc
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     invoke the dictionary LookEdit function with TRUE
//------------------------------------------------------------------------------

VOID
EQFBFuncEditTerm
(
  PTBDOCUMENT   pDoc
)
{
   if ( pDoc->hwndRichEdit )
   {
     /*****************************************************************/
     /* ensure that workseg is up-to-date                             */
     /*****************************************************************/
     if (pDoc->ulWorkSeg)
       EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );

     EQFBGetSelBlockRTF( pDoc );
   } /* endif */
  /********************************************************************/
  /* invoke the Dictionary LookEdit function with FALSE               */
  /********************************************************************/
  EQFBDictLookEdit( pDoc, TRUE );

} /* end of function EQFBFuncEditTerm */
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBDictLookEdit - call dict look or edit
//------------------------------------------------------------------------------
// Function call:     EQFBDictLook( PTBDOCUMENT, BOOL );
//
//------------------------------------------------------------------------------
// Description:       This function will start the dictionary lookup
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       -  pointer to document instance area
//                    BOOL              - TRUE: Edit, FALSE: Lookup
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:      get cursor position
//                     check if mark exist in current segment
//                         if so use this else use cursor word
//                     extract word and copy it into temp space
//                     Call services (EQFDICTLOOK to look up term)
//
//------------------------------------------------------------------------------
static VOID
EQFBDictLookEdit
(
   PTBDOCUMENT pDoc,                   // pointer to document instance
   BOOL        fAction                 // lookup or edit
)
{
   PTBSEGMENT  pSeg;                            // pointer to segment
   PSZ_W       pData;                           // pointer to data
   PSZ_W       pTemp;                           // pointer to data
   USHORT      usStart;                          // start of string
   USHORT      usEnd;                            // end of string
   PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;      // block structure
   SHORT       sLanguageId;
   BOOL        fFound = FALSE;
   PDOCUMENT_IDA  pIdaDoc;                       // pointer to document struct.


   EQFBCurSegFromCursor( pDoc );                 // get cursor position

   /*******************************************************************/
   /* check if cursor is in marked area                               */
   /*  - if so use marked area, else use cursor word                  */
   /*******************************************************************/
   pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );
   if ( (pstBlock->pDoc == pDoc)
          && (pDoc->TBCursor.ulSegNum == pstBlock->ulSegNum)
          && (pDoc->TBCursor.usSegOffset >= pstBlock->usStart)
          && (pDoc->TBCursor.usSegOffset <= pstBlock->usEnd+1))
   {
      usStart = pstBlock->usStart;
      usEnd = pstBlock->usEnd + 1;
      fFound = TRUE;
   }
   else
   {
     pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
     sLanguageId = pIdaDoc->sSrcLanguage;
     fFound = EQFBFindWord(pSeg->pDataW,
                            sLanguageId,
                            pDoc->TBCursor.usSegOffset,
                            &usStart,
                            &usEnd, pIdaDoc->ulSrcOemCP, FALSE);

     /*****************************************************************/
     /* check if we have a tag at the begin of the word...            */
     /*****************************************************************/
     if ( fFound )
     {
       USHORT usLen = usStart;
       USHORT usTypeChar;
       DISPSTYLE  DispStyle;      // temporary display style
     // store dispstyle and set it to protected if unprotected to get tags coloured
       DispStyle =  pDoc->DispStyle;
       pDoc->DispStyle = DISP_PROTECTED;

       while (usLen < usEnd)
       {
         usTypeChar = EQFBCharType(pDoc,pSeg,usLen);
         if (usTypeChar == PROTECTED_CHAR)
         {
           usLen ++;
         }
         else
         {
           /***********************************************************/
           /* leave the loop...                                       */
           /***********************************************************/
           break;
         } /* endif */
       } /* endwhile */

       /***************************************************************/
       /* set the new starting point ..                               */
       /***************************************************************/
       usStart = usLen;

       usLen = usEnd;
       while (usLen > usStart)
       {
         usTypeChar = EQFBCharType(pDoc,pSeg,usLen);
         if (usTypeChar == PROTECTED_CHAR)
         {
           usLen --;
         }
         else
         {
           /***********************************************************/
           /* leave the loop...                                       */
           /***********************************************************/
           if (usLen < usEnd )
           {
             usLen++;
           } /* endif */
           break;
         } /* endif */
       } /* endwhile */
       usEnd = usLen;

       /***************************************************************/
       /* reset protect style                                         */
       /***************************************************************/
       pDoc->DispStyle = DispStyle;
     } /* endif */
   } /* endif */

   if (fFound && (usStart < usEnd) )
   {
      if ( usEnd - usStart >= MAX_DICTLENGTH )
      {
                                                  // error message
         UtlError( TB_DICTENTRYTOOLONG, MB_CANCEL, 0, NULL, EQF_WARNING);
      }
      else
      {
         if ( UtlAlloc( (PVOID *) &pData, 0L, MAX_DICTLENGTH * sizeof(CHAR_W), ERROR_STORAGE) )
         {
            memcpy( pData,pSeg->pDataW+usStart, (usEnd-usStart)*sizeof(CHAR_W) );
            *(pData+usEnd - usStart ) = EOS ;
            pTemp = pData;
            while ( *pTemp )
            {
               if ( *pTemp == LF )
               {
                  *pTemp = BLANK;
               } /* endif */
               pTemp++;
            } /* endwhile */
            /**********************************************************/
            /* if TRUE invoke Dictionary Edit, else DictLookup...     */
            /**********************************************************/
            if ( fAction )
            {
              EQFDICTEDITW( pData );
            }
            else
            {
              EQFDICTLOOKW( pData, pData, 0 , 1);
            } /* endif */
            UtlAlloc( (PVOID *) &pData, 0L, 0L, NOMSG);
         } /* endif */
      } /* endif */
   }
   else
   {
                                                   // empty string
      UtlError( TB_DICTENTRYEMPTY, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */

}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBWordPos - find Start/end of current word
//------------------------------------------------------------------------------
// Function call:     EQFBWordPos( PTBDOCUMENT, PUSHORT, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Find Start/end of current word according to chDictLookUp
//                    table.
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       -  pointer to document instance area
//                    PUSHORT           - position of start of word
//                    PUSHORT           - position of end of word
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:      To determine Start and End of a word the chDictLookUp
//                    table is used
//------------------------------------------------------------------------------
// Function flow:     - set style to protected to skip protected chars
//                    - search start of word and pass begin value back
//                    - search end of word and pass end value back
//                    - reset style
//------------------------------------------------------------------------------
VOID
EQFBWordPos
(
   PTBDOCUMENT pDoc,         // pointer to document instance
   PUSHORT  pusWordStart,
   PUSHORT  pusWordEnd
)
{
   PSZ_W      pData;          // pointer to data
   PTBSEGMENT pSeg;           // pointer to segment struct
   SHORT      sTemp;          // temp index for word begin/end
   BOOL       fValid;         // character valid
   DISPSTYLE  DispStyle;      // temporary display style
   UCHAR chOEM[2];            // buffer for conversion

   chOEM[1] = EOS;
   pSeg = EQFBGetSegW( pDoc, pDoc->TBCursor.ulSegNum );

   sTemp = pDoc->TBCursor.usSegOffset;    // point to cursor offset
   pData = pSeg->pDataW + sTemp;           // pointer to data

   // search start of word

   fValid = TRUE;
 // store dispstyle and set it to protected if unprotected to get tags coloured
   DispStyle =  pDoc->DispStyle;
   pDoc->DispStyle = DISP_PROTECTED;
   while ( fValid && sTemp >= 0)
   {
      chOEM[0] = (UCHAR)*pData;
      AnsiToOem((LPCSTR)chOEM, (LPSTR)chOEM);
      if ( chDictLookup[chOEM[0]] )
      {
         pData--;
         sTemp --;
      }
      else
      {
         // check if it is a protected character
         if ( EQFBCharType(pDoc,pSeg,sTemp) == PROTECTED_CHAR  )
         {
            pData--;
            sTemp--;
         }
         else
         {
            fValid = FALSE;         // end reached
         } /* endif */
      } /* endif */
   } /* endwhile */
   *pusWordStart = sTemp + 1;                // point to start of word
 // reset display style
   pDoc->DispStyle = DispStyle;



   // search end of word
   sTemp = pDoc->TBCursor.usSegOffset;       // point to cursor offset
   pData = pSeg->pDataW + sTemp;              // pointer to data
   fValid = TRUE;
 // store dispstyle and set it to protected if unprotected to get tags coloured
   DispStyle =  pDoc->DispStyle;
   pDoc->DispStyle = DISP_PROTECTED;
   while ( fValid )
   {
      chOEM[0] = (UCHAR)*pData;
      AnsiToOem((LPCSTR)chOEM, (LPSTR)chOEM);

      if ( chDictLookup[chOEM[0]] )
      {
         pData++;
         sTemp ++;
      }
      else
      {
         // check if it is a protected character
         if ( EQFBCharType(pDoc,pSeg,sTemp) == PROTECTED_CHAR  )
         {
            pData++;
            sTemp++;
         }
         else
         {
            fValid = FALSE;         // end reached
         } /* endif */
      } /* endif */
   } /* endwhile */
 // reset display style
   pDoc->DispStyle = DispStyle;
   // we've gone 1 too far if something found else it might be still 0
   if ( sTemp )
   {
     *pusWordEnd = sTemp -1;
   }
   else
   {
      *pusWordEnd = 0;
   } /* endif */

   // we deal with a one character word, i.e. a delimiter like ',','.', etc.
   if ( *pusWordEnd < *pusWordStart
       && EQFBCharType(pDoc,pSeg,
                       pDoc->TBCursor.usSegOffset) == UNPROTECTED_CHAR )
   {
      *pusWordEnd =
      *pusWordStart =  pDoc->TBCursor.usSegOffset;   // point to cursor offset
   } /* endif */
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBActProp - activate proposal window
//------------------------------------------------------------------------------
// Function call:     EQFBActProp( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       This function will activate the proposal window
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       -  pointer to document instance area
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:        primitive function but necessary to allow calling
//                      logic within browser
//
//------------------------------------------------------------------------------
// Function flow:      if not in postedit mode
//                        Call services (EQFGETPROP with EQF_ACTIVATE)
//                     else
//                        beep
//                     endif
//
//------------------------------------------------------------------------------
VOID
EQFBActProp ( PTBDOCUMENT pDoc  )       // activate transl. memory
{
   if ( !pDoc->EQFBFlags.PostEdit )
   {
      EQFGETPROP ( EQF_ACTIVATE, NULL, NULL );
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no proposal wnd
   } /* endif */

}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBPropUp - scroll the proposal window up
//------------------------------------------------------------------------------
// Function call:     EQFBPropUp( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will scroll the proposal window up
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Side effects:        primitive function but necessary to allow calling
//                      logic within browser
//
//------------------------------------------------------------------------------
// Function flow:      if not in postedit mode
//                        Call services (EQFGETPROP with EQF_SCROLL_UP)
//                     else
//                        beep
//                     endif
//------------------------------------------------------------------------------
VOID
EQFBPropUp ( PTBDOCUMENT pDoc  )       // scroll transl. memory wnd up
{
   if ( !pDoc->EQFBFlags.PostEdit )
   {
      EQFGETPROP ( EQF_SCROLL_UP, NULL, NULL );
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no proposal wnd
   } /* endif */

}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBPropDown - scroll the proposal window down
//------------------------------------------------------------------------------
// Function call:     EQFBPropDown( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will scroll the proposal window down
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Side effects:       primitive function but necessary to allow calling
//                     logic within browser
//
//------------------------------------------------------------------------------
// Function flow:      if not in postedit mode
//                        Call services (EQFGETPROP with EQF_SCROLL_DOWN)
//                     else
//                        beep
//                     endif
//
//------------------------------------------------------------------------------
VOID
EQFBPropDown( PTBDOCUMENT pDoc  )       // scroll transl. memory wnd down
{
   if ( !pDoc->EQFBFlags.PostEdit )
   {
      EQFGETPROP ( EQF_SCROLL_DOWN, NULL, NULL );
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no transl.mem.wnd
   } /* endif */

}
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBActDict - activate the dictionary window
//------------------------------------------------------------------------------
// Function call:     EQFBActDict( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will activate the dictionary window
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:       primitive function but necessary to allow calling
//                     logic within browser
//
//------------------------------------------------------------------------------
// Function flow:      if not in postedit mode
//                        Call services (EQFGETDICT with EQF_ACTIVATE)
//                     else
//                        beep
//                     endif
//
//------------------------------------------------------------------------------
VOID
EQFBActDict ( PTBDOCUMENT pDoc  )       // activate dictionary window
{
   if ( !pDoc->EQFBFlags.PostEdit )
   {
      EQFGETDICT ( EQF_ACTIVATE, NULL );
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no dict. window
   } /* endif */

}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBDictUp - scroll the dictionary window up
//------------------------------------------------------------------------------
// Function call:     EQFBDictUp( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will scroll the dictionary window up
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:       primitive function but necessary to allow calling
//                     logic within browser
//
//------------------------------------------------------------------------------
// Function flow:      if not in postedit mode
//                        Call services (EQFGETDICT with EQF_SCROLL_UP)
//                     else
//                        beep
//                     endif
//
//------------------------------------------------------------------------------
VOID
EQFBDictUp  ( PTBDOCUMENT pDoc  )       // scroll dictionary wnd up
{
   if ( !pDoc->EQFBFlags.PostEdit )
   {
      EQFGETDICT ( EQF_SCROLL_UP, NULL );
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no dictionary window
   } /* endif */

}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBDictDown - scroll the dictionary window down
//------------------------------------------------------------------------------
// Function call:     EQFBDictDown( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will scroll the dictionary window down
//
//------------------------------------------------------------------------------
// Function flow:      Call services (EQFGETDICT with EQF_SCROLL_DOWN)
//
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:       primitive function but necessary to allow calling
//                     logic within browser
//
//------------------------------------------------------------------------------
// Function flow:      if not in postedit mode
//                        Call services (EQFGETDICT with EQF_SCROLL_DOWN)
//                     else
//                        beep
//                     endif
//
//------------------------------------------------------------------------------
VOID
EQFBDictDown( PTBDOCUMENT pDoc  )       // scroll dictionary wnd down
{
   if ( !pDoc->EQFBFlags.PostEdit )
   {
      EQFGETDICT ( EQF_SCROLL_DOWN, NULL );
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no dict.window
   } /* endif */

}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBDictPrefixUp - scroll the dictionary assigned prefixes up
//------------------------------------------------------------------------------
// Function call:     EQFBDictPrefixUp( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will re-assign the dictionary prefixes up
//                     to the previous terms.
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:       primitive function but necessary to allow calling
//                     logic within browser
//
//------------------------------------------------------------------------------
// Function flow:      if not in postedit mode
//                        Call services (EQFGETDICT with EQF_SCROLL_UP)
//                     else
//                        beep
//                     endif
//
//------------------------------------------------------------------------------
VOID
EQFBDictPrefixUp  ( PTBDOCUMENT pDoc  )       // scroll dictionary prefixes up
{
   if ( !pDoc->EQFBFlags.PostEdit )
   {
      EQFGETDICT ( EQF_SCROLL_PREFIX_UP, NULL );
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no dictionary window
   } /* endif */

}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBDictPrefixDown - scroll the dictionary assigned prefixes down
//------------------------------------------------------------------------------
// Function call:     EQFBDictPrefixDown( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:        This function will re-assign the dictionary prefixes down
//                     to the next terms.
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT       -  pointer to document instance area
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:       primitive function but necessary to allow calling
//                     logic within browser
//
//------------------------------------------------------------------------------
// Function flow:      if not in postedit mode
//                        Call services (EQFGETDICT with EQF_SCROLL_UP)
//                     else
//                        beep
//                     endif
//
//------------------------------------------------------------------------------
VOID
EQFBDictPrefixDown  ( PTBDOCUMENT pDoc  )       // scroll dictionary prefixes down
{
   if ( !pDoc->EQFBFlags.PostEdit )
   {
      EQFGETDICT ( EQF_SCROLL_PREFIX_DOWN, NULL );
   }
   else
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no dictionary window
   } /* endif */

}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBDocIsTranslated
//------------------------------------------------------------------------------
// Function call:     EQFBDocIsTranslated( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:         This function will inform the user and MAT Tools that
//                      document is translated.
//------------------------------------------------------------------------------
// Parameters:           PTBDOCUMENT       pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:          - set flag that document is translated
//                         - post message to MAT Tools that doc is translated
//                           if possible
//                         - display message that document is translated
//                         - set post edit mode
//------------------------------------------------------------------------------
VOID EQFBDocIsTranslated( PTBDOCUMENT pDoc )
{
   PSTEQFGEN  pstEQFGen;                     // pointer to generic structure
   PSZ        pData;                         // pointer to data
   HWND       hwndTemp;                      // temp. window handle

   pDoc->fXlated = TRUE;                     // set translated
   pDoc->Redraw |= REDRAW_ALL;               // indicate to update all of screen
   EQFBScreenData( pDoc );                   // force screen update
   EQFBScreenCursor( pDoc );                 // position cursor and slider

   hwndTemp = pDoc->hwndFrame;
   pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );

   if ( pDoc )
   {
     pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
     if ( pstEQFGen )
     {
        WinSendMsg( pstEQFGen->hwndTWBS, EQFM_DOC_IS_XLATED, NULL, NULL);
        if ( pstEQFGen->szLongName[0] != EOS )
        {
          pData     = (PSZ)pstEQFGen->szLongName;
        }
        else
        {
          pData     = (PSZ)pstEQFGen->szFileName;
        } /* endif */
      OEMTOANSI( pData );
       UtlErrorHwnd( TB_DOCXLATED, MB_OK, 1, &pData, EQF_INFO, hwndTemp);
      ANSITOOEM( pData );

     } /* endif */
     EQFBSetPostEdit( pDoc );                   // set post edit mode
   } /* endif */
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBTInitTrans
//------------------------------------------------------------------------------
// Function call:     EQFBTInitTrans( PTBDOCUMENT, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:         This function will initially position the starget
//                      and the ssource file
//                      It will position it either on the CURRENT tag or
//                      on the first NONE translated tag.
//                      If none is found user and MAT Tools will be informed
//                      that document is already translated.
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       - pointer to document instance
//                    PUSHORT           - segment number where to start with
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE :  start of segment found
//                    FALSE:  document already translated
//
//------------------------------------------------------------------------------
// Side effects:         SOURCE_DOC and TARGET_DOC will be positioned
//
//------------------------------------------------------------------------------
// Function flow:     - Scan for the current tag starting from the top of
//                      the document
//                    - if not found scan for the first to be translated
//                      segment
//                    - if no untranslated segment found inform user and
//                      MAT Tools that document is already translated and
//                      init the active segment structure
//                    - if segment found set segment number of active segment .
//
//------------------------------------------------------------------------------
static
BOOL EQFBTInitTrans
(
   PTBDOCUMENT  pDoc,                     // pointer to document instance
   PULONG       pulSegNum )               // pointer to segment number
{
   BOOL fOK = TRUE;                       // success indicator
   PTBSEGMENT  pSeg;                      // pointer to segment struct.
   ULONG       ulSegNum;                  // segment number

   ulSegNum = 1;
   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);        // reset to begin

   while ( pSeg && !pSeg->SegFlags.Current)
   {
      ulSegNum++;                         // point to next segment
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
   } /* endwhile */

   if ( !pSeg        )                      // not found - end of table reached
   {
      ulSegNum = 1;                         // reset to begin
      pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
      while ( pSeg && pSeg->qStatus != QF_TOBE && pSeg->qStatus != QF_ATTR)
      {
         ulSegNum ++;
         pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
      } /* endwhile */
   } /* endif */

   if ( pSeg && pSeg->pDataW )               // start point found
   {
      *pulSegNum = pSeg->ulSegNum;          // set return position
      pSeg->SegFlags.Current = FALSE;       // reset current flag
      if ( pSeg->qStatus == QF_CURRENT )    // get rid of current....
      {
         pSeg->qStatus = QF_TOBE;           // it's to be
      } /* endif */
      if ( pDoc->hwndRichEdit )
      {
        EQFBSetWorkSegRTF( pDoc, ulSegNum, pSeg->pDataW );
      }
   }
   else                                         // document translated
   {
      if ( !pDoc->fXlated )                     // set translated
      {
         EQFBDocIsTranslated( pDoc );
         *pulSegNum = 1;                        // set return position
         fOK = FALSE;                           // do not proceed with proc.

         // set document completion rate to 100%
         WinSendMsg( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS, EQFM_DOC_STATUS, (WPARAM)MP1FROMSHORT(EQF_DOC_COMPLRATE), (LPARAM)MP2FROMSHORT ( 100 ) );

      }
      else
      {
         *pulSegNum = pDoc->TBCursor.ulSegNum;  // activate the cursor pos
      } /* endif */
   } /* endif */


#ifdef TRANTIME

     fStream = fopen("data","w");
     fprintf(fStream,"GetProp  SaveProp TimeDelta\n");
     ulBegin = pGlobInfoSeg->msecs;
#endif

    return ( fOK );
}
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBTSegNext   - activate next untranslated segment
//------------------------------------------------------------------------------
// Function call:     EQFBTSegNext( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       This function will send (if necessary) the
//                    last segment to the TM (Save) and activates
//                    the next untranslated one
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:      sSource and sTarget will be positioned
//
//------------------------------------------------------------------------------
// Function flow:     call EQFBTrans with the position flag set to
//                     POS_TOBE
//------------------------------------------------------------------------------
VOID
EQFBTSegNext ( PTBDOCUMENT pDoc )       // pointer to document ida
{
   EQFBTrans( pDoc, POS_TOBE ) ;        // position at untranslated ones
   return ;
}
VOID
EQFBTSegNextExact ( PTBDOCUMENT pDoc )  // pointer to document ida
{
   EQFBTrans( pDoc, POS_TOBE_EXACT ) ;  // position at untranslated ones with EXACT matches
   return ;
}
VOID
EQFBTSegNextFuzzy ( PTBDOCUMENT pDoc )  // pointer to document ida
{
   EQFBTrans( pDoc, POS_TOBE_FUZZY ) ;  // position at untranslated ones with FUZZY matches
   return ;
}
VOID
EQFBTSegNextNone ( PTBDOCUMENT pDoc )   // pointer to document ida
{
   EQFBTrans( pDoc, POS_TOBE_NONE ) ;   // position at untranslated ones with NO matches
   return ;
}
VOID
EQFBTSegNextMT ( PTBDOCUMENT pDoc )   // pointer to document ida
{
   EQFBTrans( pDoc, POS_TOBE_MT ) ;   // position at untranslated ones with MT matches
   return ;
}
VOID
EQFBTSegNextGlobal ( PTBDOCUMENT pDoc )   // pointer to document ida
{
   EQFBTrans( pDoc, POS_TOBE_GLOBAL ) ;   // position at untranslated ones with GLOBAL MEMORY matches
   return ;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBTSeg
//------------------------------------------------------------------------------
// Function call:     EQFBTSeg( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       This function will send (if necessary) the last
//                    segment to the TM (Save) and activates the next
//                     segment even if it is already translated
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT         pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Side effects:      sSource and sTarget will be positioned
//------------------------------------------------------------------------------
// Function flow:     -  call EQFBTrans with position flag set to
//                       POS_TOBEORDONE
//
//------------------------------------------------------------------------------
VOID
EQFBTSeg ( PTBDOCUMENT pDoc     )             // pointer to document ida
{
   EQFBTrans( pDoc, POS_TOBEORDONE );         // pos at translated ones, too
   return ;
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBTrans
//------------------------------------------------------------------------------
// Function call:     EQFBTrans( PTBDOCUMENT );
//------------------------------------------------------------------------------
// Description:       This function will send (if necessary) the last
//                    segment to the TM and activates the next one
//                    dependent on the mode.
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT         pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:      sSource and sTarget will be positioned
//
//------------------------------------------------------------------------------
// Function flow:     - check if user moved out of active segment
//                      or not (EQFBTCheckPos),
//                    - reset post edit mode
//                    i.e. 'normal' operation (just pressed Ctrl-Enter)
//                    case XLATESEG:
//                      - set translation tag
//                      - save segment (translation together with
//                        source) in TM
//                      - find next segment to be translated or inform
//                        WorkBench
//                        if found set it current and send it to Services
//                        ( FOREGROUND)
//                        Find next and send it to services (BACKGROUND)
//                    case MOVE          -- user moved unexpected
//                      - inform user if he had changed something;
//                        let him decide what to do
//                        Save or ignore changes depending on user decision
//                      - find source segment of new position;
//                        send it to services;
//                        activate it
//                    case FIRST
//                      - find next segment to be translated
//                          (call EQFBTInitTrans)
//                        if found set it current and send it to Services
//                          (FOREGROUND)
//                        Find next and send it to services (BACKGROUND)
//
//------------------------------------------------------------------------------
VOID EQFBTrans
(
  PTBDOCUMENT pDoc,                       // pointer to document ida
  USHORT      usCond                      // start condition
)
{
   ULONG     ulSegNum;                    // segment number to start with
   USHORT    usResult;                    // return value from UtlError
   BOOL      fOK = TRUE;                  // success indicator
   PTBSEGMENT pSeg;                       // pointer to segment
   PTBSEGMENT pSourceSeg;                 // pointer to source segment
   USHORT    usStartPos;                  // start position
   PSZ_W     pData;                       // pointer to data
   HWND      hwndTemp;

   usStartPos = EQFBTCheckPos( pDoc );    // find start condition
   hwndTemp = pDoc->hwndFrame;

//   pDoc->EQFBFlags.PostEdit = FALSE;      // reset post edit mode /* @39D */
   pDoc->EQFBFlags.AutoMode  = FALSE;     // reset automatic mode

   EQFBFuncMarkClear ( pDoc );            //clear old mark

   switch ( usStartPos )
   {
      case CHECKPOS_FIRST:
         pDoc->EQFBFlags.PostEdit = FALSE;      // reset post edit  /* @39A */
         if (EQFBTInitTrans ( pDoc, &ulSegNum )) // find where to start
         {
            EQFBDoNextTwo ( pDoc,
                            &ulSegNum, usCond ); // do the next two segments
         }  /* endif */
         break;

      case CHECKPOS_POSTEDIT:
         WriteSaveSegLog( "EQFBTrans(CHECKPOS_POSTEDIT): Before EQFBSaveSeg" );
         if ( pDoc->EQFBFlags.workchng )
         {
            fOK = EQFBSaveSeg( pDoc );
            if ( fOK )
            {
              WriteSaveSegLog( "EQFBTrans(CHECKPOS_POSTEDIT): After EQFBSaveSeg" );
            }
            else
            {
              WriteSaveSegLog( "EQFBTrans(CHECKPOS_POSTEDIT): EQFBSaveSeg failed" );
            } /* endif */
         }
         else
         {
            EQFBWorkSegOut( pDoc );
         } /* endif */
         if ( usCond == POS_TOBEORDONE )
         {
            usCond = POS_CURSOR;                 // position at cursor
         } /* endif */
         if ( fOK )
         {
           pDoc->EQFBFlags.PostEdit = FALSE;      // reset post edit/* @39A */
           ulSegNum = pDoc->TBCursor.ulSegNum;   // set segment number
           WriteSaveSegLog( "EQFBTrans(CHECKPOS_POSTEDIT): before EQFBDoNextTwo" );
           EQFBDoNextTwo ( pDoc, &ulSegNum, usCond );  // do the next two segments
           WriteSaveSegLog( "EQFBTrans(CHECKPOS_POSTEDIT): after EQFBDoNextTwo" );
         } /* endif */
         break;


      case CHECKPOS_XLATESEG:
         if (pDoc->fAutoSpellCheck )
         {
           WriteSaveSegLog( "EQFBTrans(CHECKPOS_XLATESEG): EQFBWorkThreadTask-THREAD_SPELLSEGMENT" );
           EQFBWorkThreadTask ( pDoc, THREAD_SPELLSEGMENT );
         } /* endif */
         WriteSaveSegLog( "EQFBTrans(CHECKPOS_XLATESEG): Before EQFBSaveSeg" );
         if (EQFBSaveSeg( pDoc))                 // save current segment
         {
            WriteSaveSegLog( "EQFBTrans(CHECKPOS_XLATESEG): After EQFBSaveSeg, before EQFBDoNextTwo" );
            pDoc->EQFBFlags.PostEdit = FALSE;    // reset post edit /* @39A */
                                                 // store start address
            ulSegNum = pDoc->TBCursor.ulSegNum + 1;
            EQFBDoNextTwo ( pDoc, &ulSegNum , usCond  );// do the next two segments
            WriteSaveSegLog( "EQFBTrans(CHECKPOS_XLATESEG): After EQFBDoNextTwo" );
         }
         else
         {
           WriteSaveSegLog( "EQFBTrans(CHECKPOS_XLATESEG): EQFBSaveSeg failed" );
         } /* endif */
         break;

      case CHECKPOS_MOVE:
         if ( usCond == POS_TOBEORDONE )
         {
            usCond = POS_CURSOR;                // position at cursor
         } /* endif */

                  // issue warning determining if user wants to save or not
         usResult = MBID_NO;                    // ignore segment if nothing ch.
                                                // something is changed
         if ( pDoc->fFuzzyCopied || pDoc->EQFBFlags.workchng || MDCommentHasChanged( pDoc ) )
         {
            usResult = UtlError( TB_CHANGESEGMENT, MB_YESNOCANCEL, 0, NULL, EQF_QUERY);


            // refresh metadata for current segment
            {
              PTBSEGMENT  pCurSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum);
              MDGetMetadata( pDoc, pCurSeg, (usResult == MBID_YES ) );
            }
            pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );
         }
         else
         {
            PTBSEGMENT  pCurSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum);
            MDGetMetadata( pDoc, pCurSeg, (usResult == MBID_YES ) );
         } /* endif */

         if ( pDoc )
         {
           switch ( usResult )
           {
             case MBID_YES:
                if (EQFBSaveSeg( pDoc ))         // save current segment
                {
                   ulSegNum = (pDoc->TBCursor).ulSegNum; // store start addr
                   EQFBDoNextTwo ( pDoc,                 // do the next two
                                   &ulSegNum , usCond  );// untranslated ?
                } /* endif */
                break;
             case MBID_NO:                            // ignore changes
                pSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum); // get seg
                                                      // get old status
                pSeg->qStatus = pDoc->tbActSeg.qStatus;
                pSeg->SegFlags.Current = FALSE;
                pSeg->SegFlags.Expanded = FALSE;
                if ( pSeg->qStatus != QF_XLATED)
                {
                  pSeg->SegFlags.Typed = FALSE;

                  pSeg->SegFlags.Copied = FALSE;
                  pSeg->usModWords = 0;
                  memset(&pSeg->CountFlag, 0, sizeof( pSeg->CountFlag));
                }
                if ( pDoc->pSaveSegW )                           /* @TrapA */
                {                                               /* @TrapA */
                  pSeg->pDataW = pDoc->pSaveSegW;
                } /* endif */                                   /* @TrapA */
                // if untranslate active,  use original segment
                // and copy it as source of translation
                if ( pSeg->SegFlags.UnTrans )    // untranslate active
                {
                   pSourceSeg = EQFBGetSegW(pDoc->twin, pDoc->tbActSeg.ulSegNum);

                   if ( UtlAlloc( (PVOID *)&pData, 0L,
                                (LONG) max((pSourceSeg->usLength+1) * sizeof(CHAR_W),
                                            MIN_ALLOC),
                                 ERROR_STORAGE) )
                   {
                      UtlAlloc( (PVOID *) &(pSeg->pDataW) , 0L, 0L, NOMSG );
                      pSeg->pDataW = pData;
                      memcpy( (PBYTE)pData,(PBYTE)pSourceSeg->pDataW,
                               sizeof(CHAR_W) * pSourceSeg->usLength );
                      *(pData + pSourceSeg->usLength) = EOS;
                   } /* endif */
                   pSeg->SegFlags.UnTrans = FALSE;         // reset untrans flag
                } /* endif */

                pDoc->EQFBFlags.workchng = FALSE;     // no change in work seg
                EQFBCompSeg( pSeg );
                if ( pDoc->hwndRichEdit )
                {
                  EQFBSetWorkSegRTF( pDoc, pDoc->tbActSeg.ulSegNum, pSeg->pDataW );
                }

                pDoc->pSaveSegW = NULL;                 // reset save seg
                ulSegNum = pDoc->TBCursor.ulSegNum;    // store start address
                EQFBDoNextTwo ( pDoc,
                                &ulSegNum , usCond  ); // do the next two segs
                break;
             default:                                  // set back to active seg
  //              EQFBGotoSeg( pDoc, pDoc->tbActSeg.ulSegNum, 0);
                break;

           } /* endswitch */
         } /* endif */
         break;
      default:
         WinAlarm( HWND_DESKTOP, WA_ERROR);  // should not occur but listen...
         break;
   } /* endswitch */

   pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );
   if ( pDoc )
   {
     pDoc->Redraw |= REDRAW_ALL;               // indicate to redraw everything
   } /* endif */

   return ;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBDoNextTwo
//------------------------------------------------------------------------------
// Function call:     EQFBDoNextTwo( PTBDOCUMENT, PUSHORT, USHORT );
//
//------------------------------------------------------------------------------
// Description:       find the next two (untranslated) segments (if
//                    available) and send them to the services
//                    the first in FOREGROUND mode, the later in
//                    BACKGROUND mode for processing
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    -- pointer to document instance data
//                    PUSHORT        -- the segment number where to start from
//                                      will contain next active segment
//                    USHORT         -- type of segment to be searched for
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Samples:           EQFBDoNextTwo( pDoc, &ulSegNum, POS_TOBE);
//                       this call will look only for segments TOBE transl.
//                    EQFBDoNextTwo( pDoc, &ulSegNum, POS_TOBEORDONE);
//                       this call will look only for segments TOBE transl.
//------------------------------------------------------------------------------
// Function flow:     - send next segment to services as foreground request
//                    - if ok activate current segment and send
//                       next-next segment to services
//
//------------------------------------------------------------------------------
VOID EQFBDoNextTwo( PTBDOCUMENT pDoc,     // pointer to document
                    PULONG  pulSegNum,    // segment number where to start
                    USHORT  usCond )      // what to search for
{
   ULONG    ulSegNum;                     // segment number where to start
   BOOL fOK = TRUE;                       // succes indicator


   WriteSaveSegLog( "EQFBDoNextTwo: before EQFBSendNextSource" );
   ulSegNum =  *pulSegNum;
   // send next segment to services
   fOK = EQFBSendNextSource(pDoc,         // pointer to document
                            &ulSegNum,    // pointer to new segment
                            TRUE,         // send in foreground mode
                            usCond);      // untranslated only?
   if ( fOK )
   {
       WriteSaveSegLog( "EQFBDoNextTwo: before EQFBActivateSegm" );
       EQFBActivateSegm( pDoc, ulSegNum );// activate the current segment
       WriteSaveSegLog( "EQFBDoNextTwo: after EQFBActivateSegm" );
       *pulSegNum = ulSegNum;            // that's our active segment

       ulSegNum ++;
       WriteSaveSegLog( "EQFBDoNextTwo: before EQFBSendNextSource" );
       fOK = EQFBSendNextSource
                 ( pDoc,                  // pointer to document
                   &ulSegNum,             // pointer to line number
                   FALSE,                 // background mode
                   usCond );              // untranslated only?
       WriteSaveSegLog( "EQFBDoNextTwo: after EQFBSendNextSource" );
   } 
   else
   {
     WriteSaveSegLog( "EQFBDoNextTwo: EQFBSendNextSource failed" );
   } /* endif */
    return;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBActivateSegm
//------------------------------------------------------------------------------
// Function call:     EQFBActivateSegm( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       do the handling for activating a segment.
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT   -- pointer to document instance
//                    USHORT        -- segment to activate
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Side effects:      Screen redisplay will be forced;
//                    active segment structure will be set
//
//------------------------------------------------------------------------------
// Function flow:     - position at requested segment(in source and target)
//                    - update worksegment
//                    - init fFuzzycopied (to false)
//                    - fill segment structure
//                    - set the current attribute
//                    - force a repaint of the screen and position
//                      the cursor
//
//------------------------------------------------------------------------------
VOID  EQFBActivateSegm
(
   PTBDOCUMENT pDoc,                       // pointer to document instance
   ULONG       ulSegNum                    // segment number
)
{
    PTBSEGMENT     pSourceSeg;               // pointer to segment in source document
    PTBSEGMENT     pSeg;                     // pointer to segment
    ULONG          ulSrcWords = 0L;
    ULONG          ulSrcMarkUp = 0L;
    PDOCUMENT_IDA  pIdaDoc;                  // pointer to document struct.
    USHORT         usRc;
    PSZ_W          pTempBuf;                 // temp pointer
    QSTATUS        qOldStatus;

    if ( UtlAlloc( (PVOID *)&pTempBuf, 0L, 2L * MAX_PROPLENGTH * sizeof(CHAR_W), ERROR_STORAGE) )
    {
      EQFBGotoSeg( pDoc, ulSegNum, 0);       // position at this segment
      if ( WinIsWindowVisible( pDoc->twin->hwndFrame ) )
      {
         EQFBGotoSeg( pDoc->twin, ulSegNum, 0 );// position source document
      } /* endif */

      EQFBWorkSegIn( pDoc );                 // copy contents of current segment
      pDoc->fFuzzyCopied = FALSE;            // no fuzzy match right now
      pDoc->sPropCopied = -1;				 // no proposal copied right now
      pSeg = EQFBGetSegW(pDoc, ulSegNum);     // get seg
      memcpy (&(pDoc->tbActSeg),
              pSeg, sizeof(TBSEGMENT));

      qOldStatus = (QSTATUS)pSeg->qStatus;
      pSeg->qStatus = QF_CURRENT;                  // it's the active segment
      pSeg->SegFlags.Current = TRUE;         // it's the active segment
      if ( pDoc->hwndRichEdit )
      {
        USHORT usOffset = 0;
        BYTE b = pDoc->pDispFileRTF->bRTFFill;
        pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
        EQFBSetWorkSegRTF( pDoc, ulSegNum, pSeg->pDataW );
        EQFBNextUnprotected( pDoc, pSeg, &usOffset );
        EQFBGotoSegRTF( pDoc, ulSegNum, usOffset);     // position at this segment
        SendMessage( pDoc->hwndRichEdit, EM_SETMODIFY, 0, 0L ); // nothing changed
        pDoc->pDispFileRTF->bRTFFill = b;
      }
      else
      {
        pDoc->Redraw |= REDRAW_ALL;                  // redraw screen
      }
      /******************************************************************/
      /* set cursor state at insert if requested ...                    */
      /******************************************************************/
      if ( pDoc->pUserSettings->fCrsInsert )
      {
        if ( pDoc->hwndRichEdit && !pDoc->EQFBFlags.inserting )
        {
          EQFBToggleInsertRTF( pDoc );
        } /* endif */
        pDoc->EQFBFlags.inserting = TRUE;
        EQFBScreenCursorType( pDoc );
      } /* endif */
      if (pSeg->usSrcWords == 0 )
      {
        pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
        pSourceSeg = EQFBGetSegW( pDoc->twin, ulSegNum);
        usRc = EQFBWordCntPerSeg(
                     (PLOADEDTABLE)pDoc->pDocTagTable,
                     (PTOKENENTRY) pDoc->pTokBuf,
                     pSourceSeg->pDataW,
                     pIdaDoc->sSrcLanguage,
                     &ulSrcWords, &ulSrcMarkUp,
                     pDoc->twin->ulOemCodePage);
        if (!usRc)
        {
          pSeg->usSrcWords = (USHORT) ulSrcWords;
        } /* endif */
      } /* endif */
      /***************************************************************/
      /* get the prefixes of all proposals and use the best          */
      /***************************************************************/
      if ( (qOldStatus == QF_TOBE) || (qOldStatus == QF_ATTR)
           || (qOldStatus == QF_CURRENT )  )
      {
        USHORT  usState = 0;
        ULONG   ulFuzzyness = 0;
        usState = EqfGetPropState((PSTEQFGEN)pDoc->pstEQFGen, 0);

        if ( usState )
        {
          // GQ: fix for P019381:
          // new order of match handling: Exact, Replace, Fuzzy, Machine

          if ( usState & EXACT_PROP )
          {
            pSeg->CountFlag.ExactExist = TRUE;
          }
          else if ( (usState & GLOBMEM_TRANS_PROP ) || (usState & GLOBMEMSTAR_TRANS_PROP) ) 
          {
            if ( (usState & FUZZY_REPLACE_PROP ) || usState & FUZZY_PROP  )
            {
              pSeg->CountFlag.FuzzyExist = TRUE;
            } /* endif */               
            pSeg->CountFlag.GlobMemExist = TRUE;
          }
          else if (usState & REPLACE_PROP )
          {
            pSeg->CountFlag.ReplExist = TRUE;
          }
          else if (usState & FUZZY_REPLACE_PROP )
          {
            pSeg->CountFlag.FuzzyExist = TRUE;
            // if set, situation with one "rf" match cannot be distinguished
            // from case with 2 matches, one "r" and one "f" match.
            // Fix for P019381: do not set pSeg->CountFlag.ReplExist = TRUE;
          }
          else if (usState & GLOBMEMFUZZY_TRANS_PROP )
          {
            // count global memory fuzzzies as normal fuzzy matches
            pSeg->CountFlag.FuzzyExist = TRUE;
          }
          else if (usState & FUZZY_PROP )
          {
            pSeg->CountFlag.FuzzyExist = TRUE;
          }
          else if (usState & MACHINE_TRANS_PROP )
          {
            pSeg->CountFlag.MachExist = TRUE;
          }
          else
          {
            /**********************************************************/
            /* should not occur                                       */
            /**********************************************************/
          } /* endif */
          if (pSeg->CountFlag.FuzzyExist)
          {
            // get fuzziness of first fuzzy proposal
            {
              USHORT i = 0;
              ulFuzzyness = 0;
              PSTEQFGEN pstEqfGen = (PSTEQFGEN)pDoc->pstEQFGen;
              PDOCUMENT_IDA pDocIda = (PDOCUMENT_IDA)pstEqfGen->pDoc;
              while ( (ulFuzzyness == 0) && (i < EQF_NPROP_TGTS) )
              {
                USHORT usCurFuzzy = (pDocIda->stEQFSab + pDocIda->usFI)->usFuzzyPercents[i];
                if ( usCurFuzzy < 100 ) ulFuzzyness = usCurFuzzy;
                i++;
              } /* endwhile */
            }

					  if (ulFuzzyness < (FUZZY_THRESHOLD_0 * 100))
					  { // RJ-040324: DO NOT COUNT AS FUZZY, COUNT as NON_MATCH!!
					    // no flag to be set here, instead reset CountFlag.FuzzyExist!!!
					    // force that it is counted as NOne_MATCH
					    pSeg->CountFlag.FuzzyExist = FALSE;

              // GQ 2015-06-17 Check if there is a MT match
              if (usState & MACHINE_TRANS_PROP )
              {
                pSeg->CountFlag.MachExist = TRUE;
  		     		}
		     		}
					  else if (ulFuzzyness <= (FUZZY_THRESHOLD_1 * 100 ))
 					  {
						  pSeg->CountFlag.Fuzzy5070 = TRUE;
					  }
					  else if (ulFuzzyness <= (FUZZY_THRESHOLD_2 * 100))
					  {
						  pSeg->CountFlag.Fuzzy7190 = TRUE;
					  }
					  else
					  {
						  pSeg->CountFlag.Fuzzy9199 = TRUE;
					  }
 		      } /* endif */
        } /* endif */
      } /* endif */
      if ((pSeg->usModWords == 0 )
             && ((qOldStatus == QF_TOBE) || (qOldStatus == QF_ATTR)
              || (qOldStatus == QF_CURRENT)) )
      {
          EQFGETPROPW( EQF_GETLCS, pTempBuf, &(pSeg->usModWords) );
          /**************************************************************/
          /* if usModWOrds still 0 and not an exact prop: then no       */
          /* prop exists and usModWords must be set here                */
          /**************************************************************/
          if ( (!pSeg->CountFlag.ExactExist) && (! pSeg->CountFlag.MachExist)
               && (pSeg->usModWords == 0)  )
          {
            pSeg->usModWords = pSeg->usSrcWords;
          } /* endif */
          /************************************************************/
          /* force usModWords <= usSrcWords; reason:                  */
          /*      usModWords counts all tokens                        */
          /* whereas usSrcWords ignores NEWSENTENCE, NOLOOKUP         */
          /* and NUMBERS                                              */
          /************************************************************/
          if (pSeg->usModWords > pSeg->usSrcWords )
          {
            pSeg->usModWords = pSeg->usSrcWords;
          } /* endif */
      } /* endif */

      // let EQFCHECKSEGTYPE function of the user exit adjust the counting type of this segment
      if ( pDoc && pDoc->pfnCheckSegType )
      {
         PTBSEGMENT pTBPrevSourceSeg = EQFBGetSegW( pDoc->twin, ulSegNum - 1 );
         PTBSEGMENT pSourceSeg = EQFBGetSegW( pDoc->twin, ulSegNum );
         EQF_BOOL  fChanged = FALSE;
         PSZ_W     pszPrevSegData = NULL;
         PSZ_W     pszSourceSegData = NULL;
         SHORT     sMemType = CST_MANUAL_MATCH;
         SHORT     sCountType = CST_UNDEFINED_MATCH;
         PSTEQFGEN pstEqfGen = (PSTEQFGEN)pDoc->pstEQFGen;
         PDOCUMENT_IDA pDocIda = (PDOCUMENT_IDA)pstEqfGen->pDoc;

     
         pszPrevSegData = ( pTBPrevSourceSeg ) ? pTBPrevSourceSeg->pDataW : NULL;
         pszSourceSegData = ( pSourceSeg ) ? pSourceSeg->pDataW : NULL;
         fChanged = (pDoc->pfnCheckSegType)( pDocIda->szDocFormat, pszPrevSegData, pszSourceSegData, pSeg->pDataW, (LONG)pDoc, ulSegNum, &sCountType, &sMemType );
         if ( fChanged )
         {
           switch( sCountType )
           {
             case CST_AUTOSUBST_MATCH: 
               pSeg->CountFlag.FuzzyExist = FALSE; 
               pSeg->CountFlag.Fuzzy5070 = FALSE; 
               pSeg->CountFlag.Fuzzy7190 = FALSE; 
               pSeg->CountFlag.Fuzzy9199 = FALSE; 
               pSeg->CountFlag.ReplExist = FALSE; 
               pSeg->CountFlag.ExactExist = FALSE; 
               pSeg->CountFlag.GlobMemExist = FALSE;
               pSeg->CountFlag.MachExist = FALSE; 
               pSeg->CountFlag.AnalAutoSubst = TRUE; 
               break;
             case CST_MACHINE_MATCH: 
               pSeg->CountFlag.FuzzyExist = FALSE; 
               pSeg->CountFlag.Fuzzy5070 = FALSE; 
               pSeg->CountFlag.Fuzzy7190 = FALSE; 
               pSeg->CountFlag.Fuzzy9199 = FALSE; 
               pSeg->CountFlag.ReplExist = FALSE; 
               pSeg->CountFlag.ExactExist = FALSE; 
               pSeg->CountFlag.GlobMemExist = FALSE;
               pSeg->CountFlag.MachExist = TRUE; 
               break;
             case CST_EXACT_MATCH: 
               pSeg->CountFlag.FuzzyExist = FALSE; 
               pSeg->CountFlag.Fuzzy5070 = FALSE; 
               pSeg->CountFlag.Fuzzy7190 = FALSE; 
               pSeg->CountFlag.Fuzzy9199 = FALSE; 
               pSeg->CountFlag.ReplExist = FALSE; 
               pSeg->CountFlag.ExactExist = TRUE; 
               pSeg->CountFlag.GlobMemExist = FALSE;
               pSeg->CountFlag.MachExist = FALSE; 
               break;
             case CST_NO_MATCH: 
               pSeg->CountFlag.FuzzyExist = FALSE; 
               pSeg->CountFlag.Fuzzy5070 = FALSE; 
               pSeg->CountFlag.Fuzzy7190 = FALSE; 
               pSeg->CountFlag.Fuzzy9199 = FALSE; 
               pSeg->CountFlag.ReplExist = FALSE; 
               pSeg->CountFlag.ExactExist = FALSE; 
               pSeg->CountFlag.GlobMemExist = FALSE;
               pSeg->CountFlag.MachExist = FALSE; 
               break;
             case CST_GLOBMEM_MATCH: 
               pSeg->CountFlag.FuzzyExist = FALSE; 
               pSeg->CountFlag.Fuzzy5070 = FALSE; 
               pSeg->CountFlag.Fuzzy7190 = FALSE; 
               pSeg->CountFlag.Fuzzy9199 = FALSE; 
               pSeg->CountFlag.ReplExist = FALSE; 
               pSeg->CountFlag.ExactExist = FALSE; 
               pSeg->CountFlag.GlobMemExist = TRUE;
               pSeg->CountFlag.MachExist = FALSE; 
               break;
             case CST_UNDEFINED_MATCH: 
             default: 
               break;
           }
         } /* endif */
      } /* endif */


       /*****************************************************************/
       /* init logging structure                                        */
       /*****************************************************************/
       memset( &(pDoc->ActSegLog), 0, sizeof(pDoc->ActSegLog));
       pDoc->ActSegLog.ulTime = GetTickCount();
       pDoc->ActSegLog.ulTotalTime = GetTickCount();
       pDoc->ActSegLog.ulSegNum = ulSegNum;
       pDoc->ActSegLog.VersionInfoNew.Version = EQF_DRIVER_VERSION;
       pDoc->ActSegLog.VersionInfoNew.Release = EQF_DRIVER_RELEASE;
       pDoc->ActSegLog.VersionInfoNew.Update = EQF_DRIVER_SUBRELEASE;
       pDoc->ActSegLog.VersionInfoNew.Driver = EQF_DRIVER_BUILD;
       pDoc->ActSegLog.AddFlags.ComputeThinkTime = TRUE;

       // get match flags from available proposals
       EQFBGetMatchFlags( pDoc );  
       
       pDoc->ActSegLog.usWordCnt            = pSeg->usSrcWords;


      { PTBDOCUMENT   pTRNoteDoc;
        pTRNoteDoc = pDoc->next;

        while ((pTRNoteDoc->docType != TRNOTE_DOC) && (pTRNoteDoc != pDoc ))
        {
          pTRNoteDoc = pTRNoteDoc->next;
        } /* endwhile */
        if ((pTRNoteDoc->docType == TRNOTE_DOC ) &&
              WinIsWindowVisible( pTRNoteDoc->hwndFrame ))
        {
          EQFBFuncOpenTRNote(pDoc);
        } /* endif */

        {
          PSZ_W pszContext = EQFBGetContext( pDoc, pSeg, ulSegNum );
          pDoc->szContextBuffer[0] = 0;

          if ( (pszContext != NULL) && (*pszContext != 0) && (pDoc->pfnFormatContext != NULL) )
          {
            (pDoc->pfnFormatContext)( pszContext, pDoc->szContextBuffer );
          } /* endif */             

          MDRefreshMetadata( pDoc, pSeg, pDoc->szContextBuffer );
        }
      }

      /****************************************************************/
      /* free allocated resource                                      */
      /****************************************************************/
      UtlAlloc( (PVOID *)&pTempBuf, 0L, 0L, NOMSG );
    } /* endif */

}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBSendNextSource
//------------------------------------------------------------------------------
// Function call:     EQFBSendNextSource( PTBDOCUMENT, PUSHORT, BOOL, USHORT );
//
//------------------------------------------------------------------------------
// Description:       find the next (untranslated) segment and send it in
//                    the passed mode to the services
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  - pointer to docu structure
//                    PUSHORT      - the segment number where to start from
//                    BOOL         - mode of operation (FOREGROUND/BACKGROUND)
//                    USHORT       - what to search for
//------------------------------------------------------------------------------
// Returncode type:   BOOL         - success indicator
//------------------------------------------------------------------------------
// Returncodes:       TRUE    source segment found
//                    FALSE   nothing found (translation completed)
//                            or error happened
//------------------------------------------------------------------------------
// Side effects:      pulSegNum contains the active segment number on return
//
//------------------------------------------------------------------------------
// Function flow:     - scan source doc and find next untranslated segment
//                      if fFound than already positioned at correct
//                      start point
//                    - if not found and foreground mode inform user that
//                      nothing else remains to be translated
//                    - if foreground and EOF reached, ask user whether to
//                      set postedit or go on as usual
//                    - get it and send it to services in the requested mode
//
//------------------------------------------------------------------------------
BOOL
EQFBSendNextSource( PTBDOCUMENT pDoc,     // pointer to document instance
                    PULONG  pulSegNum,    // number of segment
                    BOOL    fMode,        // mode of operation
                    USHORT  usCond)       // search condition
{
   BOOL   fOK = TRUE;                     // true so far
   USHORT usRc = EQFRC_OK;                // return codes
   PTBSEGMENT pTBSourceSeg;               // pointer to source segment
   USHORT usMatchFound;                   // number of matches
   BOOL   fEndReached;                    // FALSE if not at end of file
   USHORT usMBId;                         // result of Utlerror
   USHORT fsFlags = 0;                    // configuration flags
   PVOID  pvMetaData = NULL;

   pTBSourceSeg = EQFBFindNextSource( pDoc, pulSegNum, usCond, &fEndReached, fMode, &pvMetaData );
   fOK = (pTBSourceSeg != NULL);          // source segment found

   if ( ! fOK )
   {
      if ( fMode )                        // foreground mode
      {
        EQFBDocIsTranslated( pDoc );      // document already translated
      }
      else
      {
         fOK = FALSE;
      } /* endif */
   }
   else
   {                                       //if foreground mode and
     usMBId = MBID_YES;                    //init return
     if ( fMode && fEndReached)            //if at end,should wrap be done?
     {
       /***************************************************************/
       /* toggle to post edit if user does not want to activate next  */
       /* segment from the beginning of the file                      */
       /***************************************************************/
        usMBId = UtlError( TB_EOFSTILLUNTRANS, MB_YESNO, 0, NULL,
                          EQF_QUERY);
        if ( usMBId == MBID_NO )                     //if not go on
        {
          /************************************************************/
          /* toggle to post edit                                      */
          /************************************************************/
           fOK = FALSE;                          //return:acticate no seg
           pTBSourceSeg = NULL;                  //as if doc is translated
           pDoc->Redraw |= REDRAW_ALL;           // indicate to update all of screen
           EQFBScreenData( pDoc );               // force screen update
           EQFBScreenCursor( pDoc );             // position cursor and slider
           EQFBSetPostEdit( pDoc );              // set post edit mode
        } /* endif */
     } /* endif */

     if ( usMBId == MBID_YES )
     {
       PSZ_W pszContext;

       //go to next segment as usual
#ifdef TRANTIME
        ulStart = pGlobInfoSeg->msecs;
#endif
        fsFlags |= EQFF_MOREPROPINDIC;
        if (pDoc->pUserSettings->UserOptFlags.bAllExactProposals)
        {
          fsFlags |= EQFF_ALLEXACTONES;
        }
        pszContext = EQFBGetContext( pDoc, pTBSourceSeg, pTBSourceSeg->ulSegNum );

        usRc = EQFTRANSSEG3W( pTBSourceSeg->pDataW,     // pointer to seg data
                              pszContext, pvMetaData,
                              pTBSourceSeg->ulSegNum,  // segment number
                              (EQF_BOOL)fMode,         // mode of operation
                              fsFlags,
                              (PSHORT)&usMatchFound);

#ifdef TRANTIME
        ulEnd = pGlobInfoSeg->msecs;
        ulTime += (ulEnd - ulStart);
#endif
        // fill PE data area with results from memory lookup
        memset( &pDoc->PEData, 0, sizeof(pDoc->PEData) );
        if ( usRc == EQFRC_OK )     
        {
          USHORT usProposal = 0;
          PDOCUMENT_IDA  pIdaDoc;                       // pointer to document struct.
          PSTEQFSAB     pstEQFSab;                      // ptr to SAB element
          PSTEQFGEN     pstEQFGen;                      // pointer to generic structure
          pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen ;
          pIdaDoc = (PDOCUMENT_IDA)pstEQFGen->pDoc;
          pstEQFSab = pIdaDoc->stEQFSab + pIdaDoc->usFI; // point to current element

          // copy data of first available MT proposal
          while ( usProposal < pstEQFSab->usPropCount )
          {
            if ( pstEQFSab->usMachineTrans[usProposal] & MACHINE_TRANS_PROP )
            {
              HADDDATAKEY hKey;

              // copy MT proposal target
              wcscpy( pDoc->PEData.szMTTarget, pstEQFSab->pszSortTargetSeg[usProposal] );

              // get match segment ID of MT proposal
              hKey = MADSearchKey( pstEQFSab->pszSortPropsData[usProposal], MATCHSEGID_KEY );
              if ( hKey != NULL ) 
              {
                MADGetAttr( hKey, MATCHSEGID_ATTR, pDoc->PEData.szMTMatchID, sizeof(pDoc->PEData.szMTMatchID) / sizeof(CHAR_W), L"" );
              } /* endif */

              usProposal = pstEQFSab->usPropCount; // force end of while loop
            } /* endif */
            usProposal++;
          } /* endwhile */
        }


        if (usRc != EQFRC_OK )           // && usRc != EQFRC_SEG_NOT_FOUND)
        {
           PSZ pErr = EQFERRINS();         // get error message
//         UtlDispatch();                // allow message box      /* @KIT1177D */
           UtlError( EQFERRID(), MB_CANCEL, 1, &pErr, EQF_ERROR );
           fOK = FALSE;
           /**************************************************************/
           /* stop processing if dictionary could not be activated       */
           /**************************************************************/
           if ( usRc == ERROR_MEM_NOT_ACCESSIBLE )
           {
             // nothing to do, TM is currently in use...
           }
           else if ( (usRc >= EQFS_DA_ERROR) || ( usRc == ERROR_MEM_UNDEFINED ) )
           {
               EQFBFuncClose( (PSTEQFGEN)pDoc->pstEQFGen );     // shut down editor
           } /* endif */
        } /* endif */
     } /* endif */
   } /* endif */
   return ( fOK );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBFindNextSource
//------------------------------------------------------------------------------
// Function call:     EQFBFindNextSource( PTBDOCUMENT, PUSHORT, USHORT, PBOOL);
//
//------------------------------------------------------------------------------
// Description:       find the next (untranslated) segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT -- pointer to document structure
//                    PUSHORT     -- the segment number where to start from
//                    USHORT      -- search condition
//                    PBOOL       -- TRUE if end reached
//
//------------------------------------------------------------------------------
// Returncode type:   PTBSEGMENT
//------------------------------------------------------------------------------
// Returncodes:       pointer to source segment structure
//                    NULL  if no matchin segment found
//------------------------------------------------------------------------------
// Side effects:      On output pulSegNum will contain the active
//                    segment number
//
//------------------------------------------------------------------------------
// Function flow:     - scan source doc and find next segment
//                      (either tobe or already done whatever is requested)
//                      starting from the current position
//                    - if not found in the forward scan do a wrap around
//                       and scan until segment is found
//                      (either tobe or already done whatever is requested)
//                      return indicator for wrap around/end reached
//                    - get pointer to it
//                      if none is available return a NULL
//
//------------------------------------------------------------------------------
PTBSEGMENT
EQFBFindNextSource( PTBDOCUMENT pDoc,     // pointer to document structure
                    PULONG  pulSegNum,    // pointer to segment number
                    USHORT  usCond,       // search condition
                    PBOOL   pfEndReached, // TRUE if at end of file
                    BOOL    fMode,        // mode of operation (foreground)
                    PVOID   *ppvMetaData) // callers metadata pointer
{
   BOOL   fFound = FALSE;                 // no match found
   PTBSEGMENT  pSeg;                      // pointer to segment
   PTBSEGMENT  pSegStart;                 // pointer to start
   ULONG       ulSegNum;                  // segment number
   BOOL        fUntrans;                  // untranslated only ??
   BOOL        fSecondTry = FALSE;               // avoid loop
   ULONG       usSegStartNum = 0;

   *pfEndReached = FALSE;                  //init return that not at EOF
   ulSegNum = *pulSegNum;                 // store segment number temporarily
   pSeg = EQFBGetVisSeg(pDoc, &ulSegNum); // get seg
   if ( pSeg )
   {
     usSegStartNum = pSeg->ulSegNum;
   } /* endif */

   if ( usCond != POS_CURSOR )
   {
//    fUntrans = ( usCond == POS_TOBE );  // set search criteria
      fUntrans = ( usCond != POS_TOBEORDONE );  // set search criteria
      while ( pSeg && pSeg->pDataW && ! fFound)
      {
         switch ( pSeg->qStatus)
         {
           case QF_XLATED:                         // is translated
           case QF_CURRENT:                        //
              fFound = !fUntrans;                  // set to found if we donït care
              break;
           case QF_ATTR:                           // attribute
           case QF_TOBE:                           // to be translated
              if ( ( usCond == POS_TOBEORDONE ) ||
                   ( usCond == POS_TOBE ) ||
                   ( fMode == FALSE ) ) {          // Background
                 fFound = TRUE;
              } 
              else                                 // Select based on memory matches
              {
                 PDOCUMENT_IDA  pIdaDoc;                       // pointer to document struct.
                 PSTEQFSAB     pstEQFSab;                      // ptr to SAB element
                 PSTEQFGEN     pstEQFGen;                      // pointer to generic structure

                 pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen ;
                 pstEQFGen->usRC = NO_ERROR;
                 pIdaDoc = (PDOCUMENT_IDA)pstEQFGen->pDoc;
                 pstEQFSab = pIdaDoc->stEQFSab + pIdaDoc->usFI; // point to current element
                 pstEQFSab->ulParm1 = ulSegNum;
                 pstEQFSab->usPropCount = 0;
                 wcscpy( pstEQFSab->pucSourceSeg, pSeg->pDataW ) ;
                 memset(&pstEQFSab->pszSortTargetSeg[0], '\0', EQF_NPROP_TGTS*sizeof(PSZ_W));
                 memset(&pstEQFSab->pszSortPropsSeg[0], '\0', EQF_NPROP_TGTS*sizeof(PSZ_W));
                 memset(&pstEQFSab->usFuzzyPercents[0], '\0', EQF_NPROP_TGTS*sizeof(USHORT));
                 memset(&pstEQFSab->fInvisible[0], '\0', EQF_NPROP_TGTS*sizeof(EQF_BOOL));
                 memset(&pstEQFSab->pszSortPropsData[0], '\0', EQF_NPROP_TGTS*sizeof(PSZ_W));
                 UTF16memset (pstEQFSab->pucTargetSegs, '\0', EQF_TGTLEN);
                 UTF16memset (pstEQFSab->pucPropsSegs, '\0', EQF_TGTLEN);
                 UTF16memset (pstEQFSab->pucPropAddData, '\0', EQF_TGTLEN);
                 EQFTM (pIdaDoc, EQFCMD_TRANSSEGW, pstEQFSab);     // TRANSSEG to TM

                 if ( (pstEQFGen->usRC == 0) || 
                      (pstEQFGen->usRC==EQFRS_SEG_NOT_FOUND))
                 {
                    if ( ( ( usCond == POS_TOBE_EXACT  ) ||       // look for EXACT match
                           ( usCond == POS_TOBE_MT     ) ||       // look for MT match
                           ( usCond == POS_TOBE_GLOBAL ) ) &&     // look for GLOBAL MEMORY match
                         ( pstEQFSab->usPropCount > 0 ) &&
                         ( pstEQFSab->usFuzzyPercents[0] >= (SHORT)pstEQFGen->lExactMatchLevel ) ) {
                       for( int i=0 ; i<pstEQFSab->usPropCount ; ++i ) {
                          if ( ( ( usCond == POS_TOBE_EXACT ) &&      // found EXACT match
                                 ( pstEQFSab->usMachineTrans[i] == EXACT_PROP ) ) ||
                               ( ( usCond == POS_TOBE_MT ) &&         // found MT match 
                                 ( pstEQFSab->usMachineTrans[i] == MACHINE_TRANS_PROP ) ) ||
                               ( ( usCond == POS_TOBE_GLOBAL ) &&     // found GLOBAL MEMORY match 
                                 ( ( pstEQFSab->usMachineTrans[i] == GLOBMEM_TRANS_PROP     ) ||
                                   ( pstEQFSab->usMachineTrans[i] == GLOBMEMSTAR_TRANS_PROP ) ) ) ) {
                             fFound = TRUE ; 
                             break ;
                          }
                       }
                    } else
                    if ( ( ( usCond == POS_TOBE_FUZZY ) &&        // found FUZZY match
                           ( pstEQFSab->usPropCount > 0 ) &&
                           ( pstEQFSab->usFuzzyPercents[0] < (SHORT)pstEQFGen->lExactMatchLevel ) ) ||
                         ( ( usCond == POS_TOBE_NONE ) &&         // found NO matches 
                           ( pstEQFSab->usPropCount == 0 ) ) ) {
                       fFound = TRUE ;           
                    } 
                 }

              }
              break;
           case QF_NOP:                            // nop
              break;
         } /* endswitch */
         if ( !fFound)
         {
            ulSegNum++;                            // point to next segment
            pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
         } /* endif */
      } /* endwhile */

      if ( !fFound )                               // not found,i.e. end table
      {
         fSecondTry = TRUE;                         // to ask user if wrap around
         ulSegNum = 1;                              // reset to first segment
         pSeg = EQFBGetVisSeg(pDoc, &ulSegNum) ;    // get first seg
         while ( pSeg && (pSeg->ulSegNum != usSegStartNum) && ! fFound)
         {
            switch ( pSeg->qStatus)
            {
              case QF_XLATED:                         // is translated
              case QF_CURRENT:                        //
                 break;
              case QF_ATTR:                           // attribute
              case QF_TOBE:                           // to be translated
                 fFound = TRUE;
                 break;
              case QF_NOP:                            // nop
                 break;
            } /* endswitch */
            if ( !fFound)
            {
               ulSegNum++;                            // point to next segment
               pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
            } /* endif */
         } /* endwhile */
//         pSeg = (pSeg != pSegStart) ? pSeg : NULL;  // set to NULL if not found
         if ( pSeg )
         {
           pSeg = ( pSeg->ulSegNum != usSegStartNum ) ? pSeg : NULL;
         } /* endif */
      } /* endif */
   }
   else                                            // position at cursor ...
   {
     /*****************************************************************/
     /* if on protected activate only if display style is unprotected */
     /* otherwise skip to next real segment                           */
     /*****************************************************************/
      while ( pSeg && !fFound && (pDoc->DispStyle != DISP_UNPROTECTED))
      {
        switch ( pSeg->qStatus)
        {
          case QF_XLATED:                         // is translated
          case QF_CURRENT:                        //
          case QF_ATTR:                           // attribute
          case QF_TOBE:                           // to be translated
             fFound = TRUE;
             break;
          case QF_NOP:                            // nop
             break;
        } /* endswitch */
        if ( !fFound)
        {
           ulSegNum++;                            // point to next segment
           pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
           if ( pSeg && pSeg->ulSegNum == usSegStartNum )  // wrap around
           {
              fFound = TRUE;                      // leave loop
              pSeg = NULL;
           }
           else
           {
              if ( (!pSeg || !pSeg->pDataW) && ! fSecondTry)  // if at end ...
              {
                 fSecondTry = TRUE;             // avoid loop
                 ulSegNum = 1;                   // reset to start
                 pSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
              } /* endif */
           } /* endif */
        } /* endif */
      } /* endwhile */
   } /* endif */

   if ( pSeg && pSeg->pDataW )                   // start point found
   {                                            // get  start seg
     /*****************************************************************/
     /* if reset to segnum=1 happened and doc is not translated,      */
     /* return inidcator pfEndReached if this happened                */
     /*****************************************************************/
     if ( fSecondTry)
     {
          *pfEndReached = TRUE;                //return that end reached
     } /* endif */

     *ppvMetaData = pSeg->pvMetadata;

     pSegStart = EQFBGetSegW(pDoc->twin, pSeg->ulSegNum);
     *pulSegNum = pSeg->ulSegNum;              // set segment number
   }
   else                                         // document translated
   {
      pSegStart = NULL;
   } /* endif */


   return ( pSegStart );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBTCheckPos
//------------------------------------------------------------------------------
// Function call:     EQFBTCheckPos( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       This function will determine which of the following
//                    states is true:
//                      - first call to TM (no previously segment
//                        available)
//                      - segment normally translated
//                      - user changed to another segment
//                      - in postedit mode
//
//------------------------------------------------------------------------------
// Parameters:        USHORT  type of user action
//                                 CHECKPOS_XLATESEG normal user action
//                                 CHECKPOS_MOVE     user moved
//                                 CHECKPOS_FIRST    first call
//                                 CHECKPOS_POSTEDIT postedit mode
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT -- pointer to document structure
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       type of user action
//                         CHECKPOS_XLATESEG normal user action
//                         CHECKPOS_MOVE     user moved
//                         CHECKPOS_FIRST    first call
//                         CHECKPOS_POSTEDIT postedit mode
//
//------------------------------------------------------------------------------
// Function flow:       - if postedit flag on then
//                           set output postedit
//                        elsif first invocation then
//                           set output to first call to TM
//                        elsif segnum of active segment != current segnum
//                           set output to 'move'
//                        endif
//
//------------------------------------------------------------------------------
static
USHORT EQFBTCheckPos
(
   PTBDOCUMENT   pDoc                     // pointer to document struct
)
{
   USHORT usResult = CHECKPOS_XLATESEG;      // segment normally translated


   if ( pDoc->EQFBFlags.PostEdit )
   {
      usResult = CHECKPOS_POSTEDIT;          // come from post edit mode
   }
//   else if ( pDoc->fXlated )
//   {
//      usResult = CHECKPOS_XLATED;            // already translated
//   }
   else
   {
      if ( pDoc->tbActSeg.ulSegNum == 0)         // first invocation
      {
         usResult = CHECKPOS_FIRST;
      }
      else
      {
         if ( pDoc->tbActSeg.ulSegNum != (pDoc->TBCursor).ulSegNum )
         {
            if (EQFBStillInActSeg( pDoc ))
            {
                                // ignore request -- user might wanted to stay in old active segment....
                        }
                        else
                        {
              usResult = CHECKPOS_MOVE;
                        }
         } /* endif */
      } /* endif */
   } /* endif */

   return( usResult );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBSaveSeg
//------------------------------------------------------------------------------
// Function call:     EQFBSaveSeg( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       save the current segment in the translation memory
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT -- pointer to document structure
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE      -- segment saved in translation memory
//                    FALSE     -- error during save
//
//------------------------------------------------------------------------------
// Function flow:     - check for empty segment and do appropriate handling
//                    - reset to read only and set coloring
//                    - get source segment
//                    - if nothing changed in source, ask user what to do
//                    - save it in translation memory; display error message
//                      in case of problems and reset the status flag to the
//                      old status for data integrity
//
//------------------------------------------------------------------------------
BOOL  EQFBSaveSeg
(
   PTBDOCUMENT    pDoc                          // pointer to document ida
)
{
   PTBSEGMENT pTBSourceSeg;                     // pointer to Source Segment
   PTBSEGMENT pTBTargetSeg;                     // pointer to Target Segment
   USHORT     usRc;                             // return code from Services
   BOOL       fOK = TRUE;                       // success indicator
   PSZ_W      pData;                            // pointer to error data
   USHORT     usMBId;                           // return code from UtlError
   USHORT     usLen;                            // strlen of string
   static CHAR_W chEQFSaveSeg[ MAX_SEGMENT_SIZE + 1];
   BOOL      fTransNot = FALSE;
   ULONG      ulTgtWords = 0L;            // number of untranslated words
   ULONG      ulTgtMarkUp = 0L;            // number of untranslated words
   PDOCUMENT_IDA  pIdaDoc;                       // pointer to document struct.

   if ( pDoc->pSaveSegW )                           // something to save
   {
      /****************************************************************/
      /* check for changes in tags .....                              */
      /****************************************************************/
      if ( pDoc->pUserSettings->fTagCheck )
      {
        fOK = EQFBDoTagCheck( pDoc );
      } /* endif */

      /****************************************************************/
      /* check if tags are empty ..                                   */
      /****************************************************************/
      if ( fOK )
      {
        fOK = EQFBDoEmptySegCheck( pDoc );       // check if segment is empty
      } /* endif */


      if ( fOK )
      {
        {
          PTBSEGMENT  pCurSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum);
          MDGetMetadata( pDoc, pCurSeg, pDoc->EQFBFlags.workchng );
        }

        if ( !pDoc->EQFBFlags.workchng )
        {
           MTLogStartEditing( pDoc );

           pData = pDoc->pEQFBWorkSegmentW;
           if ( pDoc->fFuzzyCopied )
           {
             /*********************************************************/
             /* display message depending on user settings ...        */
             /*********************************************************/
              if ( pDoc->pUserSettings->fFuzzyMsg )
              {
                                       //display message
                UTF16strncpy( chSeg1, pData, MSGBOXDATALEN);
                chSeg1[ MSGBOXDATALEN ] = EOS;   // set end of string
                if ( UTF16strlenCHAR(chSeg1) >= MSGBOXDATALEN )
                {                                // fill last three chars
                   UTF16strcpy(chSeg1+MSGBOXDATALEN-4,L"...");
                } /* endif */

                pData = chSeg1;
                usMBId = UtlErrorW( ( pDoc->fFuzzyCopied == 2 ) ? TB_MACHINENOTCHANGED : TB_FUZZYNOTCHANGED, MB_YESNO | MB_DEFBUTTON2,
                                   1, &pData, EQF_QUERY, TRUE);
              }
              else
              {
                usMBId = MBID_YES;
              } /* endif */

              if ( usMBId == MBID_YES )
              {
                 pDoc->EQFBFlags.workchng = TRUE;          // set workseg changed
                 /*****************************************************/
                 /* reset fuzzy copied - question already asked....   */
                 /*****************************************************/
                 pDoc->fFuzzyCopied = FALSE;              // KWT0021  A1
              }
              else
              {
                 fOK = FALSE;
              } /* endif */
           }
           else
           {
             if ( pDoc->tbActSeg.qStatus != QF_XLATED )
             {
               /*******************************************************/
               /* display message on user settings ...                */
               /*******************************************************/
               if ( pDoc->pUserSettings->fSrcUnChg )
               {
                                        //display message
                 UTF16strncpy( chSeg1, pData, MSGBOXDATALEN);
                 chSeg1[ MSGBOXDATALEN ] = EOS;   // set end of string
                 if ( UTF16strlenCHAR(chSeg1) >= MSGBOXDATALEN )
                 {                                // fill last three chars
                    UTF16strcpy(chSeg1+MSGBOXDATALEN-4,L"...");
                 } /* endif */

                 pData = chSeg1;
                 usMBId = UtlErrorW( TB_SOURCEUNCHANGED,
                                    MB_YESNOCANCEL | MB_DEFBUTTON1,
                                    1, &pData, EQF_QUERY, TRUE);
               }
               else
               {
                 usMBId = MBID_YES;
               } /* endif */

               fOK = (usMBId == MBID_YES);
               /******************************************************/
               /* if No, do not save segment but return yes to       */
               /* activate next segment                              */
               /******************************************************/
               fTransNot = (usMBId == MBID_NO);

               if ( fOK )
               {
                 pDoc->pTBSeg->SegFlags.Typed = 1;

               } /* endif */
             } /* endif */

           } /* endif */
        } /* endif */
      } /* endif */

      if ( fOK )
      {
        /**************************************************************/
        /* if fConvSOSI set in UserSettings insert them               */
        /**************************************************************/

        if (IsDBCS_CP(pDoc->ulOemCodePage) && pDoc->pUserSettings->UserOptFlags.bConvSOSI )
        {
          EQFBConvertSOSI(pDoc,pDoc->pTBSeg, INSERT_SOSI);

          if (pDoc->fLineWrap && pDoc->fAutoLineWrap )
          {
            USHORT   usNewLen;
            USHORT   usSegOffset;
            LONG     lEndCol;
            BOOL     fResult;
            usSegOffset = pDoc->TBCursor.usSegOffset;
            EQFBGotoSeg(pDoc, pDoc->TBCursor.ulSegNum, 0);
            EQFBBufRemoveSoftLF(pDoc->hwndRichEdit, pDoc->pTBSeg->pDataW,
                                &usNewLen, &usSegOffset    );
            fResult = EQFBBufAddSoftLF(pDoc, pDoc->pTBSeg,
                             pDoc->pTBSeg->pDataW,
                             pDoc->lCursorCol + pDoc->lSideScroll,
                             &lEndCol,
                             &usSegOffset);

            if (fResult)
            {
              pDoc->pTBSeg->usLength = (USHORT) UTF16strlenCHAR (pDoc->pTBSeg->pDataW);
            }
            EQFBGotoSeg(pDoc, pDoc->TBCursor.ulSegNum, usSegOffset);
          } /* endif */
        } /* endif */
        /*************************************************************/
        /* check if we have to do some special checking on the seg.  */
        /*************************************************************/
        if ( pDoc->pfnCheckSegExit || pDoc->pfnCheckSegExitW || pDoc->pfnCheckSegExExitW)
        {
          PTBSEGMENT pTBPrevSourceSeg = EQFBGetSegW(pDoc->twin,
                                                   (USHORT)(pDoc->ulWorkSeg-1) );
          EQF_BOOL  fChanged = FALSE;
          PSZ_W     pPrevSegData;


          pPrevSegData = (pTBPrevSourceSeg) ? pTBPrevSourceSeg->pDataW : NULL;

          pTBSourceSeg = EQFBGetSegW(pDoc->twin, pDoc->ulWorkSeg);

          /***********************************************************/
          /* fOK = TRUE indicates user wants to save it in TM,       */
          /* fChanged indicates that segment data was changed        */
          /* (the length of the segment data are 2k                  */
          /***********************************************************/
          if (pDoc->pfnCheckSegExExitW)
          {
            fOK = (BOOL) (pDoc->pfnCheckSegExExitW)( pPrevSegData,
                                    pTBSourceSeg->pDataW,
                                    pDoc->pTBSeg->pDataW,
                                    &fChanged, 
                                    (LONG)pDoc, pDoc->ulWorkSeg, TRUE );
		      }
		      else if (pDoc->pfnCheckSegExitW)
          {
            fOK = (BOOL) (pDoc->pfnCheckSegExitW)( pPrevSegData,
                                    pTBSourceSeg->pDataW,
                                    pDoc->pTBSeg->pDataW,
                                    &fChanged, TRUE );
		  }
		  else
		  {
			  PSZ       pData = NULL;
			  PSZ_W     pTmpBuf = NULL;
			  USHORT    usLen = 0;

			  if ( UtlAlloc( (PVOID *)&pData, 0L, 3L * MAX_SEGMENT_SIZE, NOMSG ) )
			  {
				PSZ pPrevSeg   = pData;
				PSZ pSourceSeg = pData + MAX_SEGMENT_SIZE;
				PSZ pTBSeg     = pSourceSeg + MAX_SEGMENT_SIZE;
				BOOL  fResult = TRUE;

				Unicode2ASCII( pPrevSegData, pPrevSeg, pDoc->twin->ulOemCodePage );
				Unicode2ASCII( pTBSourceSeg->pDataW, pSourceSeg, pDoc->twin->ulOemCodePage );
				Unicode2ASCII( pDoc->pTBSeg->pDataW, pTBSeg , pDoc->ulOemCodePage);
				fOK = (BOOL) (pDoc->pfnCheckSegExit)( pPrevSeg, pSourceSeg, pTBSeg,
															&fChanged, TRUE );
				// if changed... fill the changes into our structures
				if (fChanged)
				{
				 if (!pDoc->pSegmentBufferW )
				 {
			    	  fResult = UtlAlloc((PVOID *)&(pDoc->pSegmentBufferW),
									 0L, (LONG)(MAX_SEGMENT_SIZE+1)*sizeof(CHAR_W),
									 ERROR_STORAGE);
			     } /* endif */
			     if (fResult)
			     {
			        ASCII2Unicode(pTBSeg, pDoc->pSegmentBufferW, pDoc->ulOemCodePage);
			        if ( (pDoc->pTBSeg->ulSegNum != pDoc->ulWorkSeg)  ||
				                (&pDoc->pEQFBWorkSegmentW[0] != pDoc->pTBSeg->pDataW)  )   //@@
				    {
						usLen = (USHORT)max( UTF16strlenCHAR( pDoc->pSegmentBufferW )+1, MIN_ALLOC );  // get length

				        pTmpBuf = pDoc->pTBSeg->pDataW;                   // get pointer to data
				        fResult = UtlAlloc((PVOID *) &(pDoc->pTBSeg->pDataW),
				                     0L, (LONG) (usLen * sizeof(CHAR_W)), ERROR_STORAGE );
				    } /* endif */
				    if ( fResult )
				    {
				        UTF16strcpy( pDoc->pTBSeg->pDataW, pDoc->pSegmentBufferW );

						if ( pTmpBuf )
						{
						  UtlAlloc((PVOID *)&pTmpBuf, 0L, 0L, NOMSG );                // data space
						} /* endif */
				        pDoc->Redraw |= REDRAW_ALL;                          // redraw all
                    } /* endif */
			     } /* endif fResult */
                } /* endif fChanged */
			    UtlAlloc( (PVOID *) &(pData), 0L, 0L, NOMSG );// free storage
		      } /* endif pData */
           } /* endif CheckSegExit */
         // if changed we have to update our internal buffers
         // to indicate the changes to the user....
           if ( fChanged )
           {
             pDoc->EQFBFlags.workchng = TRUE;          // set workseg changed
             EQFBCompSeg( pDoc->pTBSeg );
             pDoc->TBCursor.ulSegNum = pDoc->pTBSeg->ulSegNum; // set segment number
             pDoc->TBCursor.usSegOffset = 0;           // set segment offset
             EQFBGotoSeg( pDoc, pDoc->TBCursor.ulSegNum, 0);
           } /* endif */
        } /* endif CheckSegExit or CheckSegExitW*/
      } /* endif */

      if ( fOK )
      {
        // store old contents to be able to restore it just in case of
        // problems during EQFSAVESEG
        UTF16strcpy( chEQFSaveSeg, pDoc->pSaveSegW );
        EQFBWorkSegOut( pDoc );

   #ifdef TRANTIME
       ulStart = pGlobInfoSeg->msecs;
   #endif

        if (pDoc->pTMMaint != NULL )     // if from TMEdit: avoid EQFBSaveSeg
        {
           usRc = EQFRC_OK;
        }
        else
        {
           PSZ_W pszContext = NULL;
           PVOID pvMetaData = pDoc->pTBSeg->pvMetadata;
           if ( (pvMetaData == NULL) && (pDoc->szMetaData[0] != 0) )
           {
             pvMetaData = (PVOID)pDoc->szMetaData;
           } /* endif */              
           pTBSourceSeg = EQFBGetSegW(pDoc->twin, pDoc->ulWorkSeg);
           pszContext = EQFBGetContext( pDoc->twin, pTBSourceSeg, pDoc->ulWorkSeg );
           usRc = EQFSAVESEG3W                   // save it in Translation Memory
                     ( pTBSourceSeg->pDataW,      // source data
                       pDoc->pTBSeg->pDataW,      // target data
                       pszContext,               // context infor or NULL
                        pvMetaData,              // segment meta data or NULL
                       pDoc->ulWorkSeg );        // segment number
        } /* endif */

   #ifdef TRANTIME
         ulEnd = pGlobInfoSeg->msecs;
         ulSave += ulEnd - ulStart;
         ulDelta = ulEnd - ulBegin;
         fprintf( fStream, "%6ld %6ld  %6ld\n", ulTime, ulSave, ulDelta);
   #endif

         // do not write MT log entries in post edit mode
         if ( hMTLog && !pDoc->EQFBFlags.PostEdit && (UtlQueryUShort( QS_MTLOGGING ) != 0) )
         {
           ULONG ulCurrent = GetTickCount();
           MTLogStartEditing( pDoc );
           pDoc->ActSegLog.ulTime = ulCurrent - pDoc->ActSegLog.ulTime;
           pDoc->ActSegLog.ulTotalTime = ulCurrent - pDoc->ActSegLog.ulTotalTime;
           if ( (pDoc->ActSegLog.usWordCnt == 0) && (pDoc->pTBSeg != NULL) )
           {
             pDoc->ActSegLog.usWordCnt = pDoc->pTBSeg->usSrcWords;
           } /* endif */              
           pTBSourceSeg = EQFBGetSegW(pDoc->twin, pDoc->ulWorkSeg);
           pDoc->ActSegLog.AddFlags.AutoSubst = (pDoc->pTBSeg->CountFlag.AnalAutoSubst || pDoc->pTBSeg->CountFlag.EditAutoSubst);

           // fill PE data target field
           wcscpy( pDoc->PEData.szTarget, pDoc->pTBSeg->pDataW );

           // write MTLOG entry
           WriteMTLog( &(pDoc->ActSegLog), pTBSourceSeg->pDataW, pDoc->szProposalDocName, pDoc->szMetaData, &pDoc->PEData );
         }

         if ( usRc != EQFRC_OK)
         {
            PSZ pErr = EQFERRINS();      // get error message
            UtlError( EQFERRID(), MB_CANCEL, 1, &pErr, EQF_ERROR );
            fOK = FALSE;
            // Go back to work segment
            EQFBGotoSeg( pDoc, pDoc->ulWorkSeg, 0);  // position at this seg
            EQFBWorkSegIn( pDoc );              // copy contents of current seg
            pDoc->EQFBFlags.workchng = TRUE;    // work seg changed

            usLen = (USHORT)max( UTF16strlenCHAR( chEQFSaveSeg )+1, MIN_ALLOC );  // get length

            if ( UtlAlloc( (PVOID *) &pData, 0L, (LONG) usLen*sizeof(CHAR_W), ERROR_STORAGE ) )
            {
               UTF16strcpy( pData, chEQFSaveSeg );
               UtlAlloc( (PVOID *) &(pDoc->pSaveSegW), 0L, 0L, NOMSG );// free old storage
               pDoc->pSaveSegW = pData;
            } /* endif */
         }
         else
         {
           if ( pDoc->pstEQFGen != NULL)
           {
             pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
           }
           else
           {
             pIdaDoc = NULL;
           } /* endif */

           /******************************************************************/
           /* Check if target language linguistic support is available       */
           /* if not display warning message and tell user that he can go on */
           /* nevertheless the result is not so good.                        */
           /* Note: Display this message only once....                       */
           /******************************************************************/
           if ( pIdaDoc )
           {
             if ( pIdaDoc->sTgtLanguage == -1 )
             {
               /***********************************************************/
               /* Get languages and format for current document           */
               /***********************************************************/
               if ((MorphGetLanguageID( pIdaDoc->szDocTargetLang,
                      &pIdaDoc->sTgtLanguage ) != MORPH_OK) )
               {
                  pIdaDoc->sTgtLanguage = pIdaDoc->sSrcLanguage ;
               } /* endif */
               pIdaDoc->ulTgtOemCP = GetLangOEMCP(pIdaDoc->szDocTargetLang);
             } /* endif */
             usRc = EQFBWordCntPerSeg(
                          (PLOADEDTABLE)pDoc->pDocTagTable,
                          (PTOKENENTRY) pDoc->pTokBuf,
                          pDoc->pTBSeg->pDataW,
                          pIdaDoc->sTgtLanguage,
                          &ulTgtWords, &ulTgtMarkUp,
                          pDoc->ulOemCodePage);
             pDoc->pTBSeg->usTgtWords = (USHORT) ulTgtWords;
           } /* endif */
                                                // mark segment as translated
           pDoc->pTBSeg->qStatus = QF_XLATED;
                                                // reset the active segment
           pDoc->pTBSeg->SegFlags.Current = FALSE;
           pDoc->pTBSeg->SegFlags.UnTrans = FALSE;         // and untrans flag
           pDoc->pTBSeg->SegFlags.Expanded = FALSE;
           pDoc->tbActSeg.ulSegNum = 0;         // reset active segment
           pDoc->flags.changed = TRUE;           // document changed....
           pDoc->fFuzzyCopied = FALSE;           // message already displayed..
           if ( pDoc->hwndRichEdit )
           {
             EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pDoc->pTBSeg->pDataW );
             SendMessage( pDoc->hwndRichEdit, EM_SETMODIFY, 0, 0L );
           } /* endif */
         } /* endif */
      } /* endif */
   } /* endif */
   if ( fTransNot )
   {
     pTBTargetSeg = EQFBGetSegW(pDoc, pDoc->tbActSeg.ulSegNum); // get seg
                                           // get old status
     pTBTargetSeg->qStatus = pDoc->tbActSeg.qStatus;
     pTBTargetSeg->SegFlags.Current = FALSE;
     pTBTargetSeg->SegFlags.Expanded = FALSE;

     if ( pDoc->hwndRichEdit )
     {
       EQFBSetWorkSegRTF( pDoc, pDoc->tbActSeg.ulSegNum, pTBTargetSeg->pDataW );
     }

     pTBTargetSeg->pDataW = pDoc->pSaveSegW;
     pDoc->pSaveSegW = NULL;                 // reset save seg

     fOK = TRUE;                     //inits goto next segment
   } /* endif */
   return (fOK);
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBDoEmptySegCheck
//------------------------------------------------------------------------------
// Function call:     EQFBDoEmptySegCheck( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       check if segment is empty and do appropriate
//                    handling
//
//------------------------------------------------------------------------------
// Function flow:     check for empty segment
//                    If segment empty then
//                      inform user (display message) and let user
//                      decide what he wants to do
//                      If he wants to continue then
//                        insert a such called 'EMPTY' tag to allow
//                        user to activate segment again;
//                      else
//                        return false, but toggle in triple mode
//                      endif
//                    else
//                      return TRUE
//                    endif
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    - pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  - proceed with save
//                    FALSE - user wants to work on this seg again
//------------------------------------------------------------------------------
BOOL EQFBDoEmptySegCheck
(
   PTBDOCUMENT  pDoc                        // pointer to document ida
)
{
   BOOL  fOK = TRUE;                         // success indicator
   USHORT usResult;                          // return value from UtlError
   BOOL        fNonEmpty = FALSE;            // no non white space char found
   PSZ_W       pData = pDoc->pEQFBWorkSegmentW;// pointer to work seg
   CHAR_W      c;
   BOOL        fWHITESPfound = FALSE;        // TRUE if blank or LF found

   // check for none white space character in WorkSegment
   while ( !fNonEmpty && ((c=*pData)!= NULC))
   {
      switch ( c )
      {
         case LF:
         case BLANK:
            fWHITESPfound = TRUE;                              /* @KIT975A */
            break;
         default:
            fNonEmpty = TRUE;
            break;
      } /* endswitch */
      pData ++;
   } /* endwhile */


   if ( !fNonEmpty )
   {
      /****************************************************************/
      /* in post edit fill in automaticly a :NONE tag                 */
      /****************************************************************/
      if (  pDoc->EQFBFlags.PostEdit || pDoc->EQFBFlags.NoEmptySegCheck )
      {
         usResult = MBID_YES;
      }
      else
      {
         usResult = UtlError( TB_SEGEMPTY, MB_YESNOCANCEL | MB_DEFBUTTON2,
                              0, NULL, EQF_QUERY);
      } /* endif */
      if ( usResult == MBID_YES)
      {
        if ( !fWHITESPfound )                                  /* @KIT975A */
        {                                                      /* @KIT975A */
           // make space for EMPTY_TAG and copy it in
           usResult = (USHORT)UTF16strlenBYTE(EMPTY_TAG);
           memmove(((PBYTE)pDoc->pEQFBWorkSegmentW)+usResult, pDoc->pEQFBWorkSegmentW,
                   UTF16strlenBYTE(pDoc->pEQFBWorkSegmentW)+sizeof(CHAR_W));
           memcpy(pDoc->pEQFBWorkSegmentW, EMPTY_TAG, usResult); // copy empty tag

           if ( pDoc->hwndRichEdit )
           {
             // refresh worksegment in RTFControl
             EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pDoc->pEQFBWorkSegmentW );
           }

           pDoc->EQFBFlags.workchng = TRUE;    //  change in work seg
           EQFBCompSeg( pDoc->pTBSeg );
        } /* endif */                                          /* @KIT975A */
      }
      else
      {
         fOK = FALSE;                  // user wants to change segment
                                       // go back to active segment
         EQFBGotoSeg( pDoc, pDoc->ulWorkSeg,0);
      } /* endif */
   } /* endif */

   return (fOK);
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGetDictMatch
//------------------------------------------------------------------------------
// Function call:     EQFBGetDictMatch( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       insert the requested dict. match into
//                    the current active segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    - pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - get dictionary match from services
//                    - if requested match not available then
//                        issue a warning message
//                      else
//                         if we are still in active segment then
//                           insert the match on the cursor position
//                           depending on cursor mode (replace/insert)
//                         else
//                           issue a message
//
//
//------------------------------------------------------------------------------
VOID EQFBGetDictMatch
(
   PTBDOCUMENT  pDoc                        // pointer to document instance
)
{
   PSZ_W  pData;                             // pointer to data
   PSZ_W  pTemp;                             // temporary pointer
   USHORT usRc;                              // return code from services
   USHORT  usChar;                           // number of match requested
   BOOL   fInsert;                           // status of cursor
   PDOCUMENT_IDA  pIdaDoc = NULL;            // pointer to document struct.
   USHORT         usI;
   USHORT         usTemp = 0;


    if (pDoc->pstEQFGen )
    {
      pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
    } /* endif */
    if (pIdaDoc && (pIdaDoc->chDictPrefixes[0] != EOS) )
    {
      /****************************************************************/
      /* find position in Prefixarray of selected prefix              */
      /****************************************************************/
      usI = 0;
      if (('A'<=pDoc->usChar) && (pDoc->usChar <= 'Z')  )
      {
        usTemp = pDoc->usChar + 32 ;
      }
      else if (('a' <= pDoc->usChar ) && (pDoc->usChar <= 'z'))
      {
        usTemp = pDoc->usChar - 32;
      } /* endif */
      while ((pIdaDoc->chDictPrefixes[usI] != usTemp )
             && (usI <= ALL_LETTERS ) )
      {
        usI++;
      } /* endwhile */
      if (pIdaDoc->chDictPrefixes[usI] == usTemp )
      {
        usChar = usI;
      }
      else      //what to do here?
      {
        usChar = usI;
      } /* endif */
    }
    else
    {
       usChar = ( pDoc->usChar ) - 'A';         // get VK_A character value
      /**********************************************************************/
      /* usChar = 25 for 'z'; adjust, so that usChar = 26 for 'A' and so on */
      /**********************************************************************/
      if (pDoc->usChar >= 'a' )
      {
        usChar = pDoc->usChar -'a' + 26;
      } /* endif */
    } /* endif */

   if ( UtlAlloc( (PVOID *) &pData, 0L, MAX_DICTLENGTH, ERROR_STORAGE) )
   {
      usRc = EQFGETDICTW( usChar, pData );        // get dictionary data

      if ( usRc == EQFRC_OK )
      {
         if ( pDoc->tbActSeg.ulSegNum == (pDoc->TBCursor).ulSegNum )
         {
            pTemp = pData;                   // point to data

            if ( EQFBFuncCharIn( pDoc ) )
            {
               fInsert= pDoc->EQFBFlags.inserting;
               if ( pDoc->pUndoSegW )
               {                                      //update undo buffer
                  UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
                  pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
                  pDoc->fUndoState = FALSE;
               } /* endif */
               /***********************************************************/
               /* data are in ASCII -> convert into ansi due to the fact  */
               /* are simulating a keyboard input                         */
               /***********************************************************/
               EQFBPasteInSeg(pDoc, pTemp );

               MTLogStartEditing( pDoc );
			   pDoc->ActSegLog.ucNumDictCopied ++ ;

               pDoc->EQFBFlags.inserting = (USHORT)fInsert;
            }
            else
            {
              WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if no next segment
            } /* endif */
         }
         else
         {
            UtlError( TB_NOTINACTSEG, MB_CANCEL, 0, NULL, EQF_WARNING);
         } /* endif */
      }
      else
      {
          PSZ pErr = EQFERRINS();               // get error message
          UtlError( EQFERRID(), MB_CANCEL, 1, &pErr, EQF_ERROR );
      } /* endif */
      UtlAlloc( (PVOID *) &pData, 0L, 0L, NOMSG);
   } /* endif */
}

static void
EQFBPasteInSeg
(
  PTBDOCUMENT  pDoc,
  PSZ_W        pTemp
)
{
  if (pDoc->hwndRichEdit )
  {
    EQFBReplaceSelRTF( pDoc, pTemp );
  }
  else
  {
    while ( (pDoc->usChar = *pTemp) != NULC )  // insert word in active seg
    {                                // simulate as of keystrokes
      EQFBFuncCharacter( pDoc );     // therefore don't need cursor

      pDoc->ActSegLog.usNumTyped --;       // correction of eqfbfuncchar.

      pTemp++;                       // status, etc....
      // toggle into insert mode at begin of inline tag
      if ( ! EQFBFuncCharIn( pDoc ))
      {
        pDoc->EQFBFlags.inserting = TRUE;
      } /* endif */
    } /* endwhile */
  } /* endif */
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGetPropMatch
//------------------------------------------------------------------------------
// Function call:     EQFBGetPropMatch( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       insert the requested proposal match into
//                    the current active segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT    - pointer to document instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     extract requested match number from pDoc->usChar
//                    call EQFBCopyPropMatch with matchlevel 0 and fMsg=TRUE
//------------------------------------------------------------------------------
VOID EQFBGetPropMatch
(
  PTBDOCUMENT pDoc                           // pointer to document structure
)
{
   USHORT  usChar;                           // number of match requested

     if ( (pDoc->usChar >= VK_NUMPAD0) &&                        /* @KWT0010A */
          (pDoc->usChar <= VK_NUMPAD9 ))                         /* @KWT0010A */
     {                                                           /* @KWT0010A */
       usChar = ( pDoc->usChar ) - VK_NUMPAD0;                   /* @KWT0010A */
     }                                                           /* @KWT0010A */
     else                                                        /* @KWT0010A */
     {                                                           /* @KWT0010A */
       usChar = ( pDoc->usChar ) - CHARACTER_0;                  /* @KWT0010A */
     } /* endif */                                               /* @KWT0010A */

   EQFBCopyPropMatch( pDoc, usChar, 0 , TRUE, FALSE );     // let utility copy it
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBCopyPropMatch
//------------------------------------------------------------------------------
// Function call:     EQFBCopyPropMatch( PTBDOCUMENT, USHORT, USHORT, BOOL );
//
//------------------------------------------------------------------------------
// Description:       insert the requested proposal match into the
//                    current active segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  -- pointer to document structure
//                    USHORT       -- number of proposal to insert
//                    USHORT       -- minimum match level
//                    BOOL         -- message to be displayed
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE        -- if proposal could be inserted
//                    FALSE       -- if either not in active seg or prop not
//                                   avail
//------------------------------------------------------------------------------
// Function flow:     - get proposal match from services
//                    - if requested match not available then
//                        issue a warning message
//                      else
//                        if we are still in active segment then
//                          insert the match on the cursor position
//                          add a LF if one is available in the source
//                           segment but not in the proposal.
//                        else
//                          issue a message
//                        endif
//
//------------------------------------------------------------------------------
BOOL EQFBCopyPropMatch
(
   PTBDOCUMENT pDoc,                         // pointer to document ida
   USHORT usNum ,                            // number of match
   USHORT usMatchLevel,                      // minimum level of match
   BOOL   fMsg,                              // display error message
   BOOL   fWithAnnotationData                // copy any annotation data of the proposal to the segment metadata
)  
{
   PSZ_W  pData = NULL;                      // pointer to data
   PSZ_W  pTemp;                             // temporary space
   USHORT usRc;                              // return code from services
   USHORT usLevel;                           // match level
   BOOL  fOK;                                // true so far
   USHORT  usLenSrc;                         // offset within segment
   USHORT  usLen;                            // length of segment
   PTBSEGMENT pTBSeg;                        // pointer to segment
   PEQFBBLOCK  pstBlock;                     // pointer to block struct
   BOOL   fSourceEqual = FALSE;

   PSZ_W       pTempBuf;                    // ptr to temp buffer
   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;


   fOK = UtlAlloc( (PVOID *) &pTempBuf, 0L, 4L*MAX_LONGPATH, ERROR_STORAGE);
   if ( fOK )
   {
     fOK = UtlAlloc( (PVOID *) &pData, 0L, MAX_PROPLENGTH * sizeof(CHAR_W) * 2, ERROR_STORAGE);
   } /* endif */
   if ( fOK )
   {
      usLevel = QUERYIFSOURCEISEQUAL; // request source of prop equal to source
                                      // flag, the flag is returned as first
                                      // bit of the returned usLevel
      pDoc->sPropCopied = (SHORT) usNum; // remember number of proposal copied
      usRc = EQFGETPROPW( usNum, pData, &usLevel );   // get proposal data

      if ( usRc == EQFRC_OK )
      {
        // clear meta data buffer
        pDoc->szMetaData[0] = 0;
        pDoc->PEData.szCopiedMatchID[0] = 0;
        pDoc->PEData.szCopiedTarget[0] = 0;

         /*************************************************************/
         /* Extract the source-of-proposal-is-equal-to-source flag    */
         /* which is the first bit of usLevel, if usLevel contains    */
         /* 0xFFFF (-1) ignore the flag and set usLevel to -1         */
         /*************************************************************/

         if ( usLevel & 0x8000 )       // if first bit is set ...
         {
           usLevel = usLevel & 0x7FFF; // ... mask the fist bit

           if ( usLevel == 0x7FFF )    // if usLevel was -1 ...
           {
             usLevel = (USHORT)-1;     // ... restore it to -1
           }
           else
           {
             fSourceEqual = TRUE;
           } /* endif */
         } /* endif */

         if ( pDoc->TBCursor.ulSegNum == pDoc->tbActSeg.ulSegNum )
         {
			if (!pDoc->EQFBFlags.Reflow)
			{
			  fOK = TACheckAndAdaptLFChanges(pDoc->pEQFBWorkSegmentW,
			                                  pData, fMsg);
		    }

            if ( fOK && (usLevel >= usMatchLevel) )  // match matches req
            {
               if ( pDoc->pUndoSegW )
               {                                      //update undo buffer
                  UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
                  pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
                  pDoc->fUndoState = FALSE;
               } /* endif */

               /*******************************************************/
               /* check if insert of proposal requested               */
               /*******************************************************/
               if ( pDoc->pUserSettings->fInsProposal &&
                        pDoc->EQFBFlags.inserting )
               {
                 /*****************************************************/
                 /* insert proposal at current cursor place if possib.*/
                 /*****************************************************/
                 if ( EQFBFuncCharIn( pDoc ) )
                 {
                     {
                       USHORT   usNewLen;
                       USHORT   usSegOffset;
                       if (pDoc->fLineWrap && pDoc->fAutoLineWrap )
                       {
                         usSegOffset = pDoc->TBCursor.usSegOffset;
                         EQFBBufRemoveSoftLF(pDoc->hwndRichEdit, pDoc->pTBSeg->pDataW,
                                             &usNewLen, &usSegOffset    );
                         pDoc->TBCursor.usSegOffset = usSegOffset;
                       } /* endif */
                     }
                   usLenSrc = (USHORT)UTF16strlenCHAR( pData );
                   usLen    = pDoc->TBCursor.usSegOffset;
                   if ( (pDoc->pTBSeg->usLength) + usLenSrc < MAX_SEGMENT_SIZE )
                   {
                     memmove(pDoc->pEQFBWorkSegmentW+usLenSrc+ usLen,
                             pDoc->pEQFBWorkSegmentW+usLen,
                             (MAX_SEGMENT_SIZE - usLen - usLenSrc - 1)*sizeof(CHAR_W));
                     memcpy(pDoc->pEQFBWorkSegmentW+usLen, pData,
                            usLenSrc*sizeof(CHAR_W));
                   }
                   else
                   {
                     WinAlarm( HWND_DESKTOP, WA_WARNING );
                   } /* endif */
                 }
                 else
                 {
                   WinAlarm( HWND_DESKTOP, WA_WARNING );
                 } /* endif */
               }
               else
               {
                 /*****************************************************/
                 /* replace proposal                                  */
                 /*****************************************************/
                 UTF16strcpy(pDoc->pEQFBWorkSegmentW, pData);       // copy segment
                 pDoc->TBCursor.usSegOffset = 0;
               } /* endif */
               EQFBCompSeg(pDoc->pTBSeg);

               // FIX for LPEX inconsistencies
               //   add LF at end of segment if one was available in the source
               /*******************************************************/
               /* check that there is no additional linefeed added    */
               /* at the end...                                       */
               /*******************************************************/
               if ( usNum > 0 )
               {
                  BOOL fChanged;

                  pTemp = pDoc->pEQFBWorkSegmentW;
                  pTBSeg = EQFBGetSegW(pDoc->twin, pDoc->ulWorkSeg);
                  //if ( !fEqualSource || pDoc->fForceEqualWhiteSpace )
                  if ( pDoc->pUserSettings->UserOptFlags.bAdjustLeadingWS ||
                       pDoc->pUserSettings->UserOptFlags.bAdjustTrailingWS )
                  {
					          if (pDoc->pWSList == NULL)
					          {
						          PSTEQFGEN     pstEQFGen;
						          pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
						          TAFillWSList((PSZ)(pstEQFGen->szTagTable), &(pDoc->pWSList));
						          // pWSList is freed if pDoc is freed -
					          }
                    TAAdjustWhiteSpace( pTBSeg->pDataW, pTemp,
                                        &pTemp,
                                        pDoc->pUserSettings->UserOptFlags.bAdjustLeadingWS,
                                        pDoc->pUserSettings->UserOptFlags.bAdjustTrailingWS,
                                        &fChanged,
                                        pDoc->pWSList);
                    pDoc->pTBSeg->usLength = (USHORT)UTF16strlenCHAR (pDoc->pTBSeg->pDataW);
                  } /* endif */
               } /* endif */

               /**************************************************************/
               /* if fConvSOSI set in UserSettings insert them               */
               /**************************************************************/
               if (IsDBCS_CP(pDoc->ulOemCodePage) && pDoc->pUserSettings->UserOptFlags.bConvSOSI )
               {
                 EQFBConvertSOSI(pDoc,pDoc->pTBSeg, INSERT_SOSI);
               } /* endif */

              {
                LONG     lEndCol;
                USHORT   usSegOffOld;
                if (pDoc->fLineWrap )
                {
                  if (pDoc->fAutoLineWrap )
                  {
                    BOOL fResult = TRUE;
                    /****************************************************/
                    /* no soft lf's are inserted in proposal            */
                    /****************************************************/
                    usSegOffOld = pDoc->TBCursor.usSegOffset;
                    pDoc->TBCursor.usSegOffset = 0;
                    EQFBPhysCursorFromSeg(pDoc);
                    fResult = EQFBBufAddSoftLF(pDoc, pDoc->pTBSeg,
                                     pDoc->pTBSeg->pDataW,
                                     pDoc->lCursorCol + pDoc->lSideScroll,
                                     &lEndCol,
                                     &usSegOffOld);
                    if (fResult)
                    {
                      pDoc->pTBSeg->usLength = (USHORT)UTF16strlenCHAR (pDoc->pTBSeg->pDataW);
                      pDoc->TBCursor.usSegOffset = usSegOffOld;
				    }
                  }
                  else
                  {
                    EQFBFuncLineWrap(pDoc);
                  } /* endif */
                } /* endif */
              }

               // either fuzzy or workchng will be set
               // fuzzy necessary to allow for specific check if user edited
               // in fuzzy segment

               // GQ: for copied machine matches we use the value 2 rather than 1 (TRUE)
               {
                 USHORT usState = EqfGetPropState((PSTEQFGEN)pDoc->pstEQFGen, usNum);
                 if ( (usNum != ORIGINAL_PROP) && (usState & MACHINE_TRANS_PROP) )
                 {
                   pDoc->fFuzzyCopied = 2;
                 }
                 else
                 {
                   pDoc->fFuzzyCopied = ( usLevel < EQUAL_EQUAL );
                 } /* endif */                 
               }

               EQFBCompSeg( pDoc->pTBSeg );                      // force a recompute
               /*******************************************************/
               /* if original copied, set flags as in EQFBUntrans     */
               /*******************************************************/
               if ( usNum == ORIGINAL_PROP )
               {
                  pDoc->pTBSeg->SegFlags.Typed = FALSE;
                  pDoc->pTBSeg->SegFlags.Copied = FALSE;

                  pDoc->pTBSeg->CountFlag.FuzzyCopy = FALSE;
                  pDoc->pTBSeg->CountFlag.ExactCopy = FALSE;
                  pDoc->pTBSeg->CountFlag.ReplCopy = FALSE;
                  pDoc->pTBSeg->CountFlag.MachCopy = FALSE;
                  pDoc->pTBSeg->CountFlag.GlobMemCopy = FALSE;
                  pDoc->pTBSeg->usModWords = 0;
                  pDoc->pTBSeg->usTgtWords = 0;
                  pDoc->pTBSeg->CountFlag.PropChanged = FALSE;

                  memset(&(pDoc->ActSegLog.PropTypeCopied), 0, sizeof(pDoc->ActSegLog.PropTypeCopied) );
				  
                  // adjust MT log time measurements
                  MTLogUndoProposalCopy( pDoc );

               }
               else
               {
			            USHORT  usState = 0;
				  
                  memset(&(pDoc->ActSegLog.PropTypeCopied), 0, sizeof(pDoc->ActSegLog.PropTypeCopied) );
                  if (!pDoc->pTBSeg->SegFlags.Copied
                         && !pDoc->pTBSeg->CountFlag.EditAutoSubst
                         && !pDoc->pTBSeg->CountFlag.AnalAutoSubst )
                  {
                    BOOL    fCountFuzzy = 0;
                    usState = EqfGetPropState((PSTEQFGEN)pDoc->pstEQFGen, usNum);

                    if (pDoc->pTBSeg->CountFlag.Fuzzy5070 ||
                        pDoc->pTBSeg->CountFlag.Fuzzy7190 ||
                        pDoc->pTBSeg->CountFlag.Fuzzy9199 )
                    { // fuzzies < 50% must be counted as non-matches!!
                       fCountFuzzy = TRUE;
                    }
                    if ( usState )
                    {
                      if ( usState & EXACT_PROP )
                      {
                        pDoc->pTBSeg->CountFlag.ExactCopy = TRUE;
                        pDoc->ActSegLog.PropTypeCopied.Exact = TRUE;
                      }
                      else if ( (usState & GLOBMEM_TRANS_PROP) || (usState & GLOBMEMSTAR_TRANS_PROP) )
                      {
                        if ( (usState & FUZZY_REPLACE_PROP) || (usState & FUZZY_PROP) )
                        {
                          // set fuzzy copied for global memory fuzzy
            					    pDoc->pTBSeg->CountFlag.FuzzyCopy = TRUE;  
                          pDoc->pTBSeg->CountFlag.GlobMemCopy = TRUE;
                          pDoc->ActSegLog.PropTypeCopied.GlobMemFuzzy = TRUE;
                        }
                        else
                        {
                          pDoc->pTBSeg->CountFlag.GlobMemCopy = TRUE;
                          pDoc->ActSegLog.PropTypeCopied.GlobMem = TRUE;
                        } /* endif */                           
                      }
                      else if ( usState & GLOBMEMFUZZY_TRANS_PROP )
                      {
            					  pDoc->pTBSeg->CountFlag.FuzzyCopy = TRUE;  
                        pDoc->pTBSeg->CountFlag.GlobMemCopy = TRUE;
                        pDoc->ActSegLog.PropTypeCopied.GlobMemFuzzy = TRUE;
                      }
                      else if ( usState & REPLACE_PROP )
                      {
                        pDoc->pTBSeg->CountFlag.ReplCopy = TRUE;
                        pDoc->ActSegLog.PropTypeCopied.Replace = TRUE;
                      }
                      else if ( usState & FUZZY_REPLACE_PROP )
                      {
                        if ( fCountFuzzy )
                      {
                       // P019381 pDoc->pTBSeg->CountFlag.ReplCopy = TRUE;
                        pDoc->pTBSeg->CountFlag.FuzzyCopy = TRUE;
                        } /* endif */                           
                        pDoc->ActSegLog.PropTypeCopied.Fuzzy = TRUE;
                      }
                      else if ( usState & FUZZY_PROP )
          					  {
                        if ( fCountFuzzy )
          					  {
          					    pDoc->pTBSeg->CountFlag.FuzzyCopy = TRUE;
                        } /* endif */                           
                        pDoc->ActSegLog.PropTypeCopied.Fuzzy = TRUE;
                      }
                      else if (usState & MACHINE_TRANS_PROP )
                      {
                        pDoc->pTBSeg->CountFlag.MachCopy = TRUE;
                        pDoc->ActSegLog.PropTypeCopied.MT = TRUE;
                      }
                      else
                      {
                         /***********************************************/
                         /* occurs if fuzzy < 50%!! (RJ 040324)         */
                         /***********************************************/
                      } /* endif */
                    } /* endif */
                  } /* endif */
                  pDoc->pTBSeg->SegFlags.Typed = FALSE;
                  pDoc->pTBSeg->CountFlag.PropChanged = FALSE;

                  //set SegFlags.Copied independent of FUZZYNESS!!
                  pDoc->pTBSeg->SegFlags.Copied = TRUE;

                  pDoc->ActSegLog.ulSegNum = pDoc->pTBSeg->ulSegNum;

                  MTLogProposalCopied( pDoc );

                  // fill PE data field for copied proposal
                  {
                    HADDDATAKEY hKey;
                    PSZ_W pszPropData = pData + (wcslen(pData) + 1);
                    wcscpy( pDoc->PEData.szCopiedTarget, pData );

                     // find any existing match segment ID
                     hKey = MADSearchKey( pszPropData, MATCHSEGID_KEY );
                     if ( hKey != NULL ) 
                     {
                       MADGetAttr( hKey, MATCHSEGID_ATTR, pDoc->PEData.szCopiedMatchID, sizeof(pDoc->PEData.szCopiedMatchID) / sizeof(CHAR_W), L"" );
                     } /* endif */
                  }

                  if ( fWithAnnotationData )
                  {
                    PSZ_W pszPropData = pData + (wcslen(pData) + 1);
                    HADDDATAKEY hKey = MADSearchKey( pszPropData, L"Note" );
                    if ( hKey != NULL )
                    {
                      CHAR_W szStyle[40];
                      MADGetAttr( hKey, L"style", szStyle, sizeof(szStyle) / sizeof(CHAR_W), L"" );
                      MDAGetValueForKey( hKey, pDoc->szMetaData, sizeof(pDoc->szMetaData) / sizeof(CHAR_W), L"" );
                      MDAddCommentData( &(pDoc->pTBSeg->pvMetadata), pDoc->szMetaData, szStyle );
                      pDoc->szMetaData[0] = 0;
                      pDoc->szContextBuffer[0] = 0;
                      MDRefreshMetadata( pDoc, pDoc->pTBSeg, pDoc->szContextBuffer );
                    } /* endif */
                  } /* endif */

                  // MT proposals only: copy any MT data of copied proposal
                  if ( usState & MACHINE_TRANS_PROP )
                  {
                    PSZ_W pszPropData = pData + (wcslen(pData) + 1);
                    AddData_CopyFieldsFromList( pDoc->szMetaData, pszPropData, MTFIELDLIST );
                  }



                  // GQ 2012/10/15: PropTypeCopied handling moved to proposal copy logic

               } /* endif */

               /*******************************************************/
               /* write out the updated work segment                  */
               /*******************************************************/
               if ( pDoc->hwndRichEdit )
               {
                  EQFBSetWorkSegRTF( pDoc, pDoc->tbActSeg.ulSegNum, pDoc->pTBSeg->pDataW );
               }
               EQFBGotoSeg( pDoc, pDoc->tbActSeg.ulSegNum,
                            pDoc->TBCursor.usSegOffset);
               pDoc->Redraw |= REDRAW_ALL;      // redraw the screen
               // change to workseg - only if not already fuzzy activated
               pDoc->EQFBFlags.workchng = (USHORT)(! pDoc->fFuzzyCopied);

            }
            else
            {
               fOK = FALSE;                     // matching level too low...
            } /* endif */
         }
         else
         {
            UtlError( TB_NOTINACTSEG, MB_CANCEL, 0, NULL, EQF_WARNING);
            fOK = FALSE;
         } /* endif */
      }
      else
      {
         if ( fMsg )
         {
            PSZ pErr = EQFERRINS();               // get error message
            UtlError( EQFERRID(), MB_CANCEL, 1, &pErr, EQF_ERROR );
         } /* endif */
         fOK = FALSE;
      } /* endif */
      UtlAlloc( (PVOID *) &pData, 0L, 0L, NOMSG);
   } /* endif */

   UtlAlloc( (PVOID *) &pTempBuf, 0L, 0L, NOMSG);

   // reset any old block mark in segment

   if ( pstBlock->pDoc == pDoc &&
        pstBlock->ulSegNum == pDoc->ulWorkSeg )
   {
      pstBlock->pDoc = NULL;                    // reset block mark
      pDoc->Redraw |= REDRAW_ALL;               // force redraw the screen
   } /* endif */
   return (fOK);
}



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGotoActSegment
//------------------------------------------------------------------------------
// Function call:     EQFBGotoActSegment( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       position the file at the active segment
//                    This command is active from within the
//                    source and target document
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  -- pointer to document structure
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Side effects:      The global data for line and colomn will reflect the
//                    changes
//------------------------------------------------------------------------------
// Function flow:     - if in source doc, toggle to target doc
//                    - use the ActSegment structure and goto the
//                      segment
//                      If no segment is active issue a warning
//
//------------------------------------------------------------------------------
VOID EQFBGotoActSegment
(
  PTBDOCUMENT  pDoc                         // goto active segment
)
{
   PTBDOCUMENT pDocTemp;                     // temporary document pointer

   pDocTemp = (pDoc->docType == SSOURCE_DOC ) ? pDoc->twin : pDoc;

   if ( pDocTemp->tbActSeg.ulSegNum)
   {
                                             // position at this segment
      EQFBGotoSeg( pDocTemp, pDocTemp->tbActSeg.ulSegNum,0);
      if ( pDoc->docType  == SSOURCE_DOC )
      {                          // activate starget if necessary
         SendMessage( ((PSTEQFGEN)pDocTemp->pstEQFGen)->hwndTWBS,
                      WM_EQF_SETFOCUS, 0, MP2FROMHWND(pDocTemp->hwndFrame));
         WinInvalidateRegion( pDocTemp->hwndFrame, NULLHANDLE, TRUE);
      } /* endif */
   }
   else
   {
      UtlError( TB_NOSEGACT, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGotoSeg
//------------------------------------------------------------------------------
// Function call:     EQFBGotoSeg( PTBDOCUMENT, USHORT, USHORT );
//
//------------------------------------------------------------------------------
// Description:       position the file at the specified segment
//                    and prepare the workbuffer
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pointer to document instance
//                    USHORT       segment to be found
//                    USHORT       segment offset to position at
//
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Side effects:      The global data for line and colomn will reflect
//                    the changes
//
//------------------------------------------------------------------------------
// Function flow:     - save old workbuffer
//                    - fill TBRowOffset with the requested segnum
//                      in the 'Focusline' of TBRowOffset
//                    - check if Top Row of TBRowOffset is filled
//                    - if no offset given in input then
//                        position at start of segment
//                      else
//                        position at segment offset
//                      endif
//                    - get segment into workbuffer
//                    - set cursor type (triple mode) if necessary
//------------------------------------------------------------------------------

VOID EQFBGotoSeg
(
  PTBDOCUMENT  pDoc,                   // pointer to document instance
  ULONG  ulSegNum,                       // segment number to position at
  USHORT usSegOffset                     //segment offset to position at
)
{
   LONG  lFocus;                          // start with focus line
   USHORT usI;                                   // index
   if ( pDoc->hwndRichEdit )
   {
     EQFBGotoSegRTF( pDoc, ulSegNum, usSegOffset );
   }
   else
   {
     USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
     lFocus = min(pEQFBUserOpt->sFocusLine, pDoc->lScrnRows-2);
     (pDoc->TBCursor).ulSegNum = ulSegNum;      // set segment number
     (pDoc->TBCursor).usSegOffset  = usSegOffset;            // start at the beginning
     /*******************************************************************/
     /* reset sidescroll so that 1st col is displayed                   */
     /*******************************************************************/
     pDoc->lSideScroll = 0;
     EQFBWorkLineOut( pDoc );                      // save current segment

     memset( pDoc->TBRowOffset, 0 , sizeof(TBROWOFFSET) * MAX_SCREENLINES );

     EQFBScrnLinesFromSeg ( pDoc,
                            lFocus - 1,           // starting row
                            (pDoc->lScrnRows -
                               lFocus + 1),      // number of rows
                            &(pDoc->TBCursor));  // starting segment

     EQFBFillPrevTBRow ( pDoc, lFocus);          // fill top of table

                                                 // check if top row is filled
     if ( !((pDoc->TBRowOffset+1)->ulSegNum) )
     {
        usI = 1;
        while ( !(pDoc->TBRowOffset+usI)->ulSegNum && (lFocus>0))
        {
           lFocus--;
           usI++;
        } /* endwhile */
        EQFBScrnLinesFromSeg ( pDoc,
                               lFocus - 1,// starting row
                               (pDoc->lScrnRows -
                                   lFocus + 1),     // number of rows
                               &(pDoc->TBCursor));  // starting segment

        EQFBFillPrevTBRow ( pDoc, lFocus    );      // fill top of table
     } /* endif */

     if ( !usSegOffset )                            // position at start of segm.
     {
        pDoc->EQFBFlags.EndOfSeg = FALSE;           // reset EndOfSeg flag
        EQFBFuncStartSeg( pDoc );
     }
     else
     {
       EQFBPhysCursorFromSeg(pDoc);                 //position at SegOffset
       pDoc->lDBCSCursorCol = pDoc->lCursorCol;     //set to actual Col
     } /* endif */

     pDoc->Redraw |= REDRAW_ALL;                    // redraw all
     EQFBWorkLineIn( pDoc );                        // get segment in work buffer

     EQFBScreenCursorType( pDoc );                  // set cursor shape correctly

   } /* endif */
   return;
}



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBCompSeg - compile new segment coordinates
//------------------------------------------------------------------------------
// Function call:     EQFBCompSeg( PTBSEGMENT );
//
//------------------------------------------------------------------------------
// Description:       compile the new segment identifiers
//
//------------------------------------------------------------------------------
// Parameters:        PTBSEGMENT        pointer to segment
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     get the new length of the segment
//                    force a ïreparsingï of the segment
//
//------------------------------------------------------------------------------

VOID
EQFBCompSeg
(
  PTBSEGMENT  pTBSeg                              // pointer to segment
)
{
   pTBSeg->usLength = (USHORT)UTF16strlenCHAR( pTBSeg->pDataW );     // set new size
   UtlAlloc( (PVOID *)&(pTBSeg->pusBPET) ,0L ,0L , NOMSG);   // force recompilation
   UtlAlloc( (PVOID *)&(pTBSeg->pusHLType) ,0L ,0L , NOMSG);   // del. highlighting
   pTBSeg->SegFlags.Spellchecked = FALSE;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGetVisSeg - get next visible seg
//------------------------------------------------------------------------------
// Function call:     EQFBGetVisSeg( PTBDOCUMENT, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       get the next segment which is a real seg -
//                    not a middle part of a joined segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance
//                    PUSHORT           pointer to segment number
//
//------------------------------------------------------------------------------
// Returncode type:   PTBSEGMENT
//------------------------------------------------------------------------------
// Returncodes:       pointer to segment
//                    NULL if not available
//
//------------------------------------------------------------------------------
// Function flow:     - get pointer to the requested segment number
//                    - loop until you have skipped any joined segments
//
//------------------------------------------------------------------------------

PTBSEGMENT
EQFBGetVisSeg
(
   PTBDOCUMENT pDoc,                      // pointer to Document ida
   PULONG      pulSegNum                  // pointer to segment number
)
{
   PTBSEGMENT pSeg;                       // pointer to segment
   SHORT sSign = 1;

   pSeg = EQFBGetSegW( pDoc,*pulSegNum );
   while (pSeg && pSeg->SegFlags.Joined)
   {
      (*pulSegNum)++;
      pSeg = EQFBGetSegW( pDoc, *pulSegNum );
   } /* endwhile */
   /*******************************************************************/
   /* if segment is shrinked, handle shrinked blocks                  */
   /*******************************************************************/
   if ( !pDoc->hwndRichEdit )
   {
    if ( pSeg && ISQFNOP(pSeg->qStatus) &&
        ( (pDoc->DispStyle==DISP_SHRINK) || (pDoc->DispStyle==DISP_COMPACT)) )
    {
       pSeg = EQFBGetShrinkSeg( pDoc, pulSegNum, sSign);
    } /* endif */
   } /* endif */

   return( pSeg );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBGetPrevVisSeg - get prev visible seg
//------------------------------------------------------------------------------
// Function call:     EQFBGetPrevVisSeg( PTBDOCUMENT, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       get the prev segment which is a real seg -
//                    not a middle part of a joined segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance
//                    PUSHORT           pointer to segment number
//
//------------------------------------------------------------------------------
// Returncode type:   PTBSEGMENT
//------------------------------------------------------------------------------
// Returncodes:       pointer to the segment
//                    NULL if segment not available
//------------------------------------------------------------------------------
// Function flow:     - get pointer to the requested segment number
//                    - loop until you have skipped any joined segments
//
//------------------------------------------------------------------------------

PTBSEGMENT
EQFBGetPrevVisSeg
(
   PTBDOCUMENT pDoc,                      // pointer to Document ida
   PULONG      pulSegNum                  // pointer to segment number
)
{
   PTBSEGMENT pSeg;                              // pointer to segment
   SHORT sSign = -1;

   pSeg = EQFBGetSegW( pDoc,*pulSegNum );

   // skip joined segments
   // P019981: skip "EMPTY_SEG" segments (with usLen=0 and SegFlags.NoWrite=1) too!

   while (pSeg && (pSeg->SegFlags.Joined ||
              (pSeg->SegFlags.NoWrite && pSeg->usLength == 0)))
   {
      (*pulSegNum)--;
      pSeg = EQFBGetSegW( pDoc, *pulSegNum );
   } /* endwhile */

   /*******************************************************************/
   /* if segment is shrinked, handle shrinked blocks                  */
   /*******************************************************************/
   if ( !pDoc->hwndRichEdit )
   {
     if ( pSeg && ISQFNOP(pSeg->qStatus) &&
        ( (pDoc->DispStyle==DISP_SHRINK) || (pDoc->DispStyle==DISP_COMPACT)) )
     {
       pSeg = EQFBGetShrinkSeg( pDoc, pulSegNum, sSign);
     } /* endif */
   } /* endif */

   return( pSeg );
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBGetShrinkSeg
//------------------------------------------------------------------------------
// Function call:     EQFBGetShrinkSeg(pDoc,&ulSegNum,sSign)
//------------------------------------------------------------------------------
// Description:       get ptr to segment
//                    if it has to be shrinked, return ptr to shrinked seg
//                    and whether it contained a linefeed
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT           document instance
//                    PUSHORT               ptr to segment number
//                    SHORT                 direction where to go on
//
//------------------------------------------------------------------------------
// Returncode type:   PTBSEGMENT            ptr to segment structure
//------------------------------------------------------------------------------
// Returncodes:       _                    ptr to segment
//                                         NULL if not available
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------

PTBSEGMENT
EQFBGetShrinkSeg(PTBDOCUMENT pDoc,        //ptr to doc structure
                 PULONG  pulSegNum,       //ptr to segment number
                 SHORT sSign )            //direction where to search
{
  PTBSEGMENT pSeg = NULL;                 //ptr to segment structure
  PSZ_W  pTemp;                           //ptr to string
  BOOL   fLineFeed = FALSE;               //true if linefeed in seg
  CHAR_W c ;
  ULONG  ulOurSegNum;
  BOOL   fMarked = FALSE;                 // segment marked?
  BOOL   fTRNote = FALSE;                 // true if TRNote found in NOP Block

  ulOurSegNum = *pulSegNum;
  /********************************************************************/
  /* check if at start of block -                                     */
  /* hence check whether previous is to be shrinked too               */
  /********************************************************************/
  ulOurSegNum -= sSign;
  if ( ulOurSegNum && (ulOurSegNum < (pDoc->ulMaxSeg)))           /* @39A */
  {                                                               /* @39A */
    pSeg = EQFBGetSegW( pDoc, ulOurSegNum);                 //get  segment
    if (pSeg )
    {
      fMarked |= pSeg->SegFlags.Marked;
    } /* endif */
  } /* endif */                                                   /* @39A */
  if ( pSeg && ISQFNOP(pSeg->qStatus) && (!pSeg->SegFlags.Joined))
  {
    /******************************************************************/
    /* previous belongs to shrinked block too i.e.                    */
    /* we are in the middle of shrinked block - hence:                */
    /* scan til next seg which is not to be shrinked                  */
    /******************************************************************/
    fTRNote |= EQFBIsTRNoteInNOP(pDoc, pSeg);
    ulOurSegNum += sSign;                        //point to cur segnum again
    pSeg = EQFBGetSegW( pDoc, ulOurSegNum);    //get previous segment

    while ( pSeg && ISQFNOP(pSeg->qStatus) && (!pSeg->SegFlags.Joined))
    {
      fTRNote |= EQFBIsTRNoteInNOP(pDoc, pSeg);
      ulOurSegNum +=sSign;
      pSeg = EQFBGetSegW( pDoc, ulOurSegNum);    //get  segment
    } /* endwhile */
    /******************************************************************/
    /* now pSeg points to the 1st seg after the shrinked block        */
    /* or outside of range                                            */
    /******************************************************************/
    *pulSegNum = ulOurSegNum;
  }
  else
  {
    if ( (pDoc->docType == VISSRC_DOC ) ||               // ITM-VIS
         (pDoc->docType == VISTGT_DOC )   )              // ITM-VIS
    {                                                    // ITM-VIS
      fLineFeed = TRUE;                                  // ITM-VIS
    } /* endif */                                        // ITM-VIS
    /******************************************************************/
    /* we are at start of shrinked block                              */
    /******************************************************************/
    ulOurSegNum += sSign;                        //point to cur segnum again
    pSeg = EQFBGetSegW( pDoc, ulOurSegNum);                 //get segment
    if (pSeg )
    {
      fMarked |= pSeg->SegFlags.Marked;
    } /* endif */
    if ( sSign == 1 )
    {
      while ( !fLineFeed && pSeg && ISQFNOP(pSeg->qStatus) )
      {
        fTRNote |= EQFBIsTRNoteInNOP(pDoc, pSeg);
        pTemp = pSeg->pDataW;                   //check for linefeed
        fMarked |= pSeg->SegFlags.Marked;
        while ( !fLineFeed && ( (c = *pTemp)!= NULC ) )
        {
         fLineFeed = (c == LF);
         pTemp++;
        } /* endwhile */

        ulOurSegNum +=sSign;
        pSeg = EQFBGetSegW( pDoc, ulOurSegNum);    //get segment
      } /* endwhile */
    }
    else                                          //sSIgn = -1
    {
      /****************************************************************/
      /* loop backward til the 1st seg of shrinked block              */
      /****************************************************************/
      while ( pSeg && ISQFNOP(pSeg->qStatus)
                   && (!pSeg->SegFlags.Joined))
      {
        fTRNote |= EQFBIsTRNoteInNOP(pDoc, pSeg);
        pTemp = pSeg->pDataW;                   //check for linefeed
        fMarked |= pSeg->SegFlags.Marked;
        while ( !fLineFeed && ( (c = *pTemp)!= NULC )  )
        {
         fLineFeed = (c == LF);
         pTemp++;
        } /* endwhile */

        ulOurSegNum +=sSign;
        pSeg = EQFBGetSegW( pDoc, ulOurSegNum);    //get segment
      } /* endwhile */

      ulOurSegNum -= sSign;
      *pulSegNum = ulOurSegNum;                 //point to 1st shrinked

    } /* endif */

  /*******************************************************************/
  /* if display style is SHRINK or COMPACT return pointer to         */
  /* static segment.                                                 */
  /*******************************************************************/
    pSeg = &ShrinkSeg;
    memset(&pSeg->SegFlags, 0, sizeof( pSeg->SegFlags));
    pSeg->SegFlags.Marked = (USHORT)fMarked;
    pSeg->ulSegNum = *pulSegNum;

    if ( !fTRNote )
    {
      pSeg->pDataW = (fLineFeed) ? (pDoc->szOutTagLFAbbrW):(pDoc->szOutTagAbbrW);
    }
    else
    {

      pSeg->pDataW = (fLineFeed) ?
                     (pDoc->chTRNoteLFAbbrW):(pDoc->chTRNoteAbbrW); //EQFBUserOpt.chTRNoteAbbr);
      // TRNote displayed as "<T" if docopen with Compact style and Auto-linewrap on
      // in Standard editor; hope the UTlalloc will fix it
      UtlAlloc((PVOID *)&(pSeg->pusBPET), 0L, 0L, NOMSG );
    } /* endif */

    pSeg->usLength = (USHORT)(UTF16strlenCHAR(pSeg->pDataW));
  } /* endif */


  return (pSeg);
} /* end of function EQFBGetShrinkSeg */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFBIsTRNoteInNOP
//------------------------------------------------------------------------------
// Function call:     EQFBIsTRNoteInNOP(pDoc, pSeg)
//------------------------------------------------------------------------------
// Description:       check whether seg contains TRNOTE
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT           document instance
//                    PTBSEGMENT            ptr to segment
//------------------------------------------------------------------------------
// Returncode type:   BOOL                  TRUE if TRNOTE is segmnet
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------

static BOOL
EQFBIsTRNoteInNOP(PTBDOCUMENT pDoc,        //ptr to doc structure
                 PTBSEGMENT pSeg )         //ptr to segment
{
  BOOL  fTRNote = FALSE;
   PSTARTSTOP  pstCurrent;
   USHORT      usColPos = 0;
   USHORT      usRC = NO_ERROR;

   if (pSeg && (pSeg->ulShrinkLen == SHRINKLEN_NOTYETCALC ))
   {
     if ((pDoc->hwndRichEdit) && (pSeg->pusBPET) )
     {
       UtlAlloc( (PVOID *)&(pSeg->pusBPET) ,0L ,0L , NOMSG);   // force recompilation
     } /* endif */
     if (pSeg->pusBPET == NULL )
     {
       usRC = TACreateProtectTableW(pSeg->pDataW,
                                   pDoc->pDocTagTable,
                                   usColPos,
                                   (PTOKENENTRY) pDoc->pTokBuf,
                                   TOK_BUFFER_SIZE,
                                   (PSTARTSTOP * ) &(pSeg->pusBPET),
                                   pDoc->pfnUserExit,
                                   pDoc->pfnUserExitW, pDoc->ulOemCodePage);
     } /* endif */
     if (!usRC )
     {
       pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
       while ( ( pstCurrent->usType != 0) &&
               (pstCurrent->usType != TRNOTE_CHAR) )
       {
         pstCurrent++;
       } /* endwhile */
       if (pstCurrent->usType == TRNOTE_CHAR )
       {
         USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
         pSeg->ulShrinkLen = strlen(pEQFBUserOpt->chTRNoteAbbr);
         fTRNote = TRUE;
       }
       else
       {
         pSeg->ulShrinkLen = 0;               // no TRNOTE indicator necessary
       } /* endif */
     } /* endif */
   }
   else
   {
     if (pSeg && (pSeg->ulShrinkLen != 0 ))
     {
       fTRNote = TRUE;
     } /* endif */
   } /* endif */

   return (fTRNote);
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBPosGotoSeg   find offset of given word in segment
//------------------------------------------------------------------------------
// Function call:     EQFBPosGotoSeg( PTBDOCUMENT, USHORT, PSZ, SHORT );
//
//------------------------------------------------------------------------------
// Description:       find given word in segment
//
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT       pointer to document instance
//                    USHORT            number of segment
//                    PSZ               word to search for in segmnet
//                    SHORT             segoffset where to start
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       segment offset
//
//------------------------------------------------------------------------------
// Function flow:     get pointer to segment
//
//
//------------------------------------------------------------------------------

SHORT
EQFBPosGotoSeg
(
   PTBDOCUMENT pDoc,                      // pointer to Document ida
   ULONG  ulSegNum,                       // segment number
   PSZ_W  pWord,                          // ptr to given word
   SHORT  sStartOffset                    // segoffset where to start
)
{
   PTBSEGMENT pTBSeg;                     // ptr to segment
   SHORT      sSegOffset;                 // segoffset
   PSZ_W      pFoundPos;                  // pos where word is perhaps found
   PSZ_W      pFoundFirst = NULL;         // pos where word is found 1st
   BOOL       fFound = FALSE;             // TRUE if word found ( if word
                                          // delimiter at begin + end of word)
   USHORT     usCnt = 0;                  // cnt in loop (0 to strlen(word))
   USHORT     usWordlen;                         // strlen (pWord)
   USHORT     usType;                     //return from EQFBCharType

   pTBSeg = EQFBGetSegW(pDoc,ulSegNum);

   // check for word delimiter at begin and end of word
   // while not found or end of segment
   pFoundPos = pTBSeg->pDataW + sStartOffset;
   while (!fFound && (pFoundPos != NULL))
   {
     pFoundPos = UTF16strstr(pFoundPos,pWord) ;

     // check at end of word and begin of word
     if (pFoundPos != NULL)
     {
       //check whether pFoundPos points to a word with only
       // unprotected chars
        usCnt = 0;                                // init counter
        usWordlen = (USHORT)(UTF16strlenCHAR(pWord));       //check all chars of word
        sSegOffset = (SHORT)(pFoundPos - (pTBSeg->pDataW));
        while (usCnt != usWordlen &&
              EQFBCharType(pDoc,pTBSeg,(SHORT)(sSegOffset+usCnt)) == UNPROTECTED_CHAR)
        {
              usCnt ++;
        } /* endwhile */
        //if all chars are unprotected, check end and begin of word
       if (usCnt == usWordlen)
       {
          if (pFoundFirst == NULL)
          {
             pFoundFirst = pFoundPos;              // in case no more match will
          } /* endif */                            // be found...

          /************************************************************/
          /* a protected char in front of a word is a word delimiter  */
          /* a compact character too                                  */
          /************************************************************/
          sSegOffset = (SHORT)(pFoundPos + usWordlen - (pTBSeg->pDataW));
          usType = EQFBCharType(pDoc,pTBSeg,sSegOffset) ;
          if ( usType == PROTECTED_CHAR || usType == COMPACT_CHAR )
          {
            fFound = TRUE;
          }
          else
          {
			      //if (pDoc->ulOemCodePage == 850 )
			      //{	 //if no char->end found, set fFound=FALSE - old approach
				     // CHAR chOEM[2];

				     // chOEM[0] = (UCHAR)*(pFoundPos + UTF16strlenCHAR(pWord));
				     // chOEM[1] = EOS;
				     // EQFAnsiToOem( chOEM, chOEM );
				     // fFound = !chDictLookup[chOEM[0]];
		       // }
		       // else
		        {  // new approach - P018766
    				 fFound = !iswalnum(*(pFoundPos + UTF16strlenCHAR(pWord)));
		        }
          } /* endif */

          if (fFound && (pFoundPos != (pTBSeg->pDataW)))    // check at begin of word
          {
            //a protected char at pos 'pFoundPos-1' is a word delimiter
            sSegOffset = (SHORT)(pFoundPos - (pTBSeg->pDataW)-1);
            usType = EQFBCharType(pDoc,pTBSeg,sSegOffset) ;
            if ( usType != PROTECTED_CHAR && usType != COMPACT_CHAR )
            {
			        //if (pDoc->ulOemCodePage == 850 )
			        //{
				       // CHAR chOEM[2];
				       // chOEM[0] = (UCHAR)*(pFoundPos - 1);
				       // chOEM[1] = EOS;
				       // EQFAnsiToOem( chOEM, chOEM );

				       // fFound = !chDictLookup[chOEM[0]];
			        //}
			        //else
			        {
                 fFound = !iswalnum(*(pFoundPos - 1));
    		      }
            } /* endif */
           } /* endif */

          if (!fFound)                                     //reset pFoundPos for
          {                                                // next search
             pFoundPos = pFoundPos + UTF16strlenCHAR(pWord);
          } /* endif */
       }
       else
       {                                              //reset pFoundPos for
          pFoundPos = pFoundPos + 1;                  // next search in case
       } /* endif */                                  //of protected chars
     } /* endif */
   } /* endwhile */
   if (fFound)
   {
      sSegOffset = (SHORT)(pFoundPos - (pTBSeg->pDataW));
   }
   else
   {
      if (pFoundFirst)                             // match found, but not
      {                                            // a separate word
         sSegOffset = (SHORT)(pFoundFirst - (pTBSeg->pDataW));
      }
      else
      {                                            // no match found,error..
         sSegOffset = -1;
      } /* endif */
   } /* endif */
   return (sSegOffset);
}
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFBActTrans
//------------------------------------------------------------------------------
// Function call:     EQFBActTrans(PTBDOCUMENT)
//------------------------------------------------------------------------------
// Description:       activates the translation window
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc   document instance
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------

VOID
EQFBActTrans
(
  PTBDOCUMENT pDoc                      // pointer to document ida
)
{
   PTBDOCUMENT pDocTemp;                // temporary document pointer
  /********************************************************************/
  /* function is only active if not in TARGET_DOCUMENT                */
  /********************************************************************/

//if ( pDoc->docType != STARGET_DOC )
  {
    pDocTemp = pDoc;
    while ( pDocTemp->docType != STARGET_DOC )
    {
      pDocTemp = pDocTemp->next;
    } /* endwhile */

    pDoc = pDocTemp;
    pDoc->Redraw |= REDRAW_ALL;
                                          // activate document window
    WinShowWindow( pDoc->hwndFrame, TRUE );
    WinSendMsg( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                  WM_EQF_SETFOCUS, 0, MP2FROMHWND(pDoc->hwndFrame));
  }
//else
//{
//   WinAlarm( HWND_DESKTOP, WA_WARNING ); // beep if nothing changed
//} /* endif */

} /* end of function EQFBActTrans */



// Determine if we are in active segment or on the first character of the next segment....
BOOL EQFBStillInActSeg( PTBDOCUMENT pDoc )
{
  return (pDoc->tbActSeg.ulSegNum == pDoc->TBCursor.ulSegNum); // || (((pDoc->tbActSeg.ulSegNum + 1) == pDoc->TBCursor.ulSegNum) && (pDoc->TBCursor.usSegOffset == 0));
}

void EQFBGetMatchFlags( PTBDOCUMENT pDoc )
{
  USHORT  usState = 0;
  USHORT  usFuzzy = 0;

  memset( &(pDoc->ActSegLog.PropTypeExists), 0, sizeof(pDoc->ActSegLog.PropTypeExists));

  usState = EqfGetPropState((PSTEQFGEN)pDoc->pstEQFGen, 0);

  // get fuzziness of first fuzzy proposal
  {
    USHORT i = 0;
    PSTEQFGEN pstEqfGen = (PSTEQFGEN)pDoc->pstEQFGen;
    PDOCUMENT_IDA pDocIda = (PDOCUMENT_IDA)pstEqfGen->pDoc;
    while ( (usFuzzy == 0) && (i < EQF_NPROP_TGTS) )
    {
      USHORT usCurFuzzy = (pDocIda->stEQFSab + pDocIda->usFI)->usFuzzyPercents[i];
      if ( usCurFuzzy < 100 ) usFuzzy = usCurFuzzy;
      i++;
    } /* endwhile */
  }


  if ( usState & EXACT_PROP )                   pDoc->ActSegLog.PropTypeExists.Exact = TRUE; 
  if ( usState & REPLACE_PROP )                 pDoc->ActSegLog.PropTypeExists.Replace = TRUE;
  if ( (usState & FUZZY_REPLACE_PROP) && (usFuzzy >= 50)) pDoc->ActSegLog.PropTypeExists.Fuzzy = TRUE;
  if ( (usState & FUZZY_PROP) && (usFuzzy >= 50)) pDoc->ActSegLog.PropTypeExists.Fuzzy = TRUE;
  if ( usState & MACHINE_TRANS_PROP )           pDoc->ActSegLog.PropTypeExists.MT = TRUE;
  if ( usState & GLOBMEM_TRANS_PROP )           pDoc->ActSegLog.PropTypeExists.GlobMem = TRUE;
  if ( usState & GLOBMEMSTAR_TRANS_PROP )       pDoc->ActSegLog.PropTypeExists.GlobMem = TRUE;
  if ( usState & GLOBMEMFUZZY_TRANS_PROP )      pDoc->ActSegLog.PropTypeExists.GlobMemFuzzy = TRUE;
}


// helper function: copy specific fields from additional data area
VOID AddData_CopyFieldsFromList( PSZ_W pszTarget, PSZ_W pszSource, PSZ_W pszFieldList )
{
  PSZ_W pszField = NULL;
  PSZ_W pszKeyStart = NULL;
  PSZ_W pszKeyEnd = NULL;
  PSZ_W pszEntryEnd = NULL;
  BOOL fFound = FALSE;
  CHAR_W chTemp;

  while( *pszSource != 0 )
  {
    fFound = FALSE;

    // find start of next key
    pszKeyStart = pszSource;
    while ( (*pszKeyStart != 0) && (*pszKeyStart != L'<') ) pszKeyStart++;  

    // find end of key
    pszKeyEnd = pszKeyStart;
    while ( (*pszKeyEnd != 0) && (*pszKeyEnd != L'>') && (*pszKeyEnd != L' ') ) pszKeyEnd++;  

    // handle entry
    if ( (*pszKeyStart == L'<') && ((*pszKeyEnd == L'>') || ((*pszKeyEnd == L' '))) )
    {
      // find end of entry
      {
        pszEntryEnd = pszKeyEnd + 1;

        // skip entry data
        while ( (*pszEntryEnd != 0) && (*pszEntryEnd != L'<') ) pszEntryEnd++;  

        // skip endkey
        while ( (*pszEntryEnd != 0) && (*pszEntryEnd != L'>') ) pszEntryEnd++;  
        if ( *pszEntryEnd == L'>') pszEntryEnd++;  
      }

      // search key in our list
      pszField = pszFieldList;
      chTemp = *pszKeyEnd;
      *pszKeyEnd = 0;
      while( !fFound && (*pszField != 0) )
      {
        if ( wcsicmp( pszField, pszKeyStart + 1 ) == 0 )
        {
          fFound = TRUE;
        }
        else
        {
          pszField += wcslen(pszField) + 1;
        } /* endif */           
      
      } /* endwhile */         
      *pszKeyEnd = chTemp;

      // copy any found entry
      if ( fFound )
      {
        while ( pszKeyStart < pszEntryEnd )
        {
          *pszTarget++ = *pszKeyStart++;
        } /* endwhile */           
      } /* endif */       

      // continue with next entry
      pszSource = pszEntryEnd;
    }
    else
    {
      pszSource = pszKeyEnd;
    } /* endif */       

  }
  *pszTarget = 0;
}
