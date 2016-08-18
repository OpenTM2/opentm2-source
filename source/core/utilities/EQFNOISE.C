/*----------------------------------------------------------------------------*/
/* Module EQFNoiseFiltering - TROJA Noise Filtering Module                    */
/*                                                                            */
/*                                                                            */
/*     Copyright (C) 1990-2012, International Business Machines               */
/*     Corporation and others. All rights reserved                            */
/*                                                                            */
/*  EQFNOISE.C                                                                */
/*                                                                            */
/*  TROJA Noise Filtering Module                                              */
/*                                                                            */
/*  Author:      R. Krome                                                     */
/*                                                                            */
/*  Status:      08/03/91                                                     */
/*                                                                            */
/*  Description: This module contains the function EQFNoiseFiltering          */
/*                                                                            */
/*  Calling Hierarchy:                                                        */
/*                                                                            */
/*                                                                            */
/*  Externals:                                                                */
/*                                                                            */
/*  Internals:                                                                */
/*                                                                            */
/*  Entry points:   EQFNoiseFiltering                                         */
/*                                                                            */
/*  Changes:                                                                  */
/*          release 1.0 initial release                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#define INCL_EQF_ANALYSIS         // analysis functions
#include <eqf.h>                  // General Translation Manager include file

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  EQFNoiseFiltering                                                         */
/*                                                                            */
/*  Description:                                                              */
/*    EQFNoiseFiltering ()                                                    */
/*    This function takes as input a token list with the corresponding        */
/*    text buffer and identifies noise words within the text. For that it     */
/*    uses the exclusion list provided by the calling function.               */
/*    All WORD tokens in the tokenlist are compared with the exclusion list   */
/*    and if they are identified as noise, the Token is changed to NOISEWORD. */
/*                                                                            */
/*  The following parameters have to be provided:                             */
/*    - pTextBuffer      (PSZ)                                                */
/*      Pointer to the text buffer.                                           */
/*                                                                            */
/*    - pExclList        (PEXCLUSIONLIST)                                     */
/*      Pointer to the exclusion list to be used for noise filtering.         */
/*                                                                            */
/*    - pFirstEntry       (TOKENENTRY *)                                      */
/*      Pointer to the first entry of the Tokenlist corresponding to the      */
/*      text buffer.                                                          */
/*      The tokenlist should contain the ENDOFLIST Token as last element.     */
/*                                                                            */
/*  function EQFNoiseFiltering (                                              */
/*        pTextBuffer    :  PSZ;             -- pointer to text buffer        */
/*        pExclList      : PEXCLUSIONLIST    -- pointer to exclusion list     */
/*        pFirstEntry    : TOKENENTRY *;     -- pointer to first tokenlist    */
/*                    ) return VOID             entry                         */
/*                                                                            */
/*    <*(* mark all words in the token list as noise, which are contained     */
/*         in the word table *)*> is                                          */
/*  begin                                                                     */
/*    <*(* set pointer to first tokenlist entry *)*>;                         */
/*    <*(* set pointers to exclusion list *)*>;                               */
/*    <*(* search token list for noise words *)*> is                          */
/*      while (* not end of token list *) loop                                */
/*        <*(* get Tokenid *)*>;                                              */
/*        <*(* if Tokenid is WORD, search string in word table and set        */
/*             Tokenid to NOISE if string is found *)*> is                    */
/*          if (* Tokenid is WORD *) then                                     */
/*            <*(* search text string in table and set NOISE if found *)*> is */
/*              begin                                                         */
/*                <*(* do a binary search for the text string on table *)*>;  */
/*                <*(* if found set Tokenid to NOISE *)*>;                    */
/*              end;                                                          */
/*          else                                                              */
/*            <*(* do nothing *)*>;                                           */
/*          end if;                                                           */
/*          <*(* increase pointer to token list *)*>;                         */
/*      end loop;                                                             */
/*  end EQFNoiseFiltering;                                                    */
/*                                                                            */
/*----------------------------------------------------------------------------*/
VOID EQFNoiseFiltering (
      PSZ            pTextBuffer,       /* pointer to text buffer             */
      PEXCLUSIONLIST pExclList,         /* pointer to exclusion list          */
      PTOKENENTRY    pFirstEntry        /* pointer to first tokenlist entry   */
                       )
{
   TOKENENTRY *pTokenList;              /* pointer to tokenlist               */
   CHAR       chBuffer[MAX_NOISEWORD];  /* text buffer for word to be checked */
   CHAR       * pchUprWord;              /* Pointer to normalized word         */
   USHORT     * puNoiseOffsets;         /* pointer to offset to noise words   */
   LONG       lHigh, lLower;            /* upper and lower index in excl.list */
   PSZ        pString;                  /* Pointer to first noise string      */
   INT        i, iResult;               /* result of string comparison        */
   BYTE       *pByte;                   /* pointer to exclusion list          */

  pchUprWord = NULL;

  pTextBuffer = pTextBuffer; //get rid of compiler warning
  pTokenList = pFirstEntry;
  pByte = (BYTE *) pExclList;
  puNoiseOffsets = (USHORT *)(pByte + pExclList->uFirstEntry);
  pString = (CHAR *)(pByte + pExclList->uStrings);

  while (pTokenList->sTokenid != ENDOFLIST) /* check all tokens in tokenlist */
  {
     if (pTokenList->sTokenid == TA_WORD)     /* token is word, check for noise  */
     {
        //check if word is longer than allowed length for exclusionword
        if (pTokenList->usLength >= MAX_NOISEWORD)
        {
           lHigh = -1; // don't start search, word cannot be Noise
           lLower = 0;
        }
        else
        {
           /* copy word into buffer and normalize it */
           memcpy(chBuffer, pTokenList->pDataString, pTokenList->usLength);
           chBuffer[pTokenList->usLength] = EOS;
           pchUprWord = strupr(chBuffer); // use general utility later, which
                                          // can also handle language specific
                                          // uppercasing

           lLower= 0;               /* initialize array boundaries */
           lHigh = pExclList->usNumEntries - 1;  // bound 0, n - 1
        } /* endif */


        /* binary search in Exclusion List */

        while (lLower <= lHigh )
        {
          i= lLower + (lHigh - lLower)/2;     /* calculate index             */
          iResult= strcmp(puNoiseOffsets[i] + pString, pchUprWord);

          if (iResult == 0)
          {
            lHigh = -1; // search ended, as word is found
            pTokenList->sTokenid = NOISEWORD;
          }
          else
          {
            if (iResult < 0)
            {
              /* search in upper part of table */
              lLower = i + 1;
            }
            else
            {
              /* search in lower part of table */
              lHigh = i - 1;
            } /* endif */
          } /* endif */
        } /* end while */

     } /* end if */


     pTokenList++;
  } /* endwhile */

} /* end of EQFNosieFiltering */

