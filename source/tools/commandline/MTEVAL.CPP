//+----------------------------------------------------------------------------+
//| MTEval.CPP                                                                 |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2016, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//| Description: Evaluates the MT log info of folders                          |
//+----------------------------------------------------------------------------+

extern "C"
{
  #define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_TM               // general Transl. Memory functions
  #define INCL_EQF_TP               // public translation processor functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_ASD              // dictionary access functions (Asd...)
  #define INCL_EQF_EDITORAPI        // editor API
  #define INCL_EQF_FOLDER           // folder list and document list functions

#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                  // General .H for EQF
  #include <eqfdde.h>               // batch mode definitions
  #include <eqffol00.h>             // Private folder defines
  #include "eqfutpck.h"             // FXP package header file
  #include "eqfhlog.h"                   // defines and structures of history log
  #include "eqftpi.h"  
  #include "eqfdde.h"
  #include "eqfserno.h"  
  #include "eqfrpt00.h"   // private include file for report
  #include "eqfrpt01.h"   // private include file for report
  #include "eqfrpt.h"
  #include "eqfrpt.mri"   // private mri's for report
  #include <time.h>
  int qLogCompare( const void *p1, const void *p2 );

#pragma pack( pop, TM2StructPacking )

}

#include "cxmlwriter.h"
#include "OTMCFXP.h"


// the MT log evaluation class
class CMTEval
{
public:
	CMTEval();
	~CMTEval();
  int Evaluate( const char *szFileSpec, const char *strOutFile );
  void SetTestMode( BOOL fTest );
  void SetDetailsMode( BOOL fDetails );
protected:
  // private data types 
  typedef enum { LONGFOLDERNAME, SHORTFOLDERNAME, EXPORTEDFOLDER, DIRECTORY, INVFILESPEC, SEARCHPATTERN } InputFileMode;

  // protected methods
  InputFileMode CheckInFile( const char *pszInFile, BOOL fNameCheckOnly = FALSE );
  int MakeShortFolderName( const char *pszLongName, char *pszShortName );
  int EvaluateFolder( const char *pszShortName );
  int EvaluatePackage( const char *pszPackage );
  int EvaluateFolderProps( PPROPFOLDER pProp );  
  int EvaluateLogFile( PPROPDOCUMENT pProp, PVOID pvMTLog, int iLogFileSize );
  int ScanDirAndEvaluate( const char *szSearchPattern, BOOL fScanSubDirs  );
  int LoadFileFromPackage( CFXP *pPackage, long lToken, void **pvBuf, int *piSize = NULL  );
  int LoadFileFromDisk( char *pszFile, void **pvBuf, int *piSize = NULL  );
  int CategoryOfSegment( int iWords );
  int WriteAverageTime( ULONG ulSegs[3], ULONG ulTime[3] );
  int WriteNumbers( const char *pszName, const char *type, ULONG ulCounts[3] );
  int WriteTime( const char *pszName, ULONG ulTime );
  int WriteTimePerCategory( const char *pszName, const char *type, ULONG ulCounts[3] );
  int AddNumbers( ULONG ulCountSum[3], ULONG ulCountAdd[3] );
  int CopyNumbers( ULONG ulCountTo[3], ULONG ulCountFrom[3] );
  int ClearNumbers( ULONG ulCount[3] );
  int AddFolderNumbersToTotal( void );
  int WriteFolderNumbers( void );
  int WriteTotals( void );
  int WriteMTMetaData( CXmlWriter *pWriter, PSZ_W pszMetaData );
  BOOL CheckNewRecord( PACTSEGLOGENH pEntry  );
  BOOL CheckNewRecord2( PACTSEGLOGENH2 pEntry  );
  BOOL CheckOldRecord( PACTSEGLOGOLD pEntry  );
  int CheckRecords( PVOID pvMTLog, int iLogFileSize );
  // protected instance data
  InputFileMode m_InpFileMode ;
  char m_szFolderShortName[1024];
  char m_szSearchPattern[1024];        
  char m_szErrorMessage[256];          // buffer for error message texts
  char m_szFileSpec[256];              // buffer for temporary file names
  char m_szPathBuffer[256];            // buffer for temporary file names
  char m_szBuffer[1024];               // general purpose buffer
  CXmlWriter *m_xw;                    // XML writer instance

  // counts of current folder
  ULONG m_ulFolMTTotalSegs[3];
  ULONG m_ulFolMTTotalWords[3];
  ULONG m_ulFolMTSendSegs[3];
  ULONG m_ulFolMTSendWords[3];
  ULONG m_ulFolMTReceivedSegs[3];
  ULONG m_ulFolMTReceivedWords[3];
  ULONG m_ulFolMTExistSegs[3];
  ULONG m_ulFolMTExistWords[3];
  ULONG m_ulFolMTUsedSegs[3];
  ULONG m_ulFolMTUsedWords[3];
  ULONG m_ulFolMTSegs[3];
  ULONG m_ulFolMTTime[3];
  int m_iDocsInFolder;

  // counts over all folders
  ULONG m_ulSumMTTotalSegs[3];
  ULONG m_ulSumMTTotalWords[3];
  ULONG m_ulSumMTSendSegs[3];
  ULONG m_ulSumMTSendWords[3];
  ULONG m_ulSumMTReceivedSegs[3];
  ULONG m_ulSumMTReceivedWords[3];
  ULONG m_ulSumMTExistSegs[3];
  ULONG m_ulSumMTExistWords[3];
  ULONG m_ulSumMTUsedSegs[3];
  ULONG m_ulSumMTUsedWords[3];
  ULONG m_ulSumMTSegs[3];
  ULONG m_ulSumMTTime[3];
  int m_iFoldersProcessed;

  // test mode defines
  FILE *m_hfTest;

  BOOL m_fDetails;

  // record pointer array variables
  int m_iRecords;             // number of records
  int m_iAllocatedPointers;   // size of pointer array in number of entries
  PVOID *m_pRecords;       // pointers to loaded records

  // data of currently processed MTLOG entry
  ACTSEGLOGENH2 m_ActSegLog;           // fixed part of MT log entry
  CHAR_W        m_szSegSource[MAX_SEGMENT_SIZE+1]; // segment source text
  CHAR_W        m_szMetaData[MAX_SEGMENT_SIZE+1]; // segment meta data
  char          m_szPropDoc[MAX_LONGFILESPEC];      // proposal document 
  PEDATABUFFER  m_PEData;                          // PE data buffer
};

char *pszInFile = NULL;
char *pszOutFile = NULL;

static void showHelp();

int main
(
  int argc,
  char *argv[],
  char *envp[]
)
{
  int iRC = 0;
  BOOL fLog = FALSE;
  BOOL fDetails = FALSE;

  envp;
  // check if need to show help information
  if(argc==2 && stricmp(argv[1],"-h")==0 )
  {
     showHelp();
     return 0;
  }

  //fLog = TRUE;   // always log during development...
  
  // get command line options
  printf( "MT Information Evaluation Tool V%d.%d.%d.%d\n", EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE, EQF_DRIVER_SUBRELEASE, EQF_DRIVER_BUILD );

  // Skip first commandline argument (program name)                   
  argc--;
  argv++;

  // Check commandline arguments                                      
  pszInFile = NULL;
  while ( !iRC && argc )
  {
    PSZ pszArg;

    pszArg = *argv;
    if ( (pszArg[0] == '-') || (pszArg[0] == '/') )
    {
      // Check for options                                            
      if ( stricmp( pszArg + 1, "LOG" ) == 0 )
      {
        fLog = TRUE;
      }
      else if ( stricmp( pszArg + 1, "DETAILS" ) == 0 )
      {
        fDetails = TRUE;
      }
      //else if ( strnicmp( pszArg + 1, "PROFILE=", 8 ) == 0 )
      //{
      //  pszProfile = pszArg + 9;
      //}
      else
      {
        printf( "Warning: Unknown commandline switch '%s' is ignored.\n", pszArg );
      } /* endif */
    }
    else
    {
      // Treat as file name                                           
      if ( pszInFile == NULL )
      {
        pszInFile = pszArg;
      }
      else if ( pszOutFile == NULL )
      {
        pszOutFile = pszArg;
      }
      else
      {
        // Invalid option or superfluous file name ...               
        printf( "Warning: Superfluous commandline value '%s' is ignored.\n", pszArg );
      } /* endif */
    } /* endif */
    argc--;
    argv++;
  } /* endwhile */

  // leave if no input file given
  if ( pszInFile == NULL )
  {
    printf( "Error: Missing name of input file.\n\n" );
    showHelp();
    printf( " \n" );
    return( -1 );
  } /* endif */

  // use default output file if none given
  if ( pszOutFile == NULL )
  {
    pszOutFile = "C:\\MTEVAL.XML";
    printf( "No outfile specified, using %s as default.\n", pszOutFile );
  } /* endif */


  // create MT evaluator and start evaluation
  CMTEval *evaluator = new CMTEval();
  if ( evaluator )
  {
    evaluator->SetTestMode( fLog );
    evaluator->SetDetailsMode( fDetails );
    iRC = evaluator->Evaluate( pszInFile, pszOutFile );
  } /* endif */

  return( iRC );
} /* end of main */


// constructor
CMTEval::CMTEval()
{
  m_hfTest = NULL;
  m_iRecords = 0; 
  m_iAllocatedPointers = 0;
  m_pRecords = NULL;
}

// destructor
CMTEval::~CMTEval()
{
  if ( m_hfTest ) fclose( m_hfTest );
}

// set test mode
void CMTEval::SetTestMode( BOOL fTest )
{
  if ( m_hfTest ) fclose( m_hfTest );

  if ( fTest )
  {
    m_hfTest = fopen( "C:\\MTEVAL.LOG", "a" );
  } /* endif */
}

// set details mode
void CMTEval::SetDetailsMode( BOOL fDetails )
{
  m_fDetails = fDetails;
}

// MT info evaluation
int CMTEval::Evaluate
( 
  const char *pszFileSpec, 
  const char *pszOutFile 
)
{
  int iRC = 0;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::Evaluate( \"%s\", \"%s\" )\n", pszFileSpec, pszOutFile );

  // clear total fields
  m_iFoldersProcessed = 0;
  ClearNumbers( m_ulSumMTTotalSegs );
  ClearNumbers( m_ulSumMTTotalWords );
  ClearNumbers( m_ulSumMTSendSegs );
  ClearNumbers( m_ulSumMTSendWords );
  ClearNumbers( m_ulSumMTReceivedSegs );
  ClearNumbers( m_ulSumMTReceivedWords );
  ClearNumbers( m_ulSumMTExistSegs );
  ClearNumbers( m_ulSumMTExistWords );
  ClearNumbers( m_ulSumMTUsedSegs );
  ClearNumbers( m_ulSumMTUsedWords );
  ClearNumbers( m_ulSumMTTime );
  ClearNumbers( m_ulSumMTSegs );

  // create XML writer instance for output
  m_xw = new CXmlWriter();
  if ( m_xw )
  {
    m_xw->SetFileName( pszOutFile );
    m_xw->Formatting = CXmlWriter::Indented;
    m_xw->Indention = 4;
    m_xw->Encoding = CXmlWriter::UTF8;
  }
  else
  {
    iRC = ERROR_WRITE_FAULT;
  } /* endif */

  if ( !iRC )
  {
    CHAR szVersion[20];

    if ( !m_xw->WriteStartDocument() )
    {
      printf( "Error: could not write to output file %s\n", pszOutFile );
      return( ERROR_WRITE_FAULT );
    }

    m_xw->WriteStylesheet( "MTEVAL.XSL" );
    m_xw->WriteStartElement( "mtevaluation" );
    m_xw->WriteAttributeString( "xmlns:PE", "http://www.ibm.com/PostEdit" );
    m_xw->WriteStartElement( "header" );
    m_xw->WriteElementString( "generator", "MTEVAL" );
    sprintf( szVersion, "%d.%d.%d.%d", EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE, EQF_DRIVER_SUBRELEASE, EQF_DRIVER_BUILD );
    m_xw->WriteElementString( "generatorversion", szVersion );
    {
      struct tm *newtime;
      __time64_t long_time;
      _time64( &long_time );                
      newtime = _localtime64( &long_time );
      strcpy( m_szPathBuffer, asctime( newtime ) ); 
      m_szPathBuffer[strlen(m_szPathBuffer)-1] = '\0';  // cut-off trailing LF
      m_xw->WriteElementString( "creationdate", m_szPathBuffer );
    }
    m_xw->WriteElementString( "inputspecs", pszInFile );
    m_xw->WriteEndElement();


    m_InpFileMode = CheckInFile( pszInFile );

    switch ( m_InpFileMode )
    {
      case LONGFOLDERNAME:
        if ( MakeShortFolderName( pszInFile, m_szFolderShortName ) == 0 )
        {
          EvaluateFolder( m_szFolderShortName );
        }
        else
        {
          printf( "Error: No folder found with a name of \"%s\"\n", pszInFile );
          iRC = ERROR_FILE_NOT_FOUND;
        } /* endif */
        break;

      case SHORTFOLDERNAME:
        strcpy( m_szFolderShortName, pszInFile );
        EvaluateFolder( m_szFolderShortName );
        break;

      case EXPORTEDFOLDER:
        EvaluatePackage( pszInFile );
        break;

      case DIRECTORY:
        strcpy( m_szSearchPattern, pszInFile );
        if ( m_szSearchPattern[strlen(m_szSearchPattern)-1] != '\\' )
        {
          strcat( m_szSearchPattern, "\\" );
        } /* endif */
        strcat( m_szSearchPattern, "*.*" );
        ScanDirAndEvaluate( m_szSearchPattern, TRUE );
        break;

      case SEARCHPATTERN:
        ScanDirAndEvaluate( pszInFile, FALSE );
        break;

      default:
        printf( "Error: Specified file \"%s\" is not valid as input parameter.\n", pszInFile );
        iRC = ERROR_INVALID_DATA;
        break;
    } /*endswitch */

    // write totals
    WriteTotals();

    m_xw->WriteEndElement();
    m_xw->WriteEndDocument();
    delete m_xw;
  } /* endif */

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::Evaluate returns %ld\n", iRC );

  return( iRC );
}

CMTEval::InputFileMode CMTEval::CheckInFile( const char *pszInFile, BOOL fNameCheckOnly  )
{
  InputFileMode RC = INVFILESPEC;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::CheckInFile( \"%s\", \"%s\" )\n", pszInFile, fNameCheckOnly ? "True" : "False" );

  // check for wildcard characters
  BOOL fWildCards = (strchr( pszInFile, '?' ) != NULL) || (strchr( pszInFile, '*' ) != NULL); 

  // get extention of input file
  char *pszExt = (char *)strrchr( pszInFile, '.' );


  // use file extention to identify input file 
  if ( pszExt && !fWildCards )
  {
    if ( stricmp( pszExt, ".F00" ) == 0 )
    {
      RC = SHORTFOLDERNAME;
    }
    else if ( stricmp( pszExt, ".FXP" ) == 0 )
    {
      RC = EXPORTEDFOLDER;
    } /* endif */
  } /* endif */

  // if not identified yet: check if we deal with a directory name...
  if ( (RC == INVFILESPEC) && !fNameCheckOnly )
  {
    DWORD dwAttr = GetFileAttributes( pszInFile );
    if ( dwAttr != INVALID_FILE_ATTRIBUTES )
    {
      if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
      {
        RC = DIRECTORY;
      } /* endif */
    } /* endif */
  } /* endif */

  // if not identified yet: check if file name can be used for a file search operation
  if ( (RC == INVFILESPEC) && !fNameCheckOnly )
  {
    WIN32_FIND_DATA FindData;
    HANDLE hFind = FindFirstFile( pszInFile, &FindData );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
      RC = SEARCHPATTERN;
      FindClose( hFind );
    } /* endif */
  } /* endif */

  // if not identified yet: check if name contains a backslash or a colon
  if ( (RC == INVFILESPEC) && !fNameCheckOnly  )
  {
    if ( (strchr( pszInFile, '\\' ) == NULL) && (strchr( pszInFile, ':' ) == NULL)  )
    {
      // assume this is a long folder name
      RC = LONGFOLDERNAME;
    } /* endif */
  } /* endif */

  if ( m_hfTest ) fprintf( m_hfTest, "  CMTEval::CheckInFile returns %ld\n", RC );

  return( RC );
} /* end of method CheckInFile */


// convert a long folder name into a short folder name
// the code has been borrowed from function ObjLongToShortName
int CMTEval::MakeShortFolderName
( 
  const char *pszLongName, 
  char *pszShortName 
)
{
  // our private data area
  typedef struct _OBJL2SDATA
  {
    CHAR      szShortName[MAX_FILESPEC];    // buffer for short name
    CHAR      szFolder[MAX_FILESPEC];       // buffer for folder name
    CHAR      szSearchPath[MAX_EQF_PATH];   // object search path
    CHAR      szFullPath[MAX_EQF_PATH];     // fully qualified object name
    WIN32_FIND_DATA stResultBuf;              // file find structure
    CHAR      szExt[20];                    // buffer for object extention
    CHAR      szInLongName[MAX_LONGFILESPEC];// buffer for input long name
    CHAR      szPropLongName[MAX_LONGFILESPEC];// buffer for long name in property file
  } OBJL2SDATA, *POBJL2SDATA;

  // local variables
  BOOL        fNew = TRUE;
  POBJL2SDATA pData = NULL;            // ptr to private data area
  int         iRC = 0;                // function return code
  enum { SHORTNAME, LONGNAME} NameType = SHORTNAME;
  SHORT   i = 0;                         // number of characters in short name
  const char *pszLongTemp = pszLongName;

  // preset callers's variables
  *pszShortName = '\0';

  // allocate our private data area
  pData = (POBJL2SDATA) malloc( sizeof(OBJL2SDATA) );
  if ( !pData )
  {
    iRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // scan long name and add valid characters to short file name
  if ( !iRC )
  {
    pszLongTemp = pszLongName;

    while ( (i < 5) && (*pszLongTemp != EOS) )
    {
      CHAR chTest;
      chTest = *pszLongTemp;
      if ( ((chTest >= '0') && (chTest <= '9')) || // digit or
           ((chTest >= 'a') && (chTest <= 'z')) || // lowercase char or
           ((chTest >= 'A') && (chTest <= 'Z')) )  // uppercase char ?
      {
        CHAR c = *pszLongTemp++;
        pData->szShortName[i++] = (CHAR)toupper(c);
      }
      else
      {
        NameType = LONGNAME;
        pszLongTemp++;                   // try next character
      } /* endif */
    } /* endwhile */

    // scan rest of long file name for long object name recognition
    if ( NameType != LONGNAME )        // no non-alphanumeric chars yet?
    {
      int iLen = i;                    // remember current length
      while ( (iLen <= 8) && (NameType != LONGNAME) && (*pszLongTemp != EOS) )
      {
        CHAR chTest;
        chTest = *pszLongTemp;
        if ( ((chTest >= '0') && (chTest <= '9')) || // digit or
             ((chTest >= 'a') && (chTest <= 'z')) || // lowercase char or
             ((chTest >= 'A') && (chTest <= 'Z')) )  // uppercase char ?
        {
          pszLongTemp++;
          iLen++;
        }
        else
        {
          NameType = LONGNAME;
        } /* endif */
      } /* endwhile */

      if ( iLen > 8 )
      {
        NameType = LONGNAME;
      } /* endif */
    } /* endif */

    // complete short name or use long name if it is a short one
    if ( NameType != SHORTNAME )
    {
      // padd short name with A's up to a length of 5 characters
      while ( i < 5 )
      {
        pData->szShortName[i++] = 'A';
      } /* endif */

      // terminate short file name
      pData->szShortName[i] = EOS;
    }
    else
    {
      strcpy( pData->szShortName, pszLongName );
      strupr( pData->szShortName );
    } /* endif */
  } /* endif */

  // setup search path and path of full object name
  if ( !iRC )
  {
    GetProfileString( APPL_Name, KEY_Drive, "", pData->szSearchPath, sizeof(pData->szSearchPath) );
    strcat( pData->szSearchPath, "\\OTM\\PROPERTY\\" );
    strcpy( pData->szFullPath, pData->szSearchPath );
    strcat( pData->szSearchPath, pData->szShortName );
    if ( NameType == LONGNAME ) 
    {
      strcat( pData->szSearchPath, "*.F00" );
    }
    else
    {
      strcpy( pData->szExt, EXT_FOLDER_MAIN );
    } /* endif */
  } /* endif */

  // look for objects having the same short name
  if ( !iRC )
  {
    // if specified long name is a short name use this name for search
    if ( NameType == SHORTNAME )
    {
      strcpy( pData->szSearchPath, pData->szFullPath );
      strcat( pData->szSearchPath, pData->szShortName );
      strcat( pData->szSearchPath, pData->szExt );
      strcpy( pszShortName, pData->szShortName );
    } /* endif */

    HANDLE hFind = FindFirstFile( pData->szSearchPath, &pData->stResultBuf );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
      do
      {
        PPROPFOLDER pFolProp = NULL;

        strcpy( pData->szSearchPath, pData->szFullPath );
        strcat( pData->szSearchPath, pData->stResultBuf.cFileName );

        iRC = LoadFileFromDisk( pData->szSearchPath, (void **)&pFolProp );

        if ( !iRC )
        {
          if ( !pFolProp->szLongName[0] )
          {
            strcpy( pFolProp->szLongName, pFolProp->PropHead.szName );
            char *pszExt = strrchr( pFolProp->szLongName, '.' );
            if ( pszExt ) *pszExt = '\0';
          } /* endif */

          // compare long name of found object with given long name
          strcpy( pData->szInLongName, pszLongName );
          OemToChar( pData->szInLongName, pData->szInLongName );
          strupr( pData->szInLongName  );
          strcpy( pData->szPropLongName, pFolProp->szLongName );
          OemToChar( pData->szPropLongName, pData->szPropLongName );
          strupr( pData->szPropLongName );

          fNew = (strcmp( pData->szInLongName, pData->szPropLongName ) != 0);

          if ( !fNew )
          {
            strcpy( pszShortName, pData->stResultBuf.cFileName );
          } /* endif */

          if ( pFolProp ) free( pFolProp );
        } /* endif */

        // try next object
        if ( fNew )
        {
          if ( !FindNextFile( hFind, &pData->stResultBuf ) ) iRC = ERROR_NO_MORE_FILES;
        } /* endif */
      } while ( !iRC && fNew );
      FindClose( hFind );
    } /* endif */
  } /* endif */

  // cleanup
  if ( pData ) free( pData);

  if ( !iRC && fNew ) iRC = ERROR_FILE_NOT_FOUND;

  return( iRC );
} /* end of CMTEval::MakeShortFolderName */

int CMTEval::EvaluateFolder
( 
  const char *pszShortName 
)
{
  int iRC = 0;
  PPROPFOLDER pFolProp = NULL;
  BOOL fFolderStarted = FALSE;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::EvaluateFolder( \"%s\" )\n", pszShortName );

  // setup folder property file name
  GetProfileString( APPL_Name, KEY_Drive, "", m_szPathBuffer, sizeof(m_szPathBuffer) );
  strcat( m_szPathBuffer, "\\OTM\\PROPERTY\\" );
  strcat( m_szPathBuffer, pszShortName );
  if ( m_hfTest ) fprintf( m_hfTest, "  CMTEval::EvaluateFolder PropFile=\"%s\"\n", m_szPathBuffer );

  // load folder property file
  if ( !iRC )
  {
    iRC = LoadFileFromDisk( m_szPathBuffer, (void **)&pFolProp );
  } /* endif */

  // evaluate folder property file
  if ( !iRC )
  {
    fFolderStarted = TRUE;
    iRC = EvaluateFolderProps( pFolProp );  
  } /* endif */

  // loop over all MT logs of folder
  if ( !iRC )
  {
    // setup search pattern
    sprintf( m_szPathBuffer, "%c:\\OTM\\%s\\MTLOG\\*.*",pFolProp->chDrive, pszShortName );    
    if ( m_hfTest ) fprintf( m_hfTest, "  CMTEval::EvaluateFolder SearchPattern=\"%s\"\n", m_szPathBuffer );

    // find and process MT log files of folder
    WIN32_FIND_DATA FindData;
    HANDLE hFind = FindFirstFile( m_szPathBuffer, &FindData );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
      do
      {
        void *pMTLog = NULL;
        PPROPDOCUMENT pDocProp = NULL;
        int iLogFileSize = 0;

        if ( (strcmp( FindData.cFileName, ".") != 0) &&  (strcmp( FindData.cFileName, "..") != 0) )
        {
          if ( m_hfTest ) fprintf( m_hfTest, "  CMTEval::EvaluateFolder found document \"%s\"\n", FindData.cFileName );

          // load MT log file into memory
          sprintf( m_szPathBuffer, "%c:\\OTM\\%s\\MTLOG\\", pFolProp->chDrive, pszShortName );    
          strcat( m_szPathBuffer, FindData.cFileName );
          iRC = LoadFileFromDisk( m_szPathBuffer, &pMTLog, &iLogFileSize );

          // load document property file
          if ( !iRC )
          {
            sprintf( m_szPathBuffer, "%c:\\OTM\\%s\\PROPERTY\\",pFolProp->chDrive, pszShortName );    
            strcat( m_szPathBuffer, FindData.cFileName );
            iRC = LoadFileFromDisk( m_szPathBuffer, (void **)&pDocProp );
          } /* endif */

          // evaluate log file
          if ( !iRC )
          {
            EvaluateLogFile( pDocProp, pMTLog, iLogFileSize );
          } /* endif */

          if ( pMTLog )   free( pMTLog );
          if ( pDocProp ) free( pDocProp );
        } /* endif */
      } while ( !iRC && FindNextFile( hFind, &FindData ) );

      FindClose( hFind );
    } /* endif */
  } /* endif */

  // cleanup
  if ( pFolProp ) free ( pFolProp );

  if ( fFolderStarted )
  {
    WriteFolderNumbers();             // show folder summary
    m_xw->WriteEndElement();          // write end of folder element
    AddFolderNumbersToTotal();        // add folder values to total sum of this run
  } /* endif */

  if ( m_hfTest ) fprintf( m_hfTest, "  CMTEval::EvaluateFolder returns %ld\n", iRC );

  return( iRC );
} /* end of CMTEval::EvaluateFolder */


int CMTEval::EvaluatePackage
( 
  const char *pszPackage 
)
{
  int iRC = 0;
  BOOL fFolderStarted = FALSE;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::EvaluatePackage( \"%s\" )\n", pszPackage );

  // create package instance
  CFXP *pPackage = new CFXP();
  if ( pPackage == NULL )
  {
    iRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // open package file
  if ( !iRC )
  {
    iRC = pPackage->Open( pszPackage );
    if ( iRC  )
    {
      pPackage->GetLastMessage( m_szErrorMessage, sizeof(m_szErrorMessage) );
      printf( "%s\n", m_szErrorMessage );
      if ( m_hfTest ) fprintf( m_hfTest, "  Package.Open returned %ld: \"%s\"\n", iRC, m_szErrorMessage );
    } /* endif */
  } /* endif */

  // get and process folder property file
  if ( !iRC )
  {
    PPROPFOLDER pProp = NULL;

    // search folder property file
    LONG lFolPropToken = pPackage->GetFirstFile( FOLDER_PROP_FILE );
    if ( lFolPropToken == 0 )
    {
      printf( "Error: No folder property file found in exported folder %s\n", pszPackage );
      if ( m_hfTest ) fprintf( m_hfTest, "  Package.GetFirstFile( FOLDER_PROP_FILE ) returned 0\n" );
      iRC = ERROR_FILE_NOT_FOUND;
    } /* endif */

    // load property file into memory
    if ( !iRC )
    {
      iRC = LoadFileFromPackage( pPackage, lFolPropToken, (void **)&pProp );
    } /* endif */

    // evaluate property file
    if ( !iRC )
    {
      fFolderStarted = TRUE;
      iRC = EvaluateFolderProps( pProp );  
    } /* endif */

    if ( pProp ) free( pProp );
  } /* endif */

  // loop over all document MT logs in the package
  if ( !iRC )
  {
    LONG lMTlogToken = pPackage->GetFirstFile( DOCUMENT_MTLOG_FILE );
    while ( !iRC && lMTlogToken )
    {
      void *pMTLog = NULL;
      PPROPDOCUMENT pDocProp = NULL;
      int iLogFileSize = 0;

      // load log file into memory
      if ( !iRC )
      {
        iRC = LoadFileFromPackage( pPackage, lMTlogToken, (void **)&pMTLog, &iLogFileSize );
      } /* endif */

      // get and load corresponding document property file
      if ( !iRC )
      {
        long lPropDocToken = 0;

        // get MT log file name
        pPackage->GetFileName( lMTlogToken, m_szPathBuffer, sizeof(m_szPathBuffer) );

        // change mt log dir to property dir
        char *pszFileName = strrchr( m_szPathBuffer, '\\' );
        if ( pszFileName)
        {
          *pszFileName = '\0';
          strcpy( m_szFileSpec, pszFileName + 1 );
          pszFileName = strrchr( m_szPathBuffer, '\\' );
          if ( pszFileName)
          {
            strcpy( pszFileName, "\\PROPERTY\\" );
            strcat( m_szPathBuffer, m_szFileSpec );
          }
          else
          {
            iRC = ERROR_INVALID_DATA;
          } /* endif */
        }
        else
        {
          iRC = ERROR_INVALID_DATA;
        } /* endif */

        // get property file token
        if ( !iRC )
        {
          lPropDocToken = pPackage->GetFileByName( m_szPathBuffer );
          if ( !lPropDocToken ) iRC = ERROR_FILE_NOT_FOUND;
        } /* endif */

        // load property file into memory
        if ( !iRC )
        {
          iRC = LoadFileFromPackage( pPackage, lPropDocToken, (void **)&pDocProp );
        } /* endif */
      } /* endif */

      // evaluate log file
      if ( !iRC )
      {
        EvaluateLogFile( pDocProp, pMTLog, iLogFileSize );
      } /* endif */

      // get next MT log file
      if ( !iRC )
      {
        lMTlogToken = pPackage->GetNextFile();
      } /* endif */

      if ( pMTLog )   free( pMTLog );
      if ( pDocProp ) free( pDocProp );
    } /*endwhile */
  } /* endif */


  if ( fFolderStarted )
  {
    WriteFolderNumbers();             // show folder summary
    m_xw->WriteEndElement();          // write end of folder element
    AddFolderNumbersToTotal();        // add folder values to total sum of this run
  } /* endif */

  if ( pPackage ) delete pPackage;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::EvaluatePackage returns %ld\n", iRC  );

  return( 0 );
} /* end of CMTEval::EvaluatePackage */


// scan current directory using the supplied pattern, recourse into subdirectories if requested
int CMTEval::ScanDirAndEvaluate
( 
  const char *szSearchPattern, 
  BOOL fScanSubDirs  
)
{
  int iRC = 0;
  char *pszFullPath = (char *)malloc( 512 );
  WIN32_FIND_DATA FindData;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::ScanDirAndEvaluate( \"%s\", %s )\n", szSearchPattern, fScanSubDirs ? "True" : "False" );

  HANDLE hFind = FindFirstFile( szSearchPattern, &FindData );
  if ( hFind != INVALID_HANDLE_VALUE )
  {
    BOOL fFound = TRUE;
    do
    {
      if ( (strcmp( FindData.cFileName, ".") == 0) || (strcmp( FindData.cFileName, ".." ) == 0) )
      {
        // ignore directory dummy files
      }
      else if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
      {
        if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::ScanDirAndEvaluate processing sub directory \"%s\"\n", FindData.cFileName );

        // handle sub-directories
        if ( fScanSubDirs )
        {
          strcpy( pszFullPath, szSearchPattern );
          char *pszLastBackslash = strrchr(pszFullPath, '\\' );
          if ( pszLastBackslash )
          {
            // insert found directory into search pattern string
            int iDirLen = strlen( FindData.cFileName );
            memmove( pszLastBackslash + iDirLen + 1, pszLastBackslash, strlen(pszLastBackslash) + 1);
            memcpy( pszLastBackslash + 1, FindData.cFileName, iDirLen );

            // scan subdirectory
            iRC = ScanDirAndEvaluate( pszFullPath, fScanSubDirs );
          } /* endif */
        } /* endif */
      }
      else if ( CheckInFile( FindData.cFileName, TRUE ) == EXPORTEDFOLDER )
      {
        // handle folder packages
        if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::ScanDirAndEvaluate processing folder \"%s\"\n", FindData.cFileName );

        // setup fully qualified path to package file
        strcpy( pszFullPath, szSearchPattern );
        char *pszLastBackslash = strrchr(pszFullPath, '\\' );
        if ( pszLastBackslash ) *pszLastBackslash = '\0';
        strcat( pszFullPath, "\\" ); 
        strcat( pszFullPath, FindData.cFileName ); 

        // evaluate package file
        EvaluatePackage( pszFullPath );
      } /* endif */

      // continue search
      fFound = FindNextFile( hFind, &FindData );
   
    } while ( fFound );

    FindClose( hFind );
  } /* endif */

  if ( pszFullPath ) free( pszFullPath );

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::ScanDirAndEvaluate returns %ld\n", iRC );

  return( iRC );
} /* end of CMTEval::ScanDirAndEvaluate */

// helper method: load a file from a package into memory
int CMTEval::LoadFileFromPackage( CFXP *pPackage, long lToken, void **pvBuf, int *piSize )
{
  int iRC = 0;
  int iSize = 0;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::LoadFileFromPackage( Token=%ld )\n", lToken );

  *pvBuf = NULL;

  iRC = pPackage->GetFileSize( lToken, &iSize );

  if ( !iRC )
  {
    *pvBuf = malloc( iSize );
    if ( *pvBuf == NULL )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */
      
  if ( !iRC )
  {
  	iRC = pPackage->ReadFile( lToken, *pvBuf, iSize );
  } /* endif */

  if ( iRC && *pvBuf ) free( *pvBuf );
  if ( piSize ) *piSize = iSize;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::LoadFileFromPackage returns %ld, size=%l\n", iRC, iSize );

  return( iRC );
} /* end of CMTEval::LoadFileFromPackage */

// helper method: load a file from disk into memory
int CMTEval::LoadFileFromDisk( char *pszFile, void **pvBuf,  int *piSize  )
{
  int iRC = 0;
  int iSize = 0;
  FILE *hf;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::LoadFileFromDisk( \"%s\",...)\n", pszFile );

  *pvBuf = NULL;

  // open the file
  hf = fopen( pszFile, "rb" );
  if ( hf == NULL )
  {
    iRC = ERROR_FILE_NOT_FOUND;
  } /* endif */

  // get the file size
  if ( !iRC )
  {
    iSize = _filelength( _fileno(hf) );
    if ( iSize == -1 )
    {
      iRC = ERROR_READ_FAULT;
    } /* endif */
  } /* endif */

  // allocate buffer for file
  if ( !iRC )
  {
    *pvBuf = malloc( iSize );
    if ( *pvBuf == NULL )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // read file into buffer
  if ( !iRC )
  {
    int iRead = (int)fread( *pvBuf, 1, iSize, hf );
    if ( iRead != iSize )
    {
      iRC = ERROR_READ_FAULT;
    } /* endif */
  } /* endif */

  // cleanup
  if ( iRC && *pvBuf ) free( *pvBuf );
  if ( hf ) fclose( hf );


  if ( piSize ) *piSize = iSize;

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::LoadFileFromDisk returns %ld, size=%l\n", iRC, iSize );

  return( iRC );
} /* end of CMTEval::LoadFileFromDisk */

// evaluate MT information of a folder property file
int CMTEval::EvaluateFolderProps( PPROPFOLDER pProp )
{
  int iRC = 0;

  // use folder short name if no long name exists
  if ( !pProp->szLongName[0] )
  {
    strcpy( pProp->szLongName, pProp->PropHead.szName );
    char *pszExt = strrchr( pProp->szLongName, '.' );
    if ( pszExt ) *pszExt = '\0';
  } /* endif */

  // debug/test output
  if ( m_hfTest ) 
  {
    fprintf( m_hfTest, "CMTEval::EvaluateFolderProps( \"%s\" )\n", pProp->PropHead.szName );
    fprintf( m_hfTest, "  Folder: %s\n", pProp->szLongName );
    if ( pProp->fMTFieldsFilled )
    {
      fprintf( m_hfTest, "  Total segments processed   : %ld, %ld, %ld\n", pProp->ulMTTotalSegs[0], pProp->ulMTTotalSegs[1], pProp->ulMTTotalSegs[2] );
      fprintf( m_hfTest, "  Total words processed      : %ld, %ld, %ld\n", pProp->ulMTTotalWords[0], pProp->ulMTTotalWords[1], pProp->ulMTTotalWords[2] );
      fprintf( m_hfTest, "  Segments send to MT server : %ld, %ld, %ld\n", pProp->ulMTSendSegs[0], pProp->ulMTSendSegs[1], pProp->ulMTSendSegs[2] );
      fprintf( m_hfTest, "  Words send to MT server    : %ld, %ld, %ld\n", pProp->ulMTSendWords[0], pProp->ulMTSendWords[1], pProp->ulMTSendWords[2] );
      if ( pProp->fMTReceived )
      {
        fprintf( m_hfTest, "  Segments from MT server    : %ld, %ld, %ld\n", pProp->ulMTReceivedSegs[0], pProp->ulMTReceivedSegs[1], pProp->ulMTReceivedSegs[2] );
        fprintf( m_hfTest, "  Words from MT server       : %ld, %ld, %ld\n", pProp->ulMTReceivedWords[0], pProp->ulMTReceivedWords[1], pProp->ulMTReceivedWords[2] );
      }
      else
      {
        fprintf( m_hfTest, "  No MT job received from MT server\n" );
      } /* endif */
    }
    else
    {
      fprintf( m_hfTest, "  MT fields not filled\n" );
    } /* endif */
  } /* endif */

  // clear folder values
  ClearNumbers( m_ulFolMTExistSegs );
  ClearNumbers( m_ulFolMTExistWords );
  ClearNumbers( m_ulFolMTUsedSegs );
  ClearNumbers( m_ulFolMTUsedWords );
  ClearNumbers( m_ulFolMTTime );
  ClearNumbers( m_ulFolMTSegs );
  m_iDocsInFolder = 0;

  // copy folder MT job info
  CopyNumbers( m_ulFolMTTotalSegs,     pProp->ulMTTotalSegs );
  CopyNumbers( m_ulFolMTTotalWords,    pProp->ulMTTotalWords );
  CopyNumbers( m_ulFolMTSendSegs,      pProp->ulMTSendSegs ); 
  CopyNumbers( m_ulFolMTSendWords,     pProp->ulMTSendWords );
  CopyNumbers( m_ulFolMTReceivedSegs,  pProp->ulMTReceivedSegs );
  CopyNumbers( m_ulFolMTReceivedWords, pProp->ulMTReceivedWords );

  // write folder counts
  m_xw->WriteStartElement( "folder" );
  m_xw->WriteAttributeString( "name", pProp->szLongName );
  m_xw->WriteAttributeString( "MTInfoAvailable", pProp->fMTFieldsFilled ? "1" : "0" );
  //m_xw->WriteAttributeString( "MTJobReceived", pProp->fMTReceived ? "1" : "0" );
  if ( pProp->fMTFieldsFilled )
  {
    m_xw->WriteStartElement( "mtJobData" );
    WriteNumbers( "total", "segments", pProp->ulMTTotalSegs );
    WriteNumbers( "total", "words",    pProp->ulMTTotalWords );
    WriteNumbers( "send",  "segments", pProp->ulMTSendSegs );
    WriteNumbers( "send",  "words",    pProp->ulMTSendWords );
    if ( pProp->fMTReceived )
    {
      WriteNumbers( "received", "segments",  pProp->ulMTReceivedSegs );
      WriteNumbers( "received", "words",     pProp->ulMTReceivedWords );
    } /* endif */
    m_xw->WriteEndElement();
  } /* endif */

  if ( m_hfTest ) fprintf( m_hfTest, "CMTEval::EvaluateFolderprops returns %ld\n", iRC );

  return( iRC );
} /* end of CMTEval::EvaluateFolderProps */

// check MT log record in the new format
BOOL CMTEval::CheckNewRecord( PACTSEGLOGENH pEntry  )
{
  // test eye catcher
  if ( pEntry->ulEyeCatcher != ACTSEGLOG_EYECATCHER ) return( FALSE );

  // test record length
  ULONG ulMaxSize = sizeof(ACTSEGLOGENH) + ((MAX_SEGMENT_SIZE + 1) * sizeof(CHAR_W)) + MAX_LONGFILESPEC;
  if ( pEntry->ulRecordLen > ulMaxSize ) return( FALSE );

  // test check sum
  ULONG ulTestCheckSum = 0;
  int i = 0;
  int iLen = (int)pEntry->ulRecordLen - sizeof(pEntry->ulEyeCatcher) - sizeof(pEntry->ulCheckSum);
  PBYTE pbTest = (PBYTE)&(pEntry->ulRecordLen);
  while ( i < iLen ) ulTestCheckSum = ulTestCheckSum  + (ULONG)pbTest[i++];

  return( ulTestCheckSum == pEntry->ulCheckSum );

} /* end of CMTEval::CheckNewRecord */

// check MT log record in the new format version 2
BOOL CMTEval::CheckNewRecord2( PACTSEGLOGENH2 pEntry  )
{
  // test eye catcher
  if ( pEntry->ulEyeCatcher != ACTSEGLOG2_EYECATCHER ) return( FALSE );

  // test record length
  ULONG ulMaxSize = sizeof(ACTSEGLOGENH2) + ((MAX_SEGMENT_SIZE + 1) * sizeof(CHAR_W)) + MAX_LONGFILESPEC + (5 * MAX_SEGMENT_SIZE * sizeof(CHAR_W));
  if ( pEntry->ulRecordLen > ulMaxSize ) return( FALSE );

  // test check sum (which does not include the PE data section)
  ULONG ulTestCheckSum = 0;
  int i = 0;
  int iLen = (int)pEntry->ulRecordLen - sizeof(pEntry->ulEyeCatcher) - sizeof(pEntry->ulCheckSum) - pEntry->ulPEDataLen;
  PBYTE pbTest = (PBYTE)&(pEntry->ulRecordLen);
  while ( i < iLen ) ulTestCheckSum = ulTestCheckSum  + (ULONG)pbTest[i++];

  return( ulTestCheckSum == pEntry->ulCheckSum );

} /* end of CMTEval::CheckNewRecord */

// check MT log record in the old format
BOOL CMTEval::CheckOldRecord( PACTSEGLOGOLD pEntry  )
{
  if ( pEntry->usNumTyped > 8000 ) return( FALSE );

  //if ( pEntry->ulTime > 0x7FFFF ) return( FALSE );

  if ( pEntry->usWordCnt > 2048 ) return( FALSE );

  return( TRUE );
} /* end of CMTEval::CheckOldRecord */


// check records in MT log, setup array of record pointers
int CMTEval::CheckRecords( PVOID pvMTLog, int iLogFileSize )
{
  BOOL fNewRecordsFound = FALSE;                  // TRUE = new records found, do not check anymore for old ones
  ACTSEGLOGENH2 *pActSegLog = (ACTSEGLOGENH2 *)pvMTLog;
  int iRemaining = iLogFileSize;

  // skip first dummy record in old format
  if ( (iRemaining >= sizeof(ACTSEGLOGOLD)) && (pActSegLog->ulEyeCatcher != ACTSEGLOG_EYECATCHER) && (pActSegLog->ulEyeCatcher != ACTSEGLOG2_EYECATCHER) )
  {
    iRemaining -= sizeof(ACTSEGLOGOLD);
    pActSegLog = (PACTSEGLOGENH2)((PBYTE)pActSegLog + sizeof(ACTSEGLOGOLD));
  } /* endif */     

  m_iRecords = 0;

  while ( iRemaining >= sizeof(ACTSEGLOGOLD) )
  {
    BOOL fValid = FALSE;                         // TRUE = the current record seems to be valid

    if ( pActSegLog->ulEyeCatcher == ACTSEGLOG2_EYECATCHER )
    {
      // a record in the new format version 2
      fNewRecordsFound = TRUE;
      fValid = CheckNewRecord2( pActSegLog );
    }
    else if ( pActSegLog->ulEyeCatcher == ACTSEGLOG_EYECATCHER )
    {
      // a record in the new format
      fNewRecordsFound = TRUE;
      fValid = CheckNewRecord( (PACTSEGLOGENH)pActSegLog );
    }
    else
    {
      // old type record or garbage
      if ( !fNewRecordsFound )
      {
        fValid = CheckOldRecord( (PACTSEGLOGOLD)pActSegLog );
      } /* endif */       
    } /* endif */     

    // add record pointer when record seems to be valid or report corrupted record
    if ( fValid )
    {
      // enlarge pointer array if necessary
      if ( m_iRecords >= m_iAllocatedPointers )
      {
        m_iAllocatedPointers += 500;
        m_pRecords = (PVOID *)realloc( m_pRecords, m_iAllocatedPointers * sizeof(PVOID) );
      } /* endif */         

      // store pointer to current record
      m_pRecords[m_iRecords++] = pActSegLog;
    }
    else
    {
      if ( m_hfTest ) 
      {
        int iOffset = (PBYTE)pActSegLog - (PBYTE)pvMTLog;
        fprintf( m_hfTest, "Entry at offset &ld is corrupted\n", iOffset );
      } /* endif */
    } /* endif */       

    // continue with next entry
    if ( fValid )
    {
      if ( pActSegLog->ulEyeCatcher == ACTSEGLOG2_EYECATCHER )
      {
        iRemaining -= (int)(pActSegLog->ulRecordLen);
        pActSegLog = (PACTSEGLOGENH2)((PBYTE)pActSegLog + pActSegLog->ulRecordLen);
      }
      else if ( pActSegLog->ulEyeCatcher == ACTSEGLOG_EYECATCHER )
      {
        iRemaining -= (int)(((PACTSEGLOGENH)pActSegLog)->ulRecordLen);
        pActSegLog = (PACTSEGLOGENH2)((PBYTE)pActSegLog + pActSegLog->ulRecordLen);
      }
      else
      {
        iRemaining -= sizeof(ACTSEGLOGOLD);
        pActSegLog = (PACTSEGLOGENH2)((PBYTE)pActSegLog + sizeof(ACTSEGLOGOLD));
      } /* endif */         
    }
    else
    {
      int iSkipped = 0;
      if ( !fNewRecordsFound )
      {
        pActSegLog = (PACTSEGLOGENH2)((PBYTE)pActSegLog + 1 );
        iRemaining--;
        iSkipped++;
      }
      else
      {
        // search the next eye catcher in the data stream
        do
        {
          pActSegLog = (PACTSEGLOGENH2)((PBYTE)pActSegLog + 1 );
          iRemaining--;
          iSkipped++;
        } while ( (iRemaining > sizeof(ACTSEGLOGENH))  && (pActSegLog->ulEyeCatcher != ACTSEGLOG_EYECATCHER) && (pActSegLog->ulEyeCatcher != ACTSEGLOG2_EYECATCHER)); /* enddo */         
      } /* endif */       
      if ( m_hfTest ) fprintf( m_hfTest, "Searching for next valid record, skipped %ld bytes\n", iSkipped );

    } /* endif */       
  } /* endwhile */     

  return( m_iRecords );
}

int CMTEval::EvaluateLogFile( PPROPDOCUMENT pProp, PVOID pvMTLog, int iLogFileSize )
{
  int iRC = 0;
  ULONG ulMTExistSegs[3], ulMTExistWords[3]; // arrays for existing MT proposals
  ULONG ulMTUsedSegs[3], ulMTUsedWords[3];   // array for used MT proposals
  ULONG ulMTSegs[3], ulMTTime[3];            // arrays for number of segmens and time
  int iShipment = 1;

  // use document short name if no long name exists
  if ( !pProp->szLongName[0] )
  {
    strcpy( pProp->szLongName, pProp->PropHead.szName );
  } /* endif */

  // check records and setup record pointer array
  CheckRecords( pvMTLog, iLogFileSize );

  // debug/test output
  if ( m_hfTest ) 
  {
    fprintf( m_hfTest, "CMTEval::EvaluateLogFile( \"%s\", ..., %ld )\n", pProp->PropHead.szName, iLogFileSize );
    fprintf( m_hfTest, "  Document: %s\n", pProp->szLongName );
  } /* endif */

  // convert time to seconds and insert sequence numbers in log file 
  // (in new records we use the checksum field for this purpose, in older records
  // the first part of the time field for the sequence number)
  // also add shipment number to the ulCheckSum field
  if ( m_hfTest ) 
  {
    fprintf( m_hfTest, "Before sort\n" );
  } /* endif */
  for( int i = 0; i < m_iRecords; i++ )
  {
    PACTSEGLOGENH2 pTemp = (PACTSEGLOGENH2)m_pRecords[i];
    if ( m_hfTest ) 
    {
      if ( pTemp->ulEyeCatcher == ACTSEGLOG2_EYECATCHER )
      {
        struct tm *time;
        LONG lTime = pTemp->ulTimeStamp + 10800L;  // use correct Windows time format                
        time = localtime( &lTime );
        strcpy( m_szBuffer, asctime( time ) );
        if ( m_szBuffer[strlen(m_szBuffer)-1] == '\n' ) m_szBuffer[strlen(m_szBuffer)-1] = EOS;
        if ( pTemp->AddFlags.ShipmentRec )
        {
          fprintf( m_hfTest, "Entry: %4ld  New shipment\n", i );
        }
        else
        {
          fprintf( m_hfTest, "Entry: %4ld  Seg: %5lu  Words: %3u TimeStamp: %s\n", i, pTemp->ulSegNum, pTemp->usWordCnt, m_szBuffer );
        } /* endif */           
      }
      else if ( pTemp->ulEyeCatcher == ACTSEGLOG_EYECATCHER )
      {
       PACTSEGLOGENH pTempEnh = (PACTSEGLOGENH)pTemp;
        struct tm *time;
        LONG lTime = pTempEnh->ulTimeStamp + 10800L;  // use correct Windows time format                
        time = localtime( &lTime );
        strcpy( m_szBuffer, asctime( time ) );
        if ( m_szBuffer[strlen(m_szBuffer)-1] == '\n' ) m_szBuffer[strlen(m_szBuffer)-1] = EOS;
        if ( pTempEnh->AddFlags.ShipmentRec )
        {
          fprintf( m_hfTest, "Entry: %4ld  New shipment\n", i );
        }
        else
        {
          fprintf( m_hfTest, "Entry: %4ld  Seg: %5lu  Words: %3u TimeStamp: %s\n", i, pTempEnh->ulSegNum, pTempEnh->usWordCnt, m_szBuffer );
        } /* endif */           
      }
      else
      {
        PACTSEGLOGOLD pOldEntry = (PACTSEGLOGOLD)pTemp;
        fprintf( m_hfTest, "Entry: %4ld  Seg: %5lu  Words: %3u\n", i, pOldEntry->ulSegNum, pOldEntry->usWordCnt );
      } /* endif */         
    } /* endif */

    if ( pTemp->ulEyeCatcher == ACTSEGLOG2_EYECATCHER )
    {
      if ( pTemp->AddFlags.ShipmentRec )
      {
        iShipment++;
      }
      else
      {
        // convert time to 1/100 seconds
        if ( pTemp->ulThinkTime != 0 ) pTemp->ulThinkTime = (pTemp->ulThinkTime + 5) / 10;
        if ( pTemp->ulChoiceTime != 0 ) pTemp->ulChoiceTime = (pTemp->ulChoiceTime + 5) / 10;
        if ( pTemp->ulTotalTime != 0 ) pTemp->ulTotalTime = (pTemp->ulTotalTime + 5) / 10;
        pTemp->ulTime = (pTemp->ulTime + 5) / 10;
        pTemp->ulCheckSum = (ULONG)i + ((ULONG)iShipment  << 16);
      } /* endif */         
    }
    else if ( pTemp->ulEyeCatcher == ACTSEGLOG_EYECATCHER )
    {
      PACTSEGLOGENH pTempEnh = (PACTSEGLOGENH)pTemp;
      if ( pTempEnh->AddFlags.ShipmentRec )
      {
        iShipment++;
      }
      else
      {
        // convert time to 1/100 seconds
        pTempEnh->ulTime = (pTempEnh->ulTime + 5) / 10;
        pTempEnh->ulCheckSum = (ULONG)i + ((ULONG)iShipment  << 16);
      } /* endif */         
    }
    else
    {
      PACTSEGLOGOLD pOldEntry = (PACTSEGLOGOLD)pTemp;
      pOldEntry->ulTime = (((pOldEntry->ulTime + 500) / 1000) & 0x0000FFFF) |  ((ULONG)i << 16);
    } /* endif */         

    pTemp++;
  } /* endfor */

  if ( m_iRecords > 1 )
  {
    qsort( m_pRecords, m_iRecords, sizeof(PVOID), qLogCompare );
  } /* endif */

  if ( m_hfTest ) 
  {
    fprintf( m_hfTest, "After sort\n" );
    for( int i = 0; i < m_iRecords; i++ )
    {
      PACTSEGLOGENH2 pTemp = (PACTSEGLOGENH2)m_pRecords[i];

      if ( pTemp->ulEyeCatcher == ACTSEGLOG2_EYECATCHER )
      {
        struct tm *time;
        LONG lTime = pTemp->ulTimeStamp + 10800L;  // use correct Windows time format                
        time = localtime( &lTime );
        strcpy( m_szBuffer, asctime( time ) );
        if ( m_szBuffer[strlen(m_szBuffer)-1] == '\n' ) m_szBuffer[strlen(m_szBuffer)-1] = EOS;
        fprintf( m_hfTest, "Entry: %4ld  Seg: %5lu  Words: %3u TimeStamp: %s\n", i, pTemp->ulSegNum, pTemp->usWordCnt, m_szBuffer );
      }
      else if ( pTemp->ulEyeCatcher == ACTSEGLOG_EYECATCHER )
      {
        PACTSEGLOGENH pTempEnh = (PACTSEGLOGENH)pTemp;
        struct tm *time;
        LONG lTime = pTempEnh->ulTimeStamp + 10800L;  // use correct Windows time format                
        time = localtime( &lTime );
        strcpy( m_szBuffer, asctime( time ) );
        if ( m_szBuffer[strlen(m_szBuffer)-1] == '\n' ) m_szBuffer[strlen(m_szBuffer)-1] = EOS;
        fprintf( m_hfTest, "Entry: %4ld  Seg: %5lu  Words: %3u TimeStamp: %s\n", i, pTempEnh->ulSegNum, pTempEnh->usWordCnt, m_szBuffer );
      }
      else
      {
        PACTSEGLOGOLD pOldEntry = (PACTSEGLOGOLD)pTemp;
        fprintf( m_hfTest, "Entry: %4ld  Seg: %5lu  Words: %3u\n", i, pOldEntry->ulSegNum, pOldEntry->usWordCnt );
      } /* endif */         
    } /* endfor */
  } /* endif */

  // build MT exists / MT used summary
  ClearNumbers( ulMTExistSegs ); 
  ClearNumbers( ulMTExistWords );
  ClearNumbers( ulMTUsedSegs );
  ClearNumbers( ulMTUsedWords );
  ClearNumbers( ulMTSegs );
  ClearNumbers( ulMTTime );

  ULONG ulCurrentSeg = (ULONG)-1;
  
  m_xw->WriteStartElement( "document" );
  m_xw->WriteAttributeString( "name", pProp->szLongName );

  // we always start with shipment 1
  iShipment = 1;

  if ( m_fDetails )
  {
    m_xw->WriteStartElement( "segmentList" );
    m_xw->WriteStartAttribute( "shipment" );
    m_xw->WriteInt( iShipment );
    m_xw->WriteEndAttribute();
  } /* endif */

  for( int i = 0; i < m_iRecords; i++ )
  {
    PACTSEGLOGENH2 pTemp = (PACTSEGLOGENH2)m_pRecords[i];
    PACTSEGLOGENH  pTempEnh = (PACTSEGLOGENH)pTemp;

    if ( ( (pTemp->ulEyeCatcher == ACTSEGLOG2_EYECATCHER) && pTemp->AddFlags.ShipmentRec ) ||
         ( (pTempEnh->ulEyeCatcher == ACTSEGLOG_EYECATCHER) && pTempEnh->AddFlags.ShipmentRec) )
    {
      // ignore shipment records...
    }
    else
    {
      BOOL fOldRecord = FALSE;


      // copy record to our data areas
      if ( pTemp->ulEyeCatcher == ACTSEGLOG2_EYECATCHER )
      {
        memcpy( &m_ActSegLog, pTemp, sizeof(m_ActSegLog) );
        m_szSegSource[0] = 0;
        if ( pTemp->ulSegLen != 0 ) wcscpy( m_szSegSource, (PSZ_W)((PBYTE)pTemp + sizeof(ACTSEGLOGENH2)) );
        m_szPropDoc[0] = 0;
        if (pTemp->ulPropSourceLen != 0) strcpy( m_szPropDoc, (PSZ)((PBYTE)pTemp + sizeof(ACTSEGLOGENH2) + pTemp->ulSegLen) );
        m_szMetaData[0] = 0;
        if (pTemp->ulMetaDataLen != 0) wcscpy( m_szMetaData, (PSZ_W)((PBYTE)pTemp + sizeof(ACTSEGLOGENH2) + pTemp->ulSegLen + pTemp->ulPropSourceLen) );

        memset( &m_PEData, 0, sizeof(m_PEData) );
        if ( pTemp->ulPEDataLen != 0 )
        {
          PBYTE pbData = (PBYTE)pTemp + sizeof(ACTSEGLOGENH2) + pTemp->ulSegLen + pTemp->ulPropSourceLen + pTemp->ulMetaDataLen;
          PPESEGLOGDATA pPEData = (PPESEGLOGDATA)pbData;
          if ( pPEData->ulTargetLen != 0 ) wcscpy( m_PEData.szTarget, (PSZ_W)(pbData + pPEData->ulTargetOffs) );
          if ( pPEData->ulMTTargetLen != 0 ) wcscpy( m_PEData.szMTTarget, (PSZ_W)(pbData + pPEData->ulMTTargetOffs) );
          if ( pPEData->ulMTMatchIDLen != 0 ) wcscpy( m_PEData.szMTMatchID, (PSZ_W)(pbData + pPEData->ulMTMatchIDOffs) );
          if ( pPEData->ulCopiedTargetLen != 0 ) wcscpy( m_PEData.szCopiedTarget, (PSZ_W)(pbData + pPEData->ulCopiedTargetOffs) );
          if ( pPEData->ulCopiedMatchIDLen != 0 ) wcscpy( m_PEData.szCopiedMatchID, (PSZ_W)(pbData + pPEData->ulCopiedMatchIDOffs) );
        } /* endif */
      }
      else if ( pTemp->ulEyeCatcher == ACTSEGLOG_EYECATCHER )
      {
        // convert record to enhanced version 2
        PACTSEGLOGENH pOldEntry = (PACTSEGLOGENH)pTemp;
        memset( &m_ActSegLog, 0, sizeof(m_ActSegLog) );

        m_ActSegLog.ulCheckSum         = pOldEntry->ulCheckSum;
        m_ActSegLog.ulEyeCatcher       = ACTSEGLOG2_EYECATCHER;
        m_ActSegLog.ulRecordLen        = pOldEntry->ulRecordLen + sizeof(ACTSEGLOGENH2) - sizeof(ACTSEGLOGENH);
        m_ActSegLog.ulSegLen           = pOldEntry->ulSegLen;
        m_ActSegLog.ulPropSourceLen    = pOldEntry->ulPropSourceLen;
        m_ActSegLog.ulPropSourceSegNum = pOldEntry->ulPropSourceSegNum;
        m_ActSegLog.ulSegNum           = pOldEntry->ulSegNum;
        m_ActSegLog.ulTimeStamp        = pOldEntry->ulTimeStamp;
        m_ActSegLog.ulTime             = pOldEntry->ulTime;
        m_ActSegLog.usFuzzyness        = pOldEntry->usFuzzyness;
        memcpy( &(m_ActSegLog.VersionInfo), &(pOldEntry->VersionInfo), sizeof(m_ActSegLog.VersionInfo) );
        memcpy( &(m_ActSegLog.PropTypeExists), &(pOldEntry->PropTypeExists), sizeof(m_ActSegLog.PropTypeExists) );
        memcpy( &(m_ActSegLog.PropTypeCopied), &(pOldEntry->PropTypeCopied), sizeof(m_ActSegLog.PropTypeCopied) );
        m_ActSegLog.ucNumDictCopied    = pOldEntry->ucNumDictCopied;
        m_ActSegLog.usNumTyped         = pOldEntry->usNumTyped;
        m_ActSegLog.usWordCnt          = pOldEntry->usWordCnt;
        m_ActSegLog.AddFlags.AutoSubst = pOldEntry->AddFlags.AutoSubst;
        m_ActSegLog.AddFlags.ShipmentRec= pOldEntry->AddFlags.ShipmentRec;
        m_ActSegLog.ulThinkTime = (ULONG)-1; // no think time information available
        m_ActSegLog.ulChoiceTime = (ULONG)-1; // no choice time information available
        m_ActSegLog.ulTotalTime = (ULONG)-1; // no total time information available
        m_szSegSource[0] = 0;
        if ( pOldEntry->ulSegLen != 0 ) wcscpy( m_szSegSource, (PSZ_W)((PBYTE)pOldEntry + sizeof(ACTSEGLOGENH)) );
        m_szPropDoc[0] = 0;
        if ( pOldEntry->ulPropSourceLen != 0) strcpy( m_szPropDoc, (PSZ)((PBYTE)pOldEntry + sizeof(ACTSEGLOGENH) + pOldEntry->ulSegLen) );
        m_szMetaData[0] = 0;
      } 
      else
      {
        // convert record to enhanced version 2
        PACTSEGLOGOLD pOldEntry = (PACTSEGLOGOLD)pTemp;
        memset( &m_ActSegLog, 0, sizeof(m_ActSegLog) );
        m_ActSegLog.ulSegNum               = pOldEntry->ulSegNum;       
        m_ActSegLog.PropTypeExists.None    = pOldEntry->PropTypeExists.None;
        m_ActSegLog.PropTypeExists.Exact   = pOldEntry->PropTypeExists.Exact;
        m_ActSegLog.PropTypeExists.Fuzzy   = pOldEntry->PropTypeExists.Fuzzy;
        m_ActSegLog.PropTypeExists.Replace = pOldEntry->PropTypeExists.Replace;
        m_ActSegLog.PropTypeExists.MT      = pOldEntry->PropTypeExists.MT;
        m_ActSegLog.PropTypeCopied.None    = pOldEntry->PropTypeCopied.None;
        m_ActSegLog.PropTypeCopied.Exact   = pOldEntry->PropTypeCopied.Exact;
        m_ActSegLog.PropTypeCopied.Fuzzy   = pOldEntry->PropTypeCopied.Fuzzy;
        m_ActSegLog.PropTypeCopied.Replace = pOldEntry->PropTypeCopied.Replace;
        m_ActSegLog.PropTypeCopied.MT      = pOldEntry->PropTypeCopied.MT;
        m_ActSegLog.ucNumDictCopied        = pOldEntry->ucNumDictCopied;
        m_ActSegLog.usNumTyped             = pOldEntry->usNumTyped;
        m_ActSegLog.ulTime                 = pOldEntry->ulTime & 0xFFFF;
        m_ActSegLog.usWordCnt              = pOldEntry->usWordCnt;
        m_ActSegLog.ulCheckSum             = 1; // checksum field is used for the shipment number!
        m_ActSegLog.ulThinkTime = (ULONG)-1; // no think time information available
        m_ActSegLog.ulChoiceTime = (ULONG)-1; // no choice time information available
        m_ActSegLog.ulTotalTime = (ULONG)-1; // no total time information available
        m_szSegSource[0] = 0;
        m_szPropDoc[0] = 0;
        m_szMetaData[0] = 0;
        fOldRecord = TRUE;
      } /* endif */       

      // detail output
      if ( m_fDetails )
      {
        CHAR szTemp[80];


        // when shipment changes we have to end the current segment list and start a new one
        int iNewShipment = pTemp->ulCheckSum >> 16;
        if ( iShipment != iNewShipment )
        {
          m_xw->WriteEndElement(); // "segmentList"
          iShipment = iNewShipment;
          m_xw->WriteStartElement( "segmentList" );
          m_xw->WriteStartAttribute( "shipment" );
          m_xw->WriteInt( iShipment );
          m_xw->WriteEndAttribute();
        
        } /* endif */         

        m_xw->WriteStartElement( "segment" );

        m_xw->WriteStartAttribute( "num" );
        m_xw->WriteInt( (int)m_ActSegLog.ulSegNum );
        m_xw->WriteEndAttribute();

        m_xw->WriteStartAttribute( "words" );
        m_xw->WriteInt( (int)m_ActSegLog.usWordCnt );
        m_xw->WriteEndAttribute();

        if ( fOldRecord )
        {
          // time is in seconds
          this->WriteTime( "processTime", (ULONG)(m_ActSegLog.ulTime * 100) );
        }
        else
        {
          this->WriteTime( "processTime", m_ActSegLog.ulTime );
        } /* endif */           

        if ( m_ActSegLog.ulChoiceTime != ((ULONG)-1) )
        {
          this->WriteTime( "choiceTime", m_ActSegLog.ulChoiceTime );
        } /* endif */           

        if ( m_ActSegLog.ulThinkTime != ((ULONG)-1) )
        {
          this->WriteTime( "thinkTime", m_ActSegLog.ulThinkTime );
          this->WriteTime( "editTime", m_ActSegLog.ulTime - m_ActSegLog.ulThinkTime );
        } /* endif */           

        if ( m_ActSegLog.ulTotalTime != ((ULONG)-1) )
        {
          this->WriteTime( "totalTime", m_ActSegLog.ulTotalTime );
        } /* endif */           

        m_xw->WriteStartAttribute( "charsTyped" );
        m_xw->WriteInt( (int)m_ActSegLog.usNumTyped );
        m_xw->WriteEndAttribute();

        szTemp[0] = EOS;
        if ( m_ActSegLog.PropTypeExists.Exact ) strcat( szTemp, "Exact" );
        if ( m_ActSegLog.PropTypeExists.Fuzzy) strcat( szTemp, "Fuzzy" );
        if ( m_ActSegLog.PropTypeExists.Replace ) strcat( szTemp, "Replace" );
        if ( m_ActSegLog.PropTypeExists.MT) strcat( szTemp, "Machine" );
        if ( m_ActSegLog.PropTypeExists.GlobMem) strcat( szTemp, "GlobMem" );
        if ( m_ActSegLog.PropTypeExists.GlobMemFuzzy) strcat( szTemp, "GlobMemfuzzy" );
        m_xw->WriteStartAttribute( "existingProposal" );
        m_xw->WriteString( szTemp );
        m_xw->WriteEndAttribute();

        szTemp[0] = EOS;
        if ( m_ActSegLog.PropTypeCopied.Exact ) strcat( szTemp, "Exact" );
        if ( m_ActSegLog.PropTypeCopied.Fuzzy) strcat( szTemp, "Fuzzy" );
        if ( m_ActSegLog.PropTypeCopied.Replace ) strcat( szTemp, "Replace" );
        if ( m_ActSegLog.PropTypeCopied.MT) strcat( szTemp, "Machine" );
        if ( m_ActSegLog.PropTypeCopied.GlobMem) strcat( szTemp, "GlobMem" );
        if ( m_ActSegLog.PropTypeCopied.GlobMemFuzzy) strcat( szTemp, "GlobMemfuzzy" );
        m_xw->WriteStartAttribute( "usedProposal" );
        m_xw->WriteString( szTemp );
        m_xw->WriteEndAttribute();

        if ( !fOldRecord )      
        {
          m_xw->WriteAttributeString( "autoSubst", m_ActSegLog.AddFlags.AutoSubst ? "yes" : "no" );
          if ( (m_ActSegLog.ulPropSourceSegNum != 0) && (m_ActSegLog.ulPropSourceLen != 0) )
          { 
            m_xw->WriteStartAttribute( "fuzzyness" );
            m_xw->WriteInt( (int)m_ActSegLog.usFuzzyness );
            m_xw->WriteEndAttribute();

            sprintf( m_szBuffer, "%s(%lu)", m_szPropDoc, m_ActSegLog.ulPropSourceSegNum );
            m_xw->WriteAttributeString( "propSource", m_szBuffer );
          } /* endif */           

          if ( (pTemp->VersionInfoNew.Release != 0) || (pTemp->VersionInfoNew.Version != 0) || 
               (pTemp->VersionInfoNew.Update != 0) || (pTemp->VersionInfoNew.Driver != 0) )
          {
            int ixxx = sizeof(m_ActSegLog.VersionInfoNew);
            int ixx2 = sizeof(m_ActSegLog);
            int ixx3 = sizeof(*pTemp);
            sprintf( m_szBuffer, "%d.%d.%d.%d", (SHORT)m_ActSegLog.VersionInfoNew.Version, (SHORT)m_ActSegLog.VersionInfoNew.Release,
              (SHORT)m_ActSegLog.VersionInfoNew.Update, (SHORT)m_ActSegLog.VersionInfoNew.Driver );
            m_xw->WriteAttributeString( "tm-version", m_szBuffer );
          }
          else if ( pTemp->VersionInfo.Release != 0 )
          {
            sprintf( m_szBuffer, "%d.%d.%d.%d", ((SHORT)m_ActSegLog.VersionInfo.Version + 5), (SHORT)m_ActSegLog.VersionInfo.Release,
              (SHORT)m_ActSegLog.VersionInfo.Update, (SHORT)m_ActSegLog.VersionInfo.Driver );

            m_xw->WriteAttributeString( "tm-version", m_szBuffer );
          } /* endif */           

          if ( m_ActSegLog.ulTimeStamp != 0 )
          {
            struct tm *time;
            LONG lTime = m_ActSegLog.ulTimeStamp + 10800L;  // use correct Windows time format                
            time = localtime( &lTime );
            LONG lYear = time->tm_year + 1900;

            sprintf( m_szBuffer, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d", lYear, time->tm_mon + 1, time->tm_mday,
                    time->tm_hour, time->tm_min, time->tm_sec );

            m_xw->WriteAttributeString( "timestamp", m_szBuffer );
          } /* endif */           

          if ( m_ActSegLog.ulSegLen != 0 )
          {
            m_xw->WriteStartElement( "source" );

            // remove any soft LF
            {
              PSZ_W pszSource = m_szSegSource;
              PSZ_W pszTarget = pszSource;
              while ( *pszSource != 0 )
              {
                if ( *pszSource != SOFTLF_CHAR )
                {
                  *pszTarget++  = *pszSource++;
                }
                else
                {
                  pszSource++;
                } /* endif */                   
              } /* endwhile */                 
              *pszTarget = 0;
            }

            m_xw->WriteCDataString( m_szSegSource );
            m_xw->WriteEndElement(); // "source" 
          } /* endif */           

          if ( (m_ActSegLog.ulMetaDataLen != 0) || (m_ActSegLog.ulPEDataLen != 0))
          {
            m_xw->WriteStartElement( "metadata" );

            if ( m_ActSegLog.ulMetaDataLen != 0 )
            {
              m_xw->WriteString( " " );

              // write MT metadata and ensure XML wellformedness ...
              WriteMTMetaData( m_xw, m_szMetaData );
            } /* endif */

            // write new PE data (if avaliable)#
            if ( m_PEData.szTarget[0] != 0 )
            {
              m_xw->WriteStartElement( "PE:tgt" );
              m_xw->WriteCDataString( m_PEData.szTarget  );
              m_xw->WriteEndElement(); // "PE:tgt" 
            } /* endif */

            if ( m_PEData.szCopiedTarget[0] != 0 )
            {
              m_xw->WriteStartElement( "PE:usedmatch" );

              if ( m_PEData.szCopiedMatchID[0] != 0 )
              {
                m_xw->WriteStartAttribute( "PE:ID_usedmatch" );
                m_xw->WriteString( m_PEData.szCopiedMatchID );
                m_xw->WriteEndAttribute();
              } /* endif */

              m_xw->WriteCDataString( m_PEData.szCopiedTarget );
              m_xw->WriteEndElement(); // "PE:usedmatch" 
            } /* endif */


            if ( m_PEData.szMTTarget[0] != 0 )
            {
              m_xw->WriteStartElement( "PE:mtmatch" );

              if ( m_PEData.szMTMatchID[0] != 0 )
              {
                m_xw->WriteStartAttribute( "PE:ID_mtmatch" );
                m_xw->WriteString( m_PEData.szMTMatchID );
                m_xw->WriteEndAttribute();
              } /* endif */

              m_xw->WriteCDataString( m_PEData.szMTTarget );
              m_xw->WriteEndElement(); // "PE:usedmatch" 
            } /* endif */

            m_xw->WriteEndElement(); // "metadata" 
          } /* endif */           

        } /* endif */         
        m_xw->WriteEndElement(); // "segment" 
      } /* endif */

      // update time and segment counter
      int iCat = CategoryOfSegment( m_ActSegLog.usWordCnt );
      ulMTSegs[iCat] = ulMTSegs[iCat] + 1;
      ulMTTime[iCat] = ulMTTime[iCat] + m_ActSegLog.ulTime;

      // update other counters
      if ( m_ActSegLog.ulSegNum != ulCurrentSeg )      // ignore subsequent entries for same segment  
      {
        ulCurrentSeg = m_ActSegLog.ulSegNum;
        if ( m_ActSegLog.PropTypeExists.MT )
        {
          ulMTExistWords[iCat] = ulMTExistWords[iCat] + (ULONG)m_ActSegLog.usWordCnt;
          ulMTExistSegs[iCat]  += 1;
        } /* endif */
        if ( m_ActSegLog.PropTypeCopied.MT )
        {
          ulMTUsedWords[iCat] = ulMTUsedWords[iCat] + (ULONG)m_ActSegLog.usWordCnt;
          ulMTUsedSegs[iCat]  += 1;
        } /* endif */
      } /* endif */
    } /* endif */       

    pTemp++;
  } /* endfor */

  // show results
  if ( m_hfTest ) 
  {
    fprintf( m_hfTest, "  MT Existing Segs :  %lu, %lu, %lu\n", ulMTExistSegs[0], ulMTExistSegs[1], ulMTExistSegs[2] );
    fprintf( m_hfTest, "              Words:  %lu, %lu, %lu\n", ulMTExistWords[0], ulMTExistWords[1], ulMTExistWords[2] );
    fprintf( m_hfTest, "  MT Used     Segs :  %lu, %lu, %lu\n", ulMTUsedSegs[0], ulMTUsedSegs[1], ulMTUsedSegs[2] );
    fprintf( m_hfTest, "              Words:  %lu, %lu, %lu\n", ulMTUsedWords[0], ulMTUsedWords[1], ulMTUsedWords[2] );
    fprintf( m_hfTest, "CMTEval::EvaluateLogFile returns %ld\n", iRC );
  } /* endif */

  if ( m_fDetails )
  {
    m_xw->WriteEndElement(); // "segmentlist" 
  } /* endif */

  WriteNumbers( "mtExist", "segments",  ulMTExistSegs );
  WriteNumbers( "mtExist", "words",     ulMTExistWords );
  WriteNumbers( "mtUsed",  "segments",   ulMTUsedSegs );
  WriteNumbers( "mtUsed",  "words",      ulMTUsedWords );

  WriteAverageTime( ulMTSegs, ulMTTime );

  AddNumbers( m_ulFolMTExistSegs,   ulMTExistSegs );
  AddNumbers( m_ulFolMTExistWords,  ulMTExistWords );
  AddNumbers( m_ulFolMTUsedSegs,    ulMTUsedSegs );
  AddNumbers( m_ulFolMTUsedWords,   ulMTUsedWords );
  AddNumbers( m_ulFolMTSegs,        ulMTSegs );
  AddNumbers( m_ulFolMTTime,        ulMTTime );
  m_iDocsInFolder++;

  m_xw->WriteEndElement();

  return( iRC );
} /* end of CMTEval::EvaluateLogFile */

// compare callback function for sorting the MT seg ment log
int qLogCompare
(
  const void *  p1,                // first element
  const void *  p2                 // second element
)
{
  int iRc;
  PACTSEGLOGENH2 pSegLog1 = *((PACTSEGLOGENH2 *)p1);
  PACTSEGLOGENH2 pSegLog2 = *((PACTSEGLOGENH2 *)p2);

  ULONG ulSequ1, ulSequ2, ulSeg1, ulSeg2, ulShip1, ulShip2;

  if ( pSegLog1->ulEyeCatcher == ACTSEGLOG2_EYECATCHER )
  {
    ulSequ1 = pSegLog1->ulCheckSum & 0xFFFF;
    ulSeg1 = pSegLog1->ulSegNum;
    ulShip1 = (pSegLog1->ulCheckSum & 0xFFFF0000) >> 16;
  }
  else if ( pSegLog1->ulEyeCatcher == ACTSEGLOG_EYECATCHER )
  {
    PACTSEGLOGENH pOldSegLog = *((PACTSEGLOGENH *)p1);
    ulSequ1 = pOldSegLog->ulCheckSum & 0xFFFF;
    ulSeg1 = pOldSegLog->ulSegNum;
    ulShip1 = (pOldSegLog->ulCheckSum & 0xFFFF0000) >> 16;
  }
  else
  {
    PACTSEGLOGOLD pOldEntry = (PACTSEGLOGOLD)pSegLog1;
    ulSequ1 = (pOldEntry->ulTime & 0xFFFF0000) >>16;
    ulSeg1 = pOldEntry->ulSegNum;
    ulShip1 = 1;
  } /* endif */         

  if ( pSegLog2->ulEyeCatcher == ACTSEGLOG2_EYECATCHER )
  {
    ulSequ2 = pSegLog2->ulCheckSum & 0xFFFF;
    ulSeg2 = pSegLog2->ulSegNum;
    ulShip2 = (pSegLog2->ulCheckSum & 0xFFFF000) >> 16;
  }
  else if ( pSegLog2->ulEyeCatcher == ACTSEGLOG_EYECATCHER )
  {
    PACTSEGLOGENH pOldSegLog = *((PACTSEGLOGENH *)p2);
    ulSequ2 = pOldSegLog->ulCheckSum & 0xFFFF;
    ulSeg2 = pOldSegLog->ulSegNum;
    ulShip2 = (pOldSegLog->ulCheckSum & 0xFFFF0000) >> 16;
  }
  else
  {
    PACTSEGLOGOLD pOldEntry = (PACTSEGLOGOLD)pSegLog2;
    ulSequ2 = (pOldEntry->ulTime & 0xFFFF0000) >>16;
    ulSeg2 = pOldEntry->ulSegNum;
    ulShip2 = 1;
  } /* endif */         

  if ( ulShip1 == ulShip2 )
  {
    if ( ulSeg1 == ulSeg2 )
    {
      iRc = (int)ulSequ1 - (int)ulSequ2;
    }
    else
    {
      iRc = (int)ulSeg1 - (int)ulSeg2;
    } /* endif */
  }
  else
  {
      iRc = (int)ulShip1 - (int)ulShip2;
  } /* endif */     

  return iRc;
}

// get the category (simple, medium, complex ) for a segment
int CMTEval::CategoryOfSegment( int iWords )
{
  int iCat = 0;

  if ( iWords < SIMPLE_SENT_BOUND )
  {
    iCat = 0;
  }
  else if ( iWords  < MEDIUM_SENT_BOUND )
  {
    iCat = 1;
  }
  else
  {
    iCat = 2;
  } /* endif */

  return( iCat );
} /* end of CMTEval::CategoryOfSegment */

int CMTEval::WriteAverageTime( ULONG ulSegs[3], ULONG ulTime[3] )
{
  ULONG ulAveTime[3];

  for( int i = 0; i <= 2; i++ )
  {
    if ( ulSegs[i] )
    {
      ulAveTime[i] = ulTime[i] / ulSegs[i];
    }
    else
    {
      ulAveTime[i] = 0;
    } /* endif */
  } /* endfor */
  WriteTimePerCategory( "averageTimePerSeg",  "seconds", ulAveTime );

  return( 0 );
}

int CMTEval::WriteTime( const char *pszName, ULONG ulTime )
{
  char szTemp[20];
  m_xw->WriteStartAttribute( pszName );
  int iSeconds = (int)ulTime / 100;
  int iCentiSeconds = (int)ulTime - (iSeconds * 100);
  sprintf( szTemp, "%ld.%2.2ld", iSeconds, iCentiSeconds );
  m_xw->WriteString( szTemp );
  m_xw->WriteEndAttribute();
  return( 0 );
}

int CMTEval::WriteNumbers( const char *pszName, const char *pszType, ULONG ulCount[3] )
{
  m_xw->WriteStartElement( "counts" );
  m_xw->WriteAttributeString( "type", pszName );
  m_xw->WriteAttributeString( "base", pszType );
  m_xw->WriteStartAttribute( "simple" );
  m_xw->WriteInt( (int)ulCount[0] );
  m_xw->WriteEndAttribute();
  m_xw->WriteStartAttribute( "medium" );
  m_xw->WriteInt( (int)ulCount[1] );
  m_xw->WriteEndAttribute();
  m_xw->WriteStartAttribute( "complex" );
  m_xw->WriteInt( (int)ulCount[2] );
  m_xw->WriteEndAttribute();
  m_xw->WriteStartAttribute( "sum" );
  m_xw->WriteInt( (int)(ulCount[0]+ulCount[1]+ulCount[2]) );
  m_xw->WriteEndAttribute();
  m_xw->WriteEndElement();

  return( 0 );
} /* end of CMTEval::WriteNumbers */

int CMTEval::WriteTimePerCategory( const char *pszName, const char *pszType, ULONG ulCount[3] )
{
  m_xw->WriteStartElement( "counts" );
  m_xw->WriteAttributeString( "type", pszName );
  m_xw->WriteAttributeString( "base", pszType );
  this->WriteTime( "simple", ulCount[0] );
  this->WriteTime( "medium", ulCount[1] );
  this->WriteTime( "complex", ulCount[2] );

  ULONG ulTotalTime = 0;
  int iCategories = 0;
  for ( int i=0; i<3; i++ )
  {
    if ( ulCount[i] != 0 )
    {
      ulTotalTime += ulCount[i];
      iCategories++;
    } /* endif */       
  } /* endfor */
  ULONG ulAveTime = ( iCategories != 0) ? ((ULONG)(ulTotalTime / iCategories)) : 0;
  this->WriteTime( "average", ulAveTime );
  m_xw->WriteEndElement();

  return( 0 );
} /* end of CMTEval::WriteNumbers */



int CMTEval::AddNumbers( ULONG ulCountSum[3], ULONG ulCountAdd[3] )
{
  for( int i = 0; i <= 2; i++ )
  {
    ulCountSum[i] += ulCountAdd[i];
  } /* endfor */
  return( 0 );
} /* end of CMTEval::AddNumbers */

int CMTEval::CopyNumbers( ULONG ulCountTo[3], ULONG ulCountFrom[3] )
{
  for( int i = 0; i <= 2; i++ )
  {
    ulCountTo[i] = ulCountFrom[i];
  } /* endfor */
  return( 0 );
} /* end of CMTEval::CopyNumbers */

int CMTEval::ClearNumbers( ULONG ulCount[3] )
{
  for( int i = 0; i <= 2; i++ )
  {
    ulCount[i] = 0;
  } /* endfor */
  return( 0 );
} /* end of CMTEval::ClearNumbers */

int CMTEval::AddFolderNumbersToTotal( void )
{
  AddNumbers( m_ulSumMTTotalSegs,     m_ulFolMTTotalSegs );
  AddNumbers( m_ulSumMTTotalWords,    m_ulFolMTTotalWords );
  AddNumbers( m_ulSumMTSendSegs,      m_ulFolMTSendSegs );
  AddNumbers( m_ulSumMTSendWords,     m_ulFolMTSendWords );
  AddNumbers( m_ulSumMTReceivedSegs,  m_ulFolMTReceivedSegs );
  AddNumbers( m_ulSumMTReceivedWords, m_ulFolMTReceivedWords );
  AddNumbers( m_ulSumMTExistSegs,     m_ulFolMTExistSegs );
  AddNumbers( m_ulSumMTExistWords,    m_ulFolMTExistWords );
  AddNumbers( m_ulSumMTUsedSegs,      m_ulFolMTUsedSegs );
  AddNumbers( m_ulSumMTUsedWords,     m_ulFolMTUsedWords );
  AddNumbers( m_ulSumMTSegs,          m_ulFolMTSegs );
  AddNumbers( m_ulSumMTTime,          m_ulFolMTTime );
  m_iFoldersProcessed += 1;

  return( 0 );
} /* end of CMTEval::AddFolderCountsToTotal */

// write folder values (summary of documents)
int CMTEval::WriteFolderNumbers( void )
{
  m_xw->WriteStartElement( "foldertotal" );
  m_xw->WriteStartAttribute( "documents" );
  m_xw->WriteInt( m_iDocsInFolder );
  m_xw->WriteEndAttribute();
  WriteNumbers( "mtExist", "segments",  m_ulFolMTExistSegs );
  WriteNumbers( "mtExist", "words",     m_ulFolMTExistWords );
  WriteNumbers( "mtUsed",  "segments",  m_ulFolMTUsedSegs );
  WriteNumbers( "mtUsed",  "words",     m_ulFolMTUsedWords );
  WriteAverageTime( m_ulFolMTSegs, m_ulFolMTTime );
  m_xw->WriteEndElement();

  return( 0 );
} /* end of CMTEval::WriteFolderNumbers */

// write overall totals (summary of all processed folders)
int CMTEval::WriteTotals( void )
{
  m_xw->WriteStartElement( "summary" );
  m_xw->WriteStartAttribute( "folders" );
  m_xw->WriteInt( m_iFoldersProcessed );
  m_xw->WriteEndAttribute();
  WriteNumbers( "total",   "segments",   m_ulSumMTTotalSegs );
  WriteNumbers( "total",   "words",      m_ulSumMTTotalWords );
  //WriteNumbers( "send",    "segments",   m_ulSumMTSendSegs );
  //WriteNumbers( "send",    "words",      m_ulSumMTSendWords );
  //WriteNumbers( "received","segments",   m_ulSumMTReceivedSegs );
  //WriteNumbers( "received","words",      m_ulSumMTReceivedWords );
  WriteNumbers( "mtExist", "segments",   m_ulSumMTExistSegs );
  WriteNumbers( "mtExist", "words",      m_ulSumMTExistWords );
  WriteNumbers( "mtUsed",  "segments",   m_ulSumMTUsedSegs );
  WriteNumbers( "mtUsed",  "words",      m_ulSumMTUsedWords );
  WriteAverageTime( m_ulSumMTSegs, m_ulSumMTTime );
  m_xw->WriteEndElement();

  return( 0 );
} /* end of CMTEval::WriteTotals */

// write MT data to XML output and ensure that the data is wellformed XML
int CMTEval::WriteMTMetaData( CXmlWriter *pWriter, PSZ_W pszMetaData )
{
  PSZ_W pszNameStart;                  // start of element name or attribute name
  PSZ_W pszNameEnd;                    // end of current name
  int iOpenElements = 0;

  // replace any typografic double quotes by 'normal' ones
  pszNameStart = pszMetaData;
  while ( *pszNameStart != 0 )
  {
    if ( (*pszNameStart == L'“' ) || (*pszNameStart == L'”') || (*pszNameStart == L'“' ) || (*pszNameStart == L'„') )
    {
      *pszNameStart = '\"';
    }
    pszNameStart++;
  }

  // skip any whitespace
  while ( (*pszMetaData == L' ') || (*pszMetaData == L'\n') || (*pszMetaData == L'\r') ) pszMetaData++;

  while( *pszMetaData != 0 )
  {
    // skip any whitespace
    while ( (*pszMetaData == L' ') || (*pszMetaData == L'\n') || (*pszMetaData == L'\r') ) pszMetaData++;

    // find end of name of next element
    if ( *pszMetaData == L'<' )
    {
      // found the start of an XML element...
      pszMetaData++;

      // get the name of element
      pszNameStart = pszMetaData;
      if ( *pszMetaData == L'/') pszMetaData++;
      while ( iswalpha(*pszMetaData) || (*pszMetaData == L':') || (*pszMetaData == L'.') ) pszMetaData++;
      pszNameEnd = pszMetaData;

      // skip any whitespace
      while ( (*pszMetaData == L' ') || (*pszMetaData == L'\n') || (*pszMetaData == L'\r') ) pszMetaData++;

      // handle start or end element
      if ( *pszNameStart == L'/' )
      {
        // the closing part of the element
        if ( iOpenElements != 0 )
        {
          pWriter->WriteEndElement();
          iOpenElements--;
        }
      }
      else
      {
        // terminate name part
        CHAR_W chTemp = *pszNameEnd;
        *pszNameEnd = 0;

        // convert any colon in the name to a dash
        PSZ_W pszCurChar = pszNameStart;
        while ( *pszCurChar != 0 )
        {
          if ( *pszCurChar == L':' ) *pszCurChar = L'-';
          pszCurChar++;
        } /* endwhile */

        // start of a new element 
        pWriter->WriteStartElement( pszNameStart );
        iOpenElements++;
        *pszNameEnd = chTemp;
      }

      // skip any whitespace
      while ( (*pszMetaData == L' ') || (*pszMetaData == L'\n') || (*pszMetaData == L'\r') ) pszMetaData++;

      // process any attributes
      while ( (*pszMetaData != 0) && (*pszMetaData != L'/') && (*pszMetaData != L'>') )
      {   
        // get the name of the attribute
        pszNameStart = pszMetaData;
        while ( iswalnum(*pszMetaData) || (*pszMetaData == L':') || (*pszMetaData == L'-') || (*pszMetaData == L'_') || (*pszMetaData == L'.') ) pszMetaData++;
        pszNameEnd = pszMetaData;

        // terminate name 
        CHAR_W chTemp = *pszNameEnd;
        *pszNameEnd = 0;

        // convert any colon in the name to a dash
        PSZ_W pszCurChar = pszNameStart;
        while ( *pszCurChar != 0 )
        {
          if ( *pszCurChar == L':' ) *pszCurChar = L'-';
          pszCurChar++;
        } /* endwhile */

        // write attribute name
        pWriter->WriteStartAttribute( pszNameStart );
        *pszNameEnd = chTemp;

        // skip any whitespace
        while ( (*pszMetaData == L' ') || (*pszMetaData == L'\n') || (*pszMetaData == L'\r') ) pszMetaData++;

        // get any attribute data
        if ( *pszMetaData == L'=' )
        {
          *pszMetaData++;

          // skip any whitespace
          while ( (*pszMetaData == L' ') || (*pszMetaData == L'\n') || (*pszMetaData == L'\r') ) pszMetaData++;

          // get data enclosed in quotes
          if ( *pszMetaData == L'\"' )
          {
            pszMetaData++;
            pszNameStart = pszMetaData;
            while ( (*pszMetaData != 0) && (*pszMetaData != L'\"/') ) pszMetaData++;
            chTemp = *pszMetaData;
            *pszMetaData = 0;
            pWriter->WriteString( pszNameStart );
            *pszMetaData = chTemp;
            if ( *pszMetaData == L'\"/' ) pszMetaData++;
          }
          else if ( *pszMetaData == L'\'' )
          {
            pszMetaData++;
            pszNameStart = pszMetaData;
            while ( (*pszMetaData != 0) && (*pszMetaData != L'\'/') ) pszMetaData++;
            chTemp = *pszMetaData;
            *pszMetaData = 0;
            pWriter->WriteString( pszNameStart );
            *pszMetaData = chTemp;
            if ( *pszMetaData == L'\'/' ) pszMetaData++;
          }
          else
          {
            pszNameStart = pszMetaData;
            while ( (*pszMetaData != 0) && (*pszMetaData != L'>') && (*pszMetaData != L'/') && (*pszMetaData != L' ') ) pszMetaData++;
            chTemp = *pszMetaData;
            *pszMetaData = 0;
            pWriter->WriteString( pszNameStart );
            *pszMetaData = chTemp;
          }
        }

        // end attribute
        pWriter->WriteEndAttribute();

        // skip any whitespace
        while ( (*pszMetaData == L' ') || (*pszMetaData == L'\n') || (*pszMetaData == L'\r') ) pszMetaData++;
      }

      // handle self contained elements
      if ( *pszMetaData == L'/' )
      {
        *pszMetaData++; 
        if ( iOpenElements != 0 )
        {
          pWriter->WriteEndElement();
          iOpenElements--;
        }

        // skip any whitespace
        while ( (*pszMetaData == L' ') || (*pszMetaData == L'\n') || (*pszMetaData == L'\r') ) pszMetaData++;
      }

      // handle end of element
      if ( *pszMetaData == L'>' )
      {
        *pszMetaData++; 

        // skip any whitespace
        while ( (*pszMetaData == L' ') || (*pszMetaData == L'\n') || (*pszMetaData == L'\r') ) pszMetaData++;
      }
    }
    else
    {
      // un-identified data ...
      PSZ_W pszData = pszMetaData;

      // find end of data
      while ( (*pszMetaData != L'<') && (*pszMetaData != 0) ) pszMetaData++;

      // write data to XML file 
      CHAR_W chTemp = *pszMetaData;
      *pszMetaData = 0;
      pWriter->WriteString( pszData );
      *pszMetaData = chTemp;
    }
  }

  // close any open elements
  while( iOpenElements )
  {
    pWriter->WriteEndElement();
    iOpenElements--;
  }
  return( 0 );
}

void showHelp()
{
    printf( "OtmMtEval.EXE         : MT Log evaluation tool\n" );
    printf( "Version               : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright             : %s\n",STR_COPYRIGHT );
    printf( "Purpose               : Extracts MT LOG information recorded during the translation process into XML files\n" );
    printf( "Syntax format         : OtmMtEval filespec [outfile.xml] [/details]\n" );
    printf( "\n" );
    printf( "Options and parameters:\n" );
    printf( "   filespec    is either a\n" ); 
    printf( "               long folder name (e.g. SampleFolder)\n" );
    printf( "               a short folder name with a .F00 extension (e.g. SAMPLE.F00)\n" );
    printf( "               the name of an exported folder (e.g. C:\\EXPORTS\\SAMPLE.FXP)\n" );
    printf( "               the name of a directory containing exported folders (e.g. C:\\EXPORTS\\\n" );
    printf( "   outfile.xml is the fully qualified name of the output file\n" );
    printf( "   /details    activates the output of the segment detail information\n" );
}
