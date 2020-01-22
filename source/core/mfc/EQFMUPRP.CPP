/*! \file
	Description: Code of the markup table properties classes

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqfmfc.h"

#ifndef CPPTEST
extern "C"
{
#endif
  #define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_DLGUTILS         // dialog utilities
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_EDITORAPI        // editor API
  #define INCL_EQF_TP               // public translation processor functions
  #define INCL_EQF_FOLDER           // folder related functions

  // ensure correct packing of our structures
  #pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                  // General .H for EQF
  #include "eqftag00.h"             // tag table functions
  #include "eqftimp.h"              // external tag table creation interface


  #pragma pack( pop, TM2StructPacking )
#ifndef CPPTEST
}
#endif

#include "EQF.ID"
#include "eqfmuprp.h"                  // our class definitions

#include "core\pluginmanager\PluginManager.h"
#include "core\pluginmanager\OtmMarkup.h"
#include "core\pluginmanager\OtmMarkupPlugin.h"
#include "MarkupPluginMapper.h"


// arrays with styles and corresponding string ID, arrays must be terminated by two zeroes
int aiCharsetStyles[] = { SID_TAGCHARSET_ASCII,   TAGCHARSET_ASCII,
                          SID_TAGCHARSET_ANSI,    TAGCHARSET_ANSI,
                          SID_TAGCHARSET_UNICODE, TAGCHARSET_UNICODE,
                          SID_TAGCHARSET_UTF8,    TAGCHARSET_UTF8,
                          0,                      0 };
// arrays with styles and corresponding string ID, arrays must be terminated by two zeroes
int aiReflowStyles[] = {SID_TAGREFLOW_NOTSPECIFIED, TAGREFLOW_NOTSPECIFIED,
                        SID_TAGREFLOW_YES,       TAGREFLOW_YES,
                        SID_TAGREFLOW_NO,       TAGREFLOW_NO,
                        0,                      0 };

int aiTagTypes[] = { SID_STARTDEL_TAG,     STARTDEL_TAG,
                     SID_ENDDEL_TAG,       ENDDEL_TAG,
                     SID_SELC_TAG,         SELC_TAG,
                     SID_STARTNEUTRAL_TAG, STARTNEUTRAL_TAG,
                     SID_ENDNEUTRAL_TAG,   ENDNEUTRAL_TAG,
                     0,                    0 };

int aiSegBreakStyles[] = { SID_SEG_OFF,      SEG_OFF,
                           SID_SEG_NEUTRAL,  SEG_NEUTRAL,
                           SID_SEG_ON,       SEG_ON,
                           SID_SEG_RESET,    SEG_RESET,
                           SID_SEG_LINE,     SEG_LINE,
                           SID_SEG_NOSEGMENTING,     SEG_NOSEGMENTING,
                           SID_PROTECT_ON,   PROTECT_ON,
                           SID_PROTECT_OFF,  PROTECT_OFF,
                           0,                0 };

int aiAssTextStyles[] = { SID_TSL_TEXT,      TSL_TEXT,
                          SID_TSNL_TEXT,     TSNL_TEXT,
                          SID_NOEXPL_TEXT,   NOEXPL_TEXT,
                          0,                 0 };

int aiClassStyles[] = { SID_CLS_DEFAULT,   CLS_DEFAULT,
                        SID_CLS_HEAD,      CLS_HEAD,
                        SID_CLS_EHEAD,     CLS_EHEAD,
                        SID_CLS_PARA,      CLS_PARA,
                        SID_CLS_EPARA,     CLS_EPARA,
                        SID_CLS_XTRCT,     CLS_XTRCT,
                        SID_CLS_EXTRCT,    CLS_EXTRCT,
                        SID_CLS_LIST,      CLS_LIST,
                        SID_CLS_ELIST,     CLS_ELIST,
                        SID_CLS_HILIT,     CLS_HILIT,
                        SID_CLS_EHILIT,    CLS_EHILIT,
                        SID_CLS_PROTECT,   CLS_PROTECT,
                        SID_CLS_TREF,      CLS_TREF,
                        SID_CLS_LREF,      CLS_LREF,
                        SID_CLS_ADDINF,    CLS_ADDINF,
                        SID_CLS_EADDINF,   CLS_EADDINF,
                        SID_CLS_UNCHG,     CLS_UNCHG,
                        SID_CLS_VAR,       CLS_VAR,
                        SID_CLS_WHITESPACE,CLS_WHITESPACE,
                        SID_CLS_TRANSLVAR, CLS_TRANSLVAR,
                        0,                 0   };


PSZ pszLegend =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1031{\\fonttbl{\\f0\\fnil\\fcharset0 Times New Roman;}}"
"{\\colortbl ;\\red255\\green0\\blue0;}"
"\\viewkind4\\uc1\\pard\\f0\\fs20 \"Text\" = translatable, \"\\cf1 Text\\cf0 \" = protected, \"\\cf1 <P>\\cf0 \" = segment boundary\\cf0\\par"
"}";

IMPLEMENT_DYNAMIC(CMUPropSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CMUPropSheet, CPropertySheet)
        //{{AFX_MSG_MAP(CMUPropSheet)
        ON_COMMAND( ID_APPLY_NOW, OnProtectPB )
        ON_COMMAND( IDOK, OnOKPB )
        ON_COMMAND( IDCANCEL, OnCancelPB )
        ON_COMMAND( IDHELP, OnHelpPB )
        ON_COMMAND( ID_MUPROP_TEST_LOAD_PB, OnTestLoadPB )
        ON_COMMAND( ID_MUPROP_TEST_ADDATTR_PB, OnTestAddAttrPB )
        ON_COMMAND( ID_MUPROP_TEST_ADDTAG_PB, OnTestAddTagPB )
        ON_COMMAND( ID_MUPROP_TEST_REFRESH_PB, OnTestRefreshPB )
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// constructor
CMUPropSheet::CMUPropSheet(CWnd* pWndParent)
        : CPropertySheet("Markup table properties", pWndParent)
{
        AddPage(&m_generalPage);
        AddPage(&m_tagsPage);
        AddPage(&m_attrPage);
        AddPage(&m_testPage);
        m_szTagTableObjName[0] = EOS;
        m_pTagTable = NULL;

        // create tagtable test environment (table, folder, mem,...)
        {
          // setup fully qualified test file names
          char szFolderName[MAX_FILESPEC];
          strcpy( szFolderName, MUPROP_TESTFOL );
          strcat( szFolderName, EXT_FOLDER_MAIN );
          UtlMakeEQFPath( m_szDocProp, NULC, PROPERTY_PATH, szFolderName );
          strcat( m_szDocProp, BACKSLASH_STR );
          strcat( m_szDocProp, MUPROP_TESTDOC );
          UtlMakeEQFPath( m_szDocObj, NULC, SYSTEM_PATH, szFolderName );
          strcat( m_szDocObj, BACKSLASH_STR );
          strcat( m_szDocObj, MUPROP_TESTDOC );
          UtlMakeEQFPath( m_szDocSource, NULC, DIRSOURCEDOC_PATH, szFolderName );
          strcat( m_szDocSource, BACKSLASH_STR );
          strcat( m_szDocSource, MUPROP_TESTDOC );
          UtlMakeEQFPath( m_szDocSeg, NULC, DIRSEGSOURCEDOC_PATH, szFolderName );
          strcat( m_szDocSeg, BACKSLASH_STR );
          strcat( m_szDocSeg, MUPROP_TESTDOC );
          UtlMakeEQFPath( m_szDocRTF, NULC, DIRTARGETDOC_PATH, szFolderName );
          strcat( m_szDocRTF, BACKSLASH_STR );
          strcat( m_szDocRTF, MUPROP_TESTDOC );
          UtlMakeEQFPath( m_szTestTable, NULC, TABLE_PATH, NULL );
          strcat( m_szTestTable, BACKSLASH_STR );
          strcat( m_szTestTable, MUPROP_TESTTABLE );
          strcat( m_szTestTable, EXT_OF_FORMAT );

          // delete any old objects
          FolDeleteIntFolder( MUPROP_TESTFOL );
          UtlDelete( m_szTestTable, 0L, FALSE );

          // write dummy tagtable file (also prereq for create folder function)
          UtlWriteFile( m_szTestTable, 4, "DUMMY", FALSE );

          // create the test folder
          USHORT usRC = FolCreateIntFolder( MUPROP_TESTFOL, NULL, NULC,
                                            NULL, MUPROP_TESTTABLE,
                                            "STANDARD", MUPROP_TESTLANG,
                                            MUPROP_TESTLANG );
          if ( usRC != NO_ERROR )
          {
            // GQTODO: Message: no test env.
            RemovePage(&m_testPage);
          } /* endif */
        }
}

// destructor
CMUPropSheet::~CMUPropSheet()
{
  if ( m_pTagTable )
  {
    UtlAlloc( (PVOID *)&m_pTagTable, 0L, 0L, NOMSG );
  }
  // delete any test documents
  UtlDelete( m_szDocSource, 0L, FALSE );
  UtlDelete( m_szDocRTF, 0L, FALSE );
  UtlDelete( m_szDocSeg, 0L, FALSE );

  // delete test environment
  FolDeleteIntFolder( MUPROP_TESTFOL );
  UtlDelete( m_szTestTable, 0L, FALSE );
}

int CMUPropSheet::OnCreate(LPCREATESTRUCT lpCreateStruct )
{
  int iResult = CPropertySheet::OnCreate( lpCreateStruct );

  return( iResult );
}

BOOL CMUPropSheet::OnInitDialog()
{
  BOOL bResult = FALSE;
  BOOL bProtected = FALSE ;

  // rename property sheet pushbuttons to Tmgr standards ...
  if ( m_szTagTableObjName[0] != 0 )
  {
    SetDlgItemText( IDOK, "Change" );
  }
  else
  {
    SetWindowText( "Create A new markup table" );
    SetDlgItemText( IDOK, "Create" );
  } /* endif */
  SetDlgItemText( IDCANCEL, "Cancel" );
  SetDlgItemText( ID_APPLY_NOW, "Protect" );
  SetDlgItemText( IDHELP, "Help" );

  // enable apply/protect pushbutton
  m_generalPage.SetModified( TRUE );
  (GetDlgItem( ID_APPLY_NOW ))->EnableWindow( TRUE );

  // load tag table if tag table has been set
  if ( m_szTagTableObjName[0] != 0 )
  {
    m_fNewTagTable = FALSE;
    m_generalPage.m_fNewTagTable = FALSE;

    // isolate tag table name (object name is plugin name + ':' + markup name)
    char *pszName = strrchr( m_szTagTableObjName, ':' );
    if ( pszName != NULL )
    {
      strcpy( m_szTagTableName, pszName + 1 );
    }
    else
    {
      strcpy( m_szTagTableName, m_szTagTableObjName );
    }
    MUGetMarkupTableFileName( m_szTagTableName, NULL, m_szTagTableFileName, sizeof(m_szTagTableFileName) );
    bProtected = isMarkupProtected( m_szTagTableName, NULL );

    ULONG ulRead = 0;
    if ( UtlLoadFileL( m_szTagTableFileName, (PVOID *)&(m_pTagTable), &ulRead, FALSE, TRUE ) )
    {
       if ( ! m_pTagTable->fProtected ) 
          m_pTagTable->fProtected = bProtected ;

      // get protection state
      if ( m_pTagTable->usVersion >= TAGTABLE_VERSION3 )
      {
   		m_generalPage.m_fReflow    = m_pTagTable->fReflow;
        m_fProtected = m_pTagTable->fProtected;
        m_ulPassword = m_pTagTable->ulPassword;
        if ( m_fProtected )
        {
          SetDlgItemText( ID_APPLY_NOW, "Unprotect" );
          if ( bProtected ) {
             (GetDlgItem( ID_APPLY_NOW ))->EnableWindow( FALSE );
             (GetDlgItem( IDOK ))->EnableWindow( FALSE );
          }
          m_generalPage.Protect( TRUE );
          m_tagsPage.Protect( TRUE );
          m_attrPage.Protect( TRUE );
          m_testPage.Protect( TRUE );
        } /* endif */
      }
      else
      {
        m_fProtected = FALSE;
        m_ulPassword = 0;
      } /* endif */

      // fill General property page
      strcpy( m_generalPage.m_szTableName, m_szTagTableName );
      strcpy( m_generalPage.m_szUserExit, m_pTagTable->szSegmentExit );
      if ( m_pTagTable->usVersion >= TAGTABLE_VERSION3 )
      {
        strcpy( m_generalPage.m_szDescription, m_pTagTable->szDescription );
        OEMTOANSI( m_generalPage.m_szDescription );
        if ( m_pTagTable->szDescriptiveName[0] != EOS )
        {
          strcpy( m_generalPage.m_szDescrName, m_pTagTable->szDescriptiveName );
        }
        else
        {
          m_generalPage.m_szDescrName[0] = EOS;
          if ( stricmp( m_generalPage.m_szTableName,
                        m_generalPage.m_szDescrName ) == 0 )
          {
            m_generalPage.m_szDescrName[0] = EOS;
          } /* endif */
        } /* endif */
        OEMTOANSI( m_generalPage.m_szDescrName );
        m_generalPage.m_usCharSet = m_pTagTable->usCharacterSet;
        m_generalPage.m_chSingleSubstChar = m_pTagTable->chSingleSubst;
        m_generalPage.m_chMultSubstChar = m_pTagTable->chMultSubst;
        strcpy( m_generalPage.m_szTrNoteStart, m_pTagTable->chStartText );
        OEMTOANSI( m_generalPage.m_szTrNoteStart );
        strcpy( m_generalPage.m_szTrNoteLevel1, m_pTagTable->chTrnote1Text );
        OEMTOANSI( m_generalPage.m_szTrNoteLevel1 );
        strcpy( m_generalPage.m_szTrNoteLevel2, m_pTagTable->chTrnote2Text );
        OEMTOANSI( m_generalPage.m_szTrNoteLevel2 );
        strcpy( m_generalPage.m_szTrNoteEnd, m_pTagTable->chEndText );
        OEMTOANSI( m_generalPage.m_szTrNoteEnd );


        OtmMarkup *markup = GetMarkupObject( m_szTagTableName, NULL );
        if ( markup != NULL )
        {
          markup->getLongDescription( m_generalPage.m_szLongDescription, sizeof(m_generalPage.m_szLongDescription)-1 );
          markup->getShortDescription( m_generalPage.m_szShortDescription, sizeof(m_generalPage.m_szShortDescription)-1 );
          m_generalPage.m_szPlugin[0] = EOS ;
          OtmMarkupPlugin *plugin = GetMarkupPlugin( m_szTagTableName );
          if ( plugin != NULL ) {
             strncpy( m_generalPage.m_szPlugin, plugin->getShortName(), sizeof(m_generalPage.m_szPlugin) );
             m_generalPage.m_szPlugin[sizeof(m_generalPage.m_szPlugin)-1] = EOS;
             strncpy( m_generalPage.m_szSupplier, plugin->getSupplier(), sizeof(m_generalPage.m_szSupplier) );
             m_generalPage.m_szSupplier[sizeof(m_generalPage.m_szSupplier)-1] = EOS;
          } else {
             m_generalPage.m_szPlugin[0] = EOS ;
             m_generalPage.m_szSupplier[0] = EOS ;
          }
          markup->getVersion( m_generalPage.m_szVersion, sizeof(m_generalPage.m_szVersion)-1 );
        } else {
           strcpy( m_generalPage.m_szLongDescription, m_pTagTable->szDescription );
           strcpy( m_generalPage.m_szShortDescription, m_pTagTable->szDescription );
           m_generalPage.m_szPlugin[0] = EOS ;
           m_generalPage.m_szSupplier[0] = EOS ;
           m_generalPage.m_szVersion[0] = EOS ;
        }

      } /* endif */

      // fill Tags property page
      {
        MUPRPTAG stTag;
        PTAG pTag = (PTAG)( ((PBYTE)(m_pTagTable)) + m_pTagTable->stFixTag.uOffset);
        PTAGADDINFO pAddInfo = (m_pTagTable->usVersion >= ADDINFO_VERSION) ?
                               ((PTAGADDINFO)((PBYTE)m_pTagTable + m_pTagTable->uAddInfos))  :
                               NULL;
        PSZ pszStringOffs = (PSZ)m_pTagTable + m_pTagTable->uTagNames;

        for( int i = 0; i < m_pTagTable->uNumTags; i++, pTag++ )
        {
          memset( &stTag, 0, sizeof(stTag) );
          ConvertTagString( pszStringOffs + pTag->uTagnameOffs, stTag.szName );
          ConvertTagString( pszStringOffs + pTag->uEndDelimOffs, stTag.szEndDelim );
          stTag.Tagtype = pTag->Tagtype;
          stTag.Segbreak = pTag->Segbreak;
          stTag.Asstext = pTag->Asstext;
          stTag.fAttr = pTag->fAttr;
          stTag.usLength = pTag->usLength;
          stTag.usPosition = pTag->usPosition;
          stTag.fTranslate = pTag->BitFlags.fTranslate;
          stTag.usAddInfo = pTag->BitFlags.AddInfo;
          if ( pAddInfo )
          {
            stTag.ClassId = pAddInfo[i].ClassId;
            stTag.usFixedTagId = pAddInfo[i].usFixedTagId;
          } /* endif */

          m_tagsPage.AddTag( &stTag );
        } /* endfor */
      }

      // fill attribute table of attributes property page
      {
        MUPRPATTR stAttr;
        PSZ pszStringOffs = (PSZ)m_pTagTable + m_pTagTable->uTagNames;

        int iAttrCount = m_pTagTable->stAttribute.uNumber;
        for ( int i=0 ; i <= 26 ; i++ ) iAttrCount += m_pTagTable->stAttributeIndex[i].uNumber;
        PATTRIBUTE pAttribute = (PATTRIBUTE)(((PBYTE)(m_pTagTable)) + m_pTagTable->stAttribute.uOffset);

        for( int i = 0; i < iAttrCount; i++ )
        {
          ATTRIBUTE Attribute = pAttribute[i];
          memset( &stAttr, 0, sizeof(stAttr) );
          ConvertTagString( pszStringOffs + Attribute.uStringOffs, stAttr.szName );
          ConvertTagString( pszStringOffs + Attribute.uEndDelimOffs, stAttr.szEndDelim  );
          stAttr.usLength = Attribute.usLength;
          stAttr.fTranslate = Attribute.BitFlags.fTranslate;
          stAttr.usAddInfo = Attribute.BitFlags.AddInfo;

          // fTRNote and usFixedAttrId are not contained in SGML version of
          // tag table, so we ignore these two fields for now ...
//          stAttr.fTRNote  = Attribute.BitFlags.fTRNote;
//          if ( pAddInfo )
//          {
//            stAttr.usFixedAttrId = pAddInfo[m_pTagTable->uNumTags+i].usFixedTagId;
//          } /* endif */

          m_attrPage.AddAttr( &stAttr );
        } /* endfor */
      }

    }
    else
    {
    } /* endif */
  }
  else
  {
    // a new tag table
    m_fNewTagTable = TRUE;
    m_generalPage.m_fNewTagTable = TRUE;
    m_fProtected = FALSE;
    m_ulPassword = 0;
    m_generalPage.m_fReflow = TAGREFLOW_NOTSPECIFIED;
  } /* endif */

  bResult = CPropertySheet::OnInitDialog();

  CenterWindow();

  return bResult;
}

// convert tag string of the tag table for display in the property sheet
BOOL CMUPropSheet::ConvertTagString( PSZ pszIn, PSZ pszOut )
{
  if ( *pszIn )
  {
    // insert escape characters
    TagInsEscapeChars( pszIn, pszOut );

    // remove any quotes
    if ( (*pszOut == QUOTE) && (*(pszOut + strlen(pszOut) - 1) == QUOTE) )
    {
      PSZ pszSource = pszOut + 1;
      PSZ pszTarget = pszOut;
      while ( *pszSource ) *pszTarget++ = *pszSource++;
      pszTarget--;
      *pszTarget = EOS;
    } /* endif */

    // convert to ANSI
    OEMTOANSI( pszOut );
  }
  else
  {
    *pszOut = EOS;
  } /* endif */
  return( TRUE );
}

// handle OK pushbutton
void CMUPropSheet::OnOKPB()
{
  // build in-memory tag table from property pages
  BOOL fOK = BuildTagTableFromProperties();

  // check tag table for new tag tables (name has not been tested in general property sheet)
  if ( fOK && m_fNewTagTable )
  {
    // test if name has been specified
    UtlStripBlanks ( m_generalPage.m_szTableName );
    if ( m_generalPage.m_szTableName[0] == EOS )
    {
      UtlError( ERROR_MUPROP_NO_TABLE_MAME, MB_CANCEL, 0, NULL, EQF_ERROR );
      SetActivePage( &m_generalPage );
      m_generalPage.SetDlgItemFocus( ID_MUPROP_GEN_NAME_EF );
      fOK = FALSE;
    } /* endif */

    // test if name is valid
    if ( fOK )
    {
      char szName[MAX_FILESPEC];
      fOK = UtlCheckIntName( m_generalPage.m_szTableName, szName );
      if ( fOK )
      {
        strcpy( m_generalPage.m_szTableName, szName );
        strcpy( m_szTagTableName, szName );
      }
      else
      {
        PSZ pszParm = m_generalPage.m_szTableName;
        UtlError( ERROR_MUPROP_NAME_INVALID, MB_CANCEL, 1, &pszParm, EQF_ERROR );
        SetActivePage( &m_generalPage );
        m_generalPage.SetDlgItemFocus( ID_MUPROP_GEN_NAME_EF );
      } /* endif */
    } /* endif */

    // test if there is a markup table with the given name
    if ( fOK )
    {
      if ( UtlCheckIfExist( m_generalPage.m_szTableName, MARKUP_OBJECT ) )
      {
        PSZ pszParm = m_generalPage.m_szTableName;
        UtlError( ERROR_MUPROP_NAME_EXISTS, MB_CANCEL, 1, &pszParm, EQF_ERROR );
        SetActivePage( &m_generalPage );
        m_generalPage.SetDlgItemFocus( ID_MUPROP_GEN_NAME_EF );
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  // write new tag table
  if ( fOK )
  {
    // set password and protect flag
    if ( m_fProtected )
    {
      m_pTagTable->fProtected = TRUE;
      m_pTagTable->ulPassword = m_ulPassword;
    }
    else
    {
      m_pTagTable->fProtected = FALSE;
      m_pTagTable->ulPassword = 0;
    } /* endif */

    // for new tables only: build tag table object name
    if ( m_fNewTagTable )
    {
      OtmMarkupPlugin *plugin = GetMarkupPluginObject( "UserMarkupTablePlugin" ) ;
      if ( plugin != NULL ) {
         strncpy( m_szTagTableFileName, plugin->getTablePath(), sizeof(m_szTagTableFileName) );
      } else {
         UtlMakeEQFPath( m_szTagTableFileName, NULC, TABLE_PATH, NULL );
      }
      strcat( m_szTagTableFileName, BACKSLASH_STR );
      strcat( m_szTagTableFileName, m_generalPage.m_szTableName );
      strcat( m_szTagTableFileName, EXT_OF_FORMAT );
      strcpy( m_szTagTableObjName, m_generalPage.m_szTableName );
    } /* endif */

    fOK = (UtlWriteFile( m_szTagTableFileName, m_pTagTable->uLength, m_pTagTable, TRUE ) == NO_ERROR );

    // if OK update control information for this markup table update.
    if ( fOK )
    {
       char szUserExitDLL[MAX_FILESPEC];
       strcpy( szUserExitDLL, m_generalPage.m_szUserExit );
       if ( szUserExitDLL[0] ) {
          strcat( szUserExitDLL, ".DLL" );
          strupr( szUserExitDLL );
       }
       if ( ! MUUpdateMarkupTableFiles( m_szTagTableName,
                         "UserMarkupTablePlugin", 
                         m_generalPage.m_szShortDescription,
                         m_generalPage.m_szLongDescription,
                         m_generalPage.m_szVersion,
                         m_szTagTableFileName,
                         szUserExitDLL, NULL ) ) {
          fOK = FALSE ;
       }
    }
  } /* endif */

  // broadcast new/changed tag table properties
  if ( fOK )
  {
     if ( m_fNewTagTable )  // a new table has been created
     {
       EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsTAGTABLE ),
                            MP2FROMP( m_szTagTableObjName ) );
     }
     else
     {
       EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                            MP1FROMSHORT( clsTAGIMP ),
//                            MP1FROMSHORT( PROP_CLASS_TAGTABLE  ),
                            MP2FROMP( m_szTagTableFileName ) );
     } /* endif */
  } /* endif */

  // leave dialog
  if ( fOK )
  {
    EndDialog( TRUE );
  } /* endif */
}

// handle cancel pushbutton
void CMUPropSheet::OnCancelPB()
{
  EndDialog( 0 );
}

// handle Help pushbutton
void CMUPropSheet::OnHelpPB()
{
  UtlInvokeHelp();
}


// handle protect pushbutton
void CMUPropSheet::OnProtectPB()
{
  if ( this->m_fProtected )
  {
    CMuPropUnprotectDlg dlgUnprotect;

    dlgUnprotect.m_ulPassword = m_ulPassword;

    switch ( dlgUnprotect.DoModal() )
    {
      case IDOK:
        // unprotect markup table
        m_generalPage.Protect( FALSE );
        m_tagsPage.Protect( FALSE );
        m_attrPage.Protect( FALSE );
        m_testPage.Protect( FALSE );
        m_fProtected = FALSE;
        m_ulPassword = 0;
        SetDlgItemText( ID_APPLY_NOW, "Protect" );
        break;
      case IDCANCEL:
        break;
      default:
        break;
    } /* endswitch */
  }
  else
  {
    CMuPropProtectDlg dlgProtect;

    switch ( dlgProtect.DoModal() )
    {
      case IDOK:
        // unprotect markup table
        m_generalPage.Protect( TRUE );
        m_tagsPage.Protect( TRUE );
        m_attrPage.Protect( TRUE );
        m_testPage.Protect( TRUE );
        m_fProtected = TRUE;
        m_ulPassword =  dlgProtect.m_ulPassword;
        SetDlgItemText( ID_APPLY_NOW, "Unprotect" );
        break;
      case IDCANCEL:
        break;
      default:
        break;
    } /* endswitch */
  } /* endif */
}

// handling for pushbuttons of test property page
// the test property page passes theese command messages to its parent

// handle load pushbutton of property page
void CMUPropSheet::OnTestLoadPB()
{
  CFileDialog OpenFileDialog( TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_LONGNAMES );

  if ( OpenFileDialog.DoModal() )
  {
    CString strFile = OpenFileDialog.GetPathName();
    strcpy( m_szDocName, (LPCTSTR)strFile );
    m_testPage.SetDocName( m_szDocName );
    OnTestRefreshPB();
  } /* endif */
}

void CMUPropSheet::OnTestAddTagPB()
{
  CString sSelection;
  if ( m_testPage.GetSelectedText( sSelection ) )
  {
    MUPRPTAG stTag;
    memset( &stTag, 0, sizeof(stTag) );
    strncpy( stTag.szName, (LPCTSTR)sSelection, sizeof(stTag.szName)-1 );
    stTag.szEndDelim[0] = EOS;
    stTag.Tagtype    = SELC_TAG;
    stTag.Segbreak   = SEG_NEUTRAL;
    stTag.Asstext    = NOEXPL_TEXT;
    stTag.fAttr      = FALSE;
    stTag.usLength   = (USHORT)(strlen(stTag.szName));
    stTag.usPosition = 0;
    stTag.fTranslate = FALSE;
    stTag.usAddInfo  = 0;
    stTag.ClassId    = 0;
    stTag.usFixedTagId = 0;
    m_tagsPage.AddTag( &stTag, TRUE );
  } /* endif */
}

void CMUPropSheet::OnTestAddAttrPB()
{
  CString sSelection;
  if ( m_testPage.GetSelectedText( sSelection ) )
  {
    MUPRPATTR stAttr;
    memset( &stAttr, 0, sizeof(stAttr) );
    strncpy( stAttr.szName, (LPCTSTR)sSelection, sizeof(stAttr.szName)-1 );
    stAttr.szEndDelim[0] = EOS;
    stAttr.usLength = (USHORT)(strlen(stAttr.szName));
    stAttr.fTranslate = FALSE;
    stAttr.usAddInfo  = 0;
    m_attrPage.AddAttr( &stAttr, TRUE );
  } /* endif */
}

void CMUPropSheet::OnTestRefreshPB()
{
  BOOL fOK = TRUE;

  // build new tag table and write it as test tag table
  fOK = BuildTagTableFromProperties();
  if ( !fOK )
  {
    // return asap...
    return;
  } /* endif */

  BOOL fUseUnicode = m_pTagTable->fUseUnicodeForSegFile;
  m_pTagTable->fUseUnicodeForSegFile = FALSE;
  if ( fOK ) UtlDelete( m_szTestTable, 0L, FALSE );
  if ( fOK ) UtlWriteFile( m_szTestTable, m_pTagTable->uLength, m_pTagTable, TRUE );
  m_pTagTable->fUseUnicodeForSegFile = fUseUnicode;

  // copy test document to test folder and create document property file
  if ( fOK )
  {
    fOK = CopyFile( this->m_szDocName, this->m_szDocSource, FALSE );

    // create property file
    if ( fOK )
    {
      UtlDelete( m_szDocProp, 0L, FALSE );
      ULONG ulErrorInfo;
      HPROP hprop = CreateProperties( this->m_szDocObj, NULL,
                                      PROP_CLASS_DOCUMENT, &ulErrorInfo );
      CloseProperties( hprop, PROP_FILE, &ulErrorInfo );
    } /* endif */
  } /* endif */

  // do text segmentation
  if ( fOK )
  {
    // delete any existing segmented source file
    UtlDelete( m_szDocSeg, 0L, FALSE );

    // build segmented file
    fOK = m_testPage.BuildSegFile( m_szDocObj, m_szDocSeg, m_pTagTable );
  } /* endif */

  // convert segmented file to rich text format
  if ( fOK )
  {
    if ( UtlFileExist( m_szDocSeg ) )
    {
      fOK = m_testPage.BuildRichTextFile( m_szDocSeg, m_szDocRTF, m_pTagTable );
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // show converted file in rich text control
  if ( fOK )
  {
    m_testPage.LoadRichTextFile( m_szDocRTF );
  }
  else
  {
    // show error message
    UtlError( ERROR_MUPROP_REFRESH_FAILED, MB_OK, 0, NULL, EQF_ERROR );

    // hide rich text control, show static
    CWnd *pTextGB = m_testPage.GetDlgItem( ID_MUPROP_TEST_TEXT_STATIC );
    pTextGB->ShowWindow( SW_SHOWNOACTIVATE );
    m_testPage.m_EditCtrl.ShowWindow( SW_HIDE );
  } /* endif */
}

// create an new in-memory tag table from the data of our property sheets
BOOL CMUPropSheet::BuildTagTableFromProperties()
{
  BOOL fOK = TRUE;
  PTAGIMPORT pTagImp = NULL;
  PTAGTABLE pNewTable = NULL;

  // check data in property sheets
  fOK = m_generalPage.UpdateData( TRUE );
  if ( fOK ) fOK = m_tagsPage.UpdateData( TRUE );
  if ( fOK ) fOK = m_attrPage.UpdateData( TRUE );

  // check  if tag table is empty and has no user exit
  if ( fOK )
  {
    UtlStripBlanks ( m_generalPage.m_szUserExit );

    if ( (m_generalPage.m_szUserExit[0] == EOS) && (m_tagsPage.GetTagCount() == 0) )
    {
      UtlError( ERROR_MUPROP_TABLE_IS_EMPTY, MB_CANCEL, 0, NULL, EQF_ERROR );
      SetActivePage( &m_tagsPage );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // create tag import data area
  if ( fOK ) fOK = TagImpAllocTagImport( &pTagImp );



  // fill-in values from general property page
  if ( fOK )
  {
      strncpy( pTagImp->szSegmentExit, m_generalPage.m_szUserExit, sizeof(pTagImp->szSegmentExit)-1 );

      if ( m_generalPage.m_szTrNoteStart[0] && m_generalPage.m_szTrNoteEnd[0] )
      {
        strcpy( pTagImp->TRNoteEntry.chTrnote1Text, m_generalPage.m_szTrNoteLevel1 );
        ANSITOOEM( pTagImp->TRNoteEntry.chTrnote1Text );
        strcpy( pTagImp->TRNoteEntry.chTrnote2Text, m_generalPage.m_szTrNoteLevel2 );
        ANSITOOEM( pTagImp->TRNoteEntry.chTrnote2Text );
        strcpy( pTagImp->TRNoteEntry.chStartText, m_generalPage.m_szTrNoteStart );
        ANSITOOEM( pTagImp->TRNoteEntry.chStartText );
        strcpy( pTagImp->TRNoteEntry.chEndText, m_generalPage.m_szTrNoteEnd );
        ANSITOOEM( pTagImp->TRNoteEntry.chEndText );
      } /* endif */

      pTagImp->fUseUnicodeForSegFile = m_generalPage.m_fUseUnicodeForSegFile;
      pTagImp->chSingleSubst         = m_generalPage.m_chSingleSubstChar;
      pTagImp->chMultSubst           = m_generalPage.m_chMultSubstChar;
      pTagImp->usCharacterSet        = m_generalPage.m_usCharSet;
      strcpy( pTagImp->szDescription,  m_generalPage.m_szDescription );
      ANSITOOEM( pTagImp->szDescription );
      strcpy( pTagImp->szDescriptiveName, m_generalPage.m_szDescrName );
      ANSITOOEM( pTagImp->szDescriptiveName );
      pTagImp->fReflow               = m_generalPage.m_fReflow;
  } /* endif */

  // add tags from tags property page
  if ( fOK )
  {
    MUPRPTAG TagData;
    TERMENTRY TagImpTag;

    int iTagCount = m_tagsPage.GetTagCount( );
    int iTag = 0;

    while ( fOK && (iTag < iTagCount) )
    {
      fOK = m_tagsPage.GetTag( iTag, &TagData );
      if ( fOK )
      {
        char szEndDel[MAX_TAGSIZE+10];     // buffer for end delimiter with quotes
        char szTag[MAX_TAGSIZE+10];        // buffer for tag string with quotes


        memset( &TagImpTag, 0, sizeof(TagImpTag) );
        TagImpTag.Asstext             = (ASSTEXT)TagData.Asstext;
        TagImpTag.BitFlags.AddInfo    = TagData.usAddInfo;
        TagImpTag.BitFlags.fTranslate = TagData.fTranslate;
        TagImpTag.fAttr               = TagData.fAttr;

        ANSITOOEM( TagData.szEndDelim );
        szEndDel[0] = QUOTE;
        int iEndDelLen = strlen(TagData.szEndDelim);
        memcpy( szEndDel + 1, TagData.szEndDelim, iEndDelLen );
        szEndDel[iEndDelLen+1] = QUOTE;
        szEndDel[iEndDelLen+2] = EOS;
        TagImpTag.pEndDelim           = szEndDel;

        ANSITOOEM( TagData.szName );
        szTag[0] = QUOTE;
        int iTagLen = strlen(TagData.szName);
        memcpy( szTag + 1, TagData.szName, iTagLen );
        szTag[iTagLen+1] = QUOTE;
        szTag[iTagLen+2] = EOS;
        TagImpTag.pText     = szTag;


        TagImpTag.Segbreak            = (SEGBREAK)TagData.Segbreak;
        TagImpTag.Tagtype             = (TAGTYPE)TagData.Tagtype;
        TagImpTag.usLength            = TagData.usLength;
        TagImpTag.usPosition          = TagData.usPosition;
        TagImpTag.TagClassId          = (TAGCLASSID)(TagData.ClassId);
        TagImpTag.BitFlags.fTRNote    = TagData.fTRNote;
        TagImpTag.usFixTokenId        = TagData.usFixedTagId;

        fOK = TagImpAddTag( pTagImp, &TagImpTag );

        iTag++;
      } /* endif */
    } /* endwhile */

  } /* endif */

  // add attributes from attribute property page
  if ( fOK )
  {
    MUPRPATTR AttrData;
    ATTRENTRY TagImpAttr;
    int iAttrCount = m_attrPage.GetAttrCount();
    int iAttr = 0;

    while ( fOK && (iAttr < iAttrCount) )
    {
      fOK = m_attrPage.GetAttr( iAttr, &AttrData );
      if ( fOK )
      {
        char szEndDel[MAX_TAGSIZE+10];     // buffer for end delimiter with quotes
        char szAttr[MAX_TAGSIZE+10];       // buffer for attribute string with quotes

        memset( &TagImpAttr, 0, sizeof(TagImpAttr) );
        TagImpAttr.BitFlags.AddInfo    = AttrData.usAddInfo;
        TagImpAttr.BitFlags.fTranslate = AttrData.fTranslate;

        ANSITOOEM( AttrData.szEndDelim );
        szEndDel[0] = QUOTE;
        int iEndDelLen = strlen(AttrData.szEndDelim);
        memcpy( szEndDel + 1, AttrData.szEndDelim, iEndDelLen );
        szEndDel[iEndDelLen+1] = QUOTE;
        szEndDel[iEndDelLen+2] = EOS;
        TagImpAttr.pEndDelim   = szEndDel;

        ANSITOOEM( AttrData.szName );
        szAttr[0] = QUOTE;
		int iAttrLen = strlen(AttrData.szName);
		memcpy( szAttr + 1, AttrData.szName, iAttrLen );
		szAttr[iAttrLen+1] = QUOTE;
		szAttr[iAttrLen+2] = EOS;
        TagImpAttr.pText     = szAttr;

        TagImpAttr.usLength            = AttrData.usLength;
        // the following two fields are defined in the atribute structure
        // but are not used in SGML format of tag table
        TagImpAttr.usFixTokenId        = 0;     // AttrData.usFixedAttrId;
        TagImpAttr.BitFlags.fTRNote    = FALSE; // AttrData.fTRNote;

        fOK = TagImpAddAttr( pTagImp, &TagImpAttr );
        iAttr++;
      } /* endif */
    } /* endwhile */
  } /* endif */

  // create in-memory tag table
  if ( fOK )
  {
    fOK = TagImpBuildtagTable( pTagImp, &pNewTable );
  } /* endif */

  // cleanup
  if ( pTagImp ) TagImpFreeTagImport( pTagImp );

  // if OK use new tag table as current one
  if ( fOK )
  {
    UtlAlloc( (PVOID *)&m_pTagTable, 0L, 0L, NOMSG );
    m_pTagTable = pNewTable;
  } /* endif */


  return( fOK );
}


/////////////////////////////////////////////////////////////////////////////
// CMUPropGeneralPage property page

CMUPropGeneralPage::CMUPropGeneralPage()
        : CMUPropertyPage(CMUPropGeneralPage::IDD)
{
  // initialize our member variables
  m_szTableName[0] = EOS;
  m_szDescription[0] = EOS;
  m_szDescrName[0] = EOS;
  m_szUserExit[0] = EOS;
  m_usCharSet = TAGCHARSET_ASCII;
  m_chSingleSubstChar = CHAR_SNGL_SUBST;
  m_chMultSubstChar = CHAR_MULT_SUBST;
  m_szTrNoteStart[0] = EOS;
  m_szTrNoteLevel1[0] = EOS;
  m_szTrNoteLevel2[0] = EOS;
  m_szTrNoteEnd[0] = EOS;
  m_fNewTagTable = FALSE;
  m_fModified = FALSE;
  m_fUseUnicodeForSegFile = FALSE;
  m_fPageInitialized = FALSE;
  m_fProtected = FALSE;
  m_fReflow = TAGREFLOW_NOTSPECIFIED;
  m_szLongDescription[0] = EOS;
  m_szShortDescription[0] = EOS;
  m_szPlugin[0] = EOS;
  m_szSupplier[0] = EOS;
  m_szVersion[0] = EOS;
}

BOOL CMUPropGeneralPage::UpdateData( BOOL bSaveAndValidate )
{
  BOOL fOK = TRUE;

  if ( bSaveAndValidate )
  {
    // get and validate property page data

    // set text of entry fields from the values of our member variables

    // no check for name field, will be done in save code
    if ( fOK )
    {
      GetDlgItemText( ID_MUPROP_GEN_NAME_EF, m_szTableName, sizeof(m_szTableName) /*- 1*/ );
      UtlStripBlanks( m_szTableName );
    } /* endif */

    // get and check descriptive name
    if ( fOK )
    {
      GetDlgItemText( ID_MUPROP_GEN_DESCRNAME_EF, m_szDescrName, sizeof(m_szDescrName) /*- 1*/ );
      UtlStripBlanks( m_szDescrName );
      if ( (m_szDescrName[0] != EOS) )
      {
        if ( stricmp( m_szDescrName, m_szTableName ) != 0 )
        {
          strcpy( m_szDescrName, m_szTableName ); /* Name must be same as markup table name */
//        PSZ pszParms[2];
//        pszParms[0] = m_szDescrName;
//        pszParms[1] = m_szTableName;
//        UtlError( ERROR_MUPROP_DESCRNAME_INUSE, MB_CANCEL, 2, pszParms, EQF_ERROR );
//        ActivatePage();
//        SetDlgItemFocus( ID_MUPROP_GEN_DESCRNAME_EF );
//        fOK = FALSE;
        } /* endif */
      } /* endif */
    } /* endif */

    // get and check description
    if ( fOK )
    {
      GetDlgItemText( ID_MUPROP_GEN_DESCR_EF, m_szDescription, sizeof(m_szDescription) );
    } /* endif */

    // get and check user exit
    if ( fOK )
    {
      GetDlgItemText( ID_MUPROP_GEN_EXIT_EF, m_szUserExit, sizeof(m_szUserExit) /*- 1*/ );

      // check if name is valid
      {
        PSZ pszTest = m_szUserExit;
        UtlStripBlanks( m_szUserExit );

        if ( strlen(pszTest) > 8 )
        {
          fOK = FALSE;
        } /* endif */

        while ( fOK && *pszTest )
        {
          if ( ((*pszTest >= 'a') && (*pszTest <= 'z')) ||
               ((*pszTest >= 'A') && (*pszTest <= 'Z')) ||
               ((*pszTest >= '0') && (*pszTest <= '9')) ||
               (*pszTest == '$') || (*pszTest == '_') ||
               (*pszTest == '-')                       )
          {
            pszTest++;                 // test next characters
          }
          else
          {
            fOK = FALSE;               // invalid characters in name
          } /* endif */
        } /* endwhile */

        if ( !fOK )
        {
          PSZ pszParm;
          pszParm = m_szUserExit;
          UtlError( ERROR_MUPROP_USEREXIT_INVALID, MB_CANCEL, 1, &pszParm, EQF_ERROR );
          ActivatePage();
          SetDlgItemFocus( ID_MUPROP_GEN_EXIT_EF );
        } /* endif */
      }

      // check if user exit can be loaded
      if ( fOK && (m_szUserExit[0] != EOS) )
      {
        CHAR  szExit[MAX_LONGFILESPEC];        // buffer for library name
        HINSTANCE hModule;

        OtmMarkupPlugin *plugin = GetMarkupPlugin( m_szTableName );
        if ( plugin != NULL )
        {
          strcpy( szExit, plugin->getPluginDirectory() ) ;
          strcat( szExit, "\\" );
          strcat( szExit, plugin->getUserExitDirectory() ) ;
          strcat( szExit, "\\" );
          strcat( szExit, m_szUserExit );
        } else {
           strcpy( szExit, m_szUserExit );
        }
        strcat( szExit, EXT_OF_DLL );

        hModule = LoadLibrary( szExit );
        if( hModule )
        {
          FreeLibrary( hModule );
        }
        else
        {
          PSZ pszParm;
          pszParm = m_szUserExit;
          UtlError( ERROR_MUPROP_USEREXIT_LOADERROR, MB_CANCEL, 1, &pszParm, EQF_ERROR );
          ActivatePage();
          SetDlgItemFocus( ID_MUPROP_GEN_EXIT_EF );
          fOK = FALSE;
        } /* endif */
      } /* endif */
    } /* endif */

    // get and check characters for single and multiple substitution
    if ( fOK )
    {
      char szCharString[2];
      GetDlgItemText( ID_MUPROP_GEN_SINGLESUBSTCHAR_EF, szCharString, 2 );
      m_chSingleSubstChar = szCharString[0];
      if ( (m_chSingleSubstChar == EOS) || (m_chSingleSubstChar == BLANK) )
      {
        // use default if nothing specified
        m_chSingleSubstChar = CHAR_SNGL_SUBST;
        szCharString[0] = m_chSingleSubstChar;
        SetDlgItemText( ID_MUPROP_GEN_SINGLESUBSTCHAR_EF, szCharString );
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      char szCharString[2];
      GetDlgItemText( ID_MUPROP_GEN_MULTSUBSTCHAR_EF, szCharString, 2 );
      m_chMultSubstChar = szCharString[0];
      if ( (m_chMultSubstChar == EOS) || (m_chMultSubstChar == BLANK) )
      {
        // use default if nothing specified
        m_chMultSubstChar = CHAR_MULT_SUBST;
        szCharString[0] = m_chMultSubstChar;
        SetDlgItemText( ID_MUPROP_GEN_MULTSUBSTCHAR_EF, szCharString );
      } /* endif */

      if ( m_chMultSubstChar == m_chSingleSubstChar )
      {
        UtlError( ERROR_MUPROP_SAMESUBSTCHAR, MB_CANCEL, 0, NULL, EQF_ERROR );
        ActivatePage();
        SetDlgItemFocus( ID_MUPROP_GEN_MULTSUBSTCHAR_EF );
        fOK = FALSE;
      } /* endif */
    } /* endif */

    // get and check translator note strings
    if ( fOK )
    {
      GetDlgItemText( ID_MUPROP_GEN_TRNOTESTART_EF, m_szTrNoteStart, sizeof(m_szTrNoteStart) /*- 1*/ );
      GetDlgItemText( ID_MUPROP_GEN_TRNOTELEV1_EF, m_szTrNoteLevel1, sizeof(m_szTrNoteLevel1) /*- 1*/ );
      GetDlgItemText( ID_MUPROP_GEN_TRNOTELEV2_EF, m_szTrNoteLevel2, sizeof(m_szTrNoteLevel2) /*- 1*/ );
      GetDlgItemText( ID_MUPROP_GEN_TRNOTEEND_EF, m_szTrNoteEnd, sizeof(m_szTrNoteEnd) /*- 1*/ );
    } /* endif */

    // get the active characterset
    if ( fOK )
    {
      int iValue = GetSelectedValue( ID_MUPROP_GEN_CHARSET_CB  );
      if ( iValue == LB_ERR )
      {
        // use default
        m_usCharSet = TAGCHARSET_ASCII;
        SearchSelectValue( ID_MUPROP_GEN_CHARSET_CB, m_usCharSet );
      }
      else
      {
        m_usCharSet = (USHORT)iValue;
      } /* endif */
    } /* endif */
    // get the active Reflow specification
	if ( fOK )
	{
	  int iValue = GetSelectedValue( ID_MUPROP_GEN_REFLOW_CB  );
	  m_fReflow = (USHORT)iValue;
    } /* endif */

	if ( fOK )
	{
      GetDlgItemText( ID_MUPROP_GEN_LONGDESCR_EF, m_szLongDescription, sizeof(m_szLongDescription) /*- 1*/ );
      GetDlgItemText( ID_MUPROP_GEN_SHORTDESCR_EF, m_szShortDescription, sizeof(m_szShortDescription) /*- 1*/ );
      GetDlgItemText( ID_MUPROP_GEN_PLUGIN_EF, m_szPlugin, sizeof(m_szPlugin) /*- 1*/ );
      GetDlgItemText( ID_MUPROP_GEN_SUPPLIER_EF, m_szSupplier, sizeof(m_szSupplier) /*- 1*/ );
      GetDlgItemText( ID_MUPROP_GEN_VERSION_EF, m_szVersion, sizeof(m_szVersion) /*- 1*/ );
    }
  }
  else
  {
    // show values of member variables in property page

    // set text of entry fields from the values of our member variables
    SetEditTextAndTextLimit( ID_MUPROP_GEN_NAME_EF, m_szTableName, sizeof(m_szTableName) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_DESCRNAME_EF, m_szDescrName, sizeof(m_szDescrName) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_DESCR_EF, m_szDescription, sizeof(m_szDescription) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_EXIT_EF, m_szUserExit, sizeof(m_szUserExit) - 1 );
    char szCharString[2];
    szCharString[0] = m_chSingleSubstChar;
    szCharString[1] = EOS;
    SetEditTextAndTextLimit( ID_MUPROP_GEN_SINGLESUBSTCHAR_EF, szCharString, 1 );
    szCharString[0] = m_chMultSubstChar;
    szCharString[1] = EOS;
    SetEditTextAndTextLimit( ID_MUPROP_GEN_MULTSUBSTCHAR_EF, szCharString, 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_TRNOTESTART_EF, m_szTrNoteStart, sizeof(m_szTrNoteStart) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_TRNOTELEV1_EF, m_szTrNoteLevel1, sizeof(m_szTrNoteLevel1) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_TRNOTELEV2_EF, m_szTrNoteLevel2, sizeof(m_szTrNoteLevel2) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_TRNOTEEND_EF, m_szTrNoteEnd, sizeof(m_szTrNoteEnd) - 1 );

    // select the active characterset
    SearchSelectValue( ID_MUPROP_GEN_CHARSET_CB, m_usCharSet );
    SearchSelectValue( ID_MUPROP_GEN_REFLOW_CB, (USHORT)m_fReflow );

    SetEditTextAndTextLimit( ID_MUPROP_GEN_LONGDESCR_EF, m_szLongDescription, sizeof(m_szLongDescription) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_SHORTDESCR_EF, m_szShortDescription, sizeof(m_szShortDescription) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_PLUGIN_EF, m_szPlugin, sizeof(m_szPlugin) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_SUPPLIER_EF, m_szSupplier, sizeof(m_szSupplier) - 1 );
    SetEditTextAndTextLimit( ID_MUPROP_GEN_VERSION_EF, m_szVersion, sizeof(m_szVersion) - 1 );
  }
  return( fOK );
}


BEGIN_MESSAGE_MAP(CMUPropGeneralPage, CMUPropertyPage)
        //{{AFX_MSG_MAP(CMUPropGeneralPage)
                // NOTE: the ClassWizard will add message map macros here
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CMUPropGeneralPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  // fill character set combobox
  FillWithStyles( ID_MUPROP_GEN_CHARSET_CB, aiCharsetStyles );
  FillWithStyles( ID_MUPROP_GEN_REFLOW_CB, aiReflowStyles );

  // fill dialog controls
  UpdateData( FALSE );
  CWnd *pTextGB = GetDlgItem( ID_MUPROP_GEN_DESCR_EF );
  pTextGB->ShowWindow( SW_HIDE );
  pTextGB = GetDlgItem( ID_MUPROP_GEN_DESCRNAME_EF );
  pTextGB->ShowWindow( SW_HIDE );

  // protect name field for existing tag tables
  if ( !m_fNewTagTable )
  {
    EnableDlgItem( ID_MUPROP_GEN_NAME_EF, FALSE );
    EnableDlgItem( ID_MUPROP_GEN_DESCRNAME_EF, FALSE );
    EnableDlgItem( ID_MUPROP_GEN_PLUGIN_EF, FALSE );
    EnableDlgItem( ID_MUPROP_GEN_SUPPLIER_EF, FALSE );
    SetDlgItemFocus( ID_MUPROP_GEN_DESCR_EF );
  } /* endif */

  m_fPageInitialized = TRUE;

  if ( m_fProtected ) Protect( TRUE );

  return TRUE;

}

// protect/unprotect controls of property page
void CMUPropGeneralPage::Protect( BOOL fProtect )
{
  BOOL fEnable = !fProtect;

  m_fProtected = fProtect;

  if ( m_fPageInitialized )
  {
    EnableDlgItem( ID_MUPROP_GEN_NAME_EF, fEnable && m_fNewTagTable );
    EnableDlgItem( ID_MUPROP_GEN_DESCRNAME_EF, fEnable && m_fNewTagTable );
    EnableDlgItem( ID_MUPROP_GEN_DESCR_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_EXIT_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_SINGLESUBSTCHAR_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_MULTSUBSTCHAR_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_TRNOTESTART_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_TRNOTELEV1_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_TRNOTELEV2_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_TRNOTEEND_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_CHARSET_CB, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_REFLOW_CB, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_LONGDESCR_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_SHORTDESCR_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_PLUGIN_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_SUPPLIER_EF, fEnable );
    EnableDlgItem( ID_MUPROP_GEN_VERSION_EF, fEnable );
  } /* endif */
}


/////////////////////////////////////////////////////////////////////////////
// CMUPropTagsPage property page
/////////////////////////////////////////////////////////////////////////////

CMUPropTagsPage::CMUPropTagsPage()
        : CMUPropertyPage(CMUPropTagsPage::IDD)
{
  m_pSelectedTag = NULL;
  m_iSelectedTagIndex = LB_ERR;
  m_fUpdateByDialog = FALSE;
  m_fModified = FALSE;
  m_fPageInitialized = FALSE;
  m_fProtected = FALSE;

}


BEGIN_MESSAGE_MAP(CMUPropTagsPage, CMUPropertyPage)
        //{{AFX_MSG_MAP(CMUPropTagsPage)
  ON_CBN_EDITCHANGE( ID_MUPROP_TAG_DELIM_CB, OnEditchangeTagDelimCb)
  ON_LBN_SELCHANGE( ID_MUPROP_TAG_TAGS_LB, OnSelChangeTagsLb)
  ON_EN_CHANGE( ID_MUPROP_TAG_NAME_EF, OnEditchangeTagNameEf)
  ON_EN_CHANGE( ID_MUPROP_TAG_ID_EF, OnEditchangeIdEf )
  ON_CBN_SELCHANGE( ID_MUPROP_TAG_SEGFLAGS_CB, OnSelChangeSegFlagsCb )
  ON_CBN_SELCHANGE( ID_MUPROP_TAG_TEXTSTYLE_CB, OnSelChangeTextStyleCb )
  ON_CBN_SELCHANGE( ID_MUPROP_TAG_TYPE_CB, OnSelChangeTagTypeCb )
  ON_BN_CLICKED( ID_MUPROP_TAG_WITHATTR_CHK, OnButtonClickedWithAttrChk )
  ON_EN_CHANGE( ID_MUPROP_TAG_DELIM_EF, OnEditchangeDelimEf )
  ON_EN_CHANGE( ID_MUPROP_TAG_TAGLEN_EF, OnEditchangeLenEf )
  ON_BN_CLICKED( ID_MUPROP_TAG_TRANSL_CHK, OnButtonClickedTranslChk )
  ON_BN_CLICKED( ID_MUPROP_TAG_TRNOTE_CHK, OnButtonClickedTrNoteChk )
  ON_EN_CHANGE( ID_MUPROP_TAG_ADDINFO_EF, OnEditchangeAddInfoEf )
  ON_EN_CHANGE( ID_MUPROP_TAG_COLPOS_EF, OnEditchangeColPosEf )
  ON_CBN_SELCHANGE( ID_MUPROP_TAG_CLASSID_CB, OnSelChangeClassCb )
  ON_BN_CLICKED( ID_MUPROP_TAG_ADD_PB, OnButtonClickedAddPB )
  ON_BN_CLICKED( ID_MUPROP_TAG_DELETE_PB, OnButtonClickedDeletePB )

  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// destructor
CMUPropTagsPage::~CMUPropTagsPage()
{
  // free memory used by tags in our tag list
  int i = 0;
  int iMax = m_TagList.GetUpperBound();
  while ( i <= iMax )
  {
    PMUPRPTAG pTag = (PMUPRPTAG)m_TagList.GetAt( i );
    delete( pTag );
    i++;
  } /* endwhile */
}


void CMUPropTagsPage::OnEditchangeTagDelimCb()
{
  // TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen


}

void CMUPropTagsPage::OnEditchangeTagNameEf()
{
  // handle change in tag name entry field

  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    // get new tag name and update tag info structure
    GetDlgItemText( ID_MUPROP_TAG_NAME_EF, m_pSelectedTag->szName, sizeof(m_pSelectedTag->szName)/*-1*/ );

    // update tag in tag listbox
    CListBox *pListBox = (CListBox *)GetDlgItem( ID_MUPROP_TAG_TAGS_LB );

    PMUPRPTAG pTag = (PMUPRPTAG)pListBox->GetItemData( m_iSelectedTagIndex );
    pListBox->DeleteString( m_iSelectedTagIndex );
    m_iSelectedTagIndex = pListBox->AddString( m_pSelectedTag->szName );
    pListBox->SetItemData( m_iSelectedTagIndex, (DWORD)pTag );
    m_fUpdateByDialog = TRUE;
    pListBox->SetCurSel( m_iSelectedTagIndex );
    m_fUpdateByDialog = FALSE;
  } /* endif */
}

void CMUPropTagsPage::OnSelChangeTagsLb()
{
  // get currently selected item and fill tag property controls
  if ( !m_fUpdateByDialog )
  {
    CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_TAG_TAGS_LB );
    int iSelected = pList->GetCurSel();
    if ( iSelected >= 0 )
    {
      PMUPRPTAG pTag = (PMUPRPTAG)pList->GetItemData( iSelected );
      if ( pTag )
      {
        m_pSelectedTag = pTag;
        m_iSelectedTagIndex = iSelected;

        m_fUpdateByDialog = TRUE;
        SetDlgItemText( ID_MUPROP_TAG_NAME_EF, pTag->szName );
        SetDlgItemText( ID_MUPROP_TAG_DELIM_EF, pTag->szEndDelim );
        CheckDlgButton( ID_MUPROP_TAG_WITHATTR_CHK, pTag->fAttr );
        SearchSelectValue( ID_MUPROP_TAG_TEXTSTYLE_CB, pTag->Asstext );
        CheckDlgButton( ID_MUPROP_TAG_TRANSL_CHK, pTag->fTranslate );
        CheckDlgButton( ID_MUPROP_TAG_TRNOTE_CHK, pTag->fTRNote );
        SearchSelectValue( ID_MUPROP_TAG_SEGFLAGS_CB, pTag->Segbreak );
        SearchSelectValue( ID_MUPROP_TAG_TYPE_CB, pTag->Tagtype );

        if ( pTag->usFixedTagId )
        {
          SetDlgItemInt( ID_MUPROP_TAG_ID_EF, pTag->usFixedTagId, FALSE );
        }
        else
        {
          SetDlgItemText( ID_MUPROP_TAG_ID_EF, "" );
        } /* endif */

        if ( pTag->ClassId != CLS_DEFAULT )
        {
          SearchSelectValue( ID_MUPROP_TAG_CLASSID_CB, pTag->ClassId );
        }
        else
        {
          ClearComboSel( ID_MUPROP_TAG_CLASSID_CB );
        } /* endif */

        if ( pTag->usAddInfo )
        {
          SetDlgItemInt( ID_MUPROP_TAG_ADDINFO_EF, pTag->usAddInfo, FALSE );
        }
        else
        {
          SetDlgItemText( ID_MUPROP_TAG_ADDINFO_EF, "" );
        } /* endif */

        if ( pTag->usLength )
        {
          SetDlgItemInt( ID_MUPROP_TAG_TAGLEN_EF, pTag->usLength, FALSE );
        }
        else
        {
          SetDlgItemText( ID_MUPROP_TAG_TAGLEN_EF, "" );
        } /* endif */

        if ( pTag->usPosition )
        {
          SetDlgItemInt( ID_MUPROP_TAG_COLPOS_EF, pTag->usPosition, FALSE );
        }
        else
        {
          SetDlgItemText( ID_MUPROP_TAG_COLPOS_EF, "" );
        } /* endif */
        m_fUpdateByDialog = FALSE;
      }
      else
      {
        m_pSelectedTag = NULL;
        m_iSelectedTagIndex = LB_ERR;
        ClearTagProperties();
      } /* end if pTag */
    }
    else
    {
      m_pSelectedTag = NULL;
      m_iSelectedTagIndex = LB_ERR;
      ClearTagProperties();
    } /* endif */
  } /* endif */
}

void CMUPropTagsPage::OnEditchangeIdEf()
{
  // handle change of unique ID
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    BOOL fOK = TRUE;
    m_pSelectedTag->usFixedTagId = (USHORT)GetDlgItemInt( ID_MUPROP_TAG_ID_EF, &fOK, FALSE );
    //GQTODO: handling for !fOK
  } /* endif */
}

void CMUPropTagsPage::OnSelChangeSegFlagsCb()
{
  // handle change of segmentation flags
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    CComboBox *pCombo = (CComboBox *)GetDlgItem( ID_MUPROP_TAG_SEGFLAGS_CB );
    if ( pCombo )
    {
      int iSelected = pCombo->GetCurSel();
      if ( iSelected >= 0 )
      {
        m_pSelectedTag->Segbreak = (USHORT)pCombo->GetItemData( iSelected );
      } /* endif */
    } /* endif */
  } /* endif */
}


void CMUPropTagsPage::OnSelChangeTextStyleCb()
{
  // handle change of associated text style
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    CComboBox *pCombo = (CComboBox *)GetDlgItem( ID_MUPROP_TAG_TEXTSTYLE_CB );
    if ( pCombo )
    {
      int iSelected = pCombo->GetCurSel();
      if ( iSelected >= 0 )
      {
        m_pSelectedTag->Asstext = (USHORT)pCombo->GetItemData( iSelected );
      } /* endif */
    } /* endif */
  } /* endif */
}


void CMUPropTagsPage::OnSelChangeTagTypeCb()
{
  // handle change of tag type
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    CComboBox *pCombo = (CComboBox *)GetDlgItem( ID_MUPROP_TAG_TYPE_CB );
    if ( pCombo )
    {
      int iSelected = pCombo->GetCurSel();
      if ( iSelected >= 0 )
      {
        m_pSelectedTag->Tagtype = (USHORT)pCombo->GetItemData( iSelected );
      } /* endif */
    } /* endif */
  } /* endif */
}


void CMUPropTagsPage::OnButtonClickedWithAttrChk()
{
  // handle change of with attributes flag
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    m_pSelectedTag->fAttr = (EQF_BOOL)(IsDlgButtonChecked( ID_MUPROP_TAG_WITHATTR_CHK ));
  } /* endif */
}


void CMUPropTagsPage::OnEditchangeDelimEf()
{
  // handle change of end delimiters
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    GetDlgItemText( ID_MUPROP_TAG_DELIM_EF, m_pSelectedTag->szEndDelim,
                    sizeof(m_pSelectedTag->szEndDelim)/*-1*/ );
  } /* endif */
}


void CMUPropTagsPage::OnEditchangeLenEf()
{
  // handle change in tag length
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    BOOL fOK = TRUE;
    m_pSelectedTag->usLength = (USHORT)GetDlgItemInt( ID_MUPROP_TAG_TAGLEN_EF, &fOK, FALSE );
    //GQTODO: handling for !fOK
  } /* endif */
}


void CMUPropTagsPage::OnButtonClickedTranslChk()
{
  // handle change of translatable text flag
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    m_pSelectedTag->fTranslate= (EQF_BOOL)(IsDlgButtonChecked( ID_MUPROP_TAG_TRANSL_CHK ));
  } /* endif */
}


void CMUPropTagsPage::OnButtonClickedTrNoteChk()
{
  // handle change of translator notes flag
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    m_pSelectedTag->fTRNote = (EQF_BOOL)(IsDlgButtonChecked( ID_MUPROP_TAG_TRNOTE_CHK ));
  } /* endif */
}


void CMUPropTagsPage::OnEditchangeAddInfoEf()
{
  // handle change of additional info
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    BOOL fOK = TRUE;
    m_pSelectedTag->usAddInfo = (USHORT)GetDlgItemInt( ID_MUPROP_TAG_ADDINFO_EF, &fOK, FALSE );
    //GQTODO: handling for !fOK
  } /* endif */
}


void CMUPropTagsPage::OnEditchangeColPosEf()
{
  // handle change of column position
  if ( !m_fUpdateByDialog && this->m_pSelectedTag )
  {
    BOOL fOK = TRUE;
    m_pSelectedTag->usPosition = (USHORT)GetDlgItemInt( ID_MUPROP_TAG_COLPOS_EF, &fOK, FALSE );
  } /* endif */
}


void CMUPropTagsPage::OnSelChangeClassCb()
{
  // handle change of class ID
  if ( !m_fUpdateByDialog && m_pSelectedTag )
  {
    CComboBox *pCombo = (CComboBox *)GetDlgItem( ID_MUPROP_TAG_CLASSID_CB );
    if ( pCombo )
    {
      int iSelected = pCombo->GetCurSel();
      if ( iSelected >= 0 )
      {
        m_pSelectedTag->ClassId = (USHORT)pCombo->GetItemData( iSelected );
      } /* endif */
    } /* endif */
  } /* endif */
}

void CMUPropTagsPage::OnButtonClickedAddPB()
{
  // make new and empty tag data
  PMUPRPTAG pNewTag = new( MUPRPTAG );
  ASSERT( pNewTag != NULL );
  memset( pNewTag, 0, sizeof(MUPRPTAG) );

  // add new tag to our tag list
  this->m_TagList.Add( (void *)pNewTag );

  // add new entry to tag listbox and make it the selected one
  CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_TAG_TAGS_LB );
  if ( pList )
  {
    m_fUpdateByDialog = TRUE;
    int iIndex = pList->AddString( pNewTag->szName );
    if ( iIndex >= 0 )
    {
      pList->SetItemData( iIndex, (DWORD)pNewTag );
      pList->SetCurSel( iIndex );
      m_iSelectedTagIndex = iIndex;
      m_pSelectedTag = pNewTag;
    }
    m_fUpdateByDialog = FALSE;
  } /* endif */

  // clear all tag properties
  ClearTagProperties();

  // position to tag name entry field
  SetDlgItemFocus( ID_MUPROP_TAG_NAME_EF );


}

void CMUPropTagsPage::OnButtonClickedDeletePB()
{
  CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_TAG_TAGS_LB );
  int iSelected = pList->GetCurSel();
  if ( iSelected >= 0 )
  {
    PMUPRPTAG pDelTag = (PMUPRPTAG)pList->GetItemData( iSelected );

    // look for ptr in our ptr array and remove it if found
    int i = 0;
    int iMax = m_TagList.GetUpperBound();
    while ( i <= iMax )
    {
      PMUPRPTAG pTag = (PMUPRPTAG)m_TagList.GetAt( i );
      if ( pTag == pDelTag )
      {
        m_TagList.RemoveAt( i );
        break;
      } /* endif */
      i++;
    } /* endwhile */

    // remove tag from listbox
    pList->DeleteString( iSelected );

    // select next or previous item
    if ( iSelected >= pList->GetCount() )
    {
      iSelected--;
    } /* endif */

    if ( iSelected >= 0)
    {
      pList->SetCurSel( iSelected );
      OnSelChangeTagsLb();
    }
    else
    {
     ClearTagProperties();
    } /* endif */
  } /* endif */
}


void CMUPropTagsPage::ClearTagProperties()
{
  m_fUpdateByDialog = TRUE;
  SetDlgItemText( ID_MUPROP_TAG_NAME_EF, "" );
  SetDlgItemText( ID_MUPROP_TAG_DELIM_EF, "" );
  SetDlgItemText( ID_MUPROP_TAG_ID_EF, "" );
  CheckDlgButton( ID_MUPROP_TAG_WITHATTR_CHK, FALSE );
  ClearComboSel( ID_MUPROP_TAG_TEXTSTYLE_CB );
  ClearComboSel( ID_MUPROP_TAG_CLASSID_CB );
  CheckDlgButton( ID_MUPROP_TAG_TRANSL_CHK, FALSE );
  CheckDlgButton( ID_MUPROP_TAG_TRNOTE_CHK, FALSE );
  ClearComboSel( ID_MUPROP_TAG_SEGFLAGS_CB );
  ClearComboSel( ID_MUPROP_TAG_TYPE_CB );
  SetDlgItemText( ID_MUPROP_TAG_ADDINFO_EF, "" );
  SetDlgItemText( ID_MUPROP_TAG_TAGLEN_EF, "" );
  SetDlgItemText( ID_MUPROP_TAG_COLPOS_EF, "" );
  m_fUpdateByDialog = FALSE;
}


BOOL CMUPropTagsPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  // fill style comboboxes
  FillWithStyles( ID_MUPROP_TAG_SEGFLAGS_CB, aiSegBreakStyles );
  FillWithStyles( ID_MUPROP_TAG_TEXTSTYLE_CB, aiAssTextStyles );
  FillWithStyles( ID_MUPROP_TAG_TYPE_CB, aiTagTypes );
  FillWithStyles( ID_MUPROP_TAG_CLASSID_CB, aiClassStyles );

  UpdateData( FALSE );
  m_fPageInitialized = TRUE;
  if ( m_fProtected ) Protect( TRUE );

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

// protect/unprotect controls of property page
void CMUPropTagsPage::Protect( BOOL fProtect )
{
  BOOL fEnable = !fProtect;

  m_fProtected = fProtect;

  if ( m_fPageInitialized )
  {
    EnableDlgItem( ID_MUPROP_TAG_DELIM_CB, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_NAME_EF, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_ID_EF, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_SEGFLAGS_CB, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_TEXTSTYLE_CB, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_TYPE_CB, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_WITHATTR_CHK, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_DELIM_EF, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_TAGLEN_EF, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_TRANSL_CHK, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_TRNOTE_CHK, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_ADDINFO_EF, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_COLPOS_EF, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_CLASSID_CB, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_ADD_PB, fEnable );
    EnableDlgItem( ID_MUPROP_TAG_DELETE_PB, fEnable );
  } /* endif */
}

// Copy data to/from dialog controls
BOOL CMUPropTagsPage::UpdateData( BOOL bSaveAndValidate )
{
  BOOL fOK = TRUE;

  if ( bSaveAndValidate )
  {
    // tags are already in tag list, we have to check them now
    int i = 0;
    int iMax = m_TagList.GetUpperBound();
    while ( fOK && (i <= iMax) )
    {
      PMUPRPTAG pTag = (PMUPRPTAG)m_TagList.GetAt( i );

      // check if name has been specified
      if ( pTag->szName[0] == EOS )
      {
        ActivatePage();
        int iItem = this->GetIndexForValue( ID_MUPROP_TAG_TAGS_LB, (DWORD)pTag );
        if ( iItem != LB_ERR )
        {
          CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_TAG_TAGS_LB );
          pList->SetCurSel( iItem );
          OnSelChangeTagsLb();
        } /* endif */
        SetDlgItemFocus( ID_MUPROP_TAG_NAME_EF );
        UtlError( ERROR_MUPROP_NO_TAG_MAME, MB_CANCEL, 0, NULL, EQF_ERROR );
        fOK = FALSE;
      } /* endif */

      // check if end delimiter or length has been specified
      if ( fOK )
      {
        if ( (pTag->szEndDelim[0] == EOS) && (pTag->usLength == 0) )
        {
          ActivatePage();
          int iItem = this->GetIndexForValue( ID_MUPROP_TAG_TAGS_LB, (DWORD)pTag );
          if ( iItem != LB_ERR )
          {
            CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_TAG_TAGS_LB );
            pList->SetCurSel( iItem );
            OnSelChangeTagsLb();
          } /* endif */
          SetDlgItemFocus( ID_MUPROP_TAG_DELIM_EF );
          UtlError( ERROR_MUPROP_NO_TAG_DELIM, MB_CANCEL, 0, NULL, EQF_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      // next tag
      i++;
    } /* endwhile */
  }
  else
  {
    // show values of member variables in property page

    // insert all tags of the tag list into the tag combobox
    CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_TAG_TAGS_LB );
    if ( pList )
    {
      int i = 0;
      int iMax = m_TagList.GetUpperBound();
      while ( i <= iMax )
      {
        PMUPRPTAG pTag = (PMUPRPTAG)m_TagList.GetAt( i );
        int iIndex = pList->AddString( pTag->szName );
        if ( iIndex >= 0 )
        {
          pList->SetItemData( iIndex, (DWORD)pTag );
        }
        i++;
      } /* endwhile */

      // select first tag and trigger refresh of tag properties
      pList->SetCurSel( 0 );
      OnSelChangeTagsLb();
    } /* end if ( pList ) */

  }
  return( fOK );
}

// Add the given tag to our tag list
BOOL CMUPropTagsPage::AddTag( PMUPRPTAG pTagData, BOOL fAddToListbox )
{
  // make copy of tag data
  PMUPRPTAG pNewTag = new( MUPRPTAG );
  ASSERT( pNewTag != NULL );
  memcpy( pNewTag, pTagData, sizeof(MUPRPTAG) );

  // add new tag to our tag list
  this->m_TagList.Add( (void *)pNewTag );

  // add new tag to listbox if requested
  if ( fAddToListbox && m_fPageInitialized )
  {
    CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_TAG_TAGS_LB );
    if ( pList )
    {
      int iIndex = pList->AddString( pNewTag->szName );
      if ( iIndex >= 0 )
      {
        pList->SetItemData( iIndex, (DWORD)pNewTag );
      } /* endif */
    } /* endif */
  } /* endif */

  return( TRUE );
}

// return number of tags in our tag list
int CMUPropTagsPage::GetTagCount()
{
  return( m_TagList.GetSize() );
}

// return tag data for a tag from our tag list
BOOL CMUPropTagsPage::GetTag( int iTagIndex, PMUPRPTAG pTagData )
{
  BOOL fOK = (iTagIndex >= 0) && (iTagIndex <= m_TagList.GetUpperBound() );

  if ( fOK )
  {
    PMUPRPTAG pTag = (PMUPRPTAG)m_TagList.GetAt( iTagIndex );
    if ( pTag )
    {
      memcpy( pTagData, pTag, sizeof(MUPRPTAG) );
    }
    else
    {
      fOK = FALSE;
    }
  }

  return( fOK );
}




/////////////////////////////////////////////////////////////////////////////
// CMUPropAttrPage property page

BEGIN_MESSAGE_MAP(CMUPropAttrPage, CMUPropertyPage)
        //{{AFX_MSG_MAP(CMUPropAttrPage)
        ON_EN_CHANGE( ID_MUPROP_ATTR_NAME_EF, OnEditchangeAttrNameEf)
        ON_EN_CHANGE( ID_MUPROP_ATTR_LENGTH_EF, OnEditchangeLengthEf)
        ON_LBN_SELCHANGE( ID_MUPROP_ATTR_ATTR_LB, OnSelChangeAttrLb)
        ON_EN_CHANGE( ID_MUPROP_ATTR_DELIM_EF, OnEditchangeDelimEf )
        ON_BN_CLICKED( ID_MUPROP_ATTR_TRANSL_CHK, OnButtonClickedTranslChk )
        ON_BN_CLICKED( ID_MUPROP_ATTR_ADD_PB, OnButtonClickedAddPB )
        ON_BN_CLICKED( ID_MUPROP_ATTR_DELETE_PB, OnButtonClickedDeletePB )
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


CMUPropAttrPage::CMUPropAttrPage()
        : CMUPropertyPage(CMUPropAttrPage::IDD)
{
  // initialize our member variables
  m_pSelectedAttr = NULL;
  m_iSelectedAttrIndex = LB_ERR;
  m_fUpdateByDialog = FALSE;
  m_fModified = FALSE;
  m_fPageInitialized = FALSE;
  m_fProtected = FALSE;
}

CMUPropAttrPage::~CMUPropAttrPage()
{
  // free memory used by attributes in our attributes list
  int i = 0;
  int iMax = m_AttrList.GetUpperBound();
  while ( i <= iMax )
  {
    PMUPRPATTR pAttr = (PMUPRPATTR)m_AttrList.GetAt( i );
    delete( pAttr );
    i++;
  } /* endwhile */
}


BOOL CMUPropAttrPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  UpdateData( FALSE );
  m_fPageInitialized = TRUE;
  if ( m_fProtected ) Protect( TRUE );

  return TRUE;

}

// protect/unprotect controls of property page
void CMUPropAttrPage::Protect( BOOL fProtect )
{
  BOOL fEnable = !fProtect;

  m_fProtected = fProtect;

  if ( m_fPageInitialized )
  {
    EnableDlgItem( ID_MUPROP_ATTR_NAME_EF, fEnable );
    EnableDlgItem( ID_MUPROP_ATTR_LENGTH_EF, fEnable );
    EnableDlgItem( ID_MUPROP_ATTR_DELIM_EF, fEnable );
    EnableDlgItem( ID_MUPROP_ATTR_TRANSL_CHK, fEnable );
    EnableDlgItem( ID_MUPROP_ATTR_ADD_PB, fEnable );
    EnableDlgItem( ID_MUPROP_ATTR_DELETE_PB, fEnable );
  } /* endif */
}

BOOL CMUPropAttrPage::AddAttr( PMUPRPATTR pAttrData, BOOL fAddToListbox  )
{
  // make copy of attribute data
  PMUPRPATTR pNewAttr = new( MUPRPATTR );
  ASSERT( pNewAttr != NULL );
  memcpy( pNewAttr, pAttrData, sizeof(MUPRPATTR) );

  // add new attribute to our attribute list
  m_AttrList.Add( (void *)pNewAttr );

  // add attribute to listbox if requested
  if ( fAddToListbox && m_fPageInitialized )
  {
    CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_ATTR_ATTR_LB );
    if ( pList )
    {
      int iIndex = pList->AddString( pNewAttr->szName );
      if ( iIndex >= 0 )
      {
        pList->SetItemData( iIndex, (DWORD)pNewAttr );
      } /* endif */
    } /* endif */
  } /* endif */

  return( TRUE );
}

int CMUPropAttrPage::GetAttrCount()
{
  return( m_AttrList.GetSize() );
}

BOOL CMUPropAttrPage::GetAttr( int iAttrIndex, PMUPRPATTR pAttrData )
{
  BOOL fOK = (iAttrIndex >= 0) && (iAttrIndex <= m_AttrList.GetUpperBound() );

  if ( fOK )
  {
    PMUPRPATTR pAttr = (PMUPRPATTR)m_AttrList.GetAt( iAttrIndex );
    if ( pAttr )
    {
      memcpy( pAttrData, pAttr, sizeof(MUPRPATTR) );
    }
    else
    {
      fOK = FALSE;
    }
  }

  return( fOK );
}

void CMUPropAttrPage::ClearAttrProperties()
{
  m_fUpdateByDialog = TRUE;
  SetDlgItemText( ID_MUPROP_ATTR_NAME_EF, "" );
  SetDlgItemText( ID_MUPROP_ATTR_DELIM_EF, "" );
  SetDlgItemText( ID_MUPROP_ATTR_LENGTH_EF, "" );
  CheckDlgButton( ID_MUPROP_ATTR_TRANSL_CHK, FALSE );
  SetDlgItemText( ID_MUPROP_ATTR_ADDINFO_EF, "" );
  m_fUpdateByDialog = FALSE;
}


BOOL CMUPropAttrPage::UpdateData( BOOL bSaveAndValidate )
{
  BOOL fOK = TRUE;

  if ( bSaveAndValidate )
  {
    // attributes are already in attribute list, we have to check them now
    int i = 0;
    int iMax = m_AttrList.GetUpperBound();
    while ( fOK && (i <= iMax) )
    {
      PMUPRPATTR pAttr = (PMUPRPATTR)m_AttrList.GetAt( i );

      // check if name has been specified
      if ( pAttr->szName[0] == EOS )
      {
        ActivatePage();
        int iItem = this->GetIndexForValue( ID_MUPROP_ATTR_ATTR_LB, (DWORD)pAttr );
        if ( iItem != LB_ERR )
        {
          CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_ATTR_ATTR_LB );
          pList->SetCurSel( iItem );
          OnSelChangeAttrLb();
        } /* endif */
        SetDlgItemFocus( ID_MUPROP_ATTR_NAME_EF );
        UtlError( ERROR_MUPROP_NO_ATTR_MAME, MB_CANCEL, 0, NULL, EQF_ERROR );
        fOK = FALSE;
      } /* endif */

      // check if end delimiter or length has been specified
      if ( fOK )
      {
        if ( (pAttr->szEndDelim[0] == EOS) && (pAttr->usLength == 0) )
        {
          ActivatePage();
          int iItem = this->GetIndexForValue( ID_MUPROP_ATTR_ATTR_LB, (DWORD)pAttr );
          if ( iItem != LB_ERR )
          {
            CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_ATTR_ATTR_LB );
            pList->SetCurSel( iItem );
            OnSelChangeAttrLb();
          } /* endif */
          SetDlgItemFocus( ID_MUPROP_ATTR_DELIM_EF );
          UtlError( ERROR_MUPROP_NO_ATTR_DELIM, MB_CANCEL, 0, NULL, EQF_ERROR );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      // next attribute
      i++;
    } /* endwhile */
  }
  else
  {
    // show values of member variables in property page

    // insert all tags of the tag list into the tag combobox
    CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_ATTR_ATTR_LB );
    if ( pList )
    {
      int i = 0;
      int iMax = m_AttrList.GetUpperBound();
      while ( i <= iMax )
      {
        PMUPRPATTR pAttr = (PMUPRPATTR)m_AttrList.GetAt( i );
        int iIndex = pList->AddString( pAttr->szName );
        if ( iIndex >= 0 )
        {
          pList->SetItemData( iIndex, (DWORD)pAttr );
        }
        i++;
      } /* endwhile */

      // select first attribute and trigger refresh of attribute properties
      pList->SetCurSel( 0 );
      OnSelChangeAttrLb();
    } /* end if ( pList ) */
  }
  return( fOK );
}

void CMUPropAttrPage::OnSelChangeAttrLb()
{
  // get currently selected item and fill attribute property controls
  if ( !m_fUpdateByDialog )
  {
    CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_ATTR_ATTR_LB );
    int iSelected = pList->GetCurSel();
    if ( iSelected >= 0 )
    {
      PMUPRPATTR pAttr = (PMUPRPATTR)pList->GetItemData( iSelected );
      if ( pAttr )
      {
        m_pSelectedAttr = pAttr;
        m_iSelectedAttrIndex = iSelected;

        m_fUpdateByDialog = TRUE;
        SetDlgItemText( ID_MUPROP_ATTR_NAME_EF, pAttr->szName );
        SetDlgItemText( ID_MUPROP_ATTR_DELIM_EF, pAttr->szEndDelim );
        CheckDlgButton( ID_MUPROP_ATTR_TRANSL_CHK, pAttr->fTranslate );

        if ( pAttr->usLength )
        {
          SetDlgItemInt( ID_MUPROP_ATTR_LENGTH_EF, pAttr->usLength, FALSE );
        }
        else
        {
          SetDlgItemText( ID_MUPROP_ATTR_LENGTH_EF, "" );
        } /* endif */

        if ( pAttr->usAddInfo )
        {
          SetDlgItemInt( ID_MUPROP_ATTR_ADDINFO_EF, pAttr->usAddInfo, FALSE );
        }
        else
        {
          SetDlgItemText( ID_MUPROP_ATTR_ADDINFO_EF, "" );
        } /* endif */


        m_fUpdateByDialog = FALSE;
      }
      else
      {
        m_pSelectedAttr = NULL;
        m_iSelectedAttrIndex = LB_ERR;
        ClearAttrProperties();
      } /* end if pTag */
    }
    else
    {
      m_pSelectedAttr = NULL;
      m_iSelectedAttrIndex = LB_ERR;
      ClearAttrProperties();
    } /* endif */
  } /* endif */
}

void CMUPropAttrPage::OnEditchangeAttrNameEf()
{
  // handle change in tag name entry field

  if ( !m_fUpdateByDialog && this->m_pSelectedAttr )
  {
    // get new tag name and update tag info structure
    GetDlgItemText( ID_MUPROP_ATTR_NAME_EF, m_pSelectedAttr->szName, sizeof(m_pSelectedAttr->szName)/*-1*/ );

    // update tag in tag listbox
    CListBox *pListBox = (CListBox *)GetDlgItem( ID_MUPROP_ATTR_ATTR_LB );

    PMUPRPATTR pAttr = (PMUPRPATTR)pListBox->GetItemData( m_iSelectedAttrIndex );
    pListBox->DeleteString( m_iSelectedAttrIndex );
    m_iSelectedAttrIndex = pListBox->AddString( m_pSelectedAttr->szName );
    pListBox->SetItemData( m_iSelectedAttrIndex, (DWORD)pAttr );
    m_fUpdateByDialog = TRUE;
    pListBox->SetCurSel( m_iSelectedAttrIndex );
    m_fUpdateByDialog = FALSE;
  } /* endif */
}

void CMUPropAttrPage::OnEditchangeLengthEf()
{
  // handle change of length
  if ( !m_fUpdateByDialog && this->m_pSelectedAttr )
  {
    BOOL fOK = TRUE;
    m_pSelectedAttr->usLength = (USHORT)GetDlgItemInt( ID_MUPROP_ATTR_LENGTH_EF, &fOK, FALSE );
    //GQTODO: handling for !fOK
  } /* endif */
}

void CMUPropAttrPage::OnEditchangeAddInfoEf()
{
  // handle change of length
  if ( !m_fUpdateByDialog && this->m_pSelectedAttr )
  {
    BOOL fOK = TRUE;
    m_pSelectedAttr->usAddInfo = (USHORT)GetDlgItemInt( ID_MUPROP_ATTR_ADDINFO_EF, &fOK, FALSE );
    //GQTODO: handling for !fOK
  } /* endif */
}

void CMUPropAttrPage::OnEditchangeDelimEf()
{
  // handle change of end delimiters
  if ( !m_fUpdateByDialog && this->m_pSelectedAttr )
  {
    GetDlgItemText( ID_MUPROP_ATTR_DELIM_EF, m_pSelectedAttr->szEndDelim,
                    sizeof(m_pSelectedAttr->szEndDelim)/*-1*/ );
  } /* endif */
}

void CMUPropAttrPage::OnButtonClickedTranslChk()
{
  // handle change of translatable text flag
  if ( !m_fUpdateByDialog && this->m_pSelectedAttr )
  {
    m_pSelectedAttr->fTranslate= (EQF_BOOL)(IsDlgButtonChecked( ID_MUPROP_ATTR_TRANSL_CHK ));
  } /* endif */
}

void CMUPropAttrPage::OnButtonClickedAddPB()
{
  // make new and empty tag data
  PMUPRPATTR pNewAttr = new( MUPRPATTR );
  ASSERT( pNewAttr != NULL );
  memset( pNewAttr, 0, sizeof(MUPRPATTR) );

  // add new tag to our tag list
  this->m_AttrList.Add( (void *)pNewAttr );

  // add new entry to tag listbox and make it the selected one
  CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_ATTR_ATTR_LB );
  if ( pList )
  {
    m_fUpdateByDialog = TRUE;
    int iIndex = pList->AddString( pNewAttr->szName );
    if ( iIndex >= 0 )
    {
      pList->SetItemData( iIndex, (DWORD)pNewAttr );
      pList->SetCurSel( iIndex );
      m_iSelectedAttrIndex = iIndex;
      m_pSelectedAttr = pNewAttr;
    }
    m_fUpdateByDialog = FALSE;
  } /* endif */

  // clear all tag properties
  ClearAttrProperties();

  // position to tag name entry field
  SetDlgItemFocus( ID_MUPROP_ATTR_NAME_EF );
}

void CMUPropAttrPage::OnButtonClickedDeletePB()
{
  CListBox *pList = (CListBox *)GetDlgItem( ID_MUPROP_ATTR_ATTR_LB );
  int iSelected = pList->GetCurSel();
  if ( iSelected >= 0 )
  {
    PMUPRPATTR pDelAttr = (PMUPRPATTR)pList->GetItemData( iSelected );

    // look for ptr in our ptr array and remove it if found
    int i = 0;
    int iMax = m_AttrList.GetUpperBound();
    while ( i <= iMax )
    {
      PMUPRPATTR pAttr = (PMUPRPATTR)m_AttrList.GetAt( i );
      if ( pAttr == pDelAttr )
      {
        m_AttrList.RemoveAt( i );
        break;
      } /* endif */
      i++;
    } /* endwhile */

    // remove Attribute from listbox
    pList->DeleteString( iSelected );

    // select next or previous item
    if ( iSelected >= pList->GetCount() )
    {
      iSelected--;
    } /* endif */

    if ( iSelected >= 0)
    {
      pList->SetCurSel( iSelected );
      OnSelChangeAttrLb();
    }
    else
    {
     ClearAttrProperties();
    } /* endif */
  } /* endif */
}


/////////////////////////////////////////////////////////////////////////////
// CMUPropTestPage property page

BEGIN_MESSAGE_MAP(CMUPropTestPage, CMUPropertyPage)
        //{{AFX_MSG_MAP(CMUPropTestPage)
        ON_BN_CLICKED( ID_MUPROP_TEST_LOAD_PB, OnButtonClickedLoadPB )
        ON_BN_CLICKED( ID_MUPROP_TEST_ADDATTR_PB, OnButtonClickedAddAttrPB )
        ON_BN_CLICKED( ID_MUPROP_TEST_ADDTAG_PB, OnButtonClickedAddTagPB )
        ON_CONTROL( EN_SELCHANGE, ID_MUPROP_TEST_TEXT_RICHEDIT, OnSelChangeRichEdit )
        ON_BN_CLICKED( ID_MUPROP_TEST_REFRESH_PB, OnButtonClickedRefreshPB )
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


CMUPropTestPage::CMUPropTestPage()
        : CMUPropertyPage(CMUPropTestPage::IDD)
{
  // initialize our member variables
  m_fPageInitialized = FALSE;
  m_fProtected = FALSE;
}

CMUPropTestPage::~CMUPropTestPage()
{
  // free memory used by property page
}


BOOL CMUPropTestPage::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  // create the rich edit control inside the test text frame
  CWnd *pTextGB = GetDlgItem( ID_MUPROP_TEST_TEXT_STATIC );
  if ( pTextGB )
  {
    RECT rect, rectDlg;
    pTextGB->GetWindowRect( &rect );
    GetWindowRect( &rectDlg );

    // make position relative to dialog window
    rect.top    -= rectDlg.top;
    rect.bottom -= rectDlg.top;
    rect.right  -= rectDlg.left;
    rect.left   -= rectDlg.left;

    m_EditCtrl.Create( WS_CHILD | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE,
                       rect,
                       pTextGB->GetParent(), ID_MUPROP_TEST_TEXT_RICHEDIT );
    m_EditCtrl.SetEventMask( ENM_SELCHANGE );
    m_EditCtrl.SetReadOnly( TRUE );
  } /* endif */

  // create the rich edit control for the legend text
  pTextGB = GetDlgItem( ID_MUPROP_TEST_LEGEND_STATIC );
  if ( pTextGB )
  {
    RECT rect, rectDlg;
    pTextGB->GetWindowRect( &rect );
    GetWindowRect( &rectDlg );

    // make position relative to dialog window
    rect.top    -= rectDlg.top;
    rect.bottom -= rectDlg.top;
    rect.right  -= rectDlg.left;
    rect.left   -= rectDlg.left;

    m_EditCtrlLegend.Create( WS_CHILD | WS_VISIBLE,
                       rect,
                       pTextGB->GetParent(), ID_MUPROP_TEST_TEXT_RICHEDIT );
    m_EditCtrlLegend.SetEventMask( ENM_SELCHANGE );
    m_EditCtrlLegend.SetReadOnly( TRUE );
    LoadRichTextFile( NULL );  // show legend text
    pTextGB->ShowWindow( SW_HIDE );
  } /* endif */
  m_fPageInitialized = TRUE;

  if ( m_fProtected ) Protect( TRUE );

  return TRUE;

}


// segmentate a source file using analysis function TASegmentFile
BOOL CMUPropTestPage::BuildSegFile( PSZ pszDocObj, PSZ pszSegFile, PTAGTABLE pTable )
{
  BOOL fOK = TRUE;
  PEQF_BOOL pfKill;

  pszSegFile; pTable;
  fOK = TAAnalyzeFile( pszDocObj, HWND_FUNCIF, 0L, "Standard", &pfKill );

  return( fOK );
}

// convert the segmented file into a rich text file
BOOL CMUPropTestPage::BuildRichTextFile( PSZ pszSegFile, PSZ pszRichTextFile, PTAGTABLE pTable )
{
  BOOL         fOK = TRUE;
  PTBDOCUMENT  pDoc = NULL;            // ptr to document structure for segmented file
  HMODULE      hModule = NULLHANDLE;   // address of user exit module handle
  PFN          pfnUserExit = NULL;     // address of ptr to user exit function
  PSZ          pTokBuf = NULL;         // token buffer
  LONG         lTokBufSize = 16000L;   // current size of token buffer
  PBUFCB       pBufCB = NULL;          // handle for buffered output
  PSZ          pOutBuf = NULL;         // buffer for converted segment text
  PSZ          pszStartSeq  = "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\fnil\\fcharset0 Times New Roman;}{\\f1\\froman\\fprq2\\fcharset0 Times New Roman;}}{\\colortbl ;\\red255\\green0\\blue0;\\red255\\green0\\blue255;}\r\n";
  PSZ          pszUnprotect = "\\cf0\\f0 ";
  PSZ          pszProtect   = "\\cf1\\f1";
  PSZ          pszEndSeq    = "\\par \r\n}";
  PSZ          pszSegBoundary = "\\cf1\\f1<P>\\cf0\\f0 ";

  // allocate TBDOC structure
  if ( fOK ) fOK = UtlAlloc( (PVOID *)&pDoc, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE );

  // allocate token buffer
  if ( fOK ) fOK = UtlAlloc( (PVOID *)&pTokBuf, 0L, lTokBufSize, ERROR_STORAGE );

  // allocate segment output buffer
  if ( fOK ) fOK = UtlAlloc( (PVOID *)&pOutBuf, 0L, (MAX_SEGMENT_SIZE * 4), ERROR_STORAGE );

  // load QFTAGS tag table
  if ( fOK )
  {
    SHORT sRC = TALoadTagTable( DEFAULT_QFTAG_TABLE, (PLOADEDTABLE *)&pDoc->pQFTagTable,
                                TRUE, TRUE );
    fOK = (sRC == NO_ERROR);
  } /* endif */

  // build and prepare document markup table
  if ( fOK )
  {
    // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to System Preferences
    pDoc->ulOemCodePage = GetLangCodePage(OEM_CP, NULL);
    pDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, NULL);

    fOK = UtlAlloc( (PVOID *)&pDoc->pDocTagTable, 0L, sizeof(LOADEDTABLE), ERROR_STORAGE );
    if ( fOK )
    {
      PLOADEDTABLE pDocTable = (PLOADEDTABLE)pDoc->pDocTagTable;
      pDocTable->pTagTable = pTable;
      TATagsToUnicode( pDocTable, pTable->uLength, pDoc->ulOemCodePage );
    } /* endif */
    if ( fOK )
    {
      SHORT  sRC = TAPrepProtectTable( pDoc->pDocTagTable, &hModule,
                                        &pfnUserExit, NULL, NULL, NULL );
      fOK = (sRC == NO_ERROR);
    } /* endif */
  } /* endif */

  // load the segmented file
  if ( fOK )
  {
    SHORT  sRc;

    pDoc->docType = SSOURCE_DOC; // disable hist log processing

    sRc = EQFBFileRead( pszSegFile, pDoc );
    if ( sRc != NO_ERROR )
    {
//      usMBRC = UtlErrorHwnd( ERROR_FOLFIND_DOCLOAD, MB_OKCANCEL,
//                             1, &pszErrParm, EQF_QUERY, pIda->hwnd );
      fOK = FALSE;
    } /* endif */
  } /* endif */


  // open rich text file for output
  if ( fOK )
  {
    fOK = (UtlBufOpen( &pBufCB, pszRichTextFile, 4096, FILE_CREATE, TRUE ) == NO_ERROR );
    if ( fOK )
      fOK = (UtlBufWrite( pBufCB, pszStartSeq, strlen(pszStartSeq), TRUE ) == NO_ERROR );
  } /* endif */

  // loop over all segments
  BOOL fDone = FALSE;
  ULONG ulSegNum = 1;
  while ( fOK && !fDone )
  {
    PTBSEGMENT pSeg = EQFBGetSeg( pDoc, ulSegNum );

    if ( pSeg == NULL)
    {
      fDone = TRUE;                    // End of document reached
    }
    else if ( pSeg->pData == NULL )
    {
      fDone = TRUE;                    // End of document reached
    }
    else if ( pSeg->SegFlags.Joined && !pSeg->SegFlags.JoinStart )
    {
      // Segment has been chained to a previous segment so ignore it
      if ( ulSegNum < pDoc->ulMaxSeg - 1 )
      {
        ulSegNum++;
      }
      else
      {
        fDone = TRUE;               // no more segments to follow
      } /* endif */
    }
    else if ( pSeg->qStatus == QF_NOP )
    {
      // add segment to richt text file as protected data
      fOK = ( UtlBufWrite( pBufCB, pszProtect, strlen(pszProtect), TRUE ) == NO_ERROR );
      PreprocessTextData( pSeg->pData, pOutBuf );
      if ( fOK )
        fOK = (UtlBufWrite( pBufCB, pOutBuf, strlen(pOutBuf), TRUE ) == NO_ERROR );

      if ( ulSegNum < pDoc->ulMaxSeg - 1 )
      {
        ulSegNum++;
      }
      else
      {
        fDone = TRUE;               // no more segments to follow
      } /* endif */
    }
    else                            // normal data segments
    {
      // check for tags within segment data
      {
        PBYTE  pStartStop = NULL;
        USHORT usRC = 0;
        int          iIterations = 0;

        // create protect table
        do
        {
          usRC = TACreateProtectTable( pSeg->pData, pDoc->pDocTagTable, 0,
                                       (PTOKENENTRY)pTokBuf, (USHORT)lTokBufSize,
                                       (PSTARTSTOP *)&pStartStop, pfnUserExit,
                                       pDoc->ulOemCodePage );
          if ( usRC == EQFRS_AREA_TOO_SMALL )
          {
            // enlarge token buffer
            LONG lNewSize = lTokBufSize + (100 * sizeof(TOKENENTRY));

            fOK = UtlAlloc((PVOID *)&pTokBuf, lTokBufSize, lNewSize, NOMSG);
            if ( fOK )
            {
              lTokBufSize = lNewSize;
              iIterations++;
            } /* endif */
          } /* endif */
        } while ( fOK && (iIterations < 14) && (usRC == EQFRS_AREA_TOO_SMALL));
        fOK = (usRC == NO_ERROR);

        // handle tags using start/stop table
        if ( fOK  )
        {
          PSTARTSTOP pstCurrent = (PSTARTSTOP) pStartStop;

          while ( (pstCurrent->usType != 0) && fOK )
          {
            if ( pstCurrent->usType == UNPROTECTED_CHAR )
            {
              // insert normal text RTF tags
              fOK = UtlBufWrite( pBufCB, pszUnprotect, strlen(pszUnprotect), TRUE ) == NO_ERROR;
            }
            else
            {
              // insert protect text RTF tags
              fOK = UtlBufWrite( pBufCB, pszProtect, strlen(pszProtect), TRUE ) == NO_ERROR;
            } /* endif */

            if ( fOK )
            {
              PreprocessTextData( pSeg->pData + pstCurrent->usStart, pOutBuf,
                                  pstCurrent->usStop - pstCurrent->usStart + 1 );
              fOK = UtlBufWrite( pBufCB, pOutBuf, strlen(pOutBuf), TRUE ) == NO_ERROR;
            } /* endif */
            pstCurrent++;
          } /* endwhile */
        } /* endif */

        // free protect table
        if ( pStartStop ) UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
      } /* endif */

      // add segment boundary indicator
      if ( fOK )
      {
        fOK = UtlBufWrite( pBufCB, pszSegBoundary, strlen(pszSegBoundary), TRUE ) == NO_ERROR;
      } /* endif */
      //check if next segment available
      if ( ulSegNum < pDoc->ulMaxSeg - 1 )
      {
        ulSegNum++;
      }
      else
      {
        fDone = TRUE;               // no more segments to follow
      } /* endif */
    } /* endif */
  } /*endwhile*/

  // close files and cleanup
  if ( pBufCB )
  {
     UtlBufWrite( pBufCB, pszEndSeq, strlen(pszEndSeq), TRUE );
     UtlBufClose( pBufCB, TRUE );
  } /* endif */
  if ( pTokBuf ) UtlAlloc((PVOID *)&pTokBuf, 0L, 0L, NOMSG );
  if ( pOutBuf ) UtlAlloc( (PVOID *)&pOutBuf, 0L, 0L, NOMSG );
  if ( hModule ) TAEndProtectTable( &hModule, &pfnUserExit, NULL, NULL,
                                    NULL, NULL, NULL );
  if ( pDoc )
  {
    if ( pDoc->pQFTagTable )  TAFreeTagTable( (PLOADEDTABLE)pDoc->pQFTagTable );
    if ( pDoc->pDocTagTable )
    {
      PLOADEDTABLE pLoadedTable = (PLOADEDTABLE)pDoc->pDocTagTable;
      pLoadedTable->pTagTable = NULL;  // avoid free of tag table
      TAFreeTableSpace( pLoadedTable );
      UtlAlloc( (PVOID *)&pLoadedTable, 0L, 0L, NOMSG );
    } /* endif */
    TAFreeDoc( (PVOID *)&pDoc );
  } /* endif */

  return( fOK );
}

// convert the given data for output to richt text control
void CMUPropTestPage::PreprocessTextData( PSZ pszIn, PSZ pszOut )
{
  PreprocessTextData( pszIn, pszOut, strlen(pszIn) );
}

void CMUPropTestPage::PreprocessTextData( PSZ pszIn, PSZ pszOut, int iLen )
{
  // duplicate backslash and add \\par for CRLF
  while ( iLen )
  {
    if ( *pszIn == BACKSLASH )
    {
      *pszOut++ = *pszIn++;
      *pszOut++ = BACKSLASH;
      iLen--;
    }
    else if ( *pszIn == LF )
    {
      *pszOut++ = *pszIn++;
      iLen--;
      strcpy( pszOut, "\\par ");
      pszOut += strlen(pszOut);
    }
    else
    {
      *pszOut++ = *pszIn++;
      iLen--;
    } /* endif */
  } /* endwhile */
  *pszOut = EOS;
}

// load a rich text file into the rich text control or show legend text
DWORD CMUPropTestPage::LoadRichTextFile( PSZ pszRichTextFile )
{
  MUPROP_FILELOAD_INFO FileInfo;     // info for read callback function

  FileInfo.CurrentState = MUPROP_FILELOAD_INFO::OPEN;
  FileInfo.hFileIn = NULLHANDLE;
  FileInfo.pszInFile = pszRichTextFile;

  EDITSTREAM Es;                     // edit stream data
  Es.dwCookie = (DWORD)&FileInfo;
  Es.dwError = 0;
  Es.pfnCallback = MuPropLoadTextFromFile;

  if ( pszRichTextFile )
  {
    m_EditCtrl.StreamIn( SF_RTF, Es );

    // set visible state of control
    CWnd *pTextGB = GetDlgItem( ID_MUPROP_TEST_TEXT_STATIC );
    pTextGB->ShowWindow( SW_HIDE );
    m_EditCtrl.ShowWindow( SW_SHOWNOACTIVATE );
  }
  else
  {
    m_EditCtrlLegend.StreamIn( SF_RTF, Es );
  } /* endif */

  return Es.dwError;
}



DWORD CALLBACK MuPropLoadTextFromFile
(
  DWORD dwCookie,                      // application-defined value
  LPBYTE pbBuff,                       // pointer to a buffer
  LONG cb,                             // number of bytes to read or write
  LONG *pcb                            // pointer to number of bytes transferred
)
{
  PMUPROP_FILELOAD_INFO pInfo;
  DWORD dwReturn = 0;

  pInfo = (PMUPROP_FILELOAD_INFO)dwCookie;

  switch( pInfo->CurrentState )
  {
    case MUPROP_FILELOAD_INFO::OPEN:
      // open file, continue with read
      if ( pInfo->pszInFile )
      {
        pInfo->hFileIn = CreateFile( pInfo->pszInFile, GENERIC_READ, FILE_SHARE_READ,
                                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                     NULL );

        if ( pInfo->hFileIn == INVALID_HANDLE_VALUE )
        {
          dwReturn = GetLastError();
        }
        else
        {
          // read first block of file
          pInfo->CurrentState = MUPROP_FILELOAD_INFO::READ;
          dwReturn = MuPropLoadTextFromFile( dwCookie, pbBuff, cb, pcb );
        } /* endif */
      }
      else
      {
        // copy legend text to buffer
        strcpy( (PSZ)pbBuff, pszLegend );
        *pcb = strlen( (PSZ)pbBuff );
        pInfo->CurrentState = MUPROP_FILELOAD_INFO::DONE;
      } /* endif */
      break;

    case MUPROP_FILELOAD_INFO::READ:
      {
        DWORD dwRead = 0;
        if ( ReadFile( pInfo->hFileIn, pbBuff, cb, &dwRead, NULL ) )
        {
          *pcb = dwRead;
          if ( dwRead < (DWORD)cb )
          {
            // assume end of file
            CloseHandle( pInfo->hFileIn );
            pInfo->CurrentState = MUPROP_FILELOAD_INFO::DONE;
          } /* endif */
        }
        else
        {
          dwReturn = GetLastError();
        } /* endif */
      }
      break;

    case MUPROP_FILELOAD_INFO::DONE:
      // processing is finished
      *pcb = 0;
      break;

    default:
      // undefined state
      dwReturn = ERROR_INVALID_HANDLE;
      break;
  } /* endswitch */

  return( dwReturn );
}

// set name of test document
void CMUPropTestPage::SetDocName( PSZ pszDocName )
{
  SetDlgItemText( ID_MUPROP_TEST_TEXT_GB, pszDocName );
}

// protect/unprotect controls of property page
void CMUPropTestPage::Protect( BOOL fProtect )
{
  BOOL fEnable = !fProtect;

  m_fProtected = fProtect;

  if ( m_fPageInitialized )
  {
    EnableDlgItem( ID_MUPROP_TEST_ADDTAG_PB, fEnable );
    EnableDlgItem( ID_MUPROP_TEST_ADDATTR_PB, fEnable );
  } /* endif */
}

BOOL CMUPropTestPage::UpdateData( BOOL bSaveAndValidate )
{
  BOOL fSuccess = TRUE;

  if ( bSaveAndValidate )
  {
    // get and validate property page data
  }
  else
  {
    // show values of member variables in property page
  }
  return( fSuccess );
}

void CMUPropTestPage::OnButtonClickedLoadPB()
{
  // message will be handled in property sheet, pass message to parent window
  ::PostMessage( (GetParent())->m_hWnd, WM_COMMAND,
                 MAKEWPARAM( ID_MUPROP_TEST_LOAD_PB, BN_CLICKED ), 0L );
}

void CMUPropTestPage::OnButtonClickedAddTagPB()
{
  // message will be handled in property sheet, pass message to parent window
  ::PostMessage( (GetParent())->m_hWnd, WM_COMMAND,
                 MAKEWPARAM( ID_MUPROP_TEST_ADDTAG_PB, BN_CLICKED ), 0L );
}

void CMUPropTestPage::OnButtonClickedAddAttrPB()
{
  // message will be handled in property sheet, pass message to parent window
  ::PostMessage( (GetParent())->m_hWnd, WM_COMMAND,
                 MAKEWPARAM( ID_MUPROP_TEST_ADDATTR_PB, BN_CLICKED ), 0L );
}

void CMUPropTestPage::OnButtonClickedRefreshPB()
{
  // message will be handled in property sheet, pass message to parent window
  ::PostMessage( (GetParent())->m_hWnd, WM_COMMAND,
                 MAKEWPARAM( ID_MUPROP_TEST_REFRESH_PB, BN_CLICKED ), 0L );
}

void CMUPropTestPage::OnSelChangeRichEdit()
{
  CString s =  m_EditCtrl.GetSelText();
  BOOL fTextSelected = !s.IsEmpty();
  EnableDlgItem( ID_MUPROP_TEST_ADDTAG_PB, fTextSelected );
  EnableDlgItem( ID_MUPROP_TEST_ADDATTR_PB, fTextSelected );
}

// get currently selected text
BOOL CMUPropTestPage::GetSelectedText( CString &s )
{
  s =  m_EditCtrl.GetSelText();
  return( !s.IsEmpty() );
}

//////////////////////////////////////////////////////////////////////////
// CMUPropertyPage: standard property page class with some helper functions
//                  for handling of dialog controls
//////////////////////////////////////////////////////////////////////////

// constructor, constructs property page using base class
CMUPropertyPage::CMUPropertyPage( UINT nIDTemplate, UINT nIDCaption  )
{
  CPropertyPage::Construct( nIDTemplate, nIDCaption );
}


// set the text and the text limit of an edit control
void CMUPropertyPage::SetEditTextAndTextLimit( int iControlID, PSZ pszText, int iLimit )
{
  CEdit *pEditCtrl = (CEdit *)GetDlgItem( iControlID );
  if ( pEditCtrl )
  {
    pEditCtrl->SetWindowText( pszText );
    pEditCtrl->SetLimitText( iLimit );
  }
}

// enable or diable a dialog control
void CMUPropertyPage::EnableDlgItem( int iControlId, BOOL fEnableState )
{
  CWnd *pWnd = GetDlgItem( iControlId );
  if ( *pWnd )
  {
    pWnd->EnableWindow( fEnableState );
  } /* endif */
}

// search the given value in the lParam values of the combobox and select the item
BOOL CMUPropertyPage::SearchSelectValue( int iID, USHORT usValue )
{
  BOOL fSelected = FALSE;

  CComboBox *pCombo = (CComboBox *)GetDlgItem( iID );
  if ( pCombo )
  {
    int i = 0;
    int iMax = pCombo->GetCount();
    while ( (i < iMax) && !fSelected )
    {
      if ( (USHORT)pCombo->GetItemData(i) == usValue )
      {
        pCombo->SetCurSel(i);
        fSelected = TRUE;
      } /* endif */
      i++;
    } /* endwhile */
  } /* endif pCombo */

  return( fSelected );
}

// get the value asociated with the selected combo box item
int CMUPropertyPage::GetSelectedValue( int iID )
{
  int iReturn = LB_ERR;

  CComboBox *pCombo = (CComboBox *)GetDlgItem( iID );
  if ( pCombo )
  {
    int iSelected = pCombo->GetCurSel();
    if ( iSelected >= 0 )
    {
      iReturn = (int)pCombo->GetItemData( iSelected );
    } /* endif */
  } /* endif */
  return( iReturn );
}

// fill combobox with style text strings and add style value as lParam
BOOL CMUPropertyPage::FillWithStyles( int iComboBoxID, int *piStyles )
{
  CString s;

  CComboBox *pCombo = (CComboBox *)GetDlgItem( iComboBoxID );
  if ( pCombo )
  {
    int i = 0;
    while ( (piStyles[i] != 0) || (piStyles[i+1] != 0) )
    {
      if ( s.LoadString( piStyles[i] ) )
      {
        int iIndex = pCombo->AddString( s );
        if ( iIndex >= 0 )
        {
          pCombo->SetItemData( iIndex, piStyles[i+1] );
        } /* endif */
      } /* endif */
      i += 2;
    } /* endwhile */
  } /* endif */
  return( TRUE );
}

// set input focus to specified dialog box item
void CMUPropertyPage::SetDlgItemFocus( int iControl )
{
  CWnd *wnd = GetDlgItem( iControl );
  wnd->SetFocus();
}

// clear the entry field of a combobox
void CMUPropertyPage::ClearComboSel( int iComboBoxID )
{
  CComboBox *pCombo = (CComboBox *)GetDlgItem( iComboBoxID );
  pCombo->SetCurSel( -1 );
}

// get listbox item index for item wit the given value
int CMUPropertyPage::GetIndexForValue( int iID, DWORD dwValue )
{
  int iIndex = LB_ERR;

  CListBox *pList= (CListBox *)GetDlgItem( iID );
  if ( pList )
  {
    int i = 0;
    int iMax = pList->GetCount();
    while ( (i < iMax) && (iIndex == LB_ERR) )
    {
      if ( pList->GetItemData(i) == dwValue )
      {
        iIndex = i;
      } /* endif */
      i++;
    } /* endwhile */
  } /* endif pList */

  return( iIndex );
}

// activate this property page
void CMUPropertyPage::ActivatePage()
{
  CPropertySheet *pPropSheet = (CPropertySheet *)GetParent();

  pPropSheet->SetActivePage( this );
}

///////////////////////////////////////////////////////////////////////////////
// Markup table unprotect dialog
///////////////////////////////////////////////////////////////////////////////
CMuPropUnprotectDlg::CMuPropUnprotectDlg( CWnd* pParent )
        : CDialog(CMuPropUnprotectDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CMuPropUnprotectDlg)
          // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}


BEGIN_MESSAGE_MAP(CMuPropUnprotectDlg, CDialog)
        //{{AFX_MSG_MAP(CMuPropUnprotectDlg)
        ON_BN_CLICKED(ID_MUPROP_UNPROT_OK_PB, OnOK)
        ON_BN_CLICKED(ID_MUPROP_UNPROT_CANCEL_PB, OnCancel)
        ON_BN_CLICKED(ID_MUPROP_UNPROT_HELP_PB, OnHelp)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CMuPropUnprotectDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  CEdit *pEdit = (CEdit *)GetDlgItem( ID_MUPROP_UNPROT_PASSWORD_EF );
  if ( pEdit ) pEdit->SetLimitText( MAX_FNAME - 1 );

  return TRUE;
}


void CMuPropUnprotectDlg::OnOK()
{
  CString sPassword;

  // get specified password
  GetDlgItemText( ID_MUPROP_UNPROT_PASSWORD_EF, sPassword );
  sPassword.TrimLeft();
  sPassword.TrimRight();

  // handle input
  if ( sPassword.IsEmpty() )
  {
    // password missing
    UtlError( ERROR_NO_PASSWORD_ENTERED, MB_CANCEL, 0, NULL, EQF_ERROR );
    (GetDlgItem( ID_MUPROP_UNPROT_PASSWORD_EF ))->SetFocus();
  }
  else if ( !DicGetCheckPassword( (PSZ)(LPCTSTR)sPassword, &m_ulPassword, TRUE ) )
  {
    // pasword incorrect
    UtlError( ERRMSG_WRONG_PASSWORD, MB_CANCEL, 0, NULL, EQF_ERROR );
    GetDlgItem( ID_MUPROP_UNPROT_PASSWORD_EF )->SetFocus();
  }
  else
  {
    // password correct, leave dialog
    EndDialog( IDOK );
  } /* endif */
}

void CMuPropUnprotectDlg::OnCancel()
{
  EndDialog( IDCANCEL );
}

void CMuPropUnprotectDlg::OnHelp()
{
  UtlInvokeHelp();
}


///////////////////////////////////////////////////////////////////////////////
// Markup table protect dialog
///////////////////////////////////////////////////////////////////////////////
CMuPropProtectDlg::CMuPropProtectDlg( CWnd* pParent )
        : CDialog(CMuPropProtectDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CMuPropProtectDlg)
          // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}


BEGIN_MESSAGE_MAP(CMuPropProtectDlg, CDialog)
        //{{AFX_MSG_MAP(CMuPropProtectDlg)
        ON_BN_CLICKED(ID_MUPROP_PROTECT_OK_PB, OnOK)
        ON_BN_CLICKED(ID_MUPROP_PROTECT_HELP_PB, OnHelp)
        ON_BN_CLICKED(ID_MUPROP_PROTECT_CANCEL_PB, OnCancel)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CMuPropProtectDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  CEdit *pEdit = (CEdit *)GetDlgItem( ID_MUPROP_PROTECT_PASSWORD_EF );
  if ( pEdit ) pEdit->SetLimitText( MAX_FNAME - 1 );

  pEdit = (CEdit *)GetDlgItem( ID_MUPROP_PROTECT_CONF_EF );
  if ( pEdit ) pEdit->SetLimitText( MAX_FNAME - 1 );

  return TRUE;
}


void CMuPropProtectDlg::OnOK()
{
  CString sPassword;
  CString sConfirm;

  // get specified passwords
  GetDlgItemText( ID_MUPROP_PROTECT_PASSWORD_EF, sPassword );
  sPassword.TrimLeft();
  sPassword.TrimRight();
  GetDlgItemText( ID_MUPROP_PROTECT_CONF_EF, sConfirm );
  sConfirm.TrimLeft();
  sConfirm.TrimRight();

  // handle input
  if ( sPassword != sConfirm )
  {
    // error entering password
    UtlError( ERROR_PASSWORDS_NOT_INDENTICAL, MB_CANCEL, 0, NULL, EQF_ERROR );
    (GetDlgItem( ID_MUPROP_PROTECT_PASSWORD_EF ))->SetFocus();
  }
  else if ( sPassword.IsEmpty() )
  {
    UtlError( ERROR_NO_PASSWORD_ENTERED, MB_CANCEL, 0, NULL, EQF_ERROR );
    (GetDlgItem( ID_MUPROP_PROTECT_PASSWORD_EF ))->SetFocus();
  }
  else if ( !DicGetCheckPassword( (PSZ)(LPCTSTR)sPassword, &m_ulPassword, FALSE ) )
  {
    UtlError( ERROR_INV_PASSWORD, MB_CANCEL, 0, NULL, EQF_ERROR );
    SetDlgItemText( ID_MUPROP_PROTECT_PASSWORD_EF, "" );
    SetDlgItemText( ID_MUPROP_PROTECT_CONF_EF, "" );
    (GetDlgItem( ID_MUPROP_PROTECT_PASSWORD_EF ))->SetFocus();
  }
  else
  {
    // protection succesful, leave dialog
    EndDialog( IDOK );
  } /* endif */
}

void CMuPropProtectDlg::OnCancel()
{
  EndDialog( IDCANCEL );
}

void CMuPropProtectDlg::OnHelp()
{
  UtlInvokeHelp();
}

