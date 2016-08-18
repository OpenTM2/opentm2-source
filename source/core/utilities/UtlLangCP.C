//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#include <tchar.h>
#include "eqf.h"

BOOL IsDBCS_CP( ULONG ulCP )
{
  BOOL   fIsDBCS = FALSE;

  switch(ulCP)
  {
//    case 874:                          // Thai  -- in TP5.5.2 under Unicode not necessary anymore
    case 932:                          // Japanese
    case 943:                          // Japanese2
    case 936:                          // Simplified Chinese
    case 949:                          // Korean
    case 950:                          // Chinese (Traditional)
    case 1351:                         //
      fIsDBCS = TRUE;
      break;
  } /* endswitch */

  return(fIsDBCS);
}

// check if the codepage for the given language is supported by the operating system
BOOL UtlIsLanguageSupported
(
  PSZ    pszLanguage,
  BOOL   fMsg,
  HWND   hwndErrMsg
)
{
  CHAR_W szUnicode[10];
  int    iRC = 0;
  BOOL   fOK = TRUE;

  // get OEM codepage for given language
  ULONG ulCP = GetLangOEMCP( pszLanguage );

  fMsg;
  hwndErrMsg;

  // special handling for CP 943
  if ( ulCP == 943 )
  {
    ulCP = 932;
  } /* endif */

  // convert our test string to UTF16
  iRC = MultiByteToWideChar( ulCP, 0, "ABC", -1, szUnicode, 10 );

  if ( iRC == 0 )
  {
    iRC = GetLastError();
    fOK = (iRC != ERROR_INVALID_PARAMETER);
  } /* endif */

  // error handling moved to caller functions
//  // do any error handling
//  if ( !fOK && fMsg )
//  {
//    UtlErrorHwnd( ERROR_LANG_NOTSUPPORTED, MB_CANCEL, 1, &pszLanguage, EQF_ERROR, hwndErrMsg );
//  } /* endif */

  return( fOK );
} /* end of function UtlIsLanguageSupported */
