//+----------------------------------------------------------------------------+
//| EQFXLFUE.C                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|              Copyright (C) 1990-2012, International Business Machines      |
//|              Corporation and others. All rights reserved                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Parser / user exit for the EQFXLIFF markup                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions

// use import DLL defines for dbcs_cp ...
#define DLLIMPORTDBCSCP

#include <time.h>
#include <locale.h>
#include <eqf.h>                  // General Translation Manager include file

#include <eqfpapi.h>                   // parser API

// global variables, buffers and other stuff
CHAR_W szLine[1024];                   // buffer for line data
CHAR_W szSegBuffer[8096];              // buffer for segment in SGML format
PARSSEGMENTW SegW;                     // buffer for segment data (UTF16)

#include <tchar.h>


static void AddToTable
(
  PSTARTSTOP       pStartStop,         // ptr to start/stop table
  PUSHORT          pusCurEntry,        // ptr to number of current table entry
  USHORT           usMaxEntries,       // max number of table entries
  USHORT           usCurOffs,          // current offset of character
  USHORT           usType              // type of character
);


/**********************************************************************/
/* Prototype for new API entry EQFPROTTABLE                           */
/* should be incorporated in EQF_API.H once this user exit function   */
/* gets public                                                        */
/**********************************************************************/
BOOL APIENTRY16 EQFPROTTABLEW
(
  PSZ_W            pszSegment,         // ptr to input segment
  PSTARTSTOP       *ppStartStop        // ptr to caller's start/stop table ptr
);

//
// EQFPRESEG2
// 
// Entry point called befor standard segmentation is called
//
EQF_BOOL APIENTRY16 EQFPRESEG2
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSource,                  // pointer to source file name
   PSZ pTempSource,              // pointer to source path
   PEQF_BOOL pfNoSegment,        // no further segmenting ??
   HWND  hwndProcWin,            // handle of analysis slider control
   PEQF_BOOL pfKill              // ptr to 'kill running process' flag/* @50A */
)
{
  EQF_BOOL fOK = TRUE;          // function return value
  int    iRC = 0;               // retur code of called API functions

  HPARSER  hParser = 0L;        // parser API handle

  FILE     *hInFile = NULL;     // input file handle
  FILE     *hOutFile = NULL;    // output file handle

  pTagTable; pEdit; hwndProcWin; pfKill; pProgPath;  // not used in this parser


  // set NoSegmentation flag as no segmentation of the analysis process is required. 
  *pfNoSegment = TRUE;          

  // initialize the parser API
  iRC = ParsInitialize( &hParser, pSource );

  // get a temporary file name for the produced output file based on the name and path of the input file
  if ( fOK )
  {
    ParsBuildTempName( pSource, pTempSource );
  } /* endif */

  // open the input file 
  if ( !iRC )
  {
    hInFile = fopen( pSource, "rb" );
    // ... some error handling could be here ...

    // check BOM
    if ( hInFile )
    {
      CHAR szBOM[2];

      fread( szBOM, 1, 2, hInFile );
		  if ( memcmp( szBOM, UNICODEFILEPREFIX, 2 ) != 0 )
      {
        iRC = ERROR_INVALID_DATA;
      } /* endif */
    }
    else
    {
      iRC = ERROR_FILE_NOT_FOUND;
    } /* endif */
  } /* endif */

  // open the output file 
  if ( !iRC )
  {
    hOutFile = fopen( pTempSource, "wb" );
    if ( hOutFile )
    {
      fwrite( UNICODEFILEPREFIX, 1, 2, hOutFile );
    } /* endif */
    // ... some error handling could be here ...
  } /* endif */

  // loop over the file until complete
  if ( fOK )
  {
    LONG lSegNum = 1;

    memset( szLine, 0, sizeof(szLine) );
    memset( &SegW, 0, sizeof(SegW) );
    fgetws( szLine, sizeof(szLine)-1, hInFile );

    while ( !feof(hInFile) )
    {
      // process current line
      if ( wcsnicmp( szLine, L"<TranslationUnit", 15 ) == 0 )
      {
        BOOL fLFPending = FALSE;
        BOOL fCRPending = FALSE;
        int iLen = 0;

        // remove trailing CRLF from segment data
        iLen = wcslen(SegW.szData);
        if ( iLen && (SegW.szData[iLen-1] == L'\n') )
        {
          SegW.szData[iLen-1] = 0;
          iLen--;
          fLFPending = TRUE;
        } /* endif */
        if ( iLen && (SegW.szData[iLen-1] == L'\r') )
        {
          SegW.szData[iLen-1] = 0;
          iLen--;
          fCRPending = TRUE;
        } /* endif */

        // write any text in segment buffer as translatable segment
        if ( SegW.szData[0] != 0 )
        {
          SegW.usLength = (USHORT)iLen;
          SegW.Status   = PARSSEG_TOBE;
          SegW.lSegNum  = lSegNum++;
          ParsMakeSGMLSegmentW( hParser, &SegW, szSegBuffer, sizeof(szSegBuffer) / sizeof(wchar_t), FALSE );
          fwrite( szSegBuffer, wcslen(szSegBuffer), sizeof(wchar_t), hOutFile );
        } /* endif */

        // write translation unit info as protected segment
        {
          int iPos = 0;
          memset( &SegW, 0, sizeof(SegW) );
          if ( fCRPending ) SegW.szData[iPos++] = L'\r';
          if ( fLFPending ) SegW.szData[iPos++] = L'\n';
          wcscpy( SegW.szData + iPos, szLine );
          SegW.usLength = (USHORT)wcslen(SegW.szData);
          SegW.Status   = PARSSEG_NOP;
          SegW.lSegNum  = lSegNum++;
          ParsMakeSGMLSegmentW( hParser, &SegW, szSegBuffer, sizeof(szSegBuffer), FALSE );
          fwrite( szSegBuffer, wcslen(szSegBuffer), sizeof(wchar_t), hOutFile );
          memset( &SegW, 0, sizeof(SegW) );
        }
      }
      else
      {
        // everythig else is added to the current segment
        USHORT usAddLen = (USHORT)wcslen( szLine );
        if ( (usAddLen + SegW.usLength + 1) < MAX_SEGMENT_SIZE )
        {
          wcscat( SegW.szData, szLine );
          SegW.usLength = SegW.usLength + usAddLen;
        } /* endif */
      } /* endif */
      
      // get next line from file
      memset( szLine, 0, sizeof(szLine) );
      fgetws( szLine, sizeof(szLine)-1, hInFile );
    } /* endwhile */

    // write any text in segment buffer as translatable segment
    if ( SegW.szData[0] != 0 )
    {
      SegW.usLength = (USHORT)wcslen(SegW.szData);
      SegW.Status   = PARSSEG_TOBE;
      SegW.lSegNum  = lSegNum++;
      ParsMakeSGMLSegmentW( hParser, &SegW, szSegBuffer, sizeof(szSegBuffer) / sizeof(wchar_t), FALSE );
      fwrite( szSegBuffer, wcslen(szSegBuffer), sizeof(wchar_t), hOutFile );
    } /* endif */
  } /* endif */


  if ( hInFile ) fclose( hInFile );
  if ( hOutFile ) fclose( hOutFile );

  // terminate parser API 
  ParsTerminate( hParser ); 

  fOK = (EQF_BOOL)(iRC == 0);

  return ( fOK );
}

//
// EQFPOSTSEGW
// 
// currently a NOP
//
EQF_BOOL APIENTRY16 EQFPOSTSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSegSource,               // pointer to source seg. file name
   PSZ pSegTarget,               // pointer to target seg file
   PTATAG_W pTATag,                 // ta tag structure
   HWND     hSlider,
   PEQF_BOOL pfKill
)
{
   pTagTable; pEdit; pProgPath; pSegSource; pSegTarget; pTATag; hSlider; pfKill;

   return ( TRUE );
}


//
// EQFPREUNSEGW
// 
// Entry point called befor standard unsegmentation is called
//
// as we do not support export in external format onöly an error message will be shown
EQF_BOOL APIENTRY16 EQFPREUNSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSegTarget,               // pointer to target seg file
   PSZ pTemp,                    // pointer to temp file
   PTATAG_W pTATag,              // ta tag structure
   PEQF_BOOL  pfNoUnseg,         // do no further unsegmentation ??
   PEQF_BOOL  pfKill             // kill flag
)
{
   pTagTable; pEdit; pProgPath; pSegTarget; pTATag; pTemp; pfKill;

   MessageBox( HWND_DESKTOP, "No external document export for XLIFF documents", "XLIFF Error", MB_CANCEL );

   *pfNoUnseg = TRUE;                 // no further unsegmentation

   return ( FALSE  );
}   

EQF_BOOL APIENTRY16 EQFPOSTUNSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pTarget,                  // pointer to target file
   PTATAG pTATag,                // ta tag structure
   PEQF_BOOL pfKill
)
{
   pTagTable; pEdit; pProgPath; pTarget; pTATag; pfKill;

   return( FALSE );
} 

PSZ_W PhTags[] = {
  L"<g", 
  L"<x", 
  L"<bx", 
  L"<ex", 
  L"<bpt", 
  L"<ept", 
  L"<ph", 
  L"<it", 
  L"<mrk", 
  L"<sub",
  NULL
};

//
// EQFPROTTABLEW
// 
BOOL APIENTRY16 EQFPROTTABLEW
(
  PSZ_W            pszSegment,         // ptr to input segment
  PSTARTSTOP       *ppStartStop        // ptr to caller's start/stop table ptr
)
{
  PSTARTSTOP       pStartStop = NULL;  // ptr to new start/stop table
  PSZ_W            pszWork;            // ptr for input segment processing
  PSZ_W            *pszPhTag;          // ptr to PhTags array
  BOOL             fOK = TRUE;         // function return code
  USHORT           usMaxEntries = 0;   // number of table entries required
  USHORT           usCurEntry = 0;     // current entry in table

  // Estimate size of start/stop table
  if ( fOK )
  {
    usMaxEntries = 4;                  // always leave some room for 'extras'
    for ( pszWork = pszSegment; *pszWork; pszWork++ )
    {
      if ( *pszWork == L'<' )
      {
        for ( pszPhTag = PhTags; *pszPhTag; pszPhTag++ )
        {
          if ( wcsnicmp( pszWork, *pszPhTag, wcslen( *pszPhTag ) ) == 0 )
          {
            usMaxEntries += 2;
          } /* endif */
        } /* endfor */
      } /* endif */
    } /* endfor */
  } /* endif */

  // Allocate start/stop table                                        
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pStartStop, 0L, (LONG) max( (usMaxEntries * sizeof(STARTSTOP)), MIN_ALLOC), NOMSG );
  } /* endif */

  // Fill start/stop table                                            
  if ( fOK )
  {
    pszWork = pszSegment;              // start at begin of segment
    usCurEntry = 0;                    // start with first table entry
    pStartStop[usCurEntry].usType  = UNPROTECTED_CHAR;
    pStartStop[usCurEntry].usStart = 0;

    while ( *pszWork )                 // while not end of input data ...
    {
      BOOL fFoundTag = FALSE;
      if ( *pszWork == L'<' )
      {
        for ( pszPhTag = PhTags; !fFoundTag && *pszPhTag; pszPhTag++ )
        {
          if ( wcsnicmp( pszWork, *pszPhTag, wcslen( *pszPhTag ) ) == 0 )
          {
            PSZ_W pszStart = pszWork;
            BOOL fEnd = FALSE;
            fFoundTag = TRUE;

            // find end of inline tagging
            while ( *pszWork && !fEnd )
            {
              if ( *pszWork == L'<' && *(pszWork+1) == L'/' )
              {
                if ( wcschr( pszWork, L'>' ) )
                {
                  fEnd = TRUE;
                  pszWork = wcschr( pszWork, L'>' );
                } /* endif */
              } /* endif */

              if ( !fEnd )
              {
                pszWork++;
              } /* endif */
            } /*endwhile */

            // add all characters until end of tagging as protected characters
            while ( pszStart < pszWork )
            {
              AddToTable( pStartStop, &usCurEntry, usMaxEntries, (SHORT)(pszStart - pszSegment), PROTECTED_CHAR );
              pszStart++;
            } /*endwhile */
          } /* endif */
        } /* endfor */
      }
      if ( !fFoundTag )
      {
        AddToTable( pStartStop, &usCurEntry, usMaxEntries, (SHORT)(pszWork - pszSegment), UNPROTECTED_CHAR );
        pszWork++;
      } /* endif */
    } /* endwhile */

    // terminate start/stop table                                     
    usCurEntry++;
    pStartStop[usCurEntry].usStart = 0;
    pStartStop[usCurEntry].usStop  = 0;
    pStartStop[usCurEntry].usType  = 0;
  } /* endif */

  *ppStartStop = pStartStop;

  return( fOK );
} /* end of function EQFPROTTABLEW */

//
// AddToTable
//
// Adds given character to a start/stop table and starts new table entries if type of entry has changed.
//
static void AddToTable
(
  PSTARTSTOP       pStartStop,         // ptr to start/stop table
  PUSHORT          pusCurEntry,        // ptr to number of current table entry
  USHORT           usMaxEntries,       // max number of table entries
  USHORT           usCurOffs,          // current offset of character
  USHORT           usType              // type of character
)
{
  PSTARTSTOP       pCurEntry;          // ptr to current table entry

  pCurEntry = pStartStop + *pusCurEntry;

  if ( pCurEntry->usType != usType )
  {
    // type of entry changes, complete current entry and start a new one (but only if current entry is not empty                    
    if ( pCurEntry->usStart != usCurOffs )
    {
      // Entry is not empty, so start a new one if there is still room in the start/stop table
      if ( (*pusCurEntry + 2) < usMaxEntries )
      {
        *pusCurEntry = *pusCurEntry + 1;
        pCurEntry++;
      } /* endif */
    } /* endif */
    pCurEntry->usStart = usCurOffs;
    pCurEntry->usStop  = usCurOffs;
    pCurEntry->usType  = usType;
  }
  else
  {
    pCurEntry->usStop  = usCurOffs;
  } /* endif */
} /* end of function AddToTable */

