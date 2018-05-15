//+----------------------------------------------------------------------------+
//|EQFX1API.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:       R.Jornitz                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  EQF_API contains the functions described in the document      |
//|              'Troja Editor API'                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|     EQFGETSOURCE    displays the source of prop window...                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+

// activate the following define to enable logging
//#define API_LOGGING


#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions

  #define DLLIMPORTRESMOD         // resource module handle imported from DLL

#include <eqf.h>                  // General Translation Manager include file

#include "EQFB.ID"                // Translation Processor IDs
#include <eqftpi.h>               // private Translation Processor include file


#include <eqfdoc00.h>

#include "eqffol.h"
#include "eqfhlog.h"                   // private history log include file
#include "eqfsetup.h"

VOID EQFXReadBuffer(PSZ_W, PTOKENENTRY, PLOADEDTABLE,
                    PUSHORT, PSEGFLAGS,
                    PUSHORT, PUSHORT, PUSHORT, PUSHORT, PCOUNTFLAG  );
USHORT EQFXWriteToBuffer (PSZ_W, USHORT, PLOADEDTABLE, USHORT,
                          USHORT, USHORT, USHORT, USHORT,
                          PSEGFLAGS, PCOUNTFLAG);
VOID EQFFreeDoc ( PTBDOCUMENT );
#define GETNUMBER( pszTarget, usValue ) \
{                                   \
   usValue = 0;                     \
   if ( *pszTarget++ )              \
   {                                \
      while ( isdigit(*pszTarget) ) \
      {                             \
         usValue = (usValue * 10) + (*pszTarget++ - '0'); \
      } /* endwhile */              \
   } /* endif */                    \
}

#define API_INBUF_SIZE    8192     //size of input buffer
//#define API_INBUF_SIZE    128     //for test only size of input buffer


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBuildSegAttr                                          |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBUILDSEGATTR(PSTEQFGEN, PSZ, PSEGINFO)                |
//+----------------------------------------------------------------------------+
//|Description:       merge new segment info with old buffer                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   pstEQFGen                                    |
//|                   PSZ         pszBuffer                                    |
//|                   PSEGINFO    pSegInfo                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   ERROR_STORAGE Utlalloc failed                            |
//|                   ERROR_INVALID_PARM  Tagtable load failed                 |
//|                   EQFRS_AREA_TOO_SMALL Buffer too small                    |
//+----------------------------------------------------------------------------+
//|Function flow:     fill input info into buffer                              |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFBUILDSEGATTR
(
   PSZ       pszBuffer,
   USHORT    usBufLen,
   PSEGINFO  pSegInfo
)
{
  USHORT     usRc = 0;
  CHAR_W     chBufferW[256];

  usRc = EQFBUILDSEGATTRW( &chBufferW[0], (USHORT)min( usBufLen, 256 ), pSegInfo );
  Unicode2ASCII( &chBufferW[0], pszBuffer, 0L );

  return usRc;
}


__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFBUILDSEGATTRW
(
   PSZ_W     pszBuffer,
   USHORT    usBufLen,
   PSEGINFO  pSegInfo
)
{
  USHORT        usRc = 0;                                  // return code
  BOOL          fCopied = FALSE;
  USHORT        usOldSrcWords = 0;
  USHORT        usOldTgtWords = 0;
  USHORT        usOldModWords = 0;
  USHORT        qOldStatus = 0;
  SEGFLAGS      OldSegFlags;
  COUNTFLAG     OldCountFlag;
  USHORT        usSrcWords = 0;
  USHORT        usTgtWords = 0;
  USHORT        usModWords = 0;
  USHORT        qStatus = 0;
  SEGFLAGS      SegFlags;
  COUNTFLAG     CountFlag;
  USHORT        usSegNum = 0;
  PTOKENENTRY   pTokBuf = NULL;
  PLOADEDTABLE  pLoadedTable = NULL;
  /********************************************************************/
  /* retrieve from input pszXBuffer previous values                   */
  /********************************************************************/
  memset (&OldSegFlags, 0, sizeof(SEGFLAGS));

  if (UtlAlloc((PVOID *) &pTokBuf, 0L,
           (LONG) TOK_BUFFER_SIZE, ERROR_STORAGE ))
  {
     usRc = (USHORT) TALoadTagTable( DEFAULT_QFTAG_TABLE,
                                (PLOADEDTABLE *) &pLoadedTable,
                                TRUE, FALSE );
     if (usRc != NO_ERROR )
     {
       usRc = EQFRS_INVALID_PARM;
     } /* endif */
  }
  else
  {
    usRc = ERROR_STORAGE;
  } /* endif */
  if (!usRc )
  {
    memset (&OldCountFlag, 0, sizeof(COUNTFLAG));

    EQFXReadBuffer(pszBuffer, pTokBuf, pLoadedTable,
                     &qOldStatus, &OldSegFlags, &usSegNum,
                     &usOldSrcWords, &usOldTgtWords, &usOldModWords,
                     &OldCountFlag );
  } /* endif */
  /********************************************************************/
  /* set new info by merging content of old buffer and pSegInfo       */
  /********************************************************************/
  memset (&CountFlag, 0, sizeof(COUNTFLAG));
  memset (&SegFlags, 0, sizeof(SEGFLAGS));

  switch ( pSegInfo->sCurrent )
  {
    case SET_CURRENT:
      SegFlags.Current = TRUE;
      break;
    case NOT_CURRENT:
      SegFlags.Current = FALSE;
      break;
    case UNCHANGED_CURRENT:
    default:
      SegFlags.Current = OldSegFlags.Current;
      break;
  } /* endswitch */

  switch ( pSegInfo->sNoCount )
  {
    case SET_NOCOUNT:
      SegFlags.NoCount = TRUE;
      break;
    case NOT_NOCOUNT:
      SegFlags.NoCount = FALSE;
      break;
    case UNCHANGED_NOCOUNT:
    default:
      SegFlags.NoCount = OldSegFlags.NoCount;
      break;
  } /* endswitch */
  switch ( pSegInfo->usTyped )
  {
    case SET_TYPED:
      SegFlags.Typed = TRUE;
      break;
    case NOT_TYPED:
      SegFlags.Typed = FALSE;
      break;
    case UNCHANGED_TYPED:
    default:
      SegFlags.Typed = OldSegFlags.Typed;
      break;
  } /* endswitch */
  switch ( pSegInfo->usCopied)
  {
    case SET_COPIED:
      SegFlags.Copied = TRUE;
      break;
    case NOT_COPIED:
      SegFlags.Copied = FALSE;
      break;
    case UNCHANGED_COPIED:
    default:
      SegFlags.Copied = OldSegFlags.Copied;
      break;
  } /* endswitch */
  switch ( pSegInfo->sStatus)
  {
    case XLATED_STATUS:
      qStatus = QF_XLATED;
      break;
    case TOBE_STATUS:
      qStatus = QF_TOBE;
      break;
    case NOP_STATUS:
      qStatus = QF_NOP;
      break;
    case ATTR_STATUS:
      qStatus = QF_ATTR;
      break;
    case UNCHANGED_STATUS:
    default:
      qStatus = qOldStatus;
      break;
  } /* endswitch */

  /********************************************************************/
  /* fill new CountFlag                                               */
  /********************************************************************/
  memcpy(&CountFlag, &OldCountFlag,sizeof(COUNTFLAG));
  if ((qOldStatus == QF_TOBE)|| (qOldStatus == QF_ATTR)
      || (qOldStatus == QF_CURRENT)  )
  {
    if (pSegInfo->usTypeExist )
    {
      if (pSegInfo->usTypeExist & EXACT_PROP )
      {
        CountFlag.ExactExist = TRUE;
      }
      else if (pSegInfo->usTypeExist & REPLACE_PROP )
      {
        CountFlag.ReplExist = TRUE;
      }
      else if (pSegInfo->usTypeExist & FUZZY_REPLACE_PROP )
      {
        CountFlag.FuzzyExist = TRUE;
        //P019381: CountFlag.ReplExist = TRUE;
      }
      else if (pSegInfo->usTypeExist & FUZZY_PROP )
      {
        CountFlag.FuzzyExist = TRUE;
      }
      else if (pSegInfo->usTypeExist & MACHINE_TRANS_PROP )
          {
            CountFlag.MachExist = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */
  /********************************************************************/
  /* if proposal has been copied previously already, do not set       */
  /* new "copied" flag                                                */
  /********************************************************************/

  if (!OldSegFlags.Copied && !OldCountFlag.EditAutoSubst
               && !OldCountFlag.AnalAutoSubst )
  {
    if (pSegInfo->usTypeCopied & EXACT_PROP )
    {
      CountFlag.ExactCopy = TRUE;
      fCopied = TRUE;
    }
    else if (pSegInfo->usTypeCopied & FUZZY_PROP )
    {
      CountFlag.FuzzyCopy = TRUE;
      fCopied = TRUE;
    }
    else if (pSegInfo->usTypeCopied & REPLACE_PROP )
    {
      CountFlag.ReplCopy = TRUE;
      fCopied = TRUE;
    }
    else if (pSegInfo->usTypeCopied & FUZZY_REPLACE_PROP )
    {
      CountFlag.FuzzyCopy = TRUE;
      // P019381: CountFlag.ReplCopy = TRUE;
      fCopied = TRUE;
    }
    else if (pSegInfo->usTypeCopied & MACHINE_TRANS_PROP )
        {
          CountFlag.MachCopy = TRUE;
          fCopied = TRUE;
    } /* endif */
  } /* endif */
  /********************************************************************/
  /* fill new usSrcWords, usTgtWords, usModWords                      */
  /********************************************************************/
  usSrcWords = usOldSrcWords;
  if (usSrcWords == 0 )
  {
    usSrcWords = pSegInfo->usSrcWords;
  } /* endif */
  if (qStatus == QF_XLATED )
  {
    if (pSegInfo->usTgtWords )
    {
      usTgtWords = pSegInfo->usTgtWords;
    }
    else
    {
      usTgtWords = usOldTgtWords;
    } /* endif */
  } /* endif */
  if ((usOldModWords == 0) &&
       (   (qOldStatus == QF_TOBE) || (qOldStatus == QF_ATTR)
        || (qOldStatus == QF_CURRENT) ) )
  {
    usModWords = pSegInfo->usModWords;
    /******************************************************************/
    /* if usModWords still 0 and not an exact prop: then no prop      */
    /* exists and ModWords must be set here                           */
    /******************************************************************/
    if (!CountFlag.ExactExist && !CountFlag.MachExist
                              && (usModWords == 0) )
    {
      usModWords = usSrcWords;
    } /* endif */
    if (usModWords > usSrcWords )
    {
      usModWords = usSrcWords;
    } /* endif */
  } /* endif */
    /******************************************************************/
    /* consistency check !!! should we return errors?                 */
    /******************************************************************/
    if ((pSegInfo->usSegNum != usSegNum) && (usSegNum == 0))
    {
      /****************************************************************/
      /* buffer is all empty(0) and call is used to fill it...        */
      /****************************************************************/
      usSegNum = pSegInfo->usSegNum;
    } /* endif */
    if (qStatus != QF_XLATED )
    {
      /****************************************************************/
      /* segment has been untranslated      !!                        */
      /****************************************************************/
      usTgtWords = 0;
      usModWords = 0;
      SegFlags.Typed = FALSE;
      SegFlags.Copied = FALSE;
      CountFlag.FuzzyCopy = FALSE;
      CountFlag.ExactCopy = FALSE;
      CountFlag.ReplCopy = FALSE;
      CountFlag.MachCopy = FALSE;
    } /* endif */
    if (fCopied && !SegFlags.Copied )
    {
      SegFlags.Copied = TRUE;
    } /* endif */
    /********************************************************************/
    /* write new filled attributes into buffer                          */
    /********************************************************************/
    if (!usRc )
    {
      usRc = EQFXWriteToBuffer (pszBuffer, usBufLen, pLoadedTable,
                                usSegNum, qStatus,
                                usSrcWords, usTgtWords, usModWords,
                                &SegFlags, &CountFlag);
    } /* endif */

    if ( pTokBuf )   UtlAlloc((PVOID *)&pTokBuf, 0L, 0L, NOMSG );
    if ( pLoadedTable)
    {
      TAFreeTagTable(pLoadedTable );
    }
  return usRc;
}                                      // end 'EQFBUILDSEGATTR'





//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXReadBuffer                                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXReadBuffer(PSZ, PTOKENENTRY, PLOADEDTABLE,           |
//|                                  PUSHORT, PSEGFLAGS, PUSHORT,              |
//|                               PUSHORT, PUSHORT, PUSHORT, PCOUNTFLAG)       |
//+----------------------------------------------------------------------------+
//|Description:       read buffer and fill fields                              |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ       pszBuffer,                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Sample:            in:pszBuffer =":QFX N=27 CURRENT S=1 X=000050006000000"  |
//|                   out: usSegNum = 27                                       |
//|                        usSrcWords = 5                                      |
//|                        usTgtWords = 6                                      |
//|                        qStatus                                             |
//|                        SegFlags.Current, SegFlags.Typed CountFlag=0.....   |
//+----------------------------------------------------------------------------+
//|Function flow:     Fast tokenize buffer                                     |
//|                   go thru tokens and fill info in output fields            |
//+----------------------------------------------------------------------------+
VOID
EQFXReadBuffer
(
    PSZ_W        pszBuffer,
    PTOKENENTRY  pTokBuf,
    PLOADEDTABLE pLoadedTable,
    PUSHORT      pusOldStatus,
    PSEGFLAGS    pOldSegFlags,
    PUSHORT      pusSegNum,
    PUSHORT      pusOldSrcWords,
    PUSHORT      pusOldTgtWords,
    PUSHORT      pusOldModWords,
    PCOUNTFLAG   pOldCountFlag
)
{
    USHORT       usValue = 0;
    USHORT       usColPos = 0;
    PSZ_W        pRest = NULL;
    PSZ_W        pszTarget;
    SEGFLAGS     SegFlags;
    PTOKENENTRY  pTok;

    memset (&SegFlags, 0, sizeof(SEGFLAGS));

    TAFastTokenizeW( pszBuffer, pLoadedTable,
                    TRUE, &pRest, &usColPos,
                    (PTOKENENTRY) pTokBuf,
                    TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

    *pusOldStatus = QF_TOBE;            // default if not spec.

    // convert tokens to entries in segment table
    pTok = (PTOKENENTRY) pTokBuf;
    while (pTok->sTokenid != ENDOFLIST)
    {
       switch ( pTok->sTokenid )
       {
          case QFF_TAG: *pusOldStatus = QF_TOBE; break;
          case QFN_TAG: *pusOldStatus = QF_NOP; break;
          case QFX_TAG: *pusOldStatus = QF_XLATED; break;
          case QFA_TAG: *pusOldStatus = QF_ATTR; break;
          case QFC_TAG: *pusOldStatus = QF_CURRENT; break;
          case QFJ_TAG: *pusOldStatus = QF_JOINED; break;
          case QFS_TAG: *pusOldStatus = QF_SPLIT; break;
             break;
          case NONE_TAG:             // it is similar to text
          case TEXT_TOKEN:
          // should not occurr, ignore them
              break;
          case CURRENT_ATTR: SegFlags.Current = TRUE; break;
          case MARK_ATTR:    SegFlags.Marked = TRUE; break;
          case N_ATTR:
               pszTarget = pTok->pDataStringW;
               while ( *pszTarget && (*pszTarget != '=') )
               {
                  pszTarget++;
               } /* endwhile */
               GETNUMBER( pszTarget, *pusSegNum );
               break;
          case JOIN_ATTR:
               pszTarget = UTF16strchr( pTok->pDataStringW, '=' );
               GETNUMBER( pszTarget, usValue );
               switch ( usValue )
               {
                  case 1:
                     SegFlags.JoinStart = TRUE;
                     break;
                  case 2:
                     SegFlags.Joined    = TRUE;
                     break;
               } /* endswitch */
               break;
          case STATUS_ATTR:
               pszTarget = UTF16strchr( pTok->pDataStringW, '=' );
               GETNUMBER( pszTarget, usValue );
               switch ( usValue )
               {
                  case 1:
                     SegFlags.Typed = TRUE;
                     break;
                  case 2:
                     SegFlags.Typed = TRUE;
                     SegFlags.Copied = TRUE;
                     break;
                  case 3:
                     SegFlags.Copied = TRUE;
                     break;
               } /* endswitch */
               break;

          case NOCOUNT_ATTR:
               SegFlags.NoCount = TRUE;
               break;

          case COUNT_ATTR:
               pszTarget = UTF16strchr( pTok->pDataStringW, '=' );
               EQFBGetHexNumberW( pszTarget+1, &usValue );
               *((PUSHORT)(pOldCountFlag)) = usValue;
               EQFBGetHexNumberW( pszTarget+5, pusOldSrcWords );
               EQFBGetHexNumberW( pszTarget+9, pusOldTgtWords );
               EQFBGetHexNumberW( pszTarget+13, pusOldModWords );
               break;
          default:
             // ignore unknown tokens
             break;
       } /* endswitch */
       pTok++;
    } /* endwhile */

    memcpy( pOldSegFlags, &SegFlags, sizeof(SEGFLAGS) );
  return ;
}                                      // end 'EQFXReadBuffer  '

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXWriteToBuffer                                        |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXWriteToBuffer(PSZ, USHORT, PLOADEDTABLE,             |
//|                                         USHORT, PSEGINFO, PSEGFLAGS,       |
//|                                 PCOUNTFLAG )                               |
//+----------------------------------------------------------------------------+
//|Description:       write info in fields into buffer                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ          pszBuffer,                                  |
//|                   USHORT       usBufLen,                                   |
//|                   PLOADEDTABLE pLoadedTable,                               |
//|                   USHORT       usSegNum,                                   |
//|                   USHORT       qOldStatus,                                 |
//|                   PSEGINFO     pSegInfo,                                   |
//|                   PSEGFLAGS    pOldSegFlags,                               |
//|                   PCOUNTFLAG   pCountFlag                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Return        :    usRc 0 ok                                                |
//|                   EQFRS_AREA_TOO_SMALL   buffer too small                  |
//+----------------------------------------------------------------------------+
//|Sample:            in:                                                      |
//|                   out: pszBuffer                                           |
//+----------------------------------------------------------------------------+
//|Function flow:     Fast tokenize buffer                                     |
//+----------------------------------------------------------------------------+
USHORT
EQFXWriteToBuffer
(
  PSZ_W         pszBuffer,
  USHORT        usBufLen,
  PLOADEDTABLE  pLoadedTable,
  USHORT        usSegNum,
  USHORT        qStatus,
  USHORT        usSrcWords,
  USHORT        usTgtWords,
  USHORT        usModWords,
  PSEGFLAGS     pSegFlags,
  PCOUNTFLAG    pCountFlag
)
{

   CHAR_W    szMarkAttr[ATTRIBUTE_SIZE];        // for 'MARK' attribute
   CHAR_W    szCurrentAttr[ATTRIBUTE_SIZE];     // for 'CURRENT' attribute
   CHAR_W    szNAttr[ATTRIBUTE_SIZE];           // for preprocessed N= attr
   CHAR_W    szJoinAttr[ATTRIBUTE_SIZE];        // for preprocessed JOIN= attr
   CHAR_W    szStatusAttr[ATTRIBUTE_SIZE];      // for preprocessed S= attr
   CHAR_W    szCountAttr[ATTRIBUTE_SIZE];       // for preprocessed X= attr

   CHAR_W    szNoCountAttr[ATTRIBUTE_SIZE];     // name of "NOCOUNT" attribute
   PBYTE     pByte;                      // helper pointer
   PTAGTABLE pTagTable;                  // ptr to act. QF tagtable
   PTAG      pTag;
   PSZ       pTagNames;
   SHORT     sToken = 0;
   LONG      lLen;
   USHORT    usRc = 0;
   SEGFLAGS  CurSegFlags;
   CHAR_W    szCurBuf_W[100];
   CHAR_W    szTagBuf_W[100];
   PSZ_W     pszCurBuf = szCurBuf_W;

  memcpy( &CurSegFlags, pSegFlags, sizeof(SEGFLAGS) );
  memset(szCurBuf_W, 0, sizeof(szCurBuf_W));

  pTagTable = pLoadedTable->pTagTable;
  pByte = (PBYTE) (PVOID)pTagTable;
  pTag = (PTAG) ( pByte + pTagTable->stFixTag.uOffset);
  pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);
  // address QF tag table names
  EQFBFillWriteAttrW( pLoadedTable,
                     szMarkAttr, szNoCountAttr,
                     szCurrentAttr, szJoinAttr,
                     szNAttr, szStatusAttr, szCountAttr );

    switch ( qStatus )
    {
      case QF_TOBE:    sToken = QFF_TAG;  break;
      case QF_NOP:     sToken = QFN_TAG;  break;
      case QF_XLATED:  sToken = QFX_TAG;  break;
      case QF_ATTR:    sToken = QFA_TAG;  break;
      case QF_CURRENT: sToken = QFC_TAG;  break;
      case QF_JOINED:  sToken = QFJ_TAG;  break;
      case QF_SPLIT:   sToken = QFS_TAG;  break;
    } /* endswitch */

    // write start tag to file
    ASCII2Unicode( (PSZ)(pTag[sToken].uTagnameOffs + pTagNames), szTagBuf_W, 0L );
    lLen = swprintf( pszCurBuf, L"%s ", szTagBuf_W );
    pszCurBuf += lLen;
    lLen = swprintf( pszCurBuf, szNAttr, usSegNum );
    pszCurBuf += lLen;

    if ( (CurSegFlags.JoinStart || CurSegFlags.Joined) )
    {
      *pszCurBuf++ = BLANK;
      lLen = swprintf( pszCurBuf, szJoinAttr,
                      ( CurSegFlags.JoinStart ) ? 1 : 2 );
      pszCurBuf += lLen;
    } /* endif */
    // write mark attribute to file
    if ( CurSegFlags.Marked )
    {
      *pszCurBuf++ = BLANK;
      UTF16strcpy( pszCurBuf, szMarkAttr );
      pszCurBuf += UTF16strlenCHAR( pszCurBuf );
    } /* endif */
    // write current attribute to file
    if ( CurSegFlags.Current )
    {
      *pszCurBuf++ = BLANK;
      UTF16strcpy( pszCurBuf, szCurrentAttr );
      pszCurBuf += UTF16strlenCHAR( pszCurBuf );
    } /* endif */
    // write mark attribute to file
    if ( CurSegFlags.NoCount )
    {
      *pszCurBuf++ = BLANK;
      UTF16strcpy( pszCurBuf, szNoCountAttr );
      pszCurBuf += UTF16strlenCHAR( pszCurBuf );
    } /* endif */
    // write Status state attribute to file
    /**********************************************************/
    /* if Typed = TRUE and COpied = TRUE -> szStatusAttr =2  */
    /* if Typed = TRUE and Copied =FALSE -> szStatusAttr =1  */
    /* if Typed =FALSE and Copied = TRUE -> szStatusAttr =3  */
    /**********************************************************/
    if ( CurSegFlags.Typed || CurSegFlags.Copied )
    {
      *pszCurBuf++ = BLANK;
      if ( CurSegFlags.Typed )
      {
        if ( CurSegFlags.Copied )
        {
          lLen = swprintf(pszCurBuf, szStatusAttr, 2 );
        }
        else
        {
          lLen = swprintf(pszCurBuf, szStatusAttr, 1 );
        } /* endif */
      }
      else
      {
         lLen = swprintf(pszCurBuf, szStatusAttr, 3 );
      } /* endif */
      pszCurBuf += lLen;

    } /* endif */

    // write count attribute to file
    if ( (*((PUSHORT)(pCountFlag)) == 0) && (usSrcWords == 0) &&
         (usTgtWords == 0) && (usModWords == 0) )
    {
      // nothing to do, count data is empty
    }
    else
    {
      USHORT usCheckSum = EQFBBuildCountCheckSum( *((PUSHORT)(pCountFlag)),
        usSrcWords, usTgtWords, usModWords );
      *pszCurBuf++ = BLANK;
      lLen = swprintf(pszCurBuf, szCountAttr,
                     *((PUSHORT)(pCountFlag)),
                     usSrcWords, usTgtWords, usModWords, usCheckSum );
      pszCurBuf += lLen;
    } /* endif */
    // write tag end character to file
    UTF16strcpy( pszCurBuf, TAG_END_CHAR_W );
    pszCurBuf += UTF16strlenCHAR( pszCurBuf );
    if ( UTF16strlenCHAR( szCurBuf_W ) < usBufLen )
    {
    UTF16strcpy( pszBuffer, szCurBuf_W );
  }
    else
    {
      usRc = EQFRS_AREA_TOO_SMALL;
    } /* endif */
   return(usRc);
  }           // end of EQFXWriteToBuffer

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETTYPE                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETTYPE (PSTEQFGEN, USHORT, BOOL, PUSHORT)            |
//+----------------------------------------------------------------------------+
//|Description:       get the type of the proposal( fuzzy, or replace, or      |
//|                   machine, or exact...)                                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN pstEQFGen,                                     |
//|                   USHORT    usNum,                                         |
//|                   PUSHORT   pusType                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   EQFRS_ENTRY_NOT_AVAIL  out of range                      |
//+----------------------------------------------------------------------------+
//|Function flow:     fill                                                     |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETTYPE
(
   PSTEQFGEN pstEQFGen,
   USHORT    usNum,
   PUSHORT   pusType
)
{
  USHORT        usRc = 0;                                  // return code
  PDOCUMENT_IDA  pDoc = NULL;

  *pusType = 0;

  if (pstEQFGen )
  {
    pDoc = (PDOCUMENT_IDA)pstEQFGen->pDoc;
    if (pDoc && (pDoc->stEQFSab+pDoc->usFI)->fbInUse )
    {
      /******************************************************************/
      /* retrieve type of copied proposal if usNum 1, 2, ...            */
      /* retrieves type of existing prop if usNum = 0                   */
      /******************************************************************/
      *pusType = EqfGetPropState( pstEQFGen, usNum);

    }
    else
    {
      usRc = EQFRS_ENTRY_NOT_AVAIL;
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* error return                                                   */
    /******************************************************************/
    usRc = EQFRS_INVALID_PARM;
  } /* endif */

  return usRc;

}                                      // end 'EQFGETTYPE   '

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFWRITEHISTDOCSAVE                                      |
//+----------------------------------------------------------------------------+
//|Function call:     EQFWRITEHISTDOCSAVE(  PSZ, PSZ)                          |
//+----------------------------------------------------------------------------+
//|Description:       write DOCSAVE history log record                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ       pszDocument,                                   |
//|                   PSZ       pszFolObjName                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   EQFRS_ENTRY_NOT_AVAIL  out of range                      |
//+----------------------------------------------------------------------------+
//|Function flow:     fill                                                     |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFWRITEHISTDOCSAVE
(
   PSZ       pszDocument16,                 // name of document
   PSZ       pszFolObjName16                // object name of folder
)
{
  USHORT        usRc = 0;                                  // return code
  PTBDOCUMENT   pTgtDoc = NULL;             // ptr to document structure
  PTBDOCUMENT   pSrcDoc = NULL;             // ptr to document structure
  CHAR          szFullTgtDocName[MAX_EQF_PATH];   // fully qual. doc. name
  CHAR          szFullSrcDocName[MAX_EQF_PATH];            // fully qual. doc. name
  PSZ           pTemp = NULL;
  PSZ           pFileName = NULL;
  CHAR          szDocObjName[MAX_EQF_PATH];
  CHAR          szDocFormat[MAX_FILESPEC];
  CHAR          szSrcLang[MAX_LANG_LENGTH];
  CHAR          szTgtLang[MAX_LANG_LENGTH];
  PSZ      pszDocument =  pszDocument16;
   PSZ     pszFolObjName =   pszFolObjName16 ;

  // setup fully qualified document path name
  // do not use UtlMakeEQFPath to allow usage from XLATE.EXE
  // environment
  strcpy( szFullTgtDocName, pszFolObjName );
  strcat( szFullTgtDocName, BACKSLASH_STR );
  strcat( szFullTgtDocName, SEGTARGETDIR );
  strcat( szFullTgtDocName, BACKSLASH_STR );
  strcat( szFullTgtDocName, pszDocument );


  // return immediately if there is no target document
  if ( !UtlFileExist( szFullTgtDocName ) )
  {
    return( NO_ERROR );
  } /* endif */

  strcpy( szFullSrcDocName, pszFolObjName );
  strcat( szFullSrcDocName, BACKSLASH_STR );
  strcat( szFullSrcDocName, SEGSOURCEDIR );
  strcat( szFullSrcDocName, BACKSLASH_STR );
  strcat( szFullSrcDocName, pszDocument );

  // return immediately if there is no target document
  if ( !UtlFileExist( szFullSrcDocName ) )
  {
    return( NO_ERROR );
  } /* endif */

  /********************************************************************/
  /* Allocate TBDOCUMENT structure                                    */
  /********************************************************************/
  if ( !UtlAlloc((PVOID *) &pTgtDoc, 0L,
                 (LONG) sizeof(TBDOCUMENT), ERROR_STORAGE ) )
  {
    usRc = ERROR_STORAGE;
  }
  else
  {
    if (!UtlAlloc((PVOID *) &pSrcDoc, 0L,
                  (LONG) sizeof(TBDOCUMENT), ERROR_STORAGE )  )
    {
      usRc = ERROR_STORAGE;
    } /* endif */
  } /* endif */
  if (usRc == NO_ERROR )
  {
    strcpy( pTgtDoc->szDocName, szFullTgtDocName );
    strcpy( pSrcDoc->szDocName, szFullSrcDocName );
  } /* endif */

  /********************************************************************/
  /* Load the QF tag table                                            */
  /********************************************************************/
  if ( usRc == NO_ERROR )
  {
    usRc = TALoadTagTable( QFTAG_TABLE,
                           (PLOADEDTABLE *)&(pTgtDoc->pQFTagTable),
                           TRUE,                  // load internal table
                           TRUE );                // do message handling
    if (usRc != NO_ERROR )
    {
      usRc = EQFRS_INVALID_PARM;
    } /* endif */
  } /* endif */
  if (!usRc )
  {
    pSrcDoc->pQFTagTable = pTgtDoc->pQFTagTable;

    strcpy( szDocObjName, pTgtDoc->szDocName );
    pFileName = UtlSplitFnameFromPath( szDocObjName);   // ptr to filename
    pTemp = UtlGetFnameFromPath( szDocObjName);         // ptr to starget
    //copy filename at position where STARGET was !!
    strcpy( pTemp, pFileName );                         // cpy fname
    DocQueryInfo2( szDocObjName,
                   NULL,             // memory
                   szDocFormat,      // document format
                   szSrcLang,        // source language
                   szTgtLang,        // target language
                   NULL,             // LongName,
                   NULL,             // Alias
                   NULL,             // editor
                   TRUE );
    if ( szDocFormat[0] )
    {
      usRc = TALoadTagTable( szDocFormat,
                        (PLOADEDTABLE *)&(pTgtDoc->pDocTagTable),
                        FALSE,
                        TRUE );           // do message handling
      if (usRc != NO_ERROR )
      {
        usRc = EQFRS_INVALID_PARM;
      } /* endif */
    }
    else
    {
      usRc = EQFRS_INVALID_PARM;
    } /* endif */
    pSrcDoc->pDocTagTable = pTgtDoc->pDocTagTable;
  } /* endif */

  /********************************************************************/
  /* Load the segmented target file into the document structure       */
  /********************************************************************/
  if ( usRc == NO_ERROR )
  {
    // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to DocLang
    pTgtDoc->ulOemCodePage = GetLangCodePage(OEM_CP, szTgtLang);
    pTgtDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, szTgtLang);

    usRc = (USHORT)EQFBFileReadExW( szFullTgtDocName, pTgtDoc,
                                   FILEREAD_SINGLETABLE );
    if (usRc != NO_ERROR )
    {
      usRc = EQFS_FILE_OPEN_FAILED;
    } /* endif */
  } /* endif */
  if ( usRc == NO_ERROR )
  {
    // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to DocLang
    pSrcDoc->ulOemCodePage = GetLangCodePage(OEM_CP, szSrcLang);
    pSrcDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, szSrcLang);

    usRc = (USHORT)EQFBFileReadExW( szFullSrcDocName, pSrcDoc,
                                   FILEREAD_SINGLETABLE );
    if (usRc != NO_ERROR )
    {
      usRc = EQFS_FILE_OPEN_FAILED;
    } /* endif */
  } /* endif */
  pTgtDoc->twin = pSrcDoc;
  /********************************************************************/
  /* Count words and write history log record                         */
  /********************************************************************/
  if ( usRc == NO_ERROR )
  {
    EQFBHistDocSave( szFullTgtDocName, pTgtDoc, DOCAPI_LOGTASK);
  } /* endif */

  /********************************************************************/
  /* Free the document structure                                      */
  /********************************************************************/
  if ( pTgtDoc->pQFTagTable)
  {
    TAFreeTagTable((PLOADEDTABLE)pTgtDoc->pQFTagTable );
  }
  if ( pTgtDoc->pDocTagTable)
  {
    TAFreeTagTable((PLOADEDTABLE)pTgtDoc->pDocTagTable );
  }
  if ( pTgtDoc != NULL)
  {
    EQFFreeDoc(pTgtDoc);
  } /* endif */
  if ( pSrcDoc != NULL)
  {
    EQFFreeDoc(pSrcDoc);
  } /* endif */

  return usRc;
}                                      // end 'EQFWRITEHISTDOCSAVE '

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFFREEDOC                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFFREEDOC(PTBDOCUMENT)                                  |
//+----------------------------------------------------------------------------+
//|Description:       free all                                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pDoc                                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   EQFRS_ENTRY_NOT_AVAIL  out of range                      |
//+----------------------------------------------------------------------------+
//|Function flow:     free all                                                 |
//+----------------------------------------------------------------------------+
VOID
EQFFreeDoc
(
  PTBDOCUMENT pDoc
)
{
    PTBSEGMENTTABLE pSegTable;          // ptr for segment table deleting
    ULONG           ulI, ulJ;           // loop counter
    PTBSEGMENT      pSegment;           // ptr for segment deleting

    UtlAlloc( (PVOID *) &pDoc->pInBuf,  0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pDoc->pTokBuf, 0L, 0L, NOMSG );

    pSegTable = pDoc->pSegTables;
    for ( ulI = 0; ulI < pDoc->ulSegTables; ulI++ )
    {
       pSegment = pSegTable->pSegments;
       for ( ulJ = 0; ulJ < pSegTable->ulSegments; ulJ++ )
       {
          if ( pSegment->pData )
          {
             UtlAlloc( (PVOID *) &pSegment->pData, 0L, 0L, NOMSG );
          } /* endif */
          if ( pSegment->pDataW )
          {
             UtlAlloc( (PVOID *) &pSegment->pDataW, 0L, 0L, NOMSG );
          } /* endif */
          if (pSegment->pContext) UtlAlloc((PVOID *)&(pSegment->pContext),0L,0L,NOMSG);
          if (pSegment->pvMetadata) UtlAlloc((PVOID *)&(pSegment->pvMetadata),0L,0L,NOMSG);

          pSegment++;
       } /* endfor */
       UtlAlloc( (PVOID *) &pSegTable->pSegments, 0L, 0L, NOMSG );
       pSegTable++;
    } /* endfor */
    pDoc->ulSegTables = 0;
    UtlAlloc( (PVOID *) &pDoc->pSegTables, 0L, 0L, NOMSG );
    if (pDoc->pUndoSeg) UtlAlloc( (PVOID *) &pDoc->pUndoSeg, 0L, 0L, NOMSG );  //free storage of Undo
    if (pDoc->pUndoSegW) UtlAlloc( (PVOID *)&pDoc->pUndoSegW, 0L, 0L, NOMSG );
    if (pDoc->pContext) UtlAlloc((PVOID *)&(pDoc->pContext),0L,0L,NOMSG);
    UtlAlloc( (PVOID *) &pDoc, 0L, 0L, NOMSG );

}                                      // end EQFFreeDoc


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETSEGATTR                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETSEGATTR(PSZ, USHORT, PSEGINFO)                     |
//+----------------------------------------------------------------------------+
//|Description:       get info from buffer and fill it in pSegInfo             |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ       pszBuffer                                      |
//|                   USHORT    usBufLen                                       |
//|                   PSEGINFO  pSegInfo                                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   ERROR_STORAGE   UtlAlloc failed                          |
//|                   EQFRS_INVALID_PARM   tagtable cannot be loaded           |
//+----------------------------------------------------------------------------+
//|Function flow:     fill                                                     |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETSEGATTR
(
   PSZ       pszBuffer,
   USHORT    usBufLen,
   PSEGINFO  pSegInfo
)
{
  USHORT     usRc = 0;
  CHAR_W     chBufferW[256];

  usRc = EQFGETSEGATTRW( &chBufferW[0], (USHORT)min( usBufLen, 256 ), pSegInfo );
  Unicode2ASCII( &chBufferW[0], pszBuffer, 0L );

  return usRc;
}



__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETSEGATTRW
(
   PSZ_W     pszBuffer,
   USHORT    usBufLen,
   PSEGINFO  pSegInfo
)
{
  USHORT        usRc = EQFRC_OK;                            // return code
  PTOKENENTRY   pTokBuf = NULL;
  PLOADEDTABLE  pLoadedTable = NULL;

  SEGFLAGS      OldSegFlags;
  COUNTFLAG     OldCountFlag;
  USHORT        qOldStatus;

  usBufLen;

  memset (&OldSegFlags, 0, sizeof(SEGFLAGS));
  memset (&OldCountFlag, 0, sizeof(COUNTFLAG));
  memset (pSegInfo, 0, sizeof(SEGINFO));
  qOldStatus = QF_TOBE;
  /********************************************************************/
  /* retrieve from input pszXBuffer previous values                   */
  /********************************************************************/
  if (UtlAlloc((PVOID *) &pTokBuf, 0L,
               (LONG) TOK_BUFFER_SIZE, ERROR_STORAGE ))
  {
    usRc = (USHORT) TALoadTagTable( QFTAG_TABLE,
                                  (PLOADEDTABLE *) &pLoadedTable,
                                  TRUE, FALSE );
   if (usRc != NO_ERROR )
   {
     usRc = EQFRS_INVALID_PARM;      // is that correct?
   } /* endif */
  }
  else
  {
    usRc = ERROR_STORAGE;
  } /* endif */
  if (!usRc)
  {
    EQFXReadBuffer(pszBuffer, pTokBuf, pLoadedTable,
                     &qOldStatus, &OldSegFlags,
                     &(pSegInfo->usSegNum),
                     &(pSegInfo->usSrcWords),
                     &(pSegInfo->usTgtWords),
                     &(pSegInfo->usModWords),
                     &OldCountFlag );
   if (OldSegFlags.Current )
   {
     pSegInfo->sCurrent = SET_CURRENT;
   } /* endif */
   if (OldSegFlags.NoCount )
   {
     pSegInfo->sNoCount = SET_NOCOUNT;
   } /* endif */
   if (OldSegFlags.Typed )
   {
     pSegInfo->usTyped = SET_TYPED;
   } /* endif */
   if (OldSegFlags.Copied )
   {
     pSegInfo->usCopied = SET_COPIED;
   } /* endif */


   if (OldSegFlags.Marked || OldSegFlags.JoinStart ||
       OldSegFlags.Joined )
   {
     // not yet supported for external API users!
   } /* endif */
   /*******************************************************************/
   /* return info of OldCountFlag: not yet supported                  */
   /*******************************************************************/
   if (qOldStatus == QF_XLATED )
   {
     pSegInfo->sStatus = XLATED_STATUS;
   }
   else if (qOldStatus == QF_TOBE )
   {
     pSegInfo->sStatus = TOBE_STATUS;
   }
   else if (qOldStatus == QF_NOP  )
   {
     pSegInfo->sStatus = NOP_STATUS;
   }
   else if (qOldStatus == QF_ATTR )
   {
     pSegInfo->sStatus = ATTR_STATUS;
   } /* endif */
   /*******************************************************************/
   /* info which props previously existed / copied is NOT returned    */
   /* since user does not have the defines to understand              */
   /* usTypeExist and usTypeCopied structure                          */
   /*******************************************************************/
   if (OldCountFlag.AnalAutoSubst || OldCountFlag.EditAutoSubst )
   {
     /*****************************************************************/
     /* should this be returned some time?                            */
     /*****************************************************************/
   } /* endif */
// if (OldCountFlag.FuzzyCopy && OldCountFlag.ReplaceCopy )
// {
//   pSegInfo->usTypeCopied |= FUZZY_REPLACE_PROP;
// } /* endif */
  } /* endif */

  if ( pTokBuf )   UtlAlloc((PVOID *)&pTokBuf, 0L, 0L, NOMSG );
  if ( pLoadedTable)
  {
    TAFreeTagTable(pLoadedTable );
  }

  return usRc;

} // end 'EQFGETSEGATTR '

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFSEGFILECONVERTUNICODE2ASCII                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFSEGFILECONVERTUNICODE2ASCII(PSZ, PSZ)                 |
//+----------------------------------------------------------------------------+
//|Description:       get info from buffer and fill it in pSegInfo             |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ       pszInFile                                      |
//|                   PSZ       pszOutFile                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   --??                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     convert Infile which must be a segmented UTF16file,      |
//|                     into ASCII                                             |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFSEGFILECONVERTUNICODE2ASCII
(
   PSZ       pszInFile16,
   PSZ       pszOutFile16
)
 {
  USHORT   usRc = EQFRC_OK;
   PSZ  pszInFile = pszInFile16;
   PSZ  pszOutFile = pszOutFile16;

  usRc = EQFSEGFILECONVERTUNICODE2ASCIILANG( pszInFile, pszOutFile, NULL);
  return usRc;
} // end 'EQFSegFileConvertUnicode2ASCII '

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFSEGFILECONVERTUNICODE2ASCIILANG
(
   PSZ       pszInFile16,
   PSZ       pszOutFile16,
   PSZ       pszLang16
)
 {
  USHORT   usRc = EQFRC_OK;
    PSZ  pszInFile = pszInFile16;
    PSZ  pszOutFile = pszOutFile16;
    PSZ pszLang = pszLang16;

  usRc = TASegFileConvertUnicode2ASCII( pszInFile, pszOutFile, pszLang );
  if (usRc )
  {

    if ((usRc == (USHORT)ERROR_READ_FAULT)
         || (usRc == (USHORT)ERR_OPENFILE)
         || (usRc == (USHORT)ERR_READFILE)
         || ( usRc == (USHORT)ERROR_PATH_NOT_FOUND_MSG ) )
    {
      usRc = EQFS_FILE_OPEN_FAILED;
    }
    else if ( ( usRc == (USHORT)ERR_NOMEMORY)
              || (usRc == (USHORT)ERROR_NOT_ENOUGH_MEMORY)
              || (usRc == (USHORT)ERROR_STORAGE) )
    {
      usRc = ERROR_STORAGE;
    }
    else if ( (usRc == (USHORT)ERROR_FILE_INVALID_DATA)
              || (usRc == (USHORT)ERR_TAGSAREWRONG) )
    {
      usRc = ERROR_FILE_INVALID_DATA;
    }
    else
    {
      usRc = EQFRS_INVALID_PARM;
    }
  }
  return usRc;
} // end 'EQFSegFileConvertUnicode2ASCII '

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFSEGFILECONVERTASCII2UNICODE                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFSEGFILECONVERTASCII2UNICODE(PSZ, PSZ)                 |
//+----------------------------------------------------------------------------+
//|Description:       convert segmented ASCII file into UTF16 Unicode          |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ       pszInFIle                                      |
//|                   PSZ       pszOutFile                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   --                                                       |
//+----------------------------------------------------------------------------+
//|Function flow:     fill                                                     |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFSEGFILECONVERTASCII2UNICODE
(
  PSZ       pszInFile16,
  PSZ       pszOutFile16
)
{   USHORT   usRc = EQFRC_OK;
    PSZ  pszInFile = pszInFile16;
    PSZ  pszOutFile = pszOutFile16;

  usRc = EQFSEGFILECONVERTASCII2UNICODELANG( pszInFile, pszOutFile, NULL);

  return usRc;
} // end 'EQFSegFileConvertASCII2Unicode '

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFSEGFILECONVERTASCII2UNICODELANG
(
   PSZ       pszInFile16,
   PSZ       pszOutFile16,
   PSZ       pszLang16
)
{
    PSZ  pszInFile = pszInFile16;
    PSZ  pszOutFile = pszOutFile16;
    PSZ pszLang = pszLang16;

  USHORT   usRc = EQFRC_OK;

  usRc = TASegFileConvertASCII2Unicode( pszInFile, pszOutFile, pszLang );
  if (usRc )
  {
    if ((usRc == (USHORT)ERROR_READ_FAULT)
         || (usRc == (USHORT)ERR_OPENFILE)
         || (usRc == (USHORT)ERR_READFILE)
         || ( usRc == (USHORT)ERROR_PATH_NOT_FOUND_MSG ) )
    {
      usRc = EQFS_FILE_OPEN_FAILED;
    }
    else if ( ( usRc == (USHORT)ERR_NOMEMORY)
              || (usRc == (USHORT)ERROR_NOT_ENOUGH_MEMORY)
              || (usRc == (USHORT)ERROR_STORAGE) )
    {
      usRc = ERROR_STORAGE;
    }
    else if ( (usRc == (USHORT)ERROR_FILE_INVALID_DATA )
             || (usRc == (USHORT)ERR_TAGSAREWRONG) )
    {
        usRc = ERROR_FILE_INVALID_DATA;
    }
    else
    {
      usRc = EQFRS_INVALID_PARM;
    }
  }
  return usRc;
} // end 'EQFSegFileConvertASCII2Unicode '

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFFILECONVERSIONEX                                        |
//+----------------------------------------------------------------------------+
//|Function call:     EQFFILECONVERSIONEX(PSZ, PSZ, PSZ, USHORT)                 |
//+----------------------------------------------------------------------------+
//|Description:       get file and convert acc. to type                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ       pszInFile                                      |
//|                   PSZ       pszOutFile                                     |
//|                   PSZ       pszLanguage                                    |
//|                   USHORT    usConversionType                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   --??                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     convert Infile which must be a segmented UTF16file,      |
//|                     into ASCII                                             |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFFILECONVERSIONEX
(
   PSZ       pszInFile16,
   PSZ       pszOutFile16,
   PSZ       pszLang16,
   USHORT    usConversionType
)
{
  USHORT   usRC = EQFRC_OK;
  HFILE    hInFile = NULL;
  HFILE    hOutFile = NULL;
  USHORT   usOpenAction = 0L;
  CHAR        szTempFile[MAX_LONGPATH];  // buffer for temporary file name
  ULONG       ulBytesToRead;           // number of bytes to read into buffer
  ULONG       ulBytesInBuffer;         // number of bytes in input buffer
  ULONG       ulRemaining = 0;            // number of not-read bytes
  PBYTE       pInBuf = NULL;           // ptr to input buffer
  PBYTE       pOutBuf = NULL;
  ULONG       ulOemCP = 0L;
  ULONG       ulAnsiCP = 0L;
  ULONG       ulBytesWrittenToOutBuf = 0;
  LONG        lBufLen = 0;

   PSZ    pszInFile = pszInFile16;
   PSZ    pszOutFile = pszOutFile16;
   PSZ    pszLang = pszLang16;
  LONG        lRc = 0;
  BOOL        fInFileIsUTF8 = FALSE;
  BOOL        fInFileIsUTF16 = FALSE;
  BOOL        fOutFileIsUTF16 = FALSE;
  BOOL        fUTF16BOMFound = FALSE;
  BOOL        fUTF8BOMFound = FALSE;
  LONG        lBytesLeft = FALSE;    // TRUE if DBCS1st not converted at end of inbuf
  ULONG       ulCurrentPos = 0;
  BOOL        fFirstTry = TRUE;
  BOOL        fWriteUTF8BOM = FALSE;

#ifdef API_LOGGING
  char szLogFile[MAX_EQF_PATH];
  FILE        *hLog = NULL;                      // log file handle
#endif

#ifdef API_LOGGING
  {
    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    if ( !UtlDirExist( szLogFile ) ) UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, "\\EQFX1API.LOG" );
    BOOL fLogExists = UtlFileExist( szLogFile );
    hLog = fopen( szLogFile, "ab" );
    if ( hLog )
    {
      if ( !fLogExists )
      {
        fwrite( UNICODEFILEPREFIX, 2, 1, hLog );
      } /* endif */
      fwprintf( hLog, L"EQFFILECONVERSIONEX(\"%S\",\"%S\",\"%S\",%u)\r\n", pszInFile16, pszOutFile16, pszLang16, usConversionType );
    } /* endif */
  }
#endif

  // if pszLang= NULL, default target language of the system preferences is used
  ulOemCP = GetLangOEMCP(pszLang);
  ulAnsiCP = GetLangAnsiCP(pszLang);

  // preprocess special UF8 cpnversion taype
  if ( usConversionType == EQF_UTF162UTF8BOM )
  {
    usConversionType = EQF_UTF162UTF8;
    fWriteUTF8BOM = TRUE;
  } /* endif */

  // check whether usConversionType is valid
  if (!( (usConversionType == EQF_ASCII2ANSI)  ||
         (usConversionType == EQF_ANSI2ASCII)  ||
         (usConversionType == EQF_ASCII2UTF8)  ||
         (usConversionType == EQF_UTF82ASCII)  ||
         (usConversionType == EQF_ASCII2UTF16) ||
         (usConversionType == EQF_UTF162ASCII) ||
         (usConversionType == EQF_ANSI2UTF8)   ||
         (usConversionType == EQF_UTF82ANSI)   ||
         (usConversionType == EQF_ANSI2UTF16)  ||
         (usConversionType == EQF_UTF162ANSI)  ||
         (usConversionType == EQF_UTF82UTF16)  ||
         (usConversionType == EQF_UTF162UTF8)  ))
  {
          usRC = EQFRS_INVALID_PARM;   // output file is not written!
#ifdef API_LOGGING
          if ( hLog )
          {
            fwprintf( hLog, L"Error: Invalid conversion type %u\r\n", usConversionType );
          } /* endif */
#endif

  }

  if (!usRC)
  {
          if (   (usConversionType == EQF_UTF162ASCII) ||
                           (usConversionType == EQF_UTF162ANSI) ||
                           (usConversionType == EQF_UTF162UTF8)  )
          {
                  fInFileIsUTF16 = TRUE;   // output file is not written!
          }
          if ( (usConversionType == EQF_ASCII2UTF16) ||
                        (usConversionType == EQF_ANSI2UTF16) ||
                        (usConversionType == EQF_UTF82UTF16) )
          {
                  fOutFileIsUTF16 = TRUE;  // output file is not written!
          }
          if (   (usConversionType == EQF_UTF82ASCII) ||
                                 (usConversionType == EQF_UTF82ANSI) ||
                                 (usConversionType == EQF_UTF82UTF16)  )
          {
                          fInFileIsUTF8 = TRUE;   // output file is not written!
          }
  }
 /********************************************************************/
  /* Allocate input&output buffer                                     */
  /* Alloc 2 more bytes to make sure that 2 Hex-0 Bytes are at the end*/
  /********************************************************************/
  if ( !usRC && !UtlAlloc( (PVOID *)&pInBuf, 0L, (LONG)API_INBUF_SIZE + 2, ERROR_STORAGE ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
#ifdef API_LOGGING
    if ( hLog )
    {
      fwprintf( hLog, L"Error: Memory allocation of input buffer failed\r\n" );
    } /* endif */
#endif
  } /* endif */
  lBufLen = (LONG)API_INBUF_SIZE * sizeof(CHAR_W);

  if ( !usRC && !UtlAlloc( (PVOID *)&pOutBuf, 0L,
                           lBufLen + 2, ERROR_STORAGE ) )
  {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
#ifdef API_LOGGING
    if ( hLog )
    {
      fwprintf( hLog, L"Error: Memory allocation of output buffer failed\r\n" );
    } /* endif */
#endif
  } /* endif */

  // use output file name with the extension '.conv-tmp' as termporary file name
  if ( !usRC )
  {
    strcpy( szTempFile, pszOutFile );
    strcat( szTempFile, ".conv-tmp" );
  } /* endif */

  if ( !usRC )
  {
    // open input file
    usRC = UtlOpen(pszInFile, &hInFile, &usOpenAction, 0L, FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, FALSE );
#ifdef API_LOGGING
    if ( usRC && hLog )
    {
      fwprintf( hLog, L"Error: Open of input file %S failed, return code is %u\r\n", pszInFile, usRC  );
    } /* endif */
#endif
  }

  /*******************************************************************/
  /* open output file                                                */
  /*******************************************************************/
  if ( !usRC )
  {
     USHORT      usOpenAction;                             // action performed by DosOpen

     usRC = UtlOpen( szTempFile, &hOutFile, &usOpenAction, 0L, FILE_NORMAL, FILE_TRUNCATE | FILE_CREATE, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, FALSE );
#ifdef API_LOGGING
    if ( usRC && hLog )
    {
      fwprintf( hLog, L"Error: Open of output file %S failed, return code is %u\r\n", szTempFile, usRC  );
    } /* endif */
#endif
  } /* endif */

  /*******************************************************************/
  /* get size of input file                                          */
  /*******************************************************************/
  if ( !usRC )
  {
    FILESTATUS  stStatus;               // File status information
    usRC = UtlQFileInfo( hInFile, 1, (PBYTE)&stStatus, sizeof(FILESTATUS), FALSE );
    ulRemaining = stStatus.cbFile;
#ifdef API_LOGGING
    if ( usRC && hLog )
    {
      fwprintf( hLog, L"Error: UtlQFileInfo failed for input file %S, return code is %u\r\n", pszInFile, usRC  );
    } /* endif */
#endif
  } /* endif */

  if ( !usRC )
  { // if first read check for UTF16/UTF8 BOM / Unicode  file

    PSZ pszPrefix = UNICODEFILEPREFIX;
    PSZ pszUTF8Prefix = UTF8FILEPREFIX;
    PSZ pData = (PSZ)(pInBuf);
    int iLen = strlen(pszPrefix);
    int iUTF8Len = strlen(pszUTF8Prefix);
    ULONG ulTemp;

    usRC = UtlReadL( hInFile, pData, 8, &ulBytesInBuffer, FALSE );
#ifdef API_LOGGING
    if ( usRC && hLog )
    {
      fwprintf( hLog, L"Error: Read of first 8 bytes of input file failed, return code is %u\r\n", usRC  );
    } /* endif */
#endif
    if (!usRC)
    {
      if ( (memcmp( pData, pszPrefix, iLen ) == 0) )
      {
              fUTF16BOMFound = TRUE;
      }
      else if ( memcmp( pData, pszUTF8Prefix, iUTF8Len ) == 0)
      {
              fUTF8BOMFound = TRUE;
      }

      if (fUTF16BOMFound != fInFileIsUTF16)
      {
            usRC = ERROR_FILE_INVALID_DATA;
      }
      else  if ( fUTF16BOMFound )
      {   // position right behind prefix
              usRC = UtlChgFilePtr( hInFile, iLen, FILE_BEGIN, &ulTemp, FALSE );
              ulRemaining -= iLen;
      }
      else if (fUTF8BOMFound )
      {
              if (!fInFileIsUTF8)
              {
                    usRC = ERROR_FILE_INVALID_DATA;
              }
              else
              {// position right behind prefix
                    usRC = UtlChgFilePtr( hInFile, iUTF8Len, FILE_BEGIN, &ulTemp, FALSE );
                    ulRemaining -= iUTF8Len;
              }
      }
      else
      {     // position back to start of file
              usRC = UtlChgFilePtr( hInFile, 0L, FILE_BEGIN, &ulTemp, FALSE );
      } /* endif */
    }
  }

  while ( !usRC && ulRemaining )
  {

    /****************************************************************/
    /* Fill input buffer                                            */
    /****************************************************************/
    ulBytesToRead = min( (LONG)API_INBUF_SIZE, ulRemaining );
    memset(pInBuf, 0L, API_INBUF_SIZE);
    UtlChgFilePtr( hInFile, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE );
    usRC = UtlReadL( hInFile, pInBuf, ulBytesToRead, &ulBytesInBuffer, FALSE);
#ifdef API_LOGGING
    if ( usRC && hLog )
    {
      fwprintf( hLog, L"Error: Read of %lu bytes at position %lu failed, return code is %u\r\n", ulBytesToRead, ulCurrentPos, usRC  );
    } /* endif */
#endif
    if ( !usRC )
    {
      ulRemaining -= ulBytesInBuffer;
    } /* endif */

    /****************************************************************/
    /* Process data in input buffer                                 */
    /****************************************************************/

    if ( !usRC )
    {
      LONG   lNewBufLen = 0;
      ulBytesWrittenToOutBuf= UtlConvertBuf(usConversionType, pInBuf, pOutBuf, ulBytesInBuffer, lBufLen,   // in numb. of bytes! is this correct?
                                        ulOemCP, ulAnsiCP, FALSE, &lRc, &lBytesLeft);
      usRC = (USHORT)lRc;
      if ((usRC == ERROR_INSUFFICIENT_BUFFER) )
      { // get required size in number of bytes
        lNewBufLen= UtlConvertBuf(usConversionType, pInBuf, pOutBuf, ulBytesInBuffer, 0L, ulOemCP, ulAnsiCP, FALSE, &lRc, &lBytesLeft);

        lNewBufLen = 2*(lNewBufLen/2) + 1000;  // force Buflen to be even

        if ( !UtlAlloc( (PVOID *)&pOutBuf, lBufLen,
                                    lNewBufLen+1, ERROR_STORAGE ) )
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
          lBufLen = lNewBufLen;
          ulBytesWrittenToOutBuf= UtlConvertBuf(usConversionType, pInBuf, pOutBuf, ulBytesInBuffer, lBufLen,   // size of outbuf in numb. of bytes
                                          ulOemCP, ulAnsiCP, FALSE, &lRc, &lBytesLeft);
          usRC = (USHORT)lRc;
        } /* endif */
#ifdef API_LOGGING
        if ( usRC && hLog )
        {
          fwprintf( hLog, L"Error: Conversion of %lu bytes at position %lu failed, return code is %u\r\n", ulBytesInBuffer, ulCurrentPos, usRC  );
        } /* endif */
#endif
      } /* endif */
    } /* endif */

    /* Write buffer to output file                                    */
    if ( !usRC )
    {
      ULONG      ulBytesWritten;      // number of bytes written to file
      if ( fFirstTry )
      { 
        // write any Unicode prefix to outfile
        if ( fOutFileIsUTF16 )
        {
          usRC = UtlWriteL( hOutFile, UNICODEFILEPREFIX, (SHORT)strlen(UNICODEFILEPREFIX), &ulBytesWritten, FALSE );
        }
        else if ( fWriteUTF8BOM )
        {
          usRC = UtlWriteL( hOutFile, UTF8FILEPREFIX, (SHORT)strlen(UTF8FILEPREFIX), &ulBytesWritten, FALSE );
        } /* endif */
        fFirstTry = FALSE;
#ifdef API_LOGGING
        if ( usRC && hLog )
        {
          fwprintf( hLog, L"Error: Writing of BOM to output file failed, return code is %u\r\n", usRC  );
        } /* endif */
#endif
      } /* endif */

      if (!usRC)
      {
        usRC = UtlWriteL( hOutFile, pOutBuf, ulBytesWrittenToOutBuf, &ulBytesWritten, FALSE );
#ifdef API_LOGGING
        if ( (usRC || (ulBytesWrittenToOutBuf != ulBytesWritten)) && hLog )
        {
          fwprintf( hLog, L"Error: Write of %lu bytes to output buffer failed, %lu bytes have been written, return code is %u\r\n", ulBytesWrittenToOutBuf, ulBytesWritten, usRC  );
        } /* endif */
#endif
        if (ulBytesWrittenToOutBuf != ulBytesWritten )
        {
          usRC = ERROR_STORAGE;
        }
      }

    } /* endif */
    if (lBytesLeft)
    {
      // undo the last character read...
      // reposition file pointer
      UtlChgFilePtr( hInFile, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
      ulCurrentPos = ulCurrentPos - lBytesLeft;
      UtlChgFilePtr( hInFile, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);
      ulRemaining = ulRemaining + lBytesLeft;         // one more byte to be converted...
      lBytesLeft = 0;
    }
  } /* endwhile */


   // cleanup
   if ( hOutFile )
   {
      UtlClose( hOutFile, FALSE );
      // delete output file in case of erros                          
      if ( usRC )
      {
        UtlDelete( szTempFile, 0L, FALSE );
      } /* endif */
   } /* endif */
   if ( hInFile ) UtlClose( hInFile, FALSE );
   if ( pInBuf )  UtlAlloc( (PVOID *)&pInBuf, 0L, 0L, NOMSG );
   if ( pOutBuf ) UtlAlloc( (PVOID *)&pOutBuf, 0L, 0L, NOMSG );

   /*******************************************************************/
   /* Delete old file and rename temp file                            */
   /*******************************************************************/
   if ( !usRC )
   {
     UtlDelete( pszOutFile, 0L, FALSE );
     usRC = UtlMove( szTempFile, pszOutFile, 0L, FALSE );
#ifdef API_LOGGING
      if ( usRC && hLog )
      {
        fwprintf( hLog, L"Error: Rename/Move of temp file %S to output file %S failed, return code is %u\r\n", szTempFile, pszOutFile, usRC  );
      } /* endif */
#endif
   } /* endif */

#ifdef API_LOGGING
  {
    if ( usRC && hLog )
    {
      fwprintf( hLog, L"EQFFILECONVERSIONEX Original RC=%u\r\n", usRC );
    } /* endif */
  }
#endif

  if (usRC )
  {
    switch(usRC)
    {
      case ERROR_READ_FAULT:
      case ERROR_FILE_NOT_FOUND:
      case ERR_OPENFILE:
      case ERR_READFILE:
      case ERROR_PATH_NOT_FOUND_MSG:
          usRC = EQFS_FILE_OPEN_FAILED;
          break;
      case ERR_NOMEMORY:
      case ERROR_NOT_ENOUGH_MEMORY:
      case ERROR_STORAGE:
          usRC = ERROR_STORAGE;
          break;
      case ERROR_FILE_INVALID_DATA:
      case ERR_TAGSAREWRONG:
      case ERROR_NO_UNICODE_TRANSLATION:
          usRC = ERROR_FILE_INVALID_DATA;
          break;
      default:
          usRC = EQFRS_INVALID_PARM;
          break;
    }
  }

#ifdef API_LOGGING
  if ( hLog )
  {
    fwprintf( hLog, L"EQFFILECONVERSIONEX returns %u\r\n", usRC );
    fclose( hLog );
  } /* endif */
#endif


  return usRC;
} // end 'EQFFILECONVERSIONEX'
