//------------------------------------------------------------------------------
//  EQFMEM00.C - EQF Memory List Handler
//------------------------------------------------------------------------------
//  Copyright Notice:
//
//      Copyright (C) 1990-2016, International Business Machines
//      Corporation and others. All rights reserved


#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DAM

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "MemoryFactory.h"
#include "MemoryUtil.h"

// EQF.H is included by otmmemory.h
// #include <eqf.h>                  // General Translation Manager include file EQF:H 

#include <EQFDDE.H>               // batch mode definitions
#define INCL_EQFMEM_DLGIDAS
#include <EQFMEM.ID>              // Translation Memory IDs
#include "eqfcolw.id"             // column width IDs

BOOL EQFTMMaintain( PSZ );

#include <EQFTMI.H>               // Private header file of Translation Memory


static CHAR ColHdr[10][30];       // Buffer for column header texts
static CLBCOLDATA MemColTable[] =
{
{ "",        1,                    TEXT_DATA, DT_LEFT  }, // Object
{ ColHdr[1], CLB_MAX_FNAME, AUTOWIDTHTEXT_DATA,    DT_LEFT          },
{ ColHdr[2], CLB_MAX_DESCRIPTION,  TEXT_DATA, DT_LEFT  }, // Description
{ ColHdr[3], CLB_MAX_SIZE_LENGTH,  NUMERIC_DATA, DT_RIGHT }, // Size
{ ColHdr[4], CLB_MAX_DRIVE,        TEXT_DATA, DT_LEFT  }, // Drive
{ "Type",    34,                   TEXT_DATA, DT_LEFT  }, // Memory type
{ ColHdr[6], CLB_MAX_OWNER_LENGTH, TEXT_DATA, DT_LEFT  }, // Owner
{ ColHdr[7], CLB_MAX_LANG_LENGTH,  TEXT_DATA, DT_LEFT  }, // Source
{ ColHdr[8], CLB_MAX_LANG_LENGTH,  TEXT_DATA, DT_LEFT  }, // Target
{ ColHdr[9], CLB_MAX_FNAME,        TEXT_DATA, DT_LEFT  }, // Markup
{ NULL,      (CLBDATATYPE)0,       TEXT_DATA,   0      } };

static SHORT sLastUsedView[MAX_VIEW] = { MEM_NAME_IND, CLBLISTEND };
static SHORT sDefaultView[MAX_VIEW]  = { MEM_NAME_IND, CLBLISTEND };
static SHORT sNameView[MAX_VIEW]     = { MEM_NAME_IND, CLBLISTEND };
static SHORT sDetailsView[MAX_VIEW]  = { MEM_NAME_IND, MEM_DESCRIPTION_IND,
                                         CLBLISTEND };
static SHORT sSortCriteria[MAX_VIEW] = { MEM_NAME_IND, CLBLISTEND };

static CLBCTLDATA MemCLBData =
{  sizeof(CLBCTLDATA),                 // size of control structure
   10,                                 // we have 9 data columns
   1,                                  // one character space between columns
   SYSCLR_WINDOWSTATICTEXT,            // paint title in color of static text
   SYSCLR_WINDOW,                      // background is normal window background
   SYSCLR_WINDOWTEXT,                  // paint item in color of window text
   SYSCLR_WINDOW,                      // background is normal window background
   X15,                                // use X15 character as data seperator
   sLastUsedView,                      // set current (= last used) view list
   sDefaultView,                       // set default view list
   sDetailsView,                       // set user set details view list
   sNameView,                          // set view list for 'name' view option
   sSortCriteria,                      // set sort criteria list
   MemColTable };                      // set address of column definition table



MRESULT  MemListCallBack( PLISTCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
MRESULT  MemListCreate ( PLISTCOMMAREA, LPARAM );
VOID     MemListTerminate ( PLISTCOMMAREA, WPARAM );
MRESULT  MemListCommands ( PLISTCOMMAREA, HWND, USHORT );
MRESULT  MemListProcessTask ( PLISTCOMMAREA, HWND, WPARAM, LPARAM );
BOOL     MemMakeListItem( PSZ, PSZ, PBOOL, PSZ );
USHORT   MemFillListBox( HWND, PSZ, PSZ );
VOID PASCAL FAR  MemCleanUp( USHORT );

USHORT MemBatchTMOrganize( HWND hwnd, PDDEMEMORG pMemOrg );

/**********************************************************************/
/* Handler callback function for tag table list handler               */
/**********************************************************************/
MRESULT MemListHandlerCallBack
(
  PHANDLERCOMMAREA pCommArea,
  HWND             hwnd,
  WINMSG           msg,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);

  switch ( msg )
  {
    /******************************************************************/
    /* WM_CREATE: fill variables of communication area                */
    /******************************************************************/
    case WM_CREATE :
      pCommArea->pfnCallBack          = MemListCallBack;
      strcpy( pCommArea->szHandlerName, MEMORYHANDLER );
      pCommArea->sBaseClass           = clsMEMORY;
      pCommArea->sListWindowID        = ID_MEMORY_WINDOW;
      pCommArea->sListboxID           = ID_MEMWIN_LISTBOX;
      pCommArea->fUserFlag            = FALSE; /* handler may be closed */

      /****************************************************************/
      /* Define object classes to be notified for EQFN messages       */
      /****************************************************************/
      pCommArea->asNotifyClassList[0] = clsMEMORY;
      pCommArea->asNotifyClassList[1] = clsMEMORYDB;
      pCommArea->asNotifyClassList[2] = 0;       // end of list

      /****************************************************************/
      /* Define additional messages processed by the callback function*/
      /****************************************************************/
      pCommArea->asMsgsWanted[0] = WM_EQF_QUERYPROPSIZE;
      pCommArea->asMsgsWanted[1] = WM_EQF_PROCESSTASK;
      pCommArea->asMsgsWanted[2] = WM_EQF_MEMFILL_LISTBOX;
      pCommArea->asMsgsWanted[3] = WM_EQF_DDE_REQUEST;
      pCommArea->asMsgsWanted[4] = 0;       // end of list
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_OPEN:
      {
        HWND       hwndObj;
        PSZ        pszObj = (PSZ)PVOIDFROMMP2(mp2);

        /* open only the list window, that is don't open an old */
        /* organize process etc.                                */
        if ( strstr( pszObj, MEMORY_PROPERTIES_NAME ) )
        {
          if( (hwndObj = EqfQueryObject( pszObj, clsMEMORY, 0) ) != NULLHANDLE )
          {
            SETFOCUSHWND( hwndObj );
            mResult = MRFROMSHORT( TRUE );
          }
          else
          {
            mResult = WinSendMsg( hwnd, WM_EQF_CREATELISTWINDOW, mp1, mp2 );
          } /* endif */
        } /* endif */
      } /* endcase WM_EQF_OPEN */
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_CREATE:
      {
        int                   iDlgRc = 1;

        /****************************************************************/
        /* This function creates a new TM. If mp2 is not NULL then it   */
        /* is assumed that it contains the address of an X15 string     */
        /* that contains:                                               */
        /*   - name of new translation memory                           */
        /*   - name of source language                                  */
        /****************************************************************/
        mResult = MRFROMSHORT( FALSE ); /* preset to FALSE */

        /* prevent handler from closing while dialog is open */
        pCommArea->fUserFlag = TRUE;

        /* show create TM dialog */
        DIALOGBOX( QUERYACTIVEWINDOW(), MEMCREATEDLG, hResMod, ID_MEMCREATE_DLG, mp2, iDlgRc );

        /* handler may close again */
        pCommArea->fUserFlag = FALSE;

        mResult = iDlgRc;
      } /* endcase WM_EQF_CREATE */
      break;

    case  WM_EQF_DDE_REQUEST:
      WinPostMsg( EqfQueryObject( NULL, clsMEMORY, OBJ_ACTIVE ),
                  WM_EQF_DDE_REQUEST, mp1, mp2 );
      mResult = (MRESULT)TRUE;
      break;
    /*-----------------------------------------------------------------------*/
    case WM_EQF_INSERTNAMES:
      mResult = WinSendMsg( EqfQueryObject(NULL, clsMEMORY, OBJ_ACTIVE),
                            WM_EQF_INSERTNAMES, mp1, mp2 );
      break;

    /*-----------------------------------------------------------------------*/
    case WM_DESTROY:
      /****************************************************************/
      /* Nothing to do, as nothing has been allocated by the memory   */
      /* handler callback function                                    */
      /****************************************************************/
      break;

    /*-----------------------------------------------------------------------*/
    // Handle the different notification messages
    // and the message to fill the memory listbox
    // A new memory database has been created
    case WM_EQFN_CREATED:
    // The memory database has been deleted
    case WM_EQFN_DELETED:
    // This message announces property changes of a memory db
    case WM_EQFN_PROPERTIESCHANGED:
    // Fill the memory listbox
    case WM_EQF_MEMFILL_LISTBOX:
      if ( SHORT1FROMMP1(mp1) == clsMEMORYDB )
      {
        mResult = MRFROMSHORT(EqfSend2AllObjects( clsMEMORY, msg, mp1, mp2 ));
      } /* endif */
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_QUERYPROPSIZE:
      switch ( SHORT1FROMMP1(mp1) )
      {
        case PROP_CLASS_MEMORY:
        case PROP_CLASS_MEMORYDB:
        case PROP_CLASS_MEMORY_LASTUSED:
          mResult = (MRESULT)MEM_PROP_SIZE;
          break;
      } /* endswitch */
    break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_SHUTDOWN:
      //  EQF is asking whether the handler can be terminated or not
      //  this is indicated if TWBFORCE is not set.

      // A WM_EQF_QUERY is sent to all instances
      // in order to ask them if they are busy or not. A return code
      // of TRUE means busy.
      // Also the switch pIda->stIdaHead.fMustNotClose is checked and
      // if it is TRUE a process of the handler itself is active.
      // TRUE is returned to the caller if any process is running.
      if( pCommArea->fUserFlag )
      {
          mResult = (MRESULT)TRUE;
      } /* endif */
      else
      {
        // Send to all instances WM_EQF_QUERY message
        mResult = (MRESULT)EqfSend2AllObjects( clsMEMORY, WM_EQF_QUERY, 0, 0L );
      } /* endif */
      break;
    /*-----------------------------------------------------------------------*/
    case WM_EQFN_DRIVEREMOVED:
    case WM_EQFN_DRIVEADDED:
      // a drive was removed or added via configure drives dialog
      // update TM listbox
      EqfSend2AllObjects( clsMEMORY, msg, mp1, mp2 );
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_PROCESSTASK:
      switch ( SHORT1FROMMP1( mp1 ) )
      {
        case IMPORT_TASK:
        case BATCHIMPORT_TASK:
          WinPostMsg( EqfQueryObject( NULL, clsMEMORY, OBJ_ACTIVE ),
                      WM_EQF_PROCESSTASK, mp1, mp2 );
          mResult = (MRESULT) TRUE;
          break;

        case TM_QUERY_SERVER_TASK:
        case TM_SERVER_REFERENCE_TASK:
        case PID_FILE_MI_ORGANIZE:
        case CREATE_PATH:
        case QUERY_LANG_PROP_INPUT_MEM:
        case QUERY_LANG_PROP_INPUT_LANG:
        case QUERY_LANG_PAIR_INPUT_MEM:
        case QUERY_MARKUP_INPUT_MEM:
        case WM_EQF_MEMORGANIZE_ACTIVATE:
        case PID_FILE_MI_PROPERTIES:
        case PID_FILE_MI_HTMLPROPS:
        case PID_FILE_MI_RENAME:
        case PID_FILE_MI_ADDUSER:
        case PID_FILE_MI_REMOVEUSER:
        case PID_FILE_MI_LISTUSER:
		case PID_FILE_MI_UPLOADTOSERVER:
		case PID_FILE_MI_DOWNFROMSERVER:
          mResult = WinSendMsg( EqfQueryObject( NULL, clsMEMORY, OBJ_ACTIVE ),
                                WM_EQF_PROCESSTASK, mp1, mp2 );
          break;
        case RENAMEOBJECT_TASK:
          mResult = -1; // this task is not supported anymore, use MemoryFactory method renameMemory instead!
          break;
        case WM_EQF_MT_TMMERGE:
          mResult = WinSendMsg( EqfQueryObject( NULL, clsMEMORY, OBJ_ACTIVE ),
                                WM_EQF_PROCESSTASK, mp1, mp2 );
          break;
        default:
          break;
      } /* endswitch */
      break;

      case WM_EQF_READINQUE:
      {
        int iRC = 0;
        MemoryFactory *pFactory = MemoryFactory::getInstance();
        OtmMemory *pMemory = pFactory->openMemory( NULL, (PSZ)PVOIDFROMMP2(mp2), NONEXCLUSIVE, &iRC );
        if(pMemory != NULL)
        {
          // use an empty proposal to trigger updating
          // also need to modify of the putProposal in EqfSharedMemory
          //::MessageBox( NULL, "receive message", "TranslationManager", MB_CANCEL );
          OtmProposal proposal;
          pMemory->putProposal(proposal);
          pFactory->closeMemory(pMemory);
        }
        else
        {
          // if no memory, maybe the memory is using, can't be accessed now
          // So we have to do something here
        }
        mResult = (iRC==0)?TRUE:FALSE;
      }
      break;

      case WM_EQF_ADDMEMTOLIST:
      {
        MemoryFactory *pFactory = MemoryFactory::getInstance();
        if(pFactory != NULL)
        {  
            int res = pFactory->addMemoryToList( (PSZ)PVOIDFROMMP2(mp2), CHAR1FROMMP1(mp1) );
            if(res == 0)
                EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT( clsMEMORYDB ), mp2);
        }
      }
      break;

      case WM_EQF_REMOVEMEMFROMLIST:
      {
         MemoryFactory *pFactory = MemoryFactory::getInstance();
         if(pFactory != NULL)
         {  
             int res = pFactory->removeMemoryFromList((PSZ)PVOIDFROMMP2(mp2));
             if(res == 0)
                 EqfSend2AllHandlers( WM_EQFN_DELETED, MP1FROMSHORT( clsMEMORYDB ), mp2);
         }
      }
      break;
    /*-----------------------------------------------------------------------*/
    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function MemListHandlerCallBack */


/**********************************************************************/
/* List instance callback function for memory list window             */
/**********************************************************************/
MRESULT MemListCallBack
(
  PLISTCOMMAREA    pCommArea,
  HWND             hwnd,
  WINMSG           msg,
  WPARAM           mp1,
  LPARAM           mp2
)
{
  MRESULT          mResult = MRFROMSHORT(FALSE);
  PMEM_IDA         pIDA;            // Pointer to the Main Memory data area

  switch ( msg )
  {
    case WM_CREATE :
      mResult = MemListCreate( pCommArea, mp2 );
      break;

    /*-----------------------------------------------------------------------*/
    case WM_CLOSE :
    case WM_EQF_TERMINATE :
      MemListTerminate ( pCommArea, mp1 );
      break;

    /*-----------------------------------------------------------------------*/
    case WM_DESTROY:
      /****************************************************************/
      /* Free all resource allocated by list instance callback        */
      /* function                                                     */
      /****************************************************************/
      UtlAlloc( (PVOID *)&(pCommArea->pUserIDA), 0L, 0L, NOMSG );
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_INITIALIZE:
      /****************************************************************/
      /* Fill column listbox                                          */
      /****************************************************************/
      pIDA = (PMEM_IDA)pCommArea->pUserIDA;
      MemFillListBox( pCommArea->hwndLB, pCommArea->szBuffer,
                      pIDA->szNAString );
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_BUILDITEMTEXT :
      /****************************************************************/
      /* Setup item text for the object passed in mp2 parameter       */
      /****************************************************************/
      {
        PSZ        pszObj = (PSZ)PVOIDFROMMP2(mp2);   // ptr to object name
        BOOL       fOK = FALSE;
        BOOL       fEnabled;
        pIDA = (PMEM_IDA)pCommArea->pUserIDA;

        fOK = MemMakeListItem( pszObj, pCommArea->szBuffer, &fEnabled,
                               pIDA->szNAString );
        mResult = MRFROMSHORT( fOK );
      }
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_INITMENU:
    case WM_INITMENU:
      {
        SHORT  sItem;              // listbox item index
        int    iSelItems;          // number of selected items
        HMENU  hMenu;
        UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );

        UtlMenuEnableItem( PID_VIEW_MI_NAMES );
        UtlMenuEnableItem( PID_VIEW_MI_DETAILSDLG );
        UtlMenuEnableItem( PID_VIEW_MI_DETAILS );
        UtlMenuEnableItem( PID_FILE_MI_SELECTALL );
        UtlMenuEnableItem( PID_FILE_MI_DESELECTALL );
        UtlMenuEnableItem( PID_VIEW_MI_SORT );
        UtlMenuEnableItem( PID_VIEW_MI_SOME );
        UtlMenuEnableItem( PID_VIEW_MI_ALL );
        UtlMenuEnableItem( PID_UTILS_MI_CONNECT );
        UtlMenuEnableItem( PID_FILE_MI_NEW );
        UtlMenuEnableItem( PID_FILE_MI_IMPORT );
        UtlMenuEnableItem( PID_FILE_MI_PRINTLIST );
        iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
        sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );

        hMenu = GetSubMenu(GETMENU((HWND)UtlQueryULong( QL_TWBFRAME )), PID_TWBM_SM_FILE );
        // firstly delete the add/remove/list user menu item if they exist
        if( GetMenuString(hMenu,PID_FILE_MI_ADDUSER,NULL,0,MF_BYCOMMAND) != 0 )
        {
          DeleteMenu( hMenu, PID_FILE_MI_ADDUSER, MF_BYCOMMAND );
          DeleteMenu( hMenu, PID_FILE_MI_REMOVEUSER, MF_BYCOMMAND );
          DeleteMenu( hMenu, PID_FILE_MI_LISTUSER, MF_BYCOMMAND );
		  DeleteMenu( hMenu, PID_FILE_MI_UPLOADTOSERVER, MF_BYCOMMAND );
		  DeleteMenu( hMenu, PID_FILE_MI_DOWNFROMSERVER, MF_BYCOMMAND );
        }
		
        if ( iSelItems > 1 )
        {
          // multiple selection valid for some operations only
          UtlMenuEnableItem( PID_FILE_MI_DELETE );
          UtlMenuEnableItem( PID_FILE_MI_EXPORT );
          UtlMenuEnableItem( PID_FILE_MI_ORGANIZE );
        }
        else
        if ( sItem != LIT_NONE )
        {
          BOOL  fEnabled;

          UtlMenuEnableItem( PID_FILE_MI_DELETE );
          fEnabled = (BOOL)WinSendMsg( pCommArea->hwndLB,
                                       LM_EQF_QUERYITEMSTATE,
                                       MP1FROMSHORT( sItem ),
                                       NULL );
          if ( fEnabled )
          {
            // get memory name from memory list window
            // to check whether it's shared
            static char szBuf[MAX_LONGPATH*2+1] = {0};
            static char szMem[MAX_LONGPATH*2+1] = {0};
            BOOL isSharedMem = FALSE;
            MemoryFactory *pFactory = MemoryFactory::getInstance();
            SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)szBuf);
            strncpy( szMem, UtlParseX15( szBuf, MEM_NAME_IND),MAX_EQF_PATH*2);
            isSharedMem = pFactory->isSharedMemory(szMem, NULL);

            UtlMenuEnableItem( PID_FILE_MI_OPEN );
            UtlMenuEnableItem( PID_FILE_MI_EXPORT );
            UtlMenuEnableItem( PID_FILE_MI_ORGANIZE );
            UtlMenuEnableItem( PID_FILE_MI_MERGE );
            UtlMenuEnableItem( PID_FILE_MI_PROPERTIES );
            UtlMenuEnableItem( PID_FILE_MI_HTMLPROPS );
            // disable rename menu item
            if(!isSharedMem)
			{
              UtlMenuEnableItem( PID_FILE_MI_RENAME );
			}
            else
            {
              // add menu items, add/remove/list users
              if( GetMenuString(hMenu,PID_FILE_MI_ADDUSER,NULL,0,MF_BYCOMMAND) == 0)
              {
                InsertMenu(hMenu, PID_FILE_MI_EXIT, MF_BYCOMMAND|MF_STRING, PID_FILE_MI_ADDUSER, "Add User");
                InsertMenu(hMenu, PID_FILE_MI_EXIT, MF_BYCOMMAND|MF_STRING, PID_FILE_MI_REMOVEUSER, "Remove User");
                InsertMenu(hMenu, PID_FILE_MI_EXIT, MF_BYCOMMAND|MF_STRING, PID_FILE_MI_LISTUSER, "List User");
				InsertMenu(hMenu, PID_FILE_MI_EXIT, MF_BYCOMMAND|MF_STRING, PID_FILE_MI_UPLOADTOSERVER, "Batch upload");
				InsertMenu(hMenu, PID_FILE_MI_EXIT, MF_BYCOMMAND|MF_STRING, PID_FILE_MI_DOWNFROMSERVER, "Batch download");
				
              }
              else
              {
                UtlMenuEnableItem( PID_FILE_MI_ADDUSER );
                UtlMenuEnableItem( PID_FILE_MI_REMOVEUSER );
                UtlMenuEnableItem( PID_FILE_MI_LISTUSER );
              }
            }
          } /* endif */
        }/* endif */
      }
      break;

    case WM_EQF_TOOLBAR_ENABLED:
      switch ( mp1 )
      {
        /**************************************************************/
        /* check for items to be enabled ..                           */
        /**************************************************************/
        case PID_VIEW_MI_NAMES:
        case PID_VIEW_MI_DETAILSDLG:
        case PID_VIEW_MI_DETAILS:
        case PID_UTILS_MI_CONNECT:
        case PID_FILE_MI_NEW:
        case PID_FILE_MI_IMPORT:
        case PID_FILE_MI_PRINTLIST:
        case PID_VIEW_MI_SORT:
        case PID_VIEW_MI_SOME:
        case PID_VIEW_MI_ALL:
        case PID_FILE_MI_SELECTALL:
        case PID_FILE_MI_DESELECTALL:
          mResult = TRUE;
          break;
        case PID_FILE_MI_DELETE:
        case PID_FILE_MI_EXPORT:
        case PID_FILE_MI_ORGANIZE:
          {
            int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );
            mResult = ( iSelItems >= 1 );
          }
          break;
        case PID_FILE_MI_OPEN:
        case PID_FILE_MI_MERGE:
        case PID_FILE_MI_PROPERTIES:
        case PID_FILE_MI_HTMLPROPS:
        case PID_FILE_MI_RENAME:
        case PID_FILE_MI_ADDUSER:
        case PID_FILE_MI_REMOVEUSER:
        case PID_FILE_MI_LISTUSER:
		case PID_FILE_MI_UPLOADTOSERVER:
		case PID_FILE_MI_DOWNFROMSERVER:
        {
          SHORT sItem = QUERYSELECTIONHWND( pCommArea->hwndLB );
          if ( sItem != LIT_NONE )
          {
            mResult = (BOOL)WinSendMsg( pCommArea->hwndLB,
                                        LM_EQF_QUERYITEMSTATE,
                                        MP1FROMSHORT( sItem ),
                                        NULL );
          }/* endif */
        }
        break;
        default:
          break;
      } /* endswitch */
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQFN_CREATED:             // a TM has been created
    case WM_EQFN_DELETED:             // a TM has been deleted
    case WM_EQFN_PROPERTIESCHANGED:   // properties of a TM have been changed
      if ( SHORT1FROMMP1( mp1 ) == clsMEMORYDB )
      {
        WinPostMsg( hwnd, WM_EQF_MEMFILL_LISTBOX, NULL, NULL );
      } /* endif */
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_QUERY:
      // if a process is running, return TRUE
      pIDA = (PMEM_IDA)pCommArea->pUserIDA;
      if ( pIDA )
      {
        if ( pIDA->sRunningProcesses )
        {
          mResult = (MRESULT) TRUE;
        } /* endif */
      } /* endif */
      break;

    /*-----------------------------------------------------------------------*/
    case  WM_EQF_DDE_REQUEST:
      /************************************************************/
      /*     mp1:  (DDETASK) Task                                 */
      /*     mp2:  (PVOID) pTaskIda                               */
      /************************************************************/
      // GQ 2015/02/10: deactivated obsolete code
      //switch ( SHORT1FROMMP1( mp1 ) )
      //{
      //  case  TASK_MEMCRT:
      //    {
      //      PDDEMEMCRT pMemCrt = (PDDEMEMCRT)PVOIDFROMMP2(mp2);
      //      MemBatchTMCreate( hwnd, pMemCrt );
      //    }
      //    break;
      //  case  TASK_MEMEXP:
      //    {
      //      PDDEMEMEXP pMemExp = (PDDEMEMEXP)PVOIDFROMMP2(mp2);
      //      MemBatchTMExport( hwnd, pMemExp );
      //    }
      //    break;
      //  case  TASK_MEMIMP:
      //    {
      //      PDDEMEMIMP pMemImp = (PDDEMEMIMP)PVOIDFROMMP2(mp2);
      //      MemBatchTMImport( hwnd, pMemImp );
      //    }
      //    break;
      //  case  TASK_MEMDEL:
      //    {
      //      PDDEMEMDEL pMemDel = (PDDEMEMDEL)PVOIDFROMMP2(mp2);
      //      MemBatchTMDelete( hwnd, pMemDel );
      //    }
      //    break;
      //  case  TASK_MEMORG:
      //    {
      //      PDDEMEMORG pMemOrg = (PDDEMEMORG)PVOIDFROMMP2(mp2);
      //      MemBatchTMOrganize( hwnd, pMemOrg );
      //    }
      //    break;
      //  default :
      //    break;
      //} /* endswitch */
        break;
    /*-----------------------------------------------------------------------*/
    case WM_EQF_COMMAND:
    case WM_COMMAND:
      mResult = MRFROMSHORT( TRUE );   // default return code for COMMAND msgs
      switch ( SHORT1FROMMP1(mp1) )
      {
        case PID_FILE_MI_NEW:
          EqfSend2Handler (MEMORYHANDLER, WM_EQF_CREATE, 0, 0L);
          break;
        case PID_FILE_MI_OPEN:
        case PID_FILE_MI_IMPORT:
        case PID_FILE_MI_EXPORT:
        case PID_FILE_MI_DELETE:
        case PID_FILE_MI_MERGE:
        case PID_FILE_MI_ORGANIZE:
        case PID_FILE_MI_PROPERTIES:
        case PID_FILE_MI_HTMLPROPS:
        case PID_FILE_MI_RENAME:
        case PID_FILE_MI_ADDUSER:
        case PID_FILE_MI_REMOVEUSER:
        case PID_FILE_MI_LISTUSER:
		case PID_FILE_MI_UPLOADTOSERVER:
		case PID_FILE_MI_DOWNFROMSERVER:
          mResult = (MRESULT) MemListCommands( pCommArea, hwnd,
                                               SHORT1FROMMP1( mp1 ) );
          break;
        case PID_FILE_MI_PRINTLIST:
          // pass message to column listbox control
          WinSendMsg( pCommArea->hwndLB, msg, mp1, mp2 );
          break;
      } /* endswitch */
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_INSERTNAMES:
      // This function inserts the names of all requested objects
      // into a list box. mp1 should contain the list box handle
      // and mp2 a pointer to the object name. The following object names
      // are supported:
      //    MEMORY_ALL           - inserts all available memory databases
      //    NULL                 - inserts all available memory databases
      //    FORMAT_TABLE_ALL     - Markup language
      //    EXCLUSION_LIST_ALL   - Word list, Noise list
      //    SOURCE_LANGUAGES_ALL - all available source languages
      //    TARGET_LANGUAGES_ALL - all available target languages
      // The return code contains the number of loaded items or
      // FALSE (0) if no items where loaded.
      {
        PSZ     pszObject;

        pszObject = (PSZ)PVOIDFROMMP2( mp2 );

        if (  pszObject == NULL )
        {
          pszObject = MEMORY_ALL;
        } /* endif */

        if ( !strcmpi( pszObject, MEMORY_ALL )  )
        {
          mResult = (MRESULT)FillMemoryListBox( pCommArea, mp1, mp2 );
        }
        else
        {
          USHORT      usWork = 0;

          if ( !strcmpi( pszObject, FORMAT_TABLE_ALL ) )
          {
            usWork = FORMAT_TABLE;
          } /* endif */
          if ( !strcmpi( pszObject, EXCLUSION_LIST_ALL ) )
          {
            usWork = EXCLUSION_LIST;
          } /* endif */
          if ( !strcmpi( pszObject, SOURCE_LANGUAGES_ALL ) )
          {
            usWork = SOURCE_LANGUAGES;
          } /* endif */
          if ( !strcmpi( pszObject, TARGET_LANGUAGES_ALL ) )
          {
            usWork = TARGET_LANGUAGES;
          } /* endif */

          mResult = (MRESULT)MemFillTableLB( (HWND)mp1,
                                             usWork,
                                             NULL );
        } /* endif */
      }
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_PROCESSTASK:
      mResult = MemListProcessTask( pCommArea, hwnd, mp1, mp2 );
      break;

    /*-----------------------------------------------------------------------*/
    case WM_EQF_MEMFILL_LISTBOX:
    case WM_EQFN_DRIVEREMOVED:                                      /*@89A*/
    case WM_EQFN_DRIVEADDED:                                        /*@89A*/
      pIDA = (PMEM_IDA)pCommArea->pUserIDA;
      mResult = MRFROMSHORT( MemFillListBox( pCommArea->hwndLB,
                                             pCommArea->szBuffer,
                                             pIDA->szNAString ));
      break;

    /*-----------------------------------------------------------------------*/
    default:
      break;
  } /* endswitch */
  return( mResult );
} /* end of function MemListCallBack */


/*****************************************************************************/
/* Process WM_CREATE message for MemListCallBack                             */
/*****************************************************************************/
MRESULT MemListCreate
(
  PLISTCOMMAREA    pCommArea,
  LPARAM           mp2
)
{
  BOOL            fOK = TRUE;      // initialisation is O.K. flag
  EQFINFO         ErrorInfo;       // error info of property handler
  PPROPMEMORY     pPropMem = NULL; // ptr to Memory properties
  PSZ             pReplAddr[2];
  MRESULT         mResult = MRFROMSHORT(FALSE);
  PMEM_IDA        pIDA;            // Pointer to the Main Memory data area


  mp2;                             // get rid of compiler warnings

  // Allocate storage for _MEM_IDA.
  fOK = UtlAlloc( (PVOID *)&pIDA, 0L, (LONG)sizeof( MEM_IDA ),
                  ERROR_STORAGE );
  if ( fOK )
  {
    pCommArea->pUserIDA = pIDA;
  } /* endif */

  /* copy object name to IDA */
  strcpy( pIDA->stIdaHead.szObjName, pCommArea->szObjName);
  pIDA->stIdaHead.pszObjName = pIDA->stIdaHead.szObjName;

  /* copy system path to szTemp */
  UtlMakeEQFPath( pIDA->szTemp, NULC, SYSTEM_PATH, NULL );

  //--- access or create properties ---
  pIDA->stIdaHead.hProp = OpenProperties( pCommArea->szObjName, NULL,
                                          PROP_ACCESS_READ, &ErrorInfo );
  if( !pIDA->stIdaHead.hProp )
  { //--- properties access error, let's try to create new ones ... ---
    pIDA->stIdaHead.hProp = CreateProperties( pCommArea->szObjName, NULL,
                                              PROP_CLASS_MEMORY,
                                              &ErrorInfo );
    // issue an error message if properties could not be created
    if ( !pIDA->stIdaHead.hProp )
    {
      switch ( (USHORT)ErrorInfo )
      {
        case Err_NoDiskSpace :
          {
            CHAR  szDrive[MAX_DRIVE];

            strncpy( szDrive, pIDA->szTemp, 2 );
            szDrive[2] = NULC;
            pReplAddr[0] = szDrive;
            UtlError(  ERROR_DISK_IS_FULL, MB_CANCEL, 1,
                       &pReplAddr[0], EQF_ERROR );
          }
          break;
        default :
          // If ErrorInfo is Err_NoStorage then a message has
          // been issued by CreateProperties already.
          if ( ErrorInfo != Err_NoStorage )
          {
            // Issue error message:The properties for
            // the new object %1 could not be created.
            pReplAddr[0] = pIDA->stIdaHead.szObjName;
            UtlError( ERROR_CREATE_PROPERTIES, MB_CANCEL, 1,
                      &pReplAddr[0], EQF_ERROR );
          } /* endif */
      } /* endswitch */
      fOK = FALSE;
    } /* endif */

    if ( fOK )
    {
      RECTL   RectParent;

      // Address the property area
      pPropMem = (PPROPMEMORY)MakePropPtrFromHnd( pIDA->stIdaHead.hProp );

      // Set the memory database window position, size, and
      // flags for the first time
      WinQueryWindowRect( EqfQueryTwbClient(), &RectParent );

      pPropMem->Swp.x = (SHORT)((RECTL_XRIGHT(RectParent) -  RECTL_XLEFT(RectParent)) / 3 );
      pPropMem->Swp.y = (SHORT)(RECTL_YBOTTOM(RectParent) + (RECTL_YTOP(RectParent) / 10));
      pPropMem->Swp.cx = (SHORT)((RECTL_XRIGHT(RectParent) -  RECTL_XLEFT(RectParent)) / 2 );
      pPropMem->Swp.cy = (SHORT)((RECTL_YTOP(RectParent) - RECTL_YBOTTOM(RectParent)) / 2);
      pPropMem->Swp.fs =  EQF_SWP_RESTORE | EQF_SWP_SHOW | EQF_SWP_SIZE | EQF_SWP_MOVE;

      memcpy( &(pCommArea->swpSizePos), &(pPropMem->Swp), sizeof(EQF_SWP) );

      // Write the properties
      if ( SaveProperties( pIDA->stIdaHead.hProp, &ErrorInfo) )
      {
        pReplAddr[0] = pIDA->stIdaHead.szObjName;
        UtlError( ERROR_WRITE_PROPERTIES, MB_CANCEL, 1,
                  &pReplAddr[0], EQF_ERROR );
        fOK = FALSE;
      } /* endif */

      // Set properties to R/O
      ResetPropAccess( pIDA->stIdaHead.hProp, PROP_ACCESS_WRITE );
    } /* endif */
  }
  else
  {
    // properties could be opened, address property area
    pPropMem = (PPROPMEMORY)MakePropPtrFromHnd( pIDA->stIdaHead.hProp );
  } /* endif */

  //--- access or create last used properties ---
  if ( fOK )
  {
    HPROP   hPropLastUsed;
    PSZ     pszLUPropName = MEMORYLU_PROPERTIES_NAME;

    // Try to open the last used properties
    hPropLastUsed = OpenProperties( pszLUPropName, pIDA->szTemp,
                                    PROP_ACCESS_READ,
                                    &ErrorInfo );
    if( !hPropLastUsed )
    {

      // Last used properties don't exist but will be created now
      hPropLastUsed = CreateProperties( pszLUPropName, pIDA->szTemp,
                                        PROP_CLASS_MEMORY_LASTUSED,
                                        &ErrorInfo );
      // Issue an error message if the properties could not be created
      if( !hPropLastUsed )
      {
        switch ( (USHORT)ErrorInfo )
        {
          case Err_NoDiskSpace :
            {
              CHAR  szDrive[MAX_DRIVE];

              strncpy( szDrive, pIDA->szTemp, 2 );
              szDrive[2] = NULC;
              pReplAddr[0] = szDrive;
              UtlError(  ERROR_DISK_IS_FULL, MB_CANCEL, 1,
                         &pReplAddr[0], EQF_ERROR );
            }
            break;
          default :
            // If ErrorInfo is Err_NoStorage then a message has
            // been issued by CreateProperties already.
            if ( ErrorInfo != Err_NoStorage )
            {
              pReplAddr[0] = pszLUPropName;
              UtlError( ERROR_WRITE_PROPERTIES, MB_CANCEL,
                        1, &pReplAddr[0], EQF_ERROR );
            } /* endif */
        } /* endswitch */
        fOK = FALSE;
      } /* endif */
    } /* endif */

    // Close the last used properties if they are open
    if( hPropLastUsed )
    {
      CloseProperties( hPropLastUsed, PROP_QUIT, &ErrorInfo);
    } /* endif */
  } /* endif */

  /**************************************************************/
  /* Load column listbox title strings                          */
  /**************************************************************/
  if ( fOK  )
  {
    LOADSTRING( NULLHANDLE, hResMod, MEM_NAME_H,  ColHdr[MEM_NAME_IND] );
    UtlLoadWidth( NULLHANDLE, hResMod, MEM_NAME_COLWIDTH,
                  &(MemColTable[MEM_NAME_IND].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, MEM_DESCRIPTION_H, ColHdr[MEM_DESCRIPTION_IND] );
    UtlLoadWidth( NULLHANDLE, hResMod, MEM_DESCRIPTION_COLWIDTH,
                  &(MemColTable[MEM_DESCRIPTION_IND].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, MEM_SIZE_H,  ColHdr[MEM_SIZE_IND] );
    UtlLoadWidth( NULLHANDLE, hResMod, MEM_SIZE_COLWIDTH,
                  &(MemColTable[MEM_SIZE_IND].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, MEM_DRIVE_H, ColHdr[MEM_DRIVE_IND] );
    UtlLoadWidth( NULLHANDLE, hResMod, MEM_DRIVE_COLWIDTH,
                  &(MemColTable[MEM_DRIVE_IND].usWidth) );
    /******************************************************************/
    /* Supress server column under windows                            */
    /******************************************************************/
    LOADSTRING( NULLHANDLE, hResMod, MEM_OWNER_H, ColHdr[MEM_OWNER_IND] );
    UtlLoadWidth( NULLHANDLE, hResMod, MEM_OWNER_COLWIDTH,
                  &(MemColTable[MEM_OWNER_IND].usWidth) );
    LOADSTRING( NULLHANDLE, hResMod, MEM_SOURCE_H, ColHdr[MEM_SOURCELNG_IND] );
    UtlLoadWidth( NULLHANDLE, hResMod, MEM_SOURCE_COLWIDTH,
                  &(MemColTable[MEM_SOURCELNG_IND].usWidth) );
    /* load string for N/A text (e.g. server name not available) */
    LOADSTRING( NULLHANDLE, hResMod, MEM_FILESIZE_NA_TEXT, pIDA->szNAString );
  } /* endif */

  /**************************************************************/
  /* Set column listbox view lists                              */
  /**************************************************************/
  if ( fOK  )
  {
    memcpy( pCommArea->asCurView,
            (pPropMem->sLastUsedViewList[0] != 0) ?
                       pPropMem->sLastUsedViewList :
                       sLastUsedView, sizeof( pCommArea->asCurView ) );
    MemCLBData.psLastUsedViewList = pCommArea->asCurView;

    memcpy( pCommArea->asDetailsView,
            (pPropMem->sDetailsViewList[0] != 0) ?
                       pPropMem->sDetailsViewList :
                       sDetailsView, sizeof( pCommArea->asDetailsView ) );
    MemCLBData.psDetailsViewList = pCommArea->asDetailsView;

    memcpy( pCommArea->asSortList,
            (pPropMem->sSortList[0] != 0) ?
                       pPropMem->sSortList :
                       sSortCriteria, sizeof( pCommArea->asSortList ) );
    MemCLBData.psSortList = pCommArea->asSortList;

    memcpy( &pCommArea->Filter, &pPropMem->Filter,
            sizeof( pCommArea->Filter ) );
    MemCLBData.pFilter = &pCommArea->Filter;


  {
      int i=0;
    for (i=0;pCommArea->asCurView[i]>0;i++)
    {
            int index = pCommArea->asCurView[i];
      //MemColTable[index].usWidth = pPropMem->sLastUsedViewWidth[i];
            if (pPropMem->sLastUsedViewWidth[i] > 0)
            {
                MemColTable[index].usWidth = pPropMem->sLastUsedViewWidth[i];
            }
            else
            {
                pPropMem->sLastUsedViewWidth[i] = MemColTable[index].usWidth;
            }
    }
        memcpy( pCommArea->asCurViewWidth, (pPropMem->sLastUsedViewWidth),
                sizeof(pCommArea->asCurViewWidth) );
  }

  } /* endif */

  /****************************************************************/
  /* supply all information required to create a memory list      */
  /****************************************************************/
  if ( fOK )
  {
    pCommArea->sListObjClass  = clsMEMORY;
    LOADSTRING( NULLHANDLE, hResMod, MEM_TITLE, pCommArea->szTitle );
    pCommArea->hIcon          = (HPOINTER) UtlQueryULong(QL_MEMICON); //hiconMEM;
    pCommArea->fNoClose       = TRUE;
    pCommArea->sObjNameIndex  = MEM_OBJECT_IND;
    pCommArea->sNameIndex     = MEM_NAME_IND;
    pCommArea->sListWindowID  = ID_MEMORY_WINDOW;
    pCommArea->sListboxID     = ID_MEMWIN_LISTBOX;
    pCommArea->sPopupMenuID   = ID_MEM_POPUP;
    pCommArea->sGreyedPopupMenuID   = ID_MEM_POPUP;
    pCommArea->sNoSelPopupMenuID = ID_MEM_POPUP_NOSEL;
    pCommArea->pColData       = &MemCLBData;
    pCommArea->fMultipleSel   = TRUE;
    pCommArea->sMultPopupMenuID = ID_MEM_POPUP_MULTSEL;
    pCommArea->sDefaultAction = PID_FILE_MI_OPEN;
    memcpy( &(pCommArea->swpSizePos), &(pPropMem->Swp), sizeof(EQF_SWP) );
    pCommArea->sItemClass     = clsMEMORYDB;
    pCommArea->sItemPropClass = PROP_CLASS_MEMORY;
    pCommArea->asMsgsWanted[0] = WM_EQF_PROCESSTASK;
    pCommArea->asMsgsWanted[1] = WM_EQF_DDE_REQUEST;
    pCommArea->asMsgsWanted[2] = 0;        // ends list of messages
  } /* endif */

  /**************************************************************/
  /* In case of errors set error return code                    */
  /**************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pIDA, 0L, 0L, NOMSG);
    pCommArea->pUserIDA = NULL;
    mResult = MRFROMSHORT(DO_NOT_CREATE);
  } /* endif */

  return( mResult );
} /* end of function MemListCreate */


/*****************************************************************************/
/* Process WM_EQF_TERMINATE message for MemListCallBack                      */
/*****************************************************************************/
VOID MemListTerminate
(
  PLISTCOMMAREA    pCommArea,
  WPARAM           mp1
)
{
  EQFINFO         ErrorInfo;       // error info of property handler
  PPROPMEMORY     pPropMem;        // ptr to Memory properties
  PMEM_IDA        pIDA;                // Pointer to the Main Memory data area

  /* get address of IDA */
  pIDA = (PMEM_IDA)pCommArea->pUserIDA;

  //  if the TWBSAVE bit is on then save the instance properties first
  if ( SHORT1FROMMP1( mp1 ) & TWBSAVE )
  {
    if ( SetPropAccess( pIDA->stIdaHead.hProp, PROP_ACCESS_WRITE ) )
    {
      pPropMem = (PPROPMEMORY)MakePropPtrFromHnd( pIDA->stIdaHead.hProp );
      memcpy( &(pPropMem->Swp), &(pCommArea->swpSizePos), sizeof(EQF_SWP) );
      memcpy( (pPropMem->sLastUsedViewList), (pCommArea->asCurView),
              sizeof( pCommArea->asCurView ) );

      memcpy( (pPropMem->sLastUsedViewWidth), (pCommArea->asCurViewWidth),
              sizeof( pPropMem->sLastUsedViewWidth ));

    memcpy( (pPropMem->sDetailsViewList), (pCommArea->asDetailsView),
              sizeof( pCommArea->asDetailsView ) );
      memcpy( (pPropMem->sSortList), (pCommArea->asSortList),
              sizeof( pCommArea->asSortList ) );
      memcpy( &(pPropMem->Filter), &(pCommArea->Filter),
              sizeof( pCommArea->Filter ) );
      SaveProperties( pIDA->stIdaHead.hProp, &ErrorInfo );
    } /* endif */
  } /* endif */

  CloseProperties( pIDA->stIdaHead.hProp, PROP_QUIT, &ErrorInfo );
} /* end of function MemListTerminate */

/*****************************************************************************/
/* Process WM_COMMAND message for certain TM specific commands               */
/*****************************************************************************/
MRESULT MemListCommands
(
  PLISTCOMMAREA pCommArea,
  HWND          hwnd,                       // Window handle
  USHORT        usMenuItem                  // action bar item
)
{
  BOOL          fOK = TRUE;                 // default return value
  PMEM_IDA      pIDA;                       // Pointer to the Main Memory data area
  BOOL          fEnabled = FALSE;           // item enabled ?
  OtmMemory     *pMemory = NULL;            // ptr to opened memory
  USHORT        usTmtRc;                    // Memory database return code
  SHORT         sMemSelected = LIT_NONE;
  SHORT         sRC;                        // value used for symbol checking..
  PSZ           pszNameList = NULL;         // list of selected TMs (when more than one is selected)
  PSZ           pszActiveName = NULL;       // points to active name in name list
  pIDA = (PMEM_IDA)pCommArea->pUserIDA;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  if ( pIDA )
  {
    // clear name field
    pIDA->szMemName[0] = NULC;

    // get number of selected TMs and build name list
    {
      SHORT sItem = LIT_FIRST;
      int iSelItems = SendMessage( pCommArea->hwndLB, LB_GETSELCOUNT, 0, 0 );

      if ( iSelItems > 1 )
      {
        // allocate buffer for the object name list
        fOK = UtlAlloc( (PVOID *)&pszNameList, 0, (MAX_LONGFILESPEC*2*iSelItems)+10,
                        ERROR_STORAGE );

        // fill-in names of selected TMs
        if ( fOK )
        {
          PSZ pszCurPos = pszNameList;
          sItem = LIT_FIRST;

          while ( iSelItems )
          {
            sItem = QUERYNEXTSELECTIONHWND( pCommArea->hwndLB, sItem );
            if ( sItem >= 0 )
            {
              SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItem, (LPARAM)pCommArea->szBuffer );
              strcpy( pszCurPos, UtlParseX15( pCommArea->szBuffer, MEM_OBJECT_IND ) );
              pszCurPos += strlen( pszCurPos ) + 1;
            } /* endif */
            iSelItems--;
          } /* endwhile */
          *pszCurPos = EOS;          // terminate list
          pszActiveName  = pszNameList;
        } /* endif */
      } /* endif */
    }
    if ( pIDA->szProcessTaskWithObject[0] == NULC )
    {
      /* no object name to process specified yet, read it */
      /* get current selection */
      sMemSelected = QUERYSELECTIONHWND( pCommArea->hwndLB );

      if ((sMemSelected != LIT_NONE) && (sMemSelected != LIT_ERROR) )
      {
        /* now check if item is disabled (greyed out) */
        fEnabled = (BOOL)WinSendMsg( pCommArea->hwndLB,
                                     LM_EQF_QUERYITEMSTATE,
                                     MP1FROMSHORT( sMemSelected ),
                                     NULL );
        // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//      QUERYITEMTEXTHWNDL( pCommArea->hwndLB, sMemSelected, pIDA->szTemp  );
        SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sMemSelected, (LPARAM)pIDA->szTemp );
        strcpy( pIDA->szMemObjName, UtlParseX15( pIDA->szTemp, MEM_OBJECT_IND ) );
        strcpy( pIDA->szMemName, UtlParseX15( pIDA->szTemp, MEM_NAME_IND ) );
        ANSITOOEM( pIDA->szMemName );
      }
      else
      {
        // it is allowed, that nothing is selected if we want to import
        fOK = (usMenuItem == PID_FILE_MI_IMPORT);
      } /* endif */
    }
    else
    {
      /* object name to process is specified */
      strcpy( pIDA->szMemObjName, pIDA->szProcessTaskWithObject );
      // clear task object name to avoid interference with other tasks
      pIDA->szProcessTaskWithObject[0] = NULC;
    } /* endif */

    if ( pIDA->szMemObjName[0] == NULC )
    {
      // only import is valid when no memory has been selected
      if ( usMenuItem == PID_FILE_MI_IMPORT )
      {
        pIDA->szMemName[0] = EOS;  
        fOK = MemGUIImportMemory( pCommArea, pIDA->szMemName, pIDA, hwnd );
      }
    }
    else
    {
      pFactory->splitObjName( pIDA->szMemObjName, pIDA->szPluginName, sizeof(pIDA->szPluginName), pIDA->szMemName, sizeof(pIDA->szMemName) );
      switch ( usMenuItem )
      {
        case PID_FILE_MI_EXPORT:
          {
            int iRC = 0;
            pMemory = pFactory->openMemory( pIDA->szPluginName, pIDA->szMemName, EXCLUSIVE, &iRC );
            if ( pMemory == NULL )
            {
              usTmtRc = (USHORT)iRC;
              pFactory->showLastError( pIDA->szPluginName, pIDA->szMemName, NULL, hwnd );
            }
            else
            {
              fOK = MemGUIExportMemory( pMemory, pCommArea, pIDA->szMemName, pszNameList, pIDA, hwnd );
              pszNameList = NULL;
            } /* endif */
          }
          break;

        case PID_FILE_MI_OPEN:
          {
            int iRC = 0;
            pMemory = pFactory->openMemory( pIDA->szPluginName, pIDA->szMemName, EXCLUSIVE, &iRC );
            if ( pMemory == NULL )
            {
              usTmtRc = (USHORT)iRC;
              pFactory->showLastError( pIDA->szPluginName, pIDA->szMemName, NULL, hwnd );
            }
            else
            {
              pFactory->closeMemory( pMemory );
              EQFTMMaintain( pIDA->szMemName );
            } /* endif */
          }
          break;

        case PID_FILE_MI_RENAME:
          {
            int iRC = 0;
            pMemory = pFactory->openMemory( pIDA->szPluginName, pIDA->szMemName, EXCLUSIVE, &iRC );
            if ( pMemory == NULL )
            {
              usTmtRc = (USHORT)iRC;
              pFactory->showLastError( pIDA->szPluginName, pIDA->szMemName, NULL, hwnd );
            }
            else
            {
              pFactory->closeMemory( pMemory );
              UtlRenameObjectDlg( pIDA->szMemName, clsMEMORY );
            } /* endif */
          }
          break;

        case PID_FILE_MI_MERGE:
          {
            int iRC = 0;
            pMemory = pFactory->openMemory( pIDA->szPluginName, pIDA->szMemName, NONEXCLUSIVE, &iRC );
            if ( pMemory == NULL )
            {
              usTmtRc = (USHORT)iRC;
              pFactory->showLastError( NULL, pIDA->szMemName, NULL, hwnd );
            }
            else
            {
              fOK = MemGUIMergeMemory( pMemory, pCommArea, pIDA->szMemName, pIDA, hwnd );
            } /* endif */
          }
          break;

        case PID_FILE_MI_IMPORT:
          {
            if ( !fEnabled && ( sMemSelected != LIT_NONE) ) pIDA->szMemName[0] = EOS;  
            fOK = MemGUIImportMemory( pCommArea, pIDA->szMemName, pIDA, hwnd );
          }
          break;

        case PID_FILE_MI_PROPERTIES:
          {
            int iRC = 0;
            pMemory = pFactory->openMemory( pIDA->szPluginName, pIDA->szMemName, EXCLUSIVE, &iRC );
            if ( pMemory == NULL )
            {
              usTmtRc = (USHORT)iRC;
              pFactory->showLastError( pIDA->szPluginName, pIDA->szMemName, NULL, hwnd );
            }
            else
            {
              PMEM_PROP_IDA pPropIDA = NULL;  // Pointer to the prop dlg IDA

              // allocate storage for the property dialog IDA
              fOK = UtlAlloc( (PVOID *)&pPropIDA, 0L, (LONG)sizeof( MEM_PROP_IDA ), ERROR_STORAGE);

              // activate property dialog
              if ( fOK )
              {
                int   iDlgRc = 1;

                // fill  dialog IDA with necessary values
                pPropIDA->pMem = pMemory;
                strcpy( pPropIDA->szMemName, pIDA->szMemName );

                // call property dialog
                DIALOGBOX( QUERYACTIVEWINDOW(), MEMPROPDLG, hResMod, ID_TM_PROP_DLG, pPropIDA, iDlgRc );

                // update memory list window
                //EqfSend2Handler( MEMORYHANDLER, 
                //                 WM_EQFN_PROPERTIESCHANGED, 
                //                 MP1FROMSHORT( PROP_CLASS_MEMORY ), 
                //                 MP2FROMP( pIDA->szMemName ));
                WinSendMsg( hwnd, WM_EQF_MEMFILL_LISTBOX, NULL, NULL );

                UtlAlloc( (PVOID *)&pPropIDA, 0L, 0L, NOMSG );
              } /* endif */
              pFactory->closeMemory( pMemory );
            } /* endif */
          }
          break;

        case PID_FILE_MI_HTMLPROPS:
          {
            int iRC = 0;
            pMemory = pFactory->openMemory( pIDA->szPluginName, pIDA->szMemName, EXCLUSIVE, &iRC );
            if ( pMemory == NULL )
            {
              usTmtRc = (USHORT)iRC;
              pFactory->showLastError( pIDA->szPluginName, pIDA->szMemName, NULL, hwnd );
            }
            else
            {
              EqfMemPropsToHtml( hwnd, pIDA, pMemory );
              pFactory->closeMemory( pMemory );
            } /* endif */
          }
          break;

        case PID_FILE_MI_DELETE:
          {
            USHORT usYesToAllMode = 0;
            USHORT usReply = 0;
            PSZ pReplAddr[2];
            pReplAddr[0] = pIDA->szMemName;
            BOOL fShared = pFactory->isSharedMemory( pIDA->szMemName );
            // switch to yes-to-all message when more than one TM is selected
            if ( pszNameList ) usYesToAllMode = MB_EQF_YESTOALL;
            do
            {
              // TODO: check if a shared memory is being deleted, check user against owner of memory

              // get user confirmation
              if ( usYesToAllMode == MB_EQF_YESTOALL )
              {
                usReply = UtlError( (SHORT)(fShared ? WARNING_DEL_OWN_REM_TM  : WARNING_DELETE_MEMORY),
                                    MB_EQF_YESTOALL | MB_DEFBUTTON2, 1, &pReplAddr[0], EQF_QUERY );
                if ( usReply == MBID_EQF_YESTOALL )
                {
                  usYesToAllMode = usReply = MBID_YES;
                } /* endif */
              }
              else if ( usYesToAllMode == MBID_YES )
              {
                // user has choosen yes-to-all already
                usReply = MBID_YES;
              }
              else
              {
                usReply = UtlError( (SHORT)(fShared ? WARNING_DEL_OWN_REM_TM : WARNING_DELETE_MEMORY),
                                    MB_YESNO | MB_DEFBUTTON2, 1, &pReplAddr[0], EQF_QUERY );
              } /* endif */


              if ( usReply == MBID_YES )
              {
                std::string  strError;
                int iRC = pFactory->deleteMemory( pIDA->szPluginName, pIDA->szMemName,strError);

                if(iRC == 0)
                    EqfSend2Handler( MEMORYHANDLER, WM_EQFN_DELETED, MP1FROMSHORT( clsMEMORYDB ), MP2FROMP( pIDA->szMemObjName ));
                else
                    MessageBox(hwnd, strError.c_str(),NULL,MB_OK|MB_ICONWARNING);
              } /* endif */

              // prepare next TM
              pIDA->szMemName[0] = EOS;
              if ( pszNameList && *pszActiveName )
              {
                pszActiveName += strlen(pszActiveName) + 1;
                strcpy( pIDA->szMemObjName, pszActiveName );
                pFactory->splitObjName( pIDA->szMemObjName, pIDA->szPluginName, sizeof(pIDA->szPluginName), pIDA->szMemName, sizeof(pIDA->szMemName) );
              } /* endif */
            } while ( fOK && (usYesToAllMode != MBID_CANCEL) && (pIDA->szMemName[0] != EOS) );
          }
          break;

        /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
        case PID_FILE_MI_ORGANIZE:
          {
            PMEM_ORGANIZE_IDA   pRIDA = NULL;
            int iRC = 0;

            pMemory = pFactory->openMemory( pIDA->szPluginName, pIDA->szMemName, FOR_ORGANIZE, &iRC );
            if ( pMemory == NULL )
            {
              usTmtRc = (USHORT)iRC;
              pFactory->showLastError( pIDA->szPluginName, pIDA->szMemName, NULL, hwnd );
            }
            else
            {
              // Allocate storage for the MEM_ORGANIZE_IDA.
              fOK = UtlAlloc( (PVOID *)&pRIDA, 0L, (LONG)sizeof( MEM_ORGANIZE_IDA ), ERROR_STORAGE);
              if ( fOK )
              {
                // Fill  IDA with necessary values
                pRIDA->pMem = pMemory;
                strcpy( pRIDA->szMemName, pIDA->szMemName );
                // name list is now owned by organize IDA,
                pRIDA->pszNameList   = pszNameList;
                pRIDA->pszActiveName = pszNameList;
                pszNameList = NULL;


                // build object name
                sprintf( pCommArea->szBuffer, "MEMORG: %s", pIDA->szMemName );
                sRC = QUERYSYMBOL( pCommArea->szBuffer );
                if ( sRC == -1 )
                {
                   // memory is not in use ==> so lock this memory
                   SETSYMBOL( pCommArea->szBuffer );
                   fOK = CreateProcessWindow( pCommArea->szBuffer, MemOrganizeCallBack, pRIDA );
                }
                else
                {
                  PSZ pTemp = pIDA->szMemName;
                  OEMTOANSI( pIDA->szMemName );
                  UtlError( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL, 1, &pTemp, EQF_ERROR );
                  fOK = FALSE;
                } /* endif */

                if ( !fOK )
                {
                  pFactory->closeMemory( pMemory );
                  UtlAlloc( (PVOID *)&pRIDA, 0L, 0L, NOMSG );
                } /* endif */
              }
              else
              {
                pFactory->closeMemory( pMemory );
              } /* endif */
            } /* endif */
          }
          break;

        case PID_FILE_MI_ADDUSER:
          UtlAddUserDlg( pIDA->szMemName, clsMEMORY);
          break;

        case PID_FILE_MI_REMOVEUSER:
          UtlRemoveUserDlg(pIDA->szMemName, clsMEMORY);
          break;

        case PID_FILE_MI_LISTUSER:
          UtlListUserDlg(pIDA->szMemName, clsMEMORY);
          break;

		case PID_FILE_MI_UPLOADTOSERVER: 
		case PID_FILE_MI_DOWNFROMSERVER:
			{
				MemoryFactory *pFactory = MemoryFactory::getInstance();
				int iRC = 0;
				OtmMemory *pMem = pFactory->openMemory(NULL,pIDA->szMemName,0,&iRC);
				if(pMem != NULL)
				{
					bool isUpload = false;
					if(usMenuItem == PID_FILE_MI_UPLOADTOSERVER)
						isUpload = true;

					OtmSharedMemoryPlugin *pPlugin = NULL;
					pFactory->isSharedMemory(pIDA->szMemName,&pPlugin);
					if(pPlugin != NULL)
					    pPlugin->replicateWithServer(pIDA->szMemName,pMem,isUpload);
					pFactory->closeMemory(pMem);
				}
			}
			
			break;


        default:
        {
          MessageBox( hwnd, "This command is not implemented yet", "Warning",  MB_CANCEL );
        }
      } /* endswitch */
//      BOOL fIsNew = FALSE;         // new flag
//      CHAR            szMemName[MAX_FILESPEC]; // buffer for short name
//
//      ObjLongToShortName( pIDA->szMemName, szMemName, TM_OBJECT, &fIsNew );
//      strcpy( pIDA->szMemPath, szMemName );
//      fOK = (BOOL) EqfSend2Handler( MEMORYHANDLER, WM_EQF_PROCESSTASK,
//                                    MP1FROMSHORT( CREATE_PATH ),
//                                    MP2FROMP( pIDA->szMemPath ) );
//
//      if ( fOK )
//      {
//        /* for OPEN, EXPORT, and ORGANIZE TM will be opened exclusively */
//        switch ( usMenuItem )
//        {
//          case PID_FILE_MI_ORGANIZE:
//            // Open the memory database
//            if ( usMenuItem == PID_FILE_MI_ORGANIZE )
//            {
//              usTmtRc = TmOpen( pIDA->szMemPath, &hMem, FOR_ORGANIZE,
//                                TM_LOCALREMOTE, FALSE, 0 );
//            }
//            else if ( usMenuItem == PID_FILE_MI_EXPORT )
//            {
//            } 
//            else
//            {
//              usTmtRc = TmOpen( pIDA->szMemPath, &hMem, EXCLUSIVE,
//                                TM_LOCALREMOTE, FALSE, 0 );
//            } /* endif */
//
//            if ( usTmtRc == ERROR_OLD_PROPERTY_FILE )
//            {
//            }
//            else
//            {
//              usOrgType = TM_ORGANIZE;
//            } /* endif */
//
//            // If the return code is OK or FILE_MIGHT_BE_CORRUPTED or VERSION problems
//            // then continue anyway else call the translation memory handler routine
//            switch ( usTmtRc )
//            {
//              case OK:
//              case FILE_MIGHT_BE_CORRUPTED:
//              case VERSION_MISMATCH:
//              case CORRUPT_VERSION_MISMATCH:
//              case BTREE_CORRUPTED:
//               break;
//
//              default:
//               {
//                 PSZ pszServer = NULL;  // no servers anymore...
//
//                 /*****************************************************/
//                 /* Grey-out TM for specific return codes             */
//                 /*****************************************************/
//                 switch ( usTmtRc )
//                 {
//                   case ERROR_PATH_NOT_FOUND:
//                   case ERROR_INVALID_DRIVE:
//                   case BTREE_INVALID_DRIVE :
//                   case TM_FILE_SCREWED_UP:
//                   case NOT_A_MEMORY_DATABASE:
//                   case BTREE_ILLEGAL_FILE:
//                   case TM_FILE_NOT_FOUND:
//                   case BTREE_FILE_NOTFOUND:
//                   case TMERR_PROP_NOT_FOUND:
//                     CLBSETITEMSTATEHWND( pCommArea->hwndLB, sMemSelected,
//                                          FALSE );
//                     break;
//                 } /* endswitch */
//                 // Issue the TM error message and set usRc to FALSE
//                 // Return codes for test purposes only
//                 usTmtRc = MemRcHandling( usTmtRc, pIDA->szMemPath, &hMem,
//                                          pszServer );
//
//
//                 fOK = FALSE;
//               }
//               break;
//            } /* endswitch */
//            break;

//
//          default:
//            break;
//        } /* endswitch */
//      } /* endif */
//    } /* endif */
//
//    if ( fOK )
//    {
//      /* call the appropriate dialogs or post appropriate message */
//      switch ( usMenuItem )
//      {
//
//        /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
//
//        case PID_FILE_MI_PROPERTIES:
//          {

//
//        default:
//          break;
//      } /* endswitch */
    } /* endif */
  }
  else
  {
    fOK = FALSE;
  } /* endif */

  // free name list if not done yet
  if ( pszNameList ) UtlAlloc( (PVOID *)&pszNameList, 0, 0, NOMSG );

  return( (MRESULT) fOK );
} /* end of function MemListCommands */


/*****************************************************************************/
/* Process WM_EQF_PROCESSTASK message                                        */
/*****************************************************************************/
MRESULT MemListProcessTask
(
  PLISTCOMMAREA pCommArea,
  HWND          hwnd,
  WPARAM        mp1,
  LPARAM        mp2
)
{
  BOOL          fOK = TRUE;                      // default return value
  PMEM_IDA      pIDA;                       // Pointer to the Main Memory data area
  PSZ           pszString;
  SHORT         sIndex;

  pIDA = (PMEM_IDA)pCommArea->pUserIDA;

  switch ( SHORT1FROMMP1( mp1 ) )
  {
    //-------------------------------------------------------------------------
    case IMPORT_TASK :
      //--- A translation memory import in TROJA format should be performed
      EQFMemImportTrojaStart( hwnd, mp2, NULLHANDLE );
      break;
    case BATCHIMPORT_TASK :
      //--- A translation memory import in TROJA format should be performed
      {
        PBATCHIMPORTPARMS pBatchImpParms = (PBATCHIMPORTPARMS)PVOIDFROMMP2(mp2);
        EQFMemImportTrojaStart( hwnd, MP2FROMP(pBatchImpParms->pszObject),
                                pBatchImpParms->hwndErrMsg);
        UtlAlloc( (PVOID *)&pBatchImpParms, 0L, 0L, NOMSG );
      }
      break;
    //-------------------------------------------------------------------------
    case CREATE_PATH               :
    case QUERY_LANG_PAIR_INPUT_MEM :
    case QUERY_MARKUP_INPUT_MEM    :                           /*@1276A*/
      //   This function returns:
      //    - The server name of a TM       for TM_QUERY_SERVER_TASK
      //    - The full TM path + name + ext for CREATE_PATH
      //    - The language pair of a TM     for QUERY_LANG_PAIR_INPUT_MEM
      //    - The markup of a TM            for QUERY_MARKUP_INPUT_MEM /*@1276A*/
      //
      //   mp2 must contain the address of a string containing
      //   the Translation Memory name.
      //   The string must have a length of:
      //    -  MAX_SERVER_NAME              for TM_QUERY_SERVER_TASK
      //    -  MAX_EQF_PATH                 for CREATE_PATH
      //    -  2 * MAX_LANG_LENGTH          for QUERY_LANG_PAIR_INPUT_MEM
      //    -  MAX_FNAME                    for QUERY_MARKUP_INPUT_MEM /*@1276A*/
      //
      //   After return if OK then mResult will be TRUE and for:
      //    -  TM_QUERY_SERVER_TASK   The string will contain the name of
      //           the server or a NULL string if the TM is local
      //    -  CREATE_PATH  The string will contain the full
      //           Translation Memory path (path+name+ext)
      //    -  QUERY_LANG_PAIR_INPUT_MEM  The string will contain the
      //           source and the target language of the TM in the format:
      //           SourceLanguage\x015TargetLanguage\x015
      //           Use the utility UtlParseX15 and the enum values
      //              SOURCE_LANGUAGE_TM_IND and
      //              TARGET_LANGUAGE_TM_IND
      //           to extract the values needed.
      //    - QUERY_MARKUP_INPUT_MEM                            /*@1276A*/
      //           The markup language name of the TM.          /*@1276A*/
      //           Only the name without extension: "BOOKMAST\0"/*@1276A*/
      //
      //   If return code FALSE then the function failed and the string will
      //   remain unchaged. This may happen if the TM does not exist.
      //   Example:
      //     usRc = (USHORT)EqfSend2Handler
      //                    ( MEMORYHANDLER,
      //                      WM_EQF_PROCESSTASK,
      //                      MPFROMSHORT( TM_QUERY_SERVER_TASK ),
      //                      MPFROMP( szTemp ));
      if ( pIDA )
      {
        pszString = (PSZ)PVOIDFROMMP2( mp2 );        // Pointer to the TM name

        // Start with the first entry
        UtlMakeEQFPath( &(pCommArea->szBuffer[512]), NULC, SYSTEM_PATH, NULL );
        sprintf( pCommArea->szBuffer, "%s\\%s%s", &(pCommArea->szBuffer[512]),
                 pszString, EXT_OF_MEM );
        sIndex = LBSEARCHSTRINGHWND( pCommArea->hwndLB, pCommArea->szBuffer );

        // If the name was found then perform the appropriate tasks
        if ( sIndex != LIT_NONE )
        {
          // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//        QUERYITEMTEXTHWNDL( pCommArea->hwndLB, sIndex, pCommArea->szBuffer  );
          SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sIndex, (LPARAM)pCommArea->szBuffer );
          switch ( SHORT1FROMMP1( mp1 ) )
          {
            //-----------------------------------------------------------
            //case CREATE_PATH:
            //  {
            //    USHORT          usRc;
            //    HPROP           hProp;
            //    PPROP_NTM       pPropNTM;       // new TM properties
            //    PPROPTRANSLMEM  pProp;          // Pointer to TM properties
            //    EQFINFO         ErrorInfo;      // Property handler error info

            //    /* since we now have two different versions of TM, we have */
            //    /* to open the properties to read the full name            */
            //    strcpy( &(pCommArea->szBuffer[512]),
            //            UtlParseX15( pCommArea->szBuffer, MEM_OBJECT_IND) );

            //    usRc = NTMOpenProperties( &hProp,         // property handle
            //                (PVOID *)&pPropNTM,           // property pointer
            //                &(pCommArea->szBuffer[512]),  // property file name
            //                NULL,                         // system path
            //                PROP_ACCESS_READ,             // open mode
            //                FALSE );                      // message flag
            //    if ( usRc == NO_ERROR || usRc == ERROR_OLD_PROPERTY_FILE )
            //    {
            //      if ( usRc == ERROR_OLD_PROPERTY_FILE )
            //      {
            //        pProp = (PPROPTRANSLMEM) pPropNTM;
            //        strcpy( pszString, pProp->szFullMemName );
            //      }
            //      else
            //      {
            //        strcpy( pszString, pPropNTM->szFullMemName );
            //      } /* endif */

            //      CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
            //    }
            //    else
            //    {
            //      WinSendMsg( pCommArea->hwndLB, LM_EQF_SETITEMSTATE,
            //                  MP1FROMSHORT( sIndex ),
            //                  MP2FROMSHORT( FALSE ) );
            //      fOK = FALSE;
            //      *pszString = EOS;
            //    } /* endif */
            //  }
            //  break;
            //-----------------------------------------------------------
            case QUERY_LANG_PAIR_INPUT_MEM:
              sprintf( pszString, "%s%s%s%s%s%s",
                       UtlParseX15( pCommArea->szBuffer, MEM_SOURCELNG_IND ),
                       X15_STR,
                       UtlParseX15( pCommArea->szBuffer, MEM_TARGETLNG_IND ),
                       X15_STR, X15_STR, X15_STR );
              break;
            //-------------------------------------------------/*@1276A*/
            case QUERY_MARKUP_INPUT_MEM:                       /*@1276A*/
              strcpy( pszString,                               /*@1276A*/
                      UtlParseX15( pCommArea->szBuffer, MEM_MARKUP_IND ) );
              break;                                           /*@1276A*/
            //-----------------------------------------------------------
            default:
              break;
          } /* endswitch */
        }
        else
        {
          *pszString = EOS;
          fOK = FALSE;
        } /* endif */
      } /* endif */
      break;
    //-------------------------------------------------------------------------
    case QUERY_LANG_PROP_INPUT_MEM  :
    case QUERY_LANG_PROP_INPUT_LANG :
      // All properties of a language should be returned to the caller.
      // An example of calling this function is shown here:
      //    strcpy( szTemp, "test");
      //    usRc = (USHORT)EqfSend2Handler
      //                   ( MEMORYHANDLER,
      //                   WM_EQF_PROCESSTASK,
      //                   MPFROMSHORT( QUERY_LANG_PROP_INPUT_MEM ),
      //                   MPFROMP( szTemp ));
      // mp2 must contain the address of a string. The string should have
      // the length of MAX_LANGUAGE_PROPERTIES.
      // If mp1 is QUERY_LANG_PROP_INPUT_MEM then the string must contain
      // a TM name without extension. Example:  test
      // If mp1 is QUERY_LANG_PROP_INPUT_LANG then the string must contain
      // a language name. Example:  english
      //
      // If the return code is TRUE then the string will contain the
      // data in the following format:
      // Language\x015Morph. Dict\x015Lang. Code\x015...............
      // Example:
      // English\x015US.DC2\x0156011\x015ENGNOISE.LST\x015ENGSTOP.LST\x015.......
      //
      // If the return code is FALSE then the string may contain unpredictable data !!
      //
      // The following enum values can be used to extract data from the string
      // via the utility UtlParseX15
      //      LANG_LANGUAGE_IND,
      //      LANG_MORPH_DICT_IND,
      //      LANG_LANG_CODE_IND,
      //      LANG_NOISE_LIST_IND,
      //      LANG_STOPWORD_LIST_IND,
      //      LANG_ADDENDA_DICT_IND,
      // Example: Extract the name of the noise list after successfull return
      //          with the following statement
      // strcpy( pszNoiseList, UtlParseX15( pszString, LANG_NOISE_LIST_IND ));
      //
      if ( pIDA )
      {
        pszString = (PSZ)PVOIDFROMMP2( mp2 );          // Pointer to the TM name

        // If TM name has been supplied convert it to the language name
        // via the internal TM list
        if (SHORT1FROMMP1( mp1 ) == QUERY_LANG_PROP_INPUT_MEM )
        {
          // Search the translation memory name in the listbox.
          // Start with the first entry
          {
             BOOL fIsNew1 = FALSE;         // new flag
             CHAR szMemName[MAX_FILESPEC]; // buffer for short name

             ObjLongToShortName(pszString , szMemName, TM_OBJECT, &fIsNew1 );
             strcpy( pszString, szMemName );
          }

          UtlMakeEQFPath( &(pCommArea->szBuffer[512]), NULC, SYSTEM_PATH, NULL );
          sprintf( pCommArea->szBuffer, "%s\\%s%s", &(pCommArea->szBuffer[512]),
                   pszString, EXT_OF_MEM );


          sIndex = LBSEARCHSTRINGHWND( pCommArea->hwndLB, pCommArea->szBuffer );

          // If the name was found then perform the appropriate tasks
          if ( sIndex != LIT_NONE )
          {
            // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
//          QUERYITEMTEXTHWNDL( pCommArea->hwndLB, sIndex, pCommArea->szBuffer  );
            SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sIndex, (LPARAM)pCommArea->szBuffer );
            strcpy( pszString,
                    UtlParseX15( pCommArea->szBuffer, MEM_SOURCELNG_IND ) );
          }
          else
          {
            fOK = FALSE;
          } /* endif */
        } /* endif */

      } /* endif */
      break;
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    case PID_FILE_MI_ORGANIZE:
      // An organize of a translation memory should be performed
      // mp2 must contain the address of the TM name to be organized
      // If the organize starts well TRUE will be returned else FALSE
      if ( pIDA )
      {
        // Save TM name in IDA
        strcpy( pIDA->szProcessTaskWithObject, (CHAR *)PVOIDFROMMP2( mp2 ) );

        // Send a message to start the organize process
        fOK = (BOOL) WinSendMsg( hwnd, WM_EQF_COMMAND, mp1, NULL );
      } /* endif */
      break;
    case WM_EQF_MT_TMMERGE:
      {
        MTTMMergeStart( pCommArea, (PMT_TMMERGE)PVOIDFROMMP2( mp2 ) );
      }
      break;
    //-------------------------------------------------------------------------

    default:
      break;
  } /* endswitch */

  return( (MRESULT) fOK );
} /* end of function MemListProcessTask */

// data  area to pass info to callback function
typedef struct _FILLLISTDATA
{
  PSZ  pszBuffer;
  HWND hwndLBox;
} FILLLISTDATA, *PFILLLISTDATA;

// callback function to insert memory names with details into a listbox
int FillListBox( PVOID pvData, char *pszName, OtmMemoryPlugin::PMEMORYINFO pInfo  )
{
  PFILLLISTDATA pData = (PFILLLISTDATA)pvData;

  pszName;

  // driver letter
  char szDrive[3]={0};
  if(strlen(pInfo->szFullPath) > 0)
  {   
      szDrive[0] = pInfo->szFullPath[0];
      szDrive[1] = ':';
  }

  PSZ pszDispName = (PSZ)malloc( MAX_LONGPATH );
  PSZ pszDescription = (PSZ)malloc( MAX_LONGPATH );
  if ( (pszDispName == NULL ) || (pszDescription == NULL ) ) return( 0 );

  strcpy( pszDispName, pInfo->szName );
  strcpy( pszDescription, pInfo->szDescription );
  OEMTOANSI( pszDispName );
  OEMTOANSI( pszDescription );
  sprintf( pData->pszBuffer, "%s:%s%s%s%s%s%s%lu%s%s%s%s%s%s%s%s%s%s%s%s%s",
               pInfo->szPlugin, pInfo->szName, X15_STR,
               pszDispName,              X15_STR,
               pszDescription,           X15_STR,
               pInfo->ulSize,            X15_STR,
               szDrive,                  X15_STR,
               pInfo->szDescrMemoryType, X15_STR,
               pInfo->szOwner,           X15_STR,
               pInfo->szSourceLanguage,  X15_STR,
               "n/a",                    X15_STR,
               "n/a",                    X15_STR);
  SHORT sItem = INSERTITEMHWND( pData->hwndLBox, pData->pszBuffer );
  if ( (sItem >= 0) && !pInfo->fEnabled )
  {
    WinSendMsg( pData->hwndLBox, LM_EQF_SETITEMSTATE, MP1FROMSHORT( sItem ), MP2FROMSHORT( FALSE ) );
  } /* endif */

  free( (void *)pszDispName );
  free( (void *)pszDescription );
  return( 1 );
}

// callback function to insert names of not disabled memories into a combobox
int FillComboBox( PVOID pvData, char *pszName, OtmMemoryPlugin::PMEMORYINFO pInfo  )
{
  PFILLLISTDATA pData = (PFILLLISTDATA)pvData;

  if ( pInfo->fEnabled )
  {
    CBINSERTITEMHWND( pData->hwndLBox, pszName );
  } /* end */     
  return( 1 );
}



USHORT MemFillListBox( HWND hwndLBox, PSZ pszBuffer, PSZ pszNAString )
{
  FILLLISTDATA    FillListData;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  pszNAString;

  SETCURSOR( SPTR_WAIT );
  ENABLEUPDATEHWND_FALSE ( hwndLBox);
  DELETEALLHWND( hwndLBox );

  FillListData.pszBuffer = pszBuffer;
  FillListData.hwndLBox = hwndLBox;

  pFactory->listMemories( FillListBox, (PVOID)&FillListData, TRUE );

  SETCURSOR( SPTR_ARROW );

  USHORT usCount = QUERYITEMCOUNTHWND( hwndLBox );

  if ( usCount )
  {
    SELECTITEMHWND( hwndLBox, 0 );
  } /* endif */

  ENABLEUPDATEHWND_TRUE( hwndLBox );

  return( usCount );
} /* end of function MemFillListBox */

/**********************************************************************/
/* fill all available tm's into a combobox ...                        */
/**********************************************************************/
USHORT MemFillCBNames( HWND hwndLBox, PSZ pszBuffer, PSZ pszNAString )
{
  FILLLISTDATA    FillListData;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  pszNAString;

  SETCURSOR( SPTR_WAIT );
  ENABLEUPDATEHWND_FALSE( hwndLBox );
  DELETEALLHWND( hwndLBox );

  FillListData.pszBuffer = pszBuffer;
  FillListData.hwndLBox = hwndLBox;

  USHORT usCount = (USHORT)pFactory->listMemories( FillComboBox, (PVOID)&FillListData, TRUE );

  SETCURSOR( SPTR_ARROW );

  QUERYITEMCOUNTHWND( hwndLBox );

  if ( usCount )
  {
    SELECTITEMHWND( hwndLBox, 0 );
  } /* endif */

  ENABLEUPDATEHWND_TRUE ( hwndLBox);

  return( usCount );
} /* end of function MemFillCBNames */

/****************************************************************/
/* Make TM list item                                            */
/****************************************************************/
BOOL MemMakeListItem( PSZ pszObjName, PSZ pszBuffer,
                      PBOOL pfEnabled, PSZ pszNAString )
{
  BOOL            fOK = TRUE;

  pfEnabled;

  OtmMemoryPlugin::PMEMORYINFO pInfo = new( OtmMemoryPlugin::MEMORYINFO );
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  if ( pFactory->getMemoryInfo( NULL, pszObjName, pInfo ) == 0 )
  {
    CHAR szDrive[MAX_DRIVE]; // buffer for drive letter
    CHAR szFileSize[20];     // buffer for file size
    PSZ pszDispName = (PSZ)malloc( MAX_LONGPATH );
    PSZ pszDescription = (PSZ)malloc( MAX_LONGPATH );
    if ( (pszDispName == NULL ) || (pszDescription == NULL ) ) return( 0 );

    szDrive[0] = pInfo->szFullPath[0];
    szDrive[1] = ':';
    szDrive[2] = 0;
    ltoa( pInfo->ulSize, szFileSize, 10 );
    strcpy( pszDispName, pInfo->szName );
    strcpy( pszDescription, pInfo->szDescription );
    OEMTOANSI( pszDispName );
    OEMTOANSI( pszDescription );

    sprintf( pszBuffer,
               "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
               pszObjName,  X15_STR,
               pszDispName,  X15_STR,
               pszDescription,  X15_STR,
               szFileSize,  X15_STR,
               szDrive,     X15_STR,
               pInfo->szDescrMemoryType, X15_STR,
               pInfo->szOwner, X15_STR,
               pInfo->szSourceLanguage,   X15_STR,
               pszNAString, X15_STR,
               pszNAString, X15_STR);

    free( (void *)pszDispName );
    free( (void *)pszDescription );
  }
  else
  {
    fOK = FALSE;
  } /* end */     

  delete( pInfo );

  return( fOK );
} /* end of function MemMakeListItem */

/*************************************************************************/
/* this function checks if the memory is still available and accessible  */
/* It might happen that someone get rid of a secondary drive or of a     */
/* server drive  ....                                                    */
/*************************************************************************/

BOOL fMemIsAvail
(
  PSZ  pszName,
  PSZ  pszFullName,
  PSZ  pszSrcLang
)
{
    BOOL fOK = FALSE;
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    OtmMemoryPlugin::PMEMORYINFO pInfo = new( OtmMemoryPlugin::MEMORYINFO );
    if(pInfo != NULL)
    {
        int iRC = pFactory->getMemoryInfo( NULL, pszName, pInfo );
        if(iRC == NO_ERROR)
            fOK = TRUE;

        if (fOK &&(pszFullName || pszSrcLang))
        {
            if ( pszFullName )
                strcpy( pszFullName, pInfo->szFullPath );

            if ( pszSrcLang )
                strcpy( pszSrcLang, pInfo->szSourceLanguage );

            if(!pInfo->fEnabled)
                fOK = FALSE;
        }

        if(pInfo != NULL)
        delete pInfo;
    }
    return fOK;
}

//------------------------------------------------------------------------------
//Internal function
//------------------------------------------------------------------------------
//Function name:     MemCleanUp
//------------------------------------------------------------------------------
//Function call:     MemCleanup ( USHORT usTermCode )
//------------------------------------------------------------------------------
//Description:       TM cleanup function for DosExitList
//------------------------------------------------------------------------------
//Input parameter:   USHORT usTermCode
//------------------------------------------------------------------------------
VOID PASCAL FAR MemCleanUp ( USHORT usTermCode )                     /* 4@1CA */
{
  usTermCode;
} /* end MemCleanUP */

//------------------------------------------------------------------------------
//Internal function
//------------------------------------------------------------------------------
//Function name:     MemBatchTMDelete
//------------------------------------------------------------------------------
//Description:       Deletes a Translation Memory in batch mode.
//------------------------------------------------------------------------------
//Input parameter:   HWND       hwnd     handle of memory list window
//                   PDDEMEMDEL pMemDel  ptr to memory delete info
//------------------------------------------------------------------------------
//Returncode type:   USHORT              error code or 0 if success
//------------------------------------------------------------------------------
//USHORT MemBatchTMDelete( HWND hwnd, PDDEMEMDEL pMemDel )
//{
//   BOOL            fOK = TRUE;         // return value
//   PSZ             pszParm;            // error parameter pointer
//   CHAR            szMemPath[MAX_LONGPATH];
//   hwnd;
//   /*******************************************************************/
//   /* Check if there is a TM with the given name                      */
//   /*******************************************************************/
//   if ( fOK )
//   {
//     BOOL fIsNew = FALSE;
//     ObjLongToShortName( pMemDel->szName, szMemPath,
//                         TM_OBJECT, &fIsNew );
//
//     fOK = (BOOL)EqfSend2Handler( MEMORYHANDLER,
//                                  WM_EQF_PROCESSTASK,
//                                  MP1FROMSHORT( CREATE_PATH ),
//                                  MP2FROMP( szMemPath ));
//     if ( !fOK )
//     {
//       pszParm = pMemDel->szName;
//       UtlErrorHwnd( ERROR_MEMORY_NOTFOUND, MB_CANCEL, 1, &pszParm,
//                     EQF_ERROR, pMemDel->hwndErrMsg );
//     } /* endif */
//   } /* endif */
//
//   /*******************************************************************/
//   /* Delete the TM                                                   */
//   /*******************************************************************/
//   if ( fOK )
//   {
//     USHORT usDummy = 0;
//     fOK = (TmDeleteTM( pMemDel->szName, TRUE, pMemDel->hwndErrMsg, &usDummy ) == NO_ERROR);
//   } /* endif */
//
//   /*******************************************************************/
//   /* Cleanup                                                         */
//   /*******************************************************************/
//   if ( !fOK )
//   {
//     pMemDel->DDEReturn.usRc = UtlGetDDEErrorCode( pMemDel->hwndErrMsg );
//   } /* endif */
//
//   /****************************************************************/
//   /* report end of task to DDE handler                            */
//   /****************************************************************/
//   WinPostMsg( pMemDel->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
//               MP2FROMP(&pMemDel->DDEReturn) );
//
//   return( pMemDel->DDEReturn.usRc );
//
//} /* end of function MemBatchTMDelete  */


//------------------------------------------------------------------------------
//Internal function
//------------------------------------------------------------------------------
//Function name:     MemBatchTMOrganize
//------------------------------------------------------------------------------
//Description:       Organizes a Translation Memory in batch mode.
//------------------------------------------------------------------------------
//Input parameter:   HWND       hwnd     handle of memory list window
//                   PDDEMEMORG pMemORG  ptr to memory organize info
//------------------------------------------------------------------------------
//Returncode type:   USHORT              error code or 0 if success
//------------------------------------------------------------------------------
//USHORT MemBatchTMOrganize( HWND hwnd, PDDEMEMORG pMemOrg )
//{
//   BOOL            fOK = TRUE;         // return value
//   PSZ             pszParm;            // error parameter pointer
//   CHAR            szMemPath[MAX_LONGPATH];
//   SHORT           sRC;                // value used for symbol checking..
//   hwnd;
//   MemoryFactory *pFactory = MemoryFactory::getInstance();
//   OtmMemory     *pMemory = NULL;
//
//   /*******************************************************************/
//   /* Check if there is a TM with the given name                      */
//   /*******************************************************************/
//   if ( fOK )
//   {
//     BOOL fIsNew = FALSE;
//     ObjLongToShortName( pMemOrg->szName, szMemPath,
//                         TM_OBJECT, &fIsNew );
//
//     fOK = (BOOL)EqfSend2Handler( MEMORYHANDLER,
//                                  WM_EQF_PROCESSTASK,
//                                  MP1FROMSHORT( CREATE_PATH ),
//                                  MP2FROMP( szMemPath ));
//     if ( !fOK )
//     {
//       pszParm = pMemOrg->szName;
//       UtlErrorHwnd( ERROR_MEMORY_NOTFOUND, MB_CANCEL, 1, &pszParm,
//                     EQF_ERROR, pMemOrg->hwndErrMsg );
//     } /* endif */
//   } /* endif */
//
//   /*******************************************************************/
//   /* Organize the TM                                                 */
//   /*******************************************************************/
//   if ( fOK )
//   {
//     int iRC = 0;
//
//     pMemory = pFactory->openMemory( NULL, pMemOrg->szName, FOR_ORGANIZE, &iRC );
//
//
//     // If the return code is OK or FILE_MIGHT_BE_CORRUPTED or VERSION problems
//     // then continue anyway else call the translation memory handler routine
//     switch ( iRC )
//     {
//       case OK:
//       case FILE_MIGHT_BE_CORRUPTED:
//       case VERSION_MISMATCH:
//       case CORRUPT_VERSION_MISMATCH:
//       case BTREE_CORRUPTED:
//        break;
//
//       default:
//         pFactory->showLastError( NULL, pMemOrg->szName, NULL, pMemOrg->hwndErrMsg );
//         fOK = FALSE;
//        break;
//     } /* endswitch */
//
//     /*****************************************************************/
//     /* Start TM organize process                                     */
//     /*****************************************************************/
//     if ( fOK )
//     {
//       PMEM_ORGANIZE_IDA   pRIDA;
//
//       // Allocate storage for the MEM_ORGANIZE_IDA.
//       fOK = UtlAllocHwnd( (PVOID *)&pRIDA, 0L, (LONG)sizeof( MEM_ORGANIZE_IDA ), ERROR_STORAGE, pMemOrg->hwndErrMsg );
//       if ( fOK )
//       {
//         // Fill  IDA with necessary values
//         pRIDA->pMem = pMemory;
//         strcpy( pRIDA->szMemName, pMemOrg->szName );
//         pRIDA->fBatch = TRUE;
//         pRIDA->hwndErrMsg = pMemOrg->hwndErrMsg;
//         pRIDA->pDDEMemOrg = pMemOrg;
//
//         // build object name
//         sprintf( szMemPath, "MEMORG: %s", pMemOrg->szName );
//         sRC = QUERYSYMBOL( szMemPath );
//         if ( sRC == -1 )
//         {
//            // TM is not in use ==> lock this TM
//            SETSYMBOL( szMemPath );
//            fOK = CreateProcessWindow( szMemPath, MemOrganizeCallBack, pRIDA );
//         }
//         else
//         {
//           PSZ pTemp = pMemOrg->szName;
//           UtlErrorHwnd( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL, 1, &pTemp, EQF_ERROR, pMemOrg->hwndErrMsg );
//           fOK = FALSE;
//         } /* endif */
//
//         if ( !fOK )
//         {
//           /* free IDA, otherwise it will be freed in CallBack */
//           UtlAlloc( (PVOID *)&pRIDA, 0L, 0L, NOMSG );
//         } /* endif */
//       } /* endif */
//     } /* endif */
//   } /* endif */
//
//   /*******************************************************************/
//   /* Cleanup in case of errors                                       */
//   /*******************************************************************/
//   if ( !fOK )
//   {
//     pMemOrg->DDEReturn.usRc = UtlGetDDEErrorCode( pMemOrg->hwndErrMsg );
//     if ( pMemory != NULL )
//     {
//       pFactory->closeMemory( pMemory );
//     } /* endif */
//     WinPostMsg( pMemOrg->hwndOwner, WM_EQF_DDE_ANSWER, NULL, MP2FROMP(&pMemOrg->DDEReturn) );
//   } /* endif */
//
//   return( pMemOrg->DDEReturn.usRc );
//
//} /* end of function MemBatchTMOrganize  */
//

/**********************************************************************/
/* MTTMMergeStart: merge a MT memory ...                              */
/**********************************************************************/
//------------------------------------------------------------------------------
//Internal function
//------------------------------------------------------------------------------
//Function name:     MTTMMerge
//------------------------------------------------------------------------------
//Description:       Merge a MT memory (external) into an internal TM
//------------------------------------------------------------------------------
//Input parameter:
//------------------------------------------------------------------------------
//Returncode type:   BOOL           TRUE if success
//------------------------------------------------------------------------------
BOOL MTTMMergeStart
(
  PLISTCOMMAREA pCommArea,
  PMT_TMMERGE pTMMerge
)
{
  BOOL        fOK = TRUE;
  MemoryFactory *pFactory = MemoryFactory::getInstance();
    
  if ( pTMMerge )
  {
    PMEM_LOAD_DLG_IDA   pLIDA;


    // Allocate storage for the MEM_LOAD_DLG_IDA. The area will be freed
    // in the function  EQFMemloadStart. Only in case of an error
    // it will be freed here. If OK fill the
    // IDA with necessary values else issue an error message and set usRc to FALSE
    fOK = UtlAlloc( (PVOID *)&pLIDA, 0L, (LONG)sizeof(MEM_LOAD_DLG_IDA), ERROR_STORAGE );
    if ( fOK )
    {
      USHORT usDosRc;
      USHORT usAction;
      USHORT usMemExists;


      PSZ    pTM = UtlGetFnameFromPath( pTMMerge->chMemory );
      if ( !pTM )
      {
        pTM = pTMMerge->chMemory;
      } /* endif */

      pLIDA->fAscii = TRUE;   // TM in external format
      pLIDA->fMTReceiveCounting = TRUE;          // count received segments/words and update folder props
      strcpy( pLIDA->szFolObjName, pTMMerge->szFolObjName );
      pLIDA->fIgnoreEqualSegments = TRUE;       // ignore segments with identical source and target
      pLIDA->fAdjustTrailingWhitespace = TRUE;  // adjust trailing whitespace of target string
      pLIDA->usImpMode = MEM_SGMLFORMAT_UNICODE;
      pLIDA->ulOemCP  = GetLangOEMCP(pTMMerge->szTargetLang);
      pLIDA->ulAnsiCP = GetLangAnsiCP(pTMMerge->szTargetLang);
      strcpy( pLIDA->szFilePath, pTMMerge->chSGMLFile );

      strcpy( pLIDA->szMemName, pTMMerge->chMemory );
      strcpy( pLIDA->szObjName, pTMMerge->szObjName );
      pLIDA->hwndNotify = pTMMerge->hwndNotify;

      usMemExists = (USHORT)pFactory->exists( NULL, pLIDA->szMemName );

      //--- is TM specified in the To TM entry field a new one ?
      //--- if the translation memory does not exist create it
      if( !usMemExists )
      {
         //--- set 'dialog is disabled' flag, to prevent
         //--- toogle between TM Create ant TM Import Dialog
         //--- Note: using WinEnableWindow to disable the dialog
         //---       does not work correctly
         pLIDA->fDisabled = TRUE;

         SETCURSOR( SPTR_ARROW );
         fOK = (BOOL)EqfSend2Handler( MEMORYHANDLER, WM_EQF_CREATE, MP1FROMSHORT( 0 ),  MP2FROMP( pLIDA->szMemName ) );
         SETCURSOR( SPTR_WAIT );

         //--- reset dialog is disabled flag, dialog is active
         pLIDA->fDisabled = FALSE;
      }
      else
      {
        // ask user if existing MT memory should be deleted
        USHORT usMBReturn = UtlError( QUERY_DELETEMTMEM, MB_YESNO | MB_DEFBUTTON2, 0, NULL, EQF_QUERY );
        switch ( usMBReturn )
        {
          case MBID_YES :
            pFactory->clearMemory( NULL, pLIDA->szMemName );
            break;
          case MBID_NO :
          default :
            break;
        } /* endswitch */
      } /* endif */

      if ( fOK )   //--- TM can be accessed
      {
        int iRC;
        pLIDA->pMem = pFactory->openMemory( NULL, pLIDA->szMemName, EXCLUSIVE, &iRC );
        if ( pLIDA->pMem == NULL )
        {
          usDosRc = (USHORT)iRC;
          fOK = FALSE;
        } /* endif */
      } /* endif */

      if ( fOK )  //--- TM opened without error
      {
        //--- Open the file to be imported and store file handle.
        usDosRc = UtlOpen ( pLIDA->szFilePath,
                          &(pLIDA->hFile),
                          &usAction,    // action taken (out)
                          0L,           // Create file size
                          FILE_NORMAL,  // Normal attributes
                          FILE_OPEN,    // Open if exists else fail
                          OPEN_SHARE_DENYWRITE,// Deny Write access
                          0L,           // Reserved but handle errors
                          TRUE );       // display error message
        if ( usDosRc )  //--- error from open
        {
           //--- Reset file handle to NULL and set fOK to FALSE
           pLIDA->hFile = NULLHANDLE;
           fOK = FALSE;
        }/* endif */
      } /* endif */
    } /* endif */


    // If usRc post a  WM_EQF_MEMLOAD_START message else
    // clean up allocated storage.
    if ( fOK )
    {
      // build object name
      SHORT       sRC;
      sprintf( pCommArea->szBuffer, "MEMIMP: %s", pLIDA->szMemName );
      sRC = QUERYSYMBOL( pCommArea->szBuffer );
      if ( sRC == -1 )
      {
         // memory is not in use ==> lock this folder
         SETSYMBOL( pCommArea->szBuffer );
         fOK = CreateProcessWindow2( pCommArea->szBuffer,
                                    MemImportCallBack, pLIDA, FALSE );
      }
      else
      {
        PSZ pTemp = pLIDA->szMemName;

        UtlError( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL,
                  1, &pTemp, EQF_ERROR );
        fOK = FALSE;
      } /* endif */


      if ( !fOK )
      {
        /* free IDA, otherwise it will be freed in CallBack */
        UtlAlloc( (PVOID *)&pLIDA, 0L, 0L, NOMSG );
      } /* endif */
    }
    else
    {
      // Free the MEM_LOAD_DLG_IDA area
      UtlAlloc( (PVOID *)&pLIDA, 0L, 0L, NOMSG );
    } /* endif */
  }
  else
  {
    fOK = FALSE;
  } /* endif */

  return fOK;
}

// new function to replace CREATE_PATH task message
BOOL MemCreatePath( PSZ pszString )
{
  USHORT          usRc;

  OtmMemoryPlugin::PMEMORYINFO pInfo = new( OtmMemoryPlugin::MEMORYINFO );
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  usRc = (USHORT)-1;
  if(pInfo != NULL && pFactory!= NULL)
     usRc = (USHORT)pFactory->getMemoryInfo( NULL, pszString, pInfo );

  if (usRc == 0)
  {
	  strcpy( pszString,pInfo->szName);
  }
  else
  {
     *pszString = EOS;
  }
  if(pInfo != NULL)
      delete pInfo;

  return (usRc==0);
} /* end of function MemCreatePath */

//------------------------------------------------------------------------------
//Internal function
//------------------------------------------------------------------------------
//Function name:     MemFuncDeleteMem
//------------------------------------------------------------------------------
//Description:       Deletes a Translation Memory in function I/F mode
//------------------------------------------------------------------------------
//Input parameter:  PSZ pszMemName  ptr to name of memory
//------------------------------------------------------------------------------
//Returncode type:   USHORT              error code or 0 if success
//------------------------------------------------------------------------------

__declspec(dllexport)
USHORT MemFuncDeleteMem( PSZ pszMemName )
{
   BOOL            fOK = TRUE;         // return value
   USHORT          usRC = NO_ERROR;    // function return code

   // check if a TM has been specified
   if ( fOK )
   {
     if ( (pszMemName == NULL) || (*pszMemName == EOS) )
     {
       fOK = FALSE;
       UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL,
                     EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */


   OtmMemoryPlugin::MEMORYINFO Info;
   MemoryFactory *pFactory = MemoryFactory::getInstance();
   // check if there is a TM with the given name
   if ( fOK )
   {
       // check if memory existed
        usRC = (USHORT)pFactory->getMemoryInfo( NULL, pszMemName, &Info );
        if (usRC != 0)
        {
            fOK = FALSE;
	        UtlErrorHwnd( ERROR_MEMORY_NOTFOUND, MB_CANCEL, 1,
                          &pszMemName, EQF_ERROR, HWND_FUNCIF );
        }
   }

   // Delete the TM
   if ( fOK )
   {
     int iRC = pFactory->deleteMemory( NULL,pszMemName);
     fOK = (iRC==0)?TRUE:FALSE;

     // notify OpenTM2 GUI to update
     if(fOK)
     {
        HWND hwnd = FindWindow( TWBMAIN ,NULL);
        if(hwnd != NULL)
        {
          COPYDATASTRUCT cds;
          std::string strMemName(Info.szPlugin);
          strMemName = strMemName+":";
          strMemName = strMemName +pszMemName;
          cds.dwData = 2;
          cds.cbData = (strMemName.size()+1)*sizeof(char);
          cds.lpData = (char*)strMemName.c_str();
          SendMessage(hwnd,WM_COPYDATA, 0,(LPARAM)(LPVOID)&cds);
        }
     }// end if

   }

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( !fOK )
   {
     usRC = UtlQueryUShort( QS_LASTERRORMSGID );
   }

   return( usRC );
} /* end of function MemFuncDeleteMem  */

/*! \brief Data area of the MemFuncClearMTFlag function
*/
typedef struct _CLEARMTFLAGDATA
{
  char szInMem[MAX_LONGFILESPEC];      // fully qualified name of input memory
  char szOutMem[MAX_LONGFILESPEC];     // fully qualified name of output memory
  BOOL fOverwrite;                     // TRUE = overwrite input memory with output memory
  BOOL fUTF16Mem;                      // TRUE = memory in in UTF16 format
  char szLine[8096];                   // buffer for a line of the memory
  CHAR_W szLineW[8096];                // buffer for a line of the memory
  FILE *pfInMem;                       // input memory file
  FILE *pfOutMem;                      // output memory file
} CLEARMTFLAGDATA, *PCLEARMTFLAGDATA;

/*! \brief Clears the MT flag of an external memory in the EXp format
  \param pszInMemory fully qualified file name of the input memory
  \param pszOutMemory fully qualified file name of the output memory, if not specified the output memory overwrites the input memory
	\returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT MemFuncClearMTFlag( PSZ pszInMemory, PSZ pszOutMemory )
{
   USHORT          usRC = NO_ERROR;    // function return code
   PCLEARMTFLAGDATA pData = NULL;

   // allocate private data area 
   if ( !UtlAlloc( (PVOID *)&pData, 0, sizeof(CLEARMTFLAGDATA), ERROR_STORAGE ) )
   {
     usRC = ERROR_STORAGE;
   }

   // check if a input memory has been specified
   if ( usRC == NO_ERROR )
   {
     if ( (pszInMemory == NULL) || (*pszInMemory == EOS) )
     {
       usRC = TMT_MANDCMDLINE;
       UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   // build output memory name if no output memory has been specified
   if ( usRC == NO_ERROR )
   {
     if ( (pszOutMemory == NULL) || (*pszOutMemory == EOS) )
     {
       pData->fOverwrite = TRUE;
       strcpy( pData->szOutMem, pszInMemory );
       strcat( pData->szOutMem, "-TEMPOUT" );
     }
     else
     {
       pData->fOverwrite = FALSE;
       strcpy( pData->szOutMem, pszOutMemory );
     } /* endif */
   } /* endif */

   // check type of EXP memory
   if ( usRC == NO_ERROR )
   {
     FILE *hf = NULLHANDLE;

     pData->fUTF16Mem = FALSE;

     hf = fopen( pszInMemory, "rb" );

     if ( hf )
     {
       // get first two bytes and check for BOM
       BYTE chBOM[30];
       BOOL fBOM = FALSE;
       fread( chBOM, 1, 30, hf );

       if ( memcmp( chBOM, UNICODEFILEPREFIX, 2 ) == 0 )
       {
         pData->fUTF16Mem = TRUE;
         fBOM = TRUE;
       }
       else if ( (chBOM[0] == '<') && (chBOM[1] == 0 ) )
       {
         pData->fUTF16Mem = TRUE;
       }
       fclose( hf );

       // check if this is a valid EXP file
       if ( pData->fUTF16Mem )
       {
         PSZ_W pszStart = ( fBOM ) ? ((PSZ_W)(chBOM+2)) : ((PSZ_W)(chBOM));
         if( _wcsnicmp( pszStart, L"<NTMMemoryDb>", 13 ) != 0 )
         {
           usRC = ERROR_FILE_INVALID_DATA;
    	     UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszInMemory, EQF_ERROR, HWND_FUNCIF );
         }
       }
       else
       {
         if( strnicmp( (char *)chBOM, "<NTMMemoryDb>", 13 ) != 0 )
         {
           usRC = ERROR_FILE_INVALID_DATA;
    	     UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszInMemory, EQF_ERROR, HWND_FUNCIF );
         }
       }
     } 
     else
     {
        usRC = ERROR_MEMORY_NOTFOUND;
  	    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszInMemory, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   // open the input and output memory
   if ( usRC == NO_ERROR )
   {
     if ( pData->fUTF16Mem )
     {
       pData->pfInMem = fopen( pszInMemory, "rb" );
       pData->pfOutMem = fopen( pData->szOutMem, "wb" );
     }
     else
     {
       pData->pfInMem = fopen( pszInMemory, "r" );
       pData->pfOutMem = fopen( pData->szOutMem, "w" );
     } /* endif */

     if ( pData->pfInMem == NULL )
     {
        usRC = ERROR_MEMORY_NOTFOUND;
  	    UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszInMemory, EQF_ERROR, HWND_FUNCIF );
     } 
     else if ( pData->pfOutMem == NULL )
     {
       PSZ pszParms[2];
       char szCode[10];

       pszParms[0] = pData->szOutMem;
       int iFileRC = GetLastError();
       itoa( iFileRC, szCode, 10 );
       pszParms[1] = szCode;
       usRC = ERROR_FILE_WRITE_ERROR;
       UtlErrorHwnd( usRC, MB_CANCEL, 2, pszParms, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   // process memory until done
   if ( usRC == NO_ERROR )
   {
     BOOL fControl = FALSE;
     while ( !feof( pData->pfInMem ) )
     {
       if ( pData->fUTF16Mem )
       {
         memset( pData->szLineW, 0, sizeof(pData->szLineW) );
         fgetws( pData->szLineW, sizeof(pData->szLineW)/sizeof(pData->szLineW[0]), pData->pfInMem );
         if ( pData->szLineW[0] != 0 )
         {
           int iLen = wcslen( pData->szLineW );

           if ( fControl )
           {
             // locate MT flag
             PSZ_W pszFlag = wcschr( pData->szLineW, L'\x15' );
             if ( pszFlag != NULL )
             {
               pszFlag++;
               if ( *pszFlag == L'1' )
               {
                 *pszFlag = '0';
               }
             }
           }
           else if ( (iLen >= 9 ) && (_wcsnicmp( pData->szLineW, L"<control>", 9 ) == 0) ) 
           {
             fControl = TRUE;
           }
           else
           {
             fControl = FALSE;
           }

           fputws( pData->szLineW, pData->pfOutMem );
         }
       }
       else
       {
         memset( pData->szLine, 0, sizeof(pData->szLineW) );
         fgets( pData->szLine, sizeof(pData->szLine), pData->pfInMem );
         if ( pData->szLine[0] != 0 )
         {
           int iLen = strlen( pData->szLine );

           if ( fControl )
           {
             // locate MT flag
             PSZ pszFlag = strchr( pData->szLine, '\x15' );
             if ( pszFlag != NULL )
             {
               pszFlag++;
               if ( *pszFlag == '1' )
               {
                 *pszFlag = '0';
               }
             }
           }
           else if ( (iLen >= 9 ) && (strnicmp( pData->szLine, "<control>", 9 ) == 0) ) 
           {
             fControl = TRUE;
           }
           else
           {
             fControl = FALSE;
           }

           fputs( pData->szLine, pData->pfOutMem );
         }
       } /* endif */
     } /* enwwhile */
   } /* endif */

   // cleanup
   if ( pData != NULL )
   {
     if ( pData->pfInMem != NULL ) fclose( pData->pfInMem );
     if ( pData->pfOutMem != NULL ) fclose( pData->pfOutMem );

     // if input memory should be overwritten with mofified one, delete old memory and rename the new one
     if ( (usRC == 0) && pData->fOverwrite )
     {
       UtlDelete( pszInMemory, 0, FALSE );
       UtlMove( pData->szOutMem, pszInMemory, 0L, FALSE );
     }

     UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );
   }

   return( usRC );
} /* end of function MemFuncDeleteMem  */

__declspec(dllexport)
USHORT MemFuncChangeMFlag
(
    PSZ  pszMemName,
    LONG lAction
)
{
    USHORT   usRC = NO_ERROR;            // function return code
    OtmMemory *pMem = NULL;

    MemoryFactory *pFactory = MemoryFactory::getInstance();

    // check required parameters
    if ( usRC == NO_ERROR )
    {
        if ( (pszMemName == NULL) || (*pszMemName == EOS) )
        {
            usRC = TMT_MANDCMDLINE;
            UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
    } /* endif */

    if ( usRC == NO_ERROR )
    {
      if ( !pFactory->exists( NULL, pszMemName ) )
      {
          PSZ pszParm = pszMemName;
          usRC = ERROR_MEMORY_NOTFOUND;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      } /* endif */
    }

    if ( usRC == NO_ERROR )
    {
      if ((lAction != CLEAR_MMOPT) && (lAction != SET_MMOPT) )
      {
          usRC = ERROR_CMD_TASKLISTWRONG;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, NULL, EQF_ERROR, HWND_FUNCIF );
      }
    }

    // open the memory
    if ( usRC == NO_ERROR  )
    {
      int iRC = 0;
      pMem = pFactory->openMemory( NULL, pszMemName, EXCLUSIVE, &iRC );
      if ( pMem == NULL )
      {
        usRC = (USHORT)iRC;
        pFactory->showLastError( NULL, pszMemName, pMem, HWND_FUNCIF );
      }
    } /* endif */

    if ( usRC == NO_ERROR )
    {
      OtmProposal prop;
      int iProgress = 0;

      int iRC = pMem->getFirstProposal( prop, &iProgress );
      while ( iRC == 0 )
      {
        BOOL fUpdate = FALSE;

        if ( (lAction == SET_MMOPT) && (prop.getType() == OtmProposal::eptManual) )
        {
          prop.setType( OtmProposal::eptMachine );
          fUpdate = TRUE;
        }
        else if ( (lAction == CLEAR_MMOPT) && (prop.getType() == OtmProposal::eptMachine ) )
        {
          prop.setType( OtmProposal::eptManual );
          fUpdate = TRUE;
        }

        if ( fUpdate ) pMem->updateProposal( prop, OtmMemory::UPDATE_MTFLAG );

        iRC = pMem->getNextProposal( prop, &iProgress );
      } /* endwhile */
    }

    // close the translation memory
    if ( pMem != NULL ) pFactory->closeMemory( pMem );

    return( usRC );
}
