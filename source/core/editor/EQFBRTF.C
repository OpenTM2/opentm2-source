/*! \file
	Description: This module contains a set of routines which interface to the Presentation Manager.
	It uses standard VIO-Calls to process the screen.
	
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS
#include <eqf.h>                  // General Translation Manager include file

#include "tchar.h"
#include "EQFTPI.H"               // Translation Processor priv. include file
#include <eqfdoc00.h>

/**********************************************************************/
/* if this specific bidi style not yet defined..                      */
/**********************************************************************/
#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL                    0x00400000L
#endif


#define  RTF_QFF_TAG    1
#define  RTF_N_TAG      2
#define  RTF_S_TAG      3
#define  RTF_L_TAG      4
#define  RTF_T_TAG      5
#define  RTF_QFI_TAG    10
#define  RTF_EQFI_TAG   11
#define  RTF_QFJ_TAG    20
#define  RTF_NONE_TAG   99

static HKL  hklChanged;


#define RTF_OFFSET 50
#define RTFBUFFER_INC 2000


  #define STATUSBARRTF( p )                                        \
  {                                                                \
    if ( p->hwndRichEdit && p->pDispFileRTF->fTBCursorReCalc )                   \
    {                                                              \
      PostMessage( p->hwndRichEdit, WM_EQF_UPDATERTFCTRL, 0, 0L ); \
    }                                                              \
  }


#define MEASURE_TIME( a, b ) \
 {  a = GetTickCount() - b;  }

#define MEASURE_TIME_FILE( a, b ) \
{                                                   \
    FILE *fOut;                                     \
    fOut      = fopen ( "D:\\RTFTIME.OUT", "a" );   \
    fprintf(fOut, "\n%s: %u",a,b);                  \
    fclose( fOut );                                 \
}

#ifdef RTF_DEBUG
   static ULONG ulStart;
#endif

#ifndef RTF_RTFDUMP
  #define RTFDump( a, b )
  #define RTFDumpSel( a, b )
  #define RTFDumpNoSel( a, b )
#endif

/**********************************************************************/
/* font specifics to be used                                          */
/**********************************************************************/
static CHAR_W chRTFFont[150];



#define RTF_PARA           L"\\par "
#define RTF_HIDDEN         L"<qff n=%d t=\"%s\" l=%d>"
#define RTF_STREAM_HIDDEN  L"\\v <qff n=%d t=\"%s\" l=%d>\\v0 \\cf0 "
#define RTF_JOINED         L"<qfj n=%d t=\"%s\" l=%d>"
#define RTF_INLINESTYLE    L"<qfi t=\"%s\" s=\"%s\">"
#define RTF_ENDINLINESTYLE L"</qfi>"
#define RTF_IGNORESTYLE    L"<qfi t=\"%s\" s=\"%s\"></qfi>"
#define RTF_SEGSTART  0xD6           // Start indication for text to be trans.

#define DISP_ADD "QW"
#define DISP_ADD_LEN 2

#define CONSECUTIVE            1
#define FIRSTNOP_WITHOUTLF     2
#define FIRSTNOP_WITHLF        3
#define NOTSPEC                4


#define RTF_NEXT_HIDDEN                 1
#define RTF_NEXTNEXT_HIDDEN             2
#define RTF_PREV_HIDDEN                 4
#define RTF_PREVHIDDEN_NEXTPROTECTED    8


VOID EQFBFindNOPKind( PTBDOCUMENT, ULONG, USHORT, USHORT, PUSHORT );
VOID EQFBAddSegRTF( PTBDOCUMENT, PTBSEGMENT, PSZ_W, ULONG, USHORT );
VOID EQFBAddSegRTFFast( PTBDOCUMENT, PTBSEGMENT, PSZ_W, ULONG, USHORT );
VOID DisplaySegRTF( PTBDOCUMENT, PTBSEGMENT, PSZ_W, USHORT);

ULONG EQFBGetCaretPosRTF( PTBDOCUMENT pDoc );
VOID  EQFBSetCaretPosRTF( PTBDOCUMENT pDoc, ULONG ulCaret );

VOID   EQFBMakeRTFEscapedLF ( PSZ_W pT1, PSZ_W pT2 );
VOID   RTFInlineEscape( PSZ_W pT1, PSZ_W pT2 );
USHORT RTFInlineDeEscape( PSZ_W pT1, PSZ_W pT2, USHORT usLen );

VOID FillRTFSpecialFast( PTBDOCUMENT pDoc, PSZ_W p, BOOL fConvert );

VOID SetNormal(PTBDOCUMENT pDoc, PSZ_W pData, CHARFORMAT2 *pcfDefault, PSZ_W pRTFFont);
VOID SetProtected (PTBDOCUMENT pDoc, PSZ_W pData, CHARFORMAT2 *pcfDefault, PSZ_W pRTFFont);
VOID SetHidden(PTBDOCUMENT pDoc, PSZ_W pData, CHARFORMAT2 *pcfDefault, PSZ_W pRTFFont);
MRESULT EQFBCallOrgTPFunction( PTBDOCUMENT pDoc, USHORT usFunction );
BOOL GetCharFormat( PTBDOCUMENT pDoc, ULONG ulMode );

USHORT SpecialCare( PTBDOCUMENT pDoc);
VOID ReplaceCharBeforeHidden( PTBDOCUMENT pDoc, WPARAM mp1 );
VOID ReplaceCharAfterHidden( PTBDOCUMENT pDoc, WPARAM mp1 );
VOID InsertCharAfterHidden( PTBDOCUMENT pDoc, WPARAM mp1, BOOL fProtectFollow );
VOID InsertCharBeforeHidden( PTBDOCUMENT pDoc, WPARAM mp1);
MRESULT RichEditHandleWM_CHAR(HWND, PTBDOCUMENT,  WINMSG, WPARAM, LPARAM );


void AdjustWorkSegData( PTBDOCUMENT pDoc, PSZ_W pData, ULONG ulSegNum );
PSZ_W GetWorkSegDataPtr( PTBDOCUMENT pDoc, EQFBRTFLINE *pRTFLine, ULONG ul );


SHORT PosEQF2RTF( PTBDOCUMENT pDoc, PSZ_W pEQFData, PSZ_W pRTFData, SHORT sPos );
SHORT PosRTF2EQF( PTBDOCUMENT pDoc, PSZ_W pRTFData, PSZ_W pEQFData, SHORT sPos );
SHORT GetPosInRestoredLine( PTBDOCUMENT pDoc, PSZ_W pData, SHORT sPos );

VOID  GetTextRangeRTF( PTBDOCUMENT pDoc, PSZ_W pData, ULONG cb, ULONG ulStart, ULONG ulSize);
VOID EQFBFuncBackspaceRTF( PTBDOCUMENT pDoc );
VOID EQFBFuncDeleteRTF( PTBDOCUMENT pDoc );

static BOOL CheckForLF( PSZ_W pData );

#define GETNUMBER( pszTarget, usValue ) \
{                                   \
   usValue = 0;                     \
   if ( *pszTarget++ )              \
   {                                \
      while ( isdigit((UCHAR)*pszTarget) ) \
      {                             \
         usValue = (usValue * 10) + ((UCHAR)*pszTarget++ - '0'); \
      } /* endwhile */              \
   } /* endif */                    \
}

extern PFINDDATA pFindData;


MRESULT HandleDummyMessage( HWND hwnd, PTBDOCUMENT pDoc, WINMSG msg, WPARAM mp1, LPARAM mp2);
BOOL    PrepareSegOffsTable( PTBDOCUMENT pDoc, PSZ_W pText, PEQFBRTFLINE pRTFLine );
PSZ_W   EQFBGetSegRTF( PTBDOCUMENT pDoc, PEQFBRTFLINE pRTFLine, ULONG ulSegNum );
VOID    EQFBGetMatchingRangeRTF( PTBDOCUMENT pDoc, ULONG ulSegNum, PEQFBRTFLINE pRTFLine );

VOID EQFBUpdateTBCursor( PTBDOCUMENT pDoc );

static USHORT EQFBTCheckPos( PTBDOCUMENT );     // check position of new tag
//static BOOL EQFBTInitTrans( PTBDOCUMENT, PUSHORT);   // init translation environment

VOID  BuildRTFShorten( PSZ_W pBuffer, PSZ_W pData, USHORT  usLen);
VOID  DeleteArea( PTBDOCUMENT pDoc, PEQFBRTFLINE pRTFLine,
                  CHARRANGE *pchRange, CHARRANGE *pchRange1, ULONG ulI, PBOOL pBool );


VOID  WYSIWYG_HTML_Display( PTBDOCUMENT pDoc, PSZ_W pData, CHARFORMAT2 *pCharFormat2, PTBSEGMENT pSeg );
VOID  WYSIWYG_RTF_Display( PTBDOCUMENT pDoc, PSZ_W pData, CHARFORMAT2 *pCharFormat2, PTBSEGMENT pSeg );
USHORT EQFBCalcSegOffsetFromCaret( PTBDOCUMENT pDoc, USHORT usCaret, PSZ_W pData, PEQFBRTFLINE pRTFLine );

VOID CleanIBU( PSZ_W pRTFFont );

//static DWORD CALLBACK
//EditStreamInTest( DWORD dwCookie,
//                    LPBYTE pbBuff,
//                    LONG cb,
//                    LONG *pcb )
//{
//  /******************************************************************/
//  /* stream in buffer                                               */
//  /******************************************************************/
//  strcpy( (PSZ)pbBuff, (PSZ)dwCookie );
//  *pcb = strlen( pbBuff );
//
//  return 0;
//}
//


static DWORD CALLBACK
EditStreamInCallBack( DWORD dwCookie,
                    LPBYTE pbBuff,
                    LONG cb,
                    LONG *pcb )
{
 #ifdef RTF_DEBUG
  ULONG ulStart1;
#endif

  PDISPFILERTF  pDisp = (PDISPFILERTF)dwCookie;
  PTBDOCUMENT pDoc = (PTBDOCUMENT)pDisp->pDoc;
  PTBSEGMENT pSeg;

  pDisp->cb = cb;
  pDisp->pbBuff = pbBuff;

#ifdef RTF_DEBUG
    MEASURE_TIME( ulStart1, 0L );
#endif
  if (pDisp->ulSegNum == 0 )
  {
    /******************************************************************/
    /* length of header less than 4k ...                              */
    /******************************************************************/
    LONG lLen = strlen( pDisp->pHeader );
    memcpy( pbBuff, pDisp->pHeader, lLen );
    pDisp->lPoscb += lLen;

//@@ Hebrew and Arabic -- set style to RTL
    if ((pDoc->docType == STARGET_DOC) && IS_RTL(pDoc))
    {
      PSZ pHebrew = "\\rtldoc\\rtlpar\\qr\\rtlch ";

      lLen = strlen( pHebrew);
      memcpy(pbBuff+pDisp->lPoscb, pHebrew, lLen );
      pDisp->lPoscb += lLen;
    }



    if (pDoc->docType == STARGET_DOC || pDoc->docType == SSOURCE_DOC )
    {
      CHAR_W chText[2];
      chText[0] = (CHAR_W)((UCHAR)'\xA7');
      chText[1] = EOS;

      ASCII2Unicode( pDoc->pDispFileRTF->pCharFormat[MAX_MAXRTFFONTS],
                     &pDoc->pDispFileRTF->chRTFFormatW[0],
                     pDoc->ulOemCodePage);
      SetProtected( pDoc, &chText[0], NULL, &pDoc->pDispFileRTF->chRTFFormatW[0] );

    }

    pDisp->ulSegNum++;
  }
  else
  {
    /******************************************************************/
    /* fill in rest                                                   */
    /******************************************************************/
    if (pDisp->lPos > cb)
    {
      memcpy(pbBuff, pDisp->pBufferOverflow, cb );
      pDisp->lPoscb = cb;
      pDisp->lPos -= cb;
      memcpy(pDisp->pBufferOverflow, pDisp->pBufferOverflow+cb, pDisp->lPos);
    }
    else
    {
      memcpy(pbBuff, pDisp->pBufferOverflow, pDisp->lPos );
      pDisp->lPoscb = pDisp->lPos;
      pbBuff[pDisp->lPoscb] = EOS;
      pDisp->lPos = 0;
      pDisp->pBufferOverflow[0] = EOS;
    }
  }

  while ( pDisp->ulSegNum < pDoc->ulMaxSeg && (pDisp->lPoscb < cb) )
  {
    PSZ_W      pData;
    USHORT     usKindOfNOP;
    pSeg = EQFBGetSegW(pDoc, pDisp->ulSegNum);


    if ( pDoc->docType == STARGET_DOC )
    {
      PTBSEGMENT pSegTwin = EQFBGetSegW( pDoc->twin, pDisp->ulSegNum );
      pData = pSegTwin->pDataW;
    }
    else
    {
      pData = L"";
    } /* endif */

    EQFBFindNOPKind(pDoc, pDisp->ulSegNum, pDisp->qLastSegStatus,
                    pSeg->qStatus, &usKindOfNOP );

    EQFBAddSegRTFFast( pDoc, pSeg, pData, pDisp->ulSegNum, usKindOfNOP );
    pDisp->qLastSegStatus = pSeg->qStatus;

    pDisp->ulSegNum++;
  } /* endwhile */

  /********************************************************************/
  /* add ending curly brace                                           */
  /********************************************************************/
  if ( pDisp->ulSegNum == pDoc->ulMaxSeg )
  {
    if ( pDisp->lPoscb < cb )
    {
      pDisp->pbBuff[pDisp->lPoscb++] = '}';
    }
    else
    {
      pDisp->pBufferOverflow[pDisp->lPos++] = '}';
    } /* endif */
    pDisp->ulSegNum++;
  } /* endif */


  if ( pDisp->lPoscb < cb )
  {
    *(pbBuff+pDisp->lPoscb) = EOS;
    *pcb    = pDisp->lPoscb;
  }
  else
  {
    *pcb = cb;
  } /* endif */

  /********************************************************************/
  /* update status bar                                                */
  /********************************************************************/
  if ( pDoc->ulMaxSeg > 500 )
  {
    if ( pDisp->ulSegNum < pDoc->ulMaxSeg )
    {
      CHAR chText[30];
      USHORT usLoadState = (USHORT)((pDisp->ulSegNum-1)*100/pDoc->ulMaxSeg);

      if ( usLoadState >= pDisp->usLoadState + 5 )
      {
        sprintf( chText, pDoc->chLoading, usLoadState );
        SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                     WM_EQF_UPDATESTATUSBAR_TEXT, 0, (LONG)&chText[0] );

        pDisp->usLoadState = usLoadState;
      } /* endif */
    }
    else
    {
      pDisp->usLoadState = 0;
      SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                   WM_EQF_UPDATESTATUSBAR_TEXT, 0, (LONG) "" );
    } /* endif */

  } /* endif */


#ifdef RTF_DEBUG
    {
      ULONG ulDiff;
      MEASURE_TIME( ulDiff, ulStart1 );
      MEASURE_TIME_FILE( "4k stream-in time", ulDiff );
    } /* endif */

{
    FILE *fOut;
    fOut      = fopen ( "D:\\RTFFILE.OUT", "a" );
    fprintf(fOut, "\n%5.5ul %4.4d %4.4d",pDisp->ulSegNum, pDisp->lPoscb, pDisp->lPos);
    fclose( fOut );


    if ( pDisp->ulSegNum >= 42053l )
    {
      int i;
      i = 0;
    }
    else
    {
    } /* endif */
}



#endif


  return 0;
}

//static DWORD CALLBACK
//EditStreamInSegCallBack( DWORD dwCookie,
//                    LPBYTE pbBuff,
//                    LONG cb,
//                    LONG *pcb )
//{
//  PDISPFILERTF  pDisp = (PDISPFILERTF)dwCookie;
//
//  /******************************************************************/
//  /* stream in buffer                                               */
//  /******************************************************************/
//  if (pDisp->lPos > cb)
//  {
//    memcpy(pbBuff, pDisp->pBufferOverflow, cb );
//    pDisp->lPoscb = cb;
//    pDisp->lPos -= cb;
//    memcpy(pDisp->pBufferOverflow, pDisp->pBufferOverflow+cb, pDisp->lPos);
//  }
//  else
//  {
//    memcpy(pbBuff, pDisp->pBufferOverflow, pDisp->lPos );
//    pDisp->lPoscb = pDisp->lPos;
//    pbBuff[pDisp->lPoscb] = EOS;
//    pDisp->lPos = 0;
//    pDisp->pBufferOverflow[0] = EOS;
//  } /* endif */
//
//
//  *pcb    = pDisp->lPoscb;
//
//  return 0;
//}
//
VOID EQFBAddSegRTFFast
(
   PTBDOCUMENT pDoc,
   PTBSEGMENT  pSeg,
   PSZ_W       pData,
   ULONG       ulI,
   USHORT      usKindOfNOP
)
{
  CHAR_W       chBuffer[ 2 * MAX_SEGMENT_SIZE ];
  CHARFORMAT2  cfDefault;
  USHORT usState = UNPROTECTED_CHAR;        // status of character
  pData;
  if ( pSeg->qStatus == QF_NOP )
  {
    usState = PROTECTED_CHAR;
  }

  memset( &cfDefault, 0, sizeof(cfDefault) );
  cfDefault.cbSize = sizeof(cfDefault);
  if (pSeg && pSeg->SegFlags.Joined )
  {
    swprintf( chBuffer, RTF_JOINED, ulI, L"A", 1);
  }
  else
  {
    swprintf( chBuffer, RTF_HIDDEN, ulI, L"A", 1);
  } /* endif */

  SetHidden( pDoc, chBuffer, &cfDefault, NULL );


  if (pSeg && !pSeg->SegFlags.Joined )
  {
    CHAR_W c;
    c = pSeg->pDataW[pSeg->usLength]; pSeg->pDataW[pSeg->usLength] = EOS;
    DisplaySegRTF( pDoc, pSeg, pSeg->pDataW, usKindOfNOP );
    pSeg->pDataW[pSeg->usLength] = c;
  } /* endif */



}



static DWORD CALLBACK
EditStreamOutCallBack( DWORD dwCookie,
                    LPBYTE pbBuff,
                    LONG cb,
                    LONG *pcb )
{
  PDISPFILERTF pDisp = (PDISPFILERTF) dwCookie;
  PSZ    p = pDisp->chRTFFormat;
  CHAR   c;
  USHORT usNum = 0;
  LONG lSpace = sizeof( pDisp->chRTFFormat ) - pDisp->lPos;

  pDisp->pbBuff = pbBuff;

  if ( cb < lSpace  )
  {
    memcpy( &pDisp->chRTFFormat[pDisp->lPos], pbBuff, cb );
    *pcb = cb;
    pDisp->lPos += cb;
  }
  else
  {
    /******************************************************************/
    /* should never happen                                            */
    /******************************************************************/
    *pcb = cb;
    cb  = 1;
  } /* endif */


  /********************************************************************/
  /* last call happened                                               */
  /********************************************************************/
  if ( cb < 4095 )
  {

#ifdef RTF_DEBUG
  {
    FILE *fOut;                                // test output
    fOut      = fopen ( "D:\\RTFSTART.OUT", "a" );
    fprintf(fOut, "\n\n\n");
    fprintf(fOut, pDisp->chRTFFormat );
    fclose( fOut );
  }
#endif


    /********************************************************************/
    /* set pointers correctly  -- header is the first 3 lines           */
    /********************************************************************/
    pDisp->pHeader = pDisp->chRTFFormat;

    usNum = 0;
    while ( (c = *p) != NULC )
    {
      switch ( c )
      {

        case '\n':
          usNum++;
          if ( usNum < 3 )
          {
            /**********************************************************/
            /* ignore requests                                        */
            /**********************************************************/
          }
          else if ( usNum <= 2*(MAX_MAXRTFFONTS+1) +3 )
          {
            *(p-5) = EOS;
            if ( (usNum - 3)%2 == 1)
            {
              pDisp->pCharFormat[ (usNum - 3)/2 ] = p+1;
            }
            else
            {
              /************************************************************/
              /* end any pending string                                   */
              /************************************************************/
              *(p-5) = EOS;
            } /* endif */
          }
          else
          {
            /************************************************************/
            /* end any pending string                                   */
            /************************************************************/
            *(p-5) = EOS;
          } /* endif */
          break;
        default:
          break;
      } /* endswitch */
      p++;    // point to next character
    } /* endwhile */

#ifdef RTF_DEBUG
//  {
//    FILE *fOut;                                // test output
//    USHORT i;
//    fOut      = fopen ( "D:\\RTFSTART.OUT", "a" );
//    fprintf(fOut, "\n\n\n");
//    for (i=0; i<MAX_MAXRTFFONTS+1; i++)
//    {
//      if (pDisp->pCharFormat[i])
//      {
//        fprintf(fOut, "%d: %s\n", i, pDisp->pCharFormat[i]);
//      }
//      else
//      {
//        fprintf(fOut, "%d: NULL\n", i);
//      }
//
//    } /* endfor */
//    fclose( fOut );
//  }
#endif


  }
  else
  {
  } /* endif */

  return 0;
}

static DWORD CALLBACK
DebugOut( DWORD dwCookie,
                    LPBYTE pbBuff,
                    LONG cb,
                    LONG *pcb )
{
  FILE *fOut;                          // test output
  BYTE b;

  dwCookie; pcb;

  fOut      = fopen ( "D:\\RTFDump.OUT", "a" );
  b = pbBuff[ cb ]; pbBuff[ cb ] = EOS;
  fprintf(fOut, "%s", pbBuff);
  pbBuff[cb] = b;

  fclose( fOut );

  return 0;
}

#ifdef RTF_RTFDUMP
VOID RTFDUMP( PTBDOCUMENT pDoc, PSZ_W p )
{
  CHAR chText[2];
  EDITSTREAM es;
  FILE *fOut;                          // test output

  es.dwCookie    = (LONG) chText;
  es.dwError     = 0;
  es.pfnCallback = DebugOut;


  fOut      = fopen ( "D:\\RTFDump.OUT", "w" );
  if ( p )
  {
    fprintf(fOut, "%s\n",p);
  } /* endif */
  fclose( fOut );

  SendMessage( pDoc->hwndRichEdit, EM_STREAMOUT, SF_RTF, MP2FROMP( &es ));
}

VOID RTFDumpSel( PTBDOCUMENT pDoc, PSZ p )
{
  CHAR chText[2];
  EDITSTREAM es;
  FILE *fOut;                          // test output

  es.dwCookie    = (LONG) chText;
  es.dwError     = 0;
  es.pfnCallback = DebugOut;


  fOut      = fopen ( "D:\\RTFDump.OUT", "a" );
  if ( p )
  {
    fprintf(fOut, "%s :",p);
  } /* endif */
  fclose( fOut );

  SendMessage( pDoc->hwndRichEdit, EM_STREAMOUT, SF_TEXT /*| SF_UNICODE*/ | SFF_SELECTION, MP2FROMP( &es ));

  fOut      = fopen ( "D:\\RTFDump.OUT", "a" );
  fprintf(fOut, "\n");
  fclose( fOut );


}


VOID RTFDumpNoSel( PTBDOCUMENT pDoc, PSZ p )
{
  CHAR chText[2];
  EDITSTREAM es;
  FILE *fOut;                          // test output

  es.dwCookie    = (LONG) chText;
  es.dwError     = 0;
  es.pfnCallback = DebugOut;

  fOut      = fopen ( "D:\\RTFDump.OUT", "a" );
  if ( p )
  {
    fprintf(fOut, "%s :",p);
  } /* endif */
  fclose( fOut );

  SendMessage( pDoc->hwndRichEdit, EM_STREAMOUT, SF_TEXT /*| SF_UNICODE | SFF_SELECTION*/, MP2FROMP( &es ));

  fOut      = fopen ( "D:\\RTFDump.OUT", "a" );
  fprintf(fOut, "\n");
  fclose( fOut );


}


#endif


BOOL CALLBACK EnumLocalesProc( LPTSTR lpLocaleString )
{
  BOOL fOK = TRUE;
  lpLocaleString;
  return fOK;
};



BOOL EQFBCreateRichEditCtrl( PTBDOCUMENT pNewDoc, PFNWP pfnwp )
{
  BOOL fOK = TRUE;
  LONG lFlags = ES_MULTILINE | ES_SELECTIONBAR | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_BORDER | WS_VSCROLL;

  /********************************************************************/
  /* Only docType == STARGET_DOC is r/w                               */
  /********************************************************************/
  if ( pNewDoc->docType != STARGET_DOC )
  {
    lFlags |= ES_READONLY;
  } /* endif */


  fOK = UtlAlloc( (PVOID *)&(pNewDoc->pDispFileRTF), 0L, sizeof( DISPFILERTF ), ERROR_STORAGE );
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&(pNewDoc->pDispFileRTF->pBufferOverflow), 0L, 2048L, ERROR_STORAGE );
    if ( fOK )
    {
      pNewDoc->pDispFileRTF->ulBufferAllocated = 2048;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    /******************************************************************/
    /* do special handling for some of the languages like Thai, Bidi  */
    /******************************************************************/
    PDOCUMENT_IDA pIda = (PDOCUMENT_IDA)(((PSTEQFGEN)pNewDoc->pstEQFGen)->pDoc);
    LONG lFlagEx = 0L;
    HKL  hklReturn;

     // NewDoc->docType must be set!!
    EQFBDocSetCodePage( pNewDoc, pIda );
    hklReturn =  SwitchKeyboardForSpecLangs( pIda );
    if ( hklReturn && !hklChanged )
    {
      hklChanged = hklReturn;
    }

    if (IS_RTL(pNewDoc) && pNewDoc->docType == STARGET_DOC)
    {
      lFlagEx |= WS_EX_LAYOUTRTL;
    }

#ifdef USENEWRTFEDIT
    pNewDoc->hwndRichEdit =
        CreateWindowExW ( lFlagEx,
                          MSFTEDIT_CLASS,
                          L"",
                          lFlags,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          pNewDoc->hwndClient,
                          NULL,
                          (HINSTANCE)WinQueryAnchorBlock( pNewDoc->hwndClient ),
                          NULL );
#else
    pNewDoc->hwndRichEdit =
        CreateWindowExW ( lFlagEx,
                          RICHEDIT_CLASSW,
                          L"",
                          lFlags,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          pNewDoc->hwndClient,
                          NULL,
                          (HINSTANCE)WinQueryAnchorBlock( pNewDoc->hwndClient ),
                          NULL );
#endif

    if ( pNewDoc->hwndRichEdit )
    {
      RECT  rc;
      CHARFORMAT2  cfDefault;
      PDISPFILERTF pDispFileRTF = pNewDoc->pDispFileRTF;
      pDispFileRTF->orgRichEditProc = (LONG) SUBCLASSWND( pNewDoc->hwndRichEdit, ((PFNWP) pfnwp) );

      memset( &cfDefault, 0, sizeof(cfDefault) );
      cfDefault.cbSize = sizeof(cfDefault);
      cfDefault.dwEffects = 0; // | CFE_ITALIC;
      cfDefault.dwMask = CFM_FACE | CFM_COLOR;
      cfDefault.crTextColor = RGB( 0, 0, 0 );
      cfDefault.yHeight = 200;
      strcpy(cfDefault.szFaceName, "Wingdings");
      SendMessage( pNewDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_ALL, MP2FROMP( &cfDefault ));

      GetClientRect(pNewDoc->hwndClient, &rc);
      SetWindowPos( pNewDoc->hwndRichEdit, HWND_TOP, 0, 0, rc.right, rc.bottom, SWP_SHOWWINDOW | EQF_SWP_ACTIVATE);

      // request EN_SELCHANGE, EN_CHANGE, EN_PROTECTED, and EN_DROPFILES
      SendMessage(pNewDoc->hwndRichEdit, EM_SETEVENTMASK, 0,
              ENM_SELCHANGE | ENM_CHANGE | ENM_PROTECTED | EN_DROPFILES |
              ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_DRAGDROPDONE |
              ENM_CORRECTTEXT);
      /******************************************************************/
      /* set the insert state according to the requested state          */
      /*  The RTFControl always starts in insert mode                   */
      /******************************************************************/
      if ( !pNewDoc->EQFBFlags.inserting )
      {
        EQFBToggleInsertRTF( pNewDoc );
      } /* endif */

      /******************************************************************/
      /* define the text limit to be used                               */
      /******************************************************************/
      SendMessage( pNewDoc->hwndRichEdit, EM_EXLIMITTEXT, 0L, 0xEFFFFFFF );
      /******************************************************************/
      /* disable separate statusbar...                                  */
      /******************************************************************/
      ((PSTEQFGEN)pNewDoc->pstEQFGen)->bStatusBar[pNewDoc->docType] &= ~TP_WND_STATUSBAR;

      // check for the correct BIDI settings
  //    if ( ((pNewDoc->docType == STARGET_DOC) /*|| (pNewDoc->docType == SERVPROP_DOC)*/) && IS_RTL(pNewDoc))
  //    {
  //        BIDIOPTIONS bidiOpt;
  //        memset( &bidiOpt, 0, sizeof(BIDIOPTIONS));
  //        bidiOpt.cbSize = sizeof(BIDIOPTIONS);
  //        SendMessage( pNewDoc->hwndRichEdit, EM_GETBIDIOPTIONS, 0L, (LONG)&bidiOpt );
  //
  //        bidiOpt.wMask = BOM_CONTEXTREADING | BOM_CONTEXTALIGNMENT | BOM_NEUTRALOVERRIDE;
  //        bidiOpt.wEffects = BOE_CONTEXTREADING | BOE_CONTEXTALIGNMENT | BOE_NEUTRALOVERRIDE;
  //
  //        SendMessage( pNewDoc->hwndRichEdit, EM_SETBIDIOPTIONS, 0L, (LONG)&bidiOpt );
  //    } /* endif */


    }
    else
    {
#ifdef USENEWRTFEDIT
      PSZ pData = "RichEdit 4.1";
#else
      PSZ pData = RICHEDIT_CLASS;
#endif
      UtlError(ERROR_DID_NOT_START, MB_CANCEL, 1, &pData, EQF_ERROR);
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* load tagtable if necessary                                       */
  /********************************************************************/
  if ( fOK && !(pNewDoc->pDispFileRTF->pQFRTFTagTable) )
  {
    if ( TALoadTagTable( "QFRTF",
                         (PLOADEDTABLE *) &(pNewDoc->pDispFileRTF->pQFRTFTagTable),
                         TRUE, FALSE ) )
    {
      PSZ pData = "QFRTF";
      UtlError(ERROR_FILE_ACCESS_ERROR, MB_CANCEL,
               1, &pData, EQF_ERROR);
      fOK = FALSE;
    } /* endif */
  } /* endif */
  return fOK;
}



MRESULT APIENTRY
EQFBWNDPROCRTFCTRL( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 )
{
  MRESULT   mResult = FALSE;          // return value from window proc
  PTBDOCUMENT pDoc = ACCESSWNDIDA( GetParent(hwnd), PTBDOCUMENT );
  CHARFORMAT2  cfDefault;
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  
  if ( pDoc )
  {
    switch( msg )
    {
      case WM_EQF_UPDATERTFCTRL:
        switch ( mp1 )
        {
          case RTFCTRL_UPDATECURSOR:
            EQFBUpdateTBCursor( pDoc );
            break;
          case RTFCTRL_AUTOSPELL:
            if ( pDoc->ulWorkSeg )
            {
              pDoc->pTBSeg->SegFlags.Spellchecked = FALSE;
              EQFBWorkThreadTask ( pDoc, THREAD_SPELLSEGMENT );
            } /* endif */
            break;
          default:
            break;
        } /* endswitch */
        break;
      case WM_KILLFOCUS:
        mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        SendMessage( GetParent(hwnd), WM_NCACTIVATE, FALSE, 0L );
        break;
      case WM_EQF_SETFOCUS:
      case WM_SETFOCUS:
        STATUSBARRTF( pDoc );
        mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        SendMessage( GetParent(hwnd), WM_NCACTIVATE, TRUE, 0L );
        break;

      case WM_ERASEBKGND:
        return (LRESULT) 1; // we handle it...
        break;

      case WM_RBUTTONDOWN:
        STATUSBARRTF( pDoc );
        mResult = EQFBDispClass( GetParent(hwnd), msg, mp1, mp2, pDoc );
        break;
      case WM_BUTTON1DBLCLK:          // mark segment if in valid area
         pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
         EQFBUpdateTBCursor( pDoc );
         if (!EQFBOnTRNote(pDoc) )
         {
           mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
         }
         break;

      case WM_PAINT:
        mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        DrawFormatChars( pDoc );
        STATUSBARRTF( pDoc );
        break;

     case WM_VSCROLL:  // force a DrawFormatCharacter call in case of scrolling
        mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        if (pEQFBUserOpt->UserOptFlags.bVisibleSpace || pDoc->fAutoSpellCheck)
        {
          EQFBUpdateDispTable( pDoc );
        }

        DrawFormatChars( pDoc );
        STATUSBARRTF( pDoc );
        break;


     case WM_IME_ENDCOMPOSITION:
        pDoc->EQFBFlags.workchng = TRUE;
        EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );
        EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pDoc->pEQFBWorkSegmentW );
        mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        break;

      case WM_IME_STARTCOMPOSITION:
        /****************************************************/
        /* ignore request if we are on a protected character*/
        /* if not ...                                       */
        /* check if we are immediately following a protected*/
        /* character                                        */
        /* THIS IS A MICROSOFT RTF CONTROL BUG AND THEY ARE */
        /* WORKING ON IT                                    */
        /****************************************************/
        memset( &cfDefault, 0, sizeof( cfDefault ));
        cfDefault.cbSize = sizeof(cfDefault);
        SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );
        SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
        SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, SCF_SELECTION, (LONG) &cfDefault );
        if ( cfDefault.dwEffects & CFE_PROTECTED )
        {
            SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
            SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );
            SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, SCF_SELECTION, (LONG) &cfDefault );
            if ( cfDefault.dwEffects & CFE_PROTECTED )
            {
               HIMC  hIMC = ImmGetContext(hwnd);
               if (hIMC)
               {
                 ImmSetOpenStatus( hIMC, FALSE );
                 ImmReleaseContext(hwnd, hIMC );
               }
               MessageBeep(0);
            }
            else
            {
                mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
            }
        }
        else
        {
          mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        }
        break;

       case WM_IME_COMPOSITION:
         /****************************************************/
         /* ignore request if we are on a protected character*/
         /* if not ...                                       */
         /* check if we are immediately following a protected*/
         /* character                                        */
         /* THIS IS A MICROSOFT RTF CONTROL BUG AND THEY ARE */
         /* WORKING ON IT                                    */
         /****************************************************/
         memset( &cfDefault, 0, sizeof( cfDefault ));
         cfDefault.cbSize = sizeof(cfDefault);

         SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, SCF_SELECTION, (LONG) &cfDefault );
         if ( cfDefault.dwEffects & CFE_PROTECTED )
         {
           MessageBeep(0);
         }
         else
         if (mp2 & GCS_RESULTSTR)
         {
           pDoc->EQFBFlags.workchng = TRUE;
  //         EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );
  //         EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pDoc->pEQFBWorkSegmentW );
  //         SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );
  //         SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );

  //         mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
         }
         else
         {
           mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
         }
         break;


      case WM_CHAR:
        mResult = RichEditHandleWM_CHAR( hwnd, pDoc, msg, mp1, mp2 );
        if (!mResult)
        {
            mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        }
        break;

      case WM_INPUTLANGCHANGEREQUEST:
        pDoc->pDispFileRTF->usFunc = 0;
        pDoc->pDispFileRTF->hkl = (HKL)mp2;
        mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        break;

      case WM_INPUTLANGCHANGE:
        // don't change if character funct
        if ((pDoc->pDispFileRTF->usFunc == CHARACTER_FUNC) || (pDoc->pDispFileRTF->usFunc == STARTSEG_FUNC))
        {
          mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
          ActivateKeyboardLayout( pDoc->pDispFileRTF->hkl, 0 );
        }
        else
        {
          mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        }
        break;



      default:
        mResult = CALLWINDOWPROC( pDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
        break;
    } /* endswitch */
  } /* endif */
  return mResult;
}

MRESULT APIENTRY
EQFBWNDPROCRTF( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 )
{
    PTBDOCUMENT  pDoc;                  // pointer to doc ida
    MRESULT   mResult = FALSE;          // return value from window proc
    CHARRANGE chRange;

    pDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );

    switch( msg )
    {
      case WM_ERASEBKGND:
        mResult = 1;    // we deal later on ...
        break;
      case WM_EQF_FONTCHANGED:
        if ( (pDoc->docType != SSOURCE_DOC) ||
             (pDoc->docType == SSOURCE_DOC) && pDoc->pDispFileRTF->pHeader )
        {
          TBROWOFFSET TBCursor;
          TBCursor.ulSegNum = 0;
          SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
          EQFBGetSegFromCaretRTF( pDoc, &TBCursor, chRange.cpMin );
          EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );

          /************************************************************/
          /* reset font information used and calculate it new..       */
          /************************************************************/
          SETCURSOR(SPTR_WAIT);
          pDoc->pDispFileRTF->pHeader = NULL;
          EQFBDisplayFileNewRTF( pDoc );
          SETCURSOR(SPTR_ARROW);
          EQFBGotoSegRTF( pDoc, TBCursor.ulSegNum, TBCursor.usSegOffset );
        }
        break;

        case WM_NOTIFY:
          switch(((NMHDR *) mp2)->code)
          {
             case EN_PROTECTED:
               {
                 ENPROTECTED *penProtected = (ENPROTECTED *) mp2;

                 // allow change of protected attribute
                 if ( pDoc->pDispFileRTF->bRTFFill & (RTF_FILL | RTF_INITFILL | RTF_FASTFILL) )
                 {
                   return 0;
                 }
                 else
                 if(penProtected->msg == EM_SETCHARFORMAT &&
                    ((CHARFORMAT2 *) penProtected->lParam)->dwMask & CFM_PROTECTED)
                 {
                   return 0;
                 }
                 else
                 if ( penProtected->msg == WM_CUT )
                 {
                   // we arrive here in case of Drag/Drop outside of the active segment
                   // penProtected->msg = WM_COPY;
                   // penProtected->wParam = 0;
                   // penProtected->lParam = 0;
                   return 0;
                 }
                 else
                 if ( penProtected->msg == WM_MOUSEMOVE )
                 {
                   // we arrive here in case of Drag/Drop
                   return TRUE;
                 }
                 else
                 if ( penProtected->msg == 0 )
                 {
                   // we arrive here in case of an IME active --- seems to be a bug introduced by MICROSOFT
                   return 0;
                 }
                 else
                 {
                   CHARRANGE *pchRange = &penProtected->chrg;
                   CHARFORMAT2  cfDefault;
                   pchRange;
                   /****************************************************/
                   /* ignore request if we are on a protected character*/
                   /* if not ...                                       */
                   /* check if we are immediately following a protected*/
                   /* character                                        */
                   /* THIS IS A MICROSOFT RTF CONTROL BUG AND THEY ARE */
                   /* WORKING ON IT                                    */
                   /****************************************************/
                   memset( &cfDefault, 0, sizeof( cfDefault ));
                   cfDefault.cbSize = sizeof(cfDefault);
                   SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );
                   SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
                   SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, TRUE, (LONG) &cfDefault );
                   if ( cfDefault.dwEffects & CFE_PROTECTED )
                   {
                     mResult = TRUE;
                     MessageBeep(0);
                   }
                   else
                   {
                     if ( pDoc->EQFBFlags.inserting && (penProtected->wParam != VK_BACK)
                                                  && (penProtected->wParam != VK_DELETE))
                     {
                       CHAR_W    chText[4];
                       CHARRANGE chRange;
                       memset(&chText[0], 0, sizeof( chText ));
                       SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG)&chRange );
//#ifndef _GETTEXTRANGERTF
//                 {
//                       TEXTRANGE TextRange;
//                       memcpy(&TextRange.chrg, &chRange, sizeof( CHARRANGE ));
//                       TextRange.chrg.cpMax++;
//                       TextRange.lpstrText = &chText[1];
//                       SendMessage( pDoc->hwndRichEdit, EM_GETTEXTRANGE, 0, MP2FROMP( &TextRange ));
//                 }
//#else
                       GetTextRangeRTF( pDoc, &chText[1], 2, chRange.cpMin, 1);
//#endif
                       chText[0] = (CHAR) LOWORD(penProtected->wParam);

                       chRange.cpMax++;
                       SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
                       SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) &chText[0]);
                       SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
                       mResult = TRUE;
                     }
                     else
                     {
                       mResult = TRUE;
                       MessageBeep(0);
                     } /* endif */
                   } /* endif */
                 } /* endif */
               }
               break;
             case EN_MSGFILTER:
               {
                 MSGFILTER *     pmsgFilter = (MSGFILTER *) mp2;

                 mResult = HandleDummyMessage( hwnd, pDoc, pmsgFilter->msg,
                                          pmsgFilter->wParam, pmsgFilter->lParam);
               }
               break;

             case EN_SELCHANGE:
               if ( pDoc->pDispFileRTF->bRTFFill & (RTF_FILL | RTF_INITFILL))
               {
                 return 0;
               }
               else
               {
                 SELCHANGE *pSelChange = (SELCHANGE *) mp2;
                 CHARRANGE chrg = pSelChange->chrg;
                 if ( pSelChange->seltyp == SEL_EMPTY ) /*chrg.cpMin == chrg.cpMax )*/
                 {
                   EQFBFuncMarkClear( pDoc );
                 }
                 else
                 {
                   PEQFBBLOCK   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
                   TBROWOFFSET  tbCursor;

                   memcpy( &tbCursor, &pDoc->TBCursor, sizeof( tbCursor ));
                   memset( pstBlock, 0, sizeof(EQFBBLOCK) );

                   //if ( chrg.cpMin < chrg.cpMax  )
                   //{
                   //    pstBlock->pDoc = pDoc;
                   //    EQFBGetSegFromCaretRTF( pDoc, &tbCursor, chrg.cpMin );
                   //    pstBlock->ulSegNum = tbCursor.ulSegNum;
                   //    pstBlock->usStart  = tbCursor.usSegOffset;
                   //    EQFBGetSegFromCaretRTF( pDoc, &tbCursor, chrg.cpMax - 1 );
                   //    pstBlock->ulEndSegNum = tbCursor.ulSegNum;
                   //    pstBlock->usEnd    = tbCursor.usSegOffset;
                   //} /* endif */


                   EQFBGetSegFromCaretRTF( pDoc, &pDoc->TBCursor, chrg.cpMin );
                   EQFBFuncMarkBlock( pDoc );
                   if ( chrg.cpMax-1 > chrg.cpMin )
                   {
					 pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
                     EQFBFuncMarkBlock( pDoc );
                   }
                   memcpy( &pDoc->TBCursor, &tbCursor,  sizeof( tbCursor ));
                 }
               } /* endif */
               break;
/**********************************************************************/
/* The following function has some trouble in compact mode during     */
/* DragDrop                                                           */
/**********************************************************************/
//           case EN_DRAGDROPDONE:
//             /*******************************************************/
//             /* get worksegment and rewrite it ..                   */
//             /*******************************************************/
//             {
//               pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
//               EQFBUpdateTBCursor( pDoc );
//               if ( pDoc->ulWorkSeg == pDoc->TBCursor.ulSegNum )
//               {
//                 EQFBGetWorkSegRTF( pDoc, pDoc->TBCursor.ulSegNum );
//                 EQFBSetWorkSegRTF( pDoc, pDoc->TBCursor.ulSegNum,pDoc->EQFBWorkSegment );
//                 EQFBGotoSegRTF( pDoc, pDoc->TBCursor.ulSegNum, pDoc->TBCursor.usSegOffset );
//               } /* endif */
//             }
//             return 0;
//             break;
             default:
               break;
          } /* endswitch */
          break;

        case WM_SIZE:
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          if ( pDoc && pDoc->hwndRichEdit )
            BringWindowToTop( pDoc->hwndRichEdit );
          break;

        case WM_COMMAND:
          switch ( HIWORD( mp1 ) )
          {
            case EN_CHANGE:
              /****************************************************************/
              /* take care of any changes done during post edit mode          */
              /****************************************************************/
              if ( !(pDoc->pDispFileRTF->bRTFFill & RTF_FILL) && SendMessage( pDoc->hwndRichEdit, EM_GETMODIFY, 0L, 0L ))
              {
                if ( pDoc->EQFBFlags.PostEdit )
                {

                  BOOL fLock;
                  LOCKRICHEDIT( pDoc, fLock );
                  EQFBWorkSegCheck( pDoc );
                  UNLOCKRICHEDIT_TRUE( pDoc, fLock );
                  // set the modification flag again - we did reset it somewhere
                  SendMessage( pDoc->hwndRichEdit, EM_SETMODIFY, 1L, 0L );
                } /* endif */
              } /* endif */
              break;
            case EN_UPDATE:
              break;
            default:
              EQFBWndProc_CommandRTF (hwnd, mp1, mp2 );
              break;
          }
          break;


         /***************************************************************/
         /* ATTENTION: Under Windows WM_INITMENUPOPUP will be similar   */
         /*            to WM_INITMENU, except that mp2 contains the     */
         /*            zero based position of the SubMenu (POPUP).      */
         /*            This change is taken into consideration in the   */
         /*            EQFB.ID file.                                    */
         /***************************************************************/
        case WM_INITMENUPOPUP:
        case WM_EQF_INITMENU:
            EQFBInitMenu( pDoc, LOWORD( mp2 ));
            mResult = DEFWINDOWPROC ( hwnd, msg, mp1, mp2 );
            break;

        case WM_INITMENU:
          /*******************************************************************/
          /* modify System menu to look like a secondary window...           */
          /*******************************************************************/
          {
           HMENU hSysMenu = GetSystemMenu( hwnd, FALSE );
           if ( hSysMenu != NULL )
           {
             CHAR chText[80];
             USHORT usNum;
             HMENU hSysParent = GetSystemMenu( GETPARENT(hwnd), FALSE);

             chText[0] = EOS;
             GetMenuString( hSysParent, SC_CLOSE,
                            chText, sizeof( chText ), MF_BYCOMMAND);
             if ( chText[0] )
             {
               ModifyMenu( hSysMenu, SC_CLOSE, MF_BYCOMMAND, SC_CLOSE,
                           chText );
             } /* endif */
             RemoveMenu( hSysMenu, SC_TASKLIST, MF_BYCOMMAND );
             /*******************************************************/
             /* remove separator line - separator id = 0            */
             /*******************************************************/
             usNum = (USHORT)GetMenuItemCount( hSysMenu );
             if ( (usNum > 1) &&  !GetMenuItemID( hSysMenu, usNum-1)  )
             {
               RemoveMenu( hSysMenu, usNum-1, MF_BYPOSITION );
             } /* endif */
           }
          }
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          break;

        case WM_CLOSE:        // if it is the source doc only hide it
          switch ( pDoc->docType )
          {
             case SSOURCE_DOC:
                WinShowWindow( pDoc->hwndFrame , FALSE );
                SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                             WM_EQF_SETFOCUS,
                             0, MP2FROMP( pDoc->twin->hwndFrame ));
                break;
             case TRNOTE_DOC:
              {
                PTBDOCUMENT  pDocCurrent;
                pDocCurrent = pDoc->next;
                while ((pDocCurrent ->docType != STARGET_DOC ) &&
                        (pDocCurrent != pDoc ) )
                {
                  pDocCurrent = pDocCurrent->next;
                } /* endwhile */

                if (pDocCurrent->docType == STARGET_DOC)
                {
                  WinShowWindow( pDoc->hwndFrame , FALSE );
                    SendMessage( ((PSTEQFGEN)pDocCurrent->pstEQFGen)->hwndTWBS,
                                 WM_EQF_SETFOCUS,
                                 0, MP2FROMP( pDocCurrent->hwndFrame ));
                }
                else
                {
                  pDoc = EQFBFuncQuit( pDoc       );     // quit document
                }
              }
                break;
             case STARGET_DOC:
                {
                  USHORT usRc;
                  pDoc->EQFBFlags.AutoMode  = FALSE;       // reset auto mode

                  /********************************************************************/
                  /* adjust workseg changed to real value                             */
                  /********************************************************************/
                  if ( pDoc->pSaveSegW )
                  {
                     pDoc->EQFBFlags.workchng = (USHORT)(UTF16strcmp( pDoc->pSaveSegW, pDoc->pEQFBWorkSegmentW ) != 0);
                  }


                  usRc = EQF_XDOCNUM( (PSTEQFGEN)pDoc->pstEQFGen, 1, (PSZ)pDoc->pInBuf );
                  if ( !usRc && *pDoc->pInBuf )
                  {
                    // close current document and activate new one...
                    CHAR chObjName[MAX_EQF_PATH];
                    PSTEQFGEN pstEQFGen = (PSTEQFGEN)pDoc->pstEQFGen;
                    PTBDOCUMENT pDocCurrent = pDoc;
                    if (pDocCurrent->docType == STARGET_DOC)
                      pDocCurrent = pDocCurrent->next;
                    while ( pDocCurrent != pDoc && pDocCurrent->docType != STARGET_DOC )
                       pDocCurrent = pDocCurrent->next;
                    if (pDocCurrent == pDoc)
                       pDocCurrent = NULL;

                    EQFBFuncFile( pDoc );
                    if ( !EQF_XDOCNUM(pstEQFGen, 0, chObjName) )
                    {
                      EQFBTenvStart( pDocCurrent, chObjName, pstEQFGen );
                    }
                    else
                    {
                      EQFBQuit (hwnd);                       // quit documents
                    } /* endif */
                  }
                  else
                  {
                    EQFBQuit (hwnd);                       // quit documents
                  }
                }
                break;
             case OTHER_DOC:
                pDoc = EQFBFuncQuit( pDoc       );     // quit document
                break;
          } /* endswitch */

          break;

        case WM_EQF_AUTOTRANS:
          if ( pDoc->EQFBFlags.AutoMode )
          {
             EQFBFuncAutoTrans( pDoc );
          } /* endif */
          break;

        case WM_GETMINMAXINFO:
          /************************************************************/
          /* check that the windows min/max size is not jeopardized   */
          /************************************************************/
          {
            // minimum to be sized at least 4 rows
            if ( pDoc )
            {
              MINMAXINFO FAR* plpmmi = (MINMAXINFO FAR*) mp2;

              plpmmi->ptMinTrackSize.y =
                 (USHORT) ( WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR)  +
                            WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL ) +
                            2 * WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER))+
                            WinQuerySysValue( HWND_DESKTOP, SV_CYMENU ) +  //statusbar
                            4 * pDoc->cy;

              // minimium width 40 characters...
              plpmmi->ptMinTrackSize.x = MIN_CX_SIZE * pDoc->cx;

              /********************************************************/
              /* limit our maximum size to TranslationEnvironment --  */
              /* is jeopardized by Windows due to MDI limitations..   */
              /********************************************************/
              if ( pDoc->pstEQFGen )
              {
                HWND hwndTemp;
                RECT rect;

                hwndTemp = ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS;
                GetClientRect( hwndTemp, &rect );
                plpmmi->ptMaxSize.x = rect.right +
                                      2*WinQuerySysValue(HWND_DESKTOP,SV_CXSIZEBORDER)  ;
                plpmmi->ptMaxSize.y = rect.bottom +
                                      2*WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER)  ;
              } /* endif */
            } /* endif */
          }
          break;


        case WM_DESTROY:
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          break;

        case WM_EQF_SETFOCUS:
        case WM_SETFOCUS:
          /************************************************************/
          /* set correct doc. window to be active for find            */
          /************************************************************/
          if ( pFindData && pFindData->pDoc )
          {
            pFindData->pDoc = pDoc;
          } /* endif */

          if ( pDoc )
          {
            // activate the selected translation environment
            if ( (pDoc->docType == STARGET_DOC) && !pDoc->fTransEnvAct )
            {
              ActTransEnv( pDoc );
            } /* endif */

            BringWindowToTop( pDoc->hwndRichEdit );
            WinSendMsg( hwnd, WM_NCACTIVATE, TRUE, NULL );

            /********************************************************/
            /* display IME conversion window as hot-spot conversion */
            /* window at cursor place                               */
            /********************************************************/
            if ( pDoc->hlfIME )
            {
              ImeMoveConvertWin(pDoc, hwnd,
                                (SHORT)(pDoc->lCursorCol * pDoc->cx),
                                (SHORT)(pDoc->lCursorRow * pDoc->cy) );

              ImeSetFont( pDoc, hwnd, &pDoc->lf );
            } /* endif */
            mResult = DEFWINDOWPROC( hwnd, msg, mp1, mp2 );

            WinSetFocus( HWND_DESKTOP, pDoc->hwndRichEdit );
          } /* endif */
          break;

       case WM_ACTIVATE:
          WinSetFocus( HWND_DESKTOP, pDoc->hwndRichEdit );
          break;

        case WM_MOUSEACTIVATE:
          /********************************************************/
          /* inform TENV that the active window has changed.....  */
          /********************************************************/
          if ( (hwnd != GetFocus()) && (pDoc->pstEQFGen ))
          {
            WinSendMsg( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                        WM_EQF_SETFOCUS,
                        0, MP2FROMHWND( hwnd ));
            BringWindowToTop( hwnd );
            SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                         WM_EQF_UPDATESTATUSBAR_TEXT, 0, (LONG)"" );
			// SVT604: S000007 recalculate position
		    pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
		    STATUSBARRTF( pDoc );
          } /* endif */
          mResult = MA_ACTIVATE;  // ANDEAT;
          break;

        /**************************************************************/
        /* forward menu selection to our MDI client window, if it is  */
        /* not '+' which might activate our sysmenu ....              */
        /**************************************************************/
       case WM_MENUCHAR:
         if ( (mp1 == '-') && (GetWindowLong(hwnd, GWL_STYLE) & WS_SYSMENU) )
         {
            mResult = DEFWINDOWPROC( hwnd, msg, mp1, mp2 );
         }
         else
         {
           mResult = SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                                  msg,
                                  mp1, mp2 );
         } /* endif */
         break;


        case WM_MENUSELECT:
          if ( (mp1 == '-') && (GetWindowLong(hwnd, GWL_STYLE) & WS_SYSMENU) )
          {
            mResult = DEFWINDOWPROC( hwnd, msg, mp1, mp2 );
          }
          else
          {
            mResult = SendMessage( ((PSTEQFGEN)pDoc->pstEQFGen)->hwndTWBS,
                                   msg,
                                   mp1, mp2 );
          } /* endif */
          break;

       case WM_BUTTON1DOWN:    // Position cursor to pointer
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          break;

       case WM_MOUSEMOVE:      // If button 1 down, send mark block to editor
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          break;

       case WM_BUTTON1UP:             // Send
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          break;

       case WM_TIMER:
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          break;

       case WM_BUTTON1DBLCLK:          // mark segment if in valid area
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          break;

       case WM_BUTTON2DBLCLK:  // Send un-mark to editor
          mResult = EQFBDispClass( hwnd, msg, mp1, mp2, pDoc );
          break;


        default:
          mResult = DEFWINDOWPROC( hwnd, msg, mp1, mp2 );
          break;

    } /* switch */

    return mResult;
}



VOID EQFBCreateRichEditTemplate( PTBDOCUMENT pDoc, HWND hwndParent, PDISPFILERTF pDispFile )
{
  /********************************************************************/
  /* Create Window                                                    */
  /********************************************************************/
  HWND hwndEdit;
  LONG lFlagEx = 0L;

  if (IS_RTL(pDoc) && pDoc->docType == STARGET_DOC)
  {
    lFlagEx |= WS_EX_LAYOUTRTL;
  }

#ifdef USENEWRTFEDIT
  hwndEdit =
        CreateWindowExW ( lFlagEx,
                          MSFTEDIT_CLASS,
                          L"",
                          ES_MULTILINE |
                          ES_SELECTIONBAR |
                          WS_CHILD |
                          WS_CLIPSIBLINGS |
                          0,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          hwndParent,
                          NULL,
                          (HINSTANCE)WinQueryAnchorBlock( hwndParent ),
                          NULL );
#else
  hwndEdit =
        CreateWindowExW ( lFlagEx,
                          RICHEDIT_CLASSW,
                          L"",
                          ES_MULTILINE |
                          ES_SELECTIONBAR |
                          WS_CHILD |
                          WS_CLIPSIBLINGS |
                          0,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          hwndParent,
                          NULL,
                          (HINSTANCE)WinQueryAnchorBlock( hwndParent ),
                          NULL );
#endif

  /********************************************************************/
  /* Write out template                                               */
  /********************************************************************/
  if ( hwndEdit )
  {
    EDITSTREAM es;
    USHORT i;
    CHAR chText[3];
    CHARFORMAT2 CharFormat2;

    CHARFORMAT2  cfDefault;

    es.dwCookie    = (LONG) pDispFile;
    es.dwError     = 0;
    es.pfnCallback = EditStreamOutCallBack;

    memset( &cfDefault, 0, sizeof(cfDefault) );
    cfDefault.cbSize = sizeof(cfDefault);
    cfDefault.dwEffects = CFE_ITALIC | CFE_BOLD | CFE_HIDDEN;
    cfDefault.dwMask = CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_ITALIC | CFM_BOLD;
    cfDefault.dwMask |= CFM_COLOR | CFM_BACKCOLOR | CFM_UNDERLINETYPE | CFM_STRIKEOUT | CFM_PROTECTED;
    cfDefault.crTextColor = RGB( 1, 1, 1);
    cfDefault.yHeight = 10;
    strcpy(cfDefault.szFaceName, "Wingdings");


    pDoc->pDispFileRTF->bRTFFill = RTF_FILL | RTF_INITFILL;

    SendMessage( hwndEdit, EM_SETCHARFORMAT, SCF_ALL, MP2FROMP( &cfDefault ));
    cfDefault.crTextColor = RGB( 120, 120, 120 );
    chText[0] = '\n';
    chText[1] = EOS;

    SendMessage( hwndEdit, EM_REPLACESEL, FALSE, MP2FROMP(&chText[0]) );

    for (i=0 ; i < MAX_MAXRTFFONTS ; i++ )
    {
      EQFBSegColRTFInit( &CharFormat2, i );
      if ( CharFormat2.dwEffects & CFE_BOLD )
      {
        cfDefault.dwEffects &= ~CFE_BOLD;
      }
      else
      {
        cfDefault.dwEffects |= CFE_BOLD;
      } /* endif */

      if ( CharFormat2.dwEffects & CFE_ITALIC )
      {
        cfDefault.dwEffects &= ~CFE_ITALIC;
      }
      else
      {
        cfDefault.dwEffects |= CFE_ITALIC;
      } /* endif */
      // SVT604 S000022 do the same for CFE_UNDERLINE and CFE_STRIKEOUT
      if ( CharFormat2.dwEffects & CFE_UNDERLINE )
      {
        cfDefault.dwEffects &= ~CFE_UNDERLINE;
        cfDefault.bUnderlineType = CFU_UNDERLINENONE;
      }
      else
      {
        cfDefault.dwEffects |= CFE_UNDERLINE;
        cfDefault.bUnderlineType = CFU_UNDERLINE;
      } /* endif */

      if ( CharFormat2.dwEffects & CFE_STRIKEOUT )
      {
        cfDefault.dwEffects &= ~CFE_STRIKEOUT;
      }
      else
      {
        cfDefault.dwEffects |= CFE_STRIKEOUT;
      } /* endif */

      SendMessage( hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP( &cfDefault ));
      SendMessage( hwndEdit, EM_REPLACESEL, FALSE, MP2FROMP(&chText[0]) );
      SendMessage( hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION,
                   MP2FROMP( &CharFormat2 ));
      SendMessage( hwndEdit, EM_REPLACESEL, FALSE, MP2FROMP(&chText[0]) );
    } /* endfor */
    /******************************************************************/
    /* fill in special start font -- based on source font selected    */
    /******************************************************************/
    SendMessage( hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP( &cfDefault ));
    SendMessage( hwndEdit, EM_REPLACESEL, FALSE, MP2FROMP(&chText[0]) );


    EQFBSegColRTFInit( &CharFormat2, EXT_T_SOURCE );
    CharFormat2.dwEffects |= CFE_PROTECTED;
    CharFormat2.dwMask |= CFM_FACE | CFM_COLOR | CFM_PROTECTED | CFM_CHARSET;
    CharFormat2.bCharSet = SYMBOL_CHARSET;
    CharFormat2.crTextColor = RGB( 0, 200, 0 );
    strcpy(CharFormat2.szFaceName, "Wingdings");
    SendMessage( hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP( &CharFormat2 ));
    SendMessage( hwndEdit, EM_REPLACESEL, FALSE, MP2FROMP(&chText[0]) );

    SendMessage( hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP( &cfDefault ));
    SendMessage( hwndEdit, EM_REPLACESEL, FALSE, MP2FROMP(&chText[0]) );

    pDoc->pDispFileRTF->bRTFFill = 0;


    pDoc->pDispFileRTF->lPos = 0;
    SendMessage( hwndEdit, EM_STREAMOUT, SF_RTF, MP2FROMP( &es ));

    DestroyWindow( hwndEdit );
  } /* endif */
}


VOID  EQFBDispFileRTF( PTBDOCUMENT pDoc )
{
  EDITSTREAM es;

  es.dwCookie = (LONG) pDoc->pDispFileRTF;
  es.dwError = 0;
  es.pfnCallback = EditStreamInCallBack;

  if ( !pDoc->pDispFileRTF->pHeader )
  {
    EQFBCreateRichEditTemplate( pDoc, pDoc->hwndClient, pDoc->pDispFileRTF );
  } /* endif */


#ifdef RTF_DEBUG
    MEASURE_TIME( ulStart, 0L );
#endif

  (pDoc->pDispFileRTF)->ulSegNum = 0;
  (pDoc->pDispFileRTF)->pDoc = pDoc;
  (pDoc->pDispFileRTF)->lPos = 0;
  (pDoc->pDispFileRTF)->lPoscb = 0;

  pDoc->pDispFileRTF->bRTFFill = RTF_FASTFILL | RTF_INITFILL;
  SendMessage( pDoc->hwndRichEdit, EM_STREAMIN, SF_RTF, MP2FROMP( &es ));
  pDoc->pDispFileRTF->bRTFFill = 0;

  SendMessage(pDoc->hwndRichEdit, EM_SETMODIFY, 0L,0L);

#ifdef RTF_DEBUG
    {
      ULONG ulDiff;
      MEASURE_TIME( ulDiff, ulStart );
      MEASURE_TIME_FILE( "File display time", ulDiff );
    } /* endif */

  /********************************************************************/
  /* write out file ...                                               */
  /********************************************************************/
//  if ( pDoc->docType == STARGET_DOC )
//  {
//    RTFDUMP( pDoc, NULL );
//  } /* endif */
#endif
  if ( hklChanged )
  {
    ActivateKeyboardLayout( hklChanged, 0 /*KLF_SETFORPROCESS*/);
    hklChanged = NULL;
  } /* endif */


  return;
}

VOID  EQFBFindNOPKind
(
    PTBDOCUMENT pDoc,
    ULONG       ul,
    USHORT      qLastSegStatus,
    USHORT      qSegStatus,
    PUSHORT     pusKindOfNOP
)
{
   PTBSEGMENT pNOPSeg = NULL;
   ULONG      ulNOPSegNum = 0;
   PSZ_W      pData = NULL;

    *pusKindOfNOP = NOTSPEC;

    if ((pDoc->DispStyle == DISP_SHRINK) ||
        (pDoc->DispStyle == DISP_COMPACT) )
    {
       if ((qLastSegStatus == QF_NOP) && (qSegStatus == QF_NOP))
       {
         *pusKindOfNOP = CONSECUTIVE;
       }
       else
       {
          ulNOPSegNum = ul;
          pNOPSeg = EQFBGetShrinkSeg( pDoc, &ulNOPSegNum, 1);
          pData = pNOPSeg->pDataW;
          if (pData[UTF16strlenCHAR(pData) - 1] == LF )
          {
            *pusKindOfNOP = FIRSTNOP_WITHLF;
          }
          else
          {
            *pusKindOfNOP = FIRSTNOP_WITHOUTLF;
          } /* endif */
       } /* endif */
    } /* endif */

 return;
}

VOID EQFBAddSegRTF
(
   PTBDOCUMENT pDoc,
   PTBSEGMENT  pSeg,
   PSZ_W       pData,
   ULONG       ulI,
   USHORT      usKindOfNOP
 )
{
  CHAR_W chBuffer[ 2 * MAX_SEGMENT_SIZE ];
  CHARFORMAT2  cfDefault;
  USHORT usState = UNPROTECTED_CHAR;        // status of character
  pData;
  if ( pSeg->qStatus == QF_NOP )
  {
    usState = PROTECTED_CHAR;
  }

  memset( &cfDefault, 0, sizeof(cfDefault) );
  cfDefault.cbSize = sizeof(cfDefault);
  if (pSeg && pSeg->SegFlags.Joined )
  {
    swprintf( chBuffer, RTF_JOINED, ulI, L"A", 1);
  }
  else
  {
    swprintf( chBuffer, RTF_HIDDEN, ulI, L"A", 1);
  } /* endif */

  SetHidden( pDoc, chBuffer, &cfDefault, L"" );


  if (pSeg && !pSeg->SegFlags.Joined )
  {
    CHAR_W c;
    c = pSeg->pDataW[pSeg->usLength]; pSeg->pDataW[pSeg->usLength] = EOS;
    DisplaySegRTF( pDoc, pSeg, pSeg->pDataW, usKindOfNOP );
    pSeg->pDataW[pSeg->usLength] = c;
  } /* endif */
}

VOID RTFInlineEscape( PSZ_W pT1, PSZ_W pT2 )
{
  CHAR_W c;
  while ( (c = *pT1 = *pT2) != NULC )
  {
    switch ( c )
    {
      case '\n':
        *pT1++ = '\\';
        *pT1   = c;
        break;
      case '\"':
        *pT1++ = '\"';
        *pT1   = c;
        break;
      default:
        break;
    } /* endswitch */
    pT1++; pT2++;
  } /* endwhile */
  *pT1 = EOS;
}

USHORT RTFInlineDeEscape( PSZ_W pT1, PSZ_W pT2, USHORT usLen )
{
  CHAR_W c;
  USHORT usNewLen = 0;
  int  i;

  for ( i=0; i < usLen; i++ )
  {
    c = *pT1 = *pT2;
    switch ( c )
    {
      case '\\':
        if ( *(pT2+1) == '\n' )
        {
          pT2++;  i++;
          *pT1 = '\n';
        } /* endif */
        break;
      case '\"':
        if ( *(pT2+1) == '\"' )
        {
          pT2++;  i++;
        } /* endif */
        break;
      default:
        break;
    } /* endswitch */
    pT1++; pT2++;
    usNewLen++;
  } /* endfor */
  *pT1 = EOS;
  return usNewLen;
}


VOID EQFBMakeRTFEscapedLF ( PSZ_W pT1, PSZ_W pT2 )
{
  CHAR_W c;
  while ( (c = *pT1 = *pT2) != NULC )
  {
    switch ( c )
    {
      case '\n':
        *pT1++ = '\\';
        *pT1   = c;
        break;
      default:
        break;
    } /* endswitch */
    pT1++; pT2++;
  } /* endwhile */
  *pT1 = EOS;
}

static
BOOL CheckForLF( PSZ_W pData )
{
  BOOL   fLF = FALSE;
  CHAR_W c;
  PSZ_W    pTemp = pData;
  while ( (c = *pTemp++) != NULC )
  {
    if ( c == '\n' )
    {
      fLF = TRUE;
      break;
    } /* endif */
  } /* endwhile */
  return fLF;
}

VOID DisplaySegRTF
(
  PTBDOCUMENT pDoc,
  PTBSEGMENT  pSeg,
  PSZ_W       pData,
  USHORT      usKindOfNOP
)
{
  PDISPFILERTF pDisp = pDoc->pDispFileRTF;
  USHORT usColPos = 0;
  CHARFORMAT2 CharFormat2;
  CHARFORMAT2 CharFormat;
  CHAR_W chBuffer[ MAX_SEGMENT_SIZE ];
  CHAR_W chBufferTemp[ MAX_SEGMENT_SIZE ];

  PSZ_W  pRTFFont;
  BYTE   b = pDisp->bRTFFill;
  USHORT usHLState = NO_HIGHLIGHT;
  USHORT usLastType = UNPROTECTED_CHAR;
  PSTARTSTOP pstCurrent = NULL;
  PSZ_W  pTemp;
  CHAR_W c;

  /********************************************************************/
  /* setup for streamin display                                       */
  /********************************************************************/
  if ( !(pDisp->bRTFFill & RTF_INITFILL) )
  {
    ULONG ulHeaderLen;
    pDisp->lPoscb = pDisp->cb;
    pDisp->lPos   = 0;
    ulHeaderLen = strlen( pDisp->pHeader );
    if ( pDisp->ulBufferAllocated < (ulHeaderLen + 50) )
    {
      ULONG ulLen = pDisp->ulBufferAllocated + RTFBUFFER_INC;
      UtlAlloc( (PVOID *)&(pDisp->pBufferOverflow), pDisp->ulBufferAllocated, ulLen, ERROR_STORAGE);
      if ( pDisp->pBufferOverflow )
      {
        pDisp->ulBufferAllocated = ulLen;
      } /* endif */
    } /* endif */
    memset( pDisp->pBufferOverflow, 0, pDisp->ulBufferAllocated);
    memcpy( pDisp->pBufferOverflow, pDisp->pHeader, ulHeaderLen);
    pDisp->lPos += ulHeaderLen;
    pDisp->bRTFFill = RTF_FASTFILL;
  }

  pDisp->bRTFFill |= RTF_FILL;

  TACreateProtectTableW( pData,
                         pDoc->pDocTagTable,
                         usColPos,
                         (PTOKENENTRY) pDoc->pTokBuf,
                         TOK_BUFFER_SIZE,
                         (PSTARTSTOP *) &(pSeg->pusBPET),
                         pDoc->pfnUserExit, pDoc->pfnUserExitW, pDoc->ulOemCodePage );

  if ( pSeg->qStatus == QF_NOP )
  {
    memset( &CharFormat2, 0, sizeof( CHARFORMAT2 ));
    CharFormat2.cbSize = sizeof(CharFormat2);
    if ( pSeg->pusBPET )
    {
      pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
    }


    if ( pSeg->pusBPET &&  (pstCurrent->usType == TRNOTE_CHAR) )
    {
      while (pstCurrent->usType)
      {
       switch (pstCurrent->usType)
       {
         case TRNOTE_CHAR:
           usHLState = EQFBGetHLType(pSeg, (SHORT)pstCurrent->usStart);
           pRTFFont = EQFBSegColRTF( &CharFormat2, pSeg,
                                   TRNOTE_CHAR, usHLState,
                                   pDoc->DispStyle, pDoc->EQFBFlags.PostEdit,
                                   pDisp);
           usHLState = NO_HIGHLIGHT;
           pTemp = pData+pstCurrent->usStop + 1;
           c = *pTemp;
           *pTemp = EOS;
           RTFInlineEscape( chBufferTemp, pData + pstCurrent->usStart);
           swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp, L"");
           SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont );
           if ( c == '\n')
           {
             UTF16strcpy(chBuffer, pDoc->chTRNoteLFAbbrW);
           }
           else
           {
             UTF16strcpy( chBuffer, pDoc->chTRNoteAbbrW );
           }
           SetProtected( pDoc, chBuffer, &CharFormat2, pRTFFont);

           SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont);
           *pTemp = c;
           break;
           default:
             // TODO @@@
             break;
         } /* endswitch */
         pstCurrent++;
      } /* endwhile */

    }
    else
    {
      pRTFFont   = EQFBSegColRTF( &CharFormat2, pSeg, PROTECTED_CHAR, 0,
                                  pDoc->DispStyle, pDoc->EQFBFlags.PostEdit, pDisp);

      switch ( pDoc->DispStyle )
      {
        case DISP_PROTECTED:
          SetProtected( pDoc, pData, &CharFormat2, pRTFFont  );
          break;
        case DISP_UNPROTECTED:
          SetNormal( pDoc, pData, &CharFormat2, pRTFFont );
          break;
        case DISP_COMPACT:
        case DISP_SHRINK:
          /********************************************************/
          /* store the tagging internally and add the parts to be */
          /* displayed                                            */
          /********************************************************/
          RTFInlineEscape( chBufferTemp, pData );
          swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp, L"" );
          SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont );
          if ((usKindOfNOP == FIRSTNOP_WITHLF )
              || (usKindOfNOP == FIRSTNOP_WITHOUTLF) )
          {
            if (usKindOfNOP == FIRSTNOP_WITHLF )
            {
              swprintf( chBufferTemp, L"%s\n", pDoc->szOutTagAbbrW );
              SetProtected( pDoc, chBufferTemp, &CharFormat2, pRTFFont );
            }
            else
            {
              SetProtected( pDoc, &pDoc->szOutTagAbbrW[0], &CharFormat2, pRTFFont );
            } /* endif */
          } /* endif */
          SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont );
          break;
        case DISP_SHORTEN:
          /********************************************************/
          /* store the tagging internally and add the parts to be */
          /* displayed                                            */
          /********************************************************/
          RTFInlineEscape( chBufferTemp, pData );
          swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp, L"" );
          SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont );

          memset( chBuffer, 0, sizeof( chBuffer ));
          BuildRTFShorten(chBuffer, pData, (USHORT)UTF16strlenCHAR(pData));
          SetProtected( pDoc, chBuffer, &CharFormat2, pRTFFont);

          SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont );
          break;
        case DISP_WYSIWYG:
          switch ( pDisp->WYSIWYGType )
          {
            case WYSIWYG_HTML:
              WYSIWYG_HTML_Display( pDoc, pData, &CharFormat2, pSeg );
              break;
            case WYSIWYG_RTF:
              WYSIWYG_RTF_Display( pDoc, pData, &CharFormat2, pSeg );
              break;
            default:
              RTFInlineEscape( chBufferTemp, pData );
              swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp,L"" );
              SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont );
              SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont );
              break;
          } /* endswitch */
          break;
        case DISP_HIDE:
          RTFInlineEscape( chBufferTemp, pData );
          swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp,L"" );
          SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont  );
          SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont );
          break;
        default:
          break;
      } /* endswitch */
    } /* endif */
  }
  else
  {

    if ( pSeg->pusBPET )
    {

      pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
      usLastType = UNPROTECTED_CHAR;

      while ( pstCurrent->usType )
      {
        switch ( pstCurrent->usType )
        {
          case  TAGPROT_CHAR:
          case PROTECTED_CHAR:
            memset( &CharFormat2, 0, sizeof( CHARFORMAT2 ));
            CharFormat2.cbSize = sizeof(CharFormat2);
            usHLState = EQFBGetHLType( pSeg, (SHORT)pstCurrent->usStart);
            pRTFFont = EQFBSegColRTF( &CharFormat2, pSeg, pstCurrent->usType, usHLState,
                           pDoc->DispStyle, pDoc->EQFBFlags.PostEdit, pDisp);
            usHLState = NO_HIGHLIGHT;
            pTemp = pData+pstCurrent->usStop + 1;
            c = *pTemp; *pTemp = EOS;
            switch ( pDoc->DispStyle )
            {
              case DISP_PROTECTED:
              case DISP_SHRINK:
                SetProtected( pDoc, pData+pstCurrent->usStart, &CharFormat2, pRTFFont);
                break;
              case DISP_UNPROTECTED:
                SetNormal( pDoc, pData+pstCurrent->usStart, &CharFormat2, pRTFFont );
                break;
              case DISP_SHORTEN:
                /********************************************************/
                /* store the tagging internally and add the parts to be */
                /* displayed                                            */
                /********************************************************/
                RTFInlineEscape( chBufferTemp, pData+pstCurrent->usStart );
                swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp,L"" );
                SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont );

                memset( chBuffer, 0, sizeof( chBuffer ));
                BuildRTFShorten( chBuffer,
                                 pData+pstCurrent->usStart,
                           (USHORT)(pstCurrent->usStop - pstCurrent->usStart + 1) );
                SetProtected( pDoc, chBuffer, &CharFormat2, pRTFFont);

                SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont);
                break;
              case DISP_COMPACT:
                /********************************************************/
                /* store the tagging internally and add the parts to be */
                /* displayed                                            */
                /********************************************************/
                RTFInlineEscape( chBufferTemp, pData+pstCurrent->usStart );
                swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp, L"" );
                SetHidden( pDoc, chBuffer, &CharFormat2, NULL );
                /******************************************************/
                /* if consecutive inline tags, display only one       */
                /* szInTagAbbr, add only the hidden part for all tags */
                /******************************************************/
                if (usLastType != PROTECTED_CHAR )
                {
                   if ( CheckForLF( pData+pstCurrent->usStart ) )
                   {
                     USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
                     swprintf( chBufferTemp, L"%s\n", pEQFBUserOpt->szInTagAbbr );
                     SetProtected( pDoc, chBufferTemp, &CharFormat2, pRTFFont);
                   }
                   else
                   {
                     SetProtected( pDoc, &pDoc->szInTagAbbrW[0], &CharFormat2, pRTFFont);
                   } /* endif */
                } /* endif */

                SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont );
                break;

              case DISP_WYSIWYG:
                switch ( pDisp->WYSIWYGType )
                {
                  case WYSIWYG_HTML:
                    WYSIWYG_HTML_Display( pDoc, pData+pstCurrent->usStart, &CharFormat2, pSeg );
                    break;
                  case WYSIWYG_RTF:
                    WYSIWYG_RTF_Display( pDoc, pData+pstCurrent->usStart, &CharFormat2, pSeg );
                    break;
                  default:
                    /********************************************************/
                    /* store the tagging internally and add the parts to be */
                    /* displayed                                            */
                    /********************************************************/
                    RTFInlineEscape( chBufferTemp, pData+pstCurrent->usStart );
                    swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp, L"" );
                    memcpy( &CharFormat, &CharFormat2, sizeof( CharFormat ));
                    CharFormat2.dwMask |= (CFM_PROTECTED | CFM_HIDDEN);
                    CharFormat2.dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
                    SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont );
                    /******************************************************/
                    /* if consecutive inline tags, display only one       */
                    /* szInTagAbbr, add only the hidden part for all tags */
                    /******************************************************/
                    if (usLastType != PROTECTED_CHAR )
                    {
                       memcpy( &CharFormat2, &CharFormat, sizeof( CharFormat ));
                       CharFormat2.dwMask |= (CFM_PROTECTED | CFM_HIDDEN);
                       CharFormat2.dwEffects |= CFE_PROTECTED;
                       if ( CheckForLF( pData+pstCurrent->usStart ) )
                       {
                         USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
                         swprintf( chBufferTemp, L"%s\n", pEQFBUserOpt->szInTagAbbr );
                         SetProtected( pDoc, chBufferTemp, &CharFormat2, pRTFFont);
                       }
                       else
                       {
                         SetProtected( pDoc, &pDoc->szInTagAbbrW[0], &CharFormat2, pRTFFont);
                       } /* endif */
                    } /* endif */

                    memcpy( &CharFormat2, &CharFormat, sizeof( CharFormat ));
                    CharFormat2.dwMask |= (CFM_PROTECTED | CFM_HIDDEN);
                    CharFormat2.dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
                    SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont );
                    break;
                } /* endswitch */

                break;
              case DISP_HIDE:
                RTFInlineEscape( chBufferTemp, pData+pstCurrent->usStart );
                swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp,L"" );
                SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont );
                SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont );
                break;
              default:
                break;
            } /* endswitch */
            *pTemp = c;
            break;
          case TRNOTE_CHAR:
             memset( &CharFormat2, 0, sizeof( CHARFORMAT2 ));
             CharFormat2.cbSize = sizeof(CharFormat2);
             usHLState = EQFBGetHLType( pSeg, (SHORT) pstCurrent->usStart);
             pRTFFont = EQFBSegColRTF( &CharFormat2, pSeg,
                                       TRNOTE_CHAR, usHLState,
                                       pDoc->DispStyle, pDoc->EQFBFlags.PostEdit,
                                       pDisp);
             usHLState = NO_HIGHLIGHT;
             pTemp = pData+pstCurrent->usStop + 1;
             c = *pTemp;
             *pTemp = EOS;
             RTFInlineEscape( chBufferTemp, pData + pstCurrent->usStart);
             swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp, L"");
             SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont );

             SetProtected( pDoc, &pDoc->chTRNoteAbbrW[0], &CharFormat2,
                           pRTFFont);
             SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont);
             *pTemp = c;
          break;
          case  UNPROTECTED_CHAR:
          default:
            if ( pDoc->DispStyle == DISP_WYSIWYG )
            {
              CHARFORMAT2 CharFormWYSIWYG;
              memcpy( &CharFormWYSIWYG, &CharFormat2, sizeof( CHARFORMAT2 ));
              memset( &CharFormat2, 0, sizeof( CHARFORMAT2 ));
              CharFormat2.cbSize = sizeof(CharFormat2);
              usHLState = EQFBGetHLType( pSeg, (SHORT)pstCurrent->usStart);
              pRTFFont = EQFBSegColRTF( &CharFormat2, pSeg, UNPROTECTED_CHAR, usHLState,
                             pDoc->DispStyle, pDoc->EQFBFlags.PostEdit, pDisp );
              /********************************************************/
              /* use the currently active mask due to inline tagging  */
              /********************************************************/
              CharFormat2.dwMask    |= CharFormWYSIWYG.dwMask;
              CharFormat2.dwEffects |= CharFormWYSIWYG.dwEffects;
              CharFormat2.dwEffects &= ~CFE_HIDDEN;
              CharFormat2.dwMask |= CFM_PROTECTED;
              swprintf( chBufferTemp, L"%s%s", pRTFFont, chRTFFont );
              pRTFFont = &chBufferTemp[0];
              CleanIBU( pRTFFont );
            }
            else
            {
              memset( &CharFormat2, 0, sizeof( CHARFORMAT2 ));
              CharFormat2.cbSize = sizeof(CharFormat2);
              usHLState = EQFBGetHLType( pSeg, (SHORT)pstCurrent->usStart);
              pRTFFont = EQFBSegColRTF( &CharFormat2, pSeg, UNPROTECTED_CHAR, usHLState,
                             pDoc->DispStyle, pDoc->EQFBFlags.PostEdit, pDisp );
              CharFormat2.dwMask |= CFM_PROTECTED;

              // do special handling for Arabic
              if (IS_RTL(pDoc) && ((pDoc->docType == STARGET_DOC) /*|| (pDoc->docType == SERVPROP_DOC)*/))
              {
                UTF16strcpy( &chBufferTemp[0], L"\\rtldoc\\rtlpar\\qr\\rtlch\\rtlsect");
                pRTFFont = UTF16strcat(&chBufferTemp[0], pRTFFont);
              }
            } /* endif */
            pTemp = pData+pstCurrent->usStop + 1;
            c = *pTemp; *pTemp = EOS;
            if ( (pDoc->EQFBFlags.PostEdit && (pSeg->qStatus == QF_XLATED)) ||
                 (pSeg->qStatus == QF_CURRENT) )
            {
              SetNormal( pDoc, pData+pstCurrent->usStart, &CharFormat2, pRTFFont);
            }
            else
            {
              SetProtected( pDoc, pData+pstCurrent->usStart, &CharFormat2, pRTFFont);
            } /* endif */
            *pTemp = c;
            break;
         } /* endswitch */
         usLastType = pstCurrent->usType;
         pstCurrent++;
       } /* endwhile */

    }
    else
    {
      memset( &CharFormat2, 0, sizeof( CHARFORMAT2 ));
      CharFormat2.cbSize = sizeof(CharFormat2);
      pRTFFont = EQFBSegColRTF( &CharFormat2, pSeg, UNPROTECTED_CHAR, 0,
                     pDoc->DispStyle, pDoc->EQFBFlags.PostEdit, pDisp );
      if ( pDoc->EQFBFlags.PostEdit || (pSeg->qStatus == QF_CURRENT) )
      {
        SetNormal( pDoc, pData, &CharFormat2, pRTFFont);
      }
      else
      {
        SetProtected( pDoc, pData, &CharFormat2, pRTFFont);
      } /* endif */

    } /* endif */
  } /* endif */

  if (!(pDisp->bRTFFill & RTF_INITFILL))
  {
    pDisp->pBufferOverflow[pDisp->lPos++] = '}';
    pDisp->pBufferOverflow[pDisp->lPos++] = EOS;
    pDisp->lPoscb = 0;

    /******************************************************************/
    /* fill segment in one junk -- currently we have to do a special  */
    /* handling (leaving one character beyond the hidden and replacing*/
    /* it as a special task... -- otherwise replace will not work.    */
    /******************************************************************/
    EQFBReplaceSelRTF( pDoc, (PSZ_W)pDisp->pBufferOverflow );
  }

  pDisp->bRTFFill = b;
}




VOID CleanIBU( PSZ_W pRTFFont )
{  pRTFFont;
  /********************************************************************/
  /* strip out '\b, \b0, \u, \u0, \i, \i0                             */
  /********************************************************************/
//USHORT usI = 0, usJ = 0;
//BYTE b;
//while ( (b = pRTFFont[usI]) != EOS )
//{
//  if ( b == '\\' )
//  {
//    switch ( pRTFFont[usI+1] )
//    {
//      case 'b':
//      case 'u':
//      case 'i':
//        switch ( pRTFFont[usI+2] )
//        {
//          case ' ':
//          case '\\':
//            usI++;
//            break;
//          case '0':
//            usI += 2;
//            break;
//          default:
//            pRTFFont[usJ] = b;
//            usJ++; usI++;
//            break;
//        } /* endswitch */
//        break;
//      default:
//        pRTFFont[usJ] = b;
//        usJ++; usI++;
//        break;
//    } /* endswitch */
//  }
//  else
//  {
//    pRTFFont[usJ] = b;
//    usJ++; usI++;
//  } /* endif */
//} /* endwhile */
//pRTFFont[usJ] = EOS;
}



void BuildRTFShorten
(
   PSZ_W  pBuffer,
   PSZ_W  pData,
   USHORT usLen
)
{
  BOOL   fBlankFound = FALSE;
  USHORT usShortLen = 0;
  USHORT usMaxTextLen = 0;
  PSZ_W  pStart = pData;
  USHORT usI = 0;

  usI = usLen - 1;
  while (*(pStart + usI) == '\n' && (usI > 0 )  )
  {
    usI--;
  } /* endwhile */

  if (usI < 10 )
  {
    UTF16strcpy(pBuffer, pData);
  }
  else
  {
    usMaxTextLen = 10;
    if ( usLen >= 8 && usLen <=10 )
    {
       usMaxTextLen = 8;
    }
    while (!fBlankFound &&
           (usShortLen < usMaxTextLen) )
    {
      *pBuffer++ = *pData++;
      usShortLen ++;
      if ((*pData == BLANK) || (*pData == '\n'))
      {
        fBlankFound = TRUE;
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* add SHORTEN_SEGDATA only if chars other than linefeeds follow  */
    /* in original tag/NOP                                            */
    /******************************************************************/

    if (usI >= usShortLen + 1 )         //last char != lf is added later
    {
      UTF16strcat(pBuffer, SHORTEN_SEGDATA );
      pBuffer += UTF16strlenCHAR(SHORTEN_SEGDATA );
    } /* endif */


    if (usI != (usLen - 1) )
    {
      /****************************************************************/
      /* linefeeds found at end of NOP/tag                            */
      /* copy last char of NOP/tag plus all ending linefeeds          */
      /****************************************************************/
      if (usI < usShortLen )      //avoid copying last char twice
      {
        usI = usShortLen;
      } /* endif */
      UTF16strcat(pBuffer, (pStart + usI));
    }
    else
    {
      /****************************************************************/
      /* copy last char of NOP/inline tag                             */
      /****************************************************************/
      UTF16strcat(pBuffer, (pStart + usLen - 1));
    } /* endif */
  } /* endif */

   return;
}

#ifdef RTF_DEBUG
VOID RTFDEBUGSTRING( PTBDOCUMENT pDoc, PSZ_W p )
{
  if ( pDoc->docType == STARGET_DOC )
  {
    FILE *fOut;                          // test output
    fOut      = fopen ( "D:\\RTFdebugstring.OUT", "a" );
    fwprintf(fOut, L"%s\n",p);
    fclose( fOut );
  }

}
#endif


#ifdef RTF_DEBUG
VOID RTFDEBUG( PTBDOCUMENT pDoc, PSZ_W p )
{
  if ( pDoc->docType == STARGET_DOC )
  {
    static int iii = 0;
    CHAR_W b;
    CHARRANGE chRange1;
    PEQFBRTFLINE pRTFLine1 = &(pDoc->pDispFileRTF->RTFLine);
    FILE *fOut;                          // test output
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange1 );
    iii++;

    fOut      = fopen ( "D:\\RTFstreamRTF.OUT", "a" );
    b = pRTFLine1->chText[150]; pRTFLine1->chText[150] = EOS;
    fprintf(fOut, "\n\n\n%s: %d \n %u %u %s\n",p, iii, chRange1.cpMin, chRange1.cpMax, pRTFLine1->chText);
    pRTFLine1->chText[150] = b;

    fclose( fOut );
  }
  else
  if ( pDoc->docType == SERVPROP_DOC )
  {
    static int iii = 0;
    CHARRANGE chRange1;
    EQFBRTFLINE RTFLine1;
    FILE *fOut;                          // test output
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange1 );
    memset( &RTFLine1, 0, sizeof( EQFBRTFLINE ));
    EQFBGetMatchingRangeRTF( pDoc, 0xffffffff, &RTFLine1 );
    iii++;

    fOut      = fopen ( "D:\\RTFstreamp.OUT", "a" );
//    b = RTFLine1.chText[150]; RTFLine1.chText[150] = EOS;
    fprintf(fOut, "\n\n\n%s: %d \n %u %u %s\n",p, iii, chRange1.cpMin, chRange1.cpMax, RTFLine1.chText);
//    RTFLine1.chText[150] = b;

    fclose( fOut );
  }
  else
  if ( pDoc->docType == SERVDICT_DOC )
  {
    static int iii = 0;
//    BYTE b;
    CHARRANGE chRange1;
    EQFBRTFLINE RTFLine1;
    FILE *fOut;                          // test output
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange1 );
    memset( &RTFLine1, 0, sizeof( EQFBRTFLINE ));
    EQFBGetMatchingRangeRTF( pDoc, 0xffffffff, &RTFLine1 );
    iii++;

    fOut      = fopen ( "D:\\RTFstreamd.OUT", "a" );
//    b = RTFLine1.chText[150]; RTFLine1.chText[150] = EOS;
    fprintf(fOut, "\n\n\n%s: %d \n %u %u %s\n",p, iii, chRange1.cpMin, chRange1.cpMax, RTFLine1.chText);
//    RTFLine1.chText[150] = b;

    fclose( fOut );
  } /* endif */

}

VOID RTFDEBUGEX( PTBDOCUMENT pDoc, PSZ_W p, PSZ_W p1 )
{
  if ( pDoc->docType == STARGET_DOC )
  {
    static int iii = 0;
    CHAR_W b;
    CHARRANGE chRange1;
    EQFBRTFLINE RTFLine1;
    FILE *fOut;                          // test output
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange1 );
    memset( &RTFLine1, 0, sizeof( EQFBRTFLINE ));
    EQFBGetMatchingRangeRTF( pDoc, 0xffffffff, &RTFLine1 );
    iii++;

    fOut      = fopen ( "D:\\RTFstream.OUT", "a" );
    b = RTFLine1.chText[150]; RTFLine1.chText[150] = EOS;
    fprintf(fOut, "\n\n\n%s - %s: %d \n %u %u %s\n",p1, p, iii, chRange1.cpMin, chRange1.cpMax, RTFLine1.chText);
    RTFLine1.chText[150] = b;

    fclose( fOut );
  }
  else
  if ( pDoc->docType == SERVPROP_DOC )
  {
    static int iii = 0;
    CHARRANGE chRange1;
    EQFBRTFLINE RTFLine1;
    FILE *fOut;                          // test output
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange1 );
    memset( &RTFLine1, 0, sizeof( EQFBRTFLINE ));
    EQFBGetMatchingRangeRTF( pDoc, 0xffffffff, &RTFLine1 );
    iii++;

    fOut      = fopen ( "D:\\RTFstreamp.OUT", "a" );
    fprintf(fOut, "\n\n\n%s - %s: %d \n %u %u %s\n",p1, p, iii, chRange1.cpMin, chRange1.cpMax, RTFLine1.chText);

    fclose( fOut );
  }
  else
  if ( pDoc->docType == SERVDICT_DOC )
  {
    static int iii = 0;
    CHARRANGE chRange1;
    EQFBRTFLINE RTFLine1;
    FILE *fOut;                          // test output
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange1 );
    memset( &RTFLine1, 0, sizeof( EQFBRTFLINE ));
    EQFBGetMatchingRangeRTF( pDoc, 0xffffffff, &RTFLine1 );
    iii++;

    fOut      = fopen ( "D:\\RTFstreamd.OUT", "a" );
    fprintf(fOut, "\n\n\n%s - %s: %d \n %u %u %s\n",p1, p, iii, chRange1.cpMin, chRange1.cpMax, RTFLine1.chText);

    fclose( fOut );
  } /* endif */

}
#endif


//------------------------------------------------------------------------------
// draws the format chars
//
VOID DrawFormatChars(PTBDOCUMENT pDoc)
{
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  if ( (pEQFBUserOpt->UserOptFlags.bVisibleSpace || pDoc->fAutoSpellCheck)  && !(pDoc->pDispFileRTF->bRTFFill & RTF_FILL))
  {
    CHAR_W  c, c2;
    POINT ptCharL, ptCharR;
    HDC   hdc = GetDC( pDoc->hwndRichEdit );
    RECT  rectClient;
    HRGN  rgnRect;
    SIZE  sizChar, sizCharSpell;
    HFONT hFont;
    int iPos;
    int iAbsPos;
    LOGFONT  lf;                                 // logical font structure
    PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);
    TEXTMETRIC textMetric;

    c2 = NULC;
    GetTextExtentPoint32(hdc, "Ag", 2, &sizCharSpell);
    GetTextExtentPoint32(hdc, "A", 1, &sizChar);
    if ( (pDoc->usLangTypeTgt == MORPH_THAI_LANGTYPE) ||
         (pDoc->usLangTypeSrc == MORPH_THAI_LANGTYPE) )
    {
      /****************************************************************/
      /* add oversize  (diacritics)                                   */
      /****************************************************************/
      GetTextMetrics( hdc,  &textMetric);
      sizChar.cy = textMetric.tmHeight + textMetric.tmAscent/2;
    } /* endif */

    // limit clipping region to the ctrl
    GetClientRect(pDoc->hwndClient, &rectClient);
    rgnRect = CreateRectRgnIndirect(&rectClient);
    SelectClipRgn(hdc, rgnRect);

    HideCaret( pDoc->hwndRichEdit );

//    pRTFLine->ulFirstLineIndex = SendMessage( hwnd, EM_LINEINDEX, usFirstLine, 0 );

    if ( pEQFBUserOpt->UserOptFlags.bVisibleSpace )
    {
      /******************************************************************/
      /* Draw LF and BLANKs                                             */
      /******************************************************************/
//      EQFBUpdateDispTable( pDoc );
      /********************************************************************/
      /* prepare font                                                     */
      /********************************************************************/
      memset(&lf, 0, sizeof(lf));
      strcpy( lf.lfFaceName, "Symbol" );
      lf.lfHeight = sizChar.cy-2;
      lf.lfWidth  = sizChar.cx;
      lf.lfQuality  = DEFAULT_QUALITY;

      lf.lfWeight = FW_NORMAL;
      lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
      lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
      lf.lfOutPrecision = OUT_STROKE_PRECIS;

      hFont = CreateFontIndirect( &lf );
      hFont = (HFONT)SelectObject( hdc, hFont );

      SetTextColor(hdc, RGB(0,128,0));
      SetBkMode( hdc, TRANSPARENT );

      iPos = 0;
      while ( (c = pRTFLine->chText[ iPos++ ])!= NULC)
      {
        if ( ((c == '\r') && (c2 != '\\')) || ( c == ' ' ) )
        {
          break;
        } /* endif */
        c2 = c;
      } /* endwhile */

      if ( c == EOS  )
      {
        iPos = -1;
      }
      else
      {
        iPos--;
      } /* endif */
      iAbsPos = iPos + pRTFLine->ulFirstLineIndex;

      // for all '\n' or ' '
      while(iPos > -1)
      {
        SendMessage( pDoc->hwndRichEdit, EM_POSFROMCHAR, (LONG)&ptCharL, iAbsPos );

        if(PtInRect(&rectClient, ptCharL))
        {
          SendMessage( pDoc->hwndRichEdit, EM_POSFROMCHAR, (LONG)&ptCharR, iAbsPos+1 );
          if ( ptCharR.x != ptCharL.x )
          {
            SetTextAlign(hdc, TA_LEFT);
            if ( c == '\r' )
            {
              ExtTextOut(hdc, ptCharL.x + 2, ptCharL.y, ETO_OPAQUE, NULL, "\xBF",
                         1, NULL);
            }
            else if ( c == ' ' )
            {
              ExtTextOut(hdc, ptCharL.x + 1, ptCharL.y, 0, NULL, "\xD7", 1, NULL);
            } /* endif */
          } /* endif */
        }

        if(ptCharL.y > rectClient.bottom)
          break;

        iPos++;
        while ( (c = pRTFLine->chText[ iPos++ ]) != NULC)
        {
          if ( ((c == '\r') && (c2 != '\\')) || (c == ' ') )
          {
            break;
          } /* endif */
          c2 = c;
        } /* endwhile */

        if ( c == EOS  )
        {
          iPos = -1;
        }
        else
        {
          iPos--;
        } /* endif */
        iAbsPos = iPos + pRTFLine->ulFirstLineIndex;
      }
      DeleteObject( SelectObject( hdc, hFont ) );
    } /* endif */


    if ( pDoc->fAutoSpellCheck )
    {
      ULONG ulSegNum;
      PTBSEGMENT pSeg;
      int sLow = 0;
      int sHigh = 0;

      ReleaseDC( pDoc->hwndRichEdit, hdc );

      hdc = GetDC( pDoc->hwndRichEdit );
//      EQFBUpdateDispTable( pDoc );
      /****************************************************************/
      /* update work seg ...                                          */
      /****************************************************************/
      if ( (pDoc->ulWorkSeg <= pRTFLine->ulEndSeg) &&
           (pDoc->ulWorkSeg >= pRTFLine->ulStartSeg) )
      {
        if ( SendMessage( pDoc->hwndRichEdit, EM_GETMODIFY, 0L, 0L ) )
        {
          PSZ_W pData = GetWorkSegDataPtr( pDoc, pRTFLine, pDoc->ulWorkSeg );
          AdjustWorkSegData( pDoc, pData, pDoc->ulWorkSeg );
        } /* endif */
      } /* endif */
      ulSegNum = pRTFLine->ulStartSeg;

      sLow = 3;
      sHigh = 1;
      if (pDoc->ulOemCodePage == 874L)  // if Thai!
      {
        sLow = -2;
        sHigh = -4;
      }

      hFont = (HFONT)SelectObject(hdc, CreatePen( PS_SOLID, 1, RGB(0xff,0,0)));
      while ( ulSegNum <= pRTFLine->ulEndSeg )
      {
        pSeg = EQFBGetSegW(pDoc, ulSegNum);
        if ( pSeg && pSeg->pDataW && pSeg->pusHLType )
        {
          // look for position in start/stop table
          PSTARTSTOP pstCurrent = (PSTARTSTOP) pSeg->pusHLType;
          while ( pstCurrent->usType != 0 )
          {
             if ( pstCurrent->usType == MISSPELLED_HIGHLIGHT )
             {
               LONG   lI;
               ULONG  ulLen;
               CHAR_W b;

               ULONG ulTextOffs = pRTFLine->ulSegTextOffs[ulSegNum-pRTFLine->ulStartSeg];
               ULONG ulTextMark;

               ulLen = pRTFLine->usSegTextLen[ulSegNum-pRTFLine->ulStartSeg];
               b = pRTFLine->chText[ulTextOffs + ulLen];
               pRTFLine->chText[ulTextOffs + ulLen] = EOS;

               ulTextMark = PosEQF2RTF( pDoc, pSeg->pDataW,
                                        &pRTFLine->chText[ulTextOffs],
                                        pstCurrent->usStart );
               ulTextMark += ulTextOffs;

               SendMessage( pDoc->hwndRichEdit, EM_POSFROMCHAR, (LONG)&ptCharL, ulTextMark );
               ulTextMark = PosEQF2RTF( pDoc, pSeg->pDataW,
                                        &pRTFLine->chText[ulTextOffs],
                                        (USHORT)(pstCurrent->usStop+1) );
               ulTextMark += ulTextOffs;
               pRTFLine->chText[ulTextOffs + ulLen] = b;

               SendMessage( pDoc->hwndRichEdit, EM_POSFROMCHAR, (LONG)&ptCharR, ulTextMark );

               MoveToEx( hdc, ptCharL.x, ptCharL.y + sizCharSpell.cy - sLow, NULL );


               for ( lI= ptCharL.x; lI < ptCharR.x; lI += 4 )
               {
                 LineTo( hdc, lI+2, ptCharL.y + sizCharSpell.cy - sHigh );
                 if ( lI+2 < ptCharR.x )
                 {
                   LineTo( hdc, lI+4, ptCharL.y + sizCharSpell.cy - sLow );
                 } /* endif */
               } /* endfor */
             }
             else
             {
               /*******************************************************/
               /* nothing to do                                       */
               /*******************************************************/
             } /* endif */

             pstCurrent++;
          } /* endwhile */
        } /* endif */
        ulSegNum++;
      } /* endwhile */
      DeleteObject( SelectObject( hdc, hFont ) );
    } /* endif */
    // restore old font and delete the created font
    ReleaseDC( pDoc->hwndRichEdit, hdc );
    ShowCaret( pDoc->hwndRichEdit );

  } /* endif */

  return;

}

MRESULT HandleDummyMessage( HWND hwnd, PTBDOCUMENT pDoc, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
  MRESULT mResult = FALSE;
  ULONG ulCaret;
  POINTL point;

  switch( msg )
  {
    case WM_BUTTON1DOWN:
      point.x = LOWORD( mp2 );  point.y = HIWORD( mp2 );
      ulCaret = SendMessage( pDoc->hwndRichEdit, EM_CHARFROMPOS, 0L, (LONG)&point );
      EQFBGetSegFromCaretRTF( pDoc, &pDoc->TBCursor, ulCaret );
      break;

    case WM_KEYDOWN:
      if ( mp1 == VK_F1 )
      {
        /**************************************************************/
        /* Trigger help by posting HM_HELPSUBITEM_NOT_FOUND to TWB    */
        /**************************************************************/
        USHORT usID;
        switch ( pDoc->docType )
        {
           case SSOURCE_DOC:
             usID = ID_TWBS_ORIG_WINDOW;
             break;
           case STARGET_DOC:
             usID = ID_TWBS_SOURCE_WINDOW;
             break;
           case SERVDICT_DOC:
             usID = ID_TWBS_DICT_WINDOW;
             break;
           case SERVPROP_DOC:
           case SERVSOURCE_DOC:
             usID = ID_TWBS_PROP_WINDOW;
             break;
           case TRNOTE_DOC:
             usID = ID_TWBS_TRNOTE_WINDOW;
             break;
           default:
           case OTHER_DOC:
             usID = ID_TWBS_OTHER_WINDOW;
             break;
        } /* endswitch */
        PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ),
                     HM_HELPSUBITEM_NOT_FOUND,
                     0,
                     MP2FROM2SHORT( usID, usID ));
        break;
      } /* endif */
    case WM_SYSCHAR:
    case WM_SYSKEYDOWN:
    case WM_CHAR:           // determine character and pass it to editor
      mResult = RichEditHandleWM_CHAR( hwnd, pDoc, msg, mp1, mp2 );
      break;

   case WM_INPUTLANGCHANGEREQUEST:
      mResult = 0;
      break;

   case WM_INPUTLANGCHANGE:
      mResult = 0;
      break;



    default:
      break;
  } /* end switch */
  return mResult;
}


MRESULT EQFBFuncRTFFunc( PTBDOCUMENT pDoc, USHORT usFunction,
                         HWND hwnd, WPARAM mp1, LPARAM mp2 )
{
  MRESULT mResult = FALSE;
  USHORT  usOffset = 0;
  hwnd; mp2;
  pDoc->pDispFileRTF->usFunc = usFunction;
  switch ( usFunction )
  {
    case BOTTOMDOC_FUNC:
    case TOPDOC_FUNC:
    case RIGHT_FUNC:
    case LEFT_FUNC:
    case DOWN_FUNC:
    case UP_FUNC:
    case PAGEDOWN_FUNC:
    case PAGEUP_FUNC:
    case PREVWORD_FUNC:
    case NEXTWORD_FUNC:
    case BACKTAB_FUNC:
    case TAB_FUNC:
    case ENDLINE_FUNC:
    case STARTLINE_FUNC:
      pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
      STATUSBARRTF( pDoc );
      break;
    case DELETECHAR_FUNC:
      EQFBFuncDeleteRTF( pDoc );
      mResult = TRUE;
    //{
    //  TBROWOFFSET  tbCursorMin, tbCursorMax;
    //  CHARRANGE chRange;
    //  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
    //  if (chRange.cpMin != chRange.cpMax)
    //  {
    //    PEQFBBLOCK  pstBlock;                // pointer to block struct
    //    EQFBBLOCK   eqfBlock;
    //    pstBlock = pDoc->pBlockMark;
    //    pstBlock->pDoc     = pDoc;

    //    EQFBGetSegFromCaretRTF( pDoc, &tbCursorMin, chRange.cpMin );
    //    pstBlock->usStart = tbCursorMin.usSegOffset;
    //    pstBlock->ulSegNum = tbCursorMin.ulSegNum;

    //    EQFBGetSegFromCaretRTF( pDoc, &tbCursorMax, chRange.cpMax );
    //    pstBlock->usEnd = tbCursorMax.usSegOffset-1;
    //    pstBlock->ulEndSegNum = tbCursorMax.ulSegNum;
    //    memcpy( &pDoc->TBCursor, &tbCursorMin, sizeof( TBROWOFFSET) );
    //    memcpy( &eqfBlock, pstBlock, sizeof( EQFBBLOCK ) );
    //    EQFBSetCaretPosRTF( pDoc, chRange.cpMin);
    //    memcpy( pstBlock, &eqfBlock, sizeof(EQFBBLOCK));
    //  }
    //  else
    //  {
	//	  pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
    //      EQFBUpdateTBCursor( pDoc );
    //  }
    //  mResult = EQFBCallOrgTPFunction( pDoc, usFunction );
    //  }
      break;
//  case ENDLINE_FUNC:
//    pDoc->fTBCursorReCalc = TRUE;
//    EQFBUpdateTBCursor( pDoc );
//    EQFBFuncEndLine(pDoc);
//    STATUSBARRTF( pDoc );
//    mResult = TRUE;
//    break;
//  case STARTLINE_FUNC:
//    pDoc->fTBCursorReCalc = TRUE;
//    EQFBUpdateTBCursor( pDoc );
//    EQFBFuncStartLine(pDoc);
//    EQFBGotoSegRTF( pDoc,
//                    pDoc->TBCursor.ulSegNum,     // pos. at this seg
//                    pDoc->TBCursor.usSegOffset );
//    STATUSBARRTF( pDoc );
//    mResult = TRUE;
//    break;
    case MARKSEG_FUNC:
      pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
      EQFBUpdateTBCursor( pDoc );
      EQFBMark( pDoc );
      break;
    case GOTOSEG_FUNC:
      EQFBGotoActSegment( pDoc );
      if (pDoc->tbActSeg.ulSegNum )
      {
        EQFBNextUnprotected( pDoc, &pDoc->tbActSeg, &usOffset );
        EQFBGotoSeg( pDoc, pDoc->tbActSeg.ulSegNum, usOffset );
      } /* endif */
      break;
    case CLEARSEGMARK_FUNC:
      pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
      EQFBUpdateTBCursor( pDoc );
      EQFBClearMark( pDoc );
      break;
    case INSTOGGLE_FUNC:
      pDoc->EQFBFlags.inserting = !pDoc->EQFBFlags.inserting;
//      pDoc->pDispFileRTF->fRichEditOverwrite  = !pDoc->pDispFileRTF->fRichEditOverwrite;
      STATUSBARRTF( pDoc );
      break;
    case TSEG_FUNC:
      {
        BOOL   fLock;
        LOCKRICHEDIT( pDoc, fLock );
        EQFBTransRTF( pDoc, POS_TOBEORDONE );         // pos at translated ones, too
        UNLOCKRICHEDIT_TRUE( pDoc, fLock );
        mResult = TRUE;
      }
      break;
    case TSEGNEXT_FUNC:
      {
        BOOL   fLock;
        LOCKRICHEDIT( pDoc, fLock );
        EQFBTransRTF( pDoc, POS_TOBE );               // position at untranslated ones
        UNLOCKRICHEDIT_TRUE( pDoc, fLock );
        mResult = TRUE;
      }
      break;
    case TSEGNEXT_EXACT_FUNC:
      {
        BOOL   fLock;
        LOCKRICHEDIT( pDoc, fLock );
        EQFBTransRTF( pDoc, POS_TOBE_EXACT );         // position at untranslated ones with EXACT matches
        UNLOCKRICHEDIT_TRUE( pDoc, fLock );
        mResult = TRUE;
      }
      break;
    case TSEGNEXT_FUZZY_FUNC:
      {
        BOOL   fLock;
        LOCKRICHEDIT( pDoc, fLock );
        EQFBTransRTF( pDoc, POS_TOBE_FUZZY );         // position at untranslated ones with FUZZY matches
        UNLOCKRICHEDIT_TRUE( pDoc, fLock );
        mResult = TRUE;
      }
      break;
    case TSEGNEXT_NONE_FUNC:
      {
        BOOL   fLock;
        LOCKRICHEDIT( pDoc, fLock );
        EQFBTransRTF( pDoc, POS_TOBE_NONE );          // position at untranslated ones with NO matches
        UNLOCKRICHEDIT_TRUE( pDoc, fLock );
        mResult = TRUE;
      }
      break;
    case TSEGNEXT_MT_FUNC:
      {
        BOOL   fLock;
        LOCKRICHEDIT( pDoc, fLock );
        EQFBTransRTF( pDoc, POS_TOBE_MT );            // position at untranslated ones with MT matches
        UNLOCKRICHEDIT_TRUE( pDoc, fLock );
        mResult = TRUE;
      }
      break;
    case TSEGNEXT_GLOBAL_FUNC:
      {
        BOOL   fLock;
        LOCKRICHEDIT( pDoc, fLock );
        EQFBTransRTF( pDoc, POS_TOBE_GLOBAL );        // position at untranslated ones with GLOBAL MEMORY matches
        UNLOCKRICHEDIT_TRUE( pDoc, fLock );
        mResult = TRUE;
      }
      break;
    case ENDSEG_FUNC:
      EQFBFuncEndSeg ( pDoc );             // move to the end of segment
      mResult = TRUE;
      break;
    case STARTSEG_FUNC:
      pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
      EQFBFuncStartSeg ( pDoc );           // move to the start of segment
      EQFBGotoSegRTF( pDoc,
                      pDoc->TBCursor.ulSegNum,     // pos. at this segment
                      0 );
      mResult = TRUE;
      break;
    case BACKSPACE_FUNC:     // let RTFControl handle it
       EQFBFuncBackspaceRTF( pDoc );
       mResult = TRUE;
       break;
    case CHARACTER_FUNC:
      // set the flags necessary for processing
      if  (pDoc->docType == STARGET_DOC)
      {
        pDoc->EQFBFlags.workchng = TRUE;
        pDoc->pTBSeg->SegFlags.Typed = TRUE;
        pDoc->ActSegLog.usNumTyped ++;
		MTLogStartEditing( pDoc );
        pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
        STATUSBARRTF( pDoc );
        if ( pDoc->fAutoSpellCheck && (mp1 == BLANK) )
        {
          PostMessage( pDoc->hwndRichEdit, WM_EQF_UPDATERTFCTRL,
                       RTFCTRL_AUTOSPELL, 0L );
        } /* endif */

        if ( usFunction == CHARACTER_FUNC )
        {
          USHORT usChar = pDoc->usChar;
          if ( usChar < 255 )
          {
            CHAR_W c_W[2];
            UCHAR c[2];
            LONG  lBytesLeft = 0L;
            LONG  lRc = 0;

            c[0] = (UCHAR)usChar;
            c[1] = EOS;
            UtlDirectAnsi2UnicodeBuf((PSZ)c, c_W, 1, (ULONG)GetCodePage( ANSI_CP ),
                                    TRUE, &lRc, &lBytesLeft );
            usChar = pDoc->usChar = c_W[0];
          }
          if ( SpecialCare( pDoc ) )
          {
            pDoc->usChar = usChar;
            if ( IS_RTL(pDoc) )
            {
              // avoid switching between different keyboards if in input mode..
//              HKL hkl = GetKeyboardLayout( 0 );
              mResult = EQFBCallOrgTPFunction( pDoc, usFunction );
//              ActivateKeyboardLayout( hkl, 0 );
            }
            else
            {
              mResult = EQFBCallOrgTPFunction( pDoc, usFunction );
            }
          } /* endif */
        } /* endif */
      } /* endif */
      break;
    case GETDICTMATCH_FUNC:  // copy dictionary match
      pDoc->usChar = (USHORT)mp1;
      EQFBGetDictMatch(pDoc);
	  pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
	  pDoc->EQFBFlags.workchng = TRUE;
      mResult = TRUE;        // we handled message
      break;
    case GETPROPMATCH_FUNC:
      {
        BOOL   fLock;
        USHORT usOffset = 0;
        pDoc->usChar = (USHORT)mp1;
        LOCKRICHEDIT( pDoc, fLock );
        EQFBGetPropMatch(pDoc);    // EQFBWorkSegment is filled correctly

        EQFBSetWorkSegRTF( pDoc, pDoc->tbActSeg.ulSegNum, pDoc->pEQFBWorkSegmentW );
        EQFBNextUnprotected( pDoc, &pDoc->tbActSeg, &usOffset );
        EQFBGotoSeg( pDoc, pDoc->tbActSeg.ulSegNum, usOffset );
        UNLOCKRICHEDIT_TRUE ( pDoc, fLock);

        mResult = TRUE;        // we handled message
      }
      break;
    case VISIBLESPACE_FUNC:
      EQFBFuncVisibleSpace(pDoc );      // toggle visible whitespace on/off
      InvalidateRect( pDoc->hwndRichEdit, NULL, FALSE );
      mResult = TRUE;
      break;

    case SPELLAUTO_FUNC:
      EQFBFuncSpellAuto(pDoc );      // toggle autospell checking on/off
      EQFBWriteProfile(pDoc);
      InvalidateRect( pDoc->hwndRichEdit, NULL, FALSE );
      mResult = TRUE;
      break;

    case UNDO_FUNC:
      EQFBUndoRTF( pDoc );
      mResult = TRUE;
      break;

    case REDO_FUNC:
      EQFBRedoRTF( pDoc );
      mResult = TRUE;
      break;

    /************************************************************/
    /* request handled by RTF Control                           */
    /************************************************************/
    case MARKLEFT_FUNC:
    case MARKRIGHT_FUNC:
    case MARKUP_FUNC:
    case MARKDOWN_FUNC:
    case MARKBLOCK_FUNC:
    case MARKCOPY_FUNC  :
    case MARKMOVE_FUNC  :
    case MARKNEXT_FUNC:
    case MARKPREV_FUNC:
      pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
      STATUSBARRTF( pDoc );
      mResult = FALSE;
      break;

    case MARKDELETE_FUNC:
      EQFBFuncMarkDelete( pDoc );
      mResult = TRUE;
      break;

    case MARKCLEAR_FUNC:
      EQFBFuncMarkClear( pDoc );
      SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );
      SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
      mResult = TRUE;
      break;
    case MARKSTART_FUNC:
    case MARKEND_FUNC:
    case MARKSEGSTART_FUNC:
    case MARKSEGEND_FUNC:
      /****************************************************************/
      /* do standard TP processing and add selection                  */
      /****************************************************************/
      {
	    PFUNCTIONTABLE pFuncTab = get_FuncTab();
        PEQFBBLOCK pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;
        void    (*function)( PTBDOCUMENT );
        pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
        if ( pDoc->EQFBFlags.workchng && pDoc->ulWorkSeg )
        {
          EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );
        } /* endif */

        /****************************************************************/
        /* ensure that correct segment is loaded                        */
        /****************************************************************/
        if ( pDoc->EQFBFlags.PostEdit )
        {
          EQFBWorkSegCheck( pDoc );
        } /* endif */

        if ( pDoc->pDispFileRTF->fTBCursorReCalc )
        {
          EQFBUpdateTBCursor( pDoc );
        }

        function = (pFuncTab + usFunction)->function;
        (*function)( pDoc );                                 // execute the function
        mResult = TRUE;
        if ( pstBlock && (pstBlock->pDoc == pDoc) )
        {
          PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);
          if ((pRTFLine->ulStartSeg <= pstBlock->ulSegNum) &&  (pstBlock->ulEndSegNum <= pRTFLine->ulEndSeg)  )
          {
            CHARRANGE chRange;
            chRange.cpMin = pRTFLine->ulSegTextOffs[pstBlock->ulSegNum - pRTFLine->ulStartSeg] + pstBlock->usStart;
            chRange.cpMax = pRTFLine->ulSegTextOffs[pstBlock->ulEndSegNum - pRTFLine->ulStartSeg] + pstBlock->usEnd;
            SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
          }
          else
          if ((pRTFLine->ulStartSeg <= pstBlock->ulSegNum) &&  (pstBlock->ulSegNum <= pRTFLine->ulEndSeg)  )
          {
            CHARRANGE chRange;
            chRange.cpMin = pRTFLine->ulSegTextOffs[pstBlock->ulSegNum - pRTFLine->ulStartSeg] + pstBlock->usStart;
            chRange.cpMax = pRTFLine->ulSegTextOffs[pstBlock->ulSegNum - pRTFLine->ulStartSeg] + pstBlock->usEnd;
          }
          else
          {
            int i = 1;
            i;
          } /* endif */
        } /* endif */
      }
      break;
    case NEXTLINE_FUNC:
      SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_DOWN, 0L );
      SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_HOME, 0L );
      mResult = TRUE;
      break;
    /************************************************************/
    /* our standard TP functions will handle the request        */
    /************************************************************/
    default:
      mResult = EQFBCallOrgTPFunction( pDoc, usFunction );
      break;
  } /* endswitch */
  return mResult;
}

BOOL GetCharFormat( PTBDOCUMENT pDoc, ULONG ulMode )
{
  CHARFORMAT2 cfDefault;
  memset( &cfDefault, 0, sizeof(cfDefault) );
  cfDefault.cbSize = sizeof(cfDefault);

  SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, TRUE, (LONG) &cfDefault );
  return ( cfDefault.dwEffects & ulMode );
}


MRESULT EQFBCallOrgTPFunction( PTBDOCUMENT pDoc, USHORT usFunction )
{
  PFUNCTIONTABLE pFuncTab = get_FuncTab();
  void    (*function)( PTBDOCUMENT );
  pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
  if ( pDoc->EQFBFlags.workchng && pDoc->ulWorkSeg )
  {
    EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );
  } /* endif */

  /****************************************************************/
  /* ensure that correct segment is loaded                        */
  /****************************************************************/
  if ( pDoc->EQFBFlags.PostEdit )
  {
    EQFBWorkSegCheck( pDoc );
  } /* endif */


  function = (pFuncTab + usFunction)->function;
  (*function)( pDoc );                                 // execute the function
  if (!((usFunction == QUIT_FUNC)||(usFunction == FILE_FUNC) )
       && pDoc->pSegTables )  // be sure noone else closed doc..
  {
    if ( pDoc->EQFBFlags.workchng )
    {
      // check if we have to add a none tag....OT
      if (!pDoc->pEQFBWorkSegmentW[0])
      {
        UTF16strcpy(pDoc->pEQFBWorkSegmentW,EMPTY_TAG);
      }
      else
      {
         EQFBCheckNoneTag( pDoc, pDoc->pEQFBWorkSegmentW );
      }
      EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pDoc->pEQFBWorkSegmentW );
    } /* endif */
    STATUSBARRTF( pDoc );
  } /* endif */
  return TRUE;
}

VOID EQFBUpdateTBCursor( PTBDOCUMENT pDoc )
{
  EQFBUpdateDispTable( pDoc );

  if ( pDoc->pDispFileRTF->fTBCursorReCalc )
  {
    CHARRANGE chRange;
    ULONG     ulCharPos;
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
    EQFBGetSegFromCaretRTF( pDoc, &pDoc->TBCursor, chRange.cpMax );
    pDoc->pDispFileRTF->fTBCursorReCalc = FALSE;
    /******************************************************************/
    /* get current line and set positions                             */
    /******************************************************************/
    ulCharPos = SendMessage( pDoc->hwndRichEdit, EM_LINEINDEX, (WPARAM)-1, 0L );
    pDoc->lCursorRow = SendMessage( pDoc->hwndRichEdit, EM_EXLINEFROMCHAR, 0L, ulCharPos );
    pDoc->lCursorCol =  (chRange.cpMax - ulCharPos);
  } /* endif */
}

VOID EQFBGetSegFromCaretRTF( PTBDOCUMENT pDoc, PTBROWOFFSET pTBCursor, ULONG ulCaret )
{

  if (ulCaret)
  {
    USHORT i = 0;
    PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);
    while ( pRTFLine->ulSegTextOffs[i] < ulCaret+1 )
    {
      i++;
      if (pRTFLine->ulSegTextOffs[i] == 0)
        break;
    }

    // Anchor, if our segoffset is not available as expected...
    if ( i == 0 )
    {
      // this code gets active whenever the start of the segment is not contained in the 
      // visible area; i.e there is text at the begin of the data which is not assigned 
      // to a segment 
      // related PR: P023290
        i++;
    }


    pTBCursor->ulSegNum = pRTFLine->ulStartSeg + i - 1;

    pTBCursor->usSegOffset = EQFBCalcSegOffsetFromCaret(pDoc,
                                            (USHORT) (ulCaret - pRTFLine->ulSegTextOffs[i-1]),
                                            (&pRTFLine->chText[0]) + pRTFLine->ulSegTextOffs[i-1]- pRTFLine->ulFirstLineIndex,
                                            pRTFLine);

    if ( pTBCursor->ulSegNum == 0)
    {
      pTBCursor->ulSegNum = 1;
    } /* endif */
  }
  else
  {
    pTBCursor->ulSegNum = 1;
    pTBCursor->usSegOffset = 0;
  } /* endif */

}

USHORT EQFBCalcSegOffsetFromCaret
(
  PTBDOCUMENT pDoc,
  USHORT      usCaret,
  PSZ_W       pData,
  PEQFBRTFLINE pRTFLine )
{
  ULONG        ulSegOffset = 0;
  PTOKENENTRY  pTok;
  PCHAR_W      pRest = NULL;
  BOOL         fQFIPending = FALSE;
  ULONG        ulCurRTFPos = 0;
  USHORT       usStart = 0;
  USHORT       usInlineTagLen = 0;
  BOOL         fFound = FALSE;

  pRTFLine;

  // allow to deal with buffer overflows, i.e. with pRests...
  do
  {
	  usStart = 0;
	  TATagTokenizeW(pData, (PLOADEDTABLE)pDoc->pDispFileRTF->pQFRTFTagTable, TRUE, &pRest, &usStart,
					 (PTOKENENTRY) pDoc->pTokBuf,
					 TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

	  pTok = (PTOKENENTRY) pDoc->pTokBuf;
	  // while not end of list and caret not in current token

	  while ((pTok->sTokenid != ENDOFLIST) && !fFound)
	  {
		switch ( pTok->sTokenid)
		{
		  case WHITESPACE:
			fFound = fFound;    // just for testing
			break;
		  case TEXT_TOKEN:
			if (!fQFIPending)
			{
			  if (usCaret <= ulCurRTFPos +  pTok->usLength)
			  { //token found!
				ulSegOffset += usCaret - ulCurRTFPos;
				fFound = TRUE;
			  }
			  else
			  {
				ulSegOffset += (pTok->usLength);
			  }
			}
			else
			{
			  if (usCaret <= ulCurRTFPos + pTok->usLength)
			  {
				fFound = TRUE;
			  }
			  else
			  {
				 ulSegOffset += usInlineTagLen;
			  }
			}
			break;
		  default:
			switch (pTok->usOrgId)
			{
			  case RTF_QFI_TAG:
				fQFIPending = TRUE;
				break;
			  case RTF_EQFI_TAG:
				fQFIPending = FALSE;
				break;
			  case RTF_QFJ_TAG:
				break;

			  case RTF_T_TAG:
				usInlineTagLen = (USHORT) (pTok->usLength - 5);
				break;

			  case RTF_NONE_TAG:
				if (!fQFIPending)
				{
				  if (usCaret <= ulCurRTFPos +  pTok->usLength)
				  {     //token found!
					ulSegOffset += usCaret - ulCurRTFPos;
					fFound = TRUE;
				  }
				  else
				  {
					ulSegOffset += (pTok->usLength);
				  }
				}
				else
				{
				  if (usCaret <= ulCurRTFPos + pTok->usLength)
				  {
					fFound = TRUE;
				  }
				  else
				  {
					ulSegOffset += usInlineTagLen;
				  }
				}
				break;


			  case RTF_QFF_TAG:
			  case RTF_N_TAG:
			  case RTF_S_TAG:
			  case RTF_L_TAG:
			  default:
				break;
		   } /* endswitch */
		   break;
		} /* endswitch */

		ulCurRTFPos +=  (pTok->usLength);

		pTok++;
	  } /* endwhile */

	  // if something left... go ahead and do it with the rest...
	  if (pRest)
	  {
		  pData = pRest;
		  pRest = NULL;
	  }
	  else
	  {
		  pData = NULL;
	  }
  } while (pData && !fFound);

  return ((USHORT)ulSegOffset);
}

/**********************************************************************/
/* Request update from display offsets table                          */
/**********************************************************************/
VOID EQFBUpdateDispTable( PTBDOCUMENT pDoc )
{
  PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);
  USHORT usFirstLine = (USHORT) SendMessage( pDoc->hwndRichEdit, EM_GETFIRSTVISIBLELINE, 0, 0 );
  ULONG  ulFirstLineIndex = SendMessage( pDoc->hwndRichEdit, EM_LINEINDEX, usFirstLine, 0 );

  if ( SendMessage( pDoc->hwndRichEdit, EM_GETMODIFY, 0L, 0L ) ||
       ( ulFirstLineIndex != pRTFLine->ulFirstLineIndex ) )
  {
    memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
    pRTFLine->ulFirstLineIndex = ulFirstLineIndex;
    GetTextRangeRTF( pDoc, pRTFLine->chText, sizeof( pRTFLine->chText ),
                     pRTFLine->ulFirstLineIndex,  RTFLINE_BUFFERSIZE);
    PrepareSegOffsTable( pDoc, &pRTFLine->chText[0], pRTFLine );
  } /* endif */
}

VOID EQFBGetMatchingRangeRTF( PTBDOCUMENT pDoc, ULONG ulSegNum, PEQFBRTFLINE pRTFLine )
{
  USHORT usFirstLine = (USHORT) SendMessage( pDoc->hwndRichEdit, EM_GETFIRSTVISIBLELINE, 0, 0 );
  ULONG  ulFirstLineIndex = SendMessage( pDoc->hwndRichEdit, EM_LINEINDEX, usFirstLine, 0 );

  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
  pRTFLine->ulFirstLineIndex = ulFirstLineIndex;

  GetTextRangeRTF( pDoc, pRTFLine->chText, sizeof( pRTFLine->chText ),
                       pRTFLine->ulFirstLineIndex, (ulSegNum == (ULONG)0xffffffff) ? 1000 : RTFLINE_BUFFERSIZE);

  /********************************************************************/
  /* Scan line to find our id                                         */
  /********************************************************************/
  PrepareSegOffsTable( pDoc, &pRTFLine->chText[0], pRTFLine );

  if ( ulSegNum == (ULONG)0xffffffff )
  {
    /******************************************************************/
    /* we do not need to position anywhere                            */
    /******************************************************************/
  }
  else
  {
    ULONG ulSeg;
    if ( pRTFLine->ulStartSeg > ulSegNum  )
    {
      while ( ulSegNum < pRTFLine->ulStartSeg )
      {
        if ( pRTFLine->ulFirstLineIndex > RTFLINE_BUFFERSIZE - 1 )
        {
          pRTFLine->ulFirstLineIndex -= (RTFLINE_BUFFERSIZE-1);
        }
        else
        {
          pRTFLine->ulFirstLineIndex = 0;
        } /* endif */

        ulSeg = pRTFLine->ulStartSeg;
        GetTextRangeRTF( pDoc, pRTFLine->chText, sizeof( pRTFLine->chText ),
                         pRTFLine->ulFirstLineIndex, RTFLINE_BUFFERSIZE);
        PrepareSegOffsTable( pDoc, &pRTFLine->chText[0], pRTFLine );
        if ( ulSeg == pRTFLine->ulStartSeg )
        {
          break;
        } /* endif */
      } /* endwhile */
    }
    else
    if ( pRTFLine->ulEndSeg < ulSegNum )
    {
      while ( ulSegNum > pRTFLine->ulEndSeg )
      {
        pRTFLine->ulFirstLineIndex += (RTFLINE_BUFFERSIZE-1);

        ulSeg = pRTFLine->ulEndSeg;
        GetTextRangeRTF( pDoc, pRTFLine->chText, sizeof( pRTFLine->chText ),
                         pRTFLine->ulFirstLineIndex, RTFLINE_BUFFERSIZE);
        PrepareSegOffsTable( pDoc, &pRTFLine->chText[0], pRTFLine );
        if ( ulSeg == pRTFLine->ulEndSeg )
        {
          break;
        } /* endif */
      } /* endwhile */
    } /* endif */
  } /* endif */


}

/**********************************************************************/
/* Retrieve the text from the RTFEditCtrl and place it into the       */
/* provided buffer                                                    */
/**********************************************************************/
//VOID  GetTextRangeRTF( PTBDOCUMENT pDoc, ULONG ulStart, ULONG ulSize)
//{
//  CHAR cp[10];
//  CHARRANGE chRange, chRangeNew, chRangeSet;
//  HWND hwndRE = pDoc->hwndRichEdit;
//  PDISPFILERTF pDispFile = pDoc->pDispFileRTF;
//  EDITSTREAM es = {(LONG)pDispFile, 0, EditStreamOutRange };
//  BYTE  b = pDispFile->bRTFFill;
//  BOOL fLock;
//
//  pDispFile->bRTFFill = RTF_FILL;
//
//  /********************************************************************/
//  /* get current selection, do new selection, retrieve it, restore old*/
//  /********************************************************************/
//  SendMessage( hwndRE, EM_EXGETSEL, 0, (LONG) &chRange );
//  chRangeNew.cpMin = ulStart;
//  chRangeNew.cpMax = ulStart + ulSize;
//
//  LOCKRICHEDIT( pDoc, fLock );
//
//  SendMessage( hwndRE, EM_EXSETSEL, 0, (LONG)&chRangeNew );
//  SendMessage( hwndRE, EM_EXGETSEL, 0, (LONG)&chRangeSet );
//  if ((chRangeSet.cpMax != chRangeNew.cpMax) || (chRangeSet.cpMin != chRangeNew.cpMin) )
//  {
//    chRangeNew.cpMax = chRangeSet.cpMax;
//    SendMessage( hwndRE, EM_EXSETSEL, 0, (LONG)&chRangeNew );
//    SendMessage( hwndRE, EM_EXGETSEL, 0, (LONG)&chRangeSet );
//  }
//  /********************************************************************/
//  /* @@@ Change into SF_UNICODE if we are supporting unicode directly */
//  /********************************************************************/
//  GetLocaleInfo(LOWORD( GetKeyboardLayout(0) ), LOCALE_IDEFAULTANSICODEPAGE,
//                        cp, sizeof(cp));
//
//  pDispFile->lPos = 0;
//  pDispFile->lPoscb = 0;
//
//  SendMessage( hwndRE, EM_STREAMOUT,
//               MAKELONG( SF_TEXT | SF_USECODEPAGE | SFF_SELECTION, _ttol (&cp[0]) ),
//               (LONG)&es );
//
//  SendMessage( hwndRE, EM_EXSETSEL, 0, (LONG)&chRange );
//  UNLOCKRICHEDIT( pDoc, fLock );
//
//  pDispFile->bRTFFill = b;
//}

VOID  GetTextRangeRTF( PTBDOCUMENT pDoc, PSZ_W pData, ULONG cb,
                       ULONG ulStart, ULONG ulSize)
{
  CHARRANGE chRange, chRangeNew, chRangeSet;
  HWND hwndRE = pDoc->hwndRichEdit;
  PDISPFILERTF pDispFile = pDoc->pDispFileRTF;
  BYTE  b = pDispFile->bRTFFill;
  BOOL fLock;
  POINT lPoint;
  GETTEXTEX GetTextEx;

  pDispFile->bRTFFill = RTF_FILL;

  /********************************************************************/
  /* get current selection, do new selection, retrieve it, restore old*/
  /********************************************************************/
  SendMessage( hwndRE, EM_GETSCROLLPOS, 0, (LONG) &lPoint );
  SendMessage( hwndRE, EM_EXGETSEL, 0, (LONG) &chRange );

  LOCKRICHEDIT( pDoc, fLock );

  chRangeNew.cpMin = ulStart;
  chRangeNew.cpMax = ulStart + ulSize;
  SendMessage( hwndRE, EM_EXSETSEL, 0, (LONG)&chRangeNew );
  SendMessage( hwndRE, EM_EXGETSEL, 0, (LONG)&chRangeSet );


  if ( chRangeSet.cpMin != chRangeNew.cpMin )
  {
    chRangeNew.cpMax = chRangeSet.cpMax;
    while ( chRangeNew.cpMin > 0 )
    {
      chRangeNew.cpMin--;
      SendMessage( hwndRE, EM_EXSETSEL, 0, (LONG)&chRangeNew );
      SendMessage( hwndRE, EM_EXGETSEL, 0, (LONG)&chRangeSet );
      if ( chRangeSet.cpMin == chRangeNew.cpMin )
        break;
    }
    // try to set again...
    if ( chRangeNew.cpMin == 0 )
    {
      SendMessage( hwndRE, EM_EXSETSEL, 0, (LONG)&chRangeNew );
      SendMessage( hwndRE, EM_EXGETSEL, 0, (LONG)&chRangeSet );
    } /* endif */

  }
  if ( chRangeSet.cpMax != chRangeNew.cpMax )
  {
    chRangeNew.cpMax = chRangeSet.cpMax;
    SendMessage( hwndRE, EM_EXSETSEL, 0, (LONG)&chRangeNew );
    SendMessage( hwndRE, EM_EXGETSEL, 0, (LONG)&chRangeSet );
  }
  /********************************************************************/
  /* Change into SF_UNICODE if we are supporting unicode directly     */
  /********************************************************************/
  memset( &GetTextEx, 0, sizeof( GetTextEx ));
  GetTextEx.cb    = cb;
  GetTextEx.flags = GT_DEFAULT | GT_SELECTION;
  GetTextEx.codepage = 1200; // get data in Unicode instead of pDoc->ulAnsiCodePage;

  SendMessage( hwndRE, EM_GETTEXTEX, (LONG)&GetTextEx, (LONG) pData );
  if ( ulStart > (ULONG) chRangeNew.cpMin )
  {
    // readjust data if necessary ...
    ULONG ulOffset = ulStart - chRangeNew.cpMin;
    // assure range is positive! (i.e. valid!)
    if (ulStart <= (ULONG)chRangeNew.cpMax )
    {
      memmove( pData, pData+ ulOffset, (chRangeNew.cpMax - chRangeNew.cpMin + 1 - ulOffset) * sizeof(CHAR_W));
    }
    else
    { // should not occur - TRAP SVT604 - S000017!
      WinAlarm( HWND_DESKTOP, WA_WARNING );
    }
  }
  SendMessage( hwndRE, EM_EXSETSEL, 0, (LONG)&chRange );
  SendMessage( hwndRE, EM_SETSCROLLPOS, 0, (LONG) &lPoint );
  {
    // This hook has to be added because of a bug in the Microsoft
    // RichEdit control
    // Even if we position on a specific lPoint the next GETSCROLLPOS
    // will return a different position -- therefore:
    //  loop until we reach the position, but ensure that we do not
    //  loop forever
    POINT lPoint2, lPoint1;
    USHORT i = 0;
    memcpy( &lPoint2, &lPoint, sizeof( lPoint2) );
    SendMessage( hwndRE, EM_GETSCROLLPOS, 0, (LONG) &lPoint1 );
    while ((lPoint.y > lPoint1.y) && (i<35))
    {
      i++;
      lPoint2.y += (lPoint.y - lPoint1.y);
      SendMessage( hwndRE, EM_SETSCROLLPOS, 0, (LONG) &lPoint2 );
      SendMessage( hwndRE, EM_GETSCROLLPOS, 0, (LONG) &lPoint1 );
    }
  }

  UNLOCKRICHEDIT_TRUE( pDoc, fLock );

  pDispFile->bRTFFill = b;

  /********************************************************************/
  /* request the change status from the control                       */
  /********************************************************************/
  if ( (BOOL) SendMessage( hwndRE, EM_GETMODIFY, 0L, 0L ) )
  {
    DrawFormatChars( pDoc );
    SendMessage( hwndRE, EM_SETMODIFY, 0L, 0L );
  } /* endif */

}

/**********************************************************************/
/* Scroll display, so that cursor is in usLine                        */
/**********************************************************************/
VOID EQFBScrollToLineRTF( PTBDOCUMENT pDoc, USHORT usLine )
{
  LONG lLine = SendMessage( pDoc->hwndRichEdit, EM_EXLINEFROMCHAR, 0, -1 );
  lLine; usLine;

}



/**********************************************************************/
/* GetSegmentData from screen                                         */
/**********************************************************************/
PSZ_W EQFBGetSegRTF( PTBDOCUMENT pDoc, EQFBRTFLINE * pRTFLine, ULONG ulSegNum )
{
  BOOL fLock;

  LOCKRICHEDIT( pDoc, fLock );
  // ensure that ulSegNum is inside the range
  if (ulSegNum == 0)
  {
    ulSegNum = 1;
  } /* endif */

  EQFBGetMatchingRangeRTF( pDoc, ulSegNum, pRTFLine );

  UNLOCKRICHEDIT_FALSE ( pDoc, fLock);

  return GetWorkSegDataPtr( pDoc, pRTFLine, ulSegNum );
}

PSZ_W GetWorkSegDataPtr( PTBDOCUMENT pDoc, EQFBRTFLINE * pRTFLine, ULONG ulSegNum )
{
  PSZ_W pData = NULL;


  if ( (pRTFLine->ulStartSeg <= ulSegNum) &&  (ulSegNum <= pRTFLine->ulEndSeg)   )
  {
    PTBSEGMENT  pSeg;
    ULONG  ulLen = pRTFLine->usSegTextLen[ ulSegNum - pRTFLine->ulStartSeg];
    pData = &pRTFLine->chText[ pRTFLine->ulSegTextOffs[ ulSegNum - pRTFLine->ulStartSeg] - pRTFLine->ulFirstLineIndex ];
    /******************************************************************/
    /* copy segment into temporary buffer - keep our picture of the   */
    /* screen buffer                                                  */
    /******************************************************************/
    pData = (PSZ_W)memcpy( pRTFLine->chTempBuffer, pData, ulLen*sizeof(CHAR_W) );
    pData[ ulLen ] = EOS;

    pSeg = EQFBGetSegW(pDoc, ulSegNum);
    if (pSeg && (pSeg->pusBPET) )
    {
      UtlAlloc((PVOID *)&(pSeg->pusBPET), 0L, 0L, NOMSG);
    } /* endif */
  } /* endif */

  return pData;
}

/**********************************************************************/
/* GotoSegment                                                        */
/**********************************************************************/
VOID EQFBGotoSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum, USHORT usSegOffset )
{
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);

  (pDoc->TBCursor).ulSegNum = ulSegNum;         // set segment number
  (pDoc->TBCursor).usSegOffset  = usSegOffset;  // start at the beginning
  pDoc->lSideScroll = 0;
  EQFBWorkLineOut( pDoc );                      // save current segment

  RTFDumpNoSel( pDoc, "GotoSegRTF:" );

  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
  EQFBGetMatchingRangeRTF( pDoc, ulSegNum, pRTFLine );

  /******************************************************************/
  /* find position and set caret                                    */
  /******************************************************************/
  if ( (pRTFLine->ulStartSeg <= ulSegNum) &&  (ulSegNum <= pRTFLine->ulEndSeg)  )
  {
    ULONG ulPos = pRTFLine->ulSegTextOffs[ulSegNum - pRTFLine->ulStartSeg] + usSegOffset;
    CHARRANGE chRange;
    chRange.cpMin = ulPos;
    chRange.cpMax = ulPos;
    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
  } /* endif */

  if ( pEQFBUserOpt->sFocusLine )
  {
    ULONG ulLine = (ULONG) SendMessage( pDoc->hwndRichEdit, EM_EXLINEFROMCHAR, 0, -1 );
    ULONG ulFirstLine = (ULONG) SendMessage( pDoc->hwndRichEdit, EM_GETFIRSTVISIBLELINE, 0,0);
    LONG lDiff = ulLine - ulFirstLine - pEQFBUserOpt->sFocusLine + 1;

    if (lDiff != 0)
    {
        SendMessage( pDoc->hwndRichEdit, EM_LINESCROLL, 0, lDiff );
    }
  } /* endif */

  EQFBWorkLineIn( pDoc );
}

/**********************************************************************/
/* GotoMarkSegment                                                    */
/**********************************************************************/
VOID EQFBGotoMarkSegRTF( PTBDOCUMENT pDoc,
                         ULONG ulStartSegNum, USHORT ulStartSegOffset,
                         ULONG ulEndSegNum, USHORT ulEndSegOffset )
{
  USEROPT* pEQFBUserOpt = get_EQFBUserOpt();
  PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);

  (pDoc->TBCursor).ulSegNum = ulStartSegNum;         // set segment number
  (pDoc->TBCursor).usSegOffset  = ulStartSegOffset;  // start at the beginning
  pDoc->lSideScroll = 0;
  EQFBWorkLineOut( pDoc );                      // save current segment

  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
  EQFBGetMatchingRangeRTF( pDoc, ulStartSegNum, pRTFLine );

  /******************************************************************/
  /* find position and set caret                                    */
  /******************************************************************/
  if ( (pRTFLine->ulStartSeg <= ulStartSegNum) &&  (ulStartSegNum <= pRTFLine->ulEndSeg)  )
  {
    CHARRANGE chRange;
    ULONG ulEndPos = 0l;
    ULONG ulStartPos = pRTFLine->ulSegTextOffs[ulStartSegNum - pRTFLine->ulStartSeg]
                               + ulStartSegOffset;

    if (ulStartSegNum == ulEndSegNum )
    {
       ulEndPos = ulStartPos + ulEndSegOffset - ulStartSegOffset + 1;
    }
    else
    {
      ulEndPos = ulStartPos;
    } /* endif */
    chRange.cpMin = ulStartPos;
    chRange.cpMax = ulEndPos;
    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
    UtlDispatch();
  } /* endif */
  if ( pEQFBUserOpt->sFocusLine )
  {
    ULONG ulLine = (ULONG) SendMessage( pDoc->hwndRichEdit, EM_EXLINEFROMCHAR, 0, -1 );
    ULONG ulFirstLine = (ULONG) SendMessage( pDoc->hwndRichEdit, EM_GETFIRSTVISIBLELINE, 0,0);
    LONG lDiff = ulLine - ulFirstLine - pEQFBUserOpt->sFocusLine + 1;

    if (lDiff != 0)
    {
        SendMessage( pDoc->hwndRichEdit, EM_LINESCROLL, 0, lDiff );
    }
  } /* endif */

  EQFBWorkLineIn( pDoc );
}


ULONG EQFBGetCaretPosRTF( PTBDOCUMENT pDoc )
{
  CHARRANGE chRange;
  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG)&chRange );
  return chRange.cpMin;
}

VOID EQFBSetCaretPosRTF( PTBDOCUMENT pDoc, ULONG ulCaret )
{
  CHARRANGE chRange;
  chRange.cpMin = chRange.cpMax = ulCaret;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
}


typedef struct _EQFBRTFPART
{
  PSZ     pData;                       // pointer to data
  ULONG   ulType;                      // type (i.e. dwEffects)
  USHORT  usLen;                       // length of data
} EQFBRTFPART, *PEQFBRTFPART;


VOID EQFBSetWorkSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum, PSZ_W pSetData )
{
  PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);

  BOOL   fLock;
  ULONG  ulCaret = EQFBGetCaretPosRTF( pDoc );

  LOCKRICHEDIT( pDoc, fLock );

RTFDumpNoSel( pDoc, "SetWorkSegRTF:" );

  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
  EQFBGetMatchingRangeRTF( pDoc, ulSegNum, pRTFLine );

  if ( (pRTFLine->ulStartSeg <= ulSegNum) &&  (ulSegNum <= pRTFLine->ulEndSeg)   )
  {
    BOOL fDispAdded = FALSE;
    BYTE b;
    ULONG  ulI = ulSegNum - pRTFLine->ulStartSeg;
    CHARRANGE chRange, chRange1;
    PTBSEGMENT  pSeg = EQFBGetSegW( pDoc, ulSegNum );
    CHAR   chText[2];

    chText[0] = EOS;
    // Select it and substitute it via the specified data...
    chRange.cpMin = pRTFLine->ulSegTextOffs[ ulI ];
    chRange.cpMax = pRTFLine->ulSegTextOffs[ ulI ] + pRTFLine->usSegTextLen[ulI];

    b = pDoc->pDispFileRTF->bRTFFill; pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG)&chRange1 );
    if ( chRange.cpMin == chRange.cpMax )
    {
        // do nothing -- we have no selection, therefore RTFcontrol will work...
    }
    else
    if (( !((chRange.cpMin == chRange1.cpMin) && (chRange.cpMax == chRange1.cpMax)) ) )
    {
      DeleteArea( pDoc, pRTFLine, &chRange, &chRange1, ulI, &fDispAdded );
    } /* endif */

RTFDumpSel( pDoc, "vor:");

    DisplaySegRTF( pDoc, pSeg, pSetData, FALSE);

RTFDumpNoSel( pDoc, "nach:" );

    if ( fDispAdded )
    {
      SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0L, (LONG) &chRange );
      chRange.cpMin += DISP_ADD_LEN;
      SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );

      SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) "" );
    } /* endif */
    pDoc->pDispFileRTF->bRTFFill = b;

    EQFBSetCaretPosRTF( pDoc, ulCaret );
  } /* endif */
  UNLOCKRICHEDIT_TRUE ( pDoc, fLock);
}



VOID  DeleteArea( PTBDOCUMENT pDoc, PEQFBRTFLINE pRTFLine,
                  CHARRANGE *pchRange, CHARRANGE *pchRange1, ULONG ulI, PBOOL pDispAdded )
{
  CHARFORMAT2 cfDefault;
  CHARRANGE chRange, chRangeSelStart, chRangeSelEnd, chRangeAct;
  CHAR_W chText[20];
  PSZ_W  pTemp;
  CHAR_W c;

  USHORT usI;

  *pDispAdded = FALSE;
  pchRange1->cpMin = pchRange1->cpMax = pchRange->cpMax;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)pchRange1 );
  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG)pchRange1 );
RTFDEBUG( pDoc, NULL /*"DeleteArea 1"*/ );

  /********************************************************************/
  /* handle the trailing part first...                                */
  /********************************************************************/
  if ( pchRange1->cpMax < pchRange->cpMax )
  {
    ULONG ulPos, ulPosChar;
    CHAR_W b;
    /******************************************************************/
    /* select next char, insert DISP_ADD, delete area                 */
    /******************************************************************/
    chRange.cpMax = pchRange1->cpMax;
    while ( (chRange.cpMax < pchRange->cpMax) )
    {
      SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0 );
      ulPos = chRange.cpMax;
      SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
      if ( ulPos == (ULONG)chRange.cpMax )
        break;
    } /* endwhile */
    SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0 );
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );

    chRange.cpMin = pchRange1->cpMin;
    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
    pchRange->cpMax = chRange.cpMin;

    SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) "" );

    ulPos = chRange.cpMax - pRTFLine->ulFirstLineIndex;
    b = pRTFLine->chText[ ulPos ]; pRTFLine->chText [ ulPos ] = EOS;
    memset( &cfDefault, 0, sizeof(cfDefault));
    cfDefault.cbSize = sizeof(cfDefault);
    /*******************************************************************/
    /* restore inadvertedly deleted hidden area                        */
    /*******************************************************************/
    SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, TRUE, (LONG)&cfDefault );
    ulPosChar = chRange.cpMin - pRTFLine->ulFirstLineIndex;

    if (ulPosChar < ulPos )
      SetHidden( pDoc, &pRTFLine->chText[ulPosChar], &cfDefault, NULL );
    else
      MessageBeep(0);

    pRTFLine->chText[ ulPos ] = b;


    pchRange1->cpMin = pchRange1->cpMax = pchRange->cpMax;
    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)pchRange1 );
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG)pchRange1 );

//    RTFDEBUG( pDoc, "Deleted at end" );

  } /* endif */





  /********************************************************************/
  /* now handle the heading part                                      */
  /********************************************************************/
  if (pchRange1->cpMax == pchRange->cpMax)
  {
    /*******************************************************************/
    /* Rational:                                                       */
    /* - Simulate BACKSPACE as long as we did not reach the requested  */
    /*   starting point                                                */
    /* - due to the construction we deleted one visible (protected or  */
    /*   unprotected) character too far                                */
    /* - restore this character and any requested invisible stuff      */
    /*******************************************************************/
    CHAR_W b;
    LONG   cpMin;
    ULONG  ulPos;
    ULONG  ulPosChar;

    CHARFORMAT2 cfDefault;
    chRange.cpMin = pchRange1->cpMin;
    /******************************************************************/
    /* add blank area at end                                          */
    /******************************************************************/
    SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) DISP_ADD );
    *pDispAdded = TRUE;
    cpMin = chRange.cpMin+1;
    while ( (chRange.cpMin > pchRange->cpMin) )
    {
      cpMin = chRange.cpMin;
      SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0 );
      SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
    } /* endwhile */


    cpMin = chRange.cpMin;
    SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0 );
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
    if ( cpMin+1 != chRange.cpMin )
    {
      chRange.cpMin = cpMin;
    }
    else
    {
      chRange.cpMin = cpMin+1;
    } /* endif */
    chRange.cpMax = pchRange->cpMax;

    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );

    SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) "" );

    ulPos = pchRange->cpMin - pRTFLine->ulFirstLineIndex;
    b = pRTFLine->chText[ ulPos ]; pRTFLine->chText [ ulPos ] = EOS;
    memset( &cfDefault, 0, sizeof(cfDefault));
    cfDefault.cbSize = sizeof(cfDefault);
    /*******************************************************************/
    /* restore inadvertedly deleted hidden area                        */
    /*******************************************************************/
    SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, TRUE, (LONG)&cfDefault );
    ulPosChar = chRange.cpMin - pRTFLine->ulFirstLineIndex;

    if (ulPosChar < ulPos )
      SetHidden( pDoc, &pRTFLine->chText[ulPosChar], &cfDefault, NULL );
    else
      MessageBeep(0);

    pRTFLine->chText[ ulPos ] = b;

    return;
  }
  else
  {
    int i = 5;
    i;
  }

  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
  EQFBGetMatchingRangeRTF( pDoc, ulI, pRTFLine );

  memcpy( &chRange,  pchRange,  sizeof(chRange));
  memcpy( &chRangeSelStart, pchRange1, sizeof(chRangeSelStart));
  memcpy( &chRangeSelEnd, pchRange1, sizeof(chRangeSelEnd));
  memcpy( &chRangeAct, pchRange, sizeof(chRangeAct));
  memset( &cfDefault, 0, sizeof(cfDefault));

  /********************************************************************/
  /* Rational:                                                        */
  /* 1.) find area large enough where selection settings are equal to */
  /*     selection getting                                            */
  /* 2.) Select this area and remember the header and tailer part not */
  /*     to be deleted                                                */
  /* 3.) Replace selection by the header part                         */
  /* 4.) add a blank                                                  */
  /* 5.) add the tailer part                                          */
  /* 6.) select the blank                                             */
  /********************************************************************/
  usI = 1;
  while ( (chRangeAct.cpMin != chRangeSelStart.cpMin) && (usI <= chRange.cpMin) )
  {
    chRangeSelStart.cpMin = chRange.cpMin - usI;
    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRangeSelStart );
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRangeAct );
    usI++;
  } /* endwhile */

  /********************************************************************/
  /* restore head part                                                */
  /********************************************************************/
  cfDefault.cbSize = sizeof(cfDefault);
  SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, TRUE, (LONG)&cfDefault );

  if ( chRange.cpMin > chRangeSelStart.cpMin )
  {
    cfDefault.dwEffects |= CFE_HIDDEN | CFE_PROTECTED;
    cfDefault.dwMask |= CFM_HIDDEN | CFM_PROTECTED;

    SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LONG)&cfDefault );

    pTemp = &pRTFLine->chText[ chRangeSelStart.cpMin - pRTFLine->ulFirstLineIndex ];
    usI = (USHORT)(pchRange->cpMin - chRangeSelStart.cpMin);
    c = pTemp[ usI ]; pTemp[usI] = EOS;
    SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) pTemp );
    pTemp[usI] = c;
  } /* endif */
  /********************************************************************/
  /* add a blank                                                      */
  /********************************************************************/
  cfDefault.dwEffects &= ~(CFE_HIDDEN|CFE_PROTECTED);

  SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LONG)&cfDefault );
  chText[0] = BLANK; chText[1] = EOS;
  SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) chText );
  /********************************************************************/
  /* restore tail                                                     */
  /********************************************************************/
  if ( pchRange->cpMax < chRangeSelStart.cpMax )
  {
    cfDefault.dwEffects |= CFE_HIDDEN | CFE_PROTECTED;
    SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LONG)&cfDefault );

    pTemp = &pRTFLine->chText[pchRange->cpMax - pRTFLine->ulFirstLineIndex ];
    usI = (USHORT)(chRangeSelStart.cpMax - pchRange->cpMax);
    c = pTemp[ usI ]; pTemp[usI] = EOS;
    SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) pTemp );
    pTemp[usI] = c;
  } /* endif */
  /********************************************************************/
  /* select the inserted blank                                        */
  /********************************************************************/
  memcpy( &chRange, pchRange, sizeof(chRange));
  chRange.cpMax = chRange.cpMin + 1;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
}




VOID EQFBSetJoinStartSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum )
{
  PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);
  CHAR_W      chBuffer[ 2 * MAX_SEGMENT_SIZE ];
  CHAR_W      chTemp[ 2 * MAX_SEGMENT_SIZE ];
  CHARRANGE   chRange;
  PTBSEGMENT  pSeg;
  CHARFORMAT2  cfDefault;
  BYTE         b;

  ULONG  ulCaret = EQFBGetCaretPosRTF( pDoc );

  memset( &cfDefault, 0, sizeof(cfDefault) );
  cfDefault.cbSize = sizeof(cfDefault);
  cfDefault.dwEffects = CFE_HIDDEN | CFE_PROTECTED;
  cfDefault.dwMask = CFM_HIDDEN | CFM_PROTECTED;

  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
  EQFBGetMatchingRangeRTF( pDoc, ulSegNum, pRTFLine );
  if ( (pRTFLine->ulStartSeg <= ulSegNum) &&  (ulSegNum <= pRTFLine->ulEndSeg)   )
  {
    ULONG ulI = ulSegNum - pRTFLine->ulStartSeg;
    /******************************************************************/
    /* get hidden header of segment                                   */
    /******************************************************************/
    if ( pDoc->docType == STARGET_DOC )
    {
      chRange.cpMin = pRTFLine->ulSegOffs[ ulI ];
      chRange.cpMax = chRange.cpMin;
      SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
      SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, TRUE,
                   MP2FROMP( &cfDefault));

      cfDefault.dwEffects |= (CFE_HIDDEN | CFE_PROTECTED );
      cfDefault.dwMask |= (CFM_HIDDEN | CFM_PROTECTED );
      chRange.cpMin = pRTFLine->ulSegOffs[ ulI ];
      chRange.cpMax = pRTFLine->ulSegTextOffs[ ulI ];
      SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
      SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
                    MP2FROMP( &cfDefault ));

      pSeg = EQFBGetSegW(pDoc->twin, ulSegNum);
      EQFBMakeRTFEscapedLF(chTemp, pSeg->pDataW);

//      sprintf(chBuffer, RTF_HIDDEN, ulSegNum, chTemp, strlen(chTemp) );
      swprintf(chBuffer, RTF_HIDDEN, ulSegNum, L"A", 1 );
      b = pDoc->pDispFileRTF->bRTFFill;
      pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
      SendMessage(pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, MP2FROMP(""));
      SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
                    MP2FROMP( &cfDefault ));
      SendMessage(pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, MP2FROMP(chBuffer));
      pDoc->pDispFileRTF->bRTFFill = b;
      SendMessage(pDoc->hwndRichEdit, EM_SETMODIFY, 0L,0L);

      EQFBSetCaretPosRTF( pDoc, ulCaret );
    }
  } /* endif */

}




VOID EQFBSetJoinedSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum )
{
  PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);
  CHAR_W      chBuffer[ 2 * MAX_SEGMENT_SIZE ];
  CHAR_W      chTemp[ 2 * MAX_SEGMENT_SIZE ];
  CHARRANGE   chRange;
  PTBSEGMENT  pSeg;
  CHARFORMAT2  cfDefault;
  BYTE         b;
  BOOL         fStartOfProtectAndHidden = FALSE;
  ULONG        ulHiddenCont;
  LONG         lLen = 0;

  ULONG  ulCaret = EQFBGetCaretPosRTF( pDoc );

  memset( &cfDefault, 0, sizeof(cfDefault) );
  cfDefault.cbSize = sizeof(cfDefault);
  cfDefault.dwEffects = CFE_HIDDEN | CFE_PROTECTED;
  cfDefault.dwMask = CFM_HIDDEN | CFM_PROTECTED;

  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
  EQFBGetMatchingRangeRTF( pDoc, ulSegNum, pRTFLine );

  if ( (pRTFLine->ulStartSeg <= ulSegNum) &&  (ulSegNum <= pRTFLine->ulEndSeg)   )
  {
    ULONG ulI = ulSegNum - pRTFLine->ulStartSeg;
    /******************************************************************/
    /* get hidden header of segment                                   */
    /******************************************************************/
    if ( pDoc->docType == STARGET_DOC )
    {
      chRange.cpMin = pRTFLine->ulSegOffs[ ulI ];
      chRange.cpMax = chRange.cpMin;
      SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
      SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, TRUE,
                   MP2FROMP( &cfDefault));

      cfDefault.dwEffects |= (CFE_HIDDEN | CFE_PROTECTED );
      cfDefault.dwMask |= (CFM_HIDDEN | CFM_PROTECTED );
      chRange.cpMin = pRTFLine->ulSegOffs[ ulI ];
      chRange.cpMax = pRTFLine->ulSegTextOffs[ ulI ] + pRTFLine->usSegTextLen[ulI];
      ulHiddenCont = ulI;
      /****************************************************************/
      /* this is nec. if more than 2 segs joined                      */
      /****************************************************************/
      while (!fStartOfProtectAndHidden && (ulHiddenCont > 0) )
      {
        ulHiddenCont--;
        if (pRTFLine->ulSegTextOffs[ulHiddenCont] == pRTFLine->ulSegOffs[ulHiddenCont+1] )
        {
          chRange.cpMin = pRTFLine->ulSegOffs[ulHiddenCont];
        }
        else
        {
          fStartOfProtectAndHidden = TRUE;
          ulHiddenCont ++;
        } /* endif */
      } /* endwhile */
      SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
      SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
                    MP2FROMP( &cfDefault ));

      /****************************************************************/
      /* add combined part (i.e. missing headers )                    */
      /* display all consecutive QFJ with one SendMessage call        */
      /****************************************************************/
      if ( ulHiddenCont < ulI )
      {
        lLen = pRTFLine->ulSegOffs[ulI] - pRTFLine->ulSegOffs[ulHiddenCont];
        memset( &chBuffer, 0, sizeof( chBuffer ));
        memcpy(chBuffer, &(pRTFLine->chText[pRTFLine->ulSegOffs[ulHiddenCont]]), lLen * sizeof(CHAR_W));
      }
      else
      {
      } /* endif */
      pSeg = EQFBGetSegW(pDoc->twin, ulSegNum);
      EQFBMakeRTFEscapedLF(chTemp, pSeg->pDataW);



      swprintf(&chBuffer[lLen], RTF_JOINED, ulSegNum, chTemp, UTF16strlenCHAR(chTemp) );
      b = pDoc->pDispFileRTF->bRTFFill;
      pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
      SendMessage(pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, MP2FROMP(""));
      SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
                    MP2FROMP( &cfDefault ));
      SendMessage(pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, MP2FROMP(chBuffer));
      pDoc->pDispFileRTF->bRTFFill = b;
      SendMessage(pDoc->hwndRichEdit, EM_SETMODIFY, 0L,0L);

      EQFBSetCaretPosRTF( pDoc, ulCaret );
    }
  } /* endif */
}

VOID EQFBSetSplitStartSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum )
{
  PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);
  CHAR_W       chBuffer[ 2 * MAX_SEGMENT_SIZE ];
  CHARRANGE    chRange;
  CHARFORMAT2  cfDefault;
  BYTE         b;
  ULONG        ulCaret = EQFBGetCaretPosRTF( pDoc );


  memset( &cfDefault, 0, sizeof(cfDefault) );
  cfDefault.cbSize = sizeof(cfDefault);
  cfDefault.dwEffects = CFE_HIDDEN | CFE_PROTECTED;
  cfDefault.dwMask = CFM_HIDDEN | CFM_PROTECTED;

  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
  EQFBGetMatchingRangeRTF( pDoc, ulSegNum, pRTFLine );

  if ( (pRTFLine->ulStartSeg <= ulSegNum) &&  (ulSegNum <= pRTFLine->ulEndSeg)   )
  {
    ULONG  ulI = ulSegNum - pRTFLine->ulStartSeg;
    chRange.cpMin = pRTFLine->ulSegOffs[ ulI ];
    chRange.cpMax = pRTFLine->ulSegTextOffs[ulI];
    if ( pDoc->docType == STARGET_DOC )
    {
//    pSrcSeg = EQFBGetSeg(pDoc->twin, ulSegNum);
//    EQFBMakeRTFEscapedLF(chTemp, pSrcSeg->pData);
//    sprintf(chBuffer, RTF_HIDDEN, ulSegNum, chTemp, strlen(chTemp) );
      swprintf( chBuffer, RTF_HIDDEN, ulSegNum, L"A", 1);



      SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
      SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
                    MP2FROMP( &cfDefault ));
      b = pDoc->pDispFileRTF->bRTFFill;
      pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
      SendMessage(pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, MP2FROMP(""));
      SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
                    MP2FROMP( &cfDefault ));
      SendMessage(pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, MP2FROMP(chBuffer));
      pDoc->pDispFileRTF->bRTFFill = b;
      SendMessage(pDoc->hwndRichEdit, EM_SETMODIFY, 0L,0L);

    } /* endif */
    EQFBSetCaretPosRTF( pDoc, ulCaret );
  } /* endif */
}

VOID EQFBSetSplittedSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum )
{
  ulSegNum; pDoc;
  return;




//  PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);
//  CHAR_W       chBuffer[ 2 * MAX_SEGMENT_SIZE ];
//  CHARRANGE    chRange;
//  PTBSEGMENT   pSeg;
//  PTBSEGMENT   pSrcSeg;
//  CHARFORMAT2  cfDefault;
//  CHARFORMAT2  CharFormat2;
//  BOOL         b;
//  USHORT       usState = UNPROTECTED_CHAR;        // status of character
//  USHORT       usHLState = NO_HIGHLIGHT;

//  ULONG        ulCaret = EQFBGetCaretPosRTF( pDoc );


//  memset( &cfDefault, 0, sizeof(cfDefault) );
//  cfDefault.cbSize = sizeof(cfDefault);
//  cfDefault.dwEffects = CFE_HIDDEN | CFE_PROTECTED;
//  cfDefault.dwMask = CFM_HIDDEN | CFM_PROTECTED;
//
//  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
//  EQFBGetMatchingRangeRTF( pDoc, ulSegNum, pRTFLine );
//
//  if ( (pRTFLine->ulStartSeg <= ulSegNum) &&  (ulSegNum <= pRTFLine->ulEndSeg)   )
//  {
//    /******************************************************************/
//    /* Rational:                                                      */
//    /*   Due to problems in with consecutive hidden parts, we have    */
//    /*   to select all consecutive hidden fields, delete them, insert */
//    /*   our new stuff and than add the 'accidently deleted parts'..  */
//    /* Base assumption:                                               */
//    /*   all consecutive headers are within a single RTFLine struct.. */
//    /******************************************************************/
//    ULONG  ulI = ulSegNum - pRTFLine->ulStartSeg;
//    ULONG  ulHiddenCont = ulI;
//    BOOL   fEndOfProtectAndHidden = FALSE;
//    /******************************************************************/
//    /* get hidden header of segment                                   */
//    /******************************************************************/
//    chRange.cpMin = pRTFLine->ulSegOffs[ ulI ];
//    chRange.cpMax = pRTFLine->ulSegTextOffs[ulI];
//    while (!fEndOfProtectAndHidden )
//    {
//      if (pRTFLine->ulSegTextOffs[ulHiddenCont] == pRTFLine->ulSegOffs[ulHiddenCont+1] )
//      {
//        chRange.cpMax = pRTFLine->ulSegTextOffs[ulHiddenCont+1];
//        ulHiddenCont++;
//      }
//      else
//      {
//        fEndOfProtectAndHidden = TRUE;
//      } /* endif */
//    } /* endwhile */
//    if ( pDoc->docType == STARGET_DOC )
//    {
//      PSZ_W pRTFFont;
////    pSrcSeg = EQFBGetSeg(pDoc->twin, ulSegNum);
////    EQFBMakeRTFEscapedLF(chTemp, pSrcSeg->pData);
////    sprintf(chBuffer, RTF_HIDDEN, ulSegNum, chTemp, strlen(chTemp) );
//      swprintf( chBuffer, RTF_HIDDEN, ulSegNum, L"A", 1);
//
//
//      SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
//      SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
//                    MP2FROMP( &cfDefault ));
//      b = pDoc->pDispFileRTF->bRTFFill;
//      pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
//      SendMessage(pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, MP2FROMP(""));
//      SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
//                    MP2FROMP( &cfDefault ));
//      SendMessage(pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, MP2FROMP(chBuffer));
//      pDoc->pDispFileRTF->bRTFFill = b;
//      /****************************************************************/
//      /* set in source text again                                     */
//      /****************************************************************/
//      if ( pDoc->docType == STARGET_DOC )
//      {
//        pSeg = EQFBGetSegW(pDoc, ulSegNum);
//      }
//      if ( pSeg->qStatus == QF_NOP )
//      {
//        usState = PROTECTED_CHAR;
//      }
//      memset( &CharFormat2, 0, sizeof( CharFormat2 ));
//      CharFormat2.cbSize = sizeof(CharFormat2);
//      pRTFFont = EQFBSegColRTF( &CharFormat2, pSeg, usState, usHLState,
//                    DISP_PROTECTED, pDoc->EQFBFlags.PostEdit,
//                     pDoc->pDispFileRTF );
//      CharFormat2.dwMask |= CFM_PROTECTED | CFM_HIDDEN;
//      // Ensure, that only areas of interest are freely editable...
//      if ( pDoc->EQFBFlags.PostEdit || (pSeg->qStatus == QF_CURRENT) )
//      {
//        CharFormat2.dwEffects &= ~CFE_PROTECTED;
//      }
//      else
//      {
//        CharFormat2.dwEffects |= CFE_PROTECTED;
//      } /* endif */
//      SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP( &CharFormat2 ));
//      DisplaySegRTF( pDoc, pSeg, pSrcSeg->pDataW, NOTSPEC );
//      /****************************************************************/
//      /* add combined part (i.e. missing headers )                    */
//      /****************************************************************/
//      if ( ulHiddenCont > ulI )
//      {
//        LONG  lLen = pRTFLine->ulSegTextOffs[ulHiddenCont] -
//                       pRTFLine->ulSegTextOffs[ulI];
//        memset( &chBuffer, 0, sizeof( chBuffer ));
//
//        memcpy(chBuffer, &(pRTFLine->chText[pRTFLine->ulSegTextOffs[ulI]]), lLen);
//
//        b = pDoc->pDispFileRTF->bRTFFill;
//        pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
//        SendMessage( pDoc->hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION,
//                      MP2FROMP( &cfDefault ));
//        SendMessage(pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, MP2FROMP(chBuffer));
//        pDoc->pDispFileRTF->bRTFFill = b;
//      }
//      else
//      {
//      } /* endif */
//      SendMessage(pDoc->hwndRichEdit, EM_SETMODIFY, 0L,0L);
//
//    }
//    EQFBSetCaretPosRTF( pDoc, ulCaret );
//  } /* endif */
}

VOID RTFRestoreInline( PTBDOCUMENT pDoc, PSZ_W pData )
{
  CHAR_W      chData[ MAX_SEGMENT_SIZE ];
  PSZ_W       pTemp;
  PTOKENENTRY pTok;
  PCHAR_W     pRest = NULL;
  BOOL        fQFIPending = FALSE;
  USHORT      usStart = 0;

  chData[0] = EOS;
  pTemp = &chData[0];

  // pData spans only one segment, hence no do-while necessary
  TATagTokenizeW(pData, (PLOADEDTABLE)pDoc->pDispFileRTF->pQFRTFTagTable,
                 TRUE, &pRest, &usStart,
                 (PTOKENENTRY) pDoc->pTokBuf,
                 TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );


  pTok = (PTOKENENTRY) pDoc->pTokBuf;
  while ( pTok->sTokenid != ENDOFLIST )
  {
    switch ( pTok->sTokenid )
    {
      case WHITESPACE:
        break;
      case TEXT_TOKEN:
        if ( !fQFIPending )
        {
          memcpy( pTemp, pTok->pDataStringW, pTok->usLength * sizeof(CHAR_W));
          pTemp += pTok->usLength;
        }
        break;

      default:
        switch ( pTok->usOrgId )
        {
           case RTF_QFI_TAG:
             fQFIPending = TRUE;
             break;
           case RTF_EQFI_TAG:
             fQFIPending = FALSE;
             break;
           case RTF_QFJ_TAG:
             break;
           case RTF_T_TAG:
             pTemp +=RTFInlineDeEscape( pTemp, pTok->pDataStringW+3,
                                        (USHORT)(pTok->usLength-5) );
             break;
           case RTF_NONE_TAG:
             if ( !fQFIPending )
             {
                pTemp +=RTFInlineDeEscape( pTemp, pTok->pDataStringW, pTok->usLength );
             }
             break;

           case RTF_QFF_TAG:
           case RTF_N_TAG:
           case RTF_S_TAG:
           case RTF_L_TAG:
           default:
             break;
        } /* endswitch */
        break;
    } /* endswitch */
     pTok++;
  } /* endwhile */
  *pTemp = EOS;
  UTF16strcpy( pData, chData );
  return;

}


VOID EQFBGetWorkSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum )
{
  PSZ_W pData;
  BOOL fLock;
  PEQFBRTFLINE pRTFLine = &(pDoc->pDispFileRTF->RTFLine);

  LOCKRICHEDIT( pDoc, fLock );
  memset( pRTFLine, 0, sizeof( EQFBRTFLINE ));
  pData = EQFBGetSegRTF( pDoc, pRTFLine, ulSegNum );
  UNLOCKRICHEDIT_FALSE( pDoc, fLock );

  AdjustWorkSegData( pDoc, pData, ulSegNum );
}

void AdjustWorkSegData( PTBDOCUMENT pDoc, PSZ_W pData, ULONG ulSegNum )
{
  if ( pData && ulSegNum )
  {
    PSZ_W pTemp = pDoc->pEQFBWorkSegmentW;
    CHAR_W c;
    while ( (c = *pTemp++ = *pData++) != NULC )
    {
      if (c == '\r')
        *(pTemp-1) = '\n';
    }
    *pTemp = EOS;

    RTFRestoreInline( pDoc, pDoc->pEQFBWorkSegmentW );
    if ( pDoc->pTBSeg )
    {
      EQFBCompSeg(pDoc->pTBSeg );
    } /* endif */

    // check if Workseg changed
    if ( pDoc->pSaveSegW )
    {
	  // take care if we have copied a fuzzy proposal
      if (pDoc->fFuzzyCopied && pDoc->sPropCopied >= 1)
	  {
         UtlAlloc( (PVOID *) &pData, 0L, MAX_PROPLENGTH * sizeof(CHAR_W) * 2, ERROR_STORAGE);
         if ( pData )
         {
           USHORT usLevel = QUERYIFSOURCEISEQUAL; // request source of prop equal to source
                                      // flag, the flag is returned as first
                                      // bit of the returned usLevel

           EQFGETPROPW( (USHORT)pDoc->sPropCopied, pData, &usLevel );   // get proposal data

           pDoc->EQFBFlags.workchng |= (UTF16strcmp( (PSZ_W)pData, pDoc->pEQFBWorkSegmentW ) != 0);
           UtlAlloc( (PVOID *) &pData, 0L, 0L, NOMSG);
		}
	  }
	  else
	  {
        pDoc->EQFBFlags.workchng |= (UTF16strcmp( pDoc->pSaveSegW, pDoc->pEQFBWorkSegmentW ) != 0);
	  }
    }
    else
    {
      pDoc->EQFBFlags.workchng |= (BOOL) SendMessage( pDoc->hwndRichEdit, EM_GETMODIFY, 0L, 0L );
    }
    SendMessage( pDoc->hwndRichEdit, EM_SETMODIFY, 0L, 0L );
  }
  else
  {
    pDoc->pEQFBWorkSegmentW[0] = EOS;
  } /* endif */
}


BOOL PrepareSegOffsTable( PTBDOCUMENT pDoc, PSZ_W pData, PEQFBRTFLINE pRTFLine )
{
  BOOL fOK = TRUE;
  pRTFLine->ulEndSeg = pRTFLine->ulStartSeg = 0;
  memset( &pRTFLine->ulSegOffs[0], 0, sizeof(ULONG) * OFFSET_NUM );
  memset( &pRTFLine->ulSegTextOffs[0], 0, sizeof(ULONG) * OFFSET_NUM );
  memset( &pRTFLine->usSegTextLen[0], 0, sizeof(USHORT) * OFFSET_NUM );


  if ( fOK )
  {
    PTOKENENTRY pTok;
    USHORT      usStart = 0;
    USHORT      usTagStart;
	ULONG       ulStringOff = 0;
    PCHAR_W     pRest = NULL;
    USHORT      ulI;
    BOOL        fQFIPending = FALSE;
    PSZ_W       pDataStart = pData;

	// ensure, that we can deal with buffer overflows, ie. pRests
	do
    {
      usTagStart = 0;
      TATagTokenizeW(pData, (PLOADEDTABLE)pDoc->pDispFileRTF->pQFRTFTagTable,
                     TRUE, &pRest, &usTagStart,
                     (PTOKENENTRY) pDoc->pTokBuf,
                     TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

      pTok = (PTOKENENTRY) pDoc->pTokBuf;

      while ( pTok->sTokenid != ENDOFLIST )
      {
        switch ( pTok->sTokenid )
        {
          case WHITESPACE:
            break;
          case TEXT_TOKEN:
            // ignore inserted token at begin
            if ( usStart )
            {
              if ( pRTFLine->ulSegTextOffs[usStart-1] == 0L)
              {
                pRTFLine->ulSegTextOffs[ usStart-1 ] = ulStringOff + pTok->pDataStringW - pData+
                                                       pRTFLine->ulFirstLineIndex;
              }
              pRTFLine->usSegTextLen[ usStart-1 ] = (USHORT)(pRTFLine->usSegTextLen[ usStart-1 ]+pTok->usLength);
            } /* endif */
            break;

          default:
            switch ( pTok->usOrgId )
            {
               case RTF_QFF_TAG:
                  pRTFLine->ulSegOffs[ usStart ] = ulStringOff + pTok->pDataStringW - pData +
                                                       pRTFLine->ulFirstLineIndex;
                  // check if we have to deal with a pending joined segment
                  if ( usStart > 0 && pRTFLine->usSegTextLen[ usStart-1 ] == 0 )
                  {
                    pRTFLine->ulSegTextOffs[ usStart - 1 ] = pRTFLine->ulSegOffs[ usStart ];
                  }
                  usStart++;
                  break;
                case RTF_QFI_TAG:
                  if ( usStart && pRTFLine->ulSegTextOffs[ usStart - 1 ] == 0)
                  {
                    pRTFLine->ulSegTextOffs[ usStart-1 ] = ulStringOff + pTok->pDataStringW - pData+
                                                            pRTFLine->ulFirstLineIndex;
                    pRTFLine->usSegTextLen[ usStart-1 ] = pTok->usLength;
                  } /* endif */
                  fQFIPending = TRUE;
                  break;
               case RTF_EQFI_TAG:
                  pRTFLine->usSegTextLen[ usStart-1 ] = (USHORT)(pRTFLine->usSegTextLen[ usStart-1 ]+pTok->usLength);
                  fQFIPending = FALSE;
                  break;
               case RTF_QFJ_TAG:
                  pRTFLine->ulSegOffs[ usStart ] = ulStringOff + pTok->pDataStringW - pData +
                                                       pRTFLine->ulFirstLineIndex;
                  pRTFLine->usSegTextLen[ usStart ] = 0;
                  // check if we have to deal with a pending joined segment
                  if ( usStart > 0 && pRTFLine->usSegTextLen[ usStart-1 ] == 0 )
                  {
                    pRTFLine->ulSegTextOffs[ usStart - 1 ] = pRTFLine->ulSegOffs[ usStart ];
                  }
                  usStart++;
                  break;
                case RTF_N_TAG:
                  if ( !pRTFLine->ulStartSeg  )
                  {
                    PSZ_W pszTarget = pTok->pDataStringW;
                    while ( *pszTarget && (*pszTarget != '=') )
                    {
                       pszTarget++;
                    } /* endwhile */
                    GETNUMBER( pszTarget, pRTFLine->ulStartSeg );
                  } /* endif */
                  break;
                case RTF_S_TAG:
                  if ( fQFIPending && usStart)
                  {
                    pRTFLine->usSegTextLen[ usStart-1 ] = (USHORT)(pRTFLine->usSegTextLen[ usStart-1 ]+pTok->usLength);
                  } /* endif */
                  break;
                case RTF_L_TAG:
                  break;
                case RTF_T_TAG:
                  if ( fQFIPending && usStart)
                  {
                    pRTFLine->usSegTextLen[ usStart-1 ] = (USHORT)(pRTFLine->usSegTextLen[ usStart-1 ]+pTok->usLength);
                  } /* endif */
                  break;
                case RTF_NONE_TAG:
                  if ( usStart )
                  {
                      if ( pRTFLine->ulSegTextOffs[usStart-1] == 0L)
                      {
                          pRTFLine->ulSegTextOffs[ usStart-1 ] = ulStringOff + pTok->pDataStringW - pData+
                                                                   pRTFLine->ulFirstLineIndex;
                      }
                      pRTFLine->usSegTextLen[ usStart-1 ] = (USHORT)(pRTFLine->usSegTextLen[ usStart-1 ] + pTok->usLength);
                  } /* endif */
                  break;
                default:
                  break;
            } /* endswitch */
            break;
        } /* endswitch */
         pTok++;
      } /* endwhile */
      /**********************************************************/
      /* is there still data to be processed                    */
      /**********************************************************/
      if (pRest)
      {
		ulStringOff = pRest - pDataStart;  	// new offest from start
        pData = pRest;						// reset working start
        pRest = NULL;
      }
      else
      {
        pData = NULL;
      }
    } while (pData);

    pRTFLine->ulEndSeg = pRTFLine->ulStartSeg + usStart - 1;
    // Adjust the text lengths
    for ( ulI = 1; ulI < usStart; ulI++ )
    {
      pRTFLine->usSegTextLen[ ulI - 1 ] = (USHORT)(pRTFLine->ulSegOffs[ ulI ] - pRTFLine->ulSegTextOffs[ ulI - 1]);
    }

  } /* endif */

  return fOK;
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBDoNextTwoRTF                                         |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBDoNextTwo( PTBDOCUMENT, PUSHORT, USHORT );           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       find the next two (untranslated) segments (if            |
//|                   available) and send them to the services                 |
//|                   the first in FOREGROUND mode, the later in               |
//|                   BACKGROUND mode for processing                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT    -- pointer to document instance data      |
//|                   PUSHORT        -- the segment number where to start from |
//|                                     will contain next active segment       |
//|                   USHORT         -- type of segment to be searched for     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:           EQFBDoNextTwo( pDoc, &ulSegNum, POS_TOBE);               |
//|                      this call will look only for segments TOBE transl.    |
//|                   EQFBDoNextTwo( pDoc, &ulSegNum, POS_TOBEORDONE);         |
//|                      this call will look only for segments TOBE transl.    |
//+----------------------------------------------------------------------------+
//|Function flow:     - send next segment to services as foreground request    |
//|                   - if ok activate current segment and send                |
//|                      next-next segment to services                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
VOID EQFBDoNextTwoRTF( PTBDOCUMENT pDoc,     // pointer to document
                    PULONG  pulSegNum,    // segment number where to start
                    USHORT  usCond )      // what to search for
{
   ULONG    ulSegNum;                     // segment number where to start
   BOOL fOK = TRUE;                       // succes indicator


   ulSegNum =  *pulSegNum;
   // send next segment to services
   fOK = EQFBSendNextSource(pDoc,         // pointer to document
                            &ulSegNum,    // pointer to new segment
                            TRUE,         // send in foreground mode
                            usCond);      // untranslated only?
   if ( fOK )
   {
     EQFBActivateSegm( pDoc, ulSegNum );         // activate the current segment
     SendMessage( pDoc->hwndRichEdit, EM_EMPTYUNDOBUFFER, 0L, 0L );

      *pulSegNum = ulSegNum;                   // that's our active segment

      ulSegNum ++;
      fOK = EQFBSendNextSource
               ( pDoc,                  // pointer to document
                 &ulSegNum,             // pointer to line number
                 FALSE,                 // background mode
                 usCond );              // untranslated only?
   } /* endif */
   return;
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBNextUnprotected                                      |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBNextUnprotected(PTBDOCUMENT, PTBSEGMENT, USHORT )    |
//+----------------------------------------------------------------------------+
//|Description:       determine type of character under cursor                 |
//|                   The function evaluates the type of a character at a      |
//|                    given position in the segment table of document doc.    |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT   pDoc   - ptr to the structure describing   |
//|                                          the active document               |
//|                   PTBSEGMENT    pTBSeg - entry number of active segment    |
//|                   USHORT        usOffs - offset of current character in    |
//|                                           active segment                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       usType - PROTECTED_CHAR   : the character is display only|
//|                          - UNPROTECTED_CHAR : the character can be changed |
//|                                                                            |
//| not used in RTF:         - HIDDEN_CHAR      : the character is hidden      |
//|                          - LINEBREAK_CHAR   : the character is a line break|
//|                          - ENDOFSEG_CHAR    : the character is outside the |
//|                                                   active segment           |
//|                          - COMPACT_CHAR     : the character is compact     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      The document tag table must have been loaded.            |
//+----------------------------------------------------------------------------+
//|Side effects:      If the start/stop table does not exist for the           |
//|                   active segment, it is created.                           |
//+----------------------------------------------------------------------------+
//|Function flow:     get ptr to segment                                       |
//|                   if start/stop table is NULL                              |
//|                      tokenize segment                                      |
//|                      convert tokens to entries in start/stop table         |
//|                   endif                                                    |
//|                   if given offset greater than length of segment           |
//|                     set type to UNPROTECTED_CHAR                           |
//|                   endif                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
USHORT EQFBNextUnprotected
(
   PTBDOCUMENT pDoc,                    // ptr to active document
   PTBSEGMENT  pSeg,                    // number of active segment
   PUSHORT     pusOffs                  // offset in active segment
)
{
   USHORT      usType = UNPROTECTED_CHAR;  // type of active character
   USHORT      usRC = NO_ERROR;        // no error
   HWND        hwndTemp;               // temp. window handle
   PSTEQFGEN   pstEQFGen;              // pointer to generic structure
   USHORT      usCurOffs = *pusOffs;
   USHORT      usColPos = 0;
   PSTARTSTOP  pstCurrent;             // ptr to entries of start/stop table

   if (pSeg )
   {
     usRC = TACreateProtectTableW( pSeg->pDataW,
                                  pDoc->pDocTagTable,
                                  usColPos,
                                  (PTOKENENTRY) pDoc->pTokBuf,
                                  TOK_BUFFER_SIZE,
                                  (PSTARTSTOP *) &(pSeg->pusBPET),
                                  pDoc->pfnUserExit, pDoc->pfnUserExitW, pDoc->ulOemCodePage );

     // issue a close request to the translation environment window
     if ( pDoc->pstEQFGen && usRC )
     {
       if ( !pDoc->fErrorProcessed )
       {
         UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR );
       } /* endif */
       pDoc->fErrorProcessed = TRUE;       // error is processed
       pstEQFGen = (PSTEQFGEN) pDoc->pstEQFGen;
       hwndTemp = pDoc->hwndFrame;
       UtlDispatch();                       // display error message
       pDoc = ACCESSWNDIDA( hwndTemp, PTBDOCUMENT );
       WinPostMsg( pstEQFGen->hwndTWBS, WM_CLOSE, NULL, NULL );
     } /* endif */

     if ( pDoc && !usRC && pSeg->pusBPET)
     {
       // look for position in start/stop table
       pstCurrent = (PSTARTSTOP) pSeg->pusBPET;
       while ( (pstCurrent->usType != 0) && (usCurOffs > pstCurrent->usStop) )
       {
          pstCurrent++;
       } /* endwhile */
       while ((pstCurrent->usType != 0 ) &&
              (pstCurrent->usType == PROTECTED_CHAR ))
       {
         pstCurrent ++;
         usCurOffs = pstCurrent->usStart;
       } /* endif */

       usType = pstCurrent->usType;
       *pusOffs = usCurOffs;
     } /* endif */

   } /* endif */
   return( usType );
} /* end of EQFBNextUnprotected */





VOID EQFBDisplayFileNewRTF( PTBDOCUMENT pDoc )
{
  CHARRANGE chRange;

#ifdef RICHEDIT_DUMMY1
  BYTE b;
  BOOL fLock;
  /**************************************************************/
  /* select all and write file new                              */
  /**************************************************************/
  LOCKRICHEDIT( pDoc, fLock );
  chRange.cpMin = 0; chRange.cpMax = -1;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
  b = pDoc->pDispFileRTF->bRTFFill;
  pDoc->pDispFileRTF->bRTFFill |= RTF_FILL;
  SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, FALSE, MP2FROMP(" "));
  pDoc->pDispFileRTF->bRTFFill = b;

  chRange.cpMin = 1; chRange.cpMax = 1;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );

  EQFBDispFileRTF( pDoc );

  chRange.cpMin = 0; chRange.cpMax = 1;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
  b = pDoc->pDispFileRTF->bRTFFill;
  pDoc->pDispFileRTF->bRTFFill |= RTF_FILL;
  SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, FALSE, MP2FROMP(""));
  pDoc->pDispFileRTF->bRTFFill = b;

  UNLOCKRICHEDIT( pDoc, fLock, TRUE );
#else
  BOOL fLock;
  BYTE b = pDoc->pDispFileRTF->bRTFFill;
  /**************************************************************/
  /* select all and write file new                              */
  /**************************************************************/
  pDoc->pDispFileRTF->bRTFFill |= RTF_FILL;
  LOCKRICHEDIT( pDoc, fLock );
  chRange.cpMin = 0; chRange.cpMax = -1;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
  SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, FALSE, MP2FROMP(""));
  UNLOCKRICHEDIT_TRUE( pDoc, fLock );

  EQFBDispFileRTF( pDoc );
  pDoc->pDispFileRTF->bRTFFill = b;

#endif
}


VOID  WYSIWYG_HTML_SupportedTags( PTBDOCUMENT pDoc, PSZ pData, PSHORT psTags )
{
  PTOKENENTRY pTok;
  PSZ         pRest = NULL;
  USHORT      usStart = 0;
  // no buffer overflow can happen...
  TATagTokenize( pData, (PLOADEDTABLE)pDoc->pDocTagTable, TRUE, &pRest, &usStart,
                 (PTOKENENTRY) pDoc->pTokBuf,
                 TOK_BUFFER_SIZE / sizeof(TOKENENTRY), pDoc->ulOemCodePage );


  pTok = (PTOKENENTRY) pDoc->pTokBuf;
  while ( pTok->sTokenid != ENDOFLIST )
  {
    *psTags++ = pTok->sTokenid;
     pTok++;
  } /* endwhile */
}


char HTML_SUPPORTEDTAGS[] = "<HTML></HTML><h1></h1><h2></h2><h3></h3><h4></h4><ul><li></ul><ol></ol><b></b><i></i><u></u><br><p>";
#define WYSIWYG_HTML_HTML_TAG  0
#define WYSIWYG_HTML_EHTML_TAG        1
#define WYSIWYG_HTML_HEAD1_TAG             2
#define WYSIWYG_HTML_EHEAD1_TAG                 3
#define WYSIWYG_HTML_HEAD2_TAG                       4
#define WYSIWYG_HTML_EHEAD2_TAG                           5
#define WYSIWYG_HTML_HEAD3_TAG                                6
#define WYSIWYG_HTML_EHEAD3_TAG                                   7
#define WYSIWYG_HTML_HEAD4_TAG                                         8
#define WYSIWYG_HTML_EHEAD4_TAG                                            9
#define WYSIWYG_HTML_UL_TAG                                                    10
#define WYSIWYG_HTML_LI_TAG                                                         11
#define WYSIWYG_HTML_EUL_TAG                                                            12
#define WYSIWYG_HTML_OL_TAG                                                                 13
#define WYSIWYG_HTML_EOL_TAG                                                                   14
#define WYSIWYG_HTML_BOLD_TAG                                                                      15
#define WYSIWYG_HTML_EBOLD_TAG                                                                         16
#define WYSIWYG_HTML_ITALIC_TAG                                                                            17
#define WYSIWYG_HTML_EITALIC_TAG                                                                               18
#define WYSIWYG_HTML_UNDERL_TAG                                                                                    19
#define WYSIWYG_HTML_EUNDERL_TAG                                                                                      20
#define WYSIWYG_HTML_BR_TAG                                                                                               21
#define WYSIWYG_HTML_P_TAG                                                                                                    22


VOID  WYSIWYG_HTML_Display( PTBDOCUMENT pDoc, PSZ_W pData,
                            CHARFORMAT2 *pCharFormat2, PTBSEGMENT pSeg )
{
  CHAR_W   chBuffer[ MAX_SEGMENT_SIZE ];
  CHAR_W   chBufferTemp[ MAX_SEGMENT_SIZE ];



  CHAR_W   chData[ MAX_SEGMENT_SIZE ];
  PSZ_W    pTemp;
  PTOKENENTRY pTok;
  PCHAR_W  pRest = NULL;
  USHORT   usStart = 0;
  CHAR_W   c;
  PSHORT   psTag = &(pDoc->pDispFileRTF->sTagIndeces[0]);

  pSeg;
  chData[0] = EOS;
  pTemp = &chData[0];

  /********************************************************************/
  /* set tag indeces (if not done yet)                                */
  /********************************************************************/
  if ( psTag[0] == 0 )
  {
    WYSIWYG_HTML_SupportedTags( pDoc, HTML_SUPPORTEDTAGS, psTag);
  } /* endif */


  TATagTokenizeW( pData, (PLOADEDTABLE)pDoc->pDocTagTable, TRUE, &pRest, &usStart,
                 (PTOKENENTRY) pDoc->pTokBuf,
                 TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );


  pCharFormat2->dwMask |= (CFM_PROTECTED|CFM_HIDDEN|CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE);

  pTok = (PTOKENENTRY) pDoc->pTokBuf;
  while ( pTok->sTokenid != ENDOFLIST )
  {
    SHORT sId = pTok->sTokenid;
    c = pTok->pDataStringW[pTok->usLength];
    pTok->pDataStringW[pTok->usLength] = EOS;

    if ( sId == TEXT_TOKEN )
    {
      pCharFormat2->dwEffects &= ~(CFE_PROTECTED | CFE_HIDDEN);
      SetNormal( pDoc, pTok->pDataStringW, pCharFormat2, L"" );
    }
    else if ( (sId == psTag[WYSIWYG_HTML_UL_TAG]) || (sId == psTag[WYSIWYG_HTML_EUL_TAG]) )
    {
      /****************************************************************/
      /* ignore UL...                                                 */
      /****************************************************************/
      swprintf( chBuffer, RTF_IGNORESTYLE, pTok->pDataStringW ,L"" );
      pCharFormat2->dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
    }
    else if ( sId == psTag[WYSIWYG_HTML_LI_TAG] )
    {
      /****************************************************************/
      /* add bullet                                                   */
      /****************************************************************/
      swprintf( chBuffer, RTF_INLINESTYLE, pTok->pDataStringW ,L"" );
      pCharFormat2->dwEffects |=  (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      pCharFormat2->dwEffects &= ~(CFE_HIDDEN);
      pCharFormat2->dwEffects |= CFE_BOLD;

      chBuffer[0] = BLANK;
      chBuffer[1] = 0x07;
      chBuffer[2] = BLANK;
      chBuffer[3] = EOS;
      ASCII2Unicode( pDoc->pDispFileRTF->pCharFormat[MAX_MAXRTFFONTS],
                     &pDoc->pDispFileRTF->chRTFFormatW[0],
                     pDoc->ulOemCodePage);

      SetProtected( pDoc, chBuffer, NULL, &pDoc->pDispFileRTF->chRTFFormatW[0] );

      pCharFormat2->dwEffects &= ~(CFE_BOLD);
      pCharFormat2->dwEffects |=  (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, RTF_ENDINLINESTYLE, pCharFormat2, L"" );
    }
    else if ( sId == psTag[WYSIWYG_HTML_BOLD_TAG] )
    {
      /****************************************************************/
      /* set text to bold                                             */
      /****************************************************************/
      swprintf( chBuffer, RTF_IGNORESTYLE, pTok->pDataStringW ,L"" );
      pCharFormat2->dwEffects |=  (CFE_PROTECTED | CFE_HIDDEN | CFE_BOLD);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      UTF16strcpy( &chRTFFont[0], L"\\b ");
    }
    else if ( sId == psTag[WYSIWYG_HTML_EBOLD_TAG] )
    {
      /****************************************************************/
      /* set text to /bold                                            */
      /****************************************************************/
      swprintf( chBuffer, RTF_IGNORESTYLE, pTok->pDataStringW ,L"" );
      pCharFormat2->dwEffects &=  ~CFE_BOLD;
      pCharFormat2->dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      UTF16strcpy( &chRTFFont[0], L"\\b0 ");
    }
    else if ( sId == psTag[WYSIWYG_HTML_ITALIC_TAG] )
    {
      /****************************************************************/
      /* set text to italic                                           */
      /****************************************************************/
      swprintf( chBuffer, RTF_IGNORESTYLE, pTok->pDataStringW, L"" );
      pCharFormat2->dwEffects |=  (CFE_PROTECTED | CFE_HIDDEN | CFE_ITALIC);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      UTF16strcpy( &chRTFFont[0], L"\\i ");
    }
    else if ( sId == psTag[WYSIWYG_HTML_EITALIC_TAG] )
    {
      /****************************************************************/
      /* set text to /italics                                         */
      /****************************************************************/
      swprintf( chBuffer, RTF_IGNORESTYLE, pTok->pDataStringW, L"" );
      pCharFormat2->dwEffects &=  ~CFE_ITALIC;
      pCharFormat2->dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      UTF16strcpy( &chRTFFont[0], L"\\i0 ");
    }
    else if ( sId == psTag[WYSIWYG_HTML_UNDERL_TAG] )
    {
      /****************************************************************/
      /* set text to underline                                        */
      /****************************************************************/
      swprintf( chBuffer, RTF_IGNORESTYLE, pTok->pDataStringW, L"" );
      pCharFormat2->dwEffects |=  (CFE_PROTECTED | CFE_HIDDEN | CFE_UNDERLINE);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      UTF16strcpy( &chRTFFont[0], L"\\u ");
    }
    else if ( sId == psTag[WYSIWYG_HTML_EUNDERL_TAG] )
    {
      /****************************************************************/
      /* set text to /underline                                       */
      /****************************************************************/
      swprintf( chBuffer, RTF_IGNORESTYLE, pTok->pDataStringW, L"" );
      pCharFormat2->dwEffects &=  ~CFE_UNDERLINE;
      pCharFormat2->dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      UTF16strcpy( &chRTFFont[0], L"\\u0 ");
    }
    else if ( (sId == psTag[WYSIWYG_HTML_BR_TAG]) || (sId == psTag[WYSIWYG_HTML_P_TAG]) )
    {
      USHORT usDispStyle = (USHORT)pDoc->DispStyle;
      swprintf( chBuffer, RTF_INLINESTYLE, pTok->pDataStringW, L"" );
      pCharFormat2->dwMask |= (CFM_PROTECTED | CFM_HIDDEN);
      pCharFormat2->dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      pCharFormat2->dwEffects &= ~CFE_HIDDEN;
      pDoc->DispStyle = DISP_PROTECTED;
      SetNormal( pDoc, L"\n", pCharFormat2, L"" );
      pDoc->DispStyle = (DISPSTYLE)usDispStyle;
      pCharFormat2->dwEffects |=  CFE_HIDDEN;
      SetHidden( pDoc, RTF_ENDINLINESTYLE, pCharFormat2, L"" );
    }
    else if ( sId == psTag[WYSIWYG_HTML_HEAD1_TAG] )
    {
      RTFInlineEscape( chBufferTemp, pTok->pDataStringW );
      swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp,"" );
      pCharFormat2->dwMask |= (CFM_PROTECTED | CFM_HIDDEN);
      pCharFormat2->dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      SetHidden( pDoc, RTF_ENDINLINESTYLE, pCharFormat2, L"" );
    }
    else if ( sId == psTag[WYSIWYG_HTML_EHEAD1_TAG] )
    {
      RTFInlineEscape( chBufferTemp, pTok->pDataStringW );
      swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp,"" );
      pCharFormat2->dwMask |= (CFM_PROTECTED | CFM_HIDDEN);
      pCharFormat2->dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      SetHidden( pDoc, RTF_ENDINLINESTYLE, pCharFormat2, L"" );
    }
    else
    {
      RTFInlineEscape( chBufferTemp, pTok->pDataStringW );
      swprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp,"" );
      pCharFormat2->dwMask |= (CFM_PROTECTED | CFM_HIDDEN);
      pCharFormat2->dwEffects |= (CFE_PROTECTED | CFE_HIDDEN);
      SetHidden( pDoc, chBuffer, pCharFormat2, L"" );
      SetHidden( pDoc, RTF_ENDINLINESTYLE, pCharFormat2, L"" );
    } /* endif */

    pTok->pDataStringW[pTok->usLength] = c;
    pTok++;
  } /* endwhile */


}


VOID  WYSIWYG_RTF_Display( PTBDOCUMENT pDoc, PSZ_W pData,
                           CHARFORMAT2 *pCharFormat2, PTBSEGMENT pSeg )
{
  PSZ_W pRTFFont   = EQFBSegColRTF( pCharFormat2, pSeg, PROTECTED_CHAR, 0,
                                  pDoc->DispStyle, pDoc->EQFBFlags.PostEdit, pDoc->pDispFileRTF);

  FillRTFSpecialFast( pDoc, pRTFFont, FALSE );
  FillRTFSpecialFast( pDoc, pData, FALSE );




//  RTFInlineEscape( chBufferTemp, pData );
//          sprintf( chBuffer, RTF_INLINESTYLE, chBufferTemp,"" );
//  SetHidden( pDoc, chBuffer, &CharFormat2, pRTFFont );
//   SetHidden( pDoc, RTF_ENDINLINESTYLE, &CharFormat2, pRTFFont );



}



VOID EQFBSetWYSIWYGType( PTBDOCUMENT pDoc, PSZ pTagTable )
{
  PUSHORT pusWYSIWYGType = &(pDoc->pDispFileRTF->WYSIWYGType);
  if ( strnicmp( pTagTable, "EQFHTML", 7 ) == 0 )
  {
    *pusWYSIWYGType = WYSIWYG_HTML;
  }
  else if ( strnicmp( pTagTable, "EQFRTF", 6 ) == 0 )
  {
    *pusWYSIWYGType = WYSIWYG_RTF;
  } /* endif */
}


SHORT PosEQF2RTF( PTBDOCUMENT pDoc, PSZ_W pEQFData, PSZ_W pRTFData, SHORT sPos )
{
  SHORT sNewPos = 0;
  pEQFData;
  switch ( pDoc->DispStyle )
  {
    case DISP_PROTECTED:
    case DISP_UNPROTECTED:
      sNewPos = sPos;
      break;
    default:
      sNewPos = GetPosInRestoredLine( pDoc, pRTFData, sPos );
      break;
  } /* endswitch */

  return sNewPos;
}



SHORT GetPosInRestoredLine( PTBDOCUMENT pDoc, PSZ_W pData, SHORT sPos )
{
  PTOKENENTRY pTok;
  PSZ_W       pRest = NULL;
  BOOL        fQFIPending = FALSE;
  USHORT      usStart = 0;
  LONG        lRTFStart = 0;
  LONG        lEQFStart = 0;
  BOOL        fNotEnd = TRUE;

  // have only to deal with a single segment... hence no buffer overflow, i.e. no pRest
  TATagTokenizeW( pData, (PLOADEDTABLE)pDoc->pDispFileRTF->pQFRTFTagTable,
                 TRUE, &pRest, &usStart,
                 (PTOKENENTRY) pDoc->pTokBuf,
                 TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

  pTok = (PTOKENENTRY) pDoc->pTokBuf;
  while ( (pTok->sTokenid != ENDOFLIST) && fNotEnd )
  {

    switch ( pTok->sTokenid )
    {
      case WHITESPACE:
        break;
      case TEXT_TOKEN:
        if ( !fQFIPending )
        {
          if ( lEQFStart + pTok->usLength < (USHORT) sPos)
          {
            lRTFStart += pTok->usLength;
            lEQFStart += pTok->usLength;
          }
          else
          {
            fNotEnd = FALSE;
          } /* endif */
        }
        else
        {
          lRTFStart += pTok->usLength;
        }
        break;

      default:
        switch ( pTok->usOrgId )
        {
           case RTF_QFI_TAG:
             fQFIPending = TRUE;
             lRTFStart += pTok->usLength;
             break;
           case RTF_EQFI_TAG:
             fQFIPending = FALSE;
             lRTFStart += pTok->usLength;
             break;
           case RTF_QFJ_TAG:
             lRTFStart += pTok->usLength;
             break;
           case RTF_T_TAG:
             lEQFStart += pTok->usLength-5;
             lRTFStart += pTok->usLength;
             if ( lEQFStart + pTok->usLength >= (USHORT) sPos)
             {
               fNotEnd = FALSE;
             } /* endif */
             break;
           case RTF_QFF_TAG:
           case RTF_N_TAG:
           case RTF_S_TAG:
           case RTF_L_TAG:
           case RTF_NONE_TAG:
           default:
             lRTFStart += pTok->usLength;
             break;
        } /* endswitch */
        break;
    } /* endswitch */
     pTok++;
  } /* endwhile */

  lRTFStart += (sPos- lEQFStart);

  return ((SHORT)lRTFStart);

}





SHORT PosRTF2EQF( PTBDOCUMENT pDoc, PSZ_W pRTFData, PSZ_W pEQFData, SHORT sPos )
{
  SHORT sNewPos = 0;
  pDoc; pRTFData; pEQFData;

  sNewPos = sPos;

  return sNewPos;
}


VOID SetHidden ( PTBDOCUMENT pDoc, PSZ_W pData, CHARFORMAT2 *pcfDefault, PSZ_W pRTFFont)
{ pcfDefault; pRTFFont;
  FillRTFSpecialFast( pDoc, L"\\v\\protect ", FALSE );
  FillRTFSpecialFast( pDoc, pData, TRUE );
  FillRTFSpecialFast( pDoc, L"\\v0\\protect0 ", FALSE );
  return;
}

VOID SetProtected
(  PTBDOCUMENT pDoc,
   PSZ_W       pData,
   CHARFORMAT2 *pcfDefault,
   PSZ_W       pRTFFont
)
{ pcfDefault;
  FillRTFSpecialFast( pDoc, pRTFFont, FALSE );
  FillRTFSpecialFast( pDoc, L"\\protect ", FALSE );
  FillRTFSpecialFast( pDoc, pData, TRUE );
  FillRTFSpecialFast( pDoc, L"\\protect0 ", FALSE );
  return;
}

VOID SetNormal(PTBDOCUMENT pDoc, PSZ_W pData, CHARFORMAT2 *pcfDefault, PSZ_W pRTFFont)
{
  pcfDefault;
  FillRTFSpecialFast( pDoc, pRTFFont, FALSE );
  FillRTFSpecialFast( pDoc, L" ", FALSE );
  FillRTFSpecialFast( pDoc, pData, TRUE );

  return;
}


VOID FillRTFSpecialFast( PTBDOCUMENT pDoc, PSZ_W p, BOOL fConvert )
{
  int len = (int) UTF16strlenCHAR( p );
  PDISPFILERTF pDisp = pDoc->pDispFileRTF;
  CHAR_W chBuffer[ MAX_SEGMENT_SIZE ];
  PSZ    pbBuff =    (PSZ)pDisp->pbBuff;
  PSZ_W  pszSource = &chBuffer[0];
  CHAR_W c;

  if (len > 0)
  {
      UTF16strcpy (pszSource, p );
  }

  *(pszSource+len) = EOS;

  while ( (c = *pszSource++) != NULC)
  {
    /******************************************************************/
    /* increase allocated buffer                                      */
    /******************************************************************/
    if ( pDisp->ulBufferAllocated < (ULONG)(pDisp->lPos + 50) )
    {
      ULONG ulLen = pDisp->ulBufferAllocated + RTFBUFFER_INC;
      UtlAlloc( (PVOID *)&(pDisp->pBufferOverflow),
                pDisp->ulBufferAllocated, ulLen, ERROR_STORAGE);
      if ( pDisp->pBufferOverflow )
      {
        pDisp->ulBufferAllocated = ulLen;
      } /* endif */
    } /* endif */


    switch ( c )
    {
      case '\n':
        if (pDoc->DispStyle != DISP_WYSIWYG)
        {
          FillRTFSpecialFast( pDoc, RTF_PARA, FALSE );
        } /* endif */
        break;

      case '\\':
      case '{':
      case '}':
        if ( fConvert )
        {
          if ( pDisp->lPoscb < pDisp->cb )
          {
            pbBuff[pDisp->lPoscb++] = '\\';
          }
          else
          {
            pDisp->pBufferOverflow[pDisp->lPos++] = '\\';
          } /* endif */
        } /* endif */

        if ( pDisp->lPoscb < pDisp->cb )
        {
          pbBuff[pDisp->lPoscb++] = (BYTE)c;
        }
        else
        {
          pDisp->pBufferOverflow[pDisp->lPos++] = (BYTE)c;
        } /* endif */
        break;

      default:
        if ( c < 128 )
        {
          if ( pDisp->lPoscb < pDisp->cb )
          {
            pbBuff[pDisp->lPoscb++] = (BYTE)c;
          }
          else
          {
            pDisp->pBufferOverflow[pDisp->lPos++] = (BYTE)c;
          } /* endif */
        }
        else
        {
          CHAR_W chTemp[30];
          SHORT  sVal = (SHORT)c;
          swprintf( chTemp, L"\\u%d \\\'%2.2x", sVal, (BYTE)c);
          FillRTFSpecialFast( pDoc, chTemp, FALSE );
        } /* endif */
        break;
    } /* endswitch */
  } /* endwhile */
}




/**********************************************************************/
/* MS Bug: Before a hidden area and in overtype mode, we have to      */
/* select the area (character) from right to left and than issue the  */
/* overtype mode                                                      */
/**********************************************************************/
VOID ReplaceCharBeforeHidden( PTBDOCUMENT pDoc, WPARAM mp1)
{
  CHAR_W chText[2];
  CHARRANGE chRange;

  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0L, (LONG)&chRange );
  chRange.cpMin++;


  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0L, (LONG)&chRange );
  chText[0] = (BYTE)LOWORD( mp1 ); chText[1] = 0;
  SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE,  (LONG) &chText[0] );
  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0L, (LONG)&chRange );
  chRange.cpMax = chRange.cpMin;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0L, (LONG)&chRange );

}

/**********************************************************************/
/* MS Bug: Before a hidden area and in insert mode, we have to        */
/* select the area (character) from right to left and than ?????????  */
/* ?????????????                                                      */
/**********************************************************************/
VOID InsertCharBeforeHidden( PTBDOCUMENT pDoc, WPARAM mp1)
{
  CHAR_W chText[3];
  CHARRANGE chRange;

  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0L, (LONG)&chRange );

  GetTextRangeRTF( pDoc, chText, 2, chRange.cpMin-1, 1);

  chRange.cpMax--;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0L, (LONG)&chRange );
  chText[1] = (BYTE)LOWORD( mp1 ); chText[2] = 0;
  SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE,  (LONG) &chText[0] );

}


BOOL InActSeg( PTBDOCUMENT pDoc, CHARRANGE *pchRange )
{
  BOOL fInActSeg = FALSE;
  ULONG ulActSeg;

  if ( pDoc->pDispFileRTF->fTBCursorReCalc )
  {
    EQFBUpdateTBCursor( pDoc );
  }

  ulActSeg = pDoc->TBCursor.ulSegNum;


  fInActSeg = ( ulActSeg == pDoc->ulWorkSeg );
  if ( !fInActSeg )
  {
    /************************************************************/
    /* check if we are at the beginning of a segment            */
    /************************************************************/
    if ( (ulActSeg == pDoc->ulWorkSeg-1) &&
          ((ULONG)pchRange->cpMax == pDoc->pDispFileRTF->RTFLine.ulSegOffs[ulActSeg]) )
    {
      fInActSeg = TRUE;
    } /* endif */
  } /* endif */
  return fInActSeg;
}



USHORT SpecialCare( PTBDOCUMENT pDoc)
{
  USHORT    usSpecial = 0;
  USHORT    usOffset;
  CHARRANGE chRange;
  PSZ_W     pText;
  CHAR_W    chText[ 2*RTF_OFFSET + 500 ];


  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG)&chRange );


  if ( chRange.cpMin == chRange.cpMax )
  {
    usOffset = (chRange.cpMin >= RTF_OFFSET) ? RTF_OFFSET : (USHORT)chRange.cpMin;

    GetTextRangeRTF( pDoc, chText, sizeof( chText ),
                     chRange.cpMin - usOffset, 2*usOffset );

    pText = &chText[ usOffset ];
    /******************************************************************/
    /* Logic:                                                         */
    /*    if (nextChar hidden)                                        */
    /*      if (nextChar == qff && fInActSeg )                        */
    /*         Toggle to Insert -- end of seg reached                 */
    /*      else if (nextChar == qff && !fInActSeg ) || prevChar prot */
    /*         position after hidden                                  */
    /*         usSpecial = PREV_HIDDEN                                */
    /*      else // (nextnextChar protected)                          */
    /*         do nothing -- let RTF Control handle it -- beep        */
    /*    else if (nextnextChar hidden)                               */
    /*      usSpecial = NEXTNEXT_HIDDEN;                              */
    /*    else if (prevChar hidden)                                   */
    /*      usSpecial = PREV_HIDDEN                                   */
    /******************************************************************/
    if ( *pText == '<' )
    {
      if ( memcmp( pText, L"<qff", 4 ) == 0 )
      {
        if ( InActSeg(pDoc, &chRange) )
        {
          /**************************************************************/
          /* toggle to insert -- if not yet done                        */
          /**************************************************************/
          if ( !pDoc->EQFBFlags.inserting )
          {
//            SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_INSERT, 0L );
            EQFBToggleInsertRTF( pDoc );
            pDoc->EQFBFlags.inserting = TRUE;
          } /* endif */
          usSpecial = RTF_NEXT_HIDDEN;
        }
        else
        {
          CHARFORMAT2 cfDefault;
          memset( &cfDefault, 0, sizeof(cfDefault) );
          cfDefault.cbSize = sizeof(cfDefault);
          /**************************************************************/
          /* position after hidden                                      */
          /**************************************************************/
          SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );
          SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
          usSpecial = RTF_PREV_HIDDEN;
          /************************************************************/
          /* check if next character following is protected, too...   */
          /************************************************************/
          SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, TRUE, (LONG) &cfDefault );
          if ( cfDefault.dwEffects & CFE_PROTECTED )
          {
            usSpecial = RTF_PREVHIDDEN_NEXTPROTECTED;
          } /* endif */
        } /* endif */
      }
      else
      if ( memcmp( pText, L"<qfi", 4 ) == 0 )
      {
        usSpecial = RTF_NEXT_HIDDEN;
      }
      else
      if ( memcmp( pText, L"</qfi", 5 ) == 0 )
      {
        /**************************************************************/
        /* position after hidden, but only if on /qfi                 */
        /**************************************************************/
        SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );
        SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
        usSpecial = RTF_PREV_HIDDEN;
//      }
//      else
//      {
//        /**************************************************************/
//        /* check if we are in protect mode...                         */
//        /**************************************************************/
//        CHARFORMAT2 cfDefault;
//        memset( &cfDefault, 0, sizeof(cfDefault) );
//        cfDefault.cbSize = sizeof(cfDefault);
//        SendMessage( pDoc->hwndRichEdit, EM_GETCHARFORMAT, TRUE, (LONG) &cfDefault );
//        if ( cfDefault.dwEffects & CFE_PROTECTED )
//        {
//          usSpecial = RTF_PREV_HIDDEN;
//        } /* endif */
      } /* endif */
    } /* endif */

    if ( usSpecial == 0 )
    {
      if ( (usOffset > 6) && (*(pText-1) == '>'))
      {
        BOOL fFound = (memcmp( pText-5, L"s=\"\">", 5 ) == 0);

        if ( !fFound )
        {
          fFound = (memcmp( pText-6, L"</qfi>", 6 ) == 0);
          if ( !fFound )
          {
            /********************************************************/
            /* check for end of qff sequence                        */
            /********************************************************/
            int i = 2;
            while ( !fFound )
            {
              if ( isdigit( (UCHAR)*(pText-i) ) )
              {
                i++;
              }
              else if ( *(pText-i) == '=' )
              {
                i++;
                fFound = ( *(pText-i) == 'l');
                break;
              }
              else
              {
                break;
              } /* endif */
            } /* endwhile */
          } /* endif */
        } /* endif */
        if ( fFound )
        {
          usSpecial = RTF_PREV_HIDDEN;
        } /* endif */
      } /* endif */

      if ( !usSpecial && (*(pText+1) == '<') )
      {
        if ( memcmp( pText+1, L"</qfi", 5 ) == 0 )
        {
          SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
          SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );
          usSpecial = RTF_NEXT_HIDDEN;
        }
        else
        if ( (memcmp( pText+1, L"<qff", 4 ) == 0) ||
             (memcmp( pText+1, L"<qfi", 4 ) == 0))
        {
          usSpecial = RTF_NEXTNEXT_HIDDEN;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  return usSpecial;
}

VOID ReplaceCharAfterHidden( PTBDOCUMENT pDoc, WPARAM mp1 )
{
  CHAR_W    chText[2];
  CHARRANGE chRange;

  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0L, (LONG)&chRange );
  chRange.cpMin++;


  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0L, (LONG)&chRange );

  SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );

  chText[0] = LOWORD( mp1 ); chText[1] = 0;
  SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE,  (LONG) &chText[0] );
  SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
  SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_BACK, 0L );
  SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );

}


VOID InsertCharAfterHidden( PTBDOCUMENT pDoc, WPARAM mp1, BOOL fProtectFollow )
{
  CHAR_W    chText[4];
  CHARRANGE chRange;
  memset(&chText[0], 0, sizeof( chText ));
  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG)&chRange );

  if ( fProtectFollow )
  {
    chText[0] = LOWORD(mp1);
    chText[1] = BLANK;
    chText[2] = EOS;
    SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) &chText[0]);
    chRange.cpMin++;
    chRange.cpMax = chRange.cpMin + 1;
    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
    SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) "");
  }
  else
  {
    GetTextRangeRTF( pDoc, chText, sizeof(chText), chRange.cpMin, 1 );
//    TextRange.lpstrText = &chText[1];
    chText[0] = (CHAR) LOWORD(mp1);
    chText[1] = pDoc->pDispFileRTF->RTFLine.chText[0];
    chRange.cpMax++;
    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
    SendMessage( pDoc->hwndRichEdit, EM_REPLACESEL, TRUE, (LONG) &chText[0]);
    SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_LEFT, 0L );
  } /* endif */
}


VOID EQFBFuncRightRTF( PTBDOCUMENT pDoc )
{
  CHARRANGE chRange;
  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
  chRange.cpMin++; chRange.cpMax++;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
  pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
  EQFBUpdateTBCursor( pDoc );
}


VOID EQFBFuncLeftRTF( PTBDOCUMENT pDoc )
{
  CHARRANGE chRange;
  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
  if ( chRange.cpMin > 0 )
  {
    chRange.cpMin--; chRange.cpMax--;
    SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
    pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
    EQFBUpdateTBCursor( pDoc );
  } /* endif */
}


/**********************************************************************/
/* Move cursor to End of current line                                 */
/**********************************************************************/
VOID EQFBFuncEndLineRTF( PTBDOCUMENT pDoc )
{
  LONG lEventMask = SendMessage(pDoc->hwndRichEdit, EM_GETEVENTMASK, 0L, 0L );

  SendMessage(pDoc->hwndRichEdit, EM_SETEVENTMASK, 0, 0L );
  SendMessage(pDoc->hwndRichEdit, WM_KEYDOWN, VK_END, 0L );
  SendMessage(pDoc->hwndRichEdit, EM_SETEVENTMASK, 0, lEventMask );

  pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
  EQFBUpdateTBCursor( pDoc );

  EQFBWorkSegCheck( pDoc );            //check if segment changed

}

/**********************************************************************/
/* Move cursor to Start of current line                               */
/**********************************************************************/
VOID EQFBFuncStartLineRTF( PTBDOCUMENT pDoc )
{
  CHARRANGE chRange;
  BYTE   b = pDoc->pDispFileRTF->bRTFFill;
  LONG lEventMask = SendMessage(pDoc->hwndRichEdit, EM_GETEVENTMASK, 0L, 0L );

  pDoc->pDispFileRTF->bRTFFill = RTF_FILL;
  SendMessage(pDoc->hwndRichEdit, EM_SETEVENTMASK, 0, 0L );
  SendMessage(pDoc->hwndRichEdit, WM_KEYDOWN, VK_HOME, 0L );
  SendMessage(pDoc->hwndRichEdit, EM_SETEVENTMASK, 0, lEventMask );

  // get rid of any selection and let cursor be at the beginning ...
  SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
  chRange.cpMax = chRange.cpMin;
  SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
  pDoc->pDispFileRTF->bRTFFill = b;
  // update TBCursor
  pDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
  EQFBUpdateTBCursor( pDoc );

  EQFBWorkSegCheck( pDoc );            //check if segment changed

}

/**********************************************************************/
/* Backspace                                                          */
/**********************************************************************/
VOID EQFBFuncBackspaceRTF( PTBDOCUMENT pDoc )
{
   CHARRANGE chRange;
   BOOL      fInActSeg = TRUE;

   if ( pDoc->EQFBFlags.workchng && pDoc->ulWorkSeg )
   {
     EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );
   } /* endif */

   /****************************************************************/
   /* ensure that correct segment is loaded                        */
   /****************************************************************/
   if ( pDoc->EQFBFlags.PostEdit )
   {
     EQFBWorkSegCheck( pDoc );
   } /* endif */

//   if ( pDoc->pDispFileRTF->fTBCursorReCalc )
//   {
     EQFBUpdateTBCursor( pDoc );
//   }

   // Delete selection or character
   SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
   EQFBGetSegFromCaretRTF( pDoc, &pDoc->TBCursor, chRange.cpMax );

   if (!pDoc->EQFBFlags.PostEdit )
   {
	   if (pDoc->TBCursor.ulSegNum != pDoc->ulWorkSeg)
	   {
		 fInActSeg = FALSE;
		 }
   }
   else
   {
	   if (pDoc->pTBSeg->qStatus != QF_XLATED && (pDoc->pTBSeg->qStatus != QF_CURRENT))
	   {
		   fInActSeg = FALSE;
       }
   }

   if (fInActSeg && !EQFBFuncDelSel( pDoc ))
   {
     if ( pDoc->TBCursor.usSegOffset > 0 )
     {
       switch ( EQFBCharType(pDoc,pDoc->pTBSeg,(USHORT)(pDoc->TBCursor.usSegOffset-1) ))
       {
         case UNPROTECTED_CHAR:
         case LINEBREAK_CHAR:
         {
           PSZ_W pData = &pDoc->pEQFBWorkSegmentW[pDoc->TBCursor.usSegOffset-1];
           ULONG ulLen = UTF16strlenBYTE( pData );
           memmove( pData, pData+1, ulLen );
           pDoc->TBCursor.usSegOffset--;
           EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pDoc->pEQFBWorkSegmentW );
           STATUSBARRTF( pDoc );
           EQFBFuncLeftRTF( pDoc );
           break;
         }
         default:
           MessageBeep(0);
           break;
       }
     }
     else
     {
       MessageBeep(0);
     }
   }
}

VOID EQFBFuncDeleteRTF( PTBDOCUMENT pDoc )
{
	CHARRANGE chRange;
    BOOL      fInActSeg = TRUE;

   if ( pDoc->EQFBFlags.workchng && pDoc->ulWorkSeg )
   {
     EQFBGetWorkSegRTF( pDoc, pDoc->ulWorkSeg );
   } /* endif */

   /****************************************************************/
   /* ensure that correct segment is loaded                        */
   /****************************************************************/
   if ( pDoc->EQFBFlags.PostEdit )
   {
     EQFBWorkSegCheck( pDoc );
   } /* endif */

 //  if ( pDoc->pDispFileRTF->fTBCursorReCalc )
 //  {
     EQFBUpdateTBCursor( pDoc );
 //  }

   SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
   EQFBGetSegFromCaretRTF( pDoc, &pDoc->TBCursor, chRange.cpMax );

   if (!pDoc->EQFBFlags.PostEdit )
   {
	   if (pDoc->TBCursor.ulSegNum != pDoc->ulWorkSeg)
	   {
	     fInActSeg = FALSE;
       }
   }
   else
   {
	   if (pDoc->pTBSeg->qStatus != QF_XLATED && (pDoc->pTBSeg->qStatus != QF_CURRENT))
	   {
		   fInActSeg = FALSE;
       }
   }
   // Delete selection or character
   if (fInActSeg && !EQFBFuncDelSel( pDoc ))
   {
	   switch ( EQFBCharType(pDoc,pDoc->pTBSeg,(USHORT)(pDoc->TBCursor.usSegOffset) ))
	   {
		 case UNPROTECTED_CHAR:
		 case LINEBREAK_CHAR:
		 {
		   PSZ_W pData = &pDoc->pEQFBWorkSegmentW[pDoc->TBCursor.usSegOffset];
		   ULONG ulLen = UTF16strlenBYTE( pData );
		   memmove( pData, pData+1, ulLen );

		   EQFBSetWorkSegRTF( pDoc, pDoc->ulWorkSeg, pDoc->pEQFBWorkSegmentW );
		   STATUSBARRTF( pDoc );
		   break;
		 }
		 default:
		   MessageBeep(0);
		   break;
	   }
   }
} /* end of EQFBFuncDeleteRTF*/


/**********************************************************************/
/* Toggle Insert key -- but do not send any notifications ...         */
/**********************************************************************/
VOID EQFBToggleInsertRTF( PTBDOCUMENT pDoc )
{
  LONG lEventMask = SendMessage(pDoc->hwndRichEdit, EM_GETEVENTMASK, 0L, 0L );

  SendMessage(pDoc->hwndRichEdit, EM_SETEVENTMASK, 0, 0L );
  SendMessage(pDoc->hwndRichEdit, WM_KEYDOWN, VK_INSERT, 0L );

  SendMessage(pDoc->hwndRichEdit, EM_SETEVENTMASK, 0, lEventMask );
}



VOID EQFBReplaceSelRTF( PTBDOCUMENT pDoc, PSZ_W pTemp )
{
    SETTEXTEX SetTextEx;
    CHARRANGE chRange;

    memset( &SetTextEx, 0, sizeof( SetTextEx ));
    SetTextEx.flags = ST_KEEPUNDO | ST_SELECTION;
    SetTextEx.codepage = 1200; // use Unicode cp
RTFDEBUG( pDoc, L"ReplaceSel vor" );
    SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
    if ( chRange.cpMax > chRange.cpMin )
    {
      chRange.cpMax--;
      SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
RTFDumpSel( pDoc, "ReplaceSel:" );
      SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG)pTemp );
RTFDumpNoSel( pDoc, "nach ReplaceSel:" );
//      SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_RIGHT, 0L );
//      SendMessage( pDoc->hwndRichEdit, WM_KEYDOWN, VK_BACK, 0L );
      // Trick cpMin must be larger than cpMax
      SendMessage( pDoc->hwndRichEdit, EM_EXGETSEL, 0, (LONG) &chRange );
      chRange.cpMin ++;
      SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
      SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG) L"" );



RTFDumpNoSel( pDoc, "nach ReplaceSel 2:" );

    }
    else
    {
      // nothing selected -- insert character first, select it, do replacement..

      switch ( SpecialCare( pDoc ) )
      {
         case RTF_NEXT_HIDDEN:
           SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG) L" " );

           chRange.cpMax++;
           SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG) &chRange );
           SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG)pTemp );
           break;
         case RTF_NEXTNEXT_HIDDEN:
           SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG)pTemp );
           break;
         case RTF_PREV_HIDDEN:
           if ( pDoc->EQFBFlags.inserting )
           {
             CHAR_W    chText[4];
             ULONG     ulLen = UTF16strlenCHAR( pTemp );
             PSZ_W     pMyString = NULL;

             memset(&chText[0], 0, sizeof( chText ));
             GetTextRangeRTF( pDoc, chText, sizeof(chText), chRange.cpMin, 1 );


             UtlAlloc( (PVOID *)&pMyString, 0L, (ulLen + 2)*sizeof(CHAR_W), ERROR_STORAGE );

             if ( pMyString )
             {
               UTF16strcpy( pMyString, pTemp );
               UTF16strcat( pMyString, chText );
               chRange.cpMax++;
               SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
               SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG)pMyString );
               chRange.cpMax +=  (ulLen-1);
               chRange.cpMin = chRange.cpMax;
               SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
               UtlAlloc( (PVOID *)&pMyString, 0L, 0L, NOMSG );
             }
           }
           else
           {
             CHAR_W    chText[4];
             ULONG     ulLen = UTF16strlenCHAR( pTemp );

             memset(&chText[0], 0, sizeof( chText ));
             GetTextRangeRTF( pDoc, chText, sizeof(chText), chRange.cpMin, 1 );

             chRange.cpMax++;
             SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
             SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG)pTemp );

             chRange.cpMin += ulLen + 1;

             chRange.cpMax += ulLen - 1;

             ulLen--;
             while ( ulLen-- > 0)
             {
               USHORT us =  SpecialCare( pDoc );
               SendMessage( pDoc->hwndRichEdit, EM_EXSETSEL, 0, (LONG)&chRange );
               SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG) L"" );

               if (us == RTF_NEXTNEXT_HIDDEN)
               {
                 // switch to insert if in front of a hidden...
                 EQFBToggleInsertRTF( pDoc );
                 pDoc->EQFBFlags.inserting = TRUE;
               }
             }
           } /* endif */

           break;

         case RTF_PREVHIDDEN_NEXTPROTECTED:
            SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG)pTemp );
            break;
         default :
           SendMessage( pDoc->hwndRichEdit, EM_SETTEXTEX, (LONG)&SetTextEx, (LONG)pTemp );
           break;
      } /* endswitch */
    }
RTFDEBUG( pDoc, L"ReplaceSel nach" );
}

// Do handling for a WM_CHAR character key
MRESULT RichEditHandleWM_CHAR
(
    HWND             hwnd,
    PTBDOCUMENT      pDoc,
    WINMSG           msg,
    WPARAM           mp1,
    LPARAM           mp2
 )
{
    USHORT usFunction;
    LONG   mResult = FALSE;

    if ( EQFBMapKey( msg, mp1, mp2, &usFunction, &pDoc->ucState , TPRO_MAPKEY) )
    {
        pDoc->usChar      = (USHORT)mp1;
        pDoc->usDBCS2Char = 0;
        if ((pDoc->ucState & ST_CTRL) && (pDoc->ucState & ST_SHIFT)
               && ('A' <= pDoc->usChar) && (pDoc->usChar <= 'Z') )
        {
            pDoc->usChar = pDoc->usChar + 'a' - 'A';
        } /* endif */
        mResult = EQFBFuncRTFFunc( pDoc, usFunction, hwnd, mp1, mp2 );
    }
    else
    {
        mResult = DEFWINDOWPROC( GETPARENT(hwnd), msg, mp1, mp2 );
        mResult = TRUE;
    }

    return mResult;
 }

