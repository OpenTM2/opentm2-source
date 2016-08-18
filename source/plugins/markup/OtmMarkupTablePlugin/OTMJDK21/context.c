/*
*
*  Copyright (C) 1998-2013, International Business Machines  
*         Corporation and others. All rights reserved 
*
*/
/*********************************************************************
 *
 * FUNCTION:
 *
 *   This function will find context for ResourceBundles.            
 *   It contains 3 entry points.                                     
 *   These entry points are listed below and are defined by TM2.     
 *
 ********************************************************************/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#include "otmjdk11.h"

WCHAR     szTWBCTX[9] = L"<TWBCTX>" ;
WCHAR     sz_TWBCTX[10] = L"</TWBCTX>" ;

/*******************************************************************************
*
*       function:       EQFGETSEGCONTEXT
*
* -----------------------------------------------------------------------------
*       Description:
*               Supply the context information for a given segment during analysis.
*               Only for JDK Property files.
*               Assume the current segment is a translatable one.
*       Parameters:
*               PSZ_W  // ptr to text of current segment (UTF16)
*				PSZ_W  // ptr to text of previous segment (UTF16), NULL if none
*				PSZ_W  // ptr to text of next segment (UTF16), NULL if none
*				PSZ_W  // ptr to buffer for context string (UTF16, max 2048 chars)
*				LONG   // handle for usage with next/prev segment calls
*               ULONG  // current segment number for usage with next/prev segment calls
*               
*       Return:
*               EQF_BOOL      // 0=Successful
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFGETSEGCONTEXT
(
  PSZ_W  pszCurSeg,  // ptr to text of current segment (UTF16)
  PSZ_W  pszPrevSeg, // ptr to text of previous segment (UTF16), NULL if none
  PSZ_W  pszNextSeg, // ptr to text of next segment (UTF16), NULL if none
  PSZ_W  pszContext, // ptr to buffer for context string (UTF16, max 2048 chars)
  LONG   lHandle,    // handle for usage with next/prev segment calls
  ULONG  ulSegNum    // current segment number for usage with next/prev segment calls
)
{
  WCHAR 	szPrevSeg[MAX_SEGMENT_SIZE]; /* may store <TWBCTX>, </TWBCTX> temporarily */
  WCHAR 	szContext[MAX_CONTEXT_SIZE];  
  ULONG		usSegNum;
  WCHAR 	*ptrChar1, *ptrChar2;
  USHORT 	rc;
  USHORT 	bytes ;
  USHORT 	maxbytes ;


  szPrevSeg[0] = L'\0' ;
  szContext[0] = L'\0' ;
  usSegNum = ulSegNum;
  maxbytes = (MAX_SEGMENT_SIZE +1)* sizeof(wchar_t) ;
  rc=0;
  
  *pszContext = L'\0'; /* initialization */
  
 // go back until key of property has been found
 
  if ( pszPrevSeg )       
     wcscpy( szPrevSeg, pszPrevSeg ) ;
  --usSegNum ;
 
  while( ( rc == 0        ) && 
         ( szPrevSeg[0]   ) ) {

     for( ptrChar1=NULL, ptrChar2=wcsstr(szPrevSeg, szTWBCTX) ;
          ptrChar2 ;
          ptrChar1=ptrChar2, ptrChar2=wcsstr(ptrChar2+8, szTWBCTX) ) ;
     if ( ptrChar1 ) {
        ptrChar2 = wcsstr(ptrChar1+8, sz_TWBCTX) ;
        if ( ptrChar2 ) {
           *ptrChar2 = L'\0' ;
           wcscpy( szContext, ptrChar1+8 ) ;
           break ;              /* Context found */
        }
     }

     /* get previous segment */
     bytes = maxbytes ;
     rc = EQFGETPREVSEGW( lHandle, &usSegNum, szPrevSeg, &bytes ) ;  /* usSegNum decreases automatically */
  }
  
  if ( szContext[0] ) {
     wcscpy(pszContext,szContext);
  }
  return( 0 );
}


__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFCOMPARECONTEXT
(
  PSZ_W   pszSegContext, // ptr to context of active segment (UTF16)
  PSZ_W   pszTMContext,  // ptr to context of segment from TMem (UTF16)
  PUSHORT pusRanking     // ptr to USHORT receiving the result
)
{
  
  // return 100 for exact context otherwise 0 (0 = do not use proposal at all)
  *pusRanking = ((wcsicmp( pszSegContext, pszTMContext ) == 0 ) ? 100 : 0 );     /* 1->0 */
  
  return( 0 );
}


__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFFORMATCONTEXT
(
  PSZ_W  pszContext,      // ptr to buffer with context string (UTF16)
  PSZ_W  pszDisplayString // ptr to buffer for formatted context (UTF16)
)
{ 
  wcscpy( pszDisplayString, pszContext );
  
  return( 0 );
}



__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFUPDATECONTEXT
(
  PSZ_W       pszCurSeg,               // ptr to text of current segment (UTF16)
  PSZ_W       pszContext               // ptr to buffer for context string (UTF16)
)
{
  WCHAR 	szSeg[MAX_SEGMENT_SIZE];  
  WCHAR 	*ptrChar1, *ptrChar2;
  USHORT 	rc;

  rc=0;
                             
  wcscpy( szSeg, pszCurSeg ) ;
  for( ptrChar1=NULL, ptrChar2=wcsstr(szSeg, szTWBCTX) ;
       ptrChar2 ;
       ptrChar1=ptrChar2, ptrChar2=wcsstr(ptrChar2+8, szTWBCTX) ) ;
  if ( ptrChar1 ) {
     ptrChar2 = wcsstr(ptrChar1+8, sz_TWBCTX) ;
     if ( ptrChar2 ) {
        *ptrChar2 = L'\0' ;
        wcscpy( pszContext, ptrChar1+8 ) ;
     }
  }

  return( 0 );
}
