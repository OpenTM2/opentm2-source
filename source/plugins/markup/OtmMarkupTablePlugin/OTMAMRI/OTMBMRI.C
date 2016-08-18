//+----------------------------------------------------------------------------+
//|OTMBMRI.C                           MAT Tools Parser for MRI format files   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2013, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:        G. Queck (QSoft)                                             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:   This module contains all functions necessary to build the    |
//|               MRI parser for the translation processor.                    |
//|               It is invoked via the Text Analysis UserExit function        |
//|               The comment characteristics (etc.) uses the standard 'C'     |
//|               logic supporting ( '//' and '/*' and '*/' )                  |
//|               See EQFBMRI.H for the appropriate defines                    |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|   EQFPRESEG2      - the preparser                                          |
//|   EQFPOSTSEG      - the postparser ( NOP until now )                       |
//|   EQFPREUNSEG     - the preparser during unsegmentation ( NOP until now )  |
//|   EQFPOSTUNSEG    - the postparser during unsegmentation                   |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_EDITORAPI        // editor API

// use import DLL defines for dbcs_cp ...
#define DLLIMPORTDBCSCP

#include <eqf.h>                  // General Translation Manager include file

#include "eqfparse.h"                  // headers used


#ifdef __cplusplus
extern "C"
#endif
extern void   __cdecl  GetOTMTablePath( char * basename, char * OTMname ) ;

#define INBUF_SIZE         8192        // size of input buffer

#define FONT_STR              "FONT"
#define FONT_STR_W            L"FONT"
#define CONTROL_STR           "CONTROL"
#define CONTROL_STR_W         L"CONTROL"
#define INCLUDE_STR           "#INCLUDE "
#define INCLUDE_STR_W         L"#INCLUDE "
#define SETTINGS_EXTENSION   ".CHR"
#define UNDERSCORE_CHAR_W  L'_'
#define UNDERSCORE_CHAR  '_'

#define LF_W               L'\n'       // a linefeed ...
#define CR_W               L'\r'       // a carriage retu

#define NO_BLANK               0
#define BLANK_PENDING_FOR_PARM 1
#define PARM_COUNTED           2
#define KOMMA_NEEDED           3

USHORT ParseMri( PSZ pszSource, PSZ pszTarget, HWND hwndSlider,
                 PEQF_BOOL pfKill, PSZ pszTagTable );

static PSZ FindString ( PSZ  pString1, PSZ  pSearchString );
static PSZ FindStringNotInQuotes( PSZ pString1, PSZ pSearchString );
static BOOL UnparseMRI ( PSZ pszInFile, ULONG ulTgtOemCP );

// W functions for UTF16 file
typedef EQF_BOOL (*PFNEQFFILECONVERSION) ( PSZ, PSZ, PSZ, BOOL );
USHORT ParseMriW( PSZ pszSource, PSZ pszTarget, HWND hwndSlider,
                 PEQF_BOOL pfKill, PSZ pszTagTable, PFNEQFFILECONVERSION pfnEQFFileConversion);

static PSZ_W FindStringW ( PSZ_W  pString1, PSZ_W  pSearchString );
static PSZ_W FindStringNotInQuotesW( PSZ_W pString1, PSZ_W pSearchString );
static BOOL UnparseMRI ( PSZ pszInFile, ULONG ulTgtOemCP );

static    CHAR_W  chOutSideStringW[ 2048 ];          // global data
static    CHAR    chOutSideString[ 2048 ];          // global data

BOOL  fInit = FALSE;                   // indicator for DBCS initialisation

/**********************************************************************/
/* Structure for flags used during MRI parsing                        */
/**********************************************************************/
typedef struct _MRIPARSFLAGS
{
  BOOL        fSingleQuotes;           // TRUE = handle strings in single quotes
  BOOL        fDoubleQuotes;           // TRUE = handle strings in double quotes
  BOOL        fAnsi;                   // TRUE = ANSI <> ASCII conversion required
  BOOL        fUTF8;                   // TRUE = UTF8 o UTF16 conversion required
  BOOL        fMriKeywords;            // TRUE = protect MRI keywords
} MRIPARSEFLAGS, *PMRIPARSEFLAGS;

static USHORT MriGetSettings( PSZ pszTagTable, PMRIPARSEFLAGS pFlags );
// new W functions working on UTF16 file
static VOID   MriAddToLineBufW(PSZ_W pLineString,USHORT usLen,
                              CHAR_W bCurrent, PUSHORT pusInLine);
static USHORT MriHandleControlStringW ( PPARSEDATAW, CHAR_W, PSZ_W, USHORT,
                                       PUSHORT, PMRIPARSEFLAGS );
static USHORT MriHandleFontStringW ( PPARSEDATAW, CHAR_W, PSZ_W, USHORT,
                                       PUSHORT, PMRIPARSEFLAGS );
static USHORT MriCountTokW ( PSZ_W );
static VOID   MriRemoveFromLineBufW(PSZ_W pLineString,
                              USHORT usCount, PUSHORT pusInLine);
// original functions working on Ansi file
// needed for downward compatibility: (TM601 and older)
// EQFFILECONVERSIONEX API is only available for TM602 and upwards
static VOID   MriAddToLineBuf(PSZ pLineString,USHORT usLen,
                              CHAR bCurrent, PUSHORT pusInLine);
static USHORT MriHandleControlString ( PPARSEDATA, CHAR, PSZ, USHORT,
                                       PUSHORT, PMRIPARSEFLAGS );
static USHORT MriHandleFontString ( PPARSEDATA, CHAR, PSZ, USHORT,
                                       PUSHORT, PMRIPARSEFLAGS );
static USHORT MriCountTok ( PSZ );
static VOID   MriRemoveFromLineBuf(PSZ pLineString,
                              USHORT usCount, PUSHORT pusInLine);


//+----------------------------------------------------------------------------+
//| EQFBPRESEG2      - presegment parser                                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Invoked from text analysis prior to segmentation                        |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - setup a temporary file name for the segmented file (ext. $$$ )   |
//|         - call ParseMRI                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ pTagTable,                -- name of tag table                        |
//|  PSZ pEdit,                    -- name of editor dll                       |
//|  PSZ pProgPath,                -- pointer to program path                  |
//|  PSZ pSource,                  -- pointer to source file name              |
//|  PSZ pTempSource,              -- pointer to source path                   |
//|  PBOOL pfNoSegment             -- no further segmenting ??                 |
//|  HWND  hwndSlider              -- handle of slider window                  |
//|  PBOOL pfKill                  -- ptr to 'kill running process' flag       |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  BOOL   fOK       TRUE           - success                                 |
//|                   FALSE          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPRESEG2
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSource,                  // pointer to source file name
   PSZ pTempSource,              // pointer to source path
   PEQF_BOOL pfNoSegment,        // no further segmenting ??
   HWND  hwndSlider,             // handle of analysis slider control
   PEQF_BOOL pfKill              // pointer to kill flag              /* @50A */
)
{
   PSZ           pName;                    // pointer to file name
   USHORT        usRC = NO_ERROR;
   HMODULE       hmodDll;
   PFNEQFFILECONVERSION pfnEQFFileConversion;
   USHORT        usNewAPIRC = NO_ERROR;
   CHAR          szTempFile[50];  // buffer for temporary file name

   pTagTable;                    // get rid of compiler warning
   pEdit;
   pProgPath;

   *pfNoSegment = TRUE;          // no further segmentation requested
   strcpy( pTempSource, pSource );
   pName = UtlGetFnameFromPath( pTempSource );

   // find extension and change it into temp extension '$$$'
   while ( *pName && *pName != '.')
   {
      pName ++;
   } /* endwhile */

   if ( !*pName )
   {
      *pName = '.';
   } /* endif */
   pName++;
   memset( pName, '$', 3 );
   *(pName+3) = EOS;                      // include end of string

   pfnEQFFileConversion = NULL;
   if (!usRC)
   {
         strcpy(szTempFile,"OTMAPI.DLL");
         usRC = DosLoadModule( NULL, 0 , szTempFile, &hmodDll );

         if ( usRC == NO_ERROR )
         {
             usNewAPIRC = DosGetProcAddr( hmodDll, "EQFFILECONVERSIONEX",
                                    (PFN*) (&pfnEQFFileConversion));
         }
   } /* endif */
   if (usRC)
   {
        return(!usRC);
   }
   else
   {
     if (!pfnEQFFileConversion )
     {
		 return (! ParseMri( pSource, pTempSource, hwndSlider,
             pfKill, pTagTable ) );
     }
     else
     {
         return (! ParseMriW( pSource, pTempSource, hwndSlider,
             pfKill, pTagTable, pfnEQFFileConversion ) );
     }
   } /* endif */
}


//+----------------------------------------------------------------------------+
//| EQFBPOSTSEG      - postsegment parser                                      |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    invoked from text analysis after segmentation                           |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         currently a NOP                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ pTagTable,                                                            |
//|  PSZ pEdit,                    -- name of editor dll                       |
//|  PSZ pProgPath,                -- pointer to program path                  |
//|  PSZ pSegSource,               -- pointer to source seg. file name         |
//|  PSZ pSegTarget,               -- pointer to target seg file               |
//|  PTATAG pTATag                 -- ta tag structure                         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  BOOL   fOK       TRUE           - success                                 |
//|                   FALSE          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSegSource,               // pointer to source seg. file name
   PSZ pSegTarget,               // pointer to target seg file
   PTATAG_W pTATag,                 // ta tag structure
   HWND     hSlider,
   PEQF_BOOL  pfKill
)
{
   pTagTable; pEdit; pProgPath; pSegSource; pSegTarget; pTATag; hSlider; pfKill;

   return ( TRUE );
}

//+----------------------------------------------------------------------------+
//| EQFBPREUNSEG     - preunsegment parser                                     |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    invoked from unsegment utility prior to unsegmentation                  |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         currently a NOP                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ pTagTable,                                                            |
//|  PSZ pEdit,                    // name of editor dll                       |
//|  PSZ pProgPath,                // pointer to program path                  |
//|  PSZ pSegTarget,               // pointer to target seg file               |
//|  PSZ pTemp,                    // pointer to temp file                     |
//|  PTATAG pTATag,                // ta tag structure                         |
//|  PBOOL  pfNoUnseg              // do no further unsegmentation ??          |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  BOOL   fOK       TRUE           - success                                 |
//|                   FALSE          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPREUNSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSegTarget,               // pointer to target seg file
   PSZ pTemp,                    // pointer to temp file
   PTATAG_W pTATag,                // ta tag structure
   PEQF_BOOL  pfNoUnseg,          // do no further unsegmentation ??
   PEQF_BOOL  pfKill
)
{
   pTagTable; pEdit; pProgPath; pSegTarget; pTATag; pTemp;
   pfNoUnseg; pfKill;
   return ( TRUE );
}

//+----------------------------------------------------------------------------+
//| EQFBPOSTUNSEG     - postunsegment parser                                   |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    invoked from unsegment utility after unsegmentation                     |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         currently a NOP                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ pTagTable,                                                            |
//|  PSZ pEdit,                    // name of editor dll                       |
//|  PSZ pProgPath,                // pointer to program path                  |
//|  PSZ pSegTarget,               // pointer to target seg file               |
//|  PSZ pTemp,                    // pointer to temp file                     |
//|  PTATAG pTATag,                // ta tag structure                         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  BOOL   fOK       TRUE           - success                                 |
//|                   FALSE          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTUNSEG
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pTarget,                  // pointer to target file
   PTATAG pTATag                 // ta tag structure
)
{
  MRIPARSEFLAGS Flags;                 // flags for parsing
  ULONG         ulTgtOemCP = 0L;
  USHORT        usRC = NO_ERROR;

  pTagTable; pEdit; pProgPath; pTarget; pTATag;

  // Get MRI Parser settings from .CHR file
  memset( &Flags, 0, sizeof(Flags) );
  MriGetSettings( pTagTable, &Flags );

  usRC = ParseGetCP( pTarget, NULL, &ulTgtOemCP, NULL, NULL );
  if (!usRC )
  {
    if ( !Flags.fAnsi || IsDBCS_CP(ulTgtOemCP) )
    {
      // no conversion in DBCS environment or for ASCII documents
      return( TRUE );
    }
    else
    {
       return ((EQF_BOOL)( UnparseMRI( pTarget, ulTgtOemCP ) ));
    } /* endif */
  }
  else
  {
      return(!usRC);
  }
} /* end of function EQFBPOSTUNSEG */

__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTUNSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pTarget,                  // pointer to target file
   PTATAG pTATag,                // ta tag structure
   PEQF_BOOL pfKill
)
{
  MRIPARSEFLAGS Flags;                 // flags for parsing
  ULONG         ulTgtOemCP = 0L;
  USHORT        usRC = NO_ERROR;
  char          szTgtLanguage[50];
  char          szFolder[MAX_LONGPATH ];
  char          szFileName[MAX_LONGPATH ];
  CHAR          szTempFile[MAX_LONGPATH];  // buffer for temporary file name
  PSZ           pszFileName = NULL;
  HMODULE       hmodDll;
  PFNEQFFILECONVERSION pfnEQFFileConversion;
  USHORT        usNewAPIRC = NO_ERROR;
  PSZ           pName;

  pTagTable; pEdit; pProgPath; pTarget; pTATag; pfKill;

  // Get MRI Parser settings from .CHR file
  memset( &Flags, 0, sizeof(Flags) );
  MriGetSettings( pTagTable, &Flags );

  pfnEQFFileConversion = NULL;
  if (!usRC)
  {
      strcpy(szTempFile,"OTMAPI.DLL");
      usRC = DosLoadModule( NULL, 0 , szTempFile, &hmodDll );

      if ( usRC == NO_ERROR )
      {
          usNewAPIRC = DosGetProcAddr( hmodDll, "EQFFILECONVERSIONEX",
                                 (PFN*) (&pfnEQFFileConversion));
      }
  } /* endif */
  if (usRC)
  {
     return(!usRC);
  }
  else
  {
	if (!pfnEQFFileConversion )
    { // use old unparseMRI
        usRC = ParseGetCP( pTarget, NULL, &ulTgtOemCP, NULL, NULL );
        if ( !Flags.fAnsi || IsDBCS_CP(ulTgtOemCP) )
	    {
	      // no conversion in DBCS environment or for ASCII documents
	      return( TRUE );
	    }
	    else
	    {
	       return ((EQF_BOOL)( UnparseMRI( pTarget, ulTgtOemCP ) ));
        } /* endif */
    }
    else
    {
	     strcpy( szTempFile, pTarget );
		 UtlSplitFnameFromPath( szTempFile ); // get rid off document name
		 UtlSplitFnameFromPath( szTempFile ); // get rid off document name

		 strcpy( szFolder, szTempFile );

		 strcpy( szTempFile, pTarget );
		 pszFileName = UtlSplitFnameFromPath( szTempFile);
		 strcpy(szFileName, pszFileName );

		 EQFGETTARGETLANG(szFolder, szFileName, szTgtLanguage);

         // convert input file to UTF16
       /*******************************************************************/
       /* find extension and change it into temp extension '$$$'          */
       /*******************************************************************/
       strcpy( szTempFile, pTarget );
       pName = UtlGetFnameFromPath( szTempFile );
       while ( *pName && *pName != '.' )
       {
          pName ++;
       } /* endwhile */

       if ( !*pName )
       {
          *pName = '.';
       } /* endif */
       pName++;

       if ( strcmp( pName, "$$$" ) == 0 )
       {
         memset( pName, '@', 3 );
       }
       else
       {
         memset( pName, '$', 3 );
       } /* endif */
       *(pName+3) = EOS;                      // include end of string

	   if ( Flags.fAnsi )
	   {
		   usRC = pfnEQFFileConversion(pTarget, szTempFile, szTgtLanguage, EQF_UTF162ANSI);
	   }
	   else
	   {
		   usRC = pfnEQFFileConversion(pTarget, szTempFile, szTgtLanguage, EQF_UTF162ASCII);
	   } /* endif */
	   /*******************************************************************/
       /* Delete old file and rename temp file                            */
       /*******************************************************************/
       if ( !usRC )
       {
	       UtlDelete( pTarget, 0L, FALSE );
	       usRC = UtlMove( szTempFile, pTarget, 0L, FALSE );
       } /* endif */

        return( usRC == 0 );
     } /* endif */
  } /* endif */

} /* end of function EQFBPOSTUNSEGW */

//+----------------------------------------------------------------------------+
//| ParseMriW         - parse a MRI file                                        |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Parses an input file and creates a segmented output file.               |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - setup global data and tag names                                  |
//|         - open input and output file                                       |
//|         - enclose strings of input file in QFF/EQFF tags, enclose          |
//|           all other stuff in QFN/EQFN tags and write data to output file   |
//|         - close input and output file                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ_W       pszSource             - fully qualified name of source file     |
//|  PSZ_W       pszTarget             - fully qualified name of target file     |
//|  HWND      hwndSlider            - handle of slider window                 |
//|  PBOOL     pfKill                - pointer to kill flag                    |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
USHORT ParseMriW
(
   PSZ pszSource,                      // fully qualified name of source file
   PSZ pszTarget,                      // fully qualified name of target file
   HWND hwndSlider,                    // handle of slider window
   PEQF_BOOL pfKill,                   // pointer to kill flag              /* @50A */
   PSZ pszTagTable,                     // name of tag table
   PFNEQFFILECONVERSION pfnEQFFileConversion
)
{
   USHORT      usRC = 0;               // function return code
   USHORT      usOpenAction;           // action performed by DosOpen
   FILESTATUS  stStatus;               // File status information
   CHAR_W      chCurrent;               // currently processed byte
   CHAR_W      chNext   ;               // next byte
   USHORT      usInLine = 0;           // position in line
   PPARSEDATAW  pParsDataW = NULL;       // points to parser data structure
   CHAR_W        chCurStringDelim = SPACE;// currently active string delimiter

   MRIPARSEFLAGS Flags;                // flags for parsing

   CHAR chUTF16InFile[MAX_EQF_PATH+1];
   /******************************************************************/
   /* create temp file names -- we have to use two different output  */
   /******************************************************************/
   strcpy( chUTF16InFile, pszSource );
   strcat( chUTF16InFile, "B" );


   UtlAlloc((PVOID *) &pParsDataW, 0L, (LONG)sizeof(PARSEDATAW), ERROR_STORAGE );
   if ( pParsDataW )
   {
     ParseInitW( pParsDataW, hwndSlider, pfKill );
     usRC = ParseFillCPW( pParsDataW, pszSource);
   }
   else
   {
     usRC = ERROR_STORAGE;
   } /* endif */

   memset( &Flags, 0, sizeof(Flags) );

   // get settings from .CHR file or use defaults
   if ( !usRC && !*(pParsDataW->pfKill) )                              /* @50C */
   {
     MriGetSettings( pszTagTable, &Flags );
     if ( !Flags.fDoubleQuotes && !Flags.fSingleQuotes )
     {
       Flags.fDoubleQuotes = TRUE;
     } /* endif */
   } /* endif */
// convert input file to UTF16
   if (!usRC)
   {
     if ( Flags.fAnsi )
     {
         usRC = (USHORT) pfnEQFFileConversion(pszSource, chUTF16InFile, pParsDataW->szLanguage, EQF_ANSI2UTF16);
     }
     else
     {
	     usRC = (USHORT) pfnEQFFileConversion(pszSource, chUTF16InFile, pParsDataW->szLanguage, EQF_ASCII2UTF16);
     } /* endif */
   } /* endif */

   // open input file
   if ( !usRC && !*(pParsDataW->pfKill) )                              /* @50C */
   {
      usRC = UtlOpen( chUTF16InFile,
                      &(pParsDataW->hInFile),
                      &usOpenAction, 0L,
                      FILE_NORMAL,
                      FILE_OPEN,
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                      0L,
                      TRUE );
   } /* endif */

   // open output file
   if ( !usRC && !*(pParsDataW->pfKill) )                              /* @50C */
   {
      usRC = UtlOpen( pszTarget,
                      &(pParsDataW->hOutFile),
                      &usOpenAction, 0L,
                      FILE_NORMAL,
                      FILE_TRUNCATE | FILE_CREATE,
                      OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                      0L,
                      TRUE );
   } /* endif */
   if ( !usRC )
   {
         // write Unicode prefix to output file
         ULONG ulBytesWritten = 0;
         UtlWriteL( pParsDataW->hOutFile, UNICODEFILEPREFIX,
                    (ULONG)strlen(UNICODEFILEPREFIX), &ulBytesWritten, TRUE );
   } /* endif */

   // get size of input file
   if ( !usRC && !*(pParsDataW->pfKill) )                              /* @50C */
   {
      usRC = UtlQFileInfo( pParsDataW->hInFile,
                           1,
                           (PBYTE)&stStatus,
                           sizeof(FILESTATUS),
                           TRUE );
      pParsDataW->lBytesToRead = stStatus.cbFile;
      pParsDataW->lTotalBytes  = stStatus.cbFile;

   } /* endif */
   if (!usRC)
   {
	 ULONG ulBytesInBuffer = 0;
	 ULONG ulTemp = 0;
	 PSZ pData = (PSZ)(pParsDataW->abInBuf);
	 PSZ pszPrefix = UNICODEFILEPREFIX;
	 int iLen = strlen(pszPrefix);
     usRC = UtlReadL( pParsDataW->hInFile, pData, 8, &ulBytesInBuffer, FALSE );
     if (!usRC)
     {
		  if ( (memcmp( pData, pszPrefix, iLen ) == 0) )
		  {
		 	// position right behind prefix
		    usRC = UtlChgFilePtr( pParsDataW->hInFile, iLen, FILE_BEGIN, &ulTemp, FALSE );
		    pParsDataW->lBytesToRead -= iLen;
		    pParsDataW->lTotalBytes -= iLen;
		  }
     }
   } /* endif */

   if ( !usRC && !*(pParsDataW->pfKill) )
   {
      StartSegmentW( pParsDataW );

      do {
         chCurrent = ParseNextCharW( pParsDataW, &usRC );

         // change a HEX 0 into a BLANK
         if ( chCurrent == EOS )  chCurrent = BLANK;

         if ( !usRC && !*(pParsDataW->pfKill) )                        /* @50C */
         {
			if ((pParsDataW->Mode != LINE_COMMENT_MODE) &&
			    (pParsDataW->Mode != BLOCK_COMMENT_MODE) &&
			     (pParsDataW->Mode != COMMENT_END_MODE))
			{
				// P018267: do not add comments to LineBuf!
              MriAddToLineBufW(chOutSideStringW, sizeof(chOutSideStringW)/ sizeof(CHAR_W),
                            chCurrent, &usInLine);
		    }
            switch ( pParsDataW->Mode )
            {
               case COMMENT_START_MODE:
                  // Switch to LINE_COMMENT_MODE, BLOCK_COMMENT_MODE or
                  // OUTSIDE_STRING_MODE depending on current character
                  switch ( chCurrent )
                  {
                     case BLOCK_COMMENT:
                        pParsDataW->Mode = BLOCK_COMMENT_MODE;
                        MriRemoveFromLineBufW(chOutSideStringW,
                                              2, &usInLine);
                        break;
                     case LINE_COMMENT:
                        pParsDataW->Mode = LINE_COMMENT_MODE;
                        MriRemoveFromLineBufW(chOutSideStringW,
                                              2, &usInLine);
                        break;
                     default:
                        pParsDataW->Mode = OUTSIDE_STRING_MODE;
                        break;
                  } /* endswitch */
                  // add current char to active segment
                  usRC = AddToSegmentW( pParsDataW, chCurrent );
                  break;

               case LINE_COMMENT_MODE:
                  // Add bytes to active segment until end of line is reached.
                  // At at end of line switch back to OUTSIDE_STRING_MODE
                  if ( (chCurrent == LF_W) || (chCurrent == CR_W) )
                  {
                     pParsDataW->Mode = OUTSIDE_STRING_MODE;
                     // P018267: add linefeed to buffer
                     MriAddToLineBufW(chOutSideStringW, sizeof(chOutSideStringW)/ sizeof(CHAR_W),
                            chCurrent, &usInLine);
                  } /* endif */
                  usRC = AddToSegmentW( pParsDataW, chCurrent );
                  break;

               case BLOCK_COMMENT_MODE:
                  // Add bytes to active segment until may be end of comment
                  // block is reached.
                  // At at end of block switch to COMMENT_END_MODE
                  if ( chCurrent == BLOCK_COMMENT )
                  {
                    if ( (chNext = ParseNextCharW( pParsDataW, &usRC )) == LINE_COMMENT )
                    {
                      pParsDataW->Mode = COMMENT_END_MODE;
                    } /* endif */
                    UndoNextCharW( pParsDataW, chNext );
                  } /* endif */
                  usRC = AddToSegmentW( pParsDataW, chCurrent );
                  break;

               case COMMENT_END_MODE:
                  // Switch to BLOCK_COMMENT_MODE or OUTSIDE_STRING_MODE
                  // depending on current character
                  if ( chCurrent == START_COMMENT )
                  {
                     pParsDataW->Mode = OUTSIDE_STRING_MODE;
                  }
                  else
                  {
                     pParsDataW->Mode = BLOCK_COMMENT_MODE;
                  } /* endif */
                  usRC = AddToSegmentW( pParsDataW, chCurrent );
                  break;

               case STRING_MODE:
                  // Add bytes to active segment until end string delimiter is
                  // reached or a BACKSLASH is encountered.
                  // At end of string terminate current segment, switch to
                  // OUTSIDE_STRING_MODE and start a new segment.
                  // At BACKSLASH switch to ESCAPE_CHAR_MODE to handle
                  // escape characters.
                  switch ( chCurrent )
                  {
                     case BACKSLASH:
                        pParsDataW->Mode = ESCAPE_CHAR_MODE;
                        usRC = AddToSegmentW( pParsDataW, chCurrent );
                        break;
                     case STRING_DELIMITER:
                     case QUOTE_DELIMITER:
                        if ( chCurrent == chCurStringDelim )
                        {
                          chNext = ParseNextCharW( pParsDataW, &usRC );
                          if ( usRC == EOF_REACHED )
                          {
                            // no more characters to follow, close segment, start
                            // a new one for the end delimiter
                            usRC = EndSegmentW( pParsDataW );
                            pParsDataW->Mode = OUTSIDE_STRING_MODE;
                            if ( !usRC )
                            {
                              usRC = StartSegmentW( pParsDataW );
                            } /* endif */
                            if ( !usRC )
                            {
                              usRC = AddToSegmentW( pParsDataW, chCurrent );
                            } /* endif */
                          }
                          else if ( !usRC )
                          {
                            if (chNext == chCurStringDelim)
                            {
                              usRC = AddToSegmentW( pParsDataW, chCurrent );
                              if ( !usRC )
                              {
                                usRC = AddToSegmentW( pParsDataW, chNext );
                              } /* endif */
                            }
                            else
                            {
                              UndoNextCharW( pParsDataW, chNext );
                              usRC = EndSegmentW( pParsDataW );
                              pParsDataW->Mode = OUTSIDE_STRING_MODE;
                              if ( !usRC )
                              {
                                usRC = StartSegmentW( pParsDataW );
                              } /* endif */
                              if ( !usRC )
                              {
                                usRC = AddToSegmentW( pParsDataW, chCurrent );
                              } /* endif */
                            } /* endif */
                          } /* endif */
                        }
                        else
                        {
                          // treat as normal character within the string
                          usRC = AddToSegmentW( pParsDataW, chCurrent );
                        } /* endif */
                        break;
                     default:
                        usRC = AddToSegmentW( pParsDataW, chCurrent );
                        break;
                  } /* endswitch */
                  break;

               case ESCAPE_CHAR_MODE:
                  // Add current character to segment without any checks.
                  // Switch back to STRING_MODE.
                  /****************************************************/
                  /* check if we are at the end of the string         */
                  /* might happen in case the backslash is a          */
                  /* 2nd byte of a double byte                        */
                  /* in such cases treat the backslash as a backslash */
                  /****************************************************/
                  if ( chCurrent == STRING_DELIMITER )
                  {
                    chNext = ParseNextCharW( pParsDataW, &usRC );
                    if ( !usRC )
                    {
                      if (  IsDBCS_CP(pParsDataW->ulSrcOemCP)
                             && ((chNext == CR_W) || (chNext == LF_W)) )
                      {
                        CHAR_W chNext1 = ParseNextCharW( pParsDataW, &usRC ) ;               // next byte
                        if ( !usRC && chNext1 == LF_W )
                        {
                          usRC = AddToSegmentW( pParsDataW, chCurrent );

                          UndoNextCharW( pParsDataW, chNext1 );  // undo LF
                          UndoNextCharW( pParsDataW, chNext );  // undo CR
                        }
                        else
                        {
                          UndoNextCharW( pParsDataW, chNext1 );  // undo LF
                          UndoNextCharW( pParsDataW, chNext );  // undo CR
                          UndoNextCharW( pParsDataW, chCurrent );  // undo STRING_END
                        }
                      }
                      else
                      {
                        usRC = AddToSegmentW( pParsDataW, chCurrent );
                        UndoNextCharW( pParsDataW, chNext );  // undo character
                      } /* endif */
                    } /* endif */
                  }
                  else
                  {
                    usRC = AddToSegmentW( pParsDataW, chCurrent );
                  } /* endif */
                  pParsDataW->Mode = STRING_MODE;
                  break;

               case OUTSIDE_STRING_MODE:
                  // Add bytes to active segment until a comment start or a
                  // string delimiter is found.
                  // Switch to COMMENT_START_MODE or STRING_MODE depending
                  // on character found.
                  if ( chCurrent == LF_W )
                  {
                    usRC = AddToSegmentW( pParsDataW, chCurrent );

                    if ( !usRC )
                    {
                      // start new segment only if not at end of file ( KBT0974)
                      chNext = ParseNextCharW( pParsDataW, &usRC );
                      if (usRC != EOF_REACHED )
                      {
                           usRC = EndSegmentW( pParsDataW );
                           if (!usRC)
                           {
                             usRC = StartSegmentW( pParsDataW );
                           }
                       }
                       UndoNextCharW( pParsDataW, chNext );
                    } /* endif */
                  }
                  else if ( chCurrent == START_COMMENT )
                  {
                    pParsDataW->Mode = COMMENT_START_MODE;
                    usRC = AddToSegmentW( pParsDataW, chCurrent );
                  }
                  else if ( (Flags.fDoubleQuotes && (chCurrent == STRING_DELIMITER) ) ||
                            (Flags.fSingleQuotes && (chCurrent == QUOTE_DELIMITER) ) )
                  {
                   chCurStringDelim = chCurrent;
                   usRC = AddToSegmentW( pParsDataW, chCurrent );
                   //  ignore empty strings, i.e. check if next char is
                   //    string delimiter, too.
                   if ( !usRC )
                   {
                     chCurrent = ParseNextCharW( pParsDataW, &usRC );
                   } /* endif */
                   if ( !usRC )
                   {
                     CHAR_W chNextChar = ParseNextCharW( pParsDataW, &usRC );

                     UndoNextCharW( pParsDataW, chNextChar );

                     if ( (chCurrent != chCurStringDelim) || (chNextChar == chCurStringDelim) )
                     {
                        USHORT    usStartInLine;
                        PSZ_W       pTemp;
                        UndoNextCharW( pParsDataW, chCurrent );
                        usStartInLine = usInLine;          // temp save index

                        usRC = MriHandleControlStringW(pParsDataW,
                                               chCurStringDelim,
                                               chOutSideStringW,
                                               sizeof(chOutSideStringW)/ sizeof(CHAR_W),
                                               &usInLine,
                                               &Flags );
                        // if tgtlang is DBCS Font statement must be translatable!
                        // RJ 040218: if either src or tgtlang is DBCS, Font must be translatable
                        if ((usStartInLine == usInLine) &&
                            (!IsDBCS_CP(pParsDataW->ulTgtOemCP)) &&
                            (!IsDBCS_CP(pParsDataW->ulSrcOemCP)) )
                        {
                          MriHandleFontStringW(pParsDataW,
                                            chCurStringDelim,
                                            chOutSideStringW,
                                            sizeof(chOutSideStringW)/ sizeof(CHAR_W),
                                            &usInLine,
                                            &Flags );
                        } /* endif */
                        if (usStartInLine == usInLine )
                        {
                          usRC = EndSegmentW( pParsDataW );
                          /****************************************/
                          /* remain in outside string mode if     */
                          /* include is pending ...               */
                          /****************************************/

                          pTemp = FindStringW( chOutSideStringW, INCLUDE_STR_W );
                          if (!(*pTemp) )
                          {
                            pParsDataW->Mode = STRING_MODE;
                          } /* endif */
                          if ( !usRC )
                          {
                            usRC = StartSegmentW( pParsDataW );
                          } /* endif */
                          /****************************************/
                          /* set new string mode in any case ...  */
                          /****************************************/
                          pParsDataW->Mode = STRING_MODE;
                        } /* endif */
                     }
                     else
                     {
                        MriAddToLineBufW(chOutSideStringW,
                                        sizeof(chOutSideStringW)/ sizeof(CHAR_W),
                                        chCurrent, &usInLine);
                        usRC = AddToSegmentW( pParsDataW, chCurrent );
                     } /* endif */
                    } /* endif */
                  }
                  else
                  {
                    usRC = AddToSegmentW( pParsDataW, chCurrent );
                  } /* endif */
                  break;
            } /* endswitch */
         } /* endif */
      } while ( !usRC && !*(pParsDataW->pfKill) ); /* enddo */         /* @50C */

      if ( ! *(pParsDataW->pfKill) )
      {
        if ( usRC == EOF_REACHED )
        {
           usRC = 0;
        } /* endif */

        if ( !usRC )
        {
          usRC = EndSegmentW( pParsDataW );
        } /* endif */
      } /* endif */
      /****************************************************************/
      /* do the closing                                               */
      /****************************************************************/
      if ( !usRC && !*(pParsDataW->pfKill) )                           /* @50C */
      {
        usRC = ParseCloseW( pParsDataW );
      } /* endif */
   } /* endif */

   // cleanup

   if ( pParsDataW->hOutFile )
   {
      UtlClose( pParsDataW->hOutFile, TRUE );
   } /* endif */
   if ( pParsDataW->hInFile )
   {
      UtlClose( pParsDataW->hInFile, TRUE );
   } /* endif */
    UtlDelete( chUTF16InFile, 0L, NOMSG );

   /*******************************************************************/
   /* free pParsData struct - if allocated                            */
   /*******************************************************************/
   if ( pParsDataW )
   {
     UtlAlloc((PVOID *) &pParsDataW, 0L, 0L, NOMSG );
   } /* endif */

   return( usRC );
} /* endof ParseMriW */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FindStringW                                               |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       find if pSearch is part of pString1                      |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ_W  pString1,             string to be searched         |
//|                   PSZ_W  pSearch               string to be searched         |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ_W                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    ptr to start of pSearch in pString1              |
//|                   NULL    if pSearch is not found                          |
//+----------------------------------------------------------------------------+
//|Function flow:     loop thru the first string and compare with the first    |
//|                     character of the second string                         |
//|                     if match found compare with the total string           |
//+----------------------------------------------------------------------------+
static
PSZ_W  FindStringW
(
  PSZ_W  pString1,                       // string to be searched
  PSZ_W  pSearch                         // string to be searched
)
{
  BOOL  fFound = FALSE;                // success indicator
  CHAR_W  c, d, c1;                          // active character
  ULONG  ulSearchLen = UTF16strlenCHAR( pSearch );
  PSZ_W    pStart = pString1;

  c = *pSearch;                        // first character of search string

  while ( !fFound && ((d = *pString1) != NULC))
  {
    if ( d == c )
    {
      fFound = ! UTF16strnicmp( pString1, pSearch, (USHORT)ulSearchLen );
      if (fFound)
      {
        // if pString= "  IDS_PARAM_CONTROLID  " and pSearch = "CONTROL" fFound = TRUE!
        // check whether the word pSearch has been found as a stand-a-lone word
        // the character in front of the match and after the match should not be alphanumeric!
        // and char in front+after the match should not be "_"
        // see P017028, "_I_SUM_WARE_CONTROL_DB"
        c = *(pString1 + ulSearchLen);
        if (iswalnum(c ))
        {
          fFound = FALSE;
        }
        else if (pString1 != pStart )
        {
          c1 = *(pString1 - 1);
          if (iswalnum(c1))
          {
            fFound = FALSE;
	      }
	      else
	      { 
          // if "..._CONTROL_.." set fFound = FALSE (P017028)
          if ( (c == UNDERSCORE_CHAR_W) || (c1 == UNDERSCORE_CHAR_W))
          {
				    fFound = FALSE;
  		    }
	      }
        }

      }
      if ( !fFound )
      {
        pString1++;
      }
    }
    else
    {
      pString1++;
    } /* endif */
  } /* endwhile */

  return (pString1);
} /* end of function FindStringW */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FindStringNotInQuotesW                                    |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       find if pSearch is part of pString1                      |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ  pString1,             string to be searched         |
//|                   PSZ  pSearch               string to be searched         |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    ptr to start of pSearch in pString1              |
//|                   NULL    if pSearch is not found                          |
//+----------------------------------------------------------------------------+
//|Function flow:     loop thru the first string                               |
//|                     skip all inside quoted strings                         |
//|                     if not in quoted string:                               |
//|                       compare with the first                               |
//|                       character of the second string                       |
//|                       if match found compare with the total string         |
//+----------------------------------------------------------------------------+
static
PSZ_W  FindStringNotInQuotesW
(
  PSZ_W  pString1,                       // string to be searched
  PSZ_W  pSearch                         // string to be searched
)
{
  BOOL  fFound = FALSE;                // success indicator
  CHAR_W  c, d;                          // active character
  ULONG ulSearchLen = UTF16strlenCHAR( pSearch );
  BOOL   fInsideQuotedString = FALSE;

  // keywords such as CONTROL is only searched for if it is not inside
  // a quoted string!

  c = *pSearch;                        // first character of search string

  while ( !fFound && (d = *pString1) != NULC )
  {
    if ( d == STRING_DELIMITER )
    {
        fInsideQuotedString = !fInsideQuotedString;    // skip text within quoted strings
    }
    if ( !fInsideQuotedString && (d == c) )
    {
      fFound = ! UTF16strnicmp( pString1, pSearch, (USHORT)ulSearchLen );
      if ( !fFound )
      {
        pString1++;
      } /* endif */
    }
    else
    {
      pString1++;
    } /* endif */
  } /* endwhile */

  return (pString1);

} /* end of function FindStringNotInQuotes */

/**********************************************************************/
/* Table used by function MriGetSettings to check the settings file.  */
/* The table has to be terminated by an empty entry. The values in    */
/* the value list are seperated by a comma. The function uses the     */
/* index of the specified value in the value list as new setting for  */
/* a specific keyword (e.g. "NO,YES", "NO" sets 0, "YES" sets 1).     */
/**********************************************************************/
typedef struct _SETTINGKEYWORD
{
  SHORT       sID;                     // symbolic identifier for keyword
  CHAR        szKey[20];               // keyword of switch
  SHORT       sLen;                    // length of keyword
  CHAR        szValues[80];            // list of possible values
} SETTINGKEYWORD, *PSETTINGKEYWORD;

enum
{
  DOUBLEQUOTES_ID,
  SINGLEQUOTES_ID,
  ANSI_ID
} KEYWORDIDS;

SETTINGKEYWORD SettingsKeywords[] =
{
  { DOUBLEQUOTES_ID, "DOUBLEQUOTES=", 13,  "NO,YES" },
  { SINGLEQUOTES_ID, "SINGLEQUOTES=", 13,  "NO,YES" },
  { ANSI_ID,         "ANSI=",          5,  "NO,YES" },
  { 0,               "",               0,  "" }
};


/**********************************************************************/
/*  MriGetSettings                                                    */
/*                                                                    */
/* Get any settings file (name of settings file is name of tag table  */
/* with an extension of .CHR) and set the flags in the passed         */
/* flags structure                                                    */
/**********************************************************************/
static USHORT MriGetSettings
(
  PSZ         pszTagTable,             // name of tag table
  PMRIPARSEFLAGS pFlags                // reference to MRI parser flag structure
)
{
  // private data area
  typedef struct _GETSETTINGSDATA
  {
    CHAR chInBuf[8096];                // input buffer
    CHAR szLine[1024];                 // buffer for current line
    CHAR szSettingsFile[MAX_EQF_PATH]; // path name of settings file
  } GETSETTINGSDATA, *PGETSETTINGSDATA;

  PGETSETTINGSDATA pData = NULL;       // pointer to private data
  HFILE hfSettings = NULLHANDLE;       // file handle of settings file
  BOOL        fOK = TRUE;              // internal O.K. flag
  LONG        lBytesToRead = 0L;       // number of bytes to read from file
  ULONG       ulBytesInBuffer = 0;     // number of bytes in buffer

  // allocate our private data structure
  fOK = UtlAlloc( (PVOID *)&pData, 0L, sizeof(GETSETTINGSDATA), NOMSG );

  // setup name of settings file and open settings file
  if ( fOK )
  {
    USHORT usActionTaken;              // action performed by file open

    UtlMakeEQFPath( pData->szSettingsFile, NULC, TABLE_PATH, NULL );
    GetOTMTablePath( pData->szSettingsFile, pData->szLine ) ;
    strcpy( pData->szSettingsFile, pData->szLine ) ;
    Utlstrccpy( pData->szSettingsFile + strlen(pData->szSettingsFile),
                pszTagTable, DOT );
    strcat( pData->szSettingsFile, SETTINGS_EXTENSION );
    fOK = UtlOpen( pData->szSettingsFile, &hfSettings, &usActionTaken, 0L,
             FILE_NORMAL, FILE_OPEN,
             OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
             0L, FALSE ) == NO_ERROR;
  } /* endif */

  // get size of input file
  if ( fOK )
  {
    FILESTATUS  stStatus;              // File status information

    fOK = UtlQFileInfo( hfSettings,  1, (PBYTE)&stStatus,
                        sizeof(FILESTATUS),  FALSE ) == NO_ERROR;
    lBytesToRead = stStatus.cbFile;
  } /* endif */

  // read and process until done
  while( fOK && ((lBytesToRead != 0L)|| (ulBytesInBuffer != 0)) )
  {
    PSETTINGKEYWORD pKeyWord = NULL;          // ptr for keyword processing

    // fill input buffer
    if ( lBytesToRead && (ulBytesInBuffer < sizeof(pData->chInBuf)) )
    {
      USHORT usBytesRead;              // number of bytes read from file
      ULONG  ulBytesToRead;

      ulBytesToRead = min( ((LONG)sizeof(pData->chInBuf)- ulBytesInBuffer),
                                    (ULONG)lBytesToRead );
      // do not upgrade to UtlReadL, since  downward compatibility would be lost!
      // RJ - 030325 - GQ agreed
      fOK = UtlRead( hfSettings, pData->chInBuf + ulBytesInBuffer,
                     (USHORT)ulBytesToRead,
                     &usBytesRead, FALSE ) == NO_ERROR;
      ulBytesInBuffer += usBytesRead;
      lBytesToRead -= usBytesRead;
    } /* endif */

    // extract next line from input buffer
    if ( fOK )
    {
      USHORT usPos = 0;

      // copy bytes to line buffer until EOL detected
      while ( (usPos < ulBytesInBuffer) &&
              (pData->chInBuf[usPos] != CR) &&
              (pData->chInBuf[usPos] != LF ) )
      {
        pData->szLine[usPos] = pData->chInBuf[usPos];
        usPos++;
      } /* endwhile */

      // terminate line buffer and skip EOL characters
      pData->szLine[usPos] = EOS;
      while ( (usPos < ulBytesInBuffer) &&
              ( (pData->chInBuf[usPos] == CR) ||
                (pData->chInBuf[usPos] == LF ) ) )
      {
        usPos++;
      } /* endwhile */

      // adjust data in input buffer
      memmove( pData->chInBuf, pData->chInBuf + usPos, ulBytesInBuffer - usPos );
      ulBytesInBuffer -= usPos;
    } /* endif */

    // check for settings keywords
    if ( fOK )
    {
      // lookup settings keyword in keyword table
      pKeyWord = SettingsKeywords;
      while ( (pKeyWord->szKey[0] != EOS) &&
              (strnicmp( pKeyWord->szKey, pData->szLine, pKeyWord->sLen ) != 0) )
      {
        pKeyWord++;
      }
    } /* endif */

    // check supplied value if keyword found and do any settings
    if ( pKeyWord->szKey[0] != EOS )
    {
      PSZ pszTemp;
      PSZ pszSpecifiedValue = pData->szLine + pKeyWord->sLen;
      PSZ pszValue;
      SHORT sValueIndex = 0;
      BOOL fDone = FALSE;

      // truncate value at first non-alphanumeric character
      pszTemp = pszSpecifiedValue;
      while ( isalnum(*pszTemp) ) pszTemp++;
      *pszTemp = EOS;

      // compare specified value with possible values
      pszValue = pKeyWord->szValues;

      while ( (*pszValue != EOS) && !fDone )
      {
        CHAR chTemp;                   // buffer for value end character

        // isolate current value of possible value list
        pszTemp = pszValue;
        while ( (*pszTemp != ',') && (*pszTemp != EOS) ) pszTemp++;
        chTemp = *pszTemp;
        *pszTemp = EOS;

        // compare against specified value
        if ( stricmp( pszValue, pszSpecifiedValue ) == 0 )
        {
          fDone = TRUE;
        } /* endif */

        // continue with next value (if any)
        *pszTemp = chTemp;
        if ( !fDone && (chTemp != EOS) )
        {
          pszValue = pszTemp + 1;
          sValueIndex += 1;
        }
        else
        {
          pszValue = pszTemp;
        } /* endif */
      } /* endwhile */

      // set flags if valid
      if ( fDone )
      {
        switch ( pKeyWord->sID )
        {
          case DOUBLEQUOTES_ID :
           pFlags->fDoubleQuotes = sValueIndex;
           break;
          case SINGLEQUOTES_ID :
           pFlags->fSingleQuotes = sValueIndex;
           break;
          case ANSI_ID :
           pFlags->fAnsi = sValueIndex;
           break;
          default:
           break;
        } /* endswitch */
      } /* endif */
    } /* endif */
  } /* endwhile */

  // cleanup
  if ( hfSettings ) UtlClose( hfSettings, FALSE );
  if ( pData )      UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

  // return to caller
  return( NO_ERROR );
} /* end of function MriGetSettings */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UnparseMRI                                               |
//+----------------------------------------------------------------------------+
//|Function call:     UnparseMRI( PSZ pszInFile );                             |
//+----------------------------------------------------------------------------+
//|Description:       Postprocessing of exported documents after the           |
//|                   segmentation tags (:QFx) have been removed from the      |
//|                   document by the text unsegmentation utility              |
//|                   Converts the MRi file back to ANSI.                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ  pszInFile        name of input file                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE        no errors during processing                  |
//|                   FALSE       errors occured                               |
//+----------------------------------------------------------------------------+
//|Side effects:      Input file is overwritten with the converted file.       |
//+----------------------------------------------------------------------------+
static BOOL UnparseMRI ( PSZ pszInFile, ULONG ulTgtOemCP )
{
  CHAR        szTempFile[CCHMAXPATH];  // buffer for temporary file name
  USHORT      usRC = 0;                // function return code
  USHORT      usBytesToRead;           // number of bytes to read into buffer
  USHORT      usBytesInBuffer;         // number of bytes in input buffer
  ULONG       lBytesToRead = 0;        // number of not-read bytes
  PCHAR       pConvTable = NULL;       // ptr to code conversion table
  PBYTE       pInBuf = NULL;           // ptr to input buffer
  HFILE       hInFile = NULL;          // handle of input file
  HFILE       hOutFile = NULL;          // handle of output file


  // Allocate input buffer
  if ( !UtlAlloc( (PVOID *)&pInBuf, 0L, (LONG)INBUF_SIZE, ERROR_STORAGE ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  /*******************************************************************/
  /* find extension and change it into temp extension '$$$'          */
  /*******************************************************************/
  if ( usRC == NO_ERROR )
  {
    PSZ pName;

    strcpy( szTempFile, pszInFile );
    pName = UtlGetFnameFromPath( szTempFile );
    while ( *pName && *pName != '.' )
    {
       pName ++;
    } /* endwhile */

    if ( !*pName )
    {
       *pName = '.';
    } /* endif */
    pName++;

    if ( strcmp( pName, "$$$" ) == 0 )
    {
      memset( pName, '@', 3 );
    }
    else
    {
      memset( pName, '$', 3 );
    } /* endif */
    *(pName+3) = EOS;                      // include end of string
  } /* endif */

  /*******************************************************************/
  /* open input file                                                 */
  /*******************************************************************/
  if ( !usRC )
  {
     USHORT      usOpenAction;                   // action performed by DosOpen

     usRC = UtlOpen( pszInFile,
                     &hInFile,
                     &usOpenAction, 0L,
                     FILE_NORMAL,
                     FILE_OPEN,
                     OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                     0L,
                     TRUE );
  } /* endif */

  /*******************************************************************/
  /* open output file                                                */
  /*******************************************************************/
  if ( !usRC )
  {
     USHORT      usOpenAction;                             // action performed by DosOpen

     usRC = UtlOpen( szTempFile,
                     &hOutFile,
                     &usOpenAction, 0L,
                     FILE_NORMAL,
                     FILE_TRUNCATE | FILE_CREATE,
                     OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                     0L,
                     TRUE );
  } /* endif */

  /*******************************************************************/
  /* get size of input file                                          */
  /*******************************************************************/
  if ( !usRC )
  {
    FILESTATUS  stStatus;               // File status information
    usRC = UtlQFileInfo( hInFile,
                         1,
                         (PBYTE)&stStatus,
                         sizeof(FILESTATUS),
                         TRUE );
    lBytesToRead = stStatus.cbFile;
  } /* endif */


  /**************************************************************/
  /* Get type of code conversion                                */
  /**************************************************************/
  if ( IsDBCS_CP(ulTgtOemCP)   )
  {
    pConvTable = NULL;           // no code conversion in DBCS environment
  }
  else
  {
    UtlQueryCharTableForDocConv( pszInFile, &pConvTable, ulTgtOemCP );
  } /* endif */

  /*******************************************************************/
  /* Mainloop of parser                                              */
  /*******************************************************************/
  while ( !usRC && lBytesToRead )
  {
    /****************************************************************/
    /* Fill input buffer                                            */
    /****************************************************************/
    usBytesToRead = (USHORT) min( (LONG)INBUF_SIZE, lBytesToRead );
    usRC = UtlRead( hInFile, pInBuf, usBytesToRead, &usBytesInBuffer, TRUE );
    if ( !usRC )
    {
      lBytesToRead -= usBytesInBuffer;
    } /* endif */

    // remove end-of-file character at end of last data block
    if ( lBytesToRead == 0L )
    {
      PSZ pszTest = (PSZ)(pInBuf + (usBytesInBuffer - 1));
      while ( usBytesInBuffer )
      {
        if ( (*pszTest != EOS) && (*pszTest != 0x1A) )
        {
          break;                     // end of data detected
        }
        else
        {
          // remove current character from input buffer
          pszTest--;
          usBytesInBuffer--;
        } /* endif */
      } /* endwhile */
    } /* endif */

    /****************************************************************/
    /* Process data in input buffer                                 */
    /****************************************************************/
    if ( !usRC && (pConvTable != NULL) )
    {
      USHORT usI;

      for ( usI = 0; usI < usBytesInBuffer; usI++ )
      {
        pInBuf[usI] = pConvTable[pInBuf[usI]];
      } /* endfor */
    } /* endif */

    /******************************************************************/
    /* Write buffer to output file                                    */
    /******************************************************************/
    if ( !usRC )
    {
      USHORT      usBytesWritten;      // number of bytes written to file

      usRC = UtlWrite( hOutFile, pInBuf, usBytesInBuffer, &usBytesWritten, TRUE );
    } /* endif */
  } /* endwhile */

  /*******************************************************************/
  /* cleanup                                                         */
  /*******************************************************************/
  if ( hOutFile )
  {
    UtlClose( hOutFile, TRUE );

    /****************************************************************/
    /* delete output file in case of erros                          */
    /****************************************************************/
    if ( usRC )
    {
      UtlDelete( szTempFile, 0L, FALSE );
    } /* endif */
  } /* endif */


  if ( hInFile ) UtlClose( hInFile, TRUE );
  if ( pInBuf )  UtlAlloc( (PVOID *)&pInBuf, 0L, 0L, NOMSG );

  /*******************************************************************/
  /* Delete old file and rename temp file                            */
  /*******************************************************************/
  if ( !usRC )
  {
    UtlDelete( pszInFile, 0L, FALSE );
    usRC = UtlMove( szTempFile, pszInFile, 0L, FALSE );
  } /* endif */

  return( usRC == 0 );
} /* end of function UnparseMRI */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MriAddToLineBufW                                          |
//+----------------------------------------------------------------------------+
//|Function call:     MriAddToLineBufW(PSZ, CHAR, PUSHORT)                      |
//+----------------------------------------------------------------------------+
//|Description:       add next char to buffer of current line                  |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ       pLineString,                                   |
//|                   USHORT    usLen                                          |
//|                   CHAR      bCurrent,                                      |
//|                   PUSHORT   pusInLine                                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       -                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      -                                                        |
//+----------------------------------------------------------------------------+
static VOID MriAddToLineBufW
(
   PSZ_W       pLineString,
   USHORT    usLen,
   CHAR_W      chCurrent,
   PUSHORT   pusInLine
)
{
   USHORT    usIndex;

   usIndex = *pusInLine;

   if (usIndex == 0 )        //init for new line
   {
     UTF16memset( pLineString, '\0', usLen );
   } /* endif */
   if (usIndex < usLen-1 )   // check for buffer size
   {
     *(pLineString + usIndex) = chCurrent;
     usIndex++;
   } /* endif */
   if (chCurrent == LF_W )
   {
     // if line starts with CONTROL, and not the 3rd tok found, go on
     // with same linebuffer! ( P012717)
     PSZ_W  pControl = NULL;
     PSZ_W  pCurrent = NULL;
     BOOL fGoon = FALSE;

     pControl = FindStringNotInQuotesW(pLineString, CONTROL_STR_W);
     if (*pControl )
     {
       pCurrent = pControl + UTF16strlenCHAR(CONTROL_STR_W);
       if (MriCountTokW(pCurrent) < 3 )
       {
         fGoon = TRUE;
       }
     }
     if (!fGoon)
     {
        *(pLineString + usIndex)  = EOS;  // start new line buffer
         usIndex = 0;
     }
   } /* endif */
   *pusInLine = usIndex;
  return;
} /* end of function MriAddToLineBufW */

static VOID
MriRemoveFromLineBufW
(
	PSZ_W     pLineString,
	USHORT  usCount,
	PUSHORT pusInLine
)
{ USHORT    usIndex;
  USHORT    usI = 0;

   usIndex = *pusInLine;
   if ((usIndex >= usCount) && usCount )        //init for new line
   {
	   usIndex = usIndex - usCount;
	   while (usI < usCount)
	   {
	     *(pLineString + usIndex + usI) = EOS;
	     usI++;
	   }
   } /* endif */
   *pusInLine = usIndex;
  return;
} /* end of function MriRemoveFromLineBufW */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MriHandleControlStringW                                   |
//+----------------------------------------------------------------------------+
//|Function call:     MriHandleControlStringW(pParsData,CHAR, PSZ, PUSHORT)     |
//+----------------------------------------------------------------------------+
//|Description:       handle not-translatable strings in CONTROL line          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PPARSDATA pParsData                                      |
//|                   CHAR      bCurrent,                                      |
//|                   PSZ       pLineString,                                   |
//|                   USHORT    usLen                                          |
//|                   PUSHORT   pusInLine                                      |
//|                   PMRIPARSEFLAGS pFlags                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       usRC                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      -                                                        |
//+----------------------------------------------------------------------------+
static USHORT
MriHandleControlStringW
(
   PPARSEDATAW     pParsDataW,
   CHAR_W         chCurrent,
   PSZ_W          pLineString,
   USHORT         usLen,                     // number of CHAR_Ws!
   PUSHORT        pusInLine,
   PMRIPARSEFLAGS pFlags
)
{
  PSZ_W           pCurrent;
  USHORT        usRC = 0;
  BOOL          fEndString = FALSE;
  USHORT        usCountParams = 0;
  PSZ_W           pControl = NULL;
  CHAR_W        chEmpty = 0;


  if (pFlags->fDoubleQuotes && (chCurrent == STRING_DELIMITER) )
  {
    pControl = FindStringW(pLineString, CONTROL_STR_W);

    // checke if we are really dealing with a control statement and note some other line containing the word "control"....
    if ( *pControl )
    {
      // the CONTROL statement normally follows this syntax: "CONTROL text, id, class, style, x, y, width, height [, extended-style]"
      // so it should be the first word within the given line and there should be commas seperating the arguments
      PSZ_W pszTest = pLineString;
      while ( iswspace(*pszTest) ) pszTest++;
      if ( pszTest != pControl )
      {
        pControl = &chEmpty;    // not first text in line..
      } /* endif */

      // now check for commas...
      if ( *pControl )
      {
        // position to end of first argument
        pszTest = pControl + UTF16strlenCHAR(CONTROL_STR_W);
        while ( iswspace(*pszTest) ) pszTest++;
        if ( iswalnum(*pszTest) )
        {
          while ( iswalnum(*pszTest) ) pszTest++;
        }
        else if ( *pszTest == L'\"' )
        {
          pszTest++;
          while ( *pszTest && (*pszTest != L'\"') ) pszTest++;
          if ( *pszTest ) pszTest++;
        }
        else 
        {
          pControl = &chEmpty;
        } /* endif */

        while ( iswspace(*pszTest) ) pszTest++;

        // now we should be at the seperator..
        if ( *pszTest && (*pszTest != L',') )
        {
          pControl = &chEmpty;
        } /* endif */
      } /* endif */
    } /* endif */

    if (*pControl )
    {
      // the 3rd parameter in a CONTROL line
      // is not translatable even if it is a quoted string!
      pCurrent = pControl + UTF16strlenCHAR(CONTROL_STR_W);
      usCountParams = MriCountTokW(pCurrent);

      if (usCountParams == 3)
      {
        /****************************************/
        /* loop here til end of string !!       */
        /****************************************/
        while (!usRC && !*(pParsDataW->pfKill ) && !fEndString )
        {
          chCurrent = ParseNextCharW(pParsDataW, &usRC);
          MriAddToLineBufW(pLineString, usLen, chCurrent, pusInLine);

          usRC = AddToSegmentW( pParsDataW, chCurrent );
          if ( chCurrent == STRING_DELIMITER)
          {
            fEndString = TRUE;
            pParsDataW->Mode = OUTSIDE_STRING_MODE;
          }  /* endif */
        } /* endwhile */
      } /* endif */
    } /* endif */
  } /* endif */
 return (usRC);
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MriHandleFontStringW                                      |
//+----------------------------------------------------------------------------+
//|Function call:     MriHandleFontStringW(pParsData,CHAR, PSZ, PUSHORT)        |
//+----------------------------------------------------------------------------+
//|Description:       handle not-translatable strings in CONTROL line          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PPARSDATAW pParsDataW                                      |
//|                   CHAR_W      bCurrent,                                      |
//|                   PSZ_W       pLineString,                                   |
//|                   USHORT    usLen                                          |
//|                   PUSHORT   pusInLine                                      |
//|                   PMRIPARSEFLAGS pFlags                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       usRC                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      -                                                        |
//+----------------------------------------------------------------------------+
static USHORT
MriHandleFontStringW
(
   PPARSEDATAW     pParsDataW,
   CHAR_W           chCurrent,
   PSZ_W            pLineString,
   USHORT         usLen,                  // in number of CHAR_Ws
   PUSHORT        pusInLine,
   PMRIPARSEFLAGS pFlags
)
{
  PSZ_W           pCurrent;
  USHORT        usRC = 0;
  BOOL          fEndString = FALSE;
  USHORT        usCountParams = 0;
  PSZ_W           pControl = NULL;


  if (pFlags->fDoubleQuotes && (chCurrent == STRING_DELIMITER) )
  {
    pControl = FindStringW(pLineString, FONT_STR_W);
    if (*pControl )
    {
      // the 2nd parameter in a line which starts with "FONT"
      // is not translatable even if it is a quoted string!
      pCurrent = pControl + UTF16strlenCHAR(FONT_STR_W);
      usCountParams = MriCountTokW(pCurrent);

      if (usCountParams == 2)
      {
        /****************************************/
        /* loop here til end of string !!       */
        /****************************************/
        while (!usRC && !*(pParsDataW->pfKill ) && !fEndString )
        {
          chCurrent = ParseNextCharW(pParsDataW, &usRC);
          MriAddToLineBufW(pLineString, usLen, chCurrent, pusInLine);

          usRC = AddToSegmentW( pParsDataW, chCurrent );
          if ( chCurrent == STRING_DELIMITER)
          {
            fEndString = TRUE;
            pParsDataW->Mode = OUTSIDE_STRING_MODE;
          }  /* endif */
        } /* endwhile */
      } /* endif */
    } /* endif */
  } /* endif */
 return (usRC);
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MriCountTOkW                                              |
//+----------------------------------------------------------------------------+
//|Function call:     MriCountTokW(pTemp)                                       |
//+----------------------------------------------------------------------------+
//|Description:       count tokens in pTemp: blank and , are delimiters        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ_W   pTemp                                              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       usCountParams - number of tokens in string               |
//+----------------------------------------------------------------------------+
//|Example: pTemp=' 8, "' usCountParams = 2 (' shows start and end of string)  |
//|               ' "" 144, "' usCountParams = 3                               |
//|               ' "Value", 120, "' usCountParams = 3                         |
//+----------------------------------------------------------------------------+
static
USHORT
MriCountTokW
(
     PSZ_W      pCurrent
)
{
  CHAR_W          chCurrent;
  USHORT        usBlankPending = 0;
  USHORT        usCountParams = 0;


   usBlankPending = NO_BLANK;

   chCurrent = *pCurrent++;
   while (chCurrent != EOS)
   {
     switch (chCurrent)
     {
       case '\"':
         if (usBlankPending == BLANK_PENDING_FOR_PARM)
         {
           usCountParams ++;
         } /* endif */
         usBlankPending = NO_BLANK;
         chCurrent = *pCurrent ++;        // loop til end of this string
         while ((chCurrent != EOS) && (chCurrent != '\"' ))
         {
           chCurrent = *pCurrent ++;
         } /* endwhile */
         break;
       case ' ':
       case CR:
       case LF:
         if (usBlankPending == NO_BLANK )
         {
           usBlankPending = BLANK_PENDING_FOR_PARM;
         } /* endif */
         break;
       case ',':
         usCountParams ++;
         usBlankPending = PARM_COUNTED;
         break;
       case '|':
         usBlankPending = KOMMA_NEEDED;
         break;
       default :
         //do nothing at other positions
         if (usBlankPending == BLANK_PENDING_FOR_PARM)
         {
           usCountParams ++;
         } /* endif */
         usBlankPending = NO_BLANK;
         break;
     } /* endswitch */
     if (chCurrent != EOS )
     {
       chCurrent = * pCurrent ++;
     } /* endif */
   } /* endwhile */

   // P012717: Example: 'CONTROL   "TSM", IDC_CLIE,CRLF' must return Count=2 ; therefore
   // if usBlankPending= PARM_COUNTED, decrease usCountParams

   if ((usBlankPending == PARM_COUNTED) && usCountParams)
   {
     usCountParams --;
   }
   return (usCountParams);
}


// @@@@ the following functions are only nec to allow downward compatibility
//
//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MriAddToLineBuf                                          |
//+----------------------------------------------------------------------------+
//|Function call:     MriAddToLineBuf(PSZ, CHAR, PUSHORT)                      |
//+----------------------------------------------------------------------------+
//|Description:       add next char to buffer of current line                  |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ       pLineString,                                   |
//|                   USHORT    usLen                                          |
//|                   CHAR      bCurrent,                                      |
//|                   PUSHORT   pusInLine                                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       -                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      -                                                        |
//+----------------------------------------------------------------------------+
static VOID MriAddToLineBuf
(
   PSZ       pLineString,
   USHORT    usLen,
   CHAR      bCurrent,
   PUSHORT   pusInLine
)
{
   USHORT    usIndex;

   usIndex = *pusInLine;

   if (usIndex == 0 )        //init for new line
   {
     memset( pLineString, 0, usLen );
   } /* endif */
   if (usIndex < usLen-1 )   // check for buffer size
   {
     *(pLineString + usIndex) = bCurrent;
     usIndex++;
   } /* endif */
   if (bCurrent == LF )
   {
     // if line starts with CONTROL, and not the 3rd tok found, go on
     // with same linebuffer! ( P012717)
     PSZ  pControl = NULL;
     PSZ  pCurrent = NULL;
     BOOL fGoon = FALSE;

     pControl = FindStringNotInQuotes(pLineString, CONTROL_STR);
     if (*pControl )
     {
       pCurrent = pControl + strlen(CONTROL_STR);
       if (MriCountTok(pCurrent) < 3 )
       {
         fGoon = TRUE;
       }
     }
     if (!fGoon)
     {
        *(pLineString + usIndex)  = EOS;  // start new line buffer
         usIndex = 0;
     }
   } /* endif */
   *pusInLine = usIndex;
  return;
} /* end of function MriAddToLineBuf */

static VOID
MriRemoveFromLineBuf
(
	PSZ     pLineString,
	USHORT  usCount,
	PUSHORT pusInLine
)
{ USHORT    usIndex;
  USHORT    usI = 0;

   usIndex = *pusInLine;
   if ((usIndex >= usCount) && usCount )        //init for new line
   {
	   usIndex = usIndex - usCount;
	   while (usI < usCount)
	   {
	     *(pLineString + usIndex + usI) = EOS;
	     usI++;
	   }
   } /* endif */
   *pusInLine = usIndex;
  return;
} /* end of function MriRemoveFromLineBuf */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MriHandleControlString                                   |
//+----------------------------------------------------------------------------+
//|Function call:     MriHandleControlString(pParsData,CHAR, PSZ, PUSHORT)     |
//+----------------------------------------------------------------------------+
//|Description:       handle not-translatable strings in CONTROL line          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PPARSDATA pParsData                                      |
//|                   CHAR      bCurrent,                                      |
//|                   PSZ       pLineString,                                   |
//|                   USHORT    usLen                                          |
//|                   PUSHORT   pusInLine                                      |
//|                   PMRIPARSEFLAGS pFlags                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       usRC                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      -                                                        |
//+----------------------------------------------------------------------------+
static USHORT
MriHandleControlString
(
   PPARSEDATA     pParsData,
   CHAR           bCurrent,
   PSZ            pLineString,
   USHORT         usLen,
   PUSHORT        pusInLine,
   PMRIPARSEFLAGS pFlags
)
{
  PSZ           pCurrent;
  USHORT        usRC = 0;
  BOOL          fEndString = FALSE;
  USHORT        usCountParams = 0;
  PSZ           pControl = NULL;


  if (pFlags->fDoubleQuotes && (bCurrent == STRING_DELIMITER) )
  {
    pControl = FindString(pLineString, CONTROL_STR);
    if (*pControl )
    {
      // the 3rd parameter in a CONTROL line
      // is not translatable even if it is a quoted string!
      pCurrent = pControl + strlen(CONTROL_STR);
      usCountParams = MriCountTok(pCurrent);

      if (usCountParams == 3)
      {
        /****************************************/
        /* loop here til end of string !!       */
        /****************************************/
        while (!usRC && !*(pParsData->pfKill ) && !fEndString )
        {
          bCurrent = ParseNextChar(pParsData, &usRC);
          MriAddToLineBuf(pLineString, usLen, bCurrent, pusInLine);

          usRC = AddToSegment( pParsData, bCurrent );
          if ( bCurrent == STRING_DELIMITER)
          {
            fEndString = TRUE;
            pParsData->Mode = OUTSIDE_STRING_MODE;
          }  /* endif */
        } /* endwhile */
      } /* endif */
    } /* endif */
  } /* endif */
 return (usRC);
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MriHandleFontString                                      |
//+----------------------------------------------------------------------------+
//|Function call:     MriHandleFontString(pParsData,CHAR, PSZ, PUSHORT)        |
//+----------------------------------------------------------------------------+
//|Description:       handle not-translatable strings in CONTROL line          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PPARSDATA pParsData                                      |
//|                   CHAR      bCurrent,                                      |
//|                   PSZ       pLineString,                                   |
//|                   USHORT    usLen                                          |
//|                   PUSHORT   pusInLine                                      |
//|                   PMRIPARSEFLAGS pFlags                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       usRC                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      -                                                        |
//+----------------------------------------------------------------------------+
static USHORT
MriHandleFontString
(
   PPARSEDATA     pParsData,
   CHAR           bCurrent,
   PSZ            pLineString,
   USHORT         usLen,
   PUSHORT        pusInLine,
   PMRIPARSEFLAGS pFlags
)
{
  PSZ           pCurrent;
  USHORT        usRC = 0;
  BOOL          fEndString = FALSE;
  USHORT        usCountParams = 0;
  PSZ           pControl = NULL;


  if (pFlags->fDoubleQuotes && (bCurrent == STRING_DELIMITER) )
  {
    pControl = FindString(pLineString, FONT_STR);
    if (*pControl )
    {
      // the 2nd parameter in a line which starts with "FONT"
      // is not translatable even if it is a quoted string!
      pCurrent = pControl + strlen(FONT_STR);
      usCountParams = MriCountTok(pCurrent);

      if (usCountParams == 2)
      {
        /****************************************/
        /* loop here til end of string !!       */
        /****************************************/
        while (!usRC && !*(pParsData->pfKill ) && !fEndString )
        {
          bCurrent = ParseNextChar(pParsData, &usRC);
          MriAddToLineBuf(pLineString, usLen, bCurrent, pusInLine);

          usRC = AddToSegment( pParsData, bCurrent );
          if ( bCurrent == STRING_DELIMITER)
          {
            fEndString = TRUE;
            pParsData->Mode = OUTSIDE_STRING_MODE;
          }  /* endif */
        } /* endwhile */
      } /* endif */
    } /* endif */
  } /* endif */
 return (usRC);
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MriCountTOk                                              |
//+----------------------------------------------------------------------------+
//|Function call:     MriCountTok(pTemp)                                       |
//+----------------------------------------------------------------------------+
//|Description:       count tokens in pTemp: blank and , are delimiters        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ   pTemp                                              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       usCountParams - number of tokens in string               |
//+----------------------------------------------------------------------------+
//|Example: pTemp=' 8, "' usCountParams = 2 (' shows start and end of string)  |
//|               ' "" 144, "' usCountParams = 3                               |
//|               ' "Value", 120, "' usCountParams = 3                         |
//+----------------------------------------------------------------------------+
static
USHORT
MriCountTok
(
     PSZ      pCurrent
)
{
  CHAR          chCurrent;
  USHORT        usBlankPending = 0;
  USHORT        usCountParams = 0;


   usBlankPending = NO_BLANK;

   chCurrent = *pCurrent++;
   while (chCurrent != EOS)
   {
     switch (chCurrent)
     {
       case '\"':
         if (usBlankPending == BLANK_PENDING_FOR_PARM)
         {
           usCountParams ++;
         } /* endif */
         usBlankPending = NO_BLANK;
         chCurrent = *pCurrent ++;        // loop til end of this string
         while ((chCurrent != EOS) && (chCurrent != '\"' ))
         {
           chCurrent = *pCurrent ++;
         } /* endwhile */
         break;
       case ' ':
       case CR:
       case LF:
         if (usBlankPending == NO_BLANK )
         {
           usBlankPending = BLANK_PENDING_FOR_PARM;
         } /* endif */
         break;
       case ',':
         usCountParams ++;
         usBlankPending = PARM_COUNTED;
         break;
       case '|':
         usBlankPending = KOMMA_NEEDED;
         break;
       default :
         //do nothing at other positions
         if (usBlankPending == BLANK_PENDING_FOR_PARM)
         {
           usCountParams ++;
         } /* endif */
         usBlankPending = NO_BLANK;
         break;
     } /* endswitch */
     if (chCurrent != EOS )
     {
       chCurrent = * pCurrent ++;
     } /* endif */
   } /* endwhile */

   // P012717: Example: 'CONTROL   "TSM", IDC_CLIE,CRLF' must return Count=2 ; therefore
   // if usBlankPending= PARM_COUNTED, decrease usCountParams

   if ((usBlankPending == PARM_COUNTED) && usCountParams)
   {
     usCountParams --;
   }
   return (usCountParams);
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FindString                                               |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       find if pSearch is part of pString1                      |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ  pString1,             string to be searched         |
//|                   PSZ  pSearch               string to be searched         |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    ptr to start of pSearch in pString1              |
//|                   NULL    if pSearch is not found                          |
//+----------------------------------------------------------------------------+
//|Function flow:     loop thru the first string and compare with the first    |
//|                     character of the second string                         |
//|                     if match found compare with the total string           |
//+----------------------------------------------------------------------------+
static
PSZ  FindString
(
  PSZ  pString1,                       // string to be searched
  PSZ  pSearch                         // string to be searched
)
{
  BOOL  fFound = FALSE;                // success indicator
  CHAR  c, d, c1;                          // active character
  ULONG  ulSearchLen = strlen( pSearch );
  PSZ    pStart = pString1;

  strupr( pString1 );                  // normalize the string

  c = *pSearch;                        // first character of search string

  while ( !fFound && ((d = *pString1) != NULC))
  {
    if ( d == c )
    {
      fFound = ! strncmp( pString1, pSearch, ulSearchLen );
      if (fFound)
      {
        // if pString= "  IDS_PARAM_CONTROLID  " and pSearch = "CONTROL" fFound = TRUE!
        // check whether the word pSearch has been found as a stand-a-lone word
        // the character in front of the match and after the match should not be alphanumeric!
        // and char in front+after the match should not be "_"
        // see P017028, "_I_SUM_WARE_CONTROL_DB"
        c = *(pString1 + ulSearchLen);
        if (isalnum(c ))
        {
          fFound = FALSE;
        }
        else if (pString1 != pStart )
        {
          c1 = *(pString1 - 1);
          if (isalnum(c1))
          {
            fFound = FALSE;
	      }
	      else
	      { // if "..._CONTROL_.." set fFound = FALSE (P017028)
            if ( (c == UNDERSCORE_CHAR) && (c1 == UNDERSCORE_CHAR))
            {
				fFound = FALSE;
		    }
	      }
        }

      }
      if ( !fFound )
      {
        pString1++;
      }
    }
    else
    {
      pString1++;
    } /* endif */
  } /* endwhile */

  return (pString1);
} /* end of function FindString */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FindStringNotInQuotes                                    |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       find if pSearch is part of pString1                      |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ  pString1,             string to be searched         |
//|                   PSZ  pSearch               string to be searched         |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    ptr to start of pSearch in pString1              |
//|                   NULL    if pSearch is not found                          |
//+----------------------------------------------------------------------------+
//|Function flow:     loop thru the first string                               |
//|                     skip all inside quoted strings                         |
//|                     if not in quoted string:                               |
//|                       compare with the first                               |
//|                       character of the second string                       |
//|                       if match found compare with the total string         |
//+----------------------------------------------------------------------------+
static
PSZ  FindStringNotInQuotes
(
  PSZ  pString1,                       // string to be searched
  PSZ  pSearch                         // string to be searched
)
{
  BOOL  fFound = FALSE;                // success indicator
  CHAR  c, d;                          // active character
  ULONG ulSearchLen = strlen( pSearch );
  BOOL   fInsideQuotedString = FALSE;

  // keywords such as CONTROL is only searched for if it is not inside
  // a quoted string!

  strupr( pString1 );                  // normalize the string

  c = *pSearch;                        // first character of search string

  while ( !fFound && (d = *pString1) != NULC )
  {
    if ( d == STRING_DELIMITER )
    {
        fInsideQuotedString = !fInsideQuotedString;    // skip text within quoted strings
    }
    if ( !fInsideQuotedString && (d == c) )
    {
      fFound = ! strncmp( pString1, pSearch, ulSearchLen );
      if ( !fFound )
      {
        pString1++;
      } /* endif */
    }
    else
    {
      pString1++;
    } /* endif */
  } /* endwhile */

  return (pString1);

} /* end of function FindStringNotInQuotes */

//+----------------------------------------------------------------------------+
//| ParseMri         - parse a MRI file                                        |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Parses an input file and creates a segmented output file.               |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - setup global data and tag names                                  |
//|         - open input and output file                                       |
//|         - enclose strings of input file in QFF/EQFF tags, enclose          |
//|           all other stuff in QFN/EQFN tags and write data to output file   |
//|         - close input and output file                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ       pszSource             - fully qualified name of source file     |
//|  PSZ       pszTarget             - fully qualified name of target file     |
//|  HWND      hwndSlider            - handle of slider window                 |
//|  PBOOL     pfKill                - pointer to kill flag                    |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
USHORT ParseMri
(
   PSZ pszSource,                      // fully qualified name of source file
   PSZ pszTarget,                      // fully qualified name of target file
   HWND hwndSlider,                    // handle of slider window
   PEQF_BOOL pfKill,                   // pointer to kill flag              /* @50A */
   PSZ pszTagTable                     // name of tag table
)
{
   USHORT      usRC = 0;               // function return code
   USHORT      usOpenAction;           // action performed by DosOpen
   FILESTATUS  stStatus;               // File status information
   BYTE        bCurrent;               // currently processed byte
   BYTE        bNext   ;               // next byte
   USHORT      usInLine = 0;           // position in line
   PPARSEDATA  pParsData = NULL;       // points to parser data structure
   BYTE        bCurStringDelim = SPACE;// currently active string delimiter
   PUCHAR      pConvTable = NULL;      // character conversion table
   MRIPARSEFLAGS Flags;                // flags for parsing


   UtlAlloc((PVOID *) &pParsData, 0L, (LONG)sizeof(PARSEDATA), ERROR_STORAGE );
   if ( pParsData )
   {
     ParseInit( pParsData, hwndSlider, pfKill );
     usRC = ParseFillCP( pParsData, pszSource);
   }
   else
   {
     usRC = ERROR_STORAGE;
   } /* endif */

   memset( &Flags, 0, sizeof(Flags) );

   // get settings from .CHR file or use defaults
   if ( !usRC && !*(pParsData->pfKill) )                              /* @50C */
   {
     MriGetSettings( pszTagTable, &Flags );
     if ( !Flags.fDoubleQuotes && !Flags.fSingleQuotes )
     {
       Flags.fDoubleQuotes = TRUE;
     } /* endif */
   } /* endif */

   // open input file
   if ( !usRC && !*(pParsData->pfKill) )                              /* @50C */
   {
      usRC = UtlOpen( pszSource,
                      &(pParsData->hInFile),
                      &usOpenAction, 0L,
                      FILE_NORMAL,
                      FILE_OPEN,
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                      0L,
                      TRUE );
   } /* endif */

   // open output file
   if ( !usRC && !*(pParsData->pfKill) )                              /* @50C */
   {
      usRC = UtlOpen( pszTarget,
                      &(pParsData->hOutFile),
                      &usOpenAction, 0L,
                      FILE_NORMAL,
                      FILE_TRUNCATE | FILE_CREATE,
                      OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                      0L,
                      TRUE );
   } /* endif */

   // get size of input file
   if ( !usRC && !*(pParsData->pfKill) )                              /* @50C */
   {
      usRC = UtlQFileInfo( pParsData->hInFile,
                           1,
                           (PBYTE)&stStatus,
                           sizeof(FILESTATUS),
                           TRUE );
      pParsData->lBytesToRead = stStatus.cbFile;
      pParsData->lTotalBytes  = stStatus.cbFile;
   } /* endif */

   // Get table for character conversion ANSI -> ASCII
   if ( Flags.fAnsi )
   {
    if (  IsDBCS_CP(pParsData->ulSrcOemCP)  )
    {
      pConvTable = NULL;  // no code conversion in DBCS environment
    }
    else
    {
      UtlQueryCharTableEx( ANSI_TO_ASCII_TABLE, &pConvTable, (USHORT)pParsData->ulSrcOemCP );
    } /* endif */
   } /* endif */

   if ( !usRC && !*(pParsData->pfKill) )
   {
      StartSegment( pParsData );

      do {
         bCurrent = ParseNextChar( pParsData, &usRC );
         if ( pConvTable && Flags.fAnsi ) bCurrent = pConvTable[bCurrent];

         // change a HEX 0 into a BLANK
         if ( bCurrent == EOS )  bCurrent = BLANK;

         if ( !usRC && !*(pParsData->pfKill) )                        /* @50C */
         {
			if ((pParsData->Mode != LINE_COMMENT_MODE) &&
			    (pParsData->Mode != BLOCK_COMMENT_MODE) &&
			     (pParsData->Mode != COMMENT_END_MODE))
			{
				// P018267: do not add comments to LineBuf!
              MriAddToLineBuf(chOutSideString, sizeof(chOutSideString),
                            bCurrent, &usInLine);
		    }
            switch ( pParsData->Mode )
            {
               case COMMENT_START_MODE:
                  // Switch to LINE_COMMENT_MODE, BLOCK_COMMENT_MODE or
                  // OUTSIDE_STRING_MODE depending on current character
                  switch ( bCurrent )
                  {
                     case BLOCK_COMMENT:
                        pParsData->Mode = BLOCK_COMMENT_MODE;
                        MriRemoveFromLineBuf(chOutSideString,
                                              2, &usInLine);
                        break;
                     case LINE_COMMENT:
                        pParsData->Mode = LINE_COMMENT_MODE;
                        MriRemoveFromLineBuf(chOutSideString,
                                              2, &usInLine);
                        break;
                     default:
                        pParsData->Mode = OUTSIDE_STRING_MODE;
                        break;
                  } /* endswitch */
                  // add current char to active segment
                  usRC = AddToSegment( pParsData, bCurrent );
                  break;

               case LINE_COMMENT_MODE:
                  // Add bytes to active segment until end of line is reached.
                  // At at end of line switch back to OUTSIDE_STRING_MODE
                  if ( (bCurrent == LF) || (bCurrent == CR) )
                  {
                     pParsData->Mode = OUTSIDE_STRING_MODE;
                     // P018267: add linefeed to buffer
                     MriAddToLineBuf(chOutSideString, sizeof(chOutSideString),
                            bCurrent, &usInLine);
                  } /* endif */
                  usRC = AddToSegment( pParsData, bCurrent );
                  break;

               case BLOCK_COMMENT_MODE:
                  // Add bytes to active segment until may be end of comment
                  // block is reached.
                  // At at end of block switch to COMMENT_END_MODE
                  if ( bCurrent == BLOCK_COMMENT )
                  {
                    if ( (bNext = ParseNextChar( pParsData, &usRC )) == LINE_COMMENT )
                    {
                      pParsData->Mode = COMMENT_END_MODE;
                    } /* endif */
                    UndoNextChar( pParsData, bNext );
                  } /* endif */
                  usRC = AddToSegment( pParsData, bCurrent );
                  break;

               case COMMENT_END_MODE:
                  // Switch to BLOCK_COMMENT_MODE or OUTSIDE_STRING_MODE
                  // depending on current character
                  if ( bCurrent == START_COMMENT )
                  {
                     pParsData->Mode = OUTSIDE_STRING_MODE;
                  }
                  else
                  {
                     pParsData->Mode = BLOCK_COMMENT_MODE;
                  } /* endif */
                  usRC = AddToSegment( pParsData, bCurrent );
                  break;

               case STRING_MODE:
                  // Add bytes to active segment until end string delimiter is
                  // reached or a BACKSLASH is encountered.
                  // At end of string terminate current segment, switch to
                  // OUTSIDE_STRING_MODE and start a new segment.
                  // At BACKSLASH switch to ESCAPE_CHAR_MODE to handle
                  // escape characters.
                  switch ( bCurrent )
                  {
                     case BACKSLASH:
                        pParsData->Mode = ESCAPE_CHAR_MODE;
                        usRC = AddToSegment( pParsData, bCurrent );
                        break;
                     case STRING_DELIMITER:
                     case QUOTE_DELIMITER:
                        if ( bCurrent == bCurStringDelim )
                        {
                          bNext = ParseNextChar( pParsData, &usRC );
                          if ( usRC == EOF_REACHED )
                          {
                            // no more characters to follow, close segment, start
                            // a new one for the end delimiter
                            usRC = EndSegment( pParsData );
                            pParsData->Mode = OUTSIDE_STRING_MODE;
                            if ( !usRC )
                            {
                              usRC = StartSegment( pParsData );
                            } /* endif */
                            if ( !usRC )
                            {
                              usRC = AddToSegment( pParsData, bCurrent );
                            } /* endif */
                          }
                          else if ( !usRC )
                          {
                            if (bNext == bCurStringDelim)
                            {
                              usRC = AddToSegment( pParsData, bCurrent );
                              if ( !usRC )
                              {
                                usRC = AddToSegment( pParsData, bNext );
                              } /* endif */
                            }
                            else
                            {
                              UndoNextChar( pParsData, bNext );
                              usRC = EndSegment( pParsData );
                              pParsData->Mode = OUTSIDE_STRING_MODE;
                              if ( !usRC )
                              {
                                usRC = StartSegment( pParsData );
                              } /* endif */
                              if ( !usRC )
                              {
                                usRC = AddToSegment( pParsData, bCurrent );
                              } /* endif */
                            } /* endif */
                          } /* endif */
                        }
                        else
                        {
                          // treat as normal character within the string
                          usRC = AddToSegment( pParsData, bCurrent );
                        } /* endif */
                        break;
                     default:
                        usRC = AddToSegment( pParsData, bCurrent );
                        break;
                  } /* endswitch */
                  break;

               case ESCAPE_CHAR_MODE:
                  // Add current character to segment without any checks.
                  // Switch back to STRING_MODE.
                  /****************************************************/
                  /* check if we are at the end of the string         */
                  /* might happen in case the backslash is a          */
                  /* 2nd byte of a double byte                        */
                  /* in such cases treat the backslash as a backslash */
                  /****************************************************/
                  if ( bCurrent == STRING_DELIMITER )
                  {
                    bNext = ParseNextChar( pParsData, &usRC );
                    if ( !usRC )
                    {
                      if (  IsDBCS_CP(pParsData->ulSrcOemCP)  && ((bNext == CR) || (bNext == LF)) )
                      {
                        BYTE bNext1 = ParseNextChar( pParsData, &usRC ) ;               // next byte
                        if ( !usRC && bNext1 == LF )
                        {
                          usRC = AddToSegment( pParsData, bCurrent );

                          UndoNextChar( pParsData, bNext1 );  // undo LF
                          UndoNextChar( pParsData, bNext );  // undo CR
                        }
                        else
                        {
                          UndoNextChar( pParsData, bNext1 );  // undo LF
                          UndoNextChar( pParsData, bNext );  // undo CR
                          UndoNextChar( pParsData, bCurrent );  // undo STRING_END
                        }
                      }
                      else
                      {
                        usRC = AddToSegment( pParsData, bCurrent );
                        UndoNextChar( pParsData, bNext );  // undo character
                      } /* endif */
                    } /* endif */
                  }
                  else
                  {
                    usRC = AddToSegment( pParsData, bCurrent );
                  } /* endif */
                  pParsData->Mode = STRING_MODE;
                  break;

               case OUTSIDE_STRING_MODE:
                  // Add bytes to active segment until a comment start or a
                  // string delimiter is found.
                  // Switch to COMMENT_START_MODE or STRING_MODE depending
                  // on character found.
                  if ( bCurrent == LF )
                  {
                    usRC = AddToSegment( pParsData, bCurrent );
                    if ( !usRC )
                    {

                    } /* endif */
                    if ( !usRC )
                    {
                      // start new segment only if not at end of file ( KBT0974)
                      bNext = ParseNextChar( pParsData, &usRC );
                      if (usRC != EOF_REACHED )
                      {
                           usRC = EndSegment( pParsData );
                           if (!usRC)
                           {
                             usRC = StartSegment( pParsData );
                           }
                       }
                       UndoNextChar( pParsData, bNext );
                    } /* endif */
                  }
                  else if ( bCurrent == START_COMMENT )
                  {
                    pParsData->Mode = COMMENT_START_MODE;
                    usRC = AddToSegment( pParsData, bCurrent );
                  }
                  else if ( (Flags.fDoubleQuotes && (bCurrent == STRING_DELIMITER) ) ||
                            (Flags.fSingleQuotes && (bCurrent == QUOTE_DELIMITER) ) )
                  {
                   bCurStringDelim = bCurrent;
                   usRC = AddToSegment( pParsData, bCurrent );
                   //  ignore empty strings, i.e. check if next char is
                   //    string delimiter, too.
                   if ( !usRC )
                   {
                     bCurrent = ParseNextChar( pParsData, &usRC );
                   } /* endif */
                   if ( !usRC )
                   {
                     if ( bCurrent != bCurStringDelim)
                     {
                        USHORT    usStartInLine;
                        PSZ       pTemp;
                        UndoNextChar( pParsData, bCurrent );
                        usStartInLine = usInLine;          // temp save index

                        usRC = MriHandleControlString(pParsData,
                                               bCurStringDelim,
                                               chOutSideString,
                                               sizeof(chOutSideString),
                                               &usInLine,
                                               &Flags );
                        // if tgtlang is DBCS Font statement must be translatable!
                        if ((usStartInLine == usInLine) &&
						    (!IsDBCS_CP(pParsData->ulTgtOemCP)) &&
                            (!IsDBCS_CP(pParsData->ulSrcOemCP)) )
                        {
                          MriHandleFontString(pParsData,
                                            bCurStringDelim,
                                            chOutSideString,
                                            sizeof(chOutSideString),
                                            &usInLine,
                                            &Flags );
                        } /* endif */
                        if (usStartInLine == usInLine )
                        {
                          usRC = EndSegment( pParsData );
                          /****************************************/
                          /* remain in outside string mode if     */
                          /* include is pending ...               */
                          /****************************************/

                          pTemp = FindString( chOutSideString, INCLUDE_STR );
                          if (!(*pTemp) )
                          {
                            pParsData->Mode = STRING_MODE;
                          } /* endif */
                          if ( !usRC )
                          {
                            usRC = StartSegment( pParsData );
                          } /* endif */
                          /****************************************/
                          /* set new string mode in any case ...  */
                          /****************************************/
                          pParsData->Mode = STRING_MODE;
                        } /* endif */
                     }
                     else
                     {
                        MriAddToLineBuf(chOutSideString,
                                        sizeof(chOutSideString),
                                        bCurrent, &usInLine);
                        usRC = AddToSegment( pParsData, bCurrent );
                     } /* endif */
                    } /* endif */
                  }
                  else
                  {
                    usRC = AddToSegment( pParsData, bCurrent );
                  } /* endif */
                  break;
            } /* endswitch */
         } /* endif */
      } while ( !usRC && !*(pParsData->pfKill) ); /* enddo */         /* @50C */

      if ( ! *(pParsData->pfKill) )
      {
        if ( usRC == EOF_REACHED )
        {
           usRC = 0;
        } /* endif */

        if ( !usRC )
        {
          usRC = EndSegment( pParsData );
        } /* endif */
      } /* endif */
      /****************************************************************/
      /* do the closing                                               */
      /****************************************************************/
      if ( !usRC && !*(pParsData->pfKill) )                           /* @50C */
      {
        usRC = ParseClose( pParsData );
      } /* endif */
   } /* endif */

   // cleanup
   if ( pParsData->hOutFile )
   {
      UtlClose( pParsData->hOutFile, TRUE );
   } /* endif */
   if ( pParsData->hInFile )
   {
      UtlClose( pParsData->hInFile, TRUE );
   } /* endif */

   /*******************************************************************/
   /* free pParsData struct - if allocated                            */
   /*******************************************************************/
   if ( pParsData )
   {
     UtlAlloc((PVOID *) &pParsData, 0L, 0L, NOMSG );
   } /* endif */

   return( usRC );
} /* endof ParseMri */
