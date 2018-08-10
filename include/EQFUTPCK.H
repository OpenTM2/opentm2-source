//------------------------------------------------------------------------------
//  EQFUTPCK.H - Public header file for packaging functions                   
//------------------------------------------------------------------------------
//  Copyright Notice:                                                         
//                                                                            
//      Copyright (C) 1990-2014, International Business Machines              
//      Corporation and others. All rights reserved                           
//------------------------------------------------------------------------------

#define FILEENTRIES_PER_ALLOC       32 // no of file list entries per alloc
#define FILENAMEBUFSIZE_PER_ALLOC 4096 // no of bytes in file name buffer per alloc

#define PACKHEADID "EQF\x1A"           // ID of package headers
#define PACKHEAD2ID "EQF\x1C"          // ID of package headers - version 2
#define PACKHEAD3ID "EQF\x1D"          // ID of package headers - version 3

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//   process states during packaging                                          //
//                                                                            //
//                                                                            //
//   PACK_NEW (state: package is new)
//       next state (after UtlPackAddFile): PACK_FILES_ADDED
//
//   PACK_FILESADDED (state: files have been added to package)
//       next state (after UtlPackAddFile): PACK_FILES_ADDED
//       next state (after UtlPackWritePackage): PACK_WRITEHEADER
//
//    ---- states during UtlPackWritePackage processing (while
//         UtlPackWritePackage is called repetively to process
//         packaging tasks) ----
//
//    PACK_WRITEHEADER
//       write header; set pointer to first file
//       if ok next state is PACK_NEWFILE else PACK_INERROR
//
//    PACK_NEWFILE
//       open new file; set file info in package header
//       if ok next state is PACK_READFILE else PACK_INERROR
//
//    PACK_READFILE
//       fill the input buffer with data from file
//       next state is PACK_COMPRESS
//
//    PACK_COMPRESS
//       compress data from input buffer to output buffer
//       next state is PACK_WRITEFILE
//
//    PACK_WRITEFILE
//       write output buffer to current file
//       if file is complete next state is PACK_CLOSEFILE
//       if an error occurs next state is PACK_INERROR
//       if target disk is full next state is PACK_DISKFULL
//       otherwise next state is PACK_READFILE
//
//    PACK_DISKFULL
//       set package header flag 'more disks to follow';
//       rewrite package header and close package file;
//       ask for a new disk; check disk free space
//       if user cancels next state is PACK_TERMINATED
//       otherwise next state is PACK_NEWDISK
//
//    PACK_NEWDISK
//       increment header sequence number;
//       write header and file tables;
//       if ok next state is PACK_WRITEFILE else PACK_INERROR
//
//    PACK_CLOSEFILE
//       close file; set file state in package header; skip to next file
//       if an error occurs next state is PACK_INERROR
//       if no more files are to process next state is PACK_COMPLETEPACKAGE
//       otherwise next state is PACK_NEWFILE
//
//    PACK_COMPLETEPACKAGE
//       set complete flag in package header and rewrite it;
//       close package file;
//       if ok next state is PACK_DONE else PACK_INERROR
//
//    PACK_DONE
//       return PACK_COMPLETE_RC
//       next state is PACK_DONE
//                                                                            //
//    PACK_INERROR
//       close any open file;
//       return PACK_INERROR_RC
//       next state is PACK_INERROR
//                                                                            //
//    PACK_TERMINATED
//       close any open file;
//       return PACK_TERMINATED_RC
//       next state is PACK_TERMINATED
//
//   PACK_OPEN (state: package is open for read access )
//
////////////////////////////////////////////////////////////////////////////////
typedef enum _PACKPROCESSSTATES
{
   PACK_NEW,                           // package is new and empty
   PACK_FILESADDED,                    // files have been added to package
   PACK_WRITEHEADER,                   // next task is: write package header
   PACK_NEWFILE,                       // next task is: begin a new file
   PACK_READFILE,                      // next task: read from file to buffer
   PACK_COMPRESS,                      // next task: compress data
   PACK_WRITEFILE,                     // next task: write file to package
   PACK_DISKFULL,                      // next task: get a new diskette
   PACK_NEWDISK,                       // next task: write header on new disk
   PACK_CLOSEFILE,                     // next task: close current file
   PACK_COMPLETEPACKAGE,               // next task: complete the package file
   PACK_WRITEFILELISTTOFIRSTDISK,      // next task: rewrite file list on 1st dsk
   PACK_DONE,                          // package has been completed
   PACK_INERROR,                       // packaging ended due to errors
   PACK_TERMINATED,                    // packaging was terminated by user
   PACK_OPEN,                          // packaging is opened for read access
   //PACK_CONVERT,                       // task: converting old tm to new one..
   PACK_CONVERT_NON_UNICODE,           // task: convert Unicode TM into non-Unicode TM
   PACK_CONVERT_NON_UNICODE_DA         // task: convert Unicode DA into non-Unicode DA
} PACKPROCESSSTATES;

// return codes of pack routines
typedef enum _PACKPRC
{
   PACK_OK_RC = 0,           // everything is ok, no error condition yet
   PACK_NOFILES_RC,          // nothing to do; no files in package
   PACK_COMPLETE_RC,         // package has been completed
   PACK_NOCONTROLAREA_RC,    // no ctrl area ptr has been passed to pack routine
   PACK_NOMEMORY_RC,         // out of main storage
   PACK_INVOPERATION_RC,     // the current package state doesn't allow this
                             // operation
   PACK_NODISKSPACE_RC,      // insufficient disk space to write header info
   PACK_OPENFAILED_RC,       // open on the package file failed
   PACK_WRITEFAILED_RC,      // a write operation on the package file failed
   PACK_READFAILED_RC,       // a read operation on a package file failed
   PACK_DOSERROR_RC,         // a DosXXXX call returned an error condition ...
   PACK_TERMINATE_RC,        // packaging has been terminated by the user
   PACK_INERROR_RC,          // packaging enden due to previously reported error
   PACK_HEADEREXISTS_RC,     // add failed, only one header per package allowed
   PACK_INVPROCESSSTATE_RC,  // internal pack error: invalid processing state
   PACK_NOHEADER_RC,         // the package does not have an user header
   PACK_FILENOTFOUND_RC,     // the specified file does not exist
   PACK_TOO_MUCH_DOCUMENTS_RC,         // there are to much documnets in the folder
   PACK_READ_FAILED_RC       // the specified file could not be read
} PACKRC;

//--- file list entry for export/import of files ---
typedef struct _FILELISTENTRY
{
   PSZ     pszName;                    // ptr to name of file (stored in buffer)
   ULONG   ulFileSize;                 // original size of file in bytes
   ULONG   ulPackedSize;               // packed size of file in bytes
   ULONG   ulFilePos;                  // position of file in package
   USHORT  usDiskNum;                  // number of disk containing file
   FDATE   stFileDate;                 // date of last write
   FTIME   stFileTime;                 // time of last write
   USHORT  usFileType;                 // type of file
   USHORT  usProcessFlags;             // status of file (see list below)
} FILELISTENTRY, *PFILELISTENTRY;

// constants for file process flags
#define FILE_NONE          0x00           // no processing done yet
#define FILE_STORE         0x00           // no compress to be done
#define FILE_INWORK        0x01           // file is currently processed
#define FILE_IS_PROCESSED  0x02           // file has been processed
#define FILE_COMPRESS      0x10           // file is (to be) compressed
#define FILE_SKIP          0x20                  // skip file (used for import)

//
// Values for types of packaged files
//
//    Note: 1) These values are used in folder packages with a header of type
//             RELEASE1_HEADER or higher. In packages of type RELEASE0_HEADER
//             the property class values (PROP_CLASS_....) are used instead.
//             So always examine the variable BitFlags.fHeaderType in the
//             header before using file types.
//          2) Add new values at the END of the list only! Otherwise you will
//             not be able to read older package files.
typedef enum _PACKFILETYPES
{
   DICTIONARY_DATA_FILE,               // base dictionary file  (xxx.ASD)
   DICTIONARY_INDEX_FILE,              // index dictionary file (xxx.ASI)
   DICTIONARY_PROP_FILE,               // dictionary property file (xxx.PRO)
   DOCUMENT_PROP_FILE,                 // document property file
   DOCUMENT_SEGSRC_FILE,               // document segmented source file
   DOCUMENT_SEGTGT_FILE,               // document segmented target file
   DOCUMENT_SRC_FILE,                  // document source file
   FOLDER_PROP_FILE,                   // folder property file
   MEMORY_DATA_FILE,                   // translation memory data file
   MEMORY_PROP_FILE,                   // translation memory property file
   MEMORY_TABLE_FILE,                  // translation memory table file
   TAGTABLE_DATA_FILE,                 // tagtable data file
   TAGTABLE_USEREXIT_FILE,             // tagtable user exit DLL (OS/2)
   NTMMEMORY_DATA_FILE,                // translation memory data (new format)
   NTMMEMORY_INDEX_FILE,               // translation memory index (new format)
   TAGTABLE_USEREXITWIN_FILE,          // tagtable user exit DLL (Windows)
   //NTMMEMORY_OLD_MEMORY,               // new tm will be exported as old one..
   //NTMMEMORY_OLD_PROP,                 // property file for converted TM..
   HISTLOG_DATA_FILE,                  // history log file
   DOCUMENT_EADATA_FILE,               // document EA data
   TAGTABLE_SETTINGS_FILE,             // tag table settings file (.CHR)
   SUBFOLDER_PROP_FILE,                // subfolder property file
   NTMMEMORY_NON_UNICODE,              // support non unicode memories
   NTMMEMORY_NON_UNICODE_INDEX,        // support non unicode memory index file
   DICTIONARY_NON_UNICODE,             // support non unicode dictionaries
   DICTIONARY_NON_UNICODE_INDEX,       // support non unicode dictionary index
   TAGTABLE_ADDITIONAL_FILE,           // additional files of user exit (supplied using QUERYEXITINFO)
   DOCUMENT_MTLOG_FILE,                // document MT log file
   DOCUMENT_MISC_FILE,                 // document MISC file
   REDSEGMENT_DATA_FILE,               // redundant segment list file
   DOCUMENT_XLIFF_FILE,                // document XLIFF file
   DOCUMENT_METADATA_FILE,             // document METADATA file
   ANALYSIS_PROFILE_FILE,              // analysis profile
   DOCUMENT_ENTITY_FILE,               // document entity file
   GLOBALMEMORYFILTER_FILE,            // global memory filter file 
   TAGTABLE_INFO_FILE,                 // tagtable information file
   MEMORY_INFO_FILE,                   // memory information file
   PLUGINMEMORY_DATA_FILE,             // data file of a memory belonging to a plugin
   BINCALCREPORT_FILE                  // binary calculation report data
} PACKFILETYPES;

//--- file list for export/import of files ---
typedef struct _FILELIST
{
   PFILELISTENTRY  pEntries;           // ptr to file list entries
   ULONG           ulListUsed;         // no of entries used in file list
   ULONG           ulListSize;         // no of entries allocated for file list
   PSZ             pBuffer;            // ptr to file name buffer
   ULONG           ulBufferUsed;       // bytes used in file name buffer
   ULONG           ulBufferSize;       // bytes allocated for file name buffer
} FILELIST, *PFILELIST;

typedef struct _PACKHEADER
{
   BYTE    bPackID[4];                 // package ID bytes
   USHORT  usVersion;                  // version of package pack method
   ULONG   ulPackDate;                 // date when package was created
   USHORT  usSequence;                 // package sequence number
   EQF_BOOL fCompleted;                // package file has been completed flag
   EQF_BOOL fLastFileOfPackage;        // no more package files to follow
   USHORT  usUserHeaderSize;           // size of user header
   USHORT  usFileListSize;             // size of file list [Bytes]
   USHORT  usFileListEntries;          // no of entries in file list
   USHORT  usFileNameBufferSize;       // size of file name buffer [Bytes]
} PACKHEADER, *PPACKHEADER;

typedef struct _PACKHEADER2
{
   BYTE    bPackID[4];                 // package ID bytes
   USHORT  usVersion;                  // version of package pack method
   ULONG   ulPackDate;                 // date when package was created
   USHORT  usSequence;                 // package sequence number
   EQF_BOOL fCompleted;                // package file has been completed flag
   EQF_BOOL fLastFileOfPackage;        // no more package files to follow
   ULONG   ulUserHeaderSize;           // size of user header
   ULONG   ulFileListSize;             // size of file list [Bytes]
   ULONG   ulFileListEntries;          // no of entries in file list
   ULONG   ulFileNameBufferSize;       // size of file name buffer [Bytes]
   CHAR    szToolID[4];                // ID of tool creating the package
   CHAR    chReserve[16];              // for future use
} PACKHEADER2, *PPACKHEADER2;



PVOID  UtlPackInit( CHAR, PSZ, PSZ, PSZ, BYTE, PSZ, HWND );
PVOID  UtlPackOpenPackage( CHAR, PSZ, PSZ, PSZ, HWND );
ULONG  UtlPackAddFile( PVOID, PSZ, USHORT, USHORT );
ULONG  UtlPackAddHeader ( PVOID, PBYTE, ULONG );
ULONG  UtlPackAddMatchingFiles( PVOID, PSZ, USHORT, USHORT );
ULONG  UtlPackClosePackage( PVOID pPackVoid );
BOOL   UtlPackIsInList( PVOID, PSZ );
ULONG  UtlPackReadFilePart
(
   PVOID  pVoidPack,                   // pointer to package control area
   PSZ    pszFile,                     // fully qualified file name
   ULONG  ulOffset,                    // offset within file
   PBYTE  pbBuffer,                    // buffer for the read data
   ULONG  ulLength                     // length of data to read
);

/**********************************************************************/
/* Defines for UtlPackListPackages functions                          */
/**********************************************************************/
#define PACKFLAG_WITHEXT       0x0001            // add extension to file name
#define PACKFLAG_LISTALL       0x0002            // list also incomplete pachages
ULONG  UtlPackListPackages( HWND hwndLB, CHAR chDrive, PSZ pszFolder,
                                                        PSZ pszFullPath, PSZ pszPackName, USHORT usFlags );
ULONG  UtlPackQueryHeaderSize( PVOID pPackVoid );
ULONG  UtlPackQueryRequiredSpace( PVOID pVoidPack, PULONG pulSpace );
ULONG  UtlPackReadFileList( PVOID pVoidPack );
ULONG  UtlPackReadHeader( PVOID, PBYTE, ULONG );
ULONG  UtlPackReadPackage( PVOID pPackVoid );
ULONG  UtlPackSetTargetDrive( PVOID pVoidPack, CHAR chDrive );
ULONG  UtlPackWritePackage( PVOID pPackVoid, PUSHORT pusCompleted );
ULONG  UtlPackWriteFileList( PVOID pVoidPack );
VOID   UtlPackCleanup( PVOID pPackVoid );
ULONG  UtlPackGetFileListPtr ( PVOID pVoidPack, PFILELIST *ppFileList );
BOOL   UtlPackHandleError( ULONG ulRC, PSZ pszParm, HWND hwnd );
ULONG  UtlPackQueryImportPath( PVOID pVoidPack,  PSZ pszImportPathBuffer );
ULONG  UtlPackGetLastRC( PVOID pVoidPack,  PULONG pusRC );
ULONG  UtlPackSetOwnerHwnd( PVOID, HWND );
ULONG  UtlPackSetImportDir( PVOID, PSZ );

// Function setting the values for a memory rename during folder export
// Note: The function has to be called before the actual folder export
//       (UtlPackWritePackage) has been started
ULONG UtlPackRenameMemoryDuringExport
( 
  PVOID       pVoidPack,               // package control pointer
  PVOID       pvRenameData             // pointer to memory rename data area
);

// Defines for different folder export versions
//#define EXP_IN_VERSION1    1   // TM/2 version 1 format
#define EXP_IN_NON_UNICODE 2   // non unicode TM/2 format
