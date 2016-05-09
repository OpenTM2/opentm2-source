//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|Description:       Functions working on dictionary filters and applying     |
//|                   filters on dictionary entries.                           |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//|                                                                            |
//|  FiltOpen              Opens a filter                                      |
//|  FiltClose             Close a filter                                      |
//|  FiltWork              Apply a filter on dict. entries                     |
//|  FiltCheckFields       Check if all fields used in a filter are fields of  |
//|                        the given dictionary.                               |
//|  FiltCheckSelFields    Check if fields passes in a termlist are contained  |
//|                        in the select condition of the filter               |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//|                                                                            |
//|  FiltFieldName2Number   Get number for a dictionary field                  |
//|  FiltPopResult       Pop a result from the result stack                    |
//|  FiltPushResult      Push a result on the result stack                     |
//|  FiltSetResultPos    Set result stack position                             |
//|  FiltCompareValues   Compare two operands                                  |
//|  FiltMatchStrings    Perform pattern matching                              |
//|  FiltScanResult      Scan result stack for a given type                    |
//|  FiltString2Result   Convert a string to a result value                    |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_FILT             // dictionary filter functions
#include <eqf.h>                  // General Translation Manager include file

#include "EQFDASDI.H"             // internal ASD services header file
#include "OtmDictionaryIF.H"

#include "eqffilti.h"             // Filter private include file

#define CBID_SIZE    10                // size of control block ID
#define FILTERCB_ID  "EQF\001FLT\001CB"// ID of filter control blocks
#define NOT_IN_DICT  0xFFFF            // field number for fields which are not
                                       // contained in the dictionary
#define MAX_ALLOC      0xFF00          // maximum UtlAlloc allocation size
#define PLUS_SIGN      '+'             // plus sign (used in number evaluation)
#define MINUS_SIGN     '-'             // minus sign (used in number evaluation)
#define DECIMAL_POINT  '.'             // decimal point (     "       )

/**********************************************************************/
/* Result types                                                       */
/**********************************************************************/
typedef enum _RESULTTYPE
{
  RES_BOOLEAN,                         // result is a boolean value
  RES_NOTAVAILABLE,                    // not available value
  RES_STRING,                          // string value
  RES_LONG,                            // long value
  RES_DOUBLE,                          // double value (floating point value)
  RES_DATE,                            // date value
  RES_STARTOFLIST,                     // start of list identifier
  RES_STACK_EMPTY                      // stack is empty identifier
} RESULTTYPE;

/**********************************************************************/
/* Result structure                                                   */
/**********************************************************************/
typedef struct _RESULT
{
  RESULTTYPE     Type;                 // type of result value
  BOOL           fValue;               // value for RES_BOOLEAN results
  FDATE          dateValue;            // value for RES_DATE results
  PSZ_W          pszValue;             // value for results except RES_BOOLEAN
} RESULT, *PRESULT;

/**********************************************************************/
/* Result stack                                                       */
/**********************************************************************/
typedef struct _RESSTACK
{
  USHORT            usMaxSize;         // max number of stack elements
  USHORT            usElements;        // number of elements on stack
  PRESULT           pStack;            // pointer to stack area
  PRESULT           pCurPos;           // pointer to next element
} RESSTACK, *PRESSTACK;

/**********************************************************************/
/* Result of compare function                                         */
/**********************************************************************/
typedef struct _COMPRES
{
  unsigned int   fEqual       : 1;     // operands are equal
  unsigned int   fGreater     : 1;     // operand 1 is greater than operand 2
  unsigned int   fSmaller     : 1;     // operand 1 is smaller than operand 2
} COMPRES, *PCOMPRES;

/**********************************************************************/
/* date display formats (must be in same order as OS2 setup formats)  */
/**********************************************************************/
typedef enum _DATEDISPLAYFORMAT
{
   MDY_DATE_FORMAT,                    // date format MM DD YY
   DMY_DATE_FORMAT,                    // date format DD MM YY
   YMD_DATE_FORMAT                     // date format YY MM DD
} DATEDISPLAYFORMAT;


/**********************************************************************/
/* Filter control block                                               */
/**********************************************************************/
typedef struct _FILTERCB
{
  CHAR        szCBID[CBID_SIZE];       // control block ID
  FILTER      Filt;                    // filter data
  PDCB        pDCB;                    // ptr to dictionary control block
  USHORT      usSelFields;             // # of elements in selected names array
  USHORT      usStackElements;         // # of elements in polish stack
  PSZ_W       apszFields[MAX_PROF_ENTRIES]; // buffer for field data pointer
  RESSTACK    ResStack;                // result stack
  CHAR        chDateSeper;             // date seperator from OS2.INI
  USHORT      usDateFormat;            // date format from OS2.INI
} FILTERCB, *PFILTERCB;

/**********************************************************************/
/* numbers for internal errors                                        */
/**********************************************************************/
#define FILTOPEN_INPARM_INTERR         ID_FILT_DLG + 1
#define FILTCLOSE_INPARM_INTERR        ID_FILT_DLG + 2
#define FILTWORK_INPARM_INTERR         ID_FILT_DLG + 3
#define FILTWORK_OPSTACK_INTERR        ID_FILT_DLG + 4
#define FILTCHECKFIELDS_INPARM_INTERR  ID_FILT_DLG + 5

/**********************************************************************/
/* Function prototypes                                                */
/**********************************************************************/
USHORT FiltFieldName2Number
(
  PSZ             pszFieldName,        // pointer to field name
  PPROPDICTIONARY pProp                // pointer to dictionary properties
);

VOID FiltPopResult
(
  PRESSTACK    pResStack,              // ptr to result stack
  PRESULT      pResult                 // buffer for returned result
);

BOOL FiltPushResult
(
  PRESSTACK    pResStack,              // ptr to result stack
  PRESULT      pResult                 // result to be pushed onto stack
);

VOID FiltSetResultPos
(
  PRESSTACK    pResStack,              // ptr to result stack
  PRESULT      pResult                 // new position on stack
);

BOOL FiltCompareValues
(
  PRESULT      pOp1,                   // first operand
  PRESULT      pOp2,                   // second operand
  PCOMPRES     pCompRes                // result of compare
);

BOOL FiltMatchStrings
(
  PRESULT      pOp1,                   // first operand
  PRESULT      pOp2,                   // second operand
  PCOMPRES     pCompRes                // result of compare
);

BOOL FiltScanResult
(
  PRESSTACK    pResStack,              // ptr to result stack
  RESULTTYPE   ResType,                // type being looked for
  PRESULT      *ppResult               // found stack position
);

VOID FiltString2Result
(
  PFILTERCB    pCB,                    // pointer to filter control block
  PSZ_W        pszString,              // input string
  PRESULT      pResult                 // buffer for result
);

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltOpen              Opens a filter                     |
//+----------------------------------------------------------------------------+
//|Function call:     FiltOpen( PSZ pszFiltName );                             |
//+----------------------------------------------------------------------------+
//|Description:       Opens a dictionary filter for usage in QFiltWork function|
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ   pszFiltName        Name of filter (w/o path info)  |
//|                   HDCB  hDCB               handle of dictionary being      |
//|                                            worked on                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   HFILTER                  Handle of filter                |
//+----------------------------------------------------------------------------+
//|Return codes:      NULL       filter open failed, errors have been reported |
//|                   other      handle of filter control block                |
//+----------------------------------------------------------------------------+
//|Function flow:     check input parameters                                   |
//|                   if ok allocate filter control block                      |
//|                   if ok read filter into memory                            |
//|                   if ok convert field offsets to field numbers             |
//|                   if ok allocate result stack                              |
//|                   if ok get date format from OS2.INI file                  |
//+----------------------------------------------------------------------------+
USHORT FiltOpen
(
  PSZ      pszFiltName,                // Name of filter (w/o path info)
  HDCB     hDCB,                       // handle of dictionary being worked on
  HFILTER *phFilter                    // address of the filter handle buffer
)
{
  USHORT    usRC = NO_ERROR;           // function return code
  PDCB      pDCB;                      // dictionary control block pointer
  PFILTERCB pCB = NULL;                // ptr to filter control block
  PUSHORT   pusField;                  // ptr for selected fields processing
  USHORT    usI;                       // general loop index
  USHORT    usDict;                    // index to loop over dictionaries
  PPOLISHSTACK pStack;                 // ptr for polish stack processing
  CHAR chTemp[2];                      // buffer for date seperator from OS2.INI

  /********************************************************************/
  /* Check input parameters                                           */
  /********************************************************************/
  pDCB = (PDCB)hDCB;                   // convert handle to control block ptr
  if ( !pDCB                               ||
       (pDCB->lSignature != DCB_SIGNATURE) ||
       !phFilter                           ||
       !pszFiltName                        ||
       !*pszFiltName                       ||
       (strlen( pszFiltName ) > (MAX_FNAME - 1)) )
  {
    UtlError( FILTOPEN_INPARM_INTERR, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
    usRC = ERROR_INVALID_DATA;
  } /* endif */

  /********************************************************************/
  /* Allocate filter control block                                    */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    UtlAlloc( (PVOID *) &pCB, 0L, (LONG)sizeof(FILTERCB), ERROR_STORAGE );
    if ( !pCB )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
      memcpy( pCB->szCBID, FILTERCB_ID, CBID_SIZE );
      pCB->pDCB = pDCB;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* read filter into memory                                          */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    UtlMakeEQFPath( pCB->Filt.Prop.szFilterPath, NULC, PROPERTY_PATH, NULL );
    strcat( pCB->Filt.Prop.szFilterPath, BACKSLASH_STR );
    strcat( pCB->Filt.Prop.szFilterPath, pszFiltName );
    strcat( pCB->Filt.Prop.szFilterPath, EXT_OF_FILTPROP );
    usRC = FiltRead( pCB->Filt.Prop.szFilterPath, &pCB->Filt );
  } /* endif */

  /********************************************************************/
  /* Get list of dictionaries and allocate stack and selected names   */
  /* array                                                            */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    AsdRetDictList( hDCB, pCB->Filt.hDCB, &pCB->Filt.usDicts );
    for ( usDict = 0;
          (usDict < pCB->Filt.usDicts) && (usRC == NO_ERROR);
          usDict++ )
    {
      UtlAlloc( (PVOID *) &(pCB->Filt.pOpStack[usDict]), 0L,
                (LONG)max( MIN_ALLOC, pCB->Filt.usStackUsed), ERROR_STORAGE );
      if ( pCB->Filt.pOpStack[usDict] == NULL )
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      }
      else
      {
        memcpy( pCB->Filt.pOpStack[usDict], pCB->Filt.pStack,
                pCB->Filt.usStackUsed );

        UtlAlloc( (PVOID *) &(pCB->Filt.pusSelected[usDict]), 0L,
                  (LONG)max( MIN_ALLOC, pCB->Filt.usSelNameUsed), ERROR_STORAGE );
        if ( pCB->Filt.pusSelected[usDict] == NULL )
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
          memcpy( pCB->Filt.pusSelected[usDict], pCB->Filt.pusSelNames,
                  pCB->Filt.usSelNameUsed );
        } /* endif */
      } /* endif */
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* Convert field offsets in selected names array to field numbers   */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    pCB->usSelFields = pCB->Filt.usSelNameUsed / sizeof(USHORT);
    for ( usDict = 0; usDict < pCB->Filt.usDicts; usDict++ )
    {
      pusField         = pCB->Filt.pusSelected[usDict];
      for ( usI = 0; usI < pCB->usSelFields; usI++, pusField++ )
      {
        *pusField = FiltFieldName2Number( (PSZ)(pCB->Filt.pucBuffer + *pusField),
                                    &(((PDCB)(pCB->Filt.hDCB[usDict]))->Prop) );
      } /* endfor */
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* Preprocess operands in polish stack:                             */
  /*                                                                  */
  /* - Convert field offsets in to field numbers                      */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    pCB->usStackElements = pCB->Filt.usStackUsed / sizeof(POLISHSTACK);
    for ( usDict = 0; usDict < pCB->Filt.usDicts; usDict++ )
    {
      pStack           = pCB->Filt.pOpStack[usDict];
      for ( usI = 0; usI < pCB->usStackElements; usI++, pStack++ )
      {
        if ( pStack->sOperatorId == OP_FIELDNAME )
        {
          pStack->usOffset = FiltFieldName2Number( (PSZ)(pCB->Filt.pucBuffer +
                                                   pStack->usOffset),
                                    &(((PDCB)(pCB->Filt.hDCB[usDict]))->Prop) );
        } /* endif */
      } /* endfor */
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* allocate result stack area                                       */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    pCB->ResStack.usMaxSize = min( pCB->usStackElements,
                                   MAX_ALLOC / sizeof(RESULT) );
    UtlAlloc( (PVOID *) &pCB->ResStack.pStack, 0L,
              (LONG)max( MIN_ALLOC, pCB->ResStack.usMaxSize * sizeof(RESULT) ),
              ERROR_STORAGE );
    if ( !pCB->ResStack.pStack )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
      pCB->ResStack.pCurPos    = pCB->ResStack.pStack;
      pCB->ResStack.usElements = 0;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Get date format from OS2.INI file                                */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    pCB->usDateFormat = (USHORT)WinQueryProfileInt( NULL,
                                            "PM_National",
                                            "iDate",
                                            YMD_DATE_FORMAT );
    WinQueryProfileString( NULL,
                           "PM_National",
                           "sDate",
                           "/",
                           chTemp,
                           2 );
    pCB->chDateSeper = chTemp[0];
  } /* endif */

  /********************************************************************/
  /*  Return filter handle / cleanup                                  */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    *phFilter = pCB;
  }
  else
  {
    if ( pCB )
    {
      if ( pCB->ResStack.pStack )
      {
        UtlAlloc( (PVOID *) &pCB->ResStack.pStack, 0L, 0L, NOMSG );
      } /* endif */
      UtlAlloc( (PVOID *) &pCB, 0L, 0L, NOMSG );
    } /* endif */
    *phFilter = NULL;
  } /* endif */

  return ( usRC );
} /* end of function FiltOpen */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltClose              Close a filter                    |
//+----------------------------------------------------------------------------+
//|Function call:     FiltClose( HFILTER hFilter );                            |
//+----------------------------------------------------------------------------+
//|Description:       Closes a dictionary filter which have been opened using  |
//|                   FiltOpen. All allocated memory areas are freed and       |
//|                   the filter control block is destroyed.                   |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILTER hFilter          filter handle                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Function flow:     check input parameters                                   |
//|                   if ok                                                    |
//|                     free result stack                                      |
//|                     free filter control block                              |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
USHORT FiltClose
(
  HFILTER hFilter                        // handle of filter
)
{
  USHORT    usRC = NO_ERROR;           // function return code
  PFILTERCB pCB;                       // ptr to filter control block

  /********************************************************************/
  /* Check input parameters                                           */
  /********************************************************************/
  pCB  = (PFILTERCB)hFilter;           // convert handle to control block ptr
  if ( !pCB || (memcmp( pCB->szCBID, FILTERCB_ID, CBID_SIZE ) != 0) )
  {
    UtlError( FILTCLOSE_INPARM_INTERR, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
    usRC = ERROR_INVALID_DATA;
  } /* endif */

  /********************************************************************/
  /* Destroy filter control block                                     */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    if ( pCB->ResStack.pStack )
    {
      UtlAlloc( (PVOID *) &pCB->ResStack.pStack, 0L, 0L, NOMSG );
    } /* endif */
    FiltFree( &pCB->Filt );
    UtlAlloc( (PVOID *) &pCB, 0L, 0L, NOMSG );
  } /* endif */

  return( usRC );
} /* end of function FiltClose */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltWork/FiltWork2    Apply a filter on dict. entries    |
//+----------------------------------------------------------------------------+
//|Function call:     FiltWork( HFILTER hFilter, PQLDB_HTREE hLdbTree,         |
//|                             PQLDB_HTREE *phNewLdbTree );                   |
//|Function call:     FiltWork2( HFILTER hFilter, PQLDB_HTREE hLdbTree,        |
//|                             PQLDB_HTREE *phNewLdbTree, HDCB hDCB );        |
//+----------------------------------------------------------------------------+
//|Description:       Apply a previously opened filter on a dictionary entry.  |
//|                   Only the part of the dictionary entry fulfilling the     |
//|                   the dictionary filter is left in the input/output buffer.|
//|                   If no part of the dictionary entry matches the filter    |
//|                   criteria the length of the new entry is zero.            |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILTER     hFilter      filter handle                   |
//|                   PQLDB_HTREE hLdbTree     handle of LDB tree              |
//|                   HDCB        hDCB         handle of dictionary containing |
//|                                            entry                           |
//+----------------------------------------------------------------------------+
//|Output parameter:  PQLDB_HTREE *phNewLdbTree handle of new LDB tree or NULL |
//|                   if no part of the entry matched the filter.              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                   function return code            |
//+----------------------------------------------------------------------------+
//|Return codes:      NO_ERROR                 dictionary entry matches filter |
//|                   ERROR_NO_MORE_FILES      no template of the entry        |
//|                                            matches the filter              |
//|                   ERROR_NOT_ENOUGH_MEMORY  memory shortage                 |
//|                   ERROR_INVALID_FUNCTION   filter stack is corrupted       |
//+----------------------------------------------------------------------------+
//|Function flow:     check input parameters                                   |
//|                   position to first template of LDB tree                   |
//|                   get current template                                     |
//|                   while not end and no error occurred                      |
//|                     initialize result stack                                |
//|                     loop over polish stack                                 |
//|                       switch stack operator                                |
//|                         case OP_AND:                                       |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           if result types are boolean                      |
//|                             combine result values using AND                |
//|                             push new result on stack                       |
//|                           else                                             |
//|                             stack is corrupted                             |
//|                         case OP_OR:                                        |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           if result types are boolean                      |
//|                             combine result values using OR                 |
//|                             push new result on stack                       |
//|                           else                                             |
//|                             stack is corrupted                             |
//|                         case OP_NOT:                                       |
//|                           pop result1 from stack                           |
//|                           if result type is boolean                        |
//|                             negate result value                            |
//|                             push new result on stack                       |
//|                           else                                             |
//|                             stack is corrupted                             |
//|                         case OP_EQUAL:                                     |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           compare results using FiltCompareValues          |
//|                           set result type to boolean                       |
//|                           set result value to result of compare            |
//|                           push new result on stack                         |
//|                         case OP_NOTEQUAL:                                  |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           compare results using FiltCompareValues          |
//|                           set result type to boolean                       |
//|                           set result value to result of compare            |
//|                           push new result on stack                         |
//|                         case OP_SMALLER:                                   |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           compare results using FiltCompareValues          |
//|                           set result type to boolean                       |
//|                           set result value to result of compare            |
//|                           push new result on stack                         |
//|                         case OP_GREATER:                                   |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           compare results using FiltCompareValues          |
//|                           set result type to boolean                       |
//|                           set result value to result of compare            |
//|                           push new result on stack                         |
//|                         case OP_SMALLEREQUAL:                              |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           compare results using FiltCompareValues          |
//|                           set result type to boolean                       |
//|                           set result value to result of compare            |
//|                           push new result on stack                         |
//|                         case OP_GREATEREQUAL:                              |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           compare results using FiltCompareValues          |
//|                           set result type to boolean                       |
//|                           set result value to result of compare            |
//|                           push new result on stack                         |
//|                         case OP_LIKE:                                      |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           compare results using FiltMatchStrings           |
//|                           set result type to boolean                       |
//|                           set result value to result of compare            |
//|                           push new result on stack                         |
//|                         case OP_BETWEEN:                                   |
//|                           pop result3 from stack                           |
//|                           pop result2 from stack                           |
//|                           pop result1 from stack                           |
//|                           set result type to boolean                       |
//|                           compare result 1 and 2 using FiltCompareStrings  |
//|                           if result 2 is smaller or equal                  |
//|                             compare result 1 and 3 using FiltCompareStrings|
//|                             if result 3 is greater or equal                |
//|                               result value is TRUE                         |
//|                             else                                           |
//|                               result value is FALSE                        |
//|                           else                                             |
//|                             result value is FALSE                          |
//|                           push new result on stack                         |
//|                         case OP_IN:                                        |
//|                           set result3 type to boolean                      |
//|                           set result3 value to FALSE                       |
//|                           get position of START_LIST result in stack       |
//|                           get result before START_LIST result into result1 |
//|                           while result3 value is FALSE and                 |
//|                             result type is not START_LIST                  |
//|                             pop result2 from stack                         |
//|                             compare result 1 and 2 using FiltCompareValues |
//|                             set result3 value approbriate                  |
//|                           push new result on stack                         |
//|                         case OP_STRING:                                    |
//|                         case OP_NUM:                                       |
//|                           convert operand string to result value           |
//|                           push result on stack                             |
//|                         case OP_FIELDNAME:                                 |
//|                           get field value                                  |
//|                           convert field value to result value              |
//|                           push result on stack                             |
//|                         case OP_STARTLIST                                  |
//|                           setup START_LIST result                          |
//|                           push result on stack                             |
//|                       endswitch                                            |
//|                     endloop                                                |
//|                     pop result1 from stack                                 |
//|                     pop result2 from stack                                 |
//|                     if result1 type is not boolean or                      |
//|                        result2 type is not RES_STACK_EMPTY                 |
//|                        error in polish stack                               |
//|                     else                                                   |
//|                       set found flag on result1 value                      |
//|                     if result LDB tree requested and template matched      |
//|                       clear target field array                             |
//|                       copy field in selected names list                    |
//|                       add template to result LDB tree                      |
//|                     endif                                                  |
//|                     get next template                                      |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
USHORT FiltWork
(
  HFILTER      hFilter,                // filter handler
  PVOID  hLDBTree,               // handle of LDB tree
  PVOID  *phNewLDBTree           // handle of result tree or NULL if no
                                       // result tree is required
)                                                                 /*4@KIT1007A*/
{
  return( FiltWork2( hFilter, hLDBTree, phNewLDBTree, NULL ) );
} /* end of FiltWork */

USHORT FiltWork2                                                  /*7@KIT1007A*/
(
  HFILTER      hFilter,                // filter handler
  PVOID  hLDBTree,               // handle of LDB tree
  PVOID  *phNewLDBTree,          // handle of result tree or NULL if no
                                       // result tree is required
  HDCB         hDCB                    // handle of dictionary containing entry
)
{
  USHORT    usRC = QLDB_NO_ERROR;      // function return code
  BOOL      fOK = TRUE;                // internal OK flag
  USHORT    usEnd;                     // level returned by QldbNextTemplate function
  USHORT    usI;                       // loop counter
  BOOL      fEnd = FALSE;              // end-of-templates flag
  PFILTERCB pCB = NULL;                // pointer to filter control block
  PPOLISHSTACK pStack;                 // ptr for polish stack processing
  RESULT    Result1;
  RESULT    Result2, Result3;            // result buffers
  COMPRES   CompRes;                   // result of compare
  PRESULT   pStackElement;             // ptr to result stack elements
  BOOL      fFirstTime = TRUE;         // TRUE = no template has been written
  BOOL      fFound;                    // matching templates found flag
  PUSHORT   pusField;                  // ptr for selected fields processing
                                                                  /*3@KIT1007A*/
  PPOLISHSTACK     pPolishStack = NULL;  // operator stack for current dictionary
  PUSHORT   pusSelNames = NULL;          // selected name for current dictionary
  PPOOL     pPool;
  CHAR_W    chTemp[128];

  memset(&Result1, 0, sizeof(Result1));
  /********************************************************************/
  /* Check input parameters                                           */
  /********************************************************************/
  pCB  = (PFILTERCB)hFilter;           // convert handle to control block ptr
  if ( !pCB || (memcmp( pCB->szCBID, FILTERCB_ID, CBID_SIZE ) != 0) )
  {
    UtlError( FILTWORK_INPARM_INTERR, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
    usRC = ERROR_INVALID_DATA;
  } /* endif */

  /********************************************************************/
  /* Prepare select fields and polish stack for active dictionary     */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )                                    /*2@KIT1007A*/
  {
    if ( hDCB == NULL )                // no specific dictionary specified ???
    {
      pPolishStack = pCB->Filt.pOpStack[0];    // use polish stack of first dict.
      pusSelNames  = pCB->Filt.pusSelected[0]; // use selected names of first dict.
    }
    else
    {
      /****************************************************************/
      /* Search dictionary using the supplied dictionary handle       */
      /****************************************************************/
      for ( usI = 0; usI < pCB->Filt.usDicts; usI++ )
      {
        if ( pCB->Filt.hDCB[usI] == hDCB )
        {
          break;
        } /* endif */
      } /* endfor */

      /****************************************************************/
      /* Use found dictionary or set error condition                  */
      /****************************************************************/
      if ( usI < pCB->Filt.usDicts )
      {
        pPolishStack = pCB->Filt.pOpStack[usI];
        pusSelNames  = pCB->Filt.pusSelected[usI];
      }
      else
      {
        usRC = ERROR_INVALID_DATA;
      } /* endif */
    } /* endif */
  } /* endif */

  // allocate buffer for strings of MLE which are needed in unicode
  pPool = PoolCreate( 4000 );
  fOK = (pPool != NULL);


  /********************************************************************/
  /* start at first template of LDB tree                              */
  /********************************************************************/
  // position to first template of LDB tree
  if ( usRC == QLDB_NO_ERROR )                                    /*2@KIT1007A*/
  {
    usRC = QLDBResetTreePositions( hLDBTree );
  } /* endif */                                                   /*1@KIT1007A*/

  // get first (= current) template
  if ( usRC == QLDB_NO_ERROR )
  {
    usRC = QLDBCurrTemplate( hLDBTree, pCB->apszFields );
  } /* endif */

  // filter all templates of the LDB tree
  fFound = FALSE;
  while ( !fEnd && (usRC == QLDB_NO_ERROR) )
  {
    /******************************************************************/
    /* Reset result stack                                             */
    /******************************************************************/
    pCB->ResStack.pCurPos = pCB->ResStack.pStack;
    pCB->ResStack.usElements = 0;

    /******************************************************************/
    /* Process polish stack                                           */
    /******************************************************************/
    pStack = pPolishStack;
    if ( !pCB->usStackElements )
    {
      Result1.fValue = TRUE;
      fFound |= Result1.fValue;
    }
    else
    {
      for ( usI = 0; (fOK && (usI < pCB->usStackElements)); usI++, pStack++ )
      {
        switch ( pStack->sOperatorId )
        {
          case OP_AND :
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( (Result1.Type == RES_NOTAVAILABLE) ||
                 (Result2.Type == RES_NOTAVAILABLE) )
            {
              Result1.fValue = FALSE;            // always FALSE!!!
              Result1.Type   = RES_BOOLEAN;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else if ( (Result1.Type == RES_BOOLEAN) &&
                      (Result2.Type == RES_BOOLEAN) )
            {
              Result1.fValue = Result1.fValue && Result2.fValue;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FALSE;               // polish stack error
            } /* endif */
            break;

          case OP_OR :
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( (Result1.Type == RES_NOTAVAILABLE) ||
                 (Result2.Type == RES_NOTAVAILABLE) )
            {
              Result1.fValue = FALSE;            // always FALSE!!!
              Result1.Type   = RES_BOOLEAN;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else if ( (Result1.Type == RES_BOOLEAN) &&
                      (Result2.Type == RES_BOOLEAN) )
            {
              Result1.fValue = Result1.fValue || Result2.fValue;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FALSE;               // polish stack error
            } /* endif */
            break;

          case OP_NOT :
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( Result1.Type == RES_NOTAVAILABLE )
            {
              Result1.fValue = FALSE;            // always FALSE!!!
              Result1.Type   = RES_BOOLEAN;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else if ( Result1.Type == RES_BOOLEAN )
            {
              Result1.fValue = !Result1.fValue;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FALSE;               // polish stack error
            } /* endif */
            break;

          case OP_EQUAL :
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( (Result1.Type == RES_NOTAVAILABLE) ||
                 (Result2.Type == RES_NOTAVAILABLE) )
            {
              Result1.fValue = FALSE;            // always FALSE!!!
              Result1.Type   = RES_BOOLEAN;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FiltCompareValues( &Result1, &Result2, &CompRes );
              if ( fOK )
              {
                Result1.Type   = RES_BOOLEAN;
                if ( (Result1.Type == RES_NOTAVAILABLE) ||
                     (Result2.Type == RES_NOTAVAILABLE) )
                {
                  Result1.fValue = FALSE;
                }
                else
                {
                  Result1.fValue = CompRes.fEqual;
                } /* endif */
                fOK = FiltPushResult( &pCB->ResStack, &Result1 );
              } /* endif */
            } /* endif */
            break;

          case OP_NOTEQUAL :
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( (Result1.Type == RES_NOTAVAILABLE) ||
                 (Result2.Type == RES_NOTAVAILABLE) )
            {
              Result1.fValue = FALSE;            // always FALSE!!!
              Result1.Type   = RES_BOOLEAN;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FiltCompareValues( &Result1, &Result2, &CompRes );
              if ( fOK )
              {
                Result1.Type   = RES_BOOLEAN;
                if ( (Result1.Type == RES_NOTAVAILABLE) ||
                     (Result2.Type == RES_NOTAVAILABLE) )
                {
                  Result1.fValue = FALSE;
                }
                else
                {
                  Result1.fValue = !CompRes.fEqual;
                } /* endif */
                fOK = FiltPushResult( &pCB->ResStack, &Result1 );
              } /* endif */
            } /* endif */
            break;

          case OP_SMALLER :
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( (Result1.Type == RES_NOTAVAILABLE) ||
                 (Result2.Type == RES_NOTAVAILABLE) )
            {
              Result1.fValue = FALSE;            // always FALSE!!!
              Result1.Type   = RES_BOOLEAN;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FiltCompareValues( &Result1, &Result2, &CompRes );
              if ( fOK )
              {
                Result1.Type   = RES_BOOLEAN;
                if ( (Result1.Type == RES_NOTAVAILABLE) ||
                     (Result2.Type == RES_NOTAVAILABLE) )
                {
                  Result1.fValue = FALSE;
                }
                else
                {
                  Result1.fValue = CompRes.fSmaller;
                } /* endif */
                fOK = FiltPushResult( &pCB->ResStack, &Result1 );
              } /* endif */
            } /* endif */
            break;

          case OP_GREATER :
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( (Result1.Type == RES_NOTAVAILABLE) ||
                 (Result2.Type == RES_NOTAVAILABLE) )
            {
              Result1.fValue = FALSE;            // always FALSE!!!
              Result1.Type   = RES_BOOLEAN;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FiltCompareValues( &Result1, &Result2, &CompRes );
              if ( fOK )
              {
                Result1.Type   = RES_BOOLEAN;
                if ( (Result1.Type == RES_NOTAVAILABLE) ||
                     (Result2.Type == RES_NOTAVAILABLE) )
                {
                  Result1.fValue = FALSE;
                }
                else
                {
                  Result1.fValue = CompRes.fGreater;
                } /* endif */
                fOK = FiltPushResult( &pCB->ResStack, &Result1 );
              } /* endif */
            } /* endif */
            break;

          case OP_SMALLEREQUAL :
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( (Result1.Type == RES_NOTAVAILABLE) ||
                 (Result2.Type == RES_NOTAVAILABLE) )
            {
              Result1.fValue = FALSE;            // always FALSE!!!
              Result1.Type   = RES_BOOLEAN;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FiltCompareValues( &Result1, &Result2, &CompRes );
              if ( fOK )
              {
                Result1.Type   = RES_BOOLEAN;
                if ( (Result1.Type == RES_NOTAVAILABLE) ||
                     (Result2.Type == RES_NOTAVAILABLE) )
                {
                  Result1.fValue = FALSE;
                }
                else
                {
                  Result1.fValue = CompRes.fSmaller || CompRes.fEqual;
                } /* endif */
                fOK = FiltPushResult( &pCB->ResStack, &Result1 );
              } /* endif */
            } /* endif */
            break;

          case OP_GREATEREQUAL :
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( (Result1.Type == RES_NOTAVAILABLE) ||
                 (Result2.Type == RES_NOTAVAILABLE) )
            {
              Result1.fValue = FALSE;            // always FALSE!!!
              Result1.Type   = RES_BOOLEAN;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FiltCompareValues( &Result1, &Result2, &CompRes );
              if ( fOK )
              {
                Result1.Type   = RES_BOOLEAN;
                if ( (Result1.Type == RES_NOTAVAILABLE) ||
                     (Result2.Type == RES_NOTAVAILABLE) )
                {
                  Result1.fValue = FALSE;
                }
                else
                {
                  Result1.fValue = CompRes.fGreater || CompRes.fEqual;
                } /* endif */
                fOK = FiltPushResult( &pCB->ResStack, &Result1 );
              } /* endif */
            } /* endif */
            break;

          case OP_LIKE :
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( Result1.Type == RES_NOTAVAILABLE )
            {
              Result1.Type   = RES_BOOLEAN;
              Result1.fValue = FALSE;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FiltMatchStrings( &Result1, &Result2, &CompRes );
              if ( fOK )
              {
                Result1.Type   = RES_BOOLEAN;
                Result1.fValue = CompRes.fEqual;
                fOK = FiltPushResult( &pCB->ResStack, &Result1 );
              } /* endif */
            } /* endif */
            break;


          case OP_BETWEEN :
            FiltPopResult( &pCB->ResStack, &Result3 );
            FiltPopResult( &pCB->ResStack, &Result2 );
            FiltPopResult( &pCB->ResStack, &Result1 );
            if ( Result1.Type == RES_NOTAVAILABLE )
            {
              Result1.Type   = RES_BOOLEAN;
              Result1.fValue = FALSE;
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            else
            {
              fOK = FiltCompareValues( &Result1, &Result2, &CompRes );
              if ( fOK )
              {
                if ( CompRes.fGreater || CompRes.fEqual )
                {
                  fOK = FiltCompareValues( &Result1, &Result3, &CompRes );
                  if ( fOK )
                  {
                    if ( CompRes.fSmaller || CompRes.fEqual )
                    {
                      Result1.Type   = RES_BOOLEAN;
                      Result1.fValue = TRUE;
                      fOK = FiltPushResult( &pCB->ResStack, &Result1 );
                    }
                    else
                    {
                      Result1.Type   = RES_BOOLEAN;
                      Result1.fValue = FALSE;
                      fOK = FiltPushResult( &pCB->ResStack, &Result1 );
                    } /* endif */
                  } /* endif */
                }
                else
                {
                  Result1.Type   = RES_BOOLEAN;
                  Result1.fValue = FALSE;
                  fOK = FiltPushResult( &pCB->ResStack, &Result1 );
                } /* endif */
              } /* endif */
            } /* endif */
            break;

          case OP_IN :
            /************************************************************/
            /* Prepare result buffer                                    */
            /************************************************************/
            Result3.Type   = RES_BOOLEAN;
            Result3.fValue = FALSE;

            /************************************************************/
            /* get value from result stack just before RES_STARTOFLIST  */
            /* entry                                                    */
            /************************************************************/
            fOK = FiltScanResult( &pCB->ResStack, RES_STARTOFLIST, &pStackElement );
            if ( fOK )
            {
              if ( pStackElement > pCB->ResStack.pStack )
              {
                /********************************************************/
                /* position to operand of IN and load it into Result1   */
                /********************************************************/
                pStackElement--;
                memcpy( &Result1, pStackElement, sizeof(RESULT) );
                if ( Result1.Type == RES_NOTAVAILABLE )
                {
                  Result3.fValue = FALSE;
                }
                else
                {
                  /********************************************************/
                  /* loop through IN list values                          */
                  /********************************************************/
                  Result2.Type = RES_STRING;
                  while ( fOK &&
                          !Result3.fValue &&
                          (Result2.Type != RES_STARTOFLIST))
                  {
                    FiltPopResult( &pCB->ResStack, &Result2 );
                    if ( Result2.Type != RES_STARTOFLIST )
                    {
                      fOK = FiltCompareValues( &Result1, &Result2, &CompRes );
                      if ( fOK )
                      {
                        if ( CompRes.fEqual )
                        {
                          Result3.fValue = TRUE;     // we found one matching value
                        } /* endif */
                      } /* endif */
                    } /* endif */
                  } /* endwhile */
                } /* endif */

                /********************************************************/
                /* Drop all values including IN operand and push        */
                /* result (in Result3) on result stack                  */
                /********************************************************/
                if ( fOK )
                {
                  FiltSetResultPos( &pCB->ResStack, pStackElement );
                  fOK = FiltPushResult( &pCB->ResStack, &Result3 );
                } /* endif */
              }
              else
              {
                fOK = FALSE;             // operand for IN is missing
              } /* endif */
            } /* endif */
            break;

          case OP_STRING :
          case OP_NUM :
            { PSZ_W pTmpPool;
              ULONG   ulLen;

              ulLen = ASCII2UnicodeBuf((PSZ)(pCB->Filt.pucBuffer + pStack->usOffset),
                             chTemp,
                             strlen((PSZ)(pCB->Filt.pucBuffer + pStack->usOffset)) , 0L);
              chTemp[ulLen] = EOS;
              pTmpPool = PoolAddStringW( pPool, &chTemp[0]);

              FiltString2Result( pCB, pTmpPool, &Result1 );
              fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            }
            break;

          case OP_FIELDNAME :
            if ( pStack->usOffset == NOT_IN_DICT )
            {
              Result1.Type   = RES_NOTAVAILABLE;
            }
            else
            {
              FiltString2Result( pCB, pCB->apszFields[pStack->usOffset], &Result1 );
            } /* endif */
            fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            break;

          case OP_STARTOFLIST :
            Result1.Type   = RES_STARTOFLIST;
            fOK = FiltPushResult( &pCB->ResStack, &Result1 );
            break;

          default :
            fOK = FALSE;                 // error in polish stack
            break;
        } /* endswitch */
      } /* endfor */

      /******************************************************************/
      /* get result of processing                                       */
      /******************************************************************/
      if ( fOK  )
      {
        FiltPopResult( &pCB->ResStack, &Result1 );
        FiltPopResult( &pCB->ResStack, &Result2 );
        if ( (Result1.Type != RES_BOOLEAN)    ||
             (Result2.Type != RES_STACK_EMPTY) )
        {
          fOK = FALSE;                   // error in polish stack
        }
        else
        {
          fFound |= Result1.fValue;
        } /* endif */
      } /* endif */
    } /* endif */

    if ( !fOK )
    {
      UtlError( FILTWORK_OPSTACK_INTERR, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
      fEnd = TRUE;
      usRC = ERROR_INVALID_FUNCTION;
    } /* endif */

    /******************************************************************/
    /* if requested add matching templates to result LDB tree         */
    /******************************************************************/
    if ( fOK && Result1.fValue && phNewLDBTree )
    {
      /****************************************************************/
      /* initialize target field array                                */
      /****************************************************************/
      for ( usI = 0; usI < pCB->pDCB->Prop.usLength; usI++ )
      {
        pCB->pDCB->apszFields[usI] = EMPTY_STRINGW;
      } /* endfor */

      /****************************************************************/
      /* copy fields in selected names list                           */
      /****************************************************************/
      pusField         = pusSelNames;
      if ( pCB->Filt.usSelNameUsed == 0 ) // = use all fields
      {
        for ( usI = 0; usI < pCB->pDCB->Prop.usLength; usI++ )
        {
          pCB->pDCB->apszFields[usI] = pCB->apszFields[usI];
        } /* endfor */
      }
      else
      {
        for ( usI = 0; usI < pCB->usSelFields; usI++, pusField++ )
        {
          if ( *pusField != NOT_IN_DICT )
          {
            pCB->pDCB->apszFields[*pusField] = pCB->apszFields[*pusField];
          } /* endif */
        } /* endfor */
      } /* endif */

      /****************************************************************/
      /* add template to target tree                                  */
      /****************************************************************/
      if ( fFirstTime )
      {
        usRC = QLDBCreateTree( pCB->pDCB->ausNoOfFields,
                               pCB->pDCB->apszFields,
                               phNewLDBTree );
        fFirstTime = FALSE;
      }
      else
      {
        usRC = QLDBAddSubtree( *phNewLDBTree, 2,
                          pCB->pDCB->apszFields + pCB->pDCB->ausFirstField[1] );
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* get next template                                              */
    /******************************************************************/
    if ( fOK )
    {
      /****************************************************************/
      /* if we found a matching template and no target tree is        */
      /* requested force end of loop else get next template           */
      /****************************************************************/
      if ( Result1.fValue && !phNewLDBTree )
      {
        fEnd = TRUE;
      }
      else
      {
        usRC = QLDBNextTemplate( hLDBTree, pCB->apszFields, &usEnd );
        fEnd = (usEnd == QLDB_END_OF_TREE);
      } /* endif */
    } /* endif */
  } /* endwhile */

  if ( fFound && !fFirstTime && phNewLDBTree && *phNewLDBTree )
  {
    usRC = QLDBResetTreePositions( *phNewLDBTree );
  } /* endif */

  PoolDestroy( pPool );
  if ( (usRC == NO_ERROR) && !fFound )
  {
    usRC = ERROR_NO_MORE_FILES;
  } /* endif */

  return( usRC );
} /* end of function FiltWork */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltCheckFields       Check fields used in a filter      |
//+----------------------------------------------------------------------------+
//|Function call:     FiltCheckFields( HFILTER hFilter,                        |
//|                                    PPROPDICTIONARY pDictProp,              |
//|                                    BOOL    fMsg );                         |
//+----------------------------------------------------------------------------+
//|Description:       Check if all dictionary fields used in a filter in the   |
//|                   SELECT and WHERE part are contained in the given         |
//|                   dictionary.                                              |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILTER     hFilter       filter handle                  |
//|                   PPROPDICTIONARY pDictProp properties of dictionary       |
//|                   BOOL        fMsg          show-message-box flag          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                   function return code            |
//+----------------------------------------------------------------------------+
//|Return codes:      NO_ERROR                 all filter fields are contained |
//|                                            in the dictionary               |
//|                   ERROR_INVALID_DATA       input data is incorrect         |
//|                   ERROR_FILE_NOT_FOUND     there are fields in the filter  |
//|                                            which are not part of the       |
//|                                            dictionary                      |
//+----------------------------------------------------------------------------+
//|Function flow:     check input parameters                                   |
//|                   while there are fields in the filter                     |
//|                     search field in dictionary properties                  |
//|                     if not found set return code and leave loop            |
//|                     next field                                             |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
USHORT FiltCheckFields
(
  HFILTER      hFilter,                // filter handler
  PPROPDICTIONARY pDictProp,           // properties of dictionary
  BOOL        fMsg                     // show-message-box flag
)
{
  USHORT    usRC = NO_ERROR;           // function return code
  PFILTERCB pCB;                       // pointer to filter control block
  USHORT    usNoOfNames;               // number of field names in filter
  USHORT    usI, usJ;                  // loop indices
  PSZ       pszField;                  // ptr to field name
  PPROFENTRY pEntry;                   // pointer for profile entry processing

  /********************************************************************/
  /* Check input parameters                                           */
  /********************************************************************/
  pCB  = (PFILTERCB)hFilter;           // convert handle to control block ptr
  if ( !pCB ||
       (memcmp( pCB->szCBID, FILTERCB_ID, CBID_SIZE ) != 0) ||
       (pDictProp == NULL) )
  {
    if ( fMsg )
    {
      UtlError( FILTCHECKFIELDS_INPARM_INTERR,
                MB_CANCEL, 0, NULL, INTERNAL_ERROR );
    } /* endif */
    usRC = ERROR_INVALID_DATA;
  } /* endif */

  /********************************************************************/
  /* Loop over all fields table and check against dictionary fields   */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usNoOfNames = pCB->Filt.usAllNameUsed / sizeof(USHORT);
    for ( usI = 0;
          (usI < usNoOfNames) && (usRC == NO_ERROR);
          usI++ )
    {
      /****************************************************************/
      /* Get pointer to field name                                    */
      /****************************************************************/
      pszField = (PSZ)(pCB->Filt.pucBuffer + pCB->Filt.pusAllNames[usI]);

      /****************************************************************/
      /* Search field in dictionary properties                        */
      /****************************************************************/
      pEntry = pDictProp->ProfEntry;
      usRC   = ERROR_FILE_NOT_FOUND;             // assume failure
      for ( usJ = 0;
            (usJ < pDictProp->usLength) && (usRC != NO_ERROR);
            usJ++, pEntry++ )
      {
        if ( stricmp( pszField, pEntry->chUserName ) == 0 )
        {
          usRC = NO_ERROR;                      // O.K. field matched
        } /* endif */
      } /* endfor */
    } /* endfor */
  } /* endif */

  if ( fMsg && (usRC == ERROR_FILE_NOT_FOUND ) )
  {
    if ( UtlError( ERROR_FIELDS_NOT_IN_DICT, MB_OKCANCEL, 0,
                   NULL, EQF_WARNING ) == MBID_OK )
    {
      /****************************************************************/
      /* reset return code as user wants to ignore the warning        */
      /****************************************************************/
      usRC = NO_ERROR;
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function FiltCheckFields */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltCheckSelFields    Check given fields with selected f.|
//+----------------------------------------------------------------------------+
//|Function call:     FiltCheckSelFields( HFILTER hFilter,                     |
//|                                       PSZ     pszFields );                 |
//+----------------------------------------------------------------------------+
//|Description:       Check if the given fields are contained in the select    |
//|                   condition of the filter.                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILTER     hFilter       filter handle                  |
//|                   PSZ         pszFields     ptr to field list (each field  |
//|                                             is terminated by '\0', end of  |
//|                                             list is an additional '\0'.    |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                      ptr to first field not in filter|
//+----------------------------------------------------------------------------+
//|Return codes:      NULL                     all given fields are contained  |
//|                                            in the SELECT part of the filter|
//|                   other                    ptr to first field not in filter|
//|                                            SELECT part                     |
//+----------------------------------------------------------------------------+
//|Function flow:     check input parameters                                   |
//|                   while there are fields in the field list                 |
//|                     search field in SELECTed fields of filter              |
//|                     if not found set return code and leave loop            |
//|                     next field                                             |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
PSZ FiltCheckSelFields
(
  HFILTER      hFilter,                // filter handle
  PSZ          pszFields               // ptr to field list
)
{
  USHORT    usRC = NO_ERROR;           // function return code
  PFILTERCB pCB;                       // pointer to filter control block
  USHORT    usNoOfNames;               // number of field names in filter
  USHORT    usI;                       // loop indices
  PSZ       pszField;                  // ptr to field name
  PSZ       pszErrField = NULL;        // ptr to first erraneous field

  /********************************************************************/
  /* Check input parameters                                           */
  /********************************************************************/
  pCB  = (PFILTERCB)hFilter;           // convert handle to control block ptr
  if ( !pCB ||
       (memcmp( pCB->szCBID, FILTERCB_ID, CBID_SIZE ) != 0) )
  {
    usRC = ERROR_INVALID_DATA;
  } /* endif */

  /********************************************************************/
  /* If there no SELECTed fiels in the filter, set usRC in order      */
  /* to avoid check against selected fields                           */
  /********************************************************************/
  if ( (usRC == NO_ERROR) && (pCB->Filt.usSelNameUsed == 0) )
  {
    usRC = ERROR_NO_ITEMS;
  } /* endif */

  /********************************************************************/
  /* Loop over all fields in given list and check against SELECTed    */
  /* fields                                                           */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usNoOfNames = pCB->Filt.usSelNameUsed / sizeof(USHORT);
    while ( (*pszFields != EOS) && (pszErrField == NULL) )
    {
      usRC   = ERROR_FILE_NOT_FOUND;             // assume failure
      for ( usI = 0;
            (usI < usNoOfNames) && (usRC != NO_ERROR);
            usI++ )
      {
        /****************************************************************/
        /* Get pointer to field name                                    */
        /****************************************************************/
        pszField = (PSZ)(pCB->Filt.pucBuffer + pCB->Filt.pusSelNames[usI]);

        /****************************************************************/
        /* Compare field names                                          */
        /****************************************************************/
        if ( stricmp( pszField, pszFields ) == 0 )
        {
          usRC = NO_ERROR;                      // O.K. field matched
        } /* endif */
      } /* endfor */

      /****************************************************************/
      /* Set field-in-error pointer or continue with next one         */
      /****************************************************************/
      if ( usRC != NO_ERROR )
      {
        pszErrField = pszFields;                 // remember erraneous field
      }
      else
      {
        pszFields += strlen(pszFields) + 1;
      } /* endif */
    } /* endwhile */
  } /* endif */

  return( pszErrField );
} /* end of function FiltCheckSelFields */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltFieldName2Number   Get number for a dictionary field |
//+----------------------------------------------------------------------------+
//|Function call:     FiltFieldName2Number( PSZ pszFieldName,                  |
//|                                         PPROPDICTIONARY pProp );           |
//+----------------------------------------------------------------------------+
//|Description:       Looks for a given field in the dictionary and returns    |
//|                   the number of the field.                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ             pszFieldName  pointer to field name      |
//|                   PPROPDICTIONARY pProp         pointer to dictionary props|
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NOT_IN_DICT        if field is not in dictionary         |
//|                   other              number of field in dictionary         |
//+----------------------------------------------------------------------------+
//|Samples:           usField = FiltFieldName2Number( "Author", &DictProp );   |
//+----------------------------------------------------------------------------+
//|Function flow:     set field number to NOT_IN_DICT                          |
//|                   loop over dictionary fields                              |
//|                     if field with given user name found                    |
//|                       set field number to current field                    |
//|                       break out of loop                                    |
//|                   return field number                                      |
//+----------------------------------------------------------------------------+
USHORT FiltFieldName2Number
(
  PSZ             pszFieldName,        // pointer to field name
  PPROPDICTIONARY pProp                // pointer to dictionary properties
)
{
  USHORT   usI;                        // loop counter
  PPROFENTRY  pProfEntry;              // pointer for profile entry processing
  USHORT      usField = NOT_IN_DICT;   // found field number

  pProfEntry = pProp->ProfEntry;
  for ( usI = 0; usI < pProp->usLength; usI++, pProfEntry++ )
  {
    if ( strcmp( pProfEntry->chUserName, pszFieldName ) == 0 )
    {
      usField = usI;                   // remember field number
      break;                           // break out of for loop
    } /* endif */
  } /* endfor */

  return( usField );

} /* end of function FiltFieldName2Number */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltPopResult       Pop a result from the result stack   |
//+----------------------------------------------------------------------------+
//|Function call:     FiltPopResult( pResStack, &Result );                     |
//+----------------------------------------------------------------------------+
//|Description:       Pop a result from the result stack. If the result stack  |
//|                   is empty the result type is set to RES_STACK_EMPTY       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PRESSTACK    pResStack   ptr to result stack             |
//+----------------------------------------------------------------------------+
//|Output parameter:  PRESULT      pResult     buffer for returned result      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if result stack is not empty                             |
//|                     adjust current position within stack                   |
//|                     copy result to supplied buffer                         |
//|                   else                                                     |
//|                     set result type to RES_STACK_EMPTY                     |
//+----------------------------------------------------------------------------+
VOID FiltPopResult
(
  PRESSTACK    pResStack,              // ptr to result stack
  PRESULT      pResult                 // buffer for returned result
)
{
  if ( pResStack->pCurPos > pResStack->pStack )
  {
    pResStack->pCurPos--;
    pResStack->usElements--;
    memcpy( pResult, pResStack->pCurPos, sizeof(RESULT) );
  }
  else
  {
    pResult->Type = RES_STACK_EMPTY;
  } /* endif */
} /* end of function FiltPopResult */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltPushResult      Push a result on the result stack    |
//+----------------------------------------------------------------------------+
//|Function call:     FiltPushResult( pResStack, &Result );                    |
//+----------------------------------------------------------------------------+
//|Description:       Push a result on the result stack                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PRESSTACK    pResStack   ptr to result stack             |
//|                   PRESULT      pResult     result to be pushed onto stack  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     result has been pushed on the result stack      |
//|                   FALSE    result stack is full, no result has been pushed |
//+----------------------------------------------------------------------------+
//|Function flow:     if result stack is full                                  |
//|                     set result to FALSE                                    |
//|                   else                                                     |
//|                     copy result on stack                                   |
//|                     adjust stack pointers                                  |
//+----------------------------------------------------------------------------+
BOOL FiltPushResult
(
  PRESSTACK    pResStack,              // ptr to result stack
  PRESULT      pResult                 // result to be pushed onto stack
)
{
  BOOL   fOK = TRUE;                   // function return code

  if ( pResStack->usElements >= pResStack->usMaxSize )
  {
    fOK = TRUE;
  }
  else
  {
    memcpy( pResStack->pCurPos, pResult, sizeof(RESULT) );
    pResStack->pCurPos++;
    pResStack->usElements++;
  } /* endif */

  return( fOK );
} /* end of function FiltPushResult */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltSetResultPos    Set result stack position            |
//+----------------------------------------------------------------------------+
//|Function call:     FiltSetResultPos( pResStack, pNewPos );                  |
//+----------------------------------------------------------------------------+
//|Description:       Sets the result stack pointer to the given value         |
//+----------------------------------------------------------------------------+
//|Input parameter:   PRESSTACK    pResStack   ptr to result stack             |
//|                   PRESULT      pResult     new position on stack           |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     set current position pointer to given value              |
//+----------------------------------------------------------------------------+
VOID FiltSetResultPos
(
  PRESSTACK    pResStack,              // ptr to result stack
  PRESULT      pResult                 // new position on stack
)
{
  pResStack->pCurPos    = pResult;
  pResStack->usElements = (USHORT)(pResult - pResStack->pStack);
} /* end of function FiltSetResultPos */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltCompareValues   Compare two operands                 |
//+----------------------------------------------------------------------------+
//|Function call:     FiltCompareValues( pOp1, pOp2, &CompRes )                |
//+----------------------------------------------------------------------------+
//|Description:       Compares to operands and returns the result of the       |
//|                   compare                                                  |
//+----------------------------------------------------------------------------+
//|Input parameter:   PRESULT      pOp1        first operand                   |
//|                   PRESULT      pOp2        second operand                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PCOMPRES     pCompRes    result of compare               |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    operands could be compares                       |
//|                   FALSE   operands are not valid                           |
//+----------------------------------------------------------------------------+
//|Function flow:     check if operands are of valid formats, valid are long,  |
//|                     date, double and string                                |
//|                   if both operands are of the date type                    |
//|                     compare date values                                    |
//|                   elseif on of the operands is of type date or string      |
//|                     do a string compare                                    |
//|                   elseif both operands are of type long                    |
//|                     convert operands to long values and compare them       |
//|                   else                                                     |
//|                     convert operands to double values and compare them     |
//+----------------------------------------------------------------------------+
BOOL FiltCompareValues
(
  PRESULT      pOp1,                   // first operand
  PRESULT      pOp2,                   // second operand
  PCOMPRES     pCompRes                // result of compare
)
{
  BOOL         fOK = TRUE;             // internal OK flag
  LONG         lOp1, lOp2;             // buffer for long values
  double       dbOp1, dbOp2;           // buffer for double values
  LONG         lCompare;               // buffer for compare result

  /********************************************************************/
  /* Initialize compare result                                        */
  /********************************************************************/
  pCompRes->fEqual    = FALSE;
  pCompRes->fSmaller  = FALSE;
  pCompRes->fGreater  = FALSE;

  if ( (pOp1->Type != RES_NOTAVAILABLE) &&
       (pOp2->Type != RES_NOTAVAILABLE) )
  {

    /********************************************************************/
    /* Check if operands are in valid formats                           */
    /********************************************************************/
    if ( ( (pOp1->Type != RES_STRING)   &&
           (pOp1->Type != RES_LONG)     &&
           (pOp1->Type != RES_DOUBLE)   &&
           (pOp1->Type != RES_DATE) )      ||
         ( (pOp2->Type != RES_STRING)   &&
           (pOp2->Type != RES_LONG)     &&
           (pOp2->Type != RES_DOUBLE)   &&
           (pOp2->Type != RES_DATE) ) )
    {
      fOK = FALSE;                       // at least one operand has a wrong format
    } /* endif */

    /********************************************************************/
    /* do actual compare                                                */
    /********************************************************************/
    if ( fOK )
    {
      if ( (pOp1->Type == RES_DATE) && (pOp2->Type == RES_DATE) )
      {
        /****************************************************************/
        /* do a date compare as both operands are in date format        */
        /****************************************************************/
        lCompare = memcmp( &pOp1->dateValue, &pOp2->dateValue, sizeof(FDATE) );
        switch ( lCompare )
        {
          case -1 :
            pCompRes->fSmaller = TRUE;
            break;
          case 0 :
            pCompRes->fEqual   = TRUE;
            break;
          default :
            pCompRes->fGreater = TRUE;
            break;
        } /* endswitch */
      }
      else if ( (pOp1->Type == RES_STRING) || (pOp2->Type == RES_STRING) ||
                (pOp1->Type == RES_DATE)   || (pOp2->Type == RES_DATE) )
      {
        /****************************************************************/
        /* do a string compare as at least one of the operators is a    */
        /* string or a date value                                       */
        /* Do not compare if one of the operators is empty.             */
        /****************************************************************/
        if ( pOp1->pszValue[0] && pOp2->pszValue[0] )
        {
          lCompare = UTF16stricmp( pOp1->pszValue, pOp2->pszValue );
          switch ( lCompare )
          {
            case -1 :
              pCompRes->fSmaller = TRUE;
              break;
            case 0 :
              pCompRes->fEqual   = TRUE;
              break;
            default :
              pCompRes->fGreater = TRUE;
              break;
          } /* endswitch */
        }
        else if ( !pOp1->pszValue[0] && !pOp2->pszValue[0] )
        {
          /**************************************************************/
          /* two empty strings match always                             */
          /**************************************************************/
          pCompRes->fEqual = TRUE;
        } /* endif */
      }
      else if ( (pOp1->Type == RES_LONG) && (pOp2->Type == RES_LONG) )
      {
        /****************************************************************/
        /* do a long compare as both operands are long values           */
        /****************************************************************/
        lOp1 = _wtol( pOp1->pszValue );
        lOp2 = _wtol( pOp2->pszValue );
        if ( lOp1 < lOp2 )
        {
          pCompRes->fSmaller = TRUE;
        }
        else if ( lOp1 == lOp2 )
        {
          pCompRes->fEqual   = TRUE;
        }
        else
        {
          pCompRes->fGreater = TRUE;
        } /* endif */
      }
      else
      {
        /****************************************************************/
        /* do a double compare as both operands are numeric but at least*/
        /* one is in float format                                       */
        /****************************************************************/
        CHAR    chTemp[128];
        Unicode2ASCII(pOp1->pszValue, chTemp, 0L );
        dbOp1 = atof( chTemp );

        Unicode2ASCII(pOp2->pszValue, chTemp, 0L );
        dbOp2 = atof( chTemp);
        if ( dbOp1 < dbOp2 )
        {
          pCompRes->fSmaller = TRUE;
        }
        else if ( dbOp1 == dbOp2 )
        {
          pCompRes->fEqual   = TRUE;
        }
        else
        {
          pCompRes->fGreater = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function FiltCompareValues */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltMatchStrings    Perform pattern matching             |
//+----------------------------------------------------------------------------+
//|Function call:     FiltMatchStrings( pOp1, pOp2, &CompRes )                 |
//+----------------------------------------------------------------------------+
//|Description:       Check if string of first operand matches the             |
//|                   pattern of the second operand                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PRESULT      pOp1        first operand                   |
//|                   PRESULT      pOp2        second operand                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PCOMPRES     pCompRes    result of compare               |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE (always)                                            |
//+----------------------------------------------------------------------------+
BOOL FiltMatchStrings
(
  PRESULT      pOp1,                   // first operand
  PRESULT      pOp2,                   // second operand
  PCOMPRES     pCompRes                // result of compare
)
{
  BOOL       fOK;                      // function return code
  BOOL       fMatch;                   // match flag

  /********************************************************************/
  /* Initialize compare result                                        */
  /********************************************************************/
  pCompRes->fEqual    = TRUE;
  pCompRes->fSmaller  = FALSE;
  pCompRes->fGreater  = FALSE;

  fOK = UtlMatchStringsW( pOp1->pszValue, pOp2->pszValue, &fMatch );
  pCompRes->fEqual = fMatch;

  return( fOK );

} /* end of function FiltMatchStrings */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltScanResult      Scan result stack for a given type   |
//+----------------------------------------------------------------------------+
//|Function call:     FiltScanResult( pResStack, ResType, &pResult );          |
//+----------------------------------------------------------------------------+
//|Description:       Search result stack for a specific result type           |
//+----------------------------------------------------------------------------+
//|Input parameter:   PRESSTACK    pResStack   ptr to result stack             |
//|                   RESULTTYPE   ResType     type being looked for           |
//+----------------------------------------------------------------------------+
//|Output parameter:  PRESULT      *ppResult   found stack position            |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    stack entry has been found                       |
//|                   FALSE   stack contains no entry with the given type      |
//+----------------------------------------------------------------------------+
//|Function flow:     if stack has elements                                    |
//|                     position for last element in stack                     |
//|                   while ok and type of stack entry is not the requested one|
//|                     go to previous stack entry                             |
//|                   set ppResult value                                       |
//+----------------------------------------------------------------------------+
BOOL FiltScanResult
(
  PRESSTACK    pResStack,              // ptr to result stack
  RESULTTYPE   ResType,                // type being looked for
  PRESULT      *ppResult               // found stack position
)
{
  BOOL      fOK = TRUE;                // function return code
  PRESULT   pResult = NULL;                   // ptr to current stack entry

  if ( pResStack->pCurPos > pResStack->pStack )
  {
    pResult = pResStack->pCurPos - 1;
  }
  else
  {
    fOK = FALSE;                       // no elements on stack
  } /* endif */

  while ( fOK && (pResult->Type != ResType) )
  {
    if ( pResult == pResStack->pStack )
    {
      fOK = FALSE;
    }
    else
    {
      pResult--;
    } /* endif */
  } /* endwhile */

  *ppResult = ( fOK ) ? pResult : NULL;

  return( fOK );
} /* end of function FiltScanResult */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltString2Result   Convert a string to a result value   |
//+----------------------------------------------------------------------------+
//|Function call:     FiltString2Result( pCB, pszString, pResult );            |
//+----------------------------------------------------------------------------+
//|Description:       Converts a string to a result value and evaluates        |
//|                   the type of the string which can be date, long, double   |
//|                   or string.                                               |
//+----------------------------------------------------------------------------+
//|Input parameter:   PFILTERCB    pCB         pointer to filter control block |
//|                   PSZ          pszString   input string                    |
//+----------------------------------------------------------------------------+
//|Output parameter:  PRESULT      pResult     buffer for result               |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     if string starts with a sign                             |
//|                     skip sign, clear date flag                             |
//|                   loop over string until complete or double flag is off    |
//|                     if current character is a decimal point                |
//|                       if long flag is on                                   |
//|                         clear long flag                                    |
//|                       else                                                 |
//|                         clear double flag                                  |
//|                     elseif current character is no digit                   |
//|                       clear long and double flag                           |
//|                   if long flag is set                                      |
//|                     set result type to RES_LONG                            |
//|                   else if double flag is set                               |
//|                     set result flag to RES_DOUBLE                          |
//|                   if date flag is set and neither long nor double flag is  |
//|                     set                                                    |
//|                     check if value follows the general date format         |
//|                     if valid check if value matchs the current OS2 date    |
//|                       format and fill date field in result buffer          |
//|                     if date flag is set                                    |
//|                       set result type to RES_DATE                          |
//|                   if neither date nor long nor double flag is set          |
//|                     set result type to RES_STRING                          |
//+----------------------------------------------------------------------------+
VOID FiltString2Result
(
  PFILTERCB    pCB,                    // pointer to filter control block
  PSZ_W        pszString,              // input string
  PRESULT      pResult                 // buffer for result
)
{
  BOOL         fLong   = TRUE;         // TRUE = string may be a long value
  BOOL         fDouble = TRUE;         // TRUE = string may be a double value
  BOOL         fDate   = TRUE;         // TRUE = string may be a date value
  USHORT       usGroup;                // current group in date processing
  USHORT       usDate[3];              // buffer for date values
  USHORT       usDigits;               // number of digits processed in date
  CHAR_W       chSeper;                // seperator character of date data

  pResult->pszValue = pszString;       // remember start of string

  /********************************************************************/
  /* skip any preceding sign (+ or -)                                 */
  /********************************************************************/
  if ( (*pszString == PLUS_SIGN) || (*pszString == MINUS_SIGN))
  {
    pszString++;                       // skip sign
    fDate = FALSE;                     // string is definitely no date value
  } /* endif */

  /********************************************************************/
  /* loop through string until end of string or string may not be     */
  /* a double value anymore                                           */
  /********************************************************************/
  while ( *pszString && fDouble )
  {
    if ( *pszString == DECIMAL_POINT )
    {
      if ( fLong )
      {
        fLong = FALSE;                 // string contains a decimal point
                                       // therefore it is no LONG value
      }
      else
      {
        fDouble = FALSE;               // string contains more than one decimal
                                       // point therefore it is no DOUBLE value
      } /* endif */
    }
    else if ( (*pszString < '0') || (*pszString > '9') )
    {
      fLong   = FALSE;                 // string is neither a LONG nor
      fDouble = FALSE;                 // a double value
    } /* endif */
    pszString++;
  } /* endwhile */

  /********************************************************************/
  /* Handle LONG or DOUBLE values                                     */
  /********************************************************************/
  if ( fLong )
  {
    pResult->Type       = RES_LONG;
  }
  else if ( fDouble )
  {
    pResult->Type       = RES_DOUBLE;
  } /* endif */

  /********************************************************************/
  /* check for data value if fDate is still TRUE and value was        */
  /* neither LONG nor DOUBLE                                          */
  /********************************************************************/
  if ( fDate && !fLong && !fDouble )
  {
    /******************************************************************/
    /* check for general date format which is:                        */
    /* digit [digit] seperator digit [digit] seperator digit [digit]  */
    /******************************************************************/
    usGroup  = 0;
    usDate[0] = 0;
    usDate[1] = 0;
    usDate[2] = 0;
    usDigits = 0;
    chSeper  = EOS;
    pszString = pResult->pszValue;
    while ( *pszString && fDate )
    {
      if ( (*pszString >= '0') && (*pszString <= '9') )
      {
        if ( usDigits < 2 )
        {
          usDate[usGroup] = (usDate[usGroup] * 10) + (*pszString - '0');
          usDigits++;
        }
        else
        {
          fDate = FALSE;               // to much digits for date formats
        } /* endif */
      }
      else
      {
        if ( !usDigits || (usGroup > 1) )
        {
          fDate = FALSE;               // no digits or more than 2 seperators
        }
        else if ( chSeper && (chSeper != *pszString) )
        {
          fDate = FALSE;               // different seperators used
        }
        else
        {
          chSeper = *pszString;        // remember seperator character
          usGroup++;                   // skip to next date group
          usDigits = 0;                // no digits for this group yet
        } /* endif */
      } /* endif */
      pszString++;
    } /* endwhile */

    /******************************************************************/
    /* Check if processed date follows the date format described in   */
    /* the OS/2 configuration file OS2.INI                            */
    /******************************************************************/
    if ( fDate )
    {
      if ( chSeper != pCB->chDateSeper )
      {
        fDate = FALSE;
      }
      else
      {
        switch ( pCB->usDateFormat )
        {
          case YMD_DATE_FORMAT :
            if ( (usDate[1] == 0) || (usDate[1] > 12) ||
                 (usDate[2] == 0) || (usDate[2] > 31) )
            {
              fDate = FALSE;
            }
            else
            {
              pResult->dateValue.year  = usDate[0];
              pResult->dateValue.month = usDate[1];
              pResult->dateValue.day   = usDate[2];
            } /* endif */
            break;

          case MDY_DATE_FORMAT :
            if ( (usDate[0] == 0) || (usDate[0] > 12) ||
                 (usDate[1] == 0) || (usDate[1] > 31) )
            {
              fDate = FALSE;
            }
            else
            {
              pResult->dateValue.year  = usDate[2];
              pResult->dateValue.month = usDate[0];
              pResult->dateValue.day   = usDate[1];
            } /* endif */
            break;

          case DMY_DATE_FORMAT :
            if ( (usDate[1] == 0) || (usDate[1] > 12) ||
                 (usDate[0] == 0) || (usDate[0] > 31) )
            {
              fDate = FALSE;
            }
            else
            {
              pResult->dateValue.year  = usDate[2];
              pResult->dateValue.month = usDate[1];
              pResult->dateValue.day   = usDate[0];
            } /* endif */
            break;
        } /* endswitch */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Set result type for date data                                  */
    /******************************************************************/
    if ( fDate )
    {
      pResult->Type = RES_DATE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if data is neither date nor long nor double use it as a string   */
  /********************************************************************/
  if ( !fDate && !fDouble && !fLong )
  {
    pResult->Type = RES_STRING;
  } /* endif */

} /* end of function FiltString2Result */

