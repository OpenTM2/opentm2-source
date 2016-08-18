//+----------------------------------------------------------------------------+
//|EQFFILTI.H                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:         G. Queck (QSoft)                                            |
//+----------------------------------------------------------------------------+
//|Description:    Private Include file for filter functions                   |
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
// $Revision: 1.1 $ ----------- 31 Aug 2004
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
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFFILTI.HV_   1.0   09 Jan 1996 09:46:02   BUILD  $
 *
 * $Log:   J:\DATA\EQFFILTI.HV_  $
 *
 *    Rev 1.0   09 Jan 1996 09:46:02   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
#define OPERATOR_LEN      8    //total length an operator can be
#define NAME_LEN          60   //max. length of field name
#define TXT_SIZE          300  //max length of mri strings
#define UNDERSCORE        '_'  //underscore
#define BUFFER_SIZE       2048

#define AND_STR           "AND"
#define OR_STR            "OR"
#define NOT_STR           "NOT"
#define LIKE_STR          "LIKE"
#define BETWEEN_STR       "BETWEEN"
#define IN_STR            "IN"
#define MULT_SUB_STR      "*"
#define EQUAL             '='
#define SMALLER_THAN      '<'
#define BIGGER_THAN       '>'
#define OPEN_BRACKET      '('
#define CLOSED_BRACKET    ')'
#define COMMA             ','


typedef struct _STARTLEN
{
   USHORT    usStart;          //start position
   USHORT    usLen;            //length of area
} STARTLEN , *PSTARTLEN;

typedef struct _POLISHSTACK
{
   SHORT     sOperatorId;
   USHORT    usOffset;
} POLISHSTACK , *PPOLISHSTACK;

// Structure of EQF filter properties
typedef struct _FILTPROP
{
  CHAR        szDescription[MAX_DESCRIPTION]; //filter description
  CHAR        szFilterPath[MAX_EQF_PATH];     //fully qualified filter file name
  STARTLEN    FiltAllNames;
  STARTLEN    FiltSelectNames;
  STARTLEN    FilterMle;
  STARTLEN    FilterStack;
  STARTLEN    FilterBuffer;
} FILTPROP , *PFILTPROP;

//enumeration of filter operators
typedef enum _FILTOPERATORENUM
{
  OP_OPENBRACKET,
  OP_CLOSEDBRACKET,
  OP_AND,
  OP_OR,
  OP_NOT,
  OP_EQUAL,
  OP_NOTEQUAL,
  OP_SMALLER,
  OP_GREATER,
  OP_SMALLEREQUAL,
  OP_GREATEREQUAL,
  OP_LIKE,
  OP_BETWEEN,
  OP_IN,
  OP_STRING,
  OP_NUM,
  OP_FIELDNAME,
  OP_STARTOFLIST,
  OP_END
} FILTOPERATORENUM; /* end of enum */

typedef struct _FILTOPERATOR
{
     FILTOPERATORENUM  sOperator;      //enum value of operator
     CHAR szName[OPERATOR_LEN];        //length of the opeartor
} FILTOPERATOR, *PFILTOPERATOR;

/**********************************************************************/
/* In-memory representation of a filter                               */
/**********************************************************************/
typedef struct _FILTER
{
  FILTPROP         Prop;               // filter properties
  PUSHORT          pusAllNames;        // pointer to all names offset array
  USHORT           usAllNameSize;      // allocated size of all names array
  USHORT           usAllNameUsed;      // used size of all names array
  PUSHORT          pusSelNames;        // pointer to selected names offset array
  USHORT           usSelNameSize;      // allocated size of selected names array
  USHORT           usSelNameUsed;      // used size of selected names array
  PUCHAR           pucSelMLE;          // buffer for data of SELECT MLE
  USHORT           usSelMLESize;       // allocated size of SELECT MLE buffer
  USHORT           usSelMLEUsed;       // used size of SELECT MLE buffer
  PUCHAR           pucWhereMLE;        // buffer for data of WHERE MLE
  USHORT           usWhereMLESize;     // allocated size of WHERE MLE buffer
  USHORT           usWhereMLEUsed;     // used size of WHERE MLE buffer
  PPOLISHSTACK     pStack;             // operator stack
  USHORT           usStackSize;        // allocated size of operator stack
  USHORT           usStackUsed;        // used size of operator stack
  PUCHAR           pucBuffer;          // string buffer
  USHORT           usBufferSize;       // allocated size of buffer
  USHORT           usBufferUsed;       // used size of buffer
                                                                /* 5@KIT1007A */
  USHORT           usDicts;            // number of associated dictionaries
  HDCB             hDCB[MAX_DICTS];    // dictionary handles
  PPOLISHSTACK     pOpStack[MAX_DICTS];// operator stack per dictionary
  PUSHORT          pusSelected[MAX_DICTS];// selected names per dictionary
} FILTER, *PFILTER;

/**********************************************************************/
/* Structure containing size and position of individual filter        */
/* controls                                                           */
/**********************************************************************/
typedef struct _FILTTEMPL
{
  ULONG            ulGroupBoxStyle;    // style to be used for groupbox
  CHAR             szGroupBox[256];    // text to be used for groupbox
  SWP              swpCheckBox;        // size/pos of checkbox
  ULONG            ulCheckBoxStyle;    // style to be used for checkbox
  CHAR             szCheckBox[256];    // text to be used for checkbox
  SWP              swpEditPB;          // size/pos of edit pushbutton
  ULONG            ulEditPBStyle;      // style to be used for edit pushbutton
  CHAR             szEditPB[256];      // text to be used for edit pushbutton
  SWP              swpName;            // size/pos of name static text
  ULONG            ulNameStyle;        // style to be used for static text
  CHAR             szName[256];        // text to be used for name static text
  SWP              swpCombo;           // size/pos of combobox
  ULONG            ulComboStyle;       // style to be used for combobox
  SWP              swpDescr;           // size/pos of description static text
  ULONG            ulDescrStyle;       // style to be used for description static text
  CHAR             szDescr[256];       // text to be used for description static text
  SWP              swpDescrEF;         // size/pos of description entryfield
  ULONG            ulDescrEFStyle;     // style to be used for description entryfield
} FILTTEMPL, *PFILTTEMPL;

//Function Prototypes

MRESULT EXPENTRY FILTCONTROLWP( HWND, WINMSG, WPARAM, LPARAM );
USHORT           FiltLoadFilterHeader( PSZ, PFILTPROP );
USHORT           FiltRead( PSZ, PFILTER );
USHORT           FiltFree( PFILTER );
USHORT           FiltWrite( PSZ, PFILTER );
USHORT           FiltAddToBuffer( PSZ *, PULONG, PULONG, PSZ );

#define GETFILTERDESCR_TASK    USER_TASK + 20
#define GETFILTERTEMPL_TASK    USER_TASK + 21
