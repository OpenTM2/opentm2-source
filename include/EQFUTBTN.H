//+----------------------------------------------------------------------------+
//|  EQFUTBTN.H - Header file for button utilities                             |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description:                                                              |
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
// $Revision: 1.2 $ ----------- 17 Mar 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
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
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFUTBTN.HV_   1.1   14 Jan 1998 15:47:56   BUILD  $
 *
 * $Log:   J:\DATA\EQFUTBTN.HV_  $
 *
 *    Rev 1.1   14 Jan 1998 15:47:56   BUILD
 * - removed obsolete __export directive in Win32bit environment
 *
 *    Rev 1.0   09 Jan 1996 09:51:16   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
/**********************************************************************/
/* Class name for picture buttons                                     */
/**********************************************************************/
#define PICBTNCLASS "EqfPicBtn"

//Standard image bitmap
#define IDB_STANDARDIMAGES          400

//Image indices inside the standard bitmap.
#define TOOLIMAGE_MIN               0
#define TOOLIMAGE_EDITCUT           0
#define TOOLIMAGE_EDITCOPY          1
#define TOOLIMAGE_EDITPASTE         2
#define TOOLIMAGE_FILENEW           3
#define TOOLIMAGE_FILEOPEN          4
#define TOOLIMAGE_FILESAVE          5
#define TOOLIMAGE_FILEPRINT         6
#define TOOLIMAGE_HELP              7
#define TOOLIMAGE_HELPCONTEXT       8
#define TOOLIMAGE_MAX               8


//Additional Standard Cursors as defined in the UI Design Guide.
#define IDC_NEWUICURSORMIN          500
#define IDC_RIGHTARROW              500
#define IDC_CONTEXTHELP             501
#define IDC_MAGNIFY                 502
#define IDC_NODROP                  503
#define IDC_TABLETOP                504
#define IDC_HSIZEBAR                505
#define IDC_VSIZEBAR                506
#define IDC_HSPLITBAR               507
#define IDC_VSPLITBAR               508
#define IDC_SMALLARROWS             509
#define IDC_LARGEARROWS             510
#define IDC_HARROWS                 511
#define IDC_VARROWS                 512
#define IDC_NESWARROWS              513
#define IDC_NWSEARROWS              514
#define IDC_NEWUICURSORMAX          514



//Standard sizes for toolbar buttons and bitmaps
#define TOOLBUTTON_STDWIDTH         24
#define TOOLBUTTON_STDHEIGHT        22
#define TOOLBUTTON_STDIMAGEWIDTH    16
#define TOOLBUTTON_STDIMAGEHEIGHT   15



/*
 * Values for button display states.  Each value is mutually
 * exclusive and contains one or more grouping bits.  Each group
 * represents buttons that share some sub-state in common.
 *
 * The actual state values are split into a command group and an
 * attribute group.  Up, mouse down, and disabled states are identical,
 * but only attributes can have down, down disabled, and indeterminate
 * states.
 *
 * BUTTONGROUP_BLANK is defined so an application can draw only the button
 * without an image in the up, down, mouse down, or indeterminate
 * state, that is, BUTTONGROUP_BLANK is inclusive with BUTTONGROUP_DOWN
 * and BUTTONGROUP_LIGHTFACE.
 */


#define BUTTONGROUP_DOWN                0x0001
#define BUTTONGROUP_ACTIVE              0x0002
#define BUTTONGROUP_DISABLED            0x0004
#define BUTTONGROUP_LIGHTFACE           0x0008
#define BUTTONGROUP_BLANK               0x0010


//Command buttons only
#define COMMANDBUTTON_UP                (BUTTONGROUP_ACTIVE)
#define COMMANDBUTTON_MOUSEDOWN         (BUTTONGROUP_ACTIVE | BUTTONGROUP_DOWN)
#define COMMANDBUTTON_DISABLED          (BUTTONGROUP_DISABLED)

//Attribute buttons only
#define ATTRIBUTEBUTTON_UP              (BUTTONGROUP_ACTIVE)
#define ATTRIBUTEBUTTON_MOUSEDOWN       (BUTTONGROUP_ACTIVE | BUTTONGROUP_DOWN)
#define ATTRIBUTEBUTTON_DISABLED        (BUTTONGROUP_DISABLED)
#define ATTRIBUTEBUTTON_DOWN            (BUTTONGROUP_ACTIVE | BUTTONGROUP_DOWN | BUTTONGROUP_LIGHTFACE)
#define ATTRIBUTEBUTTON_INDETERMINATE   (BUTTONGROUP_ACTIVE | BUTTONGROUP_LIGHTFACE)
#define ATTRIBUTEBUTTON_DOWNDISABLED    (BUTTONGROUP_DISABLED | BUTTONGROUP_DOWN | BUTTONGROUP_LIGHTFACE)

//Blank buttons only
#define BLANKBUTTON_UP                  (BUTTONGROUP_ACTIVE | BUTTONGROUP_BLANK)
#define BLANKBUTTON_DOWN                (BUTTONGROUP_ACTIVE | BUTTONGROUP_BLANK | BUTTONGROUP_DOWN | BUTTONGROUP_LIGHTFACE)
#define BLANKBUTTON_MOUSEDOWN           (BUTTONGROUP_ACTIVE | BUTTONGROUP_BLANK | BUTTONGROUP_DOWN)
#define BLANKBUTTON_INDETERMINATE       (BUTTONGROUP_ACTIVE | BUTTONGROUP_BLANK | BUTTONGROUP_LIGHTFACE)

/*
 * Specific bits to prevent conversions of specific colors to system
 * colors.  If an application uses this newer library and never specified
 * any bits, then they benefit from color conversion automatically.
 */
#define PRESERVE_BLACK                  0x0100
#define PRESERVE_DKGRAY                 0x0200
#define PRESERVE_LTGRAY                 0x0400
#define PRESERVE_WHITE                  0x0800

#define PRESERVE_ALL                    (PRESERVE_BLACK | PRESERVE_DKGRAY | PRESERVE_LTGRAY | PRESERVE_WHITE)
#define PRESERVE_NONE                   0   //Backwards compatible

BOOL    WINAPI UIToolButtonDraw(HDC, int, int, int, int, HBITMAP, int, int, int, UINT);

#define ID_IDEDIT               100
#define ID_TEXTEDIT             101


#define IDD_STYLEDIALOG                 1000
#define ID_COMMAND                  110
#define ID_VISIBLE                  120
#define ID_DISABLED                 121
#define ID_GROUP                    122
#define ID_TAB                      123
#define ID_AUTO                                         124
#define ID_MENUSELECT                           125
#define ID_CHECK                    111
#define ID_RADIO                    112
#define IDD_HELPDIALOG              2000

typedef struct tagPICBUT
{
    DWORD       dwStyle;        //Copy of GetWindowLong(hWnd, GWL_STYLE)
        UINT            uState;                 //State flags
        UINT            uOldState;              //copy of state flags, so we can restore the old state
        HGLOBAL         hBit;                   //handle of loaded bitmap resource
        int             width;                  //width of bitmap
        int             height;                 //height of bitmap
        BOOL            bLimitSize;     //do we restrict the size of the button?
        int             iImage;                 //Offset of image in the bitmap
        BOOL            bIgnoreSize;    //internal use. Set to TRUE to ignore WM_SIZE
        BOOL            bDeleteBitmap;  //internal use. Should we delete the bitmap after use?
} PICBUT;

typedef PICBUT FAR *PPICBUT;

#define CBPICBUT sizeof(PICBUT)



//Offsets to use with GetWindowLong
#define GWL_PICBUTHMEM  0

//Extra bytes for the window if the size of a global handle.
#define CBWINDOWEXTRA       sizeof(PVOID)

//Extra Class bytes.
#define CBCLASSEXTRA        0



/*
 * Macros to change the control state given a PPICBUT
 * and state flag(s)
 */
#define StateSet(p, wFlags)    (p->uState |=  (wFlags))
#define StateClear(p, wFlags)  (p->uState &= ~(wFlags))
#define StateTest(p, wFlags)   (p->uState &       (wFlags))




//Private functions specific to the control.

BOOL       PASCAL RegisterPicBtn(HANDLE);
LONG       PASCAL LPicButCreate(HWND, WORD, PPICBUT, LPCREATESTRUCT);
void   FAR PASCAL ResetImage( HWND hWnd, PPICBUT pPB, LPSTR lpszName );
LONG       PASCAL LPicButAPI(HWND, WORD, WORD, LONG, PPICBUT);
LONG FAR PASCAL PicButPaint( HWND, PPICBUT      );
void FAR ProcessButtonup( HWND, LPARAM, PPICBUT );
LONG FAR PASCAL  PicButWndProc(HWND, UINT, WPARAM, LPARAM);

BOOL        ToolButtonInit(void);
void        ToolButtonFree(void);


//Stringtable identifiers
#define IDS_CLASSNAME           0
#define IDS_FULLNAME            1
#define IDS_CREDITS             2

//Dialog editor interface string
#define IDS_DESCR                               3

#define IDS_PBS_COMMAND                 4
#define IDS_PBS_ATTRIBUTE               5

#define IDS_PBS_MENUSELECT              7
#define IDS_PBS_AUTO                    8




#define TOOLBUTTON_EXTRAWIDTH (TOOLBUTTON_STDWIDTH - TOOLBUTTON_STDIMAGEWIDTH)
#define TOOLBUTTON_EXTRAHEIGHT (TOOLBUTTON_STDHEIGHT - TOOLBUTTON_STDIMAGEHEIGHT)


// messages

#define PBM_SETIMAGE (WM_USER+10)


// typedefs

typedef struct tagPBM_IMAGE {
        HBITMAP         hBit;                     // handle of bitmap
        int             width;                    // width of bitmap to draw
        int             height;                   // height of bitmap to draw
        int             iImage;                   // index to the image to draw in the bitmap
        } PBM_IMAGE, FAR *LPPBM_IMAGE;





//Control specific styles.
#define PBS_COMMAND                     0x0001L
#define PBS_ATTRIBUTE           0x0000L

#define PBS_MENUSELECT          0x0002L
#define PBS_AUTO                        0x0004L
