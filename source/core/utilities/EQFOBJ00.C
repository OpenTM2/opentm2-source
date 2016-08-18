/*!
   EQFOBJ00.C - EQF Object Manager
*/
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#include "eqf.h"                  // General .H for EQF
#include "eqfobj00.h"             // Object manager defines

static HWND hObjMan[MAX_TASK] = {NULLHANDLE};
POBJM_IDA     pObjBatchIda = NULL;     // Points to IDA when in function call mode


////////////////////////////////////////////////////////////////////
// defines for symbol table in shared memory area in nonDDE mode  //
////////////////////////////////////////////////////////////////////

// name of shared memory table with locked objects
#define EQFLOCKOBJ_SHMEM     "/SHAREMEM/OTM/OTMLOCKOBJ"

// element of locked objects
typedef struct _FUNCIF_LOCK_ENTRY
{
  CHAR szLockObj[MAX_EQF_PATH];        // name of locked object
  DWORD dwProcessID;                   // ID of process locking the object
} FUNCIF_LOCK_ENTRY, *PFUNCIF_LOCK_ENTRY;

// structure of shared memory area
typedef struct _FUNCIF_LOCK_TABLE
{
  BOOL fUpdateInProgress;              // TRUE = table is currently updated
  int  iReadAccess;                    // number of processes currently reading table
  int  iEntries;                       // number of entries in table
  int  iFiller1;                       // currently not used
  int  iFiller2;                       // currently not used
  int  iFiller3;                       // currently not used
  int  iFiller4;                       // currently not used
  int  iFiller5;                       // currently not used
  int  iFiller6;                       // currently not used
  int  iFiller7;                       // currently not used
  int  iFiller8;                       // currently not used
  CHAR chFiller9[128];                 // currently not used
  FUNCIF_LOCK_ENTRY aLock[SYMBOLENTRIES]; // table of currently locked ojects
} FUNCIF_LOCK_TABLE, *PFUNCIF_LOCK_TABLE;

// handle of shared memory object
static HANDLE hSharedMem = NULLHANDLE;

// utility functions for lock table handling
HANDLE ObjLockTable_CreateOrOpenTable();
PFUNCIF_LOCK_TABLE ObjLockTable_AccessTable( HANDLE );
void ObjLockTable_ReleaseTable( PFUNCIF_LOCK_TABLE );
BOOL ObjLockTable_SetUpdateFlag( PFUNCIF_LOCK_TABLE );
BOOL ObjLockTable_ClearUpdateFlag( PFUNCIF_LOCK_TABLE );
PFUNCIF_LOCK_ENTRY ObjLockTable_Search( PFUNCIF_LOCK_TABLE, PSZ, BOOL );
BOOL ObjLockTable_Delete( PFUNCIF_LOCK_TABLE, PSZ, DWORD );
BOOL ObjLockTable_Add( PFUNCIF_LOCK_TABLE, PSZ, DWORD );
BOOL ObjLockTable_WaitWhenUpdated( PFUNCIF_LOCK_TABLE pLockTable );

HWND EqfQueryObjectManager( VOID)
{
    return( hObjMan[UtlGetTask()]);
}
USHORT Send2AllHandlers( WINMSG msg, WPARAM mp1, LPARAM mp2)
{
    POBJM_IDA     pIda;                // Points to instance data area
    USHORT        usResult;             //
    pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
    if (pIda != NULL  )                                                   /*@02A*/
    {                                                                     /*@02A*/
      usResult = SendAll( pIda->pHndlrTbl, clsANY, msg, mp1, mp2);
    }                                                                     /*@02A*/
    else                                                                  /*@02A*/
    {                                                                     /*@02A*/
      usResult = FALSE;                                                   /*@02A*/
    } /* endif */                                                         /*@02A*/
    return( usResult);                                                    /*@02A*/
}
USHORT Send2AllObjects( USHORT cls, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
    POBJM_IDA     pIda;                // Points to instance data area
    pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
    return( SendAll( pIda->pObjTbl, (CLASSES) cls, msg, mp1, mp2));
}
USHORT RegisterObject( PSZ name, HWND hwnd, USHORT cls)
{
    POBJM_IDA     pIda;                // Our Ida
    EQF_ATOM      atom;
    POBJENTRY     pEntry;
    pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
    if( (atom = WinFindAtom( pIda->hAtomTbl, name)) != NULLHANDLE )
      return( ErrObjM_AlreadyReg);
//  if( !FindObjByClass( pIda->pHndlrTbl, cls))
//    return( ErrObjM_NoHandler);
    if ( (cls != clsANY) && (cls != clsNONE) )
    {
       if( (pEntry = MakeObjEntry( pIda->pObjTbl)) == NULL)
         return( ErrObjM_CreateEntry);
       pEntry->aName = WinAddAtom( pIda->hAtomTbl, name);
       pEntry->hWnd = hwnd;
       pEntry->usClassID = cls;
       pEntry->fStatus   = OBJ_MANAGER;
    } /* endif */
    return( 0 );
}
USHORT InstallHandler( PSZ name, HWND hwnd, USHORT cls)
{
    POBJM_IDA     pIda;                // Our Ida
    EQF_ATOM      atom;
    POBJENTRY     pEntry;
    pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
    if( (atom = WinFindAtom( pIda->hAtomTbl, name)) != NULLHANDLE )
      return( ErrObjM_AlreadyReg);
    if( (pEntry = MakeObjEntry( pIda->pHndlrTbl))== NULL)
      return( ErrObjM_CreateEntry);
    pEntry->aName = WinAddAtom( pIda->hAtomTbl, name);
    pEntry->hWnd = hwnd;
    pEntry->usClassID = cls;
    return( 0 );
}

/*ÛßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßÛ
  Û  Object Manager Window Proc                                              Û
  Û                                                                          Û
  ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß
*/
MRESULT APIENTRY OBJECTMANAGERWP
( HWND hwnd, WINMSG message, WPARAM mp1, LPARAM mp2)
{
    POBJM_IDA     pIda;                // Points to instance data area
    BOOL          fNotClosed;
    POBJENTRY     pEntry;
    USHORT        us;
    EQF_ATOM      atom;
    HAB           hab;                 // our Anchor Block handle
    int           i;

    switch( message) {
/*--------------------------------------------------------------------------*/
      case WM_CREATE:
        hObjMan[UtlGetTask()] = NULLHANDLE;      // we are not ready yet
        if( !UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof( *pIda), ERROR_STORAGE ) )
          return( (MRESULT)TRUE);      // do not create the window
        memset( pIda, 0, sizeof( *pIda));
        ANCHORWNDIDA( hwnd, pIda);
        strcpy( pIda->IdaHead.szObjName, OBJECTMANAGER );
        pIda->IdaHead.pszObjName = pIda->IdaHead.szObjName;

        us = sizeof( OBJTBL) + HANDLERENTRIES * sizeof( OBJENTRY);
        UtlAlloc( (PVOID *)&pIda->pHndlrTbl, 0L, (LONG)us, ERROR_STORAGE );
        memset( pIda->pHndlrTbl, 0, us);
        pIda->pHndlrTbl->usUsed = 0;
        pIda->pHndlrTbl->usMax  = HANDLERENTRIES;
//      pIda->pHndlrTbl->pObjEntry = (PpIda->pHndlrTbl+ sizeof( OBJTBL);
        pIda->pHndlrTbl->pObjEntry =
           (POBJENTRY) ( (PBYTE)pIda->pHndlrTbl+ sizeof( OBJTBL) );

        us = sizeof( OBJTBL) + OBJECTENTRIES * sizeof( OBJENTRY);
        UtlAlloc( (PVOID *)&pIda->pObjTbl, 0L, (LONG)us, ERROR_STORAGE );
        memset( pIda->pObjTbl, 0, us);
        pIda->pObjTbl->usUsed = 0;
        pIda->pObjTbl->usMax  = OBJECTENTRIES;
//      pIda->pObjTbl->pObjEntry = (POBJENTRY)pIda->pObjTbl+ sizeof( OBJTBL);
        pIda->pObjTbl->pObjEntry =
           (POBJENTRY)( (PBYTE)pIda->pObjTbl+ sizeof( OBJTBL) );

        us = sizeof( OBJTBL) + SYMBOLENTRIES * sizeof( OBJENTRY);
        UtlAlloc( (PVOID *)&pIda->pSymbolTbl, 0L, (LONG)us, ERROR_STORAGE );
        memset( pIda->pSymbolTbl, 0, us);
        pIda->pSymbolTbl->usUsed = 0;
        pIda->pSymbolTbl->usMax  = SYMBOLENTRIES;
//      pIda->pSymbolTbl->pObjEntry = (POBJENTRY)pIda->pSymbolTbl+ sizeof( OBJTBL);
        pIda->pSymbolTbl->pObjEntry =
           (POBJENTRY) ( (PBYTE)pIda->pSymbolTbl + sizeof( OBJTBL) );

        pIda->hAtomTbl = WinCreateAtomTable( 0, 0 );
        pIda->hAtomSymTbl = WinCreateAtomTable( 0, 0 );
        hObjMan[UtlGetTask()] = hwnd;                // now we are ready for work
        return( MRFROMSHORT(0) );                 // continue with default proc

/*--------------------------------------------------------------------------*/
      case WM_CLOSE:
        WinDestroyWindow( hwnd);
        return( MRFROMSHORT(0) );

/*--------------------------------------------------------------------------*/
      case WM_DESTROY:
        pIda = ACCESSWNDIDA( hwnd, POBJM_IDA);
        UtlAlloc( (PVOID *)&(pIda->pHndlrTbl), 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&(pIda->pObjTbl), 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&(pIda->pSymbolTbl), 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );
        return( MRFROMSHORT(0) );

/*--------------------------------------------------------------------------*/
      case WM_EQF_TERMINATE:
//      terminating the object manager will WM_EQF_TERMINATE all active
//      ...handlers and objects
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        us = SHORT1FROMMP1( mp1);       // get style of close request
        if( SendAll( pIda->pHndlrTbl, clsANY, WM_EQF_SHUTDOWN,
                     MP1FROMSHORT( us ), 0L))
          if( MBID_YES == UtlError( QUERY_FORCE_SHUTDOWN,
                                    MB_YESNO | MB_ICONQUESTION,
                                    0, (PSZ *) NULP, EQF_QUERY)){
            us |= TWBFORCE;
            SendAll( pIda->pHndlrTbl, clsANY, WM_EQF_SHUTDOWN,
                     MP1FROMSHORT( us ), MP2FROMP(NULL) );
          } else {
            SendAll( pIda->pHndlrTbl, clsANY, WM_EQFN_SHUTDOWNCANCELED,
                     0, 0L);
            return( (MRESULT)TRUE);
          }
        fNotClosed = TerminateObjects( pIda, us);
        if( fNotClosed && !(us & TWBFORCE))
          if( MBID_YES == UtlError( QUERY_FORCE_SHUTDOWN,
                                    MB_YESNO | MB_ICONQUESTION,
                                    0,(PSZ *) NULP, EQF_QUERY)){
            us |= TWBFORCE;
            TerminateObjects( pIda, us);
          } else {
            SendAll( pIda->pHndlrTbl, clsANY, WM_EQFN_SHUTDOWNCANCELED,
                     0, 0L);
            return( (MRESULT)TRUE);
          }
//      Wait for objects to terminate processing
        hab = WinQueryAnchorBlock( hwnd);
       {
         HWND hwndClient =  (HWND) UtlQueryULong( QL_TWBCLIENT );
         MSG msg;                      // message queue structure

         while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
         {
           if (!TranslateMDISysAccel(hwndClient, &msg) )
           {
             TranslateMessage( &msg );
             DispatchMessage( &msg );
           } /* endif */
         } /* endwhile */
       }
        TerminateHandlers( pIda, us);
//      Wait for handlers to terminate processing
       {
         HWND hwndClient =  (HWND) UtlQueryULong( QL_TWBCLIENT );
         MSG msg;                      // message queue structure

         while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE))
         {
           if (!TranslateMDISysAccel(hwndClient, &msg) )
           {
             TranslateMessage( &msg );
             DispatchMessage( &msg );
           } /* endif */
         } /* endwhile */
       }
        hObjMan[UtlGetTask()] = NULLHANDLE;                // we finished
        WinDestroyAtomTable( pIda->hAtomTbl);
        WinDestroyAtomTable( pIda->hAtomSymTbl);
        WinPostMsg( hwnd, WM_CLOSE, 0, 0L);
        return( MRFROMSHORT(0) );

/*--------------------------------------------------------------------------*/
      case WM_EQF_QUERYHANDLER:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        if ( (atom = WinFindAtom( pIda->hAtomTbl, (PSZ)mp2))== 0)
          return( MRFROMSHORT(0) );
        pEntry = FindObjByAtom( pIda->pHndlrTbl, atom);
        return( pEntry ? (MRESULT)pEntry->hWnd : 0L);

/*--------------------------------------------------------------------------*/
      case WM_EQF_REMOVEHANDLER:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        if( (atom= WinFindAtom( pIda->hAtomTbl, (PSZ)mp2))== 0)
          return( (MRESULT)TRUE);      // handler not found
        pEntry = FindObjByAtom( pIda->pHndlrTbl, atom);
        if( pEntry)
          return( (MRESULT)RemoveHandler( pIda, pEntry, SHORT1FROMMP1( mp1)));
        return( (MRESULT)TRUE);        // not deleted

/*--------------------------------------------------------------------------*/
      case WM_EQF_QUERYHANDLERCOUNT:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        return( (MRESULT)pIda->pHndlrTbl->usUsed);

/*--------------------------------------------------------------------------*/
      case WM_EQF_REMOVEOBJECT:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        pEntry = FindObjByHwnd( pIda->pObjTbl, (HWND)mp2);
        if( pEntry)
          return( (MRESULT)RemoveObject( pIda, pEntry, SHORT1FROMMP1( mp1)));
        else
          WinSendMsg( (HWND)mp2, WM_EQF_TERMINATE, mp1, NULL);
        return( (MRESULT)TRUE);        // not deleted

/*--------------------------------------------------------------------------*/
      case WM_EQF_CHANGEOBJECTNAME:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        pEntry = FindObjByHwnd( pIda->pObjTbl, (HWND)mp1);
        if( pEntry)
        {
          EQF_ATOM      atom;
          PSZ           pszNewName = (PSZ) PVOIDFROMMP2(mp2);

          atom = WinFindAtom( pIda->hAtomTbl, pszNewName );
          if ( atom == pEntry->aName )
          {
            /**********************************************************/
            /* The new name is alread associated with the object,     */
            /* so we have to do nothing                               */
            /**********************************************************/
            return( (MRESULT)0 );
          }
          else if ( atom == NULLHANDLE )
          {
            /**********************************************************/
            /* Name is not in use, so change the name of the object   */
            /**********************************************************/
            WinDeleteAtom( pIda->hAtomTbl, pEntry->aName);
            pEntry->aName = WinAddAtom( pIda->hAtomTbl, pszNewName );
            return( (MRESULT)0 );
          }
          else
          {
            /*********************************************************/
            /* Name is already in use for another object, return     */
            /* error condition                                       */
            /*********************************************************/
            return( MRFROMSHORT(ErrObjM_AlreadyReg));
          } /* endif */
        }
        return( (MRESULT)TRUE);        // not changed

/*--------------------------------------------------------------------------*/
      case WM_EQF_QUERYOBJECT:
        {
          PQUERYOBJ  pQueryObj;

          pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
          pQueryObj = (PQUERYOBJ) PVOIDFROMMP2(mp2);

          if( pQueryObj->pBuffer )
          {                      // object name given
            if ( (atom = WinFindAtom( pIda->hAtomTbl, pQueryObj->pBuffer))== 0)
              pEntry = NULL;
            else
              pEntry = FindObjByAtomClass( pIda->pObjTbl, atom, pQueryObj->sClass);
          } else
          pEntry = FindObjByClassFlags( pIda->pObjTbl,
                                pQueryObj->sClass, pQueryObj->sFlags );
        return( pEntry ? (MRESULT)pEntry->hWnd : 0L);
        }
        break;

/*--------------------------------------------------------------------------*/
      case WM_EQF_QUERYOBJECTNAME:
        {
          PQUERYOBJNAME pQueryObjName;

          pQueryObjName = (PQUERYOBJNAME) PVOIDFROMMP2(mp2);

          pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
          pEntry = FindObjByHwnd( pIda->pObjTbl, pQueryObjName->hwnd );
        if( pEntry)
          WinQueryAtomName( pIda->hAtomTbl, pEntry->aName,
                            pQueryObjName->pBuffer, MAX_EQF_PATH);
        else
          *pQueryObjName->pBuffer = '\0';
        return( (MRESULT)strlen( pQueryObjName->pBuffer ));
        }
        break;

/*--------------------------------------------------------------------------*/
      case WM_EQF_QUERYOBJECTCLASS:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        pEntry = FindObjByHwnd( pIda->pObjTbl, (HWND)mp2);
        return( pEntry ? (MRESULT)pEntry->usClassID : (MRESULT)-1);

/*--------------------------------------------------------------------------*/
      case WM_EQF_QUERYOBJECTSTATUS:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        pEntry = FindObjByHwnd( pIda->pObjTbl, (HWND)mp2);
        return( pEntry ? (MRESULT)pEntry->fStatus : 0L);

/*--------------------------------------------------------------------------*/
      case WM_EQF_SETOBJECTSTATUS:
        {
          PSETOBJSTATUS  pSetObjStatus;

          pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
          pSetObjStatus = (PSETOBJSTATUS) PVOIDFROMMP2(mp2);

          pEntry = FindObjByHwnd( pIda->pObjTbl, pSetObjStatus->hwnd );
          if( pEntry)
          {
            pEntry->fStatus &= ~pSetObjStatus->sMask | pSetObjStatus->sStatus;
            pEntry->fStatus |=  pSetObjStatus->sMask & pSetObjStatus->sStatus;
            pEntry->fStatus |=  OBJ_MANAGER;
          }
          return( pEntry ? (MRESULT)pEntry->fStatus : 0L);
        }
        break;

/*--------------------------------------------------------------------------*/
      case WM_EQF_QUERYOBJECTCOUNT:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        if( (i=SHORT1FROMMP1( mp1)) == clsANY)
          return( (MRESULT)pIda->pObjTbl->usUsed);
        return( (MRESULT)CountObjByClass( pIda->pObjTbl, (USHORT)i));

/*--------------------------------------------------------------------------*/
      case WM_EQF_GETOBJECTLIST:
        {
          PGETOBJLIST pGetObjList;

          pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
          pGetObjList = (PGETOBJLIST) PVOIDFROMMP2(mp2);

          i = min( pGetObjList->sCount, (SHORT)pIda->pObjTbl->usUsed);
          i = LoadObjList( pIda->pObjTbl,
                           pGetObjList->sClass,
                           (POBJLST)pGetObjList->pBuffer, i);
        return( (MRESULT)i);
        }
        break;

/*--------------------------------------------------------------------------*/
      case WM_EQF_SETSYMBOL:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        pEntry = NULL;
        if( (atom = WinFindAtom( pIda->hAtomSymTbl, (PSZ) PVOIDFROMMP2(mp2))) != NULLHANDLE)
          pEntry = FindObjByAtom( pIda->pSymbolTbl, atom);
        if( !pEntry){
          if( (pEntry= MakeObjEntry( pIda->pSymbolTbl))== NULL)
            return( (MRESULT)-1);         // no storage ?
          pEntry->aName = WinAddAtom( pIda->hAtomSymTbl, (PSZ) PVOIDFROMMP2(mp2) );
          pEntry->hWnd = NULLHANDLE;
          pEntry->usClassID = clsSYMBOL;
        }
        pEntry->fStatus = TRUE;
        return( (MRESULT)pEntry->fStatus);

/*--------------------------------------------------------------------------*/
      case WM_EQF_REMOVESYMBOL:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        if( (atom= WinFindAtom( pIda->hAtomSymTbl, (PSZ)mp2)) == 0)
          return( (MRESULT)-1);        // not found
        pEntry = FindObjByAtom( pIda->pSymbolTbl, atom);
        if( pEntry){
          WinDeleteAtom( pIda->hAtomSymTbl, pEntry->aName);
          RemoveObjEntry( pIda->pSymbolTbl, pEntry->aName);
        }
        return( MRFROMSHORT(0) );

/*--------------------------------------------------------------------------*/
      case WM_EQF_QUERYSYMBOL:
        pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
        if( (atom= WinFindAtom( pIda->hAtomSymTbl, (PSZ)mp2)) == 0 )
          return( (MRESULT)-1);        // not found
        pEntry = FindObjByAtom( pIda->pSymbolTbl, atom);
        return( pEntry ? (MRESULT)pEntry->fStatus : (MRESULT)-1);

/*--------------------------------------------------------------------------*/
    }
    return( WinDefWindowProc( hwnd, message, mp1, mp2));
}

/*+--------------------------------------------------------------------------+
     Make object entry
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
POBJENTRY MakeObjEntry( POBJTBL pt)
{
    return( pt->usUsed>=pt->usMax ? NULL : pt->pObjEntry+pt->usUsed++);
}

/*+--------------------------------------------------------------------------+
     Remove object entry
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
VOID RemoveObjEntry( POBJTBL pt, EQF_ATOM at)
{
    register int i;
    POBJENTRY pe=pt->pObjEntry;

    for( i=pt->usUsed; i; --i,++pe)
      if( at == pe->aName){
        for (--i; i; --i,++pe)
          memcpy( pe, pe+1, sizeof( *pe));
        pt->usUsed--;
        return;
      }
}

/*+--------------------------------------------------------------------------+
     Terminate all objects
  +--------------------------------------------------------------------------+
*/
int TerminateObjects( POBJM_IDA pIda, USHORT flg)
{
    register int i;
    POBJENTRY pe=pIda->pObjTbl->pObjEntry;

    i = pIda->pObjTbl->usUsed;
    for( pe+=i-1; i; --i,--pe)
      if( RemoveObject( pIda, pe, flg))
        return( 1);
    return( 0 );
}

/*+--------------------------------------------------------------------------+
     Remove object and notify all handlers
  +--------------------------------------------------------------------------+
*/
int RemoveObject( POBJM_IDA pIda, POBJENTRY pe, USHORT flg)
{
    USHORT cls, status;
    OBJNAME szObjName;
    BOOL    fNotClosed;

    fNotClosed = (BOOL)WinSendMsg( pe->hWnd, WM_EQF_TERMINATE, MP1FROMSHORT( flg ), NULL);

    if( fNotClosed && !(flg & TWBFORCE) )
    {
      return( 1 );
    } /* endif */

    // attention: if the workbench is closed while the called object is
    //            processing its WM_EQF_TERMINATE message, all object tables
    //            and handler tables have been removed. So here we have to
    //            check if the object manager is still up and running

    if ( hObjMan[UtlGetTask()] )                     // object manager still working???
    {
       cls    = pe->usClassID;
       status = pe->fStatus;
       WinQueryAtomName( pIda->hAtomTbl, pe->aName,
                         szObjName, sizeof( szObjName));
       WinDeleteAtom( pIda->hAtomTbl, pe->aName);
       RemoveObjEntry( pIda->pObjTbl, pe->aName);

       SendAll( pIda->pHndlrTbl, clsANY, WM_EQFN_OBJECTREMOVED,
                MP1FROMSHORT( cls ), MP2FROMP(szObjName) );
    } /* endif */
    return( 0 );                     // success
}

/*+--------------------------------------------------------------------------+
     Terminate all Handlers
  +--------------------------------------------------------------------------+
*/
int TerminateHandlers( POBJM_IDA pIda, USHORT flg)
{
    register int i;
    POBJENTRY pe=pIda->pHndlrTbl->pObjEntry;

    i = pIda->pHndlrTbl->usUsed;
    for( pe+=i-1; i; --i,--pe)
      if( RemoveHandler( pIda, pe, flg))
        return( 1);
    return( 0 );
}

/*+--------------------------------------------------------------------------+
     Remove handler
  +--------------------------------------------------------------------------+
*/
int RemoveHandler( POBJM_IDA pIda, POBJENTRY pe, USHORT flg)
{
    USHORT cls, status;
    OBJNAME szObjName;
    if( WinSendMsg( pe->hWnd, WM_EQF_TERMINATE, MP1FROMSHORT( flg ), NULL))
      if( !(flg & TWBFORCE))
        return( 1);
    // attention: if the workbench is closed while the called handler is
    //            processing its WM_EQF_TERMINATE message, all object tables
    //            and handler tables have been removed. So here we have to
    //            check if the object manager is still up and running

    if ( hObjMan[UtlGetTask()] )                     // object manager still working???
    {
       cls    = pe->usClassID;
       status = pe->fStatus;
       WinQueryAtomName( pIda->hAtomTbl, pe->aName,
                         szObjName, sizeof( szObjName));
       WinDeleteAtom( pIda->hAtomTbl, pe->aName);
       RemoveObjEntry( pIda->pHndlrTbl, pe->aName);
    } /* endif */

    return( 0 );                     // success
}

/*+--------------------------------------------------------------------------+
     Find object entry by its atom
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
POBJENTRY FindObjByAtom( POBJTBL pt, EQF_ATOM at)
{
    register int i;
    POBJENTRY pe=pt->pObjEntry;

    for( i=pt->usUsed; i; --i,++pe)
      if( at == pe->aName)
        return( pe);
    return( NULL);
}

/*+--------------------------------------------------------------------------+
     Find object entry by its hwnd
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
POBJENTRY FindObjByHwnd( POBJTBL pt, HWND hwnd)
{
    register int i;
    POBJENTRY pe=pt->pObjEntry;

    for( i=pt->usUsed; i; --i,++pe)
      if( (hwnd == pe->hWnd) || ( hwnd == GETPARENT(pe->hWnd)) )
        return( pe);
    return( NULL);
}

POBJENTRY FindObjByAtomClass( POBJTBL pt, EQF_ATOM atom, USHORT cls)
{
    register int i;
    POBJENTRY pe=pt->pObjEntry;

    for( i=pt->usUsed; i; --i,++pe)
      if( cls == clsANY || cls == pe->usClassID)
        if( pe->aName == atom)
          return( pe);
    return( NULL);

}

/*+--------------------------------------------------------------------------+
     Find object by its class id
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
POBJENTRY FindObjByClass( POBJTBL pt, USHORT cls)
{
    register int i;
    POBJENTRY pe=pt->pObjEntry;

    for( i=pt->usUsed; i; --i,++pe)
      if( cls == pe->usClassID)
        return( pe);
    return( NULL);
}

/*+--------------------------------------------------------------------------+
     Find object by its class and status flags
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
POBJENTRY FindObjByClassFlags( POBJTBL pt, USHORT cls, USHORT flags)
{
    register int i;
    POBJENTRY pe=pt->pObjEntry;

    for( i=pt->usUsed; i; --i,++pe)
      if( cls == clsANY || cls == pe->usClassID)
        if( (pe->fStatus & flags) == flags)
          return( pe);
    return( NULL);
}

/*+--------------------------------------------------------------------------+
     Count objects of specific class
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
USHORT CountObjByClass( POBJTBL pt, USHORT cls)
{
    register int i;
    USHORT cnt;
    POBJENTRY pe=pt->pObjEntry;

    for( cnt=0,i=pt->usUsed; i; --i,++pe)
      if( cls == clsANY || cls == pe->usClassID) cnt++;
    return( cnt);
}

/*+--------------------------------------------------------------------------+
     Load list of objects of a specific class
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
USHORT LoadObjList( POBJTBL pt, USHORT cls, POBJLST ptab, register int i)
{
    USHORT cnt, j;
    POBJENTRY pe=pt->pObjEntry;

    cnt=0;
    for( j=0; i && (j < pt->usUsed); j++,++pe)
      if( cls == clsANY || cls == pe->usClassID){
        i--;
        cnt++;
        ptab->hwnd = pe->hWnd;
        ptab->flgs = pe->fStatus;
    ptab->usClassID = pe->usClassID;
        ptab++;
      }
    return( cnt);
}

/*+--------------------------------------------------------------------------+
     Send a message to all active EQF objects
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
USHORT SendAll( POBJTBL pt, CLASSES objClass, WINMSG message,
                WPARAM mp1, LPARAM mp2)
{
    register int i;
    POBJENTRY pe;
    USHORT x=0;

//  If not controlled or indicated from outside this proc it is strongly
//  required to loop through the table from bottom to top !! to meet the
//  last registered objects before sending a msg to the "elder" objects.
//  This is required e.g. for message WM_EQF_TERMINATE which might send
//  messages to their quasi parents (i.e. parent objects).
//  There is no other way at the present moment to control the sequence
//  of objects which will be sent the message.

//  wnd proc returns NULL if ok otherwise !NULL
    for( i=pt->usUsed,pe=pt->pObjEntry+i-1; i; --i,--pe)
      if( objClass == clsANY || pe->usClassID == objClass)
        x = x + (USHORT)WinSendMsg( pe->hWnd, message, mp1, mp2);
    return( x);
}

HWND EqfQueryObject( PSZ pszObj, SHORT sClass, SHORT sFlags )
{
  QUERYOBJ QueryObj;

  QueryObj.sClass = sClass;
  QueryObj.sFlags = sFlags;
  QueryObj.pBuffer = pszObj;
  return ( (HWND)WinSendMsg( EqfQueryObjectManager(),
                             WM_EQF_QUERYOBJECT,
                             MP1FROMSHORT( 0 ),
                             MP2FROMP( &QueryObj ) ) );
}

SHORT EqfQueryObjectName( HWND hwnd, PSZ pBuffer )
{
  QUERYOBJNAME  QueryObjName;

  QueryObjName.hwnd    = hwnd;
  QueryObjName.pBuffer = pBuffer;

  return( (SHORT)WinSendMsg( EqfQueryObjectManager(),
                       WM_EQF_QUERYOBJECTNAME,
                       MP1FROMSHORT(0),
                       MP2FROMP(&QueryObjName)) );
}

USHORT EqfSetObjectStatus( HWND hwnd, SHORT sMask, SHORT sStatus )
{
  SETOBJSTATUS  SetObjStatus;

  SetObjStatus.hwnd    = hwnd;
  SetObjStatus.sMask   = sMask;
  SetObjStatus.sStatus = sStatus;

  return ( (USHORT)WinSendMsg( EqfQueryObjectManager(),
                               WM_EQF_SETOBJECTSTATUS,
                               MP1FROMSHORT( 0 ),
                               MP2FROMP(&SetObjStatus) ) );
}

USHORT EqfGetObjectList( SHORT sClass, SHORT sCount, PVOID pBuffer )
{
  GETOBJLIST GetObjList;

  GetObjList.sClass  = sClass;
  GetObjList.sCount  = sCount;
  GetObjList.pBuffer = pBuffer;

  return( (USHORT)WinSendMsg( EqfQueryObjectManager(),
                       WM_EQF_GETOBJECTLIST,
                       MP1FROMSHORT( 0 ),
                       MP2FROMP( &GetObjList )) );

}

MRESULT EqfSend2Handler( PSZ psz, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
  MRESULT mResult = FALSE;
  HWND hwnd = EqfQueryHandler( psz);

  if ( hwnd )
  {
    mResult = WinSendMsg( hwnd, msg, mp1, mp2);
  } /* endif */
  return( mResult );
}

MRESULT EqfPost2Handler( PSZ psz, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
  MRESULT mResult = FALSE;
  HWND hwnd = EqfQueryHandler( psz);

  if ( hwnd )
  {
    mResult = (MRESULT)WinPostMsg( hwnd, msg, mp1, mp2);
  } /* endif */
  return( mResult );
}

// Function PropHandlerInitForBatch
// Initialize the property handler for non-windows environments;
// i.e. perform WM_CREATE handling to allocate our IDA
BOOL ObjHandlerInitForBatch( void )
{
  POBJM_IDA     pIda;                // Points to instance data area
  int us;

  hObjMan[UtlGetTask()] = NULLHANDLE;      // we are not ready yet
  if( !UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof( *pIda), ERROR_STORAGE ) )
    return( FALSE);
  memset( pIda, 0, sizeof( *pIda));
  pObjBatchIda = pIda;
  strcpy( pIda->IdaHead.szObjName, OBJECTMANAGER );
  pIda->IdaHead.pszObjName = pIda->IdaHead.szObjName;

  us = sizeof( OBJTBL) + HANDLERENTRIES * sizeof( OBJENTRY);
  UtlAlloc( (PVOID *)&pIda->pHndlrTbl, 0L, (LONG)us, ERROR_STORAGE );
  memset( pIda->pHndlrTbl, 0, us);
  pIda->pHndlrTbl->usUsed = 0;
  pIda->pHndlrTbl->usMax  = HANDLERENTRIES;
  pIda->pHndlrTbl->pObjEntry =
     (POBJENTRY) ( (PBYTE)pIda->pHndlrTbl+ sizeof( OBJTBL) );

  us = sizeof( OBJTBL) + OBJECTENTRIES * sizeof( OBJENTRY);
  UtlAlloc( (PVOID *)&pIda->pObjTbl, 0L, (LONG)us, ERROR_STORAGE );
  memset( pIda->pObjTbl, 0, us);
  pIda->pObjTbl->usUsed = 0;
  pIda->pObjTbl->usMax  = OBJECTENTRIES;
  pIda->pObjTbl->pObjEntry =
     (POBJENTRY)( (PBYTE)pIda->pObjTbl+ sizeof( OBJTBL) );
  pIda->hAtomTbl = WinCreateAtomTable( 0, 0 );

  //instead of a symbol table we use a lock table in shared memory
  hSharedMem = ObjLockTable_CreateOrOpenTable();

  return( TRUE );
} /* end of function ObjHandlerInitForBatch */

// Function PropHandlerTerminateForBatch
// Terminate the property handler in non-windows environments;
BOOL ObjHandlerTerminateForBatch( void )
{
    POBJM_IDA     pIda;                // Points to instance data area

    pIda = pObjBatchIda;
    if ( pIda != NULL )
    {
      WinDestroyAtomTable( pIda->hAtomTbl);
      // WinDestroyAtomTable( pIda->hAtomSymTbl);
    } /* endif */

  // remove all our locked entries from locked table and free handle
  if ( hSharedMem)
  {
    PFUNCIF_LOCK_TABLE pLockTable = ObjLockTable_AccessTable( hSharedMem);
    if ( pLockTable )
    {
      ObjLockTable_Delete(pLockTable, NULL, GetCurrentProcessId() );
      ObjLockTable_ReleaseTable(pLockTable);
    } /* endif */
    CloseHandle( hSharedMem );
    hSharedMem = NULLHANDLE;
  } /* endif */

  return( TRUE );
} /* end of function ObjHandlerTerminateForBatch */

SHORT ObjQuerySymbol( PSZ pszSymbol )
{
  SHORT sRC = 0;

  if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
  {
    if ( hSharedMem)
    {
      PFUNCIF_LOCK_TABLE pLockTable = ObjLockTable_AccessTable( hSharedMem);
      if ( pLockTable )
      {
        PFUNCIF_LOCK_ENTRY pEntry;
        pEntry = ObjLockTable_Search(pLockTable,pszSymbol,TRUE);
        if ( !pEntry ) sRC = -1;
        ObjLockTable_ReleaseTable(pLockTable);
      } /* endif */
    }
  }
  else
  {
    sRC = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                      WM_EQF_QUERYSYMBOL, NULL, MP2FROMP(pszSymbol) );
  } /* endif */
  return( sRC );
} /* endif ObjQuerySymbol */

SHORT ObjSetSymbol( PSZ pszSymbol )
{
  SHORT sRC = 0;

  if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
  {
    if ( hSharedMem)
    {
      PFUNCIF_LOCK_TABLE pLockTable = ObjLockTable_AccessTable( hSharedMem);
      if ( pLockTable )
      {
        if ( ObjLockTable_Add( pLockTable, pszSymbol, GetCurrentProcessId() ) )
        {
          sRC = 1;
        } /* endif */
        ObjLockTable_ReleaseTable(pLockTable);
      } /* endif */
    } /* endif */
  }
  else
  {
    WinSendMsg( EqfQueryObjectManager(), WM_EQF_SETSYMBOL,
                MP1FROMSHORT( TRUE ),
                MP2FROMP( pszSymbol ) );
  } /* endif */
  return( sRC );
} /* endif ObjSetSymbol */

SHORT ObjRemoveSymbol( PSZ pszSymbol )
{
  SHORT sRC = 0;

  if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
  {
    if ( hSharedMem)
    {
      PFUNCIF_LOCK_TABLE pLockTable = ObjLockTable_AccessTable( hSharedMem);
      if ( pLockTable )
      {
        if ( !ObjLockTable_Delete( pLockTable, pszSymbol, 0 ) )
        {
          sRC = -1;
        } /* endif */
        ObjLockTable_ReleaseTable(pLockTable);
      } /* endif */
    } /* endif */
  }
  else
  {
   sRC = (SHORT)WinSendMsg( EqfQueryObjectManager(), WM_EQF_REMOVESYMBOL,
                            NULL, MP2FROMP( pszSymbol ) );
  } /* endif */
  return( sRC );
} /* endif ObjRemoveSymbol */

//////////////////////////////////////////////////////////////////
/// Lock Table handling                                        ///
//////////////////////////////////////////////////////////////////

HANDLE ObjLockTable_CreateOrOpenTable()
{
  HANDLE hShMem = OpenFileMapping( FILE_MAP_ALL_ACCESS, TRUE, EQFLOCKOBJ_SHMEM );
  if ( !hShMem )
  {
    hShMem = CreateFileMapping(
                (HANDLE)0xFFFFFFFF,          // use page file
                NULL,                        // no security attrib
                PAGE_READWRITE,              // read/write access
                0,                           // size: high 32bits
                sizeof(FUNCIF_LOCK_TABLE),   // size: low 32bit
                EQFLOCKOBJ_SHMEM  );         // name of file mapping
    if ( !hShMem )
    {
        DWORD Error;
        Error = GetLastError();
    }
  }
  return( hShMem );
} /* end of function ObjLockTable_CreateOrOpenTable */

PFUNCIF_LOCK_TABLE ObjLockTable_AccessTable( HANDLE hSharedMem )
{
  PFUNCIF_LOCK_TABLE pTable;

  pTable = (PFUNCIF_LOCK_TABLE )MapViewOfFile( hSharedMem,
                                               FILE_MAP_WRITE,
                                               0, 0, 0);
  return( pTable );
} /* end of function ObjLockTable_AccessTable */

void ObjLockTable_ReleaseTable( PFUNCIF_LOCK_TABLE pTable )
{
  UnmapViewOfFile(pTable);
} /* end of function ObjLockTable_AccessTable */

BOOL ObjLockTable_SetUpdateFlag( PFUNCIF_LOCK_TABLE pLockTable )
{
  BOOL  fTimeOut = FALSE;

  fTimeOut = ObjLockTable_WaitWhenUpdated( pLockTable );
  if ( !fTimeOut )
  {
    pLockTable->fUpdateInProgress = TRUE;
  } /* endif */
  return( !fTimeOut );
} /* end of function ObjLockTable_SetUpdateFlag */

BOOL ObjLockTable_ClearUpdateFlag( PFUNCIF_LOCK_TABLE pLockTable )
{
  pLockTable->fUpdateInProgress = FALSE;
  return( TRUE );
} /* end of function ObjLockTable_ClearUpdateFlag */

PFUNCIF_LOCK_ENTRY ObjLockTable_Search
(
  PFUNCIF_LOCK_TABLE pLockTable,
  PSZ                pszObject,
  BOOL               fUpdateCheck      // true = check update flag
)
{
  PFUNCIF_LOCK_ENTRY pEntry = pLockTable->aLock;
  PFUNCIF_LOCK_ENTRY pFound = NULL;
  BOOL  fTimeOut = FALSE;
  BOOL fRestart = FALSE;

  // wait until any pending update has been completed
  do
  {
    int  iEntries = 0;
    fRestart = FALSE;

    // wait until any pending update has been completed
    if ( fUpdateCheck )
    {
      fTimeOut = ObjLockTable_WaitWhenUpdated( pLockTable );
    } /* endif */

    if ( !fTimeOut && pLockTable->iEntries )
    {
      pEntry = pLockTable->aLock;
      iEntries = pLockTable->iEntries;

      do
      {
        if ( fUpdateCheck && pLockTable->fUpdateInProgress )
        {
          fRestart = TRUE;
        }
        else if ( _stricmp( pEntry->szLockObj, pszObject ) == 0 )
        {
          pFound = pEntry;
        }
        else
        {
          pEntry++;
          iEntries--;
        } /* endif */
      }
      while ( !fRestart && iEntries && (pFound == NULL) );
    } /* endif */
  } while ( !fTimeOut && fRestart );

  return( pFound );
} /* end of function ObjLockTable_Search */


// add a new entry to a lock table
BOOL ObjLockTable_Add
(
  PFUNCIF_LOCK_TABLE pLockTable,
  PSZ   pszObject,                     // object being added
  DWORD dwProcessID                    // processID if process adding object
)
{
  BOOL fOK = TRUE;

  if ( ObjLockTable_SetUpdateFlag(pLockTable) )
  {
    // check if object is already in table
    PFUNCIF_LOCK_ENTRY pEntry = ObjLockTable_Search( pLockTable, pszObject, FALSE );
    if ( !pEntry )
    {
      // check if there is still room left in table
      if ( pLockTable->iEntries < SYMBOLENTRIES )
      {
        // add as new entry
        pEntry = pLockTable->aLock + pLockTable->iEntries;
        strcpy( pEntry->szLockObj, pszObject );
        pEntry->dwProcessID = dwProcessID;
        pLockTable->iEntries++;
      }
      else
      {
        // table is full
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      // object is already in lock table
      fOK = FALSE;
    } /* endif */
    ObjLockTable_ClearUpdateFlag(pLockTable);
  } /* endif */
  return( fOK );
} /* endif */

// delete a specific entry or all entries belonging to the given process
// either an object or a process ID has to be specified
BOOL ObjLockTable_Delete
(
  PFUNCIF_LOCK_TABLE pLockTable,
  PSZ   pszObject,                     // object being removed or NULL
  DWORD dwProcessID                    // processID if no object has been specified
)
{
  BOOL fOK = TRUE;

  if ( ObjLockTable_SetUpdateFlag(pLockTable) )
  {
    if ( pszObject )
    {
      // delete given object
      PFUNCIF_LOCK_ENTRY pEntry = ObjLockTable_Search( pLockTable, pszObject, FALSE );
      if ( pEntry )
      {
        int iEntry = pEntry - pLockTable->aLock;
        int iEntriesToMove = pLockTable->iEntries - iEntry - 1;
        if ( iEntriesToMove )
        {
          memmove( pEntry, pEntry+1, iEntriesToMove * sizeof(FUNCIF_LOCK_ENTRY) );
        } /* endif */
        pLockTable->iEntries--;
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      // delete all object from given process
      int i = 0;
      PFUNCIF_LOCK_ENTRY pSource, pTarget;

      pSource = pTarget = pLockTable->aLock;
      while ( i < pLockTable->iEntries )
      {
        if ( pSource->dwProcessID == dwProcessID )
        {
          // remove element
          pLockTable->iEntries--;
        }
        else
        {
          // copy element to current position
          memcpy( pTarget, pSource, sizeof(FUNCIF_LOCK_ENTRY) );
          pTarget++;
        } /* endif */
        i++;
        pSource++;
      } /* endwhile */
    } /* endif */
    ObjLockTable_ClearUpdateFlag(pLockTable);
  } /* endif */
  return( fOK );
} /* endif */

// wait until a pending update has been completed or a timeout occured
BOOL ObjLockTable_WaitWhenUpdated( PFUNCIF_LOCK_TABLE pLockTable )
{
  BOOL fTimeOut = FALSE;
  int  iNumOfRetries = 50;
  while ( pLockTable->fUpdateInProgress && !fTimeOut )
  {
    Sleep( 20 );
    iNumOfRetries--;
    fTimeOut = (iNumOfRetries == 0 );
  }
  return( fTimeOut );
} /* end of function ObjLockTable_Search */

USHORT ObjBroadcast
(
  WINMSG  msg,                         // message being broadcasted
  SHORT   sObjClass,                   // object class
  PSZ     pszObjName                   // ptr to object name
)
{
  if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
  {
    // TODO: runmode version of code
  }
  else
  {
    EqfSend2AllHandlers( msg, MP1FROMSHORT( sObjClass ),
                         MP2FROMP( pszObjName ) );
  } /* endif */
  return( 0 );
} /* endif ObjRemoveSymbol */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ObjLongToShortName                                       |
//+----------------------------------------------------------------------------+
//|Description:       Converts the given long name to a short name and checks  |
//|                   if there is already an obkect with the given name        |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ              pszLongName  ptr to object long name    |
//|                   PSZ              pszShortName ptr to buffer for short nam|
//|                   UTLCHECKOBJTYPES ObjType      type of object             |
//|                   PBOOL            pfIsNew      ptr to caller's is-new flag|
//+----------------------------------------------------------------------------+
//|Returns:           USHORT  usRC                                             |
//|                   NO_ERROR = O.K. anything other: error code               |
//+----------------------------------------------------------------------------+
USHORT ObjLongToShortName
(
  PSZ pszLongName,                     // ptr to object long name
  PSZ pszShortName,                    // ptr to buffer for short name
  UTLCHECKOBJTYPES ObjType,            // type of object
  PBOOL   pfIsNew                      // ptr to caller's is-new flag
)
{
  USHORT  usRC = NO_ERROR;
  OBJLONGTOSHORTSTATE ObjState;

  usRC = ObjLongToShortNameEx( pszLongName, EOS, pszShortName, ObjType, &ObjState );
  if ( pfIsNew != NULL )
  {
    *pfIsNew =  (ObjState != OBJ_EXISTS_ALREADY);
  } /* endif */
  return( usRC );
}

// Object long to short name conversion with check for shared resources
USHORT ObjLongToShortNameEx
(
  PSZ         pszLongName,             // ptr to object long name
  CHAR        chDrive,                 // drive to use for shared resource checking
  PSZ pszShortName,                    // ptr to buffer for short name
  UTLCHECKOBJTYPES ObjType,            // type of object
  POBJLONGTOSHORTSTATE pObjState       // ptr to buffer for returned object state
)
{
  return( ObjLongToShortNameEx2( pszLongName, chDrive, pszShortName, ObjType,
                                 pObjState, FALSE, NULL ) );
}

// Object long to short name conversion with check for shared resources and reservation of files
USHORT ObjLongToShortNameEx2
(
  PSZ         pszLongName,             // ptr to object long name
  CHAR        chDrive,                 // drive to use for shared resource checking
  PSZ pszShortName,                    // ptr to buffer for short name
  UTLCHECKOBJTYPES ObjType,            // type of object
  POBJLONGTOSHORTSTATE pObjState,      // ptr to buffer for returned object state
  BOOL        fReserveName,            // TRUE = reserve created short name
  PBOOL       pfReserved               // points to callers fReserved flag
)
{
  // our private data area
  typedef struct _OBJL2SDATA
  {
    CHAR      szShortName[MAX_FILESPEC];    // buffer for short name
    CHAR      szFolder[MAX_FILESPEC];       // buffer for folder name
    CHAR      szSearchPath[MAX_EQF_PATH];   // object search path
    CHAR      szFullPath[MAX_EQF_PATH];     // fully qualified object name
    CHAR      szSharedSearchPath[MAX_EQF_PATH];// object search path for shared resources
    CHAR      szSharedFullPath[MAX_EQF_PATH];  // fully qualified object name for shared resources
    FILEFINDBUF stResultBuf;                // DOS file find structure
    CHAR      szExt[20];                    // buffer for object extention
    CHAR      szSharedExt[20];              // buffer for shared object extention
    CHAR      szInLongName[MAX_LONGFILESPEC];// buffer for input long name
    CHAR      szPropLongName[MAX_LONGFILESPEC];// buffer for long name in property file
  } OBJL2SDATA, *POBJL2SDATA;

  // local variables
  PSZ         pszOrgLongName = pszLongName; // original start of long name
  POBJL2SDATA pData = NULL;            // ptr to private data area
  USHORT      usRC = NO_ERROR;         // function return code
  enum { SHORTNAME, LONGNAME, MAYBELONGNAME } NameType = SHORTNAME;
  OBJLONGTOSHORTSTATE  ObjState = OBJ_IS_NEW;  // local copy of caller's object state buffer
  SHORT i = 0;                         // number of characters in short name
  ULONG   ulCP = 0L;

  ulCP = GetCodePage( OEM_CP );       // use CP of installed OS

  // ignore any leading blanks
  while ( *pszOrgLongName == ' ' ) pszOrgLongName++;

  // preset callers's variables
  pszShortName[0] = EOS;

  // allocate our private data area
  if ( !UtlAlloc( (PVOID *)&pData, 0L, (LONG)sizeof(OBJL2SDATA), ERROR_STORAGE ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // scan long name and add valid characters to short file name
  if ( usRC == NO_ERROR )
  {
    while ( (i < 5) && (*pszLongName != EOS) )
    {
//    if ( isalnum(*pszLongName) )   // do not use isalnum anymore as isalnum
                                     // returns TRUE for other chars than A-Z
                                     // under Win2000/WinXP
      CHAR chTest;
      chTest = *pszLongName;
      if ( ((chTest >= '0') && (chTest <= '9')) || // digit or
           ((chTest >= 'a') && (chTest <= 'z')) || // lowercase char or
           ((chTest >= 'A') && (chTest <= 'Z')) )  // uppercase char ?
      {
        CHAR c = *pszLongName++;
        pData->szShortName[i++] = (CHAR)toupper(c);
      }
      else
      {
        NameType = LONGNAME;
        if ( IsDBCSLeadByteEx(ulCP,  *pszLongName ) && (pszLongName[1] != EOS) )
        {
          pszLongName++;                 // skip second byte of DBCS char
        }
        pszLongName++;                   // try next character
      } /* endif */
    } /* endwhile */

    // scan rest of long file name for long object name recognition
    if ( NameType != LONGNAME )        // no non-alphanumeric chars yet?
    {
      int iLen = i;                    // remember current length
      while ( (iLen <= 8) && (NameType != LONGNAME) && (*pszLongName != EOS) )
      {
  //    if ( isalnum(*pszLongName) )   // do not use isalnum anymore as isalnum
                                       // returns TRUE for other chars than A-Z
                                       // under Win2000/WinXP
        CHAR chTest;
        chTest = *pszLongName;
        if ( ((chTest >= '0') && (chTest <= '9')) || // digit or
             ((chTest >= 'a') && (chTest <= 'z')) || // lowercase char or
             ((chTest >= 'A') && (chTest <= 'Z')) )  // uppercase char ?
        {
          pszLongName++;
          iLen++;
        }
        else
        {
          NameType = LONGNAME;
        } /* endif */
      } /* endwhile */

      // due to a bug in a previous version short names with exactly
      // 8 characters were treated as long names - so we have to
      // check for it...
      if ( (iLen == 8) && (NameType != LONGNAME) )
      {
        NameType = MAYBELONGNAME;
      }
      else if ( iLen > 8 )
      {
        NameType = LONGNAME;
      } /* endif */
    } /* endif */

    // complete short name or use long name if it is a short one
    if ( NameType != SHORTNAME )
    {
      // padd short name with A's up to a length of 5 characters
      while ( i < 5 )
      {
        pData->szShortName[i++] = 'A';
      } /* endif */

      // terminate short file name
      pData->szShortName[i] = EOS;
    }
    else
    {
      strcpy( pData->szShortName, pszOrgLongName );
      UtlUpper( pData->szShortName );
    } /* endif */
  } /* endif */


  // setup search path and path of full object name depending on object type
  if ( usRC == NO_ERROR )
  {
    switch ( ObjType )
    {
      case FOLDER_OBJECT:
        UtlMakeEQFPath( pData->szSearchPath, NULC, PROPERTY_PATH, NULL );
        strcat( pData->szSearchPath, BACKSLASH_STR );
        strcpy( pData->szFullPath, pData->szSearchPath );
        strcat( pData->szSearchPath, pData->szShortName );
        if ( NameType == LONGNAME ) strcat( pData->szSearchPath, DEFAULT_PATTERN_NAME );
        strcpy( pData->szExt, EXT_FOLDER_MAIN );
        strcat( pData->szSearchPath, pData->szExt );

        chDrive = EOS; // no shared resource checking for folders

        break;
      case DICT_OBJECT:
        UtlMakeEQFPath( pData->szSearchPath, NULC, PROPERTY_PATH, NULL );
        strcat( pData->szSearchPath, BACKSLASH_STR );
        strcpy( pData->szFullPath, pData->szSearchPath );
        strcat( pData->szSearchPath, pData->szShortName );
        if ( NameType == LONGNAME ) strcat( pData->szSearchPath, DEFAULT_PATTERN_NAME );
        strcpy( pData->szExt, EXT_OF_DICTPROP );
        strcat( pData->szSearchPath, pData->szExt );

        if ( chDrive != EOS )
        {
          UtlMakeEQFPath( pData->szSharedSearchPath, chDrive, DIC_PATH, NULL );
          strcat( pData->szSharedSearchPath, BACKSLASH_STR );
          strcpy( pData->szSharedFullPath, pData->szSharedSearchPath );
          strcat( pData->szSharedSearchPath, pData->szShortName );
          if ( NameType == LONGNAME ) strcat( pData->szSharedSearchPath, DEFAULT_PATTERN_NAME );
          strcpy( pData->szSharedExt, EXT_OF_SHARED_DICTPROP );
          strcat( pData->szSharedSearchPath, pData->szSharedExt );
        } /* endif */
        break;

      case TM_OBJECT:
        UtlMakeEQFPath( pData->szSearchPath, NULC, PROPERTY_PATH, NULL );
        strcat( pData->szSearchPath, BACKSLASH_STR );
        strcpy( pData->szFullPath, pData->szSearchPath );
        strcat( pData->szSearchPath, pData->szShortName );
        if ( NameType == LONGNAME ) strcat( pData->szSearchPath, DEFAULT_PATTERN_NAME );
        // GQ: we have to check the properties of local memories (.MEM) and LAN based shared memories (.SLM)
        strcpy( pData->szExt, ".??M" );
        strcat( pData->szSearchPath, pData->szExt );

        if ( chDrive != EOS )
        {
          UtlMakeEQFPath( pData->szSharedSearchPath, chDrive, MEM_PATH, NULL );
          strcat( pData->szSharedSearchPath, BACKSLASH_STR );
          strcpy( pData->szSharedFullPath, pData->szSharedSearchPath );
          strcat( pData->szSharedSearchPath, pData->szShortName );
          if ( NameType == LONGNAME ) strcat( pData->szSharedSearchPath, DEFAULT_PATTERN_NAME );
          strcpy( pData->szSharedExt, EXT_OF_SHARED_MEMPROP );
          strcat( pData->szSharedSearchPath, pData->szSharedExt );
        } /* endif */
        break;

      default:
        usRC = ERROR_INVALID_PARAMETER;
    } /* endswitch */
  } /* endif */

  // look for objects having the same short name
  if ( usRC == NO_ERROR )
  {
    USHORT usDosRC = NO_ERROR;         // return code of called DOS functions
    USHORT usCount = 1;                // number of files requested
    HDIR   hDir = HDIR_CREATE;         // file find handle
    BOOL   fOK;
    HANDLE hMutexSem = NULL;

    GETMUTEX(hMutexSem);

    // if specified long name has exactly 8 alphanumeric characters
    // check if there is already an object with the given name
    // if not we have to use the long name search method as previous
    // versions of TMgr may have created this object using a long name
    if ( NameType == MAYBELONGNAME )
    {
      strcpy( pData->szSearchPath, pData->szFullPath );
      strcat( pData->szSearchPath, pszOrgLongName );
      strcat( pData->szSearchPath, pData->szExt );
      UtlUpper( pData->szSearchPath );

      if ( UtlFileExist(pData->szSearchPath) )
      {
        // switch to short file mode as there is already an object with this
        // short name
        NameType = SHORTNAME;
        strcpy( pData->szShortName, pszOrgLongName );
      }
      else
      {
        // restore long name search path
        strcpy( pData->szSearchPath, pData->szFullPath );
        strcat( pData->szSearchPath, pData->szShortName );
        strcat( pData->szSearchPath, DEFAULT_PATTERN_NAME );
        strcat( pData->szSearchPath, pData->szExt );
      } /* endif */
    } /* endif */

    // if specified long name is a short name use this name for search
    if ( NameType == SHORTNAME )
    {
      strcpy( pData->szSearchPath, pData->szFullPath );
      strcat( pData->szSearchPath, pData->szShortName );
      strcat( pData->szSearchPath, pData->szExt );
      strcpy( pszShortName, pData->szShortName );

      if (chDrive != EOS )
      {
        strcpy( pData->szSharedSearchPath, pData->szSharedFullPath );
        strcat( pData->szSharedSearchPath, pData->szShortName );
        strcat( pData->szSharedSearchPath, pData->szSharedExt );
      } /* endif */
    } /* endif */

    usDosRC = UtlFindFirst( pData->szSearchPath, &hDir, FILE_NORMAL,
                            &(pData->stResultBuf), sizeof(pData->stResultBuf),
                            &usCount, 0L, FALSE );

    while ( (ObjState == OBJ_IS_NEW) && (usDosRC == NO_ERROR) && usCount )
    {
      PVOID pProp = NULL;      // ptr to object properties
      ULONG ulBytesRead;

      // in TM_OBJECT mode only: ignore all files but files with the .MEM and .SLM extension
      PSZ pszExtension = strrchr( RESBUFNAME(pData->stResultBuf), DOT );
      if ( (ObjType != TM_OBJECT) || // no memory or
           (pszExtension == NULL) || // no extension found or
           (strcmp( pszExtension, EXT_OF_MEM ) == 0) || // a local memory property file or
           (strcmp( pszExtension, ".SLM" ) == 0) )      // a lan-based memory property file
      {
        strcpy( pData->szSearchPath, pData->szFullPath );
        strcat( pData->szSearchPath, RESBUFNAME(pData->stResultBuf) );

        fOK = UtlLoadFileL( pData->szSearchPath,// name of file to be loaded
                           &pProp,     // return pointer to loaded file
                           &ulBytesRead, // length of loaded file
                           FALSE, FALSE );
        if ( !fOK )
        {
          usDosRC = ERROR_PROPERTY_ACCESS;
        } /* endif */

        if ( usDosRC == NO_ERROR && (ulBytesRead > sizeof(PROPHEAD) ))
        {
          BOOL fFound;
          PSZ pszPropName = NULL;
          CHAR szShortName[MAX_FILESPEC];
          PPROPFOLDER pFolProp = (PPROPFOLDER)pProp;
          PPROPDICTIONARY pDicProp = (PPROPDICTIONARY)pProp;
          PPROP_NTM pMemProp = (PPROP_NTM)pProp;

          // get position of long name in property file
          switch ( ObjType)
          {
            case FOLDER_OBJECT:
              pszPropName = pFolProp->szLongName;
              if ( *pszPropName == EOS )
              {
                pszPropName = szShortName;
                Utlstrccpy( szShortName, pFolProp->PropHead.szName, DOT );
              } /* endif */
              break;
            case DICT_OBJECT:
              pszPropName = pDicProp->szLongName;
              if ( *pszPropName == EOS )
              {
                pszPropName = szShortName;
                Utlstrccpy( szShortName, pFolProp->PropHead.szName, DOT );
              } /* endif */
              break;

            case TM_OBJECT:
              pszPropName = pMemProp->szLongName;
              if ( *pszPropName == EOS )
              {
                pszPropName = szShortName;
                Utlstrccpy( szShortName, pFolProp->PropHead.szName, DOT );
              } /* endif */
              break;

          } /* endswitch */

          // compare long name of found object with given long name
          // use case-insensitive compare

          // Attention: using stricmp will not work for national characters
          //            as the names are stored in ASCII and stricmp expects
          //            Ansi strings...

          strcpy( pData->szInLongName, pszOrgLongName );
          OEMTOANSI( pData->szInLongName );
          CharUpper( pData->szInLongName  );
          strcpy( pData->szPropLongName, pszPropName  );
          OEMTOANSI( pData->szPropLongName );
          CharUpper( pData->szPropLongName );

          fFound = ( strcmp( pData->szInLongName, pData->szPropLongName ) == 0);

          if ( fFound )
          {
            ObjState = OBJ_EXISTS_ALREADY;
            Utlstrccpy( pszShortName, RESBUFNAME( pData->stResultBuf ), DOT );
            strcpy( pszOrgLongName, pszPropName );
          } /* endif */

          UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
        } /* endif */
      } /* endif */

      // try next object
      if ( ObjState == OBJ_IS_NEW )
      {
        usCount = 1;
        usDosRC = UtlFindNext( hDir, &pData->stResultBuf,
                               sizeof(pData->stResultBuf), &usCount, FALSE );
      } /* endif */
    } /* endwhile */

    UtlFindClose( hDir, FALSE );

    // release Mutex
    RELEASEMUTEX(hMutexSem);
  } /* endif */

  // check if there is a shared resource with the given long name
  if ( (usRC == NO_ERROR) && (ObjState == OBJ_IS_NEW) && (chDrive != EOS) )
  {
    USHORT usDosRC = NO_ERROR;         // return code of called DOS functions
    USHORT usCount = 1;                // number of files requested
    HDIR   hDir = HDIR_CREATE;         // file find handle
    BOOL   fOK;
    usDosRC = UtlFindFirst( pData->szSharedSearchPath, &hDir, FILE_NORMAL,
                            &(pData->stResultBuf), sizeof(pData->stResultBuf),
                            &usCount, 0L, FALSE );

    while ( (ObjState == OBJ_IS_NEW) && (usDosRC == NO_ERROR) && usCount )
    {
      PVOID pProp = NULL;      // ptr to object properties
      ULONG ulBytesRead;

      strcpy( pData->szSharedSearchPath, pData->szSharedFullPath );
      strcat( pData->szSharedSearchPath, RESBUFNAME(pData->stResultBuf) );

      fOK = UtlLoadFileL( pData->szSharedSearchPath,// name of file to be loaded
                         &pProp,     // return pointer to loaded file
                         &ulBytesRead, // length of loaded file
                         FALSE, FALSE );
      if ( !fOK )
      {
        usDosRC = ERROR_PROPERTY_ACCESS;
      } /* endif */

      // if load successful and file size is at least size of property header..
      if ( (usDosRC == NO_ERROR) && (ulBytesRead > sizeof(PROPHEAD) ) )
      {
        BOOL fFound;
        PSZ pszPropName = NULL;
        CHAR szShortName[MAX_FILESPEC];
        PPROPFOLDER pFolProp = (PPROPFOLDER)pProp;
        PPROPDICTIONARY pDicProp = (PPROPDICTIONARY)pProp;
        PPROP_NTM pMemProp = (PPROP_NTM)pProp;

        // get position of long name in property file
        switch ( ObjType)
        {
          case FOLDER_OBJECT:
            pszPropName = pFolProp->szLongName;
            if ( *pszPropName == EOS )
            {
              pszPropName = szShortName;
              Utlstrccpy( szShortName, pFolProp->PropHead.szName, DOT );
            } /* endif */
            break;
          case DICT_OBJECT:
            pszPropName = pDicProp->szLongName;
            if ( *pszPropName == EOS )
            {
              pszPropName = szShortName;
              Utlstrccpy( szShortName, pFolProp->PropHead.szName, DOT );
            } /* endif */
            break;

          case TM_OBJECT:
            pszPropName = pMemProp->szLongName;
            if ( *pszPropName == EOS )
            {
              pszPropName = szShortName;
              Utlstrccpy( szShortName, pFolProp->PropHead.szName, DOT );
            } /* endif */
            break;
        } /* endswitch */

        // compare long name of found object with given long name
        // use case-insensitive compare
        fFound = ( _stricmp( pszOrgLongName, pszPropName ) == 0);

        if ( fFound )
        {
          ObjState = SHARED_OBJ_EXISTS;
          Utlstrccpy( pszShortName, RESBUFNAME( pData->stResultBuf ), DOT );
          strcpy( pszOrgLongName, pszPropName );
        } /* endif */

        UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
      } /* endif */

      // try next object
      if ( ObjState == OBJ_IS_NEW )
      {
        usCount = 1;
        usDosRC = UtlFindNext( hDir, &pData->stResultBuf,
                               sizeof(pData->stResultBuf), &usCount, FALSE );
      } /* endif */
    } /* endwhile */

    UtlFindClose( hDir, FALSE );
  } /* endif */

  // find a unique name if document is not contained in the folder
  if ( (usRC == NO_ERROR) && (ObjState == OBJ_IS_NEW) && (NameType != SHORTNAME)  )
  {
    if ( NameType == MAYBELONGNAME  )
    {
      // use long name as short name to create the new object
      strcpy( pszShortName, pszOrgLongName );
      UtlUpper( pszShortName );
    }
    else
    {
      CHAR szCounter[4];                 // counter for object name
      BOOL fNameIsInUse = FALSE;
      HANDLE hMutexSem = NULL;

      strcpy( szCounter, "000" );        // counter start value

      // do not allow other process to search while we search for a new name
      GETMUTEX(hMutexSem);

      do
      {
        // build object name
        int iNameLenWithoutExt = 0;
        strcpy( pData->szSearchPath, pData->szFullPath );
        strcat( pData->szSearchPath, pData->szShortName );
        strcat( pData->szSearchPath, szCounter );
        iNameLenWithoutExt = strlen( pData->szSearchPath );
        if ( ObjType == TM_OBJECT )
        {
          strcat( pData->szSearchPath, EXT_OF_MEM );
        }
        else
        {
          strcat( pData->szSearchPath, pData->szExt );
        }

        if ( chDrive != EOS )
        {
          strcpy( pData->szSharedSearchPath, pData->szSharedFullPath );
          strcat( pData->szSharedSearchPath, pData->szShortName );
          strcat( pData->szSharedSearchPath, szCounter );
          strcat( pData->szSharedSearchPath, pData->szSharedExt );
        } /* endif */

        // increment alphanumeric 'counter' (counts from '000' to 'ZZZ')
        {
          if ( szCounter[2] == '9' )
          {
            szCounter[2] = 'A';
          }
          else if ( szCounter[2] == 'Z' )
          {
            szCounter[2] = '0';
            if ( szCounter[1] == '9' )
            {
              szCounter[1] = 'A';
            }
            else if ( szCounter[1] == 'Z' )
            {
              szCounter[1] = '0';
              if ( szCounter[0] == '9' )
              {
                szCounter[0] = 'A';
              }
              else if ( szCounter[0] == 'Z' )
              {
                // overflow of our counter ... restart with 000
                UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
                szCounter[0] = '0';
                szCounter[1] = '0';
                szCounter[2] = '0';
              }
              else
              {
                szCounter[0] += 1;
              } /* endif */
            }
            else
            {
              szCounter[1] += 1;
            } /* endif */
          }
          else
          {
            szCounter[2] += 1;
          } /* endif */
        }

        fNameIsInUse = UtlFileExist( pData->szSearchPath );

        if ( !fNameIsInUse && (chDrive != EOS) )
        {
          fNameIsInUse = UtlFileExist( pData->szSharedSearchPath );
        } /* endif */

        // for TM_OBJECTs only: try lan-based property file as well
        if ( !fNameIsInUse && (ObjType == TM_OBJECT) )
        {
          strcpy( pData->szSearchPath + iNameLenWithoutExt, ".SLM" );
          fNameIsInUse = UtlFileExist( pData->szSearchPath );
          strcpy( pData->szSearchPath + iNameLenWithoutExt, EXT_OF_MEM );
        }

      } while ( fNameIsInUse ); /* enddo */

      Utlstrccpy( pszShortName, UtlGetFnameFromPath( pData->szSearchPath ), DOT );

      // reserve name if requested
      if ( fReserveName )
      {
        switch ( ObjType)
        {
          case FOLDER_OBJECT:
            {
              PPROPFOLDER pProp = NULL;
              if ( UtlAlloc( (PVOID *)&pProp, 0L, sizeof(PROPFOLDER), NOMSG ) )
              {
                strcpy( pProp->szLongName, pszOrgLongName );
                strcpy( pProp->PropHead.szPath, pData->szSearchPath );
                strcpy( pProp->PropHead.szName, UtlSplitFnameFromPath(pProp->PropHead.szPath) );
                UtlSplitFnameFromPath( pProp->PropHead.szPath ); 
                UtlWriteFile( pData->szSearchPath, sizeof(PROPFOLDER), (PVOID)pProp, FALSE );
                UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
                if ( pfReserved ) *pfReserved = TRUE;
              } /* endif */
            }
            break;
          case DICT_OBJECT:
            {
              PPROPDICTIONARY pProp = NULL;
              if ( UtlAlloc( (PVOID *)&pProp, 0L, sizeof(PROPDICTIONARY), NOMSG ) )
              {
                strcpy( pProp->szLongName, pszOrgLongName );
                strcpy( pProp->PropHead.szPath, pData->szSearchPath );
                strcpy( pProp->PropHead.szName, UtlSplitFnameFromPath(pProp->PropHead.szPath) );
                UtlSplitFnameFromPath( pProp->PropHead.szPath ); 
                UtlWriteFile( pData->szSearchPath, sizeof(PROPDICTIONARY), (PVOID)pProp, FALSE );
                UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
                if ( pfReserved ) *pfReserved = TRUE;
              } /* endif */
            }
            break;

          case TM_OBJECT:
            {
              PPROP_NTM pProp = NULL;
              if ( UtlAlloc( (PVOID *)&pProp, 0L, sizeof(PROP_NTM ), NOMSG ) )
              {
                strcpy( pProp->szLongName, pszOrgLongName );
                strcpy( pProp->stPropHead.szPath, pData->szSearchPath );
                strcpy( pProp->stPropHead.szName, UtlSplitFnameFromPath(pProp->stPropHead.szPath) );
                UtlSplitFnameFromPath( pProp->stPropHead.szPath ); 
                UtlWriteFile( pData->szSearchPath, sizeof(PROP_NTM ), (PVOID)pProp, FALSE );
                UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
                if ( pfReserved ) *pfReserved = TRUE;
              } /* endif */
            }
            break;
        } /* endswitch */
       } /* endif */

      // release Mutex
      RELEASEMUTEX(hMutexSem);
    } /* endif */
  } /* endif */

  // cleanup
  if ( pData != NULL ) UtlAlloc( (PVOID *)&pData, 0L, 0L,NOMSG );

  // return to caller
  if ( pObjState != NULL ) *pObjState = ObjState;

  return( usRC );
} /* end of function ObjLongToShortNameEx */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ObjShortToLongName                                       |
//+----------------------------------------------------------------------------+
//|Description:       Converts the given short name to a long name and by      |
//|                   retrieving the long name from the property file          |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ              pszShortName ptr to object short name   |
//|                   PSZ              pszLongName ptr to buffer for long name |
//|                                    the buffer must have a size of          |
//|                                    MAX_LONGFILESPEC                        |
//|                   UTLCHECKOBJTYPES ObjType      type of object             |
//+----------------------------------------------------------------------------+
//|Returns:           USHORT  usRC                                             |
//|                   NO_ERROR = O.K. anything other: error code               |
//+----------------------------------------------------------------------------+
USHORT ObjShortToLongName
(
  PSZ pszShortName,                    // ptr to object short name
  PSZ pszLongName,                     // ptr to buffer for long name
  UTLCHECKOBJTYPES ObjType             // type of object
)
{
  PSZ         pszPath = NULL;          // ptr to buffer for file names
  USHORT      usRC = NO_ERROR;         // function return code

  // allocate buffer for fully qualified object name
  if ( !UtlAlloc( (PVOID *)&pszPath, 0L, (LONG)MAX_LONGPATH, ERROR_STORAGE ) )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // setup path to object's property file
  if ( usRC == NO_ERROR )
  {
    switch ( ObjType )
    {
      case FOLDER_OBJECT:
        UtlMakeEQFPath( pszPath, NULC, PROPERTY_PATH, NULL );
        strcat( pszPath, BACKSLASH_STR );
        strcat( pszPath, pszShortName );
        strcat( pszPath, EXT_FOLDER_MAIN );
        break;
      case DICT_OBJECT:
        UtlMakeEQFPath( pszPath, NULC, PROPERTY_PATH, NULL );
        strcat( pszPath, BACKSLASH_STR );
        strcat( pszPath, pszShortName );
        strcat( pszPath, EXT_OF_DICTPROP );
        break;
      case TM_OBJECT:
        UtlMakeEQFPath( pszPath, NULC, PROPERTY_PATH, NULL );
        strcat( pszPath, BACKSLASH_STR );
        strcat( pszPath, pszShortName );
        strcat( pszPath, EXT_OF_MEM );
        break;
      default:
        usRC = ERROR_INVALID_PARAMETER;
    } /* endswitch */
  } /* endif */

  // load property file
  if ( usRC == NO_ERROR )
  {
    PVOID pProp = NULL;      // ptr to object properties
    ULONG ulBytesRead;
    USHORT usDosRC = NO_ERROR;         // return code of called DOS functions
    BOOL   fOK;

    fOK = UtlLoadFileL( pszPath,        // name of file being loaded
                       &pProp,         // return pointer to loaded file
                       &ulBytesRead,   // length of loaded file
                       FALSE, FALSE );

    // for TM_OBJECT only: try lan-based property file as well
    if ( !fOK && (ObjType == TM_OBJECT) )
    {
      UtlMakeEQFPath( pszPath, NULC, PROPERTY_PATH, NULL );
      strcat( pszPath, BACKSLASH_STR );
      strcat( pszPath, pszShortName );
      strcat( pszPath, ".SLM" );
      fOK = UtlLoadFileL( pszPath,        // name of file being loaded
                         &pProp,         // return pointer to loaded file
                         &ulBytesRead,   // length of loaded file
                         FALSE, FALSE );
    }

    if ( !fOK )
    {
      usDosRC = ERROR_PROPERTY_ACCESS;
    } /* endif */

    if ( usDosRC == NO_ERROR )
    {
      PSZ pszPropName = NULL;
      PPROPFOLDER pFolProp = (PPROPFOLDER)pProp;
      PPROPDICTIONARY pDicProp = (PPROPDICTIONARY)pProp;
      PPROP_NTM pMemProp = (PPROP_NTM)pProp;

      // get position of long name in property file
      switch ( ObjType)
      {
        case FOLDER_OBJECT: pszPropName = pFolProp->szLongName; break;
        case DICT_OBJECT:   pszPropName = pDicProp->szLongName; break;
        case TM_OBJECT:     pszPropName = pMemProp->szLongName; break;
      } /* endswitch */

      if ( pszPropName != NULL )
      {
        if ( *pszPropName == EOS )
        {
          strcpy( pszLongName, pszShortName );
        }
        else
        {
          strcpy( pszLongName, pszPropName );
        } /* endif */
      } /* endif */

      UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  // cleanup
  if ( pszPath ) UtlAlloc( (PVOID *)&pszPath, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function ObjShortToLongName */

//   End of EQFOBJ00.C
//ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß
