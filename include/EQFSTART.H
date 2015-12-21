/*! EQFSTART.H - Header file for the Twb Startup program                      
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

#ifndef INCL_EQFSTART
#define INCL_EQFSTART

/**********************************************************************/
/* Name of MDI super class (used only in Windows environment)         */
/**********************************************************************/
#define MDISUPERCLASS "EqfMDIClient"

/**********************************************************************/
/* Name of background bitmap entry in configuration file (used only   */
/* in Windows environment)                                            */
/**********************************************************************/
#define KEY_BG_BITMAP "BackgroundBMP"

/**********************************************************************/
/* Name of file containing the control table for user handlers        */
/**********************************************************************/
#define USERHANDLERTABLE "EQFHNDLR.DAT"

/**********************************************************************/
/* Name of the file which must exist to enable the report handler     */
/* and directory (path ID) of the directory containing the file       */
/**********************************************************************/
#define REPHANDLERPREREQFILE "IBMIDDOC.TBL"
#define REPHANDLERPREREQPATH TABLE_PATH

/* EqfStart - Twb Main instance area */
typedef struct _TWB_IDA
{
     IDA_HEAD     IdaHead;              // Common header of instance area
     HPROP        hPropSys;             // system properties handle
     PPROPSYSTEM  pPropSys;             // system properties ptr
     HWND         hMenu;                // AB menu hwnd
     PMNUTBL      MenuTbl;              // tbl of submenus & submenu items
     USHORT       usInMenuWindows;      // number of hwnds in sub menu windows
     POBJLST      hwndsInMenuWindows;   // hwnds in sub mneu windows
     POBJLST      hHndlrTbl;            // list of registered handlers
     USHORT       usHndlr;              // num of registered handlers
     HWND         hObjFocus;            // window with focus on Twb client
     USHORT       usObjClass;           // saves the window class
     OBJNAME      szObject;             // buffer for object names
} TWBMAIN_IDA, *PTWBMAIN_IDA;

/**********************************************************************/
/* Structure for array of developer bitmaps and texts                 */
/**********************************************************************/
typedef struct _MATBITMAP
{
   USHORT   usID;                      // resource ID of bitmap
   CHAR     szName1[MAX_DESCRIPTION];  // text for NAME1 static
   CHAR     szName2[MAX_DESCRIPTION];  // text for NAME2 static
   CHAR     szDecr[MAX_DESCRIPTION];   // text for DESCR static
} MATBITMAP, *PMATBITMAP;


/**********************************************************************/
/* types for LOGOs                                                    */
/**********************************************************************/
#define EQFSTART    0
#define EQFITM      1
/**********************************************************************/
/* IDA for the logo dialog                                            */
/**********************************************************************/
typedef struct _LOGOIDA
{
  HBITMAP    hbmLogo;                  // handle of logo bitmap
  HBITMAP    hbmEqfLogo;               // handle of EQF logo bitmap
  USHORT     usTime;                   // timer wait time
  CHAR       szBuffer[1024];           // general purpose buffer
  CHAR       szTitle[80];              // dialog title text
  CHAR       szCopyRight[5][80];       // copyright strings
  CHAR       szRevision[80];           // revision string
  SIZEL      sizeLogoBmp;              // dimension of logo bitmap
  SIZEL      sizeEqfLogoBmp;           // dimension of EQF logo bitmap
  LONG       cxChar;                   // average character width
  LONG       cyChar;                   // average character height
  SWP        swpOkPB;                  // size and position of OK pushbutton
  USHORT     usMaxWidth;               // max width of copyright strings
  RECTL      rectTitle;                // drawinf rectange for title text line
  RECTL      rectLogo;                 // drawing rectangle for logo bitmap
  RECTL      rectEqfLogo;              // drawinf rectangle for EQF logo bitmap
  RECTL      rectCopyRight;            // drawing rectangle for copry right text
  SHORT      xDlg, yDlg, cxDlg, cyDlg; // dialog size and position
} LOGOIDA, *PLOGOIDA;


/**********************************************************************/
/* Layout of file containing control info of user handlers            */
/**********************************************************************/
typedef struct _HANDLERCONTROL
{
  USHORT           usCheckSum;         // checksum for this record
  CHAR             szModule[10];       // name of module containing handler
  CHAR             szFunction[40];     // name of handler (callback) function
  BOOL             fIsListHandler;     // TRUE = handler uses generic list handler
  CHAR             szHandler[40];      // name of handler
  USHORT           ausCommands[20];    // array of command values to be passed
                                       // on to this handler, must be terminated
                                       // by a zero value
  USHORT           ausMenuIDs[20];     // array of menu IDs which should be
                                       // enabled, must be terminated
                                       // by a zero value
} HANDLERCONTROL, *PHANDLERCONTROL;

/**********************************************************************/
/* In-memory table of user handler control information                */
/**********************************************************************/
typedef struct _USERHANDLER
{
  HMODULE          hModule;            // handle of module containing handler code
  HWND             hwnd;               // handle of handler/list window
  BOOL             fIsListHandler;     // TRUE = handler uses generic list handler
  CHAR             szHandler[40];      // name of handler
  USHORT           ausCommands[20];    // array of command values to be passed
                                       // on to this handler, must be terminated
                                       // by a zero value
  USHORT           ausMenuIDs[20];     // array of menu IDs which should be
                                       // enabled, must be terminated
                                       // by a zero value
} USERHANDLER, *PUSERHANDLER;

/**********************************************************************/
/* Function prototypes                                                */
/**********************************************************************/
VOID             TwbUnSetMessageFilterHook();
#ifdef _WINDOWS
__declspec(dllexport)
VOID             TwbSetMessageFilterHook( HINSTANCE, HWND );
__declspec(dllexport)
HWND             TwbInit( BOOL, BOOL );
#else
VOID             TwbSetMessageFilterHook( HAB, HWND );
HWND             TwbInit( BOOL, BOOL, HMQ );
#endif

HWND             TwbWinInit( BOOL );
__declspec(dllexport)
BOOL             TwbGetCheckProfileData( PSZ, PSZ, PSZ );
VOID             TwbFreeResources( VOID );
__declspec(dllexport)
VOID             EqfDisplayHelpIndex(VOID);
__declspec(dllexport)
VOID             EqfDisplayHelpForHelp(VOID);

__declspec(dllexport)
VOID             EqfIELookAndFeel (BOOL fIE);
__declspec(dllexport)
MRESULT          APIENTRY TwbMainWP( HWND, WINMSG, WPARAM, LPARAM);
VOID             TwbDrawBitmap( HWND hwnd, PMATBITMAP pBitmap );
USHORT           TwbStartTutorial( PSZ );
__declspec(dllexport)
VOID             TwbShowLogo( LONG, HWND, PSZ, USHORT, UCHAR );
INT_PTR CALLBACK TwbLogoDlgBox ( HWND, WINMSG, WPARAM, LPARAM );
VOID             TwbRestart( PSZ pszHandler, PSZ pszRestartList );
BOOL             TwbSwitchHelpTable ( PHELPSUBTABLE, PHELPSUBTABLE );
USHORT           TwbStartBook ( VOID );
MRESULT          TwbInitMenu( HWND hwnd, WPARAM mp1, LPARAM mp2 );
__declspec(dllexport)
MRESULT          TwbInitMenuMFC( HMENU hMenu, WPARAM mp1, LPARAM mp2);
BOOL             TwbStartHandler( HAB, PSZ, PFNWP, PHWND );
BOOL             TwbStartListHandler( PSZ, PFN_HANDLERCALLBACK, PHWND );
__declspec(dllexport)
MRESULT          TwbCreateMsg( HWND hwnd, WPARAM mp1, LPARAM mp2 );
BOOL             TwbCloseCheck( VOID );
BOOL             TwbStartUserHandler( HAB hab );

INT_PTR CALLBACK  UTLSERVERLISTDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK  MEMINCLUDEDLG( HWND, WINMSG, WPARAM, LPARAM );


#endif

//   End of EQFSTART.H
