//------------------------------------------------------------------------------
//  EQFFOL03.C - EQF Folder Handler dialog procedures; Pack tools             
//------------------------------------------------------------------------------
//  Copyright Notice:                                                         
//                                                                            
//      Copyright (C) 1990-2014, International Business Machines              
//      Corporation and others. All rights reserved                           
//------------------------------------------------------------------------------

#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_PRINT            // print functions
#define INCL_FOLDER               // folder defines
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_ANALYSIS
//#define INCL_EQF_DAM
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file
#include "eqftmi.h"
#include <eqfdasdi.h>             // Private dictionary services defines ...
#include "OtmDictionaryIF.H"
#include <eqfdde.h>               // batch mode definitions
#include <eqffol00.h>             // Private folder defines

#include "eqfutpck.h"             // our header file
#include "eqfrdics.h"             // remote dictionary functions
#include <eqfdtag.h>              // include tag definitions
#include "eqfdicti.h"             // dictionary stuff
#include <time.h>                 // C standard time functions
#include <dos.h>

#include "EQFHLOG.H"            // defines for history log processing

#include "vector"
#include "core\pluginmanager\PluginManager.h"
#include "core\pluginmanager\OtmMemory.h"
#include "core\pluginmanager\OtmMemoryPlugin.h"
#include "core\memory\MemoryFactory.h"


// structure for memory conversion request
#define NTM_COPY           0x00           // fill the export area...
#define NTM_CONVERT        0x01           // convert new TM into old TM
#define NTM_PREP_COPY      0x02           // prepare for export
#define NTM_PREP_CONVERT   0x03           // prepare for conversion

#define NTM_TEMPPROP_EXT   ".NTM"         // new translation memory prop ext.

// activate the following define to show debug messages for the UtlListPackages function
#if _DEBUG
  // #define DEBUG_LISTPACKAGES
#endif


#define PACKAGETOOLID "OTM"

typedef struct _NTM_TO_OLD_TM
{
   HTM      hTMNewTM;                  // memory database handle
   CHAR     szNewTMName[MAX_EQF_PATH];
   USHORT   usAction;                  // action (either packing or reading)
   CHAR     szPropName[MAX_EQF_PATH];
   CHAR     szTempPropName[MAX_EQF_PATH];
   PTMX_EXT_IN  pstExtIn;              // extract input struct
   PTMX_EXT_OUT pstExtOut;             // extract output
   PREP_IN  pstRepIn;                  // replace in
   PREP_OUT pstRepOut;                 // replace out
} NTM_TO_OLD_TM, *PNTM_TO_OLD_TM;

typedef struct _NTM_TO_NON_UNICODE_TM
{
   HTM      hTMNewTM;                  // memory database handle of newly created tm
   HTM      hTMExistTM;                // memory database handle of existing tm
   CHAR     szExistTMName[MAX_EQF_PATH];
   USHORT   usAction;                  // action (either packing or reading)
   CHAR     szDataFile[MAX_EQF_PATH];
   CHAR     szIndexFile[MAX_EQF_PATH];
   PTMX_EXT_IN_W  pstExtInW;           // extract input struct
   PTMX_EXT_OUT_W pstExtOutW;          // extract output
   PTMX_PUT_IN    pstPutIn;
   PTMX_PUT_OUT   pstPutOut;
} NTM_TO_NON_UNICODE_TM, *PNTM_TO_NON_UNICODE_TM;


typedef struct _NTM_TO_NON_UNICODE_DA
{
//   HTM      hTMNewTM;                  // memory database handle of newly created tm
//   HTM      hTMExistTM;                // memory database handle of existing tm
//   CHAR     szExistTMName[MAX_EQF_PATH];
   USHORT   usAction;                  // action (either packing or reading)
   CHAR     szDataFile[MAX_EQF_PATH];
   CHAR     szIndexFile[MAX_EQF_PATH];

   DORGIDA    stDorgIda;

//   PTMX_EXT_IN_W  pstExtInW;           // extract input struct
//   PTMX_EXT_OUT_W pstExtOutW;          // extract output
//   PTMX_PUT_IN    pstPutIn;
//   PTMX_PUT_OUT   pstPutOut;
} NTM_TO_NON_UNICODE_DA, *PNTM_TO_NON_UNICODE_DA;




typedef struct _PACKCONTROL
{
   USHORT       ProcessState;          // package processing state
   HWND         hwndOwner;             // owner window handler
                                       // for batch operations
   CHAR         chTargetDrive;         // target drive for package
   CHAR         szTargetDrive[3];      // target drive as string
   BOOL         fPackPath;             // package path flag
   CHAR         szPackPath[CCHMAXPATH+1];        // optional path of package
   CHAR         szPackName[CCHMAXPATH+1];        // name of package file
   BOOL         fFolder;               // use folder directory flag
   CHAR         szFolder[CCHMAXPATH+1];     // name of folder directory
   CHAR         szFullPackName[CCHMAXPATH+1];    // fully qualified package name
   CHAR         szFileName[CCHMAXPATH+1];   // general purpose file name buffer
   PACKHEADER   OldPackHead;           // buffer for version 1 headers
   PACKHEADER2  PackHead;              // header of current package
   PACKHEADER2  TempHead;              // header of other packages
   FILELIST     FileList;              // file list of current package
   PFILELISTENTRY  pCurFile;           // file currently processed/packaged
   PVOID        pUserHeader;           // pointer to user header
   HFILE        hFilePackage;          // handle of package file
   HFILE        hFilePacked;           // handle of file currently packaged
   ULONG        ulBytesRemaining;      // bytes remaining of file in work
   PBYTE        pInBuf;                // input buffer
   ULONG        ulInBufSize;           // size of input buffer
   ULONG        ulInBufUsed;           // bytes used in input buffer
   PBYTE        pOutBuf;               // output buffer
   ULONG        ulOutBufSize;          // size of output buffer
   ULONG        ulOutBufUsed;          // bytes used in output buffer
   PSZ          pszErrParm[10];        // table for error parameter ptr
   ULONG        ulCompleted;           // # of bytes completed
   ULONG        ulTotalSize;           // # of bytes to be processed
   CHAR         szImportPath[CCHMAXPATH+1]; // path to imported files
   USHORT       usDosRC;               // code returned by DosXXX calls
   HTM          hTM;                   // memory database handle
   HDCB         hDict;                 // handles for asd plus index file
   HUCB         hDictUser;             // ASD user handle
   GETPART_IN   GetPartIn;             // TM GetPart input structure
   GETDICTPART_IN GetDictPartIn;       // Dict GetPart input structure
   GETPART_OUT  GetPartOut;            // TM GetPart output structure
   CHAR         szServerName[MAX_SERVER_NAME];// servername
   CHAR         szMarkup[ MAX_EQF_PATH ];
   CHAR         szTgtLng[ MAX_LANG_LENGTH ];
   CHAR         szSrcLng[ MAX_LANG_LENGTH ];
   NTM_TO_OLD_TM NTMOld;               // structure for memory request
   BYTE         bVersion;              // folder is exported in older format
   CHAR         chTempFile[ MAX_EQF_PATH+1 ];  // space for temp filename
   NTM_TO_NON_UNICODE_TM NTMNonUnicode;     // structure for memory request
   NTM_TO_NON_UNICODE_DA NTMNonUnicodeDA;   // structure for dict requests
   BOOL         fFirstRead;            // TRUE = this is the first read from the current file
   // variables used by memory rename function
   BOOL         fMemoryRename;         // TRUE = the folder memory is to be rename during export
   //CHAR         szOldMemShortName[MAX_FILESPEC]; // buffer for old memory name (short)
   //CHAR         szOldMemLongName[MAX_LONGFILESPEC]; // buffer for old memory name (long)
   //CHAR         szNewMemLongName[MAX_LONGFILESPEC]; // buffer for new memory name (long)
   //CHAR         szNewMemShortName[MAX_FILESPEC]; // buffer for new memory name (short)
   PFOLMEMRENAMEDATA pMemRename;           // data area for memory renames
   CHAR         szTempName[MAX_LONGFILESPEC]; // buffer for temporary names
   CHAR         szImportDir[CCHMAXPATH+1]; // name of import directory
   OtmMemory::MEMORYPARTDATA MemPartData;  // data area for getMemoryPart method
   CHAR         szInfoFileLine[MAX_LONGFILESPEC]; // buffer for lines from info file
   CHAR         szMemory[MAX_LONGFILESPEC]; // buffer for memory name
   CHAR         szPlugin[MAX_LONGFILESPEC]; // buffer for plugin name
   CHAR         szMemFiles[8096];          // buffer for memory files
   OtmMemory    *pMemory;                 // active memory object
} PACKCONTROL, *PPACKCONTROL;

#define PACKVERSION1   1               // version number of packaging method
#define PACKVERSION2   2               // version number of packaging method

static ULONG UtlPackWriteSysHead( PVOID pVoidPack, PPACKHEADER2 pHead );
static ULONG UtlPackReadSysHead( PPACKCONTROL pPackCtrl, PPACKHEADER2 pHead );
static ULONG UtlPackWriteHeader( PPACKCONTROL pPackCtrl, PPACKHEADER2 pPackHead );
static ULONG  PackNewFile       ( PPACKCONTROL pPackCtrl, PSZ pFileName );

static USHORT AsdIntoNonUnicodeDAInit ( PPACKCONTROL pPackCtrl );
static USHORT AsdIntoNonUnicodeDAClose ( PPACKCONTROL pPackCtrl );
static USHORT AsdIntoNonUnicodeDAWork ( PPACKCONTROL pPackCtrl );

// UtlPackAddMatchingFiles - add all files which match a given file specification
ULONG  UtlPackAddMatchingFiles
(
   PVOID  pVoidPack,                   // pointer to package control area
   PSZ    pszMatchName,                // file specification of files to add
   USHORT usType,                      // user file type
   USHORT usPackType                   // type of package method
)
{
   CHAR        chTempName[MAX_EQF_PATH]; // buffer for file names
   FILEFINDBUF ResultBuf;               // DOS file find struct
   PSZ     pszName = RESBUFNAME(ResultBuf);   // ptr to name in result buffer
   USHORT  usCount = 1;
   HDIR    hDirHandle = HDIR_CREATE;    // DosFind routine handle
   ULONG   ulRC = PACK_OK_RC;           // code returned to caller
   USHORT  usDosRC;                     // return code of Dos... alias Utl...
   PSZ     pszFileName;                 // position of filename in MatchName

   strcpy( chTempName, pszMatchName );
   pszFileName = UtlGetFnameFromPath( chTempName );

   usDosRC = UtlFindFirst( pszMatchName, &hDirHandle, FILE_NORMAL,
                        &ResultBuf, sizeof( ResultBuf),
                        &usCount, 0L, FALSE );

   while ( usCount && (ulRC == PACK_OK_RC) )
   {
      strcpy( pszFileName, pszName );
      ulRC = UtlPackAddFile( pVoidPack, chTempName, usType, usPackType );
      if ( ulRC == PACK_OK_RC )
      {
         usDosRC = UtlFindNext( hDirHandle, &ResultBuf,
                             sizeof( ResultBuf), &usCount, FALSE );
      } /* endif */
   } /* endwhile */

   // close search file handle
   if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );

   return( ulRC );
}

//@bt
PVOID UtlPackInit
(
  CHAR chTargetDrive,
  PSZ pszFolder,
  PSZ pszFullPath,
  PSZ pszPackName,
  BYTE bVersion,
  PSZ pObjName,
  HWND hwndOwner
)
{
   PPACKCONTROL pPackCtrl;             // ptr to package control area

   //--- allocate package control area ---
   UtlAllocHwnd( (PVOID *)&pPackCtrl, 0L, (LONG) sizeof(PACKCONTROL), ERROR_STORAGE,
                 hwndOwner );

   //--- initialize pack control variables ---
   if ( pPackCtrl )
   {
      UtlAllocHwnd( (PVOID *)&pPackCtrl->pInBuf, 0L, 0x7FFFL, ERROR_STORAGE, hwndOwner );
      pPackCtrl->ulInBufSize = 0x7FFF;
      UtlAllocHwnd( (PVOID *)&pPackCtrl->pOutBuf, 0L, 0x7FFFL, ERROR_STORAGE, hwndOwner );
      pPackCtrl->ulOutBufSize = 0x7FFF;

      pPackCtrl->ProcessState  = PACK_NEW;

      pPackCtrl->chTargetDrive = chTargetDrive;
      pPackCtrl->szTargetDrive[0] = chTargetDrive;

      pPackCtrl->szTargetDrive[1] = EOS;
      strcpy( pPackCtrl->szPackName, pszPackName );

      if ( pszFullPath && (pszFullPath[0] != EOS ))
      {
           strcpy( pPackCtrl->szPackPath, pszFullPath );
           pPackCtrl->fPackPath = TRUE;
      }
      else
      {
         pPackCtrl->fPackPath = FALSE;
      }

      if ( pszFolder && (pszFolder[0] != EOS))
      {
         strcpy( pPackCtrl->szFolder, pszFolder );
         pPackCtrl->fFolder = TRUE;
      }
      else
      {
         pPackCtrl->fFolder = FALSE;
      } /* endif */

//      if ( bVersion == EXP_IN_VERSION1 )
//      {
//        memcpy( pPackCtrl->PackHead.bPackID, PACKHEADID, 4 );
//      }
//      else
//      {
        memcpy( pPackCtrl->PackHead.bPackID, PACKHEAD2ID, 4 );
//      } /* endif */
      pPackCtrl->bVersion = bVersion;
      pPackCtrl->PackHead.usVersion = PACKVERSION2;
      UtlTime( (PLONG)&pPackCtrl->PackHead.ulPackDate );
      pPackCtrl->PackHead.usSequence = 1;
      pPackCtrl->PackHead.fCompleted = FALSE;
      pPackCtrl->PackHead.ulFileListSize   = 0;
      pPackCtrl->PackHead.ulFileListEntries = 0;
      pPackCtrl->PackHead.ulFileNameBufferSize = 0;
      strcpy( pPackCtrl->PackHead.szToolID, PACKAGETOOLID );


      if ( pObjName )
      {
        CHAR  chFName[ MAX_EQF_PATH ];
        PPROPSYSTEM   pSysProp;        // ptr to EQF system properties
        PVOID         hFolProp;        // handle of folder properties
        EQFINFO       ErrorInfo;       // property error return code
        PPROPFOLDER   pFolProp;        // ptr to folder properties


        strcpy( chFName, pObjName );
        pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
        chFName[0] = pSysProp->szPrimaryDrive[0];
        hFolProp = OpenProperties( chFName, NULL,
                                   PROP_ACCESS_READ,
                                   &ErrorInfo);
        if ( hFolProp )
        {
          pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );

          strcpy( pPackCtrl->szMarkup, pFolProp->szFormat );
          strcpy( pPackCtrl->szTgtLng, pFolProp->szTargetLang );
          strcpy( pPackCtrl->szSrcLng, pFolProp->szSourceLang );

          CloseProperties( hFolProp, PROP_QUIT, &ErrorInfo );
        } /* endif */
      } /* endif */
   } /* endif */

   //--- return package control area pointer to caller
   return( pPackCtrl );
} /* end of UtlPackInit */



ULONG UtlPackAddFile
(
   PVOID  pVoidPack,                   // pointer to package control area
   PSZ    pszFile,                     // fully qualified file name
   USHORT usType,                      // user file type
   USHORT usPackType                   // type of package method for file
)
{
   PPACKCONTROL pPackCtrl;
   ULONG    ulNameLength = 0;              // length of file name in bytes
   ULONG    ulNoOfEntries;             // no of file list entries to process
   PSZ      pOldBuffer;                // adress of old file name buffer
   PFILELIST pFileList = NULL;         // ptr to filelist structure
   PFILELISTENTRY pFileEntry;          // ptr to an entry in a file list
   LONG     lOldSize;                  // old table or buffer size
   LONG     lNewSize;                  // new table or buffer size
   ULONG    ulRC = PACK_OK_RC;         // code returned to caller
   BOOL     fOK;                       // internal ok flag
   FILEFINDBUF ResultBuf;              // DOS file find struct
   USHORT  usCount = 1;                // counter
   HDIR    hDirHandle = HDIR_CREATE;   // DosFind routine handle
   USHORT  usDosRC;                    // return code of Dos... alias Utl...
   USHORT  usTMRC;                     // return code of TM function calls
   LONG        lCheckAlloc;

   memset(&ResultBuf, 0, sizeof(ResultBuf));
   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( (pPackCtrl->ProcessState != PACK_NEW) &&
             (pPackCtrl->ProcessState != PACK_FILESADDED) )
   {
      //--- in the current process state no files can be added to package ---
      ulRC = PACK_INVOPERATION_RC;
   } /* endif */

   //--- get pointer to filelist structure ---
   if ( ulRC == PACK_OK_RC )
   {
      pFileList = &pPackCtrl->FileList;

      //--- enlarge file list table if required
      if ( pFileList->ulListUsed >= pFileList->ulListSize )
      {
         lOldSize =  (LONG) sizeof(FILELISTENTRY) * pFileList->ulListSize;
         lNewSize =  (LONG) sizeof(FILELISTENTRY) *
                     (LONG) (pFileList->ulListSize + FILEENTRIES_PER_ALLOC);
         fOK = UtlAllocHwnd( (PVOID *)&pFileList->pEntries, lOldSize, lNewSize,
                             ERROR_STORAGE, pPackCtrl->hwndOwner );
         if ( fOK )
         {
            pFileList->ulListSize += FILEENTRIES_PER_ALLOC;
         }
         else
         {
            ulRC = PACK_NOMEMORY_RC;
         } /* endif */
      } /* endif */
   } /* endif */

   //--- check if file exists (for non-TM files only!) ---
   if ( ulRC == PACK_OK_RC )
   {
     BOOL fFound = TRUE;
     switch (usType)
     {
       case NTMMEMORY_DATA_FILE:
       case NTMMEMORY_INDEX_FILE:
       case MEMORY_DATA_FILE:
         hDirHandle = FindFirstFile( pszFile, &ResultBuf );
         if ( hDirHandle == INVALID_HANDLE_VALUE )
         {
           usTMRC = (USHORT)GetLastError();
           fFound = FALSE;
         }
         else
         {
           FindClose( hDirHandle );
         }
         break;
       case DICTIONARY_DATA_FILE:
       case DICTIONARY_INDEX_FILE:
         hDirHandle = FindFirstFile( pszFile, &ResultBuf );
         if ( hDirHandle == INVALID_HANDLE_VALUE )
         {
           usTMRC = (USHORT)GetLastError();
           fFound = FALSE;
         }
         else
         {
           FindClose( hDirHandle );
         }
         break;
       default:
         hDirHandle = FindFirstFile( pszFile, &ResultBuf );
         if ( hDirHandle == INVALID_HANDLE_VALUE )
         {
           usDosRC = (USHORT)GetLastError();
           fFound = FALSE;
         }
         else
         {
           FindClose( hDirHandle );
         }
         break;
     } /* endswitch */

     if ( !fFound )
     {
       ulRC = PACK_FILENOTFOUND_RC;
     } /* endif */
   } /* endif */

   //--- enlarge file name buffer if required ---
   if ( ulRC == PACK_OK_RC )
   {
     ULONG ulNameWOPathLength;

      // to allow renaming of the files within the buffer (for memory rename function
      // during folder export) we have to ensure that the buffer for the file name is large
      // enough to contain names with full 8+3 characters

      ulNameLength = strlen( pszFile ) + 1;
      ulNameWOPathLength = strlen( UtlGetFnameFromPath( pszFile ) );

      // adjustment by GQ: as the memory info file can have longer names we have to add 
      // more space for this file type. The maximum name length is MAX_FILESPEC (for folder)
      // + MAX_NAME (for memory short name) + 1 (length of dash) + 8 (length of ".MEMINFO")
      if ( usType == MEMORY_INFO_FILE )
      {
        ULONG ulMaxNameLength = MAX_FILESPEC + MAX_FNAME + 1 + 8;
        if ( ulNameWOPathLength < ulMaxNameLength )
        {
          ulNameLength = ulNameLength + ulMaxNameLength - ulNameWOPathLength;
        } /* endif */
      }
      else if ( ulNameWOPathLength < MAX_FILESPEC )
      {
        ulNameLength = ulNameLength + MAX_FILESPEC - ulNameWOPathLength;
      } /* endif */


      if ( pFileList->ulBufferUsed + ulNameLength >= pFileList->ulBufferSize )
      {
        //--- can the memory be allocated by UtlAlloc ? ----
        lCheckAlloc = (LONG)pFileList->ulBufferSize + FILENAMEBUFSIZE_PER_ALLOC;
        {
           pOldBuffer = pFileList->pBuffer;        // remember old buffer location

           //// convert file name pointers to offsets
           //{
           //   pFileEntry = pFileList->pEntries;
           //   usNoOfEntries = (USHORT)pFileList->ulListUsed;
           //   while ( usNoOfEntries )
           //   {
           //      pFileEntry->pszName = pFileList->pBuffer +
           //                            (pFileEntry->pszName - pOldBuffer);
           //      pFileEntry++;
           //      usNoOfEntries--;
           //   } /* endwhile */
           //}


           fOK = UtlAllocHwnd( (PVOID *)&pFileList->pBuffer,
                           (LONG) pFileList->ulBufferSize,
                           (LONG) (pFileList->ulBufferSize +
                                   FILENAMEBUFSIZE_PER_ALLOC),
                           ERROR_STORAGE, pPackCtrl->hwndOwner );
           if ( !fOK )
           {
              ulRC = PACK_NOMEMORY_RC;
           }
           else
           {
              //--- set new buffer size ---
              pFileList->ulBufferSize += FILENAMEBUFSIZE_PER_ALLOC;

              //--- correct file name pointers in filelist table ---
              pFileEntry = pFileList->pEntries;
              ulNoOfEntries = pFileList->ulListUsed;
              while ( ulNoOfEntries )
              {
                 pFileEntry->pszName = pFileList->pBuffer + (pFileEntry->pszName - pOldBuffer);
                 pFileEntry++;
                 ulNoOfEntries--;
              } /* endwhile */
           } /* endif */
        }
      } /* endif */

   } /* endif */

   //--- add new entry to file list ---
   if ( ulRC == PACK_OK_RC )
   {
      pFileEntry = pFileList->pEntries + pFileList->ulListUsed;
      pFileEntry->pszName = pFileList->pBuffer + pFileList->ulBufferUsed;
      memcpy( pFileList->pBuffer + pFileList->ulBufferUsed,
              pszFile, ulNameLength );
      pFileList->ulListUsed++;
      pFileList->ulBufferUsed += ulNameLength;
      pFileEntry->ulFileSize = RESBUFSIZE(ResultBuf);
      FileTimeToDosDateTime( &ResultBuf.ftLastWriteTime,
                               (LPWORD)&pFileEntry->stFileDate,
                               (LPWORD)&pFileEntry->stFileTime );
      pFileEntry->usFileType = usType;
      pFileEntry->usProcessFlags = (usPackType & FILE_COMPRESS);

      pPackCtrl->ProcessState = PACK_FILESADDED;  // set new process state
   } /* endif */

   return( ulRC );
} /* end of UtlPackAddFile */

BOOL   UtlPackIsInList
(
   PVOID  pVoidPack,                   // pointer to package control area
   PSZ    pszFile                      // fully qualified file name
)
{
   PPACKCONTROL pPackCtrl;
   PFILELIST pFileList = NULL;         // ptr to filelist structure
   PFILELISTENTRY pFileEntry;          // ptr to an entry in a file list
   USHORT   usRC = PACK_OK_RC;         // internal error code
   BOOL     fInList = FALSE;           // function return code
   ULONG    ulI;                       // loop index

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      usRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   } /* endif */

   //--- get pointer to filelist structure ---
   if ( usRC == PACK_OK_RC )
   {
      pFileList = &pPackCtrl->FileList;
   } /* endif */

   //--- search file in file list ---
   if ( usRC == PACK_OK_RC )
   {
      pFileEntry = pFileList->pEntries;
      for ( ulI = 0; ulI < pFileList->ulListUsed; ulI++ )
      {
        if ( stricmp( pFileEntry->pszName, pszFile ) == 0 )
        {
          fInList = TRUE;
          break;
        } /* endif */
        pFileEntry++;
      } /* endfor */
   } /* endif */

   return( fInList );
} /* end of UtlPackIsInList */




ULONG UtlPackAddHeader
(
   PVOID   pVoidPack,                  // ptr to package control structure
   PBYTE   pHeader,                    // ptr to user header area
   ULONG   ulHeaderSize                // size of user header
   )
{
   PPACKCONTROL pPackCtrl;
   USHORT   usRC = PACK_OK_RC;         // code returned to caller
   BOOL     fOK;                       // internal ok flag

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      usRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( (pPackCtrl->ProcessState != PACK_NEW) &&
             (pPackCtrl->ProcessState != PACK_FILESADDED) )
   {
      //--- in the current process state no user header can be added to package ---
      usRC = PACK_INVOPERATION_RC;
   }
   else if ( (pPackCtrl->pUserHeader != NULL) &&
             (pPackCtrl->PackHead.ulUserHeaderSize != 0) )
   {
      //--- only one user header per package allowed ---
      usRC = PACK_HEADEREXISTS_RC;
   } /* endif */

   //--- make a copy of user header area ---
   if ( usRC == PACK_OK_RC )
   {
      fOK = UtlAllocHwnd( &pPackCtrl->pUserHeader, 0L,
                      (LONG) ulHeaderSize,
                      ERROR_STORAGE, pPackCtrl->hwndOwner );

      if ( !fOK )
      {
         usRC = PACK_NOMEMORY_RC;
      }
      else
      {
         memcpy( pPackCtrl->pUserHeader, pHeader, ulHeaderSize );
         pPackCtrl->PackHead.ulUserHeaderSize = ulHeaderSize;
      } /* endif */
   } /* endif */

   return( usRC );
} /* end of UtlPackAddHeader */

ULONG UtlPackSetOwnerHwnd
(
   PVOID   pVoidPack,                  // ptr to package control structure
   HWND    hwndOwner                   // window handle of new owder
)
{
   PPACKCONTROL pPackCtrl;
   ULONG   ulRC = PACK_OK_RC;         // code returned to caller

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else
   {
      pPackCtrl->hwndOwner = hwndOwner;
   } /* endif */

   return( ulRC );
} /* end of UtlPackSetOwnerHwnd */


ULONG  UtlPackSetImportDir
(
   PVOID   pVoidPack,                  // ptr to package control structure
   PSZ     pszDir                      // import directory name
)
{
   PPACKCONTROL pPackCtrl;
   ULONG   ulRC = PACK_OK_RC;         // code returned to caller

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else
   {
      strcpy( pPackCtrl->szImportDir, pszDir );
   } /* endif */

   return( ulRC );
} /* end of UtlPackSetImportDir */


VOID UtlPackCleanup( PVOID pVoidPack )
{
   PPACKCONTROL pPackCtrl;             // ptr to package control area

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- free memory and close files ---
   if ( pPackCtrl )
   {
      if ( pPackCtrl->pInBuf )
      {
         UtlAlloc( (PVOID *)&pPackCtrl->pInBuf, 0L, 0L, NOMSG );
      } /* endif */

      if ( pPackCtrl->pOutBuf )
      {
         UtlAlloc( (PVOID *)&pPackCtrl->pOutBuf, 0L, 0L, NOMSG );
      } /* endif */

      if ( pPackCtrl->hFilePackage )
      {
         UtlClose( pPackCtrl->hFilePackage, FALSE );
         pPackCtrl->hFilePackage = NULLHANDLE;
      } /* endif */

      if ( pPackCtrl->hFilePacked )
      {
         UtlClose( pPackCtrl->hFilePacked, FALSE );
         pPackCtrl->hFilePacked = NULLHANDLE;
      } /* endif */

      if ( pPackCtrl->hDict)
      {
        AsdClose(pPackCtrl->hDictUser, pPackCtrl->hDict );
        pPackCtrl->hDict = NULL;
      } /* endif */

      if (pPackCtrl->hDictUser)
      {
        AsdEnd( pPackCtrl->hDictUser );
        pPackCtrl->hDictUser = NULL;
      } /* endif */

      // check if we are converting a dictionary
      if ( pPackCtrl->hDict )
      {
        AsdIntoNonUnicodeDAClose( pPackCtrl );
      }


      //if ( pPackCtrl->hTM != NULLHANDLE )
      //{
      //  TmClose( pPackCtrl->hTM, pPackCtrl->pCurFile->pszName, FALSE, 0 );
      //  pPackCtrl->hTM = NULLHANDLE;
      //} /* endif */

     if ( pPackCtrl->FileList.pEntries )
     {
       UtlAlloc( (PVOID *)&pPackCtrl->FileList.pEntries, 0L, 0L, NOMSG );
     } /* endif */

     if ( pPackCtrl->FileList.pBuffer )
     {
       UtlAlloc( (PVOID *)&pPackCtrl->FileList.pBuffer, 0L, 0L, NOMSG );
     } /* endif */

     UtlAlloc( (PVOID *)&pPackCtrl, 0L, 0L, NOMSG );
   } /* endif */
} /* end of UtlPackCleanup */

// function setting the values for a memory rename during folder export
ULONG UtlPackRenameMemoryDuringExport
( 
  PVOID       pVoidPack,               // package control pointer
  PVOID       pvMemRenameData
)
{
  PPACKCONTROL pPackCtrl;
  BOOL fIsNew = FALSE;
  ULONG       ulRC = PACK_OK_RC;      // code returned to caller

  pPackCtrl = (PPACKCONTROL) pVoidPack;

  pPackCtrl->pMemRename = (PFOLMEMRENAMEDATA)pvMemRenameData;
  pPackCtrl->fMemoryRename = TRUE;

  return( ulRC );
}

ULONG UtlPackWritePackage( PVOID pVoidPack, PUSHORT pusCompleted )
{
   PPACKCONTROL pPackCtrl;
   ULONG       ulRC = PACK_OK_RC;      // code returned to caller
   ULONG64     ulFreeSpace;            // Free space on disk
   ULONG       ulMinSpace;             // Minimum of space required on disk
   USHORT      usMBCode;               // code returned by UtlError calls
   USHORT      usOpenAction;           // action performed by DosOpen
   ULONG       ulBytesToWrite;         // # of bytes to write to file
   ULONG       ulBytesWritten = 0;     // # of bytes written to file
   ULONG       ulBytesToRead;          // # of bytes to readfrom file
   ULONG       ulBytesRead;            // # of bytes read from file
   ULONG       ulNoOfEntries;          // no of file list entries to process
   PFILELISTENTRY pFileEntry;          // ptr to an entry in a file list
   BOOL        fOK;                    // internal OK flag
   USHORT      usErrDict;              // dict return code
   CHAR        szName[MAX_FNAME];      // buffer for name
   CHAR        chTempName[MAX_EQF_PATH];  // buffer for file names
   PSZ         pszName;                // ptr to dict name
   PPROPDICTIONARY pDictProp = NULL;   // ptr to dictioanry properties
   USHORT      us1, us2;               // dummy ushorts for asd call
   PVOID       pBTree;
   LONG        LHandle;

   PUCB        pUCB;                     // pointer to user control block

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   switch ( pPackCtrl->ProcessState )
   {
      //======================================================================//
      case PACK_NEW :                     // package is new and empty
         ulRC = PACK_NOFILES_RC;
         break;
      //======================================================================//
      case PACK_FILESADDED :              // files have been added to package

         //--- set size of filelist in package header ---
         pPackCtrl->PackHead.ulFileListSize   =
            pPackCtrl->FileList.ulListUsed * sizeof(FILELISTENTRY);
         pPackCtrl->PackHead.ulFileListEntries = pPackCtrl->FileList.ulListUsed;
         pPackCtrl->PackHead.ulFileNameBufferSize =
            pPackCtrl->FileList.ulBufferUsed;

         //--- initialize other stuff ---
         pPackCtrl->ulInBufUsed = 0;
         pPackCtrl->ulOutBufUsed = 0;
         pPackCtrl->ulCompleted = 0L;
         pPackCtrl->ulTotalSize = 0L;
         pFileEntry = pPackCtrl->FileList.pEntries;
         ulNoOfEntries = pPackCtrl->FileList.ulListUsed;
         while ( ulNoOfEntries )
         {
            pPackCtrl->ulTotalSize += pFileEntry->ulFileSize;
            pFileEntry++;
            ulNoOfEntries--;
         } /* endwhile */

         //--- delete any exported file with the same name

           if (pPackCtrl->fPackPath)
          {
              strcpy( pPackCtrl->szFullPackName, pPackCtrl->szPackPath);
           }
         else
         {
            UtlMakeEQFPath( pPackCtrl->szFullPackName, pPackCtrl->chTargetDrive,
                             EXPORT_PATH, NULL );
         }

         if ( pPackCtrl->fFolder )
         {
            strcat( pPackCtrl->szFullPackName, "\\" );
            strcat( pPackCtrl->szFullPackName, pPackCtrl->szFolder );
         } /* endif */
         strcat( pPackCtrl->szFullPackName, "\\" );
         strcat( pPackCtrl->szFullPackName, pPackCtrl->szPackName );
         UtlDelete( pPackCtrl->szFullPackName, 0L, FALSE );

         //--- check if target disk has enough space for headers and tables
         ulFreeSpace = UtlQueryFreeSpace( pPackCtrl->chTargetDrive,
                         !ISBATCHHWND(pPackCtrl->hwndOwner) ? TRUE : FALSE );
         if ( ulFreeSpace == (ULONG64)-1 )
         {
           // free space evaluation failed
           ulRC = PACK_DOSERROR_RC;
         }
         else
         {
           ulMinSpace  = pPackCtrl->PackHead.ulFileListSize +
                         pPackCtrl->PackHead.ulFileNameBufferSize +
                         pPackCtrl->PackHead.ulUserHeaderSize +
                         (LONG) sizeof(PACKHEADER2);
           if ( ulMinSpace > ulFreeSpace )
           {
              pPackCtrl->pszErrParm[0] = pPackCtrl->szTargetDrive;
              usMBCode = UtlErrorHwnd( ERROR_DISK_IS_FULL,
                                   MB_CANCEL,
                                   1,
                                   pPackCtrl->pszErrParm,
                                   EQF_ERROR, pPackCtrl->hwndOwner );
              ulRC = PACK_NODISKSPACE_RC;
           } /* endif */
         } /* endif */

         //--- create package name and export directory ---
         if ( ulRC == PACK_OK_RC )
         {
            if (pPackCtrl->fPackPath)
            {
                  strcpy( pPackCtrl->szFullPackName, pPackCtrl->szPackPath);
            }
            else
            {
                UtlMakeEQFPath( pPackCtrl->szFullPackName, pPackCtrl->chTargetDrive,
                               EXPORT_PATH, NULL );
            }

            if ( pPackCtrl->fFolder )
            {
               strcat( pPackCtrl->szFullPackName, "\\" );
               strcat( pPackCtrl->szFullPackName, pPackCtrl->szFolder );
            } /* endif */
            pPackCtrl->usDosRC = UtlMkMultDir( pPackCtrl->szFullPackName,
                            !ISBATCHHWND(pPackCtrl->hwndOwner) ? TRUE : FALSE );
            ulRC = ((pPackCtrl->usDosRC == 0)|| (pPackCtrl->usDosRC == ERROR_ACCESS_DENIED) ) ?
                    PACK_OK_RC : PACK_OPENFAILED_RC;
         } /* endif */

         //--- open package file ---
         if ( ulRC == PACK_OK_RC )
         {
            strcat( pPackCtrl->szFullPackName, "\\" );
            strcat( pPackCtrl->szFullPackName, pPackCtrl->szPackName );
            pPackCtrl->usDosRC = UtlOpenHwnd( pPackCtrl->szFullPackName,
                               &pPackCtrl->hFilePackage,
                               &usOpenAction, 0L,
                               FILE_NORMAL,
                               FILE_TRUNCATE | FILE_CREATE,
                               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                               0L,
                               TRUE, pPackCtrl->hwndOwner );
            ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_OPENFAILED_RC;
         } /* endif */

         //--- set new processing state ---
         pPackCtrl->ProcessState = PACK_WRITEHEADER;

         break;
      //======================================================================//
      case PACK_WRITEHEADER :             // next task is: write package header
         //--- write UtlPack package header ---
         if ( ulRC == PACK_OK_RC )
         {
            ulRC = UtlPackWriteSysHead( pPackCtrl, &pPackCtrl->PackHead );
         } /* endif */

         //--- write user package header ---
         if ( ulRC == PACK_OK_RC )
         {
            if ( pPackCtrl->pUserHeader &&
                 pPackCtrl->PackHead.ulUserHeaderSize )
            {
              ULONG ulRemaining = pPackCtrl->PackHead.ulUserHeaderSize;
              PSZ   pszPos = (PSZ)pPackCtrl->pUserHeader;
              while ( (pPackCtrl->usDosRC == 0) && ulRemaining )
              {
                ULONG ulBytesToWrite = min( 32000, ulRemaining );
                pPackCtrl->usDosRC = UtlWriteWoCheckHwnd (
                                        pPackCtrl->hFilePackage,
                                        pPackCtrl->pUserHeader,
                                        (USHORT)ulBytesToWrite,
                                        &ulBytesWritten,
                                        TRUE,
                                        pPackCtrl->hwndOwner );
                ulRemaining -= ulBytesToWrite;
                pszPos += ulBytesToWrite;
              } /* endwhile */
              ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
            } /* endif */
         } /* endif */

         //--- write file list ---
         if ( ulRC == PACK_OK_RC )
         {
            ulRC = UtlPackWriteFileList( pPackCtrl );
         } /* endif */

         //--- position to first file ---
         if ( ulRC == PACK_OK_RC )
         {
            pPackCtrl->pCurFile = pPackCtrl->FileList.pEntries;
         } /* endif */

         //--- set new processing state ---
         pPackCtrl->ProcessState = PACK_NEWFILE;
         break;
      //======================================================================//
      case PACK_NEWFILE :                 // next task is: begin a new file
         //--- open the file ---
         pPackCtrl->ProcessState = PACK_READFILE;
         switch ( pPackCtrl->pCurFile->usFileType )
         {
       //    case MEMORY_DATA_FILE: 
       //      ulRC = TmOpen( pPackCtrl->pCurFile->pszName,
       //                     &pPackCtrl->hTM,
       //                     EXCLUSIVE_FOR_GET_PART,
       //                     TM_LOCALREMOTE,
       //                     FALSE,
							//0);
       //      switch ( ulRC )
       //      {
       //         case NO_ERROR:
       //         case FILE_MIGHT_BE_CORRUPTED:
       //         case VERSION_MISMATCH:
       //         case CORRUPT_VERSION_MISMATCH:
       //            ulRC = PACK_OK_RC;
       //            break;
       //          default:
       //             MemRcHandlingHwnd( (USHORT)ulRC, pPackCtrl->pCurFile->pszName,
       //                           &pPackCtrl->hTM,
       //                           pPackCtrl->szServerName,
       //                           pPackCtrl->hwndOwner );
       //            ulRC = PACK_OPENFAILED_RC;
       //            break;
       //      } /* endif */

       //      if ( ulRC == PACK_OK_RC )
       //      {
       //         //--- start getting at begin of translation memory
       //         pPackCtrl->GetPartIn.ulFilePos = 0;

       //         pPackCtrl->ulBytesRemaining = pPackCtrl->pCurFile->ulFileSize;
       //         pPackCtrl->pCurFile->ulFilePos  = 0L;
       //         UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,  // query current pos
       //                        0L,
       //                        FILE_CURRENT,
       //                        &pPackCtrl->pCurFile->ulFilePos,
       //                        TRUE, pPackCtrl->hwndOwner );
       //         pPackCtrl->pCurFile->ulPackedSize = 0L;
       //         pPackCtrl->pCurFile->usDiskNum  = pPackCtrl->PackHead.usSequence;
       //         pPackCtrl->ulInBufUsed = 0;
       //         pPackCtrl->ulOutBufUsed = 0;
       //      } /* endif */
       //      break;

       //    case NTMMEMORY_DATA_FILE:
       //      ulRC = TmOpen( pPackCtrl->pCurFile->pszName,
       //                     &pPackCtrl->hTM,
       //                     EXCLUSIVE_FOR_GET_PART,
       //                     TM_LOCALREMOTE,
       //                     FALSE,
							//0);
       //      switch ( ulRC )
       //      {
       //         case NO_ERROR:
       //         case FILE_MIGHT_BE_CORRUPTED:
       //         case VERSION_MISMATCH:
       //         case CORRUPT_VERSION_MISMATCH:
       //            ulRC = PACK_OK_RC;
       //            break;
       //         default:
       //             MemRcHandlingHwnd( (USHORT)ulRC, pPackCtrl->pCurFile->pszName,
       //                           &pPackCtrl->hTM,
       //                           pPackCtrl->szServerName, pPackCtrl->hwndOwner );
       //            ulRC = PACK_OPENFAILED_RC;
       //            break;
       //      } /* endif */

       //      if ( ulRC == PACK_OK_RC )
       //      {
       //         // make TM data file the name of a local TM data file
       //         PSZ pszExt = strrchr( pPackCtrl->pCurFile->pszName, DOT );
       //         if ( pszExt )
       //         {
       //           strcpy( pszExt, EXT_OF_TMDATA );
       //         } /* endif */

       //          //get file handle of asd file for local dict
       //          NTMGetHandlesFromCLB( pPackCtrl->hTM,
       //                                &pPackCtrl->GetDictPartIn.pBTree,
       //                                &pBTree );

       //         //--- start getting at begin of translation memory
       //         pPackCtrl->GetDictPartIn.ulFilePos = 0;

       //         pPackCtrl->ulBytesRemaining = pPackCtrl->pCurFile->ulFileSize;
       //         pPackCtrl->pCurFile->ulFilePos  = 0L;
       //         UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,  // query current pos
       //                        0L,
       //                        FILE_CURRENT,
       //                        &pPackCtrl->pCurFile->ulFilePos,
       //                        TRUE, pPackCtrl->hwndOwner );
       //         pPackCtrl->pCurFile->ulPackedSize = 0L;
       //         pPackCtrl->pCurFile->usDiskNum  = pPackCtrl->PackHead.usSequence;
       //         pPackCtrl->ulInBufUsed = 0;
       //         pPackCtrl->ulOutBufUsed = 0;
       //      } /* endif */
       //      break;
       //    case NTMMEMORY_INDEX_FILE:
       //      //get file handle of asi file needed for local dict
       //      NTMGetHandlesFromCLB( pPackCtrl->hTM,
       //                            &pBTree,
       //                            &pPackCtrl->GetDictPartIn.pBTree );

       //      ulRC = PACK_OK_RC;
       //      //    pPackCtrl->hTM = (HTM)LHandle;

       //      pPackCtrl->GetDictPartIn.ulFilePos = 0;

       //      pPackCtrl->ulBytesRemaining = pPackCtrl->pCurFile->ulFileSize;
       //      pPackCtrl->pCurFile->ulFilePos  = 0L;
       //      UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,  // query current pos
       //                     0L,
       //                     FILE_CURRENT,
       //                     &pPackCtrl->pCurFile->ulFilePos,
       //                     TRUE, pPackCtrl->hwndOwner );
       //      pPackCtrl->pCurFile->ulPackedSize = 0L;
       //      pPackCtrl->pCurFile->usDiskNum  = pPackCtrl->PackHead.usSequence;
       //      pPackCtrl->ulInBufUsed = 0;
       //      pPackCtrl->ulOutBufUsed = 0;

       //      // make TM index file the name of a local TM index file
       //      {
       //        PSZ pszExt = strrchr( pPackCtrl->pCurFile->pszName, DOT );
       //        if ( pszExt )
       //        {
       //          strcpy( pszExt, EXT_OF_TMINDEX );
       //        } /* endif */
       //      }
       //      ulRC = PACK_OK_RC;
       //      break;

/*------------ Export in Non-Unicode ---------------------------------------*/
           case DICTIONARY_NON_UNICODE:
             pPackCtrl->NTMNonUnicodeDA.usAction = NTM_PREP_CONVERT;
             ulRC = AsdIntoNonUnicodeDAInit ( pPackCtrl );

             if ( !ulRC )
             {
               pPackCtrl->ProcessState = PACK_CONVERT_NON_UNICODE_DA;
             }
             else
             {
               ulRC = PACK_OPENFAILED_RC;
             }
             break;

           case DICTIONARY_NON_UNICODE_INDEX:
             pPackCtrl->NTMNonUnicodeDA.usAction = NTM_PREP_COPY;
             pPackCtrl->ProcessState = PACK_READFILE;
             break;

/*--------------------------------------------------------------------------*/
           case  DICTIONARY_DATA_FILE:
             //init DAM/TOLSTOY
             ulRC = AsdBegin( 2, &pPackCtrl->hDictUser );

             if ( ulRC != LX_RC_OK_ASD )
             {
               //dam error
               ulRC = PACK_OPENFAILED_RC;
             }
             else
             {
               UtlMakeEQFPath( chTempName, NULC, PROPERTY_PATH, NULL );
               strcat( chTempName, BACKSLASH_STR );
               Utlstrccpy( szName,
                  UtlGetFnameFromPath(pPackCtrl->pCurFile->pszName  ), DOT );
               strcat( chTempName, szName );
               strcat( chTempName, EXT_OF_DICTPROP );
               pszName = chTempName;
               ulRC = AsdOpen( pPackCtrl->hDictUser, ASD_LOCKED, 1,
                               &pszName,
                               &pPackCtrl->hDict, &usErrDict );


               if (  ulRC == LX_RC_OK_ASD )
               {
                 //if a remote dictionary lock it during export
//                 ulRC = AsdLockDict( pPackCtrl->hDict, TRUE );

                 //get original dictionary properties for server name
                 AsdRetPropPtr( pPackCtrl->hDictUser, pPackCtrl->hDict,
                                &pDictProp );
                 strcpy( pPackCtrl->GetDictPartIn.szServer,
                         pDictProp->szServer );

                 //get file handle of asd file for local dict
                 ulRC = AsdHandleFromDCB ( pPackCtrl->hDict, &us1, &us2,
                                           &pPackCtrl->GetDictPartIn.pBTree,
                                           &pBTree, &LHandle );

                 ulRC = PACK_OK_RC;
                 pPackCtrl->hTM = (HTM)LHandle;
               }
               else
               {
                 //qdam error
                 if ( ulRC == LX_READ_ONLY ) // language supportfile not found       /*KAT0022A*/
                 {                                                                   /*KAT0022A*/
                    pUCB = (PUCB) pPackCtrl->hDictUser; // convert handle to pointer /*KAT0022A*/
                    pszName = pUCB->cLangProp;                                 /*KAT0022A*/
                 }
                 else
                 {
                   pszName = szName;
                 } /* endif */
                 UtlErrorHwnd( (USHORT)ulRC, MB_CANCEL, 1, &pszName, QDAM_ERROR,
                               pPackCtrl->hwndOwner );

                 ulRC = PACK_OPENFAILED_RC;
               } /* endif */
             } /* endif */

             if ( ulRC == PACK_OK_RC )
             {
                // make dict data file the name of a local dict data file
                {
                  PSZ pszExt = strrchr( pPackCtrl->pCurFile->pszName, DOT );
                  if ( pszExt )
                  {
                    strcpy( pszExt, EXT_OF_DIC );
                  } /* endif */
                }

                //start getting at begin of asd file
                pPackCtrl->GetDictPartIn.ulFilePos = 0;

                pPackCtrl->ulBytesRemaining = pPackCtrl->pCurFile->ulFileSize;
                pPackCtrl->pCurFile->ulFilePos  = 0L;
                UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,  // query current pos
                               0L,
                               FILE_CURRENT,
                               &pPackCtrl->pCurFile->ulFilePos,
                               TRUE, pPackCtrl->hwndOwner );
                pPackCtrl->pCurFile->ulPackedSize = 0L;
                pPackCtrl->pCurFile->usDiskNum  = pPackCtrl->PackHead.usSequence;
                pPackCtrl->ulInBufUsed = 0;
                pPackCtrl->ulOutBufUsed = 0;
             } /* endif */

             break;
/*--------------------------------------------------------------------------*/
           case DICTIONARY_INDEX_FILE:
             //get file handle of asi file needed for local dict
             ulRC = AsdHandleFromDCB ( pPackCtrl->hDict, &us1, &us2, &pBTree,
                                 &pPackCtrl->GetDictPartIn.pBTree, &LHandle );

             ulRC = PACK_OK_RC;
             pPackCtrl->hTM = (HTM)LHandle;

             pPackCtrl->GetDictPartIn.ulFilePos = 0;

             pPackCtrl->ulBytesRemaining = pPackCtrl->pCurFile->ulFileSize;
             pPackCtrl->pCurFile->ulFilePos  = 0L;
             UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,  // query current pos
                            0L,
                            FILE_CURRENT,
                            &pPackCtrl->pCurFile->ulFilePos,
                            TRUE, pPackCtrl->hwndOwner );
             pPackCtrl->pCurFile->ulPackedSize = 0L;
             pPackCtrl->pCurFile->usDiskNum  = pPackCtrl->PackHead.usSequence;
             pPackCtrl->ulInBufUsed = 0;
             pPackCtrl->ulOutBufUsed = 0;
             ulRC = PACK_OK_RC;

             // make dict index file the name of a local dict index file
             {
               PSZ pszExt = strrchr( pPackCtrl->pCurFile->pszName, DOT );
               if ( pszExt )
               {
                 strcpy( pszExt, EXT_OF_DICTINDEX );
               } /* endif */
             }

             break;
/*--------------- determine if we have to convert Unicode to ASCII ---------*/
           case DOCUMENT_SEGSRC_FILE:
           case DOCUMENT_SEGTGT_FILE:
             switch (pPackCtrl->bVersion)
             {
               case EXP_IN_NON_UNICODE:
                 // get segmented file and convert it to non unicode
                 strcpy( pPackCtrl->chTempFile, pPackCtrl->pCurFile->pszName );
                 strcat( pPackCtrl->chTempFile, "$" );
                 if (pPackCtrl->ProcessState == DOCUMENT_SEGSRC_FILE)
                 {
                   ulRC = TASegFileConvertUnicode2ASCII( pPackCtrl->pCurFile->pszName,
                                                       pPackCtrl->chTempFile,
                                                       pPackCtrl->szSrcLng);
                 }
                 else
                 {
                   ulRC = TASegFileConvertUnicode2ASCII( pPackCtrl->pCurFile->pszName,
                                                       pPackCtrl->chTempFile,
                                                       pPackCtrl->szTgtLng);
                 }
                 if ( !ulRC )
                 {
                   ulRC = PackNewFile( pPackCtrl, pPackCtrl->chTempFile );
                 }
                 else
                 {
                   ulRC = PACK_WRITEFAILED_RC;
                 }
                 break;

               default:
                 ulRC = PackNewFile( pPackCtrl, pPackCtrl->pCurFile->pszName );
                 break;
             }
             break;

           case MEMORY_INFO_FILE:
             {
               pPackCtrl->szMemory[0] = EOS;
               pPackCtrl->szPlugin[0] = EOS;
               fOK = TRUE;

               // scan memory info file and extract memory name and memory plugin name
               {
                 FILE * hfInfoFile = fopen( pPackCtrl->pCurFile->pszName, "r" );
                 if ( hfInfoFile != NULL )
                 {
                   while ( !feof( hfInfoFile ) )
                   {
                     memset( pPackCtrl->szInfoFileLine, 0, sizeof(pPackCtrl->szInfoFileLine) );
                     fgets( pPackCtrl->szInfoFileLine, sizeof(pPackCtrl->szInfoFileLine), hfInfoFile );
                     if ( pPackCtrl->szInfoFileLine[0] != EOS )
                     {
                       pPackCtrl->szInfoFileLine[strlen(pPackCtrl->szInfoFileLine)-1] = EOS;

                       PSZ pszValue = strchr( pPackCtrl->szInfoFileLine, '=' );
                       if ( pszValue != NULL )
                       {
                         *pszValue = EOS;
                          pszValue += 1;

                        if ( strcmp( pPackCtrl->szInfoFileLine, MEMINFOFILE_PLUGIN_KEY ) == 0 )
                        {
                          strcpy( pPackCtrl->szPlugin, pszValue );
                        }
                        else if ( strcmp( pPackCtrl->szInfoFileLine, MEMINFOFILE_NAME_KEY ) == 0 )
                        {
                          strcpy( pPackCtrl->szMemory, pszValue );
                        }
                        else if ( strcmp( pPackCtrl->szInfoFileLine, MEMINFOFILE_FILES_KEY ) == 0 )
                        {
                          strcpy( pPackCtrl->szMemFiles, pszValue );
                        } /* endif */
                      } /* endif */
                    }
                  } /* endwhile */
                  fclose( hfInfoFile );
                 } /* endif */   
               }

                // in case of on-spot memory rename, rewrite info file with new data
                if ( pPackCtrl->fMemoryRename )
                {
                  int iMemIndex = FolImpGetRenameMemoryIndex( pPackCtrl->pMemRename, pPackCtrl->szMemory );
                  if ( iMemIndex >= 0 )
                  {
                    FILE * hfInfoFile = fopen( pPackCtrl->pCurFile->pszName, "w" );
                    if ( hfInfoFile != NULL )
                    {
                      fprintf( hfInfoFile, "%s=%s\n", MEMINFOFILE_NAME_KEY, pPackCtrl->pMemRename->aszNewMemNames[iMemIndex] );
                      fprintf( hfInfoFile, "%s=%s\n", MEMINFOFILE_PLUGIN_KEY, pPackCtrl->szPlugin );
                      fprintf( hfInfoFile, "%s=", MEMINFOFILE_FILES_KEY );
                      PSZ pszPos = pPackCtrl->szMemFiles;
                      while ( *pszPos != EOS )
                      {
                        PSZ pszEndPos = strchr( pszPos, ',' );
                        if ( pszEndPos == NULL ) pszEndPos = pszPos + strlen(pszPos);
                        CHAR chEndChar = *pszEndPos;
                        *pszEndPos = EOS;

                        CHAR szMemShortName[MAX_FILESPEC];
                        CHAR szExtension[10];

                        // get file name of entry
                        strcpy( pPackCtrl->szTempName, pszPos );
                        PSZ pszName = UtlGetFnameFromPath( pPackCtrl->szTempName );
                        strcpy( szMemShortName, pszName );

                        // cut off extension
                        {
                          PSZ pszExtension = strrchr( szMemShortName, DOT );
                          szExtension[0] = EOS;
                          if ( pszExtension )
                          {
                            strcpy( szExtension, pszExtension );
                            *pszExtension = EOS;
                          } /* endif */
                        }

                        // rename file if short name matches
                        if ( strcmp( szMemShortName, pPackCtrl->pMemRename->aszOldMemShortNames[iMemIndex] ) == 0 )
                        {
                          strcpy( pszName, pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex]  );
                          strcat( pszName, szExtension );
                          fprintf( hfInfoFile, "%s", pPackCtrl->szTempName );
                        }
                        else
                        {
                          fprintf( hfInfoFile, "%s", pszPos );
                        } /* endif */
                        fprintf( hfInfoFile, "%c", (chEndChar == EOS ) ? '\n' : ',' );

                        // process next entry
                        *pszEndPos = chEndChar;
                        pszPos = (*pszEndPos == EOS ) ? pszEndPos : pszEndPos + 1;
                      } /* endwhile */

                      fclose( hfInfoFile );
                  } /* endif */
                }
               }

               // open the memory
               {
                 int iRC = 0;
                 MemoryFactory *pFactory = MemoryFactory::getInstance();
                 pPackCtrl->pMemory = pFactory->openMemory( pPackCtrl->szPlugin, pPackCtrl->szMemory, NONEXCLUSIVE, &iRC );
                 if ( pPackCtrl->pMemory == NULL )
                 {
                   pFactory->showLastError( pPackCtrl->szPlugin, pPackCtrl->szMemory, NULL, pPackCtrl->hwndOwner );
                   fOK = FALSE;
                   ulRC = (ULONG)iRC;
                 } /* endif */        

               }

               // add this check, in case of memory open error
               if(fOK)
               {   
                   // use standard method for memory info file
                   ulRC = PackNewFile( pPackCtrl, pPackCtrl->pCurFile->pszName );
               }
              
             }
             break;

/*------------ Memory files controlled by a memory plugin --------------*/
           case PLUGINMEMORY_DATA_FILE:
             {
               // prepare getMemoryPart method for new file
               pPackCtrl->MemPartData.fFirstCall = TRUE;
               strcpy( pPackCtrl->MemPartData.szFileName, pPackCtrl->pCurFile->pszName );
               UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,  // query current pos
                                  0L,
                                  FILE_CURRENT,
                                  &pPackCtrl->pCurFile->ulFilePos,
                                  TRUE, pPackCtrl->hwndOwner );
                pPackCtrl->pCurFile->ulPackedSize = 0L;
                pPackCtrl->pCurFile->usDiskNum  = pPackCtrl->PackHead.usSequence;
                pPackCtrl->ulInBufUsed = 0;
                pPackCtrl->ulOutBufUsed = 0;
             }
             break;

/*--------------------------------------------------------------------------*/
           default:
             ulRC = PackNewFile( pPackCtrl, pPackCtrl->pCurFile->pszName );
             break;
         } /* endswitch */
         pPackCtrl->fFirstRead = TRUE;
         break;

      case PACK_CONVERT_NON_UNICODE_DA:
         ulRC = AsdIntoNonUnicodeDAWork( pPackCtrl );
         /*************************************************************/
         /* if we are ready for copying, we should set the state for  */
         /* it...                                                     */
         /*************************************************************/
         if ( pPackCtrl->NTMNonUnicodeDA.usAction == NTM_PREP_COPY )
         {
           pPackCtrl->ProcessState = PACK_READFILE;
         }
         else
         {
           pPackCtrl->ProcessState = PACK_CONVERT_NON_UNICODE_DA;
         } /* endif */
         break;


      //======================================================================//
      case PACK_READFILE :                // next task: read data into buffer
         //--- fill input buffer ---
         switch ( pPackCtrl->pCurFile->usFileType )
         {
           //case NTMMEMORY_DATA_FILE: 
           //case NTMMEMORY_INDEX_FILE:
           case DICTIONARY_DATA_FILE:
           case DICTIONARY_INDEX_FILE:
             pPackCtrl->GetDictPartIn.ulBytesToRead =
                    min((ULONG)GETPART_BUFFER_SIZE,
                    (ULONG)(pPackCtrl->ulInBufSize - pPackCtrl->ulInBufUsed) );

             QDAMGetPart( pPackCtrl->hTM,
                          pPackCtrl->pCurFile->pszName,
                          &(pPackCtrl->GetDictPartIn),
                          &(pPackCtrl->GetPartOut),
                          FALSE );


             pPackCtrl->GetDictPartIn.ulFilePos =
                pPackCtrl->GetPartOut.ulNextFilePos;
             memcpy( pPackCtrl->pInBuf + pPackCtrl->ulInBufUsed,
                     pPackCtrl->GetPartOut.aucOutBuffer,
                     (USHORT)pPackCtrl->GetPartOut.ulBytesRead );
             pPackCtrl->ulInBufUsed += pPackCtrl->GetPartOut.ulBytesRead;
             if ( pPackCtrl->GetDictPartIn.ulBytesToRead ==
                  pPackCtrl->GetPartOut.ulBytesRead )
             {
                pPackCtrl->ulBytesRemaining -= pPackCtrl->GetPartOut.ulBytesRead;
             }
             else
             {
                pPackCtrl->ulBytesRemaining = 0L;
             } /* endif */
             pPackCtrl->ulCompleted += pPackCtrl->GetPartOut.ulBytesRead;
             break;

/*--------------------------------------------------------------------------*/

           case DICTIONARY_NON_UNICODE:
             switch ( pPackCtrl->NTMNonUnicodeDA.usAction )
             {
                case NTM_PREP_COPY:
                  {
                    FILEFINDBUF ResultBuf;              // DOS file find struct
                    USHORT  usCount = 1;
                    HDIR    hDirHandle = HDIR_CREATE;          // DosFind routine handle

                    UtlFindFirst( pPackCtrl->NTMNonUnicodeDA.stDorgIda.szCopyAsd,
                                  &hDirHandle, FILE_NORMAL,
                                  &ResultBuf, sizeof( ResultBuf),
                                  &usCount, 0L, FALSE);
                    pPackCtrl->pCurFile->ulFileSize = RESBUFSIZE(ResultBuf);
                    // close search file handle
                    if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );
                  }

                  //get file handle of asd file for local dict
                  ulRC = AsdHandleFromDCB ( pPackCtrl->NTMNonUnicodeDA.stDorgIda.hDict2, &us1, &us2,
                                           &pPackCtrl->GetDictPartIn.pBTree, &pBTree, &LHandle );

                  ulRC = PACK_OK_RC;
                  pPackCtrl->hTM = (HTM)LHandle;
                  //--- start getting at begin of dictionary memory
                  pPackCtrl->GetDictPartIn.ulFilePos = 0;
                  pPackCtrl->ulBytesRemaining = pPackCtrl->pCurFile->ulFileSize;
                  pPackCtrl->pCurFile->ulFilePos  = 0L;
                  UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,  // query current pos
                                 0L,
                                 FILE_CURRENT,
                                 &pPackCtrl->pCurFile->ulFilePos,
                                 TRUE, pPackCtrl->hwndOwner );
                  pPackCtrl->pCurFile->ulPackedSize = 0L;
                  pPackCtrl->pCurFile->usDiskNum  = pPackCtrl->PackHead.usSequence;
                  pPackCtrl->ulInBufUsed = 0;
                  pPackCtrl->ulOutBufUsed = 0;

                  pPackCtrl->NTMNonUnicodeDA.usAction = NTM_COPY;
                  break;


               case NTM_COPY:
                 pPackCtrl->GetDictPartIn.ulBytesToRead =
                        min((ULONG)GETPART_BUFFER_SIZE,
                        (ULONG)(pPackCtrl->ulInBufSize - pPackCtrl->ulInBufUsed) );

                 QDAMGetPart( pPackCtrl->NTMNonUnicodeDA.stDorgIda.hDict2,
                              pPackCtrl->pCurFile->pszName,
                              &(pPackCtrl->GetDictPartIn),
                              &(pPackCtrl->GetPartOut),
                              FALSE );


                 pPackCtrl->GetDictPartIn.ulFilePos =
                    pPackCtrl->GetPartOut.ulNextFilePos;
                 memcpy( pPackCtrl->pInBuf + pPackCtrl->ulInBufUsed,
                         pPackCtrl->GetPartOut.aucOutBuffer,
                         (USHORT)pPackCtrl->GetPartOut.ulBytesRead );
                 pPackCtrl->ulInBufUsed += pPackCtrl->GetPartOut.ulBytesRead;
                 if ( pPackCtrl->GetDictPartIn.ulBytesToRead ==
                      pPackCtrl->GetPartOut.ulBytesRead )
                 {
                    pPackCtrl->ulBytesRemaining -= pPackCtrl->GetPartOut.ulBytesRead;
                 }
                 else
                 {
                    pPackCtrl->ulBytesRemaining = 0L;
                 } /* endif */
                 pPackCtrl->ulCompleted += pPackCtrl->GetPartOut.ulBytesRead;
                 break;
             }

             break;

           case DICTIONARY_NON_UNICODE_INDEX:
             switch ( pPackCtrl->NTMNonUnicodeDA.usAction )
             {
                case NTM_PREP_COPY:
                  {
                    FILEFINDBUF ResultBuf;              // DOS file find struct
                    USHORT  usCount = 1;
                    HDIR    hDirHandle = HDIR_CREATE;          // DosFind routine handle

                    UtlFindFirst( pPackCtrl->NTMNonUnicodeDA.stDorgIda.szCopyAsi,
                                  &hDirHandle, FILE_NORMAL,
                                  &ResultBuf, sizeof( ResultBuf),
                                  &usCount, 0L, FALSE);
                    pPackCtrl->pCurFile->ulFileSize = RESBUFSIZE(ResultBuf);
                    // close search file handle
                    if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );


                    //get file handle of asi file needed for local dict
                    ulRC = AsdHandleFromDCB ( pPackCtrl->NTMNonUnicodeDA.stDorgIda.hDict2, &us1, &us2, &pBTree,
                                              &pPackCtrl->GetDictPartIn.pBTree, &LHandle );
                    pPackCtrl->hTM = (HTM)LHandle;

                    pPackCtrl->GetDictPartIn.ulFilePos = 0;

                    pPackCtrl->ulBytesRemaining = pPackCtrl->pCurFile->ulFileSize;
                    pPackCtrl->pCurFile->ulFilePos  = 0L;
                    UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,  // query current pos
                                       0L,
                                       FILE_CURRENT,
                                       &pPackCtrl->pCurFile->ulFilePos,
                                       TRUE, pPackCtrl->hwndOwner );
                    pPackCtrl->pCurFile->ulPackedSize = 0L;
                    pPackCtrl->pCurFile->usDiskNum  = pPackCtrl->PackHead.usSequence;
                    pPackCtrl->ulInBufUsed = 0;
                    pPackCtrl->ulOutBufUsed = 0;
                    ulRC = PACK_OK_RC;

                    pPackCtrl->NTMNonUnicodeDA.usAction = NTM_COPY;

                  }
                  break;

               case NTM_COPY:
                 pPackCtrl->GetDictPartIn.ulBytesToRead =
                        min((ULONG)GETPART_BUFFER_SIZE,
                        (ULONG)(pPackCtrl->ulInBufSize - pPackCtrl->ulInBufUsed) );

                 QDAMGetPart( pPackCtrl->NTMNonUnicodeDA.stDorgIda.hDict2,
                              pPackCtrl->pCurFile->pszName,
                              &(pPackCtrl->GetDictPartIn),
                              &(pPackCtrl->GetPartOut),
                              FALSE );


                 pPackCtrl->GetDictPartIn.ulFilePos =
                    pPackCtrl->GetPartOut.ulNextFilePos;
                 memcpy( pPackCtrl->pInBuf + pPackCtrl->ulInBufUsed,
                         pPackCtrl->GetPartOut.aucOutBuffer,
                         (USHORT)pPackCtrl->GetPartOut.ulBytesRead );
                 pPackCtrl->ulInBufUsed += pPackCtrl->GetPartOut.ulBytesRead;
                 if ( pPackCtrl->GetDictPartIn.ulBytesToRead ==
                      pPackCtrl->GetPartOut.ulBytesRead )
                 {
                    pPackCtrl->ulBytesRemaining -= pPackCtrl->GetPartOut.ulBytesRead;
                 }
                 else
                 {
                    pPackCtrl->ulBytesRemaining = 0L;
                 } /* endif */
                 pPackCtrl->ulCompleted += pPackCtrl->GetPartOut.ulBytesRead;

                 break;
             }


             break;
/*--------------------------------------------------------------------------*/
           case PLUGINMEMORY_DATA_FILE:
             {
               int iRC = 0;

               pPackCtrl->MemPartData.ulBytesToRead = pPackCtrl->ulInBufSize - pPackCtrl->ulInBufUsed;
               pPackCtrl->MemPartData.pbBuffer = pPackCtrl->pInBuf + pPackCtrl->ulInBufUsed,
               iRC = pPackCtrl->pMemory->getMemoryPart( &pPackCtrl->MemPartData );
               pPackCtrl->ulInBufUsed += pPackCtrl->MemPartData.ulBytesRead;

               // set remaining bytes value depending on completion state
               pPackCtrl->ulBytesRemaining = pPackCtrl->MemPartData.fFileIsComplete ? 0 : 1;

                // special handling for first read of memory property file if "memory rename during export" is active
                if ( (ulRC == PACK_OK_RC) && pPackCtrl->fFirstRead && pPackCtrl->fMemoryRename )
                {
                  Utlstrccpy( pPackCtrl->szTempName, UtlGetFnameFromPath( pPackCtrl->pCurFile->pszName ), DOT );
                  int iMemIndex = FolImpGetRenameMemoryShortNameIndex( pPackCtrl->pMemRename, pPackCtrl->szTempName );
                  PSZ pszExt = strrchr( pPackCtrl->pCurFile->pszName, DOT ); 
                  if ( (iMemIndex >= 0) && (pszExt != NULL) && (stricmp( pszExt, EXT_OF_MEM ) == 0) ) 
                  {
                    PPROP_NTM pProp = (PPROP_NTM)pPackCtrl->pInBuf ;

                    // we can only handle new (NTM) property files ...
                    if ( strcmp( pProp->szNTMMarker, NTM_MARKER) == 0 )
                    {
                      // adjust name in property header
                      strcpy( pProp->stPropHead.szName, pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex] );
                      strcat( pProp->stPropHead.szName, EXT_OF_TMPROP );
                      // adjust long name
                      if ( strcmp( pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex], pPackCtrl->pMemRename->aszNewMemNames[iMemIndex]) != 0 )
                      {
                        strcpy( pProp->szLongName, pPackCtrl->pMemRename->aszNewMemNames[iMemIndex] );
                      }
                      else
                      {
                        pProp->szLongName[0] = EOS;
                      } /* endif */

                      // adjust fully qualified TM name
                      {
                        CHAR szExt[10];
                        PSZ pszExt;
                        PSZ pszNamePos = UtlGetFnameFromPath( pProp->szFullMemName );
                        pszExt = strchr( pszNamePos, DOT );
                        szExt[0] = EOS;
                        if ( pszExt != NULL ) strcpy( szExt, pszExt );
                        strcpy( pszNamePos, pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex] );
                        strcat( pszNamePos, szExt );
                      }

                      // update name in signature structure
                      strcpy( pProp->stTMSignature.szName, pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex] );
                    } /* endif */
                  }
                }
             }
             break;
/*--------------------------------------------------------------------------*/
           default:
             ulBytesToRead =  min(
                (ULONG) (pPackCtrl->ulInBufSize - pPackCtrl->ulInBufUsed ),
                                           pPackCtrl->ulBytesRemaining );
             if ( ulBytesToRead )
             {
                pPackCtrl->usDosRC = UtlReadHwnd( pPackCtrl->hFilePacked,
                                   pPackCtrl->pInBuf + pPackCtrl->ulInBufUsed,
                                   ulBytesToRead,
                                   &ulBytesRead,
                                   TRUE, pPackCtrl->hwndOwner );
                ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_READFAILED_RC;
                pPackCtrl->ulInBufUsed      += ulBytesRead;
                pPackCtrl->ulBytesRemaining -= ulBytesRead;
                pPackCtrl->ulCompleted      += ulBytesRead;

                // special handling for first read if "memory rename during export" is active
                if ( (ulRC == PACK_OK_RC) && pPackCtrl->fFirstRead && pPackCtrl->fMemoryRename )
                {
                    int iMemIndex = 0;

                    switch ( pPackCtrl->pCurFile->usFileType )
                    {
                      case MEMORY_PROP_FILE:
                        // change only if this is file belongs to one of the renamed memories
                        Utlstrccpy( pPackCtrl->szTempName, UtlGetFnameFromPath( pPackCtrl->pCurFile->pszName ), DOT );
                        iMemIndex = FolImpGetRenameMemoryShortNameIndex( pPackCtrl->pMemRename, pPackCtrl->szTempName );

                        if ( iMemIndex >= 0 )
                        {
                          PPROP_NTM pProp = (PPROP_NTM)pPackCtrl->pInBuf ;

                          // we can only handle new (NTM) property files ...
                          if ( strcmp( pProp->szNTMMarker, NTM_MARKER) == 0 )
                          {
                            // adjust name in property header
                            strcpy( pProp->stPropHead.szName, pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex] );
                            strcat( pProp->stPropHead.szName, EXT_OF_TMPROP );
                            // adjust long name
                            if ( strcmp( pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex], pPackCtrl->pMemRename->aszNewMemNames[iMemIndex]) != 0 )
                            {
                              strcpy( pProp->szLongName, pPackCtrl->pMemRename->aszNewMemNames[iMemIndex] );
                            }
                            else
                            {
                              pProp->szLongName[0] = EOS;
                            } /* endif */

                            // adjust fully qualified TM name
                            {
                              CHAR szExt[10];
                              PSZ pszExt;
                              PSZ pszNamePos = UtlGetFnameFromPath( pProp->szFullMemName );
                              pszExt = strchr( pszNamePos, DOT );
                              szExt[0] = EOS;
                              if ( pszExt != NULL ) strcpy( szExt, pszExt );
                              strcpy( pszNamePos, pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex] );
                              strcat( pszNamePos, szExt );
                            }

                            // update name in signature structure
                            strcpy( pProp->stTMSignature.szName, pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex] );
                          } /* endif */
                        } /* endif */
                        break;

                      case SUBFOLDER_PROP_FILE:
                      case FOLDER_PROP_FILE:
                        {
                          // adjust references to renamed memories
                          PPROPFOLDER pProp = (PPROPFOLDER)pPackCtrl->pInBuf ;
                          {
                            int iMemIndex = -1;

                            while ( iMemIndex < MAX_NUM_OF_READONLY_MDB )
                            {
                              PSZ pszCurMem;

                              pszCurMem = (iMemIndex == -1) ? pProp->szLongMemory : pProp->aLongMemTbl[iMemIndex]; 
                              if ( pszCurMem[0] != EOS )
                              {
                                PSZ pszNewName = FolImpGetNewMemoryName( pPackCtrl->pMemRename, pszCurMem );
                                if ( pszNewName != pszCurMem )
                                {
                                  strcpy( pszCurMem, pszNewName );
                                } /* endif */
                              } /* endif */
                              iMemIndex++;
                            } /* endwhile */
                          }
                        }
                        break;

                      case DOCUMENT_PROP_FILE:
                        {
                          // adjust references to renamed memory
                          PPROPDOCUMENT pProp = (PPROPDOCUMENT)pPackCtrl->pInBuf ;
                          PSZ pszNewName = FolImpGetNewMemoryName( pPackCtrl->pMemRename, pProp->szLongMemory );
                          if ( pszNewName != pProp->szLongMemory )
                          {
                            strcpy( pProp->szLongMemory, pszNewName );
                          } /* endif */
                        }
                        break;
                      default:
                        break;
                    } /* endswitch */
                } /* endif */
             } /* endif */
         } /* endswitch */

         //--- set new processing state ---
         pPackCtrl->ProcessState = PACK_COMPRESS;
         pPackCtrl->fFirstRead = FALSE;
         break;
      //======================================================================//
      case PACK_COMPRESS :                // next task: compress data

         //--- 'compress' data ---
         if ( pPackCtrl->pCurFile->usProcessFlags & FILE_COMPRESS )
         {
            pPackCtrl->pCurFile->usProcessFlags &= ~FILE_COMPRESS;
            pPackCtrl->ProcessState = PACK_COMPRESS;
         }
         else
         {
            /**********************************************************/
            /* Correct property files in case of export in TM/2 V1    */
            /* format or to convert property files of shared TMs      */
            /* or shared dicts to local property files                */
            /*                                                        */
            /* Assumption: The complete property file is in the input */
            /* buffer (this will be true for properties up to a size  */
            /* of 32k bytes)                                          */
            /**********************************************************/
            switch ( pPackCtrl->pCurFile->usFileType )
            {
              case MEMORY_PROP_FILE :
                {
                  PPROP_NTM pProp;
                  pProp = (PPROP_NTM)pPackCtrl->pInBuf;
                  if ( pProp->usLocation == TM_SHARED )
                  {
                    PSZ pszExt = strrchr( pProp->szFullMemName, DOT );
                    if ( pszExt )
                    {
                      strcpy( pszExt, EXT_OF_TMDATA );
                    } /* endif */
                    pProp->usLocation = TM_LOCAL;
                  } /* endif */
                }
                break;

              case DICTIONARY_PROP_FILE :
                {
                  PPROPDICTIONARY pProp;
                  pProp = (PPROPDICTIONARY)pPackCtrl->pInBuf;
                  if ( pProp->usLocation == LOC_SHARED )
                  {
                    PSZ pszExt = strrchr( pProp->szDictPath, DOT );
                    if ( pszExt )
                    {
                      strcpy( pszExt, EXT_OF_DIC );
                    } /* endif */

                    pszExt = strrchr( pProp->szIndexPath, DOT );
                    if ( pszExt )
                    {
                      strcpy( pszExt, EXT_OF_DICTINDEX );
                    } /* endif */
                    pProp->usLocation = LOC_LOCAL;
                  } /* endif */
                }
                break;

              case FOLDER_PROP_FILE :
//                if ( pPackCtrl->bVersion == EXP_IN_VERSION1 )
//                {
//                  /**************************************************/
//                  /* Remove columns from view lists which did not   */
//                  /* exist in TM/2 V1.0 format                      */
//                  /**************************************************/
//                  PPROPFOLDER pProp;    // ptr to folder properties
//
//                  pProp = (PPROPFOLDER)pPackCtrl->pInBuf;
//
//                  if ( pProp->sLastUsedViewList[0] != 0 )
//                  {
//                    PSHORT psSource, psTarget;
//                    psSource = psTarget = pProp->sLastUsedViewList;
//                    while ( *psSource != CLBLISTEND )
//                    {
//                      if ( *psSource <= FOL_COPIED_IND )
//                      {
//                        *psTarget++ = *psSource++;
//                      }
//                      else
//                      {
//                        psSource++;
//                      } /* endif */
//                    } /* endwhile */
//                    *psTarget = CLBLISTEND;
//                  } /* endif */
//
//                  if ( pProp->sDetailsViewList[0] != 0 )
//                  {
//                    PSHORT psSource, psTarget;
//                    psSource = psTarget = pProp->sDetailsViewList;
//                    while ( *psSource != CLBLISTEND )
//                    {
//                      if ( *psSource <= FOL_COPIED_IND )
//                      {
//                        *psTarget++ = *psSource++;
//                      }
//                      else
//                      {
//                        psSource++;
//                      } /* endif */
//                    } /* endwhile */
//                    *psTarget = CLBLISTEND;
//                  } /* endif */
//                } /* endif */
                break;
            } /* endswitch */


            /**********************************************************/
            /* Copy data from input buffer to output buffer           */
            /*                                                        */
            /* Note: Compress is not included in this version         */
            /**********************************************************/
            ulBytesToWrite = pPackCtrl->ulOutBufSize - pPackCtrl->ulOutBufUsed;
            ulBytesToWrite = min( ulBytesToWrite, pPackCtrl->ulInBufUsed );

            memcpy( pPackCtrl->pOutBuf + pPackCtrl->ulOutBufUsed,
                    pPackCtrl->pInBuf,
                    ulBytesToWrite );
            pPackCtrl->ulOutBufUsed += ulBytesToWrite;
            if ( ulBytesToWrite != pPackCtrl->ulInBufUsed )
            {
               memmove( pPackCtrl->pInBuf,
                        pPackCtrl->pInBuf + ulBytesToWrite,
                        pPackCtrl->ulInBufUsed - ulBytesToWrite  );
               pPackCtrl->ulInBufUsed  -= ulBytesToWrite;
            }
            else
            {
               pPackCtrl->ulInBufUsed  = 0;
            } /* endif */
            pPackCtrl->ProcessState = PACK_WRITEFILE;
         } /* endif */

         break;
      //======================================================================//
      case PACK_WRITEFILE :                // next task: write data to file

         ulBytesToWrite = pPackCtrl->ulOutBufUsed;

         //--- write compressed data to package ---
         if ( ulRC == PACK_OK_RC )
         {
            pPackCtrl->usDosRC = UtlWriteWoCheckHwnd(
                                    pPackCtrl->hFilePackage,
                                    pPackCtrl->pOutBuf,
                                    ulBytesToWrite,
                                    &ulBytesWritten,
                                    TRUE,
                                    pPackCtrl->hwndOwner );
            ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
         } /* endif */

         //--- handle partially written output ---
         if ( ulRC == PACK_OK_RC )
         {
            if ( ulBytesWritten != pPackCtrl->ulOutBufUsed )
            {
               memmove( pPackCtrl->pOutBuf,
                        pPackCtrl->pOutBuf + ulBytesWritten,
                        pPackCtrl->ulOutBufUsed - ulBytesWritten  );
               pPackCtrl->ulOutBufUsed -= ulBytesWritten;
            }
            else
            {
               pPackCtrl->ulOutBufUsed = 0;
            } /* endif */
         } /* endif */


         //--- set file pointer and remaining bytes value ---
         if ( ulRC == PACK_OK_RC )
         {
            pPackCtrl->pCurFile->ulPackedSize += ulBytesWritten;
         } /* endif */

         //--- set new processing state ---
         if ( ulRC != PACK_OK_RC )
         {
            pPackCtrl->ProcessState = PACK_INERROR;
         }
         else if ( pPackCtrl->ulOutBufUsed )
         {
            pPackCtrl->ProcessState = PACK_DISKFULL;
         }
         else if ( pPackCtrl->ulBytesRemaining == 0L )
         {
            pPackCtrl->ProcessState = PACK_CLOSEFILE;
         }
         else
         {
            pPackCtrl->ProcessState = PACK_READFILE;
         } /* endif */
         break;
      //======================================================================//
      case PACK_DISKFULL :                // next task: get a new diskette
         //--- complete package file if is not the first one ---
         if ( pPackCtrl->PackHead.usSequence != 1)
         {
            //--- rewrite header ---
            pPackCtrl->PackHead.fCompleted = TRUE;
            ulRC = UtlPackWriteSysHead( pPackCtrl, &pPackCtrl->PackHead );

         } /* endif */
         //--- close package file ---
         pPackCtrl->usDosRC = UtlCloseHwnd( pPackCtrl->hFilePackage, TRUE, pPackCtrl->hwndOwner );
         pPackCtrl->hFilePackage = NULLHANDLE;
         ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;

         //-- check if drive has removable media --
         pPackCtrl->pszErrParm[0] = pPackCtrl->szTargetDrive;
         if ( (strchr( "abAB", pPackCtrl->chTargetDrive ) == NULL ) ||
              ISBATCHHWND(pPackCtrl->hwndOwner) )
         {
            UtlErrorHwnd( ERROR_FOLEXP_NOSPACE, MB_CANCEL, 1,
                      pPackCtrl->pszErrParm, EQF_ERROR, pPackCtrl->hwndOwner );
            pPackCtrl->ProcessState = PACK_INERROR;
            ulRC = PACK_NODISKSPACE_RC;
         }
         else
         {
            usMBCode = UtlErrorHwnd( ERROR_FOLEXP_NEWDISK, MB_OKCANCEL, 1,
                                 pPackCtrl->pszErrParm, EQF_WARNING,
                                 pPackCtrl->hwndOwner );
            if ( usMBCode == MBID_OK )
            {
               pPackCtrl->ProcessState = PACK_NEWDISK;
            }
            else
            {
               pPackCtrl->ProcessState = PACK_TERMINATED;
               ulRC = PACK_TERMINATE_RC;
            } /* endif */
         } /* endif */
         break;
      //======================================================================//
      case PACK_NEWDISK :                 // next task: write header on new disk
         pPackCtrl->usDosRC = UtlOpen( pPackCtrl->szFullPackName,
                            &pPackCtrl->hFilePackage,
                            &usOpenAction, 0L,
                            FILE_NORMAL,
                            FILE_OPEN,
                            OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                            0L,
                            FALSE );

         if ( pPackCtrl->usDosRC == NO_ERROR )
         {  // --> there is already a package file on the disk

            /**********************************************************/
            /* Get package header to identify package                 */
            /**********************************************************/
            pPackCtrl->usDosRC = UtlReadL( pPackCtrl->hFilePackage,
                               &pPackCtrl->TempHead,
                               sizeof( pPackCtrl->TempHead ),
                               &ulBytesRead,
                               FALSE );

            /**********************************************************/
            /* Close package file in any case                         */
            /**********************************************************/
            UtlClose( pPackCtrl->hFilePackage, FALSE );
            pPackCtrl->hFilePackage = NULLHANDLE;

            /**********************************************************/
            /* Delete package file if not from current export set     */
            /* else issue warning message                             */
            /**********************************************************/
            if ( (pPackCtrl->usDosRC == NO_ERROR) &&
                 (pPackCtrl->TempHead.ulPackDate ==
                  pPackCtrl->PackHead.ulPackDate) )
            {
              /********************************************************/
              /* Package is from current export set                   */
              /********************************************************/
              pPackCtrl->pszErrParm[0] = pPackCtrl->szTargetDrive;
              usMBCode = UtlErrorHwnd( ERROR_FOLEXP_EXPORTDISK, MB_OKCANCEL, 1,
                                       pPackCtrl->pszErrParm, EQF_ERROR,
                                       pPackCtrl->hwndOwner );
              if ( usMBCode == MBID_OK )
              {
                 pPackCtrl->ProcessState = PACK_NEWDISK;
              }
              else
              {
                 pPackCtrl->ProcessState = PACK_TERMINATED;
                 ulRC = PACK_TERMINATE_RC;
              } /* endif */
              fOK = FALSE;             // stop processing or retry new disk
            }
            else
            {
              UtlDelete( pPackCtrl->szFullPackName, 0L, FALSE );
              fOK = TRUE;              // there is no package file anymore
            } /* endif */
         }
         else
         {
           fOK = TRUE;                 // there is no package file on new disk
         } /* endif */

         if ( fOK )
         {
            UtlMakeEQFPath( pPackCtrl->szFullPackName,
                            pPackCtrl->chTargetDrive,
                            EXPORT_PATH, NULL );
            if ( pPackCtrl->fFolder )
            {
               strcat( pPackCtrl->szFullPackName, "\\" );
               strcat( pPackCtrl->szFullPackName, pPackCtrl->szFolder );
            } /* endif */
            pPackCtrl->usDosRC = UtlMkMultDir( pPackCtrl->szFullPackName, FALSE );
            if ( (pPackCtrl->usDosRC == NO_ERROR) ||
                 (pPackCtrl->usDosRC == ERROR_ACCESS_DENIED) )
            {
              ulRC = PACK_OK_RC;
            }
            else
            {
              pPackCtrl->usDosRC = UtlMkMultDirHwnd( pPackCtrl->szFullPackName,
                                                   TRUE, pPackCtrl->hwndOwner );
              ulRC = ( pPackCtrl->usDosRC == NO_ERROR ) ? PACK_OK_RC
                                                        : PACK_OPENFAILED_RC;
            } /* endif */
            if ( ulRC == PACK_OK_RC )
            {
               strcat( pPackCtrl->szFullPackName, "\\" );
               strcat( pPackCtrl->szFullPackName, pPackCtrl->szPackName );
               pPackCtrl->usDosRC = UtlOpenHwnd( pPackCtrl->szFullPackName,
                                  &pPackCtrl->hFilePackage,
                                  &usOpenAction, 0L,
                                  FILE_NORMAL,
                                  FILE_OPEN | FILE_CREATE,
                                  OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                                  0L,
                                  TRUE, pPackCtrl->hwndOwner );
               ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_OPENFAILED_RC;
            } /* endif */

            if ( ulRC == PACK_OK_RC )
            {
               pPackCtrl->PackHead.usSequence++;
               ulRC = UtlPackWriteSysHead( pPackCtrl, &pPackCtrl->PackHead );
            } /* endif */

            pPackCtrl->ProcessState = PACK_WRITEFILE;
         } /* endif */
         break;
      //======================================================================//
      case PACK_CLOSEFILE :               // next task: close current file
         switch ( pPackCtrl->pCurFile->usFileType )
         {
           //case NTMMEMORY_DATA_FILE:
           //  break;
           //case NTMMEMORY_INDEX_FILE:
           //  TmClose( pPackCtrl->hTM, pPackCtrl->pCurFile->pszName, FALSE, 0 );
           //  pPackCtrl->hTM = NULLHANDLE;
           //  break;

           case DICTIONARY_INDEX_FILE:
             //only unlock and close after index file has been read
//             AsdLockDict( pPackCtrl->hDict, FALSE );
             AsdClose( pPackCtrl->hDictUser, pPackCtrl->hDict );
             pPackCtrl->hDict = NULL;
             AsdEnd( pPackCtrl->hDictUser );
             pPackCtrl->hDictUser = NULL;
             break;
           case DICTIONARY_DATA_FILE:
             break;

           case DICTIONARY_NON_UNICODE_INDEX:
//           AsdClose( pPackCtrl->hDictUser, pPackCtrl->hDict );
//           pPackCtrl->hDict = NULL;
//           AsdEnd( pPackCtrl->hDictUser );
//           pPackCtrl->hDictUser = NULL;
             AsdIntoNonUnicodeDAClose ( pPackCtrl );
             break;

           case DICTIONARY_NON_UNICODE:
             break;

           case DOCUMENT_SEGSRC_FILE:
           case DOCUMENT_SEGTGT_FILE:
             pPackCtrl->usDosRC = UtlClose( pPackCtrl->hFilePacked, FALSE );
             pPackCtrl->hFilePacked = NULLHANDLE;
             if (pPackCtrl->bVersion == EXP_IN_NON_UNICODE)
             {
               UtlDelete(pPackCtrl->chTempFile, 0L, FALSE);
               pPackCtrl->chTempFile[0] = EOS;
             }
             break;
           case PLUGINMEMORY_DATA_FILE:
             // close memory if next file is no PLUGINMEMORY_DATA_FILE
             if ( (pPackCtrl->pCurFile < (pPackCtrl->FileList.pEntries + pPackCtrl->FileList.ulListUsed + 1)) &&
                  (pPackCtrl->pCurFile[1].usFileType == PLUGINMEMORY_DATA_FILE) )
             {
                // next file is another PLUGINMEMORY_DATA_FILE so don't close the memory
             }
             else
             {
               // close the memory
               MemoryFactory *pFactory = MemoryFactory::getInstance();
               pFactory->closeMemory( pPackCtrl->pMemory );
               pPackCtrl->pMemory = NULL;
             } /* endif */
             break;

           default :
             pPackCtrl->usDosRC = UtlClose( pPackCtrl->hFilePacked, FALSE );
             pPackCtrl->hFilePacked = NULLHANDLE;
             break;
         } /* endswitch */
         pPackCtrl->pCurFile->usProcessFlags |= FILE_IS_PROCESSED;
         pPackCtrl->pCurFile++;
         if ( pPackCtrl->pCurFile >= (pPackCtrl->FileList.pEntries +
                                      pPackCtrl->FileList.ulListUsed) )
         {
            pPackCtrl->ProcessState = PACK_COMPLETEPACKAGE;
         }
         else
         {
            pPackCtrl->ProcessState = PACK_NEWFILE;
         } /* endif */
         break;
      //======================================================================//
      case PACK_COMPLETEPACKAGE :         // next task: complete the package file
         //--- set flags in package header ---
         pPackCtrl->PackHead.fCompleted = TRUE;
         pPackCtrl->PackHead.fLastFileOfPackage = TRUE;

         // when "rename memory during export" is active, we have to rename
         // the memory files in the file list
         if ( pPackCtrl->fMemoryRename )
         {
           CHAR szMemShortName[MAX_FILESPEC];
           CHAR szExtension[10];

           PFILELISTENTRY pFileEntry = pPackCtrl->FileList.pEntries;
           ULONG ulNoOfEntries = pPackCtrl->FileList.ulListUsed;
           while ( ulNoOfEntries )
           {
               // check if file has to be renamed
               if ( pFileEntry->usFileType == MEMORY_INFO_FILE )
               {
                 // get memory name of entry
                 pszName = strchr( pFileEntry->pszName, '-' );
                 if ( pszName != NULL )
                 {
                   Utlstrccpy( szMemShortName, pszName + 1, DOT );
                   int iMemIndex = FolImpGetRenameMemoryShortNameIndex( pPackCtrl->pMemRename, szMemShortName );
                   if ( iMemIndex >= 0 )
                   {
                     strcpy( pszName + 1, pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex] );
                     strcat( pszName, EXT_OF_MEMORYINFO );
                   } /* endif */
                 }
               }
               else if ( (pFileEntry->usFileType == NTMMEMORY_DATA_FILE) ||
                    (pFileEntry->usFileType == NTMMEMORY_INDEX_FILE) ||
                    (pFileEntry->usFileType == PLUGINMEMORY_DATA_FILE) ||
                    (pFileEntry->usFileType == MEMORY_PROP_FILE) )
               {
                 // get file name of entry
                 pszName = UtlGetFnameFromPath( pFileEntry->pszName );
                 strcpy( szMemShortName, pszName );

                 // cut off extension
                 {
                   PSZ pszExtension = strrchr( szMemShortName, DOT );
                   szExtension[0] = EOS;
                   if ( pszExtension )
                   {
                      strcpy( szExtension, pszExtension );
                     *pszExtension = EOS;
                   } /* endif */
                 }

                 // rename file if short name matches
                 int iMemIndex = FolImpGetRenameMemoryShortNameIndex( pPackCtrl->pMemRename, szMemShortName );
                 if ( iMemIndex >= 0 )
                 {
                   strcpy( pszName, pPackCtrl->pMemRename->aszNewMemShortNames[iMemIndex] );
                   strcat( pszName, szExtension );
                 } /* endif */
               } /* endif */
               pFileEntry++;
               ulNoOfEntries--;
           } /* endwhile */
         } /* endif */

         //--- if current disk is not the first one ---
         if ( pPackCtrl->PackHead.usSequence != 1 )
         {
            //--- rewrite package header ---
            if ( ulRC == PACK_OK_RC )
            {
               ulRC = UtlPackWriteSysHead( pPackCtrl, &pPackCtrl->PackHead );
            } /* endif */
         } /* endif */

         //--- close package file ---
         if ( ulRC == PACK_OK_RC )
         {
            pPackCtrl->usDosRC = UtlCloseHwnd( pPackCtrl->hFilePackage, TRUE,
                                               pPackCtrl->hwndOwner );
            pPackCtrl->hFilePackage = NULLHANDLE;
            ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
         } /* endif */

         //--- if current disk is the first one ---
         if ( ulRC == PACK_OK_RC )
         {
            if ( pPackCtrl->PackHead.usSequence == 1 )
            {
               pPackCtrl->ProcessState = PACK_WRITEFILELISTTOFIRSTDISK;
            }
            else
            {
               pPackCtrl->pszErrParm[0] = pPackCtrl->szTargetDrive;
               usMBCode = UtlErrorHwnd( MESSAGE_FOLEXP_FIRSTDISK, MB_OKCANCEL, 1,
                                    pPackCtrl->pszErrParm, EQF_WARNING,
                                    pPackCtrl->hwndOwner );
               if ( usMBCode == MBID_OK )
               {
                  pPackCtrl->ProcessState = PACK_WRITEFILELISTTOFIRSTDISK;
               }
               else
               {
                  pPackCtrl->ProcessState = PACK_TERMINATED;
                  ulRC = PACK_TERMINATE_RC;
               } /* endif */
            } /* endif */
         } /* endif */
         break;
      //======================================================================//
      case PACK_WRITEFILELISTTOFIRSTDISK:   // rewrite file list on first disk
         //--- set flags in package header ---
         pPackCtrl->PackHead.fCompleted = TRUE;
         pPackCtrl->PackHead.usSequence = 1;
         pPackCtrl->hFilePackage = NULLHANDLE;

         pPackCtrl->usDosRC = UtlOpenHwnd( pPackCtrl->szFullPackName,
                            &pPackCtrl->hFilePackage,
                            &usOpenAction, 0L,
                            FILE_NORMAL,
                            FILE_OPEN,
                            OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                            0L,
                            FALSE, pPackCtrl->hwndOwner );

         if ( !pPackCtrl->usDosRC )               // if ok so far ...
         {
            //--- read package header ---
            UtlPackReadSysHead( pPackCtrl, &pPackCtrl->TempHead );
         } /* endif */

         if ( !pPackCtrl->usDosRC )               // if ok so far ...
         {
            //--- check header flags ---
            if ( (pPackCtrl->TempHead.usSequence != 1) ||
                 (pPackCtrl->TempHead.ulPackDate != pPackCtrl->PackHead.ulPackDate) )
            {
               pPackCtrl->usDosRC = 1;            // dummy value, has to be != 0
            } /* endif */
         } /* endif */

         if ( pPackCtrl->usDosRC )                // if not ok ...
         {
            usMBCode = UtlErrorHwnd( ERROR_FOLEXP_NOTFIRSTDISK, MB_OKCANCEL, 1,
                                 pPackCtrl->pszErrParm, EQF_ERROR,
                                 pPackCtrl->hwndOwner );
            if ( usMBCode == MBID_OK )
            {
               pPackCtrl->ProcessState = PACK_WRITEFILELISTTOFIRSTDISK;
            }
            else
            {
               pPackCtrl->ProcessState = PACK_TERMINATED;
               ulRC = PACK_TERMINATE_RC;
            } /* endif */

            /**********************************************************/
            /* Close any open package file                            */
            /**********************************************************/
            if ( pPackCtrl->hFilePackage != NULLHANDLE )
            {
              UtlClose( pPackCtrl->hFilePackage, FALSE );
              pPackCtrl->hFilePackage = NULLHANDLE;
            } /* endif */
         }
         else
         {
            //--- rewrite header ---
            ulRC = UtlPackWriteSysHead( pPackCtrl, &pPackCtrl->PackHead );

            //--- rewrite file table ---
            if ( ulRC == PACK_OK_RC )
            {
               ulRC = UtlPackWriteFileList( pPackCtrl );
            } /* endif */

            //--- close package file ---
            if ( ulRC == PACK_OK_RC )
            {
               pPackCtrl->usDosRC = UtlCloseHwnd( pPackCtrl->hFilePackage, TRUE,
                                                  pPackCtrl->hwndOwner );
               pPackCtrl->hFilePackage = NULLHANDLE;
               ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
            } /* endif */

            //--- set new processing state ---
            pPackCtrl->ProcessState = PACK_DONE;
         } /* endif */

         break;
      //======================================================================//
      case PACK_DONE :                    // package has been completed
         ulRC = PACK_COMPLETE_RC;
         break;
      //======================================================================//
      case PACK_INERROR :                 // packaging ended due to errors
         ulRC = PACK_INERROR_RC;
         break;
      //======================================================================//
      case PACK_TERMINATED :              // packaging was terminated by user
         ulRC = PACK_TERMINATE_RC;
         break;
      //======================================================================//
      default :                           // wrong (=unknown) process state
         ulRC = PACK_INVPROCESSSTATE_RC;
         break;
      //======================================================================//
   } /* endswitch */

   //--- fill callers completion ration if requested ---
   if ( pPackCtrl && pusCompleted )
   {
     INT64 iComplete = (INT64)pPackCtrl->ulCompleted;
     INT64 iTotal = (INT64)pPackCtrl->ulTotalSize;
     INT64 iRatio = iComplete * (INT64)100 / iTotal;
      *pusCompleted = (USHORT)iRatio;
   } /* endif */

   //--- close any open file in case of errors
   if ( ulRC != PACK_OK_RC )
   {
      if ( pPackCtrl->pMemory != NULL )
      {
        // close the memory
        MemoryFactory *pFactory = MemoryFactory::getInstance();
        pFactory->closeMemory( pPackCtrl->pMemory );
        pPackCtrl->pMemory = NULL;
      } /* endif */

      if ( pPackCtrl->hFilePackage )
      {
         UtlClose( pPackCtrl->hFilePackage, FALSE );
         pPackCtrl->hFilePackage = NULLHANDLE;
      } /* endif */
      if ( pPackCtrl->hFilePacked )
      {
         UtlClose( pPackCtrl->hFilePacked, FALSE );
         pPackCtrl->hFilePacked = NULLHANDLE;
      } /* endif */
      if ( pPackCtrl->pCurFile )
      {
        switch ( pPackCtrl->pCurFile->usFileType )
        {
          //case NTMMEMORY_DATA_FILE:
          //case NTMMEMORY_INDEX_FILE:
          //  if (pPackCtrl->hTM)
          //  {
          //    TmClose( pPackCtrl->hTM, pPackCtrl->pCurFile->pszName, FALSE, 0 );
          //    pPackCtrl->hTM = NULLHANDLE;
          //  } /* endif */
          //  break;

          case DICTIONARY_DATA_FILE:
          case DICTIONARY_INDEX_FILE:
            if ( pPackCtrl->hDict)
            {
              AsdClose(pPackCtrl->hDictUser, pPackCtrl->hDict );
              pPackCtrl->hDict = NULL;
            } /* endif */
            if (pPackCtrl->hDictUser)
            {
              AsdEnd( pPackCtrl->hDictUser );
              pPackCtrl->hDictUser = NULL;
            } /* endif */
            break;


          case DICTIONARY_NON_UNICODE:
            AsdIntoNonUnicodeDAClose ( pPackCtrl );
            break;

          case DICTIONARY_NON_UNICODE_INDEX:
            AsdIntoNonUnicodeDAClose ( pPackCtrl );
//              if ( pPackCtrl->hDict)
//            {
//              AsdClose(pPackCtrl->hDictUser, pPackCtrl->hDict );
//              pPackCtrl->hDict = NULL;
//            } /* endif */
//            if (pPackCtrl->hDictUser)
//            {
//              AsdEnd( pPackCtrl->hDictUser );
 //             pPackCtrl->hDictUser = NULL;
 //           } /* endif */
            break;
        } /* endswitch */
      } /* endif */
   } /* endif */
   return( ulRC );                     // return return code to caller
} /* end of UtlPackWritePackage */

static ULONG UtlPackWriteSysHead( PVOID pVoidPack, PPACKHEADER2 pHead )
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code
   ULONG        ulNewPos;              // new position of file location pointer
   ULONG        ulBytesWritten;         // # of bytes written to file

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else
   {
      //--- position to system package header ---
      if ( ulRC == PACK_OK_RC )
      {
         pPackCtrl->usDosRC = UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,
                                  0L,
                                  FILE_BEGIN,
                                  &ulNewPos,
                                  TRUE, pPackCtrl->hwndOwner );
         ulRC = (ulRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
      } /* endif */

      //--- write system header ---
      if ( ulRC == PACK_OK_RC )
      {
        memcpy( pHead, PACKHEAD3ID, 4 );
        pPackCtrl->usDosRC = UtlWriteWoCheckHwnd( pPackCtrl->hFilePackage,
                                                  pHead,
                                                  sizeof(PACKHEADER2),
                                                  &ulBytesWritten,
                                                  TRUE,
                                                  pPackCtrl->hwndOwner );
         ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
      } /* endif */
   } /* endif */

   return( ulRC );
} /* end of UtlPackWriteSysHead */


ULONG UtlPackReadPackage( PVOID pVoidPack )
{
   PPACKCONTROL pPackCtrl;
   ULONG       ulRC = PACK_OK_RC;      // code returned to caller
   USHORT      usMBCode;               // code returned by Messagebox calls
   USHORT      usOpenAction;           // action performed by DosOpen
   ULONG       ulBytesToWrite = 0;         // # of bytes to write to file
   ULONG       ulBytesWritten = 0;         // # of bytes written to file
   ULONG       ulBytesToRead;          // # of bytes to readfrom file
   ULONG       ulBytesRead;            // # of bytes read from file
   ULONG       ulNewPos;               // new position of file location pointer
   PSZ         pszTemp;                // temporary pointer
   FILESTATUS  FileStatus;             // status information of packed file
   CHAR        szDiskNum[20];          // diskette number
   BOOL        fOK;                    // internal OK flag

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   switch ( pPackCtrl->ProcessState )
   {
      //======================================================================//
      case PACK_OPEN :                 // package is open, nothing processed yet
         //--- allocate required buffers ---
         fOK = UtlAllocHwnd( (PVOID *)&pPackCtrl->pInBuf, 0L, 0x7FFFL, ERROR_STORAGE,
                             pPackCtrl->hwndOwner );
         if ( fOK )
         {
            pPackCtrl->ulInBufSize = 0x7FFF;
            fOK = UtlAllocHwnd( (PVOID *)&pPackCtrl->pOutBuf, 0L, 0x7FFFL, ERROR_STORAGE,
                                pPackCtrl->hwndOwner );
         } /* endif */
         if ( fOK )
         {
            pPackCtrl->ulOutBufSize = 0x7FFF;
         } /* endif */
         if ( !fOK )
         {
            ulRC = PACK_NOMEMORY_RC;
         } /* endif */

         // read file list if not done yet
         if ( pPackCtrl->FileList.pEntries == NULL )
         {
            ulRC = UtlPackReadFileList( pPackCtrl );
         } /* endif */

         //--- position to first file in package ---
         if ( ulRC == PACK_OK_RC )
         {
            pPackCtrl->pCurFile = pPackCtrl->FileList.pEntries;
         } /* endif */

         //--- build import path if necessary
         if ( ulRC == PACK_OK_RC )
         {
            ulRC = UtlPackQueryImportPath( pPackCtrl, NULL );
         } /* endif */

         //--- set new processing state ---
         pPackCtrl->ProcessState = (USHORT)((ulRC == PACK_OK_RC) ? PACK_NEWFILE :
                                                          PACK_INERROR);
         break;
      //======================================================================//
      case PACK_NEWFILE :                 // next task is: begin a new file
         //--- build new filename ---
         strcpy( pPackCtrl->szFileName, pPackCtrl->szImportPath );

         //--- skip x:\EQF part of original file name ---
         pszTemp = strchr( pPackCtrl->pCurFile->pszName, BACKSLASH );
         if ( pszTemp )
         {
            pszTemp = strchr( pszTemp + 1, BACKSLASH );
         } /* endif */
         if ( !pszTemp )
         {
            pszTemp = pPackCtrl->pCurFile->pszName;
         } /* endif */
         strcat( pPackCtrl->szFileName, pszTemp );

         /*************************************************************/
         /* Special file name handling is located here                */
         /*************************************************************/
         {
           switch ( pPackCtrl->pCurFile->usFileType )
           {
             case HISTLOG_DATA_FILE :
               /*******************************************************/
               /* Always use predefined history log file name         */
               /*******************************************************/
               {
                 PSZ pszName;          // pointer to file name

                 pszName = strrchr( pPackCtrl->szFileName, BACKSLASH );
                 if ( pszName != NULL )
                 {
                   strcpy( pszName + 1, HISTLOGFILE );
                 } /* endif */
               }
               break;
             default:
               // nothing to do here
               break;
           } /* endswitch */
         }

         pPackCtrl->ulBytesRemaining = pPackCtrl->pCurFile->ulPackedSize;

         // skip files which are invalid
         if ( pPackCtrl->pCurFile->pszName[0] == EOS )
         {
           pPackCtrl->pCurFile->usProcessFlags |= FILE_SKIP;
         } /* endif */
         if ( !(pPackCtrl->pCurFile->usProcessFlags & FILE_SKIP ))
         {
            //--- create required directories ---
            // (cut off filename from path, make directories, append filename)
            pszTemp = UtlGetFnameFromPath( pPackCtrl->szFileName ) - 1;
            *pszTemp = EOS;
            pPackCtrl->usDosRC = UtlMkMultDirHwnd( pPackCtrl->szFileName, TRUE,
                                                   pPackCtrl->hwndOwner );
            ulRC = ((pPackCtrl->usDosRC == 0)|| (pPackCtrl->usDosRC == ERROR_ACCESS_DENIED) ) ?
                    PACK_OK_RC : PACK_OPENFAILED_RC;

            *pszTemp = BACKSLASH;

            //--- open the file ---
            if ( ulRC == PACK_OK_RC )
            {
              pPackCtrl->usDosRC = UtlOpenHwnd( pPackCtrl->szFileName,
                                 &pPackCtrl->hFilePacked,
                                 &usOpenAction,
                                 0L,
                                 FILE_NORMAL,
                                 FILE_OPEN | FILE_CREATE,
                                 OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                                 0L,
                                 TRUE, pPackCtrl->hwndOwner );
              ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_OPENFAILED_RC;
            } /* endif */
         } /* endif */

         //--- position to correct position within package file ---
         if ( ulRC == PACK_OK_RC )
         {
            UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,
                           pPackCtrl->pCurFile->ulFilePos,
                           FILE_BEGIN,
                           &ulNewPos,
                           TRUE, pPackCtrl->hwndOwner );
            ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_READFAILED_RC;
         } /* endif */

         //--- set new processing state ---
         pPackCtrl->ProcessState = (USHORT)((ulRC == PACK_OK_RC) ? PACK_READFILE :
                                                          PACK_INERROR);
         break;
      //======================================================================//
      case PACK_READFILE :               // next task: write file to package
         //--- fill input buffer ---
         ulBytesToRead =  min( (ULONG) pPackCtrl->ulInBufSize,
                                       pPackCtrl->ulBytesRemaining );
         pPackCtrl->usDosRC = UtlReadHwnd( pPackCtrl->hFilePackage,
                            pPackCtrl->pInBuf,
                            ulBytesToRead,
                            &ulBytesRead,
                            TRUE, pPackCtrl->hwndOwner );
         ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_READFAILED_RC;

         //--- 'decompress' data ---
         if ( ulRC == PACK_OK_RC )
         {
            memcpy( pPackCtrl->pOutBuf, pPackCtrl->pInBuf, ulBytesRead );
            ulBytesToWrite = ulBytesRead;
         } /* endif */


         if ( !(pPackCtrl->pCurFile->usProcessFlags & FILE_SKIP ))
         {
            //--- write uncompressed data to file ---
            if ( ulRC == PACK_OK_RC )
            {
               pPackCtrl->usDosRC = UtlWriteWoCheckHwnd(
                                       pPackCtrl->hFilePacked,
                                       pPackCtrl->pOutBuf,
                                       ulBytesToWrite,
                                       &ulBytesWritten,
                                       TRUE,
                                       pPackCtrl->hwndOwner );
               ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
            } /* endif */
         } /* endif */

         //--- set file pointer and remaining bytes value ---
         if ( ulRC == PACK_OK_RC )
         {
            pPackCtrl->ulBytesRemaining -= ulBytesRead;
         } /* endif */

         //--- set new processing state ---
         if ( ulBytesToRead != ulBytesRead )
         {
            pPackCtrl->ProcessState = PACK_DISKFULL;
         }
         else if ( pPackCtrl->ulBytesRemaining == 0L )
         {
            pPackCtrl->ProcessState = PACK_CLOSEFILE;
         }
         else if ( ulRC == PACK_OK_RC )
         {
            pPackCtrl->ProcessState = PACK_READFILE;
         }
         else
         {
            pPackCtrl->ProcessState = PACK_INERROR;
         } /* endif */
         break;
      //======================================================================//
      case PACK_DISKFULL :                // next task: get a new diskette
         //--- close package file ---
         if ( ulRC == PACK_OK_RC )
         {
            pPackCtrl->usDosRC = UtlCloseHwnd( pPackCtrl->hFilePackage, TRUE,
                                               pPackCtrl->hwndOwner );
            pPackCtrl->hFilePackage = NULLHANDLE;
            ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
         } /* endif */

         //--- request next disk of export set ---
         if ( ulRC == PACK_OK_RC )
         {
            CHAR szSourceDrive[MAX_DRIVE];

            pPackCtrl->PackHead.usSequence++;
            sprintf( szDiskNum, "%d", pPackCtrl->PackHead.usSequence );
            szSourceDrive[0] = pPackCtrl->szFullPackName[0];
            szSourceDrive[1] = COLON;
            szSourceDrive[2] = EOS;
            pPackCtrl->pszErrParm[0] = szSourceDrive;
            pPackCtrl->pszErrParm[1] = szDiskNum;
            usMBCode = UtlErrorHwnd( MESSAGE_FOLIMP_NEWDISK, MB_OKCANCEL, 2,
                                 pPackCtrl->pszErrParm, EQF_WARNING,
                                 pPackCtrl->hwndOwner );
            if ( usMBCode == MBID_OK )
            {
               pPackCtrl->ProcessState = PACK_NEWDISK;
            }
            else
            {
               pPackCtrl->ProcessState = PACK_TERMINATED;
               ulRC = PACK_TERMINATE_RC;
            } /* endif */
         } /* endif */

         break;
      //======================================================================//
      case PACK_NEWDISK :                 // next task: check new disk
         pPackCtrl->hFilePackage = NULLHANDLE;
         pPackCtrl->usDosRC = UtlOpenHwnd( pPackCtrl->szFullPackName,
                            &pPackCtrl->hFilePackage,
                            &usOpenAction, 0L,
                            FILE_NORMAL,
                            FILE_OPEN,
                            OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                            0L,
                            FALSE, pPackCtrl->hwndOwner );

         if ( !pPackCtrl->usDosRC )               // if ok so far ...
         {
            //--- read package header ---
            UtlPackReadSysHead( pPackCtrl, &pPackCtrl->TempHead );
         } /* endif */

         if ( !pPackCtrl->usDosRC )               // if ok so far ...
         {
            //--- check header flags ---
            if ( (pPackCtrl->TempHead.usSequence != pPackCtrl->PackHead.usSequence) ||
                 !pPackCtrl->TempHead.fCompleted     ||
                 (pPackCtrl->TempHead.ulPackDate != pPackCtrl->PackHead.ulPackDate) )
            {
               pPackCtrl->usDosRC = 1;            // dummy value, has to be != 0
            } /* endif */
         } /* endif */

         if ( pPackCtrl->usDosRC )                // if not ok ...
         {
            CHAR szSourceDrive[MAX_DRIVE];

            sprintf( szDiskNum, "%d", pPackCtrl->PackHead.usSequence );
            szSourceDrive[0] = pPackCtrl->szFullPackName[0];
            szSourceDrive[1] = COLON;
            szSourceDrive[2] = EOS;
            pPackCtrl->pszErrParm[0] = szSourceDrive;
            pPackCtrl->pszErrParm[1] = szDiskNum;
            usMBCode = UtlErrorHwnd( ERROR_FOLIMP_WRONGDISK, MB_OKCANCEL, 2,
                                 pPackCtrl->pszErrParm, EQF_ERROR,
                                 pPackCtrl->hwndOwner);
            if ( usMBCode == MBID_OK )
            {
               pPackCtrl->ProcessState = PACK_NEWDISK;
            }
            else
            {
               pPackCtrl->ProcessState = PACK_TERMINATED;
               ulRC = PACK_TERMINATE_RC;
            } /* endif */
            if ( pPackCtrl->hFilePackage )
            {
               UtlClose( pPackCtrl->hFilePackage, FALSE );
            } /* endif */
         }
         else
         {
            pPackCtrl->ProcessState = PACK_READFILE;
         } /* endif */
         break;
      //======================================================================//
      case PACK_CLOSEFILE :               // next task: close current file
         if ( !(pPackCtrl->pCurFile->usProcessFlags & FILE_SKIP ))
         {
            //--- change file date and time to values from file list ---
            memset( &FileStatus, 0, sizeof(FILESTATUS) );
            FileStatus.fdateLastWrite = pPackCtrl->pCurFile->stFileDate;
            FileStatus.ftimeLastWrite = pPackCtrl->pCurFile->stFileTime;
            {
              FILETIME stFileTime;
              DosDateTimeToFileTime( *((WORD *)&pPackCtrl->pCurFile->stFileDate),
                                     *((WORD *)&pPackCtrl->pCurFile->stFileTime),
                                     &stFileTime );
              SetFileTime( pPackCtrl->hFilePacked, (LPFILETIME)NULL,
                           (LPFILETIME)NULL, &stFileTime );
            }

            //-- close the file ---
            pPackCtrl->usDosRC = UtlClose( pPackCtrl->hFilePacked, FALSE );
         } /* endif */

         pPackCtrl->hFilePacked = NULLHANDLE;
         pPackCtrl->pCurFile->usProcessFlags |= FILE_IS_PROCESSED;
         pPackCtrl->pCurFile++;
         if ( pPackCtrl->pCurFile >= (pPackCtrl->FileList.pEntries +
                                      pPackCtrl->FileList.ulListUsed) )

         {
            pPackCtrl->ProcessState = PACK_COMPLETEPACKAGE;
         }
         else
         {
            pPackCtrl->ProcessState = PACK_NEWFILE;
         } /* endif */
         break;
      //======================================================================//
      case PACK_COMPLETEPACKAGE :         // next task: complete the package file
         //--- close package file ---
         if ( ulRC == PACK_OK_RC )
         {
            pPackCtrl->usDosRC = UtlCloseHwnd( pPackCtrl->hFilePackage, TRUE,
                                               pPackCtrl->hwndOwner );
            pPackCtrl->hFilePackage = NULLHANDLE;
            ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
         } /* endif */

         //--- set new processing state ---
         pPackCtrl->ProcessState = (USHORT)((ulRC == PACK_OK_RC) ? PACK_DONE :
                                                          PACK_INERROR);

         break;
      //======================================================================//
      case PACK_DONE :                    // package has been completed
         ulRC = PACK_COMPLETE_RC;
         break;
      //======================================================================//
      case PACK_INERROR :                 // packaging ended due to errors
         ulRC = PACK_INERROR_RC;
         break;
      //======================================================================//
      case PACK_TERMINATED :              // packaging was terminated by user
         ulRC = PACK_TERMINATE_RC;
         break;
      //======================================================================//
      default :                           // wrong (=unknown) process state
         ulRC = PACK_INVPROCESSSTATE_RC;
         break;
      //======================================================================//
   } /* endswitch */

   //--- close any open file in case of errors
   if ( ulRC != PACK_OK_RC )
   {
      if ( ulRC != PACK_TERMINATE_RC )
      {
         pPackCtrl->ProcessState = PACK_INERROR;
      }
      else
      {
         pPackCtrl->ProcessState = PACK_TERMINATED;
      } /* endif */
      if ( pPackCtrl->hFilePackage )
      {
         UtlClose( pPackCtrl->hFilePackage, FALSE );
         pPackCtrl->hFilePackage = NULLHANDLE;
      } /* endif */
      if ( pPackCtrl->hFilePacked )
      {
         UtlClose( pPackCtrl->hFilePacked, FALSE );
         pPackCtrl->hFilePacked = NULLHANDLE;
      } /* endif */
   } /* endif */

   return( ulRC );                     // return return code to caller
} /* end of UtlPackReadPackage */

// @bt
ULONG UtlPackListPackages
(
   HWND hwndLB,
   CHAR chDrive,
   PSZ pszFolder,
   PSZ pszFolderFullPath,
   PSZ pszSearchName,
   USHORT usFlags
)
{
   CHAR   szPackName[MAX_LONGPATH];    // buffer for full package name
   CHAR   szErrParm[MAX_PATH144];      // buffer for error parameters
   PSZ    pszErrParm = szErrParm;      // ptr to error parameters
   PSZ    pszFileName = NULL;          // start of file name in szPackName
   PSZ    pszExtension;                // start of file name extension
   PACKHEADER2 PackHead;               // header of current package file
   static LONGFILEFIND LongResultBuf;      // file find struct
   static FILEFINDBUF ResultBuf;         // DOS file find struct
   USHORT  usCount = 1;                // # of files requested/found
   HDIR    hDirHandle = HDIR_CREATE;   // DosFind routine handle
   ULONG   ulRC = NO_ERROR;            // return code of Dos... alias Utl...
   USHORT  usMBCode;                   // return code of UtlError
   BOOL    fPathValid = TRUE;          // indicates if folder path is correct
   HFILE   hFilePackage;               // file handle of package file
   USHORT  usOpenAction;               // action performed by Utl/DosOpen
   ULONG   ulBytesRead;                // # of bytes read by Utl/DosRead
#ifdef DEBUG_LISTPACKAGES
      CHAR szDebugLine[80];
#endif

   // @bt

#ifdef DEBUG_LISTPACKAGES
   MessageBox ( NULL, "Start of UtlListPackages", "Debug", MB_OK );
#endif

   // changed bt001027
   // single drive letter has got priority
   //if (pszFolderFullPath && strlen(pszFolderFullPath) > 0)
   if (pszFolderFullPath && strlen(pszFolderFullPath) > 0 && pszFolderFullPath[0] == chDrive)
   {
      strcpy(szPackName, pszFolderFullPath);
   }
   else
   {
      // commented bt001027
      // let UtlMakeEQFPath do it
      //if (pszFolderFullPath && strlen(pszFolderFullPath) == 0)
      //   chDrive = 0x00;

      // added bt001027
      if (chDrive == ' ')
         chDrive = 0x00;

      //--- build name of export directory ---
      UtlMakeEQFPath( szPackName, chDrive, EXPORT_PATH, NULL );

      chDrive = szPackName[0];

      if ( pszFolder )
      {
         strcat( szPackName, BACKSLASH_STR );
         strcat( szPackName, pszFolder );
      } /* endif */
   }

   if (pszFolderFullPath)
   {
      // write back
      strcpy(pszFolderFullPath, szPackName);
   }

   //--- check if drive has the approbriate directory (only if there is
   //    a path name in szPackName ---
   if ( isalpha(szPackName[0]) && (szPackName[1] == COLON) &&
        (szPackName[2] == BACKSLASH) && (szPackName[3] == EOS) )
   {
     // only root directory in szPackName, no check necessary
   }
   else
   {
     do {
        usCount = 1;                     // (re)set search count
        hDirHandle = HDIR_CREATE;        // use system file find handle
        usMBCode   = MBID_CANCEL;        // preset user response

        if (!UtlDirExist(szPackName))
        {
#ifdef DEBUG_LISTPACKAGES
           MessageBox ( NULL, "Directory does no exist", "Debug", MB_OK );
#endif
           fPathValid = FALSE ;
           break;
        }

        ulRC = UtlFindFirstLong( szPackName, &hDirHandle, FILE_DIRECTORY, &LongResultBuf, FALSE);

#ifdef DEBUG_LISTPACKAGES
        if ( ulRC ) 
        {
           sprintf( szDebugLine, "Return code %lu from UtlFindFirstLong(\"%s\")", ulRC, szPackName );
           MessageBox ( NULL, szDebugLine, "Debug", MB_OK );
        }
#endif

        if ( hDirHandle != HDIR_CREATE )
        {
           UtlFindClose( hDirHandle, FALSE );
        }

        if ( ulRC == ERROR_DISK_CHANGE )
        {
          usCount = 1;                     // (re)set search count
          hDirHandle = HDIR_CREATE;        // use system file find handle
          ulRC = UtlFindFirst( szPackName, &hDirHandle, FILE_DIRECTORY, &ResultBuf, sizeof( ResultBuf), &usCount, 0L, TRUE );
#ifdef DEBUG_LISTPACKAGES
          if ( ulRC ) 
          {
             sprintf( szDebugLine, "Return code %lu from UtlFindFirst(\"%s\") in ERROR_DISK_CHANGE handling", ulRC, szPackName );
             MessageBox ( NULL, szDebugLine, "Debug", MB_OK );
          }
#endif
          if ( hDirHandle != HDIR_CREATE )
             UtlFindClose( hDirHandle, FALSE );

        } /* endif */
        if ( (ulRC == ERROR_NOT_READY) || (ulRC == ERROR_DISK_CHANGE) )
        {
           strncpy( szErrParm, szPackName, 2 );
           szErrParm[2] = EOS;
           usMBCode = UtlError( (USHORT)ulRC, 0, 1, &pszErrParm, DOS_ERROR );
        } /* endif */
     } while ( ulRC && (usMBCode == MBID_RETRY) && fPathValid); /* enddo */
   } /* endif */
#ifdef DEBUG_LISTPACKAGES
      sprintf( szDebugLine, "After path check logic, ulRC=%lu, fPathValid=%s", ulRC, fPathValid ? "true" : "false" );
      MessageBox ( NULL, szDebugLine, "Debug", MB_OK );
#endif

   //--- add package search pattern to export path ---
   if ( !ulRC && fPathValid)
   {
      int sLen = strlen( szPackName );
      if ( szPackName[sLen-1] != BACKSLASH )
      {
        strcat( szPackName, "\\" );
        sLen++;
      } /* endif */
      pszFileName = szPackName + sLen;
      strcat( szPackName, pszSearchName );
   } /* endif */

   //--- look for matching files ---
   if ( !ulRC && fPathValid)
   {
      hDirHandle = HDIR_CREATE;        // use system file find handle
      ulRC = UtlFindFirstLong( szPackName, &hDirHandle, FILE_NORMAL, &LongResultBuf, TRUE);
#ifdef DEBUG_LISTPACKAGES
      if ( ulRC ) 
      {
        sprintf( szDebugLine, "Return code %lu from UtlFindFirstLong(\"%s\")", ulRC, szPackName );
        MessageBox ( NULL, szDebugLine, "Debug", MB_OK );
      }
#endif
   } /* endif */

#ifdef DEBUG_LISTPACKAGES
      sprintf( szDebugLine, "Before main loop, ulRC=%lu, fPathValid=%s", ulRC, fPathValid ? "true" : "false" );
      MessageBox ( NULL, szDebugLine, "Debug", MB_OK );
#endif

   while ( !ulRC && fPathValid)
   {
#ifdef DEBUG_LISTPACKAGES
      sprintf( szDebugLine, "Found file %s in directory %s", LongResultBuf.achName, pszFolderFullPath );
      MessageBox ( NULL, szDebugLine, "Debug", MB_OK );
#endif
      //--- append current file name to package directory info ---
      strcpy( pszFileName, LongResultBuf.achName );

      //--- open the package ---
      ulRC = UtlOpen( szPackName,
                      &hFilePackage,
                      &usOpenAction,
                      0L,
                      FILE_NORMAL,
                      FILE_OPEN,
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                      0L,
                      FALSE );

      //--- get the package header ---
      if ( !ulRC )
      {
#ifdef DEBUG_LISTPACKAGES
      sprintf( szDebugLine, "Open of file %s successful", szPackName );
      MessageBox ( NULL, szDebugLine, "Debug", MB_OK );
#endif
         ulRC = UtlReadL( hFilePackage,
                         &PackHead,
                         sizeof( PackHead ),
                         &ulBytesRead,
                         TRUE );
      } /* endif */

      //--- close the package file ---
      if ( hFilePackage )
      {
         UtlClose( hFilePackage, FALSE );
      } /* endif */

      //--- check package header and add package to listbox (if ok) ---
      if ( !ulRC )
      {
#ifdef DEBUG_LISTPACKAGES
      MessageBox ( NULL, "Checking package header", "Debug", MB_OK );
#endif
         if ( ((memcmp( PackHead.bPackID, PACKHEADID, 4 ) == 0) ||
               (memcmp( PackHead.bPackID, PACKHEAD2ID, 4 ) == 0) ||
               (memcmp( PackHead.bPackID, PACKHEAD3ID, 4 ) == 0)) && // if ID OK and
              ( (PackHead.fCompleted          &&    //  package completed and
                (PackHead.usSequence == 1) ) ||     // first package file or
                (usFlags & PACKFLAG_LISTALL) ) )    // first package file or
         {
#ifdef DEBUG_LISTPACKAGES
      MessageBox ( NULL, "Header is ok", "Debug", MB_OK );
#endif

            //--- cut of file extension ---
            pszExtension = strrchr( LongResultBuf.achName, DOT );
            if ( !(usFlags & PACKFLAG_WITHEXT) && pszExtension )
            {
               *pszExtension = NULC;
            } /* endif */

            //--- add package name to listbox ---
            INSERTITEMHWND( hwndLB, LongResultBuf.achName );
         } /* endif */
      } /* endif */

      //--- get next file ---
      ulRC = UtlFindNextLong( hDirHandle, &LongResultBuf, TRUE);

   } /* endwhile */

   // close search file handle
   if ( hDirHandle != HDIR_CREATE )
      UtlFindCloseLong( hDirHandle, FALSE );

   ulRC = ( ulRC == ERROR_NO_MORE_FILES && fPathValid) ? NO_ERROR : ulRC;

   return( ulRC );
} /* end of UtlPackListPackages */

PVOID UtlPackOpenPackage
(
  CHAR chDrive,
  PSZ pszFolder,
  PSZ pszFolderFullPath,
  PSZ pszPackName,
  HWND hwndOwner
)
{
   PPACKCONTROL pPackCtrl;             // ptr to package control area
   BOOL         fOK = TRUE;            // internal OK flag
   USHORT       usOpenAction;          // action performed by DosOpen

   //--- allocate package control area ---
   fOK = UtlAllocHwnd( (PVOID *)&pPackCtrl, 0L, (LONG) sizeof(PACKCONTROL), ERROR_STORAGE, hwndOwner );

   //--- initialize pack control variables and buffers ---
   if ( fOK )
   {
      memset( pPackCtrl, 0, sizeof(PACKCONTROL) ); //redundant: done by UtlAlloc
      pPackCtrl->pInBuf = NULL;
      pPackCtrl->ulInBufSize = 0;
      pPackCtrl->pOutBuf = NULL;
      pPackCtrl->ulOutBufSize = 0;

      pPackCtrl->chTargetDrive = chDrive;

      strcpy( pPackCtrl->szPackName, pszPackName );
      pPackCtrl->hwndOwner = hwndOwner;
      if ( pszFolder && (pszFolder[0] != EOS))
      {
         strcpy( pPackCtrl->szFolder, pszFolder );
         pPackCtrl->fFolder = TRUE;
      }
      else
      {
         pPackCtrl->fFolder = FALSE;
      } /* endif */
   } /* endif */

   //--- open the package file for read access ---
   if ( fOK )
   {
      if (!pszFolderFullPath || (pszFolderFullPath && strlen(pszFolderFullPath) == 0))
      {
          UtlMakeEQFPath( pPackCtrl->szFullPackName,
                      pPackCtrl->chTargetDrive,
                      EXPORT_PATH, NULL );

          if ( pPackCtrl->fFolder )
          {
             strcat( pPackCtrl->szFullPackName, "\\" );
             strcat( pPackCtrl->szFullPackName, pPackCtrl->szFolder );
          } /* endif */
      }
      else
      {
            strcpy( pPackCtrl->szFullPackName, pszFolderFullPath );

          // remove Backslash if only drive e.g. "C:\" is given

          if (pPackCtrl->szFullPackName[strlen(pPackCtrl->szFullPackName)-1] == '\\')
             pPackCtrl->szFullPackName[strlen(pPackCtrl->szFullPackName)-1] = 0x00;
      }

      strcat( pPackCtrl->szFullPackName, "\\" );
      strcat( pPackCtrl->szFullPackName, pPackCtrl->szPackName );
      pPackCtrl->usDosRC = UtlOpenHwnd( pPackCtrl->szFullPackName,
                         &pPackCtrl->hFilePackage,
                         &usOpenAction, 0L,
                         FILE_NORMAL,
                         FILE_OPEN,
                         OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                         0L,
                         TRUE, pPackCtrl->hwndOwner );
      fOK = (pPackCtrl->usDosRC == 0);
   } /* endif */

   //--- read system package header ---
   if ( fOK )
   {
      pPackCtrl->ProcessState  = PACK_OPEN;
      UtlPackReadSysHead( pPackCtrl, &pPackCtrl->PackHead );
      fOK = (pPackCtrl->usDosRC == 0);
   } /* endif */

   //--- clean up in case of errors ---
   if ( !fOK )
   {
      if ( pPackCtrl )
      {
         if ( pPackCtrl->hFilePackage )
         {
            UtlClose( pPackCtrl->hFilePackage, FALSE );
         } /* endif */
         UtlAlloc( (PVOID *)&pPackCtrl, 0L, 0L, NOMSG );
         pPackCtrl = NULL;
      } /* endif */
   } /* endif */

   //--- return package control area pointer to caller
   return( pPackCtrl );
} /* end of UtlPackOpenPackage */

ULONG UtlPackQueryHeaderSize( PVOID pVoidPack )
{
   ULONG ulHeaderSize = 0;            // size of user header area

   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( pPackCtrl->ProcessState != PACK_OPEN )
   {
      ulRC = PACK_INVOPERATION_RC;
   }
   else if ( (pPackCtrl->PackHead.ulUserHeaderSize == 0) ||
             (pPackCtrl->PackHead.usSequence != 1)      ||
             (!pPackCtrl->PackHead.fCompleted) )
   {
      ulRC = PACK_NOHEADER_RC;
   }
   else
   {
      ulHeaderSize = pPackCtrl->PackHead.ulUserHeaderSize;
   } /* endif */

   return( ulHeaderSize );
}

ULONG UtlPackQueryRequiredSpace( PVOID pVoidPack, PULONG pulSpace )
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code
   ULONG        ulNoOfEntries;         // no of file list entries to process
   PFILELISTENTRY pFileEntry;          // ptr to an entry in a file list

   pPackCtrl = (PPACKCONTROL) pVoidPack;
   *pulSpace = 0L;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( pPackCtrl->ProcessState != PACK_OPEN )
   {
      ulRC = PACK_INVOPERATION_RC;
   }
   else
   {
      //--- read file table if not already done ---
      if ( pPackCtrl->FileList.pEntries == NULL )
      {
         ulRC = UtlPackReadFileList( pPackCtrl );
      } /* endif */

      if ( ulRC == PACK_OK_RC )
      {
         //--- get overall size of files ---
         pFileEntry = pPackCtrl->FileList.pEntries;
         ulNoOfEntries = pPackCtrl->FileList.ulListUsed;
         while ( ulNoOfEntries )
         {
            if ( !(pFileEntry->usProcessFlags & FILE_SKIP) )
            {
               *pulSpace += pFileEntry->ulFileSize;
            } /* endif */
            pFileEntry++;
            ulNoOfEntries--;
         } /* endwhile */
      } /* endif */
   } /* endif */

   return( ulRC );
}

ULONG  UtlPackGetFileListPtr ( PVOID pVoidPack, PFILELIST *ppFileList )
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code

   pPackCtrl = (PPACKCONTROL) pVoidPack;
   *ppFileList = NULL;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( pPackCtrl->ProcessState != PACK_OPEN )
   {
      ulRC = PACK_INVOPERATION_RC;
   }
   else
   {
      //--- read file table if not already done ---
      if ( pPackCtrl->FileList.pEntries == NULL )
      {
         ulRC = UtlPackReadFileList( pPackCtrl );
      } /* endif */

      if ( ulRC == PACK_OK_RC )
      {
         *ppFileList = &pPackCtrl->FileList;
      } /* endif */
   } /* endif */

   return( ulRC );
}

ULONG UtlPackSetTargetDrive( PVOID pVoidPack, CHAR chDrive )
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( pPackCtrl->ProcessState != PACK_OPEN )
   {
      ulRC = PACK_INVOPERATION_RC;
   }
   else
   {
      pPackCtrl->chTargetDrive = chDrive;
   } /* endif */

   return( ulRC );
}

ULONG  UtlPackReadFileList( PVOID pVoidPack )
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code
   ULONG        ulNoOfEntries;         // no of file list entries to process
   PFILELISTENTRY pFileEntry;          // ptr to an entry in a file list
   BOOL         fOK = TRUE;            // OK flag returned by UtlAlloc
   ULONG        ulBytesRead;           // # of bytes read from package

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( pPackCtrl->ProcessState != PACK_OPEN )
   {
      ulRC = PACK_INVOPERATION_RC;
   }
   else
   {
      fOK = UtlAllocHwnd( (PVOID *)&pPackCtrl->FileList.pEntries,
                      0L,
                      pPackCtrl->PackHead.ulFileListSize,
                      ERROR_STORAGE, pPackCtrl->hwndOwner );
      if ( fOK )
      {
         fOK = UtlAllocHwnd( (PVOID *)&pPackCtrl->FileList.pBuffer,
                         0L,
                         pPackCtrl->PackHead.ulFileNameBufferSize,
                         ERROR_STORAGE, pPackCtrl->hwndOwner );
      } /* endif */

      if ( fOK )
      {
         //--- position to start of file list in package file ---
         {
            LONG lTablePos;
            ULONG ulNewPos;
            if ( (memcmp( pPackCtrl->PackHead.bPackID, PACKHEADID, 4 ) == 0) ||
                 (memcmp( pPackCtrl->PackHead.bPackID, PACKHEAD2ID, 4 ) == 0) )
            {
              // version 0 or 1 header
              lTablePos = sizeof(PACKHEADER);
            }
            else
            {
              // version 2 header
              lTablePos = sizeof(PACKHEADER2);
            } /* endif */
            lTablePos += pPackCtrl->PackHead.ulUserHeaderSize,
            pPackCtrl->usDosRC = UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,
                                     lTablePos,
                                     FILE_BEGIN,
                                     &ulNewPos,
                                     TRUE, pPackCtrl->hwndOwner );
         }

         //--- read file list into file list buffer ---
         if ( !pPackCtrl->usDosRC )
         {
           ULONG ulRemaining = pPackCtrl->PackHead.ulFileListSize;
           ULONG ulPos       = 0L;

           while ( !pPackCtrl->usDosRC && (ulRemaining != 0L) )
           {
             ULONG ulBytesToRead = ( ulRemaining > 0x8000L ) ? 0x8000L : ulRemaining;

             pPackCtrl->usDosRC = UtlReadHwnd( pPackCtrl->hFilePackage,
                     ((PSZ)pPackCtrl->FileList.pEntries) + ulPos,
                     (USHORT)ulBytesToRead,
                     &ulBytesRead,
                     TRUE, pPackCtrl->hwndOwner );
             if ( ulBytesToRead != ulBytesRead )
             {
               ulRC = PACK_READFAILED_RC;
               if ( !pPackCtrl->usDosRC ) pPackCtrl->usDosRC = 1;
             } /* endif */

             if ( !pPackCtrl->usDosRC )
             {
               ulPos       += ulBytesToRead;
               ulRemaining -= ulBytesToRead;
             } /* endif */
           } /* endwhile */
         } /* endif */

         if ( pPackCtrl->usDosRC )
         {
            ulRC = PACK_READFAILED_RC;
         }
         else
         {
            pPackCtrl->FileList.ulListSize =
               pPackCtrl->FileList.ulListUsed =
                  pPackCtrl->PackHead.ulFileListEntries;

            //--- read file name buffer  ---
            if ( !pPackCtrl->usDosRC )
            {
              ULONG ulRemaining = pPackCtrl->PackHead.ulFileNameBufferSize;
              ULONG ulPos       = 0L;

              while ( !pPackCtrl->usDosRC && (ulRemaining != 0L) )
              {
                ULONG ulBytesToRead = ( ulRemaining > 0x8000L ) ? 0x8000L : ulRemaining;

                pPackCtrl->usDosRC = UtlReadHwnd( pPackCtrl->hFilePackage,
                        ((PSZ)pPackCtrl->FileList.pBuffer) + ulPos,
                        (USHORT)ulBytesToRead,
                        &ulBytesRead,
                        TRUE, pPackCtrl->hwndOwner );
                if ( ulBytesToRead != ulBytesRead )
                {
                  ulRC = PACK_READFAILED_RC;
                  if ( !pPackCtrl->usDosRC ) pPackCtrl->usDosRC = 1;
                } /* endif */

                if ( !pPackCtrl->usDosRC )
                {
                  ulPos       += ulBytesToRead;
                  ulRemaining -= ulBytesToRead;
                } /* endif */
              } /* endwhile */
            } /* endif */

            if ( pPackCtrl->usDosRC )
            {
               ulRC = PACK_READFAILED_RC;
            }
            else
            {
               pPackCtrl->FileList.ulBufferSize =
                  pPackCtrl->FileList.ulBufferUsed =
                     pPackCtrl->PackHead.ulFileNameBufferSize;

               //--- change offset of file names to pointers and reset file
               //    processed flags ---
               pFileEntry = pPackCtrl->FileList.pEntries;
               ulNoOfEntries = pPackCtrl->FileList.ulListUsed;
               while ( ulNoOfEntries )
               {
                  ULONG ulOffs;
                  pFileEntry->usProcessFlags &= ~FILE_IS_PROCESSED;
                  ulOffs = (ULONG)pFileEntry->pszName;
                  // fix for offsets corrupted bug!
                  if ( ulOffs > pPackCtrl->FileList.ulBufferUsed )
                  {
                    ulOffs &= 0x0000FFFF;
                  } /* endif */
                  pFileEntry->pszName = pPackCtrl->FileList.pBuffer + ulOffs;
                  pFileEntry++;
                  ulNoOfEntries--;
               } /* endwhile */
            } /* endif */
         } /* endif */
         /*************************************************************/
         /* in case of error free allocated resources                 */
         /*************************************************************/
         if ( ulRC )
         {
            if ( pPackCtrl->FileList.pEntries )
            {
              UtlAlloc( (PVOID *)&pPackCtrl->FileList.pEntries, 0L, 0L, NOMSG );
            } /* endif */
            if ( pPackCtrl->FileList.pBuffer )
            {
              UtlAlloc( (PVOID *)&pPackCtrl->FileList.pBuffer, 0L, 0L, NOMSG );
            } /* endif */
         } /* endif */
      }
      else
      {
         ulRC = PACK_NOMEMORY_RC;
      } /* endif */
   } /* endif */

   return( ulRC );
}   /* end of UtlPackReadFileList */

ULONG UtlPackReadHeader
(
   PVOID pVoidPack,                    // ptr to package control area
   PBYTE pHeader,                      // where to store the user header
   ULONG ulHeaderSize                  // size of header area
)
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code
   ULONG        ulFilePos;             // current position of file pointer
   ULONG        ulBytesRead;           // # of bytes read from package
   ULONG        ulBytesToRead;         // # of bytes to read from package

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( pPackCtrl->ProcessState != PACK_OPEN )
   {
      ulRC = PACK_INVOPERATION_RC;
   }
   else if ( pPackCtrl->PackHead.ulUserHeaderSize == 0L)
   {
      ulRC = PACK_NOHEADER_RC;
   }
   else
   {
      //--- position to start of user header in package file ---
      LONG lPos;

      if ( (memcmp( pPackCtrl->PackHead.bPackID, PACKHEADID, 4 ) == 0) ||
           (memcmp( pPackCtrl->PackHead.bPackID, PACKHEAD2ID, 4 ) == 0) )
      {
        // version 0 or 1 header
        lPos = sizeof(PACKHEADER);
      }
      else
      {
        // version 2 header
        lPos = sizeof(PACKHEADER2);
      } /* endif */
      UtlChgFilePtrHwnd( pPackCtrl->hFilePackage, lPos, FILE_BEGIN, &ulFilePos,
                         TRUE, pPackCtrl->hwndOwner );

      //--- read user header into the given buffer (in 32k blocks) ---
      ulBytesToRead =  min( (ULONG)0x8000, ulHeaderSize );
      while ( (ulRC == NO_ERROR) && ulBytesToRead )
      {
        UtlReadHwnd( pPackCtrl->hFilePackage, pHeader, ulBytesToRead, &ulBytesRead,
                 TRUE, pPackCtrl->hwndOwner );
        if ( ulBytesToRead != ulBytesRead )
        {
           ulRC = PACK_READFAILED_RC;
        }
        else
        {
          ulHeaderSize -= ulBytesToRead;
          pHeader      += ulBytesToRead;
          ulBytesToRead =  min( (ULONG)0x8000, ulHeaderSize );
        } /* endif */
      } /* endwhile */
   } /* endif */

   return( ulRC );
}

ULONG UtlPackClosePackage( PVOID pVoidPack )
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( pPackCtrl->ProcessState != PACK_OPEN )
   {
      ulRC = PACK_INVOPERATION_RC;
   }
   else
   {
      UtlClose( pPackCtrl->hFilePackage, FALSE );
   } /* endif */

   if ( pPackCtrl )
   {
      if ( pPackCtrl->pInBuf )
      {
         UtlAlloc( (PVOID *)&pPackCtrl->pInBuf, 0L, 0L, NOMSG );
      } /* endif */

      if ( pPackCtrl->pOutBuf )
      {
         UtlAlloc( (PVOID *)&pPackCtrl->pOutBuf, 0L, 0L, NOMSG );
      } /* endif */

      if ( pPackCtrl->FileList.pEntries )
      {
        UtlAlloc( (PVOID *)&pPackCtrl->FileList.pEntries, 0L, 0L, NOMSG );
      } /* endif */

      if ( pPackCtrl->FileList.pBuffer )
      {
        UtlAlloc( (PVOID *)&pPackCtrl->FileList.pBuffer, 0L, 0L, NOMSG );
      } /* endif */

      UtlAlloc( (PVOID *)&pPackCtrl, 0L, 0L, NOMSG );
   } /* endif */

   return( ulRC );
};

ULONG UtlPackWriteFileList( PVOID pVoidPack )
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code
   ULONG        ulNoOfEntries;         // no of file list entries to process
   PFILELISTENTRY pFileEntry;          // ptr to an entry in a file list
   ULONG        ulNewPos;              // new position of file location pointer
   ULONG        ulBytesWritten;         // # of bytes written to file

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else
   {
      //--- position to file table location in package file ---
      if ( ulRC == PACK_OK_RC )
      {
         LONG lTablePos;

         if ( (memcmp( pPackCtrl->PackHead.bPackID, PACKHEADID, 4 ) == 0) ||
              (memcmp( pPackCtrl->PackHead.bPackID, PACKHEAD2ID, 4 ) == 0) )
         {
           // version 0 or 1 header
           lTablePos = sizeof(PACKHEADER);
         }
         else
         {
           // version 2 header
           lTablePos = sizeof(PACKHEADER2);
         } /* endif */
         lTablePos += pPackCtrl->PackHead.ulUserHeaderSize,
         pPackCtrl->usDosRC = UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,
                                  lTablePos,
                                  FILE_BEGIN,
                                  &ulNewPos,
                                  TRUE, pPackCtrl->hwndOwner );
         ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;
      } /* endif */


      //--- write file list ---
      if ( ulRC == PACK_OK_RC )
      {
         //--- change file name pointers to mere offset values ---
         pFileEntry = pPackCtrl->FileList.pEntries;
         ulNoOfEntries = pPackCtrl->FileList.ulListUsed;
         while ( ulNoOfEntries )
         {
            pFileEntry->pszName =
               (PSZ) (pFileEntry->pszName - pPackCtrl->FileList.pBuffer );
            pFileEntry++;
            ulNoOfEntries--;
         } /* endwhile */

         //--- write the file table to the package file ---
         {
           ULONG ulRemaining = pPackCtrl->PackHead.ulFileListSize;
           ULONG ulPos       = 0L;

           while ( !pPackCtrl->usDosRC && (ulRemaining != 0L) )
           {
             ULONG ulBytesToWrite = ( ulRemaining > 0x8000L ) ? 0x8000L : ulRemaining;

             pPackCtrl->usDosRC = UtlWriteWoCheckHwnd(
                     pPackCtrl->hFilePackage,
                     ((PSZ)pPackCtrl->FileList.pEntries) + ulPos,
                     (USHORT)ulBytesToWrite,
                     &ulBytesWritten,
                     TRUE, pPackCtrl->hwndOwner );
             ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;

             if ( !pPackCtrl->usDosRC )
             {
               ulPos       += ulBytesToWrite;
               ulRemaining -= ulBytesToWrite;
             } /* endif */
           } /* endwhile */
         } /* endif */

         //--- change file name pointers back to pointers ---
         pFileEntry = pPackCtrl->FileList.pEntries;
         ulNoOfEntries = pPackCtrl->FileList.ulListUsed;
         while ( ulNoOfEntries )
         {
            pFileEntry->pszName = pPackCtrl->FileList.pBuffer +
                                     (ULONG)pFileEntry->pszName;
            pFileEntry++;
            ulNoOfEntries--;
         } /* endwhile */
      } /* endif */

      //--- write file name buffer ---
      if ( ulRC == PACK_OK_RC )
      {
         {
           ULONG ulRemaining = pPackCtrl->PackHead.ulFileNameBufferSize;
           ULONG ulPos       = 0L;

           while ( !pPackCtrl->usDosRC && (ulRemaining != 0L) )
           {
             ULONG ulBytesToWrite = ( ulRemaining > 0x8000L ) ? 0x8000L : ulRemaining;

             pPackCtrl->usDosRC = UtlWriteWoCheckHwnd(
                     pPackCtrl->hFilePackage,
                     ((PSZ)pPackCtrl->FileList.pBuffer) + ulPos,
                     ulBytesToWrite,
                     &ulBytesWritten,
                     TRUE, pPackCtrl->hwndOwner );
             ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_WRITEFAILED_RC;

             if ( !pPackCtrl->usDosRC )
             {
               ulPos       += ulBytesToWrite;
               ulRemaining -= ulBytesToWrite;
             } /* endif */
           } /* endwhile */
         } /* endif */
      } /* endif */
   } /* endif */

   return( ulRC );
} /* end of UtlPackWriteFileList */


BOOL UtlPackHandleError( ULONG ulRC, PSZ pszParm, HWND hwnd )
{
   PSZ   pszFilename;                  // separated filename
   BOOL  fOK;                          // returned OK flag

   switch ( ulRC )
   {
      case PACK_OK_RC:
      case PACK_COMPLETE_RC:
         fOK = TRUE;
         break;
      case PACK_FILENOTFOUND_RC:
      case PACK_OPENFAILED_RC:
         pszFilename = UtlSplitFnameFromPath( pszParm);
         UtlErrorHwnd( ERROR_FILE_ACCESS_ERROR, MB_CANCEL, 1, &pszFilename,
                       EQF_ERROR, hwnd );
         fOK = FALSE;
         break;
      case PACK_NOMEMORY_RC:
         // no message required, has been done already by UtlAlloc
         fOK = FALSE;
         break;
      case PACK_NOFILES_RC:
      case PACK_NOCONTROLAREA_RC:
      case PACK_INVOPERATION_RC:
      case PACK_INVPROCESSSTATE_RC:
      case PACK_HEADEREXISTS_RC:
      case PACK_NOHEADER_RC:
         UtlErrorHwnd( ERROR_INTERNAL, MB_CANCEL, 0, NULL, EQF_ERROR,
                       hwnd );
         fOK = FALSE;
         break;
      case PACK_WRITEFAILED_RC:
         UtlErrorHwnd( ERROR_FILE_WRITE_ERROR, MB_CANCEL, 1, &pszParm, EQF_ERROR,
                       hwnd );
         fOK = FALSE;
         break;
      case PACK_TOO_MUCH_DOCUMENTS_RC:
         UtlErrorHwnd( ERROR_TOO_MUCH_DOC_IN_FOLDER, MB_CANCEL, 0, NULL,
                       EQF_ERROR, hwnd );
         fOK = FALSE;
         break;
      case PACK_READFAILED_RC:
         UtlErrorHwnd( ERROR_FOLDER_READ_FAILED, MB_CANCEL, 0, NULL, EQF_ERROR,
                       hwnd );
         fOK = FALSE;
         break;
      default:
         UtlErrorHwnd( ERROR_INTERNAL, MB_CANCEL, 0, NULL, EQF_ERROR,
                       hwnd );
         fOK = FALSE;
         break;
   } /* endswitch */
   return( fOK );
}

// Description: Get fully qualified path of directory where the imported
//              files will be/are stored. Return this path if requested.
//

ULONG  UtlPackQueryImportPath( PVOID pVoidPack,  PSZ pszImportPathBuffer )
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG        ulRC = PACK_OK_RC;     // function return code

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else if ( ( pPackCtrl->ProcessState == PACK_OPEN ) ||
             ( pPackCtrl->ProcessState == PACK_DONE ) ||
             ( pPackCtrl->ProcessState == PACK_INERROR ) ||
             ( pPackCtrl->ProcessState == PACK_TERMINATED ) )
   {
      // evaluate name of import directory if not done yet
      if ( !pPackCtrl->szImportPath[0] )
      {
        if ( pPackCtrl->szImportDir[0]  )
        {
          UtlMakeEQFPath( pPackCtrl->szImportPath,
                          pPackCtrl->chTargetDrive,
                          SYSTEM_PATH, NULL );
          strcat( pPackCtrl->szImportPath, BACKSLASH_STR );
          strcat( pPackCtrl->szImportPath, pPackCtrl->szImportDir );
        }
        else
        {
          UtlMakeEQFPath( pPackCtrl->szImportPath,
                          pPackCtrl->chTargetDrive,
                          IMPORT_PATH, NULL );
        } /* endif */           
      } /* endif */

      // return import path if requested
      if ( pszImportPathBuffer )
      {
         strcpy( pszImportPathBuffer, pPackCtrl->szImportPath );
      } /* endif */
   }
   else
   {
      ulRC = PACK_INVOPERATION_RC;
   } /* endif */

   return( ulRC );
}

ULONG UtlPackGetLastRC( PVOID pVoidPack,  PULONG pulRC )
{
   PPACKCONTROL pPackCtrl;             // pointer to package control data
   ULONG       ulRC = PACK_OK_RC;     // function return code

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   }
   else
   {
      *pulRC = pPackCtrl->usDosRC;
   } /* endif */

   return( ulRC );
}

// read the package header and convert to new format if necessary
static ULONG UtlPackReadSysHead( PPACKCONTROL pPackCtrl, PPACKHEADER2 pHead )
{
  ULONG     ulBytesRead;

  // read package header
  pPackCtrl->usDosRC = UtlReadHwnd( pPackCtrl->hFilePackage,
                                    pHead, sizeof(PACKHEADER2),
                                    &ulBytesRead,
                                    TRUE, pPackCtrl->hwndOwner );
  if ( (pPackCtrl->usDosRC == NO_ERROR) &&
       (ulBytesRead != sizeof(PACKHEADER2)) )
  {
    pPackCtrl->usDosRC = 1;            // dummy value, has to be != 0
  } /* endif */

  // convert package header to new format if necessary and correct
  // file pointer position
  if ( pPackCtrl->usDosRC == NO_ERROR )
  {
    if ( (memcmp( pHead->bPackID, PACKHEADID, 4 ) == 0) ||
         (memcmp( pHead->bPackID, PACKHEAD2ID, 4 ) == 0) )
    {
      // old format header

      // convert to new layout
      memcpy( &pPackCtrl->OldPackHead, pHead, sizeof(pPackCtrl->OldPackHead) );
      memset( pHead, 0, sizeof(PACKHEADER2) );
      memcpy( pHead->bPackID, pPackCtrl->OldPackHead.bPackID, 4 );
      pHead->usVersion              = pPackCtrl->OldPackHead.usVersion;
      pHead->ulPackDate             = pPackCtrl->OldPackHead.ulPackDate;
      pHead->usSequence             = pPackCtrl->OldPackHead.usSequence;
      pHead->fCompleted             = pPackCtrl->OldPackHead.fCompleted;
      pHead->fLastFileOfPackage     = pPackCtrl->OldPackHead.fLastFileOfPackage;
      pHead->ulUserHeaderSize       = pPackCtrl->OldPackHead.usUserHeaderSize;
      pHead->ulFileListSize         = pPackCtrl->OldPackHead.usFileListSize;
      pHead->ulFileListEntries      = pPackCtrl->OldPackHead.usFileListEntries;
      pHead->ulFileNameBufferSize   = pPackCtrl->OldPackHead.usFileNameBufferSize;

      // adjust file pointer position
      {
        ULONG ulNewPos;
        pPackCtrl->usDosRC = UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,
                                                (LONG)sizeof(PACKHEADER),
                                                FILE_BEGIN,
                                                &ulNewPos,
                                                TRUE, pPackCtrl->hwndOwner );
      }
    } /* endif */
  } /* endif */
  return( (ULONG)pPackCtrl->usDosRC );
} /* end of function UtlPackReadSysHead */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlPackReadFilePart                                      |
//+----------------------------------------------------------------------------+
//Description:       Reads a part of a file contained in a package            |
//+----------------------------------------------------------------------------+
//|Parameters:        x                                                        |
//|                   x                                                        |
//+----------------------------------------------------------------------------+
//|Returns:           x                                                        |
//|                   x                                                        |
//+----------------------------------------------------------------------------+
ULONG UtlPackReadFilePart
(
   PVOID  pVoidPack,                   // pointer to package control area
   PSZ    pszFile,                     // fully qualified file name
   ULONG  ulOffset,                    // offset within file
   PBYTE  pbBuffer,                    // buffer for the read data
   ULONG  ulLength                     // length of data to read

)
{
   PPACKCONTROL pPackCtrl;
   PFILELIST pFileList = NULL;         // ptr to filelist structure
   PFILELISTENTRY pFileEntry = NULL;   // ptr to an entry in a file list
   ULONG    ulRC = PACK_OK_RC;         // internal error code
   BOOL     fInList = FALSE;           // file found in list
   int      i;                         // loop index

   pPackCtrl = (PPACKCONTROL) pVoidPack;

   //--- check if pPackCtrl is set and package control state is OK ---
   if ( !pPackCtrl )
   {
      ulRC = PACK_NOCONTROLAREA_RC;    // no control area is available
   } /* endif */

    // read file list if not done yet
   if ( pPackCtrl->FileList.pEntries == NULL )
   {
      ulRC = UtlPackReadFileList( pPackCtrl );
   } /* endif */

   //--- get pointer to filelist structure ---
   if ( ulRC == PACK_OK_RC )
   {
      pFileList = &pPackCtrl->FileList;
   } /* endif */

   //--- search file in file list ---
   if ( ulRC == PACK_OK_RC )
   {
      pFileEntry = pFileList->pEntries;
      for ( i = 0; i < (SHORT)pFileList->ulListUsed; i++ )
      {
        if ( stricmp( pFileEntry->pszName + 1, pszFile + 1 ) == 0 )
        {
          fInList = TRUE;
          break;
        } /* endif */
        pFileEntry++;
      } /* endfor */
      if ( !fInList )
      {
        ulRC = PACK_FILENOTFOUND_RC;
      } /* endif */
   } /* endif */

   //--- position to start of requested data in package file
   if ( ulRC == PACK_OK_RC )
   {
      ULONG ulNewPos;
      UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,
                         pFileEntry->ulFilePos + ulOffset,
                         FILE_BEGIN,
                         &ulNewPos,
                         TRUE, pPackCtrl->hwndOwner );
      ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_READFAILED_RC;
   } /* endif */

   if ( ulRC == PACK_OK_RC )
   {
     //--- fill caller's data buffer ---
     ULONG  ulBytesRead;
     pPackCtrl->usDosRC = UtlReadHwnd( pPackCtrl->hFilePackage,
                                       pbBuffer,
                                       ulLength,
                                       &ulBytesRead,
                                       TRUE, pPackCtrl->hwndOwner );
         ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_READFAILED_RC;
   } /* endif */

   return( ulRC );
} /* end of UtlPackReadFilePart */


// Pack a new file into export folder package
static ULONG PackNewFile( PPACKCONTROL pPackCtrl, PSZ pFileName )
{
  ULONG ulRC = PACK_OK_RC;
  FILESTATUS  FileStatus;             // status information of packed file
  USHORT usOpenAction;

  pPackCtrl->usDosRC = UtlOpenHwnd( pFileName,
                     &pPackCtrl->hFilePacked,
                     &usOpenAction,
                     0L,
                     FILE_NORMAL,
                     FILE_OPEN,
                     OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                     0L,
                     FALSE, pPackCtrl->hwndOwner );
  if ( pPackCtrl->usDosRC )
  {
    if ( (pPackCtrl->pCurFile->usFileType == TAGTABLE_USEREXIT_FILE) ||
         (pPackCtrl->pCurFile->usFileType == TAGTABLE_USEREXITWIN_FILE) )
    {
      // no error message if user exits fail to export
      // just ensure that file will be ignored during import
      ulRC = PACK_OK_RC;
      pPackCtrl->usDosRC = NO_ERROR;
      pPackCtrl->pCurFile->pszName[0] = EOS;
      pPackCtrl->pCurFile->usProcessFlags |= FILE_IS_PROCESSED;
      pPackCtrl->pCurFile++;
      if ( pPackCtrl->pCurFile >= (pPackCtrl->FileList.pEntries +
                                   pPackCtrl->FileList.ulListUsed) )
      {
         pPackCtrl->ProcessState = PACK_COMPLETEPACKAGE;
      }
      else
      {
         pPackCtrl->ProcessState = PACK_NEWFILE;
      } /* endif */
    }
    else
    {
      UtlErrorHwnd( pPackCtrl->usDosRC, 0, 1,
                    &(pPackCtrl->pCurFile->pszName),
                    DOS_ERROR, pPackCtrl->hwndOwner );
      ulRC = (pPackCtrl->usDosRC == 0) ? PACK_OK_RC : PACK_OPENFAILED_RC;
    } /* endif */
  } /* endif */

  //--- get file information ---
  if ( (ulRC == PACK_OK_RC) &&
       (pPackCtrl->ProcessState == PACK_READFILE) )
  {
     pPackCtrl->usDosRC = UtlQFileInfoHwnd( pPackCtrl->hFilePacked,
                             1,              // info level 1
                             (PBYTE) &FileStatus,
                             sizeof(FileStatus),
                             TRUE, pPackCtrl->hwndOwner );
     if (pPackCtrl->usDosRC == 0)
     {
        pPackCtrl->pCurFile->ulFileSize = FileStatus.cbFile;
        pPackCtrl->pCurFile->stFileDate = FileStatus.fdateLastWrite;
        pPackCtrl->pCurFile->stFileTime = FileStatus.ftimeLastWrite;
        pPackCtrl->ulBytesRemaining     = FileStatus.cbFile;
        pPackCtrl->pCurFile->ulFilePos  = 0L;
        UtlChgFilePtrHwnd( pPackCtrl->hFilePackage,  // query current pos
                       0L,
                       FILE_CURRENT,
                       &pPackCtrl->pCurFile->ulFilePos,
                       TRUE, pPackCtrl->hwndOwner );
        pPackCtrl->pCurFile->ulPackedSize = 0L;
        pPackCtrl->pCurFile->usDiskNum  = pPackCtrl->PackHead.usSequence;
        pPackCtrl->ulInBufUsed = 0;
        pPackCtrl->ulOutBufUsed = 0;
        ulRC = PACK_OK_RC;
     }
     else
     {
        ulRC = PACK_OPENFAILED_RC;
     } /* endif */
  } /* endif */
  return ulRC;
}



static USHORT AsdIntoNonUnicodeDAInit ( PPACKCONTROL pPackCtrl )
{
  USHORT usRC;
  PDORGIDA pDorgIda = &pPackCtrl->NTMNonUnicodeDA.stDorgIda;

  Utlstrccpy( pDorgIda->szDictName,
              UtlGetFnameFromPath(pPackCtrl->pCurFile->pszName  ), DOT );

  //create full dict property path name
  UtlMakeEQFPath( pDorgIda->szString, NULC, PROPERTY_PATH, (PSZ)NULP );
  sprintf( pDorgIda->szOriginalProps, "%s\\%s%s",
           pDorgIda->szString, pDorgIda->szDictName, EXT_OF_DICTPROP );
  strcpy( pDorgIda->szOriginalAsd, pPackCtrl->pCurFile->pszName );
  Utlstrccpy( pDorgIda->szCopyAsd, pDorgIda->szOriginalAsd, DOT );
  strcat( pDorgIda->szCopyAsd, EXT_TMP_DIC );
  Utlstrccpy( pDorgIda->szCopyAsi, pDorgIda->szOriginalAsd, DOT );
  strcat( pDorgIda->szCopyAsi, EXT_TMP_DICTINDEX );


  if (DorgInit( pPackCtrl->hwndOwner, &pPackCtrl->NTMNonUnicodeDA.stDorgIda ))
  {
    // update version number to Non-unicode dictionary
    usRC = AsdDictSetOldVersion(pPackCtrl->NTMNonUnicodeDA.stDorgIda.hDict2);

    // we now can start working on the records ...
  }
  else
  {
    usRC = PACK_OPENFAILED_RC;
  }

  return usRC;
}

static USHORT AsdIntoNonUnicodeDAClose ( PPACKCONTROL pPackCtrl )
{
  USHORT usRc = 0;
  PDORGIDA pDorgIda = &pPackCtrl->NTMNonUnicodeDA.stDorgIda;

  //remove symbol so dict free for further use
  if ( pDorgIda->fFree )
  {
     PROPNAME ( pDorgIda->szString, pDorgIda->szDictName );
     REMOVESYMBOL( pDorgIda->szString );
     pDorgIda->fFree = FALSE;
  } /* endif */

  //release memory
  if ( pDorgIda->pucDictData )
     UtlAlloc( (PVOID *) &pDorgIda->pucDictData, 0L, 0L, NOMSG );
  if ( pDorgIda->pucASCIIDictData )
     UtlAlloc( (PVOID *) &pDorgIda->pucASCIIDictData, 0L, 0L, NOMSG) ;

  if ( pPackCtrl->hDict)
  {
    AsdClose(pPackCtrl->hDictUser, pPackCtrl->hDict );
    pPackCtrl->hDict = NULL;
  } /* endif */

  if (pPackCtrl->hDictUser)
  {
    AsdEnd( pPackCtrl->hDictUser );
    pPackCtrl->hDictUser = NULL;
  } /* endif */


  if (pDorgIda->hDict1)
  {
    AsdClose(pDorgIda->hUser, pDorgIda->hDict1 );
    pDorgIda->hDict1 = NULL;
  } /* endif */

  if (pDorgIda->hDict2)
  {
    AsdClose(pDorgIda->hUser, pDorgIda->hDict2 );
    pDorgIda->hDict2 = NULL;
  } /* endif */

  // delete any prepared dictionary files



  if (pDorgIda->hUser)
  {
    AsdEnd( pDorgIda->hUser );
    pDorgIda->hUser = NULL;
  } /* endif */


  return usRc;
}

static USHORT AsdIntoNonUnicodeDAWork ( PPACKCONTROL pPackCtrl )
{
  USHORT usRC = 0;
  PDORGIDA pDorgIda = &pPackCtrl->NTMNonUnicodeDA.stDorgIda;

  // get entry, convert it, store it, position to next entry
  if ( !pDorgIda->fKill )     //processing is not to be stopped
  {
    ULONG   ulTermNum;
    HDCB    hDictFound;        // handle of dict in which term was found
    PCHAR   pucNewRecord = NULL;
    ULONG   ulNewRecLen = 0;

    usRC = AsdNxtTermW( pDorgIda->hDict1,
                        pDorgIda->hUser,
                        pDorgIda->ucTermBuf,
                        &ulTermNum,
                        &pDorgIda->ulDataLen,  // in # of w's
                        &hDictFound );

    if ( usRC == LX_RC_OK_ASD )
    {
      //retrieve one term from either Unicode or old dict
      usRC = AsdRetEntryW( pDorgIda->hDict1,
                           pDorgIda->hUser,
                           pDorgIda->ucTermBuf,
                           &ulTermNum,
                           pDorgIda->pucDictData,
                           &pDorgIda->ulDataLen, // of w's
                           &hDictFound );

      //convert record to a QLDB tree and vice-versa
      if (usRC == LX_RC_OK_ASD)
      {
        USHORT  usLDBRc;
        PVOID   phLDBTree = NULL;

        pucNewRecord = NULL;
        usLDBRc = QLDBRecordToTree( pDorgIda->ausNoOfFields,
                                    pDorgIda->pucDictData,
                                    pDorgIda->ulDataLen,
                                    &phLDBTree );

        if ( usLDBRc == QLDB_NO_ERROR )
        {
          // convert to old format ...
          usLDBRc = QLDBTreeToRecordNonUnicode( phLDBTree,
                                                &pucNewRecord,
                                                &ulNewRecLen );
        } /* endif */

        if ( phLDBTree )
        {
          QLDBDestroyTree( &phLDBTree );
        } /* endif */

        if ( usLDBRc != QLDB_NO_ERROR )
        {
          //entry seems to be corrupted
          usRC = LX_UNEXPECTED_ASD;
        } /* endif */
      } /* endif */

      //insert term in temporary dictionary Dict2
      if (usRC == LX_RC_OK_ASD)
      {
        usRC = AsdInsEntryNonUnicode( pDorgIda->hUser,
                                      pDorgIda->hDict2,
                                      pDorgIda->ucTermBuf,
                                      (PUCHAR)pucNewRecord,
                                      ulNewRecLen,
                                      &ulTermNum );
        UtlAlloc( (PVOID *) &pucNewRecord, 0L, 0L, NOMSG );
      } /* endif */

      if ( usRC != LX_RC_OK_ASD )
      {
        //dam error in AsdInsEntry
        PSZ pszTerm = QDAMErrorString( usRC, pDorgIda->pszMsgError );
        UtlErrorHwnd ( usRC, MB_CANCEL, 1, &pszTerm, QDAM_ERROR, pPackCtrl->hwndOwner );
      } /* endif */

      pDorgIda->ulCurrentEntry++;       //update counter
    }
    else
    {
      if ( usRC == LX_EOF_ASD )
      {
        //finish off and indicate what to do next
        usRC = PACK_OK_RC;
        pDorgIda->fNoOrgError = TRUE;
        pPackCtrl->NTMNonUnicodeDA.usAction = NTM_PREP_COPY;

        if (pDorgIda->hDict1)
        {
          AsdClose(pDorgIda->hUser, pDorgIda->hDict1 );
          pDorgIda->hDict1 = NULL;
        } /* endif */

        // write out dictionary stuff
        usRC =  AsdResynch( pDorgIda->hUser, pDorgIda->hDict2 );

      }
      else
      {
        PSZ pszDict =  QDAMErrorString( usRC, pDorgIda->pszMsgError );
        UtlErrorHwnd ( usRC, MB_CANCEL, 1, &pszDict, QDAM_ERROR, pPackCtrl->hwndOwner );
      } /* endif */
    }
  }
  return usRC;
}


//   End of EQFFOL03.C

