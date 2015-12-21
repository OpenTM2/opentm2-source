/*!
   EQFPRO00.C - EQF Property Handler
*/
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_EDITORAPI        // editor API
#include "eqf.h"                  // General .H for EQF
#include "eqfpro00.h"             // Property Handler defines

// activate the following define for property failure logging
//#define PROPLOGGING

// IDA pointer for batch mode
PPROP_IDA pPropBatchIda = NULL;
HWND      hwndPropertyHandler = NULL;

/*!
 Property Handler Window Proc
*/
MRESULT APIENTRY PROPERTYHANDLERWP
(
  HWND hwnd,
  WINMSG message,
  WPARAM mp1,
  LPARAM mp2
)
{
    PPROP_IDA     pIda;           // Points to instance area
    PPROPMSGPARM  pmsg;
    PPROPHND      hprop;
    PPROPCNTL     pcntl;
    USHORT        size;

    switch( message) {
/*--------------------------------------------------------------------------*/
      case WM_CREATE:
        if( !UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof( *pIda), ERROR_STORAGE ) )
          return( (MRESULT)TRUE);      // do not create the window
        memset( pIda, NULC, sizeof( *pIda));
        size = sizeof(pIda->IdaHead.szObjName);
        pIda->IdaHead.pszObjName = pIda->IdaHead.szObjName;

        GetStringFromRegistry( APPL_Name, KEY_SysProp, pIda->IdaHead.szObjName, size, "" );
        if( GetSysProp( pIda))
          return( (MRESULT)TRUE);      // do not create the window
        ANCHORWNDIDA( hwnd, pIda);
        EqfInstallHandler( PROPERTYHANDLER, hwnd, clsPROPERTY);
        hwndPropertyHandler = hwnd;
        return( 0L);                 // continue with default proc

/*--------------------------------------------------------------------------*/
      case WM_CLOSE:
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        if (pIda)
          EqfRemoveHandler( TWBFORCE, PROPERTYHANDLER);
        return( 0L);

/*--------------------------------------------------------------------------*/
      case WM_DESTROY:
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
        hwndPropertyHandler = NULL;
        return( 0L);

/*--------------------------------------------------------------------------*/
      case WM_EQF_TERMINATE:
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        if( pIda->hSystem->pCntl->lFlgs & PROP_STATUS_UPDATED)
          PutItAway( pIda->hSystem->pCntl);
        WinDestroyWindow( pIda->IdaHead.hFrame);
        return( 0L);                 // continue with TWB shutdown

/*--------------------------------------------------------------------------*/
      case WM_EQF_QUERYSYSTEMPROPHND:
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        return( (MRESULT)pIda->hSystem);

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
      case WM_EQF_OPENPROPERTIES:
//      mp1 = not used
//      mp2 = PPROPMSGPARM
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        pmsg = (PPROPMSGPARM) PVOIDFROMMP2(mp2);
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop= MakePropHnd( pIda)) == NULL)
          return( 0L);
        hprop->pCntl = LoadPropFile( pIda, pmsg->pszName, pmsg->pszPath,
                                     pmsg->fFlg);
        if( !hprop->pCntl){
          FreePropHnd( pIda, hprop);
          return( 0L);
        }
        hprop->lFlgs |= pmsg->fFlg;
        return( (MRESULT)hprop);

/*--------------------------------------------------------------------------*/
      case WM_EQF_CREATEPROPERTIES:
//      mp1 = not used
//      mp2 = PPROPMSGPARM
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        pmsg = (PPROPMSGPARM) PVOIDFROMMP2(mp2);
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop= MakePropHnd( pIda)) == NULL)
          return( 0L);
        hprop->pCntl = CreatePropFile( pIda, pmsg->pszName, pmsg->pszPath,
                                       pmsg->fFlg, pmsg->fOverWrite );
        if( !hprop->pCntl)
        {
          FreePropHnd( pIda, hprop);
          return( 0L);
        }
        hprop->lFlgs = PROP_ACCESS_READWRITE;
        return( (MRESULT)hprop);

/*--------------------------------------------------------------------------*/
      case WM_EQF_DELETEPROPERTIES:
//      mp1 = not used
//      mp2 = PPROPMSGPARM
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        pmsg = (PPROPMSGPARM) PVOIDFROMMP2(mp2);
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        pcntl = FindPropCntl( pIda->TopCntl, pmsg->pszName, pmsg->pszPath);
        if( pcntl)
        {
          *pIda->pErrorInfo = ErrProp_ObjectBusy;
          return( (MRESULT)-1);        // indicate error
        }
        return( (MRESULT)DeletePropFile( pIda, pmsg->pszName, pmsg->pszPath));

/*--------------------------------------------------------------------------*/
      case WM_EQF_CLOSEPROPERTIES:
//      mp1 = not used
//      mp2 = PPROPMSGPARM
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        pmsg = (PPROPMSGPARM) PVOIDFROMMP2(mp2);
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop=FindPropHnd( pIda->TopUsedHnd, (PPROPHND) pmsg->hObject)) == NULL)
        {
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          return( (MRESULT)-1);        // indicate error
        }
        pcntl = hprop->pCntl;
        if( pcntl->lFlgs & PROP_STATUS_UPDATED)
          if( pmsg->fFlg & PROP_FILE)
          {
            if( !(hprop->lFlgs & PROP_ACCESS_WRITE))
            {
              *pIda->pErrorInfo = ErrProp_AccessDenied;
              return( (MRESULT)-2);        // indicate error
            }
            if( (*pIda->pErrorInfo = PutItAway( pcntl)) != 0 )
              return( (MRESULT)-3);        // indicate error
          }
        if( --pcntl->usUser < 1)
          DropPropFile( pIda, pcntl);
        else
          if( hprop->lFlgs & PROP_ACCESS_WRITE)
            pcntl->lFlgs &= ~PROP_ACCESS_WRITE;
        FreePropHnd( pIda, hprop);
        return( *pIda->pErrorInfo ? (MRESULT)-4 : 0L);

/*--------------------------------------------------------------------------*/
      case WM_EQF_SAVEPROPERTIES:
//      mp1 = not used
//      mp2 = PPROPMSGPARM
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        pmsg = (PPROPMSGPARM) PVOIDFROMMP2(mp2);
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop=FindPropHnd( pIda->TopUsedHnd, (PPROPHND)  pmsg->hObject)) == NULL)
        {
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          return( (MRESULT)-1);        // indicate error
        }
        pcntl = hprop->pCntl;
        if( pcntl->lFlgs & PROP_STATUS_UPDATED)
        {
          if( !(hprop->lFlgs & PROP_ACCESS_WRITE))
          {
            *pIda->pErrorInfo = ErrProp_AccessDenied;
            return( (MRESULT)-2);        // indicate error
          }
          *pIda->pErrorInfo = PutItAway( pcntl);
        }
        return( *pIda->pErrorInfo ? (MRESULT)-3 : 0L);

/*--------------------------------------------------------------------------*/
      case WM_EQF_GETALLPROPERTIES:
//      mp1 = not used
//      mp2 = PPROPMSGPARM
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        pmsg = (PPROPMSGPARM) PVOIDFROMMP2(mp2);
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop=FindPropHnd( pIda->TopUsedHnd, (PPROPHND)  pmsg->hObject)) == NULL){
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          return( (MRESULT)-1);        // indicate error
        }
        if( !(hprop->lFlgs & PROP_ACCESS_READ)){
          *pIda->pErrorInfo = ErrProp_AccessDenied;
          return( (MRESULT)-2);        // indicate error
        }
        pcntl = hprop->pCntl;
        memcpy( pmsg->pBuffer, pcntl->pHead, pcntl->usFsize);
      return( 0L);

/*--------------------------------------------------------------------------*/
      case WM_EQF_PUTALLPROPERTIES:
//      mp1 = not used
//      mp2 = PPROPMSGPARM
        pIda = ACCESSWNDIDA( hwnd, PPROP_IDA );
        pmsg = (PPROPMSGPARM) PVOIDFROMMP2(mp2);
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop=FindPropHnd( pIda->TopUsedHnd, (PPROPHND)  pmsg->hObject)) == NULL)
        {
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          return( (MRESULT)-1);        // indicate error
        }
        if( !(hprop->lFlgs & PROP_ACCESS_WRITE))
        {
          *pIda->pErrorInfo = ErrProp_AccessDenied;
          return( (MRESULT)-2);        // indicate error
        }
        pcntl = hprop->pCntl;
        if( pmsg->pBuffer)
        {
          memcpy( pcntl->pHead, pmsg->pBuffer, pcntl->usFsize);
          hprop->pCntl->lFlgs |= PROP_STATUS_UPDATED;
        }
        return( 0L);

/*--------------------------------------------------------------------------*/
    }
    return( WinDefWindowProc( hwnd, message, mp1, mp2));
}

/*!
     Create a property handle
*/
PPROPHND MakePropHnd( PPROP_IDA pIda)
{
    int i;

    PPROPHND  ptr1, ptr2;              // Temp. ptr to handles
    if( !pIda->TopFreeHnd){
      i = sizeof( PLUG) + (PROPHWND_ENTRIES * sizeof( PROPHND));
      UtlAlloc( (PVOID *)&ptr1, 0L, (LONG)i, ERROR_STORAGE );
      if (!ptr1) {
        *pIda->pErrorInfo = Err_NoStorage;
        return( (PPROPHND) NULP);
      }
      memset( ptr1, NULC, i);
      UtlPlugIn( &ptr1->Plug, (PPLUG) NULP, pIda->ChainBlkHnd);
      if( !pIda->ChainBlkHnd)
        pIda->ChainBlkHnd = &ptr1->Plug;
      ptr1 = (PPROPHND)( (char *)ptr1 + sizeof( PLUG));
      pIda->TopFreeHnd = ptr1;
      for( ptr2= (PPROPHND) NULP,i= PROPHWND_ENTRIES-1; i; i--){
        UtlPlugIn( &ptr1->Plug, &ptr2->Plug,(PPLUG) NULP);
        ptr2 = ptr1;
        ptr1++;
      }
    }
    ptr1 = (PPROPHND)pIda->TopFreeHnd->Plug.Fw;
    ptr2 = (PPROPHND)UtlPlugOut( &pIda->TopFreeHnd->Plug);
    pIda->TopFreeHnd = ptr1;
    UtlPlugIn( &ptr2->Plug, (PPLUG) NULP, &pIda->TopUsedHnd->Plug);
    pIda->TopUsedHnd = ptr2;
//  pIda->TopUsedHnd->pHndID = pIda;   // for verification purposes
    return( ptr2);
}

/*!
     Release a property handle
*/
SHORT FreePropHnd( PPROP_IDA pIda, PPROPHND hprop)
{
    if( !hprop->Plug.Bw)               // is it the top most one ?
      pIda->TopUsedHnd = (PPROPHND)hprop->Plug.Fw;  // ..yes, set new anchor
    UtlPlugOut( &hprop->Plug);
    memset( hprop, NULC, sizeof( *hprop));
    UtlPlugIn( &hprop->Plug, (PPLUG) NULP, &pIda->TopFreeHnd->Plug);
    pIda->TopFreeHnd = hprop;          // set new anchor to free elements
    return( 0);
}

/*!
     Search for a property handle
*/
PPROPHND FindPropHnd( PPROPHND ptop, PPROPHND hprop)
{
//  nothing to be done in this release of EQF because hprop is the
//  requested pointer
//! check whether hprop is a valid handle
//  if( hprop->pHndID != testid)
//    return( NULP);
    return( hprop);
}

/*!
     Load the properties file
*/
PPROPCNTL LoadPropFile( PPROP_IDA pIda, PSZ pszName, PSZ pszPath, USHORT usAcc)
{
    PPROPCNTL     pcntl= (PPROPCNTL) NULP;          // Points to properties cntl buffer
    PROPHEAD      prophead;            // Properties heading area
    CHAR          fname[ MAX_EQF_PATH];// Temporary filename
    USHORT        sizeprop = 0;            // Size of properties area
    USHORT        size, sizread;       //
    HFILE         hf=NULLHANDLE;      // pointer to variable for file handle        */
    USHORT        usAction, usrc;
    BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

    *pIda->pErrorInfo = 0L;
    pcntl = FindPropCntl( pIda->TopCntl, pszName, pszPath);
    if( pcntl)
      if( (usAcc & PROP_ACCESS_WRITE) && (pcntl->lFlgs & PROP_ACCESS_WRITE)){
        *pIda->pErrorInfo = ErrProp_AccessDenied;
        return( (PPROPCNTL) NULP);
      } else {
        pcntl->usUser++;
        return( pcntl);
      }
    MakePropPath( fname, "", pszPath, pszName, ""); // no drive, no .ext
    do {
      usrc = UtlOpen( fname, &hf, &usAction, 0L,
                      FILE_NORMAL, FILE_OPEN,
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, 0);

      if( usrc){
        *pIda->pErrorInfo = Err_OpenFile;
        break;
      }
      usrc = UtlRead( hf, &prophead, sizeof( prophead), &sizread, 0);
      if( usrc || (sizread != sizeof( prophead))){
        *pIda->pErrorInfo = Err_ReadFile;
        break;
      }
      if( _stricmp( pszName, prophead.szName)
       || _stricmp( pszPath + 2, prophead.szPath + 2))   // ignore drive !!!
       {
        *pIda->pErrorInfo = ErrProp_InvalidFile;
        break;
      }
      if( (size = sizeprop = GetPropSize( prophead.usClass)) == 0)
      {
        *pIda->pErrorInfo = ErrProp_InvalidClass;
        break;
      }
      UtlAlloc( (PVOID *)&pcntl, 0L, (LONG)(size + sizeof( *pcntl)), ERROR_STORAGE );
      if( !pcntl){
        *pIda->pErrorInfo = Err_NoStorage;
        break;
      }
      size -= sizeof( prophead);       // sub bytes alread read
      memset( pcntl, NULC, sizeof( *pcntl));
      pcntl->pHead  = (PPROPHEAD)(pcntl+1);
      *pcntl->pHead = prophead;
      usrc = UtlRead( hf, pcntl->pHead+1, size, &sizread, 0);
      if( usrc || (sizread != size))
      {
        // for folder property files it is O.K. to read less than
        // size bytes
        if ( (prophead.usClass == PROP_CLASS_FOLDER) &&
             (sizread >= 2000) )
        {
          // continue ...
        }
        else if ( (prophead.usClass == PROP_CLASS_DOCUMENT) &&
             (sizread >= 2000) )
        {
          // may be a document property file created using a defect
          // OS/2 version which omitted the filler at the end of the
          // property structure
          // so continue ...
        }
        else
        {
          *pIda->pErrorInfo = Err_ReadFile;
          break;
        } /* endif */
      }
    } while( fTrueFalse /*TRUE & FALSE*/);
    if( hf)
      UtlClose( hf, 0);
    if( *pIda->pErrorInfo){
      UtlAlloc( (PVOID *)&pcntl, 0L, 0L, NOMSG );
      return( (PPROPCNTL) NULP);
    }
//  initialize control block
    UtlAlloc( (PVOID *)&pcntl->pszFname, 0L, (LONG) sizeof(OBJNAME), ERROR_STORAGE );
    strcpy( pcntl->pszFname, fname );
    pcntl->usFsize  = sizeprop;
    pcntl->usUser   = 1;
    pcntl->lFlgs    = usAcc & (PROP_ACCESS_READ | PROP_ACCESS_WRITE);
    UtlPlugIn( &pcntl->Plug, &pIda->LastCntl->Plug, (PPLUG) NULP);
    if( !pIda->TopCntl)
      pIda->TopCntl = pcntl;
    if( !pIda->LastCntl)
      pIda->LastCntl = pcntl;
    return( pcntl);
}

/*!
     Create the properties file
*/
PPROPCNTL CreatePropFile( PPROP_IDA pIda, PSZ pszName, PSZ pszPath,
                          USHORT usClass, BOOL fOverwriteExisting )
{
    PPROPCNTL     pcntl=(PPROPCNTL) NULP;          // Points to properties cntl buffer
    PPROPHEAD     pHead = NULL;        // Properties heading area
    CHAR          fname[ MAX_EQF_PATH];// Temporary filename
    USHORT        sizeprop = 0;            // Size of properties area
    USHORT        size = 0;
    USHORT        sizwrite = 0;         //
    HFILE         hf=NULLHANDLE;       // pointer to variable for file handle        */
    USHORT        usAction, usrc;
    BOOL          fExisted = FALSE;    // file-existed-flag

    *pIda->pErrorInfo = 0;             // everything is ok so dar ...

    if( FindPropCntl( pIda->TopCntl, pszName, pszPath))
    {
      *pIda->pErrorInfo = ErrProp_AlreadyExists;
    } /* endif */

    if ( !*pIda->pErrorInfo )
    {
       MakePropPath( fname, "", pszPath, pszName, ""); // no drive, no .ext

      if( (size = sizeprop = GetPropSize( usClass)) == 0)
      {
        *pIda->pErrorInfo = ErrProp_InvalidClass;
      } /* endif */
    } /* endif */

    if ( !*pIda->pErrorInfo )
    {
      size += sizeof( *pcntl);
      if ( fOverwriteExisting )
      {
        fExisted = FALSE;

        usrc = UtlOpen( fname, &hf, &usAction, (ULONG)sizeprop,
                        FILE_NORMAL, FILE_TRUNCATE,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, 0);
      }
      else
      {
        fExisted = UtlFileExist( fname );
        usrc = UtlOpen( fname, &hf, &usAction, (ULONG)sizeprop,
                        FILE_NORMAL, FILE_CREATE,
                        OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, 0);
      } /* endif */

      if ( usrc )
      {
         switch( usrc )
         {
            case ERROR_DISK_FULL:
               *pIda->pErrorInfo = Err_NoDiskSpace;
               break;
            default:
               *pIda->pErrorInfo = Err_OpenFile;
               break;
         } /* endswitch */
      } /* endif */
    } /* endif */

    if ( !*pIda->pErrorInfo )
    {
      if( fExisted )
      {
        *pIda->pErrorInfo = ErrProp_AlreadyExists;
      } /* endif */
    } /* endif */

    if ( !*pIda->pErrorInfo )
    {
      UtlAlloc( (PVOID *)&pcntl, 0L, (LONG)size, ERROR_STORAGE );
      if( !pcntl)
      {
        *pIda->pErrorInfo = Err_NoStorage;
      } /* endif */
    } /* endif */

    if ( !*pIda->pErrorInfo )
    {
      memset( pcntl, NULC, size);
      pHead = (PPROPHEAD)(pcntl+1);
      strcpy( pHead->szName, pszName);
      strcpy( pHead->szPath, pszPath);
      pHead->usClass = usClass;
      pHead->chType = PROP_TYPE_NEW;
      usrc = UtlWrite( hf, pHead, sizeprop, &sizwrite, 0);
      if ( usrc != NO_ERROR)
      {
         switch( usrc )
         {
            case ERROR_DISK_FULL:
               *pIda->pErrorInfo = Err_NoDiskSpace;
               break;
            default:
               *pIda->pErrorInfo = Err_WriteFile;
               break;
         } /* endswitch */
      }
      else if( sizwrite != sizeprop )
      {
        *pIda->pErrorInfo = Err_NoDiskSpace;
      } /* endif */
    } /* endif */

    if( hf)
    {
      UtlClose( hf, 0);
      if( *pIda->pErrorInfo &&
          (*pIda->pErrorInfo != ErrProp_AlreadyExists) &&
          !fExisted )
      {
        UtlDelete( fname, 0L, FALSE );
      } /* endif */
    } /* endif */

    if( *pIda->pErrorInfo )
    {
      if ( pcntl )
         UtlAlloc( (PVOID *)&pcntl, 0L, 0L, NOMSG );
      pcntl = NULL;
    }
    else
    {
       // initialize control block
       UtlAlloc( (PVOID *)&pcntl->pszFname, 0L, (LONG) sizeof(OBJNAME), ERROR_STORAGE );
       strcpy( pcntl->pszFname, fname );
       pcntl->usFsize  = sizeprop;
       pcntl->usUser   = 1;

       // set UPDATED flag to force a write back when user closes after creation
       pcntl->lFlgs    = PROP_ACCESS_READWRITE | PROP_STATUS_UPDATED;
       pcntl->pHead    = pHead;

       UtlPlugIn( &pcntl->Plug, &pIda->LastCntl->Plug, (PPLUG) NULP);
       if( !pIda->TopCntl)
         pIda->TopCntl = pcntl;
       if( !pIda->LastCntl)
         pIda->LastCntl = pcntl;
    } /* endif */

    return( pcntl);
}

/*!
     Read in system properties
*/
USHORT GetSysProp( PPROP_IDA pIda)
{
    PPROPHND      hprop;               //
    PPROPCNTL     pcntl = NULL;        // Points to properties cntl buffer
    USHORT        size, sizread;       // Size of properties area
    HFILE         hf=NULLHANDLE;       // pointer to variable for file handle        */
    USHORT        usAction, usrc;
    BOOL          error=TRUE;
    BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

    if( (hprop = MakePropHnd( pIda)) == NULL)
      return( TRUE );
    memset( hprop, NULC, sizeof( *hprop));
    do {
      if( UtlOpen( pIda->IdaHead.pszObjName, &hf, &usAction, 0L,
                   FILE_NORMAL, FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, 0))
        break;
      size = sizeof( PROPSYSTEM);
      UtlAlloc( (PVOID *)&pcntl, 0L, (LONG)(size + sizeof( *pcntl)), ERROR_STORAGE );
      if( !pcntl) break;
      memset( pcntl, NULC, sizeof( *pcntl));
      pcntl->pHead = (PPROPHEAD)(pcntl+1);
      usrc = UtlRead( hf, pcntl->pHead, size, &sizread, 0);
      if( usrc || (sizread != size)) break;
      error = FALSE;
    } while( fTrueFalse /*TRUE & FALSE*/);
    if( hf)
      UtlClose( hf, 0);
    if( error)
    {
      if ( pcntl )
      {
         UtlAlloc( (PVOID *)&pcntl, 0L, 0L, NOMSG );
      } /* endif */
      return( TRUE );
    }
//  initialize control block
    UtlAlloc( (PVOID *)&pcntl->pszFname, 0L, (LONG) sizeof(OBJNAME), ERROR_STORAGE );
    strcpy( pcntl->pszFname, pIda->IdaHead.pszObjName );
    pcntl->usFsize  = sizeof( PROPSYSTEM);
    pcntl->usUser = 1;
    pcntl->lFlgs  = PROP_ACCESS_READ;
    UtlPlugIn( &pcntl->Plug, &pIda->LastCntl->Plug, (PPLUG) NULP);
    if( !pIda->TopCntl)
      pIda->TopCntl = pcntl;
    if( !pIda->LastCntl)
      pIda->LastCntl = pcntl;

    hprop->pCntl = pcntl;
    hprop->lFlgs = PROP_ACCESS_READ;
    hprop->pCntl->usUser = 1;
    pIda->hSystem = hprop;             // anchor handle to system properties

    return( 0 );
}

/*!
     Reload system properties
*/
USHORT ReloadSysProp( PPROPSYSTEM pIda)
{
    USHORT        size, sizread;       // Size of properties area
    HFILE         hf=NULLHANDLE;       // pointer to variable for file handle        */
    USHORT        usAction, usrc;
    BOOL          error=TRUE;
    CHAR          fname[ MAX_EQF_PATH];// Temporary filename

    BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127
    do
    {
      MakePropPath( fname, "", pIda->PropHead.szPath, pIda->PropHead.szName, ""); // no drive, no .ext

      if( UtlOpen( fname, &hf, &usAction, 0L,
                 FILE_NORMAL, FILE_OPEN,
                 OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, 0))
        break;
      size = sizeof( PROPSYSTEM);
      usrc = UtlRead( hf, pIda, size, &sizread, 0);
      if( usrc || (sizread != size)) break;
      memcpy( (PBYTE)GetSystemPropPtr(), (PBYTE)pIda, size );
      error = FALSE;
    } while( fTrueFalse /*TRUE & FALSE*/);
    if( hf)
      UtlClose( hf, 0);

    return( 0 );
}



/*!
     PutItAway
*/
USHORT PutItAway( PPROPCNTL pcntl)
{
    USHORT        sizwrite;            // number of bytes written
    HFILE         hf=NULLHANDLE;       // pointer to variable for file handle        */
    USHORT        usAction, usrc;

    // always reset updated flag even if save fails, otherwise
    // the properties cannot be closed
    pcntl->lFlgs &= ~PROP_STATUS_UPDATED;

    if ( (usrc = UtlOpen( pcntl->pszFname, &hf, &usAction, 0L,
                    FILE_NORMAL, FILE_OPEN,
                    OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE,
                    0L, 0)) != NO_ERROR )
    {
	  return( usrc);
	}

    usrc = UtlWrite( hf, pcntl->pHead, pcntl->usFsize, &sizwrite, 0);

    if( !usrc && (sizwrite != pcntl->usFsize))
      usrc = Err_WriteFile;
    UtlClose( hf, 0);
    return( usrc);
}

/*!
     Drop properties file from memory
*/
VOID DropPropFile( PPROP_IDA pIda, PPROPCNTL pcntl)
{
    UtlAlloc( (PVOID *)&pcntl->pszFname, 0L, 0L, NOMSG );
    if( pcntl->Plug.Fw == NULP)
      pIda->LastCntl = (PPROPCNTL)pcntl->Plug.Bw;
    if( pcntl->Plug.Bw == NULP)
      pIda->TopCntl = (PPROPCNTL)pcntl->Plug.Fw;
    UtlPlugOut( &pcntl->Plug);
    UtlAlloc( (PVOID *)&pcntl, 0L, 0L, NOMSG );
}

/*!
     Delete a properties file
*/
SHORT DeletePropFile( PPROP_IDA pIda, PSZ pszName, PSZ pszPath)
{
    CHAR          fname[ MAX_EQF_PATH];   // Temporary filename
    USHORT        usrc;

    MakePropPath( fname, "", pszPath, pszName, ""); // no drive, no .ext
    if( (usrc = UtlDelete( fname, 0L, 0)) != NO_ERROR )
      *pIda->pErrorInfo = Err_DeleteFile;
    return( usrc);
}

/*!
     Find Properties Control block
*/
PPROPCNTL FindPropCntl( PPROPCNTL ptop, PSZ pszName, PSZ pszPath)
{
    for(; ptop; ptop=(PPROPCNTL)ptop->Plug.Fw){
      if( !strcmp( ptop->pHead->szName, pszName)
       && !strcmp( ptop->pHead->szPath, pszPath))
        return( ptop);
    }
    return( (PPROPCNTL) NULP);
}

/*!
     Load Properties Message area
*/
SHORT LoadPropMsg( PPROPMSGPARM pm, PPROPHND hprop, PSZ name, PSZ path, \
                    USHORT flg, PEQFINFO pErrorInfo, BYTE *buffer)
{
    pm->hObject = hprop;
    pm->pBuffer = buffer;
    pm->fFlg    = flg;
    pm->pErrorInfo = pErrorInfo;
    if( name || path)
      if( !path){
        strcpy( (PSZ)(pm->tmpName), name);
        if( ( pm->pszName = UtlSplitFnameFromPath((PSZ) pm->tmpName)) == NULL)
          return( TRUE );
        pm->pszPath = (PSZ)(pm->tmpName);
      } else {
        pm->pszName = name;
        pm->pszPath = path;
      }
    else {
        pm->pszName = (PSZ) NULP;
        pm->pszPath = (PSZ) NULP;
    }
    return( 0);              // no checks included now
}

/*!
     Notify All
*/
VOID NotifyAll( HPROP hprop)
{
    PPROPHEAD ph;
    char name[ MAX_EQF_PATH];

    if ( UtlQueryUShort( QS_RUNMODE ) != FUNCCALL_RUNMODE )
    {
      ph = (PPROPHEAD)(((PPROPHND)hprop)->pCntl->pHead);
      strcat( strcat( strcpy( name, ph->szPath), "\\"), ph->szName);
      EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                           MP1FROMSHORT( ph->usClass ),
                           MP2FROMP(name) );
    } /* endif */
}

/*!
    Open Properties

*/
HPROP OpenProperties( PSZ pszObjName, PSZ pszPath, USHORT usAccess,
                               PEQFINFO pErrorInfo)
{
   HPROP     hprop=NULL;
   PPROPMSGPARM pmsg = NULL;

   UtlAlloc( (PVOID *)&pmsg, 0L, (LONG)sizeof(PROPMSGPARM), ERROR_STORAGE );
   if ( !pmsg )
   {
      *pErrorInfo = Err_NoStorage;
      return( NULL );
   } /* endif */

   if( !pszObjName)
   {
     *pErrorInfo = ErrProp_InvalidObjectName;
   }
   else if( usAccess & ~PROP_ACCESS_READWRITE)
   {
     *pErrorInfo = ErrProp_InvalidAccess;
   }
   else if( LoadPropMsg( pmsg, NULL, pszObjName, pszPath, usAccess,
                         pErrorInfo, NULL ) )
   {
     *pErrorInfo = ErrProp_InvalidParms;
   }
   else
   {
     if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPHND      hProp;
        HANDLE hMutexSem = NULL;

        // keep other process from doing property related stuff..
        GETMUTEX(hMutexSem);

        pIda = pPropBatchIda;
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return

        if( (hProp = MakePropHnd( pIda)) != NULL)
        {
          hProp->pCntl = LoadPropFile( pIda, pmsg->pszName, pmsg->pszPath, pmsg->fFlg);
          if( !hProp->pCntl )
          {
            FreePropHnd( pIda, hProp );
            hProp = NULL;
          } /* endif */
        } /* endif */

        if ( hProp != NULL )
        {
          hProp->lFlgs |= pmsg->fFlg;
          hprop = (HPROP)hProp;
        } /* endif */

        // release Mutex
        RELEASEMUTEX(hMutexSem);
     }
     else
     {
     hprop = (HPROP)EqfCallPropertyHandler( WM_EQF_OPENPROPERTIES,
                                            MP1FROMSHORT(0),
                                            MP2FROMP(pmsg) );
     } /* endif */
   } /* endif */
   UtlAlloc( (PVOID *)&pmsg, 0L, 0L, NOMSG );

   return( hprop);
}

/*!
    Create Properties
*/
HPROP CreateProperties(
   PSZ       pszObjName,              // Name of properties        * input
   PSZ       pszPath,                 // Full path to properties   * input
   USHORT    usClass,                 // Class of properties       * input
   PEQFINFO  pErrorInfo               // Error indicator           * output
)
{
  return( CreatePropertiesEx( pszObjName, pszPath, usClass, pErrorInfo, FALSE ) );
}

HPROP CreatePropertiesEx(
   PSZ       pszObjName,              // Name of properties        * input
   PSZ       pszPath,                 // Full path to properties   * input
   USHORT    usClass,                 // Class of properties       * input
   PEQFINFO  pErrorInfo,              // Error indicator           * output
   BOOL      fOverwriteExisting       // TRUE = overwrite any existing property file
)
{
   HPROP  hprop=NULL;
   PPROPMSGPARM pmsg;
   BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

   UtlAlloc( (PVOID *)&pmsg, 0L, (LONG)sizeof( *pmsg), ERROR_STORAGE );
   if ( !pmsg )
   {
      *pErrorInfo = Err_NoStorage;
      return( NULL );
   } /* endif */
   do {
     if( !pszObjName){
       *pErrorInfo = ErrProp_InvalidObjectName;
       break;
     }
     if( !GetPropSize( usClass)){
       *pErrorInfo = ErrProp_InvalidClass;
       break;
     }
     if( LoadPropMsg( pmsg, NULL, pszObjName, pszPath, usClass,
                      pErrorInfo,  NULL))
     {
       *pErrorInfo = ErrProp_InvalidParms;
       break;
     }

     if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPHND      hProp = NULL;
        HANDLE hMutexSem = NULL;

        // keep other process from doing property related stuff..
        GETMUTEX(hMutexSem);

        pIda = pPropBatchIda;
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return

        hProp = MakePropHnd( pIda);

        if ( hProp )
        {
          hProp->pCntl = CreatePropFile( pIda, pmsg->pszName, pmsg->pszPath,
                                       pmsg->fFlg, fOverwriteExisting );

          if( !hProp->pCntl)
          {
            FreePropHnd( pIda, hProp);
            hProp = NULL;
          }
        } /* endif */

        if ( hProp )
        {
          hProp->lFlgs = PROP_ACCESS_READWRITE;
          hprop = (HPROP)hProp;
        } /* endif */

        // release Mutex
        RELEASEMUTEX(hMutexSem);
     }
     else
     {
       pmsg->fOverWrite = fOverwriteExisting;
       hprop = (HPROP)EqfCallPropertyHandler( WM_EQF_CREATEPROPERTIES,
                                              MP1FROMSHORT(0),
                                              MP2FROMP(pmsg) );
     } /* endif */
   } while( fTrueFalse /*TRUE & FALSE*/);
   UtlAlloc( (PVOID *)&pmsg, 0L, 0L, NOMSG );
   return( hprop);
}

/*!
    Delete Properties
*/
SHORT DeleteProperties(
   PSZ       pszObjName,              // Name of properties        * input
   PSZ       pszPath,                 // Full path to properties   * input
   PEQFINFO  pErrorInfo               // Error indicator           * output
)
{
   PROPMSGPARM PropMsg;                // buffer for message structure

   SHORT rc = 0;
   BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

   do {
     if( !pszObjName)
     {
       *pErrorInfo = ErrProp_InvalidObjectName;
       break;
     }

     if( LoadPropMsg( &PropMsg, NULL, pszObjName, pszPath, 0,
                      pErrorInfo, NULL))
     {
       *pErrorInfo = ErrProp_InvalidParms;
       break;
     }
     if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPCNTL     pcntl;
        HANDLE hMutexSem = NULL;

        // keep other process from doing property related stuff..
        GETMUTEX(hMutexSem);

        pIda = pPropBatchIda;
        pIda->pErrorInfo = PropMsg.pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        pcntl = FindPropCntl( pIda->TopCntl, PropMsg.pszName,
                              PropMsg.pszPath);
        if( pcntl)
        {
          *pIda->pErrorInfo = ErrProp_ObjectBusy;
          rc = -1;        // indicate error
        }
        else
        {
          rc = DeletePropFile( pIda, PropMsg.pszName, PropMsg.pszPath);
        }

        // release Mutex
        RELEASEMUTEX(hMutexSem);
     }
     else
     {
       rc = SHORT1FROMMR(EqfCallPropertyHandler( WM_EQF_DELETEPROPERTIES,
                                                 MP1FROMSHORT(0),
                                                 MP2FROMP(&PropMsg) ) );
     } /* endif */
   } while( fTrueFalse /*TRUE & FALSE*/);

   return( rc);
}

/*!
    Close Properties
*/
SHORT CloseProperties(
   HPROP     hObject,                 // Handle to object properties *input
   USHORT    fClose,                  // Flags for closeing          *input
   PEQFINFO  pErrorInfo               // Error indicator           * output
)
{
   PROPMSGPARM PropMsg;                // buffer for message structure

   SHORT rc = TRUE;
   BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

   do {
     if( !hObject)
     {
       *pErrorInfo = ErrProp_InvalidHandle;
       break;
     }

     if( LoadPropMsg( &PropMsg, (PPROPHND) hObject, NULL, NULL, fClose,
                      pErrorInfo, NULL))
     {
       *pErrorInfo = ErrProp_InvalidParms;
       break;
     }
     if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPCNTL     pcntl;
        PPROPHND      hProp = NULL;
        HANDLE hMutexSem = NULL;

        // keep other process from doing property related stuff..
        GETMUTEX(hMutexSem);

        pIda = pPropBatchIda;
        pIda->pErrorInfo = PropMsg.pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hProp=FindPropHnd( pIda->TopUsedHnd, (PPROPHND) PropMsg.hObject)) == NULL)
        {
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          rc = -1;        // indicate error
        }
        else
        {
          pcntl = hProp->pCntl;
          if( pcntl->lFlgs & PROP_STATUS_UPDATED)
            if( PropMsg.fFlg & PROP_FILE)
            {
              if( !(hProp->lFlgs & PROP_ACCESS_WRITE))
              {
                *pIda->pErrorInfo = ErrProp_AccessDenied;
                rc =  -2;        // indicate error
              }
              else if( (*pIda->pErrorInfo = PutItAway( pcntl)) != 0 )
              {
                rc = -3;        // indicate error
              }
            }
          if ( rc == TRUE )
          {
            if( --pcntl->usUser < 1)
              DropPropFile( pIda, pcntl);
            else
              if( hProp->lFlgs & PROP_ACCESS_WRITE)
                pcntl->lFlgs &= ~PROP_ACCESS_WRITE;
            FreePropHnd( pIda, hProp);
            rc = *pIda->pErrorInfo ? -4 : 0;
          } /* endif */
        } /* endif */

        // release Mutex
        RELEASEMUTEX(hMutexSem);
     }
     else
     {
       rc = SHORT1FROMMR(EqfCallPropertyHandler( WM_EQF_CLOSEPROPERTIES,
                                                 MP1FROMSHORT(0),
                                                 MP2FROMP(&PropMsg) ) );
     } /* endif */

   } while( fTrueFalse /*TRUE & FALSE*/);

   return( rc);
}

/*!
    Get All Properties
*/
SHORT GetAllProperties(
   HPROP     hObject,                 // Handle to object properties *input
   PVOID     pBuffer,                 // Buffer to be loaded       * output
   PEQFINFO  pErrorInfo               // Error indicator           * output
)
{
   PPROPMSGPARM   pmsg;
   SHORT          rc = TRUE;
   BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

   UtlAlloc( (PVOID *)&pmsg, 0L, (LONG)sizeof( *pmsg), ERROR_STORAGE );
   if ( !pmsg )
   {
      *pErrorInfo = Err_NoStorage;
      return( rc );
   } /* endif */
   do {
     if( !hObject){
       *pErrorInfo = ErrProp_InvalidHandle;
       break;
     }
     if( !pBuffer){
       *pErrorInfo = ErrProp_MissingBuffer;
       break;
     }
     if( LoadPropMsg( pmsg,(PPROPHND) hObject, NULL, NULL, 0,
                      pErrorInfo, (BYTE *) pBuffer)){
       *pErrorInfo = ErrProp_InvalidParms;
       break;
     }
     if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPCNTL     pcntl;
        PPROPHND      hprop;
        HANDLE        hMutexSem = NULL;

        // keep other process from doing property related stuff..
        GETMUTEX(hMutexSem);

        pIda = pPropBatchIda;
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop=FindPropHnd( pIda->TopUsedHnd, (PPROPHND)  pmsg->hObject)) == NULL)
        {
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          rc = -1;        // indicate error
        }
        else if( !(hprop->lFlgs & PROP_ACCESS_READ))
        {
          *pIda->pErrorInfo = ErrProp_AccessDenied;
          rc = -2;        // indicate error
        }
        else
        {
          pcntl = hprop->pCntl;
          memcpy( pmsg->pBuffer, pcntl->pHead, pcntl->usFsize);
        }
        RELEASEMUTEX(hMutexSem);
     }
     else
     {
       rc = SHORT1FROMMR(EqfCallPropertyHandler( WM_EQF_GETALLPROPERTIES,
                                           MP1FROMSHORT(0),
                                           MP2FROMP(pmsg) ) );
     } /* endif */
   } while( fTrueFalse /*TRUE & FALSE*/);
   UtlAlloc( (PVOID *)&pmsg, 0L, 0L, NOMSG );
   return( rc);
}

/*!
    Put All Properties
*/
SHORT PutAllProperties(
   HPROP     hObject,                 // Handle to object properties *input
   PVOID     pBuffer,                 // Buffer with properties    * input
   PEQFINFO  pErrorInfo               // Error indicator           * output
)
{
   PROPMSGPARM PropMsg;                // buffer for message structure

   SHORT rc = TRUE;
   BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

   do {
     if( !hObject)
     {
       *pErrorInfo = ErrProp_InvalidHandle;
       break;
     }
     if( LoadPropMsg( &PropMsg,(PPROPHND) hObject, NULL, NULL, 0,
                      pErrorInfo, (BYTE *) pBuffer))
     {
       *pErrorInfo = ErrProp_InvalidParms;
       break;
     }
     if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPCNTL     pcntl;
        PPROPHND      hprop;
        HANDLE        hMutexSem = NULL;

        GETMUTEX(hMutexSem);
        pIda = pPropBatchIda;
        pIda->pErrorInfo = PropMsg.pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop=FindPropHnd( pIda->TopUsedHnd, (PPROPHND) PropMsg.hObject)) == NULL)
        {
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          rc = -1;        // indicate error
        }
        else if( !(hprop->lFlgs & PROP_ACCESS_WRITE))
        {
          *pIda->pErrorInfo = ErrProp_AccessDenied;
          rc = -2;        // indicate error
        }
        else
        {
          pcntl = hprop->pCntl;
          if( PropMsg.pBuffer)
          {
            memcpy( pcntl->pHead, PropMsg.pBuffer, pcntl->usFsize);
            hprop->pCntl->lFlgs |= PROP_STATUS_UPDATED;
            rc = 0;
          }
        } /* endif */
        RELEASEMUTEX(hMutexSem);
     }
     else
     {
       rc = SHORT1FROMMR(EqfCallPropertyHandler( WM_EQF_PUTALLPROPERTIES,
                                           MP1FROMSHORT(0),
                                           MP2FROMP(&PropMsg) ) );
     } /* endif */
   } while( fTrueFalse /*TRUE & FALSE*/);

   if( !rc && (UtlQueryUShort( QS_RUNMODE ) != FUNCCALL_RUNMODE) )
   {
      NotifyAll( hObject);
   } /* endif */
   return( rc);
}


/*!
    Save Properties
*/
SHORT SaveProperties(
   HPROP     hObject,                 // Handle to object properties *input
   PEQFINFO  pErrorInfo               // Error indicator           * output
)
{
   PROPMSGPARM PropMsg;                // buffer for message structure

   SHORT rc = 0;
   BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127
   do {
     if( !hObject)
     {
       *pErrorInfo = ErrProp_InvalidHandle;
       break;
     }

     if( LoadPropMsg( &PropMsg, (PPROPHND) hObject, NULL, NULL, 0,
                      pErrorInfo, NULL))
     {
       *pErrorInfo = ErrProp_InvalidParms;
       break;
     }
     if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPCNTL     pcntl;
        PPROPHND      hprop;
        HANDLE hMutexSem = NULL;

        // keep other process from doing property related stuff..
        GETMUTEX(hMutexSem);

        pIda = pPropBatchIda;
        pIda->pErrorInfo = PropMsg.pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop=FindPropHnd( pIda->TopUsedHnd, (PPROPHND) PropMsg.hObject)) == NULL)
        {
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          rc = -1;
        }
        else
        {
          pcntl = hprop->pCntl;
          if( pcntl->lFlgs & PROP_STATUS_UPDATED)
          {
            if( !(hprop->lFlgs & PROP_ACCESS_WRITE))
            {
              *pIda->pErrorInfo = ErrProp_AccessDenied;
              rc = -2;
            }
            else
            {
              *pIda->pErrorInfo = PutItAway( pcntl);
            } /* endif */
          }
          if ( !rc ) rc = *pIda->pErrorInfo ? -3 : 0;
        } /* endif */

        // release Mutex
       RELEASEMUTEX(hMutexSem);
     }
     else
     {
       rc = SHORT1FROMMR( EqfCallPropertyHandler( WM_EQF_SAVEPROPERTIES,
                                           MP1FROMSHORT(0),
                                           MP2FROMP(&PropMsg) ) );
     } /* endif */
 } while( fTrueFalse /*TRUE & FALSE*/);

   return( rc);
}

/*!
    Miscellaneous properties functions
*/
PVOID MakePropPtrFromHnd( HPROP hprop)
{
     return( hprop ? (PVOID)(((PPROPHND)hprop)->pCntl->pHead) : NULP);
}
PVOID MakePropPtrFromHwnd( HWND hObject)
{
   PIDA_HEAD pIda = ACCESSWNDIDA( hObject, PIDA_HEAD );
    return( pIda ? MakePropPtrFromHnd( pIda->hProp) : NULL);
}
PPROPSYSTEM GetSystemPropPtr( VOID )
{
    HPROP hSysProp;
    if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
    {
      PPROP_IDA     pIda;           // Points to instance area
      pIda = pPropBatchIda;
      hSysProp = pIda->hSystem;

    }
    else
    {
      hSysProp = EqfQuerySystemPropHnd();
      assert( hSysProp != NULL );
    } /* endif */
    return( (PPROPSYSTEM)( MakePropPtrFromHnd( hSysProp )));
 }
BOOL SetPropAccess( HPROP hprop, USHORT flgs)
{
    if( flgs & PROP_ACCESS_WRITE){
      if( ((PPROPHND)hprop)->lFlgs & (ULONG)PROP_ACCESS_WRITE)
       return( FALSE);
      if( ((PPROPHND)hprop)->pCntl->lFlgs & (ULONG)PROP_ACCESS_WRITE)
       return( FALSE);
      ((PPROPHND)hprop)->pCntl->lFlgs |= (ULONG)(PROP_ACCESS_WRITE | \
                                                 PROP_STATUS_UPDATED);
      ((PPROPHND)hprop)->lFlgs |= (ULONG)PROP_ACCESS_WRITE;
    }
    return( TRUE);
}
VOID ResetPropAccess( HPROP hprop, USHORT flgs)
{
    ((PPROPHND)hprop)->pCntl->lFlgs &= (ULONG)~flgs;
    ((PPROPHND)hprop)->lFlgs &= (ULONG)~flgs;
    if( flgs & PROP_ACCESS_WRITE) NotifyAll( hprop);
}

/*!
     GetPropSize - return size of properties given by its class
*/
SHORT GetPropSize( USHORT usClass)
{
   USHORT usSize;                      // size of properties

   switch ( usClass )
   {
      case PROP_CLASS_SYSTEM :
         usSize = sizeof( PROPSYSTEM );
         break;
      case PROP_CLASS_FOLDERLIST :
         usSize = sizeof( PROPFOLDERLIST );
         break;
      case PROP_CLASS_FOLDER :
         usSize = sizeof( PROPFOLDER );
         break;
      case PROP_CLASS_DOCUMENT :
         usSize = sizeof( PROPDOCUMENT );
         break;
      case PROP_CLASS_IMEX :
         usSize = sizeof( PROPIMEX );
         break;
      case PROP_CLASS_EDITOR :
         usSize = sizeof( PROPEDIT );
         break;
      case PROP_CLASS_DICTIONARY:
         usSize = sizeof( PROPDICTIONARY );
         break;
      case PROP_CLASS_DICTLIST:
         usSize = sizeof( PROPDICTLIST );
         break;
      case PROP_CLASS_TAGTABLE:
         usSize = sizeof( PROPTAGTABLE );
         break;
      case PROP_CLASS_LIST:
         usSize = sizeof( PROPLIST );
         break;
      case PROP_CLASS_MEMORY :
      case PROP_CLASS_MEMORYDB :
      case PROP_CLASS_MEMORY_LASTUSED :
         if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
         {
           usSize = 2048; // MEM_PROP_SIZE;
         }
         else
         {
           usSize = (USHORT) EqfSend2Handler( MEMORYHANDLER, WM_EQF_QUERYPROPSIZE, MP1FROMSHORT(usClass), 0L);
         } /* endif */
         break;
      case PROP_CLASS_TQMLIST :
         usSize = (USHORT) EqfSend2Handler( TQMLISTHANDLER, WM_EQF_QUERYPROPSIZE, MP1FROMSHORT(usClass), 0L);
         break;
      default :
         usSize = 0;
         break;
   } /* endswitch */
    return( usSize );
}

/*!
     MakePropPath
*/
PSZ MakePropPath( PSZ pbuf, PSZ pd, PSZ pp, PSZ pn, PSZ pe)
{
    PPROPSYSTEM pprop;
    PPROPHND    hprop;
    CHAR        tmppath[ MAX_EQF_PATH];
    *pbuf = NULC;
    if( (hprop = (PPROPHND) EqfQuerySystemPropHnd())== NULL)
      return( pbuf);
    pprop = (PPROPSYSTEM) MakePropPtrFromHnd( hprop);
    sprintf( tmppath, "%s\\%s", pp, pprop->szPropertyPath );
    _makepath( pbuf, pd, tmppath, pn, pe);
    return( pbuf);
}

// Function PropHandlerInitForBatch
// Initialize the property handler for non-windows environments;
// i.e. perform WM_CREATE handling to allocate our IDA
BOOL PropHandlerInitForBatch( void )
{
  int size;
  BOOL fOK = TRUE;

  if( !UtlAlloc( (PVOID *)&pPropBatchIda, 0L, (LONG)sizeof( *pPropBatchIda), ERROR_STORAGE ) )
    return( FALSE );      // do not create the window
  memset( pPropBatchIda, NULC, sizeof( *pPropBatchIda));
  size = sizeof(pPropBatchIda->IdaHead.szObjName);
  pPropBatchIda->IdaHead.pszObjName = pPropBatchIda->IdaHead.szObjName;
  GetStringFromRegistry( APPL_Name, KEY_SysProp, pPropBatchIda->IdaHead.pszObjName, size, "" );

  { // keep other process from doing property related stuff..
	  HANDLE hMutexSem = NULL;

      GETMUTEX(hMutexSem);

      if( GetSysProp( pPropBatchIda))
        fOK = FALSE;
    //   return( FALSE);      // do not create the window

       RELEASEMUTEX(hMutexSem);     // release Mutex
   }

 return( fOK );
} /* end of function PropHandlerInitForBatch */

// Function PropHandlerTerminateForBatch
// Terminate the property handler in non-windows environments;
BOOL
PropHandlerTerminateForBatch( void )
{// keep other process from doing property related stuff..
  HANDLE hMutexSem = NULL;
  PPROP_IDA pIda = pPropBatchIda;

  GETMUTEX(hMutexSem);
  assert( pIda->hSystem != NULL );

  if ( pIda->hSystem != NULL )
  {
    if( pIda->hSystem->pCntl->lFlgs & PROP_STATUS_UPDATED)
    {
      PutItAway( pIda->hSystem->pCntl);
    }
  } /* endif */

  RELEASEMUTEX(hMutexSem);     // release Mutex
  return( TRUE );
} /* end of function PropHandlerTerminateForBatch */

HPROP EqfQuerySystemPropHnd( void )
{
  HPROP hprop;

  if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
  {
    PPROP_IDA pIda = pPropBatchIda;
    hprop = pIda->hSystem;
  }
  else
  {
//    hprop = (HPROP)WinSendMsg( EqfQueryHandler( PROPERTYHANDLER),
//                                        WM_EQF_QUERYSYSTEMPROPHND,
//                                        NULL, NULL);
      if (hwndPropertyHandler)
      {
      	PPROP_IDA pIda = ACCESSWNDIDA( hwndPropertyHandler, PPROP_IDA );
      	hprop = (HPROP)pIda->hSystem;
	  }
	  else
	  {
		hprop = NULL;
	  }
  } /* endif */
  return( hprop );
} /* end of function EqfQuerySystemPropHnd */
