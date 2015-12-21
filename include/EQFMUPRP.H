//+----------------------------------------------------------------------------+
//|  EQFMUPRP.H - EQF Markup table properties classes                          |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.2 $ ----------- 28 Sep 2004
// GQ: - added OnHelp method for protect and unprotect dialog
// 
// 
// $Revision: 1.1 $ ----------- 31 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.2 $ ----------- 8 Oct 2003
// --RJ: =018279: add m_fReflow to CMUPropGeneralPage
// 
//
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
//
//
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
//
//
// $Revision: 1.2 $ ----------- 24 Feb 2003
// -- RJ: .NET adaptation
//
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.4 $ ----------- 19 Dec 2001
// GQ: - Added m_fPageInitialized to CMUPropTestPage class
//
//
// $Revision: 1.3 $ ----------- 19 Nov 2001
// GQ: - Added m_fPageInitialized to tags and attributes property page
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// GQ: Continued work on markup table property sheet
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.1 $ ----------- 25 Jun 2001
// GQ: Initial put
//
//
//+----------------------------------------------------------------------------+

#include "eqf.id"
#include "eqftag00.id"


// predefined names for markup table test environment
#define MUPROP_TESTFOL   "$$MU$$"
#define MUPROP_TESTTABLE "$$MU$$"
#define MUPROP_TESTLANG  "English(U.S.)"
#define MUPROP_TESTDOC   "MUTEST.DOC"

// tag data structure as used in the markup table property dialog
typedef struct _MUPRPTAG
{
  CHAR      szName[MAX_TAGSIZE];     // buffer for tag name
  CHAR      szEndDelim[MAX_TAGSIZE]; // buffer for end delimiters
  USHORT    Tagtype;           // type of tag
  USHORT    Segbreak;          // segment info
  USHORT    Asstext;           // associated text
  EQF_BOOL  fAttr;             // does the tag allow attributes?
  USHORT    usLength;          // length of tag
  USHORT    usPosition;        // fixed position of tag in text
  EQF_BOOL  fTranslate;        // does the tag contain text to be translated?
  EQF_BOOL  fTRNote;           // does tag contain a TRNote?
  USHORT    usAddInfo;         // additional tag info
  USHORT    ClassId;           // class ID for tag
  USHORT    usFixedTagId;      // fixed ID of tag
} MUPRPTAG, *PMUPRPTAG;

// attribute data structure as used in the markup tsble property dialog
typedef struct _MUPRPATTR
{
  CHAR      szName[MAX_TAGSIZE];     // buffer for attribute name
  CHAR      szEndDelim[MAX_TAGSIZE]; // buffer for end delimiters
  EQF_BOOL  fTranslate;        // does the attribute contain text to be translated?
  USHORT    usAddInfo;         // additional attribute info
  USHORT    usLength;          // length of attribute
  USHORT    usFixedAttrId;     // fixed ID of attribute
  EQF_BOOL  fTRNote;           // does attribute contain a TRNote?
} MUPRPATTR, *PMUPRPATTR;


// standard property page class with some helper functions for MU property pages
class CMUPropertyPage: public CPropertyPage
{
  // Construction
  public:
      CMUPropertyPage( UINT nIDTemplate, UINT nIDCaption = 0 );
      void SetEditTextAndTextLimit( int iControlID, PSZ pszText, int iLimit );
      BOOL SearchSelectValue( int iID, USHORT usValue );
      BOOL FillWithStyles( int iComboBoxID, int *piStyles );
      void SetDlgItemFocus( int iControl );
      void ClearComboSel( int iComboBoxID );
      int GetSelectedValue( int iID );
      void EnableDlgItem( int iControlId, BOOL fEnableState );
      int GetIndexForValue( int iID, DWORD dwValue );
      void ActivatePage();

  // Implementation
  protected:
};




// CMUPropGeneralPage dialog

class CMUPropGeneralPage : public CMUPropertyPage
{
// Construction
public:
        CMUPropGeneralPage();
        BOOL UpdateData( BOOL bSaveAndValidate = TRUE );
        void Protect( BOOL fProtect = TRUE );

// Dialog Data
        //{{AFX_DATA(CMUPropGeneralPage)
        enum { IDD = ID_MUPROP_GENERAL_PAGE };
        //}}AFX_DATA

        // current value of dialog controls
        CHAR m_szTableName[MAX_FNAME];           // name of tag table
        CHAR m_szDescription[MAX_DESCRIPTION];   // description
        CHAR m_szDescrName[MAX_DESCRIPTION];     // descriptive name
        CHAR m_szUserExit[MAX_FILESPEC];         // name of user exit DLL
        USHORT m_usCharSet;                      // ID of selecrted character set
        CHAR m_chSingleSubstChar;                // character used as single substitution symbol
        CHAR m_chMultSubstChar;                  // character used as multiple substitution symbol
        CHAR m_szTrNoteStart[MAX_TRNOTE_DESC];   // translators note start tag
        CHAR m_szTrNoteLevel1[MAX_TRNOTE_DESC];  // translators note level 1
        CHAR m_szTrNoteLevel2[MAX_TRNOTE_DESC];  // translators note level 2
        CHAR m_szTrNoteEnd[MAX_TRNOTE_DESC];     // translators note end tag
        BOOL m_fUseUnicodeForSegFile;            // use Unicode for segmented file
        BOOL m_fNewTagTable;                     // TRUE = this is a new tag table
        BOOL m_fModified;                        // TRUE = data has been modified
        BOOL m_fPageInitialized;       // TRUE = property page has been initialized
        BOOL m_fProtected;             // TRUE = property page is in protect mode
        BOOL m_fReflow;                // 0=notspec. 1=Reflow, 2=NoReflow


// Overrides
        // ClassWizard generate virtual function overrides
        //{{AFX_VIRTUAL(CMUPropGeneralPage)
        protected:
        virtual BOOL OnInitDialog();
        //}}AFX_VIRTUAL


// Implementation
protected:
        // Generated message map functions
        //{{AFX_MSG(CMUPropGeneralPage)
                // NOTE: the ClassWizard will add member functions here
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};




// CMUPropTagsPage dialog

class CMUPropTagsPage : public CMUPropertyPage
{
// Construction
public:
        CMUPropTagsPage();
        ~CMUPropTagsPage();
        BOOL AddTag( PMUPRPTAG pTagData, BOOL fAddToListbox = FALSE );
        int GetTagCount();
        BOOL GetTag( int iTagIndex, PMUPRPTAG pTagData );
        BOOL UpdateData( BOOL bSaveAndValidate );
        void ClearTagProperties();
        void Protect( BOOL fProtect = TRUE );

// Dialog Data
        CPtrArray m_TagList;           // array with tag definitions
        PMUPRPTAG m_pSelectedTag;      // currently selected tag
        int m_iSelectedTagIndex;       // index of currently selected tag
        BOOL m_fUpdateByDialog;        // TRUE = fields are updated by dialog code
        BOOL m_fModified;              // TRUE = data has been modified
        BOOL m_fPageInitialized;       // TRUE = property page has been initialized
        BOOL m_fProtected;             // TRUE = property page is in protect mode

        //{{AFX_DATA(CMUPropTagsPage)
        enum { IDD = ID_MUPROP_TAGS_PAGE };
        //}}AFX_DATA


// Overrides
        // ClassWizard generate virtual function overrides
        //{{AFX_VIRTUAL(CMUPropTagsPage)
        protected:
        //}}AFX_VIRTUAL

// Implementation
protected:
        // Generated message map functions
        //{{AFX_MSG(CMUPropTagsPage)
        afx_msg void OnEditchangeTagDelimCb();
        afx_msg void OnSelChangeTagsLb();
        afx_msg void OnEditchangeTagNameEf();
        afx_msg void OnEditchangeIdEf();
        afx_msg void OnSelChangeSegFlagsCb();
        afx_msg void OnSelChangeTextStyleCb();
        afx_msg void OnSelChangeTagTypeCb();
        afx_msg void OnButtonClickedWithAttrChk();
        afx_msg void OnEditchangeDelimEf();
        afx_msg void OnEditchangeLenEf();
        afx_msg void OnButtonClickedTranslChk();
        afx_msg void OnButtonClickedTrNoteChk();
        afx_msg void OnEditchangeAddInfoEf();
        afx_msg void OnEditchangeColPosEf();
        afx_msg void OnSelChangeClassCb();
        afx_msg void OnButtonClickedAddPB();
        afx_msg void OnButtonClickedDeletePB();


        virtual BOOL OnInitDialog();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};


class CMUPropAttrPage : public CMUPropertyPage
{
// Construction
public:
        CMUPropAttrPage();
        ~CMUPropAttrPage();
        BOOL AddAttr( PMUPRPATTR pAttrData, BOOL fAddToListbox = FALSE  );
        int GetAttrCount();
        BOOL GetAttr( int iTagIndex, PMUPRPATTR pAttrData );
        void ClearAttrProperties();
        BOOL UpdateData( BOOL bSaveAndValidate = TRUE );
        void Protect( BOOL fProtect = TRUE );

// Dialog Data
        CPtrArray m_AttrList;          // array with attribute definitions
        PMUPRPATTR m_pSelectedAttr;    // currently selected attribute
        int m_iSelectedAttrIndex;      // index of currently selected attribute
        BOOL m_fUpdateByDialog;        // TRUE = fields are updated by dialog code
        BOOL m_fModified;              // TRUE = data has been modified
        BOOL m_fPageInitialized;       // TRUE = property page has been initialized
        BOOL m_fProtected;             // TRUE = property page is in protect mode
        //{{AFX_DATA(CMUPropAttrPage)

        enum { IDD = ID_MUPROP_ATTR_PAGE };
        //}}AFX_DATA

        // current value of dialog controls


// Overrides
        // ClassWizard generate virtual function overrides
        //{{AFX_VIRTUAL(CMUPropAttrPage)
        protected:
        afx_msg void OnSelChangeAttrLb();
        afx_msg void OnEditchangeAttrNameEf();
        afx_msg void OnEditchangeLengthEf();
        afx_msg void OnEditchangeAddInfoEf();
        afx_msg void OnEditchangeDelimEf();
        afx_msg void OnButtonClickedTranslChk();
        afx_msg void OnButtonClickedAddPB();
        afx_msg void OnButtonClickedDeletePB();
        virtual BOOL OnInitDialog();
        //}}AFX_VIRTUAL


// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CMUPropAttrPage)
                // NOTE: the ClassWizard will add member functions here
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

class CMUPropTestPage : public CMUPropertyPage
{
// Construction
public:
        CMUPropTestPage();
        ~CMUPropTestPage();
        BOOL UpdateData( BOOL bSaveAndValidate = TRUE );
        void Protect( BOOL fProtect = TRUE );
        DWORD LoadRichTextFile( PSZ pszRichTextFile );
        BOOL BuildSegFile( PSZ pszDoc, PSZ pszSegFile, PTAGTABLE pTable );
        BOOL BuildRichTextFile( PSZ pszSegFile, PSZ pszRichTextFile, PTAGTABLE pTable );
        void SetDocName( PSZ pszDocName );
        BOOL GetSelectedText( CString &s );

// Dialog Data
        BOOL m_fUpdateByDialog;        // TRUE = fields are updated by dialog code
        BOOL m_fModified;              // TRUE = data has been modified
        CRichEditCtrl m_EditCtrl;      // our rich edit control
        CRichEditCtrl m_EditCtrlLegend; // legend text edit control
        BOOL m_fPageInitialized;       // TRUE = property page has been initialized
        BOOL m_fProtected;             // TRUE = property page is in protect mode
        //{{AFX_DATA(CMUPropTestPage)

        enum { IDD = ID_MUPROP_TEST_PAGE };
        //}}AFX_DATA

        // current value of dialog controls


// Overrides
        // ClassWizard generate virtual function overrides
        //{{AFX_VIRTUAL(CMUPropTestPage)
        protected:
        afx_msg void OnSelChangeAttrLb();
        afx_msg void OnButtonClickedLoadPB();
        afx_msg void OnButtonClickedAddTagPB();
        afx_msg void OnButtonClickedAddAttrPB();
        afx_msg void OnButtonClickedRefreshPB();
        afx_msg void OnSelChangeRichEdit();
        virtual BOOL OnInitDialog();
        //}}AFX_VIRTUAL


// Implementation
protected:
        void PreprocessTextData( PSZ pszIn, PSZ pszOut );
        void PreprocessTextData( PSZ pszIn, PSZ pszOut, int iLen );

        // Generated message map functions
        //{{AFX_MSG(CMUPropTestPage)
                // NOTE: the ClassWizard will add member functions here
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};




// Interface of the CMUPropSheet class
//
class CMUPropSheet : public CPropertySheet
{
public:
        DECLARE_DYNAMIC(CMUPropSheet)
        CMUPropSheet(CWnd* pWndParent = NULL);
        ~CMUPropSheet();
        int OnCreate(LPCREATESTRUCT lpCreateStruct );

// Attributes

// Operations
        void SetTagTableObjName( PSZ pszTagTableObjName )
        { strcpy( m_szTagTableObjName, pszTagTableObjName ); }
// Overrides
        virtual BOOL OnInitDialog();

// Message Handlers
protected:
        BOOL BuildTagTableFromProperties();
        BOOL ConvertTagString( PSZ pszIn, PSZ pszOut );
        CMUPropGeneralPage m_generalPage;
        CMUPropTagsPage    m_tagsPage;
        CMUPropAttrPage    m_attrPage;
        CMUPropTestPage    m_testPage;
        char               m_szTagTableObjName[MAX_EQF_PATH];
        char               m_szTagTableFileName[MAX_LONGFILESPEC];
        char               m_szTagTableName[MAX_FILESPEC];
        PTAGTABLE          m_pTagTable;
        BOOL               m_fNewTagTable;       // TRUE = this is a new tag table
        BOOL               m_fProtected;         // TRUE = tag tabe is protected
        ULONG              m_ulPassword;         // protection password
        char               m_szDocName[MAX_LONGPATH];   // name of test document
        // fully qualified file names for test mode
        char               m_szDocProp[MAX_EQF_PATH];   // name of test document propfile
        char               m_szDocObj[MAX_EQF_PATH];    // test document object name
        char               m_szDocSource[MAX_EQF_PATH]; // name of source document
        char               m_szDocSeg[MAX_EQF_PATH];    // name of segmented document
        char               m_szDocRTF[MAX_EQF_PATH];    // name of document in RTF format
        char               m_szTestTable[MAX_EQF_PATH]; // name of markup table file

        //{{AFX_MSG(CMUPropSheet)
        afx_msg void OnOKPB();
        afx_msg void OnCancelPB();
        afx_msg void OnProtectPB();
        afx_msg void OnHelpPB();
        afx_msg void OnTestLoadPB();
        afx_msg void OnTestAddTagPB();
        afx_msg void OnTestAddAttrPB();
        afx_msg void OnTestRefreshPB();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};



class CMuPropProtectDlg : public CDialog
{
// Construction
public:
        CMuPropProtectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(CMuPropProtectDlg)
        enum { IDD = ID_MUPROP_PROTECT_DLG };
                // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CMuPropProtectDlg)
        protected:
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CMuPropProtectDlg)
        virtual BOOL OnInitDialog();
        afx_msg void OnCancel();
        afx_msg void OnHelp();
        afx_msg void OnOK();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
public:
        ULONG m_ulPassword;               // protection password

};

class CMuPropUnprotectDlg : public CDialog
{
// Construction
public:
        CMuPropUnprotectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(CMuPropUnprotectDlg)
        enum { IDD = ID_MUPROP_UNPROTECT_DLG };
                // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CMuPropUnprotectDlg)
        protected:
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CMuPropUnprotectDlg)
        virtual BOOL OnInitDialog();
        afx_msg void OnCancel();
        afx_msg void OnHelp();
        afx_msg void OnOK();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
public:
        ULONG m_ulPassword;               // unprotection password

};

// structure used by callback MuPropLoadTextFromFile for current state info
typedef struct _MUPROP_FILELOAD_INFO
{
  enum
  {
    OPEN,                              // open the input file
    READ,                              // read next block of data
    DONE                               // processing finished
  } CurrentState;                      // current state of processing
  HFILE hFileIn;                       // handle of input file
  PSZ pszInFile;                       // points to fully qualified name of input file
} MUPROP_FILELOAD_INFO, *PMUPROP_FILELOAD_INFO;

// callback function for rich text control StreamIn processing
DWORD CALLBACK MuPropLoadTextFromFile(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
