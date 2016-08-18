/* otmmsofc.h file */

/*
*
*  Copyright (C) 1998-2016, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 2/26/2016: IBM : Original Source                                         */
/*==========================================================================*/

  using namespace mku ;                       

#define   MAX_XML_RCD_LENGTH              8000
#define   MAX_XML_RCD_LENGTH2             8500      /* Buffer allocated size */
#define   XML_TAG_LEN                       80

#define   FILE_FORMAT_UNKNOWN                0     // Unknown file format
#define   FILE_FORMAT_WORD                   1     // Office 2003 format
#define   FILE_FORMAT_XML                    2     // XML format 
#define   FILE_FORMAT_ZIP                    3     // Office 2007 format

#define   NODE_TYPE_NONE                     0     // NODE type has not been determined 
#define   NODE_TYPE_TEXT                     1     // NODE contains text information
#define   NODE_TYPE_NEUTRAL                  2     // NODE contains neutral tag information 
#define   NODE_TYPE_TEXT_REF                 3     // NODE contains a reference to text     
#define   NODE_TYPE_SHEET_REF                4     // NODE contains a reference to a sheet    
#define   NODE_TYPE_FILE_NAME                5     // NODE contains the name of file being processed


#define   TAG_ACTION_NONE                    0     //  Unknown classiciation
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
#define   TAG_ID_INSTRTEXT                  21
#define   TAG_ID_LASTRENDEREDPAGEBREAK      22
#define   TAG_ID_MONTHLONG                  23
#define   TAG_ID_MONTHSHORT                 24
#define   TAG_ID_NOBREAKHYPHEN              25
#define   TAG_ID_OBJECT                     26
#define   TAG_ID_PGNUM                      27
#define   TAG_ID_PICT                       28
#define   TAG_ID_PTAB                       29
#define   TAG_ID_SEPARATOR                  30
#define   TAG_ID_SOFTHYPHEN                 31
#define   TAG_ID_SYM                        32
#define   TAG_ID_TAB                        33
#define   TAG_ID_UNKNOWN                    34
#define   TAG_ID_YEARLONG                   35
#define   TAG_ID_YEARSHORT                  36



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

#define   DBCS_STATE_NONE                    0
#define   DBCS_STATE_FONTS                   1
#define   DBCS_STATE_RFONTS                  2
#define   DBCS_STATE_XFONT                   3
                                              
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
   WCHAR     *Tag ;                          // Name of tag for this node
   BOOL      bDoNotTranslate ;               // Special non-translatable section   1-21-13
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


typedef struct                               // Structure to define paragraph <w:p> information
{
   short     SeqNum ;                        // Sequence number
   short     TagType ;                       // Begin, End, Empty
   short     NestLevel ;                     // Relative nesting level from top
   short     Match ;                         // 0=No match, 1=Match                 
   WCHAR     Tag[20] ;                       // Entire tag                      
} CHK_INFO ;



typedef struct                               // Structure to define DBCS-unique information
{
   ULONG     NodeType ;                      // Type of node.
   ULONG     StartPos ;                      // Starting file position of block  (fseek)
   ULONG     EndPos ;                        // Starting file position of block  (fseek)
   WCHAR     *Tag ;                          // Name of tag for this node
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
   VOID      *ptrPrev ;                      // Pointer to previous item in linked list 
} DBCS_INFO ;



typedef struct                               // Structure to define spreadsheet non-translatable info
{
   ULONG     ulData ;                        // Data
   BOOL      bTrans ;                        // TRUE=String both translatable and non-translatable
   WCHAR     *szString ;                     // String value if bTrans=TRUE
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
} SSNT_INFO ;


// Routines defined in OTMMSOFC.C       ----------------------------------------------

    BOOL       ConvertWordToXml(char*, char*, char*, char*, USHORT* );
    BOOL       ConvertXmlToWord(char*, char*, char*, USHORT );
    BOOL       GetWordProcessList( ULONG * ); 
    BOOL       AutomationWrapper(int,VARIANT*,IDispatch*,char*,char*,int ...);
    BOOL       ExtractXmlFromZip(char*, char*, char*, char* );
    BOOL       ReplaceXmlInZip( char*, char*, char* );
    BOOL       ExecuteCommand( char*, char*, char* );
    BOOL       ConcatFiles( char*, char*, char* );
    BOOL       SplitFiles( char*, char*, char* );
    VOID       ReorderFiles( char* );
    VOID       GetSettings( char* );


// Routines defined in PARSE.CPP       ----------------------------------------------

    BOOL       PreParse(PSZ, PSZ, PSZ, HWND, USHORT);
    BOOL       PostParse(PSZ, PSZ, HWND);

    USHORT     fnCreateInputList( P_INFO **, SSNT_INFO **, ULONG, HWND, USHORT *, USHORT * ) ;
    USHORT     fnGetXMLTag( WCHAR *, ULONG *, ULONG *, WCHAR *, WCHAR *,ULONG *, ULONG *, BOOL * ) ;
    BOOL       fnGetAttributeValue( WCHAR *, WCHAR *, WCHAR * ) ;
    BOOL       fnGetNextAttributeValue( WCHAR *, USHORT, WCHAR *, WCHAR *, WCHAR ** ) ;
    BOOL       fnGetCompleteString( WCHAR *, WCHAR *, WCHAR *, LONG ) ;
    USHORT     fnGetXMLRcd( WCHAR *, ULONG * ) ;
    USHORT     fnWriteBlock( USHORT, ULONG, ULONG, wofstream*, BOOL ) ;
    USHORT     fnReadBlock( USHORT, ULONG, ULONG, WCHAR*, ULONG ) ;
    BOOL       fnAllocateParaNode( P_INFO**, P_INFO**, WCHAR*, BOOL ) ;
    BOOL       fnAllocateRunNode( R_INFO**, R_INFO**, P_INFO*, WCHAR*, BOOL ) ;
    BOOL       fnAllocateTextNode( T_INFO**, T_INFO**, R_INFO*, WCHAR*, BOOL ) ;
    BOOL       fnAllocateRefNode( P_INFO**, P_INFO**, WCHAR*, ULONG*, ULONG, ULONG, ULONG ) ;
    BOOL       fnSaveRunProperties( R_INFO*, WCHAR* ) ;
    BOOL       fnIsEmptyTag( WCHAR * ) ;
    BOOL       fnCheckCommonProp( WCHAR * ) ;
    VOID       fnFreeParaList( P_INFO ** ) ;
    VOID       fnFreeRunList( R_INFO ** ) ;
    VOID       fnFreeTextList( T_INFO ** ) ;
    VOID       fnFreeSSNTList( SSNT_INFO ** ) ;
    BOOL       fnCheckSpreadsheetNonTrans( void ) ;
    BOOL       fnPrepSpreadsheetNonTrans( SSNT_INFO** ) ;


// Routines defined in EXPORT.CPP      ----------------------------------------------

    BOOL       PostExport(PSZ in, PSZ out, PSZ source, USHORT * ) ;
    USHORT     fnCreateDbcsList( DBCS_INFO ** ) ;
    BOOL       fnAllocateDbcsNode( DBCS_INFO**, DBCS_INFO**, WCHAR* ) ;
    VOID       fnFreeDbcsList( DBCS_INFO ** ) ;
    USHORT     fnGetDbcsBlock( ULONG, ULONG, WCHAR* ) ;
    VOID       fnAddTagAttr( WCHAR*, WCHAR*, WCHAR*, USHORT ) ;
    VOID       fnRemoveTagAttr( WCHAR*, WCHAR* ) ;
    BOOL       fnWriteReferenceText( P_INFO*, P_INFO*, P_INFO*, wofstream* ) ;
    BOOL       fnUpdateSpreadsheetNonTrans( PSZ, PSZ, SSNT_INFO* ) ;


// Routines defined in CHECK.CPP      ----------------------------------------------

    BOOL       CheckParseSegment(WCHAR *, CHK_INFO* , short *, WCHAR *, BOOL ) ;
    BOOL       CheckXmlText(WCHAR *, WCHAR *, WCHAR *, EQF_BOOL *, LONG, ULONG, EQF_BOOL ) ;

