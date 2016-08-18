//+----------------------------------------------------------------------------+
//| EQFMETADATA.H                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Metadata defines and functions                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//
//
// metadata types
typedef enum _MD_TYPE
{
  MD_NOTELIST_TYPE = 1,                          // list of translator notes
  MD_HISTORY_TYPE,                               // segment history data
  MD_STATUS_TYPE,                                // segment status data
  MD_UNKNOWN_TYPE,                               // metadata not processed by TM
  MD_PROPOSAL_TYPE,                              // metadata proposal
  MD_ENDOFLIST_TYPE,                             // dummy type, marks end of list
  MD_COMMENT_TYPE,                               // segment comment
} MD_TYPE;


// header used by all metadata elements
typedef struct _MD_ELEMENT_HDR
{
  LONG             lSize;                        // overall length of metadata
  MD_TYPE          Type;                         // type of metadata 
} MD_ELEMENT_HDR, *PMD_ELEMENT_HDR;

// segment comment
//   HDR followed by zero terminated CHAR_W value
typedef struct _MD_COMMENT_DATA
{
  MD_ELEMENT_HDR   Hdr;
  CHAR_W           szStyle[40];                 // status (textual form)
  CHAR_W           szComment[1];                // array for comments (dynamically allocated)
} MD_COMMENT_DATA, *PMD_COMMENT_DATA;


// segment status
typedef struct _MD_STATUS_DATA
{
  MD_ELEMENT_HDR   Hdr;
  BOOL             fApproved;                   // segment has been approved flag
  CHAR_W           szStatus[40];                // status (textual form)
} MD_STATUS_DATA, *PMD_STATUS_DATA;

// segment history
typedef struct _MD_HIST_DATA
{
  MD_ELEMENT_HDR   Hdr;
  LONG             lCreationDate;               // creation date
  CHAR_W           szCreator[40];               // creator
  LONG             lModificationDate;           // modification date
  CHAR_W           szModifier[40];              // modifier
} MD_HIST_DATA, *PMD_HIST_DATA;

// proposals data
typedef struct _MD_PROPOSAL_DATA
{
  MD_ELEMENT_HDR   Hdr;
  ULONG            ulSegmentId;                 // seg. number of proposal source
  LONG             lCreationDate;               // creation/update date of proposal
  LONG             lModificationDate;           // creation/update date of proposal
  SHORT            sQuality;                    // proposal quality 1..100
  BOOL             fApproved;                   // segment has been approved flag
  LONG             lSourcePos;                  // position of source within buffer
  LONG             lTargetPos;                  // position of target within buffer
  LONG             lAuthorPos;                  // position of author within buffer
  LONG             lModifierPos;                // position of author within buffer
  LONG             lDocNamePos;                 // position of document name within buffer
  BYTE             Buffer[1];                   // string buffer: source, target, author, mod, docname
} MD_PROPOSAL_DATA, *PMD_PROPOSAL_DATA;

// structure for a single translator note
typedef struct _MD_NOTE_DATA
{
  LONG             lSize;                        // size of this note data
  CHAR_W           szAuthor[40];                 // author of note
  CHAR_W           szNote[1];                    // null terminated note text (variable length)
} MD_NOTE_DATA, *PMD_NOTE_DATA;

// list of translator notes (the structure is followed by a list of MD_NOTE_DATA structures)
typedef struct _MD_NOTELIST_DATA
{
  MD_ELEMENT_HDR   Hdr;
  LONG             lNumberOfNotes;              // number of notes in list
} MD_NOTELIST_DATA, *PMD_NOTELIST_DATA;

// unknown/not processed metadaza
// layout: header,null-terminated key string, null-terminated data string
typedef struct _MD_UNKNOWN_DATA
{
  MD_ELEMENT_HDR   Hdr;
  BYTE             bData[1];                    // variable sized data area
} MD_UNKNOWN_DATA, *PMD_UNKNOWN_DATA;

// attribute element
//   HDR followed by zero terminated CHAR_W name, followed by zero terminated CHAR_W value
typedef struct _MD_ATTRIBUTE_DATA
{
  MD_ELEMENT_HDR   Hdr;
  BYTE             bData[1];                    // variable sized data area
} MD_ATTRIBUTE_DATA, *PMD_ATTRIBUTE_DATA;

// value element
//   HDR followed by zero terminated CHAR_W value
typedef struct _MD_VALUE_DATA
{
  MD_ELEMENT_HDR   Hdr;
  BYTE             bData[1];                    // variable sized data area
} MD_VALUE_DATA, *PMD_VALUE_DATA;


// metadata definition for in-memory copy of metadata
typedef struct _MD_METADATA
{
  LONG             lSize;                        // overall length of metadata
  ULONG            ulSegNum;                     // segment number of this metadata
  BYTE             bData[1];                     // variable length data area (list of other MD_.. structures) always ended by a MD_ENDOFLIST_TYPE      
} MD_METADATA, *PMD_METADATA;


__declspec(dllexport)
PMD_ELEMENT_HDR MDGetFirstElement( PMD_METADATA pData );

__declspec(dllexport)
PMD_ELEMENT_HDR MDGetNextElement( PMD_ELEMENT_HDR pElement );

// Create an empty metadata area
__declspec(dllexport)
BOOL MDCreateMetaData( PMD_METADATA *ppData );

// Find first metadata element of the given type
__declspec(dllexport)
PMD_ELEMENT_HDR MDFindElementByType( PMD_METADATA pData, MD_TYPE Type );

// Add a metadata element to a metadata area
__declspec(dllexport)
BOOL MDAddElement( PMD_METADATA *ppData, PMD_ELEMENT_HDR pElement );

// Release a metadata area
__declspec(dllexport)
BOOL MDFreeMetaData( PMD_METADATA *ppData );
