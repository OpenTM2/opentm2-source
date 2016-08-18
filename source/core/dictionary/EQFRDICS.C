// *************************** Prolog *********************************
//
//               Copyright (C) 1990-2012, International Business Machines      
//               Corporation and others. All rights reserved         
//
//  Short description: Dialog to include remote dictionaries
//                     from a server into the list of dictionaries
//
//  Author:
//
//  Description:       This program provides the end user dialog
//                     to select from a list of servers and their
//                     corresponding dictionaries the
//                     dictionaries which are to be included in
//                     the Dictionary list window
//
// *********************** End Prolog *********************************

#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#include <eqf.h>                  // General Translation Manager include file

#include "eqfrdics.h"
#include "eqfrdics.id"

// macro to create dictionary property name out of dictionary name
#define PROPNAME( pszBuffer, pszDict )                       \
{                                                            \
   UtlMakeEQFPath( pszBuffer, NULC, SYSTEM_PATH, NULL );     \
   strcat( pszBuffer, "\\" );                                \
   Utlstrccpy( pszBuffer + strlen(pszBuffer), pszDict, '.' );\
   strcat( pszBuffer, EXT_OF_DICTPROP );                     \
}

BOOL fCompDictProps( PPROPDICTIONARY pProp1, PPROPDICTIONARY pProp2 )
{
  BOOL   fDifferent = FALSE;
  PPROFENTRY          pProp1Entry;     // ptr to entry structure
  PPROFENTRY          pProp2Entry;     // ptr to entry structure
  USHORT              usCount1 = 1;          // counter
  USHORT              usCount2 = 1;          // counter

  //compare source language, protection flag, number of entry fields
  //and possibly the
  //names of the entry fields if the number of entry fields in both
  //cases is the same.

  pProp1Entry = pProp1->ProfEntry;
  pProp2Entry = pProp2->ProfEntry;

  //compare protection flag
  if ( (pProp2->ulPassWord != pProp1->ulPassWord) ||
       (strcmp( pProp2->szSourceLang, pProp1->szSourceLang ) != 0 ) ||
       (pProp1->usLength != pProp2->usLength) )
  {
    fDifferent = TRUE;
  }
  else
  {
    /******************************************************************/
    /* Check dictionary fields                                        */
    /******************************************************************/
    while( (usCount1 <= pProp1->usLength) && !fDifferent )
    {
      //in the case of user defined fields the tokenids are the
      //same and so the system field names have to be compared
      if ( stricmp( pProp1Entry->chSystName, pProp2Entry->chSystName) == 0 )
      {
        pProp1Entry++;                 //get next entry for comparison
        usCount1++;                    //increase dict counter
        pProp2Entry = pProp2->ProfEntry;  //set to first
        usCount2 = 1;                //reset counter to 1
      }
      else  //look in next LocalPropEntry for possible equivalence
      {
        if ( usCount2 <= pProp2->usLength )
        {
          pProp2Entry++;   //get next entry
          usCount2++;          //increase local entry counter
        }
        else   // system name not in LocalPropEntry
        {
          fDifferent = TRUE; //entry structure varies
        } /* endif */
      } /* endif */
    } /* endwhile */
  } /* endif */
  return( fDifferent );
}

USHORT QDAMGetPart( HTM      htm,              //handle
                    PSZ      pszPath,          //Full file path
                    PGETDICTPART_IN  pGetPartIn, //Ptr to the input structure
                    PGETPART_OUT pGetPartOut,  //Ptr to the output structure
                    USHORT   usMsgHandling )   //Message handling parameter
{
  USHORT usURc = NO_ERROR;      // U function return code

  /********************************************************************/
  /* we can skip the U-code, we are not dealing with communication yet*/
  /********************************************************************/
  usURc = GetFilePart( pGetPartIn, pGetPartOut );

  //Perform appropriate error handling if requested and required
  if ( usMsgHandling && ( usURc != NO_ERROR) )
  {
     usURc = DictRcHandling( usURc, pszPath, htm, NULL );
  } /* endif */

  return usURc;
} /* End of function QDAMGetPart */

