// Copyright (c) 1996-2012, International Business Machines
// Corporation and others.  All rights reserved.
/*--------------------------------------------------------------------------*\
|    FILE NAME: EQFTMUTL.C - Utilities.............                          |
|  TMT                                                                       |
|  GetFirstSegAddress                                                        |
|  CleanSource                                                               |
|  CalcPrimaryKey                                                            |
|  CalcEntryInKeyDir                                                         |
|  CalcSecondaryKey                                                          |
|  CharCompare                                                               |
|  Get4Chars                                                                 |
|  WordsTokenize                                                             |
|  UselessFiltering                                                          |
|  WriteTmhToDisk                                                            |
|  WriteToDisk                                                               |
|  FormatMore                                                                |
|  ReadStringFromDisk                                                        |
|  ReadSegFromDisk                                                           |
|  ReadBlock                                                                 |
\*--------------------------------------------------------------------------*/

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#include <eqf.h>                  // General Translation Manager include file

#include <EQFTMI.H>               // Private header file of Translation Memory

/*---------------------------------------------------------------------*\
|                     T M T   (main)                                    |
+-----------------------------------------------------------------------+
|  Function name      : Tmt                                             |
|  Description        : Calls one of the TMT commands.                  |
|  Function Prototype : Tmt (ptmtg, pIn, pOut)                          |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The Tmt function is a switching function.                            |
|  The command name is read from the input buffer.                      |
|  The Tmt input buffer is cast to the input buffer type of the         |
|  corresponding function which is then invoked.                        |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  Same as the rc of any of the invoked functions.                      |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  TmtCreate, TmtOpen, TmtClose, TmtInfo                                |
|  TmtAdd, TmtDelete, TmtReplace, TmtExtract, TmtGet                    |
\*---------------------------------------------------------------------*/
USHORT
Tmt(HTM              htm,                 /* Pointer to TmtGlobals.....*/
    PIN              pIn,                 /* Pointer to input buffer...*/
    POUT             pOut) {              /* Pointer to output buffer..*/
  /*-------------------------------------------------------------------*/
  USHORT   rc;                           /* Returned rc................*/
  PTMT_GLOBALS ptmtg = (PTMT_GLOBALS)htm;/* Pointer to the Tmt Globals.*/
  /*-------------------------------------------------------------------*/

  pOut->fDiskFull = pOut->fDBfull = FALSE;

  /* Get idCommand from input buffer...................................*/
  switch (pIn->idCommand) {
    case (TMC_CREATE):
      rc = TmtCreate ((PCREATE_IN)pIn, (PCREATE_OUT)pOut);
      break;

    case (TMC_OPEN):
      rc = TmtOpen ((POPEN_IN)pIn, (POPEN_OUT)pOut);
      break;

    case (TMC_CLOSE):
      rc = TmtClose (ptmtg, (PCLOSE_OUT)pOut);
      break;

    case (TMC_INFO):
      rc = TmtInfo (ptmtg,  (PINFO_OUT)pOut);
      break;

    case (TMC_ADD):
      rc = TmtAdd (ptmtg, (PADD_IN)pIn, (PADD_OUT)pOut);
      break;

    case (TMC_DELETE):
      rc = TmtDelete (ptmtg, (PDEL_IN)pIn, (PDEL_OUT)pOut);
      break;

    case (TMC_REPLACE):
      rc = TmtReplace (ptmtg, (PREP_IN)pIn, (PREP_OUT)pOut);
      break;

    case (TMC_GET):
      rc = TmtGet (ptmtg, (PGET_IN)pIn, (PGET_OUT)pOut);
      break;

    case ( TMC_EXTRACT):
      rc = TmtExtract (ptmtg, (PEXT_IN)pIn, (PEXT_OUT)pOut);
      break;

    case TMC_DELETE_TM:
      rc = TmtDeleteTM( htm, (PDELTM_IN)pIn, (PDELTM_OUT)pOut );
      break;

    case TMC_GET_PART_OF_TM_FILE:
      rc = TmtGetTMPart( ptmtg, (PGETPART_IN)pIn, (PGETPART_OUT)pOut );
      break;

    case TMC_END_ORGANIZE:
      rc = TmtCloseOrganize( htm, (PENDORG_IN)pIn, (PENDORG_OUT)pOut );
      break;

    default: rc = ILLEGAL_TM_COMMAND;
  } /* End switch (idCommand ) */

  // If the disk full or the DB full flag is on but no other error
  // then set the appropriate return code

  if ( pOut->fDiskFull && !rc )
  {
    rc = DISK_FULL;
  }
  else if ( pOut->fDBfull && !rc )
  {
    rc = DB_FULL;
  } /* endif */
  return(pOut->rcTmt = rc);
} /* End Tmt */

/*---------------------------------------------------------------------*\
|                       Get First Seg Address                           |
+-----------------------------------------------------------------------+
|  Function name      : GetFirstSegAddress                              |
|  Description        : Gets the key and the blocknumber of the first   |
|                       segment in the cluster                          |
|  Function Prototype : GetFirstSegAddress(ptmtg, pszSource, pAddr)     |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function is used several times to find the key and the block    |
|  number of the first segment in a cluster.                            |
|  It performs the following actions:                                   |
|  1. Calls CleanSource to generate the WordsTable and TextTable        |
|  2. Calculates the primary key (4 chars) based on these tables.       |
|  3. Calculates the numerical key that corresponds to this key,        |
|     and assigns it to usEntryInDir field pointed by pAddr.            |
|  4. Assigns to usBlockNumber field pointed by pAddr the corresponding |
|     value taken from KeyDirectory.                                    |
|  5. Assign the value BLOCK_HEADER_SIZE to the displacement field.     |
|     pointed by pAddr.                                                 |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  CleanSource                                                          |
|  CalcPrimaryKey                                                       |
|  CalcEntryInKeyDir                                                    |
\*---------------------------------------------------------------------*/
VOID
GetFirstSegAddress (PTMT_GLOBALS ptmtg,   /* Pointer to Tmt globals....*/
                    PSZ   pszSource,      /* pointer to source segment.*/
                    PTM_ADDRESS  pAddr) { /* pointer to address........*/
  /*-------------------------------------------------------------------*/
  ACHPRIMKEY    achPrimaryKey;              /*Primary key of the source*/
  PTM_HEADER ptmh = ptmtg->pTmHeader;        /* Pointer to TM header...*/
  /*-------------------------------------------------------------------*/

  /* Call CleanSource to form TextTable and WordsTable. The sentence...*/
  /* is unchanged. ....................................................*/
  CleanSource(ptmtg, pszSource);

  /* Calculate 4 chars of primary key (not necessarily letters)........*/
  CalcPrimaryKey(ptmtg, achPrimaryKey);

  /* Calculate the location of achPrimaryKey within the KeyDirectory...*/
  /* table, assign the block number and the pointer displacement.......*/
  pAddr->usEntryInDir = CalcEntryInKeyDir(ptmh, achPrimaryKey);
  pAddr->usBlockNumber = ptmh->ausKeyDirectory[pAddr->usEntryInDir];
  pAddr->usDispBlockPtr = BLOCK_HEADER_SIZE;

} /* end of GetFirstSegAddress */

/*---------------------------------------------------------------------*\
|                              CleanSource                              |
+-----------------------------------------------------------------------+
|  Function name      : CleanSource                                     |
|  Description        : Build parse tables to locate tags and words.    |
|  Function prototype : CleanSource (ptmtg, pszSource)                  |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function is made up of 4 steps:                                 |
|  1. Builds TextTable containing Tags and Text entries, using the      |
|  exclude tag list in the TmHeader.                                    |
|  2. Builds table of tags and words. Each text entry in the previous   |
|  table is split into words by eliminating all non letters chars.      |
|  3. Updates the last table by labeling some words as noise words.     |
|  4. Updates the last table by labeling some words as useless word.    |
|  These two tables provide enough information to build the primary     |
|  key of 4 chars by the function CalcPrimaryKey (as well as the        |
|  secondary key).                                                      |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  EQFTagTokenize                                                       |
|  WordsTokenize                                                        |
|  EQFNoiseFiltering                                                    |
|  UselessFiltering                                                     |
\*---------------------------------------------------------------------*/
VOID
CleanSource (PTMT_GLOBALS ptmtg,  /* Pointer to Tmt globals............*/
             PSZ     pszSource) { /* Pointer to string to be tokenized.*/
   /*------------------------------------------------------------------*/
   PLOADEDTABLE pstExclTagTab;        /* Pointer to loaded ExclTagTable*//*@AT17C*/
   PEXCLUSIONLIST pelExclWordTab;        /* Pointer to ExclWordTable...*/
   PSZ          pszRest;                 /* output of EQFTextTokenize..*/
   USHORT       usLastColPos = 0;        /* output of EQFTextTokenize..*/
   PTM_HEADER   ptmh = ptmtg->pTmHeader; /* Pointer to TM Header.......*/
   /*------------------------------------------------------------------*/
   /* set the exclusion lists from the Tm header.......................*/
   /* set pointer to loaded tag table */                             /*@AT17A*/
   pstExclTagTab = (PLOADEDTABLE)ptmtg->pstLoadedTagTable;           /*@AT17C*/
   pelExclWordTab = (PEXCLUSIONLIST)
                             (((PCHAR) ptmh) + ptmh->usDispExclWordList);

   /* Clean the tags...................................................*/
   /* now using TATagTokenize instead of EqfTagTokenize */           /*@AT17C*/
   TATagTokenize (pszSource, pstExclTagTab, TRUE,                    /*@AT17C*/
                   &pszRest, &usLastColPos,
                   ptmtg->pteTextTable, MAX_TAGS_TAB_ENTRIES, 0L );

   /* TextTable contains now id of TAG and TEXT........................*/
   /* build WordsTable from TextTable..................................*/
   WordsTokenize (ptmtg);

   /* Clean the noise words............................................*/
   EQFNoiseFiltering(pszSource, pelExclWordTab, ptmtg->pteWordsTable);
   UselessFiltering (ptmtg->pteWordsTable);

} /* End of CleanSource */

/*---------------------------------------------------------------------*\
|                    CalcPrimaryKey                                     |
+-----------------------------------------------------------------------+
|  Function name      : CalcPrimaryKey                                  |
|  Description        : Calculates the primary key which is composed    |
|                       of 4 chars.                                     |
|  Function Prototype : CalcPrimaryKey (ptmtg,pchPrimaryKey)            |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function checks what are the tokenids from which the primary    |
|  key can be built.                                                    |
|  If any entry in WordsTable is labeled as WORD then the initials of   |
|  two (or one) words build the key.  If  WORD  not found - but         |
|  USELESSWORD (at least one) exists, then the initials of them build   |
|  the key.  If no USELESSWORD then NOISEWORD are the next candidates.  |
|  If none of these, then TextTable is scanned to decide whether        |
|  TEXT or TAG entries will build the key.                              |
|  A function Get4Chars is called to build the final key.               |
|  It has to decide whether initials of one or two entries are taken,   |
|  and wrap around if there are not enough chars in the entry.          |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  Get4Chars                                                            |
\*---------------------------------------------------------------------*/
VOID
CalcPrimaryKey (PTMT_GLOBALS ptmtg,     /* Pointer to Tmt globals......*/
                PUCHAR pchPrimaryKey){  /* Pointer to the primary key..*/
  /*-------------------------------------------------------------------*/
  USHORT      usCurrentRank = 0;   // Value of current entry in WordTable
  USHORT      usHighestRank = 0;   // Value of highest entry in WordTable
  USHORT      usRc = TRUE;         // Process control switch
  PTOKENENTRY  pteWord = ptmtg->pteWordsTable,
                                           /* Pointer to WordsTable....*/
               pteHighest = pteWord,       /* Pointer to WordsTable....*/
               pteText = ptmtg->pteTextTable; /* Pointer to TextEntry..*/
  /*-------------------------------------------------------------------*/
  // Scan WordsTable to find the "highest" label
  while (pteWord->sTokenid != ENDOFLIST && usRc )
  {
    switch ( pteWord->sTokenid )
    {
      case TA_WORD:
        usCurrentRank = 3 ;
        usRc = FALSE;
        break;
      case USELESSWORD:
        usCurrentRank = 2 ;
        break;
      case NOISEWORD:
        usCurrentRank = 1;
        break;
    } /* End switch */
    if (usCurrentRank > usHighestRank)
    {
      pteHighest = pteWord;
      usHighestRank = usCurrentRank;
    }
    pteWord++;
  } /* Endwhile */

  /* Check if something (not ENDOFLIST) was found. ....................*/
  if (pteHighest->sTokenid != ENDOFLIST)
  {
    /* Get4Chars computes the pchPrimaryKey from entry in WordsTable...*/
    Get4Chars(ptmtg, pteHighest->sTokenid, pteHighest, pchPrimaryKey);
  }
  else
  {
    /* Scan TextTable, to build key get chars from labels of TEXT......*/
    while (pteText->sTokenid != ENDOFLIST &&
           pteText->sTokenid != TEXT_TOKEN) pteText++;

    /* Check if TEXT has been found....................................*/
    if (pteText->sTokenid != ENDOFLIST)
    {
      /* Assign the pchPrimaryKey in Get4Chars.........................*/
      Get4Chars(ptmtg, TEXT_TOKEN, pteText, pchPrimaryKey);
    }
    else // No text has been found build key otherwise
    {
      /*Assign the pchPrimaryKey in Get4Chars..........................*/
      Get4Chars(ptmtg, ptmtg->pteTextTable->sTokenid,
                ptmtg->pteTextTable, pchPrimaryKey);
    }
  } /* Endif pWordsTable not empty.....................................*/
} /* End of CalcPrimaryKey */

/*---------------------------------------------------------------------*\
|                    CalcEntryInKeyDir                                  |
+-----------------------------------------------------------------------+
|  Function name      : CalcEntryInKeyDir                               |
|  Description        : Calculates the entry number in KeyDirectory.    |
|  Function Prototype : CalcEntryInKeyDir (ptmh, achPrimaryKey)         |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  For the duration of the remarks, GN = Group_NUM.                     |
|  This function returns a number which is the entry of achPrimaryKey   |
|  in the KeyDirectory.                                                 |
|  Each character of achPrimaryKey is converted to a digit and these    |
|  4 digits build the number of the entry.                              |
|  A letter of the alphabet is converted according to the DistTable     |
|  For other characters, their ascii code mod(GN) is used as the        |
|  corresponding digit.                                                 |
|  Suppose the digits are x,y,z,v. (If PRIN_KEY_LENGTH == 4).           |
|  The entry is the conversion of the number xyzv into base GN,         |
|  i.e,   the entry = x*GN^3 + y*GN^2 + z*GN + v                        |
|  The pointer to TmHeader is needed to get the distribution tables and |
|  to check whether a character is a letter.                            |
\*---------------------------------------------------------------------*/
USHORT
CalcEntryInKeyDir (PTM_HEADER  ptmh,        /* Pointer to Tm Header....*/
                   PUCHAR pchPrimaryKey) {  /* Primary Key.............*/
  /*-------------------------------------------------------------------*/
  USHORT       usKey = 0,        /* The entry in KeyDirectory..........*/
               usDigit,          /* A Digit corresponding to a char....*/
               usIndex;          /* Index variable.....................*/
  /*-------------------------------------------------------------------*/
  /*Loop to convert pchPrimaryKey[i] into a digit in GROUP_NUM radix...*/
  for (usIndex = 0; usIndex<PRIM_KEY_LENGTH; usIndex++) {
    usDigit = ptmh->abABGrouping[pchPrimaryKey[usIndex]];

    if (usDigit == GROUP_NUM) {
      /* If the character is not in the alphabet, its value will be....*/
      /* the ASCII code modulu GROUP_NUM. This will hopefully result...*/
      /* in a uniform distribution of non-alphabet characters..........*/
      usDigit = ((USHORT)pchPrimaryKey[usIndex]) % GROUP_NUM;

    } /* Endif pchPrimaryKey is a letter */

    /* Calculate the entry in KeyDirectory using Horner's method.......*/
    usKey = (usKey * GROUP_NUM) + usDigit;
  } /* End for Loop */

  return(usKey);
} /* End of CalcEntryInKeyDir */

/*---------------------------------------------------------------------*\
|                    CalcSecondaryKey                                   |
+-----------------------------------------------------------------------+
|  Function name      : CalcSecondaryKey                                |
|  Description        : Calculates the secondary key (initials of the   |
|                       proper words of the sentence)                   |
|  Function Prototype : CalcSecondaryKey (ptmtg, pszSortedSecKey)       |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function calculates the secondary key and sorts it.  The        |
|  WordsTable is scanned and each entry tokenized as WORD, NOISEWORD or |
|  USELESSWORD lower-cased and added to an internal string - szSecKey.  |
|  Lastly, a sort routine - QSort, is called and assignes the pointer to|
|  the sorted key to pszSortedSecKey.                                   |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  qsort                                                                |
\*---------------------------------------------------------------------*/
VOID
CalcSecondaryKey (PTMT_GLOBALS ptmtg,     /* Pointer to Tmt globals....*/
                  PSZ  pszSecKey) {       /* Pointer to sorted sec key.*/
  /*-------------------------------------------------------------------*/
  USHORT      cteEntries = 0;        /* Counts entries in WordsTable...*/
  PTOKENENTRY pteWord = ptmtg->pteWordsTable;
                                           /* Pointer to WordsTable....*/
  SHORT       sTokenId;
  /*-------------------------------------------------------------------*/
  /* Take first char from each entry in WordsTable which is WORD, .....*/
  /* NOISEWORD or USELESSWORD .........................................*/
  while ((cteEntries < MAX_SEC_LENGTH) &&
         (pteWord->sTokenid != ENDOFLIST)) {
    sTokenId = pteWord->sTokenid;
    if ((sTokenId == TA_WORD) || (sTokenId == NOISEWORD) ||
        (sTokenId == USELESSWORD)) {
      /* It is qualified Append first character of entry to pszSecKey..*/
      pszSecKey[cteEntries++] = *(pteWord->pDataString);
    }
    pteWord++;
  } /* end while */
  pszSecKey[cteEntries] = NULC;

  if (cteEntries > 0) {
    /* The secondary key is not empty, sort it.........................*/
    _strlwr (pszSecKey);
    qsort ((PVOID) pszSecKey,
           (size_t) cteEntries,
           sizeof(CHAR),
           CharCompare);
  }
} /* end of function CalcSecondaryKey */

/*---------------------------------------------------------------------*\
|                            CharCompare                                |
+-----------------------------------------------------------------------+
|  Function name      : CharCompare                                     |
|  Description        : Compares two elements for use by qsort function |
|                       of the C librery routine                        |
|  Function Prototype : CharCompare (arg1, arg2 )                       |
+-----------------------------------------------------------------------|
|  Implementation remarks                                               |
|  ----------------------                                               |
| This function compares two characters and return zero if the are equal|
|  , a positive number if arg1 > arg2 , negative number if arg1 < arg2  |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
\*---------------------------------------------------------------------*/
INT
CharCompare (const void * arg1,   /* First comparand ..................*/
             const void * arg2) { /* Second comparand .................*/
  /*-------------------------------------------------------------------*/
  return (*((PUCHAR)arg1) - *((PUCHAR)arg2));
}/* end of CharCompare */

/*---------------------------------------------------------------------*\
|                         Get4Chars                                     |
+-----------------------------------------------------------------------+
|  Function name      : Get4Chars                                       |
|  Description        : Get 4 chars from initials of words or tags to   |
|                       build the primary key.                          |
|  Function Prototype : Get4Chars (ptmtg, idToken, pteFirst,            |
|                                  pchPrimaryKey)                       |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The function checks the input table which is either TextTable of     |
|  WordsTable, starting from location pointed by pteFirst.  It looks    |
|  for another entry with the same tokenid as the first parameter       |
|  idToken.  If only one item of this label exists in the table, then   |
|  4 chars are taken from this item.  If this item has less than 4      |
|  chars it is wrapped around to get 4 chars.  If two items are found,  |
|  then from each item two initials are taken.  Again, wrap-around is   |
|  used if the item has less then 2 chars.  pchPrimaryKey is finally    |
|  assigned these 4 chars.                                              |
|  In addition to the above this function updates two pointers in       |
|  Tmt globlas which point to the first & second(if exist) significant. |
|  x'0D' should never be part of the primary key for compatibility      |
|  purposes with editors either working with x'0D0A' or x'0A' as CRLF.  |
\*---------------------------------------------------------------------*/
VOID
Get4Chars (PTMT_GLOBALS ptmtg,       /* Pointer to Tmt globals.........*/
           SHORT        sTokenId,    /* WORD/USELESS/NOISE/TEXT/TAG....*/
           PTOKENENTRY  pteFirst,    /* Ptr to Text or Word Table......*/
           PUCHAR  pchPrimaryKey) {  /* 4 chars for primary key........*/
  /*-------------------------------------------------------------------*/
  USHORT        usMaxLoop  = 1000;   // Used to stop loop if a '0D'
                                     // character is present only
  USHORT        usCharToBeFound;     // Number of character for primary key
  USHORT        usCharProcessed = 0; // Number of characters processed
  USHORT        usCharFound;         // Number of valid characters found
  PUCHAR        pchData;             // points to string in segment
  PTOKENENTRY  pteSecond = pteFirst + 1; /* Pointer to second token....*/
  /*-------------------------------------------------------------------*/
  /* Get4Chars from 2 (or 1) entries in Table which are labeled as.....*/
  /* sTokenId..........................................................*/

  /* Update the pointer to first significant word in TmtGlobals........*/
  /* if second significant word does not exist it will point to the first
     one ..............................................................*/
  ptmtg->pteFirstSigWord = pteFirst ;
  ptmtg->pteSecondSigWord = pteFirst ;

  /* Search in Table starting after pteFirst to find a second..........*/
  /* entry with same tokenid. The search can also end if the list......*/
  /* ended.............................................................*/
  for (; ((pteSecond->sTokenid != ENDOFLIST) &&
          (pteSecond->sTokenid != sTokenId)); pteSecond++);

  /* Set a pointer to the beginning of the Data string.................*/
  pchData = (PUCHAR)(pteFirst->pDataString);

  // Initialize the primary key in case the only valid
  // character is '\r'
  for ( usCharFound = 0; usCharFound < PRIM_KEY_LENGTH; usCharFound++ )
  {
    pchPrimaryKey[usCharFound]= '\r';
  } /* endfor */

  // Reinitialize usCharFound
  usCharFound = 0;

  if (pteSecond->sTokenid == sTokenId)
  {
    // Another entry with same token id was found
    // Update the pointer to the second significant word in TmtGlobals
    ptmtg->pteSecondSigWord = pteSecond ;

    // Assign the first two bytes of the primary key from first entry
    // but skip '\r' characters
    usCharToBeFound = PRIM_KEY_LENGTH / 2;
    while ( usCharFound < usCharToBeFound && usMaxLoop )
    {
      if ( *(pchData + usCharProcessed) != '\r' )
      {
        pchPrimaryKey[usCharFound] = *(pchData + usCharProcessed);
        usCharFound++;
      } /* endif */
      usCharProcessed++;
      if ( usCharProcessed == pteFirst->usLength )
      {
        usCharProcessed = 0;
        usMaxLoop--;
      } /* endif */
    } /* endwhile */

    // Assign the last two bytes of the primary key from second entry
    // but skip '\r' characters
    pchData         = (PUCHAR)(pteSecond->pDataString);
    usCharToBeFound = PRIM_KEY_LENGTH;
    usCharProcessed = 0;
    while ( usCharFound < usCharToBeFound && usMaxLoop )
    {
      if ( *(pchData + usCharProcessed) != '\r' )
      {
        pchPrimaryKey[usCharFound] = *(pchData + usCharProcessed);
        usCharFound++;
      } /* endif */
      usCharProcessed++;
      if ( usCharProcessed == pteSecond->usLength )
      {
        usCharProcessed = 0;
        usMaxLoop--;
      } /* endif */
    } /* endwhile */
  }
  // Only one tokenid exists
  else
  {
    // Set pchPrimaryKey as 4 chars from the first entry copying last
    // character if neccessary
    usCharToBeFound = PRIM_KEY_LENGTH;
    while ( usCharFound < usCharToBeFound && usMaxLoop )
    {
      if ( *(pchData + usCharProcessed) != '\r' )
      {
        pchPrimaryKey[usCharFound] = *(pchData + usCharProcessed);
        usCharFound++;
      } /* endif */
      usCharProcessed++;
      if ( usCharProcessed == pteFirst->usLength )
      {
        usCharProcessed = 0;
        usMaxLoop--;
      } /* endif */
    } /* endwhile */
  } /* Endif two entries exist */

  // Lower case the primary key
  for( usCharFound = 0;  usCharFound < PRIM_KEY_LENGTH; usCharFound++)
    pchPrimaryKey[usCharFound] =
                          (UCHAR) tolower((int)(pchPrimaryKey[usCharFound]));
} /* End of Get4Chars */

/*---------------------------------------------------------------------*\
|                             WordsTokenize                             |
+-----------------------------------------------------------------------+
|  Function name      : WordsTokenize                                   |
|  Description        : Split each text entry into words.               |
|  Function prototype : WordsTokenize (ptmtg)                           |
+-----------------------------------------------------------------------|
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function scans the entries in TextTable and those that are      |
|  labeled as TEXT are split into words.  A word is a string consisting |
|  of the letters of the alphabet of the language.  (The alphabet list  |
|  is a part of the TM header.)  Each char that is not a letter is      |
|  discarded.  Here are some examples of strings and the result of the  |
|  tokenization.                                                        |
|  1. "here" --> here                                                   |
|  2. don't  --> don t (split to 2 words)                               |
|  3. I.B.M  --> I B M (split into 3 words).                            |
|  4.   ---Good Day ---    results in 2 words :   Good Day              |
|  5.   -----      this string  discarded completely and thus does not  |
|     add any entry to the WordsTable. (However it is tokenized as      |
|     a TEXT in the TextTable.                                          |
\*---------------------------------------------------------------------*/
VOID
WordsTokenize (PTMT_GLOBALS  ptmtg) { /* Pointer to Tmt globals........*/
  /*-------------------------------------------------------------------*/
   PUCHAR      pchBegWord,      /* Points to beginning of word.........*/
              pchLastLocation,  /* Points to last char of text segment.*/
              pchLocation;      /* Points to each char of text.........*/
   PBYTE      pabABGrouping = ptmtg->pTmHeader->abABGrouping;
                                 /* The abABGrouping array.............*/
   PTOKENENTRY pteWord = ptmtg->pteWordsTable,  /* Ptr to WordsTable...*/
               pteText = ptmtg->pteTextTable;   /* Ptr to TextTable....*/
   USHORT   usEntryCounter = 1, /* Counting entries in WordsTable......*/
            usLenWord;          /* Length of a word....................*/
   /*------------------------------------------------------------------*/
   /*     Split each text entry in TextTable into words................*/
   while (pteText->sTokenid != ENDOFLIST &&
           usEntryCounter < MAX_WORDS_TAB_ENTRIES) {
     /* if sTokenId is TEXT, split it into words. Otherwise nothing)...*/

     if (pteText->sTokenid == TEXT_TOKEN) {
       /* Set pchLocation to point to data string in TextTable.........*/
       pchLocation = (PUCHAR)(pteText->pDataString);

       /*Set pchLastLocation to pchLocation + length of this text field*/
       pchLastLocation = pchLocation + pteText->usLength;

       /*    Start splitting loop......................................*/
       while ( pchLocation < pchLastLocation ) {
         /*      Search to find begining of a word.....................*/
         /*      Letter of the alphabet is taken from TM header........*/
         while (pabABGrouping[*pchLocation] == GROUP_NUM &&
                pchLocation < pchLastLocation) pchLocation++;

         /*    pchLocation is the beginning of the word................*/
         pchBegWord = pchLocation;

         /* Search to find the end of the word.........................*/
         while (pabABGrouping[*pchLocation] < GROUP_NUM &&
                pchLocation < pchLastLocation) pchLocation++;

         /*    pchLocation now points to the end of the word...........*/
         usLenWord = (USHORT)(pchLocation - pchBegWord) ;
         if (usLenWord > 0) {
           /* Write new entry in WordsTable............................*/
           pteWord->sTokenid = TA_WORD ;
           pteWord->usLength = usLenWord ;
           pteWord->pDataString = (CHAR * )pchBegWord ;

           /* Advance variables........................................*/
           pteWord++;
           usEntryCounter++;
         }/* Endif (usLenWord > 0) */
       } /* End while (pchLocation < pchLastLocation) */
     } /* Endif (sTokenId == TEXT) */
     /* Advance pTextTable to next entry...............................*/
     pteText++;

   } /*  End while ((sTokenId  <> ENDOFLIST) and... */
   /* Write ENDOFLIST token............................................*/
   pteWord->sTokenid = ENDOFLIST ;

} /* End of WordsTokenize */

/*---------------------------------------------------------------------*\
|                           UselessFiltering                            |
+-----------------------------------------------------------------------+
|  Function name      : UselessFiltering                                |
|  Description        : Changes WordsTable such that entries which are  |
|                       useless are tokenized USELESSWORD.              |
|  Function Prototype : UselessFiltering (PTOKENENTRY pteWord)          |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  as of 5/8/90 this function is an empty one which does nothing until  |
|  further descision will be made                                       |
+-----------------------------------------------------------------------+
|  This function scans the entries in WordsTable and labels the useless |
|  words as USELESSWORD. Currently useless words are those which consist|
|  of only capital letters, but other definition of USELESSWORD can be  |
|  added later.                                                         |
\*---------------------------------------------------------------------*/
VOID
UselessFiltering (PTOKENENTRY  pteWord) { /* Ptr to word in WordsTable.*/
  /*-------------------------------------------------------------------*/
  PUCHAR     pchWord,            /* points to each word in the table...*/
             pchEndOfWord;
  /*-------------------------------------------------------------------*/
  for (;pteWord->sTokenid != ENDOFLIST ; pteWord++) {
    pchEndOfWord = (PUCHAR)(pteWord->pDataString + pteWord->usLength);
                         /* points to a character past the word*/
    /* Loop as long as not end of word and the character is an upper case*/
    for (pchWord = (PUCHAR)(pteWord->pDataString);
         pchWord != pchEndOfWord && isupper (*pchWord);
         pchWord++);

    /* if all characters are upper case then mark the word as USELESSWORD*/
    if (pchWord == pchEndOfWord) pteWord->sTokenid = USELESSWORD;
  }
  pteWord = pteWord;
} /* End of UselessFiltering */

/*---------------------------------------------------------------------*\
|                           Write Tmh To Disk                           |
+-----------------------------------------------------------------------+
|  Function name      : WriteTmhToDisk                                  |
|  Description        : Builds the AB grouping array                    |
|  Function Prototype : WriteTmhToDisk(ptmtg, fsWrite)                  |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  Based on the flags passed in fsWrite, the function will write the    |
|  appropriate fields of the TM Header to disk.                         |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - An API call failed.                                   |
|                                                                       |
|  Function Calls:                                                      |
|  ---------------                                                      |
|  WriteToDisk                                                          |
\*---------------------------------------------------------------------*/
USHORT
WriteTmhToDisk ( PTMT_GLOBALS ptmtg,   /* Pointer to globals...........*/
                 USHORT       fsWrite){/* Write control flags word.....*/
  /*-------------------------------------------------------------------*/
  USHORT  rc;                        /* Returned Rc....................*/
  PTM_HEADER ptmh;                   /* Pointer to the TM header.......*/
  /*-------------------------------------------------------------------*/
  /* Assign the ptmh...................................................*/
  ptmh = ptmtg->pTmHeader;
  if (fsWrite == WF_ALL) {
    /* Write the Entire TM header to disk..............................*/
    rc = WriteToDisk(ptmtg, 0L, (PVOID)ptmh, ptmh->usTmHeaderSize);
  }
  else /* Check other flags */ {
    /* Initialize the RC...............................................*/
    rc = OK;
    if (fsWrite & WF_KEYDIR) {
      /* Write the key directory entry.................................*/
      rc = WriteToDisk(ptmtg,
                       FPTR(TM_HEADER, ausKeyDirectory[0]),
                       (PVOID)(ptmh->ausKeyDirectory),
                       KEY_DIR_SIZE * sizeof(USHORT));
    }
    if ((fsWrite & WF_UPDATE) && (rc == OK)) {
      /* Write the key directory entry.................................*/
      rc = WriteToDisk(ptmtg,
                       FPTR(TM_HEADER, fCorruption),
                       (PVOID)&(ptmh->fCorruption),
                       sizeof(TM_HEADER_UPDATE));
    } else {
      if ((fsWrite & WF_CORRUPT) && (rc == OK)) {
        /* Write the corruption flag.....................................*/
        rc = WriteToDisk(ptmtg,
                         FPTR(TM_HEADER, fCorruption),
                         (PVOID)&(ptmh->fCorruption),
                         sizeof (ptmh->fCorruption));
      }
      if ((fsWrite & WF_TMBLOCKS) && (rc == OK)) {
        /* Write the number of TM blocks.................................*/
        rc = WriteToDisk(ptmtg,
                         FPTR(TM_HEADER, usNumTMBlocks),
                         (PVOID)&(ptmh->usNumTMBlocks),
                         sizeof (ptmh->usNumTMBlocks));
      }
      if ((fsWrite & WF_FREEBLOCKS) && (rc == OK)) {
        /* Write the number of free blocks...............................*/
        rc = WriteToDisk(ptmtg,
                         FPTR(TM_HEADER, usNumFreeBlocks),
                         (PVOID)&(ptmh->usNumFreeBlocks),
                         sizeof (ptmh->usNumFreeBlocks));
      }
      if ((fsWrite & WF_FIRSTAVAIL) && (rc == OK)) {
        /* Write the number of the first available block.................*/
        rc = WriteToDisk(ptmtg,
                         FPTR(TM_HEADER, usFirstAvailBlock),
                         (PVOID)&(ptmh->usFirstAvailBlock),
                         sizeof (ptmh->usFirstAvailBlock));
      }
    }
  } /* Endelse other flags */

  //--- assure the TM header is really written to Disk
  if ( rc == OK )                                                    /* 2@1CA */
  {
     rc = UtlBufReset( ptmtg->hfTM, FALSE );
  } /* endif */

  return (rc);
} /* End of WriteTmhToDisk */

/*---------------------------------------------------------------------*\
|                            Write to Disk                              |
+-----------------------------------------------------------------------+
|  Function name      : WriteToDisk                                     |
|  Description        : Builds the AB grouping array                    |
|  Function Prototype : WriteToDisk(ptmtg, ldispPtr, pvWrite, numWrite) |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The function tries to set the file pointer to ldispPtr and then      |
|  writes numWrite bytes from pvWrite to the file. The file handle      |
|  is in the TMT Globals structure (*ptmtg).                            |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - An API call failed.                                   |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
|  UtlWrite                                                             |
|  UtlChgFilePtr                                                        |
\*---------------------------------------------------------------------*/
USHORT
WriteToDisk (PTMT_GLOBALS ptmtg,     /* TMT globals....................*/
             ULONG    ldispPtr,      /* New file pointer location......*/
             PVOID    pvWrite,       /* Pointer to write buffer .......*/
             USHORT   usNumWrite) {  /* Number of bytes to write.......*/
  /*-------------------------------------------------------------------*/
  USHORT  rc,                        /* Returned Rc....................*/
          usBytesWritten;            /* Number of bytes written........*/
  ULONG   ulFilePtr;                 /* New file pointer...............*/
  /*-------------------------------------------------------------------*/
  /* Conditionally locate the file pointer.............................*/
  rc = UtlChgFilePtr(ptmtg->hfTM,        /* File Handle................*/
                     ldispPtr,           /* Displacement...............*/
                     FILE_BEGIN,         /* Where from.................*/
                     &ulFilePtr,         /* New location (Out).........*/
                     FALSE);
  if (rc == OK) {
    /* The file pointer was relocated, now write.......................*/
    rc = UtlWrite(ptmtg->hfTM,           /* File Handle................*/
                  pvWrite,               /* Write Buffer...............*/
                  usNumWrite,            /* Number of bytes to write...*/
                  &usBytesWritten,       /* Number written (out).......*/
                  FALSE);
  }
  return (rc);
} /* End of WriteToDisk */

/*---------------------------------------------------------------------*\
|                       Format More                                     |
+-----------------------------------------------------------------------+
|  Function name      : FormatMore                                      |
|  Description        : Formats more blocks and add them into the TM.   |
|  Function Prototype : FormatMore (ptmtg)                              |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  This function formats more blocks and ensures that each Add          |
|  or Replace command will not fail because of Disk Full or Db Full     |
|  situation.                                                           |
|  There are 2 flags fields in PREFIX_OUT stuctre which are set to      |
|  TRUE if needed.                                                      |
|  If the spool of preformatted blocks (as indicated by a variable in   |
|  TM header numFreeBlocks) is too small, some more blocks              |
|  are formatted. The variable numTMBlocks, numFreeBlocks are           |
|  changed and thus Tm header has to be updated also on disk.           |
|  If the number of blocks to be formatted, will set pOut.fDBfull to    |
|  TRUE, The following action is taken:                                 |
|  1. Number of blocks added to the TM is such that the total number of |
|     blocks is MAX_BLOCKS_NUM. (This avoids situation of formatting    |
|     blocks that will not be ever used).                               |
|  2. A check is made if the number of TM blocks is already             |
|     MAX_BLOCKS_NUM, and if so fDBfull = TRUE   , else the file is     |
|     opened with the new size, resulting in OK, or fDiskFull & DosRc.  |
|  If the number of blocks to be formatted, will not set fDBfull = TRUE |
|  then possible rc are OK, or fDiskFull & DosRc (failure of Dos        |
|  operation).                                                          |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)       - The command ended successfully.                       |
|  Other values - A Dos operation like write/close failed.              |
|                                                                       |
|  Function Calls:                                                      |
|  ---------------                                                      |
|  WriteTmhToDisk                                                       |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
|  DosNewSize                                                           |
\*---------------------------------------------------------------------*/
USHORT
FormatMore (PTMT_GLOBALS ptmtg) {     /* Pointer to Tmt globals........*/
  /*-------------------------------------------------------------------*/
  PTM_HEADER ptmh = ptmtg->pTmHeader; /* Pointer to the TM Header......*/
  USHORT     rc = 0;                  /* Returned rc...................*/
  USHORT     usNumTMBlocks =          /* Number of blocks in TM........*/
                          ptmh->usNumTMBlocks,
             usBlockSize =            /* The size of each TM block.....*/
                          ptmh->ctmh.usBlockSize,
             usNumNewBlocks =         /* The number of new blocks......*/
                          MIN_SPOOL_SIZE / usBlockSize;
  BOOL       fDBFull;                 /* Could not format enough flag..*/
  /*-------------------------------------------------------------------*/
  /* check if it possible to format numNewBlocks.......................*/
  fDBFull = (usNumNewBlocks >= (MAX_BLOCKS_NUM  - usNumTMBlocks));
  if (fDBFull) usNumNewBlocks = MAX_BLOCKS_NUM - usNumTMBlocks;

  if (usNumNewBlocks == 0) {
     ptmtg->pPrefixOut->fDBfull = TRUE;
  } else /* (usNumNewBlocks > 0) */ {
    usNumTMBlocks = usNumTMBlocks + usNumNewBlocks;

    /* re-size Tm file.................................................*/
    rc = DosNewSize(ptmtg->hfTM,                         /* Handle.....*/
                    (ULONG)((ULONG)usNumTMBlocks * (ULONG)usBlockSize) +
                     ptmh->ldispFirstBlock);             /* New size...*/
  }

  switch (rc) {
     case ERROR_DISK_FULL:
       ptmtg->pPrefixOut->fDiskFull = TRUE;
       break;
     case OK:
       ptmh->usNumTMBlocks = usNumTMBlocks;
       ptmh->usNumFreeBlocks = ptmh->usNumFreeBlocks + usNumNewBlocks;
       rc = WriteTmhToDisk(ptmtg, WF_FREEBLOCKS | WF_TMBLOCKS);
       if (fDBFull) ptmtg->pPrefixOut->fDBfull = TRUE;
       break;
     default:;
  } /* End switch */

  return (rc) ;
} /* End of FormatMore */

/*---------------------------------------------------------------------*\
|                       ReadStringFromDisk                              |
+-----------------------------------------------------------------------+
|  Function name      : ReadStringFromDisk                              |
|  Description        : Reads a variable sized string from the disk     |
|                       according to a length count.                    |
|  Function Prototype : ReadStringFromDisk (ptmtg, bufRead, lenRead,    |
|                                           pAddr)                      |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  The string to be read may span over more than one block.             |
|  Current block image is a TM block that has been initialized by the   |
|  calling function.                                                    |
|                                                                       |
|  The function receives the following parameters:                      |
|  1. ptmtg - A pointer to TMT_GLOBALS structure to access BlockImage   |
|  2. bufRead - A pointer to the buffer into which the string is read   |
|  3. lenRead - The number of bytes to read.                            |
|  4. pAddr  - pointer to the address from which reading should start.  |
|  This address is updated by the function and at exit it points to the |
|  location just after the last byte read.                              |
|                                                                       |
|  The function returns the return code in rc.                          |
|  Rc other than OK causes immediate exit from the function.            |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)        - The command ended successfully.                      |
|  Other values  - A Dos operation like write/close failed.             |
|                  The returned value is DosRc.                         |
|  CLUSTER_EMPTY -                                                      |
|  This function calls:                                                 |
|  --------------------                                                 |
|  ReadBlock                                                            |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
|  UtlRead                                                              |
\*---------------------------------------------------------------------*/
USHORT
ReadStringFromDisk (PTMT_GLOBALS ptmtg,      /* pointer to TMT GLOBALS.*/
                    PCHAR       bufRead,     /* Read buffer............*/
                    USHORT       usLenRead,  /* String length..........*/
                    PTM_ADDRESS  pAddr) {    /* Where from.............*/
  /*-------------------------------------------------------------------*/
  USHORT    rc = OK,                 /* Rc returned by this function...*/
            usBytesToRead,           /* Number of bytes to read........*/
            usBlockSize = ptmtg->pTmHeader->ctmh.usBlockSize;
                                     /* block size.....................*/
  PUCHAR    pchBlockImage = ptmtg->pchBlockImage;
                                          /* block image...............*/
  PBLOCK_HEADER  pbh = (PBLOCK_HEADER) pchBlockImage;
                                    /* pointer to block header ........*/
  /*-------------------------------------------------------------------*/
  /* Loop as long as there are more bytes to read and there are no.....*/
  /* problems in reading. .............................................*/
  while ((rc == OK) && (usLenRead > 0)) {
    /* Check if the current block is full..............................*/
    if (pAddr->usDispBlockPtr == usBlockSize)  {
      /* Read the next block using the pointer to the next block.......*/
      rc = ReadBlock (ptmtg, pAddr, FALSE);

      if (rc == OK) {
        pAddr->usDispBlockPtr = BLOCK_HEADER_SIZE;
        pAddr->usBlockNumber =((PBLOCK_HEADER)pchBlockImage)->usBlockNum;
      }
    } /* Endif pAddr->usDispBlockPtr == usBlockSize */

    /* If everything is OK, set and update various values..............*/
    if (rc == OK) {
      /* if segment is NOT physicly deleted read it otherwise return ..*/
      /* CLUSTER_EMPTY ................................................*/
      if (pAddr->usDispBlockPtr < pbh->usFirstAvailByte) {
        /*Calculate the number of bytes to be read from the block image*/
        usBytesToRead=min(usBlockSize-pAddr->usDispBlockPtr, usLenRead);

        /* Move usBytesToRead bytes from pchBlockImage to bufRead */
        memcpy( bufRead, pchBlockImage + pAddr->usDispBlockPtr,
                   usBytesToRead);

        bufRead += usBytesToRead;
        usLenRead = usLenRead - usBytesToRead;
        pAddr->usDispBlockPtr = pAddr->usDispBlockPtr + usBytesToRead;
      }
      else {
        /* no bytes to read beyond dispBlockPtr may happen in case of .*/
        /* physical delete and it means that the cluster is empty .....*/
        /* beyond the input address....................................*/
        rc = CLUSTER_EMPTY ;
      }/* end if (pAddr->usDispBlockPtr < pbh->usFirstAvailByte ) .....*/
    } /* Endif rc == OK */
  } /* End while */

  return (rc) ;
} /* End of ReadStringFromDisk */

/*---------------------------------------------------------------------*\
|                          ReadSegFromDisk                              |
+-----------------------------------------------------------------------+
|  Function name      : ReadSegFromDisk                                 |
|  Description        : Reads a segment from the TM file                |
|  Function Prototype : ReadSegFromDisk (ptmtg, pAddr, pSegment,        |
|                                        pfFirstSeg, pfLastSeg)         |
+-----------------------------------------------------------------------+
| Implementation remarks                                                |
| ----------------------                                                |
| ReadSegFromDisk reads a segment from the disk to a segment buffer     |
| pointed by pSegment. The segment address is given as input (pAddr)    |
| and a flag fFirstSeg indicates whether the block in which the segment |
| starts is already in the BlockImage in memory.                        |
|                                                                       |
| The segment is read by two calls to ReadStringFromDisk. The first     |
| call reads a constant number of bytes SEG_LENGTH_DISP which includes  |
| the segment marker + total length of the segment. The second call     |
| uses that length to read the rest of the segment. The function has an |
| output flag fLastSeg to indicate that last segment has been reached.  |
|                                                                       |
| Upon Exit pAddr points to next address (maybe logically deleted).     |
| pAddr->dispBlockPtr always points to a legal address, and the only    |
| case that it has a value of BlockSize is if this segment is the last  |
| one in the cluster.                                                   |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)        - The command ended successfully.                      |
|  CLUSTER EMPTY - There are no blocks in the cluster.                  |
|  Other values  - A Dos operation like write/close failed.             |
|                                                                       |
|  Function calls:                                                      |
|  ---------------                                                      |
|  ReadStringFromDisk                                                   |
|  ReadBlock                                                            |
|  WriteBlock                                                           |
\*---------------------------------------------------------------------*/
USHORT
ReadSegmentFromDisk (PTMT_GLOBALS ptmtg,      /* pointer to Tmt globals*/
                     PTM_ADDRESS  pAddr,      /* Pointer to an address */
                     PSEGMENT     pSegment,   /* Ptr to segment buffer */
                     PBOOL        pfFirstSeg, /* Ptr to 1st seg. flag .*/
                     PBOOL        pfLastSeg ) { /* Ptr to last sg. flag*/
  /*-------------------------------------------------------------------*/
  USHORT    rc = OK,                /* returned rc ....................*/
            usLenString,            /* length of string to read .......*/
            usBlockSize = ptmtg->pTmHeader->ctmh.usBlockSize;
                                    /* block size .....................*/
  PBLOCK_HEADER  pbh = (PBLOCK_HEADER) (ptmtg->pchBlockImage);
                                    /* pointer to block header ........*/
  /*-------------------------------------------------------------------*/

  /* If it is the 1st segment in cluster, the first block from cluster */
  if (*pfFirstSeg) {
    rc = ReadBlock (ptmtg, pAddr, TRUE)  ;
    *pfFirstSeg = FALSE;
  }

  /* If FirstSeg was FALSE, pchBlockImage was already loaded ..........*/
  /* If the read of the first block succeeded, proceed ................*/
  if (rc == OK) {
    /* The first call to ReadString provides the length of the ........*/
    /* segment.  It reads SEG_LENGTH_DISP bytes, the first ones are ...*/
    /* the segment marker, the last two are the length ................*/
    /* pAddr is updated on exit from ReadString .......................*/
    rc = ReadStringFromDisk (ptmtg,
                             (PCHAR)pSegment,
                             SEG_LENGTH_DISP, pAddr);

    // Check if the segment begins with the segment marker.
    // If not then the cluster has been corrupted during a system
    // break down. In that case set return code to CLUSTER_EMPTY
    if (rc == OK)
    {
      if ( memcmp( pSegment->achSegMarker, SEGMARKER, SEG_MARKER_LENGTH )/*@22C*/
           || pSegment->usLenSegment > sizeof(SEGMENT) )                 /*@22A*/
      {
        // pSegment points to an invalid segment set rc to CLUSTER_EMPTY
        rc = CLUSTER_EMPTY;
      } /* endif */
    } /* endif */

    if (rc == OK)
    {
      /* The first ReadString succeeded ...............................*/
      /* The second call to ReadString reads the rest of the segment ..*/
      /* into pSegment ................................................*/
      PUCHAR pSegTemp;

      usLenString = pSegment->usLenSegment - SEG_LENGTH_DISP;
      pSegTemp = (PUCHAR)pSegment + SEG_LENGTH_DISP;

      rc = ReadStringFromDisk (ptmtg, (PCHAR)pSegTemp, usLenString, pAddr);

      // adjust segment as the segment structure has been changed:
      //    There is now a long file name field following the
      //    document name. The external TM in V1.0 format has no
      //    such field
      {
        USHORT usOffsLongName = (USHORT)((PUCHAR)&(pSegment->szLongName) - (PUCHAR)pSegment);
        memmove( &(pSegment->usSegNumber), &(pSegment->szLongName),
                pSegment->usLenSegment - usOffsLongName );
        pSegment->szLongName[0] = EOS;
        pSegment->usLenSegment += sizeof(pSegment->szLongName);
      }

      /* pAddr points now to next valid segment. If dispBlockPtr == ...*/
      /*  Blocksize it means that no more blocks are chained ..........*/
      if (rc == OK) {
        /* Second ReadString succeeded ................................*/
        /* Check if last segement of cluster has been reached .........*/
        *pfLastSeg = ((pAddr->usDispBlockPtr == usBlockSize ) &&
                      (pbh->usNextBlock == 0)) ||
                     ((pAddr->usDispBlockPtr != usBlockSize ) &&
                      (pAddr->usDispBlockPtr == pbh->usFirstAvailByte)) ;
      } /* Endif (Second ReadString succeeded) */
    } /* Endif (First ReadString succeeded) */
  } /* Endif (First-block read succeeded) */

  if (rc == CLUSTER_EMPTY) *pfLastSeg = TRUE;
  return (rc) ;
} /* End of ReadSegFromDisk */

/*---------------------------------------------------------------------*\
|                            ReadBlock                                  |
+-----------------------------------------------------------------------+
|  Function name      : ReadBlock                                       |
|  Description        : Reads a block image from the TM on the disk.    |
|  Function Prototype : ReadBlock (ptmtg, pAddr, fAddressInParameter)   |
+-----------------------------------------------------------------------+
|  Implementation remarks                                               |
|  ----------------------                                               |
|  ReadBlock reads a block from the disk into the block image. The      |
|  address of that block is an input variable. In most cases where      |
|  ReadBlock is applied, the number of the block to be read appears     |
|  within the current block image in the next_block_pointer field, and  |
|  the input address is not used.                                       |
|  However, if the block to be read is not in BlockImage, the           |
|  block number is taken from the input address. An input flag          |
|  (fAddressInParameter) indicates whether the address of the block to  |
|  read is taken from pAddr or from BlockImage.                         |
|                                                                       |
|  Return Codes:                                                        |
|  -------------                                                        |
|  OK (0)        - The command ended successfully.                      |
|  CLUSTER_EMPTY - There are no blocks in the cluster.                  |
|  Other values  - A Dos operation like write/close failed.             |
|                                                                       |
|  API calls:                                                           |
|  ----------                                                           |
|  UtlRead                                                              |
\*---------------------------------------------------------------------*/
USHORT
ReadBlock (PTMT_GLOBALS  ptmtg,    /* Pointer to TMT_GLOBALS structure.*/
           PTM_ADDRESS   pAddr,    /* ptr to address of read block.....*/
           BOOL fAddressInParameter)   /* Address in pAddr flag........*/
{
  /*-------------------------------------------------------------------*/
  USHORT   rc;                    /* Returned rc.......................*/
  USHORT    usBytesRead = 0;       /* Temporary variable................*/
  ULONG    ulBlockNumber;         /* Block number......................*/
  USHORT   usBlockSize = ptmtg->pTmHeader->ctmh.usBlockSize;
                                  /* block size........................*/
  ULONG     uldispBlock;          /* displacement......................*/
  PUCHAR    pchBlockImage = ptmtg->pchBlockImage;
                                  /* block image.......................*/
  /*-------------------------------------------------------------------*/
  /* The block number is either from the pAddr passed as a parameter...*/
  /* or from the next block field in the block header..................*/
  ulBlockNumber = (fAddressInParameter) ?
                            pAddr->usBlockNumber :
                            ((PBLOCK_HEADER)pchBlockImage)->usNextBlock;

  if (ulBlockNumber == 0) rc = CLUSTER_EMPTY;
  else
  {
    /* The cluster is not empty so... .................................*/
    uldispBlock = ptmtg->pTmHeader->ldispFirstBlock +
       ((LONG)ulBlockNumber - 1L) * (LONG)usBlockSize; /* no. of bytes */

    rc = UtlChgFilePtr (ptmtg->hfTM,              /* file handle.......*/
                        uldispBlock,              /* displacement......*/
                        FILE_BEGIN,               /* displacement from.*/
                        &uldispBlock,             /* new pointer.......*/
                        FALSE);

    if (rc == OK)
      rc = UtlRead (ptmtg->hfTM,                  /* file handle.......*/
                    pchBlockImage,                /* pointer to block..*/
                    usBlockSize,                  /* block size........*/
                    &usBytesRead,                 /* bytes read........*/
                    FALSE);

      if ( rc == OK )                                                /* 5@1CA */
      {                                                             /*30 lines*/
        //--- check if bytes are really read
        //--- Reason:
        //--- A TM was cutted at the end because of a power failure,
        //--- but the key directory and the pointer were saved to disk.
        //--- So when the block didpositions are calculated, the are
        //--- pointing outside of the TM. UtlChgFilePtr returns rc = OK
        //--- but don't chabges the file pointer. UtlRead returns also
        //--- OK. So the same block of segments is read again and again

        if (  usBytesRead < usBlockSize )
        {
           //--- no bytes or too few bytes read,
           //--- skip the rest of this cluster and continue with the next
           rc = CLUSTER_EMPTY;
        }
        else
        {
           //--- check if the read block is really the block to be read
           //--- this is done by comparing the blocknumber of the block to
           //--- be read and the really read block
           if ( ulBlockNumber != ((PBLOCK_HEADER)pchBlockImage)->usBlockNum )
           {
              //--- the blocknumbers do not match, skip
              //---  the rest of this cluster and continue with the next
              rc = CLUSTER_EMPTY;
           } /* endif */
        } /* endif */
      } /* endif */

      // Hier muesste geprueft werden ob usBlockNum (die ersten 2 bytes von
      // pchBlockImage) auch uebereinstimmen mit ulBlockNumber. Fall dies
      // nicht der Fall ist muss das TM als corrupted declariert werden.
      // So eine Konstellation kann passieren wenn in einem Replace das
      // System abstuerzt.
      // Der code ist noch nicht implementiert weil das richtige verarbeiten
      // des neuen Return codes in den oberen Layers sehr complex ist und
      // intensiv studiert werden muss. Die Zeit dazu ist im Moment nicht
      // vorhanden. Aufwandschaetzung ca 1 Woche.
  }
  return(rc) ;
} /* End of ReadBlock */
