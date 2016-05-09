//+----------------------------------------------------------------------------+
//|  EQFUTPRT.C - EQF general print utilities                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:         MAT development team                                        |
//|                reworked and maintained by G.Queck (QSoft)                  |
//+----------------------------------------------------------------------------+
//|Description:    General print utilities                                     |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|    UtlPrintClose          Close a print file/device                        |
//|    UtlPrintFormFeed       Start a new print page                           |
//|    UtlPrintLine           Print a text line                                |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//|  - MRI isolation for hardcoded error title strings                         |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
// 
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
//
// $Revision: 1.2 $ ----------- 29 Jul 2002
// --RJ: R07197: use system pref. lang to print listname
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.4 $ ----------- 12 Nov 2001
// RJ: correct allocation size
//
//
// $Revision: 1.3 $ ----------- 19 Oct 2001
// --RJ: fix error in UtlPrintLineW, add UtlPrintOpenW
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: Unicode enabling
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
// $Revision: 1.3 $ ----------- 19 Feb 2001
// -- Thai: add UtlPrintSetAnsiConv
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFUTPRT.CV_   1.5   20 Apr 1998 11:54:58   BUILD  $
 *
 * $Log:   J:\DATA\EQFUTPRT.CV_  $
 *
 *    Rev 1.5   20 Apr 1998 11:54:58   BUILD
 * - use supplied owner handle for call to PrintDlg
 *
 *    Rev 1.4   23 Feb 1998 11:41:06   BUILD
 * - Win32: avoid compiler warnings
 *
 *    Rev 1.3   09 Feb 1998 17:20:32   BUILD
 * - ???
 *
 *    Rev 1.2   20 Sep 1996 20:39:26   BUILD
 * - added OEM->Ansi conversion for strings being printed via UtlPrintLine
 *
 *    Rev 1.1   11 Mar 1996 10:33:18   BUILD
 * - avoid compiler ewarning in line 362
 *
 *    Rev 1.0   09 Jan 1996 09:17:00   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+


/**********************************************************************/
/* Include files                                                      */
/**********************************************************************/
#define INCL_DEV
#include <eqf.h>                  // General Translation Manager include file
#include <eqfutpri.h>                  // internal printer file

/**********************************************************************/
/* under OS/2 we only could use the normal printing direct to LPT1    */
/**********************************************************************/
#undef USE_LPT1_PRINT

#ifndef USE_LPT1_PRINT
  #include <commdlg.h>
#endif

static BOOL fPrintOK = FALSE;
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlPrintOpen           Open a print file/device          |
//+----------------------------------------------------------------------------+
//|Description:       Opens a print device context.                            |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlPrintOpen( PHPRINT phPrint, PSZ pszListName)     |
//+----------------------------------------------------------------------------+
//|Parameters:        - phPrint points to a print handle of type HPRINT.       |
//|                     This handle must be specified on subsequent UtlPrintxx |
//|                     calls.                                                 |
//|                   - pszListName points the list name (the name under which |
//|                     the print output appears in the print queue).          |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   if successful                                     |
//|                   FALSE  if function failed                                |
//+----------------------------------------------------------------------------+
//|Function flow:     a) in case of USE_LPT1_PRINT defined                     |
//|                   allocate print control block                             |
//|                   if ok then                                               |
//|                     prompt for printer                                     |
//|                     open device context for printer                        |
//|                     start a new document                                   |
//|                     get printer page size                                  |
//|                     get printer character sizes                            |
//|                   endif                                                    |
//|                                                                            |
//|                   b) in case of USE_LPT1_PRINT not defined                 |
//|                   allocate print control block                             |
//|                   if ok then                                               |
//|                     open the printer file LPT1                             |
//|                     if open fails                                          |
//|                       free printer control block                           |
//|                     endif                                                  |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
BOOL UtlPrintOpenW
(
  PHPRINT    phPrint,
  PSZ_W      pszListNameW,
  HWND       hwndOwner
)
{
  BOOL  fOK = TRUE;
  PSZ   pszListName;
  ULONG  ulI = 0;
  ULONG  ulLen = 0;

  ulI = UTF16strlenCHAR(pszListNameW);

  ulLen = max( (ulI+1) * sizeof(CHAR), MIN_ALLOC );
  if ( UtlAlloc( (PVOID *) &pszListName, 0L,
                   (LONG) ulLen, ERROR_STORAGE ) )
  {
    Unicode2ASCII(pszListNameW, pszListName, 0L);
    fOK = UtlPrintOpen( phPrint, pszListName, hwndOwner);
  }
  else
  {
    fOK = FALSE;
  }
  return (fOK);
}

BOOL UtlPrintOpen
(
  PHPRINT    phPrint,                 // print handle
  PSZ        pszListName,             // name of output listing
  HWND       hwndOwner                // owner HWND
)
{
  PPRINTDATA pPrintData;              // print data structure
#ifndef USE_LPT1_PRINT
  PRINTDLG    pd;
  LPDEVMODE   lpDevMode = NULL;
  LPDEVNAMES  lpDevNames;
  PSZ         pszDriverName;
  PSZ         pszDeviceName;
  PSZ         pszPortName;
  TEXTMETRIC  TextMetric;
  DOCINFO     *pDocInfo;
  ULONG       ulPageSize;
  int         sRc;
#endif
  BOOL  fOK = TRUE;                               // success indicator

  UtlAlloc( (PVOID *)&pPrintData, 0L, (LONG) sizeof(PRINTDATA), ERROR_STORAGE );
  *phPrint = (ULONG) pPrintData;

  if ( pPrintData )
  {
          pPrintData->fAnsiConv = TRUE;   //default: do OEM-ansi conversion
#ifndef USE_LPT1_PRINT
    memset(&pd, 0, sizeof(PRINTDLG));
    pd.lStructSize = sizeof(PRINTDLG);
    pd.hDevMode   = NULL;
    pd.hDevNames  = NULL;
    pd.Flags      = PD_RETURNDC | PD_PRINTSETUP;
    pd.hwndOwner  = hwndOwner;
    if ( !PrintDlg((LPPRINTDLG)&pd) )
    {
      UtlAlloc( (PVOID *)&pPrintData, 0L, 0L, ERROR_STORAGE );
      *phPrint = 0L;
      fOK = FALSE;
    }
    else
    {
      if (pd.hDC)
      {
        pPrintData->hdcPrinter = pd.hDC;
      }
      else
      {
        if ( !pd.hDevNames )
        {
          UtlAlloc( (PVOID *)&pPrintData, 0L, 0L, ERROR_STORAGE );
          *phPrint = 0L;
          fOK = FALSE;
        }
        else
        {
          lpDevNames = (LPDEVNAMES) GlobalLock(pd.hDevNames);
          pszDriverName = (LPSTR)lpDevNames + lpDevNames->wDriverOffset;
          pszDeviceName = (LPSTR)lpDevNames + lpDevNames->wDeviceOffset;
          pszPortName   = (LPSTR)lpDevNames + lpDevNames->wOutputOffset;
          GlobalUnlock(pd.hDevNames);

          if (pd.hDevMode) lpDevMode = (LPDEVMODE) GlobalLock(pd.hDevMode);

          pPrintData->hdcPrinter = CreateDC(pszDriverName, pszDeviceName,
                                            pszPortName, lpDevMode);

          if (pd.hDevMode && lpDevMode) GlobalUnlock(pd.hDevMode);
        } /* endif */
      } /* endif */

      if (pd.hDevNames)
      {
        GlobalFree(pd.hDevNames);
        pd.hDevNames=NULL;
      } /* endif */
      if (pd.hDevMode)
      {
        GlobalFree(pd.hDevMode);
        pd.hDevMode=NULL;
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      UtlAlloc( (PVOID *) &pDocInfo, 0L, (LONG) sizeof(DOCINFO), ERROR_STORAGE );
      pDocInfo->cbSize = sizeof(DOCINFO);
      pDocInfo->lpszDocName = pszListName;
      pDocInfo->lpszOutput = NULL;

      sRc = StartDoc(pPrintData->hdcPrinter, pDocInfo);
      if ( sRc < 0 )
      {
        DeleteDC(pPrintData->hdcPrinter);
        UtlAlloc( (PVOID *)&pPrintData, 0L, 0L, ERROR_STORAGE );
        *phPrint = 0L;
        fOK = FALSE;
      } /* endif */
      UtlAlloc((PVOID *) &pDocInfo, 0L, 0L, ERROR_STORAGE );
    } /* endif */

    if ( fOK )
    {
      StartPage( pPrintData->hdcPrinter );
      // Since you may have more than one line, you need to
      // compute the spacing between lines.  You can do that by
      // retrieving the height of the characters you are printing
      // and advancing their height plus the recommended external
      // leading height.
      GetTextMetrics(pPrintData->hdcPrinter, &TextMetric);
      pPrintData->usLineSpace = (USHORT)(TextMetric.tmHeight + TextMetric.tmExternalLeading);

      // Since you may have more lines than can fit on one
      // page, you need to compute the number of lines you can
      // print per page.  You can do that by retrieving the
      // dimensions of the page and dividing the height
      // by the line spacing.
      ulPageSize = GetDeviceCaps (pPrintData->hdcPrinter, VERTRES);
      pPrintData->usLinesPerPage = (USHORT)(ulPageSize /
                                   pPrintData->usLineSpace
                                   - 1);

      pPrintData->usLine = 1;
    } /* endif */
#else
    pszListName;                      // avoid compiler warning

    pPrintData->hFile = fopen( "LPT1", "at" );
    if ( !pPrintData->hFile )
    {
       UtlAlloc( (PVOID *)&pPrintData, 0L, 0L, ERROR_STORAGE );
       *phPrint = 0L;
       fOK = FALSE;
    }
    else
    {
      fPrintOK = TRUE;
    } /* endif */
#endif
  } /* endif */
  return( fOK );
}

BOOL UtlPrintSetAnsiConv
(
          HPRINT    hPrint,                    // print handle
          BOOL      fAnsiConv
)
{
          PPRINTDATA pPrintData;              // print data structure

          pPrintData = (PPRINTDATA) hPrint;
      if ( pPrintData )
      {
                  pPrintData->fAnsiConv = fAnsiConv;
      } /* endif */
    return( TRUE );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlPrintClose          Close a print file/device         |
//+----------------------------------------------------------------------------+
//|Description:       Closes a print device context opened using UtlPrintOpen  |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlPrintClose( HPRINT hPrint )                      |
//+----------------------------------------------------------------------------+
//|Parameters:        - hPrint is the print handle set by UtlPrintOpen.        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      - Print device context must have benn opened using       |
//|                     UtlPrintOpen                                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   if successful                                     |
//|                   FALSE  if function failed                                |
//+----------------------------------------------------------------------------+
//|Function flow:     a) in case of USE_LPT1_PRINT defined                     |
//|                   if printdata exists then                                 |
//|                     free printer association                               |
//|                     send end document command                              |
//|                     close printer device context                           |
//|                     destroy printer presentation space                     |
//|                     free printdata control block                           |
//|                   endif                                                    |
//|                                                                            |
//|                   b) in case of USE_LPT1_PRINT not defined                 |
//|                   if printdata exists then                                 |
//|                     close printer file                                     |
//|                     free printdata control block                           |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
BOOL UtlPrintClose
(
  HPRINT    hPrint                    // print handle
)
{
  PPRINTDATA pPrintData;              // print data structure

  pPrintData = (PPRINTDATA) hPrint;
  if ( pPrintData )
  {
#ifndef USE_LPT1_PRINT
    EndPage( pPrintData->hdcPrinter );
    EndDoc(pPrintData->hdcPrinter);
    DeleteDC(pPrintData->hdcPrinter);
#else
    if ( fPrintOK )
    {
      UtlPrintFormFeed( hPrint );
    } /* endif */
    fclose( pPrintData->hFile );
#endif
    UtlAlloc( (PVOID *)&pPrintData, 0L, 0L, NOMSG );
  } /* endif */
  return( TRUE );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlPrintFormFeed       Start a new print page            |
//+----------------------------------------------------------------------------+
//|Description:       Start a new page for a print device context              |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlPrintFormFeed( HPRINT hPrint )                   |
//+----------------------------------------------------------------------------+
//|Parameters:        - hPrint is the print handle set by UtlPrintOpen.        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      - Print device context must have benn opened using       |
//|                     UtlPrintOpen                                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   if successful                                     |
//|                   FALSE  if function failed                                |
//+----------------------------------------------------------------------------+
//|Function flow:     a) in case of USE_LPT1_PRINT defined                     |
//|                   if printdata exists then                                 |
//|                     send new fram command to printer                       |
//|                   endif                                                    |
//|                                                                            |
//|                   b) in case of USE_LPT1_PRINT not defined                 |
//|                   if printdata exists then                                 |
//|                     write formfeed character to printer file               |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
BOOL UtlPrintFormFeed
(
  HPRINT    hPrint                    // print handle
)
{
  PPRINTDATA pPrintData;              // print data structure
  BOOL       fOK = TRUE;

  pPrintData = (PPRINTDATA) hPrint;
  if ( pPrintData )
  {
#ifndef USE_LPT1_PRINT
    EndPage( pPrintData->hdcPrinter );
    StartPage( pPrintData->hdcPrinter );
#else
    fOK = (fputs( "\f", pPrintData->hFile ) > 0);
#endif
    pPrintData->usLine = 1;
  } /* endif */
  return( fOK );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlPrintLine           Print a text line                 |
//+----------------------------------------------------------------------------+
//|Description:       Write a text line to a print device context              |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlPrintLine( HPRINT hPrint, PSZ pszLine )          |
//+----------------------------------------------------------------------------+
//|Parameters:        - hPrint is the print handle set by UtlPrintOpen.        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      - Print device context must have been opened using       |
//|                     UtlPrintOpen                                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   if successful                                     |
//|                   FALSE  if function failed                                |
//+----------------------------------------------------------------------------+
//|Function flow:     a) in case of USE_LPT1_PRINT defined                     |
//|                   if printdata exists then                                 |
//|                     remove trailing linefeed character (if any)            |
//|                     determine output position                              |
//|                     write string to printer using GpiCharStringAt          |
//|                     if page size has been exceeded then                    |
//|                       send new frame command to printer                    |
//|                     endif                                                  |
//|                   endif                                                    |
//|                                                                            |
//|                   b) in case of USE_LPT1_PRINT not defined                 |
//|                   if printdata exists then                                 |
//|                     write supplied line data to print file                 |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
BOOL UtlPrintLine
(
  HPRINT    hPrint,                   // print handle
  PSZ       pszLine                   // ptr to line data
)
{
  BOOL  fOK;
  LONG  ulLen = strlen( pszLine )+1;
  PSZ_W pszLineW;

  fOK = UtlAlloc((PVOID *) &pszLineW, 0L, ulLen * sizeof(CHAR_W), ERROR_STORAGE );
  if ( fOK )
  {
  ASCII2Unicode( pszLine, pszLineW, 0L );
  fOK = UtlPrintLineW( hPrint, pszLineW );
  UtlAlloc((PVOID *) &pszLineW, 0L, 0L, NOMSG );
  }
  return fOK;
}


BOOL UtlPrintLineW
(
  HPRINT    hPrint,                   // print handle
  PSZ_W     pszLine                   // ptr to line data
)
{
  PPRINTDATA pPrintData;              // print data structure
  BOOL       fOK = TRUE;

#ifndef USE_LPT1_PRINT
  ULONG ulLineLength;
#endif

  pPrintData = (PPRINTDATA) hPrint;
  if ( pPrintData )
  {

#ifndef USE_LPT1_PRINT
    *(pszLine + UTF16strcspn(pszLine,L"\n") ) = '\0';  // remove trailing \n
    *(pszLine + UTF16strcspn(pszLine,L"\r") ) = '\0';  // remove trailing \r

    ulLineLength = UTF16strlenCHAR( pszLine );
//  if (pPrintData->fAnsiConv )
//    {
//      OEMTOANSI( pszLine );
//    }
    fOK = TextOutW(pPrintData->hdcPrinter,
                  0,
                  pPrintData->usLine*pPrintData->usLineSpace,
                  pszLine,
                  ulLineLength);
    pPrintData->usLine++;

    if (pPrintData->usLine > pPrintData->usLinesPerPage)
      UtlPrintFormFeed(hPrint);
#else
    fOK = (fputs( pszLine, pPrintData->hFile ) >= 0);

    /****************************************************************/
    /* Add LF if input string does not end with a LF                */
    /****************************************************************/
    if ( fOK && ((*pszLine == EOS) ||
                 (pszLine[UTF16strlenCHAR(pszLine)-1] != LF )) )
    {
      fOK = ( fputs( "\n", pPrintData->hFile ) >= 0 );
    } /* endif */

#endif
  } /* endif */
  /*******************************************************************/
  /* set current status of print..                                   */
  /*******************************************************************/
  fPrintOK = fOK;
  return( fOK );
}
