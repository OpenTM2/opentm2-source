/*! EQF.H - General .H file 
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.

	Include Files:
	without any special defines the following files are included
	per default:

		OS2.H (w/ INCL_DOS, INCL_WIN, INCL_GPI, INCL_ERRORS, INCL_NLS)

		STDIO.H
		STDLIB.H
		STRING.H
		CTYPE.H
		DBCS.H
		PMSTDDLG.H        (w/ INCL_WINSTDSPIN + w/ INCL_WINSTDSLIDER)

		eqf.id            (Eqf PM IDs)
		eqfmsg.h          (general eqf message ids)

	special defines to control the include of files
		NO_OS2DEFS                no default defines for OS2 inclue
		NO_CINCLUDES              no include of standard c files
		NO_SPININCLUDE            no include of spinbutton defines
		NO_UTLINCLUDE             no include of utility headers
		NO_EQF_WMIDS              no EQF PM message IDs

		INCL_EQF_TM               general Transl. Memory functions
		INCL_EQF_TMREMOTE         remote Transl. Memory functions (U-Code)
		INCL_EQF_TP               public translation processor functions
		INCL_EQF_MORPH            morphologic functions
		INCL_EQF_ASD              dictionary access functions (Asd...)
		INCL_EQF_DAM              low level dict. access functions (Nlp/Dam)
		INCL_EQF_LDB              dictionary data encoding functions
		INCL_EQF_ANALYSIS         analysis functions
		INCL_EQF_TAGTABLE         tagtable defines and functions
		INCL_EQF_SLIDER           slider utility functions
		INCL_EQF_WCOUNT           word count functions
		INCL_EQF_DICT             dictionary handler functions
		INCL_EQF_FILT             dictionary filter functions
		INCL_EQF_LIST             terminology list functions
		INCL_EQF_PRINT            print functions
		INCL_EQF_DLGUTILS         dialog utilities
		INCL_EQF_DICTPRINT        dictionary print functions
		INCL_EQF_EDITORAPI        editor API
		INCL_EQF_FOLDER           folder list and document list functions
*/

#ifndef _EQF_H_INCLUDED
#define _EQF_H_INCLUDED

/**********************************************************************/
/* add support for MFC compiling via predefined                       */
/**********************************************************************/

 #ifndef __cplusplus
  #define CPLUSPLUS
 #else
  #define CPLUSPLUS  ::
 #endif
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/* Resolve dependencies between conditionally include header files    */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

/**********************************************************************/
/* Translation processor uses VIO and AVIO data types                 */
/**********************************************************************/
#if defined(INCL_EQF_TP)
  #define INCL_VIO
  #define INCL_AVIO
  #define INCL_DEV
#endif

/**********************************************************************/
/* Remote Translation Memory functions require base TM functions      */
/**********************************************************************/
#if defined(INCL_EQF_TMREMOTE)
  #define INCL_EQF_TM
#endif

/**********************************************************************/
/* Terminology list functions require ANALYSIS defines                */
/**********************************************************************/
#if defined(INCL_EQF_LIST)
  #define INCL_EQF_ANALYSIS
#endif

/**********************************************************************/
/* Analysis functions require TAGTABLE defines                        */
/**********************************************************************/
#if defined(INCL_EQF_ANALYSIS)
  #define INCL_EQF_TAGTABLE
#endif

/**********************************************************************/
/* Setup defines for the include of OS2.H                             */
/**********************************************************************/
#ifndef NO_OS2DEFS
   #define INCL_DOS
   #define INCL_DOSPROCESS
   #define INCL_WIN
   #define INCL_GPI
   #define INCL_ERRORS
   #define INCL_NLS
   #define INCL_DEF
#endif

#ifndef NO_SPININCLUDE
  #define INCL_WINSTDSPIN
  #define INCL_WINSTDSLIDER
#endif

/**********************************************************************/
/* Include OS2.H or WINDOWS.H depending on the execution environment  */
/* Add some special settings necessary for both environments          */
/* The Windows settings are in the file EQFOSWIN.H                    */
/**********************************************************************/
  #define OEMRESOURCE                  // allow system bitmaps to be used
//  #define STRICT

  // force usage of old 32bit time value (Note: only dates up to the year 2038 are supported)
//  #define _USE_32BIT_TIME_T

  // use non-ISO version of swprintf which was used up to VS2005, once older versions are not used anymore we
  // should use the new version of the function asap
  #define _CRT_NON_CONFORMING_SWPRINTFS

  #include <windows.h>
  #include <winbase.h>

  /**********************************************************************/
  /* Include the special definitions for unicode                        */
  /**********************************************************************/
  #ifndef _EQF_UNICODE
    #define _EQF_UNICODE 1

    #ifndef _WCHAR_T_DEFINED
      typedef unsigned short wchar_t;
      #define _WCHAR_T_DEFINED
    #endif

    #if MSC_VER < 1400
        #ifndef _WCTYPE_T_DEFINED
          typedef wchar_t wint_t;
          typedef wchar_t wctype_t;
          #define _WCTYPE_T_DEFINED
        #endif
    #endif

    #ifndef _CHAR_W_DEFINED
      typedef wchar_t CHAR_W;
      typedef wchar_t * PCHAR_W;
      typedef wchar_t * PSZ_W;
      #define _CHAR_W_DEFINED
    #endif
  #endif

  #include "commctrl.h"          // common controls

  #include "eqfoswin.h"
  #include <dos.h>
  #include <io.h>
  #include <share.h>
  #include <fcntl.h>
  #include <assert.h>

/**********************************************************************/
/* Include some of the basic C library functions                      */
/**********************************************************************/
#ifndef NO_CINCLUDES
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <ctype.H>
#endif

/**********************************************************************/
/* Include support for spinbutton and slider controls                 */
/**********************************************************************/
#ifndef NO_SPININCLUDE
  #include <fclspbw.h>                 // include spinbutton stuff...
  #include <fclsldw.h>                 // include slider stuff...
#endif

/**********************************************************************/
/* Type definitions for OS/2 types - the new types are length         */
/* compatible to the TKT1.3 types. Use these defines in all           */
/* property file definitions.                                         */
/**********************************************************************/
typedef UINT   WINMSG;
typedef USHORT EQF_BOOL;
typedef PUSHORT PEQF_BOOL;
typedef SWP   EQF_SWP;
typedef PSWP  EQF_PSWP;
  #define NULLHANDLE 0
  #define SWP_FLAG(swp) swp.fs
  #define PSWP_FLAG(pswp) pswp->fs
  #define APIENTRY16 APIENTRY
  #define PSZ16 PSZ
  #define PBOOL16 PBOOL

// define for directory name containing user exit DLLs which should be
// copied to the WIN directory on start-up
#define PENDINGEXITS_DIR "USEREXITCOPY"

/**********************************************************************/
/* Defines for the Yes-to-all message box                             */
/**********************************************************************/
#define MB_EQF_YESTOALL    0x000E
#define MBID_EQF_YESTOALL  14

/**********************************************************************/
/* Conversion macros SWP <-> EQF_SWP                                  */
/**********************************************************************/
#define EQFSWP2SWP( e, s ) \
    { s.cx = e.cx; s.cy = e.cy; s.x = e.x; s.y = e.y; s.fs = e.fs; }
#define SWP2EQFSWP( s, e ) \
    { e.cx = s.cx; e.cy = s.cy; e.x = s.x; e.y = s.y; e.fs = s.fs; }

/**********************************************************************/
/* type of values for Dos... calls                                    */
/* Under toolkit 1.3 these values are of type USHORT, under           */
/* toolkit 2.1++ these are of type ULONG                              */
/**********************************************************************/
#define DOSVALUE USHORT

/**********************************************************************/
/* Include the general ID file                                        */
/**********************************************************************/
#include <eqf.id>                      // Eqf PM IDs

// Send a notification message to a dialog
#define SENDNOTIFICATION( hwnd, id, notification ) \
    CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( id, notification), 0L )
// post a WM_COMMAND message to a dialog
#define POSTCOMMAND( hwnd, id, notification ) \
    CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( id, notification), 0L )

// Get the ID of a window
#define IDFROMWINDOW( hwnd ) GetWindowLong( hwnd, GWL_ID )

// Set the ID of a window
#define SETWINDOWID( hwnd, id ) \
      SetWindowLong( hwnd, GWL_ID, (LONG)id )

/**********************************************************************/
/* Handle WM_EQF_QUERYID message                                      */
/**********************************************************************/
#define HANDLEQUERYID( id, mp2 ) *((PSHORT)mp2) = id

// Get the style of a window
#define STYLEFROMWINDOW( hwnd ) GetWindowLong( hwnd, GWL_STYLE )

// Set the style of a window
#define SETWINDOWSTYLE( hwnd, style )  \
      SetWindowLong( hwnd, GWL_STYLE, style )

// Obtain a presentation space handle / device context
#define GETPS( hwnd )   GetDC( hwnd )

// Release a presentation space handle / device context
#define RELEASEPS( hwnd, ps )   ReleaseDC( hwnd, ps )

// Message parameter from standard types
  #define MAKESHORT( c1, c2 ) (((SHORT)c1) | ((SHORT)c2 << 8))
  #define MP1FROMP(p)                 ((WPARAM)(VOID *)(p))
  #define MP1FROMHWND(hwnd)           ((WPARAM)(HWND)(hwnd))
  #define MP1FROMCHAR(ch)             ((WPARAM)(USHORT)(ch))
  #define MP1FROMSHORT(s)             ((WPARAM)(USHORT)(s))
  #define MP1FROM2CHAR( c1, c2)       ((WPARAM)MAKESHORT(c1, c2))
  #define MP2FROMP(p)                 ((LPARAM)(VOID *)(p))
  #define MP2FROMHWND(hwnd)           ((LPARAM)(HWND)(hwnd))
  #define MP2FROMCHAR(ch)             ((LPARAM)(USHORT)(ch))
  #define MP2FROMSHORT(s)             ((LPARAM)(USHORT)(s))
  #define MP2FROM2SHORT(x,y)          (LPARAM)((ULONG)y<<16 | x)
  #define MP2FROMLONG(l)              (LPARAM)(l)

// Message result from standard types
  #define MRFROMSHORT(s)             ((MRESULT)(SHORT)(s))
  #define MRFROMUSHORT(s)            ((MRESULT)(USHORT)(s))
  #define MRFROM2SHORT(s1, s2)       ((MRESULT)MAKELONG(s1, s2))
  #define SHORT1FROMMR(mr)           ((USHORT)((ULONG)mr))
  #define SHORT2FROMMR(mr)           ((USHORT)((ULONG)mr >> 16))

/* Macros to extract standard types from an MPARAM */
  #define CHAR1FROMMP1(mp)            ((UCHAR)(mp))
  #define CHAR2FROMMP1(mp)            ((UCHAR)((USHORT)mp >> 8))
  #define SHORT1FROMMP1(mp)           ((SHORT)(mp))
  #define SHORTFROMMP1(mp)            ((SHORT)(mp))
  #define USHORT1FROMMP1(mp)          ((USHORT)(mp))
  #define USHORTFROMMP1(mp)           ((USHORT)(mp))
  #define PVOIDFROMMP1(mp)            ((VOID FAR *)(mp))
  #define HWNDFROMMP1(mp)             ((HWND)(mp))
  #define PVOIDFROMMP2(mp)            ((VOID FAR *)(mp))
  #define HWNDFROMMP2(mp)             ((HWND)(mp))
  #define CHAR1FROMMP2(mp)            ((UCHAR)(mp))
  #define CHAR2FROMMP2(mp)            ((UCHAR)((ULONG)mp >> 8))
  #define CHAR3FROMMP2(mp)            ((UCHAR)((ULONG)mp >> 16))
  #define CHAR4FROMMP2(mp)            ((UCHAR)((ULONG)mp >> 24))
  #define USHORT1FROMMP2(mp)          ((USHORT)(ULONG)(mp))
  #define USHORT2FROMMP2(mp)          ((USHORT)((ULONG)mp >> 16))
  #define SHORT1FROMMP2(mp)           ((SHORT)(ULONG)(mp))
  #define SHORT2FROMMP2(mp)           ((SHORT)((ULONG)mp >> 16))
  #define LONGFROMMP2(mp)             ((ULONG)(mp))

// register a window class
#define REGISTERCLASS( inst, name, proc, instyle, extra, hicon, fOK ) \
    { WNDCLASS   wc; wc.style = (UINT)instyle; wc.lpfnWndProc = proc; wc.cbClsExtra = 0; \
      wc.cbWndExtra = (int)extra; wc.hInstance = (HINSTANCE)inst;   wc.hIcon = hicon; \
      wc.hCursor = LoadCursor( NULL, IDC_ARROW ); wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); \
      wc.lpszMenuName = NULL; wc.lpszClassName = name; fOK = (RegisterClass( &wc ) != (ATOM)NULL); }

// load an icon
#define LOADICON( module, id ) \
     LoadIcon( module, MAKEINTRESOURCE(id) )
// show/hide dropdownlist of a combobox
#define CBSHOWLIST( hwnd, id, flag ) \
     WinSendMsg( WinWindowFromID( hwnd, id ), CB_SHOWDROPDOWN, MP1FROMSHORT(flag), 0L )

#define CBSHOWLISTHWND( hwnd, flag ) \
     WinSendMsg( hwnd, CB_SHOWDROPDOWN, MP1FROMSHORT(flag), 0L )

// set the default state of a pushbutton
#define SETDEFAULTPB( hwnd, id, flag ) \
    { if ( flag ) CPLUSPLUS SendMessage( hwnd, DM_SETDEFID, (WPARAM)id, 0L ) ; }
#define SETDEFAULTPB_TRUE( hwnd, id ) \
    { CPLUSPLUS SendMessage( hwnd, DM_SETDEFID, (WPARAM)id, 0L ) ; }

/**********************************************************************/
/* Set a spinbutton array                                             */
/**********************************************************************/
#define SETSPINARRAY( hwnd, id, num, array ) \
    SendDlgItemMessage( hwnd, id, SPBM_SETARRAY, MP1FROMSHORT(num), MP2FROMP( array ) )

/**********************************************************************/
/* Query a spinbutton value                                           */
/**********************************************************************/
#define QUERYSPINVALUE( hwnd, id, buffer ) \
    { \
      SPBQUERYVALUE QueryValue; \
      QueryValue.lpValueBuf = buffer; QueryValue.wBufSize = sizeof(buffer); \
      QueryValue.wValue = SPBQ_DONOTUPDATE; \
      SendDlgItemMessage( hwnd, id, SPBM_SETARRAY, 0, MP2FROMP( &QueryValue ) );\
    }

/**********************************************************************/
/* Get length of a MLE text                                           */
/**********************************************************************/
#define MLEQUERYTEXTLENGTH( hwnd, id, len ) \
  len = (USHORT)SendDlgItemMessage( hwnd, id, WM_GETTEXTLENGTH, 0, 0L ) + 1

#define MLEQUERYTEXTLENGTHHWND( hwnd, len ) \
  len = (USHORT)CPLUSPLUS SendMessage( hwnd, WM_GETTEXTLENGTH, 0, 0L ) + 1


/**********************************************************************/
/* Export MLE Data                                                    */
/**********************************************************************/
#define MLEEXPORT( hwnd, id, buffer, len ) \
  len = (LONG)GetDlgItemText( hwnd, id, buffer, (SHORT)len )

#define MLEEXPORTW( hwnd, id, buffer, len ) \
  len = (LONG)GetDlgItemTextW( hwnd, id, buffer, (SHORT)len )

#define MLEEXPORTHWND( hwnd, buffer, len ) \
  len = (LONG)GetWindowText( hwnd, buffer, (SHORT)len )

#define MLEEXPORTHWNDW( hwnd, buffer, len ) \
  len = (LONG)GetWindowTextW( hwnd, buffer, (SHORT)len )

#define MLEEXPORTHWNDIPT( hwnd, buffer, ipt, len ) \
  len = (LONG)GetWindowText( hwnd, buffer, (SHORT)len )

/**********************************************************************/
/* Import MLE Data                                                    */
/**********************************************************************/
#define MLEIMPORT( hwnd, id, buffer, len ) \
  len = SetDlgItemText( hwnd, id, buffer )

#define MLEIMPORTHWND( hwnd, buffer, len ) \
    SetWindowText( hwnd, buffer )
#define MLEIMPORTHWNDW( hwnd, buffer, len ) \
      SetWindowTextW( hwnd, buffer )

#define MLEIMPORTHWNDIPTW( hwnd, buffer, ipt, len ) \
    SetWindowTextW( hwnd, buffer )
#define MLEIMPORTHWNDIPT( hwnd, buffer, ipt, len ) \
    SetWindowText( hwnd, buffer )

/**********************************************************************/
/* Set MLE text limit                                                 */
/**********************************************************************/
#define MLESETTEXTLIMITHWND( hwnd, len ) \
    CPLUSPLUS SendMessage( hwnd, EM_LIMITTEXT, (WPARAM)len, 0L )

/**********************************************************************/
/* Insert text into MLE                                               */
/**********************************************************************/
#define MLEINSERT( hwnd, id, text ) \
    SendDlgItemMessage( hwnd, id, EM_REPLACESEL, 0, MP2FROMP( text ) )

#define MLEINSERTHWND( hwnd, text ) \
    CPLUSPLUS SendMessage( hwnd, EM_REPLACESEL, 0, MP2FROMP( text ) )

/**********************************************************************/
/* Get current MLE selection                                          */
/**********************************************************************/
#define MLEQUERYSEL( hwnd, id, cursor, anchor ) \
  { \
    DWORD dwSel; \
    dwSel = SendDlgItemMessage( hwnd, id, EM_GETSEL, 0, 0L ); \
    cursor = LOWORD(dwSel); anchor = HIWORD(dwSel); \
  }

#define MLEQUERYSELHWND( hwnd, cursor, anchor ) \
  { \
    DWORD dwSel; \
    dwSel = CPLUSPLUS SendMessage( hwnd, EM_GETSEL, 0, 0L ); \
    cursor = LOWORD(dwSel); anchor = HIWORD(dwSel); \
  }


/**********************************************************************/
/* Set MLE selection                                                  */
/**********************************************************************/
#define MLESETSEL( hwnd, id, cursor, anchor ) \
    SendDlgItemMessage( hwnd, id, EM_SETSEL, (WPARAM)cursor, (LPARAM)anchor )

#define MLESETSELHWND( hwnd, cursor, anchor ) \
    CPLUSPLUS SendMessage( hwnd, EM_SETSEL, (WPARAM)cursor, (LPARAM)anchor )

/**********************************************************************/
/* Delete MLE contents                                                */
/**********************************************************************/
#define MLEDELETE( hwnd, id ) SetDialogItemText( hwnd, id, "" )

#define MLEDELETEHWND( hwnd ) SetWindowTextW( hwnd, L"" )

/**********************************************************************/
/* Set MLE changed flag                                               */
/**********************************************************************/
#define MLESETCHANGED( hwnd, id, flag ) \
    SendDlgItemMessage( hwnd, id, EM_SETMODIFY, MP1FROMSHORT(flag), 0L )

#define MLESETCHANGEDHWND( hwnd, flag ) \
    CPLUSPLUS SendMessage( hwnd, EM_SETMODIFY, MP1FROMSHORT(flag), 0L )

/**********************************************************************/
/* Set SLE changed flag                                               */
/**********************************************************************/
#define SLESETCHANGED( hwnd, id, flag ) \
    SendDlgItemMessage( hwnd, id, EM_SETMODIFY, MP1FROMSHORT(flag), 0L )

#define SLESETCHANGEDHWND( hwnd, flag ) \
    CPLUSPLUS SendMessage( hwnd, EM_SETMODIFY, MP1FROMSHORT(flag), 0L )


/**********************************************************************/
/* Set MLE readonly flag                                              */
/**********************************************************************/
#define MLESETREADONLY( hwnd, id, flag ) \
    SendDlgItemMessage( hwnd, id, EM_SETREADONLY, (WPARAM)flag, 0L )

#define MLESETREADONLYHWND( hwnd, flag ) \
    CPLUSPLUS SendMessage( hwnd, EM_SETREADONLY, (WPARAM)flag, 0L )

/**********************************************************************/
/* Set SLE readonly flag                                              */
/**********************************************************************/
#define SETREADONLY( hwnd, id, flag ) \
    SendDlgItemMessage( hwnd, id, EM_SETREADONLY, (WPARAM)flag, 0L )

/**********************************************************************/
/* Get character index for an MLE line                                */
/**********************************************************************/
#define MLECHARFROMLINE( hwnd, id, line, index ) \
    index = (USHORT)SendDlgItemMessage( hwnd, id, EM_LINEINDEX, MP1FROMSHORT(line), 0L )

#define MLECHARFROMLINEHWND( hwnd, line, index ) \
    index = CPLUSPLUS SendMessage( hwnd, EM_LINEINDEX, line, 0L )


/**********************************************************************/
/* Set tabstop style for a dialog control                             */
/**********************************************************************/
#define SETTABSTOP( hwnd, id ) \
    SetWindowLong( GetDlgItem( hwnd, id ), GWL_STYLE, \
      GetWindowLong( GetDlgItem( hwnd, id ), GWL_STYLE ) | WS_TABSTOP )

/**********************************************************************/
/* clear tabstop style for a dialog control                           */
/**********************************************************************/
#define CLEARTABSTOP( hwnd, id ) \
    SetWindowLong( GetDlgItem( hwnd, id ), GWL_STYLE, \
      GetWindowLong( GetDlgItem( hwnd, id ), GWL_STYLE ) & ~WS_TABSTOP )

/**********************************************************************/
/* Remove a object handler symbol                                     */
/**********************************************************************/
#define REMOVESYMBOL( symbol ) \
  ObjRemoveSymbol( symbol )

/**********************************************************************/
/* Set item state of column listbox items                             */
/**********************************************************************/
#define CLBSETITEMSTATE( hwnd, id, item, flag ) \
    SendDlgItemMessage( hwnd, id, LM_EQF_SETITEMSTATE, MP1FROMSHORT(item), \
                        MP2FROMSHORT(flag) )
#define CLBSETITEMSTATEHWND( hwndLB, item, flag ) \
    CPLUSPLUS SendMessage( hwndLB, LM_EQF_SETITEMSTATE, MP1FROMSHORT(item), \
                 MP2FROMSHORT(flag) )

/**********************************************************************/
/* Query item state of column listbox items                           */
/**********************************************************************/
#define CLBQUERYITEMSTATE( hwnd, id, item ) \
    (BOOL)SendDlgItemMessage( hwnd, id, LM_EQF_QUERYITEMSTATE, \
                              MP1FROMSHORT(item), 0L )
#define CLBQUERYITEMSTATEHWND( hwndLB, item ) \
    (BOOL)WinSendMsg( hwndLB, LM_EQF_QUERYITEMSTATE, MP1FROMSHORT(item), 0L )

/**********************************************************************/
/* Access the size field in a findfirst/findnext result buffer        */
/**********************************************************************/
#define RESBUFSIZE( resbuf ) resbuf.nFileSizeLow

/**********************************************************************/
/* Access the date field in a findfirst/findnext result buffer        */
/**********************************************************************/
#define RESBUFDATE( resbuf ) resbuf.wr_date
/**********************************************************************/
/* Access the time field in a findfirst/findnext result buffer        */
/**********************************************************************/
#define RESBUFTIME( resbuf ) resbuf.wr_time

/**********************************************************************/
/* Access the name field in a findfirst/findnext result buffer        */
/**********************************************************************/
#define RESBUFNAME( resbuf ) resbuf.cAlternateFileName
#define RESBUFNAMELONG( resbuf ) resbuf.achName

/**********************************************************************/
/* !!!! Added by CHM !!!!                                             */
/**********************************************************************/
#define CBSEARCHSELECTHWND( sItem, hwnd, pszItem ) \
  { \
    sItem = (SHORT)CPLUSPLUS SendMessage( hwnd, CB_FINDSTRINGEXACT, 0, (LPARAM) pszItem ); \
    if( sItem != CB_ERR ) \
    { \
       CPLUSPLUS SendMessage( hwnd, CB_SETCURSEL, sItem, 0L );   \
    } \
  }

#define CBQUERYSELECTEDITEMTEXT( sItem, hwnd, id, szBuffer ) \
  { \
    sItem = (SHORT)SendDlgItemMessage( hwnd, id, CB_GETCURSEL, 0, 0L ); \
    if( sItem != CB_ERR ) \
    { \
       SendDlgItemMessage( hwnd, id, CB_GETLBTEXT, MP1FROMSHORT( sItem ), \
                           MP2FROMP( szBuffer ) ); \
    } \
    else \
    { \
      szBuffer [0] = EOS; \
    } /* endif */ \
  }

#define CBQUERYSELECTEDITEMTEXTHWND( sItem, hwnd, szBuffer ) \
  { \
    sItem = (SHORT)CPLUSPLUS SendMessage( hwnd, CB_GETCURSEL, 0, 0L ); \
    if( sItem != CB_ERR ) \
    { \
       CPLUSPLUS SendMessage( hwnd, CB_GETLBTEXT, MP1FROMSHORT( sItem ), \
                    MP2FROMP( szBuffer ) ); \
    } \
    else \
    { \
      szBuffer [0] = EOS; \
    } /* endif */ \
  }

#define LBQUERYTOPINDEX( hwnd, usID ) \
    (SHORT)SendDlgItemMessage( hwnd, usID, LB_GETTOPINDEX, 0, 0L )

#define LBQUERYTOPINDEXHWND( hwnd ) \
    (SHORT)CPLUSPLUS SendMessage( hwnd, LB_GETTOPINDEX, 0, 0L )

#define LBSEARCHSTRING( hwnd, usID, pszString ) \
    (SHORT)SendDlgItemMessage( hwnd, usID, LB_FINDSTRING, MP1FROMSHORT( 0 ), \
                               MP2FROMP( pszString ) )
#define LBSEARCHSTRINGHWND( hwnd, pszString ) \
    (SHORT)CPLUSPLUS SendMessage( hwnd, LB_FINDSTRING, MP1FROMSHORT( 0 ), \
                        MP2FROMP( pszString ) )

#define ISCTRLENABLED( hwnd, id ) \
    IsWindowEnabled( GetDlgItem( hwnd, id ) )

#define ISCTRLENABLEDHWND( hwnd ) \
    IsWindowEnabled( hwnd )

// Post a WM_CLOSE message to close window
#define POSTEQFCLOSE( hwnd, flag ) \
    CPLUSPLUS PostMessage( hwnd, WM_EQF_CLOSE, flag, 0L )

#define WMCOMMANDID( mp1, mp2 ) LOWORD(mp1)

#define WMCOMMANDCMD( mp1, mp2 ) HIWORD(mp1)


////////////////////////////////////////////////////////////////////////
//
// Macro to convert codepages not supported by Windows
//    currently Japanese 943 is set to Japanese 932
//
////////////////////////////////////////////////////////////////////////
#define ADJUSTCP( cp ) ( ( cp == 943 ) ? 932 : cp )

/**********************************************************************/
/* Eqf editor api                                                     */
/**********************************************************************/


#if defined(INCL_EQF_EDITORAPI)
  #include <OTMAPI.H>
#endif

/**********************************************************************/
/* Message ID file                                                    */
/**********************************************************************/
#include <eqfmsg.h>                    // general eqf message ids

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/*                        Message Section                             */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
#ifndef NO_EQF_WMIDS

typedef enum _eqfmsgids
{
WM_EQF = WM_USER+1000,                 // this id starts the range of Eqf messages

/*
                            ------------
                              General
                            ------------
*/
WM_EQF_INITIALIZE,
/*-----------------------------------------------------------------------------
     descr.:  sent by anchor to its instance; this is normally the 1st
              application message an instance receives after it was created
              by its anchor (handler).
        mp1:  NULL or application specific data
        mp2:  NULL or application specific data
    process:  SEND/POST
    returns:  (application depending convention)
      hints:  this message was installed to allow instance to startup and to
              keep code under WM_CREATE as simple and common as possible.
  -----------------------------------------------------------------------------
*/

WM_EQF_TERMINATE,
/*-----------------------------------------------------------------------------
     descr.:  sent by object manager to terminate processing of an
              instance or handler
        mp1:  SHORT1 = close flag TWBCLOSE, TWBSAVE, TWBFORCE
              SHORT2 = NULL
        mp2:  NULL
    process:  SEND
    returns:  BOOL - TRUE if handler or instance REJECTS the request
                   - FALSE if handler or instance ACCEPTS the request
      hints:  this message MUST be sent to allow subsequent processing to
              finish before receiver window gets destroyed;
              object manager ignores return values if flag TWBFORCE is set;
              the receiving handler or instance must close if TWBFORCE is
              requested otherwise an unpredictable status may be left
              and may cause subsequent errors or an
              unpredictanable result when the Eqf system gets restartet.
           >> Handlers and objects will receive this message in reverse order
              from which they registered to the object manager, i.e. the last
              registered object will receive this message as the first in
              the chain;
           >> before any handler gets this message the object manager ensures
              that message traffic caused by objects (instances) has
              finished, therefore allowing the handlers to perform extra stuff
              without having to obey some of its instances.
  -----------------------------------------------------------------------------
*/

WM_EQF_SHUTDOWN,
/*-----------------------------------------------------------------------------
     descr.:  sent by object manager to all active handlers when a request
              was initiated to close down the Eqf system
        mp1:  SHORT1 = flags, type of close: TWBFORCE, TWBSAVE, TWBCLOSE
              SHORT2 = NULL
        mp2:  NULL
    process:  SEND
    returns:  BOOL - TRUE if handler REJECTS the request
                   - FALSE if handler ACCEPTS the request
      hints:  this message is only sent to Eqf handlers to allow for
              special shutdown processes before WM_EQF_TERMINATE
  -----------------------------------------------------------------------------
*/

WM_EQFN_SHUTDOWNCANCELED,
/*-----------------------------------------------------------------------------
     descr.:  sent by object manager to all active handlers when a request
              for SHUTDOWN was canceled (e.g. due to user input)
        mp1:  NULL
        mp2:  NULL
    process:  SEND
    returns:  NULL
      hints:  this message is only sent to Eqf handlers to allow for
              reset of initiated shutdown processing
  -----------------------------------------------------------------------------
*/
/*                          ------------
                              TWB Main
                            ------------
*/
WM_EQF_QUERYSYSTEMPATH,
/*-----------------------------------------------------------------------------
     descr.:  sent to handler TWBMAIN to query the path of system EQF
        mp1:  NULL
        mp2:  NULL
    process:  SEND
    returns:  PSZ - to System Properties
      hints:  this message is normmaly sent only by the property handler
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYSYSPROPPATH,
/*-----------------------------------------------------------------------------
     descr.:  sent to handler TWBMAIN to query the name of System Properties
        mp1:  NULL
        mp2:  NULL
    process:  SEND
    returns:  PSZ - of System Properties (full qulified)
      hints:  this message is normmaly sent only by the property handler
  -----------------------------------------------------------------------------
*/

WM_EQF_ACTIVATEINSTANCE,
/*-----------------------------------------------------------------------------
     descr.:  sent to handler TWBMAIN to inform the handler of
              a focus and/or status change for an instance of a
              specific class
        mp1:  SHORT1 = TRUE if active and focus
                       FALSE if inactive (no focus)
              SHORT2 = NULL
        mp2:  HWND - to object instance
    process:  SEND
    returns:  NULL
      hints:  this message tells the TABMAIN handler which instance has got
              the focus if TRUE is indicated with mp1 or that an instance has
              lost the focus if FALSE is given in mp1;
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYMENUTBL,
/*-----------------------------------------------------------------------------
     descr.:  sent to handler TWBMAIN to request the table of menu items and
              menu commands
        mp1:  NULL
        mp2:  NULL
    process:  SEND
    returns:  PMNULST - ptr to menu table
      hints:
  -----------------------------------------------------------------------------
*/
/*                       -----------------------
                           Anchor <-> Instance
                         -----------------------
*/

WM_EQF_OPEN,
/*-----------------------------------------------------------------------------
     descr.:  sent to a handler to open an instance of an existing object
        mp1:  TRUE  -> object is restarted by TWB
              FALSE -> object is opened by user
        mp2:  PSZ  - object name
    process:  SEND/POST
    returns:  BOOL - TRUE if object was not opened successfully
                     FALSE if object was opened
      hints:  this message will cause msg WM_EQFN_OBJECTREGISTERED to be
              sent to all handlers; it is sent by the object manager due to
              a successful registration of an object
  -----------------------------------------------------------------------------
*/

WM_EQF_CREATE,
/*-----------------------------------------------------------------------------
     descr.:  sent to a handler to create a new object and optionally open it
        mp1:  SHORT1 = TRUE : open object if creation was ok
                       FALSE: do not open
              SHORT2 = NULL
        mp2:  PSZ  - name of object to be created
              NULL - ask for name of new object
    process:  SEND
    returns:  BOOL - TRUE if object not created
                     FALSE if object was successfully created
      hints:  this message should cause notification message WM_EQFN_CREATED
              to be sent to all handlers in the system when
              creation of the object was successfully performed, i.e. when
              its properties were created.
           >> Failing to open the instance window is not always a reason to
              suppress message WM_EQFN_CREATED when the object can live in
              the system (without beeing present with an instance window)
              otherwise object properties should be deleted etc.
  -----------------------------------------------------------------------------
*/

WM_EQF_DELETE,
/*-----------------------------------------------------------------------------
     descr.:  sent to a handler to request deletion of an object
        mp1:  NULL
        mp2:  PSZ  - object name
    process:  SEND
    returns:  BOOL - TRUE if the object was not deleted for what reason ever
                   - FALSE if the object was successfully deleted
      hints:  this message should cause message WM_EQF_ABOUTTODELETE
              to be sent to all handlers, decision for deletion of the
              object should depend on the return values of all handlers,
              i.e. if one single handler rejects this message then
              deletion of the object should be cancelled.
           >> message WM_EQFN_DELETED should be sent to all handlers if the
              object was successfully deleted
  -----------------------------------------------------------------------------
*/

WM_EQF_ABOUTTODELETE,
/*-----------------------------------------------------------------------------
     descr.:  sent to all handlers to inform that a request for deletion of
              an object is about to be performed
        mp1:  SHORT1 = class id
              SHORT2 = NULL
        mp2:  PSZ  - object name
    process:  SEND
    returns:  BOOL - TRUE if receiver REJECTS the deletion
                   - FALSE if receiver ACCEPTS the deletion
      hints:  this message should be sent to all handlers and return values
              of all handlers should be respected before the deletion of the
              object is performed.
  -----------------------------------------------------------------------------
*/

WM_EQFN_CREATED,
/*-----------------------------------------------------------------------------
     descr.:  sent to all handlers to inform about creation of a new object
        mp1:  SHORT1 = class id
              SHORT2 = NULL
        mp2:  PSZ  - object name
    process:  POST/SEND
    returns:  NULL
      hints:  this message is a notification message only and requires no
              action if not appropriate.
              Handlers should route this message to their instances if the
              new object might be of interest to some or all instances.
  -----------------------------------------------------------------------------
*/

WM_EQFN_DELETED,
/*-----------------------------------------------------------------------------
     descr.:  sent to all handlers to inform about deletion of a object
        mp1:  SHORT1 = class id
              SHORT2 = NULL
        mp2:  PSZ  - object name
    process:  POST/SEND
    returns:  NULL
      hints:  this message is a notification message only and requires no
              action if not appropriate.
              Handlers should route this message to their instances if the
              object was of interest to some or all instances.
  -----------------------------------------------------------------------------
*/

WM_EQFN_PROPERTIESCHANGED,
/*-----------------------------------------------------------------------------
     descr.:  sent to all handlers to inform about modification of some
              object properties.
        mp1:  SHORT1 = class id
              SHORT2 = NULL
        mp2:  PSZ  - object name
    process:  POST/SEND
    returns:  NULL
      hints:  this message is a notification message only and requires no
              action if not appropriate.
              Handlers should route this message to their instances if the
              properties might be of interest to some or all instances.
           >> This message is sent by the property handler for the following
              reasons: WRITE_ACCESS to some properties were reset or
              PutAllProperties was performed by the property handler.
  -----------------------------------------------------------------------------
*/

WM_EQF_INSERTNAMES,
/*-----------------------------------------------------------------------------
     descr.:  sent to handler or instance to request load of an list box
              with class specific information
        mp1:  HWND - list box handle
        mp2:  PSZ - object name
    process:  SEND
    returns:  USHORT - number of itmes loaded
                     - NULL if list box is empty
      hints:  Class specific information is under convention of the receiving
              instance, e.g. a folder instance will insert names of its
              documents, a folderlist instance will insert names of its
              folders etc.
           >> Handlers receiving this message check for existance of the
              appropriate instance and route the message to it, otherwise
              they reject (return) if they have no way defined to load the
              requested names.
           >> The memory handler supports the following object names:
                 MEMORY_ALL        (inserts all TM's (locals and remotes))
                 FORMAT_TABLE_ALL  (inserts all Markup language)
                 EXCLUSION_LIST_ALL (inserts all Word list/Noise list)
                 SOURCE_LANGUAGES_ALL (inserts all source languages )
                 TARGET_LANGUAGES_ALL (inserts all target languages )
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYSELECTEDNAMES,
/*-----------------------------------------------------------------------------
     descr.:  same as WM_EQF_INSERTNAMES message except that listbox is
              filled with the currently selected class specific information
        mp1:  HWND - list box handle
        mp2:  PSZ - object name
    process:  SEND
    returns:  USHORT - number of itmes loaded
                     - NULL if list box is empty
      hints:  Class specific information is under convention of the receiving
              instance, e.g. a folder instance will insert names of its
              selected documents, a folderlist instance will insert the name
              of its selected folder etc.
           >> Handlers receiving this message check for existance of the
              appropriate instance and route the message to it, otherwise
              they reject (return) if they have no way defined to load the
              requested names.
  -----------------------------------------------------------------------------
*/

WM_EQF_COMMAND,
/*-----------------------------------------------------------------------------
     descr.:  sent by TWBMAIN to the active instance as a result of a
              WM_COMMAND received by TWBMAIN from the menu bar
              (active instance having the focus)
        mp1:  (same as WM_COMMAND)
        mp2:  (same as WM_COMMAND)
    process:  SEND
    returns:  BOOL - TRUE if the instance ACCEPTS the command
                   - FALSE if the instance REJECTS the command
      hints:  This message was defined to allow the receiver to distinguish
              between a WM_COMMAND from its own window or from the
              TWB menu bar.
  -----------------------------------------------------------------------------
*/
/*                       ------------------
                           Object Manager
                         ------------------
*/

WM_EQF_QUERYHANDLER,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to query the handle of a specific handler
        mp1:  NULL
        mp2:  PSZ  - handler name
    process:  SEND
    returns:  HWND - to handler
              NULL - if handler was not found
      hints:
  -----------------------------------------------------------------------------
*/

WM_EQF_REMOVEHANDLER,
/*-----------------------------------------------------------------------------
     descr.:  sent by a handler to the object manager to terminate
              processing of itself or another handler
        mp1:  SHORT1 = close flag TWBCLOSE, TWBSAVE, TWBFORCE
              SHORT2 = NULL
        mp2:  PSZ - handler name
    process:  SEND/POST
    returns:  BOOL - TRUE if requested handler was not found or rejected
                          the request
                   - FALSE if handler was successfully removed
      hints:  Process must be SEND if handler wants to remove itself
  -----------------------------------------------------------------------------
*/

WM_EQF_REMOVEOBJECT,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to terminate processing
              of a specific instance
        mp1:  SHORT1 = close flag TWBCLOSE, TWBSAVE, TWBFORCE
              SHORT2 = NULL
        mp2:  HWND to object
    process:  SEND/POST
    returns:  BOOL - TRUE if requested object  was not found or rejected
                          the request
                   - FALSE if object  was successfully removed
      hints:  process must be SEND if object wants to remove itself;
              this message causes msg WM_EQFN_OBJECTREMOVED to be sent to all
              handlers
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYOBJECT,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to query the handle of
              a specific instance
        mp1:  SHORT1 = class id or NULL
              SHORT2 = status flags or NULL
        mp2:  PSZ - instance object name or NULL
    process:  SEND
    returns:  HWND - of instance
              NULL - if instance was not found
      hints:  if PSZ points to an object name (i.e. !NULL) than it is
              searched in the class of objects specified with SHORT1(mp1),
              if PSZ is NULL then search takes place in the class
              specified with SHORT1(mp1) and the first object that
              matches the flags will be returned (as HWND)
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYOBJECTNAME,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to query the name of a specific object
        mp1:  HWND - handle to object
        mp2:  PSZ - ptr to buffer receiving the object name
    process:  SEND
    returns:  USHORT - length of object name
              NULL - if instance was not found
      hints:  Sender must provide a buffer large enough to receive the
              object name ( e.g. type OBJNAME or of length _MAX_PATH)
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYOBJECTCLASS,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to query the class id of
              a specific instance
        mp1:  NULL
        mp2:  HWND - handle to object
    process:  SEND
    returns:  SHORT - class id of instance
              -1    - if instance was not found
      hints:
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYOBJECTSTATUS,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to query the status flags
              of a specific object
        mp1:  NULL
        mp2:  HWND - handle to object
    process:  SEND
    returns:  USHORT - status flags of instance
              NULL - if instance was not found
      hints:  status should never become zero because the object manager
              maintains a status flag which is not maskable by the
              caller and can therefore not be cleared.
  -----------------------------------------------------------------------------
*/

WM_EQF_SETOBJECTSTATUS,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to set status flags
              for a specific instance
        mp1:  SHORT1 = mask of flags to manipulate
              SHORT2 = flags to manipulate according to mask
        mp2:  HWND - handle to object
    process:  SEND
    returns:  USHORT - new status of instance
              NULL   - if instance was not found
      hints:  the status should never become zero because the object manager
              maintains a status flag which is not maskable by the
              caller and can therefore not be cleared.
              The following flags are defined:
              OBJ_ACTIVE   - object is active (not disabled)
              OBJ_BUSY     - object is busy (i.e. in a specific class of
                             objects only one object can be busy = last used
                             object;
              OBJ_FOCUS    - object instance has the PM focus
                             implies also that the object is active and busy
              (see eqfobj01.h for more details)
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYOBJECTCOUNT,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to query the number of instances for a
              specific object class or for all classes
        mp1:  SHORT1 = class id of requested objects
              SHORT2 = NULL
        mp2:  NULL
    process:  SEND
    returns:  USHORT - number of objects found
              NULL   - if there was no object found for the requested class
      hints:
  -----------------------------------------------------------------------------
*/

WM_EQF_GETOBJECTLIST,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to query a list of object handles to
              instances for a specific object class or for all classes
        mp1:  SHORT1 = class id of requested objects
              SHORT2 = number of entries in table given with mp2,
                       entries are of type OBJLST
        mp2:  POBJLST - ptr to table of OBJLST entries
    process:  SEND
    returns:  USHORT - number of entries loaded
              NULL   - if there was no object found for the requested class
      hints:
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYHANDLERCOUNT,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to query the number of registered
              handlers
        mp1:  NULL
        mp2:  NULL
    process:  SEND
    returns:  USHORT - number of handlers found
              NULL   - if there was no handler found
      hints:
  -----------------------------------------------------------------------------
*/

WM_EQF_SETSYMBOL,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to init/set a user defined symbol and
              to let it be maintained by the object manager
        mp1:  SHORT1 - mask for status flags to manipulate
              SHORT2 - flags to manipulate
        mp2:  PSZ  - symbol name
    process:  SEND
    returns:  SHORT - status of symbol set (according to values in mp1)
              -1    - if sysmbol  was not found
      hints:  see SETOBJECTSTATUS for flags
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYSYMBOL,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to query the existance and status of a
              user defined symbol
        mp1:  NULL
        mp2:  PSZ  - symbol name
    process:  SEND
    returns:  SHORT - status of symbol set by SETSYMBOL
              -1    - if sysmbol  was not found
      hints:
  -----------------------------------------------------------------------------
*/

WM_EQF_REMOVESYMBOL,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to remove a user defined symbol from
              object managers symbol table
        mp1:  NULL
        mp2:  PSZ  - symbol name
    process:  SEND
    returns:  NULL  - symbol removed
              -1    - if sysmbol  was not found
      hints:
  -----------------------------------------------------------------------------
*/

WM_EQFN_OBJECTREMOVED,
/*-----------------------------------------------------------------------------
     descr.:  sent to all handlers due to a successful deregistration of an
              object
        mp1:  SHORT = class id of registered object
        mp2:  PSZ - object name
    process:  POST/SEND
    returns:  NULL
      hints:
  -----------------------------------------------------------------------------
*/

/*                       --------------------
                           Property Handler
                         --------------------
*/
WM_EQF_QUERYSYSTEMPROPHND,
/*-----------------------------------------------------------------------------
     descr.:  sent to property handler to query the handle to
              Eqf system properties
        mp1:  NULL
        mp2:  NULL
    process:  SEND
    returns:  HPROP - Handle to system properties
      hints:  System properties are available after start up
              of the property handler
  -----------------------------------------------------------------------------
*/

WM_EQF_OPENPROPERTIES,
/*-----------------------------------------------------------------------------
     descr.:  sent to property handler for open object properties
        mp1 = PPROPMSGPARM - ptr to internal structure
        mp2 = pErrorInfo - ptr to error info
    process:  SEND
    returns:  HPROP - handle to open properties
              NULL  - if request failed
      hints:  This message should not be sent except using
              API OpenProperties
  -----------------------------------------------------------------------------
*/

WM_EQF_CREATEPROPERTIES,
/*-----------------------------------------------------------------------------
     descr.:  sent to property handler to create new object properties
        mp1 = PPROPMSGPARM - ptr to internal structure
        mp2 = pErrorInfo - ptr to error info
    process:  SEND
    returns:  HPROP - handle to open properties
              NULL  - if request failed
      hints:  This message should not be sent except using
              API CreateProperties
  -----------------------------------------------------------------------------
*/

WM_EQF_DELETEPROPERTIES,
/*-----------------------------------------------------------------------------
     descr.:  sent to property handler to delete object properties
        mp1 = PPROPMSGPARM - ptr to internal structure
        mp2 = pErrorInfo - ptr to error info
    process:  SEND
    returns:  SHORT - NULL if successfully deleted
                    - !NULL if request failed
      hints:  This message should not be sent except using
              API DeleteProperties
  -----------------------------------------------------------------------------
*/

WM_EQF_CLOSEPROPERTIES,
/*-----------------------------------------------------------------------------
     descr.:  sent to property handler to close object properties
        mp1 = PPROPMSGPARM - ptr to internal structure
        mp2 = pErrorInfo - ptr to error info
    process:  SEND
    returns:  SHORT - NULL if successfully closed
                    - !NULL if request failed
      hints:  This message should not be sent except using
              API CloseProperties
  -----------------------------------------------------------------------------
*/

WM_EQF_GETALLPROPERTIES,
/*-----------------------------------------------------------------------------
     descr.:  sent to property handler to get a complete copy of object
              properties
        mp1 = PPROPMSGPARM - ptr to internal structure
        mp2 = pErrorInfo - ptr to error info
    process:  SEND
    returns:  SHORT - NULL if successfully copied
                    - !NULL if request failed
      hints:  This message should not be sent except using
              API GetAllProperties
  -----------------------------------------------------------------------------
*/

WM_EQF_PUTALLPROPERTIES,
/*-----------------------------------------------------------------------------
     descr.:  sent to property handler to write back a complete copy of
              object properties (instorage)
        mp1 = PPROPMSGPARM - ptr to internal structure
        mp2 = pErrorInfo - ptr to error info
    process:  SEND
    returns:  SHORT - NULL if successfully copied
                    - !NULL if request failed
      hints:  This message should not be sent except using
              API PutAllProperties
  -----------------------------------------------------------------------------
*/

WM_EQF_SAVEPROPERTIES,
/*-----------------------------------------------------------------------------
     descr.:  sent to property handler to explicitely write the instorage
              version of object properties to disk
        mp1 = PPROPMSGPARM - ptr to internal structure
        mp2 = pErrorInfo - ptr to error info
    process:  SEND
    returns:  SHORT - NULL if successfully copied
                    - !NULL if request failed
      hints:  This message should not be sent except using
              API SaveProperties
  -----------------------------------------------------------------------------
*/

WM_EQF_QUERYPROPSIZE,
/*-----------------------------------------------------------------------------
     descr.:  sent to handler to query the size of its properties
        mp1 = USHORT - class id of properties
        mp2 = NULL
    process:  SEND
    returns:  the handler should return the size of its properties
  -----------------------------------------------------------------------------
*/

    WM_EQF_REGISTER,                 // register/deregister windows
    WM_EQF_REINIT,                   // (re)init an alread loaded dialog
    WM_EQF_LSTREGISTER,              // register list interest
    WM_EQF_LSTEVENT,                 // communicate lst event
    WM_EQF_LSTQRYLOCK,               // query if a list is locked
    WM_EQF_LSTSETLOCK,               // set/release list lock
    WM_EQF_REFRESH,                  // refresh all controls
    WM_EQF_SETFOCUS,                 // for posting focus changes

LM_EQF_QUERYVIEWLIST,
/*-----------------------------------------------------------------------------
     descr.:  sent to column listbox control to query the a view list
        mp1 = USHORT - view list ID:
                       CURRENT_VIEW  for currently active view list
                       DETAILS_VIEW  for the details view list
                       DEFAULT_VIEW  for the details default view list
                       NAME_VIEW     for the name option view list
        mp2 = PSHORT - ptr to buffer for the returned view list
    process:  SEND
    returns:  number of entries in returned view list
  -----------------------------------------------------------------------------
*/
LM_EQF_SETVIEWLIST,
/*-----------------------------------------------------------------------------
     descr.:  sent to column listbox control to set a new view list
        mp1 = USHORT - view list ID:
                       CURRENT_VIEW  for currently active view list
                       DETAILS_VIEW  for the details view list
                       DEFAULT_VIEW  for the details default view list
                       NAME_VIEW     for the name option view list
        mp2 = PSHORT - ptr to the new view list
    process:  SEND
    returns:  number of entries of the new view list
  -----------------------------------------------------------------------------
*/
LM_EQF_SETSORTLIST,
/*-----------------------------------------------------------------------------
     descr.:  sent to column listbox control to set a new sort list
        mp1 = NULL     not used
        mp2 = PSHORT - ptr to the new sort list
    process:  SEND
    returns:  number of entries of the new sort list
  -----------------------------------------------------------------------------
*/
LM_EQF_QUERYSORTLIST,
/*-----------------------------------------------------------------------------
     descr.:  sent to column listbox control to query the current sort list
        mp1 = NULL     not used
        mp2 = PSHORT - ptr to the buffer for the returned sort list
    process:  SEND
    returns:  number of entries in returned sort list
  -----------------------------------------------------------------------------
*/
WM_EQF_NEXTSTEP,
/*-----------------------------------------------------------------------------
     descr.:  this message can be used to serialize processing or control
              the processing inside a window procedure. There are no
              restrictions for the usage of the message parameters. This
              message should be send only to the own window.
        mp1 = up to the user
        mp2 = up to the user
    process:  POST
    returns:  up to the user
  -----------------------------------------------------------------------------
*/
WM_EQF_PROCESSTASK,
/*-----------------------------------------------------------------------------
     descr.:  this message is sent to a handler to request the processing
              of a handler specific task (e.g. to export an object).
              This message can be used like the WM_EQF_COMMAND message
              except that an object name can be passed to the receiver
              and other tasks than tasks of pulldown items can be requested.
              The task ID can be one of the predefines task identifiers (see
              list below), user tasks (must be defined relative to the ID
              USER_TASK) and IDs of TWB pulldown item.

              List of predefined task identifiers:

               - IMPORT_TASK (for post-import processing of files)

               - EXPORT_TASK (for pre-export processing of files)

               - CREATE_PATH (for translation memory handler only)
                   A translation memory path should be created out of the TM
                   name.  mp2 should contain the address of a string in
                   length MAX_EQF_PATH containing the translation memory
                   name.  After return if OK mr will be TRUE and the string
                   will contain the full translation memory path + name +
                   ext.  If mr FALSE the function failed and the string will
                   be unchanged. Example:
                     usRc = (USHORT)EqfSend2Handler
                                    ( MEMORYHANDLER,
                                      WM_EQF_PROCESSTASK,
                                      MPFROMSHORT( CREATE_PATH ),
                                      MPFROMP( pIDA->szMemPath ));

               - QUERY_LANG_PROP_INPUT_MEM (for TM handler only)
               - QUERY_LANG_PROP_INPUT_LANG (for TM handler only)
                   All properties of a language should be returned to the
                   caller.  An example of calling this function is shown
                   here:
                      strcpy( szTemp, "test");
                      usRc = (USHORT)EqfSend2Handler
                                ( MEMORYHANDLER,
                                WM_EQF_PROCESSTASK,
                                MPFROMSHORT( QUERY_LANG_PROP_INPUT_MEM ),
                                MPFROMP( szTemp ));
                   mp2 must contain the address of a string.  The string
                   should have the length of MAX_LANGUAGE_PROPERTIES.
                   If mp1 is QUERY_LANG_PROP_INPUT_MEM then the string must
                   contain a TM name without extension.  Example:  test
                   If mp1 is QUERY_LANG_PROP_INPUT_LANG then the string must
                   contain a language name.  Example:  english

                   If the return code is TRUE then the string will contain
                   the data in the following format:
                   Language\x015Morph. Dict\x015Lang.Code\x015...............
                   Example:
           English\x015US.DC2\x0156011\x015ENGNOISE.LST\x015ENGSTOP.LST\x015..

                   If the return code is FALSE then the string may contain
                   unpredictable data !!

                   The following enum values can be used to extract data
                   from the string via the utility UtlParseX15
                        LANG_LANGUAGE_IND,
                        LANG_MORPH_DICT_IND,
                        LANG_LANG_CODE_IND,
                        LANG_NOISE_LIST_IND,
                        LANG_STOPWORD_LIST_IND,
                        LANG_ADDENDA_DICT_IND,
                   Example: Extract the name of the noise list after
                            successfull return with the following statement
                   strcpy( pszNoiseList,
                           UtlParseX15( pszString, LANG_NOISE_LIST_IND ));

               - TM_SERVER_REFERENCE_TASK (for TM handler only)
                   This function checks if a TM is used/referenced on a
                   specific server.  mp2 should contain the address of a
                   string in length MAX_SERVER_NAME containing the server to
                   be checked.  After return if OK mr will be TRUE and the
                   string will contain the first TM-name which is used and
                   located on that server.  If mr FALSE than no TM is
                   used/referenced of that server and the string will
                   remain unchaged . Example:
                     usRc = (USHORT)EqfSend2Handler
                                    ( MEMORYHANDLER,
                                      WM_EQF_PROCESSTASK,
                                      MPFROMSHORT( TM_SERVER_REFERENCE_TASK ),
                                      MPFROMP( pszServer ));

               - TM_QUERY_SERVER_TASK (for translation memory handler only)
                   This function returns the server name of a translation
                   memory.  mp2 should contain the address of a string in
                   length MAX_SERVER_NAME containing the translation memory
                   name.  After return if OK mr will be TRUE and the string
                   will contain the name of the server or a NULL string if
                   the TM is local.  If mr FALSE the function failed and the
                   string will remain unchaged. This may happen if the TM
                   does not exist. Example:
                     usRc = (USHORT)EqfSend2Handler
                                    ( MEMORYHANDLER,
                                      WM_EQF_PROCESSTASK,
                                      MPFROMSHORT( TM_QUERY_SERVER_TASK ),
                                      MPFROMP( szTemp ));



        mp1 = SHORT1FROMMP - task identifier or pulldown item id
              SHORT2FROMMP - not used
        mp2 = PSZ          - pointer to object name
    process:  SEND
    example:  EqfSend2Handler( FOLDERHANDLER, WM_EQF_PROCESSTASK,
                               MPFROM2SHORT( PID_FILE_MI_EXPORT, 0 ),
                               MPFROMP( "D:\\EQF\\FOLDER1.F00" ) );
    returns:  TRUE if task was processed successful else FALSE
  -----------------------------------------------------------------------------
*/
WM_EQFN_TASKDONE,
/*-----------------------------------------------------------------------------
     descr.:  this message tells the receiving function that a task started
              using WM_EQF_PROCESSTASK has been completed.
        mp1 = SHORT1FROMMP - success (TRUE) or failure (FALSE) of the
                             performed function
              SHORT2FROMMP - not used
        mp2 = PSZ          - not used
    process:  SEND/POST
  -----------------------------------------------------------------------------
*/
WM_EQF_ABOUTTOREMOVEDRIVE,
/*-----------------------------------------------------------------------------
     descr.:  sent to all handlers to inform that a secondary EQF drive is
              about to be detached
        mp1:  CHAR1FROMMP = drive being detached; e.g. 'X'
        mp2:  NULL
    process:  SEND
    returns:  BOOL - TRUE if receiver REJECTS the detach
                   - FALSE if receiver ACCEPTS the detach
      hints:  this message should be sent to all handlers and return values
              of all handlers should be respected before the drive is detached
              The actual detach of a secondary EQF drive is reported by means
              of the WM_EQFN_PROPERTIESCHANGED message for the system
              properties.
  -----------------------------------------------------------------------------
*/

/*
                            ------------------
                              Slider Utility
                            ------------------
*/

WM_EQF_SLIDER_POSITION,
/*-----------------------------------------------------------------------------
     descr.:  sent by a process to its slider window to change the actual
              position of the slider; value must be in the range 0 .. 100
        mp1:  (LONG/ULONG) position of slider (0L .. 100L)
        mp2:  NULL
    process:  SEND
    returns:  FALSE
      hints:  when the process has completed, you should send separately a
              value of 100 to position the slider at the end of the sliderbar
  -----------------------------------------------------------------------------
*/


WM_EQF_SLIDER_SETTEXT,
/*-----------------------------------------------------------------------------
     descr.:  sent by a process to its slider window to change the text lines
              displayed in the slider window (be careful with text lines
              larger than the previously defined lines)
        mp1:  pointer to array of string pointers (PSZ *)
        mp2:  NULL
    process:  SEND
    returns:  FALSE
      hints:  the array containing pointers to the text lines must end with
              a NULL pointer otherwise the program may run into an error.
  -----------------------------------------------------------------------------
*/
WM_EQF_MEMLOAD_START,                   // memory Db load start msg
WM_EQF_MEMLOAD_PROCESS,                 // memory Db load process msg
WM_EQF_MEMLOAD_END,                     // memory Db load end msg
WM_EQF_MEMFILL_LISTBOX,                 // fill or renew the memory database list box

WM_EQF_MEMEXPORT_START,                 // memory Db export start msg
WM_EQF_MEMEXPORT_PROCESS,               // memory Db export process msg
WM_EQF_MEMEXPORT_END,                   // memory Db export end msg
WM_EQF_MEMORGANIZE_START,               // TM organize start msg
WM_EQF_MEMORGANIZE_PROCESS,             // TM organize process msg
WM_EQF_MEMORGANIZE_END,                 // TM organize end msg
WM_EQF_MEMMERGE_START,                  // TM merge start msg
WM_EQF_MEMMERGE_PROCESS,                // TM merge process msg
WM_EQF_MEMMERGE_END,                    // TM merge end msg
WM_EQF_QUERY,
/*-----------------------------------------------------------------------------
     descr.:  This message is sent to an instance in order to ask it about
              an activity.
        mp1:  ????
        mp2:  ????
    process:  SEND
    returns:  TRUE a process is active
              FALSE no process is active
  -----------------------------------------------------------------------------
*/

WM_EQF_FONTCHANGED,                    // font change in service window
/*
                            -----------------
                              TM Com Server
                            -----------------
*/

WM_EQF_COM_INITIALIZE,
/*-----------------------------------------------------------------------------
     descr.:  used to start the initialization process of the EQFTMCOM code
              (is posted after WM_CREATE message)

        mp1:  NULL
        mp2:  NULL
    process:  POST
    returns:  TRUE
      hints:  after this window message the coordination thread is spawned

  -----------------------------------------------------------------------------
*/

WM_EQF_COM_EXIT_COMPLETE,
/*-----------------------------------------------------------------------------
     descr.:  used to tell EQFTMCOM that the coordination thread has exited

        mp1:  NULL
        mp2:  NULL
    process:  POST
    returns:  TRUE
      hints:  this message is posted from the coordination thread to indicate
              completion of the thread end processing; after receiving this
              message, the window procedure polls a flag waiting for real
              completion of the thread, then frees its memory and terminates
              the TM Server code

  -----------------------------------------------------------------------------
*/
WM_EQF_MEMORGANIZE_ACTIVATE,
WM_DRIVEBUTTON_INIT,
WM_DRIVEBUTTON_SELECT,
WM_EQF_PROOF,
WM_EQF_MISSPELLED,
WM_EQF_AUTOTRANS,
/*-----------------------------------------------------------------------------
     descr.:  used to tell that DDE_REQUEST is waiting

        mp1:  (DDETASK) Task
        mp2:  (PVOID) pTaskIda
    process:  POST
    returns:  TRUE
      hints:  this message is posted from the dde handler instance to the
              handler which is necessary to execute the task defined in
              the DDE request from the DDE client

  -----------------------------------------------------------------------------
*/
WM_EQF_DDE_REQUEST,
/*-----------------------------------------------------------------------------
     descr.:  used to tell that DDE_REQUEST finished work

        mp1:  (DDERETURN) DDEReturn
        mp2:  NULL
    process:  POST
    returns:  TRUE
      hints:  this message is posted from the handler to the ddehandler
              instance to indicate that the requested task is completed
              mp1 holds the return structure with the error code and message

  -----------------------------------------------------------------------------
*/
WM_EQF_DDE_ANSWER,
LM_EQF_SETITEMSTATE,
LM_EQF_QUERYITEMSTATE,
LM_EQF_INSERTITEMSTATE,
WM_EQFN_DRIVEREMOVED,
WM_EQFN_DRIVEADDED,
WM_EQF_COLCHANGED,
WM_EQF_WD_MAIN_NOTIFY,
WM_EQF_INITMENU,                       // message for init menu from TWB AAB
WM_EQF_BUILDITEMTEXT,
/*-----------------------------------------------------------------------------
     descr.:  sent to a list instance callback function to create item text
              of an list object. The item text is to be stored in the
              szBuffer area of the list instance communication area
        mp1:  unused
        mp2:  PSZ  - object name
    process:  SEND
    returns:  BOOL - TRUE if item text for object could be created
                     FALSE if function failed
  -----------------------------------------------------------------------------
*/
WM_EQF_CREATELISTWINDOW,              // create a list instance window
WM_EQF_UPDATESLIDER,                  // send to a process window to update the
                                      //   slider position
WM_EQF_SHOWPOPUP,                     // send to any window to handle the
                                      //   request for a pop-window
EQF_CD_BM_CLICK,                      //  Used in the TM create dialog
WM_EQF_CHECK_REM_PROPS,               //  check which REMote TM's are available
WM_EQF_FILTSETDICT,                   // set dictionary handle of a filter control
WM_EQF_QUERYID,                       // query the ID of a dialog or window
WM_EQF_CHANGEOBJECTNAME,
/*-----------------------------------------------------------------------------
     descr.:  sent to object manager to change the name associated
              with an object
        mp1:  HWND to object
        mp2:  PSZ = pointer to new object name
    process:  SEND/POST
    returns:  BOOL - TRUE if requested object  was not found or rejected
                          the request
                   - FALSE if object  was successfully removed
  -----------------------------------------------------------------------------
*/
WM_EQF_CLOSE,                          // used instead of WM_CLOSE inside dialogs

/*-----------------------------------------------------------------------------
     descr.:  sent to column listbox control to query the item located under
              the current pointer position
        mp1:  0L (unused)
        mp2:  0L (ununsed)
    process:  SEND
    returns:  SHORT - LIT_NONE if there is no item under the pointer
                    - other    index of listbox item
  -----------------------------------------------------------------------------
*/
LM_EQF_GETITEMFROMPOINT,
/*-----------------------------------------------------------------------------
     descr.:  sent to column listbox control to force a refresh of the CLB
        mp1 = NULL     not used
        mp2 = NULL     not used
    process:  SEND/POST
    returns:  0L
  -----------------------------------------------------------------------------
*/
LM_EQF_REFRESH,
WM_EQF_MT_TMMERGE, // force merge of an MT memory
// WM_EQF_SHOWHTML has been moved to EQF_API.H for access by other parser user exits
// WM_EQF_SHOWHTMLXXXX is only a placeholder for this message
WM_EQF_SHOWHTMLXXXX,  // place holder for WM_EQF_SHOWHTML message
WM_EQF_IS_TENV,    // Indicate if processing window is Transl.Environment.
/*-----------------------------------------------------------------------------
     descr.:  sent to active handler to determine state of toolbar item
        mp1 = usID     id of toolbar item
        mp2 = NULL     not used
    process:  SEND
    returns:  TRUE:  toolbar is enabled, FALSE otherwise
  -----------------------------------------------------------------------------
*/
WM_EQF_TOOLBAR_ENABLED,
WM_EQF_MDIACTIVATE,
WM_EQF_DRAGDROP,
WM_EQF_COLUMNLIST,
WM_EQF_FLLTREE_COMMAND,   // WM_COMMAND message issued from the foldertree
//
// get currently active column listbox filter
LM_EQF_QUERYFILTER,
//
// set column listbox filter
LM_EQF_SETFILTER,
WM_EQF_UPDATERTFCTRL,   // update the RTF control
WM_EQFN_DELETEDNAME,    // like EQFN_DELETED but with name instead of object name
WM_EQF_UPDATESTATUSBAR_TEXT, // MP2: Text to be displayed...
LM_EQF_SETITEMTEXT,
LM_EQF_QUERYITEMTEXT,
WM_EQF_TAGTABLEPROPS,
WM_EQF_GETCOLUMNWIDTH,
WM_EQF_MDIMINALL,
WM_EQF_MDIRESTOREALL,
WM_EQF_READINQUE,
WM_EQF_ADDMEMTOLIST,
WM_EQF_REMOVEMEMFROMLIST,
WM_EQF_END         // this id ends the range of messages

} EQFMSGIDS;

#define HM_HELPSUBITEM_NOT_FOUND WM_EQF_END + 10

//------------------------------------------------------------------------------
//  Task identifiers for WM_EQF_PROCESSTASK message
//------------------------------------------------------------------------------

#define IMPORT_TASK   20001            // post-import processing
#define EXPORT_TASK   20002            // pre-export processing
#define CREATE_PATH   20003            // create a fully qualified object name
#define QUERY_LANG_PROP_INPUT_MEM   20004 // query language props of TM
#define QUERY_LANG_PROP_INPUT_LANG  20005 // query language props of language
#define TM_SERVER_REFERENCE_TASK    20006 // query TM references on/of server
#define TM_QUERY_SERVER_TASK        20007 // query server name for a TM
#define QUERY_LANG_PAIR_INPUT_MEM   20008 // query language pair for a TM
#define QUERY_MARKUP_INPUT_MEM      20009 // query markup for a TM
#define BATCHIMPORT_TASK            20010 // post-import processing in batch mode
#define OPEN_AND_POSITION_TASK      20011 // open doc and position to seg
#define ADJUST_STATUSBAR_TASK       20012 // adjust statusbar within TPRO
#define RENAMEOBJECT_TASK           20013 // rename given object
#define REIMPORT_TASK               20014 // re-import a document from the TE
#define USER_TASK     20500            // start value for user defined tasks

#endif

// status flag for Eqf objects
#define OBJ_ACTIVE   0x0001
#define OBJ_FOCUS    0x0002
#define OBJ_BUSY     0x0004
#define OBJ_MANAGER  0x8000

typedef struct _objlst {
   HWND    hwnd;
   USHORT  flgs;
   USHORT  usClassID;
} OBJLST, *POBJLST;


__declspec(dllexport)
HWND      EqfQueryObjectManager( VOID);
__declspec(dllexport)
USHORT    Send2AllHandlers( WINMSG, WPARAM, LPARAM);
__declspec(dllexport)
USHORT    Send2AllObjects( USHORT, WINMSG, WPARAM, LPARAM);
__declspec(dllexport)
USHORT    InstallHandler( PSZ name, HWND hwnd, USHORT cls);
__declspec(dllexport)
USHORT    RegisterObject( PSZ name, HWND hwnd, USHORT cls);
__declspec(dllexport)
MRESULT   APIENTRY OBJECTMANAGERWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport)
HWND      EqfQueryObject( PSZ pszObj, SHORT sClass, SHORT sFlags );
__declspec(dllexport)
SHORT     EqfQueryObjectName( HWND hwnd, PSZ pBuffer );
__declspec(dllexport)
USHORT    EqfSetObjectStatus( HWND hwnd, SHORT sMask, SHORT sStatus );
__declspec(dllexport)
USHORT    EqfGetObjectList( SHORT sClass, SHORT sCount, PVOID pBuffer );
__declspec(dllexport)
MRESULT   EqfSend2Handler( PSZ psz, WINMSG msg, WPARAM mp1, LPARAM mp2);
__declspec(dllexport)
MRESULT   EqfPost2Handler( PSZ psz, WINMSG msg, WPARAM mp1, LPARAM mp2);
__declspec(dllexport)
USHORT    ObjBroadcast( WINMSG msg, SHORT sClass, PSZ pszObjName );

/**********************************************************************/
/* parent and ownership for dialogs called using WinDlgBox            */
/**********************************************************************/
#define DIALOG_PARENT      HWND_DESKTOP
#define DIALOG_OWNER       EqfQueryTwbClient()

// batch mode  / DDE connection defines
#define HWND_DDE           (HWND) 10
#define ISBATCHHWND(hwnd) FALSE // old define: DDEIsBatchHwnd(hwnd)
// pseudo window handle to identify function call I/F error messages
#define HWND_FUNCIF           (HWND)11


/**********************************************************************/
/* general resource files                                             */
/**********************************************************************/
#define EQFRES_DLL                   "EQFRES.DLL"    // general resource DLL

#define TWBRESOURCE_DLL              "EQFSTARR.DLL"  // workbench resource
#define TWBIANARES_DLL               "EQFIANAR.DLL"  // analysis resource
#define TWBDICTRES_DLL               "EQFDICTR.DLL"  //
#define EQF_MEM_RESOURCE_DLL         "EQFMEMR.DLL"   // Memory resource DLL
#define TWBCOUNTRES_DLL              "EQFCNTR.DLL"   // Word count resource
#define TWBTAGIMPRES_DLL             "EQFTIMPR.DLL"  // tag table import resource
#define TWBTAGEXPRES_DLL             "EQFTEXPR.DLL"  // tag table export resource
#define TP_RESOURCE_DLL              "EQFBROWR.DLL"  // translation processor res.
#define TWBDLUPRES_DLL               "EQFDDLLR.DLL"  // dictionary lookup resource
#define TWBLISTRES_DLL               "EQFLISTR.DLL"  // list handler resource
#define EQFLNGR_DLL                  "EQFLNGR.DLL"   // list handler resource
#define TWBFILTERRES_DLL             "EQFFILTR.DLL"  // filter handler resource
#define TWBDICTSERVRES_DLL           "EQFDDLLR.DLL"  // dictionary services resource
#define EQFLOGOR_DLL                 "OTMLOGOR.DLL"  // logo bitmaps resource DLL
#define QDPR_RES_DLL                 "EQFQDPRR.DLL"  // QDPR ressource DLL

/**********************************************************************/
/* special DLLs                                                       */
/**********************************************************************/
// NETAPI (LAN services) DLL
#define NETAPI_DLL                   "NETAPI"

/**********************************************************************/
/* general character and pointer constants                            */
/**********************************************************************/
#define MULTI_LINGUAL_CODEPAGE  850    // Multilingual Code Page
#define NULC      '\0'
#define NULP      (void *)0
#define EOS          '\0'              // end of string delimiter
#define BACKSLASH    '\\'              // backslash character
#define COLON        ':'               // colon character
#define CR           '\r'              // carriage return character
#define LF           '\n'              // line feed character
#define CRLF_STR     "\r\n"            // carriage return line feed string
#define CRLF_STRINGW  L"\r\n"            // carriage return line feed string
#define SOFTLF_CHAR  '\x07'            // lf for autolinewrap (softlf)
#define QUOTE        '\''              // quotation character
#define DOUBLEQUOTE      '\"'                      // double quote character
#define COMMA            ','                       // comma character (used as delimiter)
#define DOT          '.'               // dot/period character
#define BLANK        ' '               // SBCS blank character
#define SPACE        ' '               // space or blank character
#define EOFCHAR      '\x1A'            // end-of-file character
#define NUMBERSIGN   '#'               // number sign
#define X15          '\x15'            // internal delimiter character
#define X15_STR      "\x15"            // internal delimiter character as string
#define X15_STRW     L"\x15"            // internal delimiter character as string
#define EMPTY_STRING ""                // empty string
#define EMPTY_STRINGW L""       // empty string (Unicode version)
#define DEFAULT_PATTERN  "*.*"         // default name when looking for files
#define FILE_LISTSEL     "from list"
#define BACKSLASH_STR           "\\"   // backslash string (directory seperator)
#define DEFAULT_PATTERN_NAME    "*"    // default names to be shown for load dialog
#define DEFAULT_PATTERN_EXT     ".*"   // default extension to be shown for load dialog
#define CURRENT_DIR_NAME        "."    // file name of current directory file
#define PARENT_DIR_NAME         ".."   // name of parent directory
#define MULTIPLE_SUBSTITUTION   '*'    // multiple substitution character
#define SINGLE_SUBSTITUTION     '?'    // single substitution character
#define SO                      '\x0e' // SO (shift out) character
#define SI                      '\x0f' // SI (shift in) character
#define KEY_FIRSTTIME    "FirstTime"
#define VALUE_NO         "N"
#define VALUE_YES        "Y"
#define DEFAULTBARHEIGHT 27           // default toolbar height ...
#define SEMICOLON           ';'

/**********************************************************************/
/* size and length defines                                            */
/**********************************************************************/
#define MAX_FILESPEC            13     // max length of file name + extension
#define MAX_LONGFILESPEC       256     // max length of long file name + extension
#define MAX_PATH144            144     // max length of fully qualified path
#define MAX_LONGPATH           260     // max length of fully qualified path
#define MAX_FNAME                9     // max length of name part of a file names
#define MAX_FEXT                 5     // max length of a file name extension
#define MAX_EQF_PATH            60     // max length of a file in an EQF dir
#define MAX_FOLDERNAME           8     // max length of a folder name
#define MAX_DRIVELIST           27     // max size of a drive list
#define MAX_DRIVE                3     // max size of a drive specification
#define MAX_VIEW                20     // max. viewable # of columns in a CLB listbox
#define MAX_DEFINEDCOLUMNS      50     // max. definable # of columns in a CLB listbox
#define MAX_MEM_DESCRIPTION     41     // length of a memory description field
#define MAX_LANG_LENGTH         20     // length of the name of a language
#define MAX_DESCRIPTION         41     // length of description fields
#define MAX_LONG_DESCRIPTION    300    // long length of description fields
#define MAX_DICTS               20     // max. number of dictionaries
#define MAX_SERVER_NAME         15     // max. length of a server name
#define MAX_SERVER_LIST_SIZE    100    // length of a server list in
                                       // system properties
#define MAX_USERID              15     // max. length of LAN user ID
#define MAX_LANGUAGE_PROPERTIES 200    // max. length of language properties
#define MAX_EDIT_LINE_LENGTH    66    // max. length of a line
#define MAX_SEGMENT_SIZE        2048  // maximum segment size
#define MAX_TERM_LEN            255   // maximum length of a single term
#define NUM_OF_DICTS            10    // number of dictionaries per folder
#define MAX_FINDCHANGE_LEN      80    // max length of find/change buffer
#define MAX_MOSTUSEDDOCSPERSEG   5    // max number of docs in most used segment
                                      // area of redundancy log
#define MAX_SEARCH_HIST         5     // max number of search history
#define MAX_FILELIST            8096  // max length of non-dde filelist buffer
#define MAX_DICTPID_VALUES      400   // max. length of dict. PID values

/*********************************************************************/
/* defines of CLB colum widths                                       */
/*********************************************************************/
#define CLB_DBCS_MAX_DATE           12     // max. length of the date
#define CLB_DBCS_MAX_DRIVE           8     // max size of a drive specification
#define CLB_DBCS_MAX_FNAME          12     // max length of name part of a file names
#define CLB_DBCS_MAX_LNG_STATE      12     // max. length of language state
#define CLB_DBCS_MAX_SEG_NUM         9           // max. length of number of seg.

#define CLB_MAX_DATE                12     // max. length of the date
#define CLB_MAX_DATE_TIME           24     // max. length for date and time
#define CLB_MAX_DESCRIPTION         50     // length of description fields
#define CLB_MAX_DIC_LENGTH          20     // max. length of dictionary
#define CLB_MAX_DOC_LENGTH          20     // max. length of document (with ext)
#define CLB_MAX_DRIVE                5     // max size of a drive specification
#define CLB_MAX_FNAME               24     // max length of name part of a file names
#define CLB_MAX_LANG_LENGTH         20     // length of the name of a language
#define CLB_MAX_LNG_STATE            8     // max. length of language state
#define CLB_MAX_NOISE_LIST          16     // max. length of noise list
#define CLB_MAX_OWNER_LENGTH        20     // max. length of owner (logon uid)
#define CLB_MAX_SEG_NUM              7     // max. length of number of seg.
#define CLB_MAX_SERVER_NAME          8     // max. length of a server name
#define CLB_MAX_SIZE_LENGTH          8     // max. length for filesize
#define CLB_MAX_SUPP_STATE          13     // max. length of supportfile state
#define CLB_MAX_VERSION              8     // max. length of version          
#define CLB_MAX_MT_STATUS           20     // max. length of MT status
#define CLB_MAX_MT_ENGINE           20     // max. length of MT engine


#define CLB_INSERTITEMSTATE_ENABLED  0x8000            // enable inserted item..            01416  INSERT       01386 LEN=    1

/**********************************************************************/
/* define profile name and keywords                                   */
/**********************************************************************/
#define APPL_Name        "OpenTM2"
#define KEY_Vers         "Version"
#define KEY_Drive        "Drive"
#define KEY_LanDrive     "LanDrive"
#define KEY_Path         "Path"
#define KEY_SysProp      "Sysprop"
#define KEY_MsgFile      "MsgFile"
#define KEY_HlpFile      "HlpFile"
#define KEY_SYSLANGUAGE  "UserLang"
#define KEY_MTEngine     "MTEngine"
#define KEY_MTMachine    "MTServer"
#define KEY_TMTitle      "TMTitle"
#define LOGOS_ENGINE     "LOGOS"
#define MTEngine_NA      "n/a"
#define DEFAULT_SYSTEM_LANGUAGE "English(U.S.)"            // default user language

#if defined(_TLEX)
  #define SYSLANGFILE      "PANELL.LNG"
#else
  #define SYSLANGFILE      "PANELW.LNG"
#endif

/* Eqf Termination Flags */
#define TWBCLOSE          1
#define TWBSAVE           2
#define TWBFORCE          8

/**********************************************************************/
/* Eqf property names and file extensions                             */
/**********************************************************************/
#define SYSTEM_PROPERTIES_NAME    "EQFSYSW.PRP"

#define IMEX_PROPERTIES_NAME      "EQFIMEX.PRP"
#define DEFAULT_FOLDERLIST_NAME   "FLIST.L00"
#define MEMORY_PROPERTIES_NAME    "EQFMEMN.PRP"
#define MEMORYLU_PROPERTIES_NAME  "EQFMEMLU.PRP"
#define EDITOR_PROPERTIES_NAME    "STANDARD.EDI"
#define DICT_PROPERTIES_NAME      "EQFDICT.PRP"
#define TAGTABLE_PROPERTIES_NAME  "EQFTAGTB.PRP"
#define LISTHANDLER_PROPERTIES_NAME "EQFLIST.PRP"

#define TPRO_PROFILE             "EQFPROW.PRP"
#define SPEC_CHAR_KEY_PROFILE     "OtmSpecCharKey.PRP"

#define EQFMTINIT                 "EQFMTINIT"    // init MT subsystem
#define EQFMTTRANS                "EQFMTTRANS"   // translate passed segment
#define EQFMTCLOSE                "EQFMTCLOSE"   // close MT subsystem
#define QDPR_PROP_FILENAME        "EQFQDPR.PRP"  // QDPR property file
#define EXT_FOLDER_MAIN           ".F00"
#define EXT_OF_SUBFOLDER          ".F01"
#define EXT_DOCUMENT              ".*"
#define EXT_OF_DIC                ".ASD"       // Extention of dictionaries
#define EXT_OF_EXTERNAL_DIC       ".ASX"       // Ext. of external dict. format
#define EXT_OF_ABBREV_DICT        ".ABR"       // extension of abbreviation dict
#define EXT_OF_MEM                ".MEM"       // Extention of memory properties
#define EXT_OF_FORMAT             ".TBL"       // Extention of format tables
#define EXT_OF_FORMATSETTINGS     ".CHR"       // Extention of format table setting files
#define EXT_OF_INTTABLE           ".IBL"       // Extention of internal tables
#define EXT_OF_EXTERNAL_FORMAT    ".TBX"       // Ext. of external format tables
#define EXT_OF_EXCLUSION          ".NOI"       // Extention of word lists
#define EXT_OF_NEWTERMS_LIST      ".NTL"       // Extention of new terms list
#define EXT_OF_FOUNDTERMS_LIST    ".FTL"       // Extention of found terms list
#define LPEX_MACROEXT             ".LXL"       // Extension of LPEX macros
#define EXT_OF_EDITOR             ".EDI"       // Extention of editor properties
#define EXT_OF_DICTPROP           ".PRO"       // Ext. of dict properties
#define EXT_OF_DICTINDEX          ".ASI"       // Ext. of dict index
#define EXT_TMP_DICTPROP          ".TPR"       // Ext. of temp dict prop
#define EXT_TMP_DICTPROP_COPY     ".TPC"       // Ext. of temp dict prop copy
#define EXT_TMP_DICTINDEX         ".TAI"       // Ext. of temp dict index
#define EXT_TMP_DICTINDEX_COPY    ".TIC"       // Ext. of temp dict index copy
#define EXT_TMP_DIC_COPY          ".TSC"       // Ext. of temp dict copy
#define EXT_TMP_DIC               ".TAS"       // Ext. of temp dict
#define EXT_OF_DLL                ".DLL"       // Extention of DLLs
#define QDPR_FORMAT_EXT           ".FRM"       // extension of a QDPR format file
#define  MEM_EXPORT_PATTERN_EXT   ".EXP"       // Default extension for an
#define EXT_OF_SHARED_DIC         ".RAD"       // Extention of shared dict
#define EXT_OF_SHARED_DICTINDEX   ".RAI"       // Ext. of shared dict index
#define EXT_OF_SHARED_DICTPROP    ".RPR"       // Ext. of shared dict properties
#define EXT_OF_SHARED_MEM         ".RMD"       // Extention of shared TM
#define EXT_OF_SHARED_MEMINDEX    ".RMI"       // Ext. of shared TM index
#define EXT_OF_SHARED_MEMPROP     ".RMP"       // Ext. of shared TM properties
                                               // exported memory database
#define EXT_OF_TMDATA             ".TMD"       // Ext. of TM data file
#define EXT_OF_TMINDEX            ".TMI"       // Ext. of TM index file
#define EXT_OF_TEMP_MEM           ".MET"       // Temporary translation memory (For Organize)
#define EXT_OF_RENAMED_MEM        ".MER"       // Renamed original memory (For Organize)
#define EXT_OF_STEMCACHE          ".STU"       // extention for stem form cache files - Unicode case
#define EXT_OF_NOLOOKUP           ".NLU"       // extention for no-lookup hash table - Unicode case
#define EXT_OF_POSCACHE           ".POW"       // extention for POS cache files - Unicode case
#define EXT_OF_POSNOLOOKUP        ".PNW"       // extention for POS no-lookup hash table - Unicode case
#define EQFXSTART    "EQF_XSTART"              // start editor procedure name
#define EQFXSTARTEX  "EQF_XSTARTEX"            // start editor procedure name for multiple docs.
#define MULT_DOCUMENTS   2                     // support multiple documents
#define OPENPOS_USED     3                     // open called with openpos structure
#define EQFXSTOP     "EQF_XSTOP"               // stop  editor procedure name
#define EMPTY_TAG      L":NONE."               // empty tag

#define FOLDEREXPORT_EXT     ".FXP"            // ext. of an exported folder
#define DOCUMENTEXPORT_EXT   ".DXP"            // ext. of an exported docs
#define DICTIONARYEXPORT_EXT ".IXP"            // ext. of an exported dict
#define EXT_OF_FILTPROP      ".FIL"            // ext. of filter properties

#define TM_EXE   "EQFTMMNL.EXE"          // Tutorial Manager exe file name
#define TUT_NAME "EQFTWB.PAC"            // standard tutorial name
#define VIEW_EXE "VIEW.EXE"              // name of view program
#define MAT_BOOK "MATBOOK"               // name of mat book

#define CONVERSIONTABLENAME "EQFCONV.LST" // name of file with conversion entries

// EQF predefined object names
#define MEMORY_ALL            "MemoryAll"        // alias for 'all memory dbs'
#define FORMAT_TABLE_ALL      "FormatTableAll"   // alias to get all Format Tables
#define EXCLUSION_LIST_ALL    "ExclusionListAll" // alias to get all Exclusion List
#define SOURCE_LANGUAGES_ALL  "SourceLanguagesAll" // alias to get all Source Languages
#define TARGET_LANGUAGES_ALL  "TargetLanguagesAll" // alias to get all Target Languages
#define EXCLUSIONLISTOBJ      "ExclusionList"    // objname for exclusion lists
#define NOISELISTOBJ          "NoiseList"        // objname for noise lists
#define NEWTERMLISTOBJ        "NewTermList"      // objname for new terms lists
#define FOUNDTERMLISTOBJ      "FoundTermList"    // objname for found terms lists
#define ABBRLISTOBJ           "Abbreviation"     // objname for abbreviation lists


/**********************************************************************/
/* Tag table names                                                    */
/**********************************************************************/
#define QFTAG_TABLE            "QFTAGS"          // standard name of QF tags table
#define MEM_FORMAT_TABLE       "MEMTABLE"        // Name of memory tag table
#define DIMPTAGTABLE           "DICFORMT"        // name of tag table for dict creation
#define TIMPTAGTABLE           "FORMAT"          // name of tag table for tag tables
#define DEFAULT_DOCUMENT_TABLE "EQFBOOK"         // default document tag table
#define DEFAULT_QFTAG_TABLE    "QFTAGS"          // default QF tag table
#define QDPR_TAGTABLE_NAME     "QDPRTAGS"        // QDPR tag table name
#define QFSHOW_TABLE           "QFSHOW"          // tagtable for show WYSIWYG
#define LISTFORMATTABLE        "LISTFORM"        // name of list format tag table

// attention: the values must be less or equal to the equivalent MAX... value
#define NUM_OF_FOLDER_DICS        10   // folder's list of dictionaries
#define NUM_OF_FOLDER_MDB         10   // folder's table of memory dbs
#define NUM_OF_READONLY_MDB        5   // folder's R/O memory dbs

// boundaries of arrays in folder properties structure
#define MAX_NUM_OF_FOLDER_DICS    15   // folder's list of dictionaries
#define MAX_NUM_OF_FOLDER_MDB     25   // folder's table of memory dbs
#define MAX_NUM_OF_READONLY_MDB   25   // folder's R/O memory dbs

/**********************************************************************/
/* Handler and instance names                                         */
/**********************************************************************/
#define TWBMAIN                      "OpenTM2"
#define PROPERTYHANDLER              "EqfPropertyHandler"
#define OBJECTMANAGER                "EqfObjectManager"
#define FOLDERLISTHANDLER            "EqfFolderListHandler"
#define FOLDERHANDLER                "EqfFolderHandler"
#define DOCUMENTHANDLER              "EqfDocumentHandler"
#define ANALYSISHANDLER              "EqfAnalysisHandler"
#define FOLDERLIST                   "EqfFolderList"
#define FOLDER                       "EqfFolder"
#define DOCUMENT                     "EqfDocument"
#define ANALYSIS                     "EqfAnalysis"
#define DICTIONARYHANDLER            "EqfDictionaryHandler"
#define DICTIONARY                   "EqfDictionary"
#define MEMORYHANDLER                "EqfMemoryHandler"
#define MEMORY                       "EqfMemory"
#define PROFILEHANDLER               "EqhProfileHandler"
#define DICTIMP                      "EqfDictionaryImport"
#define DICTEXP                      "EqfDictionaryExport"
#define COUNTHANDLER                 "EqfCountHandler"
#define COUNTINSTANCE                "EqfCountInstance"
#define TAGTABLEHANDLER              "EqfTagTableHandler"
#define TAGTABLEINSTANCE             "EqfTagTableInstance"
#define FOLIMP                       "EqfFolderImport"
#define FOLEXP                       "EqfFolderExport"
#define SERVICESHANDLER              "EqfServices"
#define SERVICESINSTANCE             "EqfServicesInstance"
#define LISTHANDLER                  "EqfListHandler"
#define LISTINSTANCE                 "EqfList"
#define LNGUPDATEHANDLER             "EqfLngUpdateHandler"
#define FILTERHANDLER                "EqfFilterHandler"
#define DDEHANDLER                   "EqfDDEHandler"
#define GENERICHANDLER               "EqfGenericHandler"
#define GENERICLIST                  "EqfGenericList"
#define GENERICPROCESS               "EqfGenericProcess"
#define REPORTHANDLER                "EqfReportHandler"
#define MTLISTHANDLER                "EqfMTListHandler"
#define TMM_CLASS                    "EqfTMMClass"       // class name
#define TQMLISTHANDLER               "EqfTqmListHandler"

#define DICTENTRYLENGTH 30             /* length of the name of a dict entry  */
#define FONTNAMELENGTH  40             /* length of the a font name           */

// prefixes for process object names
#define MEMMERGE_PROC_OBJPREFIX "MEMMERGE: "
#define DICIMP_PROC_OBJPREFIX   ""

/**********************************************************************/
/* Names of system semaphores                                         */
/**********************************************************************/
#define TMCOM_SEMAPHORE "\\SEM\\EQFTMCOM\\ISACTIVE"        // sem for active EQFTMCOM
#define TWBACTSEM       "\\SEM\\EQF\\ISACTIVE"      // sem for active TWB

//-------------------- symbolic values for profile entries ---------------------
//--- defines for dict entry authorization ---
#define RW_STATUS  1                   // read/write status
#define RO_STATUS  2                   // read only status
#define NA_STATUS  3                   // no access

//--- defines for profile types ---
#define TYPE_MASTER 0                  // system created profile
#define TYPE_USER 1                    // user created profile

//--- defines for type of entry field ---
#define TYPE_SMALL 1                   // 256 bytes / SLE
#define TYPE_LARGE 2                   // 512 bytes / MLE

//--- defines for dictionary entry display via profile ---
#define NO_DISPLAY 0                   // not displayed at all
#define FIRST_DISPLAY 1                // displayed on first panel
#define SECOND_DISPLAY 2               // displayed on second panel
#define THIRD_DISPLAY 3                // displayed on third panel


// The following enum values can be used to extract data from the string
// via the utility UtlParseX15
typedef enum
{
  LANG_LANGUAGE_IND,
  LANG_MORPH_DICT_IND,
  LANG_LANG_CODE_IND,
  LANG_NOISE_LIST_IND,
  LANG_STOPWORD_LIST_IND,
  LANG_ADDENDA_DICT_IND,
  LANG_COMPOUND_SEP_IND,
  LANG_LANGUAGE_EXIT_IND,
  LANG_CODEPAGE_IND,
  LANG_GROUP_IND,
  LANG_SPELLSUPPORT_IND,
  LANG_ANSICODEPAGE_IND,
} LANG_PROPINDEX;

// Example: Extract the name of the noise list after successfull return
//          with the following statement
// strcpy( pszNoiseList, UtlParseX15( pszString, LANG_NOISE_LIST_IND ));

// The following enum values can be used to extract data via the utility
// UtlParseX15 from the string returned by QUERY_LANG_PAIR_INPUT_MEM
typedef enum
{
  SOURCE_LANGUAGE_TM_IND,
  TARGET_LANGUAGE_TM_IND
} LANGUAGE_PAIR_OF_TM;

// token IDs for QF... tags
#define EQFA_TAG     0
#define EQFC_TAG     1
#define EQFF_TAG     2
#define EQFJ_TAG     3
#define EQFN_TAG     4
#define EQFS_TAG     5
#define EQFX_TAG     6
#define NONE_TAG     7
#define QFA_TAG      8
#define QFC_TAG      9
#define QFF_TAG     10
#define QFJ_TAG     11
#define QFMARK_TAG  12
#define QFN_TAG     13
#define QFS_TAG     14
#define QFX_TAG     15
#define CURRENT_ATTR 16
#define MARK_ATTR   17
#define NOCOUNT_ATTR 18
#define JOIN_ATTR    19
#define N_ATTR       20
#define STATUS_ATTR  21
#define COUNT_ATTR   22

typedef enum _QSTATUS                  // segment status (:QFx tags)
{
   QF_XLATED,                          // is translated
   QF_TOBE,                            // to be translated
   QF_NOP,                             //
   QF_ATTR,                            //
   QF_CURRENT,                         //
   QF_PROP0TEXT,                       // proposal 0 text
   QF_PROP0PREFIX,                     // proposal 0 prefix
   QF_PROPNTEXT,                       // proposal n text
   QF_PROPNPREFIX,                     // proposal n text
   QF_DICTHEAD,                        // dictionary header
   QF_DICTTRANS,                       // dictionary translation
   QF_DICTPREFIX,                      // dictionary prefix
   QF_PROPSRCEQU,                      // proposal source equal text
   QF_PROPSRCUNEQU,                    // proposal source unequal text
   QF_JOINED,                          //
   QF_SPLIT,                           //
   OF_PROPSRCINS,
   OF_PROPSRCDEL,
   QF_PROPSRCDEL,                      // proposal Source Deleted
   QF_PROPSRCINS,                      // proposal Source Deleted
   QF_ANCHOR_1,                          // VisITM: anchor
   QF_ANCHOR_2,                          // VisITM: anchor
   QF_ANCHOR_3,                          // VisITM: anchor
   QF_VALID_01,                        // VisITM: tgt segment not aligned
   QF_VALID_10,                        // VisITM: src segment not aligned
   QF_VALID_11_1,                      // VisITM:aligned
   QF_VALID_11_2,                      // VisITM:
   QF_VALID_11_3,                      // VisITM:
   QF_CROSSED_OUT,                     // VisITM: seg crossed out by user
   QF_NOP_ANCHOR_1,                      // VisITM:nop seg as anchor
   QF_NOP_ANCHOR_2,                      // VisITM:nop seg as anchor
   QF_NOP_ANCHOR_3,                      // VisITM:nop seg as anchor
   QF_VISACT,                            // active VisITM alignment
   QF_OVERCROSS,                         // user anchor overcrossing anchors
   QF_DICTADDINFO,                       // additional info for dict terms
   QF_CROSSED_OUT_NOP,                   // VisITM: nop temp. crossed out
   QF_OWN_COLOR,                         // use color as provided in high byte
   QF_TRNOTE_L1_1,                       // trnote level 1, first color
   QF_TRNOTE_L1_2,                       // trnote level 1, second color
   QF_TRNOTE_L2,                         // trnote level 2, color
   QF_DICTINDIC,                         // dictionary indicator
   QF_DICTSTYLEPREF,                     // dictionary style indicator "preferred term"
   QF_DICTSTYLENOT,                      // dictionary style indicator "not allowedterm"
   QF_MACHPROPOSAL,                      // machine match proposal
   QF_FUZZYPROPOSAL                      // fuzzy match proposal
} QSTATUS;

/**********************************************************************/
/* Symbolic values for list tag table tokens                          */
/* The tokens must be in sorted order based on the strings in the     */
/* tag table.                                                         */
/**********************************************************************/
typedef enum _LISTTAGS
{
  ECONREF_TAG,
  ECONTEXT_TAG,
  ECREATEDATE_TAG,
  ECREATETIME_TAG,
  EDEST_TAG,
  EDICTNAME_TAG,
  EEXCLDICT_TAG,
  EEXCLLISTNAME_TAG,
  EFREQUENCY_TAG,
  EFTLIST_TAG,
  EHEADER_TAG,
  ELEMMA_TAG,
  EMARK_TAG,
  ENTLIST_TAG,
  EOUTDICT_TAG,
  ETERM_TAG,
  ETEXTNAME_TAG,
  ETRANSLATION_TAG,
  CONREF_TAG,
  CONTEXT_TAG,
  CREATEDATE_TAG,
  CREATETIME_TAG,
  DEST_TAG,
  DICTNAME_TAG,
  EXCLDICT_TAG,
  EXCLLISTNAME_TAG,
  FREQUENCY_TAG,
  FTLIST_TAG,
  HEADER_TAG,
  LEMMA_TAG,
  MARK_TAG,
  NTLIST_TAG,
  OUTDICT_TAG,
  TERM_TAG,
  TEXTNAME_TAG,
  TRANSLATION_TAG,
  ID_ATTR
} LISTTAGS;

// Define for Tmt Commands
#define TMC_CREATE                         1
#define TMC_OPEN                           2
#define TMC_CLOSE                          3
#define TMC_INFO                           4
#define TMC_ADD                            5
#define TMC_DELETE                         6
#define TMC_REPLACE                        7
#define TMC_GET                            8
#define TMC_EXTRACT                        9
#define TMC_GET_PART_OF_TM_FILE           10

// Define for Server Internal Commands
#define TMC_CLOSE_QUERY_HANDLER           19
#define TMC_EXIT_HANDLER                  20
#define TMC_CLOSE_HANDLER                 21
#define TMC_SPAWN_TMPIPE                  22
#define TMC_EXIT                          23

// Define for Server Application Commands
#define TMC_GET_SERVER_DRIVES             24
#define TMC_GET_SERVER_TM_LIST            25
#define TMC_PUT_TM_PROPERTIES             27
#define TMC_GET_TM_PROPERTIES             28
#define TMC_END_ORGANIZE                  29
#define TMC_DELETE_TM                     30
#define TMC_DELETE_FILE                   31
#define TMC_QUERY_FILE_INFO               32
#define TMC_UPDATE_TM_PROPERTIES          33
#define TMC_FORCE_CLOSE                   34

//Define for dict server application commands
#define GET_SERVER_DICT_DRIVES            40
#define GET_SERVER_DICT_LIST              41
#define PUT_DICT_PROPERTIES               42
#define GET_DICT_PROPERTIES               43
#define QDAM_FUNCTION                     44
#define QDAM_OPEN                         45
#define QDAM_CREATE                       46
#define QDAM_CLOSE                        47
#define QDAM_RENAME                       48
#define QDAM_DELETE_FILE                  49
#define UPDATE_DICT_PROPERTIES            50
#define QDAM_GET_PART                     51
#define QDAM_CLOSEORGANIZE                52

// define of window classes
#define WC_EQF_FILTER   "EqfFilter"    // window class for filter controls



typedef CHAR FILENAME  [CCHMAXPATH];
typedef CHAR SERVERNAME [MAX_SERVER_NAME];

// return value for WM_CREATE if window should not be created
#define DO_NOT_CREATE   -1


/**********************************************************************/
/* Tokenlist and exclusion list defines which are of general interest */
/**********************************************************************/
/*----------- token values ----------------------------------------------------*/
#define TEXT_TOKEN -1                /* Text token                              */
#define TA_WORD -2                   /* Word token                              */
#define NOISEWORD -3                 /* Noise word token                        */
#define NOISECHAR -4                 /* Noise character token                   */
#define ENDOFLIST -5                 /* end of list token                       */
#define WHITESPACE -6                /* WHITESPACE text token                   */
#define USELESSWORD -7               /* useless word token                      */
#define DUMMYTAG    -8               /* dummy tag token which indicates end of */
                                     /* associated text                        */
typedef struct _TOKENENTRY     // entry in tokenlist :
{
  // !!!! Attention: below has to match TOKENENTRYSEG definition ....  !!!!
  SHORT     sTokenid;          // Tokenid
  USHORT    usLength;          // Length of data string
  SHORT     sAddInfo;          // additional information from tag table
  CHAR    * pDataString;       // pointer to data string
  USHORT    usOrgId;           // original id
  USHORT    ClassId;           // class id of token
  CHAR_W * pDataStringW;       // pointer to data string  - Unicode
  // !!!! Attention: above has to match TOKENENTRYSEG definition ....  !!!!

} TOKENENTRY, *PTOKENENTRY;

typedef struct _EXCLUSIONLIST
   {
   USHORT        uLength;                  /* length of exclusion list in byte  */
   USHORT      usNumEntries;             /* number of entries in list         */
   USHORT        uFirstEntry;              /* pointer to first entry (offset)   */
   USHORT        uStrings;                 /* offset to noise strings           */
   } EXCLUSIONLIST, *PEXCLUSIONLIST;

/**********************************************************************/
/* Object classes                                                     */
/* also defines parent-child relationship for TWB                     */
/**********************************************************************/

typedef enum {
   clsANY,                            // must be zero
   clsTWBMAIN,
   clsPROPERTY,
   clsSYMBOL,
   clsFOLDERLIST,
   clsFOLDER,
   clsDOCUMENT,
   clsIMEX,
   clsANALYSIS,
   clsDICTIONARY,
   clsMEMORY,
   clsMEMORYDB,
   clsDICTIMP,                         // dictionary import object class
   clsWORDCOUNT,
   clsTAGTABLE,                        // tag table instance object class
   clsTAGIMP,                          // tag table import instance object
   clsTAGEXP,                          // tag table export instance object
   clsDICTEXP,                         // dictionary export object class
   clsSERVICES,                        // services object class
   clsNONE,                            // 'do not register' class
   clsFOLDEREXP,                       // folder export class
   clsFOLDERIMP,                       // folder import class
   clsSLIDER,                          // slider windows
   clsDICTLOOKUP,                      // dictionary lookup windows
   clsDICTDISP,                        // dictionary entry display windows
   clsLIST,                            // list handler window(s)
   clsLNGUPDATE,                       // language update
   clsDICTORG,                         // dictionary organize
   clsFILTER,                          // filter controls
   clsDDE,                             // DDE controls
   clsMTJOB,                           // TQM vendor
   clsFree2,                           // TQM document
   clsFree3,                           // TQM vendor lists
   clsDICTPRINT,                       // dictionary print
   clsMEMORYORG,                       // TM organize
   clsMEMORYIMP,                       // TM import
   clsMEMORYEXP,                       // TM export
   clsMEMORYMERGE,                     // TM merge
   clsFOLFIND,                         // Global-Find-And-Change
   clsTMMAINT,                         // Translation Memory maintenance
   clsREPORT,                          // Counting Report
   clsARCHTM,                          // Build Archive TM
   clsMTLIST,                          // MT listwindow
   clsTQMLIST,                         // TQM list window
   clsFUZZYSEARCH                      // fuzzy search window
} CLASSES;

typedef ULONG EQFINFO, *PEQFINFO;
typedef CHAR  OBJNAME[MAX_PATH144+MAX_FILESPEC];
typedef OBJNAME *POBJNAME;
typedef CHAR LANGUAGE[MAX_LANG_LENGTH];
typedef LONG  TIME_L;                  // new typedef to avoid conflicts with Lotus
typedef PUSHORT HUCB, *PHUCB;          // user control block handle
typedef PUCHAR  HDCB, *PHDCB;          // dictionary control block handle
typedef PVOID HLUPCB, *PHLUPCB;        // lookup control block handle


///////////////////////////////////////////////////////////////////
// Layout of column based list window filter                     //
///////////////////////////////////////////////////////////////////
typedef struct _CLBFILTER
{
  SHORT            asColumn[3];         // columns used in condition
  SHORT            asOperator[3];       // IDs of operator of condition
  CHAR             aszValue[3][40];     // values for conditions
  CHAR             szFiller[108];       // room for enhancements in the future
} CLBFILTER, *PCLBFILTER;

//--- common part of instance areas ---
typedef struct _IDA_HEAD
{
     PSZ          pszObjName;           // Object name ( old format )
     PVOID        hProp;                // Handle to object properties
     HWND         hFrame;               // Frame wnd handle
     EQF_BOOL     fMustNotClose;        // reject WM_TERMINATE etc. if TRUE
     OBJNAME      szObjName;            // Object name ( new format )
} IDA_HEAD, *PIDA_HEAD;




/* area for color and font description                                        */
typedef struct _COLFONT
{
   LONG lForeCol;                // foreground colour
   LONG lBackCol;                // background colour
   CHAR chFont[FONTNAMELENGTH];  // name of font
} COLFONT, *PCOLFONT;

/*structure of a profile entry for a dictionary entry field */
typedef struct _PROFENTRY
{
  CHAR chSystName[DICTENTRYLENGTH];    // entry field name allocated by system
  CHAR chUserName[DICTENTRYLENGTH];    // entry filed name renamed by user
  SHORT    sTokenId;                   // token id
  SHORT    sId;                        // indicator for user-created or system
                                       // field
  USHORT usLevel;                      // level to which dict entry is assigned
  USHORT usStatus;                     // authorization tag:
                                       // r/w, r/o, no access set by user
  USHORT usSystStatus;                 // authorization tag set by system
  USHORT usDisplay;                    // staggered display panel:
                                       // first, second or third
  USHORT usEntryFieldType;             // size of field, either mle or sle
  unsigned short fVital    :  1;             // TRUE if entry if vital
  unsigned short fSysVital :  1;             // system vital flag
  unsigned short fAutLookup:  1;             // display in automatic lookup flag
  unsigned short fReserved : 13;                 // TRUE if segment is current segment
} PROFENTRY, *PPROFENTRY;

/**********************************************************************/
/* Data structure for WM_EQF_PROCESSTASK value OPEN_AND_POSITION_TASK */
/**********************************************************************/
typedef struct _OPENANDPOS
{
  CHAR       szDocName[MAX_EQF_PATH];  // fully qualified document name
  ULONG      ulSeg;                    // number of segment being activated
  USHORT     usOffs;                   // offset of string within segment
  USHORT     usLen;                    // length of string
  CHAR_W     chFind[MAX_FINDCHANGE_LEN+1]; // buffer for find string
  ULONG      ulLine;                    // line in document to find (ulSeg has to be 0)
  BOOL       fSpellcheck;               // TRUE = start spellchecking after opening of documents
  PSZ        pszDocumentList;           // list of document object names to be spellchecked
} OPENANDPOS, *POPENANDPOS;

/**********************************************************************/
/* Prototypes for handler and instance window procedures              */
/**********************************************************************/
__declspec(dllexport) MRESULT APIENTRY DOCUMENTHANDLERWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY ANALYSISHANDLERWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY PROPERTYHANDLERWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY SERVICESHANDLERWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY LNGUPDATEHANDLERWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY FILTHANDLERWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport) MRESULT APIENTRY DDEHANDLERWP( HWND, WINMSG, WPARAM,LPARAM);
__declspec(dllexport) MRESULT APIENTRY GENERICHANDLERWP( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT APIENTRY GENERICLISTWP( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT APIENTRY GENERICPROCESSWP( HWND, WINMSG, WPARAM, LPARAM );

/**********************************************************************/
/* External referenced to the handles of preloaded icons declared in  */
/* EQFHNDLR.C (linked into EQFHNDLR.DLL)                              */
/**********************************************************************/
#ifdef NOEXTICONS
#elif defined(DLLIMPORTICONS)
__declspec(dllimport) HPOINTER hiconDICTIMP;          // dictionary import icon
__declspec(dllimport) HPOINTER hiconDICTEXP;          // dictionary export icon
__declspec(dllimport) HPOINTER hiconFOLIMP;           // folder import icon
__declspec(dllimport) HPOINTER hiconFOLEXP;           // folder export icon
__declspec(dllimport) HPOINTER hiconTWB;              // workbench icon
__declspec(dllimport) HPOINTER hiconFLL;              // folder list icon
__declspec(dllimport) HPOINTER hiconFOL;              // folder icon
__declspec(dllimport) HPOINTER hiconDOC;              // document icon
__declspec(dllimport) HPOINTER hiconANA;              // analysis icon
__declspec(dllimport) HPOINTER hiconMEM;              // memory list icon
__declspec(dllimport) HPOINTER hiconTAG;              // tagtable list icon
__declspec(dllimport) HPOINTER hiconCOUNT;            // count icon
__declspec(dllimport) HPOINTER hiconTMEMIMP;          // memory import icon
__declspec(dllimport) HPOINTER hiconTMEMEXP;          // memory export icon
__declspec(dllimport) HPOINTER hiconTMEMMERGE;        // memory merge icon
__declspec(dllimport) HPOINTER hiconTMEMORG;          // memory organize icon
__declspec(dllimport) HPOINTER hiconMARKUPIMP;        // tagtable import icon
__declspec(dllimport) HPOINTER hiconMARKUPEXP;        // tagtable export icon
__declspec(dllimport) HPOINTER hiconDICTORG;          // dictionary organize icon
__declspec(dllimport) HPOINTER hiconDICTLIST;         // dicionary list icon
__declspec(dllimport) HPOINTER hiconLIST;             // terminology list icon
__declspec(dllimport) HPOINTER hiconEXCLLIST;         // terminology list icon
__declspec(dllimport) HPOINTER hiconNEWLIST;          // terminology list icon
__declspec(dllimport) HPOINTER hiconFOUNDLIST;        // terminology list icon
__declspec(dllimport) HPOINTER hiconDICTPRINT;        // dictionary print icon
__declspec(dllimport) HPOINTER hiconDICTDISP;         // dictionary display entry icon
__declspec(dllimport) HPOINTER hiconTMM;              // translation memory maintenance icon
#else
extern HPOINTER hiconDICTIMP;          // dictionary import icon
extern HPOINTER hiconDICTEXP;          // dictionary export icon
extern HPOINTER hiconFOLIMP;           // folder import icon
extern HPOINTER hiconFOLEXP;           // folder export icon
extern HPOINTER hiconTWB;              // workbench icon
extern HPOINTER hiconFLL;              // folder list icon
extern HPOINTER hiconFOL;              // folder icon
extern HPOINTER hiconDOC;              // document icon
extern HPOINTER hiconANA;              // analysis icon
extern HPOINTER hiconMEM;              // memory list icon
extern HPOINTER hiconTAG;              // tagtable list icon
extern HPOINTER hiconCOUNT;            // count icon
extern HPOINTER hiconTMEMIMP;          // memory import icon
extern HPOINTER hiconTMEMEXP;          // memory export icon
extern HPOINTER hiconTMEMMERGE;        // memory merge icon
extern HPOINTER hiconTMEMORG;          // memory organize icon
extern HPOINTER hiconMARKUPIMP;        // tagtable import icon
extern HPOINTER hiconMARKUPEXP;        // tagtable export icon
extern HPOINTER hiconDICTORG;          // dictionary organize icon
extern HPOINTER hiconDICTLIST;         // dicionary list icon
extern HPOINTER hiconLIST;             // terminology list icon
extern HPOINTER hiconEXCLLIST;         // terminology list icon
extern HPOINTER hiconNEWLIST;          // terminology list icon
extern HPOINTER hiconFOUNDLIST;        // terminology list icon
extern HPOINTER hiconDICTPRINT;        // dictionary print icon
extern HPOINTER hiconDICTDISP;         // dictionary display entry icon
extern HPOINTER hiconTMM;              // translation memory maintenance icon
#endif

/**********************************************************************/
/* Macro interface to object manager functions                        */
/**********************************************************************/
#define EqfInstallHandler( psz, hwnd, cls)  InstallHandler( psz, hwnd, cls)
#define EqfRegisterObject( psz, hwnd, cls)  RegisterObject( psz, hwnd, cls)
#define EqfSend2AllHandlers( msg, mp1, mp2) Send2AllHandlers( msg, mp1, mp2)
#define EqfSend2AllObjects( cls, msg, mp1, mp2)                                \
                                           Send2AllObjects( cls, msg, mp1, mp2)
#define EqfQueryHandler( psz)        (HWND)WinSendMsg( EqfQueryObjectManager(),\
                                                WM_EQF_QUERYHANDLER,           \
                                                NULL, MP2FROMP(psz) )
#define EqfStopObjectManager( flag)  (BOOL)WinSendMsg( EqfQueryObjectManager(),\
                                                WM_EQF_TERMINATE,              \
                                                MP1FROMSHORT( flag ),          \
                                                MP2FROMP(NULL) )
#define EqfRemoveHandler( flg, psz)  (BOOL)WinSendMsg( EqfQueryObjectManager(),\
                                                WM_EQF_REMOVEHANDLER,          \
                                                MP1FROMSHORT( flg ),           \
                                                MP2FROMP(psz) )
#define EqfRemoveObject( flg, hwnd)  (BOOL)WinSendMsg( EqfQueryObjectManager(),\
                                                WM_EQF_REMOVEOBJECT,           \
                                                MP1FROMSHORT( flg ),           \
                                                MP2FROMHWND(hwnd) )
#define EqfChangeObjectName( hwnd, name )  (BOOL)WinSendMsg( EqfQueryObjectManager(),\
                                                WM_EQF_CHANGEOBJECTNAME,           \
                                                MP1FROMHWND( hwnd ),           \
                                                MP2FROMP(name) )

#define EqfQueryObjectClass( hwnd)                                             \
                                   (SHORT)WinSendMsg( EqfQueryObjectManager(), \
                                                WM_EQF_QUERYOBJECTCLASS,       \
                                                MP1FROMSHORT(0),               \
                                                MP2FROMHWND(hwnd) )

#define EqfQueryObjectStatus( hwnd)            \
  (USHORT)WinSendMsg( EqfQueryObjectManager(), \
                      WM_EQF_QUERYOBJECTSTATUS,\
                      MP1FROMSHORT(0),         \
                      MP2FROMHWND(hwnd))


#define EqfQueryObjectCount( cls) \
                                   (USHORT)WinSendMsg( EqfQueryObjectManager(),\
                                                WM_EQF_QUERYOBJECTCOUNT,       \
                                                MP1FROMSHORT( cls ),         \
                                                NULL)

#define EqfQueryHandlerCount()     (USHORT)WinSendMsg( EqfQueryObjectManager(),\
                                                WM_EQF_QUERYHANDLERCOUNT,      \
                                                NULL,                          \
                                                NULL)

#define EqfQueryActiveFolderlistHwnd()  \
  EqfQueryObject( NULL, clsFOLDERLIST, OBJ_ACTIVE | OBJ_BUSY )

#define EqfQueryActiveFolderHwnd()   \
  EqfQueryObject( NULL, clsFOLDER, OBJ_ACTIVE | OBJ_BUSY )

#define EqfQueryActiveDocumentHwnd() \
  EqfQueryObject( NULL, clsDOCUMENT, OBJ_ACTIVE | OBJ_BUSY )

#define EqfQueryActiveAnalysisHwnd() \
  EqfQueryObject( NULL, clsANALYSIS, OBJ_ACTIVE | OBJ_BUSY )

/**********************************************************************/
/* Macro interface to workbench code (EQFSTART)                       */
/**********************************************************************/
#define EqfActivateInstance( hwnd, flg)                                        \
                                   (VOID)WinSendMsg( EqfQueryHandler( TWBMAIN),\
                                                WM_EQF_ACTIVATEINSTANCE,       \
                                                MP1FROMSHORT( flg ),         \
                                                MP2FROMHWND(hwnd) )
#define EqfQueryMenuTbl()         (PVOID)WinSendMsg( EqfQueryHandler( TWBMAIN),\
                                                WM_EQF_QUERYMENUTBL,           \
                                                NULL, NULL)
#define EqfQueryTwbClient()          (HWND)EqfQueryHandler( TWBMAIN)
#define EqfQueryTwbFrame()  (HWND)WinQueryWindow( EqfQueryTwbClient(),\
                                                QW_PARENT, FALSE)

/**********************************************************************/
/* Macro interface to property handler                                */
/**********************************************************************/
#define EqfCallPropertyHandler( msg, mp1, mp2)                                 \
                          (HPROP)WinSendMsg( EqfQueryHandler( PROPERTYHANDLER),\
                                                msg, mp1, mp2)

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/*                          Macro Section                             */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

// Macro for DefWindowProc for Unicode and non Unicode
#define  DEFWINDOWPROC( hwnd, msg, mp1, mp2 )     \
    DefWindowProcW( hwnd, msg, mp1, mp2 )

#define  SENDMESSAGE( hwnd, msg, mp1, mp2 )       \
    SendMessageW( hwnd, msg, mp1, mp2)

// macro to get the index of the first selected item in a listbox
#define QUERYSELECTION( hwnd, id )                              \
     (SHORT) SendDlgItemMessage( (hwnd), (id), LB_GETCURSEL, 0, 0L )

// macro to get the index of the first selected item in a listbox
#define QUERYSELECTIONHWND( hwnd )                       \
     (SHORT) CPLUSPLUS SendMessage( (hwnd), LB_GETCURSEL, 0, 0L )

#define CBQUERYSELECTION( hwnd, id )                              \
     (SHORT) SendDlgItemMessage( (hwnd), (id), CB_GETCURSEL, 0, 0L )

// macro to get the index of the first selected item in a listbox
#define CBQUERYSELECTIONHWND( hwnd )                       \
     (SHORT) CPLUSPLUS SendMessage( (hwnd), CB_GETCURSEL, 0, 0L )

/**********************************************************************/
/* macro to select a listbox item                                     */
/* Attention:                                                         */
/* 1.) For Windows this is only true for SingleSelection boxes        */
/* 2.) Windows does not send a notification in such cases - we have   */
/*     to do it on our own...                                         */
/**********************************************************************/
#define SELECTITEM( hwnd, id, index )                                  \
  {                                                                      \
     SendDlgItemMessage( hwnd, id, LB_SETCURSEL, index, 0L );            \
     CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( id, LBN_SELCHANGE),        \
                  (LPARAM)GETHANDLEFROMID( hwnd, id) ); \
  }

#define SELECTITEMHWND( hwnd, index )                                  \
  {                                                                      \
     CPLUSPLUS SendMessage( hwnd, LB_SETCURSEL, index, 0L );            \
     CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( GetDlgCtrlID(hwnd), LBN_SELCHANGE), \
                  (LPARAM)hwnd ); \
  }

/**********************************************************************/
/* macro to select a combobox item                                     */
/* Attention:                                                         */
/* 1.) For Windows this is only true for SingleSelection boxes        */
/**********************************************************************/
#define CBSELECTITEM( hwnd, id, index ) \
  {                                                                      \
     SendDlgItemMessage( hwnd, id, CB_SETCURSEL, index, 0L );            \
     CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( id, CBN_SELCHANGE), \
                  (LPARAM)GETHANDLEFROMID( hwnd, id) ); \
  }

#define CBSELECTITEMHWND( hwnd, index ) \
  {                                                                      \
     CPLUSPLUS SendMessage( hwnd, CB_SETCURSEL, index, 0L );             \
     CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( GetDlgCtrlID(hwnd), CBN_SELCHANGE), \
                  (LPARAM)hwnd ); \
  }

/**********************************************************************/
/* macro to select a listbox item in a Multiple selection listbox     */
/* Attention:                                                         */
/* 1.) Windows does not send a notification in such cases - we have   */
/*     to do it on our own...                                         */
/**********************************************************************/
#define SELECTITEMMS( hwnd, id, index )                                \
  {                                                                      \
     SendDlgItemMessage( hwnd, id, LB_SETSEL, 1, MAKELONG(index, 0) );   \
     CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( id, LBN_SELCHANGE ),       \
                  (LPARAM)GETHANDLEFROMID( hwnd, id) ); \
  }

#define SELECTITEMMSHWND( hwnd,  index )                     \
  {                                                            \
     CPLUSPLUS SendMessage( hwnd, LB_SETSEL, 1, MAKELONG(index, 0) );    \
     CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( GetDlgCtrlID(hwnd), \
                  LBN_SELCHANGE ), (LPARAM)hwnd );            \
  }

/**********************************************************************/
/* macro to deselect a listbox item                                   */
/* Attention: For windows this macro could only be used for single    */
/*            selection listboxes (index will not be used)            */
/**********************************************************************/
#define DESELECTITEM( hwnd, id, index )                       \
     SendDlgItemMessage( hwnd, id, LB_SETCURSEL, (WPARAM) -1, 0L )
#define DESELECTITEMHWND( hwnd, index )                       \
     CPLUSPLUS SendMessage( hwnd, LB_SETCURSEL, (WPARAM) -1, 0L )

/**********************************************************************/
/* macro to deselect a combobox item                                  */
/**********************************************************************/
#define CBDESELECTITEM( hwnd, id, index )                       \
     SendDlgItemMessage( hwnd, id, CB_SETCURSEL, (WPARAM) -1, 0L )
#define CBDESELECTITEMHWND( hwnd, index )                       \
     CPLUSPLUS SendMessage( hwnd, CB_SETCURSEL, (WPARAM) -1, 0L )

/**********************************************************************/
/* macro to deselect a listbox item in a multiple selection listbox   */
/* Attention: For windows this macro MUST only be used for multiple   */
/*            selection listboxes                                     */
/**********************************************************************/
#define DESELECTITEMMS( hwnd, id, index )                       \
     CPLUSPLUS SendDlgItemMessage( hwnd, id, LB_SETSEL, 0, MAKELONG(index,0) )
#define DESELECTITEMMSHWND( hwnd, index )                       \
     CPLUSPLUS SendMessage( hwnd, LB_SETSEL, 0, MAKELONG( index, 0) )

// macro to insert a listbox items (sorted)
#define INSERTITEM( hwnd, id, text )                           \
     (SHORT) CPLUSPLUS SendDlgItemMessage( hwnd, id, LB_ADDSTRING, 0,      \
                                 (LPARAM) (LPCSTR) text )
#define INSERTITEMW( hwnd, id, text )                           \
 (SHORT) CPLUSPLUS SendDlgItemMessageW( hwnd, id, LB_ADDSTRING, 0, \
                       (LPARAM)(LPCSTR) text)

#define INSERTITEMHWND( hwnd, text )                           \
     (SHORT) CPLUSPLUS SendMessage( hwnd, LB_ADDSTRING, 0, (LPARAM) (LPCSTR) text )

// macro to check if the given handle is the handle of a combobox
#define ISCOMBOBOX( hwnd, flag ) \
  { CHAR cl[40]; GetClassName( hwnd, cl, 40 ); flag = _stricmp( cl, "COMBOBOX" ) == 0; }

// macro to insert a combobox item (sorted)
#define CBINSERTITEM( hwnd, id, text )                           \
     (SHORT) SendDlgItemMessage( hwnd, id, CB_ADDSTRING, 0,      \
                                 (LPARAM) (LPCSTR) text )
#define CBINSERTITEMW( hwnd, id, text )                           \
     (SHORT) SendDlgItemMessageW( hwnd, id, CB_ADDSTRING, 0,      \
                                 (LPARAM)  text )
#define CBINSERTITEMHWND( hwnd, text )                           \
     (SHORT) CPLUSPLUS SendMessage( hwnd, CB_ADDSTRING, 0, (LPARAM) (LPCSTR) text )

#define CBINSERTITEMHWNDW( hwnd, text )                           \
  (SHORT) CPLUSPLUS SendMessageW( hwnd, CB_ADDSTRING, 0, (LPARAM) text )


// macro to insert a listbox items (at END)
#define INSERTITEMEND( hwnd, id, text )                        \
     (SHORT) SendDlgItemMessage( hwnd, id, LB_INSERTSTRING, (WPARAM) -1,  \
                                 (LPARAM) (LPCSTR) text )
#define INSERTITEMENDW( hwnd, id, text )                        \
     (SHORT) SendDlgItemMessageW( hwnd, id, LB_INSERTSTRING, (WPARAM) -1,  \
                                 (LPARAM)text )

// macro to insert a listbox items (at END)
#define CBINSERTITEMEND( hwnd, id, text )                        \
     (SHORT) SendDlgItemMessage( hwnd, id, CB_INSERTSTRING, (WPARAM) -1,  \
                                 (LPARAM) (LPCSTR) text )
#define CBINSERTITEMENDW( hwnd, id, text )                        \
    (SHORT) SendDlgItemMessageW( hwnd, id, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) text )

#define INSERTITEMENDHWND( hwnd, text )                        \
     (SHORT) CPLUSPLUS SendMessage( hwnd, LB_INSERTSTRING, (WPARAM) -1,    \
                          (LPARAM) (LPCSTR) text )
#define INSERTITEMENDHWNDW( hwnd, text )                                 \
     (SHORT) CPLUSPLUS SendMessageW( hwnd, LB_INSERTSTRING, (WPARAM) -1, (LPARAM)  text )

#define INSERTITEMHWNDW( hwnd, index, text )                                 \
          (SHORT) CPLUSPLUS SendMessageW( hwnd, LB_INSERTSTRING, (WPARAM) index, (LPARAM)  text )


// macro to delete a listbox item
#define DELETEITEM( hwnd, id, index )                           \
    SendDlgItemMessage( hwnd, id, LB_DELETESTRING, index, 0L )
#define DELETEITEMW( hwnd, id, index )                           \
    SendDlgItemMessageW( hwnd, id, LB_DELETESTRING, index, 0L )

#define DELETEITEMHWND( hwnd, index )                           \
    CPLUSPLUS SendMessage( hwnd, LB_DELETESTRING, index, 0L )
#define DELETEITEMHWNDW( hwnd, index )                           \
    CPLUSPLUS SendMessageW( hwnd, LB_DELETESTRING, index, 0L )

/**********************************************************************/
/* macro to extract the text of a listbox item                        */
/* Attention: Under Windows there is NO checking for the size of the  */
/*            provided buffer - it is assumed that the buffer is large*/
/*            enough.                                                 */
/**********************************************************************/
#define QUERYITEMTEXT( hwnd, id, index, buffer )                  \
    SendDlgItemMessage( hwnd, id, LB_GETTEXT, index, (LPARAM) (LPCSTR) buffer )

#define QUERYITEMTEXTW( hwnd, id, index, buffer )                   \
  SendDlgItemMessageW( hwnd, id, LB_GETTEXT,  index, (LPARAM) buffer )
#define QUERYITEMTEXTL( hwnd, id, index, buffer, len )                  \
    SendDlgItemMessage( hwnd, id, LB_GETTEXT, index, (LPARAM) (LPCSTR) buffer )

#define QUERYITEMTEXTHWND( hwnd, index, buffer )                  \
    CPLUSPLUS SendMessage( hwnd, LB_GETTEXT, index, (LPARAM) (LPCSTR) buffer )
#define QUERYITEMTEXTHWNDL( hwnd, index, buffer, len )                  \
    CPLUSPLUS SendMessage( hwnd, LB_GETTEXT, index, (LPARAM) (LPCSTR) buffer )


  #define CBQUERYITEMTEXT( hwnd, id, index, buffer )                  \
    SendDlgItemMessage( hwnd, id, CB_GETLBTEXT, index, (LPARAM) (LPCSTR) buffer )
  #define CBQUERYITEMTEXTW( hwnd, id, index, buffer )                  \
    SendDlgItemMessageW( hwnd, id, CB_GETLBTEXT, index, (LPARAM) buffer )

  #define CBQUERYITEMTEXTHWND( hwnd, index, buffer )                  \
    CPLUSPLUS SendMessageW( hwnd, CB_GETLBTEXT, index, (LPARAM) (LPCSTR) buffer )


  #define CBQUERYITEMTEXTHWNDW( hwnd, index, buffer )                  \
    CPLUSPLUS SendMessageW( hwnd, CB_GETLBTEXT, index, (LPARAM) buffer )

// macro to extract the handle of a listbox item
#define QUERYITEMHANDLE( hwnd, id, index )                        \
      (ULONG) SendDlgItemMessage( hwnd, id, LB_GETITEMDATA, index, 0L )
#define QUERYITEMHANDLEHWND( hwnd, index )                        \
      (ULONG) CPLUSPLUS SendMessage( hwnd, LB_GETITEMDATA, index, 0L )

// macro to extract the handle of a listbox item
#define CBQUERYITEMHANDLE( hwnd, id, index )                        \
      (ULONG) SendDlgItemMessage( hwnd, id, CB_GETITEMDATA, index, 0L )

// macro to extract the handle of a listbox item
#define CBQUERYITEMHANDLE( hwnd, id, index )                        \
      (ULONG) SendDlgItemMessage( hwnd, id, CB_GETITEMDATA, index, 0L )

// macro to generate a drive character from a drive control ID
#define DRIVEFROMID( startid, id )                              \
   (CHAR) ( 'A' + (CHAR) ( (id) - (startid) ) )

// macro to generate a drive control ID from a drive character
#define IDFROMDRIVE( startid, drive )                           \
   ( (SHORT) (drive - 'A') + startid )

// macro to set the hilite state of a drive icon
#define SETDRIVE( hwnd, id, flag )                           \
    SendDlgItemMessage( hwnd, id, WM_DRIVEBUTTON_SELECT, flag, 0L )

// macro to set the check state of a checkbox
  #define SETCHECK( hwnd, id, flag )                       \
  {                                                                  \
     SendDlgItemMessage( hwnd, id, BM_SETCHECK, flag, 0L );          \
     if ( flag ) CPLUSPLUS SendMessage(hwnd, WM_COMMAND, MAKELONG( id, BN_CLICKED), 0 );   \
  }
  #define SETCHECK_TRUE( hwnd, id )                       \
  {                                                                  \
       SendDlgItemMessage( hwnd, id, BM_SETCHECK, TRUE, 0L );          \
       CPLUSPLUS SendMessage(hwnd, WM_COMMAND, MAKELONG( id, BN_CLICKED), 0 );   \
  }
  #define SETCHECK_FALSE( hwnd, id )                       \
  {                                                                  \
       SendDlgItemMessage( hwnd, id, BM_SETCHECK, FALSE, 0L );          \
  }

#define SETCHECKHWND( hwnd, flag )                       \
  {                                                        \
     CPLUSPLUS SendMessage( hwnd, BM_SETCHECK, flag, 0L );           \
  }

// macro to query the check state of a checkbox
#define QUERYCHECK( hwnd, id )                       \
     (USHORT) SendDlgItemMessage( hwnd, id, BM_GETCHECK, 0, 0L )
#define QUERYCHECKHWND( hwnd )                       \
     (USHORT) CPLUSPLUS SendMessage( hwnd, BM_GETCHECK, 0, 0L )

// macro to delete all items of a listbox
#define DELETEALL( hwnd, id )                       \
     SendDlgItemMessage( hwnd, id, LB_RESETCONTENT, 0, 0L )

#define DELETEALLHWND( hwnd )                       \
     CPLUSPLUS SendMessage( hwnd, LB_RESETCONTENT, 0, 0L )

// macro to delete all items of a listbox
#define CBDELETEALL( hwnd, id )                       \
     SendDlgItemMessage( hwnd, id, CB_RESETCONTENT, 0, 0L )
#define CBDELETEALLHWND( hwnd )                       \
     CPLUSPLUS SendMessage( hwnd, CB_RESETCONTENT, 0, 0L )

// macro to query the number of items in a listbox control
#define QUERYITEMCOUNT( hwnd, id )                       \
     (SHORT) SendDlgItemMessage( hwnd, id, LB_GETCOUNT, 0, 0L )
#define QUERYITEMCOUNTHWND( hwnd )                       \
     (SHORT) CPLUSPLUS SendMessage( hwnd, LB_GETCOUNT, 0, 0L )

// macro to query the number of items in a combobox control
#define CBQUERYITEMCOUNT( hwnd, id )                       \
     (SHORT) SendDlgItemMessage( hwnd, id, CB_GETCOUNT, 0, 0L )
#define CBQUERYITEMCOUNTHWND( hwnd )                       \
     (SHORT) CPLUSPLUS SendMessage( hwnd, CB_GETCOUNT, 0, 0L )

// macro to query the next selected item of a listbox control
#define QUERYNEXTSELECTION( hwnd, id, pos )                     \
     GetNextSelection( GetDlgItem( hwnd, id), pos )
#define QUERYNEXTSELECTIONHWND( hwnd, pos )                     \
     GetNextSelection( hwnd, pos )

// macro to deselect all listbox items
#define DESELECTALL( hwnd, id )  \
    if ( (SHORT)SendDlgItemMessage( hwnd, id, LB_SETSEL, FALSE, (LPARAM)-1L ) == LB_ERR ) \
      SendDlgItemMessage( hwnd, id, LB_SETCURSEL, (WPARAM)-1, 0L )

#define DESELECTALLHWND( hwnd )  \
    if ( CPLUSPLUS SendMessage( hwnd, LB_SETSEL, FALSE, (LPARAM)-1L ) == LB_ERR ) \
      CPLUSPLUS SendMessage( hwnd, LB_SETCURSEL, (WPARAM)-1, 0L )

// macro to select all listbox items
#define SELECTALL( hwnd, id )  \
    SendDlgItemMessage( hwnd, id, LB_SETSEL, TRUE, (LPARAM)-1L )
#define SELECTALLHWND( hwnd )  \
    CPLUSPLUS SendMessage( hwnd, LB_SETSEL, TRUE, (LPARAM)-1L )

// macro to set the textlimit of an entry field
#define SETTEXTLIMIT( hwnd, id, limit )                         \
     SendDlgItemMessage( hwnd, id, EM_LIMITTEXT, limit, 0L)
#define SETTEXTLIMITHWND( hwnd, limit )                         \
     CPLUSPLUS SendMessage( hwnd, EM_LIMITTEXT, limit, 0L)

// macro to set the textlimit of an combobox entry field
#define CBSETTEXTLIMIT( hwnd, id, limit )                         \
     SendDlgItemMessage( hwnd, id, CB_LIMITTEXT, limit, 0L)
#define CBSETTEXTLIMITHWND( hwnd, limit )                         \
     CPLUSPLUS SendMessage( hwnd, CB_LIMITTEXT, limit, 0L)

// macro to get text from a dialog control
#define QUERYTEXT( hwnd, id, buffer ) \
    GetDlgItemText( hwnd, id, buffer, sizeof(buffer) )
#define QUERYTEXTW( hwnd, id, buffer ) \
    GetDlgItemTextW( hwnd, id, buffer, sizeof(buffer)/sizeof(CHAR_W))
#define QUERYTEXTHWND( hwnd, buffer ) \
    GetWindowText( hwnd, buffer, sizeof(buffer) )
  // the specified length is in CHAR_W
#define QUERYTEXTHWNDW( hwnd, buffer, usLen ) \
    GetWindowTextW( hwnd, buffer, usLen)

#define QUERYTEXTHWNDLEN( hwnd, buffer, len ) \
    GetWindowText( hwnd, buffer, len )

// macro to get text from a dialog control and convert text to OEM characterset
#define OEMQUERYTEXT( hwnd, id, buffer ) \
    { GetDlgItemText( hwnd, id, buffer, sizeof(buffer) ); EQFAnsiToOem( buffer, buffer ); }
#define OEMQUERYTEXTHWND( hwnd, buffer ) \
    { GetWindowText( hwnd, buffer, sizeof(buffer) ); EQFAnsiToOem( buffer, buffer ); }

// macro to convert the characterset from ANSI to OEM
#define OEMTOANSI( buffer ) \
    EQFOemToAnsi( buffer, buffer )

// macro to convert the characterset from OEM to ANSI
#define ANSITOOEM( buffer ) \
    EQFAnsiToOem( buffer, buffer )

// macro for conditional conversion the characterset from ANSI to OEM

#define CONDOEMTOANSI( flag, buffer ) \
    { \
      if( flag) \
      { \
         EQFOemToAnsi(buffer, buffer);  \
      } \
  }

// macro to convert the characterset from OEM to ANSI
#define CONDANSITOOEM( flag, buffer ) \
  { \
     if( flag )\
     { \
       EQFAnsiToOem( buffer, buffer ); \
     } \
  }

// Post a WM_CLOSE message to close window
#define POSTCLOSE( hwnd, flag ) \
    CPLUSPLUS PostMessage( hwnd, WM_CLOSE, flag, 0L )

/**********************************************************************/
/* macro to get text from a window                                    */
/**********************************************************************/
#define QUERYWINDOWTEXT( hwnd, buffer ) \
    GetWindowText( hwnd, buffer, sizeof(buffer) )

// macro to set text of a dialog control
#define SETTEXT( hwnd, id, text ) \
    SetDlgItemText( hwnd, id,(LPCTSTR) text )
#define SETTEXTW( hwnd, id, text ) \
   SetDlgItemTextW( hwnd, id, text )
#define SETTEXTHWND( hwnd, text ) \
    SetWindowText( hwnd, text )
#define SETTEXTHWNDW( hwnd, text ) \
    SetWindowTextW( hwnd, text )

// macro to convert text to ansi and set text of a dialog control
#define OEMSETTEXT( hwnd, id, text ) \
    { EQFOemToAnsi( text, text ); SetDlgItemText( hwnd, id, text ); }
#define OEMSETTEXTHWND( hwnd, text ) \
    { EQFOemToAnsi( text, text ); SetWindowText( hwnd, text ); }

/**********************************************************************/
/* set text limit in MLE fields                                       */
/**********************************************************************/
#define MLESETTEXTLIMIT( hwnd, id, limit )                         \
     SendDlgItemMessage( hwnd, id, EM_LIMITTEXT, limit, 0L)

/**********************************************************************/
/* set text data in MLE                                               */
/**********************************************************************/
#define MLESETTEXT( hwnd, id, text ) \
    SetDlgItemText( hwnd, id, text )
  #define MLESETTEXTW( hwnd, id, text ) \
    SetDlgItemTextW( hwnd, id, text )
/**********************************************************************/
/* get text data out of MLE                                           */
/* ATTENTION:                                                         */
/*  it is assumed that BUFFER is the address of the buffer used for   */
/*  all the other calls.....                                          */
/*  This macro will ALWAYS return all the data ....                   */
/**********************************************************************/
#define MLEGETTEXT( hwnd, id, buffer, usRc ) \
    usRc = GetDlgItemText( hwnd, id, buffer, sizeof(buffer) )
#define MLEGETTEXTW( hwnd, id, buffer, usRc ) \
    usRc = (LONG)GetDlgItemTextW( hwnd, id, buffer, sizeof(buffer))
/**********************************************************************/
/* macro to set Entryfield data to be selected....                    */
/**********************************************************************/
#define SETEFSEL( hwnd, id, start, end ) \
    SendDlgItemMessage( hwnd, id, EM_SETSEL, 0, MAKELPARAM( start, end ))

#define SEARCHSELECT( sItem, hwnd, usID, pszItem ) \
  { \
    sItem = (SHORT)SendDlgItemMessage( hwnd, usID, LB_FINDSTRINGEXACT, 0, (LPARAM) pszItem ); \
    if( sItem != LIT_NONE) \
    { \
       SendDlgItemMessage( hwnd, usID, LB_SETCURSEL, sItem, 0L );   \
       CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( usID, LBN_SELCHANGE ), \
          (LPARAM)hwnd ); \
    } \
  }

#define SEARCHSELECTHWND( sItem, hwndLB, pszItem ) \
  { \
    sItem = (SHORT)CPLUSPLUS SendMessage( hwndLB, LB_FINDSTRINGEXACT, 0, (LPARAM) pszItem ); \
    if( sItem != LIT_NONE) \
    { \
       CPLUSPLUS SendMessage( hwndLB, LB_SETCURSEL, sItem, 0L );   \
       CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( GetDlgCtrlID(hwnd), LBN_SELCHANGE), \
                    (LPARAM)hwnd );            \
    } \
  }

#define CBSEARCHSELECT( sItem, hwnd, usID, pszItem ) \
  { \
    sItem = (SHORT)SendDlgItemMessage( hwnd, usID, CB_FINDSTRINGEXACT, 0, (LPARAM) pszItem ); \
    if( sItem != LIT_NONE) \
    { \
       SendDlgItemMessage( hwnd, usID, CB_SETCURSEL, sItem, 0L );   \
       CPLUSPLUS PostMessage( hwnd, WM_COMMAND, MAKELONG( usID,CBN_SELCHANGE), \
          (LPARAM)hwnd ); \
    } \
  }


/**********************************************************************/
/* check if the specified string is part of the listbox               */
/**********************************************************************/
#define SEARCHITEM( hwnd, usID, pszItem )          \
            (SHORT)SendDlgItemMessage( hwnd, usID, LB_FINDSTRINGEXACT, \
                                   0, (LPARAM) pszItem )
// Add for R012027 start
#define SEARCHITEMW( hwnd, usID, pszItem )          \
            (SHORT)SendDlgItemMessageW( hwnd, usID, LB_FINDSTRINGEXACT, \
                                   0, (LPARAM) pszItem )
// Add end
#define SEARCHITEMHWND( hwnd, pszItem )          \
             (SHORT)CPLUSPLUS SendMessage( hwnd, LB_FINDSTRINGEXACT, \
                                   0, (LPARAM) pszItem )
#define SEARCHITEMHWNDW( hwnd, pszItem )          \
             (SHORT)CPLUSPLUS SendMessageW( hwnd, LB_FINDSTRINGEXACT, \
                                   0, (LPARAM) pszItem )

/**********************************************************************/
/* check if the specified string is part of the listbox (case         */
/* insensitive                                                        */
/**********************************************************************/
#define ISEARCHITEM( hwnd, usID, pszItem )          \
            (SHORT)SendDlgItemMessage( hwnd, usID, LB_FINDSTRING, \
                                   0, (LPARAM) pszItem )

#define ISEARCHITEMHWND( hwnd, pszItem )          \
             (SHORT)CPLUSPLUS SendMessage( hwnd, LB_FINDSTRING, \
                                   0, (LPARAM) pszItem )
// Especially for invisible listbox concerning DDE based counting report
#define ISEARCHITEMEXACTHWND( hwnd, pszItem )          \
             (SHORT)CPLUSPLUS SendMessage( hwnd, LB_FINDSTRINGEXACT, \
                                   (WPARAM) -1, (LPARAM) pszItem )

  #define CBSEARCHITEM( hwnd, usID, pszItem )          \
            (SHORT)SendDlgItemMessage( hwnd, usID, CB_FINDSTRINGEXACT, \
                                   0, (LPARAM) pszItem )
  #define CBSEARCHITEMW( hwnd, usID, pszItem )          \
     (SHORT)SendDlgItemMessageW( hwnd, usID, CB_FINDSTRINGEXACT, 0, (LPARAM) pszItem )
  #define CBSEARCHITEMPOSW( hwnd, sPos, pszItem )          \
     (SHORT)SendMessageW( hwnd, CB_FINDSTRINGEXACT, sPos, (LPARAM) pszItem )
  #define CBSEARCHITEMHWND( hwnd, pszItem )          \
             (SHORT)CPLUSPLUS SendMessage( hwnd, CB_FINDSTRINGEXACT, \
                                   0, (LPARAM) pszItem )
  #define CBSEARCHITEMHWNDW( hwnd, pszItem )          \
             (SHORT)CPLUSPLUS SendMessageW( hwnd, CB_FINDSTRINGEXACT, \
                                   0, (LPARAM) pszItem )

  #define SEARCHITEMPREFIXHWND( hwnd, pszItem )          \
             (SHORT)CPLUSPLUS SendMessage( hwnd, LB_FINDSTRING, \
                                   0, (LPARAM) pszItem )
  #define SEARCHITEMPREFIXHWNDW( hwnd, pszItem )          \
             (SHORT)CPLUSPLUS SendMessageW( hwnd, LB_FINDSTRING, \
                                   0, (LPARAM) pszItem )

/**********************************************************************/
/* set the top selection of a listbox                                 */
/**********************************************************************/
#define SETTOPINDEX( hwnd, usID, item )          \
            (SHORT)SendDlgItemMessage( hwnd, usID, LB_SETTOPINDEX, \
                                   (WPARAM)item, 0L )
#define SETTOPINDEXHWND( hwnd, item )          \
            (SHORT)CPLUSPLUS SendMessage( hwnd, LB_SETTOPINDEX, \
            (WPARAM)item, 0L )
#define CBSETTOPINDEX( hwnd, usID, item )    {  }
#define CBSETTOPINDEXHWND( hwnd, item )    { }

// macro to enable or disable a dialog control
#define ENABLECTRL( hwnd, id, flag ) \
     EnableWindow( GetDlgItem( hwnd, id ), flag )
#define ENABLECTRLHWND( hwnd, flag ) \
     EnableWindow( hwnd, flag )

// macro to set window update state of a control
#define ENABLEUPDATE( hwnd, id, flag ) \
  { \
    SendDlgItemMessage( hwnd, id , WM_SETREDRAW, flag, 0L ); \
    if ( flag ) InvalidateRect( GetDlgItem( hwnd, id ), NULL, FALSE ); \
  }
  #define ENABLEUPDATE_TRUE( hwnd, id ) \
  { \
      SendDlgItemMessage( hwnd, id , WM_SETREDRAW, TRUE, 0L ); \
      InvalidateRect( GetDlgItem( hwnd, id ), NULL, FALSE ); \
  }
  #define ENABLEUPDATE_FALSE( hwnd, id ) \
  { \
    SendDlgItemMessage( hwnd, id , WM_SETREDRAW, FALSE, 0L ); \
  }

#define ENABLEUPDATEHWND( hwnd, flag )                   \
  { \
    CPLUSPLUS SendMessage( hwnd, WM_SETREDRAW, flag, 0L ); \
    if ( flag ) InvalidateRect( hwnd, NULL, FALSE ); \
  }
  #define ENABLEUPDATEHWND_TRUE( hwnd )                   \
    { \
      CPLUSPLUS SendMessage( hwnd, WM_SETREDRAW, TRUE, 0L ); \
      InvalidateRect( hwnd, NULL, FALSE ); \
  }
  #define ENABLEUPDATEHWND_FALSE( hwnd )                   \
    { \
      CPLUSPLUS SendMessage( hwnd, WM_SETREDRAW, FALSE, 0L ); \
    }

/**********************************************************************/
/* Add a new object to a restart list                                 */
/**********************************************************************/
#define ADDTOLIST( list, obj ) \
  if( sizeof(list)-strlen(list)-2 > strlen( obj) )\
    { strcat( list, X15_STR ); strcat ( list, obj ); }

// macro to set AAB item state Enabled/Disabled
#define SETAABITEM( hwnd, id, fState )                       \
          EnableMenuItem (hwnd, id,                            \
                          fState ? MF_ENABLED : MF_GRAYED)

// macro to set AAB item text
#define SETAABITEMTEXT( hwnd, id, text )                       \
          ModifyMenu(hwnd, id, MF_STRING | MF_BYCOMMAND, id, (LPSTR)text )

// macro to get menu handle of a window
#define GETMENU( hwnd ) \
      GetMenu( hwnd )

// macro to set AAB item state Checked/Unchecked
#define SETAABITEMCHECK( hwnd, id, fState )                  \
          CheckMenuItem( hwnd, id,                             \
                         fState ? MF_CHECKED : MF_UNCHECKED)
#define GETMENUTEXT( hwndMenu, sItem, szBuffer ) \
       GetMenuString( hwndMenu, sItem, szBuffer,sizeof(szBuffer), MF_BYCOMMAND )

/**********************************************************************/
/* update the menu text                                               */
/**********************************************************************/
#define SETMENUTEXT( hwndMenu, sItem, szBuffer ) \
     ModifyMenu( hwndMenu, sItem, MF_BYCOMMAND, sItem, szBuffer )
#define QUERYSYMBOL(symbol) \
  ObjQuerySymbol( symbol )

#define SETSYMBOL(symbol) \
  ObjSetSymbol( symbol )

// show the specified control
#define SHOWCONTROL( hwnd, id ) \
    ShowWindow( GetDlgItem( hwnd, id ), SW_SHOWNORMAL )

// hide the specified control
#define HIDECONTROL( hwnd, id ) \
    ShowWindow( GetDlgItem( hwnd, id ), SW_HIDE )

// Set the focus on the specified control
#define SETFOCUS( hwnd, id ) \
    SetFocus( GetDlgItem( hwnd, id ) )
#define SETFOCUSHWND( hwnd ) \
    SetFocus( hwnd )

// get the handle of the control with the focus
#define GETFOCUS()           GetFocus()

/**********************************************************************/
/* Lock the window update                                             */
/**********************************************************************/
#define LOCKWINDOWUPDATE( hwnd ) \
    LockWindowUpdate( hwnd )

#define DELETESELECTEDLBITEM( hwndLb )                               \
     CPLUSPLUS SendMessage( hwndLb, LB_DELETESTRING, QUERYSELECTIONHWND( hwndLb ), 0L )

/* Macro to query changed flag of entry fields                        */
#define QUERYCHANGED( hwnd, id ) \
    (BOOL)SendDlgItemMessage( hwnd, id, EM_GETMODIFY, 0, 0L )

// macro to click a button
#define CLICK( hwnd, id )                       \
    {                                                      \
     SendDlgItemMessage( hwnd, id, BM_SETCHECK, 1, 0L );   \
     SetFocus( GetDlgItem( hwnd, id ) );                   \
     CPLUSPLUS SendMessage( hwnd, WM_COMMAND, MAKELONG( id, BN_CLICKED ), 0 ); \
    }

// SETTEXTFROMRES:  do a WinSetWindowText from a resource string table string
#define SETTEXTFROMRES( hwnd, id, buffer, hmod, sid )              \
    if ( LoadString( hmod, sid, buffer, sizeof(buffer) ) ) \
       SetDlgItemText( hwnd, id, buffer ); else SetDlgItemText( hwnd, id, "" )
// Load a string from a resource
#define LOADSTRING( hInst, hmod, sid, buffer)   \
    LoadString( hmod, sid, buffer, sizeof(buffer) )

// Load a string from a resource with buffer length specified
#define LOADSTRINGLEN( hInst, hmod, sid, buffer, len )   \
    LoadString( hmod, sid, buffer, len )

/**********************************************************************/
/* macro to 'anchor' IDA in window structure                          */
/* ATTENTION: keep in mind the difference between WinSetWindowULong   */
/*            and WinSetWindowPtr                                     */
/**********************************************************************/
  /********************************************************************/
  /* SetWindowLong returns the value of the last long set and 0L      */
  /* in case of error - for the first call it's the same, therefore   */
  /* try again to be sure....                                         */
  /********************************************************************/
  #define ANCHORDLGIDA( hwnd, pIda ) \
    (BOOL)((SetWindowLong( hwnd, DWL_USER, (LONG)((PVOID)pIda) )) ?            \
                TRUE : SetWindowLong( hwnd, DWL_USER, (LONG)((PVOID) pIda)))

  /********************************************************************/
  /* SetWindowLong returns the value of the last long set and 0L      */
  /* in case of error - for the first call it's the same, therefore   */
  /* try again to be sure....                                         */
  /********************************************************************/
  #define ANCHORWNDIDA( hwnd, pIda ) \
   ((SetWindowLong( hwnd, GWL_USERDATA /*0*/, (LONG)((PVOID) pIda) )) ?                   \
                    1 : SetWindowLong( hwnd, GWL_USERDATA /*0*/, (LONG)((PVOID) pIda) ) != 0L )

/**********************************************************************/
/* Set the window word                                                */
/**********************************************************************/
#define QWS_ID      GWW_ID
#define SETWINDOWWORD( hwnd, id, usVal )  \
      SetWindowWord( hwnd, id, usVal )

/**********************************************************************/
/* get parent from specified handle                                   */
/**********************************************************************/
#define GETPARENT( hwnd )                   \
     CPLUSPLUS GetParent( hwnd )

/**********************************************************************/
/* get owner from specified handle                                    */
/**********************************************************************/
#define GETOWNER( hwnd )                   \
     GetWindow( hwnd, GW_OWNER )

/**********************************************************************/
/* get active window                                                  */
/**********************************************************************/
#define QUERYACTIVEWINDOW() GetActiveWindow()

/**********************************************************************/
/* Get instance (anchor block under PM)                               */
/**********************************************************************/
#define GETINSTANCE( hwnd )               \
    (HINSTANCE) GetWindowLong( hwnd, GWL_HINSTANCE )

/**********************************************************************/
/* macro to retrieve store IDA pointer from window structure          */
/**********************************************************************/
#define ACCESSDLGIDA( hwnd, type ) \
    (type)GetWindowLong( hwnd, DWL_USER)

#define ACCESSWNDIDA( hwnd, type ) \
    (type)GetWindowLong( hwnd, GWL_USERDATA /*0*/ )

#define DISMISSDLG( hwnd, value ) \
    EndDialog( hwnd, value )

#define COMMANDID( mp1, mp2 ) \
  SHORT1FROMMP1(mp1)

/**********************************************************************/
/* set color for dialog item                                          */
/* ATTENTION: Currently only SYSCLR_WINDOWTEXT supported              */
/**********************************************************************/
/**********************************************************************/
/* ???????  instead of WinSetPresParms we might have to process a     */
/* WM_CTLCOLOR message for the static fields...                       */
/**********************************************************************/
#define SETCOLOR( hwnd, id, color )
//  the following line is not true ...
//    SetTextColor( GetDC(GETHANDLEFROMID(hwnd,id)), GetSysColor(color))

#define LONGTODATETIME( ltime, buffer ) \
{ \
  UtlLongToDateString( ltime, buffer, sizeof(buffer) ); \
  strcat( buffer, " " );  \
  UtlLongToTimeString( ltime, buffer + strlen(buffer), (sizeof(buffer) - strlen(buffer)) ); \
}

#define LONGTODATETIMELEN( ltime, buffer, len ) \
{ ULONG  l = len; PSZ p = buffer; \
  UtlLongToDateString( ltime, p, l ); \
  strcat( p, " " );  l -= strlen(p); \
  UtlLongToTimeString( ltime, p + strlen(p), l ); \
}

/* Macroes to query the change flags of MLEs and SLEs                 */
  #define MLECHANGED( hwnd, id  ) (BOOL) SendDlgItemMessage( hwnd, id , EM_GETMODIFY, 0, 0L )
  #define SLECHANGED( hwnd, id  ) (BOOL) SendDlgItemMessage( hwnd, id , EM_GETMODIFY, 0, 0L )
  #define MLECHANGEDHWND( hwnd ) (BOOL) CPLUSPLUS SendMessage( hwnd, EM_GETMODIFY, 0, 0L )
  #define SLECHANGEDHWND( hwnd ) (BOOL) CPLUSPLUS SendMessage( hwnd, EM_GETMODIFY, 0, 0L )

/**********************************************************************/
/* Get window handle from ID                                          */
/**********************************************************************/
#define GETHANDLEFROMID( hwnd, id )  \
    GetDlgItem( hwnd, id )
/**********************************************************************/
/* set text for specified item                                        */
/* Attention: Under windows you have to delete and insert the text AND*/
/*            temp. store any IDA handle (this is done in function    */
/*            LBSetItemText which is located in OS2TOWIN.C            */
/**********************************************************************/
#define SETITEMTEXT( hwnd, id, index, pText ) \
    LBSetItemText( GetDlgItem( hwnd, id), index, pText )

#define SETITEMTEXTW( hwnd, id, index, pText ) \
     LBSetItemTextW( GetDlgItem( hwnd, id), index, pText )

#define SETITEMTEXTHWND( hwnd, index, pText )                          \
    LBSetItemText( hwnd, index, pText )

/**********************************************************************/
/* anchor pointer in listbox item handle                              */
/**********************************************************************/
#define SETITEMHANDLE( hwnd, id, index, handle ) \
     SendDlgItemMessage( hwnd, id, LB_SETITEMDATA, index, (ULONG) handle)
// Add for R012027 start
#define SETITEMHANDLEW( hwnd, id, index, handle ) \
     SendDlgItemMessageW( hwnd, id, LB_SETITEMDATA, index, (ULONG) handle)
// Add end
#define SETITEMHANDLEHWND( hwnd, index, handle ) \
     CPLUSPLUS SendMessage( hwnd, LB_SETITEMDATA, index, (ULONG) handle)

#define CBSETITEMHANDLE( hwnd, id, index, handle ) \
     SendDlgItemMessage( hwnd, id, CB_SETITEMDATA, index, (ULONG) handle)

#define CBSETITEMHANDLEHWND( hwnd, index, handle ) \
     CPLUSPLUS SendMessage( hwnd, CB_SETITEMDATA, index, (ULONG) handle)

/**********************************************************************/
/* retrieve pointer from listbox item handle                          */
/**********************************************************************/
#define GETITEMHANDLE( hwnd, id, index, type ) \
     (type) SendDlgItemMessage( hwnd, id, LB_GETITEMDATA, index, 0L)

#define GETITEMHANDLEHWND( hwnd, index, type ) \
     (type) CPLUSPLUS SendMessage( hwnd, LB_GETITEMDATA, index, 0L)

#define CBGETITEMHANDLE( hwnd, id, index, type ) \
     (type) SendDlgItemMessage( hwnd, id, CB_GETITEMDATA, index, 0L)

#define CBGETITEMHANDLEHWND( hwnd, index, type ) \
     (type) CPLUSPLUS SendMessage( hwnd, CB_GETITEMDATA, index, 0L)

/**********************************************************************/
/* modal dialog                                                       */
/**********************************************************************/
  #define DIALOGBOX( hwnd, proc, hresource, id, param, rc )    \
    rc = DialogBoxParam( hresource, MAKEINTRESOURCE( id ), hwnd, proc, (LPARAM)param );
  #define DIALOGBOXW( hwnd, proc, hresource, id, param, rc )    \
    rc = DialogBoxParamW( hresource, (PSZ_W)MAKEINTRESOURCE( id ), hwnd, proc, (LPARAM)param );

/**********************************************************************/
/* get system bitmaps                                                 */
/* Attention: Under Windows we've only defined the Checkmark...       */
/**********************************************************************/
  #define SBMP_MENUCHECK    OBM_CHECK

  #define GETSYSBITMAP( name )             \
     LoadBitmap( NULL, MAKEINTRESOURCE(name) )

  #define FREESYSBITMAP( hbm )             \
     DeleteObject( hbm )
/**********************************************************************/
/* set mouse pointer                                                  */
/**********************************************************************/
  #define SPTR_ARROW             IDC_ARROW
  #define SPTR_TEXT              IDC_IBEAM
  #define SPTR_WAIT              IDC_WAIT
  #define SPTR_SIZE              IDC_SIZE
  #define SPTR_MOVE              IDC_UPARROW
  #define SPTR_SIZENWSE          IDC_SIZENWSE
  #define SPTR_SIZENESW          IDC_SIZENESW
  #define SPTR_SIZEWE            IDC_SIZEEWE
  #define SPTR_SIZENS            IDC_SIZEENS
  #define SPTR_APPICON           IDC_ICON

  #define SETCURSOR( id )       \
    SetCursor( LoadCursor( NULL, id )  )

/**********************************************************************/
/* set mouse pointer from handle                                      */
/**********************************************************************/
#define SETCURSORFROMHANDLE( handle )    \
    SetCursor( handle )

/**********************************************************************/
/* get mouse pointer                                                  */
/**********************************************************************/
#define GETCURSOR()       \
    GetCursor()
/**********************************************************************/
/* set/reset the mouse capture state...                               */
/**********************************************************************/
#define RELEASECAPTURE                          \
    ReleaseCapture()

#define SETCAPTURE( hwnd )                     \
    SetCapture( hwnd )

#define GETCAPTURE                             \
     GetCapture()

/**********************************************************************/
/* Fill rectangle with specified color                                */
/* ATTENTION: Under Windows we have to change the color to be a brush.*/
/*            Currently only CLR_WHITE is supported...                */
/**********************************************************************/
#define FILLRECT( hps, rectl, color )    \
      FillRect(hps, &rectl, (HBRUSH) GetStockObject( WHITE_BRUSH ))
/**********************************************************************/
/* Draw a text into a specified rectangle                             */
/**********************************************************************/
#define DRAWTEXT( hps, chWork, rectl, fg, bg, flags ) \
   {                                                    \
     SetTextColor(hps, UtlGetColorref(fg));             \
     SetBkColor(hps,UtlGetColorref(bg));                \
     DrawText(hps, chWork, -1, &rectl, flags);          \
   }
#define DRAWTEXTW( hps, chWork, rectl, fg, bg, flags ) \
   {                                                    \
     SetTextColor(hps, UtlGetColorref(fg));             \
     SetBkColor(hps,UtlGetColorref(bg));                \
     DrawTextW(hps, chWork, -1, &rectl, flags);          \
   }
/**********************************************************************/
/* spinbutton section ....                                            */
/**********************************************************************/
#define SPBMSETARRAY( hwnd, id, pStyle, num )              \
     SendDlgItemMessage( hwnd, id, SPBM_SETARRAY, num, (LPARAM) pStyle)
#define SPBMSETCURRENTVALUE( hwnd, id, index )             \
     SendDlgItemMessage( hwnd, id, SPBM_SETCURRENTVALUE, 0, (LPARAM) index)

#define SPBMQUERYVALUE( hwnd, id, buffer, flags )                  \
  {                                                                  \
     SPBQUERYVALUE   spbQueryValue;                                  \
     spbQueryValue.lpValueBuf  = buffer;                             \
     spbQueryValue.wBufSize    = sizeof( buffer );                   \
     spbQueryValue.wValue      = flags;                              \
     SendDlgItemMessage( hwnd, id, SPBM_QUERYVALUE,                  \
                         0L, ((DWORD)(LPVOID) (&spbQueryValue)) ); \
  }

/**********************************************************************/
/* clipboard section                                                  */
/**********************************************************************/
#define ISCLIPBOARDFORMATAVAILABLE( format, psinfo ) \
      IsClipboardFormatAvailable( format )

/**********************************************************************/
/* get line height                                                    */
/**********************************************************************/
#define GETLINEHEIGHT( hwnd, usLineHeight )                               \
  {                                                                         \
     HDC  hDC = GetDC( hwnd );                                              \
     TEXTMETRIC FontMetrics;                                                \
     GetTextMetrics( hDC, &FontMetrics );                                   \
     usLineHeight = (USHORT)FontMetrics.tmHeight;                                   \
     ReleaseDC( hwnd, hDC );                                                \
  }
/**********************************************************************/
/* Message beep                                                       */
/**********************************************************************/
#define BEEP( b )      \
    MessageBeep( (USHORT) b )

#define  MEASUREITEM( mp2, height, mResult ) \
  {                                                     \
    ((LPMEASUREITEMSTRUCT)mp2)->itemHeight = height;    \
    mResult = TRUE;                                     \
  }

/**********************************************************************/
/* return value from within DIALOGINIT                                */
/* Attention: Windows wants to get the reverse value to set the focus */
/*            correctly                                               */
/**********************************************************************/
#define DIALOGINITRETURN( mResult ) !mResult

/**********************************************************************/
/* subclassing of windows                                             */
/**********************************************************************/
#define SUBCLASSWND( hwnd, pfnSubProc )    \
      (PFNWP) SetWindowLong( hwnd, GWL_WNDPROC, (LONG) pfnSubProc )

/**********************************************************************/
/* Invalidate Rectangle                                               */
/**********************************************************************/
#define INVALIDATERECT( hwnd, pRect, fChildInclude )   \
      InvalidateRect( hwnd, pRect, FALSE )
/**********************************************************************/
/* Invalidate Region                                                  */
/**********************************************************************/
#define INVALIDATEREGION( hwnd, hrgn, fErase )   \
      InvalidateRgn( hwnd, hrgn, fErase )

//  macro for indirect oldwindow proc calls versus CallWindowProc....
#define CALLWINDOWPROC( Proc, hwnd, msg, mp1, mp2 )   \
     (MRESULT) CallWindowProc( (WNDPROC) Proc, hwnd, msg, mp1, mp2 )
#define CALLWINDOWPROCW( Proc, hwnd, msg, mp1, mp2 )   \
     (MRESULT) CallWindowProcW( (WNDPROC) Proc, hwnd, msg, mp1, mp2 )

/**********************************************************************/
/* Query and Set window ptr                                           */
/**********************************************************************/
#define GETWINDOWPTR( a, b )        GetWindowLong( a, b )
/**********************************************************************/
/* LS_EXTENDSEL style and WinPopupMenu prototype borrowed from        */
/* TOOLKT21                                                           */
/**********************************************************************/

/**********************************************************************/
/* Access to members of a RECTL structure                             */
/**********************************************************************/
  #define RECTL_YBOTTOM(r) r.bottom
  #define RECTL_YTOP(r)    r.top
  #define RECTL_XLEFT(r)   r.left
  #define RECTL_XRIGHT(r)  r.right

  #define PRECTL_YBOTTOM(r) r->bottom
  #define PRECTL_YTOP(r)    r->top
  #define PRECTL_XLEFT(r)   r->left
  #define PRECTL_XRIGHT(r)  r->right

/**********************************************************************/
/* Get text width (in pixel)                                          */
/**********************************************************************/
#define TEXTSIZE( hps, str, x, y ) \
      { SIZE size; GetTextExtentPoint32( hps, str, strlen(str), &size ); x = size.cx; y = size.cy; }

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/*                       Utilities Section                            */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/


#ifndef NO_UTLINCLUDE

#define MAX_TASK  5          // currently five concurrent task are available

PVOID Mymalloc( int size);
VOID  Myfree( void *pmem);

/**********************************************************************/
/* Plug structure for UtlPlugIn and UtlPlugOut calls                  */
/**********************************************************************/
struct _plug
{
     struct _plug *Bw;                 // Point to previous plug
     struct _plug *Fw;                 // Point to next plug
};
typedef struct _plug PLUG, *PPLUG;

/**********************************************************************/
/* Symbolic path ID values for UtlMakeEQFPath function                */
/**********************************************************************/
typedef enum _PATHID
{
   SYSTEM_PATH,                        // path to EQF root directory
   PROPERTY_PATH,                      // path to properties
   CTRL_PATH,                          // path to control files (profiles)
   PROGRAM_PATH,                       // path to program files
   DIC_PATH,                           // path to dictionary data bases
   MEM_PATH,                           // path to memory data bases
   TABLE_PATH,                         // path to translation tables
   LIST_PATH,                          // path to lists
   DIRSOURCEDOC_PATH,                  // path to source documents
   DIRSEGSOURCEDOC_PATH,               // path to segmented source docs
   DIRSEGTARGETDOC_PATH,               // path to segmented target docs
   DIRTARGETDOC_PATH,                  // path to target documents
   DLL_PATH,                           // path to DLLs
   MSG_PATH,                           // path to message and help files
   DOCU_PATH,                          // path to documentation files
   EXPORT_PATH,                        // path to exported files
   IMPORT_PATH,                        // path to imported files
   BACKUP_PATH,                        // path to backup directory
   COMMEM_PATH,                        // path to server TMs
   COMPROP_PATH,                       // path to server TM properties
   DIRSEGNOMATCH_PATH,                 // path to segmented not found matches
   COMDICT_PATH,                       // name of server dictionary properties
   PRT_PATH,                           // print format path
   WIN_PATH,                           // path to Windows DLLs and EXEs
   EADATA_PATH,                        // path to EA data of documents
   POE_PATH,                           // path to POE dicts on LAN main drive
   TABLEIMPORT_PATH,                   // path to tables during folder import
   DLLIMPORT_PATH,                     // path to user exit dlls during fol.imp.
   WINIMPORT_PATH,                     // path to Windows DLLs during fol.import
   DIRSEGMT_PATH,                      // path for Maschine Translation
   DIRSEGRTF_PATH,                     // path for RTF source for MS Word filter
   TQMEVAL_PATH,                       // path for TQM evaluation profiles
   TQMPROJ_PATH,                       // path for TQM projects
   TQMARCHIVE_PATH,                    // path for archived TQM projects
   TQMREPORT_PATH,                     // path for TQM report definitions
   TQMVENDOR_PATH,                     // path for TQM vendor profiles
   MTLOG_PATH,                         // path for MT log files
   MISC_PATH,                          // path for misc document files
   LOG_PATH,                           // path for debug and logging output
   XLIFF_PATH,                         // path for XLIFF source files
   METADATA_PATH,                      // path for segment metadata files
   JAVA_PATH,                          // path for JAVA classes
   ENTITY_PATH,                        // path for entity files
   PLUGIN_PATH,                        // path for plugins
   REMOVED_PATH                        // path for removed documents 
} PATHID;

/**********************************************************************/
/* Structure of item list of menu table                               */
/**********************************************************************/
typedef struct _itmlst
{
    SHORT   id;
    SHORT   attr;
    USHORT  mnu;                       // index to submenu entry
    USHORT  flg;
} ITMLST, *PITMLST;

/**********************************************************************/
/* Structure of menu list of menu table                               */
/**********************************************************************/
typedef struct _mnulst
{
    SHORT   id;
    SHORT   attr;
    HWND    hwnd;
    SHORT   items;
    USHORT  flg;
} MNULST, *PMNULST;

/**********************************************************************/
/* Menu table                                                         */
/**********************************************************************/
typedef struct _mnutbl
{
    HWND    hMenu;                     // hwnd of id FID_MENU
    USHORT  usMenus;                   // submenus in list
    USHORT  usItems;                   // submenu items in list
    PMNULST pTopMnu;                   // top submenu, no linked lists yet
    PITMLST pTopItm;                   // top menu item, no linked lists yet
} MNUTBL, *PMNUTBL;

/**********************************************************************/
/* Flags for UtlSetMenuItems function                                 */
/**********************************************************************/
#define fSETATTR              0x01
#define fCLEARATTR            0x02

/**********************************************************************/
/* Flags for UtlLoadFileNames function                                */
/**********************************************************************/
#define NAMFMT_FULL     0x0000   // insert full path
#define NAMFMT_NODRV    0x0001   // cut of drive letter and next "\"
#define NAMFMT_NODIR    0x0002   // cut off dir path and "\"
#define NAMFMT_NOEXT    0x0004   // cut off file extension
#define NAMFMT_NOROOT   0x0008   // do no insert parent dir
#define NAMFMT_TOPSEL   0x0100   // select top most list box entry
#define NAMFMT_NOERROR  0x0200   // do no popup error message boxes
#define UTLERROR        (-1)           // error occured

/**********************************************************************/
/* Symbolic value for no error handling / no messages                 */
/**********************************************************************/
#define NOMSG      (USHORT) (-1)

/**********************************************************************/
/* Types of error displayed by UtlError                               */
/**********************************************************************/
typedef enum _ERRTYPE
{
   SYSTEM_ERROR,                       // add. info through WinQueryLastError
   DOS_ERROR,                          // add. info for DOS disk access error
   EQF_ERROR,                          // work bench error
   EQF_WARNING,                        // work bench warning
   EQF_INFO,                           // work bench information
   EQF_QUERY,                          // work bench query
   INTERNAL_ERROR,                     // common internal  error
   QDAM_ERROR,                         // handle QDAM error
   QLDB_ERROR,                         // handle QLDB error
   PROP_ERROR,                         // handle property error
   MEM_ERROR,                          // handle transl. memory error
   CMD_ERROR,                          // handle errors of EQFCMD.EXE
   CMD_INFO,                           // handle info of EQFCMD.EXE
   SHOW_ERROR                          // only show a already prepared message text supplied in the pszParms parameter
} ERRTYPE;

/**********************************************************************/
/* Enum and structure typedefs                                        */
/**********************************************************************/

// message buffer to keep info about last message
#define MSGBUFFERLENGTH   2048         // length of error message buffer
typedef struct _DDEMSGBUFFER
{
  HWND  hwndDDE;                       // handle of DDE window
  SHORT sErrNumber;                    // error number / return code
  CHAR  chMsgText[MSGBUFFERLENGTH];    // associated message text
} DDEMSGBUFFER, *PDDEMSGBUFFER;
typedef struct _ERRDATA                // global data of UtlError
{
   HAB  hab;                           // anchor block handle
   HWND hwndFrame;                     // handle of main window frame
   HWND hwndHelpInstance;              // help instance handle
   CHAR chMsgFile[CCHMAXPATH];         // name of message file
   HFILE hMsgFile;                     // handle of message file
   PDDEMSGBUFFER pLastMessage;         // ptr to buffer for last message
} ERRDATA, *PERRDATA;
/**********************************************************************/
/* Minimum allocation size for memory blocks allocated using UtlAlloc */
/**********************************************************************/
#define MIN_ALLOC      10              // minimum UtlAlloc allocation size
#define MAX_ALLOC      0xFF00          // maximum UtlAlloc allocation size

/**********************************************************************/
/* prototypes for Setter functions for MAT ULONG/USHORT               */
/**********************************************************************/
__declspec(dllexport)
VOID UtlSetULong( SHORT sID, ULONG ulValue );
__declspec(dllexport)
VOID UtlSetString( SHORT sID, PSZ pszString );
__declspec(dllexport)
VOID UtlSetUShort( SHORT sID, USHORT usValue );


/**********************************************************************/
/* MAT ULONG  identifiers                                             */
/**********************************************************************/
typedef enum _QL_VALUES
{
   QL_FIRST,                           // starts QL values
   QL_HAB,                             // MAT anchor block handle
   QL_TWBFRAME,                        // MAT twb frame window handle
   QL_TWBCLIENT,                       // MAT twb client handle
   QL_PELSPERLINE,                     // pels-per-line value of default font
   QL_AVECHARWIDTH,                    // average character width of default font
   QL_CHARHEIGHT,                      // character height of default font
   QL_HELPINSTANCE,                    // help instance handle
   QL_EMCHARWIDTH,                     // width of 'M' character)
   QL_TWBMENU,                         // handle of AAB menu of default TWB
   QL_TWBWINDOWMENU,                   // handle of Windows Pulldown in def.menu
   QL_TWBTOOLBAR,                      // handle of toolbar ...
   QL_TOOLBARHEIGHT,                   // height of the toolbar ...
   QL_TWBSTATUSBAR,                    // handle of statusbar ...
   QL_PROCESSLASTXPOS,                 // last x position for process windows
   QL_PROCESSLASTYPOS,                 // last y position for process windows
   QL_HRESMOD,                         // resource module handle
   QL_DICTIMPICON,
   QL_DICTEXPICON,
   QL_FOLIMPICON,
   QL_FOLEXPICON,
   QL_TWBICON,
   QL_FLLICON,
   QL_FOLICON,
   QL_DOCICON,
   QL_ANAICON,
   QL_MEMICON,
   QL_TAGICON,
   QL_COUNTICON,
   QL_TMEMIMPICON,
   QL_TMEMEXPICON,
   QL_TMEMMERGEICON,
   QL_TMEMORGICON,
   QL_MARKUPIMPICON,
   QL_MARKUPEXPICON,
   QL_DICTORGICON,
   QL_DICTLISTICON,
   QL_LISTICON,
   QL_DICTPRINTICON,
   QL_DICTENTRYDISPICO,
   QL_TMMICON,
   QL_EXCLLISTICON,
   QL_NEWLISTICON,
   QL_FOUNDLISTICON,
   QL_SMALLFUZZLEVEL,
   QL_MEDIUMFUZZLEVEL,
   QL_LARGEFUZZLEVEL,
   QL_SMALLLOOKUPFUZZLEVEL,
   QL_MEDIUMLOOKUPFUZZLEVEL,
   QL_LARGELOOKUPFUZZLEVEL,
   QL_MTMQSAVAILABLE,
   QL_MTDISPLAYFACTOR,
   QL_TREEVIEWMENU,                    // TWB handle of AAB menu when tree view folder/document window is active
   QL_LAST                             // ends QL values
} QL_VALUES;

/**********************************************************************/
/* MAT USHORT identifiers                                             */
/**********************************************************************/
// values for QS_RUNMODE
#define  TWB_RUNMODE       1
#define  FUNCCALL_RUNMODE  2

// values for QS_GUIMODE
#define  NONMFC_GUIMODE    0
#define  MFC_GUIMODE       1

// values for QS_PROGRAMID
#define  EQFSTART_PROGID  1
#define  BATCHUTIL_PROGID  2
#define  NONDDEAPI_PROGID 3


typedef enum _QS_VALUES
{
   QS_FIRST,                           // starts QS values
   QS_CURMSGID,                        // ID of current message box
   QS_CURMENUITEMID,                   // ID of current menu item
   QS_CURMENUID,                       // ID of current menu
   QS_LASTERRORMSGID,                  // last error code
   QS_LASTDDEMSGID,                    // last DDE message error code
   QS_RUNMODE,                         // run mode (TWB_RUNMODE or FUNCCALL_RUNMODE)
   QS_GUIMODE,                         // GUI mode (MFC, non-MFC)
   QS_CLIPBOARDFORMAT,                 // custom clipboard format //cv
   QS_SGMLDITAPROCESSING,              // SGML-DITA processing enabled flag
   QS_XSLTENGINE,                      // XSLT engine to be used
   QS_PROGRAMID,                       // ID of program running TM
   QS_ENTITYPROCESSING,                // ID of entity processing flag
   QS_MTLOGGING,                       // ID of MT logging flag
   QS_MEMIMPMRKUPACTION,               // ID of memory import markup action
   QS_VIETNAMESEKEYBOARDMESSAGESHOWN,  // flag: the Vietnames keyboard message has been shon in this session
   QS_HIDETOOLBARS,                    // flag: whether to hide TWB and Translation Env. toolbars
   QS_LAST                             // ends QS values
} QS_VALUES;


/**********************************************************************/
/* MAT string identifiers                                             */
/**********************************************************************/
typedef enum _QST_VALUES
{
   QST_FIRST,                          // starts QST values
   QST_MSGFILE,                        // name of message file
   QST_HLPFILE,                        // name of help file
   not_used,                           //
   QST_ORGEQFDRIVES,                   // list of EQF drives out of system props
   QST_VALIDEQFDRIVES,                 // list of valid EQF drives
   QST_PRIMARYDRIVE,                   // primary EQF drive
   QST_PROPDIR,                        // name of property directory
   QST_CONTROLDIR,                     // name of control directory
   QST_PROGRAMDIR,                     // name of program directory
   QST_MEMDIR,                         // name of memory directory
   QST_DICDIR,                         // name of dictionary directory
   QST_TABLEDIR,                       // name of table directory
   QST_LISTDIR,                        // name of list directory
   QST_SOURCEDIR,                      // name of source documents directory
   QST_SEGSOURCEDIR,                   // name of segmented source doc directory
   QST_SEGTARGETDIR,                   // name of segmented target doc directory
   QST_TARGETDIR,                      // name of target doc directory
   QST_DLLDIR,                         // name of DLL directory
   QST_MSGDIR,                         // name of message and help directory
   QST_EXPORTDIR,                      // name of export directory
   QST_BACKUPDIR,                      // name of backup directory
   QST_IMPORTDIR,                      // name of import directory
   QST_SYSTEMDIR,                      // name of system directory
   QST_COMMEMDIR,                      // name of server TM directory
   QST_COMPROPDIR,                     // name of server TM property directory
   QST_DIRSEGNOMATCHDIR,               // name of not found segments directory
   QST_COMDICTDIR,                     // name of server dictionary properties
   QST_PRTPATH,                        // name of print format directory
   QST_WINPATH,                        // name of Windows DLL and EXE directory
   QST_RESFILE,                        // name of resource file
   QST_EADATAPATH,                     // name of EA data directory
   QST_LANDRIVE,                       // name of EQF LAN drive
   QST_DIRSEGMTDIR,                    // name of not found segments directory
   QST_DIRSEGRTFDIR,                   // name of not found segments directory
   QST_TQMPROJECTPATH,                 // name of TQM project directory
   QST_TQMREPORTPATH,                  // name of TQM report directory
   QST_TQMARCHIVEPATH,                 // name of TQM project archive directory
   QST_TQMEVALUATIONPATH,              // name of TQM evaluation profile directory
   QST_TQMVENDORPATH,                  // name of TQM vendor directory
   QST_SYSPROPFILE,                    // fully qualified name of system property file
   QST_MTLOGPATH,                      // name of MT log path
   QST_MISCPATH,                       // name of MISC path
   QST_LOGPATH,                        // name LOG path
   QST_XLIFFPATH,                      // name XLIFF oath
   QST_METADATAPATH,                   // name METADATA path
   QST_JAVAPATH,                       // name JAVA path
   QST_ENTITY,                         // name of ENTITY path
   QST_PLUGINPATH,                     // name of PLUGIN path
   QST_REMOVEDDOCDIR,                  // name of path for removed documents
   QST_LAST                            // ends QST values
} QST_VALUES;

/**********************************************************************/
/* Symbolic value for the end of a CLB view list                      */
/**********************************************************************/
#define CLBLISTEND (-1)                // end of a view and sort list

/**********************************************************************/
/* Symbolic IDs for CLB view lists                                    */
/**********************************************************************/
typedef enum _VIEWLISTIDS              // types of column listbox data
{
   CURRENT_VIEW,                       // currently displayed view
   DETAILS_VIEW,                       // user's details view list
   DEFAULT_VIEW,                       // default view list for details
   NAME_VIEW,                          // view list for 'name' view option
   SORT_VIEW                           // sort column list
} VIEWLISTIDS;

/**********************************************************************/
/* Types of column listbox data                                       */
/**********************************************************************/
typedef enum _CLBDATATYPE              // types of column listbox data
{
   TEXT_DATA,                          // text string (alphabetic sort is used)
   NUMERIC_DATA,                       // numeric data (numeric sort is used)
   DATE_DATA,                          // date in long format (C time function )
   TIME_DATA,                          // time in long format (C time function )
   DATETIME_DATA,                      // date/time in long format ( s.a. )
   FDATE_DATA,                         // file date (FDATE) stored as a decimal
   FTIME_DATA,                         // file time (FTIME) stored as a decimal
   FDATETIME_DATA,                     // file date and time(FDATE/FTIME)
                                       // stored as two decimals seperated by
                                       // a period ('.')
   AUTOWIDTHTEXT_DATA                  // text string with autowidth feature
} CLBDATATYPE;

// special path mode display flags for listbox columns (are ORed with the column number)
#define CLB_SHRINKPATH    0x4000
#define CLB_HIDEPATH      0x2000
// masks to access path mode display flags or column number
#define CLB_PATHMODEMASK  0x6000
#define CLB_COLNUMBERMASK 0x9FFF

/**********************************************************************/
/* Data structure for the control information of a single column of   */
/* the column listbox                                                 */
/**********************************************************************/
typedef struct _CLBCOLDATA             // data of a single column
{
   PSZ          pszTitle;              // title text of listbox column
   USHORT       usWidth;               // width of column in characters!
   CLBDATATYPE  DataType;              // type of column
   USHORT       usFormat;              // column format flags
} CLBCOLDATA, *PCLBCOLDATA;

/**********************************************************************/
/* Column listbox control data                                        */
/**********************************************************************/
typedef struct _CLBCTLDATA             // control data for column listbox
{
   USHORT       usSize;                // size of column listbox control data
   USHORT       usNoOfColumns;         // number of listbox columns
   USHORT       usColDistance;         // distance between columns
   LONG         lTitleColor;           // color of title text
   LONG         lTitleBackColor;       // color of title text background
   LONG         lItemColor;            // color of listbox items
   LONG         lItemBackColor;        // color of listbox items background
   CHAR         chDataSeperator;       // character seperating item data
   PSHORT       psLastUsedViewList;    // 'Last used' view list
   PSHORT       psDefaultViewList;     // default view list for details
   PSHORT       psDetailsViewList;     // details view list set by user
   PSHORT       psNameViewList;        // view list for 'name' view option
   PSHORT       psSortList;            // list of sort criteria columns
   PCLBCOLDATA  pColData;              // ptr to column table
   HWND         hwndOwner;             // owner handle of column listbox control
   PCLBFILTER   pFilter;               // pointer to filter for CLB items
} CLBCTLDATA, *PCLBCTLDATA;

//------------------------------------------------------------------------------
//   Control block for the buffered input/output utilities
//------------------------------------------------------------------------------
typedef struct _BUFCB
{
  HFILE        hFile;                  // handle of file
  ULONG        ulUsed;                 // used bytes in buffer
  ULONG        ulSize;                 // size of input/output buffer
  ULONG        ulProcessed;            // number of bytes processed in buffer
  ULONG        ulRemaining;            // # of bytes remaining in file (READ)
  BOOL         fWrite;                 // file access flag
  CHAR         szFileName[CCHMAXPATH]; // buffer for file name
  BYTE         Buffer[1];              // input/output buffer (variable length)
} BUFCB, *PBUFCB;


typedef enum _CHARTABLEID
{
  IS_TEXT_TABLE,                       // table to identify text characters
  ASCII_TO_ANSI_TABLE,                 // table ASCII (850, 852, 855, 866, 915,
                                       // 857, 1251 depending on active CP) to
                                       // ANSI
  ANSI_TO_ASCII_TABLE,                 // table ANSI to ASCII (850, 852, 855,
                                       // 866, 915, 857, 1251 depending on
                                       // currently active CP)
} CHARTABLEID;



#define FILE_APPEND 0x0011             // FILE_CREATE defined in bsedos.h
/**********************************************************************/
/* Window class for column listbox controls                           */
/**********************************************************************/
#define WC_EQF_CLBCLASS "EqfCLB"       // window class for CLB

/**********************************************************************/
/* Error code returned by UtlGetLANUserID function                    */
/**********************************************************************/
#define LANUID_NO_LAN            1000
#define LANUID_REQ_NOT_STARTED   1001
#define LANUID_USER_NOT_LOG_ON   1002
#define LANUID_OTHER_ERROR       (-1)

/**********************************************************************/
/* User types returned by UtlGetLANUserID function                    */
/**********************************************************************/
#define USER_GUEST               0
#define USER_USER                1
#define USER_ADMIN               2

/**********************************************************************/
/* Buffer size for UtlGetLANUserID function                           */
/**********************************************************************/
#define NET_MAX_BUF_SIZE         1020

/**********************************************************************/
/* Print handle for UtlPrint functions                                */
/**********************************************************************/
typedef ULONG HPRINT, *PHPRINT;

/**********************************************************************/
/* Function prototypes                                                */
/**********************************************************************/
__declspec(dllexport)
USHORT  UtlOpen( PSZ pszFname, PHFILE phfOpen, PUSHORT pusAction, ULONG ulFSize,
                USHORT usAttr, USHORT fsOpenFlags, USHORT fsOpenMode,
                ULONG ulReserved, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlClose( HFILE hf, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlRead( HFILE hf, PVOID pBuf, USHORT cbBuf, PUSHORT pcbBytesRead,
                BOOL fMsg );
__declspec(dllexport)
USHORT  UtlReadL( HFILE hf, PVOID pBuf, ULONG cbBuf, PULONG pcbBytesRead,
                BOOL fMsg );
__declspec(dllexport)
USHORT  UtlWrite( HFILE hf, PVOID pBuf, USHORT cbBuf, PUSHORT pcbBytesWritten,
                 BOOL fMsg );
__declspec(dllexport)
USHORT  UtlWriteL( HFILE hf, PVOID pBuf, ULONG cbBuf, PULONG pcbBytesWritten,
                 BOOL fMsg );
USHORT  UtlDupHandle( HFILE hfOld, PHFILE phfNew, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlChgFilePtr( HFILE hf, LONG lOffset, USHORT fsMethod,
                      PULONG pulNewOffset, BOOL fMsg );

__declspec(dllexport)
USHORT  UtlDelete( PSZ pszFName, ULONG ulReserved, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlMkDir( PSZ pszDirName, ULONG usReserved, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlRmDir( PSZ pszDir, ULONG ulReserved, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlCopy( PSZ pszSrc, PSZ pszDst, USHORT usOpt, ULONG ulReserved, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlMove( PSZ pszSrc, PSZ pszDst, ULONG ulReserved, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlFindFirst( PSZ pszFSpec, PHDIR phdir, USHORT usAttr,
                     PFILEFINDBUF pffb, USHORT cbBuf, PUSHORT pcSearch,
                     ULONG ulReserved, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlFindNext( HDIR hdir, PFILEFINDBUF pffb, USHORT cbBuf,
                    PUSHORT pcSearch, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlFindClose( HDIR hdir, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlQFileInfo( HFILE hf, USHORT usInfoLevel, PBYTE pInfoBuf,
                     USHORT cbInfoBuf, BOOL fMsg );
__declspec(dllexport)
USHORT UtlGetFileSize
(
   HFILE    hf,                        // file handle
   PULONG   pulSize,                   // points to buffer for file size
   BOOL     fMsg                        // if TRUE handle errors in utility
);
USHORT UtlGetFileSizeHwnd
(
   HFILE    hf,                        // file handle
   PULONG   pulSize,                   // points to buffer for file size
   BOOL     fMsg,                       // if TRUE handle errors in utility
   HWND     hwndParent
);
USHORT  UtlSetFileInfo( HFILE hf, USHORT usInfoLevel, PBYTE pInfoBuf,
                       USHORT cbInfoBuf, BOOL fMsg );
USHORT  UtlQFSInfo( USHORT usDriveNo, USHORT usInfoLevel, PBYTE pInfoBuf,
                   USHORT cbInfoBuf, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlQFileMode( PSZ pszFileName, PUSHORT pusAttribute, ULONG ulReserved,
                     BOOL fMsg );
__declspec(dllexport)
USHORT  UtlQPathInfo( PSZ pszPath, USHORT usInfoLevel, PBYTE pInfoBuf,
                     USHORT cbInfoBuf, ULONG ulReserved, BOOL fMsg );
VOID    UtlDestroyChildResources( VOID );
BOOL    UtlCreateChildResources( HWND hwnd );
MRESULT UtlCreateChildSysMenu( HWND hwnd );
MRESULT UtlTransAccel ( HWND, WPARAM, LPARAM );
// not compatible with MFC-DLL compiling bt 10042001
#ifndef _CEQFPPT
//MRESULT EXPENTRY UTLDEFDIALOGPROC( HWND, WINMSG, WPARAM, LPARAM );
#define UTLDEFDIALOGPROC( hwnd, usMsg, mp1, mp2) WinDefDlgProc( hwnd, usMsg, mp1, mp2 )
__declspec(dllexport)
MRESULT EXPENTRY UTLDEFHANDLERPROC( HWND, WINMSG, WPARAM, LPARAM );
MRESULT EXPENTRY UTLDEFINSTANCEPROC( HWND, WINMSG, WPARAM, LPARAM );
//MRESULT EXPENTRY UTLDEFWINDOWPROC( HWND, WINMSG, WPARAM, LPARAM );
#define UTLDEFWINDOWPROC( hwnd, usMsg, mp1, mp2) WinDefWindowProc( hwnd, usMsg, mp1, mp2 )
#endif
__declspec(dllexport)
BOOL    UtlRegisterCLB( HAB );
__declspec(dllexport)
PSZ     Utlstrccpy( PSZ, PSZ, CHAR );
__declspec(dllexport)
PSZ     Utlstrnccpy( PSZ, PSZ, USHORT, CHAR );
__declspec(dllexport)
VOID    UtlDispatch( VOID );
__declspec(dllexport)
ULONG   UtlQueryULong( SHORT sID );
__declspec(dllexport)
VOID    UtlMenuDisableAll( HWND hwnd, SHORT sMenuID );
__declspec(dllexport)
VOID    UtlMenuDisableItem( SHORT sItemID );
__declspec(dllexport)
VOID    UtlMenuEnableItem( SHORT sItemID );
__declspec(dllexport)
ULONG   UtlLongToTimeString( LONG, PSZ, ULONG );
__declspec(dllexport)
ULONG   UtlLongToDateString( LONG, PSZ, ULONG );
__declspec(dllexport)
ULONG   UtlLongToTimeStringW( LONG, PSZ_W, ULONG );
__declspec(dllexport)
ULONG   UtlLongToDateStringW( LONG, PSZ_W, ULONG );
__declspec(dllexport)
BOOL    UtlLoadFile( PSZ, PVOID *, USHORT *, BOOL, BOOL );
__declspec(dllexport)
BOOL    UtlLoadFileL( PSZ, PVOID *, ULONG *, BOOL, BOOL );
__declspec(dllexport)
BOOL    UtlLoadFileHwnd( PSZ, PVOID *, ULONG *, BOOL, BOOL, HWND );
__declspec(dllexport)
USHORT  UtlCheckSpaceForFile( PSZ, USHORT, LONG, PSZ *, LONG *, BOOL );
__declspec(dllexport)
USHORT  UtlCheckSpaceForFileEx( PSZ, USHORT, LONG, PSZ *, LONG *, BOOL, PUSHORT );
__declspec(dllexport)
ULONG64 UtlQueryFreeSpace( CHAR, BOOL );
__declspec(dllexport)
USHORT  UtlMkMultDir( PSZ, BOOL );
__declspec(dllexport)
USHORT  UtlMkMultDirHwnd( PSZ, BOOL, HWND );
__declspec(dllexport)
BOOL    UtlPrintOpen( PHPRINT phPrint, PSZ pszListName, HWND hwndOwner );
__declspec(dllexport)
BOOL    UtlPrintOpenW( PHPRINT phPrint, PSZ_W pszListName, HWND hwndOwner );
__declspec(dllexport)
BOOL    UtlPrintClose( HPRINT hPrint );
BOOL    UtlPrintFormFeed( HPRINT hPrint );
__declspec(dllexport)
BOOL    UtlPrintLineW( HPRINT hPrint, PSZ_W pszLine );
__declspec(dllexport)
BOOL    UtlPrintLine( HPRINT hPrint, PSZ pszLine );
__declspec(dllexport)
BOOL    UtlPrintSetAnsiConv( HPRINT hPrint, BOOL fAnsiConv );
__declspec(dllexport)
BOOL    UtlFileExist( PSZ pszFileName );
__declspec(dllexport)
BOOL    UtlDirExist( PSZ pszDirName );
BOOL    UtlMoveIfNewerOrNotExist( PSZ pszSource, PSZ pszTarget );
__declspec(dllexport)
BOOL    UtlMoveIfNewerOrNotExistHwnd( PSZ pszSource, PSZ pszTarget, HWND hwnd );
__declspec(dllexport)
BOOL    UtlMoveIfNewerOrNotExistHwnd2( PSZ pszSource, PSZ pszTarget, HWND hwnd, BOOL fMsg, PUSHORT pusDosRC );
__declspec(dllexport)
USHORT  UtlSmartMove( PSZ pszSource, PSZ pszTarget, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlSmartMoveHwnd( PSZ pszSource, PSZ pszTarget, BOOL fMsg, HWND hwnd );
__declspec(dllexport)
USHORT  UtlRemoveDir( PSZ pszDir, BOOL fMsg );
__declspec(dllexport)
SHORT   UtlCopyListBox( HWND, HWND );
__declspec(dllexport)
PSZ     UtlMakeFullPath ( PSZ, PSZ, PSZ, PSZ, PSZ );
__declspec(dllexport)
USHORT  UtlWriteFile( PSZ pszFile, USHORT usDataLength, PVOID pData, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlWriteFileL( PSZ pszFile, ULONG ulDataLength, PVOID pData, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlWriteFileHwnd( PSZ pszFile, ULONG ulDataLength, PVOID pData,
        BOOL fMsg, HWND hwndParent );
BOOL    UtlIsEqfDrive( CHAR chDrive );
__declspec(dllexport)
BOOL    UtlGetCheckedEqfDrives( PSZ pszDrives );
__declspec(dllexport)
USHORT  UtlGetLANUserID( PSZ pszLANUserID, USHORT *pusUserPriv, USHORT usMsgHandling );
__declspec(dllexport)
USHORT  UtlGetLANUserIDW( PSZ_W pszLANUserID, USHORT *pusUserPriv, USHORT usMsgHandling );
__declspec(dllexport)
PSZ     UtlGetFnameFromPath( PSZ path);
PSZ     UtlMakeObjName( PSZ p, PSZ drive, PSZ path, PSZ fname, PSZ ext);
__declspec(dllexport) VOID UtlMakeFNameAndPath( PSZ, PSZ, PSZ );
__declspec(dllexport)
PSZ     UtlParseX15( PSZ str, SHORT id);
PSZ_W   UtlParseX15W( PSZ_W str, SHORT id);
__declspec(dllexport)
PSZ_W   UtlParseCharW( PSZ_W str, SHORT id, CHAR_W chParse );

__declspec(dllexport)
USHORT  UtlExtractX15( PSZ pszTarget, PSZ pszX15String, SHORT sStringId );

PPLUG   UtlPlugIn( PPLUG new1, PPLUG bw, PPLUG fw);
PPLUG   UtlPlugOut( PPLUG plug);
__declspec(dllexport)
PSZ     UtlSplitFnameFromPath( PSZ path);
__declspec(dllexport)
PSZ     UtlMakeEQFPath( PSZ path, CHAR chDrive, USHORT usPathID, PSZ pszFolder );
__declspec(dllexport)
USHORT  UtlGetDriveList( BYTE *szList);
__declspec(dllexport)
USHORT  UtlGetLANDriveList( BYTE *szList);
BYTE    UtlQCurDisk( void);
BOOL    UtlChangePropHead( PSZ, CHAR, PSZ, PSZ );
__declspec(dllexport)
SHORT   UtlLoadFileNames( PSZ search_path, USHORT atrb, HWND hlb, USHORT flg);
__declspec(dllexport)
SHORT   UtlLoadFileNamesHwnd( PSZ search_path, USHORT atrb,
                              HWND hlb, USHORT flg, HWND hwnd);
PMNUTBL UtlLoadMenuTbl( HWND hmenu);
VOID    UtlFreeMenuTbl( PMNUTBL ptbl);
VOID    UtlSetMenuItemsAttr( PMNUTBL ptbl);
MRESULT APIENTRY UTLDRIVEBUTTONWP( HWND, USHORT, WPARAM, LPARAM);
__declspec(dllexport)
USHORT  UtlCreateDriveButtons( HWND hwnd, PSZ plist, USHORT sid,
                               ULONG style1, ULONG style);
__declspec(dllexport)
USHORT UtlSetPosDriveButtons2( HWND, USHORT, ULONG, ULONG, ULONG, ULONG,
                                ULONG, ULONG, ULONG, ULONG, HWND );

__declspec(dllexport)
USHORT UtlDriveButtons( HWND hwnd, PSZ plist, USHORT sid,                          // starting id
                        ULONG style1, ULONG style,
                        HWND  hwndGroupBox, HWND  hwndDummy, HWND hwndDelim );
__declspec(dllexport)
SHORT UtlDriveType( CHAR chDrive );

__declspec(dllexport)
VOID UtlSetHorzScrollingForLB(HWND hLB);

__declspec(dllexport)
VOID   UtlSaveWindowPos( HWND hFrame, EQF_PSWP pSwp);
__declspec(dllexport)
BOOL   UtlAlloc ( PVOID *, LONG, LONG, USHORT );
__declspec(dllexport)
BOOL   UtlAllocHwnd ( PVOID *, LONG, LONG, USHORT, HWND );
__declspec(dllexport)
BOOL   UtlInitUtils( HAB );
__declspec(dllexport)
BOOL   UtlRegisterEqfHelp( HAB );
__declspec(dllexport)
BOOL   UtlTerminateUtils( VOID );
__declspec(dllexport)
USHORT UtlError ( SHORT, USHORT, USHORT, PSZ *, ERRTYPE );
__declspec(dllexport)
USHORT UtlErrorHwnd ( SHORT, USHORT, USHORT, PSZ *, ERRTYPE, HWND );
__declspec(dllexport)
USHORT UtlErrorW ( SHORT, USHORT, USHORT, PSZ_W *, ERRTYPE, BOOL );
__declspec(dllexport)
USHORT UtlErrorHwndW ( SHORT, USHORT, USHORT, PSZ_W *, ERRTYPE, HWND, BOOL );
__declspec(dllexport)
VOID UtlGetMsgTxt( SHORT, PSZ, USHORT, PSZ *);

__declspec(dllexport)
VOID   UtlInitError ( HAB, HWND, HWND, PSZ );
__declspec(dllexport)
VOID   UtlTerminateError ( VOID );
__declspec(dllexport)
USHORT UtlQueryUShort( SHORT sID );
__declspec(dllexport)
BOOL   UtlQueryString( SHORT sID, PSZ pszBuffer, USHORT usLength );
__declspec(dllexport)
ULONG  UtlFDateToDateString( FDATE *, PSZ, ULONG );
__declspec(dllexport)
ULONG  UtlFTimeToTimeString( FTIME *, PSZ, ULONG );
__declspec(dllexport)
BOOL   UtlDateStringToFDate( PSZ, FDATE * );
__declspec(dllexport)
BOOL   UtlTimeStringToFTime( PSZ, FTIME * );
__declspec(dllexport)
USHORT UtlSetFileMode ( PSZ, USHORT, ULONG, BOOL );
__declspec(dllexport)
VOID   UtlStripBlanks ( PSZ );
__declspec(dllexport)
VOID   UtlStripBlanksW( PSZ_W );
__declspec(dllexport)
BOOL   UtlSetDrive( CHAR szNewDrive );
__declspec(dllexport)
USHORT  UtlBufOpen( PBUFCB *, PSZ, ULONG, ULONG, BOOL );
__declspec(dllexport)
USHORT  UtlBufClose( PBUFCB pBufCB, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlBufWrite( PBUFCB pBufCB, PSZ pszData, ULONG ulDataLength, BOOL fMsg );
__declspec(dllexport)
USHORT  UtlBufRead( PBUFCB, PSZ, ULONG, PULONG, BOOL );
USHORT  UtlBufOpenHwnd( PBUFCB *, PSZ, ULONG, ULONG, BOOL, HWND );
USHORT  UtlBufCloseHwnd( PBUFCB pBufCB, BOOL fMsg, HWND hwnd );
__declspec(dllexport)
USHORT  UtlBufWriteHwnd( PBUFCB pBufCB, PSZ pszData, ULONG ulDataLength, BOOL fMsg, HWND hwnd );
USHORT  UtlBufReadHwnd( PBUFCB, PSZ, ULONG, PULONG, BOOL, HWND );
__declspec(dllexport)
USHORT  UtlBufWriteW( PBUFCB, PSZ_W, ULONG, BOOL );
USHORT  UtlBufWriteHwndW( PBUFCB, PSZ_W, ULONG, BOOL, HWND );
__declspec(dllexport)
USHORT  UtlBufWriteConv( PBUFCB, PSZ_W, ULONG, BOOL, USHORT, ULONG, ULONG );

VOID    UtlIncludeRemoteResources(VOID);
__declspec(dllexport)
VOID    UtlGetLastError( HWND, PSZ );
__declspec(dllexport)
USHORT  UtlGetDDEErrorCode( HWND );
__declspec(dllexport)
SHORT   UtlQdamMsgTxt ( SHORT );

__declspec(dllexport)
SHORT   UtlGetOperatingSystemInfo( VOID );
__declspec(dllexport)
VOID    UtlGetWinError( HRESULT, PSZ );
__declspec(dllexport)
BOOL    UtlGetPidMultipleInstances( PSZ, DWORD * );
__declspec(dllexport)
BOOL    UtlKillProcess( DWORD );
BOOL    UtlProcessWalk(SHORT, PSZ, DWORD *);

USHORT  UtlFindCloseHwnd( HDIR, BOOL, HWND );
__declspec(dllexport)
USHORT  UtlCloseHwnd( HFILE hf, BOOL fMsg, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlOpenHwnd( PSZ pszFname, PHFILE phfOpen, PUSHORT pusAction, ULONG ulFSize,
                     USHORT usAttr, USHORT fsOpenFlags, USHORT fsOpenMode,
                     ULONG ulReserved, BOOL fMsg, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlReadHwnd( HFILE hf, PVOID pBuf, ULONG cbBuf, PULONG pcbBytesRead,
                     BOOL fMsg, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlWriteHwnd( HFILE hf, PVOID pBuf, ULONG cbBuf, PULONG pcbBytesWritten,
                      BOOL fMsg, HWND hwndParent  );
__declspec(dllexport)
USHORT  UtlWriteWoCheckHwnd( HFILE hf, PVOID pBuf, ULONG cbBuf,
                             PULONG pcbBytesWritten, BOOL fMsg, HWND hwndParent  );
USHORT  UtlDupHandleHwnd( HFILE hfOld, PHFILE phfNew, BOOL fMsg,
                          HWND hwndParent );
__declspec(dllexport)
USHORT  UtlChgFilePtrHwnd( HFILE hf, LONG lOffset, USHORT fsMethod,
                           PULONG pulNewOffset, BOOL fMsg, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlDeleteHwnd( PSZ pszFName, ULONG ulReserved, BOOL fMsg, HWND hwndParent );
USHORT  UtlMkDirHwnd( PSZ pszDirName, ULONG usReserved, BOOL fMsg, HWND hwndParent );
USHORT  UtlRmDirHwnd( PSZ pszDir, ULONG ulReserved, BOOL fMsg, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlCopyHwnd( PSZ pszSrc, PSZ pszDst, USHORT usOpt, ULONG ulReserved,
                     BOOL fMsg, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlCopyHwnd2( PSZ pszSrc, PSZ pszDst, USHORT usOpt, ULONG ulReserved,
                      BOOL fMsg, PUSHORT pusRCList, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlMoveHwnd( PSZ pszSrc, PSZ pszDst, ULONG ulReserved,
                     BOOL fMsg, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlFindFirstHwnd( PSZ pszFSpec, PHDIR phdir, USHORT usAttr,
                          PFILEFINDBUF pffb, USHORT cbBuf, PUSHORT pcSearch,
                         ULONG ulReserved, BOOL fMsg, HWND hwndParent );
USHORT  UtlFindNextHwnd( HDIR hdir, PFILEFINDBUF pffb, USHORT cbBuf,
                         PUSHORT pcSearch, BOOL fMsg, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlQFileInfoHwnd( HFILE hf, USHORT usInfoLevel, PBYTE pInfoBuf,
                          USHORT cbInfoBuf, BOOL fMsg, HWND hwndParent );
USHORT  UtlSetFileInfoHwnd( HFILE hf, USHORT usInfoLevel, PBYTE pInfoBuf,
                            USHORT cbInfoBuf, BOOL fMsg, HWND hwndParent );
__declspec(dllexport)
USHORT  UtlQFSInfoHwnd( USHORT usDriveNo, USHORT usInfoLevel, PBYTE pInfoBuf,
                        USHORT cbInfoBuf, BOOL fMsg, HWND hwndParent );
USHORT  UtlQFileModeHwnd( PSZ pszFileName, PUSHORT pusAttribute, ULONG ulReserved,
                          BOOL fMsg, HWND hwndParent );
USHORT  UtlQPathInfoHwnd( PSZ pszPath, USHORT usInfoLevel, PBYTE pInfoBuf,
                          USHORT cbInfoBuf, ULONG ulReserved, BOOL fMsg,
                          HWND hwndParent);
__declspec(dllexport)
USHORT  UtlSetFileModeHwnd ( PSZ, USHORT, ULONG, BOOL, HWND );
__declspec(dllexport)
USHORT  UtlFileLocks( HFILE, PFILELOCK, PFILELOCK, BOOL );
USHORT  UtlFileLocksHwnd( HFILE, PFILELOCK, PFILELOCK, BOOL, HWND );
USHORT  UtlGetLongFTime( HFILE hFile, PLONG plTime );
__declspec(dllexport)
PSZ     UtlUpper ( PSZ  pData );
__declspec(dllexport)
PSZ_W   UtlUpperW( PSZ_W  pData );
BYTE    UtlToUpper (BYTE c);
__declspec(dllexport)
CHAR_W  UtlToUpperW(CHAR_W c);
__declspec(dllexport)
PSZ     UtlLower ( PSZ  pData );
__declspec(dllexport)
PSZ_W   UtlLowerW( PSZ_W  pData );
BYTE    UtlToLower (BYTE c);
__declspec(dllexport)
CHAR_W  UtlToLowerW(CHAR_W c);
__declspec(dllexport)
BOOL UtlIsWhiteSpaceW( CHAR_W c );
__declspec(dllexport)
SHORT   UtlCompIgnWhiteSpace(PSZ, PSZ, USHORT);
__declspec(dllexport)
LONG    UtlCompIgnWhiteSpaceW(PSZ_W, PSZ_W, ULONG);
__declspec(dllexport)
LONG    UtlCompIgnSpaceW( PSZ_W pD1, PSZ_W pD2, ULONG ulLen );
__declspec(dllexport)
LONG    UtlCompareDate( FDATE *fDateFirst, FDATE *fDateSecond );
__declspec(dllexport)
LONG    UtlCompareTime( FTIME *fTimeFirst, FTIME *fTimeSecond );
__declspec(dllexport)
USHORT UtlCompFDates
(
  PSZ    pszFile1,                     // fully qualified path first file
  PSZ    pszFile2,                     // fully qualified path of second file
  PSHORT psResult,                     // address of result buffer
  BOOL   fMsg                          // error handling flag
);
__declspec(dllexport)
BOOL    UtlCopyParameter ( PSZ pszTgt, PSZ pszSrc, USHORT usSize, BOOL fMsg);
__declspec(dllexport)
USHORT  UtlQueryCharTable( CHARTABLEID TableID, PUCHAR *ppTable );
__declspec(dllexport)
USHORT  UtlQueryCharTableEx( CHARTABLEID TableID, PUCHAR *ppTable, USHORT usCodePage );
__declspec(dllexport)
USHORT  UtlQueryCharTableLang( CHARTABLEID TableID, PUCHAR *ppTable, PSZ pszLanguage );
__declspec(dllexport)
VOID    UtlSortString( PSZ );
BYTE    UtlAnsiEquivCharForDBCS1st( BYTE, ULONG ulCP );
__declspec(dllexport)
BOOL    UtlRegisterModelessDlg( HWND hwndDlg );
__declspec(dllexport)
BOOL    UtlUnregisterModelessDlg( HWND hwndDlg );
__declspec(dllexport)
BOOL    UtlIsDialogMsg( MSG FAR * pMsg );
__declspec(dllexport)
USHORT  UtlGetTask ( VOID );
__declspec(dllexport)
BOOL    UtlInsertCurDir ( PSZ , PSZ **, PSZ );
__declspec(dllexport)
BOOL isValidLanguage( PSZ pszLanguage, BOOL fAdjustLangName );
__declspec(dllexport)
BOOL GetLanguageGroup( PSZ pszLanguage, PSZ pszGroup );
__declspec(dllexport)
BOOL isSpellCheckerAvailable( PSZ pszLanguage );
__declspec(dllexport)
BOOL isMorphSupportAvailable( PSZ pszLanguage );


__declspec(dllexport)
BOOL UtlQuerySysLangFile
(
  PSZ              pszLanguage,        // language to search in file
  PSZ              pszResFile,         // buffer for name of resource file
  PSZ              pszHelpFile,        // buffer for name of help file
  PSZ              pszMsgFile          // buffer for name of message file
);
SHORT UtlFillSysLangPopUp
(
  HMENU            hMenu,              // handle of menu
  SHORT            sDummy,             // ID of dummy item
  SHORT            sID,                // ID to be used for menu items
  PSZ              pszCurLanguage      // name of currently active language
);

__declspec(dllexport)
PSZ UtlEditorLangSupport( PSZ pEditor, PSZ pSrcLang, PSZ pTgtLang );

BOOL UtlIsDriveShared( CHAR chDrive );
__declspec(dllexport)
BOOL UtlMatchStrings( PSZ pszString, PSZ pszPattern, PBOOL pfMatch );
__declspec(dllexport)
BOOL UtlMatchStringsW( PSZ_W pszString, PSZ_W pszPattern, PBOOL pfMatch );
USHORT UtlLoadConversionStrings(HWND hwndLB );  // handle of listbox receiving conversion strings
// create fully qualified file name from supplied data
USHORT UtlMakeFileName
(
  CHAR        chDrive,                 // driver letter or NULC
  PSZ         pszPath,                 // path part
  PSZ         pszName,                 // file name
  PSZ         pszExt,                  // extension or NULL if none
  PSZ         pszBuffer                // buffer for created file name
);

/*! \brief extract all files contained in a ZIP package to a directory
  \param pszPackage fully qualified name of the ZIP package
  \param pszDestPath fully qualified name of the destination directory
  \returns 0 in any case
*/
__declspec(dllexport)
int UtlUnzipToDirectory( const char * pszPackage, const char *pszDestPath );

/*! \brief add all files of a directoy to a ZIP package
  The current version of this function does not recurse into subdirectories
  \param pszSourcePath fully qualified name of the directory contaiing the files to be added to the package
  \param pszPackage fully qualified name of the ZIP package being created
  \returns 0 in any case
*/
__declspec(dllexport)
int UtlZipDirectory( const char *pszSourcePath, const char * pszPackage );

/*! \brief add a list of files to a ZIP package
  \param pszFileList list of comma separated, fully qualified file names to be added to the package
  \param pszPackage fully qualified name of the ZIP package being created
  \returns 0 in any case
*/
__declspec(dllexport)
int UtlZipFiles( const char *pszFileList, const char * pszPackage  );

/*! \brief delete all files and directories located in a specific directory
  \param pszDirectory fully qualified name of the directory contaiing the files to be added to the package
*/
__declspec(dllexport)
void UtlDeleteAllFiles( const char *pszDirectory );

// Mode defines for function UtlHandleConversionStrings
// CONVLOAD_MODE loads the names of the conversion into a listbox or combo-box
#define CONVLOAD_MODE     1
// CONVCHECK_MODE checks the supplied convsersion and gets the associated cp and flag
#define CONVCHECK_MODE    2

// conversion flags returned in pusConvFlag
#define CONVERT_USECHARSETINFO 0
#define CONVERT_TO_ANSI  1
#define CONVERT_TO_UTF8  2
#define CONVERT_TO_UTF16 3
#define CONVERT_TO_ASCII 4

__declspec(dllexport)
USHORT UtlHandleConversionStrings
(
  USHORT      usMode,                  // mode of function:
                                       //   CONVLOAD_MODE
                                       //   CONVCHECK_MODE
  HWND        hwndLB,                  // handle of listbox receiving conversion strings
                                       // only used in CONVLOAD_MODE
  PSZ         pszConversion,           // ptr to conversion being checked
                                       // only used in CONVCHECK_MODE
  PUSHORT     pusCodePage,             // ptr to buffer for code-page
                                       // only used in CONVCHECK_MODE
  PUSHORT     pusConvFlag              // ptr to buffer for conversion flag
                                       // only used in CONVCHECK_MODE
);
__declspec(dllexport)
USHORT UtlQueryCharTableForDocConv
(
  PSZ         pszDocFullName,          // full file name of segmented document
  PCHAR       *ppConvTable,             // ptr to conversion table pointer
  ULONG       ulOemCodePage
);




// ======= Defines for the function FillListBox =====================
#define  FORMAT_TABLE          1  // Identifier for the format tables
#define  EXCLUSION_LIST        2  // Identifier for the exclusion lists
#define  SOURCE_LANGUAGES      3  // Identifier for source and target languages
#define  TARGET_LANGUAGES      4  // which are not marked as disabled
#define  SERVER_CREATE_LB      5  // Identifier for server lb in new dlg
#define  SERVER_INCLUDE_LB     6  // Identifier for server lb in include dlg
#define  SERVER_MAINTAIN_LB    7  // Identifier for server lb in maintain dlg
#define  ALL_SOURCE_LANGUAGES  8  // Identifier for source and target languages
#define  ALL_TARGET_LANGUAGES  9  // including languages marked as disabled
__declspec(dllexport)
USHORT  UtlFillTableLB( HWND, USHORT );     // get languages
__declspec(dllexport)
ULONG GetLangOEMCP(PSZ pTargetLang);
__declspec(dllexport)
ULONG GetLangAnsiCP( PSZ pLanguage);

__declspec(dllexport)
USHORT SetSystemLanguage( PSZ pLanguage );
__declspec(dllexport)
PSZ    GetSystemLanguage( );
__declspec(dllexport)
USHORT UtlSetFHandState( HFILE hf, USHORT fsState, BOOL fMsg );
__declspec(dllexport)
USHORT UtlBufReset( HFILE hf, BOOL fMsg );
USHORT UtlSetFHandStateHwnd( HFILE hf, USHORT fsState, BOOL fMsg, HWND hwnd );
USHORT UtlBufResetHwnd( HFILE hf, BOOL fMsg, HWND hwnd );
BOOL   UtlAllocTrace ( PVOID *, LONG, LONG, USHORT, PSZ, USHORT );
BOOL   UtlAllocTraceHwnd ( PVOID *, LONG, LONG, USHORT, HWND, PSZ, USHORT );
#ifdef UTLFFST
  #define UtlAlloc( par1, par2, par3, par4 ) \
              UtlAllocTrace(par1, par2, par3, par4, __FILE__, __LINE__ )
  #define UtlAllocHwnd( par1, par2, par3, par4, par5 ) \
              UtlAllocTraceHwnd(par1, par2, par3, par4, par5, __FILE__, __LINE__ )

#endif
USHORT EqfDriveType(USHORT iDrive);

/**********************************************************************/
/* Flag definitions for UtlCheckPath                                  */
/**********************************************************************/
// Input Flags
#define CHKPATH_WILDCARDS       0x00000001L // wildcards in path are allowed

// Output flags describing the error condition
#define CHKPATH_PATH_IS_EMPTY   0x00000001L // path does not contain any data
#define CHKPATH_DRIVE_INVALID   0x00000002L // drive letter is invalid
#define CHKPATH_DRIVE_NOTEXIST  0x00000004L // drive does not exist
#define CHKPATH_PATH_INVALID    0x00000008L // path is invalid (i.e. path contains
                                            // invalid characters or does not
                                            // follow the 8.3 notation)
// Output flags describing the path contents
#define CHKPATH_DRIVE_SPECIFIED 0x00010000L // drive letter is part of path
#define CHKPATH_ABSOLUT_PATH    0x00020000L // path start with a backslash
#define CHKPATH_BS_AT_END       0x00040000L // path ends with a backslash
#define CHKPATH_IS_DEVICE       0x00080000L // path is a device name (e.g.
                                            // CON, LPT1: ) this flag/check
                                            // is not active yet!
__declspec(dllexport)
BOOL UtlCheckPath( PSZ pszPath, ULONG ulInFlags, PULONG pulOutFlags );
__declspec(dllexport)
BOOL UtlCheckIntName( PSZ pszName, PSZ pszOutName );
__declspec(dllexport)
BOOL UtlCheckLongName( PSZ pszName );
__declspec(dllexport)
BOOL UtlKeepInTWB( PSWP pswpWin );
__declspec(dllexport)
USHORT UtlCheckDlgPos( HWND hwndDlg, BOOL fShow );
__declspec(dllexport)
LONG UtlTime( PLONG plTime );

#ifdef UTLFFST
#define UtlAlloc( par1, par2, par3, par4 ) \
            UtlAllocTrace(par1, par2, par3, par4, __FILE__, __LINE__ )

#endif

__declspec(dllexport)
VOID UtlWait( SHORT sWaitTime );       // wait the given time
__declspec(dllexport)
USHORT UtlLoadWidth( HAB, HMODULE, SHORT, PUSHORT );

/**********************************************************************/
/* Long file name enabled versions of some Utl... functions           */
/**********************************************************************/
// structure used by long file name enabled file find functions
typedef struct _LONGFILEFIND
{
   FDATE  fdateCreation;
   FTIME  ftimeCreation;
   FDATE  fdateLastAccess;
   FTIME  ftimeLastAccess;
   FDATE  fdateLastWrite;
   FTIME  ftimeLastWrite;
   ULONG  cbFile;
   ULONG  attrFile;
   CHAR   achName[MAX_LONGFILESPEC];
} LONGFILEFIND, *PLONGFILEFIND;

__declspec(dllexport)
USHORT UtlFindFirstLong( PSZ, PHDIR, USHORT, PLONGFILEFIND, BOOL );
__declspec(dllexport)
USHORT UtlFindNextLong( HDIR, PLONGFILEFIND, BOOL );
__declspec(dllexport)
USHORT UtlFindCloseLong( HDIR, BOOL );
USHORT UtlFindFirstLongHwnd( PSZ, PHDIR, USHORT, PLONGFILEFIND, BOOL, HWND );
USHORT UtlFindNextLongHwnd( HDIR, PLONGFILEFIND, BOOL, HWND );
USHORT UtlFindCloseLongHwnd( HDIR, BOOL, HWND );
__declspec(dllexport)
SHORT  UtlLoadLongFileNames( PSZ, USHORT, HWND, USHORT );
__declspec(dllexport)
SHORT  UtlLoadLongFileNamesHwnd( PSZ, USHORT, HWND, USHORT, HWND );
__declspec(dllexport)
BOOL   UtlIsLongFileName( PSZ pszFileName );
__declspec(dllexport)
USHORT UtlLongToShortName( PSZ pszLongName, PSZ pszShortName );
__declspec(dllexport)
BOOL   UtlSupportsLongNames( CHAR chDrive );
__declspec(dllexport)
BOOL   UtlFileExistLong( PSZ pszFileName );
__declspec(dllexport)
USHORT UtlRenameObjectDlg( PSZ pszObjName, CLASSES ObjClass );
__declspec(dllexport)
BOOL   UtlPreloadIcons( void );
BOOL   UtlFreeIcons( void );


__declspec(dllexport)
USHORT UtlAddUserDlg( PSZ pszObjName, CLASSES ObjClass );

__declspec(dllexport)
USHORT UtlRemoveUserDlg( PSZ pszObjName, CLASSES ObjClass );

__declspec(dllexport)
USHORT UtlListUserDlg( PSZ pszObjName, CLASSES ObjClass );

__declspec(dllexport)
BOOL UtlGetNextFileFromCommaList
(
  PSZ  *ppszNameStart,                 // current start of file name
  PSZ  *ppszNameEnd                    // current end of file name, NULL if none yet
);

// convert a list of file names into a pointer array
__declspec(dllexport)
BOOL UtlValidateList
(
  PSZ pStart,                          // start of list
  PSZ **pppListIndex,                  // pointer to list array
  USHORT usMaxAlloc                    // max ptrs to be allocated
);

__declspec(dllexport)
ULONG UtlFileTimeToLong(PSZ pszFileName);
// new UTL functions for unicode enabling
__declspec(dllexport)
VOID UtlLoadStringW ( HAB hInst, HMODULE hmod, UINT sid, PCHAR szBuffer, PSZ_W pBufW, USHORT usBufWMax );

//#define LOADSTRING( hInst, hmod, sid, buffer)
// #define LOADSTRING( hInst, hmod, sid, buffer)   \
//    LoadString( hmod, sid, buffer, sizeof(buffer) )
//------------------------------------------------------------------------------
//   Pool definition used by the string pool functions
//------------------------------------------------------------------------------
typedef struct _POOL
{
  LONG  lSize;                       // size of pool
  LONG  lUsed;                       // used size of pool
  PSZ    pFreeChain;                   // free-block-chain anchor
  struct _POOL *pNextPool;             // ptr to next pool or NULL if none
} POOL, *PPOOL;


//------------------------------------------------------------------------------
//   Hash Table Element (View of the elements by the low level hash routines)
//------------------------------------------------------------------------------
typedef SHORT (*PFN_HASHCOMP)(PVOID obj1, PVOID obj2, PVOID pUserPtr );
typedef SHORT (*PFN_HASHFREE)(PVOID obj1, PVOID pUserPtr );
typedef SHORT (*PFN_HASHITERATE)(PVOID obj, PVOID ptr );
typedef ULONG (*PFN_HASHVALUE)(ULONG hash_size,PVOID obj);

//------------------------------------------------------------------------------
//   Hash Table Link (Hash element view of low level hash routines)
//      must be first variable in any hash element
//------------------------------------------------------------------------------
typedef struct _HASHLINK
{
  struct _HASHLINK *pNext;
} HASHLINK, *PHASHLINK;

//------------------------------------------------------------------------------
//   Hash Table Structure (used by the low level hash routines)
//------------------------------------------------------------------------------
typedef struct _HASH
{
  USHORT           usElementSize;
  ULONG            ulHashSize;
  PFN_HASHVALUE    pfnHashValue;
  PFN_HASHFREE     pfnFree;
  PFN_HASHCOMP     pfnCompare;
  PVOID            pUserPtr;           // User pointer
  PHASHLINK       *apHashElements;
} HASH, *PHASH;



//------------------------------------------------------------------------------
//   Prototypes for the string pool functions
//------------------------------------------------------------------------------
__declspec(dllexport)
PPOOL PoolCreate( LONG lPoolSize );
__declspec(dllexport)
VOID  PoolDestroy( PPOOL pPool );
__declspec(dllexport)
PSZ   PoolAddString( PPOOL pPool, PSZ pszString );
__declspec(dllexport)
PSZ_W PoolAddStringW( PPOOL pPool, PSZ_W pszString );
VOID  PoolRemoveString( PPOOL pPool, PSZ pszString );
__declspec(dllexport)
PVOID PoolAddData( PPOOL pPool, LONG lDataLen, PVOID pData );

//------------------------------------------------------------------------------
//   Prototypes for the low level hash routines
//------------------------------------------------------------------------------
__declspec(dllexport)
PHASH  HashCreate( USHORT, ULONG, PFN_HASHVALUE, PFN_HASHCOMP, PFN_HASHFREE, PVOID );
__declspec(dllexport)
BOOL   HashAdd( PHASH, PVOID );
__declspec(dllexport)
BOOL   HashDelete( PHASH pHash, PVOID pDelElement );
__declspec(dllexport)
VOID   HashDestroy( PHASH pHash );
__declspec(dllexport)
PVOID  HashSearch( PHASH, PVOID );
PVOID  HashIterate( PHASH,PFN_HASHITERATE, PVOID );
__declspec(dllexport)
BOOL   HashReset(PHASH pHash );

/**********************************************************************/
/* Replacement for OS/2 Atom table functions                          */
/* (Windows does not support multiple atom tables)                    */
/**********************************************************************/

/**********************************************************************/
/* Hash element as used by the atom table functions                   */
/**********************************************************************/
typedef struct _ATOMENTRY
{
  HASHLINK HashLink;                   // link used by low level hash routines
  CHAR     szAtomName[1];              // dynamically size buffer for atom name
} ATOMENTRY, *PATOMENTRY;

typedef struct _ATOMTBL
{
  PHASH            pHash;              // ptr to hash table for atoms
  ATOMENTRY        NewEntry;           // buffer for new entry
  CHAR             szBuffer[1024];     // buffer for new entry data
} ATOMTBL, *HATOMTBL;

typedef PVOID    EQF_ATOM;

__declspec(dllexport)
HATOMTBL APIENTRY WinCreateAtomTable( USHORT cbInitial, USHORT cBuckets );
__declspec(dllexport)
VOID     APIENTRY WinDestroyAtomTable( HATOMTBL hAtomTbl );
__declspec(dllexport)
EQF_ATOM APIENTRY WinAddAtom( HATOMTBL hAtomTbl, PSZ pszAtomName );
__declspec(dllexport)
EQF_ATOM APIENTRY WinFindAtom( HATOMTBL hAtomTbl, PSZ pszAtomName );
__declspec(dllexport)
EQF_ATOM APIENTRY WinDeleteAtom( HATOMTBL hAtomTbl, EQF_ATOM atom );
USHORT   APIENTRY WinQueryAtomName( HATOMTBL hAtomTbl, EQF_ATOM atom, PSZ pchBuffer,
                                   USHORT cchBufferMax );
__declspec(dllexport)
EQF_ATOM APIENTRY WinAddAtomW( HATOMTBL hAtomTbl, PSZ_W pszAtomNameW, ULONG ulCP );
__declspec(dllexport)
EQF_ATOM APIENTRY WinFindAtomW( HATOMTBL hAtomTbl, PSZ_W pszAtomNameW, ULONG ulCP );
//USHORT   APIENTRY WinQueryAtomNameW( HATOMTBL hAtomTbl, EQF_ATOM atom, PSZ_W pchBuffer,
//                                   USHORT cchBufferMax );


#endif

/**********************************************************************/
/* Structure for the communication between generic list window        */
/* instance procedure and the list instance callback function         */
/**********************************************************************/
typedef struct _LISTCOMMAREA
{
  HWND             hwndLB;             // R/O: handle of column listbox
                                       // value is supplied by ListInstance
  OBJNAME          szObjName;          // R/O: object name of list window
                                       // value is supplied by ListInstance
  SHORT            sListObjClass;      // R/W: class of list instance window
                                       // e.g. clsFolder
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szTitle[1024];      // R/W: title text of list window
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  HPOINTER    hIcon;                   // handle of icon for list window
                                       //   e.g. hiconFOL
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  BOOL             fNoClose;           // R/W: disable-system-menu-close flag
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sObjNameIndex;      // R/W: index of object name in list item
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sNameIndex;         // R/W: index of name in list item string
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sListWindowID;      // ID to be used for list instance window
                                       //   e.g. ID_FOLDER_WINDOW
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sListboxID;         // R/W: ID to be used for column listbox
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sPopupMenuID;       // R/W: ID of popup menu (-1 = none)
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sNoSelPopupMenuID;  // R/W: ID of popup menu when nothing
                                       // is selected in listbox  (-1 = none)
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sGreyedPopupMenuID; // R/W: ID of popup menu when selected
                                       // item is greyed out (-1 = none)
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  PCLBCTLDATA      pColData;           // R/W: column listbox control area
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            asCurView[MAX_VIEW];// R/W: current view list
                                       // is updated by list instance before
                                       // callback function is called with
                                       // WM_CLOSE and WM_EQF_TERMINATE
  SHORT            asDetailsView[MAX_VIEW]; // R/W: details view list
                                       // same handling as for asCurView
  SHORT            asSortList[MAX_VIEW]; // R/W: sort list
                                       // same handling as for asCurView
  BOOL             fMultipleSel;       // R/W: if TRUE more than one item in
                                       // the listbox can be selected
                                       // MUST be set by callback function
                                       // during WM_CREATE processing
  SHORT            sDefaultAction;     // R/W: default action to be performed
                                       // for doubleclick on a list item
                                       // e.g. PID_FILE_MI_OPEN
                                       // or 0 if no default action exists
                                       // MUST be set by callback function
                                       // during WM_CREATE processing and is
  EQF_SWP          swpSizePos;         // R/W: size and position of list window
                                       // MUST be set by callback function
                                       // during WM_CREATE processing and is
                                       // updated by list instance before
                                       // callback function is called with
                                       // WM_CLOSE and WM_EQF_TERMINATE
  SHORT            sItemClass;         // R/W: object class of items in the
                                       // listbox; e.g clsDOCUMENT
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sItemPropClass;     // R/W: property class of items in the
                                       // listbox; e.g PROP_CLASS_DOCUMENT
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  PVOID            pUserIDA;           // R/W: ptr to IDA of callback function
                                       // use this variable to anchor the IDA
                                       // of the callback function
  BOOL             fUserFlag;          // R/W: user flag
                                       // not used by generic list instance
  SHORT            asMsgsWanted[20];   // R/W: additional messages to be passed
                                       // to the callback function, list nust
                                       // be terminated by a zero value
  CHAR             szBuffer[4096];     // R/W: buffer for temporary values of
                                       // the callback function and to return
                                       // the text for listbox items
  BOOL             fInvisible;         // R/W: create-an-invisibke-list window
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CLBFILTER        Filter;             // R/W: filter for listbox items
  SHORT            sMultPopupMenuID;   // R/W: ID of popup menu (-1 = none)
                                       // when more than one item is selected
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing

  SHORT            asCurViewWidth[MAX_VIEW]; // R/W: details view list
} LISTCOMMAREA, FAR *PLISTCOMMAREA;

/**********************************************************************/
/* type definition for list instance callback function                */
/**********************************************************************/
typedef MRESULT (*PFN_LISTCALLBACK)( PLISTCOMMAREA, HWND,
                                                WINMSG, WPARAM, LPARAM );
__declspec(dllexport)
  MRESULT FolderCallBack( PLISTCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport)
  MRESULT FolderListCallBack( PLISTCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
/**********************************************************************/
/* Defines for the layout of the process window                       */
/**********************************************************************/
typedef enum _PROCWINSTYLE
{
  PROCWIN_TEXTONLY,                    // Process window contains a single
                                       // text line (e.g. folder import );
  PROCWIN_SLIDERONLY,                  // Process window contains a slider
                                       // control (e.g. folder export)
  PROCWIN_TEXTSLIDER,                  // Process window contains a text line
                                       // and a slider control (e.g. tm export)
  PROCWIN_SLIDERENTRY,                 // Process window contains a slider
                                       // control and a groupbox containing a
                                       // text line
                                       // for the current entry (e.g. dict export)
  PROCWIN_TEXTSLIDERENTRY,             // Process window contains a text line
                                       // and a slider control (e.g. tm export)
                                       // and a groupbox containing a text line
                                       // for the current entry (e.g. dict export)
  PROCWIN_TEXTSLIDERLISTBOX,           // Process window contains a text line
                                       // and a slider control (e.g. tm export)
                                       // and a two listboxes (e.g. analysis)
  PROCWIN_BATCH                        // Process window is invisible (for use
                                       // in batch mode processes)
} PROCWINSTYLE;

/**********************************************************************/
/* Structure for the communication between generic process window     */
/* instance procedure and the process instance callback function      */
/**********************************************************************/
typedef struct _PROCESSCOMMAREA
{
  /********************************************************************/
  /* Variables independent of the type of the process window          */
  /********************************************************************/
  CHAR             szObjName[1024];    // R/O: object name of process window
                                       // value is supplied by ProcessInstance
  PROCWINSTYLE     Style;              // R/W: style of the process window
                                       // for a description of the styles see
                                       // PROCWINSTYLE enumeration
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sProcessObjClass;   // R/W: class of process instance window
                                       // e.g. clsFOLDEREXP
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sProcessWindowID;   // R/W: ID to be used for process
                                       // instance window; e.g. ID_FOLEXP_WINDOW
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szTitle[1024];      // R/W: title text of list window
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  HPOINTER         hIcon;              // handle of icon for process window
                                       //   e.g. hiconFOLEXP
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  BOOL             fNoClose;           // R/W: disable-system-menu-close flag
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  BOOL             fDoNotRegisterObject;// R/W: do-not-register-object flag
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
                                       // if this flag is set the generic
                                       // process window procedure will not
                                       // register the process window, is it
                                       // up to the callback function to
                                       // register it
  SWP              swpSizePos;         // R/W: size and position of list window
                                       // MUST be set by callback function
                                       // during WM_CREATE processing
  PVOID            pUserIDA;           // R/W: ptr to IDA of callback function
                                       // use this variable to anchor the IDA
                                       // of the callback function
  BOOL             fUserFlag;          // R/W: user flag
                                       // not used by generic list instance
  SHORT            asMsgsWanted[20];   // R/W: additional messages to be passed
                                       // to the callback function, list nust
                                       // be terminated by a zero value
  CHAR             szBuffer[1024];     // R/W: buffer for temporary values of
                                       // the callback function

  /********************************************************************/
  /* Variables not used for style PROCWIN_SLIDERONLY                  */
  /********************************************************************/
  SHORT            sTextID;            // R/W: ID to be used for text line
                                       // control
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szText[1024];       // R/W: text to be displayed in text
                                       // line of the the process window
  CHAR             szText2[1024];      // R/W: text to be displayed in the
                                       // second text line of the the process window

  /********************************************************************/
  /* Variables not used for style PROCWIN_TEXTONLY                    */
  /********************************************************************/
  SHORT            sSliderID;          // R/W: ID to be used for slider
                                       // control
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  USHORT           usComplete;         // R/W: current completion rate
                                       // value must be in the range
                                       // from 0 to 100

  /********************************************************************/
  /* Variables only used for style PROCWIN_TEXTSLIDERENTRY            */
  /********************************************************************/
  SHORT            sEntryGBID;         // R/W: ID to be used for entry groupbox
                                       // control
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szGroupBoxTitle[80];// R/W: text to be displayed as label
                                       // of the groupbox for process windows
                                       // of style PROCWIN_TEXTSLIDERENTRY
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sEntryID;           // R/W: ID to be used for entry text
                                       // line control
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szEntry[80];        // R/W: text to be displayed in the
                                       // entry text line of the the process
                                       // window

  /********************************************************************/
  /* Variables only used for style PROCWIN_TEXTSLIDERLISTBOX          */
  /********************************************************************/
  HWND             hwndLB1;            // R/O: handle of the first listbox for
                                       // process windows of type
                                       // PROCWIN_TEXTSLIDERLISTBOX
  HWND             hwndLB2;            // R/O: handle of the second listbox for
                                       // process windows of type
                                       // PROCWIN_TEXTSLIDERLISTBOX
  SHORT            sLB1TextID;         // R/W: ID to be used for the label
                                       // of the first listbox
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sLB2TextID;         // R/W: ID to be used for the label
                                       // of the second listbox
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sLB1ID;             // R/W: ID to be used for the first
                                       // listbox
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sLB2ID;             // R/W: ID to be used for the second
                                       // listbox
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szLB1Text[80];      // R/W: text to be displayed as label
                                       // for the first listbox
  CHAR             szLB2Text[80];      // R/W: text to be displayed as label
                                       // for the second listbox
} PROCESSCOMMAREA, FAR *PPROCESSCOMMAREA;

/**********************************************************************/
/* type definition for process instance callback function             */
/**********************************************************************/
typedef MRESULT (*PFN_PROCESSCALLBACK)( PPROCESSCOMMAREA, HWND,
                                                 WINMSG, WPARAM, LPARAM );

/**********************************************************************/
/* Structure for the communication between generic list window        */
/* handler and the handler callback function                          */
/**********************************************************************/
typedef struct _HANDLERCOMMAREA
{
  PFN_LISTCALLBACK pfnCallBack;        // ptr to list instance callback function
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  OBJNAME     szHandlerName;           // name of this handler
                                       //   e.g. FOLDERHANDLER
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT       sBaseClass;              // base class processed by this handler
                                       //   e.g. clsFOLDER
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT       sListWindowID;           // ID to be used for list instance window
                                       //   e.g. ID_FOLDER_WINDOW
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT       sListboxID;              // ID to be used for column listbox
                                       //   e.g. ID_FOLDER_LB
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT       asNotifyClassList[11];   // list of object classes to be notified
                                       // for WM_EQFN_ messages, the list has
                                       // to be terminated by a zero value
                                       //  e.g { clsFOLDEREXP, clsFOLDERIMP, 0 }
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  BOOL             fUserFlag;          // R/W: user flag
                                       // not used by generic handler function
                                       // can be used by callback function as
                                       // 'must not close' flag or for any other
                                       // purpose
  SHORT            asMsgsWanted[20];   // R/W: additional messages to be passed
                                       // to the callback function, list nust
                                       // be terminated by a zero value
  CHAR             szBuffer[1024];     // R/W: buffer for temporary values of
                                       // the callback function
  PVOID            pUserIDA;           // R/W: ptr to IDA of callback function
                                       // use this variable to anchor the IDA
                                       // of the callback function
} HANDLERCOMMAREA, FAR *PHANDLERCOMMAREA;


/**********************************************************************/
/* type definition for handler callback function                      */
/**********************************************************************/
typedef MRESULT (*PFN_HANDLERCALLBACK)( PHANDLERCOMMAREA, HWND,
                                                   WINMSG, WPARAM, LPARAM );

/**********************************************************************/
/* Create handler and create list window information structure        */
/**********************************************************************/
typedef struct _GENHANDLERCREATEPARMS
{
  USHORT      usSize;                  // overall size of structure
  PFN_HANDLERCALLBACK pfnCallBack;     // ptr to handler callback structure
} GENHANDLERCREATEPARMS, *PGENHANDLERCREATEPARMS;



typedef char VENDORNAME[MAX_DESCRIPTION];
typedef VENDORNAME *PVENDORNAME;

typedef char VENDORMAIL[MAX_DESCRIPTION];
typedef VENDORMAIL *PVENDORMAIL;

typedef CHAR_W FINDNAME[MAX_FINDCHANGE_LEN+1];
typedef FINDNAME *PFINDNAME;








/**********************************************************************/
/* Prototype for create process window function                       */
/**********************************************************************/
__declspec(dllexport)
BOOL CreateProcessWindow
(
  PSZ                  pszObjName,     // ptr to object name for process window
  PFN_PROCESSCALLBACK  pfnCallBack,    // callback function for process window
  PVOID                pvUserData      // ptr to user data (is passed to callback
                                       // function in mp2 of WM_CREATE message)
);
__declspec(dllexport)
BOOL CreateProcessWindow2
(
  PSZ                  pszObjName,     // ptr to object name for process window
  PFN_PROCESSCALLBACK  pfnCallBack,    // callback function for process window
  PVOID                pvUserData,     // ptr to user data (is passed to callback
                                       // function in mp2 of WM_CREATE message)
  BOOL                 fVisible        // create-a-visible-process-window flag
);

/**********************************************************************/
/* Prototype for create list window function                          */
/**********************************************************************/
__declspec(dllexport)
BOOL CreateListWindow
(
  PSZ                  pszObjName,     // ptr to object name for process window
  PFN_LISTCALLBACK     pfnCallBack,    // callback function for list window
  PVOID                pvUserData,     // ptr to user data (is passed to callback
                                       // function in mp2 of WM_CREATE message)
  BOOL                 fRestart        // TRUE = restarted by TWB
);


typedef struct _GENHANDLERIDA
{
  HWND             hFrame;             // handle of frame window
  HANDLERCOMMAREA  CommArea;           // area for communication between handler
                                       // and handler callback function
  PFN_HANDLERCALLBACK pfnCallBack;     // ptr to callback function of handler
} GENHANDLERIDA, FAR *PGENHANDLERIDA;


typedef struct _GENLISTINSTIDA
{
  HWND             hFrame;             // handle of frame window
  PFN_LISTCALLBACK pfnCallBack;        // ptr to callback function of instance
  LISTCOMMAREA     CommArea;           // area for communication between list
                                       // instance and callback function
  BOOL             fWantsPropMsg;      // TRUE = pass thru WM_EQFN_PROPERTIESCHANGED msg
  BOOL             fWantsDelMsg;       // TRUE = pass thru WM_EQFN_DELETED msg
  BOOL             fWantsCreateMsg;    // TRUE = pass thru WM_EQFN_CREATED msg
  BOOL             fWantsSelectedNamesMsg;  // TRUE = pass thru WM_EQF_QUERYSELECTEDNAMES msg
  PVOID            pvUserData;         // ptr to optional user creation data
  BOOL             fRestart;           // TRUE = restarted by TWB
  OBJNAME          szSearchName;       // buffer for searched object name
} GENLISTINSTIDA, *PGENLISTINSTIDA;

typedef struct _GENPROCESSINSTIDA
{
  HWND             hFrame;             // handle of frame window
  HWND             hwndSlider;         // handle of slider window
  HWND             hwndText;           // handle of text line control
  HWND             hwndText2;          // handle of second text line control
  HWND             hwndGB;             // handle of entry groupbox
  HWND             hwndGBText;         // handle of entry groupbox text (Windows)
  SHORT            sGBTextWidth;       // width of groupbox text in pels
  HWND             hwndEntry;          // handle of entry text line
  HWND             hwndLB1Text;        // handle of first listbox header
  HWND             hwndLB2Text;        // handle of second listbox header
  HWND             hwndRect;           // handle of groubbox background rectangle
  PVOID            pvUserData;         // ptr to optional user creation data
  PFN_PROCESSCALLBACK pfnCallBack;     // ptr to callback function of instance
  USHORT           usComplete;         // current slider arm position
  CHAR             szCurTitle[80];     // currently displayed title text
  CHAR             szCurText[80];      // currently displayed slider text
  CHAR             szCurText2[80];     // currently displayed slider text 2
  CHAR             szCurEntry[80];     // currently displayed entry text
  CHAR             szBuffer[160];      // buffer for OEM conversion
  BOOL             fVisible;           // create-a-visible-process-window flag
  PROCESSCOMMAREA  CommArea;           // area for communication between
                                       // process instance and callback function
  BOOL             fWPStatic;          // WordPerfect static field
} GENPROCESSINSTIDA, *PGENPROCESSINSTIDA;

typedef struct _PROCESSCREATEPARMS
{
  PVOID            pvUserData;         // ptr to user data to be passed to
                                       // callback function
  PFN_PROCESSCALLBACK pfnCallBack;     // ptr process callback function
  BOOL                fVisible;        // create-a-visible-process-window flag
  SHORT            sClassId;          // list class to be generated
} PROCESSCREATEPARMS, *PPROCESSCREATEPARMS;


typedef struct _LISTCREATEPARMS
{
  PVOID            pvUserData;         // ptr to user data to be passed to
                                       // callback function
  PFN_LISTCALLBACK pfnCallBack;        // ptr list callback function
  BOOL                 fRestart;       // TRUE = restarted by TWB
} LISTCREATEPARMS, *PLISTCREATEPARMS;



BOOL CreateProcessControls( HWND hwnd, PGENPROCESSINSTIDA  pIda, WPARAM mp1 );
void CheckForUpdate( HWND hwnd, PGENPROCESSINSTIDA  pIda );
VOID PositionProcessControls( PGENPROCESSINSTIDA  pIda,
                              SHORT sHeigth, SHORT sSize  );


/**********************************************************************/
/* Procedure to get or refresh the commarea pointer for a generic     */
/* list window or a generic process window (e.g. adfter a call to     */
/* UtlDispatch)                                                       */
/*                                                                    */
/* If NULL is returned either the list window or the process window   */
/* has been destroyed or it's data areas have been freed              */
/**********************************************************************/
PVOID AccessGenListCommArea( HWND hwnd );

__declspec(dllexport)
PVOID AccessGenProcCommArea( HWND hwnd );

/**********************************************************************/
/* Prototype for popup window function                                */
/**********************************************************************/
__declspec(dllexport)
void HandlePopupMenu( HWND hwnd, POINT point, SHORT sMenuID );

/**********************************************************************/
/* Prototype for activate MDI child window function                   */
/**********************************************************************/
__declspec(dllexport)
BOOL ActivateMDIChild
(
  HWND                 hwndChild       // handle of window being activated
);



__declspec(dllexport) MRESULT FolderHandlerCallBack( PHANDLERCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT FolderListHandlerCallBack( PHANDLERCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT DictListHandlerCallBack( PHANDLERCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT TagListHandlerCallBack( PHANDLERCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT MemListHandlerCallBack( PHANDLERCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT LngListHandlerCallBack( PHANDLERCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT WCntListHandlerCallBack( PHANDLERCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT ListHandlerCallBack( PHANDLERCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
#ifdef _TQM
  MRESULT TQMListHandlerCallBack( PHANDLERCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
#endif

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/* Error Base IDs                                                     */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
#define  Err_NoError          0        // no error occured

/* Error Base Values */
#define  Err_Eqfstart      1000
#define  Err_Prop          2000        // property handler rc
#define  Err_ObjM          2100        // object manager rc
#define  Err_Nfol          2200        // new folder messages
#define  ERR_BTREE_BASE    5000
#define  ERR_BTREE_END     5999
#define  ERR_MEM_BASE      6000
#define  ERR_MEM_END       6999
#define  ERR_MORPH_BASE    7000
#define  ERR_MORPH_END     7999
#define  ERR_DICT_BASE     8000
#define  ERR_DICT_END      8999
#define  ERR_PRINT_BASE    9000
#define  ERR_PRINT_END     9999
#define  ERR_COUNT_BASE   10000
#define  ERR_COUNT_END    10100
#define  QLDB_ERR_BASE    10500
#define  QLDB_ERR_END     11000

/* General error message IDs */

#define Err_NoDiskSpace        (Err_Eqfstart+01) // too less disk space
#define Err_NoStorage          (Err_Eqfstart+02) // no more storage
#define Err_System             (Err_Eqfstart+03) // unexpected error
#define Err_OpenFile           (Err_Eqfstart+04) // error opening file
#define Err_ReadFile           (Err_Eqfstart+05) // error reading file
#define Err_DeleteFile         (Err_Eqfstart+06) // cannot delete file
#define Err_WriteFile          (Err_Eqfstart+07) // error writing to file
#define Err_HandlerExists      (Err_Eqfstart+08) // handler already installed


/* Object manager error message IDs */

#define ErrObjM_AlreadyReg     (Err_ObjM+ 1) // already registered
#define ErrObjM_CreateEntry    (Err_ObjM+ 2) // cannot create tbl entry (nostor?)
#define ErrObjM_NoHandler      (Err_ObjM+ 3) // no anchor found for class

/* Properties handler error message IDs */

#define ErrProp_AccessDenied   (Err_Prop+ 1) // wrong access to properties
#define ErrProp_AlreadyDefined (Err_Prop+ 2) // symbolic name already defined
#define ErrProp_AlreadyExists  (Err_Prop+ 3) // properties already exist
#define ErrProp_InvalidClass   (Err_Prop+ 4) // invalid properties class
#define ErrProp_InvalidData    (Err_Prop+ 5) // invalid properties data
#define ErrProp_InvalidEntry   (Err_Prop+ 6) // invalid entry index
#define ErrProp_InvalidFile    (Err_Prop+ 7) // invalid file detected
//
#define ErrProp_InvalidName    (Err_Prop+ 9) // symbolic name invalid
#define ErrProp_InvalidObjectName (Err_Prop+10) // object name invalid
#define ErrProp_InvalidType    (Err_Prop+11) // properties type invalid
#define ErrProp_NoEnvironment  (Err_Prop+12) // properties environment not active
#define ErrProp_NoName         (Err_Prop+13) // invalid handles
#define ErrProp_NoSystem       (Err_Prop+14) // system properties not located
#define ErrProp_NotAuthorized  (Err_Prop+15) // properties of class not created
#define ErrProp_NotFound       (Err_Prop+16) // property name not located
#define ErrProp_ObjectBusy     (Err_Prop+17) // objects busy
#define ErrProp_SystemInvalid  (Err_Prop+18) // system properties invalid
#define ErrProp_InvalidAccess  (Err_Prop+19) // access flg invalid
#define ErrProp_MissingBuffer  (Err_Prop+20) // buffer addr missing
#define ErrProp_NotLoaded      (Err_Prop+21) // prop load error
#define ErrProp_InvalidParms   (Err_Prop+22) // invalid parm specified
#define ErrProp_InvalidHandle  (Err_Prop+23) // invalid handle to properties

#define ErrNfol_NoName         (Err_Nfol+ 1) // missing folder name
#define ErrNfol_NoDescription  (Err_Nfol+ 2) // missing folder description
#define ErrNfol_NoFormat       (Err_Nfol+ 3) // missing documents format
#define ErrNfol_InvalidFormat  (Err_Nfol+ 4) // format specified is wrong
#define ErrNfol_NoMemory       (Err_Nfol+ 5) // no memory db specified
#define ErrNfol_NoExlist       (Err_Nfol+ 6) // no excusion list specified
#define ErrNfol_NameExists     (Err_Nfol+ 7) // folder name already exists
#define ErrNfol_DicsTooMany    (Err_Nfol+ 8) // too many dics selected
#define ErrNfol_DicsTooLess    (Err_Nfol+ 9) // too less dics selected
#define ErrNfol_SubsTooMany    (Err_Nfol+10) // too many dics selected
#define ErrNfol_SubsTooLess    (Err_Nfol+11) // too less dics selected
#define ErrNfol_NoSLang        (Err_Nfol+12) // no source language specified
#define ErrNfol_NoTLang        (Err_Nfol+13) // no target language specified
#define ErrNfol_NoSCodp        (Err_Nfol+14) // no source code page specified
#define ErrNfol_NoTCodp        (Err_Nfol+15) // no target code page specified


#define BTREE_BASE          ERR_BTREE_BASE        //
#define BTREE_NO_ROOM       BTREE_BASE+1          // Insufficent memory
#define BTREE_ILLEGAL_FILE  BTREE_BASE+2          // Not an index file
#define BTREE_DUPLICATE_KEY BTREE_BASE+3          // Key is not unique
#define BTREE_NO_BUFFER     BTREE_BASE+4          // No buffer available
#define BTREE_NOT_FOUND     BTREE_BASE+5          // Key not found
#define BTREE_INVALID       BTREE_BASE+6          // Btree pointer invalid
#define BTREE_READ_ERROR    BTREE_BASE+7          // read error on file
#define BTREE_CORRUPTED     BTREE_BASE+8          // binary tree is corrupted
#define BTREE_BUFFER_SMALL  BTREE_BASE+9          // buffer to small for data
#define BTREE_DISK_FULL     BTREE_BASE+10         // disk is full
#define BTREE_USERDATA      BTREE_BASE+11         // user data too large
#define BTREE_EOF_REACHED   BTREE_BASE+12         // eof or start reached
#define BTREE_EMPTY         BTREE_BASE+13         // no entries in dictionary
#define BTREE_WRITE_ERROR   BTREE_BASE+14         // error during write
#define BTREE_OPEN_ERROR    BTREE_BASE+15         // error during open
#define BTREE_CLOSE_ERROR   BTREE_BASE+16         // error during close
#define BTREE_NUMBER_RANGE  BTREE_BASE+17         // number out of range
#define BTREE_INV_SERVER    BTREE_BASE+18         // invalid server name
#define BTREE_READONLY      BTREE_BASE+19         // opened for read/only
#define BTREE_DATA_RANGE    BTREE_BASE+20         // data length out of range
#define BTREE_DICT_LOCKED   BTREE_BASE+21         // dictionary is locked
#define BTREE_ENTRY_LOCKED  BTREE_BASE+22         // entry is locked
#define BTREE_MAX_DICTS     BTREE_BASE+23         // max. number of dicts reached
#define BTREE_NO_EXCLUSIVE  BTREE_BASE+24         // no excl. use possible
#define BTREE_FILE_NOTFOUND BTREE_BASE+25         // file not found
#define BTREE_ACCESS_ERROR  BTREE_BASE+26         // access error
#define BTREE_INVALID_DRIVE           BTREE_BASE+27
#define BTREE_OPEN_FAILED             BTREE_BASE+28
#define BTREE_NETWORK_ACCESS_DENIED   BTREE_BASE+29
#define BTREE_LOCK_ERROR              BTREE_BASE+30  // locking failed
#define BTREE_IN_USE                  BTREE_BASE+31  // data is locked/modified
#define BTREE_INVALIDATED             BTREE_BASE+32  // data has been invalidated
#define BTREE_LOOKUPTABLE_NULL        BTREE_BASE+33  // pointer to lookup table is null
#define BTREE_LOOKUPTABLE_CORRUPTED   BTREE_BASE+34  // lookup table is corrupted
#define BTREE_LOOKUPTABLE_TOO_SMALL   BTREE_BASE+35  // lookup table is too small, file is too big

#define HPROP PVOID


// define property classes
// if changes are required to this definition change modules which
// relay on $PROP_CLASS too, e.g. EQFPRO00.C !!
typedef enum {
   PROP_CLASS_SYSTEM,
   PROP_CLASS_FOLDERLIST,
   PROP_CLASS_FOLDER,
   PROP_CLASS_DOCUMENT,
   PROP_CLASS_IMEX,
   PROP_CLASS_MEMORY,
   PROP_CLASS_MEMORYDB,
   PROP_CLASS_MEMORY_LASTUSED,
   PROP_CLASS_EDITOR,                  // editor properties
   PROP_CLASS_DICTLIST,                // dictionary instance window properties
   PROP_CLASS_TAGTABLE,                // tag table instance window properties
   PROP_CLASS_DICTIONARY,              // tag table instance window properties
   PROP_CLASS_LIST,                    // list handler instance window properties
   PROP_CLASS_TQMLIST,                 // TQM list handler instance window properties
   PROP_CLASS_END                      // always the last element
} PROPCLASS;

#define PROP_TYPE_INSTANCE     'I'
#define PROP_TYPE_TEMPLATE     'T'
#define PROP_TYPE_NEW          'N'


// Access/close formats to object properties
#define PROP_ACCESS_READ        0x01
#define PROP_ACCESS_WRITE       0x02
#define PROP_ACCESS_READWRITE  (PROP_ACCESS_READ | PROP_ACCESS_WRITE)
#define PROP_STATUS_UPDATED     0x08
#define PROP_QUIT               0x00
#define PROP_FILE               0x10


// Structure of properties heading area
typedef struct _PROPHEAD
{
   char szName[MAX_FILESPEC];   // Properties name (= object name)
   char szPath[MAX_PATH144];    // Path to properties
   USHORT usClass;              // Class name ID of properties
   char chType;                 // Indicator to type of properties
                                // T=Template, I=Instance, N=New
} PROPHEAD, *PPROPHEAD;

// list of internal code versions and current code version

// pre-TP5522 code version
#define CODE_VERSION_000   0

// stem form cache files have to be deleted, they contain corrupted entries
#define CODE_VERSION_001   2

// currend code version level
#define CURRENT_CODE_VERSION CODE_VERSION_001

// defines for XSLT engines to be used
#define XSLT_USE_XSLT4C   0
#define XSLT_USE_MSXML    1

#define XSLT_XSLT4C_DLL "EQFXSLIBM.DLL"
#define XSLT_MSXML_DLL  "EQFXSLMS.DLL"

// Structure of EQF system properties
typedef struct _PROPSYSTEM
{
   //-------------------------- property header --------------------------------
   PROPHEAD  PropHead;                      // common property header
   //------------------ drive and path information -----------------------------
   CHAR szPrimaryDrive[MAX_DRIVE];          // primary TWB drive( e.g. "X:\0")
   CHAR szDriveList[MAX_DRIVELIST];         // list of EQF drives
   CHAR szPropertyPath[MAX_FILESPEC];       // directory for properties
   CHAR szProgramPath[MAX_FILESPEC];        // directory for program files
   CHAR szDicPath[MAX_FILESPEC];            // directory for dictionaries
   CHAR szMemPath[MAX_FILESPEC];            // directory for memory data bases
   CHAR szTablePath[MAX_FILESPEC];          // directory for translation tables
   CHAR szListPath[MAX_FILESPEC];           // directory for lists
   CHAR szExportPath[MAX_FILESPEC];         // directory for exported files
   CHAR szBackupPath[MAX_FILESPEC];         // directory for backup files
   CHAR szDirSourceDoc[MAX_FILESPEC];       // directory for source documents
   CHAR szDirSegSourceDoc[MAX_FILESPEC];    // directory for segm. source docs
   CHAR szDirSegTargetDoc[MAX_FILESPEC];    // directory for segm. target docs
   CHAR szDirTargetDoc[MAX_FILESPEC];       // directory for target docs
   CHAR szDirDocWordLists[MAX_FILESPEC];    // directory for document lists
   CHAR szDllPath[MAX_FILESPEC];            // directory for DLLs
   CHAR szMsgPath[MAX_FILESPEC];            // directory for msg and help files
   CHAR szCtrlPath[MAX_FILESPEC];           // directory for control information
   //----------------------- restart information -------------------------------
   CHAR RestartFolderLists[ 128];           // list of folderlists for restart
   CHAR RestartFolders[ 256];               // list of folder for restart
   EQF_SWP  SwpDef;                         // TWB default window size
   EQF_SWP  Swp;                            // TWB window position
   CHAR RestartMemory[ 256];                // list of mem windows for restart
   CHAR RestartDicts[ 256];                 // list for dict windows for restart
   OBJNAME FocusObject;                     // object which had the focus
   CHAR RestartTagTables[ 128];             // list of tag tables for restart
   CHAR szDefaultEditor[MAX_FILESPEC];      // default editor
   //---------------------- TM server code values ------------------------------
   CHAR szServerList[MAX_SERVER_LIST_SIZE]; // list of servers
   //------------------ drive and path information (2) -------------------------
   CHAR szDirComMem[MAX_FILESPEC];          // directory for server TMs
   CHAR szDirComProp[MAX_FILESPEC];         // directory for server TM props
   CHAR szDirImport[MAX_FILESPEC];          // directory for imported files
   //----------------------- restart information (2) ---------------------------
   CHAR RestartLists[ 256];                 // list of list handlers for restart
   CHAR RestartDocs[ 1000];                 // list of documents for restart
   //----------------------- new paths and directories -------------------------
   CHAR szPrtPath[MAX_FILESPEC];            // directory for print formats
   CHAR szDirComDict[MAX_FILESPEC];         // directory for server Dicts
   CHAR szWinPath[MAX_FILESPEC];            // directory for Windows DLLs
   //------------------------- general options ---------------------------------
   EQF_BOOL  fFolImpNoDocExistsQuery;       // FolImp no query for existing docs
   EQF_BOOL  fReserveOption01;              // reserve option
   EQF_BOOL  fReserveOption02;              // reserve option
   EQF_BOOL  fReserveOption03;              // reserve option
   EQF_BOOL  fReserveOption04;              // reserve option
   EQF_BOOL  fReserveOption05;              // reserve option
   EQF_BOOL  fReserveOption06;              // reserve option
   EQF_BOOL  fReserveOption07;              // reserve option
   EQF_BOOL  fReserveOption08;              // reserve option
   EQF_BOOL  fReserveOption09;              // reserve option
   EQF_BOOL  fReserveOption10;              // reserve option
   EQF_BOOL  fReserveOption11;              // reserve option
   EQF_BOOL  fReserveOption12;              // reserve option
   EQF_BOOL  fReserveOption13;              // reserve option
   EQF_BOOL  fReserveOption14;              // reserve option
   EQF_BOOL  fReserveOption15;              // reserve option
   EQF_BOOL  fReserveOption16;              // reserve option
   EQF_BOOL  fReserveOption17;              // reserve option
   EQF_BOOL  fReserveOption18;              // reserve option
   EQF_BOOL  fReserveOption19;              // reserve option
   //---------------------------------------------------------------------------
   CHAR szEADataPath[MAX_FILESPEC];         // directory for EA data
   //----------------------------- LAN drive letter ----------------------------
   CHAR szLanDrive[MAX_DRIVE];    // primary TWB LAN drive( e.g. "X:\0")
   //-------------------- values in system property dialog ---------------------
   EQF_BOOL  fNoShutDownConf;               // no shutdown confirmation flag
   EQF_BOOL  fNoDeleteConf;                 // no delete confirmation flag
   EQF_BOOL  fSaveAlways;                   // always save workbench during shutdown
   EQF_BOOL  fUseSymbolicMarkupNames;       // use symbolic markup names flag
   EQF_BOOL  fNoGenericMarkup;              // suppress generic markup handling
   CHAR      szWebBrowser[MAX_LONGPATH];    // buffer for web browser name/path

   //---------------------------- TQM stuff --------------------------------------
   CHAR      szTQMProjectPath[MAX_EQF_PATH];// name of TQM project directory
   CHAR      szTQMEvalPath[MAX_EQF_PATH];   // name of TQM evalutation profile directory
   CHAR      szTQMArchivePath[MAX_EQF_PATH];// name of TQM archived projects directory
   CHAR      szTQMReportPath[MAX_EQF_PATH]; // name of TQM report directory
   CHAR      szTQMVendorPath[MAX_EQF_PATH]; // name of TQM vendor directory

   //-------------------- more values in system property dialog ---------------------
   EQF_BOOL  fUseIELikeListWindows;         // use IE like folder list

   // currently active fuzziness levels with two desimal places
   // (e.g. 50 is stored as 5000)
   LONG         lSmallFuzzLevel;       // fuzzines level for small segments
   LONG         lMediumFuzzLevel;      // fuzzines level for medium sized segments
   LONG         lLargeFuzzLevel;       // fuzzines level for large segments

   // internal code version number, used to special preprocessing if a new
   // code version is about to be started for the first time (e.g. delete
   // old stem form cache files if format haas been changed)
   LONG         lCodeVersion;          // current EQFD/EQFDLL code version
   CHAR         szSystemPrefLang[MAX_LANG_LENGTH]; // System Preferences LANGUAGE
   ULONG        ulSystemPrefCP;

   CHAR         RestartMTList[80];    // list of MT job lists to restart (only one!)
   LONG         lSmallLkupFuzzLevel;       // fuzzines level for small segments(1LONG=3CHAR)
   LONG         lMediumLkupFuzzLevel;      // fuzzines level for medium sized segments
   LONG         lLargeLkupFuzzLevel;       // fuzzines level for large segments

   SHORT  		sCalcReportWinX;
   SHORT        sCalcReportWinY;
   SHORT        sCalcReportWinCX;
   SHORT        sCalcReportWinCY;
   SHORT        sLeftPaneWidth;           // P021036

   EQF_BOOL     fNoSgmlDitaProcessing;// true = disable preprocessing of IDDOC proposals for DITA documents
   SHORT        sXSLTEngine;          // XSLT engine to be used
   EQF_BOOL     fEntityProcessing;    // true = perform special processing for IDDOC and DITA entities

   CHAR szPluginPath[MAX_FILESPEC];   // directory for plugins

   USHORT       usMemImpMrkupAction;  // Action for unknown markup: 0=Abort, 1=Skip, 2=Use default
   USHORT       usHideToolBars;       // Whether to hide TWB and translation environment tool bars

   //----------------------------- filler --------------------------------------
   CHAR         chReserve[240];       //  reserve space/ fills props to 4K)
} PROPSYSTEM, *PPROPSYSTEM;


// Structure of EQF folderlist properties
// currently saved in property\flist.l00
// GQ 2015/07/08 beginning with OpenTM2 1.2.3 global change and find uses a private file for the last used values, fields marked with /*OLD*/ are not used anymore
// 
typedef struct _PROPFOLDERLIST
{
   //-------------------------- property header --------------------------------
   PROPHEAD  PropHead;                      // Common header of properties
   //------------- last used values of folder list window ----------------------
   EQF_SWP    Swp;                          // Window position
   BYTE   szDriveList[MAX_DRIVELIST];       // List of drive letters
   SHORT  sLastUsedViewList[MAX_VIEW];      // last used view list
   SHORT  sDetailsViewList[MAX_VIEW];       // user's details view list
   SHORT  sSortList[MAX_VIEW];              // user's sort criteria list
   //----------------- last used values of folder export -----------------------
   CHAR chFolExpDrive;          // target drive for export
   EQF_BOOL fFolExpWithDict;    // 'With dictionary' checkbox flag
   EQF_BOOL fFolExpWithMem;     // 'With translation memory' checkbox flag
   EQF_BOOL fFolExpAddNote;     // 'Add note' checkbox flag
   EQF_BOOL fFolExpDelFolder;   // 'Delete folder' checkbox flag
   //----------------- last used values of folder import -----------------------
   CHAR chFolImpFromDrive;      // source drive for import
   CHAR chFolImpToDrive;        // target drive for import
   EQF_BOOL fFolImpWithDict;    // 'With dictionary' checkbox flag
   EQF_BOOL fFolImpWithMem;     // 'With translation memory' checkbox flag
   //------------------ last used values of folder languages -------------------
   CHAR      szSourceLang[MAX_LANG_LENGTH]; // FOLDER SOURCE LANGUAGE
   CHAR      szTargetLang[MAX_LANG_LENGTH]; // FOLDER TARGET LANGUAGE
   //----------------- last used values of folder export -----------------------
   EQF_BOOL fFolExpOldVers;     // 'Export in old format' checkbox flag
   //----------------- last used values of global find 1 -----------------------
/*OLD*/   EQF_BOOL fFolFindConfirm;    // 'Confirm changes' checkbox flag
/*OLD*/   EQF_BOOL fFolFindUpdateTM;   // 'Update translation memory' checkbox flag
/*OLD*/   EQF_BOOL fFolFindCaseRespect;// 'Respect case' radiobutton state
   //----------- last used values of folder markup and editor ------------------
   CHAR szFormat[MAX_FILESPEC];        // folder's documents format table
   CHAR szEditor[MAX_FILESPEC];        // name of editor
   //----------------- last used values of global find 2 -----------------------
   CHAR szDummy1[MAX_FINDCHANGE_LEN+1];     // dummy1
   CHAR szDummy2[MAX_FINDCHANGE_LEN+1]; // dummy2
   EQF_BOOL fFolExpWithROMem;    // 'With search Translation Memory databases' checkbox flag
   EQF_BOOL fFolExpWithDocMem;   // 'With document TM' checkbox flag
   //----------------------------- export history log only
   EQF_BOOL fFolExpHistLog;      // 'Wordcount data only' checkbox flag
   EQF_BOOL fUseShortFolderName; // use short folder name

   CLBFILTER Filter;               // filter used by column list box

   //----------------------------- filler --------------------------------------

   CHAR      szExpOriginator[MAX_DESCRIPTION];    // Originator defined during export
   CHAR      szExpEMail[MAX_DESCRIPTION];         // Originator Name e-mail

   VENDORNAME szTranslatorList[5];                // List of Last used translator
   VENDORMAIL szTranslatorMailList[5];                // List of translator's mail

/*OLD*/ FINDNAME szFindList[MAX_SEARCH_HIST];                   // List of Last used find values in global find and change
/*OLD*/ FINDNAME szReplaceList[MAX_SEARCH_HIST];                // List of Last used replace values

   //----------------- folder import -----------------------
   EQF_BOOL  fImportPath;
   CHAR      szImportPath[MAX_LONGPATH];           // preset of Import EF

   //----------------- folder export -----------------------
   EQF_BOOL  fExportPath;
   CHAR      szExportPath[MAX_LONGPATH];           // preset of Export EF

   //-------------- last used values of global find part 3 -----------------------
/*OLD*/   EQF_BOOL  fFolFindTranslTextOnly;             // LU: translatable text only checkbox
/*OLD*/   EQF_SWP   swpFolFindSizePos;                  // LU: dialog size and position
   //---- added for Unicode
/*OLD*/   CHAR_W szFolFind[MAX_FINDCHANGE_LEN+1];       // string being looked for
/*OLD*/   CHAR_W szFolChangeTo[MAX_FINDCHANGE_LEN+1];   // new value for string
   EQF_BOOL fFolNonUnicode;     // 'Export in non Unicode format' checkbox flag
   SHORT  sLastUsedViewWidth[MAX_VIEW];          // last used view list

   //-------------- last used values of global find part 4 -----------------------
/*OLD*/   EQF_BOOL  fFolFindWholeWordsOnly;             // LU: find whole words only
   EQF_SWP   swpTEnvFindSizePos;                 // LU: TranslationEnvironment find dialog size and position
/*OLD*/   CHAR_W    chWildCardSingleChar;               // wildcard for single characters
/*OLD*/   CHAR_W    chWildCardMultChar;                 // wildcard for multiple characters

   EQF_SWP   swpSegPropSizePos;                  // LU: TranslationEnvironment segment properties window
   EQF_BOOL  fSegPropDialogActive;               // TRUE: Segment properties window was active when TE was closed
   USHORT    usFolExpFormat;                     // LU: format of folder export
   USHORT    usFolImpFormat;                     // LU: format of folder import
   EQF_SWP   swpTEnvCFindSizePos;                // LU: TranslationEnvironment concordance search dialog size and position

   // fuzz segment search related stuff
   EQF_SWP   swpFSearchSizePos;                  // LU: fuzzy search dialog size and position
   
   int       iFSLastUsedClass;                   // LU: class selected in fuzzy search dialog
   int       iFSLastUsedMode;                    // LU: mode selected in fuzzy search dialog: 0 = up to selected class, 1 = only selected class, 2 = selected class and higher

   CHAR      szFSLastExportFile[MAX_LONGFILESPEC];// LU: last used file name for the export of the fuzzy segment search results
   BOOL      fFSWithMarks;                        // LU: last used state of with marks checkbox

   COLORREF  aclrFSLastUsedForeground[10];        // LU: user foreground color settings in the fuzzy segment search 
   COLORREF  aclrFSLastUsedBackground[10];        // LU: user background color settings in the fuzzy segment search 

   LONG lFSFontHeight;
   LONG lFSFontWidth;
   LONG lFSFontEscapement;
   LONG lFSFontOrientation;
   LONG lFSFontWeight;
   BYTE bFSFontItalic;
   BYTE bFSFontUnderline;
   BYTE bFSFontStrikeOut;
   BYTE bFSFontCharSet;
   BYTE bFSFontOutPrecision;
   BYTE bFSFontClipPrecision;
   BYTE bFSFontQuality;
   BYTE bFSFontPitchAndFamily;
   TCHAR szFSFontFaceName[40];

   CHAR chReserve[3846];                          // reserve space/filler

} PROPFOLDERLIST, *PPROPFOLDERLIST;


// Structure of EQF folder properties (also used for sub folder properties and for calc.profiles)
typedef struct _PROPFOLDER
{
   PROPHEAD  PropHead;          // Common header of properties

   EQF_SWP  Swp;                // Window position
   char RestartFolders[ 128];   // Names of subfolders to be restarted
   char RestartDocuments[ 256]; // Names of documents to be restarted

   char UniqueID[ 21];          // System generated unique folder ID
   char ExportStamp[ 81];       // Values saved during export of folder
   char szDescription[ 41];     // folder description

   char szFormat[MAX_FILESPEC];         // folder's documents format table
   char szMemory[MAX_FILESPEC];         // folder's memory db
   char szExlist[MAX_FILESPEC];         // folder's exclusion list
   char DicTbl[ NUM_OF_FOLDER_DICS*MAX_FILESPEC];  // folder's list of dictionaries
   char Unused1[262];                   // unused area
   CHAR chDrive;                        // drive where folder data is stored

   //----------------- Analysis(1) - last used values -------------------------
   char   szSavedDlgIanaInDic[NUM_OF_FOLDER_DICS+1][MAX_FNAME+MAX_FEXT];
                                            // input dictionaries
   // old (short name) list of analysis TMs
   char   szSavedDlgIanaInMdb[NUM_OF_FOLDER_MDB+1][MAX_FNAME+MAX_FEXT];
                                            // input memory databases
   CHAR   szDummy1[MAX_FNAME+MAX_FEXT];     // ** variablee has been removed **
   EQF_BOOL   fSavedDlgIanaChkMdb;          // state of out mdb checkb
   EQF_BOOL   fSavedDlgIanaChkAddSeg;       // state of add segm. checkb
   //----------------- Analysis(2) - last used values ------------------------
   EQF_BOOL   fAddTermsToDicCb;             // state of add terms to dict. CB
   EQF_BOOL   fExTermsInListCb;             //state of excl. terms in list CB
   CHAR   szExTermsInListDd[MAX_FILESPEC];  //excl terms in list dropdown LB
   EQF_BOOL   fExTermsInDicCb;              //state of excl. terms in dic. CB
   CHAR   szOLDExTermsInDicDd[MAX_FILESPEC];   // old field: excl terms in dic. dropdown LB, not used anymore
   EQF_BOOL   fCreateNewTermsCb;            // state of create new terms  CB
   USHORT usFree;                           // not used
   EQF_BOOL   fCreateFndTermsCb;            // state of create found terms  CB

   ULONG  ulLastChange;                     // date of last folder change

   SHORT  sLastUsedViewList[MAX_VIEW]; // last used view list
   SHORT  sDetailsViewList[MAX_VIEW];  // user's details view list
   SHORT  sSortList[MAX_VIEW];         // user's sort criteria list

   //------------------- Count dialog last used values ------------------------
   EQF_BOOL    fOrg;                   // status of original RB
   EQF_BOOL    fTran;                  // status of original RB
   EQF_BOOL    fTranslated;            // status of already translated CB
   EQF_BOOL    fToBeTran;              // status of to be translated CB
   EQF_BOOL    fTotal;                 // status of total CB
   EQF_BOOL    fFile;                  // status of file CB
   EQF_BOOL    fPrint;                 // status of print CB
   CHAR    szOldOutputFile[MAX_PATH144];  // filename for output file
   //----------------------- editor stuff --------------------------------
   CHAR    szEditor[MAX_FILESPEC];     // name of editor
   USHORT  usMatchLevel;               // exact match level

   USHORT       usNTLNumOccurences;    // NTL min. Num. of occurences EF
   EQF_BOOL     fNTLcontextCb;         // NTL with context info CB
   USHORT       usFTLNumOccurences;    // FTL min. Num. of occurences EF
   EQF_BOOL     fFTLcontextCb;         // FTL with context info CB
   CHAR         szOLDAddTermsToDicDd[MAX_FILESPEC]; // old field: add terms to dict. CB, not used anymore
   CHAR         szCreateNewTermsDd[MAX_FILESPEC];// create new terms CB
   CHAR         szCreateFndTermsDd[MAX_FILESPEC];          // create found terms CB

   EQF_BOOL  fSavedDlgIanaChkNoMatch;          // state of out new matches checkb

   CHAR      szSourceLang[MAX_LANG_LENGTH]; // FOLDER SOURCE LANGUAGE
   CHAR      szTargetLang[MAX_LANG_LENGTH]; // FOLDER TARGET LANGUAGE
   EQF_BOOL  fUsedLatestExactMatch;         //
   EQF_BOOL  fAnalAutoJoin;                 //
   CHAR      szOrgName[MAX_FNAME];          // original folder name

   USHORT    usReport;                      // selected Report
   USHORT    usDummyOption;                 // old selected options field (unused)
   EQF_BOOL  fRptFile;                      // status of CHK File
   CHAR      szOldRptOutputFile[MAX_PATH144];  // filename for output file
   char MemTbl[ NUM_OF_READONLY_MDB*MAX_FILESPEC];  // folder's list of mems
   EQF_BOOL  fExactContextTMMatch;          // support for Exact Context TM..
   EQF_BOOL  fSavedDlgIanaTMMatch;          // state of TMMatch checkbox
   //----------------new Count dialog last used value ------------------------
   EQF_BOOL    fReplMatch;                  // status of replace match flag
    //---Summary counting report -----
   USHORT    usFormat;                      // selected Format 1951 -> 1949
   USHORT    usOption1;                     // selected Options no 1 for summary counting report
   USHORT    usOption2;                     // selected Options no 2 for summary counting report
   USHORT    usOption3;                     // selected Options no 3 for summary counting report
   USHORT    usOption4;                     // selected Options no 4 for summary counting report
   USHORT    usOption5;                     // selected Options no 5 for summary counting report
   USHORT    usColumns[4];                  // columns to be switched on/off
   USHORT    usOption21;                    // selected Options no 1 for final fact sheet
   USHORT    usOption22;                    // selected Options no 2 for final fact sheet

   float     Complexity_Factor[10][3];       // factor for final fact sheet
   float     Pay_Factor[10][3];              // factor for final fact sheet
   USHORT    First_Report_Factor;            // first call of counting report structure
   float     Pay_per_Standard;               // Pay factor per standard page rep line
   USHORT    usStandard;                     // Standard page or Standard Line

   USHORT    usDummy[1];                     // filler

   EQF_SWP   SwpMTListWindow;               // Window position of MT List
   SHORT     sLastUsedMTViewList[MAX_VIEW]; // last used view list
   SHORT     sDetailsMTViewList[MAX_VIEW];  // user's details view list
   SHORT     sSortListMT[MAX_VIEW];         // user's sort criteria list
   EQF_BOOL  fSavedDlgIanaRedundCount;      // state of redundancy count checkbox
   USHORT    usOption[6];                   // selected Options  with history
   CHAR      szProfile[40];                 // Profile for counting info

   CHAR      szRptDescription[MAX_DESCRIPTION]; // Description field
   USHORT    usCurrency;                    // selected Currency

   USHORT    usColumns4[5];                 // columns to be switched on/off
                                            // more counting report options


   CHAR      szConversion[MAX_DESCRIPTION]; // document export conversion / CP
   CHAR      szLongName[MAX_LONGFILESPEC];  // folder long (descriptive) name
   ULONG     ulParentFolder;                // ID of folder's parent folder or
                                            // OL for main folders
   char szLongMemory[MAX_LONGFILESPEC];     // long name of folder's memory db
   EQF_BOOL  fAnalIgnoreIdentical;          // analysis flag 'ignore identical'
   EQF_BOOL  fAnalIgnorePath;               // analysis flag 'ignore document path'

   CHAR aLongMemTbl[MAX_NUM_OF_READONLY_MDB][MAX_LONGFILESPEC];   // folder's list of R/O mems
   CHAR aLongDicTbl[MAX_NUM_OF_FOLDER_DICS][MAX_LONGFILESPEC];  // folder's list of dictionaries
   CHAR aLongSavedAnaDic[MAX_NUM_OF_FOLDER_DICS+1][MAX_LONGFILESPEC];
   CHAR aLongSavedAnaMdb[MAX_NUM_OF_FOLDER_MDB+1][MAX_LONGFILESPEC];

   CHAR      szOrgLongName[MAX_LONGFILESPEC];  // original folder long (descriptive) name

   EQF_BOOL  fAnalLeadingWS;                // state of our leading whitspace handling flag
   EQF_BOOL  fAnalTrailingWS;               // state of our trailing whitspace handling flag

   CLBFILTER Filter;                   // filter used by column list box


   //----------------AFC folder properties -----------------------------------
   EQF_BOOL  fAFCFolder;                          // Mark the folder as AFC folder
   EQF_BOOL  fTCMasterFolder;                     // Mark TC Master folder
   EQF_BOOL  fPasswordOK;                         // Mark TC Master folder
   USHORT    usChildNr;                           // Child Nr
   CHAR      szAFCPassword[7];                    // AFC Password (ONLY TC Master)
   CHAR      szCoordinator[MAX_DESCRIPTION];      // Project coordinator
   CHAR      szCoordinatorEMail[MAX_DESCRIPTION]; // Project coordinator e-mail
   CHAR      szParent[MAX_DESCRIPTION];           // Parent Name
   CHAR      szParentEMail[MAX_DESCRIPTION];      // Parent Name e-mail
   CHAR      szExpOriginator[MAX_DESCRIPTION];    // Originator defined during export
   CHAR      szExpEMail[MAX_DESCRIPTION];         // Originator Name e-mail
   CHAR      szVendor[MAX_DESCRIPTION];           // Vendor Name (Translator)
   CHAR      szVendorEMail[MAX_DESCRIPTION];      // Vendor e-mail
   USHORT    usMaxHierachies;                     // Max Number of child hierachies
   USHORT    usShipment;                          // consistent cross document shipment number
   CHAR      szProduct[MAX_DESCRIPTION];          // Product Name
   CHAR      szProductFamily[MAX_DESCRIPTION];    // Product Family
   CHAR      szSimilarProduct[MAX_DESCRIPTION];   // Similar Product Family
   CHAR      szSubjectDict[MAX_DESCRIPTION];      // Product subject area dictionary
   CHAR      szSubjectMem[MAX_DESCRIPTION];       // Product subject area memory
   CHAR      szPrevVersion[MAX_DESCRIPTION];      // previous version of the product
   CHAR      szVersion[MAX_DESCRIPTION];          // version of the product
   CHAR      szShipment[MAX_DESCRIPTION];         // shipment string
   CHAR      szLogShipment[MAX_DESCRIPTION];      // last shipment string in history log


   USHORT    usShipmentChk;                       // shipment checked in counting report
   ULONG     ulNextSubFolderID;                   // ID to be used for next subfolder
   ULONG     ulSubFolderID;                       // ID of this subfolder (0 for main folder)


   EQF_BOOL     fNTLMwtermCb;                      // NTL with multi-term info CB
   CHAR         szCreateNewMWTermsDd[MAX_FILESPEC];// create new multi-terms CB

   USHORT    usMWTOption;                     // selected Options MWT

   EQF_BOOL  fAnalRespectCRLF;                    // Last used value: analysis flag respect CRLF
   EQF_BOOL  fAnalNoAddBlank;                     // Last used value: anaylsis flag no additional blanks
   SHORT     sLastUsedViewWidth[MAX_VIEW]; // last used view list

   EQF_BOOL  fSavedDlgIanaChkMachTrans;        // last used machine translation flag in analysis
   //--- the follwing fields are used in calculation profiles only ---
   LONG      lCheckSum;                           // checksum of property data
   CHAR      szProtectStamp[10];                  // profile protection stamp
   CHAR      szCalcProfileVersion[40];            // calc.profile version (for future use)

   // new long name list of analysis TMs
   char szSavedDlgInMdb[NUM_OF_FOLDER_MDB+1][MAX_LONGFILESPEC];

   //--- the following field is used in calculation profiles only ---
   CHAR      szIntProfileName[9];                 // internal profile name

   //
   // fields for enhanced MT logging
   // these fields are filled during analysis with MT processing
   //
   // the array is for the three categories: simple, medium, complex
   EQF_BOOL fMTFieldsFilled;           // TRUE: MT fields have been filled and are valid
   EQF_BOOL fMTReceived;               // TRUE: MT job has been received, ulMTReceive.. fields are valid
   ULONG ulMTTotalWords[3];            // total # of words processed in MT handling of analysis
   ULONG ulMTTotalSegs[3];             // total # of segments processed in MT handling of analysis
   ULONG ulMTSendWords[3];             // # of words send to MT server
   ULONG ulMTSendSegs[3];              // # of segments send to MT server
   ULONG ulMTReceivedWords[3];         // # of words received from MT server
   ULONG ulMTReceivedSegs[3];          // # of segments received from MT server

   // new folder flag for controlled folders: MT logging
   EQF_BOOL fMTLogging;                // TRUE: perform full MT logging


   // document export last used values (from DOCIMEX properties) for LU values on folder level
   EQF_BOOL fDocExpLastUsed;          // TRUE = the document export fields have been filled
   SHORT    sSavedDlgFExpoFormat;     // last used format for export
   EQF_BOOL fSavedDlgFExpoTranslation;// selection of translation button
   EQF_BOOL fSavedDlgFExpoWithRevMark;// selection of with rev mark button
   EQF_BOOL fSavedDlgFExpoOriginal;   // selection of original button
   EQF_BOOL fSavedDlgFExpoSNOMATCH;   // selection of SNOMATCH button
   char cSavedDlgFExpoDrive;          // last used drive for export
   char cSavedDlgFExpoDriveTrans;          // last used drive for export
   char cSavedDlgFExpoDriveOrg;          // last used drive for export
   char cSavedDlgFExpoDriveSno;          // last used drive for export
   char szSavedRevMark[MAX_FNAME];    // last used revision mark
   char szSavedDlgFExpoTPath[MAX_LONGPATH]; // last used target path export files dlg
   char szSavedDlgFExpoSPath[MAX_LONGPATH]; // last used source path export files dlg
   char szSavedDlgFExpoNPath[MAX_LONGPATH]; // last used SNOMATCH path export files dlg

   // document import last used values (from DOCIMEX properties) for LU values on folder level
   EQF_BOOL  fDocImpLastUsed;            // TRUE = the document import fields have been filled
   char szSavedDlgLoadDrive[3];          // last used drive in LOAD dialog
   SHORT sSavedDocImpDlgMode;            // last used format for import
   char szSavedStartPath[MAX_LONGPATH];  // last used start path
   char szSavedDlgLoadPath[MAX_LONGPATH];// last used path in LOAD dialog

   // long name enabled fields for output dictionary and exlusion dictionary in term list processing
   CHAR   szAddTermsToDicDd[MAX_LONGFILESPEC]; // add terms to dict. CB
   CHAR   szExTermsInDicDd[MAX_LONGFILESPEC];  //excl terms in dic. dropdown LB

   // analysis "create match list flag"
   EQF_BOOL  fSavedDlgIanaMatchList;

   // fields for document export in validation format
   char      szSavedDlgFExpoValPath[MAX_LONGPATH]; // last used valdiation format path export files dlg
   SHORT     sSavedDlgFExpoValFormat;              // selected export format
   EQF_BOOL  fSavedDlgFExpoValWithProtSegs;        // TRUE = export with protected segments
   EQF_BOOL  fSavedDlgFExpoValCombine;             // TRUE = combine into  a single output file

   // analysis "load balancer list flag"
   EQF_BOOL  fSavedDlgIanaBalanceList;

   // more fields for document export in validation format
   EQF_BOOL  fSavedDlgFExpoValValFormat;
   EQF_BOOL  fSavedDlgFExpoValProofFormat;
   EQF_BOOL  fSavedDlgFExpoValAllMatch;
   EQF_BOOL  fSavedDlgFExpoValNewMatch;
   EQF_BOOL  fSavedDlgFExpoValProtMatch;
   EQF_BOOL  fSavedDlgFExpoValAutoMatch;
   EQF_BOOL  fSavedDlgFExpoValNotTransl;
   EQF_BOOL  fSavedDlgFExpoValFuzzyMatch;
   EQF_BOOL  fSavedDlgFExpoValExactMatch;
   EQF_BOOL  fSavedDlgFExpoValMachMatch;
   EQF_BOOL  fSavedDlgFExpoValReplMatch;

   // analysis option fields for replacement list handling
   EQF_BOOL  fAnalUseReplacementList;
   CHAR      szAnalReplacementList[MAX_LONGFILESPEC];

   // more fields for document export in validation format
   USHORT    usSavedDlgFExpoValRemoveTagging; // remove tagging; 0 = not set, 1 = remove tagging, 2 = leave tagging
   EQF_BOOL  fSavedDlgFExpoValInclCount;
   EQF_BOOL  fSavedDlgFExpoValInclExisting;
   EQF_BOOL  fSavedDlgFExpoValMismatchOnly;
   EQF_BOOL  fSavedDlgFExpoValModExactMatch;

   // more fields for word count LUs
   EQF_BOOL  fInclMemMatch;            // include memory match info in duplicae word count

   // new fields for report output paths
   CHAR      szOutputFile[MAX_LONGPATH];  // filename for output file
   CHAR      szRptOutputFile[MAX_LONGPATH];  // filename for output file

   // last used values for aalysis profiles
   BOOL      fSavedDlgIanaProfile;         // profile checkbox
   CHAR      szSavedDlgIanaProfile[MAX_LONGFILESPEC]; // profile name

   // more fields for document export in validation format
   BOOL      fSavedDlgFExpoValLinksImages;

   // old analysis flag 'protect xmp/screen section'
   EQF_BOOL  fSavedDlgIanaProtXmpScreen;

   // wordcount dialog: last used report format
   USHORT    usCntFormat;

   // stop at first exact match flag
   EQF_BOOL  fStopAtFirstExact;              // folder properties dialog
   EQF_BOOL  fSavedDlgIanaStopAtFirstExact;  // analysis dialog

   EQF_BOOL  fAnalIgnoreComment;          // analysis flag 'ignore proposals with comments'

   CHAR      szGlobalMemOptFile[MAX_LONGFILESPEC]; // name of Global Memory Option file
   EQF_BOOL  fGlobalMemOptCheckRequired;           // FALSE = global memory options for this folder have been checked by user

   EQF_BOOL  fSavedDlgFExpoValGlobMemMatch;

   // last used values for export segments function
   CHAR      szLastOutputFile[MAX_LONGFILESPEC]; // name of last output file
   CHAR      szLastStartStopFile[MAX_LONGFILESPEC]; // name of last start/stop file

   // flag: disabled because of user exit refresh failed (needs restart of workbench)
   EQF_BOOL fDisabled_UserExitRefresh;

   // another validation document export option
   EQF_BOOL  fSavedDlgFExpoValModAutoMatch;

   // new individual section protection flags
   EQF_BOOL  fSavedDlgIanaProtXmp;
   EQF_BOOL  fSavedDlgIanaProtMsgNum;
   EQF_BOOL  fSavedDlgIanaProtMeta;
   EQF_BOOL  fSavedDlgIanaProtScreen;
   EQF_BOOL  fSavedDlgIanaProtCodeBlock;

   // dictionary selection by PID value            
   EQF_BOOL  fDictPIDSelect;                 // TRUE = select dict. by PID value
   CHAR      szDictPIDSelect1[60];           // PID values to select (Old value)

   // TVT tracking info
   USHORT    usLastAssignedDocNum ;         // Last used document number in folder
   EQF_BOOL  fSavedDlgFExpoWithTrackID;     // selection of with TVT tracking ID button

   CHAR      szDictPIDSelect2[MAX_DICTPID_VALUES]; // PID values to select


   //----------------Reserve -------------------------------------------------
   CHAR      CHRESERVE[389];          // RESERVE SPACE/FILLER (FILLS PROPS TO 32kB)

} PROPFOLDER, *PPROPFOLDER;

// structure for counting results as stored in document properties
typedef struct _COUNTSUMS
{
  ULONG       ulSimpleWords;           // number of words in simple segments
  ULONG       ulSimpleSegs;            // number of simple segments
  ULONG       ulMediumWords;           // number of words in medium segments
  ULONG       ulMediumSegs;            // number of medium segments
  ULONG       ulComplexWords;          // number of words in complex segments
  ULONG       ulComplexSegs;           // number of comples segments
} COUNTSUMS, *PCOUNTSUMS;

// Structure of EQF document properties
typedef struct _PROPDOCUMENT
{
   PROPHEAD  PropHead;          // Common header of properties

   EQF_SWP  Swp;                // Window position
   char szTemplate[MAX_FILESPEC];  // Name of the document template
   char Format[ 9];             // Document format
   char ExportStamp[ 81];       // Values saved during document export

   ULONG ulXLated;              // date/time  translation completed
   ULONG ulSeg;                 // date/time  segmented
   ULONG ulExp;                 // date/time  last time exported
   ULONG ulImp;                 // date/time  imported
   ULONG ulTouched;             // date/time  translation started
   USHORT usComplete;           // competionrate in per cent
   USHORT usCopied;             // translation copied without modification
   USHORT usModified;           // copied translation modified
   USHORT usScratch;            // translation started from scratch

   /*******************************************************************/
   /* New data fields! Use with care, may not be set for older        */
   /* property files (in this case use fields in folder props)        */
   /*******************************************************************/
   CHAR szFormat[MAX_FILESPEC];        // document format / Tag Table
   CHAR szMemory[MAX_FILESPEC];        // document Translation Memory
   CHAR szSourceLang[MAX_LANG_LENGTH]; // document source language
   CHAR szTargetLang[MAX_LANG_LENGTH]; // document target language

   /*******************************************************************/
   /* New fields for long file name support and alias function!       */
   /* Use with care, may not be set for older property files          */
   /*******************************************************************/
   CHAR szLongName[MAX_LONGFILESPEC];  // long file name of document
   CHAR szAlias[MAX_LONGFILESPEC];     // alias for TM access
   CHAR szImportPath[MAX_LONGPATH];    // import path
   CHAR szImportExitData[512];         // buffer for data of import exit
   CHAR szEditor[MAX_FILESPEC];        // document editor

   /*******************************************************************/
   /* New fields for word counts during analysis phase 2              */
   /*******************************************************************/
   ULONG ulTotal;                      // total numbeer of words in document
   ULONG ulExactExact;                 // words of exact-exact matches
   ULONG ulExactOne;                   // words of exact matches (one proposal)
   ULONG ulExactMore;                  // words of exact matches (more props)
   ULONG ulFuzzy;                      // words of fuzzy proposals
   ULONG ulNoProps;                    // words with no proposal at all
   ULONG ulFuzzyRepl;                  // words of fuzzy replace proposals
   ULONG ulRepl;                       // words of replace proposals
   ULONG ulSegTotal;                   // total numbeer of segments in document
   ULONG ulSegExactExact;              // segments of exact-exact matches
   ULONG ulSegExactOne;                // segments of exact matches (one proposal)
   ULONG ulSegExactMore;               // segments of exact matches (more props)
   ULONG ulSegFuzzy;                   // segments of fuzzy proposals
   ULONG ulSegNoProps;                 // segments with no proposal at all
   ULONG ulSegFuzzyRepl;               // segments of fuzzy replace proposals
   ULONG ulSegRepl;                    // segments of replace proposals
   ULONG ulMachineMatch;               // words of machine matchs
   ULONG ulSegMachineMatch;            // segments of machine matchs
   ULONG ulNotTranslated;              // number of not translated words in the document

  /* machine translation information */
   LONG  lMTStatus;                    // string for MT status
   CHAR  chMTEngine[ 20 ];             // string for MT Engine 2284
   CHAR  chMTEngineDll[ 13 ];          // string for MT Engine DLL
   CHAR  chMTMemory[ 20 ];             // string for MT Merge Memory
   CHAR  chMTId    [100 ];             // ID given by the MT engine
   ULONG ulMTLated;                    // date/time of MT status change

   CHAR  chMTServer[ 20 ];             // name of the MT server
   CHAR  chMTTCPIP[ 100 ];             // TCPIP
   CHAR  chMTPort[ 20 ];               // Port
   CHAR  chMTUser[ 20 ];               // login name for MT engine
   CHAR  chMTPass[ 20 ];               // password for MT engine
   CHAR  chMTProfile [ 20 ];           // selected profile settings
   CHAR  chMTTemp1 [ 13 ];             // MT temp name 1
   CHAR  chMTTemp2 [ 13 ];             // MT temp name 2
   CHAR  chMTTemp3 [ 13 ];             // MT temp name 3
   USHORT usMerged;                    // MT merged status
   USHORT usMTComplete;                // MT completion status

   // new fields for result of translation memory match counting
   COUNTSUMS Total;                    // total numbers
   COUNTSUMS ExactExact;               // counting results - exact-exact matches
   COUNTSUMS ExactOne;                 // counting results - exact matches
   COUNTSUMS Fuzzy1;                   // counting results - fuzzy matches 1
   COUNTSUMS Fuzzy2;                   // counting results - fuzzy matches 2
   COUNTSUMS Fuzzy3;                   // counting results - fuzzy matches 3
   COUNTSUMS NoProps;                  // counting results - no proposals
   COUNTSUMS MTProps;                  // counting results - MT proposals
   COUNTSUMS Repl;                     // counting results - replace matches
   COUNTSUMS ExactMore;                // counting results - more than one exact match

   CHAR      szConversion[MAX_DESCRIPTION]; // document export conversion / CP

   ULONG     ulParentFolder;           // ID of document's parent folder
                                       // 0L = main folder
   CHAR      szLongMemory[MAX_LONGFILESPEC]; // document Translation Memory (long name)




    //----------------AFC document properties -----------------------------------

//#ifdef R00_AFC


   CHAR      szVendor[MAX_DESCRIPTION];       // Vendor Name
   CHAR      szVendorEMail[MAX_DESCRIPTION];  // Vendor e-mail
   EQF_BOOL  fTranslatable;                   // Translatable or not
   EQF_BOOL  fGraphics;                       // (CorelDraw) graphics

   //---------------- filter used for document list ------------------------------
   CLBFILTER Filter;                   // filter used by column list box

   CHAR      szShipment[MAX_DESCRIPTION];     // shipment string

   ULONG ulSrc;                               // date/time  source document (added bt 19062001)
   EQF_BOOL  fSTargetInUnicode;
   LONG      lSmallFuzzLevel;       // fuzzines level for small segments
   LONG      lMediumFuzzLevel;      // fuzzines level for medium sized segments
   LONG      lLargeFuzzLevel;       // fuzzines level for large segments
   EQF_BOOL  fMarkupLocked;         // TRUE = markup of document is locked and may not be changed by user
   EQF_BOOL  fSourceLangLocked;     // TRUE = source language is locked and may not be changed by user
   EQF_BOOL  fTargetLangLocked;     // TRUE = target language is locked and may not be changed by user

   //--- TM version at time of analysis
   CHAR szAnalysisTMVersion[20];    // TM version at the time of the analysis run

   // TVT tracking info
   USHORT    usTrackDocNum ;       // Assigned document number in folder

   CHAR      szExternalDocPathName[MAX_LONGFILESPEC]; // fully qualified file name of the external file from which the document has been imported

   //----------------Reserve -------------------------------------------------
   CHAR   chReserve[710];               // reserve space/filler ( fills props to 4K)
} PROPDOCUMENT, *PPROPDOCUMENT;


// Structure of Import / Export properties
struct imex_prop {
   PROPHEAD  PropHead;          // Common header of properties

   char szSavedDlgLoadDrive[3];       // last used drive in LOAD dialog
   char szOldSavedDlgLoadPath[144];   // last used path in LOAD dialog (old length)
   char szSavedDlgLoadPatternName[9]; // last used pattern (name)
   char szSavedDlgLoadPatternExt[5];  // last used pattern (extension)
                                      // including leading dot
   char szOldSavedDlgFExpoTPath[144]; // last used target path export files dlg (old length)
   char szOldSavedDlgFExpoSPath[144]; // last used source path export files dlg (old length)

   char szSavedRevMark[MAX_FNAME];    // last used revision mark
   char szOldSavedDlgFExpoNPath[144]; // last used SNOMATCH path export files dlg (old length)

   char cSavedDlgFExpoDrive;          // last used drive for export
   SHORT    sSavedDlgFExpoFormat;     // last used format for export

   EQF_BOOL fSavedDlgFExpoTranslation;// selection of translation button
   EQF_BOOL fSavedDlgFExpoWithRevMark;// selection of with rev mark button
   EQF_BOOL fSavedDlgFExpoOriginal;   // selection of original button
   EQF_BOOL fSavedDlgFExpoSNOMATCH;   // selection of SNOMATCH button

   SHORT sSavedDocImpDlgMode;      // last used format for import

   char szSavedStartPath[MAX_LONGPATH];   // last used start path

   char cSavedDlgFExpoDriveTrans;          // last used drive for export
   char cSavedDlgFExpoDriveOrg;          // last used drive for export
   char cSavedDlgFExpoDriveSno;          // last used drive for export

   // new path fields for last used values which are large enough to contain full path
   char szSavedDlgLoadPath[MAX_LONGPATH];   // last used path in LOAD dialog
   char szSavedDlgFExpoTPath[MAX_LONGPATH]; // last used target path export files dlg
   char szSavedDlgFExpoSPath[MAX_LONGPATH]; // last used source path export files dlg
   char szSavedDlgFExpoNPath[MAX_LONGPATH]; // last used SNOMATCH path export files dlg

   // fields for document export in validation format
   char      szSavedDlgFExpoValPath[MAX_LONGPATH]; // last used valdiation format path export files dlg
   SHORT     sSavedDlgFExpoValFormat;              // selected export format
   EQF_BOOL  fSavedDlgFExpoValWithProtSegs;        // TRUE = export with protected segments
   EQF_BOOL  fSavedDlgFExpoValCombine;             // TRUE = combine all documents into a single output file

   // more fields for document export in validation format
   EQF_BOOL  fSavedDlgFExpoValValFormat;
   EQF_BOOL  fSavedDlgFExpoValProofFormat;
   EQF_BOOL  fSavedDlgFExpoValAllMatch;
   EQF_BOOL  fSavedDlgFExpoValNewMatch;
   EQF_BOOL  fSavedDlgFExpoValProtMatch;
   EQF_BOOL  fSavedDlgFExpoValAutoMatch;
   EQF_BOOL  fSavedDlgFExpoValNotTransl;
   EQF_BOOL  fSavedDlgFExpoValFuzzyMatch;
   EQF_BOOL  fSavedDlgFExpoValExactMatch;
   EQF_BOOL  fSavedDlgFExpoValMachMatch;
   EQF_BOOL  fSavedDlgFExpoValReplMatch;
   USHORT    usSavedDlgFExpoValRemoveTagging; // remove tagging; 0 = not set, 1 = remove tagging, 2 = leave tagging

   EQF_BOOL  fSavedDlgFExpoValInclCount;
   EQF_BOOL  fSavedDlgFExpoValInclExisting;
   EQF_BOOL  fSavedDlgFExpoValMismatchOnly;
   EQF_BOOL  fSavedDlgFExpoValModExactMatch;
   EQF_BOOL  fSavedDlgFExpoValLinksImages;
   EQF_BOOL  fSavedDlgFExpoValGlobMemMatch;
   EQF_BOOL  fSavedDlgFExpoValModAutoMatch;
   EQF_BOOL  fSavedDlgFExpoValExactFromManual;      // check for exact matches coming from manual translation switch
   EQF_BOOL  fSavedDlgFExpoValTransOnly;            // check for export trans text in simple format switch
   EQF_BOOL  fSavedDlgFExpoWithTrackID;             // selection of with TVT tracking ID button

   CHAR chReserve[1708];        // reserve space/filler (fills props to 4K )
};
typedef struct imex_prop PROPIMEX, *PPROPIMEX;

/**********************************************************************/
/* Editor properties required definitions from EQF_API.H!             */
/*                                                                    */
/* So the editor properties can only be defined when                  */
/* INCL_EQF_EDITORAPI is defined                                      */
/**********************************************************************/
#if defined(INCL_EQF_EDITORAPI)

// Structure of Editor properties
typedef struct _PROPEDIT
{
   PROPHEAD  PropHead;                 // Common header of properties
   CHAR szMFEStartProc[MAX_FILESPEC];  // editor start procedure
   STEQFGEN stEQFGen;                  // generic structure for api
// CHAR chReserve[1667];               // reserve space/filler (fills props to 4K)
// CHAR chReserve[1451];               // reserve space/filler
// CHAR chReserve[1195];               // reserve space/filler
   CHAR chReserve[937];                // reserve space/filler
} PROPEDIT, *PPROPEDIT;

#else
#define  GML_TAG_LEN       18
typedef struct _TATAG1
{
   CHAR chSTag[ GML_TAG_LEN ];      // start tag
   CHAR chETag[ GML_TAG_LEN ];      // end tag
} TATAG, * PTATAG;

typedef struct _TATAG1_W
{
   CHAR_W chSTag[ GML_TAG_LEN ];      // start tag
   CHAR_W chETag[ GML_TAG_LEN ];      // end tag
} TATAG_W, * PTATAG_W;

#endif

// Structure of EQF dictionary list window properties
typedef struct _PROPDICTLIST
{
   PROPHEAD  PropHead;                 // Common header of properties
   EQF_SWP   Swp;                      // Window position
   SHORT  sLastUsedViewList[MAX_VIEW]; // last used view list
   SHORT  sDetailsViewList[MAX_VIEW];  // user's details view list
   SHORT  sSortList[MAX_VIEW];         // user's sort criteria list
   //----------------- last used values of dictionary export -----------------------
   CHAR chDexDrive;                    // target drive for export
   CHAR chOldDexPath[MAX_PATH144];        // target directory for export
   EQF_BOOL fDexFormat;                // troja or external
   //----------------- last used values of dictionary import -----------------------
   CHAR chDimpDrive;                   // target drive for import
   CHAR chOldDimpPath[MAX_PATH144];       // target directory for import
   EQF_BOOL fDimpFormat;               // troja or external

   //----------------- last used values of dictionary display -----------------------
   RECTL     rclDisp;                  // Position of 'Display Dictionary Entry'
   CLBFILTER Filter;                   // filter used by column list box

   USHORT    usLastExpFormat;          // number of last used export format
   USHORT    usLastImpFormat;          // number of last used import format
   SHORT  sLastUsedViewWidth[MAX_VIEW]; // last used view list

   CHAR chDexPath[MAX_LONGPATH];        // target directory for export
   CHAR chDimpPath[MAX_LONGPATH];       // target directory for import

   CHAR      chReserve[2684];          // reserve space/filler (fills props to 4K)
} PROPDICTLIST, *PPROPDICTLIST;

#define MAX_DICT_DESCR   40            // max length of a dictionary description
#define MAX_TGT_LANG     10            // max number of target languages
#define MAX_PROF_ENTRIES 100           // max number of profile entries

// Defines for dictionary location
#define LOC_LOCAL        0             // dictionary is a local one
#define LOC_REMOTE       1             // dictionary is remote using EQFTMCOM
#define LOC_SHARED       2             // dictionary is shared on a LAN drive

// Structure of EQF dictionary properties
typedef struct _PROPDICTIONARY
{
   //--- common property part ---
   PROPHEAD  PropHead;                      // header of properties
   //--- general dictionary information ---
   CHAR      szDescription[MAX_DICT_DESCR]; // dictionary description
   CHAR      szSourceLang[MAX_LANG_LENGTH]; // dictionary source language
                                            // dictionary target language(s)
   CHAR      szTargetLang[MAX_TGT_LANG][MAX_LANG_LENGTH];
   EQF_BOOL  fCopyRight;                    // dictionary-is-copyrighted flag
   CHAR      szDictPath[MAX_EQF_PATH];      // fully qualified dict file name
   CHAR      szIndexPath[MAX_EQF_PATH];     // fully qualified index file name
   //--- dictionary profile ---
   USHORT    usLength;                      // number of user profile entries
   USHORT    usUserNameCount;               // number of user created dict names
   COLFONT   ColFontDictEntry;              // colour/font for dict entry
   COLFONT   ColFontEntryVal;               // colour/font for dict entry value
   PROFENTRY ProfEntry[MAX_PROF_ENTRIES];   // user profile entries
   EQF_BOOL  fProtected;                    // dictionary-is-protected flag
   ULONG     ulPassWord;
   CHAR      szServer[MAX_SERVER_NAME];     // Server Name of TM or \0 if TM is local
   CHAR      szUserid[MAX_USERID];          // LAN Userid of TM: if local '\0'
   CHAR      chRemPrimDrive;                // LAN primary drive
   USHORT    usLocation;                    // location of dictionary
   CHAR      szLongName[MAX_LONGFILESPEC];  // dictionary long (descriptive) name
   CHAR      szLongDesc[MAX_LONG_DESCRIPTION];  // dictionary long (descriptive) name
   USHORT    usVersion;                     // Version of dictionary - used for
                                            // comparison during organize if copyrighted
   //--- reserved space ---
   CHAR      chReserve[3402];               // reserve space / filler
} PROPDICTIONARY, *PPROPDICTIONARY;

// Structure of EQF tag table window properties
typedef struct _PROPTAGTABLE
{
   PROPHEAD  PropHead;          // Common header of properties
   EQF_SWP       Swp;           // Window position
   //----------------- last used values of tag table export -----------------------
   CHAR chTexDrive;             // target drive for export
   CHAR chOldTexPath[MAX_PATH144]; // target directory for export
   EQF_BOOL fTexFormat;         // troja or external
   //----------------- last used values of tag table import -----------------------
   CHAR chTimpDrive;            // target drive for import
   CHAR chOldTimpPath[MAX_PATH144];// target directory for import
   EQF_BOOL fTimpFormat;        // troja or external
   SHORT  sLastUsedViewList[MAX_VIEW]; // last used view list
   SHORT  sDetailsViewList[MAX_VIEW];  // user's details view list
   SHORT  sSortList[MAX_VIEW];         // user's sort criteria list
   SHORT  sLastUsedViewWidth[MAX_VIEW]; // last used view list ( needs 40B from Reserve)

   CHAR chTexPath[MAX_LONGPATH]; // target directory for export
   CHAR chTimpPath[MAX_LONGPATH];// target directory for import

   CHAR   chReserve[2944];
} PROPTAGTABLE, *PPROPTAGTABLE;

// Structure of EQF memory properties and IDA of the memcreate dialog
// and required define statements
// This part of the include file is maintained by CON

typedef struct _PROPMEMORY
{
 PROPHEAD     stPropHead;                     // Common header of properties
 // szName must be the first item after the property header
 // Do only insert new values at the end of the stucture !!!!!!!!!!
 CHAR         szName[MAX_FILESPEC];           // Property name of memory database ~TEST.MEM~
 CHAR         szPath[MAX_EQF_PATH];           // Property path of memory database  ~C:\EQF~
 USHORT       usCreate;                       // Work variable: If FALSE the memory database dialog failed
 PFNWP        wProcName;                      // Work variable: Old window procedure of name field
 CHAR         szMemName[MAX_FILESPEC];        // Memory database name without extension  ~TEST~
 CHAR         szMemPath[MAX_EQF_PATH];        // Work variable: path to the memory databases with any disk drive ~T:\EQF\MEM\~
 CHAR         szFullMemName[MAX_EQF_PATH];    // Full name and path to the memory databases
                                              // with correct drive, name and ext. ~C:\EQF\MEM\TEST.MEM~
 CHAR         szMemDesc[MAX_MEM_DESCRIPTION]; // Description of memory database  ~Test 25 Beschreibung~
 CHAR         szFormat[MAX_FILESPEC];         // Format table name without extension ~GML~
 CHAR         szXTagsListFile[MAX_EQF_PATH];  // Full path + name + ext to format table ~C:\EQF\TABLE\GML.TBL~
 CHAR         szExclusion[MAX_FILESPEC];      // Exclusion list name without extension  ~ENGNOISE~
 CHAR         szXWordsListFile[MAX_EQF_PATH]; // Full path + name + ext to noise list ~C:\EQF\TABLE\ENGNOISE.LST~
 LANGUAGE     szSourceLang;                   // Source language ~English~
 LANGUAGE     szTargetLang;                   // Target language ~German~
 CHAR         szLanguageFile[MAX_EQF_PATH];   // Full path + name + ext to the language file ~C:\EQF\TABLE\SOURCE.LNG~
 CHAR         szFullFtabPath[MAX_EQF_PATH];   // Full path to tag tables and exclusion lists ~C:\EQF\TABLE\~
 CHAR         szTemp[MAX_PATH144];            // Work field to construct a specific path  ~??????????~
 CHAR         szServerDriveList[MAX_DRIVELIST];  // Work variable: List of server drives
 USHORT       usBlockSize;                    // Block size of memory database ~1024 stored in USHORT~
 EQF_BOOL     fDbcs;                          // DBCS indicator TRUE = DBCS, FALSE = SBCS
 CHAR         szPathLastExported[MAX_PATH144];// Full name and path of exported file in ASCII format ~C:\EQFTEST\TEST.EXP~
 CHAR         szEmpty1[4];
 // Structure end in Troja 2.1
 CHAR         szServer[MAX_SERVER_NAME];      // Server Name of TM or \0 if TM is local
 CHAR         szUserid[MAX_USERID];           // LAN Userid of TM: if local '\0'
 TIME_L       tCreate;                        // TM ID which is the creation time stamp
                                              // TM properties of Troja 2.1 or eaelier have
                                              // set the ID value to '\0'
 USHORT       usLocation;                     // Work variable: Possible values: TM_LOCAL,
                                              //  TM_REMOTE, TM_SHARED
 EQF_BOOL     fDeleteDriveIcons;                   // Work variable: Flag for dialog process
 CHAR         szLangProp[MAX_LANGUAGE_PROPERTIES]; // Work variable: Work area to get the language properties
 // Structure end in Troja 3.0
 CHAR         szLANDriveList[MAX_DRIVELIST];  // Work variable: List of LAN drives
} PROPTRANSLMEM, *PPROPTRANSLMEM;


typedef struct _LISTLASTUSED
{
  EQF_SWP Swp;                             // Window position
  SHORT  sLastUsedViewList[MAX_VIEW];      // last used view list
  SHORT  sDetailsViewList[MAX_VIEW];       // user's details view list
  SHORT  sSortList[MAX_VIEW];              // user's sort criteria list
  CHAR   szListType[MAX_EQF_PATH];         // current list type (unused)
  CHAR   szSelectedList[MAX_EQF_PATH];     // selected list in listbox
  CHAR   szFirstList[MAX_EQF_PATH];        // first visible list in listbox
  //SHORT  sLastUsedViewWidth[MAX_VIEW];      // user's details view list
} LISTLASTUSED, *PLISTLASTUSED;

typedef struct _PROPLIST
{
   //-------------------------- property header --------------------------------
   PROPHEAD  PropHead;                      // Common header of properties
   //------------- last used values of list handler window (old style) --------------
   LISTLASTUSED    LU;                      // old style last used values
   //----------- last used values for list export  ------------------------
   USHORT usListExpFormatID;                // ID of format radiobutton
   CHAR   chListExpDrive;                   // drive character
   CHAR   szListExpPath[MAX_PATH144];       // path name
   //------------------- last used values for list import ----------------------
   USHORT usListImpFormatID;                // ID of format radiobutton
   CHAR   chListImpDrive;                   // drive character
   CHAR   szListImpPath[MAX_PATH144];       // path name
   //------------- last used values per type of list ---------
   LISTLASTUSED    NTLLU;                   // last used NTL values
   LISTLASTUSED    FTLLU;                   // last used FTL values
   LISTLASTUSED    EXCLU;                   // last used EXC values
   LISTLASTUSED    NOILU;                   // last used NOI values
   LISTLASTUSED    ABRLU;                   // last used ABR values
   CLBFILTER Filter;                   // filter used by column list box
   //----------- new last used values for list import and export  ---------------
   USHORT usExpFormat;                      // format of export
   USHORT usImpFormat;                      // format of export
   //----------------------------- filler --------------------------------------
   CHAR chReserve[1422];                    // reserve space/filler (fills props to 4K)
} PROPLIST, *PPROPLIST;


/**********************************************************************/
/* Function declarations for the EQF Property Handler                 */
/**********************************************************************/
__declspec(dllexport)
HPROP   OpenProperties(   PSZ, PSZ, USHORT, PEQFINFO);
__declspec(dllexport)
HPROP   CreateProperties( PSZ, PSZ, USHORT, PEQFINFO);
__declspec(dllexport)
HPROP   CreatePropertiesEx( PSZ, PSZ, USHORT, PEQFINFO, BOOL );
__declspec(dllexport)
SHORT   DeleteProperties( PSZ, PSZ, PEQFINFO);
__declspec(dllexport)
SHORT   CloseProperties(  HPROP, USHORT, PEQFINFO);
__declspec(dllexport)
SHORT   GetAllProperties( HPROP, PVOID, PEQFINFO);
__declspec(dllexport)
SHORT   PutAllProperties( HPROP, PVOID, PEQFINFO);
__declspec(dllexport)
SHORT   SaveProperties( HPROP, PEQFINFO);
__declspec(dllexport)
PVOID   MakePropPtrFromHnd( HPROP hObject);
PVOID   MakePropPtrFromHWnd( HWND hObject);
__declspec(dllexport)
BOOL    SetPropAccess( HPROP hprop, USHORT flgs);
__declspec(dllexport)
VOID    ResetPropAccess( HPROP hprop, USHORT flgs);
PSZ     MakePropPath( PSZ pbuf, PSZ pd, PSZ pp, PSZ pn, PSZ pe);
SHORT   GetPropSize( USHORT usClass);
__declspec(dllexport)
PPROPSYSTEM GetSystemPropPtr( VOID );
BOOL    PropHandlerInitForBatch( void );
BOOL    PropHandlerTerminateForBatch( void );
__declspec(dllexport)
HPROP   EqfQuerySystemPropHnd( void );
__declspec(dllexport)
USHORT  ReloadSysProp(PPROPSYSTEM);

// other function prototypes
BOOL DDEIsBatchHwnd( HWND hwndInQuestion );
BOOL ObjHandlerInitForBatch( void );
BOOL ObjHandlerTerminateForBatch( void );
__declspec(dllexport)
SHORT ObjQuerySymbol( PSZ pszSymbol );
__declspec(dllexport)
SHORT ObjSetSymbol( PSZ pszSymbol );
__declspec(dllexport)
SHORT ObjRemoveSymbol( PSZ pszSymbol );
__declspec(dllexport)
USHORT MemFuncCreateMem
(
  PSZ         pszMemName,              // name of new Translation Memory
  PSZ         pszDescription,          // description for new Translation Memory or NULL
  CHAR        chToDrive,               // target drive for new Translation Memory
  PSZ         pszSourceLanguage,       // Translation Memory source language
  LONG        lOptions                 // type of new Translation Memory
);
__declspec(dllexport)
USHORT MarkupFuncCreateMarkup
(
  PSZ         pszInFile,               // Input TBX file
  PSZ         pszOutFile               // Output TBL file
);

typedef enum _UTLCHECKOBJTYPES
{
  FOLDER_OBJECT,
  TM_OBJECT,
  DICT_OBJECT,
  EDITOR_OBJECT,
  DOCUMENT_OBJECT,
  MARKUP_OBJECT,
  TARGET_LANGUAGE_OBJECT,
  SOURCE_LANGUAGE_OBJECT,
} UTLCHECKOBJTYPES;
__declspec(dllexport)
BOOL UtlCheckIfExist( PSZ pszName, SHORT sObjType );

__declspec(dllexport)
USHORT ObjLongToShortName
(
  PSZ pszLongName,                     // ptr to object long name
  PSZ pszShortName,                    // ptr to buffer for short name
  UTLCHECKOBJTYPES ObjType,            // type of object
  PBOOL   pfIsNew                      // ptr to caller's is-new flag
);
__declspec(dllexport)
USHORT ObjShortToLongName
(
  PSZ pszShortName,                    // ptr to object short name
  PSZ pszLongName,                     // ptr to buffer for long name
  UTLCHECKOBJTYPES ObjType             // type of object
);

// possible return stated of ObjLongToShortEx function
typedef enum _OBJLONGTOSHORTSTATE
{
  OBJ_EXISTS_ALREADY,                  // an object with the given name exists
  SHARED_OBJ_EXISTS,                   // there is a shared object with the same name
  OBJ_IS_NEW                           // the object is a new one
} OBJLONGTOSHORTSTATE, * POBJLONGTOSHORTSTATE;

// Object long to short name conversion with check for shared resources
__declspec(dllexport)
USHORT ObjLongToShortNameEx
(
  PSZ         pszLongName,             // ptr to object long name
  CHAR        chDrive,                 // drive to use for shared resource checking
  PSZ pszShortName,                    // ptr to buffer for short name
  UTLCHECKOBJTYPES ObjType,            // type of object
  POBJLONGTOSHORTSTATE pObjState       // ptr to buffer for returned object state
);

// Object long to short name conversion with check for shared resources and reservation of files
__declspec(dllexport)
USHORT ObjLongToShortNameEx2
(
  PSZ         pszLongName,             // ptr to object long name
  CHAR        chDrive,                 // drive to use for shared resource checking
  PSZ pszShortName,                    // ptr to buffer for short name
  UTLCHECKOBJTYPES ObjType,            // type of object
  POBJLONGTOSHORTSTATE pObjState,      // ptr to buffer for returned object state
  BOOL        fReserveName,            // TRUE = reserve created short name
  PBOOL       pfReserved               // points to callers fReserved flag
);



__declspec(dllexport)
BOOL StartBrowser( PSZ pszUrl );       // start the standard Browser
__declspec(dllexport)
VOID EqfDisplayContextHelp( HWND, PHELPSUBTABLE);

// OEM/ASCII to ANSI conversions -- to be removed if full Unicode support implemented
#define ANSI_CP 1
#define OEM_CP  2
__declspec(dllexport)
VOID EQFAnsiToOem  ( PSZ pIn, PSZ pOut );
VOID EQFCPAnsiToOem( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut );
__declspec(dllexport)
VOID EQFOemToAnsi  ( PSZ pIn, PSZ pOut );
VOID EQFCPOemToAnsi( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut );
VOID EQFOemToAnsiBuff( PSZ pIn, PSZ pOut, USHORT usLen );
VOID EQFCPOemToAnsiBuff( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut, USHORT usLen );
VOID EQFAnsiToOemBuff( PSZ pIn, PSZ pOut, USHORT usLen );
VOID EQFCPAnsiToOemBuff( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut, USHORT usLen );

__declspec(dllexport)
ULONG  GetCodePage( USHORT usType );
__declspec(dllexport)
ULONG GetLangCodePage( USHORT usType, PSZ pLanguage );
__declspec(dllexport)
ULONG GetOrgLangCP( PSZ pLanguage, BOOL  fASCII );

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/*                 Public Data Structure Section                      */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/


//------------------------------------------------------------------------------
//   Layout of term entries stored in term length and term offset format
//------------------------------------------------------------------------------
typedef struct _TERMLENOFFS
{
   USHORT  usLength;                   // length of term
   USHORT  usOffset;                   // offset of term data in buffer
} TERMLENOFFS, *PTERMLENOFFS;

/**********************************************************************/
/* Log file functions                                                 */
/**********************************************************************/
#ifdef _DEBUG

#define LOG( s )              Log( s )
#define LOGSTRING( f, s )     LogString( f, s )
#define LOGSHORT( f, i )      LogShort( f, i )
#define LOG2SHORT( f, i, j )  Log2Short( f, i, j )
#define LOGLONG( f, l )       LogLong( f, l )
#define LOGSYSINFO()          LogSysInfo()

void Log( PSZ pszString );
void LogString( PSZ pszFormat, PSZ pszString );
void LogShort( PSZ pszFormat, SHORT sValue );
void Log2Short( PSZ pszFormat, SHORT sValue1, SHORT sValue2 );
void LogLong( PSZ pszFormat, LONG lValue );
void WriteLog( PSZ pszString );
void LogSysInfo( VOID );
#else

#define LOG( s )
#define LOGSTRING( f, s )
#define LOGSHORT( f, i )
#define LOG2SHORT( f, i, j )
#define LOGLONG( f, l )
#define LOGSYSINFO()

#endif

///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///  UNICODE enabled string functions (UTF16 only)                          ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

__declspec(dllexport)
PSZ_W UTF16strcpy( PSZ_W pszTarget, PSZ_W pszSource );
__declspec(dllexport)
PSZ_W UTF16strcat( PSZ_W pszTarget, PSZ_W pszSource );
__declspec(dllexport)
PSZ_W UTF16strncpy(PSZ_W pusTarget, PSZ_W pusSource, LONG lLen);
__declspec(dllexport)
int   UTF16strcmp( PSZ_W pszString1, PSZ_W pszString2 );
int   UTF16strncmpL( PSZ_W pszString1, PSZ_W pszString2, LONG lLen );
int   UTF16strnicmpL( PSZ_W pszString1, PSZ_W pszString2, LONG lLen );
__declspec(dllexport)
int   UTF16strncmp( PSZ_W pszString1, PSZ_W pszString2, USHORT usLen );
__declspec(dllexport)
int   UTF16strnicmp( PSZ_W pszString1, PSZ_W pszString2, USHORT usLen );
__declspec(dllexport)
int   UTF16strlenCHAR( PSZ_W pszString );
__declspec(dllexport)
int   UTF16strlenBYTE( PSZ_W pszString );
int   UTF16strcspn( PSZ_W pszString1, PSZ_W pszString2 );
__declspec(dllexport)
PSZ_W UTF16strchr( PSZ_W pszString, CHAR_W ch );
__declspec(dllexport)
PSZ_W UTF16strstr( PSZ_W pszString1, PSZ_W pszString2 );
__declspec(dllexport)
PSZ_W UTF16strrev( PSZ_W pszString );
__declspec(dllexport)
PSZ   Unicode2ASCII( PSZ_W pszUni, PSZ pszASCII, ULONG ulCP );
__declspec(dllexport)
PSZ_W ASCII2Unicode( PSZ pszASCII, PSZ_W pszUni, ULONG ulCP );
__declspec(dllexport)
PSZ   Unicode2Ansi( PSZ_W pszUni, PSZ pszAnsi, ULONG ulCP );
__declspec(dllexport)
PSZ_W Ansi2Unicode( PSZ pszAnsi, PSZ_W pszUni, ULONG ulCP );
__declspec(dllexport)
ULONG ASCII2UnicodeBuf( PSZ pszASCII, PSZ_W pszUni, ULONG ulLen, ULONG ulCP );
__declspec(dllexport)
ULONG ASCII2UnicodeBufEx( PSZ pszASCII, PSZ_W pszUni, ULONG ulLen, ULONG ulCP, BOOL fMsg, PLONG plRc, PLONG plBytesLeft );
ULONG Ansi2UnicodeBuf( PSZ pszAnsi, PSZ_W pszUni, ULONG ulLen, ULONG ulCP );
__declspec(dllexport)
ULONG   UTF82UnicodeBuf( PSZ pszUTF8, PSZ_W pszUni, LONG Len );
__declspec(dllexport)
ULONG   UTF82UnicodeBufEx( PSZ pszUTF8, PSZ_W pszUni, LONG Len, BOOL fMsg, PLONG plRc, PLONG plBytesLeft );
__declspec(dllexport)
ULONG   Unicode2UTF8Buf( PSZ_W pszUni, PSZ pszUTF8, LONG Len );
__declspec(dllexport)
ULONG   Unicode2UTF8BufEx( PSZ_W pszUni, PSZ pszUTF8, LONG Len, LONG lBufLen, BOOL fMsg, PLONG plRc );

// GQ: The lBufLen parameter in the following two functions specifies the
//     actual size of the pAscii/pAnsi buffer in number of bytes
__declspec(dllexport)
ULONG Unicode2ASCIIBuf( PSZ_W pszUni, PSZ pszASCII, ULONG usLen, LONG lBufLen, ULONG ulCP );
//ULONG Unicode2ASCIIBufEx( PSZ_W pszUni, PSZ pszASCII, ULONG usLen, LONG lBufLen,
//                              ULONG ulCP,BOOL fMsg, PLONG plRc, DWORD dwFlags );
ULONG Unicode2AnsiBuf( PSZ_W pszUni, PSZ pszAnsi, ULONG usLen, LONG lBufLen, ULONG ulCP  );
__declspec(dllexport)
ULONG UtlDirectUnicode2AnsiBuf( PSZ_W pszUni, PSZ pszAnsi, ULONG ulLen, LONG lBufLen,
                             ULONG ulAnsiCP, BOOL fMsg, PLONG plRc );
__declspec(dllexport)
ULONG UtlDirectAnsi2UnicodeBuf( PSZ pszAnsi, PSZ_W pszUni, ULONG ulLen, ULONG ulAnsiCP,
                             BOOL fMsg, PLONG plRc, PLONG plBytesLeft );
__declspec(dllexport)
PSZ   UtlDirectUnicode2Ansi( PSZ_W pszUni, PSZ pszAnsi, ULONG ulAnsiCP );
__declspec(dllexport)
PSZ_W UtlDirectAnsi2Unicode( PSZ pszAnsi, PSZ_W pszUni, ULONG ulAnsiCP );
__declspec(dllexport)
ULONG UtlConvertBuf(USHORT usType, PVOID pvInString, PVOID pvOutString, ULONG ulLen,
                    LONG lBufLen, ULONG ulASCIICP, ULONG ulAnsiCP,
                    BOOL fMsg, PLONG plRc, PLONG plBytesLeft);

__declspec(dllexport)
USHORT usConvertCRLF(
  PCHAR  pcInput,            // pointer to input character array
  USHORT usInputLen,         // length of input area
  PCHAR  pcOutput,           // pointer to result character array
  BOOL   fMode);             // flag indicating whether CRLF or LF is desired
                             // as the output format; TRUE means only LF is
                             // used. FALSE uses CRLF as the line end code
__declspec(dllexport)
USHORT usConvertCRLFW(PCHAR_W pcInput,     // pointer to character input area
                      USHORT  usInputLen,  // length of input area
                      PCHAR_W pcOutput,    // pointer to output area
                      BOOL    fMode);      // conversion mode :

__declspec(dllexport)
BOOL EQFIsDBCSChar( CHAR_W c, ULONG ulCP);
//USHORT EQFGetCPOem();
PSZ_W UTF16strccpy( PSZ_W pszTarget, PSZ_W pszSource, CHAR_W chStop );
__declspec(dllexport)
int   UTF16stricmp(PSZ_W a, PSZ_W b);
__declspec(dllexport)
PSZ_W UTF16memset( PSZ_W pusString, CHAR_W c, USHORT usNum );
__declspec(dllexport)
PSZ_W UTF16memsetL( PSZ_W pusString, CHAR_W c, ULONG ulNum );

ULONG UtlQueryTextW( HWND hwnd, LONG id, PSZ_W buffer, LONG usLen );
LONG  UtlQueryTextHwndW( HWND hwnd, PSZ_W buffer, LONG Len );
BOOL UtlSetWindowTextW( HWND hwnd, PSZ_W pText );
BOOL UtlSetTextW( HWND hwnd, USHORT id, PSZ_W pText );
LONG UtlCBInsertItemHwndW( HWND hwnd, PSZ_W pText );
LONG UtlInsertItemHwndW( HWND hwnd, WPARAM index, PSZ_W pText );
LONG UtlCBFindStringExact( HWND hwnd, SHORT sPos, PSZ_W pText );
LONG UtlGetTextW( HWND hwnd, USHORT id, PSZ_W pText, LONG len );
LONG UtlQueryItemTextW( HWND hwnd, USHORT id,  SHORT index, PSZ_W buffer, USHORT usLen );
LONG UtlCBQueryItemTextW( HWND hwnd, SHORT index, PSZ_W buffer, USHORT usLen );
__declspec(dllexport)
USHORT UtlFindAndShowConversionError
(
  PSZ    pszBuffer,
  ULONG  ulLen,
  ULONG  ulCP
);

// Generic check if instance of TM or ITM is currently running
// If not create semaphore to disable another instance of us to try to start
// otherwise display messagebox indicating the situation...
__declspec(dllexport)
BOOL UtlIsAlreadyRunning(HANDLE *phTM_Sem);


#ifdef __cplusplus
extern "C" {
#endif
__declspec(dllexport) void InitTMPluginWrapper();
__declspec(dllexport) void InitDictPluginWrapper();
__declspec(dllexport) void InitMarkupPluginMapper();
__declspec(dllexport) void InitDocumentPluginMapper();

typedef EQF_BOOL (__cdecl *PFNCHECKSEGEXIT)( PSZ, PSZ, PSZ, PEQF_BOOL, EQF_BOOL);
typedef EQF_BOOL (__cdecl *PFNCHECKSEGEXITW)( PSZ_W, PSZ_W, PSZ_W, PEQF_BOOL, EQF_BOOL);
typedef EQF_BOOL (__cdecl *PFNCHECKSEGEXEXITW)( PSZ_W, PSZ_W, PSZ_W, PEQF_BOOL, LONG, ULONG, EQF_BOOL);

typedef EQF_BOOL (__cdecl *PFNSHOWTRANS)( LONG, HWND );
typedef EQF_BOOL (__cdecl *PFNTOCGOTO)( LONG, HWND );

typedef EQF_BOOL (__cdecl *PFNGETSEGCONTTEXT)( PSZ_W, PSZ_W, PSZ_W, PSZ_W, LONG, ULONG );
typedef EQF_BOOL (__cdecl *PFNFORMATCONTTEXT)( PSZ_W, PSZ_W );
#ifdef __cplusplus
}
#endif


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/* Conditional Includes of Header Files                               */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

/**********************************************************************/
/* Tagtable defines and functions                                     */
/**********************************************************************/
#if defined(INCL_EQF_TAGTABLE)
  #include <EQFTAG.H>
#endif

/**********************************************************************/
/* General Transl. Memory functions                                   */
/**********************************************************************/
#if defined(INCL_EQF_TM)
  #include <EQFTM.H>
#endif

/**********************************************************************/
/* Remote Transl. Memory functions (U-Code)                           */
/**********************************************************************/
#if defined(INCL_EQF_TMREMOTE)
  #include <EQFTMREM.H>
#endif

/**********************************************************************/
/* Public translation processor functions                             */
/**********************************************************************/
#if defined(INCL_EQF_TP)
  #include <EQFTP.H>
#endif

/**********************************************************************/
/* Morphologic functions                                              */
/**********************************************************************/
#if defined(INCL_EQF_MORPH)
  #include <EQFMORPH.H>
#endif

/**********************************************************************/
/* Dictionary access functions (Asd...)                               */
/**********************************************************************/
#if defined(INCL_EQF_ASD)
  #include <EQFDASD.H>
#endif

/**********************************************************************/
/* Low level dict. access functions (Nlp/Dam)                         */
/**********************************************************************/
#if defined(INCL_EQF_DAM)
  #include <EQFQDAM.H>
#endif

/**********************************************************************/
/* Dictionary data encoding functions                                 */
/**********************************************************************/
#if defined(INCL_EQF_LDB)
  #include <EQFLDB.H>
#endif

/**********************************************************************/
/* Analysis functions                                                 */
/**********************************************************************/
#if defined(INCL_EQF_ANALYSIS)
  #include <EQFTA.H>
#endif

/**********************************************************************/
/* Slider utility functions                                           */
/**********************************************************************/
#if defined(INCL_EQF_SLIDER)
  #include <EQFSLDR.H>
#endif

/**********************************************************************/
/* Word count functions                                               */
/**********************************************************************/
#if defined(INCL_EQF_WCOUNT)
  #include <EQFWCNT.H>
#endif

/**********************************************************************/
/* Dictionary handler functions                                       */
/**********************************************************************/
__declspec(dllexport) USHORT DicFillDictLB( HWND, HWND, PSZ, BOOL );
__declspec(dllexport) VOID DictionaryImport( HWND, PSZ, BOOL, HWND );
__declspec(dllexport) VOID DictionaryOrganize( HWND, PSZ, PVOID );
__declspec(dllexport) BOOL DicGetCheckPassword( PSZ pszPassword, PULONG pulPassword, BOOL fCheckPassword );

/**********************************************************************/
/* Dictionary filter functions                                        */
/**********************************************************************/
#if defined(INCL_EQF_FILT)
  #include <EQFFILT.H>
#endif

/**********************************************************************/
/* Terminology list functions                                         */
/**********************************************************************/
#if defined(INCL_EQF_LIST)
  #include <EQFLIST.H>
#endif

/**********************************************************************/
/* Print functions                                                    */
/**********************************************************************/
#if defined(INCL_EQF_PRINT)
  #include <EQFQPRU.H>
#endif

/**********************************************************************/
/* Dialog utilities                                                   */
/**********************************************************************/
#if defined(INCL_EQF_DLGUTILS)
  #include <EQFUTDLG.H>
#endif

/**********************************************************************/
/* Dictionary print functions                                         */
/**********************************************************************/
#if defined(INCL_EQF_DICTPRINT)
  #include <EQFQDPR.H>
#endif

/**********************************************************************/
/* Folder list and document list functions                            */
/**********************************************************************/
#if defined(INCL_EQF_FOLDER)
  #include <EQFFOL.H>
#endif

// XLATE markups (markups which can't be handled within this function)
#define XLATEOS2_MARKUP "XLATEOS2"
#define XLATEWIN_MARKUP "XLATEWIN"

// support additional pages in document import/export
typedef struct _EXTRAIDA
{
   CHAR     szFileName[ MAX_PATH144 ];  // filename to be used
   HWND     hwndExpListbox;             // handle of listbox containing selected docs
   OBJNAME  szFolderObjName;            // folder object name
   PVOID    pIda;                       // product specifics
   USHORT   usExtraPage;                // number of extra page
   HWND     hwnd;                       // handle of parent dialog
   OBJNAME  szParentObjName;            // parent (sub)folder object name
} EXTRAIDA, *PEXTRAIDA;



// defines for SGML Formats
#define SGMLFORMAT_ANSI     1
#define SGMLFORMAT_ASCII    2
#define SGMLFORMAT_UNICODE  3

// filter for SGML formats (same seqence as SGML format defines required)
#define SGML_FORMAT_FILTERS "SGML ANSI\0*.*\0SGML ASCII\0*.*\0SGML UTF-16\0*.*\0\0\0"

// Prefix for Unicode Text Files (at least WordPad requires this prefix...)
#define UNICODEFILEPREFIX "\xFF\xFE"
#define UTF8FILEPREFIX    "\xEF\xBB\xBF"

// some defines for process handling
#define PROC_TABLE_FIRST   1
#define PROC_TABLE_NEXT    2
#define PROC_TABLE_CLOSE   3

// and operating system information
#define OP_NO_WINDOWS      0
#define OP_WIN31X          1
#define OP_WINDOWS         2
#define OP_WINDOWSNT       3
#define OP_WINDOWS2K       4
#define OP_WINXP           5

/*------------------------------------------------------------------------------
* Auto Refresh
*-----------------------------------------------------------------------------*/
#define EQFNDDE_SHFLAG     "/SHAREMEM/EQF/NDDEREF"   // name of shared memory
#define EQF_REFR_FOLLIST 0x1
#define EQF_REFR_DICLIST 0x2
#define EQF_REFR_MEMLIST 0x4


#define MAX_REVMARK_SIZE  256          // max size of revision marks

//added prototypes to avoid warnings!!
__declspec(dllexport)
BOOL EqfDocPropsToHtml(PSZ pszDocumentObj,HWND hwndParent);
__declspec(dllexport)
BOOL EqfDictPropsToHtml(PSZ pszDictionaryObj,HWND hwndParent);


// cv
/*-----------------------------------------------------------------------------
*   defines for cut/copy/paste and drag/drop functionality using clipboard
*-----------------------------------------------------------------------------*/
// custom clipboard format
#define CF_DOCLIST "EQFDOCLIST"

// document spec
typedef struct _DocProp
{
  CHAR szLongName[MAX_LONGFILESPEC];
  CHAR szName[MAX_FILESPEC];
} DOCPROP,*PDOCPROP;

// document list
typedef struct _DocList
{
  LONG  lBytes;                                   // structure length
  SHORT sAction;                                  // user action, cut or copy
  SHORT sCount;                                   // document count
  CHAR  szFolder[MAX_LONGFILESPEC];               // source folder
  DOCPROP aDoc[1];                                // document specs, short and long name
} DOCLIST,*PDOCLIST;

__declspec(dllexport)
BOOL  SubFolderPropsToHtml( PSZ pszObjName, HWND hwnd, BOOL fIsSubFolder );

__declspec(dllexport)
BOOL
NTMCompareBetweenTokens
(
    PSZ_W pD1,
    PSZ_W pD2,
    PSZ   pszMarkup,
    SHORT sLangID,
    ULONG ulSrcCP,
    PBOOL pfStringEqual
);

__declspec(dllexport)
ULONG EQFGETDLLVERSION( void );
__declspec(dllexport)
ULONG EQFGETEXEVERSION( void );

__declspec(dllexport)
int CALLBACK UtlBrowseForFolderCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData);

__declspec(dllexport)
BOOL UtlIsLanguageSupported
(
  PSZ    pszLanguage,
  BOOL   fMsg,
  HWND   hwndErrMsg
);

// helper function for automatic subst in analysis and editor
__declspec(dllexport) BOOL TACheckAndAdaptLFChanges
(
	PSZ_W          pSegData,
	PSZ_W          pszMatch,
	BOOL           fMsg
);

// name of mutex semaphore for property access
#define PROP_ACCESS_MUTEX "EQFPRO00-PropAccess"

// keep other process from doing property related stuff..
// GETMUTEX to create a mutex
#define GETMUTEX(hMutexSem) \
{ hMutexSem = OpenMutex( MUTEX_ALL_ACCESS, TRUE, PROP_ACCESS_MUTEX );                \
  if ( hMutexSem == NULL) hMutexSem = CreateMutex( NULL, FALSE, PROP_ACCESS_MUTEX ); \
  if ( hMutexSem ) WaitForSingleObject( hMutexSem, INFINITE );                       \
}

#define RELEASEMUTEX(hMutexSem)   \
{ if ( hMutexSem )                                           \
  {  ReleaseMutex( hMutexSem ); CloseHandle( hMutexSem ); }  \
}

__declspec(dllexport)
MRESULT UtlInvokeHelp();
__declspec(dllexport)
BOOL    UtlIsHighContrast();

// function checking a given codepage value by doing a test conversion
__declspec(dllexport)
BOOL UtlIsValidCP( ULONG ulCP );


__declspec(dllexport)
USHORT UtlFuncRenameObject( USHORT usMode, PSZ pszName, PSZ pszNewName,LONG lOptions );
__declspec(dllexport)
USHORT UtlFuncGetShortName( USHORT ObjectType, PSZ pszLongName, PSZ pszShortName );

__declspec(dllexport)
void UtlLogStart( PSZ pszLogFilePrefix );
__declspec(dllexport)
void UtlLogStop();
void UtlLogFlush();
__declspec(dllexport)
void UtlLogWriteString( PSZ pszFormat, PSZ pszString );
__declspec(dllexport)
void UtlLogWriteString2( PSZ pszFormat, PSZ pszString1, PSZ pszString2 );
__declspec(dllexport)
void UtlLogWrite( PSZ pszString );

__declspec(dllexport)
BOOL GetStringFromRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszBuffer, int iBufSize, PSZ pszDefault );
__declspec(dllexport)
int GetIntFromRegistry( PSZ pszAppl, PSZ pszKey, int iDefault );
__declspec(dllexport)
BOOL WriteStringToRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszValue );
__declspec(dllexport)
BOOL WriteIntToRegistry( PSZ pszAppl, PSZ pszKey, int iValue );


// some prefefined key for usage within the additional data section
#define MATCHSEGID_KEY L"MatchSegID" 
#define MATCHSEGID_ATTR L"ID" 

// utility functions for the processing of proposal additional data

// functions and defines for the access to additional data of proposals
typedef PSZ_W HADDDATAKEY;

// Search a specific key in the additional memory data
__declspec(dllexport)
HADDDATAKEY MADSearchKey( PSZ_W pAddData, PSZ_W pszKey );

// retrieve the data associated with a specific key
__declspec(dllexport)
BOOL MDAGetValueForKey( HADDDATAKEY pKey, PSZ_W pszBuffer, int iBufSize, PSZ_W pszDefault );

// get the value for a specific attribute
__declspec(dllexport)
BOOL MADGetAttr( HADDDATAKEY pKey, PSZ_W pszAttrName, PSZ_W pszBuffer, int iBufSize, PSZ_W pszDefault );

// delete a key and its associated data
__declspec(dllexport)
BOOL MADDeleteKey( HADDDATAKEY pKey );

// Add a match segment ID to the additional data section
BOOL MADAddMatchSegID( PSZ_W pszAddData, PSZ_W pszMatchIDPrefix, ULONG ulNum, BOOL fForce );

// prepare the match segment ID prefix using the provided TM_ID and StoreID
BOOL MADPrepareMatchSegIDPrefix( PSZ pszTM_ID, PSZ pszStoreID, PSZ pszMatchID );


__declspec(dllexport)
VOID    CloseFile( HFILE * );

__declspec(dllexport)
void UtlSaveLastUsedString( HWND hwndDlg, int iID, char *pszName, int iMaxNumOfStrings );

__declspec(dllexport)
void UtlLoadLastUsedStrings( HWND hwndDlg, int iID, char  *pszName );

__declspec(dllexport)
BOOL UtlAddMenuItem( const char *pszMenuName, const char *pszMenuItem, int iMenuItemID, BOOL isSubMenu );

__declspec(dllexport)
BOOL UtlDeleteMenuItem( const char *pszMenuName, int iMenuItemID );

// Input/output data for the UtlColorChooser dialog
typedef struct _COLORCHOOSERDATA
{
  SHORT             sID;               // ID to be used for dialog, 0 = use standard ID
  CHAR              szTitle[256];      // text to be used for dialog caption
  HWND              hwndOwner;         // owner window for the color chooser dialog
  struct
  {
    CHAR            szElement[80];     // name of the text element
    COLORREF        cForeground;       // current foreground color
    COLORREF        cBackground;       // current background color
    COLORREF        cDefaultForeground;// default foreground color
    COLORREF        cDefaultBackground;// default background color
  } ColorSetting[30];                  // table of color settings for the various elements
} COLORCHOOSERDATA, *PCOLORCHOOSERDATA;


// let the user select the color settings for a group of items
__declspec(dllexport)
USHORT UtlColorChooserDlg
(
  PCOLORCHOOSERDATA pColorChooserData  // pointer to caller's color chooser data structure
);

/**
* \brief Count the number of words in a given string
*
*	\param pszMarkup name of the markup table to be used for the recognition of in-line tags,
*  if this parameter is NULL no in-line tag recognition will be performed
*	\param pszLanguage OpenTM2 name for the language of the given text
*	\param pszText null-terminated string containing the text to be counted, the encoding is UTF-16
* \param pulWords points to an unsigned long value receiving the number of words in the text
* \param pulInlineTags points to an unsigned long value receiving the number of inline tags in the text
*	\returns 0 if successful or an error code
*	
**/
__declspec(dllexport)
unsigned short CntFuncCountWordsInstring
(
  char        *pszMarkup,              // name of the markup table to be used for the recognition of in-line tags
  char        *pszLanguage,            // OpenTM2 name for the language of the given text
  wchar_t     *pszText,                // null-terminated string containing the text to be counted, the encoding is UTF-16
  unsigned long *pulWords,             // points to an unsigned long value receiving the number of words in the text
  unsigned long *pulInlineTags         // points to an unsigned long value receiving the number of inline tags in the text
);

/**
* \brief Name used for the calculation report data file
**/
#define CALREPORTDATAFILE "BINCLCRPT.DAT"

/**
* \brief Layout of calculation report data
**/
typedef struct _CALCREPORTBINDATA
{
  // actual words per column
  unsigned long ulAnaAustoSubst;
  unsigned long ulAnaAustoSubst2;
  unsigned long ulEditAustoSubst;
  unsigned long ulEditExact;
  unsigned long ulEditReplace;
  unsigned long ulEditFuzzy5070;
  unsigned long ulEditFuzzy7190;
  unsigned long ulEditFuzzy91;
  unsigned long ulEditMachine;
  unsigned long ulEditManual;
  unsigned long ulNotTranslated;
  unsigned long ulUnused[10];  // room for additional word counts

  // payable words 
  unsigned long ulPayable;  // payable words as integer number
  float         flPayable;  // payable words as a floating number 

  // profile information
  char          szProfile[40];                 

  // room for future enhancements
  char          szUnUsed[256];
} CALCREPORTBINDATA, *PCALCREPORTBINDATA;

#endif


// These values are used to define the action to take when 
// importing a memory and an unknown markup table is encountered.
#define   MEMIMP_MRKUP_ACTION_CANCEL   0
#define   MEMIMP_MRKUP_ACTION_SKIP     1
#define   MEMIMP_MRKUP_ACTION_RESET    2

// segmented file related function prototypes
__declspec(dllexport) VOID SegFileFreeDoc( PVOID *ppvDoc );

__declspec(dllexport) int XSLTConversion( PSZ pszXmlFile, PSZ pszHtmlFile, PSZ pszStyleSheet, BOOL fMsg, HWND hwndErrMsg );

#define  MAX_LIST_FILES   10000
__declspec(dllexport) BOOL UtlListOfFiles ( PSZ **, PSZ , PSZ **);

#define FUZZY_THRESHOLD_0                  0.5 // Theshold for different fuzzy classes
#define FUZZY_THRESHOLD_1                  0.7 // Theshold for different fuzzy classes
#define FUZZY_THRESHOLD_2                  0.9 // Theshold for different fuzzy classes


