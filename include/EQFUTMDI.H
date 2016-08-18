/*! EQFUTMDI.H - Header file for modeless MDI dialogs
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

// constant definitions

//old:#define GWL_MDIDIALOGPROC        0
//old:#define GWW_MDIDIALOGHFONT       4
//old:#define GWW_MDIDIALOGHMENU       6
//old:#define GWW_MDIDIALOGHWNDMENU    8
//old:#define GWW_MDIDIALOGHICON       10
//old:#define GWW_MDIDIALOGEXTRABYTES  12

#define MAXLEN_MENUNAME          64
#define MAXLEN_CLASSNAME         64
#define MAXLEN_CAPTIONTEXT       132
#define MAXLEN_TYPEFACE          64

#define WS_MDIALLOWED (WS_MINIMIZE | WS_MAXIMIZE | WS_CLIPCHILDREN |\
                       WS_DISABLED | WS_HSCROLL | WS_VSCROLL | 0x0000FFFFL)
#define WS_MDICHILD   (WS_CHILD | WS_CLIPSIBLINGS | WS_SYSMENU | WS_CAPTION |\
                       WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |\
                       WS_VISIBLE)
#define WS_MDIDLGCHILD (WS_CHILD | WS_CLIPSIBLINGS | WS_SYSMENU | WS_CAPTION |\
                       WS_DLGFRAME | WS_VISIBLE)

// This message is required for painting MDI icons

#define WM_PAINTICON 0x0026

// Notification for menu change

#define IDM_MENUCHANGE 0x8000

// atom definitions

#define ATOM_HICON  0x8000

/*** this string is used to compute the average character width ***/

#define WIDTHSTRING "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

typedef struct tagMDIDIALOGINFO
{
   int      dtItemCount ;
//   FARPROC  lpDialogProc ;
   WNDPROC  lpDialogProc ;
   HFONT    hFont ;
   HMENU    hMenu ;
   HICON    hIcon ;
   LPARAM   lParam ;
   LPSTR    lpDlgItems ;
   WORD     wUnitsX, wUnitsY ;
   BOOL     fDialogFrame;

} MDIDIALOGINFO, FAR *LPMDIDIALOGINFO ;

// macros

#define GETHINST( hWnd )    ((HINSTANCE) GetWindowWord( hWnd, GWW_HINSTANCE ))

#define DLGTOCLIENTX( x, units )   MulDiv( x, units, 4 )
#define DLGTOCLIENTY( y, units )   MulDiv( y, units, 8 )

#define WC_EQFMDIDLG  "EqfMDIDlg"

// macros

#define SET_PROP( x, atom,  y )  SetProp( x, MAKEINTATOM( atom ), y )
#define GET_PROP( x, atom )      GetProp( x, MAKEINTATOM( atom ) )
#define REMOVE_PROP( x, atom )   RemoveProp( x, MAKEINTATOM( atom ) )

// function declarations

BOOL NEAR CreateMDIDialogChildren( HWND, LPSTR, int, HFONT, WORD, WORD ) ;
__declspec(dllexport)
BOOL FAR PASCAL RegisterMDIDialog( HINSTANCE ) ;
__declspec(dllexport)
HWND FAR PASCAL CreateMDIDialogParam( HINSTANCE, LPCSTR, HWND, FARPROC,
                                      LPARAM, BOOL, HICON ) ;
__declspec(dllexport)
LRESULT FAR PASCAL MDIDialogProc( HWND, UINT, WPARAM, LPARAM ) ;
