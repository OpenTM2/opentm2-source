//+----------------------------------------------------------------------------+
//|EQFPARSE.H                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:   R.Jornitz,  QSoft GmbH                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:   Definitions for generic parser routines                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                            |
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
//+----------------------------------------------------------------------------+

//+----------------------------------------------------------------------------+
//| General constants                                                          |
//+----------------------------------------------------------------------------+
#define EOS                '\0'        // 'C' end-of-string character
#define END_OF_FILE        '\x1A'      // end of file delimiter
#define MAX_SEG_SIZE       1700        // size of segment buffer
#define MAX_OUTBUF_SIZE    8192        // size of output buffer
#define INBUF_SIZE         8192        // size of input buffer
#define EOF_REACHED        0xFF01      // end-of-file return code
#define STRING_DELIMITER   '\"'        // string delimiting character
#define STRING_DELIMITER_W  L'\"'        // string delimiting character
#define QUOTE_DELIMITER    '\''        // quote delimiting character
#define QUOTE_DELIMITER_W   L'\''        // quote delimiting character
#define START_COMMENT      '/'         // comment start character
#define BLOCK_COMMENT      '*'         // block comment identifier
#define LINE_COMMENT       '/'         // line comment identifier
#define CR                 '\r'        // carriage return
#define CR_W               L'\r'        // carriage return

#define LF                 '\n'        // linefeed
#define LF_W                 L'\n'        // linefeed
#define BACKSLASH          '\\'        // backslash character
#define BLANK              ' '         // blank character
#define BLANK_W            L' '
#define BLANKSUBST         0xff        // blank substitution character
#define BLANKSUBST_W       0x00A0
//OLD til 04/12/15/P020990: #define CONTINUE_CHAR      '*'         // continuation character
#define CONTINUE_CHAR      'X'         // continuation character
#define CONTINUE_CHAR_W    L'X'         // continuation character
#define COMMA_CHAR         ','         // indicator that another attribute fol.
#define COMMA_CHAR_W       L','         // indicator that another attribute fol.
#define MSG_COMMENT        '#'         // Comment for IBMXLMSG table
#define ASM_CONTINUE       72          // continuation position for asm...
#define ASM_STARTCONT      15          // prefix of continuation line

//+----------------------------------------------------------------------------+
//| Utl-to-Dos macros (to allow usage of parser without utilities)             |
//+----------------------------------------------------------------------------+
//#define UtlWrite( f, p, l, w, b )                 DosWrite( f, p, l, w )
//#define UtlOpen( n, f, o, l, ft, fa, fm, res, b ) DosOpen( n, f, o, l, ft, fa, fm, res )
//#define UtlQFileInfo( f, t, p, l, b )             DosQFileInfo( f, t, p, l )
//#define UtlClose( f, b )                          DosClose( f )
//#define UtlRead( f, p, l, lr, b )                 DosRead( f, p, l, lr )

//+----------------------------------------------------------------------------+
//| Types of output segments created by the parser                             |
//+----------------------------------------------------------------------------+
typedef enum _SEGTYPES
{
   QFF_SEGMENT,                        // QFF for strings
   QFN_SEGMENT                         // QFN for all other stuff
} SEGTYPES;

//+----------------------------------------------------------------------------+
//| Processing modes of parser                                                 |
//+----------------------------------------------------------------------------+
typedef enum _PROCESSMODES
{
   COMMENT_START_MODE,                 // processing start of a comment
   LINE_COMMENT_MODE,                  // processing data inside a line comment
   BLOCK_COMMENT_MODE,                 // processing data in a block comment
   COMMENT_END_MODE,                   // processing end of a block comment
   STRING_MODE,                        // processing string data
   ESCAPE_CHAR_MODE,                   // processing escape sequence in a string
   OUTSIDE_STRING_MODE,                // processing data outside of strings
   COMMENT_MODE                        // processing start of comment(IBMXLMSG)
} PROCESSMODES;

//+----------------------------------------------------------------------------+
//| Structure for global data of parser                                        |
//+----------------------------------------------------------------------------+
typedef struct _PARSEDATA
{
   USHORT      Mode;                   // current processing mode
   HFILE       hInFile;                // handle of input file
   CHAR        abInBuf[INBUF_SIZE];    // buffer for input data
   LONG        lBytesToRead;           // number of bytes to read from input
   USHORT      usBytesInBuffer;        // number of bytes in input buffer
   PCHAR       pInBuf;                 // ptr for input buffer processing
   HFILE       hOutFile;               // handle of output file
   CHAR        abOutBuf[MAX_OUTBUF_SIZE]; // buffer for output data
   PSZ         pQFNTag;                // ptr to QFN tag name
   PSZ         pEQFNTag;               // ptr to EQFN tag name
   CHAR        abTempBuf[MAX_OUTBUF_SIZE];// buffer for temporary output data
   PSZ         pQFFTag;                // ptr to QFF tag name
   PSZ         pEQFFTag;               // ptr to EQFF tag name
   USHORT      usOutBufUsed;           // number of bytes used in output buffer
   PCHAR       pOutBuf;                // ptr for output buffer processing
   USHORT      usSegType;              // type of currently active segment
   ULONG       ulSegNum;               // number of segments processed
   USHORT      usLinePos;              // position in line
   LONG        lTotalBytes;            // total file size in bytes
   USHORT      usPercentComplete;      // current completion ratio
   HWND        hwndSlider;             // handle of analysis slider control
   USHORT      usOutPos;               // position in output line
   PEQF_BOOL   pfKill;                 // ptr to 'kill running process' flag
   USHORT      usUndoBufUsed;          // used bytes in undo buffer
   CHAR        abUndoBuf[MAX_SEG_SIZE];// buffer for undo characters
   HFILE       hSrcFile;               // file handle of opened source file
   PVOID       pPrivateData;           // ptr. to private data struct
   CHAR        szDocName[MAX_LONGFILESPEC]; // buffer for document name
   CHAR        szLineBuf[4095];        // buffer for input line
   CHAR        szTempLine[4096];       // buffer for temporary input line
   PSZ         pTermList;              // pointer to term list
   USHORT      usTermListSize;         // current term list size
   CHAR        szFolder[MAX_FNAME];    // folder of document
   CHAR        szLanguage[MAX_LANG_LENGTH]; // source language of document
   SHORT       sLangID;                // language ID for morph services
   PCHAR       pConvTable;             // ptr to code conversion table
   ULONG       ulSrcOemCP;
   ULONG       ulTgtOemCP;
   ULONG       ulSrcAnsiCP;
   ULONG       ulTgtAnsiCP;
} PARSEDATA, *PPARSEDATA;

typedef struct _PARSEDATAW
{
   USHORT      Mode;                   // current processing mode
   HFILE       hInFile;                // handle of input file
   CHAR_W      abInBuf[INBUF_SIZE];    // buffer for input data
   LONG        lBytesToRead;           // number of bytes to read from input
   ULONG       ulBytesInBuffer;        // number of bytes in input buffer
   PCHAR_W     pInBuf;                 // ptr for input buffer processing
   HFILE       hOutFile;               // handle of output file
   CHAR_W      abOutBuf[MAX_OUTBUF_SIZE]; // buffer for output data
   PSZ_W       pQFNTag;                // ptr to QFN tag name
   PSZ_W       pEQFNTag;               // ptr to EQFN tag name
   CHAR_W      abTempBuf[MAX_OUTBUF_SIZE];// buffer for temporary output data
   PSZ_W       pQFFTag;                // ptr to QFF tag name
   PSZ_W       pEQFFTag;               // ptr to EQFF tag name
   ULONG       ulOutBufUsed;           // number of bytes used in output buffer
   PCHAR_W     pOutBuf;                // ptr for output buffer processing
   USHORT      usSegType;              // type of currently active segment
   ULONG       ulSegNum;               // number of segments processed
   ULONG       ulLinePos;              // position in line
   LONG        lTotalBytes;            // total file size in bytes
   USHORT      usPercentComplete;      // current completion ratio
   HWND        hwndSlider;             // handle of analysis slider control
   ULONG       ulOutPos;               // position in output line
   PEQF_BOOL   pfKill;                 // ptr to 'kill running process' flag
   ULONG       ulUndoBufUsed;          // used bytes in undo buffer
   CHAR_W      abUndoBuf[MAX_SEG_SIZE];// buffer for undo characters
   HFILE       hSrcFile;               // file handle of opened source file
   PVOID       pPrivateData;           // ptr. to private data struct
   CHAR        szDocName[MAX_LONGFILESPEC]; // buffer for document name
   CHAR_W      szLineBuf[4095];        // buffer for input line
   CHAR_W      szTempLine[4096];       // buffer for temporary input line
   PSZ         pTermList;              // pointer to term list
   USHORT      usTermListSize;         // current term list size
   CHAR        szFolder[MAX_FNAME];    // folder of document
   CHAR        szLanguage[MAX_LANG_LENGTH]; // source language of document
   SHORT       sLangID;                // language ID for morph services
   PCHAR       pConvTable;             // ptr to code conversion table
   ULONG       ulSrcOemCP;
   ULONG       ulTgtOemCP;
   ULONG       ulSrcAnsiCP;
   ULONG       ulTgtAnsiCP;
} PARSEDATAW, *PPARSEDATAW;

//+----------------------------------------------------------------------------+
//| Prototypes for internal functions                                          |
//+----------------------------------------------------------------------------+
USHORT StartSegment ( PPARSEDATA pParsData );
USHORT AddToSegment ( PPARSEDATA pParsData, BYTE bAddByte );
BYTE   ParseNextChar ( PPARSEDATA pParsData, PUSHORT pusRC );
VOID   UndoNextChar ( PPARSEDATA pParsData, BYTE bChar );
USHORT EndSegment ( PPARSEDATA pParsData );
USHORT AddToFile ( PPARSEDATA pParsData, BYTE bAddByte );
USHORT WriteBuffer ( PPARSEDATA pParsData );
VOID   ParseFreeDoc( PVOID * );
VOID   ParseInit ( PPARSEDATA, HWND, PEQF_BOOL );
USHORT ParseClose ( PPARSEDATA );
BYTE   ConvertAscBin ( BYTE, BYTE );
PSZ    ConvertBinAsc ( BYTE, PSZ );
USHORT AddCharsToFile ( PPARSEDATA, PSZ, USHORT );
USHORT AddCharsToFileL ( PPARSEDATA, PSZ, ULONG );
USHORT AddStringToFile ( PPARSEDATA, PSZ );
USHORT AddIgnoredOnes ( PPARSEDATA, PSZ );
VOID ParseDeleteSegment ( PVOID, ULONG );
VOID ParseGetDocName( PSZ pszInFile, PSZ pszDocName );
USHORT ParseNextLine ( PPARSEDATA pParsData, PSZ pszBuffer, USHORT usBufSize );
USHORT ParseInitMorphSupport( PSZ pszDocPath, PSHORT psLangID );
USHORT ParseFillCP( PPARSEDATA pParsData, PSZ pszSource);
USHORT ParseGetCP ( PSZ  pszInFile, PULONG pulSrcOemCP, PULONG pulTgtOemCP,
                     PULONG pulSrcAnsiCP, PULONG pulTgtAnsiCP );

USHORT StartSegmentW ( PPARSEDATAW pParsDataW );
USHORT AddToSegmentW ( PPARSEDATAW pParsDataW, CHAR_W chAddCharW );
CHAR_W  ParseNextCharW ( PPARSEDATAW pParsDataW, PUSHORT pusRC );
VOID   UndoNextCharW ( PPARSEDATAW pParsDataW, CHAR_W chCharW );
USHORT EndSegmentW ( PPARSEDATAW pParsDataW );
USHORT AddToFileW ( PPARSEDATAW pParsDataW, CHAR_W  chCharW );
USHORT WriteBufferW ( PPARSEDATAW pParsDataW );
VOID   ParseInitW ( PPARSEDATAW, HWND, PEQF_BOOL );
USHORT ParseCloseW ( PPARSEDATAW );
USHORT AddCharsToFileW ( PPARSEDATAW, PSZ_W, ULONG );
USHORT AddStringToFileW ( PPARSEDATAW, PSZ_W );
USHORT ParseFillCPW( PPARSEDATAW pParsDataW, PSZ pszSource);

CHAR_W ParseNextCharWEx( PPARSEDATAW pParsDataW, PUSHORT pusRC, PBOOL pfIsDBCS);
VOID   UndoNextCharWEx ( PPARSEDATAW pParsDataW, CHAR_W chCharW, PBOOL  pfIsDBCS );
