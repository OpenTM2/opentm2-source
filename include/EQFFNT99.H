
//+----------------------------------------------------------------------------+
//|EQFFNT99.H    document handler for MFC                                      |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
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
// $Revision: 1.2 $ ----------- 3 May 2005
// GQ: - fixed S607006: Help button does not work: Fonts (RTFEdit)
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
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
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.3 $ ----------- 18 Jun 2001
// -- RJ: R006405_COLOR_TRNOTE: adapt for new item
//
//
// $Revision: 1.2 $ ----------- 9 Oct 2000
// -- clean up
//
//
//
// $Revision: 1.1 $ ----------- 21 Jun 2000
// -- new defines for R_FONT
//
//+----------------------------------------------------------------------------+

//#include "eqftpi.h"

/**********************************************************************/
/* 1st default for extended font specs                                */
/**********************************************************************/
#define  DEF1_FNAME   "System"



class CDisplayIC : public CDC
{
public:
        CDisplayIC() { CreateIC(_T("DISPLAY"), NULL, NULL, NULL); }
};

struct CCharFormat : public CHARFORMAT
{
        CCharFormat() {cbSize = sizeof(CHARFORMAT);}
        BOOL operator==(CCharFormat& cf);
};

static int nFontSizes[] =
{ 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};



#define FN_SETFORMAT    0x1000
#define FN_GETFORMAT    0x1001

#define FIRST   1
#define NOT_FIRST 2
/////////////////////////////////////////////////////////////////////////////
// CFontDesc

class CFontDesc
{
public:
        CFontDesc(LPCTSTR lpszName, LPCTSTR lpszScript, BYTE nCharSet,
                BYTE nPitchAndFamily, DWORD dwFlags) //;
        {
                m_strName = lpszName;
                m_strScript = lpszScript;
                m_nCharSet = nCharSet;
                m_nPitchAndFamily = nPitchAndFamily;
                m_dwFlags = dwFlags;
                m_nFirst = 0;
        };
        void       SetNext(CFontDesc * pFontDesc){ pNext = pFontDesc;}
        CFontDesc * GetNext(){ return pNext;}
        CString     m_strName;
        CString     m_strScript;
        BYTE        m_nCharSet;
        BYTE        m_nPitchAndFamily;
        DWORD       m_dwFlags;
        CFontDesc * pNext;
        BYTE        m_nFirst;
};



/////////////////////////////////////////////////////////////////////////////
// CColorComboBox

  class CColorComboBox : public CComboBox
  {
  public:
        CColorComboBox();
        CColorComboBox(HWND);
  //Attribute
        HWND       hwndParent;
  // Operationen
        void       SetHwnd(HWND hwnd){ hwndParent = hwnd;}

  // Implementierung
  public:
//šberladungen
        virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
        virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
//      virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCIS);needed??

          // Generierte Message-Map-Funktionen
          //{{AFX_MSG(CColorComboBox)
          afx_msg void OnDestroy();
          afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
          //}}AFX_MSG
          DECLARE_MESSAGE_MAP()
  };

class CFontComboBox : public CComboBox
{
public:
        CFontComboBox();
        CFontComboBox(HWND);

//Attribute
        CBitmap    m_bmFontType;
        HWND       hwndParent;
        CPtrArray  m_arrayFontDesc;
        static int m_nFontHeight;
        int        m_nLimitText;
        void       SetHwnd(HWND hwnd){ hwndParent = hwnd;}
        BOOL HasFocus()
        {
                HWND hWnd = ::GetFocus();
                return (hWnd == m_hWnd || ::IsChild(m_hWnd, hWnd));
        }
        void       GetTheText(CString& str);
        void       SetTheText(LPCTSTR lpszText,BOOL bMatchExact = FALSE);

//Operationen
        BOOL LimitText(int nMaxChars);
        void EnumFontFamiliesEx(HDC hDC, BYTE nCharSet = DEFAULT_CHARSET);
        void AddFont(ENUMLOGFONT* pelf, DWORD dwType, LPCTSTR lpszScript = NULL);
        void MatchFont(LPCTSTR lpszName, BYTE nCharSet);
        void EmptyContents();

        static BOOL CALLBACK AFX_EXPORT EnumFamScreenCallBackEx(
                ENUMLOGFONTEX* pelf, NEWTEXTMETRICEX* /*lpntm*/, int FontType,
                LPVOID pThis);
        static BOOL CALLBACK AFX_EXPORT EnumFamPrinterCallBackEx(
                ENUMLOGFONTEX* pelf, NEWTEXTMETRICEX* /*lpntm*/, int FontType,
                LPVOID pThis);

//Überladungen
        virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
        virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
        virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCIS);
        // Generierte Message-Map-Funktionen
        //{{AFX_MSG(CFontComboBox)
        afx_msg void OnDestroy();
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// EQFFONTEXT dialog

class CFontExtDlg : public CDialog
{
// Construction
public:
        CFontExtDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
        //{{AFX_DATA(CFontExtDlg)
        enum { IDD = ID_TB_FONTEXT_DLG };
                // NOTE: the ClassWizard will add data members here
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CFontExtDlg)
        protected:
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CFontExtDlg)
        virtual BOOL OnInitDialog();
        afx_msg void OnCancel();
        afx_msg void OnOK();
        afx_msg void OnHelp();
        afx_msg void OnDefault();
        afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnFontNameKillFocus();
        afx_msg void OnComboSetFocus();
        afx_msg void OnFontComboCloseUp();
        afx_msg void OnSizeComboCloseUp();
        afx_msg void OnUpdateRequest();
        afx_msg MRESULT On_WM_EQF_QUERYID( WPARAM mp1, LPARAM mp2 )
                { mp1; HANDLEQUERYID( ID_TB_FONTEXT_DLG, mp2 ); return( 0 ); }
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
public:
       void EQFInsertTree();
       HTREEITEM EQFFillOneTreeGroup(HTREEITEM hParent, UINT uiStart,
                            UINT uiNumInGroup, CTreeCtrl* pTree,
                            TV_INSERTSTRUCT* pTreeCtrlItem, PUSHORT pusItemIndex );
       void EQFInsertComboBoxes();
       void EQFFillAndSelSize(CFontComboBox* pFontCBS);
       void EQFFillAndSelScript(CFontComboBox* pFontCBS, BYTE nCharSet);
       void SetHwnd(HWND hwnd){ hwndDoc = hwnd;}
       HWND GetHwnd(){return hwndDoc;}
       void GetAllSelects(int i);
       void SaveAllSpecs();
       void EQFInitAllSpecs(int i);
       void EQFEnableAll(BOOL bEnable);

//functions for Preview box
       void FillPreview( int PreviewOpt );

//functions for size combobox
       void SetTwipSize(int nSize, CComboBox * pCBS);
       int  GetTwipSize(CComboBox* pCBS);
        void EnumFontSizes(CDC& dc, LPCTSTR pFontName, CComboBox* pCBS);
        static BOOL FAR PASCAL EnumSizeCallBack(LOGFONT FAR* lplf,
                LPNEWTEXTMETRIC lpntm,int FontType, LPVOID lpv);
        void TwipsToPointString(LPTSTR lpszBuf, int nTwips);
        void InsertSize(int nSize);
//Attribute for size combobox
        int m_nLogVert;
        int m_nTwipsLast;

//Attribute
       HWND            hwndDoc;
       CColorComboBox  m_FGColorCBS;
       CColorComboBox  m_BGColorCBS;
       CFontComboBox   m_FontCBS;
       CHARFORMAT2     m_CharFormat[MAXEXTTYPE];
       HTREEITEM       hTreeItem1;   // 1st item with fontspec in tree
           int             actSel;
};
