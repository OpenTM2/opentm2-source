//	Copyright Notice:
//
//	Copyright (C) 1990-2014, International Business Machines
//	Corporation and others. All rights reserved

/*! \file

	
	This function identifies all tags in the passed  text block and builds
	a token list for the text block. The tags are identified using the
	Tagtable provided by the calling function.
	The token list area has to be allocated by the calling function.
	A token in the token list consists of:

		-  Token Identifier (which is a Tag Id or TEXT or ENDOFLIST)
		-  pointer to start of token in the text block
		-  length of token

	The token list is terminated by the ENDOFLIST token.

	If the calling function passes an incomplete text block, i.e. more
	text will follow in subsequent calls, the current text block will
	be tokenized up to the last complete token, and a pointer to the
	rest of input, which has not been tokenized will be returned.

	If the calling function does not provide an area large enough to
	build the token list for the complete text block in, EQFTagTokenize
	will stop tokenizing when the token list area is full and return
	a pointer to the rest of input that has not been tokenized.

	The following parameters have to be provided by the calling function:

	- pTextBuffer  (PSZ)
	  Pointer to the text buffer that shall be tokenized.
	  The text must be null-terminated.

	- pTagTable    (TAGTABLE *)
	  Pointer to the start of the tag table to be used for tokenizing.

	- fAll         (BOOL)
	  Flag which indicates whether a complete text block was passed or
	  more text will follow.
	  fAll = TRUE:  the text is complete and is to be tokenized up to the
					very end
	  fAll = FALSE: the text is not complete and more text blocks will follow
					in subsequent calls, thus there might be an incomplete
					token at the end.

	- ppRest       (PSZ *)
	  Pointer to the rest of the text buffer. It indicates up to where the
	  text has been tokenized by EQFTagTokenize. If the text block has been
	  completely tokenized, *ppRest points to the last character in the text
	  buffer.
	  Cases where a text block will not be completely tokenized are either
	  if fAll is FALSE and thus there may be an incomplete token at the end
	  of the text or if the memory allocated for the tokenlist is not large
	  enough to hold all tokens of the text block.

	- pusLastColPos (USHORT *)
	  Pointer to the column position where the text block starts. When
	  EQFTagTokenize is called for the first time for a text the column
	  position has to be initialized with 0 (* pusLastColPos = 0).
	  EQFTagTokenize will return the column position of the last character
	  up to where the text block has been tokenized. If subsequent text
	  blocks are passed to EQFTagTokenize by the calling function the
	  previously returned column position has to be provided.

	- pTokenlist   (TOKENENTRY *)
	  Pointer to the memory allocated for the tokenlist by the calling
	  function

	- uMaxNumTokens (USHORT)
	  The maximum number of Tokenlist entries that fit in the memory
	  allocated by the calling function

*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqftai.h"               // Private include file for Text Analysis
#include "eqftag00.h"             

#define DBLQUOTE   '\"'                     // double quote character (")

// size of buffer (in number of tokens) for quoted string tokenization in TACreateProtectTableW
#define MAX_ATTR_TOKENS 512

//--- Function prototypes for internal C functions                                */
static BOOL Tagsearch (PSZ, ULONG, USHORT, BOOL, PTAG, CHAR *, TAGTABLE *, SHORT *);
static BOOL Attributesearch(PSZ, ULONG, PATTRIBUTE, CHAR * ,TAGTABLE *, SHORT *);
static BOOL Skipwhitespace(PSZ *, USHORT *);
static BOOL WildCardSearch (PSZ, PSZ );
static PSZ SkipToTextEnd( PSZ  );
static PSZ SkipTextString ( PSZ, USHORT * );
#define MAX_CHINPUTW 40000
static CHAR_W chInputW[ MAX_CHINPUTW ];

#define TAG_END_CHAR       "."        // tag end character
#define TAG_END_CHARW      L"."
PNODE  TAFastCreateNodeTree( PTBTAGENTRY pTags, ULONG ulNoOfTags );
PNODE  TAFastCreateTagTree( PLOADEDTABLE pLoadedTable, PTAGTABLE pTagTable );
PNODE  TAFastCreateAttrTree( PLOADEDTABLE pLoadedTable, PTAGTABLE pTagTable );
BOOL   TAFastAddNode ( PNODE *, PNODE, PULONG, PULONG );
int    TATastTagCompare( const void *, const void * );

VOID TASetStartStopType(PSTARTSTOP pCurrent, USHORT usStart, USHORT usStop, USHORT usType );
USHORT TAFindQuote ( PTOKENENTRY pTok, USHORT usStartSearch, PCHAR_W pchQuote );
USHORT  TAProtectedPartsInQuotedText
(
  PTOKENENTRY   pTok,
  int           iAttrStartOffset,
  PSTARTSTOP    * pStartStop,
  PSTARTSTOP    * pCurrent,
  PTOKENENTRY     pAttrTokBuffer,
  PLOADEDTABLE    pTagTable,
  PULONG          pulTableAlloc,
  PULONG          pulTableUsed,
  PUSHORT         pusColPos,
  CHAR_W          chQuote,
  LONG            lAttrSize,
  BOOL            fSpecialMode
);

USHORT TAGotoNextStartStopEntry
(
	PSTARTSTOP  * ppStartStop,
	PSTARTSTOP  * ppCurrent,
	PULONG    pulTableUsed,
	PULONG    pulTableAlloc
);
static void
 TATagAddWSpace
   (
	   PSZ_W            pszSegment,
	   PTOKENENTRY  	pTokBuffer,
	   SHORT            sNumTags
   );


/**********************************************************************/
/* add pragmas to force load into different text segments             */
/**********************************************************************/
//#pragma alloc_text(EQFTOKEN1, EQFTagTokenize)
//
//#pragma alloc_text(EQFTOKEN2, TAFastTokenize)
//
//
//#pragma alloc_text(EQFTOKEN3, TACreateTagTree, TACreateAttrTree,          \
//                              TACreateNodeTree)
//
//#pragma alloc_text(EQFTOKEN4, TATagTokenize, TAMatchTag,                  \
//                              TACreateProtectTable, TAPrepProtectTable,   \
//                              TAEndProtectTable)

#define TA_IS_TAG(a,b)  ( (a < b) && (a >=0) )

#define TA_IS_ATTRIBUTE(a,b)  (a >= b)

//------------------------------------------------------------------------------
//  EQFTagTokenize
//  Tokenization main program
//------------------------------------------------------------------------------

// used currently only for TagTable import: since tagtable are not allowed
// to be UTF16 files, this function is not allowed to be changed!!
void EQFTagTokenize(
  PSZ    pBuffer,                  /* Buffer for Input text                   */
  TAGTABLE  *pTagTable,            /* pointer to Tagtable                     */
  BOOL    fAll,                    /* does text block contain the whole text? */
  PSZ  *ppRest,                    /* pointer to end of tokenized input       */
  USHORT  *pusLastColPos,          /* last column position                    */
  TOKENENTRY  *pTokenentry,        /* pointer to Token List                   */
  USHORT  uMaxNumTokens )            /* Maximum number of Tokens                */

{
   TAGSTART * pTagstart;                /* pointer to Tagstart table           */
   TAGSTART * pCurTagstart;             /* pointer to current characterin table*/
   STARTPOS * pStartpos;                /* pointer to Tag Start Position Table */
   STARTPOS * pCurStartpos;             /* pointer to Tag Start Position Table */
   char     chFirst;
   PSZ      pszEndDel = NULL;           /* pointer to end delimiters           */
   PSZ      pAttrEndDelim;              /* pointer to Attribute End Delimiters */
   BOOL     fColumn = FALSE;            /*  is tag search column based?        */
   PROCESSINGMODE Processingmode;       /* current processingmode              */
   CHAR     chStringbuffer[MAXTAGLEN+1];/*  string buffer                      */
   PSZ      pPosTagStart = NULL;        /*  pointer to possible start of tag   */
   PSZ      pTagEnd = NULL;             /* pointer to end of tag/attribute found*/
   PSZ      pCurTextPos;                /*  pointer to current position in text */
   BOOL     fEndcondfound;              /*  end condition for string    found? */
   BOOL     fOverflow;                  /* buffer overflow ?                   */
   USHORT   usColumnPos = 0;            /* column position of possible tag/    */
                                        /* attribute start                     */
   USHORT   usCurColumnPos;             /* current column position             */
   ULONG    ulBuffercount;              /* count of Stringbuffer               */
   USHORT   usTagLen = 0;               /* length of possible tag              */
   SHORT    sTokenid;                   /* Tokenid                             */
   BOOL     fFound;                     /* Tagstring, Attributestring found?   */
   USHORT     uTokencount;              /* actual number of tokens             */
   BOOL     fTagstart;                  /* has start of Tag been found         */
   ULONG    i;                          // index
   BOOL     fAttr = FALSE;              // can attributes follow after tag?
   PTAG     pTag;                       // pointer to TAG
   BYTE     *pByte;                     // PBYTE pointer to Tagtable
   USHORT     uActiveTag = 0;                 // index to active tag in tokenlist
   USHORT     uNumTags;                   // number of tags in tagtable
   PSZ      pEndString;                 // pointer to end of text string
   USHORT   usTokenColPos;              // column position of active token
   BOOL     fTextString = FALSE;        // can text string occur?
   CHAR     chCurrent;
   PTOKENENTRY pToken;
   CHAR      * pTagNames;           /* pointer to Strings in Tagtable          */
   PATTRIBUTE   pAttribute;         /* pointer to first fix attribute          */
   PSZ      pTemp;
   CHAR     c;

   usCurColumnPos =  *pusLastColPos + 1;    /* init column position            */
   usTokenColPos = *pusLastColPos;      /* init token column position      */

   pByte = (BYTE *) pTagTable;

   uTokencount = 0;                         /* initialize Tokenlist            */
   pTokenentry->sTokenid = TEXT_TOKEN;
   pTokenentry->pDataString = pBuffer;
   pTokenentry->usLength = 0;
   pToken = pTokenentry;                   /* remember start of tokenlist      */

   pCurTextPos = pBuffer;
   chCurrent = *pCurTextPos;

   ulBuffercount = 0;                       /* init string buffer              */

   /* init TagTable variables                                                  */
   pStartpos = OFFSETTOPOINTER (PSTARTPOS, pTagTable->uStartpos);

   uNumTags = pTagTable->uNumTags;
   pTagstart = OFFSETTOPOINTER(TAGSTART *, pTagTable->uTagstart);
   pTag = OFFSETTOPOINTER(PTAG, pTagTable->stFixTag.uOffset);
   pAttribute = OFFSETTOPOINTER(PATTRIBUTE, pTagTable->stAttribute.uOffset);
   pAttrEndDelim = OFFSETTOPOINTER(PSZ, pTagTable->uAttrEndDelim);
   pTagNames = OFFSETTOPOINTER(PSZ, pTagTable-> uTagNames);

   fOverflow = FALSE;                       /* init fOverflow to false         */

   Processingmode = TAGSEARCH;             /* init Processingmode to TAGSEARCH */

   /* parse the text block and build token list                                */
   while ( (chCurrent != EOS) ||
           (fAll && Processingmode != TAGSEARCH  && !fOverflow) )
   {
      /* handle character depending on processing mode, fill Tokenlist        */
      /* and change processing mode if necessary                              */
      switch ( Processingmode )
      {
         //------------------------------------------------------------------
         case  TAGSEARCH:
            /* parse text to possible start of next tag                    */
            fTagstart = FALSE;         /* check if character is start char */
            pCurTagstart = pTagstart;
            chFirst = pCurTagstart->chFirstchar;
            fColumn = FALSE;
            // check for tag start characters
            while (!fTagstart && (chFirst != EOS ) )
            {
               if (chFirst == chCurrent)
               {
                  // set tag start, tag length and possible end delimiter
                  fTagstart = TRUE;
                  pszEndDel = (PSZ)(pByte + pCurTagstart->uEndDelim);
                  fColumn = FALSE;
                  usTagLen = pCurTagstart->usLength;
               }
               else
               {
                  pCurTagstart++;
                  chFirst = pCurTagstart->chFirstchar;
               } /* endif */
            }/* endwhile */

///         for (pCurTagstart= pTagstart;
///              pCurTagstart->chFirstchar != EOS;
///              pCurTagstart++)
///              {
///              if (chCurrent == pCurTagstart->chFirstchar)
///                 {
///                 fTagstart = TRUE;                        // tag start found
///                 pszEndDel = pByte + pCurTagstart->uEndDelim; // set end delim
///                 usTagLen = pCurTagstart->usLength;           // tag length
///                 break;
///                 }
///              } /* endfor */

            pCurStartpos = pStartpos;
            while (!fTagstart && pCurStartpos->usPosition != 0)
               {             /* check if column is start column */
               if (pCurStartpos->usPosition == usCurColumnPos)
                  {
                  pszEndDel = (PSZ)(pByte + pCurStartpos->uEndDelim);
                  if (strchr(pszEndDel, chCurrent)==NULL)
                     {   /* current character is not end delimiter   */
                     fTagstart = TRUE;
                     fColumn = TRUE;
                     usTagLen = pCurStartpos->usLength;
                     } /* endif */
                  } /* endif */
               pCurStartpos ++;
               } /* endwhile */

            if (fTagstart)                  /* possible start of tag found */
               {
               Processingmode = TAGVALIDATE;
               pPosTagStart = pCurTextPos;   // remember position
               usColumnPos = usCurColumnPos; // and colmn number
               chStringbuffer[ulBuffercount] = chCurrent;
               ulBuffercount++;
               //--- special handling if length of tag is 1
               if ( usTagLen == 1 )
                  {
                  pCurTextPos--;
                  ulBuffercount = 0;
                  }
               }
            break;

         //------------------------------------------------------------------------------
         case TAGVALIDATE:
            /* check for a valid tag                                           */
            /* check if character is corresponding end character               */
            fEndcondfound = FALSE;
            pTemp = pszEndDel;

///         if (!fEndcondfound)
///            fEndcondfound = strchr(pszEndDel, chCurrent) != NULL;
            while ( ( c=*pTemp ) != EOS && !fEndcondfound )
            {
               fEndcondfound = ( c == chCurrent);
               pTemp ++;
            } /* endwhile */


            /* check if current tag buffer length matches tag length           */
            if (usTagLen == ulBuffercount + 1)
               {
               fEndcondfound = TRUE;
               chStringbuffer[ulBuffercount++] = chCurrent;
               }

            if (fEndcondfound)                /* search string in tagtable     */
               {
               chStringbuffer[ulBuffercount] = EOS;
               fFound = Tagsearch (chStringbuffer, ulBuffercount, usColumnPos,
                                   fColumn, pTag, pTagNames, pTagTable, &sTokenid);
               if (fFound)
                  {
                  if (pToken->pDataString != pPosTagStart)
                     {
                     fOverflow = (uTokencount +2 >= uMaxNumTokens);
                     if ( fOverflow )
                        {         /* check  overflow   */
                        pCurTextPos = SkipToTextEnd(pCurTextPos);
                        chCurrent = *pCurTextPos;
                        }
                     else                  /* if no overflow             */
                        {                 /* increase tokenlist pointer */
                        uTokencount++;
                        pToken++;
                        } /* endif */
                     } /* endif */

                 if (!fOverflow)
                    {
                    /* store tagid and tagstart position in tokenlist          */
                    pToken->sTokenid = sTokenid;
                    pToken->pDataString = pPosTagStart;
                    usTokenColPos = usColumnPos - 1;
                    uActiveTag = uTokencount;    // remember last identified tag
                    Processingmode = ATTRIBUTESEARCH;

                    fAttr = FALSE;
                    /* set Processingmode according to attribute flag          */
                    if (pTag[sTokenid].fAttr)
                       {
                       pTagEnd = pCurTextPos;   /* remember pointer and */
                       usColumnPos = usCurColumnPos;   /* column position      */
                       // check if an attribute can follow
                       if (chCurrent == ' ' ||
                           chCurrent == CR  ||
                           chCurrent == LF)
                          fAttr = Skipwhitespace(&pCurTextPos, &usCurColumnPos);
                       else
                          fAttr = FALSE;

                       chCurrent = *pCurTextPos;
                       pPosTagStart = pCurTextPos + 1; // remember start of attr
                       if (fAttr)
                          {
                          fTextString = TRUE;
                          } /* endif */
                       } /* endif */

                    if (!fAttr)            /* no attributes can follow         */
                       {
                       fOverflow = (uTokencount + 2 >= uMaxNumTokens);
                       if ( fOverflow )
                          {                             /* check overflow */
                          pCurTextPos = SkipToTextEnd(pCurTextPos);
                          chCurrent = *pCurTextPos;
                          }
                       else
                          {
                          Processingmode = TAGSEARCH;
                          uTokencount ++;
                          pToken++;
                          pToken->sTokenid = TEXT_TOKEN;
                          SETCOLUMNPOS (usTokenColPos);

                          /* set token pointer to next input position          */
                          /* if it is not EOS                                  */
                          if (chCurrent == EOS)
                             {
                             pToken->pDataString = pCurTextPos;
                             }
                          else
                             {
                             pToken->pDataString = pCurTextPos+1;
                             } /* endif */
                          } /* endif */
                       } /* endif */
                    } /* endif */
                  } /* endif fFound */
              else                   /* no match found in Tagtable */
                 {
                 usCurColumnPos = usColumnPos;   /* reset column position and  */
                 pCurTextPos = pPosTagStart;     /* text pointer               */
                 chCurrent = *pCurTextPos;
                 Processingmode = TAGSEARCH;
                 }

               ulBuffercount = 0;                 /* init string buffer */
               }  /* endif end of tag found */
            else                                       /* not fEndcondfound    */
               {
               /* copy character to string buffer and increase
                  string buffer pointer, check for overflow */

               chStringbuffer[ulBuffercount++] = chCurrent;
               if (ulBuffercount > MAXTAGLEN)
                  {
                  Processingmode = TAGSEARCH;        /* reset column position  */
                  usCurColumnPos = usColumnPos;      /* and text pointer       */
                  pCurTextPos = pPosTagStart;
                  chCurrent = *pCurTextPos;
                  ulBuffercount = 0;
                  } /*endif */
               }
            break;

         //------------------------------------------------------------------------------
         case ATTRIBUTESEARCH:
            /* check for a valid attribute */
            fEndcondfound = FALSE;

            /* check if character is literal, if yes skip to end of Textstring */
            if ((chCurrent == QUOTE || chCurrent == '(' )
                 && pTagTable->uAttrLength == 0
                 && fTextString)
               {
               /* skip text string and copy string into string buffer    */
               fTextString = FALSE; /* text string only once per attribute */
               pEndString = SkipTextString(pCurTextPos, &usCurColumnPos);

               /* copy string into buffer                      */
               i = min((SHORT)(MAXTAGLEN-1-ulBuffercount), pEndString-pCurTextPos);
               memcpy(&(chStringbuffer[ulBuffercount]), pCurTextPos, i);
               pCurTextPos +=i;
               chCurrent = *pCurTextPos;
               ulBuffercount += i;
               }
            else                              /* check for endcondition        */
               {
               /* check if character is end delimiter in attribute table */
               fEndcondfound = strchr(pAttrEndDelim, chCurrent) != NULL;

               /* check if current buffer length matches attribute length      */
               if (pTagTable->uAttrLength == ulBuffercount +1)
                  {
                  fEndcondfound = TRUE;
                  chStringbuffer[ulBuffercount++] =  chCurrent;
                  } /* endif */
               } /* endif */

            if (fEndcondfound)
               {
               /* search with string buffer in Attribute table                 */
               chStringbuffer[ulBuffercount]= EOS;
               fFound = Attributesearch(chStringbuffer, ulBuffercount,
                                        pAttribute, pTagNames,
                                        pTagTable, &sTokenid);
               if (fFound)
                  {
                  fOverflow = (uTokencount + 2 >= uMaxNumTokens);
                  if ( fOverflow )
                     {                              /* check overflow */
                     pCurTextPos = SkipToTextEnd(pCurTextPos);
                     chCurrent = *pCurTextPos;
                     }
                  else                 /* store attribute in tokenlist         */
                     {
                     uTokencount ++;
                     pToken++;
                     pToken->sTokenid = sTokenid + uNumTags;
                     pToken->pDataString = pPosTagStart;
                     pTagEnd = pCurTextPos;      // remember reset position
                     usColumnPos = usCurColumnPos; // and column
                     // check if an attribute can follow
                     if (chCurrent == ' ' || chCurrent == CR || chCurrent == LF)
                        fAttr = Skipwhitespace(&pCurTextPos, &usCurColumnPos);
                     else
                        fAttr = FALSE;

                     chCurrent = *pCurTextPos;
                     pPosTagStart = pCurTextPos + 1; // remember start of attr
                  } /* endif */
               } /* endif */

               if (!fFound || !fAttr)        /* no more attributes can follow  */
               {
                  fOverflow = (uTokencount + 2 >= uMaxNumTokens);
                  if ( fOverflow )
                  {                                /* check tokenlist overflow */
                     pCurTextPos = SkipToTextEnd(pCurTextPos);
                     chCurrent = *pCurTextPos;
                  }
                  else          /* store text id as next token in tokenlist    */
                  {
                    pCurTextPos = pTagEnd;   // reset position
                    chCurrent = *pCurTextPos;
                    usCurColumnPos = usColumnPos;  // and column
                    Processingmode = TAGSEARCH;
                    uTokencount++;
                    pToken++;
                    pToken->sTokenid = TEXT_TOKEN;
                    SETCOLUMNPOS(usTokenColPos);

                    /* set token pointer to next input position          */
                    /* if it is not EOS                                  */
                    if (chCurrent == EOS)
                    {
                      pToken->pDataString = pCurTextPos;
                    }
                    else
                    {
                      pToken->pDataString = pCurTextPos+1;
                    } /* endif */
                  } /* endif */
               } /* endif */

               ulBuffercount = 0;                        /* init Stringbuffer  */
               fTextString = TRUE;
            }
            else                                     /* no end delimiter found */
            {
               chStringbuffer[ulBuffercount] =  chCurrent;
               ulBuffercount ++;

               if (ulBuffercount > MAXTAGLEN)   /* check for buffer overflow  */
               {
                  fOverflow = (uTokencount + 2 >= uMaxNumTokens);
                  if ( fOverflow )
                  {                                /* check tokenlist overflow */
                     pCurTextPos = SkipToTextEnd(pCurTextPos);
                     chCurrent = *pCurTextPos;
                  }
                  else          /* store text id as next token in tokenlist    */
                  {
                    pCurTextPos = pTagEnd;
                    chCurrent = *pCurTextPos;
                    usCurColumnPos = usColumnPos;
                    Processingmode = TAGSEARCH;
                    uTokencount++;
                    pToken++;
                    pToken->sTokenid = TEXT_TOKEN;
                    SETCOLUMNPOS(usTokenColPos);

                    /* set token pointer to next input position          */
                    /* if it is not EOS                                  */
                    if (chCurrent == EOS)
                    {
                      pToken->pDataString = pCurTextPos;
                    }
                    else
                    {
                      pToken->pDataString = pCurTextPos+1;
                    } /* endif */
                  } /* endif */
                  ulBuffercount = 0;
               } /*endif */
            } /* endif */
            break;
         //------------------------------------------------------------------------------
      }/* endswitch */

      /* check for end of line, if yes reset current column position */
      if (chCurrent == LF)
      {
         usCurColumnPos=0;
      } /* endif */

      if (chCurrent != EOS)     /* point to next character               */
      {
         usCurColumnPos++;
         pCurTextPos ++;
         chCurrent = *pCurTextPos;
      } /* endif */
   } /* endwhile */

   /* set pRest if more text is following */
   if (!fAll || fOverflow)
   {
      switch (Processingmode)
      {
         //------------------------------------------------------------------------------
         case TAGSEARCH:
            if (uTokencount > 1) // at least one tag identified
            {
               *ppRest = pToken->pDataString;    // discard last TEXT token
               *pusLastColPos = usTokenColPos;
               uTokencount --;
               pToken--;
            }
            else // no tag identified
            {
               /* set pRest to last CR/LF in text buffer */
               while (chCurrent != LF
                      && pCurTextPos > pBuffer)
               {
                  pCurTextPos--;
                  chCurrent = *pCurTextPos;
               } /* endwhile */

               if (chCurrent == LF)
               {
                  *ppRest = pCurTextPos + 1;
                  *pusLastColPos = 0;
               }
               else
               {
                  *ppRest = pCurTextPos;
               } /* endif */
            } /* endif */
            break;
         //------------------------------------------------------------------------------
         case TAGVALIDATE:
             if (uTokencount > 1)    // at least one tag identified
             {
                *ppRest = pToken->pDataString;   // discard last text token
                *pusLastColPos = usTokenColPos;
                uTokencount --;
                pToken--;
             }
             else    // no tag identified
             {
                pCurTextPos = pPosTagStart;  //set pRest to last LF before start
                chCurrent = *pCurTextPos;    // of possible tag
                while (chCurrent != LF &&
                       pCurTextPos > pBuffer)
                {
                   pCurTextPos--;
                   chCurrent = *pCurTextPos;
                } /* endwhile */

                if (chCurrent == LF)
                {
                  *ppRest = pCurTextPos + 1;
                  *pusLastColPos = 0;
                }
                else
                {
                  *ppRest = pCurTextPos;
                } /* endif */
             } /* endif */
             break;
         //------------------------------------------------------------------------------
         case ATTRIBUTESEARCH:
            // set pRest to start of last identified tag token
            *ppRest = pTokenentry[uActiveTag].pDataString;
            *pusLastColPos = usTokenColPos;
            uTokencount = uActiveTag - 1;
            pToken = & (pTokenentry[uActiveTag - 1]);
            break;
         //------------------------------------------------------------------------------
      } /* endswitch */
   }
   else
   {
     if (fAll && !fOverflow )
     {
       *ppRest = pCurTextPos;
       *pusLastColPos = usCurColumnPos - 1;
      } /* endif */
   } /* endif */

   /* insert ENDOFLIST token as last element in Tokenlist *)*>; */
   uTokencount ++;
   pToken ++;
   pToken->sTokenid= ENDOFLIST;
   pToken->pDataString = *ppRest;

   /* calculate length information for Tokenlist */
   pToken = pTokenentry;
   for (i=1; i<=uTokencount; i++, pToken ++ )
   {
      pToken->usLength = (USHORT)((pToken+1)->pDataString - pToken->pDataString);
   } /* endfor */

   if (pTokenentry[uTokencount-1].usLength == 0 )      /* discard last token, */
   {                                                   /* if the length is 0  */
      pTokenentry[uTokencount-1].sTokenid= ENDOFLIST;
   } /* endif */

   if (**ppRest == EOS)           /* set *ppRest to null it text is completely */
   {                              /* tokenized                                 */
      *ppRest = NULL;
   } /* endif */
}/* end EQFTagTokenize */

/*
//------------------------------------------------------------------------------
  function Tagsearch
  search the passed string in the tag table
//------------------------------------------------------------------------------
*/
static
BOOL Tagsearch
  (PSZ      pchStringbuffer,       /* pointer to Stringbuffer                 */
  ULONG     ulLength,              /* length of string                        */
  USHORT    usColumnPos,           /* start column position                   */
  BOOL      fColumn,               /* is search column or string based?       */
  PTAG      pTag,                  /* pointer to first fix  Tag               */
  CHAR      * pTagNames,           /* pointer to Strings in Tagtable          */
  TAGTABLE  * pTagTable,           /* pointer to tagtable                     */
  SHORT     * psTagid)             /* pointer to Tagid of identified Tag      */
{
   CHAR   * pchUprTag;              /* pointer to normalized input tag         */
   PTAG   pVarTag;                  /* pointer to first varible Tag            */
   BYTE   * pByte;                  /* pointer to TagTable                     */
   SHORT  sLower;                   /* Lower Array boundary to be searched     */
   SHORT  sIndex;                   /* Index into variable part of table       */
   SHORT  sHigh;                    /* High  Array boundary to be searched     */
   USHORT i;                        // index
   BOOL   fFound;                   /* tag found?                              */
   INT    iResult;                  /*  result of string compare               */
   USHORT   uNumTags;                 /* number of tags in array to be searched  */
   BOOL   fCR = FALSE;

   pByte = (PBYTE) pTagTable;

   pchUprTag = strupr(pchStringbuffer);   /* normalize input string - DBCS!    */

   if (pchStringbuffer[ulLength-1] == CR)
   {
      pchStringbuffer[--ulLength] = EOS;
      fCR = TRUE;                            // string originally contained CR
   } /* endif */

   fFound = FALSE;                               /* tag not found yet          */

   if (!fColumn)                                 /* search tag character based */
   {
      /* do binary search in fixed part of tagtable */
      sLower= 0;                                 /* init array boundaries      */
      sHigh = pTagTable->stFixTag.uNumber - 1;

      while ((sLower <= sHigh) && !fFound)
      {
         i= (sLower + sHigh) / 2;
         iResult= memcmp(pTag[i].uTagnameOffs + pTagNames, pchUprTag, ulLength+1);

         if (iResult == 0)
         {
            fFound = TRUE;                   /* set end condition             */
            *psTagid = i;                  /* store found tag as result value */
         }
         else
         {
            if (iResult < 0)                 /* search in upper part of table */
            {
               sLower = i + 1;
            }
            else                             /* search in lower part of table */
            {
               sHigh = i - 1;
            } /* endif */
         } /* endif */
      } /* endwhile */

      /* check if start position matches                                     */
      if (fFound && pTag[*psTagid].usPosition != 0)
      {
        if (pTag[*psTagid].usPosition != usColumnPos)
        {                                   /* position does not match       */
           fFound = FALSE;
           *psTagid = 0;
        } /* endif */
      } /* endif */
   } /* endif */

   if (!fFound)
   {                                 /* do wildcard search on index entries    */
      if (!fColumn)
      {              /* calculate index depending on first character           */
//         sIndex = (isalpha(*pchUprTag)) ? (*pchUprTag - 'A') : 26; /* DBCS !!  */
         if ( (*pchUprTag >= 'A') && (*pchUprTag <= 'Z') )
         {
           sIndex = *pchUprTag - 'A';
         }
         else
         {
           sIndex = 26;
         } /* endif */
         pVarTag = OFFSETTOPOINTER(PTAG, pTagTable->stTagIndex[sIndex].uOffset);
         uNumTags = pTagTable->stTagIndex[sIndex].uNumber;
      }
      else
      {        /* set pointer to part of tagtable containing wildcard at start */
         uNumTags = pTagTable->stVarStartTag.uNumber;
         pVarTag = OFFSETTOPOINTER(PTAG, pTagTable->stVarStartTag.uOffset);
      }/* endif */

      i=0;
      if (fCR)      // string originally contained CR, restore it
      {
         pchStringbuffer[ulLength++] = CR;
      } /* endif */

      while (i<uNumTags && !fFound)
      {
         fFound = WildCardSearch (pchUprTag, pVarTag->uTagnameOffs + pTagNames);
         if (!fFound)
         {
            pVarTag++;
            i++;
         }
         else
         {  /* check if start position matches                                 */
            *psTagid = (SHORT)(pVarTag - pTag);
            if (pTag[*psTagid].usPosition != 0 &&
                pTag[*psTagid].usPosition != usColumnPos)
            {
               fFound = FALSE;                  /* position does not match      */
               *psTagid = 0;
               pVarTag++;
               i++;
            } /* endif */
         } /* endif */
      } /* endwhile */
   } /* endif */
   return (fFound);
}/* end Tagsearch */

/*
//------------------------------------------------------------------------------
  function Attributesearch
  search the passed string in the tag table
//------------------------------------------------------------------------------
*/
static
BOOL Attributesearch
  (PSZ      pchStringbuffer,             /* pointer to Stringbuffer           */
   ULONG    ulLength,
   PATTRIBUTE   pAttribute,         /* pointer to first fix attribute          */
   CHAR      * pTagNames,           /* pointer to Strings in Tagtable          */
   TAGTABLE  * pTagTable,                 /* pointer to tagtable               */
   SHORT     * psAttrid)                  /* pointer to Tagid of identified Tag*/
{
   CHAR          *pchUprAttr;       /* pointer to normalized input attribute   */
   BOOL         fFound;             /* has Attribute been found in Table?      */
   USHORT         uNumAttr;           /* number of attr in array to be searched  */
   SHORT        sIndex;             /* Index into variable part of table       */
   PATTRIBUTE   pVarAttr;           /* pointer to first variable attribute     */
   LONG         lHigh, lLower;      /* array boundaries                        */
   INT          i;
   INT          iResult;            /*  result of string compare               */
   PBYTE        pByte;

   pByte = (PBYTE) pTagTable;
   pchUprAttr = strupr(pchStringbuffer);             /* normalize input string */

   // discard last character in string, if it is CR
   if (pchStringbuffer[ulLength-1] == CR)
   {
      pchStringbuffer[--ulLength] = EOS;
   } /* endif */

   fFound = FALSE;

   /* do binary search in fixed part of tagtable */
   lLower= 0;                                  /* init array boundaries        */
   lHigh = pTagTable->stAttribute.uNumber - 1;

   while ((lLower <= lHigh) && !fFound)
   {
      i= (lLower + lHigh)/2;
      /* DBCS???!   */
      iResult= memcmp(pAttribute[i].uStringOffs + pTagNames, pchUprAttr, ulLength + 1);

      if (iResult == 0)
      {
         fFound = TRUE;                  /* set end condition and attribute id  */
         *psAttrid = (SHORT)i;
      }
      else
      {
         if (iResult < 0)                      /* search in upper part of table */
         {
            lLower = i + 1;
         }
         else                                  /* search in lower part of table */
         {
            lHigh = i - 1;
         } /* endif */
      } /* endif */
   } /* endwhile */

   if (!fFound)
   {                                 /* do wildcard search on index entries    */
      /* calculate index                                                       */
      sIndex = (isalpha(*pchUprAttr)) ? (*pchUprAttr - 'A') : 26; /* DBCS??!   */

      pVarAttr = OFFSETTOPOINTER(PATTRIBUTE,
                                 pTagTable->stAttributeIndex[sIndex].uOffset);
      uNumAttr = pTagTable->stAttributeIndex[sIndex].uNumber;

      i=0;
      while (i<(SHORT)uNumAttr && !fFound)
      {
         fFound = WildCardSearch (pchUprAttr, pVarAttr->uStringOffs + pTagNames);
         if (!fFound)
         {
            pVarAttr++;
            i++;
         }
         else
         {                                                  /* calculate TagId */
            *psAttrid = (SHORT)(pVarAttr - pAttribute);
         } /* endif */
      } /* endwhile */
   } /* endif */
   return (fFound);
}/* end Attributesearch */

/*
//------------------------------------------------------------------------------
  function Skipwhitespace
  Skip whitespace until second Newline is encountered
//------------------------------------------------------------------------------
*/
static
BOOL Skipwhitespace(
  PSZ * ppInput,               /* pointer to input string                    */
  USHORT *pusPosition)         /* pointer to column position                 */
{
   BOOL fPossAttr;             /* can an attribute follow after that tag?    */
   USHORT usLncount;           /* number of newlines encountered             */
   USHORT usCurPosition;       /* current column position                     */
   PSZ pCurInput;              /* pointer to current text position            */

   usLncount = 0;               /* initialization                              */
   fPossAttr = TRUE;
   if (**ppInput != EOS)
   {
      usCurPosition = (**ppInput == LF) ? 1 : *pusPosition + 1;
      pCurInput = *ppInput + 1;
      while ((*pCurInput == ' ' ||
              *pCurInput == CR  ||
              *pCurInput == LF ) &&
              usLncount < 2)
      {
         if (*pCurInput == LF)                            /* handle newline   */
         {
            usCurPosition = 0;
            usLncount ++;
          } /* endif */
          usCurPosition ++;
          pCurInput ++;
      } /* endwhile */

      /* set return according to number of newlines */
      if (usLncount == 2)
      {
         fPossAttr = FALSE;
      }
      else
      {
         *pusPosition = usCurPosition - 1;
         *ppInput = pCurInput - 1;
      } /* endif */
   } /* endif */
   return (fPossAttr);
}/* end Skipwhitespace */

/*
//------------------------------------------------------------------------------
  function WildCardSearch
  check if two strings match, the second string may contain substitution
  characters for one character (?) or multiple characters (*). The
  string is normalized, i.e. uppercase and * is never followed by
  ? or *. The normalization is done during import of the tagtable *)*> is
//------------------------------------------------------------------------------
*/
static
BOOL WildCardSearch (  PSZ pszInput,      /* input string */
                       PSZ pszTagString)  /* tag string to be matched against */

{
   BOOL fNoMatch;                         /* no match found                   */
   BOOL fMatch;                           /* string match found               */
   BOOL fSubMatch;                        /* match in substring found         */
   BOOL fNoSubMatch;                      /* no match in current substring    */
   PSZ  pszCurInput;                      /* pointer to current input char    */
   PSZ  pszCurTagString;                  /* pointer to current Tagstring char*/
   PSZ  pszSubTag;                        /* pointer to start of substring    */
   PSZ  pszSubInput;                      /* pointer to start of substring    */

   fMatch = FALSE;
   fNoMatch = FALSE;

   pszCurInput = pszInput;                // address start if strings
   pszCurTagString = pszTagString;

   while (!fNoMatch && !fMatch )
   {
      switch (*pszCurTagString)
      {
         //------------------------------------------------------------------------------
         case CHAR_SNGL_SUBST :           // substitute single character,
            if ( *pszCurInput == EOS )   // if available
            {
               fNoMatch = TRUE;
            }
            else
            {
               ++pszCurTagString;
               ++pszCurInput;
            } /* endif */
            break;
         //------------------------------------------------------------------------------
         case CHAR_MULT_SUBST :           // substitute more characters
            ++pszCurTagString;
            if (*pszCurTagString == EOS )
            {
               fMatch = TRUE;               // * substitutes rest of input
            }
            else
            {
               /* compare strings until next * or end of tagstring */
               fSubMatch = FALSE;

               while (!fSubMatch && !fNoMatch)
               {
                  /* find first matching character in input string */
                  pszCurInput = strchr(pszCurInput, *pszCurTagString);
                  if (pszCurInput == NULL)
                  {
                     fNoMatch = TRUE;              // no matching character
                  }
                  else
                  {
                     /* compare substrings */
                     pszSubTag = pszCurTagString;    // remember start of
                     pszSubInput = pszCurInput;     // substring positions
                     fNoSubMatch = FALSE;
                     while (!fSubMatch && !fNoSubMatch && !fNoMatch )
                     {
                        pszCurTagString ++;
                        pszCurInput ++;

                        if ((*pszCurInput == EOS && *pszCurTagString == EOS) ||
                            (*pszCurTagString == CHAR_MULT_SUBST ))
                        {
                           fSubMatch = TRUE;
                        }
                        else if (*pszCurInput == EOS)
                        {
                           fNoMatch = TRUE;
                        }
                        else if (*pszCurTagString != CHAR_SNGL_SUBST &&
                                 *pszCurTagString != *pszCurInput        )
                        {
                           /* characters do not match, reset string */
                           /* positions and try again               */
                           fNoSubMatch = TRUE;
                           pszCurTagString = pszSubTag;
                           pszCurInput = pszSubInput + 1;
                        } /* endif */
                     } /* endwhile */
                  } /* endif */
               } /* endwhile */
            } /* endif */
            break;
         //------------------------------------------------------------------------------
         default :
            if ( *pszCurInput != *pszCurTagString )   // characters not equal
            {
               fNoMatch = TRUE;
            }
            else if (*pszCurTagString == EOS )        // end of strings found
            {
               fMatch = TRUE;
            }
            else
            {
               pszCurTagString ++;                // increase string pointers
               pszCurInput ++;
            } /* endif */
            break;
         //------------------------------------------------------------------------------
      } /* endswitch */
   } /* endwhile */
   return (fMatch);
}/* end WildCardSearch */

/*
//------------------------------------------------------------------------------
  function SkipToTextEnd
  skip text to end of textstring
//------------------------------------------------------------------------------
*/
static
PSZ SkipToTextEnd( PSZ  pszCurTextPos )  // pointer to current text  position

{
   PSZ     pszTextEnd;                     // pointer to end of text

   /* set pointer to end of text block */
   pszTextEnd =  pszCurTextPos;

   while (*pszTextEnd != EOS)
   {
      pszTextEnd ++;
   } /* endwhile */
   return (pszTextEnd -1);
}/* end SkipToTextEnd */

/*
//------------------------------------------------------------------------------
  function SkipTextString

//------------------------------------------------------------------------------
*/
static
PSZ SkipTextString ( PSZ      pInput,       // pointer to input string
                     USHORT * pusPosition   // pointer to column position
                   )
  /* skip text to end of textstring */
{
   BOOL  fEndofLiteral = FALSE;               // is textstring complete?
   PSZ   pTemp;                               // pointer to look-ahead actions
   USHORT usTempPosition;                     // column position for look-ahead
                                              // actions
   if (*pInput == QUOTE)
   {
      while ( ! fEndofLiteral )
      {
         pInput++;                         /* increase input position          */
         (*pusPosition) ++;
         switch (*pInput)
         {
            //------------------------------------------------------------------
            case QUOTE:
               if (*(pInput +1) == QUOTE)
               {
                  (*pusPosition) ++;       /* skip double quote                */
                  pInput ++;
               }
               else
               {
                  fEndofLiteral = TRUE;  /* literal stopped at ending quote  */
               } /* endif */
               break;
            //------------------------------------------------------------------
            case LF:
               /* check for continuation line                                */
               pTemp = pInput +1;        /* start after end of line          */
               usTempPosition = 1;       /* column position is one           */
               while ( (*pTemp == ' ') || (*pTemp == CR) || (*pTemp == LF))
               {
                  if (*pTemp == LF)
                  {
                     usTempPosition = 0;
                  } /* endif */
                  usTempPosition ++;
                  pTemp ++;
               } /* endwhile */

               if (*pTemp != QUOTE)
               {
                  fEndofLiteral = TRUE;  /* no continuation line             */
                  pInput --;             /* reset pInput before newline      */
                  (*pusPosition)--;      /* and adjust column position       */
               }
               else                      /* pTemp is QUOTE                   */
               {
                  pInput = pTemp;    /* continue with continuation line  */
                  (*pusPosition) = usTempPosition;
               } /* endif */
               break;
            //------------------------------------------------------------------
            case EOS:
               fEndofLiteral = TRUE;
               break;
            //------------------------------------------------------------------
            default:
              break;
            //------------------------------------------------------------------
         }/* endswitch */
      }/* endwhile */
   }
   else
   {
      while (!fEndofLiteral)
      {
         (*pusPosition) ++;
         pInput ++;
         if (*pInput == ')' || *pInput == EOS)
         {
            fEndofLiteral = TRUE;
         } /* endif */
      } /* endwhile */
   } /* endif */
   return (pInput);
}/* end SkipTextString */

//------------------------------------------------------------------------------
//   New TagTokenize function TATagTokenize
//   and related defines, macros and functions
//
//   The new approach for tag recognition reduces the time for tag recognition
//   by minimizing the number of compare operation required to check for tags.
//
//   The tag table is converted to a node tree there each node represents a
//   character of the tag table and the links between the nodes are the
//   possible paths for valid tags.
//
//   An example:  Assume a tag table containing the following tags:
//
//                    :TAG1.
//                    :TAG2.
//                    :NONE.
//
//                The node tree for this trivial tag table looks like this:
//
//                                        :
//                                    +---Á---+
//                                    N       T
//
//                                    O       A
//
//                                    N       G
//                                         +--Á--+
//                                    E    1     2
//
//                                    .    .     .
//
//
//                The scan for tags always starts at the nodes in the root
//                level of the tree. If the character in text does not match
//                the root character(s), the character in the text is no tag
//                start.
//
//                If a match occurs the next character in the text is
//                compared with the nodes linked to the matched root character.
//                If again a matching character is found, the check continues
//                on the next level of the node tree until a mismatch occurs
//                or the end of a branch has been reached (in this case a
//                tag has been recognized).
//
//                This approache ensures that only a minimal number of
//                characters has to be compared for tag recognition.
//
//------------------------------------------------------------------------------

#define RETRYSTACKSIZE      50         // retry stack size in # of elements
#define MSSTACKSIZE         10         // mulitple substitution stack size
                                       //    in # of elements

//------------------------------------------------------------------------------
//   Table to compare characters in node trees
//   the index value for the table is:
//
//   0 for multiple substitution characters
//   1 for single substitution characters
//   2 for end of string delimiters (EOS)
//   3 for column based tag characters
//   4 for all other characters
//
//   the first index is the index for the current tag character
//   the second index is the index for current node character
//
//   the table contains -1 if the character is smaller
//                       1 if the character is greater
//                       0 if the characters should be compared
//------------------------------------------------------------------------------
// fix for KBT0022: treat '?' chars as equal
SHORT asCharCompare[5][5] =
{ {  1,  1, -1,  1,  1 },
  { -1,  0, -1,  1,  1 },
  {  1,  1,  1,  1,  1 },
  { -1, -1, -1,  0, -1 },
  { -1, -1, -1,  1,  0 } };



//------------------------------------------------------------------------------
//   Update column position macro: increment column position, set to
//   zero if line end is reached
//------------------------------------------------------------------------------
#define UPDATECOLPOS( chChar, usColPos )    \
  if ( ( chChar == CR) || (chChar == LF))   \
  {                                         \
    usColPos = 0;                           \
  }                                         \
  else                                      \
  {                                         \
    usColPos++;                             \
  } /* endif */

//------------------------------------------------------------------------------
//   Table for uppercase conversion
//   the index into the table is the character being uppercased, the value
//   at the table position is the uppercased character
//------------------------------------------------------------------------------
BYTE chLookup[256] =
   { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
     0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
     0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
     0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
     0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
     0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
     0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
     0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
     0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
     0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
     0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
     0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
     0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
     0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
     0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
     0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
     0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
     0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
     0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
     0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
     0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
     0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

USHORT chLookupW[256] =
   { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
     0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
     0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
     0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
     0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
     0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
     0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
     0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
     0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
     0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
     0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
     0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
     0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
     0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
     0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
     0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
     0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
     0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
     0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
     0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
     0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
     0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };


//------------------------------------------------------------------------------
//   Table for whitespace recognition
//   the index into the table is the character being tested, the value
//   at the table position is 0 if the character is no whitespace character or
//   1 if the character is a whitespace character
//------------------------------------------------------------------------------
CHAR fWhiteSpace[256] =
   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TATagTokenize
//------------------------------------------------------------------------------
// Function call:     TATagTokenize( PSZ pszInput, PLOADEDTABLE pVoidTable,
//                                   BOOL  fComplete, PSZ * &pRest,
//                                   PUSHORT &usColPos, PTOKENENTRY pTokBuf,
//                                   USHORT usTokens );
//------------------------------------------------------------------------------
// Description:       Mainline for tag tokenization based on tag tables.
//------------------------------------------------------------------------------
// Parameters:        PSZ          pszInput    pointer to input data
//                    PLOADEDTABLE pVoidTable  pointer to loaded tag table
//                    BOOL         fComplete   TRUE = no more buffers to follow
//                    PSZ          *ppRest     pointer to not processed data in
//                                             buffer
//                    PUSHORT      pusColPos   column position
//                    PTOKENENTRY  pTokBuf     buffer for created tokens
//                    USHORT       usTokens    max entries in token buffer
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//                         TRUE  = succesful tokenized
//                         FALSE = something went wrong (main reason for this
//                                 are memory allocation problems)
//------------------------------------------------------------------------------
// Prerequesits:      Tag table pointed to by pVoidTable must have been loaded
//                    using TALoadtagTable function.
//------------------------------------------------------------------------------
// Function flow:     Initialize tokenize status area
//                    Create tag trees if not done yet
//                    Loop while there is data and token buffer is not full
//                       match next tag
//                       if a tag was found then
//                          Build token for data up to start of recognized tag
//                          Add tag token
//                          if tag may have attributes then
//                          do
//                             match attribute
//                          while attributes are found and token buffer is
//                            not full
//                       else
//                         if data is complete then
//                            add remaining data as TEXT token
//                         else
//                            process remaining data the next time ...
//                         endif
//                       endif
//                    endloop
//                    if not processed completely or not last part of file
//                    set pRest pointer to begin of last tag or (if no tags
//                    are contained in data) to last linebreak.
//------------------------------------------------------------------------------
BOOL TATagTokenizeW
(
   PSZ_W       pszInput,                 // pointer to input data
   PLOADEDTABLE pVoidTable,            // pointer to loaded tag table
   BOOL      fComplete,                // TRUE = no more buffers to follow
   PSZ_W      *ppRest,                   // pointer to not processed data in buffer
   PUSHORT   pusColPos,                // column position
   PTOKENENTRY pTokBuf,                // buffer for created tokens
   USHORT      usTokens                  // max entries in token buffer
)
{
   PSZ_W       pszData;                // pointer for input data processing
   PSZ_W       pszDataStart;           // pointer for end of data block
   PTOKENENTRY pToken;                 // pointer to current token
   PSZ_W       pszEnd;                 // pointer to end of data
   BOOL        fFound = FALSE;         // flag used in setting pRest
   PLOADEDTABLE pTable = (PLOADEDTABLE)pVoidTable;   // pointer to loaded tag table
   TOKSTATUS   TokStatus;              // tokenize status area
   TOKENENTRY  NewToken;               // buffer for new tokens
   MSSTACK     MSStack[10];
   PTAG        pTag;                   // pointer to first tag in tagtable
   PTOKENENTRY pLastTag = NULL;        // pointer to last tag token
   USHORT      usTagColPos = 0;        // column position of last tag
   USHORT      usCurColPos;            // current column position
   BOOL        fProcessingAttributes;  // TRUE = attributes are processed
   PATTRIBUTE  pAttr;                  // ptr to attribute array of tag table
   PBYTE       pByte;                  // helper pointer
   SHORT       sTags;                  // number of tags in tag table
   BOOL        fOK = TRUE;             // function return code
   PTAGADDINFO pTagAddInfo;            // additional taginfo (tokenid, classid)

   // address tag and attribute array
   pByte = (PBYTE)pTable->pTagTable;
   pTag = (PTAG) (pByte + pTable->pTagTable->stFixTag.uOffset);
   pAttr = (PATTRIBUTE) ( pByte + pTable->pTagTable->stAttribute.uOffset);
   sTags = (SHORT)pTable->pTagTable->uNumTags;

   if ( pTable->pTagTable->usVersion >= ADDINFO_VERSION )
   {
     pTagAddInfo = (PTAGADDINFO) (pByte + pTable->pTagTable->uAddInfos);
   }
   else
   {
     pTagAddInfo = NULL;
   } /* endif */


   /*******************************************************************/
   /* Initialize tokenize status area                                 */
   /*******************************************************************/
   memset( &TokStatus, 0, sizeof(TokStatus) );
   TokStatus.usColPos = *pusColPos;

   TokStatus.chSingleSubst = pTable->chSingleSubstW;
   TokStatus.chMultSubst   = pTable->chMultSubstW;
   TokStatus.usMSStackSize = 10;
   TokStatus.pMSStack      = MSStack;
   TokStatus.pTable        = pTable;

   /*******************************************************************/
   /* Create tag trees if not done yet                                */
   /*******************************************************************/
   if ( pTable->pNodeArea == NULL )
   {
     // additional parameter needed for pTagNamesW
     pTable->pNodeArea = TACreateTagTree( pTable,
                                          pTable->pTagTable,
                                          TokStatus.chMultSubst,
                                          TokStatus.chSingleSubst,
                                          TRUE );
     fOK = ( pTable->pNodeArea != NULL );
   } /* endif */

   if ( fOK && (pTable->pAttrNodeArea == NULL) )
   {
     pTable->pAttrNodeArea = TACreateAttrTree( pTable,
                                               pTable->pTagTable,
                                               TokStatus.chMultSubst,
                                               TokStatus.chSingleSubst,
                                               TRUE );
     fOK = ( pTable->pAttrNodeArea != NULL );
   } /* endif */

   pTag = (PTAG)((PBYTE)pTable->pTagTable + pTable->pTagTable->stFixTag.uOffset );

   pszData = pszInput;                 // start at begin of input buffer
   pToken  = pTokBuf;                  // start at begin of token buffer
   pToken->sTokenid = TEXT_TOKEN;      // assume text data
   pToken->pDataStringW = pszData;      // remember start of data
   usTokens--;                         // reserve space for end of list token
   fProcessingAttributes = FALSE;      // we are waiting for tags ...

   while( fOK && *pszData && usTokens )// while not end of data and free tokens
   {
     USHORT      usProcessFlag;        // processing flag        /* @KWT0010A */

//   the following statement has been disabled to fix PTM KAT0330
//   fProcessingAttributes = FALSE;    // we are waiting for tags ...
     pszDataStart = pszData;
     usCurColPos  = TokStatus.usColPos;
     usProcessFlag = (fComplete ) ?                              /* @KWT0010A */
                      ( MATCH_SKIP_DATA|MATCH_DATA_COMPLETE) : MATCH_SKIP_DATA;
     fFound = TAMatchTag( &TokStatus,
                           &pszData,
                           pTable->pNodeArea->pRootNode,
                           &NewToken,
                           usProcessFlag);                       /* @KWT0010M */
     if ( fFound )
     {
       fProcessingAttributes = FALSE;

       /*************************************************************/
       /* Build token for data up to start of recognized tag        */
       /*************************************************************/
       if ( pszDataStart != NewToken.pDataStringW )
       {
         pToken->sTokenid = TEXT_TOKEN;      // text data
         pToken->pDataStringW = pszDataStart; // set start of data
         pToken->usLength = (USHORT)(NewToken.pDataStringW - pszDataStart);
         pToken->sAddInfo = 0;
         pToken->ClassId = CLS_DEFAULT;
         pToken->usOrgId = 0;
         usTokens--;
         pToken++;
       } /* endif */

       /*************************************************************/
       /* Add tag token                                             */
       /*************************************************************/
       if ( usTokens )
       {
         // special handling for tags which may be followed by attributes:
         // check if tag end character is a blank, when it is one look-ahead
         // for first non-blank character, when this non-blank character is
         // a valid end character for this tag include all blanks in tag
         if ( (NewToken.sTokenid >= 0) && pTag[NewToken.sTokenid].fAttr && (pTag[NewToken.sTokenid].uEndDelimOffs != 0) )
         {
           PSZ_W pszTest = NewToken.pDataStringW + (NewToken.usLength - 1);

           // only for blank end delimiters...
           if ( *pszTest == L' ')
           {
             PSZ_W pszEndDelim = pTable->pTagNamesW + pTag[NewToken.sTokenid].uEndDelimOffs;

             // find first non-blank character
             while ( *pszTest == L' ' )
             {
               pszTest++;
             } /*endwhile */

             // check if this character is a valid end-of-tag character
             if ( *pszTest != 0 )
             {
               while ( (*pszEndDelim != 0)  && (*pszEndDelim != *pszTest) )
               {
                 pszEndDelim++;
               } /*endwhile */

               // adjust tag length and scan position when valid end character found
               if ( *pszEndDelim != 0 )
               {
                 pszData = pszTest + 1;
                 NewToken.usLength = (USHORT)(pszData - NewToken.pDataStringW);
               } /* endif */
             } /* endif */
           } /* endif */
         } /* endif */

         // add tag token to token list
         pLastTag    = pToken;
         usTagColPos = TokStatus.usStartColPos;
         memcpy( pToken, &NewToken, sizeof(TOKENENTRY) );
         pToken->sAddInfo = pTag[NewToken.sTokenid].BitFlags.AddInfo;
         if ( pTagAddInfo )
         {
           pToken->ClassId = pTagAddInfo[NewToken.sTokenid].ClassId;
           pToken->usOrgId = pTagAddInfo[NewToken.sTokenid].usFixedTagId;
         } /* endif */
         pToken++;
         usTokens--;
       } /* endif */

       /**********************************************************/
       /* Process attributes if tag has any                      */
       /**********************************************************/
       if ( (NewToken.sTokenid >= 0) && pTag[NewToken.sTokenid].fAttr )
       {
         fProcessingAttributes = TRUE;      // attributes may follow ...
         do
         {
           /***********************************************************/
           /* If attributes are not delimited by whitespace           */
           /* characters they will be ignored (this makes sense only  */
           /* for bookmaster tags (e.g. :DL. COMPACT) but had to be   */
           /* added to be compatible to the old EQFTagTokenize        */
           /* function)                                               */
           /***********************************************************/
           /*********************************************************/
           /* Check the last character of the tag                   */
           /*********************************************************/
           if ( !fWhiteSpace[NewToken.pDataStringW[NewToken.usLength-1]] )
           {
             fFound = FALSE;
             fProcessingAttributes = FALSE;  // no more attributes
           }
           else
           {
             fFound = TRUE;
           } /* endif */

           /***********************************************************/
           /* Check for attributes                                    */
           /***********************************************************/
           if ( fFound )
           {
             usCurColPos  = TokStatus.usColPos;
             fFound = TAMatchTag( &TokStatus,
                                   &pszData,
                                   pTable->pAttrNodeArea->pRootNode,
                                   &NewToken,
                                   MATCH_SKIP_WHITESPACE |
                                   MATCH_QUOTED_STRINGS );
             // the following code has been disabled to fix KAT0330
             // if no attribute has been found the pszData pointer still
             // points to the begin of the maybe attribute ...
             // if ( !fFound && *pszData )
             // {
             //   /*******************************************************/
             //   /* Attribute mismatch, but not end of text block       */
             //   /*******************************************************/
             //   fProcessingAttributes = FALSE;  // no more attributes
             // } /* endif */
           } /* endif */

           if ( fFound && usTokens )
           {
             memcpy( pToken, &NewToken, sizeof(TOKENENTRY) );
             pToken->sAddInfo = pAttr[NewToken.sTokenid-sTags].BitFlags.AddInfo;

             if ( pTagAddInfo )
             {
               pToken->ClassId = pTagAddInfo[NewToken.sTokenid].ClassId;
               pToken->usOrgId = pTagAddInfo[NewToken.sTokenid].usFixedTagId;
             } /* endif */
             pToken++;
             usTokens--;
           } /* endif */
         } while ( fFound && usTokens ); /* enddo */
       } /* endif */
     }
     else
     {
       /*************************************************************/
       /* No tag in remaining data found                            */
       /*************************************************************/
       if ( fComplete )
       {
         /***********************************************************/
         /* Add remaining data as TEXT token                        */
         /***********************************************************/
         pToken->sTokenid = TEXT_TOKEN;      // text data
         pToken->pDataStringW = pszDataStart; // set start of data
         pToken->usLength = (USHORT)UTF16strlenCHAR(pszDataStart);
         pszData = pszDataStart + pToken->usLength;
         usTokens--;
         pToken++;
       }
       else
       {
         /*************************************************************/
         /* Process remaining data the next time ...                  */
         /*************************************************************/
         pszData = pszDataStart + UTF16strlenCHAR(pszDataStart);
       } /* endif */
     } /* endif */
   } /* endwhile */

   // if not processed completely or not last part of file
   // set pRest pointer to begin of last tag or (if no tags
   // are contained in data) to last linebreak.
   if ( !fOK )
   {
     // nothing was processed due to errors (memory allocation?)
     *ppRest = pszInput;
   }
   else if ( !usTokens ||              // end of token table reached
             !fComplete )              // or not last block of data  ???
   {
     // at least two tokens in buffer ....
     if ( (pToken != pTokBuf) && ( pToken != (pTokBuf+1)) )   // if tokens in buffer
     {
        /**************************************************************/
        /* There are tokens in the buffer, let's see how we have to   */
        /*  deal with them ...                                        */
        /**************************************************************/

        // go back to the last tag to allow tag attribute and whitespace
        // processing in the next tokenization block (unless this tag
        // is the first token in the token buffer or the remaining data
        // is longer than twice the max segment size )
        int iRemainLength = UTF16strlenCHAR(pToken[-1].pDataStringW + pToken[-1].usLength);
        if ( ( (pLastTag != pTokBuf) &&                    // more than one tag in buffer
               (iRemainLength < (2*MAX_SEGMENT_SIZE)) ) || // rest is smaller than 2 segments
             (usTokens == 0) )                             // token buffer overflow
        {
          pToken = pLastTag;
          TokStatus.usColPos = usTagColPos;
          *ppRest = pToken->pDataStringW;
        }
        else
        {
          // continue right behind the last recognized token
          *ppRest = pToken[-1].pDataStringW + pToken[-1].usLength;
          if ( (*pszData == EOS) && (*ppRest >= pszData) )
          {
            // no more data to follow, segment has been processed completely
            *ppRest = NULL;
          } /* endif */
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* The token buffer is empty, no tags were recognized ...     */
        /* In addition we came here if only one tag is recognized...  */
        /*                                                            */
        /* Go back to the last LF and add that data as a TEXT token   */
        /**************************************************************/
        PSZ_W pszAreaStart = NULL;
        pszEnd = pszData;
        pszData--;
        if ( pToken == pTokBuf )
        { 
           // no token in buffer
           pszAreaStart = pszInput;     // set start of data
        }
        else
        {
           // one token in buffer
           pszAreaStart = pToken[-1].pDataStringW + pToken[-1].usLength;
        } /* endif */            

        while ( pszData > pszAreaStart )
        {
          if ( *pszData == LF )
          {
             break;                  // leave while loop
          }
          else
          {
             pszData--;              // test previous character
          } /* endif */
        } /* endwhile */

        if ( pszData == pszInput )    // no linefeed found ???
        {
            *ppRest = pszEnd - 1;      // set pRest to end of data CHECK in DEBUG
        }
        else
        {
            *ppRest = pszData;         // set pRest to last linefeed
        } /* endif */
        pToken->sTokenid    = TEXT_TOKEN;   // text data
        pToken->pDataStringW = pszAreaStart;     // set start of data
        pToken->usLength    = (USHORT)(*ppRest - pszAreaStart);
        
        usTokens--;
        pToken++;
      } /* endif */
   }
   else
   {
      *ppRest = NULL;
   } /* endif */

   // add end-of-list token
   if ( fOK )
   {
     memset( pToken, 0, sizeof(TOKENENTRY) );
     pToken->usLength = 0;
     pToken->sTokenid = ENDOFLIST;

     *pusColPos = TokStatus.usColPos;
   } /* endif */

   /*******************************************************************/
   /* cleanup                                                         */
   /*******************************************************************/
   if ( TokStatus.pRetryStack != NULL )
   {
     UtlAlloc( (PVOID *)&TokStatus.pRetryStack, 0L, 0L, NOMSG );
   } /* endif */

   return( fOK );
} /* end of function TATagTokenize */


BOOL TATagTokenize
(
   PSZ      pszInput,                // pointer to input data
   PLOADEDTABLE pVoidTable,            // pointer to loaded tag table
   BOOL      fComplete,                // TRUE = no more buffers to follow
   PSZ      *ppRest,                  // pointer to not processed data in buffer
   PUSHORT   pusColPos,                // column position
   PTOKENENTRY pTokBuf,                // buffer for created tokens
   USHORT      usTokens,                  // max entries in token buffer
   ULONG       ulInputCP                  // ASCII/OEM cp of input data
)
{
   BOOL       fOK = TRUE;
   PSZ_W        pRestW = NULL;
   PSZ_W        pszInputW = &chInputW[0];
   ULONG      ulUsed = strlen(pszInput)+1;
   USHORT     usTokLengthW = 0;
   ULONG      ulBytesInAscii;
   PTOKENENTRY pNextTokBuf;
   PTOKENENTRY pCurTokBuf;

   if ( ulUsed > sizeof( chInputW) )
   {
     assert( ulUsed > sizeof(chInputW ));
   }
   memset( &chInputW, 0, sizeof( chInputW ));
   ASCII2UnicodeBuf( pszInput, pszInputW, (USHORT) ulUsed, ulInputCP );

   fOK = TATagTokenizeW( pszInputW, pVoidTable, fComplete, &pRestW, pusColPos, pTokBuf, usTokens);
   // readjust return values
   pCurTokBuf = pTokBuf;
   if ( pCurTokBuf->pDataStringW)
   {
     if ( pCurTokBuf->pDataStringW == pszInputW)
     {
        pCurTokBuf->pDataString = pszInput;
     }
     else
     {
       ulBytesInAscii = WideCharToMultiByte( ulInputCP, 0, pszInputW,
                                             pCurTokBuf->pDataStringW - pszInputW,
                                            NULL, 0 , NULL, NULL );
       pCurTokBuf->pDataString = pszInput + ulBytesInAscii;
     }
   } /* endwhile */

   while (pCurTokBuf->pDataStringW)
   {
       usTokLengthW = pCurTokBuf->usLength;

       ulBytesInAscii = WideCharToMultiByte( ulInputCP, 0, pCurTokBuf->pDataStringW,
                                             usTokLengthW, NULL, 0 , NULL, NULL );
       pCurTokBuf->usLength = (USHORT)ulBytesInAscii;
       // set next pDataString ptr
       pNextTokBuf = pCurTokBuf + 1;
       if ( pCurTokBuf->pDataStringW + usTokLengthW == pNextTokBuf->pDataStringW)
       {
         pNextTokBuf->pDataString = pCurTokBuf->pDataString + ulBytesInAscii;
       }
       else
       {
         ulBytesInAscii = WideCharToMultiByte( ulInputCP, 0, pCurTokBuf->pDataStringW,
                                                pNextTokBuf->pDataStringW - pCurTokBuf->pDataStringW ,
                                                NULL, 0 , NULL, NULL );
         pNextTokBuf->pDataString = pCurTokBuf->pDataString + ulBytesInAscii;
       }
       pCurTokBuf->pDataStringW = NULL;
       pCurTokBuf++;
    }
    if (pRestW != NULL)
    {
      ulUsed = pRestW - pszInputW;
      ulBytesInAscii = WideCharToMultiByte( ulInputCP, 0, pszInputW,
                                            pRestW - pszInputW,
                                            NULL, 0, NULL, NULL );
      (*ppRest) = pszInput + ulBytesInAscii;
    }
    else
    {
        *ppRest = NULL;
    }

   return fOK;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TAMatchTag
//------------------------------------------------------------------------------
// Function call:     TAMatchTag( PSZ *ppszData, PTREENODE pNode,
//                                PTOKENENTRY pToken, USHORT usFlag );
//------------------------------------------------------------------------------
// Description:       Look for a tag match in the input data.
//------------------------------------------------------------------------------
// Parameters:        PTOKSTATUS  pTokStatus  ptr to tokenize status area
//                    PSZ         *ppszData   pointer to current position
//                    PTREENODE   pNode       root node of node tree
//                    PTOKENENTRY pToken      ptr to token buffer for results
//                    USHORT      usFlag      flags for processing
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     a tag has been matched
//                    FALSE    no tag found in input data
//------------------------------------------------------------------------------
BOOL TAMatchTag
(
  PTOKSTATUS  pTokStatus,              // ptr to tokenize status area
  PSZ_W         *ppszData,               // pointer to current position in data
  PTREENODE   pNode,                   // root node of node tree for matching
  PTOKENENTRY pToken,                  // ptr to token buffer for results
  USHORT      usFlags                  // flags for processing
)
{
  PSZ_W         pszData;                 // ptr to current position
  PSZ_W         pszStartPos;             // ptr to data start position
  PSZ_W         pszTag;                  // ptr to start of attribute
  PSZ_W         pszEndDel;               // ptr for end delimiter processing
  CHAR_W        chTest;                  // character being tested
  USHORT      usMSElement = 0;         // number of current MS stack element
  USHORT      usRetryStackElement = 0; // number of current MS stack element
  BOOL        fQuote;                  // flag for quoted strings processing
  PTREENODE   pFirstNode;              // start of node tree
  ULONG       ulTagLen;                // length of tag
  USHORT      usColPos;                // current column position
  USHORT      usTagCol;                // tag start column position
  CHAR_W      chTemp;
  enum        PROCESS_STATES           // states during processing:
  {
    IN_ERROR_STATE,                    // an error occured
    TAG_MATCH_STATE,                   // text matches tag
    TAG_NOMATCH_STATE,                 // text does not match tag
    WORKING_STATE                      // during tag matching
  } ProcState = WORKING_STATE;

  /********************************************************************/
  /* Start at supplied data pointer                                   */
  /********************************************************************/
  pszData    = *ppszData;
  pFirstNode = pNode;
  usColPos   = pTokStatus->usColPos;

  /********************************************************************/
  /* Ignore any whitespace (whitespace table returns TRUE for         */
  /* LF, CR, TAB and BLANK)                                           */
  /********************************************************************/
  if ( usFlags & MATCH_SKIP_WHITESPACE )
  {
    CHAR_W c;
    while ( ((c = *pszData) <= 0x0ff) && fWhiteSpace[c] )
    {
      UPDATECOLPOS( *pszData, usColPos );
      pszData++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Check for tag                                                    */
  /********************************************************************/
  pszStartPos = pszData;
  pszTag   = pszData;                  // remember start of tag
  usTagCol = usColPos;

  do
  {
    UPDATECOLPOS( *pszData, usColPos );
    chTest = ((chTest = *pszData++) <= 0x0ff ) ? chLookupW[chTest] : chTest;

    /****************************************************************/
    /* Look for character on current level of node list             */
    /****************************************************************/
    while ( pNode &&
            ((pNode->chNode != chTest) ||
             (pNode->usColPos && (pNode->usColPos != usColPos))) &&
            (pNode->chNode != pTokStatus->chMultSubst) &&
            (pNode->chNode != pTokStatus->chSingleSubst) &&
            (pNode->chNode != EOS) )
    {
       pNode = pNode->pNextChar;
    } /* endwhile */

    if ( pNode )
    {
      /****************************************************************/
      /* Check if the selected path is unique; if not add current     */
      /* position to retry stack to allow a retry at this position    */
      /* (but only if current position is not on stack already!)      */
      /****************************************************************/
      if ( !pNode->fUnique && pNode->pNextChar )
      {
        /**************************************************************/
        /* Check if current position is already on retry stack        */
        /**************************************************************/
        if ( (usRetryStackElement == 0) ||
             (pTokStatus->pRetryStack[usRetryStackElement-1].pNode !=
                         pNode->pNextChar ) )
        {
          /**************************************************************/
          /* Enlarge stack if stack limits have been reached            */
          /**************************************************************/
          if ( usRetryStackElement >= pTokStatus->usRetryStackSize )
          {
            if ( UtlAlloc( (PVOID *)&pTokStatus->pRetryStack,
                      (LONG)(pTokStatus->usRetryStackSize * sizeof(RETRYSTACK)),
                      (LONG)((pTokStatus->usRetryStackSize + RETRYSTACKSIZE ) *
                                                            sizeof(RETRYSTACK)),
                      (USHORT)(( pTokStatus->fMsg ) ? ERROR_STORAGE : NOMSG) ) )
            {
              pTokStatus->usRetryStackSize += RETRYSTACKSIZE;

            }
            else
            {
              ProcState = IN_ERROR_STATE;
            } /* endif */
          } /* endif */

          /**************************************************************/
          /* push retry position on retry stack                         */
          /**************************************************************/
          if ( ProcState != IN_ERROR_STATE )
          {
            pTokStatus->pRetryStack[usRetryStackElement].pNode = pNode->pNextChar;
            pTokStatus->pRetryStack[usRetryStackElement].pszData = pszData - 1;
            pTokStatus->pRetryStack[usRetryStackElement].pszTag = pszTag;
            usRetryStackElement++;
          } /* endif */
        } /* endif */
      } /* endif */

      /**************************************************************/
      /* Character has been located on curent level of node tree    */
      /**************************************************************/
      if ( pNode->chNode == pTokStatus->chMultSubst )
      {
        /**************************************************************/
        /* Handle strings enclosed in quotes                          */
        /**************************************************************/
        if ( ((usFlags & MATCH_QUOTED_STRINGS) || pNode->fTransInfo)
                          &&
             ((chTest == QUOTE) || (chTest == DBLQUOTE)) )
        {
          CHAR_W chQuote = chTest;

          fQuote = TRUE;
          while (  fQuote && ((chTest = ((chTest = *pszData) <= 0x0ff ) ? chLookupW[chTest] : chTest) != 0))
          //while (chTest = chLookupW[(UCHAR)*pszData]) && fQuote )
          {
            if ( chTest == chQuote )
            {
              /********************************************/
              /* if two consecutive QUOTES we are dealing */
              /* with inline quote else we are done..     */
              /********************************************/
              chTemp = ((chTemp = *(pszData+1)) <= 0x0ff ) ? chLookupW[chTemp] : chTemp;
              if ( chTemp != chQuote )
              //if ( chLookupW[(UCHAR)*(pszData+1)] != chQuote )
              {
                fQuote = FALSE;
              }
              else
              {
                pszData += 2;
                usColPos += 2;
              } /* endif */
            }
            else
            {
              UPDATECOLPOS( *pszData, usColPos );
              pszData++;
            } /* endif */
          } /* endwhile */

          if ( chTest != EOS )
          {
            UPDATECOLPOS( *pszData, usColPos );
            pszData++;                   // point to next active charact.
            UPDATECOLPOS( *pszData, usColPos );
            chTest = ((chTest = *pszData++) <= 0x0ff ) ? chLookupW[chTest] : chTest;
            //chTest = chLookupW[(UCHAR)*pszData++]; // and use this character for testing
          }
          else
          {
            pszData--;                // closing quote not contained in data
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Check for tag end delimiter or for tag length              */
        /**************************************************************/
        if ( pNode->usLength )
        {
          ulTagLen = (pszData - pszTag) - 1;  //@@ DEBUG here!
          if ( ulTagLen == pNode->usLength )
          {
            /********************************************************/
            /* End of length-terminated tag has been reached        */
            /********************************************************/
            if ( pNode->pNextLevel->chNode == EOS )
            {
              ProcState = TAG_MATCH_STATE;
              pToken->sTokenid    = pNode->sTokenID;
              pToken->pDataStringW = pszTag;
              pszData--;
              usColPos--;
              pToken->usLength    = (USHORT)(pszData - pszTag);
            }
            else
            {
              /********************************************************/
              /* end of tag but not end of pattern string ...         */
              /********************************************************/
              ProcState = TAG_NOMATCH_STATE;
              pszData--;
              usColPos--;
            } /* endif */
          }
          else if ( ulTagLen > pNode->usLength )
          {
            /********************************************************/
            /* Tag will not match as length has been                */
            /* exceeded                                             */
            /********************************************************/
            ProcState = TAG_NOMATCH_STATE;
            pszData--;
            usColPos--;
          } /* endif */
        }
        else
        {
          /************************************************************/
          /* Check for tag end when pattern string ends               */
          /************************************************************/
          if ( pNode->pNextLevel->chNode == EOS )
          {
            pszEndDel = pNode->pszEndDel;
            while ( *pszEndDel )
            {
              if ( (CHAR_W)*pszEndDel == chTest )
              {
                break;
              }
              else
              {
                pszEndDel++;
              } /* endif */
            } /* endwhile */
            if ( *pszEndDel )
            {
              ProcState           = TAG_MATCH_STATE;
              pToken->sTokenid    = pNode->sTokenID;
              pToken->pDataStringW = pszTag;
              pToken->usLength    = (USHORT)(pszData - pszTag);
            } /* endif */
          } /* endif */
        } /* endif */

        if (ProcState == WORKING_STATE)
        {
          if ( (pNode->pNextLevel->chNode == chTest) )
          {
            /************************************************/
            /* Push current node to multiple substitution   */
            /* stack and follow the node path               */
            /************************************************/
            if ( usMSElement <  pTokStatus->usMSStackSize )
            {
              pTokStatus->pMSStack[usMSElement].pNode = pNode;
              usMSElement++;
            } /* endif */

            pNode = pNode->pNextLevel;
            pszData--;
            usColPos--;
          }
          else
          {
            /**********************************************************/
            /* check all further multiple substitution tags if they   */
            /* may be a matching starting point...                    */
            /**********************************************************/
            PTREENODE   pTempNode  = pNode;
            while ( pTempNode->pNextChar )
            {
              pTempNode = pTempNode->pNextChar;
              if ( pTempNode->pNextLevel &&
                   (pTempNode->pNextLevel->chNode == chTest) )
              {
                /************************************************/
                /* Push current node to multiple substitution   */
                /* stack and follow the node path               */
                /************************************************/
                if ( usMSElement <  pTokStatus->usMSStackSize )
                {
                  pTokStatus->pMSStack[usMSElement].pNode = pNode;
                  usMSElement++;
                } /* endif */

                pNode = pTempNode->pNextLevel;
                pszData--;
                usColPos--;
                break;
              } /* endif */
            } /* endwhile */
          } /* endif */
        } /* endif */
      }
      else if ( pNode->chNode == EOS )
      {
        /**************************************************************/
        /* Pattern string has ended, check if current tag has ended   */
        /* also                                                       */
        /**************************************************************/

        /************************************************/
        /* Check for tag end delimiter or for tag length*/
        /************************************************/
        if ( pNode->usLength )
        {
          ulTagLen = (pszData - pszTag) - 1;
          if ( ulTagLen == pNode->usLength )
          {
            /********************************************************/
            /* End of length terminated tag has been reached        */
            /********************************************************/
            ProcState = TAG_MATCH_STATE;
            pToken->sTokenid    = pNode->sTokenID;
            pToken->pDataStringW = pszTag;
            pszData--;
            usColPos--;
            pToken->usLength    = (USHORT)(pszData - pszTag);
          }
          else
          {
            /********************************************************/
            /* Attribute will not match as length does not match    */
            /********************************************************/
            ProcState = TAG_NOMATCH_STATE;
            pszData--;
            usColPos--;
          } /* endif */
        }
        else
        {
          pszEndDel = pNode->pszEndDel;
          while ( *pszEndDel )
          {
            if ( (CHAR_W)*pszEndDel == chTest )
            {
              break;                   // valid end delimiter, leave loop
            }
            else
            {
              pszEndDel++;
            } /* endif */
          } /* endwhile */

          /************************************************/
          /* If found, fill-in attribute data in token    */
          /* otherwise check redo stack, redo or end                  */
          /* loop depending on stack                                  */
          /************************************************/
          if ( *pszEndDel )
          {
            ProcState = TAG_MATCH_STATE;
            pToken->sTokenid    = pNode->sTokenID;
            pToken->pDataStringW = pszTag;
            pToken->usLength    = (USHORT)(pszData - pszTag);
          }
          else
          {
            /****************************************************/
            /* Mismatch of currently tested character:          */
            /*   If multiple-substitution-stack is active       */
            /*      reset node position to last element on stack*/
            /*   else                                           */
            /*      leave attribute recognition loop            */
            /****************************************************/
            if ( usMSElement != 0 )                              /* @KWT0034A */
            {                                                    /* @KWT0034A */
              usMSElement--;                                     /* @KWT0034A */
              pNode = pTokStatus->pMSStack[usMSElement].pNode;   /* @KWT0034A */
              pszData--;                                         /* @KWT0034A */
            }                                                    /* @KWT0034A */
            else                                                 /* @KWT0034A */
            {                                                    /* @KWT0034A */
              /************************************************/ /* @KWT0034M */
              /* Leave loop                                   */ /* @KWT0034M */
              /************************************************/ /* @KWT0034M */
              ProcState = TAG_NOMATCH_STATE;                     /* @KWT0034M */
              pszData--;                                         /* @KWT0034M */
              usColPos--;                                        /* @KWT0034M */
            } /* endif */                                        /* @KWT0034A */
          } /* endif */
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* Check if tag end delimiter has been reached or tag length */
        /* has been exceeded.                                         */
        /**************************************************************/
        if ( pNode->usLength )
        {
          ulTagLen = (pszData - pszTag) - 1;
          if ( ulTagLen >= pNode->usLength )
          {
            /**********************************************************/
            /* Tag is not at it's end but tag length has been         */
            /* exceeded                                               */
            /**********************************************************/
            ProcState = TAG_NOMATCH_STATE;
            pszData--;
            usColPos--;
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Follow current path if no mismatch yet                     */
        /**************************************************************/
        if ( ProcState != TAG_NOMATCH_STATE )
        {
          if ( pNode->pNextLevel )
          {
            /************************************************************/
            /* The are more levels to follow ...                        */
            /************************************************************/
            pNode = pNode->pNextLevel;
          }
        } /* endif */
      } /* endif */
    }
    else
    {
      /****************************************************/
      /* Mismatch of currently tested character:          */
      /*   If multiple-substitution-stack is active       */
      /*      reset node position to last element on stack*/
      /*   else                                           */
      /*      leave attribute recognition loop            */
      /****************************************************/
      if ( usMSElement != 0 )
      {
        usMSElement--;
        pNode = pTokStatus->pMSStack[usMSElement].pNode;
        pszData--;
      }
      else
      {
        /**************************************************/
        /* Leave loop                                      */
        /**************************************************/
        ProcState = TAG_NOMATCH_STATE;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Attribute mismatch detected, check the retry stack leave       */
    /* loop if stack is empty else pop stack entry and retry          */
    /******************************************************************/
    if ( ProcState == TAG_NOMATCH_STATE )
    {
      if ( usRetryStackElement )
      {
        usRetryStackElement--;
        pNode   = pTokStatus->pRetryStack[usRetryStackElement].pNode;
        pszData = pTokStatus->pRetryStack[usRetryStackElement].pszData;
        pszTag  = pTokStatus->pRetryStack[usRetryStackElement].pszTag;

        usMSElement = 0;               // reset multiiple-substitution-stack

        ProcState = WORKING_STATE;

        if ( pszData == pszStartPos )
        {
          chTest = 0x20;               // dummy value (not equal to EOS!)
        }
        else
        {
          chTest = ((chTest = *(pszData-1)) <= 0x0ff ) ? chLookupW[chTest] : chTest;
          //chTest = chLookupW[(UCHAR)(pszData[-1])];// reset to old value
        } /* endif */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* continue search if we are in WORKING_STATE and already at end  */
    /* of passed data, but the passed data are a complete record.     */
    /******************************************************************/
    if ( (!chTest) && (usFlags & MATCH_DATA_COMPLETE) &&         /* @KWT0010A */
         (ProcState == WORKING_STATE)  )                         /* @KWT0010A */
    {                                                            /* @KWT0010A */
      /************************************************/         /* @KWT0010A */
      /* reset state to no tag match - dirty work of  */         /* @KWT0010A */
      /* resetting will be done below ....            */         /* @KWT0010A */
      /************************************************/         /* @KWT0010A */
      ProcState  = TAG_NOMATCH_STATE;                            /* @KWT0010A */
      chTest     = *pszTag;    /* reset to try again  */         /* @KWT0010A */
    } /* endif */                                                /* @KWT0010A */
    /******************************************************************/
    /* Continue search if we are in TAG_NOMATCH_STATE and             */
    /* MATCH_SKIP_DATA has been set                                   */
    /******************************************************************/
    if ( (ProcState == TAG_NOMATCH_STATE) && (usFlags & MATCH_SKIP_DATA) )
    {
      ProcState  = WORKING_STATE;      // reset state to "Working"
      usColPos   = usTagCol;           // restore column position
      UPDATECOLPOS( *pszTag, usColPos );

      /****************************************************************/
      /* Check for DBCS character and avoid restart of search for     */
      /* tags on second DBCS characters                               */
      /****************************************************************/
     // if ( (_dbcs_cp == DBCS_CP) && (isdbcs1(*pszTag) == DBCS_1ST) )
     // {
     //   // skip second DBCS character ...
     //   pszTag++;                        // skip DBCS 2nd character
     // } /* endif */
      pszTag++;                        // restart at next character
      usTagCol   = usColPos;
      pszData    = pszTag;             // set data pointer
      pNode      = pFirstNode;         // restart at begin of node tree
    } /* endif */
  } while( (ProcState == WORKING_STATE) && (chTest != EOS) );

  if ( ProcState == TAG_MATCH_STATE )
  {
    /******************************************************************/
    /* Avoid split of CRLF combination at end of tags                 */
    /******************************************************************/
    if ( (pToken->pDataStringW[pToken->usLength-1] == CR ) &&
         (*pszData == LF) )
    {
      pszData++;
      usColPos = 0;
      pToken->usLength++;
    } /* endif */

    /******************************************************************/
    /* Update caller's data pointer and column position               */
    /******************************************************************/
    *ppszData = pszData;
    pTokStatus->usColPos = usColPos;
    pTokStatus->usStartColPos = usTagCol;

  } /* endif */

  return( (ProcState == TAG_MATCH_STATE) );
} /* end of MatchTag */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TACreateTagTree
//------------------------------------------------------------------------------
// Function call:     TACreateTagTree( PTAGTABLE pTagTable, CHAR chMultSubst,
//                                     CHAR chSingleSubst, BOOL fMsg );
//------------------------------------------------------------------------------
// Description:       Create a tag tree required for TATagTokenize
//------------------------------------------------------------------------------
// Parameters:        PTAGTABLE  pTagTable     ptr to a loaded tag table
//                    CHAR       chMultSubst   muliple substitution character
//                    CHAR       chSingleSubst single substitution character
//                    BOOL       fMsg          show error messages flag
//------------------------------------------------------------------------------
// Returncode type:   PNODEAREA
//------------------------------------------------------------------------------
// Returncodes:       NULL        create of tag tree failed
//                    other       ptr to created tag tree
//------------------------------------------------------------------------------
PNODEAREA TACreateTagTree
(
  PLOADEDTABLE pLoadedTable,
  PTAGTABLE   pTagTable,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
)
{
   PTAG        pTag;                   // pointer to structure of active tag
   PSZ_W       pTagNamesW;              // pointer to start of tagnames
   PBYTE       pByte;                  // helper pointer
   ULONG       ulNoOfTags;             // number of tags to process
   BOOL        fOK = TRUE;             // internal OK flag
   USHORT      usI, usJ;               // general loop index
   PTBTAGENTRY pExtractedTags = NULL;  // ptr to tags extracted from tag table
   PNODEAREA   pNodeArea = NULL;       // area containing node tree
   USHORT      usEntry = 0;            // index into extracted tags array

   // address tag table lists
   pByte = (PBYTE) pTagTable;
   pTag = (PTAG) ( pByte + pTagTable->stFixTag.uOffset);
   //pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);
   pTagNamesW = pLoadedTable->pTagNamesW;
   ulNoOfTags = pTagTable->stFixTag.uNumber;
   for ( usI = 0; usI < 27; usI++ )
   {
      ulNoOfTags += pTagTable->stTagIndex[usI].uNumber;
   } /* endfor */
   ulNoOfTags += pTagTable->stVarStartTag.uNumber;

   // allocate tag entry array
   fOK = UtlAlloc( (PVOID *)&pExtractedTags, 0L,
                   (LONG) (ulNoOfTags * sizeof(TBTAGENTRY)),
                   ERROR_STORAGE );

   // extract tags from tag table
   if ( fOK )
   {
      for ( usI = 0; usI < pTagTable->stFixTag.uNumber; usI++ )
      {
         pExtractedTags[usEntry].pszTag    = pTagNamesW + pTag->uTagnameOffs;
         pExtractedTags[usEntry].pszEndDel = pTagNamesW + pTag->uEndDelimOffs;
         pExtractedTags[usEntry].usColPos  = pTag->usPosition;
         pExtractedTags[usEntry].usLength  = pTag->usLength;
         pExtractedTags[usEntry].fAttr     = pTag->fAttr;
         pExtractedTags[usEntry].fTranslInfo = pTag->BitFlags.fTranslate;
         pExtractedTags[usEntry].sID       = usEntry;
         usEntry++;
         pTag++;
      } /* endfor */

      for ( usJ = 0; usJ < 27; usJ++ )
      {
         pTag = (PTAG) ( pByte + pTagTable->stTagIndex[usJ].uOffset);
         for ( usI = 0; usI < pTagTable->stTagIndex[usJ].uNumber; usI++ )
         {
            pExtractedTags[usEntry].pszTag   = pTagNamesW + pTag->uTagnameOffs;
            pExtractedTags[usEntry].pszEndDel= pTagNamesW + pTag->uEndDelimOffs;
            pExtractedTags[usEntry].usColPos = pTag->usPosition;
            pExtractedTags[usEntry].usLength = pTag->usLength;
            pExtractedTags[usEntry].fAttr    = pTag->fAttr;
            pExtractedTags[usEntry].fTranslInfo = pTag->BitFlags.fTranslate;
            pExtractedTags[usEntry].sID      = usEntry;
            pTag++;
            usEntry++;
         } /* endfor */
      } /* endfor */

      pTag = (PTAG) ( pByte + pTagTable->stVarStartTag.uOffset);
      for ( usI = 0; usI < pTagTable->stVarStartTag.uNumber; usI++ )
      {
         pExtractedTags[usEntry].pszTag    = pTagNamesW + pTag->uTagnameOffs;
         pExtractedTags[usEntry].pszEndDel = pTagNamesW + pTag->uEndDelimOffs;
         pExtractedTags[usEntry].usColPos  = pTag->usPosition;
         pExtractedTags[usEntry].usLength  = pTag->usLength;
         pExtractedTags[usEntry].fAttr     = pTag->fAttr;
         pExtractedTags[usEntry].fTranslInfo = pTag->BitFlags.fTranslate;
         pExtractedTags[usEntry].sID       = usEntry;
         pTag++;
         usEntry++;
      } /* endfor */

      pNodeArea = TACreateNodeTree( pExtractedTags, ulNoOfTags,
                                     chMultSubst, chSingleSubst,
                                     fMsg );
   } /* endif */


   if ( pExtractedTags ) UtlAlloc( (PVOID *)&pExtractedTags, 0L, 0L, NOMSG );

   return ( pNodeArea );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TACreateAttrTree
//------------------------------------------------------------------------------
// Function call:     TACreateAttrTree( PTAGTABLE pTagTable, CHAR chMultSubst,
//                                     CHAR chSingleSubst, BOOL fMsg );
//------------------------------------------------------------------------------
// Description:       Create an attribute tree required for TATagTokenize
//------------------------------------------------------------------------------
// Parameters:        PTAGTABLE pTagTable      ptr to a loaded tag table
//                    CHAR       chMultSubst   muliple substitution character
//                    CHAR       chSingleSubst single substitution character
//                    BOOL       fMsg          show error messages flag
//------------------------------------------------------------------------------
// Returncode type:   PNODEAREA
//------------------------------------------------------------------------------
// Returncodes:       NULL        create of attribute tree failed
//                    other       ptr to created attribute tree
//------------------------------------------------------------------------------
PNODEAREA TACreateAttrTree
(
  PLOADEDTABLE pLoadedTable,
  PTAGTABLE   pTagTable,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
)
{
   PATTRIBUTE  pAttr;                  // ptr to structure of active attribute
   PSZ_W       pTagNamesW;              // ptr to start of tagnames
   PBYTE       pByte;                  // helper pointer
   ULONG       ulNoOfTags;             // number of tags to process
   BOOL        fOK = TRUE;             // internal OK flag
   USHORT      usI, usJ;               // general loop index
   USHORT      usEntry;                // current entry in pExtractedTags
   PTBTAGENTRY pExtractedTags = NULL;  // ptr to tags extracted from tag table
   SHORT       sID;                    // IDs of attributes
   ULONG       ulLength;               // length of attribute array in bytes
   PNODEAREA   pNodeArea = NULL;       // area containing node tree

   // address tag table lists
   pByte = (PBYTE) pTagTable;
//   pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);

   pTagNamesW = pLoadedTable->pTagNamesW;
   ulNoOfTags = pTagTable->stAttribute.uNumber;
   for ( usI = 0; usI < 27; usI++ )
   {
      ulNoOfTags += pTagTable->stAttributeIndex[usI].uNumber;
   } /* endfor */
   sID = pTagTable->uNumTags;

   // allocate attribute entry array
   ulLength = ulNoOfTags * sizeof(TBTAGENTRY);
   fOK = UtlAlloc( (PVOID *)&pExtractedTags, 0L,
                   (LONG) max( ulLength, MIN_ALLOC ),
                   ERROR_STORAGE);

   // extract fixed attributes from tag table
   usEntry = 0;                      // start with first entry in table
   sID = pTagTable->uNumTags;          // set ID of first attribute
   pAttr = (PATTRIBUTE) ( pByte + pTagTable->stAttribute.uOffset);
   if ( fOK )
   {
      for ( usI = 0; usI < pTagTable->stAttribute.uNumber; usI++ )
      {
         pExtractedTags[usEntry].pszTag = pAttr->uStringOffs + pTagNamesW;
         pExtractedTags[usEntry].sID    = sID++;
         pExtractedTags[usEntry].pszEndDel = pTagNamesW + pAttr->uEndDelimOffs;
         pExtractedTags[usEntry].usColPos  = 0;
         pExtractedTags[usEntry].usLength  = pAttr->usLength;
         pExtractedTags[usEntry].fAttr     = FALSE;
         pAttr++;
         usEntry++;
      } /* endfor */
   } /* endif */

   // extract variable attributes from tag table
   if ( fOK )
   {
      for ( usJ = 0; usJ < 27; usJ++ )
      {
         pAttr = (PATTRIBUTE) ( pByte + pTagTable->stAttributeIndex[usJ].uOffset);
         for ( usI = 0; usI < pTagTable->stAttributeIndex[usJ].uNumber; usI++ )
         {
            pExtractedTags[usEntry].pszTag = pAttr->uStringOffs + pTagNamesW;
            pExtractedTags[usEntry].sID    = sID++;
            pExtractedTags[usEntry].pszEndDel = pTagNamesW + pAttr->uEndDelimOffs;
            pExtractedTags[usEntry].usColPos  = 0;
            pExtractedTags[usEntry].usLength  = pAttr->usLength;
            pExtractedTags[usEntry].fAttr     = FALSE;
            pAttr++;
            usEntry++;
         } /* endfor */
      } /* endfor */
      pNodeArea = TACreateNodeTree( pExtractedTags, ulNoOfTags,
                                     chMultSubst, chSingleSubst,
                                     fMsg );
   } /* endif */


   if ( fOK )
   {
      UtlAlloc( (PVOID *)&pExtractedTags, 0L, 0L, NOMSG );
   } /* endif */

   return ( pNodeArea );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TACreateNodeTree
//------------------------------------------------------------------------------
// Function call:     TACreateNodeTree( PTBTAGENTRY pTags, USHORT ulNoOfTags,
//                                      CHAR chMultSubst, CHAR chSingleSubst,
//                                      BOOL fMsg );
//------------------------------------------------------------------------------
// Description:       Creates a node tree for a given list of tags or
//                    attributes.
//------------------------------------------------------------------------------
// Parameters:        PTBTAGENTRY pTags         ptr to list of tags
//                    USHORT      ulNoOfTags    number of tags in list
//                    CHAR        chMultSubst   muliple substitution character
//                    CHAR        chSingleSubst single substitution character
//                    BOOL        fMsg          do error message handling flag
//------------------------------------------------------------------------------
// Returncode type:   PNODEAREA
//------------------------------------------------------------------------------
// Returncodes:       NULL        create of node tree failed
//                    other       ptr to created node tree
//------------------------------------------------------------------------------
PNODEAREA TACreateNodeTree
(
  PTBTAGENTRY pTags,                   // ptr to list of tags
  ULONG       ulNoOfTags,              // number of tags in list
  CHAR_W        chMultSubst,             // muliple substitution character
  CHAR_W        chSingleSubst,           // single substitution character
  BOOL        fMsg                     // do error message handling flag
)
{
   BOOL        fOK = TRUE;             // internal OK flag
   CHAR_W      chCurChar;              // currently processed character
   PTREENODE   pNode;                  // currently processed node
   PTREENODE   pTempNode;              // temporary node pointer
   PTREENODE   pLastNode;              // last node processed
   PTREENODE   pPrevNode;              // previous node of current node
   PTREENODE   pNextNode = NULL;       // next node of current node
   PTREENODE   pFoundNode;             // found node
   PTREENODE   pFirstNodeOnLevel;      // first node on current level
   USHORT      usCurPos;               // currently tested character position
   USHORT      usCharPos;              // position of character to test
   PSZ_W         pszTag;               // pointer into current tag
   PNODEAREA   pNewArea = NULL;        // ptr to newly allocated node area
   PNODEAREA   pNodeArea = NULL;       // ptr to current node area
   PNODEAREA   pRootArea = NULL;       // ptr to root node area
   ULONG       ulAreaSize = 0;
   USHORT      usTag;                  // number (=ID) of current tag
   USHORT      usCurCharIndex;         // index of current character
   USHORT      usNodeCharIndex;        // index of current node character
   SHORT       sCompare;               // result of character comparism
   BOOL        fInsert;                // insert-a-new-node flag


   pNewArea = NULL;
   pNodeArea = NULL;
   usTag = 0;

  /********************************************************************/
  /* Allocate area for node tree (initial size = 3 times number of    */
  /* tags)                                                            */
  /********************************************************************/
  if ( fOK )
  {
    ulAreaSize = min( (LONG)(ulNoOfTags * 3 * sizeof(TREENODE)) +
                             sizeof(NODEAREA), MAX_ALLOC );
    fOK = UtlAlloc( (PVOID *)&pNodeArea, 0L, ulAreaSize,
                            (USHORT)(( fMsg ) ? ERROR_STORAGE : NOMSG) );
    if ( fOK )
    {
      pRootArea = pNodeArea;
      pNodeArea->usFreeNodes = ((USHORT)ulAreaSize - sizeof(NODEAREA)) /
                               sizeof(TREENODE);
      pNodeArea->pFreeNode   = &(pNodeArea->Nodes[0]);
      pRootArea->pRootNode   = NULL;
    }
    else
    {
      pRootArea = NULL;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Loop over all tags and add tags to node tree                     */
  /********************************************************************/
  while ( fOK && ulNoOfTags )
  {
    /******************************************************************/
    /* Add current tag to node tree                                   */
    /******************************************************************/
    pszTag     = pTags->pszTag;
    usCharPos  = 0;
    pNode      = pRootArea->pRootNode;
    pLastNode  = NULL;

    do
    {
      chCurChar = *pszTag++;
      usCurPos  = usCharPos++;

      if ( pNode == NULL )
      {
        /**************************************************************/
        /* No nodes on this level yet, create new node on this level  */
        /**************************************************************/

        /************************************************************/
        /* Get a free node                                          */
        /************************************************************/
        if ( pNodeArea->usFreeNodes )
        {
          pNode = pNodeArea->pFreeNode++;
          pNodeArea->usFreeNodes--;
        }
        else
        {
          pNewArea = NULL;
          fOK = UtlAlloc( (PVOID *)&pNewArea, 0L, ulAreaSize,
                          (USHORT)(( fMsg ) ? ERROR_STORAGE : NOMSG) );
          if ( fOK )
          {
            pNodeArea->pNext = pNewArea;
            pNodeArea = pNewArea;
            pNodeArea->usFreeNodes = ((USHORT)ulAreaSize - sizeof(NODEAREA)) /
                                     sizeof(TREENODE);
            pNodeArea->pFreeNode   = &pNodeArea->Nodes[0];
            pNode = pNodeArea->pFreeNode++;
            pNodeArea->usFreeNodes--;
          } /* endif */
        } /* endif */

        /************************************************************/
        /* fill-in node data                                        */
        /************************************************************/
        pNode->chNode     = chCurChar;
        pNode->chPos      = (CHAR_W)usCurPos;
        pNode->pNextLevel = NULL;
        pNode->pNextChar  = NULL;
        pNode->pszEndDel  = pTags->pszEndDel;
        pNode->usLength   = pTags->usLength;
        pNode->usColPos   = ( usCurPos == 0 ) ? pTags->usColPos : 0;
        pNode->fAttr      = pTags->fAttr;
        pNode->fTransInfo = pTags->fTranslInfo;
        pNode->sTokenID   = pTags->sID;
        pNode->fUnique    = TRUE;

        /************************************************************/
        /* Anchor node                                              */
        /************************************************************/
        if ( pRootArea->pRootNode == NULL )
        {
          pRootArea->pRootNode = pNode;
        } /* endif */

        if ( pLastNode )
        {
          pLastNode->pNextLevel = pNode;
        } /* endif */

        /************************************************************/
        /* Use current node as last node for next operation         */
        /************************************************************/
        pLastNode = pNode;
        pNode     = NULL;
      }
      else
      {
        /**************************************************************/
        /* Get compare table index for current character              */
        /**************************************************************/
        if ( chCurChar == chMultSubst )
        {
          usCurCharIndex = 0;
        }
        else if ( chCurChar == chSingleSubst )
        {
          usCurCharIndex = 1;
        }
        else if ( chCurChar == EOS )
        {
          usCurCharIndex = 2;
        }
        else if ( (pTags->usColPos != 0) && (usCurPos == 0) )
        {
          usCurCharIndex = 3;
        }
        else
        {
          usCurCharIndex = 4;
        } /* endif */

        /**************************************************************/
        /*                                                            */
        /**************************************************************/
        pFirstNodeOnLevel = pNode;
        pPrevNode  = NULL;             // no previous node yet
        pFoundNode = NULL;             // no matching node found yet
        fInsert    = FALSE;            // nothing to insert yet
        while ( !fInsert && !pFoundNode )
        {
          /************************************************************/
          /* Compare tag character with character of current node     */
          /* order of characters is:                                  */
          /* 1. alpha with column position                            */
          /* 2. alpha                                                 */
          /* 3. single substitution character                         */
          /* 4. multiple substitution character                       */
          /* 5. end of string delimiter (EOS)                         */
          /*                                                          */
          /* The following table is used to do the comparism:         */
          /*                                                          */
          /*               Node Character                             */
          /*                %     ?    EOS   Pos   other              */
          /*         ----Å-----Å-----Å-----Å-----Å-------+            */
          /*            %   >     >     <     >      >                */
          /*         ----Å-----Å-----Å-----Å-----Å-------+            */
          /* current    ?   <     >     <     >      >                */
          /*  char  -----Å-----Å-----Å-----Å-----Å-------+            */
          /*          EOS   >     >     >     >      >                */
          /*        -----Å-----Å-----Å-----Å-----Å-------+            */
          /*          Pos   <     <     <     =      <                */
          /*        -----Å-----Å-----Å-----Å-----Å-------+            */
          /*        Other   <     <     <     >      =                */
          /*        -----Á-----Á-----Á-----Á-----Á-------+            */
          /************************************************************/


          /**************************************************************/
          /* Get compare table index for node character                 */
          /**************************************************************/
          if ( pNode->chNode == chMultSubst )
          {
            usNodeCharIndex = 0;
          }
          else if ( pNode->chNode == chSingleSubst )
          {
            usNodeCharIndex = 1;
          }
          else if ( pNode->chNode == EOS )
          {
            usNodeCharIndex = 2;
          }
          else if ( pNode->usColPos != 0 )
          {
            usNodeCharIndex = 3;
          }
          else
          {
            usNodeCharIndex = 4;
          } /* endif */

          sCompare = asCharCompare[ usCurCharIndex ][ usNodeCharIndex ];
          if ( sCompare == 0 )
          {
            if ( chCurChar > pNode->chNode )
            {
              sCompare = 1;
            }
            else if ( chCurChar < pNode->chNode )
            {
              sCompare = -1;
            }
            else if ( (pTags->usColPos != pNode->usColPos) && (usCurPos == 0) )
            {
              sCompare = 1;
            } /* endif */
          } /* endif */

          if ( sCompare > 0 )
          {
            if ( pNode->pNextChar )
            {
              /********************************************************/
              /* Continue search with next node on this level         */
              /********************************************************/
              pPrevNode = pNode;
              pNode     = pNode->pNextChar;
            }
            else
            {
              /********************************************************/
              /* No nodes to follow on this level, add new node       */
              /* behind last node of this level                       */
              /********************************************************/
              pPrevNode = pNode;
              pNextNode = NULL;
              fInsert   = TRUE;
            } /* endif */
          }
          else if ( sCompare == 0 )
          {
            /**********************************************************/
            /* Follow the given path ...                              */
            /**********************************************************/
            pFoundNode = pNode;
            pLastNode  = pFoundNode;
            pNode      = pFoundNode->pNextLevel;
          }
          else
          {
            /**********************************************************/
            /* Insert new node right before current node              */
            /**********************************************************/
            pNextNode = pNode;
            fInsert   = TRUE;
          } /* endif */
        } /* endwhile */

        /****************************************************************/
        /* Follow found path or add new node on this level              */
        /****************************************************************/
        if ( fInsert )
        {
          /**************************************************************/
          /* Get a free node                                            */
          /**************************************************************/
          if ( pNodeArea->usFreeNodes )
          {
            pNode = pNodeArea->pFreeNode++;
            pNodeArea->usFreeNodes--;
          }
          else
          {
            pNewArea = NULL;
            fOK = UtlAlloc( (PVOID *)&pNewArea, 0L, ulAreaSize,
                            (USHORT)(( fMsg ) ? ERROR_STORAGE : NOMSG) );
            if ( fOK )
            {
              pNodeArea->pNext = pNewArea;
              pNodeArea = pNewArea;
              pNodeArea->usFreeNodes = ((USHORT)ulAreaSize - sizeof(NODEAREA)) /
                                       sizeof(TREENODE);
              pNodeArea->pFreeNode   = &pNodeArea->Nodes[0];
              pNode = pNodeArea->pFreeNode++;
              pNodeArea->usFreeNodes--;
            } /* endif */
          } /* endif */

          /**************************************************************/
          /* fill-in node data                                          */
          /**************************************************************/
          pNode->chNode     = chCurChar;
          pNode->chPos      = (CHAR_W)usCurPos;
          pNode->pNextLevel = NULL;
          pNode->pNextChar  = pNextNode;
          pNode->pszEndDel  = pTags->pszEndDel;
          pNode->usLength   = pTags->usLength;
          pNode->usColPos   = ( usCurPos == 0 ) ? pTags->usColPos : 0;
          pNode->fAttr      = pTags->fAttr;
          pNode->fTransInfo = pTags->fTranslInfo;
          pNode->sTokenID   = pTags->sID;
          pNode->fUnique    = TRUE;

          /**************************************************************/
          /* Anchor node                                                */
          /**************************************************************/
          if ( pPrevNode )
          {
            pPrevNode->pNextChar = pNode;
          }
          else if ( pLastNode )
          {
            pLastNode->pNextLevel = pNode;
          }
          else
          {
            pRootArea->pRootNode = pNode;
          } /* endif */
          if ( pPrevNode == NULL )
          {
            pFirstNodeOnLevel = pNode;
          } /* endif */

          /************************************************************/
          /* Update unique flags of nodes on this level               */
          /*                                                          */
          /* Nodes are not unique if either there are nodes with      */
          /* multiple substitution, single substitution or EOS        */
          /* characters or if the same character is more than once in */
          /* the node list of this level                              */
          /************************************************************/
          pTempNode = pFirstNodeOnLevel;
          while ( pTempNode )
          {
            /**********************************************************/
            /* Check remaining nodes following the current node       */
            /**********************************************************/
            pNextNode = pTempNode->pNextChar;
            while ( pNextNode && pTempNode->fUnique )
            {
              if ( (pNextNode->chNode == EOS)           ||
                   (pNextNode->chNode == chSingleSubst) ||
                   (pNextNode->chNode == chMultSubst)   ||
                   (pNextNode->chNode == pTempNode->chNode) )
              {
                pTempNode->fUnique = FALSE;
              }
              else
              {
                pNextNode = pNextNode->pNextChar;
              } /* endif */
            } /* endwhile */
            pTempNode = pTempNode->pNextChar;
          } /* endwhile */

          /**************************************************************/
          /* Use current node as last node for next operation           */
          /**************************************************************/
          pLastNode = pNode;
          pNode     = NULL;
        } /* endif */
      } /* endif */
    } while ( chCurChar ); /* enddo */

    /******************************************************************/
    /* Continue with next tag                                         */
    /******************************************************************/
    pTags++;
    ulNoOfTags--;
    usTag++;
  } /* endwhile */

  // free allocated memory in case of errors
  if ( !fOK && pRootArea )
  {
    do
    {
      pNodeArea = pRootArea;
      pRootArea = pNodeArea->pNext;
      UtlAlloc( (PVOID *)&pNodeArea, 0L, 0L, NOMSG );
    } while ( pRootArea ); /* enddo */
  } /* endif */

  return( pRootArea );
}

// Create SO/SI Protect table
USHORT TASoSiProtectTable
(
  PSZ_W              pszSegment,         // ptr to text of segment being processed
  PSTARTSTOP       *ppStartStop        // ptr to caller's start/stop table ptr
)
{
  USHORT           usRC = NO_ERROR;    // function return code
  PSTARTSTOP       pStartStop = *ppStartStop;  // ptr to new start/stop table
  PSTARTSTOP       pCurrent = NULL;    // ptr for start/stop table processing
  PSTARTSTOP       pAct, pActNew;      // ptr for start/stop table processing
  PSZ_W              pText = pszSegment; // ptr to segment
  USHORT           usCountSOSI = 0;
  CHAR_W           c;

  /********************************************************************/
  /* check if something todo                                          */
  /********************************************************************/
  while ( (c = *pText++ ) != NULC )
  {
    if ((c == SO ) || (c== SI) )
    {
      // a text string containing an SO/SI is split at that character into
      // two additional units, the SO/SI and the rest of the text string
      usCountSOSI += 2;
    } /* endif */
  } /* endwhile */



  if ( usCountSOSI )
  {
    pAct = pStartStop;
    while ( pAct->usStop )
    {
      pAct++;
      usCountSOSI++;
    } /* endwhile */
    usCountSOSI++;


    if ( !UtlAlloc( (PVOID *)&pCurrent, 0L,
                   (LONG) max( (usCountSOSI * sizeof(STARTSTOP)), MIN_ALLOC),
                   NOMSG ))
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */


    if ( !usRC )
    {
      pAct = pStartStop;
      pActNew = pCurrent;
      while ( pAct->usStop )
      {
        memcpy( pActNew, pAct, sizeof(STARTSTOP));
        if ( pAct->usType != PROTECTED_CHAR )
        {
          CHAR_W c, b;
          SHORT sEnd;
          USHORT usType;
          pText = pszSegment + pAct->usStart;

          b = pszSegment[pAct->usStop];
          pszSegment[pAct->usStop] = EOS;

          while( (c = *pText++) != NULC)
          {
            switch (c)
            {
              case SO:
              case SI:
                sEnd  = pActNew->usStop = (USHORT)(pText - pszSegment- 2);
                usType = pActNew->usType;
                // insert new element for SO/SI as PROTECTED_CHAR
                if ( sEnd > pActNew->usStart )
        {
                  pActNew++;
        }
                sEnd++;
                pActNew->usStart = pActNew->usStop = sEnd;
                pActNew->usType  = PROTECTED_CHAR;
                // insert start of normal text again with old status
        sEnd++;
        if ( sEnd < pAct->usStop )
        {
                  pActNew++;
                  pActNew->usStart = sEnd; //+1;
                  pActNew->usStop  = pAct->usStop;
                  pActNew->usType  = usType;
        }
                break;
            }
          }

          pszSegment[pAct->usStop] = b;
        }
        pActNew++; pAct++;
      } /* endwhile */
      // Add end element ..
      memcpy( pActNew, pAct, sizeof(STARTSTOP));

      UtlAlloc( (PVOID *) ppStartStop, 0L, 0L, NOMSG );
      *ppStartStop = pCurrent;
    } /* endif */
  } /* endif */

  return (usRC);
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TACreateProtectTable
//------------------------------------------------------------------------------
// Function call:     TACreateProtectTable
//------------------------------------------------------------------------------
// Description:       Creates a table with start/stop info for protected/
//                    unprotected data in input string.
//------------------------------------------------------------------------------
// Parameters:        PSZ          pszSegmen  ptr to text of segment
//                    PVOID        pVoidTabl  ptr to tag table
//                    USHORT       usColPos   colpos of first char in segment
//                    PTOKENENTRY  pTokBuffe  buffer used temporarly for tokens
//                    USHORT       usTokBuff  size of token buffer in bytes
//                    PSTARTSTOP   *ppStartS  ptr to caller's start/stop table
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR              if function completed successfully
// Returncodes:       ERROR_INVALID_DATA    if one of the input parameter is in
//                                          error
//                    ERROR_NOT_ENOUGH_MEMORY if memory allocation failed
//------------------------------------------------------------------------------
USHORT TACreateProtectTableW
(
  PSZ_W            pszSegment,         // ptr to text of segment being processed
  PVOID            pVoidTable,         // ptr to tag table
  USHORT           usColPos,           // column position of first char in segment
  PTOKENENTRY      pTokBuffer,         // buffer used temporarly for tokens
  USHORT           usTokBufferSize,    // size of token buffer in bytes
  PSTARTSTOP       *ppStartStop,       // ptr to caller's start/stop table ptr
  PFN              pvUserExit,          // ptr to user exit function
  PFN              pvUserExitW,
  ULONG            ulCP                 // ASCII cp fitting to Segmenttext
)
{
  return( TACreateProtectTableWEx( pszSegment, pVoidTable, usColPos, pTokBuffer, usTokBufferSize,
          ppStartStop, pvUserExit, pvUserExitW, ulCP, 0 ) );
}

USHORT TACreateProtectTableWEx
(
  PSZ_W            pszSegment,         // ptr to text of segment being processed
  PVOID            pVoidTable,         // ptr to tag table
  USHORT           usColPos,           // column position of first char in segment
  PTOKENENTRY      pTokBuffer,         // buffer used temporarly for tokens
  USHORT           usTokBufferSize,    // size of token buffer in bytes
  PSTARTSTOP       *ppStartStop,       // ptr to caller's start/stop table ptr
  PFN              pvUserExit,          // ptr to user exit function
  PFN              pvUserExitW,
  ULONG            ulCP,                // ASCII cp fitting to Segmenttext
  int              iMode                // mode for function
)
{
  USHORT           usRC = NO_ERROR;    // function return code
  PSTARTSTOP       pStartStop = NULL;  // ptr to new start/stop table
  PSTARTSTOP       pCurrent;           // ptr for start/stop table processing
  PSTARTSTOP       pAct;               // ptr for start/stop table processing
  BOOL             fTag;               // we are in tag processing mode
  ULONG            ulTableUsed = 0;
  ULONG            ulTableAlloc = 0;
  PLOADEDTABLE     pTagTable;          // ptr to loaded tag table
  PTOKENENTRY      pTok;               // ptr for token list processing
  PSZ_W            pRest;              // ptr to not tokenized data
  PTAG             pTag = NULL;        // pointer to tags in tagtable
  PATTRIBUTE       pAttribute = NULL;  // pointer to attributes in tagtable
  PBYTE            pByte;              // help pointer for tag table addressing
  SHORT            sNumTags = 0;       // number of tags in tag table
  PSZ_W            pszTemp;            // temporary text pointer
  USHORT           usI;                // loop counter
  CHAR_W           chQuote = NULC;     // buffer for starting quote
  PFNSTARTSTOPEXIT pfnUserExit;        // type corrected user exit pointer
  PFNSTARTSTOPEXITW pfnUserExitW;
  BOOL             fTRNoteFound = 0;   // true if TRNOTE found in curr segment
  BOOL             fTRTag= FALSE;      // true if inside of a trnote
  CHAR             chAsciiSeg[MAX_SEGMENT_SIZE];
  PTOKENENTRY      pAttrTokBuffer = NULL;  // used temp.for translatable attributetokens
  LONG             lAttrSize = 0L;
  PTOKENENTRY      pNextTok;             // ptr for next token in token list

  /********************************************************************/
  /* Check input parameters                                           */
  /********************************************************************/
  pTagTable = (PLOADEDTABLE) pVoidTable;

  if ( (pszSegment == NULL)    ||
       (pTagTable  == NULL)    ||
       (pTokBuffer == NULL)    ||
       (usTokBufferSize == 0)  ||
       (ppStartStop == NULL) )
  {
    usRC = ERROR_INVALID_DATA;
  } /* endif */

  /********************************************************************/
  /* Use user exit for start/stop table if there is one               */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    if (pvUserExitW )
    {
      pfnUserExitW = (PFNSTARTSTOPEXITW) pvUserExitW;
      if ( !pfnUserExitW( pszSegment, &pStartStop ) )
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    }
    else  if ( pvUserExit )
    {
      /****************************************************************/
      /* Let user exit create the start/stop table                    */
      /****************************************************************/
      pfnUserExit = (PFNSTARTSTOPEXIT)pvUserExit;
      Unicode2ASCII( pszSegment, &chAsciiSeg[0], ulCP);
      if ( !pfnUserExit( &chAsciiSeg[0], &pStartStop ) )
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    }
    else
    {
      /******************************************************************/
      /* Create start/stop table using tokenizer                        */
      /******************************************************************/

      /********************************************************************/
      /* Set tag table variables                                          */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        sNumTags   = (SHORT)pTagTable->pTagTable->uNumTags;
        pByte      = (PBYTE)pTagTable->pTagTable;
        pTag       = OFFSETTOPOINTER(PTAG, pTagTable->pTagTable->stFixTag.uOffset );
        pAttribute = OFFSETTOPOINTER(PATTRIBUTE,
                                     pTagTable->pTagTable->stAttribute.uOffset );
      } /* endif */

      /********************************************************************/
      /* Tokenize input segment                                           */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        pTagTable = (PLOADEDTABLE)pVoidTable;
        pRest     = NULL;
        TATagTokenizeW( pszSegment,
                       pTagTable,
                       TRUE,
                       &pRest,
                       &usColPos,
                       pTokBuffer,
                       (USHORT)(usTokBufferSize / sizeof(TOKENENTRY)) );
        usRC = ( pRest == NULL ) ? NO_ERROR : EQFRS_AREA_TOO_SMALL;
      } /* endif */

      /********************************************************************/
      /* Count number of tokens in list to get start/stop list size       */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        pTok        = pTokBuffer;

        while ( pTok->sTokenid != ENDOFLIST )
        {
          /****************************************************************/
          /* Check for attributes and tags with translatable info         */
          /****************************************************************/
          if ( ( (pTok->sTokenid >= sNumTags) &&
                 (pAttribute[pTok->sTokenid - sNumTags].BitFlags.fTranslate) ) ||
               ( (pTok->sTokenid >= 0)       &&
                 (pTok->sTokenid < sNumTags) &&
                 (pTag[pTok->sTokenid].BitFlags.fTranslate) ) )
          {
            /************************************************************/
            /* for any quote in the token text leave room for one more  */
            /* start/stop entry                                         */
            /************************************************************/
            pszTemp = pTok->pDataStringW;
            for ( usI = 0; usI < pTok->usLength; usI++, pszTemp++ )
            {
              if ( (*pszTemp == QUOTE) || (*pszTemp == DBLQUOTE) )
              {
                ulTableAlloc++;           // leave room for one more entry
				// P016238: leave room for 4 more entries assuming 2 inline tags
				// per each translatable part of an attribute
                ulTableAlloc = ulTableAlloc + 4;
              } /* endif */
            } /* endfor */
          } /* endif */
          ulTableAlloc++;                   // leave room for token
          pTok++;
        } /* endwhile */
        ulTableAlloc++;                     // leave room for table end delimiter
      } /* endif */

      // assure that ulTableAlloc * sizeof(STARTSTOP) is at least MIN_ALLOC
      if (ulTableAlloc < 4 )
      {
		  ulTableAlloc = 4;
      }

      /********************************************************************/
      /* Allocate start/stop list                                         */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        if ( !UtlAlloc( (PVOID *)&pStartStop, 0L,
                       (LONG) (ulTableAlloc * sizeof(STARTSTOP)),
                       NOMSG ))
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
        ulTableUsed = 0;
      } /* endif */

      TATagAddWSpace(pszSegment, pTokBuffer, sNumTags);

      /********************************************************************/
      /* Fill start/stop table                                            */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        pCurrent = pStartStop;
        pTok     = pTokBuffer;
        while ( (pTok->sTokenid != ENDOFLIST) && (usRC == NO_ERROR))
        {
          /****************************************************************/
          /* Check for attributes or tags with translatable info          */
          /****************************************************************/
          if ( ( (pTok->sTokenid >= sNumTags) &&
                 (pAttribute[pTok->sTokenid - sNumTags].BitFlags.fTranslate) ) ||
               ( (pTok->sTokenid >= 0)       &&
                 (pTok->sTokenid < sNumTags) &&
                 (pTag[pTok->sTokenid].BitFlags.fTranslate) ) )
          {
            /**************************************************************/
            /* Process data up to first quote or up to end of token       */
            /* Complete protected start/stop entry in front of          */
            /* quoted string                                            */
            /**************************************************************/
            pCurrent->usStart = (USHORT)(pTok->pDataStringW - pszSegment);
            pCurrent->usStop = TAFindQuote( pTok, (USHORT) (pCurrent->usStart-1), &chQuote);
            pCurrent->usType  =
                (pTok->sTokenid >= sNumTags) ?  PROTECTED_CHAR : TAGPROT_CHAR;

            usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
                                               &ulTableUsed, &ulTableAlloc);

            /**************************************************************/
            /* Process quoted strings if any                              */
            /**************************************************************/
            while ( pTok->usLength && (usRC == NO_ERROR) )
			{ // P016238: check whether quoted text contains protected parts!
			  if (!pAttrTokBuffer)
			  {
				lAttrSize = MAX_ATTR_TOKENS * sizeof(TOKENENTRY);
				if (!UtlAlloc((PVOID *) &pAttrTokBuffer, 0L, lAttrSize, NOMSG) )
				{
				   usRC = ERROR_NOT_ENOUGH_MEMORY;
				} /* endif */
			  } /* endif */
			  if ( usRC == NO_ERROR)
			  {
				usRC = TAProtectedPartsInQuotedText( pTok,
										  pTok->pDataStringW - pszSegment,
										  &pStartStop,
										  &pCurrent,
										  pAttrTokBuffer,
										  pTagTable,
										  &ulTableAlloc,
										  &ulTableUsed,
										  &usColPos,
										  chQuote,
										  lAttrSize / sizeof(TOKENENTRY),
                      ((iMode & CREATEPROTTABLE_MARKATTR) != 0) );
			  }

			  /************************************************************/
			  /* Add entry for protected data up to next quote or up to   */
			  /* end of token                                             */
			  /************************************************************/
			  if ( pTok->usLength && (usRC == NO_ERROR))
			  {
				pCurrent->usStart = (USHORT)(pTok->pDataStringW - pszSegment);
				pCurrent->usType  = PROTECTED_CHAR;

				pTok->pDataStringW++;          // assure quote is in pCurrent
				pTok->usLength--;
				pCurrent->usStop = TAFindQuote( pTok, (USHORT) (pCurrent->usStart), &chQuote);

				usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
				                                 &ulTableUsed, &ulTableAlloc);

			  } /* endif */
            } /* endwhile */
          }
          else
          {
            pCurrent->usStart = (USHORT)(pTok->pDataStringW - pszSegment);

            if ( pTok->sTokenid == TEXT_TOKEN  )
            {
              pCurrent->usType = UNPROTECTED_CHAR;
            }
            else if (pTok->sTokenid >= sNumTags )
            {
              // do not include translatable variables in protected text when working in CREATEPROTECTTABLE_NOTRANSLVAR mode
              if ( (pTok->ClassId == CLS_TRANSLVAR) && (iMode & CREATEPROTTABLE_NOTRANSLVAR) )
              {
                pCurrent->usType = UNPROTECTED_CHAR;
              }
              else
              {
                pCurrent->usType = PROTECTED_CHAR;
              } /* endif */                 
            }
            else
            {
              pCurrent->usType = TAGPROT_CHAR;
              if (pTag[pTok->sTokenid].BitFlags.fTRNote  )
              {
                if (fTRNoteFound )
                {
                  /****************************************************/
                  /* this approach requires that a TRNote is within   */
                  /* one segment; also nesting of notes is not allowed*/
                  /****************************************************/
                  pCurrent->usType = TRNOTE_CHAR;
                }
                else
                {
                  if (memicmp((PBYTE)&pTagTable->chTrnote1TextW[0],
                              (PBYTE)(pTok->pDataStringW+pTok->usLength),
                              pTagTable->ulTRNote1Len) == 0 )
                  {
                    pCurrent->usType = TRNOTE_CHAR;
                    fTRNoteFound = TRUE;
                  }
                  else if (memicmp((PBYTE)&pTagTable->chTrnote2TextW[0],
                                   (PBYTE)(pTok->pDataStringW+pTok->usLength),
                                   pTagTable->ulTRNote2Len) == 0 )
                  {
                    pCurrent->usType = TRNOTE_CHAR;
                    fTRNoteFound = TRUE;
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */
            pCurrent->usStop  = pCurrent->usStart + pTok->usLength - 1;
            usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
		                                     &ulTableUsed, &ulTableAlloc);

          } /* endif */

          pTok++;
        } /* endwhile */

       if (usRC == NO_ERROR)
       {
         TASetStartStopType(pCurrent, 0,0,0 );     //terminate start/stoptabl.

	     usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
	                                      &ulTableUsed, &ulTableAlloc);

       }
      } /* endif */


      /********************************************************************/
      /* now loop true list again and join tags with their attributes     */
      /* this is necessary to avoid splitting of tags and attributes      */
      /* during the insert operation in the editor...                     */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        pAct = pCurrent = pStartStop;
        pTok = pTokBuffer;
        pNextTok = pTok + 1;
        if ( (pAct->usType == TAGPROT_CHAR) && TA_IS_TAG(pTok->sTokenid, sNumTags))
        {
          pAct->usType = PROTECTED_CHAR;
          fTag = TRUE;
        }
        else
        {
          fTag = FALSE;
        } /* endif */
        if ( pCurrent->usType )
        {
          pCurrent++;
          if (pNextTok && pNextTok->pDataStringW &&
               (pCurrent->usStart >=(USHORT)(pNextTok->pDataStringW - pszSegment)))
          {
			  pTok ++;
			  pNextTok++;
	      }

          while ( pCurrent->usType )
          {
            switch ( pCurrent->usType )
            {
              case  TAGPROT_CHAR:
                if ( (pCurrent->usStart == (USHORT)(pTok->pDataStringW-pszSegment))
                      && TA_IS_TAG(pTok->sTokenid, sNumTags))
				{
				    fTag = TRUE;
                }
                pAct++;
                pAct->usStart = pCurrent->usStart;
                pAct->usStop  = pCurrent->usStop;
                pAct->usType = PROTECTED_CHAR;
                break;
              case  PROTECTED_CHAR:
                if ( fTag  && (pCurrent->usType == pAct->usType))
                {
                  /************************************************************/
                  /* combine tag and attribute...                             */
                  /************************************************************/
                  pAct->usStop = pCurrent->usStop;
                }
                else
                {
                  pAct++;
                  pAct->usStart = pCurrent->usStart;
                  pAct->usStop  = pCurrent->usStop;
                  pAct->usType  = pCurrent->usType;
                } /* endif */
                break;
              default :
                pAct++;
                pAct->usStart = pCurrent->usStart;
                pAct->usStop  = pCurrent->usStop;
                pAct->usType  = pCurrent->usType;
                if ( ((pCurrent->usStart == (USHORT)(pTok->pDataStringW-pszSegment))
				      && (pTok->sTokenid == TEXT_TOKEN)) ||
				      (pCurrent->usType == TRNOTE_CHAR))
				{
				    fTag = FALSE;
                }
                // P016804: if text-token is from quoted text, do not reset!!
                //fTag = FALSE;
                break;
            } /* endswitch */
            /******************************************************************/
            /* point to next one...                                           */
            /******************************************************************/
            pCurrent++;
            if (pNextTok && pNextTok->pDataStringW &&
                (pCurrent->usStart >=(USHORT)(pNextTok->pDataStringW - pszSegment)))
			{
			  pTok ++;
			  pNextTok++;
	        }
          } /* endwhile */
          /********************************************************************/
          /* add stopping mode....                                            */
          /********************************************************************/
          pAct++;
          pAct->usStart = 0;
          pAct->usStop  = 0;
          pAct->usType  = 0;
        } /* endif */

        /********************************************************************/
        /* now loop thru list and combine the tokens of a translators note  */
        /* into one start-stop token                                        */
        /********************************************************************/
        if (fTRNoteFound )
        {
          pAct = pCurrent = pStartStop;
          if ( pAct->usType == TRNOTE_CHAR )
          {
            fTRTag = TRUE;
          }
          else
          {
            fTRTag = FALSE;
          } /* endif */
          if ( pCurrent->usType )
          {
            pCurrent++;

            while ( pCurrent->usType )
            {
              switch ( pCurrent->usType )
              {
                case TRNOTE_CHAR :      // end of TRNOTE of begin of next TRNOTE
                  if (fTRTag )
                  {
                    pAct->usStop = pCurrent->usStop;
                  }
                  else
                  {
                    pAct++;
                    pAct->usStart = pCurrent->usStart;
                    pAct->usStop  = pCurrent->usStop;
                    pAct->usType  = pCurrent->usType;
                  } /* endif */
                  fTRTag = !fTRTag;
                  break;
                default:
                  if ( fTRTag )
                  {
                    /************************************************************/
                    /* combine tag and attribute...                             */
                    /************************************************************/
                    pAct->usStop = pCurrent->usStop;
                  }
                  else
                  {
                    pAct++;
                    pAct->usStart = pCurrent->usStart;
                    pAct->usStop  = pCurrent->usStop;
                    pAct->usType  = pCurrent->usType;
                  } /* endif */
                  break;
              } /* endswitch */
              /******************************************************************/
              /* point to next one...                                           */
              /******************************************************************/
              pCurrent++;
            } /* endwhile */
            /********************************************************************/
            /* add stopping mode....                                            */
            /********************************************************************/
            pAct++;
            pAct->usStart = 0;
            pAct->usStop  = 0;
            pAct->usType  = 0;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  if ( pAttrTokBuffer )
  {
     UtlAlloc( (PVOID *)&pAttrTokBuffer, 0L, 0L, NOMSG );
  } /* endif */

  /********************************************************************/
  /* Return start/stop list to caller or cleanup                      */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    *ppStartStop = pStartStop;
    if (IsDBCS_CP(ulCP))
    {
      usRC =  TASoSiProtectTable( pszSegment, ppStartStop);
    }
  }
  else
  {
    if ( pStartStop )
    {
      UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
    } /* endif */
    *ppStartStop = NULL;
  } /* endif */

  return( usRC );
}

VOID TASetStartStopType
(
  PSTARTSTOP       pCurrent,        // ptr to caller's current start/stop entry
  USHORT           usStart,
  USHORT           usStop,
  USHORT           usType
)
{
  pCurrent->usStart = usStart;
  pCurrent->usStop = usStop;
  pCurrent->usType = usType;
  return;
}

USHORT TAFindQuote
(
    PTOKENENTRY pTok,
    USHORT      usStartSearch,
    PCHAR_W     pchQuote
)
{
  USHORT  usStop = usStartSearch;
  while ( pTok->usLength &&
          (pTok->pDataStringW[0] != QUOTE) &&
          (pTok->pDataStringW[0] != DBLQUOTE) )
  {
    pTok->pDataStringW++;
    pTok->usLength--;
    usStop++;
  } /* endwhile */

  if ( pTok->usLength )
  {
    *pchQuote = pTok->pDataStringW[0];
    pTok->pDataStringW++;
    pTok->usLength--;
    usStop++;
  } /* endif */
  return(usStop);
}

USHORT TAGotoNextStartStopEntry
(
	PSTARTSTOP  * ppStartStop,
	PSTARTSTOP  * ppCurrent,
	PULONG    pulTableUsed,
	PULONG    pulTableAlloc
)
{
	PSTARTSTOP pCurrent    = *ppCurrent;
	ULONG      ulTableUsed = * pulTableUsed;
	ULONG      ulTableAlloc = *pulTableAlloc;
	USHORT     usRC        = NO_ERROR;    // function return code

    pCurrent ++;
    ulTableUsed ++;
    // for tests only is pCurrent after add as above?
    { PSTARTSTOP pStartStop = *ppStartStop;
       pCurrent = pStartStop + ulTableUsed;
    }
	if ( ulTableAlloc <= ulTableUsed + 1)
	{
    	 LONG  lOldLen;                       // old length
         LONG  lNewLen;                       // new length
    	PSTARTSTOP pNewStartStop = *ppStartStop;

    	lOldLen = ulTableAlloc * sizeof(STARTSTOP);
    	ulTableAlloc = ulTableAlloc + 20;
    	lNewLen = ulTableAlloc * sizeof(STARTSTOP);

		// realloc nec
        if ( !UtlAlloc( (PVOID *)&pNewStartStop, lOldLen,
                                  lNewLen, NOMSG ))
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
        else
        {
			// if realloc ok, set pointers
			pCurrent = pNewStartStop + ulTableUsed;
            *ppStartStop = pNewStartStop;
            *pulTableAlloc = ulTableAlloc;
	    }
    }
    if ( usRC == NO_ERROR)
    {
      // set return values
      *ppCurrent = pCurrent;
      *pulTableUsed = ulTableUsed;
    }
	return ( usRC );
}


USHORT  TAProtectedPartsInQuotedText
(
  PTOKENENTRY   pTok,
  int           iAttrStartOffset,
  PSTARTSTOP    * ppStartStop,
  PSTARTSTOP    * ppCurrent,
  PTOKENENTRY     pAttrTokBuffer,
  PLOADEDTABLE    pTagTable,
  PULONG          pulTableAlloc,
  PULONG          pulTableUsed,
  PUSHORT         pusColPos,
  CHAR_W          chQuote,
  LONG            lAttrSize,
  BOOL            fSpecialMode
)
{
    USHORT           usRC = NO_ERROR;
    CHAR_W           chTempBuf[MAX_SEGMENT_SIZE];
    PTOKENENTRY      pAttrTok;
    PSZ_W            pAttrStart;
    USHORT           usAttrLength = 0;
    PSZ_W            pRest;              // ptr to not tokenized data
    PSZ_W            pTemp;
    PSTARTSTOP       pCurrent = *ppCurrent;
    PSTARTSTOP       pStartStop = *ppStartStop;


      memset (&chTempBuf[0], 0, sizeof(CHAR_W) * (MAX_SEGMENT_SIZE));

      UTF16strcat(chTempBuf, pTok->pDataStringW);
      pAttrStart = &chTempBuf[0];
      pTemp = &chTempBuf[0];
      while ( pTemp && (*pTemp != chQuote) )
      {
        pTemp++;
        usAttrLength ++;
      } /* endwhile */

      if (*pTemp == chQuote )
      {
          *pTemp = EOS;    //investigate only text inside quotes!
      }

      if ( usRC == NO_ERROR )
      {
        TATagTokenizeW( chTempBuf,
                        pTagTable,
                        TRUE,
                        &pRest,
                        pusColPos,
                        pAttrTokBuffer,
                        (USHORT)lAttrSize );
        usRC = ( pRest == NULL ) ? NO_ERROR : EQFRS_AREA_TOO_SMALL;
      } /* endif */

      if ( usRC == NO_ERROR )
      {
          pAttrTok        = pAttrTokBuffer;
          while ( (pAttrTok->sTokenid != ENDOFLIST) && (usRC == NO_ERROR))
          {
            pCurrent->usStart = (USHORT)(pAttrTok->pDataStringW - pAttrStart
                                + iAttrStartOffset);
            pCurrent->usType = (pAttrTok->sTokenid == TEXT_TOKEN) ?
                                UNPROTECTED_CHAR : PROTECTED_CHAR;
            if ( fSpecialMode )
            {
              pCurrent->usType |= 0x8000;
            } /* endif */
            pCurrent->usStop = pCurrent->usStart + pAttrTok->usLength - 1;

            usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
	                                      pulTableUsed, pulTableAlloc);

            pAttrTok++;
          } /* endwhile */
      } /* endif */

      /**********************************************************/
      /* Add data up to first quote                             */
      /**********************************************************/
      while ( pTok->usLength && (pTok->pDataStringW[0] != chQuote) )
      {
        pTok->pDataStringW++;
        pTok->usLength--;
        usAttrLength --;
      } /* endwhile */
      // CHECK: does this loop run as far as pTemp in chTempBuf?
      if (usAttrLength != 0)
      {
          // this should not happen!!
          usAttrLength = 0;
      }

      *ppCurrent = pCurrent;
      *ppStartStop = pStartStop;
    return (usRC);
}



USHORT TACreateProtectTable
(
  PSZ              pszSegment,        // ptr to text of segment being processed
  PVOID            pVoidTable,         // ptr to tag table
  USHORT           usColPos,           // column position of first char in segment
  PTOKENENTRY      pTokBuffer,         // buffer used temporarly for tokens
  USHORT           usTokBufferSize,    // size of token buffer in bytes
  PSTARTSTOP       *ppStartStop,       // ptr to caller's start/stop table ptr
  PFN              pvUserExit,          // ptr to user exit function
  ULONG            ulCP
)
{
  USHORT usRC = 0;
     PSZ_W        pszInputW = &chInputW[0];

   ASCII2Unicode( pszSegment, pszInputW, ulCP );

   usRC = TACreateProtectTableW( pszInputW, pVoidTable, usColPos, pTokBuffer, usTokBufferSize,
                                ppStartStop, pvUserExit, NULL, ulCP );

   // attention:
   // for DBCS code pages the returned offsets and lengths do not match the actual
   // position in the source data and have to be re-adjusted

  return usRC;
}

USHORT TAPrepProtectTable
(
  PVOID            pVoidTable,         // ptr to tag table (PLOADEDTABLE)
  HMODULE          *phModule,          // address of user exit module handle
  PFN              *ppfnUserExit,      // address of ptr to user exit function
  PFN              *ppfnCheckSegExit,  // address of ptr to segment check func
  PFN              *ppfnWYSIWYGExit,   // address of ptr to show transl.  func
  PFN              *ppfnTocGotoExit    // address of ptr to TOC  func
)
{
  return( TALoadEditUserExit( pVoidTable, "", phModule, ppfnUserExit, ppfnCheckSegExit,
                              ppfnWYSIWYGExit, ppfnTocGotoExit, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL ) );

} /* end of function TAPrepProtectTable */

USHORT TALoadEditUserExit
(
  PVOID            pVoidTable,         // ptr to tag table (PLOADEDTABLE)
  PSZ              pszTableName,       // name of tag table (w/o path and ext.)
  HMODULE          *phModule,          // address of user exit module handle
  PFN              *ppfnUserExit,      // address of ptr to user exit function
  PFN              *ppfnCheckSegExit,  // address of ptr to segment check func
  PFN              *ppfnWYSIWYGExit,   // address of ptr to show transl.  func
  PFN              *ppfnTocGotoExit,   // address of ptr to TOC  func
  PFN              *ppfnGetSegContext, // address of ptr to EQFGETSEGCONTEXT function
  PFN              *ppfnUpdateContext, // address of ptr to EQFUPDATECONTEXT function
  PFN              *ppfnFormatContext, // address of ptr to EQFFORMATCONTEXT function
  PFN              *ppfnCompareContext, // address of ptr to EQFCOMPARECONTEXT function
  PFN              *ppfnUserExitW,     // unicode user exit to create start-stop-tbl
  PFN              *ppfnCheckSegExitW,  // unicode user exit to check segment
  PFN              *ppfnCheckSegExExitW,// unicode user exit to check segment Ex
  PFN              *ppfnCheckSegType   // user exit to check segment type
)
{
  PTAGTABLE        pTagTable;          // pointer to tag table
  USHORT           usRC = NO_ERROR;    // function return code

  /********************************************************************/
  /* initialize user's data fields                                    */
  /********************************************************************/
  *phModule = NULLHANDLE;
  if ( ppfnUserExit ) *ppfnUserExit = NULL;
  if ( ppfnCheckSegExit ) *ppfnCheckSegExit = NULL;
  if ( ppfnWYSIWYGExit ) *ppfnWYSIWYGExit = NULL;
  if ( ppfnTocGotoExit ) *ppfnTocGotoExit = NULL;
  if ( ppfnGetSegContext ) *ppfnGetSegContext  = NULL;
  if ( ppfnUpdateContext ) *ppfnUpdateContext  = NULL;
  if ( ppfnFormatContext ) *ppfnFormatContext  = NULL;
  if ( ppfnCompareContext ) *ppfnCompareContext  = NULL;
  if ( ppfnUserExitW ) *ppfnUserExitW = NULL;
  if ( ppfnCheckSegExitW ) *ppfnCheckSegExitW = NULL;
  if ( ppfnCheckSegExExitW ) *ppfnCheckSegExExitW = NULL;
  if ( ppfnCheckSegType ) *ppfnCheckSegType = NULL;


  /********************************************************************/
  /* Address tag table                                                */
  /********************************************************************/
  pTagTable = ((PLOADEDTABLE)pVoidTable)->pTagTable;

  /********************************************************************/
  /* Load user exit if tag table has one                              */
  /********************************************************************/
  if ( pTagTable->szSegmentExit[0] != EOS )
  {
    CHAR  szExit[MAX_LONGFILESPEC];        // buffer for library name
    CHAR  *ptr ;

//  strcpy( szExit, pTagTable->szSegmentExit );
//  strcat( szExit, EXT_OF_DLL );
//  if ( MUGetUserExitFileName( pTagTable->szSegmentExit, NULL, szExit, sizeof(szExit) ) )
    if ( MUGetUserExitFileName( pszTableName, NULL, szExit, sizeof(szExit) ) )
    {
      ptr = strrchr( szExit, '\\' ) ;
      if ( ptr ) 
         strcpy( ++ptr, pTagTable->szSegmentExit ) ;

      usRC = DosLoadModule( NULL, 0, szExit, phModule );
     
      if ( usRC == NO_ERROR )
      {
        if ( ppfnUserExit )
        {
     
          usRC = DosGetProcAddr( *phModule, EQFPROTTABLE_EXIT, (PFN*)ppfnUserExit);
          if ( usRC != NO_ERROR )
          {
            *ppfnUserExit = NULL;
          } /* endif */
        }
        /****************************************************************/
        /* try to load SegmentCheckExit function                        */
        /****************************************************************/
        if ( ppfnCheckSegExit )
        {
          usRC = DosGetProcAddr( *phModule, EQFCHECKSEG_EXIT,
                                  (PFN*)ppfnCheckSegExit);
          if ( usRC != NO_ERROR )
          {
            *ppfnCheckSegExit = NULL;
          } /* endif */
        }
        /****************************************************************/
        /* try to load ShowTransl  Exit function                        */
        /****************************************************************/
        if ( ppfnWYSIWYGExit )
        {
          usRC = DosGetProcAddr( *phModule, EQFSHOWTRANS_EXIT,
                                (PFN*)ppfnWYSIWYGExit);
          if ( usRC != NO_ERROR )
          {
            *ppfnWYSIWYGExit = NULL;
          } /* endif */
        }
        /****************************************************************/
        /* try to load CheckSegType  Exit function                        */
        /****************************************************************/
        if ( ppfnCheckSegType )
        {
          *ppfnCheckSegType = NULL;
          DosGetProcAddr( *phModule, EQFCHECKSEGTYPE_EXIT, (PFN*)ppfnCheckSegType);
        }
        /****************************************************************/
        /* try to load TOC user    Exit function                        */
        /****************************************************************/
        if ( ppfnTocGotoExit )
        {
          usRC = DosGetProcAddr( *phModule, EQFTOCGOTO_EXIT,
                                  (PFN*)ppfnTocGotoExit);
          if ( usRC != NO_ERROR )
          {
            *ppfnTocGotoExit = NULL;
          } /* endif */
        }
        // try to load context related user exit functions
        if ( ppfnGetSegContext )
        {
          usRC = DosGetProcAddr( *phModule, EQFGETSEGCONTEXT_EXIT, (PFN*)ppfnGetSegContext );
          if ( usRC != NO_ERROR ) *ppfnGetSegContext = NULL;
        }
        if ( ppfnUpdateContext )
        {
          usRC = DosGetProcAddr( *phModule, EQFUPDATECONTEXT_EXIT, (PFN*)ppfnUpdateContext );
          if ( usRC != NO_ERROR ) *ppfnUpdateContext = NULL;
        }
        if ( ppfnCompareContext )
        {
          usRC = DosGetProcAddr( *phModule, EQFCOMPARECONTEXT_EXIT, (PFN*)ppfnCompareContext );
          if ( usRC != NO_ERROR ) *ppfnCompareContext = NULL;
        }
        if ( ppfnFormatContext )
        {
          usRC = DosGetProcAddr( *phModule, EQFFORMATCONTEXT_EXIT, (PFN*)ppfnFormatContext );
          if ( usRC != NO_ERROR ) *ppfnFormatContext = NULL;
        }
        if ( ppfnUserExitW )
        {
           usRC = DosGetProcAddr( *phModule, EQFPROTTABLEW_EXIT, (PFN*)ppfnUserExitW);
           if ( usRC != NO_ERROR )  *ppfnUserExitW = NULL;
        }
        if ( ppfnCheckSegExitW )
        {
          usRC = DosGetProcAddr( *phModule, EQFCHECKSEGW_EXIT, (PFN*)ppfnCheckSegExitW);
          if ( usRC != NO_ERROR )     *ppfnCheckSegExitW = NULL;
        }
        if ( ppfnCheckSegExExitW )
        {
          usRC = DosGetProcAddr( *phModule, EQFCHECKSEGEXW_EXIT, (PFN*)ppfnCheckSegExExitW);
          if ( usRC != NO_ERROR )     *ppfnCheckSegExExitW = NULL;
        }
     
      } /* endif */
    }
  } /* endif */

  return( NO_ERROR );
} /* end of function TAPrepProtectTable */


USHORT TAEndProtectTable
(
  HMODULE          *phModule,          // address of user exit module handle
  PFN              *ppfnUserExit ,     // address of ptr to user exit function
  PFN              *ppfnCheckSegExit,  // address of ptr to segment check func
  PFN              *ppfnWYSIWYGExit,   // address of ptr to show transl.  func
  PFN              *ppfnTocGotoExit,    // address of ptr to TOC func
  PFN              *ppfnUserExitW,      // unicode UserExit to make StartStoptable
  PFN              *ppfnCheckSegExitW   // unicode CheckSegExit
)
{
  TAFreeEditUserExit( phModule );

  if ( ppfnUserExit )    *ppfnUserExit = NULL;
  if ( ppfnCheckSegExit) *ppfnCheckSegExit = NULL;
  if ( ppfnWYSIWYGExit ) *ppfnWYSIWYGExit = NULL;
  if ( ppfnTocGotoExit ) *ppfnTocGotoExit = NULL;
  if ( ppfnUserExitW )    *ppfnUserExitW = NULL;
  if ( ppfnCheckSegExitW) *ppfnCheckSegExitW = NULL;

  return( NO_ERROR );
} /* end of function TAEndProtectTable */

USHORT TAFreeEditUserExit
(
  HMODULE      *phModule
)
{
  if ( *phModule )
  {
      DosFreeModule( *phModule );
      *phModule = NULLHANDLE;
  } /* endif */
  return( NO_ERROR );
} /* end of function TAFreeEditUserExit */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TAFastTokenize
//------------------------------------------------------------------------------
// Function call:     TAFastTokenize(PSZ,PLOADEDTABLE,BOOL,PSZ,PSUHORT
//                              PTOKENENTRY,USHORT)
//------------------------------------------------------------------------------
// Description:       Replacement for EqfTagTokenize to allow a faster
//                    tokenization of segmented documents
//                    (TAFastTokenize needs only 20% of the time
//                    EqfTagTokenize needs to tokenize a specific file).
//                    TAFastTokenize requires preprocessed tag and
//                    attribute tables (tree like character arrays)
//                    and does not support all functions of EqfTagTokenize.
//                    Only functions required for segmented Troja
//                    documents have been included.
//------------------------------------------------------------------------------
// Parameters:        PSZ       pszInput,    pointer to input data
//                    PLOADEDTABLE pTable,   pointer to loaded tag table
//                    BOOL      fComplete,   TRUE = no more buffers to follow
//                    PSZ      *ppRest,      ptr to not processed data in buffe
//                    PUSHORT   pusColPos,   column position (not used)
//                    PTOKENENTRY pTokBuf,   buffer for created tokens
//                    USHORT    usTokens     max entries in token buffer
//------------------------------------------------------------------------------
// Returncode type:   none
//------------------------------------------------------------------------------
// Prerequesits:      Tag and attribute node trees for loaded tag table
//                       are required
//------------------------------------------------------------------------------
// Function flow:     while not end of data
//                       lookup current character in current node list
//                       if found
//                          if this is not the last node of this branch
//                             set current node to next node
//                          else
//                             complete open data token if any
//                             create tag token
//                             process tag attributes
//                             prepare next token
//                       else
//                          reset current node to root node
//                    complete current token
//                    handle any incomplete data / tag
//                    add end-of-list token
//------------------------------------------------------------------------------
VOID TAFastTokenizeW
(
   PSZ_W     pszInput,                 // pointer to input data
   PLOADEDTABLE     pVoidTable,               // pointer to loaded tag table
   BOOL      fComplete,                // TRUE = no more buffers to follow
   PSZ_W    *ppRest,                   // pointer to not processed data in buffer
   PUSHORT   pusColPos,                // column position (not used)
   PTOKENENTRY pTokBuf,                // buffer for created tokens
   USHORT    usTokens                  // max entries in token buffer
)
{
   CHAR_W  chTest;                       // buffer for currently tested character
   CHAR_W  chNode;                       // buffer for current node character
   CHAR_W  chTagEnd;                     // tag end character
   PNODE pNode;                        // pointer to active tag node
   PNODE pAttrNode;                    // pointer to active attribute node
   PSZ_W   pszData;                      // pointer for input data processing
   PSZ_W   pszEnd;                       // pointer for end of data block
   PSZ_W   pszTagStart;                  // pointer to the start of a tag
   PTOKENENTRY pToken;                 // pointer to current token
   PSZ_W         pszAttr;                // pointer to start of tag attributes
   PSZ_W         pTagNamesW;              // pointer to name buffer in tagtable
   PTAGSTART   pTagStart;              // pointer to name buffer in tagtable
   PSZ           pszEndDel;              // pointer to end delimiter
   CHAR_W        chTagEndChar;           // tag end character
   BOOL        fInTag = FALSE;         // TRUE = one or more tag characters
                                       //        recognized
   BOOL        fFound = FALSE;         // flag used in setting pRest
   PLOADEDTABLE pTable = (PLOADEDTABLE) pVoidTable;   // pointer to loaded tag table
   BOOL        fString;                // string identifier found

   pusColPos ;                         // get rid of compiler warnings
   pszData = TAG_END_CHARW;
   chTagEndChar = *pszData;

   /*******************************************************************/
   /* Create tag and attribute trees if not done yet                  */
   /*******************************************************************/
   if ( pTable->pTagTree == NULL )
   {
     pTable->pTagTree = TAFastCreateTagTree( pTable, pTable->pTagTable );
   } /* endif */
   if ( pTable->pAttrTree == NULL )
   {
     pTable->pAttrTree = TAFastCreateAttrTree( pTable, pTable->pTagTable );
   } /* endif */

   /*******************************************************************/
   /* find end character for tags depending on tag table....          */
   /*******************************************************************/
   pTagStart = (PTAGSTART)( (PBYTE)pTable->pTagTable + pTable->pTagTable->uTagstart);

   pszEndDel = (PSZ) ((PBYTE)pTable->pTagTable + pTagStart->uEndDelim);

   while ( *pszEndDel )
   {
     if ( *pszEndDel == BLANK )
     {
       pszEndDel++;
     }
     else
     {
       chTagEndChar = *pszEndDel;
       break;
     } /* endif */
   } /* endwhile */

   //pTagNames = (PSZ)( (PBYTE)pTable->pTagTable + pTable->pTagTable->uTagNames);
   pTagNamesW =  pTable->pTagNamesW;

   pszData = pszInput;                 // start at begin of input buffer
   pNode   = pTable->pTagTree;         // start at begin of node tree
   pToken  = pTokBuf;                  // start at begin of token buffer
   pToken->sTokenid = TEXT_TOKEN;      // assume text data
   pToken->pDataStringW = pszData;      // remember start of data
   usTokens--;                         // reserve space for end of list token

   while ( (chTest = ((chTest = *pszData++) <= 0x0ff ) ? chLookupW[chTest] : chTest ) != 0)
   //while( chTest = chLookupW[(UCHAR)*pszData++] )     // while not end of data
   {
      // look for character in current node list
      while ( ((chNode = pNode->chNode) != NULC) && (chNode != chTest) )
      {
         pNode++;
      } /* endwhile */
      if ( chNode )
      {
         if ( pNode->pNext )
         {
            pNode = pNode->pNext;
            fInTag = TRUE;             // we are leaving the outmost level
         }
         else
         {
            // gotcha! a new tag has been located
            pszTagStart = pszData - pNode->chPos;

            // complete open data token if any
            pToken->usLength = (USHORT)(pszTagStart - pToken->pDataStringW);
            if ( pToken->usLength )
            {
               pToken++;
               usTokens--;
            } /* endif */

            // exit loop if token table is full
            if ( !usTokens )
            {
               break;
            } /* endif */

            // create tag token
            pToken->sTokenid = pNode->sTokenID;
            pToken->usLength = pNode->chPos;
            pToken->pDataStringW = pszTagStart;
            pToken++;
            usTokens--;

            // exit loop if token table is full
            if ( !usTokens )
            {
               break;
            } /* endif */

            // look for end of tag

            // process attributes (only if tag not ended by end delimiter)
            if ( pNode->chNode != chTagEndChar )
            {
              CHAR_W chTemp;
              pszAttr = pszData;         // remember start of tag attributes
              while ( *pszData && ((CHAR) *pszData != chTagEndChar) )
              {
                 pszData++;
              } /* endwhile */
              chTagEnd = *pszData;       // remember tag end character
              *pszData = EOS;                    // terminate atttribute data
              // skip whitespaces
              while ( (chTemp = ((chTemp = *pszAttr) <= 0x0ff ) ? chLookupW[chTemp] : chTemp) == ' ' )
              {
                pszAttr++;
              } /* endwhile */
               //while( chLookupW[(UCHAR)*pszAttr] == ' ' ) pszAttr++; // skip whitespace

              pAttrNode = pTable->pAttrTree;      // start at begin of node tree
              pToken->pDataStringW = pszAttr;      // remember start of data
              do
              {
                 // exit loop if token table is full
                 if ( !usTokens )
                 {
                    break;
                 } /* endif */
                 chTest = ((chTest = *pszAttr++) <= 0x0ff ) ? chLookupW[chTest] : chTest;
                // chTest = chLookupW[(UCHAR)*pszAttr++];
                 if ( !chTest || (chTest == ' ') ) // attribute end reached ???
                 {
                    if ( pszAttr - pToken->pDataStringW > 1 ) // any data ???
                    {
                       pToken->sTokenid = pAttrNode->sTokenID;
                       pToken->usLength = (USHORT)(pszAttr - pToken->pDataStringW - 1);
                       pToken++;
                       usTokens--;
                       pAttrNode = pTable->pAttrTree; // start at begin of node tree
                       // skip whitespaces
                       while ((chTemp = ((chTemp = *pszAttr) <= 0x0ff ) ? chLookupW[chTemp] : chTemp) == ' ' )
                       {
                         pszAttr++;
                       } /* endwhile */

                       //while( chLookupW[(UCHAR)*pszAttr] == ' ' ) pszAttr++; // skip whitespace
                       pToken->pDataStringW = pszAttr; // remember start of data
                    } /* endif */
                 }
                 else
                 {
                    chNode = pAttrNode->chNode;
                    if ( chNode != CHAR_MULT_SUBST )
                    {
                       while ( ((chNode = pAttrNode->chNode)!= NULC)  &&
                               (chNode != CHAR_SNGL_SUBST) &&
                               (chNode != chTest) )
                       {
                          pAttrNode++;
                       } /* endwhile */
                       if ( chNode )
                       {
                          if ( pAttrNode->pNext )
                          {
                             pAttrNode = pAttrNode->pNext;
                          }
                          else
                          {

                             pToken->sTokenid = pAttrNode->sTokenID;
                             pToken->usLength = (USHORT)(pszAttr - pToken->pDataStringW);
                             pToken++;
                             usTokens--;
                             pAttrNode = pTable->pAttrTree; // start at begin of node tree

                             while ((chTemp = ((chTemp = *pszAttr) <= 0x0ff ) ? chLookupW[chTemp] : chTemp) == ' ' )
                             {
                               pszAttr++;
                             } /* endwhile */

                             //while( chLookupW[(UCHAR)*pszAttr] == ' ' ) pszAttr++; // skip whitespace
                             pToken->pDataStringW = pszAttr; // remember start of data
                          } /* endif */
                       }
                       else
                       {
                          pAttrNode = pTable->pTagTree;
                       } /* endif */
                    }
                    else
                    {
                      CHAR_W chQuote;
                      CHAR_W chTemp;

                      /**************************************************/
                      /* skip string tags..                             */
                      /**************************************************/
                      if ( (chTest == QUOTE) || (chTest == DBLQUOTE) )
                      {
                        fString = TRUE;
                        chQuote = chTest;
                        while ( ((chTest = ((chTest = *pszAttr) <= 0x0ff ) ? chLookupW[chTest] : chTest) != 0) && fString )
                        //while ( (chTest=chLookupW[(UCHAR)*pszAttr]) && fString )
                        {
                          if ( chTest == chQuote )
                          {
                            /********************************************/
                            /* if two consecutive QUOTES we are dealing */
                            /* with inline quote else we are done..     */
                            /********************************************/
                            if ( (chTemp = ((chTemp = *(pszAttr+1)) <= 0x0ff ) ? chLookupW[chTemp] : chTemp)  != chQuote)
                            //if ( chLookupW[(UCHAR)*(pszAttr+1)] != chQuote )
                            {
                              fString = FALSE;
                            }
                            else
                            {
                              pszAttr += 2;
                            } /* endif */
                          }
                          else
                          {
                              pszAttr ++;
                          } /* endif */
                        } /* endwhile */
                        pszAttr ++;               // point to next active charact.
                      } /* endif */
                    } /* endif */
                 } /* endif */
              } while( chTest );

              // correct tag end character
              if ( chTagEnd )            // if normal tag end ...
              {
                 *pszData = chTagEnd;    // restore tag end character
                 pszData++;              // skip tag end character
              }
              else
              {
                 pszData--;              // reposition to last character
              } /* endif */
            } /* endif */

            // prepare next token
            pNode = pTable->pTagTree;           // start at begin of node tree
            pToken->sTokenid = TEXT_TOKEN;     // assume text data
            pToken->pDataStringW = pszData;     // remember start of data

         } /* endif */
      }
      else
      {
         if ( fInTag )
         {
            fInTag = FALSE;            // we are on the outmost level again
            pszData--;                 // redo last character
         } /* endif */
         pNode = pTable->pTagTree;     // start at begin of node tree again
      } /* endif */
      // exit loop if token table is full
      if ( !usTokens )
      {
         break;
      } /* endif */
   } /* endwhile */

   // complete current token
   pToken->usLength = (USHORT)(pszData - pToken->pDataStringW - 1);
   pszEnd = pszData - 1;               // remember end position

   // if not processed completely or not last part of file
   // set pRest pointer to begin of last tag or (if no tags
   // are contained in data) to last linebreak.
   if ( !usTokens ||                   // end of token table reached
        !fComplete )                   // or not last block of data  ???
   {
      if ( pToken != pTokBuf )         // if tokens in buffer
      {
         pToken--;                    // go back to last text or tag token
         while ( pToken != pTokBuf && !fFound )
         {
            switch ( pToken->sTokenid )
            {
              case QFF_TAG:
              case QFN_TAG:
              case QFA_TAG:
              case QFC_TAG:
              case QFJ_TAG:
              case QFS_TAG:
              case QFX_TAG:
                 fFound = TRUE;
                 break;
              case TEXT_TOKEN:
                 if ( (pToken-1)->sTokenid != NONE_TAG)
                 {
                   fFound = TRUE;
                 }
                 else
                 {
                   if ( (pToken-1) != pTokBuf )
                   {
                     pToken--;
                   }
                   else
                   {
                     fFound = TRUE;
                   } /* endif */
                 } /* endif */
                 break;
              default :
                 pToken--;
                 break;
            } /* endswitch */
         } /* endwhile */
         *ppRest = pToken->pDataStringW;
      }
      else
      {
         pszData--;
         while ( pszData > pszInput )
         {
            if ( *pszData == LF )
            {
               break;                  // leave while loop
            }
            else
            {
               pszData--;              // test previous character
            } /* endif */
         } /* endwhile */

         if ( pszData == pszInput )    // no linefeed found ???
         {
            *ppRest = pszEnd - 1;      // set pRest to end of data
         }
         else
         {
            *ppRest = pszData;         // set pRest to last linefeed
         } /* endif */
      } /* endif */
   }
   else if ( pToken->usLength > 1 )   // last token not completed
   {
      *ppRest = pToken->pDataStringW;
   }
   else
   {
      *ppRest = NULL;
   } /* endif */

   // add end-of-list token
   pToken->usLength = 0;
   pToken->sTokenid = ENDOFLIST;
}

VOID TAFastTokenize
(
   PSZ       pszInput,                // pointer to input data
   PLOADEDTABLE     pVoidTable,               // pointer to loaded tag table
   BOOL      fComplete,                // TRUE = no more buffers to follow
   PSZ       *ppRest,                 // pointer to not processed data in buffer
   PUSHORT   pusColPos,                // column position (not used)
   PTOKENENTRY pTokBuf,                // buffer for created tokens
   USHORT    usTokens,                  // max entries in token buffer
   ULONG     ulCP
)
{
     PSZ_W        pRestW = NULL;
   PSZ_W        pszInputW = &chInputW[0];
   ULONG        ulUsed = strlen(pszInput)+1;
   USHORT       usTokLengthW = 0;
   ULONG        ulBytesInAscii;
   PTOKENENTRY  pNextTokBuf;
   PTOKENENTRY  pCurTokBuf;

   memset( &chInputW, 0, sizeof( chInputW ));


   ASCII2UnicodeBuf( pszInput, pszInputW, (USHORT)ulUsed, ulCP );

   TAFastTokenizeW( pszInputW, pVoidTable, fComplete, &pRestW, pusColPos, pTokBuf, usTokens);
    pCurTokBuf = pTokBuf;
    if ( pCurTokBuf->pDataStringW)
      {
        if ( pCurTokBuf->pDataStringW == pszInputW)
        {
           pCurTokBuf->pDataString = pszInput;
        }
        else
        {
          ulBytesInAscii = WideCharToMultiByte( ulCP, 0, pszInputW,
                                                pCurTokBuf->pDataStringW - pszInputW,
                                               NULL, 0 , NULL, NULL );
          pCurTokBuf->pDataString = pszInput + ulBytesInAscii;
        }
      } /* endwhile */

      while (pCurTokBuf->pDataStringW)
      {
          usTokLengthW = pCurTokBuf->usLength;

          ulBytesInAscii = WideCharToMultiByte( ulCP, 0, pCurTokBuf->pDataStringW,
                                                usTokLengthW, NULL, 0 , NULL, NULL );
          pCurTokBuf->usLength = (USHORT)ulBytesInAscii;
          // set next pDataString ptr
          pNextTokBuf = pCurTokBuf + 1;
          if ( pCurTokBuf->pDataStringW + usTokLengthW == pNextTokBuf->pDataStringW)
          {
            pNextTokBuf->pDataString = pCurTokBuf->pDataString + ulBytesInAscii;
          }
          else
          {
            ulBytesInAscii = WideCharToMultiByte( ulCP, 0, pCurTokBuf->pDataStringW,
                                                   pNextTokBuf->pDataStringW - pCurTokBuf->pDataStringW ,
                                                   NULL, 0 , NULL, NULL );
            pNextTokBuf->pDataString = pCurTokBuf->pDataString + ulBytesInAscii;
          }
          pCurTokBuf->pDataStringW = NULL;
          pCurTokBuf++;
       }
       if (pRestW != NULL)
       {
         ulUsed = pRestW - pszInputW;
         ulBytesInAscii = WideCharToMultiByte( ulCP, 0, pszInputW,
                                               pRestW - pszInputW,
                                               NULL, 0, NULL, NULL );
         (*ppRest) = pszInput + ulBytesInAscii;
       }
       else
       {
           *ppRest = NULL;
       }


}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TATagCompare
//------------------------------------------------------------------------------
// Function call:     TATagCompare(PTBTAGENTRY,PTBTAGENTRY)
//------------------------------------------------------------------------------
// Description:       Compare routine for two tags (called by qsort)
//------------------------------------------------------------------------------
// Parameters:        PTBTAGENTRY  pElem1
//                    PTBTAGENTRY  pElem2
//------------------------------------------------------------------------------
// Returncode type:   INT
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     compare two tags
//------------------------------------------------------------------------------

int TATagCompare ( const void *pElem1, const void *pElem2 )
{
   return( UTF16strcmp( ((PTBTAGENTRY)pElem1)->pszTag,
                   ((PTBTAGENTRY)pElem2)->pszTag ) );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TAFastCreateTagTree
//------------------------------------------------------------------------------
// Function call:     TAFastCreateTagTree(PTAGTABLE)
//------------------------------------------------------------------------------
// Description:       Create a tag tree required for TAFastTokenize
//------------------------------------------------------------------------------
// Parameters:        PTAGTABLE pTagTable     // ptr to a loaded tag table
//------------------------------------------------------------------------------
// Returncode type:   PNODE
//------------------------------------------------------------------------------
// Returncodes:       pRootNode       // ptr to create tag tree
//------------------------------------------------------------------------------
// Function flow:     address tag table lists
//                    allocate tag entry array
//                    extract tags from tag table
//                    call TAFastCreateNodeTree to create the tag tree
//                    free tag entry array
//                    return created tag tree
//------------------------------------------------------------------------------

PNODE TAFastCreateTagTree( PLOADEDTABLE pLoadedTable, PTAGTABLE pTagTable )
{
   PTAG        pTag;                   // pointer to structure of active tag
   PSZ_W       pTagNamesW;              // pointer to start of tagnames
   PBYTE       pByte;                  // helper pointer
   ULONG       ulNoOfTags;             // number of tags to process
   BOOL        fOK = TRUE;             // internal OK flag
   USHORT      usI;                    // general loop index
   PTBTAGENTRY pExtractedTags;         // ptr to tags extracted from tag table
   PNODE       pRoot = NULL;           // root node of generated node tree

   // address tag table lists
   pByte = (PBYTE) pTagTable;
   pTag = (PTAG) ( pByte + pTagTable->stFixTag.uOffset);
   //pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);
   pTagNamesW =   pLoadedTable->pTagNamesW ;
   ulNoOfTags = pTagTable->stFixTag.uNumber;

   // allocate tag entry array
   fOK = UtlAlloc((PVOID *) &pExtractedTags, 0L,
                   (LONG) (ulNoOfTags * sizeof(TBTAGENTRY)),
                   ERROR_STORAGE);

   // extract tags from tag table
   if ( fOK )
   {
      for ( usI = 0; usI < ulNoOfTags; usI++ )
      {
         pExtractedTags[usI].pszTag = pTag->uTagnameOffs + pTagNamesW;
         pExtractedTags[usI].sID    = usI;
         pTag++;
      } /* endfor */
      pRoot = TAFastCreateNodeTree( pExtractedTags, ulNoOfTags );
   } /* endif */


   if ( fOK )
   {
      UtlAlloc((PVOID *) &pExtractedTags, 0L, 0L, NOMSG );
   } /* endif */

   return ( pRoot );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TAFastCreateNodeTree
//------------------------------------------------------------------------------
// Function call:     TAFastCreateNodeTree(PTBTAGENTRY,USHORT)
//------------------------------------------------------------------------------
// Description:       Create a node tree out of a tag list. The node tree is
//                    used in TAFastTokenize to speedup the lookup of tags.
//                    Sample of a node tree for the tags ":ATAGA", ":ATAGB",
//                      ":ETAG":
//                             +--'E'-'T'-'A'-'G'
//                    root ':'-+                      +--'A'
//                             +--'A'-'T'-'A'-'G'-+
//                                                    +--'B'
//------------------------------------------------------------------------------
// Parameters:        PTBTAGENTRY pTags       // ptr to tag entry array
//                    USHORT      ulNoOfTags  // number of tags in tag entry
//                                               array
//------------------------------------------------------------------------------
// Returncode type:   PNODE
//------------------------------------------------------------------------------
// Returncodes:       pRootNode    // root of created node tree or
//                                // NULL in case of errors
//------------------------------------------------------------------------------
// Function flow:     sort tags
//                    allocate tag tree
//                    create first level of nodes (= list of first characters)
//                    process nodes in list and complete any uncompleted node
//                    convert offset values in node list to pointers
//                    free any allocated memory in case of errors
//                    return root of node tree to caller
//------------------------------------------------------------------------------

PNODE TAFastCreateNodeTree( PTBTAGENTRY pTags, ULONG ulNoOfTags )
{
   BOOL        fOK = TRUE;             // internal OK flag
   USHORT      usI;                    // general loop index
   CHAR_W        chLastChar;             // last processed character
   ULONG       ulAllocNodes;           // number of allocated nodes
   ULONG       ulNodes = 0;            // number of active node
   PNODE       pRootNode = NULL;       // root of node tree
   PNODE       pNode;                  // currently processed node
   USHORT      usCharPos;              // currently tested character position
   NODE        NewNode;                // buffer for new nodes
   ULONG       ulNode;                 // number of current node
   USHORT      usCurrent;              // number of current tag
   PSZ_W         pszCurrent;             // pointer to current tag
   USHORT      usPos;                  // current position in tag
//   USHORT      usTempNode;

   // sort tags
   qsort( (VOID *) pTags, (size_t)ulNoOfTags,
          sizeof(TBTAGENTRY), TATagCompare );

   // allocate tag tree
   if ( fOK )
   {
      ulAllocNodes = ulNoOfTags * 3;
      fOK = UtlAlloc((PVOID *) &pRootNode, 0L, (LONG) (ulAllocNodes * sizeof(NODE)),
                      ERROR_STORAGE);
   } /* endif */

   // create first level of nodes (= list of first characters)
   if ( fOK )
   {
      usCharPos = 0;                   // start with first character position
      ulNodes   = 0;                   // no nodes created yet
      chLastChar = NULC;               // no character processed yet

      for ( usI = 0; fOK, usI < ulNoOfTags; usI++ )
      {
         if ( (CHAR_W) pTags[usI].pszTag[0] != chLastChar )
         {
            // create new node
            NewNode.chPos     = 1;
            NewNode.chNode    = chLastChar = pTags[usI].pszTag[0];
            NewNode.sTokenID  = usI;
            NewNode.pNext     = NULL;
            fOK = TAFastAddNode( &pRootNode, &NewNode, &ulNodes, &ulAllocNodes );
         } /* endif */
      } /* endfor */

      // create seperator node
      memset( &NewNode, 0, sizeof(NODE) );
      fOK = TAFastAddNode( &pRootNode, &NewNode, &ulNodes, &ulAllocNodes );
   } /* endif */

   // process node list and complete any uncompleted node
   if ( fOK )
   {
      ulNode = 0;                      // start with first node in list

      while ( ulNode < ulNodes)
      {
         pNode = pRootNode + ulNode;
         if ( pNode->chNode )          // process data nodes only
         {
            usCurrent  = pNode->sTokenID;
            pszCurrent = pTags[usCurrent].pszTag;
            usPos      = pNode->chPos;
            if ( pszCurrent[usPos] )
            {
               // process new level in node list
               chLastChar = NULC;      // no character processed yet
               pNode->pNext = (PNODE) ulNodes;
               usI = usCurrent;
               while ( fOK                &&
                       (usI < ulNoOfTags) &&
                       (UTF16strncmp( pszCurrent, pTags[usI].pszTag, usPos ) == 0) )
               {
                  if ( (CHAR_W) pTags[usI].pszTag[usPos] != chLastChar )
                  {
                     // create new node
                     NewNode.chPos  = (CHAR_W)(usPos + 1);
                     NewNode.chNode = chLastChar = pTags[usI].pszTag[usPos];
                     NewNode.sTokenID = usI;
                     NewNode.pNext    = NULL;
                     fOK = TAFastAddNode( &pRootNode, &NewNode,
                                        &ulNodes, &ulAllocNodes );
                  } /* endif */
                  usI++;
               } /* endwhile */

               // create seperator node
               memset( &NewNode, 0, sizeof(NODE) );
               fOK = TAFastAddNode( &pRootNode, &NewNode,
                                  &ulNodes, &ulAllocNodes );
            }
            else
            {
               // complete current node
               pNode->sTokenID = pTags[pNode->sTokenID].sID;
               pNode->pNext = NULL;
            } /* endif */
         } /* endif */
         ulNode++;
      } /* endwhile */
   } /* endif */

   // convert offset values in node list to pointers
   if ( fOK )
   {
      ulNode = 0;                      // start with first node in list
      while ( ulNode < ulNodes)
      {
         pNode = pRootNode + ulNode;
         if ( pNode->pNext )
         {
//            usTempNode = (USHORT)(ULONG)(pNode->pNext);
//            pNode->pNext = pRootNode + usTempNode;
            pNode->pNext = pRootNode + (USHORT)(ULONG)pNode->pNext;
         } /* endif */
         ulNode++;
      } /* endwhile */
   } /* endif */

   // free allocated memory in case of errors
   if ( !fOK && pRootNode )
   {
      UtlAlloc((PVOID *) &pRootNode, 0L, 0L, NOMSG );
   } /* endif */

   return( pRootNode );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TAFastAddNode
//------------------------------------------------------------------------------
// Function call:     TAFastAddNode(PNODE,PNODE,PUSHORT,PUSHORT)
//------------------------------------------------------------------------------
// Description:       add a new node to the node list
//------------------------------------------------------------------------------
// Parameters:        PNODE   *ppNodeList      // ptr to active node list
//                    PNODE   pNewNode         // ptr to node to be added
//                    PUSHORT pusUsed          // number of used nodes
//                    PUSHORT pusAlloc         // number of allocated nodes
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  if successful
//                    FALSE if (memory allocation) errors occured
//------------------------------------------------------------------------------
// Function flow:     enlarge node list if required
//                    if OK add new node
//------------------------------------------------------------------------------

BOOL TAFastAddNode
(
   PNODE   *ppNodeList,                // ptr to active node list
   PNODE   pNewNode,                   // ptr to node to be added
   PULONG  pulUsed,                    // number of used nodes
   PULONG  pulAlloc                    // number of allocated nodes
)
{
   BOOL  fOK = TRUE;                   // internal OK flag
   PNODE pFreeNode;                    // ptr to next free entry in node list

   // check size of node list
   if ( *pulUsed >= *pulAlloc )
   {
      fOK = UtlAlloc((PVOID *) ppNodeList,
                      (LONG) (*pulAlloc * sizeof(NODE)),
                      (LONG) ((*pulAlloc + 10) * sizeof(NODE)),
                      ERROR_STORAGE );
      *pulAlloc += 10;
   } /* endif */

   // add new node
   if ( fOK )
   {
      pFreeNode = *ppNodeList + *pulUsed;
      memcpy( pFreeNode, pNewNode, sizeof(NODE) );
      *pulUsed += 1;
   } /* endif */

   return( fOK );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TAFastCreateAttrTree
//------------------------------------------------------------------------------
// Function call:     TAFastCreateAttrTree(PTAGTABLE)
//------------------------------------------------------------------------------
// Description:       Create an attribute tree required for TAFastTokenize
//------------------------------------------------------------------------------
// Parameters:       PTAGTABLE pTagTable // ptr to a loaded tag table
//------------------------------------------------------------------------------
// Returncode type:   PNODE
//------------------------------------------------------------------------------
// Returncodes:       pRootNode   // ptr to create tag tree
//------------------------------------------------------------------------------
// Function flow:     address tag table lists
//                    allocate tag entry array
//                    extract fixed attributes from tag table
//                    extract varaible attributes from tag table
//                    call TAFastCreateNodeTree to create the attribute tree
//                    free tag entry array
//                    return created attribute tree
//------------------------------------------------------------------------------

PNODE TAFastCreateAttrTree(PLOADEDTABLE pLoadedTable,  PTAGTABLE pTagTable )
{
   PATTRIBUTE  pAttr;                  // ptr to structure of active attribute
   PSZ_W       pTagNamesW;              // ptr to start of tagnames
   PBYTE       pByte;                  // helper pointer
   ULONG       ulNoOfTags;             // number of tags to process
   BOOL        fOK = TRUE;             // internal OK flag
   USHORT      usI, usJ;               // general loop index
   USHORT      usEntry;                // current entry in pExtractedTags
   PTBTAGENTRY pExtractedTags;         // ptr to tags extracted from tag table
   PNODE       pRoot = NULL;           // root node of generated node tree
   SHORT       sID;                    // IDs of attributes
   ULONG       ulLength;               // length of attribute array in bytes

   // address tag table lists
   pByte = (PBYTE) pTagTable;
   //pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);
   pTagNamesW =   pLoadedTable->pTagNamesW ;
   ulNoOfTags = pTagTable->stAttribute.uNumber;
   for ( usI = 0; usI < 27; usI++ )
   {
      ulNoOfTags += pTagTable->stAttributeIndex[usI].uNumber;
   } /* endfor */
   sID = pTagTable->uNumTags;

   // allocate attribute entry array
   ulLength = ulNoOfTags * sizeof(TBTAGENTRY);
   fOK = UtlAlloc((PVOID *) &pExtractedTags, 0L,
                   (LONG) max( ulLength, MIN_ALLOC ),
                   ERROR_STORAGE);

   // extract fixed attributes from tag table
   usEntry = 0;                      // start with first entry in table
   sID = pTagTable->uNumTags;          // set ID of first attribute
   pAttr = (PATTRIBUTE) ( pByte + pTagTable->stAttribute.uOffset);
   if ( fOK )
   {
      for ( usI = 0; usI < pTagTable->stAttribute.uNumber; usI++ )
      {
         pExtractedTags[usEntry].pszTag = pAttr->uStringOffs + pTagNamesW;
         pExtractedTags[usEntry].sID    = sID++;
         pAttr++;
         usEntry++;
      } /* endfor */
   } /* endif */

   // extract variable attributes from tag table
   if ( fOK )
   {
      for ( usJ = 0; usJ < 27; usJ++ )
      {
         pAttr = (PATTRIBUTE) ( pByte + pTagTable->stAttributeIndex[usJ].uOffset);
         for ( usI = 0; usI < pTagTable->stAttributeIndex[usJ].uNumber; usI++ )
         {
            pExtractedTags[usEntry].pszTag = pAttr->uStringOffs + pTagNamesW;
            pExtractedTags[usEntry].sID    = sID++;
            pAttr++;
            usEntry++;
         } /* endfor */
      } /* endfor */
      pRoot = TAFastCreateNodeTree( pExtractedTags, ulNoOfTags );
   } /* endif */


   if ( fOK )
   {
      UtlAlloc((PVOID *) &pExtractedTags, 0L, 0L, NOMSG );
   } /* endif */

   return ( pRoot );
}

// P016804:
 static void
 TATagAddWSpace
   (
	   PSZ_W                pszSegment,
	   PTOKENENTRY  	    pTokBuffer,
	   SHORT                sNumTags
   )
   {
     PTOKENENTRY      pTok;
     USHORT           usRC = NO_ERROR;
     PTOKENENTRY      pNextTok;
     USHORT           usStart = 0;
     USHORT           usNextStart = 0;
     PSZ_W            pTmp = NULL;
     USHORT           usI = 0;
     USHORT           usNewLen = 0;

        pTok     = pTokBuffer;
        pNextTok = pTok + 1;
        while ( (pTok->sTokenid != ENDOFLIST) && (usRC == NO_ERROR) && (pNextTok->sTokenid != ENDOFLIST))
        {
		  if (TA_IS_TAG(pTok->sTokenid, sNumTags) || TA_IS_ATTRIBUTE(pTok->sTokenid, sNumTags))
		  {
			  if (TA_IS_ATTRIBUTE(pNextTok->sTokenid, sNumTags))
			  { // if there are whitespace between pTok and pNextTok, add them to pTok
				 usStart = (USHORT)(pTok->pDataStringW - pszSegment);
				 usNextStart = (USHORT)(pNextTok->pDataStringW - pszSegment);
				 usI = usStart + pTok->usLength ;
				 pTmp = pTok->pDataStringW + pTok->usLength;
				 usNewLen = pTok->usLength;
				 while ( usI < usNextStart && iswspace(*pTmp))
				 {
					 usI++;
					 pTmp++;
					 usNewLen ++;
				 }
				 if (usI == usNextStart)
				 { // add the whitespaces to the length of pTok!
					pTok->usLength = usNewLen;
				 }
			  }
		   } /* endif */
           pTok ++;
		   pNextTok++;
        } /* endwhile */
        return;
	}

// test if markup table contains tags with a special class ID
BOOL ContainsClassID( PLOADEDTABLE pLoadedTable, USHORT usClassID )
{
   PBYTE       pByte;                  // helper pointer
   ULONG       ulNoOfTags;             // number of tags to process
   PTAGTABLE   pTagTable = pLoadedTable->pTagTable;
   PTAGADDINFO pTagAddInfo;            // additional taginfo (tokenid, classid)
   USHORT      usTag = 0;

   // return asap when table contains no class information
   if ( pTagTable->usVersion < ADDINFO_VERSION ) return( FALSE );

   // address tag table lists
   pByte = (PBYTE) pTagTable;
   ulNoOfTags = pTagTable->stFixTag.uNumber;
   pTagAddInfo = (PTAGADDINFO) (pByte + pTagTable->uAddInfos);

   // check tag class IDs
   {
     USHORT usI = 0;
     for ( usI = 0; usI < ulNoOfTags; usI++ )
     {
       if ( pTagAddInfo[usTag].ClassId == usClassID ) return( TRUE );
       usTag++;
     } /* endfor */
   } 

   {
     USHORT usJ = 0;
     for ( usJ = 0; usJ < 27; usJ++ )
     {
       USHORT usI = 0;
       for ( usI = 0; usI < pTagTable->stTagIndex[usJ].uNumber; usI++ )
       {
         if ( pTagAddInfo[usTag].ClassId == usClassID ) return( TRUE );
         usTag++;
       } /* endfor */
     } /* endfor */
   } 

   {
     USHORT usI = 0;
     for ( usI = 0; usI < pTagTable->stVarStartTag.uNumber; usI++ )
     {
        if ( pTagAddInfo[usTag].ClassId == usClassID ) return( TRUE );
        usTag++;
     } /* endfor */
  }


  return ( FALSE );
}

