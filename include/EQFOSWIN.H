/*! OS2TOWIN.H    macros and prototypes used for OS2->Win mapping
	Copyright (c) 1990-2015, International Business Machines Corporation and others. All rights reserved.
	main file containing defines and macros to ease up the conversion
	from an OS/2 program to WINDOWS...
*/

/**********************************************************************/
/* Defines for window classes                                         */
/**********************************************************************/
#define WC_SPINBUTTON     CCL_SPINBTN
#define WC_SLIDER         CCL_SLIDER
#ifndef WC_STATIC
    #define WC_STATIC         "STATIC"
#endif
#ifndef WC_LISTBOX
    #define WC_LISTBOX        "LISTBOX"
#endif
#ifndef WC_BUTTON
    #define WC_BUTTON         "BUTTON"
#endif
#ifndef WC_COMBOBOX
    #define WC_COMBOBOX       "COMBOBOX"
#endif
#ifndef WC_ENTRYFIELD
    #define WC_ENTRYFIELD     "EDIT"
#endif
#ifndef WC_SCROLLBAR
    #define WC_SCROLLBAR      "SCROLLBAR"
#endif

/**********************************************************************/
/* The following defines are copied from BSEERR.H to ensure common    */
/* return codes among platforms                                       */
/**********************************************************************/


// error codes not contained in Win32 include files:
#define ERROR_NO_ITEMS                  93
#define ERROR_MR_MSG_TOO_LONG           316
#define ERROR_MR_INV_MSGF_FORMAT        319
#define ERROR_MR_INV_IVCOUNT            320

/**********************************************************************/
/* error defines                                                      */
/*   -- should be moved to eqferr00.h                                 */
/**********************************************************************/
#define  ERROR_API_NOT_SUPPORTED 4711
/**********************************************************************/
/* define for WM_CONTROL                                              */
/**********************************************************************/
#define WM_CONTROL  WM_USER + 250

/**********************************************************************/
/* support of OS2 typedefs in Windows like USHORT, etc.               */
/**********************************************************************/

/**********************************************************************/
/* substitute typedefs - use definitely far items                     */
/**********************************************************************/
#define HFILE         HANDLE

/* Extract high and low order parts of 16 and 32 bit quantity */
//#define LOBYTE(w)       LOUCHAR(w)
//#define HIBYTE(w)       HIUCHAR(w)
#define LOUCHAR(w)      ((UCHAR)(w))
#define HIUCHAR(w)      ((UCHAR)(((USHORT)(w) >> 8) & 0xff))
#define LOUSHORT(l)     ((USHORT)(l))
#define HIUSHORT(l)     ((USHORT)(((ULONG)(l) >> 16) & 0xffff))


typedef unsigned char UCHAR;    /* uch */
typedef unsigned char BYTE;     /* b   */
typedef void far      *LHANDLE;
typedef unsigned short SHANDLE;

typedef HFILE * PHFILE;

typedef int   (pascal far  *PFN)();

typedef CHAR   FAR *PCHAR;

typedef UCHAR  FAR *PUCHAR;
typedef ULONG  FAR *PULONG;


typedef BOOL FAR *PBOOL;
typedef HWND FAR *PHWND;
typedef int       INT;
/**********************************************************************/
/* different KEYBOard defines                                         */
/* we will leave and use the PM defines all over                      */
/**********************************************************************/
#define VK_ESC        VK_ESCAPE
#define VK_BACKSPACE  VK_BACK
#define VK_ENTER      VK_RETURN
#define VK_PAGEUP     VK_PRIOR
#define VK_PAGEDOWN   VK_NEXT
#define VK_SCRLLOCK   VK_SCROLL
#define VK_BACKTAB    0x07             // no special define in windows
#define VK_CTRL       VK_CONTROL
#define VK_ALT        VK_MENU
#define VK_BREAK      VK_CANCEL
#define VK_NEWLINE    VK_RETURN
#define VK_ALTGRAF    (UCHAR) -2
#define VK_SYSRQ      (UCHAR) -2
#define VK_CAPSLOCK   VK_CAPITAL
#define VK_PRINTSCRN  VK_SNAPSHOT

/**********************************************************************/
/* WinQueryWindow substitutes                                         */
/**********************************************************************/
__declspec(dllexport)
HWND APIENTRY WinQueryWindow ( HWND, SHORT, BOOL );

#define  QW_OWNER                    GW_OWNER
#define  QW_TOP                      GW_HWNDFIRST
#define  QW_BOTTOM                   GW_HWNDLAST
#define  QW_NEXT                     GW_HWNDNEXT
#define  QW_PREVIOUS                 GW_HWNDPREV
#define  QW_PARENT                   4711

/**********************************************************************/
/* anchor block handling                                              */
/**********************************************************************/
#define HAB             HANDLE

/**********************************************************************/
/* one might probably use NULL as default, but we use our UTL-function*/
/* which stores the anchor block for PM - we store the instead instead*/
/**********************************************************************/
#define WinQueryAnchorBlock( a )     ((HAB) UtlQueryULong( QL_HAB ))
/**********************************************************************/
/* resource strings                                                   */
/**********************************************************************/
#define WinLoadString(a,b,c,d,e) LoadString(b,c,e,d)
#define WinLoadAccelTable(a,b,c) LoadAccelerators(b, MAKEINTRESOURCE(c))
#define WinLoadPointer(desktop,mod,id) LoadIcon( mod, MAKEINTRESOURCE(id))
#define WinDestroyPointer( handle) DestroyIcon( handle )


/**********************************************************************/
/* Load Dynamic link module handling ....                             */
/**********************************************************************/
/***   DosLoadModule - Load dynamic link module
 *     Loads a dynamic link module and returns a handle for the module.
 */
// typedef USHORT    HMODULE;  /* hmod */
typedef HMODULE FAR *PHMODULE;

__declspec(dllexport)
USHORT APIENTRY DosLoadModule(
        PSZ pszFailName,                /* object name buffer                 */
        USHORT cbFileName,              /* length of object name buffer       */
        PSZ pszModName,                 /* dynamic link module name string    */
        PHMODULE hmod);                 /* module handle (returned)           */

/***   DosFreeModule - Free dynamic link module
 *     Frees the reference to a dynamic link module for a process. When the
 *     dynamic link module is no longer needed by any process, the module is
 *     freed from system memory.
 */
__declspec(dllexport)
USHORT APIENTRY DosFreeModule(
        HMODULE hmod);                  /* module handle                      */

/***   DosGetProcAddr - Get dynamic link procedure address
 *     Returns a far address to a desired procedure within a dynamic link
 *     module.
 */
__declspec(dllexport)
USHORT APIENTRY DosGetProcAddr(
        HMODULE hmod,          /* handle of module where procedure is located */
        PSZ pszProcName,       /* name of module where procedure is located   */
        PFN FAR * ppfnProcAddr);        /* procedure address (returned)       */

/***   DosGetModHandle - Get dynamic link module handle
 *     Returns a handle to a previously loaded dynamic link module.
 */
USHORT APIENTRY DosGetModHandle(
        PSZ pszModName,                 /* dynamic link module name           */
        PHMODULE phmod);                /* module handle (returned)           */

/***   DosGetModName - Get dynamic link module name
 *     Returns the fully qualified drive, path, filename, and extension
 *     associated with a referenced module handle.
 */
USHORT APIENTRY DosGetModName(
        HMODULE hmod,            /* module handle                             */
        USHORT cbBuf,            /* max length of buffer where name is stored */
        PCHAR pchBuf);           /* buffer (returned) where info is stored    */

/***   DosError - Enable hard error processing
 *     Allows a process to disable user notification (from OS/2) on hard errors
 *     and program exceptions.
 */
USHORT APIENTRY DosError(
        USHORT fEnable);                /* action flag (bit field)            */
/***   DosInsMessage - Insert variable text strings in message
 *
 *     Inserts variable text string information into the body of a message.
 *     This is useful when messages are loaded before insertion text strings
 *     are known.
 *
 */
__declspec(dllexport)
USHORT APIENTRY DosInsMessage(
        PCHAR FAR * ppchVTable,         /* table of variables to insert   */
        USHORT usVCount,                /* number of variables            */
        PSZ pszMsg,                     /* address of input message       */
        ULONG cbMsg,                   /* length of input message        */
        PCHAR pchBuf,                   /* data area address to return    */
                                        /* message to                     */
        ULONG  cbBuf,                   /* length of data area (returned) */
        PULONG pcbMsg);                /* length of updated messsage     */

__declspec(dllexport)
USHORT APIENTRY DosInsMessageW(
        PCHAR_W FAR * ppchVTable,         /* table of variables to insert   */
        USHORT usVCount,                  /* number of variables            */
        PSZ_W pszMsg,                     /* address of input message       */
        ULONG cbMsg,                     /* length of input message        */
        PCHAR_W pchBuf,                   /* data area address to return    */
                                          /* message to                     */
        ULONG   cbBuf,                     /* length of data area (returned) */
        PULONG  pcbMsg);                  /* length of updated messsage     */

/***   DosGetMessage - Get message out of text file
 *
 *     Retrieves a message from a message file and inserts variable information
 *     into the body of the message.
 *
 */
__declspec(dllexport)
USHORT APIENTRY DosGetMessage (
  PCHAR FAR * ppchVTable ,             // table of variables to insert
  USHORT  usVCount,                    // number of variables
  PCHAR pchBuf,                        // data area (where message is returned)
  ULONG  cbBuf,                        // length of data area
  USHORT usMsgNum,                     // number of the message requested
  PSZ pszFileName,                     // message path and file name
  PULONG pcbMsg);                     // length of message (returned)

/**********************************************************************/
/* DosRead/Write substitutes...                                       */
/**********************************************************************/

/***   DosCopy - Copy file */
USHORT APIENTRY  DosCopy (
        PSZ pszSrc,                     /* old path name                   */
        PSZ pszDst,                     /* new path name                   */
        USHORT usOpt,                   /* operation mode                  */
        ULONG ulReserved);              /* reserved (must be 0)            */

/***   DosNewSize - Change file size
 *
 *     Changes the size of a file.
 *
 */

__declspec(dllexport)
USHORT APIENTRY DosNewSize(
        HFILE hf,                       /* file handle     */
        ULONG ulNewSize);               /* file's new size */

/**********************************************************************/
/* file handling                                                      */
/**********************************************************************/
/* DosCopy control bits; may be or'ed together */
#define DCPY_EXISTING   0x00001         /* Copy even if target exists */
#define DCPY_APPEND     0x00002         /* Append to existing file,   */

  /* DosOpen() actions */

  #define FILE_EXISTED    0x0001
  #define FILE_CREATED    0x0002
  #define FILE_TRUNCATED  0x0003

  // the following defines are NOT the flags used by Win32!
  // these are internal-use-only flags and are converted
  // in UtlOpen to the flags required for Win32
  #define OPEN_ACTION_OPEN_IF_EXISTS     0x0001
  #define OPEN_ACTION_REPLACE_IF_EXISTS  0x0002
  #define OPEN_ACTION_CREATE_IF_NEW      0x0010
  #define OPEN_ACTION_FAIL_IF_EXISTS     0x0000
  #define OPEN_ACTION_FAIL_IF_NEW        0x0000
  #define FILE_OPEN                      0x0001
  #define FILE_TRUNCATE                  0x0002
  #define FILE_CREATE                    0x0010

  #define OPEN_ACCESS_READONLY         0x0100
  #define OPEN_ACCESS_WRITEONLY        0x0200
  #define OPEN_ACCESS_READWRITE        0x0300
  #define OPEN_FLAGS_NOINHERIT         0x0400
  #define OPEN_FLAGS_WRITE_THROUGH     0x4000
  #define OPEN_SHARE_DENYREADWRITE     0x3000
  #define OPEN_SHARE_DENYWRITE         0x2000
  #define OPEN_SHARE_DENYREAD          0x1000
  #define OPEN_SHARE_DENYNONE          0x0000


/**********************************************************************/
/* the following flags are not available under Windows                */
/**********************************************************************/
#define OPEN_FLAGS_FAIL_ON_ERROR     0
#define OPEN_FLAGS_NO_CACHE          0
#define OPEN_FLAGS_SEQUENTIAL        0

/**********************************************************************/
/* Defines for DosFindFirst/DosFindNext                               */
/**********************************************************************/
        #define FILE_NORMAL     FILE_ATTRIBUTE_NORMAL
        #define FILE_READONLY   FILE_ATTRIBUTE_READONLY
        #define FILE_HIDDEN     FILE_ATTRIBUTE_HIDDEN
        #define FILE_SYSTEM     FILE_ATTRIBUTE_SYSTEM
        #define FILE_DIRECTORY  FILE_ATTRIBUTE_DIRECTORY
        #define FILE_ARCHIVED   FILE_ATTRIBUTE_ARCHIVE


  #define HDIR_SYSTEM  (HANDLE)0x0001     /* Use system handle (1)         */
  #define HDIR_CREATE  (HANDLE)0xFFFF     /* Allocate a new, unused handle */
  typedef HANDLE HDIR;           /* hdir */

typedef HDIR FAR *PHDIR;

  #define FILEFINDBUF   WIN32_FIND_DATA
  #define PFILEFINDBUF  LPWIN32_FIND_DATA


/***    FSAllocate - structure of file system allocation */

typedef struct _FSALLOCATE {
    ULONG  idFileSystem;                /* file system ID                     */
    ULONG  cSectorUnit;                 /* number sectors per allocation unit */
    ULONG  cUnit;                       /* number of allocation units         */
    ULONG  cUnitAvail;                  /* avalaible allocation units         */
    USHORT cbSector;                    /* bytes per sector                   */
} FSALLOCATE;
typedef FSALLOCATE FAR *PFSALLOCATE;

/**********************************************************************/
/* listbox messages .....                                             */
/**********************************************************************/

/**********************************************************************/
/* List box notification messages                                     */
/**********************************************************************/
#define LN_SELECT                  LBN_SELCHANGE
#define LN_SETFOCUS                LBN_SETFOCUS
#define LN_KILLFOCUS               LBN_KILLFOCUS
#define LN_SCROLL                  LBN_SELCANCEL
#define LN_ENTER                   LBN_DBLCLK
#define CBN_EFCHANGE               CBN_EDITCHANGE
#define BN_DBLCLICKED              BN_DOUBLECLICKED

/* List box messages */

#define LM_QUERYITEMCOUNT          LB_GETCOUNT
#define LM_INSERTITEM              LB_ADDSTRING
#define LM_SETTOPINDEX             LB_SETTOPINDEX
#define LM_DELETEITEM              LB_DELETESTRING
#define LM_SELECTITEM              LB_SETCURSEL
#define LM_QUERYSELECTION          LB_GETCURSEL
//  #define LM_SETITEMTEXT             0x0166
#define LM_QUERYITEMTEXTLENGTH     LB_GETTEXTLEN
#define LM_QUERYITEMTEXT           LB_GETTEXT

#define LM_SETITEMHANDLE           LB_SETITEMDATA
#define LM_QUERYITEMHANDLE         LB_GETITEMDATA
#define LM_SEARCHSTRING            LB_FINDSTRING
#define LM_SETITEMHEIGHT           LB_SETITEMHEIGHT
#define LM_QUERYTOPINDEX           LB_GETTOPINDEX
#define LM_DELETEALL               LB_RESETCONTENT

/* List box constants */

#define LIT_ERROR                  LB_ERR
#define LIT_MEMERROR               LB_ERRSPACE
#define LIT_NONE                   LB_ERR
#define LIT_FIRST                  LB_ERR

/**********************************************************************/
/* LM_INSERTITEM messages                                             */
/* only inserting/appending is supported                              */
/* In Windows LBS_SORT as style should be set in the listbox creation */
/**********************************************************************/
#define LIT_END                    (-1)
#define LIT_SORTASCENDING          (-1)
#define LIT_SORTDESCENDING         (-1)

/* For LM_SEARCHSTRING msg */

//#define LSS_SUBSTRING              0x0001
//#define LSS_PREFIX                 0x0002
//#define LSS_CASESENSITIVE          0x0004

/**********************************************************************/
/* Menu Control Messages                                              */
/**********************************************************************/
//#define MM_INSERTITEM              MF_INSERT
//#define MM_DELETEITEM              MF_DELETE
//#define MM_QUERYITEM               0x0182
//#define MM_SETITEM                 0x0183
//#define MM_QUERYITEMCOUNT          0x0184
//#define MM_STARTMENUMODE           0x0185
//#define MM_ENDMENUMODE             0x0186
//#define MM_DISMISSMENU             0x0187
//#define MM_REMOVEITEM              0x0188
//#define MM_SELECTITEM              0x0189
//#define MM_QUERYSELITEMID          0x018a
//#define MM_QUERYITEMTEXT           0x018b
//#define MM_QUERYITEMTEXTLENGTH     0x018c
//#define MM_SETITEMHANDLE           0x018d
//#define MM_SETITEMTEXT             0x018e
//#define MM_ITEMPOSITIONFROMID      0x018f
//#define MM_ITEMIDFROMPOSITION      0x0190
//#define MM_QUERYITEMATTR           0x0191
//#define MM_SETITEMATTR             0x0192
//#define MM_ISITEMVALID             0x0193


/**********************************************************************/
/* profile handling                                                   */
/**********************************************************************/
#define  PM_National                  intl
#define  WinQueryProfileInt( a, b, c, d) GetProfileInt( b, c, d)
#define  WinQueryProfileString( a, b, c, d, e, f) GetProfileString( b, c, d, e, f)
#define  WinSetProfileString( a, b, c, d ) WriteProfileString( b, c, d )
#define  WinWriteProfileString( a, b, c, d ) WriteProfileString( b, c, d )
#define  PrfQueryProfileString( a, b, c, d, e, f) GetProfileString( b, c, d, e, f)
#define  PrfWriteProfileString( a, b, c, d ) WriteProfileString( b, c, d )

/**********************************************************************/
/* file time and date types ....                                      */
/**********************************************************************/

typedef struct _FTIME {          /* ftime */
    unsigned short twosecs : 5;
    unsigned short minutes : 6;
    unsigned short hours   : 5;
} FTIME;
typedef FTIME FAR *PFTIME;

typedef struct _FDATE {          /* fdate */
    unsigned short day     : 5;
    unsigned short month   : 4;
    unsigned short year    : 7;
} FDATE;
typedef FDATE FAR *PFDATE;

typedef struct _FILESTATUS {     /* fsts  */
    FDATE  fdateCreation;               /* date of file creation   */
    FTIME  ftimeCreation;               /* time of file creation   */
    FDATE  fdateLastAccess;             /* date of last access     */
    FTIME  ftimeLastAccess;             /* time of last access     */
    FDATE  fdateLastWrite;              /* date of last write      */
    FTIME  ftimeLastWrite;              /* time of last write      */
    ULONG  cbFile;                      /* file size (end of data) */
    ULONG  cbFileAlloc;                 /* file allocated size     */
    USHORT attrFile;                    /* attributes of the file  */
} FILESTATUS;
typedef FILESTATUS FAR *PFILESTATUS;

/* File info levels: All listed API's */
#define FIL_STANDARD            1       /* Info level 1, standard file info */

/**********************************************************************/
/* OS/2 Message File header ...                                       */
/**********************************************************************/
typedef  struct _MESSAGEFILE
{
    CHAR    chID[ 8 ];                 // file id
    CHAR    chCompID[ 3 ];             // component id
    USHORT  usNumMsg;                  // number of messages
    USHORT  usStartNum;                // starting number
    USHORT  usUseShort;                // use short for index
    CHAR    chFiller[14];              // not determined
} MESSAGEFILE;

/**********************************************************************/
/* substitute scroll bar messages                                     */
/**********************************************************************/
#define SB_SLIDERPOSITION     SB_THUMBPOSITION
/**********************************************************************/
/* substitute Messagebox returns                                      */
/**********************************************************************/
#define MB_CANCEL     MB_OK            // windows only provides OK button
#define MB_ENTER       MB_OK
#define MB_ENTERCANCEL MB_OKCANCEL
#define MB_INFORMATION MB_ICONINFORMATION
#define MB_QUERY       MB_ICONQUESTION
#define MB_WARNING     MB_ICONEXCLAMATION  /* Message box IDs */
#define MB_ERROR       MB_ICONSTOP
#define MB_MOVEABLE    0

#define MBID_YES      IDYES
#define MBID_NO       IDNO
#define MBID_RETRY    IDRETRY
#define MBID_OK       IDOK
#define MBID_CANCEL   IDCANCEL
#define MBID_ABORT    IDABORT
#define MBID_IGNORE   IDIGNORE


/**********************************************************************/
/* WinAlarm codes/beeps                                               */
/**********************************************************************/
#define WA_WARNING                 -1
#define WA_NOTE                    -1
#define WA_ERROR                   -1
#define WA_CWINALARMS              -1
/**********************************************************************/
/* windows functions macros                                           */
/**********************************************************************/
  #define WinSetPointer(d,p)        SetCursor(p)
  #define WinWindowFromPoint(a,b,c,d) WindowFromPoint(b)
  #define HINI_USERPROFILE          0
  #define WinQueryWindowText(h,l,s) GetWindowText(h,s,l)
  #define WinSendDlgItemMsg         SendDlgItemMessage
  #define WinQueryPointerPos(d,p)   GetCursorPos(p)
  #define WinSetPointerPos(d,x,y)   SetCursorPos(x,y)
  /********************************************************************/
  /* under OS/2 WinSetActiveWindow also used to set the input focus   */
  /* to the active window...                                          */
  /********************************************************************/
  #define WinSetActiveWindow(d,w)   \
  {                                 \
      SetActiveWindow(w);           \
      SetFocus( w );                \
  }
  #define WinQueryFocus(d,f)        GetFocus()
  #define WinSetFocus(d,f)          SetFocus(f)
  #define GpiSetColor      SetTextColor
  #define GpiSetBackColor  SetBkColor
  #define WinPostMsg(a, b, c, d)     CPLUSPLUS   PostMessage(a, b, (WPARAM)c, (LPARAM)d)
  #define WinSendMsg(a, b, c, d)     CPLUSPLUS   SendMessage(a, b, (WPARAM)c, (LPARAM)d)
  #define WinDefWindowProc DefWindowProc
  #define GpiQueryFontMetrics(a,b,c) GetTextMetrics(a,c);
  #define FONTMETRICS TEXTMETRIC
  #define WinQuerySysValue(a,b) GetSystemMetrics(b)
    #define SV_CXMINMAXBUTTON SM_CXSIZE
    #define SV_CYMINMAXBUTTON SM_CYSIZE
    #define SV_CYTITLEBAR     SM_CYCAPTION
    #define SV_CYMENU         SM_CYMENU
    #define SV_CXSCREEN       SM_CXSCREEN
    #define SV_CYSCREEN       SM_CYSCREEN
    #define SV_CXFULLSCREEN   SM_CXFULLSCREEN
    #define SV_CYFULLSCREEN   SM_CXFULLSCREEN
    #define SV_CXSIZEBORDER   SM_CXFRAME
    #define SV_CYSIZEBORDER   SM_CYFRAME
    #define SV_CXDLGFRAME     SM_CXDLGFRAME
    #define SV_CYDLGFRAME     SM_CYDLGFRAME
    #define SV_CXBORDER       SM_CXBORDER
    #define SV_CYBORDER       SM_CYBORDER
    #define SV_CXVSCROLL      SM_CXVSCROLL
    #define SV_CYVSCROLL      SM_CYVSCROLL
    #define SV_CXHSCROLL      SM_CXHSCROLL
    #define SV_CYHSCROLL      SM_CYHSCROLL
  #define DosBeep(a,b) MessageBeep(0)
  #define WinAlarm( a, b )               MessageBeep( (UINT) b )
  #define WINSETWINDOWPTR( a, b, c)      SetWindowLong( a, b, (LONG) c)
  #define WINQUERYWINDOWPTR( a, b)       GetWindowLong( a, b)

  #define WinDestroyWindow( a )          DestroyWindow( a )
  #define WinInvalidateRegion(a, b, c)   InvalidateRgn(a, (HRGN) b, c)

  #define WinShowWindow( a, b )          ShowWindow(a,(b) ? SW_SHOW : SW_HIDE)

  #define WinIsWindowVisible( a )        IsWindowVisible( a )
  #define WinIsWindowEnabled( a )        IsWindowEnabled( a )
  #define WinDismissDlg(a,b)             EndDialog(a,b)
  /********************************************************************/
  /* DefDlgProc should not be called for standard dialogs under Win   */
  /* Doing this would cause a recursion....                           */
  /********************************************************************/
  #define WinDefDlgProc(a,b,c,d)         FALSE
  #define WinSetWindowText               SetWindowText
  #define WinEnableWindow                EnableWindow
  #define WinWindowFromID                GetDlgItem

  #define WinMessageBox(a,b,c,d,e,f)     MessageBox(b,c,d,f)
    #define MBID_OK      IDOK
    #define DID_OK       IDOK                /* Dialog IDs */
    #define DID_CANCEL   IDCANCEL
    #define DID_ERROR    0xffff        // IDABORT
  #define WM_INITDLG     WM_INITDIALOG
  #define FACESIZE       35

  #define CLR_BLACK      RGB(0,0,0)
  #define CLR_DARKBLUE   RGB(0,0,128)
  #define CLR_DARKGREEN  RGB(0,128,0)
  #define CLR_DARKCYAN   RGB(0,128,128)
  #define CLR_DARKRED    RGB(128,0,0)
  #define CLR_DARKPINK   RGB(128,0,128)
  #define CLR_BROWN      RGB(128,128,0)
  #define CLR_PALEGRAY   RGB(192,192,192)
  #define CLR_DARKGRAY   RGB(128,128,128)
  #define CLR_BLUE       RGB(0,0,255)
  #define CLR_GREEN      RGB(0,255,0)
  #define CLR_CYAN       RGB(0,255,255)
  #define CLR_RED        RGB(255,0,0)
  #define CLR_PINK       RGB(255,0,255)
  #define CLR_YELLOW     RGB(255,255,0)
  #define CLR_WHITE      RGB(255,255,255)

/**********************************************************************/
/* RECTL struct of PM has to be converted to windows structure        */
/* using primitive functions...                                       */
/**********************************************************************/
typedef RECTL FAR  *PRECTL;
__declspec(dllexport)
BOOL    APIENTRY WinQueryWindowRect(HWND hwnd, PRECTL prclDest);


/* Macros to make an MPARAM from standard types. */
/*                                                                    */
#define MRESULT                    LRESULT

/* Frame window styles */
#define FCF_STANDARD     \
   FCF_TITLEBAR | FCF_SYSMENU | FCF_MENU | FCF_SIZEBORDER | FCF_MINMAX | \
   FCF_ICON | FCF_ACCELTABLE | FCF_SHELLPOSITION | FCF_TASKLIST | FCF_PALETTE_NORMAL

#define FCF_TITLEBAR               WS_CAPTION
#define FCF_SYSMENU                WS_SYSMENU
#define FCF_MENU                   0
#define FCF_SIZEBORDER             WS_THICKFRAME
#define FCF_MINBUTTON              WS_MINIMIZEBOX
#define FCF_MAXBUTTON              WS_MAXIMIZEBOX
#define FCF_MINMAX                 WS_MAXIMIZEBOX | WS_MINIMIZEBOX
#define FCF_VERTSCROLL             WS_VSCROLL
#define FCF_HORZSCROLL             WS_HSCROLL
#define FCF_DLGBORDER              WS_DLGFRAME
#define FCF_BORDER                 WS_BORDER
#define FCF_SHELLPOSITION          0
#define FCF_TASKLIST               0
#define FCF_NOBYTEALIGN            0
#define FCF_NOMOVEWITHOWNER        0
#define FCF_ICON                   0
#define FCF_ACCELTABLE             0
#define FCF_SYSMODAL               0
#define FCF_SCREENALIGN            0
#define FCF_MOUSEALIGN             0

#define FCF_PALETTE_NORMAL         0
#define FCF_PALETTE_HELP           0
#define FCF_PALETTE_POPUPODD       0
#define FCF_PALETTE_POPUPEVEN      0
#define FCF_DBE_APPSTAT            0



// typedef LHANDLE   HAB;      /* hab  */
//typedef HAB FAR *PHAB;

/*** Common GPI/DEV types */

#define  HPS  HDC
#define  HPOINTER HICON
//typedef LHANDLE   HPS;      /* hps  */
//typedef HPS FAR *PHPS;

typedef MRESULT (pascal FAR *PFNWP)(HWND, UINT, WPARAM, LPARAM);

/**********************************************************************/
/* constants                                                          */
/**********************************************************************/
/*
 * CCHMAXPATH is the maximum fully qualified path name length including
 * the drive letter, colon, backslashes and terminating NULL.
 */
#define CCHMAXPATH      260

/*
 * CCHMAXPATHCOMP is the maximum individual path component name length
 * including a terminating NULL.
 */
#define CCHMAXPATHCOMP  256

/**********************************************************************/
/* DBCS definitions                                                   */
/**********************************************************************/

#define NON_DBCS_CP     0x0000
#define DBCS_CP         0x0001

#define SBCS            0x0000
#define DBCS_1ST        0x0001
#define DBCS_2ND        0x0002
/* the global variables */

#define _dbcs_type      FALSE

/* the character classification macro definitions */
#define isdbcs1(c)      ((IsDBCSLeadByte((BYTE) c )) ? DBCS_1ST : SBCS)
#define isdbcs1ex(usCP, c)  ((IsDBCSLeadByteEx( usCP, (BYTE) c )) ? DBCS_1ST : SBCS )

#if defined(WIN32BIT) & defined(DLLIMPORTDBCSCP)
  __declspec(dllimport) unsigned int _dbcs_cp;
  __declspec(dllimport) unsigned int _IsUnicodeSystem;
#else
  extern unsigned int _dbcs_cp;
  extern unsigned int _IsUnicodeSystem;
#endif
BOOL IsUnicodeSystem();
unsigned int getctype(const char *string, unsigned int n) ;
VOID initdbcs();
__declspec(dllexport)
VOID InitUnicode();
BOOL UserDefaultLangIsAPLang();
__declspec(dllexport)
BOOL IsAPLang(char *pszLang);

VOID initdbcsLang( PSZ pTargetLang );
__declspec(dllexport)
BOOL IsDBCS_CP(ULONG ulCP);



/* the definitions of the compatible functions */
//#define  mc_memccpy       memccpy
//#define  mc_memchr        memchr
//#define  mc_memcpy        memcpy
//#define  mc_memicmp       memicmp
//#define  mc_memmove       memmove
//#define  mc_memset        memset

//#define  mc_strchr        strchr
//#define  mc_strcspn       strcspn
//#define  mc_stricmp       stricmp
//#define  mc_strlwr        strlwr
//#define  mc_strncat       strncat
//#define  mc_strncpy       strncpy
//#define  mc_strnicmp      strnicmp
//#define  mc_strnset       strnset
//#define  mc_strpbrk       strpbrk
//#define  mc_strrchr       strrchr
//#define  mc_strrev        strrev
//#define  mc_strset        strset
//#define  mc_strspn        strspn
//#define  mc_strstr        strstr
//#define  mc_strtok        strtok

#define  mc_strupr        AnsiUpper
//
/* the definitions of the equivalent functions */
//#define  ms_memccpy       memccpy
//#define  ms_memchr        memchr
//#define  ms_memcpy        memcpy
//#define  ms_memmove       memmove
//#define  ms_memset        memset
//#define  ms_strchr        strchr
//#define  ms_strncat       strncat
//#define  ms_strncpy       strncpy
//#define  ms_strnset       strnset
//#define  ms_strrchr       strrchr
//#define  ms_strset        strset


/**********************************************************************/
/* Equivalent to OS/2 SWP structure                                   */
/* Note: bFiller1 and bFiller2 have been inserted to enlarge the      */
/*       structure to the size of the OS/2 version (HWND has a        */
/*       size of 4 bytes under OS/2 and a size of 2 bytes under       */
/*       Windows)                                                     */
/**********************************************************************/
typedef struct _SWP { /* swp */
    USHORT  fs;
    SHORT   cy;
    SHORT   cx;
    SHORT   y;
    SHORT   x;
    HWND    hwndInsertBehind;
    HWND    hwnd;
} SWP;
typedef SWP FAR *PSWP;

typedef SIZEL FAR *PSIZEL;

/**********************************************************************/
/* thread IDs..                                                       */
/*   they should not occure in the real code later on - or we have a  */
/*   very clever solution                                             */
/**********************************************************************/
typedef USHORT    TID;      /* tid  */
typedef TID FAR *PTID;

/**********************************************************************/
/* SWP defines                                                        */
/* values with 0x0000 are windows default - we have to explicitly     */
/* negate them...                                                     */
/**********************************************************************/
#define EQF_SWP_SIZE                   0x0001
#define EQF_SWP_MOVE                   0x0002
#define EQF_SWP_ZORDER                 0x0004
#define EQF_SWP_SHOW                   0x0008
#define EQF_SWP_HIDE                   0x0010
#define EQF_SWP_NOREDRAW               0x0020
#define EQF_SWP_NOADJUST               0x0040
#define EQF_SWP_ACTIVATE               0x0080
#define EQF_SWP_DEACTIVATE             0x0100
#define EQF_SWP_EXTSTATECHANGE         0x0200
#define EQF_SWP_MINIMIZE               0x0400
#define EQF_SWP_MAXIMIZE               0x0800
#define EQF_SWP_RESTORE                0x1000
#define EQF_SWP_FOCUSACTIVATE          0x2000
#define EQF_SWP_FOCUSDEACTIVATE        0x4000

#define WinIsWindow( a, b )           IsWindow( b )
__declspec(dllexport)
BOOL    APIENTRY WinQueryWindowPos(HWND hwnd, PSWP pSwp);
__declspec(dllexport)
BOOL   APIENTRY WinSetWindowPos(HWND hwnd, HWND hwndInsertBehind, LONG x,
                                LONG y, LONG cx, LONG cy, USHORT fs);

/**********************************************************************/
/* do defines for the font cell sizes settings...                     */
/**********************************************************************/
typedef struct _VIOFONTCELLSIZE { /* viof */
       LONG cx;
       LONG cy;
       } VIOFONTCELLSIZE;
typedef VIOFONTCELLSIZE FAR * PVIOFONTCELLSIZE;

typedef struct _VIOSIZECOUNT { /* vios */
       LONG maxcount;
       LONG count;
       } VIOSIZECOUNT;
typedef VIOSIZECOUNT FAR * PVIOSIZECOUNT;

// VIOCURSORINFO is not used any more
// but keep it until it is sure! (RJ 03/02/12)
typedef struct _VIOCURSORINFO {
        USHORT   yStart; /* cursor start line                           */
        USHORT   cEnd;   /* cursor end line                             */
        USHORT   cx;     /* cursor width                                */
        USHORT   attr;   /* -1=hidden cursor, any other=normal cursor   */
        } VIOCURSORINFO;
typedef VIOCURSORINFO FAR *PVIOCURSORINFO;
//
// sizeof(_NEWVIOCURSORINFO) = sizeof(_VIOCURSORINFO)
// this is prereq and should not be changed! since within TBDOCUMENT
// structure VIOCURSORINFO is replaced by NEWVIOCURSORINFO!!

typedef struct _NEWVIOCURSORINFO {
        ULONG    cEnd;   /* cursor end line                             */
        ULONG    cx;     /* cursor width                                */
       } NEWVIOCURSORINFO;
typedef NEWVIOCURSORINFO FAR *PNEWVIOCURSORINFO;

/**********************************************************************/
/* mouse messages ...                                                 */
/**********************************************************************/
#define WM_BUTTON1DOWN    WM_LBUTTONDOWN
#define WM_BUTTON1UP      WM_LBUTTONUP
#define WM_BUTTON2DOWN    WM_RBUTTONDOWN
#define WM_BUTTON2UP      WM_RBUTTONUP
#define WM_BUTTON1DBLCLK  WM_LBUTTONDBLCLK
#define WM_BUTTON2DBLCLK  WM_RBUTTONDBLCLK
#define WM_BUTTON2DBLCLK  WM_RBUTTONDBLCLK

/**********************************************************************/
/* define Menu IDs                                                    */
/**********************************************************************/
#define FID_SYSMENU                0x8002
#define FID_TITLEBAR               0x8003
#define FID_MINMAX                 0x8004
#define FID_MENU                   0x8005
#define FID_VERTSCROLL             0x8006
#define FID_HORZSCROLL             0x8007
#define FID_CLIENT                 0x8008
/* Note 0x8009 is reserved */
#define FID_DBE_APPSTAT            0x8010
#define FID_DBE_KBDSTAT            0x8011
#define FID_DBE_PECIC              0x8012
#define FID_DBE_KKPOPUP            0x8013

/**********************************************************************/
/* no border style                                                    */
/**********************************************************************/
#define BS_NOBORDER                0

/**********************************************************************/
/* draw a bitmap..                                                    */
/**********************************************************************/
typedef POINTL  FAR  *PPOINTL;

__declspec(dllexport)
BOOL  APIENTRY WinDrawBitmap(HDC hdc, HBITMAP hbm, PRECTL pwrcSrc,
                             PPOINTL pptlDst, LONG clrFore, LONG clrBack,
                             USHORT fs);

#define DBM_NORMAL                 0x0000
#define DBM_INVERT                 0x0001
#define DBM_HALFTONE               0x0002
#define DBM_STRETCH                0x0004
#define DBM_IMAGEATTRS             0x0008


/**********************************************************************/
/* system and other color settings ....                               */
/**********************************************************************/
#define DT_HALFTONE  0

#define SYSCLR_HILITEFOREGROUND         (-33L)
#define SYSCLR_HILITEBACKGROUND         (-32L)
#define SYSCLR_SCROLLBAR                (-25L)
#define SYSCLR_BACKGROUND               (-24L)
#define SYSCLR_ACTIVETITLE              (-23L)
#define SYSCLR_INACTIVETITLE            (-22L)
#define SYSCLR_MENU                     (-21L)
#define SYSCLR_WINDOW                   (-20L)
#define SYSCLR_WINDOWFRAME              (-19L)
#define SYSCLR_MENUTEXT                 (-18L)
#define SYSCLR_WINDOWTEXT               (-17L)
#define SYSCLR_TITLETEXT                (-16L)
#define SYSCLR_ACTIVEBORDER             (-15L)
#define SYSCLR_INACTIVEBORDER           (-14L)
#define SYSCLR_APPWORKSPACE             (-13L)

#define CLR_BACKGROUND SYSCLR_BACKGROUND

__declspec(dllexport)
COLORREF UtlGetColorref( LONG lColor );

// set the drive types defines (use same values as used under Win/NT)
#define DRIVE_CDROM      5
#define DRIVE_RAM        6


// substitute for DosExitList
//  -- avoid compiler warnings....
#define EXLST_ADD       1
#define EXLST_REMOVE    2
#define EXLST_EXIT      3
#define DosExitList( a, b )

// Substitute for DosSleep
#define DosSleep( a ) Sleep( a )

// ignore DosEnterCritSec
#define DosEnterCritSec()

/**********************************************************************/
/* get arguments from invocation                                      */
/**********************************************************************/
__declspec(dllexport)
void UtlGetArgs
(
  HINSTANCE hInstance,                 // application instance
  PSZ pCmdLine,                        // passed command line
  int *argc,                           // return number of arguments
  char ** argv                         // return pointer array
);

/**********************************************************************/
/* delete file                                                        */
/**********************************************************************/
#define DosDelete( a, b )   _unlink( a )

/**********************************************************************/
/* Function to simulate LM_SETITEMTEXT message                        */
/**********************************************************************/
__declspec(dllexport)
SHORT LBSetItemText( HWND hwnd, SHORT index, PSZ pText );
__declspec(dllexport)
SHORT LBSetItemTextW( HWND hwnd, SHORT index, PSZ_W pText );

/**********************************************************************/
/* ANSI to OEM and vice versa conversion under DBCS environment       */
/**********************************************************************/
VOID AnsiToOem850Buff( PSZ, PSZ, USHORT );
VOID Oem850ToAnsiBuff( PSZ, PSZ, USHORT );

/**********************************************************************/
/* special font needed to allow dbcs input in the dialog EF's         */
/**********************************************************************/

#define SHEIGHTINCTRL    3

__declspec(dllexport)
VOID DelCtrlFont ( HWND , USHORT );              //del special font for ctrl
VOID DelCtrlFontHwnd ( HWND );
__declspec(dllexport)
USHORT GetCharSet (VOID);                        // get charset (SHIFTJIS etc.)
__declspec(dllexport)
VOID SetCtrlFnt ( HWND , USHORT , USHORT, USHORT  );       //set special font for ctrl
VOID SetCtrlFntHwnd ( HWND , USHORT  );
__declspec(dllexport)
VOID ClearIME
(
   HWND    hwndDlg                        // handle of dialog window
);

/**********************************************************************/
/* File lock structure                                                */
/**********************************************************************/
typedef struct _FILELOCK {
    LONG    lOffset;
    LONG    lRange;
} FILELOCK;
typedef FILELOCK  FAR  *PFILELOCK;


USHORT WPToEQFConv ( PUCHAR pucData, PBYTE  pwzData, USHORT usLen );
USHORT EQFToWPConv ( PUCHAR pucData, PBYTE  pwzData, USHORT usLen );

/**********************************************************************/
/* OS/2 PM types directly mapped to Windows types                     */
/**********************************************************************/
#define HPOINTER    HICON

#define WS_SAVEBITS     0
#define QWL_STYLE       GWL_STYLE
#define WS_MINIMIZED    WS_MINIMIZE
#define HWND_OBJECT     HWND_DESKTOP
#define CS_SIZEREDRAW   (CS_VREDRAW | CS_HREDRAW)

/**********************************************************************/
/* Help table structures                                              */
/**********************************************************************/
/******************************************************************************/
/* HelpSubTable entry structure                                               */
/******************************************************************************/

typedef SHORT HELPSUBTABLE;
typedef SHORT far *PHELPSUBTABLE;

/******************************************************************************/
/* HelpTable entry structure                                                  */
/******************************************************************************/

typedef struct _HELPTABLE   {  /* ht */
  USHORT          idAppWindow;
  PHELPSUBTABLE   phstHelpSubTable;
  USHORT          idExtPanel;
} HELPTABLE;
typedef HELPTABLE FAR *PHELPTABLE;

/**********************************************************************/
/* Defines for system color settings                                  */
/**********************************************************************/
#define SYSCLR_WINDOWSTATICTEXT  COLOR_HIGHLIGHT  // COLOR_WINDOWTEXT

/**********************************************************************/
/* OS/2 PM functions directly mapped to Windows functions             */
/**********************************************************************/
// use macro in EQF.H
// #define WinDlgBox( parent, owner, dlgproc, hmodule, id, parm )
//   DialogBoxParam( hmodule, id, owner, dlgproc, (LPARAM)parm )
#define QWL_STYLE GWL_STYLE
#define WinQueryWindowULong( hwnd, id ) \
  GetWindowLong( hwnd, id )
#define WinQueryWindowUShort( hwnd, id ) \
  GetWindowWord( hwnd, id )
#define WinStartTimer( hab, hwnd, id, timeout ) \
  SetTimer( hwnd, id, (UINT) timeout, NULL )
#define WinStopTimer( hab, hwnd, id ) \
  KillTimer( hwnd, id )

/**********************************************************************/
/* OS/2 Date and time structure used for DosGetDateTime function      */
/**********************************************************************/
typedef struct _DATETIME {
    UCHAR   hours;                        /* current hour                   */
    UCHAR   minutes;                      /* current minute                 */
    UCHAR   seconds;                      /* current second                 */
    UCHAR   hundredths;                   /* current hundredths of a second */
    UCHAR   day;                          /* current day                    */
    UCHAR   month;                        /* current month                  */
    USHORT  year;                         /* current year                   */
    SHORT   timezone;                     /* minutes of time west of GMT    */
    UCHAR   weekday;                      /* current day of week            */
} DATETIME, far *PDATETIME;

/**********************************************************************/
/* OS/2 PM functions converted to Windows                             */
/**********************************************************************/
__declspec(dllexport)
BOOL APIENTRY WinRegisterClass( HINSTANCE hInstance, PSZ pszClass,
                                WNDPROC lpfnWindowProc, LONG lStyle,
                                SHORT sExtraBytes );
BOOL APIENTRY WinSetMultWindowPos( HINSTANCE hInstance, PSWP pSwp,
                                   USHORT usWindows );
__declspec(dllexport)
SHORT APIENTRY WinDrawText(HPS hps, SHORT cchText, PSZ lpchText, PRECTL prcl,
                           LONG clrFore, LONG clrBack, USHORT rgfCmd);
BOOL  APIENTRY WinMapDlgPoints( HWND hwndDlg, PPOINTL prgwptl, USHORT cwpt,
                                BOOL fCalcWindowCoords );

__declspec(dllexport)
HWND    APIENTRY WinCreateWindow(HWND hwndParent, PSZ pszClass, PSZ pszName,
                                 ULONG flStyle, LONG x, LONG y, LONG cx,
                                 LONG cy, HWND hwndOwner,
                                 HWND hwndInsertBehind, USHORT id,
                                 PVOID pCtlData, PVOID pPresParams);


/**********************************************************************/
/* OS/2 DOS Functions converted to Windows                            */
/**********************************************************************/
__declspec(dllexport)
USHORT APIENTRY DosGetDateTime( PDATETIME pDateTime );

/**********************************************************************/
/* New functions to handle messages not supported by windows          */
/**********************************************************************/
__declspec(dllexport)
SHORT GetNextSelection( HWND hwndLB, SHORT sPos );

/**********************************************************************/
/* force use of GetOEMCP- needed for 32-bit                           */
/* GetKBCodePage returns  437 for TChin/SChin/Korean under Win95!     */
/* which causes that no DBCS chars are detected!!                     */
/**********************************************************************/
#define GetKBCodePage() GetOEMCP()

/**********************************************************************/
/* allow to fill the rectangle in the selected color                  */
/**********************************************************************/
__declspec(dllexport)
void ERASERECT(HDC hdc, const RECT* pRect, LONG bg);
