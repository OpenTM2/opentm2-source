// #define MEASURETIME
/*----------------------------------------------------------------------------\*
|                                                                              |
|                Copyright (C) 1990-2013, International Business Machines      |
|                Corporation and others. All rights reserved                   |
|                                                                              |
|   EQFTALP0.C                                                                 |
|                                                                              |
|   Terms List processing main module                                          |
|                                                                              |
|   Author:      G. Queck                                                      |
|                                                                              |
|   Description: this module contains the main entry points for Terms          |
|                List Processing (init, add terms, output lists, terminate)    |
|                                                                              |
|                                                                              |
|   Entry points:  LPInit, LPSegProc, LPCompletion, LPTerminate, UtlSysCheck   |
|                                                                              |
\*----------------------------------------------------------------------------*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#include <eqf.h>                  // General Translation Manager include file

#include "OtmDictionaryIF.H"
//#include "OtmProposal.h"

#include "eqftai.h"               // Private include file for Text Analysis

#if defined(MEASURETIME)
    GINFOSEG FAR *pInfoSeg;             // ptr to global info segment
    ULONG        ulLastTime;
    ULONG        ulAsdOther = 0L;
    ULONG        ulAsdStem = 0L;
    ULONG        ulAsdMerge = 0L;
    ULONG        ulAsdIndex = 0L;
    ULONG        ulAsdRet = 0L;
    ULONG        ulAsdNlp    = 0L;
    ULONG        ulOther = 0L;
    ULONG        ulAsdStemNo = 0L;
    ULONG        ulAsdIndexNo = 0L;
    ULONG        ulAsdRetNo = 0L;

// set last time variable in UCB
#define INITTIME( )  ulLastTime = pInfoSeg->msecs

// get delta time and store it in supplied variable; reset last time
#define GETTIME( ulTime )                       \
{                                               \
   ulTime += pInfoSeg->msecs - ulLastTime;      \
   ulLastTime = pInfoSeg->msecs;                \
}
#endif
/*-------------------- private structures ------------------------------------*/
typedef struct _DICTPROP
   {
   PSZ   pszDictProp    [MAX_DICTS + 1];
   UCHAR szDictPropName [MAX_DICTS + 1][MAX_EQF_PATH];
   } DICTPROP, *PDICTPROP;

/*-------------- private functions definitions -------------------------------*/
//static BOOL LPWordProc
//                (
//                 PTAINPUT ,            // input parameters
//                 PLPDATA  ,                   // local working area
//                 PSZ      ,                   // string containing the term
//                 PLPSEG                       // current segment definition
//                );

//static BOOL LPWordMatchMWT
//                 (
//                 PSZ      ,                   // string containing the term
//                 PSZ      ,                   // string containing found term
//                 PLPDATA  ,                   // local working area
//                 HDCB     ,                   // handle  to dictionaries
//                 PBOOL    ,                   // ptr to termination indicator
//                 PBOOL    ,                   // ptr to found flag
//                 PULONG   ,                   // term number in asd
//                 HDCB    *,                   // handle of dictionary of match
//                 PULONG                       // data length
//                 );

//static BOOL LPWordGetNext
//                 (
//                 PSZ     ,                    // area to get token
//                 USHORT  ,                    // service handle
//                 PVOID   *,                   // current token pointer
//                 PBOOL   ,                    // end of token list
//                 PBOOL                        // termination flag pointer
//                 );

static BOOL LPWordCntCopy
                 (
                 PTAINPUT ,                   // data area for Text Analysis
                 PLPDATA  ,                   // data area for List process
                 PSZ_W      ,                   // term to process
                 PLPSEG   ,                   // text segment
                 ULONG    ,                   // length of dictionary data
                 ULONG    ,                   // term number in dict
                 HDCB     ,                   // dictionary of match
                 PBOOL                        // ptr to termination variable
                );

//static BOOL LPWordGetStem
//                (
//                PSZ     pszTerm,          // input term
//                PSZ     pszStem,          // stem form
//                PBOOL   pfStem,           // stem flag
//                PLPDATA pLP,              // LP data area
//                PBOOL   pfTerminate       // termination flag
//                );

static BOOL LPExclInit
                (
                PTAINPUT ,            // input parameter
                PLPEXCLLIST *         // ptr to ptr to excl list area
                );

static BOOL LPExclEnd
                (
                PLPEXCLLIST *         // pt to pt to excl list.
                );


static BOOL LPExclSearch
                (
                PSZ_W       ,           // term to be searched
                BOOL    * ,           // pt to bool found
                PLPEXCLLIST           // pointer to excl list.
                );

static BOOL LPSegNorm
                (
                PLPDATA      ,        // local data
                PLOADEDTABLE ,        // pointer to TagTable
                PSZ_W          ,        // ptr to data
                PSZ_W                   // ptr to output area
                );


/*--------------------- Macros -----------------------------------------------*/
#define NNODE_LINK_OFFSET ((char *)(&((LPNENTRY *)0)->link) - (char *)0)
#define FNODE_LINK_OFFSET ((char *)(&((LPFENTRY *)0)->link) - (char *)0)


// macro to create dictionary property name w/o property path
#define PROPDICT( pszBuffer, pszDict )                       \
{                                                            \
   UtlMakeEQFPath( pszBuffer, NULC, SYSTEM_PATH, NULL );     \
   strcat( pszBuffer, "\\" );                                \
   strcat(pszBuffer, UtlGetFnameFromPath( pszDict ) );       \
}

/*----------------------------------------------------------------------------*\
| LPInit                                                                       |
| purpose   : Initialize areas and pointers for term list processing           |
|                                                                              |
| Allocates memory for LPDATA structure, Allocates area for Asd services       |
| and initialize them, open the dictionaries                                   |
| initialize the FTL and NTL Lists                                             |
| initialize variables                                                         |
|                                                                              |
| Input     :                                                                  |
|             pointer to TAINPUTPARAM Structure                                |
|             pointer to pointer to LPDATA structure                           |
|                                                                              |
|                                                                              |
| Output    : the LPDATA structure is allocated and initialized                |
|             returns TRUE if successful.                                      |
|                                                                              |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL   LPInit (
              PTAINPUT  pTAInput, // pointer to input parameters struct.
              LPDATA  **ppLPDATA  // pt to pt to Local List processing data
              )
   {
   BOOL     fOK;            // completion indicator
   LPDATA   * pLP = NULL;     // pointer to LPDATA
   PSZ      pID;            // pt to array of string
   PSZ      pszDictName;    // psz to dictionary name
   PSZ      pszTemp1;       // psz temporary 1
   UCHAR    szPropPath[MAX_EQF_PATH];            // psz to property path
   enum error_codes ASDRet; // return code from ASD functions
   USHORT   i;              // loop counter
   USHORT   usErrDict;      // no. of dictionary in error
   SHORT    sRc;            // short return code of called functions
   PDICTPROP pDictProp;                // pointer to structure for dictionaries properties
   BOOL      fNoLockExclDict = FALSE; // no locking for excl dict required
   BOOL      fNoLockOutDict  = FALSE; // no locking for output dict required
   PTAINSTDATA pInD;
                                                                /* 1@KIT1066A */
   PPROPDICTIONARY pProp;             // ptr to dictionary properties
   CHAR            szDictShortName[MAX_FNAME]; // buffer for dictionary short name
   BOOL fIsNew = FALSE;                   // is-new flag

   #if defined(MEASURETIME)
      SEL    selGlobSeg, selLocalSeg;  // selectors returned by DosGetInfoSeg
      DosGetInfoSeg( &selGlobSeg, &selLocalSeg );


      pInfoSeg = MAKEPGINFOSEG(selGlobSeg);
      INITTIME();
   #endif

   pInD = pTAInput->pInD;

    // alloc space for local process data
   fOK = UtlAlloc( (PVOID *) ppLPDATA,                // pt to pt to allocated memory
                   0L,                      // 0 if first allocation
                   (LONG) (sizeof (LPDATA)),  // amount of memory
                   ERROR_STORAGE);            // display error if not space

   if (fOK) // address of LPDATA structure
     {
       pLP = *ppLPDATA;
     }
   else
     {
       pTAInput->pInD->fTerminate = TRUE;       // set termination
     }

   // init ASD services
   if (fOK)
      {

 #if defined(MEASURETIME)
    GETTIME( ulOther );
 #endif
        ASDRet = (enum error_codes) AsdBegin (MAX_DICTS, &(pLP->hUCB));
        fOK = (ASDRet == LX_RC_OK_ASD);

      if (!fOK)
         {
         UtlError( EQFS_DICT_START, // Dictionary services could not be started
                   MB_CANCEL,
                   0,
                   NULL,
                   EQF_ERROR);
         pTAInput->pInD->fTerminate = TRUE;
         }
      } // end of asd initialization

 #if defined(MEASURETIME)
    GETTIME( ulAsdOther);
 #endif

   // open dictionaries
   if (fOK)
      {
      // pointer to input dictionary string array
      pID = (PSZ) pTAInput +  pTAInput->stInputDict.ulOffset;

      // get properties path
      UtlMakeEQFPath ((PSZ)szPropPath, NULC, PROPERTY_PATH, NULL);

      // allocate struc for dict prop
      // build the array of pointer to dictionaries properties
      fOK = UtlAlloc( (PVOID *)&pDictProp,
                      0L,
                      (LONG) (sizeof (DICTPROP)),
                      ERROR_STORAGE);
      if (!fOK)
        {
          pTAInput->pInD->fTerminate = TRUE;
        }

      // fill structure data: prepare each dictionary
      // property name (property path + dict.name + property ext.)
      // and an array of pointer to pointers to prop.names
      if (fOK)
         {
         for (i = 0; i < pTAInput->stInputDict.usNumber; i++)
             {
             /*********************************************************/
             /* Check if name is name of an exclusion or output dict  */
             /*********************************************************/
             if ( pTAInput->szExclDictname[0] &&
                  (stricmp( pID, pTAInput->szExclDictname ) == 0 ) )
             {
               fNoLockExclDict = TRUE;
             } /* endif */
             if ( pTAInput->szOutDictName[0] &&
                  (stricmp( pID, pTAInput->szOutDictName ) == 0 ) )
             {
               fNoLockOutDict  = TRUE;
             } /* endif */
            fIsNew = FALSE;                   // is-new flag
            memset( szDictShortName, 0, sizeof(szDictShortName) );
            ANSITOOEM(pID);
            ObjLongToShortName( pID, szDictShortName, DICT_OBJECT, &fIsNew );
            OEMTOANSI(pID);
             pszTemp1 = (PSZ)(pDictProp->szDictPropName [i]);
             sprintf (pszTemp1,
                      PATHCATFILECATEXTENSION,
                      szPropPath,
                      szDictShortName, /*          pID, */
                      EXT_OF_DICTPROP);
             pDictProp->pszDictProp [i] = pszTemp1; // store pointer in array
             pID += strlen(pID) + 1;  // point to next dict
             } /* endfor fill struc data */
         }

      /****************************************************************/
      /* Check if input dictionaries are locked                       */
      /****************************************************************/
      if (fOK && pTAInput->stInputDict.usNumber > 0)  // input dictionaries
      {
        for ( i = 0; (fOK && (i < pTAInput->stInputDict.usNumber)); i++)
        {
          PROPDICT( pTAInput->szTempName, pDictProp->pszDictProp[i] );
          sRc = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                                    WM_EQF_QUERYSYMBOL,
                                    NULL,
                                    MP2FROMP( pTAInput->szTempName ));
          if ( sRc != -1 )
          {
             pszTemp1 = UtlGetFnameFromPath( pDictProp->pszDictProp[i] );
             pszTemp1 = Utlstrccpy( pTAInput->szTempName, pszTemp1, DOT );
             UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszTemp1, EQF_ERROR );
             fOK = FALSE;
          } /* endif */
        } /* endfor */
      } /* endif */

      /****************************************************************/
      /* Lock input dictionaries                                      */
      /****************************************************************/
      if (fOK && pTAInput->stInputDict.usNumber > 0)  // input dictionaries
      {
        for ( i = 0; i < pTAInput->stInputDict.usNumber; i++)
        {
          PROPDICT( pTAInput->szTempName, pDictProp->pszDictProp[i] );
        } /* endfor */
      } /* endif */


      if (fOK && pTAInput->stInputDict.usNumber > 0)  // input dictionaries
         {
 #if defined(MEASURETIME)
    GETTIME( ulOther);
 #endif
         ASDRet = (enum error_codes) AsdOpen (
                       pLP->hUCB,           // current user service area
                       0,                   // no guarded mode, no NOINDEX
                       pTAInput->stInputDict.usNumber, // no of dictionaries
                       (PSZ *) pDictProp,   // ptr to array of dict properties
                       &(pLP->hSearchDict), // dictionary handle
                       &usErrDict           // no of dictionaries in error
                       );
        fOK = (ASDRet == LX_RC_OK_ASD) ;

 #if defined(MEASURETIME)
    GETTIME( ulAsdOther);
 #endif

         // check single/multiple dict err.
         if (!fOK)
            {   // general error
            if (usErrDict >= pTAInput->stInputDict.usNumber + 1)
               {
               UtlError(
                       ERROR_TA_LPASSOC, //  the association of dictionaries
                       MB_CANCEL,         //  failed;
                       0,
                       NULL,
                       EQF_ERROR
                       );

               pTAInput->pInD->fTerminate = TRUE;
               }
            else
               {
               // single dict. error: retrieve the dict. name
               pszDictName = (PSZ) pTAInput +
                             pTAInput->stInputDict.ulOffset;  // first name
               for (i = 1; i < usErrDict; i++)
                   {
                   pszDictName += strlen(pszDictName) + 1;
                   } /* endfor get dict name */

               UtlError(
                         ERROR_TA_LPDICTOPEN, //  the dictionary %1 failed
                         MB_CANCEL,           //  to open;
                         1,
                         &(pszDictName),
                         EQF_ERROR
                         );

               pTAInput->pInD->fTerminate = TRUE;
               }


            /**********************************************************/
            /* Unlock all dictionaries                                */
            /**********************************************************/
            for ( i = 0; i < pTAInput->stInputDict.usNumber; i++)
            {
              PROPDICT( pTAInput->szTempName, pDictProp->pszDictProp[i] );
              sRc = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                                        WM_EQF_REMOVESYMBOL,
                                        NULL,
                                        MP2FROMP( pTAInput->szTempName ));
            } /* endfor */
          } /* endif errors from dictionaries open */

        if ( fOK )
        {
          // get dictionary name
          pszDictName = (PSZ) pTAInput +
                        pTAInput->stInputDict.ulOffset;  // first name
          for (i = 1; i < usErrDict; i++)
              {
              pszDictName += strlen(pszDictName) + 1;
              } /* endfor get dict name */
        } /* endif */

 #if defined(MEASURETIME)
    GETTIME( ulOther);
 #endif
 #if defined(MEASURETIME)
    GETTIME( ulAsdOther);
 #endif


            if (!fOK && !pTAInput->pInD->fTerminate)
               {   // general error
               UtlError( ERROR_TA_LPASSOC, // Association of Input dictionaries
                         MB_CANCEL,        // failed. The execution terminates.
                         0,
                         NULL,
                         EQF_ERROR
                         );

               pTAInput->pInD->fTerminate = TRUE;
               }

            /**********************************************************/
            /* Build name of exclusion dictionary                     */
            /**********************************************************/
            if (fOK && *(pTAInput->szExclDictname))   // exclusion dictionary
            {
               BOOL fIsNew = FALSE;
               CHAR szShortName[MAX_FILESPEC];

               pszTemp1 = (PSZ)(pDictProp->szDictPropName [0]);  // prepare properties
               memset( szShortName, 0, sizeof(szShortName) );
               ANSITOOEM(pTAInput->szExclDictname);
               ObjLongToShortName( pTAInput->szExclDictname, szShortName, DICT_OBJECT, &fIsNew );
               OEMTOANSI(pTAInput->szExclDictname);
               sprintf( pszTemp1, PATHCATFILECATEXTENSION, szPropPath, szShortName, EXT_OF_DICTPROP);
               pDictProp->pszDictProp [0] = pszTemp1;      // store pointer in array
            } /* endif */

            /**********************************************************/
            /* Check lock and lock of exclusion dictionary (only if   */
            /* not contained in input dictionary array)               */
            /**********************************************************/
            if ( fOK &&
                 *(pTAInput->szExclDictname) &&
                 !fNoLockExclDict )
            {
              PROPDICT( pTAInput->szTempName, pDictProp->pszDictProp[0] );
              sRc = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                                        WM_EQF_QUERYSYMBOL,
                                        NULL,
                                        MP2FROMP( pTAInput->szTempName ));
              if ( sRc != -1 )
              {
                pszTemp1 = UtlGetFnameFromPath( pDictProp->pszDictProp[0] );
                pszTemp1 = Utlstrccpy( pTAInput->szTempName, pszTemp1, DOT );
                UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszTemp1, EQF_ERROR );
                fOK = FALSE;
              }
              else
              {
                // no locking for exclusion dictionary here as dictionary
                // is locked in function TALockFiles
              } /* endif */
            } /* endif */

            if (fOK && *(pTAInput->szExclDictname))   // exclusion dictionary
               {
 #if defined(MEASURETIME)
    GETTIME( ulOther);
 #endif
               ASDRet = (enum error_codes) AsdOpen (
                             pLP->hUCB,           // current user service area
                             0,                   // no guarded mode, no NOINDEX
                             1,                   // no of dictionaries
                             (PSZ *) pDictProp,   // ptr to array of dict prop.
                             &(pLP->hExclDict),   // dictionary handle
                             &usErrDict           // no of dictionaries in error
                             );

 #if defined(MEASURETIME)
    GETTIME( ulAsdOther);
 #endif
               // check single/multiple dict err.
               if ((fOK = (ASDRet == LX_RC_OK_ASD)) == 0)
                  {
                  pszTemp1 = pTAInput->szExclDictname;
                  UtlError(
                          ERROR_TA_LPDICTOPEN, //  the dictionary %1 failed
                          MB_CANCEL,           //  to open;
                          1,
                          &pszTemp1,
                          EQF_ERROR
                          );

                  pTAInput->pInD->fTerminate = TRUE;

                  /****************************************************/
                  /* Unlock dictionary                                */
                  /****************************************************/
                  if ( !fNoLockExclDict )
                  {
                    sRc = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                                              WM_EQF_REMOVESYMBOL,
                                              0L,
                                              MP2FROMP( pTAInput->szTempName ));
                  } /* endif */
                  }
               }

            if (fOK && *(pTAInput->szOutDictName))   // output dictionary
            {
              BOOL fIsNew = FALSE;

              pszTemp1 = (PSZ)(pDictProp->szDictPropName[0]);  // prepare properties
              strcpy( pszTemp1, (PSZ)szPropPath );
              strcat( pszTemp1, BACKSLASH_STR );
              ObjLongToShortName( pTAInput->szOutDictName, pszTemp1 + strlen(pszTemp1), DICT_OBJECT, &fIsNew );
              strcat( pszTemp1, EXT_OF_DICTPROP);
              pDictProp->pszDictProp[0] = pszTemp1; // store pointer in array
            } /* endif */

            /**********************************************************/
            /* Check lock and lock of output dictionary (only if      */
            /* not contained in input dictionary array and not used   */
            /* as exclusion dictionary)                               */
            /**********************************************************/
            if ( fOK &&
                 *(pTAInput->szOutDictName) &&
                 (stricmp( pTAInput->szOutDictName, pTAInput->szExclDictname) != 0 ) &&
                 !fNoLockOutDict )
            {
              PROPDICT( pTAInput->szTempName, pDictProp->pszDictProp[0] );
              sRc = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                                        WM_EQF_QUERYSYMBOL,
                                        NULL,
                                        MP2FROMP( pTAInput->szTempName ));
              if ( sRc != -1 )
              {
                pszTemp1 = UtlGetFnameFromPath( pDictProp->pszDictProp[0] );
                pszTemp1 = Utlstrccpy( pTAInput->szTempName, pszTemp1, DOT );
                UtlError( ERROR_DICT_LOCKED, MB_CANCEL, 1, &pszTemp1, EQF_ERROR );
                fOK = FALSE;
              } /* endif */
            } /* endif */


            if (fOK && *(pTAInput->szOutDictName))   // output dictionary
               {
 #if defined(MEASURETIME)
    GETTIME( ulOther);
 #endif
               ASDRet = (enum error_codes) AsdOpen (
                             pLP->hUCB,           // current user service area
                             0,                   // no guarded mode, no NOINDEX
                             1,                   // no of dictionaries
                             (PSZ *) pDictProp,   // ptr to array of dict prop.
                             &(pLP->hOutDict),    // dictionary handle
                             &usErrDict           // no of dictionaries in error
                             );
 #if defined(MEASURETIME)
    GETTIME( ulAsdOther);
 #endif

               // check single/multiple dict err.
               if ((fOK = (ASDRet == LX_RC_OK_ASD)) == 0)
                  {
                  pszTemp1 = pTAInput->szOutDictName;
                  UtlError(
                          ERROR_TA_LPDICTOPEN, //  the dictionary %1 failed
                          MB_CANCEL,           //  to open;
                          1,
                          &pszTemp1,
                          EQF_ERROR
                          );

                  pTAInput->pInD->fTerminate = TRUE;

                  /****************************************************/
                  /* Unlock dictionary                                */
                  /****************************************************/
                  if ( (stricmp( pTAInput->szOutDictName,
                                 pTAInput->szExclDictname) != 0 ) &&
                       !fNoLockOutDict )
                  {
                    sRc = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                                              WM_EQF_REMOVESYMBOL,
                                              0L,
                                              MP2FROMP( pTAInput->szTempName ));
                  } /* endif */
                }
                                                               /* 40@KIT1066A */
               else
               {
                 /*****************************************************/
                 /* Check if output dictionary is protected           */
                 /*****************************************************/
                 ASDRet = (enum error_codes) AsdRetPropPtr( pLP->hUCB, pLP->hOutDict, &pProp );

                 if ( ASDRet != LX_RC_OK_ASD )
                 {
                   pszTemp1 = pTAInput->szOutDictName;
                   UtlError( ERROR_TA_LPDICTOPEN, MB_CANCEL, 1, &pszTemp1,
                             EQF_ERROR );
                   fOK = FALSE;
                 }
                 else if ( pProp->fProtected || pProp->fCopyRight )
                 {
                   pszTemp1 = pTAInput->szOutDictName;
                   UtlError( ERROR_ADDTODICT_PROTECTED, MB_CANCEL, 1, &pszTemp1,
                             EQF_ERROR );
                   fOK = FALSE;
                 } /* endif */

                 if ( !fOK )
                 {
                   pTAInput->pInD->fTerminate = TRUE;
                   AsdClose( pLP->hUCB, pLP->hOutDict );

                   /****************************************************/
                   /* Unlock dictionary                                */
                   /****************************************************/
                   if ( (stricmp( pTAInput->szOutDictName,
                                  pTAInput->szExclDictname) != 0 ) &&
                        !fNoLockOutDict )
                   {
                     sRc = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                                               WM_EQF_REMOVESYMBOL,
                                               0L,
                                               MP2FROMP( pTAInput->szTempName ));
                   } /* endif */
                 } /* endif */
                }
               } /* endif */

         UtlAlloc( (PVOID *)&pDictProp, 0L, 0L, NOMSG); // free space alloc.
         } /* end of input dict open */

      } /* end of ASD open */

   if (fOK && *(pTAInput->szNTLname) != '\0') // allocate NT list
      {
      pLP->pNTL = ListAlloc (NNODE_LINK_OFFSET,
                       (PFN_CMP)LPNCompare,
                       (PFN_FREE)LPNFree);

      fOK = (pLP->pNTL != NULL);    // errors in allocation
      if (!fOK)
         {
         pTAInput->pInD->fTerminate = TRUE;
         }
      }

   if (fOK) // allocate FT list   in any case
      {
      pLP->pFTL = ListAlloc (FNODE_LINK_OFFSET,
                             (PFN_CMP)LPFCompare,
                             (PFN_FREE)LPFFree);

      fOK = (pLP->pFTL != NULL);  // errors in allocation
      if (!fOK)
         {
         pTAInput->pInD->fTerminate = TRUE;
         }
      }

   if (fOK)              // init scalars
      {
      // init max number of words in MWT
      pLP->usMWTMaxLength = 1;
      // init merge dictionaries flags; these could be eventually user options
      pLP->usMergeFlags = MERGE_ADD | MERGE_NOUSERPROMPT;
      }

   // init dictionary output work area
   if (fOK)
      {
      fOK = UtlAlloc( (PVOID *)&(pLP->puchTermBuf), 0L, DICT_INIT_SIZE * sizeof(CHAR_W), ERROR_STORAGE);
      if (fOK)
         pLP->ulTermBufSize = DICT_INIT_SIZE;
      else
         pTAInput->pInD->fTerminate = TRUE;   // set termination
      }

   // init term work area
   if (fOK)
      {
      fOK = UtlAlloc( (PVOID *)&(pLP->puchTermArea), 0L, 8192L, ERROR_STORAGE);
      if (fOK)
         pLP->ulTermAreaSize = 8192L;
      else
         pTAInput->pInD->fTerminate = TRUE;   // set termination
      }

   if (fOK && *(pTAInput->szExclusionList))
      {
      fOK = LPExclInit (pTAInput, &(pLP->pExclList));
      }  // fTerminate is set in LPEclInit, in case of errors

   if (fOK)
      {                         // copy source files path name to lpdata
      strcpy((PSZ)(pLP->szSSourcePath), pTAInput->szSEGSOURCE_Path);
      }

    if (fOK)
      {                        // Load the tag table for LPTLOutput
      fOK = LPLoadTagTable(pLP, pTAInput);
      }

   return (fOK);
   }

/*----------------------------------------------------------------------------*\
| LPTerminate                                                                  |
| purpose   : Close all dictionaries, free allocated resources, free local data|
|                                                                              |
| the resources are freed in the reverse order in which they were allocated    |
|                                                                              |
| Input     :                                                                  |
|             pointer to pointer to LPDATA structure                           |
|                                                                              |
|                                                                              |
| Output    : the LPDATA structure is freed, all opened services are terminated|
|             all opened files are closed, all allocated memory is freed       |
|             returns always TRUE.                                             |
|                                                                              |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL   LPTerminate (
                   LPDATA  **ppLPDATA  // pt to pt to Local List processing data
                   )
   {
   LPDATA   *pLP;                      // pointer to LPDATA
   HDCB   ahDCB[MAX_DICTS+1];          // list of associated dictionaries
   USHORT usNumOfDicts;                // number of dictionaries in association
   USHORT usI;                         // loop index

   pLP = *ppLPDATA;        // get pointer to LPDATA

   if (pLP != NULL)
      {
      // free allocated memory for MWT work area (allocated automatically)
      UtlAlloc( (PVOID *)&(pLP->pchMatches), 0L, 0L, NOMSG);

      // free allocated memory for dictionary output (allocated in LPWordProc
      UtlAlloc( (PVOID *)&(pLP->puchTermBuf), 0L, 0L, NOMSG);
      UtlAlloc( (PVOID *)&(pLP->puchTermArea), 0L, 0L, NOMSG);

      if (pLP->pListFormatTable != NULL )
      {
         TAFreeTagTable(pLP->pListFormatTable);
         pLP->pListFormatTable = NULL;
      }
      // free the exclusion list data area
      if (pLP->pExclList != NULL)
         {
         LPExclEnd( &(pLP->pExclList));
         }

      // free NTL List
      if (pLP->pNTL != NULL)
         ListFree (pLP->pNTL);

      // free FTL List
      if (pLP->pFTL != NULL)
         ListFree (pLP->pFTL);

      // free the memory allocated for words
      LPTermFree (pLP);

      // close context files & free list of file handles
      LPCloseFiles(&(pLP->pLstFCB));

      if (pLP->hUCB != NULL)
         {
 #if defined(MEASURETIME)
    GETTIME( ulOther);
 #endif
         // close exclusion  dictionary
         if (pLP->hExclDict != NULL)
            {
            /**********************************************************/
            /* unlock dictionary                                      */
            /**********************************************************/
            UtlMakeEQFPath((PSZ)( pLP->uchWorkArea), NULC, SYSTEM_PATH, NULL );
            strcat( (PSZ)(pLP->uchWorkArea), BACKSLASH_STR );
            AsdQueryDictShortName( pLP->hExclDict,
                              (PSZ)(pLP->uchWorkArea + strlen((PSZ)(pLP->uchWorkArea)) ));
            strcat( (PSZ)(pLP->uchWorkArea), EXT_OF_DICTPROP );
            WinSendMsg( EqfQueryObjectManager(),
                        WM_EQF_REMOVESYMBOL,
                        NULL,
                        MP2FROMP( pLP->uchWorkArea ));

            /**********************************************************/
            /* Close dictionary                                       */
            /**********************************************************/
            AsdClose (
                      pLP->hUCB,         // user control block handle
                      pLP->hExclDict     // dict. control block h.
                     );

            }

         // close output   dictionary
         if (pLP->hOutDict != NULL)
            {
            /**********************************************************/
            /* unlock dictionary                                      */
            /**********************************************************/
            UtlMakeEQFPath( (PSZ)(pLP->uchWorkArea), NULC, SYSTEM_PATH, NULL );
            strcat( (PSZ)(pLP->uchWorkArea), BACKSLASH_STR );
            AsdQueryDictShortName( pLP->hOutDict,
                              (PSZ)(pLP->uchWorkArea + strlen((PSZ)(pLP->uchWorkArea)) ));
            strcat( (PSZ)(pLP->uchWorkArea), EXT_OF_DICTPROP );
            WinSendMsg( EqfQueryObjectManager(),
                        WM_EQF_REMOVESYMBOL,
                        NULL,
                        MP2FROMP( pLP->uchWorkArea ));

            /**********************************************************/
            /* Close dictionary                                       */
            /**********************************************************/
            AsdClose (
                      pLP->hUCB,         // user control block handle
                      pLP->hOutDict     // dict. control block h.
                     );
            }

         // close input (search) dictionaries
         if (pLP->hSearchDict != NULL)
            {
            /**********************************************************/
            /* unlock dictionaries                                    */
            /**********************************************************/
            AsdRetDictList( pLP->hSearchDict, ahDCB, &usNumOfDicts );
            for ( usI = 0; usI < usNumOfDicts; usI++ )
            {
              UtlMakeEQFPath( (PSZ)(pLP->uchWorkArea), NULC, SYSTEM_PATH, NULL );
              strcat( (PSZ)(pLP->uchWorkArea), BACKSLASH_STR );
              AsdQueryDictShortName( ahDCB[usI],
                               (PSZ)( pLP->uchWorkArea + strlen((PSZ)(pLP->uchWorkArea)) ));
              strcat( (PSZ)pLP->uchWorkArea, EXT_OF_DICTPROP );
              WinSendMsg( EqfQueryObjectManager(),
                          WM_EQF_REMOVESYMBOL,
                          NULL,
                          MP2FROMP( pLP->uchWorkArea ));
            } /* endfor */

            /**********************************************************/
            /* Close dictionary                                       */
            /**********************************************************/
            AsdClose (
                      pLP->hUCB,         // user control block handle
                      pLP->hSearchDict   // dict. control block h.
                     );
            }


                   if ( pLP->pTermList )
                   {
                     UtlAlloc( (PVOID *) (PVOID *)&pLP->pTermList, 0L, 0L, NOMSG );
                   } /* endif */

                   if ( pLP->pStemTermList )
                   {
                     UtlAlloc( (PVOID *) (PVOID *)&pLP->pStemTermList, 0L, 0L, NOMSG );
                   } /* endif */

         // terminate ASD services
         AsdEnd (pLP->hUCB);                     // user control block handle

         } /* endif valid service area */
 #if defined(MEASURETIME)
    GETTIME( ulAsdOther);
    {
      FILE *hTimeLog;

       hTimeLog = fopen( "\\LIST.LOG", "a" );
       fprintf( hTimeLog, "==========================================\n");
       fprintf( hTimeLog, "ListProcessing: %10ld ms \n",ulOther );
       fprintf( hTimeLog, "AsdOther:       %10ld ms \n",ulAsdOther );
       fprintf( hTimeLog, "AsdNlp  :       %10ld ms \n",ulAsdNlp   );
       fprintf( hTimeLog, "AsdStem:        %10ld ms \n",ulAsdStem );
       fprintf( hTimeLog, "   Number:      %10ld ms \n",ulAsdStemNo );
       fprintf( hTimeLog, "AsdIndex:       %10ld ms \n",ulAsdIndex );
       fprintf( hTimeLog, "   Number:      %10ld ms \n",ulAsdIndexNo );
       fprintf( hTimeLog, "AsdRet:         %10ld ms \n",ulAsdRet );
       fprintf( hTimeLog, "   Number:      %10ld ms \n",ulAsdRetNo );
       fprintf( hTimeLog, "AsdMerge:       %10ld ms \n",ulAsdMerge );

       fclose( hTimeLog );
    }
 #endif
      // free alloc space for local process data
      UtlAlloc( (PVOID *) ppLPDATA, 0L, 0L, NOMSG);
      } /* endif LP valid */


   return (TRUE);
   }

/*----------------------------------------------------------------------------*\
| LPSegProc                                                                    |
| purpose   : Extract terms from segments; pass the terms extracted to         |
|             LPWordProc to do actual Term processing.  The data received in   |
|             input is first cleaned from all remaining markup strings; it     |
|             is then parsed with NlpComplex and the generated list of         |
|             tokens is scanned.  Token of type TEXT and without the           |
|             attribute LX_NO_DICT_LOOK (as defined in EQFDASD.H and           |
|             returned from NlpFlgToken:  this excludes punctuation and        |
|             numbers from process) are extracted and passed to LPWordProc.    |
|                                                                              |
| Input     :                                                                  |
|             window handle                                                    |
|             pointer to LPDATA structure                                      |
|             pointer to TAINPUTPARAM Structure                                |
|             psz Segment to process                                           |
|             ptr to the structure that defines the segment                    |
|                                                                              |
| prerequisites : must be preceded by a successful call to LPInit              |
|                                                                              |
| Output    :                                                                  |
|             side effects: the NTL & FTL lists are updated                    |
|             returns TRUE if successful, else FALSE                           |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL   LPSegProc (
                 HWND     hwnd,        // window h
                 PLPDATA  pLP,         // pt to Local List processing data
                 PTAINPUT pTAInput,    // pointer to input parameters struct.
                 PSZ_W    pszSegment,  // the segment of text to process
                 PLPSEG   pLPSeg       // segment definition
                 )
   {
   BOOL         fOK;                // global status indicator
   TAGTABLE     *pTagTable;         // pointer to TagTable
   PSZ_W        pszTerm;               // pointer to term
   PSZ_W        pszTemp;            // temporary pointer
   USHORT       usRC;               // return code for NLP functions
   USHORT       usSrvc;             // Service handle
   enum TERM_REASON TermCode;       // external termination reason
   BOOL         fFound;                // term found flag
   USHORT       usASDRC;               // return code of Asd... functions
   ULONG        ulTermNumber;          // term number in asd
   ULONG        ulDataLength;          // length of data record from ASD
   HDCB         hMatchDict;            // handle of dictionary of match
   PSZ          pszDictName;           // Temp dictionary name
   USHORT       usMWTOption;
   // fix for KIT1016
   CHAR         chDictName[MAX_LONGFILESPEC]; // dict name
   // /fix for KIT1016
   USHORT       usFreq;                // frequency of a term in tables

   // assure that ulSrcOemCP is filled with CP of document src language
   pLP->ulSrcOemCP = pTAInput->pInD->TolstControl.ulOemCP;
   if (!pLP->ulSrcOemCP)
   {
     pLP->ulSrcOemCP = GetLangOEMCP(pTAInput->pInD->szDocSourceLang);
   }
   pLP->szWorkSegment[0] = EOS;                                 /* 1@KIT1185A */

   usSrvc = pLP->usServiceHandle; // service handle temporary

   pLP->ulWorkSize = WORK_AREA_SIZE; // init work area size

                           // get tag table pointer and check for validity
   fOK = ((pTagTable = pTAInput->pInD->pLoadedTable->pTagTable) != NULL);

   if (fOK)
      {
      fOK = LPSegNorm (pLP, pTAInput->pInD->pLoadedTable,
                       pszSegment, pLP->szWorkSegment);
      }

   /*******************************************************************/
   /* Get words for lookup (including MWT recognition)                */
   /*******************************************************************/
   if ( fOK )
   {
 #if defined(MEASURETIME)
    GETTIME( ulOther );
 #endif

    // get MWT depending on selected option

    if (pTAInput->fNTLMwterm)
    {
        if (pTAInput->usMWTOption == 0)
        {
          usMWTOption = EQF_MWT_START_NOUN;
        }
        else if (pTAInput->usMWTOption == 1)
        {
          usMWTOption = EQF_MWT_START_ADJ;
        }

        else if (pTAInput->usMWTOption == 2)
        {
          usMWTOption = EQF_MWT_START_NOUN_ADJ;
        }
        else
        {
           usMWTOption = EQF_MWT_EMPTY;
        }

    }
    else
    {
       usMWTOption = EQF_MWT_EMPTY;
    }


    usRC = MorphWordRecognitionW( pTAInput->pInD->TolstControl.sLangID,
                           &(pLP->szWorkSegment[0]),
                           pLP->hUCB,           // user control block
                           pLP->hSearchDict,    // input dictionary
                           ((PULONG)&pLP->ulTermListSize),// size of term list
                           &pLP->pTermList,     // ptr to term list
                           NULL,                // size of org term list
                           NULL,                // ptr to org term list
                           TRUE,                // include not found terms
                           &usASDRC,            // ptr to ASD return call
                           usMWTOption ); // MWT option




 #if defined(MEASURETIME)
    GETTIME( ulAsdNlp   );
 #endif
     if ( usRC == MORPH_ASD_ERROR )
     {
       fOK = FALSE;
       pszDictName = chDictName;    // set pointer
       AsdQueryDictName( pLP->hSearchDict, pszDictName );
       OEMTOANSI( pszDictName );
       UtlError( usASDRC, MB_CANCEL, 1, &pszDictName, QDAM_ERROR );
     }
     else if ( usRC != MORPH_OK )
     {
       fOK = FALSE;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Process returned term list                                      */
   /*******************************************************************/
   pszTerm = pLP->pTermList;
   while ( fOK && *pszTerm )           // loop while fOK and not end-of-list
   {
     fFound = FALSE;                   // initialize found flag

     /*****************************************************************/
     /* Ignore terms with a length of 2 or smaller                    */
     /*****************************************************************/
     if ( UTF16strlenCHAR(pszTerm) < 3 )        // ignore terms with less than 3 chars
     {
       fFound = TRUE;
     } /* endif */

     /*****************************************************************/
     /* Ignore terms consisting of digits and punctuation only (dates,*/
     /* numbers, ... )                                                */
     /*****************************************************************/
     if ( !fFound )
     {
       pszTemp = pszTerm;
       fFound = TRUE;
       while ( *pszTemp && fFound )
       {
         switch ( *pszTemp )
         {
           case '0' :
           case '1' :
           case '2' :
           case '3' :
           case '4' :
           case '5' :
           case '6' :
           case '7' :
           case '8' :
           case '9' :
           case '.' :
           case ',' :
           case ' ' :
             pszTemp++;                // test next character
             break;
           default :
             fFound = FALSE;           // leave loop
             break;
         } /* endswitch */
       } /* endwhile */
     } /* endif */

     /*****************************************************************/
     /* Check term against exclusion list                             */
     /*****************************************************************/
     if ( !fFound && (pLP->pExclList != NULL) )
     {
       LPExclSearch( pszTerm,             // term to be searched
                     &fFound,             // bool found
                     pLP->pExclList);     // pointer to excl list.
     } /* endif */

     if ( !fFound )                       // get stem form
     {
 #if defined(MEASURETIME)
    GETTIME( ulOther );
 #endif
       usRC = MorphGetStemForm( pTAInput->pInD->TolstControl.sLangID,
                                pszTerm,
                                &pLP->usStemTermListSize,
                                &pLP->pStemTermList,
                                pTAInput->pInD->TolstControl.ulOemCP);
 #if defined(MEASURETIME)
    GETTIME( ulAsdStem );
    ulAsdStemNo++;
 #endif
       if ( usRC == MORPH_NOT_FOUND )
       {
         UTF16strcpy( pLP->szStem, pszTerm );
         usRC = MORPH_OK;
       }
       else if ( usRC == MORPH_OK )
       {
         UTF16strcpy( pLP->szStem, pLP->pStemTermList );
       }
       else
       {
         fOK = FALSE;
       } /* endif */

       if ( (UTF16strcmp(pszTerm, pLP->szStem) != 0 ) &&
            (pLP->pExclList != NULL ) )
       {
         LPExclSearch( pLP->szStem,         // term to be searched
                       &fFound,             // bool found
                       pLP->pExclList);     // pointer to excl list.
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Check term against exclusion dictionary                       */
     /*****************************************************************/
     if (fOK && !fFound && pLP->hExclDict)
     {
       /***************************************************************/
       /* Try term as-is                                              */
       /***************************************************************/
 #if defined(MEASURETIME)
    GETTIME( ulOther );
 #endif
       usASDRC = AsdFndEquivW (
                            pszTerm,          // desired term
                            pLP->hExclDict,   // handle to dictionaries
                            pLP->hUCB,        // user control block
                            pLP->szFnd,       // string containing found term
                            &ulTermNumber,    // term number in dict
                            &ulDataLength,    // entry data length
                            &hMatchDict );    // handle of dictionary of match
 #if defined(MEASURETIME)
    GETTIME( ulAsdOther );
 #endif

       fFound = (usASDRC == LX_RC_OK_ASD); // found condition
       fOK    = ((usASDRC == LX_RC_OK_ASD) || (usASDRC == LX_WRD_NT_FND_ASD));

       /***************************************************************/
       /* Try stem form of term                                       */
       /***************************************************************/
       if (fOK && !fFound && (UTF16strcmp(pszTerm, pLP->szStem) != 0 ) )
       {
 #if defined(MEASURETIME)
    GETTIME( ulOther );
 #endif
         usASDRC = AsdFndEquivW (
                            pLP->szStem,      // stem form desired term
                            pLP->hExclDict,   // handle to dictionaries
                            pLP->hUCB,        // user control block
                            pLP->szFnd,       // string containing found term
                            &ulTermNumber,    // term number in dict
                            &ulDataLength,    // entry data length
                            &hMatchDict );    // handle of dictionary of match
 #if defined(MEASURETIME)
    GETTIME( ulAsdOther );
 #endif
         fFound = (usASDRC == LX_RC_OK_ASD);
         fOK    = ((usASDRC == LX_RC_OK_ASD) || (usASDRC == LX_WRD_NT_FND_ASD));
       } /* endif */

       if ( !fOK )
       {
         // fix for KIT1016
         pszDictName = chDictName;    // set pointer
         // /fix for KIT1016
         AsdQueryDictName( pLP->hExclDict, pszDictName );
         OEMTOANSI( pszDictName );
         UtlError( usASDRC, MB_CANCEL, 1, &pszDictName, QDAM_ERROR );
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Search term in input dictionaries                             */
     /*****************************************************************/
     if ( fOK && !fFound )
     {
       /***************************************************************/
       /* Try term as-is                                              */
       /***************************************************************/
 #if defined(MEASURETIME)
    GETTIME( ulOther );
 #endif
       usASDRC = AsdFndEquivW (
                            pszTerm,          // desired term
                            pLP->hSearchDict, // handle to dictionaries
                            pLP->hUCB,        // user control block
                            pLP->szFnd,       // string containing found term
                            &ulTermNumber,    // term number in dict
                            &ulDataLength,    // entry data length
                            &hMatchDict );    // handle of dictionary of match
 #if defined(MEASURETIME)
    GETTIME( ulAsdOther );
 #endif

       fFound = (usASDRC == LX_RC_OK_ASD); // found condition
       fOK    = ((usASDRC == LX_RC_OK_ASD) || (usASDRC == LX_WRD_NT_FND_ASD));

       /***************************************************************/
       /* Try stem form of term                                       */
       /***************************************************************/
       if (fOK && !fFound && (UTF16strcmp(pszTerm, pLP->szStem) != 0 ) )
       {
 #if defined(MEASURETIME)
    GETTIME( ulOther );
 #endif
         usASDRC = AsdFndEquivW (
                            pLP->szStem,      // stem form desired term
                            pLP->hSearchDict, // handle to dictionaries
                            pLP->hUCB,        // user control block
                            pLP->szFnd,       // string containing found term
                            &ulTermNumber,    // term number in dict
                            &ulDataLength,    // entry data length
                            &hMatchDict );    // handle of dictionary of match
 #if defined(MEASURETIME)
    GETTIME( ulAsdOther );
 #endif
         fFound = (usASDRC == LX_RC_OK_ASD);
         fOK    = ((usASDRC == LX_RC_OK_ASD) || (usASDRC == LX_WRD_NT_FND_ASD));
       } /* endif */

       if ( !fOK )
       {
         // fix for KIT1016
         pszDictName = chDictName;    // set pointer
         // /fix for KIT1016
         AsdQueryDictName( pLP->hSearchDict, pszDictName );
         OEMTOANSI( pszDictName );
         UtlError( usASDRC, MB_CANCEL, 1, &pszDictName, QDAM_ERROR );
       } /* endif */

       /***************************************************************/
       /* If term found in dictionaries:                              */
       /*          count it in FTL and copy to output dictionary      */
       /***************************************************************/
       if ( fOK && fFound && ((*(pTAInput->szFTLname) || pLP->hOutDict)) )
       {
         fOK  = LPWordCntCopy ( pTAInput,
                                pLP,
                                pLP->szFnd,   // term to process
                                pLPSeg,
                                ulDataLength,
                                ulTermNumber,
                                hMatchDict,
                                &(pTAInput->pInD->fTerminate)
                                );
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Term not found so far: try to hit term on "NEW" List          */
     /*****************************************************************/
     if ( fOK && !fFound && *(pTAInput->szNTLname) )
     {
       fOK = LPNAdd( pTAInput,
                     pLP,
                     &fFound,               // address of the found flag
                     pszTerm,           // term to search
                     pLPSeg,
                     &usFreq                // freq.
                    );
     } /* endif */


     /*****************************************************************/
     /* Term NOT found in the above: add it to NEW Terms List         */
     /*****************************************************************/
     if ( fOK && !fFound )
     {
       if (*(pTAInput->szNTLname))          // NTL requested
       {
         fOK = LPNCreate (pTAInput,
                         pLP,              // local data
                         pszTerm,      // term to create
                         pLPSeg
                         );
       } /* endif NTL requested */
     } /* endif !fFound */

     /*****************************************************************/
     /* Check for termination request                                 */
     /*****************************************************************/
     if (fOK)                            // check for term requests
     {
       BOOL fUserFlag = pTAInput->fKill;
       fOK = UtlSysCheck (hwnd,
                           NULL,               // no system flag
                           &fUserFlag,
                           ERROR_CANCELTA,
                           &TermCode);
       pTAInput->fKill = (EQF_BOOL)fUserFlag;
       if ( !fOK )
       {
           pTAInput->pInD->fTerminate = (EQF_BOOL)(TermCode != TERM_NONE);
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* continue with next term                                       */
     /*****************************************************************/
     pszTerm += UTF16strlenCHAR(pszTerm) + 1;
   } /* endwhile */

   /*----------------- general error mamagement ------------------*/
   if ( !fOK && !(pTAInput->pInD->fTerminate) )
   {
      if (UTF16strlenCHAR(pLP->szWorkSegment) > MAX_DISPL_SEG) // truncates segments
      {
         UTF16strcpy ((pLP->szWorkSegment + MAX_DISPL_SEG), ELLIPSISW);
      }

      pszTemp = pLP->szWorkSegment;

      usRC = UtlErrorW( ERROR_TA_LPPROC, // Cannot process data
                       MB_YESNO,        //   do you want to continue ?
                       1,
                       &pszTemp,
                       EQF_QUERY, TRUE);


      if (usRC != MBID_YES)     // user doesn't want to continue
          pTAInput->pInD->fTerminate = TRUE;
      }

   return (fOK);
   }


/*----------------------------------------------------------------------------*\
| LPWordCntCopy                                                                |
|                                                                              |
| purpose   :   count the word in the FTL                                      |
|               if not found get the data from dictionary services             |
|               and generate the entry in FTL                                  |
|               if minimum number of occurrences has not been specified        |
|                  and output dictionary is requested, copy the data           |
|                  to the output dict in the same moment.                      |
|               if a function returns an error, the variable pointed by        |
|               pfTerminate is set.                                            |
|                                                                              |
| Input     :                                                                  |
|              ptr to global data area for text analysis                       |
|              ptr to data area for list processing                            |
|              psz to the term   string                                        |
|              ptr to text segment reference structure                         |
|              length of dictionary data                                       |
|              term number as returned by ASD services                         |
|              match dict (from ASD)                                           |
|              ptr to termination variable                                     |
|                                                                              |
|                                                                              |
| Output    :  Side effects: the term will be counted in the FTL and           |
|              copied to out dict (if requested)                               |
|              returns: TRUE if successful, else FALSE                         |
|                                                                              |
\*----------------------------------------------------------------------------*/
static BOOL LPWordCntCopy (
                          PTAINPUT pTAInput,    // data area for Text Analysis
                          PLPDATA  pLP,         // data area for List process
                          PSZ_W    pszTerm,     // term to process
                          PLPSEG   pLPSeg,      // text segment
                          ULONG    ulDataLength, // length of dictionary data
                          ULONG    ulTermNumber, // term number in dict
                          HDCB      hMatchDict,  // dictionary of match
                          PBOOL    pfTerminate   // ptr to termination variable
                          )
   {
   BOOL    fOK;
   BOOL    fAlready;                    // the term is already in FTL
   USHORT  usFreq;                      // frequency of a term in tables
   USHORT  usRC;                        // return code from Asd
   UCHAR   uchFTL;
   PSZ     pszDictName;   // Temp dictionary name
   CHAR         chDictName[MAX_LONGFILESPEC]; // dict name


   uchFTL = *(pTAInput->szFTLname);

   //  try to hit term on "found" List
   fAlready = FALSE;
   fOK = LPFAdd (pTAInput,
                 pLP,
                 &fAlready,         // address of the found flag
                 pszTerm,           // term to search
                 pLPSeg,
                 &usFreq            // frequency of the term
                );

//    IF  it is the first time and out dict requested
//        or the num. of occurrences match the minimun (and is >1)
//        and output dictionary requested
//    THEN
//       get data from dictionary

   if (fOK &&
       (!fAlready ||
         (
         fAlready &&
         uchFTL &&
         pLP->hOutDict &&
         usFreq == pTAInput->usFTLNumOccurences
         )
       )
      )
      {
                                   //    reallocate memory area
      if (ulDataLength > pLP->ulTermBufSize)
         {
         fOK = UtlAlloc( (PVOID *)&(pLP->puchTermBuf),
                         pLP->ulTermBufSize,
                         ulDataLength * sizeof(CHAR_W),
                         ERROR_STORAGE);

         if(!fOK)
           *pfTerminate = TRUE;
         else
           pLP->ulTermBufSize = ulDataLength;
         }

      if (fOK)
         {                          // get term data from dictionary
 #if defined(MEASURETIME)
    GETTIME( ulOther  );
 #endif
         usRC = AsdRetEntryW (hMatchDict,
                             pLP->hUCB,
                             pszTerm,
                             &ulTermNumber,
                             pLP->puchTermBuf,
                             &ulDataLength,
                             &hMatchDict);

 #if defined(MEASURETIME)
    GETTIME( ulAsdRet );
    ulAsdRetNo++;
 #endif
         fOK = (usRC == LX_RC_OK_ASD);

// IF ok and out dict requested and first time (!fAlready)
//    THEN
//         Term is added to out Dict.

         if (fOK && pLP->hOutDict && !fAlready )
            {
 #if defined(MEASURETIME)
    GETTIME( ulOther  );
 #endif
            usRC = AsdMergeEntry (pLP->hUCB,           // user area
                                  hMatchDict,          // input dict
                                  pszTerm,             // headword
                                  ulDataLength,        // retrieved data l
                                  pLP->puchTermBuf,    // data
                                  pLP->hOutDict,       // target dict
                                  &(pLP->usMergeFlags) // merge flags
                                  );

 #if defined(MEASURETIME)
    GETTIME( ulAsdMerge );
 #endif
            fOK = (usRC == LX_RC_OK_ASD);
            }

         if (!fOK)
            {
             pszDictName = chDictName;         // set pointer
             AsdQueryDictName(hMatchDict, pszDictName);
             OEMTOANSI( pszDictName );
             UtlError(
                      ERROR_TA_ASDERR, // ASD returned a fatal error
                      MB_CANCEL,
                      1,
                      &pszDictName,
                      EQF_ERROR
                      );
            *pfTerminate = TRUE;
            }


         }

      if ( fOK && !fAlready )
      {
        usRC = AsdGetTranslationW( pLP->hUCB,           // user area
                                  hMatchDict,          // input dict
                                  pLP->puchTermBuf,
                                  (USHORT)(pLP->ulTermAreaSize / 2),  // buffer size in number of wide characters
                                  pLP->puchTermArea,
                                  SEMIKOLON_TERM_LIST );
        fOK = (usRC == LX_RC_OK_ASD);

        if ( !fOK )
        {
          pszDictName = chDictName;         // set pointer
          AsdQueryDictName(hMatchDict, pszDictName);
          OEMTOANSI( pszDictName );
          UtlError(
                   ERROR_TA_ASDERR, // ASD returned a fatal error
                   MB_CANCEL,
                   1,
                   &pszDictName,
                   EQF_ERROR
                   );
         *pfTerminate = TRUE;
        } /* endif */
      } /* endif */

      if (fOK && !fAlready)             // create new entry in FTL
         {
         fOK = LPFCreate (pTAInput,
                         pLP,                // local data
                         pszTerm,            // term to create
                         pLPSeg,             // segment definition
                         ( pLP->puchTermArea[0] ) ? pLP->puchTermArea : NULL,
                         hMatchDict // dict. handle of match
                         );
         }

      }  // end of copy data to output dictionary and add to FTL

   return fOK;
   }


/*----------------------------------------------------------------------------*\
| LPCompletion                                                                 |
| purpose   : Write the NTL and FTL Lists; call the functions to output the    |
|             NTL List and then the function to output FTL.                    |
|             If an error occurs during output, the corresponding list file    |
|             is deleted and a message is displayed; operation is interrupted. |
|                                                                              |
| Input     :  window handle                                                   |
|              pointer to the input parameters TAINPUTPARAM                    |
|               inside TAINPUTPARAM: File names of NTL, FTL                    |
|               If the file name is empty, the corresponding list is not       |
|               required.                                                      |
|              pointer to the LPDATA structure.                                |
|               inside LPDATA: handles to NTL & FTL structures                 |
|                                                                              |
| Output    :  Side effects: the Lists are written on disk.                    |
|              returns: TRUE if successful, else FALSE (output errors)         |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL LPCompletion (
                   HWND     hwnd,        // window handle
                   PTAINPUT pTAInput,    // ptr to Input parameter structure
                   PLPDATA  pLP          // ptr to Local data structure
                  )
   {
   BOOL       fOK = TRUE;                // general status indicator
   PSZ        pszTemp;                   // temporary

   if (*(pTAInput->szFTLname))  // FTL requested
      {
      pLP->ListType = FOUND_LIST;
      fOK = LPTLOutput (hwnd,
                        pLP,
                        pTAInput
                       );
      if (!fOK)                        // FTL output failed: delete list
         UtlDelete(pTAInput->szFTLname, 0L, FALSE);
      }

   if (fOK && *(pTAInput->szNTLname))  // NTL requested
      {
      pLP->ListType = NEW_LIST;
      fOK = LPTLOutput (hwnd,
                        pLP,
                        pTAInput
                       );
      if (!fOK)                        // NTL output failed: delete list
         UtlDelete(pTAInput->szNTLname, 0L, FALSE);
      }

   if (!fOK && !(pTAInput->pInD->fTerminate))
      {
      if (pLP->ListType == NEW_LIST)       // get current list file name
         pszTemp = pTAInput->szNTLname;
      else
         pszTemp = pTAInput->szFTLname;

      UtlError(
               ERROR_LIST_WRITING,       // List processing returned a
               MB_CANCEL,                //      fatal error;
               1,
               &pszTemp,
               EQF_ERROR
               );
      pTAInput->pInD->fTerminate = TRUE;
      }

   return (fOK);
   }

/*----------------------------------------------------------------------------*\
| LPExclInit                                                                   |
| purpose   : Load in memory the exclusion list                                |
|                                                                              |
| Input     : ptr to ptr to the exclusion list structure                       |
|                                                                              |
| Output    : side effects: the list is loaded; if load errors, set fTerminate |
|             returns TRUE if successful, else FALSE.                          |
|                                                                              |
\*----------------------------------------------------------------------------*/
static BOOL LPExclInit (
                       PTAINPUT      pTAInput,     // input parameter
                       PLPEXCLLIST    *ppExclList    // ptr to ptr to excl list area
                       )
   {
           BOOL            fOK;             // status variable
           ULONG           ulBytesRead;   // bytes read
           PLPEXCLLIST     pExclList = NULL;

           fOK = UtlLoadFileL( pTAInput->szExclusionList, // exclus. list name
                                                   (PVOID*) &pExclList,       // ptr to excl.list
                                                  &ulBytesRead,              // number of bytes read
                                                  FALSE,                     // don't continue
                                                                                                         //   in case of error
                                                  TRUE);                     // do error handling

           if (!fOK)
           {
                 pTAInput->pInD->fTerminate = TRUE;
           }
           else
           { // this is duplicate to LstReadNoiseExclList!
                   PBYTE p;
                  // check whether start is UNICODEFILEPREFIX!
                  ULONG ulPrefLen = strlen(UNICODEFILEPREFIX);
                  if ( memcmp( (PBYTE) pExclList, UNICODEFILEPREFIX, ulPrefLen) == 0)
                  {
                        // it is Unicode -- get rid of Unicode prefix
                        memmove( (PBYTE)pExclList, (PBYTE)pExclList + ulPrefLen, ulBytesRead - ulPrefLen );
                        ulBytesRead -= ulPrefLen;

                        p = (PBYTE) pExclList;
                        *(p + ulBytesRead ) = EOS;
                        *(p + ulBytesRead+1) = EOS;
                 } /* endif */

                 *ppExclList = pExclList;
           }

           return (fOK);
   }



/*----------------------------------------------------------------------------*\
| LPExclEnd                                                                    |
| purpose   : Free the memory associated with exclusion list to  terminate     |
|             operation on it                                                  |
|                                                                              |
| Input     : pointer to the exclusion list structure                          |
|                                                                              |
| Output    : the structure is freed                                           |
|             returns always TRUE.                                             |
|                                                                              |
\*----------------------------------------------------------------------------*/
static BOOL LPExclEnd
                    (
                    PLPEXCLLIST    *ppExclList      // pt to pt to excl list.
                    )
   {
   UtlAlloc( (PVOID *) ppExclList, 0L, 0L, NOMSG);        // free all memory
   return (TRUE);
   }


/*----------------------------------------------------------------------------*\
| LPExclSearch                                                                 |
| purpose   : Check if a term belongs to the exclusion list.                   |
|                                                                              |
|                                                                              |
| Input     : PSZ to term to be searched                                       |
|             pointer to the boolean variable fFound                           |
|             pointer to the exclusion list structure                          |
|                                                                              |
| Output    : Side effects: set fFound according to the result of the search   |
|             returns always TRUE.                                             |
|                                                                              |
\*----------------------------------------------------------------------------*/
static BOOL LPExclSearch
                   (
                   PSZ_W        pszTerm,       // term to be searched
                   BOOL         *pfFound,      // pt to bool found
                   PLPEXCLLIST  pExclList      // pointer to excl list.
                   )
   {
   LONG       lHigh;            // upper limit for search
   LONG       lLower;           //   lower limit for search
   LONG       i;                //      mid of search limits
   LONG       iResult;          // result of comparison
   BYTE       *pByte;           // pointer to exclusion list
   USHORT     *puNoiseOffsets;  // pointer to offset to noise words
   PSZ_W      pString;          // Pointer to first noise string
   static CHAR_W szUprTerm[MAX_NOISEWORD+1]; // buffer for uppercased term

   pByte = (BYTE *) pExclList;
   puNoiseOffsets = (USHORT *)(pByte + pExclList->uFirstEntry);
   pString = (CHAR_W *)(pByte + pExclList->uStrings);

   //check if word is longer than allowed length for exclusionword
   if (UTF16strlenCHAR(pszTerm) >= MAX_NOISEWORD)
   {
      lHigh = -1; // don't start search, word cannot be Noise
      lLower = 0;
   }
   else
   {
      lLower= 0;    /* initialize array boundaries */
      lHigh = pExclList->usNumEntries - 1;       // bound 0, n - 1
      UTF16strcpy( szUprTerm, pszTerm );
      UtlUpperW( szUprTerm );
   }

   /* binary search in Exclusion List */
   *pfFound = FALSE;                    // set found to false
   while (lLower <= lHigh )
   {
     i = lLower + (lHigh - lLower)/2;     /* calculate index */

     iResult= UTF16strcmp(puNoiseOffsets[i] + pString, szUprTerm );

     if (iResult == 0)
        {
          lHigh = -1; // search ended, as word is found
          *pfFound = TRUE;
        }
     else
       {

           if (iResult < 0)
              lLower = i + 1; /* search in upper part of table */
           else
              lHigh = i - 1;  /* search in lower part of table */
       } /* endif */
   } /* end while binary search */

   return (TRUE);
   }


/*----------------------------------------------------------------------------*\
| LPSegNorm                                                                    |
| purpose   : This function will use EqfTagTokenize to normalize a             |
|             segment. This is done in getting rid of all tags.                |
|             Inline Tags of length > 1 are changed into 1 Blank               |
| Input     :                                                                  |
|             PLPDATA   pLP            local data area                         |
|             TAGTABLE  *pTagTable     ptr to Tag table of the markup          |
|             PSZ       pszSegment     ptr to input data                       |
|             PSZ       pszWorkSegment ptr to output area                      |
|                                                                              |
| prerequisites: valid tag table, valid local data                             |
| concerns  : the function must clean the segment from every markup tag        |
|             for       markup that are special characters (o umlaut is an     |
|             example) the substitution with the real character will be        |
|             necessary in the future. A temporary simple fix is made for      |
|             Bookmaster. These tags are substituted by a single blank.        |
|                                                                              |
|                                                                              |
| Output    : the segment is copied to work segm; the output length            |
|             will be same or shorter.                                         |
|             returns TRUE if successful, else FALSE                           |
|                                                                              |
\*----------------------------------------------------------------------------*/
static BOOL   LPSegNorm
          (
          PLPDATA      pLP,             // local data
          PLOADEDTABLE pLoadedTable,    // pointer to TagTable
          PSZ_W        pszSegment,      // ptr to data
          PSZ_W        pszWorkSegment   // ptr to output area
          )
   {
   BOOL        fOK;
   PCHAR_W     pRest;                  // ptr to start of not-processed bytes
   USHORT      usColPos = 0;           // column pos used by EQFTagTokenize
   PTOKENENTRY pTok;                   // ptr for token table processing

   TATagTokenizeW( pszSegment,   // data to be processed
                  pLoadedTable,        // tag table
                  TRUE,             // end of data
                  &pRest,           // rest of data
                  &usColPos,        // current column position
                  pLP->TokenList,
                  MAX_NUM_TOKENS);
   fOK = (pRest != pszSegment);      // text has not been tokenized

   pTok = pLP->TokenList;
   while (pTok->sTokenid != ENDOFLIST && fOK)
   {
      if (pTok->sTokenid == TEXT_TOKEN )
      {
         memcpy(pszWorkSegment, pTok->pDataStringW, (pTok->usLength) * sizeof(CHAR_W));
         pszWorkSegment += pTok->usLength;
      }
      else
      {
         /*************************************************************/
         /* token is variable identification (used for subst. chars)  */
         /*************************************************************/
         if ( (*(pTok->pDataStringW) == '&') && (pTok->usLength > 1) )
         {
           memcpy(pszWorkSegment, pTok->pDataStringW, (pTok->usLength) * sizeof(CHAR_W));
           pszWorkSegment += pTok->usLength;
         }
         else if (pTok->usLength > 1)
         {
           *pszWorkSegment++ = ' ';
         } /* endif */
      }
      pTok++;
   } /* endwhile token loop */

   *pszWorkSegment = EOS;

   return (fOK);
} /* end of EQFBNormSeg */


/*----------------------------------------------------------------------------*\
| Name:       UtlSysCheck                                                      |
| Purpose:    allow service of messages in the message queue.                  |
|             check if, as a result of servicing the message queue             |
|             one or more conditions for termination have occurred             |
|             Such conditions are specified by the caller: they can be         |
|             1. a window handle: the routine check if the pointer associated  |
|                to the window is not NULL                                     |
|             2. a flag defined some where in the system that is set if        |
|                immediate termination is requested by the system              |
|             3. a flag defined some where in the system that is set if        |
|                termination is requested by the user. A message number        |
|                (along the standard of UtlError) can be optionally            |
|                specified; if it is, confirmation for termination             |
|                is asked to the user as a result of setting of the            |
|                user flag. The flag is then immediately reset.                |
|                                                                              |
|                                                                              |
| Parameters: 1. current window handle           (zero if not available)       |
|             2. system termination flag address (NULL if not available)       |
|             3. user termination flag address   (NULL if not available)       |
|             5. user confirmation message number(NULL if not available)       |
|             4. termination indicator adress    (must be specified)           |
|                                                                              |
| returns:    FALSE if one or more of the specified terminations reasons is    |
|             true, else TRUE. If confirmation message is specified,           |
|             user termination request is confirmed by this dialog             |
|                                                                              |
| note:       the termination flags (both user and system) must be true        |
|             if termination is requested                                      |
|                                                                              |
| Example:                                                                     |
|             the routine is designed to be used in long processing loops      |
|             to avoid to monopolise the system and allow message processing   |
|             in other windows. A processing loop must call this routine       |
|             as often as possible; as a result the user will be able to       |
|             carry on other task. The processing loop can specify the termi-  |
|             nation condition to check.   These conditions will be checked    |
|             and the routine will return FALSE if some of them is active.     |
|             The termination indicator will tell if the termination has been  |
|             requested by the system or by the user. In the latter case       |
|             the processing loop could decide to save some intermediate       |
|             result.                                                          |
|                                                                              |
|                                                                              |
|             #define ERROR_CONFIRM nn (message number for UtlError)           |
|                                                                              |
|             enum TERM_REASON usTermCode;                                     |
|             BOOL             fOK;                                            |
|             BOOL             fTerminate;                                     |
|             HWND             hwnd;                                           |
|                                                                              |
|                                                                              |
|             while (fOK)                                                      |
|                {                                                             |
|                // here application processing                                |
|                                                                              |
|                fOK = UtlSysCheck (hwnd,                                      |
|                                   NULL,               // no system flag      |
|                                   &(fKill),           // user flag           |
|                                   ERROR_CONFIRM,      // confirmation        |
|                                   &usTermCode);                              |
|                if (!fOK)                                                     |  |
|                   {                                                          |  |
|                   fTerminate = (usTermCode != TERM_NONE);                    |  |
|                   }                                                          |  |
|                }                                                             |  |
|                                                                              |
| called modules, dependencies: UtlDispatch, UtlError, WinQueryWindowPtr       |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL UtlSysCheck       (
                       HWND    hwnd,       // instance window handle
                       PBOOL   pfSysFlag,  // system termination flag
                       PBOOL   pfUserFlag, // user termination flag
                       USHORT  usMsgNum,   // user confirmaton message
                       enum TERM_REASON *pTermination // termination code address
                       )

   {

   usMsgNum;

   UtlDispatch();                               // process window messages

   *pTermination = TERM_NONE;

   if (hwnd != NULLHANDLE)                      // instance window check
      {
      if ( !WinIsWindow( NULLHANDLE, hwnd ) )
         *pTermination = TERM_SYSTEM;
      }


   if (pfSysFlag)                               // system request check
      {
      if (*pfSysFlag)
         *pTermination = TERM_SYSTEM;
      }

                                                                /* 8@KIT1156C */
   if (pfUserFlag)                              // user request check
      {
      if (*pfUserFlag)
         {
         *pfUserFlag = FALSE;                   // reset user flag
         *pTermination = TERM_USER;
        }
      }

   return (*pTermination == TERM_NONE);
}

