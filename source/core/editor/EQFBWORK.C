/*! \file
	Description: This module contains the editor 'work line' and a set of routines to help manipulate it.
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_ANALYSIS         // analysis function
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                       // General Translation Manager include file

#include "EQFTPI.H"                    // Translation Processor priv. include file
#include "eqfdoc00.h"             // private document handler include file
#include <process.h>

static VOID   EQFBScrnSpellCheck ( PTBDOCUMENT  );
static VOID   EQFBActSegSpellCheck ( PTBDOCUMENT );
static VOID   EQFBFileSpellCheck ( PTBDOCUMENT );
static BOOL   EQFBSegSpellCheck ( PTBDOCUMENT, ULONG, PSPELLDATA, PBOOL);
static USHORT EQFDoProofSeg ( PTBDOCUMENT, PSZ_W, PSZ_W, PUSHORT );

static BOOL EQFBSegIsVisible ( PTBDOCUMENT, ULONG, ULONG );

static VOID   EQFBThreadAutoSave ( PTBDOCUMENT );
static VOID   EQFBThreadAutoSubst ( PTBDOCUMENT );

#define NEXTNUMTOSPELL  10

#ifdef ITMTEST
  FILE *fOut;                          // test output
#endif

//------------------------------------------------------------------------------
// Function name:      EQFBWorkSegCheck ()  - check if work segment if correct  
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBWorkSegCheck( PTBDOCUMENT );                          
//                                                                              
//------------------------------------------------------------------------------
// Description  :     check if work segment is correct filled in the case       
//                    of PostEdit                                               
//                                                                              
//------------------------------------------------------------------------------
// Parameter:          PTBDOCUMENT   - pointer to document ida                  
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Side effects:       Work Segment is refilled                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:      if PostEdit && Worksegment != cursor segment then        
//                        - call EQFBWorkSegOut                                 
//                        - call EQFBWorkSegIn                                  
//                     endif                                                    
//------------------------------------------------------------------------------
VOID EQFBWorkSegCheck
(
  PTBDOCUMENT pDoc
)
{
   if ( pDoc->EQFBFlags.PostEdit &&
           pDoc->ulWorkSeg != pDoc->TBCursor.ulSegNum )
   {
      EQFBWorkLineOut( pDoc );        // check if we have to save a segm. in TM
      EQFBWorkSegIn( pDoc );
   } /* endif */
}


//------------------------------------------------------------------------------
// Function name:      EQFBWorkLineIn()  copy segment to work segment           
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBWorkLineIn( PTBDOCUMENT );                            
//                                                                              
//------------------------------------------------------------------------------
// Description  :     Make a copy of the current document segment into          
//                    the work segment.                                         
//                                                                              
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     - if PostEdit and WorkSegment != cursor segment then      
//                         copy segment data into worksegment                   
//                      endif                                                   
//                                                                              
//------------------------------------------------------------------------------
 void EQFBWorkLineIn
 (
   PTBDOCUMENT pDoc                     //pointer to Doc instance
 )
 {
    if ( pDoc && pDoc->EQFBFlags.PostEdit &&
            (pDoc->TBCursor.ulSegNum != pDoc->ulWorkSeg))
    {
       EQFBWorkSegIn( pDoc );
    } /* endif */
    return;
 }

//------------------------------------------------------------------------------
// Function name:      EQFBWorkSegIn()  copy segment to work segment            
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBWorkSegIn( PTBDOCUMENT );                             
//                                                                              
//------------------------------------------------------------------------------
// Description  :     Make a copy of the current document segment into          
//                    the work segment.                                         
//                                                                              
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     - copy data into worksegment, set pSaveSeg and segment    
//                      number                                                  
//                                                                              
//------------------------------------------------------------------------------
 void EQFBWorkSegIn
 (
   PTBDOCUMENT pDoc                              // pointer to Doc instance
 )
 {
    PSZ_W         pData;                         // pointer to data
    PTBSEGMENT    pSeg;                          // ptr to active segment
    ULONG         ulSegNum;                      // segment number

    ulSegNum = pDoc->TBCursor.ulSegNum;          // current seg num

    pSeg = EQFBGetSegW( pDoc, ulSegNum );
    pData = ( pSeg ) ? pSeg->pDataW : NULL;       // set data pointer
    pDoc->pSaveSegW = pData;
    if ( pData )
    {
       if (pDoc->pUndoSegW)                       // if allocated(=not readonly
       {                                         // document)
          UTF16strcpy(pDoc->pUndoSegW,pData);     // fill for later undo handling
          pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
          pDoc->fUndoState = FALSE;
       } /* endif */
       UTF16strcpy(pDoc->pEQFBWorkSegmentW, pData);// copy segment data
       pSeg->pDataW = pDoc->pEQFBWorkSegmentW;      // set ptr to seg data
    }
    else
    {
       ulSegNum = 0;
    } /* endif */
    pDoc->ulWorkSeg = ulSegNum;                            // save working segm.
    pDoc->pTBSeg = EQFBGetSegW(pDoc,ulSegNum);
    pDoc->EQFBFlags.workchng = FALSE;

     /*****************************************************************/
     /* init logging structure                                        */
     /*****************************************************************/
     memset( &(pDoc->ActSegLog), 0, sizeof(pDoc->ActSegLog) );
     pDoc->ActSegLog.ulTime = GetTickCount();
     pDoc->ActSegLog.ulSegNum = ulSegNum;

    return;
 }


//------------------------------------------------------------------------------
// Function name:     EQFBWorkSegOut ()  - save work segment                    
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBWorkSegOut( PTBDOCUMENT );                            
//                                                                              
//------------------------------------------------------------------------------
// Description  :     save working segment                                      
//                                                                              
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     get rid of trailing blanks in a line                      
//                    get rid of superfluous :NONE tags                         
//                    save working segment                                      
//                    in postedit mode readjust display                         
//                                                                              
//------------------------------------------------------------------------------

 void EQFBWorkSegOut
 (
   PTBDOCUMENT pDoc                     //pointer to Doc instance
 )

 {
    USHORT usI;                                          // length of string
    USHORT usLen;                                        // length of Alloced
    USHORT usStart;                                      // start of string
    ULONG  ulSegNum;                                     // current segment
    PSZ_W  pChar;                                        // pointer to string
    PSZ_W  pTarget;                                      // pointer to target
    PSZ_W  pLast;                                        // pointer to last char
    BOOL   fSearch;                                     // search for blanks
    BOOL   fNoneRemoved = FALSE;                          // 1 if none removed
    BOOL   fSkipped = FALSE;

    ulSegNum = pDoc->TBCursor.ulSegNum;                  // current seg num
    if ( ulSegNum )
    {
      /****************************************************************/
      /* fill in worksegment with data from RichEdit control...       */
      /****************************************************************/

       if ( pDoc->hwndRichEdit && pDoc->ulWorkSeg )
       {
         EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );
       } /* endif */

       if (pDoc->EQFBFlags.workchng && pDoc->pTBSeg->pDataW )
       {
          pDoc->flags.changed = TRUE;
          // check if :NONE tag will be in a normal segment, then get rid of it
          fNoneRemoved = EQFBCheckNoneTag( pDoc, pDoc->pEQFBWorkSegmentW );
          /* Strip trailing blanks before saving line */

          usI = (USHORT)UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW );
          if ( usI )
          {
            usStart = usI ;
            pChar = pDoc->pEQFBWorkSegmentW + usStart - 1;// point to last char
            pTarget = pLast = pChar;

             // search for blanks only if no seg is following in same line
             if ( *pChar == L'\n' )
             {
                 fSearch = TRUE;                         // search for blanks
             }
             else
             {
                 fSearch = FALSE;                        // ignore blanks
                *pTarget-- = *pChar--;                   // copy character
                usStart--;
             } /* endif */


            while ( usStart )
            {
               if ( fSearch )                            // skip blanks
               {
                  while ( usStart && *pChar == ' ' &&
                    (EQFBCharType(pDoc, pDoc->pTBSeg, (USHORT)(usStart-1))
                                  == UNPROTECTED_CHAR))
                  {
                     pChar--;
                     usStart--;
                     fSkipped = TRUE;
                  } /* endwhile */
                  fSearch = FALSE;
               }
               else
               {
                  fSearch = ( *pChar == '\n');
                  *pTarget-- = *pChar--;             // copy character
                  usStart--;
               } /* endif */
            } /* endwhile */
            pTarget++;                               //weïve gone one too far
          }
          else
          {
             pLast = pTarget = pDoc->pEQFBWorkSegmentW; // init pointers
          } /* endif */
          usI = (USHORT)(pLast - pTarget + 1);                  // get new length

          usI = (USHORT)UTF16strlenCHAR(pTarget);

          usLen = max( (usI+1)*sizeof(CHAR_W), MIN_ALLOC );            // get rid of warning
                                                         // in UtlAlloc
          if ( UtlAlloc((PVOID *) &(pDoc->pTBSeg->pDataW),
                         0L, (LONG) (usLen), ERROR_STORAGE ))
          {
            UTF16strcpy( pDoc->pTBSeg->pDataW, pTarget );
            pDoc->pTBSeg->usLength = usI;
            if (pDoc->pSaveSegW) UtlAlloc((PVOID *) &(pDoc->pSaveSegW), 0L, 0L, NOMSG );// free old storage
            if (pDoc->pSaveSeg) UtlAlloc((PVOID *) &(pDoc->pSaveSeg), 0L, 0L, NOMSG );
            UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusBPET), 0L, 0L, NOMSG );// free start/stop
            if (pDoc->pTBSeg->pusHLType )
            {
              pDoc->pTBSeg->SegFlags.Spellchecked = FALSE;
              UtlAlloc((PVOID *)&(pDoc->pTBSeg->pusHLType) ,0L ,0L , NOMSG);
            } /* endif */
          }
          else                                           // reset old one
          {
            pDoc->pTBSeg->pDataW = pDoc->pSaveSegW;
          } /* endif */
          /************************************************************/
          /* make sure that EQFBWorkSeg is correct if NONEtag was     */
          /* deleted or other changes                                 */
          /************************************************************/

          UTF16strcpy(pDoc->pEQFBWorkSegmentW, pDoc->pTBSeg->pDataW);

          /************************************************************/
          /* in postedit mode readjust screen start positions -       */
          /* not necessary in any other mode because there will be    */
          /* a EQFBGotoSeg in any other case                          */
          /************************************************************/
          if ( pDoc->EQFBFlags.PostEdit || fNoneRemoved || fSkipped)
          {
             EQFBScrnLinesFromSeg ( pDoc,              // pointer to doc ida
                                    0,                 // starting row
                                    pDoc->lScrnRows,   // number of rows
                                                       // starting segment
                                    (pDoc->TBRowOffset+1));
          } /* endif */
          pDoc->EQFBFlags.workchng = FALSE;              // reset changes
       }
       else
       {
          if ( pDoc->pSaveSegW )
          {
            pDoc->pTBSeg->pDataW = pDoc->pSaveSegW;
          } /* endif */
       } /* endif */
       pDoc->pSaveSegW = NULL;                              // reset saved segm.
    } /* endif */

    return;
 }

//------------------------------------------------------------------------------
// Function name:      EQFBWorkLineOut()  - change of cursor in active segment  
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBWorkLineOut( PTBDOCUMENT );                           
//                                                                              
//------------------------------------------------------------------------------
// Description  :     if changes in work segment save it to file                
//                                                                              
//------------------------------------------------------------------------------
// Function flow:     check if post edit mode and cursor segment                
//                    is different to work segment                              
//                                                                              
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//                                                                              
//------------------------------------------------------------------------------
void EQFBWorkLineOut
(
  PTBDOCUMENT pDoc                     //pointer to Doc instance
)
{
  PTBSEGMENT pSeg;
   if ( pDoc->EQFBFlags.PostEdit && pDoc->TBCursor.ulSegNum != pDoc->ulWorkSeg)
   {
      if ( pDoc->EQFBFlags.workchng )
      {                                                              /* @39A */
         if (!EQFBSaveSeg( pDoc ))     //if NO if inline tags changed/* @39C */
         {                                                           /* @39A */
           pSeg = EQFBGetSegW(pDoc,pDoc->ulWorkSeg);                  /* @39A */
           if ( pDoc->pSaveSegW )                                     /* @39A */
           {                                                         /* @39A */
             pSeg->pDataW = pDoc->pSaveSegW;                           /* @39A */
           } /* endif */                                             /* @39A */
           pDoc->pSaveSegW = NULL;                                    /* @39A */
           EQFBCompSeg(pSeg);                                        /* @39A */
           pDoc->Redraw |= REDRAW_ALL;      // indicate to update all/* @39A */
         } /* endif */                                               /* @39A */
      }
      else
      {
         EQFBWorkSegOut( pDoc );
      } /* endif */
   } /* endif */
   return;
}


//------------------------------------------------------------------------------
// Function name:      EQFBWorkRight ()  - shift work segment right             
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBWorkRight( PTBDOCUMENT );                             
//                                                                              
//------------------------------------------------------------------------------
// Function flow:     - get length of work segment                              
//                    - if overflow issue warning else shift the rest           
//                                                                              
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//                    USHORT        - position where to start                   
//                    USHORT        - number of characters                      
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Description  :     shift the worksegment from the passed position            
//                    count characters to the right.                            
//                    set work segment changed flag                             
//------------------------------------------------------------------------------
 void EQFBWorkRight
 (
    PTBDOCUMENT pDoc,
    USHORT      usPosition,
    USHORT      usCount
 )

 {
   ULONG ulLen;                         // length of string

   ulLen = UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW );

   if ( ulLen + usCount < MAX_SEGMENT_SIZE )
   {
     if ( ulLen >= usPosition )
     {
       memmove( pDoc->pEQFBWorkSegmentW+usPosition+usCount,
                pDoc->pEQFBWorkSegmentW+usPosition,
                (ulLen - usPosition + 1) * sizeof(CHAR_W));
     }
   }
   else
   {
     UtlError(TB_TOOLONG, MB_OK, 0, NULL, EQF_ERROR);
   } /* endif */

   pDoc->EQFBFlags.workchng = TRUE;
   return;
 }

//------------------------------------------------------------------------------
// Function name:     EQFBWorkLeft ()  - shift work segment to the left         
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBWorkLeft( PTBDOCUMENT );                              
//                                                                              
//------------------------------------------------------------------------------
// Description  :     shift the worksegment from the passed position            
//                    count characters to the left.                             
//                                                                              
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//                    USHORT        - position where to start                   
//                    USHORT        - number of characters                      
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//                                                                              
//------------------------------------------------------------------------------
// Function flow:     - copy the string from the passed position count          
//                      characters to the left using strcpy.                    
//                    - set worksegment changed flag                            
//------------------------------------------------------------------------------

void EQFBWorkLeft
(
   PTBDOCUMENT pDoc,
   USHORT      usPosition,
   USHORT      usCount
)

{
    PSZ_W    pData;               //ptr to position in Worksegment

    pData = pDoc->pEQFBWorkSegmentW+usPosition;
    UTF16strcpy(pData,pData+usCount);

    pDoc->EQFBFlags.workchng = TRUE;

    return;
}


//------------------------------------------------------------------------------
// Function name:     EQFBWorkThreadTask  thread for translation processor      
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBWorkThreadTask (pDoc, usTask)                         
//------------------------------------------------------------------------------
// Description  :     helper thread for spellcheck, filesave etc.               
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//                    USHORT        - task to be done in thread                 
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//                                                                              
//------------------------------------------------------------------------------
// Function flow:     if thread not active, start it                            
//                    if thread not busy: set new usTask and let thread do the  
//                     work                                                     
//------------------------------------------------------------------------------
VOID EQFBWorkThreadTask
(
  PTBDOCUMENT pDoc,
  USHORT      usTask
)
{
  if ( !pDoc->fThreadAct )
  {
    // activate language support from main thread - otherwise we are having problems with the Java interface to jFrost...
    {
      PDOCUMENT_IDA pIdaDoc = (PDOCUMENT_IDA)((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
      if ( pIdaDoc->sTgtLanguage == -1)
      {
        MorphGetLanguageID( pIdaDoc->szDocTargetLang, &pIdaDoc->sTgtLanguage );
        pIdaDoc->ulTgtOemCP = GetLangOEMCP( pIdaDoc->szDocTargetLang);
      } /* endif */
    }

    /******************************************************************/
    /* start thread                                                   */
    /******************************************************************/
    pDoc->fThreadAct = (EQF_BOOL)( _beginthread( EQFBWorkThread,
                                       WORKTHREAD_STACKSIZE, pDoc ) != -1);
    DosSleep(100L);
    // init spelldata for first task -- use ulSegNum not
    if (pDoc->pvSpellData)
    {
		((PSPELLDATA) pDoc->pvSpellData)->TBFirstLine.ulSegNum = 0xFFFFFFFF;
	}
  } /* endif */


  if ( !pDoc->fThreadAct )
  {
    /******************************************************************/
    /* issue error message                                            */
    /******************************************************************/
  }
  else
  {
    /******************************************************************/
    /* wait until thread ready for taking over additional work        */
    /******************************************************************/
    USHORT usI = 0;
    while ( pDoc->fThreadNotIdle && usI < 2 /*100 RJ 030321*/ )
    {
      DosSleep( 30L /*100L*/ );
      usI++;
    } /* endwhile */
    if ( !pDoc->fThreadNotIdle  )
    {
      /****************************************************************/
      /* set task and let thread do dirty work                        */
      /****************************************************************/
      if ((usTask == THREAD_SPELLFILE) && (usTask == pDoc->usThreadTask ))
      {
        /**************************************************************/
        /* do not spellcheck total doc twice                          */
        /* aber schon wenn irgendwo spellcheck info invalid wurde!    */
        /**************************************************************/
        usI = usI;
      }
      else
      {
        pDoc->usThreadTask = usTask;
        pDoc->fThreadNotIdle = TRUE;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* Thread busy -- pls try later                                 */
      /****************************************************************/
      usI = usI;
    } /* endif */
  } /* endif */
}

//------------------------------------------------------------------------------
// Function name:     EQFBWorkThread      thread for translation processor      
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBWorkThread     (pDoc, usTask)                         
//------------------------------------------------------------------------------
// Description  :     helper thread for spellcheck, filesave etc.               
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//                                                                              
//------------------------------------------------------------------------------
// Function flow:     while thread active                                       
//                      while thread not busy                                   
//                       sleep a little bit                                     
//                       if FileSpell called previously and not totally done    
//                         yet, goon with next portion of filespell             
//                       else AutoSpell ON, reparse current screen              
//                      switch(threadtask)                                      
//                        case THREAD_SPELLFILE: spell file in portions         
//                        case THREAD_SPELLSCRN: spell screen                   
//                        case THREAD_SPELLSEGMENT:  spell given segment        
//                        THREAD_TEMPADD: add word to temp addenda              
//                        THREAD_AUTOSAVE: - filewrite automatically            
//                        THREAD_AUTOSUBST: subst.segments by exactmemmatches   
//                      endswitch                                               
//                      reset threadtask                                        
//                      thread not active any more                              
//                     work                                                     
//------------------------------------------------------------------------------

VOID EQFBWorkThread
(
  PVOID pvDoc
)
{
  PTBDOCUMENT    pDoc = (PTBDOCUMENT) pvDoc;
  PSPELLDATA     pSpellData;
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  
  while ( pDoc->fThreadAct && !pDoc->fThreadKill )
  {
    while ( (!pDoc->fThreadNotIdle) && (!pDoc->fThreadKill) )
    {
      DosSleep( 100L );                // check every 1/10 second
      if ( !pDoc->fThreadKill)
      {
		  if (pDoc->pvFileWriteData )
		  {
			/**************************************************************/
			/* no other thread task should interrupt filesave in thread   */
			/**************************************************************/
			EQFBThreadAutoSave(pDoc);
		  }
		  else
		  {
			if (pDoc->fAutoSpellCheck && pDoc->pvSpellData )
			{
			   // check whether screen has changed
			   // if current screen is different to "spellchecked screen",
			   // invoke screen spellcheck

			   pSpellData = (PSPELLDATA) pDoc->pvSpellData;
			   if (pDoc->TBRowOffset[0].ulSegNum != pSpellData->TBFirstLine.ulSegNum )
			   {
				  pDoc->usThreadTask = THREAD_SPELLSCRN;
				  pDoc->fThreadNotIdle = TRUE;
			   }
			   else if (pDoc->TBRowOffset[0].usSegOffset != pSpellData->TBFirstLine.usSegOffset )
			   {
				  pDoc->usThreadTask = THREAD_SPELLSCRN;
				  pDoc->fThreadNotIdle = TRUE;
			   }
			   else
			   {
				   if (pDoc->usThreadTask != THREAD_SPELLFILE )
				   {
				     ULONG ulSeg = pDoc->ulWorkSeg;
				     PTBSEGMENT pTBSeg = EQFBGetVisSeg(pDoc, &ulSeg);
				     if ( pTBSeg && !pTBSeg->pusHLType)
				     {
					    pDoc->usThreadTask = THREAD_SPELLSEGMENT;
					    pDoc->fThreadNotIdle = TRUE;
				     }
				   }
				   else  // usThreadTask == THREAD_SPELLFILE
				   {
					   // set thread not idle..
					   pDoc->fThreadNotIdle = TRUE;
				   }
			   } /* endif */
			}

			/**************************************************************/
			/* if AutoSave is ON, check elapsed  time                     */
			/**************************************************************/
			if (pEQFBUserOpt->UserOptFlags.bBackSave )
			{
			  EQFBThreadAutoSave(pDoc);
			} /* endif */
		  } /* endif */
       } /* endif */
    } /* endwhile */

    if ( !pDoc->fThreadKill)
    {
		switch ( pDoc->usThreadTask )
		{
		  case THREAD_SPELLFILE:
			EQFBFileSpellCheck(pDoc);
			pDoc->fThreadNotIdle = FALSE;
			break;
		  case THREAD_SPELLSCRN:
			EQFBScrnSpellCheck(pDoc);
			pDoc->fThreadNotIdle = FALSE;
			pDoc->usThreadTask = 0;
			break;
		  case THREAD_SPELLSEGMENT:
			EQFBActSegSpellCheck(pDoc);
			pDoc->fThreadNotIdle = FALSE;
			pDoc->usThreadTask = 0;
			break;
		  case THREAD_TEMPADD:
			EQFBTempAdd(pDoc);
			pDoc->fThreadNotIdle = FALSE;
			pDoc->usThreadTask = 0;
			break;
		  case THREAD_AUTOSAVE:
			EQFBThreadAutoSave(pDoc);
			pDoc->fThreadNotIdle = FALSE;
			pDoc->usThreadTask = 0;
			break;
		  case THREAD_AUTOSUBST:
			EQFBThreadAutoSubst(pDoc);
			pDoc->fThreadNotIdle = FALSE;
			pDoc->usThreadTask = 0;
			break;
		  default:
			pDoc->usThreadTask = 0;
			break;
		} /* endswitch */
     } /* endif */
  } /* endwhile */

  // call 'thread ends method' for active language 
  {
   PDOCUMENT_IDA pIdaDoc = (PDOCUMENT_IDA)((PSTEQFGEN)pDoc->pstEQFGen)->pDoc;
   if ( pIdaDoc->sTgtLanguage != -1)
   {
     MorphThreadEnds( pIdaDoc->sTgtLanguage );
   } /* endif */
  }

  pDoc->fThreadAct = FALSE;

  _endthread();
  return;
}

//------------------------------------------------------------------------------
// Function name:     EQFBFuncSpellAuto - automatic spellcheck on/off           
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBFuncSpellAuto (PTBDOCUMENT);                          
//------------------------------------------------------------------------------
// Description  :     turn automatic spellcheck on/off                          
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if in target document                                     
//                    toggle fAutoSpellCheck ON/OFF                             
//                    if fAutoSpellCheck ON and thread not active               
//                      if nec, allocate pSpellData                             
//                      call thread with task "SPELLSCRN"                       
//                    else (fAUTOSpellCheck OFF)                                
//                      reset pusHLType = NULL for total document               
//                      reset SegFlags.SpellChecked = FALSE for total document  
//                    force redraw                                              
//------------------------------------------------------------------------------
VOID
EQFBFuncSpellAuto
(
   PTBDOCUMENT    pDoc
)
{
  PSPELLDATA      pSpellData;
  USHORT          ulSegNum;
  PTBSEGMENT      pSeg;
  USEROPT*        pEQFBUserOpt = get_EQFBUserOpt();
  /********************************************************************/
  /* call thread with task spellcheck file                            */
  /********************************************************************/
  if (pDoc->fSpellCheck && (pDoc->docType == STARGET_DOC))
  {
     pDoc->fAutoSpellCheck = !pDoc->fAutoSpellCheck;
     pEQFBUserOpt->UserOptFlags.bAutoSpellCheck = pDoc->fAutoSpellCheck;
     if ( pDoc->fAutoSpellCheck )
     {
	   if ( !pDoc->fThreadAct)
	   {
		   if (!pDoc->pvSpellData)
		   {
			  UtlAlloc( (PVOID *) &(pDoc->pvSpellData), 0L,
						(LONG) sizeof(SPELLDATA), ERROR_STORAGE );
			  if (pDoc->pvSpellData)                    //if 1st call to Spellcheck
			  {                                         //init ptr to ignorelist

				 pSpellData = (PSPELLDATA) pDoc->pvSpellData;
				 pSpellData->pDoc = pDoc;
				 UtlAlloc( (PVOID *)&(pSpellData->pIgnoreData) ,
						   0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W) , ERROR_STORAGE );
				 if ( pSpellData->pIgnoreData )
				 {
				   pSpellData->usIgnoreLen = MAX_SEGMENT_SIZE;
				   pSpellData->pIgnoreNextFree = pSpellData->pIgnoreData;
				 } /* endif */
				 UtlAlloc( (PVOID *) &(pSpellData->pSpellWorkSegW),
							0L, (LONG) (MAX_SEGMENT_SIZE + 1) * sizeof(CHAR_W), ERROR_STORAGE);
				 UtlAlloc( (PVOID *) &(pSpellData->pSpellSeg),
							0L, (LONG) sizeof(TBSEGMENT), ERROR_STORAGE);

				 pSpellData->FindData.fChange = FALSE;
				 pSpellData->FindData.ulFirstSegNum = 0;
				 pSpellData->FindData.ulStartSegNum = 0;
			  } /* endif */
		   } /* endif */
	   }
	   if ( pDoc->pvSpellData )
	   {
		  /**************************************************************/
		  /* call thread with task= Spellcheck current screen           */
		  /**************************************************************/
		 // force that thread recalcs pusHLType of screen
		   PSPELLDATA pSpellData = (PSPELLDATA) pDoc->pvSpellData;
		   pSpellData->TBFirstLine.ulSegNum = 0;
		   pSpellData->TBFirstLine.usSegOffset = (USHORT)-1; // cannot be segoffs

		  EQFBWorkThreadTask ( pDoc, THREAD_SPELLSCRN );
	   } /* endif */

    }
    else
    {
      /******************************************************************/
      /* auto spellcheck turn off                                       */
      /******************************************************************/
      ulSegNum = 1;
      pSeg = EQFBGetSegW(pDoc, ulSegNum);

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
    } /* endif */
    pDoc->Redraw |= REDRAW_ALL;
    WinPostMsg( pDoc->hwndClient, WM_PAINT, 0, NULL );
    if ( pDoc->hwndRichEdit )
    {
      WinPostMsg( pDoc->hwndRichEdit, WM_PAINT, 0, NULL );
    } /* endif */
  }
  else
  {
    //in otherdoc and ssource doc  spellcheck not allowed
    // beep also if spellcheck for target language not available
    WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
  } /* endif */

  return;
}

//------------------------------------------------------------------------------
// Function name:     EQFBFuncNextMisspelled - goto next misspelled word        
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBFuncNextMisspelled (PTBDOCUMENT)                      
//------------------------------------------------------------------------------
// Description  :     position cursor at next misspelled word                   
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     does not run in the thread!                               
//                    init SpellData if not done yet                            
//                    turn AutoSpellCheck on                                    
//                    call thread with task "SPELLFILE"                         
//                    sleep to give thread some time                            
//                    find next segment which is current or xlated              
//                    wait until this segment is spellchecked                   
//                    ( sleep at most one minute)                               
//                    do                                                        
//                       check whether segment has misspelled word              
//                       if not, goto next segment which is current or xlated   
//                          (wrap aroung at end of document)                    
//                       wait until this segment is spellchecked                
//                    while all words correctly spelled and not thru total file 
//                    if misspelled found                                       
//                       goto segment and to misspelled word                    
//                    else                                                      
//                       display msg "No misspelled found"                      
//------------------------------------------------------------------------------
VOID
EQFBFuncNextMisspelled
(
   PTBDOCUMENT    pDoc
)
{
  BOOL         fMisspelled = FALSE;
  USHORT       usMisOffset = 0;
  ULONG        ulSegNum;
  USHORT       ulSegOffset;
  PTBSEGMENT   pSeg;
  PSTARTSTOP   pHLCurrent;
  PSPELLDATA   pSpellData;
  USHORT       usI;
  ULONG        ulStartSegNum;
#ifdef ITMTEST
  USHORT       usJ = 0;
#endif

  if (pDoc->fSpellCheck && (pDoc->docType == STARGET_DOC))
  {
    /********************************************************************/
    /* update statusbar                                                 */
    /********************************************************************/
    STATUSBAR( pDoc );
    SETCURSOR(SPTR_WAIT);
    /********************************************************************/
    /* force that total file is spellchecked  without AUTOSPELLCHECK on */
    /********************************************************************/
    if (!pDoc->pvSpellData)
    {
       UtlAlloc( (PVOID *) &(pDoc->pvSpellData), 0L, (LONG) sizeof(SPELLDATA), ERROR_STORAGE );
       if (pDoc->pvSpellData)                    //if 1st call to Spellcheck
       {                                         //init ptr to ignorelist

          pSpellData = (PSPELLDATA) pDoc->pvSpellData;
          UtlAlloc( (PVOID *)&(pSpellData->pIgnoreData) , 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE );
          if ( pSpellData->pIgnoreData )
          {
            pSpellData->usIgnoreLen = MAX_SEGMENT_SIZE;
            pSpellData->pIgnoreNextFree = pSpellData->pIgnoreData;
          } /* endif */
          UtlAlloc( (PVOID *) &(pSpellData->pSpellWorkSegW), 0L, (LONG) (MAX_SEGMENT_SIZE + 1) * sizeof(CHAR_W), ERROR_STORAGE);
		      UtlAlloc( (PVOID *) &(pSpellData->pSpellSeg), 0L, (LONG) sizeof(TBSEGMENT), ERROR_STORAGE);
          pSpellData->ulProofSeg = 1;
          pSpellData->pDoc = pDoc;
          pSpellData->FindData.fChange = FALSE;
          pSpellData->FindData.ulFirstSegNum = pDoc->TBCursor.ulSegNum;
          pSpellData->FindData.ulStartSegNum = pDoc->TBCursor.ulSegNum;
       } /* endif */
    } /* endif */
    if ( pDoc->pvSpellData )
    {
       pDoc->fAutoSpellCheck = TRUE;
       EQFBWorkThreadTask ( pDoc, THREAD_SPELLFILE );
    } /* endif */

    pSpellData = (PSPELLDATA) pDoc->pvSpellData;
    DosSleep(100L);
    if (pSpellData )
    {
      ulSegNum = pDoc->TBCursor.ulSegNum;
      ulSegOffset = pDoc->TBCursor.usSegOffset;
      ulStartSegNum = ulSegNum;
      /******************************************************************/
      /* wait until Spellcheck is done with next segnum which is xlated */
      /* or current                                                     */
      /******************************************************************/
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
      if ((pSeg->qStatus != QF_XLATED) && (pSeg->qStatus != QF_CURRENT) )
      {
        do
        {
          ulSegNum ++;
          if (ulSegNum >= pDoc->ulMaxSeg )
          {
            ulSegNum = 1;
          } /* endif */
          pSeg = EQFBGetSegW(pDoc, ulSegNum);
        } while ((pSeg->qStatus != QF_XLATED)
               && (pSeg->qStatus != QF_CURRENT)
               && (ulSegNum != ulStartSegNum)) ; /* enddo */
      } /* endif */

      usI = 0;
      while (pSeg && !pSeg->SegFlags.Spellchecked
             && pDoc->fThreadAct && !pDoc->fThreadKill && (usI < 600))
      {
        DosSleep(100L);
        usI++;
        if ((pSpellData->FindData.ulStartSegNum == 0) &&
              (pSpellData->FindData.ulFirstSegNum == 0))
        {  // finish loop if FileSpellcheck once thru the file
			usI = 600;
	    }
      } /* endwhile */
      if ( usI < 600  )
      {
        usI = 0;
        do
        {
          if (pSeg->pusHLType )
          {
            pHLCurrent = (PSTARTSTOP) pSeg->pusHLType;
            while (pHLCurrent->usType && !fMisspelled )
            {
              if (pSeg->ulSegNum == pDoc->TBCursor.ulSegNum  )
              {
                if ((pHLCurrent->usType == MISSPELLED_HIGHLIGHT) &&
                    (pHLCurrent->usStart > pDoc->TBCursor.usSegOffset) )
                {
                  fMisspelled = TRUE;
                  usMisOffset = pHLCurrent->usStart;
                }
                else
                {
                  pHLCurrent ++;
                } /* endif */
              }
              else
              {
                if (pHLCurrent->usType == MISSPELLED_HIGHLIGHT )
                {
                  fMisspelled = TRUE;
                  usMisOffset = pHLCurrent->usStart;
                }
                else
                {
                  pHLCurrent ++;
                } /* endif */
              } /* endif */
            } /* endwhile */

          } /* endif */
          if (!fMisspelled )
          {
            usI = 0;
            ulSegNum++;
            if (ulSegNum >= pDoc->ulMaxSeg )
            {
              ulSegNum = 1;
            } /* endif */
            pSeg = EQFBGetSegW(pDoc, ulSegNum);
            while ((pSeg->qStatus != QF_XLATED)
                    && (pSeg->qStatus != QF_CURRENT)
                    && (ulSegNum != ulStartSegNum))
            {
              ulSegNum ++;
              if (ulSegNum >= pDoc->ulMaxSeg )
              {
                ulSegNum = 1;
              } /* endif */
              pSeg = EQFBGetSegW(pDoc, ulSegNum);
            } /* endwhile */
            while (pSeg && !pSeg->SegFlags.Spellchecked
                   && pDoc->fThreadAct && !pDoc->fThreadKill && (usI < 600))
            {
              usI ++;
              DosSleep(100L);
              if ((pSpellData->FindData.ulStartSegNum == 0) &&
			                (pSpellData->FindData.ulFirstSegNum == 0))
		      {  // finish loop if FileSpellcheck once thru the file
			  			usI = 600;
	          }
            } /* endwhile */
          } /* endif */

        } while (!fMisspelled && (ulSegNum != ulStartSegNum)
                  && pDoc->fThreadAct &&
                    !pDoc->fThreadKill && (usI < 600)); /* enddo */

        if (fMisspelled )
        {
          EQFBGotoSeg(pDoc,ulSegNum, usMisOffset);
        }
        else
        {
          UtlError( TB_ENDSPELLCHECK, MB_CANCEL, 0, NULL, EQF_WARNING );
        } /* endif */
      }
      else
      {
        /****************************************************************/
        /* no more misspelled found ??                                  */
        /****************************************************************/
        UtlError( TB_ENDSPELLCHECK, MB_CANCEL, 0, NULL, EQF_WARNING );
      } /* endif */
    } /* endif */

    /********************************************************************/
    /* update statusbar                                                 */
    /********************************************************************/
    SETCURSOR(SPTR_ARROW);
    STATUSBAR( pDoc );
  }
  else
  {
    //in otherdoc and ssource doc  spellcheck not allowed
    // beep also if spellcheck for target language not available
    WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
  } /* endif */

  return;
}

//------------------------------------------------------------------------------
// Function name:     EQFBScrnSpellCheck - spellcheck all segs on screen        
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBScrnSpellCheck     (PTBDOCUMENT)                      
//                                                                              
//------------------------------------------------------------------------------
// Description  :     spellcheck all translated segments and current segment    
//                    on screen                                                 
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     get segnum of ScrnRows structure                          
//                    while not thru all rows of the screen                     
//                      spellcheck segment                                      
//                    if misspelled found on screen                             
//                       force repaint                                          
//                    find next misspelled word outside of screen               
//------------------------------------------------------------------------------
static VOID
EQFBScrnSpellCheck
(
   PTBDOCUMENT    pDoc
)
{
  USHORT         usDataLen;
  ULONG          ulSegTemp = 0;
  PSPELLDATA     pSpellData;
  BOOL           fMisspelledFound = FALSE;
  BOOL           fNextMisFound = FALSE;
  ULONG          ulLastSeg;
  ULONG          ulStartSeg;
  USHORT         fUpdate = FALSE;              // redraw nec?

  pSpellData = (PSPELLDATA) pDoc->pvSpellData;
  if (pSpellData )
  {  // remember which screen is spellchecked
	pSpellData->TBFirstLine.ulSegNum = pDoc->TBRowOffset[0].ulSegNum;
	pSpellData->TBFirstLine.usSegOffset = pDoc->TBRowOffset[0].usSegOffset;

    usDataLen = sizeof(pSpellData->chProofData);
    ulStartSeg = pDoc->TBRowOffset[0].ulSegNum;
    if (ulStartSeg == 0 )
    {
      ulStartSeg = 1;
    } /* endif */

    ulSegTemp = ulStartSeg;
    ulLastSeg = pDoc->TBRowOffset[pDoc->lScrnRows + 1].ulSegNum;
    if ( ulLastSeg == 0 )
    {
      ulLastSeg = pDoc->ulMaxSeg;
    } /* endif */

    while ((ulSegTemp <= ulLastSeg)
            && pDoc->fThreadAct && !pDoc->fThreadKill)
    {
      fUpdate |=  EQFBSegSpellCheck( pDoc, ulSegTemp,
                                     pSpellData, &fMisspelledFound );
      ulSegTemp++;
    } /* endwhile */
    if (fUpdate )
    {
      pDoc->Redraw |= REDRAW_ALL;
      WinPostMsg( pDoc->hwndClient, WM_PAINT, 0, NULL );

      if ( pDoc->hwndRichEdit )
      {
        WinPostMsg( pDoc->hwndRichEdit, WM_PAINT, 0, NULL );
      } /* endif */
    } /* endif */

    while (!fNextMisFound  && (ulSegTemp <= pDoc->ulMaxSeg)
            && pDoc->fThreadAct && !pDoc->fThreadKill)
    {
      fMisspelledFound = FALSE;
      fUpdate |= EQFBSegSpellCheck( pDoc, ulSegTemp,
                                    pSpellData, &fMisspelledFound);
      fNextMisFound |= fMisspelledFound;
      ulSegTemp++;
    } /* endwhile */
    /****************************************************************/
    /* wrap around at end of document if no misspelled found yet    */
    /****************************************************************/
    if (ulSegTemp > pDoc->ulMaxSeg && !fNextMisFound )
    {
      ulSegTemp = 1;
      while (!fNextMisFound  && (ulSegTemp < ulStartSeg)
              && pDoc->fThreadAct && !pDoc->fThreadKill)
      {
        fMisspelledFound = FALSE;
        fUpdate |= EQFBSegSpellCheck( pDoc, ulSegTemp,
                                      pSpellData, &fMisspelledFound);
        fNextMisFound |= fMisspelledFound;
        ulSegTemp++;
      } /* endwhile */
    } /* endif */

  } /* endif */

  DosSleep(0);
  return;
}

//------------------------------------------------------------------------------
// Function name:     EQFBActSegSpellCheck - spellcheck active segment          
// Internal function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBActSegSpellCheck   (PTBDOCUMENT)                      
//------------------------------------------------------------------------------
// Description  :     spellcheck active segment as far as it is translated      
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     get active segment                                        
//                    if actseg not spellchecked                                
//                      if cursor in actseg                                     
//                        check actseg from begin until cursor position         
//                      else check total actseg                                 
//                      check misspelled termlist again tempaddenda             
//                      build pusHLType list from remaining misspelled termlist 
//------------------------------------------------------------------------------
static VOID
EQFBActSegSpellCheck
(
    PTBDOCUMENT   pDoc
)
{
  USHORT         usDataLen;
  PTBSEGMENT     pTBSeg;
  ULONG          ulSegNum;
  PSPELLDATA     pSpellData;
  USHORT         usRc = 0;
  USHORT         usLength;
  BOOL           fMisspelledFound = FALSE;
  //for debug only


  pSpellData = (PSPELLDATA) pDoc->pvSpellData;
  if (pSpellData )
  {
    usDataLen = sizeof(pSpellData->chProofData);
    ulSegNum = pDoc->tbActSeg.ulSegNum;
    if (ulSegNum == 0)
    {  // THREAD_SPELLSEGMENT is called if pDoc->ulWorkSeg should be checked!
		ulSegNum = pDoc->ulWorkSeg;
    }
    pTBSeg = EQFBGetVisSeg(pDoc, &ulSegNum);
    if (pTBSeg && (pTBSeg->qStatus != QF_NOP )
          && !pTBSeg->SegFlags.Spellchecked && !pDoc->fThreadKill)
    {
      memset(&pSpellData->chProofData[0],
               0, sizeof(pSpellData->chProofData));
      usLength = pDoc->TBCursor.usSegOffset;
      // tbActSeg.qStatus is status of segment prior to activation of segment
      // If segment has been translated already, spellcheck whole segment
      // If segment has not been translated, spellcheck only until cursor
      // Go in if-clause only if
      if ((!pDoc->EQFBFlags.PostEdit) && (pDoc->TBCursor.ulSegNum == ulSegNum)
             && (pDoc->tbActSeg.qStatus != QF_XLATED) )
      {
        UTF16strcpy (pSpellData->pSpellWorkSegW, pTBSeg->pDataW);
        pSpellData->pSpellWorkSegW[usLength] = EOS;
        usRc = EQFDoProofSeg(pDoc, pSpellData->pSpellWorkSegW,
                             pSpellData->chProofData, &usDataLen);
      }
      else
      {
		// work on copy of Data
		UTF16strcpy(pSpellData->pSpellWorkSegW,pTBSeg->pDataW);
        usRc = EQFDoProofSeg(pDoc, pSpellData->pSpellWorkSegW, /*pTBSeg->pDataW,*/
                             pSpellData->chProofData, &usDataLen);
      } /* endif */

      if (!usRc && !pDoc->fThreadKill)
      {
        fMisspelledFound = EQFBAllSpellIgnoreCheck(pSpellData);
        EQFBMisspelledHLType(pDoc, pTBSeg, pSpellData->chProofData);

        pTBSeg->SegFlags.Spellchecked = TRUE;
        if (fMisspelledFound )
        {
          pDoc->Redraw |= REDRAW_ALL;
          WinPostMsg( pDoc->hwndClient, WM_PAINT, 0, NULL );

          if ( pDoc->hwndRichEdit )
          {
            WinPostMsg( pDoc->hwndRichEdit, WM_PAINT, 0, NULL );
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  DosSleep(0);
  return;

}

//------------------------------------------------------------------------------
// Function name:     EQFBFileSpellCheck   - spellcheck file in portions        
// Internal function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBFileSpellCheck     (PTBDOCUMENT)                      
//------------------------------------------------------------------------------
// Description  :     spellcheck file in portions of NEXTNUMTOSPELL many segs   
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if first call: store starting segment in usFirstSegNum    
//                    get   start segnum of current portion of segments         
//                    set indicator if current portion is last of file          
//                    spell first segment of portion                            
//                    loop thru portion and spell all segments in it            
//                      (wrap around at end of document)                        
//                    if portion checked and not thru total file:               
//                       store startsegnum for next portion                     
//                    else set firstsegnum = startsegnum=0 to indicate that     
//                       filespellcheck is done!                                
//------------------------------------------------------------------------------

static VOID
EQFBFileSpellCheck
(
    PTBDOCUMENT   pDoc
)
{
  ULONG          ulCurSeg;
  PSPELLDATA     pSpellData;
  ULONG          ulStartSeg;
  BOOL           fMisspelledFound = FALSE;
  USHORT         usI = 1;
  PFINDDATA      pFindData;
 // BOOL           fLast = FALSE;
  BOOL           fUpdate = FALSE;
  BOOL           fWrap = FALSE;

  pSpellData = (PSPELLDATA) pDoc->pvSpellData;
  if (pSpellData )
  {
    pFindData = &(pSpellData->FindData);
    /******************************************************************/
    /* start at actseg if first call or reset due to ADDENDA change   */
    /******************************************************************/
    if ((pFindData->ulFirstSegNum == 0) || (pFindData->fChange)  )
    {
		if (pDoc->tbActSeg.ulSegNum)
		{
          ulStartSeg = pDoc->tbActSeg.ulSegNum;
	    }
	    else
	    {
			ulStartSeg = pDoc->ulWorkSeg;
	    }
      pFindData->ulFirstSegNum = ulStartSeg;
      pFindData->fChange = FALSE;
      pFindData->ulStartSegNum = 0;
    }
    else
    {
      /****************************************************************/
      /* check if wrap around                                         */
      /****************************************************************/
//      if ( abs(pFindData->ulStartSegNum - pFindData->ulFirstSegNum)
//               < NEXTNUMTOSPELL   )
//      {
//        fLast = TRUE;
//      } /* endif */
      ulStartSeg = pFindData->ulStartSegNum;
    } /* endif */

    fUpdate = EQFBSegSpellCheck(pDoc, ulStartSeg,
                                pSpellData, &fMisspelledFound);

    ulCurSeg = ulStartSeg + 1;
    if (ulCurSeg >= pDoc->ulMaxSeg )
    {
      ulCurSeg = 1;             // wrap around at end of document
      fWrap = TRUE;
    } /* endif */
    usI = 1;
    /*****************************************************************/
    /* loop once thru whole document                                 */
    /*****************************************************************/
    while ((ulCurSeg  != ulStartSeg) && (usI < NEXTNUMTOSPELL )
            && pDoc->fThreadAct && !pDoc->fThreadKill)
    {
      if (ulCurSeg  )
      {
        /*************************************************************/
        /* call EQFPROOF for ulCurSeg and fill result in pusHLType!! */
        /*************************************************************/
        fUpdate |= EQFBSegSpellCheck( pDoc, ulCurSeg,
                                      pSpellData, &fMisspelledFound );
      } /* endif */
      ulCurSeg ++;
      usI++;
      if (ulCurSeg >= pDoc->ulMaxSeg )
      {
        ulCurSeg = 1;             // wrap around at end of document
        fWrap = TRUE;
      } /* endif */
    } /* endwhile */
    if ((ulCurSeg != ulStartSeg) && (usI == NEXTNUMTOSPELL )
         && !pDoc->fThreadKill && !fWrap /*&& !fLast*/ )
    {
      pFindData->ulStartSegNum = ulCurSeg;
    }
    else
    {
      /***************************************************************/
      /* all of file checked or thread kill                          */
      /***************************************************************/
      pFindData->ulFirstSegNum = 0;
      pFindData->ulStartSegNum = 0;
      pDoc->fThreadNotIdle = FALSE;
	  pDoc->usThreadTask = 0;
    } /* endif */
    /******************************************************************/
    /* TODO: only if misspelled on current screen !!                  */
    /******************************************************************/
    if (fUpdate && EQFBSegIsVisible(pDoc, ulStartSeg, ulCurSeg ))
    {
      pDoc->Redraw |= REDRAW_ALL;
      WinPostMsg( pDoc->hwndClient, WM_PAINT, 0, NULL );

      if ( pDoc->hwndRichEdit )
      {
        WinPostMsg( pDoc->hwndRichEdit, WM_PAINT, 0, NULL );
      } /* endif */
    } /* endif */
  } /* endif */
  return;
}

static BOOL
EQFBSegIsVisible
(
  PTBDOCUMENT   pDoc,
  ULONG         ulStartSeg,
  ULONG         ulEndSeg
)
{
  ULONG     ulFirstRowSegNum;
  ULONG     ulLastRowSegNum;
  BOOL      fSegIsVisible = FALSE;

    ulFirstRowSegNum = pDoc->TBRowOffset[0].ulSegNum;
    ulLastRowSegNum = pDoc->TBRowOffset[pDoc->lScrnRows+2].ulSegNum;
    if (ulLastRowSegNum == 0 )
    {
      ulLastRowSegNum = pDoc->ulMaxSeg;
    } /* endif */
    if ((ulStartSeg >= ulFirstRowSegNum) && (ulStartSeg <= ulLastRowSegNum ) )
    {
      fSegIsVisible = TRUE;
    }
    else if ((ulEndSeg >= ulFirstRowSegNum) && (ulEndSeg <= ulLastRowSegNum) )
    {
      fSegIsVisible = TRUE;
    } /* endif */


  return(fSegIsVisible);
}

//------------------------------------------------------------------------------
// Function name:     EQFBSegSpellCheck    - spellcheck given segment           
// Internal function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBSegSpellCheck(pDoc, ulCurSeg, pSpellData)             
//------------------------------------------------------------------------------
// Description  :     spellcheck given segment                                  
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//                    USHORT        - number of current segment                 
//                    PSPELLDATA    - ptr to spelldata structure                
//                    PBOOL         - fMisspelledFound                          
//------------------------------------------------------------------------------
// Returncode type:   BOOL  TRUE: misspelled found in segment                   
//                          FALSE: segment spelled correctly                    
//------------------------------------------------------------------------------
// Function flow:     get segment                                               
//                    if segment is XLATED or CURRENT and not yet spellchecked  
//                       spellcheck segment                                     
//                       check misspelled words against tempaddenda             
//                       fill  pusHLType buffer with positions of misspelled    
//------------------------------------------------------------------------------
static BOOL
EQFBSegSpellCheck
(
    PTBDOCUMENT   pDoc,
    ULONG         ulCurSeg,
    PSPELLDATA    pSpellData,
    PBOOL         pfMisspelledFound
)
{
  PTBSEGMENT    pTBSeg;
  USHORT        usRc = 0;
  USHORT        usDataLen;
  BOOL          fMisspelledFound = FALSE;
  PSTARTSTOP    pHLCurrent;
  BOOL          fUpdate = FALSE;

  if (pSpellData && pSpellData->pSpellWorkSegW && pSpellData->pSpellSeg)
  {
    fMisspelledFound = FALSE;
    pTBSeg = EQFBGetVisSeg(pDoc, &ulCurSeg);
    if (pTBSeg && ((pTBSeg->qStatus == QF_XLATED) ||
         (pTBSeg->qStatus ==  QF_CURRENT)) )
    {
      if (!pTBSeg->SegFlags.Spellchecked )
      {
		// work on copy of Data
		UTF16strcpy(pSpellData->pSpellWorkSegW,pTBSeg->pDataW);

        usDataLen = sizeof(pSpellData->chProofData);

        memset(&pSpellData->chProofData[0],
                 0, sizeof(pSpellData->chProofData));
        pSpellData->ulProofSeg = ulCurSeg;
        usRc = EQFDoProofSeg(pDoc, pSpellData->pSpellWorkSegW,
                             pSpellData->chProofData, &usDataLen);

        if (!usRc )
        {
          /**************************************************************/
          /* check whether all words in chProofData whether they are in */
          /* ignore list; if so delete them in chProofData              */
          /**************************************************************/
          fMisspelledFound = EQFBAllSpellIgnoreCheck(pSpellData);
          EQFBMisspelledHLType(pDoc, pTBSeg, pSpellData->chProofData);
          pTBSeg->SegFlags.Spellchecked = TRUE;
        } /* endif */
        pSpellData->ulProofSeg = 1;
        fUpdate = TRUE;
        DosSleep(0);
      }
      else
      {
        if (pTBSeg->pusHLType  )
        {
          if (pTBSeg->ulSegNum != pDoc->tbActSeg.ulSegNum )
          {
            /**********************************************************/
            /* TAG_HIGHLIGHT only in ACTSEG possible                  */
            /**********************************************************/
            fMisspelledFound = TRUE;
          }
          else
          {
            pHLCurrent = (PSTARTSTOP) pTBSeg->pusHLType;
            while (pHLCurrent->usType && !fMisspelledFound )
            {
              if (pHLCurrent->usType == MISSPELLED_HIGHLIGHT)
              {
                fMisspelledFound = TRUE;
              }
              else
              {
                pHLCurrent ++;
              } /* endif */
            } /* endwhile */
          } /* endif */

        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  *pfMisspelledFound = fMisspelledFound;
  return(fUpdate);
}

//------------------------------------------------------------------------------
// Function name:     EQFBTempAdd          - add given word to tempaddenda      
// Internal function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBTempAdd(pDoc)    - called from thread only !          
//------------------------------------------------------------------------------
// Description  :     add given word to temp addenda                            
//                    ( word is in pSpellData->FindData.chFind , DO NOT USE     
//                     this field in thread for any other purpose !)            
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if not enough buffer for word to be ignored:              
//                      allocate larger buffer                                  
//                    if buffer large enough                                    
//                      copy word in it                                         
//------------------------------------------------------------------------------

VOID
EQFBTempAdd
(
    PTBDOCUMENT   pDoc
)
{
  USHORT      usNextFreePos = 0;
  PSPELLDATA  pSpellData;
  PSZ_W       pWord;

  pSpellData = (PSPELLDATA) pDoc->pvSpellData;
  pWord = &(pSpellData->FindData.chFind[0]);

  if ( pSpellData &&
       (pSpellData->pIgnoreNextFree + UTF16strlenCHAR(pWord)
         >= pSpellData->pIgnoreData+pSpellData->usIgnoreLen-2 ))
  {
    usNextFreePos = (USHORT)(pSpellData->pIgnoreNextFree - pSpellData->pIgnoreData);
    UtlAlloc( (PVOID *)&(pSpellData->pIgnoreData) ,
              (LONG) pSpellData->usIgnoreLen,
              (LONG) (pSpellData->usIgnoreLen+MAX_SEGMENT_SIZE)* sizeof(CHAR_W), ERROR_STORAGE );
    if ( pSpellData->pIgnoreData )
    {
      pSpellData->pIgnoreNextFree = usNextFreePos + pSpellData->pIgnoreData;
      pSpellData->usIgnoreLen += MAX_SEGMENT_SIZE;
    } /* endif */
  } /* endif */
  // get word and pass it in ignorelist, reset pointer to next free
  // position in ignorelist
  /********************************************************************/
  /* if alloc not possible, no error msg since in thread              */
  /********************************************************************/
  if (pSpellData->pIgnoreNextFree + UTF16strlenCHAR(pWord)
      < pSpellData->pIgnoreData + pSpellData->usIgnoreLen - 2 )
  {
     UTF16strcpy(pSpellData->pIgnoreNextFree,pWord);
     pSpellData->pIgnoreNextFree += UTF16strlenCHAR(pWord);
     *(pSpellData->pIgnoreNextFree) = EOS;
     pSpellData->pIgnoreNextFree ++;
  } /* endif */

  // reset area where misspelled word was stored for input purposes
  pSpellData->FindData.chFind[0] = EOS;
  return;
}

//------------------------------------------------------------------------------
// Function name:     EQFBAllSpellIgnoreCheck - check words agains tempaddenda  
// Internal function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBAllSpellIgnoreCheck -                                 
//------------------------------------------------------------------------------
// Description  :     check words in chProofData against tempaddenda            
//------------------------------------------------------------------------------
// Parameter:         PSPELLDATA   - ptr to spelldata                           
//------------------------------------------------------------------------------
// Returncode type:   BOOL  TRUE if misspelled words left in chProofData        
//                          FALSE if no misspelled word left in chProofData     
//------------------------------------------------------------------------------
// Function flow:     get ptr to buffer with misspelled words detected by POE   
//                    get target ptr ( where misspelled words are copied to)    
//                    while not at end of buffer                                
//                      check current word agains tempaddenda                   
//                      if it is not in tempaddenda                             
//                        copy it at target position                            
//                        set indicator that misspelled word is in  output      
//                      goto next misspelled word                               
//                    endwhile                                                  
//                    set EOS at end of output buffer                           
//------------------------------------------------------------------------------
BOOL
EQFBAllSpellIgnoreCheck
(
   PSPELLDATA  pSpellData
)
{
  PSZ_W       pCurWord;
  BOOL        fIgnore;
  USHORT      usCurLen;
  PSZ_W       pTempTgt;
  BOOL        fMisspelledFound = FALSE;

  /********************************************************************/
  /* get ptr to list of misspelled words                              */
  /********************************************************************/
  pSpellData->pProofCurrent = &(pSpellData->chProofData[0]);
  pCurWord = pSpellData->pProofCurrent;
  pTempTgt = pCurWord;
  while (*pCurWord )
  {
    fIgnore = EQFBSpellIgnoreCheck(pSpellData);
    usCurLen = (USHORT)(UTF16strlenCHAR(pCurWord) + 1);
    if (!fIgnore )
    {
      memmove(pTempTgt, pCurWord, usCurLen*sizeof(CHAR_W));
      pTempTgt  += usCurLen;
      fMisspelledFound = TRUE;
    } /* endif */
    pCurWord += usCurLen;
    pSpellData->pProofCurrent = pCurWord;
  } /* endwhile */
  *pTempTgt = EOS;

  return(fMisspelledFound);
}

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFDoProofSeg                                             
//------------------------------------------------------------------------------
// Function call:     _                                                         
//------------------------------------------------------------------------------
// Description:       performs proofreading against the passed string.          
//------------------------------------------------------------------------------
// Parameters:         PTBDOCUMENT pTBDoc,        // pointer to tbdocument      
//                     PSZ         pInData,       // data to be checked         
//                     PSZ         pProofData,    // buffer for termlistreturn  
//                     PUSHORT     pusDataLen     // length of returnbuffer     
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Function flow:     check that the morphological dictionary is activated      
//                    if not try to activate it                                 
//                                                                              
//                    if okay remove any tags and attributes from the segment   
//                    put the resulting segment into the same buffer            
//                    and call the MorphVerify function for verifying the seg   
//                                                                              
//------------------------------------------------------------------------------

static
USHORT EQFDoProofSeg
(
   PTBDOCUMENT pTBDoc,                           // pointer to tbdocument
   PSZ_W       pInData,                          // data to be checked
   PSZ_W       pProofData,                       // buffer for termlistreturn
   PUSHORT     pusDataLen                        // length of returnbuffer
)
{
   PTERMLENOFFS  pTermList = NULL;              // ptr to created term list
   USHORT        usLen = 0;                     // length of buffer
   USHORT        usRC = MORPH_OK;               // return code from NLP services
   PSZ_W         pData;                          // pointer to data
   PDOCUMENT_IDA pIdaDoc;

   /*******************************************************************/
   /*  check that the morphological dictionary is activated           */
   /*  if not try to activate it                                      */
   /*******************************************************************/
   pIdaDoc = (PDOCUMENT_IDA) ((PSTEQFGEN)pTBDoc->pstEQFGen)->pDoc;
   if ( pIdaDoc->sTgtLanguage == -1)
   {
     usRC = MorphGetLanguageID( pIdaDoc->szDocTargetLang,
                                &pIdaDoc->sTgtLanguage );
     pIdaDoc->ulTgtOemCP = GetLangOEMCP( pIdaDoc->szDocTargetLang);
   } /* endif */

   /*******************************************************************/
   /*  if okay remove any tags and attributes from the segment        */
   /*  put the resulting segment into the same buffer                 */
   /*  and call the MorphVerify function for verifying the segment    */
   /*******************************************************************/
   if ( usRC == MORPH_OK )
   {
      EQFBNormSeg( pTBDoc, pInData, pProofData );
      /****************************************************************/
      /* get rid of newline characters because they make trouble      */
      /* in the morphological analysis.                               */
      /****************************************************************/
      pData = pProofData;
      while ( *pData )
      {
        if ( *pData == LF )
        {
          *pData = BLANK;
        } /* endif */
        pData++;
      } /* endwhile */

      usRC = MorphVerify( pIdaDoc->sTgtLanguage,
                          pProofData,
                          &usLen,
                          (PVOID *)&pTermList,
                          MORPH_ZTERMLIST, pIdaDoc->ulTgtOemCP);
      if (  usRC == MORPH_OK )
      {
        usLen = min( usLen, *pusDataLen - 1 );
        memcpy( pProofData, pTermList, usLen * sizeof(CHAR_W) );
        *pusDataLen = usLen;
        UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
      } /* endif */
   }
   else
   {
     usRC = ERROR_STORAGE;
   } /* endif */

  return usRC;
} /* end of function EQFDoProofSeg */

//------------------------------------------------------------------------------
// Function name:     EQFBFuncGoUpdSegment - got next updated segment           
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBFuncGoUpdSegment   (PTBDOCUMENT)                      
//------------------------------------------------------------------------------
// Description  :     position cursor at next updated segment                   
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     goto next function which has not been                     
//                    substistuted during analysis                              
//------------------------------------------------------------------------------
VOID
EQFBFuncGoUpdSegment
(
   PTBDOCUMENT    pDoc
)
{
  ULONG        ulSegNum;
  USHORT       usSegOffset;
  PTBSEGMENT   pSeg;
  ULONG        ulStartSegNum;
  BOOL         fFound = FALSE;                   // no updated segment found
  USHORT       usStatus = QF_XLATED;

  if (pDoc->docType == STARGET_DOC)
  {
    /********************************************************************/
    /* update statusbar                                                 */
    /********************************************************************/
    STATUSBAR( pDoc );

    ulSegNum = pDoc->TBCursor.ulSegNum;
    usSegOffset = pDoc->TBCursor.usSegOffset;
    ulStartSegNum = ulSegNum;
    do
    {
      ulSegNum ++;
      if (ulSegNum >= pDoc->ulMaxSeg )
      {
        ulSegNum = 1;
      } /* endif */
      pSeg = EQFBGetSegW(pDoc, ulSegNum);
      if (pSeg )
      {
        usStatus = pSeg->qStatus;
        /**************************************************************/
        /* force stop at current seg if it has been translated        */
        /* previously                                                 */
        /**************************************************************/
        if (ulSegNum == pDoc->tbActSeg.ulSegNum )
        {
          usStatus = pDoc->tbActSeg.qStatus;
        } /* endif */
      } /* endif */

      if (pSeg && (usStatus == QF_XLATED)  )
      {
        if (!pSeg->CountFlag.AnalAutoSubst )
        {
          fFound = TRUE;              // seg translated without anal-subst
        }
        else
        {
          /************************************************************/
          /* pick up analysis-substituted segments which have been    */
          /* overtyped by translator                                  */
          /************************************************************/
          if (pSeg->SegFlags.Typed )
          {
            fFound = TRUE;
          } /* endif */
        } /* endif */
      } /* endif */
    } while ((ulSegNum != ulStartSegNum) && !fFound ); /* enddo */

    if ( fFound && (ulSegNum != ulStartSegNum))
    {
      /****************************************************************/
      /* translated segment found which has not been auto-substituted */
      /* during analysis                                              */
      /****************************************************************/
      EQFBGotoSeg(pDoc, ulSegNum, 0);
    }
    else
    {
      /****************************************************************/
      /* all translated segments are anal-autosubst translated!       */
      /* no more updated segments found!                              */
      /****************************************************************/
       UtlError( TB_NOUPDATEDINFILE, MB_OK, 0, NULL, EQF_WARNING );

    } /* endif */

    STATUSBAR( pDoc );
  }
  else
  {
    // beep if not in starget doc
    WinAlarm( HWND_DESKTOP, WA_WARNING );    // issue a beep
  } /* endif */

  return;
}

//------------------------------------------------------------------------------
// Function name:     EQFBThreadAutoSave   - filewrite in background            
// Internal function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBThreadAutoSave     (PTBDOCUMENT)                      
//------------------------------------------------------------------------------
// Description  :     filewrite of segmented target file                        
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:                                                               
//------------------------------------------------------------------------------

static VOID
EQFBThreadAutoSave
(
    PTBDOCUMENT   pDoc
)
{
  PVOID            pFWData = pDoc->pvFileWriteData;
  USHORT           usCPConversion = 0;
  USHORT           usRC = 0;
  ULONG            ulCP = 0L;
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();

  // GQ: 2015/10/28 Temporarely disable auto-save function as it seems to cause incorrect STARGET documents

  //if (pEQFBUserOpt->UserOptFlags.bBackSave )
  //{
  //  if ( (pFWData == NULL) &&  !pDoc->fSaveRunning )
  //  {
  //    LONG   lCurrentTime;                      // date analyzed
  //    LONG   lTimeElapsed= 0L;
  //    LONG   lTimeForSaveAgain = 0L;

  //    UtlTime( (PLONG)&lCurrentTime );
  //    lTimeForSaveAgain = ((LONG)pEQFBUserOpt->sMinuteTilNextSave) * 60L;
  //    if (lCurrentTime > pDoc->lTimeLastFileSave )
  //    {
  //      lTimeElapsed = lCurrentTime - (pDoc->lTimeLastFileSave);
  //    } /* endif */
  //    if (lTimeElapsed > lTimeForSaveAgain )
  //    {
  //      /******************************************************************/
  //      /* do not write histlog record! , init FileWRiteData, with indic. */
  //      /* fAutoSave = TRUE;                                              */
  //      /******************************************************************/
  //      usRC = EQFBPrepareFileWrite(pDoc, &pFWData, pDoc->szDocName, 0, usCPConversion, TRUE);
  //      pDoc->pvFileWriteData = pFWData;
  //    } /* endif */
  //  } /* endif */

  //  if (!usRC && (pFWData != NULL) )
  //  {
  //    // cancel the task when a foreground save is running
  //    if ( pDoc->fSaveRunning )
  //    {
  //      UtlTime(&pDoc->lTimeLastFileSave);
  //      EQFBTerminateFileWrite( pDoc, pFWData, 1 );
  //      pDoc->pvFileWriteData = pFWData = NULL;
  //    }
  //    else
  //    {
  //      BOOL fDone = FALSE;

  //      if ( usCPConversion == SGMLFORMAT_ASCII)
  //      {
  //          ulCP = pDoc->ulOemCodePage;
  //      }
  //      else if ( usCPConversion == SGMLFORMAT_ANSI)
  //      {
  //          ulCP = pDoc->ulAnsiCodePage;
  //      }
  //      usRC = EQFBWriteNextSegment(pDoc, pFWData, usCPConversion, ulCP, &fDone );
  //      
  //      /******************************************************************/
  //      /* if all segments saved, close up autosave                       */
  //      /******************************************************************/
  //      if ( fDone )
  //      {
  //        usRC = EQFBTerminateFileWrite( pDoc, pFWData, usRC );
  //        pDoc->pvFileWriteData = pFWData = NULL;

  //        if (!usRC )
  //        {
  //          UtlTime(&pDoc->lTimeLastFileSave);
  //        }
  //        else
  //        {
  //          usRC = (USHORT)ERR_WRITEFILE;
  //        } /* endif */
  //      } /* endif */
  //    } /* endif */
  //  } /* endif */
  //}
  //else            // fBackSave = FALSE, clear up if nec
  //{
  //  if ( pDoc->pvFileWriteData != NULL )
  //  {
  //    EQFBTerminateFileWrite( pDoc, pFWData, 1 );
  //    pDoc->pvFileWriteData = pFWData = NULL;
  //  }
  //} /* endif */
  return;
}

//------------------------------------------------------------------------------
// Function name:     EQFBThreadAutoSubst   - filewrite in background            
// Internal function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBThreadAutoSubst    (PTBDOCUMENT)                      
//------------------------------------------------------------------------------
// Description  :     automatically substitute exact matches                    
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:                                                               
//------------------------------------------------------------------------------

static VOID
EQFBThreadAutoSubst
(
    PTBDOCUMENT   pDoc
)
{
  /********************************************************************/
  /* do not run this while a Ctrl-Enter is running....!!              */
  /********************************************************************/
  pDoc;
  return;
}

//------------------------------------------------------------------------------
// Function name:     EQFBFuncVisibleSpace - make white spaces visible          
// External function                                                            
//------------------------------------------------------------------------------
// Function call:     EQFBFuncVisibleSpace   (PTBDOCUMENT)                      
//------------------------------------------------------------------------------
// Description  :     make white spaces visible                                 
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida                   
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     make white spaces visible                                 
//------------------------------------------------------------------------------
VOID
EQFBFuncVisibleSpace
(
   PTBDOCUMENT    pDoc
)
{
      PTBDOCUMENT  pDocStart;
	  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
      pDocStart = pDoc;
      pEQFBUserOpt->UserOptFlags.bVisibleSpace = !pEQFBUserOpt->UserOptFlags.bVisibleSpace;
      do
      {
        pDoc->Redraw |= REDRAW_ALL;
        EQFBRefreshScreen(pDoc);
        pDoc = pDoc->next;
      } while (pDoc != pDocStart ); /* enddo */
   return;
}
