/*! \file
	Description: Internal include file for the QDPR dictionary print code
	
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

/**********************************************************************/
/*                        various definitions                         */
/**********************************************************************/
#define QDPR_CR                   CR
#define QDPR_LF                   LF
#define QDPR_EOF                  EOFCHAR
#define QDPR_BLANK                BLANK
#define QDPR_EMPTY_STRING         ""
#define QDPR_BACKSLASH            BACKSLASH
#define QDPR_PARENT_DIR           PARENT_DIR_NAME
#define QDPR_PROCESS_CLASS "QDPRProcess"        // QDPR window class

#define QDPR_MAX_ALLOC_SIZE       0xFFF0         // maximum size of
                                                 // storage that can be
                                                 // allocated using UtlAlloc

#define QDPR_MAX_DICTNAME         MAX_FNAME      // maximum length of
                                                 // a dictionary name
#define QDPR_MAX_PRTS             10             // maximum number of
                                                 // printers available
#define QDPR_MAX_PRTNAME          20             // maximum length of
                                                 // a printer name
#define QDPR_MAX_FILENAME         MAX_FILESPEC   // maximum length of
                                                 // a filename
#define QDPR_MAX_PATH_FILENAME    MAX_PATH144    // maximum length of
                                                 // a fully qualified
                                                 // file and path name
#define QDPR_MAX_FILTER           MAX_FNAME      // maximum length of
                                                 // a filter
#define QDPR_MAX_DESCRIPTION      60             // maximum length of
                                                 // the displayable
                                                 // text in the
                                                 // <description> tag
#define QDPR_MAX_FCRT_ELEMENTS    100            // maximum numbers of
                                                 // elements in a FCRT
#define QDPR_MAX_PROCESS_BUFFER   4096           // maximum buffer size
#define QDPR_MAX_STRING           256            // maximum string length

#define QDPR_MAX_INTERN_BUFFER    4096           // internal buffer size

#define QDPR_MAX_LINE_LENGTH      255            // maximum line length
#define QDPR_MAX_PAGE_LENGTH      200            // maximum page length
#define QDPR_DEF_PAGE_LENGTH      72             // default page
                                                 // length
#define QDPR_DEF_LINE_LENGTH      80             // default line
                                                 // length
#define QDPR_DEF_PAGE_NO          1L             // default start page
                                                 // number
#define QDPR_DEF_PAGE_EJECT       ""             // default page eject
                                                 // string

#define QDPR_THREAD_STACK         8096           // size of thread's
                                                 // stack

#define QDPR_FILE_KEEP            10L            // if file size after an
                                                 // error is smaller than this
                                                 // the file will be deleted

//!!!!!!!!!!!!!!!!!!!!! check with other include files !!!!!!!!!!!!!!!!
#define QDPR_TIMER                99             // id of QDPR timer
#define QDPR_TIMER_DELAY          150L           // QDPR timer delay
#define QDPR_THREAD_SLEEP         22L            // QDPR thread sleep delay

/**********************************************************************/
/*                 definitions for the QDPR tag table                 */
/**********************************************************************/
#define QDPR_END_TAG_STR          ">"            // end-tag character string
#define QDPR_END_TAG_CHAR         '>'            // end-tag character
#define QDPR_EQUAL_CHAR           '='            // equal character
#define QDPR_QUOTE_CHAR           '\''           // quote charcater
typedef enum _QDPRTAGENUM
{
  /********************************************************************/
  /*       definitions corresponding to QDPRTAGS.IBL tag table        */
  /********************************************************************/
  QDPR_COMMENT_ETOKEN,                // </comment>
  QDPR_DESCRIPTION_ETOKEN,            // </description>
  QDPR_DICTBACK_ETOKEN,               // </dictback>
  QDPR_DICTFRONT_ETOKEN,              // </dictfront>
  QDPR_ENTRY_ETOKEN,                  // </entry>
  QDPR_PAGEFOOT_ETOKEN,               // </pagefoot>
  QDPR_PAGEHEAD_ETOKEN,               // </pagehead>
  QDPR_REPEAT_ETOKEN,                 // </repeat>
  QDPR_COMMENT_TOKEN,                 // <comment>
  QDPR_DESCRIPTION_TOKEN,             // <description>
  QDPR_DICTBACK_TOKEN,                // <dictback>
  QDPR_DICTFRONT_TOKEN,               // <dictfront>
  QDPR_ENTRY_TOKEN,                   // <entry>
  QDPR_PAGEFOOT_TOKEN,                // <pagefoot>
  QDPR_PAGEHEAD_TOKEN,                // <pagehead>
  QDPR_REPEAT_TOKEN,                  // <repeat>
  QDPR_SET_TOKEN,                     // <set>
  QDPR_VAR_TOKEN,                     // <var>
  QDPR_FIRST_ON_PAGE_ATTR,            // $FIRST_ON_PAGE
  QDPR_LAST_ON_PAGE_ATTR,             // $LAST_ON_PAGE
  QDPR_LEFT_ATTR,                     // $LEFT
  QDPR_NO_DISPLAY_ATTR,               // $NO_DISPLAY
  QDPR_RIGHT_ATTR,                    // $RIGHT
  QDPR_SAME_ENTRY_AGAIN_ATTR,         // $SAME_ENTRY_AGAIN
  QDPR_LEVEL_ENTRY_ATTR,              // level=ENTRY
  QDPR_LEVEL_HOM_ATTR,                // level=HOM
  QDPR_LEVEL_SENSE_ATTR,              // level=SENSE
  QDPR_LEVEL_TARGET_ATTR,             // level=TARGET
  QDPR_SYSNAME_DATE_ATTR,             // sysname=$DATE
  QDPR_SYSNAME_DICTNAME_ATTR,         // sysname=$DICTNAME
  QDPR_SYSNAME_FILENAME_ATTR,         // sysname=$FILENAME
  QDPR_SYSNAME_LINE_LENGTH_ATTR,      // sysname=$LINE_LENGTH
  QDPR_SYSNAME_PAGE_EJECT_ATTR,       // sysname=$PAGE_EJECT
  QDPR_SYSNAME_PAGE_LENGTH_ATTR,      // sysname=$PAGE_LENGTH
  QDPR_SYSNAME_PAGE_NO_ATTR,          // sysname=$PAGE_NO
  QDPR_SYSNAME_TIME_ATTR,             // sysname=$TIME
  QDPR_FORMAT_ATTR,                   // format=
  QDPR_NAME_ATTR,                     // name=
  QDPR_VALUE_ATTR,                    // value=
  QDPR_MAX_ATTR,                      // $MAX=
  QDPR_MIN_ATTR,                      // $MIN=

  /********************************************************************/
  /*                          own definitions                         */
  /********************************************************************/
  QDPR_REPTAG_FIELD,                  // "\0f"
  QDPR_REPTAG_REPEAT,                 // "\0r"
  QDPR_REPTAG_END_REPEAT,             // "\0/r"
  QDPR_REPTAG_SYSVAR,                 // "\0$"
  QDPR_ID_CRLF,                       // LF or CRLF
  QDPR_NO_VALID_CHAR,                 // none of allowed chars
  QDPR_NO_QDPR_TAG                    // none of above tags
} QDPRTAGENUM; /* end of enum */

/**********************************************************************/
/*                      tag replacement strings                       */
/**********************************************************************/
#define QDPR_REP_FIELD_TAG             "f"      // replacement for
                                                // <field> tag
#define QDPR_REP_REPEAT_TAG            "r"      // replacement for
                                                // <repeat> tag
#define QDPR_REP_END_REPEAT_TAG        "/r"     // replacement for
                                                // </repeat> tag
#define QDPR_REP_SYSVAR                "$"      // replacement for
                                                // system variable

/**********************************************************************/
/*                 field attributes bit combinations                  */
/**********************************************************************/
#define QDPR_ATTR_FIRSTONPAGE          0x02     // 00000010
#define QDPR_ATTR_LASTONPAGE           0x03     // 00000011
#define QDPR_ATTR_MIN                  0x04     // 00000100
#define QDPR_ATTR_MAX                  0x08     // 00001000
#define QDPR_ATTR_LEFT                 0x20     // 00100000
#define QDPR_ATTR_RIGHT                0x30     // 00110000
#define QDPR_ATTR_NODISPLAY            0x40     // 01000000
#define QDPR_ATTR_SAMEENTRYAGAIN       0x80     // 10000000
#define QDPR_NO_QDPR_ATTR              0xFF     // 11111111

/**********************************************************************/
/*                format definitions for time and date                */
/**********************************************************************/
#define QDPR_DATE_UK_FULL_FORMAT       "mm/dd/yyyy"
#define QDPR_DATE_UK_FORMAT            "mm/dd/yy"
#define QDPR_DATE_US_FULL_FORMAT       "yyyy/mm/dd"
#define QDPR_DATE_US_FORMAT            "yy/mm/dd"
#define QDPR_DATE_EU_FULL_FORMAT       "dd.mm.yyyy"
#define QDPR_DATE_EU_FORMAT            "dd.mm.yy"
#define QDPR_DATE_FULL_FORMAT_LEN      10
#define QDPR_DATE_FORMAT_LEN           8
#define QDPR_TIME_NORM_FORMAT          "hh:mm:ss"
#define QDPR_TIME_NORM_FORMAT_LEN      8
#define QDPR_TIME_NORM_NSEC_FORMAT     "hh:mm"
#define QDPR_TIME_NORM_NSEC_FORMAT_LEN 5
#define QDPR_TIME_PM_FORMAT            "hh:mm:ssPM"
#define QDPR_TIME_PM_FORMAT_LEN        10
#define QDPR_TIME_PM_NSEC_FORMAT       "hh:mmPM"
#define QDPR_TIME_PM_NSEC_FORMAT_LEN   7

#define QDPR_NO_FORMAT                 0
#define QDPR_FORMAT_DATE_UK_FULL       1        // "mm/dd/yyyy"
#define QDPR_FORMAT_DATE_UK            2        // "mm/dd/yy"
#define QDPR_FORMAT_DATE_US_FULL       3        // "yyyy/mm/dd"
#define QDPR_FORMAT_DATE_US            4        // "yy/mm/dd"
#define QDPR_FORMAT_DATE_EU_FULL       5        // "dd.mm.yyyy"
#define QDPR_FORMAT_DATE_EU            6        // "dd.mm.yy"
#define QDPR_FORMAT_TIME_NORM          7        // "hh:mm:ss"
#define QDPR_FORMAT_TIME_NORM_NSEC     8        // "hh:mm"
#define QDPR_FORMAT_TIME_PM            9        // "hh:mm:ssPM"
#define QDPR_FORMAT_TIME_PM_NSEC       10       // "hh:mmPM"

/**********************************************************************/
/*               analyze and print process identifiers                */
/**********************************************************************/
typedef enum _QDPR_STATUS_IDS
{
  QDPR_THREAD_WAIT,                      // thread waits for work
  QDPR_ANAST_START,                      // analyze start
  QDPR_ANAST_PROCESS_FILE,               // process the loaded
                                         // format information file
  QDPR_ANAST_FINISHED,                   // stop analyze process
  QDPR_PRTST_START,                      // start print process
  QDPR_PRTST_PRINT_HEADER,               // print the header buffer
  QDPR_PRTST_FILTER,                     // filter the current entry
  QDPR_PRTST_START_PAGEHEAD,             // start process of pagehead buffer
  QDPR_PRTST_PRINT_PAGEHEAD,             // print the pagehead buffer
  QDPR_PRTST_START_ENTRY,                // start process of entry buffer
  QDPR_PRTST_PRINT_ENTRY,                // print the entry buffer
  QDPR_PRTST_START_PAGEFOOT,             // start process of pagefoot buffer
  QDPR_PRTST_PRINT_PAGEFOOT,             // print the pagefoot buffer
  QDPR_PRTST_PRINT_TRAILER,              // print the trailer buffer
  QDPR_PRTST_FINISHED,                   // stop print process
  QDPR_THREAD_FINISHED                   // thread finished all work
} QDPR_STATUS_IDS; /* end of enum */

/**********************************************************************/
/*                    filtering result identifiers                    */
/**********************************************************************/
#define QDPR_NO_FILTER_MATCH            0
#define QDPR_FILTER_MATCH               1
#define QDPR_NO_MORE_ENTRIES            2

/**********************************************************************/
/*                          QDPR error codes                          */
/**********************************************************************/
typedef enum _QDPR_ERRORS
{
  QDPR_NO_ERROR,             // everything ok
  QDPR_NO_MEMORY,            // not enough memory is left to allocate
                             // requested storage
  QDPR_PATH_NOT_FOUND,       // path could not be found
  QDPR_SYNTAX_ERROR,         // a syntax error is found in the format info file
                             // check special function return code
                             // (IDs are in EQFMSG.H)
  QDPR_WRONG_FIELDNAME,      // the fieldname given in a tag does not
                             // exist in the dictionary maptable
  QDPR_RESDLL_FAILURE,       // failure in loading the ressource DLL
  QDPR_OPEN_PROPFILE,        // error opening the property file
  QDPR_USER_ERROR,           // user made an error in the dialogs
  QDPR_FILLING_CONTROLS,     // error filling the controls in a dialog
  QDPR_CREATING_PROCWND,     // error when creating the process window
  QDPR_BUFFER_FULL,          // buffer is full
  QDPR_BUFFER_AT_END,        // buffer is at its end
  QDPR_PROGRAM_ERROR,        // program error which should not occur
  QDPR_NOMSG_ERROR          // internal processing error - gives no message
} QDPR_ERRORS; /* end of enum */

/**********************************************************************/
/*                          data structures                           */
/**********************************************************************/
//------------- input/output structure -------------------------------//
typedef struct _QDPR_IN_OUTPUT
{
  OBJNAME               szObjName;                          // Print object name
  CHAR                  szDictName[MAX_LONGFILESPEC];       // Dictionary name
  CHAR                  szDictShortName[MAX_FILESPEC];      // Dictionary name (short)
  CHAR                  szFormatFile[QDPR_MAX_FILENAME];    // Format info file
                                                            // name
  CHAR                  szFilter[QDPR_MAX_FILTER];          // Filter name
  CHAR                  szPrintDest[MAX_LONGPATH];          // Print destination
                                                            // either printer or
                                                            // file name
  BOOL                  fPrinterDest;                       // TRUE if printer is
                                                            // print destination
  BOOL                  fReplaceFile;                       // TRUE if print file
                                                            // is to be replaced
                                                            // or if printer is
                                                            // selected
  BOOL                  fStartPrinting;                     // TRUE if printing
                                                            // shall be started
  USHORT                usRC;                               // Returncode form
                                                            // dialogs
  HUCB                  hUCB;                               // Handle for Asd
                                                            // processing
  HDCB                  hDCB;                               // Dictionary Asd
                                                            // handle
  PSZ                   pszFormatFile;                      // Pointer to start
                                                            // of buffer area
                                                            // where format file
                                                            // is being loaded
  PTOKENENTRY           pTokFormatFile;                     // Pointer to
                                                            // tokenized
                                                            // format info file
                                                                /* 2@KIT1084A */
  BOOL                  fDictHasBeenLocked;                 // dict has been locked flag
} QDPR_IN_OUTPUT, *PQDPR_IN_OUTPUT;

//------------- dialog last-used values structure --------------------//
typedef struct _QDPR_LAST_USED_VALS
{
  BOOL                  fPrintDest;                        // TRUE if printer
                                                           // radiobutton was
                                                           // last selected
  CHAR                  szFormatFile[QDPR_MAX_FILENAME];   // Last-used format
                                                           // info filename
  CHAR                  szFilter[QDPR_MAX_FILTER];         // Last-used filter
  CHAR                  szFileName[QDPR_MAX_PATH_FILENAME];// Last-used filename
} QDPR_LAST_USED_VALS, *PQDPR_LAST_USED_VALS;

//------------- properties structure ---------------------------------//
typedef struct _QDPR_PROPS
{
  QDPR_LAST_USED_VALS   sctLastUsedValues;                 // Last-used values
                                                           // (see above)
} QDPR_PROPS, *PQDPR_PROPS;

//------------- dialogs IDA structure --------------------------------//
typedef struct _QDPR_DLG_IDA
{
  PQDPR_IN_OUTPUT       psctInOutput;                      // Input/output struct
                                                           // (see above)
  PPROPDICTIONARY       pDictProp;                         // Ptr to dictionary
                                                           // properties
  CHAR                  szDriveList[QDPR_MAX_STRING];      // Valid drive letters
  PSZ                   *apszFormatFilenames;              // Pointer array
                                                           // of pointers to
                                                           // strings containing
                                                           // format info file
                                                           // names
  PSZ                   pszFormatFilenames;                // Pointer to array of
                                                           // strings containing
                                                           // format info file
                                                           // names
  CHAR                  szWorkBuffer[QDPR_MAX_STRING];     // Work buffer
  USHORT                usFormatFileLen;                   // len of formfile
} QDPR_DLG_IDA, *PQDPR_DLG_IDA;

//------------- repeat stack structure -------------------------------//
typedef struct _QDPR_REPEAT_STACK
{
  PSZ_W                        *ppszFieldData;      // Pointer to field
                                                    // data ptr in template
  PCHAR                        pchrRepeatStart;     // Pointer to buffer
                                                    // location where
                                                    // repeat tag starts
  struct _QDPR_REPEAT_STACK    *psctPrev;           // Pointer to prev
                                                    // stack element
  PSZ_W                         pszOldFieldData;     // Pointer to stored
                                                    // data when repeat
                                                    // loop was started
  USHORT                       usRepeatLevelID;     // Tag ID indicating
                                                    // if a whole level
                                                    // is being compared
                                                    // or only the field
  PQLDB_HTREE                  pOldTemplate;        // if whole levels are
                                                    // being compared
                                                    // this template contains
                                                    // the data on entry of
                                                    // the repeat loop
} QDPR_REPEAT_STACK, *PQDPR_REPEAT_STACK;

//------------- field attributes structure ---------------------------//
typedef struct _QDPR_ATTRIBS
{
  UCHAR                 bitAttribs;      // Attribute bits
  /********************************************************************/
  /*                                                                  */
  /* bitAttribs =  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0            */
  /*                                                                  */
  /* Explanation of the bits:                                         */
  /*                                                                  */
  /* Bit 0: Page position bit: 1 if field is $LAST_ON_PAGE            */
  /*                           0 if field is $FIRST_ON_PAGE           */
  /* Bit 1: Different field bit: 1 if Bit 0 is valid                  */
  /*                             0 if field is "normal" field         */
  /* Bit 2: MIN bit: 1 if field has $MIN attribute                    */
  /*                 0 otherwise                                      */
  /* Bit 3: MAX bit: 1 if field has $MAX attribute                    */
  /*                 0 otherwise                                      */
  /* Bit 4: Justification bit: 1 right justify field                  */
  /*                           0 left justify field                   */
  /* Bit 5: Default justification bit: 1 use Bit 4 to justify         */
  /*                                   0 use default justification    */
  /* Bit 6: Display bit: 1 do not display data for field              */
  /*                     0 display the data for field                 */
  /* Bit 7: Start entry again bit: 1 start the same entry again       */
  /*                               0 keep current entry as it is      */
  /********************************************************************/
  USHORT                usMinChars;      // No of chars if $MIN is set
  USHORT                usMaxChars;      // No of chars if $MAX is set
  USHORT                usFormatID;      // Format ID if $TIME or $DATE
                                         // otherwise invalid
} QDPR_ATTRIBS, *PQDPR_ATTRIBS;

//------------- field cross-reference table structures ---------------//
typedef struct _QDPR_FCRT_ELEMENT
{
  PSZ_W                 *ppszFieldData;  // Pointer to field data ptr in
                                         // template (or NULL if it is
                                         // a system variable)
  PCHAR                 pchrFieldStart;  // Pointer to buffer location where
                                         // field tag start
  USHORT                usTagID;         // Identifies the tag or system
                                         // variable that is referenced
  QDPR_ATTRIBS          sctAttribs;      // Attribute structure (see above)
} QDPR_FCRT_ELEMENT, *PQDPR_FCRT_ELEMENT;

typedef struct _QDPR_FCRT
{
  QDPR_FCRT_ELEMENT     asctFCRTElements[QDPR_MAX_FCRT_ELEMENTS];
                                             // Array of field cross-reference
                                             // table entries
  USHORT                usElements;          // No of table entries
  struct _QDPR_FCRT     *psctFCRTExtension;  // If table is full next
                                             // table part
} QDPR_FCRT, *PQDPR_FCRT;

//------------- buffers structure ------------------------------------//
typedef struct _QDPR_PROCESS_BUFFER
{
  CHAR                         achrBuffer[QDPR_MAX_PROCESS_BUFFER];
                                                     // Buffer area
  struct _QDPR_PROCESS_BUFFER  *psctBufferExtension; // If buffer is full
                                                     // next buffer area
} QDPR_PROCESS_BUFFER, *PQDPR_PROCESS_BUFFER;

//------------- format buffers structure -----------------------------//
typedef struct _QDPR_FORMAT_BUFFERS
{
  PQDPR_PROCESS_BUFFER psctBuffer;       // buffer area
  PQDPR_PROCESS_BUFFER psctCurBufExt;    // current buffer extension
  PCHAR                pchrLastWritten;  // Pointer to next free character
                                         // in the current buffer area
  USHORT               usCharsUsed;      // No of bytes used in the current
                                         // line of the current buffer area
  USHORT               usCurIndent;      // Indent of current line in
                                         // current buffer area
  BOOL                 fNewLineStarts;   // TRUE = a new line from the
                                         // format information file is
                                         // processed
  BOOL                 fLineSplit;       // TRUE = Line had to be split
} QDPR_FORMAT_BUFFERS, *PQDPR_FORMAT_BUFFERS;

//------------- format IDA structure ---------------------------------//
typedef struct _QDPR_FORMAT_IDA
{
  PSZ                  pszPageBuffer;    // Pointer to buffer area where the
                                         // whole page is stored
                                         // ($LINELENGTH * PAGELENGTH)
  ULONG                ulSizePageBuffer; // Size of the buffer area
  PCHAR                pchrLastLF;       // Pointer to character behind the
                                         // last LF in the PageBuffer
  PCHAR                pchrLastChar;     // Pointer to character behind the
                                         // last character in the PageBuffer
  PQDPR_FORMAT_BUFFERS psctEntry;        // format buffers used for
                                         // processing the entry part
  PQDPR_FORMAT_BUFFERS psctOther;        // format buffers used for
                                         // processing the other parts
} QDPR_FORMAT_IDA, *PQDPR_FORMAT_IDA;

//------------- structure for print functions ------------------------//
typedef struct _QDPR_PRINT_DEST
{
  BOOL                 fPrinter;          // TRUE if printer is output
                                          // print destination
  HPRINT               hPrinter;          // print handle
  PBUFCB               pFile;             // File pointer, only valid
                                          // if fPrinter is FALSE
  CHAR                 szPrintDest[QDPR_MAX_PATH_FILENAME];// Print destination
                                                           // either printer or
                                                           // file name
} QDPR_PRINT_DEST, *PQDPR_PRINT_DEST;

typedef enum _RCTYPE
{
   ASD_RC,                                // return code is from an ASD call
   LDB_RC,                                // return code is from a LDB call
   FILT_RC                                // return code is from a Filt call
} RCTYPE;                                 // type of following return code

//------------- thread structure -------------------------------------//
typedef struct _QDPR_THREAD
{
  PQDPR_IN_OUTPUT       psctInOutput;          // In/output structure
                                               // (see above)
  HWND                  hwndSlider;            // Slider window handle

  /********************************************************************/
  /*                    thread control information                    */
  /********************************************************************/
  PVOID                 pThreadStack;          // Thread stack pointer
  QDPR_STATUS_IDS       usThreadStatus;        // Thread status
  QDPR_STATUS_IDS       usNextThreadStatus;    // Next thread status
  QDPR_STATUS_IDS       usPreviousPrintStatus; // Previous print status
  BOOL                  fStopThread;           // TRUE = stop thread
  BOOL                  fThreadReady;          // TRUE = thread is NOT busy
  USHORT                usThreadError;         // Error code
  USHORT                usSyntaxError;         // Syntax error code
  PSZ                   pReplAddr[5];          // Parms table for UtlError
  USHORT                usNoOfSyErrParms;      // No. of Parms in pReplAddr
  BOOL                  fNoShowSuccessMsg;     // TRUE = Do not show success msg
  BOOL                  fThreadRunning;        // TRUE = thread is running

  /********************************************************************/
  /*                  dictionary control information                  */
  /********************************************************************/
  PPROPDICTIONARY       pDictProp;             // Ptr to dictionary properties
  USHORT                ausNoOfFields[QLDB_MAX_LEVELS]; // No. of fields array
  USHORT                ausFieldTable[QLDB_MAX_LEVELS]; // Fields table array
  USHORT                usTotalFields;         // Total number of fields in
                                               // a dictionary entry
  ULONG                 ulDictEntries;         // No. of total dict entries
  ULONG                 ulEntriesProcessed;    // No. of entries processed
  ULONG                 ulOldPercent;          // % of entries processed
  HFILTER               hFilter;               // Filter handle
  PSZ_W                 pucEntry;              // Pointer to entry buffer
  CHAR_W                aucTerm[MAX_TERM_LEN]; // Term buffer
  ULONG                 ulEntrySize;           // size of allocated entry buffer
  BOOL                  fEntriesPrinted;       // TRUE if at least one entry
                                               // was actually printed

  PQDPR_PRINT_DEST      psctPrintDest;         // Print destination file ptr

  /********************************************************************/
  /*         field cross-reference tables control information         */
  /********************************************************************/
  PQDPR_FCRT            psctHeaderFCRT;        // Header fcrt
  PQDPR_FCRT            psctCurHeaderFCRT;     // Current header fcrt
  PQDPR_FCRT            psctPageheadFCRT;      // Pagehead fcrt
  PQDPR_FCRT            psctCurPageheadFCRT;   // Current pagehead fcrt
  PQDPR_FCRT            psctEntryFCRT;         // Entry fcrt
  PQDPR_FCRT            psctCurEntryFCRT;      // Current entry fcrt
  PQDPR_FCRT            psctRepeatFCRT;        // Repeat fcrt
  PQDPR_FCRT            psctCurRepeatFCRT;     // Current repeat fcrt
  PQDPR_FCRT            psctPagefootFCRT;      // Pagefoot fcrt
  PQDPR_FCRT            psctCurPagefootFCRT;   // Current pagefoot fcrt
  PQDPR_FCRT            psctTrailerFCRT;       // Trailer fcrt
  PQDPR_FCRT            psctCurTrailerFCRT;    // Current trailer fcrt

  /********************************************************************/
  /*               process buffers control information                */
  /********************************************************************/
  PQDPR_PROCESS_BUFFER  psctHeaderBuffer;      // Header buffer
  PQDPR_PROCESS_BUFFER  psctCurHeaderExt;      // Current header buf extension
  PCHAR                 pchrHeaderBuffer;      // Run pointer for header buf
  PQDPR_PROCESS_BUFFER  psctPageheadBuffer;    // Pagehead buffer
  PQDPR_PROCESS_BUFFER  psctCurPageheadExt;    // Current pagehead buf extension
  PCHAR                 pchrPageheadBuffer;    // Run pointer for pagehead buf
  PQDPR_PROCESS_BUFFER  psctEntryBuffer;       // Entry buffer
  PQDPR_PROCESS_BUFFER  psctCurEntryExt;       // Current entry buf extension
  PCHAR                 pchrEntryBuffer;       // Run pointer for entry buffer
  PQDPR_PROCESS_BUFFER  psctPagefootBuffer;    // Pagefoot buffer
  PQDPR_PROCESS_BUFFER  psctCurPagefootExt;    // Current pagefoot buf extension
  PCHAR                 pchrPagefootBuffer;    // Run pointer for pagefoot buf
  PQDPR_PROCESS_BUFFER  psctTrailerBuffer;     // Trailer buffer
  PQDPR_PROCESS_BUFFER  psctCurTrailerExt;     // Current trailer buf extension
  PCHAR                 pchrTrailerBuffer;     // Run pointer for trailer buf

  /********************************************************************/
  /*                            templates                             */
  /********************************************************************/
  PVOID           psctEntry;             // Current entry (full tree)
  PVOID           psctCurrentTemplate;   // Current template (only template)
  PVOID           psctFirstPageTemplate; // First page template
  PVOID           psctLastPageTemplate;  // Last page template

  /********************************************************************/
  /*            information from analyze to print process             */
  /********************************************************************/
  USHORT                usLineNumber;          // Current line number
  USHORT                usCaPageheadLines;     // Ca. no of pagehead lines
  USHORT                usCaPagefootLines;     // Ca. no of pagefoot lines
  USHORT                usLineLength;          // $LINELENGTH setting
  USHORT                usPageLength;          // $PAGELENGTH setting
  ULONG                 ulPageNumber;          // Current print page number
  BOOL                  fCRLFUsed;             // Is CRLF or only LF used ?
                                               // TRUE if CRLF is used

  /********************************************************************/
  /*                print process control information                 */
  /********************************************************************/
  BOOL                  fDestLocked;           // destination has been locked
  USHORT                usPagesProcessed;      // No of pages processed
  USHORT                usLinesPrinted;        // No of lines already printed
  USHORT                usEntryTotalLines;     // Total number of lines in
                                               // entry format buffer
  USHORT                usEntryLinesPrinted;   // Last line number in entry
                                               // format buffer that has been
                                               // printed
  PSZ                   pszPageEject;          // Page eject string
  BOOL                  fPrintEntry;           // TRUE = set next entry
                                               // processing status to printing
  BOOL                  fEntryBufferEnd;       // TRUE if entry buffer end
                                               // is reached
  BOOL                  fNotGetNextTemplate;   // TRUE = do not get the
                                               // next template in a /repeat
  PQDPR_REPEAT_STACK    psctRepeatStack;       // Pointer to top of repeat-stack
  PQDPR_FORMAT_IDA      psctFormatIDA;         // Pointer to format IDA
  USHORT                usDosRC;               // return code of Dos... calls
                                                                /* 8@KIT1138A */
  RCTYPE                RCType;                // type of following return code
  USHORT                usExtRC;               // return code of external calls

  /********************************************************************/
  /*                           work buffers                           */
  /********************************************************************/
  CHAR                  szWorkBuffer[QDPR_MAX_STRING];
  CHAR                  szWorkBuffer2[QDPR_MAX_STRING];
  CHAR                  szWorkBuffer3[QDPR_MAX_STRING];
  CHAR                  szWorkBuffer4[QDPR_MAX_STRING];
  CHAR                  szFieldname[DICTENTRYLENGTH + 1];
} QDPR_THREAD, *PQDPR_THREAD;

/**********************************************************************/
/*                               macros                               */
/**********************************************************************/
#define QDPR_CRLF_USED( fCRLFUsed, pszStart, pszRun )                 \
{                                                                     \
  pszRun = pszStart;                                                  \
  while ( ( *pszRun != NULC ) && ( *pszRun != QDPR_LF ) )             \
  {                                                                   \
    pszRun++;                                                         \
  } /* endwhile */                                                    \
  pszRun--;                                                           \
  fCRLFUsed = ( *pszRun == QDPR_CR );                                 \
}

/**********************************************************************/
/*                        function prototypes                         */
/**********************************************************************/
//------------- functions in EQFQDPR.C -------------------------------//
MRESULT QDPRCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
VOID   QDPRPrintThread( PVOID );
VOID   QDPRAnalyze( PQDPR_THREAD );
VOID   QDPRPrint( PQDPR_THREAD );
VOID   QDPRReportError( PQDPR_THREAD psctIDA );

//------------- functions in EQFQDPRD.C ------------------------------//
INT_PTR CALLBACK QDPRPRINTDIALOG( HWND, WINMSG, WPARAM, LPARAM );
USHORT QDPRReadProps( PQDPR_PROPS * );
USHORT QDPRWriteProps( PQDPR_PROPS );
USHORT QDPRSetLastUsedValues( HWND, USHORT, PQDPR_DLG_IDA,
                              PQDPR_LAST_USED_VALS );
USHORT QDPRSaveLastUsedValues( HWND, USHORT, PQDPR_DLG_IDA );
USHORT QDPRProcessFormatFile( PQDPR_DLG_IDA);
USHORT QDPRScanDescTag( PSZ, PSZ, USHORT, PTOKENENTRY );
MRESULT QDPRPrintDlgControl( HWND, SHORT, SHORT );

//------------- functions in EQFQDPRA.C ------------------------------//
USHORT QDPRCreateFCRTElement( PQDPR_THREAD, PQDPR_FCRT *, PSZ, USHORT,
                              QDPR_ATTRIBS, PCHAR, PUSHORT );
BOOL   QDPREndTagInTag( PSZ, USHORT, BOOL );
USHORT QDPRReadEntry( PQDPR_THREAD, PTOKENENTRY * );
USHORT QDPRReadHeader( PQDPR_THREAD, PTOKENENTRY * );
USHORT QDPRReadOverComment( PTOKENENTRY * );
USHORT QDPRReadOverDescription( PTOKENENTRY * );
USHORT QDPRReadPagefoot( PQDPR_THREAD, PTOKENENTRY * );
USHORT QDPRReadPagehead( PQDPR_THREAD, PTOKENENTRY * );
USHORT QDPRReadRepeatTag( PQDPR_THREAD, PTOKENENTRY *, PUSHORT, PSZ,
                          USHORT, PQDPR_ATTRIBS );
USHORT QDPRReadSetTag( PQDPR_THREAD, PTOKENENTRY * );
USHORT QDPRReadTrailer( PQDPR_THREAD, PTOKENENTRY * );
USHORT QDPRReadVarTag( PQDPR_THREAD, PTOKENENTRY *, PUSHORT, PSZ,
                       USHORT, PQDPR_ATTRIBS );
USHORT QDPRRetrieveValue( PTOKENENTRY, PSZ, USHORT );
VOID   QDPRSuppressFirstCRLF( PTOKENENTRY );
VOID   QDPRSyntaxError( PQDPR_THREAD, USHORT, USHORT, PSZ, USHORT, USHORT,
                        USHORT, PUSHORT, PUSHORT );

//------------- functions in EQFQDPRP.C ------------------------------//
USHORT QDPREvaluateEndRepeatTag( PQDPR_THREAD, PQDPR_REPEAT_STACK *,
                                 PVOID, PVOID, PCHAR * );
USHORT QDPREvaluateFieldTag( PQDPR_THREAD, PQDPR_FCRT, USHORT,
                             PQDPR_FORMAT_BUFFERS, PUSHORT );
USHORT QDPREvaluateRepeatTag( PQDPR_THREAD, PQDPR_REPEAT_STACK *,
                              PQDPR_FCRT, USHORT );
USHORT QDPREvaluateSysVar( PQDPR_THREAD, USHORT, PQDPR_FCRT, USHORT,
                           PQDPR_FORMAT_BUFFERS, PUSHORT );
USHORT QDPRFormatStrToLineLength( PQDPR_THREAD, PQDPR_FORMAT_IDA, PSZ,
                                  PQDPR_FORMAT_BUFFERS, PUSHORT );
USHORT QDPRPrintEntry( PQDPR_THREAD );
USHORT QDPRPrintHeader( PQDPR_THREAD );
USHORT QDPRPrintPageEject( PQDPR_THREAD, PQDPR_FORMAT_BUFFERS );
USHORT QDPRPrintPagefoot( PQDPR_THREAD );
USHORT QDPRPrintPagehead( PQDPR_THREAD );
USHORT QDPRPrintToPageBuffer( PQDPR_THREAD, PQDPR_FORMAT_IDA,
                              PQDPR_FORMAT_BUFFERS, USHORT, USHORT, BOOL );
USHORT QDPRPrintTrailer( PQDPR_THREAD );
USHORT QDPRPrintUntilTag( PQDPR_THREAD, PQDPR_PROCESS_BUFFER *,
                          PCHAR *, PQDPR_FORMAT_BUFFERS, PQDPR_FCRT *,
                          PQDPR_FCRT *, PUSHORT, PUSHORT, PUSHORT );

//------------- functions in EQFQDPRU.C ------------------------------//
USHORT QDPRAddToProcessBuffer( PQDPR_PROCESS_BUFFER *, PCHAR *, PSZ );
USHORT QDPRAllocateBufferExt( PQDPR_PROCESS_BUFFER * );
USHORT QDPRClearAndDeallocateBuffer( PQDPR_PROCESS_BUFFER );
VOID   QDPRCloseDictionary( HUCB, HDCB, BOOL );
BOOL   QDPRCompareNodes( PQLDB_NODE, PQLDB_NODE, USHORT );
USHORT QDPRCopyCurrentTemplates( PVOID, PVOID );
USHORT QDPRDeallocateIDAStorage( PQDPR_THREAD );
VOID   QDPRDeallocateInOutStruct( PQDPR_IN_OUTPUT * );
USHORT QDPRLineNumbers( PSZ, PSZ, USHORT );
USHORT QDPRMakeTagFromTagID( PSZ, USHORT, USHORT, BOOL );
USHORT QDPROpenDictionary( PSZ, PHUCB, PHDCB );
USHORT QDPRPrintDestClose( PQDPR_PRINT_DEST *, PUSHORT );
USHORT QDPRPrintDestOpen( PQDPR_PRINT_DEST *, PSZ, BOOL, BOOL, PUSHORT );
USHORT QDPRPrintDestWrite( PQDPR_PRINT_DEST, PSZ, PUSHORT );
USHORT QDPRQueryFileSize( PSZ, PULONG );
USHORT QDPRResetFormatBuffer( PQDPR_FORMAT_BUFFERS, BOOL );


#define QDPR_SYER_MISSING_FLTFIELDNAME                          750
