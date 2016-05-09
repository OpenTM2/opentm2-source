//+----------------------------------------------------------------------------+
//|EQFQDPRP.C                                                                  |
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
//|  Functions for QDPR print process                                          |
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
//|    QDPREvaluateEndRepeatTag                                                |
//|    QDPREvaluateFieldTag                                                    |
//|    QDPREvaluateRepeatTag                                                   |
//|    QDPREvaluateSysVar                                                      |
//|    QDPRFormatStrToLineLength                                               |
//|    QDPRPrintEntry                                                          |
//|    QDPRPrintHeader                                                         |
//|    QDPRPrintPageEject                                                      |
//|    QDPRPrintPagefoot                                                       |
//|    QDPRPrintPagehead                                                       |
//|    QDPRPrintToPageBuffer                                                   |
//|    QDPRPrintTrailer                                                        |
//|    QDPRPrintUntilTag                                                       |
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
// $Revision: 1.2 $ ----------- 29 Jul 2002
// --RJ R7197: use system preference language for conversion
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.3 $ ----------- 18 Mar 2002
// --RJ: P013574: copy UTF16 data to ASCII prior to sprintf in EvaluateFieldTag
//
//
// $Revision: 1.2 $ ----------- 22 Oct 2001
// --RJ: unicode enabling of dict.print
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFQDPRP.CV_   1.1   24 Feb 1997 11:08:18   BUILD  $
 *
 * $Log:   J:\DATA\EQFQDPRP.CV_  $
 *
 *    Rev 1.1   24 Feb 1997 11:08:18   BUILD
 * - fixed PTM KAT0265: Print of dicts with context longer than 126 chars does
 *   not work
 *
 *    Rev 1.0   09 Jan 1996 09:13:32   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

/**********************************************************************/
/*                           include files                            */
/**********************************************************************/
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DICTPRINT        // dictionary print functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_FILT             // dictionary filter functions
#include <eqf.h>                  // General Translation Manager include file
#include <eqfldbi.h>
#include "EQFQDPRI.H"                  // internal header file for dictionary print
#include "EQFQDPR.ID"                  // IDs for dictionary print




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPREvaluateEndRepeatTag                                     |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPREvaluateEndRepeatTag( psctIDA, ppsctRepeatStack, psctEntry,    |
//|                                   psctCurTemplate, ppchrBuffer )           |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function evaluates the end-repeat tag.                               |
//|                                                                            |
//|  It checks if the data that is on top of the repeat-stack is the same      |
//|  as the data in the current template. If so the repeat loop is started     |
//|  again otherwise the top element of the repeat-stack is taken              |
//|  from the stack and the processing continues after the end-repeat tag.     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD           psctIDA;           // pointer to thread IDA        |
//|  PQDPR_REPAT_STACK      *ppsctRepeatStack; // pointer to top of            |
//|                                            // repeat stack                 |
//|  PQLDB_HTREE            psctEntry;         // pointer to tree handle       |
//|                                            // of entry tree                |
//|  PQLDB_HTREE            psctCurTemplate;   // pointer to tree handle of    |
//|                                            // current template             |
//|  PCHAR                  *ppchrBuffer;      // pointer to current           |
//|                                            // location in entry buffer     |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_READ_DICTIONARY  - error while reading dictionary                    |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - if the repeat loop has to be started again then ppchrBuffer is set      |
//|    behind the repeat tag                                                   |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Get next template from the entry and copy it into the                     |
//|    current template                                                        |
//|  IF next template exists                                                   |
//|    Compare the data in the top repeat-stack element and the                |
//|      field data referenced in the current template                         |
//|  IF data is not the same or next template doesn't exist                    |
//|    Set new repeat-stack pointer to psctPrev of top element                 |
//|    Delete top element                                                      |
//|    Set entry buffer pointer behind </repeat> tag                           |
//|  ELSE                                                                      |
//|    Set entry buffer pointer behind <repeat> tag                            |
//+----------------------------------------------------------------------------+

USHORT QDPREvaluateEndRepeatTag(

  PQDPR_THREAD           psctIDA,           // pointer to thread IDA
  PQDPR_REPEAT_STACK     *ppsctRepeatStack, // pointer to top of
                                            // repeat stack
  PVOID                  pvEntry,           // pointer to tree handle
                                            // of entry tree
  PVOID                  pvCurTemplate,     // pointer to tree handle of
                                            // current template
  PCHAR                  *ppchrBuffer )     // pointer to current
                                            // location in entry buffer

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  PQLDB_HTREE  psctEntry =                       // pointer to tree handle
                 (PQLDB_HTREE)pvEntry;           // of entry tree
  PQLDB_HTREE  psctCurTemplate =                 // pointer to tree handle of
                 (PQLDB_HTREE)pvCurTemplate;     // current template

  USHORT       usRC = QDPR_NO_ERROR;             // function returncode
  USHORT       usTempRC;                         // temp returncode
  USHORT       usLevel = QLDB_MAX_LEVELS;        // returned level number
  PSZ_W        *ppszData = NULL;                 // data arary pointer
  BOOL         fEqual = 0;                       // old and new data is
                                                  // equal ?
  PQDPR_REPEAT_STACK    psctTemp;                 // temp pointer to
                                                  // repeat stack element
  PQLDB_NODE            pOldNode;                 // temp pointer to old
                                                  // data node
  PQLDB_NODE            pNode;                    // temp pointer to
                                                  // data node



  /********************************************************************/
  /*      allocate storage for a temp array of pointers which is      */
  /*               needed for calling QLDBNextTemplate                */
  /********************************************************************/
  if ( UtlAlloc( (PVOID *)&ppszData, 0L,
                 (LONG)( psctIDA->usTotalFields * sizeof( PSZ_W ) ),
                 NOMSG ) )
  {
    /******************************************************************/
    /*           check if a new template as to be retrieved           */
    /******************************************************************/
    if ( !( psctIDA->fNotGetNextTemplate ) )
    {
      /****************************************************************/
      /*             get the next template from the entry             */
      /****************************************************************/
      usTempRC = QLDBNextTemplate( psctEntry, ppszData, &usLevel );

      if ( usTempRC == QLDB_NO_ERROR )
      {
        /**************************************************************/
        /*           indicate that a template was retrieved           */
        /**************************************************************/
        psctIDA->fNotGetNextTemplate = TRUE;

        if ( usLevel == QLDB_MAX_LEVELS )
        {
          /************************************************************/
          /*      copy the next template to the current template      */
          /************************************************************/
          usRC = QDPRCopyCurrentTemplates( psctEntry, psctCurTemplate );
        } /* endif */
      }
      else
      {
        usRC = QDPR_READ_DICTIONARY;
      } /* endif */
    } /* endif */

    if ( ( usRC == QDPR_NO_ERROR ) && ( usLevel == QLDB_MAX_LEVELS ) )
    {
      /****************************************************************/
      /*       now compare the data in the new current template       */
      /*                      with the old data                       */
      /****************************************************************/
      fEqual = FALSE;

      switch ( (*ppsctRepeatStack)->usRepeatLevelID )
      {
        case QDPR_NAME_ATTR :
          {
            if ( UTF16strcmp( *((*ppsctRepeatStack)->ppszFieldData),
                         (*ppsctRepeatStack)->pszOldFieldData ) == 0 )
            {
              fEqual = TRUE;
            } /* endif */
          }
        break;
        case QDPR_LEVEL_ENTRY_ATTR :
          {
            pOldNode = (*ppsctRepeatStack)->pOldTemplate->apCurLevelNode[0];
            pNode = psctCurTemplate->apCurLevelNode[0];
            fEqual = QDPRCompareNodes( pOldNode, pNode,
                                       psctCurTemplate->ausNoOfFields[0] );
          }
        break;
        case QDPR_LEVEL_HOM_ATTR :
          {
            pOldNode = (*ppsctRepeatStack)->pOldTemplate->apCurLevelNode[1];
            pNode = psctCurTemplate->apCurLevelNode[1];
            fEqual = QDPRCompareNodes( pOldNode, pNode,
                                       psctCurTemplate->ausNoOfFields[1] );
          }
        break;
        case QDPR_LEVEL_SENSE_ATTR :
          {
            pOldNode = (*ppsctRepeatStack)->pOldTemplate->apCurLevelNode[2];
            pNode = psctCurTemplate->apCurLevelNode[2];
            fEqual = QDPRCompareNodes( pOldNode, pNode,
                                       psctCurTemplate->ausNoOfFields[2] );
          }
        break;
        case QDPR_LEVEL_TARGET_ATTR :
          {
            pOldNode = (*ppsctRepeatStack)->pOldTemplate->apCurLevelNode[3];
            pNode = psctCurTemplate->apCurLevelNode[3];
            fEqual = QDPRCompareNodes( pOldNode, pNode,
                                       psctCurTemplate->ausNoOfFields[3] );
          }
        break;
      } /* endswitch */
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      if ( !fEqual || ( usLevel != QLDB_MAX_LEVELS ) )
      {
        /**************************************************************/
        /*     the data is not the same or the tree is at its end     */
        /* if so set a new repeat-stack pointer, pop the top element  */
        /*                   from the repeat-stack                    */
        /*            if an old template exists destroy it            */
        /**************************************************************/
        psctTemp = *ppsctRepeatStack;
        if ( psctTemp->pOldTemplate != NULL )
        {
          QLDBDestroyTree( (PVOID *)&(psctTemp->pOldTemplate) );
        } /* endif */
        *ppsctRepeatStack = psctTemp->psctPrev;
        UtlAlloc( (PVOID *) &psctTemp, 0L, 0L, NOMSG );

        /**************************************************************/
        /*    check if the tree is at its end, if so indicate that    */
        /*      in the next repeat loop a new template has to be      */
        /*      fetched (in order to leave the loop due to tree       */
        /*                     being at the end)                      */
        /*   otherwise (if the data is not the same) indicate that    */
        /*   in the next repeat loop only the checking of the data    */
        /*                       has to be done                       */
        /**************************************************************/
        if ( usLevel != QLDB_MAX_LEVELS )
        {
          psctIDA->fNotGetNextTemplate = FALSE;
        }
        else
        {
          psctIDA->fNotGetNextTemplate = TRUE;
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /*     the new and old data is still the same, so set the     */
        /*  entry buffer pointer behind the start of the repeat tag   */
        /*  and indicate that the next time a new template has to be  */
        /*                         retrieved                          */
        /**************************************************************/
        *ppchrBuffer = (*ppsctRepeatStack)->pchrRepeatStart + 2;

        psctIDA->fNotGetNextTemplate = FALSE;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /*                 deallocate pointer array again                 */
    /******************************************************************/
    UtlAlloc( (PVOID *)&ppszData, 0L, 0L, NOMSG );
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  return( usRC );

} /* end of function QDPREvaluateEndRepeatTag */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPREvaluateFieldTag                                         |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPREvaluateFieldTag( psctIDA, psctCurFCRT, usFCRTElement,         |
//|                               psctCurBufExt, pusLines )                    |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD           psctIDA;       // pointer to thread IDA            |
//|  PQDPR_FCRT             psctCurFCRT;   // pointer to current FCRT extension|
//|  USHORT                 usFCRTElement; // element number in psctCurFCRT    |
//|  PQDPR_FORMAT_BUFFERS   psctCurBufExt; // pointer to current format        |
//|                                        // buffer extension used for        |
//|                                        // writing the evaluated system     |
//|                                        // variable to                      |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PUSHORT                pusLines;      // no of lines the system variable  |
//|                                        // takes up in the pagebuffer       |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
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
//|  Calculate size needed for a buffer into which the data from the           |
//|    field is formatted                                                      |
//|  IF same entry is needed                                                   |
//|    Reset the tree positions in the entry tree (QLDBResetTreePositions)     |
//|  According to the attributes set format the data from the field            |
//+----------------------------------------------------------------------------+

USHORT QDPREvaluateFieldTag(


  PQDPR_THREAD           psctIDA,       // pointer to thread IDA
  PQDPR_FCRT             psctCurFCRT,   // pointer to current FCRT extension
  USHORT                 usFCRTElement, // element number in psctCurFCRT
  PQDPR_FORMAT_BUFFERS   psctCurBufExt, // pointer to current format
                                        // buffer extension used for
                                        // writing the evaluated system
                                        // variable to
  PUSHORT                pusLines )     // no of lines the system variable
                                        // takes up in the pagebuffer

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  ULONG                 ulSize;                   // allocated size
  PSZ                   pszBuffer = NULL;         // temporary buffer area
  PSZ_W                 pszData;                  // pointer to field data
  PQDPR_ATTRIBS         psctAttribs;              // pointer to attribute
                                                  // structure of FCRT element
  PSZ                   pszAsciiData = NULL;      // to convert UnicodepszData



  /********************************************************************/
  /*        make the pointer to the attribute structure of the        */
  /*                       current FCRT element                       */
  /********************************************************************/
  psctAttribs = &(psctCurFCRT->asctFCRTElements[usFCRTElement].sctAttribs);

  /********************************************************************/
  /*                make the pointer to the field data                */
  /********************************************************************/
  pszData = *(psctCurFCRT->asctFCRTElements[usFCRTElement].ppszFieldData);

  if ( pszData != NULL )
  {
    /******************************************************************/
    /*             calculate size needed for temp buffer              */
    /******************************************************************/
    ulSize = (LONG)max( psctAttribs->usMaxChars,
                        max( UTF16strlenBYTE( pszData ),
                             psctAttribs->usMinChars ) ) + 1* sizeof(CHAR_W);
    if (!UtlAlloc( (PVOID *) &pszAsciiData, 0L, max( ulSize, (LONG)MIN_ALLOC ), NOMSG ))
    {
      usRC = QDPR_NO_MEMORY;
    }

    /******************************************************************/
    /*              allocate temporary storage for field              */
    /******************************************************************/
    if (!usRC &&
         UtlAlloc( (PVOID *) &pszBuffer, 0L, max( ulSize, (LONG)MIN_ALLOC ), NOMSG ) )
    {
      Unicode2ASCII( pszData, pszAsciiData, 0L );
      /****************************************************************/
      /*             check which attributes have been set             */
      /****************************************************************/
      if ( ( psctAttribs->bitAttribs & QDPR_ATTR_SAMEENTRYAGAIN ) ==
           QDPR_ATTR_SAMEENTRYAGAIN )
      {
        /**************************************************************/
        /*   same entry is needed, so reset the tree positions and    */
        /*            copy into the corresponding template            */
        /**************************************************************/
        if ( QLDBResetTreePositions( psctIDA->psctEntry ) ==
             QLDB_NO_ERROR )
        {
          if ( ( psctAttribs->bitAttribs & QDPR_ATTR_LASTONPAGE ) ==
               QDPR_ATTR_LASTONPAGE )
          {
            usRC = QDPRCopyCurrentTemplates( psctIDA->psctEntry,
                                  psctIDA->psctLastPageTemplate );
          }
          else
          {
            if ( ( psctAttribs->bitAttribs & QDPR_ATTR_FIRSTONPAGE ) ==
                 QDPR_ATTR_FIRSTONPAGE )
            {
              usRC = QDPRCopyCurrentTemplates( psctIDA->psctEntry,
                                    psctIDA->psctFirstPageTemplate );
            }
            else
            {
              usRC = QDPRCopyCurrentTemplates( psctIDA->psctEntry,
                                    psctIDA->psctCurrentTemplate );
            } /* endif */
          } /* endif */
        }
        else
        {
          usRC = QDPR_READ_DICTIONARY;
        } /* endif */
      } /* endif */

      if ( usRC == QDPR_NO_ERROR )
      {
        /**************************************************************/
        /* now check which of the formatting attributes have been set */
        /**************************************************************/
        if ( ( psctAttribs->bitAttribs & QDPR_ATTR_NODISPLAY ) !=
               QDPR_ATTR_NODISPLAY )
        {
          if ( ( psctAttribs->bitAttribs & QDPR_ATTR_RIGHT ) ==
               QDPR_ATTR_RIGHT )
          {
            if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MIN ) ==
                 QDPR_ATTR_MIN )
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              { // convert pszData to ASCIIdata prior to copying into Buffer
                sprintf( pszBuffer, "%*.*s", psctAttribs->usMinChars,
                         psctAttribs->usMaxChars, pszAsciiData );
              }
              else
              {
                sprintf( pszBuffer, "%*s", psctAttribs->usMinChars,
                         pszAsciiData );
              } /* endif */
            }
            else
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%.*s", psctAttribs->usMaxChars,
                         pszAsciiData );
              }
              else
              {
                sprintf( pszBuffer, "%s", pszAsciiData );
              } /* endif */
            } /* endif */
          }
          else
          {
            if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MIN ) ==
                 QDPR_ATTR_MIN )
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%-*.*s", psctAttribs->usMinChars,
                         psctAttribs->usMaxChars, pszAsciiData );
              }
              else
              {
                sprintf( pszBuffer, "%-*s", psctAttribs->usMinChars,
                         pszAsciiData );
              } /* endif */
            }
            else
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%.*s", psctAttribs->usMaxChars,
                         pszAsciiData );
              }
              else
              {
                sprintf( pszBuffer, "%s", pszAsciiData );
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    }
    else
    {
      usRC = QDPR_NO_MEMORY;
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      /****************************************************************/
      /*         format the string to fit to the line length          */
      /****************************************************************/
      usRC = QDPRFormatStrToLineLength( psctIDA, psctIDA->psctFormatIDA,
                                        pszBuffer, psctCurBufExt,
                                        pusLines );

      /******************************************************************/
      /*          deallocate the buffer temporarily allocated           */
      /******************************************************************/
      UtlAlloc( (PVOID *) &pszBuffer, 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *) &pszAsciiData, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPREvaluateFieldTag */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPREvaluateRepeatTag                                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPREvaluateRepeatTag( psctIDA, ppsctRepeatStack, psctCurFCRT,     |
//|                                usFCRTElement )                             |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function evaluates the found repeat tag. It creates a new            |
//|  repeat-stack element and allocates storage for an area to store the       |
//|  data from the current field.                                              |
//|  Then it fills the created element with the appropriate data and           |
//|  pushes the element on the stack.                                          |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD           psctIDA;           // thread IDA                   |
//|  PQDPR_REPAT_STACK      *ppsctRepeatStack; // pointer to top of            |
//|                                            // repeat stack                 |
//|  PQDPR_FCRT             psctCurFCRT;       // pointer to current           |
//|                                            // FCRT extension               |
//|  USHORT                 usFCRTElement;     // element number in            |
//|                                            // psctCurFCRT                  |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//|  QDPR_READ_DICTIONARY  - error reading the dictionary entry                |
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
//|  Create repeat-stack element                                               |
//|  Store input buffer pointer in the created repeat-stack element            |
//|  IF repeat tag is a NAME= repeat tag                                       |
//|    Store data from the field referenced in the field cross-reference       |
//|      table to the created repeat-stack element                             |
//|    Store pointer to the field in the created repeat-stack element          |
//|  ELSE                                                                      |
//|    Create old data template (QLDBCreateTree)                               |
//|    Copy the data from the current template to the old data template        |
//|      (QDPRCopyCurrentTemplates)                                            |
//|  Set psctPrev of created element to current repeat-stack pointer           |
//|  Set new repeat-stack pointer to pointer of created element                |
//+----------------------------------------------------------------------------+

USHORT QDPREvaluateRepeatTag(

  PQDPR_THREAD           psctIDA,           // thread IDA
  PQDPR_REPEAT_STACK     *ppsctRepeatStack, // pointer to top of
                                            // repeat stack
  PQDPR_FCRT             psctCurFCRT,       // pointer to current
                                            // FCRT extension
  USHORT                 usFCRTElement )    // element number in
                                            // psctCurFCRT

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  USHORT                i;                        // an index
  ULONG                 ulAllocSize;              // allocate storage size
  PSZ_W                 *ppszData;                  // pointer to field data ptr
  PSZ_W                 *papszData = NULL;        // data arary pointer
  PQDPR_REPEAT_STACK    psctTemp;                 // temp pointer to
                                                  // repeat stack element



  /********************************************************************/
  /*                make the pointer to the field data                */
  /********************************************************************/
  ppszData = psctCurFCRT->asctFCRTElements[usFCRTElement].ppszFieldData;

  ulAllocSize = (ULONG)sizeof( QDPR_REPEAT_STACK );
  if ( *ppszData != NULL )
  {
    ulAllocSize += (ULONG)( UTF16strlenBYTE( *ppszData ) + 1 * sizeof(CHAR_W));
  } /* endif */

  /********************************************************************/
  /*               allocate a new repeat stack element                */
  /*            and the buffer area for the old field data            */
  /********************************************************************/
  if ( UtlAlloc( (PVOID *) &psctTemp, 0L, ulAllocSize, NOMSG ) )
  {
    psctTemp->pszOldFieldData = (PSZ_W)( psctTemp + 1 );
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*        set pointer to start location of the repeat tag         */
    /*                 (on the location with the \0r)                 */
    /******************************************************************/
    psctTemp->pchrRepeatStart =
        psctCurFCRT->asctFCRTElements[usFCRTElement].pchrFieldStart;

    /******************************************************************/
    /*                    set tag ID of repeat tag                    */
    /******************************************************************/
    psctTemp->usRepeatLevelID =
      psctCurFCRT->asctFCRTElements[usFCRTElement].usTagID;

    /******************************************************************/
    /*      check if a NAME= or LEVEL= repeat tag has been found      */
    /******************************************************************/
    switch ( psctCurFCRT->asctFCRTElements[usFCRTElement].usTagID )
    {
      case QDPR_NAME_ATTR :
        {
          if ( *ppszData != NULL )
          {
            /**********************************************************/
            /*        save the current data as old field data         */
            /**********************************************************/
            UTF16strcpy( psctTemp->pszOldFieldData, *ppszData );

            /**********************************************************/
            /*    set pointer to data area in the current template    */
            /**********************************************************/
            psctTemp->ppszFieldData = ppszData;
          }
          else
          {
            usRC = QDPR_PROGRAM_ERROR;
          } /* endif */
        }
      break;
      case QDPR_LEVEL_ENTRY_ATTR :
      case QDPR_LEVEL_HOM_ATTR :
      case QDPR_LEVEL_SENSE_ATTR :
      case QDPR_LEVEL_TARGET_ATTR :
        {
          /************************************************************/
          /*             allocate temporarily storage for             */
          /*                creating the old template                 */
          /************************************************************/
          if ( UtlAlloc( (PVOID *)&papszData, 0L, (LONG)( psctIDA->usTotalFields *
                                                ( sizeof( PSZ_W ) + 2 ) ),
                         NOMSG ) )
          {
            papszData[0] = (PSZ_W)( papszData +
                                 ( psctIDA->usTotalFields * sizeof( PSZ_W ) ) );
            for ( i = 1; ( i < psctIDA->usTotalFields ) &&
                         ( usRC == QDPR_NO_ERROR ); i++ )
            {// for DEBUG: is 2 correct in Uncode?
              papszData[i] = (PSZ_W)( papszData[i-1] + 2 );
            } /* endfor */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*               create the old template                */
              /********************************************************/
              if ( QLDBCreateTree( psctIDA->ausNoOfFields, papszData,
                      (PVOID *)&(psctTemp->pOldTemplate) ) != QLDB_NO_ERROR )

              {
                usRC = QDPR_READ_DICTIONARY;
              } /* endif */
            } /* endif */

            /**********************************************************/
            /*      deallocate the temporarily allocated storage      */
            /**********************************************************/
            if ( papszData != NULL )
            {
              UtlAlloc( (PVOID *)&papszData, 0L, 0L, NOMSG );
            } /* endif */

            /**********************************************************/
            /*     copy the current template to the old template      */
            /**********************************************************/
            if ( usRC == QDPR_NO_ERROR )
            {
              usRC = QDPRCopyCurrentTemplates( psctIDA->psctCurrentTemplate,
                                               psctTemp->pOldTemplate );
            } /* endif */
          }
          else
          {
            usRC = QDPR_NO_MEMORY;
          } /* endif */
        }
      break;
    } /* endswitch */
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*        push the new created element on the repeat stack        */
    /******************************************************************/
    psctTemp->psctPrev = *ppsctRepeatStack;

    *ppsctRepeatStack = psctTemp;
  } /* endif */

  return( usRC );

} /* end of function QDPREvaluateRepeatTag */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPREvaluateSysVar                                           |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPREvaluateSysVar( psctIDA, usTag, psctCurFCRT, usFCRTElement,    |
//|                             psctCurBufExt, pusLines )                      |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function evaluates a found system variable.                          |
//|                                                                            |
//|  According to the tag ID of the system variable it performs different      |
//|  actions needed for the different system variables (e.g. for               |
//|  <$TIME> get the current time).                                            |
//|                                                                            |
//|  After the value for the system variable has been determined the           |
//|  string is formatted to fit into the page buffer.                          |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD           psctIDA;       // pointer to thread IDA            |
//|  USHORT                 usTag;         // system variable ID               |
//|  PQDPR_FCRT             psctCurFCRT;   // pointer to current FCRT extension|
//|  USHORT                 usFCRTElement; // element number in psctCurFCRT    |
//|  PQDPR_FORMAT_BUFFERS   psctCurBufExt; // pointer to current format        |
//|                                        // buffer extension used for        |
//|                                        // writing the evaluated system     |
//|                                        // variable to                      |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PUSHORT                pusLines;      // no of lines the system variable  |
//|                                        // takes up in the pagebuffer       |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
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
//|  Allocate storage for temp buffer                                          |
//|  SWITCH on system variable                                                 |
//|    CASE $FILENAME                                                          |
//|    CASE $DICTNAME                                                          |
//|      IF system variable is $FILENAME                                       |
//|        Put filename in temp buffer                                         |
//|      ELSE                                                                  |
//|        Put dictionary name in temp buffer                                  |
//|      Format the string in the temp buffer according to the attributes      |
//|    CASE $DATE                                                              |
//|    CASE $TIME                                                              |
//|      Get current date and time                                             |
//|      Put the date or time in the temp buffer according to the format       |
//|        chosen                                                              |
//|    CASE $PAGE_NO                                                           |
//|      Put ulPageNo in temp buffer                                           |
//|      Format the string in the temp buffer according to the attributes      |
//|    CASE $PAGE_EJECT                                                        |
//|      Add so many CRLFs that the page is filled                             |
//|  Format string in temp buffer to fit to page buffer line length            |
//|    (QDPRFormatStrToLineLength)                                             |
//|  Deallocate the storage for the temp buffer                                |
//+----------------------------------------------------------------------------+

USHORT QDPREvaluateSysVar(


  PQDPR_THREAD           psctIDA,       // pointer to thread IDA
  USHORT                 usTag,         // system variable ID
  PQDPR_FCRT             psctCurFCRT,   // pointer to current FCRT extension
  USHORT                 usFCRTElement, // element number in psctCurFCRT
  PQDPR_FORMAT_BUFFERS   psctCurBufExt, // pointer to current format
                                        // buffer extension used for
                                        // writing the evaluated system
                                        // variable to
  PUSHORT                pusLines )     // no of lines the system variable
                                        // takes up in the pagebuffer

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  USHORT                i;                        // an index
  USHORT                usLines;                  // no of lines the system
                                                  // variables uses
  USHORT                usLinesFree;              // lines free on page
  PSZ                   pszBuffer = NULL;         // temporary buffer area
  PQDPR_ATTRIBS         psctAttribs;              // pointer to attribute
                                                  // structure of FCRT element
  DATETIME              sctDateTime;              // date, time structure
  USHORT                usCurPgFootLen;           // curlength of pgfoot


  /********************************************************************/
  /*        make the pointer to the attribute structure of the        */
  /*                       current FCRT element                       */
  /********************************************************************/
  psctAttribs = &(psctCurFCRT->asctFCRTElements[usFCRTElement].sctAttribs);

  memset( psctIDA->szWorkBuffer, NULC, QDPR_MAX_STRING );

  *pusLines = 0;

  /********************************************************************/
  /*            allocate storage for temporary buffer area            */
  /********************************************************************/
  if ( !( UtlAlloc( (PVOID *) &pszBuffer, 0L, (LONG)( QDPR_MAX_PROCESS_BUFFER ),
                    NOMSG ) ) )
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    switch ( usTag )
    {
      case QDPR_SYSNAME_FILENAME_ATTR :
      case QDPR_SYSNAME_DICTNAME_ATTR :
        {
          if ( usTag == QDPR_SYSNAME_FILENAME_ATTR )
          {
            if ( psctIDA->psctInOutput->fPrinterDest )
            {
              sprintf( psctIDA->szWorkBuffer, "%s",
                       psctIDA->psctInOutput->szPrintDest );
            }
            else
            {
              sprintf( psctIDA->szWorkBuffer, "%s",
                       UtlGetFnameFromPath(
                             psctIDA->psctInOutput->szPrintDest ) );
            } /* endif */
          }
          else
          {
            sprintf( psctIDA->szWorkBuffer, "%s",
                     psctIDA->psctInOutput->szDictName );
          } /* endif */

          if ( ( psctAttribs->bitAttribs & QDPR_ATTR_RIGHT ) ==
               QDPR_ATTR_RIGHT )
          {
            if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MIN ) ==
                 QDPR_ATTR_MIN )
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%*.*s", psctAttribs->usMinChars,
                         psctAttribs->usMaxChars, psctIDA->szWorkBuffer );
              }
              else
              {
                sprintf( pszBuffer, "%*s", psctAttribs->usMinChars,
                         psctIDA->szWorkBuffer );
              } /* endif */
            }
            else
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%.*s", psctAttribs->usMaxChars,
                         psctIDA->szWorkBuffer );
              }
              else
              {
                sprintf( pszBuffer, "%s", psctIDA->szWorkBuffer );
              } /* endif */
            } /* endif */
          }
          else
          {
            if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MIN ) ==
                 QDPR_ATTR_MIN )
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%-*.*s", psctAttribs->usMinChars,
                         psctAttribs->usMaxChars, psctIDA->szWorkBuffer );
              }
              else
              {
                sprintf( pszBuffer, "%-*s", psctAttribs->usMinChars,
                         psctIDA->szWorkBuffer );
              } /* endif */
            }
            else
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%.*s", psctAttribs->usMaxChars,
                         psctIDA->szWorkBuffer );
              }
              else
              {
                sprintf( pszBuffer, "%s", psctIDA->szWorkBuffer );
              } /* endif */
            } /* endif */
          } /* endif */
        }
      break;
      case QDPR_SYSNAME_DATE_ATTR :
      case QDPR_SYSNAME_TIME_ATTR :
        {
          /************************************************************/
          /*                      get date, time                      */
          /************************************************************/
          DosGetDateTime( &sctDateTime );

          switch ( psctAttribs->usFormatID )
          {
            case QDPR_FORMAT_DATE_UK_FULL :
              {
                sprintf( pszBuffer, "%d/%d/%d", sctDateTime.month,
                         sctDateTime.day, sctDateTime.year );
              }
            break;
            case QDPR_FORMAT_DATE_UK :
              {
                sctDateTime.year -= ( ( sctDateTime.year / 100 ) * 100 );
                sprintf( pszBuffer, "%d/%d/%d", sctDateTime.month,
                         sctDateTime.day, sctDateTime.year );
              }
            break;
            case QDPR_FORMAT_DATE_US_FULL :
              {
                sprintf( pszBuffer, "%d/%d/%d", sctDateTime.year,
                         sctDateTime.month, sctDateTime.day );
              }
            break;
            case QDPR_FORMAT_DATE_US :
              {
                sctDateTime.year -= ( ( sctDateTime.year / 100 ) * 100 );
                sprintf( pszBuffer, "%d/%d/%d", sctDateTime.year,
                         sctDateTime.month, sctDateTime.day );
              }
            break;
            case QDPR_FORMAT_DATE_EU_FULL :
              {
                sprintf( pszBuffer, "%d.%d.%d", sctDateTime.day,
                         sctDateTime.month, sctDateTime.year );
              }
            break;
            case QDPR_FORMAT_DATE_EU :
              {
                sctDateTime.year -= ( ( sctDateTime.year / 100 ) * 100 );
                sprintf( pszBuffer, "%d.%d.%d", sctDateTime.day,
                         sctDateTime.month, sctDateTime.year );
              }
            break;
            case QDPR_FORMAT_TIME_NORM :
              {
                sprintf( pszBuffer, "%d:%02d:%02d", sctDateTime.hours,
                         sctDateTime.minutes, sctDateTime.seconds );
              }
            break;
            case QDPR_FORMAT_TIME_NORM_NSEC :
              {
                sprintf( pszBuffer, "%d:%02d", sctDateTime.hours,
                         sctDateTime.minutes );
              }
            break;
            case QDPR_FORMAT_TIME_PM :
              {
                if ( ( sctDateTime.hours >= 12 ) &&
                     ( sctDateTime.hours <= 24 ) )
                {
                  if ( sctDateTime.hours != 12 )
                  {
                    sctDateTime.hours -= 12;
                  } /* endif */

                  sprintf( pszBuffer, "%d:%02d:%02dpm", sctDateTime.hours,
                           sctDateTime.minutes, sctDateTime.seconds );
                }
                else
                {
                  if ( sctDateTime.hours == 24 )
                  {
                    sctDateTime.hours -= 12;
                  } /* endif */

                  sprintf( pszBuffer, "%d:%02d:%02dam", sctDateTime.hours,
                           sctDateTime.minutes, sctDateTime.seconds );
                } /* endif */
              }
            break;
            case QDPR_FORMAT_TIME_PM_NSEC :
              {
                if ( ( sctDateTime.hours >= 12 ) &&
                     ( sctDateTime.hours <= 24 ) )
                {
                  if ( sctDateTime.hours != 12 )
                  {
                    sctDateTime.hours -= 12;
                  } /* endif */

                  sprintf( pszBuffer, "%d:%02dpm", sctDateTime.hours,
                           sctDateTime.minutes );
                }
                else
                {
                  if ( sctDateTime.hours == 24 )
                  {
                    sctDateTime.hours -= 12;
                  } /* endif */

                  sprintf( pszBuffer, "%d:%02dam", sctDateTime.hours,
                           sctDateTime.minutes );
                } /* endif */
              }
            break;
          } /* endswitch */
        }
      break;
      case QDPR_SYSNAME_PAGE_NO_ATTR :
        {
          sprintf( psctIDA->szWorkBuffer, "%ld", psctIDA->ulPageNumber );

          if ( ( psctAttribs->bitAttribs & QDPR_ATTR_RIGHT ) ==
               QDPR_ATTR_RIGHT )
          {
            if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MIN ) ==
                 QDPR_ATTR_MIN )
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%*.*s", psctAttribs->usMinChars,
                         psctAttribs->usMaxChars, psctIDA->szWorkBuffer );
              }
              else
              {
                sprintf( pszBuffer, "%*s", psctAttribs->usMinChars,
                         psctIDA->szWorkBuffer );
              } /* endif */
            }
            else
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%.*s", psctAttribs->usMaxChars,
                         psctIDA->szWorkBuffer );
              }
              else
              {
                sprintf( pszBuffer, "%s", psctIDA->szWorkBuffer );
              } /* endif */
            } /* endif */
          }
          else
          {
            if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MIN ) ==
                 QDPR_ATTR_MIN )
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%-*.*s", psctAttribs->usMinChars,
                         psctAttribs->usMaxChars, psctIDA->szWorkBuffer );
              }
              else
              {
                sprintf( pszBuffer, "%-*s", psctAttribs->usMinChars,
                         psctIDA->szWorkBuffer );
              } /* endif */
            }
            else
            {
              if ( ( psctAttribs->bitAttribs & QDPR_ATTR_MAX ) ==
                   QDPR_ATTR_MAX )
              {
                sprintf( pszBuffer, "%.*s", psctAttribs->usMaxChars,
                         psctIDA->szWorkBuffer );
              }
              else
              {
                sprintf( pszBuffer, "%s", psctIDA->szWorkBuffer );
              } /* endif */
            } /* endif */
          } /* endif */
        }
      break;
      case QDPR_SYSNAME_PAGE_EJECT_ATTR :
        {
          /************************************************************/
          /*            add so many CRLFs to fill the page            */
          /************************************************************/
          /************************************************************/
          /* header pages do not have Pagefootlines, so do different  */
          /* calculation                                              */
          /************************************************************/
          if ( psctIDA->usNextThreadStatus == QDPR_PRTST_PRINT_HEADER )
          {
            usCurPgFootLen = 0;
          }
          else
          {
            usCurPgFootLen = psctIDA->usCaPagefootLines;
          } /* endif */

          if ( (psctIDA->usPageLength) - usCurPgFootLen + 1
                >= ( psctIDA->usLineNumber)    )
          {
            usLinesFree = psctIDA->usPageLength - usCurPgFootLen
                           - psctIDA->usLineNumber + 1;
          }
          else
          {
            usLinesFree = 0;
          } /* endif */
          for ( i = 1; i <= usLinesFree; i++ )
          {
            if ( psctIDA->fCRLFUsed )
            {
              pszBuffer[strlen( pszBuffer )] = QDPR_CR;
            } /* endif */

            pszBuffer[strlen( pszBuffer )] = QDPR_LF;
            (*pusLines)++;

            if ( i < usLinesFree )
            {
              usRC = QDPRFormatStrToLineLength(
                                  psctIDA, psctIDA->psctFormatIDA,
                                  pszBuffer, psctCurBufExt, &usLines );
              memset( pszBuffer, NULC, 3 );
            } /* endif */
          } /* endfor */
        }
      break;
    } /* endswitch */

    /******************************************************************/
    /*          format the string to fit to the line length           */
    /******************************************************************/
    usRC = QDPRFormatStrToLineLength( psctIDA, psctIDA->psctFormatIDA,
                                      pszBuffer, psctCurBufExt,
                                      &usLines );
    (*pusLines) = (*pusLines) + usLines;

    /******************************************************************/
    /*          deallocate the buffer temporarily allocated           */
    /******************************************************************/
    UtlAlloc( (PVOID *) &pszBuffer, 0L, 0L, NOMSG );
  } /* endif */

  return( usRC );

} /* end of function QDPREvaluateSysVar */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRFormatStrToLineLength                                    |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRFormatStrToLineLength( psctIDA, psctFormatIDA, pszString,      |
//|                                    psctCurBuffer, pusLineNumbers )         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function formats a string so that it fits into the linelength        |
//|  of the page buffer. The string itself is not directly written to the      |
//|  page buffer, but to the buffer area in psctCurBuffer, so that             |
//|  further processing of the string may take place.                          |
//|                                                                            |
//|  If the whole string fits into the current line of the page buffer         |
//|  the string will just be copied to the buffer area.                        |
//|                                                                            |
//|  If the string doesn't fit the string will be formatted to fit in the      |
//|  page buffer. In this case the existing CRLFs are preserved and if         |
//|  a single line is too long, it will be splitted trying to split            |
//|  between words, preserving the identation (this will not be possible       |
//|  if the whole word doesn't fit on the line, in this case the word          |
//|  is split).                                                                |
//|                                                                            |
//|  The function returns the number of extra lines the string would           |
//|  need in the page buffer (0 if the string fits on the same line,           |
//|  1 if the string would take the current line plus another line, etc.).     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD         psctIDA;        // pointer to thread IDA             |
//|  PQDPR_FORMAT_IDA     psctFormatIDA;  // pointer to format IDA             |
//|  PSZ                  pszString;      // string to be formatted            |
//|  PQDPR_FORMAT_BUFFERS psctCurBuffer;  // pointer to current format         |
//|                                       // buffer                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PUSHORT              pusLineNumbers; // No of lines the string now uses   |
//|                                       // it will be 0 if the string        |
//|                                       // fitted on the current line        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a correctly set up format IDA                                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - changes the pchrLastWritten in the format IDA                           |
//|  - updates the psctCurStrBufExt if necessary                               |
//|  - writes the string to psctCurBuffer->achrBuffer                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRFormatStrToLineLength( psctIDA, psctFormatIDA, "TEST",         |
//|                                    psctFormatIDA->psctEntry, &usNo );      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Calculate number of free bytes in page buffer                             |
//|  Calculate current line indent in page buffer                              |
//|  WHILE pszString is not at the end                                         |
//|    Run to the first CRLF or the end of the string or stop if               |
//|      the linelength is overrun                                             |
//|    IF string doesn't fit completely on the line                            |
//|      Run to the beginning of the word which doesn't fit anymore on the     |
//|        line                                                                |
//|      IF the whole word doesn't fit anymore on the line                     |
//|        Split up the word                                                   |
//|      Write the part that fits on the line to the string buffer and         |
//|        increase the number of lines returned                               |
//|    ELSE                                                                    |
//|      Copy the part just scanned to the string buffer                       |
//|      IF a CRLF if found                                                    |
//|        Increase the number of lines returned                               |
//+----------------------------------------------------------------------------+

USHORT QDPRFormatStrToLineLength
(
  PQDPR_THREAD         psctIDA,        // pointer to thread IDA
  PQDPR_FORMAT_IDA     psctFormatIDA,  // pointer to format IDA
  PSZ                  pszString,      // string to be formatted
  PQDPR_FORMAT_BUFFERS psctCurBuffer,  // pointer to current format
                                       // buffer
  PUSHORT              pusLineNumbers  // No of lines the string now uses
                                       // it will be 0 if the string
                                       // fitted on the current line
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  USHORT                usLineFree;               // no of chars on line
                                                  // that are free
  USHORT                usCurIndent;              // current indent
  USHORT                usTempCurIndent;          // temp current indent
  PSZ                   pszTempString;            // temp string
  ULONG                 ulTempStringSize;         // temp string size
  PCHAR                 pchrTest;                 // test char pointer
  PCHAR                 pchrSearch;               // search pointer
  PCHAR                 pchrWord;                 // pointer to start
                                                  // of a full word
  PCHAR                 pchrBlanks;               // pointer to test for
                                                  // blanks
  BOOL                  fStringBegin;             // String begins ?
  BOOL                  fStopProcessing = FALSE;  // stop processing the
                                                  // string



  if ( pszString != NULL )
  {
    /******************************************************************/
    /*                      initialize variables                      */
    /******************************************************************/
    *pusLineNumbers = 0;
    fStringBegin = TRUE;

    /******************************************************************/
    /*   if a new line starts calculate the current indent from the   */
    /*                             string                             */
    /*  but if this indent is larger than the linelength set it to 0  */
    /******************************************************************/
    if ( psctCurBuffer->fNewLineStarts && ( pszString[0] != NULC ) )
    {
      psctCurBuffer->usCurIndent = 0;
      pchrTest = pszString;
      while ( *pchrTest == ' ' )
      {
        (psctCurBuffer->usCurIndent)++;
        pchrTest++;
      } /* endwhile */

      if ( psctCurBuffer->usCurIndent >= psctIDA->usLineLength )
      {
        psctCurBuffer->usCurIndent = 0;
      } /* endif */

      psctCurBuffer->fNewLineStarts = FALSE;
    } /* endif */

    /******************************************************************/
    /*      check if there are characters still in the  buffer,       */
    /*           if so calculate no of chars that are free            */
    /*           on the current line of the current buffer            */
    /******************************************************************/
    if ( psctCurBuffer->pchrLastWritten !=
         psctCurBuffer->psctBuffer->achrBuffer )
    {
      usLineFree = psctIDA->usLineLength - psctCurBuffer->usCharsUsed;
    }
    else
    {
      /****************************************************************/
      /*   calculate no of chars that are free on the current line    */
      /*                      in the page buffer                      */
      /****************************************************************/
      usLineFree = (USHORT)(psctIDA->usLineLength -
                   ( (psctFormatIDA->pchrLastChar) -
                     (psctFormatIDA->pchrLastLF) ));
      psctCurBuffer->usCharsUsed = (USHORT)(( (psctFormatIDA->pchrLastChar) -
                                     (psctFormatIDA->pchrLastLF) ));
    } /* endif */

    if ( usLineFree == 0 )
    {
      /****************************************************************/
      /* the current line is full, now check if the next string only  */
      /*  consists of ' 's and a CRLF or a NULC (to end the string)   */
      /*    if so put a CRLF to the buffer and set a flag, so that    */
      /*                no further processing is done                 */
      /****************************************************************/
      pchrTest = pszString;
      while ( ( *pchrTest == ' ' ) && ( usRC == QDPR_NO_ERROR ) )
      {
        pchrTest++;
      } /* endwhile */

      if ( *pchrTest != NULC )
      {
        /****************************************************************/
        /*              add a (CR)LF to the format buffer               */
        /****************************************************************/
        if ( UtlAlloc( (PVOID *) &pszTempString, 0L, 3L, NOMSG ) )
        {
          if ( psctIDA->fCRLFUsed )
          {
            pszTempString[0] = QDPR_CR;
          } /* endif */

          pszTempString[ strlen(pszTempString) ] = QDPR_LF;

          usRC = QDPRAddToProcessBuffer(
                          &(psctCurBuffer->psctCurBufExt),
                          &(psctCurBuffer->pchrLastWritten),
                          pszTempString );

          UtlAlloc( (PVOID *) &pszTempString, 0L, 0L, NOMSG );

          if ( *pchrTest == QDPR_LF )
          {
            psctCurBuffer->fNewLineStarts = TRUE;
            psctCurBuffer->usCharsUsed = 0;
            psctCurBuffer->usCurIndent = 0;
            psctCurBuffer->fLineSplit = FALSE;
          }
          else
          {
            psctCurBuffer->usCharsUsed = 0;
            psctCurBuffer->fLineSplit = TRUE;
          } /* endif */
        }
        else
        {
          usRC = QDPR_NO_MEMORY;
        } /* endif */

        usLineFree = psctIDA->usLineLength;
      } /* endif */

      if ( ( *pchrTest == QDPR_LF ) || ( *pchrTest == NULC ) )
      {
        fStopProcessing = TRUE;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /*                  now loop till the string end                  */
    /******************************************************************/
    pchrTest = pszString;
    while ( ( *pchrTest != NULC ) && !fStopProcessing )
    {
      usCurIndent = 0;
      usTempCurIndent = 0;
      pchrSearch = pchrTest;

      /****************************************************************/
      /*   if the line has been split set the current indent to the   */
      /*             indent at the beginning of the line              */
      /****************************************************************/
      if ( psctCurBuffer->fLineSplit )
      {
        usCurIndent = psctCurBuffer->usCurIndent;
      } /* endif */

      /****************************************************************/
      /*     run until a LF is found or the linelength is overrun     */
      /****************************************************************/
      while ( ( *pchrSearch != QDPR_LF ) && ( *pchrSearch != NULC ) &&
              ( ( pchrSearch - pchrTest + usCurIndent ) < usLineFree ) )
      {
        switch ( *pchrSearch )
        {
          case ' ' :
            {
              /********************************************************/
              /* a blank has been found, so increase the indentation  */
              /*value, but only if it is at the beginning of the line */
              /********************************************************/
              if ( fStringBegin )
              {
                usTempCurIndent++;
              } /* endif */
            }
          break;
          case QDPR_CR :
            {
              /********************************************************/
              /*  a CR has been found, so increase usLineFree by one  */
              /*     in order to get the LF as well and not being     */
              /*   disturbed by the fact that the linelength may be   */
              /*                       overrun                        */
              /********************************************************/
              usLineFree++;
            }
          break;
          default  :
            {
              fStringBegin = FALSE;
            }
          break;
        } /* endswitch */

        pchrSearch++;
      } /* endwhile */

      /****************************************************************/
      /* check if the current indent is greater than the linelength,  */
      /*                      if so set it to 0                       */
      /*  remove any leading blanks and start the search for a line   */
      /*                          fit again                           */
      /****************************************************************/
      if ( ( usCurIndent + usTempCurIndent >= usLineFree ) &&
           ( usTempCurIndent != 0 ) )
      {
        usCurIndent = 0;
        while ( *pchrTest == ' ' )
        {
          pchrTest++;
        } /* endwhile */
        pchrSearch = pchrTest;
      }
      else
      {
        if ( usCurIndent >= usLineFree )
        {
          usCurIndent = 0;
        } /* endif */

        /**************************************************************/
        /*  check if the text till the next LF fits completly on the  */
        /*           line or if the linelength was overrun            */
        /**************************************************************/
        if ( ( *pchrSearch != QDPR_LF ) && ( *pchrSearch != NULC ) )
        {
          /************************************************************/
          /* text does not fit completly on the line, so walk back to */
          /*         the first blank before the current word          */
          /************************************************************/
          pchrWord = pchrSearch;
          while ( ( *pchrWord != ' ' ) && ( pchrWord != pchrTest ) )
          {
            pchrWord--;
          } /* endwhile */

          if ( pchrWord != pchrTest )
          {
            pchrWord++;
          } /* endif */

          /************************************************************/
          /*   check if there are only blanks between pchrTest and    */
          /*                         pchrWord                         */
          /************************************************************/
          pchrBlanks = pchrTest;
          while ( ( *pchrBlanks == ' ' ) && ( pchrBlanks != pchrWord ) )
          {
            pchrBlanks++;
          } /* endwhile */

          /************************************************************/
          /*check if the two pointers met which means that the current*/
          /*    word is too long to fit on the line, so run to the    */
          /*         location where the word has to be split          */
          /*   or if there are only blanks between the pchrTest and   */
          /*               pchrWord split the word also               */
          /************************************************************/
          if ( ( pchrWord == pchrBlanks ) &&
               ( psctCurBuffer->usCharsUsed == 0 ) )
          {
            while ( ((pchrWord - pchrTest) + usCurIndent) < usLineFree )
            {
              pchrWord++;
            } /* endwhile */
          } /* endif */

          /************************************************************/
          /*  now write the text between pchrWord and pchrTest to a   */
          /*    temporarily allocated string (preserve the current    */
          /*     indentation), then write the temp string to the      */
          /*                      current buffer                      */
          /************************************************************/
          ulTempStringSize = (LONG)( ( pchrWord - pchrTest ) +
                                     usCurIndent + 2 + 1 );
          if ( UtlAlloc( (PVOID *) &pszTempString, 0L, ulTempStringSize, NOMSG ) )
          {
            memset( pszTempString, ' ', usCurIndent );
            memcpy( &(pszTempString[usCurIndent]), pchrTest,
                    ( pchrWord - pchrTest ) );

            if ( psctIDA->fCRLFUsed )
            {
              pszTempString[ strlen(pszTempString) ] = QDPR_CR;
            } /* endif */

            pszTempString[ strlen(pszTempString) ] = QDPR_LF;

            (*pusLineNumbers)++;

            usRC = QDPRAddToProcessBuffer(
                       &(psctCurBuffer->psctCurBufExt),
                       &(psctCurBuffer->pchrLastWritten),
                       pszTempString );

            UtlAlloc( (PVOID *) &pszTempString, 0L, 0L, NOMSG );
          }
          else
          {
            usRC = QDPR_NO_MEMORY;
          } /* endif */

          pchrTest = pchrWord;
          psctCurBuffer->usCharsUsed = 0;
          psctCurBuffer->fLineSplit = TRUE;
        }
        else
        {
          /************************************************************/
          /*      text between pchrTest and pchrSearch now fits       */
          /*                  completely on the line                  */
          /************************************************************/
          if ( *pchrSearch != NULC )
          {
            pchrSearch++;
            psctCurBuffer->usCharsUsed = 0;
            psctCurBuffer->usCurIndent = 0;
            psctCurBuffer->fNewLineStarts = TRUE;
            psctCurBuffer->fLineSplit = FALSE;
          }
          else
          {
            (psctCurBuffer->usCharsUsed) = (USHORT)((psctCurBuffer->usCharsUsed) +
                                             pchrSearch - pchrTest +
                                            usCurIndent);
          } /* endif */

          /************************************************************/
          /* now write the text between pchrSearch and pchrTest to a  */
          /*    temporarily allocated string (preserve the current    */
          /*     indentation), then write the temp string to the      */
          /*                      current buffer                      */
          /************************************************************/
          ulTempStringSize = (LONG)( ( pchrSearch - pchrTest ) +
                                     usCurIndent + 2 + 1 );
          if ( UtlAlloc( (PVOID *) &pszTempString, 0L,
                         max( (LONG)MIN_ALLOC, ulTempStringSize), NOMSG ) )
          {
            memset( pszTempString, ' ', usCurIndent );
            memcpy( &(pszTempString[usCurIndent]), pchrTest,
                    ( pchrSearch - pchrTest ) );

            usRC = QDPRAddToProcessBuffer(
                            &(psctCurBuffer->psctCurBufExt),
                            &(psctCurBuffer->pchrLastWritten),
                            pszTempString );

            UtlAlloc( (PVOID *) &pszTempString, 0L, 0L, NOMSG );
          }
          else
          {
            usRC = QDPR_NO_MEMORY;
          } /* endif */

          pchrTest = pchrSearch;
        } /* endif */

        usLineFree = psctIDA->usLineLength;
      } /* endif */
    } /* endwhile */
  } /* endif */

  return( usRC );

} /* end of function QDPRFormatStrToLineLength */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintEntry                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintEntry( psctIDA )                                          |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function prints the entry part.                                      |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD    psctIDA;       // thread IDA                              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_PROGRAM_ERROR    - internal program error - should not occurr        |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid and filled thread IDA                                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - a lot                                                                   |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  WHILE print status is QDPR_PRTST_PRINT_ENTRY                              |
//|    IF there are lines to be printed in the entry format buffer             |
//|      Print the lines to the page buffer (QDPRPrintToPageBuffer)            |
//|    Print from the entry buffer to the psctIDA->psctFormatIDA->psctEntry    |
//|      buffer until a system variable is found or CRLF occurred              |
//|      (QDPRPrintUntilTag)                                                   |
//|    SWITCH on the tag read by QDPRPrintUntilTag                             |
//|      CASE a CRLF has been read                                             |
//|        Increase the number of lines in psctEntry                           |
//|      CASE a system variable                                                |
//|        CALL QDPREvaluateSysVar                                             |
//|      CASE a field tag                                                      |
//|        CALL QDPREvaluateFieldTag                                           |
//|      CASE a repeat tag                                                     |
//|        CALL QDPREvaluateRepeatTag                                          |
//|      CASE an end-repeat tag                                                |
//|        CALL QDPREvaluateEndRepeatTag                                       |
//|      DEFAULT                                                               |
//|        Nothing else should occurr than the tags above so return            |
//|          a QDPR_PROGRAM_ERROR if this happens                              |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintEntry
(
  PQDPR_THREAD    psctIDA        // thread IDA
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT         usRC = QDPR_NO_ERROR;           // function returncode
  USHORT         usTagID;                        // ID of read tag
  USHORT         usLines;                        // no of lines read
  USHORT         usPrintLines;                   // no of lines to print
  USHORT         usFCRTElement;                  // FCRT element number



  /********************************************************************/
  /*           loop as long as the entry is to be printed             */
  /********************************************************************/
  while ( ( psctIDA->usNextThreadStatus == QDPR_PRTST_PRINT_ENTRY ) &&
          ( usRC == QDPR_NO_ERROR ) )
  {
    /********************************************************************/
    /*        check if there are lines left in the entry format         */
    /*                buffer that have not been printed                 */
    /********************************************************************/
    if ( psctIDA->usEntryTotalLines != 0 )
    {
      /******************************************************************/
      /*   there are lines in the entry format buffer, so print them    */
      /*            calculate how many lines may be printed             */
      /******************************************************************/
      usPrintLines = psctIDA->usPageLength - psctIDA->usCaPagefootLines -
                     psctIDA->usLineNumber + 1;

      /******************************************************************/
      /*       check if all lines left in the format entry buffer       */
      /*                   would fit on the new page                    */
      /******************************************************************/
      if ( psctIDA->usEntryLinesPrinted + usPrintLines >=
           psctIDA->usEntryTotalLines )
      {
        usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                              psctIDA->psctFormatIDA->psctEntry,
                              (USHORT)(psctIDA->usEntryLinesPrinted + 1),
                              (USHORT)(psctIDA->usEntryTotalLines + 2), TRUE );

        if ( usRC == QDPR_NO_ERROR )
        {
          usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctEntry,
                                        FALSE );

          psctIDA->usLineNumber += ( psctIDA->usEntryTotalLines -
                                     psctIDA->usEntryLinesPrinted );

          psctIDA->usEntryLinesPrinted = 0;
          psctIDA->usEntryTotalLines = 0;
        } /* endif */

        /****************************************************************/
        /*          if buffer was at end set new print status           */
        /*         to filtering a new entry from the dictionary         */
        /* or if last entry has been processed go to pagefoot processing*/
        /****************************************************************/
        if ( psctIDA->fEntryBufferEnd )
        {
          if ( psctIDA->ulDictEntries != psctIDA->ulEntriesProcessed )
          {
            psctIDA->usNextThreadStatus = QDPR_PRTST_FILTER;
            psctIDA->usPreviousPrintStatus = QDPR_PRTST_START_ENTRY;
            psctIDA->fPrintEntry = FALSE;
          }
          else
          {
            psctIDA->usNextThreadStatus = QDPR_PRTST_START_PAGEFOOT;
            psctIDA->usPreviousPrintStatus = QDPR_PRTST_START_PAGEHEAD;
            psctIDA->fPrintEntry = FALSE;
          } /* endif */
        } /* endif */
      }
      else
      {
        /****************************************************************/
        /*     not all lines fit, so print those that fit and then      */
        /*               print the pagefoot and pagehead                */
        /****************************************************************/
        usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                                      psctIDA->psctFormatIDA->psctEntry,
                                      (USHORT)(psctIDA->usEntryLinesPrinted + 1),
                                      (USHORT)(psctIDA->usEntryLinesPrinted +
                                      usPrintLines + 1), TRUE );

        if ( usRC == QDPR_NO_ERROR )
        {
          (psctIDA->usEntryLinesPrinted) = (psctIDA->usEntryLinesPrinted) + usPrintLines;

          psctIDA->usNextThreadStatus = QDPR_PRTST_START_PAGEFOOT;
          psctIDA->usPreviousPrintStatus = QDPR_PRTST_PRINT_ENTRY;
          psctIDA->fPrintEntry = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */

    if ( ( psctIDA->usEntryTotalLines == 0 ) &&
         !( psctIDA->fEntryBufferEnd ) )
    {
      /******************************************************************/
      /*     print until a system variable, a tag or a CRLF occurs      */
      /******************************************************************/
      usRC = QDPRPrintUntilTag( psctIDA, &(psctIDA->psctCurEntryExt),
                                &(psctIDA->pchrEntryBuffer),
                                psctIDA->psctFormatIDA->psctEntry,
                                &(psctIDA->psctCurEntryFCRT),
                                &(psctIDA->psctCurRepeatFCRT),
                                &usTagID, &usFCRTElement, &usLines );

      if ( usRC == QDPR_BUFFER_AT_END )
      {
        /****************************************************************/
        /*           increase the number of entries processed           */
        /****************************************************************/
        (psctIDA->ulEntriesProcessed)++;

        /****************************************************************/
        /*             indicate that buffer end was reached             */
        /****************************************************************/
        psctIDA->fEntryBufferEnd = TRUE;

        /****************************************************************/
        /*          check if all entries are processed, if so           */
        /*       let it seem as if there was a $PAGE_EJECT system       */
        /*      variable encountered and use the processing below       */
        /****************************************************************/
        if ( ( psctIDA->ulDictEntries == psctIDA->ulEntriesProcessed ) )
        {
          /************************************************************/
          /* XQG: Do this only if a page footer exists!               */
          /************************************************************/
          if ( psctIDA->psctPagefootBuffer != NULL )
          {
            usTagID = QDPR_SYSNAME_PAGE_EJECT_ATTR;
            usRC = QDPR_NO_ERROR;
          } /* endif */
        }
        else
        {
          if ( ( psctIDA->usEntryTotalLines == 0 ) &&
               ( psctIDA->psctFormatIDA->psctEntry->usCharsUsed != 0 )
               && ( usLines == 0 ) )
          {
            usTagID = QDPR_ID_CRLF;
            usRC = QDPR_NO_ERROR;
          }
          else
          {
            psctIDA->usNextThreadStatus = QDPR_PRTST_FILTER;
            psctIDA->usPreviousPrintStatus = QDPR_PRTST_START_ENTRY;
            psctIDA->fPrintEntry = FALSE;
          } /* endif */
        } /* endif */
      } /* endif */

      if ( usRC == QDPR_NO_ERROR )
      {
        (psctIDA->usEntryTotalLines) = (psctIDA->usEntryTotalLines) + usLines;

        /****************************************************************/
        /*                check which tag has been found                */
        /****************************************************************/
        switch ( usTagID )
        {
          case QDPR_ID_CRLF :
            {
              /**********************************************************/
              /*       CRLF has been found, so just increase the        */
              /*       number of lines in the entry format buffer       */
              /**********************************************************/
              (psctIDA->usEntryTotalLines)++;

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_ENTRY;
            }
          break;
          case QDPR_SYSNAME_FILENAME_ATTR :
          case QDPR_SYSNAME_DICTNAME_ATTR :
          case QDPR_SYSNAME_DATE_ATTR :
          case QDPR_SYSNAME_TIME_ATTR :
          case QDPR_SYSNAME_PAGE_NO_ATTR :
          case QDPR_SYSNAME_PAGE_EJECT_ATTR :
            {
              /**********************************************************/
              /*              Evaluate the system variable              */
              /**********************************************************/
              usRC = QDPREvaluateSysVar( psctIDA, usTagID,
                                         psctIDA->psctCurEntryFCRT,
                                         usFCRTElement,
                                         psctIDA->psctFormatIDA->psctEntry,
                                         &usLines );

              (psctIDA->usEntryTotalLines) = (psctIDA->usEntryTotalLines) + usLines;

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_ENTRY;
            }
          break;
          case QDPR_NAME_ATTR :
            {
              /**********************************************************/
              /*      a field tag has been found, so get the data       */
              /*                     from the field                     */
              /**********************************************************/
              usRC = QDPREvaluateFieldTag( psctIDA,
                                           psctIDA->psctCurEntryFCRT,
                                           usFCRTElement,
                                           psctIDA->psctFormatIDA->psctEntry,
                                           &usLines );

              (psctIDA->usEntryTotalLines) = (psctIDA->usEntryTotalLines) + usLines;

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_ENTRY;
            }
          break;
          case QDPR_REPEAT_TOKEN :
            {
              /**********************************************************/
              /*      a repeat tag has been found, so evaluate it       */
              /**********************************************************/
              usRC = QDPREvaluateRepeatTag( psctIDA,
                                            &(psctIDA->psctRepeatStack),
                                            psctIDA->psctCurRepeatFCRT,
                                            usFCRTElement );
            }
          break;
          case QDPR_REPEAT_ETOKEN :
            {
              /**********************************************************/
              /*     an end repeat tag has been found, evaluate it      */
              /**********************************************************/
              usRC = QDPREvaluateEndRepeatTag( psctIDA,
                                               &(psctIDA->psctRepeatStack),
                                               psctIDA->psctEntry,
                                               psctIDA->psctCurrentTemplate,
                                               &(psctIDA->pchrEntryBuffer) );
            }
          break;
          default :
            {
              /**********************************************************/
              /*         this should not occurr, but anyway set         */
              /*                   QDPR_PROGRAM_ERROR                   */
              /**********************************************************/
              usRC = QDPR_PROGRAM_ERROR;
            }
          break;
        } /* endswitch */
      }
      else
      {
        /****************************************************************/
        /*        if the error code was buffer at end, set it to        */
        /*       no error in order not to get a message and stop        */
        /****************************************************************/
        if ( usRC == QDPR_BUFFER_AT_END )
        {
          usRC = QDPR_NO_ERROR;
        } /* endif */
      } /* endif */
    }
    else
    {
      /******************************************************************/
      /*                   check if buffer is at end                    */
      /******************************************************************/
      if ( psctIDA->fEntryBufferEnd )
      {
        /****************************************************************/
        /*        check if there are still lines to print in the        */
        /*                      entry format buffer                     */
        /****************************************************************/
        if ( psctIDA->usEntryTotalLines == 0 )
        {
          /**************************************************************/
          /*          no more lines for the entry to print, so          */
          /*               start to process the pagefoot                */
          /**************************************************************/
          if ( psctIDA->ulDictEntries != psctIDA->ulEntriesProcessed )
          {
            psctIDA->usNextThreadStatus = QDPR_PRTST_FILTER;
            psctIDA->usPreviousPrintStatus = QDPR_PRTST_START_ENTRY;
            psctIDA->fPrintEntry = FALSE;
          }
          else
          {
            psctIDA->usNextThreadStatus = QDPR_PRTST_START_PAGEFOOT;
            psctIDA->usPreviousPrintStatus = QDPR_PRTST_START_PAGEHEAD;
            psctIDA->fPrintEntry = FALSE;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endwhile */

  return( usRC );

} /* end of function QDPRPrintEntry */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintHeader                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintHeader( psctIDA )                                         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function prints the header part.                                     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD    psctIDA;       // thread IDA                              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_PROGRAM_ERROR    - internal program error - should not occurr        |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid and filled thread IDA                                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - a lot                                                                   |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  WHILE print status is QDPR_PRTST_PRINT_HEADER                             |
//|    Print from the header buffer to the psctIDA->psctFormatIDA->psctOther   |
//|      buffer until a system variable is found or CRLF occurred              |
//|      (QDPRPrintUntilTag)                                                   |
//|    IF current page doesn't become full                                     |
//|      Increase the number of lines in psctOther                             |
//|      IF header buffer is at the end                                        |
//|        Print the full psctOther buffer to the page buffer                  |
//|    ELSE                                                                    |
//|      Calculate how many pages are in the psctOther buffer                  |
//|      IF header buffer is at the end                                        |
//|        Print the full psctOther buffer to the page buffer                  |
//|    SWITCH on the tag read by QDPRPrintUntilTag                             |
//|      CASE a CRLF has been read                                             |
//|        Increase the number of lines in psctOther                           |
//|      CASE a system variable                                                |
//|        CALL QDPREvaluateSysVar                                             |
//|      DEFAULT                                                               |
//|        Nothing else should occurr than the tags above so return            |
//|          a QDPR_PROGRAM_ERROR if this happens                              |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintHeader
(
  PQDPR_THREAD    psctIDA        // thread IDA
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT         usRC = QDPR_NO_ERROR;           // function returncode
  USHORT         usTempRC = 0;                   // temp returncode
  USHORT         usTagID;                        // ID of read tag
  USHORT         usLines;                        // no of lines read
  USHORT         usPrintLines;                   // no of lines to print
  USHORT         usFCRTElement;                  // FCRT element number
  USHORT         i;                              // an index



  /********************************************************************/
  /*           loop as long as the header is to be printed            */
  /********************************************************************/
  while ( ( psctIDA->usNextThreadStatus == QDPR_PRTST_PRINT_HEADER ) &&
          ( usRC == QDPR_NO_ERROR ) )
  {
    /******************************************************************/
    /*        print until a system variable or a CRLF occurrs         */
    /*    printing is done into psctIDA->psctFormatIDA->psctOther     */
    /******************************************************************/
    usRC = QDPRPrintUntilTag( psctIDA, &(psctIDA->psctCurHeaderExt),
                              &(psctIDA->pchrHeaderBuffer),
                              psctIDA->psctFormatIDA->psctOther,
                              &(psctIDA->psctCurHeaderFCRT),
                              NULL, &usTagID, &usFCRTElement, &usLines );

    /******************************************************************/
    /*       check if returncode is ok or buffer is at the end        */
    /******************************************************************/
    if ( ( usRC == QDPR_NO_ERROR ) || ( usRC == QDPR_BUFFER_AT_END ) )
    {
      /****************************************************************/
      /*     save current returncode, because it is needed later      */
      /****************************************************************/
      usTempRC = usRC;
      usRC = QDPR_NO_ERROR;

      /****************************************************************/
      /*            check if the current page becomes full            */
      /****************************************************************/
      if ( ( psctIDA->usLineNumber + usLines ) <= psctIDA->usPageLength )
      {
        /**************************************************************/
        /* the page doesn't run full, so increase the number of lines */
        /**************************************************************/
        (psctIDA->usLineNumber) = (psctIDA->usLineNumber) + usLines;

        /**************************************************************/
        /*   if the header buffer is at its end print the formatted   */
        /*        header text in psctOther to the page buffer         */
        /**************************************************************/
        if ( usTempRC == QDPR_BUFFER_AT_END )
        {
          usPrintLines = 1;

          /************************************************************/
          /*              print all pages that are full               */
          /************************************************************/
          for ( i = 0; ( i < psctIDA->usPagesProcessed ) &&
                       ( usRC == QDPR_NO_ERROR ); i++ )
          {
            usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                       psctIDA->psctFormatIDA->psctOther, usPrintLines,
                       (USHORT)(usPrintLines + psctIDA->usPageLength), FALSE );

            if ( usRC == QDPR_NO_ERROR )
            {
              usRC = QDPRPrintPageEject( psctIDA,
                                         psctIDA->psctFormatIDA->psctEntry );

              usPrintLines = usPrintLines + psctIDA->usPageLength;
            } /* endif */
          } /* endfor */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*   print the page that is left (i.e. it is not full)    */
            /**********************************************************/
            usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                       psctIDA->psctFormatIDA->psctOther, usPrintLines,
                      (USHORT)( usPrintLines + psctIDA->usLineNumber), TRUE );
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            usRC = QDPRPrintPageEject( psctIDA,
                                       psctIDA->psctFormatIDA->psctEntry );

            psctIDA->usLineNumber = 1;
            psctIDA->usPagesProcessed = 0;
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*      the format buffer is not needed anymore, so       */
            /*     clear it and deallocate the buffer extensions      */
            /**********************************************************/
            usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                          TRUE );
          } /* endif */

          /************************************************************/
          /*                   set new print status                   */
          /************************************************************/
          psctIDA->usNextThreadStatus = QDPR_PRTST_FILTER;
          psctIDA->usPreviousPrintStatus = QDPR_PRTST_START_PAGEHEAD;
        }
        else
        {
          psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_HEADER;
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /*       page becomes full, so calculate the number of        */
        /*      lines that still fit on the page (usPrintLines)       */
        /*       then calculate how many new (full) pages there       */
        /*            will result out of usLines and then             */
        /*               calculate the new line number                */
        /**************************************************************/
        /**************************************************************/
        /* do not use usCaPagefootLines because header has no footnote*/
        /**************************************************************/
//        usPrintLines = psctIDA->usPageLength - (psctIDA->usLineNumber - 1);
        usPrintLines = psctIDA->usPageLength
                       - psctIDA->usLineNumber + 1;
        (psctIDA->usPagesProcessed)++;
        (psctIDA->ulPageNumber)++;

        psctIDA->usPagesProcessed += (usLines - usPrintLines) /
                                     psctIDA->usPageLength;
        (psctIDA->ulPageNumber) += (LONG)( (usLines - usPrintLines) /
                                            psctIDA->usPageLength );

        psctIDA->usLineNumber = ( (usLines - usPrintLines) %
                                  psctIDA->usPageLength ) + 1;
        if ( psctIDA->usLineNumber == 0 )
        {
          (psctIDA->usLineNumber)++;
        } /* endif */

        /**************************************************************/
        /*             if the header buffer is at its end             */
        /*        print the formatted header text in psctOther        */
        /*                     to the page buffer                     */
        /**************************************************************/
        if ( usTempRC == QDPR_BUFFER_AT_END )
        {
          usPrintLines = 1;

          /************************************************************/
          /*              print all pages that are full               */
          /************************************************************/
          for ( i = 0; ( i < psctIDA->usPagesProcessed ) &&
                       ( usRC == QDPR_NO_ERROR ); i++ )
          {
            usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                       psctIDA->psctFormatIDA->psctOther, usPrintLines,
                       (USHORT)(usPrintLines + psctIDA->usPageLength), FALSE );

            if ( usRC == QDPR_NO_ERROR )
            {
              usRC = QDPRPrintPageEject( psctIDA,
                                         psctIDA->psctFormatIDA->psctEntry );

              usPrintLines = usPrintLines + psctIDA->usPageLength;
            } /* endif */
          } /* endfor */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*   print the page that is left (i.e. it is not full)    */
            /**********************************************************/
            usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                       psctIDA->psctFormatIDA->psctOther, usPrintLines,
                       (USHORT)(usPrintLines + psctIDA->usLineNumber), TRUE );
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            usRC = QDPRPrintPageEject( psctIDA,
                                       psctIDA->psctFormatIDA->psctEntry );

            psctIDA->usLineNumber = 1;
            psctIDA->usPagesProcessed = 0;
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*      the format buffer is not needed anymore, so       */
            /*     clear it and deallocate the buffer extensions      */
            /**********************************************************/
            usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                          TRUE );
          } /* endif */

          /************************************************************/
          /*                   set new print status                   */
          /************************************************************/
          psctIDA->usNextThreadStatus = QDPR_PRTST_FILTER;
          psctIDA->usPreviousPrintStatus = QDPR_PRTST_START_PAGEHEAD;
        }
        else
        {
          psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_HEADER;
        } /* endif */
      } /* endif */
    } /* endif */

    if ( ( usRC == QDPR_NO_ERROR ) && ( usTempRC != QDPR_BUFFER_AT_END ) )
    {
      /****************************************************************/
      /*                check which tag has been found                */
      /****************************************************************/
      switch ( usTagID )
      {
        case QDPR_ID_CRLF :
          {
            /**********************************************************/
            /*       CRLF has been found, so just increase the        */
            /*        current line number by one, in order to         */
            /*            move processing to the next line            */
            /**********************************************************/
            (psctIDA->usLineNumber)++;

            /**********************************************************/
            /*       check if due to increasing the line number       */
            /*       the page becomes full, if so write a page        */
            /*                      eject string                      */
            /**********************************************************/
            if ( psctIDA->usLineNumber > psctIDA->usPageLength )
            {
              (psctIDA->usPagesProcessed)++;
              (psctIDA->ulPageNumber)++;
              psctIDA->usLineNumber = 1;
            } /* endif */

            psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_HEADER;
          }
        break;
        case QDPR_SYSNAME_FILENAME_ATTR :
        case QDPR_SYSNAME_DICTNAME_ATTR :
        case QDPR_SYSNAME_DATE_ATTR :
        case QDPR_SYSNAME_TIME_ATTR :
        case QDPR_SYSNAME_PAGE_NO_ATTR :
        case QDPR_SYSNAME_PAGE_EJECT_ATTR :
          {
            /**********************************************************/
            /*              Evaluate the system variable              */
            /**********************************************************/
            usRC = QDPREvaluateSysVar( psctIDA, usTagID,
                                       psctIDA->psctCurHeaderFCRT,
                                       usFCRTElement,
                                       psctIDA->psctFormatIDA->psctOther,
                                       &usLines );

            /**********************************************************/
            /*                check if page runs full                 */
            /**********************************************************/
            if ( ( psctIDA->usLineNumber + usLines ) <=
                 psctIDA->usPageLength )
            {
              /********************************************************/
              /*   the page doesn't run full, so just increase the    */
              /*                   number of lines                    */
              /********************************************************/
              (psctIDA->usLineNumber) = (psctIDA->usLineNumber) + usLines;

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_HEADER;
            }
            else
            {
              /********************************************************/
              /*    page becomes full, so calculate the number of     */
              /*   lines that still fit on the page (usPrintLines)    */
              /*    then calculate how many new (full) pages there    */
              /*         will result out of usLines and then          */
              /*            calculate the new line number             */
              /********************************************************/
              usPrintLines = psctIDA->usPageLength
                            - psctIDA->usCaPagefootLines
                            - psctIDA->usLineNumber + 1;
//              usPrintLines = psctIDA->usPageLength -
//                             (psctIDA->usLineNumber - 1);
              (psctIDA->usPagesProcessed)++;
              (psctIDA->ulPageNumber)++;

              psctIDA->usPagesProcessed += (usLines - usPrintLines) /
                                           psctIDA->usPageLength;
              (psctIDA->ulPageNumber) += (LONG)( (usLines - usPrintLines) /
                                                 psctIDA->usPageLength );
              psctIDA->usLineNumber = ( (usLines - usPrintLines) %
                                         psctIDA->usPageLength) + 1;
              if ( psctIDA->usLineNumber == 0 )
              {
                (psctIDA->usLineNumber)++;
              } /* endif */

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_HEADER;
            } /* endif */
          }
        break;
        default :
          {
            /**********************************************************/
            /*         this should not occurr, but anyway set         */
            /*                   QDPR_PROGRAM_ERROR                   */
            /**********************************************************/
            usRC = QDPR_PROGRAM_ERROR;
          }
        break;
      } /* endswitch */
    } /* endif */
  } /* endwhile */

  return( usRC );

} /* end of function QDPRPrintHeader */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintPageEject                                           |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintPageEject( psctIDA, psctTempFormatBuffer )                |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function prints the page eject string to the page buffer.            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA;              // pointer to thread IDA      |
//|  PQDPR_FORMAT_BUFFERS  psctTempFormatBuffer; // pointer to a format        |
//|                                              // buffer area that must      |
//|                                              // be cleared                 |
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
//|  - psctTempFormatBuffer must be cleared                                    |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - none                                                                    |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRPrintPageEject( psctIDA, psctFormatIDA->psctOther );           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF a page eject string exists                                             |
//|    Put the page eject string in the format buffer                          |
//|      (QDPRAddToProcessBuffer)                                              |
//|    Print the formatted string to the page buffer (QDPRPrintToPageBuffer)   |
//|    Clear the used format buffer area again (QDPRClearAndDeallocateBuffer)  |
//|                                                                            |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintPageEject
(
   PQDPR_THREAD          psctIDA,              // pointer to thread IDA
   PQDPR_FORMAT_BUFFERS  psctTempFormatBuffer  // pointer to a format
                                               // buffer area that must
                                               // be cleared
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode



  if ( psctIDA->pszPageEject != NULL  )
  {
    /******************************************************************/
    /*        put the string in the appropriate process buffer        */
    /******************************************************************/
    if ( usRC == QDPR_NO_ERROR )
    {
      usRC = QDPRAddToProcessBuffer( &(psctTempFormatBuffer->psctCurBufExt),
                                     &(psctTempFormatBuffer->pchrLastWritten),
                                     psctIDA->pszPageEject );
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      /****************************************************************/
      /*        print the page eject string to the page buffer        */
      /*            and write the page buffer to the file             */
      /****************************************************************/
      usRC = QDPRPrintToPageBuffer( psctIDA,
                                    psctIDA->psctFormatIDA,
                                    psctTempFormatBuffer,
                                    1, 2, TRUE );
    } /* endif */

    if ( usRC == QDPR_NO_ERROR )
    {
      /****************************************************************/
      /*     clear the temp format buffer area and deallocate its     */
      /*                          extensions                          */
      /****************************************************************/
      usRC = QDPRClearAndDeallocateBuffer(
                                   psctTempFormatBuffer->psctBuffer );

       psctTempFormatBuffer->pchrLastWritten =
                     psctTempFormatBuffer->psctBuffer->achrBuffer;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRPrintPageEject */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintPagefoot                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintPagefoot( psctIDA )                                       |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This functions print the pagefoot part.                                   |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD    psctIDA;       // thread IDA                              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_PROGRAM_ERROR    - internal program error - should not occurr        |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid and filled thread IDA                                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - a lot                                                                   |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  WHILE print status is QDPR_PRTST_PRINT_PAGEFOOT                           |
//|    Print from the pagefoot buffer to the psctIDA->psctFormatIDA->psctOther |
//|      buffer until a system variable is found or CRLF occurred              |
//|      (QDPRPrintUntilTag)                                                   |
//|    IF pagefoot buffer is at the end                                        |
//|      Print from psctOther buffer to the page buffer (QDPRPrintToPageBuffer)|
//|    SWITCH on the tag read by QDPRPrintUntilTag                             |
//|      CASE a CRLF has been read                                             |
//|        Print the first line in psctOther to the page buffer                |
//|          (QDPRPrintToPageBuffer)                                           |
//|      CASE a system variable                                                |
//|        CALL QDPREvaluateSysVar                                             |
//|      CASE a field tag                                                      |
//|        CALL QDPREvaluateFieldTag                                           |
//|      DEFAULT                                                               |
//|        Nothing else should occurr than the tags above so return            |
//|          a QDPR_PROGRAM_ERROR if this happens                              |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintPagefoot
(
  PQDPR_THREAD    psctIDA        // thread IDA
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT         usRC = QDPR_NO_ERROR;           // function returncode
  USHORT         usTagID;                        // ID of read tag
  USHORT         usLines;                        // no of lines read
  USHORT         usFCRTElement;                  // FCRT element number



  /********************************************************************/
  /*         loop as long as the pagefooter is to be printed          */
  /********************************************************************/
  while ( ( psctIDA->usNextThreadStatus == QDPR_PRTST_PRINT_PAGEFOOT ) &&
          ( usRC == QDPR_NO_ERROR ) )
  {
    /********************************************************************/
    /*         print until a system variable or a CRLF occurrs          */
    /********************************************************************/
    usRC = QDPRPrintUntilTag( psctIDA, &(psctIDA->psctCurPagefootExt),
                              &(psctIDA->pchrPagefootBuffer),
                              psctIDA->psctFormatIDA->psctOther,
                              &(psctIDA->psctCurPagefootFCRT),
                              NULL, &usTagID, &usFCRTElement, &usLines );

    /********************************************************************/
    /*                  check if buffer is at the end                   */
    /********************************************************************/
    if ( usRC == QDPR_BUFFER_AT_END )
    {
      usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                 psctIDA->psctFormatIDA->psctOther,
                 (USHORT)(psctIDA->usLinesPrinted + 1),
                 (USHORT)(psctIDA->usLineNumber + 1),
                 TRUE );

      if ( usRC == QDPR_NO_ERROR )
      {
        /****************************************************************/
        /*         the format buffer is not needed anymore, so          */
        /*        clear it and deallocate the buffer extensions         */
        /****************************************************************/
        usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                      TRUE );
      } /* endif */

      /******************************************************************/
      /*                    print page eject string                     */
      /******************************************************************/
      usRC = QDPRPrintPageEject( psctIDA, psctIDA->psctFormatIDA->psctOther );

      (psctIDA->ulPageNumber)++;

      /******************************************************************/
      /*                      set new print status                      */
      /******************************************************************/
      if ( usRC == QDPR_NO_ERROR )
      {
        if ( psctIDA->usPreviousPrintStatus == QDPR_PRTST_START_PAGEHEAD )
        {
          psctIDA->usNextThreadStatus = QDPR_PRTST_FILTER;
        }
        else
        {
          psctIDA->usNextThreadStatus = QDPR_PRTST_START_PAGEHEAD;
        } /* endif */
      } /* endif */
    }
    else
    {
      if ( usRC == QDPR_NO_ERROR )
      {
        /****************************************************************/
        /*                check which tag has been found                */
        /****************************************************************/
        switch ( usTagID )
        {
          case QDPR_ID_CRLF :
            {
              /**********************************************************/
              /*     a CRLF has been found, so print the first line     */
              /*      in the format buffer to the page buffer then      */
              /*        clear the format buffer and reset it and        */
              /*          increase the number of lines printed          */
              /**********************************************************/
              usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                                            psctIDA->psctFormatIDA->psctOther,
                                            1, 2, FALSE );

              if ( usRC == QDPR_NO_ERROR )
              {
                usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                              TRUE );

                (psctIDA->usLineNumber)++;
                (psctIDA->usLinesPrinted)++;
              } /* endif */

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_PAGEFOOT;
            }
          break;
          case QDPR_SYSNAME_FILENAME_ATTR :
          case QDPR_SYSNAME_DICTNAME_ATTR :
          case QDPR_SYSNAME_DATE_ATTR :
          case QDPR_SYSNAME_TIME_ATTR :
          case QDPR_SYSNAME_PAGE_NO_ATTR :
          case QDPR_SYSNAME_PAGE_EJECT_ATTR :
            {
              /**********************************************************/
              /*              Evaluate the system variable              */
              /**********************************************************/
              usRC = QDPREvaluateSysVar( psctIDA, usTagID,
                                         psctIDA->psctCurPagefootFCRT,
                                         usFCRTElement,
                                         psctIDA->psctFormatIDA->psctOther,
                                         &usLines );

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_PAGEFOOT;
            }
          break;
          case QDPR_NAME_ATTR :
            {
              /**********************************************************/
              /*      a field tag has been found, so get the data       */
              /*                     from the field                     */
              /**********************************************************/
              usRC = QDPREvaluateFieldTag( psctIDA,
                                           psctIDA->psctCurPagefootFCRT,
                                           usFCRTElement,
                                           psctIDA->psctFormatIDA->psctOther,
                                           &usLines );

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_PAGEFOOT;
            }
          break;
          default :
            {
              /**********************************************************/
              /*         this should not occurr, but anyway set         */
              /*                   QDPR_PROGRAM_ERROR                   */
              /**********************************************************/
              usRC = QDPR_PROGRAM_ERROR;
            }
          break;
        } /* endswitch */
      } /* endif */
    } /* endif */
  } /* endwhile */

  return( usRC );

} /* end of function QDPRPrintPagefoot */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintPagehead                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintPagehead( psctIDA )                                       |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This functions print the pagehead part.                                   |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD    psctIDA;       // thread IDA                              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_PROGRAM_ERROR    - internal program error - should not occurr        |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid and filled thread IDA                                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - a lot                                                                   |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  WHILE print status is QDPR_PRTST_PRINT_PAGEHEAD                           |
//|    Print from the pagehead buffer to the psctIDA->psctFormatIDA->psctOther |
//|      buffer until a system variable is found or CRLF occurred              |
//|      (QDPRPrintUntilTag)                                                   |
//|    IF pagehead buffer is at the end                                        |
//|      Print from psctOther buffer to the page buffer (QDPRPrintToPageBuffer)|
//|    SWITCH on the tag read by QDPRPrintUntilTag                             |
//|      CASE a CRLF has been read                                             |
//|        Print the first line in psctOther to the page buffer                |
//|          (QDPRPrintToPageBuffer)                                           |
//|      CASE a system variable                                                |
//|        CALL QDPREvaluateSysVar                                             |
//|      CASE a field tag                                                      |
//|        CALL QDPREvaluateFieldTag                                           |
//|      DEFAULT                                                               |
//|        Nothing else should occurr than the tags above so return            |
//|          a QDPR_PROGRAM_ERROR if this happens                              |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintPagehead
(
  PQDPR_THREAD    psctIDA        // thread IDA
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT         usRC = QDPR_NO_ERROR;           // function returncode
  USHORT         usTagID;                        // ID of read tag
  USHORT         usLines;                        // no of lines read
  USHORT         usFCRTElement;                  // FCRT element number



  /********************************************************************/
  /*         loop as long as the pageheader is to be printed          */
  /********************************************************************/
  while ( ( psctIDA->usNextThreadStatus == QDPR_PRTST_PRINT_PAGEHEAD ) &&
          ( usRC == QDPR_NO_ERROR ) )
  {
    /********************************************************************/
    /*         print until a system variable or a CRLF occurrs          */
    /********************************************************************/
    usRC = QDPRPrintUntilTag( psctIDA, &(psctIDA->psctCurPageheadExt),
                              &(psctIDA->pchrPageheadBuffer),
                              psctIDA->psctFormatIDA->psctOther,
                              &(psctIDA->psctCurPageheadFCRT),
                              NULL, &usTagID, &usFCRTElement, &usLines );

    /********************************************************************/
    /*                  check if buffer is at the end                   */
    /********************************************************************/
    if ( usRC == QDPR_BUFFER_AT_END )
    {
      usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                 psctIDA->psctFormatIDA->psctOther,
                 (USHORT)(psctIDA->usLinesPrinted + 1),
                 (USHORT)(psctIDA->usLineNumber + 1),
                 TRUE );

      if ( usRC == QDPR_NO_ERROR )
      {
        /****************************************************************/
        /*         the format buffer is not needed anymore, so          */
        /*        clear it and deallocate the buffer extensions         */
        /****************************************************************/
        usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                      TRUE );
      } /* endif */

      /******************************************************************/
      /*                      set new print status                      */
      /******************************************************************/
      if ( psctIDA->fPrintEntry )
      {
        psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_ENTRY;
      }
      else
      {
        psctIDA->usNextThreadStatus = QDPR_PRTST_START_ENTRY;
      } /* endif */
    }
    else
    {
      if ( usRC == QDPR_NO_ERROR )
      {
        /****************************************************************/
        /*                check which tag has been found                */
        /****************************************************************/
        switch ( usTagID )
        {
          case QDPR_ID_CRLF :
            {
              /**********************************************************/
              /*     a CRLF has been found, so print the first line     */
              /*      in the format buffer to the page buffer then      */
              /*        clear the format buffer and reset it and        */
              /*          increase the number of lines printed          */
              /**********************************************************/
              usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                                            psctIDA->psctFormatIDA->psctOther,
                                            1, 2, FALSE );

              if ( usRC == QDPR_NO_ERROR )
              {
                usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                              TRUE );

                (psctIDA->usLineNumber)++;
                (psctIDA->usLinesPrinted)++;
              } /* endif */

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_PAGEHEAD;
            }
          break;
          case QDPR_SYSNAME_FILENAME_ATTR :
          case QDPR_SYSNAME_DICTNAME_ATTR :
          case QDPR_SYSNAME_DATE_ATTR :
          case QDPR_SYSNAME_TIME_ATTR :
          case QDPR_SYSNAME_PAGE_NO_ATTR :
          case QDPR_SYSNAME_PAGE_EJECT_ATTR :
            {
              /**********************************************************/
              /*              Evaluate the system variable              */
              /**********************************************************/
              usRC = QDPREvaluateSysVar( psctIDA, usTagID,
                                         psctIDA->psctCurPageheadFCRT,
                                         usFCRTElement,
                                         psctIDA->psctFormatIDA->psctOther,
                                         &usLines );

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_PAGEHEAD;
            }
          break;
          case QDPR_NAME_ATTR :
            {
              /**********************************************************/
              /*      a field tag has been found, so get the data       */
              /*                     from the field                     */
              /**********************************************************/
              usRC = QDPREvaluateFieldTag( psctIDA,
                                           psctIDA->psctCurPageheadFCRT,
                                           usFCRTElement,
                                           psctIDA->psctFormatIDA->psctOther,
                                           &usLines );

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_PAGEHEAD;
            }
          break;
          default :
            {
              /**********************************************************/
              /*         this should not occurr, but anyway set         */
              /*                   QDPR_PROGRAM_ERROR                   */
              /**********************************************************/
              usRC = QDPR_PROGRAM_ERROR;
            }
          break;
        } /* endswitch */
      } /* endif */
    } /* endif */
  } /* endwhile */

  return( usRC );

} /* end of function QDPRPrintPagehead */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintToPageBuffer                                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintToPageBuffer( psctIDA, psctFormatIDA, psctStartBuffer,    |
//|                                usStartLine, usStopLine, fMoveToFile )      |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function copies the string that is in                                |
//|  psctStartBuffer->achrBuffer to psctFormatIDA->pszPageBuffer.              |
//|  It will copy the lines from usStartLine till usStopLine (not including).  |
//|  If a NULC is met before usStopLine is encountered the function will       |
//|  also stop copying.                                                        |
//|  E.g. usStartLine = 1, usStopLine = 2 will copy only that part in          |
//|  psctStartBuffer->achrBuffer from the start till the first CRLF            |
//|  (including it).                                                           |
//|  E.g. usStartLine = 2, usStopLine = 4 will copy from the first CRLF        |
//|  (excluding it) till the third CRLF (including it).                        |
//|                                                                            |
//|  If usStartLine could not be found, the function stops.                    |
//|                                                                            |
//|  If the page buffer becomes full it is moved to the print destination      |
//|  file or if fMoveToFile is set to TRUE.                                    |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD         psctIDA;         // pointer to thread IDA            |
//|  PQDPR_FORMAT_IDA     psctFormatIDA;   // pointer to format IDA            |
//|  PQDPR_FORMAT_BUFFERS psctStartBuffer; // pointer to start of format       |
//|                                        // buffer                           |
//|  USHORT               usStartLine;     // Line no to start printing        |
//|                                        // 1 = current (/first) line        |
//|                                        // 2 = second line                  |
//|  USHORT               usStopLine;      // Line no before which             |
//|                                        // printing should stop             |
//|  BOOL                 fMoveToFile;     // TRUE = move the content to       |
//|                                        // the print file before            |
//|                                        // function finishes                |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_ERROR_WRITE_TO_DEST_FILE - error writing to the destination file     |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a correctly set up IDA and format IDA                                   |
//|  - psctStartBuffer->achrBuffer must have been set up to fit in the line    |
//|    length (use QDPRFormatStrToLineLength)                                  |
//|  - usStartLine and usStopLine have to be line numbers that exist in        |
//|    the buffer area (usStartLine <= usStopLine)                             |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Run to CRLF after which printing has to start                             |
//|  WHILE not string is at end and not maximum of lines printed               |
//|    WHILE character read is NOT a LF or a NULC                              |
//|      Read next character                                                   |
//|    IF character read is a LF                                               |
//|      Move on one character                                                 |
//|      Copy string to buffer, but check for a buffer overflow                |
//|      IF buffer is full                                                     |
//|        Put buffer to print destination file (QDPRPrintDestWrite)           |
//|    IF character read is a NULC                                             |
//|      Copy string to buffer, but check for a buffer overflow                |
//|      IF buffer is full                                                     |
//|        Put buffer to print destination file (QDPRPrintDestWrite)           |
//|      IF psctStartBuffer has extensions                                     |
//|        Load the next extension                                             |
//|  IF fMoveToFile is set                                                     |
//|  Print the page buffer to the file (QDPRPrintDestWrite)                    |
//|                                                                            |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintToPageBuffer(

  PQDPR_THREAD         psctIDA,         // pointer to thread IDA
  PQDPR_FORMAT_IDA     psctFormatIDA,   // pointer to format IDA
  PQDPR_FORMAT_BUFFERS psctStartBuffer, // pointer to start of format
                                        // buffer
  USHORT               usStartLine,     // Line no to start printing
                                        // 1 = current (/first) line
                                        // 2 = second line
  USHORT               usStopLine,      // Line no before which
                                        // printing should stop
  BOOL                 fMoveToFile )    // TRUE = move the content to
                                        // the print file before
                                        // function finishes

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  USHORT                i;                        // an index
  USHORT                usLines;                  // no of lines processed
  ULONG                 ulBufFree;                // bytes free in page buffer
  CHAR                  chrReplaced;              // replacement character
  PCHAR                 pchrFirst;                // work pointer to string
                                                  // buffer
  PCHAR                 pchrLast;                 // work pointer to string
                                                  // buffer
  PQDPR_PROCESS_BUFFER  psctTemp;                 // temp buffer pointer



  /********************************************************************/
  /*                       initialize variables                       */
  /*                                                                  */
  /*   psctStartBuffer->psctBuffer->achrBuffer                        */
  /*   |          pchrFirst        pchrLast                           */
  /*   |          |                |                                  */
  /*   |                                                            */
  /*   | +-------------------------------------------------------+    */
  /*   +|....0D0A...............0D0A..........................00|    */
  /*     +-------------------------------------------------------+    */
  /********************************************************************/
  psctStartBuffer->psctCurBufExt = psctStartBuffer->psctBuffer;
  pchrFirst = psctStartBuffer->psctBuffer->achrBuffer;
  psctTemp = psctStartBuffer->psctBuffer;

  /********************************************************************/
  /*                  check if buffer area is empty                   */
  /********************************************************************/
  if ( *pchrFirst != NULC )
  {
    /******************************************************************/
    /*     move to the location from where printing should start      */
    /******************************************************************/
    for ( i = 1; ( i < usStartLine ) && ( usRC == QDPR_NO_ERROR ); i++ )
    {
      while ( ( *pchrFirst != QDPR_LF ) && ( i < usStartLine ) &&
              ( usRC == QDPR_NO_ERROR ) )
      {
        /**************************************************************/
        /*check if the NULC character was found, which means that the */
        /* buffer is at the end and so load another buffer extension  */
        /**************************************************************/
        if ( *pchrFirst == NULC )
        {
          if ( psctStartBuffer->psctCurBufExt->psctBufferExtension != NULL )
          {
            psctStartBuffer->psctCurBufExt =
                psctStartBuffer->psctCurBufExt->psctBufferExtension;
            pchrFirst = psctStartBuffer->psctCurBufExt->achrBuffer;
          }
          else
          {
            /**********************************************************/
            /*   this should not happen, but set i to end the loop    */
            /**********************************************************/
            i = usStartLine;
          } /* endif */
        }
        else
        {
          pchrFirst++;
        } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* a LF has been found, so move pchrFirst one character forward */
      /****************************************************************/
      if ( i < usStartLine )
      {
        pchrFirst++;
      } /* endif */
    } /* endfor */

    /******************************************************************/
    /*  check if the location found is the end of the current buffer  */
    /*        extension, if so load the next buffer extension         */
    /******************************************************************/
    if ( ( *pchrFirst == NULC ) && ( usRC == QDPR_NO_ERROR ) )
    {
      if ( psctStartBuffer->psctCurBufExt->psctBufferExtension != NULL )
      {
        psctStartBuffer->psctCurBufExt =
            psctStartBuffer->psctCurBufExt->psctBufferExtension;
        pchrFirst = psctStartBuffer->psctCurBufExt->achrBuffer;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /*              loop until the string end is reached              */
    /*           or the maximum number of lines is printed            */
    /******************************************************************/
    pchrLast = pchrFirst;
    usLines = usStartLine;
    while ( ( *pchrLast != NULC ) && ( usRC == QDPR_NO_ERROR ) &&
            ( usLines < usStopLine ) )
    {
      /****************************************************************/
      /*               look for the LF or for the NULC                */
      /****************************************************************/
      while ( ( *pchrLast != QDPR_LF ) && ( *pchrLast != NULC ) )
      {
        pchrLast++;
      } /* endwhile */

      /****************************************************************/
      /*      check why the loop stopped, was it a LF or a NULC       */
      /****************************************************************/
      if ( *pchrLast == QDPR_LF )
      {
        pchrLast++;
        usLines++;

        /**************************************************************/
        /*   copy the part from pchrFirst to pchrLast to the print    */
        /*                           buffer                           */
        /*      but check before if the buffer wouldn't overflow      */
        /**************************************************************/
        ulBufFree = psctFormatIDA->ulSizePageBuffer -
                    (LONG)( psctFormatIDA->pchrLastChar -
                            psctFormatIDA->pszPageBuffer - 1 );

        if ( ulBufFree < (ULONG)( pchrLast - pchrFirst ) )
        {
          /************************************************************/
          /*  not enough space, so copy that part of the page buffer  */
          /*                 that has completed lines                 */
          /*  therefore replace the character under pchrLastLF with   */
          /*   NULC, write it to the destination file and then move   */
          /*    the rest left in the page buffer to the front and     */
          /*           copy the string into the page buffer           */
          /************************************************************/
          chrReplaced = *( psctFormatIDA->pchrLastLF );
          *( psctFormatIDA->pchrLastLF ) = NULC;

          usRC = QDPRPrintDestWrite( psctIDA->psctPrintDest,
                                     psctFormatIDA->pszPageBuffer,
                                     &(psctIDA->usDosRC) );
          *( psctFormatIDA->pchrLastLF ) = chrReplaced;

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*     move the rest in the page buffer to the front      */
            /**********************************************************/
            memmove( psctFormatIDA->pszPageBuffer,
                     psctFormatIDA->pchrLastLF,
                     ( psctFormatIDA->pchrLastChar -
                       psctFormatIDA->pchrLastLF ) );

            /**********************************************************/
            /*                    set the pointers                    */
            /**********************************************************/
            psctFormatIDA->pchrLastChar = psctFormatIDA->pszPageBuffer +
                                          ( psctFormatIDA->pchrLastChar -
                                            psctFormatIDA->pchrLastLF );
            psctFormatIDA->pchrLastLF = psctFormatIDA->pszPageBuffer;

            /**********************************************************/
            /*                    clear the buffer                    */
            /**********************************************************/
            memset( psctFormatIDA->pchrLastChar, NULC,
                    (USHORT)psctFormatIDA->ulSizePageBuffer -
                    ( psctFormatIDA->pchrLastChar -
                      psctFormatIDA->pchrLastLF ) );
          } /* endif */
        } /* endif */

        /**************************************************************/
        /*         copy everything from pchrFirst to pchrLast         */
        /**************************************************************/
        if ( usRC == QDPR_NO_ERROR )
        {
          memcpy( psctFormatIDA->pchrLastChar, pchrFirst,
                  ( pchrLast - pchrFirst ) );
          (psctFormatIDA->pchrLastChar) += ( pchrLast - pchrFirst );
          psctFormatIDA->pchrLastLF = psctFormatIDA->pchrLastChar;
          pchrFirst = pchrLast;
        } /* endif */
      } /* endif */

      if ( ( *pchrLast == NULC ) && ( usRC == QDPR_NO_ERROR ) )
      {
        /**************************************************************/
        /*         write the current text (between pchrFirst          */
        /*              and pchrLast to the page buffer)              */
        /*        check if a page buffer overflow would occurr        */
        /**************************************************************/
        ulBufFree = psctFormatIDA->ulSizePageBuffer -
                    (LONG)( psctFormatIDA->pchrLastChar -
                            psctFormatIDA->pszPageBuffer - 1 );

        if ( ulBufFree < (ULONG)( pchrLast - pchrFirst ) )
        {
          /************************************************************/
          /*  not enough space, so copy that part of the page buffer  */
          /*                 that has completed lines                 */
          /*  therefore replace the character under pchrLastLF with   */
          /*   NULC, write it to the destination file and then move   */
          /*    the rest left in the page buffer to the front and     */
          /*           copy the string into the page buffer           */
          /************************************************************/
          chrReplaced = *( psctFormatIDA->pchrLastLF );
          *( psctFormatIDA->pchrLastLF ) = NULC;

          usRC = QDPRPrintDestWrite( psctIDA->psctPrintDest,
                                     psctFormatIDA->pszPageBuffer,
                                     &(psctIDA->usDosRC) );
          *( psctFormatIDA->pchrLastLF ) = chrReplaced;

          if ( usRC == QDPR_NO_ERROR )
          {
            /**********************************************************/
            /*     move the rest in the page buffer to the front      */
            /**********************************************************/
            memmove( psctFormatIDA->pszPageBuffer,
                     psctFormatIDA->pchrLastLF,
                     ( psctFormatIDA->pchrLastChar -
                       psctFormatIDA->pchrLastLF ) );

            /**********************************************************/
            /*                    set the pointers                    */
            /**********************************************************/
            psctFormatIDA->pchrLastChar = psctFormatIDA->pszPageBuffer +
                                          ( psctFormatIDA->pchrLastChar -
                                            psctFormatIDA->pchrLastLF );
            psctFormatIDA->pchrLastLF = psctFormatIDA->pszPageBuffer;

            /**********************************************************/
            /*                    clear the buffer                    */
            /**********************************************************/
            memset( psctFormatIDA->pchrLastChar, NULC,
                    (USHORT)psctFormatIDA->ulSizePageBuffer -
                    ( psctFormatIDA->pchrLastChar -
                      psctFormatIDA->pchrLastLF ) );
          } /* endif */
        } /* endif */

        /**************************************************************/
        /*         copy everything from pchrFirst to pchrLast         */
        /**************************************************************/
        if ( usRC == QDPR_NO_ERROR )
        {
          memcpy( psctFormatIDA->pchrLastChar, pchrFirst,
                  ( pchrLast - pchrFirst ) );
          (psctFormatIDA->pchrLastChar) += ( pchrLast - pchrFirst );
        } /* endif */

        /**************************************************************/
        /*    check if buffer area is finished or if only the next    */
        /*                 extension has to be loaded                 */
        /**************************************************************/
        if ( ( psctStartBuffer->psctCurBufExt->psctBufferExtension != NULL ) &&
             ( usRC == QDPR_NO_ERROR ) )
        {
          /************************************************************/
          /*               now load the next extension                */
          /************************************************************/
          psctStartBuffer->psctCurBufExt =
                     psctStartBuffer->psctCurBufExt->psctBufferExtension;
          pchrFirst = psctStartBuffer->psctCurBufExt->achrBuffer;
          pchrLast = pchrFirst;
        } /* endif */
      } /* endif */
    } /* endwhile */
  } /* endif */

  if ( ( usRC == QDPR_NO_ERROR ) && fMoveToFile )
  {
    /******************************************************************/
    /*   if no error occurred and the page buffer shall be moved to   */
    /*                     the file, move it then                     */
    /******************************************************************/
    usRC = QDPRPrintDestWrite( psctIDA->psctPrintDest,
                               psctFormatIDA->pszPageBuffer,
                               &(psctIDA->usDosRC) );

    /******************************************************************/
    /*    buffer has been written to file, so clear it and set the    */
    /*                two pointers to the page buffer                 */
    /******************************************************************/
    if ( usRC == QDPR_NO_ERROR )
    {
      memset( psctFormatIDA->pszPageBuffer, NULC,
              (USHORT)psctFormatIDA->ulSizePageBuffer );
      psctFormatIDA->pchrLastChar = psctFormatIDA->pszPageBuffer;
      psctFormatIDA->pchrLastLF = psctFormatIDA->pchrLastChar;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function PrintToPageBuffer */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintTrailer                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintTrailer( psctIDA )                                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function prints the trailer part.                                    |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD    psctIDA;       // thread IDA                              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_PROGRAM_ERROR    - internal program error - should not occurr        |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid and filled thread IDA                                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - a lot                                                                   |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  WHILE print status is QDPR_PRTST_PRINT_TRAILER                            |
//|    Print from the trailer buffer to the psctIDA->psctFormatIDA->psctOther  |
//|      buffer until a system variable is found or CRLF occurred              |
//|      (QDPRPrintUntilTag)                                                   |
//|    IF current page doesn't become full                                     |
//|      Increase the number of lines in psctOther                             |
//|      IF trailer buffer is at the end                                       |
//|        Print the full psctOther buffer to the page buffer                  |
//|    ELSE                                                                    |
//|      Calculate how many pages are in the psctOther buffer                  |
//|      IF trailer buffer is at the end                                       |
//|        Print the full psctOther buffer to the page buffer                  |
//|    SWITCH on the tag read by QDPRPrintUntilTag                             |
//|      CASE a CRLF has been read                                             |
//|        Increase the number of lines in psctOther                           |
//|      CASE a system variable                                                |
//|        CALL QDPREvaluateSysVar                                             |
//|      DEFAULT                                                               |
//|        Nothing else should occurr than the tags above so return            |
//|          a QDPR_PROGRAM_ERROR if this happens                              |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintTrailer
(
  PQDPR_THREAD    psctIDA        // thread IDA
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT         usRC = QDPR_NO_ERROR;           // function returncode
  USHORT         usTempRC = 0;                       // temp returncode
  USHORT         usTagID;                        // ID of read tag
  USHORT         usLines;                        // no of lines read
  USHORT         usPrintLines;                   // no of lines to print
  USHORT         usFCRTElement;                  // FCRT element number
  USHORT         i;                              // an index



  /********************************************************************/
  /*           loop as long as the trailer is to be printed           */
  /********************************************************************/
  while ( ( psctIDA->usNextThreadStatus == QDPR_PRTST_PRINT_TRAILER ) &&
          ( usRC == QDPR_NO_ERROR ) )
  {
    /********************************************************************/
    /*         print until a system variable or a CRLF occurrs          */
    /********************************************************************/
    usRC = QDPRPrintUntilTag( psctIDA, &(psctIDA->psctCurTrailerExt),
                              &(psctIDA->pchrTrailerBuffer),
                              psctIDA->psctFormatIDA->psctOther,
                              &(psctIDA->psctCurTrailerFCRT),
                              NULL, &usTagID, &usFCRTElement, &usLines );

    /********************************************************************/
    /*        check if returncode is ok or buffer is at the end         */
    /********************************************************************/
    if ( ( usRC == QDPR_NO_ERROR ) || ( usRC == QDPR_BUFFER_AT_END ) )
    {
      /******************************************************************/
      /*      save current returncode, because it is needed later       */
      /******************************************************************/
      usTempRC = usRC;
      usRC = QDPR_NO_ERROR;

      /******************************************************************/
      /*             check if the current page becomes full             */
      /******************************************************************/
      if ( ( psctIDA->usLineNumber + usLines ) <= psctIDA->usPageLength )
      {
        /****************************************************************/
        /*       the page doesn't run full, so just increase the        */
        /*                       number of lines                        */
        /****************************************************************/
        (psctIDA->usLineNumber) = (psctIDA->usLineNumber) + usLines;

        /****************************************************************/
        /*             if the trailer buffer is at its end              */
        /*        print the formatted trailer text in psctOther         */
        /*                      to the page buffer                      */
        /****************************************************************/
        if ( usTempRC == QDPR_BUFFER_AT_END )
        {
          usPrintLines = 1;

          /**************************************************************/
          /*               print all pages that are full                */
          /**************************************************************/
          for ( i = 0; ( i < psctIDA->usPagesProcessed ) &&
                       ( usRC == QDPR_NO_ERROR ); i++ )
          {
            usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                       psctIDA->psctFormatIDA->psctOther, usPrintLines,
                       (USHORT)(usPrintLines + psctIDA->usPageLength), FALSE );

            if ( usRC == QDPR_NO_ERROR )
            {
              usRC = QDPRPrintPageEject( psctIDA,
                                         psctIDA->psctFormatIDA->psctEntry );

              usPrintLines = usPrintLines + psctIDA->usPageLength;
            } /* endif */
          } /* endfor */

          if ( usRC == QDPR_NO_ERROR )
          {
            /************************************************************/
            /*    print the page that is left (i.e. it is not full)     */
            /************************************************************/
            usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                       psctIDA->psctFormatIDA->psctOther, usPrintLines,
                       (USHORT)(usPrintLines + psctIDA->usLineNumber), TRUE );
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            usRC = QDPRPrintPageEject( psctIDA,
                                       psctIDA->psctFormatIDA->psctEntry );

            psctIDA->usLineNumber = 1;
            psctIDA->usPagesProcessed = 0;
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /************************************************************/
            /*       the format buffer is not needed anymore, so        */
            /*      clear it and deallocate the buffer extensions       */
            /************************************************************/
            usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                          FALSE );
          } /* endif */

          /**************************************************************/
          /*                    set new print status                    */
          /**************************************************************/
          psctIDA->usNextThreadStatus = QDPR_PRTST_FINISHED;
        }
        else
        {
          psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_TRAILER;
        } /* endif */
      }
      else
      {
        /****************************************************************/
        /*        page becomes full, so calculate the number of         */
        /*       lines that still fit on the page (usPrintLines)        */
        /*        then calculate how many new (full) pages there        */
        /*             will result out of usLines and then              */
        /*                calculate the new line number                 */
        /****************************************************************/
        /**************************************************************/
        /* trailer has no footnote, so do not user usCaPagefootlines  */
        /**************************************************************/
//        usPrintLines = psctIDA->usPageLength - (psctIDA->usLineNumber - 1);
        usPrintLines = psctIDA->usPageLength  -
                     psctIDA->usLineNumber + 1;
        (psctIDA->usPagesProcessed)++;
        (psctIDA->ulPageNumber)++;

        psctIDA->usPagesProcessed += (usLines - usPrintLines) /
                                     psctIDA->usPageLength;
        (psctIDA->ulPageNumber) += (LONG)( (usLines - usPrintLines) /
                                           psctIDA->usPageLength );

        psctIDA->usLineNumber = ( (usLines - usPrintLines) %
                                  psctIDA->usPageLength ) + 1;
        if ( psctIDA->usLineNumber == 0 )
        {
          (psctIDA->usLineNumber)++;
        } /* endif */

        /****************************************************************/
        /*             if the trailer buffer is at its end              */
        /*        print the formatted trailer text in psctOther         */
        /*                      to the page buffer                      */
        /****************************************************************/
        if ( usTempRC == QDPR_BUFFER_AT_END )
        {
          usPrintLines = 1;

          /**************************************************************/
          /*               print all pages that are full                */
          /**************************************************************/
          for ( i = 0; ( i < psctIDA->usPagesProcessed ) &&
                       ( usRC == QDPR_NO_ERROR ); i++ )
          {
            usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                       psctIDA->psctFormatIDA->psctOther, usPrintLines,
                       (USHORT)(usPrintLines + psctIDA->usPageLength), FALSE );

            if ( usRC == QDPR_NO_ERROR )
            {
              usRC = QDPRPrintPageEject( psctIDA,
                                         psctIDA->psctFormatIDA->psctEntry );

              usPrintLines = usPrintLines + psctIDA->usPageLength;
            } /* endif */
          } /* endfor */

          if ( usRC == QDPR_NO_ERROR )
          {
            /************************************************************/
            /*    print the page that is left (i.e. it is not full)     */
            /************************************************************/
            usRC = QDPRPrintToPageBuffer( psctIDA, psctIDA->psctFormatIDA,
                       psctIDA->psctFormatIDA->psctOther, usPrintLines,
                       (USHORT)(usPrintLines + psctIDA->usLineNumber), TRUE );
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            usRC = QDPRPrintPageEject( psctIDA,
                                       psctIDA->psctFormatIDA->psctEntry );

            psctIDA->usLineNumber = 1;
            psctIDA->usPagesProcessed = 0;
          } /* endif */

          if ( usRC == QDPR_NO_ERROR )
          {
            /************************************************************/
            /*       the format buffer is not needed anymore, so        */
            /*      clear it and deallocate the buffer extensions       */
            /************************************************************/
            usRC = QDPRResetFormatBuffer( psctIDA->psctFormatIDA->psctOther,
                                          FALSE );
          } /* endif */

          /**************************************************************/
          /*                    set new print status                    */
          /**************************************************************/
          psctIDA->usNextThreadStatus = QDPR_PRTST_FINISHED;
        }
        else
        {
          psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_TRAILER;
        } /* endif */
      } /* endif */
    } /* endif */

    if ( ( usRC == QDPR_NO_ERROR ) && ( usTempRC != QDPR_BUFFER_AT_END ) )
    {
      /******************************************************************/
      /*                 check which tag has been found                 */
      /******************************************************************/
      switch ( usTagID )
      {
        case QDPR_ID_CRLF :
          {
            /************************************************************/
            /*        CRLF has been found, so just increase the         */
            /*         current line number by one, in order to          */
            /*             move processing to the next line             */
            /************************************************************/
            (psctIDA->usLineNumber)++;

            /************************************************************/
            /*        check if due to increasing the line number        */
            /*        the page becomes full, if so write a page         */
            /*                       eject string                       */
            /************************************************************/
            if ( psctIDA->usLineNumber > psctIDA->usPageLength )
            {
              (psctIDA->usPagesProcessed)++;
              (psctIDA->ulPageNumber)++;
              psctIDA->usLineNumber = 1;
            } /* endif */

            psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_TRAILER;
          }
        break;
        case QDPR_SYSNAME_FILENAME_ATTR :
        case QDPR_SYSNAME_DICTNAME_ATTR :
        case QDPR_SYSNAME_DATE_ATTR :
        case QDPR_SYSNAME_TIME_ATTR :
        case QDPR_SYSNAME_PAGE_NO_ATTR :
        case QDPR_SYSNAME_PAGE_EJECT_ATTR :
          {
            /************************************************************/
            /*               Evaluate the system variable               */
            /************************************************************/
            usRC = QDPREvaluateSysVar( psctIDA, usTagID,
                                       psctIDA->psctCurTrailerFCRT,
                                       usFCRTElement,
                                       psctIDA->psctFormatIDA->psctOther,
                                       &usLines );

            /************************************************************/
            /*                 check if page runs full                  */
            /************************************************************/
            if ( ( psctIDA->usLineNumber + usLines ) <= psctIDA->usPageLength )
            {
              /**********************************************************/
              /*    the page doesn't run full, so just increase the     */
              /*                    number of lines                     */
              /**********************************************************/
              (psctIDA->usLineNumber) = (psctIDA->usLineNumber) + usLines;

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_TRAILER;
            }
            else
            {
              /**********************************************************/
              /*     page becomes full, so calculate the number of      */
              /*    lines that still fit on the page (usPrintLines)     */
              /*     then calculate how many new (full) pages there     */
              /*          will result out of usLines and then           */
              /*             calculate the new line number              */
              /**********************************************************/
              usPrintLines = psctIDA->usPageLength
                             - psctIDA->usCaPagefootLines
                             - psctIDA->usLineNumber + 1;
//              usPrintLines = psctIDA->usPageLength - (psctIDA->usLineNumber - 1);
              (psctIDA->usPagesProcessed)++;
              (psctIDA->ulPageNumber)++;

              psctIDA->usPagesProcessed += (usLines - usPrintLines) /
                                           psctIDA->usPageLength;
              (psctIDA->ulPageNumber) += (LONG)( (usLines - usPrintLines) /
                                                 psctIDA->usPageLength );
              psctIDA->usLineNumber = ( (usLines - usPrintLines) %
                                         psctIDA->usPageLength) + 1;
              if ( psctIDA->usLineNumber == 0 )
              {
                (psctIDA->usLineNumber)++;
              } /* endif */

              psctIDA->usNextThreadStatus = QDPR_PRTST_PRINT_TRAILER;
            } /* endif */
          }
        break;
        default :
          {
            /************************************************************/
            /*          this should not occurr, but anyway set          */
            /*                    QDPR_PROGRAM_ERROR                    */
            /************************************************************/
            usRC = QDPR_PROGRAM_ERROR;
          }
        break;
      } /* endswitch */
    } /* endif */
  } /* endwhile */

  return( usRC );

} /* end of function QDPRPrintTrailer */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintUntilTag                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintUntilTag( psctIDA, ppsctCurBufferExt, ppchrBuffer,        |
//|                            psctCurBuf, ppsctFieldFCRT, ppsctRepeatFCRT,    |
//|                            pusTagID, pusFCRTElement, pusLines )            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function prints the characters from the ppchrBuffer to the           |
//|  current buffer area in the format IDA until a <field> tag, a <repeat> tag,|
//|  a </repeat> tag or a system variable occurrs or until the buffer          |
//|  and all its extensions has reached the end.                               |
//|                                                                            |
//|  If a tag or a system variable is found the function scans the             |
//|  corresponding FCRT and returns the element number of the FCRT element     |
//|  in the current FCRT extension (which may be updated if required).         |
//|  The appropriate tag identifier is returned as well.                       |
//|                                                                            |
//|  If the buffer is at the end, the function has a returncode of             |
//|  QDPR_BUFFER_AT_END and everything from the buffer is printed to the       |
//|  current buffer area in the format IDA.                                    |
//|                                                                            |
//|  If a CRLF is found everything (including the CRLF) is printed to the      |
//|  string buffer and the tag ID is QDPR_ID_CRLF.                             |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD          psctIDA;            // pointer to thread IDA        |
//|  PQDPR_PROCESS_BUFFER  *ppsctCurBufferExt; // pointer to the current       |
//|                                            // process buffer extension     |
//|  PCHAR                 *ppchrBuffer;       // pointer to the currrent      |
//|                                            // work location in the         |
//|                                            // process buffer               |
//|  PQDPR_FORMAT_BUFFER   psctCurBuf;         // current format buffer used   |
//|                                            // for printing into            |
//|  PQDPR_FCRT            *ppsctFieldFCRT;    // current <field> and          |
//|                                            // system variables FCRT        |
//|                                            // (must be always supplied)    |
//|  PQDPR_FCRT            *ppsctRepeatFCRT;   // current <repeat> tag FCRT    |
//|                                            // (must be NULL if not         |
//|                                            // <entry> is processed         |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PUSHORT               pusTagID;           // tag ID of tag found          |
//|  PUSHORT               pusFCRTElement;     // FCRT element number          |
//|                                            // of current FCRT              |
//|                                            // in which the corresponding   |
//|                                            // buffer location is found     |
//|  PUSHORT               pusLines;           // No of lines put in the       |
//|                                            // string buffer of the         |
//|                                            // format IDA                   |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_BUFFER_AT_END    - the buffer is printed to the end                  |
//|  QDPR_PROGRAM_ERROR    - this should not occurr otherwise it is a          |
//|                          programming error                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - correctly setup thread IDA, format IDA, FCRTs                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - if the corresponding extension are needed ppsctCurBufExt, ppchrBuffer,  |
//|    ppsctFieldFCRT and ppsctRepeatFCRT are updated                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  WHILE no tag has been found                                               |
//|    SWITCH on the character read                                            |
//|      CASE NULC                                                             |
//|        IF buffer is at end                                                 |
//|          Print everything from ppchrBuffer to the NULC to the string       |
//|            buffer                                                          |
//|          Load the next extension of the buffer                             |
//|          IF the extension doesn't exist                                    |
//|            RETURN QDPR_BUFFER_AT_END                                       |
//|        ELSE                                                                |
//|          Read next character                                               |
//|          IF buffer is at end                                               |
//|            Print everything from ppchrBuffer to the NULC to the string     |
//|              buffer                                                        |
//|            Load the next extension of the buffer                           |
//|            IF the extension doesn't exist                                  |
//|              RETURN QDPR_BUFFER_AT_END                                     |
//|          SWITCH on the next character                                      |
//|            CASE 'f'                                                        |
//|            CASE '$'                                                        |
//|              Scan the field tag FCRT for the location and return           |
//|                tag ID and element number                                   |
//|            CASE 'r'                                                        |
//|              Scan the repeat tag FCRT for the location and return          |
//|                tag ID and element number                                   |
//|            CASE '/'                                                        |
//|              Return tag ID of </repeat> tag                                |
//|            CASE NULC                                                       |
//|              RETURN QDPR_BUFFER_AT_END                                     |
//|      CASE LF                                                               |
//|      DEFAULT                                                               |
//|        Read next character                                                 |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintUntilTag
(
  PQDPR_THREAD          psctIDA,            // pointer to thread IDA
  PQDPR_PROCESS_BUFFER  *ppsctCurBufferExt, // pointer to the current
                                            // process buffer extension
  PCHAR                 *ppchrBuffer,       // pointer to the currrent
                                            // work location in the
                                            // process buffer
  PQDPR_FORMAT_BUFFERS   psctCurBuf,        // current format buffer used
                                            // for printing into
  PQDPR_FCRT            *ppsctFieldFCRT,    // current <field> and
                                            // system variables FCRT
                                            // (must be always supplied)
  PQDPR_FCRT            *ppsctRepeatFCRT,   // current <repeat> tag FCRT
                                            // (must be NULL if not
                                            // <entry> is processed
  PUSHORT               pusTagID,           // tag ID of tag found
  PUSHORT               pusFCRTElement,     // FCRT element number
                                            // of current FCRT
                                            // in which the corresponding
                                            // buffer location is found
  PUSHORT               pusLines            // No of lines put in the
                                            // string buffer of the
                                            // format IDA
)

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  USHORT                usLines;                  // temp line numbers
  PSZ                   pszTemp;                  // temp buffer area
  PCHAR                 pchrTest;                 // pointer to walk
                                                  // along the buffer
  PCHAR                 pchrOldTest;              // old test pointer
  PCHAR                 pchrFCRT;                 // field start pointer
  CHAR                  chrReplaced;              // which character has
                                                  // been replaced for
                                                  // the '\0'
  BOOL                  fFound = FALSE;           // found ?
  BOOL                  fFCRTElementFound;        // FCRT element found ?



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pusLines = 0;

  /********************************************************************/
  /*     loop till a tag or a CRLF has been found, or the buffer      */
  /*                          is at the end                           */
  /********************************************************************/
  pchrTest = *ppchrBuffer;
  while ( !fFound && ( usRC == QDPR_NO_ERROR ) )
  {
    switch ( *pchrTest )
    {
      case NULC :
        {
          /************************************************************/
          /*  check if the buffer in the current buffer extension is  */
          /*   at the end, if so get the next buffer extension and    */
          /*  check if the current line has been split by the buffer  */
          /*   extensions, if so run to the end of the line and put   */
          /*   thw two parts in pszTemp and then format pszTemp to    */
          /*                 fit into the linelength                  */
          /************************************************************/
          if ( ( pchrTest - (*ppsctCurBufferExt)->achrBuffer ) ==
               QDPR_MAX_PROCESS_BUFFER - 1 )
          {
            /**********************************************************/
            /*    now load the next buffer extension, if one exits    */
            /*            if not return QDPR_BUFFER_AT_END            */
            /**********************************************************/
            if ( (*ppsctCurBufferExt)->psctBufferExtension != NULL )
            {
              pchrOldTest = *ppchrBuffer;

              (*ppsctCurBufferExt) =
                (*ppsctCurBufferExt)->psctBufferExtension;
              (*ppchrBuffer) = (*ppsctCurBufferExt)->achrBuffer;

              pchrTest = (*ppsctCurBufferExt)->achrBuffer;

              /********************************************************/
              /*              run to the end of the line              */
              /********************************************************/
              while ( ( *pchrTest != QDPR_LF ) && ( *pchrTest != NULC ) )
              {
                pchrTest++;
              } /* endwhile */

              /********************************************************/
              /*  allocate storage to hold the text from pchrOldTest  */
              /*   to pchrTest (which is now in the new extension)    */
              /********************************************************/
              if ( UtlAlloc( (PVOID *) &pszTemp, 0L,
                             (LONG)( strlen( pchrOldTest ) +
                                     pchrTest - *ppchrBuffer + 1 ),
                             NOMSG ) )
              {
                /******************************************************/
                /*    copy the part of the first extension and the    */
                /*      part of the second extension to the temp      */
                /*                    buffer area                     */
                /******************************************************/
                strcpy( pszTemp, pchrOldTest );
                memcpy( &(pszTemp[strlen(pchrOldTest)]), *ppchrBuffer,
                        pchrTest - *ppchrBuffer );

                /******************************************************/
                /*    and now format the string in the temp buffer    */
                /*               to fit the linelength                */
                /******************************************************/
                usRC = QDPRFormatStrToLineLength( psctIDA,
                                                  psctIDA->psctFormatIDA,
                                                  pszTemp, psctCurBuf,
                                                  &usLines );
                if ( usRC == QDPR_NO_ERROR )
                {
                  *ppchrBuffer = pchrTest;
                  (*pusLines) = (*pusLines) + usLines;
                } /* endif */

                /******************************************************/
                /*               deallocate temp buffer               */
                /******************************************************/
                UtlAlloc( (PVOID *) &pszTemp, 0L, 0L, NOMSG );
              }
              else
              {
                usRC = QDPR_NO_MEMORY;
              } /* endif */
            }
            else
            {
              usRC = QDPRFormatStrToLineLength( psctIDA,
                                                psctIDA->psctFormatIDA,
                                                *ppchrBuffer, psctCurBuf,
                                                &usLines );
              if ( usRC == QDPR_NO_ERROR )
              {
                (*pusLines) = (*pusLines) + usLines;
                usRC = QDPR_BUFFER_AT_END;
              } /* endif */
            } /* endif */
          }
          else
          {
            /**********************************************************/
            /*    now check if the NULC is the beginning of one of    */
            /*   the field, repeat or system variables replace tags   */
            /*   but before check if maybe the buffer is at the end   */
            /*    and the next identifier is in the next extension    */
            /*          but remember the old pchrTest value           */
            /*      anyway write the string to the string buffer      */
            /**********************************************************/
            pchrOldTest = pchrTest;
            pchrTest++;

            usRC = QDPRFormatStrToLineLength( psctIDA,
                                              psctIDA->psctFormatIDA,
                                              *ppchrBuffer, psctCurBuf,
                                              &usLines );
            if ( usRC == QDPR_NO_ERROR )
            {
              (*pusLines) = (*pusLines) + usLines;
            } /* endif */

            if ( usRC == QDPR_NO_ERROR )
            {
              /********************************************************/
              /*    check the case if the '\0' of the replaced tag    */
              /*   is in the first extension of the buffer and the    */
              /*   identifier ('f', 'r', '/r' or '$' is in the next   */
              /*                      extension                       */
              /********************************************************/
              if ( ( pchrTest - (*ppsctCurBufferExt)->achrBuffer ) ==
                   QDPR_MAX_PROCESS_BUFFER - 1 )
              {
                /******************************************************/
                /*  now load the next buffer extension, if one exits  */
                /*          if not return QDPR_BUFFER_AT_END          */
                /******************************************************/
                if ( (*ppsctCurBufferExt)->psctBufferExtension != NULL )
                {
                  (*ppsctCurBufferExt) =
                    (*ppsctCurBufferExt)->psctBufferExtension;
                  (*ppchrBuffer) = (*ppsctCurBufferExt)->achrBuffer;
                  pchrTest = (*ppsctCurBufferExt)->achrBuffer;
                }
                else
                {
                  usRC = QDPR_BUFFER_AT_END;
                } /* endif */
              } /* endif */

              if ( usRC == QDPR_NO_ERROR )
              {
                switch ( *pchrTest )
                {
                  case 'f' :
                  case '$' :
                    {
                      /************************************************/
                      /*  a field tag or a system variable has been   */
                      /*     found, so run through the field FCRT     */
                      /************************************************/
                      fFound = TRUE;
                      (*ppchrBuffer) = pchrTest + 1;

                      *pusFCRTElement = 0;
                      fFCRTElementFound = FALSE;

                      while ( !fFCRTElementFound &&
                              ( usRC == QDPR_NO_ERROR ) )
                      {
                        /**********************************************/
                        /* check if the same buffer location pointer  */
                        /*               has been found               */
                        /**********************************************/
                        pchrFCRT = (*ppsctFieldFCRT)->
                            asctFCRTElements[(*pusFCRTElement)].pchrFieldStart;

                        if ( pchrOldTest == pchrFCRT )
                        {
                          fFCRTElementFound = TRUE;
                          *pusTagID = (*ppsctFieldFCRT)->
                              asctFCRTElements[(*pusFCRTElement)].usTagID;
                        }
                        else
                        {
                          (*pusFCRTElement)++;

                          /********************************************/
                          /* check if the FCRT is at the end, so load */
                          /*               the next one               */
                          /********************************************/
                          if ( *pusFCRTElement ==
                               (*ppsctFieldFCRT)->usElements )
                          {
                            if ( (*ppsctFieldFCRT)->psctFCRTExtension
                                 != NULL )
                            {
                              (*ppsctFieldFCRT) =
                                (*ppsctFieldFCRT)->psctFCRTExtension;
                              *pusFCRTElement = 0;
                            }
                            else
                            {
                              /****************************************/
                              /*    this should not happen, but is    */
                              /*        coded to end the loop         */
                              /****************************************/
                              usRC = QDPR_PROGRAM_ERROR;
                            } /* endif */
                          } /* endif */
                        } /* endif */
                      } /* endwhile */
                    }
                  break;
                  case 'r' :
                    {
                      /************************************************/
                      /*         a repeat tag has been found          */
                      /*        so run through the repeat FCRT        */
                      /************************************************/
                      fFound = TRUE;
                      (*ppchrBuffer) = pchrTest + 1;

                      *pusFCRTElement = 0;
                      fFCRTElementFound = FALSE;

                      while ( !fFCRTElementFound &&
                              ( usRC == QDPR_NO_ERROR ) )
                      {
                        /**********************************************/
                        /* check if the same buffer location pointer  */
                        /*               has been found               */
                        /**********************************************/
                        pchrFCRT = (*ppsctRepeatFCRT)->
                            asctFCRTElements[(*pusFCRTElement)].pchrFieldStart;

                        if ( pchrOldTest == pchrFCRT )
                        {
                          fFCRTElementFound = TRUE;
                          *pusTagID = QDPR_REPEAT_TOKEN;
                        }
                        else
                        {
                          (*pusFCRTElement)++;

                          /********************************************/
                          /* check if the FCRT is at the end, so load */
                          /*               the next one               */
                          /********************************************/
                          if ( *pusFCRTElement ==
                               (*ppsctRepeatFCRT)->usElements )
                          {
                            if ( (*ppsctRepeatFCRT)->psctFCRTExtension
                                 != NULL )
                            {
                              (*ppsctRepeatFCRT) =
                                (*ppsctRepeatFCRT)->psctFCRTExtension;
                              *pusFCRTElement = 0;
                            }
                            else
                            {
                              /****************************************/
                              /*    this should not happen, but is    */
                              /*        coded to end the loop         */
                              /****************************************/
                              usRC = QDPR_PROGRAM_ERROR;
                            } /* endif */
                          } /* endif */
                        } /* endif */
                      } /* endwhile */
                    }
                  break;
                  case '/' :
                    {
                      fFound = TRUE;
                      (*ppchrBuffer) = pchrTest + 2;
                      *pusTagID = QDPR_REPEAT_ETOKEN;
                    }
                  break;
                  case NULC :
                    {
                      /************************************************/
                      /*  another NULC has been found, so the buffer  */
                      /*  is at the logical but not at the physical   */
                      /*       end, but anyway he IS at the end       */
                      /************************************************/
                      usRC = QDPR_BUFFER_AT_END;
                    }
                  break;
                } /* endswitch */
              } /* endif */
            } /* endif */
          } /* endif */
        }
      break;
      case QDPR_LF :
        {
          /************************************************************/
          /*  get over the LF and replace the next character with a   */
          /*   NULC (to form a PSZ which is to be put in the string   */
          /*       buffer), but remember the replaced character       */
          /************************************************************/
          fFound = TRUE;
          pchrTest++;

          chrReplaced = *pchrTest;
          *pchrTest = NULC;

          /************************************************************/
          /*      now format the line to fit in the page buffer       */
          /************************************************************/
          usRC = QDPRFormatStrToLineLength( psctIDA,
                                            psctIDA->psctFormatIDA,
                                            *ppchrBuffer, psctCurBuf,
                                            &usLines );
          if ( usRC == QDPR_NO_ERROR )
          {
            (*pusLines) = (*pusLines) + usLines;
          } /* endif */

          *pchrTest = chrReplaced;
          (*ppchrBuffer) = pchrTest;
          *pusTagID = QDPR_ID_CRLF;
        }
      break;
      default :
        {
          pchrTest++;
        }
      break;
    } /* endswitch */
  } /* endwhile */

  return( usRC );

} /* end of function QDPRPrintUntilTag */
