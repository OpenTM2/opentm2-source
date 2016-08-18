/*! Parser API include file
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

#define EQF_SEGLEN 2048

#define _EQFPAPI_H_INCLUDED

typedef HANDLE HPARSER;                // parser API handle
typedef HANDLE HPARSSEGFILE;           // handle of a segmented file loaded into memory
typedef HANDLE HPARSMARKUP;            // handle of loaded markup table


#pragma pack( push, segmentpacking, 1 )

// status of a segment
typedef enum _PARSSEGSTATUS
{
  PARSSEG_XLATED,                      // segment has been translated
  PARSSEG_TOBE,                        // segment is to be translated
  PARSSEG_NOP,                         // segment contains no translatable data
  PARSSEG_ATTR                         // segment contains translatable attributes
} PARSSEGSTATUS;



// segment information flags
typedef struct _PARSESEGFLAGS
{
  unsigned short JoinStart;            // TRUE if segment is start of a joined chain
  unsigned short Joined;               // TRUE if segment is part of a joined chain
  unsigned short Marked;               // TRUE if segment is marked
  unsigned short Current;              // TRUE if segment is current segment
  unsigned short UnTrans;              // TRUE if segment is untranslated
  unsigned short Typed;                // TRUE if typing done in segment
  unsigned short Copied;               // TRUE if proposal copied (but nor ctrl 0)
  unsigned short Expanded;             // TRUE if segment is expanded
  unsigned short NoCount;              // TRUE if segment should not be counted
  unsigned short InsertAdd;            // TRUE: seg is preceeded by "addtablesegs"
  unsigned short NoWrite;              // TRUE if seg not written in STARGETfile (XLATE)
  unsigned short Spellchecked;         // TRUE: segment is spellchecked
  unsigned short Free1;                // for future use
  unsigned short Free2;                // for future use
  unsigned short Free3;                // for future use
  unsigned short Free4;                // for future use
} PARSSEGFLAGS, *PPARSSEGFLAGS;

// segment counting flags
typedef struct _PARSCOUNTFLAGS
{
  unsigned short AnalAutoSubst    : 1;
  unsigned short EditAutoSubst    : 1;
  unsigned short ExactCopy        : 1;
  unsigned short MachCopy         : 1;
  unsigned short FuzzyCopy        : 1;
  unsigned short ReplCopy         : 1;
  unsigned short ExactExist       : 1;
  unsigned short MachExist        : 1;
  unsigned short FuzzyExist       : 1;
  unsigned short ReplExist        : 1;
  unsigned short Free1 : 1;                // for future use
  unsigned short Free2 : 1;                // for future use
  unsigned short Free3 : 1;                // for future use
  unsigned short Free4 : 1;                // for future use
  unsigned short Free5 : 1;                // for future use
  unsigned short Free6 : 1;                // for future use
} PARSCOUNTFLAG, *PPARSCOUNTFLAG;

// segment data structure
typedef struct _PARSSEGMENT
{
  char             szData[EQF_SEGLEN*2]; // buffer for segment data
  unsigned short   usLength;           // length of segment data (incl. string end delimiter)
  PARSSEGSTATUS    Status;             // status of segment
  LONG             lSegNum;            // segment number
  PARSSEGFLAGS     SegFlags;           // segmentation flags
  PARSCOUNTFLAG    CountFlags;         // counting flags
  unsigned short   usSrcWords;         // number of source words
  unsigned short   usTgtWords;         // number of target words
  unsigned short   usModWords;         // number of modified words
} PARSSEGMENT, *PPARSSEGMENT;

// segment data structure
typedef struct _PARSSEGMENTW
{
  WCHAR            szData[EQF_SEGLEN*2]; // buffer for segment data
  unsigned short   usLength;           // length of segment data (incl. string end delimiter)
  PARSSEGSTATUS    Status;             // status of segment
  LONG             lSegNum;            // segment number
  PARSSEGFLAGS     SegFlags;           // segmentation flags
  PARSCOUNTFLAG    CountFlags;         // counting flags
  unsigned short   usSrcWords;         // number of source words
  unsigned short   usTgtWords;         // number of target words
  unsigned short   usModWords;         // number of modified words
} PARSSEGMENTW, *PPARSSEGMENTW;

#pragma pack( pop, segmentpacking )

// special token IDs
#define PARSTOKEN_TEXT -1              // text token
#define PARSTOKEN_ENDOFLIST -2         // end of token list

// token data structure (produced by ParsTokenize and returned by ParsGetNextToken)
typedef struct _PARSTOKEN
{
  int              iTokenID;           // token ID (position of tag in tab table)
  int              iStart;             // token start postion in tokenized data
  int              iLength;            // length of token
  unsigned short   usFixedID;          // fixed token ID of tag or 0 if none set
  unsigned short   usAddInfo;          // addition info of tag or 0 if none set
  unsigned short   usClassID;          // class ID of tag or 0 if none set
} PARSTOKEN, *PPARSTOKEN;


// terminate parser API environment
__declspec(dllexport)
int ParsTerminate
(
  HPARSER hParser                      // parser API handle returned by ParsInitialize
);

// initialize parser API environment
__declspec(dllexport)
int ParsInitialize
(
  HPARSER *phParser,                   // ptr to buffer for parser API handle
  char    *pszDocPathName              // document path name
);

// create a temporary file name using the given file name as model
int ParsBuildTempName
(
  PSZ         pszSourceName,           // model filename
  PSZ         pszTempName              // ptr to buffer for temporary filename
                                       // (should have a size of 128 bytes or more)
);

// load a segmented file into memory
__declspec(dllexport)
int  ParsLoadSegFile
(
  HPARSER     hParser,                 // parser API handle
  char       *pszFileName,             // fully qualified document filename
  HPARSSEGFILE *phSegFile              // buffer for returned segmented file handle
);


// get number of segments in loaded segmented file
__declspec(dllexport)
int ParsGetSegNum
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  long         *plSegCount             // ptr to buffer for returned segment number
);

// get a segment from a segmented file loaded into memory
int  ParsGetSeg
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  long         lSegNum,                // number of segment to retrieve
  PPARSSEGMENT pSeg                    // ptr to buffer for segment data
);

// get a segment from a segmented file loaded into memory (Unicode version)
__declspec(dllexport)
int  ParsGetSegW
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  long         lSegNum,                // number of segment to retrieve
  PPARSSEGMENTW pSeg                   // ptr to buffer for segment data
);

// update a segment of a segmented file loaded into memory
int  ParsUpdateSeg
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  long         lSegNum,                // number of segment to update
  PPARSSEGMENT pSeg                    // ptr new segment data
);

// update a segment of a segmented file loaded into memory (Unicode version)
__declspec(dllexport)
int  ParsUpdateSegW
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  long         lSegNum,                // number of segment to update
  PPARSSEGMENTW pSeg                   // ptr new segment data
);

// write a segmented file loaded into memory to an external file
__declspec(dllexport)
int  ParsWriteSegFile
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  char         *pszFileName            // fully qualified document filename
);

// free a segmented file loaded into memory
__declspec(dllexport)
int  ParsFreeSegFile
(
  HPARSSEGFILE hSegFile                // handle of loaded segmented file
);

// make a SGML tagged segment as used in the segmented files
int ParsMakeSGMLSegment
(
  HPARSER     hParser,                 // parser API handle
  PPARSSEGMENT pSegment,               // ptr to segment data
  char        *pszBuffer,              // buffer for SGML tagged segment
                                       // (should be at least twice the max segment size)
  int         iBufferSize,             // size of buffer for SGML tagged segment
  BOOL        fSourceFile              // TRUE = create SGML for a segmented source file
                                       // FALSE = create SGML for a segmented target file
);

// make a SGML tagged segment as used in the segmented files (Unicode version)
int ParsMakeSGMLSegmentW
(
  HPARSER     hParser,                 // parser API handle
  PPARSSEGMENTW pSegment,              // ptr to segment data
  WCHAR       *pszBuffer,              // buffer for SGML tagged segment
                                       // (should be at least twice the max segment size)
  int         iBufferSize,             // size of buffer for SGML tagged segment
  BOOL        fSourceFile              // TRUE = create SGML for a segmented source file
                                       // FALSE = create SGML for a segmented target file
);

typedef enum _PARSCONVERSION
{
  ASCIItoANSI,                         // convert from ASCII to ANSI codepage
  ANSItoASCII,                         // convert from ANSI to ASCII codepage
  ANSItoUTF16,                         // convert from ANSI to UTF16 format
  ASCIItoUTF16,                        // convert from ASCII to UTF16 format
  UTF16toASCII,                        // convert from UTF16 to ASCII
  UTF16toANSI                          // convert from UTF16 to ANSI
} PARSCONVERSION;

// inplace conversion from ASCII to ANSI or vice versa, or from
// ASCII/ANSI to/from UTF16 format
// keep in mind that the buffer must be twice usLen size when converting to
// UTF16 format
int ParsConvert
(
  HPARSER     hParser,                 // parser API handle
  PARSCONVERSION Conversion,           // conversion mode
  char       *pszData,                 // ptr to data being converted
  unsigned short usLen                 // length of data being converted
);

int ParsConvert2
(
  HPARSER     hParser,                 // parser API handle
  PARSCONVERSION Conversion,           // conversion mode
  char       *pszData,                 // ptr to data being converted
  unsigned short usLen,                 // length of data being converted
  char       *pszLanguage
);


// get long document name for current document
int ParsGetDocName
(
  HPARSER     hParser,                 // parser API handle
  char       *pszDocName               // ptr to buffer for document name (should
                                       // be 256 bytes in size)
);

// get language settings for current document
int ParsGetDocLang
(
  HPARSER     hParser,                 // parser API handle
  char       *pszSourceLang,           // ptr to buffer for source language or NULL
  char       *pszTaregtLang            // ptr to buffer for target language or NULL
                                       // (size should be at least 40 bytes)
);


// split data into segments using the morphologic functions
// The functions look for segment breaks in the supplied data applying the
// morphology for the document source language. The segment breaks are
// returned in form of an segment break list. The segment break list is the
// list of the offsets of segbreaks within the data. The last element of the
// list is zero. If the buffer for the list is too small the return code
// ERROR_BAD_LENGTH is returned and the the first element of the segbreak list
// contains the required size of the list (in number of elements)
int ParsSplitSeg
(
  HPARSER     hParser,                 // parser API handle
  char       *pszData,                 // text data being splitted into segments
  unsigned short usDataLength,         // length of text data
  unsigned short *pusSegBreaks,        // ptr to buffer which receives the segbreaks
  unsigned short usElements            // size of segbreak buffer in number of elements
);

int ParsSplitSegW
(
  HPARSER     hParser,                 // parser API handle
  WCHAR      *pszData,                 // text data being splitted into segments
  unsigned short usDataLength,         // length of text data (characters)
  unsigned short *pusSegBreaks,        // ptr to buffer which receives the segbreaks
  unsigned short usElements            // size of segbreak buffer in number of elements
);

// Load a markup table into memory for usage with function ParsTokenize
// In case of errors a return value other than zero is returned
int ParsLoadMarkup
(
  HPARSER     hParser,                 // parser API handle
  HPARSMARKUP *phMarkup,               // ptr to buffer receiving the markup handle
  char        *pszMarkup               // name of the markup table
);

// Free a markup table loaded with ParsLoadMarkup
int ParsFreeMarkup
// In case of errors a return value other than zero is returned
(
  HPARSMARKUP  hMarkup                  // handle of markup as created by ParsloadMarkup
);

// Look for tags in the given buffer, the buffer must be zero terminated
// In case of errors a return value other than zero is returned
int ParsTokenize
(
  HPARSMARKUP  hMarkup,                 // handle of markup as created by ParsloadMarkup
  char         *pszData                 // pointer to data being tokenized

);

int ParsTokenizeW
(
  HPARSMARKUP  hMarkup,                 // handle of markup as created by ParsloadMarkup
  WCHAR        *pszData                 // pointer to data being tokenized

);

// return the next token from the token list produced with ParsTokenize
// At the end of the token list a token with an ID of PARSTOKEN_ENDOFLIST
// is returned.
// In case of errors a return value other than zero is returned
int ParsGetNextToken
(
  HPARSMARKUP hMarkup,                 // handle of markup as created by ParsloadMarkup
  PPARSTOKEN  pToken                   // ptr to a PARSTOKEN structure receiving the
                                       // data of the token
);

__declspec(dllexport)
int  ParsGetSourceLine
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  LONG         lSegNum,                // number of segment to check
  PLONG        plStartLine,            // ptr to buffer for start line
  PLONG        plEndLine               // ptr to buffer for end line
);

// get the line number for the given segment
__declspec(dllexport)
int  ParsGetSegmentNumber
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  LONG         lLine,                  // l9ine number of segment
  LONG         lColumn,                // column position of segment
  PLONG        plSegNum                // ptr to buffer for segment number
);

