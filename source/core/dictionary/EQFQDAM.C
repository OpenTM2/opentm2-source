/*! \file

	Description:  The underlying technique used is a modified B+ tree.
				  Each node in the B+ Tree contains a variable number of keys.
				  The key data are stored separately, but in the same
				  file.
				  The elements in the non-leaf nodes contain
				  pointers to other nodes within the tree.
				  The leaf nodes contain pointers to the actual data,
				  which are stored in the same file.

				  The leaf nodes are chained so that the data can be
				  searched both sequentially and via a key.
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqfqdami.h>             // Private QDAM defines
#include <eqfcmpr.h>              // defines for compression/expand...
#include <eqfchtbl.h>             // character tables
#include <time.h>
#include "eqfevent.h"                  // event logging facility

#if defined(ASDLOGGING)
 extern FILE *hAsdLog;
 #define ASDLOG()                \
    if ( hAsdLog )               \
       fprintf( hAsdLog, "QDAM: %s, %d \n",__FILE__, __LINE__ );    \
    else                         \
       DosBeep( 1200, 200 );
#else
 #define ASDLOG()
#endif

extern UCHAR chDefCollate[];

/**********************************************************************/
/* 'Magic word' for record containing locked terms                    */
/**********************************************************************/
static CHAR szLOCKEDTERMSKEY[] = "0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";
static CHAR_W szLOCKEDTERMSKEYW[] = L"0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictSignLocal    Read User Data
//------------------------------------------------------------------------------
// Function call:     QDAMDictSignLocal( PBTREE, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Gets the second part of the first record ( user data )
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    PUSHORT                length of user data area (input)
//                                           filled length (output)
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//
//------------------------------------------------------------------------------
// Side effects:      return signature record even if dictionary is corrupted
//
//------------------------------------------------------------------------------
// Function flow:
//                    position to begin of data start in first record
//                    if not possible set  Rc = BTREE_READ_ERROR
//                    if not Rc
//                      read in length of userdata
//                      if ok
//                        if user requests only length
//                          return only length
//                        else
//                          if length not correct
//                            set Rc =BTREE_USERDATA
//                          else
//                            read in user data
//                          endif
//                        endif
//                      else
//                        set Rc = BTREE_READ_ERROR
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictSignLocal
(
   PBTREE pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
)
{
  SHORT  sRc=0;                        // return code
  USHORT  usNumBytesRead;              // bytes read from disk
  ULONG   ulNewOffset;                 // new offset
  USHORT  usLen;                       // contain length of user record
  PBTREEGLOB  pBT = pBTIda->pBTree;

  if ( UtlChgFilePtr( pBT->fp, (LONG) USERDATA_START,
                      FILE_BEGIN, &ulNewOffset, FALSE) )
  {
    sRc = BTREE_READ_ERROR;
  } /* endif */

  if ( ! sRc )
  {
     ASDLOG();

     sRc = UtlRead( pBT->fp, (PVOID) &usLen, sizeof(USHORT),
                    &usNumBytesRead, FALSE );
     if ( !sRc )
     {
        // user requests only length or full data
        if ( ! pUserData  || *pusLen == 0 )
        {
           *pusLen = usLen;         // return only length
        }
        else
        {
           if ( *pusLen <  usLen )
           {
              sRc = BTREE_USERDATA;
           }
           else
           {
              // read in data part
              sRc = UtlRead( pBT->fp, pUserData, usLen,
                             &usNumBytesRead, FALSE );
              if ( !sRc )
              {
                 *pusLen = usLen;
              }
              else
              {
                 sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
              } /* endif */
           } /* endif */
        } /* endif */

        ASDLOG();
     }
     else
     {
        sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
     } /* endif */
  } /* endif */
  return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictOpenLocal   Open local Dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictOpenLocal( PSZ, SHORT, BOOL, PPBTREE );
//
//------------------------------------------------------------------------------
// Description:       Open a file locally for processing
//
//------------------------------------------------------------------------------
// Parameters:        PSZ              name of the index file
//                    SHORT            number of bytes per record
//                    BOOL             TRUE  read/write FALSE  read/only
//                    PPBTREE          pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_ILLEGAL_FILE not a valid dictionary
//                    BTREE_CORRUPTED   dictionary is corrupted
//------------------------------------------------------------------------------
// Function flow:     Open the file read/write or readonly
//                    if ok
//                      read in header
//                      if ok
//                        check for validity of the header
//                        if ok
//                          fill tree structure
//                          call UtlQFileInfo to get the next free record
//                          if ok
//                            allocate buffer space
//                            if alloc fails
//                              set Rc = BTREE_NO_ROOM
//                            endif
//                          endif
//                        endif
//                      else
//                        set Rc = BTREE_READ_ERROR
//                      endif
//                      if error
//                        close dictionary
//                      endif
//                      if read/write open was done
//                        set open flag and write it into file
//                      endif
//                      get corruption flag and set rc if nec.
//                    else
//                     set Rc = BTREE_OPEN_ERROR
//                    endif
//                    return Rc
//------------------------------------------------------------------------------
SHORT  QDAMDictOpenLocal
(
  PSZ   pName,                        // name of the file
  SHORT sNumberOfBuffers,             // number of buffers
  USHORT usOpenFlags,                 // Read Only or Read/Write
  PPBTREE  ppBTIda                    // pointer to BTREE structure
)
{
   SHORT     i;
   BTREEHEADRECORD header;
   SHORT sRc = 0;                      // return code
   USHORT  usFlags;                    // set the open flags
   USHORT  usAction;                   // return code from UtlOpen
   USHORT  usNumBytesRead;             // number of bytes read
   PBTREE  pBTIda = *ppBTIda;          // set work pointer to passed pointer
   PBTREEGLOB pBT;                     // set work pointer to passed pointer
   BOOL    fWrite = usOpenFlags & (ASD_GUARDED | ASD_LOCKED);
   BOOL    fTransMem = FALSE;          // Transl. Memory...
   SHORT   sRc1;

   sNumberOfBuffers;
   /*******************************************************************/
   /* allocate global area first ...                                  */
   /*******************************************************************/
   if ( ! UtlAlloc( (PVOID *)&pBT, 0L, (LONG) sizeof(BTREEGLOB ), NOMSG ) )
   {
      sRc = BTREE_NO_ROOM;
   }
   else
   {
      pBTIda->pBTree = pBT;

      if ( usOpenFlags & ASD_SHARED )
      {
         usFlags = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE;
      }
      else if ( fWrite )
      {
         usFlags = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE;
      }
      else
      {
         usFlags = OPEN_ACCESS_READONLY | OPEN_SHARE_DENYREADWRITE;
      } /* endif */

      /****************************************************************/
      /* check if immeadiate write of any changed necessary           */
      /****************************************************************/
      pBT->fGuard = ( usOpenFlags & ASD_FORCE_WRITE );
      pBT->usOpenFlags = usOpenFlags;

      ASDLOG();

      // open the file
      sRc = UtlOpen( pName, &pBT->fp, &usAction, 0L,
                     FILE_NORMAL, FILE_OPEN,
                     usFlags,
                     0L, FALSE);
      ASDLOG();
   } /* endif */


   if ( !sRc )
   {
     // remember file name
     strcpy( pBTIda->szFileName, pName );

     // Read in the data for this index, make sure it is an index file

     sRc = UtlRead( pBT->fp, (PVOID)&header,
                    sizeof(BTREEHEADRECORD), &usNumBytesRead, FALSE);
     if ( ! sRc )
     {
        memcpy( pBT->chEQF, header.chEQF, sizeof(pBT->chEQF));
        pBT->bVersion = header.Flags.bVersion;
        if ( header.Flags.f16kRec )
        {
          pBT->bRecSizeVersion = BTREE_V3;
        }
        else
        {
          pBT->bRecSizeVersion = BTREE_V2;
        } /* endif */

        /**************************************************************/
        /* support either old or new format or TM...                  */
        /**************************************************************/
        if ( (strncmp( header.chEQF, BTREE_HEADER_VALUE_V3,
                      sizeof(BTREE_HEADER_VALUE_V3) ) == 0) ||
             (strncmp( header.chEQF, BTREE_HEADER_VALUE_V2,
                      sizeof(BTREE_HEADER_VALUE_V2) ) == 0) ||
             (strncmp( header.chEQF, BTREE_HEADER_VALUE_V0,
                      sizeof(BTREE_HEADER_VALUE_V0) ) == 0) ||
             (strncmp( header.chEQF, BTREE_HEADER_VALUE_V1,
                      sizeof(BTREE_HEADER_VALUE_V1) ) == 0) ||
             (strncmp( header.chEQF, BTREE_HEADER_VALUE_TM2,
                      sizeof(BTREE_HEADER_VALUE_TM2) ) == 0) ||
             (strncmp( header.chEQF, BTREE_HEADER_VALUE_TM1,
                      sizeof(BTREE_HEADER_VALUE_TM1) ) == 0) ||
             (strncmp( header.chEQF, BTREE_HEADER_VALUE_TM3,
                      sizeof(BTREE_HEADER_VALUE_TM1) ) == 0) )
        {
          /************************************************************/
          /* check if we are dealing with a TM...                     */
          /* (Check only first 3 characters of BTREE identifier to    */
          /*  ignore the version number)                              */
          /************************************************************/
          if ( strncmp( header.chEQF, BTREE_HEADER_VALUE_TM1, 3 ) == 0 )
          {
            fTransMem = TRUE;
            pBT->usVersion = (USHORT) header.chEQF[3];
            pBT->compare =  NTMKeyCompare;
          }
          else
          {
            pBT->compare =  QDAMKeyCompare;
            /************************************************************/
            /* determine version                                        */
            /* for UNICODE dicts: usVersion is BTREE_VERSION2, which is */
            /* the same as NTM_VERSION2 ( has also length field         */
            /* ULONG at begin to support data recs > 32 k      )        */
            /************************************************************/
            if ( header.chEQF[3] == BTREE_VERSION3 )
            {
              pBT->usVersion = BTREE_VERSION3;
            }
            else
            if ( header.chEQF[3] == BTREE_VERSION2 )
            {
                pBT->usVersion = BTREE_VERSION2;
                header.fOpen = TRUE;  // force sRc = BTREE_CORRUPTED in ln 946
            }
            else
            {
                pBT->usVersion = 0;
                pBT->compare =  QDAMKeyCompareNonUnicode;

                /******************************************************/
                /* we do not support version 0 any more -> force a    */
                /* reorganize                                         */
                /******************************************************/
                header.fOpen = TRUE;  // force sRc = BTREE_CORRUPTED in ln 946
 //               sRc = BTREE_CORRUPTED;
            } /* endif */
          } /* endif */
          UtlTime( &(pBT->lTime) );                             // set open time
          pBTIda->sCurrentIndex = 0;
          pBT->usFirstNode = header.usFirstNode;
          pBT->usFirstLeaf = header.usFirstLeaf;
          pBTIda->usCurrentRecord = 0;
          pBT->usFreeKeyBuffer = header.usFreeKeyBuffer;
          pBT->usFreeDataBuffer = header.usFreeDataBuffer;
          pBT->usFirstDataBuffer = header.usFirstDataBuffer;  //  data buffer
          pBT->fTransMem = fTransMem;
          pBT->fpDummy = NULLHANDLE;
          if ( pBT->bRecSizeVersion == BTREE_V3 )
          {
            pBT->usBtreeRecSize = BTREE_REC_SIZE_V3;
          }
          else
          {
            pBT->usBtreeRecSize = BTREE_REC_SIZE_V2;
          } /* endif */

          // load usNextFreeRecord either from header record of from file info
          if ( pBT->bVersion == BTREE_V1 )
          {
              pBT->usNextFreeRecord = header.usNextFreeRecord;
          }
          else
          {
            ULONG ulTemp;
            sRc1 = UtlGetFileSize( pBT->fp, &ulTemp, FALSE );
            if (!sRc)
              sRc = sRc1;
            pBT->usNextFreeRecord = (USHORT)(ulTemp/pBT->usBtreeRecSize);
          } /* endif */
          ASDLOG();

          if ( !sRc )
          {
             strcpy(pBT->chFileName, pName);

             // copy prev. allocated free list
             // DataRecList in header is in old format (RECPARAMOLD),
             // so convert it to the new format (RECPARAM)
             {
               int i;
               for ( i = 0; i < MAX_LIST; i++ )
               {
                 pBT->DataRecList[i].usOffset = header.DataRecList[i].usOffset;
                 pBT->DataRecList[i].usNum    = header.DataRecList[i].usNum;
                 pBT->DataRecList[i].ulLen    = (ULONG)header.DataRecList[i].sLen;
               } /* endfor */
             }
             memcpy( pBT->chEntryEncode,header.chEntryEncode,ENTRYENCODE_LEN );
             pBT->fTerse = header.fTerse;
             if ( pBT->fTerse == BTREE_TERSE_HUFFMAN )
             {
               QDAMTerseInit( pBTIda, pBT->chEntryEncode );   // init compression
             } /* endif */

             memcpy( pBT->chCollate, header.chCollate, COLLATE_SIZE );
             /*********************************************************/
             /* check if something is in collating sequence - this is */
             /* mandatory for the dictionary processing -             */
             /* use the character A as checking point                 */
             /* if nothing in there, we will use the default collating*/
             /* sequence - in addition we will put it in the          */
             /*********************************************************/
             if ( (! pBT->fTransMem) && (pBT->chCollate[65] == 0) )
             {
               memcpy( pBT->chCollate, chDefCollate, COLLATE_SIZE );
             } /* endif */

             memcpy( pBT->chCaseMap, header.chCaseMap, COLLATE_SIZE );
             if ( (!pBT->fTransMem) && (pBT->chCaseMap[65] == 0) )
             {
               /****************************************************************/
               /* fill in the characters and use the UtlLower function ...     */
               /****************************************************************/
               PUCHAR pTable;
               UCHAR  chTemp;
               pTable = pBT->chCaseMap;
               for ( i=0;i < COLLATE_SIZE; i++ )
               {
                  *pTable++ = (UCHAR) i;
               } /* endfor */
               chTemp = pBT->chCaseMap[ COLLATE_SIZE - 1];
               pBT->chCaseMap[ COLLATE_SIZE - 1] = EOS;
               pTable = pBT->chCaseMap;
               pTable++;
               UtlLower( (PSZ)pTable );
               pBT->chCaseMap[ COLLATE_SIZE - 1] = chTemp;
             } /* endif */

             ASDLOG();


             /* Allocate space for AccessCtrTable */
             pBT->usNumberOfLookupEntries = 0;
             UtlAlloc( (PVOID *)&pBT->AccessCtrTable, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
             if ( !pBT->AccessCtrTable )
             {
               sRc = BTREE_NO_ROOM;
             } /* endif */

             /* Allocate space for LookupTable */
             if ( !sRc )
             {
                if ( pBT->bRecSizeVersion == BTREE_V3 )
                {
                  UtlAlloc( (PVOID *)&pBT->LookupTable_V3, 0L,(LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(LOOKUPENTRY_V3), NOMSG );
                  if ( pBT->LookupTable_V3 )
                  {
                    pBT->usNumberOfLookupEntries = MIN_NUMBER_OF_LOOKUP_ENTRIES;
                  }
                  else
                  {
                    sRc = BTREE_NO_ROOM;
                  } /* endif */
                } /* endif */
                else
                {
                  UtlAlloc( (PVOID *)&pBT->LookupTable_V2, 0L,(LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(LOOKUPENTRY_V2), NOMSG );
                  if ( pBT->LookupTable_V2 )
                  {
                    pBT->usNumberOfLookupEntries = MIN_NUMBER_OF_LOOKUP_ENTRIES;
                  }
                  else
                  {
                    sRc = BTREE_NO_ROOM;
                  } /* endif */
                } /* endif */
             } /* endif */

             pBT->usNumberOfAllocatedBuffers = 0;

          } /* endif */

          if ( !sRc )
          {
            ASDLOG();

            if ( !sRc )
            {
              sRc =  QDAMAllocTempAreas( pBTIda );
            } /* endif */
          } /* endif */
        }
        else
        {
           sRc = BTREE_ILLEGAL_FILE;
        } /* endif */
     }
     else
     {
        sRc = QDAMDosRC2BtreeRC( sRc, BTREE_OPEN_ERROR, pBT->usOpenFlags );
     } /* endif */
     ASDLOG();

     /*******************************************************************/
     /* for shared databases only:                                      */
     /* open/create dummy file as update semaphore and buffer for       */
     /* locked terms                                                   */
     /*                                                                */
     /* for all other databases:                                         */
     /* delete any existing dummy file                                 */
     /*******************************************************************/
     if ( !sRc )
     {
        CHAR szDummyName[MAX_EQF_PATH];   // buffer for name of dummy file
        PSZ  pszExt;                      // points to extension of file name

        // Setup name of dummy file
        strcpy( szDummyName, pName );
        pszExt = strrchr( szDummyName, DOT );
        pszExt[2] = '-';


         if ( usOpenFlags & ASD_SHARED )
         {
           sRc = UtlOpen( szDummyName, &pBT->fpDummy, &usAction, 0L,
                          FILE_NORMAL, FILE_OPEN | FILE_CREATE,
                          OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
                          0L, FALSE);
          sRc = QDAMDosRC2BtreeRC( sRc, BTREE_OPEN_ERROR, pBT->usOpenFlags );
          if ( !sRc )
          {
            if ( usAction == FILE_CREATED )
            {
//              // Write initial update counter setting
//              sRc = UtlWrite( pBT->fpDummy, (PVOID)pBT->alUpdCtr,
//                              sizeof(pBT->alUpdCtr), &usNumBytes, FALSE );
//              if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_WRITE_ERROR, pBT->usOpenFlags );

              // Close and re-open dummy file to set correct share flags
              if ( !sRc )
              {
                UtlClose( pBT->fpDummy, FALSE );
                pBT->fpDummy = NULLHANDLE;
                sRc = UtlOpen( szDummyName, &pBT->fpDummy, &usAction, 0L,
                            FILE_NORMAL, FILE_OPEN | FILE_CREATE,
                            OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
                            0L, FALSE);
                sRc = QDAMDosRC2BtreeRC( sRc, BTREE_OPEN_ERROR, pBT->usOpenFlags );
              } /* endif */

              // Write initial file update counter
              if ( sRc == NO_ERROR )
              {
                memset( pBT->alUpdCtr, 0, sizeof(pBT->alUpdCtr) );
                sRc = QDAMIncrUpdCounter( pBTIda, 0, NULL );
              } /* endif */
            }
            else
            {
              // Read last update counter
              sRc = QDAMGetUpdCounter( pBTIda, pBT->alUpdCtr, 0, MAX_UPD_CTR );
            } /* endif */
          }
          else
          {
            pBT->fpDummy = NULLHANDLE;
          } /* endif */
        }
        else
        {
          UtlDelete( szDummyName, 0L, FALSE );
        } /* endif */
     } /* endif */

     // close in case of error
     if ( sRc && pBTIda )
     {
       QDAMDictClose( &pBTIda );
     } /* endif */
     ASDLOG();

     if ( !sRc && fWrite && !header.fOpen)
     {
        pBT->fOpen = TRUE;                       // set open flag
        pBT->fWriteHeaderPending = TRUE;         // postpone write until change
     } /* endif */

     if ( !sRc && (pBT->usOpenFlags & (ASD_SHARED | ASD_NOOPENCHECK)))
     {
        pBT->fOpen = TRUE;                       // set open flag
     } /* endif */


     // get the corruption flag but still go ahead - nec. for Organize
     // ignore the corruption flag for shared resources (here we allow
     // open of our database more than once)
     if ( !sRc && header.fOpen &&
          !(pBT->usOpenFlags & (ASD_SHARED | ASD_NOOPENCHECK)) )
     {
        sRc = BTREE_CORRUPTED;

        /**************************************************************/
        /* if in Write Mode opened disable write in case of Corruption*/
        /* and reset it to Read/Only mode                             */
        /**************************************************************/
        if ( fWrite  )
        {
          i  = UtlSetFHandState( pBT->fp, OPEN_ACCESS_READONLY, FALSE );

          if ( i )               // handle could not be set read/only
          {
            pBT->fCorrupted = TRUE;
          } /* endif */
        } /* endif */
     } /* endif */

     ASDLOG();
   }
   else
   {
    sRc = QDAMDosRC2BtreeRC( sRc, BTREE_OPEN_ERROR, pBT->usOpenFlags );
   } /* endif */

   /*******************************************************************/
   /* set BTREE pointer in case it was changed or freed               */
   /*******************************************************************/
   *ppBTIda = pBTIda;

   /*******************************************************************/
   /* add the dictionary to the open list ...                         */
   /*******************************************************************/
   if ( !sRc || (sRc == BTREE_CORRUPTED) )
   {
     QDAMAddDict( pName, pBTIda );
     /*****************************************************************/
     /* add the lock if necessary                                     */
     /*****************************************************************/
     if ( usOpenFlags & ASD_LOCKED )
     {
       QDAMDictLockDictLocal( pBTIda, TRUE );
     } /* endif */
   } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMDICTOPENLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

   return ( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCloseLocal  close the dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictCloseLocal( PPBTREE );
//
//------------------------------------------------------------------------------
// Description:       Close the file
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_CLOSE_ERROR error closing dictionary
//------------------------------------------------------------------------------
// Function flow:     if corrupted
//                      set RC = BTREE_CORRUPTED
//                    else
//                      Flush all records
//                    endif
//                    if okay then
//                      reset open flag in header and force a write to disk
//                    endif
//                    if okay then
//                      close file and set RC to BTREE_CLOSE_ERROR in case of
//                      problems
//                    endif
//                    if okay then
//                      free space of buffers
//                      free space of BTree
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictCloseLocal
(
   PBTREE pBTIda
)
{
   SHORT sRc = 0;                            // error return
   PBTREEGLOB  pBT = NULL;

   /*******************************************************************/
   /* validate passed pointer ...                                     */
   /*******************************************************************/
   CHECKPBTREE( pBTIda, sRc );
   if ( !sRc )
   {
     pBT = pBTIda->pBTree;
   } /* endif */

   /*******************************************************************/
   /* decrement the counter and check if we have to physically close  */
   /* the dictionary                                                  */
   /*******************************************************************/
   if ( !sRc && ! QDAMRemoveDict( pBTIda ) )
   {
      sRc = QDAMDictFlushLocal( pBTIda );

     //  reset open flag in header and force a write to disk
     // open flag will only be set if opened for r/w
     if ( !sRc && pBT->fOpen && ! pBT->fCorrupted )
     {
        pBT->fOpen = FALSE;

        // Only for non-shared databases:
        // re-write header record
        if ( !(pBT->usOpenFlags & ASD_SHARED) || !(pBT->usOpenFlags & ASD_LOCKED & ASD_GUARDED) )
        {
          if ( sRc == NO_ERROR ) sRc = QDAMWriteHeader( pBTIda );
        } /* endif */
     } /* endif */

     if ( UtlClose(pBT->fp, FALSE) && !sRc )
     {
        sRc = BTREE_CLOSE_ERROR;
     } /* endif */

     if ( pBT->fpDummy )
     {
      UtlClose( pBT->fpDummy, FALSE );
     } /* endif */

     /*******************************************************************/
     /* free the allocated buffers                                      */
     /*******************************************************************/
     if ( pBT  )
     {
       /* free allocated space for lookup-table and buffers */
       if ( pBT->bRecSizeVersion == BTREE_V3)
       {
        if ( pBT->LookupTable_V3 )
        {
          USHORT i;
          PLOOKUPENTRY_V3 pLEntry = pBT->LookupTable_V3;

          for ( i=0; i < pBT->usNumberOfLookupEntries; i++ )
          {
            if ( pLEntry->pBuffer )
            {
              UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L, NOMSG );
            } /* endif */
            pLEntry++;
          } /* endfor */

          UtlAlloc( (PVOID *)&pBT->LookupTable_V3, 0L, 0L, NOMSG );
          UtlAlloc( (PVOID *)&pBT->AccessCtrTable, 0L, 0L, NOMSG );
          pBT->usNumberOfLookupEntries = 0;
          pBT->usNumberOfAllocatedBuffers = 0;
        } /* endif */
       }
       else
       {
        if ( pBT->LookupTable_V2 )
        {
          USHORT i;
          PLOOKUPENTRY_V2 pLEntry = pBT->LookupTable_V2;

          for ( i=0; i < pBT->usNumberOfLookupEntries; i++ )
          {
            if ( pLEntry->pBuffer )
            {
              UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L, NOMSG );
            } /* endif */
            pLEntry++;
          } /* endfor */

          UtlAlloc( (PVOID *)&pBT->LookupTable_V2, 0L, 0L, NOMSG );
          UtlAlloc( (PVOID *)&pBT->AccessCtrTable, 0L, 0L, NOMSG );
          pBT->usNumberOfLookupEntries = 0;
          pBT->usNumberOfAllocatedBuffers = 0;
        } /* endif */
       } /* endif */

       /*****************************************************************/
       /* free index buffer list                                        */
       /*****************************************************************/
       if ( pBT->bRecSizeVersion == BTREE_V3)
       {
         PBTREEINDEX_V3 pIndexBuffer, pTempIndexBuffer;  // temp ptr for freeing index
         pIndexBuffer = pBT->pIndexBuffer_V3;
         while ( pIndexBuffer  )
         {
           pTempIndexBuffer = pIndexBuffer->pNext;
           UtlAlloc( (PVOID *)&pIndexBuffer, 0L, 0L, NOMSG );
           pIndexBuffer = pTempIndexBuffer;
         } /* endwhile */
       }
       else
       {
         PBTREEINDEX_V2 pIndexBuffer, pTempIndexBuffer;  // temp ptr for freeing index
         pIndexBuffer = pBT->pIndexBuffer_V2;
         while ( pIndexBuffer  )
         {
           pTempIndexBuffer = pIndexBuffer->pNext;
           UtlAlloc( (PVOID *)&pIndexBuffer, 0L, 0L, NOMSG );
           pIndexBuffer = pTempIndexBuffer;
         } /* endwhile */
       } /* endif */

       UtlAlloc( (PVOID *)&pBT->pTempKey, 0L, 0L, NOMSG );
       UtlAlloc( (PVOID *)&pBT->pTempRecord, 0L, 0L, NOMSG );
       UtlAlloc( (PVOID *)&pBTIda->pQDAMLanIn, 0L, 0L, NOMSG );  // free allocated memory
       UtlAlloc( (PVOID *)&pBTIda->pQDAMLanOut,0L, 0L, NOMSG );     // free allocated memory

       UtlAlloc( (PVOID *)&pBTIda->pBTree,0L, 0L, NOMSG );     // free allocated memory
     } /* endif */
     /********************************************************************/
     /* unlock the dictionary                                            */
     /* -- do not take care about return code...                         */
     /********************************************************************/
     QDAMDictLockDictLocal( pBTIda, FALSE );
   } /* endif */


  if ( sRc )
  {
    ERREVENT2( QDAMDICTCLOSELOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */


   return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictExactLocal   Find Exact match
//------------------------------------------------------------------------------
// Function call:     QDAMDictExactLocal( PBTREE, PCHAR, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:        Find an exact match for the passed key
//
//------------------------------------------------------------------------------
// Parameters:         PBTREE               pointer to btree structure
//                     PCHAR                key to be inserted
//                     PCHAR                buffer for user data
//                     PUSHORT              on input length of buffer
//                                          on output length of filled data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted, then
//                      set Rc = BTREE_CORRUPTED
//                    else
//                      locate the leaf node that contains appropriate key
//                      set Rc correspondingly
//                    endif
//                    if okay so far then
//                      locate the key with option set to FEXACT;  set Rc
//                      if key found then
//                        set current position to the found key
//                        pass back either length only or already data,
//                         depending on user request.
//                        endif
//                      else
//                        set Rc to BTREE_NOT_FOUND
//                        set current position to the nearest key
//                      endif
//                    endif
//                    return Rc
//
//
//------------------------------------------------------------------------------

SHORT QDAMDictExactLocal
(
   PBTREE pBTIda,                      // pointer to btree struct
   PCHAR_W pKey,                       // key to be searched for
   PBYTE  pchBuffer,                   // space for user data
   PULONG pulLength                    // in/out length of returned user data
)
{
  SHORT    i;
  SHORT    sNearKey;                   // nearest key position
  SHORT    sRc  = 0;                   // return code
  RECPARAM     recData;                // point to data structure
  PBTREEGLOB    pBT = pBTIda->pBTree;

  if ( pBT->fCorrupted )
  {
     sRc = BTREE_CORRUPTED;
  } /* endif */

   if ( !sRc )
   {
     if ( pBT->bRecSizeVersion == BTREE_V3)
     {
       PBTREEBUFFER_V3 pRecord = NULL;
       SHORT sRetries = MAX_RETRY_COUNT;
       do
       {
          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
         if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
         {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

          if ( !sRc )
          {
             /* Locate the Leaf node that contains the appropriate key */
             sRc = QDAMFindRecord_V3( pBTIda, pKey, &pRecord );
          } /* endif */

          if ( !sRc )
          {
            sRc = QDAMLocateKey_V3(pBTIda, pRecord, pKey, &i, FEXACT, &sNearKey);
            if ( !sRc )
            {
               if ( i != -1 )
               {
                  // set new current position
                 pBTIda->sCurrentIndex = i;
                 pBTIda->usCurrentRecord = RECORDNUM( pRecord );

                 if ( pBT->fTransMem )
                 {
                   memcpy( pBTIda->chHeadTerm, pKey, sizeof(ULONG) );  // save data
                 }
                 else
                 {
                   UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
                 } /* endif */
                 recData = QDAMGetrecData_V3( pRecord, i, pBT->usVersion );
                 if ( *pulLength == 0 || ! pchBuffer )
                 {
                    *pulLength = recData.ulLen;
                 }
                 else if ( *pulLength < recData.ulLen )
                 {
                    *pulLength = recData.ulLen;
                    sRc = BTREE_BUFFER_SMALL;
                 }
                 else
                 {
                    sRc = QDAMGetszData_V3( pBTIda, recData, pchBuffer, pulLength, DATA_NODE );
                 } /* endif */
               }
               else
               {
                 sRc = BTREE_NOT_FOUND;
                  // set new current position
                 pBTIda->sCurrentIndex = sNearKey;
                 pBTIda->usCurrentRecord = RECORDNUM( pRecord );
                 *pulLength = 0;            // init returned length
               } /* endif */
            } /* endif */
          } /* endif */

         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) && (sRetries > 0));
     }
     else
     {
       PBTREEBUFFER_V2 pRecord = NULL;
       SHORT sRetries = MAX_RETRY_COUNT;
       do
       {
          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
         if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
         {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

          if ( !sRc )
          {
             /* Locate the Leaf node that contains the appropriate key */
             sRc = QDAMFindRecord_V2( pBTIda, pKey, &pRecord );
          } /* endif */

          if ( !sRc )
          {
            sRc = QDAMLocateKey_V2(pBTIda, pRecord, pKey, &i, FEXACT, &sNearKey);
            if ( !sRc )
            {
               if ( i != -1 )
               {
                  // set new current position
                 pBTIda->sCurrentIndex = i;
                 pBTIda->usCurrentRecord = RECORDNUM( pRecord );

                 if ( pBT->fTransMem )
                 {
                   memcpy( pBTIda->chHeadTerm, pKey, sizeof(ULONG) );  // save data
                 }
                 else
                 {
                   UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
                 } /* endif */
                 recData = QDAMGetrecData_V2( pRecord, i, pBT->usVersion );
                 if ( *pulLength == 0 || ! pchBuffer )
                 {
                    *pulLength = recData.ulLen;
                 }
                 else if ( *pulLength < recData.ulLen )
                 {
                    *pulLength = recData.ulLen;
                    sRc = BTREE_BUFFER_SMALL;
                 }
                 else
                 {
                    sRc = QDAMGetszData_V2( pBTIda, recData, pchBuffer, pulLength, DATA_NODE );
                 } /* endif */
               }
               else
               {
                 sRc = BTREE_NOT_FOUND;
                  // set new current position
                 pBTIda->sCurrentIndex = sNearKey;
                 pBTIda->usCurrentRecord = RECORDNUM( pRecord );
                 *pulLength = 0;            // init returned length
               } /* endif */
            } /* endif */
          } /* endif */

         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) && (sRetries > 0));
     } /* endif */
   } /* endif */


  if ( sRc && (sRc != BTREE_NOT_FOUND))
  {
    ERREVENT2( QDAMDICTEXACTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return ( sRc );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictSubStrLocal   Find Key starting with stubstring
//------------------------------------------------------------------------------
// Function call:     QDAMDictSubStrLocal( PBTREE, PCHAR, PCHAR, PUSHORT,
//                                         PCHAR,  PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Find the first key starting with the passed key and
//                    pass it back.
//                    Special attention has to be taken in the cases where
//                    the first entry of a record is the substring!!!
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key to be looked for
//                    PCHAR                  buffer for the key
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//                    PCHAR                  buffer for the user data
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted, then
//                      set Rc = BTREE_CORRUPTED
//                    else
//                      locate the leaf node that contains appropriate key
//                      set Rc correspondingly
//                    endif
//                    if okay so far then
//                      call QDAMLocSubstr to locate the substring
//                      if Rc is BTREE_NOT_FOUND and next record exists,
//                      try to locate the substring in the next record
//                    endif
//                    return Rc
//------------------------------------------------------------------------------

SHORT QDAMDictSubStrLocal
(
   PBTREE pBTIda,                      // pointer to btree struct
   PCHAR_W  pKey,                        // key to be searched for
   PBYTE  pchBuffer,                   // space for key data
   PULONG pulLength,                  // in/out length of returned key data
   PBYTE  pchUserData,                 // space for user data
   PULONG pulUserLen                  // in/out length of returned user data
)
{
  SHORT    sRc  = 0;                   // return code
  PBTREEGLOB    pBT = pBTIda->pBTree;

  if ( pBT->fCorrupted )
  {
     sRc = BTREE_CORRUPTED;
  } /* endif */

   if ( !sRc )
   {
     if ( pBT->bRecSizeVersion == BTREE_V3 )
     {
       PBTREEBUFFER_V3 pRecord = NULL;
       SHORT sRetries = MAX_RETRY_COUNT;
       ULONG ulKeyLen;

       do
       {
          ulKeyLen = *pulLength;
          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
          if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
          {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

          if ( !sRc )
          {
             UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
             /* Locate the Leaf node that contains the appropriate key */
             sRc = QDAMFindRecord_V3( pBTIda, pKey, &pRecord );
          } /* endif */
          if ( !sRc )
          {
             sRc = QDAMLocSubstr_V3( pBTIda, pRecord, pKey, pchBuffer, &ulKeyLen, pchUserData, pulUserLen );

             /*****************************************************************/
             /* if substring not found try it again in the next record, we may*/
             /* have missed a matching substring due to the construction of   */
             /* the dictionary.                                               */
             /* This only could happen if the best matching key is the last   */
             /* one in the current record.                                    */
             /* Example: look for term starting with 'pig' and 'pigskin' is   */
             /*          the first entry in a record.                         */
             /*          Due to the construction 'pig' will be looked for in  */
             /*          the previous record, since it is '<' than 'pigskin'  */
             /*****************************************************************/
             if ( (sRc == BTREE_NOT_FOUND) &&
                  ((SHORT) OCCUPIED( pRecord ) == pBTIda->sCurrentIndex))
             {
               if ( NEXT(pRecord)  )
               {
                 sRc = QDAMReadRecord_V3( pBTIda, NEXT( pRecord ), &pRecord, FALSE  );
                 if ( !sRc )
                 {
                    ulKeyLen = *pulLength;
                    sRc = QDAMLocSubstr_V3(pBTIda, pRecord, pKey, pchBuffer, &ulKeyLen, pchUserData, pulUserLen );
                 } /* endif */
               } /* endif */
             } /* endif */
          } /* endif */

         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (sRetries > 0));
       *pulLength = ulKeyLen;
     }
     else
     {
       PBTREEBUFFER_V2 pRecord = NULL;
       SHORT sRetries = MAX_RETRY_COUNT;
       ULONG ulKeyLen;

       do
       {
          ulKeyLen = *pulLength;
          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
          if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
          {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

          if ( !sRc )
          {
             UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
             /* Locate the Leaf node that contains the appropriate key */
             sRc = QDAMFindRecord_V2( pBTIda, pKey, &pRecord );
          } /* endif */
          if ( !sRc )
          {
             sRc = QDAMLocSubstr_V2( pBTIda, pRecord, pKey, pchBuffer, &ulKeyLen, pchUserData, pulUserLen );

             /*****************************************************************/
             /* if substring not found try it again in the next record, we may*/
             /* have missed a matching substring due to the construction of   */
             /* the dictionary.                                               */
             /* This only could happen if the best matching key is the last   */
             /* one in the current record.                                    */
             /* Example: look for term starting with 'pig' and 'pigskin' is   */
             /*          the first entry in a record.                         */
             /*          Due to the construction 'pig' will be looked for in  */
             /*          the previous record, since it is '<' than 'pigskin'  */
             /*****************************************************************/
             if ( (sRc == BTREE_NOT_FOUND) &&
                  ((SHORT) OCCUPIED( pRecord ) == pBTIda->sCurrentIndex))
             {
               if ( NEXT(pRecord)  )
               {
                 sRc = QDAMReadRecord_V2( pBTIda, NEXT( pRecord ), &pRecord, FALSE  );
                 if ( !sRc )
                 {
                    ulKeyLen = *pulLength;
                    sRc = QDAMLocSubstr_V2(pBTIda, pRecord, pKey, pchBuffer, &ulKeyLen, pchUserData, pulUserLen );
                 } /* endif */
               } /* endif */
             } /* endif */
          } /* endif */

         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (sRetries > 0));
       *pulLength = ulKeyLen;
     } /* endif */
   } /* endif */


  if ( sRc && (sRc != BTREE_NOT_FOUND))
  {
    ERREVENT2( QDAMDICTSUBSTRLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return ( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictEquivLocal Find equivalent match
//------------------------------------------------------------------------------
// Function call:     QDAMDictEquivLocal(PBTREE, PCHAR, PCHAR, PUSHORT,
//                                       PCHAR, PUSHORT );
//------------------------------------------------------------------------------
// Description:       Find the first key which is equivalent to
//                    the passed key
//                    If no error happened set this location as
//                    new current position
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE               pointer to btree structure
//                    PCHAR                key to be looked for
//                    PCHAR                buffer for the key
//                    PUSHORT              on input length of buffer
//                                         on output length of filled data
//                    PCHAR                buffer for the user data
//                    PUSHORT              on input length of buffer
//                                         on output length of filled data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted, then
//                      set Rc = BTREE_CORRUPTED
//                    else
//                      locate the leaf node that contains appropriate key
//                      set Rc correspondingly
//                    endif
//                    if okay so far then
//                      locate the key with option set to FEQUIV;  set Rc
//                      if key found then
//                        check that key fulfills substring option
//                        if not okay
//                          set Rc  = BTREE_NOT_FOUND
//                        else
//                          set current position to the found key
//                          pass back either length only or already data,
//                           depending on user request.
//                        endif
//                      else
//                        set Rc to BTREE_NOT_FOUND
//                        set current position to the nearest key
//                      endif
//                    endif
//                    return Rc
//
//------------------------------------------------------------------------------

SHORT QDAMDictEquivLocal
(
   PBTREE pBTIda,                      // pointer to btree struct
   PCHAR_W  pKey,                      // key to be searched for
   PBYTE  pchBuffer,                   // space for key data
   PULONG pulLength,                  // in/out length of returned key data
   PBYTE  pchUserData,                // space for user data
   PULONG pulUserLen                  // in/out length of returned user data
)
{
  SHORT    i;
  SHORT    sNearKey;                   // nearest key found
  SHORT    sRc  = 0;                   // return code
  RECPARAM     recData;                // point to data structure
  PCHAR_W  pKey2;                      // pointer to key
  PBTREEGLOB    pBT = pBTIda->pBTree;

  ASDLOG();

  if ( pBT->fCorrupted )
  {
     sRc = BTREE_CORRUPTED;
  } /* endif */

   if ( !sRc )
   {
     if ( pBT->bRecSizeVersion == BTREE_V3)
     {
       PBTREEBUFFER_V3 pRecord = NULL;
       SHORT sRetries = MAX_RETRY_COUNT;
       ULONG ulLength;

       do
       {
          ulLength = *pulLength;

          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
          if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
          {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

           if ( !sRc )
           {
              UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
              /* Locate the Leaf node that contains the appropriate key */
              sRc = QDAMFindRecord_V3( pBTIda, pKey, &pRecord );
           } /* endif */

           if ( !sRc )
           {
             sRc = QDAMLocateKey_V3(pBTIda, pRecord, pKey, &i, FEQUIV, &sNearKey );
             if ( !sRc )
             {
                if ( i != -1 )
                {
                   // set new current position
                   pBTIda->sCurrentIndex = i;
                   pBTIda->usCurrentRecord = RECORDNUM( pRecord );

                   pKey2 = QDAMGetszKey_V3(  pRecord, i, pBT->usVersion );
                   if ( pKey2 )
                   {
                      UTF16strcpy( (PSZ_W)pchBuffer, pKey2 );                // copy data for key
                      *pulLength = UTF16strlenBYTE( pKey2 ) + sizeof(CHAR_W);               /* @KIT0912U     */
                      recData = QDAMGetrecData_V3( pRecord, i, pBT->usVersion );
                      if ( *pulUserLen == 0 || ! pchUserData )
                      {
                         *pulUserLen = recData.ulLen;
                      }
                      else
                      {
                         sRc =  QDAMGetszData_V3( pBTIda, recData, pchUserData, pulUserLen, DATA_NODE );
                      } /* endif */
                   }
                   else
                   {
                     sRc = BTREE_CORRUPTED;
                   } /* endif */
                }
                else
                {
                   sRc = BTREE_NOT_FOUND;
                   *pulUserLen = 0;
                   // set new current position
                   pBTIda->sCurrentIndex = sNearKey;
                   pBTIda->usCurrentRecord = RECORDNUM( pRecord );
                } /* endif */
             } /* endif */
             ASDLOG();
           } /* endif */

         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (sRetries > 0));
       *pulLength = ulLength;
     }
     else
     {
       PBTREEBUFFER_V2 pRecord = NULL;
       SHORT sRetries = MAX_RETRY_COUNT;
       ULONG ulLength;

       do
       {
          ulLength = *pulLength;

          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
          if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
          {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

           if ( !sRc )
           {
              UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
              /* Locate the Leaf node that contains the appropriate key */
              sRc = QDAMFindRecord_V2( pBTIda, pKey, &pRecord );
           } /* endif */

           if ( !sRc )
           {
             sRc = QDAMLocateKey_V2(pBTIda, pRecord, pKey, &i, FEQUIV, &sNearKey );
             if ( !sRc )
             {
                if ( i != -1 )
                {
                   // set new current position
                   pBTIda->sCurrentIndex = i;
                   pBTIda->usCurrentRecord = RECORDNUM( pRecord );

                   pKey2 = QDAMGetszKey_V2(  pRecord, i, pBT->usVersion );
                   if ( pKey2 )
                   {
                      UTF16strcpy( (PSZ_W)pchBuffer, pKey2 );                // copy data for key
                      *pulLength = UTF16strlenBYTE( pKey2 ) + sizeof(CHAR_W);               /* @KIT0912U     */
                      recData = QDAMGetrecData_V2( pRecord, i, pBT->usVersion );
                      if ( *pulUserLen == 0 || ! pchUserData )
                      {
                         *pulUserLen = recData.ulLen;
                      }
                      else
                      {
                         sRc =  QDAMGetszData_V2( pBTIda, recData, pchUserData, pulUserLen, DATA_NODE );
                      } /* endif */
                   }
                   else
                   {
                     sRc = BTREE_CORRUPTED;
                   } /* endif */
                }
                else
                {
                   sRc = BTREE_NOT_FOUND;
                   *pulUserLen = 0;
                   // set new current position
                   pBTIda->sCurrentIndex = sNearKey;
                   pBTIda->usCurrentRecord = RECORDNUM( pRecord );
                } /* endif */
             } /* endif */
             ASDLOG();
           } /* endif */

         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (sRetries > 0));
       *pulLength = ulLength;
     } /* endif */
   } /* endif */


  if ( sRc && (sRc != BTREE_NOT_FOUND))
  {
    ERREVENT2( QDAMDICTEQUIVLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return ( sRc );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictFirstLocal   Get the first entry back
//------------------------------------------------------------------------------
// Function call:     QDAMDictFirstLocal(PBTREE,PCHAR,PUSHORT,PCHAR,PUSHORT);
//------------------------------------------------------------------------------
// Description:       Locate the first entry and pass back the
//                    associated information into the user provided
//                    buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     if tree is corrupted
//                      set RC = BTREE_CORRUPTED
//                    else
//                      locate first entry
//                    endif
//                    if ok
//                      set ptr to key data, length of key data
//                    endif
//                    if ok
//                      set ptr to user data, length of user data
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictFirstLocal
(
   PBTREE     pBTIda,
   PCHAR_W    pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
)
{
   SHORT     sRc = 0;                  // return code
   RECPARAM  recBTree;                 // record parameter for index data
   RECPARAM  recKey;                   //      ...   for key value
   RECPARAM  recData;                  //      ...   for user value
   PBTREEGLOB    pBT = pBTIda->pBTree;
   ULONG ulKeyLen;

   memset(&recData, 0, sizeof(recData));
   memset(&recKey, 0, sizeof(recKey));
   if ( pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */

   if ( !sRc )
   {
       SHORT sRetries = MAX_RETRY_COUNT;

       do
       {
          ulKeyLen = *pulKeyLen;

          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
          if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
          {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

           if ( !sRc )
           {
              if ( pBT->bRecSizeVersion == BTREE_V3 )
              {
                 sRc = QDAMFirst_V3( pBTIda, &recBTree,&recKey, &recData);
              }
              else
              {
                 sRc = QDAMFirst_V2( pBTIda, &recBTree,&recKey, &recData);
              } /* endif */
           } /* endif */

           if ( !sRc )
           {
              if ( pBT->bRecSizeVersion == BTREE_V3 )
              {
                sRc = QDAMGetszKeyParam_V3( pBTIda, recKey, pKeyData, &ulKeyLen );
              }
              else
              {
                sRc = QDAMGetszKeyParam_V2( pBTIda, recKey, pKeyData, &ulKeyLen );
              } /* endif */
              if ( !sRc && ulKeyLen )
              {
                if ( pBT->fTransMem )
                {
                  ULONG ul = 0l;
                  memcpy( pBTIda->chHeadTerm, &ul, sizeof(ULONG) );
                }
                else
                {
                  pBTIda->chHeadTerm[0] = EOS;
                } /* endif */
              } /* endif */
           } /* endif */

           if ( !sRc )
           {
             if ( pBT->bRecSizeVersion == BTREE_V3 )
             {
                sRc =  QDAMGetszData_V3( pBTIda, recData, pUserData, pulUserLen, DATA_NODE );
             }
             else
             {
                sRc =  QDAMGetszData_V2( pBTIda, recData, pUserData, pulUserLen, DATA_NODE );
             } /* endif */
           } /* endif */

         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (sRetries > 0));
       *pulKeyLen = ulKeyLen;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDICTFIRSTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
   return ( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictNextLocal   Get the next entry back
//------------------------------------------------------------------------------
// Function call:     QDAMDictNextLocal( PBTREE, PCHAR,PUSHORT, PCHAR,PUSHORT);
//
//------------------------------------------------------------------------------
// Description:       Locate the next entry (by collating sequence) and
//                    pass back the associated information into
//                    the user provided buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted then
//                      set Rc to BTREE_CORRUPTED
//                    else
//                      call internal function for getting next tree element
//                      and set Rc accordingly
//                    endif
//                    if okay then
//                      Get the key data from the returned record key parameter
//                    endif
//                    if okay then
//                      Get the data from the returned record data parameter
//                    endif
//                    return Rc
//------------------------------------------------------------------------------

SHORT QDAMDictNextLocal
(
   PBTREE     pBTIda,
   PCHAR_W    pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
)
{
   SHORT  sRc = 0;                     // return code
   RECPARAM  recBTree;                 // record parameter for index data
   RECPARAM  recKey;                   //      ...   for key value
   RECPARAM  recData;                  //      ...   for user value
   PBTREEGLOB    pBT = pBTIda->pBTree;
   ULONG      ulKeyLen = *pulKeyLen;

   memset(&recData, 0, sizeof(recData));
   if ( pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */

   if ( !sRc )
   {
       SHORT sRetries = MAX_RETRY_COUNT;
       do
       {
           ulKeyLen = *pulKeyLen;

           /*******************************************************************/
           /* For shared databases: discard all in-memory pages if database   */
           /* has been changed since last access                              */
           /*******************************************************************/
           if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
           {
             sRc = QDAMCheckForUpdates( pBTIda );
           } /* endif */

           if ( pBT->bRecSizeVersion == BTREE_V3 )
           {
              if ( !sRc )
              {
                if ( !pBTIda->usCurrentRecord )
                {
                  /****************************************************************/
                  /* try a substring call first to check if an invalidate might   */
                  /* be the reason                                                */
                  /****************************************************************/
                  if ( pBTIda->chHeadTerm[0] && (pBTIda->sCurrentIndex == RESET_VALUE) )
                  {
                    ULONG ulKeyLen = *pulKeyLen;
                    ULONG ulUserLen = *pulUserLen;
                    sRc = QDAMDictSubStrLocal( pBTIda, pBTIda->chHeadTerm,
                                                (PBYTE) pKeyData, &ulKeyLen,
                                                pUserData, &ulUserLen );
                  } /* endif */
                } /* endif */
                while ( !sRc )
                {
                  sRc = QDAMNext_V3( pBTIda, &recBTree,&recKey, &recData);
                  if ( !sRc )
                  {
                      ULONG ulLocalKeyLen = ulKeyLen;
                      sRc = QDAMGetszKeyParam_V3( pBTIda, recKey, pKeyData, &ulLocalKeyLen );
                      if ( ulLocalKeyLen )
                      {
                        if ( (UTF16strcmp( pKeyData, szLOCKEDTERMSKEYW ) != 0 ) ||
                              ( strcmp((PSZ)pKeyData, szLOCKEDTERMSKEY) != 0 ) )
                        {
                          /********************************************************/
                          /* found a valid record ...                             */
                          /********************************************************/
                          ulKeyLen = ulLocalKeyLen;
                          memcpy( (PBYTE)pBTIda->chHeadTerm, (PBYTE)pKeyData, ulKeyLen );
                          break;                   // exit while loop
                        } /* endif */
                      }
                      else
                      {
                          break;                   // exit while loop
                      } /* endif */
                  } /* endif */
                } /* endif */
              } /* endif */


              if ( !sRc )
              {
                  sRc =  QDAMGetszData_V3( pBTIda, recData, pUserData, pulUserLen, DATA_NODE );
              } /* endif */
           }
           else
           {
              if ( !sRc )
              {
                if ( !pBTIda->usCurrentRecord )
                {
                  /****************************************************************/
                  /* try a substring call first to check if an invalidate might   */
                  /* be the reason                                                */
                  /****************************************************************/
                  if ( pBTIda->chHeadTerm[0] && (pBTIda->sCurrentIndex == RESET_VALUE) )
                  {
                    ULONG ulKeyLen = *pulKeyLen;
                    ULONG ulUserLen = *pulUserLen;
                    sRc = QDAMDictSubStrLocal( pBTIda, pBTIda->chHeadTerm,
                                                (PBYTE) pKeyData, &ulKeyLen,
                                                pUserData, &ulUserLen );
                  } /* endif */
                } /* endif */
                while ( !sRc )
                {
                  sRc = QDAMNext_V2( pBTIda, &recBTree,&recKey, &recData);
                  if ( !sRc )
                  {
                      ULONG ulLocalKeyLen = ulKeyLen;
                      sRc = QDAMGetszKeyParam_V2( pBTIda, recKey, pKeyData, &ulLocalKeyLen );
                      if ( ulLocalKeyLen )
                      {
                        if ( (UTF16strcmp( pKeyData, szLOCKEDTERMSKEYW ) != 0 ) ||
                              ( strcmp((PSZ)pKeyData, szLOCKEDTERMSKEY) != 0 ) )
                        {
                          /********************************************************/
                          /* found a valid record ...                             */
                          /********************************************************/
                          ulKeyLen = ulLocalKeyLen;
                          memcpy( (PBYTE)pBTIda->chHeadTerm, (PBYTE)pKeyData, ulKeyLen );
                          break;                   // exit while loop
                        } /* endif */
                      }
                      else
                      {
                          break;                   // exit while loop
                      } /* endif */
                  } /* endif */
                } /* endif */
              } /* endif */


              if ( !sRc )
              {
                  sRc =  QDAMGetszData_V2( pBTIda, recData, pUserData, pulUserLen, DATA_NODE );
              } /* endif */
           } /* endif */


         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (sRetries > 0));
   } /* endif */

   *pulKeyLen = ulKeyLen;

   if ( sRc )
   {
     ERREVENT2( QDAMDICTNEXTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return ( sRc );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictPrevLocal   Get the prev entry back
//------------------------------------------------------------------------------
// Function call:     QDAMDictPrevLocal( PBTREE,PCHAR,PUSHORT,PCHAR,PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Locate the previous entry (by collating sequence)
//                    and pass back the associated
//                    information into the user provided buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted then
//                      set Rc to BTREE_CORRUPTED
//                    else
//                      call internal function for getting prev.tree element
//                      and set Rc accordingly
//                    endif
//                    if okay then
//                      Get the key data from the returned record key parameter
//                    endif
//                    if okay then
//                      Get the data from the returned record data parameter
//                    endif
//                    return Rc
//
//------------------------------------------------------------------------------

SHORT QDAMDictPrevLocal
(
   PBTREE     pBTIda,
   PCHAR_W    pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
)
{
   SHORT  sRc = 0;                     // return code
   RECPARAM  recBTree;                 // record parameter for index data
   RECPARAM  recKey;                   //      ...   for key value
   RECPARAM  recData;                  //      ...   for user value
   PBTREEGLOB    pBT = pBTIda->pBTree;
   ULONG     ulKeyLen;

   memset(&recData, 0, sizeof(recData));
   if ( pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */

   if ( !sRc )
   {
       SHORT sRetries = MAX_RETRY_COUNT;
       ulKeyLen = *pulKeyLen;

       do
       {
           /*******************************************************************/
           /* For shared databases: discard all in-memory pages if database   */
           /* has been changed since last access                              */
           /*******************************************************************/
           if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
           {
             sRc = QDAMCheckForUpdates( pBTIda );
           } /* endif */

           if ( pBT->bRecSizeVersion == BTREE_V3 )
           {
              if ( !sRc )
              {
                if ( !pBTIda->usCurrentRecord && (pBTIda->sCurrentIndex == RESET_VALUE) )
                {
                  /****************************************************************/
                  /* try a substring call first to check if an invalidate might   */
                  /* be the reason                                                */
                  /****************************************************************/
                  ULONG ulKeyLen = *pulKeyLen;
                  ULONG ulUserLen = *pulUserLen;
                  sRc = QDAMDictSubStrLocal( pBTIda, pBTIda->chHeadTerm,
                                              (PBYTE) pKeyData, &ulKeyLen,
                                              pUserData, &ulUserLen );
                } /* endif */
                while ( !sRc )
                {
                  sRc = QDAMPrev_V3( pBTIda, &recBTree,&recKey, &recData);
                  if ( !sRc )
                  {
                      ULONG ulLocalKeyLen = ulKeyLen;
                      sRc = QDAMGetszKeyParam_V3( pBTIda, recKey, pKeyData, &ulLocalKeyLen );
                      if ( ulLocalKeyLen )
                      {
                        if ( UTF16strcmp( pKeyData, szLOCKEDTERMSKEYW ) != 0 )
                        {
                          /********************************************************/
                          /* found a valid record ...                             */
                          /********************************************************/
                          ulKeyLen = ulLocalKeyLen;
                          memcpy( (PBYTE)pBTIda->chHeadTerm, (PBYTE)pKeyData, ulKeyLen );
                          break;                   // exit while loop
                        } /* endif */
                      }
                      else
                      {
                        break;                     // exit while loop
                      } /* endif */
                  } /* endif */
                } /* endwhile */
              } /* endif */


              if ( !sRc )
              {
                  sRc =  QDAMGetszData_V3( pBTIda, recData, pUserData, pulUserLen, DATA_NODE );
              } /* endif */
           }
           else
           {
              if ( !sRc )
              {
                if ( !pBTIda->usCurrentRecord && (pBTIda->sCurrentIndex == RESET_VALUE) )
                {
                  /****************************************************************/
                  /* try a substring call first to check if an invalidate might   */
                  /* be the reason                                                */
                  /****************************************************************/
                  ULONG ulKeyLen = *pulKeyLen;
                  ULONG ulUserLen = *pulUserLen;
                  sRc = QDAMDictSubStrLocal( pBTIda, pBTIda->chHeadTerm,
                                              (PBYTE) pKeyData, &ulKeyLen,
                                              pUserData, &ulUserLen );
                } /* endif */
                while ( !sRc )
                {
                  sRc = QDAMPrev_V2( pBTIda, &recBTree,&recKey, &recData);
                  if ( !sRc )
                  {
                      ULONG ulLocalKeyLen = ulKeyLen;
                      sRc = QDAMGetszKeyParam_V2( pBTIda, recKey, pKeyData, &ulLocalKeyLen );
                      if ( ulLocalKeyLen )
                      {
                        if ( UTF16strcmp( pKeyData, szLOCKEDTERMSKEYW ) != 0 )
                        {
                          /********************************************************/
                          /* found a valid record ...                             */
                          /********************************************************/
                          ulKeyLen = ulLocalKeyLen;
                          memcpy( (PBYTE)pBTIda->chHeadTerm, (PBYTE)pKeyData, ulKeyLen );
                          break;                   // exit while loop
                        } /* endif */
                      }
                      else
                      {
                        break;                     // exit while loop
                      } /* endif */
                  } /* endif */
                } /* endwhile */
              } /* endif */


              if ( !sRc )
              {
                  sRc =  QDAMGetszData_V2( pBTIda, recData, pUserData, pulUserLen, DATA_NODE );
              } /* endif */
            } /* endif */

            if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
            {
              UtlWait( MAX_WAIT_TIME );
              sRetries--;
            } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (sRetries > 0));
       *pulKeyLen = ulKeyLen;
   } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMDICTPREVLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

   return ( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCurrentLocal  Get current entry back
//------------------------------------------------------------------------------
// Function call:     QDAMDictCurrentLocal(PBTREE,PCHAR,PUSHORT,PCHAR,PUSHORT);
//
//------------------------------------------------------------------------------
// Description:       Locate the current entry  and pass back the
//                    associated information into the user provided buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     invalid pointer passed
//                    BTREE_EOF_REACHED eof or start reached
//                    BTREE_CORRUPTED   dictionary corrupted
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted then
//                      set Rc to BTREE_CORRUPTED
//                    elsif ! usCurrentRecord set than
//                      set Rc to BTREE_EOF_REACHED
//                    else
//                      read current record and set Rc accordingly
//                      if okay then
//                        get parameter data for key and data
//                      endif
//                    endif
//                    if okay then
//                      Get the key data from the returned record key parameter
//                    endif
//                    if okay then
//                      Get the data from the returned record data parameter
//                    endif
//                    return Rc
//------------------------------------------------------------------------------

SHORT QDAMDictCurrentLocal
(
   PBTREE     pBTIda,
   PBYTE      pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
)
{
   SHORT  sRc=0;                       // return code
   RECPARAM  recKey;                   //      ...   for key value
   RECPARAM  recData;                  //      ...   for user value
   PBTREEGLOB    pBT = pBTIda->pBTree;
   ULONG     ulKeyLen;

   memset(&recData, 0, sizeof(recData));
   memset(&recKey, 0, sizeof(recKey));

   if ( pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */

   if ( !sRc )
   {
       SHORT sRetries = MAX_RETRY_COUNT;
       ulKeyLen = *pulKeyLen;

       do
       {
           /*******************************************************************/
           /* For shared databases: discard all in-memory pages if database   */
           /* has been changed since last access                              */
           /*******************************************************************/
           if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
           {
             sRc = QDAMCheckForUpdates( pBTIda );
           } /* endif */

           if ( !sRc )
           {
              if ( pBT->bRecSizeVersion == BTREE_V3 )
              {
                PBTREEBUFFER_V3  pRecord;              // pointer to record

                if ( !pBTIda->usCurrentRecord )
                {
                    /****************************************************************/
                    /* try a substring call first to check if an invalidate might   */
                    /* be the reason                                                */
                    /****************************************************************/
                    ULONG ulKeyLen = *pulKeyLen;
                    ULONG ulUserLen = *pulUserLen;
                    sRc = QDAMDictSubStrLocal( pBTIda, pBTIda->chHeadTerm, pKeyData, &ulKeyLen, pUserData, &ulUserLen );

                } /* endif */

                if ( !sRc && !pBTIda->usCurrentRecord  )
                {
                    sRc = BTREE_EOF_REACHED;
                }
                else
                {
                    sRc = QDAMReadRecord_V3( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
                    if ( !sRc )
                    {
                      recKey = QDAMGetrecKey_V3( pRecord, pBTIda->sCurrentIndex  );
                      recData = QDAMGetrecData_V3( pRecord, pBTIda->sCurrentIndex, pBT->usVersion  );
                    } /* endif */
                } /* endif */

                if ( !sRc )
                {
                    sRc = QDAMGetszKeyParam_V3( pBTIda, recKey, (PCHAR_W)pKeyData, &ulKeyLen );
                    if ( ulKeyLen )
                    {
                      memcpy( (PBYTE)pBTIda->chHeadTerm, pKeyData, ulKeyLen );
                    } /* endif */
                } /* endif */

                if ( !sRc )
                {
                    sRc =  QDAMGetszData_V3( pBTIda, recData, pUserData, pulUserLen, DATA_NODE );
                } /* endif */
              }
              else
              {
                PBTREEBUFFER_V2  pRecord;              // pointer to record

                if ( !pBTIda->usCurrentRecord )
                {
                    /****************************************************************/
                    /* try a substring call first to check if an invalidate might   */
                    /* be the reason                                                */
                    /****************************************************************/
                    ULONG ulKeyLen = *pulKeyLen;
                    ULONG ulUserLen = *pulUserLen;
                    sRc = QDAMDictSubStrLocal( pBTIda, pBTIda->chHeadTerm, pKeyData, &ulKeyLen, pUserData, &ulUserLen );

                } /* endif */

                if ( !sRc && !pBTIda->usCurrentRecord  )
                {
                    sRc = BTREE_EOF_REACHED;
                }
                else
                {
                    sRc = QDAMReadRecord_V2( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
                    if ( !sRc )
                    {
                      recKey = QDAMGetrecKey_V2( pRecord, pBTIda->sCurrentIndex  );
                      recData = QDAMGetrecData_V2( pRecord, pBTIda->sCurrentIndex, pBT->usVersion  );
                    } /* endif */
                } /* endif */

                if ( !sRc )
                {
                    sRc = QDAMGetszKeyParam_V2( pBTIda, recKey, (PCHAR_W)pKeyData, &ulKeyLen );
                    if ( ulKeyLen )
                    {
                      memcpy( (PBYTE)pBTIda->chHeadTerm, pKeyData, ulKeyLen );
                    } /* endif */
                } /* endif */

                if ( !sRc )
                {
                    sRc =  QDAMGetszData_V2( pBTIda, recData, pUserData, pulUserLen, DATA_NODE );
                } /* endif */
              } /* endif */

            if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
            {
              UtlWait( MAX_WAIT_TIME );
              sRetries--;
            } /* endif */
        } /* endif */
       } while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) && (sRetries > 0));
       *pulKeyLen = ulKeyLen;
   } /* endif */


  if ( sRc )
  {
    ERREVENT2( QDAMDICTCURRENTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

   return ( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictNumEntriesLocal  number of entries in dict
//------------------------------------------------------------------------------
// Function call:     QDAMDictNumEntriesLocal( PBTREE, PULONG );
//
//------------------------------------------------------------------------------
// Description:       Return the number of entries in the dictionary
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PULONG                 pointer to number of entries
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//------------------------------------------------------------------------------
// Function flow:     if dictionary is corrupted then
//                      set Rc to BTREE_CORRUPTED
//                    else
//                      position at first leaf; set Rc
//                      if okay then
//                        loop through all record and cumulate numbers;set Rc
//                      endif
//                      if okay then
//                        set return number
//                      endif
//                    endif
//                    return Rc
//------------------------------------------------------------------------------

SHORT QDAMDictNumEntriesLocal
(
   PBTREE pBTIda,                   // pointer to generic structure
   PULONG pulNum                    // number of entries in tree
)
{
   SHORT sRc = 0;                   // return code
   ULONG  ulNum = 0;                // number of entries in btree
   USHORT  usCurrentRecord;         // temp record number
   SHORT   sCurrentIndex;           // temp index number
   PBTREEGLOB    pBT = pBTIda->pBTree;

   /*****************************************************************/
   /* store old settings                                            */
   /*****************************************************************/
   usCurrentRecord = pBTIda->usCurrentRecord;
   sCurrentIndex = pBTIda->sCurrentIndex;

   if ( pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */

   /*******************************************************************/
   /* For shared databases: discard all in-memory pages if database   */
   /* has been changed since last access                              */
   /*******************************************************************/
   if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
   {
     sRc = QDAMCheckForUpdates( pBTIda );
   } /* endif */

   if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3 pRecord = NULL;     // pointer to record buffer
      if ( !sRc )
      {
          // get first record
          pBTIda->usCurrentRecord = pBT->usFirstLeaf;
          if ( pBT->usFirstLeaf  == 0 )
          {
            sRc = BTREE_NOT_FOUND;
          }
          else
          {
            sRc = QDAMReadRecord_V3( pBTIda, pBT->usFirstLeaf, &pRecord, FALSE );
            // if okay add number of entries
            if ( !sRc )
            {
              ulNum = (ULONG) OCCUPIED( pRecord );
            } /* endif */
          } /* endif */
      } /* endif */

      // if okay loop tru all other entries leaves

      while ( !sRc && NEXT(pRecord))
      {
          sRc = QDAMReadRecord_V3( pBTIda, NEXT( pRecord ), &pRecord, FALSE  );
          if ( !sRc )
          {
            ulNum += (ULONG) OCCUPIED( pRecord );
          } /* endif */
      } /* endwhile */
   }
   else
   {
      PBTREEBUFFER_V2 pRecord = NULL;     // pointer to record buffer
      if ( !sRc )
      {
          // get first record
          pBTIda->usCurrentRecord = pBT->usFirstLeaf;
          if ( pBT->usFirstLeaf  == 0 )
          {
            sRc = BTREE_NOT_FOUND;
          }
          else
          {
            sRc = QDAMReadRecord_V2( pBTIda, pBT->usFirstLeaf, &pRecord, FALSE );
            // if okay add number of entries
            if ( !sRc )
            {
              ulNum = (ULONG) OCCUPIED( pRecord );
            } /* endif */
          } /* endif */
      } /* endif */

      // if okay loop tru all other entries leaves

      while ( !sRc && NEXT(pRecord))
      {
          sRc = QDAMReadRecord_V2( pBTIda, NEXT( pRecord ), &pRecord, FALSE  );
          if ( !sRc )
          {
            ulNum += (ULONG) OCCUPIED( pRecord );
          } /* endif */
      } /* endwhile */
   } /* endif */

   // set number in provided long
   if ( !sRc )
   {
      *pulNum = ulNum;
   } /* endif */

   /*****************************************************************/
   /* restore old settings                                          */
   /*****************************************************************/
   pBTIda->usCurrentRecord = usCurrentRecord;
   pBTIda->sCurrentIndex = sCurrentIndex;

   return( sRc );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictNumber  Position at passed entry number
//------------------------------------------------------------------------------
// Function call:     QDAMDictNumber( PBTREE, ULONG, PUCHAR, PUSHORT, PUCHAR,
//                                    PUSHORT );
//------------------------------------------------------------------------------
// Description:       Try to position at the passed entry number
//                    and fill the provided area with the data.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    ULONG                  term number
//                    PUCHAR                 pointer to space for term
//                    PUSHORT                length of key
//                    PUCHAR                 pointer to data
//                    PUSHORT                length of data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//
//------------------------------------------------------------------------------
// Function flow:
//
//------------------------------------------------------------------------------
SHORT QDAMDictNumber
(
    PBTREE   pBTIda,                 // pointer to generic structure
    ULONG    ulTermNum,              // term number
    PCHAR_W  pKey,                   // pointer to space for term
    PULONG   pulKeyLen,              // length of key
    PBYTE    pData,                  // pointer to data
    PULONG   pulLen                  // length of data
)
{
   SHORT sRc = 0;                   // return code
   RECPARAM  recData;               // data position
   RECPARAM  recKey;                // key position
   BOOL      fFound = FALSE;        // not found yet
   PBTREEGLOB    pBT = pBTIda->pBTree;

   if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3 pRecord = NULL;     // pointer to record buffer

      // get first record
      if ( pBT )
      {
          if ( pBT->fCorrupted )
          {
            sRc = BTREE_CORRUPTED;
          } /* endif */

          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
          if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
          {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

          if ( !sRc )
          {
            ulTermNum --;              // we are 0 based
            pBTIda->usCurrentRecord = pBT->usFirstLeaf;
            if ( pBT->usFirstLeaf  == 0 )
            {
              sRc = BTREE_NOT_FOUND;
            }
            else
            {
              sRc = QDAMReadRecord_V3( pBTIda, pBT->usFirstLeaf, &pRecord, FALSE );
              // if okay check if number already found
              if ( !sRc )
              {
                  if ( ulTermNum < (ULONG) OCCUPIED(pRecord) )
                  {
                    fFound = TRUE;
                  }
                  else
                  {
                    ulTermNum -= (ULONG) OCCUPIED(pRecord);
                  } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */
      }
      else
      {
          sRc = BTREE_INVALID;
      } /* endif */

      // if okay loop tru all other entries leaves

      while ( !sRc && NEXT(pRecord) && !fFound )
      {
          sRc = QDAMReadRecord_V3( pBTIda, NEXT( pRecord ), &pRecord, FALSE  );
          if ( !sRc )
          {
            if ( ulTermNum < (ULONG) OCCUPIED(pRecord) )
            {
                fFound = TRUE;
            }
            else
            {
                ulTermNum -= (ULONG) OCCUPIED(pRecord);
            } /* endif */
          } /* endif */
      } /* endwhile */

      // set number in provided long
      if ( !sRc )
      {
          if ( !fFound  )
          {
            sRc = BTREE_NUMBER_RANGE;
          }
          else
          {
            // set position
            pBTIda->sCurrentIndex = (SHORT) ulTermNum;
            pBTIda->usCurrentRecord = RECORDNUM( pRecord );

            recKey = QDAMGetrecKey_V3( pRecord, (SHORT) ulTermNum);

            recData = QDAMGetrecData_V3( pRecord, (SHORT) ulTermNum, pBT->usVersion );
            // get term
            if ( !sRc )
            {
                sRc = QDAMGetszKeyParam_V3( pBTIda, recKey, pKey, pulKeyLen );
            } /* endif */

            // get data
            if ( !sRc )
            {
                sRc =  QDAMGetszData_V3( pBTIda, recData, pData, pulLen, DATA_NODE );
            } /* endif */
          } /* endif */
      } /* endif */
   }
   else
   {
      PBTREEBUFFER_V2 pRecord = NULL;     // pointer to record buffer

      // get first record
      if ( pBT )
      {
          if ( pBT->fCorrupted )
          {
            sRc = BTREE_CORRUPTED;
          } /* endif */

          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
          if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
          {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

          if ( !sRc )
          {
            ulTermNum --;              // we are 0 based
            pBTIda->usCurrentRecord = pBT->usFirstLeaf;
            if ( pBT->usFirstLeaf  == 0 )
            {
              sRc = BTREE_NOT_FOUND;
            }
            else
            {
              sRc = QDAMReadRecord_V2( pBTIda, pBT->usFirstLeaf, &pRecord, FALSE );
              // if okay check if number already found
              if ( !sRc )
              {
                  if ( ulTermNum < (ULONG) OCCUPIED(pRecord) )
                  {
                    fFound = TRUE;
                  }
                  else
                  {
                    ulTermNum -= (ULONG) OCCUPIED(pRecord);
                  } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */
      }
      else
      {
          sRc = BTREE_INVALID;
      } /* endif */

      // if okay loop tru all other entries leaves

      while ( !sRc && NEXT(pRecord) && !fFound )
      {
          sRc = QDAMReadRecord_V2( pBTIda, NEXT( pRecord ), &pRecord, FALSE  );
          if ( !sRc )
          {
            if ( ulTermNum < (ULONG) OCCUPIED(pRecord) )
            {
                fFound = TRUE;
            }
            else
            {
                ulTermNum -= (ULONG) OCCUPIED(pRecord);
            } /* endif */
          } /* endif */
      } /* endwhile */

      // set number in provided long
      if ( !sRc )
      {
          if ( !fFound  )
          {
            sRc = BTREE_NUMBER_RANGE;
          }
          else
          {
            // set position
            pBTIda->sCurrentIndex = (SHORT) ulTermNum;
            pBTIda->usCurrentRecord = RECORDNUM( pRecord );

            recKey = QDAMGetrecKey_V2( pRecord, (SHORT) ulTermNum);

            recData = QDAMGetrecData_V2( pRecord, (SHORT) ulTermNum, pBT->usVersion );
            // get term
            if ( !sRc )
            {
                sRc = QDAMGetszKeyParam_V2( pBTIda, recKey, pKey, pulKeyLen );
            } /* endif */

            // get data
            if ( !sRc )
            {
                sRc =  QDAMGetszData_V2( pBTIda, recData, pData, pulLen, DATA_NODE );
            } /* endif */
          } /* endif */
      } /* endif */
   } /* endif */
   return( sRc );
}





//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictNextWildLocal   Get the next matching entry back
//------------------------------------------------------------------------------
// Description:       Locate the next entry matching the given pattern
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PSZ                    pointer to search pattern
//                    BOOL                   compound search flag
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//------------------------------------------------------------------------------
SHORT QDAMDictNextWildLocal
(
   PBTREE     pBTIda,
   PSZ_W      pPattern,                // search pattern or compound
   BOOL       fCompound,               // compound search flag
   PBYTE      pKeyData,                //   pointer to space for key data
   PULONG     pulKeyLen                //   length of space for key data
)
{
   SHORT  sRc = 0;                     // return code
   RECPARAM  recBTree;                 // record parameter for index data
   RECPARAM  recKey;                   //      ...   for key value
   PBTREEGLOB    pBT = pBTIda->pBTree;
   ULONG ulKeyLen = *pulKeyLen;

   memset (&recKey, 0, sizeof(recKey));

   if ( pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */

   if ( !sRc )
   {
     if ( pBT->bRecSizeVersion == BTREE_V3 )
     {
       PBTREEBUFFER_V3 pRecord;
       SHORT sRetries = MAX_RETRY_COUNT;
       do
       {
           ulKeyLen = *pulKeyLen;

           /*******************************************************************/
           /* For shared databases: discard all in-memory pages if database   */
           /* has been changed since last access                              */
           /*******************************************************************/
           if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
           {
             sRc = QDAMCheckForUpdates( pBTIda );
           } /* endif */

           if ( !sRc )
           {
             if ( !pBTIda->usCurrentRecord )
             {
               /****************************************************************/
               /* try a substring call first to check if an invalidate might   */
               /* be the reason                                                */
               /****************************************************************/
               if ( pBTIda->chHeadTerm[0] && (pBTIda->sCurrentIndex == RESET_VALUE) )
               {
                 ULONG ulKeyLen = *pulKeyLen;
                 ULONG ulUserLen = 0;
                 sRc = QDAMDictSubStrLocal( pBTIda, pBTIda->chHeadTerm, pKeyData, &ulKeyLen, NULL, &ulUserLen );
               } /* endif */
             } /* endif */

             while ( !sRc )
             {
               // Step to next index in the chain and read the record                     */
               // if usCurrentRecord = 0 we are at EOF
               if ( !pBTIda->usCurrentRecord )
               {
                  sRc = QDAMFirstEntry_V3( pBTIda, &pRecord );
               }
               else
               {
                  pBTIda->sCurrentIndex++;
                  sRc = QDAMReadRecord_V3( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
               } /* endif */

               if ( !sRc )
               {
                 sRc = QDAMValidateIndex_V3( pBTIda, &pRecord );
                 if ( !sRc )
                 {
                     recBTree.usOffset = pBTIda->sCurrentIndex;
                     recBTree.usNum    = pBTIda->usCurrentRecord;
                     recKey = QDAMGetrecKey_V3( pRecord, pBTIda->sCurrentIndex  );
                 } /* endif */
               } /* endif */

               if ( !sRc )
               {
                  ULONG ulLocalKeyLen = *pulKeyLen;
                  sRc = QDAMGetszKeyParam_V3( pBTIda, recKey, (PCHAR_W)pKeyData, &ulLocalKeyLen );
                  if ( !sRc && ulLocalKeyLen )
                  {
                    /********************************************************/
                    /* check for a matching key                             */
                    /********************************************************/
                    BOOL fMatch;

                    if ( fCompound )
                    {
                      fMatch = QDAMMatchCompound((PSZ_W) pKeyData, pPattern );
                    }
                    else
                    {
                      if ( !UtlMatchStringsW( (PSZ_W)pKeyData, pPattern, &fMatch ) )
                      {
                        sRc = BTREE_NO_ROOM;
                      } /* endif */
                    } /* endif */

                    if ( !sRc && fMatch )
                    {
                      ulKeyLen = ulLocalKeyLen;
                      memcpy( (PBYTE) pBTIda->chHeadTerm, pKeyData, ulKeyLen );
                      break;                   // exit while loop
                    } /* endif */
                  }
                  else
                  {
                    break;                   // exit while loop
                  } /* endif */
               } /* endif */
             } /* endwhile */
           } /* endif */

         if ( sRc == BTREE_IN_USE )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( (sRc == BTREE_IN_USE) && (sRetries > 0));
     }
     else
     {
       PBTREEBUFFER_V2 pRecord;
       SHORT sRetries = MAX_RETRY_COUNT;
       do
       {
           ulKeyLen = *pulKeyLen;

           /*******************************************************************/
           /* For shared databases: discard all in-memory pages if database   */
           /* has been changed since last access                              */
           /*******************************************************************/
           if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
           {
             sRc = QDAMCheckForUpdates( pBTIda );
           } /* endif */

           if ( !sRc )
           {
             if ( !pBTIda->usCurrentRecord )
             {
               /****************************************************************/
               /* try a substring call first to check if an invalidate might   */
               /* be the reason                                                */
               /****************************************************************/
               if ( pBTIda->chHeadTerm[0] && (pBTIda->sCurrentIndex == RESET_VALUE) )
               {
                 ULONG ulKeyLen = *pulKeyLen;
                 ULONG ulUserLen = 0;
                 sRc = QDAMDictSubStrLocal( pBTIda, pBTIda->chHeadTerm, pKeyData, &ulKeyLen, NULL, &ulUserLen );
               } /* endif */
             } /* endif */

             while ( !sRc )
             {
               // Step to next index in the chain and read the record                     */
               // if usCurrentRecord = 0 we are at EOF
               if ( !pBTIda->usCurrentRecord )
               {
                  sRc = QDAMFirstEntry_V2( pBTIda, &pRecord );
               }
               else
               {
                  pBTIda->sCurrentIndex++;
                  sRc = QDAMReadRecord_V2( pBTIda, pBTIda->usCurrentRecord, &pRecord, FALSE  );
               } /* endif */

               if ( !sRc )
               {
                 sRc = QDAMValidateIndex_V2( pBTIda, &pRecord );
                 if ( !sRc )
                 {
                     recBTree.usOffset = pBTIda->sCurrentIndex;
                     recBTree.usNum    = pBTIda->usCurrentRecord;
                     recKey = QDAMGetrecKey_V2( pRecord, pBTIda->sCurrentIndex  );
                 } /* endif */
               } /* endif */

               if ( !sRc )
               {
                  ULONG ulLocalKeyLen = *pulKeyLen;
                  sRc = QDAMGetszKeyParam_V2( pBTIda, recKey, (PCHAR_W)pKeyData, &ulLocalKeyLen );
                  if ( !sRc && ulLocalKeyLen )
                  {
                    /********************************************************/
                    /* check for a matching key                             */
                    /********************************************************/
                    BOOL fMatch;

                    if ( fCompound )
                    {
                      fMatch = QDAMMatchCompound((PSZ_W) pKeyData, pPattern );
                    }
                    else
                    {
                      if ( !UtlMatchStringsW( (PSZ_W)pKeyData, pPattern, &fMatch ) )
                      {
                        sRc = BTREE_NO_ROOM;
                      } /* endif */
                    } /* endif */

                    if ( !sRc && fMatch )
                    {
                      ulKeyLen = ulLocalKeyLen;
                      memcpy( (PBYTE) pBTIda->chHeadTerm, pKeyData, ulKeyLen );
                      break;                   // exit while loop
                    } /* endif */
                  }
                  else
                  {
                    break;                   // exit while loop
                  } /* endif */
               } /* endif */
             } /* endwhile */
           } /* endif */

         if ( sRc == BTREE_IN_USE )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( (sRc == BTREE_IN_USE) && (sRetries > 0));
     } /* endif */
   } /* endif */

   *pulKeyLen = ulKeyLen;

   return ( sRc );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMKeyCompare      Generic compare function              |
//+----------------------------------------------------------------------------+
//|Function call:     NTMKeyCompare( PBTREE,PULONG,PULONG);                    |
//+----------------------------------------------------------------------------+
//|Description:       This is the generic compare function used                |
//|                   for comparision                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to tree structure         |
//|                   PULONG                 first key                         |
//|                   PULONG                 second key                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0   keys are equal                                       |
//|                   <> keys are unequal                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     return (SHORT) (*pulKey1 - *pulKey2)                     |
//+----------------------------------------------------------------------------+

SHORT NTMKeyCompare
(
    PVOID  pBTIda,                     // pointer to tree structure
    PVOID  pulKey1,                    // pointer to first key
    PVOID  pulKey2                     // pointer to second key
)
{
  ULONG  ulKey1 = *((PULONG)pulKey1);
  ULONG  ulKey2 = *((PULONG)pulKey2);
  SHORT  sRc;
  pBTIda;                              // avoid compiler warnings

  if (ulKey1 > ulKey2 )
  {
    sRc = 1;
  }
  else if (ulKey1 < ulKey2 )
  {
    sRc = -1;
  }
  else
  {
    sRc = 0;
  } /* endif */
  return sRc;
}

