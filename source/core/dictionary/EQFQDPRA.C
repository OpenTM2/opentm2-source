//+----------------------------------------------------------------------------+
//|EQFQDPRA.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|  Marc Hoffmann                                                             |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Functions for QDPR analyze process                                        |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|    QDPRCreateFCRTElement                                                   |
//|    QDPREndTagInTag                                                         |
//|    QDPRReadEntry                                                           |
//|    QDPRReadHeader                                                          |
//|    QDPRReadOverComment                                                     |
//|    QDPRReadOverDescription                                                 |
//|    QDPRReadPagefoot                                                        |
//|    QDPRReadPagehead                                                        |
//|    QDPRReadRepeatTag                                                       |
//|    QDPRReadSetTag                                                          |
//|    QDPRReadTrailer                                                         |
//|    QDPRReadVarTag                                                          |
//|    QDPRRetrieveValue                                                       |
//|    QDPRSuppressFirstCRLF                                                   |
//|    QDPRSyntaxError                                                         |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
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
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.2 $ ----------- 9 Nov 2001
// --RJ: remove warnings
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 3 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFQDPRA.CV_   1.2   14 Jan 1998 14:36:24   BUILD  $
 *
 * $Log:   J:\DATA\EQFQDPRA.CV_  $
 *
 *    Rev 1.2   14 Jan 1998 14:36:24   BUILD
 * - changed TEXT to TEXT_TOKEN
 *
 *    Rev 1.1   24 Feb 1997 11:09:04   BUILD
 * - fixed PTM KAT0265: Print of dicts with context longer than 126 chars does
 *   not work
 *
 *    Rev 1.0   09 Jan 1996 09:13:14   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define INCL_EQF_DICTPRINT        // dictionary print functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_FILT             // dictionary filter functions
#include <eqf.h>                  // General Translation Manager include file
#include <eqfldbi.h>
#include "EQFQDPRI.H"             // internal header file for dictionary print
#include "EQFQDPR.ID"             // IDs for dictionary print


USHORT QDPRNumberOfLF(USHORT,PSZ);     // checks pszstring for LF

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRCreateFCRTElement                                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRCreateFCRTElement( psctIDA, ppsctCurFCRT, pszFieldname,        |
//|                                usTagID, sctAttribs, pchrBuffer,            |
//|                                pusSyntaxError )                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function adds an element to the current FCRT.                        |
//|                                                                            |
//|  It scans the fieldname from the input <field> or <repeat> tag and         |
//|  looks for it in the dictionary maptable. If the field is found there      |
//|  the function sets the pointer to the data of the field in the node        |
//|  of the appropriate template (current, first_page or last_page             |
//|  template).                                                                |
//|                                                                            |
//|  It is also valid for the system variables.                                |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD psctIDA;        // pointer to thread IDA                     |
//|  PQDPR_FCRT   *ppsctCurFCRT;  // current FCRT structure                    |
//|                               // (or its extension)                        |
//|  PSZ          pszFieldname;   // if it is a <var NAME=> or a               |
//|                               // <repeat NAME=> tag give here the          |
//|                               // fieldname read                            |
//|  USHORT       usTagID;        // tag or system variable ID                 |
//|  QDPR_ATTRIBS sctAttribs;     // filled attribute structure (for           |
//|                               // <repeat> tag all set to '\0'              |
//|  PCHAR        pchrBuffer;     // pointer to buffer location where          |
//|                               // field starts                              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PUSHORT      pusSyntaxError; // syntax error code (only valid if          |
//|                               // returncode is QDPR_SYNTAX_ERROR)          |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_SYNTAX_ERROR     - a syntax error occurred, check syntax error code  |
//|                          for details                                       |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a correctly set up thread IDA                                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - if a new FCRT structure has to be allocated ppsctCurFCRT is             |
//|    set to the new FCRT structure                                           |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF tag ID is QDPR_NAME_ATTR                                               |
//|    Scan maptable for fieldname                                             |
//|    IF fieldname is in maptable                                             |
//|      Get template fieldname                                                |
//|      SWITCH on page position identifier in the attributes                  |
//|        CASE $FIRST_ON_PAGE                                                 |
//|          Set pointer to field data in first_page template                  |
//|        CASE $LAST_ON_PAGE                                                  |
//|          Set pointer to field data in last_page template                   |
//|        DEFAULT                                                             |
//|          Set pointer to field data in current template                     |
//|    ELSE                                                                    |
//|      RETURN QDPR_SYNTAX_ERROR "QDPR_SYER_WRONG_FIELDNAME"                  |
//|  Set pointer to start location of tag in buffer                            |
//|  Set attributes                                                            |
//+----------------------------------------------------------------------------+

USHORT QDPRCreateFCRTElement
(
  PQDPR_THREAD psctIDA,                 // pointer to thread IDA
  PQDPR_FCRT   *ppsctCurFCRT,          // current FCRT structure
                                       // (or its extension)
  PSZ          pszFieldname,           // if it is a <var NAME=> or a
                                       // <repeat NAME=> tag give here the
                                       // fieldname read
  USHORT       usTagID,                // tag or system variable ID
  QDPR_ATTRIBS sctAttribs,             // filled attribute structure (for
                                       // <repeat> tag all set to '\0'
  PCHAR        pchrBuffer,             // pointer to buffer location where
                                       // field starts
  PUSHORT      pusSyntaxError          // syntax error code (only valid if
                                       // returncode is QDPR_SYNTAX_ERROR)
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  USHORT                usElements = 0;           // no of elements
  USHORT                usLevel;                  // level
  USHORT                usFieldIndex;             // field index in maptbl
  PQLDB_NODE            pNode;                    // node pointer
  BOOL                  fFound;                   // found ?



  /********************************************************************/
  /*   check if the current FCRT structure is full, if so allocate    */
  /*            a new one and attach it to the current one            */
  /********************************************************************/
  if ( (*ppsctCurFCRT)->usElements == QDPR_MAX_FCRT_ELEMENTS )
  {
    if ( UtlAlloc( (PVOID *)&((*ppsctCurFCRT)->psctFCRTExtension), 0L,
                   (LONG)sizeof( QDPR_FCRT ), NOMSG ) )
    {
      *ppsctCurFCRT = (*ppsctCurFCRT)->psctFCRTExtension;
    }
    else
    {
      usRC = QDPR_NO_MEMORY;
    } /* endif */
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*         get the current number of elements in the fcrt         */
    /******************************************************************/
    usElements = (*ppsctCurFCRT)->usElements;
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*   check if a NAME= fieldname (either for <var> or <repeat>)    */
    /*                            is given                            */
    /******************************************************************/
    if ( usTagID == QDPR_NAME_ATTR )
    {
      /****************************************************************/
      /* scan the dictionary maptable to look if the fieldname exists */
      /****************************************************************/
      usFieldIndex = 0;
      fFound = FALSE;
      while ( !fFound && ( usFieldIndex < MAX_PROF_ENTRIES ) )
      {
        if ( strcmpi( pszFieldname,
                      psctIDA->pDictProp->ProfEntry[usFieldIndex].chUserName )
             == 0 )
        {
          fFound = TRUE;
        }
        else
        {
          usFieldIndex++;
        } /* endif */
      } /* endwhile */

      if ( fFound )
      {
        /**************************************************************/
        /*          compute index of the field on the level           */
        /**************************************************************/
        usLevel = psctIDA->pDictProp->ProfEntry[usFieldIndex].usLevel - 1;

        usFieldIndex = usFieldIndex - (psctIDA->ausFieldTable)[usLevel];

        /**************************************************************/
        /*          set the pointer to the data in the FCRT           */
        /**************************************************************/
        if ( ( sctAttribs.bitAttribs & QDPR_ATTR_LASTONPAGE )
             == QDPR_ATTR_LASTONPAGE )
        {
          pNode = ((PQLDB_HTREE)psctIDA->psctLastPageTemplate)->apCurLevelNode[usLevel];
        }
        else
        {
          if ( ( sctAttribs.bitAttribs & QDPR_ATTR_FIRSTONPAGE )
               == QDPR_ATTR_FIRSTONPAGE )
          {
            pNode = ((PQLDB_HTREE)psctIDA->psctFirstPageTemplate)->apCurLevelNode[usLevel];
          }
          else
          {
            pNode = ((PQLDB_HTREE)psctIDA->psctCurrentTemplate)->apCurLevelNode[usLevel];
          } /* endif */
        } /* endif */

        (*ppsctCurFCRT)->asctFCRTElements[usElements].ppszFieldData =
          &(pNode->aFields[usFieldIndex].pszData);         //pszData is psz_w
      }
      else
      {
        usRC = QDPR_SYNTAX_ERROR;
        *pusSyntaxError = QDPR_SYER_WRONG_FIELDNAME;
      } /* endif */
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      /****************************************************************/
      /*     set the pointer to the buffer location and copy the      */
      /*                          attributes                          */
      /*                      and set the tag ID                      */
      /****************************************************************/
      (*ppsctCurFCRT)->asctFCRTElements[usElements].pchrFieldStart =
        pchrBuffer;

      memcpy( &((*ppsctCurFCRT)->asctFCRTElements[usElements].sctAttribs),
              &sctAttribs, sizeof( QDPR_ATTRIBS ) );

      (*ppsctCurFCRT)->asctFCRTElements[usElements].usTagID = usTagID;

      /****************************************************************/
      /*         increase the number of elements in the fcrt          */
      /****************************************************************/
      ((*ppsctCurFCRT)->usElements)++;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRCreateFCRTElement */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPREndTagInTag                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  BOOL QDPREndTagInTag( pszString, usStringLen, fBlanksTillEndTag )         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function checks if an end-tag character can be found in the          |
//|  string.                                                                   |
//|                                                                            |
//|  If the string may only contain BLANKS until the end-tag character is      |
//|  reached fBlanksTillEndTag must be set to TRUE.                            |
//|                                                                            |
//|  If an end-tag character is found the function returns TRUE otherwise      |
//|  FALSE.                                                                    |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PSZ    pszString;         // string to be checked for end tag             |
//|  USHORT usStringLen;       // length of pszString or 0 if pszString is     |
//|                            // ended with '\0'                              |
//|  BOOL   fBlanksTillEndTag; // TRUE = only blanks may occurr till the       |
//|                            // end tag character is reached, if a           |
//|                            // non-blank char is reached before the         |
//|                            // returncode is FALSE                          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: BOOL                                                       |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  TRUE      - end-tag character has been found in pszString                 |
//|  FALSE     - either no end-tag character has been found in pszString       |
//|              or pszString contained non-blank characters although          |
//|              fBlanksTillEndTag has been set to TRUE                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Set run pointer to pszString                                              |
//|  WHILE loop should not stop and end-tag character is not found             |
//|    Run over the characters in pszString                                    |
//+----------------------------------------------------------------------------+

BOOL QDPREndTagInTag(

  PSZ    pszString,         // string to be checked for end tag
  USHORT usStringLen,       // length of pszString or 0 if pszString is
                            // ended with '\0'
  BOOL   fBlanksTillEndTag )// TRUE = only blanks may occurr till the
                            // end tag character is reached, if a
                            // non-blank char is reached before the
                            // returncode is FALSE

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  BOOL      fEndTag = FALSE;           // function returncode
  BOOL      fStopLoop = FALSE;         // stop loop
  PSZ       pszRun;                    // run pointer


  if ( pszString != NULL )
  {
    if ( *pszString != NULC )
    {
      pszRun = pszString;

      /****************************************************************/
      /*                   loop over the characters                   */
      /****************************************************************/
      while ( !fStopLoop && !fEndTag )
      {
        if ( *pszRun == QDPR_END_TAG_CHAR )
        {
          /************************************************************/
          /*  an end-tag character has been found, so indicate this   */
          /************************************************************/
          fEndTag = TRUE;
        }
        else
        {
          if ( *pszRun == NULC )
          {
            /**********************************************************/
            /*        a NULC has been found, so stop the loop         */
            /**********************************************************/
            fStopLoop = TRUE;
          }
          else
          {
            /**********************************************************/
            /*    check if a non-blank character is found although    */
            /*              only blanks may only occurr               */
            /**********************************************************/
            if ( ( *pszRun != QDPR_BLANK ) && fBlanksTillEndTag )
            {
              fStopLoop = TRUE;
            }
            else
            {
              /********************************************************/
              /*          the string length has been reached          */
              /********************************************************/
              if ( (USHORT)(pszRun - pszString) >= usStringLen )
              {
                fStopLoop = TRUE;
              }
              else
              {
                pszRun++;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endwhile */
    } /* endif */
  } /* endif */

  return( fEndTag );

} /* end of function QDPREndTagInTag */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadEntry                                                |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadEntry( psctIDA, ppTok )                                    |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads the text and tags between the <entry> and             |
//|  </entry> tags into the entry buffer.                                      |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA; // pointer to thread IDA                   |
//|  PTOKENENTRY           *ppTok;  // pointer to token where the entry        |
//|                                 // token starts                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_SYNTAX_ERROR     - syntax error occurred, psctIDA->usSyntaxError     |
//|                          is set                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the values for the entry part in the IDA will be set up                 |
//|  - if an error occurred psctIDA->usSyntaxError is set appropriately        |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Inititialize entry buffer, entry and repeat FCRT                          |
//|  WHILE end-entry tag is not yet reached                                    |
//|    SWITCH on the tag found                                                 |
//|      CASE var tag                                                          |
//|        Read the var tag (QDPRReadVarTag)                                   |
//|        IF it is not a <var NAME=xxx $LAST_ON_PAGE> tag                     |
//|          Put the var tag in the entry FCRT (QDPRCreateFCRTElement)         |
//|      CASE repeat tag                                                       |
//|        Read the repeat tag (QDPRReadRepeatTag)                             |
//|        Put the repeat tag in the entry FCRT (QDPRCreateFCRTElement)        |
//|      CASE end-repeat tag                                                   |
//|        Put the replacement string for the end-repeat tag in                |
//|          the entry buffer                                                  |
//|      CASE set tag                                                          |
//|        Read the set tag (QDPRReadSetTag)                                   |
//|      CASE text                                                             |
//|        Put the text in the entry buffer (QDPRAddToProcessBuffer)           |
//|      CASE comment tag                                                      |
//|        Read over the comment tag (QDPRReadOverComment)                     |
//|      CASE end-comment tag                                                  |
//|        RETURN a syntax error as the start-comment tag is missing           |
//|      DEFAULT                                                               |
//|        RETURN syntax error that tags are in wrong order                    |
//+----------------------------------------------------------------------------+

USHORT QDPRReadEntry
(
  PQDPR_THREAD          psctIDA, // pointer to thread IDA
  PTOKENENTRY           *ppTok   // pointer to token where the entry
                                 // token starts
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usNameID;                          // name ID of var tag
  USHORT       usRepeatTagsActive = 0;            // no. of active repeat tags
  CHAR         chrReplaced;                       // replace character
  QDPR_ATTRIBS sctAttribs;                        // attributes for var tag



  /********************************************************************/
  /*                        get the next token                        */
  /********************************************************************/
  (*ppTok)++;

  /********************************************************************/
  /*      check if the next token is already the end-entry token      */
  /*                         if so do nothing                         */
  /********************************************************************/
  if ( (*ppTok)->sTokenid != QDPR_ENTRY_ETOKEN )
  {
    /******************************************************************/
    /*             initialize the entry buffers and FCRTs             */
    /*                      and the repeat FCRT                       */
    /******************************************************************/
    if ( UtlAlloc( (PVOID *)&(psctIDA->psctEntryBuffer), 0L,
                   (LONG)( sizeof( QDPR_PROCESS_BUFFER ) +
                           2 * sizeof( QDPR_FCRT ) ), NOMSG ) )
    {
      psctIDA->psctEntryFCRT = (PQDPR_FCRT)( psctIDA->psctEntryBuffer + 1 );
      psctIDA->psctCurEntryFCRT = psctIDA->psctEntryFCRT;

      psctIDA->psctRepeatFCRT = (PQDPR_FCRT)( psctIDA->psctEntryFCRT + 1 );
      psctIDA->psctCurRepeatFCRT = psctIDA->psctRepeatFCRT;

      psctIDA->pchrEntryBuffer = psctIDA->psctEntryBuffer->achrBuffer;
      psctIDA->psctCurEntryExt = psctIDA->psctEntryBuffer;
    }
    else
    {
      usRC = QDPR_NO_MEMORY;
    } /* endif */

    /******************************************************************/
    /*       check if a CRLF follows directly the <entry> tag,        */
    /*                      if so get rid of it                       */
    /******************************************************************/
    if ( usRC == QDPR_NO_ERROR )
    {
      QDPRSuppressFirstCRLF( *ppTok );
    } /* endif */

    /******************************************************************/
    /*           now loop till the end-entry tag is reached           */
    /******************************************************************/
    while ( ( (*ppTok)->sTokenid != QDPR_ENTRY_ETOKEN ) &&
            ( (*ppTok)->sTokenid != ENDOFLIST ) &&
            ( usRC == QDPR_NO_ERROR ) )
    {
      switch ( (*ppTok)->sTokenid )
      {
        case QDPR_VAR_TOKEN :
          {
            /**********************************************************/
            /*                a var tag has been found                */
            /**********************************************************/
            usRC = QDPRReadVarTag( psctIDA, ppTok, &usNameID,
                                   psctIDA->szFieldname,
                                   DICTENTRYLENGTH + 1,
                                   &sctAttribs );
            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*   check if a $LAST_ON_PAGE attribute has been set    */
              /*                 which is not allowed                 */
              /********************************************************/
              if ( ( sctAttribs.bitAttribs & QDPR_ATTR_LASTONPAGE ) ==
                   QDPR_ATTR_LASTONPAGE )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_ATTRIBUTE,
                                 QDPR_VAR_TOKEN, QDPR_EMPTY_STRING, 0,
                                 usNameID, QDPR_LAST_ON_PAGE_ATTR,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*                create a FCRT element                 */
              /********************************************************/
              usRC = QDPRCreateFCRTElement( psctIDA,
                                            &(psctIDA->psctCurEntryFCRT),
                                            psctIDA->szFieldname, usNameID,
                                            sctAttribs,
                                            psctIDA->pchrEntryBuffer,
                                            &(psctIDA->usSyntaxError) );
              if ( ( usRC == QDPR_SYNTAX_ERROR ) &&
                   ( psctIDA->usSyntaxError == QDPR_SYER_WRONG_FIELDNAME ) )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_FIELDNAME,
                                 QDPR_VAR_TOKEN, psctIDA->szFieldname, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*    put the appropriate replacement string in the     */
              /*                     entry buffer                     */
              /********************************************************/
              psctIDA->pchrEntryBuffer[0] = NULC;
              (psctIDA->pchrEntryBuffer)++;
              if ( usNameID == QDPR_NAME_ATTR )
              {
                usRC = QDPRAddToProcessBuffer( &(psctIDA->psctCurEntryExt),
                                               &(psctIDA->pchrEntryBuffer),
                                               QDPR_REP_FIELD_TAG );
              }
              else
              {
                usRC = QDPRAddToProcessBuffer( &(psctIDA->psctCurEntryExt),
                                               &(psctIDA->pchrEntryBuffer),
                                               QDPR_REP_SYSVAR );
              } /* endif */
            } /* endif */
          }
        break;
        case QDPR_REPEAT_TOKEN :
          {
            /**********************************************************/
            /*              a repeat tag has been found               */
            /**********************************************************/
            usRC = QDPRReadRepeatTag( psctIDA, ppTok, &usNameID,
                                      psctIDA->szFieldname,
                                      DICTENTRYLENGTH + 1,
                                      &sctAttribs );

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*                create a FCRT element                 */
              /********************************************************/
              usRC = QDPRCreateFCRTElement( psctIDA,
                                            &(psctIDA->psctCurRepeatFCRT),
                                            psctIDA->szFieldname, usNameID,
                                            sctAttribs,
                                            psctIDA->pchrEntryBuffer,
                                            &(psctIDA->usSyntaxError) );
              if ( ( usRC == QDPR_SYNTAX_ERROR ) &&
                   ( psctIDA->usSyntaxError == QDPR_SYER_WRONG_FIELDNAME ) )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_FIELDNAME,
                                 QDPR_VAR_TOKEN, psctIDA->szFieldname, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*     put the repeat-tag replacement string in the     */
              /*                     entry buffer                     */
              /********************************************************/
              psctIDA->pchrEntryBuffer[0] = NULC;
              (psctIDA->pchrEntryBuffer)++;
              usRC = QDPRAddToProcessBuffer( &(psctIDA->psctCurEntryExt),
                                             &(psctIDA->pchrEntryBuffer),
                                             QDPR_REP_REPEAT_TAG );
            } /* endif */

            /**********************************************************/
            /*   check if a CRLF follows directly the <repeat> tag,   */
            /*                  if so get rid of it                   */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              QDPRSuppressFirstCRLF( *ppTok );
            } /* endif */

            /**********************************************************/
            /*       increase the number of active repeat tags        */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              usRepeatTagsActive++;
            } /* endif */
          }
        break;
        case QDPR_REPEAT_ETOKEN :
          {
            /**********************************************************/
            /* check if the end-repeat tag closes an open repeat tag  */
            /**********************************************************/
            if ( usRepeatTagsActive > 0 )
            {
              usRepeatTagsActive--;
            }
            else
            {
              /********************************************************/
              /*    the corresponding start repeat tag is missing     */
              /********************************************************/
              QDPRSyntaxError( psctIDA, QDPR_SYER_MISSING_START_TAG,
                               QDPR_REPEAT_ETOKEN, QDPR_EMPTY_STRING, 0,
                               QDPR_REPEAT_TOKEN, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */

            /**********************************************************/
            /*    put the end-repeat tag replacement string in the    */
            /*                      entry buffer                      */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              psctIDA->pchrEntryBuffer[0] = NULC;
              (psctIDA->pchrEntryBuffer)++;
              usRC = QDPRAddToProcessBuffer( &(psctIDA->psctCurEntryExt),
                                             &(psctIDA->pchrEntryBuffer),
                                             QDPR_REP_END_REPEAT_TAG );
            } /* endif */

            /**********************************************************/
            /*                   get the next token                   */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              (*ppTok)++;
            } /* endif */

            /**********************************************************/
            /*   check if a CRLF follows directly the </repeat> tag,  */
            /*                  if so get rid of it                   */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              QDPRSuppressFirstCRLF( *ppTok );
            } /* endif */
          }
        break;
        case QDPR_SET_TOKEN :
          {
            /**********************************************************/
            /*                a set tag has been found                */
            /**********************************************************/
            usRC = QDPRReadSetTag( psctIDA, ppTok );

            /**********************************************************/
            /*    check if a CRLF follows directly the <set> tag,     */
            /*                  if so get rid of it                   */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              QDPRSuppressFirstCRLF( *ppTok );
            } /* endif */
          }
        break;
        case TEXT_TOKEN :
          {
            /**********************************************************/
            /*            just simple text has been found             */
            /*    as (*ppTok)->pDataString doesn't end with a '\0'    */
            /*    put one delibertely at the end, but remember the    */
            /*  repleaced character and re-replace it after putting   */
            /*             the text to the process buffer             */
            /**********************************************************/
            chrReplaced = ((*ppTok)->pDataString)[(*ppTok)->usLength];
            ((*ppTok)->pDataString)[(*ppTok)->usLength] = NULC;

            usRC = QDPRAddToProcessBuffer( &(psctIDA->psctCurEntryExt),
                                           &(psctIDA->pchrEntryBuffer),
                                           (*ppTok)->pDataString );

            ((*ppTok)->pDataString)[(*ppTok)->usLength] = chrReplaced;

            (*ppTok)++;
          }
        break;
        case QDPR_COMMENT_TOKEN :
          {
            /**********************************************************/
            /*               comment tag has been found               */
            /**********************************************************/
            usRC = QDPRReadOverComment( ppTok );

            /**********************************************************/
            /*      if a syntax error occurred (i.e. the comment      */
            /*              tag was not properly closed)              */
            /**********************************************************/
            if ( usRC == QDPR_SYNTAX_ERROR )
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                               QDPR_COMMENT_TOKEN, QDPR_EMPTY_STRING, 0,
                               QDPR_COMMENT_ETOKEN, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_COMMENT_ETOKEN :
          {
            /**********************************************************/
            /*  end-comment tag has been found, i.e. a start-comment  */
            /*                     tag is missing                     */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_MISSING_START_TAG,
                             QDPR_COMMENT_ETOKEN, QDPR_EMPTY_STRING, 0,
                             QDPR_COMMENT_TOKEN, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
        default :
          {
            /**********************************************************/
            /*  another tag has been found, so it is in wrong order   */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_TAG_ORDER,
                             QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                             QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
      } /* endswitch */
    } /* endwhile */

    if ( ( (*ppTok)->sTokenid != QDPR_ENTRY_ETOKEN ) &&
         ( usRC == QDPR_NO_ERROR ) )
    {
      /****************************************************************/
      /*     no end-entry tag has been found, so return an error      */
      /****************************************************************/
      QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                       QDPR_ENTRY_TOKEN, QDPR_EMPTY_STRING, 0,
                       QDPR_ENTRY_ETOKEN, QDPR_NO_QDPR_TAG,
                       &usRC, &(psctIDA->usSyntaxError) );
    } /* endif */

    /******************************************************************/
    /*           check if all repeat tags have been closed            */
    /******************************************************************/
    if ( ( usRC == QDPR_NO_ERROR ) && ( usRepeatTagsActive > 0 ) )
    {
      QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                       QDPR_REPEAT_TOKEN, QDPR_EMPTY_STRING, 0,
                       QDPR_REPEAT_ETOKEN, QDPR_NO_QDPR_TAG,
                       &usRC, &(psctIDA->usSyntaxError) );
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      (*ppTok)++;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRReadEntry */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadHeader                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadHeader( psctIDA, ppTok )                                   |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads the text and tags between the <dictfront> and         |
//|  </dictfront> tags into the header buffer.                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA; // pointer to thread IDA                   |
//|  PTOKENENTRY           *ppTok;  // pointer to token where the header       |
//|                                 // token starts                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_SYNTAX_ERROR     - syntax error occurred, psctIDA->usSyntaxError     |
//|                          is set                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the values for the header part in the IDA will be set up                |
//|  - if an error occurred psctIDA->usSyntaxError is set appropriately        |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Inititialize header buffer and header FCRT                                |
//|  WHILE end-header tag is not yet reached                                   |
//|    SWITCH on the tag found                                                 |
//|      CASE var tag                                                          |
//|        Read the var tag (QDPRReadVarTag)                                   |
//|        IF it is not a <var NAME=xxx> tag                                   |
//|          Put the var tag in the header FCRT (QDPRCreateFCRTElement)        |
//|      CASE set tag                                                          |
//|        Read the set tag (QDPRReadSetTag)                                   |
//|      CASE text                                                             |
//|        Put the text in the header buffer (QDPRAddToProcessBuffer)          |
//|      CASE comment tag                                                      |
//|        Read over the comment tag (QDPRReadOverComment)                     |
//|      CASE end-comment tag                                                  |
//|        RETURN a syntax error as the start-comment tag is missing           |
//|      DEFAULT                                                               |
//|        RETURN syntax error that tags are in wrong order                    |
//+----------------------------------------------------------------------------+

USHORT QDPRReadHeader
(
  PQDPR_THREAD          psctIDA, // pointer to thread IDA
  PTOKENENTRY           *ppTok   // pointer to token where the header
                                 // token starts
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usNameID;                          // name ID of var tag
  CHAR         chrReplaced;                       // replace character
  QDPR_ATTRIBS sctAttribs;                        // attributes for var tag



  /********************************************************************/
  /*                        get the next token                        */
  /********************************************************************/
  (*ppTok)++;

  /********************************************************************/
  /*     check if the next token is already the end-header token      */
  /*                         if so do nothing                         */
  /********************************************************************/
  if ( (*ppTok)->sTokenid != QDPR_DICTFRONT_ETOKEN )
  {
    /******************************************************************/
    /*            initialize the header buffers and FCRTs             */
    /******************************************************************/
    if ( UtlAlloc( (PVOID *)&(psctIDA->psctHeaderBuffer), 0L,
                   (LONG)( sizeof( QDPR_PROCESS_BUFFER ) +
                           sizeof( QDPR_FCRT ) ), NOMSG ) )
    {
      psctIDA->psctHeaderFCRT = (PQDPR_FCRT)( psctIDA->psctHeaderBuffer + 1 );
      psctIDA->psctCurHeaderFCRT = psctIDA->psctHeaderFCRT;

      psctIDA->pchrHeaderBuffer = psctIDA->psctHeaderBuffer->achrBuffer;
      psctIDA->psctCurHeaderExt = psctIDA->psctHeaderBuffer;
    }
    else
    {
      usRC = QDPR_NO_MEMORY;
    } /* endif */

    /******************************************************************/
    /*       check if a CRLF follows directly the <header> tag,       */
    /*                      if so get rid of it                       */
    /******************************************************************/
    if ( usRC == QDPR_NO_ERROR )
    {
      QDPRSuppressFirstCRLF( *ppTok );
    } /* endif */

    /******************************************************************/
    /*          now loop till the end-header tag is reached           */
    /******************************************************************/
    while ( ( (*ppTok)->sTokenid != QDPR_DICTFRONT_ETOKEN ) &&
            ( (*ppTok)->sTokenid != ENDOFLIST ) &&
            ( usRC == QDPR_NO_ERROR ) )
    {
      switch ( (*ppTok)->sTokenid )
      {
        case QDPR_VAR_TOKEN :
          {
            /**********************************************************/
            /*                a var tag has been found                */
            /**********************************************************/
            usRC = QDPRReadVarTag( psctIDA, ppTok, &usNameID,
                                   psctIDA->szFieldname,
                                   DICTENTRYLENGTH + 1,
                                   &sctAttribs );
            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*   check if a NAME= var has been specified which is   */
              /*                     not allowed                      */
              /********************************************************/
              if ( usNameID == QDPR_NAME_ATTR )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_TAG_ORDER,
                                 QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*                create a FCRT element                 */
              /********************************************************/
              usRC = QDPRCreateFCRTElement( psctIDA,
                                            &(psctIDA->psctCurHeaderFCRT),
                                            psctIDA->szFieldname, usNameID,
                                            sctAttribs,
                                            psctIDA->pchrHeaderBuffer,
                                            &(psctIDA->usSyntaxError) );
              if ( ( usRC == QDPR_SYNTAX_ERROR ) &&
                   ( psctIDA->usSyntaxError == QDPR_SYER_WRONG_FIELDNAME ) )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_FIELDNAME,
                                 QDPR_VAR_TOKEN, psctIDA->szFieldname, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*   put the replacement string in the header buffer    */
              /********************************************************/
              psctIDA->pchrHeaderBuffer[0] = NULC;
              (psctIDA->pchrHeaderBuffer)++;
              usRC = QDPRAddToProcessBuffer(
                            &(psctIDA->psctCurHeaderExt),
                            &(psctIDA->pchrHeaderBuffer),
                            QDPR_REP_SYSVAR );
            } /* endif */
          }
        break;
        case QDPR_SET_TOKEN :
          {
            /**********************************************************/
            /*                a set tag has been found                */
            /**********************************************************/
            usRC = QDPRReadSetTag( psctIDA, ppTok );

            /**********************************************************/
            /*    check if a CRLF follows directly the <set> tag,     */
            /*                  if so get rid of it                   */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              QDPRSuppressFirstCRLF( *ppTok );
            } /* endif */
          }
        break;
        case TEXT_TOKEN :
          {
            /**********************************************************/
            /*            just simple text has been found             */
            /*    as (*ppTok)->pDataString doesn't end with a '\0'    */
            /*    put one delibertely at the end, but remember the    */
            /*  repleaced character and re-replace it after putting   */
            /*             the text to the process buffer             */
            /**********************************************************/
            chrReplaced = ((*ppTok)->pDataString)[(*ppTok)->usLength];
            ((*ppTok)->pDataString)[(*ppTok)->usLength] = NULC;

            usRC = QDPRAddToProcessBuffer( &(psctIDA->psctCurHeaderExt),
                                           &(psctIDA->pchrHeaderBuffer),
                                           (*ppTok)->pDataString );

            ((*ppTok)->pDataString)[(*ppTok)->usLength] = chrReplaced;

            (*ppTok)++;
          }
        break;
        case QDPR_COMMENT_TOKEN :
          {
            /**********************************************************/
            /*               comment tag has been found               */
            /**********************************************************/
            usRC = QDPRReadOverComment( ppTok );

            /**********************************************************/
            /*      if a syntax error occurred (i.e. the comment      */
            /*              tag was not properly closed)              */
            /**********************************************************/
            if ( usRC == QDPR_SYNTAX_ERROR )
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                               QDPR_COMMENT_TOKEN, QDPR_EMPTY_STRING, 0,
                               QDPR_COMMENT_ETOKEN, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_COMMENT_ETOKEN :
          {
            /**********************************************************/
            /*  end-comment tag has been found, i.e. a start-comment  */
            /*                     tag is missing                     */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_MISSING_START_TAG,
                             QDPR_COMMENT_ETOKEN, QDPR_EMPTY_STRING, 0,
                             QDPR_COMMENT_TOKEN, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
        default :
          {
            /**********************************************************/
            /*  another tag has been found, so it is in wrong order   */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_TAG_ORDER,
                             QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                             QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
      } /* endswitch */
    } /* endwhile */

    if ( ( (*ppTok)->sTokenid != QDPR_DICTFRONT_ETOKEN ) &&
         ( usRC == QDPR_NO_ERROR ) )
    {
      /****************************************************************/
      /*     no end-header tag has been found, so return an error     */
      /****************************************************************/
      QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                       QDPR_DICTFRONT_TOKEN, QDPR_EMPTY_STRING, 0,
                       QDPR_DICTFRONT_ETOKEN, QDPR_NO_QDPR_TAG,
                       &usRC, &(psctIDA->usSyntaxError) );
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      (*ppTok)++;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRReadHeader */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadOverComment                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadOverComment( ppTok )                                       |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads over the text (and tags) that are inbetween the       |
//|  comment and end-comment tag.                                              |
//|                                                                            |
//|  The function expects a pointer to the tokenized format info file buffer   |
//|  area which is positioned on the comment token.                            |
//|                                                                            |
//|  It reads as long as no end-comment token appears or until the end of      |
//|  the list is reached.                                                      |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PTOKENENTRY *ppTok; // pointer to token where the comment                 |
//|                      // token starts                                       |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_SYNTAX_ERROR     - the corresponding end-comment tag could           |
//|                          not be found                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - tokenized format information file                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - ppTok is set to the token behind the end-comment token                  |
//|    or to the ENDOFLIST token                                               |
//|  - if CRLF follows end-comment, this is skipped too                        |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  WHILE end-comment has not appeared and list is not at the end             |
//|    Get next token                                                          |
//|    IF token is a new comment token                                         |
//|      CALL QDPRReadOverComment recursively                                  |
//|  IF end-comment token has been found                                       |
//|    Get next token                                                          |
//|  If CRLF follows skip it                                                   |
//+----------------------------------------------------------------------------+

USHORT QDPRReadOverComment
(
  PTOKENENTRY *ppTok  // pointer to token where the comment
                      // token starts
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;               // function returncode
  PSZ          pszData;



  if ( (*ppTok)->sTokenid == QDPR_COMMENT_TOKEN )
  {
    (*ppTok)++;

    while ( ( (*ppTok)->sTokenid != QDPR_COMMENT_ETOKEN ) &&
            ( (*ppTok)->sTokenid != ENDOFLIST ) &&
            ( usRC == QDPR_NO_ERROR ) )
    {
      /****************************************************************/
      /*      if another comment token in comment has been found      */
      /*              start searching for a token again               */
      /****************************************************************/
      if ( (*ppTok)->sTokenid == QDPR_COMMENT_TOKEN )
      {
        usRC = QDPRReadOverComment( ppTok );
      } /* endif */

      if ( ( (*ppTok)->sTokenid != QDPR_COMMENT_ETOKEN ) &&
           ( (*ppTok)->sTokenid != ENDOFLIST ) &&
           ( usRC == QDPR_NO_ERROR ) )
      {
        (*ppTok)++;
      } /* endif */
    } /* endwhile */

    if ( (*ppTok)->sTokenid == QDPR_COMMENT_ETOKEN )
    {
      (*ppTok)++;
      /****************************************************************/
      /* skip CRLF if it follows immediately                          */
      /****************************************************************/
      if ( (*ppTok)->sTokenid == TEXT_TOKEN )
      {
        pszData = (*ppTok)->pDataString;
        if ( ((*ppTok)->usLength >= 2 )
            &&  (*pszData == CR) &&  (*(pszData+1) == LF) )
        {
           if ( (*ppTok)->usLength == 2 )
           {
             /*********************************************************/
             /* set length to zero                                    */
             /*********************************************************/
             (*ppTok)->usLength = 0;
           }
           else
           {
             (*ppTok)->usLength -= 2;
             (*ppTok)->pDataString += 2;
           } /* endif */

        } /* endif */
      } /* endif */
    }
    else
    {
      usRC = QDPR_SYNTAX_ERROR;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRReadOverComment */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadOverDescription                                      |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadOverDescription( ppTok )                                   |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads over anything between the <description> and           |
//|  </description> tag.                                                       |
//|                                                                            |
//|  If the </description> tag cannot be found QDPR_SYNTAX_ERROR is returned.  |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PTOKENENTRY *ppTok; // pointer to token where the description             |
//|                      // token starts                                       |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_SYNTAX_ERROR     - the corresponding end-description tag could       |
//|                          not be found                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - tokenized format information file                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - ppTok is set to the token behind the end-description token              |
//|    or to the ENDOFLIST token                                               |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  WHILE end-description has not appeared and list is not at the end         |
//|    Get next token                                                          |
//|  IF end-description token has been found                                   |
//|    Get next token                                                          |
//|  ELSE                                                                      |
//|    RETURN QDPR_SYNTAX_ERROR                                                |
//+----------------------------------------------------------------------------+

USHORT QDPRReadOverDescription
(
  PTOKENENTRY *ppTok // pointer to token where the description
                      // token starts
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode



  while ( ( (*ppTok)->sTokenid != QDPR_DESCRIPTION_ETOKEN ) &&
          ( (*ppTok)->sTokenid != ENDOFLIST ) )
  {
    (*ppTok)++;
  } /* endwhile */

  if ( (*ppTok)->sTokenid == QDPR_DESCRIPTION_ETOKEN )
  {
    (*ppTok)++;
  }
  else
  {
    usRC = QDPR_SYNTAX_ERROR;
  } /* endif */

  return( usRC );

} /* end of function QDPRReadOverDescription */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadPagefoot                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadPagefoot( psctIDA, ppTok )                                 |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads the text and tags between the <pagefoot> and          |
//|  </pagefoot> tags into the pagefoot buffer.                                |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA; // pointer to thread IDA                   |
//|  PTOKENENTRY           *ppTok;  // pointer to token where the pagefoot     |
//|                                 // token starts                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_SYNTAX_ERROR     - syntax error occurred, psctIDA->usSyntaxError     |
//|                          is set                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the values for the pagefoot part in the IDA will be set up              |
//|  - if an error occurred psctIDA->usSyntaxError is set appropriately        |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Inititialize pagefoot buffer and pagefoot FCRT                            |
//|  WHILE end-pagefoot tag is not yet reached                                 |
//|    SWITCH on the tag found                                                 |
//|      CASE var tag                                                          |
//|        Read the var tag (QDPRReadVarTag)                                   |
//|        Put the var tag in the pagefoot FCRT (QDPRCreateFCRTElement)        |
//|      CASE set tag                                                          |
//|        Read the set tag (QDPRReadSetTag)                                   |
//|      CASE text                                                             |
//|        Put the text in the pagefoot buffer (QDPRAddToProcessBuffer)        |
//|      CASE comment tag                                                      |
//|        Read over the comment tag (QDPRReadOverComment)                     |
//|      CASE end-comment tag                                                  |
//|        RETURN a syntax error as the start-comment tag is missing           |
//|      DEFAULT                                                               |
//|        RETURN syntax error that tags are in wrong order                    |
//+----------------------------------------------------------------------------+

USHORT QDPRReadPagefoot
(
  PQDPR_THREAD          psctIDA, // pointer to thread IDA
  PTOKENENTRY           *ppTok   // pointer to token where the pagefoot
                                 // token starts
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usNameID;                          // name ID of var tag
  CHAR         chrReplaced;                       // replace character
  QDPR_ATTRIBS sctAttribs;                        // attributes for var tag



  /********************************************************************/
  /*                        get the next token                        */
  /********************************************************************/
  (*ppTok)++;

  /********************************************************************/
  /*    check if the next token is already the end-pagefoot token     */
  /*                         if so do nothing                         */
  /********************************************************************/
  if ( (*ppTok)->sTokenid != QDPR_PAGEFOOT_ETOKEN )
  {
    /******************************************************************/
    /*           initialize the pagefoot buffers and FCRTs            */
    /******************************************************************/
    if ( UtlAlloc( (PVOID *)&(psctIDA->psctPagefootBuffer), 0L,
                   (LONG)( sizeof( QDPR_PROCESS_BUFFER ) +
                           sizeof( QDPR_FCRT ) ), NOMSG ) )
    {
      psctIDA->psctPagefootFCRT =
                   (PQDPR_FCRT)( psctIDA->psctPagefootBuffer + 1 );
      psctIDA->psctCurPagefootFCRT = psctIDA->psctPagefootFCRT;

      psctIDA->pchrPagefootBuffer = psctIDA->psctPagefootBuffer->achrBuffer;
      psctIDA->psctCurPagefootExt = psctIDA->psctPagefootBuffer;
    }
    else
    {
      usRC = QDPR_NO_MEMORY;
    } /* endif */

    /******************************************************************/
    /*      check if a CRLF follows directly the <pagefoot> tag,      */
    /*                      if so get rid of it                       */
    /******************************************************************/
    if ( usRC == QDPR_NO_ERROR )
    {
      QDPRSuppressFirstCRLF( *ppTok );
    } /* endif */

    /******************************************************************/
    /*         now loop till the end-pagefoot tag is reached          */
    /******************************************************************/
    while ( ( (*ppTok)->sTokenid != QDPR_PAGEFOOT_ETOKEN ) &&
            ( (*ppTok)->sTokenid != ENDOFLIST ) &&
            ( usRC == QDPR_NO_ERROR ) )
    {
      switch ( (*ppTok)->sTokenid )
      {
        case QDPR_VAR_TOKEN :
          {
            /**********************************************************/
            /*                a var tag has been found                */
            /**********************************************************/
            usRC = QDPRReadVarTag( psctIDA, ppTok, &usNameID,
                                   psctIDA->szFieldname,
                                   DICTENTRYLENGTH + 1,
                                   &sctAttribs );

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*                create a FCRT element                 */
              /********************************************************/
              usRC = QDPRCreateFCRTElement( psctIDA,
                                            &(psctIDA->psctCurPagefootFCRT),
                                            psctIDA->szFieldname, usNameID,
                                            sctAttribs,
                                            psctIDA->pchrPagefootBuffer,
                                            &(psctIDA->usSyntaxError) );
              if ( ( usRC == QDPR_SYNTAX_ERROR ) &&
                   ( psctIDA->usSyntaxError == QDPR_SYER_WRONG_FIELDNAME ) )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_FIELDNAME,
                                 QDPR_VAR_TOKEN, psctIDA->szFieldname, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*    put the appropriate replacement string in the     */
              /*                   pagefoot buffer                    */
              /********************************************************/
              psctIDA->pchrPagefootBuffer[0] = NULC;
              (psctIDA->pchrPagefootBuffer)++;
              if ( usNameID == QDPR_NAME_ATTR )
              {
                usRC = QDPRAddToProcessBuffer(
                              &(psctIDA->psctCurPagefootExt),
                              &(psctIDA->pchrPagefootBuffer),
                              QDPR_REP_FIELD_TAG );
              }
              else
              {
                usRC = QDPRAddToProcessBuffer(
                              &(psctIDA->psctCurPagefootExt),
                              &(psctIDA->pchrPagefootBuffer),
                              QDPR_REP_SYSVAR );
              } /* endif */
            } /* endif */
          }
        break;
        case QDPR_SET_TOKEN :
          {
            /**********************************************************/
            /*                a set tag has been found                */
            /**********************************************************/
            usRC = QDPRReadSetTag( psctIDA, ppTok );

            /**********************************************************/
            /*    check if a CRLF follows directly the <set> tag,     */
            /*                  if so get rid of it                   */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              QDPRSuppressFirstCRLF( *ppTok );
            } /* endif */
          }
        break;
        case TEXT_TOKEN :
          {
            /**********************************************************/
            /* adjust # of linefeeds in pageheader                    */
            /**********************************************************/
            psctIDA->usCaPagefootLines = psctIDA->usCaPagefootLines +
                                  QDPRNumberOfLF(((*ppTok)->usLength),
                                                 ((*ppTok)->pDataString));
            /**********************************************************/
            /*            just simple text has been found             */
            /*    as (*ppTok)->pDataString doesn't end with a '\0'    */
            /*    put one delibertely at the end, but remember the    */
            /*  repleaced character and re-replace it after putting   */
            /*             the text to the process buffer             */
            /**********************************************************/
            chrReplaced = ((*ppTok)->pDataString)[(*ppTok)->usLength];
            ((*ppTok)->pDataString)[(*ppTok)->usLength] = NULC;

            usRC = QDPRAddToProcessBuffer( &(psctIDA->psctCurPagefootExt),
                                           &(psctIDA->pchrPagefootBuffer),
                                           (*ppTok)->pDataString );

            ((*ppTok)->pDataString)[(*ppTok)->usLength] = chrReplaced;

            (*ppTok)++;
          }
        break;
        case QDPR_COMMENT_TOKEN :
          {
            /**********************************************************/
            /*               comment tag has been found               */
            /**********************************************************/
            usRC = QDPRReadOverComment( ppTok );

            /**********************************************************/
            /*      if a syntax error occurred (i.e. the comment      */
            /*              tag was not properly closed)              */
            /**********************************************************/
            if ( usRC == QDPR_SYNTAX_ERROR )
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                               QDPR_COMMENT_TOKEN, QDPR_EMPTY_STRING, 0,
                               QDPR_COMMENT_ETOKEN, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_COMMENT_ETOKEN :
          {
            /**********************************************************/
            /*  end-comment tag has been found, i.e. a start-comment  */
            /*                     tag is missing                     */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_MISSING_START_TAG,
                             QDPR_COMMENT_ETOKEN, QDPR_EMPTY_STRING, 0,
                             QDPR_COMMENT_TOKEN, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
        default :
          {
            /**********************************************************/
            /*  another tag has been found, so it is in wrong order   */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_TAG_ORDER,
                             QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                             QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
      } /* endswitch */
    } /* endwhile */

    if ( ( (*ppTok)->sTokenid != QDPR_PAGEFOOT_ETOKEN ) &&
         ( usRC == QDPR_NO_ERROR ) )
    {
      /****************************************************************/
      /*    no end-pagefoot tag has been found, so return an error    */
      /****************************************************************/
      QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                       QDPR_PAGEFOOT_TOKEN, QDPR_EMPTY_STRING, 0,
                       QDPR_PAGEFOOT_ETOKEN, QDPR_NO_QDPR_TAG,
                       &usRC, &(psctIDA->usSyntaxError) );
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      (*ppTok)++;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRReadPagefoot */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadPagehead                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadPagehead( psctIDA, ppTok )                                 |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads the text and tags between the <pagehead> and          |
//|  </pagehead> tags into the pagehead buffer.                                |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA; // pointer to thread IDA                   |
//|  PTOKENENTRY           *ppTok;  // pointer to token where the pagehead     |
//|                                 // token starts                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_SYNTAX_ERROR     - syntax error occurred, psctIDA->usSyntaxError     |
//|                          is set                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the values for the pagehead part in the IDA will be set up              |
//|  - if an error occurred psctIDA->usSyntaxError is set appropriately        |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Inititialize pagehead buffer and pagehead FCRT                            |
//|  WHILE end-pagehead tag is not yet reached                                 |
//|    SWITCH on the tag found                                                 |
//|      CASE var tag                                                          |
//|        Read the var tag (QDPRReadVarTag)                                   |
//|        IF it is not a <var NAME=xxx $LAST_ON_PAGE> tag                     |
//|          Put the var tag in the pagehead FCRT (QDPRCreateFCRTElement)      |
//|      CASE set tag                                                          |
//|        Read the set tag (QDPRReadSetTag)                                   |
//|      CASE text                                                             |
//|        Put the text in the pagehead buffer (QDPRAddToProcessBuffer)        |
//|      CASE comment tag                                                      |
//|        Read over the comment tag (QDPRReadOverComment)                     |
//|      CASE end-comment tag                                                  |
//|        RETURN a syntax error as the start-comment tag is missing           |
//|      DEFAULT                                                               |
//|        RETURN syntax error that tags are in wrong order                    |
//+----------------------------------------------------------------------------+

USHORT QDPRReadPagehead
(
  PQDPR_THREAD          psctIDA, // pointer to thread IDA
  PTOKENENTRY           *ppTok   // pointer to token where the pagehead
)                                 // token starts

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usNameID;                          // name ID of var tag
  CHAR         chrReplaced;                       // replace character
  QDPR_ATTRIBS sctAttribs;                        // attributes for var tag



  /********************************************************************/
  /*                        get the next token                        */
  /********************************************************************/
  (*ppTok)++;

  /********************************************************************/
  /*    check if the next token is already the end-pagehead token     */
  /*                         if so do nothing                         */
  /********************************************************************/
  if ( (*ppTok)->sTokenid != QDPR_PAGEHEAD_ETOKEN )
  {
    /******************************************************************/
    /*           initialize the pagehead buffers and FCRTs            */
    /******************************************************************/
    if ( UtlAlloc( (PVOID *)&(psctIDA->psctPageheadBuffer), 0L,
                   (LONG)( sizeof( QDPR_PROCESS_BUFFER ) +
                           sizeof( QDPR_FCRT ) ), NOMSG ) )
    {
      psctIDA->psctPageheadFCRT =
                   (PQDPR_FCRT)( psctIDA->psctPageheadBuffer + 1 );
      psctIDA->psctCurPageheadFCRT = psctIDA->psctPageheadFCRT;

      psctIDA->pchrPageheadBuffer = psctIDA->psctPageheadBuffer->achrBuffer;
      psctIDA->psctCurPageheadExt = psctIDA->psctPageheadBuffer;
    }
    else
    {
      usRC = QDPR_NO_MEMORY;
    } /* endif */

    /******************************************************************/
    /*      check if a CRLF follows directly the <pagefoot> tag,      */
    /*                      if so get rid of it                       */
    /******************************************************************/
    if ( usRC == QDPR_NO_ERROR )
    {
      QDPRSuppressFirstCRLF( *ppTok );
    } /* endif */

    /******************************************************************/
    /*         now loop till the end-pagehead tag is reached          */
    /******************************************************************/
    while ( ( (*ppTok)->sTokenid != QDPR_PAGEHEAD_ETOKEN ) &&
            ( (*ppTok)->sTokenid != ENDOFLIST ) &&
            ( usRC == QDPR_NO_ERROR ) )
    {
      switch ( (*ppTok)->sTokenid )
      {
        case QDPR_VAR_TOKEN :
          {
            /**********************************************************/
            /*                a var tag has been found                */
            /**********************************************************/
            usRC = QDPRReadVarTag( psctIDA, ppTok, &usNameID,
                                   psctIDA->szFieldname,
                                   DICTENTRYLENGTH + 1,
                                   &sctAttribs );
            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*   check if a $LAST_ON_PAGE attribute has been set    */
              /*                 which is not allowed                 */
              /********************************************************/
              if ( ( sctAttribs.bitAttribs & QDPR_ATTR_LASTONPAGE ) ==
                   QDPR_ATTR_LASTONPAGE )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_ATTRIBUTE,
                                 QDPR_VAR_TOKEN, QDPR_EMPTY_STRING, 0,
                                 usNameID, QDPR_LAST_ON_PAGE_ATTR,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*                create a FCRT element                 */
              /********************************************************/
              usRC = QDPRCreateFCRTElement( psctIDA,
                                            &(psctIDA->psctCurPageheadFCRT),
                                            psctIDA->szFieldname, usNameID,
                                            sctAttribs,
                                            psctIDA->pchrPageheadBuffer,
                                            &(psctIDA->usSyntaxError) );
              if ( ( usRC == QDPR_SYNTAX_ERROR ) &&
                   ( psctIDA->usSyntaxError == QDPR_SYER_WRONG_FIELDNAME ) )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_FIELDNAME,
                                 QDPR_VAR_TOKEN, psctIDA->szFieldname, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*    put the appropriate replacement string in the     */
              /*                   pagehead buffer                    */
              /********************************************************/
              psctIDA->pchrPageheadBuffer[0] = NULC;
              (psctIDA->pchrPageheadBuffer)++;
              if ( usNameID == QDPR_NAME_ATTR )
              {
                usRC = QDPRAddToProcessBuffer(
                              &(psctIDA->psctCurPageheadExt),
                              &(psctIDA->pchrPageheadBuffer),
                              QDPR_REP_FIELD_TAG );
              }
              else
              {
                usRC = QDPRAddToProcessBuffer(
                              &(psctIDA->psctCurPageheadExt),
                              &(psctIDA->pchrPageheadBuffer),
                              QDPR_REP_SYSVAR );
              } /* endif */
            } /* endif */
          }
        break;
        case QDPR_SET_TOKEN :
          {
            /**********************************************************/
            /*                a set tag has been found                */
            /**********************************************************/
            usRC = QDPRReadSetTag( psctIDA, ppTok );

            /**********************************************************/
            /*    check if a CRLF follows directly the <set> tag,     */
            /*                  if so get rid of it                   */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              QDPRSuppressFirstCRLF( *ppTok );
            } /* endif */
          }
        break;
        case TEXT_TOKEN :
          {
            /**********************************************************/
            /* adjust # of linefeeds in pageheader                    */
            /**********************************************************/
            psctIDA->usCaPageheadLines = psctIDA->usCaPageheadLines +
                                  QDPRNumberOfLF(((*ppTok)->usLength),
                                                 ((*ppTok)->pDataString));

            /**********************************************************/
            /*            just simple text has been found             */
            /*    as (*ppTok)->pDataString doesn't end with a '\0'    */
            /*    put one delibertely at the end, but remember the    */
            /*  repleaced character and re-replace it after putting   */
            /*             the text to the process buffer             */
            /**********************************************************/
            chrReplaced = ((*ppTok)->pDataString)[(*ppTok)->usLength];
            ((*ppTok)->pDataString)[(*ppTok)->usLength] = NULC;

            usRC = QDPRAddToProcessBuffer( &(psctIDA->psctCurPageheadExt),
                                           &(psctIDA->pchrPageheadBuffer),
                                           (*ppTok)->pDataString );

            ((*ppTok)->pDataString)[(*ppTok)->usLength] = chrReplaced;

            (*ppTok)++;
          }
        break;
        case QDPR_COMMENT_TOKEN :
          {
            /**********************************************************/
            /*               comment tag has been found               */
            /**********************************************************/
            usRC = QDPRReadOverComment( ppTok );

            /**********************************************************/
            /*      if a syntax error occurred (i.e. the comment      */
            /*              tag was not properly closed)              */
            /**********************************************************/
            if ( usRC == QDPR_SYNTAX_ERROR )
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                               QDPR_COMMENT_TOKEN, QDPR_EMPTY_STRING, 0,
                               QDPR_COMMENT_ETOKEN, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_COMMENT_ETOKEN :
          {
            /**********************************************************/
            /*  end-comment tag has been found, i.e. a start-comment  */
            /*                     tag is missing                     */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_MISSING_START_TAG,
                             QDPR_COMMENT_ETOKEN, QDPR_EMPTY_STRING, 0,
                             QDPR_COMMENT_TOKEN, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
        default :
          {
            /**********************************************************/
            /*  another tag has been found, so it is in wrong order   */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_TAG_ORDER,
                             QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                             QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
      } /* endswitch */
    } /* endwhile */

    if ( ( (*ppTok)->sTokenid != QDPR_PAGEHEAD_ETOKEN ) &&
         ( usRC == QDPR_NO_ERROR ) )
    {
      /****************************************************************/
      /*    no end-pagehead tag has been found, so return an error    */
      /****************************************************************/
      QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                       QDPR_PAGEHEAD_TOKEN, QDPR_EMPTY_STRING, 0,
                       QDPR_PAGEHEAD_ETOKEN, QDPR_NO_QDPR_TAG,
                       &usRC, &(psctIDA->usSyntaxError) );
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      (*ppTok)++;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRReadPagehead */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadRepeatTag                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadRepeatTag( psctIDA, ppTok, pusNameID, pszNameBuf,          |
//|                            usNameBufLen, psctAttribs )                     |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads the attributes of the <repeat> tag and sets the       |
//|  corresponding values in pszNameBuf, pusNameID and psctAttribs.            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA;      // pointer to thread IDA              |
//|  PTOKENENTRY           *ppTok;       // pointer to token where the repeat  |
//|                                      // token starts                       |
//|  PSZ                   pszNameBuf;   // name buffer if NAME='xxx' is       |
//|                                      // specified                          |
//|  USHORT                usNameBufLen; // name buffer length (including '\0')|
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PUSHORT               pusNameID;    // name ID in the repeat tag          |
//|  PQDPR_ATTRIBS         psctAttribs;  // attribute structure to be filled   |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_SYNTAX_ERROR     - syntax error occurred, psctIDA->usSyntaxError     |
//|                          is set                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - ppTok must point to the repeat token                                    |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - ppTok is set to the token behind the <repeat> tag                       |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Get the var name and check if it is a valid one                           |
//|  IF var name is NAME=                                                      |
//|    Get the fieldname (QDPRRetrieveValue)                                   |
//+----------------------------------------------------------------------------+

USHORT QDPRReadRepeatTag(

  PQDPR_THREAD          psctIDA,      // pointer to thread IDA
  PTOKENENTRY           *ppTok,       // pointer to token where the repeat
                                      // token starts
  PUSHORT               pusNameID,    // name ID in the repeat tag
  PSZ                   pszNameBuf,   // name buffer if NAME='xxx' is
                                      // specified
  USHORT                usNameBufLen, // name buffer length (including '\0')
  PQDPR_ATTRIBS         psctAttribs ) // attribute structure to be filled

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode



  /********************************************************************/
  /*                  clear the attribute structure                   */
  /*                       and the name buffer                        */
  /********************************************************************/
  memset( psctAttribs, NULC, sizeof( QDPR_ATTRIBS ) );
  memset( pszNameBuf, NULC, usNameBufLen );

  /********************************************************************/
  /*                        get the next token                        */
  /********************************************************************/
  (*ppTok)++;

  /********************************************************************/
  /*                 check if a correct name is given                 */
  /********************************************************************/
  if ( ( (*ppTok)->sTokenid == QDPR_NAME_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_LEVEL_ENTRY_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_LEVEL_HOM_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_LEVEL_SENSE_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_LEVEL_TARGET_ATTR ) )
  {
    *pusNameID = (*ppTok)->sTokenid;

    /******************************************************************/
    /*      if it is a NAME= attribute get the fieldname from it      */
    /******************************************************************/
    if ( *pusNameID == QDPR_NAME_ATTR )
    {
      QDPRRetrieveValue( *ppTok, pszNameBuf, usNameBufLen );
    } /* endif */

    /******************************************************************/
    /*        check if the repeat tag has an end-tag character        */
    /******************************************************************/
    if ( !QDPREndTagInTag( (*ppTok)->pDataString, (*ppTok)->usLength,
                            FALSE ) )
    {
      /****************************************************************/
      /*  no end-tag character has been found, so check if maybe the  */
      /*     following situtation occurred "<repeat NAME=xxx  >"      */
      /*           in which the "  >" is an own TEXT token            */
      /****************************************************************/
      if ( (*ppTok+1)->sTokenid == TEXT_TOKEN )
      {
        if ( QDPREndTagInTag( (*ppTok+1)->pDataString, (*ppTok+1)->usLength,
                              TRUE ) )
        {
          (*ppTok)++;
        }
        else
        {
          QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                           QDPR_REPEAT_TOKEN, QDPR_END_TAG_STR,
                           (USHORT)(strlen( QDPR_END_TAG_STR )),
                           QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                           &usRC, &(psctIDA->usSyntaxError) );
        } /* endif */
      }
      else
      {
        QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                         QDPR_REPEAT_TOKEN, QDPR_END_TAG_STR,
                         (USHORT)(strlen( QDPR_END_TAG_STR )),
                         QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                         &usRC, &(psctIDA->usSyntaxError) );
      } /* endif */
    } /* endif */

    /******************************************************************/
    /*                         get next token                         */
    /******************************************************************/
    if ( usRC == QDPR_NO_ERROR )
    {
      (*ppTok)++;
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /*             no valid repeat name has been entered              */
    /******************************************************************/
    QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_REPEAT_NAME,
                     QDPR_REPEAT_TOKEN, (*ppTok)->pDataString,
                     (*ppTok)->usLength,
                     QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                     &usRC, &(psctIDA->usSyntaxError) );
  } /* endif */

  if ( usRC != QDPR_NO_ERROR )
  {
    memset( psctAttribs, NULC, sizeof( QDPR_ATTRIBS ) );
    memset( pszNameBuf, NULC, usNameBufLen );
    *pusNameID = QDPR_NO_QDPR_TAG;
  } /* endif */

  return( usRC );

} /* end of function QDPRReadRepeatTag */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadSetTag                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadSetTag( psctIDA, ppTok )                                   |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads the attributes of the <set> tag and sets the          |
//|  corresponding values in psctIDA to the value specified.                   |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA; // pointer to thread IDA                   |
//|  PTOKENENTRY           *ppTok;  // pointer to token where the set          |
//|                                 // token starts                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_SYNTAX_ERROR     - syntax error occurred, psctIDA->usSyntaxError     |
//|                          is set                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - ppTok must point to the set token                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - ppTok is set to the token behind the <set> tag                          |
//|  - psctIDA->usLineLength is updated if $LINE_LENGTH is specified           |
//|  - psctIDA->usPageLength is updated if $PAGE_LENGTH is specified           |
//|  - psctIDA->pszPageEject is updated if $PAGE_EJECT is specified            |
//|  - psctIDA->ulPageNumber is updated if $PAGE_NO is specified               |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//| usRC = QDPRReadSetTag( psctIDA, &pTok );                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF a valid system variable name is given                                  |
//|    IF a valid value for the system variable name is given                  |
//|      Set the corresponding value in the IDA                                |
//+----------------------------------------------------------------------------+

USHORT QDPRReadSetTag
(
  PQDPR_THREAD          psctIDA, // pointer to thread IDA
  PTOKENENTRY           *ppTok   // pointer to token where the set
)                                 // token starts

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usSysnameID;                       // sysname tag ID
  ULONG        ulValue;                           // converted value



  /********************************************************************/
  /*                          get next token                          */
  /********************************************************************/
  (*ppTok)++;

  /********************************************************************/
  /*         check if a correct system variable name is given         */
  /********************************************************************/
  if ( ( (*ppTok)->sTokenid == QDPR_SYSNAME_LINE_LENGTH_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_SYSNAME_PAGE_EJECT_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_SYSNAME_PAGE_LENGTH_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_SYSNAME_PAGE_NO_ATTR ) )
  {
    usSysnameID = (*ppTok)->sTokenid;

    (*ppTok)++;

    /******************************************************************/
    /*             check if the value attribute is given              */
    /******************************************************************/
    if ( (*ppTok)->sTokenid == QDPR_VALUE_ATTR )
    {
      QDPRRetrieveValue( *ppTok, psctIDA->szWorkBuffer, QDPR_MAX_STRING );

      switch ( usSysnameID )
      {
        case QDPR_SYSNAME_LINE_LENGTH_ATTR :
        case QDPR_SYSNAME_PAGE_LENGTH_ATTR :
        case QDPR_SYSNAME_PAGE_NO_ATTR :
          {
            /**********************************************************/
            /*       get the numeric value from the work buffer       */
            /**********************************************************/
            if ( (psctIDA->szWorkBuffer)[0] != NULC )
            {
              /********************************************************/
              /*      convert the value read to a numeric value       */
              /********************************************************/
              if ( ( ulValue = atol( psctIDA->szWorkBuffer ) ) != 0L )
              {
                switch ( usSysnameID )
                {
                  case QDPR_SYSNAME_LINE_LENGTH_ATTR :
                    {
                      if ( psctIDA->usLineLength == 0 )
                      {
                        /**********************************************/
                        /* check if maximum line length is overflown  */
                        /**********************************************/
                        if ( (USHORT)ulValue <= QDPR_MAX_LINE_LENGTH )
                        {
                          psctIDA->usLineLength = (USHORT)ulValue;
                        }
                        else
                        {
                          QDPRSyntaxError( psctIDA,
                                           QDPR_SYER_LINELENGTH_TO_LARGE,
                                           QDPR_SET_TOKEN,
                                           QDPR_EMPTY_STRING, 0,
                                           QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                           &usRC, &(psctIDA->usSyntaxError) );
                        } /* endif */
                      }
                      else
                      {
                        /**********************************************/
                        /*   line length has been specified before    */
                        /**********************************************/
                        QDPRSyntaxError( psctIDA, QDPR_SYER_MANY_SYSVAR,
                                         QDPR_SYSNAME_LINE_LENGTH_ATTR,
                                         QDPR_EMPTY_STRING, 0,
                                         QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                         &usRC, &(psctIDA->usSyntaxError) );
                      } /* endif */
                    }
                  break;
                  case QDPR_SYSNAME_PAGE_LENGTH_ATTR :
                    {
                      if ( psctIDA->usPageLength == 0 )
                      {
                        /**********************************************/
                        /* check if maximum line length is overflown  */
                        /**********************************************/
                        if ( (USHORT)ulValue <= QDPR_MAX_PAGE_LENGTH )
                        {
                          psctIDA->usPageLength = (USHORT)ulValue;
                        }
                        else
                        {
                          QDPRSyntaxError( psctIDA,
                                           QDPR_SYER_PAGELENGTH_TO_LARGE,
                                           QDPR_SET_TOKEN,
                                           QDPR_EMPTY_STRING, 0,
                                           QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                           &usRC, &(psctIDA->usSyntaxError) );
                        } /* endif */
                      }
                      else
                      {
                        /**********************************************/
                        /*   page length has been specified before    */
                        /**********************************************/
                        QDPRSyntaxError( psctIDA, QDPR_SYER_MANY_SYSVAR,
                                         QDPR_SYSNAME_PAGE_LENGTH_ATTR,
                                         QDPR_EMPTY_STRING, 0,
                                         QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                         &usRC, &(psctIDA->usSyntaxError) );
                      } /* endif */
                    }
                  break;
                  case QDPR_SYSNAME_PAGE_NO_ATTR :
                    {
                      if ( psctIDA->ulPageNumber == 0L )
                      {
                        psctIDA->ulPageNumber = ulValue;
                      }
                      else
                      {
                        /**********************************************/
                        /*   page number has been specified before    */
                        /**********************************************/
                        QDPRSyntaxError( psctIDA, QDPR_SYER_MANY_SYSVAR,
                                         QDPR_SYSNAME_PAGE_NO_ATTR,
                                         QDPR_EMPTY_STRING, 0,
                                         QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                         &usRC, &(psctIDA->usSyntaxError) );
                      } /* endif */
                    }
                  break;
                } /* endswitch */
              }
              else
              {
                /******************************************************/
                /*             it was not a numeric value             */
                /******************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_NO_NUM_VALUE,
                                 QDPR_SET_TOKEN,
                                 (*ppTok)->pDataString, (*ppTok)->usLength,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            }
            else
            {
              /********************************************************/
              /*             no value has been specified              */
              /********************************************************/
              QDPRSyntaxError( psctIDA, QDPR_SYER_NO_NUM_VALUE,
                               QDPR_SET_TOKEN,
                               (*ppTok)->pDataString, (*ppTok)->usLength,
                               QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_SYSNAME_PAGE_EJECT_ATTR :
          {
            if ( (psctIDA->szWorkBuffer)[0] != NULC )
            {
              if ( psctIDA->pszPageEject == NULL )
              {
                /******************************************************/
                /*     allocate storage for the page eject string     */
                /*    and copy the string returned from the value     */
                /******************************************************/
                if ( UtlAlloc( (PVOID *)&(psctIDA->pszPageEject), 0L,
                               (LONG)( strlen( psctIDA->szWorkBuffer ) + 1 ),
                               NOMSG ) )
                {
                  strcpy( psctIDA->pszPageEject, psctIDA->szWorkBuffer );
                }
                else
                {
                  usRC = QDPR_NO_MEMORY;
                } /* endif */
              }
              else
              {
                /******************************************************/
                /*  the page eject string was already defined before  */
                /******************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_MANY_SYSVAR,
                                 QDPR_SYSNAME_PAGE_EJECT_ATTR,
                                 QDPR_EMPTY_STRING, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */
          }
        break;
      } /* endswitch */

      if ( usRC == QDPR_NO_ERROR )
      {
        /**************************************************************/
        /*       check if the set tag has an end-tag character        */
        /**************************************************************/
        if ( !QDPREndTagInTag( (*ppTok)->pDataString, (*ppTok)->usLength,
                                FALSE ) )
        {
          /************************************************************/
          /*     no end-tag character has been found, so check if     */
          /*         maybe the following situtation occurred          */
          /*              "<set sysname=xxx value=xx  >               */
          /*         in which the "  >" is an own TEXT token          */
          /************************************************************/
          if ( (*ppTok+1)->sTokenid == TEXT_TOKEN )
          {
            if ( QDPREndTagInTag( (*ppTok+1)->pDataString,
                                  (*ppTok+1)->usLength, TRUE ) )
            {
              (*ppTok)++;
            }
            else
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                               QDPR_SET_TOKEN, QDPR_END_TAG_STR,
                               (USHORT)(strlen( QDPR_END_TAG_STR )),
                               QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
          else
          {
            QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                             QDPR_SET_TOKEN, QDPR_END_TAG_STR,
                             (USHORT)(strlen( QDPR_END_TAG_STR )),
                             QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          } /* endif */
        } /* endif */
      } /* endif */

      /****************************************************************/
      /*             move to token behind the value token             */
      /****************************************************************/
      if ( usRC == QDPR_NO_ERROR )
      {
        (*ppTok)++;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /*                value specification is missing                */
      /****************************************************************/
      QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VITAL_ATTRIBUTE,
                       QDPR_SET_TOKEN,
                       QDPR_EMPTY_STRING, 0,
                       QDPR_VALUE_ATTR, usSysnameID,
                       &usRC, &(psctIDA->usSyntaxError) );
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /*               no valid sysname has been entered                */
    /******************************************************************/
    QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_SYSNAME, QDPR_SET_TOKEN,
                     (*ppTok)->pDataString, (*ppTok)->usLength,
                     QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                     &usRC, &(psctIDA->usSyntaxError) );
  } /* endif */

  return( usRC );

} /* end of function QDPRReadSetTag */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadTrailer                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadTrailer( psctIDA, ppTok )                                  |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads the text and tags between the <dictback> and          |
//|  </dictback> tags into the trailer buffer.                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA; // pointer to thread IDA                   |
//|  PTOKENENTRY           *ppTok;  // pointer to token where the trailer      |
//|                                 // token starts                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_SYNTAX_ERROR     - syntax error occurred, psctIDA->usSyntaxError     |
//|                          is set                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the values for the trailer part in the IDA will be set up               |
//|  - if an error occurred psctIDA->usSyntaxError is set appropriately        |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Inititialize trailer buffer and trailer FCRT                              |
//|  WHILE end-trailer tag is not yet reached                                  |
//|    SWITCH on the tag found                                                 |
//|      CASE var tag                                                          |
//|        Read the var tag (QDPRReadVarTag)                                   |
//|        IF it is not a <var NAME=xxx> tag                                   |
//|          Put the var tag in the trailer FCRT (QDPRCreateFCRTElement)       |
//|      CASE set tag                                                          |
//|        Read the set tag (QDPRReadSetTag)                                   |
//|      CASE text                                                             |
//|        Put the text in the trailer buffer (QDPRAddToProcessBuffer)         |
//|      CASE comment tag                                                      |
//|        Read over the comment tag (QDPRReadOverComment)                     |
//|      CASE end-comment tag                                                  |
//|        RETURN a syntax error as the start-comment tag is missing           |
//|      DEFAULT                                                               |
//|        RETURN syntax error that tags are in wrong order                    |
//+----------------------------------------------------------------------------+

USHORT QDPRReadTrailer
(
  PQDPR_THREAD          psctIDA, // pointer to thread IDA
  PTOKENENTRY           *ppTok   // pointer to token where the trailer
)                                 // token starts

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usNameID;                          // name ID of var tag
  CHAR         chrReplaced;                       // replace character
  QDPR_ATTRIBS sctAttribs;                        // attributes for var tag



  /********************************************************************/
  /*         check if the trailer tag has an end-tag character        */
  /********************************************************************/
  if ( !QDPREndTagInTag( (*ppTok)->pDataString, (*ppTok)->usLength,
                          FALSE ) )
  {
    if ( (*ppTok+1)->sTokenid == TEXT_TOKEN )
    {
      if ( !QDPREndTagInTag( (*ppTok+1)->pDataString, (*ppTok+1)->usLength,
                             TRUE ) )
      {
        QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                         (*ppTok)->sTokenid, QDPR_END_TAG_STR,
                         (USHORT)(strlen( QDPR_END_TAG_STR )),
                         QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                         &usRC, &(psctIDA->usSyntaxError) );
      } /* endif */
    }
    else
    {
      QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                       (*ppTok)->sTokenid, QDPR_END_TAG_STR,
                       (USHORT)(strlen( QDPR_END_TAG_STR )),
                       QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                       &usRC, &(psctIDA->usSyntaxError) );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*                        get the next token                        */
  /********************************************************************/
  if ( usRC == QDPR_NO_ERROR )
  {
    (*ppTok)++;
  } /* endif */

  /********************************************************************/
  /*     check if the next token is already the end-trailer token     */
  /*                         if so do nothing                         */
  /********************************************************************/
  if ( ( (*ppTok)->sTokenid != QDPR_DICTBACK_ETOKEN ) &&
       ( usRC == QDPR_NO_ERROR ) )
  {
    /******************************************************************/
    /*            initialize the trailer buffers and FCRTs            */
    /******************************************************************/
    if ( UtlAlloc( (PVOID *)&(psctIDA->psctTrailerBuffer), 0L,
                   (LONG)( sizeof( QDPR_PROCESS_BUFFER ) +
                           sizeof( QDPR_FCRT ) ), NOMSG ) )
    {
      psctIDA->psctTrailerFCRT = (PQDPR_FCRT)( psctIDA->psctTrailerBuffer + 1 );
      psctIDA->psctCurTrailerFCRT = psctIDA->psctTrailerFCRT;

      psctIDA->pchrTrailerBuffer = psctIDA->psctTrailerBuffer->achrBuffer;
      psctIDA->psctCurTrailerExt = psctIDA->psctTrailerBuffer;
    }
    else
    {
      usRC = QDPR_NO_MEMORY;
    } /* endif */

    /******************************************************************/
    /*      check if a CRLF follows directly the <trailer> tag,       */
    /*                      if so get rid of it                       */
    /******************************************************************/
    if ( usRC == QDPR_NO_ERROR )
    {
      QDPRSuppressFirstCRLF( *ppTok );
    } /* endif */

    /******************************************************************/
    /*          now loop till the end-trailer tag is reached          */
    /******************************************************************/
    while ( ( (*ppTok)->sTokenid != QDPR_DICTBACK_ETOKEN ) &&
            ( (*ppTok)->sTokenid != ENDOFLIST ) &&
            ( usRC == QDPR_NO_ERROR ) )
    {
      switch ( (*ppTok)->sTokenid )
      {
        case QDPR_VAR_TOKEN :
          {
            /**********************************************************/
            /*                a var tag has been found                */
            /**********************************************************/
            usRC = QDPRReadVarTag( psctIDA, ppTok, &usNameID,
                                   psctIDA->szFieldname,
                                   DICTENTRYLENGTH + 1,
                                   &sctAttribs );
            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*   check if a NAME= var has been specified which is   */
              /*                     not allowed                      */
              /********************************************************/
              if ( usNameID == QDPR_NAME_ATTR )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_TAG_ORDER,
                                 QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*                create a FCRT element                 */
              /********************************************************/
              usRC = QDPRCreateFCRTElement( psctIDA,
                                            &(psctIDA->psctCurTrailerFCRT),
                                            psctIDA->szFieldname, usNameID,
                                            sctAttribs,
                                            psctIDA->pchrTrailerBuffer,
                                            &(psctIDA->usSyntaxError) );
              if ( ( usRC == QDPR_SYNTAX_ERROR ) &&
                   ( psctIDA->usSyntaxError == QDPR_SYER_WRONG_FIELDNAME ) )
              {
                QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_FIELDNAME,
                                 QDPR_VAR_TOKEN, psctIDA->szFieldname, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*   put the replacement string in the trailer buffer   */
              /********************************************************/
              psctIDA->pchrTrailerBuffer[0] = NULC;
              (psctIDA->pchrTrailerBuffer)++;
              usRC = QDPRAddToProcessBuffer(
                            &(psctIDA->psctCurTrailerExt),
                            &(psctIDA->pchrTrailerBuffer),
                            QDPR_REP_SYSVAR );
            } /* endif */
          }
        break;
        case QDPR_SET_TOKEN :
          {
            /**********************************************************/
            /*                a set tag has been found                */
            /**********************************************************/
            usRC = QDPRReadSetTag( psctIDA, ppTok );

            /**********************************************************/
            /*    check if a CRLF follows directly the <set> tag,     */
            /*                  if so get rid of it                   */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              QDPRSuppressFirstCRLF( *ppTok );
            } /* endif */
          }
        break;
        case TEXT_TOKEN :
          {
            /**********************************************************/
            /*            just simple text has been found             */
            /*    as (*ppTok)->pDataString doesn't end with a '\0'    */
            /*    put one delibertely at the end, but remember the    */
            /*  repleaced character and re-replace it after putting   */
            /*             the text to the process buffer             */
            /**********************************************************/
            chrReplaced = ((*ppTok)->pDataString)[(*ppTok)->usLength];
            ((*ppTok)->pDataString)[(*ppTok)->usLength] = NULC;

            usRC = QDPRAddToProcessBuffer( &(psctIDA->psctCurTrailerExt),
                                           &(psctIDA->pchrTrailerBuffer),
                                           (*ppTok)->pDataString );

            ((*ppTok)->pDataString)[(*ppTok)->usLength] = chrReplaced;

            (*ppTok)++;
          }
        break;
        case QDPR_COMMENT_TOKEN :
          {
            /**********************************************************/
            /*               comment tag has been found               */
            /**********************************************************/
            usRC = QDPRReadOverComment( ppTok );

            /**********************************************************/
            /*      if a syntax error occurred (i.e. the comment      */
            /*              tag was not properly closed)              */
            /**********************************************************/
            if ( usRC == QDPR_SYNTAX_ERROR )
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                               QDPR_COMMENT_TOKEN, QDPR_EMPTY_STRING, 0,
                               QDPR_COMMENT_ETOKEN, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_COMMENT_ETOKEN :
          {
            /**********************************************************/
            /*  end-comment tag has been found, i.e. a start-comment  */
            /*                     tag is missing                     */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_MISSING_START_TAG,
                             QDPR_COMMENT_ETOKEN, QDPR_EMPTY_STRING, 0,
                             QDPR_COMMENT_TOKEN, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
        default :
          {
            /**********************************************************/
            /*  another tag has been found, so it is in wrong order   */
            /**********************************************************/
            QDPRSyntaxError( psctIDA, QDPR_SYER_WRONG_TAG_ORDER,
                             QDPR_NO_QDPR_TAG, QDPR_EMPTY_STRING, 0,
                             QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          }
        break;
      } /* endswitch */
    } /* endwhile */

    if ( ( (*ppTok)->sTokenid != QDPR_DICTBACK_ETOKEN ) &&
         ( usRC == QDPR_NO_ERROR ) )
    {
      /****************************************************************/
      /*     no end-trailer tag has been found, so return an error    */
      /****************************************************************/
      QDPRSyntaxError( psctIDA, QDPR_SYER_TAG_NOT_CLOSED,
                       QDPR_DICTBACK_TOKEN, QDPR_EMPTY_STRING, 0,
                       QDPR_DICTBACK_ETOKEN, QDPR_NO_QDPR_TAG,
                       &usRC, &(psctIDA->usSyntaxError) );
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      /****************************************************************/
      /*    check if the end-trailer tag has an end-tag character     */
      /****************************************************************/
      if ( !QDPREndTagInTag( (*ppTok)->pDataString, (*ppTok)->usLength,
                              FALSE ) )
      {
        if ( (*ppTok+1)->sTokenid == TEXT_TOKEN )
        {
          if ( !QDPREndTagInTag( (*ppTok+1)->pDataString,
                                 (*ppTok+1)->usLength, TRUE ) )
          {
            QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                             (*ppTok)->sTokenid, QDPR_END_TAG_STR,
                             (USHORT)(strlen( QDPR_END_TAG_STR )),
                             QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          } /* endif */
        }
        else
        {
          QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                           (*ppTok)->sTokenid, QDPR_END_TAG_STR,
                           (USHORT)(strlen( QDPR_END_TAG_STR )),
                           QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                           &usRC, &(psctIDA->usSyntaxError) );
        } /* endif */
      } /* endif */

      if ( usRC == QDPR_NO_ERROR )
      {
        (*ppTok)++;
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRReadTrailer */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRReadVarTag                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRReadVarTag( psctIDA, ppTok, pusNameID, pszNameBuf,             |
//|                         usNameBufLen, psctAttribs )                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function reads the attributes of the <var> tag and sets the          |
//|  corresponding values in pszNameBuf, pusNameID and psctAttribs.            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA;      // pointer to thread IDA              |
//|  PTOKENENTRY           *ppTok;       // pointer to token where the var     |
//|                                      // token starts                       |
//|  PSZ                   pszNameBuf;   // name buffer if NAME='xxx' is       |
//|                                      // specified                          |
//|  USHORT                usNameBufLen; // name buffer length (including '\0')|
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PUSHORT               pusNameID;    // name ID in the var tag             |
//|  PQDPR_ATTRIBS         psctAttribs;  // attribute structure to be filled   |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_SYNTAX_ERROR     - syntax error occurred, psctIDA->usSyntaxError     |
//|                          is set                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - ppTok must point to the var token                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - ppTok is set to the token behind the <var> tag                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Get the var name and check if it is a valid one                           |
//|  IF var name is NAME=                                                      |
//|    Get the fieldname (QDPRRetrieveValue)                                   |
//|  WHILE there are attributes                                                |
//|    SWITCH on attribute token id                                            |
//|      For each attribute check whether it was defined already for this      |
//|        var tag, if no excluding attributes are already defined and         |
//|        if values are correct                                               |
//+----------------------------------------------------------------------------+

USHORT QDPRReadVarTag(

  PQDPR_THREAD          psctIDA,      // pointer to thread IDA
  PTOKENENTRY           *ppTok,       // pointer to token where the var
                                      // token starts
  PUSHORT               pusNameID,    // name ID in the var tag
  PSZ                   pszNameBuf,   // name buffer if NAME='xxx' is
                                      // specified
  USHORT                usNameBufLen, // name buffer length (including '\0')
  PQDPR_ATTRIBS         psctAttribs ) // attribute structure to be filled

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  BOOL         fStopLoop = FALSE;                 // stop loop ?



  /********************************************************************/
  /*                  clear the attribute structure                   */
  /*                       and the name buffer                        */
  /********************************************************************/
  memset( psctAttribs, NULC, sizeof( QDPR_ATTRIBS ) );
  memset( pszNameBuf, NULC, usNameBufLen );

  /********************************************************************/
  /*                        get the next token                        */
  /********************************************************************/
  (*ppTok)++;

  /********************************************************************/
  /*                 check if a correct name is given                 */
  /********************************************************************/
  if ( ( (*ppTok)->sTokenid == QDPR_NAME_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_SYSNAME_FILENAME_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_SYSNAME_DICTNAME_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_SYSNAME_DATE_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_SYSNAME_TIME_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_SYSNAME_PAGE_NO_ATTR ) ||
       ( (*ppTok)->sTokenid == QDPR_SYSNAME_PAGE_EJECT_ATTR ) )
  {
    *pusNameID = (*ppTok)->sTokenid;

    /******************************************************************/
    /*      if it is a NAME= attribute get the fieldname from it      */
    /******************************************************************/
    if ( *pusNameID == QDPR_NAME_ATTR )
    {
      QDPRRetrieveValue( *ppTok, pszNameBuf, usNameBufLen );
    } /* endif */

    /******************************************************************/
    /*                         get next token                         */
    /******************************************************************/
    (*ppTok)++;

    /******************************************************************/
    /*             now loop over the following attributes             */
    /******************************************************************/
    while ( ( !fStopLoop ) && ( usRC == QDPR_NO_ERROR ) )
    {
      switch ( (*ppTok)->sTokenid )
      {
        case QDPR_FORMAT_ATTR :
          {
            /**********************************************************/
            /*            format attribute has been found             */
            /*    it may only occurr together with $DATE or $TIME     */
            /**********************************************************/
            switch ( *pusNameID )
            {
              case QDPR_SYSNAME_DATE_ATTR :
                {
                  QDPRRetrieveValue( *ppTok, psctIDA->szWorkBuffer,
                                      QDPR_MAX_STRING );

                  if ( strnicmp( psctIDA->szWorkBuffer,
                                 QDPR_DATE_UK_FULL_FORMAT,
                                 QDPR_DATE_FULL_FORMAT_LEN ) == 0 )
                  {
                    psctAttribs->usFormatID = QDPR_FORMAT_DATE_UK_FULL;
                  }
                  else
                  {
                    if ( strnicmp( psctIDA->szWorkBuffer,
                                   QDPR_DATE_US_FULL_FORMAT,
                                   QDPR_DATE_FULL_FORMAT_LEN ) == 0 )
                    {
                      psctAttribs->usFormatID = QDPR_FORMAT_DATE_US_FULL;
                    }
                    else
                    {
                      if ( strnicmp( psctIDA->szWorkBuffer,
                                     QDPR_DATE_EU_FULL_FORMAT,
                                     QDPR_DATE_FULL_FORMAT_LEN ) == 0 )
                      {
                        psctAttribs->usFormatID = QDPR_FORMAT_DATE_EU_FULL;
                      }
                      else
                      {
                        if ( strnicmp( psctIDA->szWorkBuffer,
                                       QDPR_DATE_UK_FORMAT,
                                       QDPR_DATE_FORMAT_LEN ) == 0 )
                        {
                          psctAttribs->usFormatID = QDPR_FORMAT_DATE_UK;
                        }
                        else
                        {
                          if ( strnicmp( psctIDA->szWorkBuffer,
                                         QDPR_DATE_US_FORMAT,
                                         QDPR_DATE_FORMAT_LEN ) == 0 )
                          {
                            psctAttribs->usFormatID = QDPR_FORMAT_DATE_US;
                          }
                          else
                          {
                            if ( strnicmp( psctIDA->szWorkBuffer,
                                           QDPR_DATE_EU_FORMAT,
                                           QDPR_DATE_FORMAT_LEN ) == 0 )
                            {
                              psctAttribs->usFormatID = QDPR_FORMAT_DATE_EU;
                            }
                            else
                            {
                              QDPRSyntaxError( psctIDA,
                                               QDPR_SYER_NO_DATE_TIME_FORMAT,
                                               QDPR_SYSNAME_DATE_ATTR,
                                               (*ppTok)->pDataString,
                                               (*ppTok)->usLength,
                                               QDPR_NO_QDPR_TAG,
                                               QDPR_NO_QDPR_TAG,
                                               &usRC,
                                               &(psctIDA->usSyntaxError) );
                            } /* endif */
                          } /* endif */
                        } /* endif */
                      } /* endif */
                    } /* endif */
                  } /* endif */
                }
              break;
              case QDPR_SYSNAME_TIME_ATTR :
                {
                  QDPRRetrieveValue( *ppTok, psctIDA->szWorkBuffer,
                                     QDPR_MAX_STRING );

                  if ( strnicmp( psctIDA->szWorkBuffer,
                                 QDPR_TIME_PM_FORMAT,
                                 QDPR_TIME_PM_FORMAT_LEN ) == 0 )
                  {
                    psctAttribs->usFormatID = QDPR_FORMAT_TIME_PM;
                  }
                  else
                  {
                    if ( strnicmp( psctIDA->szWorkBuffer,
                                   QDPR_TIME_NORM_FORMAT,
                                   QDPR_TIME_NORM_FORMAT_LEN ) == 0 )
                    {
                      psctAttribs->usFormatID = QDPR_FORMAT_TIME_NORM;
                    }
                    else
                    {
                      if ( strnicmp( psctIDA->szWorkBuffer,
                                     QDPR_TIME_PM_NSEC_FORMAT,
                                     QDPR_TIME_PM_NSEC_FORMAT_LEN ) == 0 )
                      {
                        psctAttribs->usFormatID = QDPR_FORMAT_TIME_PM_NSEC;
                      }
                      else
                      {
                        if ( strnicmp( psctIDA->szWorkBuffer,
                                       QDPR_TIME_NORM_NSEC_FORMAT,
                                       QDPR_TIME_NORM_NSEC_FORMAT_LEN ) == 0 )
                        {
                          psctAttribs->usFormatID = QDPR_FORMAT_TIME_NORM_NSEC;
                        }
                        else
                        {
                          QDPRSyntaxError( psctIDA,
                                           QDPR_SYER_NO_DATE_TIME_FORMAT,
                                           QDPR_SYSNAME_TIME_ATTR,
                                           (*ppTok)->pDataString,
                                           (*ppTok)->usLength,
                                           QDPR_NO_QDPR_TAG,
                                           QDPR_NO_QDPR_TAG,
                                           &usRC, &(psctIDA->usSyntaxError) );
                        } /* endif */
                      } /* endif */
                    } /* endif */
                  } /* endif */
                }
              break;
              default :
                {
                  QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_ATTRIBUTE,
                                   QDPR_VAR_TOKEN, (*ppTok)->pDataString,
                                   (*ppTok)->usLength,
                                   *pusNameID, QDPR_NO_QDPR_TAG,
                                   &usRC, &(psctIDA->usSyntaxError) );
                }
              break;
            } /* endswitch */
          }
        break;
        case QDPR_FIRST_ON_PAGE_ATTR :
          {
            if ( ( *pusNameID != QDPR_SYSNAME_DATE_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_TIME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_DICTNAME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_FILENAME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_PAGE_NO_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_PAGE_EJECT_ATTR ) )
            {
              /********************************************************/
              /* check if the last on page attribute has been defined */
              /********************************************************/
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_LASTONPAGE ) !=
                   QDPR_ATTR_LASTONPAGE )
              {
                /******************************************************/
                /*      check if the first on page attribute has      */
                /*                already been defined                */
                /******************************************************/
                if ( ( psctAttribs->bitAttribs & QDPR_ATTR_FIRSTONPAGE ) !=
                     QDPR_ATTR_FIRSTONPAGE )
                {
                  psctAttribs->bitAttribs |= QDPR_ATTR_FIRSTONPAGE;
                }
                else
                {
                  /******************************************************/
                  /*             attribute already defined              */
                  /******************************************************/
                  QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_ATTRIBUTE_TWICE,
                                   QDPR_VAR_TOKEN,
                                   QDPR_EMPTY_STRING, 0,
                                   (*ppTok)->sTokenid, QDPR_NO_QDPR_TAG,
                                   &usRC, &(psctIDA->usSyntaxError) );
                } /* endif */
              }
              else
              {
                /********************************************************/
                /*   last on page attribute has already been defined    */
                /********************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_EXCL_ATTR_IN_SAME_TAG,
                                 QDPR_VAR_TOKEN, QDPR_EMPTY_STRING, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            }
            else
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_ATTRIBUTE,
                               QDPR_VAR_TOKEN, (*ppTok)->pDataString,
                               (*ppTok)->usLength,
                               *pusNameID, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_LAST_ON_PAGE_ATTR :
          {
            if ( ( *pusNameID != QDPR_SYSNAME_DATE_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_TIME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_DICTNAME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_FILENAME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_PAGE_NO_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_PAGE_EJECT_ATTR ) )
            {
              /********************************************************/
              /* check if the last on page attribute has been defined */
              /********************************************************/
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_LASTONPAGE ) !=
                   QDPR_ATTR_LASTONPAGE )
              {
                /******************************************************/
                /*      check if the first on page attribute has      */
                /*                already been defined                */
                /******************************************************/
                if ( ( psctAttribs->bitAttribs & QDPR_ATTR_FIRSTONPAGE ) !=
                     QDPR_ATTR_FIRSTONPAGE )
                {
                  psctAttribs->bitAttribs |= QDPR_ATTR_FIRSTONPAGE;
                }
                else
                {
                  /****************************************************/
                  /*     first on page attribute already defined      */
                  /****************************************************/
                  QDPRSyntaxError( psctIDA, QDPR_SYER_EXCL_ATTR_IN_SAME_TAG,
                                   QDPR_VAR_TOKEN, QDPR_EMPTY_STRING, 0,
                                   QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                   &usRC, &(psctIDA->usSyntaxError) );
                } /* endif */
              }
              else
              {
                /********************************************************/
                /*   last on page attribute has already been defined    */
                /********************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_ATTRIBUTE_TWICE,
                                 QDPR_VAR_TOKEN,
                                 QDPR_EMPTY_STRING, 0,
                                 (*ppTok)->sTokenid, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            }
            else
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_ATTRIBUTE,
                               QDPR_VAR_TOKEN, (*ppTok)->pDataString,
                               (*ppTok)->usLength,
                               *pusNameID, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_MIN_ATTR :
          {
            /**********************************************************/
            /*     check if the attribute has been defined before     */
            /**********************************************************/
            if ( psctAttribs->usMinChars == 0 )
            {
              QDPRRetrieveValue( *ppTok, psctIDA->szWorkBuffer,
                                 QDPR_MAX_STRING );

              if ( (psctIDA->szWorkBuffer)[0] != NULC )
              {
                if ( ( psctAttribs->usMinChars =
                      (USHORT)( atoi( psctIDA->szWorkBuffer )) ) != 0 )
                {
                  psctAttribs->bitAttribs |= QDPR_ATTR_MIN;
                }
                else
                {
                  /****************************************************/
                  /*              no numeric value found              */
                  /****************************************************/
                  QDPRSyntaxError( psctIDA, QDPR_SYER_NO_NUM_VALUE,
                                   QDPR_VAR_TOKEN,
                                   (*ppTok)->pDataString, (*ppTok)->usLength,
                                   QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                   &usRC, &(psctIDA->usSyntaxError) );
                } /* endif */
              }
              else
              {
                /******************************************************/
                /*               no numeric value found               */
                /******************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_NO_NUM_VALUE,
                                 QDPR_VAR_TOKEN,
                                 (*ppTok)->pDataString, (*ppTok)->usLength,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            }
            else
            {
              /********************************************************/
              /*        min attribute has already been defined        */
              /********************************************************/
              QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_ATTRIBUTE_TWICE,
                               QDPR_VAR_TOKEN,
                               QDPR_EMPTY_STRING, 0,
                               (*ppTok)->sTokenid, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_MAX_ATTR :
          {
            /**********************************************************/
            /*     check if the attribute has been defined before     */
            /**********************************************************/
            if ( psctAttribs->usMaxChars == 0 )
            {
              QDPRRetrieveValue( *ppTok, psctIDA->szWorkBuffer,
                                 QDPR_MAX_STRING );

              if ( (psctIDA->szWorkBuffer)[0] != NULC )
              {
                if ( ( psctAttribs->usMaxChars =
                       (USHORT)(atoi( psctIDA->szWorkBuffer )) ) != 0 )
                {
                  psctAttribs->bitAttribs |= QDPR_ATTR_MAX;
                }
                else
                {
                  /****************************************************/
                  /*              no numeric value found              */
                  /****************************************************/
                  QDPRSyntaxError( psctIDA, QDPR_SYER_NO_NUM_VALUE,
                                   QDPR_VAR_TOKEN,
                                   (*ppTok)->pDataString, (*ppTok)->usLength,
                                   QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                   &usRC, &(psctIDA->usSyntaxError) );
                } /* endif */
              }
              else
              {
                /******************************************************/
                /*               no numeric value found               */
                /******************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_NO_NUM_VALUE,
                                 QDPR_VAR_TOKEN,
                                 (*ppTok)->pDataString, (*ppTok)->usLength,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            }
            else
            {
              /********************************************************/
              /*        max attribute has already been defined        */
              /********************************************************/
              QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_ATTRIBUTE_TWICE,
                               QDPR_VAR_TOKEN,
                               QDPR_EMPTY_STRING, 0,
                               (*ppTok)->sTokenid, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_LEFT_ATTR :
          {
            /**********************************************************/
            /*  check if the right attribute has already been defined */
            /**********************************************************/
            if ( ( psctAttribs->bitAttribs & QDPR_ATTR_RIGHT ) !=
                 QDPR_ATTR_RIGHT )
            {
              /********************************************************/
              /*   check if the attribute has already been defined    */
              /********************************************************/
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_LEFT ) !=
                   QDPR_ATTR_LEFT )
              {
                psctAttribs->bitAttribs |= QDPR_ATTR_LEFT;
              }
              else
              {
                /******************************************************/
                /*      left attribute has already been defined       */
                /******************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_ATTRIBUTE_TWICE,
                                 QDPR_VAR_TOKEN,
                                 QDPR_EMPTY_STRING, 0,
                                 (*ppTok)->sTokenid, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            }
            else
            {
              /********************************************************/
              /*               right attribute defined                */
              /********************************************************/
              QDPRSyntaxError( psctIDA, QDPR_SYER_EXCL_ATTR_IN_SAME_TAG,
                               QDPR_VAR_TOKEN, QDPR_EMPTY_STRING, 0,
                               QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_RIGHT_ATTR :
          {
            /**********************************************************/
            /*    check if the attribute has already been defined     */
            /**********************************************************/
            if ( ( psctAttribs->bitAttribs & QDPR_ATTR_RIGHT ) !=
                 QDPR_ATTR_RIGHT )
            {
              /********************************************************/
              /* check if the left attribute has already been defined */
              /********************************************************/
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_LEFT ) !=
                   QDPR_ATTR_LEFT )
              {
                psctAttribs->bitAttribs |= QDPR_ATTR_RIGHT;
              }
              else
              {
                /******************************************************/
                /*              left attribute defined                */
                /******************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_EXCL_ATTR_IN_SAME_TAG,
                                 QDPR_VAR_TOKEN, QDPR_EMPTY_STRING, 0,
                                 QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            }
            else
            {
              /********************************************************/
              /*       right attribute has already been defined       */
              /********************************************************/
              QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_ATTRIBUTE_TWICE,
                               QDPR_VAR_TOKEN,
                               QDPR_EMPTY_STRING, 0,
                               (*ppTok)->sTokenid, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_NO_DISPLAY_ATTR :
          {
            if ( ( *pusNameID != QDPR_SYSNAME_DATE_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_TIME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_DICTNAME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_FILENAME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_PAGE_NO_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_PAGE_EJECT_ATTR ) )
            {
              /********************************************************/
              /*   check if the attribute has already been defined    */
              /********************************************************/
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_NODISPLAY ) !=
                   QDPR_ATTR_NODISPLAY )
              {
                psctAttribs->bitAttribs |= QDPR_ATTR_NODISPLAY;
              }
              else
              {
                /******************************************************/
                /*         attribute has already been defined         */
                /******************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_ATTRIBUTE_TWICE,
                                 QDPR_VAR_TOKEN,
                                 QDPR_EMPTY_STRING, 0,
                                 (*ppTok)->sTokenid, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            }
            else
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_ATTRIBUTE,
                               QDPR_VAR_TOKEN, (*ppTok)->pDataString,
                               (*ppTok)->usLength,
                               *pusNameID, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        case QDPR_SAME_ENTRY_AGAIN_ATTR :
          {
            if ( ( *pusNameID != QDPR_SYSNAME_DATE_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_TIME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_DICTNAME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_FILENAME_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_PAGE_NO_ATTR ) &&
                 ( *pusNameID != QDPR_SYSNAME_PAGE_EJECT_ATTR ) )
            {
              /********************************************************/
              /*   check if the attribute has already been defined    */
              /********************************************************/
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_SAMEENTRYAGAIN ) !=
                   QDPR_ATTR_SAMEENTRYAGAIN )
              {
                psctAttribs->bitAttribs |= QDPR_ATTR_SAMEENTRYAGAIN;
              }
              else
              {
                /******************************************************/
                /*         attribute has already been defined         */
                /******************************************************/
                QDPRSyntaxError( psctIDA, QDPR_SYER_SAME_ATTRIBUTE_TWICE,
                                 QDPR_VAR_TOKEN,
                                 QDPR_EMPTY_STRING, 0,
                                 (*ppTok)->sTokenid, QDPR_NO_QDPR_TAG,
                                 &usRC, &(psctIDA->usSyntaxError) );
              } /* endif */
            }
            else
            {
              QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_ATTRIBUTE,
                               QDPR_VAR_TOKEN, (*ppTok)->pDataString,
                               (*ppTok)->usLength,
                               *pusNameID, QDPR_NO_QDPR_TAG,
                               &usRC, &(psctIDA->usSyntaxError) );
            } /* endif */
          }
        break;
        default :
          {
            /**********************************************************/
            /* a non-attribute token has been read, so stop the loop  */
            /**********************************************************/
            fStopLoop = TRUE;
          }
        break;
      } /* endswitch */

      /****************************************************************/
      /*                        get next token                        */
      /****************************************************************/
      if ( ( usRC == QDPR_NO_ERROR ) && !fStopLoop )
      {
        (*ppTok)++;
      } /* endif */
    } /* endwhile */

    if ( usRC == QDPR_NO_ERROR )
    {
      /****************************************************************/
      /*        check if the var tag has an end-tag character         */
      /****************************************************************/
      if ( !QDPREndTagInTag( (*ppTok-1)->pDataString, (*ppTok-1)->usLength,
                              FALSE ) )
      {
        /**************************************************************/
        /*      no end-tag character has been found, so check if      */
        /*          maybe the following situtation occurred           */
        /*               "<set sysname=xxx value=xx  >                */
        /*          in which the "  >" is an own TEXT token           */
        /**************************************************************/
        if ( (*ppTok)->sTokenid == TEXT_TOKEN )
        {
          if ( QDPREndTagInTag( (*ppTok)->pDataString,
                                (*ppTok)->usLength, TRUE ) )
          {
            (*ppTok)++;
          }
          else
          {
            QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                             QDPR_VAR_TOKEN, QDPR_END_TAG_STR,
                             (USHORT)(strlen( QDPR_END_TAG_STR )),
                             QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                             &usRC, &(psctIDA->usSyntaxError) );
          } /* endif */
        }
        else
        {
          QDPRSyntaxError( psctIDA, QDPR_SYER_NO_END_TAG_CHAR,
                           QDPR_VAR_TOKEN, QDPR_END_TAG_STR,
                           (USHORT)(strlen( QDPR_END_TAG_STR )),
                           QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                           &usRC, &(psctIDA->usSyntaxError) );
        } /* endif */
      } /* endif */
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      /****************************************************************/
      /*  check if max and min attributes have been specified and if  */
      /*                          min <= max                          */
      /****************************************************************/
      if ( ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
             QDPR_ATTR_MAX ) &&
           ( ( psctAttribs->bitAttribs & QDPR_ATTR_MIN ) ==
             QDPR_ATTR_MIN ) )
      {
        if ( psctAttribs->usMinChars > psctAttribs->usMaxChars )
        {
          QDPRSyntaxError( psctIDA, QDPR_SYER_MIN_GREATER_MAX,
                           QDPR_VAR_TOKEN,
                           QDPR_EMPTY_STRING, 0,
                           QDPR_MAX_ATTR, QDPR_MIN_ATTR,
                           &usRC, &(psctIDA->usSyntaxError) );
        } /* endif */
      } /* endif */
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      /****************************************************************/
      /*     if a date or time sysname is given check if a format     */
      /*                    specification is given                    */
      /****************************************************************/
      if ( ( *pusNameID == QDPR_SYSNAME_DATE_ATTR ) ||
           ( *pusNameID == QDPR_SYSNAME_TIME_ATTR ) )
      {
        if ( psctAttribs->usFormatID == QDPR_NO_FORMAT )
        {
          QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VITAL_ATTRIBUTE,
                           QDPR_VAR_TOKEN, QDPR_EMPTY_STRING, 0,
                           QDPR_FORMAT_ATTR, *pusNameID,
                           &usRC, &(psctIDA->usSyntaxError) );
        } /* endif */
      } /* endif */
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /*               no valid sysname has been entered                */
    /******************************************************************/
    QDPRSyntaxError( psctIDA, QDPR_SYER_NO_VALID_SYSNAME, QDPR_VAR_TOKEN,
                     (*ppTok)->pDataString, (*ppTok)->usLength,
                     QDPR_NO_QDPR_TAG, QDPR_NO_QDPR_TAG,
                     &usRC, &(psctIDA->usSyntaxError) );
  } /* endif */

  if ( usRC != QDPR_NO_ERROR )
  {
    memset( psctAttribs, NULC, sizeof( QDPR_ATTRIBS ) );
    memset( pszNameBuf, NULC, usNameBufLen );
    *pusNameID = QDPR_NO_QDPR_TAG;
  } /* endif */

  return( usRC );

} /* end of function QDPRReadVarTag */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRRetrieveValue                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRRetrieveValue( pTok, pszBuffer, usBufLength )                  |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function retrieves the value following the VALUE=, the NAME=,        |
//|  the FORMAT=, the $MIN= or the $MAX= attribute.                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PTOKENENTRY     pTok;        // token pointer pointing to the start       |
//|                               // of the "XXX=" attribute                   |
//|  PSZ             pszBuffer;   // buffer to receive the value               |
//|  USHORT          usBufLength; // buffer length (including '\0')            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - pointer to the XXX= attribute in pTok                                   |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Run to the '='                                                            |
//|  Retrieve the characters following the '=' and copy them to the buffer     |
//+----------------------------------------------------------------------------+

USHORT QDPRRetrieveValue(

  PTOKENENTRY     pTok,        // token pointer pointing to the start
                               // of the "XXX=" attribute
  PSZ             pszBuffer,   // buffer to receive the value
  USHORT          usBufLength) // buffer length (including '\0')

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       i;                                 // a counter
  USHORT       j;                                 // a counter
  PSZ          pszRun;                            // run pointer
  BOOL         fQuoteUsed = FALSE;                // quote character used ?
  BOOL         fStopLoop = FALSE;                 // stop the loop ?
  BOOL         fValueStarts = FALSE;              // value starts ?



  /********************************************************************/
  /*                         clear the buffer                         */
  /********************************************************************/
  memset( pszBuffer, NULC, usBufLength );

  pszRun = pTok->pDataString;

  /********************************************************************/
  /*                          run to the '='                          */
  /********************************************************************/
  i = 0;

  while ( *pszRun != QDPR_EQUAL_CHAR )
  {
    pszRun++;
    i++;
  } /* endwhile */
  pszRun++;

  if ( *pszRun == QDPR_QUOTE_CHAR )
  {
    fQuoteUsed = TRUE;
    pszRun++;
  } /* endif */

  /********************************************************************/
  /*         now loop over the chacracters following the '='          */
  /********************************************************************/
  j = 0;
  while ( !fStopLoop )
  {
    /******************************************************************/
    /*       check if any of the loop stop conditions occurred        */
    /******************************************************************/
    if ( *pszRun == QDPR_END_TAG_CHAR )
    {
      fStopLoop = TRUE;
    }
    else
    {
      if ( ( *pszRun == QDPR_QUOTE_CHAR ) && fQuoteUsed )
      {
        fStopLoop = TRUE;
      }
      else
      {
        if ( ( *pszRun == QDPR_BLANK ) && !fQuoteUsed )
        {
          fStopLoop = TRUE;
        }
        else
        {
          if ( i >= pTok->usLength )
          {
            fStopLoop = TRUE;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /*        write the current character to the output buffer        */
    /******************************************************************/
    if ( !fStopLoop )
    {
      /****************************************************************/
      /*   check if there are blanks on the beginning of the value    */
      /*                    (only if a ' followed)                    */
      /*         if so, do not put them in the output buffer          */
      /****************************************************************/
      if ( *pszRun != QDPR_BLANK )
      {
        fValueStarts = TRUE;
      } /* endif */

      if ( fValueStarts )
      {
        if ( j < usBufLength - 1 )
        {
          pszBuffer[j] = *pszRun;
          j++;
        } /* endif */
      } /* endif */

      pszRun++;
      i++;
    } /* endif */
  } /* endwhile */

  return( usRC );

} /* end of function QDPRRetrieveValue */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRSuppressFirstCRLF                                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  VOID QDPRSuppressFirstCRLF( pTok )                                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function searches in pTok->pDataString for the first CRLF            |
//|  that appears after the start of the string and with only blanks           |
//|  preceeding.                                                               |
//|                                                                            |
//|  If such a CRLF is found, pTok->pDataString is set behind the CRLF         |
//|  and pTok->usLength reflects the new length.                               |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PTOKENENTRY     pTok;    // pointer to token in which first CRLF          |
//|                           // is to be suppressed                           |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: VOID                                                       |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - pTok must point to a TEXT token                                         |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//| - if a CRLF with only blanks preceeding is found pTok->pDataString         |
//|   is set behind the CRLF and pTok->usLength is set to the new              |
//|   length of the string                                                     |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  WHILE there are blanks                                                    |
//|    Run over the blanks                                                     |
//|  IF a CRLF has been found                                                  |
//|   Set pTok->pDataString to character behind the CRLF                       |
//|   Set pTok->usLength to new string length                                  |
//+----------------------------------------------------------------------------+

VOID QDPRSuppressFirstCRLF
(
  PTOKENENTRY     pTok    // pointer to token in which first CRLF
                           // is to be suppressed
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       i;                                 // a counter
  PSZ          pszRun;                            // run pointer



  /********************************************************************/
  /*         check if it is a text token from which the CRLF          */
  /*                       has to be suppressed                       */
  /*                        if not do nothing                         */
  /********************************************************************/
  if ( pTok->sTokenid == TEXT_TOKEN )
  {
    pszRun = pTok->pDataString;
    i = 0;

    /******************************************************************/
    /*              run to the first non-blank character              */
    /******************************************************************/
    while ( ( *pszRun == QDPR_BLANK ) && ( i < pTok->usLength ) )
    {
      pszRun++;
      i++;
    } /* endwhile */

    if ( i < pTok->usLength )
    {
      /****************************************************************/
      /*                check if a CRLF has been found                */
      /****************************************************************/
      if ( *pszRun == QDPR_CR )
      {
        pszRun++;
        i++;
      } /* endif */

      if ( *pszRun == QDPR_LF )
      {
        pszRun++;
        i++;

        /**************************************************************/
        /*   now set the new data pointer and the new token length    */
        /**************************************************************/
        pTok->pDataString = pszRun;
        pTok->usLength = pTok->usLength - i;
      } /* endif */
    } /* endif */
  } /* endif */

} /* end of function QDPRSuppressFirstCRLF */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRSyntaxError                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  VOID QDPRSyntaxError( psctIDA, usSyErrID, usTagID, pszString, usStringLen,|
//|                        usNameID, usAttrID, pusRC, pusSyntaxError )         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function creates the necessary informations for the syntax error.    |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD psctIDA;             // thread IDA                           |
//|  USHORT       usSyErrID;           // syntax error ID (e.g.                |
//|                                    // QDPR_SYER_TAG_NOT_CLOSED             |
//|  USHORT       usTagID;             // tag ID to be put in pszWorkBuffer    |
//|  PSZ          pszString;           // string to be put in one of the       |
//|                                    // work buffers                         |
//|  USHORT       usStringLen;         // length of pszString                  |
//|  USHORT       usNameID;            // tag ID to be put in one of the       |
//|                                    // work buffers                         |
//|  USHORT       usAttrID;            // tag ID to be put in one of the       |
//|                                    // work buffers                         |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PUSHORT      pusRC;               // returned returncode                  |
//|  PUSHORT      pusSyntaxError;      // returned syntax error code           |
//+----------------------------------------------------------------------------+
//|Returncode type: VOID                                                       |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - pusRC is either set to QDPR_SYNTAX_ERROR if everything went OK          |
//|    otherwise it is set to the returncode of the function that failed       |
//|    in QDPRSyntaxError                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  SWITCH on syntax error ID                                                 |
//|    Make the appropriate preparations for each syntax error ID              |
//+----------------------------------------------------------------------------+

VOID QDPRSyntaxError(


  PQDPR_THREAD psctIDA,             // thread IDA
  USHORT       usSyErrID,           // syntax error ID (e.g.
                                    // QDPR_SYER_TAG_NOT_CLOSED
  USHORT       usTagID,             // tag ID to be put in pszWorkBuffer
  PSZ          pszString,           // string to be put in one of the
                                    // work buffers
  USHORT       usStringLen,         // length of pszString
  USHORT       usNameID,            // tag ID to be put in one of the
                                    // work buffers
  USHORT       usAttrID,            // tag ID to be put in one of the
                                    // work buffers
  PUSHORT      pusRC,               // returned returncode
  PUSHORT      pusSyntaxError )     // returned syntax error code

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;                       // function returncode
  USHORT       usMinLen;                      // min of lengths




  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pusRC = QDPR_SYNTAX_ERROR;
  *pusSyntaxError = usSyErrID;

  (psctIDA->pReplAddr)[0] = psctIDA->psctInOutput->szFormatFile;

  switch ( usSyErrID )
  {
    case QDPR_SYER_WRONG_FIELDNAME :
      {
        usRC = QDPRMakeTagFromTagID( psctIDA->szWorkBuffer,
                                     sizeof( psctIDA->szWorkBuffer ),
                                     usTagID, FALSE );
        if ( usRC != QDPR_NO_ERROR )
        {
          *pusRC = usRC;
        }
        else
        {
          strncpy( psctIDA->szWorkBuffer3, pszString,
                   sizeof( psctIDA->szWorkBuffer3 ) );
        } /* endif */

        (psctIDA->pReplAddr)[2] = psctIDA->szWorkBuffer;
        (psctIDA->pReplAddr)[3] = psctIDA->psctInOutput->szDictName;
        (psctIDA->pReplAddr)[4] = psctIDA->szWorkBuffer3;
        psctIDA->usNoOfSyErrParms = 5;
      }
    break;

    case QDPR_SYER_NO_VALID_ATTRIBUTE :
    case QDPR_SYER_NO_VITAL_ATTRIBUTE :
    case QDPR_SYER_MIN_GREATER_MAX :
      {
        usRC = QDPRMakeTagFromTagID( psctIDA->szWorkBuffer,
                                     sizeof( psctIDA->szWorkBuffer ),
                                     usTagID, FALSE );
        if ( usRC != QDPR_NO_ERROR )
        {
          *pusRC = usRC;
        }
        else
        {
          if ( usAttrID != QDPR_NO_QDPR_TAG )
          {
            usRC = QDPRMakeTagFromTagID( psctIDA->szWorkBuffer3,
                                         sizeof( psctIDA->szWorkBuffer3 ),
                                         usAttrID, FALSE );
            if ( usRC != QDPR_NO_ERROR )
            {
              *pusRC = usRC;
            } /* endif */
          }
          else
          {
            usMinLen = min(usStringLen, sizeof( psctIDA->szWorkBuffer3 ));
            strncpy( psctIDA->szWorkBuffer3, pszString,
                     min( usStringLen, sizeof( psctIDA->szWorkBuffer3 ) ) );
            *(psctIDA->szWorkBuffer3 + usMinLen) = EOS;
          } /* endif */
        } /* endif */

        if ( usRC == QDPR_NO_ERROR )
        {
          usRC = QDPRMakeTagFromTagID( psctIDA->szWorkBuffer4,
                                       sizeof( psctIDA->szWorkBuffer4 ),
                                       usNameID, FALSE );
          if ( usRC != QDPR_NO_ERROR )
          {
            *pusRC = usRC;
          } /* endif */
        } /* endif */

        (psctIDA->pReplAddr)[2] = psctIDA->szWorkBuffer;
        (psctIDA->pReplAddr)[3] = psctIDA->szWorkBuffer3;
        (psctIDA->pReplAddr)[4] = psctIDA->szWorkBuffer4;
        psctIDA->usNoOfSyErrParms = 5;
      }
    break;

    case QDPR_SYER_TAG_NOT_CLOSED :
    case QDPR_SYER_MISSING_START_TAG :
    case QDPR_SYER_SAME_ATTRIBUTE_TWICE :
      {
        usRC = QDPRMakeTagFromTagID( psctIDA->szWorkBuffer,
                                     sizeof( psctIDA->szWorkBuffer ),
                                     usTagID, FALSE );
        if ( usRC != QDPR_NO_ERROR )
        {
          *pusRC = usRC;
        }
        else
        {
          usRC = QDPRMakeTagFromTagID( psctIDA->szWorkBuffer3,
                                       sizeof( psctIDA->szWorkBuffer3 ),
                                       usNameID, FALSE );
          if ( usRC != QDPR_NO_ERROR )
          {
            *pusRC = usRC;
          } /* endif */
        } /* endif */

        (psctIDA->pReplAddr)[2] = psctIDA->szWorkBuffer;
        (psctIDA->pReplAddr)[3] = psctIDA->szWorkBuffer3;
        psctIDA->usNoOfSyErrParms = 4;
      }
    break;

    case QDPR_SYER_NO_VALID_REPEAT_NAME :
    case QDPR_SYER_NO_NUM_VALUE :
    case QDPR_SYER_NO_VALID_SYSNAME :
    case QDPR_SYER_NO_DATE_TIME_FORMAT :
    case QDPR_SYER_NO_END_TAG_CHAR :
      {
        usRC = QDPRMakeTagFromTagID( psctIDA->szWorkBuffer,
                                     sizeof( psctIDA->szWorkBuffer ),
                                     usTagID, FALSE );
        if ( usRC != QDPR_NO_ERROR )
        {
          *pusRC = usRC;
        }
        else
        {
          usMinLen = min(usStringLen, sizeof( psctIDA->szWorkBuffer3 ));
          strncpy( psctIDA->szWorkBuffer3, pszString, usMinLen  );
          *(psctIDA->szWorkBuffer3 + usMinLen) = EOS;
        } /* endif */

        (psctIDA->pReplAddr)[2] = psctIDA->szWorkBuffer;
        (psctIDA->pReplAddr)[3] = psctIDA->szWorkBuffer3;
        psctIDA->usNoOfSyErrParms = 4;
      }
    break;

    case QDPR_SYER_SAME_TAG_TWICE :
    case QDPR_SYER_EXCL_ATTR_IN_SAME_TAG :
    case QDPR_SYER_MANY_SYSVAR :
    case QDPR_SYER_PAGELENGTH_TO_LARGE :
    case QDPR_SYER_LINELENGTH_TO_LARGE :
      {
        usRC = QDPRMakeTagFromTagID( psctIDA->szWorkBuffer,
                                     sizeof( psctIDA->szWorkBuffer ),
                                     usTagID, FALSE );
        if ( usRC != QDPR_NO_ERROR )
        {
          *pusRC = usRC;
        } /* endif */

        (psctIDA->pReplAddr)[2] = psctIDA->szWorkBuffer;
        psctIDA->usNoOfSyErrParms = 3;
      }
    break;

    case QDPR_SYER_NO_ENTRY_TAG_FOUND :
    case QDPR_SYER_TOO_LONG_PAGEHEADFOOT :
    case QDPR_SYER_TOO_LONG_PAGEHEAD :
      {
        psctIDA->usNoOfSyErrParms = 1;
      }
    break;

    default :
      {
        psctIDA->usNoOfSyErrParms = 2;
      }
    break;
  } /* endswitch */

} /* end of function QDPRSyntaxError */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDPRNumberOfLF                                           |
//+----------------------------------------------------------------------------+
//|Function call:     QDPRNumberOfLF(USHORT,PSZ)                               |
//+----------------------------------------------------------------------------+
//|Description:       checks whether pszstring contains a lf                   |
//+----------------------------------------------------------------------------+
//|Parameters:        USHORT  usLen        length of string to be tested       |
//|                   PSZ     pData        ptr to data string                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0       no LF contained                                  |
//|                   1       1 LF foundd                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+

USHORT QDPRNumberOfLF
(
  USHORT   usLen,                      // length to be checked
  PSZ      pData                       // ptr to data string
)
{
  USHORT   usI = 0;
  USHORT   usRc= 0;

/**********************************************************************/
/* loop while not end of string                                       */
/**********************************************************************/
  while ( usI < usLen )
  {
    /********************************************************************/
    /* if crlf follows add 1 to counter                                 */
    /********************************************************************/
    if ( (*pData == QDPR_CR) && (*(pData+1) == QDPR_LF ) )
    {
      usRc ++;
    } /* endif */
    pData ++;
    usI++;
  } /* endwhile */
  return(usRc);
} /* end of function QDPRNumberOfLF */
