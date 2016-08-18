/*! \file
	Description: EQF Column listbox class

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

// force usage of old 32bit time value (Note: only dates up to the year 2038 are supported)
#define _USE_32BIT_TIME_T

#define OEMRESOURCE

#include "eqfmfc.h"

#ifndef CPPTEST
extern "C"
{
#endif
/**********************************************************************/
/* ensure correct packing of our structures                           */
/**********************************************************************/
#pragma pack( push, TM2StructPacking, 1 )
  // use import DLL defines for dbcs_cp ...
  #define DLLIMPORTDBCSCP

  #include "eqf.h"                     // General .H for EQF
  #include "eqffol.h"
  #include "eqfutclb.id"               // ids for View selection dialog
  #include "eqftwb.id"                 // ids for sort selection dialog
  #include "eqfdrvex.h"

#pragma pack( pop, TM2StructPacking )

#ifndef CPPTEST
}
#endif


#define CLB_START_COMMENT   '['
#define CLB_END_COMMENT     ']'

#include "eqfclbox.h"
#include "eqfd.h"
#include "shlobj.h"
#include "eqfplgmg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CLB_300_OFFSET 4

// flag to mark sort items for descending sort
#define SORT_DESCENDING_FLAG  0x0800

/////////////////////////////////////////////////////////////////////////////
// CListViewEx

IMPLEMENT_DYNCREATE(CListViewEx, CListView)

BEGIN_MESSAGE_MAP(CListViewEx, CListView)
        //{{AFX_MSG_MAP(CListViewEx)
//        ON_WM_HELPINFO()
        ON_WM_SIZE()
        ON_WM_MOVE()
        ON_WM_PAINT()
        ON_WM_SETFOCUS()
        ON_WM_KILLFOCUS()
        ON_WM_DESTROY()
        ON_WM_RBUTTONDOWN()
        ON_WM_SYSKEYDOWN()

        ON_MESSAGE(WM_EQFN_DELETED, OnDeleted)
        ON_MESSAGE(WM_EQF_INSERTNAMES, OnInsertNames)
        ON_MESSAGE(WM_EQF_GETCOLUMNWIDTH, OnGetColumnWidth)
        ON_MESSAGE(WM_EQFN_CREATED, OnCreated )
        ON_MESSAGE(WM_EQFN_PROPERTIESCHANGED, OnPropertiesChanged)
        ON_MESSAGE(LM_EQF_SETITEMTEXT, OnSetItemText)
//        ON_COMMAND( WM_KEYDOWN, OnKeyDown)
        ON_COMMAND( ID_FILE_PRINT, CView::OnFilePrint )
        ON_NOTIFY_RANGE( LVN_COLUMNCLICK, 0, 60000, OnHeaderClick )
        ON_NOTIFY(HDN_ITEMCHANGED, 0, OnHeaderchanged)
        //}}AFX_MSG_MAP

        ON_MESSAGE(LVM_SETIMAGELIST, OnSetImageList)
        ON_MESSAGE(LVM_SETTEXTCOLOR, OnSetTextColor)
        ON_MESSAGE(LVM_SETTEXTBKCOLOR, OnSetTextBkColor)
        ON_MESSAGE(LVM_SETBKCOLOR, OnSetBkColor)

END_MESSAGE_MAP()


// data area and function for item sort
typedef struct _CLBCOMPAREITEMDATA
{
  PSHORT psSortColumns;
  PUSHORT pusColumnType;
} CLBCOMPAREITEMDATA, *PCLBCOMPAREITEMDATA;

int CALLBACK CLBCompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK CLBCompareSingleColum( USHORT usColType, PSZ pszColText1, PSZ pszColText2 );

// convert a normal button to a bitmap button using the specified Windows bitmap
int MakeBitmapButton( CDialog *pDlg, int iButtonID, int iBitmapID );


/////////////////////////////////////////////////////////////////////////////
// Operators of column filter conditions
typedef enum _CLBOPID
{
  EQUAL_CLBOPID = 1,
  NOTEQUAL_CLBOPID,
  LESS_CLBOPID,
  LESSOREQUAL_CLBOPID,
  GREATER_CLBOPID,
  GREATEROREQUAL_CLBOPID,
  LIKE_CLBOPID,
  LAST_CLBOPID
} CLBOPID;

typedef struct _CLBFILTOP
{
  CLBOPID     OpID;                    // ID of operator
  SHORT       sStringID;               // ID of string for operator
} CLBFILTOP;

CLBFILTOP ClbOperator[] =
{
  { EQUAL_CLBOPID,          SID_EQUALOP },
  { NOTEQUAL_CLBOPID,       SID_NOTEQUALOP },
  { LESS_CLBOPID,           SID_LESSOP },
  { LESSOREQUAL_CLBOPID,    SID_LESSEQOP },
  { GREATER_CLBOPID,        SID_GREATEROP },
  { GREATEROREQUAL_CLBOPID, SID_GREATEREQOP },
  { LIKE_CLBOPID,           SID_LIKEOP },
  { LAST_CLBOPID,           0 }
};




/////////////////////////////////////////////////////////////////////////////
// CListViewEx construction/destruction

CListViewEx::CListViewEx()
{
  m_bFullRowSel = FALSE;
  m_bClientWidthSel = TRUE;

  m_cxClient = 0;
  m_cxStateImageOffset = 0;

  m_clrText = ::GetSysColor(COLOR_WINDOWTEXT);
  m_clrTextBk = ::GetSysColor(COLOR_WINDOW);
  m_clrBkgnd = ::GetSysColor(COLOR_WINDOW);
  memset( ColumnHeader, 0, sizeof( ColumnHeader ));
  memset( asDetailsView, 0, sizeof( asDetailsView ));
  memset( asNameView, 0, sizeof( asNameView ));
  memset( asCurView, 0, sizeof( asCurView ));
  memset( asSortView, 0, sizeof( asSortView ));
  memset( usColumnWidth, 0, sizeof( usColumnWidth ));
  memset( usColumnType, 0, sizeof( usColumnType ));
  memset( usColumnAdjust, 0, sizeof( usColumnAdjust ));
}

CListViewEx::~CListViewEx()
{
}

void CListViewEx::OnDestroy()
{
  /********************************************************************/
  /* Freeing of original strings has to be done here                  */
  /* The destructor ~CListViewEx comes tooooooooo late....            */
  /********************************************************************/
  CListCtrl& ListCtrl=GetListCtrl();
  int iNumItems = ListCtrl.GetItemCount();
  for (int i=0; i<iNumItems; i++ )
  {
    free( (PSZ) ListCtrl.GetItemData( i ));
  } /* endfor */
}


BOOL CListViewEx::PreCreateWindow(CREATESTRUCT& cs)
{
        // default is report view and full row selection
        cs.style &= ~LVS_TYPEMASK;
        cs.style |= LVS_REPORT | LVS_OWNERDRAWFIXED;
        m_bFullRowSel = TRUE;

        return(CListView::PreCreateWindow(cs));
}

BOOL CListViewEx::SetFullRowSel(BOOL bFullRowSel)
{
        // no painting during change
        LockWindowUpdate();

        m_bFullRowSel = bFullRowSel;

        BOOL bRet;

        if (m_bFullRowSel)
                bRet = ModifyStyle(0L, LVS_OWNERDRAWFIXED);
        else
                bRet = ModifyStyle(LVS_OWNERDRAWFIXED, 0L);

        // repaint window if we are not changing view type
        if (bRet && (GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
                Invalidate();

        // repaint changes
        UnlockWindowUpdate();

        return(bRet);
}

BOOL CListViewEx::GetFullRowSel()
{
  return(m_bFullRowSel);
}

/////////////////////////////////////////////////////////////////////////////
// CListViewEx drawing

// offsets for first and other columns
#define OFFSET_FIRST    2
#define OFFSET_OTHER    6

void CListViewEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  CListCtrl& ListCtrl=GetListCtrl();
  CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
  CRect rcItem(lpDrawItemStruct->rcItem);
  UINT uiFlags = ILD_TRANSPARENT;
  CImageList* pImageList;
  int nItem = lpDrawItemStruct->itemID;
  BOOL bFocus = (GetFocus() == this);
  COLORREF clrTextSave, clrBkSave;
  COLORREF clrImage = m_clrBkgnd;
  static _TCHAR szBuff[MAX_PATH];
  LPCTSTR pszText;

  // init data
  memset(&clrBkSave, 0, sizeof(clrBkSave));
  memset(&clrTextSave, 0, sizeof(clrTextSave));
  // get item data

  LV_ITEM lvi;
  lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
  lvi.iItem = nItem;
  lvi.iSubItem = 0;
  lvi.pszText = szBuff;
  lvi.cchTextMax = sizeof(szBuff);
  lvi.stateMask = 0xFFFF;         // get all state flags
  ListCtrl.GetItem(&lvi);

  BOOL bSelected = (bFocus || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;
  bSelected = bSelected || (lvi.state & LVIS_DROPHILITED);

  BOOL bDisabled = (BOOL)*((PULONG)lvi.lParam );

  // set colors if item is selected
  CRect rcAllLabels;
  ListCtrl.GetItemRect(nItem, rcAllLabels, LVIR_BOUNDS);

  CRect rcLabel;
  ListCtrl.GetItemRect(nItem, rcLabel, LVIR_LABEL);

  rcAllLabels.left = rcLabel.left;
  if (m_bClientWidthSel && rcAllLabels.right<m_cxClient)
          rcAllLabels.right = m_cxClient;

  if ( bDisabled  )
  {
    clrTextSave = pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
    if ( bSelected )
    {
      clrBkSave = pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
      pDC->FillRect(rcAllLabels, &CBrush(::GetSysColor(COLOR_HIGHLIGHT)));
    }
    else
    {
      pDC->FillRect(rcAllLabels, &CBrush(::GetSysColor(COLOR_WINDOW)));
    } /* endif */
  }
  else if (bSelected)
  {
    clrTextSave = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
    clrBkSave = pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));

    pDC->FillRect(rcAllLabels, &CBrush(::GetSysColor(COLOR_HIGHLIGHT)));
  }
  else
  {
    clrTextSave = pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
    pDC->FillRect(rcAllLabels, &CBrush(::GetSysColor(COLOR_WINDOW)));
  } /* endif */

  // set color and mask for the icon
  if (lvi.state & LVIS_CUT)
  {
    clrImage = ::GetSysColor(COLOR_WINDOW);
    uiFlags |= ILD_BLEND50;
  }
  else if (bSelected)
  {
    clrImage = ::GetSysColor(COLOR_HIGHLIGHT);
    uiFlags |= ILD_BLEND50;
  }

  // draw state icon
  UINT nStateImageMask = lvi.state & LVIS_STATEIMAGEMASK;
  if (nStateImageMask)
  {
    int nImage = (nStateImageMask>>12) - 1;
    pImageList = ListCtrl.GetImageList(LVSIL_STATE);
    if (pImageList)
    {
      pImageList->Draw(pDC, nImage,
              CPoint(rcItem.left, rcItem.top), ILD_TRANSPARENT);
    }
  }

  // draw normal and overlay icon
  CRect rcIcon;
  ListCtrl.GetItemRect(nItem, rcIcon, LVIR_ICON);

  pImageList = ListCtrl.GetImageList(LVSIL_SMALL);
  if (pImageList)
  {
    UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
    if (rcItem.left<rcItem.right-1)
    {
      ImageList_DrawEx(pImageList->m_hImageList, lvi.iImage,
                      pDC->m_hDC,rcIcon.left,rcIcon.top, 16, 16,
                      m_clrBkgnd, clrImage, uiFlags | nOvlImageMask);
    }
  }

  // draw item label

  ListCtrl.GetItemRect(nItem, rcItem, LVIR_LABEL);
  rcItem.right -= m_cxStateImageOffset;

  pszText = MakeShortString(pDC, szBuff,
                          rcItem.right-rcItem.left, 2*OFFSET_FIRST);

  rcLabel = rcItem;
  rcLabel.left += OFFSET_FIRST;
  rcLabel.right -= OFFSET_FIRST;

  pDC->DrawText(pszText,-1,rcLabel,DT_HALFTONE |
                DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);

  // draw labels for extra columns
  LV_COLUMN lvc;
  memset( &lvc, 0, sizeof( lvc ));
  lvc.mask = LVCF_FMT | LVCF_WIDTH;

  for(int nColumn = 1; ListCtrl.GetColumn(nColumn, &lvc); nColumn++)
  {
    rcItem.left = rcItem.right;
    rcItem.right += lvc.cx;

    int nRetLen = ListCtrl.GetItemText(nItem, nColumn,
                                    szBuff, sizeof(szBuff));
    if (nRetLen == 0)
            continue;

    pszText = MakeShortString(pDC, szBuff,
                              rcItem.right - rcItem.left, 2*OFFSET_OTHER);

    UINT nJustify = DT_LEFT;

    if(pszText == szBuff)
    {
      switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
      {
        case LVCFMT_RIGHT:
          nJustify = DT_RIGHT;
          break;
        case LVCFMT_CENTER:
          nJustify = DT_CENTER;
          break;
        default:
          break;
      }
    }

    rcLabel = rcItem;
    rcLabel.left += OFFSET_OTHER;
    rcLabel.right -= OFFSET_OTHER;

    pDC->DrawText(pszText, -1, rcLabel,
                  nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);
  }

  // draw focus rectangle if item has focus
  if (lvi.state & LVIS_FOCUSED && bFocus)
          pDC->DrawFocusRect(rcAllLabels);

  // set original colors if item was selected
  if ( bDisabled )
  {
    pDC->SetTextColor(clrTextSave);
    if ( bSelected )
    {
      pDC->SetBkColor(clrBkSave);
    } /* endif */
  }
  else if ( bSelected )
  {
    pDC->SetTextColor(clrTextSave);
    pDC->SetBkColor(clrBkSave);
  }
}

LPCTSTR CListViewEx::MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset)
{
        static const _TCHAR szThreeDots[] = _T("...");

        int nStringLen = lstrlen(lpszLong);

        if(nStringLen == 0 ||
                (pDC->GetTextExtent(lpszLong, nStringLen).cx + nOffset) <= nColumnLen)
        {
                return(lpszLong);
        }

        static _TCHAR szShort[MAX_PATH];

        lstrcpy(szShort,lpszLong);
        int nAddLen = pDC->GetTextExtent(szThreeDots,sizeof(szThreeDots)).cx;

        for(int i = nStringLen-1; i > 0; i--)
        {
                szShort[i] = 0;
                if((pDC->GetTextExtent(szShort, i).cx + nOffset + nAddLen)
                        <= nColumnLen)
                {
                        break;
                }
        }

        lstrcat(szShort, szThreeDots);
        return(szShort);
}

void CListViewEx::RepaintSelectedItems()
{
        CListCtrl& ListCtrl = GetListCtrl();
        CRect rcItem, rcLabel;

// invalidate focused item so it can repaint properly

        int nItem = ListCtrl.GetNextItem(-1, LVNI_FOCUSED);

        if(nItem != -1)
        {
                ListCtrl.GetItemRect(nItem, rcItem, LVIR_BOUNDS);
                ListCtrl.GetItemRect(nItem, rcLabel, LVIR_LABEL);
                rcItem.left = rcLabel.left;

                InvalidateRect(rcItem, FALSE);
        }

// if selected items should not be preserved, invalidate them

        if(!(GetStyle() & LVS_SHOWSELALWAYS))
        {
                for(nItem = ListCtrl.GetNextItem(-1, LVNI_SELECTED);
                        nItem != -1; nItem = ListCtrl.GetNextItem(nItem, LVNI_SELECTED))
                {
                        ListCtrl.GetItemRect(nItem, rcItem, LVIR_BOUNDS);
                        ListCtrl.GetItemRect(nItem, rcLabel, LVIR_LABEL);
                        rcItem.left = rcLabel.left;

                        InvalidateRect(rcItem, FALSE);
                }
        }

// update changes

        UpdateWindow();
}

//LONG CListViewEx::DeleteContentAll()
//{
//  CListCtrl& ListCtrl = GetListCtrl();
//  USHORT usI = ListCtrl.GetItemCount();
//  usI = 0;
//  for ( int i=usI-1; i >= 0;i-- )
//  {
//    /******************************************************************/
//    /* free allocated resources and finally delete item               */
//    /******************************************************************/
//    PSZ p = (PSZ) ListCtrl.GetItemData(i);
//    if ( p )
//    {
//      free( p );
//      ListCtrl.DeleteItem( i );
//    } /* endif */
//  } /* endfor */
//  Invalidate();
//  UpdateWindow();
//  return 1L;
//}
/////////////////////////////////////////////////////////////////////////////
// CListViewEx diagnostics

#ifdef _DEBUG

void CListViewEx::Dump(CDumpContext& dc) const
{
        CListView::Dump(dc);

        dc << "m_bFullRowSel = " << (UINT)m_bFullRowSel;
        dc << "\n";
        dc << "m_cxStateImageOffset = " << m_cxStateImageOffset;
        dc << "\n";
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CListViewEx message handlers

LRESULT CListViewEx::OnSetImageList(WPARAM wParam, LPARAM lParam)
{
        if( (int) wParam == LVSIL_STATE)
        {
                int cx, cy;

                if(::ImageList_GetIconSize((HIMAGELIST)lParam, &cx, &cy))
                        m_cxStateImageOffset = cx;
                else
                        m_cxStateImageOffset = 0;
        }

        return(Default());
}

LRESULT CListViewEx::OnSetTextColor(WPARAM wParam, LPARAM lParam)
{
	wParam;
        m_clrText = (COLORREF)lParam;
        return(Default());
}

LRESULT CListViewEx::OnSetTextBkColor(WPARAM wParam, LPARAM lParam)
{
	wParam;
        m_clrTextBk = (COLORREF)lParam;
        return(Default());
}

LRESULT CListViewEx::OnSetBkColor(WPARAM wParam, LPARAM lParam)
{
	wParam;
        m_clrBkgnd = (COLORREF)lParam;
        return(Default());
}

LRESULT CListViewEx::OnDeleted(WPARAM wParam, LPARAM lParam)
{
  SHORT sClassId = (SHORT) wParam;
  PSZ pObject = (PSZ) lParam;
  if ( pIda->fWantsDelMsg )
  {
    (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd,
                                    WM_EQFN_DELETED,
                                    MP1FROMSHORT(sClassId),
                                    MP2FROMP( pObject ) );
  }
  else if ( sClassId == pIda->CommArea.sItemClass )
  {
    DeleteRow( pObject);
  } /* endif */

  return 0;
}


LRESULT  CListViewEx::OnInsertNames(WPARAM mp1, LPARAM mp2)
{
  HWND       hwndTargetLB = HWNDFROMMP1(mp1);
  SHORT      sItemCount;
  SHORT      sIndex;
  BOOL       fCombo;

  mp2;
  ISCOMBOBOX( hwndTargetLB, fCombo );

  sItemCount = QUERYITEMCOUNTHWND( pIda->CommArea.hwndLB );

  for ( sIndex = 0; sIndex < sItemCount; sIndex++ )
  {
          if ( (BOOL)WinSendMsg( pIda->CommArea.hwndLB,
                           LM_EQF_QUERYITEMSTATE,
                           MP1FROMSHORT( sIndex ),
                           MP2FROMP(NULL) ) )
    {
      QUERYITEMTEXTHWND( pIda->CommArea.hwndLB, sIndex,
                         pIda->CommArea.szBuffer );
      if ( fCombo )
      {
        CBINSERTITEMHWND( hwndTargetLB,
                          UtlParseX15( pIda->CommArea.szBuffer,
                                       pIda->CommArea.sNameIndex ) );
      }
      else
      {
        INSERTITEMHWND( hwndTargetLB,
                        UtlParseX15( pIda->CommArea.szBuffer,
                                     pIda->CommArea.sNameIndex ) );
      } /* endif */
    } /* endif */
  } /* endfor */
  if ( fCombo )
  {
    sItemCount = CBQUERYITEMCOUNTHWND( hwndTargetLB );
  }
  else
  {
    sItemCount = QUERYITEMCOUNTHWND( hwndTargetLB );
  } /* endif */

  return sItemCount;
}


void CListViewEx::OnHeaderchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
//  HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
  NMHEADER *pHdr = (NMHEADER*)pNMHDR;
    CDC *pCDC = GetDC();

  // Get a pointer to the header control and figure out what happened
    CListCtrl& list = GetListCtrl();
  CHeaderCtrl *pHeader = list.GetHeaderCtrl();
  ASSERT(pHeader);
  HDITEM hdi;
  hdi.mask = HDI_WIDTH;
  pHeader->GetItem(pHdr->iItem, &hdi);
    CSize size = pCDC->GetTextExtent( "A",1);
    usColumnWidth[pHdr->iItem+1] = (USHORT)(hdi.cxy/size.cx);

  //UpdateData(TRUE);
    pIda->CommArea.asCurViewWidth[pHdr->iItem] = usColumnWidth[pHdr->iItem+1];

  *pResult = 0;
}



LRESULT  CListViewEx::OnGetColumnWidth(WPARAM mp1, LPARAM mp2)
{
  USHORT usWidth;
  CDC *pCDC = GetDC();
  PUSHORT  pusWidth = (PUSHORT)mp1;
  CListCtrl& list = GetListCtrl();

  mp2;
//for (int i=0;i<21;i++)
  for (int i=0;i<MAX_VIEW;i++)
  {
    if (asCurView[i] != -1)
  {
        usWidth = (USHORT)(list.GetColumnWidth(i));

        CSize size =  pCDC->GetTextExtent( "A",1);
        pusWidth[i] = (USHORT)(usWidth/size.cx);
    }
    else
        break;
  }

  return 0;
}


LRESULT  CListViewEx::OnCreated(WPARAM wParam, LPARAM lParam)
{
  LRESULT LRESULT;
  SHORT   sClassReqId = (SHORT) wParam;
  PSZ pszObjName =  (PSZ) lParam;

  if ( pIda->fWantsCreateMsg )
  {
    LRESULT = (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd, WM_EQFN_CREATED,
                                    sClassReqId,
                                     MP2FROMP( pszObjName ) );
  }
  else if ( sClassReqId == pIda->CommArea.sItemClass )
  {
    BOOL       fInsert;

    /**************************************************************/
    /* Leave it to callback function to supply the text for the   */
    /* new item                                                   */
    /**************************************************************/
    fInsert = (BOOL)(*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd,
                                     WM_EQF_BUILDITEMTEXT,
                                     MP1FROMSHORT(0),
                                     MP2FROMP( pszObjName ) );

    /**************************************************************/
    /* Insert new item                                            */
    /**************************************************************/
    if ( fInsert )
    {
      InsertRow( pIda->CommArea.szBuffer );
    } /* endif */
  } /* endif */
  return TRUE;
}

LRESULT CListViewEx::OnPropertiesChanged(WPARAM mp1, LPARAM mp2)
{

  SHORT sPropClassReq = (SHORT) mp1;
  PSZ pszData = (PSZ) mp2;

  if ( pIda->fWantsPropMsg )
  {
    (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd, WM_EQFN_PROPERTIESCHANGED,
                          MP1FROMSHORT(sPropClassReq),
                                                  MP2FROMP(pszData) );
  }
  else if ( sPropClassReq == pIda->CommArea.sItemPropClass )
  {
    int      iItem;
    BOOL       fUpdate = FALSE;

    /**************************************************************/
    /* Search object name of item in our listbox                  */
    /**************************************************************/
    {
      CListCtrl& ListCtrl = GetListCtrl();

      BOOL fFound = FALSE;
      int iLen = strlen( pszData );
      iItem = -1;
      do
      {
        iItem = ListCtrl.GetNextItem( iItem, LVNI_ALL );
        if (iItem != -1)
        {
          PSZ pTemp =(PSZ) ListCtrl.GetItemData( iItem );
          if ( pTemp && strncmp( pTemp+CLB_300_OFFSET, pszData, iLen ) == 0)
          {
            fFound = TRUE;
          } /* endif */
        } /* endif */
      } while ( !fFound && (iItem != -1) ); /* enddo */
      if ( !fFound ) iItem = LIT_NONE;
    }

    /**************************************************************/
    /* If found, leave it to callback function to supply the new  */
    /* item text for the item                                     */
    /**************************************************************/
    if ( iItem != LIT_NONE )
    {
      fUpdate = (BOOL)(*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd,
                                       WM_EQF_BUILDITEMTEXT,
                                       MP1FROMSHORT(0),
                                       MP2FROMP( pszData ) );
    } /* endif */

    /**************************************************************/
    /* Update item with new item text                             */
    /**************************************************************/
    if ( fUpdate )
    {
//    SETITEMTEXTHWND( pIda->CommArea.hwndLB, iItem, pIda->CommArea.szBuffer );
      LV_ITEM lvi;
      lvi.mask = LVIF_PARAM;
      lvi.iItem = iItem;
      lvi.iSubItem = 0;
      CListCtrl& ListCtrl=GetListCtrl();
      if ( ListCtrl.GetItem( &lvi ) )
      {
        LONG l = 0L;
        if ( lvi.lParam )
        {
          l = *((PULONG) lvi.lParam );
        } /* endif */
        free( (PSZ) lvi.lParam);
        PSZ pData = new char[ strlen( pIda->CommArea.szBuffer ) +
                                      CLB_300_OFFSET+1 ];
        *((PULONG) pData) = l;
        strcpy( pData+CLB_300_OFFSET, pIda->CommArea.szBuffer );
        lvi.lParam = (LONG) pData;
        ListCtrl.SetItem( &lvi );
        InsertColumnItems( iItem, (PSZ) (lvi.lParam + CLB_300_OFFSET) );
      } /* endif */
    } /* endif */
  } /* endif */
  return 0;
}

SHORT  CListViewEx::GetSelectedItems( SHORT sMaxItems, LPINT lpnItems )
{
  SHORT sFilled = 0;
  int   iItem;
  CListCtrl& ListCtrl=GetListCtrl();

  for(iItem = ListCtrl.GetNextItem(-1, LVNI_SELECTED);
          iItem != -1; iItem = ListCtrl.GetNextItem(iItem, LVNI_SELECTED))
  {
    if ( sFilled < sMaxItems )
    {
      *lpnItems++ = iItem;
      sFilled++;
        }
        else
        {
                iItem = -1;  // stop the loop
    } /* endif */
  }
  return sFilled;
}



void CListViewEx::OnSize(UINT nType, int cx, int cy)
{
  m_cxClient = cx;
  CListView::OnSize(nType, cx, cy);
  if ( pIda )
  {
    UtlSaveWindowPos( pIda->hFrame, &(pIda->CommArea.swpSizePos) );
  } /* endif */
}

void CListViewEx::OnMove(int cx, int cy)
{
  CListView::OnMove(cx, cy);
  UtlSaveWindowPos( pIda->hFrame, &(pIda->CommArea.swpSizePos) );
}

void CListViewEx::OnPaint()
{
  // in full row select mode, we need to extend the clipping region
  // so we can paint a selection all the way to the right
  if (m_bClientWidthSel &&
          (GetStyle() & LVS_TYPEMASK) == LVS_REPORT &&
          GetFullRowSel())
  {
    CRect rcAllLabels, rcUpdate;
    GetListCtrl().GetItemRect(0, rcAllLabels, LVIR_BOUNDS);

    if(rcAllLabels.right < m_cxClient)
    {
      GetUpdateRect( &rcUpdate, FALSE );
      // need to call BeginPaint (in CPaintDC c-tor)
      // to get correct clipping rect
      CPaintDC dc(this);

      CRect rcClip;
      dc.GetClipBox(rcClip);

      rcClip.left = min(rcAllLabels.right-1, rcClip.left);
      rcClip.right = m_cxClient;
      rcClip.top = rcUpdate.top;
      rcClip.bottom = rcUpdate.bottom;

      InvalidateRect(rcClip, FALSE);
      // EndPaint will be called in CPaintDC d-tor
    }
  } /* endif */

  if (IsWindowVisible())
     CListView::OnPaint();

}



void CListViewEx::OnSetFocus(CWnd* pOldWnd)
{
        CListView::OnSetFocus(pOldWnd);

        // check if we are getting focus from label edit box
        if(pOldWnd!=NULL && IsWindow(pOldWnd->m_hWnd) && pOldWnd->GetParent()==this)
                return;

        // repaint items that should change appearance
        if(m_bFullRowSel && (GetStyle() & LVS_TYPEMASK)==LVS_REPORT)
                RepaintSelectedItems();
}

void CListViewEx::OnKillFocus(CWnd* pNewWnd)
{
        CListView::OnKillFocus(pNewWnd);

        // check if we are losing focus to label edit box
        if(pNewWnd != NULL && pNewWnd->GetParent() == this)
                return;

        // repaint items that should change appearance
        if(m_bFullRowSel && (GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
                RepaintSelectedItems();
}


void CListViewEx::OnRButtonDown(UINT nFlags, CPoint point)
{
  RECT rc;
  CListCtrl& ListCtrl=GetListCtrl();
  GetClientRect( (LPRECT)&rc);

  if (PtInRect ((LPRECT)&rc, point))
  {
    LV_HITTESTINFO HitTestInfo;

    HitTestInfo.pt = point;
    int nHitItem = ListCtrl.HitTest(&HitTestInfo);

    if ( nHitItem != -1 )
    {
      LV_ITEM lvi;
      lvi.mask = LVIF_STATE;
      lvi.iItem = nHitItem;
      lvi.iSubItem = 0;
   //   lvi.pszText = szBuff;
   //   lvi.cchTextMax = sizeof(szBuff);
      lvi.stateMask = 0xFFFF;         // get all state flags
      ListCtrl.GetItem(&lvi);

      EqfActivateInstance( m_hWnd, TRUE );

      BOOL bSelected = lvi.state & LVIS_SELECTED;
      bSelected = bSelected || (lvi.state & LVIS_DROPHILITED);
      if ( !bSelected )
      {
        /**************************************************************/
        /* select item                                                */
        /**************************************************************/
        int i = point.x;
        point.x = 4;
        OnLButtonDown(nFlags, point);
        point.x = i;
      } /* endif */

      UINT uiSelectedItems = ListCtrl.GetSelectedCount();
      if ( (uiSelectedItems > 1) && (sMultSelPopUpId > 0) )
      {
        HandlePopupMenu( this->m_hWnd, point, sMultSelPopUpId );
      }
      else
      {
        HandlePopupMenu( this->m_hWnd, point, sPopUpId );
      } /* endif */
    }
    else
    {
      HandlePopupMenu( this->m_hWnd, point, sNoSelPopUpId );
    } /* endif */
  } /* endif */

}


VOID CListViewEx::OnSysKeyDown(UINT mp1, UINT n, UINT m )
{
	BOOL fShift;
	UINT nFlags = 0;
	CPoint point;

    switch ( mp1 )
  {
	  case VK_F10:
      {
        fShift = GetKeyState(VK_SHIFT) >> 15 ;
        if (fShift)
        {
           point = GetCaretPos();

           RECT rc;
		   CListCtrl& ListCtrl=GetListCtrl();
		   GetClientRect( (LPRECT)&rc);

           point.x = (rc.left + rc.right)/4;
//         point.y = (rc.top + rc.bottom)/4;
           point.y = (rc.top + 10);
           CListViewEx::OnRButtonDown( nFlags,  point);
        }
        else
        {
		  CListView::OnSysKeyDown(mp1,n, m);
	    }
      }
      break;

    default:
      CListView::OnSysKeyDown(mp1,n, m);
      break;
  } /* endswitch */

  //return;
}
/**********************************************************************/
/* delete the row (object) in the listbox                             */
/**********************************************************************/
VOID CListViewEx::DeleteRow( PSZ pszBuffer )
{
  CListCtrl& ListCtrl = GetListCtrl();

  ULONG ulLen = strlen( pszBuffer );
  int  iItem = -1;
  do
  {
    iItem = ListCtrl.GetNextItem( iItem, LVNI_ALL );
    if (iItem != -1)
    {
      PSZ pTemp =(PSZ) ListCtrl.GetItemData( iItem );
      if ( pTemp && strncmp( pTemp+CLB_300_OFFSET, pszBuffer, ulLen ) == 0)
      {
        /******************************************************************/
        /* free allocated resources and finally delete item               */
        /******************************************************************/
        free( (PSZ) ListCtrl.GetItemData( iItem ));
        ListCtrl.DeleteItem( iItem );
        iItem = -1;
      } /* endif */
    } /* endif */
  } while ( iItem != -1 ); /* enddo */

  Invalidate();
  UpdateWindow();

  return;
}

BOOL CListViewEx::DeleteItem( int iItem )
{
  BOOL fOK = FALSE;
  if ( iItem != -1 )
  {
    /******************************************************************/
    /* free allocated resources and finally delete item               */
    /******************************************************************/
    CListCtrl& ListCtrl = GetListCtrl();
    free( (PSZ) ListCtrl.GetItemData( iItem ));
    fOK = ListCtrl.DeleteItem( iItem );
  } /* endif */

  Invalidate();
  UpdateWindow();
  return fOK;
}

BOOL CListViewEx::SetItemData( int iItem, LONG lText )
{
  CListCtrl& ListCtrl = GetListCtrl();
  if ( (iItem >= 0) && (iItem <= ListCtrl.GetItemCount()) )
  {
    PSZ pData = (PSZ) ListCtrl.GetItemData( iItem );
    if ( pData )
    {
      *((PULONG)pData) = lText;
    }
    else
    {
      pData = new char[ CLB_300_OFFSET + 1 ];
      memset( pData, 0, CLB_300_OFFSET + 1 );
      *((PULONG) pData) = lText;
    } /* endif */

    return ListCtrl.SetItemData( iItem, (LONG) pData );
  }
  else
  {
    return FALSE;
  } /* endif */
}

LONG CListViewEx::GetItemData( int iItem )
{
  CListCtrl& ListCtrl = GetListCtrl();
  LONG       lReturn  = 0L;

  if ( iItem <= ListCtrl.GetItemCount() )
  {
    PSZ        pData    = (PSZ) ListCtrl.GetItemData( iItem );
    if ( pData )
    {
      lReturn = *((PULONG)pData);
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* return 0L                                                      */
    /******************************************************************/
  } /* endif */

  return lReturn;
}


// find the row/object in the listbox
int CListViewEx::FindRow( PSZ pszBuffer )
{
  CListCtrl& ListCtrl = GetListCtrl();


  ULONG ulLen = strlen( pszBuffer );
  int  iItem = -1;
  do
  {
    iItem = ListCtrl.GetNextItem( iItem, LVNI_ALL );
    if (iItem != -1)
    {
      PSZ pTemp =(PSZ) ListCtrl.GetItemData( iItem );
      if ( pTemp && strncmp( pTemp+CLB_300_OFFSET, pszBuffer, ulLen ) == 0)
      {
        break;
      } /* endif */
    } /* endif */
  } while ( iItem != -1 ); /* enddo */

  return iItem;
}

void CListViewEx::FillColumnHeaders( PCLBOXCOLDATA pColData )
{
  HMODULE hResMod = AfxGetResourceHandle();
  memset( usColumnType, 0, sizeof(usColumnType) );
  memset( usColumnAdjust, 0, sizeof(usColumnAdjust) );
  memset( ColumnHeader, 0, sizeof(ColumnHeader) );
  int i = 1;
  while ( pColData->usId )
  {
    LOADSTRING( NULLHANDLE, hResMod, pColData->usId,   ColumnHeader[i]);
    if ( pColData->usWidth )
    {
      UtlLoadWidth(NULLHANDLE, hResMod, pColData->usWidth, &usColumnWidth[i]);
    }
    else
    {
      usColumnWidth[i] = 0;
    } /* endif */
    usColumnType[i] = (USHORT)pColData->DataType;
    usColumnAdjust[i] = pColData->usAdjust;
    i++; pColData++;
  } /* endwhile */
}


void CListViewEx::InsertTitle()
{
  /********************************************************************/
  /* insert only columns selected by the view                         */
  /********************************************************************/
  int iItem = 0;
  LV_COLUMN lvc;
  PPROPSYSTEM pPropSys = GetSystemPropPtr();

  memset( &lvc, 0, sizeof( lvc ));
  CListCtrl& ListCtrl = GetListCtrl();

  ListCtrl.SetImageList(&m_ImageList, LVSIL_SMALL);

  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  int i = 0;
//while ( (i<21)&&(iItem = asCurView[i]) != -1 )
  while ( (i<MAX_VIEW)&&((iItem = asCurView[i]) != -1) )
  {
    CHAR chTitle[81];

    iItem &= CLB_COLNUMBERMASK;

    lvc.iSubItem = i; // iItem;
    PSZ pszSource = ColumnHeader[iItem];
    lvc.pszText = chTitle;

    /*************************************************************/
    /* Filter column title -- we do not want to display parts in */
    /* brackets like e.g. Updated [Date+Time].                   */
    /*************************************************************/
    PSZ pszTarget = lvc.pszText;
    while ( *pszSource )
    {
      if ( *pszSource == CLB_START_COMMENT )
      {
        while ( *pszSource && (*pszSource != CLB_END_COMMENT ) )
        {
          if ( IsDBCS_CP(pPropSys->ulSystemPrefCP) &&
               (isdbcs1ex((USHORT)pPropSys->ulSystemPrefCP, *pszSource) == DBCS_1ST) )
          {
            // skip both bytes of DBCS character
            pszSource++;
            if ( *pszSource ) pszSource++;
          }
          else
          {
            pszSource++;
          } /* endif */
        } /* endwhile */
        if ( *pszSource == CLB_END_COMMENT )
        {
          pszSource++;
        } /* endif */
      }
      else
      {
        if ( IsDBCS_CP(pPropSys->ulSystemPrefCP) &&
             (isdbcs1ex((USHORT)pPropSys->ulSystemPrefCP, *pszSource) == DBCS_1ST) )
        {
          // copy both DBCS bytes
          *pszTarget++ = *pszSource++;
          if ( *pszSource ) *pszTarget++ = *pszSource++;
        }
        else
        {
          *pszTarget++ = *pszSource++;
        } /* endif */
      } /* endif */
    } /* endwhile */
    *pszTarget = EOS;

    if ( lvc.pszText[0] == EOS )
    {
      lvc.cx = 0;
    }
    else
    {
      //CString strText("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMN");
      // fix character size needed
      //CString strText("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
      if ( usColumnWidth[iItem] )
      {
        CDC *pCDC = GetDC();
        //CSize size =  pCDC->GetTextExtent( strText, min( 40, usColumnWidth[iItem]));
        //lvc.cx = size.cx;
        CSize size =  pCDC->GetTextExtent( "A", 1) ;         /* 10-15-14 */
        lvc.cx = size.cx * usColumnWidth[iItem] ;
      } /* endif */
    } /* endif */
    lvc.fmt = usColumnAdjust[ iItem ];
    ListCtrl.InsertColumn(i /*iItem*/,&lvc);
    i++;
  } /* endwhile */
}

BOOL  CListViewEx::CheckFilterMatch( PSZ pszBuffer )
{
  BOOL fShowItem = TRUE;
  int iPos = 0;
  PSZ  pszFilterValue;                 // ptr to filter value to check for
  int iResult = 0;

  // apply any active filter
  while ( fShowItem && (iPos < 3) &&
          ((this->m_Filter.asColumn[iPos] != 0) && (this->m_Filter.asOperator[iPos] != 0)) )

  {
    CHAR szColData[MAX_LONGFILESPEC+1]; // buffer for column value

    // get value for column used in condition
    UtlExtractX15( szColData, pszBuffer, m_Filter.asColumn[iPos] );

    // compare depending on column type
    switch ( usColumnType[m_Filter.asColumn[iPos]] )
    {
       case DATE_DATA:
         // convert values to Win32 FILETIME and compare it
         {
           FILETIME ftFilterDate;      // date specified in filter
           FILETIME ftColDate;         // date listed in column

           // convert filter date
           {
             WORD  wDate;                // date specified in filter converted to FDATE
             UtlDateStringToFDate( m_Filter.aszValue[iPos], (FDATE *)&wDate );
             DosDateTimeToFileTime( wDate, 0, &ftFilterDate );
           }

           // convert column date
           {
             LONG  lDate;                // date of column as LONG value
             struct tm  *pTimeDate;      // time/date structure

             lDate = atol( szColData );
             if ( lDate ) lDate += 10800L;// correction: + 3 hours (why???)

             pTimeDate = localtime( (time_t *)&lDate );

             if ( (lDate == 0L) || (pTimeDate == NULL) )
             {
               DosDateTimeToFileTime( 0, 0, &ftColDate );
             }
             else
             {
               SYSTEMTIME sysDate;     // date converted to SYSTEMTIME format

               sysDate.wYear         = (WORD)(pTimeDate->tm_year + 1900);
               sysDate.wMonth        = (WORD)(pTimeDate->tm_mon  + 1);
               sysDate.wDayOfWeek    = (WORD)(pTimeDate->tm_wday);
               sysDate.wDay          = (WORD)(pTimeDate->tm_mday);
               sysDate.wHour         = 0;
               sysDate.wMinute       = 0;
               sysDate.wSecond       = 0;
               sysDate.wMilliseconds = 0;

               SystemTimeToFileTime( &sysDate, &ftColDate );
             } /* endif */
           }

           // compare dates
           iResult = (int)CompareFileTime( &ftColDate, &ftFilterDate );
         }
         break;

       case DATETIME_DATA:
         // convert values to Win32 FILETIME and compare it
         {
           FILETIME ftFilterDate;      // date specified in filter
           FILETIME ftColDate;         // date listed in column

           // convert filter date
           {
             WORD  wDate;                // date specified in filter converted to FDATE
             WORD  wTime;                // time specified in filter converted to FDATE
             PSZ pszSplitPos;


             pszSplitPos = strchr( m_Filter.aszValue[iPos], ' ' );
             if ( pszSplitPos )
             {
               UtlTimeStringToFTime( pszSplitPos + 1, (FTIME *)&wTime );
               *pszSplitPos = EOS;
             }
             else
             {
               wTime = 0;
             } /* endif */
             UtlDateStringToFDate( m_Filter.aszValue[iPos], (FDATE *)&wDate );
             if ( pszSplitPos ) *pszSplitPos = ' ';
             DosDateTimeToFileTime( wDate, wTime, &ftFilterDate );
           }

           // convert column date
           {
             LONG  lDate;                // date of column as LONG value
             struct tm  *pTimeDate;      // time/date structure

             lDate = atol( szColData );
             if ( lDate ) lDate += 10800L;// correction: + 3 hours (why???)

             pTimeDate = localtime( (time_t *)&lDate );

             if ( (lDate == 0L) || (pTimeDate == NULL) )
             {
               DosDateTimeToFileTime( 0, 0, &ftColDate );
             }
             else
             {
               SYSTEMTIME sysDate;     // date converted to SYSTEMTIME format

               sysDate.wYear         = (WORD)(pTimeDate->tm_year + 1900);
               sysDate.wMonth        = (WORD)(pTimeDate->tm_mon  + 1);
               sysDate.wDayOfWeek    = (WORD)(pTimeDate->tm_wday);
               sysDate.wDay          = (WORD)(pTimeDate->tm_mday);
               sysDate.wHour         = (WORD)(pTimeDate->tm_hour);
               sysDate.wMinute       = (WORD)(pTimeDate->tm_min);
               sysDate.wSecond       = (WORD)(pTimeDate->tm_sec);
               sysDate.wMilliseconds = 0;

               SystemTimeToFileTime( &sysDate, &ftColDate );
             } /* endif */
           }

           // compare dates
           iResult = (int)CompareFileTime( &ftColDate, &ftFilterDate );
         }
         break;


       case FDATE_DATA:
         // convert values to Win32 FILETIME and compare it
         {
           FILETIME ftFilterDate;      // date specified in filter
           FILETIME ftColDate;         // date listed in column

           // convert filter date
           {
             WORD  wDate;                // date specified in filter converted to FDATE
             UtlDateStringToFDate( m_Filter.aszValue[iPos], (FDATE *)&wDate );
             DosDateTimeToFileTime( wDate, 0, &ftFilterDate );
           }

           // convert column date
           {
             WORD wDate = (WORD)(atoi( szColData ));
             DosDateTimeToFileTime( wDate, 0, &ftColDate );
           }

           // compare dates
           iResult = (int)CompareFileTime( &ftColDate, &ftFilterDate );
         }
         break;

       case FTIME_DATA:
         // convert value to FTIME value and comapre wit col data
         {
           WORD wFilterTime;
           WORD wColTime;

           UtlTimeStringToFTime( m_Filter.aszValue[iPos], (FTIME *)&wFilterTime );
           wColTime = (WORD)(atoi( szColData ));

           if ( wFilterTime < wColTime )
           {
             iResult = 1;
           }
           else if ( wFilterTime > wColTime )
           {
             iResult = -1;
           }
           else
           {
             iResult = 0;
           } /* endif */
         }
         break;

       case FDATETIME_DATA:
         // convert values to Win32 FILETIME and compare it
         {
           FILETIME ftFilterDate;      // date specified in filter
           FILETIME ftColDate;         // date listed in column

           // convert filter date
           {
             WORD  wDate;                // date specified in filter converted to FDATE
             WORD  wTime;                // time specified in filter converted to FDATE
             PSZ pszSplitPos;


             pszSplitPos = strchr( m_Filter.aszValue[iPos], ' ' );
             if ( pszSplitPos )
             {
               UtlTimeStringToFTime( pszSplitPos + 1, (FTIME *)&wTime );
               *pszSplitPos = EOS;
             }
             else
             {
               wTime = 0;
             } /* endif */
             UtlDateStringToFDate( m_Filter.aszValue[iPos], (FDATE *)&wDate );
             if ( pszSplitPos ) *pszSplitPos = ' ';
             DosDateTimeToFileTime( wDate, wTime, &ftFilterDate );
           }

           // convert column date
           {
              WORD wDate;
              WORD wTime;
              PSZ  pszTemp;

              wDate = (WORD)(atoi( szColData ));
              pszTemp = strchr( szColData, '.' );
              if ( pszTemp )
              {
                 wTime = (WORD)(atoi( pszTemp + 1 ));
              }
              else
              {
                 wTime = 0;
              } /* endif */
              DosDateTimeToFileTime( wDate, wTime, &ftColDate );
           }

           // compare dates
           iResult = (int)CompareFileTime( &ftColDate, &ftFilterDate );
         }
         break;

       case NUMERIC_DATA:
       default:
          // nothing to do here, use standard compare function
          pszFilterValue = m_Filter.aszValue[iPos];
          iResult = CLBCompareSingleColum( usColumnType[m_Filter.asColumn[iPos]], szColData, pszFilterValue );
          break;
    } /* endswitch */

    // check if condition is satified
    switch ( m_Filter.asOperator[iPos] )
    {
      case EQUAL_CLBOPID:
        fShowItem = (iResult == 0);
        break;

      case NOTEQUAL_CLBOPID:
        fShowItem = (iResult != 0);
        break;

      case LESS_CLBOPID:
        fShowItem = (iResult < 0);
        break;

      case LESSOREQUAL_CLBOPID:
        fShowItem = (iResult <= 0);
        break;

      case GREATER_CLBOPID:
        fShowItem = (iResult > 0);
        break;

      case GREATEROREQUAL_CLBOPID:
        fShowItem = (iResult >= 0);
        break;

      case LIKE_CLBOPID :
        // use pattern matching
        UtlMatchStrings( szColData, m_Filter.aszValue[iPos], &fShowItem );
        break;

      default:
        // unknown condition operator
        break;
    } /* endswitch */

    // continue with next condition
    iPos++;
  } /* endwhile */
  return( fShowItem );
}

int CListViewEx::InsertRow( PSZ pszBuffer, int iItem, SHORT sImage )
{
  int iInsertedItem;                 // index of inserted item
  BOOL fShowItem = TRUE;               // does item match active filter?

  fShowItem = CheckFilterMatch( pszBuffer );

  /*******************************************************/
  /* insert item                                         */
  /*******************************************************/
  if ( fShowItem )
  {
    PSZ pData = new char[ strlen( pszBuffer ) + CLB_300_OFFSET + 1 ];
    memset( pData, 0, CLB_300_OFFSET + 1 );
    strcpy( pData+CLB_300_OFFSET, pszBuffer );
    CListCtrl& ListCtrl = GetListCtrl();
    LV_ITEM lvi;
  //  SHORT iItem = 0;
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
    lvi.iSubItem = 0;

    // prepare text of first column
    PrepareNameColumn( asCurView[0],
                       UtlParseX15( pszBuffer,
                                    asCurView[0] & CLB_COLNUMBERMASK ),
                       chItemBuffer );
    lvi.pszText = chItemBuffer;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.state = INDEXTOSTATEIMAGEMASK(1);
    lvi.lParam = (LONG) pData;
    if ( iItem == -1 )
    {
      /******************************************************************/
      /* insert at the end, i.e. get rid of LVS_SORTASCENDING           */
      /******************************************************************/
      ModifyStyle( LVS_SORTASCENDING, 0 );
      iItem = ListCtrl.GetItemCount();
    } /* endif */

    lvi.iItem = iItem;
    lvi.iImage = sImage;

    iInsertedItem = ListCtrl.InsertItem(&lvi);
    InsertColumnItems( iInsertedItem, pszBuffer );
  }
  else
  {
    iInsertedItem = CB_ERR;
  } /* endif */

  return iInsertedItem;
}

VOID CListViewEx::InsertColumnItems( int iInsertedItem, PSZ pszBuffer )
{
  int i = 1;
  int j;
// while ( (i<21)&&((j = asCurView[i]) != -1 ) )
   while ( (i<MAX_VIEW+1)&&((j = asCurView[i]) != -1 ) )
  {
    PSZ    pszColText = UtlParseX15( pszBuffer, (SHORT)(j & CLB_COLNUMBERMASK) );
    PSZ    pszTemp;       // general work pointer
    LONG   lDate;
    CHAR   chDate[50]        ;     // buffer for date string
    FDATE  FDate;                  // date in FDATE format
    FTIME  FTime;                      // time in FTIME format
    ULONG  ulLen;

    int  iPathModeFlags = j & CLB_PATHMODEMASK;
    j &= CLB_COLNUMBERMASK;

    switch ( usColumnType[j] )
    {
       case DATE_DATA:
          lDate = atol( pszColText );
          UtlLongToDateString( lDate, chDate, sizeof(chDate) );
          pszColText = chDate;
          break;

       case TIME_DATA:
          lDate = atol( pszColText );
          UtlLongToTimeString( lDate, chDate, sizeof(chDate) );
          pszColText = chDate;
          break;

       case DATETIME_DATA:
          lDate = atol( pszColText );
          UtlLongToDateString( lDate, chDate, sizeof(chDate) );
          ulLen = strlen(chDate);
          chDate[ulLen] = BLANK;
          UtlLongToTimeString( lDate, chDate + ulLen + 1,
                               (USHORT)(sizeof(chDate) - ulLen - 1) );
          pszColText = chDate;
          break;

       case FDATE_DATA:
          *((PUSHORT)&FDate) = (USHORT)(atoi( pszColText ));
          UtlFDateToDateString( &FDate, chDate, sizeof(chDate) );
          pszColText = chDate;
          break;

       case FTIME_DATA:
          *((PUSHORT)&FTime) = (USHORT)(atoi( pszColText ));
          UtlFTimeToTimeString( &FTime, chDate, sizeof(chDate) );
          pszColText = chDate;
          break;

       case FDATETIME_DATA:
          *((PUSHORT)&FDate) = (USHORT)(atoi( pszColText ));
          pszTemp = strchr( pszColText, '.' );
          if ( pszTemp )
          {
             *((PUSHORT)&FTime) = (USHORT)(atoi( pszTemp + 1 ));
          }
          else
          {
             *((PUSHORT)&FTime) = 0;
          } /* endif */
          UtlFDateToDateString( &FDate, chDate, sizeof(chDate) );
          ulLen = strlen(chDate);
          chDate[ulLen] = BLANK;
          UtlFTimeToTimeString( &FTime, chDate + ulLen + 1,
                               (sizeof(chDate) - ulLen - 1) );
          pszColText = chDate;
          break;

       case TEXT_DATA:
       case AUTOWIDTHTEXT_DATA:
       default:
          // handle path mode flags
          PrepareNameColumn( iPathModeFlags, pszColText, chItemBuffer);
          pszColText = chItemBuffer;
          break;
    } /* endswitch */
    GetListCtrl().SetItemText(iInsertedItem, i, pszColText );
    pszBuffer[strlen(pszBuffer)] = X15;
    i++;
  } /* endwhile */
}

BOOL  CListViewEx::SetItemState( int iItem, USHORT usState )
{
  return (BOOL) SetItemData( iItem, (LONG) !usState );
}

USHORT  CListViewEx::GetItemState( int iItem )
{
  return (USHORT) ! GetItemData( iItem );
}

LRESULT  CListViewEx::OnSetItemText( WPARAM mp1, LPARAM mp2 )
{

  int iItem = (SHORT) mp1;
  PSZ pszItemText = (PSZ) mp2;

  // get rid off current column data
  CListCtrl& ListCtrl = GetListCtrl();
  free( (PSZ) ListCtrl.GetItemData( iItem ));

  // add new column data
  PSZ pData = new char[ strlen( pszItemText ) + CLB_300_OFFSET + 1 ];
//  UtlAlloc( &pData, strlen( pszItemText ) + CLB_300_OFFSET + 1, 0L, ERROR_STORAGE );
  memset( pData, 0, CLB_300_OFFSET + 1 );
  strcpy( pData+CLB_300_OFFSET, pszItemText );
  ListCtrl.SetItemData( iItem, (LONG)pData );

  // show new column text
  InsertColumnItems( iItem, pszItemText );

  return (LRESULT) TRUE;
}



VOID  CListViewEx::SelectAll()
{
  CListCtrl& ListCtrl = GetListCtrl();
  int   iNumItems = ListCtrl.GetItemCount();
  int   iItem;

  for ( iItem=0; iItem < iNumItems; iItem++ )
  {
    ListCtrl.SetItemState( iItem, LVIS_SELECTED, LVIS_SELECTED);
  } /* endfor */
}

VOID  CListViewEx::DeSelectAll()
{
  CListCtrl& ListCtrl = GetListCtrl();
  int   iNumItems = ListCtrl.GetItemCount();
  int   iItem;

  for ( iItem=0; iItem < iNumItems; iItem++ )
  {
    ListCtrl.SetItemState( iItem, (UINT)~LVIS_SELECTED, (UINT)LVIS_SELECTED);
  } /* endfor */
}

/**********************************************************************/
/* Get selection                                                      */
/**********************************************************************/
int CListViewEx::QuerySelection( )
{
  CListCtrl&  ListCtrl = GetListCtrl();
  int  iItem;

  iItem = ListCtrl.GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );

  return ( iItem);
}

/**********************************************************************/
/* Set selection                                                      */
/**********************************************************************/
BOOL  CListViewEx::SetSelection( int iItem )
{
  CListCtrl&  ListCtrl = GetListCtrl();
  if ( iItem == -1 )
  {
    return (ListCtrl.SetItemState( iItem, LVIS_SELECTED, 0 ));
  }
  else
  {
    return (ListCtrl.SetItemState( iItem, LVIS_SELECTED | LVIS_FOCUSED,
                                          LVIS_SELECTED | LVIS_FOCUSED));
  } /* endif */
}

/**********************************************************************/
/* Set top index                                                      */
/**********************************************************************/
BOOL  CListViewEx::SetTopIndex( int iItem )
{
  CListCtrl&  ListCtrl = GetListCtrl();
  return (ListCtrl.EnsureVisible( iItem, 0));
}
/**********************************************************************/
/* Clear the ColumnList box (header titles and entries)               */
/**********************************************************************/
VOID  CListViewEx::ClearCLBox()
{
  CListCtrl& ListCtrl = GetListCtrl();
  /********************************************************************/
  /* delete all columns                                               */
  /********************************************************************/
  BOOL fOK = TRUE;
  while ( fOK )
  {
    fOK =  ListCtrl.DeleteColumn(0);
  } /* endwhile */

  /**********************************************************************/
  /* Delete all of the items from the list view control.                */
  /**********************************************************************/
  int nCount = ListCtrl.GetItemCount();
  for (int i=0;i < nCount;i++)
  {
    ListCtrl.DeleteItem(0);
  }
}

VOID  CListViewEx::UpdateCLBox()
{
  CListCtrl& ListCtrl = GetListCtrl();

  // remember width of first column
  int iWidthColumn0 = ListCtrl.GetColumnWidth( 0 );


  /********************************************************************/
  /* delete all columns                                               */
  /********************************************************************/
  BOOL fOK = TRUE;
  while ( fOK )
  {
    fOK =  ListCtrl.DeleteColumn(0);
  } /* endwhile */

  /********************************************************************/
  /* insert the header                                                */
  /********************************************************************/
  InsertTitle();
  /********************************************************************/
  /* insert the rows                                                  */
  /********************************************************************/
  int iNumItems = ListCtrl.GetItemCount();
  int   iItem;
  for ( iItem=0; iItem < iNumItems; iItem++ )
  {
    LV_ITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    if ( ListCtrl.GetItem( &lvi ) )
    {
      InsertColumnItems( iItem, (PSZ) (lvi.lParam+CLB_300_OFFSET) );
      // update text of item (first displayed column)
      PrepareNameColumn( asCurView[0],
                         UtlParseX15( (PSZ)(lvi.lParam+CLB_300_OFFSET),
                                      asCurView[0] & CLB_COLNUMBERMASK ),
                         chItemBuffer );
      ((PSZ) (lvi.lParam + CLB_300_OFFSET))[strlen((PSZ) (lvi.lParam + CLB_300_OFFSET))] = X15;
      ListCtrl.SetItemText( iItem, 0, chItemBuffer );
    } /* endif */
  } /* endfor */
  /********************************************************************/
  /* force update of window                                           */
  /********************************************************************/

  // set width of first column
  if ( iWidthColumn0 ) ListCtrl.SetColumnWidth( 0, iWidthColumn0 );


  if (asCurView[1] == -1  )
  {
    Invalidate();
    UpdateWindow();
  } /* endif */
}


void CListViewEx::OnDetailsDlg()
{

  CDetailView detailDlg;

  memcpy( detailDlg.asDetailsView, GetDetailsView(),
            sizeof(detailDlg.asDetailsView) );
  memcpy( detailDlg.ColumnHeader, GetColumnHeader(),
          sizeof(detailDlg.ColumnHeader) );
  memcpy( detailDlg.usColumnWidth, GetColumnWidth(),
          sizeof(detailDlg.usColumnWidth) );
  detailDlg.sPopupID = this->sPopUpId;

  switch ( detailDlg.DoModal() )
  {
    case IDOK:
      /****************************************************************/
      /* force an update of the columns                               */
      /****************************************************************/
      SetDetailsView( detailDlg.asDetailsView );
      OnDetailsView();
      break;
    case IDCANCEL:
      break;
    default:
      break;
  } /* endswitch */
  return;
}


VOID CListViewEx::OnSortDlg()
{
  CSortDlg dlgSort;

  memcpy( dlgSort.asSortView, GetSortView(),
            sizeof(dlgSort.asSortView) );
  memcpy( dlgSort.ColumnHeader, GetColumnHeader(),
          sizeof(dlgSort.ColumnHeader) );
  memcpy( dlgSort.usColumnWidth, GetColumnWidth(),
          sizeof(dlgSort.usColumnWidth) );

  switch ( dlgSort.DoModal() )
  {
    case IDOK:
      /****************************************************************/
      /* force an update of the columns                               */
      /****************************************************************/
      SetSortView( dlgSort.asSortView );
      Sort();
      break;
    case IDCANCEL:
      break;
    default:
      break;
  } /* endswitch */
  return;
}

void CListViewEx::OnFilterDlg()
{
  CFilterDlg dlgFilter;

  memcpy( dlgFilter.ColumnHeader, GetColumnHeader(),
          sizeof(dlgFilter.ColumnHeader) );
  memcpy( dlgFilter.usColumnWidth, GetColumnWidth(),
          sizeof(dlgFilter.usColumnWidth) );
  memcpy( dlgFilter.usColumnWidth, GetColumnWidth(),
          sizeof(dlgFilter.usColumnWidth) );
  QueryFilter( &(dlgFilter.Filter) );

  switch ( dlgFilter.DoModal() )
  {
    case IDOK:
      SetFilter( &(dlgFilter.Filter) );
      Filter();
      break;
    case IDCANCEL:
      break;
    default:
      break;
  } /* endswitch */
  return;
}

void CListViewEx::OnAll()
{
  // discard any filter
  memset( &m_Filter, 0, sizeof(m_Filter) );

  // force a reload of column listbox items
  Filter();

  return;
}

void CListViewEx::OnNamesView()
{
  SetCurrentView( asNameView );
  UpdateCLBox();
  return;
}

void CListViewEx::OnDetailsView()
{
  SetCurrentView( GetDetailsView() );
  UpdateCLBox();
  return;
}


void CListViewEx::OnShrinkPath()
{
  // modify current view
  int i = 0;
  while ( asCurView[i] != CLBLISTEND )
  {
    if ( (asCurView[i] & CLB_COLNUMBERMASK) == 1 ) // name column?
    {
      asCurView[i] = (asCurView[i] & CLB_COLNUMBERMASK) | CLB_SHRINKPATH;
    } /* endif */
    i++;
  } /* endwhile */
  UpdateCLBox();
  return;
}

void CListViewEx::OnHidePath()
{
  // modify current view
  int i = 0;
  while ( asCurView[i] != CLBLISTEND )
  {
    if ( (asCurView[i] & CLB_COLNUMBERMASK) == 1 ) // name column?
    {
      asCurView[i] = (asCurView[i] & CLB_COLNUMBERMASK) | CLB_HIDEPATH;
    } /* endif */
    i++;
  } /* endwhile */
  UpdateCLBox();
  return;
}

void CListViewEx::OnShowPath()
{
  // modify current view
  int i = 0;
  while ( asCurView[i] != CLBLISTEND )
  {
    if ( (asCurView[i] & CLB_COLNUMBERMASK) == 1 ) // name column?
    {
      asCurView[i] = asCurView[i] & CLB_COLNUMBERMASK;
    } /* endif */
    i++;
  } /* endwhile */
  UpdateCLBox();
  return;
}

// prepare text for name column by handling path mode flags
void CListViewEx::PrepareNameColumn( int iColumn, PSZ pszColText, PSZ pszBuffer )
{
  // handle path mode flags
  if ( iColumn & CLB_HIDEPATH )
  {
    // skip any path part of name
    PSZ pszTemp = strrchr( pszColText, BACKSLASH );
    if ( pszTemp )
    {
      strcpy( pszBuffer, pszTemp + 1 );
    }
    else
    {
      strcpy( pszBuffer, pszColText );
    } /* endif */
  }
  else if ( iColumn & CLB_SHRINKPATH )
  {
    // reduce any path part of name
    PSZ pszTemp = strrchr( pszColText, BACKSLASH );
    if ( pszTemp )
    {
      PSZ pszSource = pszColText;
      PSZ pszTarget = pszBuffer;
      int i = 8;
      while ( i && (pszSource < pszTemp) )
      {
        *pszTarget++ = *pszSource++;
        i--;
      } /* endwhile */
      if ( (pszSource + 1) < pszTemp )
      {
        *pszTarget++ = '.';
        *pszTarget++ = '.';
        *pszTarget++ = '.';
      } /* endif */
      strcpy( pszTarget, pszTemp );
    }
    else
    {
      strcpy( pszBuffer, pszColText );
    } /* endif */
  }
  else
  {
    strcpy( pszBuffer, pszColText );
  } /* endif */
}


LRESULT CListViewEx::QueryView( WPARAM mp1, LPARAM mp2 )
{
  switch ( SHORT1FROMMP1(mp1) )
  {
     case CURRENT_VIEW :
       memcpy( (PSHORT) mp2, asCurView, sizeof(asCurView));
       break;
     case DETAILS_VIEW :
       memcpy( (PSHORT) mp2, asDetailsView, sizeof(asDetailsView));
       break;
     case DEFAULT_VIEW :
       memcpy( (PSHORT) mp2, asNameView, sizeof(asNameView));
       break;
     case NAME_VIEW    :
       memcpy( (PSHORT) mp2, asNameView, sizeof(asNameView));
       break;
     case SORT_VIEW    :
       memcpy( (PSHORT) mp2, asSortView, sizeof(asSortView));
       break;
  } /* endswitch */
  return FALSE;
}

LRESULT CListViewEx::SetView( WPARAM mp1, LPARAM mp2 )
{
  mp1; mp2;
  return FALSE;
}

// LRESULT  CListViewEx::QueryFilter( PCLBFILTER pFilter )
// {
//   memcpy( pFilter, &Filter, sizeof(Filter));
//   return( 0 );
// }
//
// LRESULT  CListViewEx::SetFilter( PCLBFILTER pFilter )
// {
//   memcpy( &Filter, pFilter, sizeof(Filter));
//   return( 0 );
// }

VOID CListViewEx::Sort()
{
  CListCtrl& ListCtrl = GetListCtrl();
  CLBCOMPAREITEMDATA SortData;

  SortData.psSortColumns = asSortView;
  SortData.pusColumnType = usColumnType;

  ListCtrl.SortItems( CLBCompareItem, (LPARAM)&SortData );
}

VOID CListViewEx::OnTagTable()
{
  /*********************************************************/
  /* Handle request for tag table list window              */
  /*********************************************************/
  HWND mpHwnd = EqfQueryObject( NULL, clsTAGTABLE, 0 );
  if ( mpHwnd != NULLHANDLE )
  {
     /*****************************************************/
     /* activate existing tag table list                  */
     /*****************************************************/
          ::WinSetFocus( HWND_DESKTOP, mpHwnd );
  }
  else
  {
    /*****************************************************/
    /* start a new tag table list                        */
    /*****************************************************/
    OBJNAME       szObjName;           // buffer for object name
    UtlMakeEQFPath( (PSZ)szObjName, NULC, SYSTEM_PATH, NULL );
    strcat( (PSZ)szObjName, BACKSLASH_STR );
    strcat( (PSZ)szObjName, TAGTABLE_PROPERTIES_NAME );
    EqfSend2Handler( TAGTABLEHANDLER,
                     WM_EQF_OPEN,
                     MP1FROMSHORT(FALSE),
                     MP2FROMP(szObjName) );
    UtlDispatch();                   // wait for object to come up
  } /* endif */
}

VOID CListViewEx::OnMTList()
{
  // Handle request for MT job list
  HWND mpHwnd = EqfQueryObject( NULL, clsMTLIST, 0 );
  if ( mpHwnd != NULLHANDLE )
  {
     // activate existing list
     ::WinSetFocus( HWND_DESKTOP, mpHwnd );
  }
  else
  {
    // start a new MT Job list
    EqfSend2Handler( MTLISTHANDLER,
                     WM_EQF_OPEN,
                     MP1FROMSHORT(FALSE),
                     MP2FROMP("") );
    UtlDispatch();                   // wait for object to come up
  } /* endif */
}

VOID CListViewEx::OnLangList()
{
  /********************************************************/
  /* Handle request for language update window            */
  /********************************************************/
  HWND mpHwnd = EqfQueryObject( NULL, clsLNGUPDATE, 0 );
  if ( mpHwnd != NULLHANDLE )
  {
     /*****************************************************/
     /* activate existing language update window          */
     /*****************************************************/
          ::WinSetFocus( HWND_DESKTOP, mpHwnd );
  }
  else
  {
    /*****************************************************/
    /* start a language properties update list           */
    /*****************************************************/
    EqfSend2Handler( LNGUPDATEHANDLER,
                     WM_EQF_CREATE,
                     MP1FROMSHORT(FALSE),
                     MP2FROMP(EMPTY_STRING) );
    UtlDispatch();                   // wait for object to come up
  } /* endif */
}

VOID CListViewEx::OnExclList()
{
  EqfSend2Handler( LISTHANDLER,
                   WM_EQF_OPEN,
                   MP1FROMSHORT(FALSE),
                   MP2FROMP(EXCLUSIONLISTOBJ) );
}

VOID CListViewEx::OnNewTerms()
{
  EqfSend2Handler( LISTHANDLER,
                   WM_EQF_OPEN,
                   MP1FROMSHORT(FALSE),
                   MP2FROMP(NEWTERMLISTOBJ) );
}

VOID CListViewEx::OnFoundTerms()
{
  EqfSend2Handler( LISTHANDLER,
                   WM_EQF_OPEN,
                   MP1FROMSHORT(FALSE),
                   MP2FROMP(FOUNDTERMLISTOBJ) );
}


VOID CListViewEx::OnPrintList()
{
  CLBPrintList();
  return;
}

VOID CListViewEx::OnSysProps()
{
  TWBSystemProps();
  return;
}

VOID CListViewEx::OnExit()
{
  ::PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ), WM_CLOSE, 0, 0 );
  return;
}

VOID CListViewEx::OnConfDrives()
{
  ((CEQFDoc*)GetDocument())->OnConfDrives();
}

VOID CListViewEx::OnPluginManager()
{
  PluginManagerMenuProps();
}

VOID CListViewEx::OnAutoVersionUp()
{
  AutoVerUpProps(TRUE);
}

VOID CListViewEx::OnConnectResources()
{
  ((CEQFDoc*)GetDocument())->OnConnectResources();
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CLBPrintList                                             |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       Print the contents of the column listbox using the active|
//|                   view list                                                |
//+----------------------------------------------------------------------------+
//|Input parameter:   PCLBIDA       pIda          ptr to column listbox ida    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE       function completed successfully               |
//|                   FALSE      an error occured                              |
//+----------------------------------------------------------------------------+

// size of print line buffer
#define PRINTBUFSIZE        8096

BOOL CListViewEx::CLBPrintList()
{
  BOOL        fOK = TRUE;              // internal O.K. flag and return code
  PSHORT      psViewColumn;            // current column in view column list
  PSZ         pszLine = NULL;          // ptr to buffer for print line
  HPRINT      hPrint = NULLHANDLE;     // print handle
  PSZ         pszBuffer = NULL;
  PSZ_W       pszLineW = NULL;
  PPROPSYSTEM pPropSys = GetSystemPropPtr();


  // allocate print line buffer
  fOK = UtlAlloc( (PVOID *)&pszLine, 0L, (LONG)PRINTBUFSIZE * (sizeof(CHAR_W)+1), ERROR_STORAGE );
  if ( fOK ) fOK = UtlAlloc( (PVOID *)&pszBuffer, 0L, PRINTBUFSIZE+1, ERROR_STORAGE );

  // open print context
  if ( fOK )
  {
    pszLineW = (PSZ_W)(pszLine+PRINTBUFSIZE);
    GetWindowText( pszLine, PRINTBUFSIZE );
    fOK = UtlPrintOpen( &hPrint, pszLine, GETPARENT(m_hWnd) );
  } /* endif */

  // print header
  if ( fOK )
  {
    strcat( pszLine, "\n" );
    fOK = UtlPrintLine( hPrint, pszLine );
    strcpy( pszLine, "\n" );
    fOK = UtlPrintLine( hPrint, pszLine );
  } /* endif */

  // print column title line
  if ( fOK )
  {
    USHORT      usCurColStart;         // start of current column in output line
    USHORT      usActColStart;         // actual start of column text (may be
    int         iItem;
    USHORT      i = 0;

    usCurColStart = 0;                 // we start at begin of pszLine
    usActColStart = 0;                 // we start at begin of pszLine

    while ( (i<MAX_VIEW) && ((iItem = asCurView[i]) != CLBLISTEND) )
    {
      iItem &= CLB_COLNUMBERMASK;

      // Filter column title
      PSZ pszSource = ColumnHeader[iItem];
      PSZ pszTarget = pszBuffer;
      while ( *pszSource && ((pszTarget - pszBuffer) < (PRINTBUFSIZE - 1)))
      {
        if ( *pszSource == CLB_START_COMMENT )
        {
          while ( *pszSource && (*pszSource != CLB_END_COMMENT ) )
          {
            if ( IsDBCS_CP(pPropSys->ulSystemPrefCP) &&
                 (isdbcs1ex((USHORT)pPropSys->ulSystemPrefCP, *pszSource) == DBCS_1ST) )
            {
              // skip both bytes of DBCS character
              pszSource++;
              if ( *pszSource ) pszSource++;
            }
            else
            {
              pszSource++;
            } /* endif */
          } /* endwhile */
          if ( *pszSource == CLB_END_COMMENT )
          {
            pszSource++;
          } /* endif */
        }
        else
        {
          if ( IsDBCS_CP(pPropSys->ulSystemPrefCP) &&
               (isdbcs1ex((USHORT)pPropSys->ulSystemPrefCP, *pszSource) == DBCS_1ST) )
          {
            // copy both DBCS bytes
            *pszTarget++ = *pszSource++;
            if ( *pszSource ) *pszTarget++ = *pszSource++;
          }
          else
          {
            *pszTarget++ = *pszSource++;
          } /* endif */
        } /* endif */
      } /* endwhile */
      *pszTarget = EOS;

      // fill any gaps in front of column with blanks
      while ( usActColStart < usCurColStart )
      {
        pszLine[usActColStart++] = BLANK;
      } /* endif */

      // right adjust data if required
      if ( usColumnAdjust[iItem] & DT_RIGHT )
      {
        ULONG ulDataLen = strlen(pszBuffer);
        if ( (ULONG)(usActColStart + ulDataLen) < (ULONG)(usCurColStart + usColumnWidth[iItem]) )
        {
          ULONG  ulPadChars = usCurColStart + usColumnWidth[iItem] -
                              usActColStart - ulDataLen;
          while ( ulPadChars )
          {
            pszLine[usActColStart++] = BLANK;
            ulPadChars--;
          } /* endwhile */
        } /* endif */
      } /* endif */

      // insert title of current column
      {
        PSZ pszTextTemp = pszBuffer;
        while ( *pszTextTemp )
        {
          pszLine[usActColStart++] = *pszTextTemp++;
        } /* endwhile */
        pszLine[usActColStart++] = BLANK;
      }

      usCurColStart += usColumnWidth[iItem] + 1;   // add with of current column
      i++;
    } /* endwhile */

    pszLine[usActColStart++] = LF;
    pszLine[usActColStart++] = EOS;

    ANSITOOEM( pszLine );
    fOK = UtlPrintLine( hPrint, pszLine );
  } /* endif */

  // print all items of column listbox
  if ( fOK )
  {
    CListCtrl& ListCtrl = GetListCtrl();
    LV_ITEM lvi;
    memset( &lvi, 0, sizeof( lvi ));
    lvi.mask      = LVIF_PARAM;

    int iNoOfItems = ListCtrl.GetItemCount();

    int   iItem = 0;                   // start with first item
    PSZ   pszColText;                  // ptr to text for a specific column
    CHAR  chDate[50];                  // buffer for date in ASCII format
    USHORT usCurColStart;              // start of current column in output line
    USHORT usActColStart;              // actual start of column text (may be
                                       // larger than usCurColPos if the data for
                                       // a column exceeded the width of the
                                       // column)
    PSZ    pDataPtr[MAX_DEFINEDCOLUMNS];  /* 8-20-14 */


    // loop over all listbox items
    while ( iItem < iNoOfItems )
    {
      lvi.iItem     = iItem;
      lvi.iSubItem  = 0;
      lvi.pszText   = NULL;
      lvi.cchTextMax = 0;
      lvi.stateMask = 0xFFFF;
      ListCtrl.GetItem( &lvi );

      PSZ pszData = (PSZ) lvi.lParam;
      if ( pszData != NULL )
      {
        strcpy( pszBuffer, pszData + 4 );
      }
      else
      {
        pszBuffer[0] = EOS;
      } /* endif */

      // setup column data pointers
      {
        PSZ   pEndOfColumn;            // ptr to end of column data
        PSZ   pColumnStart;            // ptr to start of column data
        PSZ   *pColData;               // current position in data ptr table
        USHORT usI;                    // loop index

        pColData = pDataPtr;        // start with first string ptr
        pColumnStart = pszBuffer;       // set column data start pointer
        for ( usI = 0; usI < MAX_DEFINEDCOLUMNS; usI++ ) /* 8-20-14 */
        {
           pEndOfColumn = strchr( pColumnStart, 0x15 );
           if ( ! pEndOfColumn )
              break;
           *pEndOfColumn = '\0';

           *pColData++ = pColumnStart;
           pColumnStart = ( pEndOfColumn ) ? (pEndOfColumn + 1) :
                             (pColumnStart + strlen( pColumnStart ) );
        } /* endfor */
      }

      // add data of selected columns to output buffer
      psViewColumn = &asCurView[0];
      usCurColStart = 0;               // we start at begin of pszLine
      usActColStart = 0;               // we start at begin of pszLine
      USHORT      i = 0;
      while ( (i<MAX_VIEW) && (*psViewColumn != CLBLISTEND) )
      {
        SHORT sColumn = *psViewColumn & CLB_COLNUMBERMASK;
        switch ( usColumnType[sColumn] )
        {
           case DATE_DATA:
              {
                LONG lDate = atol(*(pDataPtr + sColumn));
                UtlLongToDateString( lDate, chDate, sizeof(chDate) );
                pszColText = chDate;
              }
              break;

           case TIME_DATA:
              {
                LONG lDate = atol(*(pDataPtr + sColumn));
                UtlLongToTimeString( lDate, chDate, sizeof(chDate) );
                pszColText = chDate;
              }
              break;

           case DATETIME_DATA:
              {
                ULONG ulLen;
                LONG lDate = atol(*(pDataPtr + sColumn));
                UtlLongToDateString( lDate, chDate, sizeof(chDate) );
                ulLen = strlen(chDate);
                chDate[ulLen] = BLANK;
                UtlLongToTimeString( lDate, chDate + ulLen + 1,
                                     (USHORT)(sizeof(chDate) - ulLen - 1) );
                pszColText = chDate;
              }
              break;

           case FDATE_DATA:
              {
                FDATE FDate;          // date in FDATE format

                *((PUSHORT)&FDate) = (USHORT)atoi(*(pDataPtr + sColumn));
                UtlFDateToDateString( &FDate, chDate, sizeof(chDate) );
                pszColText = chDate;
              }
              break;

           case FTIME_DATA:
              {
                FTIME FTime;          // time in FTIME format

                *((PUSHORT)&FTime) = (USHORT)atoi(*(pDataPtr + sColumn));
                UtlFTimeToTimeString( &FTime, chDate, sizeof(chDate) );
                pszColText = chDate;
              }
              break;

           case FDATETIME_DATA:
              {
                ULONG ulLen;
                FDATE FDate;          // date in FDATE format
                FTIME FTime;          // time in FTIME format
                PSZ pszTemp;          // temporary data pointer

                *((PUSHORT)&FDate) = (USHORT)atoi(*(pDataPtr + sColumn));
                pszTemp = strchr( *(pDataPtr + sColumn), '.' );
                if ( pszTemp )
                {
                   *((PUSHORT)&FTime) = (USHORT) atoi( pszTemp + 1 );
                }
                else
                {
                   *((PUSHORT)&FTime) = 0;
                } /* endif */
                UtlFDateToDateString( &FDate, chDate, sizeof(chDate) );
                ulLen = strlen(chDate);
                chDate[ulLen] = BLANK;
                UtlFTimeToTimeString( &FTime, chDate + ulLen + 1,
                                     (sizeof(chDate) - ulLen - 1) );
                pszColText = chDate;
              }
              break;

           case TEXT_DATA:
           case AUTOWIDTHTEXT_DATA:
           default:
              pszColText = *(pDataPtr + sColumn);
              break;
        } /* endswitch */

        // add data of column to output line
        {
          // fill any gaps in front of column with blanks
          while ( usActColStart < usCurColStart )
          {
            pszLine[usActColStart++] = BLANK;
          } /* endif */

          // right adjust data if required
          if ( usColumnAdjust[ sColumn ] & DT_RIGHT )
          {
            ULONG ulDataLen = strlen(pszColText);
            if ( (ULONG)(usActColStart + ulDataLen) < (ULONG)(usCurColStart + usColumnWidth[sColumn]) )
            {
              ULONG ulPadChars = usCurColStart + usColumnWidth[sColumn] -
                                  usActColStart - ulDataLen;
              while ( ulPadChars )
              {
                pszLine[usActColStart++] = BLANK;
                ulPadChars--;
              } /* endwhile */
            } /* endif */
          } /* endif */

          // insert data of current column
          {
            PSZ pszTextTemp = pszColText;
            while ( *pszTextTemp )
            {
              pszLine[usActColStart++] = *pszTextTemp++;
            } /* endwhile */
            pszLine[usActColStart++] = BLANK;
          }
          usCurColStart += usColumnWidth[sColumn] + 1; // add width of current column
        }

        // next column
        psViewColumn++;
        i++;
      } /* endwhile */

      // print current item line
      pszLine[usActColStart++] = LF;
      pszLine[usActColStart++] = EOS;

      ANSITOOEM( pszLine );
      fOK = UtlPrintLine( hPrint, pszLine );
      // continue with next item
      iItem++;
    } /* endwhile */
  } /* endif */

  // close print context and cleanup
  if ( hPrint )          UtlPrintClose( hPrint );
  if ( pszLine != NULL ) UtlAlloc( (PVOID *)&pszLine, 0L, 0L, NOMSG );
  if ( pszBuffer != NULL ) UtlAlloc( (PVOID *)&pszBuffer, 0L, 0L, NOMSG );

  return( fOK );
}
/* end of CLBPrintList */

void CListViewEx::OnHeaderClick(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
        NM_LISTVIEW *pNm = (NM_LISTVIEW *) pNMHDR;
        LV_DISPINFO *pLvdi = (LV_DISPINFO *) pNMHDR;
        // TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
        id;
        switch ( pLvdi->hdr.code )
        {
          case LVN_COLUMNCLICK:
            {
              SHORT asNewSortView[ 21 ];
              BOOL fDescending = FALSE;
              SHORT sClickedColumn;

              // get clicked column
              {
                int i;
                PSHORT psCurView = GetCurrentView();
                i = pNm->iSubItem;
                sClickedColumn = psCurView[i];
              }

              // get current sort view and check if
              // the clicked column is already the active sort
              // column
              {
                SHORT sCurSortColumn;

                QuerySortView( asNewSortView );
                sCurSortColumn = asNewSortView[0] & ~SORT_DESCENDING_FLAG;
                if ( (sCurSortColumn == sClickedColumn) &&
                     (asNewSortView[1] == CLBLISTEND) )
                {
                  fDescending = !((asNewSortView[0] & SORT_DESCENDING_FLAG) != 0);
                } /* endif */
              }

              // setup new sort list
              memset( asNewSortView, 0, sizeof(asNewSortView) );
              asNewSortView[0] = sClickedColumn;
              if ( fDescending )
              {
                asNewSortView[0] |= SORT_DESCENDING_FLAG;
              } /* endif */
              asNewSortView[1] = CLBLISTEND;
              SetSortView( asNewSortView );
              Sort();
            }
            break;
          default:
            break;
        }
        *pResult = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CDetailView dialog


CDetailView::CDetailView(CWnd* pParent /*=NULL*/)
        : CDialog(CDetailView::IDD, pParent)
{
        //{{AFX_DATA_INIT(CDetailView)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT
  m_pToolTip = NULL;
  this->sPopupID = -1;
}

CDetailView::~CDetailView()
{
  delete m_pToolTip;
}

BEGIN_MESSAGE_MAP(CDetailView, CDialog)
        //{{AFX_MSG_MAP(CDetailView)
        ON_LBN_DBLCLK(ID_SELVIEW_AVAIL_LB,  OnDblclkAvail)
        ON_LBN_DBLCLK(ID_SELVIEW_SELECT_LB, OnDblclkSelect)
        ON_BN_CLICKED(ID_SELVIEW_SELECT_PB, OnSelect)
        ON_BN_CLICKED(ID_SELVIEW_DESELECT_PB, OnDeselect)
        ON_BN_CLICKED(ID_SELVIEW_UP_PB, OnUp )
        ON_BN_CLICKED(ID_SELVIEW_DOWN_PB, OnDown )
        ON_BN_CLICKED(ID_SELVIEW_SAVEDEF_PB, OnSaveDefault )
        ON_BN_CLICKED(ID_SELVIEW_LOADDEF_PB, OnLoadDefault )
        ON_BN_CLICKED(ID_SELVIEW_VIEW_PB, OnView)
        ON_BN_CLICKED(ID_SELVIEW_HELP_PB, OnHelp)
        ON_BN_CLICKED(ID_SELVIEW_CLEAR_PB, OnClear)
        ON_BN_CLICKED(ID_SELVIEW_CANCEL_PB, OnCancel)
        ON_MESSAGE( DM_GETDEFID, OnDefID )
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDetailView message handlers

BOOL CDetailView::OnInitDialog()
{
  // adjust header depending on type of view

  // LOADSTRING( NULLHANDLE, hResMod, pColData->usId,   ColumnHeader[i]);
  switch( this->sPopupID )
  {
    case ID_FOL_POPUP:
      SetWindowText( "Change document list view details" );
      break;
    case ID_DIC_POPUP:
      SetWindowText( "Change dictionary list view details" );
      break;
    case ID_MEM_POPUP:
      SetWindowText( "Change translation memory list view details" );
      break;
    case ID_TAG_POPUP:
      SetWindowText( "Change markup table list view details" );
      break;
    case ID_FLL_POPUP:
      SetWindowText( "Change folder list view details" );
      break;
    case ID_MT_POPUP:
      SetWindowText( "Change MT job list view details" );
      break;
    case ID_LST_POPUP:
      SetWindowText( "Change terminology list view details" );
      break;
  }

  // fill the available listbox and ignore already selected columns

  RefillListboxes( this->asDetailsView );

  CListBox * pLBoxAvail = (CListBox*)GetDlgItem( ID_SELVIEW_AVAIL_LB );
  pLBoxAvail->SetFocus();

  // change our buttons to bitmap buttons
  MakeBitmapButton( this, ID_SELVIEW_SELECT_PB, OBM_RGARROW );
  MakeBitmapButton( this, ID_SELVIEW_DESELECT_PB, OBM_LFARROW );
  MakeBitmapButton( this, ID_SELVIEW_UP_PB, OBM_UPARROW );
  MakeBitmapButton( this, ID_SELVIEW_DOWN_PB, OBM_DNARROW );

  //Set up the tooltip
  m_pToolTip = new CToolTipCtrl;
  if( !m_pToolTip->Create(this) )
  {
      return TRUE;
  }

  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_AVAIL_LB ),"Available view columns, double-click entry to move it to the selected view columns");
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_SELECT_LB  ),"Selected view columns, double-click entry to remove it from the selected view columns");
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_SELECT_PB  ),"Move selected colum to the list of selected view columns" );
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_DESELECT_PB  ),"Remove selectede item(s) from the list of selected view columns" );
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_UP_PB ), "Shift the selected columns one position up" ); 
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_DOWN_PB ), "Shift the selected columns one positon down" ); 
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_LOADDEF_PB ), "Load default view columns" );
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_SAVEDEF_PB ), "Save currently selected view columns as default" );
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_VIEW_PB ), "Change the details view using the selected view columns" );
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_CLEAR_PB ), "Remove all selected view columns except for the name columns" );
  m_pToolTip->AddTool( GetDlgItem( ID_SELVIEW_CANCEL_PB ), "Leave dialog and discard any changes" );

  m_pToolTip->Activate(TRUE);

  return /*TRUE*/ FALSE;
}

BOOL CDetailView::PreTranslateMessage(MSG* pMsg)
{
  if (NULL != m_pToolTip)
      m_pToolTip->RelayEvent(pMsg);

  return CDialog::PreTranslateMessage(pMsg);
}

void CDetailView::OnSelect()
{  // should do the same as dblclick on an item in "available" listbox
  CDetailView::OnDblclkAvail();
}

void CDetailView::OnUp()
{
  CListBox *pLBoxSelected = (CListBox*)GetDlgItem( ID_SELVIEW_SELECT_LB );
  if ( pLBoxSelected == NULL ) return;

  int iNumOfItems = pLBoxSelected->GetCount();
  for( int i = 0; i < iNumOfItems; i++)
  {
    if ( pLBoxSelected->GetSel( i ) )
    {
      // swap the current listbox item with the previous item
      if (i > 0 && !pLBoxSelected->GetSel( i - 1 ))
      {
        CString itemText;
        pLBoxSelected->GetText( i, itemText );
        DWORD_PTR pData = pLBoxSelected->GetItemData( i );
        pLBoxSelected->DeleteString( i );
        pLBoxSelected->InsertString( i - 1, itemText );
        pLBoxSelected->SetItemData( i - 1, pData );
        pLBoxSelected->SetSel( i - 1, TRUE );
      } /* endif */
    } /* endif */
  } /* endfor */
}

void CDetailView::OnDown()
{
  CListBox *pLBoxSelected = (CListBox*)GetDlgItem( ID_SELVIEW_SELECT_LB );
  if ( pLBoxSelected == NULL ) return;

  int iStartIndex = pLBoxSelected->GetCount() - 1;
  for( int i = iStartIndex; i > -1; i-- )
  {
    if ( pLBoxSelected->GetSel( i ) )
    { 
      // swap listbox item with the next one
      if ( i < iStartIndex && !pLBoxSelected->GetSel( i + 1 ))
      {
        CString itemText;
        pLBoxSelected->GetText( i, itemText );
        DWORD_PTR pData = pLBoxSelected->GetItemData( i );
        pLBoxSelected->DeleteString( i );
        pLBoxSelected->InsertString( i + 1, itemText );
        pLBoxSelected->SetItemData( i + 1, pData );
        pLBoxSelected->SetSel( i + 1, TRUE );
      } /* endif */
    } /* endif */
  } /* endfor */
}


void CDetailView::RefillListboxes( PSHORT psViewList )
{
  CListBox *pLBoxAvail;
  CListBox *pLBoxSelected;

  if ( (pLBoxAvail = (CListBox*)GetDlgItem( ID_SELVIEW_AVAIL_LB ))!= NULL)
  {
    pLBoxAvail->ResetContent();

    for ( int i = 1; i < MAX_DEFINEDCOLUMNS; i++ )
    {
      // copy only columns which have a length > 0                    
      if (ColumnHeader[i][0] && usColumnWidth[i])
      {
        // search the current column in the list of selected columns
        BOOL fSelected = FALSE;
        int j = 0;
        int sColumn;
        while ( ( j < MAX_VIEW) && ((psViewList[j]) != CLBLISTEND) )
        {
          sColumn = psViewList[j] & CLB_COLNUMBERMASK;
          if ( sColumn == i )
          {
            fSelected = TRUE;
          } /* endif */
          j++;      
        } /* endwhile */

        // add to available listbox if not selected
        if ( !fSelected )
        {
          int iItem = pLBoxAvail->InsertString( -1, ColumnHeader[i] );
          if ( iItem != LB_ERR )
          {
            pLBoxAvail->SetItemData( iItem, i );
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endfor */
    pLBoxAvail->SetCurSel( 0 ); 
  } /* endif */

  if ( (pLBoxSelected = (CListBox*)GetDlgItem( ID_SELVIEW_SELECT_LB )) != NULL)
  {
    int i = 0;
    int sColumn;

    pLBoxSelected->ResetContent();

    while ( (i<MAX_VIEW)&&((sColumn = psViewList[i]) != CLBLISTEND) )
    {
      sColumn &= CLB_COLNUMBERMASK;

      if (ColumnHeader[sColumn][0] && usColumnWidth[sColumn])
      {
        int	iItem = pLBoxSelected->InsertString( -1, ColumnHeader[sColumn] );
        if ( iItem != LB_ERR )
        {
          pLBoxSelected->SetItemData( iItem, sColumn );
        } /* endif */
      } /* endif */
      i++;      // point to next item in list
    } /* endwhile */
    psViewList[MAX_VIEW-1] = CLBLISTEND;
    pLBoxSelected->SetCurSel( 0 );
  } /* endif */
}


void CDetailView::RemoveFromSelected( CListBox *pLBoxAvail, CListBox *pLBoxSelected, int i )
{
  int iColumnIndex = pLBoxSelected->GetItemData( i );
  if (iColumnIndex != 1 )  // do not remove name column...
  {
    // delete item in selected listbox
    pLBoxSelected->DeleteString( i );

    // add item at correct position in available listbox
    int iNumAvailable = pLBoxAvail->GetCount();
    int iInsertHere = 0;
    for ( iInsertHere = 0; iInsertHere < iNumAvailable; iInsertHere++ )
    {
      int iAvailColumnIndex = pLBoxAvail->GetItemData( iInsertHere );
      if ( iAvailColumnIndex > iColumnIndex )
        break;
    }
    int iIndex = pLBoxAvail->InsertString( iInsertHere, ColumnHeader[iColumnIndex] );
    if ( iIndex != LB_ERR ) pLBoxAvail->SetItemData( iIndex, iColumnIndex );
  } /* endif */
}

void CDetailView::GetDefaultsFileName( int iPopupID, PSZ pszName )
{
  switch( iPopupID )
  {
    case ID_FOL_POPUP: strcpy( pszName, "FOLDERVIEW.DEFAULT" ); break;
    case ID_DIC_POPUP: strcpy( pszName, "DICTVIEW.DEFAULT" ); break;
    case ID_MEM_POPUP: strcpy( pszName, "MEMORYVIEW.DEFAULT" ); break;
    case ID_TAG_POPUP: strcpy( pszName, "MARKUPVIEW.DEFAULT" ); break;
    case ID_FLL_POPUP: strcpy( pszName, "FOLDERLISTVIEW.DEFAULT" ); break;
    case ID_MT_POPUP:  strcpy( pszName, "MTJOBVIEW.DEFAULT" ); break;
    case ID_LST_POPUP: strcpy( pszName, "LISTVIEW.DEFAULT" ); break;
  }
}


void CDetailView::OnSaveDefault()
{ 
  SHORT sView[MAX_VIEW];

  CListBox *pLBoxSelected = (CListBox*)GetDlgItem( ID_SELVIEW_SELECT_LB);
  int iNoOfItems = pLBoxSelected->GetCount();
  int i;

  memset( sView, 0, sizeof(sView) );

  for (i=0; i< iNoOfItems; i++)
  {
    sView[i] = (SHORT) pLBoxSelected->GetItemData( i );
  } /* endfor */
  sView[i] = CLBLISTEND;

  CHAR szDefaults[MAX_LONGFILESPEC];

  UtlMakeEQFPath( szDefaults, NULC, PROPERTY_PATH, NULL );
  strcat( szDefaults, BACKSLASH_STR );
  GetDefaultsFileName( this->sPopupID, szDefaults + strlen(szDefaults) );

  UtlWriteFile( szDefaults, sizeof(sView), (PVOID)sView, FALSE );
}


void CDetailView::OnLoadDefault()
{
  CHAR szDefaults[MAX_LONGFILESPEC];

  UtlMakeEQFPath( szDefaults, NULC, PROPERTY_PATH, NULL );
  strcat( szDefaults, BACKSLASH_STR );
  GetDefaultsFileName( this->sPopupID, szDefaults + strlen(szDefaults) );

  PSHORT psView = NULL;
  USHORT usLen = 0;
  if ( UtlLoadFile( szDefaults, (PVOID *)&psView, &usLen, FALSE, FALSE ) )
  {
    if ( usLen <= (MAX_VIEW * sizeof(USHORT)) )   // is size valid?
    {
      RefillListboxes( psView );
    } /* enif */
    UtlAlloc( (PVOID *)&psView, 0, 0, NOMSG );
  } /* endif */

}


void CDetailView::OnDblclkAvail()
{

  CListBox *pLBoxAvail =    (CListBox*)GetDlgItem( ID_SELVIEW_AVAIL_LB );
  CListBox *pLBoxSelected = (CListBox*)GetDlgItem( ID_SELVIEW_SELECT_LB);

  int iNoOfItems = pLBoxSelected->GetCount();

  if ( pLBoxAvail && pLBoxSelected && (iNoOfItems < MAX_VIEW) )
  {
    // remove entry from available listbox
    int iSelItem = pLBoxAvail->GetCurSel();
    if ( iSelItem >= 0 )
    {
      int iColumnIndex = pLBoxAvail->GetItemData( iSelItem );
      pLBoxAvail->DeleteString( iSelItem );

      // add entry to the end of the seleced listbox
      int iIndex = pLBoxSelected->InsertString( -1, ColumnHeader[iColumnIndex] );
      if ( iIndex != LB_ERR ) pLBoxSelected->SetItemData( iIndex, iColumnIndex );

      int iNumSelected = pLBoxSelected->GetCount();
      ((CWnd *)GetDlgItem( ID_SELVIEW_VIEW_PB ))->EnableWindow( iNumSelected != 0 );
      ((CWnd *)GetDlgItem( ID_SELVIEW_SELECT_PB ))->EnableWindow( iNumSelected < MAX_VIEW );
      ((CWnd *)GetDlgItem( ID_SELVIEW_DESELECT_PB ))->EnableWindow( iNumSelected != 0 );
    } /* endif */
  } /* endif */
}

void CDetailView::OnDeselect()
{  // should do the same as dblclick on an item in "available" listbox
  CDetailView::OnDblclkSelect();
}

void CDetailView::OnDblclkSelect()
{
  CListBox *pLBoxAvail =    (CListBox*)GetDlgItem( ID_SELVIEW_AVAIL_LB );
  CListBox *pLBoxSelected = (CListBox*)GetDlgItem( ID_SELVIEW_SELECT_LB);
  if ( pLBoxSelected && pLBoxAvail )
  {
    int iStartIndex = pLBoxSelected->GetCount() - 1;
    for( int i = iStartIndex; i > -1; i-- )
    {
      if ( pLBoxSelected->GetSel( i ) )
      { 
        // move back to available list
        RemoveFromSelected( pLBoxAvail, pLBoxSelected, i );
      } /* endif */
    } /* endfor */

    int iNumSelected = pLBoxSelected->GetCount();
    ((CWnd *)GetDlgItem( ID_SELVIEW_VIEW_PB ))->EnableWindow( iNumSelected != 0 );
    ((CWnd *)GetDlgItem( ID_SELVIEW_DESELECT_PB ))->EnableWindow( iNumSelected != 0 );
  } /* endif */
}

void CDetailView::OnView()
{
  /********************************************************************/
  /* get all items of the selected listbox in the requested order and */
  /* fill it in our detailsview structure                             */
  /********************************************************************/
  memset( asDetailsView, 0, sizeof( asDetailsView ));

  CListBox *pLBoxSelected = (CListBox*)GetDlgItem( ID_SELVIEW_SELECT_LB);
  int iNoOfItems = pLBoxSelected->GetCount();
  int i;

  for (i=0; i< iNoOfItems; i++)
  {
    asDetailsView[i] = (SHORT) pLBoxSelected->GetItemData( i );
  } /* endfor */
  asDetailsView[i] = CLBLISTEND;
  /********************************************************************/
  /* continue with normal processing                                  */
  /********************************************************************/
  CDialog::OnOK();
}

void CDetailView::OnCancel()
{
  CDialog::OnCancel();
}

void CDetailView::OnHelp()
{
  UtlInvokeHelp();
}

void CDetailView::OnClear()
{
  CListBox *pLBoxAvail =    (CListBox*)GetDlgItem( ID_SELVIEW_AVAIL_LB );
  CListBox *pLBoxSelected = (CListBox*)GetDlgItem( ID_SELVIEW_SELECT_LB);
  if ( pLBoxSelected && pLBoxAvail )
  {
    int iStartIndex = pLBoxSelected->GetCount() - 1;
    for( int i = iStartIndex; i > -1; i-- )
    {
      RemoveFromSelected( pLBoxAvail, pLBoxSelected, i );
    } /* endfor */
  } /* endif */
}

/**********************************************************************/
/* Disable the default pushbutton and treat the Enter key as double   */
/* click                                                              */
/**********************************************************************/
LRESULT CDetailView::OnDefID( WPARAM mp1, LPARAM mp2)
{
  LRESULT lResult = MAKELONG( 0,0 );
  mp1; mp2;

  if (::GetKeyState( VK_RETURN ) & 0x8000 )
  {
    CWnd  * pFocusWnd = GetFocus();
    if ( pFocusWnd ==  GetDlgItem( ID_SELVIEW_AVAIL_LB))
    {
      OnDblclkAvail();
      lResult = TRUE; // MAKELONG(0,0);
    }
    else
    if ( pFocusWnd ==  GetDlgItem( ID_SELVIEW_SELECT_LB))
    {
      OnDblclkSelect();
      lResult = TRUE; // MAKELONG(0,0);
    } /* endif */
  } /* endif */
  return lResult;
}


/////////////////////////////////////////////////////////////////////////////
// CEQFDoc

IMPLEMENT_DYNCREATE(CEQFDoc, CDocument)

BEGIN_MESSAGE_MAP(CEQFDoc, CDocument)
        //{{AFX_MSG_MAP(CEQFDoc)
                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!
//        ON_WM_CLOSE()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFllListDoc construction/destruction

CEQFDoc::CEQFDoc()
{

  fUserFlag = FALSE;
  fNoClose = FALSE;
}

CEQFDoc::~CEQFDoc()
{
}

BOOL CEQFDoc::CanCloseFrame( CFrameWnd * pFrame )
{
  pFrame;
  if ( !fNoClose )
  {

//  UtlSaveWindowPos( pIda->hFrame, &(pIda->CommArea.swpSizePos) );
//
//  /**************************************************************/
//  /* Call instance callback function                            */
//  /**************************************************************/
//  (*pIda->pfnCallBack)( &pIda->CommArea, hwnd, message, mp1, mp2 );

    /************************************************************/
    /* End list window                                          */
    /************************************************************/
    EqfRemoveObject( TWBFORCE, pActView->m_hWnd);
  }
  else
  {
  } /* endif */
  return ( !fNoClose );
}



BOOL CEQFDoc::OnNewDocument()
{
  if (!CDocument::OnNewDocument())
          return FALSE;

  POSITION pos = GetFirstViewPosition();
  pActView = GetNextView( pos );

  SetObjName( (char *)((CEQFDApp*)AfxGetApp())->chTitle  );

  return TRUE;
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TwbInitMenu            Process WM_INITMENU message of Twb|
//+----------------------------------------------------------------------------+
//|Function call:     TwbInitMenu( HWND hwnd, MPARAM mp1, MPARAM mp2 );        |
//+----------------------------------------------------------------------------+
//|Description:       Process the intit menu message for the Twb actionbar.    |
//|                   Either process the messs                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   HWND   hwnd                                              |
//+----------------------------------------------------------------------------+
//|Returncode type:   LRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       FALSE (always)                                           |
//+----------------------------------------------------------------------------+
//|Function flow:     switch pulldown id                                       |
//|                     case help pulldown                                     |
//|                       do nothing                                           |
//|                     case windows pulldown                                  |
//|                       delete all window entries in pulldown                |
//|                       get list of active objects                           |
//|                       add active objects to pulldown                       |
//|                     default                                                |
//|                       pass message to active object                        |
//|                       enable specific utilities                            |
//|                   endswitch                                                |
//+----------------------------------------------------------------------------+
LRESULT CEQFDoc::TwbInitMenu( HWND hwnd, WPARAM mp1, LPARAM mp2 )
{
   hwnd;
   switch ( SHORT1FROMMP1( mp1) )
   {
      case PID_TWBM_SM_WINDOWS:
         // do nothing; leave windows items active ...
         break;

      case PID_TWBM_SM_HELP:
         // do nothing; leave help items active ...

         // GQ 2015/10/13: disable some of the help items until out help system is working again
         UtlMenuDisableItem( PID_HELP_FOR_HELP );
         UtlMenuDisableItem( PID_HELP_MI_INDEX );


         break;

      case PID_UTILS_SM_PANELLANG:
         // do nothing; leave panel languages active ...
         break;

#if defined(_TQM)
      case PID_TWBM_SM_TQM:
         // Pass TQM stuff to TQM list handler
         return EqfSend2Handler( TQMLISTHANDLER, WM_INITMENU, mp1, mp2 );
         break;
#endif

       default:
         /*************************************************************/
         /* remove report handler menu item from pulldown if          */
         /* report handler is not active                              */
         /*************************************************************/
         if ( (SHORT1FROMMP1( mp1) == PID_TWBM_SM_UTILITIES) &&
              !((CEQFDApp*)AfxGetApp())->fReportHandler )
         {
           HMENU hwndMenu = ::GETMENU((HWND)UtlQueryULong( QL_TWBFRAME ));

           DeleteMenu( hwndMenu, PID_UTILS_MI_REPORT, MF_BYCOMMAND );
         } /* endif */

         //--- leave it to active instance to set menu items ---
         ::SendMessage( pActView->m_hWnd, WM_EQF_INITMENU, mp1, mp2);

         //--- ensure that some of the items are enabled anyway ---
         UtlMenuEnableItem( PID_UTILS_MI_TAGTABLE );
         UtlMenuEnableItem( PID_UTILS_MI_LNGUPDATE );
         UtlMenuEnableItem( PID_UTILS_MI_DRIVES );
         UtlMenuEnableItem( PID_UTILS_MI_PLGINMGR );  // new start
         UtlMenuEnableItem( PID_UTILS_MI_ATOVERUP );  // new end
         UtlMenuEnableItem( PID_TERMLISTS_POPUP );
         UtlMenuEnableItem( PID_UTILS_MI_EXCLUSION );
         UtlMenuEnableItem( PID_UTILS_MI_NEWTERMS );
         UtlMenuEnableItem( PID_UTILS_MI_FOUNDTERMS );
         UtlMenuEnableItem( PID_UTILS_MI_ABBR );
         UtlMenuEnableItem( PID_UTILS_SM_PANELLANG );
         UtlMenuEnableItem( PID_FILE_MI_SYSPROP );
         UtlMenuEnableItem( PID_UTILS_MI_MT );
         UtlMenuEnableItem( PID_FILE_MI_EXIT );

//       /*************************************************************/
//       /* Enable all menu items required by user handlers           */
//       /*************************************************************/
//       if ( usUserHandler )
//       {
//         USHORT i, j;                 // loop indices
//         for ( i = 0; i < usUserHandler; i++ )
//         {
//           j = 0;
//           while ( pUserHandler[i].ausMenuIDs[j] )
//           {
//             UtlMenuEnableItem( pUserHandler[i].ausMenuIDs[j] );
//             j++;
//           } /* endwhile */
//         } /* endfor */
//       } /* endif */

         break;
   } /* endswitch */

   return( FALSE );
} /* end of function TwbInitMenu */



/////////////////////////////////////////////////////////////////////////////
// CEQFDoc serialization

void CEQFDoc::Serialize(CArchive& ar)
{
        if (ar.IsStoring())
        {
                // TODO: add storing code here
        }
        else
        {
                // TODO: add loading code here
        }
}

VOID CEQFDoc::OnConfDrives()
{
  TWBDrives( pActView->m_hWnd, AfxGetResourceHandle(), FALSE );
}

VOID CEQFDoc::OnConnectResources()
{
  /*********************************************************/
  /* Handle request for connect dialog                     */
  /*********************************************************/
//  TWBConnect( pActView->m_hWnd, AfxGetResourceHandle(), usClass );
//if ( pIda->hObjFocus )
//{
//  usClass = EqfQueryObjectClass( pIda->hObjFocus );
  USHORT usClass = EqfQueryObjectClass( GetParent(pActView->m_hWnd));
  switch ( usClass )
  {
     case clsMEMORY :
     case clsDICTIONARY :
        TWBConnect( pActView->m_hWnd, AfxGetResourceHandle(), usClass );
        break;
     case clsFOLDERLIST :
     case clsFOLDER :
     case clsTAGTABLE:
     case clsLIST:
     case clsLNGUPDATE:
     case clsDOCUMENT :
     default:
        break;
  } /* endswitch */
//} /* endif */
}

/////////////////////////////////////////////////////////////////////////////
// CFllListDoc diagnostics

#ifdef _DEBUG
void CEQFDoc::AssertValid() const
{
        CDocument::AssertValid();
}

void CEQFDoc::Dump(CDumpContext& dc) const
{
        CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEQFDoc commands

int CALLBACK CLBCompareItem
(
  LPARAM lParam1,
  LPARAM lParam2,
  LPARAM lParamSort
)
{
  PSZ pszItem1 = (PSZ)lParam1 + CLB_300_OFFSET;
  PSZ pszItem2 = (PSZ)lParam2 + CLB_300_OFFSET;
  PCLBCOMPAREITEMDATA pData = (PCLBCOMPAREITEMDATA)lParamSort;
  PSHORT psSortColumn = pData->psSortColumns;
  PUSHORT pusColumnType = pData->pusColumnType;
  int iResult = 0;
  int i = 0;

  // special handling for total line and dividerline
  if ( strncmp( pszItem1, "Total\x15", 6 ) == 0 )
  {
    return( 1 );  // total line is always last line in list
  }
  else if ( strncmp( pszItem2, "Total\x15", 6 ) == 0 )
  {
    return( -1 );  // total line is always last line in list
  }
  else if ( strncmp( pszItem1, "===", 3 ) == 0 )
  {
    return( 1 );  // diver line comes always behind any other line (except total)
  }
  else if ( strncmp( pszItem2, "===", 3 ) == 0 )
  {
    return( -1 );  // diver line comes always behind any other line (except total)
  } /* endif */

  while ( (i<MAX_VIEW) && (*psSortColumn != -1 ) && (iResult == 0))
  {
    // buffers for item data
    static char szBuffer1[4096];
    static char szBuffer2[4096];

    // copy item data to local buffers
    memset( szBuffer1, 0, sizeof(szBuffer1) );
    strncpy( szBuffer1, pszItem1, sizeof(szBuffer1)-1 );
    memset( szBuffer2, 0, sizeof(szBuffer2) );
    strncpy( szBuffer2, pszItem2, sizeof(szBuffer2)-1 );

    // get current sort column and sort order flag
    int sCurSort = *psSortColumn;
    BOOL fDescending = ((sCurSort & SORT_DESCENDING_FLAG) != 0);
    sCurSort &= ~SORT_DESCENDING_FLAG;
    // get rid of any special settings
    sCurSort &= 0xff;

    // position to column data
    PSZ pszColText1 = UtlParseX15( szBuffer1, (SHORT)sCurSort );
    PSZ pszColText2 = UtlParseX15( szBuffer2, (SHORT)sCurSort );

    // compare column data depending on column type
    iResult = CLBCompareSingleColum( pusColumnType[sCurSort], pszColText1, pszColText2 );
    if ( fDescending )
    {
      iResult *= -1;
    } /* endif */

    // next column
    i++;
    psSortColumn++;
  } /* endwhile */

  return( iResult );
} /* end of function CompareItems */

// do a numerical compare
int NumCompare( PSZ s1, PSZ s2 )
{
  int iResult = 0;

  LONG lValue1 = atol( s1 );
  LONG lValue2 = atol( s2 );
  if ( lValue1 > lValue2 )
  {
    iResult = 1;
  }
  else if ( lValue1 < lValue2  )
  {
    iResult = -1;
  } /* endif */
  return( iResult );
}

BOOL isNumber( PSZ pszText )
{
  while ( *pszText )
  {
    char c = *pszText++;

    // cannot use isdigit here as it throws a debug assertion when the text contains German Umlaut characters and
    // as the EQFD.EXE has to be compiled in debug mode...
//    if ( !isdigit( c ) ) return( FALSE );
    if ( (c < '0') || (c > '9') ) return( FALSE );
  } /*endwhile */
  return( TRUE );
}

// compare data of a single columm
int CALLBACK CLBCompareSingleColum
(
  USHORT usColType,                    // type of column
  PSZ    pszColText1,                  // text of column 1
  PSZ    pszColText2                   // text of column 2
)
{
  int iResult = 0;

  // compare column data depending on column type
  switch ( usColType )
  {
     case NUMERIC_DATA:
     case DATE_DATA:
     case TIME_DATA:
     case DATETIME_DATA:
     case FDATE_DATA:
     case FTIME_DATA:
     case FDATETIME_DATA:
        {
          // numerical compare
          iResult = NumCompare( pszColText1, pszColText2 );
        }
        break;

     default:
        // check for numbers in text columns (e.g. word count results)
        if ( isNumber( pszColText1 ) && isNumber( pszColText2 ) )
        {
          iResult = NumCompare( pszColText1, pszColText2 );
        }
        else
        {
          // string compare case insensitive and using the collating sequence
          iResult = _stricoll( pszColText1, pszColText2 );
        } /* endif */
        break;
  } /* endswitch */

  return( iResult );
} /* end of function CLBCompareSingleColum */

/////////////////////////////////////////////////////////////////////////////
// CSortDlg dialog


CSortDlg::CSortDlg(CWnd* pParent /*=NULL*/)
        : CDialog(CSortDlg::IDD, pParent)
{
        //{{AFX_DATA_INIT(CSortDlg)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT

}


BEGIN_MESSAGE_MAP(CSortDlg, CDialog)
        //{{AFX_MSG_MAP(CSortDlg)
        ON_BN_CLICKED(ID_SORT_SORT_PB, OnSort)
        ON_BN_CLICKED(ID_SORT_CANCEL_PB, OnCancel)
        ON_BN_CLICKED(PID_PB_HELP, OnHelp)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSortDlg message handlers

BOOL CSortDlg::OnInitDialog()
{
  /********************************************************************/
  /*                                                                  */
  /********************************************************************/
  int j;

  for (j = 0; j < 3; j++ )
  {
    CComboBox *pComboSortKey;
    CButton   *pDescendingFlag;
    CButton   *pAscendingFlag;

    switch ( j )
    {
      case 0:
        pComboSortKey = (CComboBox*)GetDlgItem( ID_SORT_KEY1_CB );
        pDescendingFlag = (CButton*)GetDlgItem( ID_SORT_DESCENDING1_RB );
        pAscendingFlag = (CButton*)GetDlgItem( ID_SORT_ASCENDING1_RB );
        break;
      case 1:
        pComboSortKey = (CComboBox*)GetDlgItem( ID_SORT_KEY2_CB );
        pDescendingFlag = (CButton*)GetDlgItem( ID_SORT_DESCENDING2_RB );
        pAscendingFlag = (CButton*)GetDlgItem( ID_SORT_ASCENDING2_RB );
        break;
      default:
      case 2:
        pComboSortKey = (CComboBox*)GetDlgItem( ID_SORT_KEY3_CB );
        pDescendingFlag = (CButton*)GetDlgItem( ID_SORT_DESCENDING3_RB );
        pAscendingFlag = (CButton*)GetDlgItem( ID_SORT_ASCENDING3_RB );
        break;
    } /* endswitch */

    if ( pComboSortKey  )
    {
      BOOL fSet = FALSE;

      for ( int i=1; i < MAX_DEFINEDCOLUMNS; i++ )
      {
        /****************************************************************/
        /* copy only columns which have a length > 0                    */
        /****************************************************************/
        if (ColumnHeader[i][0] && usColumnWidth[i])
        {
          int iItem = pComboSortKey->InsertString( -1, ColumnHeader[i] );
          if ( iItem != LB_ERR )
          {
            pComboSortKey->SetItemData( iItem, i );
          } /* endif */

          SHORT sCurSort = asSortView[j];
          if ( sCurSort > 0 )
          {
            BOOL fDescending = ((sCurSort & SORT_DESCENDING_FLAG) != 0);
            sCurSort &= ~SORT_DESCENDING_FLAG;
            if ( i == sCurSort )
            {
              pComboSortKey->SetCurSel( iItem );
              if ( fDescending )
              {
                fSet = TRUE;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
        pDescendingFlag->SetCheck( fSet );
        pAscendingFlag->SetCheck( !fSet );
      } /* endfor */
    } /* endif */
  } /* endfor */

  return TRUE;
}

void CSortDlg::OnSort()
{
  int j;
  int iPos = 0;

  // clear sort view list
  memset( asSortView, 0, sizeof( asSortView ));

  for ( j = 0; j < 3; j++ )
  {
    CComboBox *pComboSortKey;
    CButton   *pDescendingFlag;
    char szText[40];                   // buffer for selected column text

    switch ( j )
    {
      case 0:
        pComboSortKey = (CComboBox*)GetDlgItem( ID_SORT_KEY1_CB );
        pDescendingFlag = (CButton*)GetDlgItem( ID_SORT_DESCENDING1_RB );
        break;
      case 1:
        pComboSortKey = (CComboBox*)GetDlgItem( ID_SORT_KEY2_CB );
        pDescendingFlag = (CButton*)GetDlgItem( ID_SORT_DESCENDING2_RB );
        break;
      default:
      case 2:
        pComboSortKey = (CComboBox*)GetDlgItem( ID_SORT_KEY3_CB );
        pDescendingFlag = (CButton*)GetDlgItem( ID_SORT_DESCENDING3_RB );
        break;
    } /* endswitch */

    pComboSortKey->GetWindowText( szText, sizeof(szText)-1 );
    UtlStripBlanks( szText );
    if ( szText[0] != EOS )
    {
      int iItem = pComboSortKey->FindStringExact( -1, szText );
      if ( iItem != CB_ERR )
      {
        asSortView[iPos] = (SHORT)pComboSortKey->GetItemData( iItem );
        if ( pDescendingFlag->GetCheck() )
        {
          asSortView[iPos] |= SORT_DESCENDING_FLAG;
        } /* endif */
        iPos++;
      } /* endif */
    } /* endif */
  } /* endfor */

  // terminate selected sort list
  asSortView[iPos] = CLBLISTEND;

  /********************************************************************/
  /* continue with normal processing                                  */
  /********************************************************************/
  CDialog::OnOK();
}

void CSortDlg::OnCancel()
{

  CDialog::OnCancel();
}

void CSortDlg::OnHelp()
{
  UtlInvokeHelp();
}

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg dialog


CFilterDlg::CFilterDlg(CWnd* pParent /*=NULL*/)
        : CDialog(CFilterDlg::IDD, pParent)
{
        //{{AFX_DATA_INIT(CFilterDlg)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT

}


BEGIN_MESSAGE_MAP(CFilterDlg, CDialog)
        //{{AFX_MSG_MAP(CFilterDlg)
        ON_BN_CLICKED(ID_FILT2_FILTER_PB, OnFilter )
        ON_BN_CLICKED(ID_FILT2_CANCEL_PB, OnCancel)
        ON_BN_CLICKED(PID_PB_HELP,        OnHelp )
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg message handlers

BOOL CFilterDlg::OnInitDialog()
{
  /********************************************************************/
  /*                                                                  */
  /********************************************************************/
  int iRow;

  for (iRow = 0; iRow < 3; iRow++ )
  {
    CComboBox *pComboFilterKey;
    CComboBox *pComboOperator;
    CEdit     *pEditValue;
    BOOL      fConditionAvail;

    fConditionAvail = (Filter.asColumn[iRow] != 0) &&
                      (Filter.asOperator[iRow] != 0);

    switch ( iRow )
    {
      case 0:
        pComboFilterKey = (CComboBox*)GetDlgItem( ID_FILT2_KEY1_CB );
        pComboOperator  = (CComboBox*)GetDlgItem( ID_FILT2_OP1_CB );
        pEditValue      = (CEdit*)GetDlgItem( ID_FILT2_VALUE1_EF );
        break;
      case 1:
        pComboFilterKey = (CComboBox*)GetDlgItem( ID_FILT2_KEY2_CB );
        pComboOperator  = (CComboBox*)GetDlgItem( ID_FILT2_OP2_CB );
        pEditValue      = (CEdit*)GetDlgItem( ID_FILT2_VALUE2_EF );
        break;
      default:
      case 2:
        pComboFilterKey = (CComboBox*)GetDlgItem( ID_FILT2_KEY3_CB );
        pComboOperator  = (CComboBox*)GetDlgItem( ID_FILT2_OP3_CB );
        pEditValue      = (CEdit*)GetDlgItem( ID_FILT2_VALUE3_EF );
        break;
    } /* endswitch */

    if ( pEditValue )
    {
      // set text limit to 39 (value field is defined as char[40] )
      pEditValue->SetLimitText( 39 );
    } /* endif */

    if ( pComboFilterKey  )
    {
      for ( int i=1; i < MAX_DEFINEDCOLUMNS; i++ )
      {
        /****************************************************************/
        /* copy only columns which have a length > 0                    */
        /****************************************************************/
        if (ColumnHeader[i][0] && usColumnWidth[i])
        {
          int iItem = pComboFilterKey->InsertString( -1, ColumnHeader[i] );
          if ( iItem != LB_ERR )
          {
            pComboFilterKey->SetItemData( iItem, i );
            if ( fConditionAvail && (Filter.asColumn[iRow] == i) )
            {
              pComboFilterKey->SetCurSel( iItem );
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endfor */

      int j = 0;
      while ( ClbOperator[j].OpID != LAST_CLBOPID )
      {
        CString s;
        s.LoadString( ClbOperator[j].sStringID );
        int iItem = pComboOperator->InsertString( -1, s );
        if ( iItem != LB_ERR )
        {
          pComboOperator->SetItemData( iItem, ClbOperator[j].OpID );
          if ( fConditionAvail &&
               (Filter.asOperator[iRow] == ClbOperator[j].OpID) )
          {
            pComboOperator->SetCurSel( iItem );
          } /* endif */
        } /* endif */
        j++;
      } /* endwhile */
    } /* endif */
    if ( fConditionAvail )
    {
      pEditValue->SetWindowText( Filter.aszValue[iRow] );
    } /* endif */
  } /* endfor */

  return TRUE;
}

void CFilterDlg::OnFilter()
{
  int iControlGroup;
  int iPos = 0;
  CLBFILTER NewFilter;
  BOOL fOK = TRUE;

  // clear NewFilter
  memset( &NewFilter, 0, sizeof(NewFilter));

  for ( iControlGroup = 0; iControlGroup < 3; iControlGroup++ )
  {
    CComboBox *pComboFilterKey;
    CComboBox *pComboOperator;
    CEdit     *pEditValue;
    int       iColumn;                 // selected column
    int       iOperator;               // selected operator
    char      szValue[40];             // selected value

    switch ( iControlGroup )
    {
      case 0:
        pComboFilterKey = (CComboBox*)GetDlgItem( ID_FILT2_KEY1_CB );
        pComboOperator  = (CComboBox*)GetDlgItem( ID_FILT2_OP1_CB );
        pEditValue      = (CEdit*)GetDlgItem( ID_FILT2_VALUE1_EF );
        break;
      case 1:
        pComboFilterKey = (CComboBox*)GetDlgItem( ID_FILT2_KEY2_CB );
        pComboOperator  = (CComboBox*)GetDlgItem( ID_FILT2_OP2_CB );
        pEditValue      = (CEdit*)GetDlgItem( ID_FILT2_VALUE2_EF );
        break;
      default:
      case 2:
        pComboFilterKey = (CComboBox*)GetDlgItem( ID_FILT2_KEY3_CB );
        pComboOperator  = (CComboBox*)GetDlgItem( ID_FILT2_OP3_CB );
        pEditValue      = (CEdit*)GetDlgItem( ID_FILT2_VALUE3_EF );
        break;
    } /* endswitch */

    // get values of currently processed row

    int iColumnIndex = pComboFilterKey->GetCurSel();
    if ( iColumnIndex != CB_ERR )
    {
      iColumn = pComboFilterKey->GetItemData(iColumnIndex);
    }
    else
    {
      iColumn = -1;
    } /* endif */


    int iOperatorIndex = pComboOperator->GetCurSel();
    if ( iOperatorIndex != CB_ERR )
    {
      iOperator = pComboOperator->GetItemData(iOperatorIndex);
    }
    else
    {
      iOperator = -1;
    } /* endif */

    pEditValue->GetWindowText( szValue, sizeof(szValue) );
    UtlStripBlanks( szValue );


    // test consistency of data:
    // 1) if a column has been selected also an operator is required and vice versa
    // 2) if a value has been specified also column and operator is required
    // 3) an empty value is only valid for equal and not equal operator

    if ( fOK )
    {
      if ( (iColumn != -1) && (iOperator == -1) )
      {
        fOK = FALSE;
        UtlError( CLBFILT_OP_REQUIRED, MB_OK, 0, NULL, EQF_ERROR );
        pComboOperator->SetFocus();
      }
    }

    if ( fOK )
    {
      if ( (iColumn == -1) && (iOperator != -1) )
      {
        fOK = FALSE;
        UtlError ( CLBFILT_COL_REQUIRED, MB_OK, 0, NULL, EQF_ERROR );
        pComboFilterKey->SetFocus();
      }
    }

    if ( fOK && (szValue[0] != EOS) )
    {
      if ( iColumn == -1)
      {
        fOK = FALSE;
        UtlError ( CLBFILT_COL_REQUIRED, MB_OK, 0, NULL, EQF_ERROR );
        pComboFilterKey->SetFocus();
      }
      else if ( iOperator == -1 )
      {
        fOK = FALSE;
        UtlError ( CLBFILT_OP_REQUIRED, MB_OK, 0, NULL, EQF_ERROR );
        pComboOperator->SetFocus();
      }
    }

    if ( fOK && (szValue[0] == EOS) &&
         ((iColumn != -1) || (iOperator != -1)) )
    {
      if ( (iOperator != EQUAL_CLBOPID) &&
           (iOperator != NOTEQUAL_CLBOPID) )
      {
        fOK = FALSE;
        UtlError ( CLBFILT_VAL_REQUIRED, MB_OK, 0, NULL, EQF_ERROR );
        pEditValue->SetFocus();
      } /* endif */
    } /* endif */

    // add current values to filter if something specified
    if ( fOK && (iColumn != -1) && (iOperator != -1) )
    {
      NewFilter.asColumn[iPos] = (SHORT)iColumn;
      NewFilter.asOperator[iPos] = (SHORT)iOperator;
      strcpy( NewFilter.aszValue[iPos], szValue );
      iPos++;
    } /* endif */
  } /* endfor */

  // store new filter and leave dialog
  if ( fOK )
  {
    memcpy( &Filter, &NewFilter, sizeof(Filter) );
    CDialog::OnOK();
  } /* endif */

}

void CFilterDlg::OnCancel()
{

  CDialog::OnCancel();
}

void CFilterDlg::OnHelp()
{
  UtlInvokeHelp();
}

// apply a filter
VOID CListViewEx::Filter()
{
  // update window title text
  {
    CString s;
    GetWindowText( s );

    // remove any trailing [Some] from title
    int iPos;
    iPos = s.Find( " [Some]" );
    if ( iPos != -1 )
    {
      s = s.Left( iPos );
    } /* endif */

    // added [Some] if a filter has been activated
    if ( (m_Filter.asColumn[0] != 0) &&
         (m_Filter.asOperator[0] != 0) )
    {
      s += " [Some]";
    } /* endif */
    SetWindowText( s );

    CDocument *pDoc = GetDocument();
    pDoc->SetTitle( s );
  }

  LockWindowUpdate();


  // delete all items in the list control
  {
    CListCtrl& ListCtrl = GetListCtrl();

    int  iCount = ListCtrl.GetItemCount();
    for ( int i = 0; i < iCount; i++ )
    {
      free( (PSZ) ListCtrl.GetItemData( 0 ));
      ListCtrl.DeleteItem( 0 );
    }
  }

  // Reload items using the callback function
  {
    (*pIda->pfnCallBack)( &pIda->CommArea, m_hWnd, WM_EQF_INITIALIZE,
                          MP1FROMSHORT(0), MP2FROMP(NULL) );
  }

  // repaint the window
  Invalidate();
  UnlockWindowUpdate();
}

// convert a normal button to a bitmap button using the specified Windows bitmap
int MakeBitmapButton( CDialog *pDlg, int iButtonID, int iBitmapID )
{
  CButton* pBtn= (CButton*)pDlg->GetDlgItem(iButtonID);
  pBtn->ModifyStyle( 0, BS_BITMAP );
  pBtn->SetBitmap(::LoadBitmap(NULL, MAKEINTRESOURCE(iBitmapID)));
  return( 0 );
}
