// Copyright (c) 2012-2016, International Business Machines
// Corporation and others.  All rights reserved.
//

#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#include <eqf.h>                  // General EQF include file
#include <eqfevent.h>             // event logging definitions

//#include <eqfmem.h>
#include "EQFQDAMI.H"             // private fastdam header file

__declspec(dllexport)
void DAMLINK
QDamOpenAsd( PUCHAR,                /* in  - name of dictionary  */
             PUCHAR,                /* server name                    */
             USHORT,                /* in  - number of pages     */
             USHORT,                /* in  - immediate writing?  */
             USHORT,                /* in  - user handle         */
             PUSHORT,               /* out - dictionary handles  */
             PUSHORT,               /* out - length of user area */
             PUSHORT);              /* out - return code         */

USHORT DamBTreeRc ( SHORT sRc );
static BOOL FindDict ( PSZ, PUSHORT );

/**********************************************************************/
/* typedefs and function for list of dictionary processing            */
/**********************************************************************/
typedef struct _DICTHANDLETERM
{
  USHORT  usHandle;                    // handle of dictionary
  PSZ_W   pTerm;                       // pointer to term
} DICTHANDLETERM, * PDICTHANDLETERM;

static USHORT NlpForwardList( PDICTHANDLETERM, PSZ_W, USHORT, PSZ_W *,
                              PUSHORT );
static USHORT NlpBackwardList( PDICTHANDLETERM, PSZ_W, USHORT, PSZ_W *,
                               PUSHORT, BOOL);
static USHORT GetNextTerm ( PDICTHANDLETERM, PSZ_W );
static USHORT GetNextWildTerm ( PDICTHANDLETERM, PSZ_W, BOOL, PSZ_W );
static USHORT GetPrevTerm ( PDICTHANDLETERM, PSZ_W );

#define  DUMMY_STR              "*"
DAM2QDAM DamRec[MAX_NUM_DICTS];

extern QDAMDICT QDAMDict[];
static BOOL fDamInit = FALSE;

/**********************************************************************/
/* add pragmas to force load into different text segments             */
/**********************************************************************/

///////////////////////////////////////////////////////////////////////////////
///////   Layer for NlpCalls Substitution
///////////////////////////////////////////////////////////////////////////////

__declspec(dllexport)
void DAMLINK
NlpBegAsd
(
  USHORT usFiles,
  USHORT usIndex,
  USHORT usAssocs,
  USHORT usFPA,
  PUSHORT pusUser,
  PUSHORT  pusRc
)
{
   usFiles; usIndex; usAssocs; usFPA;
    *pusUser  = 1;
    *pusRc   = LX_RC_OK_ASD;


   if ( !fDamInit )
   {
     memset(DamRec, 0, sizeof(DamRec));
     fDamInit = TRUE;
   } /* endif */

#if defined(MEASURE)
   DosGetInfoSeg( &selGlobalSeg, &selLocalSeg );
   pGlobInfoSeg = MAKEPGINFOSEG(selGlobalSeg);
#endif
#if defined(MEASURE)
    usASI = 0;
    usASD = 0;
    ulTerseEnd = 0;
    ulBuffer = 0;
    ulSeek = 0;
    ulUnTerseEnd = 0;
    ulUpdate = 0;
    ulFind = 0;
    ulComp = 0;
    ulBegComp = 0;
    ulBegRead = 0;
    ulBegRealRead = 0;
    ulBegString = 0;
    ulString = 0;
    ulRealReadASD = 0;
    ulRealReadASI = 0;
    ulRead = 0;
    ulGet = 0;
    ulAdd = 0;
    usEntry = 0;
    usEntryLength = 0;
#endif
}

void DAMLINK
NlpDictUpdTime
(
  USHORT  usHandle,
  PLONG   plUpdTime,
  PUSHORT pusRc
)
{
   BOOL   fAssoc;
   LONG   lUpdTime;

   *pusRc     = LX_RC_OK_ASD;
   *plUpdTime = 0L;

   // is it a associate handle ????
   if ( ! DamRec[usHandle].pDamBTree )
   {
      usHandle = DamRec[usHandle].usNextHandle ;
      fAssoc = TRUE;
   }
   else
   {
     fAssoc = FALSE;
   } /* endif */

   while ( (*pusRc == LX_RC_OK_ASD ) && usHandle )
   {
      *pusRc = DamBTreeRc( QDAMDictUpdTimeLocal( DamRec[usHandle].pDamBTree,
                                            &lUpdTime ) );
      if ( *pusRc == LX_RC_OK_ASD )
      {
        if ( *plUpdTime < lUpdTime )
        {
          *plUpdTime = lUpdTime;
        } /* endif */
      } /* endif */

      if ( (*pusRc == LX_RC_OK_ASD) &&
           DamRec[usHandle].usNextHandle &&
           fAssoc )
      {
         usHandle = DamRec[usHandle].usNextHandle;
      }
      else
      {
        usHandle = 0;
      } /* endif */
   } /* endwhile */
}


__declspec(dllexport)
void DAMLINK
NlpEndAsd
(
   USHORT user,
   PUSHORT pusRc
)
{
   user;
    *pusRc = LX_RC_OK_ASD;

}

__declspec(dllexport)
void DAMLINK
QDamOpenAsd
(
  PUCHAR  pName,
  PUCHAR  pServer,
  USHORT  usPages,
  USHORT  usOpenFlags,
  USHORT  usUser,
  PUSHORT pusHandle,
  PUSHORT pusLenArea,
  PUSHORT pusRc
)
{
   PBTREE pBTIda;
   USHORT  usI = 1;
// SHORT  sRc;
   BOOL   fFound = FALSE;

   usUser; pusLenArea;

   if ( usPages < 20 || usPages > 50 )
   {
      usPages = 20;
   } /* endif */

#if defined(MEASURE)
  ulStart  = pGlobInfoSeg->msecs;
#endif
   /*******************************************************************/
   /* check if dict is already open - if so use this handle           */
   /*******************************************************************/
   fFound = FindDict ( (PSZ)pName, &usI );

   if ( !fFound  )
   {
     *pusRc = DamBTreeRc ( QDAMDictOpen( (PSZ)pName, (PSZ)pServer, usPages,
                                           usOpenFlags, &pBTIda ) );

     if ( pBTIda &&
          (( *pusRc == LX_RC_OK_ASD) || (*pusRc == LX_RENUM_RQD_ASD)) )
     {
        // fill pointer into structure
        DamRec[usI].pDamBTree = pBTIda;
        DamRec[usI].usOpenCount = 1;
        DamRec[usI].usOpenRC    = *pusRc;
        strcpy( DamRec[usI].chDictName, (PSZ)pName );
        *pusHandle = usI;
     } /* endif */
   }
   else
   {
     /*****************************************************************/
     /* dictionary already open....                                   */
     /*****************************************************************/
     if ( usOpenFlags & ASD_LOCKED )
     {
       /***************************************************************/
       /* return dictionary locked...                                 */
       /***************************************************************/
       *pusRc = DamBTreeRc ( BTREE_DICT_LOCKED );
     }
     else
     {
       *pusRc = DamRec[usI].usOpenRC;
       *pusHandle = usI;
       DamRec[usI].usOpenCount ++;
     } /* endif */
   } /* endif */

  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( QDAMOPENASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}

void DAMLINK
NlpBuildAsd
(
    PUCHAR  pName,
    USHORT usType,
    USHORT usPages,
    USHORT fGuard,
    USHORT uslg,
    USHORT usCp,
    USHORT usfm,
    PUCHAR pc,
    PUCHAR server,
    PUCHAR ed,
    PUCHAR enc,
    PUCHAR pTermEncode,
    PUSHORT pusLen,
    PUCHAR pUserData,
    PUCHAR idx,
    USHORT usUser,
    PUSHORT pusHandle,
    PUSHORT pusRc
)
{
   PBTREE pBTIda;
   USHORT usI = 1;

   usType; fGuard; uslg; usCp; usfm; pc; ed; enc; idx; usUser;

   /// if feature mask not set we are creating an index
   // for the moment no encoding
   if ( ! usfm )
   {
      pTermEncode = NULL;
      usPages = 20;
   }
   else
   {
      usPages = 20;
   } /* endif */
#if defined(MEASURE)
  ulStart  = pGlobInfoSeg->msecs;
#endif
   /*******************************************************************/
   /* limit length of user data ...                                   */
   /*******************************************************************/
   if ( *pusLen > 1000 )
   {
     *pusLen = 1000;
   } /* endif */

   *pusRc = DamBTreeRc ( QDAMDictCreate( (PSZ)pName, (PSZ)server, usPages, (PSZ)pUserData,
                                         *pusLen,
                                         (PCHAR)pTermEncode,
                                         NULL,             // Collating sequence
                                         NULL,             // case map
                                         &pBTIda  ) );
    if ( pBTIda )
    {
       /***************************************************************/
       /* find free slot ....                                         */
       /***************************************************************/
       FindDict ( (PSZ)pName, &usI );
//     // fill pointer into structure
//     while ( DamRec[usI].pDamBTree || DamRec[usI].usNextHandle )
//     {
//        usI++;
//     } /* endwhile */
       DamRec[usI].pDamBTree = pBTIda;
       DamRec[usI].usOpenCount = 1;
       DamRec[usI].usOpenRC    = *pusRc;
       strcpy( DamRec[usI].chDictName, (PSZ)pName );
       *pusHandle = usI;
       /***************************************************************/
       /* run the DictSignature record again to get correct settings  */
       /* in case of piping problems                                  */
       /***************************************************************/
       if ( *pusRc == LX_RC_OK_ASD )
       {
         *pusLen = 0;
         *pusRc = DamBTreeRc ( QDAMDictSign( DamRec[usI].pDamBTree,
                                             NULL, pusLen));
       } /* endif */
    } /* endif */
}

void DAMLINK
NlpCloseAsd
(
   USHORT usHandle,
   USHORT usUser,
   PUSHORT pusRc
)
{
   usUser;

   /*******************************************************************/
   /* if more than once open decrease count, else close dict physical.*/
   /*******************************************************************/
   if ( DamRec[usHandle].usOpenCount > 1 )
   {
     DamRec[usHandle].usOpenCount --;
     *pusRc = LX_RC_OK_ASD;
   }
   else
   {
     /*****************************************************************/
     /* is it an associated handle                                    */
     /*****************************************************************/
     DamRec[usHandle].usNextHandle = 0;
     DamRec[usHandle].usOpenCount = 0;
     DamRec[usHandle].chDictName[0] = '\0';
     if ( DamRec[usHandle].pDamBTree )
     {
       *pusRc = DamBTreeRc ( QDAMDictClose( &(DamRec[usHandle].pDamBTree) ));
     }
     else
     {
       *pusRc = LX_RC_OK_ASD;
     } /* endif */
   } /* endif */
#if defined(MEASURE)
  if ( ulStart )
  {
     ulEnd  = pGlobInfoSeg->msecs - ulStart;
     ulStart = 0L;
  } /* endif */
#endif
}


void DAMLINK
NlpAssocAsd
(
   PUSHORT pausHandles,
   USHORT  usUser,
   PUSHORT pusAssoc,
   PUSHORT pusRc
)
{
   USHORT  usI = 1;

   usUser;

   // find next free handle
   while ( DamRec[usI].pDamBTree || DamRec[usI].usNextHandle )
   {
      usI++;
   } /* endwhile */

   strcpy(DamRec[usI].chDictName, DUMMY_STR);

   *pusAssoc = usI;

   // concatenate the handles to build the association
   DamRec[usI].usNextHandle = *pausHandles;
   while ( *(++pausHandles) )
   {
      DamRec[*(pausHandles-1)].usNextHandle = *pausHandles;
   } /* endwhile */

   *pusRc = LX_RC_OK_ASD;
   return;
}

void DAMLINK
NlpRetSignature
(
   USHORT usHandle,
   USHORT usUser,
   PUSHORT t,
   PUSHORT fm,
   PUSHORT pusCodePage,
   PUCHAR  pc,
   PUCHAR  sc,
   PUCHAR  ed,
   PUCHAR  te,
   PUCHAR  ee,
   PUCHAR  n,
   PUSHORT pusLen,
   PUCHAR  pUserData,
   PUSHORT pusRc
)
{
   usUser; t; fm; pc; sc; ed; te; ee; n;

   *pusCodePage = 850;      // set the default code page
   *pusRc = DamBTreeRc ( QDAMDictSign( DamRec[usHandle].pDamBTree,
                                       (PCHAR)pUserData, pusLen) );
}

void DAMLINK
NlpUpdSignature
(
   USHORT usHandle,
   USHORT usUser,
   USHORT pm,
   PUCHAR pName,
   USHORT usLen,
   PUCHAR pUserData,
   PUSHORT pusRc
)
{
   pName; usUser; pm;
   *pusRc = DamBTreeRc( QDAMDictUpdSignLocal( DamRec[usHandle].pDamBTree,
                                         (PCHAR)pUserData,usLen));
}

/**********************************************************************/
/*  renumber currently invoked for reorg purposes only                */
/*  reset corruption flag in such a case                              */
/**********************************************************************/

void DAMLINK
NlpRenumberAsd
(
   USHORT usHandle,
   USHORT usUser,
   PUSHORT pusRc
)
{
  PBTREE pBTIda;
  usUser;

  pBTIda = DamRec[usHandle].pDamBTree;
  pBTIda->pBTree->fCorrupted = FALSE;       // reset corruption flag
   *pusRc = LX_RC_OK_ASD;
}




#define NlpCompTermsAsd( term1,term2,type,handle,user,result,rc ) \
      { *rc = LX_RC_OK_ASD }

//
void DAMLINK
NlpFndBeginAsdW
(
  PSZ_W   pSubStr,
  USHORT  usHandle,
  USHORT  usUser,
  PSZ_W   pTerm,
  PULONG  pulNumber,
  PULONG  pulLen,
  PUSHORT pusDict,
  PUSHORT pusRc
)
{
   ULONG  ulLen;
   ULONG  ulUserLen = 0;
   BOOL   fFound = FALSE;
   BOOL   fAssoc = FALSE;

   USHORT usActHandle = 0;             // handle of matching dict
   USHORT usActLen;                    // active length
   USHORT usActRc = 0;                 // return code
   CHAR_W   chTerm[ HEADTERM_SIZE ];     // temp buffer for headterm

   usUser; pulNumber;


   ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
#if defined(MEASURE)
  ulBegin  = pGlobInfoSeg->msecs;
#endif
   // is it a associate handle ????
   if ( ! DamRec[usHandle].pDamBTree )
   {
      usHandle = DamRec[usHandle].usNextHandle ;
      fAssoc = TRUE;
   } /* endif */

   /*******************************************************************/
   /* loop thru all dictionaries and position at the earliest match   */
   /* Note: All of the associated dictionaries are positionned        */
   /*******************************************************************/
   chTerm[0] = EOS;
   while ( !fFound && DamRec[ usHandle ].pDamBTree )
   {
      *pusRc = DamBTreeRc( QDAMDictSubStr( DamRec[usHandle].pDamBTree,
                                           pSubStr, (PBYTE)pTerm,
                                           &ulLen, NULL, &ulUserLen ) );
      if ( *pusRc == LX_RC_OK_ASD )
      {
        /**************************************************************/
        /* compare it with previous match                             */
        /**************************************************************/
        if ( usActHandle )
        {
          if ( QDAMKeyCompare( DamRec[usHandle].pDamBTree, chTerm, pTerm ) > 0 )
//          if ( strcmp(chTerm, pTerm) > 0  )
          {
            usActHandle = usHandle;
            UTF16strcpy( chTerm, pTerm );
            usActLen = (USHORT)ulUserLen;
            usActRc  = *pusRc;
          } /* endif */
        }
        else
        {
          usActHandle = usHandle;
          UTF16strcpy( chTerm, pTerm );
          usActLen = (USHORT)ulUserLen;
          usActRc  = *pusRc;
        } /* endif */
      } /* endif */

      if ( fAssoc && DamRec[usHandle].usNextHandle )
      {
        usHandle = DamRec[usHandle].usNextHandle;
      }
      else
      {
        fFound = TRUE;
      } /* endif */
   } /* endwhile */
#if defined(MEASURE)
  ulFind += (pGlobInfoSeg->msecs - ulBegin);
#endif

   *pulLen = ulUserLen / sizeof(CHAR_W);
   *pusDict = usHandle;
  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPFNDBEGINASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}
//
void DAMLINK
NlpFndEquivAsdW
(
  PSZ_W   pSubStr,
  USHORT  usHandle,
  USHORT  usUser,
  PSZ_W   pTerm,
  PULONG  pulNumber,
  PULONG  pulLen,            // in # of CHAR_W'S
  PUSHORT pusDict,
  PUSHORT pusRc
)
{
   ULONG  ulLen;
   ULONG  ulUserLen = 0;
   BOOL   fFound = FALSE;
   BOOL   fAssoc = FALSE;

   usUser; pulNumber;

   DEBUGEVENT2( NLPFNDEQUIVASD_LOC, FUNCENTRY_EVENT, 0, DICT_GROUP, NULL );

   ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);

#if defined(MEASURE)
  ulBegin  = pGlobInfoSeg->msecs;
#endif
   // is it a associate handle ????
   if ( ! DamRec[usHandle].pDamBTree )
   {
      usHandle = DamRec[usHandle].usNextHandle ;
      fAssoc = TRUE;
   } /* endif */

   while ( !fFound )
   {
      //QDAMDictEquiv: all len in # of bytes
      *pusRc = DamBTreeRc( QDAMDictEquiv( DamRec[usHandle].pDamBTree, pSubStr,
                                          (PBYTE)pTerm, &ulLen, NULL, &ulUserLen ) );
      if ( (*pusRc == LX_WRD_NT_FND_ASD) &&
           DamRec[usHandle].usNextHandle &&
           fAssoc )
      {
         usHandle = DamRec[usHandle].usNextHandle;
      }
      else
      {
         fFound = TRUE;
      } /* endif */
   } /* endwhile */

#if defined(MEASURE)
  ulFind += (pGlobInfoSeg->msecs - ulBegin);
#endif

   *pusDict = usHandle;
   *pulLen =  ulUserLen / sizeof(CHAR_W);
  if ( *pusRc && (*pusRc != LX_WRD_NT_FND_ASD))
  {
    ERREVENT( NLPFNDEQUIVASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
  DEBUGEVENT2( NLPFNDEQUIVASD_LOC, FUNCEXIT_EVENT, *pusRc, DICT_GROUP, NULL );
}

//
void DAMLINK
NlpFndMatchAsdW
(
  PSZ_W   pTerm,
  USHORT  usHandle,
  USHORT  usUser,
  PBYTE   pTermMatch,
  PULONG  pulNumber,
  PULONG  pulLen,           // # of bytes
  PUSHORT pusDict,
  PUSHORT pusRc,
   USHORT usSearchSubType   // special hyphenation lookup flag
)
{
  ULONG   ulLen;
  BOOL    fFound = FALSE;                       // not yet found
   BOOL   fAssoc = FALSE;

  usUser; pulNumber;

    ulLen = 0;

   DEBUGEVENT2( NLPFNDMATCHASD_LOC, FUNCENTRY_EVENT, 0, DICT_GROUP, NULL );

#if defined(MEASURE)
  ulBegin  = pGlobInfoSeg->msecs;
#endif
   // is it a associate handle ????
   if ( ! DamRec[usHandle].pDamBTree )
   {
      usHandle = DamRec[usHandle].usNextHandle ;
      fAssoc = TRUE;
   } /* endif */

   while ( !fFound )
   {
      *pusRc = DamBTreeRc( QDAMDictExact( DamRec[usHandle].pDamBTree,
                                          pTerm, NULL, &ulLen, usSearchSubType ) );
      if ( (*pusRc == LX_WRD_NT_FND_ASD) &&
           DamRec[usHandle].usNextHandle &&
           fAssoc )
      {
         usHandle = DamRec[usHandle].usNextHandle;
      }
      else
      {
         fFound = TRUE;
      } /* endif */
   } /* endwhile */
#if defined(MEASURE)
  ulFind += (pGlobInfoSeg->msecs - ulBegin);
#endif

  UTF16strcpy( (PSZ_W)pTermMatch, pTerm );
  *pusDict = usHandle;
  *pulLen = ulLen;
  if ( *pusRc && (*pusRc != LX_WRD_NT_FND_ASD))
  {
    ERREVENT( NLPFNDMATCHASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
  DEBUGEVENT2( NLPFNDMATCHASD_LOC, FUNCEXIT_EVENT, *pusRc, DICT_GROUP, NULL );
}

void DAMLINK
NlpRetEntryAsdW
(
   USHORT usHandle,
   USHORT usUser,
   PSZ_W  pTerm,
   PULONG pulNum,
   PBYTE  pData,
   PULONG pulDataLen,                  // no of bytes
   PUSHORT pusDict,
   PUSHORT pusRc
)
{
   ULONG ulLen;
   ULONG ulKeyLen = HEADTERM_SIZE * sizeof(CHAR_W);

   usUser; pulNum;

   *pusDict = usHandle;
   ulLen = *pulDataLen;

#if defined(MEASURE)
  ulBegin  = pGlobInfoSeg->msecs;
#endif
// QDAM calls: length of PBYTE fields in number of bytes!!
  *pusRc = DamBTreeRc( QDAMDictCurrent( DamRec[usHandle].pDamBTree, (PBYTE)pTerm,
                                        &ulKeyLen, (PBYTE)pData, &ulLen) );
#if defined(MEASURE)
  ulGet += (pGlobInfoSeg->msecs - ulBegin);
#endif

  *pulDataLen = ulLen;                        // return # of bytes in pData
  if ( *pusRc != LX_RC_OK_ASD)
  {
    ERREVENT( NLPRETENTRYASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}
//


void DAMLINK
NlpFndNumberAsdW
(
   ULONG ulTermNum,
   USHORT usHandle,
   USHORT fRel,
   USHORT usUser,
   PSZ_W  pTerm,
   PULONG pulLen,            // # of CHAR_W'S
   PUSHORT pusDict,
   PUSHORT pusRc
)
{
   ULONG ulLen;
   ULONG ulKeyLen = HEADTERM_SIZE * sizeof(CHAR_W);

   usUser; fRel;

   *pusDict = usHandle;
   ulLen = 0;
#if defined(MEASURE)
  ulBegin  = pGlobInfoSeg->msecs;
#endif
  *pusRc = DamBTreeRc( QDAMDictNumber( DamRec[usHandle].pDamBTree, ulTermNum, pTerm,
                                       &ulKeyLen, NULL, &ulLen) );
  // get the number of entries if not yet done
  if ( DamRec[usHandle].ulNum == 0)
  {
      QDAMDictNumEntriesLocal ( DamRec[usHandle].pDamBTree, &DamRec[usHandle].ulNum );
  } /* endif */
#if defined(MEASURE)
  ulFind += (pGlobInfoSeg->msecs - ulBegin);
#endif
  *pulLen = ulLen / sizeof(CHAR_W);
  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPFNDNUMBERASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}
//
void DAMLINK
NlpNxtTermAsdW
(
   USHORT usHandle,
   USHORT usUser,
   PSZ_W  pMatch,
   PULONG pulNum,
   PULONG pulLen,                  // # of CHAR_W's
   PUSHORT pusDict,
   PUSHORT pusRc
)
{
   ULONG  ulLen = 0;
   ULONG  ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
   CHAR_W szMatch[ HEADTERM_SIZE ];
   ULONG  ulLen2;                      // length of second data param.
   USHORT usTmpHandle;                 // 2.nd handle in case of assoc
   USHORT usRc;
   BOOL   fAssoc = FALSE;
   SHORT  sCompare;                    // compare value


   usUser; pulNum;
   *pusDict = usHandle;


#if defined(MEASURE)
  ulBegin = pGlobInfoSeg->msecs;
#endif
   // is it a associate handle ????
   if ( ! DamRec[usHandle].pDamBTree )
   {
      usHandle = DamRec[usHandle].usNextHandle ;
      fAssoc = TRUE;
   } /* endif */

   /*******************************************************************/
   /* skip corrupted entries -- this is only useful in case we are    */
   /* dealing with organizing the dictionary                          */
   /* This functionality has to be considered only for stand-alone    */
   /* dictionaries - association are not interesting during an        */
   /* organize...                                                     */
   /*******************************************************************/
   *pusRc = LX_RENUM_RQD_ASD;
   while ( *pusRc == LX_RENUM_RQD_ASD )
   {
     /******************************************************************/
     /* disable corruption flag to allow get of data in case dictionary*/
     /* is corrupted                                                   */
     /******************************************************************/
     PBTREE pBT = DamRec[usHandle].pDamBTree;
     BOOL   fCorrupted = pBT->pBTree->fCorrupted;
     pBT->pBTree->fCorrupted = FALSE;       // reset corruption flag
     *pMatch = '\0';
     ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
     *pusRc = DamBTreeRc( QDAMDictNext( DamRec[usHandle].pDamBTree, pMatch,
                                        &ulLen1, NULL,  &ulLen) );
     pBT->pBTree->fCorrupted = fCorrupted;
   } /* endwhile */

   if ( (*pusRc == LX_RC_OK_ASD) || ( *pusRc == LX_EOF_ASD ) )
   {
     /*******************************************************************/
     /* still associations left to be looked after ....                 */
     /*******************************************************************/
     usTmpHandle = DamRec[usHandle].usNextHandle;

     while ( fAssoc && usTmpHandle )
     {
       ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
       usRc = DamBTreeRc( QDAMDictNext(DamRec[usTmpHandle].pDamBTree, szMatch,
                                          &ulLen1, NULL,  &ulLen2) );

       /*****************************************************************/
       /* if term found compare it with term of other dictionary,       */
       /*  use the alphabetical first one and reset the other one       */
       /* IF they are the same, do NO reset                           */
       /*****************************************************************/
       if ( usRc == LX_RC_OK_ASD )
       {
         /*************************************************************/
         /* set sCompare depending on real value or EOF condition     */
         /*************************************************************/
         if ( *pusRc != LX_EOF_ASD  )
         {
           sCompare =(SHORT) UTF16stricmp( pMatch, szMatch );
         }
         else
         {
           sCompare = 1;
         } /* endif */

         if ( sCompare > 0 )
         {
           UTF16strcpy( pMatch, szMatch );
           ulLen = ulLen2;
           *pusDict = usTmpHandle;
           /************************************************************/
           /* first one reached end                                    */
           /************************************************************/
           if ( *pusRc == LX_EOF_ASD )
           {
             *pusRc = usRc;
           } /* endif */
           /**************************************************************/
           /* reset the not used part (don't care about errors - we      */
           /* will get them on the next try... )                         */
           /**************************************************************/
           ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
           QDAMDictPrev( DamRec[usHandle].pDamBTree,
                         szMatch, &ulLen1, NULL,  &ulLen2) ;
         }
         else if ( sCompare < 0 )
         {
           /**************************************************************/
           /* reset the not used part (don't care about errors - we      */
           /* will get them on the next try... )                         */
           /**************************************************************/
           ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
           QDAMDictPrev( DamRec[usTmpHandle].pDamBTree,
                         szMatch, &ulLen1, NULL,  &ulLen2) ;
         } /* endif */

       }
       else if ( usRc == LX_EOF_ASD )
       {
         ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
         QDAMDictPrev(DamRec[usTmpHandle].pDamBTree,
                      szMatch, &ulLen1, NULL,  &ulLen2 );
         /***************************************************************/
         /* reset our primary dictionary if it is at EOF, too           */
         /***************************************************************/
         if ( *pusRc == LX_EOF_ASD )
         {
           ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
           QDAMDictPrev(DamRec[usHandle].pDamBTree,
                        szMatch, &ulLen1, NULL,  &ulLen2);
         } /* endif */
       } /* endif */
       usTmpHandle = DamRec[usTmpHandle].usNextHandle;
     } /* endwhile */
   } /* endif */
#if defined(MEASURE)
  ulFind += (pGlobInfoSeg->msecs - ulBegin);
#endif
   *pulLen = ulLen / sizeof(CHAR_W);

  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPNXTTERMASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}

//
void DAMLINK
NlpPrvTermAsdW
(
   USHORT usHandle,
   USHORT usUser,
   PSZ_W  pMatch,
   PULONG pulNum,
   PULONG pulLen,                         // # of CHAR_W's
   PUSHORT pusDict,
   PUSHORT pusRc
)
{
   ULONG  ulLen;
   ULONG  ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
   CHAR_W   szMatch[ HEADTERM_SIZE ];
   ULONG  ulLen2;                      // length of second data param.
   USHORT usTmpHandle;                 // 2.nd handle in case of assoc
   USHORT usRc;
   BOOL   fAssoc = FALSE;
   SHORT  sCompare;                    // compare value


   usUser; pulNum;
   *pusDict = usHandle;

#if defined(MEASURE)
  ulBegin = pGlobInfoSeg->msecs;
#endif
   // is it a associate handle ????
   if ( ! DamRec[usHandle].pDamBTree )
   {
      usHandle = DamRec[usHandle].usNextHandle ;
      fAssoc = TRUE;
   } /* endif */

   *pMatch = '\0';
   *pusRc = DamBTreeRc( QDAMDictPrev( DamRec[usHandle].pDamBTree, pMatch,
                                      &ulLen1, NULL,  &ulLen) );

   if ( (*pusRc == LX_RC_OK_ASD) || ( *pusRc == LX_EOF_ASD ))
   {
     /*******************************************************************/
     /* still associations left to be looked after ....                 */
     /*******************************************************************/
     usTmpHandle = DamRec[usHandle].usNextHandle;

     while ( fAssoc && usTmpHandle )
     {
       ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
       usRc = DamBTreeRc( QDAMDictPrev(DamRec[usTmpHandle].pDamBTree, szMatch,
                                          &ulLen1, NULL,  &ulLen2) );

       /*****************************************************************/
       /* if term found compare it with term of other dictionary,       */
       /*  use the alphabetical last  one ...                           */
       /*****************************************************************/
       if ( usRc == LX_RC_OK_ASD )
       {
         /*************************************************************/
         /* set sCompare depending on real value or EOF condition     */
         /*************************************************************/
         if ( *pusRc != LX_EOF_ASD  )                           /* 8@KIT0967A */
         {
           sCompare = (SHORT)UTF16stricmp( pMatch, szMatch );
         }
         else
         {
           sCompare = -1;
         } /* endif */

         if ( sCompare < 0 )
         {
           UTF16strcpy( pMatch, szMatch );
           ulLen = ulLen2;
           *pusDict = usTmpHandle;
           if ( *pusRc == LX_EOF_ASD )
           {
             *pusRc = usRc;
           } /* endif */
           /**************************************************************/
           /* reset the not used part (don't care about errors - we      */
           /* will get them on the next try... )                         */
           /**************************************************************/
           ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
           QDAMDictNext(DamRec[usHandle].pDamBTree,
                        szMatch, &ulLen1, NULL,  &ulLen2) ;
         }
         else if ( sCompare > 0 )
         {
           /**************************************************************/
           /* reset the not used part (don't care about errors - we      */
           /* will get them on the next try... )                         */
           /**************************************************************/
           ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
           QDAMDictNext(DamRec[usTmpHandle].pDamBTree,
                        szMatch, &ulLen1, NULL,  &ulLen2) ;
         } /* endif */
       }
       else if ( usRc == LX_EOF_ASD )
       {
         ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
         QDAMDictNext(DamRec[usTmpHandle].pDamBTree,
                      szMatch, &ulLen1, NULL,  &ulLen2);
         /***************************************************************/
         /* reset our primary dictionary if it is at EOF, too           */
         /***************************************************************/
         if ( *pusRc == LX_EOF_ASD )
         {
           ulLen1 = HEADTERM_SIZE * sizeof(CHAR_W);
           QDAMDictNext(DamRec[usHandle].pDamBTree,
                        szMatch, &ulLen1, NULL,  &ulLen2);
         } /* endif */
       } /* endif */
       usTmpHandle = DamRec[usTmpHandle].usNextHandle;
     } /* endwhile */
   } /* endif */
   // return the number of entries - used by Gaby for export
   *pulNum = DamRec[usHandle].ulNum;
#if defined(MEASURE)
  ulFind += (pGlobInfoSeg->msecs - ulBegin);
#endif
   *pulLen = ulLen / sizeof(CHAR_W);

  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPPRVTERMASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}
//

void DAMLINK
NlpInsEntryAsdW
(
   PSZ_W  pTerm,
   PBYTE  pData,
   ULONG  ulLen,     // # of bytes
   USHORT usHandle,
   USHORT usUser,
   PULONG pulNum,
   PUSHORT pusRc
)
{
   usUser; pulNum;

#if defined(MEASURE)
  ulBegin = pGlobInfoSeg->msecs;
#endif

   *pusRc = DamBTreeRc( QDAMDictInsertLocal( DamRec[usHandle].pDamBTree, pTerm, (PBYTE)pData, ulLen) );


#if defined(MEASURE)
  ulAdd += (pGlobInfoSeg->msecs - ulBegin);
#endif
  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPINSENTRYASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}

void DAMLINK
NlpUpdEntryAsdW
(
   PSZ_W  pTerm,
   PBYTE  pData,
   ULONG  ulLen,         // # of BYTES
   USHORT usHandle,
   USHORT usUser,
   PULONG pulNum,
   PUSHORT pusRc
)
{
   usUser; pulNum;

#if defined(MEASURE)
  ulBegin = pGlobInfoSeg->msecs;
#endif

   *pusRc = DamBTreeRc( QDAMDictUpdateLocal( DamRec[usHandle].pDamBTree, pTerm, (PBYTE)pData, ulLen) );

#if defined(MEASURE)
  ulUpdate += (pGlobInfoSeg->msecs - ulBegin);
#endif
  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPUPDENTRYASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}

//
void DAMLINK
NlpDelEntryAsdW
(
   PSZ_W  pTerm,
   USHORT usHandle,
   USHORT usUser,
   PUSHORT pusRc
)
{
   usUser;

#if defined(MEASURE)
  ulBegin = pGlobInfoSeg->msecs;
#endif

   *pusRc = DamBTreeRc( QDAMDictDeleteLocal( DamRec[usHandle].pDamBTree, pTerm) );

#if defined(MEASURE)
  ulUpdate += (pGlobInfoSeg->msecs - ulBegin);
#endif
  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPDELENTRYASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}

#define NlpRetSpellAsd( term,handle,rule,ver,user,num,list,len,rc ) \
    { *rc = LX_RC_OK_ASD }
//
#define NlpRetListAsd( handle,count,user,num,list,len,rc ) \
    { *rc = LX_RC_OK_ASD }
//
void DAMLINK
NlpResynchAsd
(
   USHORT usHandle,
   USHORT usUser,
   PUSHORT pusRc
)
{
   usUser;

   *pusRc = DamBTreeRc ( QDAMDictFlushLocal( DamRec[usHandle].pDamBTree ) );
}
//
//    - Find number of terms in dictionary      QDAMDictNumEntries
//    - find first entry in dictionary          QDAMDictFirst


USHORT DamBTreeRc
(
   SHORT sRc
)
{
   USHORT  usRc;

   switch ( sRc )
   {
     case 0:
        usRc = LX_RC_OK_ASD;
        break;
     case BTREE_NO_ROOM:
        usRc = LX_MEM_ALLOC_ASD;
        break;
     case BTREE_ILLEGAL_FILE:                   /* Not an index file       */
        usRc =  LX_OLD_FORMAT_ASD;
        break;
     case BTREE_DUPLICATE_KEY:
        usRc = LX_WRD_EXISTS_ASD;
        break;
     case BTREE_NO_BUFFER:                      /* No buffer available     */
        usRc = LX_INSUF_STOR_ASD;
        break;
     case BTREE_NOT_FOUND:                      /* Key not found           */
        usRc = LX_WRD_NT_FND_ASD;
        break;
     case BTREE_INVALID:                        /* Btree pointer invalid   */
        usRc = LX_INCOMP_SIG_ASD;
        break;
     case BTREE_READ_ERROR:                     /* read error on file      */
        usRc = LX_DICT_WRT_ASD;
        break;
     case BTREE_CORRUPTED:                      /* binary tree is corrupted*/
        usRc = LX_RENUM_RQD_ASD;
        break;
     case BTREE_BUFFER_SMALL:                   /* buffer to small for data*/
        usRc = LX_DATA_2_LRG_ASD;
        break;
     case BTREE_DISK_FULL:                      /* disk is full            */
        usRc = LX_IDX_FN_ERR;
        break;
     case BTREE_USERDATA:                       /* user data too large     */
        usRc = LX_DATA_2_LRG_ASD;
        break;
     case BTREE_EOF_REACHED:                    /* eof or start reached    */
        usRc = LX_EOF_ASD;
        break;
     case BTREE_EMPTY:                          /* no entries in dictionary*/
        usRc = LX_INVLD_WRDNM_ASD;
        break;
     case BTREE_WRITE_ERROR:                    /* error during write      */
        usRc = LX_DICT_WRT_ASD;
        break;
     case BTREE_OPEN_ERROR:                     /* error during open       */
        usRc = LX_OPEN_FLD_ASD;
        break;
     case BTREE_CLOSE_ERROR:                    /* error during close      */
        usRc = LX_DICT_WRT_ASD;
        break;
     case BTREE_NUMBER_RANGE:
        usRc = LX_INVLD_WRDNM_ASD;
        break;
     case BTREE_DICT_LOCKED:
        usRc = LX_PROTECTED_ASD;
        break;
     case BTREE_ENTRY_LOCKED:
        usRc = LX_OTHER_USER_ASD;
        break;
     case BTREE_LOCK_ERROR:
     case BTREE_IN_USE:
     case BTREE_ACCESS_ERROR:
     case BTREE_FILE_NOTFOUND:
     case BTREE_INVALID_DRIVE:
     case BTREE_OPEN_FAILED:
     case BTREE_NETWORK_ACCESS_DENIED:
     case TMERR_TOO_MANY_QUERIES:
     case TMERR_TOO_MANY_OPEN_DATABASES:
     case TMERR_TOO_MANY_USERS_CONNECTED:
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
       /*****************************************************************/
       /* Leave return code as-is                                      */
       /****************************************************************/
       usRc = sRc;
       break;

     default:
        usRc = LX_UNEXPECTED_ASD;
        break;
   } /* endswitch */


   return usRc ;
}
/**********************************************************************/
/* our own NLP calls                                                  */
/**********************************************************************/
void DAMLINK
NlpLockEntryW
(
  USHORT  usHandle,
  PSZ_W   pTerm,
  BOOL    fLock,
  PUSHORT pusRc
)
{
   USHORT usTmpHandle;                       // 2.nd handle in case of assoc
   BOOL   fAssoc = FALSE;

   // is it a associate handle ????
   if ( ! DamRec[usHandle].pDamBTree )
   {
      usHandle = DamRec[usHandle].usNextHandle ;
      fAssoc = TRUE;
   } /* endif */

   *pusRc = DamBTreeRc( QDAMDictLockEntryLocal( DamRec[usHandle].pDamBTree,
                                             pTerm, fLock ));
   usTmpHandle = DamRec[usHandle].usNextHandle;
   while ( fAssoc && usTmpHandle && (*pusRc == LX_RC_OK_ASD) )
   {
     *pusRc = DamBTreeRc( QDAMDictLockEntryLocal( DamRec[usTmpHandle].pDamBTree,
                                               pTerm, fLock ));
     usTmpHandle = DamRec[usTmpHandle].usNextHandle;
   } /* endwhile */
  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPLOCKENTRY_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}

void DAMLINK
NlpLockDict
(
  USHORT  usHandle,
  BOOL    fLock,
  PUSHORT pusRc
)
{
   USHORT usTmpHandle;                       // 2.nd handle in case of assoc
   BOOL   fAssoc = FALSE;

   // is it a associate handle ????
   if ( ! DamRec[usHandle].pDamBTree )
   {
      usHandle = DamRec[usHandle].usNextHandle ;
      fAssoc = TRUE;
   } /* endif */

   *pusRc = DamBTreeRc( QDAMDictLockDictLocal( DamRec[usHandle].pDamBTree, fLock ));

   usTmpHandle = DamRec[usHandle].usNextHandle;
   while ( fAssoc && usTmpHandle && (*pusRc == LX_RC_OK_ASD) )
   {
     *pusRc = DamBTreeRc( QDAMDictLockDictLocal( DamRec[usTmpHandle].pDamBTree, fLock ));
     usTmpHandle = DamRec[usTmpHandle].usNextHandle;
   } /* endwhile */
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FindDict                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       find if dictionary is open or set new slot               |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ  pName,               dictionary name                |
//|                   PUSHORT  pusI             number of slot                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   dictionary found, slot returned                   |
//|                   FALSE  dictionary not found, free slot returned          |
//+----------------------------------------------------------------------------+
//|Function flow:     check if dict is available in list                       |
//|                   return slot identifier                                   |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
static BOOL
FindDict
(
  PSZ  pName,                          // dictionary name
  PUSHORT  pusI                        // number of slot
)
{
  USHORT  usI = 1;
  USHORT  usActive = 0;
  BOOL    fFound = FALSE;

   /*******************************************************************/
   /* check if dict is available in list                              */
   /*******************************************************************/
   for ( usI =1; (usI < MAX_NUM_DICTS) && !fFound ; usI++ )
   {
     if ( DamRec[usI].pDamBTree || DamRec[usI].usNextHandle )
     {
       if ( stricmp(DamRec[usI].chDictName, pName ) == 0)
       {
         fFound = TRUE;
         usActive = usI;
       }
     }
     else
     {
       if (! usActive )
       {
         usActive = usI;
       } /* endif */
     } /* endif */
   } /* endfor */

   /*******************************************************************/
   /* return slot identifier                                          */
   /*******************************************************************/
   *pusI = usActive;

   return fFound;
} /* end of function FindDict */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NlpRetBTree                                              |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT usHandle,                                         |
//|                   PVOID  * ppBTree,                                        |
//|                   PLONG  pLHandle                                          |
//+----------------------------------------------------------------------------+
//|Description:       this function will return the BTree handle and the       |
//|                   HTM if available..                                       |
//+----------------------------------------------------------------------------+
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+

void DAMLINK
NlpRetBTree
(
  USHORT usHandle,
  PVOID  * ppBTree,
  PLONG  pLHandle
)
{
  if ( DamRec[usHandle].pDamBTree )
  {
    if ( DamRec[usHandle].pDamBTree->pBTree )
    {
      *ppBTree = DamRec[usHandle].pDamBTree;
      *pLHandle = NULLHANDLE;
    }
    else
    {
      *ppBTree = DamRec[usHandle].pDamBTree->pBTreeRemote;
      *pLHandle = (LONG) (DamRec[usHandle].pDamBTree->htm);
    } /* endif */
  }
  else
  {
    *ppBTree = NULL;
    *pLHandle = NULLHANDLE;
  } /* endif */
} /* end of function NlpRetBTree */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NlpCloseOrganize                                         |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       function called for closing a remote organize process    |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT usHandle     // handle of dict                    |
//|                   PSZ pszDictPath,    // dictionary name                   |
//|                   CHAR chPrimDrive,   // primary drive                     |
//|                   PUSHORT pusRc,      // return code                       |
//|                   USHORT usRc         // action code                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
void DAMLINK
NlpCloseOrganize( USHORT usHandle,
                  PSZ pszDictPath,
                  CHAR chPrimDrive,
                  PUSHORT pusRc,
                  USHORT usRc )
{
  if ( DamRec[usHandle].pDamBTree )
  {
    *pusRc = DamBTreeRc( QDAMDictCloseOrganize( &(DamRec[usHandle].pDamBTree),
                                    pszDictPath, chPrimDrive, usRc ));
  }
  else
  {
    *pusRc = LX_RC_OK_ASD;
  } /* endif */
  DamRec[usHandle].usNextHandle = 0;
  DamRec[usHandle].usOpenCount = 0;
  DamRec[usHandle].chDictName[0] = '\0';

} /* end of function NlpCloseOrganize */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NlpTermList                                              |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       will return a list with the number of requested terms    |
//|                   (if available) for single and associated dictionary      |
//|                   handles.                                                 |
//|                   The returned list is an ASCII-Z list.                    |
//|                   The list won't be sorted, because this will be done      |
//|                   automatically by PM.                                     |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT  usHandle,     handle of dictionary               |
//|                   USHORT  usUser,       user control block                 |
//|                   PUCHAR  pTerm,        term to start with                 |
//|                   USHORT  usNumTerms,   number of terms to be looked up    |
//|                   USHORT  usAction,     required action                    |
//|                   PUCHAR  pucBuffer,    pointer to buffer                  |
//|                   USHORT  usLen,        length of provided buffer          |
//|                   PUSHORT pusRc         pointer to success indicator       |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:   - init structures                                          |
//|                 - determine number of terms in each direction, i.e. if both|
//|                   directions requested split number of terms into two parts|
//|                                                                            |
//|                 - determine the dictionary handle to start with, i.e. check|
//|                   if we are dealing with associate handles...              |
//|                 - allocate space for head terms and anchor it in dicthandle|
//|                   array                                                    |
//|                 - fill list with forward positions...                      |
//|                 - fill list with backward positions...                     |
//|                 - set the return code and free the allocated resources     |
//|                 - return return code                                       |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
void DAMLINK
NlpTermListW
(
  USHORT  usUser,                      // user control block
  USHORT  usHandle,                    // handle of dictionary
  PSZ_W   pTerm,                       // term to start with
  USHORT  usNumTerms,                  // number of terms to be looked up
  USHORT  usAction,                    // required action
  PSZ_W   pucBuffer,                   // pointer to buffer
  USHORT  usBufLen,                    // length of provided buffer (# CHAR_W's)
  PUSHORT pusRc                        // pointer to success indicator
)
{
  USHORT  usRc = 0;                    // success indicator
  DICTHANDLETERM stDictHT[ MAX_DICTS ];// list of dictionaries
  USHORT  i;                           // index
  PSZ_W   pActTerm = pucBuffer;        // pointer to first matching term

  usUser;
  /********************************************************************/
  /* init structures                                                  */
  /********************************************************************/
  memset( stDictHT, 0, sizeof( stDictHT ));
  memset( (PBYTE)pucBuffer, 0, usBufLen * sizeof(CHAR_W) );
  /********************************************************************/
  /* determine number of terms in each direction, i.e. if both        */
  /* directions requested split number of terms into two parts.       */
  /********************************************************************/
  if ( (usAction & QDAM_LIST_FORWARD) && ( usAction & QDAM_LIST_BACKWARD ) )
  {
    usNumTerms /= 2;
  } /* endif */
  /********************************************************************/
  /* determine the dictionary handle to start with, i.e. check if we  */
  /* are dealing with associate handles...                            */
  /********************************************************************/
  if ( ! DamRec[usHandle].pDamBTree )
  {
     usHandle = DamRec[usHandle].usNextHandle ;
  } /* endif */

  /********************************************************************/
  /* allocate space for head terms and anchor it in dicthandle array  */
  /********************************************************************/
  i = 0;
  while ( usHandle && !usRc)
  {
    stDictHT[i].usHandle = usHandle;
    if ( UtlAlloc( (PVOID *)&(stDictHT[i].pTerm), 0L, (LONG) HEADTERM_SIZE * sizeof(CHAR_W), NOMSG ) )
    {
      usHandle = DamRec[usHandle].usNextHandle ;
    }
    else
    {
      usRc = BTREE_NO_ROOM;
    } /* endif */

    i++;                               // next entry
  } /* endwhile */

  /********************************************************************/
  /* fill list with forward positions...                              */
  /********************************************************************/
  if ( !usRc && (usAction & QDAM_LIST_FORWARD))
  {
    usRc = NlpForwardList(stDictHT, pTerm, usNumTerms, &pucBuffer, &usBufLen);
  } /* endif */

  /********************************************************************/
  /* fill list in backward positions ...                              */
  /********************************************************************/
  if ( !usRc && (usAction & QDAM_LIST_BACKWARD))
  {
    /******************************************************************/
    /* position at the first found term - if any found                */
    /******************************************************************/
    if ( !*pActTerm )
    {
      pActTerm = pTerm;
    } /* endif */

    usRc = NlpBackwardList(stDictHT, pActTerm, usNumTerms, &pucBuffer,
                           &usBufLen, (usAction & QDAM_LIST_FORWARD) );
  } /* endif */


  /********************************************************************/
  /* set the return code and free the allocated resources             */
  /********************************************************************/
  *pusRc = usRc;

  i = 0;
  while ( stDictHT[i].pTerm )
  {
    UtlAlloc( (PVOID *)&(stDictHT[i].pTerm), 0L, 0L, NOMSG );
    i++;
  } /* endwhile */

  if ( usRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPTERMLIST_LOC, INTFUNCFAILED_EVENT, usRc );
  } /* endif */


} /* end of function NlpTermList */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NlpForwardList                                           |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       this function will fill the ASCII-Z list with the        |
//|                   requested number of terms larger than the input term     |
//+----------------------------------------------------------------------------+
//|Parameters:        PDICTHANDLETERM pstDictHT,  list of dictionaries         |
//|                   PUCHAR          pTerm,      pointer to term              |
//|                   USHORT          usNumTerms, number of terms to be looked |
//|                   PUCHAR         *ppBuffer,   pointer to buffer            |
//|                   PUSHORT         pusBufLen,  pointer to buffer length     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return codes from BTREE operations                       |
//+----------------------------------------------------------------------------+
//|Function flow:     find the start position for each of the dictionaries     |
//|                   in case of EOF condition reset usRc and init the returned|
//|                     term, if word not found position at prev. term         |
//|                   get number of terms and fill them in buffer.             |
//|                   update free length and buffer                            |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+

static USHORT
NlpForwardList
(
  PDICTHANDLETERM pstDictHT,           // list of dictionaries
  PSZ_W           pTerm,               // pointer to term
  USHORT          usNumTerms,          // number of terms to be looked up
  PSZ_W           *ppBuffer,            // pointer to buffer
  PUSHORT         pusBufLen            // buffer length in no of CHAR_W's
)
{
  USHORT usRc = 0;                     // success indicator
  USHORT usHandle;                     // handle of dictionary
  USHORT i;                            // index
  ULONG  ulLen;                        // length of term
  ULONG  ulUserLen;                    // length of user data
  PBYTE  pucBuffer = (PBYTE)(*ppBuffer);        // pointer to buffer
  USHORT usBufLen = *pusBufLen;        // length of buffer
  USHORT usTermCnt;                    // number of terms..
  CHAR_W   chTerm[ HEADTERM_SIZE ];      // temp buffer for headterm

  /********************************************************************/
  /* find the start position for each of the dictionaries             */
  /********************************************************************/

  i=0;
  while ( ((usHandle = (pstDictHT+i)->usHandle)!= 0) && !usRc  )
  {
    ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
    ulUserLen = 0;
    usRc = DamBTreeRc( QDAMDictSubStr( DamRec[usHandle].pDamBTree,
                                         pTerm, (PBYTE)(pstDictHT+i)->pTerm,
                                         &ulLen, NULL, &ulUserLen ) );

    /****************************************************************/
    /* in case of EOF condition reset usRc and init the returned    */
    /* term                                                         */
    /****************************************************************/
    if ( usRc == LX_EOF_ASD )
    {
      usRc = 0;
      (pstDictHT+i)->pTerm[0] = EOS;
    }
    else if (( usRc == LX_WRD_NT_FND_ASD )
           && (DamRec[usHandle].pDamBTree->sCurrentIndex != -1))
    {
      /****************************************************************/
      /* get next possible term...                                    */
      /****************************************************************/
      ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
      usRc = DamBTreeRc( QDAMDictCurrent( DamRec[usHandle].pDamBTree,
                                          (PBYTE)((pstDictHT+i)->pTerm),
                                          &ulLen, NULL,  &ulUserLen) );
      /****************************************************************/
      /* if term is still too small, go forward  ...                  */
      /****************************************************************/
      if ( !usRc &&
//        (strcmp((pstDictHT+i)->pTerm, pTerm) < 0) )
          ( QDAMKeyCompare( DamRec[usHandle].pDamBTree,
                            (pstDictHT+i)->pTerm, pTerm ) < 0 ))
      {
        ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
        ulUserLen = 0;
        usRc = DamBTreeRc( QDAMDictNext( DamRec[usHandle].pDamBTree,
                                         (pstDictHT+i)->pTerm,
                                         &ulLen, NULL,  &ulUserLen) );
        if ( usRc == LX_EOF_ASD )
        {
          usRc = 0;
          (pstDictHT+i)->pTerm[0] = EOS;
        } /* endif */
      } /* endif */
    } /* endif */
    i++;
  } /* endwhile */

  /********************************************************************/
  /* if forward search - get the items and fill them in               */
  /********************************************************************/
  usTermCnt = 0;
  while ( !usRc && (usTermCnt < usNumTerms) )
  {
    usRc = GetNextTerm( pstDictHT, chTerm );
    /******************************************************************/
    /* copy smallest term into provided area and fill next            */
    /******************************************************************/
    if ( !usRc )
    {
      usTermCnt++;                     // we filled in another term

      ulLen = UTF16strlenBYTE( chTerm );
      /******************************************************************/
      /* if we have identified a term ...                               */
      /******************************************************************/
      if ( ulLen )
      {
        ulLen += sizeof(CHAR_W);
        if ( ulLen < (ULONG)usBufLen * sizeof(CHAR_W) )
        {
          memcpy( pucBuffer, (PBYTE)chTerm, ulLen  );
          pucBuffer += ulLen;
          chTerm[0] = EOS;             // init for new term
          usBufLen = (USHORT)(usBufLen - (UTF16strlenCHAR(chTerm)));
        }
        else
        {
          usRc = BTREE_BUFFER_SMALL;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* set return values, i.e. still free buffer lenght and pointer     */
  /* to next free buffer position                                     */
  /********************************************************************/
  *pusBufLen = usBufLen;
  *ppBuffer = (PSZ_W)pucBuffer;

  if ( usRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPFORWARDDLIST_LOC, INTFUNCFAILED_EVENT, usRc );
  } /* endif */

  return usRc;
} /* end of function NlpForwardList */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NlpBackwardList                                          |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       this function will fill the ASCII-Z list with the        |
//|                   requested number of terms smaller than the input term    |
//+----------------------------------------------------------------------------+
//|Parameters:        PDICTHANDLETERM pstDictHT,  list of dictionaries         |
//|                   PUCHAR          pTerm,      pointer to term              |
//|                   USHORT          usNumTerms, number of terms to be looked |
//|                   PUCHAR         *ppBuffer,   pointer to buffer            |
//|                   PUSHORT         pusBufLen,  pointer to buffer length     |
//|                   BOOL            fForwardAct forward already active??     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return codes from BTREE operations                       |
//+----------------------------------------------------------------------------+
//|Function flow:     find the start position for each of the dictionaries     |
//|                   in case of EOF condition reset usRc and init the returned|
//|                     term, if word not found position at prev. term         |
//|                   skip current term if forward and backward list req.      |
//|                   get number of terms and fill them in buffer.             |
//|                   update free length and buffer                            |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+

static USHORT
NlpBackwardList
(
  PDICTHANDLETERM pstDictHT,           // list of dictionaries
  PSZ_W           pTerm,               // pointer to term
  USHORT          usNumTerms,          // number of terms to be looked up
  PSZ_W           *ppBuffer,            // pointer to buffer
  PUSHORT         pusBufLen,           // pointer to buffer length ( # CHAR_W's)
  BOOL            fForwardAct          // forward already active??
)
{
  USHORT usRc = 0;                     // success indicator
  USHORT usHandle;                     // handle of dictionary
  USHORT i;                            // index
  ULONG  ulLen;                        // length of term
  ULONG  ulUserLen;                    // length of user data
  PBYTE  pucBuffer = (PBYTE)(*ppBuffer);        // pointer to buffer
  USHORT usBufLen = *pusBufLen;        // length of buffer
  CHAR_W   chTerm[ HEADTERM_SIZE ];      // temp buffer for headterm
  USHORT usTermCnt;                    // number of terms inserted

  /********************************************************************/
  /* find the start position for each of the dictionaries             */
  /********************************************************************/

  i=0;
  while ( ((usHandle = (pstDictHT+i)->usHandle)!= 0) && !usRc  )
  {
    ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
    ulUserLen = 0;
    usRc = DamBTreeRc( QDAMDictSubStr( DamRec[usHandle].pDamBTree,
                                         pTerm, (PBYTE)((pstDictHT+i)->pTerm),
                                         &ulLen, NULL, &ulUserLen ) );

    /****************************************************************/
    /* in case of EOF condition reset usRc and init the returned    */
    /* term                                                         */
    /****************************************************************/
    if ( usRc == LX_EOF_ASD )
    {
      usRc = 0;
      (pstDictHT+i)->pTerm[0] = EOS;
    }
    else if (( usRc == LX_WRD_NT_FND_ASD )
           && (DamRec[(pstDictHT+i)->usHandle].pDamBTree->sCurrentIndex != -1))
    {

      /****************************************************************/
      /* get next possible term...                                    */
      /****************************************************************/
      ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
      usRc = DamBTreeRc( QDAMDictCurrent( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                            (PBYTE)((pstDictHT+i)->pTerm),
                             &ulLen, NULL,  &ulUserLen) );
      /****************************************************************/
      /* if term is still too large, go backward ...                  */
      /****************************************************************/
      if ( !usRc &&
//       (strcmp((pstDictHT+i)->pTerm, pTerm) > 0) )
          ( QDAMKeyCompare( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                            (pstDictHT+i)->pTerm, pTerm ) > 0 ))
      {
        ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
        ulUserLen = 0;
        usRc = DamBTreeRc(
                QDAMDictPrev( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                              (pstDictHT+i)->pTerm,
                               &ulLen, NULL,  &ulUserLen) );
        if ( usRc == LX_EOF_ASD )
        {
          usRc = 0;
          (pstDictHT+i)->pTerm[0] = EOS;
        } /* endif */
      } /* endif */
    } /* endif */
    i++;
  } /* endwhile */

  /********************************************************************/
  /* skip current term ...                                            */
  /********************************************************************/
  if ( fForwardAct )
  {
    usRc = GetPrevTerm( pstDictHT, chTerm );
  } /* endif */
  /********************************************************************/
  /* get the items and fill them                                      */
  /********************************************************************/
  usTermCnt = 0;
  while ( !usRc && (usTermCnt < usNumTerms) )
  {
    usRc = GetPrevTerm( pstDictHT, chTerm );
    /******************************************************************/
    /* copy smallest term into provided area and fill next            */
    /******************************************************************/
    if ( !usRc )
    {
      usTermCnt++;                     // we filled in another term

      ulLen = UTF16strlenBYTE( chTerm );
      /******************************************************************/
      /* if we have identified a term ...                               */
      /******************************************************************/
      if ( ulLen )
      {
        ulLen += sizeof(CHAR_W);
        if ( ulLen < (ULONG)usBufLen * sizeof(CHAR_W) )
        {
          memcpy( pucBuffer, (PBYTE)chTerm, ulLen );
          pucBuffer += ulLen;
          usBufLen = (USHORT)(usBufLen - ( UTF16strlenCHAR(chTerm)));
        }
        else
        {
          usRc = BTREE_BUFFER_SMALL;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* set return values, i.e. still free buffer lenght and pointer     */
  /* to next free buffer position                                     */
  /********************************************************************/
  *pusBufLen = usBufLen;
  *ppBuffer = (PSZ_W)pucBuffer;

  if ( usRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPBACKWARDLIST_LOC, INTFUNCFAILED_EVENT, usRc );
  } /* endif */

  return usRc;
} /* end of function NlpBackwardList */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetNextTerm                                              |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       This function will work on a (list of) dictionaries and  |
//|                   return the next matching term.                           |
//+----------------------------------------------------------------------------+
//|Parameters:        PDICTHANDLETERM pstDictHT    list of dictionaries        |
//|                   PUCHAR          pchTerm      pointer to found term       |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of BTREE functions                           |
//+----------------------------------------------------------------------------+
//|Function flow:     loop thru all associated dictionaries and return         |
//|                    the smallest term.                                      |
//+----------------------------------------------------------------------------+
static USHORT
GetNextTerm
(
  PDICTHANDLETERM pstDictHT,           // list of dictionaries
  PSZ_W           pchTerm              // pointer to found term
)
{
  USHORT  usRc = 0;                    // success indicator
  USHORT  usAct = 0;                   // active term
  USHORT  i;                           // index
  SHORT   sComp;                       // compare value between terms
  ULONG   ulLen;                       // length of headterm
  ULONG   ulUserLen;                   // length of user data

  *pchTerm = EOS;                      // init values
  i = 0;
  /******************************************************************/
  /* find out smallest term ...                                     */
  /******************************************************************/
  while ( ((pstDictHT+i)->usHandle) && !usRc )
  {
    if ( (pstDictHT+i)->pTerm[0])
    {
       sComp =  QDAMKeyCompare( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                                pchTerm, (pstDictHT+i)->pTerm );
//       sComp = strcmp(pchTerm, (pstDictHT+i)->pTerm);
       if ( (sComp > 0) || (*pchTerm == EOS) )
       {
         usAct = i;
         UTF16strcpy( pchTerm, (pstDictHT+i)->pTerm );
       }
       else if (sComp == 0 )
       {
         /***********************************************************/
         /* we have already a matching term -- go ahead with next   */
         /* term                                                    */
         /***********************************************************/
         (pstDictHT+i)->pTerm[0] = '\0';
         ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
         usRc = DamBTreeRc(
                 QDAMDictNext( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                               (pstDictHT+i)->pTerm,
                                &ulLen, NULL,  &ulUserLen) );

         /****************************************************************/
         /* in case of EOF condition reset usRc and init the returned    */
         /* term                                                         */
         /****************************************************************/
         if ( usRc == LX_EOF_ASD)
         {
           ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
           usRc = DamBTreeRc(
                    QDAMDictPrev( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                                 (pstDictHT+i)->pTerm,
                                 &ulLen, NULL,  &ulUserLen) );
           (pstDictHT+i)->pTerm[0] = EOS;
         } /* endif */
       } /* endif */
    } /* endif */
    i++;
  } /* endwhile */

  /********************************************************************/
  /* get next term in the dict (if we found a matching one )          */
  /********************************************************************/
  if ( !usRc && *pchTerm )
  {
    /******************************************************************/
    /* get a new term for the dictionary we currently used            */
    /******************************************************************/
    (pstDictHT+usAct)->pTerm[0] = '\0';
    ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
    usRc = DamBTreeRc(
            QDAMDictNext( DamRec[(pstDictHT+usAct)->usHandle].pDamBTree,
                          (pstDictHT+usAct)->pTerm,
                           &ulLen, NULL,  &ulUserLen) );

    /****************************************************************/
    /* in case of EOF condition reset usRc and init the returned    */
    /* term                                                         */
    /****************************************************************/
    if ( usRc == LX_EOF_ASD)
    {
      usRc = 0;
      (pstDictHT+usAct)->pTerm[0] = EOS;
    } /* endif */
  } /* endif */
  return usRc;
} /* end of function GetNextTerm */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetPrevTerm                                              |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       This function will work on a (list of) dictionaries and  |
//|                   return the largest entry.                                |
//+----------------------------------------------------------------------------+
//|Parameters:        PDICTHANDLETERM pstDictHT    list of dictionaries        |
//|                   PUCHAR          pchTerm      pointer to found term       |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of BTREE functions                           |
//+----------------------------------------------------------------------------+
//|Function flow:     loop thru all associated dictionaries and return         |
//|                    the largest  term.                                      |
//+----------------------------------------------------------------------------+
static USHORT
GetPrevTerm
(
  PDICTHANDLETERM pstDictHT,           // list of dictionaries
  PSZ_W           pchTerm              // pointer to found term
)
{
  USHORT  usRc = 0;                    // success indicator
  USHORT  usAct = 0;                   // active term
  USHORT  i;                           // index
  SHORT   sComp;                       // compare value between terms
  ULONG   ulLen;                       // length of headterm
  ULONG   ulUserLen;                   // length of user data


  *pchTerm = EOS;                      // init values
  i = 0;
  /******************************************************************/
  /* find out largest term ...                                      */
  /******************************************************************/
  while ( ((pstDictHT+i)->usHandle) && !usRc )
  {
    if ( (pstDictHT+i)->pTerm[0])
    {
//       sComp = strcmp(pchTerm, (pstDictHT+i)->pTerm);
       sComp =  QDAMKeyCompare( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                                pchTerm, (pstDictHT+i)->pTerm );
       if ( (sComp < 0) || (*pchTerm == EOS) )
       {
         usAct = i;
         UTF16strcpy( pchTerm, (pstDictHT+i)->pTerm );
       }
       else if (sComp == 0 )
       {
         /***********************************************************/
         /* we have already a matching term -- go ahead with next   */
         /* term                                                    */
         /***********************************************************/
         (pstDictHT+i)->pTerm[0] = '\0';
         ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
         usRc = DamBTreeRc(
                 QDAMDictPrev( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                               (pstDictHT+i)->pTerm,
                                &ulLen, NULL,  &ulUserLen) );
         /****************************************************************/
         /* in case of EOF condition reset usRc and init the returned    */
         /* term                                                         */
         /****************************************************************/
         if ( usRc == LX_EOF_ASD)
         {
           ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
           usRc = DamBTreeRc(
                    QDAMDictNext( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                                 (pstDictHT+i)->pTerm,
                                 &ulLen, NULL,  &ulUserLen) );
           (pstDictHT+i)->pTerm[0] = EOS;
         } /* endif */
       } /* endif */
    } /* endif */
    i++;
  } /* endwhile */

  /********************************************************************/
  /* get next term in the dict (if we found a matching one)           */
  /********************************************************************/
  if ( !usRc && *pchTerm )
  {
    /******************************************************************/
    /* get a new term for the dictionary we currently used            */
    /******************************************************************/
    (pstDictHT+usAct)->pTerm[0] = '\0';
    ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
    usRc = DamBTreeRc(
            QDAMDictPrev( DamRec[(pstDictHT+usAct)->usHandle].pDamBTree,
                          (pstDictHT+usAct)->pTerm,
                           &ulLen, NULL,  &ulUserLen) );
    /****************************************************************/
    /* in case of EOF condition reset usRc and init the returned    */
    /* term                                                         */
    /****************************************************************/
    if ( usRc == LX_EOF_ASD)
    {
      usRc = 0;
      (pstDictHT+usAct)->pTerm[0] = EOS;
    } /* endif */
  } /* endif */
  return usRc;
} /* end of function GetPrevTerm */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMInit                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMInit();                                              |
//+----------------------------------------------------------------------------+
//|Description:       init data areas for QDAM functions                       |
//|                   This function will be called by EQFSTART...              |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
VOID QDAMInit()
{
    memset( &DamRec[0], 0, sizeof(DAM2QDAM) * MAX_NUM_DICTS );
    memset( &QDAMDict[0], 0, sizeof(QDAMDICT) * MAX_NUM_DICTS );
    fDamInit = TRUE;
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDamWildCardList                                         |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       will return a list of terms matching the given pattern   |
//|                   (if available) for single and associated dictionary      |
//|                   handles.                                                 |
//|                   The returned list is an ASCII-Z list.                    |
//|                   The list won't be sorted, because this will be done      |
//|                   automatically by PM.                                     |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT  usUser,       // in  - user handle               |
//|                   USHORT  usHandle,     // in  - dictionary handle         |
//|                   PSZ     pTerm,        // in - term to start with         |
//|                   PSZ     pPattern,     // in - search pattern/compound(s) |
//|                   USHORT  usNumTerms,   // in - number of terms requested  | up
//|                   BOOL    fCompound,    // in - compound search flag       |
//|                   PUCHAR  pucBuffer,    // in - buffer for term list       |
//|                   USHORT  usLen         // in - size of buffer             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT (Nlp return code)                                 |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT QDamWildCardList
(
  USHORT  usUser,                      // in  - user handle
  USHORT  usHandle,                    // in  - dictionary handle
  PSZ_W   pTerm,                       // in - term to start with
  PSZ_W     pPattern,                    // in - search pattern/compound(s)
  USHORT  usNumTerms,                  // in - number of terms to be looked up
  BOOL    fCompound,                   // in - compound search flag
  PSZ_W   pucBuffer,                   // in - buffer for term list
  ULONG   ulBufLen                     // in - size of buffer
)
{
  USHORT  usRc = 0;                    // success indicator
  DICTHANDLETERM stDictHT[ MAX_DICTS ];// list of dictionaries
  USHORT  i;                           // index
  PSZ_W   pEndString = NULL;           // ptr to and start/end string
  USHORT  usEndLen = 0;                // length of end string

  usUser;

  /********************************************************************/
  /* init structures                                                  */
  /********************************************************************/
  memset( stDictHT, 0, sizeof( stDictHT ));
  memset( pucBuffer, 0, ulBufLen * sizeof(CHAR_W) );

  /********************************************************************/
  /* determine the dictionary handle to start with, i.e. check if we  */
  /* are dealing with associate handles...                            */
  /********************************************************************/
  if ( !DamRec[usHandle].pDamBTree )
  {
     usHandle = DamRec[usHandle].usNextHandle ;
  } /* endif */

  /********************************************************************/
  /* allocate space for head terms and anchor it in dicthandle array  */
  /********************************************************************/
  i = 0;
  while ( usHandle && !usRc)
  {
    stDictHT[i].usHandle = usHandle;
    if ( UtlAlloc( (PVOID *)&(stDictHT[i].pTerm), 0L,
                   (LONG) HEADTERM_SIZE * sizeof(CHAR_W), NOMSG ) )
    {
      usHandle = DamRec[usHandle].usNextHandle ;
    }
    else
    {
      usRc = BTREE_NO_ROOM;
    } /* endif */

    i++;                               // next entry
  } /* endwhile */

  /********************************************************************/
  /* Setup start position for pattern matching and non-substitution   */
  /* characters at begin of pattern and no start term given           */
  /********************************************************************/
  if ( !usRc && !fCompound )
  {
    PSZ_W    pszTemp;                    // ptr for pattern string processing

    pszTemp = pPattern;
    usEndLen = 0;
    while ( (*pszTemp != EOS) &&
            (*pszTemp != MULTIPLE_SUBSTITUTION) &&
            (*pszTemp != SINGLE_SUBSTITUTION) )
    {
      usEndLen++;
      pszTemp++;
    } /* endwhile */

    if ( usEndLen )
    {
      if ( UtlAlloc( (PVOID *)&pEndString, 0L, (LONG) HEADTERM_SIZE, NOMSG ) )
      {
        memcpy( (PBYTE)pEndString, (PBYTE)pPattern, usEndLen  * sizeof(CHAR_W));
        pEndString[usEndLen] = EOS;

        /**************************************************************/
        /* if no start term is supplied use the end term as start term*/
        /**************************************************************/
        if ( *pTerm == EOS )
        {
          pTerm = pEndString;
        } /* endif */
      }
      else
      {
        usRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* fill the list with terms...                                      */
  /********************************************************************/
  if ( !usRc )
  {
    USHORT usHandle;                     // handle of dictionary
    ULONG  ulLen;                        // length of term
    ULONG  ulUserLen;                    // length of user data
    USHORT usTermCnt;                    // number of terms..
    CHAR_W   chTerm[ HEADTERM_SIZE ];      // temp buffer for headterm

    /********************************************************************/
    /* find the start position for each of the dictionaries             */
    /********************************************************************/
    i=0;
    while ( ((usHandle = stDictHT[i].usHandle)!= 0) && !usRc  )
    {
      ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
      ulUserLen = 0;
      usRc = DamBTreeRc( QDAMDictSubStr( DamRec[usHandle].pDamBTree,
                                           pTerm, (PBYTE)(stDictHT[i].pTerm),
                                           &ulLen, NULL, &ulUserLen ) );
      if ( !usRc )
      {
        /************************************************************/
        /* Check if key matchs our pattern/compond, if not use next */
        /* wildcard term instead                                    */
        /************************************************************/
        BOOL fMatch;

        if ( fCompound )
        {
          fMatch = QDAMMatchCompound( stDictHT[i].pTerm, pPattern );
        }
        else
        {
          if ( !UtlMatchStringsW( stDictHT[i].pTerm, pPattern, &fMatch ) )
          {
            usRc = BTREE_NO_ROOM;
          } /* endif */
        } /* endif */

        if ( !usRc && !fMatch )
        {
          ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
          usRc = DamBTreeRc(
                QDAMDictNextWildLocal( DamRec[stDictHT[i].usHandle].pDamBTree,
                                       pPattern, fCompound,
                                       (PBYTE)(stDictHT[i].pTerm), &ulLen ) );
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* in case of EOF condition reset usRc and init the returned    */
      /* term                                                         */
      /****************************************************************/
      if ( usRc == LX_EOF_ASD )
      {
        usRc = 0;
        stDictHT[i].pTerm[0] = EOS;
      }
      else if (( usRc == LX_WRD_NT_FND_ASD )
             && (DamRec[stDictHT[i].usHandle].pDamBTree->sCurrentIndex != -1))
      {
        /****************************************************************/
        /* get next possible term...                                    */
        /****************************************************************/
        ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
        usRc = DamBTreeRc(
                QDAMDictCurrent( DamRec[stDictHT[i].usHandle].pDamBTree,
                                 (PBYTE) (stDictHT[i].pTerm),
                                 &ulLen, NULL,  &ulUserLen) );
        /****************************************************************/
        /* if term is still too small, go forward  ...                  */
        /****************************************************************/
        if ( !usRc &&
            ( QDAMKeyCompare( DamRec[stDictHT[i].usHandle].pDamBTree,
                              stDictHT[i].pTerm, pTerm ) < 0 ))
        {
          ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
          ulUserLen = 0;
          usRc = DamBTreeRc(
                  QDAMDictNextWildLocal( DamRec[stDictHT[i].usHandle].pDamBTree,
                                         pPattern, fCompound,
                                         (PBYTE)stDictHT[i].pTerm, &ulLen ) );
          if ( usRc == LX_EOF_ASD )
          {
            usRc = 0;
            stDictHT[i].pTerm[0] = EOS;
          } /* endif */
        } /* endif */
      } /* endif */
      i++;
    } /* endwhile */

    /********************************************************************/
    /* get the terms                                                    */
    /********************************************************************/
    usTermCnt = 0;
    while ( !usRc && (usTermCnt < usNumTerms) )
    {
      usRc = GetNextWildTerm( stDictHT, pPattern, fCompound, chTerm );

      /****************************************************************/
      /* For pattern search: check if the current term does not       */
      /* start with the evaluated end string                          */
      /****************************************************************/
      if ( !usRc && !fCompound && (pEndString != NULL) )
      {
        if ( memicmp( (PBYTE)chTerm, (PBYTE)pEndString,
                       usEndLen * sizeof(CHAR_W) ) != 0 )
        {
          // term seems to be out of our search range, simulate an
          // end-of-dictionary condition
          usRc = LX_EOF_ASD;
        } /* endif */
      } /* endif */

      /******************************************************************/
      /* copy smallest term into provided area and fill next            */
      /*********************************+*********************************/
      if ( !usRc )
      {
        usTermCnt++;                     // we filled in another term

        ulLen = UTF16strlenCHAR( chTerm );
        /******************************************************************/
        /* if we have identified a term ...                               */
        /******************************************************************/
        if ( ulLen )
        {
          ulLen++;
          if ( ulLen < ulBufLen )
          {
            memcpy( (PBYTE)pucBuffer, (PBYTE)chTerm, ulLen * sizeof(CHAR_W) );
            pucBuffer += ulLen;
            chTerm[0] = EOS;             // init for new term
          }
          else
          {
            usRc = BTREE_BUFFER_SMALL;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* free the allocated resources                                     */
  /********************************************************************/
  i = 0;
  if ( pEndString ) UtlAlloc( (PVOID *)&pEndString, 0L, 0L, NOMSG );
  while ( stDictHT[i].pTerm )
  {
    UtlAlloc( (PVOID *)&(stDictHT[i].pTerm), 0L, 0L, NOMSG );
    i++;
  } /* endwhile */

  return( usRc );
} /* end of function QDamWildCardList */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetNextWildTerm                                          |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       This function will work on a (list of) dictionaries and  |
//|                   return the next matching term.                           |
//+----------------------------------------------------------------------------+
//|Parameters:        PDICTHANDLETERM pstDictHT    list of dictionaries        |
//|                   PSZ             pPattern     pointer to search pattern   |
//|                   BOOL            fCompound    compound search flag        |
//|                   PUCHAR          pchTerm      pointer to found term       |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of BTREE functions                           |
//+----------------------------------------------------------------------------+
//|Function flow:     loop thru all associated dictionaries and return         |
//|                    the smallest term.                                      |
//+----------------------------------------------------------------------------+
static USHORT
GetNextWildTerm
(
  PDICTHANDLETERM pstDictHT,           // list of dictionaries
  PSZ_W           pPattern,            // pointer to search pattern
  BOOL            fCompound,           // compound search flag
  PSZ_W           pchTerm              // pointer to found term
)
{
  USHORT  usRc = 0;                    // success indicator
  USHORT  usAct = 0;                   // active term
  USHORT  i;                           // index
  SHORT   sComp;                       // compare value between terms
  ULONG   ulLen;                       // length of headterm

  *pchTerm = EOS;                      // init values
  i = 0;

  /******************************************************************/
  /* find out smallest term ...                                     */
  /******************************************************************/
  while ( ((pstDictHT+i)->usHandle) && !usRc )
  {
    if ( (pstDictHT+i)->pTerm[0])
    {
       sComp =  QDAMKeyCompare( DamRec[(pstDictHT+i)->usHandle].pDamBTree,
                                pchTerm, (pstDictHT+i)->pTerm );
       if ( (sComp > 0) || (*pchTerm == EOS) )
       {
         usAct = i;
         UTF16strcpy( pchTerm, (pstDictHT+i)->pTerm );
       }
       else if (sComp == 0 )
       {
         /***********************************************************/
         /* we have already a matching term -- go ahead with next   */
         /* term                                                    */
         /***********************************************************/
         (pstDictHT+i)->pTerm[0] = '\0';
         ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
         usRc = DamBTreeRc(
                 QDAMDictNextWildLocal( DamRec[pstDictHT[i].usHandle].pDamBTree,
                                        (PSZ_W)pPattern, fCompound,
                                        (PBYTE)pstDictHT[i].pTerm, &ulLen ) );

         /****************************************************************/
         /* in case of EOF condition reset usRc and init the returned    */
         /* term                                                         */
         /****************************************************************/
         if ( usRc == LX_EOF_ASD)
         {
           usRc = 0;
           (pstDictHT+i)->pTerm[0] = EOS;
         } /* endif */
       } /* endif */
    } /* endif */
    i++;
  } /* endwhile */

  /********************************************************************/
  /* get next term in the dict (if we found a matching one )          */
  /********************************************************************/
  if ( !usRc && *pchTerm )
  {
    /******************************************************************/
    /* get a new term for the dictionary we currently used            */
    /******************************************************************/
    (pstDictHT+usAct)->pTerm[0] = '\0';
    ulLen =  HEADTERM_SIZE * sizeof(CHAR_W);
    usRc = DamBTreeRc(
                 QDAMDictNextWildLocal(
                                   DamRec[pstDictHT[usAct].usHandle].pDamBTree,
                                   (PSZ_W)pPattern, fCompound,
                                   (PBYTE)pstDictHT[usAct].pTerm, &ulLen ) );

    /****************************************************************/
    /* in case of EOF condition reset usRc and init the returned    */
    /* term                                                         */
    /****************************************************************/
    if ( usRc == LX_EOF_ASD)
    {
      usRc = 0;
      (pstDictHT+usAct)->pTerm[0] = EOS;
    } /* endif */
  } /* endif */
  return usRc;
} /* end of function GetNextWildTerm */

// get nect dictionary of an association list
USHORT DAMGetNextDict
(
  USHORT  usHandle                     /* in  - dictionary handle   */
)
{
  usHandle = DamRec[usHandle].usNextHandle ;
  return( usHandle );
} /* end of function DAMGetNextDict */


BOOL APIENTRY DllEntryPoint
(
  HINSTANCE   hInstDll,                // handle of library instance
  DWORD      fdwReason,         // reason for calling function
  LPVOID      lpvReserved         // reserved
)
{
  hInstDll; lpvReserved;
  switch ( fdwReason )
  {
   case DLL_PROCESS_ATTACH:
      // code for first load of DLL
      memset( &DamRec[0], 0, sizeof(DAM2QDAM) * MAX_NUM_DICTS );
      memset( &QDAMDict[0], 0, sizeof(QDAMDICT) * MAX_NUM_DICTS );
      fDamInit = FALSE;
      break;
   case DLL_THREAD_ATTACH:
      // code for additional threads using/loading DLL
      break;
   case DLL_THREAD_DETACH:
      // cleanup for single process
      break;
   case DLL_PROCESS_DETACH:
      // cleanup/unload of DLL
      {
     USHORT i;
     // force a close of every (still open dict and memory)
     for (i=0; i<MAX_NUM_DICTS; i++ )
     {
       if ( DamRec[i].pDamBTree )
       {
         QDAMDictCloseLocal ( DamRec[i].pDamBTree );
       } /* endif */
     } /* endfor */
     memset(&DamRec[0], 0, sizeof(DAM2QDAM) * MAX_NUM_DICTS);
     fDamInit = FALSE;
      }
      break;
  }
  return( TRUE );
} /* end of function DllEntryPoint */


// the following old functions are needed for organize of an old dict!!
void DAMLINK
NlpNxtTermAsd
(
   USHORT usHandle,
   USHORT usUser,
   PUCHAR pMatch,
   PULONG pulNum,
   PULONG pulLen,
   PUSHORT pusDict,
   PUSHORT pusRc
)
{
   ULONG  ulLen = 0;
   ULONG  ulLen1 = HEADTERM_SIZE;
   CHAR   szMatch[ HEADTERM_SIZE ];
   ULONG  ulLen2;                      // length of second data param.
   USHORT usTmpHandle;                 // 2.nd handle in case of assoc
   USHORT usRc;
   BOOL   fAssoc = FALSE;
   SHORT  sCompare;                    // compare value


   usUser; pulNum;
   *pusDict = usHandle;


#if defined(MEASURE)
  ulBegin = pGlobInfoSeg->msecs;
#endif
   // is it a associate handle ????
   if ( ! DamRec[usHandle].pDamBTree )
   {
      usHandle = DamRec[usHandle].usNextHandle ;
      fAssoc = TRUE;
   } /* endif */

   /*******************************************************************/
   /* skip corrupted entries -- this is only useful in case we are    */
   /* dealing with organizing the dictionary                          */
   /* This functionality has to be considered only for stand-alone    */
   /* dictionaries - association are not interesting during an        */
   /* organize...                                                     */
   /*******************************************************************/
   *pusRc = LX_RENUM_RQD_ASD;
   while ( *pusRc == LX_RENUM_RQD_ASD )
   {
     /******************************************************************/
     /* disable corruption flag to allow get of data in case dictionary*/
     /* is corrupted                                                   */
     /******************************************************************/
     PBTREE pBT = DamRec[usHandle].pDamBTree;
     BOOL   fCorrupted = pBT->pBTree->fCorrupted;
     pBT->pBTree->fCorrupted = FALSE;       // reset corruption flag
     *pMatch = '\0';
     ulLen1 = HEADTERM_SIZE;
     // typecast to PSZ_W to use new functions!
     *pusRc = DamBTreeRc( QDAMDictNext( DamRec[usHandle].pDamBTree, (PSZ_W) pMatch,
                                        &ulLen1, NULL,  &ulLen) );
     pBT->pBTree->fCorrupted = fCorrupted;
   } /* endwhile */

   if ( (*pusRc == LX_RC_OK_ASD) || ( *pusRc == LX_EOF_ASD ) )
   {
     /*******************************************************************/
     /* still associations left to be looked after ....                 */
     /*******************************************************************/
     usTmpHandle = DamRec[usHandle].usNextHandle;

     while ( fAssoc && usTmpHandle )
     {
       ulLen1 = HEADTERM_SIZE;
       usRc = DamBTreeRc( QDAMDictNext(DamRec[usTmpHandle].pDamBTree, (PSZ_W)szMatch,
                                          &ulLen1, NULL,  &ulLen2) );

       /*****************************************************************/
       /* if term found compare it with term of other dictionary,       */
       /*  use the alphabetical first one and reset the other one       */
       /* IF they are the same, do NO reset                           */
       /*****************************************************************/
       if ( usRc == LX_RC_OK_ASD )
       {
         /*************************************************************/
         /* set sCompare depending on real value or EOF condition     */
         /*************************************************************/
         if ( *pusRc != LX_EOF_ASD  )
         {
           sCompare = (SHORT)stricmp( (PSZ)pMatch, szMatch );
         }
         else
         {
           sCompare = 1;
         } /* endif */

         if ( sCompare > 0 )
         {
           strcpy( (PSZ)pMatch, szMatch );
           ulLen = ulLen2;
           *pusDict = usTmpHandle;
           /************************************************************/
           /* first one reached end                                    */
           /************************************************************/
           if ( *pusRc == LX_EOF_ASD )
           {
             *pusRc = usRc;
           } /* endif */
           /**************************************************************/
           /* reset the not used part (don't care about errors - we      */
           /* will get them on the next try... )                         */
           /**************************************************************/
           ulLen1 = HEADTERM_SIZE;
           QDAMDictPrev( DamRec[usHandle].pDamBTree,
                         (PSZ_W) szMatch, &ulLen1, NULL,  &ulLen2) ;
         }
         else if ( sCompare < 0 )
         {
           /**************************************************************/
           /* reset the not used part (don't care about errors - we      */
           /* will get them on the next try... )                         */
           /**************************************************************/
           ulLen1 = HEADTERM_SIZE;
           QDAMDictPrev( DamRec[usTmpHandle].pDamBTree,
                         (PSZ_W) szMatch, &ulLen1, NULL,  &ulLen2) ;
         } /* endif */

       }
       else if ( usRc == LX_EOF_ASD )
       {
         ulLen1 = HEADTERM_SIZE;
         QDAMDictPrev(DamRec[usTmpHandle].pDamBTree,
                      (PSZ_W) szMatch, &ulLen1, NULL,  &ulLen2 );
         /***************************************************************/
         /* reset our primary dictionary if it is at EOF, too           */
         /***************************************************************/
         if ( *pusRc == LX_EOF_ASD )
         {
           ulLen1 = HEADTERM_SIZE;
           QDAMDictPrev(DamRec[usHandle].pDamBTree,
                        (PSZ_W) szMatch, &ulLen1, NULL,  &ulLen2);
         } /* endif */
       } /* endif */
       usTmpHandle = DamRec[usTmpHandle].usNextHandle;
     } /* endwhile */
   } /* endif */
#if defined(MEASURE)
  ulFind += (pGlobInfoSeg->msecs - ulBegin);
#endif
   *pulLen = ulLen;

  if ( *pusRc != LX_RC_OK_ASD )
  {
    ERREVENT( NLPNXTTERMASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}

void DAMLINK
NlpRetEntryAsd
(
   USHORT usHandle,
   USHORT usUser,
   PUCHAR pTerm,
   PULONG pulNum,
   PUCHAR pData,
   PULONG pulDataLen,
   PUSHORT pusDict,
   PUSHORT pusRc
)
{
   ULONG ulLen;
   ULONG ulKeyLen = HEADTERM_SIZE;

   usUser; pulNum;

   *pusDict = usHandle;
   ulLen = *pulDataLen;
#if defined(MEASURE)
  ulBegin  = pGlobInfoSeg->msecs;
#endif
  *pusRc = DamBTreeRc( QDAMDictCurrent( DamRec[usHandle].pDamBTree, (PBYTE)pTerm,
                                        &ulKeyLen, pData, &ulLen) );
#if defined(MEASURE)
  ulGet += (pGlobInfoSeg->msecs - ulBegin);
#endif
  *pulDataLen = ulLen;
  if ( *pusRc != LX_RC_OK_ASD)
  {
    ERREVENT( NLPRETENTRYASD_LOC, INTFUNCFAILED_EVENT, *pusRc );
  } /* endif */
}

PBTREE DamGetBTreeFromDamRec(USHORT usIndex)
{
	return DamRec[usIndex].pDamBTree;
}