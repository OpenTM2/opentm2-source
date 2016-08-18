/*! \file
	Description: Program for ITM visualization - LF's insert and delete

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

  #define INCL_EQF_MORPH
  #define INCL_EQF_TM               // general Transl. Memory functions
  #define INCL_EQF_TP               // public translation processor functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_EDITORAPI        // editor API
  #define INCL_EQF_FOLDER           // folder API
  #include <eqf.h>                  // General Translation Manager include file

  #include "EQFITM.H"
  #define PATHCATFILECATEXTENSION "%s\\%s%s" // concatenate path, file and extension

/**********************************************************************/
/* test output ...                                                    */
/**********************************************************************/
#ifdef ITMTEST
  FILE *fOut;                          // test output
#endif

#define MAX_CHUNK     32000
#define POOLSIZE      40000

// types of ITMAli  data
typedef enum _ITMALITYPE
{
   ITMALI_HEADER = 1,                    // header
   ITMALI_SAVESTRUCT,                    // ITM save structure
   ITMALI_SSOURCE,                       // segmented source file
   ITMALI_STARGET,                       // segmented target file
   ITMALI_ALIGNSRC,                      // ALLALIGNED struct pusSrc
   ITMALI_ALIGNTGT,                      // ALLALIGNED struct pusTgt1
   ITMALI_ALIGNDIST,                     // ALLALIGNED struct psDist
   ITMALI_ALIGNTYPE,                     // ALLALIGNED struct pbType
   ITMALI_VISSRCNUMALIGNED,              // SRCVISDOC: pusNumALigned
   ITMALI_VISSRCANCHOR,                  // SRCVISDOC: pusAnchor
   ITMALI_VISSRCVISSTATE,                // SRCVISDOC: pVisState
   ITMALI_VISTGTNUMALIGNED,              // TGTVISDOC: pusNumALigned
   ITMALI_VISTGTANCHOR,                  // TGTVISDOC: pusAnchor
   ITMALI_VISTGTVISSTATE,                // TGTVISDOC: pVisState
   ITMALI_EOF = 999                      // end of profile file marker
} ITMALITYPE;

typedef struct _ITMSAVE
{
  CHAR        chTagTableName[MAX_EQF_PATH];
  CHAR        chTranslMemory[MAX_EQF_PATH];
  CHAR        chSGMLMem[MAX_LONGPATH/*MAX_EQF_PATH*/];
  CHAR        chSourceFile[MAX_LONGPATH];
  CHAR        chTargetFile[MAX_LONGPATH];
  EQF_BOOL    fNoTMDB;
  EQF_BOOL    fNoConfirm;
  EQF_BOOL    fSGMLITM;
  LANGUAGE    szSourceLang;
  LANGUAGE    szTargetLang;
  ULONG       ulSrcVisActSeg;
  ULONG       ulTgtVisActSeg;
  ULONG       ulAlignedAlloc;
  ULONG       ulAlignedUsed;
  STATUSINFO  stSrcInfo;               // srcdoc info for statusline
  STATUSINFO  stTgtInfo;               // tgtdoc info for statusline
  ULONG       ulSrcMaxSeg;
  ULONG       ulTgtMaxSeg;
  DOUBLE      dbMean;                  // mean
  DOUBLE      dbVar;                   // variance
  EQF_BOOL    fPrepIsVisual;
  USHORT      usSGMLFormat;            // Format of SGMLfile: Unicode/ASCII/ANsi
} ITMSAVE, * PITMSAVE;

typedef struct _ITMHEADER              // struct of header = 1st block in alifile
{
  CHAR       szPrefix[11];             // prefix: a fixed string
  BYTE       usVersion;                // version number, ITMVERSION3 02/06/10
  FDATE      fSrcDate;                 // date of src file
  FTIME      fSrcTime;                 // time of src file
  FDATE      fTgtDate;                 // date of tgt file
  FTIME      fTgtTime;                 // time of tgt file
  FDATE      fTagDate;                 // date of tagtable
  FTIME      fTagTime;                 // time of tagtable
  BYTE       bDummy[100];              // space for later enhancements
} ITMHEADER, *PITMHEADER;

// itmali  block header
typedef struct _ITMALIHEAD
{
   USHORT usType;                      // current profile data type
   ULONG  ulSize;                      // size of current profile data
} ITMALIHEAD, *PITMALIHEAD;

static SHORT WriteAliSegFile ( PITMIDA, USHORT,HFILE  );
static PVOID ITMSetVisDocBlock ( PITMALIHEAD, PITMVISDOC, USHORT );
static SHORT ReadAliSegFile ( PITMIDA, PITMALIHEAD,HFILE  );
static SHORT FillItmIda ( PITMIDA , PITMSAVE, PITMHEADER );
static VOID FillItmSave ( PITMIDA, PITMSAVE );
static SHORT ReadAliToDest ( HFILE, PITMALIHEAD, PVOID );
static BOOL  EQFITMFindAliName ( PSZ, PITMIDA, PUSHORT );
static VOID EQFITMFillPropAli ( PITMIDA , USHORT );
static USHORT ITMGetFileDateTime ( PSZ, PFDATE, PFTIME );
static USHORT FillItmHeader ( PITMIDA, PITMHEADER );
static BOOL ITMCompFileDateTime ( PSZ, PFDATE, PFTIME );
static BOOL ITMPoolAddData(PSZ , PFILEPOOL *);

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFITMWriteAli
//------------------------------------------------------------------------------
// Function call:     EQFITMWriteAli ()
//------------------------------------------------------------------------------
// Description:       Write save+continue file
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda
//------------------------------------------------------------------------------
// Returncode type:   SHORT sRC
//------------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_WRITEFILE  - error writing file
//------------------------------------------------------------------------------
// Function flow:     if no free aliname is available
//                      return error writing file
//                    if ok
//                      open/ create new alifile
//                    if ok
//                      set usType = 0
//                    while ok and usType not ITMALI_EOF
//                      init ItmAli (type& size) of block
//                      switch(current type of block)
//                      case ITMALI_HEADER: fill header struct
//                      case ITMALI_SAVESTRUCT: fill savestruct
//                             (saves all variables which are later needed)
//                      case ITMALI_SSOURCE: write segsourcefile to alifile
//                      case ITMALI_STARGET: write segTargetfile to alifile
//                      case ITMALI_ALIGNSRC: set ptr to Aligned.pusSrc array
//                      case ITMALI_ALIGNTGT: set ptr to Aligned.pusTgt array
//                      case ITMALI_ALIGNDIST: set ptr to Aligned.psDist array
//                      case ITMALI_ALIGNTYPE: set ptr to Aligned.pbType array
//                      case ITMALI_VISSRCNUMALIGNED:
//                      case ITMALI_VISSRCANCHOR:
//                      case ITMALI_VISSRCVISSTATE:
//                                set ptr to spec. VisDoc structure
//                      case ITMALI_VISTGTNUMALIGNED:
//                      case ITMALI_VISTGTANCHOR:
//                      case ITMALI_VISTGTVISSTATE:
//                                set ptr to spec. VisDoc structure
//                      case ITMALI_EOF: end indicator
//                      endswitch
//                      if not yet written
//                         write block header to alifile
//                         if ok write block to alifile
//                      endif
//                      get next type of block
//                   endwhile
//                   close continuation file
//                   if ok
//                     fill property file filepair with ref to alifile
//                   else
//                     delete corrupted alifile
//                     display error that file could not be written
//                   endif
//                   return rc if error
//------------------------------------------------------------------------------

SHORT EQFITMWriteAli
(
  PITMIDA   pITMIda
)
{
   CHAR        szAliName[MAX_EQF_PATH];         // buffer for profile file name
   ITMALIHEAD  ItmAli;
   HFILE       hAlifile = (HFILE) NULL;// handle of profile file
   USHORT      usOpenAction;           // action performed by DosOpen
   USHORT      usDosRC;                // return code of DosXXX calls
   USHORT      usType;                 // current profile data type
   USHORT      usBytesWritten;         // # of bytes written to file
   PVOID       pData;                  // ptr to current alifile data
   PBYTE       pCurData;               // ptr to current alifile data
   SHORT       sRC = 0;                // function return code
   ITMSAVE     ItmSave;
   BOOL        fNotYetWritten = TRUE;            // FALSE if segsource/tgt already written
   USHORT      usBytesToWrite;
   ULONG       ulSizeWritten;
   ULONG       ulRestSize;
   USHORT      usNumofAli;
   ITMHEADER   ItmHeader;

   if (! EQFITMFindAliName( szAliName, pITMIda, &usNumofAli ))
   {
     sRC = 1;                          // error aliname cannot be created
     /*****************************************************************/
     /* check whether already too many alifiles exist ...             */
     /*****************************************************************/
     ITMUtlError( pITMIda, ITM_MAXALI, MB_CANCEL, 0, NULL, EQF_ERROR );
   }
   else
   {
     // open ali file for output
     if ( !sRC )
     {
        usDosRC = UtlOpen( szAliName,
                           &hAlifile,
                           &usOpenAction, 0L,
                           FILE_NORMAL,
                           FILE_OPEN | FILE_CREATE,
                           OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                           0L,
                           FALSE );
        if ( usDosRC )
        {
           hAlifile = (HFILE) NULL;
           sRC = ERR_OPENFILE;
        } /* endif */
     } /* endif */
#ifdef ITMTEST
{
  ULONG  ulTemp1;
  fOut      = fopen ( "ITMSTAT.OUT", "a" );
  fprintf (fOut, "ALiFile Write out \n");
  fprintf (fOut, "Index Seg: ulSrc, ulTgt1, pbType \n");
  for ( ulTemp1=0;ulTemp1 < (pITMIda->Aligned.ulUsed);ulTemp1++ )
  {
    fprintf( fOut,
          "%4d Seg: %4d %4d 4%d \n",
           ulTemp1,
           pITMIda->Aligned.pulSrc[ulTemp1],
           pITMIda->Aligned.pulTgt1[ulTemp1],
           (USHORT) pITMIda->Aligned.pbType[ulTemp1]);
   } /* endfor */
   fprintf(fOut, " pITMIda-stSrcVisDoc: pulAnchor, pulNumAlign\n");
   for ( ulTemp1=0; ulTemp1 < pITMIda->TBSourceDoc.ulMaxSeg;ulTemp1++ )
   {
     fprintf( fOut,
              "%4d Seg: %4d     %4d \n",
               ulTemp1,
               pITMIda->stVisDocSrc.pulAnchor[ulTemp1],
               pITMIda->stVisDocTgt.pulNumAligned[ulTemp1]);
   } /* endfor */
   fprintf(fOut, " pITMIda-stVisDocTgt: pulAnchor, pulNumAlign\n");
   for ( ulTemp1=0; ulTemp1 < pITMIda->TBTargetDoc.ulMaxSeg;ulTemp1++ )
   {
     fprintf( fOut,
              "%4d Seg: %4d     %4d \n",
               ulTemp1,
               pITMIda->stVisDocTgt.pulAnchor[ulTemp1],
               pITMIda->stVisDocTgt.pulNumAligned[ulTemp1]);
   } /* endfor */
   fclose( fOut      );
   }
#endif

     // write all itmali  data types
     usType = 0;                         // start with first data type
     while ( !sRC && (usType <= ITMALI_EOF) )   // while not all data processed
     {
        // set default values (to trap notimplemented alifile parts)
        memset( &ItmAli, 0, sizeof(ItmAli) );
        pData  = NULL;

        // supply size and pointer of profile data
        switch ( usType )
        {
            case  ITMALI_HEADER:
              sRC = FillItmHeader(pITMIda, &ItmHeader);
              pData = &ItmHeader;
              ItmAli.usType = usType;
              ItmAli.ulSize = (LONG)sizeof(ItmHeader);
              fNotYetWritten = TRUE;
              break;
            case  ITMALI_SAVESTRUCT:
              /**********************************************************/
              /* fill ItmSave from ITMIda and VisDoc                    */
              /**********************************************************/
              FillItmSave(pITMIda, &ItmSave);
              pData = &ItmSave;
              ItmAli.usType = usType;
              ItmAli.ulSize = (LONG)sizeof(ItmSave);
              fNotYetWritten = TRUE;
              break;
            case  ITMALI_SSOURCE:
              sRC = WriteAliSegFile(pITMIda, usType, hAlifile);
              fNotYetWritten = FALSE;
              break;
            case  ITMALI_STARGET:
              sRC = WriteAliSegFile(pITMIda, usType, hAlifile);
              fNotYetWritten = FALSE;
              break;
            case  ITMALI_ALIGNSRC:
              pData = pITMIda->Aligned.pulSrc;
              ItmAli.usType = usType;
              ItmAli.ulSize = ((LONG)(pITMIda->Aligned.ulUsed))
                                 * sizeof(ULONG);
              fNotYetWritten = TRUE;
              break;
            case  ITMALI_ALIGNTGT:
              pData = pITMIda->Aligned.pulTgt1;
              ItmAli.usType = usType;
              ItmAli.ulSize = ((LONG)(pITMIda->Aligned.ulUsed))
                                 * sizeof(ULONG);
              fNotYetWritten = TRUE;
              break;
            case  ITMALI_ALIGNDIST:
              pData = pITMIda->Aligned.psDist;
              ItmAli.usType = usType;
              ItmAli.ulSize = ((LONG)(pITMIda->Aligned.ulUsed))
                                 * sizeof(USHORT);
              fNotYetWritten = TRUE;
              break;
            case ITMALI_ALIGNTYPE :
              pData = pITMIda->Aligned.pbType;
              ItmAli.usType = usType;
              ItmAli.ulSize = ((LONG)(pITMIda->Aligned.ulUsed))
                                 * sizeof(BYTE);
              fNotYetWritten = TRUE;
              break;
            case  ITMALI_VISSRCNUMALIGNED:
            case  ITMALI_VISSRCANCHOR :
            case  ITMALI_VISSRCVISSTATE :
              pData = ITMSetVisDocBlock(&ItmAli,
                                        &(pITMIda->stVisDocSrc),
                                        usType );
              fNotYetWritten = TRUE;
              break;
            case  ITMALI_VISTGTNUMALIGNED:
            case  ITMALI_VISTGTANCHOR :
            case  ITMALI_VISTGTVISSTATE :
              pData = ITMSetVisDocBlock(&ItmAli,
                                        &(pITMIda->stVisDocTgt),
                                        usType );
              fNotYetWritten = TRUE;
              break;
           case ITMALI_EOF:
              ItmAli.usType = usType;
              ItmAli.ulSize = 0L;       // no data area for end-of-file marker
              pData  = NULL;
              fNotYetWritten = TRUE;
              break;
        } /* endswitch */

        if ( fNotYetWritten )
        {
          // write ITMAli block header to file
          if ( ItmAli.usType )
          {
             usDosRC = UtlWrite( hAlifile,
                                 &ItmAli,
                                 sizeof(ItmAli),
                                 &usBytesWritten,
                                 FALSE );
             sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
          } /* endif */

          ulRestSize = ItmAli.ulSize;
          ulSizeWritten = 0L;
          while ( !sRC && ulRestSize )
          {
             pCurData = ((PBYTE)pData) + ulSizeWritten;
             usBytesToWrite = (USHORT) min( ulRestSize, (ULONG)MAX_CHUNK);
             usDosRC = UtlWrite( hAlifile,             //write block data
                                 pCurData,
                                 usBytesToWrite,
                                 &usBytesWritten,
                                 FALSE );
             sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
             if ( !sRC )
             {
               ulSizeWritten += usBytesWritten;
               ulRestSize -= usBytesWritten;
             } /* endif */
          } /* endwhile */
        } /* endif */

        // continue with next data type
        usType++;

     } /* endwhile */

     // close Itmali file
     if ( hAlifile )
     {
        UtlClose( hAlifile, FALSE );
     } /* endif */
     if ( !sRC )                 // if no error happened
     {
       EQFITMFillPropAli(pITMIda, usNumofAli );
       pITMIda->stVisDocSrc.fChanged = FALSE;
       pITMIda->stVisDocTgt.fChanged = FALSE;
     }
     else
     {
       /*****************************************************************/
       /* if error occured during writing, delete corrupted file        */
       /*****************************************************************/
       PSZ  pTemp;
       UtlDelete(szAliName, 0L, FALSE);
       pTemp = szAliName;
       szAliName[2] = EOS;
       ITMUtlError( pITMIda, ERROR_DISK_IS_FULL,MB_CANCEL, 1,&pTemp ,EQF_ERROR );
     } /* endif */
   } /* endif */

   return( sRC );
}
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     WriteAliSegFile
//------------------------------------------------------------------------------
// Function call:     WriteALiSegFile(pITMIda, usType, hAlifile)
//------------------------------------------------------------------------------
// Description:       write segmented file to continuation file
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda,
//                    USHORT     usType,
//                    HFILE      hAlifile
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       sRC = 0 if ok
//                      else if error occurred during writing to file
//------------------------------------------------------------------------------
// Function flow:     get filename of segmented file
//                    open segmented file
//                    if ok get size of segmented file
//                    if ok allocate temporary buffer
//                    if ok write block header
//                    while ok and not whole segmented file written
//                      write next chunk of segmented file to continuationfile
//                    endwhile
//                    free allocated temp buffer
//                    close segmented file
//------------------------------------------------------------------------------
static SHORT
WriteAliSegFile
(
  PITMIDA    pITMIda,
  USHORT     usType,
  HFILE      hAlifile
)
{
  CHAR       chSegFile[MAX_EQF_PATH];                 // segmented filename
  SHORT      sRC = 0;
  USHORT     usDosRC = 0;
  HFILE      hSegfile = (HFILE) NULL;            // handle of segmented file
  USHORT     usOpenAction;
  FILESTATUS       stStatus;                     // File status information
  ITMALIHEAD ItmAli;
  PSZ        pAliBuf = NULL;
  BOOL       fOK = TRUE;
  ULONG      ulBytesWritten = 0;
  ULONG      ulBytesToRead;
  ULONG      ulBytesRead;
  ULONG      ulRestSize;
  LONG       lAllocLen;

  memset( &stStatus, 0, sizeof(stStatus));
  if ( usType == ITMALI_SSOURCE )
  {
     strcpy(chSegFile, pITMIda->chSegSourceFile);
  }
  else
  {
     strcpy(chSegFile, pITMIda->chSegTargetFile);
  } /* endif */
   // open ali file for output
   if ( !sRC )
   {
      usDosRC = UtlOpen( chSegFile,
                         &hSegfile,
                         &usOpenAction, 0L,
                         FILE_NORMAL,
                         OPEN_ACTION_OPEN_IF_EXISTS,
                         OPEN_SHARE_DENYWRITE,
                         0L,
                         FALSE );
      if ( usDosRC )
      {
         hSegfile = (HFILE) NULL;
         sRC = ERR_OPENFILE;
      } /* endif */
   } /* endif */
   if ( !sRC )
   {
      // Get status info of input file (file size will be used only)
      usDosRC = UtlQFileInfo( hSegfile , 1,
                             (PBYTE)&stStatus,
                             sizeof(FILESTATUS),
                             FALSE );
   } /* endif */
   if ( !sRC && !usDosRC )
   {
     lAllocLen = (LONG)ITMALI_BUFSIZE;
     fOK = UtlAlloc( (PVOID *)&pAliBuf, 0L, lAllocLen, ERROR_STORAGE );
   }
   if ( fOK && !sRC && !usDosRC )
   {

     ItmAli.usType = usType;
     ItmAli.ulSize = (ULONG) stStatus.cbFile;
     // write ITMAli block header to file
     if ( ItmAli.usType )
     {
        usDosRC = UtlWriteL( hAlifile,
                            &ItmAli,
                            sizeof(ItmAli),
                            &ulBytesWritten,
                            FALSE );
        sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
     } /* endif */
     ulRestSize = ItmAli.ulSize;
     while ( !sRC && ulRestSize )
     {
       ulBytesToRead =  min( ulRestSize, (ULONG)ITMALI_BUFSIZE);
       usDosRC = UtlReadL( hSegfile,
                          pAliBuf,
                          ulBytesToRead,
                          &ulBytesRead,
                          TRUE );
       sRC = ( usDosRC ) ? ERR_READFILE : 0;
       if ( !sRC )
       {
          usDosRC = UtlWriteL( hAlifile,             //write block data
                              pAliBuf,
                              ulBytesRead,
                              &ulBytesWritten,
                              FALSE );
          sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
       } /* endif */
       if ( !sRC )
       {
         ulRestSize -= ulBytesWritten;
       } /* endif */
     } /* endwhile */

   } /* endif */
   if ( pAliBuf )
   {
     fOK = UtlAlloc( (PVOID *)&pAliBuf, 0L, 0L, NOMSG );
   } /* endif */
   // close segmented source
   if ( hSegfile )
   {
      UtlClose( hSegfile, FALSE );
   } /* endif */

  return (sRC);
} /* end of function WriteAliSegFile */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMSetVisDocBlock
//------------------------------------------------------------------------------
// Function call:     ITMSetVisDocBlock(pItmAli, pVisDoc, usType)
//------------------------------------------------------------------------------
// Description:       set ptr to specified array of visdoc
//------------------------------------------------------------------------------
// Parameters:        PITMALIHEAD   pItmAli,
//                    PITMVISDOC    pVisDoc,
//                    USHORT        usType
//------------------------------------------------------------------------------
// Returncode type:   PSZ
//------------------------------------------------------------------------------
// Returncodes:       pointer to data to be written
//------------------------------------------------------------------------------
// Function flow:     set size of specified block
//                    set ptr to specified data
//------------------------------------------------------------------------------
static PVOID
ITMSetVisDocBlock
(
  PITMALIHEAD   pItmAli,
  PITMVISDOC    pVisDoc,
  USHORT        usType
)
{
  PVOID      pData;
  ULONG      ulAllocLen;

   pItmAli->usType = usType;
   ulAllocLen = (pVisDoc->pDoc->ulMaxSeg) + 1;
   switch ( usType )
   {
     case  ITMALI_VISSRCVISSTATE:
     case  ITMALI_VISTGTVISSTATE:
       pItmAli->ulSize = ulAllocLen * sizeof (FLAGVIS);
       pData = pVisDoc->pVisState;
       break;
     case  ITMALI_VISSRCANCHOR :
     case  ITMALI_VISTGTANCHOR :
       pItmAli->ulSize = ulAllocLen * sizeof (ULONG);
       pData = pVisDoc->pulAnchor;
       break;
     default :
       /*************************************************************/
       /* ITMALI_VISSRCNUMALIGNED and ITMALI_VISTGTNUMALIGNED       */
       /*************************************************************/
       pItmAli->ulSize = ulAllocLen * sizeof (ULONG);
       pData = pVisDoc->pulNumAligned;
       break;
   } /* endswitch */

   return (pData);
} /* end of function ITMSetVisDocBlock */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFITMReadAli
//------------------------------------------------------------------------------
// Function call:     EQFITMReadAli ()
//------------------------------------------------------------------------------
// Description:       read in alignment save file to continue now
//------------------------------------------------------------------------------
// Parameters:        PITMIDA pITMIda
//                    BOOL    fReadAll
//------------------------------------------------------------------------------
// Returncode type:   SHORT sRC
//------------------------------------------------------------------------------
// Returncodes:       GOON           - success
//                    REALIGN        - delete alifile and align again
//                    STOPNEC        - cancel process
//------------------------------------------------------------------------------
// Function flow:     build name of continuation file
//                    open continuation file
//                    if ok read 1st block header
//                    if header has rigth size, read in header block
//                    if ok, check version and prefix description
//                    if not ok, issue error message
//                    while ok and not at last type ITMALI_EOF
//                      read next block header
//                      if ok
//                      switch (type of block)
//                      case ITMALI_SAVESTRUCT: alloc new savestruct
//                                              fill it with saved contents
//                      case ITMALI_SSOURCE: fill segmented source file
//                      case ITMALI_STARGET: fill segmented target file
//                      case ITMALI_ALIGNSRC: alloc&fill Aligned.pulSrc
//                      case ITMALI_ALIGNTGT: alloc&fill Aligned.pulTgt1
//                      case ITMALI_ALIGNDIST: alloc&fill Aligned.psDist
//                      case ITMALI_ALIGNTYPE: alloc&fill Aligned.pbType
//                      case ITMALI_VISSRCNUMALIGNED: alloc&Fill Visdoc array
//                      case ITMALI_VISSRCANCHOR    : alloc&Fill Visdoc array
//                      case ITMALI_VISSRCVISSTATE  : alloc&Fill Visdoc array
//                      case ITMALI_VISTGTNUMALIGNED: alloc&Fill Visdoc array
//                      case ITMALI_VISTGTANCHOR    : alloc&Fill Visdoc array
//                      case ITMALI_VISTGTVISSTATE  : alloc&Fill Visdoc array
//                      case ITMALI_EOF: end of continuation file
//                      default: msg corrupted continuation file
//                      endswitch
//                   endwhile
//                   close alifile
//                   fill ITMIda with all parameters from ITMSave structure
//                   free ITMSave structure
//                   return sRC   if not ok
//------------------------------------------------------------------------------

SHORT EQFITMReadAli
(
  PITMIDA   pITMIda,
  BOOL      fReadAll                    //false if no visdoc exists
)
{
   CHAR        szAliName[MAX_EQF_PATH]; // buffer for profile file name
   ITMALIHEAD  ItmAli;                 // profile block header
   SHORT       sRC = GOON;             // function return code
   USHORT      usOpenAction;           // action performed by DosOpen
   USHORT      usDosRC;                // return code of DosXXX calls
   USHORT      ulBytesRead;            // # of bytes read from file
   HFILE       hAlifile = (HFILE) NULL;// handle of alignment file
   PITMSAVE    pItmSave = NULL;
   PITMVISDOC  pVisDoc;
   ULONG       ulAllocLen;
   CHAR        szDrive[ MAX_DRIVE ];             // drive
   ULONG       ulNewPos;
   ITMHEADER   ItmHeader;
#ifdef ITMTEST
  USHORT     ulTemp1;
#endif

   /*******************************************************************/
   /*  TODO:                                                          */
   /* change all read cmds to TRUE so that they do the err handling   */
   /* bei UtlOpen: TRUE only if we want err msg if file does not      */
   /* exist                                                           */
   /*******************************************************************/

   UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
   UtlMakeEQFPath( szAliName, szDrive[0], SYSTEM_PATH, TEMPDIRSAVE );
   sprintf(szAliName, "%s\\$$ITMAli.%3.3d",
                       szAliName, pITMIda->usNumPrepared);

   // try to open alifile file
   if ( !sRC )
   {
      usDosRC = UtlOpen( szAliName,
                         &hAlifile,
                         &usOpenAction, 0L,
                         FILE_NORMAL,
                         FILE_OPEN,
                         OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                         0L,
                         FALSE );
      if ( usDosRC )
      {
         hAlifile = (HFILE) NULL;
         sRC = STOPNEC;
      } /* endif */
   } /* endif */
   // read and process alifile data until end of alifile reached
   ItmAli.usType = 0;
   /*******************************************************************/
   /* first type must be ITMALI_HEADER and its correspondent size     */
   /*******************************************************************/
   if ( !sRC && hAlifile )
   {
      // read alifile block header
      usDosRC = UtlRead( hAlifile,
                         &ItmAli,
                         sizeof(ItmAli),
                         &ulBytesRead,
                         FALSE );
      if ( usDosRC )
      {
        sRC = STOPNEC;
      } /* endif */
      if ( !sRC && (ItmAli.usType == ITMALI_HEADER )
           && (ItmAli.ulSize == (LONG)sizeof(ItmHeader)) )
      {
        sRC = ReadAliToDest ( hAlifile, &ItmAli, &ItmHeader);
        /**********************************************************/
        /* check version number and prefix                        */
        /**********************************************************/
        if ( (ItmHeader.usVersion == ITMVERSION3)
              && !strcmp(ItmHeader.szPrefix, ITMALIPREFIX) )
        {
          /********************************************************/
          /* alifile is ok                                        */
          /********************************************************/
        }
        else
        {
          ITMUtlError(pITMIda, ITM_ALICORRUPT, MB_OK, 0, NULL, EQF_WARNING );
          sRC = REALIGN;
        } /* endif */
      }
      else
      {
        ITMUtlError( pITMIda, ITM_ALICORRUPT, MB_OK, 0, NULL, EQF_WARNING );
        sRC = REALIGN;
      } /* endif */
   } /* endif */

   while ( !sRC && hAlifile && (ItmAli.usType != ITMALI_EOF) )
   {
      // read alifile block header
      usDosRC = UtlRead( hAlifile,
                         &ItmAli,
                         sizeof(ItmAli),
                         &ulBytesRead,
                         FALSE );
      if ( usDosRC )
      {
        sRC = STOPNEC;
      } /* endif */
      if ( !sRC )
      {
        switch ( ItmAli.usType )
        {
          case  ITMALI_SAVESTRUCT:
            if (ItmAli.ulSize != (LONG)sizeof(ITMSAVE) )
            {
              ITMUtlError(pITMIda, ITM_ALICORRUPT, MB_OK, 0, NULL, EQF_WARNING );
              sRC = REALIGN;
            }
            else
            {
              if (! UtlAlloc( (PVOID *)&pItmSave, 0L,
                        ItmAli.ulSize, ERROR_STORAGE) )
              {
                sRC = STOPNEC;
              }
              else
              {
                sRC = ReadAliToDest ( hAlifile, &ItmAli,pItmSave );
              }
            } /* endif */
            break;
          case  ITMALI_SSOURCE:
            sRC = ReadAliSegFile ( pITMIda, &ItmAli, hAlifile );
            break;
          case  ITMALI_STARGET:
            sRC = ReadAliSegFile ( pITMIda, &ItmAli, hAlifile );
            break;
          case  ITMALI_ALIGNSRC:

            if (! UtlAlloc( (PVOID *)&(pITMIda->Aligned.pulSrc), 0L,
                    (LONG)pItmSave->ulAlignedAlloc* sizeof(ULONG),
                    ERROR_STORAGE) )
            {
              sRC = STOPNEC;
            }
            else
            {
              sRC = ReadAliToDest ( hAlifile, &ItmAli,
                              pITMIda->Aligned.pulSrc);
            } /* endif */
            break;
          case  ITMALI_ALIGNTGT:
            if ( ! UtlAlloc( (PVOID *)&(pITMIda->Aligned.pulTgt1), 0L,
                    (LONG)pItmSave->ulAlignedAlloc * sizeof(ULONG),
                    ERROR_STORAGE) )
            {
              sRC = STOPNEC;
            }
            else
            {
              sRC = ReadAliToDest ( hAlifile, &ItmAli,
                              pITMIda->Aligned.pulTgt1);
            } /* endif */
            break;
          case  ITMALI_ALIGNDIST:
            if ( ! UtlAlloc( (PVOID *)&(pITMIda->Aligned.psDist), 0L,
                    (LONG)pItmSave->ulAlignedAlloc * sizeof(USHORT),
                    ERROR_STORAGE) )
            {
              sRC = STOPNEC;
            }
            else
            {
              sRC = ReadAliToDest ( hAlifile, &ItmAli,
                              pITMIda->Aligned.psDist);
            } /* endif */
            break;
          case  ITMALI_ALIGNTYPE:
            if ( ! UtlAlloc( (PVOID *)&(pITMIda->Aligned.pbType), 0L,
                    (LONG)pItmSave->ulAlignedAlloc* sizeof(BYTE),
                    ERROR_STORAGE) )
            {
              sRC = STOPNEC;
            }
            else
            {
              sRC = ReadAliToDest ( hAlifile, &ItmAli,
                              pITMIda->Aligned.pbType);
            } /* endif */
            break;
          case  ITMALI_VISSRCNUMALIGNED:
            if ( fReadAll )
            {
              pVisDoc = &(pITMIda->stVisDocSrc);
              if ( !pVisDoc->pulNumAligned )
              {
                ulAllocLen = ( pItmSave->ulSrcMaxSeg) + 1;
                ALLOCHUGE( pITMIda->pulSrcNumAlign, ULONG*, ulAllocLen, sizeof( ULONG ) );
              } /* endif */
              sRC = ReadAliToDest ( hAlifile, &ItmAli,pITMIda->pulSrcNumAlign);
            }
            else
            {
              sRC = (SHORT) UtlChgFilePtr(hAlifile,ItmAli.ulSize,
                             FILE_CURRENT,&ulNewPos, TRUE);
            } /* endif */
            break;
          case  ITMALI_VISSRCANCHOR:
            if ( fReadAll )
            {
              pVisDoc = &(pITMIda->stVisDocSrc);
              if ( !pVisDoc->pulAnchor )
              {
                ulAllocLen = ( pItmSave->ulSrcMaxSeg) + 1;
                ALLOCHUGE( pITMIda->pulSrcAnchor, ULONG*, ulAllocLen, sizeof( ULONG ) );
              } /* endif */
              sRC = ReadAliToDest ( hAlifile, &ItmAli,pITMIda->pulSrcAnchor);
            }
            else
            {
              sRC = (SHORT) UtlChgFilePtr(hAlifile,ItmAli.ulSize,
                             FILE_CURRENT,&ulNewPos, TRUE);
            } /* endif */
            break;
          case  ITMALI_VISSRCVISSTATE:
            if ( fReadAll )
            {
              pVisDoc = &(pITMIda->stVisDocSrc);
              if ( !pVisDoc->pVisState )
              {
                ulAllocLen = ( pItmSave->ulSrcMaxSeg) + 1;
                ALLOCHUGE( pITMIda->pSrcVisState, FLAGVIS*, ulAllocLen, sizeof( FLAGVIS) );
              } /* endif */
              sRC = ReadAliToDest ( hAlifile, &ItmAli,pITMIda->pSrcVisState);
            }
            else
            {
              sRC = (SHORT) UtlChgFilePtr(hAlifile,ItmAli.ulSize,
                             FILE_CURRENT,&ulNewPos, TRUE);
            } /* endif */

            break;
          case  ITMALI_VISTGTNUMALIGNED:
            if ( fReadAll )
            {
              pVisDoc = &(pITMIda->stVisDocTgt);
              if ( !pVisDoc->pulNumAligned )
              {
                ulAllocLen = ( pItmSave->ulTgtMaxSeg) + 1;
                ALLOCHUGE( pITMIda->pulTgtNumAlign, ULONG*, ulAllocLen, sizeof( ULONG) );
              } /* endif */
              sRC = ReadAliToDest ( hAlifile, &ItmAli,pITMIda->pulTgtNumAlign);
            }
            else
            {
              sRC = (SHORT) UtlChgFilePtr(hAlifile,ItmAli.ulSize,
                             FILE_CURRENT,&ulNewPos, TRUE);
            } /* endif */
            break;
          case  ITMALI_VISTGTANCHOR:
            if ( fReadAll )
            {
              pVisDoc = &(pITMIda->stVisDocTgt);
              if ( !pVisDoc->pulAnchor )
              {
                ulAllocLen = ( pItmSave->ulTgtMaxSeg) + 1;
                ALLOCHUGE( pITMIda->pulTgtAnchor, ULONG*, ulAllocLen, sizeof( ULONG ) );
              } /* endif */
              sRC = ReadAliToDest ( hAlifile, &ItmAli,pITMIda->pulTgtAnchor);
            }
            else
            {
              sRC = (SHORT) UtlChgFilePtr(hAlifile,ItmAli.ulSize,
                             FILE_CURRENT,&ulNewPos, TRUE);
            } /* endif */
            break;
          case  ITMALI_VISTGTVISSTATE:
            if ( fReadAll )
            {
              pVisDoc = &(pITMIda->stVisDocTgt);
              if ( !pVisDoc->pVisState )
              {
                ulAllocLen = ( pItmSave->ulTgtMaxSeg) + 1;
                ALLOCHUGE( pITMIda->pTgtVisState, FLAGVIS*, ulAllocLen, sizeof( FLAGVIS) );
              } /* endif */
              sRC = ReadAliToDest ( hAlifile, &ItmAli,pITMIda->pTgtVisState);
            }
            else
            {
              sRC = (SHORT) UtlChgFilePtr(hAlifile,ItmAli.ulSize,
                             FILE_CURRENT,&ulNewPos, TRUE);
            } /* endif */
            break;
         case ITMALI_EOF:
            if (ItmAli.ulSize != 0L)      // no data area for end-of-file marker
            {
              ITMUtlError(pITMIda, ITM_ALICORRUPT, MB_OK, 0, NULL, EQF_WARNING );
              sRC = REALIGN;
            }
            break;
          default :
            ITMUtlError(pITMIda, ITM_ALICORRUPT, MB_OK, 0, NULL, EQF_WARNING );
            sRC = REALIGN;
            break;
        } /* endswitch */
      } /* endif */
   } /* endwhile */

   // close Itmali file
   if ( hAlifile )
   {
      UtlClose( hAlifile, FALSE );
   } /* endif */
   if ( pItmSave )
   {
     if ( !sRC )
     {
       sRC = FillItmIda (pITMIda, pItmSave, &ItmHeader );
     } /* endif */
     UtlAlloc( (PVOID *)&pItmSave, 0L, 0L, NOMSG );
   } /* endif */

#ifdef ITMTEST
  fOut      = fopen ( "ITMSTAT.OUT", "a" );
  fprintf (fOut, "ALiFile ReadIn \n");
  fprintf (fOut, "Index Seg: ulSrc, ulTgt1, pbType \n");
  for ( ulTemp1=0;ulTemp1 < (pITMIda->Aligned.ulUsed);ulTemp1++ )
  {
    fprintf( fOut,
          "%4d Seg: %4d %4d 4%d \n",
           ulTemp1,
           pITMIda->Aligned.pulSrc[ulTemp1],
           pITMIda->Aligned.pulTgt1[ulTemp1],
           (USHORT) pITMIda->Aligned.pbType[ulTemp1]);
   } /* endfor */
   fprintf(fOut, " pITMIda: pulSrcAnchor, pulSrcNumAlign\n");
   for ( ulTemp1=0; ulTemp1 < pITMIda->TBSourceDoc.ulMaxSeg;ulTemp1++ )
   {
     fprintf( fOut,
              "%4d Seg: %4d     %4d \n",
               ulTemp1,
               pITMIda->pulSrcAnchor[ulTemp1],
               pITMIda->pulSrcNumAlign[ulTemp1]);
   } /* endfor */
   fprintf(fOut, " pITMIda: pulTgtAnchor, pulTgtNumAlign\n");
   for ( ulTemp1=0; ulTemp1 < pITMIda->TBTargetDoc.ulMaxSeg;ulTemp1++ )
   {
     fprintf( fOut,
              "%4d Seg: %4d     %4d \n",
               ulTemp1,
               pITMIda->pulTgtAnchor[ulTemp1],
               pITMIda->pulTgtNumAlign[ulTemp1]);
   } /* endfor */
   fclose( fOut      );
#endif
   return( sRC );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ReadAliToDest
//------------------------------------------------------------------------------
// Function call:     ReadAliToDest(hALifile, pITMAli, pData)
//------------------------------------------------------------------------------
// Description:       read in one block to specified ptr
//------------------------------------------------------------------------------
// Parameters:        HFILE        hAlifile
//                    PITMALIHEAD  pItmAli,
//                    PVOID        pData
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       sRC   0 ok
//                    else   read error
//------------------------------------------------------------------------------
// Function flow:     get size of block from block header
//                    while ok and not all read in
//                      read in next chunk
//------------------------------------------------------------------------------
static SHORT
ReadAliToDest
(
  HFILE        hAlifile,
  PITMALIHEAD  pItmAli,
  PVOID        pData
)
{
  ULONG    ulRestSize;
  ULONG    ulSizeRead;
  SHORT    sRC = 0;
  ULONG    ulBytesToRead;
  ULONG    ulBytesRead;
  PVOID    pCurData;
  USHORT   usDosRC;

   ulRestSize = pItmAli->ulSize;
   ulSizeRead = 0L;
   while ( !sRC && ulRestSize )
   {
     pCurData = ((PBYTE)pData) + ulSizeRead;
     ulBytesToRead = min(ulRestSize, (ULONG)MAX_CHUNK );
     usDosRC = UtlReadL( hAlifile,
                        pCurData,
                        ulBytesToRead,
                        &ulBytesRead,
                        FALSE );
     if ( usDosRC )
     {
       sRC = STOPNEC;
     } /* endif */
     if ( !sRC )
     {
       ulSizeRead += ulBytesRead;
       ulRestSize -= ulBytesRead;
     } /* endif */
   } /* endwhile */
  return (sRC);
} /* end of function ReadAliToDest */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     FillItmSave
//------------------------------------------------------------------------------
// Function call:     FillItmSave(pITMIda, pITMSave)
//------------------------------------------------------------------------------
// Description:       copy parameters from ITMIda to ITMSave structure
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda,
//                    PITMSAVE  pItmSave
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     copy parameters from ITMIda to ITMSave structure
//------------------------------------------------------------------------------
static VOID
FillItmSave
(
  PITMIDA   pITMIda,
  PITMSAVE  pItmSave
)
{
  BOOL      fOK;

  fOK = UtlCopyParameter(pItmSave->chTranslMemory,
                         pITMIda->chTranslMemory,
                         MAX_EQF_PATH,
                         TRUE);
  fOK = UtlCopyParameter(pItmSave->chTagTableName,
                         pITMIda->chTagTableName,
                         MAX_EQF_PATH,
                         TRUE);

  fOK = UtlCopyParameter(pItmSave->chSGMLMem,
                         pITMIda->chSGMLMem,
                         sizeof(pITMIda->chSGMLMem),
                         TRUE);

  fOK = UtlCopyParameter(pItmSave->chSourceFile,
                         pITMIda->chSourceFile,
                         sizeof(pITMIda->chSourceFile),
                         FALSE);
  fOK = UtlCopyParameter(pItmSave->chTargetFile,
                         pITMIda->chTargetFile,
                         sizeof(pITMIda->chSourceFile),
                         FALSE);
  fOK = UtlCopyParameter(pItmSave->szSourceLang,
                         pITMIda->szSourceLang,
                         sizeof(pITMIda->szSourceLang),
                         TRUE);
  fOK = UtlCopyParameter(pItmSave->szTargetLang,
                         pITMIda->szTargetInputLang,
                         sizeof(pITMIda->szTargetInputLang),
                         TRUE);
  pItmSave->fNoTMDB    = (EQF_BOOL)pITMIda->fNoTMDB;
  pItmSave->fNoConfirm = (EQF_BOOL)pITMIda->fNoConfirm;
  pItmSave->fSGMLITM   = (EQF_BOOL)pITMIda->fSGMLITM;
  pItmSave->ulAlignedAlloc = pITMIda->Aligned.ulAlloc;
  pItmSave->ulAlignedUsed  = pITMIda->Aligned.ulUsed;
  pItmSave->ulSrcVisActSeg = pITMIda->stVisDocSrc.ulVisActSeg;
  pItmSave->ulTgtVisActSeg = pITMIda->stVisDocTgt.ulVisActSeg;

  pItmSave->stSrcInfo.ulSegTotal = pITMIda->stSrcInfo.ulSegTotal;
  pItmSave->stSrcInfo.ulSegUnAligned = pITMIda->stSrcInfo.ulSegUnAligned;
  pItmSave->stSrcInfo.ulSegIrregular= pITMIda->stSrcInfo.ulSegIrregular;
  pItmSave->stSrcInfo.usSegCrossOut = pITMIda->stSrcInfo.usSegCrossOut;
  strcpy (pItmSave->stSrcInfo.chType, pITMIda->stSrcInfo.chType);

  pItmSave->stTgtInfo.ulSegTotal = pITMIda->stTgtInfo.ulSegTotal;
  pItmSave->stTgtInfo.ulSegUnAligned = pITMIda->stTgtInfo.ulSegUnAligned;
  pItmSave->stTgtInfo.ulSegIrregular = pITMIda->stTgtInfo.ulSegIrregular;
  pItmSave->stTgtInfo.usSegCrossOut = pITMIda->stTgtInfo.usSegCrossOut;
  strcpy (pItmSave->stTgtInfo.chType, pITMIda->stTgtInfo.chType);

  pItmSave->ulSrcMaxSeg = pITMIda->TBSourceDoc.ulMaxSeg;
  pItmSave->ulTgtMaxSeg = pITMIda->TBTargetDoc.ulMaxSeg;
  pItmSave->dbMean = pITMIda->dbMean;
  pItmSave->dbVar  = pITMIda->dbVar ;

  pItmSave->fPrepIsVisual = (EQF_BOOL)pITMIda->fVisual;
  pItmSave->usSGMLFormat = pITMIda->usSGMLFormat;
} /* end of function FillItmSave */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     FillItmIda
//------------------------------------------------------------------------------
// Function call:     FillItmIda(pITMIda, pItmSave, pItmHeader)
//------------------------------------------------------------------------------
// Description:       fill parameters from ITMSave to ITMIda and check
//                    whether they are valid
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda,
//                    PITMSAVE   pItmSave,
//                    PITMHEADER pItmHeader
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       GOON   ok
//                    REALIGN  realign files again
//                    STOPNEC  stop process
//------------------------------------------------------------------------------
// Function flow:     check that memory is equal to currently specified mem
//                    issue error, either stop or realign with new setting
//                    check that tagtable is equal to currently specified one
//                    issue error, either stop or realign with new one
//                    copy & check src/tgt language
//                    check if source/targetfilename exist
//                    if ok
//                      check whether date/time of src/tgtfile and tagtable
//                      match
//                      if updated, msg updated, realign files
//                    if ok, fill ITMIda variables
//------------------------------------------------------------------------------
static SHORT
FillItmIda
(
  PITMIDA    pITMIda,
  PITMSAVE   pItmSave,
  PITMHEADER pItmHeader
)
{
  SHORT    sRC = GOON;
  USHORT   usMBID;
  PSZ      pData;
  BOOL     fOK = TRUE;
  CHAR     szBuf1[ MAX_PATH144];           // temp buffer ...
  CHAR     szBuf2[ MAX_PATH144];           // temp buffer ...
  PSZ      pTempName;                      // pointer to temp. filename

  /********************************************************************/
  /* check that mem and tagtable match with the current values of     */
  /* the dialog or cmdline                                            */
  /********************************************************************/
  if (stricmp(pITMIda->chTranslMemory, pItmSave->chTranslMemory))
  {
    pData = UtlGetFnameFromPath(pItmSave->chTranslMemory);
    if ( !pData )
    {
      pData = pItmSave->chTranslMemory;
    } /* endif */
    pData = Utlstrccpy( szBuf1, pData, DOT );


    usMBID = ITMUtlError(pITMIda, ITM_PARAMNOTMATCH, MB_YESNO, 1,
                         &pData, EQF_ERROR );
    if ( usMBID == MBID_YES )
    {
      sRC = REALIGN;
    }
    else
    {
      sRC = STOPNEC;
    } /* endif */
  }
  if (!sRC && (stricmp(pITMIda->chTagTableName, pItmSave->chTagTableName)))
  {
    pData = pItmSave->chTagTableName;
    usMBID = ITMUtlError(pITMIda, ITM_PARAMNOTMATCH, MB_YESNO, 1,
                         &pData, EQF_ERROR );
    if ( usMBID == MBID_YES )
    {
      sRC = REALIGN;
    }
    else
    {
      sRC = STOPNEC;
    } /* endif */
  }

/**********************************************************************/
/* in windows srclng and tgtlng must be checked also                  */
/**********************************************************************/
  if (!sRC )
  {
    fOK = UtlCopyParameter( pITMIda->szSourceLang,
                           pItmSave->szSourceLang,
                           sizeof(pITMIda->szSourceLang),
                           TRUE);
    fOK = UtlCopyParameter( pITMIda->szTargetLang,
                           pItmSave->szTargetLang,
                           sizeof(pITMIda->szTargetLang),
                           TRUE);
    fOK = UtlCopyParameter( pITMIda->szTargetInputLang,
                           pItmSave->szTargetLang,
                           sizeof(pITMIda->szTargetLang),
                           TRUE);

    fOK = UtlCopyParameter( pITMIda->chSGMLMem,
                           pItmSave->chSGMLMem,
                           sizeof(pITMIda->chSGMLMem),
                           TRUE);

    fOK = UtlCopyParameter( pITMIda->chSourceFile,
                           pItmSave->chSourceFile,
                           sizeof(pITMIda->chSourceFile),
                           FALSE);
    fOK = UtlCopyParameter( pITMIda->chTargetFile,
                           pItmSave->chTargetFile,
                           sizeof(pITMIda->chSourceFile),
                           FALSE);
    /******************************************************************/
    /* check whether source/targetfile exist                          */
    /******************************************************************/
    if ( fOK && !ITMFileExist( pITMIda->chSourceFile ) )                /* @CA7 */
    {                                                                   /* @CA7 */
      fOK = FALSE;                                                      /* @CA7 */
      pTempName = pITMIda->chSourceFile;                                /* @CA7 */
    } /* endif */                                                       /* @CA7 */
    if ( fOK && !ITMFileExist( pITMIda->chTargetFile ) )                /* @CA7 */
    {                                                                   /* @CA7 */
      fOK = FALSE;                                                      /* @CA7 */
      pTempName = pITMIda->chTargetFile;                                /* @CA7 */
    } /* endif */                                                       /* @CA7 */

    if ( !fOK )
    {
      ITMUtlError( pITMIda, FILE_NOT_EXISTS, MB_CANCEL, 1, &pTempName, EQF_ERROR );
      sRC = STOPNEC;
    }
    else
    {
      /******************************************************************/
      /* check whether date and time of src/tgtfile match               */
      /******************************************************************/
      fOK = ITMCompFileDateTime ( pITMIda->chSourceFile, &(pItmHeader->fSrcDate),
                                  &(pItmHeader->fSrcTime));
      if ( !fOK )
      {
        pData = pITMIda->chSourceFile;
      }
      else
      {
        fOK = ITMCompFileDateTime ( pITMIda->chTargetFile, &(pItmHeader->fTgtDate),
                                    &(pItmHeader->fTgtTime));
        if ( !fOK )
        {
          pData = pITMIda->chTargetFile;
        }
        else
        {
          /******************************************************************/
          /* fill in name of tagtable and pathes                            */
          /******************************************************************/
          UtlMakeEQFPath( szBuf1, NULC, TABLE_PATH, NULL);
          sprintf( szBuf2, PATHCATFILECATEXTENSION, szBuf1,
                     pITMIda->chTagTableName,
                     EXT_OF_FORMAT );
          fOK = ITMCompFileDateTime ( szBuf2,
                                      &(pItmHeader->fTagDate),
                                      &(pItmHeader->fTagTime));
          if ( !fOK )
          {
            pData = pITMIda->chTagTableName;
          } /* endif */
        } /* endif */
      } /* endif */
      if ( !fOK  )
      {
        ITMUtlError(pITMIda, ITM_FILEUPDATE, MB_CANCEL, 1, &pData, EQF_ERROR);
        sRC = REALIGN;
      }
    } /* endif */
    if (fOK)
    {
      pITMIda->fNoTMDB                 =   pItmSave->fNoTMDB;
      pITMIda->fNoConfirm              =   pItmSave->fNoConfirm;
      pITMIda->fSGMLITM                =   pItmSave->fSGMLITM;
      pITMIda->Aligned.ulAlloc         =   pItmSave->ulAlignedAlloc;
      pITMIda->Aligned.ulUsed          =   pItmSave->ulAlignedUsed;
      pITMIda->stVisDocSrc.ulVisActSeg =   pItmSave->ulSrcVisActSeg;
      pITMIda->stVisDocTgt.ulVisActSeg =   pItmSave->ulTgtVisActSeg;

      pITMIda->ulSegTotal            += pItmSave->stSrcInfo.ulSegTotal;
      pITMIda->stSrcInfo.ulSegTotal   = pItmSave->stSrcInfo.ulSegTotal;
      pITMIda->stSrcInfo.ulSegUnAligned=pItmSave->stSrcInfo.ulSegUnAligned;
      pITMIda->stSrcInfo.ulSegIrregular=pItmSave->stSrcInfo.ulSegIrregular;
      pITMIda->stSrcInfo.usSegCrossOut= pItmSave->stSrcInfo.usSegCrossOut;
      strcpy (pITMIda->stSrcInfo.chType, pItmSave->stSrcInfo.chType);

      pITMIda->stTgtInfo.ulSegTotal   =  pItmSave->stTgtInfo.ulSegTotal;
      pITMIda->stTgtInfo.ulSegUnAligned=pItmSave->stTgtInfo.ulSegUnAligned;
      pITMIda->stTgtInfo.ulSegIrregular=pItmSave->stTgtInfo.ulSegIrregular;
      pITMIda->stTgtInfo.usSegCrossOut= pItmSave->stTgtInfo.usSegCrossOut;
      strcpy (pITMIda->stTgtInfo.chType, pItmSave->stTgtInfo.chType);

      pITMIda->TBSourceDoc.ulMaxSeg  = pItmSave->ulSrcMaxSeg;
      pITMIda->TBTargetDoc.ulMaxSeg  = pItmSave->ulTgtMaxSeg;

      pITMIda->dbMean = pItmSave->dbMean;
      pITMIda->dbVar  = pItmSave->dbVar;
      pITMIda->fPrepIsVisual = pItmSave->fPrepIsVisual;
      pITMIda->usSGMLFormat = pItmSave->usSGMLFormat;
      if ((pITMIda->usSGMLFormat == SGMLFORMAT_ASCII) ||
           (pITMIda->usSGMLFormat == SGMLFORMAT_ANSI))
      { //use system preferences lang to write SGML memory
        //P015855:OEM-CP needed for Unicode2Ansi too
        pITMIda->ulSGMLFormatCP = GetLangOEMCP(NULL);
        pITMIda->ulAnsiCP = GetLangAnsiCP(NULL);
      }
      else
      {
        pITMIda->ulSGMLFormatCP = 0L;
        pITMIda->ulAnsiCP = 0L;
      }
    } /* endif */
  } /* endif */
  return (sRC);
}                                  /* end of function FillItmIda */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ReadAliSegFile
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       read in segmented file from continuation file
//                    and write it to segmented file
//------------------------------------------------------------------------------
// Parameters:        PITMIDA    pITMIda,
//                    PITMALIHEAD pITMAli,
//                    HFILE      hAlifile
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0  ok
//                    STOPNEC  if error occurred
//------------------------------------------------------------------------------
// Function flow:     build filename of segmented file
//                    open segmented file
//                    if ok, alloc temporary buffer
//                    while not all read
//                      read in next chunk
//                      if ok, write it to segmented file
//                    endwhile
//                    free temporary buffer
//                    close segmented file
//                    if not ok, return that process must be stopped
//------------------------------------------------------------------------------
static SHORT
ReadAliSegFile
(
  PITMIDA    pITMIda,
  PITMALIHEAD pITMAli,
  HFILE      hAlifile
)
{
  CHAR       chSegFile[MAX_EQF_PATH];                 // segmented filename
  SHORT      sRC = 0;
  USHORT     usDosRC = 0;
  HFILE      hSegfile = (HFILE) NULL;            // handle of segmented file
  USHORT     usOpenAction;
  PSZ        pAliBuf = NULL;
  BOOL       fOK = TRUE;
  ULONG      ulBytesWritten = 0;
  ULONG      ulBytesToRead;
  ULONG      ulBytesRead;
  ULONG      ulRestSize;
  CHAR       szDrive[ MAX_DRIVE ];           // drive
  CHAR       chTempName[MAX_EQF_PATH];
  PSZ        pFile;                       // pointer to source filename


  UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;

  if ( pITMAli->usType == ITMALI_SSOURCE )
  {
     pFile = UtlGetFnameFromPath(pITMIda->chSourceFile);
     UtlMakeEQFPath( chTempName, szDrive[0],
                     DIRSEGSOURCEDOC_PATH, pITMIda->chITMSFolder );
     strcat( chTempName, BACKSLASH_STR );
    // handle long or short file name
     if ( UtlIsLongFileName( pFile ))
     {
       BOOL fIsNew;                   // new document flag

       // get the correct short file name for the document
       FolLongToShortDocName( chTempName,
                              pFile,
                              pITMIda->chShortSrcFName,
                              &fIsNew );
       pFile = pITMIda->chShortSrcFName;
     }
     else
     {
       strcpy( pITMIda->chShortSrcFName, pFile );
     } /* endif */

     strcpy(pITMIda->chSegSourceFile, chTempName);
     strcat( pITMIda->chSegSourceFile, pFile );
     strcpy(chSegFile, pITMIda->chSegSourceFile);
  }
  else
  {
     pFile = UtlGetFnameFromPath(pITMIda->chTargetFile);
     UtlMakeEQFPath( chTempName, szDrive[0],
                     DIRSEGSOURCEDOC_PATH, pITMIda->chITMTFolder);
     strcat( chTempName, BACKSLASH_STR );
    // handle long or short file name
     if ( UtlIsLongFileName( pFile ))
     {
       BOOL fIsNew;                   // new document flag

       // get the correct short file name for the document
       FolLongToShortDocName( chTempName,
                              pFile,
                              pITMIda->chShortTgtFName,
                              &fIsNew );
       pFile = pITMIda->chShortTgtFName;
     }
     else
     {
       strcpy( pITMIda->chShortTgtFName, pFile );
     } /* endif */

     strcpy(pITMIda->chSegTargetFile, chTempName);
     strcat( pITMIda->chSegTargetFile, pFile );
     strcpy(chSegFile, pITMIda->chSegTargetFile);
  } /* endif */
   // open ali file for output
   if ( !sRC )
   {
      usDosRC = UtlOpen( chSegFile,
                         &hSegfile,
                         &usOpenAction, 0L,
                         FILE_NORMAL,
                         FILE_OPEN | FILE_CREATE,
                         OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                         0L,
                         FALSE );
      if ( usDosRC )
      {
         hSegfile = (HFILE) NULL;
         sRC = ( usDosRC ) ? STOPNEC : GOON;
      } /* endif */
   } /* endif */
   if ( !sRC && !usDosRC )
   {
     fOK = UtlAlloc( (PVOID *)&pAliBuf, 0L, (LONG)ITMALI_BUFSIZE, ERROR_STORAGE );
   }

   if ( fOK && !sRC && !usDosRC )
   {
     ulRestSize = pITMAli->ulSize;
     while ( !sRC && ulRestSize )
     {
       ulBytesToRead =  min( ulRestSize, (ULONG)ITMALI_BUFSIZE);
       usDosRC = UtlReadL( hAlifile,
                          pAliBuf,
                          ulBytesToRead,
                          &ulBytesRead,
                          TRUE );
       sRC = ( usDosRC ) ? STOPNEC : GOON;
       if ( !sRC )
       {
          usDosRC = UtlWriteL( hSegfile,             //write block data
                              pAliBuf,
                              ulBytesRead,
                              &ulBytesWritten,
                              FALSE );
          sRC = ( usDosRC ) ? STOPNEC : GOON;
       } /* endif */
       if ( !sRC )
       {
         ulRestSize -= ulBytesWritten;
       } /* endif */
     } /* endwhile */

   } /* endif */
   if ( !sRC && !usDosRC )
   {
     fOK = UtlAlloc( (PVOID *)&pAliBuf, 0L, 0L, NOMSG );
   }
   // close segmented source
   if ( hSegfile )
   {
      UtlClose( hSegfile, FALSE );
   } /* endif */
   if ( !fOK )
   {
     sRC = STOPNEC;
   } /* endif */

  return (sRC);
} /* end of function ReadAliSegFile */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFITMPropRead
//------------------------------------------------------------------------------
// Function call:     EQFITMPropRead()
//------------------------------------------------------------------------------
// Description:       read in property file
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_READFILE   - error reading file
//                    ERR_NOMEMORY   - memory allocation error
//------------------------------------------------------------------------------
// Function flow:     build name of propertyfile
//                    alloc space for property contents
//                    if ok, open property file
//                    if ok, read in property file
//                    if ok, read in list of filenames
//                    close propertyfile
//------------------------------------------------------------------------------

SHORT EQFITMPropRead
(
  PITMIDA   pITMIda
)
{
   CHAR        szDrive[ MAX_DRIVE ];             // drive
   USHORT      usSize;
   USHORT      usOldSize;
   BOOL        fOK;
   USHORT      usDosRC;
   SHORT       sRC = 0;
   HFILE       hPropfile = (HFILE) NULL;         // handle of propertyfile
   ULONG       ulBytesRead;
   USHORT      usOpenAction;

   UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
   UtlMakeEQFPath( pITMIda->szPropName, szDrive[0], PROPERTY_PATH, NULL );
   strcat (pITMIda->szPropName, "\\");
   strcat (pITMIda->szPropName, ITM_PROPERTYFILE);

   /*******************************************************************/
   /* allocate space for property contents and try to fill it....     */
   /*******************************************************************/
   usSize = sizeof(PROPITM);
   usOldSize = sizeof(PROPITMOLD);

   fOK = UtlAlloc( (PVOID *) &pITMIda->pstPropItm, 0L, (LONG) usSize, ERROR_STORAGE );

   if ( fOK )
   {
     usDosRC = UtlOpen( pITMIda->szPropName, &hPropfile, &usOpenAction, 0L, FILE_NORMAL,
                        FILE_OPEN | FILE_CREATE, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, FALSE );
     if ( usDosRC )
     {
        hPropfile = (HFILE) NULL;
        sRC = ERR_OPENFILE;
     } /* endif */
   }
   else
   {
     sRC = ERR_NOMEMORY;
   } /* endif */
   if ( !sRC )
   {
     usDosRC = UtlReadL( hPropfile, pITMIda->pstPropItm, usOldSize, &ulBytesRead, TRUE );
     sRC = ( usDosRC ) ? ERR_READFILE : 0;

     pITMIda->pstPropItm->pFileNamePool= NULL;       // reset
     pITMIda->pstPropItm->pListFiles = NULL;         // reset
   } /* endif */

   /*******************************************************************/
   /* check whether it is new PropItm structure ( 28.9.99)            */
   /*******************************************************************/
   if (!sRC )
   {
     if ( (strcmp( pITMIda->pstPropItm->chVersion, ITM_VERSION3) == 0) ||
          (strcmp( pITMIda->pstPropItm->chVersion, ITM_VERSION4) == 0) )
     {
       // a new version prop file, read remaining part of the properties
       usDosRC = UtlReadL( hPropfile, (PBYTE)(pITMIda->pstPropItm) + usOldSize, usSize - usOldSize, &ulBytesRead, TRUE );
       sRC = ( usDosRC ) ? ERR_READFILE : 0;
     }
     else if (!strcmp(&(pITMIda->pstPropItm->chVersion[0]), ITM_VERSION) )
     {
       // copy path info to new fields
       strncpy( pITMIda->pstPropItm->szSrcDirectory, pITMIda->pstPropItm->szOldSrcDirectory, sizeof(pITMIda->pstPropItm->szOldSrcDirectory) );
       strncpy( pITMIda->pstPropItm->szTgtDirectory, pITMIda->pstPropItm->szOldTgtDirectory, sizeof(pITMIda->pstPropItm->szOldTgtDirectory) );
     }
     else
     {
       sRC = ERR_READFILE;                  // do not use filepairs info
     } /* endif */
   } /* endif */

   // load file pair table
   if ( !sRC )
   {
     if ( (strcmp( pITMIda->pstPropItm->chVersion, ITM_VERSION4) == 0) )
     {
       // file pair table is using the new layout, so just read in the table
       ULONG ulSize = (pITMIda->pstPropItm->usNumFiles) * sizeof (FILEPAIR);
       if (ulSize )
       {
         fOK = UtlAlloc( (PVOID *) &(pITMIda->pstPropItm->pListFiles), 0L, ulSize, ERROR_STORAGE );
         if ( fOK )
         {
           usDosRC = UtlReadL( hPropfile, pITMIda->pstPropItm->pListFiles, ulSize, &ulBytesRead, TRUE );
           sRC = ( usDosRC ) ? ERR_READFILE : 0;
         }
         else
         {
           sRC = ERR_NOMEMORY;
         } /* endif */
       }
     }
     else
     {
       // load old version file pair table and convert file name table to the new layout
       usSize = (pITMIda->pstPropItm->usNumFiles) * sizeof (FILEPAIR_VITM3);
       if (usSize )
       {
         // load old version file pair table 
         PFILEPAIR_VITM3 pOldFilePairs = NULL;

         fOK = UtlAlloc( (PVOID *) &pOldFilePairs, 0L, (LONG) usSize, ERROR_STORAGE );
         if ( fOK )
         {
           usDosRC = UtlReadL( hPropfile, pOldFilePairs, usSize, &ulBytesRead, TRUE );
           sRC = ( usDosRC ) ? ERR_READFILE : 0;
         }
         else
         {
           sRC = ERR_NOMEMORY;
         } /* endif */

         // convert file pairname table to the new layout
         if ( !sRC )
         {
           ULONG ulSize = (pITMIda->pstPropItm->usNumFiles) * sizeof (FILEPAIR);
           fOK = UtlAlloc( (PVOID *)&(pITMIda->pstPropItm->pListFiles), 0L, (LONG) ulSize, ERROR_STORAGE );
           if ( fOK )
           {
             PFILEPAIR_VITM3 pCurOld = pOldFilePairs;
             PFILEPAIR pCurNew = pITMIda->pstPropItm->pListFiles;
             USHORT usI = 0;
             while ( usI < pITMIda->pstPropItm->usNumFiles )
             {
               pCurNew->ulSrcOffset = pCurOld->usSrcOffset + 4;
               pCurNew->ulTgtOffset = pCurOld->usTgtOffset + 4;
               pCurNew->usAliNum = pCurOld->usAliNum;
               pCurNew++; pCurOld++; usI++;
             }
           }
           else
           {
             sRC = ERR_NOMEMORY;
           } /* endif */

         }

         if ( pOldFilePairs != NULL ) UtlAlloc( (PVOID *)&pOldFilePairs, 0, 0, NOMSG );
       } /* endif */
     } /* endif */
   } /* endif */

   if ( !sRC )
   {
     if ( (strcmp( pITMIda->pstPropItm->chVersion, ITM_VERSION4) == 0) )
     {
       // file pool has variable length, so we have to read the first 4 bytes of the pool area to get the size
       ULONG ulSize = 0;
       usDosRC = UtlReadL( hPropfile, &ulSize, sizeof(ULONG), &ulBytesRead, TRUE );
       sRC = ( usDosRC ) ? ERR_READFILE : 0;

       if ( !sRC )
       {
         fOK = UtlAlloc( (PVOID *) &(pITMIda->pstPropItm->pFileNamePool), 0L, ulSize, ERROR_STORAGE );
         if ( fOK )
         {
           pITMIda->pstPropItm->pFileNamePool->ulAlloc = ulSize;
           usDosRC = UtlReadL( hPropfile, &(pITMIda->pstPropItm->pFileNamePool->ulUsed), ulSize-4, &ulBytesRead, TRUE );
           sRC = ( usDosRC ) ? ERR_READFILE : 0;
         }
         else
         {
           sRC = ERR_NOMEMORY;
         } /* endif */
       }
     }
     else
     {
       // file pool has a fixed size, but has a smaller pool header area, so we have to adjust the header part
       USHORT usDiff = sizeof(FILEPOOL) - sizeof(FILEPOOL_VITM3);
       usSize = POOLSIZE;
       fOK = UtlAlloc( (PVOID *) &(pITMIda->pstPropItm->pFileNamePool), 0L, (LONG)(usSize+usDiff), ERROR_STORAGE );
       if ( fOK )
       {
         usDosRC = UtlReadL( hPropfile, ((PBYTE)pITMIda->pstPropItm->pFileNamePool)+usDiff, usSize, &ulBytesRead, TRUE );
         sRC = ( usDosRC ) ? ERR_READFILE : 0;
         if ( !sRC )
         {
           PFILEPOOL_VITM3 pOldPool = (PFILEPOOL_VITM3)(((PBYTE)pITMIda->pstPropItm->pFileNamePool)+usDiff);
           ULONG ulAlloc = pOldPool->usAlloc;
           ULONG ulUsed = pOldPool->usUsed;
           pITMIda->pstPropItm->pFileNamePool->ulAlloc = ulAlloc;
           pITMIda->pstPropItm->pFileNamePool->ulUsed = ulUsed;
         }
       }
       else
       {
         sRC = ERR_NOMEMORY;
       } /* endif */
     }
   } /* endif */

   // close propertyfile
   if ( hPropfile )
   {
      UtlClose( hPropfile, FALSE );
   } /* endif */
   if (sRC )
   {
     /*****************************************************************/
     /* donot use filepair info if error during UTlRead /UtlAlloc     */
     /*****************************************************************/
     if (pITMIda->pstPropItm )
     {
       pITMIda->pstPropItm->usNumFiles = 0;
       if ( pITMIda->pstPropItm->pFileNamePool )
       {
         UtlAlloc( (PVOID *) &(pITMIda->pstPropItm->pFileNamePool), 0L, 0L, ERROR_STORAGE );
       } /* endif */
       if ( pITMIda->pstPropItm->pListFiles )
       {
         UtlAlloc( (PVOID *) &(pITMIda->pstPropItm->pListFiles), 0L, 0L, ERROR_STORAGE );
       } /* endif */
     } /* endif */

   } /* endif */

   return( sRC );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFITMPropWrite
//------------------------------------------------------------------------------
// Function call:     EQFITMPropWrite()
//------------------------------------------------------------------------------
// Description:       Write property      file
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda
//------------------------------------------------------------------------------
// Returncode type:   USHORT usRC
//------------------------------------------------------------------------------
// Returncodes:       0              - success
//                    ERR_OPENFILE   - error opening file
//                    ERR_WRITEFILE  - error writing file
//------------------------------------------------------------------------------
// Function flow:     - open property file
//                    if ok write property struct to property file
//                    if ok, write list of filepairs to propertyfile
//                    free space for list of filepairs
//                    free space for propertystruct
//                    close propertyfile
//------------------------------------------------------------------------------

SHORT EQFITMPropWrite
(
  PITMIDA   pITMIda
)
{
   SHORT       sRC = 0;
   USHORT      usSize;
   USHORT      usDosRC;
   HFILE       hPropfile = (HFILE) NULL;         // handle of propertyfile
   USHORT      usBytesWritten;
   USHORT      usOpenAction;

   /*******************************************************************/
   /* check that we still need to write property file...              */
   /*******************************************************************/
   if ( pITMIda->pstPropItm )
   {
     usDosRC = UtlOpen( pITMIda->szPropName, &hPropfile, &usOpenAction, 0L, FILE_NORMAL, FILE_OPEN | FILE_CREATE, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, FALSE );
     if ( usDosRC )
     {
        hPropfile = (HFILE) NULL;
        sRC = ERR_OPENFILE;
     } /* endif */
     if ( !sRC )
     {
       usDosRC = UtlWrite( hPropfile, pITMIda->pstPropItm, sizeof(PROPITM), &usBytesWritten, FALSE );
       sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
     } /* endif */
     if ( !sRC )
     {
       ULONG ulSize = (pITMIda->pstPropItm->usNumFiles) * sizeof (FILEPAIR);
       if (ulSize )
       {
         ULONG ulBytesWritten = 0;
         usDosRC = UtlWriteL( hPropfile, pITMIda->pstPropItm->pListFiles, ulSize, &ulBytesWritten, FALSE );
         sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
       } /* endif */
     } /* endif */
     if ( !sRC && pITMIda->pstPropItm->pFileNamePool)
     {
       ULONG ulBytesWritten = 0;
       usDosRC = UtlWriteL( hPropfile, pITMIda->pstPropItm->pFileNamePool, pITMIda->pstPropItm->pFileNamePool->ulAlloc, &ulBytesWritten, FALSE );
       sRC = ( usDosRC ) ? ERR_WRITEFILE : 0;
     } /* endif */


     // close propertyfile
     if ( hPropfile )
     {
        UtlClose( hPropfile, FALSE );
     } /* endif */

     if ( pITMIda->pstPropItm->pFileNamePool )
     {
       UtlAlloc( (PVOID *) &(pITMIda->pstPropItm->pFileNamePool), 0L, 0L, ERROR_STORAGE );
     } /* endif */
     if ( pITMIda->pstPropItm->pListFiles )
     {
       UtlAlloc( (PVOID *) &(pITMIda->pstPropItm->pListFiles), 0L, 0L, ERROR_STORAGE );
     } /* endif */

     if ( pITMIda->pstPropItm )
     {
       UtlAlloc( (PVOID *) &pITMIda->pstPropItm, 0L, 0L, ERROR_STORAGE );
     } /* endif */
   } /* endif */
   return( sRC );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFITMFindAliName ( (&szAliName), pITMIda )
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       find next free alifilename from propertyfile contents
//------------------------------------------------------------------------------
// Parameters:        PSZ     pszAliName
//                    PITMIDA pITMIda
//                    PUSHORT pusNum
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       true if number was found (number is always found)
//------------------------------------------------------------------------------
// Function flow:     get ptr to list of filepairs in property struct
//                    while no free number found and not end of list
//                      check if number is used already as extension of an
//                      alifile
//                      if number is not free, increase number
//                    endwhile
//                    build name of free alifile
//                    fill pushort with free number
//------------------------------------------------------------------------------
static BOOL
EQFITMFindAliName
(
  PSZ      pszAliName,
  PITMIDA  pITMIda,
  PUSHORT  pusNum
)
{
  USHORT     usI = 0;
  PFILEPAIR  pFilePair;
  CHAR       szDrive[ MAX_DRIVE ];           // drive
  CHAR       chTempName[ MAX_EQF_PATH ];         // temp. filename
  USHORT     usRc;
  USHORT     usNewAliNum = 1;
  BOOL       fFree;
  BOOL       fOK = TRUE;

  /*******************************************************************/
  /* check if usNewAliNum is free, if not increase usNewAliNum       */
  /*******************************************************************/
  pFilePair = pITMIda->pstPropItm->pListFiles;
  fFree = FALSE;
  while ( !fFree )
  {
    fFree = TRUE;
    usI = 0;
    while ( fFree && (usI < pITMIda->pstPropItm->usNumFiles ))
    {
      if ( usNewAliNum == pFilePair[usI].usAliNum )
      {
        fFree = FALSE;
      } /* endif */
      usI++;
    } /* endwhile */
    if ( !fFree )
    {
      usNewAliNum++;
    } /* endif */
  } /* endwhile */
  /*******************************************************************/
  /* make directory d:\eqf\$$ITMAli if not existent yet              */
  /* check which $$ITMAli.xxx files exist                            */
  /*******************************************************************/
  if ( usNewAliNum < MAX_PROPFILES )
  {
    UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
    UtlMakeEQFPath( chTempName, szDrive[0], SYSTEM_PATH, TEMPDIRSAVE );
    usRc = UtlMkDir (chTempName, 0L, FALSE);
    sprintf(pszAliName, "%s\\$$ITMAli.%3.3d", chTempName, usNewAliNum);
    /****************************************************************/
    /* remember usI for later filling the property structure        */
    /****************************************************************/
    *pusNum = usNewAliNum;
  }
  else
  {
    fOK = FALSE;
  } /* endif */
  return(fOK);
} /* end of function EQFITMFindAliName  */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFITMFillPropAli(pITMIda, usNumofAli )
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       fill Num in Propertyfile to indicate that filepair is
//                    prepared
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda
//                    USHORT    usNum
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     loop thru list of filepairs in propertyfile
//                    if found,add number of alifile in which alignments
//                    of this filepair are saved
//------------------------------------------------------------------------------
static VOID
EQFITMFillPropAli
(
   PITMIDA   pITMIda,
   USHORT    usNum
)
{
  PFILEPAIR  pFilePair;
  USHORT     usI = 0;
  BOOL       fFound = FALSE;
  PFILEPOOL  pPool;
  PBYTE      pTemp;
  BOOL       fOK = TRUE;

  /********************************************************************/
  /* find filepair and add the number of the alifile                  */
  /********************************************************************/
  pFilePair = pITMIda->pstPropItm->pListFiles;
  pPool = pITMIda->pstPropItm->pFileNamePool;
  while ( !fFound && (usI<pITMIda->pstPropItm->usNumFiles ))
  {

    if (pITMIda->szSrcStartPath[0] == EOS )
    {
      if (!stricmp((const char *)((PBYTE)pPool + pFilePair[usI].ulSrcOffset),
                                 pITMIda->chSourceFile)  )
      {
        if (!stricmp((const char *)((PBYTE)pPool + pFilePair[usI].ulTgtOffset),
                                      pITMIda->chTargetFile)  )
        {
          fFound = TRUE;
          pFilePair[usI].usAliNum = usNum;
        } /* endif */
      } /* endif */
    }
    else
    {
      fOK = TRUE;
      pTemp = (PBYTE)pPool + pFilePair[usI].ulSrcOffset;

      memset( pITMIda->szBuffer, 0, sizeof(pITMIda->szBuffer) );
      fOK = EQFBITMAddStartToRelPath(pITMIda->szBuffer,
                                     pITMIda->szSrcStartPath,
                                     (PSZ)pTemp,
                                     sizeof(pITMIda->szBuffer));

      if (fOK && !stricmp(pITMIda->szBuffer, pITMIda->chSourceFile) )
      {
        pTemp = (PBYTE)pPool + pFilePair[usI].ulTgtOffset;

        memset( pITMIda->szBuffer, 0, sizeof(pITMIda->szBuffer) );

        fOK = EQFBITMAddStartToRelPath(pITMIda->szBuffer,
                                       pITMIda->szTgtStartPath,
                                       (PSZ)pTemp,
                                       sizeof(pITMIda->szBuffer));
        if (fOK && !stricmp(pITMIda->szBuffer, pITMIda->chTargetFile) )
        {
          fFound = TRUE;
          pFilePair[usI].usAliNum = usNum;
        } /* endif */
      } /* endif */
    } /* endif */
    usI++;
  } /* endwhile */
} /* end of function EQFITMFillPropAli */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     FillPropFilePairs
//------------------------------------------------------------------------------
// Function call:     FillPropFilePairs(pITMIda)
//------------------------------------------------------------------------------
// Description:       fill list of filepairs in property structure
//                    from list of pairs in ITMIda
//------------------------------------------------------------------------------
// Parameters:        PITMIDa   pITMIda
//------------------------------------------------------------------------------
// Returncode type:   void
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     alloc space for list of filepairs
//                    loop thru the list of filenames
//                      copy filename as srcfilename in propertystruct
//                      copy next filename as tgtfilename into propstruct
//                      compare filepair with old filepairlist
//                      if it is contained there and is prepared, copy number
//                      of continuation file to new filepair list
//                    end of loop
//                    free old filepairlist
//                    set propstruct ptr to new filepair list
//------------------------------------------------------------------------------
BOOL
FillPropFilePairs
(
    PITMIDA       pITMIda
)
{
  USHORT     usI=0, usJ, usNum;
  PFILEPAIR  pOldFilePair = NULL;
  PFILEPAIR  pFilePair = NULL;
  PSZ        * ppListIndex;                      // pointer to file array
  BOOL       fOK = TRUE;
  ULONG      ulSize;
  USHORT     usPairNum= 0;
  BOOL       fFound = FALSE;
  PFILEPOOL  pNewPool = NULL;
  PFILEPOOL  pOldPool = NULL;

   ppListIndex = pITMIda->ppArgv;
   usNum = pITMIda->usArgc;

   ulSize = (usNum  / 2 ) * sizeof (FILEPAIR);
   fOK = UtlAlloc( (PVOID *) &pFilePair, 0L, ulSize, ERROR_STORAGE );
   if ( fOK )
   {
     fOK =  UtlAlloc( (PVOID *)&pNewPool, 0L, (LONG) POOLSIZE, ERROR_STORAGE );
   } /* endif */

   if ( fOK )
   {
     pNewPool->ulUsed = 2* sizeof(ULONG);
     pNewPool->ulAlloc = POOLSIZE;

     pOldFilePair = pITMIda->pstPropItm->pListFiles;
     pOldPool = pITMIda->pstPropItm->pFileNamePool;

     usI = 0;
   } /* endif */

   while ( fOK && (usI < usNum ))
   {
     pFilePair[usPairNum].ulSrcOffset = pNewPool->ulUsed;

     fOK = ITMPoolAddData( *ppListIndex, &pNewPool );
     if ( !fOK )
     {
       pFilePair[usPairNum].ulSrcOffset = 0;
       ITMUtlError(pITMIda, ITM_TOOMANYFILES, MB_CANCEL, 0, NULL, EQF_ERROR);
     } /* endif */

     ppListIndex ++;

     pFilePair[usPairNum].ulTgtOffset = pNewPool->ulUsed;

     fOK = ITMPoolAddData(*ppListIndex, &pNewPool );
     if ( !fOK )
     {
       pFilePair[usPairNum].ulTgtOffset = 0;
       ITMUtlError(pITMIda, ITM_TOOMANYFILES, MB_CANCEL, 0, NULL, EQF_ERROR);
     } /* endif */

     ppListIndex ++;

     /********************************************************************/
     /* check whether filepair is in other alifilepairlist;              */
     /* if so do copy number of alifile                                  */
     /********************************************************************/
     usJ = 0;
     fFound = FALSE;

     while ( fOK && !fFound && (usJ < pITMIda->pstPropItm->usNumFiles) )
     {
       if (!stricmp((const char *)((PBYTE)pOldPool + pOldFilePair[usJ].ulSrcOffset),
                   (const char *)((PBYTE)pNewPool + pFilePair[usPairNum].ulSrcOffset))  )
       {
         if (!stricmp((const char *)((PBYTE)pOldPool + pOldFilePair[usJ].ulTgtOffset),
                     (const char *)((PBYTE)pNewPool +  pFilePair[usPairNum].ulTgtOffset))  )
         {
           fFound = TRUE;
           pFilePair[usPairNum].usAliNum = pOldFilePair[usJ].usAliNum;
         }
       } /* endif */
       usJ ++;
     } /* endwhile */
     usPairNum++;
     usI += 2;
   } /* endwhile */


   if ( fOK )
   {
     UtlAlloc( (PVOID *) &(pITMIda->pstPropItm->pListFiles), 0L, 0L, ERROR_STORAGE );
     UtlAlloc( (PVOID *) &(pITMIda->pstPropItm->pFileNamePool), 0L, 0L, ERROR_STORAGE );

     pITMIda->pstPropItm->pListFiles = pFilePair;
     pITMIda->pstPropItm->usNumFiles = usPairNum;
     pITMIda->pstPropItm->pFileNamePool = pNewPool;
   }
   else
   {
     if ( pFilePair )
     {
       UtlAlloc( (PVOID *) &pFilePair, 0L, 0L, ERROR_STORAGE );
     } /* endif */
     if ( pNewPool )
     {
       UtlAlloc( (PVOID *)&pNewPool, 0L, 0L, ERROR_STORAGE );
     } /* endif */
     pITMIda->pstPropItm->usNumFiles = 0;
   } /* endif */
   return (fOK);
} /* end of function FillPropFilePairs */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     CheckDoubleFilePairs
//------------------------------------------------------------------------------
// Function call:     CheckDoubleFilePairs(&ppListindex)
//------------------------------------------------------------------------------
// Description:       go thru filelist and delete filepairs which occur more
//                    than once
//------------------------------------------------------------------------------
// Parameters:        PSZ  **pppListIndex
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     alloc space for new list
//                    get ptr to 1st src/tgt filenames
//                    while not at end of list
//                      while not equal to a previous pair and not all compared
//                        compare src and tgt filenames
//                      endwhile
//                      if not equal to a previous pair
//                       add ptr to src&tgtfilenames to new list
//                      get ptr to next src/tgt filenames
//                    endwhile
//                    if ok free old list of filenames
//                    set Listpointer to new list
//------------------------------------------------------------------------------
BOOL CheckDoubleFilePairs
(
  PSZ **pppListIndex                   // pointer to list array
)
{
  BOOL fOK = TRUE;
  int i = 0;                          // index in list pointer array
  int j = 0;                          // index in list pointer array
  int l = 0;                          // index in list pointer array
  PSZ  pSrcTok;                         // pointer to tokens
  PSZ  pTgtTok;                         // pointer to tokens
  PSZ  * ppNewList = NULL;              // pointer to list token array
  PSZ  * ppOldList;                              // pointer to list token array
  BOOL  fEqualToPrev = FALSE;

  // count number of entries in list
  int iFiles = 0;
  ppOldList = *pppListIndex;
  while( ppOldList[iFiles] ) iFiles++;

   // allocate memory for file name pointers
   fOK = UtlAlloc( (PVOID *) (PVOID *)&ppNewList, 0L, (iFiles + 10) * sizeof(PSZ),
                   ERROR_STORAGE );
   if ( fOK )
   {
     ppOldList = *pppListIndex;
     pSrcTok =  ppOldList[i++];
     pTgtTok =  ppOldList[i++];

     while ( pSrcTok && pTgtTok && (i <= iFiles) )
     {
       l = 0;
       fEqualToPrev = FALSE;
       while ( !fEqualToPrev  && (l < j ))
       {
         if (!stricmp(ppNewList[l], pSrcTok) )
         {
           if (!stricmp(ppNewList[l+1], pTgtTok))
           {
             fEqualToPrev = TRUE;
           }
         } /* endif */
         l += 2;         // point to next pair
       } /* endwhile */
       /***************************************************************/
       /* if not equal to a previous pair                             */
       /***************************************************************/
       if (!fEqualToPrev)
       {
         ppNewList[j++] = pSrcTok;
         ppNewList[j++] = pTgtTok;
       }
       pSrcTok =  ppOldList[i++];
       pTgtTok =  ppOldList[i++];
     } /* endwhile */

     if ( (pSrcTok != NULL) && (pTgtTok == NULL)  )
     {
       // uneven number of file specified, file list is not usable
       fOK = FALSE;
     }

   } /* endif */
   if ( fOK )
   {
     UtlAlloc( (PVOID *) (PVOID *)&ppOldList, 0L, 0L, ERROR_STORAGE);
     *pppListIndex = ppNewList;
   } /* endif */
   return( fOK );
} /* end of function CheckDoubleFilePairs */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     IsPrepared
//------------------------------------------------------------------------------
// Function call:     IsPrepared(pITMIda, pusNum)
//------------------------------------------------------------------------------
// Description:       Check whether filepair is already prepared
//                    If so return Number of ext of alifile in pusNum
//------------------------------------------------------------------------------
// Parameters:        PITMIDA     pITMIda
//                    PUSHORT     pusNum
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     if already prepared
//                    FALSE    if not prepared
//------------------------------------------------------------------------------
// Function flow:     while not found & not at end of filepairlist in propstr.
//                      compare src and tgt filename with current filenames
//                      in ITMIDa
//                      if found set pusNum to AliNum of that entry
//------------------------------------------------------------------------------
BOOL
IsPrepared
(
  PITMIDA     pITMIda,
  PUSHORT     pusNum
)
{
  USHORT      usJ = 0;
  BOOL        fFound = FALSE;
  PFILEPAIR   pFilePair;
  PFILEPOOL   pPool;
  BOOL        fOK = TRUE;
  PSZ         pBuf;
  PSZ         pSrcStartPath = NULL;
  PSZ         pTgtStartPath = NULL;

  *pusNum = 0;                        // default if not prepared

  pFilePair = pITMIda->pstPropItm->pListFiles;
  pPool = pITMIda->pstPropItm->pFileNamePool;
  pSrcStartPath = (PCHAR)&pITMIda->pstPropItm->szSrcStartPath;
  pTgtStartPath = (PCHAR)&pITMIda->pstPropItm->szTgtStartPath;

  while ( !fFound && (usJ < pITMIda->pstPropItm->usNumFiles) )
  {
    if (*pSrcStartPath == EOS )
    {
      if (!stricmp((const char *)((PBYTE)pPool + pFilePair[usJ].ulSrcOffset),
                  pITMIda->chSourceFile)  )
      {
        if (!stricmp((const char *)((PBYTE)pPool + pFilePair[usJ].ulTgtOffset),
                     pITMIda->chTargetFile)  )
        {
          if ( pFilePair [usJ].usAliNum )
          {
            fFound = TRUE;
            *pusNum = pFilePair[usJ].usAliNum;
          } /* endif */
        } /* endif */
      } /* endif */
    }
    else
    {
      pBuf = pITMIda->szBuffer;
      memset(pITMIda->szBuffer, 0, sizeof(pITMIda->szBuffer) );
      fOK = EQFBITMAddStartToRelPath( pBuf, pSrcStartPath,
                            (PSZ)((PBYTE)pPool + pFilePair[usJ].ulSrcOffset),
                            sizeof(pITMIda->szBuffer) );
      if (fOK && !stricmp(pBuf, pITMIda->chSourceFile)  )
      {
        memset(pITMIda->szBuffer, 0, sizeof(pITMIda->szBuffer) );
        fOK = EQFBITMAddStartToRelPath( pBuf, pTgtStartPath,
                              (PSZ)((PBYTE)pPool + pFilePair[usJ].ulTgtOffset),
                              sizeof(pITMIda->szBuffer) );

        if (fOK && !stricmp(pBuf, pITMIda->chTargetFile)  )
        {
          if ( pFilePair [usJ].usAliNum )
          {
            fFound = TRUE;
            *pusNum = pFilePair[usJ].usAliNum;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
    usJ ++;
  } /* endwhile */

  return(fFound);
} /* end of function IsPrepared */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFITMDelAli
//------------------------------------------------------------------------------
// Function call:     EQFITMDelAli(pITMIda)
//------------------------------------------------------------------------------
// Description:       delete alifile and reference to it in propertyfile
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     loop thru list of filepairs in propstruct and
//                      set aliNum to 0
//                    if alifile found
//                       delete alifile
//------------------------------------------------------------------------------
VOID
EQFITMDelAli
(
  PITMIDA   pITMIda
)
{
  PFILEPAIR  pFilePair;
  USHORT     usI = 0;
  BOOL       fFound = FALSE;
  CHAR       szAliName[MAX_EQF_PATH];         // buffer for profile file name
  CHAR       szDrive[ MAX_DRIVE ];               // drive
  PFILEPOOL  pPool;
  PBYTE      pTemp = NULL;
  BOOL       fOK = FALSE;

  /********************************************************************/
  /* find filepair and set the number of the alifile  to 0            */
  /********************************************************************/
  if ( pITMIda->usNumPrepared )
  {
    pFilePair = pITMIda->pstPropItm->pListFiles;
    pPool = pITMIda->pstPropItm->pFileNamePool;
    while ( !fFound && (usI<pITMIda->pstPropItm->usNumFiles ))
    {
      if (pITMIda->szSrcStartPath[0] == EOS )
      {
        if (!stricmp((const char *)((PBYTE)pPool + pFilePair[usI].ulSrcOffset),
                              pITMIda->chSourceFile)  )
        {
          if (!stricmp((const char *)((PBYTE)pPool + pFilePair[usI].ulTgtOffset),
                              pITMIda->chTargetFile)  )
          {
            fFound = TRUE;
            pFilePair[usI].usAliNum = 0;
          } /* endif */
        } /* endif */
      }
      else
      {
         fOK = TRUE;
         pTemp = (PBYTE)pPool + pFilePair[usI].ulSrcOffset;

         memset( pITMIda->szBuffer, 0, sizeof(pITMIda->szBuffer) );
         fOK = EQFBITMAddStartToRelPath(pITMIda->szBuffer,
                                        pITMIda->szSrcStartPath,
                                        (PSZ)pTemp,
                                        sizeof(pITMIda->szBuffer));

         if (fOK && !stricmp(pITMIda->szBuffer, pITMIda->chSourceFile) )
         {
           pTemp = (PBYTE)pPool + pFilePair[usI].ulTgtOffset;

           memset( pITMIda->szBuffer, 0, sizeof(pITMIda->szBuffer) );

           fOK = EQFBITMAddStartToRelPath(pITMIda->szBuffer,
                                          pITMIda->szTgtStartPath,
                                          (PSZ)pTemp,
                                          sizeof(pITMIda->szBuffer));
           if (fOK && !stricmp(pITMIda->szBuffer, pITMIda->chTargetFile) )
           {
             fFound = TRUE;
             pFilePair[usI].usAliNum = 0;
           } /* endif */
         } /* endif */

      }
      usI++;
    } /* endwhile */
  } /* endif */
  /********************************************************************/
  /* delete alifile with extension usNum                              */
  /********************************************************************/
  if ( fFound)
  {
    UtlQueryString(QST_PRIMARYDRIVE, szDrive, MAX_DRIVE) ;
    UtlMakeEQFPath( szAliName, szDrive[0], SYSTEM_PATH, TEMPDIRSAVE );
    sprintf(szAliName, "%s\\$$ITMAli.%3.3d", szAliName,
               pITMIda->usNumPrepared);
    UtlDelete(szAliName, 0L, FALSE);
    pITMIda->usNumPrepared = 0;
  } /* endif */

} /* end of function EQFITMDelAli */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFITMDelFilePairList
//------------------------------------------------------------------------------
// Function call:     EQFITMDelFilePairList(pITMIda, usNumPrepared)
//------------------------------------------------------------------------------
// Description:       delete filepair in filepairlist
//                    (is called if alignment is written to memory)
//------------------------------------------------------------------------------
// Parameters:        PITMIDA   pITMIda,
//                    USHORT    usNumPrepared
//------------------------------------------------------------------------------
// Returncode type:   void
//------------------------------------------------------------------------------
// Returncodes:       none
//------------------------------------------------------------------------------
// Function flow:     alloc new space for filepairlist
//                    copy all filepairs to new list except the specified
//                    free old filepairlist
//                    set ptr to new filepairlist
//------------------------------------------------------------------------------
VOID
EQFITMDelFilePairList
(
   PITMIDA   pITMIda,
   USHORT    usNumPrepared
)
{
  PFILEPAIR  pFilePair = NULL;
  PFILEPAIR  pNewFilePair = NULL;
  USHORT     usI = 0;
  USHORT     usJ = 0;
  BOOL       fFound = FALSE;
  USHORT     usSize;
  BOOL       fOK;
  PFILEPOOL  pPool;
  PBYTE      pTemp = NULL;

  if ( usNumPrepared )
  {
    usSize = (pITMIda->pstPropItm->usNumFiles - 1) * sizeof (FILEPAIR);
    fOK = UtlAlloc( (PVOID *) &pNewFilePair, 0L,
                     (LONG) usSize, ERROR_STORAGE );
/**********************************************************************/
/* rearranging FilePairList nec only if more than one filepair        */
/**********************************************************************/
    if ( fOK && usSize)
    {
      pFilePair = pITMIda->pstPropItm->pListFiles;
      pPool = pITMIda->pstPropItm->pFileNamePool;
      while ( usI<pITMIda->pstPropItm->usNumFiles )
      {
        if (pITMIda->szSrcStartPath[0] == EOS )
        {
          if (!stricmp((const char *)((PBYTE)pPool + pFilePair[usI].ulSrcOffset),
                             pITMIda->chSourceFile)  )
          {
            if (!stricmp((const char *)((PBYTE) pPool + pFilePair[usI].ulTgtOffset),
                               pITMIda->chTargetFile)  )
            {
              fFound = TRUE;
            } /* endif */
          } /* endif */
        }
        else
        {
           fOK = TRUE;
           pTemp = (PBYTE)pPool + pFilePair[usI].ulSrcOffset;

           memset( pITMIda->szBuffer, 0, sizeof(pITMIda->szBuffer) );
           fOK = EQFBITMAddStartToRelPath(pITMIda->szBuffer,
                                          pITMIda->szSrcStartPath,
                                          (PSZ)pTemp,
                                          sizeof(pITMIda->szBuffer));

           if (fOK && !stricmp(pITMIda->szBuffer, pITMIda->chSourceFile) )
           {
             pTemp = (PBYTE)pPool + pFilePair[usI].ulTgtOffset;

             memset( &pITMIda->szBuffer[0], 0, sizeof(pITMIda->szBuffer) );

             fOK = EQFBITMAddStartToRelPath(pITMIda->szBuffer,
                                            pITMIda->szTgtStartPath,
                                            (PSZ)pTemp,
                                            sizeof(pITMIda->szBuffer));
             if (fOK && !stricmp(pITMIda->szBuffer, pITMIda->chTargetFile) )
             {
               fFound = TRUE;
             } /* endif */
           } /* endif */

        } /* endif */

        if ( !fFound && usSize && (usJ+1 < pITMIda->pstPropItm->usNumFiles) )
        {
          pNewFilePair[usJ].ulSrcOffset = pFilePair[usI].ulSrcOffset;
          pNewFilePair[usJ].ulTgtOffset = pFilePair[usI].ulTgtOffset;
          pNewFilePair[usJ].usAliNum = pFilePair[usI].usAliNum;
          usJ++;
        } /* endif */
        fFound = FALSE;
        usI++;
      } /* endwhile */
    } /* endif */
    UtlAlloc( (PVOID *)&pITMIda->pstPropItm->pListFiles, 0L, 0L, NOMSG );
    pITMIda->pstPropItm->pListFiles = pNewFilePair;
    pITMIda->pstPropItm->usNumFiles  = usJ;
  } /* endif */


} /* end of function EQFITMDelFilePairList(pITMIda) */
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     FillItmHeader
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       fill ITMHEADER struct
//------------------------------------------------------------------------------
// Parameters:        PITMIDA  pITMIda,
//                    PITMHEADER pCurItmHeader
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       0 if successful
//                    else if error occurred
//------------------------------------------------------------------------------
// Function flow:     fill header struct of alifile with
//                    - prefix description
//                    - version number
//                    - date and time of srcfile
//                    - date and time of tgtfile
//                    - date and time of tagtable
//------------------------------------------------------------------------------

static USHORT
FillItmHeader
(
  PITMIDA  pITMIda,
  PITMHEADER pCurItmHeader
)
{
   USHORT  usDosRC;                                        // return code of Dos... alias Utl...
   CHAR    szBuf1[ MAX_PATH144];        // temp buffer ...
   CHAR    szBuf2[ MAX_PATH144];        // temp buffer ...

   strcpy(pCurItmHeader->szPrefix, ITMALIPREFIX);
   pCurItmHeader->usVersion = ITMVERSION3;

   usDosRC = ITMGetFileDateTime(pITMIda->chSourceFile,
                                &(pCurItmHeader->fSrcDate),
                                &(pCurItmHeader->fSrcTime));
   if ( !usDosRC )
   {
     usDosRC = ITMGetFileDateTime(pITMIda->chTargetFile,
                                  &(pCurItmHeader->fTgtDate),
                                  &(pCurItmHeader->fTgtTime));
   } /* endif */
   if ( !usDosRC )
   {
    /******************************************************************/
    /* fill in name of tagtable and pathes                            */
    /******************************************************************/
    UtlMakeEQFPath( szBuf1, NULC, TABLE_PATH, NULL);
    sprintf( szBuf2, PATHCATFILECATEXTENSION, szBuf1,
               pITMIda->chTagTableName,
               EXT_OF_FORMAT );
    usDosRC = ITMGetFileDateTime(szBuf2, &(pCurItmHeader->fTagDate),
                                   &(pCurItmHeader->fTagTime));
   } /* endif */

   return(usDosRC);
} /* end of function FillItmHeader */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMGetFileDateTime
//------------------------------------------------------------------------------
// Function call:     ITMGetFileDateTime(pFileName, pFileDate, pFileTime)
//------------------------------------------------------------------------------
// Description:       fill structs with date and time of specified file
//------------------------------------------------------------------------------
// Parameters:        PSZ  pFileName,
//                    PFDATE  pFileDate,
//                    PFTIME  pFileTime
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       0 if ok
//                    other if date/time is not available
//------------------------------------------------------------------------------
// Function flow:     use UtlFindFirst to get date and time of file
//------------------------------------------------------------------------------
static USHORT
ITMGetFileDateTime
(
   PSZ  pFileName,
   PFDATE  pFileDate,
   PFTIME  pFileTime
)
{
   HDIR    hDirHandle = HDIR_CREATE;    // DosFind routine handle
   USHORT  usDosRC;                     // return code of Dos... alias Utl...

   LONGFILEFIND    ResultBuf;           // DOS file find struct

   usDosRC = UtlFindFirstLong( pFileName, &hDirHandle,
                               FILE_NORMAL,
                               &ResultBuf, FALSE );
   UtlFindCloseLong( hDirHandle, FALSE );
   if ( !usDosRC )
   {
     *pFileDate = ResultBuf.fdateLastWrite;
     *pFileTime = ResultBuf.ftimeLastWrite;
   } /* endif */

  return(usDosRC);
} /* end of function ITMGetFileDateTime */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMCompFileDateTime
//------------------------------------------------------------------------------
// Function call:     ITMCompFileDateTime(pFileName, pFileDate, pFileTime )
//------------------------------------------------------------------------------
// Description:       compare stored date&time of file with actual
//------------------------------------------------------------------------------
// Parameters:        PSZ         pFileName
//                    PFDATE      pFileDate
//                    PFTIME      pFileTime
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE      date and time is equal
//                    FALSE     file is updated
//------------------------------------------------------------------------------
// Function flow:     get actual date/time of file
//                    compare date, compare time
//                    if not both equal, return FALSE
//                    else return TRUE
//------------------------------------------------------------------------------
static BOOL
ITMCompFileDateTime
(
   PSZ         pFileName,
   PFDATE      pFileDate,
   PFTIME      pFileTime
)
{
  USHORT  usDosRC;
  BOOL    fOK = TRUE;
  FDATE   fNewFileDate;
  FTIME   fNewFileTime;
  LONG    lCompare = 0;

  /******************************************************************/
  /* check whether date and time of src/tgtfile match               */
  /******************************************************************/
  usDosRC = ITMGetFileDateTime(pFileName, &fNewFileDate, &fNewFileTime);
  if ( !usDosRC )
  {
    lCompare = UtlCompareDate( &fNewFileDate, pFileDate );
    if ( lCompare == 0 )
    {
      lCompare = UtlCompareTime( &fNewFileTime, pFileTime );
    } /* endif */
  } /* endif */
  if (usDosRC || (lCompare != 0))
  {
    fOK = FALSE;
  }
  return(fOK);
} /* end of function ITMCompFileDateTime */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     ITMPoolAddData
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------
static
BOOL
ITMPoolAddData
(
  PSZ         pData,
  PFILEPOOL   *ppFilePool
)
{
  ULONG      ulLen = 0;
  BOOL       fOK = TRUE;
  PBYTE      pDataInPool;
  PFILEPOOL pFilePool = *ppFilePool;

  ulLen = strlen(pData) + 1;

  // enlarge pool when necessary
  if ( pFilePool->ulUsed + ulLen >= pFilePool->ulAlloc )
  {
    ULONG ulNewSize = pFilePool->ulAlloc + POOLSIZE;
    fOK =  UtlAlloc( (PVOID *)&pFilePool, pFilePool->ulAlloc, ulNewSize, ERROR_STORAGE );
    if ( fOK )
    {
      pFilePool->ulAlloc = ulNewSize;
    }
  } /* endif */

  if ( fOK )
  {
    pDataInPool = (PBYTE)pFilePool + pFilePool->ulUsed;
    memcpy(pDataInPool, pData, ulLen);
    pFilePool->ulUsed = pFilePool->ulUsed + ulLen;
  } /* endif */

  *ppFilePool = pFilePool;

  return (fOK);
} /* end of function ITMPoolAddData */

