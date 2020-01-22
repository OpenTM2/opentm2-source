/*! \file
	Description: This module contains a set of routines which interface to the display-system.
	It uses standard VIO-Calls to process the screen.

	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file
#include <eqfdoc00.h>

#define DISPLAY_DIRECT  16        // display next xx characters

CHAR_W BLANKLine[256];

WINGDIAPI BOOL  WINAPI GetTextExtentExPointI(HDC,LPWORD,int,int,LPINT,LPINT,LPSIZE);

VOID AdjustCharacterPlacement( PTBDOCUMENT pDoc, ULONG ulRow, ULONG ulCol, HDC hdc,
                               PSZ_W pText, ULONG ulLen, LONG lOffset);
VOID AdjustCharacterPlacementUnicode( PTBDOCUMENT pDoc, ULONG ulRow, ULONG ulCol, HDC hdc, PSZ_W pText,
                                      ULONG  ulLen, LONG lOffset );

  extern ULONG NLSLanguage;

 static NEWVIOCURSORINFO SysCurShapes[MAXCURSOR];

#define BIDI_FLAGS( a ) ((a & FLI_MASK) | GCP_SYMSWAPOFF | GCP_CLASSIN)
  #define BIDI_OUT      (GCPCLASS_PREBOUNDRTL | GCPCLASS_POSTBOUNDRTL)
  CHAR_W chBidiText[ MAX_SEGMENT_SIZE ];
#define ID_BIDI_TIMER   990911   // date timer defined

VOID  BidiDisplayLine( PTBDOCUMENT, PBIDISTRUCT, HDC, ULONG );
VOID  BidiDisplayLineNonRTL ( PTBDOCUMENT, PBIDISTRUCT, HDC, ULONG);
void CALLBACK TimerDisp ( HWND  hwnd, UINT uiMsg, UINT uiEvent, DWORD dwTime );
void CALLBACK TimerDispDev ( HWND  hwnd, UINT uiMsg, UINT uiEvent, DWORD dwTime );
void BidiDisplayLineEx ( PTBDOCUMENT, ULONG );
static VOID AdjustForBidi ( PSZ_W pText, ULONG ulDisplayRowLen );


static USHORT usInStyleTgt = 4;
static USHORT usOutStyleTgt = 11;

static USHORT usInStyleSrc = 4;
static USHORT usOutStyleSrc = 5;

static USHORT usInStyleProp = 4;
static USHORT usOutStyleProp =10;

 void EQFBSysScrnInit( PTBDOCUMENT);   // get initial sceen data
 VOID EQFBSysAttrib( COLOUR Color, PBYTE pCEll, BOOL fMark );
 VOID CleanDisplayString( PTBDOCUMENT pDoc, HDC hDC, PSZ_W pBidiText );

/**********************************************************************/
/* Load and free special bidi variables                               */
/**********************************************************************/

VOID InitBIDIVars( )
{
  ULONG ulCP = GetACP();
   //Initialize NlsLanguage to LATIN
   switch ( ulCP )
   {
     case 1255:  // Hebrew
       NLSLanguage = NLS_PROCESS_HEBREW | NLS_PROCESS_BIDI;
       break;
     case 1256:  // Arabic
       NLSLanguage = NLS_PROCESS_ARABIC | NLS_PROCESS_BIDI;
       break;
     default:
       NLSLanguage = NLS_PROCESS_LATIN;
       break;
   } /* endswitch */
}

VOID ClearBidi( )
{
}

//////////////////////////////////////////////////////////////////////
// SysCurShapes - an array of data needed to set the cursor to one
//                of three different shapes.  This array is indexed
//                by the CURSOR_XXXX values.  The actual values of
//                the elements of the array are filled in by the
//                SysScrnInit() routine, based on the current
//                display type.
//////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Function name:     EQFBSysScrnInit() - the screen start-up routine
// Internal function
//------------------------------------------------------------------------------
// Function call:     EQFBSysScrnInit( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description  :     this function will pick up the various essential
//                    details about the current screen
//
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT     pointer to document instance data
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Side effects:      A static structure with the cursor shape will be set
//
//------------------------------------------------------------------------------
// Function flow:     Determine the following details and store them:
//                    - use device cell size ( VioGetDeviceCellSize )
//                      to set cursor shape ( insert and replace )
//
//------------------------------------------------------------------------------
 void EQFBSysScrnInit
 (
   PTBDOCUMENT  pDoc                         // pointer to document ida
 )
 {

     LONG lH, lW;

     /* Set the cursor shape information */
     // defaults for the moment
     lW = pDoc->cx;
     lH = pDoc->cy;

     /*****************************************************************/
     /* only cx for width and cEnd for heigth will be filled....      */
     /*****************************************************************/
     memset( &SysCurShapes, 0, sizeof( SysCurShapes ));
     SysCurShapes[CURSOR_INSERT].cEnd    = (USHORT)lH; // sH/2 + 1;
     SysCurShapes[CURSOR_INSERT].cx      = 1;

     SysCurShapes[CURSOR_REPLACE].cEnd   = (USHORT)lH; // sH / 4 + 1;
     SysCurShapes[CURSOR_REPLACE].cx     = (USHORT)lW;

     SysCurShapes[CURSOR_SEGMENT].cEnd   = (USHORT)lH;
     SysCurShapes[CURSOR_SEGMENT].cx     = (USHORT)lW;  // 1; //  sW;

//     memset(&BLANKLine, BLANK, sizeof(BLANKLine) );
     {
       int i, iMax = sizeof(BLANKLine)/sizeof(CHAR_W);
       for (i=0; i< iMax; i++)
         BLANKLine[i] = BLANK;
       BLANKLine[iMax-1] = EOS;
     }

     // store info in pDoc struct
     memcpy( pDoc->vioCurShapes, SysCurShapes, sizeof( SysCurShapes ));
     EQFBSysScrnCurShape ( pDoc, (CURSOR)pDoc->usCursorType );     // set default cursor


   return;
 }

VOID EQFBBidiDispReorder
(
   PTBDOCUMENT  pDoc,                  // pointer to document ida
   ULONG        ulRow,                 // row
   ULONG        ulCol,                 // column
   PSZ_W        pText,                 // text to be displayed
   ULONG        ulLen,                 // length of text
   COLOUR       Colour,                // color to be displayed
   BYTE         bMark,                 // mark active
   PSZ_W        pTextCompl,            // complete text
   HDC          hdc
)
{
  PBIDISTRUCT pBidiStruct = pDoc->pBidiStruct;
  if ( !pBidiStruct )
  {
    UtlAlloc( (PVOID *)&pDoc->pBidiStruct,
              0L, sizeof( BIDISTRUCT ), ERROR_STORAGE );
    pBidiStruct = pDoc->pBidiStruct;
    if ( pBidiStruct )
    {
      pBidiStruct->pPool = PoolCreate( 4000 );
    } /* endif */
  } /* endif */

  EQFBBidiLRSwap( TRUE );
  /***************************************************************/
  /* determine display mode                                      */
  /***************************************************************/
  if ( ((pDoc->docType == STARGET_DOC) && IS_RTL_TGT( pDoc )) ||
       ( IS_RTL_SRC( pDoc ) && (pDoc->docType != STARGET_DOC))  )
  {
    SetTextAlign( hdc, TA_RTLREADING );
    pBidiStruct->fRTL = TRUE;
  } /* endif */

  /***************************************************************/
  /* prepare current row in buffer                               */
  /***************************************************************/
  if ( pBidiStruct->fRTL && !pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay)
  {
    if ( !pBidiStruct->usTimerID )
    {
      pBidiStruct->usTimerID = (USHORT)SetTimer( pDoc->hwndClient, ID_BIDI_TIMER, 100,
                                         (TIMERPROC) TimerDisp );
    } /* endif */
    if ( pBidiStruct->bOutputPending &&
         (ulRow != pBidiStruct->ulDisplayRow) )
    {
      BidiDisplayLine( pDoc, pBidiStruct, hdc, pBidiStruct->ulDisplayRow );
    } /* endif */

    pBidiStruct->ulDisplayRow   = ulRow;

    /*************************************************************/
    /* prepare display of row                                    */
    /*************************************************************/
    EQFBCheckBidiPart( pDoc, pTextCompl, ulLen, ulCol, Colour, bMark, ulRow );

  }
  else
  {
    /***************************************************************/
    /* toggle text according to the selected Bidi ordering...      */
    /***************************************************************/
//   pBidiStruct->usCCSIdOut  = pBidiStruct->usCCSIdIn   = GetACP();
    pBidiStruct->usCCSIdOut  = pBidiStruct->usCCSIdIn   = (USHORT)pDoc->ulAnsiCodePage;
    switch ( pDoc->docType )
    {
      case SERVPROP_DOC:      // proposal window
        if ( !pBidiStruct->usTimerID )
        {
          pBidiStruct->usTimerID = (USHORT)SetTimer( pDoc->hwndClient, ID_BIDI_TIMER, 100,
                                             (TIMERPROC) TimerDispDev );
        } /* endif */
        if ( pBidiStruct->bOutputPending &&
             (ulRow != pBidiStruct->ulDisplayRow) )
        {
          BidiDisplayLine( pDoc, pBidiStruct, hdc, pBidiStruct->ulDisplayRow );
        } /* endif */

        pBidiStruct->ulDisplayRow   = ulRow;

        /*************************************************************/
        /* prepare display of row                                    */
        /*************************************************************/
        EQFBCheckBidiPart( pDoc, pTextCompl, ulLen, ulCol, Colour, bMark, ulRow );

        break;
      case STARGET_DOC:
      case SSOURCE_DOC:
      case SERVDICT_DOC:      // dicitonary window
      default:
        TextOutW(hdc,ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                 pText, ulLen );
        break;
    } /* endswitch */
  } /* endif */
}
//------------------------------------------------------------------------------
// Function name:     EQFBSysScrnText  - write some text to the screen
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBSysScrnText(PTBDOCUMENT, USHORT, USHORT, PSZ, USHORT,
//                                    COLOUR, BOOL );
//------------------------------------------------------------------------------
// Description  :     this funcion will write the passed string at the
//                    requested position
//
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida
//                    USHORT        - row to start writing
//                    USHORT        - column to start writing
//                    PSZ           - text to write
//                    USHORT        - length of text
//                    COLOUR        - colour to use
//                    BOOL          - is marking active ?
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     Use the passed parameters and pass it to a
//                    call to a VIO call ( VioWrtCharStrAtt )
//
//------------------------------------------------------------------------------
 void EQFBSysScrnText
 (
   PTBDOCUMENT  pDoc,                  // pointer to document ida
   ULONG        ulRow,                 // row
   ULONG        ulCol,                 // column
   PSZ_W        pText,                 // text to be displayed
   ULONG        ulLen,                 // length of text
   COLOUR       Colour,                // color to be displayed
   BYTE         bMark,                 // mark active
   PSZ_W        pTextCompl,            // complete text
   PULONG       pulLenDisplayed        // length of displayed text in Non-Unicode
                                         // DBCS systems
 )
 {
   HDC      hdc;
   COLORREF clrBkRGB;
   COLORREF clrFGRGB;
   HFONT hFont;
   BOOL     fFocus = FALSE;
   BOOL     fMark = (bMark & DISP_MARK);
   ULONG    ulLenDisplayed = ulLen;
   TEXTTYPETABLE TextType;

   // if color is one of our hard-coded colors use hard-coded values...
   memset( &TextType, 0, sizeof(TextType) );
   if ( Colour == COLOUR_DICTSTYLEPREF )
   {
     TextType.sBGColor = COL_LIGHTGREEN;
     TextType.sFGColor = COL_BLACK;
   }
   else if ( Colour == COLOUR_DICTSTYLENOT )
   {
     TextType.sBGColor = COL_LIGHTRED;
     TextType.sFGColor = COL_BLACK;
   }
   else
   {
     memcpy( &TextType, get_TextTypeTable() + Colour, sizeof(TextType) );
   } /* endif */

   if (!UtlIsHighContrast())
   {
	   if ( (TextType.fReverse && !fMark) || (fMark && !TextType.fReverse))
	   {
		 clrFGRGB =   COLORRGBTABLE[ TextType.sBGColor];
		 clrBkRGB =   COLORRGBTABLE[ TextType.sFGColor];
	   }
	   else
	   {
		 clrFGRGB =   COLORRGBTABLE[ TextType.sFGColor];
		 clrBkRGB =   COLORRGBTABLE[ TextType.sBGColor];
	   } /* endif */
   }
   else
   {
	   switch (Colour)
	   {
		   case (COLOUR_VISACT):
		   case (COLOUR_ACTIVE):  // Black high contrast: white on lila bg
		     clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_HIGHLIGHTTEXT));
		     clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_HIGHLIGHT));
		   break;
		   case (COLOUR_P_ACTIVE): // Black high contrast: white on green bg
		     clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_INACTIVECAPTIONTEXT));
		     clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_INACTIVECAPTION));
		   break;
		   case (COLOUR_UNMATCHTAG):
		     clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_GRAYTEXT));
		     clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_INACTIVECAPTION));
		   break;
		   case (COLOUR_P_XLATED):
		   case (COLOUR_P_NOP):
		   case (COLOUR_P_TOBE):         // Black high contrast: white on black bg
		      clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_BTNTEXT));
		      clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_BTNFACE));
		   break;
		   case (COLOUR_CROSSED_OUT):        // crossed out by user
		   case (COLOUR_VALID_01):
		   case (COLOUR_VALID_10 ):    // green on black background
		       clrFGRGB = UtlGetColorref(GetSysColor(COLOR_INACTIVECAPTION));
		       clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_WINDOW));
		   break;
		   case (COLOUR_SRV_PROPSRCINS): // proposal source inserted text
		   case (COLOUR_SRV_PROPSRCDEL):
		     clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_INACTIVECAPTIONTEXT));
		     clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_INACTIVECAPTION));
		      break;
		   case (COLOUR_SRV_PROPSRCUNEQU):    // white on lila
              clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_HIGHLIGHTTEXT));
		      clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_HIGHLIGHT));
		      break;
		   case (COLOUR_TMODPROPOSAL):              // modified proposal is translation
		      clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_INACTIVECAPTIONTEXT));
		     clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_INACTIVECAPTION));
		     break;
           case (COLOUR_TCOPYPROPOSAL):              // proposal copy is translation
             clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_HIGHLIGHTTEXT));
		     clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_HIGHLIGHT));
		     break;
		   default:       // Black high contrast: white on black bg
		    clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_WINDOWTEXT));
		    clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_WINDOW));
		    break;
	  }
	  if ( fMark )
	  {
		  COLORREF clrTemp = clrFGRGB;
		  clrFGRGB = clrBkRGB;
		  clrBkRGB = clrTemp;
      }
   }
   /*****************************************************************/
   /* get the caret position and hide it (if we have the focus)     */
   /*****************************************************************/
   if ( pDoc->hwndClient == GetFocus() )
   {
     HideCaret( pDoc->hwndClient );
     fFocus = TRUE;
   } /* endif */
   hdc = GetDC(pDoc->hwndClient);

   pDoc->lf.lfWeight = FW_NORMAL;
   pDoc->lf.lfItalic = 0;
   pDoc->lf.lfUnderline = (BYTE) TextType.fUnderscore;

   // do a special handling for Win2k systems .. DBCS fonts are not recognized and treated correctly if SHIFTJIS_CHARSET is set
   if (IsDBCS_CP(pDoc->ulOemCodePage) && (pDoc->bOperatingSystem != OP_WINDOWSNT))
   {
      UCHAR lfCharSet = pDoc->lf.lfCharSet;
      pDoc->lf.lfCharSet &=  ~SHIFTJIS_CHARSET;
      hFont = CreateFontIndirect( &pDoc->lf );
      pDoc->lf.lfCharSet = lfCharSet;
  }
  else
  {
     hFont = CreateFontIndirect( &pDoc->lf );
  }


   hFont = (HFONT)SelectObject( hdc, hFont );

   SetTextColor(hdc, clrFGRGB);
   SetBkColor(hdc,clrBkRGB);
   /*****************************************************************/
   /* support BIDI stuff                                            */
   /*****************************************************************/
   if ( ulRow == 0xFFFF )
   {
       // ignore request
   }
   else
   {
     // Convert any tabs into spaces
     {
       CHAR_W ch = pText[ulLen];
       pText[ulLen] = EOS;
       CleanDisplayString( pDoc, hdc, pText );
       pText[ulLen] = ch;
     }

     if (IS_RTL(pDoc))
     {
       if ( ulRow < 1000 )  // if more rows on the screen -- ignore them
       {
         if ( pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay )
         {
           LONG lOffset;
           CHAR_W c;
           ULONG ulIndex;
           /***************************************************************/
           /* determine display mode                                      */
           /***************************************************************/
           switch ( pDoc->docType )
           {
             case VISTGT_DOC:
             case STARGET_DOC:
               if ( IS_RTL_TGT( pDoc ) )
               {
                 SetTextAlign( hdc, TA_RIGHT | TA_RTLREADING );
                 EQFBBidiLRSwap( TRUE );
                 pDoc->fTARight = TRUE;
                 if ( !pDoc->pArabicStruct )
                 {
                   ReAllocArabicStruct( pDoc );
                 }
                 c = pText[ulLen]; pText[ulLen] = EOS;
                 ulIndex = ulLen;
                 {
					 ULONG ulTest = ulIndex * sizeof(CHAR_W);
					 ulTest = ulTest + 1;
			     }
                 memset( &chBidiText[0], 0, ulIndex * sizeof(CHAR_W));
                 UTF16strcpy(chBidiText, pText);
                 CleanDisplayString( pDoc, hdc, chBidiText );
                 UTF16strrev(&chBidiText[0]);

                 pText[ulLen] = c; chBidiText[ulIndex] = EOS;
                 lOffset = pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol];

                 AdjustCharacterPlacement( pDoc, ulRow, ulCol, hdc, chBidiText, ulLen, lOffset );

                 if ( bMark & DISP_MISSPELLED )
                 {
                   LONG lEnd = (pDoc->lScrnCols + 1) * (pDoc->cx) - lOffset -4;
                   LONG lX = (pDoc->lScrnCols + 1) * (pDoc->cx) - pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol+ulLen] -4;
                   //usEnd - pDoc->cx * ulLen -1;
                   LONG lY = ulRow * pDoc->cy + pDoc->ulRulerSize;
                   LONG lI;

                   MoveToEx( hdc, lX, lY+ pDoc->cy-3, NULL );

                   DeleteObject( SelectObject( hdc, hFont ) );
                   hFont = (HFONT)SelectObject(hdc, CreatePen( PS_SOLID, 1, RGB(0xff,0,0)));

                   for ( lI=lX; lI < lEnd; lI += 4 )
                   {
                     LineTo( hdc, lI+2, lY + pDoc->cy -1 );
                     LineTo( hdc, lI+4, lY + pDoc->cy -3 );
                   } /* endfor */
                 }
               }
               else
               {
			     if ( !pDoc->pArabicStruct )
			     {
			    	 ReAllocArabicStruct( pDoc );
			     }
			     lOffset = pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol];
			     AdjustCharacterPlacementUnicode( pDoc, ulRow, ulCol, hdc, pText, ulLen, lOffset );
               } /* endif */
               break;
             case  VISSRC_DOC:
             case SSOURCE_DOC:
             case OTHER_DOC:
               if ( IS_RTL_SRC(pDoc) )
               {
                 SetTextAlign( hdc, TA_RIGHT | TA_RTLREADING );
                 EQFBBidiLRSwap( TRUE );
                 pDoc->fTARight = TRUE;
                 if ( !pDoc->pArabicStruct )
                 {
                   ReAllocArabicStruct( pDoc );
                 }

                 c = pText[ulLen]; pText[ulLen] = EOS;
                 ulIndex = ulLen;
                 memset( &chBidiText[0], 0, ulIndex);
                 UTF16strcpy(chBidiText, pText);
                 CleanDisplayString( pDoc, hdc, chBidiText );
                 UTF16strrev(&chBidiText[0]);
                 pText[ulLen] = c; chBidiText[ulIndex] = EOS;

                 lOffset = pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol];

                 AdjustCharacterPlacement( pDoc, ulRow, ulCol, hdc, chBidiText, ulLen, lOffset);

               }
               else
               {
                 pDoc->fTARight = FALSE;
                 TextOutW(hdc,ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                          pText, ulLen );
               } /* endif */
               break;
             case SERVPROP_DOC:
               if ( IS_RTL_TGT(pDoc) )
               {
//               int rc;
                 CHAR_W c;
                 ULONG ulIndex;
                 if ( bMark & DISP_NOREORDER )
                 {
                   SetTextAlign( hdc, TA_LEFT );
                   TextOutW(hdc,ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                            pText, ulLen );
                 }
                 else
                 {
                   SetTextAlign( hdc, TA_RIGHT | TA_RTLREADING );
                   EQFBBidiLRSwap( TRUE );
                   pDoc->fTARight = TRUE;

                   if ( !pDoc->pArabicStruct )
                   {
                     ReAllocArabicStruct( pDoc );
                   }

                   c = pText[ulLen]; pText[ulLen] = EOS;
                   ulIndex = ulLen;
                   memset( &chBidiText[0], 0, ulIndex);
                   UTF16strcpy(chBidiText, pText);
                   CleanDisplayString( pDoc, hdc, chBidiText );
                   UTF16strrev(&chBidiText[0]);
                   pText[ulLen] = c; chBidiText[ ulIndex ] = EOS;

                   lOffset = pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol];
                   AdjustCharacterPlacement( pDoc, ulRow, ulCol, hdc, chBidiText, ulLen, lOffset );
                 } /* endif */
               }
               else
               {
                 TextOutW(hdc,ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                          pText, ulLen );
               } /* endif */
               break;
             case SERVDICT_DOC:          // dicitonary window
               if ( IS_RTL_TGT(pDoc) )
               {
                 if ( bMark & DISP_NOREORDER )
                 {
                   SetTextAlign( hdc, TA_LEFT );
                   TextOutW(hdc,ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                            pText, ulLen );

                 }
                 else
                 {
                   GCP_RESULTSW gcpResults;
                   USHORT  wGlyph[256];
                   int FlagETO;
                   PSZ_W  pszDisp;

                   memset( &gcpResults, 0, sizeof( gcpResults ) );
                   gcpResults.lStructSize = sizeof( gcpResults );

                   memset( &wGlyph[0], 0, sizeof( wGlyph ));
                   gcpResults.lpGlyphs = (LPWSTR)&wGlyph[0];
                   //  ETO_NUMERICSLATIN  not to be used, because of problem in WinNT
                   FlagETO = ETO_OPAQUE | ETO_GLYPH_INDEX | ETO_CLIPPED;
                   pszDisp = (PSZ_W)wGlyph;
                   gcpResults.nGlyphs = min(MAX_SEGMENT_SIZE+1 - ulCol, 256);
                   gcpResults.nMaxFit = min(MAX_SEGMENT_SIZE+1 - ulCol, 256);
                   gcpResults.lpDx    = NULL;

                   SetTextAlign( hdc, TA_RTLREADING );
                   GetCharacterPlacementW( hdc, pText, ulLen, 0, &gcpResults, /*GCP_GLYPHSHAPE |*/ GCP_REORDER |GCP_LIGATE | GCP_DIACRITIC | GCP_DISPLAYZWG);
                   {
                     //Skip leading blank glyphs and move them to the end
                     ULONG  i = 0L;
                     ULONG  j = 0L;
                     PSZ_W p = pszDisp, q = pszDisp;
                     while ((*p == 0x0003) && (i< gcpResults.nGlyphs))
                     {
                       p++;
                       i++;
                     }
                     j = gcpResults.nGlyphs - i;
                     if ( (i > 0) && (j > 0))
                     {
                       while ( j-- > 0)
                       {
                         *q++ = *p++;
                       }
                       while ( i-- > 0)
                       {
                         *q++ = 0x0003;
                       }
                     }
                   }
                   ExtTextOutW( hdc, ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                                FlagETO, NULL,
                                pszDisp, gcpResults.nGlyphs, NULL  );
//                 TextOutW(hdc,ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
//                        pText, ulLen );
                 }
               }
               else
               {
                 TextOutW(hdc,ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                        pText, ulLen );
               }
               break;
             case SERVSOURCE_DOC:        // source window for proposals
             default:
               TextOutW(hdc,ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                        pText, ulLen );
               break;
           } /* endswitch */
         }
         else
         {
           EQFBBidiDispReorder( pDoc, ulRow, ulCol, pText, ulLen, Colour, bMark,
                                pTextCompl, hdc );
         } /* endif */
       }
     }
     else
     {
       LONG lOffset;
       if ( !pDoc->pArabicStruct )
       {
           ReAllocArabicStruct( pDoc );
       }
       lOffset = pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol];
       AdjustCharacterPlacementUnicode( pDoc, ulRow, ulCol, hdc, pText, ulLen, lOffset );

       if ( bMark & DISP_MISSPELLED )
       {
         ULONG ulX = ulCol * pDoc->cx;
         ULONG ulEnd = ulX + pDoc->cx * ulLen -1;
         ULONG ulY = ulRow * pDoc->cy + pDoc->ulRulerSize;
         ULONG ulI;

         MoveToEx( hdc, ulX, ulY+ pDoc->cy-3, NULL );

         DeleteObject( SelectObject( hdc, hFont ) );
         hFont = (HFONT)SelectObject(hdc, CreatePen( PS_SOLID, 1, RGB(0xff,0,0)));

         for ( ulI=ulX; ulI < ulEnd; ulI += 4 )
         {
           LineTo( hdc, ulI+2, ulY + pDoc->cy -1 );
           LineTo( hdc, ulI+4, ulY + pDoc->cy -3 );
         } /* endfor */
       } /* endif */
     } /* endif */
   } /* endif */
   DeleteObject( SelectObject( hdc, hFont ) );

   /*****************************************************************/
   /* show previously hidden caret  ....                            */
   /*****************************************************************/
   if ( fFocus )
   {
     ShowCaret( pDoc->hwndClient );
   } /* endif */
   ReleaseDC (pDoc->hwndClient, hdc);

   *pulLenDisplayed = ulLenDisplayed;
   return;
 }

//------------------------------------------------------------------------------
// Function name:     EQFBSysScrnChar() - write repeated char to the screen
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBSysScrnChar( PTBDOCUMENT, USHORT, USHORT, CHAR,
//                                     USHORT, COLOUR, BOOL );
//------------------------------------------------------------------------------
// Description  :      This function will write a repeated character to
//                     the presentation space passed via a VioWrtNCell
//                     function.
//
//------------------------------------------------------------------------------
// Parameter:          PTBDOCUMENT   - pointer to document ida
//                     USHORT        - row to start writing
//                     USHORT        - column to start writing
//                     CHAR          - character to use
//                     USHORT        - number of characters to write
//                     COLOUR        - colour to be used
//                     BOOL          - is marking active ?
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     Use the passed parameters and pass it to a
//                    call to a VIO call ( VioWrtNCell )
//
//------------------------------------------------------------------------------
 void EQFBSysScrnChar
 (
   PTBDOCUMENT  pDoc,                        // pointer to document ida
   ULONG        ulRow,                       // row to start writing
   ULONG        ulCol,                       // column to start writing
   LONG         lLen,                       // number of characters to write
   COLOUR       Colour,                      // colour to be used
   BYTE         bMark                        // mark active
 )
 {
   ULONG      ulLenInDBCSNonUnicode = 0L;
   BLANKLine[ lLen ] = EOS;
   EQFBSysScrnText (  pDoc,                  // pointer to document ida
                        ulRow,                 // row
                        ulCol,                 // column
                        &BLANKLine[0],         // text to be displayed
                        lLen,                 // length of text
                        Colour,                // color to be displayed
                        (BYTE)(bMark | DISP_SEGMENT_START),  // mark active
                        &BLANKLine[0],
                        &ulLenInDBCSNonUnicode);
   BLANKLine[ lLen ] = BLANK;
   return;
 }


VOID AdjustCharacterPlacementUnicode( PTBDOCUMENT pDoc, ULONG ulRow, ULONG ulCol, HDC hdc, PSZ_W pText,
                                      ULONG ulLen, LONG lOffset )
{
  GCP_RESULTSW gcpResults;
  PLONG plCaretPos = &(pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol]);
  SIZE size;
  int  lPos[256];
  USHORT  wGlyph[256];
  RECT rect;
  ULONG  i;
  int FlagETO;
  PSZ_W  pszDisp;

static CHAR_W szTextBuf[MAX_SEGMENT_SIZE];

  memset( &gcpResults, 0, sizeof( gcpResults ) );
  memset(&lPos[0], 0, sizeof( lPos ));
  gcpResults.lStructSize = sizeof( gcpResults );
  gcpResults.lpCaretPos = &lPos[0];


//GQTEst: replace required blanks by blanks before continuing
  {
    int i = 0;
    while ( i < (int)ulLen )
    {
      if ( pText[i] == 0x16 )
      {
        szTextBuf[i] = L' ';
      }
      else
      {
        szTextBuf[i] = pText[i];
      } /* endif */
      i++;
    } /*endwhile */
    pText = szTextBuf;
  }


  memset( &wGlyph[0], 0, sizeof( wGlyph ));
  gcpResults.lpGlyphs = (LPWSTR)&wGlyph[0];
     //  ETO_NUMERICSLATIN  not to be used, because of problem in WinNT
  FlagETO = ETO_OPAQUE | ETO_GLYPH_INDEX | ETO_CLIPPED;
  pszDisp = (PSZ_W)wGlyph;
  gcpResults.nGlyphs = min(MAX_SEGMENT_SIZE+1 - ulCol, 256);
  gcpResults.nMaxFit = min(MAX_SEGMENT_SIZE+1 - ulCol, 256);
  gcpResults.lpDx    = NULL;

  if (IsDBCS_CP(pDoc->ulOemCodePage)||
      (pDoc->twin && IsDBCS_CP(pDoc->twin->ulOemCodePage)))
  {
     GetCharacterPlacementW( hdc, pText, ulLen+1, 0, &gcpResults, GCP_DBCS );
      plCaretPos = &(pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol]);

      for (i=0; i < ulLen; i++)
      {
        plCaretPos[i] = lOffset + lPos[ i ];
      }
      plCaretPos[ulLen] = lPos[ulLen - 1] + lOffset + pDoc->cx;
     if ( !gcpResults.lpCaretPos[ulLen] )
     {
       size.cx = gcpResults.lpCaretPos[ ulLen -1 ] + pDoc->cx;
       plCaretPos[ulLen] = lPos[ulLen - 1] + lOffset + pDoc->cx;
     }
     else
     {
       size.cx = gcpResults.lpCaretPos[ ulLen ];
       plCaretPos[ulLen] = lPos[ulLen] + lOffset;
     }



      rect.left   =   lOffset;
      rect.top    =   ulRow * (pDoc->cy) + pDoc->ulRulerSize;
      rect.right  =   min( rect.left+size.cx, pDoc->lScrnCols * pDoc->cx);
      rect.bottom =   rect.top + pDoc->cy;

      ExtTextOutW( hdc, lOffset, ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                       FlagETO, &rect,
                       pszDisp ,  ulLen, NULL  );
  }
  else
  {
    GetCharacterPlacementW( hdc, pText, ulLen, 0, &gcpResults, /*GCP_GLYPHSHAPE |*/ GCP_LIGATE | GCP_DIACRITIC | GCP_DISPLAYZWG /*| GCP_SYMSWAPOFF*/);
    size.cx = gcpResults.lpCaretPos[ ulLen -1 ] + pDoc->cx;

    plCaretPos = &(pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol]);

    for (i=0; i < ulLen; i++)
    {
      plCaretPos[i] = lOffset + lPos[ i ];
    }
    plCaretPos[ulLen] = lPos[ulLen - 1] + lOffset + pDoc->cx;

    rect.left   =   lOffset;
    rect.top    =   ulRow * (pDoc->cy) + pDoc->ulRulerSize;

    rect.right  =   min( rect.left+size.cx, pDoc->lScrnCols * pDoc->cx);
    rect.bottom =   rect.top + pDoc->cy;

     ExtTextOutW( hdc, ulCol * (pDoc->cx), ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                 FlagETO, &rect,
                 pszDisp, gcpResults.nGlyphs, NULL  );
  }

}


VOID AdjustCharacterPlacement( PTBDOCUMENT pDoc, ULONG ulRow, ULONG ulCol, HDC hdc, PSZ_W pText,
                               ULONG ulLen, LONG lOffset )
{
  GCP_RESULTSW gcpResults;
  PLONG plCaretPos = &(pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol]);
  SIZE size;
  int  lPos[256];
  int  lpDx[256];
  USHORT  wGlyph[256];
  RECT rect;
  ULONG i;
  int FlagETO;
  PSZ_W  pszDisp;

  memset( &gcpResults, 0, sizeof( gcpResults ) );
  memset(&lPos[0], 0, sizeof( lPos ));
  memset(&lpDx[0], 0, sizeof( lpDx ));
  gcpResults.lStructSize = sizeof( gcpResults );
  gcpResults.lpCaretPos = &lPos[0];

  pszDisp = pText;
  memset( &wGlyph[0], 0, sizeof( wGlyph ));
  gcpResults.lpGlyphs = (LPWSTR)&wGlyph[0];
     //  ETO_NUMERICSLATIN  not to be used, because of problem in WinNT
  FlagETO = ETO_OPAQUE | ETO_GLYPH_INDEX;
  pszDisp = (PSZ_W)wGlyph;
  gcpResults.nGlyphs = MAX_SEGMENT_SIZE+1 - ulCol;
  gcpResults.nMaxFit = MAX_SEGMENT_SIZE+1 - ulCol;
  gcpResults.lpDx    = &lpDx[0];

  GetCharacterPlacementW( hdc, pText, ulLen, 0, &gcpResults, GCP_USEKERNING | GCP_LIGATE | GCP_DIACRITIC | GCP_DISPLAYZWG /*| GCP_SYMSWAPOFF*/);

  size.cx = gcpResults.lpCaretPos[ ulLen -1 ] + pDoc->cx;

  plCaretPos = &(pDoc->pArabicStruct->plCaretPos[ulRow*(pDoc->lScrnCols+1)+ulCol]);


  for (i=0; i < ulLen; i++)
  {
    plCaretPos[i] = lOffset + lPos[ulLen-1] - lPos[ulLen-1-i];
  }
  plCaretPos[ulLen] = lPos[ulLen - 1] + lOffset + pDoc->cx;


  rect.left   =   (pDoc->lScrnCols + 1) * (pDoc->cx) - lOffset - 4;
  rect.top    =   ulRow * (pDoc->cy) + pDoc->ulRulerSize;
  rect.right  =   (rect.left > size.cx) ? (rect.left - size.cx) : 0;
  rect.bottom =   rect.top + pDoc->cy;
  if (ulLen+ulCol == (ULONG)pDoc->lScrnCols)
  {
          rect.right = 0;
  }

  ExtTextOutW( hdc, (pDoc->lScrnCols + 1) * (pDoc->cx) - lOffset - 4, ulRow * (pDoc->cy) + pDoc->ulRulerSize,
              FlagETO, &rect,
              pszDisp, gcpResults.nGlyphs, &lpDx[0]);

}
// Allocate structure to handle Arabic specifics
VOID ReAllocArabicStruct( PTBDOCUMENT pDoc )
{
  EQFBGetUserSettings( pDoc );

  if (!pDoc->pArabicStruct)
  {
    UtlAlloc( (PVOID *)&pDoc->pArabicStruct, 0L, sizeof( ARABICSTRUCT ), ERROR_STORAGE );
  }
  if ( pDoc->pArabicStruct )
  {
    ULONG ul = ((ULONG)(pDoc->lScrnRows+1))*((ULONG)(pDoc->lScrnCols+1))*sizeof(ULONG);
    UtlAlloc( (PVOID *)&pDoc->pArabicStruct->plCaretPos, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *)&pDoc->pArabicStruct->plCaretPos, 0L, ul, ERROR_STORAGE );
  }
}

//------------------------------------------------------------------------------
// Function name:     EQFBSysScrnCurPos - position the cursor
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBSysScrnCurPos( PTBDOCUMENT, USHORT, USHORT );
//
//------------------------------------------------------------------------------
// Description  :     This routine positions the real cursor on the display.
//
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida
//                    USHORT        - cursor row
//                    USHORT        - cursor column
//
//------------------------------------------------------------------------------
// Side effects:      Physical cursor will be set
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     Use the passed parameters and pass it to a
//                    VIO call ( VioSetCurPos )
//
//------------------------------------------------------------------------------
 void EQFBSysScrnCurPos
 (
   PTBDOCUMENT  pDoc,                        // pointer to document ida
   LONG         lRow,                       // row
   LONG         lCol                        // column
 )
 {
   /*****************************************************************/
   /* set caret position if we have the focus...                    */
   /*****************************************************************/
   if ( pDoc->hwndClient == GetFocus() )
   {
     if ( IS_RTL(pDoc) )
     {
       if ( pDoc->pUserSettings->UserOptFlags.bBidiLogicDisplay )
       {
         if (pDoc->fTARight)
         {
           LONG lCaretPos  = pDoc->pArabicStruct->plCaretPos[lRow*(pDoc->lScrnCols+1)+lCol];
           SetCaretPos(pDoc->lScrnCols * pDoc->cx - lCaretPos - 4, lRow * (pDoc->cy)+ pDoc->ulRulerSize);
         }
         else
         {
           SetCaretPos(lCol * (pDoc->cx), lRow * (pDoc->cy)+ pDoc->ulRulerSize);
         }
       }
       else
       {
         EQFBBidiSysScrnCurPos ( pDoc, lRow, lCol );
       }
     }
     else
     {
       if (pDoc->pArabicStruct)
       {
         LONG lCaretPos  = pDoc->pArabicStruct->plCaretPos[lRow*(pDoc->lScrnCols+1)+lCol];
         SetCaretPos(lCaretPos, lRow * (pDoc->cy)+ pDoc->ulRulerSize);
       }
       else
       {
         SetCaretPos(lCol * (pDoc->cx), lRow * (pDoc->cy)+ pDoc->ulRulerSize);
       }
     } /* endif */


     if ( pDoc->hlfIME )
     {
       ImeMoveConvertWin(pDoc, pDoc->hwndClient,
                         (USHORT)(lCol * (pDoc->cx)),
                         (USHORT)(lRow * (pDoc->cy)+ pDoc->ulRulerSize));
     } /* endif */
   } /* endif */
   return;
 }


//------------------------------------------------------------------------------
// Function name:     EQFBSysScrnCurShape - set the cursor shape
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBSysScrnCurShape( PTBDOCUMENT, CURSOR );
//
//------------------------------------------------------------------------------
// Description  :     This routine adjusts the shape of the cursor to one
//                    of three sizes indexed by the CURSOR_XXXX values.
//                     CURSOR_REPLACE is used to indicate 'replace' mode
//                     CURSOR_INSERT  is used to indicate 'insert' mode
//
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida
//                    CURSOR        - the cursor shape to be set
//
//------------------------------------------------------------------------------
// Side effects:      Physical cursor shape will be set
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     Use the passed parameters and pass it to a
//                    VIO call ( VioSetCurPos )
//
//------------------------------------------------------------------------------
 void EQFBSysScrnCurShape
 (
   PTBDOCUMENT  pDoc,                        // pointer to document ida
   CURSOR shape
 )
 {
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

     /*****************************************************************/
     /* if shape of cursor has changed and window is active, change   */
     /* cursor shape on the display                                   */
     /* (must create a new caret - the old one will be destroyed      */
     /*  automatically)....                                           */
     /*****************************************************************/
     if (IS_RTL(pDoc))
     {
       /***************************************************************/
       /* do nothing                                                  */
       /***************************************************************/
       if (pDoc->hwndClient == GetFocus() )
       {
         HideCaret(pDoc->hwndClient);
         if ( pDoc->hCaretBitmap )
         {
                 DeleteObject( pDoc->hCaretBitmap );
                 pDoc->hCaretBitmap = NULL;
         }
         switch ( shape )
         {
           case CURSOR_SEGMENT:
             pDoc->hCaretBitmap = LoadImage( hResMod,
                                      MAKEINTRESOURCE( ID_BIDI_CARET_SEGMENT ),
                                      IMAGE_BITMAP,
                                      pDoc->cx,
                                      pDoc->cy,
                                      LR_DEFAULTCOLOR );

             break;
           case CURSOR_REPLACE:
             break;
           case CURSOR_INSERT:
           default:
             pDoc->hCaretBitmap = LoadImage( hResMod,
                                      MAKEINTRESOURCE( ID_BIDI_CARET_POP ),
                                      IMAGE_BITMAP,
                                      pDoc->cx,
                                      pDoc->cy,
                                      LR_DEFAULTCOLOR );
             break;
         }
         if ( pDoc->hCaretBitmap )
         {
           CreateCaret(pDoc->hwndClient, (HBITMAP) pDoc->hCaretBitmap,
                       pDoc->vioCurShapes[pDoc->usCursorType].cx,
                       pDoc->vioCurShapes[pDoc->usCursorType].cEnd);
           ShowCaret(pDoc->hwndClient);
         }
         else
         {
           CreateCaret(pDoc->hwndClient,
                       (HBITMAP)((shape == CURSOR_SEGMENT) ? 1:0),
                       pDoc->vioCurShapes[shape].cx,
                       pDoc->vioCurShapes[shape].cEnd);
           ShowCaret(pDoc->hwndClient);
     }
       } /* endif */
     }
     else
     {
       if (pDoc->hwndClient == GetFocus() )
       {
         CreateCaret(pDoc->hwndClient,
                     (HBITMAP)((shape == CURSOR_SEGMENT) ? 1:0),
                     pDoc->vioCurShapes[shape].cx,
                     pDoc->vioCurShapes[shape].cEnd);
         ShowCaret(pDoc->hwndClient);
       } /* endif */
     } /* endif */
   return;
 }

//------------------------------------------------------------------------------
// Function name:     EQFBSysFilename - standardise a file name
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBSysFilename( PSZ, PSZ );
//
//------------------------------------------------------------------------------
// Description  :     This routine converts a name into a form that will
//                    be unique to the operating system.
//                    Under OS/2 it expands the name to the full
//                    drive/path/filename form.
//
//------------------------------------------------------------------------------
// Parameter:         PSZ   - the expanded file name
//                            (must have a size of CCHMAXPATH)
//                    PSZ   - the base file file
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     - init return area
//                    - get current disk drive
//                    - add drive letter to filename if necessary
//                    - add current path to filename if necessary
//                    - uppercase filename
//------------------------------------------------------------------------------
VOID EQFBSysFilename( PSZ pszNewName, PSZ pszName)

{
   OFSTRUCT   OpenBuf;                // file structure


   OpenFile( pszName, &OpenBuf, OF_PARSE );

   strcpy( pszNewName, OpenBuf.szPathName);
  // set name to uppercase
   strupr( pszNewName );

}


//------------------------------------------------------------------------------
// Function name: EQFBSysInit
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBSysInit( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       do initalisation processing
//                    This routine performs all the special code
//                    needed to start-up the editor under OS/2.
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT     pointer to document area
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     call EQFBSysScrnInit
//
//------------------------------------------------------------------------------

 VOID EQFBSysInit
 (
    PTBDOCUMENT  pDoc                        // pointer to document ida
 )
 {

   /* Start-up the screen functions */
   EQFBSysScrnInit( pDoc );

   return;
 }

///////////////////////////////////////////////////////////////////////
// void EQFBSysExit(void) - close down the system interface
///////////////////////////////////////////////////////////////////////
// This routine would contain all the code needed
// to close down the // low level system interface.
// in our case it is a dummy for the moment
///////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Function name: EQFBSysExit
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBSysExit( PTBDOCUMENT );
//
//------------------------------------------------------------------------------
// Description:       This routine would contain all the code needed
//                    to close down the // low level system interface.
//                    in our case it is a dummy for the moment
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT     pointer to document area
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     dummy for now
//
//------------------------------------------------------------------------------
 VOID EQFBSysExit
 (
      PTBDOCUMENT  pDoc                      // pointer to document ida
 )
 {
   pDoc;                                     // get rid of compiler warnings
   return;
 }

//------------------------------------------------------------------------------
// Function name:     EQFBSysAttrib - build screen attribute cell
// Internal function
//------------------------------------------------------------------------------
// Function call:     EQFBSysAttrib( COLOUR, PBYTE, BOOL );
//
//------------------------------------------------------------------------------
// Description  :     this function will convert a COLOUR value to a screen
//                    attribute cell
//
//------------------------------------------------------------------------------
// Parameter:         COLOUR          the colour value
//                    PBYTE           pointer to the attribute cell
//                    BOOL            is reversing requested due to mark ??
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     - adress text type table
//                    - build colour attribute
//                    - build font and font flag attribute
//
//------------------------------------------------------------------------------
VOID EQFBSysAttrib
(
   COLOUR Color,                       // color value
   PBYTE pCell,                        // pointer to attribute cell
   BOOL fMark                          // mark active ?
)
{
    PTEXTTYPETABLE pTextType;          // ptr to active text type entry

    pTextType = get_TextTypeTable() + Color;
    pCell[0] = (BYTE) (pTextType->sFGColor | (pTextType->sBGColor << 4));
    pCell[1] = (BYTE) pTextType->usFont;
    if ( pTextType->fUnderscore )
    {
       pCell[1] |= 0x80;
    } /* endif */
    if ( (pTextType->fReverse && !fMark) || ( fMark && ! pTextType->fReverse ))
    {
       pCell[1] |= 0x40;
    } /* endif */
    pCell[2] = 0;
}


//------------------------------------------------------------------------------
// Function name:     EQFBSysDispRestOfLine - display rest of line
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBSysDispRestOfLine( PTBDOCUMENT, PSZ, BOOL );
//
//------------------------------------------------------------------------------
// Description  :     display rest of line directly to get good performance
//
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT       pointer to document instance data
//                    PSZ               pointer to data
//                    BOOL              display old contents
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     if not DBCS then
//                      if inserting or end of segment then
//                         read rest of line
//                      endif
//                      read color of current character
//                      display new character
//                      if inserting or end of segment then
//                         display rest of line moved one char to the right
//                      endif
//                    else
//                      force redraw of line
//                    endif
//------------------------------------------------------------------------------

VOID EQFBSysDispRestOfLine
(
   PTBDOCUMENT pDoc                              // pointer to document ida
)
{
   pDoc->Redraw |= REDRAW_LINE;                           // redraw the line
}

//------------------------------------------------------------------------------
// Function name:     EQFBBidiLRSwap   - swap the left and right functions
// External function
//------------------------------------------------------------------------------
// Function call:     EQFBBidiLRSwap(PTBDOCUMENT, BOOL);
//------------------------------------------------------------------------------
// Description  :     this funcion will switch the left and right functions
//                    to allow for RTL editing..
//
//------------------------------------------------------------------------------
// Parameter:         PTBDOCUMENT   - pointer to document ida
//                    BOOL          - swap to Bidi in case of TRUE
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//
//------------------------------------------------------------------------------
VOID EQFBBidiLRSwap(BOOL bSwap)
{
	BOOL bSwitch = FALSE;
	PFUNCTIONTABLE pFuncTab = get_FuncTab();
	
	/* switch screen dependent procedures (RTL screen always) */
	if ( bSwap )
	{
		if ( (pFuncTab + LEFT_FUNC)->function == EQFBFuncLeft )
		{
			bSwitch = TRUE;
		} /* endif */
	}
	else
	{
		if ( (pFuncTab + LEFT_FUNC)->function != EQFBFuncLeft )
		{
			bSwitch = TRUE;
		} /* endif */
	}

	if (bSwitch == TRUE)
	{
		FUNCTIONTABLE Temp;

		Temp                         = *(pFuncTab + LEFT_FUNC);     /* Right/Left cursor movements */
		*(pFuncTab + LEFT_FUNC)      = *(pFuncTab + RIGHT_FUNC);
		*(pFuncTab + RIGHT_FUNC)     = Temp;

		Temp                         = *(pFuncTab + NEXTWORD_FUNC); /* Right/Left wordtab */
		*(pFuncTab + NEXTWORD_FUNC)  = *(pFuncTab + PREVWORD_FUNC);
		*(pFuncTab + PREVWORD_FUNC)  = Temp;

		Temp                         = *(pFuncTab + MARKLEFT_FUNC); /* Right/Left marking */
		*(pFuncTab + MARKLEFT_FUNC)  = *(pFuncTab + MARKRIGHT_FUNC);
		*(pFuncTab + MARKRIGHT_FUNC) = Temp;

		Temp                         = *(pFuncTab + MARKNEXT_FUNC); /* Prev/Next Word marking */
		*(pFuncTab + MARKNEXT_FUNC)  = *(pFuncTab + MARKPREV_FUNC);
		*(pFuncTab + MARKPREV_FUNC)  = Temp;
	}
}


FUNCTIONTABLE EQFBBidiRedoLRSwap(USHORT usFunction)
{
    FUNCTIONTABLE Temp;
	PFUNCTIONTABLE pFuncTab = get_FuncTab();

    switch(usFunction)
    {
        case LEFT_FUNC:
            Temp = *(pFuncTab + RIGHT_FUNC);
            break;
        case RIGHT_FUNC:
            Temp = *(pFuncTab + LEFT_FUNC);
            break;
        case NEXTWORD_FUNC:
            Temp = *(pFuncTab + PREVWORD_FUNC);
            break;
        case PREVWORD_FUNC:
            Temp = *(pFuncTab + NEXTWORD_FUNC);
            break;
        case MARKLEFT_FUNC:
            Temp = *(pFuncTab + MARKRIGHT_FUNC);
            break;
        case MARKRIGHT_FUNC:
            Temp = *(pFuncTab + MARKLEFT_FUNC);
            break;
        case MARKNEXT_FUNC:
            Temp = *(pFuncTab + MARKPREV_FUNC);
            break;
        case MARKPREV_FUNC:
            Temp = *(pFuncTab + MARKNEXT_FUNC);
            break;
        default:
            Temp = *(pFuncTab + usFunction);
            break;

    } /* endswitch */

  return Temp;
}


VOID  EQFBBidiSysScrnCurPos
(
  PTBDOCUMENT pDoc,
  LONG      lRow,
  LONG      lCol
)
{
  BOOL   fOK    = TRUE;
  ULONG  ulLen;
  LONG   lCurCol   = lCol;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  PBIDISTRUCT   pBidiStruct = pDoc->pBidiStruct;
  /******************************************************************/
  /* fill in cursor row                                             */
  /******************************************************************/
  if (pBidiStruct && pBidiStruct->bOutputPending )
  {
    BidiDisplayLineEx( pDoc , lRow /*pBidiStruct->ulDisplayRow*/ );
  } /* endif */

  if ( fOK )
  {
    USHORT   usCursorMode;
    UINT     usBidiCol;

    /******************************************************************/
    /* init structure                                                 */
    /******************************************************************/
    ulLen                  = pDoc->lScrnCols-1;

    if ( !pBidiStruct->fRTL )
    {
      if ( pDoc->docType == SERVPROP_DOC )
      {
        PUINT   pusOrder = pBidiStruct->BidiLine[lRow].pusOrder;
        PBYTE   pbClass  = pBidiStruct->BidiLine[lRow].pbClass;
        if ( !pusOrder || !pbClass )
        {
          BidiDisplayLineEx( pDoc , lRow );
          pusOrder = pBidiStruct->BidiLine[lRow].pusOrder;
          pbClass  = pBidiStruct->BidiLine[lRow].pbClass;
        } /* endif */
        usBidiCol = pusOrder[ lCurCol ];
        usCursorMode = ID_BIDI_CARET_PUSH;
        if (pbClass[ lCurCol ] & (GCPCLASS_LATIN | GCPCLASS_LATINNUMBER))
        {
          usBidiCol = pusOrder[ lCurCol ];
          usCursorMode = ID_BIDI_CARET_PUSH;
          if ( !(pbClass[ lCurCol-1 ] & (GCPCLASS_LATIN| GCPCLASS_LATINNUMBER)))
          {
            /**********************************************************/
            /* only reposition if within a segment                    */
            /**********************************************************/
            PBIDIDISP pBidiDisp = pBidiStruct->pBidiDisp[ lRow ];
            while ( pBidiDisp && ((LONG)pBidiDisp->ulCol != lCurCol) )
            {
              pBidiDisp = pBidiDisp->pNext;
            } /* endwhile */
            if ( !pBidiDisp )
            {
              usBidiCol = pusOrder[ lCurCol - 1 ] - 1;
              usCursorMode = ID_BIDI_CARET_POP;
            } /* endif */
          } /* endif */
        }
        else
        {
          usBidiCol = pusOrder[ lCurCol ];
          usCursorMode = ID_BIDI_CARET_POP;
          if ( (pbClass[ lCurCol-1 ] & (GCPCLASS_LATIN| GCPCLASS_LATINNUMBER)))
          {
            usBidiCol = pusOrder[ lCurCol - 1 ] + 1;
            usCursorMode = ID_BIDI_CARET_PUSH;
          } /* endif */
        } /* endif */
        usBidiCol = usBidiCol * pDoc->cx;
      }
      else
      {
        usBidiCol    = lCol * pDoc->cx;
        usCursorMode = ID_BIDI_CARET_PUSH;
      } /* endif */
    }
    else
    {
      /***********************************************************/
      /* fill in correct cursor mode                             */
      /***********************************************************/
      if ( lCurCol > 0 )
      {
        PUINT   pusOrder = pBidiStruct->BidiLine[lRow].pusOrder;
        PBYTE   pbClass  = pBidiStruct->BidiLine[lRow].pbClass;
        if ( !pusOrder || !pbClass )
        {
          BidiDisplayLineEx( pDoc , lRow );
          pusOrder = pBidiStruct->BidiLine[lRow].pusOrder;
          pbClass  = pBidiStruct->BidiLine[lRow].pbClass;
        } /* endif */

        if (pbClass[ lCurCol ] & (GCPCLASS_LATIN | GCPCLASS_LATINNUMBER))
        {
          usBidiCol = pusOrder[ lCurCol ];
          usCursorMode = ID_BIDI_CARET_PUSH;
          if ( !(pbClass[ lCurCol-1 ] & (GCPCLASS_LATIN| GCPCLASS_LATINNUMBER)))
          {
            /**********************************************************/
            /* only reposition if within a segment                    */
            /**********************************************************/
            PBIDIDISP pBidiDisp = pBidiStruct->pBidiDisp[ lRow ];
            while ( pBidiDisp && ((LONG)pBidiDisp->ulCol != lCurCol) )
            {
              pBidiDisp = pBidiDisp->pNext;
            } /* endwhile */
            if ( !pBidiDisp )
            {
              usBidiCol = pusOrder[ lCurCol - 1 ] - 1;
              usCursorMode = ID_BIDI_CARET_POP;
            } /* endif */
          } /* endif */
        }
        else
        {
          usBidiCol = pusOrder[ lCurCol ];
          usCursorMode = ID_BIDI_CARET_POP;
          if ( (pbClass[ lCurCol-1 ] & (GCPCLASS_LATIN| GCPCLASS_LATINNUMBER)))
          {
            usBidiCol = pusOrder[ lCurCol - 1 ] + 1;
            usCursorMode = ID_BIDI_CARET_PUSH;
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* adjust for screen position                                 */
        /**************************************************************/
        if ( usBidiCol >= pBidiStruct->BidiLine[lRow].usOffset  )
        {
          usBidiCol -= pBidiStruct->BidiLine[lRow].usOffset;
          usBidiCol = (usBidiCol+1) * pDoc->cx;
        }
        else
        {
          usBidiCol = (pDoc->lScrnCols-1) * pDoc->cx+3;
        } /* endif */
      }
      else
      {
        usBidiCol = (pDoc->lScrnCols/*-1*/) * pDoc->cx;/*+3;*/
        usCursorMode = ID_BIDI_CARET_POP;
      } /* endif */
    } /* endif */

    /***********************************************************/
    /* adjust caret shape if necessary                         */
    /***********************************************************/
    if ( usCursorMode != pDoc->pBidiStruct->usBIDICursorMode )
    {
      HideCaret( pDoc->hwndClient );
      if ( pDoc->hCaretBitmap  )
      {
        DeleteObject( pDoc->hCaretBitmap );
        pDoc->hCaretBitmap = NULLHANDLE;
      } /* endif */
      DestroyCaret();

      pDoc->hCaretBitmap = LoadImage( hResMod,
                                      MAKEINTRESOURCE( usCursorMode ),
                                      IMAGE_BITMAP,
                                      pDoc->cx,
                                      pDoc->cy,
                                      LR_DEFAULTCOLOR );
      pDoc->pBidiStruct->usBIDICursorMode = usCursorMode;
      CreateCaret(pDoc->hwndClient, (HBITMAP) pDoc->hCaretBitmap, 0, 0 );
      ShowCaret( pDoc->hwndClient );
    } /* endif */
    /******************************************************************/
    /* position caret                                                 */
    /******************************************************************/
    pDoc->pBidiStruct->ulBidiCursorCol = (usBidiCol / pDoc->cx);
    SetCaretPos( usBidiCol, lRow * (pDoc->cy)+ pDoc->ulRulerSize);
  }
  else
  {
    pDoc->pBidiStruct->ulBidiCursorCol = pDoc->lScrnCols-lCol-1;
    SetCaretPos((pDoc->lScrnCols-lCol-1) * (pDoc->cx), lRow * (pDoc->cy)+ pDoc->ulRulerSize);
  } /* endif */
  return;
}


BOOL EQFBCheckBidiPart
(
  PTBDOCUMENT pDoc,
//  PSZ         pText,
  PSZ_W       pTextCompl,
  ULONG       ulLen,
  ULONG       ulCol,
  COLOUR      Colour,                // color to be displayed
  BYTE        bMark,                 // mark active
  ULONG       ulRow                  // row to be displayed
)
{
  PBIDISTRUCT  pBidiStruct = pDoc->pBidiStruct;
  BIDIDISP     BidiDisp;
  PBIDIDISP    pBidiDisp;
  LONG         lSideScroll = 0;
  /********************************************************************/
  /* check if we have to clean                                        */
  /********************************************************************/
  if ( pBidiStruct->fRedrawScreen )
  {
    pBidiStruct->fRedrawScreen = FALSE;
    { ULONG ulTest = sizeof(pBidiStruct->pBidiDisp);
      ulTest = sizeof(pBidiStruct->BidiLine);
      ulTest = ulTest + 1;
    }
    memset( &pBidiStruct->pBidiDisp[0], 0, sizeof( pBidiStruct->pBidiDisp ));
    memset( &pBidiStruct->BidiLine[0], 0, sizeof( pBidiStruct->BidiLine ));
    if ( pBidiStruct->pPool )
    {
      PoolDestroy( pBidiStruct->pPool );
    } /* endif */
    pBidiStruct->pPool = PoolCreate( 4000 );
  } /* endif */
  /********************************************************************/
  /* reset row - if we start with ulCol = 0                           */
  /********************************************************************/
  if ( ulCol == 0 )
  {
    /******************************************************************/
    /* start row from scratch - resources are managed in the pool and */
    /* freed the next time the screen is redrawn                      */
    /******************************************************************/
    pBidiStruct->pBidiDisp[ ulRow ] = NULL;
    pBidiStruct->BidiLine[ ulRow ].pusOrder = NULL;
  } /* endif */
  /********************************************************************/
  /* anchor new structure                                             */
  /********************************************************************/
  memset( &BidiDisp, 0, sizeof( BidiDisp ) );
  pBidiDisp = pBidiStruct->pBidiDisp[ ulRow ];
  if ( pBidiDisp )
  {
    PBIDIDISP pBidiTemp = pBidiDisp;
    while ( pBidiTemp )
    {
      pBidiDisp = pBidiTemp;
      pBidiTemp = pBidiTemp->pNext;
    } /* endwhile */
    lSideScroll = pBidiDisp->lSideScroll;
    pBidiDisp->pNext = (PBIDIDISP) PoolAddData( pBidiStruct->pPool,
                                                sizeof( BidiDisp ),
                                                &BidiDisp );
    pBidiDisp = pBidiDisp->pNext;
  }
  else
  {
    pBidiDisp = (PBIDIDISP)PoolAddData( pBidiStruct->pPool, sizeof( BidiDisp ),&BidiDisp);
    pBidiStruct->pBidiDisp[ ulRow ] = pBidiDisp;
  } /* endif */

  if ( pBidiDisp )
  {
//  PSZ_W pText =  &pBidiStruct->chOutBuf[0];
    if ( ulCol == 0 )
    {
      pBidiDisp->lSideScroll = (UTF16strlenCHAR( pTextCompl ) - ulLen);
    }
    else
    {
      pBidiDisp->lSideScroll= lSideScroll;   // sidescroll offset
    } /* endif */
    pBidiDisp->ulCol   = ulCol;                  // column of text
    pBidiDisp->ulLen   = UTF16strlenCHAR(pTextCompl);     // length of text
    pBidiDisp->usColor = (USHORT)Colour;         // color to be used
    pBidiDisp->bMark   = bMark;                  // mark active
    pBidiDisp->pTextCompl = (PSZ_W)PoolAddData( pBidiStruct->pPool,
                                         ( (pBidiDisp->ulLen+1) * sizeof(CHAR_W) ), pTextCompl);
  } /* endif */
  /********************************************************************/
  /* indicate we need an update                                       */
  /********************************************************************/
  pBidiStruct->bOutputPending = TRUE;
  pBidiStruct->bOutputPendingRow[ulRow] = TRUE;

  return TRUE;
}

VOID  BidiDisplayLine
(
  PTBDOCUMENT pDoc,
  PBIDISTRUCT pBidiStruct,
  HDC         hdc,
  ULONG       ulRow
)
{
  if ( pBidiStruct->fRTL )
  {
    pBidiStruct->bOutputPending = FALSE;
    for ( ulRow = 0; ulRow < (ULONG)(pDoc->lScrnRows); ulRow++ )
    {
      if ( pBidiStruct->bOutputPendingRow[ulRow]  )
      {
        COLORREF   clrBkRGB;
        COLORREF   clrFGRGB;
        USHORT     i;
        USHORT     usOffset;
        USHORT     usPos;
        PBIDIDISP  pBidiDisp = pBidiStruct->pBidiDisp[ ulRow ];
        PSZ_W      pTextW;
        PUINT      pusOrder;
        PBYTE      pbClass;

        PTEXTTYPETABLE pTextType;
        BOOL       fMark;
        ULONG      ulStartCol;
        ULONG      ulStringCol = 0L;
        RECT       rect;


        LONG         lFlags = BIDI_FLAGS(pDoc->lFontLangInfo);
        ULONG        ulDisplayRowLen;

        GCP_RESULTSW  GCPResults;

        /********************************************************************/
        /* get length of line and adjust symbol swapping characters         */
        /********************************************************************/
        ulStringCol = 0;
        while ( pBidiDisp )
        {
          PSZ_W pTextW = pBidiDisp->pTextCompl;
          ULONG i;
          for ( i=0; i<pBidiDisp->ulLen; i++ )
          {
            switch ( pTextW[i] )
            {
              case L'{':
                pTextW[i] = L'}';
                break;
              case L'}':
                pTextW[i]  = L'{';
                break;
              case L')':
                pTextW[i]  = L'(';
                break;
              case L'(':
                pTextW[i]  = L')';
                break;
              case L'<':
                pTextW[i]  = L'>';
                break;
              case L'>':
                pTextW[i]  = L'<';
                break;
              case L'\t':
                pTextW[i]  = L' ';
                break;
            } /* endswitch */
          } /* endfor */

          ulStringCol += pBidiDisp->ulLen;
          pBidiDisp = pBidiDisp->pNext;
        } /* endwhile */

        ulStringCol = max( ulStringCol, (ULONG)(pDoc->lScrnCols + pDoc->lSideScroll + 1) );

        if ( !pBidiStruct->BidiLine[ulRow].pusOrder )
        {
          pBidiStruct->BidiLine[ulRow].pusOrder =(PUINT)
                     PoolAddData( pBidiStruct->pPool,
                                  ((ulStringCol+1)* sizeof(UINT)),
                                  &pBidiStruct->usOrder[0]);
        } /* endif */

        if ( !pBidiStruct->BidiLine[ulRow].pbClass )
        {
          pBidiStruct->BidiLine[ulRow].pbClass =(PBYTE)
                     PoolAddData( pBidiStruct->pPool,
                                  ((ulStringCol+1)* sizeof(BYTE)),
                                  &pBidiStruct->bClass[0]);
        } /* endif */

        pusOrder = pBidiStruct->BidiLine[ulRow].pusOrder;
        pbClass  = pBidiStruct->BidiLine[ulRow].pbClass;

        /********************************************************************/
        /* fill usOrder (for complete segment)                              */
        /********************************************************************/
        ulStartCol = ulStringCol;
        ulDisplayRowLen = 0L;
        pBidiDisp = pBidiStruct->pBidiDisp[ ulRow ];
        while ( pBidiDisp )
        {
          ULONG ulCol = pBidiDisp->ulCol;
          ULONG ulLen = pBidiDisp->ulLen;
          /******************************************************************/
          /* adjust for side scroll of all columns (except first one)       */
          /******************************************************************/
          if ( ulCol != 0 )
          {
            ulCol += pBidiDisp->lSideScroll;
          } /* endif */


          ulDisplayRowLen          = ulLen;
          pTextW = &pBidiStruct->chDisplayRow[ulCol];
          memcpy( pTextW, pBidiDisp->pTextCompl, ulLen * sizeof(CHAR_W) );

//          ASCII2Unicode( PSZ pszASCII, PSZ_W pszUni, ULONG ulCP );
          pBidiDisp = pBidiDisp->pNext;
          while ( pBidiDisp &&
                  !(pBidiDisp->bMark & DISP_SEGMENT_START ) )
          {
            memcpy( &(pBidiStruct->chDisplayRow[pBidiDisp->ulCol+pBidiDisp->lSideScroll]),
                    pBidiDisp->pTextCompl, (pBidiDisp->ulLen) * sizeof(CHAR_W));
            ulDisplayRowLen += pBidiDisp->ulLen;
            pBidiDisp = pBidiDisp->pNext;
          } /* endwhile */

          ulStartCol  -= ulDisplayRowLen;

          memset( &GCPResults, 0, sizeof( GCP_RESULTS ));
          GCPResults.lStructSize   = sizeof( GCP_RESULTS );
          GCPResults.lpOrder       = (pusOrder+ulCol);
          GCPResults.lpClass       = (LPSTR)(pbClass+ulCol);
          GCPResults.lpCaretPos    = &pBidiStruct->usCaretPos[ulCol];
          GCPResults.nGlyphs       = ulDisplayRowLen;
          memset( GCPResults.lpOrder, 0, ulDisplayRowLen*sizeof(UINT) );
          memset( GCPResults.lpClass, 0, ulDisplayRowLen*sizeof( BYTE ));
          memset( GCPResults.lpCaretPos, 0, ulDisplayRowLen*sizeof(int));
          //GCPResults.lpClass[0]    = BIDI_OUT;
          /**************************************************************/
          /* adjust special characters ...                              */
          /**************************************************************/
          AdjustForBidi( pTextW, ulDisplayRowLen );
          // need to enforce GCP_REORDER... some of the Hebrew fonts do not
          // necessarily have this set correctly.
          GetCharacterPlacementW( hdc, pTextW,
                                 ulDisplayRowLen, 1000, &GCPResults,
                                 lFlags | GCP_REORDER );
          /******************************************************************/
          /* adjust lpCaretPos and lpOrder to offset used                   */
          /******************************************************************/
          for ( i=0; i<ulDisplayRowLen; i++ )
          {
            UINT us1 = *(pusOrder+ulCol+i);
            *(pusOrder+ulCol+i)    = us1 + ulStartCol;
          } /* endfor */

        } /* endwhile */


        pBidiDisp = pBidiStruct->pBidiDisp[ ulRow ];
        pTextW = &pBidiStruct->chDisplayRow[0];

        while ( pBidiDisp )
        {
          ULONG   i;
          ULONG   ulCol;
          ULONG   ulLen = 0;
          USHORT  usPosNew, usPosPrev;
          USHORT  usStartOffset = 0;
          SHORT   sDispPos;
          pTextType = get_TextTypeTable() + (COLOUR) pBidiDisp->usColor;
          fMark = (pBidiDisp->bMark & DISP_MARK);
          if (!UtlIsHighContrast())
          {
			  if ( (pTextType->fReverse && !fMark) || (fMark && !pTextType->fReverse))
			  {
				clrFGRGB =   COLORRGBTABLE[ pTextType->sBGColor];
				clrBkRGB =   COLORRGBTABLE[ pTextType->sFGColor];
			  }
			  else
			  {
				clrFGRGB =   COLORRGBTABLE[ pTextType->sFGColor];
				clrBkRGB =   COLORRGBTABLE[ pTextType->sBGColor];
			  } /* endif */
	      }
	      else
	      {
                clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_WINDOWTEXT));
				clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_WINDOW));
	      }
          SetTextColor(hdc, clrFGRGB);
          SetBkColor(hdc,clrBkRGB);

          usOffset = (USHORT)(ulStringCol - (pDoc->lScrnCols + pBidiDisp->lSideScroll));
          pBidiStruct->BidiLine[ulRow].usOffset = usOffset;
          ulCol    = pBidiDisp->ulCol;
          if ( ulCol > 0 )
          {
            ulCol += pBidiDisp->lSideScroll;
          } /* endif */
          // test only instead of the following for-loop
          usPosPrev = usPos = (USHORT)(pusOrder[ ulCol ]);
          ulLen = pBidiDisp->ulLen;

          for ( i=1; i < ulLen; i++ )
          {
            usPosNew = (USHORT)(pusOrder[ i + ulCol ]);
        if ( abs( usPosNew - usPosPrev ) == 1 )
        {
            usPos = min(usPosNew, usPos);
          usPosPrev = usPosNew;
        }
        else
        {
          // direction changed ... -- put it out and start again ..
              sDispPos = (SHORT)usPos - (SHORT)usOffset;

              rect.left = (sDispPos+1) * (pDoc->cx) ;
              rect.top  = ulRow * (pDoc->cy) + pDoc->ulRulerSize;
              rect.right = rect.left+(i - usStartOffset)*pDoc->cx;
              rect.bottom = rect.top + pDoc->cy;
              ExtTextOutW (hdc,
                          (sDispPos+1) * pDoc->cx,
                          ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                          ETO_OPAQUE,
                          &rect,
                          &pBidiDisp->pTextCompl[ usStartOffset ],
                          (i - usStartOffset), NULL);
              if (sDispPos == 0)
              {
                rect.left = 0;
                rect.right = pDoc->cx;

                ExtTextOut (hdc, 0, ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                            ETO_OPAQUE,
                            &rect, " ", 1, NULL );
              }
              // prepare for going ahead
              usPosPrev = usPos = usPosNew;
          usStartOffset = (USHORT) i;
        }
          }

          sDispPos = (SHORT)usPos - (SHORT)usOffset;

          rect.left = (sDispPos+1) * (pDoc->cx) ;
          rect.top  = ulRow * (pDoc->cy) + pDoc->ulRulerSize;
          rect.right = rect.left+(ulLen-usStartOffset)*pDoc->cx;
          rect.bottom = rect.top + pDoc->cy;
          ExtTextOutW(hdc,
                     (sDispPos+1) * pDoc->cx,
                     ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                     ETO_OPAQUE,
                     &rect,
                     &pBidiDisp->pTextCompl[ usStartOffset ],
           (ulLen-usStartOffset) , NULL);
          if (sDispPos == 0)
          {
            rect.left = 0;
            rect.right = pDoc->cx;

            ExtTextOut(hdc, 0, ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                        ETO_OPAQUE,
                        &rect, " ", 1, NULL );
          }
          /******************************************************************/
          /* point to next entry                                            */
          /******************************************************************/
          pBidiDisp = pBidiDisp->pNext;
        } /* endwhile */
        pBidiStruct->bOutputPendingRow[ulRow] = FALSE;
      } /* endif */
    } /* endfor */
  }
  else
  {
    BidiDisplayLineNonRTL( pDoc, pBidiStruct, hdc, ulRow );
  } /* endif */

  return;
}


VOID  BidiDisplayLineNonRTL
(
  PTBDOCUMENT pDoc,
  PBIDISTRUCT pBidiStruct,
  HDC         hdc,
  ULONG       ulRow
)
{
  pBidiStruct->bOutputPending = FALSE;
  for ( ulRow = 0; ulRow < (ULONG)(pDoc->lScrnRows); ulRow++ )
  {
    if ( pBidiStruct->bOutputPendingRow[ulRow]  )
    {
      COLORREF   clrBkRGB;
      COLORREF   clrFGRGB;
      USHORT     i;
      USHORT     usOffset;
      UINT       uiPos;
      PBIDIDISP  pBidiDisp = pBidiStruct->pBidiDisp[ ulRow ];
      PSZ_W      pText;
      PUINT      pusOrder;
      PBYTE      pbClass;

      PTEXTTYPETABLE pTextType;
      BOOL       fMark;
      ULONG      ulStartCol = 0L;
      ULONG      ulStringCol = 0L;
      ULONG      ulDisplayRowLen;

      /********************************************************************/
      /* get length of line                                               */
      /********************************************************************/
      ulStringCol = 0L;
      while ( pBidiDisp )
      {
        ulStringCol += pBidiDisp->ulLen;
        pBidiDisp = pBidiDisp->pNext;
      } /* endwhile */

      ulStringCol = max( ulStringCol, (ULONG)(pDoc->lScrnCols + pDoc->lSideScroll + 1) );

      if ( !pBidiStruct->BidiLine[ulRow].pusOrder )
      {
        pBidiStruct->BidiLine[ulRow].pusOrder =(PUINT)
                   PoolAddData( pBidiStruct->pPool,
                                ((ulStringCol+1)* sizeof(UINT)),
                                &pBidiStruct->usOrder[0]);
      } /* endif */

      if ( !pBidiStruct->BidiLine[ulRow].pbClass )
      {
        pBidiStruct->BidiLine[ulRow].pbClass =(PBYTE)
                   PoolAddData( pBidiStruct->pPool,
                                ((ulStringCol+1)* sizeof(BYTE)),
                                &pBidiStruct->bClass[0]);
      } /* endif */

      pusOrder = pBidiStruct->BidiLine[ulRow].pusOrder;
      pbClass  = pBidiStruct->BidiLine[ulRow].pbClass;

      /********************************************************************/
      /* fill usOrder (for complete segment)                              */
      /********************************************************************/
      ulStartCol = ulStringCol;
      ulDisplayRowLen = 0L;
      pBidiDisp = pBidiStruct->pBidiDisp[ ulRow ];
      while ( pBidiDisp )
      {
        ULONG ulCol = pBidiDisp->ulCol;
        ULONG ulLen = pBidiDisp->ulLen;
        BOOL   fPrefix = FALSE;
        /******************************************************************/
        /* adjust for side scroll of all columns (except first one)       */
        /******************************************************************/
        if ( ulCol != 0 )
        {
          ulCol = ulCol + pBidiDisp->lSideScroll;
        }
        else
        {
          fPrefix = TRUE;
        } /* endif */


        ulDisplayRowLen          = ulLen;
        pText = &pBidiStruct->chDisplayRow[ulCol];
        memcpy( pText, pBidiDisp->pTextCompl, ulLen * sizeof(CHAR_W) );

        pBidiDisp = pBidiDisp->pNext;
        while ( pBidiDisp &&
                !(pBidiDisp->bMark & DISP_SEGMENT_START ) )
        {
          memcpy( &(pBidiStruct->chDisplayRow[pBidiDisp->ulCol+pBidiDisp->lSideScroll]),
                  pBidiDisp->pTextCompl, (pBidiDisp->ulLen )* sizeof(CHAR_W));
          ulDisplayRowLen +=  pBidiDisp->ulLen;
          pBidiDisp = pBidiDisp->pNext;
        } /* endwhile */

        ulStartCol  -= ulDisplayRowLen;


        if ( fPrefix )
        {
          for ( i=0; i<ulDisplayRowLen; i++ )
          {
            pusOrder[ i ] = i;
            *(pbClass+i)  = ((USHORT)pText[i] < 0x80) ? GCPCLASS_LATIN : 0;
          } /* endfor */
        }
        else
        {
          UINT   usIndex;
          CHAR   chText[256];
          CHAR_W c   = pText[ulDisplayRowLen];
          pText[ulDisplayRowLen] = EOS;
          usIndex = sizeof( pBidiStruct->chOutBuf );
          memset( pBidiStruct->chOutBuf, 0, usIndex);

          /**************************************************************/
          /* adjust special characters ...                              */
          /**************************************************************/
          AdjustForBidi( pText, ulDisplayRowLen );
          Unicode2ASCII( pText, chText, 0L );
          BiDitransform( (unsigned char *)&chText[0], ulDisplayRowLen,
                         pBidiStruct->usCCSIdIn, 6,
                         (unsigned char *)&pBidiStruct->chOutBuf[0], &usIndex,
                         pBidiStruct->usCCSIdOut, 4, 0 ,
                         &pBidiStruct->usOrder[0]);
          pText[ulDisplayRowLen] = c;

          usIndex = sizeof( chBidiText );
          memset( &chBidiText[0], 0, usIndex);
          BiDitransform( (unsigned char *)&pBidiStruct->chOutBuf[0], ulDisplayRowLen,
                         pBidiStruct->usCCSIdIn, 4,
                         (unsigned char *)&chBidiText[0], &usIndex,
                         pBidiStruct->usCCSIdOut, 5, 0,
                         NULL);
          /******************************************************************/
          /* adjust lpCaretPos and lpOrder to offset used                   */
          /******************************************************************/
          for ( i=0; i<ulDisplayRowLen; i++ )
          {
            *(pusOrder+ulCol+i) = pBidiStruct->usOrder[i]+ulCol;
            if ( (BYTE)pText[i] < 0x80 )
            {
              *(pbClass+ulCol+i)  = (isalnum((BYTE)pText[i])) ? GCPCLASS_LATIN : 0; /* Add (BYTE) 7-23-14 */
            }
            else
            {
              *(pbClass+ulCol+i)  = 0;
            } /* endif */
          } /* endfor */

        } /* endif */

      } /* endwhile */


      pBidiDisp = pBidiStruct->pBidiDisp[ ulRow ];
      pText = &pBidiStruct->chDisplayRow[0];
      while ( pBidiDisp )
      {
        ULONG ulCol;
        pTextType = get_TextTypeTable() + (COLOUR) pBidiDisp->usColor;
        fMark = (pBidiDisp->bMark & DISP_MARK);
        if (!UtlIsHighContrast())
        {
			if ( (pTextType->fReverse && !fMark) || (fMark && !pTextType->fReverse))
			{
			  clrFGRGB =   COLORRGBTABLE[ pTextType->sBGColor];
			  clrBkRGB =   COLORRGBTABLE[ pTextType->sFGColor];
			}
			else
			{
			  clrFGRGB =   COLORRGBTABLE[ pTextType->sFGColor];
			  clrBkRGB =   COLORRGBTABLE[ pTextType->sBGColor];
			} /* endif */
        }
        else
        {
			clrFGRGB =   UtlGetColorref(GetSysColor(COLOR_WINDOWTEXT));
			clrBkRGB =   UtlGetColorref(GetSysColor(COLOR_WINDOW));
        }
        SetTextColor(hdc, clrFGRGB);
        SetBkColor(hdc,clrBkRGB);

        usOffset = (USHORT)(ulStringCol - (pDoc->lScrnCols + pBidiDisp->lSideScroll));
        pBidiStruct->BidiLine[ulRow].usOffset = usOffset;
        ulCol    = pBidiDisp->ulCol;
        if ( ulCol > 0 )
        {
          ulCol += pBidiDisp->lSideScroll;
        } /* endif */


        for ( i=0; i < pBidiDisp->ulLen; i++ )
        {
          uiPos = pusOrder[ i + ulCol ];
          if ( uiPos < (USHORT) pDoc->lScrnCols )
          {
            TextOutW (hdc,
                     uiPos * (pDoc->cx),
                     ulRow * (pDoc->cy) + pDoc->ulRulerSize,
                     &pBidiDisp->pTextCompl[i], 1 );
          } /* endif */

        } /* endfor */

        /******************************************************************/
        /* point to next entry                                            */
        /******************************************************************/
        pBidiDisp = pBidiDisp->pNext;
      } /* endwhile */
      pBidiStruct->bOutputPendingRow[ulRow] = FALSE;
    } /* endif */
  } /* endfor */

  return;
}

void BidiDisplayLineEx
(
  PTBDOCUMENT pDoc,
  ULONG       ulRow
)
{
  BOOL  fFocus = FALSE;
  HDC   hdc;
  HFONT hFont;
  PBIDISTRUCT pBidiStruct = pDoc->pBidiStruct;

  if ( pDoc->hwndClient == GetFocus() )
  {
    HideCaret( pDoc->hwndClient );
    fFocus = TRUE;
  } /* endif */
  hdc = GetDC(pDoc->hwndClient);
  hFont = CreateFontIndirect( &pDoc->lf );
  hFont = (HFONT)SelectObject( hdc, hFont );
  if ( pBidiStruct->fRTL )
  {
    SetTextAlign( hdc, TA_RTLREADING );
    BidiDisplayLine( pDoc, pBidiStruct, hdc, ulRow );
  }
  else
  {
    BidiDisplayLineNonRTL( pDoc, pBidiStruct, hdc, ulRow );
  } /* endif */
  DeleteObject( SelectObject( hdc, hFont ) );
  ReleaseDC( pDoc->hwndClient, hdc );
  if ( fFocus )
  {
    ShowCaret( pDoc->hwndClient );
  } /* endif */

  return;
}
/**********************************************************************/
/* Callback function from Timer for Display update under BIDI         */
/**********************************************************************/
void CALLBACK TimerDisp
(
  HWND  hwnd,
  UINT  uiMsg,
  UINT  uiEvent,
  DWORD dwTime
)
{
  PTBDOCUMENT pDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );
  PBIDISTRUCT pBidiStruct = pDoc->pBidiStruct;

  uiMsg; uiEvent; dwTime;

  if (pBidiStruct && pBidiStruct->bOutputPending )
  {
    BidiDisplayLineEx( pDoc , pBidiStruct->ulDisplayRow );
  } /* endif */

  return;
}

/**********************************************************************/
/* Callback function from Timer for Display update under BIDI         */
/**********************************************************************/
void CALLBACK TimerDispDev
(
  HWND  hwnd,
  UINT  uiMsg,
  UINT  uiEvent,
  DWORD dwTime
)
{
  PTWBSDEVICE pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
  PTBDOCUMENT pDoc =&pDevice->tbDoc;
  PBIDISTRUCT pBidiStruct = pDoc->pBidiStruct;

  uiMsg; uiEvent; dwTime;
  if (pBidiStruct && pBidiStruct->bOutputPending )
  {
    BidiDisplayLineEx( pDoc , pBidiStruct->ulDisplayRow );
  } /* endif */

  return;
}
/**********************************************************************/
/* Adjust neutral characters to be displayed as strong latin          */
/* Note:                                                              */
/*    pText will be modified                                          */
/**********************************************************************/
static VOID AdjustForBidi
(
  PSZ_W  pText,
  ULONG  ulDisplayRowLen
)
{
  /**************************************************************/
  /* adjust special characters ...                              */
  /**************************************************************/
  ULONG i = 0L;
  while ( i < ulDisplayRowLen )
  {
    switch ( pText[i] )
    {
      case ')':
      case '(':
      case '%':
      case '&':
      case '*':
      case '>':
      case '<':
      case '#':
        pText[i] = 'x';
        break;
      default:
        break;
    } /* endswitch */
    i++;
  } /* endwhile */
}




// Get valid unicode ranges for current font and anchor them in the document structure
LPGLYPHSET GetFontContRanges( PTBDOCUMENT pDoc, HDC hDC)
{
  LPGLYPHSET lpGlyphSet = NULL;

  // check if a glypset is already available
  if ( pDoc->pvGlyphSet != NULL )
  {
    return( (LPGLYPHSET)pDoc->pvGlyphSet );
  } /* endif */

  // get required size for glyphset
  DWORD dwLen = GetFontUnicodeRanges( hDC, NULL );

  // allocate glyphset structure
  if ( !UtlAlloc( (PVOID *)&lpGlyphSet, 0, dwLen, ERROR_STORAGE ) )
  {
    return( NULL );
  } /* endif */

  // get glyphset
  lpGlyphSet->cbThis = dwLen;
  lpGlyphSet->flAccel = 0;
  lpGlyphSet->cRanges = 0;
  lpGlyphSet->cGlyphsSupported = 0;
  dwLen = GetFontUnicodeRanges( hDC, lpGlyphSet );
  pDoc->pvGlyphSet = (PVOID)lpGlyphSet;

  return( lpGlyphSet );
} /* end of GetFontContRanges */

// check if a character is in the provided range (=0), or before the range (=-1) of following the range (=1)
int CheckAgainstRange( CHAR_W chTest, CHAR_W chRangeStart, int iGlyphsInRange )
{
  if ( chTest < chRangeStart ) return( -1 );
  if ( chTest >= chRangeStart + iGlyphsInRange ) return( 1 );
  return( 0 );
}

// Check if given character is contained in the provided glyph list
BOOL IsInGlyphSet( LPGLYPHSET lpGlyphSet, CHAR_W chTest )
{
  // this is the standard binary search algorythm, we could not use bsearch as the ranges complicates the compare function
  const int iNotFound = -1;
  int iLeft = 0;                       // first entry 
  int iRight = lpGlyphSet->cRanges;    // last entry

  // as long as the list being searched is not empty
  while ( iLeft <= iRight )
  {
      int iMid = iLeft + ((iRight - iLeft) / 2); 
      int iCompareResult = CheckAgainstRange( chTest, lpGlyphSet->ranges[iMid].wcLow, lpGlyphSet->ranges[iMid].cGlyphs );

      if ( iCompareResult == 0)       // we found the correct range
          return ( TRUE );
      else
          if ( iCompareResult < 0 )
              iRight = iMid - 1;     // continue on the left side of the list
          else 
              iLeft = iMid + 1;      // continue on the right side of the list
          /* end if */
      /* end if */
  }
  return( FALSE );
} /* end of IsInGlyphSet */

// Replace all code points not contained in current font by a question mark
void MaskInvalidCharacters( PTBDOCUMENT pDoc, HDC hDC, PSZ_W pszText )
{
  CHAR_W chNoGlyphChar = 0;

  LPGLYPHSET lpGlyphSet = GetFontContRanges( pDoc, hDC );
  if ( lpGlyphSet == NULL ) return;

  while( *pszText )
  {
    if ( !IsInGlyphSet( lpGlyphSet, *pszText ) )
    {
      if ( chNoGlyphChar == 0 )
      {
        chNoGlyphChar = 0x25A1;
        if ( !IsInGlyphSet( lpGlyphSet, chNoGlyphChar ) )
        {
          chNoGlyphChar = L'?';
        } /* endif */
      } /* endif */
      *pszText = chNoGlyphChar;
    } /* endif */
    pszText++;
  } /* endwhile */
} /* end of MaskInvalidCharacters */

// Replace special characters in the display string
//    tab to space
//    control codes
//    FFFC to bullet
//
VOID CleanDisplayString( PTBDOCUMENT pDoc, HDC hDC, PSZ_W pText )
{
  CHAR_W b;
  PSZ_W pszTest = pText;
  while ((b = *pszTest) != NULC)
  {
    if ( b == '\t')
    {
      *pszTest = ' ';
    }
    else if (b == 0xFFFC)
    {
  		*pszTest = 183;
    }
    else if (b == 0x0006)
    {
  		*pszTest = 0x002D;
    }
    else if (b == 0x0005)
  	{
	  	*pszTest = 0x007C;
    }
    else if ((b <= 0x0004) && (b>=0x0001))
	  {
		  *pszTest = 0x00B7;
    }
    else if ((b == 0x0017) || (b == 0x0019))
    {
		  *pszTest = 0x00B7;
    }
    pszTest++;
  }

  // mask all characters not valid for the currently selected font
  MaskInvalidCharacters( pDoc, hDC, pText );
}

