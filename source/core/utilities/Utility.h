//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define MAX_MEM_TABLES			8		// max. no of segment tables
#define MAX_MEM_SEGMENTS		8162	// max. no of segments for UtlAlloc
#define SEG_TABLE_EMPTY			(-1)	// there are no segments in the table

#define DATEFORMATSTRING_YMD	"%4.4d%c%2.2d%c%2.2d"
#define DATEFORMATSTRING_DMY	"%2.2d%c%2.2d%c%4.4d"
#define DATEFORMATSTRING_MDY	"%2.2d%c%2.2d%c%4.4d"
#define TIMEFORMATSTRING		"%2.2d%c%2.2d%c%2.2d"

// date display formats (must be in same order as OS2 setup formats)
typedef enum _DATEDISPLAYFORMAT
{
   MDY_DATE_FORMAT,                    // date format MM DD YY
   DMY_DATE_FORMAT,                    // date format DD MM YY
   YMD_DATE_FORMAT                     // date format YY MM DD
} DATEDISPLAYFORMAT;

// time display formats (must be in same order as OS2 setup formats)
typedef enum _TIMEDISPLAYFORMAT        // time display formats
{
   S_12_HOURS_TIME_FORMAT,             // time 0..11 format
   S_24_HOURS_TIME_FORMAT              // time 0..23 format
} TIMEDISPLAYFORMAT;

typedef struct _SEGENTRY               // structure for segment entry
{
   PBYTE    pSel;                      // pointer to segment
   USHORT   usNoOfBlocks;              // number of memory blocks in segment
} SEGENTRY, *PSEGENTRY;

typedef struct _SEGTABLE               // structure for segment tables
{
   PSEGENTRY pSegment;                 //  pointer to table of segments
   SHORT    sMaxSel;                   // table size
   USHORT   usMaxBlockSize;            // max. memory block size
   USHORT   usMinBlockSize;            // min. memory block size
   SHORT    sSearchSel;                // where to start search in segment table
} SEGTABLE, *PSEGTABLE;

typedef struct _SUBALLOC_HDR
{
  USHORT usSizeOfBlock;
  USHORT usNumOfBlocks;
  USHORT usLastUsed;
} SUBALLOC_HDR, *PSUBALLOC_HDR;

typedef struct _UTIVARS                // structure for static variables of UTI
{
   USHORT   usTask;                    // task id..
   SEGTABLE SegTable[MAX_MEM_TABLES];  // table of segment tables
   ULONG    ulQueryArea[QL_LAST+1];    // area for UtlQueryULong/UtlSetULong
   USHORT   usQueryArea[QS_LAST+1];    // area for UtlQueryUShort/UtlSetUSchort
   PSZ      pszQueryArea[QST_LAST+1];  // area for UtlQueryString/UtlSetString
   //--- area for NLS date and time info obtained from OS2.INI file ---
   USHORT   usDateFormat;              // date format for display
   CHAR     chDateSeperator;           // character used as seper in date
   USHORT   usTimeFormat;              // time format for display
   CHAR     chTimeSeperator;           // character used as seper in time
   CHAR     szTime1159[20];            // suffix for times up to 11:59
   CHAR     szTime2359[20];            // suffix for times after 11:59
   SHORT    sDriveType[26];            // buffer for drive types
   FILE     *hfUtilsLog;              // handle of log file for this session (if any)
#if defined(UTL_SIMALLOCERROR)
   HSYSSEM  hMemAllocSem;              // system semaphore to control allocation
                                       // of memory
#endif
} UTIVARS;

#ifdef STATIC_OWNER
__declspec(dllexport)
UTIVARS UtiVar[ MAX_TASK + 1 ] =       // static variables of module UTI
{
 {
   0,                                  // task id..
  { {  NULL, SEG_TABLE_EMPTY,     16,    16,    0 },
    {  NULL, SEG_TABLE_EMPTY,     64,    64,    0 },
    {  NULL, SEG_TABLE_EMPTY,    128,   128,    0 },
    {  NULL, SEG_TABLE_EMPTY,    256,   256,    0 },
    {  NULL, SEG_TABLE_EMPTY,    512,   512,    0 },
    {  NULL, SEG_TABLE_EMPTY,   1020,  1020,    0 },
    {  NULL, SEG_TABLE_EMPTY,   2040,  2040,    0 },
    {  NULL, SEG_TABLE_EMPTY, 0xFFFF,     1,    0 } },
  { 0L },
  { 0 },
  { NULL },
  0,
  0,
  0,
#ifdef UTL_SIMALLOCERROR
  0,
#endif
},
};

__declspec(dllexport)
UCHAR  chEQFLower[ 257 ];       // lower case table
__declspec(dllexport)
UCHAR  chEQFUpper[ 257 ];       // upper case table

#else
__declspec(dllimport)
extern UTIVARS UtiVar[ MAX_TASK + 1 ];
__declspec(dllimport)
extern UCHAR  chEQFLower[ 257 ];       // lower case table
__declspec(dllimport)
extern UCHAR  chEQFUpper[ 257 ];       // upper case table
#endif

ULONG Unicode2ASCIIBufEx( PSZ_W pszUni, PSZ pszASCII, ULONG usLen, LONG lBufLen,
                          ULONG ulCP, PLONG plRc );
VOID UtlLowUpInit( );

