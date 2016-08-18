/*! \file
	Description: Dictionary thread for MAT Tools
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_ANALYSIS         // analysis defines (required for EQFTAI.H)
#include <eqf.h>                  // General Translation Manager include file

#include "OtmDictionaryIF.H"
#include <eqftai.h>               // private analysis include file for EQFDOC00.H
#include <eqftpi.h>               // private Translation Processor include file
#include <eqfdoc00.h>             // for document handler defines
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_LDB              // dictionary data encoding functions
#include "EQFLDB.H"               // internal ASD services header file
#include "EQFDASDI.H"             // internal ASD services header file
#include "EQFDTAG.H"              // tag definition
#include <process.h>              // C process functions
#include "eqfevent.h"             // event logging facility

VOID PASCAL FAR DictCleanUp( USHORT );
static  PDOCUMENT_IDA pStaticDoc = NULL;

/* $PAGEIF20 */

/*******************************************************************************
**                                                                            **
**  Server thread: EQFDA()                                                    **
**                                                                            **
*******************************************************************************/
//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name: EQFDA                                                         
//------------------------------------------------------------------------------
// Function call:     EQFDA                                                     
//                                                                              
//------------------------------------------------------------------------------
// Description:       Dictionary access thread                                  
//                    Will be invoked from main program, opens the requested    
//                    dictionaries, retrieves the dictionary hits;              
//                    if main program issues a close the accessed resources     
//                    will be freed.                                            
//                                                                              
//------------------------------------------------------------------------------
// Parameter:         PVOID     -- pointer to PDOC structure                    
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//                                                                              
//------------------------------------------------------------------------------
// Side effects:      fSemDAProc will be used for controling access             
//                     fSemDAProc = TRUE: thread is working                     
//                                = FALSE: thread waits for work                
//                                                                              
//------------------------------------------------------------------------------
// Function flow:     activate dictionaries                                     
//                    set fSemDAProc to FALSE                                   
//                    while pDoc->fRunDA                                        
//                      while !fSemDAProc                                       
//                         sleep a short amount                                 
//                      endwhile                                                
//                      call EQFDAEntries to retrieve entries for sentence...   
//                      indicate waiting for further work (fSemDAProc = FALSE)  
//                    endwhile                                                  
//                    close dictionary if prev. openend                         
//                    end ASD services if allocated                             
//                    indicate finished                                         
//                    return                                                    
//------------------------------------------------------------------------------
VOID  EQFDA
(
   PVOID pInstance                     // pointer to request structure
)
{  PSTEQFGEN     pstEQFGen;                     // pointer to generic structure
   PDOCUMENT_IDA pDoc;

   DEBUGEVENT( EQFDA_LOC, FUNCENTRY_EVENT, 0 );

   pDoc     = (PDOCUMENT_IDA) pInstance;
   pStaticDoc = pDoc;
   pstEQFGen = pDoc->pstEQFGen;

   DosExitList(EXLST_ADD, (PFNEXITLIST)DictCleanUp);          // adds address to the exit list

   ActivateASD( pDoc );                     // activate the user dicts


   while (pDoc->fRunDA)
   {
      while ( !pDoc->fSemDAProc )
      {
        DosSleep( 50L );
      } /* endwhile */

      /**************************************************************/
      /* someone wants to use new dictionaries ....                 */
      /**************************************************************/
      if ( pDoc->fNewDictAccess )
      {
        if ( pDoc->hDCB )
        {
          AsdClose( pDoc->hUCB, pDoc->hDCB);  // close dictionaries
          pDoc->hDCB = NULL;
        } /* endif */
        ActivateASD( pDoc );
        pDoc->fNewDictAccess = FALSE;
      }
      else
      if ( pDoc->fRunDA )
      {
        /**************************************************************/
        /* retrieve entries for sentence  ...                         */
        /**************************************************************/
        EQFDAEntries( pDoc );
        DosSleep( 0L );                // give up rest of this time slice
      }

   } // endwhile (running)

   /*******************************************************************/
   /* close any open dictionaries                                     */
   /*******************************************************************/
   DEBUGEVENT( EQFDA_LOC, STATE_EVENT, 0 );

   EQFDAClose( pDoc );

   DEBUGEVENT( EQFDA_LOC, FUNCEXIT_EVENT, 0 );

   DosEnterCritSec();
   _endthread();
} // end thread 'EQFDA'

/* $PAGEIF20 */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name: ActivateASD                                                   
//------------------------------------------------------------------------------
// Function call:     ActivateASD( PTBDOCUMENT )                                
//------------------------------------------------------------------------------
// Description:       open the requested dictionaries                           
//                                                                              
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pointer to document ida                      
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       LX_MAX_OPEN_ASD      too many dictionaries specified      
//                    LX_RC_OK_ASD         every thing okay                     
//                    ...                  errors returned by AsdBegin          
//                                                                              
//------------------------------------------------------------------------------
// Function flow:     check that number of specified dictionaries is valid      
//                    if okay so far then                                       
//                      call AsdBegin and set return code                       
//                    endif                                                     
//                    if okay so far then                                       
//                      prepare pointer array with dictionary names             
//                      call AsdOpen and set return code                        
//                      if return code then                                     
//                        call error handling module and provide dictionary nam 
//                      endif                                                   
//                    endif                                                     
//                    set pDoc->fRunDA depending on results so far              
//                    return returncode                                         
//------------------------------------------------------------------------------
USHORT  ActivateASD
(
    PDOCUMENT_IDA pDoc                          // pointer to document
)
{
  USHORT        usRc = NO_ERROR;
  BOOL          fOK = TRUE;                     // success indicator
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSZ           apszDicts[NUM_OF_DICTS];        // pointer array for dicts
  USHORT        usNumDict;                      // number of failing dictionary
  USHORT        usNumLDicts = 0;                // number of dictionaries

  pstEQFGen = pDoc->pstEQFGen;                  // get pointer to generic struct

  while ( *(pDoc->szDicts[usNumLDicts++]) );  // determine num of passed dicts
  usNumLDicts--;                                // we've gone too far

  if (!usNumLDicts)                             // no dict specified
  {
     pstEQFGen->fsConfiguration &= ~EQFF_DA_CONF; // no dict configured
     usRc = LX_MAX_OPEN_ASD;
     fOK = FALSE;
  }
  else if ( usNumLDicts > NUM_OF_DICTS )  // more dicts than allowed
  {
     usRc = LX_MAX_OPEN_ASD;
     pDoc->usDAError = pstEQFGen->usRC = UtlQdamMsgTxt ( usRc );
     fOK = FALSE;
  } /* endif */

  /********************************************************************/
  /* start dictionary services if not done yet...                     */
  /********************************************************************/
  if ( fOK && !pDoc->hUCB )
  {
     usRc = AsdBegin( NUM_OF_DICTS, &pDoc->hUCB );
  } /* endif */

  if (usRc == LX_RC_OK_ASD && fOK)
  {
     for ( usNumDict = 0; usNumDict < usNumLDicts ; usNumDict++ )
     {
        apszDicts[ usNumDict ] = pDoc->szDicts[ usNumDict ];
     } /* endfor */

     usRc = AsdOpen( pDoc->hUCB,                 // user control block handle
                     ASD_GUARDED,                // guarded mode
                     usNumLDicts,                // number of dictionaries
                     apszDicts,                  // address of first dict.
                     &pDoc->hDCB,                // dict control block handle
                     &usNumDict );               // number of dict. failed

    if ( usRc != LX_RC_OK_ASD )
    {
       // get name of failing dictionary
      if ( usNumDict && usNumDict <= usNumLDicts)
      {
        /**************************************************************/
        /* display name of dictionary only in case of NONE LAN Errors */
        /**************************************************************/
        switch ( usRc )
        {
          case TMERR_TOO_MANY_QUERIES:
          case TMERR_PROP_WRITE_ERROR:
          case TMERR_PROP_NOT_FOUND:
          case TMERR_PROP_READ_ERROR:
          case TMERR_PROP_EXIST:
          case TMERR_SERVER_NOT_STARTED:
          case TMERR_SERVERCODE_NOT_STARTED:
          case TMERR_COMMUNICATION_FAILURE:
          case TMERR_SERVER_ABOUT_TO_EXIT:
          case LANUID_NO_LAN:
          case LANUID_REQ_NOT_STARTED:
          case LANUID_USER_NOT_LOG_ON:
          case TMERR_THREAD_NOT_SPAWNED:
          case TMERR_THREAD_INIT_FAILED:
            pstEQFGen->szMsgBuffer[0] = EOS;
            break;
          case LX_BAD_LANG_CODE:
            /**********************************************************/
            /* Activation of dictionary source language failed ->     */
            /* use dict. source language as message parameter         */
            /**********************************************************/
            {
              HPROP           hProp;                       // properties handle
              PPROPDICTIONARY pDictProp;                   // ptr to dict props
              CHAR            szPropName[MAX_EQF_PATH];    // buffer for propname
              EQFINFO         ErrorInfo;                   // buffer for error info

              pstEQFGen->szMsgBuffer[0] = EOS;
              UtlMakeEQFPath( szPropName, NULC, SYSTEM_PATH, NULL );
              strcat( szPropName, BACKSLASH_STR );
              strcat( szPropName, UtlGetFnameFromPath(pDoc->szDicts[usNumDict-1]) );
              hProp = OpenProperties( szPropName, NULL,
                                      PROP_ACCESS_READ, &ErrorInfo );
              if ( hProp )
              {
                 pDictProp = (PPROPDICTIONARY)MakePropPtrFromHnd( hProp );
                 ASCII2Unicode( pDictProp->szSourceLang, pstEQFGen->szMsgBuffer, 0L );
                 CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
              } /* endif */
            }
            break;
          default :
            {
              HPROP           hProp;                       // properties handle
              PPROPDICTIONARY pDictProp;                   // ptr to dict props
              CHAR            szPropName[MAX_EQF_PATH];    // buffer for propname
              EQFINFO         ErrorInfo;                   // buffer for error info

              pstEQFGen->szMsgBuffer[0] = EOS;
              UtlMakeEQFPath( szPropName, NULC, SYSTEM_PATH, NULL );
              strcat( szPropName, BACKSLASH_STR );
              strcat( szPropName, UtlGetFnameFromPath(pDoc->szDicts[usNumDict-1]) );
              hProp = OpenProperties( szPropName, NULL,
                                      PROP_ACCESS_READ, &ErrorInfo );
              if ( hProp )
              {
                 pDictProp = (PPROPDICTIONARY)MakePropPtrFromHnd( hProp );
                 ASCII2Unicode( pDictProp->szLongName, pstEQFGen->szMsgBuffer, 0L );
                 CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
              }
              else
              {
                CHAR chDict[MAX_EQF_PATH];

                Utlstrccpy (chDict,
                            UtlGetFnameFromPath(pDoc->szDicts[ usNumDict - 1 ]),
                            DOT );
                ASCII2Unicode( chDict, pstEQFGen->szMsgBuffer, 0L );
              } /* endif */
            }

            break;
        } /* endswitch */
      }
      else
      {
         pstEQFGen->szMsgBuffer[0] = EOS;
      } /* endif */
      pstEQFGen->usRC = UtlQdamMsgTxt ( usRc );
      UTF16strcpy (pDoc->szMsgBuffer, pstEQFGen->szMsgBuffer);
      pDoc->usDAError = pstEQFGen->usRC;
      pstEQFGen->fsConfiguration &= ~EQFF_DA_CONF; // no dict configured
      fOK =  FALSE;                              // do not go ahead
    } /* endif */
  } /* endif */

  pDoc->fRunDA = (USHORT)fOK ;                           // set global run flag
  pDoc->fSemDAProc = FALSE;                      //we could proceed...

  return usRc;
} // end 'ActivateASD'
/* $PAGEIF20 */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFDAInit                                                 
//------------------------------------------------------------------------------
// Function call:     EQFDAInit( PDOCUMENT_IDA );                               
//------------------------------------------------------------------------------
// Description:       start dictionary services and the dictionary              
//                    thread                                                    
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA       pointer to document ida               
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     check if we have to configure for dictionary thread       
//                    start the dictionary thread and remember the threadid     
//                    if start fails then                                       
//                      set return code to indicate memory shortage             
//                    else                                                      
//                      let thread some time to start processing  100 MS        
//                    endif                                                     
//                    return                                                    
//------------------------------------------------------------------------------
VOID EQFDAInit
(
   PDOCUMENT_IDA pDoc                           // pointer to document ida
)
{
   /*******************************************************************/
   /* check if dictionary thread has to be started, i.e. at least     */
   /* one dictionary specified                                        */
   /*******************************************************************/
   if ( ! *(pDoc->szDicts[0]) )
   {
     pDoc->tidDA = (TID) -1;                            // no thread started
     pDoc->pstEQFGen->fsConfiguration &= ~EQFF_DA_CONF; // no dict configured
   }
   else
   {
     pDoc->pstEQFQDA  = pDoc->stEQFSab;            // get dictionary entries
       /***************************************************************/
       /* activate Dictionary                                         */
       /***************************************************************/
       ActivateASD( pDoc );
       pDoc->tidDA = (TID) 1;           // thread successfully started ...
   } /* endif */
   return;
}


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFDAEntries                                              
//------------------------------------------------------------------------------
// Function call:     EQFDAEntries( PDOCUMENT_IDA );                            
//------------------------------------------------------------------------------
// Description:       find all entries for a specific sentence                  
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA       pointer to document ida               
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:                                                               
//                    normalize segment                                         
//                    call ASDTranslate with normalized segment                 
//                    set fDAReady for this segment                             
//                    return                                                    
//------------------------------------------------------------------------------
VOID EQFDAEntries
(
   PDOCUMENT_IDA pDoc
)
{
   PSTEQFSAB     pstQReq;                       // request queue
   PSTEQFGEN     pstEQFGen;                     // pointer to generic structure
   PTWBSDEVICE   pTBDevice;                     // pointer to device structure
   PTBDOCUMENT   pTBDoc;                        // pointer to document
   PSZ_W         pData;                         // pointer to data
   USHORT        usRc;                          // return code from ASDTransl.

   pstEQFGen = pDoc->pstEQFGen;

   pstQReq  = pDoc->pstEQFQDA;        // get pointer to new buffer

   pstQReq->fDAReady  = 0;            // init return code
   /**************************************************************/
   /*  if nothing in segment we are done, else sent request      */
   /*  to AsdTranslate ...                                       */
   /**************************************************************/
   if ( *(pstQReq->pucSourceSeg) )
   {
     USHORT usMode;
     // get rid of any tags or attributes within segment
     pTBDevice = &(pDoc->tbDevProposal);
     pTBDoc = &(pTBDevice->tbDoc);
     /*****************************************************************/
     /* determine if to search for additional info                    */
     /*****************************************************************/
     if (pTBDoc->pUserSettings->fAddInfoDic)
     {
       usMode = ASDT_MARKED_DATA;
     }
     else
     {
       usMode = ASDT_TRANSLATIONS;
     } /* endif */

     // set compound search mode
     if (pTBDoc->pUserSettings->fLkupSingleOfCompounds)
     {
       usMode |= ASDT_SINGLECOMP_LOOKUP;
     } /* endif */

     /*****************************************************************/
     /* determine if to search in all dicts ...                       */
     /*****************************************************************/
     if (pTBDoc->pUserSettings->fAllDictTerms)
     {
       usMode |= ASDT_SEARCH_ALL_DICTS;
     } /* endif */

     pData = pDoc->stTWBS.chNormSegment;
     EQFBNormSeg( pTBDoc, pstQReq->pucSourceSeg, pData );


     /*****************************************************************/
     /* If to select dictionary entries based on PID value, then      */
     /* find out where the necessary PID values are saved in each     */
     /* selected dictionary by reading the MAPTABLE.                  */
     /*****************************************************************/
     PPROPFOLDER ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pDoc->hpropFolder );
     PUCB pUCB = (PUCB)(pDoc->hUCB) ;
     pUCB->fDictPIDSelect = ppropFolder->fDictPIDSelect ;
     if ( ( pUCB->fDictPIDSelect      ) &&
          ( ! pUCB->fDictPIDFieldsSet ) ) {
        strcpy( pUCB->szDictPIDSelect, ppropFolder->szDictPIDSelect1 ) ;
        strcat( pUCB->szDictPIDSelect, ppropFolder->szDictPIDSelect2 ) ;
        pUCB->fDictPIDFieldsSet = TRUE ; 

        HPROP            hDicProp;               // dictionary properties handle
        PPROPDICTIONARY  pDicProp = NULL;        // ptr to dictionary properties
        EQFINFO          ErrorInfo;                   // return code from prop. handler
        USHORT           usLevel ;
        CHAR             szPropName[MAX_EQF_PATH];
        for( int i=0 ; *pDoc->szDicts[i] ; ++i ) {
           pUCB->usDictPIDSelect[i] = 0 ;
           UtlMakeEQFPath( szPropName, NULC, SYSTEM_PATH, NULL );
           strcat( szPropName, BACKSLASH_STR );
           strcat( szPropName, UtlGetFnameFromPath(pDoc->szDicts[i]) );
           hDicProp = OpenProperties( szPropName, NULL,
                                      PROP_ACCESS_READ, &ErrorInfo);
           if ( hDicProp )
           {
             pDicProp = (PPROPDICTIONARY) MakePropPtrFromHnd( hDicProp );
             usLevel = 0 ;
             for (int j=0,iField=0 ; j<=MAX_PROF_ENTRIES; j++)
             {
                iField++;
                if ( pDicProp->ProfEntry[j].usLevel > usLevel ) {
                   usLevel = pDicProp->ProfEntry[j].usLevel ;
                   iField = 0 ;
                }
                if ( ! stricmp( pDicProp->ProfEntry[j].chUserName, "NL PRODUCT" ) ) {
                   pUCB->usDictPIDSelect[i] = (USHORT)iField ;
                   break;
                }
             }
             CloseProperties( hDicProp, PROP_QUIT, &ErrorInfo );
          } 
        }
     }

     // find translations for segment
     usRc = AsdTranslateW(pDoc->hUCB,         // user control block
                          pDoc->hDCB,         // dictionary control block
                          pData,              // source segment
                          EQF_DICTLEN,        // area size
                          pstQReq->pucDictWords,
                          usMode        );       // transl.-and-other-data mode
     /*****************************************************************/
     /* check if we are dealing with dictionary in use...             */
     /*****************************************************************/
     if ( usRc == BTREE_IN_USE )
     {
       pstQReq->fDAReady   = DICT_IN_USE;   // dict not processed because in use
       usRc = LX_RC_OK_ASD;
     }
     else
     {
     } /* endif */
     /************************************************************/
     /* remember error condition if not already set....          */
     /************************************************************/
     if ( (usRc != LX_RC_OK_ASD) && ! pDoc->usDAError )
     {
       pstEQFGen->szMsgBuffer[0] = EOS;
       pstEQFGen->usRC = UtlQdamMsgTxt ( usRc );
       UTF16strcpy (pDoc->szMsgBuffer, pstEQFGen->szMsgBuffer);
       pDoc->usDAError = pstEQFGen->usRC;
     } /* endif */
   }
   else
   {
     memset(pstQReq->pucDictWords, 0, 8 );
   } /* endif */

   pstQReq->fDAReady   |= DICT_DATA_PROCESSED;   // data processed for segment.
   pDoc->fSemDAProc = FALSE;                     // we are waiting for work ..
}


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFDAClose                                                
//------------------------------------------------------------------------------
// Function call:     EQFDAClose( PDOCUMENT_IDA );                              
//------------------------------------------------------------------------------
// Description:       close all open dictionaries                               
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA       pointer to document ida               
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:                                                               
//                    call ASDClose                                             
//                    call ASDEnd                                               
//                    indicate thread finished                                  
//                    return                                                    
//------------------------------------------------------------------------------
VOID  EQFDAClose
(
   PDOCUMENT_IDA pDoc
)
{
   DEBUGEVENT( EQFDACLOSE_LOC, FUNCENTRY_EVENT, 0 );

   pDoc->fSemDAProc = TRUE;              // we are working on cleanup
   if ( pDoc->hDCB )
   {
     AsdClose( pDoc->hUCB, pDoc->hDCB);  // close dictionaries
     pDoc->hDCB = NULL;
   } /* endif */

   if ( pDoc->hUCB )                     // dictionary active
   {
     AsdEnd( pDoc->hUCB );               //      and services
     pDoc->hUCB = NULL;
   } /* endif */

   pStaticDoc = NULL;                    // reset document pointer

   pDoc->fSemDAProc = FALSE;             // indicate ready with cleanup
   pDoc->tidDA  = (TID) -1;                      // indicate thread finished

   DEBUGEVENT( EQFDACLOSE_LOC, FUNCEXIT_EVENT, 0 );
}
