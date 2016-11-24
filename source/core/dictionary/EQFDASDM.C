//+----------------------------------------------------------------------------+
//|  EQFDASDM.C - Merge dictionary entry function of the Dictionary Services   |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//|  D  AsdMergeEntry       - Merge a dictionary entry into another dictionary |
//|                                                                           |
//|  +-- status ('H'= Header, 'D'=Design, 'C'=Code, 'T'=Test, ' '=complete,    |
//|              'Q'=Quick-and-dirty )                                         |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//|     UtlAlloc                - General memory allocation routine            |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//|                                                                           |
//|  +-- status ('H'= Header, 'D'=Design, 'C'=Code, 'T'=Test, ' '=complete,    |
//|              'Q'=Quick-and-dirty )                                         |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|                                                                            |
//|  - source to target field table creation:                                  |
//|      use user name for user defined fields                                 |
//+----------------------------------------------------------------------------+
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_LDB              // dictionary data encoding functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFQDAMI.H"             // fastdam header file
#include <time.h>                 // C time functions

#include "EQFDASDI.H"             // internal ASD services header file
#include "EQFDDLG.ID"             // IDs for dialog controls
#include "OtmDictionaryIF.H"

#define MLE_BUFFER_SIZE   0xFF00  // size of MLE text buffer
#define NO_TARGET         -1      // identifier for 'no target field'

/**********************************************************************/
/* Merge dialog IDA                                                   */
/**********************************************************************/
typedef struct _MERGEIDA
{
   PSZ_W     pucTerm;                  // headword of new term
   PUSHORT   pusFlags;                 // pointer to control flags variable
   PUCB      pUCB;                     // ptr to user control block
   PDCB      pDCBSource;               // ptr to source DCB
   PDCB      pDCBTarget;               // ptr to target DCB
   PVOID     hLdbSourceTree;           // QLDB tree handle
   PVOID     hLdbTargetTree;           // QLDB tree handle
   PVOID     hLdbNewTree;              // QLDB tree handle
   USHORT    usSourceMLECharWidth;     // character width of source MLE
   USHORT    usTargetMLECharWidth;     // character width of target MLE
   PSZ_W     pucMLEBuffer;             // ptr to buffer for MLE data
   USHORT    usMLEBufSize;             // current size of MLE buffer
   CHAR      szOrgText[80];            // original message text
   CHAR      szNewText[80];            // message text with name inserted
   CHAR      szDictName[MAX_LONGFILESPEC]; //  buffer for dictionary names
} MERGEIDA, *PMERGEIDA;

/**********************************************************************/
/* Macro to convert LDB return code to NLP return code                */
/**********************************************************************/
#define LDB2NLPRC( rc ) \
    (USHORT)( ( rc == QLDB_NO_ERROR ) ? LX_RC_OK_ASD : \
     ( rc == QLDB_NO_MEMORY ) ? LX_INSUF_STOR_ASD : LX_UNEXPECTED_ASD); \

/**********************************************************************/
/* pseudo error code to skip processing in AsdMergeEntry if source    */
/* and target entry are identical                                     */
/**********************************************************************/
#define LX_NOPROCESSING_REQUIRED     LX_MAX_ERR + 2000

INT_PTR CALLBACK AsdMergeEntryDlgProc( HWND, WINMSG, WPARAM, LPARAM );

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdMergeEntry          Merge two dictionary entries      |
//+----------------------------------------------------------------------------+
//|Function call:     AsdMergEntry( HUCB hUCB, HDCB hDCBSource, PUCHAR pucTerm,|
//|                                 ULONG ulSourceLen, PUCHAR pucSourceData,   |
//|                                 HDCB hDCBTarget, PUSHORT pusControlFlags); |
//+----------------------------------------------------------------------------+
//|Description:       Merge a dictionary entry into another dictionary. If the |
//|                   dictionary entry exists in the target dictionary, the new|
//|                   entry is merged into the existing entry, the new entry   |
//|                   is ignored or the existing entry is replaced with the    |
//|                   new one depending on the control flag setting and        |
//|                   the user selection.                                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   HUCB     hUCB              handle of user control block  |
//|                   HDCB     hDCBSource        handle of source dictionary   |
//|                   PUCHAR   pucTerm           headword of new term          |
//|                   ULONG    ulSourceLen       length of new term data       |
//|                   PUCHAR   pucSourceData     pointer to new term data      |
//|                   HDCB     hDCBTarget        handle of target dictionary   |
//|                   PUSHORT  pusControlFlags   a combination of ctrl flags:  |
//|                                                                            |
//|                   type of merge flags, these flags are mutually exclusive  |
//|                   MERGE_REPLACE    replace existing entries                |
//|                   MERGE_NOREPLACE  do not replace existing entries         |
//|                   MERGE_ADD        add data of new entry to data of        |
//|                                    existing entry                          |
//|                                                                            |
//|                   MERGE_NOUSERPROMPT do not ask user for type of merge     |
//|                                      if this flag is specified, on of the  |
//|                                      merge type flags has to be specified  |
//|                                      too                                   |
//|                   MERGE_USERPROMPT   ask user for type of merge            |
//|                                                                            |
//|                   source of merge flags, these flags are mutually exclusive|
//|                   these flags control the heading text of the source MLE   |
//|                   MERGE_SOURCE_ASD   source is an ASD dictionary           |
//|                   MERGE_SOURCE_SGML  source is a SGML dictionary           |
//|                   MERGE_SOURCE_EDIT  source is the edit dictionary dialog  |
//|                                                                            |
//|                   MERGE_NOPROMT_CHECKBOX if set the checkbox with the      |
//|                                "Use the selected option for all entries"   |
//|                                option is hidden                            |
//|                                                                            |
//|                   the default is:                                          |
//|                     MERGE_SOURCE_ASD | MERGE_ADD | MERGE_NOUSERPROMPT      |
//|                                                                            |
//|                   the control flags are changed on return to reflect the   |
//|                   user's type of merge selection. Also the flag            |
//|                   MERGE_NOUSERPROMPT may be set, if the user has choosen   |
//|                   to merge without being prompted anymore.                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       LX_RC_OK_ASD        =  OK                                |
//|                   LX_MAX_ERR          =  merge dialog has been canceled    |
//|                                          by the user                       |
//|                   LX_IDX_FN_LOAD      =  merge dialog could not be loaded  |
//|                   LX_BAD_DICT_TKN_ASD =  dictionary handle is invalid      |
//|                   LX_UNINIT_PRM_ASD   =  invalid combination of flags or   |
//|                                          input parameters missing (pucTerm,|
//|                                          ulDataLength or pucData is NULL or|
//|                                          empty)                            |
//|                   LX_UNEXPECTED_ASD   =  error in LDB processing; source or|
//|                                          target entry may be corrupted     |
//|                   LX_ASC_NT_ALLWD_ASD =  source or target dictionary is an |
//|                                          associated dictionary             |
//|                   LX_...              =  other Nlp errors                  |
//+----------------------------------------------------------------------------+
//|Prerequesits:      HUCB must have been created using AsdBegin.              |
//|                   hDCBSource must have been created using AsdOpen.         |
//|                   hDCBTarget must have been created using AsdOpen.         |
//+----------------------------------------------------------------------------+
//|Side effects:      The target dictionary is updated.                        |
//+----------------------------------------------------------------------------+
//|Samples:           usFlags = MERGE_ADD | MERGE_USERPROMPT;                  |
//|                   usRC = AsdMergeEntry( hUCB, hDCBSource, ucTermBuf,       |
//|                                         ulDataLen, pucData, hDCBTarget,    |
//|                                         &usFlags );                        |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate merge IDA                                       |
//|                   check supplied parameters and set default values         |
//|                   get entry from target dictionary                         |
//|                   check if source and target entry are identical           |
//|                   convert target entry to LDB tree                         |
//|                   convert source entry to LDB tree                         |
//|                   if user prompt requested and                             |
//|                      term exists in target dictionary then                 |
//|                     call merge entry dialog                                |
//|                   endif                                                    |
//|                   allocate and fill source field to target field table     |
//|                   add all target templates to new LDB tree if target exists|
//|                     and MERGE_ADD is set                                   |
//|                   add all source templates to new LDB tree if no target    |
//|                     exists or MERGE_NOREPLACE is not set                   |
//|                   if no target exists or MERGE_NOREPLACE is not set then   |
//|                     join nodes containing the same data                    |
//|                     insert entry or update existing entry                  |
//|                   endif                                                    |
//|                   cleanup                                                  |
//+----------------------------------------------------------------------------+

USHORT AsdMergeEntry
(
   HUCB     hUCB,                      // handle of user control block
   HDCB     hDCBSource,                // handle of source dictionary
   PSZ_W    pucTerm,                   // headword of new term
   ULONG    ulSourceLen,               // len of new term data( in char_w's)
   PSZ_W    pucSourceData,             // pointer to new term data
   HDCB     hDCBTarget,                // handle of target dictionary
   PUSHORT  pusFlags                   // pointer to control flags variable
)
{
   USHORT    usNlpRC = LX_RC_OK_ASD;   // return code of Nlp call(s)
   PSZ_W       pucNewRecord = NULL;    // pointer to new record
   ULONG       ulNewRecLen = 0;        // length of new record
   USHORT      usLdbRC = 0;            // LDB return code
   USHORT      usLevel;                // level returned by QLDB functions
   ULONG     ulTermNumber;             // term number in target dictionary
   ULONG     ulTargetLen = 0;          // term data length
   PPROFENTRY pSourceField,            // pointer to source fields
              pTargetField;            // pointer to target fields
   USHORT    usI, usJ;                 // general use indices
   USHORT    usDictHandle;             // buffer for dictionary handles
   PSHORT    psSource2Target = NULL;   // source field to target field table
   BOOL      fTarget = FALSE;          // TRUE = entry is in target dictionary
   BOOL      fSameDict = TRUE;         // TRUE = source and target dict are same
   PMERGEIDA pIda;                     // ptr to merge IDA
   PSZ_W     pucTargetData = NULL;     // pointer to data of target entry
   BOOL      fFirstTime = TRUE;        // TRUE = no data added to node tree yet
   PDCB      pDCBSrc = NULL;           // ptr to DCB of source dictionary
   PDCB      pDCBTrg = NULL;           // ptr to DCB of target dictionary
   CHAR_W     szDateW[20];             // buffer for current date string
   LONG       lTime;                   // date/time as long value

   /*******************************************************************/
   /* Allocate IDA                                                    */
   /*******************************************************************/
   UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(MERGEIDA), NOMSG );
   if ( !pIda )
   {
     usNlpRC = LX_MEM_ALLOC_ASD;
   } /* endif */

   /*******************************************************************/
   /* Check passed user and dictionary handles                        */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) )
   {
     pIda->pucTerm = pucTerm;
     pIda->pusFlags = pusFlags;

     pIda->pUCB = (PUCB)hUCB;            // convert handles to pointer
     pDCBSrc = pIda->pDCBSource = (PDCB)hDCBSource;
     pDCBTrg = pIda->pDCBTarget = (PDCB)hDCBTarget;
     CHECKUCB( pIda->pUCB, usNlpRC );    // check user and dictionary handles
     if ( ASDOK(usNlpRC) )
     {
        CHECKDCB( pDCBSrc, usNlpRC );
     } /* endif */
     if ( ASDOK(usNlpRC) )
     {
        CHECKDCB( pDCBTrg, usNlpRC );
     } /* endif */
     if ( ASDOK(usNlpRC) )
     {
        if ( pDCBSrc->fAssoc || pDCBTrg->fAssoc )
        {
          usNlpRC = LX_ASC_NT_ALLWD_ASD;
        } /* endif */
     } /* endif */
     fSameDict = ( pDCBSrc == pDCBTrg );
   } /* endif */

   /*******************************************************************/
   /* Check input flags                                               */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) )
   {
     if (((*pusFlags & MERGE_REPLACE)      && (*pusFlags & MERGE_NOREPLACE)) ||
         ((*pusFlags & MERGE_REPLACE)      && (*pusFlags & MERGE_ADD)) ||
         ((*pusFlags & MERGE_NOREPLACE)    && (*pusFlags & MERGE_ADD)) ||
         ((*pusFlags & MERGE_NOUSERPROMPT) && (*pusFlags & MERGE_USERPROMPT)) ||
         ((*pusFlags & MERGE_SOURCE_ASD)   && (*pusFlags & MERGE_SOURCE_SGML))||
         ((*pusFlags & MERGE_SOURCE_ASD)   && (*pusFlags & MERGE_SOURCE_EDIT))||
         ((*pusFlags & MERGE_SOURCE_SGML)  && (*pusFlags & MERGE_SOURCE_EDIT)))
     {
       usNlpRC = LX_UNINIT_PRM_ASD;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Check input data                                                */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) )
   {
     if ( (pucTerm == NULL) || (*pucTerm == EOS) ||
          (ulSourceLen == 0L) ||
          (pucSourceData == NULL) || (*pucSourceData == EOS) )
     {
       usNlpRC = LX_UNINIT_PRM_ASD;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Set default values                                              */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) )
   {
     if ( !(*pusFlags & (MERGE_NOUSERPROMPT | MERGE_USERPROMPT)) )
     {
        *pusFlags |= MERGE_NOUSERPROMPT;
     } /* endif */
     if ( !(*pusFlags & (MERGE_REPLACE | MERGE_NOREPLACE | MERGE_ADD)) )
     {
        *pusFlags |= MERGE_ADD;
     } /* endif */
     if ( !(*pusFlags &
            (MERGE_SOURCE_ASD | MERGE_SOURCE_EDIT | MERGE_SOURCE_SGML)) )
     {
        *pusFlags |= MERGE_SOURCE_ASD;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Check if term exists in target dictionary                       */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) )
   {
      NlpFndMatchAsdW( pucTerm,                  // desired term
                      pDCBTrg->usDictHandle,    // dictionary handle
                      pIda->pUCB->usUser,       // user handle
                      (PBYTE)pIda->pUCB->ucTermBuf,    // term found
                      &ulTermNumber,            // term number
                      &ulTargetLen,             // entry data length in bytes
                      &usDictHandle,            // dictionary of match
                      &usNlpRC,                 // return code
                      pIda->pUCB->usDictSearchSubType );

     if ( usNlpRC == LX_WRD_NT_FND_ASD )
     {
       usNlpRC = LX_RC_OK_ASD;
     }
     else if ( usNlpRC == LX_RC_OK_ASD )
     {
       /***************************************************************/
       /* the following string compare is required as AsdFndMatch     */
       /* has become case-insensitive                                 */
       /***************************************************************/
       if ( UTF16strcmp( pucTerm, pIda->pUCB->ucTermBuf ) == 0 )
       {
         fTarget = TRUE;
       } /* endif */
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* get entry in target dictionary (if any)                         */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) && fTarget )
   {
     UtlAlloc( (PVOID *)&pucTargetData, 0L, ulTargetLen , NOMSG );
     if ( pucTargetData )
     {
        NlpRetEntryAsdW( usDictHandle,           // dictionary handle
                        pIda->pUCB->usUser,     // ASD user handle
                        pIda->pUCB->ucTermBuf,  // term for this entry
                        &ulTermNumber,          // number of term
                        (PBYTE)pucTargetData,          // entry data
                        &ulTargetLen,           // data len in # of bytes
                        &usDictHandle,          // dictionary of term
                        &usNlpRC );
     }
     else
     {
        usNlpRC = LX_MEM_ALLOC_ASD;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Check if source and target entry are identical and              */
   /* dictionary profiles are the same                                */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) && fTarget )
   {
     if ( (ulTargetLen == ulSourceLen) &&
          (memcmp( pDCBSrc->ausNoOfFields, pDCBTrg->ausNoOfFields,
                   sizeof(pDCBSrc->ausNoOfFields) ) == 0) &&
          (memcmp( (PBYTE)pucSourceData, (PBYTE)pucTargetData,
                (USHORT)(ulSourceLen * sizeof(CHAR_W)))  == 0) )
     {
       /***************************************************************/
       /* nothing to do as the source entry already exists in the     */
       /* target dictionary                                           */
       /***************************************************************/
       usNlpRC = LX_NOPROCESSING_REQUIRED;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* convert target entry to LDB tree                                */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) && fTarget )
   {
     usLdbRC = QLDBRecordToTree( pDCBTrg->ausNoOfFields,
                                 (PSZ_W)pucTargetData,
                                 (ulTargetLen / sizeof(CHAR_W)),
                                 &pIda->hLdbTargetTree );
     usNlpRC = LDB2NLPRC( usLdbRC );
   } /* endif */

   /*******************************************************************/
   /* convert source entry to LDB tree                                */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) )
   {
     usLdbRC = QLDBRecordToTree( pDCBSrc->ausNoOfFields,
                                 pucSourceData,
                                 ulSourceLen,    // # of w's
                                 &pIda->hLdbSourceTree );
     usNlpRC = LDB2NLPRC( usLdbRC );
   } /* endif */


   /*******************************************************************/
   /* get user response when user prompt has been requested and a     */
   /* user decision is required (i.e. a target entry exists)          */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) &&
        fTarget &&
        (*pusFlags & MERGE_USERPROMPT)  )
   {
     /*****************************************************************/
     /* Allocate MLE buffer                                           */
     /*****************************************************************/
     UtlAlloc( (PVOID *)&pIda->pucMLEBuffer, 0L,
               (LONG) MLE_BUFFER_SIZE * sizeof(CHAR_W),
               ERROR_STORAGE );
     if ( !pIda->pucMLEBuffer )
     {
       usNlpRC = LX_MEM_ALLOC_ASD;
     } /* endif */

     if ( ASDOK(usNlpRC) )
     {
	   INT_PTR iRC = 0;
	   HMODULE hResMod;
	   hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
       pIda->usMLEBufSize = MLE_BUFFER_SIZE;
       DIALOGBOX( (HWND)UtlQueryULong( QL_TWBCLIENT ), AsdMergeEntryDlgProc,
                  hResMod, ID_ASDMERGE_DLG, pIda, iRC );
       switch ( iRC )
       {
         case DID_ERROR :
           /*************************************************************/
           /* WinDlgBox call failed                                     */
           /*************************************************************/
           usNlpRC = LX_IDX_FN_LOAD;
           break;

         case  TRUE:
           /*************************************************************/
           /* user pressed merge button                                 */
           /*************************************************************/
           usNlpRC = LX_RC_OK_ASD;
           break;

         default :
           /*************************************************************/
           /* dialog has been canceled                                  */
           /*************************************************************/
           usNlpRC = LX_MAX_ERR;
           break;
       } /* endswitch */

       /***************************************************************/
       /* free MLE buffer                                             */
       /***************************************************************/
       UtlAlloc( (PVOID *)&pIda->pucMLEBuffer, 0L, 0L, NOMSG );

     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Allocate source to target field table if different dictionaries */
   /* are used                                                        */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) && !fSameDict )
   {
     UtlAlloc( (PVOID *)&psSource2Target, 0L,
               (LONG)max( MIN_ALLOC,
                          pDCBSrc->Prop.usLength * sizeof(SHORT)),
               NOMSG );
     if ( !psSource2Target )
     {
       usNlpRC = LX_MEM_ALLOC_ASD;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Fill source field to target field table if different            */
   /* dictionaries are used                                           */
   /*    The index into this table will be the number of the source   */
   /*    field. The table contains the number of the corresponding    */
   /*    target field or NO_TARGET  if no target field exists.        */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) && !fSameDict )
   {
     pSourceField = pDCBSrc->Prop.ProfEntry;
     for ( usI = 0; usI < pDCBSrc->Prop.usLength; usI++ )
     {
       psSource2Target[usI] = NO_TARGET;
       usJ = 0;
       pTargetField = pDCBTrg->Prop.ProfEntry;
       while ( (usJ < pDCBTrg->Prop.usLength) &&
               (psSource2Target[usI] == NO_TARGET) )
       {
         if ( strcmp( pSourceField->chSystName,
                      pTargetField->chSystName ) == 0)
         {
           psSource2Target[usI] = usJ;
         } /* endif */
         usJ++;
         pTargetField++;
       } /* endwhile */
       pSourceField++;
     } /* endfor */
   } /* endif */

   /*******************************************************************/
   /* Add target data to output node tree if a target entry exists    */
   /* and MERGE_ADD is set                                            */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) && fTarget && (*pusFlags & MERGE_ADD) )
   {
     usLdbRC = QLDBResetTreePositions( pIda->hLdbTargetTree );

     if ( usLdbRC == QLDB_NO_ERROR )
     {
       usLdbRC = QLDBCurrTemplate( pIda->hLdbTargetTree,
                                   pDCBTrg->apszFields );
     } /* endif */

     usLevel = 0;
     while ( (usLdbRC == QLDB_NO_ERROR) && (usLevel != QLDB_END_OF_TREE) )
     {
       /***************************************************************/
       /* Add template to new LDB tree                                */
       /***************************************************************/
       if ( fFirstTime )
       {
         /*************************************************************/
         /* set term update date field                                */
         /*************************************************************/
         if ( pDCBTrg->UpdateField.fInDict )
         {
           UtlTime( &lTime );
           UtlLongToDateStringW( lTime, szDateW, sizeof(szDateW)/sizeof(CHAR_W) );
           pDCBTrg->apszFields[pDCBTrg->UpdateField.usField] = szDateW;
         } /* endif */

         /*************************************************************/
         /* create target tree                                        */
         /*************************************************************/
         usLdbRC = QLDBCreateTree( pDCBTrg->ausNoOfFields,
                                   pDCBTrg->apszFields,
                                   &pIda->hLdbNewTree );
         fFirstTime = FALSE;
       }
       else
       {
         usLdbRC = QLDBAddSubtree( pIda->hLdbNewTree, 2,
                                   pDCBTrg->apszFields +
                                      pDCBTrg->ausFirstField[1] );
       } /* endif */

       /***************************************************************/
       /* Get next template                                           */
       /***************************************************************/
       if ( usLdbRC == QLDB_NO_ERROR )
       {
         usLdbRC = QLDBNextTemplate( pIda->hLdbTargetTree,
                                     pDCBTrg->apszFields,
                                     &usLevel );
       } /* endif */
     } /* endwile */

     usNlpRC = LDB2NLPRC( usNlpRC );
   } /* endif */

   /*******************************************************************/
   /* Add source data to output node tree if no target exists or      */
   /* MERGE_NOREPLACE is not set                                      */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) && (!fTarget || !(*pusFlags & MERGE_NOREPLACE)) )
   {
     usLdbRC = QLDBResetTreePositions( pIda->hLdbSourceTree );

     if ( usLdbRC == QLDB_NO_ERROR )
     {
       usLdbRC = QLDBCurrTemplate( pIda->hLdbSourceTree,
                                   pDCBSrc->apszFields );
     } /* endif */

     usLevel = 0;
     while ( (usLdbRC == QLDB_NO_ERROR) && (usLevel != QLDB_END_OF_TREE) )
     {
       if ( !fSameDict )
       {
         /***************************************************************/
         /* Clear target field array                                    */
         /***************************************************************/
         for ( usI = 0; usI < pDCBTrg->Prop.usLength; usI++ )
         {
           pDCBTrg->apszFields[usI] = EMPTY_STRINGW;
         } /* endfor */

         /***************************************************************/
         /* Convert template to target fields                           */
         /***************************************************************/
         for ( usI = 0; usI < pDCBSrc->Prop.usLength; usI++ )
         {
           if ( psSource2Target[usI] != NO_TARGET  )
           {
             pDCBTrg->apszFields[psSource2Target[usI]] =
                pDCBSrc->apszFields[usI];
           } /* endif */
         } /* endfor */
       } /* endif */

       /***************************************************************/
       /* Add template to new LDB tree                                */
       /***************************************************************/
       if ( fFirstTime )
       {
         usLdbRC = QLDBCreateTree( pDCBTrg->ausNoOfFields,
                                   pDCBTrg->apszFields,
                                   &pIda->hLdbNewTree );
         fFirstTime = FALSE;
       }
       else
       {
         usLdbRC = QLDBAddSubtree( pIda->hLdbNewTree, 2,
                                   pDCBTrg->apszFields +
                                     pDCBTrg->ausFirstField[1] );
       } /* endif */

       /***************************************************************/
       /* Get next template                                           */
       /***************************************************************/
       if ( usLdbRC == QLDB_NO_ERROR )
       {
         usLdbRC = QLDBNextTemplate( pIda->hLdbSourceTree,
                                     pDCBSrc->apszFields,
                                     &usLevel );
       } /* endif */
     } /* endwile */

     usNlpRC = LDB2NLPRC( usNlpRC );

   } /* endif */

   /*******************************************************************/
   /* Write output node tree if no target exists or MERGE_NOREPLACE   */
   /* is not set                                                      */
   /*******************************************************************/
   if ( ASDOK(usNlpRC) && (!fTarget || !(*pusFlags & MERGE_NOREPLACE)) )
   {
      /****************************************************************/
      /* Combine nodes containing the same data (the function         */
      /* QLDBJoinSameNodes is not required anymore as the function    */
      /* is also performed inside of QLDBTreeToRecord)                */
      /****************************************************************/

      /****************************************************************/
      /* Convert the LDB tree to a flat record                        */
      /****************************************************************/
      if ( usLdbRC == QLDB_NO_ERROR )
      {
        usLdbRC = QLDBTreeToRecord( pIda->hLdbNewTree,
                                    &pucNewRecord, &ulNewRecLen );
      } /* endif */

      if ( usLdbRC == QLDB_NO_ERROR )
      {
        if ( fTarget )
        {
           usNlpRC = AsdRepEntry( hUCB,
                                  hDCBTarget,
                                  pucTerm,
                                  pucNewRecord,
                                  ulNewRecLen,   // numb of charw's
                                  &ulTermNumber );
        }
        else
        {
           usNlpRC = AsdInsEntry( hUCB,
                                  hDCBTarget,
                                  pucTerm,
                                  pucNewRecord,
                                  ulNewRecLen,   //numb of charw's
                                  &ulTermNumber );
        } /* endif */
      }
      else
      {
        usNlpRC = LDB2NLPRC( usNlpRC );
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* cleanup                                                         */
   /*******************************************************************/
   if ( pIda )
   {
     if ( pIda->hLdbTargetTree ) QLDBDestroyTree( &pIda->hLdbTargetTree );
     if ( pIda->hLdbSourceTree ) QLDBDestroyTree( &pIda->hLdbSourceTree );
     if ( pIda->hLdbNewTree )    QLDBDestroyTree( &pIda->hLdbNewTree );
     UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
   } /* endif */
   if ( psSource2Target ) UtlAlloc( (PVOID *)&psSource2Target, 0L, 0L, NOMSG );
   if ( pucNewRecord )    UtlAlloc( (PVOID *)&pucNewRecord, 0L, 0L, NOMSG );
   if ( pucTargetData )   UtlAlloc( (PVOID *)&pucTargetData, 0L, 0L, NOMSG );

   /*******************************************************************/
   /* Remove 'now processing required' pseudo error code              */
   /*******************************************************************/
   if ( usNlpRC == LX_NOPROCESSING_REQUIRED )
   {
     usNlpRC = LX_RC_OK_ASD;
   } /* endif */

   if ( usNlpRC != LX_RC_OK_ASD )
   {
     return ( usNlpRC );               // just to have a breakpoint address ...
   } /* endif */

   return ( usNlpRC );
} /* end of AsdMergeEntry */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AsdMergeEntryDlgProc                                     |
//+----------------------------------------------------------------------------+
//|Function call:     AsdMergeEntryDlgProc ( HWND hwnd, USHORT msg, MPARAM mp1,|
//|                                          MPARAM mp2 )                      |
//+----------------------------------------------------------------------------+
//|Description:      User prompt of the AsdMergeEntry function.                |
//+----------------------------------------------------------------------------+
//|Input parameter:  HWND    hwnd     handle of window                         |
//|                  USHORT  msg      type of message                          |
//|                  MPARAM  mp1      first message parameter                  |
//|                  MPARAM  mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  MRESULT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:      depends on message type                                   |
//|                  normal return codes are:                                  |
//|                  TRUE  = message has been processed                        |
//|                  FALSE = message has not been processed                    |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg;                                              |
//|                     WM_INITDLG:                                            |
//|                       get and anchor IDA                                   |
//|                       fill term SLE                                        |
//|                       set heading of source MLE                            |
//|                       add dictionary name to target MLE heading            |
//|                       fill source and target entry MLE                     |
//|                       set selected merge type radio button                 |
//|                     WM_COMMAND:                                            |
//|                       address IDA                                          |
//|                       switch command value;                                |
//|                         case 'Merge' PB:                                   |
//|                           update control flags                             |
//|                           leave dialog                                     |
//|                         case DID_CANCEL:                                   |
//|                         case 'Cancel' pushbutton                           |
//|                           leave dialog                                     |
//|                       endswitch;                                           |
//|                   endswitch;                                               |
//+----------------------------------------------------------------------------+
INT_PTR CALLBACK AsdMergeEntryDlgProc
(
  HWND      hwnd,                      // dialog window handle
  WINMSG    msg,                       // message identifier
  WPARAM    mp1,                       // first message parameter
  LPARAM    mp2                        // second message parameter
)
{
  PMERGEIDA   pIda;                    // pointer to dialog IDA
  MRESULT     mResult = FALSE;         // result of message processing
  HWND        hwndMLE;                 // handle of MLE control
  PSZ         pszDict;                 // ptr to dictionary name
  ULONG       ulLen;                   // buffer for message length

  switch ( msg )
  {
    case WM_INITDLG :
	{
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      /****************************************************************/
      /* Get and anchor IDA                                           */
      /****************************************************************/
      pIda = (PMERGEIDA)mp2;
      ANCHORDLGIDA( hwnd, pIda );
      /****************************************************************/
      /* Fill headword field                                          */
      /****************************************************************/
      UTF16strcpy( pIda->pucMLEBuffer, pIda->pucTerm );
      //OEMTOANSI( pIda->pucMLEBuffer );
      SETTEXTW( hwnd, ID_ASDMERGE_ENTRY_EF, pIda->pucMLEBuffer );

      SetCtrlFnt(hwnd, GetCharSet(), ID_ASDMERGE_ENTRY_EF, 0);
      SetCtrlFnt(hwnd, GetCharSet(), ID_ASDMERGE_NEW_MLE, 0);
      SetCtrlFnt(hwnd, GetCharSet(), ID_ASDMERGE_OLD_MLE, 0);

      /****************************************************************/
      /* Set heading of source entry MLE depend on flag settings      */
      /****************************************************************/
      pIda->szNewText[0] = EOS;
      if ( *pIda->pusFlags & MERGE_SOURCE_EDIT )
      {
        /**************************************************************/
        /* Get heading for EDIT source                                */
        /**************************************************************/
        LOADSTRING( (HAB)UtlQueryULong( QL_HAB ), hResMod, SID_MERGE_EDIT_HEADING,
                    pIda->szNewText );
      }
      else if ( *pIda->pusFlags & MERGE_SOURCE_SGML )
      {
        /**************************************************************/
        /* Get heading for SGML source                                */
        /**************************************************************/
        LOADSTRING( (HAB)UtlQueryULong( QL_HAB ), hResMod,
                    SID_MERGE_SGML_HEADING, pIda->szNewText );
      }
      else
      {
        /****************************************************************/
        /* Insert source dictionary into heading for ASD source         */
        /****************************************************************/
        LOADSTRING( (HAB)UtlQueryULong( QL_HAB ), hResMod,
                    SID_MERGE_ASD_HEADING, pIda->szOrgText );
        AsdQueryDictName( (HDCB)pIda->pDCBSource, pIda->szDictName );

        OEMTOANSI( pIda->szDictName );

        pszDict = pIda->szDictName;
        DosInsMessage( &pszDict, (SHORT)1, pIda->szOrgText, strlen(pIda->szOrgText),
                       pIda->szNewText, (sizeof(pIda->szNewText) - 1),
                       &ulLen );

        pIda->szNewText[ulLen] = EOS;
      } /* endif */
      SETTEXT( hwnd, ID_ASDMERGE_NEW_TEXT, pIda->szNewText );

      /****************************************************************/
      /* Insert target dictionary into target MLE heading             */
      /****************************************************************/
      QUERYTEXT( hwnd, ID_ASDMERGE_OLD_TEXT, pIda->szOrgText );
      AsdQueryDictName( (HDCB)pIda->pDCBTarget, pIda->szDictName );
      pszDict = pIda->szDictName;
      OEMTOANSI( pIda->szDictName );
      DosInsMessage( &pszDict, 1, pIda->szOrgText, strlen(pIda->szOrgText),
                     pIda->szNewText, (sizeof(pIda->szNewText) - 1),
                     &ulLen );
      pIda->szNewText[ulLen] = EOS;
      SETTEXT( hwnd, ID_ASDMERGE_OLD_TEXT, pIda->szNewText );

      /****************************************************************/
      /* Fill source MLE                                              */
      /****************************************************************/
      hwndMLE = WinWindowFromID( hwnd, ID_ASDMERGE_NEW_MLE );
      SetFixedPitchMLEFont( hwndMLE, pIda->pUCB->fDBCS| GetCharSet(),
                            &pIda->usSourceMLECharWidth );

      FillMLE( hwndMLE,                          // MLE handle
               pIda->usSourceMLECharWidth,       // MLE character width
               pIda->pucMLEBuffer,               // buffer for fill
               pIda->usMLEBufSize,               // size of buffer
               (HDCB)pIda->pDCBSource,           // dictionary handle
               pIda->hLdbSourceTree,             // LDB tree handle
               3);                                // display level
      //         pIda->pUCB->fisDBCS1);             // DBCS flag array

      /****************************************************************/
      /* Fill target MLE                                              */
      /****************************************************************/
      hwndMLE = WinWindowFromID( hwnd, ID_ASDMERGE_OLD_MLE );

      SetFixedPitchMLEFont( hwndMLE, pIda->pUCB->fDBCS| GetCharSet(),
                                                      &pIda->usTargetMLECharWidth );
      FillMLE( hwndMLE,                          // MLE handle
               pIda->usTargetMLECharWidth,       // MLE character width
               pIda->pucMLEBuffer,               // buffer for fill
               pIda->usMLEBufSize,               // size of buffer
               (HDCB)pIda->pDCBTarget,           // dictionary handle
               pIda->hLdbTargetTree,             // LDB tree handle
               3);                                // display level
              // pIda->pUCB->fisDBCS1 );             // DBCS flag array

      /****************************************************************/
      /* Set radiobuttons according current flags                     */
      /****************************************************************/
      if ( *pIda->pusFlags & MERGE_ADD )
      {
        CLICK( hwnd, ID_ASDMERGE_ADD_RB );
      }
      else if ( *pIda->pusFlags & MERGE_REPLACE )
      {
        CLICK( hwnd, ID_ASDMERGE_REPLACE_RB );
      }
      else
      {
        CLICK( hwnd, ID_ASDMERGE_IGNORE_RB );
      } /* endif */

      /****************************************************************/
      /* Set visibility state of confirm checkbox                     */
      /****************************************************************/
      if ( *pIda->pusFlags & MERGE_NOPROMT_CHECKBOX )
      {
        HIDECONTROL( hwnd, ID_ASDMERGE_NOPROMPT_CHK );
      } /* endif */
	  }
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case ID_ASDMERGE_HELP_PB :
          UtlInvokeHelp();
          break;

        case ID_ASDMERGE_MERGE_PB :
          /************************************************************/
          /* access IDA                                               */
          /************************************************************/
          pIda = ACCESSDLGIDA( hwnd, PMERGEIDA );

          /************************************************************/
          /* Set user's merge type options                            */
          /************************************************************/
          *pIda->pusFlags &= ~( MERGE_NOUSERPROMPT |            /* 1@KIT1165C */
                                MERGE_USERPROMPT   |            /* 4@KIT1165A */
                                MERGE_REPLACE      |
                                MERGE_NOREPLACE    |
                                MERGE_ADD );

          if ( QUERYCHECK( hwnd, ID_ASDMERGE_NOPROMPT_CHK ) )
          {
            *pIda->pusFlags |= MERGE_NOUSERPROMPT;
          }
          else
          {
            *pIda->pusFlags |= MERGE_USERPROMPT;
          } /* endif */

          if ( QUERYCHECK( hwnd, ID_ASDMERGE_REPLACE_RB ) )
          {
            *pIda->pusFlags |= MERGE_REPLACE;
          }
          else if ( QUERYCHECK( hwnd, ID_ASDMERGE_IGNORE_RB ) )
          {
            *pIda->pusFlags |= MERGE_NOREPLACE;
          }
          else
          {
            *pIda->pusFlags |= MERGE_ADD;
          } /* endif */
          WinPostMsg( hwnd, WM_CLOSE, MP1FROMSHORT(TRUE), 0L );
          break;

        case ID_ASDMERGE_CANCEL_PB :
        case DID_CANCEL:
          WinPostMsg( hwnd, WM_CLOSE, MP1FROMSHORT(FALSE), 0L );
          break;

      case ID_ASDMERGE_ENTRY_EF:
      case ID_ASDMERGE_NEW_MLE:
      case ID_ASDMERGE_OLD_MLE:
         if ( WMCOMMANDCMD( mp1, mp2 ) == EN_KILLFOCUS )
         {
           ClearIME( hwnd );
         } /* endif */
         break;
      } /* endswitch */
      break;

    case WM_CLOSE :
      DelCtrlFont(hwnd, ID_ASDMERGE_ENTRY_EF );
      DelCtrlFont(hwnd, ID_ASDMERGE_NEW_MLE );
      DelCtrlFont(hwnd, ID_ASDMERGE_OLD_MLE );
      WinDismissDlg( hwnd, SHORT1FROMMP1(mp1) );
      break;

    default :
      mResult = WinDefDlgProc( hwnd, msg, mp1, mp2 );
      break;
  } /* endswitch */
  return( mResult );

} /* end of function AsdMergeEntryDlgProc */
