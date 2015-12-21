/*! \file
	Description: EQF Extended font dialog
	
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqfmfc.h"

#ifndef CPPTEST
extern "C"
{
#endif
  #define INCL_EQF_EDITORAPI
  #define INCL_EQF_TP
  #define INCL_EQF_ASD
  #define INCL_EQF_TM
  #define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_FOLDER           // folder list and document list functions
  #define INCL_EQF_ANALYSIS            // Analysis functions

/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                  // General .H for EQF
  #include "eqfstart.id"            // IDs for EQFSTARR resource
  #include "eqfdde.h"               // batch mode definitions
  #include "eqfdoc00.h"             // for FLL_OBJECT_IND
  #include "eqfstart.h"
  #include "eqfb.id"
  #include "eqfbdlg.id"
  #include "eqftpi.h"

#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif

#include "eqfclbox.h"   // base class for Column listbox
#include "eqfDOC99.h"
#include "eqffnt99.h"
#include "eqfcfrm.h"    // base class for child frames
#include "eqfd.h"

#define HAS_NOFONTSPEC 2
#define HAS_FONTSPEC 1

#define PRINTER_FONT 0x0100
#define TT_FONT      0x0200
#define DEVICE_FONT  0x0400

#define BMW   16
#define BMH   15


#define PREVIEW_TRANSLATION 1
#define PREVIEW_DICT        2
#define PREVIEW_PROPOSAL    4
#define PREVIEW_POSTEDIT    8


  EXTCOLOREX aszDef1FontExtSpecs[MAXEXTTYPE] =
  {
  // FG, BG,
  //---------------------------------------------------------------------------
   { COL_BLUE,  COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0}, // EXT_T_SOURCE
   { COL_BLACK, COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_T_TRANSLATION
   { COL_RED,   COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_T_LAYOUT
   { COL_BLACK, COL_YELLOW, STYLE_STANDARD, UNDERLINE_NONE, 0 },   // EXT_T_ACTIVE
   { COL_BLACK, COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_T_TRNOTE
   { COL_BLACK, COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_R_SCRATCH
   { COL_BLACK, COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_R_MODIFIED
   { COL_BLACK, COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_R_COPIED
   { COL_BLACK, COL_LIGHTGRAY, STYLE_STANDARD, UNDERLINE_YES, 0 },// EXT_R_LAYOUTCHANGES
   { COL_RED,   COL_WHITE, STYLE_ITALIC,   UNDERLINE_NONE, 0 },    // EXT_D_HEADWORD
   { COL_BLUE,  COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_D_TRANSLATION
   { COL_RED,   COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_D_PREFIX
   { COL_LIGHTGRAY, COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },// EXT_D_ADDITIONINFO
   { COL_RED,   COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_M_PREFIX
   { COL_BLACK, COL_LIGHTCYAN, STYLE_STANDARD, UNDERLINE_NONE, 0 },// EXT_M_PMODIFIED
   { COL_BLACK, COL_LIGHTGRAY, STYLE_STANDARD, UNDERLINE_NONE, 0 },// EXT_M_PINSERTED
   { COL_BLACK, COL_LIGHTPINK, STYLE_STANDARD, UNDERLINE_NONE, STRIKEOUT },// EXT_M_PDELETED
   { COL_BLACK, COL_LIGHTGRAY, STYLE_STANDARD, UNDERLINE_NONE, 0 },// EXT_T_RTFEDIT_TRNOTE
// { COL_BLACK, COL_CYAN, STYLE_STANDARD, UNDERLINE_NONE, 0  },    // EXT_I_ANCHOR1
// { COL_BLUE,  COL_CYAN, STYLE_STANDARD, UNDERLINE_NONE, 0  },    // EXT_I_ANCHOR2
// { COL_LIGHTGRAY, COL_CYAN, STYLE_STANDARD, UNDERLINE_NONE, 0 }, // EXT_I_ANCHOR3
// { COL_BLACK, COL_LIGHTRED, STYLE_STANDARD, UNDERLINE_NONE, 0 }, // EXT_I_UNALIGNED
// { COL_WHITE, COL_LIGHTGRAY, STYLE_STANDARD, UNDERLINE_NONE, STRIKEOUT },// EXT_I_CROSSEDOUT
// { COL_BLACK, COL_LIGHTCYAN, STYLE_STANDARD, UNDERLINE_NONE, 0 },// EXT_I_OVERCROSS
// { COL_BLACK, COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_I_ALIGN1
// { COL_BLUE,  COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 },    // EXT_I_ALIGN2
// { COL_GREEN, COL_WHITE, STYLE_STANDARD, UNDERLINE_NONE, 0 }     // EXT_I_ALIGN3
   };

int CFontComboBox::m_nFontHeight = 0;

/**********************************************************************/
/* Translation preview                                                */
/*   Die {\\i Computeranwendungen}, die gewnscht sind.               */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/**********************************************************************/
typedef struct _PREVIEWTRANS
{
  USHORT usDispType;                   // display type
  USHORT usSegStatus;                  // segment status
  USHORT usHLState;                    // TAG_HIGHLIGHT
  CHAR   chString[50];                 // text to be displayed
} PREVIEWTRANS, *PPREVIEWTRANS;

PREVIEWTRANS PreviewTrans[] =
{
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "Die "                     },
  { PROTECTED_CHAR,    QF_XLATED, 0, "{\\i "                    },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "Computeranwendungen"      },
  { PROTECTED_CHAR,    QF_XLATED, 0, "}"                        },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, ", die angestrebt sind.\n"  },
  { PROTECTED_CHAR,    QF_NOP,    0, "\\par\n"                  },
  { UNPROTECTED_CHAR,  QF_CURRENT,0, "The amount of data that you wish to process\n"  },
  { PROTECTED_CHAR,    QF_NOP,    0, "\\par\n"                  },
  { UNPROTECTED_CHAR,  QF_TOBE,   0, "The amount of data storage required\n"  },
  { 0,                 0,         0, ""  }
};

PREVIEWTRANS PreviewPostEdit[] =
{
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "Die "                     },
  { PROTECTED_CHAR,    QF_XLATED, 0, "{\\i "                    },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "Computeranwendungen"      },
  { PROTECTED_CHAR,    QF_XLATED, 0, "}"                        },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, ", die angestrebt sind.\n"  },
  { PROTECTED_CHAR,    QF_NOP,    0, "\\par\n"                  },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "Die "                     },
  { PROTECTED_CHAR,    QF_XLATED, TAG_HIGHLIGHT, "{\\i "        },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "Datenmenge"               },
  { PROTECTED_CHAR,    QF_XLATED, TAG_HIGHLIGHT, "}"            },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, ", die verarbeitet werden soll\n"  },
  { PROTECTED_CHAR,    QF_NOP,    0, "\\par\n"                  },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "Der notwendige Datenspeicher\n"  },
  { 0,                 0,         0, ""  }
};

// EXT_M_PREFIX
// EXT_M_PMODIFIED
// EXT_M_PINSERTED
// EXT_M_PDELETED
PREVIEWTRANS PreviewProposal[] =
{
  { UNPROTECTED_CHAR,  QF_PROP0PREFIX, 0, "0[3] - "                     },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "The amount of data that you wish to process\n"      },
  { UNPROTECTED_CHAR,  QF_PROPNPREFIX, 0, "1[f 06.10.2000] - "                     },
  { UNPROTECTED_CHAR,  QF_PROPSRCEQU, 0, "The amount of data that you wish to process "      },
  { PROTECTED_CHAR,    QF_PROPSRCINS, 0, "today"                        },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "\n"  },
  { UNPROTECTED_CHAR,  QF_PROPNPREFIX, 0, "2[f 06.10.2000] - "                     },
  { UNPROTECTED_CHAR,  QF_PROPSRCEQU,  0, "The amount of data that you wish to "      },
  { PROTECTED_CHAR,    QF_PROPSRCUNEQU,0, "translate"                        },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "\n"  },
  { UNPROTECTED_CHAR,  QF_PROPNPREFIX,   0, "3[f 06.10.2000] - "                     },
  { UNPROTECTED_CHAR,  QF_PROPSRCEQU,  0, "The amount of data that you wish "      },
  { PROTECTED_CHAR,    QF_PROPSRCDEL,  0, "?"                        },
  { UNPROTECTED_CHAR,  QF_XLATED, 0, "\n"  },
  { 0,                 0,         0, ""  }
};


PREVIEWTRANS PreviewDict[] =
{
  { UNPROTECTED_CHAR, QF_DICTHEAD,   0, "amount\n"                     },
  { UNPROTECTED_CHAR, QF_DICTADDINFO,0, "     no additional information\n"},
  { UNPROTECTED_CHAR, QF_DICTPREFIX, 0, "a) "                          },
  { UNPROTECTED_CHAR, QF_DICTADDINFO,0, "[1] "                         },
  { UNPROTECTED_CHAR, QF_DICTTRANS,  0, "Menge"                        },
  { UNPROTECTED_CHAR, QF_XLATED,     0, "\n"  },
  { UNPROTECTED_CHAR, QF_DICTPREFIX, 0, "b) "                          },
  { UNPROTECTED_CHAR, QF_DICTADDINFO,0, "[1] "                         },
  { UNPROTECTED_CHAR, QF_DICTTRANS,  0, "betragen"                     },
  { UNPROTECTED_CHAR, QF_XLATED,     0,  "\n"  },
  { 0,                0,             0,  ""  }
};


/////////////////////////////////////////////////////////////////////////////
// CFontExtDlg dialog


CFontExtDlg::CFontExtDlg(CWnd* pParent /*=NULL*/)
        : CDialog(CFontExtDlg::IDD, pParent)
{
        //{{AFX_DATA_INIT(CFontExtDlg)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT
  actSel = 0;
}


BEGIN_MESSAGE_MAP(CFontExtDlg, CDialog)
        //{{AFX_MSG_MAP(CFontExtDlg)
        ON_WM_DESTROY()
        ON_BN_CLICKED(ID_TB_FONTEXT_CANCEL_PB, OnCancel)
        ON_BN_CLICKED(ID_TB_FONTEXT_OK_PB, OnOK)
        ON_BN_CLICKED(ID_TB_FONTEXT_HELP_PB, OnHelp)
        ON_BN_CLICKED(ID_TB_FONTEXT_DEFAULT_PB, OnDefault)
        ON_NOTIFY(TVN_SELCHANGED, ID_TB_FONTEXT_TREE, OnSelchanged)
        ON_NOTIFY(TVN_SELCHANGING, ID_TB_FONTEXT_TREE, OnSelchanging)

        ON_CBN_SETFOCUS(ID_TB_FONTEXT_SIZE, OnComboSetFocus)
        ON_CBN_CLOSEUP(ID_TB_FONTEXT_SIZE, OnSizeComboCloseUp)
         ON_CBN_CLOSEUP(ID_TB_FONTEXT_FONT_CBS, OnFontComboCloseUp)

//        ON_CBN_SELENDOK(ID_TB_FONTEXT_FONT_CBS, OnUpdateRequest )
        ON_CBN_SELENDOK(ID_TB_FONTEXT_FONTSTYLE_CBS, OnUpdateRequest )
        ON_CBN_SELENDOK(ID_TB_FONTEXT_SIZE, OnUpdateRequest )
        ON_CBN_SELENDOK(ID_TB_FONTEXT_FGCOLOR, OnUpdateRequest )
        ON_CBN_SELENDOK(ID_TB_FONTEXT_BGCOLOR, OnUpdateRequest )
        ON_CBN_SELENDOK(ID_TB_FONTEXT_SCRIPT, OnUpdateRequest )


        ON_BN_CLICKED( ID_TB_FONTEXT_STRIKETHROUGH, OnUpdateRequest )
        ON_BN_CLICKED( ID_TB_FONTEXT_UNDERLINE, OnUpdateRequest )

        ON_CBN_KILLFOCUS(ID_TB_FONTEXT_FONT_CBS, OnFontNameKillFocus)
        ON_MESSAGE( WM_EQF_QUERYID,                   On_WM_EQF_QUERYID)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontExtDlg message handlers

BOOL CFontExtDlg::OnInitDialog()
{
  /********************************************************************/
  /* Dialog initialisation                                            */
  /********************************************************************/
  int i;
  CHAR  chStrBuf[40];
  CHARFORMAT2 * pDlgCharFormat;
  CHARFORMAT2 * pEditCharFormat;

  //SetWindowLong( GWL_ID, ID_TB_FONTEXT_DLG );
  
  m_nTwipsLast = 0;
  CDialog::OnInitDialog();
  m_FontCBS.SubclassDlgItem(ID_TB_FONTEXT_FONT_CBS, this);
  m_FGColorCBS.SubclassDlgItem(ID_TB_FONTEXT_FGCOLOR, this);
  m_BGColorCBS.SubclassDlgItem(ID_TB_FONTEXT_BGCOLOR, this);

  // enable DBCS display for combo-boxes..
  CComboBox *pCBS = (CComboBox*)GetDlgItem(ID_TB_FONTEXT_FONT_CBS );
  if (pCBS)
  {
	  COMBOBOXINFO cbi;
	  cbi.cbSize = sizeof(COMBOBOXINFO);
	  if (pCBS->GetComboBoxInfo(&cbi))
	  {
		  ::SendMessage( cbi.hwndItem, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), NULL);
		  ::SendMessage( cbi.hwndList, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), NULL);
	  }

  }

  pCBS = (CComboBox*)GetDlgItem(ID_TB_FONTEXT_SCRIPT );
  if (pCBS)
  {
	  COMBOBOXINFO cbi;
	  cbi.cbSize = sizeof(COMBOBOXINFO);
	  if (pCBS->GetComboBoxInfo(&cbi))
	  {
		  ::SendMessage( cbi.hwndItem, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), NULL);
		  ::SendMessage( cbi.hwndList, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), NULL);
	  }

  }




  EQFInsertTree();                  //fill tree structure
  EQFInsertComboBoxes();            //fill cbs according to availb. fonts

  pEditCharFormat = (CHARFORMAT2 *)EQFBGetFontSpecs();
  for (i = 0; i < MAXEXTTYPE; i++)
  {
    pDlgCharFormat = &m_CharFormat[i];
    memcpy(pDlgCharFormat, pEditCharFormat, sizeof(CHARFORMAT2));
    pEditCharFormat ++;
  }

  CTreeCtrl* pTree = ((CTreeCtrl *)GetDlgItem( ID_TB_FONTEXT_TREE));
  pTree->SelectItem(hTreeItem1);

  LOADSTRING(NULLHANDLE, AfxGetResourceHandle(),
               IDS_TB_FONTEXT_TITLE, chStrBuf);
  SetWindowText( chStrBuf );
  return TRUE;
}


void CFontExtDlg::EQFInsertTree()
{
   TV_INSERTSTRUCT TreeCtrlItem;
   CHAR            chStrBuf[50];
   UINT            uiItemsInGroup;
   UINT            uiStart;
   USHORT          usI = 0;
   USHORT          usGroupStrID = 0;

   HTREEITEM       hRoot;
   HTREEITEM       hParent;
   BOOL            fFirst = TRUE;
   USHORT          usJ = 0;
   UINT            uiExtTextTypeNum = 0;

   CTreeCtrl* pTree = ((CTreeCtrl *)GetDlgItem( ID_TB_FONTEXT_TREE));

   TreeCtrlItem.hInsertAfter = TVI_LAST;
   TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM;

   TreeCtrlItem.hParent = TVI_ROOT;
   LOADSTRING(NULLHANDLE, AfxGetResourceHandle(),
                IDS_TB_FONTEXT_ROOT, chStrBuf);
   TreeCtrlItem.item.pszText = chStrBuf;
   TreeCtrlItem.item.lParam = 0;
   hRoot = pTree->InsertItem(&TreeCtrlItem);

   usI = EXT_T_GROUP;
   usJ = EXT_T_GROUP;
   usGroupStrID = IDS_TB_FONTEXT_TGROUP;
   //usStart = IDS_TB_FONTEXT_T_SOURCE;
   uiStart = 0;
   PUSHORT pusFTypesInGroup = EQFBGetFontTypesInGroup();
   uiExtTextTypeNum = EXT_T_SOURCE;        // should be first in EXTTEXTTYPE enum!
   while (usI < MAXEXTGROUP )
   {
     TreeCtrlItem.hParent = hRoot;
     LOADSTRING(NULLHANDLE, AfxGetResourceHandle(),
                  usGroupStrID, chStrBuf);
     TreeCtrlItem.item.pszText = chStrBuf;
     TreeCtrlItem.item.lParam = MAKELONG(usI, HAS_NOFONTSPEC);  //@@
     hParent = pTree->InsertItem(&TreeCtrlItem);

     uiItemsInGroup = pusFTypesInGroup[uiStart];

     if (fFirst )
     {
       hTreeItem1 = EQFFillOneTreeGroup ( hParent, uiStart, uiExtTextTypeNum,
                       pTree, &TreeCtrlItem, pusFTypesInGroup);
       fFirst = FALSE;
     }
     else
     {
       EQFFillOneTreeGroup ( hParent, uiStart, uiExtTextTypeNum,
                       pTree, &TreeCtrlItem, pusFTypesInGroup);
     } /* endif */
     pTree->Expand(hParent, TVE_EXPAND );

     uiStart += uiItemsInGroup + 1;
     uiExtTextTypeNum += uiItemsInGroup;
     usI ++;
     usGroupStrID ++;
   } /* endwhile */
   pTree->Expand(hRoot, TVE_EXPAND );
  }


HTREEITEM CFontExtDlg::EQFFillOneTreeGroup(HTREEITEM hParent, UINT uiStart,
                                  UINT uiExtTextTypeNum, CTreeCtrl* pTree,
                                  TV_INSERTSTRUCT* pTreeCtrlItem,
                                  PUSHORT pusItemIndex)
{
  CHAR            chStrBuf[60];
  USHORT          usIndex = 0;
  USHORT          usJ;
  BOOL            fFirst = TRUE;
  HTREEITEM       hTreeItem = NULL;
  USHORT          usNumInGroup;

  pTreeCtrlItem->hParent = hParent;

  //usJ = usStart - (IDS_TB_FONTEXT_T_SOURCE);
  usNumInGroup = pusItemIndex[uiStart];
  while (usIndex < usNumInGroup )
  {
    // for debugging only:
    usJ = pusItemIndex[uiStart + usIndex + 1];
     LOADSTRING(NULLHANDLE, AfxGetResourceHandle(),
                   pusItemIndex[uiStart + usIndex + 1], chStrBuf);  // old : usStart + usIndex
     pTreeCtrlItem->item.pszText = chStrBuf;
     pTreeCtrlItem->item.lParam = MAKELONG(uiExtTextTypeNum + usIndex, HAS_FONTSPEC);
     if (fFirst )
     {
       hTreeItem = pTree->InsertItem(pTreeCtrlItem);
       fFirst = FALSE;
     }
     else
     {
       pTree->InsertItem(pTreeCtrlItem);
     } /* endif */
     usIndex ++;
     //usJ++;
  } /* endwhile */
  return (hTreeItem);
  }

void CFontExtDlg::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	pNMHDR; pResult;
}

void CFontExtDlg::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult)
{
  pResult;
  BOOL  bEnable;
  NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

  LONG lOldParam = pNMTreeView->itemOld.lParam;
  LONG lNewParam = pNMTreeView->itemNew.lParam;
  if (lOldParam && (HIWORD(lOldParam) == HAS_FONTSPEC ))
  {
    int i = LOWORD(lOldParam);
    //retrieve all font settings of the old tree item
    GetAllSelects(i);
  } /* endif */
  if (lNewParam && (HIWORD(lNewParam) == HAS_FONTSPEC ))
  {
    //set all selections accordingly
    actSel = LOWORD(lNewParam);
    if ((0 <= actSel) && (actSel < MAXEXTTYPE) )
    {
      EQFInitAllSpecs(actSel);
      switch ( actSel )
      {
        case EXT_T_SOURCE:
        case EXT_T_TRANSLATION:
        case EXT_T_LAYOUT:
        case EXT_T_ACTIVE:
        case EXT_T_TRNOTE:
        case EXT_T_RTFEDIT_TRNOTE:
          FillPreview( PREVIEW_TRANSLATION );
          break;
        case EXT_R_SCRATCH:
        case EXT_R_MODIFIED:
        case EXT_R_COPIED:
        case EXT_R_LAYOUTCHANGES:
          FillPreview( PREVIEW_POSTEDIT );
          break;
        case EXT_D_HEADWORD:
        case EXT_D_TRANSLATION:
        case EXT_D_PREFIX:
        case EXT_D_ADDITIONINFO:
          FillPreview( PREVIEW_DICT );
          break;
        case EXT_M_PREFIX:
        case EXT_M_PMODIFIED:
        case EXT_M_PINSERTED:
        case EXT_M_PDELETED:
          FillPreview( PREVIEW_PROPOSAL );
          break;

      } /* endswitch */

    } /* endif */
  } /* endif */


  if ( HIWORD(lNewParam) == HAS_NOFONTSPEC )
  {
    //set all selections accordingly
    actSel = LOWORD(lNewParam);
    if ((0 <= actSel) && (actSel < MAXEXTGROUP) )
    {
      switch ( actSel )
      {
        case EXT_T_GROUP:
          FillPreview( PREVIEW_TRANSLATION );
          break;
        case EXT_R_GROUP:
          FillPreview( PREVIEW_POSTEDIT );
          break;
        case EXT_D_GROUP:
          FillPreview( PREVIEW_DICT );
          break;
        case EXT_M_GROUP:
          FillPreview( PREVIEW_PROPOSAL );
          break;

      } /* endswitch */

    } /* endif */
  } /* endif */




  if ((HIWORD(lOldParam) == HAS_FONTSPEC) &&
      (HIWORD(lNewParam) == HAS_NOFONTSPEC) )
  {
    //disable the dialog controls except the tree
    bEnable = FALSE;
    EQFEnableAll(bEnable);
  } /* endif */
  if ((HIWORD(lOldParam) == HAS_NOFONTSPEC) &&
      (HIWORD(lNewParam) == HAS_FONTSPEC) )
  {
    //enable the dialog controls except the tree
    bEnable = TRUE;
    EQFEnableAll(bEnable);
  } /* endif */
}

void CFontExtDlg::EQFEnableAll(BOOL bEnable)
{
  CButton* pUnderline = (CButton*) GetDlgItem( ID_TB_FONTEXT_UNDERLINE );
  CButton* pStrikeOut = (CButton*) GetDlgItem( ID_TB_FONTEXT_STRIKETHROUGH );
  CComboBox* pSizeCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_SIZE);
  CComboBox* pFontStyleCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_FONTSTYLE_CBS);
  CComboBox* pFGColor = (CComboBox* ) GetDlgItem( ID_TB_FONTEXT_FGCOLOR );
  CComboBox* pBGColor = (CComboBox* ) GetDlgItem( ID_TB_FONTEXT_BGCOLOR );
  CComboBox* pScript  = (CComboBox* ) GetDlgItem( ID_TB_FONTEXT_SCRIPT  );

    m_FontCBS.EnableWindow(bEnable);
    pUnderline->EnableWindow(bEnable);
    pStrikeOut->EnableWindow(bEnable);
    pSizeCBS->EnableWindow(bEnable);
    pFontStyleCBS->EnableWindow(bEnable);
    pBGColor->EnableWindow(bEnable);
    pFGColor->EnableWindow(bEnable);
    pScript ->EnableWindow(bEnable);
}

void CFontExtDlg::EQFInsertComboBoxes()
{
   int   iIndex = 0;
   CFont fnt;

   fnt.Attach(GetStockObject(DEFAULT_GUI_FONT));
   CDC * pDC = GetDC();
   CFont*   pFont = pDC->SelectObject(&fnt);
   TEXTMETRIC tm;
   pDC->GetTextMetrics(&tm);

   pDC->SelectObject(pFont);
   ReleaseDC(pDC);
   m_FontCBS.m_nFontHeight = tm.tmHeight;

  /********************************************************************/
  /* fill fonts combobox                                              */
  /********************************************************************/
  CFontComboBox* pFontCBS=(CFontComboBox*) GetDlgItem( ID_TB_FONTEXT_FONT_CBS);

  pFontCBS->SetHwnd(GetHwnd());
  pFontCBS->LimitText(LF_FACESIZE);

  HDC hdc = ::GetDC( GetHwnd() );
  pFontCBS->EnumFontFamiliesEx(hdc);
  ::ReleaseDC(GetHwnd(), hdc );
  int i = pFontCBS->SetCurSel(0);
  /********************************************************************/
  /* fill font size combo box                                         */
  /********************************************************************/
  EQFFillAndSelSize(pFontCBS);
  CComboBox* pSizeCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_SIZE);
  pSizeCBS->SetCurSel(0);

  i = pFontCBS->GetCurSel();
  CFontDesc* pDesc = (CFontDesc*)pFontCBS->GetItemData(i);
  EQFFillAndSelScript(pFontCBS, pDesc->m_nCharSet);
  /********************************************************************/
  /* fill fontstyle combobox                                          */
  /********************************************************************/
  CComboBox* pStyleCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_FONTSTYLE_CBS);
  ASSERT(pStyleCBS != NULL);
  CHAR       chStrBuf[50];

  for (i = 0; i < 4; i++)
  {
       LOADSTRING(NULLHANDLE, AfxGetResourceHandle(),
                    IDS_TB_FONTEXT_STANDARD + i, chStrBuf);
       pStyleCBS->AddString(chStrBuf);
  }
  i = pStyleCBS->SetCurSel(0);
  /********************************************************************/
  /* fill background and foreground colors comboboxes                 */
  /********************************************************************/
  CComboBox* pBGColorCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_BGCOLOR);
  CComboBox* pFGColorCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_FGCOLOR);
  ASSERT(pBGColorCBS != NULL);
  ASSERT(pFGColorCBS != NULL);
  COLORREF *  pColorRGB = EQFBGetColorRGB();
  for (i = 0; i < MAXVIOCOLOR; i++)
  {
       LOADSTRING(NULLHANDLE, AfxGetResourceHandle(),
                    IDS_TB_COL_BLACK + i, chStrBuf);
       iIndex = pBGColorCBS->AddString(chStrBuf);
//       ASSERT(iIndex >=0);
       if (iIndex >=0) //kein Fehler
          pBGColorCBS->SetItemData(iIndex, (DWORD) pColorRGB[i]);
       iIndex = pFGColorCBS->AddString(chStrBuf);
//     ASSERT(iIndex >=0);
       if (iIndex >=0) //kein Fehler
          pFGColorCBS->SetItemData(iIndex, (DWORD) pColorRGB[i]);
  }
  i = pBGColorCBS->SetCurSel(0);
  i = pFGColorCBS->SetCurSel(0);
 }

void CFontExtDlg:: EQFFillAndSelSize(CFontComboBox* pFontCBS)
{
  CString str;
  LPCTSTR lpszName = NULL;
  BOOL bPrinterFont = FALSE;

  pFontCBS->GetTheText(str);
  int nIndex = pFontCBS->FindStringExact(-1, str);
  if (nIndex != CB_ERR)
  {
     CFontDesc* pDesc = (CFontDesc*)pFontCBS->GetItemData(nIndex);
     ASSERT(pDesc != NULL);
     bPrinterFont = pDesc->m_dwFlags & PRINTER_FONT;
     lpszName = pDesc->m_strName;
  }

  CComboBox* pSizeCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_SIZE);
  int nSize = GetTwipSize(pSizeCBS);
  if (nSize == -2) // Error
  {
       AfxMessageBox("nSIze is minus 2", MB_OK|MB_ICONINFORMATION);
          nSize = m_nTwipsLast;
  }
  else if ((nSize >= 0 && nSize < 20) || nSize > 32760)
  {
       AfxMessageBox("nSize is between 0 and 20", MB_OK|MB_ICONINFORMATION);
          nSize = m_nTwipsLast;
  }

  CDC * pDC = GetDC();

  if (bPrinterFont)
          EnumFontSizes(*pDC, lpszName, pSizeCBS);
  else
          EnumFontSizes(*pDC, lpszName, pSizeCBS);
  ReleaseDC(pDC);

  if (nSize == -1 )     // no text in control selected
  {
    nSize = pSizeCBS->SetCurSel(0);
  } /* endif */
  CFontExtDlg::SetTwipSize(nSize, pSizeCBS);
}

void CFontExtDlg:: EQFFillAndSelScript
(
   CFontComboBox* pFontCBS,
   BYTE nCharSet
)
{
  CString str;
  CFontDesc* pNextDesc = NULL;
  CFontDesc* pDesc = NULL;
  int  iIndex = 0;
  int  iIndexToSelect = 0;

  pFontCBS->GetTheText(str);
  int nIndex = pFontCBS->FindStringExact(-1, str);
  if (nIndex != CB_ERR)
  {
     pDesc = (CFontDesc*)pFontCBS->GetItemData(nIndex);
     ASSERT(pDesc != NULL);
  }
  CComboBox* pScriptCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_SCRIPT);
  if (pScriptCBS->GetCount() != 0)
  {
    pScriptCBS->ResetContent();
  }

  if ( pDesc && !pDesc->m_strScript.IsEmpty())
  {
    iIndex = pScriptCBS->AddString(pDesc->m_strScript);
    if (pDesc->m_nCharSet == nCharSet)
    {
      iIndexToSelect = iIndex;
    } /* endif */
  }
  if (pDesc )
  {
    pNextDesc = pDesc->GetNext();
    while (pNextDesc )
    {
      if ( !pNextDesc->m_strScript.IsEmpty())
      {
        iIndex = pScriptCBS->AddString(pNextDesc->m_strScript);
        if (pNextDesc->m_nCharSet == nCharSet)
        {
          iIndexToSelect = iIndex;
        } /* endif */
      }
      pNextDesc = pNextDesc->GetNext();
    } /* endwhile */
  } /* endif */

  pScriptCBS->SetCurSel(iIndexToSelect);
}


void CFontExtDlg::OnCancel()
{
  CDialog::OnCancel();
}

void CFontExtDlg::OnOK()
{
  CTreeCtrl* pTree = ((CTreeCtrl *)GetDlgItem( ID_TB_FONTEXT_TREE));
  HTREEITEM hItem = pTree->GetSelectedItem();
  LONG lParam = pTree->GetItemData(hItem);
  if (HIWORD(lParam) == HAS_FONTSPEC )
  {
    int i = LOWORD(lParam);
    //retrieve all font settings of the old tree item
    GetAllSelects(i);
  } /* endif */
  SaveAllSpecs();
  CDialog::OnOK();
}

void CFontExtDlg::OnHelp()
{
  UtlInvokeHelp();
}

void CFontExtDlg::OnDefault()
{
  int i;
  EXTCOLOREX * pDef1Color;
  BYTE bCharSet;
  BYTE bUsedCharSet;
  BYTE bPitchAndFamily;
  CHAR  chFaceName[LF_FACESIZE];
  CHAR  chSearchFaceName[LF_FACESIZE];
  int nIndex;
  LONG  lHeight;

  /********************************************************************/
  /* Select default font                                              */
  /********************************************************************/
  bUsedCharSet = (BYTE) GetCharSet();

  if (bUsedCharSet == THAI_CHARSET )
  {
    strcpy( chSearchFaceName, "Tahoma" );
  }
  else
  {
    strcpy(chSearchFaceName, "Arial");
  }

  nIndex = m_FontCBS.FindStringExact(-1, chSearchFaceName);
  if (nIndex != CB_ERR)
  {
    CFontDesc* pDesc = (CFontDesc*)m_FontCBS.GetItemData(nIndex);
    ASSERT(pDesc != NULL);
    lstrcpyn(chFaceName, (LPCTSTR)pDesc->m_strName, LF_FACESIZE);
    bCharSet = pDesc->m_nCharSet;
    bPitchAndFamily = pDesc->m_nPitchAndFamily;

    // now find default fitting to CHARSET
    while ( pDesc )
    {
      if ( bUsedCharSet == pDesc->m_nCharSet )
      {
        bCharSet = pDesc->m_nCharSet;
        bPitchAndFamily = pDesc->m_nPitchAndFamily;
        ASSERT(pDesc->m_strName.GetLength() < LF_FACESIZE);
        lstrcpyn(chFaceName, (LPCTSTR) pDesc->m_strName, LF_FACESIZE);
        break;     // leave loop
      }
      else
      {
        pDesc = pDesc->pNext;
      } /* endif */
    } /* endwhile */

  }
  else
  {
    /******************************************************************/
    /* This is the default if our selection is not available          */
    /******************************************************************/
     strcpy(chFaceName, "Arial");
     bCharSet = DEFAULT_CHARSET;
     bPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  }




  COLORREF *  pColorRGB = EQFBGetColorRGB();
  lHeight = 200L;
  if (bCharSet == THAI_CHARSET )
  {
    lHeight = 240L;
  }
  for (i = 0; i < MAXEXTTYPE; i++)
  {
    memset(&m_CharFormat[i], 0,sizeof(CHARFORMAT2));
    pDef1Color = &aszDef1FontExtSpecs[i];
    EQFBSetCharFormat2 (&m_CharFormat[i],
                         &chFaceName[0],
                         bCharSet,
                         bPitchAndFamily,
                         lHeight,
                         pDef1Color->bStyle,
                         pColorRGB[ pDef1Color->usTextColor],
                         pColorRGB[ pDef1Color->usBackColor],
                         pDef1Color->bUnderline,
                         pDef1Color->bStrikeOut );
// m_FontCBS.m_nFontHeight = tm.tmHeight; // d*20??
  }
  CTreeCtrl* pTree = ((CTreeCtrl *)GetDlgItem( ID_TB_FONTEXT_TREE));
  pTree->SelectItem(hTreeItem1);
  EQFInitAllSpecs(0);

  FillPreview( PREVIEW_TRANSLATION );

}

void CFontExtDlg::OnFontComboCloseUp()
{
  int iCurIndex;
  /********************************************************************/
  /* update fontsize cbs selection                                    */
  /********************************************************************/
  CFontComboBox* pFontCBS=(CFontComboBox*) GetDlgItem( ID_TB_FONTEXT_FONT_CBS);
  EQFFillAndSelSize(pFontCBS);
  CComboBox* pSizeCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_SIZE);
  pSizeCBS->SetCurSel(0);

  iCurIndex = pFontCBS->GetCurSel();
  if (iCurIndex > -1)
  {
    CFontDesc* pDesc = (CFontDesc*)pFontCBS->GetItemData(iCurIndex);
    EQFFillAndSelScript(pFontCBS, pDesc->m_nCharSet);
    OnUpdateRequest();
  }
}

void CFontExtDlg::OnSizeComboCloseUp()
{
  /********************************************************************/
  /* nothing to do??                                                  */
  /********************************************************************/
}


void CFontExtDlg::GetAllSelects(int iTreeIndex)
{
   int iBGIndex = 0;
   int iFGIndex = 0;
   int iCurIndex = 0;
   int iStyleIndex = 0;
   int iStrikeOut = 0;
   int iScript = 0;
   int iUnderline = UNDERLINE_NOSPEC;
   CHAR  chFaceName[LF_FACESIZE];
   CString str;
   BOOL    fFound = FALSE;

   CFontComboBox* pFontCBS=(CFontComboBox*) GetDlgItem( ID_TB_FONTEXT_FONT_CBS);
   iCurIndex = pFontCBS->GetCurSel();
   if ( iCurIndex >= 0)
   {
     CFontDesc* pDesc = (CFontDesc*)pFontCBS->GetItemData(iCurIndex);
     ASSERT(pDesc != NULL);
     ASSERT(pDesc->m_strName.GetLength() < LF_FACESIZE);
     lstrcpyn(chFaceName, (LPCTSTR)pDesc->m_strName, LF_FACESIZE);

     if ( pDesc && !pDesc->m_strScript.IsEmpty())
     {
       CComboBox* pScriptCBS = (CComboBox*) GetDlgItem( ID_TB_FONTEXT_SCRIPT);
       iScript = pScriptCBS->GetCurSel();
       pScriptCBS->GetLBText(iScript, str);
       do
       {
         if (lstrcmp(str, pDesc->m_strScript) == 0 )
         {
           fFound = TRUE;                  //correct pDesc found!
         }
         else
         {
           pDesc = pDesc->GetNext();
         } /* endif */
       } while (!fFound && pDesc); /* enddo */
     }
     CComboBox* pSizeCBS = (CComboBox*) GetDlgItem( ID_TB_FONTEXT_SIZE);

     CComboBox* pStyleCBS = (CComboBox*) GetDlgItem( ID_TB_FONTEXT_FONTSTYLE_CBS);
     ASSERT(pStyleCBS != NULL);
     iStyleIndex = pStyleCBS->GetCurSel();
     if (iStyleIndex == CB_ERR)
     { //what should we do?
       iStyleIndex = 0;
     }
     CComboBox* pBGColorCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_BGCOLOR);
     CComboBox* pFGColorCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_FGCOLOR);
     iFGIndex = pFGColorCBS->GetCurSel();
     if (iFGIndex == CB_ERR)
     { //what should we do?
       iFGIndex = 0;
     }
     iBGIndex = pBGColorCBS->GetCurSel();
     if (iBGIndex == CB_ERR)
     { //what should we do?
       iBGIndex = 0;
     }

     // Underline and strikeout
     CButton* pUnderline = (CButton*) GetDlgItem( ID_TB_FONTEXT_UNDERLINE );
     CButton* pStrikeOut = (CButton*) GetDlgItem( ID_TB_FONTEXT_STRIKETHROUGH );
     if ( pStrikeOut->GetCheck() == 1)
     {
        iStrikeOut = STRIKEOUT;
     } /* endif */

     if ( pUnderline->GetCheck() == 0)
     {
        iUnderline = UNDERLINE_NONE;
     }
     else
     {
       iUnderline = UNDERLINE_YES;
     } /* endif */
     EQFBSetCharFormat2(&m_CharFormat[iTreeIndex],
                       chFaceName,
                       pDesc->m_nCharSet, pDesc->m_nPitchAndFamily,
                       GetTwipSize(pSizeCBS),
                       (SHORT)iStyleIndex,
                       (COLORREF)pFGColorCBS->GetItemData(iFGIndex),
                       (COLORREF)pBGColorCBS->GetItemData(iBGIndex),
                       (SHORT)iUnderline,
                       (SHORT)iStrikeOut);
  }
}

void CFontExtDlg::SaveAllSpecs()
{
   /********************************************************************/
   /* copy specs for all tree items to editor structure                */
   /********************************************************************/
   int i;
   CHARFORMAT2 * pDlgCharFormat;
   CHARFORMAT2 * pEditCharFormat;
   pEditCharFormat = (CHARFORMAT2 *)EQFBGetFontSpecs();
   for (i = 0; i < MAXEXTTYPE; i++)
   {
     pDlgCharFormat = &m_CharFormat[i];
     memcpy(pEditCharFormat, pDlgCharFormat, sizeof(CHARFORMAT2));
     pEditCharFormat ++;
   }
}

void CFontExtDlg::EQFInitAllSpecs(int iIndex)
{
  int i;
  CHARFORMAT2 * pDlgCharFormat;
  CComboBox* pCBS;

  /********************************************************************/
  /* set selections in cbs according to selected tree item ( usIndex) */
  /********************************************************************/
  pDlgCharFormat = &m_CharFormat[iIndex];

  CFontComboBox* pFontCBS = (CFontComboBox*) GetDlgItem( ID_TB_FONTEXT_FONT_CBS);
  if ((pDlgCharFormat->dwMask & CFM_FACE) &&
      (pDlgCharFormat->dwMask & CFM_CHARSET)  )
  {
    pFontCBS->MatchFont(pDlgCharFormat->szFaceName, pDlgCharFormat->bCharSet);
  } /* endif */

  EQFFillAndSelScript(pFontCBS, pDlgCharFormat->bCharSet);
  EQFFillAndSelSize(pFontCBS);
  CComboBox* pSizeCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_SIZE);
  ASSERT(pSizeCBS != NULL);
  pSizeCBS->SetCurSel(0);

  if (pDlgCharFormat->dwMask & CFM_SIZE )
  {
     SetTwipSize(pDlgCharFormat->yHeight, pSizeCBS);
  } /* endif */

  pCBS = (CComboBox*) GetDlgItem( ID_TB_FONTEXT_FONTSTYLE_CBS);
  if (pDlgCharFormat->dwMask & CFM_ITALIC &&
      pDlgCharFormat->dwEffects & CFE_ITALIC )
  {
    if (pDlgCharFormat->dwMask & CFM_BOLD &&
        pDlgCharFormat->dwEffects & CFE_BOLD )
    {
      //bold and italics
      pCBS->SetCurSel(3);  // sollte itemdata benutzt werden?
    }
    else
    {
      pCBS->SetCurSel(1);
    } /* endif */
  }
  else
  {
    if (pDlgCharFormat->dwMask & CFM_BOLD &&
        pDlgCharFormat->dwEffects & CFE_BOLD )
    {
      //bold
      pCBS->SetCurSel(2);  // sollte itemdata benutzt werden?
    }
    else
    {
      pCBS->SetCurSel(0);  //standard
    } /* endif */

  } /* endif */

  pCBS = (CComboBox*) GetDlgItem( ID_TB_FONTEXT_BGCOLOR);
  BOOL fFound = FALSE;
  i = 0;
  if (pDlgCharFormat->dwMask & CFM_BACKCOLOR )
  {
    while (!fFound && (i < MAXVIOCOLOR))
    {
      if (pDlgCharFormat->crBackColor == (COLORREF)pCBS->GetItemData(i) )
      {
        fFound = TRUE;
        pCBS->SetCurSel(i);
      } /* endif */
      i++;
    } /* endwhile */
  } /* endif */
  pCBS = (CComboBox*) GetDlgItem( ID_TB_FONTEXT_FGCOLOR);
  fFound = FALSE;
  i = 0;
  if (pDlgCharFormat->dwMask & CFM_COLOR )
  {
    while (!fFound && (i < MAXVIOCOLOR))
    {
      if (pDlgCharFormat->crTextColor == (COLORREF)pCBS->GetItemData(i) )
      {
        fFound = TRUE;
        pCBS->SetCurSel(i);
      } /* endif */
      i++;
    } /* endwhile */
  } /* endif */
  CButton * pStrikeOut = (CButton*) GetDlgItem( ID_TB_FONTEXT_STRIKETHROUGH );

  if ((pDlgCharFormat->dwMask & CFM_STRIKEOUT) &&
      (pDlgCharFormat->dwEffects & CFE_STRIKEOUT)  )
  {
    pStrikeOut->SetCheck(1);
  }
  else
  {
    pStrikeOut->SetCheck(0);
  } /* endif */
  CButton * pUnderline = (CButton*) GetDlgItem( ID_TB_FONTEXT_UNDERLINE );

  if ((pDlgCharFormat->dwMask & CFM_UNDERLINETYPE) &&
      (pDlgCharFormat->dwEffects & CFE_UNDERLINE) &&
       (pDlgCharFormat->bUnderlineType == CFU_UNDERLINE)  )
  {
    pUnderline->SetCheck(1);
  }
  else
  {
    pUnderline->SetCheck(0);
  } /* endif */

}

void CFontExtDlg::FillPreview( int previewOpt)
{
  HWND hwndRichEdit = ((CWnd *)GetDlgItem( ID_TB_FONTEXT_TEXT ))->m_hWnd;
  CHARFORMAT2 CharFormat;
  CHAR        chBuffer[ 256 ];
  CHAR_W      chBufferW[ 256 ];
  TBSEGMENT   tbSeg;
  CHARRANGE   chRange;
  int         i = 0;

  CHARFORMAT2 *pszFontExtSpecs;


  pszFontExtSpecs = (CHARFORMAT2 *)malloc( sizeof( CHARFORMAT2 ) * MAXEXTTYPE);

  memcpy( pszFontExtSpecs, EQFBGetFontSpecs(), sizeof( CHARFORMAT2 ) * MAXEXTTYPE);
  SaveAllSpecs();

  chRange.cpMin = 0; chRange.cpMax = -1;
  ::SendMessage( hwndRichEdit, EM_EXSETSEL, 0L, (LONG)&chRange );
  ::SendMessage( hwndRichEdit, EM_REPLACESEL, FALSE, MP2FROMP( "" ) );

  tbSeg.pDataW = &chBufferW[0];

  switch ( previewOpt )
  {
    case PREVIEW_TRANSLATION:

      while ( PreviewTrans[i].chString[0] )
      {
        tbSeg.qStatus = PreviewTrans[i].usSegStatus;
        memset( &CharFormat, 0, sizeof( CharFormat ));
        CharFormat.cbSize = sizeof(CharFormat);
        EQFBSegColRTF( &CharFormat, &tbSeg, PreviewTrans[i].usDispType, 0,
                       DISP_PROTECTED, FALSE, NULL );
        ::SendMessage( hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP( &CharFormat ));
        EQFOemToAnsi( PreviewTrans[i].chString, chBuffer );
        ::SendMessage( hwndRichEdit, EM_REPLACESEL, FALSE, MP2FROMP( chBuffer ) );
        ASCII2Unicode( chBuffer, &chBufferW[0], 0L );
        i++;
      } /* endwhile */
      break;
    case PREVIEW_POSTEDIT:

      while ( PreviewPostEdit[i].chString[0] )
      {
        tbSeg.qStatus = PreviewPostEdit[i].usSegStatus;
        switch ( i )
        {
          case 0:
          case 2:
          case 4:
            tbSeg.SegFlags.Typed = TRUE;
            tbSeg.SegFlags.Copied = FALSE;
            break;
          case 6:
          case 8:
          case 10:
            tbSeg.SegFlags.Typed = FALSE;
            tbSeg.SegFlags.Copied = TRUE;
            break;
          case 12:
            tbSeg.SegFlags.Typed = TRUE;
            tbSeg.SegFlags.Copied = TRUE;
            break;
        } /* endswitch */
        memset( &CharFormat, 0, sizeof( CharFormat ));
        CharFormat.cbSize = sizeof(CharFormat);
        EQFBSegColRTF( &CharFormat, &tbSeg, PreviewPostEdit[i].usDispType,
                       PreviewPostEdit[i].usHLState,
                       DISP_PROTECTED, TRUE, NULL );
        ::SendMessage( hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP( &CharFormat ));
        EQFOemToAnsi( PreviewPostEdit[i].chString, chBuffer );
        ::SendMessage( hwndRichEdit, EM_REPLACESEL, FALSE, MP2FROMP( chBuffer ) );
        ASCII2Unicode( chBuffer, &chBufferW[0], 0L );
        i++;
      } /* endwhile */
      break;
    case PREVIEW_DICT:
      while ( PreviewDict[i].chString[0] )
      {
        tbSeg.qStatus = PreviewDict[i].usSegStatus;
        memset( &CharFormat, 0, sizeof( CharFormat ));
        CharFormat.cbSize = sizeof(CharFormat);
        EQFBSegColRTF( &CharFormat, &tbSeg, PreviewDict[i].usDispType, 0,
                       DISP_PROTECTED, FALSE, NULL );
        ::SendMessage( hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP( &CharFormat ));
        EQFOemToAnsi( PreviewDict[i].chString, chBuffer );
        ::SendMessage( hwndRichEdit, EM_REPLACESEL, FALSE, MP2FROMP( chBuffer ) );
        ASCII2Unicode( chBuffer, &chBufferW[0], 0L );
        i++;
      } /* endwhile */
      break;
    case PREVIEW_PROPOSAL:
      while ( PreviewProposal[i].chString[0] )
      {
        tbSeg.qStatus = PreviewProposal[i].usSegStatus;
        memset( &CharFormat, 0, sizeof( CharFormat ));
        CharFormat.cbSize = sizeof(CharFormat);
        EQFBSegColRTF( &CharFormat, &tbSeg, PreviewProposal[i].usDispType, 0,
                        DISP_PROTECTED, FALSE, NULL );
        ::SendMessage( hwndRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, MP2FROMP( &CharFormat ));
        EQFOemToAnsi( PreviewProposal[i].chString, chBuffer );
        ::SendMessage( hwndRichEdit, EM_REPLACESEL, FALSE, MP2FROMP( chBuffer ) );
        ASCII2Unicode( chBuffer, &chBufferW[0], 0L );
        i++;
      } /* endwhile */
      break;
    default:
      break;
  } /* endswitch */


  /********************************************************************/
  /* free allocated resources                                         */
  /********************************************************************/
  memcpy( EQFBGetFontSpecs(), pszFontExtSpecs, sizeof( CHARFORMAT2 ) * MAXEXTTYPE);
  free( pszFontExtSpecs );
}



void CFontExtDlg::OnComboSetFocus()
{
//        NotifyOwner(NM_SETFOCUS);
}

void CFontExtDlg::OnUpdateRequest()
{
  GetAllSelects( actSel );
  if ((0 <= actSel) && (actSel < MAXEXTTYPE) )
  {
    EQFInitAllSpecs(actSel);
    switch ( actSel )
    {
      case EXT_T_SOURCE:
      case EXT_T_TRANSLATION:
      case EXT_T_LAYOUT:
      case EXT_T_ACTIVE:
      case EXT_T_TRNOTE:
      case EXT_T_RTFEDIT_TRNOTE:
        FillPreview( PREVIEW_TRANSLATION);
        break;
      case EXT_R_SCRATCH:
      case EXT_R_MODIFIED:
      case EXT_R_COPIED:
      case EXT_R_LAYOUTCHANGES:
        FillPreview( PREVIEW_POSTEDIT);
        break;
      case EXT_D_HEADWORD:
      case EXT_D_TRANSLATION:
      case EXT_D_PREFIX:
      case EXT_D_ADDITIONINFO:
        FillPreview( PREVIEW_DICT );
        break;
      case EXT_M_PREFIX:
      case EXT_M_PMODIFIED:
      case EXT_M_PINSERTED:
      case EXT_M_PDELETED:
        FillPreview( PREVIEW_PROPOSAL );
        break;

    } /* endswitch */

  } /* endif */
}

void CFontExtDlg::OnFontNameKillFocus()
{
//        USES_CONVERSION;
        // Besorgen des aktuellen Fonts von der Ansicht und Update
  //      NotifyOwner(NM_KILLFOCUS);

        CCharFormat cf;
        cf.szFaceName[0] = NULL;

        // Dies liefert den Font, der im Edit Control eingegeben wird.
        // Es wird versucht Übereinstimmung zwischen dem gewünschten Font und
        // einem bereits in der ComboBox vorhandenen zu erreichen.
        // Dabei wird Gross-/Kleinschreibung ignoriert, falls ein Benutzer "arial" eingibt
        // wird daraus "Arial"
        CString str;
        m_FontCBS.GetTheText(str);        // liefert "arial"
        m_FontCBS.SetTheText(str);        // selektiert "Arial"
        m_FontCBS.GetTheText(str);        // liefert "Arial"

        // falls Font Namen Box nicht leer ist
        if (str[0] != NULL)
        {
           cf.dwMask = CFM_FACE | CFM_CHARSET;
           int nIndex = m_FontCBS.FindStringExact(-1, str);
           if (nIndex != CB_ERR)
           {
              CFontDesc* pDesc = (CFontDesc*)m_FontCBS.GetItemData(nIndex);
              ASSERT(pDesc != NULL);
              ASSERT(pDesc->m_strName.GetLength() < LF_FACESIZE);
              lstrcpyn(cf.szFaceName, pDesc->m_strName, LF_FACESIZE);
//              lstrcpynA(cf.szFaceName,
//                      T2A((LPTSTR) (LPCTSTR) pDesc->m_strName), LF_FACESIZE);
              cf.bCharSet = pDesc->m_nCharSet;
              cf.bPitchAndFamily = pDesc->m_nPitchAndFamily;
           }
           else // unbekannter Font
           {
              ASSERT(str.GetLength() < LF_FACESIZE);
              lstrcpyn(cf.szFaceName, str, LF_FACESIZE);
//              lstrcpynA(cf.szFaceName,
//                      T2A((LPTSTR) (LPCTSTR) str), LF_FACESIZE);
              cf.bCharSet = DEFAULT_CHARSET;
              cf.bPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
           }
//           SetCharFormat(cf);
        }
}

void CFontExtDlg::EnumFontSizes(CDC& dc, LPCTSTR pFontName, CComboBox * pCBS)
{
   pCBS->ResetContent();
   if (pFontName == NULL)
           return;
   if (pFontName[0] == NULL)
           return;

   ASSERT(dc.m_hDC != NULL);
   m_nLogVert = dc.GetDeviceCaps(LOGPIXELSY);

   ::EnumFontFamilies(dc.m_hDC, pFontName,
           (FONTENUMPROC) EnumSizeCallBack, (LPARAM) this);
}

void CFontExtDlg::TwipsToPointString(LPTSTR lpszBuf, int nTwips)
{
   ASSERT(lpszBuf != NULL);
   lpszBuf[0] = NULL;
   if (nTwips >= 0)
   {
      // Rundung auf n-chsten halben Punkt
      nTwips = (nTwips+5)/10;
      if ((nTwips%2) == 0)
              _stprintf(lpszBuf, _T("%ld"), nTwips/2);
      else
              _stprintf(lpszBuf, _T("%.1f"), (float)nTwips/2.F);
   }
}

void CFontExtDlg::SetTwipSize(int nTwips, CComboBox* pCBS)
{
   if (nTwips != GetTwipSize(pCBS))
   {
      TCHAR buf[20];
      TwipsToPointString(buf, nTwips);
      int idx = pCBS->FindStringExact(-1,buf);
      pCBS->SetCurSel( (idx==CB_ERR) ? -1 : idx);
      if (idx == CB_ERR)
              SetWindowText(buf);
   }
   m_nTwipsLast = nTwips;
}

int CFontExtDlg::GetTwipSize(CComboBox* pCBS)
{
    // Rckgabewerte
    // -2 -- Fehler
    // -1 -- Edit Box leer
    // >=0 -- Font Grösse in Twips
    CString str;
    int nIndex = pCBS->GetCurSel();
    if (nIndex != CB_ERR)
    {
      pCBS->GetLBText(nIndex, str);
    }

    LPCTSTR lpszText = str;

    while (*lpszText == ' ' || *lpszText == '\t')
            lpszText++;

    if (lpszText[0] == NULL)
            return -1; // kein Text im Control

    double d = _tcstod(lpszText, (LPTSTR*)&lpszText);
    while (*lpszText == ' ' || *lpszText == '\t')
            lpszText++;

    if (*lpszText != NULL)
            return -2;   // nicht richtig terminiert

    return (d<0.) ? 0 : (int)(d*20.);
}

BOOL CALLBACK AFX_EXPORT CFontExtDlg::EnumSizeCallBack(LOGFONT FAR* /*lplf*/,
                LPNEWTEXTMETRIC lpntm, int FontType, LPVOID lpv)
  {
     CFontExtDlg* pThis = (CFontExtDlg*)lpv;
     ASSERT(pThis != NULL);
     CComboBox* pSizeCBS=(CComboBox*) pThis->GetDlgItem( ID_TB_FONTEXT_SIZE);
     TCHAR buf[10];
     if ( (FontType & TRUETYPE_FONTTYPE) ||
             !( (FontType & TRUETYPE_FONTTYPE) || (FontType & RASTER_FONTTYPE) )
             ) // falls Truetype oder Vektorfont
     {
        // dies passiert, wenn eine Truetype- und eine Non-Truetype-Version eines Fonts exisitiert
        if (pSizeCBS->GetCount() != 0)
                pSizeCBS->ResetContent();

        for (int i = 0; i < 16; i++)
        {
                wsprintf(buf, _T("%d"), nFontSizes[i]);
                pSizeCBS->AddString(buf);
        }
        return FALSE; // kein erneuter Aufruf
     }
     // berechne Character-Höhe in Pixels
     pThis->InsertSize(MulDiv(lpntm->tmHeight-lpntm->tmInternalLeading,
             1440, pThis->m_nLogVert));
     return TRUE; // erneuter Aufruf
  }

void CFontExtDlg::InsertSize(int nSize)
{
   ASSERT(nSize > 0);
   DWORD dwSize = (DWORD)nSize;
   TCHAR buf[10];

   TwipsToPointString(buf, nSize);
   CComboBox* pSizeCBS=(CComboBox*) GetDlgItem( ID_TB_FONTEXT_SIZE);
   ASSERT(pSizeCBS != NULL);
   if (pSizeCBS->FindStringExact(-1, buf) == CB_ERR)
   {
      int nIndex = -1;
      int nPos = 0;
      DWORD dw;
      while ((dw = pSizeCBS->GetItemData(nPos)) != CB_ERR)
      {
         if (dw > dwSize)
         {
            nIndex = nPos;
            break;
         }
         nPos++;
      }
      nIndex = pSizeCBS->InsertString(nIndex, buf);
      ASSERT(nIndex != CB_ERR);
      if (nIndex != CB_ERR)
              pSizeCBS->SetItemData(nIndex, dwSize);
   }
}

/////////////////////////////////////////////////////////////////////////////
// CFontComboBox

BEGIN_MESSAGE_MAP(CFontComboBox, CComboBox)
        //{{AFX_MSG_MAP(CFontComboBox)
        ON_WM_DESTROY()
        ON_WM_CREATE()
        //}}AFX_MSG_MAP
        // Globale Hilfsbefehle
END_MESSAGE_MAP()

void CFontComboBox::GetTheText(CString& str)
{
        int nIndex = GetCurSel();
        if (nIndex == CB_ERR)
                GetWindowText(str);
        else
                GetLBText(nIndex, str);
}

void CFontComboBox::SetTheText(LPCTSTR lpszText,BOOL bMatchExact)
{
        int idx = (bMatchExact) ? FindStringExact(-1,lpszText) :
                FindString(-1, lpszText);
        SetCurSel( (idx==CB_ERR) ? -1 : idx);
        if (idx == CB_ERR)
                SetWindowText(lpszText);
}

BOOL CFontComboBox::LimitText(int nMaxChars)
{
        BOOL b = CComboBox::LimitText(nMaxChars);
        if (b)
                m_nLimitText = nMaxChars;
        return b;
}

int CFontComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if (CComboBox::OnCreate(lpCreateStruct) == -1)
           return -1;
   SendMessage( WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FONT));
   return 0;
}



CFontComboBox::CFontComboBox()
{
   VERIFY(m_bmFontType.LoadBitmap(ID_TB_FONTEXT_FONTTYPE));
}

CFontComboBox::CFontComboBox(HWND hwnd)
{
   hwndParent = hwnd;
}

void CFontComboBox::OnDestroy()
{
   // zerst”re alle CFontDesc's
   EmptyContents();
   CComboBox::OnDestroy();
}

void CFontComboBox::EmptyContents()
{
   // zerst”re alle CFontDesc's
   int nCount = GetCount();
   for (int i=0;i<nCount;i++)
           delete (CFontDesc*)GetItemData(i);
}

void CFontComboBox::EnumFontFamiliesEx(HDC hDC, BYTE nCharSet)
{
   CMapStringToPtr map;
   CString str;
   GetTheText(str);

   EmptyContents();
   ResetContent();
   LOGFONT lf;
   memset(&lf, 0, sizeof(LOGFONT));
   lf.lfCharSet = nCharSet;

   if (hDC != NULL)
   {
     ::EnumFontFamiliesEx(hDC, &lf,
             (FONTENUMPROC) EnumFamPrinterCallBackEx, (LPARAM) this, NULL);
   }
   else
   {
     HDC hDC1 = ::GetDC(hwndParent);
     ASSERT(hDC1 != NULL);
     ::EnumFontFamiliesEx(hDC1, &lf,
             (FONTENUMPROC) EnumFamScreenCallBackEx,
             (LPARAM) this, NULL);
   }
   // nun durch die Fonts hangeln und entfernen der Charsets von Fonts mit nur einem

   int nCount = m_arrayFontDesc.GetSize();
   // durch Fonts hangeln und Namen in Stringmap hinzufügen
   // beim ersten Mal füge Wert 0 ein, danach Wert 1
   int j;
   for (int i = 0; i<nCount;i++)
   {
      CFontDesc* pDesc = (CFontDesc*)m_arrayFontDesc[i];
      CFontDesc * pLastDesc;
      void* pv = NULL;
      if (map.Lookup(pDesc->m_strName, pv)) // gefunden
      {
         pDesc->m_nFirst = NOT_FIRST;
         pDesc->SetNext(NULL);

         j = int(pv);
         pLastDesc = (CFontDesc* )m_arrayFontDesc[j];
         pLastDesc->SetNext(pDesc);  // hang at previous

         map.RemoveKey(pDesc->m_strName);
         map.SetAt(pDesc->m_strName, (void*)i);  // index of last in list

      }
      else                             // nicht gefunden
      {
         map.SetAt(pDesc->m_strName, (void*)i);
         pDesc->SetNext(NULL);
         pDesc->m_nFirst = FIRST;
      }
   }

   for ( int i = 0; i<nCount;i++)
   {
      CFontDesc* pDesc = (CFontDesc*)m_arrayFontDesc[i];
      CString str = pDesc->m_strName;
      void* pv = NULL;
      VERIFY(map.Lookup(str, pv));
//    if (pv != NULL && !pDesc->m_strScript.IsEmpty())
//    {
//            str += " (";
//            str += pDesc->m_strScript;    //(Arabisch) or (Western)...
//            str += ")";
//            pScript->AddString(chStrBuf);
//    }
      if (pDesc->m_nFirst == FIRST )       // first font desc of array
      {
        int nIndex = AddString(str);
        ASSERT(nIndex >=0);
        if (nIndex >=0) //kein Fehler
                SetItemData(nIndex, (DWORD)pDesc);
      } /* endif */
   }

   SetTheText(str);
   m_arrayFontDesc.RemoveAll();
}

void CFontComboBox::AddFont(ENUMLOGFONT* pelf, DWORD dwType, LPCTSTR lpszScript)
{
   LOGFONT& lf = pelf->elfLogFont;
   if (lf.lfCharSet == MAC_CHARSET) // keine MAC Fonts erlauben, commdlg tut es auch nicht
           return;
   // vertikalen Font für FE Plattform nicht anzeigen
   if ((GetSystemMetrics(SM_DBCSENABLED)) && (lf.lfFaceName[0] == '@'))
           return;
   // keine non-printer Raster Fonts erlauben
   CFontDesc* pDesc = new CFontDesc(lf.lfFaceName, lpszScript,
           lf.lfCharSet, lf.lfPitchAndFamily, dwType);
   m_arrayFontDesc.Add(pDesc);
}



BOOL CALLBACK AFX_EXPORT CFontComboBox::EnumFamScreenCallBackEx(ENUMLOGFONTEX* pelf,
        NEWTEXTMETRICEX* /*lpntm*/, int FontType, LPVOID pThis)
{
   // keine non-printer Raster Fonts erlauben
   if (FontType & RASTER_FONTTYPE)
           return 1;
   DWORD dwData = (FontType & TRUETYPE_FONTTYPE) ? TT_FONT : 0;
   ((CFontComboBox *)pThis)->AddFont((ENUMLOGFONT*)pelf, dwData, CString(pelf->elfScript));
   return 1;
}

BOOL CALLBACK AFX_EXPORT CFontComboBox::EnumFamPrinterCallBackEx(ENUMLOGFONTEX* pelf,
        NEWTEXTMETRICEX* /*lpntm*/, int FontType, LPVOID pThis)
{
   DWORD dwData = PRINTER_FONT;
   if (FontType & TRUETYPE_FONTTYPE)
           dwData |= TT_FONT;
   else if (FontType & DEVICE_FONTTYPE)
           dwData |= DEVICE_FONT;
   ((CFontComboBox *)pThis)->AddFont((ENUMLOGFONT*)pelf, dwData, CString(pelf->elfScript));
   return 1;
}


void CFontComboBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
   ASSERT(lpDIS->CtlType == ODT_COMBOBOX);
   int id = (int)(WORD)lpDIS->itemID;

   CDC *pDC = CDC::FromHandle(lpDIS->hDC);
   CRect rc(lpDIS->rcItem);
   if (lpDIS->itemState & ODS_FOCUS)
           pDC->DrawFocusRect(rc);
   int nIndexDC = pDC->SaveDC();

   CBrush brushFill;
   if (lpDIS->itemState & ODS_SELECTED)
   {
      brushFill.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
      pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
   }
   else
      brushFill.CreateSolidBrush(pDC->GetBkColor());
   pDC->SetBkMode(TRANSPARENT);
   pDC->FillRect(rc, &brushFill);

   CFontDesc* pDesc= (CFontDesc*)lpDIS->itemData;
   ASSERT(pDesc != NULL);
   DWORD dwData = pDesc->m_dwFlags;
   if (dwData & (TT_FONT|DEVICE_FONT)) // Truetype oder Device Attribut gesetzt von SetItemData
   {
      CDC dc;
      dc.CreateCompatibleDC(pDC);
      CBitmap* pBitmap = dc.SelectObject(&m_bmFontType);
      if (dwData & TT_FONT)
              pDC->BitBlt(rc.left, rc.top, BMW, BMH, &dc, BMW, 0, SRCAND);
      else // DEVICE_FONT
              pDC->BitBlt(rc.left, rc.top, BMW, BMH, &dc, 0, 0, SRCAND);
      dc.SelectObject(pBitmap);
   }

   rc.left += BMW + 6;
   CString strText;
   GetLBText(id, strText);
   pDC->TextOut(rc.left,rc.top,strText,strText.GetLength());

   pDC->RestoreDC(nIndexDC);
}

void CFontComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
   ASSERT(lpMIS->CtlType == ODT_COMBOBOX);
   ASSERT(m_nFontHeight > 0);
   CRect rc;

   GetWindowRect(&rc);
   lpMIS->itemWidth = rc.Width();
   lpMIS->itemHeight = max(BMH, m_nFontHeight);
}

int CFontComboBox::CompareItem(LPCOMPAREITEMSTRUCT lpCIS)
{
   ASSERT(lpCIS->CtlType == ODT_COMBOBOX);
   int id1 = (int)(WORD)lpCIS->itemID1;
   int id2 = (int)(WORD)lpCIS->itemID2;
   CString str1,str2;
   if (id1 == -1)
           return -1;
   if (id2 == -1)
           return 1;
   GetLBText(id1, str1);
   GetLBText(id2, str2);
   return str1.Collate(str2);
}

// finde einen Font mit dem entsprechenden Namen und Charset
void CFontComboBox::MatchFont(LPCTSTR lpszName, BYTE nCharSet)
{
  BOOL fFound = FALSE;
   int nFirstIndex = FindString(-1, lpszName);
   if (nFirstIndex != CB_ERR)
   {
      int nIndex = nFirstIndex;
      do
      {
         CFontDesc* pDesc = (CFontDesc*)GetItemData(nIndex);
         ASSERT(pDesc != NULL);
         // überprüfe den aktuellen Font Namen um zu vermeiden, dass Courier Western
         // mit Courier New Western gematcht wird.
//         if ((nCharSet == DEFAULT_CHARSET) ||
           if ( lstrcmp(lpszName, pDesc->m_strName)== 0)
         {
            do
            {
              if (pDesc->m_nCharSet == nCharSet)
              {
                fFound = TRUE;                  //correct pDesc found!
                if (GetCurSel() != nIndex)
                        SetCurSel(nIndex);
              }
              else
              {
                pDesc = pDesc->GetNext();
              } /* endif */
            } while (!fFound && pDesc); /* enddo */
            return;
         }
         nIndex = FindString(nIndex, lpszName);
      } while (nIndex != nFirstIndex);
      // Schleife bis gefunden oder zurck zum ersten Element
   }
   //Font Name eingeben
   SetTheText(lpszName, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CColorComboBox

BEGIN_MESSAGE_MAP(CColorComboBox, CComboBox)
        //{{AFX_MSG_MAP(CColorComboBox)
        ON_WM_DESTROY()
        ON_WM_CREATE()
        //}}AFX_MSG_MAP
        // Globale Hilfsbefehle
END_MESSAGE_MAP()

CColorComboBox::CColorComboBox()
{
//   VERIFY(m_bmFontType.LoadBitmap(ID_TB_FONTEXT_FONTTYPE));
}

int CColorComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if (CComboBox::OnCreate(lpCreateStruct) == -1)
           return -1;
   SendMessage( WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FONT));
   return 0;
}

void CColorComboBox::OnDestroy()
{
   CComboBox::OnDestroy();
}

void CColorComboBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
        ASSERT(lpDIS->CtlType == ODT_COMBOBOX);
        UINT id = (UINT)(WORD)lpDIS->itemID;
        CDC dc;
        dc.Attach(lpDIS->hDC);

        CRect rc(lpDIS->rcItem);
        ASSERT(rc.Width() < 500);
        if (lpDIS->itemState & ODS_FOCUS)
                dc.DrawFocusRect(&rc);

        COLORREF cr = (lpDIS->itemState & ODS_SELECTED) ?
                ::GetSysColor(COLOR_HIGHLIGHT) :
                dc.GetBkColor();

        CBrush brushFill(cr);
        cr = dc.GetTextColor();

        if (lpDIS->itemState & ODS_SELECTED)
                dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));

        int nBkMode = dc.SetBkMode(TRANSPARENT);
        dc.FillRect(&rc, &brushFill);

        rc.left += 50;
        CString strColor;
        if (id != CB_ERR )
        {
          GetLBText(id, strColor);

          dc.TextOut(rc.left,rc.top,strColor,strColor.GetLength());
          rc.left -= 45;
          rc.top += 2;
          rc.bottom -= 2;
          rc.right = rc.left + 40;
          COLORREF crItem = (COLORREF) lpDIS->itemData;
          CBrush brush(crItem);
          CBrush* pOldBrush = dc.SelectObject(&brush);
          dc.Rectangle(rc);

          dc.SelectObject(pOldBrush);
          dc.SetTextColor(cr);
          dc.SetBkMode(nBkMode);
        } /* endif */

        dc.Detach();
}
void CColorComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
        ASSERT(lpMIS->CtlType == ODT_COMBOBOX);
        UINT id = (UINT)(WORD)lpMIS->itemID;
//      ASSERT(id == lpMIS->itemData);
        CDisplayIC dc;
        CString strColor;
        if (id != CB_ERR )
        {
          GetLBText(id, strColor);
          CSize sizeText = dc.GetTextExtent(strColor,strColor.GetLength());
          ASSERT(sizeText.cx < 500);
          lpMIS->itemWidth = sizeText.cx + 50;
          lpMIS->itemHeight = sizeText.cy;
        } /* endif */
}
