// #define LOG_NETACCESS
//+----------------------------------------------------------------------------+
//|  EQFUTFIL.C - EQF general services                                         |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//+----------------------------------------------------------------------------+

/**********************************************************************/
/* Include files                                                      */
/**********************************************************************/
#define INCL_DEV
#include <eqf.h>                  // General Translation Manager include file

#include <time.h>                 // C time related functions

#include "EQFUTPRI.H"                  // private utility header file

static USHORT usLastDosRc;             // buffer for last DOS return code

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlQCurDisk            Query current disk drive          |
//+----------------------------------------------------------------------------+
//|Description:       Similar to DosQCurDisk. Returns drive letter of current  |
//|                   disk                                                     |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   BYTE                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       drive letter of current disk                             |
//+----------------------------------------------------------------------------+
//|Samples:           curletter = UtlQCurDisk();                               |
//|                   if A:, B: and C: are available and C: is the current     |
//|                   drive then curletter is loaded with 'C'                  |
//+----------------------------------------------------------------------------+
//|Function flow:     get current drive using DosQCurDisk                      |
//|                   return to drive letter converted return code             |
//+----------------------------------------------------------------------------+

BYTE UtlQCurDisk()
{
    static CHAR szCurDir[MAX_PATH+100];

    DosError(0);
    if ( GetCurrentDirectory( sizeof(szCurDir), szCurDir ) == 0 )
    {
      szCurDir[0] = EOS;
    } /* endif */
    DosError(1);

    return (BYTE)szCurDir[0];
}

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlGetDriveList        Get list of drive letters         |
//+----------------------------------------------------------------------------+
//|Description:       Load string with drive letters of available drives       |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT UtlGetDriveList( BYTE *szList)                    |
//+----------------------------------------------------------------------------+
//|Parameters:        1. BYTE * - array of bytes to contain drive letters      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       - index of letter of current drive within list           |
//+----------------------------------------------------------------------------+
//|Samples:           curixletter = UtlDriveList( str);                        |
//|                   if A:, B: and C: are available and C: is the current     |
//|                   drive then str is loaded "ABC" and curixletter is 2      |
//+----------------------------------------------------------------------------+
//|Function flow:     get number of floppy drives usinf DosDevConfig           |
//|                   get drive bitmap using DosQCurDisk                       |
//|                   loop through bitmap and generate drive letters for all   |
//|                    existing drives                                         |
//|                   return index of current drive                            |
//+----------------------------------------------------------------------------+

USHORT UtlGetDriveList( BYTE *szList)
{
    BYTE   bDrive;                     // currently logged drive
    WORD   wReturn;                    // return value from GetDriveType
    register int i;

    for (i=0, wReturn=0; i<26;i++)
    {
      CHAR szRoot[4] = "C:\\";
      szRoot[0] = (CHAR)('A' + i);
      wReturn = (WORD)GetDriveType( szRoot );
      switch ( wReturn )
      {
        case DRIVE_REMOVABLE :
        case DRIVE_FIXED   :
        case DRIVE_REMOTE  :
        case DRIVE_CDROM   :
        case DRIVE_RAMDISK :
          *szList++ = (BYTE) ('A' + i);
          break;

        case DRIVE_UNKNOWN :
        case DRIVE_NO_ROOT_DIR :
        default:
          break;
      } /* endswitch */
    } /* endfor */

    *szList = '\0';

    bDrive = UtlQCurDisk();
    return( (USHORT) (bDrive - 'A' + 1));   // set index relative to 0
}

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlGetLANDriveList        Get list of LAN drive letters  |
//+----------------------------------------------------------------------------+
//|Description:       Load string with drive letters of LAN drives or LAN      |
//|                   capable drives                                           |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT UtlGetLANDriveList( BYTE *szList)                 |
//+----------------------------------------------------------------------------+
//|Parameters:        1. BYTE * - array of bytes to contain drive letters      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       - index of letter of current drive within list           |
//+----------------------------------------------------------------------------+

USHORT UtlGetLANDriveList( PBYTE pszList )
{
  PBYTE pSource, pTarget;              // pointer for drive list processing
  SHORT sDriveType;                    // type of currently tested drive
  USHORT usRC;

  usRC = UtlGetDriveList( pszList);    // get all available drives

  pSource = pTarget = pszList;         // start at begin of drive list

  while ( *pSource != NULC )           // while not end of list ....
  {
    sDriveType = UtlDriveType( *pSource );
    switch ( sDriveType )
    {
      case DRIVE_FIXED  :              // check if fixed disk is shared
        /**************************************************************/
        /* Note: Under Windows fixed disks are always treated as      */
        /*       shared drives as the UtlIsDriveShared function does  */
        /*       not work under Win95 anymore (the functions          */
        /*       WNetGetDirectoryType always returns a not-supported  */
        /*       return code) and so there is no way to distinguish   */
        /*       local disks from shared local disks                  */
        /**************************************************************/
//      if ( UtlIsDriveShared( *pSource ) )
//      {
          *pTarget++ = *pSource++;     // leave shared drives in list
//      }
//      else
//      {
//        *pSource++;                  // ignore drive
//      } /* endif */
        break;

      case DRIVE_REMOTE :
        *pTarget++ = *pSource++;       // leave drive in list
        break;

      default :
        pSource++;                     // ignore drive
        break;
    } /* endswitch */
  } /* endwhile */
  *pTarget = NULC;                     // terminate new drive list

  return( usRC );
} /* end of function UtlGetLANDriveList */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlLoadFileNames                                         |
//+----------------------------------------------------------------------------+
//|Function call:     UtlOpen(                       hfOpen, PUSHORT pusAction,|
//|                            ULONG ulFSize, USHORT usAttr, USHORT fsOpenFlags|
//|                            USHORT fsOpenMode, ULONG ulReserved, BOOL fMsg);|
//+----------------------------------------------------------------------------+
//|Description:       Interface function to UtlLoadFileNamesHwnd               |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ - path to search for files (or directories)       |
//|                   2. USHORT - file attributes (NORMAL, DIRECTORY, etc)     |
//|                   3. HWND - handle of target list box                      |
//|                   4. USHORT - flags for special formats of file names      |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       number   number of file names inserted                   |
//|                   UTLERROR in case of errors                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtLoadFileNamesHwnd                                 |
//+----------------------------------------------------------------------------+
SHORT UtlLoadFileNames( PSZ pszSearch, USHORT atrb, HWND hlbox, USHORT flg)
{
   return( UtlLoadFileNamesHwnd( pszSearch,
                                 atrb,
                                 hlbox,
                                 flg,
                                 NULLHANDLE ) );
}
//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlLoadFileNamesHwnd   Load file names into a list box   |
//+----------------------------------------------------------------------------+
//|Description:       Load file names found in specified path into a list box  |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ - path to search for files (or directories)       |
//|                   2. USHORT - file attributes (NORMAL, DIRECTORY, etc)     |
//|                   3. HWND - handle of target list box                      |
//|                   4. USHORT - flags for special formats of file names      |
//|                   5. HWND - window handle                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       number   number of file names inserted                   |
//|                   UTLERROR in case of errors                               |
//+----------------------------------------------------------------------------+
//|Samples:           n = UtlLoadFileNames( "d:\eqfd", FILE_DIRECTORY,         |
//|                                         hLbox,                             |
//|                                         NAMFMT_NOEXT | NAMFMT_TOPSEL);     |
//+----------------------------------------------------------------------------+
//|Function flow:     initialize search using UtlFindFirst                     |
//|                   while ok and files found                                 |
//|                     if file does not match requested file type then        |
//|                       ignore file                                          |
//|                     elseif file name is CURRENT_DIR_NAME                   |
//|                       ignore file                                          |
//|                     elseif file name is PARENT_DIR_NAME                    |
//|                       if no directories requested or                       |
//|                          NAMFMT_ROOT flag has been specified then          |
//|                          ignore file                                       |
//|                       endif                                                |
//|                     else                                                   |
//|                       discard file extension if requested (NAMFMT_NOEXT)   |
//|                     endif                                                  |
//|                     if file is not to be ignored then                      |
//|                       insert file name into listbox                        |
//|                     endif                                                  |
//|                     search next file using UtlFindNext                     |
//|                   endwhile                                                 |
//|                   select first listbox item if NAMFMT_TOPSEL is set        |
//|                   return number of inserted items or UTLERROR in case of   |
//|                    errors                                                  |
//+----------------------------------------------------------------------------+
SHORT UtlLoadFileNamesHwnd( PSZ pszSearch, USHORT atrb, HWND hlbox,
                            USHORT flg, HWND hwnd)
{
  FILEFINDBUF ResultBuf;               // DOS file find struct
  USHORT  usCount = 1;
  HDIR    hDirHandle = HDIR_CREATE;    // DosFind routine handle
  USHORT  usTotal;
  USHORT  usRC;                        // return code of Dos... alias Utl...
  char    *p, *p2;
  USHORT  fdir = atrb & FILE_DIRECTORY;
  SHORT   sRetCode;                    // return code of function
  BOOL    fCombo;                      // is-a-combobox flag
  BOOL    fMsg = !(flg & NAMFMT_NOERROR);

  ISCOMBOBOX( hlbox, fCombo );

  usRC = UtlFindFirstHwnd( pszSearch, &hDirHandle, atrb, &ResultBuf,
                           sizeof( ResultBuf), &usCount, 0L,
                           fMsg, hwnd);

  while ( usCount && !usRC )
  {
     p2 = RESBUFNAME(ResultBuf);
     if( fdir && !(fdir & ResultBuf.dwFileAttributes) )
     {
       /***************************************************************/
       /* File does not match requested file type                     */
       /***************************************************************/
       p2 = NULL;                    // do not insert into listbox
     }
     else if( strcmp( p2, CURRENT_DIR_NAME ) == 0 )
     {
       /***************************************************************/
       /* File is current directory (".")                             */
       /***************************************************************/
         p2 = NULL;                    // do not insert into listbox
     }
     else if ( strcmp( p2, PARENT_DIR_NAME ) == 0 )
     {
       /***************************************************************/
       /* Process parent directory name ".."                          */
       /***************************************************************/
       if ( !fdir || (flg & NAMFMT_NOROOT) )
       {
          p2 = NULL;               // do not insert into listbox
       }
       else
       {
          p = strchr( pszSearch, BACKSLASH );
          if ( (p == NULL) || (p == strrchr( pszSearch, BACKSLASH) ) )
          {
             // root directory shouldn't have any parent directory !!!
             p2 = NULL;            // do not insert into listbox
          } /* endif */
       } /* endif */
     }
     else
     {
       /***************************************************************/
       /* Process file names using the supplied process flags         */
       /***************************************************************/
       if( flg & NAMFMT_NOEXT )
       {
          if( (p = strrchr( p2, DOT)) != NULL )
          {
             *p = '\0';
          } /* endif */
       } /* endif */

       if ( flg & NAMFMT_NODIR )
       {
          p2 = ( (p=strrchr( p2, BACKSLASH))!=NULL ) ? p + 1 : p2;
       }
       else if ( flg & NAMFMT_NODRV )
       {
          p2 = ( *(p2+1) == COLON) ? p2 + 2 : p2;
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Insert string into listbox                                    */
     /*****************************************************************/
     if ( p2 )
     {
        if ( fCombo )
        {
          CBINSERTITEMHWND( hlbox, p2 );
        }
        else
        {
          INSERTITEMHWND( hlbox, p2 );
        } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Get next file                                                 */
     /*****************************************************************/
     usRC = UtlFindNextHwnd( hDirHandle, &ResultBuf, sizeof( ResultBuf),
                                 &usCount, fMsg , hwnd);

  } /* endwhile */

  // close file search handle
  if ( hDirHandle != HDIR_CREATE ) UtlFindClose( hDirHandle, FALSE );


  /********************************************************************/
  /* Get total number of list box entries                             */
  /********************************************************************/
  if ( fCombo )
  {
    usTotal = (USHORT)CBQUERYITEMCOUNTHWND( hlbox );
  }
  else
  {
    usTotal = (USHORT)QUERYITEMCOUNTHWND( hlbox );
  } /* endif */

  /********************************************************************/
  /* Select first item on request                                     */
  /********************************************************************/
  if ( (flg & NAMFMT_TOPSEL) && usTotal )
  {
    if ( fCombo )
    {
      CBSELECTITEMHWND(hlbox, 0);
    }
    else
    {
      SELECTITEMHWND(hlbox, 0);
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Build return code                                                */
  /********************************************************************/
  if ( ( usRC == 0 )                   ||
       ( usRC == ERROR_NO_MORE_FILES ) ||
       ( usRC == ERROR_FILE_NOT_FOUND ) )
  {
     sRetCode = usTotal;
  }
  else
  {
     sRetCode = UTLERROR;
  } /* endif */

  /********************************************************************/
  /* Return return code to caller                                     */
  /********************************************************************/
  return( sRetCode );

}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:    UtlDriveButtonWP        Window proc for drive buttons     |
//+----------------------------------------------------------------------------+
//|Function call:    UtlDriveButtonWP( HWND hwnd, USHORT msg, WPARAM mp1,      |
//|                                      LPARAM mp2 )                          |
//+----------------------------------------------------------------------------+
//|Description:      Window procedure for the subclassed drive icon buttons.   |
//|                  This procedures handles initialization, painting and      |
//|                  highlighting of drive icon buttons. All other functions   |
//|                  are performed by the original button procedure.           |
//+----------------------------------------------------------------------------+
//|Input parameter:  HWND    hwnd     handle of window                         |
//|                  USHORT  msg      type of message                          |
//|                  WPARAM  mp1      first message parameter                  |
//|                  LPARAM  mp2      second message parameter                 |
//+----------------------------------------------------------------------------+
//|Returncode type:  MRESULT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:      depends on message type                                   |
//|                  normal return codes are:                                  |
//|                  TRUE  = message has been processed                        |
//|                  FALSE = message has not been processed                    |
//+----------------------------------------------------------------------------+
//|Function flow:     switch msg;                                              |
//|                     case WM_DRIVEBUTTON_INIT:                              |
//|                       remember old button procedure                        |
//|                       store current selection and drive ID in QWL_USER     |
//|                     case WM_DRIVEBUTTON_SELECT:                            |
//|                       change selection flag in QWL_USER                    |
//|                       invalidate button rectangle to force a repaint       |
//|                     case WM_PAINT:                                         |
//|                       get system bitmap SBMP_DRIVE                         |
//|                       draw bitmap in current selection and enabled style   |
//|                       delete bitmap                                        |
//|                       draw drive letter text                               |
//|                     default:                                               |
//|                       pass message to old button procedure                 |
//|                   endswitch;                                               |
//+----------------------------------------------------------------------------+

MRESULT APIENTRY UTLDRIVEBUTTONWP
(
   HWND hwnd,                          // handle of window
   USHORT msg,                         // type of message
   WPARAM mp1,                         // first message parameter
   LPARAM mp2                          // second message parameter
)
{
    static  PFNWP oldButtonProc;
    RECT  rcl;
    BYTE    driveid;
    BOOL    selected;
    BOOL    fEnabled;                  // TRUE <=> drive button is enabled
    USHORT  usBitmapStyle;             // bitmap draw style
    USHORT  usTextStyle;               // text draw style


    switch( msg )
    {
      case WM_DRIVEBUTTON_INIT:
        // mp2 = oldButtonProc
        // mp1 = SHORT1 = drive letter
        oldButtonProc = (PFNWP)mp2;
        {
          /************************************************************/
          /* store drive and selection status as button text          */
          /************************************************************/
          CHAR chText[4];

          chText[0] = (CHAR)(SHORT1FROMMP1(mp1));
          chText[1] = ':';
          chText[2] = EOS;
          SetWindowText( hwnd, chText );
          SetWindowLong( hwnd, GWL_USERDATA, (LONG)(CHAR)UtlDriveType( chText[0] )  );
        }
        return( 0L);

      case WM_DRIVEBUTTON_SELECT:
        //  mp1 = SHORT1 = TRUE = select drive
          {
            LONG lButtonInfo = GetWindowLong( hwnd, GWL_USERDATA );
            
            if ( SHORT1FROMMP1( mp1 ) )
            {
              lButtonInfo = (lButtonInfo & 0xFFFF) | 0x10000;
            }
            else
            {
              lButtonInfo = lButtonInfo & 0xFFFF;
            } /* endif */
            SetWindowLong( hwnd, GWL_USERDATA, lButtonInfo );
            InvalidateRgn( hwnd, NULL, FALSE);
          }
          return( 0L);

      case WM_PAINT:
        fEnabled = WinIsWindowEnabled( hwnd );
          {
            PAINTSTRUCT  ps;
            BITMAP  bm;                  // bitmap structure
            HBITMAP  hbm = (HBITMAP) NULL;
            HDC      hdc;                // device context
            CHAR    chWork[2];           // work structure
            POINTL  pointl;
            CHAR    chText[4];
            USHORT  usDriveType;
			HMODULE hResMod;
            LONG lButtonInfo = GetWindowLong( hwnd, GWL_USERDATA );

			hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            GetWindowText( hwnd, chText, sizeof(chText) );
            driveid = (BYTE) chText[0];
            selected = ((lButtonInfo & 0x10000) == 0x10000);
            usDriveType = (USHORT)(lButtonInfo & 0xFFFF);
            switch ( usDriveType )
            {
              case  DRIVE_REMOVABLE:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFFD_BITMAP ));
                break;
              case  DRIVE_FIXED:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFHD_BITMAP ));
                break;
              case  DRIVE_REMOTE:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFNET_BITMAP ));
                break;
              case DRIVE_CDROM:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFCD_BITMAP ));
                break;
              case DRIVE_RAM:
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFRAM_BITMAP ));
                break;
              default :
                hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFFD_BITMAP ));
                break;
            } /* endswitch */

            hdc = BeginPaint( hwnd, &ps );
            GetClientRect( hwnd, &rcl );

            if ( hbm  )
            {
              HBRUSH hBrush = GetSysColorBrush( selected ? COLOR_HIGHLIGHT : COLOR_BTNFACE );
              FillRect( hdc, &rcl, hBrush );

              GetObject( hbm, sizeof( BITMAP ), (LPSTR) &bm );
              pointl.x = 2;

              if ( (rcl.bottom - rcl.top) > bm.bmHeight )
              {
                pointl.y = ((rcl.bottom - rcl.top) - bm.bmHeight)/2;
              }
              else
              {
                pointl.y = 0;
              } /* endif */

              usBitmapStyle = (selected) ? DBM_INVERT : DBM_NORMAL;

              usBitmapStyle |= (fEnabled) ? 0 : DBM_HALFTONE;
              WinDrawBitmap( hdc, hbm, (PRECTL)NULL, &pointl,
                             CLR_RED, CLR_BACKGROUND,
                             usBitmapStyle );

              rcl.left = bm.bmWidth +2;
              rcl.top = 0;
            }
            else
            {
               MessageBeep( (USHORT)-1 );
              } /* endif */
            usTextStyle = DT_LEFT | DT_VCENTER;
            usTextStyle |= (fEnabled) ? 0 : DT_HALFTONE;

            chWork[0] = driveid;
            chWork[1] = EOS;
            {
              if ( selected )
              {
                SetBkColor( hdc, GetSysColor(COLOR_HIGHLIGHT) );
                SetTextColor( hdc, GetSysColor(COLOR_HIGHLIGHTTEXT) );
              }
              else
              {
                SetBkColor( hdc, GetSysColor(COLOR_BTNFACE) );
                SetTextColor( hdc, GetSysColor(COLOR_BTNTEXT) );
              } /* endif */

              DrawText( hdc, chWork, -1, &rcl, usTextStyle );
            }
            // draw a focus rectangle
            if (hwnd == GetFocus())
            {
              GetClientRect( hwnd, &rcl );
              DrawFocusRect( hdc, &rcl );
            } /* endif */

            EndPaint(hwnd, &ps);
            if ( hbm )
            {
              DeleteObject( hbm );
            } /* endif */
          }
        return( 0L);
      /****************************************************************/
      /* trap set and kill focus to set drawing rectangle correctly   */
      /****************************************************************/
      case WM_KILLFOCUS:
      case WM_SETFOCUS:
        InvalidateRect( hwnd, NULL, FALSE);
        break;

      default:
        return( CALLWINDOWPROC( oldButtonProc, hwnd, msg, mp1, mp2 ));
   } /* endswitch */
   return( 0L);
}

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlSetPosDriveButtons2                                   |
//+----------------------------------------------------------------------------+
//|Description:       Position the drive buttons within given limits and set   |
//|                   z-order of drive buttons                                 |
//+----------------------------------------------------------------------------+
//|Function Call:     UtlSetPosDriveButtons2( HWND  hwndParent,                |
//|                                           USHORT usStartID,                |
//|                                           USHORT usStartx,                 |
//|                                           USHORT usStarty,                 |
//|                                           USHORT usSizex,                  |
//|                                           USHORT usSizey,                  |
//|                                           USHORT usBoundingx,              |
//|                                           USHORT usBoundingy,              |
//|                                           USHORT usDistx,                  |
//|                                           USHORT usDisty,                  |
//|                                           HWND   hwndInsertBehind )        |
//+----------------------------------------------------------------------------+
//|Parameters:        1. HWND - handle of parent window                        |
//|                   2. USHORT - PID of starting drive button                 |
//|                   3. USHORT - x-coordinate of 1st drive button             |
//|                   4. USHORT - y- ....                                      |
//|                   5. USHORT - cx - length of each button                   |
//|                   6. USHORT - cy - height of each button                   |
//|                   7. USHORT - xright -  max x position of buttons          |
//|                   8. USHORT - ybot -  max y position of buttons            |
//|                   7. USHORT - xdist  -  x distance between buttons         |
//|                   8. USHORT - ydist -  y distance between buttons          |
//|                   9. HWND   - hwndInsertBehind - where to insert buttons   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     set current x and y position                             |
//|                   for all drive buttons do                                 |
//|                     get button window handle                               |
//|                     if right hand side boundary is reached then            |
//|                       set current x position to x start position           |
//|                       subtract y distance from current y position          |
//|                     endif                                                  |
//|                     if lower boundary has not been reached then            |
//|                       set window position of button to current position    |
//|                       add x distance to current x position                 |
//|                     endif                                                  |
//|                   endfor                                                   |
//+----------------------------------------------------------------------------+
USHORT UtlSetPosDriveButtons2
(
   HWND   hwndParent,                  // handle of drive button parent window
   USHORT usStartID,                   // drive button start ID
   ULONG  ulStartx,                    // x start position of first drive button
   ULONG  ulStarty,                    // y start position of first drive button
   ULONG  ulSizex,                     // size of drive button x value
   ULONG  ulSizey,                     // size of drive button y value
   ULONG  ulBoundingx,                 // bounding x right position
   ULONG  ulBoundingy,                 // bounding y bottom position
   ULONG  ulDistx,                     // distance between buttons in x direction
   ULONG  ulDisty,                     // distance between buttons in y direction
   HWND   hwndInsertBehind             // where to insert the drive buttons
)
{
    USHORT    usID;                    // current drive ID
    USHORT    usLastID;                // last drive ID
    ULONG     ulx;                     // current x position
    ULONG     uly;                     // current y position
    HWND      hwndButton;              // handle of current drive button

    // enlarge button width if it is too small
    {
      HBITMAP hbm = NULL;
      BITMAP  bm;
      SIZE    size;
      HDC     hdc;
	  HMODULE hResMod;

	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      // load bitmap and get bitmap data
      hbm = LoadBitmap( hResMod, MAKEINTRESOURCE( EQFFD_BITMAP ));
      GetObject( hbm, sizeof( BITMAP ), (LPSTR) &bm );

      // get width of W: drive string (should be largest text string)
      hdc = GetDC( hwndParent );
      GetTextExtentPoint32( hdc, "W", 1, &size );
      ReleaseDC( hwndParent, hdc );

      // enlarge width if button would be too small
      size.cx += bm.bmWidth + 2;
      if ( size.cx > (LONG)ulSizex )
      {
        ulSizex = size.cx;
      } /* endif */
      // cleanup
      if ( hbm ) DeleteObject( hbm );
    }

    ulx = ulStartx;
    uly = ulStarty;
    usLastID = usStartID +'Z' - 'A';   // set last ID
    for( usID = usStartID; usID <= usLastID; usID++)
    {
      if( (hwndButton = WinWindowFromID( hwndParent, usID )) != NULLHANDLE )
      {
         if ( (ulx + ulSizex + ulDistx) >= ulBoundingx )
         {
            ulx = ulStartx;
            uly -= ulSizey + ulDisty;
         } /* endif */
         if ( uly >= ulBoundingy )
         {
            WinSetWindowPos( hwndButton,
                             hwndInsertBehind,
                             ulx, uly,
                             ulSizex, ulSizey,
                             EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_ZORDER );
            ulx += ulSizex + ulDistx;
            hwndInsertBehind = hwndButton;
            ShowWindow( hwndButton, SW_SHOWNORMAL );
         } /* endif */
      } /* endif */
    } /* endfor */
    return( 0 );
}


//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlCreateDriveButtons  Create drive icon buttons         |
//+----------------------------------------------------------------------------+
//|Description:       Create buttons with drive bitmap and drive letter        |
//|                   according to the list of drive letters specified         |
//+----------------------------------------------------------------------------+
//|Function Call:     UtlCreateDriveButtons( HWND hwnd, PSZ plist, USHORT sid, |
//|                                          ULONG style1, ULONG style)        |
//+----------------------------------------------------------------------------+
//|Parameters:        1. HWND - handle of parent window                        |
//|                   2. PSZ - list of drive letters                           |
//|                   3. USHORT - PID of starting drive button                 |
//|                   4. ULONG - button style used for 1st button              |
//|                   5. ULONG - button style used for all buttons (incl 1st)  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       - number of button created                               |
//+----------------------------------------------------------------------------+
//|Note on usage:     The proc creates buttons of type BS_USERBUTTON and       |
//|                   subclasses the button window proc with UtlDriveButtonWP  |
//+----------------------------------------------------------------------------+
//|Samples:           s. EQFFLL00.C - MakeDriveButtons                         |
//+----------------------------------------------------------------------------+
//|Function flow:     for all drives in drive list do                          |
//|                     create button window                                   |
//|                     subclass button window                                 |
//|                     send WM_DRIVEBUTTON_INIT message to button window      |
//|                   endfor                                                   |
//+----------------------------------------------------------------------------+
USHORT UtlCreateDriveButtons( HWND hwnd, PSZ plist, USHORT sid,
                              ULONG style1, ULONG style)
{
    HWND    hwndButton,
            hwndPrevButton = HWND_TOP;
    int     cnt = 0;
    PFNWP   oldproc;

    for( ; *plist; plist++)
    {
      hwndButton = CreateWindow( "BUTTON", (PSZ) NULL,
                      style1 | style | BS_NOBORDER | WS_CHILD | BS_OWNERDRAW,
                      0, 0, 0, 0,
                      hwnd,
                      (HMENU)(sid + *plist - 'A'),
                      (HINSTANCE)GetWindowLong( hwnd, GWL_HINSTANCE ),
                       NULL);
      if( hwndButton )
      {
        oldproc = SUBCLASSWND( hwndButton, ((PFNWP)UTLDRIVEBUTTONWP) );
        WinSendMsg( hwndButton, WM_DRIVEBUTTON_INIT, MP1FROMSHORT( ((BYTE)(*plist)) ),
                    oldproc );
        cnt++;
        hwndPrevButton = hwndButton;
        style1 = 0;
      } /* endif */
    } /* endfor */
    return( (USHORT)cnt);
}


//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlDriveButtons  Create and position drive icon buttons  |
//+----------------------------------------------------------------------------+
//|Description:       Create buttons with drive bitmap and drive letter        |
//|                   according to the list of drive letters specified         |
//|                   and position them                                        |
//+----------------------------------------------------------------------------+
//|Function Call:     UtlDriveButtons( HWND hwnd, PSZ plist, USHORT sid,       |
//|                                    ULONG style1, ULONG style,              |
//|                                    HWND  hwndGroupBox, HWND hwndDummy,     |
//|                                    HWND  hwndDelim );                      |
//+----------------------------------------------------------------------------+
//|Parameters:        1. HWND - handle of parent window                        |
//|                   2. PSZ - list of drive letters                           |
//|                   3. USHORT - PID of starting drive button                 |
//|                   4. ULONG - button style used for 1st button              |
//|                   5. ULONG - button style used for all buttons (incl 1st)  |
//|                   6. HWND  - handle of the groupbox                        |
//|                   7. HWND  - handle of the dummy button                    |
//|                   8. HWND  - NULL or handle of control limiting the        |
//|                              vertical space of the groupbox; if NULL       |
//|                              the complete groupbox is used for drive icons |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       - number of button created                               |
//+----------------------------------------------------------------------------+
//|Note on usage:     The proc creates buttons of type BS_USERBUTTON and       |
//|                   subclasses the button window proc with UtlDriveButtonWP  |
//|                   and positions them                                       |
//+----------------------------------------------------------------------------+
//|Samples:           s. EQFBDLG.C                                             |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlCreateDriveButtons                               |
//|                   position them within the given rectangle (i.e. groupbox) |
//|                   return number of created buttons                         |
//+----------------------------------------------------------------------------+
USHORT UtlDriveButtons
(
  HWND hwnd,                           // parent handle
  PSZ plist,                           // list of drives
  USHORT sid,                          // starting id
  ULONG style1,                        // style for first one
  ULONG style,                         // style for all others
  HWND  hwndGroupBox,                  // box where to position
  HWND  hwndDummy,                     // handle of dummy to get size from
  HWND  hwndDelim                      // NULL or handle of control limiting
                                       // the vertical space of the groupbox;
                                       // if NULL the complete groupbox is used
                                       // for drive icons
)
{
  USHORT  usCnt = 0;                   // number of drives created
  SWP     swpDrive;                    // size/position for drive icons
  SWP     swpBox;                      // size/position of drive icon groupbox
  SWP     swpDelim;                    // size/position of delimiting control

  /********************************************************************/
  /* create drive buttons ....                                        */
  /********************************************************************/
  usCnt = UtlCreateDriveButtons( hwnd, plist, sid, style1, style );

  /********************************************************************/
  /* position them depending on dummy one                             */
  /********************************************************************/
  WinQueryWindowPos( hwndDummy, &swpDrive );

  /********************************************************************/
  /* hide dummy button - the default style under OS/2                 */
  /********************************************************************/
  ShowWindow( hwndDummy, SW_HIDE );

  WinQueryWindowPos( hwndGroupBox, &swpBox );
  if ( hwndDelim != NULLHANDLE )
  {
    WinQueryWindowPos( hwndDelim, &swpDelim );
    swpBox.cy = swpBox.y + swpBox.cy - (swpDelim.y + swpDelim.cy);
    swpBox.y  = swpDelim.y + swpDelim.cy;
  } /* endif */

  UtlSetPosDriveButtons2( hwnd, sid,
                          swpDrive.x,   swpDrive.y,
                          swpDrive.cx,  swpDrive.cy,
                          (swpBox.x + swpBox.cx),
                          swpBox.y,
                          6,       // leave 6 pels in x direction and
                          5,       // 5 pels in y direction between buttons
                          hwndDummy);

  return( usCnt);
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlLoadFile            Load a file into memory           |
//+----------------------------------------------------------------------------+
//|Description:       Load a file into memory                                  |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlLoadFile(PSZ    pszFilename,                     |
//|                                    PVOID  *ppLoadedFile,                   |
//|                                    USHORT *pulBytesRead,                   |
//|                                    BOOL   fContinue,                       |
//|                                    BOOL   fMsg );                          |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFilename contains the full path to the file to      |
//|                     be loaded.                                             |
//|                   - ppLoadedFile is the pointer to a pointer which         |
//|                     addresses the memory area the file will be loaded into.|
//|                     If this value is not NULL the utility assumes          |
//|                     the storage has been allocated already and it will     |
//|                     load the file at that point. If *ppLoadedFile is       |
//|                     NULL at entry storage will be allocated and            |
//|                     *ppLoadedFile will be set accordingly.                 |
//|                   - pulBytesRead will point to a USHORT containing         |
//|                     the size of the file which has been loaded.            |
//|                   - fContinue if set will prompt the user with a message   |
//|                     if the load failed. The user may then continue or      |
//|                     cancel. If fContinue is set the flag fMsg is set       |
//|                     automatically by the program.                          |
//|                   - fMsg if set will show error messages.                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   if OK. If the load failed but fContinue           |
//|                          is set and the user comitted to continue then TRUE|
//|                          will be returned.                                 |
//|                   FALSE  in case of errors                                 |
//+----------------------------------------------------------------------------+
//|Function flow:     open the file                                            |
//|                   if ok then                                               |
//|                     get size of file                                       |
//|                     if ok and no area has been allocated for the file then |
//|                       allocate buffer for file                             |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   if ok then                                               |
//|                     read the input file into the buffer                    |
//|                   endif                                                    |
//|                   if file has been opened then                             |
//|                     close the file                                         |
//|                   endif                                                    |
//|                   if not ok and memory has been allocated then             |
//|                     free allocated memory                                  |
//|                   endif                                                    |
//|                   if not ok and message handling has been requested then   |
//|                     issue error message                                    |
//|                   endif                                                    |
//|                   return ok flag                                           |
//+----------------------------------------------------------------------------+
BOOL UtlLoadFile(PSZ    pszFilename,         // name of file to be loaded
                   PVOID  *ppLoadedFile,       // return pointer to loaded file
                   USHORT *pusBytesRead,       // length of loaded file
                   BOOL   fContinue,           // continue in case of error. If this flag
                                               // is set fMsg must be set too.
                   BOOL   fMsg )                  // Display message in case of error
{
	ULONG  ulBytesRead = *pusBytesRead;

    BOOL fOK = UtlLoadFileHwnd( pszFilename, ppLoadedFile, &ulBytesRead, fContinue,
                             fMsg, NULLHANDLE ) ;
    *pusBytesRead = (USHORT)ulBytesRead;

    return(fOK);
}

BOOL UtlLoadFileL(PSZ    pszFilename,         // name of file to be loaded
                  PVOID  *ppLoadedFile,       // return pointer to loaded file
                  ULONG *pulBytesRead,       // length of loaded file
                  BOOL   fContinue,           // continue in case of error. If this flag
                                              // is set fMsg must be set too.
                  BOOL   fMsg )                  // Display message in case of error
{
   return( UtlLoadFileHwnd( pszFilename, ppLoadedFile, pulBytesRead, fContinue,
                            fMsg, NULLHANDLE ) );
}

BOOL UtlLoadFileHwnd
(
   PSZ    pszFilename,                 // name of file to be loaded
   PVOID  *ppLoadedFile,               // return pointer to loaded file
   ULONG  *pulBytesRead,               // length of loaded file
   BOOL   fContinue,                   // continue in case of error. If this flag
                                       // is set fMsg must be set too.
   BOOL   fMsg,                        // Display message in case of error
   HWND   hwnd                         // parent handle for error message box
)
{
    BOOL             fOK = TRUE;           // Procssing status flag
    USHORT           usDosRc = TRUE;       // Return code from Dos operations
    HFILE            hInputfile = NULLHANDLE; // File handle for input file
    USHORT           usAction;             // input file action
    USHORT           usMsgButton;          // buttons to be set on msgbox
    USHORT           usMessage;            // Message number
    BOOL             fStorage = FALSE;     // If storage was allocated then TRUE
    ULONG            ulSize = 0;

    *pulBytesRead = 0;                     // init length of file
    if (fContinue)
    {
      fMsg = TRUE;
    } /* endif */

    if ( fMsg == NOMSG )
    {
      fMsg = FALSE;
    } /* endif */

    usDosRc = UtlOpen(pszFilename, &hInputfile , &usAction,
                      0L,                            // Create file size
                      FILE_NORMAL,                   // Normal attribute
                      OPEN_ACTION_OPEN_IF_EXISTS,    // Open if exists
                      OPEN_SHARE_DENYWRITE,          // Deny Write access
                      0L,                            // Reserved
                      FALSE );                       // no error handling

    fOK = ( usDosRc == 0 );

    // If fOK. Get the size of the input data and allocate storage to load the file.
    // If an error occurred set usRc to FALSE and set the appropriate message code.
    if ( fOK )
    {
      // Get file size
      usDosRc = UtlGetFileSize( hInputfile, &ulSize, FALSE );

      // If previous call OK allocate storage if required
      if ( !usDosRc && (ulSize > 0L) )
      {
        if ( *ppLoadedFile == NULL )
        {
          usMessage = ( fMsg ) ? ERROR_STORAGE : NOMSG;
          fOK = UtlAllocHwnd( (PVOID *)ppLoadedFile,  // Allocate storage for input data
                          0L,
                          max( (LONG) MIN_ALLOC, ulSize ),
                          usMessage, hwnd );
          fStorage = TRUE;
        } /* endif */
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */

    // If fOK. Load the input data in storage and close it if load was OK
    if (fOK)
    {
      usDosRc = UtlReadL( hInputfile , *ppLoadedFile,
                         ulSize, pulBytesRead, FALSE );

      fOK = (usDosRc == 0 );

      // Check if the entire file was read
      if ( ulSize != *pulBytesRead )
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */

    if ( hInputfile )
    {
      UtlCloseHwnd( hInputfile, fMsg, hwnd );          // Close Input file if needed
    } /* endif */

    // Free storage allocation if allocated and something went wrong
    if ( !fOK && fStorage )
    {
      UtlAlloc( (PVOID *)ppLoadedFile, 0L, 0L, NOMSG );
    } /* endif */

    if ( !fOK && fMsg )                       // display error message if not ok
    {                                         // set msg btn depending on Contin.
       usMsgButton = (USHORT)(( fContinue ) ? MB_OKCANCEL : MB_CANCEL);
       // Message text:   Table or file %1 could not be accessed.
       usDosRc = UtlErrorHwnd( ERROR_TA_ACC_FILE,
                           usMsgButton,
                           1,
                           &pszFilename,
                           EQF_ERROR, hwnd);

       fOK = fContinue && ( usDosRc == MBID_OK );
    } /* endif */
    return ( fOK );
 } /* end of function UtlLoadFile */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlCheckSpaceForFile   Check space on a disk for a file  |
//|Function Name:     UtlCheckSpaceForFileEx Check space on a disk for a file  |
//+----------------------------------------------------------------------------+
//|Description:       Checks whether an amount of bytes is available on        |
//|                   a disk drive or not. The amount of bytes to be checked   |
//|                   may be specified implicitly via a file or explicitly     |
//|                   by specifying an amount of bytes.                        |
//+----------------------------------------------------------------------------+
//|Function Call:     USHORT  UtlCheckSpaceForFile(                            |
//|                                  PSZ     szFullFilePath,                   |
//|                                  USHORT  usFactor,                         |
//|                                  LONG    lSize,                            |
//|                                  PSZ  *  pszDrive,                         |
//|                                  LONG *  plBytesShort,                     |
//|                                  BOOL    fMsg );                           |
//| for UtlCheckSpaceForFileEx       PUSHORT pusDosRC );                       |
//+----------------------------------------------------------------------------+
//|Parameters:        - szFullFilePath points to the file the                  |
//|                     size should be taken from. If szFullFilePath points    |
//|                     to a NULL string then lSize must be specified.         |
//|                   - usFactor manipulates the file size in %. If the size   |
//|                     of the file is 1200 bytes and usFactor                 |
//|                     is 110 then the disk is checked for 1320 bytes.        |
//|                     Normally this value should be set to 100.              |
//|                   - lSize specifies the amount of bytes the disk should    |
//|                     be checked for. lSize will apply only if               |
//|                     szFullFilePath points to a NULL string. usFactor will  |
//|                     not be applied if lSize is specified.                  |
//|                   - pszDrive points to an address of a string which        |
//|                     contains the drive to be checked. ( without colon )    |
//|                   - plBytesShort points to a LONG value containing         |
//|                     the amount of bytes the                                |
//|                     disk is short on bytes to fullfill the requirement.    |
//|                     This value will be set to 0L if there is enough        |
//|                     disk space available.                                  |
//|                   - fMsg if set will show error messages. No message will  |
//|                     be displayed if the disk is short on storage.          |
//|                   - pusDosRC points to a variable receiving any return     |
//|                     code returned by Dosxxx functions                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   if no errors occured even if the disk has not     |
//|                          enough free space to store the file               |
//|                   FALSE  an error occured                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     get the size of the file being checked for or use        |
//|                    supplied size                                           |
//|                   if ok get free space on target drive                     |
//|                   calculater missing bytes on disk                         |
//+----------------------------------------------------------------------------+
 USHORT  UtlCheckSpaceForFile(
                PSZ     szFullFilePath,     // Full path of file to be checked
                USHORT  usFactor,           // Factor in %
                LONG    lSize,              // Explicite size
                PSZ  *  pszDrive,           // Pointer to drive to be checked
                LONG *  plBytesShort,       // Number of bytes short. Any number > 0
                                            // means short on space. usRc will only
                                            // be FALSE if a processing error occurs
                                            // but not if lBytesShort > 0.
                BOOL    fMsg )              // Display error flag
{
   return( UtlCheckSpaceForFileEx( szFullFilePath, usFactor, lSize, pszDrive,
                                   plBytesShort, fMsg, NULL ) );
}

 USHORT  UtlCheckSpaceForFileEx(
                PSZ     szFullFilePath,     // Full path of file to be checked
                USHORT  usFactor,           // Factor in %
                LONG    lSize,              // Explicite size
                PSZ  *  pszDrive,           // Pointer to drive to be checked
                LONG *  plBytesShort,       // Number of bytes short. Any number > 0
                                            // means short on space. usRc will only
                                            // be FALSE if a processing error occurs
                                            // but not if lBytesShort > 0.
                BOOL    fMsg,               // Display error flag
                PUSHORT pusDosRC )          // ptr to buffer for DOS return code
 {
    BOOL             fOK = TRUE;            // Procssing status flag
    USHORT           usDosRc = NO_ERROR;    // Return code from Dos operations
    USHORT           usCount = 1;           // For DosFindFirst
    HDIR             hSearch = HDIR_CREATE; // Directory handle for DosFindxxx calls
    FILEFINDBUF      stFile;                // Output buffer of DosFindxxx
    ULONG64          ulFreeSpace;           // Free space on disk

    // Get the size to be checked.
    // If szFullFilePath is not a NULL string then get the file size
    if ( szFullFilePath[0] )
    {
      // A filename is specified the size is set to 0
      lSize = 0L;

      // Get the size of the file
      usDosRc = UtlFindFirst( szFullFilePath,
                              &hSearch, 0, &stFile,
                              sizeof( FILEFINDBUF ), &usCount,
                              0L, fMsg );

      // If no error occurred set lSize
      if ( !usDosRc )
      {
        UtlFindClose( hSearch, FALSE );
        lSize = stFile.nFileSizeLow * (LONG)usFactor / 100L;

        // In case the file is 0 bytes long set lSize to a minimum of 1
        if ( !lSize )
        {
          lSize = 1;
        } /* endif */
      } /* endif */
    } /* endif */

    if ( (usDosRc == NO_ERROR) && lSize )
    {
      // Get corrected size (using usFactor)
      lSize = (lSize * (LONG)usFactor / 100L );

      // Get the number of free bytes from the disk
      ulFreeSpace = UtlQueryFreeSpace( *pszDrive[0], fMsg );
      if ( ulFreeSpace == (ULONG64)-1 )
      {
        usDosRc = usLastDosRc;
      } /* endif */

      // If the Dos call was OK
      if ( !usDosRc )
      {
        // Calculate the missing bytes on the disk
        if ( ulFreeSpace < (ULONG)lSize )
        {
          // The disk is short on space
          *plBytesShort = (LONG)(lSize - (LONG)ulFreeSpace);
        }
        else
        {
          // No shortage on space
          *plBytesShort = 0L;
        } /* endif */
      }
      else
      {
        // A Dos error occurred in call UtlQFSInfo
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      // An error occurred. Either the file specified does not exist or
      // a Dos error occurred or neither file nor lSize is specified.
      fOK = FALSE;
    } /* endif */

    if ( pusDosRC != NULL )
    {
      *pusDosRC = usDosRc;
    } /* endif */
    return ((USHORT) fOK );
 } /* end of function UtlCheckSpaceForFile(Ex) */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlQueryFreeSpace      Query free space of a disk        |
//+----------------------------------------------------------------------------+
//|Description:       Returns the number of available bytes on the specified   |
//|                   drive.                                                   |
//+----------------------------------------------------------------------------+
//|Function Call:     LONG   UtlQueryFreeSpace( CHAR  chDrive, BOOL fMsg )     |
//+----------------------------------------------------------------------------+
//|Parameters:        - chDrive is the letter of the drive being queried.      |
//|                   - fMsg if set will show error messages.                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   LONG                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       -1L    in case of errors                                 |
//|                   other  number of free bytes on disk                      |
//+----------------------------------------------------------------------------+
//|Function flow:     get file system information of specified drive           |
//|                   calculate free space                                     |
//|                   return calcualted size or -1L in case of errors          |
//+----------------------------------------------------------------------------+
ULONG64 UtlQueryFreeSpace( CHAR chDrive, BOOL fMsg )
{
   FSALLOCATE  stInfoBuf;              // Device info buffer
   USHORT      usDosRc;                // Return code from Dos functions
   ULONG64     lFreeSpace;             // Free space on disk

   usDosRc = UtlQFSInfo(
                  (USHORT)(chDrive - 'A' + 1),   // Drive number
                  1,                             // Information level
                  (PBYTE)&stInfoBuf,             // Address of info buffer
                  sizeof(stInfoBuf),             // Size of buffer
                  fMsg );                        // Issue error msg if required
   if ( !usDosRc )
   {
      // Calculate available disk space
      /****************************************************************/
      /* Note: to avoid overflow of our LONG value (which allows up   */
      /* to 2GB) we compute the size in kB rather than bytes. If the  */
      /* free space is larger than 2GB (=0x7FFFFFFF) we set it to 2GB.*/
      /****************************************************************/
      ULONG64 lFreeKB;                    // free space in kilo bytes
      LONG lKBDivisor = 1024L;         // divisor for byte to kB division
      LONG lBytesPerSector, lSectorsPerUnit, lUnitsAvail;

      // store the returend values in our local variables
      lBytesPerSector = stInfoBuf.cbSector;
      lSectorsPerUnit = stInfoBuf.cSectorUnit;
      lUnitsAvail     = stInfoBuf.cUnitAvail;

      // preprocess bytes per sector
      if ( lBytesPerSector > lKBDivisor )
      {
         lBytesPerSector = lBytesPerSector / lKBDivisor;
         lKBDivisor = 1;
      }
      else
      {
         lKBDivisor = lKBDivisor  / lBytesPerSector;
         lBytesPerSector = 1L;
      } /* endif */

      // preprocess sectors per unit
      if ( lSectorsPerUnit > lKBDivisor )
      {
         lSectorsPerUnit = lSectorsPerUnit / lKBDivisor;
         lKBDivisor = 1L;
      }
      else
      {
         lKBDivisor = lKBDivisor  / lSectorsPerUnit;
         lSectorsPerUnit = 1L;
      } /* endif */

      // compute free space in kB
      lFreeKB = lBytesPerSector * lSectorsPerUnit * lUnitsAvail / lKBDivisor;

      lFreeSpace = lFreeKB * 1024L;
   }
   else
   {
      lFreeSpace = (ULONG64)-1L;
      usLastDosRc = usDosRc;
   } /* endif */
   return( lFreeSpace );
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlMkMultDir           Make dir with all intermediate dir|
//+----------------------------------------------------------------------------+
//|Description:       Creates all directories specified in the input path      |
//|                   if required. The function uses UtlMkDir for all          |
//|                   directories contained in the specified path.             |
//+----------------------------------------------------------------------------+
//|Function Call:     USHORT UtlMkMultDir( PSZ pszPath, BOOL fMsg )            |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszPath is the fully qualified path without a file     |
//|                     name at the end (e.g. C:\EQF\IMPORT\PROPERTY).         |
//|                   - fMsg if set will show error messages.                  |
//+----------------------------------------------------------------------------+
//|Samples:           usRC =  UtlMkMultDir( "C:\EQF\FOLDER\SOURCE", TRUE )     |
//|                   will create the directories C:\EQF, C:\EQF\FOLDER and    |
//|                   C:\EQF\FOLDER\SOURCE.                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       Return code of UtlMkDir function. (It is no error if     |
//|                   one or more of the directories do exist).                |
//+----------------------------------------------------------------------------+
//|Function flow:     get position of backslash in path                        |
//|                   while backslash position found                           |
//|                     if next character is EOS then                          |
//|                       leave loop                                           |
//|                     else                                                   |
//|                       get position of next backslash in path               |
//|                       truncate path at found location                      |
//|                       make directory (don't care if directory exists)      |
//|                       append rest of path                                  |
//|                     endif                                                  |
//|                   endwhile                                                 |
//|                   return retcode of called functions                       |
//+----------------------------------------------------------------------------+
USHORT UtlMkMultDir( PSZ pszPath, BOOL fMsg )
{
  return ( UtlMkMultDirHwnd( pszPath, fMsg, NULLHANDLE ) );
}

USHORT UtlMkMultDirHwnd( PSZ pszPath, BOOL fMsg, HWND hwnd )
{
   PSZ   pszPathEnd;                   // ptr to current end of path
   USHORT usRC = 0;                    // function return value

   pszPathEnd = strchr( pszPath, BACKSLASH );
   while ( pszPathEnd && !usRC )
   {
      if ( *(pszPathEnd+1) == EOS )    // at terminating backslash ...
      {
         pszPathEnd = NULL;            // ... force end of loop
      }
      else                             // else create directory
      {
         pszPathEnd = strchr( pszPathEnd+1, BACKSLASH );
         if ( pszPathEnd )
         {
            *pszPathEnd = EOS;
         } /* endif */

         usRC = UtlMkDirHwnd( pszPath, 0L, fMsg, hwnd );

         if ( pszPathEnd )
         {
            *pszPathEnd = BACKSLASH;
         } /* endif */
      } /* endif */
   } /* endwhile */
   return( usRC );
} /* UtlMkMultDir */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlFileExist           Check if a file exists            |
//+----------------------------------------------------------------------------+
//|Description:       Checks if a file exists.                                 |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlFileExist( PSZ pszFileName )                     |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFileName is the fully qualified file name           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   file exists                                       |
//|                   FALSE  file does not exist                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindFirst to check if file exists                |
//|                   close file search handle                                 |
//|                   return result                                            |
//+----------------------------------------------------------------------------+
BOOL UtlFileExist( PSZ pszFile )
{
    BOOL fFound = FALSE;
    FILEFINDBUF ResultBuf;               // DOS file find struct
    HDIR    hDirHandle = HDIR_CREATE;    // DosFind routine handle

    hDirHandle = FindFirstFile( pszFile, &ResultBuf );
    if ( hDirHandle == INVALID_HANDLE_VALUE )
    {
      fFound = FALSE;
    }
    else
    {
      fFound = TRUE;
      FindClose( hDirHandle );
    } /* endif */

    return( fFound );
} /* endof UtlFileExist */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlFileExistLong       Check if a file exists            |
//+----------------------------------------------------------------------------+
//|Description:       Checks if a file exists (long file name enabled).        |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlFileExistLong( PSZ pszFileName );                |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFileName is the fully qualified file name           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   file exists                                       |
//|                   FALSE  file does not exist                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindFirstLong to check if file exists            |
//|                   close file search handle                                 |
//|                   return result                                            |
//+----------------------------------------------------------------------------+
BOOL UtlFileExistLong( PSZ pszFile )
{
  static LONGFILEFIND ResultBuf;       // Buffer for long file names
  HDIR    hDirHandle = HDIR_CREATE;    // DosFind routine handle
  USHORT  usDosRC;                     // return code of Dos... alias Utl...

  usDosRC = UtlFindFirstLongHwnd( pszFile, &hDirHandle, FILE_NORMAL,
                                  &ResultBuf, FALSE, NULLHANDLE );
  UtlFindCloseLongHwnd( hDirHandle, FALSE, NULLHANDLE );

  return( usDosRC == NO_ERROR );

} /* endof UtlFileExistLong */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlDirExist           Check if a directory exists        |
//+----------------------------------------------------------------------------+
//|Description:       Checks if a file exists.                                 |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlDirExist( PSZ pszDirName )                       |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszDirName is the fully qualified direcorty name       |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   directory exists                                  |
//|                   FALSE  directory does not exist                          |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindFirst to check if directory exists           |
//|                   close file search handle                                 |
//|                   return result                                            |
//+----------------------------------------------------------------------------+
BOOL UtlDirExist( PSZ pszDir )
{
   DWORD dwAttrib;

   if (strlen(pszDir) <= 3)
   {
     // pure drive e.g. "A:\", "E:"
     BYTE    driveletter[30];

     UtlGetDriveList(driveletter);
     if (strchr((PSZ)driveletter, toupper(pszDir[0])) && strlen(pszDir) > 0)
     {
       return TRUE;
     }
   }

   dwAttrib = GetFileAttributes(pszDir);

   return( dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) );
} /* endof UtlDirExist */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlMoveIfNewerOrNotExist Move a file if it is newer      |
//+----------------------------------------------------------------------------+
//|Description:       Move a file from source to target if the source file is  |
//|                   newer than the target file or the target file does not   |
//|                   exist.                                                   |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlMoveIfNewerOrNotExist( PSZ pszSource,            |
//|                                                  PSZ pszTarget )           |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszSource is the fully qualified source file name      |
//|                   - pszTarget is the fully qualified target file name      |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   if pszSource has been moved                       |
//|                   FALSE  if no move has been performed                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get date and time of source file                         |
//|                   get date and time of target file                         |
//|                   if source file exists then                               |
//|                     if target file exists then                             |
//|                       compare file dates and times                         |
//|                       if source file is newer then                         |
//|                         set move flag                                      |
//|                       endif                                                |
//|                     else                                                   |
//|                       set move flag                                        |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   if move has to be done then                              |
//|                     delete target file if it exists                        |
//|                     move source file to target file using UtlSmartMove     |
//|                   endif                                                    |
//|                   return file moved flag                                   |
//+----------------------------------------------------------------------------+
BOOL UtlMoveIfNewerOrNotExist( PSZ pszSource, PSZ pszTarget )
{
  return( UtlMoveIfNewerOrNotExistHwnd( pszSource, pszTarget, NULLHANDLE ) );
}

BOOL UtlMoveIfNewerOrNotExistHwnd( PSZ pszSource, PSZ pszTarget, HWND hwnd )
{
  return( UtlMoveIfNewerOrNotExistHwnd2( pszSource, pszTarget, hwnd, TRUE, NULL ) );
}

BOOL UtlMoveIfNewerOrNotExistHwnd2( PSZ pszSource, PSZ pszTarget, HWND hwnd, BOOL fMsg, PUSHORT pusDosRC )
{
   FILEFINDBUF ResultBuf;               // DOS file find struct
   USHORT  usCount = 1;
   HDIR    hDirHandle = HDIR_CREATE;    // DosFind routine handle
   USHORT  usDosRC = 0;                // return code of Dos... alias Utl...
   BOOL    fSource;                    // TRUE if source file exists
   FDATE   fdateSource;                // compare date of source
   FTIME   ftimeSource;                // compare time of source
   BOOL    fTarget;                    // TRUE if target exists
   FDATE   fdateTarget;                // compare date of target
   FTIME   ftimeTarget;                // compare time of target
   BOOL    fFileMoved = FALSE;         // TRUE if file has been moved
   BOOL    fMove = FALSE;              // TRUE if move is required
   LONG    sCompare;                   // result of date/time compare

   //--- check source file ---
   usDosRC = UtlFindFirstHwnd( pszSource, &hDirHandle, FILE_NORMAL, &ResultBuf, sizeof( ResultBuf), &usCount, 0L, FALSE, hwnd );
   UtlFindCloseHwnd( hDirHandle, FALSE, hwnd );
   if ( usCount && !usDosRC )
   {
      fSource = TRUE;
      FileTimeToDosDateTime( &ResultBuf.ftLastWriteTime, (LPWORD)&fdateSource, (LPWORD)&ftimeSource );
   }
   else
   {
      fSource = FALSE;
   } /* endif */

   //--- check target file ---
   usCount = 1;
   hDirHandle = HDIR_CREATE;
   usDosRC = UtlFindFirstHwnd( pszTarget, &hDirHandle, FILE_NORMAL,
                        &ResultBuf, sizeof( ResultBuf),
                        &usCount, 0L, FALSE, hwnd );
   UtlFindCloseHwnd( hDirHandle, FALSE, hwnd );
   if ( usCount && !usDosRC )
   {
      fTarget = TRUE;
      FileTimeToDosDateTime( &ResultBuf.ftLastWriteTime, (LPWORD)&fdateTarget, (LPWORD)&ftimeTarget );
   }
   else
   {
      fTarget = FALSE;
   } /* endif */

   if ( fSource )                      // move only if source exists
   {
      if ( fTarget )                   // if there is a target, check dates
      {
         sCompare = UtlCompareDate( &fdateTarget, &fdateSource );
         if ( sCompare < 0 )
         {
            fMove = TRUE;
         }
         else if ( sCompare == 0 )
         {
            sCompare = UtlCompareTime(&ftimeTarget, &ftimeSource );
            if ( sCompare < 0 )
            {
               fMove = TRUE;
            } /* endif */
         } /* endif */
      }
      else
      {
         fMove = TRUE;                 // file should be moved
      } /* endif */
   } /* endif */

   if ( fMove )
   {
      if ( fTarget )                   // delete existing target file
      {
         UtlDelete( pszTarget, 0L, fMsg );
      } /* endif */

      usDosRC = UtlSmartMoveHwnd( pszSource, pszTarget, fMsg, hwnd );

      if ( !usDosRC )
      {
         fFileMoved = TRUE;
      } /* endif */
   } /* endif */

   if ( pusDosRC )  *pusDosRC = usDosRC;

   return( fFileMoved );
} /* endof UtlMoveIfNewerOrNotExist */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlSmartMove           Move or copy/delete a file        |
//+----------------------------------------------------------------------------+
//|Description:       Works like UtlMove. If the files are not on the same     |
//|                   drive, UtlCopy and UtlDelete are used to move the file.  |
//+----------------------------------------------------------------------------+
//|Function Call:     USHORT UtlSmartMove( PSZ pszSource,                      |
//|                                        PSZ pszTarget,                      |
//|                                        BOOL fMsg )                         |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszSource is the fully qualified source file name      |
//|                   - pszTarget is the fully qualified target file name      |
//|                   - if fMsg is TRUE error handling is done in the utility; |
//|                     if fMsg is FALSE, errors have to be handled by the     |
//|                     calling program.                                       |
//+----------------------------------------------------------------------------+
//|Return Values:     USHORT return code of UtlMove, UtlCopy or UtlDelete      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of UtlMove, UtlCopy or UtlDelete             |
//+----------------------------------------------------------------------------+
//|Function flow:     if drive of source file is not drive of target file then |
//|                     call UtlCopy to copy the source file to the target file|
//|                     call UtlDelete to delete the source file               |
//|                   else                                                     |
//|                     call UtlMove to move the source file to the target file|
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
USHORT UtlSmartMove( PSZ pszSource, PSZ pszTarget, BOOL fMsg )
{ return(  UtlSmartMoveHwnd( pszSource, pszTarget, fMsg, NULLHANDLE ) ); }

USHORT UtlSmartMoveHwnd( PSZ pszSource, PSZ pszTarget, BOOL fMsg, HWND hwnd )
{
   USHORT  usDosRC;                              // return code of Dos... alias Utl...

   if ( *pszSource != *pszTarget )  // on different drives ???
   {
      /****************************************************************/
      /* Source and target drives differ, use copy and delete to move */
      /* the file                                                     */
      /****************************************************************/
      usDosRC = UtlCopyHwnd( pszSource, pszTarget, 1, 0L, fMsg, hwnd );
      if ( !usDosRC )
      {
         UtlDeleteHwnd( pszSource, 0L, fMsg, hwnd );
      } /* endif */
   }
   else
   {
      /****************************************************************/
      /* Use move as source and target file reside on the same disk   */
      /****************************************************************/
      usDosRC = UtlMoveHwnd( pszSource, pszTarget, 0L, fMsg, hwnd );
   } /* endif */

   return( usDosRC );
} /* endof UtlSmartMove */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlRemoveDir           Remove a directory with all files |
//+----------------------------------------------------------------------------+
//|Description:       Removes a directory and all files and subdirectories     |
//|                   contained in the directory.                              |
//+----------------------------------------------------------------------------+
//|Function Call:     USHORT UtlRemoveDir( PSZ pszDir, BOOL fMsg )             |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszDir is the fully qualified directory name           |
//|                   - if fMsg is TRUE error handling is done in the utility; |
//|                     if fMsg is FALSE, errors have to be handled by the     |
//|                     calling program.                                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of UtlRmDir or UtlDelete                     |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate buffer for path names                           |
//|                   if ok then                                               |
//|                     find first file in directory                           |
//|                     while there are files in directory                     |
//|                       if file is a directory then                          |
//|                         call UtlRemoveDir to remove the directory          |
//|                       else                                                 |
//|                         call UtlDelete to delete the file                  |
//|                       endif                                                |
//|                       find next file                                       |
//|                     close the file search handle                           |
//|                     call UtlRmDir to remove the directory                  |
//|                     free the path name buffer                              |
//|                   endif                                                    |
//|                   return error code to caller                              |
//+----------------------------------------------------------------------------+
USHORT UtlRemoveDir( PSZ pszDir, BOOL fMsg )
{
   PSZ         pszSearchPath;          // ptr to search path buffer
   LONGFILEFIND ResultBuf;              // DOS file find struct
   HDIR        hDirHandle = HDIR_CREATE;  // DosFind routine handle
   USHORT      usRC = 0;               // return code of Dos... alias Utl...

   /*******************************************************************/
   /* Allocate buffer for search path                                 */
   /*******************************************************************/
   UtlAlloc( (PVOID *)&pszSearchPath, 0L, (LONG) CCHMAXPATH, ERROR_STORAGE );

   if ( pszSearchPath )
   {
      strcpy( pszSearchPath, pszDir );
      strcat( pszSearchPath, "\\*.*" );

      /****************************************************************/
      /* loop through all files in directory                          */
      /****************************************************************/
      usRC = UtlFindFirstLong( pszSearchPath, &hDirHandle,
                               FILE_NORMAL | FILE_DIRECTORY,
                               &ResultBuf, FALSE );
      while ( !usRC )
      {
         strcpy( pszSearchPath, pszDir );
         strcat( pszSearchPath, "\\" );
         strcat( pszSearchPath, ResultBuf.achName );
         if ( ResultBuf.attrFile & FILE_DIRECTORY )
         {
            if ( ResultBuf.achName[0] != '.' )
            {
               usRC = UtlRemoveDir( pszSearchPath, fMsg );
            } /* endif */
         }
         else
         {
            usRC = UtlDelete( pszSearchPath, 0L, fMsg );
         } /* endif */
         usRC = UtlFindNextLong( hDirHandle, &ResultBuf, FALSE );
      } /* endwhile */
      if ( usRC == ERROR_NO_MORE_FILES) usRC = NO_ERROR;
      UtlFindCloseLong( hDirHandle, FALSE );
      if ( !usRC ) usRC = UtlRmDir( pszDir, 0L, fMsg );
      UtlAlloc( (PVOID *)&pszSearchPath, 0L, 0L, NOMSG );
   }
   else
   {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
   } /* endif */

   return( usRC );
} /* endof UtlRemoveDir */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlWriteFile           Write a file to disk              |
//+----------------------------------------------------------------------------+
//|Description:       Writes a file (counterpart to UtlLoadFile)               |
//+----------------------------------------------------------------------------+
//|Function Call:     USHORT UtlWriteFile( PSZ    pszFile,                     |
//|                                        USHORT usDataLength,                |
//|                                        PVOID  pData,                       |
//|                                        BOOL   fMsg );                      |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFile is the fully qualified file name               |
//|                   - usDataLength is the length of the data to write        |
//|                   - pData points to the data to write to the file          |
//|                   - if fMsg is TRUE error handling is done in the function |
//+----------------------------------------------------------------------------+
//|Samples:           UtlWriteFile( "TEST.DATA", 10, "test data" );            |
//|                     will create/rewrite a file named "TEST.DAT" which will |
//|                     contain the string "test data"                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of called Dosxxxx functions in               |
//|                   UtlWriteFileHwnd                                         |
//+----------------------------------------------------------------------------+
//|Function flow:     calls UtlWriteFileHwnd with NULL window handle           |
//+----------------------------------------------------------------------------+

USHORT UtlWriteFile
(
  PSZ       pszFile,            // name of file to write
  USHORT    usDataLength,       // length of data to write to the file
  PVOID     pData,              // pointer to data being written to file
  BOOL      fMsg                // TRUE = do message handling
)
{
   ULONG  ulDataLength = usDataLength;

   return( UtlWriteFileHwnd( pszFile,
                             ulDataLength,
                             pData,
                             fMsg,
                             NULLHANDLE ) );;

}

USHORT UtlWriteFileL
(
  PSZ       pszFile,            // name of file to write
  ULONG     ulDataLength,       // length of data to write to the file
  PVOID     pData,              // pointer to data being written to file
  BOOL      fMsg                // TRUE = do message handling
)
{
   return( UtlWriteFileHwnd( pszFile,
                             ulDataLength,
                             pData,
                             fMsg,
                             NULLHANDLE ) );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlWriteFileHwnd       Write a file to disk              |
//+----------------------------------------------------------------------------+
//|Description:       Writes a file (counterpart to UtlLoadFile)               |
//+----------------------------------------------------------------------------+
//|Function Call:     USHORT UtlWriteFileHwnd( PSZ    pszFile,                 |
//|                                            USHORT usDataLength,            |
//|                                            PVOID  pData,                   |
//|                                            BOOL   fMsg,                    |
//|                                            HWND   hwndParent );            |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFile is the fully qualified file name               |
//|                   - usDataLength is the length of the data to write        |
//|                   - pData points to the data to write to the file          |
//|                   - if fMsg is TRUE error handling is done in the function |
//|                   - hwndParent is the parent window                        |
//+----------------------------------------------------------------------------+
//|Samples:           UtlWriteFile( "TEST.DATA", 10, "test data" );            |
//|                     will create/rewrite a file named "TEST.DAT" which will |
//|                     contain the string "test data"                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of called Dosxxxx functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     open the file for output                                 |
//|                   if ok write given data to file                           |
//|                   if file is open close file                               |
//|                   return any error codes to caller                         |
//+----------------------------------------------------------------------------+
USHORT UtlWriteFileHwnd                                                  /*@86C*/
(
  PSZ       pszFile,                   // name of file to write
  ULONG     ulDataLength,              // length of data to write to the file
  PVOID     pData,                     // pointer to data being written to file
  BOOL      fMsg,                      // TRUE = do message handling
  HWND      hwndParent
)
{
    USHORT           usDosRC;              // Return code from Dos operations
    HFILE            hOutFile = NULLHANDLE;// File handle for input file
    USHORT           usAction;             // file action performed by DosOpen
    ULONG            ulBytesWritten;       // number of bytes written to file
//    PSZ              pszErrParm;          // ptr to error parameter

    /******************************************************************/
    /* Open the file                                                  */
    /******************************************************************/
    usDosRC = UtlOpenHwnd( pszFile, &hOutFile, &usAction,
                           0L,
                           FILE_NORMAL,
                           FILE_CREATE | FILE_TRUNCATE,
                           OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,
                           0L,
                           fMsg,
                           hwndParent );

    /******************************************************************/
    /* if ok, write given data to file                                */
    /******************************************************************/
    if ( !usDosRC )
    {
      usDosRC = UtlWriteHwnd( hOutFile,
                              pData,
                              ulDataLength,
                              &ulBytesWritten,
                              fMsg,
                              hwndParent );
//      if ( !usDosRC && ( usBytesWritten != usDataLength ) )      /*KIT1274*/
//      {                                                          /*KIT1274*/
//        usDosRC = ERROR_DISK_FULL;                               /*KIT1274*/
//      } /* endif */                                              /*KIT1274*/
    } /* endif */

    /******************************************************************/
    /* Close the file                                                 */
    /******************************************************************/
    if ( hOutFile )
    {
      UtlCloseHwnd( hOutFile,
                    fMsg,
                    hwndParent );
      /******************************************************************/
      /* Delete file if no Bytes written                                */
      /******************************************************************/
      if ( usDosRC )
      {
           UtlDeleteHwnd( pszFile,
                          0L,
                          FALSE,
                          hwndParent );
//           if ( fMsg )                                           /*KIT1274*/
//           {                                                     /*KIT1274*/
//             pszErrParm = pszFile;                               /*KIT1274*/
//             UtlErrorHwnd( ERROR_EQF_DISK_FULL, MB_CANCEL, 1,    /*KIT1274*/
//                           &pszErrParm, EQF_ERROR,               /*KIT1274*/
//                           hwndParent );                         /*KIT1274*/
//           } /* endif */                                         /*KIT1274*/
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Return error codes to calling function                         */
    /******************************************************************/
    return ( usDosRC );

} /* UtlWriteFile */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlIsEqfDrive          Check if a drive is a MAT drive   |
//+----------------------------------------------------------------------------+
//|Description:       Check if a given drive is a) in the EQF drive list       |
//|                   and b) that all required directories exist on this drive |
//+----------------------------------------------------------------------------+
//|Parameters:        1. CHAR   - letter of drive beeing checked               |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE ==> function completed successfully                 |
//|                   FALSE ==> function could not complete due to             |
//|                             serious errors                                 |
//+----------------------------------------------------------------------------+
//|Samples:           if ( UtlIsSecondaryDrive ( 'E' ) )                       |
//|                   ...                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     set isEqfDrive flag to TRUE                              |
//|                   if drive is not in EQF drive list reset isEqfDrive flag  |
//|                   if isEqfDrive then                                       |
//|                     reset isEqfDrive flag if drive letter is invalid       |
//|                   endif                                                    |
//|                   if isEqfDrive then                                       |
//|                     reset isEqfDrive flag if EQF directories are missing   |
//|                   endif                                                    |
//|                   return isEqfDrive flag                                   |
//+----------------------------------------------------------------------------+
BOOL UtlIsEqfDrive
(
   CHAR chDrive                        // drive letter being checked
)
{
   BOOL fIsEqfDrive = TRUE;            // function return value
   union
   {
      CHAR  szDrives[MAX_DRIVELIST];   // buffer for drive letters
      CHAR  szPath[MAX_EQF_PATH+1];    // buffer for path names
   } Buffers;

   //
   // get string containing all known EQF drives
   //
   UtlQueryString( QST_ORGEQFDRIVES, Buffers.szDrives,
                   sizeof(Buffers.szDrives) );

   //
   // check if given drive is contained in EQF drive list
   //
   if ( !strchr( Buffers.szDrives, chDrive ) )
   {
      fIsEqfDrive = FALSE;
   } /* endif */

   //
   // check if given drive is a valid drive for this system
   //
   if ( fIsEqfDrive )
   {
      UtlGetDriveList( (PBYTE)Buffers.szDrives );     // get list of attached drives
      if ( !strchr( Buffers.szDrives, chDrive ) )
      {
         fIsEqfDrive = FALSE;
      } /* endif */
   } /* endif */

   //
   // check if required directories exist on drive
   //
   if ( fIsEqfDrive )
   {
      UtlMakeEQFPath( Buffers.szPath, chDrive, MEM_PATH, NULL );
      if ( !UtlDirExist( Buffers.szPath ) )
      {
         fIsEqfDrive = FALSE;
      }
      else
      {
         UtlMakeEQFPath( Buffers.szPath, chDrive, DIC_PATH, NULL );
         if ( !UtlDirExist( Buffers.szPath ) )
         {
            fIsEqfDrive = FALSE;
         }
         else
         {
            UtlMakeEQFPath( Buffers.szPath, chDrive, TABLE_PATH, NULL );
            if ( !UtlDirExist( Buffers.szPath ) )
            {
               fIsEqfDrive = FALSE;
            }
            else
            {
               UtlMakeEQFPath( Buffers.szPath, chDrive, LIST_PATH, NULL );
               if ( !UtlDirExist( Buffers.szPath ) )
               {
                  fIsEqfDrive = FALSE;
               }
               else
               {
                  // everything seems to be ok with the given drive
               } /* endif */
            } /* endif */
         } /* endif */
      } /* endif */
   } /* endif */

   return( fIsEqfDrive );
} /* end of UtlIsEqfDrive */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlGetCheckedEqfDrives Get list of checked MAT drives    |
//+----------------------------------------------------------------------------+
//|Description:       Returns a list of EQF drives and checks each             |
//|                   of the drives if it is still valid                       |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ    - ptr to buffer for drive letter string        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE ==> function completed successfully                 |
//|                   FALSE ==> function could not complete due to             |
//|                             serious errors                                 |
//+----------------------------------------------------------------------------+
//|Samples:           UtlGetCheckedEqfDrives( pszDrives );                     |
//|                   would place "DE" in pszDrives if "DET" where EQF drives, |
//|                   but T: did not have the EQF directories anymore.         |
//+----------------------------------------------------------------------------+
//|Function flow:     get list of original EQF drives                          |
//|                   for all drives in list do                                |
//|                     if drive is an EQF drive (call to UtlIsEqfDrive) then  |
//|                       add drive to checked EQF drive list                  |
//|                     endif                                                  |
//|                   endfor                                                   |
//|                   replace QST_VALIDDRIVES string with checked EQF drive    |
//|                    list                                                    |
//|                   copy checked EQF drives to supplied buffer               |
//|                   return ok flag                                           |
//+----------------------------------------------------------------------------+
BOOL UtlGetCheckedEqfDrives
(
   PSZ pszDrives                       // ptr to buffer for EQF drives
)
{
   BOOL fOK = TRUE;                    // function return code
   CHAR  szOrgDrives[MAX_DRIVELIST];   // buffer for original EQF drive letters
   PSZ   pszOrgDrive = szOrgDrives;    // pointer for drive string processing
   CHAR  szValidDrives[MAX_DRIVELIST]; // buffer for valid EQF drive letters
   PSZ   pszValidDrive = szValidDrives;// pointer for drive string processing

   /*******************************************************************/
   /* Get list of MAT drives (source: system properties)              */
   /*******************************************************************/
   UtlQueryString( QST_ORGEQFDRIVES, szOrgDrives, sizeof(szOrgDrives) );

   /*******************************************************************/
   /* Check each of the drives in the drive list                      */
   /*******************************************************************/
   while ( *pszOrgDrive)
   {
      if ( UtlIsEqfDrive( *pszOrgDrive ) )
      {
         *pszValidDrive++ = *pszOrgDrive;        // add to valid drives
      } /* endif */
      pszOrgDrive++;                             // test next drive
   } /* endwhile */
   *pszValidDrive = EOS;                                   // terminate valid drives list

   /*******************************************************************/
   /* Set list of checked MAT drives                                  */
   /*******************************************************************/
   UtlReplString( QST_VALIDEQFDRIVES, szValidDrives );
   if ( pszDrives )
   {
      strcpy( pszDrives, szValidDrives );
   } /* endif */

   return( fOK );
} /* end of UtlGetCheckedEqfDrives */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlGetCheckedEqfLanDrives Get list of checked            |
//|                   MAT LAN drives                                           |
//+----------------------------------------------------------------------------+
//|Description:       Returns a list of EQF LAN drives and checks each         |
//|                   of the drives if it is still valid                       |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ    - ptr to buffer for drive letter string        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE ==> function completed successfully                 |
//|                   FALSE ==> function could not complete due to             |
//|                             serious errors                                 |
//+----------------------------------------------------------------------------+
//|Samples:           UtlGetCheckedEqfLanDrives( pszDrives );                  |
//|                   would place "DE" in pszDrives if "DET" where EQF drives, |
//|                   but T: did not have the EQF directories anymore.         |
//+----------------------------------------------------------------------------+
//|Function flow:     get list of original EQF LAN drives                      |
//|                   for all drives in list do                                |
//|                     if drive is an EQF drive (call to UtlIsEqfLANDrive)    |
//|                       add drive to checked EQF drive list                  |
//|                     endif                                                  |
//|                   endfor                                                   |
//|                   replace QST_VALIDDRIVES string with checked EQF drive    |
//|                    list                                                    |
//|                   copy checked EQF drives to supplied buffer               |
//|                   return ok flag                                           |
//+----------------------------------------------------------------------------+
BOOL UtlGetCheckedEqfLanDrives
(
   PSZ pszDrives                       // ptr to buffer for EQF drives
)
{
   BOOL fOK = TRUE;                    // function return code
   CHAR  szOrgDrives[MAX_DRIVELIST];   // buffer for original EQF drive letters
   PSZ   pszOrgDrive = szOrgDrives;    // pointer for drive string processing
   CHAR  szValidDrives[MAX_DRIVELIST]; // buffer for valid EQF drive letters
   PSZ   pszValidDrive = szValidDrives;// pointer for drive string processing

   /*******************************************************************/
   /* Check each of the drives in the drive list                      */
   /*******************************************************************/
   while ( *pszOrgDrive)
   {
      if ( UtlIsEqfDrive( *pszOrgDrive ) )
      {
         *pszValidDrive++ = *pszOrgDrive;        // add to valid drives
      } /* endif */
      pszOrgDrive++;                             // test next drive
   } /* endwhile */
   *pszValidDrive = EOS;                                   // terminate valid drives list

   /*******************************************************************/
   /* Set list of checked MAT drives                                  */
   /*******************************************************************/
   UtlReplString( QST_VALIDEQFDRIVES, szValidDrives );
   if ( pszDrives )
   {
      strcpy( pszDrives, szValidDrives );
   } /* endif */

   return( fOK );
} /* end of UtlGetCheckedEqfDrives */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlBufOpen                                               |
//+----------------------------------------------------------------------------+
//|Function call:     UtlBufOpen( PBUFCB *ppBufCB, PSZ pszFile,                |
//|                               USHORT usBufSize, BOOL fMsg )                |
//+----------------------------------------------------------------------------+
//|Description:       Opens a file for buffered output                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBUFCB   *ppBufCB   address of buffered output control   |
//|                                       block pointer                        |
//|                   USHORT   usBufSize  size of input/output buffer          |
//|                   USHORT   usReadWrite read/write flag                     |
//|                            FILE_OPEN   for read access                     |
//|                            FILE_CREATE for write access                    |
//|                            FILE_APPEND for write/append access             |
//|                   BOOL     fMsg  TRUE if error messages should be          |
//|                                  displayed, otherwise FALSE                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0       function completed successfully                  |
//|                   ERROR_NOT_ENOUGH_MEMORY  allocation of memory failed     |
//|                   other   return codes of Dosxxxx calls                    |
//+----------------------------------------------------------------------------+
//|Samples:           usRC = UtlBufOpen( &pCB, "TEST.LST", 2048, FILE_CREATE ) |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate control block / output buffer                   |
//|                   open the file                                            |
//|                   initialize control block variables                       |
//+----------------------------------------------------------------------------+
USHORT UtlBufOpen
(
  PBUFCB *ppBufCB,
  PSZ pszFile,
  ULONG ulBufSize,
  ULONG ulReadWrite,
  BOOL   fMsg
)
{
  return( UtlBufOpenHwnd( ppBufCB, pszFile, ulBufSize, ulReadWrite, fMsg,
                          NULLHANDLE ) );
}

USHORT UtlBufOpenHwnd
(
  PBUFCB *ppBufCB,
  PSZ pszFile,
  ULONG ulBufSize,
  ULONG ulReadWrite,
  BOOL   fMsg,
  HWND   hwnd
)
{
  USHORT       usOpenAction;           // action performed by UtlOpen
  USHORT       usRC = 0;               // return code of function
  USHORT       usOpenFlag;             // File open flag
  USHORT       usOpenMode;             // File open mode
  PBUFCB       pBufCB;                 // buffered input/output control block
  ULONG        ulFilePos;              // new file position

  /********************************************************************/
  /* Allocate control block / input output buffer                     */
  /********************************************************************/
  UtlAllocHwnd( (PVOID *)&pBufCB, 0L, (sizeof(BUFCB) + ulBufSize ),
                 ERROR_STORAGE, hwnd );
  if ( !pBufCB )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  /********************************************************************/
  /* Open the output file                                             */
  /********************************************************************/
  if ( !usRC )
  {
     switch (ulReadWrite)
     {
      case FILE_CREATE:
        {
        usOpenFlag = FILE_TRUNCATE | FILE_CREATE ;
        usOpenMode = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE;
        }
        break;
      case  FILE_APPEND:
        {
        usOpenFlag = FILE_OPEN;
        usOpenMode = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE;
        }
        break;
      default:
        {
        usOpenFlag = FILE_TRUNCATE | FILE_OPEN;
        usOpenMode = OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE;
        }
        break;
     } /* endswitch */

     usRC = UtlOpenHwnd( pszFile,
                         &pBufCB->hFile,
                     &usOpenAction, 0L,
                     FILE_NORMAL,
                     usOpenFlag,
                     usOpenMode,
                     0L,
                     fMsg, hwnd );
  } /* endif */
  if ( !usRC && (ulReadWrite == FILE_APPEND) )
  {
    usRC = UtlChgFilePtrHwnd( pBufCB->hFile, 0L, FILE_END, &ulFilePos, fMsg, hwnd );
  } /* endif */


  /********************************************************************/
  /* Initialize control block variables                               */
  /********************************************************************/
  if ( !usRC )
  {
     pBufCB->ulUsed = 0;
     pBufCB->ulSize = ulBufSize;
     pBufCB->ulProcessed  = 0;
     strcpy( pBufCB->szFileName, pszFile );
     if ( ulReadWrite != FILE_CREATE )
     {
       usRC = UtlGetFileSizeHwnd( pBufCB->hFile,
                                  &(pBufCB->ulRemaining),
                                  fMsg, hwnd );
       pBufCB->fWrite = ( ulReadWrite == FILE_APPEND );
     }
     else
     {
       pBufCB->fWrite = TRUE;
     } /* endif */
     *ppBufCB = pBufCB;
  } /* endif */

  return( usRC );
} /* end of function UtlBufOpen */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlBufClose            Close a buffered output file      |
//+----------------------------------------------------------------------------+
//|Function call:     UtlBufClose( PBUFCB pBufCB, BOOL fMsg )                  |
//+----------------------------------------------------------------------------+
//|Description:       Closes a file which has been opened for buffered output  |
//|                   using UtlBufOpen.                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBUFCB   pBufCB     pointer to buffered output CB        |
//|                   BOOL     fMsg       TRUE = show error messages           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      function completed successfully                   |
//|                   other  error codes from Dosxxxx calls                    |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pBufCB must have been created using UtlBufOpen           |
//+----------------------------------------------------------------------------+
//|Function flow:     write pending data to output file                        |
//|                   close output file                                        |
//|                   free memory of buffered output control block             |
//+----------------------------------------------------------------------------+
USHORT UtlBufClose
(
  PBUFCB pBufCB,                       // pointer to buffered output CB
  BOOL   fMsg                          // TRUE for message processing
)
{
  return( UtlBufCloseHwnd( pBufCB, fMsg, NULLHANDLE ) );
}

USHORT UtlBufCloseHwnd
(
  PBUFCB pBufCB,                       // pointer to buffered output CB
  BOOL   fMsg,                         // TRUE for message processing
  HWND   hwnd                          // handle for error messages
)
{
  USHORT      usRC = 0;               // return code of function
  ULONG       ulBytesWritten;         // number of bytes written to file
  PSZ         pszErrParm;             // ptr to error parameter

  /********************************************************************/
  /* Write pending data to output file                                */
  /********************************************************************/
  if ( pBufCB->fWrite && pBufCB->ulUsed )
  {
    usRC = UtlWriteHwnd( pBufCB->hFile,
                     (PVOID)pBufCB->Buffer,
                     pBufCB->ulUsed,
                     &ulBytesWritten,
                     fMsg, hwnd );
    if ( !usRC && (pBufCB->ulUsed != ulBytesWritten) )
    {
      usRC = ERROR_DISK_FULL;
      if ( fMsg )
      {
        pszErrParm = pBufCB->szFileName,
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszErrParm, DOS_ERROR, hwnd );
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Close the file in any case                                       */
  /********************************************************************/
  UtlCloseHwnd( pBufCB->hFile, fMsg, hwnd );

  /********************************************************************/
  /* Free control block                                               */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pBufCB, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function UtlBufClose */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlBufWrite            Write string to buffered file     |
//+----------------------------------------------------------------------------+
//|Function call:     UtlBufWrite( PBUFCB pBufCB, PSZ pszData, USHORT usLength,|
//|                                BOOL fMsg )                                 |
//+----------------------------------------------------------------------------+
//|Description:       Writes a string to a buffered output file .              |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBUFCB   pBufCB     pointer to buffered output CB        |
//|                   PSZ      pszData    data to be written to the file       |
//|                   USHORT   usLength   number of bytes of data              |
//|                   BOOL     fMsg       TRUE = show error messages           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      function completed successfully                   |
//|                   other  error codes from Dosxxxx calls                    |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pBufCB must have been created using UtlBufOpen           |
//+----------------------------------------------------------------------------+
//|Samples:           usRC = UtlBufWrite( pCB, "this is a test string" );      |
//+----------------------------------------------------------------------------+
//|Function flow:     add pszData to buffer up to free space in buffer         |
//|                   adjust buffer used count                                 |
//|                   if buffer is full then                                   |
//|                     write buffer to disk                                   |
//|                     reset buffer used count                                |
//|                   endif                                                    |
//|                   if remaining data of pszData exceeds buffer size then    |
//|                     write remaining data directly to file                  |
//|                   else                                                     |
//|                     add rest of pszData to buffer                          |
//|                     adjust buffer used count                               |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+
USHORT UtlBufWrite( PBUFCB pBufCB, PSZ pszData, ULONG ulLength, BOOL fMsg )
{
  return( UtlBufWriteHwnd( pBufCB, pszData, ulLength, fMsg, NULLHANDLE ) );
}

USHORT UtlBufWriteHwnd
(
  PBUFCB pBufCB,
  PSZ pszData,
  ULONG ulLength,             // number of bytes to be written!
  BOOL fMsg,
  HWND hwnd
)
{
   USHORT      usRC = 0;               // return code returned to caller
   ULONG       ulOutLength;            // actual data write length
   ULONG       ulRemaining;            // data not written to buffer
   ULONG       ulBytesWritten;         // number of bytes actual written to disk
   PSZ         pszErrParm;             // ptr to error parameter

   if ( pBufCB->fWrite )
   {
     ulOutLength = min( ulLength, (pBufCB->ulSize - pBufCB->ulUsed) );

     if ( ulOutLength )
     {
        memcpy( pBufCB->Buffer + pBufCB->ulUsed, pszData, ulOutLength );
        pBufCB->ulUsed += ulOutLength;
     } /* endif */

     if ( pBufCB->ulUsed == pBufCB->ulSize)  // is buffer full ???
     {
       usRC = UtlWriteHwnd( pBufCB->hFile,
                        pBufCB->Buffer,
                        pBufCB->ulUsed,
                        &ulBytesWritten,
                        fMsg, hwnd );
       if ( !usRC && (pBufCB->ulUsed != ulBytesWritten) )
       {
         usRC = ERROR_DISK_FULL;
         if ( fMsg )
         {
           pszErrParm = pBufCB->szFileName,
           UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszErrParm, DOS_ERROR, hwnd );
         } /* endif */
       } /* endif */
       pBufCB->ulUsed = 0;
     } /* endif */

     ulRemaining = ulLength - ulOutLength;

     if ( (usRC == NO_ERROR) && ( ulRemaining >= pBufCB->ulSize ) )
     {
       /*****************************************************************/
       /* size of remaining data exceeds buffer size, write data        */
       /* directly                                                      */
       /*****************************************************************/
       usRC = UtlWriteHwnd( pBufCB->hFile,
                        pszData + ulOutLength,
                        ulRemaining,
                        &ulBytesWritten,
                        fMsg, hwnd );
       if ( !usRC && (ulRemaining != ulBytesWritten) )
       {
         usRC = ERROR_DISK_FULL;
         if ( fMsg )
         {
           pszErrParm = pBufCB->szFileName,
           UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszErrParm, DOS_ERROR, hwnd );
         } /* endif */
       } /* endif */
     }
     else if ( ulRemaining )             // data not written completely ???
     {
        memcpy( pBufCB->Buffer + pBufCB->ulUsed,
                pszData + ulOutLength,
                ulRemaining );
        pBufCB->ulUsed += ulRemaining;
     } /* endif */
   }
   else
   {
     usRC = ERROR_INVALID_ACCESS;
   } /* endif */

   return( usRC );
} /* end of function UtlBufWrite */

USHORT UtlBufWriteW( PBUFCB pBufCB, PSZ_W pszData, ULONG ulLength, BOOL fMsg )
{
  return( UtlBufWriteHwndW( pBufCB, pszData, ulLength, fMsg, NULLHANDLE ) );
}

USHORT UtlBufWriteHwndW
(
  PBUFCB pBufCB,
  PSZ_W  pszData,
  ULONG  ulLength,    // bytes to be written!!
  BOOL fMsg,
  HWND hwnd
)
{
   return(UtlBufWriteHwnd( pBufCB, (PSZ) pszData, ulLength, fMsg, hwnd ));

} /* end of function UtlBufWriteHwndW */

// I/F to UtlBufWrite with character conversion
USHORT UtlBufWriteConv( PBUFCB pBufCB, PSZ_W pBuf, ULONG lLen, BOOL fMsg,
                          USHORT usFormat, ULONG ulCP, ULONG ulAnsiCP )
{
  USHORT usRC = NO_ERROR;

  // handle different formats
  switch ( usFormat )
  {
    case SGMLFORMAT_ANSI:
    {
         PSZ pConvBuffer = NULL;

         if ( UtlAlloc( (PVOID *)&pConvBuffer, 0L, lLen, ERROR_STORAGE ) )
         {
			 LONG   lRc = NO_ERROR;
			 ULONG  ulChars = 0;
             ulChars = UtlDirectUnicode2AnsiBuf( pBuf, pConvBuffer, (lLen/2), lLen,
                                              ulAnsiCP, fMsg, &lRc );

			 usRC = (USHORT)lRc;
			 if (!usRC)
			 {
             	usRC = UtlBufWrite( pBufCB, pConvBuffer, ulChars, fMsg );
		 	 }
           UtlAlloc( (PVOID *)&pConvBuffer, 0L, 0L, NOMSG );
         }
         else
         {
           usRC = ERROR_NOT_ENOUGH_MEMORY;
         } /* endif */
     }
     break;

    case SGMLFORMAT_ASCII:
     {
          PSZ pConvBuffer = NULL;

          if ( UtlAlloc( (PVOID *)&pConvBuffer, 0L, (lLen), ERROR_STORAGE ) )
          {
            ULONG ulChars = Unicode2ASCIIBuf( pBuf, pConvBuffer, (lLen/2), lLen, ulCP );

            usRC = UtlBufWrite( pBufCB, pConvBuffer, ulChars, fMsg );
            UtlAlloc( (PVOID *)&pConvBuffer, 0L, 0L, NOMSG );
          }
          else
          {
            usRC = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
      }
      break;

   case SGMLFORMAT_UNICODE:
   default:
      usRC = UtlBufWriteW( pBufCB, pBuf, lLen, fMsg );
      break;


   }/*endswitch */

  return( usRC );

} /* end of function UtlBufWriteConv */




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlBufRead             Read data buffered from file      |
//+----------------------------------------------------------------------------+
//|Function call:     UtlBufRead( PBUFCB pBufCB, PSZ pszData, BOOL fMsg )      |
//+----------------------------------------------------------------------------+
//|Description:       Reads data buffered from file.                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBUFCB   pBufCB     pointer to buffered input/output CB  |
//|                   PSZ      pszData    ptr to buffer for data               |
//|                   USHORT   ulLength   length of data to read               |
//|                   PUSHORT  pulBytesRead bytes actually read from file      |
//|                   BOOL     fMsg  TRUE = show error messages                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      function completed successfully                   |
//|                   other  error codes from Dosxxxx calls                    |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pBufCB must have been created using UtlBufOpen           |
//+----------------------------------------------------------------------------+
//|Samples:           usRC = UtlBufRead( pCB, pBuffer, ulLength, &usRead );    |
//+----------------------------------------------------------------------------+
//|Function flow:     TBD                                                      |
//+----------------------------------------------------------------------------+
USHORT UtlBufRead
(
  PBUFCB   pBufCB,                     // pointer to buffered input/output CB
  PSZ      pszData,                    // ptr to buffer for data
  ULONG    ulLength,                   // length of data to read
  PULONG   pulBytesRead,               // bytes actually read from file
  BOOL     fMsg                        // TRUE = show error messages
)
{
   return( UtlBufReadHwnd( pBufCB, pszData, ulLength, pulBytesRead, fMsg,
                           NULLHANDLE ) );
}

USHORT UtlBufReadHwnd
(
  PBUFCB   pBufCB,                     // pointer to buffered input/output CB
  PSZ      pszData,                    // ptr to buffer for data
  ULONG    ulLength,                   // length of data to read
  PULONG   pulBytesRead,               // bytes actually read from file
  BOOL     fMsg,                       // TRUE = show error messages
  HWND     hwnd                        // handle for error messages
)
{
   USHORT      usRC = 0;               // return code returned to caller
   ULONG       ulBytesToRead;          // number of bytes to read from disk
   ULONG       ulBytesRead;            // number of bytes actual read from disk
   ULONG       ulBytesToCopy;          // number of bytes to be copied
   ULONG       ulBytesCopied = 0;      // number of bytes copied to data buffer

   if ( !pBufCB->fWrite )
   {
     /*****************************************************************/
     /* loop until all bytes are read or end-of-file is reached       */
     /*****************************************************************/
     while ( (usRC == NO_ERROR) &&
             (ulBytesCopied < ulLength) &&
             (pBufCB->ulRemaining || pBufCB->ulUsed) )
     {
       /***************************************************************/
       /* copy data from input buffer to data area                    */
       /***************************************************************/
       if ( pBufCB->ulUsed )
       {
         /*************************************************************/
         /* Get length of data area which can be copied to target     */
         /* buffer                                                    */
         /*************************************************************/
         ulBytesToCopy   = min( pBufCB->ulUsed, (ulLength - ulBytesCopied) );

         /*************************************************************/
         /* copy data and adjust variables                            */
         /*************************************************************/
         memcpy( pszData + ulBytesCopied,
                 pBufCB->Buffer + pBufCB->ulProcessed,
                 ulBytesToCopy );
         ulBytesCopied   += ulBytesToCopy;
         pBufCB->ulUsed  -= ulBytesToCopy;
         pBufCB->ulProcessed += ulBytesToCopy;
       } /* endif */

       /***************************************************************/
       /* read into buffer if buffer is empty                         */
       /***************************************************************/
       if ( !pBufCB->ulUsed && pBufCB->ulRemaining )
       {
         ulBytesToRead = min( pBufCB->ulSize, pBufCB->ulRemaining );
         usRC = UtlReadHwnd( pBufCB->hFile,
                         pBufCB->Buffer,
                         ulBytesToRead,
                         &ulBytesRead,
                         fMsg, hwnd );
         if ( !usRC && (ulBytesToRead != ulBytesRead) )
         {
           usRC = ERROR_READ_FAULT;
         } /* endif */
         if ( usRC == NO_ERROR )
         {
           pBufCB->ulRemaining -= ulBytesRead;
           pBufCB->ulUsed       = ulBytesRead;
           pBufCB->ulProcessed  = 0;
         } /* endif */
       } /* endif */
     } /* endwhile */
   }
   else
   {
     usRC = ERROR_INVALID_ACCESS;
   } /* endif */

   /*******************************************************************/
   /* set caller's number-of-bytes-read variable                      */
   /*******************************************************************/
   *pulBytesRead = ulBytesCopied;

   return( usRC );
} /* end of function UtlBufRead */

/**********************************************************************/
/* Array for drive name checking                                      */
/**********************************************************************/
BYTE isValidChar[256] = {
/* 0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 1 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 2 */  0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
/* 3 */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
/* 4 */  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 5 */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
/* 6 */  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 7 */  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
/* 8 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 9 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* A */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* B */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* C */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* D */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* E */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* F */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlCheckPath           Check syntax of give path         |
//+----------------------------------------------------------------------------+
//|Function call:     UtlCheckPath( PSZ pszPath, PULONG pulFlags )             |
//+----------------------------------------------------------------------------+
//|Description:       Checks given path and sets flags describing the error    |
//|                   condition and/or path contents.                          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszPath    pointer to path being checked        |
//|                   ULONG    ulInFlags  input flags:                         |
//|                      CHKPATH_WILDCARDS       allow wildcard chars in path  |
//|                                                                            |
//|                   PULONG   pulFlags   ptr to ORed path description/error   |
//|                                       flags or NULL if flags are not       |
//|                                       requested                            |
//|                                                                            |
//|                      Flags describing the error                            |
//|                      CHKPATH_PATH_IS_EMPTY   path does not contain any data|
//|                      CHKPATH_DRIVE_INVALID   drive letter is invalid       |
//|                      CHKPATH_DRIVE_NOTEXIST  drive does not exist          |
//|                      CHKPATH_PATH_INVALID    path is invalid (i.e. path    |
//|                                              contains invalid characters or|
//|                                              does not follow the 8.3 nota- |
//|                                              tion)                         |
//|                                                                            |
//|                      Flags describing the path                             |
//|                      CHKPATH_DRIVE_SPECIFIED drive letter is part of path  |
//|                      CHKPATH_ABSOLUT_PATH    path start with a backslash   |
//|                      CHKPATH_BS_AT_END       path ends with a backslash    |
//|                      CHKPATH_IS_DEVICE       path is a device name (e.g.   |
//|                                              CON, LPT1: )                  |
//|                                              this flag/check is not active |
//|                                              yet!                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   path is (= seems to be) valid                     |
//|                   FALSE  path is invalid                                   |
//+----------------------------------------------------------------------------+
BOOL UtlCheckPath( PSZ pszPath, ULONG ulInFlags, PULONG pulFlags )
{
   BOOL    fOK  = TRUE;                // function return code
   CHAR    szDriveList[MAX_DRIVELIST]; // buffer for valid drives

   /*******************************************************************/
   /* Initialize flags                                                */
   /*******************************************************************/
   if ( pulFlags != NULL )
   {
     *pulFlags = 0L;
   } /* endif */

   /*******************************************************************/
   /* Strip any blanks from path                                      */
   /*******************************************************************/
   if ( fOK )
   {
     UtlStripBlanks( pszPath );
     if ( pszPath[0] == EOS )
     {
       if ( pulFlags != NULL )   *pulFlags |= CHKPATH_PATH_IS_EMPTY;
       fOK = FALSE;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Process any drive letter                                        */
   /*******************************************************************/
   if ( fOK )
   {
     if ( (pszPath[0] != EOS) && (pszPath[1] == COLON) )
     {
       if ( pulFlags != NULL )   *pulFlags |= CHKPATH_DRIVE_SPECIFIED;

       if ( isalpha(pszPath[0])  )
       {
         UtlGetDriveList( (PBYTE)szDriveList );
         if ( strchr( szDriveList, toupper(pszPath[0]) ) == NULL )
         {
           if ( pulFlags != NULL )   *pulFlags |= CHKPATH_DRIVE_NOTEXIST;
             fOK = FALSE;
         } /* endif */
       }
       else
       {
         if ( pulFlags != NULL )   *pulFlags |= CHKPATH_DRIVE_INVALID;
         fOK = FALSE;
       } /* endif */
       pszPath = pszPath += 2;              // skip drive info
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Set absolut/relative path flag                                  */
   /*******************************************************************/
   if ( fOK )
   {
     if ( pszPath[0] == BACKSLASH )
     {
       if ( pulFlags != NULL )   *pulFlags |= CHKPATH_ABSOLUT_PATH;
     } /* endif */
   } /* endif */

   /*****************************************************************/
   /* Check length of directory and filenames and check for         */
   /* valid characters                                              */
   /*****************************************************************/
   if ( fOK )
   {
     USHORT usNameLen  = 0;
     USHORT usExtLen   = 0;
     BOOL   fExtension = FALSE;
     PSZ    pszCurPos  = pszPath;

     while ( fOK && (*pszCurPos != EOS) )
     {
       switch ( *pszCurPos )
       {
         case BACKSLASH:
           usNameLen  = 0;
           usExtLen   = 0;
           fExtension = FALSE;
           break;

         case DOT :
           if ( fExtension )
           {
             fOK = TRUE; // we do not support .. in directory names right now
           }
           else
           {
             fExtension = TRUE;
           } /* endif */
           break;

         case '*':
         case '?':
           if ( !(ulInFlags & CHKPATH_WILDCARDS) )
           {
             fOK = FALSE;
           }
           else if ( fExtension )
           {
// removed length check to allow long directory names
//           if (usExtLen >= 3 )
//           {
//             fOK = FALSE;
//           }
//           else
             {
               usExtLen++;
             } /* endif */
           }
           else
           {
// removed length check to allow long directory names
//           if (usNameLen >= 8 )
//           {
//             fOK = FALSE;
//           }
//           else
             {
               usNameLen++;
             } /* endif */
           } /* endif */
           break;

         default :
           if ( !isValidChar[*pszCurPos] )
           {
//             fOK = FALSE;            // removed to allow all characters in file and directory names
           }
           else if ( fExtension )
           {
// removed length check to allow long directory names
//             if (usExtLen >= 3 )
//             {
//               fOK = FALSE;
//             }
//             else
             {
               usExtLen++;
             } /* endif */
           }
           else
           {
// removed length check to allow long directory names
//             if (usNameLen >= 8 )
//             {
//               fOK = FALSE;
//             }
//             else
             {
               usNameLen++;
             } /* endif */
           } /* endif */
           break;
       } /* endswitch */
       pszCurPos++;
     } /* endwhile */

     if ( !fOK )
     {
       if ( pulFlags != NULL )   *pulFlags |= CHKPATH_PATH_INVALID;

     } /* endif */
   } /* endif */

   return ( fOK );
} /* end of UtlCheckPath */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlCheckIntName        Check syntax of internal names    |
//+----------------------------------------------------------------------------+
//|Function call:     UtlCheckPath( PSZ pszName, PSZ pszOutName )              |
//+----------------------------------------------------------------------------+
//|Description:       Checks syntactically given internal name and (if         |
//|                   pszOutName is not NULL) returns the stripped and         |
//|                   uppercased internal name in buffer pszOutName.           |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszName    pointer to name being checked        |
//|                   PSZ      pszOutName ptr to buffer for stripped and       |
//|                                       uppercase name or NULL if no         |
//|                                       update name is required              |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   name is valid                                     |
//|                   FALSE  name is invalid                                   |
//+----------------------------------------------------------------------------+
BOOL UtlCheckIntName( PSZ pszName, PSZ pszOutName )
{
   BOOL    fOK  = TRUE;                // function return code
   CHAR    szTempName[CCHMAXPATH];     // buffer for modified name

   /*******************************************************************/
   /* Preprocess name                                                 */
   /*******************************************************************/
   if ( fOK )
   {
     strncpy( szTempName, pszName, sizeof(szTempName) - 1 );
     szTempName[sizeof(szTempName)-1] = EOS;
     UtlStripBlanks( szTempName );
     _strupr( szTempName );
     if ( szTempName[0] == EOS )
     {
       fOK = FALSE;                    // no name has been specified
     }
     else if ( strlen(szTempName) > MAX_FOLDERNAME )
     {
       fOK = FALSE;                    // name is too long
     } /* endif */
   } /* endif */

   /*****************************************************************/
   /* Check letters of given name (only alphanumeric characters     */
   /* may be used)                                                  */
   /*****************************************************************/
   if ( fOK )
   {
     PSZ    pszCurPos  = szTempName;

     while ( fOK && (*pszCurPos != EOS) )
     {
       // do not use isalnum/isdigit anymore, german umlaut ue and oe are treated as alnums
       // as well...
       BYTE bTest = (BYTE)*pszCurPos;
       if ( isdigit(bTest) ||
            ((bTest >= 'A') && (bTest <= 'Z')) )
       {
         pszCurPos++;                  // continue with next character
       }
       else
       {
         fOK = FALSE;                  // invalid character in name
       } /* endif */
     } /* endwhile */
   } /* endif */

   /*******************************************************************/
   /* return stripped and uppercased name                             */
   /*******************************************************************/
   if ( fOK && (pszOutName != NULL) )
   {
     strcpy( pszOutName, szTempName );
   } /* endif */

   return ( fOK );
} /* end of UtlCheckIntName */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlCheckLongName        Check syntax of long names       |
//+----------------------------------------------------------------------------+
//|Function call:     UtlCheckLongName( PSZ pszName )                          |
//+----------------------------------------------------------------------------+
//|Description:       Checks the length and the characters of the given long   |
//|                   name (invalid characters are backslash, question mark and|
//|                   asterisk).                                               |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszName    pointer to name being checked        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   name is valid                                     |
//|                   FALSE  name is invalid                                   |
//+----------------------------------------------------------------------------+
BOOL UtlCheckLongName( PSZ pszName )
{
   BOOL    fOK  = TRUE;                // function return code
   PPROPSYSTEM pPropSys= NULL;

   // check length of name
   if ( strlen(pszName) >= MAX_LONGFILESPEC )
   {
     fOK = FALSE;                    // name is too long
   } /* endif */

   // check for invalid characters in the given name
   if ( fOK )
   {
     pPropSys = GetSystemPropPtr();
     while ( fOK && (*pszName != EOS) )
     {
       if ( IsDBCS_CP(pPropSys->ulSystemPrefCP) &&
            (isdbcs1ex((USHORT)pPropSys->ulSystemPrefCP, *pszName ) == DBCS_1ST) &&
            (pszName[1] != EOS) )
       {
         pszName++;                    // skip both bytes of DBCS character
         pszName++;
       }
//     if ( (*pszName == '\\') || (*pszName == '?') || (*pszName == '*') )
       if ( strchr("\\?*<>:|/", *pszName ) ) 
       {
         fOK = FALSE;                  // invalid character in name
       }
       else
       {
         pszName++;                    // continue with next character
       } /* endif */
     } /* endwhile */
   } /* endif */

   return ( fOK );
} /* end of UtlCheckLongName */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlInsertCurDir                                          |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ      pInFilename,                                    |
//|                   PSZ      **pppCurDirIndex                                |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     filename is now fully qualified                 |
//|                   FALSE    filename wrong                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     get ptr to curdir array                                  |
//|                   if filename has a ':' at 2nd position:                   |
//|                     get list of all drives                                 |
//|                     find out which drive is spec at 1st pos of filename    |
//|                     insert path of specified drive in filename             |
//|                     set filename to new fully qualified filename           |
//+----------------------------------------------------------------------------+
BOOL
UtlInsertCurDir
(
   PSZ      pInFilename,
   PSZ      **pppCurDirIndex,
   PSZ      pBuf
)
{
   PSZ        *ppCurDirToken;
   ULONG      ulI;
   BOOL       fOK = FALSE;
   PSZ        pTemp;
   ULONG      ulJ;

   ppCurDirToken = *pppCurDirIndex;

   /******************************************************************/
   /* if filename is e.g. d:device.scr, the current directory of d:  */
   /* has to be inserted                                             */
   /******************************************************************/
   if (*(pInFilename+1) == ':' )
   {
     ulI = 0;
     /*****************************************************************/
     /* loop til specified drive in curdir array                      */
     /*****************************************************************/
     while ( !fOK && ppCurDirToken[ulI] )
     {
       strcpy(pBuf, ppCurDirToken[ulI]);
       if ( toupper(*pInFilename) == toupper(*pBuf) ) // if same drive...
       {
         ulJ = strlen(pBuf);
         if (*(pBuf + ulJ - 1) != '\\' )
         {
           *(pBuf + ulJ) = '\\';
           ulJ++;
         } /* endif */
         pTemp = pBuf + ulJ;
         strcpy(pTemp, pInFilename + 2);
         fOK = TRUE;
       }
       else
       {
         ulI++;
       } /* endif */
     } /* endwhile */
   }
   else
   {
     /*****************************************************************/
     /* filename is e.g.device.scr, insert path and drive at begin    */
     /* 1st entry in CurDir array is current directory                */
     /*****************************************************************/
     strcpy(pBuf, ppCurDirToken[0]);
     ulI = strlen(pBuf);
     if (*(pBuf + ulI - 1) != '\\' )
     {
       *(pBuf + ulI) = '\\';
       ulI++;
     } /* endif */
     pTemp = pBuf + ulI;
     strcpy(pTemp, pInFilename);
     fOK = TRUE;
   } /* endif */

   return(fOK);
} /* end of function UtlInsertCurDir */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlLoadLongFileNames                                     |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to UtlLoadLongFileNamesHwnd           |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ - path to search for files (or directories)       |
//|                   2. USHORT - file attributes (NORMAL, DIRECTORY, etc)     |
//|                   3. HWND - handle of target list box                      |
//|                   4. USHORT - flags for special formats of file names      |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       number   number of file names inserted                   |
//|                   UTLERROR in case of errors                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtLoadFileNamesHwnd                                 |
//+----------------------------------------------------------------------------+
SHORT UtlLoadLongFileNames( PSZ pszSearch, USHORT atrb, HWND hlbox, USHORT flg)
{
   return( UtlLoadLongFileNamesHwnd( pszSearch,
                                 atrb,
                                 hlbox,
                                 flg,
                                 NULLHANDLE ) );
}
//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlLoadLongFileNamesHwnd  Load file names into a list box|
//+----------------------------------------------------------------------------+
//|Description:       Load file names found in specified path into a list box  |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ - path to search for files (or directories)       |
//|                   2. USHORT - file attributes (NORMAL, DIRECTORY, etc)     |
//|                   3. HWND - handle of target list box                      |
//|                   4. USHORT - flags for special formats of file names      |
//|                   5. HWND - window handle                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       number   number of file names inserted                   |
//|                   UTLERROR in case of errors                               |
//+----------------------------------------------------------------------------+
//|Samples:           n = UtlLoadLongFileNames( "d:\eqfd", FILE_DIRECTORY,     |
//|                                         hLbox,                             |
//|                                         NAMFMT_NOEXT | NAMFMT_TOPSEL);     |
//+----------------------------------------------------------------------------+
//|Function flow:     initialize search using UtlFindFirst                     |
//|                   while ok and files found                                 |
//|                     if file does not match requested file type then        |
//|                       ignore file                                          |
//|                     elseif file name is CURRENT_DIR_NAME                   |
//|                       ignore file                                          |
//|                     elseif file name is PARENT_DIR_NAME                    |
//|                       if no directories requested or                       |
//|                          NAMFMT_ROOT flag has been specified then          |
//|                          ignore file                                       |
//|                       endif                                                |
//|                     else                                                   |
//|                       discard file extension if requested (NAMFMT_NOEXT)   |
//|                     endif                                                  |
//|                     if file is not to be ignored then                      |
//|                       insert file name into listbox                        |
//|                     endif                                                  |
//|                     search next file using UtlFindNext                     |
//|                   endwhile                                                 |
//|                   select first listbox item if NAMFMT_TOPSEL is set        |
//|                   return number of inserted items or UTLERROR in case of   |
//|                    errors                                                  |
//+----------------------------------------------------------------------------+
SHORT UtlLoadLongFileNamesHwnd( PSZ pszSearch, USHORT atrb, HWND hlbox,
                                USHORT flg, HWND hwnd )
{
  static LONGFILEFIND ResultBuf;       // Buffer for long file names
  HDIR    hDirHandle = HDIR_CREATE;    // DosFind routine handle
  USHORT  usTotal;
  USHORT  usRC;                        // return code of Dos... alias Utl...
  char    *p, *p2;
  USHORT  fdir = atrb & FILE_DIRECTORY;
  SHORT   sRetCode;                    // return code of function
  BOOL    fCombo;                      // is-a-combobox flag
  BOOL    fMsg = !(flg & NAMFMT_NOERROR);
  BOOL    fhDirCreated = FALSE;        // directory handle has been created

  ISCOMBOBOX( hlbox, fCombo );

  usRC = UtlFindFirstLongHwnd( pszSearch, &hDirHandle, atrb, &ResultBuf,
                               fMsg, hwnd);
  if ( usRC == NO_ERROR )
  {
    fhDirCreated = TRUE;               // directory handle has been created
  } /* endif */

  while ( !usRC )
  {
     p2 = ResultBuf.achName;

     if( fdir && !(fdir & ResultBuf.attrFile) )
     {
       /***************************************************************/
       /* File does not match requested file type                     */
       /***************************************************************/
       p2 = NULL;                    // do not insert into listbox
     }
     else if( strcmp( p2, CURRENT_DIR_NAME ) == 0 )
     {
       /***************************************************************/
       /* File is current directory (".")                             */
       /***************************************************************/
         p2 = NULL;                    // do not insert into listbox
     }
     else if ( strcmp( p2, PARENT_DIR_NAME ) == 0 )
     {
       /***************************************************************/
       /* Process parent directory name ".."                          */
       /***************************************************************/
       if ( !fdir || (flg & NAMFMT_NOROOT) )
       {
          p2 = NULL;               // do not insert into listbox
       }
       else
       {
          p = strchr( pszSearch, BACKSLASH );
          if ( (p == NULL) || (p == strrchr( pszSearch, BACKSLASH) ) )
          {
             // root directory shouldn't have any parent directory !!!
             p2 = NULL;            // do not insert into listbox
          } /* endif */
       } /* endif */
     }
     else
     {
       /***************************************************************/
       /* Process file names using the supplied process flags         */
       /***************************************************************/
       if( flg & NAMFMT_NOEXT )
       {
          if( (p = strrchr( p2, DOT)) != NULL )
          {
             *p = '\0';
          } /* endif */
       } /* endif */

       if ( flg & NAMFMT_NODIR )
       {
          p2 = ( (p=strrchr( p2, BACKSLASH))!= NULL ) ? p + 1 : p2;
       }
       else if ( flg & NAMFMT_NODRV )
       {
          p2 = ( *(p2+1) == COLON) ? p2 + 2 : p2;
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Insert string into listbox                                    */
     /*****************************************************************/
     if ( p2 )
     {
        if ( fCombo )
        {
          CBINSERTITEMHWND( hlbox, p2 );
        }
        else
        {
          INSERTITEMHWND( hlbox, p2 );
        } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Get next file                                                 */
     /*****************************************************************/
     usRC = UtlFindNextLongHwnd( hDirHandle, &ResultBuf, fMsg, hwnd);
  } /* endwhile */

  /********************************************************************/
  /* Close file find handle                                           */
  /********************************************************************/
  if ( fhDirCreated )
  {
     UtlFindCloseLongHwnd( hDirHandle, fMsg, hwnd);
  } /* endif */

  /********************************************************************/
  /* Get total number of list box entries                             */
  /********************************************************************/
  if ( fCombo )
  {
    usTotal = (USHORT)CBQUERYITEMCOUNTHWND( hlbox );
  }
  else
  {
    usTotal = (USHORT)QUERYITEMCOUNTHWND( hlbox );
  } /* endif */

  /********************************************************************/
  /* Select first item on request                                     */
  /********************************************************************/
  if ( (flg & NAMFMT_TOPSEL) && usTotal )
  {
    if ( fCombo )
    {
      CBSELECTITEMHWND(hlbox, 0);
    }
    else
    {
      SELECTITEMHWND(hlbox, 0);
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Build return code                                                */
  /********************************************************************/
  if ( ( usRC == 0 )                    ||
       ( usRC == ERROR_FILE_NOT_FOUND ) ||
       ( usRC == ERROR_NO_MORE_FILES ) )
  {
     sRetCode = usTotal;
  }
  else
  {
     sRetCode = UTLERROR;
  } /* endif */

  /********************************************************************/
  /* Return return code to caller                                     */
  /********************************************************************/
  return( sRetCode );

}

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlIsLongFileName         Checks for long file names     |
//+----------------------------------------------------------------------------+
//|Description:       Checks if the given file name is long file name or       |
//|                   contains lowercase or non-alphanumeric characters.       |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ - file name being tested                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL  (Tri-state!)                                       |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE  1  the file name is a long or special file name    |
//|                   FALSE 0  it is a normal file name                        |
//|                   2        it is a normal file name containing mixed-case  |
//|                            characters                                      |
//+----------------------------------------------------------------------------+
BOOL UtlIsLongFileName( PSZ pszFileName )
{
  BOOL  fIsLongName = FALSE;           // return code
  BOOL  fLowerCase  = FALSE;           // name-contains-lower-case-characters
  PSZ   pszExt;                        // ptr to file name extension

  // check for path delimiters witin fil ename (from realtive path import)
  if ( strchr( pszFileName, BACKSLASH ) != NULL )
  {
    fIsLongName = TRUE;                // treat as long file name
  } /* endif */

  // cut off extension and process seperately
  pszExt = strrchr( pszFileName, DOT );
  if ( pszExt != NULL )
  {
    *pszExt = EOS;
  } /* endif */

  // check length of file name
  if ( strlen(pszFileName) > 8 )
  {
    fIsLongName = TRUE;
  } /* endif */

  // check characters of file name
  if ( !fIsLongName )
  {
    while ( !fIsLongName && *pszFileName != EOS )
    {
      if ( islower(*pszFileName) )
      {
        fLowerCase = TRUE;
      }
      else
      {
        fIsLongName = !( isdigit(*pszFileName) || (*pszFileName == '@') ||
                         (*pszFileName == '$') || (*pszFileName == '_') ||
                         (*pszFileName == '-') || isupper(*pszFileName) );
      } /* endif */
      pszFileName++;
    } /* endwhile */
  } /* endif */

  // restore extension delimiter
  if ( pszExt != NULL )
  {
    *pszExt = DOT;
  } /* endif */

  // check length of extension
  if ( !fIsLongName && (pszExt != NULL) )
  {
    pszExt++;                          // position to first extension character

    if ( strlen(pszExt) > 3 )
    {
      fIsLongName = TRUE;
    } /* endif */
  } /* endif */

  // check characters of extension
  if ( !fIsLongName && (pszExt != NULL) )
  {
    while ( !fIsLongName && *pszExt != EOS )
    {
      if ( islower(*pszExt) )
      {
        fLowerCase = TRUE;
      }
      else
      {
        fIsLongName = !( isdigit(*pszExt) || (*pszExt == '@') ||
                         (*pszExt == '$') || (*pszExt == '_') ||
                         (*pszExt == '-') || isupper(*pszExt) );
      } /* endif */
      pszExt++;
    } /* endwhile */
  } /* endif */


  // return result to caller
  if ( !fIsLongName && fLowerCase )
  {
    fIsLongName = 2;  // set indicator for normal, but mixed-case name
  } /* endif */
  return( fIsLongName );
}

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlLongToShortName   Convert long file name to short name|
//+----------------------------------------------------------------------------+
//|Description:       Convert a long file name to a short file name by         |
//|                   stripping all invalid characters until the end of the    |
//|                   long name is detected or 8 characters have been          |
//|                   found. There is no check done if a file with this name   |
//|                   already exists.                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ - ptr to long file name                           |
//|                   2. PSZ - ptr to buffer for short name                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0        always                                          |
//+----------------------------------------------------------------------------+
USHORT UtlLongToShortName( PSZ pszLongName, PSZ pszShortName )
{
  SHORT i = 0;                         // number of characters in short name

  // skip any path information in long file name
  {
    PSZ pszTemp = strrchr( pszLongName, BACKSLASH );
    if ( pszTemp != NULL )
    {
      pszLongName = pszTemp + 1;
    } /* endif */
  }

  // scan long name and add valid characters to short file name
  while ( (i < 8) && (*pszLongName != EOS) )
  {
    BYTE bTemp = (BYTE)*pszLongName;
    if ( isalnum(bTemp) || (bTemp == '@') ||
         (bTemp == '$') || (bTemp == '_') ||
         (bTemp == '-') )
    {
      CHAR c = *pszLongName++;
      *pszShortName++ = (CHAR)toupper(c);
      i++;
    }
    else
    {
      pszLongName++;                   // try next character
    } /* endif */
  } /* endwhile */

  // add at least a dummy character if no valid characters were detected
  if ( i == 0 )
  {
    *pszShortName++ = 'X';
  } /* endif */

  // terminate short file name
  *pszShortName = EOS;

  // return to caller
  return( 0 );
}
//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlSupportsLongNames  Check drive for long file name supp|
//+----------------------------------------------------------------------------+
//|Description:       Checks if the specified drive supports long file names.  |
//+----------------------------------------------------------------------------+
//|Parameters:        1. CHAR - drive letter of drive to check                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     drive supports long file names                  |
//|                   FALSE    drive does not support long file names          |
//+----------------------------------------------------------------------------+
BOOL UtlSupportsLongNames( CHAR chDrive )
{
  BOOL fSupportsLongNames = FALSE;

  {
        DWORD dwMaxLength, dwFileSysFlags;
    CHAR szRoot[4] = "C:\\";
    szRoot[0] = chDrive;
    if ( GetVolumeInformation( szRoot,
                                                   NULL, // ptr to volume name buffer
                                       0,       // length of volume name buffer
                                       NULL, // ptr to volume serial number
                                       &dwMaxLength, // max file name length
                                       &dwFileSysFlags, // ptr to file system flags
                                       NULL, // ptr to file system name buffer
                                       0  // length of file system name buffer
                                   ) != 0 )
    {
      if ( dwMaxLength > (8 + 3 + 2) )
      {
        fSupportsLongNames = TRUE;
      } /* endif */
    } /* endif */
  }
  return( fSupportsLongNames );
} /* end of function UtlSupportsLongNames */


//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlHandleConversionStrings  Handle conversion options    |
//+----------------------------------------------------------------------------+
//|Description:       Has two different modes of operation:                    |
//|                     CONVLOAD_MODE                                          |
//|                       Loads the conversion strings from the external       |
//|                       conversion table into a combobox or listbox          |
//|                     CONVCHECK_MODE                                         |
//|                       Lookups the supplied conversion in the conversion    |
//|                       table and returns the associated code page           |
//|                       and conversion flag if found                         |
//+----------------------------------------------------------------------------+
//|Parameters:        1. HWND - handle of listbox or combobox                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     drive supports long file names                  |
//|                   FALSE    drive does not support long file names          |
//+----------------------------------------------------------------------------+
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
)
{
  BOOL    fOK = TRUE;                  // internal O.K. flag
  BOOL    fCombo = TRUE;               // is-a-combobox flag
  USHORT  usEntries = 0;               // number of entries added t listbox
  CHAR    szFileName[MAX_EQF_PATH];    // buffer for file name
  ULONG   ulLength;                    // length of buffer data
  PSZ     pszBuffer = NULL;            // ptr to buffer with loaded file

  // check if target is a listbox or a combobox
  if ( usMode == CONVLOAD_MODE )
  {
    ISCOMBOBOX( hwndLB, fCombo );
  }
  else if ( usMode == CONVCHECK_MODE )
  {
    if ( pusCodePage ) *pusCodePage = 0;
    if ( pusConvFlag ) *pusConvFlag = 0;
  } /* endif */

  // setup name of file containg the conversion entries
  UtlMakeEQFPath( szFileName, NULC, TABLE_PATH, NULL );
  strcat( szFileName, BACKSLASH_STR );
  strcat( szFileName, CONVERSIONTABLENAME );

  // load the file into memory
  if ( fOK )
  {
    fOK = UtlLoadFileL( szFileName, (PVOID *)&pszBuffer,
                       &ulLength, FALSE, FALSE );

    // remove EOF character, set file end to EOS
    if ( fOK )
    {
      ULONG i = ulLength;
      PSZ    pszTemp = pszBuffer + ulLength - 1;
      if ( !ulLength) ulLength = 1;
      while ((i > 0) && ( *pszTemp == 0x1A ))
      {
        pszTemp--;
        i--;
      } /* endwhile */;
      *pszTemp = EOS;
    } /* endif */
  } /* endif */

  // process the entries in the conversion file
  if ( fOK )
  {
    CHAR szConversion[MAX_DESCRIPTION];// buffer for current conversion

    PSZ pszLine = pszBuffer;
    BOOL  fDone = FALSE;

    while ( !fDone && (*pszLine != EOS) )
    {
      // handle current line
      if ( *pszLine == '*' )
      {
        // skip comment line
        while ( *pszLine && (*pszLine != LF) && (*pszLine != CR) )
        {
          pszLine++;
        } /* endwhile */
      }
      else
      {
        PSZ pszStartOfEntry;

        // skip leading whitespace
        while ( (*pszLine == SPACE) || (*pszLine == LF) || (*pszLine == CR) )
        {
          pszLine++;
        } /* endwhile */

        // find end of conversion text or end of line
        pszStartOfEntry = pszLine;
        while ( *pszLine && (*pszLine != LF) && (*pszLine != CR) &&
                (*pszLine != SEMICOLON) )
        {
          pszLine++;
        } /* endwhile */

        // strip trailing whitespace from conversion entry
        // and add string to listbox or use it for checking
        {
          PSZ pszEndOfEntry = pszLine - 1;
          while ( (pszEndOfEntry > pszStartOfEntry) &&
                  (*pszEndOfEntry == SPACE)  )
          {
            pszEndOfEntry--;
          } /* endwhile */
          if ( pszEndOfEntry > pszStartOfEntry )
          {
            CHAR chTemp;
            pszEndOfEntry++;
            chTemp = *pszEndOfEntry;
            *pszEndOfEntry = EOS;
            strncpy( szConversion, pszStartOfEntry, sizeof(szConversion) );
            szConversion[sizeof(szConversion)-1] = EOS;
            *pszEndOfEntry = chTemp;

            if ( usMode == CONVLOAD_MODE )
            {
              EQFOemToAnsi( szConversion, szConversion );
              if ( fCombo )
              {
                CBINSERTITEMHWND( hwndLB, szConversion );
              }
              else
              {
                INSERTITEMHWND( hwndLB, szConversion );
              } /* endif */
              usEntries++;
            }
            else if ( usMode == CONVCHECK_MODE )
            {
              if ( _stricmp( pszConversion, szConversion ) == 0 )
              {
                PSZ pszTemp = pszEndOfEntry;
                USHORT   usCodePage = 0;
                USHORT   usConvFlag = 0;

                fDone = TRUE;          // we detected the correct entry

                // extract the codepage
                while ( *pszTemp == BLANK ) pszTemp++;
                if ( *pszTemp == SEMICOLON )
                {
                  pszTemp++;
                  while ( *pszTemp == BLANK ) pszTemp++;
                  while ( isdigit(*pszTemp) )
                  {
                    usCodePage = (usCodePage * 10) + (*pszTemp - '0');
                    pszTemp++;
                  } /* endwhile */
                  while ( *pszTemp == BLANK ) pszTemp++;
                } /* endif */

                // extract the conversion flag
                if ( *pszTemp == SEMICOLON )
                {
                  pszTemp++;
                  while ( *pszTemp == BLANK ) pszTemp++;
                  while ( isdigit(*pszTemp) )
                  {
                    usConvFlag = (usConvFlag * 10) + (*pszTemp - '0');
                    pszTemp++;
                  } /* endwhile */
                } /* endif */

                // return any found values to caller
                if ( pusCodePage ) *pusCodePage = usCodePage;
                if ( pusConvFlag ) *pusConvFlag = usConvFlag;
              } /* endif */
            } /* endif */
          } /* endif */
        }

        // skip everything up to end of line
        while ( *pszLine && (*pszLine != LF) && (*pszLine != CR) )
        {
          pszLine++;
        } /* endwhile */
      } /* endif */

      // skip line end delimiters
      while ( (*pszLine == LF) || (*pszLine == CR) )
      {
        pszLine++;
      } /* endwhile */
    } /* endwhile */
  } /* endif */

  // cleanup
  if ( pszBuffer ) UtlAlloc( (PVOID *)&pszBuffer, 0L, 0L, NOMSG );

  return( usEntries );
} /* end of function UtlHandleConversionStrings */

// create fully qualified file name from supplied data
USHORT UtlMakeFileName
(
  CHAR        chDrive,                 // driver letter or NULC
  PSZ         pszPath,                 // path part
  PSZ         pszName,                 // file name
  PSZ         pszExt,                  // extension or NULL if none
  PSZ         pszBuffer                // buffer for created file name
)
{
  if ( chDrive != EOS )
  {
    *pszBuffer++ = chDrive;
  } /* endif */

  strcpy( pszBuffer, pszPath );

  strcat( pszBuffer, BACKSLASH_STR );

  strcat( pszBuffer, pszName );

  if ( pszExt != NULL )
  {
    if ( *pszExt != '.' )
    {
      strcat( pszBuffer, "." );
    } /* endif */
  strcat( pszBuffer, pszExt );
  } /* endif */
  return( 0 );
} /* end of function UtlMakeFileName */

VOID CloseFile
(
  HFILE * hFile                   // Pointer to the file handle
)
  // Close the file if the file handle is not NULL
{
  USHORT       usDosRc;           // Dos function return code

  // Return code for test purposes only
  if (*hFile)
  {
    usDosRc = UtlClose( *hFile, FALSE );
    *hFile  = NULLHANDLE;
  } /* endif */
} /* end of function CloseFile */


