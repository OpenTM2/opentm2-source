/*! \brief EQFXAPI.C
	Copyright (c) 1990-2015, International Business Machines Corporation and others. All rights reserved.
	Description: EQF_API contains the functions described in the document 'Troja Editor API'                                            |
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TP               // public translation processor functions
//  #define DLLIMPORTRESMOD         // resource module handle imported from DLL

#include <eqf.h>                  // General Translation Manager include file

#include "EQFB.ID"                // Translation Processor IDs
#include <eqftpi.h>               // private Translation Processor include file
#include "eqftai.h"               // Private include file for Text Analysis

#include <eqfdoc00.h>

#include "eqffol.h"
#include "eqfhlog.h"              // private history log include file

#define SLIDER_INCREMENT    101                  // number of increments on slider bar

PSTEQFGEN        pstEQFGeneric = NULL;          // pointer to generic struct
HGLOBAL       sel;
HANDLE        hMapObject = NULL;              // file map object for shared memory

static USHORT  EQFAllocate (VOID);
static BOOL    fUnicodeSystem = 0;

INT_PTR CALLBACK EQFSHOWDLG ( HWND, WINMSG, WPARAM, LPARAM );
static VOID RegisterShowWnd( VOID );
BOOL EQFShowWndCreate ( PTBDOCUMENT, HWND, ULONG, USHORT );
USHORT WriteHistLogAndAdjustCountInfo( PSZ pszSegTargetFile, BOOL fAdjustCountInfo );
MRESULT APIENTRY EQFSHOWWP ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2);
MRESULT HandleWMChar ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );
#define EQFSHOW_CLASS   "EQFSHOW"
#define EQFSHOW_CLASS_W L"EQFSHOW"

typedef struct   _TextPanel
{
    HWND         hwndParent;
    PSZ_W   *    ppData;
    USHORT       usDefFG;
    USHORT       usDefBG;
    USHORT       usForeground;
    USHORT       usBackground;
    PSZ_W        pCaptionText;
    ULONG        flFrameStyle;
    DOCTYPE      docType;
    TBDOCUMENT   TBDoc;
    BYTE         TokBuf[TOK_BUFFER_SIZE * 2];
    BYTE         BlockMark[ sizeof( EQFBBLOCK )];
    BYTE         InBuf[ IO_BUFFER_SIZE * 2];
    BOOL         fUnicode;    // unicode indication
} TEXTPANEL, * PTEXTPANEL;
static BOOL  FillDisplay ( PTEXTPANEL pTextPanel );
static BOOL PrepareLine ( PTEXTPANEL, PSZ_W, PULONG );

static TBSEGMENT  tbInitSegment = { NULL, 0, QF_PROP0PREFIX, 0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, L"", NULL};
static TBSEGMENT  tbNewLineSegment = { NULL , 1,QF_XLATED,0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, L"\n", NULL};
static VOID  EQFDispWindow ( PTBDOCUMENT  pTBDoc );
static SHORT sTagging[4];

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
/*------------------------------------------------------------------------------
* Function prototypes (internal)
*-----------------------------------------------------------------------------*/
USHORT  EQF_PackBuf     (PUCHAR, PSZ *);

/*------------------------------------------------------------------------------
* Globals
*-----------------------------------------------------------------------------*/
USHORT          usEQFErrID   = EQFRC_OK;        // ret.message ID
USHORT          usEQFRcClass = EQF_INFO;        // ret.code class
USHORT          usEQFSysRc   = NO_ERROR;        // system return code
ERRTYPE         ErrorType;                      // error classes
CHAR            szMsgFile[CCHMAXPATH] = "c:\\eqf\\msg\\eqf.msg"; // def. msg file
CHAR_W          szMsgBuf[EQF_MSGBUF_SIZE];      // buffer for message
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFINIT                                                  |
//+----------------------------------------------------------------------------+
//|Function call:     EQFINIT (PSTEQFGEN, PSZ *, PSZ *);                       |
//+----------------------------------------------------------------------------+
//|Description:       This function will initialize the translation workbench  |
//|                   services                                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   pointer to generic structure                 |
//|                   PSZ *       pointer array of translation memories        |
//|                   PSZ *       pointer array of dictionaries                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      static pstEQFGen will be used.                           |
//+----------------------------------------------------------------------------+
//|Function flow:     set pstEQFGeneric and call EQFXINIT                      |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFINIT (PSTEQFGEN pstEQFGen, PSZ *apszTlm, PSZ *apszLDict)

{
       pstEQFGeneric = pstEQFGen;          // pointer to generic struct
       return ( EQFXINIT (pstEQFGen, apszTlm, apszLDict));
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXINIT                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXINIT (PSTEQFGEN, PSZ *, PSZ *);                      |
//+----------------------------------------------------------------------------+
//|Description:       This function will initialize the translation workbench  |
//|                   services                                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   pointer to generic structure                 |
//|                   PSZ *       pointer array of translation memories        |
//|                   PSZ *       pointer array of dictionaries                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen not allocated then                          |
//|                     call EQFAllocate and set Error if necessary            |
//|                     set generic structure address                          |
//|                   endif                                                    |
//|                   if pstEQFGen then                                        |
//|                     prepare structure for init call                        |
//|                     if no message queue available then                     |
//|                       init PM and create message queue                     |
//|                     endif                                                  |
//|                     send the command (WM_EQFCMD_INIT)                      |
//|                     set return code                                        |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT EQFXINIT
(
   PSTEQFGEN pstEQFGen,
   PSZ *apszTlm,
   PSZ *apszLDict
)

{
  USHORT        usRc = 0;
  ULONG         ulOffset;
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct

  if ( !pstEQFGen )
  {
     usRc =  EQFAllocate ( );
     if (usRc == NO_ERROR)
     {
         pstEQFGen = pstEQFGeneric;
     } /* endif */
  } /* endif */

  if ( pstEQFGen )
  {
     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     memset (pstEQFPCmd, '\0', sizeof (STEQFPCMD));    // clear cmd/rsp struct
     pstEQFPCmd->usCmd   = EQFCMD_INIT;

     if (pstEQFGen)
     {
       ulOffset = sizeof (STEQFGEN);
       memcpy (pstEQFPCmd->ucbBuffer, pstEQFGen, ulOffset );
       ulOffset += 2;
       pstEQFPCmd->usLen1  = (USHORT)ulOffset;
     }
     else
     {
       ulOffset =
       pstEQFPCmd->usLen1 = 0;
     }
     pstEQFPCmd->ulParm1 = EQF_PackBuf (pstEQFPCmd->ucbBuffer+ulOffset ,
                                        apszTlm);
     ulOffset += (USHORT)pstEQFPCmd->ulParm1;
     pstEQFPCmd->usParm2 = EQF_PackBuf (pstEQFPCmd->ucbBuffer+ulOffset ,
                                        apszLDict);

     pstEQFPCmd->usLen2  = (USHORT)pstEQFPCmd->ulParm1 +
                           pstEQFPCmd->usParm2 ;
                                          // check if message queue is avail.
                                          // Send the command
     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_INIT, NULL, NULL);
     usRc = pstEQFGen->usRC;              // set return code
  } /* endif */

  return usRc;
} // end 'EQFINIT'
/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFCLEAR                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     EQFCLEAR ( USHORT );                                     |
//+----------------------------------------------------------------------------+
//|Description:       This function will clear the internal buffers            |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT      flags as described in OTMAPI.H               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXCLEAR and return the result                     |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFCLEAR (USHORT fsFlags)
{
   return (EQFXCLEAR ( pstEQFGeneric, fsFlags ));

}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXCLEAR                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXCLEAR ( PSTEQFGEN, USHORT );                         |
//+----------------------------------------------------------------------------+
//|Description:       This function will clear the internal buffers            |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   USHORT      flags as described in OTMAPI.H               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     initialize values for the EQFCLEAR call                |
//|                     send message to document handler                       |
//|                     set return code                                        |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT                       |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT EQFXCLEAR
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   USHORT fsFlags
)

{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_CLEAR;
     pstEQFPCmd->ulParm1 = fsFlags;
     pstEQFPCmd->usLen1  =
     pstEQFPCmd->usLen2  = 0;

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_CLEAR, NULL, NULL);

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFCLEAR'
/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFCLOSE                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     EQFCLOSE ( BOOL );                                       |
//+----------------------------------------------------------------------------+
//|Description:       This function will close the services to standby mode    |
//|                   or close them directly                                   |
//|                   The later one is only left to be upward compatible.      |
//+----------------------------------------------------------------------------+
//|Parameters:        BOOL        flag indicating shutdown or standby mode     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXCLOSE and return return code from EQFXCLOSE     |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFCLOSE
(
   EQF_BOOL fShutdown
)
{
   return( EQFXCLOSE( pstEQFGeneric, fShutdown ));

}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXCLOSE                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXCLOSE ( PSTEQFGEN, BOOL   );                         |
//+----------------------------------------------------------------------------+
//|Description:       This function will close the services to standby mode    |
//|                   or close them directly                                   |
//|                   The later one is only left to be upward compatible.      |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   BOOL        flag indicating shutdown or standby mode     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set and ! fShutDown then                 |
//|                     initialize values for the EQFCLOSE call                |
//|                     send message to document handler                       |
//|                     set return code                                        |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXCLOSE
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   EQF_BOOL fShutdown                           // shutdown services ??
)
{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc = NO_ERROR;                // return value

  if ( pstEQFGen && ! fShutdown )
  {

    pstEQFPCmd = pstEQFGen->pstEQFPCmd;

    pstEQFPCmd->usCmd   = EQFCMD_CLOSE;
    pstEQFPCmd->ulParm1 = fShutdown;
    pstEQFPCmd->usLen1  = pstEQFPCmd->usLen2  = 0;

     pstEQFGen->usRC = 0;
    WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_CLOSE, NULL, NULL);

    usRc = pstEQFGen->usRC;                     // set return value
  } /* endif */

  return usRc;
} // end 'EQFCLOSE'
/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETPROP                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETPROP( USHORT, PSZ, PUSHORT );                      |
//+----------------------------------------------------------------------------+
//|Description:       This function will try to get the proposal with the      |
//|                   specified number.                                        |
//|                   It returns the level of the match.                       |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT      segment number                               |
//|                   PSZ         text of the segment                          |
//|                   PUSHORT     level of matching proposal                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXGETPROP with the appropriate parameters         |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETPROP (USHORT usNum, PSZ pszBuffer, PUSHORT pusLevel)
{
   return( EQFXGETPROP (pstEQFGeneric, usNum, pszBuffer, pusLevel));

}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXGETPROP                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXGETPROP( PSTEQFGEN, USHORT, PSZ, PUSHORT );          |
//|                   EQFXGETPROPW( PSTEQFGEN, USHORT, PSZ, PUSHORT );         |
//+----------------------------------------------------------------------------+
//|Description:       This function will try to get the proposals for the      |
//|                   specified segment with the specified segment number.     |
//|                   It returns the level of the match.                       |
//|                   EQFXGETPROPW is the unicode version.                     |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   USHORT      segment number                               |
//|                   PSZ         text of the segment                          |
//|                   PUSHORT     level of matching proposal                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler                       |
//|                     if no error then                                       |
//|                       copy buffer and match level in provided buffer       |
//|                     endif                                                  |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXGETPROP
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   USHORT usNum,                                // number of proposal to get
   PSZ pszBuffer,                               // buffer for proposal
   PUSHORT pusLevel                             // matching level
)

{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code
  BOOL          fEqualFlagRequested = FALSE;
  ULONG         ulOEMCP = 0L;


  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;


     pstEQFPCmd->usCmd   = EQFCMD_GETPROPW;
     pstEQFPCmd->ulParm1 = usNum;
     pstEQFPCmd->usLen1  = pstEQFPCmd->usLen2  = 0;
     if (pusLevel)
     {
       if ( *pusLevel == QUERYIFSOURCEISEQUAL )
       {
         fEqualFlagRequested = TRUE;
       } /* endif */
       *pusLevel = (USHORT) -1;
     } /* endif */

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_GETPROP, NULL, NULL);

     if ( ! pstEQFGen->usRC )
     {
       if (pszBuffer)
       {
         PDOCUMENT_IDA pIdaDoc = NULL;
         pIdaDoc  = (PDOCUMENT_IDA) pstEQFGen->pDoc;
         ulOEMCP = GetLangOEMCP( pIdaDoc->szDocTargetLang);
         Unicode2ASCIIBuf( (PSZ_W)&pstEQFPCmd->ucbBuffer[0], pszBuffer,
                           (USHORT)(UTF16strlenCHAR((PSZ_W)pstEQFPCmd->ucbBuffer)+1),
                           min (pstEQFPCmd->usLen1, EQF_SEGLEN ), ulOEMCP );
       } /* endif */
       if (pusLevel)
         *pusLevel = (USHORT)pstEQFPCmd->ulParm1;
       if ( fEqualFlagRequested )
       {
         if ( pstEQFPCmd->usParm2 )
         {
           *pusLevel = *pusLevel | 0x8000;
         } /* endif */
       } /* endif */
     } /* endif */

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFGETPROP'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETPROPW
(
   USHORT  usNum,                               // number of proposal to get
   PSZ_W   pszBuffer,                           // buffer for proposal
   PUSHORT pusLevel                             // matching level
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code
  BOOL          fEqualFlagRequested = FALSE;


  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;


     pstEQFPCmd->usCmd   = EQFCMD_GETPROPW;
     pstEQFPCmd->ulParm1 = usNum;
     pstEQFPCmd->usLen1  = pstEQFPCmd->usLen2  = 0;
     if (pusLevel)
     {
       if ( *pusLevel == QUERYIFSOURCEISEQUAL )
       {
         fEqualFlagRequested = TRUE;
       } /* endif */
       *pusLevel = (USHORT) -1;
     } /* endif */

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_GETPROP, NULL, NULL);

     if ( ! pstEQFGen->usRC )
     {
       if (pszBuffer)
       {
         int iLen = min ((pstEQFPCmd->usLen1 + pstEQFPCmd->usLen2) * sizeof(CHAR_W), EQF_SEGLEN*sizeof(CHAR_W)*2);
         memcpy ((PVOID)pszBuffer, pstEQFPCmd->ucbBuffer, iLen );
         if ( pstEQFPCmd->usLen2 == 0) pszBuffer[pstEQFPCmd->usLen1] = 0; // ensure proper termination of adidtional data
       } /* endif */
       if (pusLevel)
         *pusLevel = (USHORT)pstEQFPCmd->ulParm1;
       if ( fEqualFlagRequested )
       {
         if ( pstEQFPCmd->usParm2 )
         {
           *pusLevel = *pusLevel | 0x8000;
         } /* endif */
       } /* endif */
     } /* endif */

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFXGETPROPW'


/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETDICT                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETDICT( USHORT, PSZ );                               |
//+----------------------------------------------------------------------------+
//|Description:       This function will try to get the requested dictionary   |
//|                   entry                                                    |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT      dictionary number                            |
//|                   PSZ         text of the dictionary entry                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXGETDICT with the appropriate parameters         |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETDICT (USHORT usNum, PSZ pszBuffer)
{
   return( EQFXGETDICT (pstEQFGeneric, usNum, pszBuffer));
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXGETDICT                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXGETDICT( PSTEQFGEN, USHORT, PSZ );                   |
//+----------------------------------------------------------------------------+
//|Description:       This function will try to get the specified dict. entry  |
//|                   and fetch it into the provided buffer                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   USHORT      segment number                               |
//|                   PSZ         text of the segment                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler                       |
//|                     if no error then                                       |
//|                       copy dictionary entry in provided buffer             |
//|                     endif                                                  |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXGETDICT
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   USHORT usNum,                                // number of dictionary hit
   PSZ pszBuffer                                // pointer to buffer
)

{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code
  ULONG         ulOEMCP;

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;


     pstEQFPCmd->usCmd   = EQFCMD_GETDICTW;
     pstEQFPCmd->ulParm1 = usNum;
     pstEQFPCmd->usLen1  =
     pstEQFPCmd->usLen2  = 0;

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_GETDICT, NULL, NULL);

     if ( !pstEQFGen->usRC && pszBuffer)
     {
       PDOCUMENT_IDA pIdaDoc = NULL;
       pIdaDoc  = (PDOCUMENT_IDA) pstEQFGen->pDoc;
       ulOEMCP = GetLangOEMCP( pIdaDoc->szDocTargetLang);

       Unicode2ASCIIBuf( (PSZ_W)&pstEQFPCmd->ucbBuffer[0], pszBuffer,
                         (USHORT)(UTF16strlenCHAR((PSZ_W)pstEQFPCmd->ucbBuffer)+1),
                         min (pstEQFPCmd->usLen1, EQF_SEGLEN), ulOEMCP );
     } /* endif */

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFGETDICT'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETDICTW
(
   USHORT    usNum,                             // number of dictionary hit
   PSZ_W     pszBuffer                          // pointer to buffer
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code


  if ( pstEQFGen )
  {
     pstEQFPCmd = pstEQFGen->pstEQFPCmd;


     pstEQFPCmd->usCmd   = EQFCMD_GETDICTW;
     pstEQFPCmd->ulParm1 = usNum;
     pstEQFPCmd->usLen1  =
     pstEQFPCmd->usLen2  = 0;

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_GETDICT, NULL, NULL);

     if ( !pstEQFGen->usRC && pszBuffer)
     {
       memcpy ((PVOID)pszBuffer, pstEQFPCmd->ucbBuffer,
                 min (pstEQFPCmd->usLen1*sizeof(CHAR_W), EQF_SEGLEN*sizeof(CHAR_W)));
     } /* endif */

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFGETDICT'

/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFDICTLOOK                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXDICTLOOK( PSZ, PSZ, USHORT, BOOL );                  |
//+----------------------------------------------------------------------------+
//|Description:       This function will try to lookup the specified dict.     |
//|                   entry.                                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         pointer to segment text (curr. not used).    |
//|                   PSZ         pointer to word to be looked up              |
//|                   USHORT      position of word in segment (curr. not used) |
//|                   BOOL        lookup term is in source/target lang         |
//|                               (curr. not used).                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXDICTLOOK with the appropriate parameters        |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDICTLOOK( PSZ pszSegBuf, PSZ pszWord,
                               USHORT usPos, EQF_BOOL fSource)
{
    return( EQFXDICTLOOK (pstEQFGeneric, pszSegBuf, pszWord,
                          usPos, fSource));
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXDICTLOOK                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXDICTLOOK(PSTEQFGEN, PSZ, PSZ, USHORT, BOOL);         |
//+----------------------------------------------------------------------------+
//|Description:       This function will try to lookup the specified dict.     |
//|                   entry.                                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         pointer to segment text (curr. not used).    |
//|                   PSZ         pointer to word to be looked up              |
//|                   USHORT      position of word in segment (curr. not used) |
//|                   BOOL        lookup term is in source/target lang         |
//|                               (curr. not used).                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT EQFXDICTLOOK
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   PSZ pszSegBuf,
   PSZ pszWord,
   USHORT usPos,
   EQF_BOOL fSource
)
{
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {
     PSZ_W pSrcW = NULL;
     ULONG ulOEMCP = 0L;

     UtlAlloc( (PVOID *)&pSrcW, 0L, EQF_SEGLEN * sizeof(CHAR_W) * 2, NOMSG );
     if ( pSrcW )
     {
       PSZ_W pTgtW = pSrcW + EQF_SEGLEN;
       PDOCUMENT_IDA pIdaDoc = NULL;
       pIdaDoc  = (PDOCUMENT_IDA) pstEQFGen->pDoc;
       ulOEMCP = GetLangOEMCP( pIdaDoc->szDocSourceLang);

       ASCII2Unicode( pszSegBuf, pSrcW, ulOEMCP );
       ASCII2Unicode( pszWord, pTgtW, ulOEMCP );

       usRc = EQFDICTLOOKW( pSrcW, pTgtW, usPos, fSource );

       UtlAlloc( (PVOID *)&pSrcW, 0L, 0L, NOMSG );
     }
     else
     {
       usRc = EQFRS_NO_INIT;                      // no generic structure yet
     }
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFDICTLOOK'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDICTLOOKW
(
   PSZ_W     pszSegBuf,
   PSZ_W     pszWord,
   USHORT    usPos,
   EQF_BOOL  fSource
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;


     pstEQFPCmd->usCmd   = EQFCMD_DICTLOOKW;
     pstEQFPCmd->ulParm1 = usPos;
     pstEQFPCmd->usParm2 = fSource;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pszSegBuf) + 1);
     pstEQFPCmd->usLen2  = (USHORT)(UTF16strlenCHAR(pszWord) + 1);
     memcpy (pstEQFPCmd->ucbBuffer, (PVOID)pszSegBuf, pstEQFPCmd->usLen1 * sizeof(CHAR_W));
     memcpy (pstEQFPCmd->ucbBuffer+pstEQFPCmd->usLen1, (PVOID)pszWord, pstEQFPCmd->usLen2 * sizeof(CHAR_W) );

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_DICTLOOK, NULL, NULL);
     usEQFErrID = pstEQFGen->usRC;

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFDICTLOOK'

/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFSAVESEG                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFSAVESEG( PSZ, PSZ, USHORT );                          |
//+----------------------------------------------------------------------------+
//|Description:       This function will save the source and target segment in |
//|                   the translation memory                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         pointer to source segment                    |
//|                   PSZ         pointer to target segment                    |
//|                   USHORT      segment number                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXSAVESEG  with the appropriate parameters        |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFSAVESEG (PSZ pszSrc, PSZ pszTgt, USHORT usSegNum)
{
   return( EQFXSAVESEG (pstEQFGeneric, pszSrc,  pszTgt,  usSegNum));

}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXSAVESEG                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXSAVESEG(PSTEQFGEN, PSZ, PSZ, USHORT );               |
//+----------------------------------------------------------------------------+
//|Description:       This function will save the source and target segment in |
//|                   the translation memory                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         pointer to source segment                    |
//|                   PSZ         pointer to target segment                    |
//|                   USHORT      segment number                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXSAVESEG
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   PSZ pszSrc,                                  // source segment
   PSZ pszTgt,                                  // target segment
   USHORT usSegNum                              // segment number
)
{
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {
     PSZ_W pSrcW = NULL;
     PSZ_W pTgtW = NULL;
     ULONG ulOEMCP = 0L;

     UtlAlloc( (PVOID *)&pSrcW, 0L, EQF_SEGLEN * sizeof(CHAR_W) * 2, NOMSG );
     if ( pSrcW )
     {
       PDOCUMENT_IDA pIdaDoc = NULL;
       pIdaDoc  = (PDOCUMENT_IDA) pstEQFGen->pDoc;
       // TODO @@ Should we use szDocSOurceLang for Src??
       ulOEMCP = GetLangOEMCP( pIdaDoc->szDocTargetLang);

       pTgtW = pSrcW + EQF_SEGLEN;
       ASCII2Unicode( pszSrc, pSrcW, ulOEMCP );
       ASCII2Unicode( pszTgt, pTgtW, ulOEMCP );

       usRc = EQFSAVESEGW( pSrcW, pTgtW, (ULONG) usSegNum );

       UtlAlloc( (PVOID *)&pSrcW, 0L, 0L, NOMSG );
     }
     else
     {
       usRc = EQFRS_NO_INIT;                      // no generic structure yet
     }
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */

  return usRc;
} // end 'EQFSAVESEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFSAVESEGW
(
   PSZ_W     pszSrc,                            // source segment
   PSZ_W     pszTgt,                            // target segment
   ULONG     ulSegNum                          // segment number
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code


  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;
     pstEQFPCmd->usCmd   = EQFCMD_SAVESEGW;
     pstEQFPCmd->ulParm1 = ulSegNum;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pszSrc) + 1);
     pstEQFPCmd->usLen2  = (USHORT)(UTF16strlenCHAR(pszTgt) + 1);
     memcpy (pstEQFPCmd->ucbBuffer, (PVOID)pszSrc, pstEQFPCmd->usLen1 * sizeof(CHAR_W));
     memcpy (pstEQFPCmd->ucbBuffer+pstEQFPCmd->usLen1*sizeof(CHAR_W), (PVOID)pszTgt, pstEQFPCmd->usLen2 * sizeof(CHAR_W));

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_SAVESEG, NULL, NULL);
     usEQFErrID = pstEQFGen->usRC;

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */

  return usRc;
} // end 'EQFSAVESEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFSAVESEG2W
(
   PSZ_W     pszSrc,                            // source segment
   PSZ_W     pszTgt,                            // target segment
   PSZ_W     pszContext,                        // segment context or NULL
   ULONG     ulSegNum                           // segment number
)
{
  return( EQFSAVESEG3W( pszSrc, pszTgt, pszContext, NULL, ulSegNum ) );
}

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFSAVESEG3W
(
   PSZ_W     pszSrc,                            // source segment
   PSZ_W     pszTgt,                            // target segment
   PSZ_W     pszContext,                        // segment context or NULL
   PVOID     pvMetaData,                        // segment meta data or NULL
   ULONG     ulSegNum                           // segment number
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code


  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;
     pstEQFPCmd->usCmd   = EQFCMD_SAVESEG2W;
     pstEQFPCmd->ulParm1 = ulSegNum;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pszSrc) + 1);
     pstEQFPCmd->usLen2  = (USHORT)(UTF16strlenCHAR(pszTgt) + 1);
     memcpy (pstEQFPCmd->ucbBuffer, (PVOID)pszSrc, pstEQFPCmd->usLen1 * sizeof(CHAR_W));
     memcpy (pstEQFPCmd->ucbBuffer+pstEQFPCmd->usLen1*sizeof(CHAR_W), (PVOID)pszTgt, pstEQFPCmd->usLen2 * sizeof(CHAR_W));

     if ( pszContext )
     {
       pstEQFPCmd->usParm3 = pstEQFPCmd->usLen3 = (USHORT)(UTF16strlenCHAR(pszContext) + 1);
       memcpy( pstEQFPCmd->ucbBuffer + pstEQFPCmd->usLen1*sizeof(CHAR_W) + pstEQFPCmd->usLen2*sizeof(CHAR_W),
               (PVOID)pszContext, pstEQFPCmd->usParm3 * sizeof(CHAR_W));
     }
     else
     {
       pstEQFPCmd->usParm3 = pstEQFPCmd->usLen3 = 0;
     } /* endif */

     if ( pvMetaData )
     {
       PSZ_W pszMetaData = (PSZ_W)(pstEQFPCmd->ucbBuffer + ((pstEQFPCmd->usLen1 + pstEQFPCmd->usLen2 + pstEQFPCmd->usLen3) * sizeof(CHAR_W)));

       if ( *((PSZ_W)pvMetaData) == L'<' )
       {
         // metadata alread in correct format
         wcscpy( pszMetaData, (PSZ_W)pvMetaData );
       }
       else
       {
         // convert document segment meta data format to memory segment meta data format
         MDConvertToMemMetadata( pvMetaData, pszMetaData );
       } /* endif */          

       if ( *pszMetaData != 0 )
       {
         pstEQFPCmd->usLen4  = (USHORT)(UTF16strlenCHAR(pszMetaData) + 1);
       }
       else
       {
         pstEQFPCmd->usLen4  = 0;
       } /* endif */          
     }
     else
     {
       pstEQFPCmd->usLen4 = 0;
     } /* endif */


     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_SAVESEG, NULL, NULL);
     usEQFErrID = pstEQFGen->usRC;

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */

  return usRc;
} // end 'EQFSAVESEG3W'


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFTRANSSEG                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFTRANSSEG( PSZ, USHORT, BOOL, PSHORT );                |
//+----------------------------------------------------------------------------+
//|Description:       This function will retrieve the dictionary and transl.   |
//|                   memory hits for the passed segment and segment number.   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         pointer to source segment                    |
//|                   USHORT      segment number                               |
//|                   BOOL        foreground/background request                |
//|                   PSHORT      level of match                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXTRANSSEG with the appropriate parameters        |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFTRANSSEG
(
  PSZ pszSrc, USHORT usSegNum,
  EQF_BOOL fForeground, USHORT fsFlags, PSHORT psExact
)
{

   return ( EQFXTRANSSEG( pstEQFGeneric, pszSrc,  usSegNum,
                          fForeground, fsFlags,  psExact));
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXTRANSSEG                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXTRANSSEG(PSTEQFGEN, PSZ, USHORT, BOOL, PSHORT );     |
//+----------------------------------------------------------------------------+
//|Description:       This function will retrieve the dictionary and transl.   |
//|                   memory hits for the passed segment and segment number.   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         pointer to source segment                    |
//|                   USHORT      segment number                               |
//|                   BOOL        foreground/background request                |
//|                   PSHORT      level of match                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXTRANSSEG
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   PSZ pszSrc,                                  // source segment
   USHORT usSegNum,                             // segment number
   EQF_BOOL fForeground,                        // foreground request
   USHORT fsFlags,                              // display flags
   PSHORT psExact                               // return level of exactness
)

{
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {
     PSZ_W pSrcW = NULL;
     ULONG ulOEMCP = 0L;

     UtlAlloc( (PVOID *)&pSrcW, 0L, EQF_SEGLEN * sizeof(CHAR_W), NOMSG );
     if ( pSrcW )
     {
       PDOCUMENT_IDA pIdaDoc = NULL;
       pIdaDoc  = (PDOCUMENT_IDA) pstEQFGen->pDoc;
       ulOEMCP = GetLangOEMCP( pIdaDoc->szDocSourceLang);

       ASCII2Unicode( pszSrc, pSrcW, ulOEMCP );
       usRc = EQFTRANSSEG2W( pSrcW, NULL, (ULONG) usSegNum, fForeground, fsFlags, psExact );
       UtlAlloc( (PVOID *)&pSrcW, 0L, 0L, NOMSG );
     }
     else
     {
       usRc = EQFRS_NO_INIT;                      // no generic structure yet
     }
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */

  return usRc;
} // end 'EQFTRANSSEG'


__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFTRANSSEGW
(
   PSZ_W     pszSrc,                            // source segment
   ULONG     ulSegNum,                          // segment number
   EQF_BOOL  fForeground,                       // foreground request
   USHORT    fsFlags,                           // display flags
   PSHORT    psExact                            // return level of exactness
)

{
  return( EQFTRANSSEG2W( pszSrc, NULL, ulSegNum, fForeground, fsFlags, psExact ) );
} // end 'EQFTRANSSEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFTRANSSEG2W
(
   PSZ_W     pszSrc,                            // source segment
   PSZ_W     pszContext,                        // segment context or NULL
   ULONG     ulSegNum,                          // segment number
   EQF_BOOL  fForeground,                       // foreground request
   USHORT    fsFlags,                           // display flags
   PSHORT    psExact                            // return level of exactness
)
{
  return( EQFTRANSSEG3W( pszSrc, pszContext, NULL, ulSegNum, fForeground, fsFlags, psExact ) );
}

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFTRANSSEG3W
(
   PSZ_W     pszSrc,                            // source segment
   PSZ_W     pszContext,                        // segment context or NULL
   PVOID     pvMetaData,                        // segment metadatacontext or NULL
   ULONG     ulSegNum,                          // segment number
   EQF_BOOL  fForeground,                       // foreground request
   USHORT    fsFlags,                           // display flags
   PSHORT    psExact                            // return level of exactness
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code


  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;


     pstEQFPCmd->usCmd   = EQFCMD_TRANSSEGW;
     pstEQFPCmd->ulParm1 = ulSegNum;
     pstEQFPCmd->usParm2 = fForeground;
     pstEQFPCmd->usParm3 = fsFlags;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pszSrc) + 1);
     pstEQFPCmd->usLen2  = 0;
     memcpy (pstEQFPCmd->ucbBuffer, (PVOID)pszSrc, pstEQFPCmd->usLen1 * sizeof(CHAR_W));
     if ( pszContext && *pszContext )
     {
       pstEQFPCmd->usLen2  = (USHORT)(UTF16strlenCHAR( pszContext ) + 1);
       memcpy( pstEQFPCmd->ucbBuffer + pstEQFPCmd->usLen1 * sizeof(CHAR_W),
               (PVOID)pszContext, pstEQFPCmd->usLen2 * sizeof(CHAR_W));
     } /* endif */

     pstEQFPCmd->pvMetaData = pvMetaData;

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_TRANSSEG, NULL, NULL);
     usEQFErrID = pstEQFGen->usRC;

     if (fForeground)
       *psExact = (SHORT)pstEQFPCmd->ulParm1;                     // indicator for exact match


     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */

  return usRc;
} // end 'EQFTRANSSEG2W'


/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFDELSEG                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFDELSEG( PSZ, PSZ, USHORT );                           |
//+----------------------------------------------------------------------------+
//|Description:       This function will delete the passed segment from the    |
//|                   translation memory.                                      |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         pointer to source segment                    |
//|                   PSZ         pointer to target segment                    |
//|                   USHORT      segment number                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXTRANSSEG with the appropriate parameters        |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDELSEG (PSZ pszSrc, PSZ pszTgt, USHORT usSegNum)
{
   return ( EQFXDELSEG (pstEQFGeneric, pszSrc, pszTgt, usSegNum));

}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXDELSEG                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXDELSEG( PSTEQFGEN, PSZ, PSZ, USHORT );               |
//+----------------------------------------------------------------------------+
//|Description:       This function will delete the passed segment from the    |
//|                   translation memory.                                      |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         pointer to source segment                    |
//|                   PSZ         pointer to target segment                    |
//|                   USHORT      segment number                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXDELSEG
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   PSZ pszSrc,                                  // source segment
   PSZ pszTgt,                                  // target segment
   USHORT usSegNum                              // segment number
)
{
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {
     PSZ_W pSrcW = NULL;

     UtlAlloc( (PVOID *)&pSrcW, 0L, EQF_SEGLEN * sizeof(CHAR_W) * 2, NOMSG );
     if ( pSrcW )
     {
       PSZ_W pTgtW = pSrcW + EQF_SEGLEN;
       ASCII2Unicode( pszSrc, pSrcW, 0L );
       ASCII2Unicode( pszTgt, pTgtW, 0L );

       usRc = EQFDELSEGW( pSrcW, pTgtW, (ULONG) usSegNum );

       UtlAlloc( (PVOID *)&pSrcW, 0L, 0L, NOMSG );
     }
     else
     {
       usRc = EQFRS_NO_INIT;                      // no generic structure yet
     }
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFDELSEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDELSEGW
(
   PSZ_W     pszSrc,                            // source segment
   PSZ_W     pszTgt,                            // target segment
   ULONG     ulSegNum                           // segment number
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code


  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_DELSEGW;
     pstEQFPCmd->ulParm1 = ulSegNum;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pszSrc) + 1);
     pstEQFPCmd->usLen2  = (USHORT)(UTF16strlenCHAR(pszTgt) + 1);
     memcpy (pstEQFPCmd->ucbBuffer, (PVOID)pszSrc, pstEQFPCmd->usLen1 * sizeof(CHAR_W));
     memcpy (pstEQFPCmd->ucbBuffer+pstEQFPCmd->usLen1 * sizeof(CHAR_W), (PVOID)pszTgt, pstEQFPCmd->usLen2 * sizeof(CHAR_W) );

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_DELSEG, NULL, NULL);

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFDELSEG'

/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETSEGNUM                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETSEGNUM( PSZ, PSZ, USHORT );                        |
//+----------------------------------------------------------------------------+
//|Description:       This function gets the number of the currently active    |
//|                   segment.                                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PULONG      pointer to ULONG variable receiving the      |
//|                               segment number                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETSEGNUM (PULONG pulSegNum )
{
   return ( EQFXGETSEGNUM( pstEQFGeneric, pulSegNum ));

}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXGETSEGNUM                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXGETSEGNUM( PSTEQFGEN, PULONG );                      |
//+----------------------------------------------------------------------------+
//|Description:       This function gets the number of the currently active    |
//|                   segment.                                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PULONG      pointer to ULONG variable receiving the      |
//|                               segment number                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT EQFXGETSEGNUM
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   PULONG pulSegNum                             // pointer to buffer for segment numer
)
{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_GETSEGNUM;
     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_GETSEGNUM, NULL, NULL);

     *pulSegNum = MAKELONG( pstEQFPCmd->usParm2, pstEQFPCmd->ulParm1 );

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFXGETSEGNUM'


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFEXTSEG                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFEXTSEG( PSZ, USHORT );                                |
//+----------------------------------------------------------------------------+
//|Description:       This function will extract the segment with the passed   |
//|                   segment number                                           |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         pointer to segment buffer                    |
//|                   USHORT      segment number                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXTRANSSEG with the appropriate parameters        |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFEXTSEG (PSZ pszBuffer, USHORT usNum)
{
    return ( EQFXEXTSEG (pstEQFGeneric, pszBuffer, usNum));
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXEXTSEG                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXEXTSEG( PSTEQFGEN, PSZ, USHORT );                    |
//+----------------------------------------------------------------------------+
//|Description:       This function will extract the segment with the passed   |
//|                   segment number                                           |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         pointer to segment buffer                    |
//|                   USHORT      segment number                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXEXTSEG
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   PSZ pszBuffer,                               // buffer for the extracted seg
   USHORT usNum                                 // segment number
)
{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code
  ULONG         ulOEMCP = 0L;

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_EXTSEGW;
     pstEQFPCmd->ulParm1 = usNum;
     pstEQFPCmd->usLen1  =
     pstEQFPCmd->usLen2  = 0;

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_EXTSEG, NULL, NULL);
     if ( !pstEQFGen->usRC && pszBuffer)
     {
       PDOCUMENT_IDA pIdaDoc = NULL;
       pIdaDoc  = (PDOCUMENT_IDA) pstEQFGen->pDoc;
       // TODO @@ Should we use szDocSOurceLang??
       ulOEMCP = GetLangOEMCP( pIdaDoc->szDocTargetLang);
       Unicode2ASCIIBuf( (PSZ_W)&pstEQFPCmd->ucbBuffer[0], pszBuffer,
                         (USHORT)(UTF16strlenCHAR((PSZ_W)pstEQFPCmd->ucbBuffer)+1),
                         min (pstEQFPCmd->usLen1, EQF_SEGLEN),
                         ulOEMCP);
     }
     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFEXTSEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFEXTSEGW
(
   PSZ_W     pszBuffer,                         // buffer for the extracted seg
   ULONG     ulNum                              // segment number
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code


  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_EXTSEGW;
     pstEQFPCmd->ulParm1 = ulNum;
     pstEQFPCmd->usLen1  =
     pstEQFPCmd->usLen2  = 0;

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_EXTSEG, NULL, NULL);
     if ( !pstEQFGen->usRC && pszBuffer)
     {
       memcpy ((PVOID)pszBuffer, pstEQFPCmd->ucbBuffer,
                 min (pstEQFPCmd->usLen1 * sizeof(CHAR_W), EQF_SEGLEN * sizeof(CHAR_W)));
     }
     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFEXTSEG'


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETTMNAME                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFEXTSEG( PSZ, PSZ, PSZ );                              |
//+----------------------------------------------------------------------------+
//|Description:       This function will retrieve the translation memory name  |
//|                   for a given folder                                       |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         pointer to folder name                       |
//|                   PSZ         pointer to folder path                       |
//|                   PSZ         buffer for translation memory name           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     open properties of the folder                            |
//|                   extract translation memory name                          |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETTMNAME (PSZ    pszFolder,
                              PSZ    pszPath,
                              PSZ    pszTMName)
{
 BOOL            fOk = TRUE;             // success indicator
 ULONG           ulErrorInfo;             //error indicator
 HPROP           hpropFolder;            //handel of folder properties
 PPROPFOLDER     ppropFolder;                    //pointer to folder properties
 CHAR            szFolder[CCHMAXPATH];

 strcpy( szFolder, pszPath);
 strcat( szFolder, pszFolder);
 if ((hpropFolder = OpenProperties( szFolder, NULL,
                                     PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
 {
    fOk = FALSE;
 } /* endif */
 if ( fOk )
 {
    ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( hpropFolder );
    strcpy( pszTMName, ppropFolder->szMemory);
    CloseProperties( hpropFolder, 0, &ulErrorInfo);
 } /* endif */

 return ( (USHORT)fOk );

}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFPROOF                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     EQFPROOF ( PSZ, PCHAR, PUSHORT );                        |
//+----------------------------------------------------------------------------+
//|Description:       This function performs proof reading against the passed  |
//|                   string                                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         pointer to segment buffer                    |
//|                   PCHAR       output area filled with list of misspelled   |
//|                               words                                        |
//|                               Two consecutive '\0' indicate end of list    |
//|                   PUSHORT     length of area                               |
//|                               at output this will be filled with number of |
//|                               characters filled                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       OK        everything okay                                |
//|                   EQFRS_AREA_TOO_SMALL    passed area too small            |
//|                   EQFRS_NOMORPH_DICT      no morphological dictionary      |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXPROOF    with the appropriate parameters        |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFPROOF
(
  PSZ  pszData,
  PCHAR pOutData,
  PUSHORT  pusLen
)
{
   return ( EQFXPROOF (pstEQFGeneric, pszData, pOutData, pusLen));
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXPROOF                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXPROOF ( PSTEQFGEN, PSZ, PCHAR, PUSHORT );            |
//+----------------------------------------------------------------------------+
//|Description:       This function performs proof reading against the passed  |
//|                   string                                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         pointer to segment buffer                    |
//|                   PCHAR       output area filled with list of misspelled   |
//|                               words                                        |
//|                               Two consecutive '\0' indicate end of list    |
//|                   PUSHORT     length of area                               |
//|                               at output this will be filled with number of |
//|                               characters filled                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       OK        everything okay                                |
//|                   EQFRS_AREA_TOO_SMALL    passed area too small            |
//|                   EQFRS_NOMORPH_DICT      no morphological dictionary      |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXPROOF
(
  PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
  PSZ  pszData,                                // pointer to string
  PCHAR pOutData,                              // pointer to output field
  PUSHORT  pusLen                              // length of output field
)
{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {
    USHORT usLen = *pusLen;
    PSZ    pTemp = pszData;
    PBYTE pTemp1 = (PBYTE) pOutData;
    pstEQFPCmd = pstEQFGen->pstEQFPCmd;

    pstEQFPCmd->usCmd   = EQFCMD_PROOF;
    pstEQFPCmd->ulParm1 = 0;
    pstEQFPCmd->usLen1  = (USHORT)(strlen (pTemp) + 1);
    pstEQFPCmd->usLen2  = 0;
    memcpy (pstEQFPCmd->ucbBuffer, pTemp, pstEQFPCmd->usLen1 );
    pstEQFGen->usRC = 0;
    WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_PROOF, NULL, NULL);
    if ( usLen )
    {
      usLen = min( pstEQFPCmd->usLen1, usLen-1 );
      memcpy ( pTemp1, pstEQFPCmd->ucbBuffer, usLen );
    }
    else
    {
      *pTemp1 = EOS;
    } /* endif */
    *pusLen = usLen;
    usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFPROOF'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFPROOFW
(
  PSZ_W     pszData,                           // pointer to string
  PCHAR_W   pOutData,                          // pointer to output field
  PUSHORT   pusLen                             // length of output field
)
{
  PSTEQFGEN pstEQFGen = pstEQFGeneric;          // pointer to generic struct.
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code


  if ( pstEQFGen )
  {
    USHORT usLen = *pusLen;
    PSZ_W    pTemp = pszData;
    PBYTE pTemp1 = (PBYTE) pOutData;
    pstEQFPCmd = pstEQFGen->pstEQFPCmd;

    pstEQFPCmd->usCmd   = EQFCMD_PROOFW;
    pstEQFPCmd->ulParm1 = 0;
    pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pTemp) + 1);
    pstEQFPCmd->usLen2  = 0;
    memcpy (pstEQFPCmd->ucbBuffer, (PVOID)pTemp, pstEQFPCmd->usLen1 * sizeof(CHAR_W));
    pstEQFGen->usRC = 0;
    WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_PROOF, NULL, NULL);
    if ( usLen )
    {
      usLen = min( pstEQFPCmd->usLen1, (usLen-1) );
      memcpy ( pTemp1, pstEQFPCmd->ucbBuffer, usLen *sizeof(CHAR_W));
    }
    else
    {
      *pTemp1 = EOS;
    } /* endif */
    *pusLen = usLen;
    usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFPROOF'


/* $PAGEIF20 */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFPROOFADD                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFPROOFADD ( PSZ );                                     |
//+----------------------------------------------------------------------------+
//|Description:       This function adds the passed word to the addenda        |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         word to be added to the addenda              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       OK        everything okay                                |
//|                   EQFRS_NOADDENDA_DICT    no addenda dictionary available. |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXPROOFADD with the appropriate parameters        |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFPROOFADD
(
   PSZ pszData
)
{
   return ( EQFXPROOFADD (pstEQFGeneric, pszData));
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXPROOFADD                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXPROOFADD ( PSTEQFGEN, PSZ );                         |
//+----------------------------------------------------------------------------+
//|Description:       This function adds the passed word to the addenda        |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         word to be added to the addenda              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       OK        everything okay                                |
//|                   EQFRS_NOADDENDA_DICT    no addenda dictionary available. |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT EQFXPROOFADD
(
  PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
  PSZ  pszData                                 // pointer to string
)
{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {
     PSZ pTemp = pszData;
     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_PROOFADD;
     pstEQFPCmd->ulParm1 = 0;
     pstEQFPCmd->usLen1  = (USHORT)(strlen (pTemp) + 1);
     pstEQFPCmd->usLen2  = 0;
     memcpy (pstEQFPCmd->ucbBuffer, pTemp, pstEQFPCmd->usLen1 );


     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_PROOFADD, NULL, NULL);
     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFPROOFADD'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFPROOFADDW
(
  PSZ_W     pszData                            // pointer to string
)
{
  PSTEQFGEN pstEQFGen = pstEQFGeneric;         // pointer to generic struct.

  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {
     PSZ_W pTemp = pszData;
     pstEQFPCmd  = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_PROOFADDW;
     pstEQFPCmd->ulParm1 = 0;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pTemp) + 1);
     pstEQFPCmd->usLen2  = 0;
     memcpy (pstEQFPCmd->ucbBuffer, (PVOID)pTemp, pstEQFPCmd->usLen1 * sizeof(CHAR_W));


     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_PROOFADD, NULL, NULL);
     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFPROOFADD'

/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFPROOFAID                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFPROOFAID( PSZ, PCHAR, PUSHORT );                      |
//+----------------------------------------------------------------------------+
//|Description:       This function gets the provided aid for the passed word  |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         pointer to segment buffer                    |
//|                   PCHAR       output area filled with list of misspelled   |
//|                               words                                        |
//|                               Two consecutive '\0' indicate end of list    |
//|                   PUSHORT     length of area                               |
//|                               at output this will be filled with number of |
//|                               characters filled                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       OK        everything okay                                |
//|                   EQFRS_AREA_TOO_SMALL    passed area too small            |
//|                   EQFRS_NOMORPH_DICT      no morphological dictionary      |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXPROOFAID with the appropriate parameters        |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFPROOFAID
(
  PSZ  pszData,                      // pointer to data
  PCHAR pOutData,                    // pointer to output area
  PUSHORT  pusLen                    // pointer to filled length
)
{
   return ( EQFXPROOFAID (pstEQFGeneric, pszData, pOutData, pusLen));
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXPROOFAID                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXPROOFAID( PSTEQFGEN, PSZ, PCHAR, PUSHORT );          |
//+----------------------------------------------------------------------------+
//|Description:       This function gets the provided aid for the passed word  |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         pointer to segment buffer                    |
//|                   PCHAR       output area filled with list of misspelled   |
//|                               words                                        |
//|                               Two consecutive '\0' indicate end of list    |
//|                   PUSHORT     length of area                               |
//|                               at output this will be filled with number of |
//|                               characters filled                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       OK        everything okay                                |
//|                   EQFRS_AREA_TOO_SMALL    passed area too small            |
//|                   EQFRS_NOMORPH_DICT      no morphological dictionary      |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXPROOFAID
(
  PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
  PSZ  pszData,                                // pointer to string
  PCHAR pOutData,                              // pointer to output field
  PUSHORT  pusLen                              // length of output field
)
{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {
     PSZ  pTemp = pszData;
     USHORT usLen = *pusLen;
     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_PROOFAID;
     pstEQFPCmd->ulParm1 = 0;
     pstEQFPCmd->usLen1  = (USHORT)(strlen (pTemp) + 1);
     pstEQFPCmd->usLen2  = 0;
     memcpy (pstEQFPCmd->ucbBuffer, pTemp, pstEQFPCmd->usLen1 );

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_PROOFAID, NULL, NULL);

     pTemp = pOutData;             // force 16-32 bit conversion
     if ( usLen )
     {
       usLen = min( pstEQFPCmd->usLen1, usLen-1 );
       memcpy (pTemp, pstEQFPCmd->ucbBuffer, usLen );
     }
     else
     {
       *pTemp = EOS;
     } /* endif */
     *pusLen = usLen;
     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFPROOFAID'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFPROOFAIDW
(
  PSZ_W     pszData,                           // pointer to string
  PCHAR_W   pOutData,                          // pointer to output field
  PUSHORT   pusLen                             // length of output field
)
{
  PSTEQFGEN pstEQFGen = pstEQFGeneric;          // pointer to generic struct.

  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {
     PSZ_W  pTemp = pszData;
     USHORT usLen = *pusLen;
     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_PROOFAIDW;
     pstEQFPCmd->ulParm1 = 0;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pTemp) + 1);
     pstEQFPCmd->usLen2  = 0;
     memcpy (pstEQFPCmd->ucbBuffer, (PVOID) pTemp, pstEQFPCmd->usLen1 * sizeof(CHAR_W));

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_PROOFAID, NULL, NULL);

     pTemp = pOutData;             // force 16-32 bit conversion
     if ( usLen )
     {
       usLen = min( pstEQFPCmd->usLen1, (usLen-1) );
       memcpy ((PVOID)pTemp, pstEQFPCmd->ucbBuffer, usLen * sizeof(CHAR_W));
     }
     else
     {
       *pTemp = EOS;
     } /* endif */
     *pusLen = usLen;
     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFPROOFAID'



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFSHMEM                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     EQFSHMEM ( PSTEQFGEN );                                  |
//+----------------------------------------------------------------------------+
//|Description:       This function return the pointer to the shared memory    |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       as listed in OTMAPI.H                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     get pointer to shared memory;                            |
//|                   if error then                                            |
//|                     return NULL                                            |
//|                   else                                                     |
//|                     return pointer to shared memory                        |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
PSTEQFGEN __cdecl /*APIENTRY*/ EQFSHMEM (PSTEQFGEN pstEQFShare)

{
  PSTEQFGEN     pGen;

  pstEQFShare;
  pGen = pstEQFGeneric;

  return pGen;
} // end 'EQFSHMEM'


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFSYSRC                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     EQFSYSRC ( VOID );                                       |
//+----------------------------------------------------------------------------+
//|Description:       return the system error code                             |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       as listed in OTMAPI.H                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     return system error code                                 |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/  EQFSYSRC (VOID)

{
  return usEQFSysRc;
} // end 'EQFSYSRC'

/*******************************************************************************
*
*       function:       EQFERRID
*
*******************************************************************************/

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFERRID                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     EQFERRID ( VOID );                                       |
//+----------------------------------------------------------------------------+
//|Description:       return the set error id                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       as listed in OTMAPI.H                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     return last error or EQFRS_NO_INIT if not init.          |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/  EQFERRID (VOID)

{
  USHORT usRc;                         // return code

  if ( pstEQFGeneric )
  {
     usRc = pstEQFGeneric->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFERRID'
/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFERRINS                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFERRINS( VOID );                                       |
//+----------------------------------------------------------------------------+
//|Description:       fill the message buffer with the last error              |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       pointer to buffer                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXERRINS with the appropriate parameter.          |
//|                   return the error from the function call                  |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
PSZ __cdecl /*APIENTRY*/     EQFERRINS (VOID)
{
    return ( EQFXERRINS( pstEQFGeneric ));
}

__declspec(dllexport)
PSZ_W __cdecl /*APIENTRY*/   EQFERRINSW (VOID)
{
    return ( EQFXERRINSW( pstEQFGeneric ));
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXERRINS                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXERRINS( PSTEQFGEN );                                 |
//+----------------------------------------------------------------------------+
//|Description:       return pointer to message buffer filled with last error  |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN    pointer to generic structure                |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       pointer to message buffer                                |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen then                                        |
//|                     set pointer to message buffer                          |
//|                   else                                                     |
//|                     pass back empty string                                 |
//|                   endif                                                    |
//|                   return pointer                                           |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
PSZ     EQFXERRINS
(
   PSTEQFGEN pstEQFGen
)

{
   PSZ  pData = (PSZ)&szMsgBuf[0];

   if ( pstEQFGen )
   {
     Unicode2ASCII( pstEQFGen->szMsgBuffer, pData, 0L );
   }
   else
   {
      szMsgBuf[0] = '\0';           // give empty string back
      pData = (PSZ)szMsgBuf;
   } /* endif */
   return pData;
} // end 'EQFXERRINS'

__declspec(dllexport)
PSZ_W  EQFXERRINSW
(
   PSTEQFGEN pstEQFGen
)
{
   PSZ_W pData;

   if ( pstEQFGen )
   {
     pData = pstEQFGen->szMsgBuffer;
   }
   else
   {
      szMsgBuf[0] = '\0';           // give empty string back
      pData = szMsgBuf;
   } /* endif */
   return pData;
} // end 'EQFXERRINS'

/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFERRMSG                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFERRMSG( VOID );                                       |
//+----------------------------------------------------------------------------+
//|Description:       return pointer to message buffer filled with error msg   |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       pointer to message buffer                                |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXERRINS with the appropriate parameter.          |
//|                   return the error from the function call                  |
//+----------------------------------------------------------------------------+


__declspec(dllexport)
PSZ __cdecl /*APIENTRY*/     EQFERRMSG (VOID)
{
   return (EQFXERRMSG( pstEQFGeneric ));
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXERRMSG                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXERRMSG( PSTEQFGEN );                                 |
//+----------------------------------------------------------------------------+
//|Description:       return pointer to message buffer filled with error msg   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN    pointer to generic structure                |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       pointer to message buffer                                |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen then                                        |
//|                     issue DosGetMessage with the currently active return   |
//|                     code                                                   |
//|                   else                                                     |
//|                     pass back empty string                                 |
//|                   endif                                                    |
//|                   return pointer                                           |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
PSZ     EQFXERRMSG
(
   PSTEQFGEN pstEQFGen
)
{
  PSZ           pszBuffer, pMsgText;
  PSTEQFPCMD    pstEQFPCmd;                      // pointer to packed struct
  PSZ           pMsg[3];

  if ( pstEQFGen )
  {
    szMsgFile[0] = pstEQFGen->chSysDrive;       // set system drive
    pstEQFPCmd = pstEQFGen->pstEQFPCmd;


    if ( pstEQFGen->usRC )
    {
      ULONG    Length = 0;

      pMsg[1] = pMsg[2] = "";                   // init to empty string
      pMsg[0] = (PSZ)(pstEQFPCmd->ucbBuffer);

      DosGetMessage (&pMsg[0], 2,
                     (PSZ)&szMsgBuf[0], EQF_MSGBUF_SIZE - 1,
                     pstEQFGen->usRC, szMsgFile, &Length);
      *(szMsgBuf+Length) = '\0';

      // remove CR/LF, add LF for backslash-n
      pMsgText = pszBuffer = (PSZ)&szMsgBuf[0];
      while (*pMsgText != '\0')
      {
        switch (*pMsgText)
        {
        case '\\':
             *pszBuffer++ = *pMsgText++;    // move character to target
             if ( *pMsgText == 'n' )        // if backslash is followed by 'n' ..
             {
                pMsgText++;                 // ... skip it and
                *(pszBuffer-1) = '\n';      // replace backslash with linefeed
             } // endif
             break;
        case '\r':
             pMsgText++;                    // ignore carriage return
             break;
        case '\n':
             *pszBuffer++ = ' ';            // replace linefeed with space
             pMsgText++;
             break;
        default:
             *pszBuffer++ = *pMsgText++;    // copy character to output buffer
             break;
        } // endswitch
      } // endwhile
      *(PUSHORT)pszBuffer = 0;                    // terminate string
    } // endif (valid message)
    else
      *(PUSHORT)szMsgBuf = 0;

  }
  else
  {
     szMsgBuf[0] = '\0';
  } /* endif */
  return (PSZ)&szMsgBuf[0];
} // end 'EQFERRMSG'

__declspec(dllexport)
PSZ_W __cdecl /*APIENTRY*/   EQFERRMSGW( VOID )
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // set generic pointer
  PSZ_W         pszBuffer, pMsgText;
  PSTEQFPCMD    pstEQFPCmd;                      // pointer to packed struct
  PSZ           pMsg[3];

  if ( pstEQFGen )
  {
    szMsgFile[0] = pstEQFGen->chSysDrive;       // set system drive
    pstEQFPCmd = pstEQFGen->pstEQFPCmd;


    if ( pstEQFGen->usRC )
    {
      CHAR          szBuf[EQF_MSGBUF_SIZE];
      ULONG     Length = 0;

      pMsg[1] = pMsg[2] = "";                   // init to empty string
      pMsg[0] = (PSZ)(pstEQFPCmd->ucbBuffer);

      DosGetMessage (&pMsg[0], 2,
                     szBuf, EQF_MSGBUF_SIZE - 1,
                     pstEQFGen->usRC, szMsgFile, &Length);
      *(szBuf+Length) = '\0';
      ASCII2Unicode( szBuf, szMsgBuf, 0L );

      // remove CR/LF, add LF for backslash-n
      pMsgText = pszBuffer = szMsgBuf;
      while (*pMsgText != '\0')
      {
        switch (*pMsgText)
        {
        case '\\':
             *pszBuffer++ = *pMsgText++;    // move character to target
             if ( *pMsgText == 'n' )        // if backslash is followed by 'n' ..
             {
                pMsgText++;                 // ... skip it and
                *(pszBuffer-1) = '\n';      // replace backslash with linefeed
             } // endif
             break;
        case '\r':
             pMsgText++;                    // ignore carriage return
             break;
        case '\n':
             *pszBuffer++ = ' ';            // replace linefeed with space
             pMsgText++;
             break;
        default:
             *pszBuffer++ = *pMsgText++;    // copy character to output buffer
             break;
        } // endswitch
      } // endwhile
      *(PUSHORT)pszBuffer = 0;                    // terminate string
    } // endif (valid message)
    else
      *(PUSHORT)szMsgBuf = 0;

  }
  else
  {
     szMsgBuf[0] = '\0';
  } /* endif */
  return &szMsgBuf[0];
} // end 'EQFERRMSG'


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFDUMPSEG                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFDUMPSEG( PSZ, PSZ, USHORT, PCHAR, PCHAR );            |
//+----------------------------------------------------------------------------+
//|Description:       return the translation memory proposals and the dict.hits|
//|                   for the passed segment.                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ          pointer to source segment                   |
//|                   PSZ          pointer to filename                         |
//|                   USHORT       segment number                              |
//|                   PCHAR        pointer to output area for TM hits          |
//|                   PCHAR        pointer to output area for dictionary hits. |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       as defined in OTMAPI.H                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXDUMPSEG with the appropriate parameter.         |
//|                   return the error from the function call                  |
//+----------------------------------------------------------------------------+


__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDUMPSEG (PSZ pszSrc,
                            PSZ pFileName,
                            USHORT usSegNum,
                            PCHAR pchProp, PCHAR pchDict)
{
   return( EQFXDUMPSEG (pstEQFGeneric, pszSrc, pFileName,
                        usSegNum, FALSE, pchProp, pchDict));

}

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDUMPSEG2(PSZ pszSrc,
                            PSZ pFileName,
                            USHORT usSegNum,
                            USHORT usFlags,
                            PCHAR pchProp, PCHAR pchDict)
{
   return( EQFXDUMPSEG (pstEQFGeneric, pszSrc, pFileName,
                        usSegNum, usFlags, pchProp, pchDict));

}

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDUMPSEGW (PSZ_W pszSrc,
                               PSZ   pFileName,
                               ULONG ulSegNum,
                               PCHAR_W pchProp, PCHAR_W pchDict)
{
   return( EQFDUMPSEG2W (pszSrc, pFileName, ulSegNum, 0, pchProp, pchDict));
}





//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXDUMPSEG                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXDUMPSEG( PSTEQFGEN, PSZ, PSZ, USHORT, PCHAR, PCHAR );|
//+----------------------------------------------------------------------------+
//|Description:       return the translation memory proposals and the dict.hits|
//|                   for the passed segment.                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN    pointer to generic structure                |
//|                   PSZ          pointer to source segment                   |
//|                   PSZ          pointer to filename                         |
//|                   USHORT       segment number                              |
//|                   PCHAR        pointer to output area for TM hits          |
//|                   PCHAR        pointer to output area for dictionary hits. |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       as defined in OTMAPI.H                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen then                                        |
//|                     prepare call and send message to document handler;     |
//|                     set return code                                        |
//|                     if okay then                                           |
//|                       copy dictionary hits into the provided buffer        |
//|                       copy transl.mem hits into the provided buffer        |
//|                     endif                                                  |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return return code                                       |
//+----------------------------------------------------------------------------+


__declspec(dllexport)
USHORT EQFXDUMPSEG
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   PSZ pszSrc,                                  // source segment
   PSZ pFileName,                               // pointer to file name
   USHORT  usSegNum,                            // segment number
   USHORT  usFlags,                             // retrieve indication for flags
   PCHAR pchProp,                               // proposal area
   PCHAR pchDict                                // dictionary area
)
{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  PSTEQFDUMP    pstEQFDump;                     // pointer to dump area
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;
     pstEQFPCmd->usCmd   = EQFCMD_DUMPSEG;
     pstEQFPCmd->ulParm1 = usSegNum;
     pstEQFPCmd->usParm2 = usFlags;
     pstEQFPCmd->usLen1  = (USHORT)(strlen (pszSrc) + 1);
     pstEQFPCmd->usLen2  = (USHORT)(strlen (pFileName) + 1);
     memcpy (pstEQFPCmd->ucbBuffer, pszSrc, pstEQFPCmd->usLen1);
     memcpy (pstEQFPCmd->ucbBuffer+pstEQFPCmd->usLen1,
             pFileName, pstEQFPCmd->usLen2 );
     if ( pstEQFGen->hwndTWBS )
     {
        pstEQFGen->usRC = 0;
        WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_DUMPSEG, NULL, NULL);
        if ( !pstEQFGen->usRC )
        {
           pstEQFDump = ( PSTEQFDUMP ) (pstEQFPCmd + 1);
           // copy proposal and dictionary buffer
           memcpy( pchProp, pstEQFDump->chProp, EQF_TGTLEN );
           memcpy( pchDict, pstEQFDump->chDict, EQF_DICTLEN );
        } /* endif */
        usEQFErrID =  pstEQFGen->usRC ;

        usRc = pstEQFGen->usRC;
     }
     else
     {
        usRc = EQFRS_NO_INIT;                  // no generic structure yet
     } /* endif */
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  usEQFErrID =  usRc ;
  return usRc;
} // end 'EQFDUMPSEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDUMPSEG2W
(
   PSZ_W     pszSrc,                            // source segment
   PSZ       pFileName,                         // pointer to file name
   ULONG     ulSegNum,                          // segment number
   USHORT    usFlags,                           // retrieve indication for flags
   PCHAR_W   pchProp,                           // proposal area
   PCHAR_W   pchDict                            // dictionary area
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct.
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  PSTEQFDUMP    pstEQFDump;                     // pointer to dump area
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;
     pstEQFPCmd->usCmd   = EQFCMD_DUMPSEGW;
     pstEQFPCmd->ulParm1 = ulSegNum;
     pstEQFPCmd->usParm2 = usFlags;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pszSrc) + 1);
     pstEQFPCmd->usLen2  = (USHORT)(strlen (pFileName) + 1);
     memcpy (pstEQFPCmd->ucbBuffer, (PVOID)pszSrc, pstEQFPCmd->usLen1 * sizeof(CHAR_W));
     memcpy (pstEQFPCmd->ucbBuffer+pstEQFPCmd->usLen1* sizeof(CHAR_W), pFileName, pstEQFPCmd->usLen2 );
     if ( pstEQFGen->hwndTWBS )
     {
        pstEQFGen->usRC = 0;
        WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_DUMPSEG, NULL, NULL);
        if ( !pstEQFGen->usRC )
        {
           pstEQFDump = ( PSTEQFDUMP ) (pstEQFPCmd + 1);
           // copy proposal and dictionary buffer
           memcpy( (PVOID)pchProp, pstEQFDump->chProp, EQF_TGTLEN  * sizeof(CHAR_W));
           memcpy( (PVOID)pchDict, pstEQFDump->chDict, EQF_DICTLEN * sizeof(CHAR_W));
        } /* endif */
        usEQFErrID =  pstEQFGen->usRC ;

        usRc = pstEQFGen->usRC;
     }
     else
     {
        usRc = EQFRS_NO_INIT;                  // no generic structure yet
     } /* endif */
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  usEQFErrID =  usRc ;
  return usRc;
} // end 'EQFDUMPSEG'

/* $PAGEIF20 */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQF_PackBuf                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQF_PackBuf( PUCHAR, PSZ * );                            |
//+----------------------------------------------------------------------------+
//|Description:       flatten a list of pointers into one string               |
//+----------------------------------------------------------------------------+
//|Parameters:        PUCHAR       pointer to character buffer                 |
//|                   PSZ *        pointer to array of strings                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       as defined in OTMAPI.H                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     while ( pointer available )                              |
//|                     copy string  into buffer                               |
//|                     increase buffer position and length;                   |
//|                     point to next item;                                    |
//|                   endwhile                                                 |
//|                   return packed length                                     |
//+----------------------------------------------------------------------------+
USHORT  EQF_PackBuf (PUCHAR pucBuff, PSZ *apszItems)
{
  ULONG        ulItemLen, ulLen = 0;

  while ((*apszItems != NULC) && ((ulItemLen = strlen (*apszItems)) != 0))
  {
    memcpy (&pucBuff[ulLen], *apszItems, ulItemLen);
    ulLen     += ulItemLen + 2;
    apszItems += 1;
  } // endwile (valid item)

  return (USHORT)(ulLen + 2);
}                                      // end 'EQF_PackBuf'

USHORT  EQF_PackBufW (PUCHAR pucBuff, PSZ_W *apszItems)
{
  ULONG        ulItemLen, ulLen = 0;

  while ((*apszItems != NULC) && ((ulItemLen = UTF16strlenCHAR (*apszItems))!= 0))
  {
    memcpy (&pucBuff[ulLen*sizeof(CHAR_W)], *apszItems, ulItemLen * sizeof(CHAR_W));
    ulLen     += ulItemLen + 2*sizeof(CHAR_W);
    apszItems += 1;
  } // endwile (valid item)

  return (USHORT)(ulLen + 2*sizeof(CHAR_W));
}                                      // end 'EQF_PackBuf'


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFAllocate                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFAllocate( VOID );                                     |
//+----------------------------------------------------------------------------+
//|Description:       allocate memory for the generic structure                |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       as defined in OTMAPI.H                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     use DosAllocShrSeg to allocate memory for generic struct |
//|                   if okay then                                             |
//|                     init memory                                            |
//|                   else if error ERROR_ALREADY_EXISTS then                  |
//|                     get shared segment pointer                             |
//|                     if okay init it                                        |
//|                   endif                                                    |
//|                   return return code                                       |
//+----------------------------------------------------------------------------+
static
USHORT  EQFAllocate (VOID)
{

  /********************************************************************/
  /* Windows version                                                  */
  /********************************************************************/
  USHORT        usRc = 0;
  PSTEQFGEN     pstEQFGen;
  LONG          lLen;

  /*-------------------------------------------------------------------------*
  * Allocate memory for generic struct
  *-------------------------------------------------------------------------*/
  lLen = sizeof( STEQFGEN ) + sizeof( STEQFPCMD ) + sizeof( STEQFDUMP )
                             + sizeof( EQFXLATE );
#ifdef USE_GLOBALALLOC_FOR_STEQFGEN
  sel = GlobalAlloc( GMEM_SHARE | GPTR, lLen );
  if ( ! sel )
  {
    usRc = ERROR_STORAGE;
  } /* endif */

  if ( ! usRc )
  {
    pstEQFGeneric = pstEQFGen = (PSTEQFGEN) GlobalLock( sel );

    memset (pstEQFGen, '\0', sizeof (STEQFGEN));
    pstEQFGen->pstEQFPCmd = (PSTEQFPCMD) (pstEQFGen+1);
  }
#else
  hMapObject = CreateFileMapping(
                 (HANDLE)0xFFFFFFFF,   // use page file
                 NULL,                 // no security attrib
                 PAGE_READWRITE,       // read/write access
                 0,                    // size: high 32bits
                 lLen,                // size: low 32bit
                 EQFNM_SHSEG );        // name of file mapping
  if ( hMapObject == NULL )
  {
    usRc = ERROR_STORAGE;
  }
  else
  {
    pstEQFGeneric = pstEQFGen =
    (PSTEQFGEN)MapViewOfFile( hMapObject,         // object to map view of
                   FILE_MAP_WRITE,     // read/write access
                   0,                  // high offset: map from
                   0,                  // log offset:  begin
                   0 );                // default: map entire file

    if ( pstEQFGeneric == NULL )
    {
      usRc = ERROR_STORAGE;
    }
    else
    {
      memset (pstEQFGen, '\0', sizeof (STEQFGEN));
      pstEQFGen->pstEQFPCmd = (PSTEQFPCMD) (pstEQFGen+1);
    } /* endif */
  } /* endif */
#endif
  return usRc;

} // end 'EQFAllocate'
/* $PAGEIF20 */

__declspec(dllexport)
VOID __cdecl /*APIENTRY*/ EQFSETSLIDER
(
  HWND      hwndSlider,                // window handle of slider control
  USHORT    usPercentage               // complete percentage 0 - 100 [%]
)
{
  WinSendMsg( hwndSlider, WM_EQF_UPDATESLIDER,
              MP1FROMSHORT(usPercentage), NULL );
} /* end of function EQFSETSLIDER */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETSOURCE                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETSOURCE ( USHORT );                                 |
//+----------------------------------------------------------------------------+
//|Description:       This function will display the source of proposal window |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT  usType   EQF_ACTIVATE is the only supported type |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXGETSOURCE with the appropriate parameters       |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETSOURCE (USHORT usNum)
{
   return( EQFXGETSOURCE (pstEQFGeneric, usNum));
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXGETSOURCE                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXGETSOURCE( PSTEQFGEN, USHORT );                      |
//+----------------------------------------------------------------------------+
//|Description:       This function will try to activate the source of proposal|
//|                   window                                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   USHORT      required action                              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler                       |
//|                     set return code                                        |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT EQFXGETSOURCE
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   USHORT usAction                              // requested action
)

{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;


     pstEQFPCmd->usCmd   = EQFCMD_GETSOURCE;
     pstEQFPCmd->ulParm1 = usAction;
     pstEQFPCmd->usLen1  =
     pstEQFPCmd->usLen2  = 0;

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_GETSOURCE, NULL, NULL);

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFGETSOURCE'
/* $PAGEIF20 */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFDICTEDIT                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXDICTEDIT( PSZ );                                     |
//+----------------------------------------------------------------------------+
//|Description:       This function will try to edit the specified dict. entry |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         pointer to word to be looked up              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXDICTEDIT with the appropriate parameters        |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDICTEDIT ( PSZ pszWord )
{
    return( EQFXDICTEDIT (pstEQFGeneric, pszWord ));
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXDICTEDIT                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXDICTEDIT(PSTEQFGEN, PSZ );                           |
//+----------------------------------------------------------------------------+
//|Description:       This function will try to edit the specified dict. entry |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         pointer to word to be looked up              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       see list in OTMAPI.H                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT EQFXDICTEDIT
(
   PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
   PSZ pszWord
)
{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;


     pstEQFPCmd->usCmd   = EQFCMD_DICTEDIT;
     pstEQFPCmd->usLen1  = (USHORT)(strlen (pszWord) + 1);
     memcpy (pstEQFPCmd->ucbBuffer, pszWord, pstEQFPCmd->usLen1 );

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_DICTEDIT, NULL, NULL);
     usEQFErrID = pstEQFGen->usRC;

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFXDICTEDIT'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFDICTEDITW
(
   PSZ_W pszWord
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct.
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;


     pstEQFPCmd->usCmd   = EQFCMD_DICTEDITW;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR(pszWord) + 1);
     memcpy (pstEQFPCmd->ucbBuffer, pszWord, pstEQFPCmd->usLen1 * sizeof(CHAR_W));

     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_DICTEDIT, NULL, NULL);
     usEQFErrID = pstEQFGen->usRC;

     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;
} // end 'EQFDICTEDITW'


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETSTRUCT                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETSTRUCT ( )                                         |
//+----------------------------------------------------------------------------+
//|Description:       This function will retrieve a pointer to the generic     |
//|                   structure.                                               |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSTEQFGEN                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGeneric not set allocate the structure and      |
//|                   pass back the pointer ...                                |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
PSTEQFGEN __cdecl /*APIENTRY*/ EQFGETSTRUCT ( VOID )

{
  if ( !pstEQFGeneric )
  {
    EQFAllocate();
  } /* endif */

  return ( pstEQFGeneric );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFFREESTRUCT                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFFREESTRUCT ( )                                        |
//+----------------------------------------------------------------------------+
//|Description:       This function will free the generic structure            |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGeneric set unlock and free the structure       |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
VOID __cdecl /*APIENTRY*/ EQFFREESTRUCT( VOID )
{
  if ( pstEQFGeneric )
  {

    #ifdef USE_GLOBALALLOC_FOR_STEQFGEN
      GlobalUnlock( sel );
      GlobalFree( sel );
      sel = 0;
    #else
      UnmapViewOfFile( pstEQFGeneric );
      CloseHandle( hMapObject );
      hMapObject = NULL;
    #endif
    pstEQFGeneric = NULL;
  } /* endif */

  return;
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFCHECKSTRUCT                                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFCHECKSTRUCT( )                                        |
//+----------------------------------------------------------------------------+
//|Description:       This function will check, if a generic structure is      |
//|                   allocated.                                               |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSTEQFGEN                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     return pstEQFGeneric ...                                 |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
PSTEQFGEN __cdecl /*APIENTRY*/ EQFCHECKSTRUCT ( VOID )

{
  return ( pstEQFGeneric );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFADDABBREV                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFADDABBREV( PSZ );                                     |
//+----------------------------------------------------------------------------+
//|Description:       This function adds the passed word to the abbrev. dict.  |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ         word to be added to the abbreviation dict.   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       OK        everything okay                                |
//|                   EQFRS_NOADDENDA_DICT    no addenda dictionary available. |
//+----------------------------------------------------------------------------+
//|Function flow:     call EQFXADDABBREV with the appropriate parameters       |
//|                   return the error code from the function call             |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFADDABBREV
(
   PSZ pszData
)
{
   return ( EQFXADDABBREV(pstEQFGeneric, pszData));
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFXADDABBREV                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFXADDABBREV( PSTEQFGEN, PSZ );                         |
//+----------------------------------------------------------------------------+
//|Description:       This function adds the passed word to the addenda        |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN   generic structure                            |
//|                   PSZ         word to be added to the addenda              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       OK        everything okay                                |
//|                   EQFRS_NOADDENDA_DICT    no addenda dictionary available. |
//+----------------------------------------------------------------------------+
//|Function flow:     if pstEQFGen is set then                                 |
//|                     prepare values for function call                       |
//|                     send message to document handler; set return code      |
//|                   else                                                     |
//|                     set return code to EQFRS_NO_INIT;                      |
//|                   endif                                                    |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT EQFXADDABBREV
(
  PSTEQFGEN pstEQFGen,                         // pointer to generic struct.
  PSZ  pszData                                 // pointer to string
)
{
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_ADDABBREV;
     pstEQFPCmd->ulParm1 = 0;
     pstEQFPCmd->usLen1  = (USHORT)(strlen (pszData) + 1);
     pstEQFPCmd->usLen2  = 0;
     memcpy (pstEQFPCmd->ucbBuffer, pszData, pstEQFPCmd->usLen1 );


     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_ADDABBREV, NULL, NULL);
     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFADDABBREV'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFADDABBREVW
(
  PSZ_W  pszData                                // pointer to string
)
{
  PSTEQFGEN     pstEQFGen = pstEQFGeneric;      // pointer to generic struct.
  PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
  USHORT        usRc;                           // return code

  if ( pstEQFGen )
  {

     pstEQFPCmd = pstEQFGen->pstEQFPCmd;

     pstEQFPCmd->usCmd   = EQFCMD_ADDABBREVW;
     pstEQFPCmd->ulParm1 = 0;
     pstEQFPCmd->usLen1  = (USHORT)(UTF16strlenCHAR (pszData) + 1);
     pstEQFPCmd->usLen2  = 0;
     memcpy (pstEQFPCmd->ucbBuffer, (PVOID)pszData, pstEQFPCmd->usLen1 * sizeof(CHAR_W));


     pstEQFGen->usRC = 0;
     WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_ADDABBREV, NULL, NULL);
     usRc = pstEQFGen->usRC;
  }
  else
  {
     usRc = EQFRS_NO_INIT;                      // no generic structure yet
  } /* endif */
  return usRc;

} // end 'EQFADDABBREV'


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETNEXTSEG                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETNEXTSEG(LONG, PUSHORT, PCHAR, PUSHORT)             |
//+----------------------------------------------------------------------------+
//|Description:       Fill the allocated buffer with datastring of segment next|
//|                   to given segment number                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        LONG      lInfo,                                         |
//|                   PUSHORT   pusSegNum,                                     |
//|                   PCHAR     pchData,                                       |
//|                   PUSHORT   pusBufSize                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   EQFRS_ENTRY_NOT_AVAIL  out of range                      |
//+----------------------------------------------------------------------------+
//|Function flow:     get pointer to document struct                           |
//|                   if not at end of document                                |
//|                     get next segment                                       |
//|                   else                                                     |
//|                     return out of range ( EQFRS_ENTRY_NOT_AVAIL)           |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETNEXTSEG
(
  LONG      lInfo,
  PUSHORT   pusSegNum,
  PCHAR     pchData,
  PUSHORT   pusBufSize
)
{
  USHORT        usRc;                                      // return code
  USHORT        usStatus;                                  // segment status
  PTBDOCUMENT   pDoc;
  ULONG         ulSegNum = (ULONG) *pusSegNum;

  pDoc = (PTBDOCUMENT ) lInfo;                      // is pDOc ok?
  if (ulSegNum < (pDoc->ulMaxSeg-1) )
  {
  CHAR_W    chDataW[MAX_SEGMENT_SIZE];

  if (*pusBufSize > MAX_SEGMENT_SIZE)
  {
    *pusBufSize = MAX_SEGMENT_SIZE;
  }


    ulSegNum++;
    usRc = EQFBGetReqSeg( pDoc,
                          1, &ulSegNum,
                          &chDataW[0], pusBufSize, &usStatus );
    Unicode2ASCII( &chDataW[0], pchData, pDoc->ulOemCodePage );
    *pusSegNum = (USHORT) ulSegNum;
  }
  else
  {
    usRc = EQFRS_ENTRY_NOT_AVAIL;      // out of range
  } /* endif */

  return usRc;

} // end 'EQFGETNEXTSEG'


__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETNEXTSEGW
(
  LONG      lInfo,
  PULONG    pulSegNum,
  PCHAR_W   pchData,
  PUSHORT   pusBufSize
)
{
  USHORT        usRc;                                      // return code
  USHORT        usStatus;                                  // segment status
  PTBDOCUMENT   pDoc;
  ULONG         ulSegNum = (ULONG) *pulSegNum;

  pDoc = (PTBDOCUMENT ) lInfo;                      // is pDOc ok?
  if (ulSegNum < (pDoc->ulMaxSeg-1) )
  {
    ulSegNum++;
    usRc = EQFBGetReqSeg( pDoc,
                          1, &ulSegNum,
                          pchData, pusBufSize, &usStatus );
    *pulSegNum = ulSegNum;
  }
  else
  {
    usRc = EQFRS_ENTRY_NOT_AVAIL;      // out of range
  } /* endif */

  return usRc;

} // end 'EQFGETNEXTSEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETNEXTSEGS
(
  LONG      lInfo,
  PUSHORT   pusSegNum,
  PCHAR     pchData,
  PUSHORT   pusBufSize,
  PUSHORT   pusSegStatus
)
{
  USHORT        usRc;                                      // return code
  USHORT        usStatus;                                  // segment status
  PTBDOCUMENT   pDoc;
  ULONG         ulSegNum = (ULONG) *pusSegNum;

  pDoc = (PTBDOCUMENT ) lInfo;                      // is pDOc ok?
  if (ulSegNum < (pDoc->ulMaxSeg-1) )
  {
  CHAR_W    chDataW[MAX_SEGMENT_SIZE];

  if (*pusBufSize > MAX_SEGMENT_SIZE)
  {
    *pusBufSize = MAX_SEGMENT_SIZE;
  }


    ulSegNum++;
    usRc = EQFBGetReqSeg( pDoc,
                          1, &ulSegNum,
                          &chDataW[0], pusBufSize, &usStatus );
    Unicode2ASCII( &chDataW[0], pchData, pDoc->ulOemCodePage );
    *pusSegNum = (USHORT) ulSegNum;
    *pusSegStatus = usStatus;
  }
  else
  {
    usRc = EQFRS_ENTRY_NOT_AVAIL;      // out of range
  } /* endif */

  return usRc;

} // end 'EQFGETNEXTSEGS'


__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETNEXTSEGSW
(
  LONG      lInfo,
  PULONG    pulSegNum,
  PCHAR_W   pchData,
  PUSHORT   pusBufSize,
  PUSHORT   pusSegStatus
)
{
  USHORT        usRc;                                      // return code
  USHORT        usStatus;                                  // segment status
  PTBDOCUMENT   pDoc;
  ULONG         ulSegNum = (ULONG) *pulSegNum;

  pDoc = (PTBDOCUMENT ) lInfo;                      // is pDOc ok?
  if (ulSegNum < (pDoc->ulMaxSeg-1) )
  {
    ulSegNum++;
    usRc = EQFBGetReqSeg( pDoc,
                          1, &ulSegNum,
                          pchData, pusBufSize, &usStatus );
    *pulSegNum = ulSegNum;
    *pusSegStatus = usStatus;
  }
  else
  {
    usRc = EQFRS_ENTRY_NOT_AVAIL;      // out of range
  } /* endif */

  return usRc;

} // end 'EQFGETNEXTSEGSW'


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETPREVSEG                                            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETPREVSEG(LONG, PUSHORT, PCHAR, PUSHORT)             |
//+----------------------------------------------------------------------------+
//|Description:       Fill the allocated buffer with datastring of segment     |
//|                   previous to given segment number                         |
//+----------------------------------------------------------------------------+
//|Parameters:        LONG      lInfo,                                         |
//|                   PUSHORT   pusSegNum,                                     |
//|                   PCHAR     pchData,                                       |
//|                   PUSHORT   pusBufSize                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   EQFRS_ENTRY_NOT_AVAIL    out of range                    |
//+----------------------------------------------------------------------------+
//|Function flow:     if not already at start of file                          |
//|                     get previous segment                                   |
//|                   else                                                     |
//|                     return EQFRS_ENTRY_NOT_AVAIL ( out of range)           |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETPREVSEG
(
  LONG      lInfo,
  PUSHORT   pusSegNum,
  PCHAR     pchData,
  PUSHORT   pusBufSize
)
{
  USHORT    usRc = EQFRC_OK;                            // return code
  USHORT    usStatus;                                   // segment status
  ULONG     ulSegNum = (ULONG) *pusSegNum;
  PTBDOCUMENT pDoc = (PTBDOCUMENT) lInfo;

  if ( ulSegNum > 1 )
  {
    CHAR_W    chDataW[MAX_SEGMENT_SIZE];

    if (*pusBufSize > MAX_SEGMENT_SIZE)
    {
      *pusBufSize = MAX_SEGMENT_SIZE;
    }

    ulSegNum --;
    usRc = EQFBGetReqSeg( (PTBDOCUMENT) lInfo,
                          -1, &ulSegNum,
                          &chDataW[0], pusBufSize, &usStatus);
    Unicode2ASCII( &chDataW[0], pchData, pDoc->ulOemCodePage );
    *pusSegNum = (USHORT) ulSegNum;
  }
  else
  {
    usRc = EQFRS_ENTRY_NOT_AVAIL;      // out of range
  } /* endif */


  return usRc;

} // end 'EQFGETPREVSEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETPREVSEGW
(
  LONG      lInfo,
  PULONG    pulSegNum,
  PCHAR_W   pchData,
  PUSHORT   pusBufSize
)
{
  USHORT    usRc = EQFRC_OK;                            // return code
  USHORT    usStatus;                                   // segment status
  ULONG     ulSegNum = *pulSegNum;

  if ( ulSegNum > 1 )
  {
    ulSegNum --;
    usRc = EQFBGetReqSeg( (PTBDOCUMENT) lInfo,
                         -1, &ulSegNum,
                        pchData, pusBufSize, &usStatus);
    *pulSegNum =  ulSegNum;
  }
  else
  {
    usRc = EQFRS_ENTRY_NOT_AVAIL;      // out of range
  } /* endif */


  return usRc;

} // end 'EQFGETPREVSEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETPREVSEGS
(
  LONG      lInfo,
  PUSHORT   pusSegNum,
  PCHAR     pchData,
  PUSHORT   pusBufSize,
  PUSHORT   pusSegStatus
)
{
  USHORT    usRc = EQFRC_OK;                            // return code
  USHORT    usStatus;                                   // segment status
  ULONG     ulSegNum = (ULONG) *pusSegNum;
  PTBDOCUMENT pDoc = (PTBDOCUMENT) lInfo;

  if ( ulSegNum > 1 )
  {
    CHAR_W    chDataW[MAX_SEGMENT_SIZE];

    if (*pusBufSize > MAX_SEGMENT_SIZE)
    {
      *pusBufSize = MAX_SEGMENT_SIZE;
    }

    ulSegNum --;
    usRc = EQFBGetReqSeg( (PTBDOCUMENT) lInfo,
                          -1, &ulSegNum,
                          &chDataW[0], pusBufSize, &usStatus);
    Unicode2ASCII( &chDataW[0], pchData, pDoc->ulOemCodePage );
    *pusSegNum = (USHORT) ulSegNum;
    *pusSegStatus = usStatus;
  }
  else
  {
    usRc = EQFRS_ENTRY_NOT_AVAIL;      // out of range
  } /* endif */


  return usRc;

} // end 'EQFGETPREVSEGS'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETPREVSEGSW
(
  LONG      lInfo,
  PULONG    pulSegNum,
  PCHAR_W   pchData,
  PUSHORT   pusBufSize,
  PUSHORT   pusSegStatus
)
{
  USHORT    usRc = EQFRC_OK;                            // return code
  USHORT    usStatus;                                   // segment status
  ULONG     ulSegNum = *pulSegNum;

  if ( ulSegNum > 1 )
  {
    ulSegNum --;
    usRc = EQFBGetReqSeg( (PTBDOCUMENT) lInfo,
                         -1, &ulSegNum,
                        pchData, pusBufSize, &usStatus);
    *pulSegNum =  ulSegNum;
    *pusSegStatus = usStatus;
  }
  else
  {
    usRc = EQFRS_ENTRY_NOT_AVAIL;      // out of range
  } /* endif */


  return usRc;

} // end 'EQFGETPREVSEGSW'



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETCURSEG                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETCURSEG(LONG, PUSHORT, PCHAR, PUSHORT)              |
//+----------------------------------------------------------------------------+
//|Description:       Fill the allocated buffer with datastring of segment     |
//|                   with the given segment number                            |
//+----------------------------------------------------------------------------+
//|Parameters:        LONG      lInfo,                                         |
//|                   PUSHORT   pusSegNum,                                     |
//|                   PCHAR     pchData,                                       |
//|                   PUSHORT   pusBufSize                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//+----------------------------------------------------------------------------+
//|Function flow:     get required segment                                     |
//|                                                                            |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETCURSEG
(
  LONG      lInfo,
  PUSHORT   pusSegNum,
  PCHAR     pchData,
  PUSHORT   pusBufSize
)
{
  USHORT        usRc = EQFRC_OK;                            // return code
  USHORT        usStatus;                                   // segment status
  PTBDOCUMENT   pDoc;
  ULONG         ulSegNum = (ULONG) *pusSegNum;
  CHAR_W        chDataW[MAX_SEGMENT_SIZE];

  pDoc = (PTBDOCUMENT) lInfo;
  if (*pusBufSize > MAX_SEGMENT_SIZE)
  {
    *pusBufSize = MAX_SEGMENT_SIZE;
  }
  /********************************************************************/
  /* check if pDoc is valid....                                       */
  /********************************************************************/

  usRc = EQFBGetReqSeg(pDoc, 0,
                      &ulSegNum,
                      &chDataW[0], pusBufSize, &usStatus);
  // prepare data in ASCII
  Unicode2ASCII( &chDataW[0], pchData, pDoc->ulOemCodePage );
  *pusSegNum = (USHORT) ulSegNum;
  return usRc;

}                                      // end 'EQFGETCURSEG'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETCURSEGW
(
  LONG      lInfo,
  PULONG    pulSegNum,
  PCHAR_W   pchData,
  PUSHORT   pusBufSize
)
{
  USHORT        usRc = EQFRC_OK;                            // return code
  USHORT        usStatus;                                   // segment status
  PTBDOCUMENT   pDoc;
  ULONG         ulSegNum = *pulSegNum;

  pDoc = (PTBDOCUMENT) lInfo;
  /********************************************************************/
  /* check if pDoc is valid....                                       */
  /********************************************************************/

  usRc = EQFBGetReqSeg(pDoc, 0,
                      &ulSegNum,
                      pchData, pusBufSize, &usStatus);
  *pulSegNum = ulSegNum;
  return usRc;

}                                      // end 'EQFGETCURSEGW'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETCURSEGS
(
  LONG      lInfo,
  PUSHORT   pusSegNum,
  PCHAR     pchData,
  PUSHORT   pusBufSize,
  PUSHORT   pusSegStatus
)
{
  USHORT        usRc = EQFRC_OK;                            // return code
  USHORT        usStatus;                                   // segment status
  PTBDOCUMENT   pDoc;
  ULONG         ulSegNum = (ULONG) *pusSegNum;
  CHAR_W        chDataW[MAX_SEGMENT_SIZE];

  pDoc = (PTBDOCUMENT) lInfo;
  if (*pusBufSize > MAX_SEGMENT_SIZE)
  {
    *pusBufSize = MAX_SEGMENT_SIZE;
  }
  /********************************************************************/
  /* check if pDoc is valid....                                       */
  /********************************************************************/

  usRc = EQFBGetReqSeg(pDoc, 0,
                      &ulSegNum,
                      &chDataW[0], pusBufSize, &usStatus);
  // prepare data in ASCII
  Unicode2ASCII( &chDataW[0], pchData, pDoc->ulOemCodePage );
  *pusSegNum = (USHORT) ulSegNum;
  *pusSegStatus = usStatus;
  return usRc;

}                                      // end 'EQFGETCURSEGS'

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETCURSEGSW
(
  LONG      lInfo,
  PULONG    pulSegNum,
  PCHAR_W   pchData,
  PUSHORT   pusBufSize,
  PUSHORT   pusSegStatus
)
{
  USHORT        usRc = EQFRC_OK;                            // return code
  USHORT        usStatus;                                   // segment status
  PTBDOCUMENT   pDoc;
  ULONG         ulSegNum = *pulSegNum;

  pDoc = (PTBDOCUMENT) lInfo;
  /********************************************************************/
  /* check if pDoc is valid....                                       */
  /********************************************************************/

  usRc = EQFBGetReqSeg(pDoc, 0,
                      &ulSegNum,
                      pchData, pusBufSize, &usStatus);
  *pulSegNum = ulSegNum;
  *pusSegStatus = usStatus;
  return usRc;

}                                      // end 'EQFGETCURSEGSW'



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFTEXTPANEL                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFTEXTPANEL(HWND, PSZ *,  USHORT, USHORT, PSZ, ULONG )  |
//+----------------------------------------------------------------------------+
//|Description:       Create test wnd and display data                         |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND      hwndParent,                                    |
//|                   PSZ *     ppData,                                        |
//|                   USHORT    usForeground,                                  |
//|                   USHORT    usBackground,                                  |
//|                   PSZ       pCaptionText                                   |
//|                   ULONG     flFrameFlags                                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR          everything okay                        |
//|                   ERROR_STORAGE     no storage avail.                      |
//|                   ERROR_DIALOG_LOAD_FAILED   dialog cannot be loaded       |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate struct. for display                             |
//|                   init foreground,background color                         |
//|                   init fields in TBDoc struct.                             |
//|                   display dialog box                                       |
//|                   free allocated struct.                                   |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFTEXTPANEL
(
    HWND      hwndParent,
    PSZ  *    ppData,
    USHORT    usForeground,
    USHORT    usBackground,
    PSZ       pCaptionText,
    ULONG     ulFrameFlags
)
{
  USHORT        usRc = NO_ERROR;                           // return code
  PSZ        *  ppXY;
  ULONG       ulLines = 0L;
  ULONG       ulTotalLen = 0L;
  PSZ_W       pDataW;
  PSZ_W      * ppXY_W;
  PSZ_W       pCurXY_W;
  ULONG       ulNewLen;
  PSZ_W       pPtrBuf;
  ULONG       ulOEMCP = 0L;

  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  /********************************************************************/
  /* create window/dialog                                             */
  /********************************************************************/
  if ( hResMod )
  {
    INT_PTR sRc = 0;
    PTEXTPANEL     pTextPanel;
    CHAR_W         chTextW[128];

    if ( UtlAlloc( (PVOID *) &pTextPanel, 0L, (LONG) sizeof( TEXTPANEL ),
                    ERROR_STORAGE ) )
    {
      PSZ p;
      CHAR c;
      pTextPanel->fUnicode     = FALSE;
      ulOEMCP = GetLangOEMCP(NULL);

      RegisterShowWnd();     // register our wnd class - don't care about return
      pTextPanel->hwndParent   = hwndParent;
      //pTextPanel->ppData       = (PSZ_W *)ppData;
      pTextPanel->usForeground = usForeground;
      pTextPanel->usBackground = usBackground;
      pTextPanel->usDefFG      = usForeground;
      pTextPanel->usDefBG      = usBackground;
      ASCII2Unicode( pCaptionText, chTextW, ulOEMCP );
      pTextPanel->pCaptionText = (PSZ_W) &chTextW[0];
      pTextPanel->TBDoc.pTokBuf= &(pTextPanel->TokBuf[0]);
      pTextPanel->TBDoc.pBlockMark = &(pTextPanel->BlockMark[0]);
      pTextPanel->TBDoc.pInBuf = &(pTextPanel->InBuf[0]);
      pTextPanel->TBDoc.docType = STARGET_DOC;
      pTextPanel->TBDoc.ulAnsiCodePage = GetLangAnsiCP(NULL);
      pTextPanel->TBDoc.ulOemCodePage = ulOEMCP;
      if ( ulFrameFlags & EQF_HORZSCROLL )
      {
        pTextPanel->flFrameStyle |= FCF_HORZSCROLL;
      } /* endif */
      if ( ulFrameFlags & EQF_VERTSCROLL )
      {
        pTextPanel->flFrameStyle |= FCF_VERTSCROLL;
      } /* endif */

      ppXY = ppData;
      while ( *ppXY )
      {
        ulTotalLen += strlen(*ppXY) + 1;
        ulLines ++;
        p = *ppXY;
        while ( (c = *p++) != NULC )
        {
            if ( c == '\n' )
            {
              ulLines ++;
              ulTotalLen++;
            }
        }
        ppXY++;
      } /* endwhile */
      ulLines ++;        // for NULL pointer at end of ptrlist

      if ( UtlAlloc( (PVOID *) &pDataW, 0L, (LONG) ulTotalLen * sizeof(CHAR_W),
                    ERROR_STORAGE ) )
      {
         if (UtlAlloc( (PVOID *) &pPtrBuf, 0L, (LONG) ulLines * sizeof(PSZ_W),
                    ERROR_STORAGE) )
         {
           PSZ q;

           ppXY_W = (PSZ_W * ) &pPtrBuf[0];
           pTextPanel->ppData = ppXY_W;
           pCurXY_W = pDataW;
           ppXY = ppData;
           while ( *ppXY )
           {
             q = p = *ppXY;
             while (*p)
             {
               while ( (c = *p++) != NULC )
               {
                 if (c == '\n')
                 {
                   *(p-1) = EOS;
                   ulNewLen = ASCII2UnicodeBuf( q, pCurXY_W, (strlen(q) + 1), ulOEMCP );
                   *ppXY_W++ = pCurXY_W;
                   pCurXY_W += ulNewLen;
                   *(p-1) = '\n';
                   q = p;
                 }
               }
               p--;  // one too far

               ulNewLen = ASCII2UnicodeBuf( q, pCurXY_W, (strlen(q) + 1), ulOEMCP );
               *ppXY_W++ = pCurXY_W;
               pCurXY_W += ulNewLen;

             }
             ppXY++;   // point to next string
           }
           sRc = DialogBoxParamW( hResMod, (PSZ_W)MAKEINTRESOURCE( ID_TB_SHOW_DLG ),
                                   hwndParent, EQFSHOWDLG, (LPARAM)pTextPanel );

         }
      }
      if ( sRc == DID_ERROR )
      {
        UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
        usRc = ERROR_DIALOG_LOAD_FAILED;
      } /* endif */
      UtlAlloc( (PVOID *) &pDataW, 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *) &pPtrBuf, 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *) &pTextPanel, 0L, 0L, NOMSG );

    }
    else
    {
      usRc = ERROR_STORAGE;
    } /* endif */
  } /* endif */

  return usRc;

} // end 'EQFTEXTPANEL'


__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFTEXTPANELW
(
    HWND      hwndParent,
    PSZ_W  *  ppData,
    USHORT    usForeground,
    USHORT    usBackground,
    PSZ_W     pCaptionText,
    ULONG     ulFrameFlags
)
{
  USHORT        usRc = NO_ERROR;                           // return code
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  /********************************************************************/
  /* create window/dialog                                             */
  /********************************************************************/
  if ( hResMod )
  {
    INT_PTR sRc;
    PTEXTPANEL     pTextPanel;

    if ( UtlAlloc( (PVOID *) &pTextPanel, 0L, (LONG) sizeof( TEXTPANEL ),
                    ERROR_STORAGE ) )
    {
      RegisterShowWnd();     // register our wnd class - don't care about return
      pTextPanel->fUnicode     = TRUE;
      pTextPanel->hwndParent   = hwndParent;
      pTextPanel->ppData       = ppData;
      pTextPanel->usForeground = usForeground;
      pTextPanel->usBackground = usBackground;
      pTextPanel->usDefFG      = usForeground;
      pTextPanel->usDefBG      = usBackground;
      pTextPanel->pCaptionText = pCaptionText;
      pTextPanel->TBDoc.pTokBuf= &(pTextPanel->TokBuf[0]);
      pTextPanel->TBDoc.pBlockMark = &(pTextPanel->BlockMark[0]);
      pTextPanel->TBDoc.pInBuf = &(pTextPanel->InBuf[0]);
      pTextPanel->TBDoc.docType = STARGET_DOC;
      if ( ulFrameFlags & EQF_HORZSCROLL )
      {
        pTextPanel->flFrameStyle |= FCF_HORZSCROLL;
      } /* endif */
      if ( ulFrameFlags & EQF_VERTSCROLL )
      {
        pTextPanel->flFrameStyle |= FCF_VERTSCROLL;
      } /* endif */

      sRc = DialogBoxParamW( hResMod, (PSZ_W)MAKEINTRESOURCE( ID_TB_SHOW_DLG ),
                               hwndParent, EQFSHOWDLG, (LPARAM)pTextPanel );

      if ( sRc == DID_ERROR )
      {
        UtlError( ERROR_DIALOG_LOAD_FAILED, MB_CANCEL, 0 , NULL, EQF_ERROR );
        usRc = ERROR_DIALOG_LOAD_FAILED;
      } /* endif */
      UtlAlloc( (PVOID *) &pTextPanel, 0L, 0L, NOMSG );

    }
    else
    {
      usRc = ERROR_STORAGE;
    } /* endif */
  } /* endif */

  return usRc;

} // end 'EQFTEXTPANEL'




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFSHOWDLG                                               |
//+----------------------------------------------------------------------------+
//|Description:       Displays the show Dlg                                    |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hDlg,          Dialog window handle                 |
//|                   USHORT msg,         Message ID                           |
//|                   WPARAM mp1,         Message parameter 1                  |
//|                   LPARAM mp2          Message parameter 2                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     switch ( dialog message)                                 |
//|                   WM_INITDLG:                                              |
//|                      calculate size and position of dlg                    |
//|                      load tagtable with our display tagging                |
//|                      post WM_EQF_INITIALIZE                                |
//|                   WM_EQF_INITIALIZE:                                       |
//|                      FillDisplay                                           |
//|                   WM_EQF_PROCESSTASK:                                      |
//|                      position dialog as requested                          |
//|                   WM_CLOSE:                                                |
//|                   WM_EQF_CLOSE:                                            |
//|                    free tagtable                                           |
//|                    dismiss dialog                                          |
//|                   WM_MOVE:                                                 |
//|                   WM_SIZE:                                                 |
//|                     query wnd position and set wnd position                |
//|                   WM_SYSCOMMAND:                                           |
//|                    case IDM_PRINT:                                         |
//|                      print document displayed in show dialog wnd           |
//|                   WM_COMMAND: case IDM_PRINT:                              |
//|                      print document displayed in show dialog wnd           |
//|                    case CANCEL:                                            |
//|                      close dialog                                          |
//+----------------------------------------------------------------------------+

INT_PTR CALLBACK EQFSHOWDLG
(
  HWND hDlg,                           // Dialog window handle
  WINMSG msg,                          // Message ID
  WPARAM mp1,                          // Message parameter 1
  LPARAM mp2                           // Message parameter 2
)
{
  MRESULT mResult = FALSE;
  switch ( msg )
  {
    case WM_INITDLG:
      {
        LONG lCX = WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);
        LONG lCY = WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN);
        PTEXTPANEL pTextPanel = (PTEXTPANEL) mp2;
        PTBDOCUMENT   pXTBDoc;
        PTBDOCUMENT   pDoc = &pTextPanel->TBDoc;
        PSTEQFGEN     pstEQFGen;
        PRECTL        prcl;             // pointer to Rectl struct
        ANCHORDLGIDA( hDlg, pTextPanel );
        pXTBDoc = ACCESSWNDIDA( pTextPanel->hwndParent, PTBDOCUMENT );
        pstEQFGen = (PSTEQFGEN) pXTBDoc->pstEQFGen;
        SetWindowTextW( hDlg, pTextPanel->pCaptionText );
        prcl = &pstEQFGen->rclShowWndPos;
        if ( PRECTL_XLEFT(prcl) >= lCX )
        {
          PRECTL_XLEFT(prcl) = 100;
        } /* endif */
        if ( PRECTL_XRIGHT(prcl) >= lCX )
        {
          PRECTL_XRIGHT(prcl) = lCX - 100;
          if ( PRECTL_XLEFT(prcl) >= PRECTL_XRIGHT(prcl) )
          {
            PRECTL_XLEFT(prcl) = 100;
          } /* endif */
        } /* endif */

        if ( PRECTL_YTOP(prcl) >= lCY )
        {
          PRECTL_YTOP(prcl) = 100;
        } /* endif */
        if ( PRECTL_YBOTTOM(prcl) >= lCY )
        {
          PRECTL_YBOTTOM(prcl) = lCY - 100;
          if ( PRECTL_YTOP(prcl) >= PRECTL_YBOTTOM(prcl) )
          {
            PRECTL_YTOP(prcl) = 100;
          } /* endif */
        } /* endif */
        if ( PRECTL_YBOTTOM(prcl) < PRECTL_YTOP(prcl) + 100)
        {
          PRECTL_YBOTTOM(prcl) = PRECTL_YTOP(prcl) + 100;
        } /* endif */

        EQFBGetUserSettings(pDoc);              // get user settings
        pDoc->bOperatingSystem = (BYTE) UtlGetOperatingSystemInfo();

        TALoadTagTable( QFSHOW_TABLE,
                        (PLOADEDTABLE *) &pDoc->pDocTagTable,
                        TRUE, TRUE );  // internal table...

        if ( pDoc->pDocTagTable )
        {
          EQFShowWndCreate( &pTextPanel->TBDoc, hDlg,
                           pTextPanel->flFrameStyle, STARGET_DOC );

          // position the dialog window
          WinSetWindowPos( hDlg, HWND_TOP,
                           (SHORT) PRECTL_XLEFT(prcl), (SHORT) PRECTL_YTOP(prcl),
                           (SHORT) (PRECTL_XRIGHT(prcl)-PRECTL_XLEFT(prcl)),
                           (SHORT)(PRECTL_YBOTTOM(prcl)- PRECTL_YTOP(prcl)),
                           EQF_SWP_MOVE | EQF_SWP_SIZE ); // | EQF_SWP_SHOW );

          WinPostMsg( hDlg, WM_EQF_INITIALIZE, NULL, mp2 );
        }
        else
        {
          WinPostMsg( hDlg, WM_COMMAND, MP1FROMSHORT(DID_CANCEL), NULL );
        } /* endif */
      }
      break;
    case WM_EQF_INITIALIZE:
      if (!FillDisplay( (PTEXTPANEL) mp2 ))
      {
        CHAR      chErrData[31];             // space for print button
        HAB       hab;                       // anchor block handle
        PSZ       pErrData = &chErrData[0];
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        hab = WinQueryAnchorBlock( hDlg );
        WinLoadString( hab, hResMod, IDS_TB_FUNC_SHOWTRANS, 30, chErrData );
        UtlError( ERROR_FILE_INVALID_DATA, MB_CANCEL, 1 ,
                   &pErrData, EQF_ERROR );
        WinPostMsg( hDlg, WM_COMMAND, MP1FROMSHORT(DID_CANCEL), NULL );
      } /* endif */
      break;
    case  WM_EQF_PROCESSTASK:
      if ( SHORTFROMMP1(mp1) == USER_TASK )
      {
         PTEXTPANEL    pTextPanel = ACCESSDLGIDA(hDlg, PTEXTPANEL);
         if (  pTextPanel )
         {
           SWP  swp, swp1;
           USHORT usX = (USHORT) WinQuerySysValue(HWND_DESKTOP,SV_CXSIZEBORDER);
           USHORT usY = (USHORT) WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER);
           USHORT usTitle = (USHORT) WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR );

           WinQueryWindowPos( hDlg, &swp);
           WinQueryWindowPos( pTextPanel->TBDoc.hwndFrame, &swp1 );
           if ( (swp1.cx != (swp.cx - (SHORT)(2*usX))) ||
                ( swp1.cy != swp.cy-(SHORT)(2*usY)-(SHORT)usTitle) )
           {
             WinSetWindowPos( pTextPanel->TBDoc.hwndFrame, HWND_TOP,
                              usX, usY,
                              (SHORT)(swp.cx - 2*usX),
                              (SHORT)(swp.cy - 2*usY - usTitle),
                              (ULONG)(EQF_SWP_SIZE | EQF_SWP_MOVE |
                              EQF_SWP_ACTIVATE | EQF_SWP_SHOW));
           } /* endif */
         } /* endif */
      } /* endif */
      break;
    case WM_EQF_CLOSE:
      {
        PTEXTPANEL    pTextPanel = ACCESSDLGIDA(hDlg, PTEXTPANEL);
        PTBDOCUMENT   pDoc = ACCESSWNDIDA( pTextPanel->hwndParent, PTBDOCUMENT );
        TAFreeTagTable( (PLOADEDTABLE)pDoc->pDocTagTable );
        DISMISSDLG( hDlg, SHORT1FROMMP2(mp2) );
      }
      break;

    case WM_MOVE:
    case WM_SIZE:
      {
        PRECTL        prcl;             // pointer to Rectl struct
        PTEXTPANEL    pTextPanel = ACCESSDLGIDA(hDlg, PTEXTPANEL);
        /**************************************************************/
        /* Under OS/2 we came here before we get the initialize       */
        /**************************************************************/
        if ( pTextPanel )
        {
          PTBDOCUMENT   pParentDoc = ACCESSWNDIDA( pTextPanel->hwndParent, PTBDOCUMENT );
          PSTEQFGEN     pstEQFGen;
          SWP           swp;              // size position array


          pstEQFGen = (PSTEQFGEN) pParentDoc->pstEQFGen;
          /***********************************************************/
          /* force repaint ....                                      */
          /***********************************************************/
          pTextPanel->TBDoc.Redraw |= REDRAW_ALL;
          INVALIDATERECT( pTextPanel->TBDoc.hwndClient, NULL, TRUE );

          if (hDlg && WinQueryWindowPos ( hDlg, &swp))
          {
            RECT rcl;

            prcl = &pstEQFGen->rclShowWndPos;
            PRECTL_XLEFT(prcl)      = (LONG)swp.x;
            PRECTL_XRIGHT(prcl)     = (LONG)(swp.x + swp.cx);
            PRECTL_YTOP(prcl)       = (LONG)swp.y;
            PRECTL_YBOTTOM(prcl)    = (LONG)(swp.y + swp.cy);

            GetClientRect( hDlg, &rcl);

            SetWindowPos( pTextPanel->TBDoc.hwndFrame, HWND_TOP,
                          0, 0,
                          rcl.right, rcl.bottom,
                          SWP_SHOWWINDOW | SWP_NOACTIVATE );
          } /* endif */

        } /* endif */
      }
      mResult = UTLDEFDIALOGPROC( hDlg, msg, mp1, mp2 );
      break;
    case WM_PAINT:
      {
        PTEXTPANEL    pTextPanel = ACCESSDLGIDA(hDlg, PTEXTPANEL);
        /**************************************************************/
        /* Under OS/2 we came here before we get the initialize       */
        /**************************************************************/
        if ( pTextPanel )
        {
          /***********************************************************/
          /* force repaint ....                                      */
          /***********************************************************/
          pTextPanel->TBDoc.Redraw |= REDRAW_ALL;
          INVALIDATERECT( pTextPanel->TBDoc.hwndClient, NULL, TRUE );
        } /* endif */
      }
      mResult = UTLDEFDIALOGPROC( hDlg, msg, mp1, mp2 );
      break;
    case WM_SYSCOMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case IDM_PRINT:
          {
            PTEXTPANEL    pTextPanel = ACCESSDLGIDA(hDlg, PTEXTPANEL);
            if (pTextPanel->pCaptionText)
            {
              PTBDOCUMENT pTBDoc = ACCESSWNDIDA( pTextPanel->hwndParent, PTBDOCUMENT );
              sprintf( pTextPanel->TBDoc.szDocName, "%s - %s",
                       pTBDoc->chTitle, pTextPanel->pCaptionText );
            } /* endif */
            EQFBDocPrint( &pTextPanel->TBDoc );  // Print current document
          }
          break;
        default:
          mResult = UTLDEFDIALOGPROC( hDlg, msg, mp1, mp2 );
          break;
      } /* endswitch */
      break;

    case WM_COMMAND:
      switch ( WMCOMMANDID( mp1, mp2 ) )
      {
        case DID_CANCEL:
          POSTEQFCLOSE( hDlg, FALSE );
          mResult = MRFROMSHORT(TRUE); // TRUE = command is processed
          break;
        case IDM_PRINT:
          {
            PTEXTPANEL    pTextPanel = ACCESSDLGIDA(hDlg, PTEXTPANEL);
            if (pTextPanel->pCaptionText)
            {
              PTBDOCUMENT pTBDoc = ACCESSWNDIDA( pTextPanel->hwndParent, PTBDOCUMENT );
              sprintf( pTextPanel->TBDoc.szDocName, "%s - %s",
                       pTBDoc->chTitle, pTextPanel->pCaptionText );
            } /* endif */
            EQFBDocPrint( &pTextPanel->TBDoc );  // Print current document
          }
          break;
        default:
          mResult = UTLDEFDIALOGPROC( hDlg, msg, mp1, mp2 );
          break;
      } /* endswitch */
      break;
    default:
      mResult = UTLDEFDIALOGPROC( hDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */
  return ( mResult );

} /* end of function EQFSHOWDLG */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFShowWndCreate                                         |
//+----------------------------------------------------------------------------+
//|Description:       Create our special EQFB-like window for display          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pNewdoc,                                     |
//|                   HWND        hwndParent,                                  |
//|                   ULONG       flFrameStyle,                                |
//|                   USHORT      usDocType                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   -- window was created                             |
//|                   FALSE  -- window creation failed                         |
//+----------------------------------------------------------------------------+
//|Function flow:     get anchor block                                         |
//|                   create window of new class EQFSHOW_CLASS                 |
//|                   add print item to systemmenue                            |
//+----------------------------------------------------------------------------+
BOOL EQFShowWndCreate
(
  PTBDOCUMENT pNewdoc,                 // pointer to document structure
  HWND        hwndParent,
  ULONG       flFrameStyle,
  USHORT      usDocType
)
{

  BOOL  fOK = TRUE;                    // success indicator
  CHAR  chPrint[31];                   // space for print button

  HAB       hab;                       // anchor block handle
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
  hab = WinQueryAnchorBlock( hwndParent );

  WinLoadString( hab, hResMod, IDS_TB_WYSIWYG_PRINT, 30, chPrint );

       pNewdoc->hwndClient = pNewdoc->hwndFrame =
         CreateWindow ( EQFSHOW_CLASS,
                        pNewdoc->chTitle,    //window caption
                        WS_CHILD |
                        WS_CLIPSIBLINGS |
                        flFrameStyle,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        hwndParent, //parent wnd handle
                        NULL,
                        (HINSTANCE) hab,               //program instance handle
                        NULL );

       /***************************************************************/
       /* add our print menu item                                     */
       /***************************************************************/
       if ( pNewdoc->hwndFrame )
       {
         HMENU hMenu = GetSystemMenu( hwndParent, FALSE );
         InsertMenu( hMenu, SC_CLOSE, MF_ENABLED | MF_BYCOMMAND,
                     IDM_PRINT, chPrint );
         InsertMenu( hMenu, SC_CLOSE, MF_SEPARATOR | MF_BYCOMMAND,
                     0, NULL );
       } /* endif */
     if ( pNewdoc->hwndFrame )
     {
        ANCHORWNDIDA( pNewdoc->hwndFrame, pNewdoc );
        ANCHORWNDIDA( pNewdoc->hwndClient, pNewdoc );
       {
         PVIOFONTCELLSIZE pVioFont;               // pointer to font
         pVioFont = get_vioFontSize();

         EQFBSetNewCellSize( pNewdoc, (pVioFont + usDocType)->cx,
                                      (pVioFont + usDocType)->cy );
       }
     }
     else
     {
        // display message - now only a warning beep
        WinAlarm( HWND_DESKTOP, WA_ERROR );
        fOK = FALSE;
     } /* endif */
   return ( fOK );
} /* end of function EQFShowWndCreate */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     RegisterShowWnd                                          |
//+----------------------------------------------------------------------------+
//|Description:       register our displaywindow class                         |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     register our EQFSHOW class                               |
//+----------------------------------------------------------------------------+
static VOID RegisterShowWnd()
{
//register our display window class
    WNDCLASSW  wndclassW;
   HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

    wndclassW.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wndclassW.lpfnWndProc   = EQFSHOWWP;
    wndclassW.cbClsExtra    = 0;
    wndclassW.cbWndExtra    = sizeof(PVOID);
    wndclassW.hInstance     = (HINSTANCE)(HAB)UtlQueryULong( QL_HAB );
    wndclassW.hIcon         = LoadIcon( hResMod, MAKEINTRESOURCE( DOC_ICON ) );
    wndclassW.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wndclassW.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
    wndclassW.lpszMenuName  = NULL;
    wndclassW.lpszClassName = EQFSHOW_CLASS_W;

    RegisterClassW( &wndclassW );

} /* end of function RegisterShowWnd */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFSHOWWP                                                |
//+----------------------------------------------------------------------------+
//|Description:       Our instance data window procedure                       |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2            |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     window procedure for document wnd in dialog              |
//|                   handle cases :                                           |
//|                   WM_PAINT:                                                |
//|                   WM_SIZE:                                                 |
//|                   WM_KEYDOWN/WM_SYSCHAR/WM_SYSDEADCHAR/WM_DEADCHAR/WM_CHAR |
//|                   or WM_CHAR in OS/2: handle key input                     |
//|                   WM_HSCROLL:                                              |
//|                   WM_VSCROLL:                                              |
//|                   in Windows: WM_KILLFOCUS                                 |
//|                   WM_EQF_SETFOCUS:                                         |
//|                   WM_SETFOCUS:                                             |
//|                   WM_ACTIVATE:                                             |
//+----------------------------------------------------------------------------+
MRESULT APIENTRY EQFSHOWWP
( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
  MRESULT       mResult = FALSE;       //result value of window proc
  PTBDOCUMENT   pTBDoc  = ACCESSWNDIDA( hwnd, PTBDOCUMENT );
  USHORT usSBType;
  USHORT usThumbPosition;

  switch( msg )
  {
    case WM_GETDLGCODE:
      mResult = DLGC_WANTMESSAGE;
      break;

    case WM_PAINT:
      {
         HDC      hdc;
         PAINTSTRUCT ps;                                     // pointer to paint struct
         hdc = BeginPaint(hwnd, &ps );
         pTBDoc->Redraw |= REDRAW_ALL; // enforce repaint....
         EQFBRefreshScreen( pTBDoc );  // refresh the screen
         EndPaint(hwnd, &ps);
         mResult = (LRESULT) FALSE;
      } /* endif */
      break;



    case WM_SIZE:
      if ( pTBDoc )
      {
        pTBDoc->xClient = SHORT1FROMMP2(mp2) ;
        pTBDoc->yClient = SHORT2FROMMP2(mp2) ;
                 // set row and columns of currently available size
        pTBDoc->lScrnCols = pTBDoc->xClient / pTBDoc->cx ;
        pTBDoc->lScrnRows = pTBDoc->yClient / pTBDoc->cy ;
        EQFBVioSetNewDocSize( pTBDoc );
        /***********************************************************/
        /* force repaint ....                                      */
        /***********************************************************/
        EQFDispWindow( pTBDoc );
      } /* endif */
      break;

    case WM_KEYDOWN:
    case WM_SYSCHAR:
    case WM_SYSDEADCHAR:
    case WM_DEADCHAR:
    case WM_CHAR:
        mResult = HandleWMChar( hwnd, msg, mp1, mp2 );
        break;

    case WM_HSCROLL:
        usSBType        = (USHORT)LOWORD(mp1);
        usThumbPosition = (USHORT)HIWORD(mp1);
        EQFBHScroll ( pTBDoc, usSBType, usThumbPosition );
        break;

    case WM_VSCROLL:
        usSBType        = (USHORT)LOWORD(mp1);
        usThumbPosition = (USHORT)HIWORD(mp1);
        EQFBVScroll ( pTBDoc, usSBType, usThumbPosition );
        break;

    case WM_BUTTON1DOWN:    // Position cursor to pointer
        EQFBMousePosition( pTBDoc, msg, mp1, mp2 );
        break;

    /**************************************************************/
    /* special handling of CARET for windows                      */
    /**************************************************************/
    case WM_KILLFOCUS:
      WinSendMsg( hwnd, WM_NCACTIVATE, FALSE, NULL );
      HideCaret( hwnd );
      DestroyCaret();
      break;

    case WM_EQF_SETFOCUS:
    case WM_SETFOCUS:
      BringWindowToTop( hwnd );
      WinSendMsg( hwnd, WM_NCACTIVATE, TRUE, NULL );
      CreateCaret(hwnd, (HBITMAP)NULL,
                  pTBDoc->vioCurShapes[pTBDoc->usCursorType].cx,
                  pTBDoc->vioCurShapes[pTBDoc->usCursorType].cEnd);
      SetCaretPos( pTBDoc->lCursorCol * pTBDoc->cx,
                   pTBDoc->lCursorRow * pTBDoc->cy);
      ShowCaret( hwnd );
      break;

   case WM_ACTIVATE:
      WinSetFocus( HWND_DESKTOP, hwnd );
      break;

    default:
      /****************************************************************/
      /* our generic display procedure cannot be used, because this   */
      /* window is not a MDI window...                                */
      /****************************************************************/
      mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
      break;
  }/*end switch*/
  return( mResult );
} /* end of function EQFSHOWWP */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FillDisplay                                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare the colors for the passed input characters via   |
//|                   tokenizing regarding the color taggings.                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PTEXTPANEL    -- our textpanel structure                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     determine base ids for <COLOR and </COLOR                |
//|                   loop through prepared string buffer and prepare screen   |
//|                   display window                                           |
//+----------------------------------------------------------------------------+

static BOOL  FillDisplay
(
  PTEXTPANEL pTextPanel
)
{
  PSZ_W    *  ppXY      = pTextPanel->ppData;
  PTBDOCUMENT pTBDoc   = &pTextPanel->TBDoc;
  ULONG       ulSegNum = 0;
  PCHAR_W     pRest = NULL;            // ptr to start of not-processed bytes
  USHORT      usColPos = 0;            // column pos used by EQFTagTokenize
  PTOKENENTRY pTok;                    // ptr for token table processing
  BOOL        fOK = TRUE;

  /********************************************************************/
  /* run TATagTokenize to find our supported tokens...                */
  /********************************************************************/
  TATagTokenizeW( L"<COLOR=1,1></COLOR>",
                 (PLOADEDTABLE)pTextPanel->TBDoc.pDocTagTable,
                 TRUE,
                 &pRest,
                 &usColPos,
                 (PTOKENENTRY) pTextPanel->TokBuf,
                 TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

  pTok = (PTOKENENTRY) pTextPanel->TokBuf;
  sTagging[0] = pTok->sTokenid;
  sTagging[1] = (pTok+1)->sTokenid;

  EQFBAddSegW( pTBDoc, &tbInitSegment );

  while ( *ppXY && fOK)
  {
    /******************************************************/
    /* setup segment data                                 */
    /******************************************************/
    fOK = PrepareLine ( pTextPanel, *ppXY, &ulSegNum );

    tbNewLineSegment.ulSegNum = ulSegNum ++;
    EQFBAddSegW( pTBDoc, &tbNewLineSegment );
    pTBDoc->ulMaxLine ++;                        // increment line number

    ppXY++;
  } /* endwhile */

  /********************************************************************/
  /* fill window                                                      */
  /********************************************************************/
  if ( fOK )
  {
    EQFDispWindow( pTBDoc );
  } /* endif */

  return (fOK);
} /* end of function FillDisplay */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFDispWindow                                            |
//+----------------------------------------------------------------------------+
//|Description:       Display the prepared window and set the scrollbars       |
//|                   correctly                                                |
//+----------------------------------------------------------------------------+
//|Parameters:        PTBDOCUMENT pTBDoc   -- EQFB document structure          |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      PTBDOCUMENT structure is correctly filled                |
//+----------------------------------------------------------------------------+
//|Function flow:     build up screen cursor/row buffer and display the        |
//|                   screen.                                                  |
//|                   Adjust scrollbars                                        |
//+----------------------------------------------------------------------------+

static
VOID  EQFDispWindow
(
   PTBDOCUMENT  pTBDoc
)
{
  pTBDoc->lCursorRow = 0;
  pTBDoc->lCursorCol = 0;
  pTBDoc->lSideScroll = 0;

  pTBDoc->TBCursor.ulSegNum = 1;
  pTBDoc->TBCursor.usSegOffset = 0;
  EQFBScrnLinesFromSeg                   // build line table
         ( pTBDoc,
           pTBDoc->lCursorRow,           // starting row
           (pTBDoc->lScrnRows-pTBDoc->lCursorRow), // number of rows
           &(pTBDoc->TBCursor) );        // starting segment

  /**********************************************************************/
  /* no possibility found to set the thumbsize in windows, so only      */
  /* the setscrollbar is done in windows                                */
  /**********************************************************************/
     if ( GetWindowLong( pTBDoc->hwndFrame, GWL_STYLE ) & WS_VSCROLL )
     {
       SetScrollRange (pTBDoc->hwndFrame, SB_VERT,     //scroll slider
                       0,                              // min
                       max(pTBDoc->ulMaxSeg - 2, 2),   //   max
                       FALSE);
       SetScrollPos(pTBDoc->hwndFrame, SB_VERT,
                    pTBDoc->TBCursor.ulSegNum - 1, TRUE );    // Position
     } /* endif */

     if ( GetWindowLong( pTBDoc->hwndFrame, GWL_STYLE ) & WS_HSCROLL )
     {
       SetScrollRange (pTBDoc->hwndFrame, SB_HORZ, 0, 255, FALSE);

       if (pTBDoc->fTARight)
	   {
	   		if (pTBDoc-> lSideScroll < 255)
	   		{
	   	        SetScrollPos(pTBDoc->hwndFrame, SB_HORZ,255 - pTBDoc->lSideScroll, TRUE );
	   	    }
	   	    else
	   	    {
	   	    	SetScrollPos(pTBDoc->hwndFrame, SB_HORZ,0, TRUE );
	   	    }
	   }
	   else
	   {
	      SetScrollPos(pTBDoc->hwndFrame, SB_HORZ,pTBDoc->lSideScroll, TRUE );
       }
     } /* endif */


  pTBDoc->Redraw |= REDRAW_ALL;          // redraw the screen
  EQFBScreenData( pTBDoc );
  EQFBScreenCursor( pTBDoc );            // update cursor and sliders
} /* end of function EQFDispWindow */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HandleWMChar                                             |
//+----------------------------------------------------------------------------+
//|Description:       handle all keystrokes in our display window              |
//+----------------------------------------------------------------------------+
//|Parameters:        Window message parameters                                |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     get status flags (Ctrl, Shift) and do action accordingly |
//|                   handle the following cases:                              |
//|                   mark previous word                                       |
//|                   mark next word                                           |
//|                   goto top/bottom of doc                                   |
//|                   goto previous/next word                                  |
//|                   do not allow to type in characters                       |
//|                   unmark marked area                                       |
//|                   goto start /End of line                                  |
//|                   cursor left/right                                        |
//|                   page up/down                                             |
//+----------------------------------------------------------------------------+
MRESULT HandleWMChar
(
   HWND        hwnd,               // window handle
   WINMSG      msg,
   WPARAM      mp1,
   LPARAM      mp2
)
{
   PTBDOCUMENT   pTBDoc;
   BOOL          fShift;
   BOOL          fCtrl;
   UCHAR         ucCode;
   MRESULT       mResult = MRFROMSHORT(TRUE);// indicate message is processed

   pTBDoc = ACCESSWNDIDA( hwnd, PTBDOCUMENT );
   /*******************************************************************/
   /* determine control key states ....                               */
   /*******************************************************************/
     fCtrl   = GetKeyState(VK_CTRL ) >> 15 ;
     fShift  = GetKeyState(VK_SHIFT) >> 15 ;
     ucCode = (UCHAR) mp1;
   if ( fCtrl )  // ctrl key pressed
   {
     if ( fShift )
     {
       switch ( ucCode )
       {
         case  VK_LEFT:
           EQFBFuncMarkPrevWord( pTBDoc );
           break;
         case  VK_RIGHT:
           EQFBFuncMarkNextWord( pTBDoc );
           break;
         default :
           break;
       } /* endswitch */
     }
     else
     {
       switch ( ucCode )
       {
          case VK_HOME:
             EQFBFuncTopDoc( pTBDoc );
             break;
          case VK_END:
             EQFBFuncBottomDoc( pTBDoc );
             break;
          case VK_LEFT:
             EQFBFuncPrevWord( pTBDoc );
             break;
          case VK_RIGHT:
             EQFBFuncNextWord( pTBDoc );
             break;
          default:
             break;
       } /* endswitch */
     } /* endif */
   }
   else if ( fShift )  // shift key pressed
   {
      switch ( ucCode )
      {
         case VK_INSERT:
            WinAlarm( HWND_DESKTOP, WA_WARNING );
            break;
         case VK_TAB:     // shift tab == backtab...
            EQFBFuncBacktab( pTBDoc );
            break;
         case VK_SHIFT:                // do nothing
         case VK_ESC:                  // do nothing
         default:
            break;
      } /* endswitch */
   }
   else                                   // no control key
   {
      switch ( ucCode )
      {
         case VK_UP:
            EQFBFuncMarkClear( pTBDoc );
            EQFBFuncUp( pTBDoc );
            break;
         case VK_DOWN:
            EQFBFuncMarkClear( pTBDoc );
            EQFBFuncDown( pTBDoc );
            break;
         case VK_LEFT:
            EQFBFuncMarkClear( pTBDoc );
            EQFBFuncLeft( pTBDoc );
            break;
         case VK_RIGHT:
            EQFBFuncMarkClear( pTBDoc );
            EQFBFuncRight( pTBDoc );
            break;
         case VK_HOME:
            EQFBFuncStartLine( pTBDoc );
            break;
         case VK_END:
            EQFBFuncEndLine( pTBDoc );
            break;
         case VK_PAGEUP:
            EQFBFuncPageUp( pTBDoc );
            break;
         case VK_PAGEDOWN:
            EQFBFuncPageDown( pTBDoc );
            break;
         case VK_TAB:
            EQFBFuncTab( pTBDoc );
            break;
          case VK_ENTER:
            break;
         case VK_ESC:                 // close the dialog ....
            WinPostMsg( GETPARENT( hwnd ), WM_COMMAND,
                        MP1FROMSHORT(DID_CANCEL), NULL );
            break;

         default :
            break;
      } /* endswitch */
    } /* endif */

    /******************************************************************/
    /* pass on any default keystrokes to editor window....            */
    /******************************************************************/
    mResult = WinDefWindowProc ( hwnd, msg, mp1, mp2 );

    EQFBScreenData( pTBDoc );          // display screen
    EQFBScreenCursor( pTBDoc );        // update cursor and sliders
    return mResult;
} /* end of function HandleWMChar */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PrepareLine                                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare a line for display in our TranslationProcessor   |
//|                   syntax as segment                                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PTEXTPANEL    -- pointer to textpanel structure          |
//|                   PSZ           -- pointer to string to be displayed       |
//|                   PULONG        -- segment number in/out                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     use TATagTokenize to determine our special tokens        |
//|                   if  <COLOR is detected, use status QF_OWN_STATUS and     |
//|                   use high byte to specify color for encoding              |
//+----------------------------------------------------------------------------+

BOOL PrepareLine
(
  PTEXTPANEL pTextPanel,
  PSZ_W      pData,
  PULONG     pulSegNum
)
{
  USHORT usFG = pTextPanel->usForeground;
  USHORT usBG = pTextPanel->usBackground;
  ULONG  ulSegNum = *pulSegNum;
  TBSEGMENT   tbSegment;
  PCHAR_W     pRest = NULL;            // ptr to start of not-processed bytes
  USHORT      usColPos = 0;            // column pos used by EQFTagTokenize
  PTOKENENTRY pTok;                    // ptr for token table processing
  BOOL        fOK = TRUE;

  /********************************************************************/
  /* run TATagTokenize to find tokens ....                            */
  /********************************************************************/
  TATagTokenizeW( pData,
                 (PLOADEDTABLE)pTextPanel->TBDoc.pDocTagTable,
                 TRUE,
                 &pRest,
                 &usColPos,
                 (PTOKENENTRY) pTextPanel->TokBuf,
                 TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

  if ( pRest )
  {
    fOK = FALSE;
  }
  else
  {
    /********************************************************************/
    /* build tokenlist, i.e.                                            */
    /* convert tokens/text strings to entries in start-stop table       */
    /*                                                                  */
    /* Rational: use input buffer for temporary list ....               */
    /*           this is large enough, we can avoid any checking...     */
    /********************************************************************/
    pTok = (PTOKENENTRY) pTextPanel->TokBuf;

    while ( (pTok->sTokenid != ENDOFLIST) )
    {
      if ( pTok->sTokenid == TEXT_TOKEN )
      {
        memset( &tbSegment, 0, sizeof ( TBSEGMENT ));
        tbSegment.pDataW = pTok->pDataStringW;
        tbSegment.usLength = pTok->usLength;
        tbSegment.ulSegNum = ulSegNum ++;
        tbSegment.pusBPET = NULL;
        tbSegment.pusHLType = NULL;
        tbSegment.qStatus = QF_OWN_COLOR;
        tbSegment.qStatus |= ((usFG & 0x0f) << 8); //-- set fg color
        tbSegment.qStatus |= ((usBG & 0x0f ) << 12); //-- set bg color

        EQFBAddSegW( &pTextPanel->TBDoc, &tbSegment );

      }
      else
      {
        /****************************************************************/
        /* find out new color settings                                  */
        /****************************************************************/
        if ( sTagging[0] == pTok->sTokenid )
        {
          /**************************************************************/
          /* we are dealing with a <COLOR=x,y> tag                      */
          /**************************************************************/
          PSZ_W  pszTarget = pTok->pDataStringW;
          USHORT usNum;
          while ( *pszTarget && (*pszTarget != '=') )
          {
             pszTarget++;
          } /* endwhile */
          GETNUMBER( pszTarget, usNum );
          usFG = (usNum & 0x0F);

          GETNUMBER( pszTarget, usNum );
          usBG = (usNum & 0x0F);

        }
        else
        if ( sTagging[1] == pTok->sTokenid )
        {
          /**************************************************************/
          /* we are dealing with a </COLOR> tag                         */
          /**************************************************************/
          usFG = pTextPanel->usDefFG;
          usBG = pTextPanel->usDefBG;
        }
        else
//      if ( sTagging[2] == pTok->sTokenid )
//      {
//        /**************************************************************/
//        /* we are dealing with a newline  tag; if \n is in QFSHOW.ibl */
//        /**************************************************************/
//        tbNewLineSegment.usSegNum = usSegNum ++;
//        EQFBAddSegW( &pTextPanel->TBDoc, &tbNewLineSegment );
//        pTextPanel->TBDoc.usMaxLine ++;                        // increment line number
//      }
//      else
        {
          /**************************************************************/
          /* this kind of tagging not yet supported...                  */
          /**************************************************************/
        } /* endif */

      } /* endif */
      /****************************************************************/
      /* adjust current offset to point to new offset in string...    */
      /****************************************************************/
      pTok++;
    } /* endwhile */

    /********************************************************************/
    /* save current status to go ahead...                               */
    /********************************************************************/
    pTextPanel->usForeground = usFG;
    pTextPanel->usBackground = usBG;

    *pulSegNum = ulSegNum;
  } /* endif */
  return (fOK);
} /* end of function PrepareLine */



//+----------------------------------------------------------------------------+
//|External function    EQFCONVERTFILENAMES                                    |
//+----------------------------------------------------------------------------+
//|Function name:   EQFCONVERTFILENAMES                                        |
//+----------------------------------------------------------------------------+
//|Function call:  usRC = EQFCONVERTFILENAMES (pszFolder,pszLongFileName,      |
//|                                            pszShortFileName)               |
//+----------------------------------------------------------------------------+
//|Description:  converts LongFileName into ShortFileName and                  |
//|                        vice versa.                                         |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:  pszFolder        : Folder name with path information           |
//|                                                                            |
//|                                     FOLDER_DRIVE:\EQF\FOLDER_NAME.F00      |
//|                                                                            |
//|                                The Folder name can be extracted out        |
//|                                of pSegTarget, pSegSource as defined in     |
//|                                eqf_xstart.                                 |
//|                                                                            |
//|                                                                            |
//|             pszLongFileName  : LongFileName without path information       |
//|                                    O R                                     |
//|                                EMPTY_STRING                                |
//|                                                                            |
//|             pszShortFileName : ShortFileName without path information      |
//|                                    O R                                     |
//|                                EMPTY_STRING                                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:   see list in OTMAPI.H                                         |
//|               TM2 return code                                              |
//|               0=OK                                                         |
//|                                                                            |
//|               LongFileName evaluated out of ShortFileName  O R             |
//|               ShortFileName evaluated out of LongFileName                  |
//+----------------------------------------------------------------------------+
//|Function flow:     if (LongFileName==EMPTY_STRING)                          |
//|                                     evaluate LongFileName  out of          |
//|                                     ShortFileName                          |
//|                   else if (ShortFileName==EMPTY_STRING)                    |
//|                                     evaluate ShortFileName out of          |
//|                                     LongFileName                           |
//|                   else                                                     |
//|                                     error                                  |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFCONVERTFILENAMES
(
   PSZ      pszFolder,           //  Folder path
   PSZ      pszLongFileName,     //  LongFileName
   PSZ      pszShortFileName     //  ShortFileName
)
{

   USHORT   usRC=NO_ERROR;           // success indicator
   BOOL     fIsNew;                  // is shortversion new
   CHAR     szProperty[MAX_LONGPATH];// Property path


   if(pszLongFileName==NULL || pszShortFileName==NULL
                            || pszFolder==NULL)
   {
     usRC = ERROR_INVALID_DATA;
   }


   if (usRC==NO_ERROR)
   {

     /*******************************************/
     /*  Get ShortFileName out of LongFileName  */
     /*******************************************/
     if (!*pszShortFileName)
     {
         // is ShortFileName defined
         if(*pszLongFileName)
         {
             usRC=FolLongToShortDocName(pszFolder,
                                        pszLongFileName,
                                        pszShortFileName,
                                        &fIsNew);
             if(fIsNew)
             {
                 usRC = ERROR_FILE_NOT_FOUND;
             }/* end if */


         }
         else
         {
            usRC=ERROR_INVALID_DATA;
         }/* end if */

     }
     /*******************************************/
     /*  Get LongFileName out of ShortFileName  */
     /*******************************************/
     else if (!*pszLongFileName)
     {
        // is LongFileName defined
        if(*pszShortFileName && *pszFolder)
        {

           strcpy(szProperty,pszFolder);
           strcat(szProperty,BACKSLASH_STR);
           strcat(szProperty,pszShortFileName);

           usRC =  DocQueryInfo2(szProperty,
                                 NULL,NULL,NULL,NULL,
                                 pszLongFileName,
                                 NULL,NULL,FALSE);

          // if(pszLongFileName[0]==EOS)
          // {
          //    strcpy(pszLongFileName,pszShortFileName);
          // }

        }
        else
        {
           usRC=ERROR_INVALID_DATA;
        }/* end if */

     }
     else
     {
        usRC=ERROR_INVALID_DATA;
     } /* end if */

   } /* end if NO_ERROR */



   return usRC;

}/* end of function EQF_CONVERT_FILE_NAMES  */



//+----------------------------------------------------------------------------+
//|Function name: EQFGETSOURCELANG                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              | //|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: get source language of document                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETSOURCELANG
(
  PSZ      pszFolder,       //  Folder path
  PSZ      pszFileName,     //  FileName
  PSZ      pszSrcLang       //  Source Language out
)
{
   USHORT   usRC=NO_ERROR;           // success indicator
   CHAR     szProperty[MAX_LONGPATH];  // Property path


   // is FileName defined
   if(*pszFileName && *pszFolder)
   {

      strcpy(szProperty,pszFolder);
      strcat(szProperty,BACKSLASH_STR);
      strcat(szProperty,pszFileName);

      usRC =  DocQueryInfo2(szProperty,
                            NULL,NULL,pszSrcLang,NULL,
                            NULL,
                            NULL,NULL,FALSE);


   }
   else
   {
      usRC=ERROR_INVALID_DATA;
   }/* end if */

   return usRC;

} /* end of function EQFGETSOURCELANG */



//+----------------------------------------------------------------------------+
//|Function name: EQFGETTARGETLANG                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETTARGETLANG
(
  PSZ      pszFolder,       //  Folder path
  PSZ      pszFileName,     //  FileName
  PSZ      pszTrgLang       //  Target Language out
)
{
   USHORT   usRC=NO_ERROR;           // success indicator
   CHAR     szProperty[MAX_LONGPATH];  // Property path


   // is FileName defined
   if(*pszFileName && *pszFolder)
   {

      strcpy(szProperty,pszFolder);
      strcat(szProperty,BACKSLASH_STR);
      strcat(szProperty,pszFileName);

      usRC =  DocQueryInfo2(szProperty,
                            NULL,NULL,NULL,pszTrgLang,
                            NULL,
                            NULL,NULL,FALSE);


   }
   else
   {
      usRC=ERROR_INVALID_DATA;
   }/* end if */



   return usRC;

} /* end of function EQFGETTARGETLANG */


//+----------------------------------------------------------------------------+
//|Function name: EQFGETDOCFORMAT                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETDOCFORMAT
(
  PSZ      pszFolder,       //  Folder path
  PSZ      pszFileName,     //  FileName
  PSZ      pszFormat        //  Document Format out
)
{
   USHORT   usRC=NO_ERROR;           // success indicator
   CHAR     szProperty[MAX_LONGPATH];  // Property path


   // is FileName defined
   if(*pszFileName && *pszFolder)
   {

      strcpy(szProperty,pszFolder);
      strcat(szProperty,BACKSLASH_STR);
      strcat(szProperty,pszFileName);

      usRC =  DocQueryInfo2(szProperty,
                            NULL,pszFormat,NULL,NULL,
                            NULL,
                            NULL,NULL,FALSE);


   }
   else
   {
      usRC=ERROR_INVALID_DATA;
   }/* end if */


   return usRC;

} /* end of function EQFGETDOCFORMAT */



//+----------------------------------------------------------------------------+
//|Function name: EQFWORDCNTPERSEG                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFWORDCNTPERSEG
(
  PSZ            pszSeg,               // ptr to Segment
  PSZ            pszLang,              // ptr to Language of Segment
  PSZ            pszFormat,            // ptr to Format
  PULONG         pulResult,            // result to be counted
  PULONG         pulMarkUp              // result for markup
)
{
  USHORT         usRC=NO_ERROR;        // success indicator

  if(*pszSeg && *pszLang && *pszFormat && pulResult && pulMarkUp)
  {
    usRC = NO_ERROR;
  }
  else
  {
    usRC = ERROR_INVALID_DATA;
  }/* end if */

  if ( usRC == NO_ERROR )
  {
    PVOID       pVoidTable = NULL;       // ptr to loaded tag table
    PTOKENENTRY pTokBuf = NULL;
    SHORT       sLangID = -1;


    // allocate required buffers
    if ( usRC == NO_ERROR )
    {
      BOOL fOK = UtlAlloc((PVOID *)&pTokBuf, 0L, (LONG)TOK_BUFFER_SIZE, NOMSG );
      if (!fOK )
      {
        usRC = ERROR_INVALID_DATA;
      } /* endif */
    } /* endif */

    // Load Tag Table
    if ( usRC == NO_ERROR )
    {
      usRC =  TALoadTagTable( pszFormat,
                              (PLOADEDTABLE *) &pVoidTable,
                               FALSE, FALSE );
    } /* endif */

    // Get Language ID
    if ( usRC == NO_ERROR )
    {
      usRC = MorphGetLanguageID( pszLang, &sLangID );
    } /* end if */

    // do the actual counting
    if ( usRC == NO_ERROR )
    {
    PSZ_W  pszSegW;
    ULONG  ulOEMCP = 0L;
    UtlAlloc( (PVOID *)&pszSegW, 0L, MAX_SEGMENT_SIZE *sizeof(CHAR_W), NOMSG );

      ulOEMCP = GetLangOEMCP(pszLang);
      ASCII2Unicode( pszSeg, pszSegW , ulOEMCP);

      *pulResult = 0L;
      *pulMarkUp = 0L;

      usRC = EQFBWordCntPerSeg(
                       (PLOADEDTABLE)pVoidTable,
                       pTokBuf,
                       pszSegW,
                       sLangID,
                       pulResult, pulMarkUp, ulOEMCP);

      UtlAlloc( (PVOID *)&pszSegW, 0L, 0L, NOMSG );
    } /* endif */

    // cleanup
    if ( pTokBuf )   UtlAlloc((PVOID *)&pTokBuf, 0L, 0L, NOMSG );
    if ( pVoidTable) TAFreeTagTable( (PLOADEDTABLE)pVoidTable );
  } /* endif */

  return usRC;

} /* end of function EQFWORDCNTPERSEG */

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFWORDCNTPERSEGW
(
  PSZ_W          pszSeg,               // ptr to Segment
  PSZ            pszLang,              // ptr to Language of Segment
  PSZ            pszFormat,            // ptr to Format
  PULONG         pulResult,            // result to be counted
  PULONG         pulMarkUp              // result for markup
)
{
  USHORT         usRC=NO_ERROR;        // success indicator

  if(*pszSeg && *pszLang && *pszFormat && pulResult && pulMarkUp)
  {
    usRC = NO_ERROR;
  }
  else
  {
    usRC = ERROR_INVALID_DATA;
  }/* end if */

  if ( usRC == NO_ERROR )
  {
    PVOID       pVoidTable = NULL;       // ptr to loaded tag table
    PTOKENENTRY pTokBuf = NULL;
    SHORT       sLangID = -1;


    // allocate required buffers
    if ( usRC == NO_ERROR )
    {
      BOOL fOK = UtlAlloc((PVOID *)&pTokBuf, 0L, (LONG)TOK_BUFFER_SIZE, NOMSG );
      if (!fOK )
      {
        usRC = ERROR_INVALID_DATA;
      } /* endif */
    } /* endif */

    // Load Tag Table
    if ( usRC == NO_ERROR )
    {
      usRC =  TALoadTagTable( pszFormat,
                              (PLOADEDTABLE *) &pVoidTable,
                               FALSE, FALSE );
    } /* endif */

    // Get Language ID
    if ( usRC == NO_ERROR )
    {
      usRC = MorphGetLanguageID( pszLang, &sLangID );
    } /* end if */

    // do the actual counting
    if ( usRC == NO_ERROR )
    {
      ULONG  ulOEMCP = 0L;

      *pulResult = 0L;
      *pulMarkUp = 0L;

      ulOEMCP = GetLangOEMCP(pszLang);
      usRC = EQFBWordCntPerSeg(
                       (PLOADEDTABLE)pVoidTable,
                       pTokBuf,
                       pszSeg,
                       sLangID,
                       pulResult, pulMarkUp, ulOEMCP);
    } /* endif */

    // cleanup
    if ( pTokBuf )   UtlAlloc((PVOID *)&pTokBuf, 0L, 0L, NOMSG );
    if ( pVoidTable) TAFreeTagTable( (PLOADEDTABLE)pVoidTable );
  } /* endif */

  return usRC;

} /* end of function EQFWORDCNTPERSEG */





//+----------------------------------------------------------------------------+
//|Function name: CheckHistLogData                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


USHORT CheckHistLogData
(
   PAPICRITERIASUM  pInDataCnt,
   PCRITERIASUM     pOutDataCnt
)
{
   BOOL fOK = TRUE;

   pOutDataCnt->SimpleSum.usNumSegs   =  pInDataCnt->SimpleSum.usNumSegs  ;
   pOutDataCnt->SimpleSum.ulSrcWords  =  pInDataCnt->SimpleSum.ulSrcWords ;
   pOutDataCnt->SimpleSum.ulTgtWords  =  pInDataCnt->SimpleSum.ulTgtWords ;
   pOutDataCnt->SimpleSum.ulModWords  =  0 ;

   pOutDataCnt->MediumSum.usNumSegs   =  pInDataCnt->MediumSum.usNumSegs  ;
   pOutDataCnt->MediumSum.ulSrcWords  =  pInDataCnt->MediumSum.ulSrcWords ;
   pOutDataCnt->MediumSum.ulTgtWords  =  pInDataCnt->MediumSum.ulTgtWords ;
   pOutDataCnt->MediumSum.ulModWords  =  0 ;

   pOutDataCnt->ComplexSum.usNumSegs  =  pInDataCnt->ComplexSum.usNumSegs ;
   pOutDataCnt->ComplexSum.ulSrcWords =  pInDataCnt->ComplexSum.ulSrcWords;
   pOutDataCnt->ComplexSum.ulTgtWords =  pInDataCnt->ComplexSum.ulTgtWords;
   pOutDataCnt->ComplexSum.ulModWords =  0 ;

   return ((USHORT)fOK);

} /* End of CheckHistLogData */



//+----------------------------------------------------------------------------+
//|Function name: EQFWRITEHISTLOG                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: add hist log record to file                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+





__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFWRITEHISTLOG
(
   PSZ              pszFolObjName,    // folder object name (e.g. "E:\\EQF\\SAMPLE1.F00")
   PSZ              pszDocName,       // name of document (e.g. "DEVICE.SCR")
   PAPIDOCSAVEHIST  pHistLogApi       // HistLog Record structure
)
{


   USHORT        usRC=NO_ERROR;       // success indicator
   DOCSAVEHIST2  HistLog2;           // HistLog Record structure
   PDOCSAVEHIST2 pHistLog2;           // HistLog Record structure
   static CHAR szDocLongName[MAX_LONGFILESPEC];

   pHistLog2 = &(HistLog2);


   if(*pszFolObjName && *pszDocName && pHistLogApi)
   {
     usRC = NO_ERROR;
   }
   else
   {
     usRC = ERROR_INVALID_DATA;
   }/* end if */


   // check Folder Object Name
   // ------------------------

   if (usRC == NO_ERROR )
   {

      if ( FolQueryInfo2( pszFolObjName,
                           NULL,NULL,NULL,NULL,NULL,
                           FALSE ) != NO_ERROR )
      {
        usRC  = ERROR_XLATE_FOLDER_NOT_EXIST;
      } /* endif */

   } /* endif */

   // get document long name
   if (usRC == NO_ERROR )
   {
     OBJNAME szDocObjName;

     szDocLongName[0] = EOS;
     strcpy( szDocObjName, pszFolObjName );
     strcat( szDocObjName, BACKSLASH_STR );
     strcat( szDocObjName, pszDocName );
     DocQueryInfo2( szDocObjName, NULL, NULL, NULL, NULL, szDocLongName, NULL, NULL, FALSE );
   } /* endif */

   // convert Input Data
   // ------------------

   if (usRC == NO_ERROR )
   {

     memset(  pHistLog2,0, sizeof(DOCSAVEHIST2));

     CheckHistLogData (&pHistLogApi->EditAutoSubst , &pHistLog2->EditAutoSubst);
     CheckHistLogData (&pHistLogApi->ExactExist    , &pHistLog2->ExactExist   );
     CheckHistLogData (&pHistLogApi->ExactUsed     , &pHistLog2->ExactUsed    );
     CheckHistLogData (&pHistLogApi->FuzzyExist    , &pHistLog2->FuzzyExist   );
     CheckHistLogData (&pHistLogApi->FuzzyUsed     , &pHistLog2->FuzzyUsed    );
     CheckHistLogData (&pHistLogApi->FuzzyExist_1  , &pHistLog2->FuzzyExist_1 );
     CheckHistLogData (&pHistLogApi->FuzzyUsed_1   , &pHistLog2->FuzzyUsed_1  );
     CheckHistLogData (&pHistLogApi->FuzzyExist_2  , &pHistLog2->FuzzyExist_2 );
     CheckHistLogData (&pHistLogApi->FuzzyUsed_2   , &pHistLog2->FuzzyUsed_2  );
     CheckHistLogData (&pHistLogApi->FuzzyExist_3  , &pHistLog2->FuzzyExist_3 );
     CheckHistLogData (&pHistLogApi->FuzzyUsed_3   , &pHistLog2->FuzzyUsed_3  );
     CheckHistLogData (&pHistLogApi->MachExist     , &pHistLog2->MachExist    );
     CheckHistLogData (&pHistLogApi->MachUsed      , &pHistLog2->MachUsed     );
     CheckHistLogData (&pHistLogApi->NoneExist     , &pHistLog2->NoneExist    );
     CheckHistLogData (&pHistLogApi->NoneExist     , &pHistLog2->NoneExist2   );
     CheckHistLogData (&pHistLogApi->NotXlated     , &pHistLog2->NotXlated    );

     EQFBWriteHistLog2( pszFolObjName,
                        pszDocName,
                        DOCAPI_LOGTASK,
                        sizeof(DOCSAVEHIST2),
                        (pHistLog2),
                        FALSE, NULLHANDLE, szDocLongName );

   } /* endif */

   return(usRC);


} /* end of function EQFWRITEHISTLOG */




//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQF_XDOCGETPATHS                                         |
//+----------------------------------------------------------------------------+
//|Function call:     usRc = EQF_XDOCGETPATHS( pDocObjName, pSegSrc, pSegTgt); |
//+----------------------------------------------------------------------------+
//|Description:       generate the fully qualified segmented source and target |
//|                   file names for the specified document object name        |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ      pszDocObjName   - document object name - input  |
//|                   PSZ      pszSegSource    - segmented source name - output|
//|                   PSZ      pszSegTarget    - segmented target name - output|
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      -- success                                        |
//|                   TM/2 rc                                                  |
//+----------------------------------------------------------------------------+
//|Prerequesits:      doc. object name is a valid object name                  |
//|                   pszSegSource and pszSegTarget are allocated in large     |
//|                   enough size                                              |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate temporary variables                             |
//|                   call DocQueryInfo2 to get possible long file name        |
//|                   build segmented source and target according to provided  |
//|                     document name                                          |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQF_XDOCGETPATHS
(
   PSZ      pszDocObjName16,
   PSZ      pszSegSource16,
   PSZ      pszSegTarget16
)
{
   USHORT usRC = 0;
   PSZ    pszDocObject = NULL;
   /*******************************************************************/
   /* force conversion to 32 bit mode -- if this is not done, Visual  */
   /* Age has some trouble in type adjustement in opt. version        */
   /*******************************************************************/
   PSZ    pszDocObjName = pszDocObjName16;
   PSZ    pszSegSource  = pszSegSource16;
   PSZ    pszSegTarget  = pszSegTarget16;
   /*******************************************************************/
   /* get the appropriate file document file                          */
   /*******************************************************************/
   if (  pszDocObjName && *pszDocObjName && pszSegSource && pszSegTarget )
   {
     UtlAlloc( (PVOID *) &pszDocObject, 0L, (LONG) MAX_LONGPATH, NOMSG );
     if ( pszDocObject )
     {
       strcpy( pszDocObject, pszDocObjName );
     }
     else
     {
       usRC = ERROR_STORAGE;
     } /* endif */

     /*******************************************************************/
     /* create the segmented source and target filename and return it   */
     /*******************************************************************/
     if ( !usRC )
     {
       PSZ pFileName;
       PSZ pFolder;

       pFileName = UtlSplitFnameFromPath( pszDocObject );
       pFolder = UtlGetFnameFromPath( pszDocObject );


       /***************************************************************/
       /* check if filename filled                                    */
       /***************************************************************/
       UtlMakeEQFPath( pszSegSource, pszDocObject[0],
                       DIRSEGSOURCEDOC_PATH, pFolder );
       strcat( pszSegSource, BACKSLASH_STR );
       strcat( pszSegSource, pFileName );

       UtlMakeEQFPath( pszSegTarget, pszDocObject[0],
                       DIRSEGTARGETDOC_PATH, pFolder );
       strcat( pszSegTarget, BACKSLASH_STR );
       strcat( pszSegTarget, pFileName );

     } /* endif */
   }
   else
   {
     usRC=ERROR_INVALID_DATA;
   } /* endif */

   /*******************************************************************/
   /* free allocated resources                                        */
   /*******************************************************************/
   UtlAlloc( (PVOID *) &pszDocObject, 0L, 0L, NOMSG );

   return( usRC );
} /* end of function EQF_XDOCGETPATHS */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQF_XDOCACT                                              |
//+----------------------------------------------------------------------------+
//|Function call:     usRc = EQF_XDOCACT( pstEQFGen, pDocObjName );            |
//+----------------------------------------------------------------------------+
//|Description:       make the specified document the active Translation Env.  |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN pstEQFGen      - ptr to generic structure      |
//|                   PSZ       pszDocObjName  - document object name - input  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0        -- success                                      |
//|                   TM/2 rc                                                  |
//+----------------------------------------------------------------------------+
//|Prerequesits:      doc. object name is a valid object name                  |
//|                   pstEQFGen is available and initialized                   |
//+----------------------------------------------------------------------------+
//|Function flow:     fill in parameter for EQFCMD_XDOCACT request             |
//|                   send message to command handler for WM_EQFCMD_XDOCACT    |
//|                   return success                                           |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQF_XDOCACT
(
   PSTEQFGEN pstEQFGen,
   PSZ       pszDocObjName16
)
{
   PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
   USHORT        usRc;                           // return code

   if ( pstEQFGen )
   {
     /*****************************************************************/
     /* use temp variable to overcome VisualAge trouble in 16/32 mod. */
     /*****************************************************************/
      PSZ  pszDocObjName =  pszDocObjName16;

      pstEQFPCmd = pstEQFGen->pstEQFPCmd;

      pstEQFPCmd->usCmd   = EQFCMD_XDOCACT;
      pstEQFPCmd->usLen1  = (USHORT)(strlen (pszDocObjName) + 1);
      pstEQFPCmd->usLen2  = 0;
      memcpy (pstEQFPCmd->ucbBuffer, pszDocObjName, pstEQFPCmd->usLen1);
      pstEQFGen->usRC = 0;

      WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_XDOCACT, NULL, NULL);
      usEQFErrID = pstEQFGen->usRC;

      usRc = pstEQFGen->usRC;
   }
   else
   {
      usRc = EQFRS_NO_INIT;                      // no generic structure yet
   } /* endif */

   return( usRc );
} /* end of function EQF_XDOCACT */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQF_XDOCREMOVE                                           |
//+----------------------------------------------------------------------------+
//|Function call:     usRc = EQF_XDOCREMOVE( pstEQFGen, pInDoc );              |
//+----------------------------------------------------------------------------+
//|Description:       remove specified document from list                      |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN -- pointer to generic struct                   |
//|                   PSZ       -- input buffer                                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0         -- success                                     |
//|                   TM/2 rc                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     fill in parameter for EQFCMD_XDOCREMOVE request          |
//|                   send message to command handler for WM_EQFCMD_XDOCREMOVE |
//|                   return success                                           |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQF_XDOCREMOVE
(
   PSTEQFGEN pstEQFGen,
   PSZ       pDocObjName16
)
{
   PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
   USHORT        usRc;                           // return code

   if ( pstEQFGen )
   {
     /*****************************************************************/
     /* use temp variable to overcome VisualAge trouble in 16/32 mod. */
     /*****************************************************************/
      PSZ  pDocObjName =  pDocObjName16;

      pstEQFPCmd = pstEQFGen->pstEQFPCmd;

      pstEQFPCmd->usCmd   = EQFCMD_XDOCREMOVE;
      pstEQFPCmd->usLen1  = (USHORT)(strlen (pDocObjName) + 1);
      pstEQFPCmd->usLen2  = 0;
      memcpy (pstEQFPCmd->ucbBuffer, pDocObjName, pstEQFPCmd->usLen1);
      pstEQFGen->usRC = 0;

      WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_XDOCREMOVE, NULL, NULL);
      usEQFErrID = pstEQFGen->usRC;

      usRc = pstEQFGen->usRC;
   }
   else
   {
      usRc = EQFRS_NO_INIT;                      // no generic structure yet
   } /* endif */

   return usRc;
} /* end of function EQF_XDOCREMOVE */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQF_XDOCADD                                              |
//+----------------------------------------------------------------------------+
//|Function call:     usRc = EQF_XDOCADD( pstEQFGen, pInDoc );                 |
//+----------------------------------------------------------------------------+
//|Description:       add specified document to list                           |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN -- pointer to generic struct                   |
//|                   PSZ       -- input buffer                                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0         -- success                                     |
//|                   TM/2 rc                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     fill in parameter for EQFCMD_XDOCADD request             |
//|                   send message to command handler for WM_EQFCMD_XDOCADD    |
//|                   return success                                           |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQF_XDOCADD
(
   PSTEQFGEN pstEQFGen,
   PSZ       pDocObjName16
)
{
   PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
   USHORT        usRc;                           // return code

   if ( pstEQFGen )
   {
     /*****************************************************************/
     /* use temp variable to overcome VisualAge trouble in 16/32 mod. */
     /*****************************************************************/
      PSZ  pDocObjName =  pDocObjName16;

      pstEQFPCmd = pstEQFGen->pstEQFPCmd;

      pstEQFPCmd->usCmd   = EQFCMD_XDOCADD;
      pstEQFPCmd->usLen1  = (USHORT)(strlen (pDocObjName) + 1);
      pstEQFPCmd->usLen2  = 0;
      memcpy (pstEQFPCmd->ucbBuffer, pDocObjName, pstEQFPCmd->usLen1);
      pstEQFGen->usRC = 0;

      WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_XDOCADD, NULL, NULL);
      usEQFErrID = pstEQFGen->usRC;

      usRc = pstEQFGen->usRC;
   }
   else
   {
      usRc = EQFRS_NO_INIT;                      // no generic structure yet
   } /* endif */

   return usRc;
} /* end of function EQF_XDOCADD */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQF_XDOCNEXT                                             |
//+----------------------------------------------------------------------------+
//|Function call:     usRc = EQF_XDOCNEXT( pstEQFGen, pInDoc, pNextDoc );      |
//+----------------------------------------------------------------------------+
//|Description:       get next document from list                              |
//|                   Assumption: pOutDocObjName is large enough, pInDoc points|
//|                               to current document                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN -- pointer to generic struct                   |
//|                   PSZ       -- input buffer                                |
//|                   PSZ       -- output buffer                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0         -- success                                     |
//|                   TM/2 rc                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     fill in parameter for EQFCMD_XDOCNEXT request            |
//|                   send message to command handler for WM_EQFCMD_XDOCNEXT   |
//|                   fill output parameter                                    |
//|                   return success                                           |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQF_XDOCNEXT
(
   PSTEQFGEN pstEQFGen,
   PSZ       pInDocObjName16,
   PSZ       pOutDocObjName16
)
{
   PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
   USHORT        usRc;                           // return code

   if ( pstEQFGen )
   {
     /*****************************************************************/
     /* use temp variable to overcome VisualAge trouble in 16/32 mod. */
     /*****************************************************************/
      PSZ  pInDocObjName  =  pInDocObjName16;
      PSZ  pOutDocObjName =  pOutDocObjName16;

      pstEQFPCmd = pstEQFGen->pstEQFPCmd;

      pstEQFPCmd->usCmd   = EQFCMD_XDOCNEXT;
      pstEQFPCmd->usLen1  = (USHORT)(strlen (pInDocObjName) + 1);
      pstEQFPCmd->usLen2  = 0;
      memcpy (pstEQFPCmd->ucbBuffer, pInDocObjName, pstEQFPCmd->usLen1);
      pstEQFGen->usRC = 0;

      WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_XDOCNEXT, NULL, NULL);
      usEQFErrID = pstEQFGen->usRC;
      if ( ! pstEQFGen->usRC )
      {
        if (pOutDocObjName)
        {
          memcpy (pOutDocObjName, pstEQFPCmd->ucbBuffer, pstEQFPCmd->usLen1);
        } /* endif */
      } /* endif */


      usRc = pstEQFGen->usRC;
   }
   else
   {
      usRc = EQFRS_NO_INIT;                      // no generic structure yet
   } /* endif */

   return usRc;
} /* end of function EQF_XDOCNEXT */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQF_XDOCNUM                                              |
//+----------------------------------------------------------------------------+
//|Function call:     usRc = EQF_XDOCNUM( pstEQFGen, i, pOutDocObjName );      |
//+----------------------------------------------------------------------------+
//|Description:       get i-th document from list                              |
//|                   Assumption: pOutDocObjName is large enough               |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN -- pointer to generic struct                   |
//|                   USHORT    -- index of document (starting with 0)         |
//|                   PSZ       -- output buffer                               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0         -- success                                     |
//|                   TM/2 rc                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     fill in parameter for EQFCMD_XDOCNUM request             |
//|                   send message to command handler for WM_EQFCMD_XDOCNUM    |
//|                   fill output parameter                                    |
//|                   return success                                           |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQF_XDOCNUM
(
   PSTEQFGEN pstEQFGen,
   USHORT    usI,
   PSZ       pOutDocObjName16
)
{
   PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
   USHORT        usRc;                           // return code

   if ( pstEQFGen )
   {
     /*****************************************************************/
     /* use temp variable to overcome VisualAge trouble in 16/32 mod. */
     /*****************************************************************/
      PSZ  pOutDocObjName =  pOutDocObjName16;

      pstEQFPCmd = pstEQFGen->pstEQFPCmd;

      pstEQFPCmd->usCmd   = EQFCMD_XDOCNUM;
      pstEQFPCmd->usLen1  = usI;
      pstEQFPCmd->usLen2  = 0;
      pstEQFGen->usRC = 0;

      WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_XDOCNUM, NULL, NULL);
      usEQFErrID = pstEQFGen->usRC;
      if ( ! pstEQFGen->usRC )
      {
        if (pOutDocObjName)
        {
          memcpy (pOutDocObjName, pstEQFPCmd->ucbBuffer, pstEQFPCmd->usLen1);
        } /* endif */
      } /* endif */


      usRc = pstEQFGen->usRC;
   }
   else
   {
      usRc = EQFRS_NO_INIT;                      // no generic structure yet
   } /* endif */

   return usRc;
} /* end of function EQF_XDOCNUM */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQF_XDOCINLIST                                           |
//+----------------------------------------------------------------------------+
//|Function call:     usRc = EQF_XDOCINLIST( pstEQFGen, pDocName, &usI );      |
//+----------------------------------------------------------------------------+
//|Description:       check if the specified document is in the list           |
//+----------------------------------------------------------------------------+
//|Parameters:        PSTEQFGEN -- pointer to generic struct                   |
//|                   PSZ       -- input document                              |
//|                   PUSHORT   -- index of document (starting with 0)         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0         -- everything okay                             |
//|                   TM/2 error                                               |
//+----------------------------------------------------------------------------+
//|Function flow:     fill in parameter for EQFCMD_XDOCINLIST request          |
//|                   send message to command handler for WM_EQFCMD_XDOCINLIST |
//|                   fill output parameter                                    |
//|                   return success                                           |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQF_XDOCINLIST
(
   PSTEQFGEN pstEQFGen,
   PSZ       pInDocObjName16,
   PUSHORT   pusI
)
{
   PSTEQFPCMD    pstEQFPCmd;                     // pointer to packed struct
   USHORT        usRc;                           // return code

   if ( pstEQFGen )
   {
     /*****************************************************************/
     /* use temp variable to overcome VisualAge trouble in 16/32 mod. */
     /*****************************************************************/
      PSZ  pInDocObjName =  pInDocObjName16;

      pstEQFPCmd = pstEQFGen->pstEQFPCmd;

      pstEQFPCmd->usCmd   = EQFCMD_XDOCINLIST;
      pstEQFPCmd->usLen1  = (USHORT)(strlen (pInDocObjName) + 1);
      pstEQFPCmd->usLen2  = 0;
      memcpy (pstEQFPCmd->ucbBuffer, pInDocObjName, pstEQFPCmd->usLen1);
      pstEQFGen->usRC = 0;

      WinSendMsg( pstEQFGen->hwndTWBS, WM_EQFCMD_XDOCINLIST, NULL, NULL);
      usEQFErrID = pstEQFGen->usRC;
      if ( ! pstEQFGen->usRC )
      {
        *pusI = pstEQFPCmd->usLen1;
      } /* endif */


      usRc = pstEQFGen->usRC;
   }
   else
   {
      usRc = EQFRS_NO_INIT;                      // no generic structure yet
   } /* endif */

   return usRc;
} /* end of function EQF_XDOCINLIST */

//__cdecl /*APIENTRY*/ DllEntryPoint
//(
//  HINSTANCE   hInstDll,                // handle of library instance
//  DWORD       fdwReason,               // reason for calling function
//  LPVOID      lpvReserved              // reserved
//)
//{
//  hInstDll; lpvReserved;
//  switch ( fdwReason )
//  {
//    case DLL_PROCESS_ATTACH:
//      // code for first load of DLL
//      break;
//    case DLL_THREAD_ATTACH:
//      // code for additional threads usin/loading DLL
//      break;
//    case DLL_THREAD_DETACH:
//      // cleanup for single process
//      break;
//    case DLL_PROCESS_DETACH:
//      // cleanup/unload of DLL
//      break;
//  }
//  return( TRUE );
//} /* end of function DllEntryPoint */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFADDTOTOC                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFADDTOTOC  (LONG, PUSHORT, PCHAR, PUSHORT)             |
//+----------------------------------------------------------------------------+
//|Description:       Fill the datastring as next item into the listbox        |
//+----------------------------------------------------------------------------+
//|Parameters:        LONG      lInfo,                                         |
//|                   PUSHORT   pusSegNum,                                     |
//|                   PCHAR     pchData,                                       |
//|                   PUSHORT   pusBufSize                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       EQFRC_OK  everything okay                                |
//|                   EQFRS_ENTRY_NOT_AVAIL  out of range                      |
//+----------------------------------------------------------------------------+
//|Function flow:     get pointer to document struct                           |
//|                   return error code                                        |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFADDTOTOC
(
  LONG      lInfo,
  PUSHORT   pusSegNum,                      // segnum of toc entry
  PCHAR     pchData16,                      // data for listbox
  HWND      hwndLB
)
{
  USHORT        usRc = 0;                                  // return code
  PTBDOCUMENT   pDoc;
  SHORT         sItem;
  PCHAR         pchData = pchData16;

  pDoc = (PTBDOCUMENT ) lInfo;                             // is pDOc ok?

  sItem = INSERTITEMENDHWND( hwndLB, pchData);
  if ( sItem != LIT_NONE )
  {
    SETITEMHANDLEHWND( hwndLB, sItem, *(pusSegNum));
  } /* endif */

  return usRc;

} // end 'EQFADDTOTOC  '

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFADDTOTOCW
(
  LONG      lInfo,
  PULONG    pulSegNum,                      // segnum of toc entry
  PCHAR_W   pchData16,                      // data for listbox
  HWND      hwndLB
)
{
  USHORT        usRc = 0;                                  // return code
  PTBDOCUMENT   pDoc;
  SHORT         sItem;
  PCHAR_W       pchData = pchData16;

  pDoc = (PTBDOCUMENT ) lInfo;                             // is pDOc ok?

  sItem = (SHORT) CPLUSPLUS SendMessageW( hwndLB, LB_INSERTSTRING, (WPARAM) -1, (LPARAM)  pchData );
  if ( sItem != LIT_NONE )
  {
    SETITEMHANDLEHWND( hwndLB, sItem, *(pulSegNum));
  } /* endif */

  return usRc;

} // end 'EQFADDTOTOC  '


//+----------------------------------------------------------------------------+
//|API function                                                                |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETDOCCONV                                            |
//+----------------------------------------------------------------------------+
//|Description:       Get the document conversion settings for the specified   |
//|                   document                                                 |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|                   PSZ        pszFolderPath folder path                     |
//|                   PSZ        pszDocName    document short name             |
//|                   PSZ        pszConversion pointer to buffer for selected  |
//|                                            conversion (up to 40 characters)|
//|                   PUSHORT    pusConvFlag   pointer to conversion flags     |
//|                   PUSHORT    pusCodePage   pointer to associated ASCII cp  |
//+----------------------------------------------------------------------------+
//|Returns:           USHORT     return code of called functions               |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETDOCCONV
(
   PSZ        pszFolderPath, // folder path; e.g. "C:\EQF\SAMPLE1.F00"
   PSZ        pszDocName,    // document short name; e.g. "TEST.DOC"
   PSZ        pszConversion, // pointer to buffer for selected
                             // conversion (up to 40 characters)
   PUSHORT    pusConvFlag,   // pointer to buffer for conversion flags
   PUSHORT    pusCodePage    // pointer to buffer for associated ASCII code-page
)
{
   USHORT     usRC = NO_ERROR;         // success indicator
   CHAR       szProperty[MAX_LONGPATH];// Property path


   // if folder and document name has bee specified ...
   if ( (pszFolderPath != NULL) &&
        (*pszFolderPath != EOS) &&
        (pszDocName != NULL)    &&
        (*pszDocName != EOS) )
   {
      CHAR szConversion[MAX_DESCRIPTION]; // buffer for document conversion

      // initialize caller's buffers
      if ( pszConversion )   *pszConversion = EOS;
      if ( pusCodePage )     *pusCodePage = 0;
      if ( pusConvFlag )     *pusConvFlag = 0;

      // setup document object name
      strcpy( szProperty, pszFolderPath );
      strcat( szProperty, BACKSLASH_STR );
      strcat( szProperty, pszDocName );

      // get document conversion
      usRC =  DocQueryInfoEx( szProperty,
                              NULL, NULL, NULL, NULL,
                              NULL, NULL, NULL,
                              szConversion, NULL, NULL,
                              FALSE, NULLHANDLE );

      // get conversion information
      if ( (usRC == NO_ERROR) && (szConversion[0] != EOS) )
      {
        usRC = UtlHandleConversionStrings( CONVCHECK_MODE, NULLHANDLE,
                                           szConversion, pusCodePage, pusConvFlag );
      } /* endif */
   }
   else
   {
     usRC = ERROR_INVALID_DATA;
   }/* end if */

   return usRC;
} /* end of function EQFGETDOCCONV */

// return current version of TranslationManager
__declspec(dllexport)
ULONG __cdecl /*APIENTRY*/ EQFGETVERSION( void )
{
   return( EQFGETDLLVERSION() );
} /* end of function EQFGETVERSION */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETINFO                                               |
//+----------------------------------------------------------------------------+
//|Function call:     EQFGETINFO(LONG, GETINFOID, PCHAR, PUSHORT)              |
//+----------------------------------------------------------------------------+
//|Description:       Return the requested information in the supplied         |
//|                   buffer.                                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        LONG      lInfo,                                         |
//|                   GETINFOID InfoID,                                        |
//|                   PCHAR     pchBuffer,                                     |
//|                   PUSHORT   pusBufSize                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                        everything okay                 |
//|                   ERROR_INVALID_PARAMETER  unknown InfoID or missing       |
//|                                            parameter                       | 
//|                   ERROR_INVALID_HANDLE     invalid lInfo handle            |
//|                   ERROR_NOT_ENOUGH_MEMORY  not enough memory / memory      |
//|                                            allocation failed               | 
//|                   ERROR_INSUFFICIENT_BUFFER buffer is not large enough for |
//|                                            returned information, pusBufSize|
//|                                            contains required buffer size   |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETINFO
(
  LONG         lInfo,                  // info handle from EQFSHOW entry point
  EQFGETINFOID InfoID,                 // ID for requested info  
  PCHAR        pchBuffer,              // buffer for returned info or NULL for info length
  PUSHORT      pusBufSize              // size of buffer, on return size of returned info
)
{
  USHORT        usRC = 0;              // function return code
  PTBDOCUMENT   pDoc = NULL;           // pointer to document structure
  PSZ           pTempData = NULL;      // temporary buffer for requested information

  // check supplied parameters
  if ( (lInfo == 0) || (pusBufSize == NULL) )
  {
    usRC = ERROR_INVALID_PARAMETER;
  } /* endif */

  // check supplied lInfo handle / pDoc pointer
  if ( !usRC )
  {
    pDoc = (PTBDOCUMENT )lInfo;  
  } /* endif */

  // allocate temporary data buffer 
  if ( !usRC )
  {
    if ( !UtlAlloc( (PVOID *)&pTempData, 0L, 2048, NOMSG ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // supply requested information
  if ( !usRC )
  {
    switch ( InfoID )
    {
      case GETINFO_MARKUP:
        {
          CHAR szDocShortName[MAX_FILESPEC];
          CHAR szDocObjName[MAX_EQF_PATH];

          strcpy( szDocObjName, pDoc->szDocName );
          strcpy( szDocShortName, UtlSplitFnameFromPath( szDocObjName ) );
          UtlSplitFnameFromPath( szDocObjName );
          strcat( szDocObjName, BACKSLASH_STR );
          strcat( szDocObjName, szDocShortName );
          DocQueryInfo( szDocObjName, NULL, pTempData, NULL, NULL, FALSE );
        }
        break;
      case GETINFO_FOLDEROBJECT:
        {
          CHAR szPrimDrive[4];

          // setup folder object name
          strcpy( pTempData, pDoc->szDocName );
          UtlSplitFnameFromPath( pTempData );
          UtlSplitFnameFromPath( pTempData );
          UtlQueryString( QST_PRIMARYDRIVE, szPrimDrive, sizeof(szPrimDrive) );
          pTempData[0] = szPrimDrive[0];
        }
        break;
      case GETINFO_FOLDERLONGNAME:
        {
          CHAR szPrimDrive[4];

          // setup folder object name
          strcpy( pTempData, pDoc->szDocName );
          UtlSplitFnameFromPath( pTempData );
          UtlSplitFnameFromPath( pTempData );
          UtlQueryString( QST_PRIMARYDRIVE, szPrimDrive, sizeof(szPrimDrive) );
          pTempData[0] = szPrimDrive[0];

          // get folder long name
          SubFolObjectNameToName( pTempData, pTempData );
        }
        break;
      case GETINFO_DOCFULLPATH:
        strcpy( pTempData, pDoc->szDocName );
        break;
      case GETINFO_DOCLONGNAME:
        strcpy( pTempData, pDoc->szDocLongName );
        break;
      default:
        usRC = ERROR_INVALID_PARAMETER;
        break;
    } /*endswitch */
  } /* endif */

  // return info to caller
  if ( !usRC )
  {
    USHORT usLen = (USHORT)strlen(pTempData) + 1;


    if ( pchBuffer == NULL )
    {
      // caller wants required buffer size for information only
      *pusBufSize = usLen;
    }
    else if ( *pusBufSize < usLen )
    {
      usRC = ERROR_INSUFFICIENT_BUFFER;
      *pusBufSize = usLen;
    }
    else
    {
      strcpy( pchBuffer, pTempData );
      *pusBufSize = usLen;
    } /* endif */
  } /* endif */

  // cleanup
  if ( pTempData ) UtlAlloc( (PVOID *)&pTempData, 0L, 0L, NOMSG );

  // return to caller
  return usRC;

} // end of function EQFGETINFO

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBUILDDOCPATH                                          |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBUILDDOCPATH( PSZ, PSZ, EQFPATHID)                    |
//+----------------------------------------------------------------------------+
//|Description:       Build the fully qualified path name for a document in    |
//|                   a translationManager folder                              |
//+----------------------------------------------------------------------------+
//|Parameters:        PCHAR        pszFolObjName,  folder object name          | 
//|                   PCHAR        pszDocLongName  document long name          | 
//|                   EQFPATHID    PathID          indentifier for requested   |
//|                                                 document subdirectory      |
//|                   PCHAR        pszBuffer       buffer receiving the path   |
//|                                                name, (required size:       |
//|                                                60 bytes or more)           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                        everything okay                 |
//|                   ERROR_INVALID_PARAMETER  wrong or missing parameter      |
//|                   ERROR_FILE_NOT_FOUND     document does not exist         |
//|                   ERROR_PATH_NOT_FOUND     folder does not exist           |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFBUILDDOCPATH
(
  PCHAR        pszFolObjName,          // folder object name
  PCHAR        pszDocLongName,         // document long name  
  EQFPATHID    PathID,                 // indentifier for requested document subdirectory
  PCHAR        pszBuffer               // buffer receiving the path name, (required size: 60 bytes or more)
)
{
  USHORT        usRC = 0;              // function return code
  CHAR          szDocShortName[MAX_FILESPEC]; // buffer for document short name
  BOOL          fIsNew = FALSE;        // document-is-new-flag
  CHAR          chFolDrive = EOS;      // drive where folder resides  

  // check supplied parameters
  if ( (pszFolObjName == NULL) || (pszDocLongName == NULL) || (pszBuffer == NULL))
  {
    usRC = ERROR_INVALID_PARAMETER;
  } /* endif */

  // check if folder exists (load its property file to get the drive letter)
  if ( !usRC )
  {
    EQFINFO     ErrorInfo;           // error code of property handler calls
    PPROPFOLDER pFolProp;            // ptr to folder properties
    PVOID       hFolProp;            // handle of folder properties
    
    hFolProp = OpenProperties( pszFolObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
    if ( hFolProp )
    {
      pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
      chFolDrive = pFolProp->chDrive;
      CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
    }
    else
    {
      usRC = ERROR_PATH_NOT_FOUND;
    } /* endif */
  } /* endif */

  // try to create document short name
  if ( !usRC )
  {  
    FolLongToShortDocName( pszFolObjName, pszDocLongName, szDocShortName, &fIsNew );
  } /* endif */

  // if not successful, scan all documents of folder searching the given document name
  if ( !usRC && fIsNew )
  {
    PSZ pDocNameBuffer = NULL;         // buffer for document names
    PSZ pszCurDoc;                     // ptr to current ocument
    USHORT usDocuments = 0;            // number of documents in buffer  
    PSZ pszDocName;                    // pointer to name part of document name

    // position to file name part in document name
    pszDocName = UtlGetFnameFromPath( pszDocLongName );
    if ( !pszDocName ) pszDocName = pszDocLongName;

    // get list of folder documents using the buffer mode of LoadDocumentNames
    usDocuments = LoadDocumentNames( pszFolObjName, HWND_FUNCIF, 
                                     LOADDOCNAMES_LONGNAME | LOADDOCNAMES_INCLSUBFOLDERS,
                                    (PSZ)&pDocNameBuffer );

    // search document name in returned list
    pszCurDoc = pDocNameBuffer;
    while ( fIsNew && usDocuments )
    {
      // find name part of current document
      PSZ pszCurDocName = UtlGetFnameFromPath( pszCurDoc );
      if ( !pszCurDocName ) pszCurDocName  = pszCurDoc;

      // compare name part with document name being looked for
      if ( stricmp( pszDocName, pszCurDocName ) == 0 )
      {
        // create document short name
        FolLongToShortDocName( pszFolObjName, pszCurDoc, szDocShortName, &fIsNew );
      } 

      // next document
      pszCurDoc = pszCurDoc + strlen(pszCurDoc) + 1;
      usDocuments--;
    } /*endwhile */

    // free any document name buffer
    if ( pDocNameBuffer ) UtlAlloc( (PVOID *)&pDocNameBuffer, 0L, 0L, NOMSG );

    // set error code if document does not exist
    if ( fIsNew)
    {
      usRC = ERROR_FILE_NOT_FOUND;
    } /* endif */
  } /* endif */

  // build document path
  if ( !usRC )
  {
    PSZ pszFolder = UtlGetFnameFromPath( pszFolObjName );
    USHORT usPathID = 0;

    switch ( PathID )
    {
      case PATHID_SOURCE:      usPathID = DIRSOURCEDOC_PATH;     break;
      case PATHID_SEGSOURCE:   usPathID = DIRSEGSOURCEDOC_PATH;  break;
      case PATHID_TARGET:      usPathID = DIRTARGETDOC_PATH;     break;
      case PATHID_SEGTARGET:   usPathID = DIRSEGTARGETDOC_PATH;  break;
      default:                 usRC = ERROR_INVALID_PARAMETER;   break;
    } /*endswitch */

    if ( !usRC )
    {
      UtlMakeEQFPath( pszBuffer, chFolDrive, usPathID, pszFolder );
      strcat( pszBuffer, BACKSLASH_STR );
      strcat( pszBuffer, szDocShortName );
    } /* endif */
  } /* endif */

  // cleanup

  // return to caller
  return( usRC );
} /* end of function EQFBUILDDOCPATH */

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETTAOPTIONS
(
  HANDLE           hAnalysis,                    // analysis process handle
  PEQFTAOPTIONS    pOptions                      // ptr to analysis options
)
{
  USHORT usRC = 0;

  if ( (hAnalysis == 0) || (pOptions == NULL)  )
  {
    usRC = ERROR_INVALID_PARAMETER;
  } /* endif */

  if ( !usRC )
  {
    PTAINPUT         pTAInput = (PTAINPUT)hAnalysis; // input structure for text analysis

    pTAInput->fLeadingWS  = pOptions->fAdjustLeadingWS;
    pTAInput->fTrailingWS = pOptions->fAdjustTrailingWS;
  } /* endif */

  return( usRC );
} /* end of function EQFGETTAOPTIONS */

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFSETTAOPTIONS
(
  HANDLE           hAnalysis,                    // analysis process handle
  PEQFTAOPTIONS    pOptions                      // ptr to analysis options
)
{
  USHORT usRC = 0;

  if ( (hAnalysis == 0) || (pOptions == NULL)  )
  {
    usRC = ERROR_INVALID_PARAMETER;
  } /* endif */

  if ( !usRC )
  {
    PTAINPUT         pTAInput = (PTAINPUT)hAnalysis; // input structure for text analysis

    pTAInput->fLeadingWS  = pOptions->fAdjustLeadingWS;
    pTAInput->fTrailingWS = pOptions->fAdjustTrailingWS;
  } /* endif */
  return( usRC );
} /* end of function EQFGETTAOPTIONS */

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFWRITEHISTLOGFROMFILE
(
  PSZ         pszSegTargetFile                   // fully qualified file name of STARGET file of document
)
{
  return( WriteHistLogAndAdjustCountInfo( pszSegTargetFile, FALSE ) );
} /* end of function EQFWRITEHISTLOGFROMFILE */


// API call to write a history log save record using the count data stored in a segmented target file and adjust the counting information
// stored in the document properties
__declspec(dllexport)
USHORT __cdecl EQFADJUSTCOUNTINFO
(
  PSZ         pszSegTargetFile                   // fully qualified file name of STARGET file of document
)
{
  return( WriteHistLogAndAdjustCountInfo( pszSegTargetFile, TRUE ) );
} /* end of function EQFADJUSTCOUNTINFO */

USHORT WriteHistLogAndAdjustCountInfo
(
  PSZ         pszSegTargetFile,                  // fully qualified file name of STARGET file of document
  BOOL        fAdjustCountInfo                   // TRUE = adjust counting info in document properties 
)
{
  USHORT      usRC = 0;                          // function return code
  PTBDOCUMENT pDocument = NULL;                  // loaded target document
  PTBDOCUMENT pSourceDocument = NULL;            // loaded source document
  CHAR szDocObjName[MAX_EQF_PATH];
  CHAR szDocFormat[MAX_FILESPEC];
  CHAR szSrcLang[MAX_LANG_LENGTH];
  CHAR szTgtLang[MAX_LANG_LENGTH];

  // check input parameter
  if ( !usRC )
  {
    if ( (pszSegTargetFile == NULL) || (*pszSegTargetFile == EOS) )
    {
      usRC = ERROR_INVALID_PARAMETER;
    } /* endif */
  } /* endif */

  // check existence of segmented target file
  if ( !usRC )
  {
    if ( !UtlFileExist( pszSegTargetFile ) )
    {
      usRC = ERROR_NOTARGETFILE;
    } /* endif */
  } /* endif */

  // load segmented target file into memory
  if ( !usRC )
  {
    if ( !UtlAlloc( (PVOID *)&pDocument, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    if ( !usRC )
    {
      strcpy( pDocument->szDocName, pszSegTargetFile );
      usRC = TALoadTagTable( DEFAULT_QFTAG_TABLE, (PLOADEDTABLE *)&(pDocument->pQFTagTable), TRUE, FALSE );
    } /* endif */

    // load document tag table
    if (!usRC )
    {
      strcpy( szDocObjName, pszSegTargetFile );
      PSZ pFileName = UtlSplitFnameFromPath( szDocObjName);   // ptr to filename
      PSZ pTemp = UtlGetFnameFromPath( szDocObjName);         // ptr to starget
      strcpy( pTemp, pFileName );                             // copy filename at position where STARGET was !!
      DocQueryInfo2( szDocObjName, NULL, szDocFormat, szSrcLang, szTgtLang, NULL, NULL, NULL, TRUE );
      if ( szDocFormat[0] )
      {
        usRC = TALoadTagTable( szDocFormat, (PLOADEDTABLE *)&(pDocument->pDocTagTable), FALSE, TRUE );
      } /* endif */
      pDocument->ulOemCodePage = GetLangCodePage( OEM_CP, szTgtLang );
      pDocument->ulAnsiCodePage = GetLangCodePage( ANSI_CP, szTgtLang );
    } /* endif */

    // load document
    if ( !usRC )
    {
      usRC = EQFBFileReadExW( pszSegTargetFile, pDocument, 0L );
    } /* endif */
  } /* endif */

  // load segmented source file into memory
  if ( !usRC )
  {
    CHAR szSegSourceFile[MAX_EQF_PATH];

    if ( !UtlAlloc( (PVOID *)&pSourceDocument, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    if ( !usRC )
    {
      CHAR szTemp[MAX_EQF_PATH];
      strcpy( szTemp, pszSegTargetFile );
      PSZ pszDocName = UtlSplitFnameFromPath( szTemp );
      UtlSplitFnameFromPath( szTemp );
      PSZ pszFolder = UtlGetFnameFromPath( szTemp );
      UtlMakeEQFPath( szSegSourceFile, szTemp[0], DIRSEGSOURCEDOC_PATH, pszFolder );
      strcat( szSegSourceFile, "\\" );
      strcat( szSegSourceFile, pszDocName );
      strcpy( pDocument->szDocName, szSegSourceFile );
      pSourceDocument->pQFTagTable = pDocument->pQFTagTable ;
      pSourceDocument->pDocTagTable = pDocument->pDocTagTable;
      pSourceDocument->ulOemCodePage = GetLangCodePage(OEM_CP, szSrcLang);
      pSourceDocument->ulAnsiCodePage = GetLangCodePage(ANSI_CP, szSrcLang);
    } /* endif */

    if ( !usRC )
    {
      usRC = EQFBFileReadExW( szSegSourceFile, pSourceDocument, 0L );
    } /* endif */
    if ( !usRC )
    {
      pDocument->twin = pSourceDocument;
    } /* endif */
  } /* endif */


  // write history log save record from loaded file
  if ( !usRC )
  {
    usRC = EQFBHistDocSaveEx( pszSegTargetFile, pDocument, DOCAPI_LOGTASK3 );
    if ( !usRC && fAdjustCountInfo )
    {
      EQFBAdjustCountInfo( pDocument, szDocObjName );
    } /* endif */
  } /* endif */

  // cleanup
  if ( pDocument ) 
  {
    if ( pDocument->pQFTagTable) TAFreeTagTable((PLOADEDTABLE)pDocument->pQFTagTable );
    if ( pDocument->pDocTagTable) TAFreeTagTable((PLOADEDTABLE)pDocument->pDocTagTable );
    EQFBFreeDoc( &pDocument, 0 );
  } /* endif */
  if ( pSourceDocument ) EQFBFreeDoc( &pSourceDocument, 0 );

  return( usRC );
} /* end of function WriteHistLogAndAdjustCountInfo */

