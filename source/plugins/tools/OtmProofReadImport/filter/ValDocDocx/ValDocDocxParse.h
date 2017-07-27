/*
*  Copyright (C) 1998-2017, International Business Machines          
*         Corporation and others. All rights reserved 
*                 IBM Internal Use Only
*
* ValDocDocxParse.h
*
* CHANGES:
*      02/23/05   DAW  Inital version
*
*/

using namespace mku ;                       


#define   NODE_TYPE_NONE                     0     // NODE type has not been determined 
#define   NODE_TYPE_TEXT                     1     // NODE contains text information
#define   NODE_TYPE_NEUTRAL                  2     // NODE contains neutral tag information 
#define   NODE_TYPE_TEXT_REF                 3     // NODE contains a reference to text     
#define   NODE_TYPE_SHEET_REF                4     // NODE contains a reference to a sheet    
#define   NODE_TYPE_FILE_NAME                5     // NODE contains the name of file being processed


#define   TAG_ACTION_NONE                    0     //  Unknown classification
#define   TAG_ACTION_TEXT                    1     //  Translatable text - KEEP
#define   TAG_ACTION_KEEP                    1     //  Always keep this tag
#define   TAG_ACTION_SKIP                    2     //  Skip if first or last tag in paragraph
#define   TAG_ACTION_UNKNOWN                 3     //  Unknown tag - KEEP


#define   TAG_TYPE_NONE                      0     //  Unknown classification
#define   TAG_TYPE_BEGIN                     1     //  Begin tag
#define   TAG_TYPE_END                       2     //  End tag
#define   TAG_TYPE_EMPTY                     3     //  Empty tag


#define   TAG_ID_NONE                        0
#define   TAG_ID_TEXT                        1
#define   TAG_ID_ANNOTATEREF                 2
#define   TAG_ID_BR                          3
#define   TAG_ID_BR_BEGIN                    4
#define   TAG_ID_BR_END                      5
#define   TAG_ID_CONTINUATIONSEPARATOR       6
#define   TAG_ID_CR                          7
#define   TAG_ID_DAYLONG                     8
#define   TAG_ID_DAYSHORT                    9
#define   TAG_ID_DRAWING                    10
#define   TAG_ID_ENDNOTE                    11
#define   TAG_ID_ENDNOTEREF                 12
#define   TAG_ID_FLDCHAR                    13
#define     TAG_ID_FLDCHAR_END              14
#define     TAG_ID_FLDCHAR_BEGIN            15
#define   TAG_ID_FOOTNOTE                   16
#define   TAG_ID_FOOTNOTEREF                17
#define   TAG_ID_HLINK                      18
#define     TAG_ID_HLINK_BEGIN              19
#define     TAG_ID_HLINK_END                20
#define   TAG_ID_HYPERLINK                  21
#define     TAG_ID_HYPERLINK_BEGIN          22
#define     TAG_ID_HYPERLINK_END            23
#define   TAG_ID_INSTRTEXT                  24
#define   TAG_ID_LASTRENDEREDPAGEBREAK      25
#define   TAG_ID_MONTHLONG                  26
#define   TAG_ID_MONTHSHORT                 27
#define   TAG_ID_NOBREAKHYPHEN              28
#define   TAG_ID_OBJECT                     29
#define   TAG_ID_PGNUM                      30
#define   TAG_ID_PICT                       31
#define   TAG_ID_PTAB                       32
#define   TAG_ID_SEPARATOR                  33
#define   TAG_ID_SOFTHYPHEN                 34
#define   TAG_ID_SYM                        35
#define   TAG_ID_TAB                        36
#define   TAG_ID_UNKNOWN                    37
#define   TAG_ID_YEARLONG                   38
#define   TAG_ID_YEARSHORT                  39
#define   TAG_ID_DELTEXT                    40
#define   TAG_ID_INSTEXT                    41



#define   PARA_STATE_NONE                    0
#define   PARA_STATE_PARA                    1
#define   PARA_STATE_PROPERTY                2

#define   RUN_STATE_NONE                     0
#define   RUN_STATE_RUN                      1
#define   RUN_STATE_PROPERTY                 2
#define   RUN_STATE_UNKNOWN                  3

#define   FIELD_STATE_NONE                   0
#define   FIELD_STATE_BEGIN                  1
#define   FIELD_STATE_SEPARATE               2
#define   FIELD_STATE_END                    3
                                              
#define   COND_ADDATTR_ALWAYS                0
#define   COND_ADDATTR_CHGONLY               1
#define   COND_ADDATTR_NEWONLY               2

#define   CONCAT_TEXT_CHECK                  0
#define   CONCAT_TEXT_YES                    1
#define   CONCAT_TEXT_NO                     2
                                              
#define   NEST_NONE                          0
#define   NEST_PARA                          1
#define   NEST_RUN                           2
#define   NEST_RESET_PARA                    3
#define   NEST_RESET_RUN                     4
#define   NEST_RESET_END_PARA                5
#define   NEST_RESET_END_RUN                 6
#define   NEST_RESET_END_DONE                7

#define   WRITE_SOURCE                       1
#define   WRITE_TARGET                       2


#define   COMMONPROPERTY_FORCE_OFF           9999


typedef struct                               // Structure to define paragraph <w:p> information
{
   ULONG     SeqNum ;                        // Sequence number
   ULONG     NodeType ;                      // Type of node: text, tag   
   ULONG     StartPos ;                      // Starting file position of <w:p>  (fseek)
   ULONG     EndPos ;                        // Ending   file position of </w:p> (fseek)
   ULONG     BlockStartPos ;                 // Starting file position of block  (fseek)
   ULONG     BlockEndPos ;                   // Ending   file position of block  (fseek)
   ULONG     NumTextTags ;                   // Number of <w:t> tags in this paragraph
   ULONG     NumNeutralTags ;                // Number of neutral tags in this paragraph
   ULONG     CommonProperty ;                // Run number if common properties used
   BOOL      PreserveBlanks ;                // TRUE=Preserve leading/trailing blanks (export)        
   BOOL      StartTableRow ;                 // TRUE=Start of table row.
   WCHAR     *Tag ;                          // Name of tag for this node
   VOID      *ptrRunList ;                   // Pointer to <w:r> information
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
   VOID      *ptrPrev ;                      // Pointer to previous item in linked list 
} P_INFO ;


typedef struct                               // Structure to define run-level <w:r> information
{
   ULONG     SeqNum ;                        // Sequence number
   ULONG     BeginSeqNum ;                   // Sequence number of related begin tag
   ULONG     NodeType ;                      // Type of node: text, tag   
   ULONG     TagAction ;                     // Text, Keep, Skip, Unknown                    
   ULONG     TagType ;                       // Begin, End, Empty
   ULONG     TagId ;                         // Tag ID number             
   ULONG     StartPos ;                      // Starting file position of block  (fseek)
   ULONG     EndPos ;                        // Starting file position of block  (fseek)
   ULONG     BlockStartPos ;                 // Starting file position of text   (fseek)
   ULONG     BlockEndPos ;                   // Ending   file position of text   (fseek)
   ULONG     NumTextTags ;                   // Number of <w:t> tags in this paragraph
   ULONG     NumNeutralTags ;                // Number of neutral tags in this paragraph
   ULONG     Concat ;                        // Concatenate text with previous run of text
   ULONG     CommonProperty ;                // Count used to determine if common properties used
   BOOL      PreserveBlanks ;                // TRUE=Preserve leading/trailing blanks (export)        
   BOOL      BreakBeforeText;                // TRUE=<w:br> before text in same run
   BOOL      TextIns;                        // TRUE=Run is inserted text.
   BOOL      TextDel;                           // TRUE=Run is deleted text.
   WCHAR     *Tag ;                          // Name of tag for this node
   WCHAR     *Properties ;                   // Properties associated with this run
   VOID      *ptrTextList ;                  // Pointer to <w:t> information
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
   VOID      *ptrPrev ;                      // Pointer to previous item in linked list 
} R_INFO ;


typedef struct                               // Structure to define text-level <w:t> information
{
   ULONG     SeqNum ;                        // Sequence number
   ULONG     BeginSeqNum ;                   // Sequence number of related begin tag
   ULONG     NodeType ;                      // Type of node: text, tag   
   ULONG     TagAction ;                     // Action to be taken: text, keep, skip, unknown                    
   ULONG     TagType ;                       // Type of tag: begin, end, empty
   ULONG     TagId ;                         // Tag ID number             
   ULONG     StartPos ;                      // Starting file position of block  (fseek)
   ULONG     EndPos ;                        // Ending   file position of block  (fseek)
   ULONG     BlockStartPos ;                 // Starting file position of text   (fseek)
   ULONG     BlockEndPos ;                   // Ending   file position of text   (fseek)
   WCHAR     *Tag ;                          // Name of tag for this node
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
   VOID      *ptrPrev ;                      // Pointer to previous item in linked list 
} T_INFO ;


    USHORT     fnCreateInputList( P_INFO ** ) ;
    USHORT     fnGetXMLTag( WCHAR *, ULONG *, ULONG *, WCHAR *, WCHAR *,ULONG *, ULONG *, BOOL * ) ;
    BOOL       fnGetAttributeValue( WCHAR *, WCHAR *, WCHAR * ) ;
    BOOL       fnGetNextAttributeValue( WCHAR *, USHORT, WCHAR *, WCHAR *, WCHAR **, ULONG ) ;
    BOOL       fnGetCompleteString( WCHAR *, WCHAR *, WCHAR *, LONG ) ;
    USHORT     fnGetXMLRcd( WCHAR *, ULONG * ) ;
    USHORT     fnWriteBlock( USHORT, ULONG, ULONG, wofstream*, BOOL ) ;
    USHORT     fnReadBlock( USHORT, ULONG, ULONG, WCHAR*, ULONG ) ;
    BOOL       fnAllocateParaNode( P_INFO**, P_INFO**, WCHAR*, BOOL ) ;
    BOOL       fnAllocateRunNode( R_INFO**, R_INFO**, P_INFO*, WCHAR*, BOOL ) ;
    BOOL       fnAllocateTextNode( T_INFO**, T_INFO**, R_INFO*, WCHAR*, BOOL ) ;
    BOOL       fnSaveRunProperties( R_INFO*, WCHAR* ) ;
    BOOL       fnIsEmptyTag( WCHAR * ) ;
    BOOL       fnCheckCommonProp( WCHAR * ) ;
    VOID       fnFreeParaList( P_INFO ** ) ;
    VOID       fnFreeRunList( R_INFO ** ) ;
    VOID       fnFreeTextList( T_INFO ** ) ;


