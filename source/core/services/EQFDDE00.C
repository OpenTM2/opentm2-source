//+----------------------------------------------------------------------------+
//|EQFDDE00.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013 International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: Regine Jornitz                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Object handler and instance for DDE conversation               |
//+----------------------------------------------------------------------------+
#include <eqf.h>                       // General Translation Manager include file

#include "eqfdde.h"

#include <dde.h>

#define  MAX_DDE_FILES   MAX_LIST_FILES

/**********************************************************************/
/* Option strings                                                     */
/**********************************************************************/
#define DICT_OPTION     "DICT"
#define MEM_OPTION      "MEM"
#define DELETE_OPTION   "DELETE"
#define ADDTOMEM_OPTION "ADDTOMEM"
#define AUTO_OPTION     "AUTO"
#define AUTOLAST_OPTION "AUTOLAST"
#define AUTOJOIN_OPTION "AUTOJOIN"
#define AUTOCONTEXT_OPTION "AUTOCONTEXT"
#define UNTRANSLATED_OPTION "UNTRANSLATED"
#define SHARED_OPTION   "SHARED"
#define LOCAL_OPTION    "LOCAL"
#define EXTERNAL_OPTION "EXTERNAL"
#define READONLYMEM_OPTION "ROMEM"
#define DOCMEM_OPTION "DOCMEM"
#define REDUNDCOUNT_OPTION "REDCOUNT"
#define ADJUSTLEADWS_OPTION "ADJUSTLEADWS"
#define ADJUSTTRAILWS_OPTION "ADJUSTTRAILWS"
#define RESPECTCRLF_OPTION   "RESPECTCRLF"
#define ASMASTERFOLDER_OPTION "MASTERFOLDER"

// TO DO

static CMDLIST EQFCmdList[ BATCH_END + 1 ] =
{
  {BATCH_TASK,         "/TASK=",        "/TA="     },     // task
  {BATCH_FLD,          "/FLD=",         "/FLD="    },     // folder name
  {BATCH_FILES,        "/FILES=",       "/FI="     },     // input file or list of files
  {BATCH_OUT,          "/OUT=",         "/OUT="    },     // name of output file
  {BATCH_TYPE,         "/TYPE=",        "/TY="     },     // type of selection
  {BATCH_MARKUP,       "/MARKUP=",      "/MA="     },     // markup language
  {BATCH_FROMDRIVE,    "/FROMDRIVE=",   "/FR="     },     // from which drive
  {BATCH_TODRIVE,      "/TODRIVE=",     "/TO="     },     // to which drive
  {BATCH_FROMPATH,     "/FROMPATH=",    "/FP="     },     // from which path
  {BATCH_TOPATH,       "/TOPATH=",      "/TP="     },     // to which path
  {BATCH_EDIT,         "/EDIT=",        "/ED="   },       // name of editor
  {BATCH_SRCLNG,       "/SRCLNG=",      "/SR=" },         // source language
  {BATCH_TGTLNG,       "/TGTLNG=",      "/TG=" },         // target language
  {BATCH_DESC,         "/DESC=",        "/DE="   },       // description of a file
  {BATCH_NAME,         "/NAME=",        "/NA="   },       // name of a file
  {BATCH_MEM,          "/MEM=",         "/ME="    },      // translation memory
  {BATCH_DICT,         "/DICT=",        "/DI="   },       // name of dictionary
  {BATCH_FILT,         "/FILT=",        "/FILT="   },     // name of filter
  {BATCH_OPTIONS,      "/OPTIONS=",     "/OP=" },         // options
  {BATCH_EXCLLIST,     "/EXCLLIST=",    "/EXCLL="  },     // exclusion list
  {BATCH_EXCLDICT,     "/EXCLDICT=",    "/EXCLD="  },     // exclusion dictionary
  {BATCH_OVERWRITE,    "/OVERWRITE=",   "/OV=" },         // overwrite existing files
  {BATCH_SGMLMEM,      "/SGMLMEM=",     "/SG=" },         // name of the SGML output file
  {BATCH_NOANA,        "/NOANA=",       "/NOA="   },      // NO analysis
  {BATCH_NOTMDB,       "/NOTMDB=",      "/NOT=" },        // NO translation memory database
  {BATCH_NOCONF,       "/NOCONF=",      "/NOC=" },        // NO confirmation message at end
  {BATCH_LEVEL ,       "/LEVEL=",       "/LE="  },        // level of match
  {BATCH_CURDIR,       CURDIR_NAME,     CURDIR_NAME, },   // current drive and directory
  {BATCH_PASSWORD,     "/PASSWORD=",    "/PA=" },         // password for dictionary
  {BATCH_ALIAS,        "/ALIAS=",       "/AL=" },         // alias
  {BATCH_REPORT,       "/REPORT=",      "/RE=" },         // Counting Report
  {BATCH_PROFILE,      "/PROFILE=",     "/PR=" },         // Counting Profile
  {BATCH_STARTPATH,    "/STARTPATH=",   "/ST=" },         // Start path
  {BATCH_CONVERSION,   "/CONV=",        "/CO=" },         // Conversion
  {BATCH_FORMAT,       "/FORMAT=",      "/FO=" },         // reports: format for output file
  {BATCH_SEGMENT,      "/SEGMENT=",     "/SE=" },         // open doc: segment number
  {BATCH_END,          "",              ""       }        // end of list indicator
                } ;

static TASKLIST EQFTaskList[TASK_END+1] =
{
 {TASK_FLDCRT,     "FLDCRT",  DDEFolderCreateParms, FOLDERHANDLER,
                  { BATCH_TASK, BATCH_NAME, BATCH_DESC, BATCH_TODRIVE,
                    BATCH_MEM, BATCH_MARKUP, BATCH_EDIT, BATCH_DICT,
                    BATCH_SRCLNG, BATCH_TGTLNG,
                    BATCH_CONVERSION,
                    BATCH_CURDIR, BATCH_END },
                  { BATCH_NAME, BATCH_TODRIVE, BATCH_SRCLNG, BATCH_TGTLNG,
                    BATCH_MEM, BATCH_MARKUP, BATCH_EDIT, BATCH_END },
                                            },       //create a folder
 {TASK_FLDEXP,     "FLDEXP",  DDEFolderExportParms, FOLDERHANDLER,
                  { BATCH_TASK, BATCH_FLD, BATCH_FILES, BATCH_TODRIVE, BATCH_TOPATH,
                    BATCH_OVERWRITE, BATCH_OPTIONS, BATCH_DESC,
                    BATCH_CURDIR, BATCH_END },
                  { BATCH_FLD, BATCH_TODRIVE, BATCH_END },
                                            },      //export a folder
 {TASK_FLDIMP,     "FLDIMP",  DDEFolderImportParms, FOLDERHANDLER,
                  { BATCH_TASK, BATCH_FLD, BATCH_FROMDRIVE, BATCH_FROMPATH,
                    BATCH_TODRIVE, BATCH_OPTIONS, BATCH_CURDIR, BATCH_END },
                  { BATCH_FLD, BATCH_FROMDRIVE, BATCH_END },
                                            },      //import a folder
 {TASK_WORDCNT,    "WORDCNT", DDEWordCountParms, COUNTHANDLER,
                  { BATCH_TASK, BATCH_FLD, BATCH_FILES, BATCH_OUT,
                    BATCH_OVERWRITE, BATCH_OPTIONS, BATCH_CURDIR, BATCH_END },
                  { BATCH_FLD, BATCH_OUT, BATCH_END },
                                            },      //wordcount a folder
 {TASK_XLATE,      "XLATE",   EQFNot, DOCUMENTHANDLER,
                  { BATCH_TASK, BATCH_CURDIR, BATCH_END },
                  { BATCH_END },
                                            },      //import,translate,export doc
 {TASK_DICIMP,     "DICIMP",  EQFDicImp, DICTIONARYHANDLER,
                  { BATCH_TASK, BATCH_FILES, BATCH_DICT, BATCH_OPTIONS,
                    BATCH_PASSWORD, BATCH_CURDIR, BATCH_END },
                  { BATCH_FILES, BATCH_DICT, BATCH_END },
                                            },      //dictionary import
 // export dictionary
 {TASK_DICEXP,     "DICEXP",  DDEDicExpParms, DICTIONARYHANDLER,
                  { BATCH_TASK, BATCH_OUT, BATCH_DICT,
                    BATCH_OVERWRITE, BATCH_CURDIR, BATCH_END },
                  { BATCH_OUT, BATCH_DICT, BATCH_END } },

 {TASK_DOCIMP,     "DOCIMP",  EQFDocImp, DOCUMENTHANDLER,
                  { BATCH_TASK,BATCH_FLD,BATCH_FILES,BATCH_OVERWRITE,
                    BATCH_MEM, BATCH_MARKUP, BATCH_EDIT, BATCH_SRCLNG,
                    BATCH_TGTLNG, BATCH_ALIAS, BATCH_CURDIR,
                    BATCH_STARTPATH,
                    BATCH_CONVERSION,
                    BATCH_END},
                  { BATCH_FLD, BATCH_FILES, BATCH_END },
                                            },      //document import
 {TASK_DOCEXP,     "DOCEXP",  EQFDocExp, DOCUMENTHANDLER,
                  { BATCH_TASK,BATCH_FLD, BATCH_FILES,
                    BATCH_OPTIONS,BATCH_OVERWRITE, BATCH_CURDIR,
                    BATCH_STARTPATH,
                    BATCH_END },
                  { BATCH_FLD, BATCH_FILES, BATCH_OPTIONS, BATCH_END },
                                            },      //document export
 {TASK_MEMIMP,     "MEMIMP",  DDEMemImpParms, MEMORYHANDLER,
                  { BATCH_TASK, BATCH_FILES, BATCH_TYPE, BATCH_MEM,
                    BATCH_CURDIR, BATCH_END },
                  { BATCH_MEM, BATCH_FILES, BATCH_END },
                                            },      //import translation memory
 {TASK_MEMEXP,     "MEMEXP",  DDEMemExpParms, MEMORYHANDLER,
                  { BATCH_TASK, BATCH_OUT, BATCH_TYPE, BATCH_MEM,
                    BATCH_OVERWRITE, BATCH_CURDIR, BATCH_END },
                  { BATCH_OUT, BATCH_MEM, BATCH_END },
                                            },      //export translation memory
 {TASK_MEMCRT,     "MEMCRT",  DDEMemCrtParms, MEMORYHANDLER,
                  { BATCH_TASK, BATCH_NAME, BATCH_DESC, BATCH_TYPE,
                    BATCH_TODRIVE, BATCH_SRCLNG,
                    BATCH_CURDIR, BATCH_END },
                  { BATCH_NAME, BATCH_TODRIVE, BATCH_SRCLNG,
                    BATCH_END },
                                            },      //create translation memory
 {TASK_LOOKUP,     "LOOKUP",  EQFNot, DOCUMENTHANDLER,
                  { BATCH_TASK, BATCH_CURDIR, BATCH_END },
                  { BATCH_END },
                                            },      //lookup a sentence
 {TASK_ANALYSIS,   "ANALYSIS", DDEAnalysisParms, ANALYSISHANDLER,
                  { BATCH_TASK, BATCH_FLD, BATCH_FILES, BATCH_MEM,
                    BATCH_OPTIONS, BATCH_OVERWRITE, BATCH_CURDIR, BATCH_END },
                  { BATCH_FLD, BATCH_END },
                                            },      //analyse docs or folder
 {TASK_LSTCRT,     "LSTCRT",  EQFNot, LISTHANDLER,
                  { BATCH_TASK, BATCH_CURDIR, BATCH_END },
                  { BATCH_END },
                                            },      //create new/found terms list
 {TASK_STATUS,     "STATUS",  EQFNot, PROFILEHANDLER,
                  { BATCH_TASK, BATCH_CURDIR, BATCH_END },
                  { BATCH_END },
                                            },      //query status info of docs
 {TASK_DOCOPEN,    "DOCOPEN",  DDEDocOpenParms, DOCUMENTHANDLER,
                  { BATCH_TASK, BATCH_FLD, BATCH_NAME, BATCH_SEGMENT, BATCH_CURDIR, BATCH_END },
                  { BATCH_FLD, BATCH_NAME, BATCH_END },
                                            },      //lookup a sentence
 {TASK_MEMDEL,     "MEMDEL",  DDEMemDelParms, MEMORYHANDLER,
                  { BATCH_TASK, BATCH_MEM, BATCH_CURDIR, BATCH_END },
                  { BATCH_MEM, BATCH_END }
                                            },      //delete translation memory
 {TASK_MEMORG,     "MEMORG",  DDEMemOrgParms, MEMORYHANDLER,
                  { BATCH_TASK, BATCH_MEM, BATCH_CURDIR, BATCH_END },
                  { BATCH_MEM, BATCH_END }
                                            },             // organize memory

 // delete folder
 {TASK_FLDDEL,     "FLDDEL",  DDEFolDelParms, FOLDERHANDLER,
                  { BATCH_TASK, BATCH_FLD, BATCH_CURDIR, BATCH_END },
                  { BATCH_FLD, BATCH_END } },

 // delete document
 {TASK_DOCDEL,     "DOCDEL",  DDEDocDelParms, DOCUMENTHANDLER,
                  { BATCH_TASK,BATCH_FLD, BATCH_FILES, BATCH_CURDIR, BATCH_END },
                  { BATCH_FLD, BATCH_FILES, BATCH_END} },


 {TASK_CNTRPT,     "CNTRPT", DDECountReportParms, REPORTHANDLER,
                  { BATCH_TASK, BATCH_FLD, BATCH_FILES, BATCH_OUT,
                    BATCH_REPORT, BATCH_TYPE, BATCH_PROFILE, BATCH_FORMAT,
                    BATCH_OVERWRITE, BATCH_CURDIR, BATCH_END },
                  { BATCH_FLD, BATCH_OUT,
                    BATCH_REPORT, BATCH_TYPE, BATCH_END }
                                            },      //counting report
 {TASK_ARCHTM,   "ARCHTM", DDEArchTMParms, ANALYSISHANDLER,
                  { BATCH_TASK, BATCH_FLD, BATCH_FILES, BATCH_MEM,
                    BATCH_OVERWRITE, BATCH_CURDIR, BATCH_END },
                  { BATCH_FLD, BATCH_MEM, BATCH_END } },

 {TASK_END,        "",        EQFNot, DOCUMENTHANDLER,
                  { BATCH_TASK, BATCH_CURDIR, BATCH_END },
                  { BATCH_END }              }
} ;

BOOL EQFCopyStrip ( PDDECLIENT, PSZ, PSZ, USHORT );
static VOID EQFBothStripDuplSlash(PDDECLIENT, PSZ, PSZ);
static BOOL EQFStripDuplSlash( PSZ);

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEHandlerWP                                             |
//+----------------------------------------------------------------------------+
//|Function call:     MRESULT APIENTRY DDEHANDLERWP                            |
//|                      ( HWND hwnd, USHORT message, WPARAM mp1, LPARAM mp2 ) |
//+----------------------------------------------------------------------------+
//|Description:       Object handler for dde client conversation               |
//+----------------------------------------------------------------------------+
//|Parameters:        HWND    hwnd    handle                                   |
//|                   USHORT  msg     message                                  |
//|                   WPARAM  mp1     message parameter 1                      |
//|                   LPARAM  mp2     message parameter 2                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE                                                     |
//|                   FALSE                                                    |
//+----------------------------------------------------------------------------+
//|Function flow:     switch according to message send                         |
//|                   case WM_CREATE: register instance window class           |
//|                                   if ok and allocate DDEBatch ok           |
//|                                     set access to DDEBatch struct          |
//|                                     if ok                                  |
//|                                        fill IdaHead in DDEBatch            |
//|                                        install DDEHANDLER                  |
//|                                     endif                                  |
//|                                   endif                                    |
//|                                   return (not ok)                          |
//|                   case WM_EQF_SHUTDOWN:                                    |
//|                                   get access to ida                        |
//|                                   check if instances refuse to terminate   |
//|                                   return whether to accept shutdown or not |
//|                   case WM_DDE_INITIATE:                                    |
//|                                   get handle of client who is broadcasting |
//|                                   if not our own broadcast                 |
//|                                     get access to ida                      |
//|                                     get ptr to DDEINIT structure           |
//|                                     if application and topic are supported |
//|                                       create instance window for new client|
//|                                       if ok                                |
//|                                        get ptr to instance structure       |
//|                                        create uniqe instance object name   |
//|                                        if object with this name exists     |
//|                                          set fOk = false                   |
//|                                        else                                |
//|                                          store handle of client            |
//|                                          initialize DDE instance           |
//|                                          inform handlers about new instance|
//|                                        endif                               |
//|                                       else ok = FALSE                      |
//|                                       endif                                |
//|                                     else return error                      |
//|                                     endif                                  |
//|                                   else return error                        |
//|                                   endif                                    |
//|                                   free shared memory                       |
//+----------------------------------------------------------------------------+
MRESULT APIENTRY DDEHANDLERWP ( HWND  hwnd,
                                WINMSG msg,
                                WPARAM mp1,
                                LPARAM mp2 )
{
    PDDEBATCH       pDDEBatch;           //ptr to DDE handler
    HWND            hframe;              //instance window handles
    HWND            hddeservinst = 0;    //handle of dde server instance
    HWND            hddeclient;          //handle of dde client
    ULONG           flStyle;             //style of handler object window
    BOOL            fOK = TRUE;          //success indicator
    MRESULT         mResult = FALSE;     //return code for handler proc
    PDDECLIENT      pDDEClient;          //ptr to DDE instance structure
    ATOM            atomApplication;     // application
    ATOM            atomTopic;           // topic
    CHAR            szApplication[ MAX_DDEAPPL_SZ + 1 ];
    CHAR            szTopic[ MAX_DDETOPIC_SZ + 1 ];
    ATOM            atomApplReturn;
    ATOM            atomTopicReturn;
    OBJNAME         ObjName;             // object name of instance

    switch( msg )
    {
       //----------------------------------------------------------------------
       case WM_CREATE:
          //--- allocate storage for handler instance area, if fails
          //--- display error message
          fOK = WinRegisterClass( (HINSTANCE) WinQueryAnchorBlock( hwnd),
                                  WC_EQF_DDE, DDEINSTWNDPROC, CS_SIZEREDRAW,
                                  sizeof(PVOID) );
          if ( fOK && UtlAlloc( (PVOID *)&pDDEBatch, 0L, (LONG) sizeof( DDEBATCH ),
                                ERROR_STORAGE ) )
          {
             //--- set access to DDEBatch structure
             ANCHORWNDIDA( hwnd, pDDEBatch );

             //--- save handler frame handle to ida
             pDDEBatch->IdaHead.hFrame = hwnd;
             //--- save handler object name to ida
             strcpy( pDDEBatch->IdaHead.szObjName, DDEHANDLER );

             //--- save pointer to object name to ida (will be removed later)
             pDDEBatch->IdaHead.pszObjName = pDDEBatch->IdaHead.szObjName;

             //--- if error registering DDE handler (NULL = no error )
             if ( EqfInstallHandler( DDEHANDLER, hwnd, clsDDE ) )
             {
                fOK = FALSE;
             }/*endif*/
          }
          else                               //error UtlAlloc
          {
             //--- stop further processing
             fOK = FALSE;
          }/*endif*/

          //--- if previous error set return velue

          mResult = (MRESULT)!fOK;  //FALSE: system should continue creating the window
                                    //TRUE : continue not
          break;
       //----------------------------------------------------------------------
       case WM_EQF_SHUTDOWN:                // come here before EQF_TERMINATE

          //--- TRUE  : reject WM_EQF_SHUTDOWN request
          //--- FALSE : accept WM_EQF_SHUTDOWN request
          mResult = (MRESULT)( EqfSend2AllObjects( clsDDE, msg, mp1, mp2 ));
          break;
       //----------------------------------------------------------------------
       case WM_DDE_INITIATE:         // a client is broadcasting to find a server
         /*************************************************************/
         /* is this our own broadcast? then don't process it          */
         /*************************************************************/
         hddeclient = (HWND)mp1;
         if (hddeclient != hwnd)
         {
           pDDEBatch = ACCESSWNDIDA( hwnd, PDDEBATCH );
           /**************************************************************/
           /* create ptr to client's specified appplication and topic    */
           /**************************************************************/
           if ( (atomApplication = LOWORD( mp2 )) != 0 )
           {
             GlobalGetAtomName(atomApplication, szApplication, MAX_DDEAPPL_SZ);
           }
           else
           {
             szApplication[0] = EOS;
           } /* endif */
           if ( (atomTopic = HIWORD( mp2 )) != 0 )
           {
             GlobalGetAtomName( atomTopic, szTopic, MAX_DDETOPIC_SZ);
           }
           else
           {
             szTopic[0] = EOS;
           } /* endif */
           /**************************************************************/
           /* if application and topic name can be supported...          */
           /**************************************************************/
           if (! strcmp(CV_DDEAPPNAME, szApplication))
           {
             if ( !strcmp(CV_DDETOPIC, szTopic))
             {                          // Server and topic names matched
                /******************************************************/
                /* build objectname, concatenate topicname with       */
                /* hddeclient to get unique instance object name      */
                /******************************************************/
                 sprintf(ObjName,"%s\\%ld", CV_DDETOPIC,hddeclient);
                /******************************************************/
                /*  check whether object with this name exists        */
                /******************************************************/
                hframe = EqfQueryObject(ObjName,clsDDE,0);
                if ( hframe )
                {
                  fOK = FALSE;          // do not connect again
                }
                else
                {
                  /****************************************************/
                  /* register object and create instance window       */
                  /****************************************************/
                  flStyle = FCF_TITLEBAR | FCF_SIZEBORDER | FCF_MINMAX;
                  {
                    hddeservinst = hframe =
                      CreateWindow (WC_EQF_DDE,
                                     NULL,    //window caption
                                     WS_OVERLAPPED | WS_CLIPSIBLINGS | flStyle,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     EqfQueryTwbClient(),
                                     NULL,
                                     (HINSTANCE) UtlQueryULong( QL_HAB ),
                                      NULL );

                  }

                  if ( !hframe)
                  {
                    fOK = FALSE;
                  }
                  else if ( EqfRegisterObject
                      ( ObjName, hddeservinst, clsDDE ) )
                  {
                     //--- stop further processing
                     fOK = FALSE;
                  }
                } /* endif */
                if ( fOK )
                {
                  /****************************************************/
                  /* fill instance ida (handle of DDEclient,          */
                  /* DDEinstance, topic,objname )                     */
                  /****************************************************/
                   pDDEClient = ACCESSWNDIDA(hddeservinst, PDDECLIENT );
                   pDDEClient->hwndDDEClient = hddeclient;
                   pDDEClient->hwndErrMsg = hddeservinst;
                   strcpy(pDDEClient->szTopic, szTopic);
                   //--- get access to dde instance ida
                   pDDEClient->hwndInstance = hddeservinst;
                   strcpy(pDDEClient->IdaHead.szObjName,ObjName);
                   pDDEClient->IdaHead.pszObjName =
                                  pDDEClient->IdaHead.szObjName;
                   /***************************************************/
                   /* respond to DDE client                           */
                   /***************************************************/

                   atomApplReturn   = GlobalAddAtom(szApplication);
                   atomTopicReturn = GlobalAddAtom(szTopic);
                   SendMessage( pDDEClient->hwndDDEClient, WM_DDE_ACK,
                                (WPARAM)hddeservinst,
                                MAKELPARAM( atomApplReturn, atomTopicReturn));

                   /***************************************************/
                   /* inform all handler that a new DDE instance crea */
                   /***************************************************/
                   EqfSend2AllHandlers( WM_EQFN_CREATED,
                                        MP1FROMSHORT( clsDDE ),
                                        MP2FROMP(pDDEClient->IdaHead.szObjName));


                } /* endif */
             } /* endif */
           } /* endif */
           if ( !fOK )
           {
             mResult = WinDefWindowProc(hwnd, msg, mp1, mp2);
           } /* endif */
         } /* endif */
         break;
       case WM_DESTROY:
         /*************************************************************/
         /* free allocation for this batch process...                 */
         /*************************************************************/
         pDDEBatch = ACCESSWNDIDA( hwnd, PDDEBATCH );
         if ( pDDEBatch )
         {
           UtlAlloc( (PVOID *)&pDDEBatch, 0L, 0L, NOMSG );
           ANCHORWNDIDA( hwnd, pDDEBatch );
         } /* endif */
         break;
       default:
         mResult = WinDefWindowProc(hwnd, msg, mp1, mp2);
         break;

    }/*end switch*/

    return( mResult );

}/*end DDEHandler_WP*/

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEINSTWNDPROC                                           |
//+----------------------------------------------------------------------------+
//|Function call:     MRESULT APIENTRY DDEINSTWNDPROC(HWND hwnd,               |
//|                                               USHORT message,              |
//|                                               WPARAM mp1,                  |
//|                                               LPARAM mp2      )            |
//+----------------------------------------------------------------------------+
//|Description:       DDEclient  instance window procedure                     |
//+----------------------------------------------------------------------------+
//|Parameters:         HWND    hwnd    window handle                           |
//|                    USHORT  msg     message                                 |
//|                    WPARAM  mp1     message parameter 1                     |
//|                    LPARAM  mp2     message parameter 2                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   MRESULT                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE                                                     |
//|                   FALSE                                                    |
//+----------------------------------------------------------------------------+
//|Function flow:    WM_CREATE:                                                |
//|                     allocate storage for DDE instance ida                  |
//|                     if ok                                                  |
//|                       connect ida to instance window                       |
//|                       if ok                                                |
//|                         fill IdaHead of DDE instance ida                   |
//|                       endif                                                |
//|                     endif                                                  |
//|                  WM_CLOSE:                                                 |
//|                     remove DDE instance                                    |
//|                     post WM_QUIT                                           |
//|                  WM_EQF_SHUTDOWN:                                          |
//|                  WM_DESTROY:                                               |
//|                     get access to dde instance ida                         |
//|                     free it                                                |
//|                  WM_EQF_TERMINATE:                                         |
//|                     get access to instance ida                             |
//|                     if not allowed                                         |
//|                       reject termination                                   |
//|                     else                                                   |
//|                       deactivate instance                                  |
//|                       destrow window                                       |
//|                  WM_DDE_REQUEST:                                           |
//|                    get ptr to DDESTRUCT                                    |
//|                    if ok                                                   |
//|                      get ptr to instance ida                               |
//|                      copy cmdline in instance ida                          |
//|                      check cmdline                                         |
//|                      if ok                                                 |
//|                        post message to nec handler                         |
//|                      else                                                  |
//|                        return structure with error code                    |
//|                      endif                                                 |
//|                      free shared memory                                    |
//|                    endif                                                   |
//|                  WM_EQF_DDE_ANSWER:                                        |
//|                    get ptr to instance ida                                 |
//|                    copy return message to instance                         |
//|                    post WM_DDE_DATA to DDE client                          |
//|                  WM_DDE_TERMINATE:                                         |
//|                    send WM_DDE_TERMINATE back to client                    |
//|                    close instance                                          |
//+----------------------------------------------------------------------------+
MRESULT APIENTRY DDEINSTWNDPROC ( HWND   hwnd,
                            WINMSG msg,
                            WPARAM mp1,
                            LPARAM mp2      )
{
    MRESULT          mResult = FALSE;       // rc for instance proc initialized
    BOOL             fOK = TRUE;            // error indicator
    PDDECLIENT       pDDEClient;            // client instance structure
    HGLOBAL       hDDEOut;
    ATOM          atomItem;

    switch ( msg)
    {
       //----------------------------------------------------------------------
       case WM_CREATE:
          //--- allocate storage for DDE instance ida
          fOK = UtlAlloc( (PVOID *)&pDDEClient, 0L, (LONG) sizeof( DDECLIENT),
                            NOMSG);
          if ( fOK )
          {
             //--- connect dde instance window with ida
             ANCHORWNDIDA( hwnd, pDDEClient );

             //--- save frame handle of dde inst window to instance header
             pDDEClient->IdaHead.hFrame = hwnd;
          }/*endif*/

          // reset QS_LASTDDEMSG to zero

          UtlSetUShort(QS_LASTDDEMSGID,0);

          mResult =  (MRESULT) !fOK;
          break;
       //----------------------------------------------------------------------
       case WM_CLOSE:
          //--- tell the dde handler to remove dde instance
          EqfRemoveObject( TWBFORCE, hwnd );
          break;
       //----------------------------------------------------------------------
       case WM_EQF_SHUTDOWN:                //msg from MAT main to ask for
                                            //shutdown
          /************************************************************/
          /* reject shutdown - we are still working ....              */
          /************************************************************/
          mResult = (MRESULT) TRUE ;
          break;
       //----------------------------------------------------------------------
       case WM_DESTROY:
          //--- get access to dde instance ida
          pDDEClient = ACCESSWNDIDA( hwnd, PDDECLIENT );
          if ( pDDEClient )
          {
             // clear any message for this DDE object
             UtlGetLastError( pDDEClient->hwndErrMsg, pDDEClient->DDEReturn.chMsgBuf );

             //--- free dde instance ida
             UtlAlloc( (PVOID *)&pDDEClient, 0L, 0L, NOMSG) ;
          }/*endif*/
          break;
       //----------------------------------------------------------------------
       case WM_EQF_TERMINATE:
          //--- mp1: SHORT1 = close flag SHORT2 = NULL
          //--- mp2: NULL

          //--- get access to dde instance ida
          pDDEClient = ACCESSWNDIDA( hwnd, PDDECLIENT );

          //--- if close flag is not TWBFORCE and dde instance close
          //--- flag is set
          if( !( SHORT1FROMMP1( mp1 ) & TWBFORCE ) && pDDEClient->IdaHead.fMustNotClose )
          {
             //--- reject termination
             mResult = (MRESULT)TRUE;
          }
          else
          {
             //--- tell MATMAIN that dde instance is deactivated
             EqfActivateInstance( hwnd, FALSE );
             //--- now destroy the child windows and the dde instance window
             WinDestroyWindow( pDDEClient->IdaHead.hFrame );
          }/*end if*/
          break;
       //----------------------------------------------------------------------
   case WM_DDE_POKE:
     {
         HANDLE        hPokeData;
         DDEPOKE FAR * lpPokeData;
         BOOL          bRelease;

         UnpackDDElParam( WM_DDE_POKE, mp2, (PUINT)&hPokeData, (PUINT)&atomItem );
         FreeDDElParam( WM_DDE_POKE, mp2 );
         pDDEClient = ACCESSWNDIDA( hwnd, PDDECLIENT );

         GlobalGetAtomName( atomItem, pDDEClient->szItem, MAX_DDEITEM_SZ );
         if ((lpPokeData = (DDEPOKE FAR *)GlobalLock(hPokeData)) != NULL)
         {
           strncpy(pDDEClient->szCmdLine, (PSZ)&lpPokeData->Value[0], MAX_CMDLINELEN );

           if ( !strcmp(CV_DDEITEM, pDDEClient->szItem ))
           {
             /**********************************************************/
             /* check cmdline                                          */
             /**********************************************************/
             fOK = EQFFunctionCmdLine( pDDEClient );
             if ( fOK )
             {
               /********************************************************/
               /* post msg to active handler                           */
               /********************************************************/
               EqfPost2Handler( pDDEClient->szHandler,
                                WM_EQF_DDE_REQUEST,
                                MP1FROMSHORT (pDDEClient->Task),
                                MP2FROMP(pDDEClient->pTaskIda) );
               if ( atomItem )
               {
                 GlobalDeleteAtom(atomItem);
               } /* endif */
             }
             else
             {
               /********************************************************/
               /* return structure filled with error codes            */
               /********************************************************/
               UtlGetLastError( pDDEClient->hwndErrMsg, pDDEClient->DDEReturn.chMsgBuf );
               {
                  LONG lPackedVal = PackDDElParam( WM_DDE_ACK, 0x8000, atomItem );
                 PostMessage(pDDEClient->hwndDDEClient,
                             WM_DDE_ACK, (WPARAM)hwnd,
                             lPackedVal);
               }
             } /* endif */
           }
           else
           {
             /*********************************************************/
             /* item not supported, pass back negative WM_DDE_ACK     */
             /* message , pass back same atomItem                     */
             /*********************************************************/
           {
              LONG lPackedVal = PackDDElParam( WM_DDE_ACK, 0, atomItem );
             PostMessage(pDDEClient->hwndDDEClient,
                         WM_DDE_ACK, (WPARAM)hwnd,
                         lPackedVal);
           }

           } /* endif */
           /* Save value of fRelease, since pointer may be invalidated by */
           /*  GlobalUnlock()                                            */
           bRelease = lpPokeData->fRelease;
           GlobalUnlock(hPokeData);

           if (bRelease)
           {
               GlobalFree(hPokeData);
           } /* endif */
         } /* endif */
     }
     break;
    /******************************************************************/
    /* a one time data request for  a task arrives                    */
    /******************************************************************/
    case WM_DDE_REQUEST :
      /****************************************************************/
      /* check that item is ok and connected client                   */
      /* then return error structure                                  */
      /****************************************************************/
      atomItem = HIWORD( mp2 );

      pDDEClient = ACCESSWNDIDA( hwnd, PDDECLIENT );

      GlobalGetAtomName( atomItem, pDDEClient->szItem, MAX_DDEITEM_SZ );

      if ( !strcmp(CV_DDEITEM, pDDEClient->szItem ))
      {
        hDDEOut = MakeDDEDataSeg(CF_TEXT, CV_DDEITEM,
                                 (PVOID) &pDDEClient->DDEReturn,
                                 sizeof( pDDEClient->DDEReturn), 0);
        {
          LONG lPackedVal = PackDDElParam( WM_DDE_DATA, (UINT)hDDEOut, (UINT)atomItem );
          PostMessage( pDDEClient->hwndDDEClient, WM_DDE_DATA, (WPARAM)hwnd,
                       lPackedVal );
        }
     }
     else
     {
        GlobalDeleteAtom(atomItem);
     } /* endif */
      break;

    case  WM_EQF_DDE_ANSWER :
      /****************************************************************/
      /* answer from handler arrives                                  */
      /****************************************************************/
      pDDEClient = ACCESSWNDIDA( hwnd, PDDECLIENT );
      /****************************************************************/
      /* get rc   with memcpy, fill chMsgBuf with UtlGetLastError     */
      /****************************************************************/
      memcpy(&pDDEClient->DDEReturn,(PDDERETURN) mp2, sizeof( DDERETURN));
      UtlGetLastError( pDDEClient->hwndErrMsg, pDDEClient->DDEReturn.chMsgBuf );
      /****************************************************************/
      /* build DDEStruct                                              */
      /****************************************************************/
      atomItem  = GlobalAddAtom(pDDEClient->szItem);
      {
        LONG lPackedVal = PackDDElParam( WM_DDE_ACK, 0x8000, atomItem );
        PostMessage(pDDEClient->hwndDDEClient,
                    WM_DDE_ACK, (WPARAM)hwnd,
                    lPackedVal);
      }
      break;
    case  WM_DDE_TERMINATE :
      /****************************************************************/
      /* DDE conversation termination request                         */
      /****************************************************************/
      pDDEClient = ACCESSWNDIDA( hwnd, PDDECLIENT );
      PostMessage( pDDEClient->hwndDDEClient, WM_DDE_TERMINATE, (WPARAM)hwnd, 0L );
      WinPostMsg(hwnd, WM_CLOSE, 0L, 0L);
      break;

    default:
      mResult = WinDefWindowProc(hwnd, msg, mp1, mp2);
      break;

    }/*end switch*/

    return( mResult );

}/*end DDEINSTWNDPROC*/

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MakeDDEDataSeg                                           |
//+----------------------------------------------------------------------------+
//|Function call:     MakeDDEDataSeg(USHORT usFormat,PSZ pszItemName,PVOID     |
//|                                 pvData,USHORT cbData,USHORT fsStatus)      |
//+----------------------------------------------------------------------------+
//|Description:       build DDESTRUCT for WM_DDE_REQUEST(with data string)     |
//+----------------------------------------------------------------------------+
//|Input parameter:   USHORT usFormat   format to fill in DDESTRUCT            |
//|                   PSZ pszItemName   Item Name                              |
//|                   PVOID pvData      data string to be combined to DDESTRUCT|
//|                   USHORT cbData     length of pvData                       |
//|                   USHORT fsStatus   Status for DDESTRUCT                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   PDDESTRUCT       ptr to the created & filled DDESTRUCT   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL if error                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     - allocate givable memory segment                        |
//|                   - fill in the new DDE struct                             |
//+----------------------------------------------------------------------------+

HGLOBAL MakeDDEDataSeg
(
  USHORT usFormat,
  PSZ pszItemName,
  PVOID pvData,
  USHORT cbData,
  USHORT fsStatus
)
{
  HGLOBAL sel = NULL;
  USHORT cbSegSize = 0;                // count of bytes to request
  DDEDATA FAR * lpData = NULL;         // string ptr for text format

  fsStatus;
  /**************************************************************************/
  /* Allocate a givable memory segment                                      */
  /**************************************************************************/
  cbSegSize = sizeof(DDEDATA)+(USHORT)strlen(pszItemName)+1+cbData;
  sel = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, cbSegSize );
  if ( sel )                     // check api return code and selector
  {
    /******************************************************************/
    /* lock the data ...                                              */
    /******************************************************************/
    if ((lpData = (DDEDATA FAR*)GlobalLock( sel )) == NULL)
    {
      GlobalFree( sel );
      sel = NULL;
    }
    else
    {
      lpData->fAckReq = FALSE;
      lpData->cfFormat = usFormat;
      lpData->fResponse = FALSE;
      lpData->fRelease = TRUE;
      if (pvData && cbData )
      {
        memcpy(lpData->Value, pvData, (size_t)cbData);
      }
      else
      {
        memset(lpData->Value, 0, cbData);
      } /* endif */
      GlobalUnlock( sel );
    } /* endif */
  }
  return  sel;

} /* end of function MakeDDEDataSeg */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFFunctionCmdLine                                       |
//+----------------------------------------------------------------------------+
//|Function call:     EQFFunctionCmdLine( PDDECLIENT );                        |
//+----------------------------------------------------------------------------+
//|Description:       get and analyze the commandline                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT pDDEClient   client ida                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   commandline is correct                            |
//|                   FALSE  commandline has an error in it                    |
//+----------------------------------------------------------------------------+
//|Function flow:     copy cmdline in clientida                                |
//|                   validate task                                            |
//|                   get batchlist and mandatory list for current task        |
//|                   if task specified                                        |
//|                     execute function specified in task                     |
//|                   endif                                                    |
//+----------------------------------------------------------------------------+

static BOOL
EQFFunctionCmdLine
(
  PDDECLIENT pDDEClient               // ptr to client ida
)
{
  BOOL  fOK = FALSE;                  // success indicator
  BOOL (*function)( PDDECLIENT );    // and function to be processed

  /****************************************************************/
  /* append '/' to allow for easier processing of last token.     */
  /****************************************************************/
  strcat( pDDEClient->szCmdLine, DELSLASH_STR );
  strcpy( pDDEClient->szOrgCmdLine, pDDEClient->szCmdLine );
  strupr( pDDEClient->szCmdLine );
  pDDEClient->DDEReturn.usRc = 0;

  /******************************************************************/
  /* tokenize and analyze it ...                                    */
  /******************************************************************/
  pDDEClient->Task = ValidateTask(pDDEClient);
  pDDEClient->pBatchList = EQFTaskList[pDDEClient->Task].BatchList;
  pDDEClient->pMandList = EQFTaskList[pDDEClient->Task].MandList;
  strcpy(pDDEClient->szHandler, EQFTaskList[pDDEClient->Task].szHandler );
  if ( pDDEClient->Task != TASK_END )
  {
    function = EQFTaskList[pDDEClient->Task].function;
    fOK = (*function)(pDDEClient);                //execute the function
  } /* endif */

  /******************************************************************/
  /* the commandline buffer will be freed at the end of the program */
  /******************************************************************/

  return( fOK );
} /* end of function CheckCmdLine */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ValidateTask                                             |
//+----------------------------------------------------------------------------+
//|Function call:     ValidateTask(PDDECLIENT)                                 |
//+----------------------------------------------------------------------------+
//|Description:       loop through given commandline to find specified task    |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT pDDEClient                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   DDETASK                                                  |
//+----------------------------------------------------------------------------+
//|Returncodes:       matching task or end of task list                        |
//+----------------------------------------------------------------------------+
//|Function flow:     get ptr to CmdLine and to List of Task-Descriptions      |
//|                   find description of task in cmdline ("/TASK=")           |
//|                   if found                                                 |
//|                     while not end of list of available tasks               |
//|                       if specification is current task                     |
//|                         set return to identifier of this task              |
//|                       else                                                 |
//|                         set ptr to next task in list                       |
//|                       endif                                                |
//|                     endwhile                                               |
//|                     if not found                                           |
//|                       set error msg to WRONGTASK                           |
//|                     endif                                                  |
//|                   else                                                     |
//|                     set error msg to NOTASK                                |
//|                   endif                                                    |
//|                   return task identifier                                   |
//+----------------------------------------------------------------------------+

static DDETASK
ValidateTask
(
  PDDECLIENT pDDEClient             //ptr to client ida
)
{
  PTASKLIST  pEQFTaskList;          //ptr to list with tasks / batchlists
  BOOL  fFound = FALSE;             //success indicator
  PSZ   pDDECmdData;                //temp ptr to cmdline
  PSZ   pszCmdDesc;                 //description string of task (/TASK=)
  PSZ   pTaskStr;                   //ptr where task found in cmdline
  DDETASK  TaskCmd = (DDETASK)0;             //task identifier to be returned
  USHORT  usLen;

  pEQFTaskList = &EQFTaskList[0];
  /********************************************************************/
  /* find task of this request                                        */
  /********************************************************************/
  pDDECmdData = pDDEClient->szCmdLine;
  pszCmdDesc = EQFCmdList[BATCH_TASK].szDesc;
  pTaskStr = strstr(pDDECmdData,pszCmdDesc);
  if ( !pTaskStr )
  {
    pszCmdDesc = EQFCmdList[BATCH_TASK].szShortCut;
    pTaskStr = strstr(pDDECmdData,pszCmdDesc);
  } /* endif */
  if ( pTaskStr )
  {
    pDDECmdData = pTaskStr + strlen(pszCmdDesc);
    while ( !fFound && ( pEQFTaskList->TaskCmd != TASK_END) )
    {
      usLen = (USHORT)strlen(pEQFTaskList->szDesc);
      if ( (!strncmp( pEQFTaskList->szDesc, pDDECmdData, usLen))
             && ( *(pDDECmdData+usLen) == DELSLASH )  )
      {
        fFound = TRUE;
        TaskCmd = pEQFTaskList->TaskCmd;
      }
      else
      {
        pEQFTaskList ++;
      } /* endif */
    } /* endwhile */
    /******************************************************************/
    /* check if a valid task specified                                */
    /******************************************************************/
    if ( !fFound )
    {
      /****************************************************************/
      /* error not available task specified                           */
      /****************************************************************/
      pDDEClient->DDEReturn.usRc = DDE_WRONGTASK;

      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                       NULL, EQF_ERROR, pDDEClient->hwndErrMsg );

      TaskCmd = TASK_END;
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* error no task specified                                        */
    /******************************************************************/
    pDDEClient->DDEReturn.usRc = DDE_NOTASK;
    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                       NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
    TaskCmd = TASK_END;
  } /* endif */
  return (TaskCmd);
} /* end of function ValidateTask */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ValidateToken                                            |
//+----------------------------------------------------------------------------+
//|Function call:     ValidateToken( ppToken, pBatchList );                    |
//+----------------------------------------------------------------------------+
//|Description:       loop through list of passed command list and try to find |
//|                   matching token; return address to datapart of this token |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ *ppToken    pointer to token                         |
//|                   PBATCHLIST pBatchList ptr to comnd list of current task  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BATCHCMD                                                 |
//+----------------------------------------------------------------------------+
//|Returncodes:       matching command line parameter or end of list           |
//+----------------------------------------------------------------------------+
//|Function flow:     get pointer to data                                      |
//|                   while not end of available commands                      |
//|                     compare data with command string                       |
//|                     if match found                                         |
//|                       set token pointer to data part of command            |
//|                     else                                                   |
//|                       increase list pointer                                |
//|                     endif                                                  |
//|                   endwhile                                                 |
//|                   return matching command                                  |
//+----------------------------------------------------------------------------+

static BATCHCMD
ValidateToken
(
  PSZ  *ppToken,                     // token to be analysed
  PBATCHLIST  pBatchList             // pointer to cmd list of task
)
{
  BOOL fFound = FALSE;              // success indicator
  PSZ  pData = *ppToken;
  PSZ  pszTempDesc;                  //description of temp batchcmd

  /********************************************************************/
  /* Note: we can loop through commands without the need of any fancy */
  /*       search algorithm because there are not as many commands and*/
  /*       we do it only once!                                        */
  /********************************************************************/
  while ( !fFound && pBatchList->BatchCmd != BATCH_END)
  {
    pszTempDesc = EQFCmdList[pBatchList->BatchCmd].szDesc;
    if ( !strncmp(pszTempDesc, pData, strlen(pszTempDesc) ))
    {
      fFound = TRUE;
      *ppToken += strlen( pszTempDesc );   // return data area of token
    }
    else
    {
      pszTempDesc = EQFCmdList[pBatchList->BatchCmd].szShortCut;
      if ( !strncmp(pszTempDesc, pData, strlen(pszTempDesc) ))
      {
        fFound = TRUE;
        *ppToken += strlen( pszTempDesc );   // return data area of token
      }
      else
      {
        pBatchList++;
      } /* endif */
    } /* endif */
  } /* endwhile */

  return( pBatchList->BatchCmd );
} /* end of function ValidateToken */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNot                                                   |
//+----------------------------------------------------------------------------+
//|Function call:     EQFNot(PDDECLIENT)                                       |
//+----------------------------------------------------------------------------+
//|Description:       dummy function for not implemented tasks                 |
//+----------------------------------------------------------------------------+
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   succcess indicator                                |
//+----------------------------------------------------------------------------+
//|Function flow:     set return not implemented yet                           |
//+----------------------------------------------------------------------------+

BOOL EQFNot
(
 PDDECLIENT pDDEClient
)
{
  pDDEClient->DDEReturn.usRc = DDE_NOTIMPLEMENTED;
  UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                       NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  return( FALSE );
} /* end of function EQFNot */
//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFDocImp                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFDocImp(PDDECLIENT);                                   |
//+----------------------------------------------------------------------------+
//|Description:       do handling if task is Document import                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for DocImport structure                  |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD:                                   |
//|                           copy foldername in docimp structure              |
//|                          case BATCH_FILES:                                 |
//|                           prepare list of files                            |
//|                           get number of files given                        |
//|                           store ptr to each filename                       |
//|                          case BATCH_OVERWRITE:                             |
//|                           if YES specified, set flag = TRUE                |
//|                           if NO specified, set flag= FALSE                 |
//|                           else set return WRONGCMDLINE                     |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+

BOOL
EQFDocImp
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                       // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDOCIMPEXP pDocImpExp;              //document import /export struct
  USHORT usI;
  PSZ   * ppListIndex;
  PSZ   * ppCurDirListIndex;
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;
  CHAR    chBuffer[MAX_STRINGLEN];

  int   DeltaBeginStart;               // Delta from pBegin to pStart
  PSZ   pBegin;                        // pointer to begin of command line
  PSZ   pOriginal;                     // pointer to original command line
                                       // (up and downcase)

  /********************************************************************/
  /* allocate memory for DocImport structure                          */
  /********************************************************************/

  fOK = UtlAlloc( (PVOID *)&pDocImpExp, 0L, (LONG) sizeof(DOCIMPEXP), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in Doc import instance                  */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pDocImpExp;
    pDocImpExp->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;

    // pointer to Original Command Line
    pBegin        = pStart;
    pOriginal     = pDDEClient->szOrgCmdLine;

    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;                       //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {

              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                /****************************************/
                /*  strip one of the duplicated slashes */
                /****************************************/
                EQFBothStripDuplSlash(pDDEClient, pStart, pActive);
                pDocImpExp->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy(pDocImpExp->chFldName, pStart);
                      UtlStripBlanks(pDocImpExp->chFldName);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;
                  case  BATCH_FILES:
                    if ( *pStart == LISTINDICATOR )
                    {
                      /****************************************************/
                      /* store the list file                              */
                      /****************************************************/
                      fOK = EQFCopyStrip(pDDEClient, pStart + 1,
                                   pDocImpExp->chListName,
                                   sizeof(pDocImpExp->chListName) );
                    }
                    else
                    {
                      /**************************************************/
                      /* work against list of files                     */
                      /**************************************************/
                      // Set proper pointers
                      // pBegin and pOriginal are in parallel
                      DeltaBeginStart=pStart-pBegin;
                      pOriginal[strlen(pBegin)]=EOS;
                      // uses lower and upper case characters
                      fOK = UtlValidateList( pOriginal+DeltaBeginStart,
                          &ppListIndex, MAX_DDE_FILES );

                      if ( fOK )
                      {
                        /**************************************************/
                        /* get number of files given                      */
                        /* store ptr to each filename                     */
                        /**************************************************/
                        usI = 0;
                        while ( ppListIndex[usI] )
                        {
                          usI++;
                        } /* endwhile */
                        pDocImpExp->ppFileArray = ppListIndex;
                        pDocImpExp->usFileNums = usI;
                      }
                      else
                      {
                        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */

                    break;
                  case  BATCH_OVERWRITE:                            /* KIT1288c */
                    /******************************************************/
                    /* set fOverwrite to yes or no, set error if other     */
                    /* string is specified for Confirm                    */
                    /******************************************************/
                    fOK = EQFCopyStrip(pDDEClient, pStart,
                                 chBuffer, MAX_STRINGLEN );
                    if (fOK )
                    {
                      if ( !(stricmp(chBuffer, YES_STRING)) )
                      {
                        pDocImpExp->fOverwrite = TRUE;
                      }
                      else if ( (stricmp(chBuffer, NO_STRING)) != 0 )
                      {
                        fOK = FALSE;
                        pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */
                    break;

                  case  BATCH_MEM:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pDocImpExp->szMem, pStart);
                      strupr( pDocImpExp->szMem );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_MARKUP:
                    if ( strlen( pStart ) < MAX_FNAME )
                    {
                      strcpy( pDocImpExp->szFormat, pStart);
                      strupr( pDocImpExp->szFormat );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_EDIT:
                    if ( strlen( pStart ) < MAX_FNAME )
                    {
                      strcpy( pDocImpExp->szEdit, pStart);
                      strupr( pDocImpExp->szEdit );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_ALIAS:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      ULONG  ulLen = strlen(pStart);
                      PSZ     pszOrgStart = pDDEClient->szOrgCmdLine +
                                            (pStart - pDDEClient->szCmdLine);
                      strncpy( pDocImpExp->szAlias, pszOrgStart, ulLen );
                      pDocImpExp->szAlias[ulLen] = EOS;
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_SRCLNG:
                    if ( strlen( pStart ) <= MAX_LANG_LENGTH )
                    {
                      strcpy( pDocImpExp->szSourceLang, pStart);
                      UtlUpper( pDocImpExp->szSourceLang );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TGTLNG:
                    if ( strlen( pStart ) <= MAX_LANG_LENGTH )
                    {
                      strcpy( pDocImpExp->szTargetLang, pStart);
                      UtlUpper( pDocImpExp->szTargetLang );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_STARTPATH:
                    if ( strlen( pStart ) < sizeof(pDocImpExp->szStartPath) )
                    {
                      strcpy( pDocImpExp->szStartPath, pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_CONVERSION:
                    if ( strlen( pStart ) < MAX_DESCRIPTION )
                    {
                      strcpy( pDocImpExp->szConversion, pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;
                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    fOK = UtlValidateList( pStart,
                                       &ppCurDirListIndex,MAX_DRIVELIST );
                    if (fOK )
                    {
                      pDocImpExp->ppCurDirArray = ppCurDirListIndex;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */

                    break;
                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;
        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether the passed list is found and valid               */
    /******************************************************************/
    if ( fOK )
    {
      /****************************************************************/
      /* if list name specified, load the list                        */
      /****************************************************************/
      if ( pDocImpExp->chListName[0] )
      {
        fOK = UtlListOfFiles( &ppCurDirListIndex, pDocImpExp->chListName, &ppListIndex );
        if ( fOK )
        {
          /**************************************************/
          /* get number of files given                      */
          /* store ptr to each filename                     */
          /**************************************************/
          usI = 0;
          while ( ppListIndex[usI] )
          {
            usI++;
          } /* endwhile */
          pDocImpExp->ppFileArray = ppListIndex;
          pDocImpExp->usFileNums = usI;
        }
        else
        {
          pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
          pData = EQFCmdList[BATCH_FILES].szDesc;
          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
        } /* endif */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pDocImpExp->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pDocImpExp->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pDocImpExp->ppFileArray, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *)&pDocImpExp, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function EQFDocImp */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFDocExp                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFDocExp(PDDECLIENT);                                   |
//+----------------------------------------------------------------------------+
//|Description:       do handling if task is Document export                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for DocImpExp structure                  |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD:                                   |
//|                           copy foldername in docimp structure              |
//|                          case BATCH_FILES:                                 |
//|                           prepare list of files                            |
//|                           get number of files given                        |
//|                           store ptr to each filename                       |
//|                          case BATCH_OPTIONS:                               |
//|                           if SOURCE specified, set flag = TRUE             |
//|                           if TARGET specified, set flag = FALSE            |
//|                           else set return WRONCMDLINE                      |
//|                          case BATCH_OVERWRITE:                             |
//|                           if YES specified, set flag = TRUE                |
//|                           if NO specified, set flag= FALSE                 |
//|                           else set return WRONGCMDLINE                     |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+

BOOL
EQFDocExp
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                       // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDOCIMPEXP pDocImpExp;              //document import /export struct
  USHORT usI;
  PSZ   * ppListIndex;
  PSZ   * ppCurDirListIndex;
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;
  CHAR    chBuffer[MAX_STRINGLEN];

  int   DeltaBeginStart;               // Delta from pBegin to pStart
  PSZ   pBegin;                        // pointer to begin of command line
  PSZ   pOriginal;                     // pointer to original command line
                                       // (up and downcase)

  /********************************************************************/
  /* allocate memory for DocImpExp structure                          */
  /********************************************************************/

  fOK = UtlAlloc( (PVOID *)&pDocImpExp, 0L, (LONG) sizeof(DOCIMPEXP), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in Doc export instance                  */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pDocImpExp;
    pDocImpExp->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;

    // pointer to Original Command Line
    pBegin        = pStart;
    pOriginal     = pDDEClient->szOrgCmdLine;

    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              pDocImpExp->usBatchOccurrence[ BatchCmd ]++;
              EQFBothStripDuplSlash(pDDEClient, pStart, pActive);

              switch ( BatchCmd )
              {
                case  BATCH_FLD:
                  fOK = EQFCopyStrip(pDDEClient, pStart,
                               pDocImpExp->chFldName, MAX_LONGFILESPEC );
                  break;
                case  BATCH_FILES:
                  if ( *pStart == LISTINDICATOR )
                  {
                    /****************************************************/
                    /* store the list file                              */
                    /****************************************************/
                    fOK = EQFCopyStrip(pDDEClient, pStart+1,
                                 pDocImpExp->chListName,
                                 sizeof(pDocImpExp->chListName) );
                  }
                  else
                  {
                    /**************************************************/
                    /* work against list of files                     */
                    /**************************************************/
                    // pBegin and pOriginal are in parallel
                    DeltaBeginStart=pStart-pBegin;
                    pOriginal[strlen(pBegin)]=EOS;
                    // uses lower and upper case characters

                    fOK = UtlValidateList( pOriginal+DeltaBeginStart,
                          &ppListIndex, MAX_DDE_FILES );
                    if ( fOK )
                    {
                      /**************************************************/
                      /* get number of files given                      */
                      /* store ptr to each filename                     */
                      /**************************************************/
                      usI = 0;
                      while ( ppListIndex[usI] )
                      {
                        usI++;
                      } /* endwhile */
                      pDocImpExp->ppFileArray = ppListIndex;
                      pDocImpExp->usFileNums = usI;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                  } /* endif */
                  break;
                case BATCH_OPTIONS:
                  fOK = EQFCopyStrip(pDDEClient, pStart,
                               chBuffer,MAX_STRINGLEN);
                  if (fOK )
                  {
                    if ( !(stricmp(chBuffer, SOURCE_STRING)) )
                    {
                      pDocImpExp->fSource = TRUE;
                    }
                    else if ( !(stricmp(chBuffer, SNOMATCH_STRING)) )
                    {
                      pDocImpExp->fSNOMATCH = TRUE;
                    }
                    else if ((stricmp(chBuffer, TARGET_STRING)) != 0)
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                  } /* endif */
                  break;
                case  BATCH_OVERWRITE:                              /* KIT1288c */
                  /******************************************************/
                  /* set NoConfirm to yes or no, set error if other     */
                  /* string is specified for Confirm                    */
                  /******************************************************/
                  fOK = EQFCopyStrip(pDDEClient, pStart,
                               chBuffer,MAX_STRINGLEN);
                  if (fOK )
                  {
                    if ( !(stricmp(chBuffer, YES_STRING)) )
                    {
                      pDocImpExp->fOverwrite = TRUE;
                    }
                    else if ( (stricmp(chBuffer, NO_STRING)) != 0 )
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                  } /* endif */
                  break;

                case  BATCH_STARTPATH:
                  if ( strlen( pStart ) < sizeof(pDocImpExp->szStartPath) )
                  {
                    strcpy( pDocImpExp->szStartPath, pStart);
                  }
                  else
                  {
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                    pData = EQFCmdList[BatchCmd].szDesc;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                  } /* endif */
                  break;

                case  BATCH_TASK:
                  /******************************************************/
                  /* skip it, already stored in pDDEClient structure    */
                  /******************************************************/
                  break;
                case  BATCH_CURDIR:
                  /******************************************************/
                  /* current directory passed ...                       */
                  /******************************************************/
                  fOK = UtlValidateList( pStart,
                                     &ppCurDirListIndex,MAX_DRIVELIST );
                  if (fOK )
                  {
                    pDocImpExp->ppCurDirArray = ppCurDirListIndex;
                  }
                  else
                  {
                    pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                    pData = EQFCmdList[BatchCmd].szDesc;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                  } /* endif */
                  break;
                default :
                  /******************************************************/
                  /* batch found is not allowed for current task        */
                  /******************************************************/
                  fOK = FALSE;
                  pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                  pData = pStart;
                  UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                  break;
              } /* endswitch */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;
        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether the passed list is found and valid               */
    /******************************************************************/
    if ( fOK )
    {
      /****************************************************************/
      /* if list name specified, load the list                        */
      /****************************************************************/
      if ( pDocImpExp->chListName[0] )
      {
        fOK = UtlListOfFiles( &ppCurDirListIndex, pDocImpExp->chListName, &ppListIndex );
        if ( fOK )
        {
          /**************************************************/
          /* get number of files given                      */
          /* store ptr to each filename                     */
          /**************************************************/
          usI = 0;
          while ( ppListIndex[usI] )
          {
            usI++;
          } /* endwhile */
          pDocImpExp->ppFileArray = ppListIndex;
          pDocImpExp->usFileNums = usI;
        }
        else
        {
          pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
          pData = EQFCmdList[BATCH_FILES].szDesc;
          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
        } /* endif */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pDocImpExp->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pDocImpExp->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pDocImpExp->ppFileArray, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *)&pDocImpExp, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function EQFDocExp */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CheckBatchOccurrence                                     |
//+----------------------------------------------------------------------------+
//|Function call:     CheckBatchOccurrence(pDDEClient,PUSHORT)                 |
//+----------------------------------------------------------------------------+
//|Description:       check whether all mandatory parameters are available     |
//|                   check whether no parameters are specified twice          |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT pDDEClient                                    |
//|                   PUSHORT   pusBatchOccurrence []                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE         ok                                          |
//|                   FALSE        error in parameters                         |
//+----------------------------------------------------------------------------+
//|Function flow:     set ptr to batchlist of current task                     |
//|                   while ok and not end of batchlist                        |
//|                     if this batch occurred more than once                  |
//|                       ok = false , error code wrong cmdline                |
//|                     else                                                   |
//|                       point to next entry in batchlist                     |
//|                     endif                                                  |
//|                   endwhile                                                 |
//|                   set ptr to list of mandatory parameters                  |
//|                   while ok and not end of list                             |
//|                     if parameter occurred once                             |
//|                       ok, point to next entry in list                      |
//|                     else                                                   |
//|                       ok = FALSE, error code: mandatory parameter missing  |
//|                     endif                                                  |
//|                   endwhile                                                 |
//|                   return (ok)                                              |
//+----------------------------------------------------------------------------+

BOOL
 CheckBatchOccurrence
 (
  PDDECLIENT pDDEClient,                         //ptr to client structure
  PUSHORT pusBatchOccurrence                     //array with occurencies
 )
{
  PBATCHLIST pTempList;                          //ptr to allowed/mandatory
  PSZ pData;
                                                 // batchlist
  BOOL  fOK = TRUE;

  /********************************************************************/
  /* check whether no parameter is specified more than once           */
  /********************************************************************/
  pTempList = pDDEClient->pBatchList;
  while ( fOK && pTempList->BatchCmd != BATCH_END )
  {
    if ( (pusBatchOccurrence[pTempList->BatchCmd]) > 1 )
    {
      fOK = FALSE;
      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
      pData = EQFCmdList[pTempList->BatchCmd].szDesc;
      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
    }
    else
    {
      pTempList++;
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* check whether all mandatory parameters are specified             */
  /********************************************************************/
  pTempList = pDDEClient->pMandList;
  while ( fOK && pTempList->BatchCmd != BATCH_END )
  {
    if ( (pusBatchOccurrence[pTempList->BatchCmd]) == 1 )
    {
      pTempList++;
    }
    else
    {
      fOK = FALSE;
      pData = EQFCmdList[pTempList->BatchCmd].szDesc;
      pDDEClient->DDEReturn.usRc = DDE_MANDPARAMISSING;
      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
    } /* endif */
  } /* endwhile */

  return( fOK );
} /* end of function CheckBatchOccurrence */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFDicImp                                                |
//+----------------------------------------------------------------------------+
//|Function call:     EQFDicImp(PDDECLIENT);                                   |
//+----------------------------------------------------------------------------+
//|Description:       do handling if task is Dictionary import                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for DicImport structure                  |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FILES:                                 |
//|                           prepare list of files                            |
//|                           get number of files given                        |
//|                           store ptr to each filename                       |
//|                          case BATCH_OPTIONS:                               |
//|                           if YES specified, set flag = TRUE                |
//|                           if NO specified, set flag= FALSE                 |
//|                           else set return WRONGCMDLINE                     |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+

BOOL
EQFDicImp
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                       // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDICIMPEXP pDicImpExp;              //document import /export struct
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;
  PSZ   * ppCurDirListIndex;
  CHAR    chBuffer[MAX_STRINGLEN];


  /********************************************************************/
  /* allocate memory for DicImport structure                          */
  /********************************************************************/

  fOK = UtlAlloc( (PVOID *)&pDicImpExp, 0L, (LONG) sizeof(DICIMPEXP), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in Dict import instance                 */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pDicImpExp;
    pDicImpExp->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    pDicImpExp->usOption = DDE_COMBINE;           // default value
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pDicImpExp->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case BATCH_DICT:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy (pDicImpExp->chDictName, pStart);
                      UtlStripBlanks(pDicImpExp->chDictName);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL,1,
                                   &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;
                  case  BATCH_FILES:
                    if ( *pStart == LISTINDICATOR )
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL,1,
                                   &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    }
                    else
                    {
                       pData = strtok(pStart," \n\r");
                      /****************************************************/
                      /* store the dictionary name to be imported         */
                      /****************************************************/
                      fOK = EQFCopyStrip(pDDEClient, pData,
                                   pDicImpExp->chListName,
                                   MAX_PATH144+MAX_FILESPEC);
                    } /* endif */

                    break;
                  case  BATCH_OPTIONS:
                    /******************************************************/
                    /* set OPTIONS to ignore, combine or replace           */
                    /******************************************************/
                    fOK = EQFCopyStrip(pDDEClient, pStart,
                                 chBuffer,MAX_STRINGLEN);
                    if (fOK )
                    {
                      if ( !(stricmp(chBuffer, IGNORE_STRING)) )
                      {
                        pDicImpExp->usOption = DDE_IGNORE;
                      }
                      else if ( !stricmp(chBuffer, REPLACE_STRING) )
                      {
                        pDicImpExp->usOption = DDE_REPLACE;
                      }
                      else if ( !stricmp(chBuffer, COMBINE_STRING) )
                      {
                        pDicImpExp->usOption = DDE_COMBINE;
                      }
                      else
                      {
                         fOK = FALSE;
                         pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                         pData = EQFCmdList[BatchCmd].szDesc;
                         UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    }
                    break;
                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;
                  case  BATCH_PASSWORD:
                    fOK = EQFCopyStrip(pDDEClient, pStart,
                                pDicImpExp->chDicPassWord,MAX_FNAME);
                    break;
                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    fOK = UtlValidateList( pStart,
                                       &ppCurDirListIndex,MAX_DRIVELIST );
                    if (fOK )
                    {
                      pDicImpExp->ppCurDirArray = ppCurDirListIndex;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;
                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;
        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */


    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pDicImpExp->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pDicImpExp->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pDicImpExp, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function EQFDicImp */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFCopyStrip                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFCopyStrip(PSZ, PSZ, USHORT)                           |
//+----------------------------------------------------------------------------+
//|Description:       copy from src to dest, check for length and strip blanks |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ          ptr to source string                        |
//|                   PSZ          ptr to destination string                   |
//|                   USHORT       length allowed of string                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for DicImport structure                  |
//+----------------------------------------------------------------------------+

BOOL
EQFCopyStrip
(
  PDDECLIENT pDDEClient,
  PSZ        pSource,
  PSZ        pDest,
  USHORT     usMaxLen
)
{
  BOOL       fOK = TRUE;

   if ( strlen( pSource ) < usMaxLen )
   {
     strcpy( pDest, pSource );
     UtlStripBlanks(pDest);
   }
   else
   {
     fOK = FALSE;
     pDDEClient->DDEReturn.usRc = UTL_PARAM_TOO_LONG;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                   &pSource, EQF_ERROR, pDDEClient->hwndErrMsg );
   } /* endif */
   return (fOK);
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEFolderCreateParms                                     |
//+----------------------------------------------------------------------------+
//|Function call:     DDEFolderCreateParms(PDDECLIENT);                        |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch folder create     |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for FolderCreate structure               |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_NAME                                   |
//|                           copy folder name to DDE data area                |
//|                          case BATCH_DESC:                                  |
//|                           copy folder description to DDE data area         |
//|                          case BATCH_TODRIVE:                               |
//|                           copy target drive to DDE data area               |
//|                          case BATCH_MEM:                                   |
//|                           copy memory to DDE data area                     |
//|                          case BATCH_MARKUP:                                |
//|                           copy markup to DDE data area                     |
//|                          case BATCH_EDIT:                                  |
//|                           copy editor name to DDE data area                |
//|                          case BATCH_DICTS:                                 |
//|                           preprocess list of dictionaries                  |
//|                           copy dictionary names to DDE data area           |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEFolderCreateParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                        // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDDEFOLCRT pFolCrt;                  // folder create struct
  USHORT usI;
  PSZ   * ppListIndex;
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;

  /********************************************************************/
  /* allocate memory for FolCrt structure                             */
  /********************************************************************/

  fOK = UtlAlloc( (PVOID *)&pFolCrt, 0L, (LONG) sizeof(DDEFOLCRT), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in folder create data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pFolCrt;
    pFolCrt->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;                       //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( (pStart != pActive) || (BatchCmd == BATCH_DESC) )
              {
                EQFBothStripDuplSlash(pDDEClient, pStart, pActive);
                pFolCrt->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_NAME:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pFolCrt->szName, pStart);
                      strupr( pFolCrt->szName );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_DESC:
                    if ( strlen( pStart ) < MAX_DESCRIPTION )
                    {
                      int  iLen = strlen(pStart);
                      PSZ     pszOrgStart = pDDEClient->szOrgCmdLine +
                                            (pStart - pDDEClient->szCmdLine);
                      strncpy( pFolCrt->szDescr, pszOrgStart, iLen );
                      pFolCrt->szDescr[iLen] = EOS;
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_MEM:
                    /**************************************************/
                    /* work against list of files                     */
                    /**************************************************/
                    fOK = UtlValidateList( pStart, &ppListIndex ,MAX_DDE_FILES  );
                    if ( fOK )
                    {
                      /**************************************************/
                      /* copy TM names to TM area                       */
                      /**************************************************/
                      usI = 0;
                      memset( pFolCrt->MemTbl, 0, sizeof(pFolCrt->MemTbl) );
                      while ( fOK && ppListIndex[usI] )
                      {
                        if ( usI > MAX_NUM_OF_READONLY_MDB )
                        {
                          /**********************************************/
                          /* Too many TMs specified                     */
                          /**********************************************/
                          fOK = FALSE;
                          pDDEClient->DDEReturn.usRc = ERROR_MAX_READONLY_TM;
                          pData = EQFCmdList[BatchCmd].szDesc;
                          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                        }
                        else if ( strlen(ppListIndex[usI]) >= MAX_LONGFILESPEC )
                        {
                          /**********************************************/
                          /* TM name is too long                        */
                          /**********************************************/
                          fOK = FALSE;
                          pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                          pData = EQFCmdList[BatchCmd].szDesc;
                          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                        }
                        else
                        {
                          if ( usI == 0 )
                          {
                            // first TM in list, write to main TM field
                            strcpy( pFolCrt->szMem, ppListIndex[usI] );
                            strupr( pFolCrt->szMem );
                          }
                          else
                          {
                            // second or more TM in list, write to R/O TM table
                            strcpy( pFolCrt->MemTbl[usI-1], ppListIndex[usI] );
                            strupr( pFolCrt->MemTbl[usI-1] );
                          } /* endif */
                        } /* endif */
                        usI++;
                      } /* endwhile */

                      /**************************************************/
                      /* Free pointer array memory                      */
                      /**************************************************/
                      UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TODRIVE:
                    if ( strlen( pStart ) == 1 )
                    {
                      pFolCrt->chToDrive = (CHAR)toupper(*pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_MARKUP:
                    if ( strlen( pStart ) < MAX_FNAME )
                    {
                      strcpy( pFolCrt->szFormat, pStart);
                      strupr( pFolCrt->szFormat );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_EDIT:
                    if ( strlen( pStart ) < MAX_FNAME )
                    {
                      strcpy( pFolCrt->szEdit, pStart);
                      strupr( pFolCrt->szEdit );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_DICT:
                    /**************************************************/
                    /* work against list of files                     */
                    /**************************************************/
                    fOK = UtlValidateList( pStart, &ppListIndex ,MAX_DDE_FILES  );
                    if ( fOK )
                    {
                      /**************************************************/
                      /* copy dictionary names to dictionary area       */
                      /**************************************************/
                      usI = 0;
                      memset( pFolCrt->szDicts, 0, sizeof(pFolCrt->szDicts) );
                      while ( ppListIndex[usI] )
                      {
                        if ( usI >= NUM_OF_FOLDER_DICS )
                        {
                          /**********************************************/
                          /* Too many dictionaries specified            */
                          /**********************************************/
                          fOK = FALSE;
                          pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                          pData = EQFCmdList[BatchCmd].szDesc;
                          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                        }
                        else if ( strlen(ppListIndex[usI]) >= MAX_LONGFILESPEC )
                        {
                          /**********************************************/
                          /* Dictionary name is too long                */
                          /**********************************************/
                          fOK = FALSE;
                          pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                          pData = EQFCmdList[BatchCmd].szDesc;
                          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                        }
                        else
                        {
                          strcpy( pFolCrt->szDicts[usI], ppListIndex[usI] );
                          strupr( pFolCrt->szDicts[usI] );
                        } /* endif */
                        usI++;
                      } /* endwhile */

                      /**************************************************/
                      /* Free pointer array memory                      */
                      /**************************************************/
                      UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_SRCLNG:
                    if ( strlen( pStart ) <= MAX_LANG_LENGTH )
                    {
                      strcpy( pFolCrt->szSourceLang, pStart);
                      UtlUpper( pFolCrt->szSourceLang );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TGTLNG:
                    if ( strlen( pStart ) <= MAX_LANG_LENGTH )
                    {
                      strcpy( pFolCrt->szTargetLang, pStart);
                      UtlUpper( pFolCrt->szTargetLang );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_CONVERSION:
                    if ( strlen( pStart ) < MAX_DESCRIPTION )
                    {
                      strcpy( pFolCrt->szConversion, pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* We have no need for this info ...                  */
                    /******************************************************/
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        case DOUBLEQUOTE:
          // loop to end of string = up to closing double quote
          pActive++;
          while ( *pActive && (*pActive != DOUBLEQUOTE) )
          {
            pActive++;
          } /* endwhile */
          if ( *pActive == DOUBLEQUOTE )
          {
            pActive++;
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pFolCrt->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pFolCrt->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pFolCrt, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEFolderCreateParms */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEFolderExportParms                                     |
//+----------------------------------------------------------------------------+
//|Function call:     DDEFolderExportParms(PDDECLIENT);                        |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch folder export     |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for FolderExport structure               |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD:                                   |
//|                           copy foldername in DDE data area                 |
//|                          case BATCH_TODRIVE:                               |
//|                           copy target drive to DDE data area               |
//|                          case BATCH_OPTIONS:                               |
//|                           prepare list of options                          |
//|                           loop over list of options                        |
//|                             if option is DICT                              |
//|                                set with-dictionary flag                    |
//|                             elsif option is MEM                            |
//|                                set with-memory flag                        |
//|                             elseif option is DELETE                        |
//|                                set delete-after-export flag                |
//|                             else                                           |
//|                                set return WRONGCMDLINE                     |
//|                           endloop                                          |
//|                          case BATCH_DESC                                   |
//|                           if description starts with an ampersand          |
//|                             load note file and anchor loaded file          |
//|                               in DDE data area                             |
//|                           else                                             |
//|                             anchor note in DDE data area                   |
//|                          case BATCH_FILES:                                 |
//|                           prepare list of files                            |
//|                           get number of files given                        |
//|                           store ptr to each filename                       |
//|                          case BATCH_OVERWRITE:                             |
//|                           if YES specified, set flag = TRUE                |
//|                           if NO specified, set flag= FALSE                 |
//|                           else set return WRONGCMDLINE                     |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEFolderExportParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                        // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDDEFOLEXP pFolExp = NULL;              // folder export struct
  USHORT usI;
  PSZ   * ppListIndex = NULL;
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;
  PSZ   * ppCurDirListIndex = NULL;

  /********************************************************************/
  /* allocate memory for FolExp structure                             */
  /********************************************************************/

  fOK = UtlAlloc( (PVOID *)&pFolExp, 0L, (LONG) sizeof(DDEFOLEXP), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in folder export data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pFolExp;
    pFolExp->hwndErrMsg = pDDEClient->hwndErrMsg;

    pFolExp->fToPath = FALSE; // unless we've got a TOPATH Batch clause bt 050101

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/

              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pFolExp->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pFolExp->szFolder, pStart);
                      strupr( pFolExp->szFolder );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_DESC:
                    if ( *pStart == LISTINDICATOR )
                    {
                      PSZ     pData = NULL;
                      ULONG   ulSize = 0;

                      fOK = UtlLoadFileL( pStart+1, (PVOID *)&pData, &ulSize, FALSE, FALSE );
                      if ( !fOK )
                      {
                        pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      }
                      else
                      {
                        pData[ulSize-1] = EOS;
                        pFolExp->pszNote = pData;
                      } /* endif */
                    }
                    else
                    {
                      int  iLen = strlen(pStart);
                      PSZ     pszOrgStart = pDDEClient->szOrgCmdLine +
                                            (pStart - pDDEClient->szCmdLine);
                      fOK = UtlAlloc( (PVOID *)&pFolExp->pszNote, 0L,
                                      (LONG)max(MIN_ALLOC,strlen(pStart)+1), NOMSG );
                      if ( fOK )
                      {
                        strncpy( pFolExp->pszNote, pszOrgStart, iLen );
                        pFolExp->pszNote[iLen] = EOS;
                      }
                      else
                      {
                        pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                                      NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */
                    break;

                  case  BATCH_TODRIVE:
                    if ( strlen( pStart ) == 1 )
                    {
                      pFolExp->chToDrive = (CHAR)toupper(*pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case BATCH_TOPATH:
                    if ( strlen( pStart ) > 0)
                    {
                      // cut off first backslash

                      if (pStart[0] == '\\')
                        strcpy ( pFolExp->chToPath, pStart + 1 );
                      else
                        strcpy ( pFolExp->chToPath, pStart );

                      pFolExp->fToPath = TRUE;

                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_FILES:
                    if ( *pStart == LISTINDICATOR )
                    {
                      /****************************************************/
                      /* store the list file                              */
                      /****************************************************/
                      fOK = EQFCopyStrip(pDDEClient, pStart + 1,
                                   pFolExp->chListName,
                                   sizeof(pFolExp->chListName) );
                    }
                    else
                    {
                      /**************************************************/
                      /* work against list of files                     */
                      /**************************************************/
                      fOK = UtlValidateList( pStart, &ppListIndex,MAX_DDE_FILES );
                      if ( fOK )
                      {
                        /**************************************************/
                        /* get number of files given                      */
                        /* store ptr array                                */
                        /**************************************************/
                        usI = 0;
                        while ( ppListIndex[usI] )
                        {
                          strupr( ppListIndex[usI] );
                          usI++;
                        } /* endwhile */
                        pFolExp->ppFileArray = ppListIndex;
                        pFolExp->usFileNums = usI;
                      }
                      else
                      {
                        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */
                    break;

                  case  BATCH_OPTIONS:
                    /**************************************************/
                    /* work against list of options                   */
                    /**************************************************/
                    fOK = UtlValidateList( pStart, &ppListIndex ,MAX_DDE_FILES);
                    if ( fOK )
                    {
                      usI = 0;
                      while ( ppListIndex[usI] )
                      {
                        if ( stricmp( ppListIndex[usI], DICT_OPTION ) == 0 )
                        {
                          pFolExp->fWithDict = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], MEM_OPTION ) == 0 )
                        {
                          pFolExp->fWithMem = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], DELETE_OPTION ) == 0 )
                        {
                          pFolExp->fDelete = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], READONLYMEM_OPTION ) == 0 )
                        {
                          pFolExp->fWithROMem = TRUE;
                          pFolExp->fWithMem = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], DOCMEM_OPTION ) == 0 )
                        {
                          pFolExp->fWithDocMem = TRUE;
                          pFolExp->fWithMem = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], ASMASTERFOLDER_OPTION ) == 0 )
                        {
                          pFolExp->fAsMasterFolder = TRUE;
                        }
                        else
                        {
                          fOK = FALSE;
                          pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                          pData = ppListIndex[usI];
                          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                        } /* endif */
                        usI++;
                      } /* endwhile */

                      /**************************************************/
                      /* Free pointer array memory                      */
                      /**************************************************/
                      UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_OVERWRITE:
                    /******************************************************/
                    /* set fOverwrite to yes or no, set error if other     */
                    /* string is specified for Confirm                    */
                    /******************************************************/
                    if ( !(stricmp(pStart, YES_STRING)) )
                    {
                      pFolExp->fOverWrite = TRUE;
                    }
                    else if ( (stricmp(pStart, NO_STRING)) != 0 )
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    fOK = UtlValidateList( pStart,
                                       &ppCurDirListIndex,MAX_DRIVELIST );
                    if (fOK )
                    {
                      pFolExp->ppCurDirArray = ppCurDirListIndex;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */

                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        case DOUBLEQUOTE:
          // loop to end of string = up to closing double quote
          pActive++;
          while ( *pActive && (*pActive != DOUBLEQUOTE) )
          {
            pActive++;
          } /* endwhile */
          if ( *pActive == DOUBLEQUOTE )
          {
            pActive++;
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */
    if ( fOK )
    {
      /****************************************************************/
      /* if list name specified, load the list                        */
      /****************************************************************/
      if ( pFolExp->chListName[0] )
      {
        fOK = UtlListOfFiles( &ppCurDirListIndex, pFolExp->chListName, &ppListIndex );
        if ( fOK )
        {
          /**************************************************/
          /* get number of files given                      */
          /* store ptr to each filename                     */
          /**************************************************/
          usI = 0;
          while ( ppListIndex[usI] )
          {
            usI++;
          } /* endwhile */
          pFolExp->ppFileArray = ppListIndex;
          pFolExp->usFileNums = usI;
        }
        else
        {
          pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
          pData = EQFCmdList[BATCH_FILES].szDesc;
          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
        } /* endif */
      } /* endif */
    } /* endif */
    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pFolExp->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pFolExp->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    if ( pFolExp != NULL )
    {
      if ( pFolExp->pszNote != NULL ) UtlAlloc( (PVOID *)&pFolExp->pszNote, 0L, 0L, NOMSG );
      UtlAlloc( (PVOID *)&pFolExp, 0L, 0L, NOMSG );
    } /* endif */
    if ( ppListIndex != NULL ) UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEFolderExportParms */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEFolderImportParms                                     |
//+----------------------------------------------------------------------------+
//|Function call:     DDEFolderImportParms(PDDECLIENT);                        |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch folder import     |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for FolderImport structure               |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD:                                   |
//|                           copy folder name to DDE data area                |
//|                          case BATCH_FROMDRIVE:                             |
//|                           copy source drive to DDE data area               |
//|                          case BATCH_TODRIVE:                               |
//|                           copy target drive to DDE data area               |
//|                          case BATCH_OPTIONS:                               |
//|                           prepare list of options                          |
//|                           loop over list of options                        |
//|                             if option is DICT                              |
//|                                set with-dictionary flag                    |
//|                             elsif option is MEM                            |
//|                                set with-memory flag                        |
//|                             else                                           |
//|                                set return WRONGCMDLINE                     |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEFolderImportParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                        // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDDEFOLIMP pFolImp;                     // folder import struct
  USHORT usI;
  PSZ   * ppListIndex;
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;

  /********************************************************************/
  /* allocate memory for FolImp structure                             */
  /********************************************************************/

  fOK = UtlAlloc( (PVOID *)&pFolImp, 0L, (LONG) sizeof(DDEFOLIMP), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in folder import data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pFolImp;
    pFolImp->hwndErrMsg = pDDEClient->hwndErrMsg;

    pFolImp->fFromPath = FALSE; // unless we've got a FROMPATH Batch clause bt 050101

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pFolImp->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pFolImp->szFolder, pStart);
                      strupr( pFolImp->szFolder );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TODRIVE:
                    if ( strlen( pStart ) == 1 )
                    {
                      pFolImp->chToDrive = (CHAR)toupper(*pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_FROMDRIVE:
                    if ( strlen( pStart ) == 1 )
                    {
                      pFolImp->chFromDrive = (CHAR)toupper(*pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case BATCH_FROMPATH:
                    if ( strlen( pStart ) > 0)
                    {
                      // cut off first backslash

                      if (pStart[0] == '\\')
                        strcpy ( pFolImp->chFromPath, pStart + 1 );
                      else
                        strcpy ( pFolImp->chFromPath, pStart );

                        pFolImp->fFromPath = TRUE;
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_OPTIONS:
                    /**************************************************/
                    /* work against list of options                   */
                    /**************************************************/
                    fOK = UtlValidateList( pStart, &ppListIndex ,MAX_DDE_FILES);
                    if ( fOK )
                    {
                      usI = 0;
                      while ( ppListIndex[usI] )
                      {
                        if ( stricmp( ppListIndex[usI], DICT_OPTION ) == 0 )
                        {
                          pFolImp->fWithDict = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], MEM_OPTION ) == 0 )
                        {
                          pFolImp->fWithMem = TRUE;
                        }
                        else
                        {
                          fOK = FALSE;
                          pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                          pData = ppListIndex[usI];
                          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                        } /* endif */
                        usI++;
                      } /* endwhile */

                      /**************************************************/
                      /* Free pointer array memory                      */
                      /**************************************************/
                      UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* We have no need for this info ...                  */
                    /******************************************************/
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pFolImp->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pFolImp->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pFolImp, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEFolderImportParms */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEWordCountParms                                        |
//+----------------------------------------------------------------------------+
//|Function call:     DDEWordCountParms(PDDECLIENT);                           |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch wordcount         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for WordCount structure                  |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD:                                   |
//|                           copy folder name to DDE data area                |
//|                          case BATCH_FROMDRIVE:                             |
//|                           copy source drive to DDE data area               |
//|                          case BATCH_TODRIVE:                               |
//|                           copy target drive to DDE data area               |
//|                          case BATCH_OUT:                                   |
//|                           copy output file name to DDE data area           |
//|                          case BATCH_OVERWRITE:                             |
//|                           set overwrite flag depending on value            |
//|                             specified                                      |
//|                          case BATCH_OPTIONS:                               |
//|                           if specified option is SOURCE                    |
//|                              set count-source flag                         |
//|                           elsif option is TARGET                           |
//|                              set count-target flag                         |
//|                           else                                             |
//|                              set return WRONGCMDLINE                       |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEWordCountParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                        // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDDEWRDCNT pWrdCnt;                     // word count struct
  USHORT usI;
  PSZ   * ppListIndex;
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;
  PSZ   * ppCurDirListIndex;
  int   DeltaBeginStart;               // Delta from pBegin to pStart
  PSZ   pBegin;                        // pointer to begin of command line
  PSZ   pOriginal;                     // pointer to original command line
                                       // (up and downcase)


  /********************************************************************/
  /* allocate memory for FolImp structure                             */
  /********************************************************************/

  fOK = UtlAlloc( (PVOID *)&pWrdCnt, 0L, (LONG) sizeof(DDEWRDCNT), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in folder import data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pWrdCnt;
    pWrdCnt->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;

    // pointer to Original Command Line
    pBegin        = pStart;
    pOriginal     = pDDEClient->szOrgCmdLine;

    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pWrdCnt->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pWrdCnt->szFolder, pStart);
                      strupr( pWrdCnt->szFolder );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_FILES:
                    if ( *pStart == LISTINDICATOR )
                    {
                      /****************************************************/
                      /* store the list file                              */
                      /****************************************************/
                      if ( strlen( pStart ) < sizeof(pWrdCnt->chListName) )
                      {
                        strcpy( pWrdCnt->chListName, pStart+1 );
                        strupr( pWrdCnt->chListName );
                      }
                      else
                      {
                        fOK = FALSE;
                        pDDEClient->DDEReturn.usRc = UTL_PARAM_TOO_LONG;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pStart, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    }
                    else
                    {
                      /**************************************************/
                      /* work against list of files                     */
                      /**************************************************/

                       // pBegin and pOriginal are in parallel
                      DeltaBeginStart=pStart-pBegin;
                      pOriginal[strlen(pBegin)]=EOS;
                      // uses lower and upper case characters
                      fOK = UtlValidateList( pOriginal+DeltaBeginStart,
                          &ppListIndex, MAX_DDE_FILES );
                      if ( fOK )
                      {
                        /**************************************************/
                        /* get number of files given                      */
                        /* store ptr array                                */
                        /**************************************************/
                        usI = 0;
                        while ( ppListIndex[usI] )
                        {
                          // taken out to provide mixed case characters
                          //strupr( ppListIndex[usI] );
                          usI++;
                        } /* endwhile */
                        pWrdCnt->ppFileArray = ppListIndex;
                        pWrdCnt->usFileNums = usI;
                      }
                      else
                      {
                        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */
                    break;

                  case  BATCH_OVERWRITE:
                    /******************************************************/
                    /* set fOverwrite to yes or no, set error if other     */
                    /* string is specified for Confirm                    */
                    /******************************************************/
                    if ( !(stricmp(pStart, YES_STRING)) )
                    {
                      pWrdCnt->fOverWrite = TRUE;
                    }
                    else if ( (stricmp(pStart, NO_STRING)) != 0 )
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_OPTIONS:
                    if ( !(stricmp(pStart, TMMATCH_STRING)) )
                    {
                      pWrdCnt->fTMMatch = TRUE;
                    }
                    else
                    if ( !(stricmp(pStart, TARGET_STRING)) )
                    {
                      pWrdCnt->fTarget = TRUE;
                    }
                    else if ((stricmp(pStart, SOURCE_STRING)) != 0)
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_OUT:
                    if ( strlen( pStart ) < CCHMAXPATH )
                    {
                      strcpy( pWrdCnt->szOutFile, pStart);
                      strupr( pWrdCnt->szOutFile );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    fOK = UtlValidateList( pStart,
                                       &ppCurDirListIndex,MAX_DRIVELIST );
                    if (fOK )
                    {
                      pWrdCnt->ppCurDirArray = ppCurDirListIndex;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /****************************************************************/
    /* if list name specified, load the list                        */
    /****************************************************************/
    if ( fOK && pWrdCnt->chListName[0] )
    {
      fOK = UtlListOfFiles( &pWrdCnt->ppCurDirArray, pWrdCnt->chListName, &ppListIndex );
      if ( fOK )
      {
        /**************************************************/
        /* get number of files given                      */
        /* store ptr to each filename                     */
        /**************************************************/
        usI = 0;
        while ( ppListIndex[usI] )
        {
          strupr( ppListIndex[usI] );
          usI++;
        } /* endwhile */
        pWrdCnt->ppFileArray = ppListIndex;
        pWrdCnt->usFileNums = usI;
      }
      else
      {
        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
        pData = EQFCmdList[BatchCmd].szDesc;
        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
      } /* endif */
    } /* endif */
    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pWrdCnt->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pWrdCnt->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pWrdCnt, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEWordCountParms */




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDECountReportParms                                      |
//+----------------------------------------------------------------------------+
//|Function call:     DDECountReportParms(PDDECLIENT);                         |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch wordcount         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for WordCount structure                  |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD:                                   |
//|                           copy folder name to DDE data area                |
//|                          case BATCH_FROMDRIVE:                             |
//|                           copy source drive to DDE data area               |
//|                          case BATCH_TODRIVE:                               |
//|                           copy target drive to DDE data area               |
//|                          case BATCH_OUT:                                   |
//|                           copy output file name to DDE data area           |
//|                          case BATCH_OVERWRITE:                             |
//|                           set overwrite flag depending on value            |
//|                             specified                                      |
//|                          case BATCH_OPTIONS:                               |
//|                           if specified option is SOURCE                    |
//|                              set count-source flag                         |
//|                           elsif option is TARGET                           |
//|                              set count-target flag                         |
//|                           else                                             |
//|                              set return WRONGCMDLINE                       |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDECountReportParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                        // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDDECNTRPT pCntRpt;                  // counting report struct
  USHORT usI;
  PSZ   * ppListIndex;
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;
  PSZ   * ppCurDirListIndex;
  int   DeltaBeginStart;               // Delta from pBegin to pStart
  PSZ   pBegin;                        // pointer to begin of command line
  PSZ   pOriginal;                     // pointer to original command line
                                       // (up and downcase)


  /********************************************************************/
  /* allocate memory for Counting Report structure                    */
  /********************************************************************/

  fOK = UtlAlloc( (PVOID *)&pCntRpt, 0L, (LONG) sizeof(DDECNTRPT), NOMSG );
  pCntRpt->szProfile[0] = EOS;

  /**********************************************************************/
  /* get parameter from Cmdline in Counting Report data area            */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pCntRpt;
    pCntRpt->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;

    // pointer to Original Command Line
    pBegin        = pStart;
    pOriginal     = pDDEClient->szOrgCmdLine;

    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pCntRpt->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
               //--------------------
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pCntRpt->szFolder, pStart);
                      strupr( pCntRpt->szFolder );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_FILES:
               //--------------------
                    if ( *pStart == LISTINDICATOR )
                    {
                      /****************************************************/
                      /* store the list file                              */
                      /****************************************************/
                      if ( strlen( pStart ) < sizeof(pCntRpt->chListName) )
                      {
                        strcpy( pCntRpt->chListName, pStart+1 );
                        strupr( pCntRpt->chListName );
                      }
                      else
                      {
                        fOK = FALSE;
                        pDDEClient->DDEReturn.usRc = UTL_PARAM_TOO_LONG;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pStart, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    }
                    else
                    {
                      /**************************************************/
                      /* work against list of files                     */
                      /**************************************************/

                       // pBegin and pOriginal are in parallel
                      DeltaBeginStart=pStart-pBegin;
                      pOriginal[strlen(pBegin)]=EOS;
                      // uses lower and upper case characters
                      fOK = UtlValidateList( pOriginal+DeltaBeginStart,
                          &ppListIndex, MAX_DDE_FILES );

                      if ( fOK )
                      {
                        /**************************************************/
                        /* get number of files given                      */
                        /* store ptr array                                */
                        /**************************************************/
                        usI = 0;
                        while ( ppListIndex[usI] )
                        {
                          // taken out to provide mixed case characters
                          //strupr( ppListIndex[usI] );
                          usI++;
                        } /* endwhile */
                        pCntRpt->ppFileArray = ppListIndex;
                        pCntRpt->usFileNums = usI;
                      }
                      else
                      {
                        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */
                    break;

                  case  BATCH_OVERWRITE:
               //-----------------------
                    /******************************************************/
                    /* set fOverwrite to yes or no, set error if other     */
                    /* string is specified for Confirm                    */
                    /******************************************************/
                    if ( !(stricmp(pStart, YES_STRING)) )
                    {
                      pCntRpt->fOverWrite = TRUE;
                    }
                    else if ( (stricmp(pStart, NO_STRING)) != 0 )
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_REPORT:
               //-----------------------


                    if ( !(stricmp(pStart, BATCH_RPT_REPORT_1)) )
                    {
                      pCntRpt->usReport = 0;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_REPORT_2)) )
                    {
                      pCntRpt->usReport = 1;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_REPORT_3)) )
                    {
                      pCntRpt->usReport = 2;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_REPORT_4)) )
                    {
                      pCntRpt->usReport = 3;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_REPORT_5)) )
                    {
                      pCntRpt->usReport = 4;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_REPORT_6)) )
                    {
                      pCntRpt->usReport = 5;
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */


                    break;

                  case  BATCH_TYPE:
               //-----------------------
                    if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_1)) )
                    {
                      pCntRpt->usOptions = 0;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_2)) )
                    {
                      pCntRpt->usOptions = 1;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_3)) )
                    {
                      pCntRpt->usOptions = 2;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_4)) )
                    {
                      pCntRpt->usOptions = 0;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_5)) )
                    {
                      pCntRpt->usOptions = 1;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_6)) )
                    {
                      pCntRpt->usOptions = 0;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_7)) )
                    {
                      pCntRpt->usOptions = 1;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_8)) )
                    {
                      pCntRpt->usOptions = 2;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_9)) )
                    {
                      pCntRpt->usOptions = 3;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_10)) )
                    {
                      pCntRpt->usOptions = 4;
                    }
                    else if ( !(stricmp(pStart, BATCH_RPT_OPTIONS_1)) )
                    {
                      pCntRpt->usOptions = 5;
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;


                  case  BATCH_PROFILE:
               //-----------------------

                    if ( strlen( pStart ) < 13 )
                    {
                      PSZ   pszEnd;
                      strcpy( pCntRpt->szProfile, pStart);
                      strupr( pCntRpt->szProfile );

                      pszEnd = strrchr(pCntRpt->szProfile,'.');
                      if (pszEnd )
                      {
                        *pszEnd = EOS;
                      } /* endif */
                      strcat(pCntRpt->szProfile, ".R00");
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */

                    break;

                  case  BATCH_OUT:
               //-----------------------
                    if ( strlen( pStart ) < CCHMAXPATH )
                    {
                      strcpy( pCntRpt->szOutFile, pStart);
                      strupr( pCntRpt->szOutFile );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_FORMAT:
                    if ( strlen( pStart ) < sizeof(pCntRpt->szFormat) )
                    {
                      strcpy( pCntRpt->szFormat, pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
               //-----------------------
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
               //-----------------------
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    fOK = UtlValidateList( pStart,
                                       &ppCurDirListIndex,MAX_DRIVELIST );
                    if (fOK )
                    {
                      pCntRpt->ppCurDirArray = ppCurDirListIndex;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
               //-----------------------
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /****************************************************************/
    /* if list name specified, load the list                        */
    /****************************************************************/
    if ( fOK && pCntRpt->chListName[0] )
    {
      fOK = UtlListOfFiles( &pCntRpt->ppCurDirArray, pCntRpt->chListName, &ppListIndex );
      if ( fOK )
      {
        /**************************************************/
        /* get number of files given                      */
        /* store ptr to each filename                     */
        /**************************************************/
        usI = 0;
        while ( ppListIndex[usI] )
        {
          strupr( ppListIndex[usI] );
          usI++;
        } /* endwhile */
        pCntRpt->ppFileArray = ppListIndex;
        pCntRpt->usFileNums = usI;
      }
      else
      {
        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
        pData = EQFCmdList[BatchCmd].szDesc;
        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
      } /* endif */
    } /* endif */
    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, out ,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pCntRpt->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pCntRpt->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pCntRpt, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDECountReportParms */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEAnalysisParms                                         |
//+----------------------------------------------------------------------------+
//|Function call:     DDEAnalysisParms(PDDECLIENT);                            |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch analysis          |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for Analsysis structure                  |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD:                                   |
//|                           copy folder name to DDE data area                |
//|                          case BATCH_FILES:                                 |
//|                           prepare list of files                            |
//|                           get number of files given                        |
//|                           store ptr to each filename                       |
//|                          case BATCH_OPTIONS:                               |
//|                           prepare list of options                          |
//|                           loop over options specified                      |
//|                             if specified option is ADDTOMEM                |
//|                                set add-to-folder-memory flag               |
//|                             elsif option is AUTO                           |
//|                                set automatic-translation flag              |
//|                             elsif option is AUTOLAST                       |
//|                                set automatic-translation flag              |
//|                                set use-latest-match flag                   |
//|                             elsif option is UNTRANSLATED                   |
//|                                set SNOMATCH flag                           |
//|                             else                                           |
//|                                set return WRONGCMDLINE                     |
//|                           endloop                                          |
//|                          case BATCH_MEM:                                   |
//|                           prepare list of translation memory databases     |
//|                           get number of translation memory databases       |
//|                           store ptr to each translation memory             |
//|                          case BATCH_OVERWRITE:                             |
//|                           set overwrite flag depending on value            |
//|                             specified                                      |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEAnalysisParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                        // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDDEANALYSIS pAnalysis;              // analysis struct
  USHORT usI;
  PSZ   * ppListIndex;
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;
  PSZ   * ppCurDirListIndex;
  int   DeltaBeginStart;               // Delta from pBegin to pStart
  PSZ   pBegin;                        // pointer to begin of command line
  PSZ   pOriginal;                     // pointer to original command line
                                       // (up and downcase)


  /********************************************************************/
  /* allocate memory for Analysis structure                           */
  /********************************************************************/

  fOK = UtlAlloc( (PVOID *)&pAnalysis, 0L, (LONG) sizeof(DDEANALYSIS), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in folder import data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pAnalysis;
    pAnalysis->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;

    // pointer to Original Command Line
    pBegin        = pStart;
    pOriginal     = pDDEClient->szOrgCmdLine;

    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pAnalysis->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pAnalysis->szFolder, pStart);
                      strupr( pAnalysis->szFolder );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_FILES:
                    if ( *pStart == LISTINDICATOR )
                    {
                      /****************************************************/
                      /* store the list file                              */
                      /****************************************************/
                      if ( strlen( pStart ) < sizeof(pAnalysis->chListName) )
                      {
                        strcpy( pAnalysis->chListName, pStart+1 );
                        //strupr( pAnalysis->chListName );
                      }
                      else
                      {
                        fOK = FALSE;
                        pDDEClient->DDEReturn.usRc = UTL_PARAM_TOO_LONG;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pStart, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    }
                    else
                    {
                      /**************************************************/
                      /* work against list of files                     */
                      /**************************************************/

                      // pBegin and pOriginal are in parallel
                      DeltaBeginStart=pStart-pBegin;
                      pOriginal[strlen(pBegin)]=EOS;
                      // uses lower and upper case characters
                      fOK = UtlValidateList( pOriginal+DeltaBeginStart,
                          &ppListIndex, MAX_DDE_FILES );

                      if ( fOK )
                      {
                        /**************************************************/
                        /* get number of files given                      */
                        /* store ptr array                                */
                        /**************************************************/
                        usI = 0;
                        while ( ppListIndex[usI] )
                        {
                          // taken out to provide mixed case characters
                          // strupr( ppListIndex[usI] );
                          usI++;
                        } /* endwhile */
                        pAnalysis->ppFileArray = ppListIndex;
                        pAnalysis->usFileNums = usI;
                      }
                      else
                      {
                        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */
                    break;

                  case  BATCH_OPTIONS:
                    /**************************************************/
                    /* work against list of options                   */
                    /**************************************************/
                    fOK = UtlValidateList( pStart, &ppListIndex ,MAX_DDE_FILES);
                    if ( fOK )
                    {
                      usI = 0;
                      while ( ppListIndex[usI] )
                      {
                        if ( stricmp( ppListIndex[usI], TMMATCH_STRING ) == 0 )
                        {
                          pAnalysis->fTMMatch = TRUE;
                        }
                        else
                        if ( stricmp( ppListIndex[usI], ADDTOMEM_OPTION ) == 0 )
                        {
                          pAnalysis->fAddToMem = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], AUTO_OPTION ) == 0 )
                        {
                          pAnalysis->fReplExactMatchs = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], UNTRANSLATED_OPTION ) == 0 )
                        {
                          pAnalysis->fUNSEG = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], AUTOLAST_OPTION ) == 0 )
                        {
                          pAnalysis->fUseLatestMatch = TRUE;
                          pAnalysis->fReplExactMatchs = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], AUTOJOIN_OPTION ) == 0 )
                        {
                          pAnalysis->fReplExactMatchs = TRUE;
                          pAnalysis->fAutoJoin = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], AUTOCONTEXT_OPTION ) == 0 )
                        {
                          pAnalysis->fReplExactMatchs = TRUE;
                          pAnalysis->fExactContextTMMatch = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], REDUNDCOUNT_OPTION ) == 0 )
                        {
                          pAnalysis->fRedundCount = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], ADJUSTLEADWS_OPTION ) == 0 )
					    {
					       pAnalysis->fLeadingWS = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], ADJUSTTRAILWS_OPTION ) == 0 )
					    {
					       pAnalysis->fTrailingWS = TRUE;
                        }
                        else if ( stricmp( ppListIndex[usI], RESPECTCRLF_OPTION ) == 0 )
					    {
					       pAnalysis->fRespectCRLF = TRUE;
                        }
                        else
                        {
                          fOK = FALSE;
                          pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                          pData = ppListIndex[usI];
                          UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                        &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                        } /* endif */
                        usI++;
                      } /* endwhile */

                      /**************************************************/
                      /* Free pointer array memory                      */
                      /**************************************************/
                      UtlAlloc( (PVOID *)&ppListIndex, 0L, 0L, NOMSG );
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_MEM:
                    /**************************************************/
                    /* work against list of memories                  */
                    /**************************************************/
                    fOK = UtlValidateList( pStart, &ppListIndex ,MAX_DDE_FILES);
                    if ( fOK )
                    {
                      /**************************************************/
                      /* get number of files given                      */
                      /* store ptr array                                */
                      /**************************************************/
                      usI = 0;
                      while ( ppListIndex[usI] )
                      {
                        strupr( ppListIndex[usI] );
                        usI++;
                      } /* endwhile */
                      pAnalysis->ppMemArray = ppListIndex;
                      pAnalysis->usMemNums = usI;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    fOK = UtlValidateList( pStart,
                                       &ppCurDirListIndex,MAX_DRIVELIST );
                    if (fOK )
                    {
                      pAnalysis->ppCurDirArray = ppCurDirListIndex;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_OVERWRITE:
                    /******************************************************/
                    /* set fOverwrite to yes or no, set error if other     */
                    /* string is specified for Confirm                    */
                    /******************************************************/
                    if ( !(stricmp(pStart, YES_STRING)) )
                    {
                      pAnalysis->fOverWrite = TRUE;
                    }
                    else if ( (stricmp(pStart, NO_STRING)) != 0 )
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /****************************************************************/
    /* if list name specified, load the list                        */
    /****************************************************************/
    if ( fOK && pAnalysis->chListName[0] )
    {
      fOK = UtlListOfFiles( &pAnalysis->ppCurDirArray, pAnalysis->chListName, &ppListIndex );
      if ( fOK )
      {
        /**************************************************/
        /* get number of files given                      */
        /* store ptr to each filename                     */
        /**************************************************/
        usI = 0;
        while ( ppListIndex[usI] )
        {
          //strupr( ppListIndex[usI] );
          usI++;
        } /* endwhile */
        pAnalysis->ppFileArray = ppListIndex;
        pAnalysis->usFileNums = usI;
      }
      else
      {
        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
        pData = EQFCmdList[BatchCmd].szDesc;
        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pAnalysis->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pAnalysis->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pAnalysis, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEAnalysisParms */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEMemCrtParms                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DDEMemCrtParms(PDDECLIENT);                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch TM create         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for memory create structure              |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_NAME:                                  |
//|                           copy TM name to DDE data area                    |
//|                          case BATCH_DESC:                                  |
//|                           copy TM description to DDE data area             |
//|                          case BATCH_TYPE:                                  |
//|                           if type is REMOTE                                |
//|                              set TM mode to remote                         |
//|                           elseif type is LOCAL                             |
//|                              set TM mode to local                          |
//|                           else                                             |
//|                              set return WRONGCMDLINE                       |
//|                          case BATCH_TODRIVE:                               |
//|                           copy todrive to DDE data area                    |
//|                          case BATCH_MARKUP:                                |
//|                           copy markup to DDE data area                     |
//|                          case BATCH_SRCLNG:                                |
//|                           copy source language to DDE data area            |
//|                          case BATCH_TGTLNG:                                |
//|                           copy target language to DDE data area            |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEMemCrtParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ        pStart;                   // pointer to start of current token
  PSZ        pActive;                  // pointer to active position
  CHAR       c;
  BOOL       fOK;
  PDDEMEMCRT pMemCrt;              // analysis struct
  BATCHCMD   BatchCmd = (BATCHCMD)0;                 // active batch command
  PSZ        pData;

  /********************************************************************/
  /* allocate memory for memory create structure                      */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pMemCrt, 0L, (LONG) sizeof(DDEMEMCRT), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in memory create data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pMemCrt;
    pMemCrt->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( (pStart != pActive) || (BatchCmd == BATCH_DESC) )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pMemCrt->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_NAME:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pMemCrt->szName, pStart);
                      strupr( pMemCrt->szName );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_DESC:
                    if ( strlen( pStart ) < MAX_MEM_DESCRIPTION )
                    {
                      int  iLen = strlen(pStart);
                      PSZ     pszOrgStart = pDDEClient->szOrgCmdLine +
                                            (pStart - pDDEClient->szCmdLine);
                      strncpy( pMemCrt->szDescr, pszOrgStart, iLen );
                      pMemCrt->szDescr[iLen] = EOS;
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TYPE:
                    if ( stricmp( pStart, SHARED_OPTION ) == 0 )
                    {
                      pMemCrt->fShared = TRUE;
                    }
                    else if ( stricmp( pStart, LOCAL_OPTION ) == 0 )
                    {
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TODRIVE:
                    if ( strlen( pStart ) == 1 )
                    {
                      pMemCrt->chToDrive = (CHAR)toupper(*pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_SRCLNG:
                    if ( strlen( pStart ) <= MAX_LANG_LENGTH )
                    {
                      strcpy( pMemCrt->szSourceLang, pStart);
                      strupr( pMemCrt->szSourceLang );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* We have no need for this info ...                  */
                    /******************************************************/
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        case DOUBLEQUOTE:
          // loop to end of string = up to closing double quote
          pActive++;
          while ( *pActive && (*pActive != DOUBLEQUOTE) )
          {
            pActive++;
          } /* endwhile */
          if ( *pActive == DOUBLEQUOTE )
          {
            pActive++;
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pMemCrt->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pMemCrt->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */

  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pMemCrt, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEMemCrtParms */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEMemExpParms                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DDEMemExpParms(PDDECLIENT);                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch TM export         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for memory export structure              |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_NAME:                                  |
//|                           copy TM name to DDE data area                    |
//|                          case BATCH_DESC:                                  |
//|                           copy TM description to DDE data area             |
//|                          case BATCH_TYPE:                                  |
//|                           if type is REMOTE                                |
//|                              set TM mode to remote                         |
//|                           elseif type is LOCAL                             |
//|                              set TM mode to local                          |
//|                           else                                             |
//|                              set return WRONGCMDLINE                       |
//|                          case BATCH_TODRIVE:                               |
//|                           copy todrive to DDE data area                    |
//|                          case BATCH_MARKUP:                                |
//|                           copy markup to DDE data area                     |
//|                          case BATCH_SRCLNG:                                |
//|                           copy source language to DDE data area            |
//|                          case BATCH_TGTLNG:                                |
//|                           copy target language to DDE data area            |
//|                          case BATCH_SERVER:                                |
//|                           copy server name to DDE data area                |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEMemExpParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ        pStart;                   // pointer to start of current token
  PSZ        pActive;                  // pointer to active position
  CHAR       c;
  BOOL       fOK;
  PDDEMEMEXP pMemExp;                  // TM export struct
  BATCHCMD   BatchCmd = (BATCHCMD)0;                 // active batch command
  PSZ        pData;

  /********************************************************************/
  /* allocate memory for memory export structure                      */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pMemExp, 0L, (LONG) sizeof(DDEMEMEXP), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in memory create data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pMemExp;
    pMemExp->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pMemExp->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_MEM:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pMemExp->szName, pStart);
                      strupr( pMemExp->szName );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TYPE:
                    if ( stricmp( pStart, EXTERNAL_OPTION ) == 0 )
                    {
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_OVERWRITE:
                    /******************************************************/
                    /* set fOverwrite to yes or no, set error if other     */
                    /* string is specified for Confirm                    */
                    /******************************************************/
                    if ( !(stricmp(pStart, YES_STRING)) )
                    {
                      pMemExp->fOverWrite = TRUE;
                    }
                    else if ( (stricmp(pStart, NO_STRING)) != 0 )
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_OUT:
                    if ( strlen( pStart ) < CCHMAXPATH )
                    {
                      strcpy( pMemExp->szOutFile, pStart);
                      strupr( pMemExp->szOutFile );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    if ( strlen( pStart ) < sizeof(pMemExp->szCurDir) )
                    {
                      strcpy( pMemExp->szCurDir, pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pMemExp->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pMemExp->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pMemExp, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEMemExpParms */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEMemImpParms                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DDEMemImpParms(PDDECLIENT);                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch TM import         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for memory import structure              |
//|                   TBD                                                      |
//+----------------------------------------------------------------------------+
BOOL DDEMemImpParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ        pStart;                   // pointer to start of current token
  PSZ        pActive;                  // pointer to active position
  CHAR       c;
  BOOL       fOK;
  PDDEMEMIMP pMemImp;                  // TM export struct
  BATCHCMD   BatchCmd = (BATCHCMD)0;                 // active batch command
  PSZ        pData;

  /********************************************************************/
  /* allocate memory for memory export structure                      */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pMemImp, 0L, (LONG) sizeof(DDEMEMEXP), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in memory create data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pMemImp;
    pMemImp->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pMemImp->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_MEM:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pMemImp->szName, pStart);
                      strupr( pMemImp->szName );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TYPE:
                    if ( stricmp( pStart, EXTERNAL_OPTION ) == 0 )
                    {
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_FILES:
                    if ( strlen( pStart ) < CCHMAXPATH )
                    {
                      strcpy( pMemImp->szInFile, pStart);
                      strupr( pMemImp->szInFile );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    if ( strlen( pStart ) < sizeof(pMemImp->szCurDir) )
                    {
                      strcpy( pMemImp->szCurDir, pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pMemImp->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pMemImp->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pMemImp, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEMemExpParms */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEDocOpenParms                                          |
//+----------------------------------------------------------------------------+
//|Function call:     DDEDocOpenParms(PDDECLIENT);                             |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch document open     |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for document open structure              |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD:                                   |
//|                           copy folder name to DDE data area                |
//|                          case BATCH_NAME:                                  |
//|                           copy document name to DDE data area              |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEDocOpenParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                        // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDDEDOCOPEN  pDocOpen;               // DDE structure for document open
  BATCHCMD  BatchCmd = (BATCHCMD)0;                  // active batch command
  PSZ pData;

  /********************************************************************/
  /* allocate memory for document open structure                      */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pDocOpen, 0L, (LONG) sizeof(DDEDOCOPEN), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in data area                            */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pDocOpen;
    pDocOpen->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pDocOpen->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pDocOpen->szFolder, pStart);
                      strupr( pDocOpen->szFolder );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_NAME:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pDocOpen->szName, pStart);
                      strupr( pDocOpen->szName );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_SEGMENT:
                    {
                      LONG lSegNum = atol( pStart );
                      if ( lSegNum != 0 )
                      {
                        pDocOpen->lSegNum = lSegNum;
                      }
                      else
                      {
                        fOK = FALSE;
                        pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    }
                    break;


                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, Name                                    */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pDocOpen->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pDocOpen->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pDocOpen, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEDocOpenParms */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEMemDelParms                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DDEMemDelParms(PDDECLIENT);                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch TM delete         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for memory delete structure              |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_MEM :                                  |
//|                           copy TM name to DDE data area                    |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEMemDelParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ        pStart;                   // pointer to start of current token
  PSZ        pActive;                  // pointer to active position
  CHAR       c;
  BOOL       fOK;
  PDDEMEMDEL pMemDel;                  // TM export struct
  BATCHCMD   BatchCmd = (BATCHCMD)0;   // active batch command
  PSZ        pData;

  /********************************************************************/
  /* allocate memory for memory delete structure                      */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pMemDel, 0L, (LONG) sizeof(DDEMEMDEL), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in memory delete data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pMemDel;
    pMemDel->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pMemDel->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_MEM:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pMemDel->szName, pStart);
                      strupr( pMemDel->szName );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    if ( strlen( pStart ) < sizeof(pMemDel->szCurDir) )
                    {
                      strcpy( pMemDel->szCurDir, pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pMemDel->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pMemDel->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pMemDel, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEMemDelParms */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEMemOrgParms                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DDEMemOrgParms(PDDECLIENT);                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch TM delete         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for memory delete structure              |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_MEM :                                  |
//|                           copy TM name to DDE data area                    |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEMemOrgParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ        pStart;                   // pointer to start of current token
  PSZ        pActive;                  // pointer to active position
  CHAR       c;
  BOOL       fOK;
  PDDEMEMORG pMemOrg;                  // TM organize struct
  BATCHCMD   BatchCmd = (BATCHCMD)0;                 // active batch command
  PSZ        pData;

  /********************************************************************/
  /* allocate memory for memory delete structure                      */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pMemOrg, 0L, (LONG) sizeof(DDEMEMORG), NOMSG );

  /**********************************************************************/
  /* get parameter from Cmdline in memory delete data area              */
  /**********************************************************************/
  if ( fOK )
  {
    /******************************************************************/
    /* anchor ida ....                                                */
    /******************************************************************/
    pDDEClient->pTaskIda = pMemOrg;
    pMemOrg->hwndErrMsg = pDDEClient->hwndErrMsg;

    /******************************************************************/
    /* while no error and not end of cmdline                          */
    /******************************************************************/
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            /************************************************************/
            /* if slash found, pActive points to end of cur token       */
            /************************************************************/
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              /**********************************************************/
              /* get batch specified after slash                        */
              /**********************************************************/
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pMemOrg->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_MEM:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pMemOrg->szName, pStart);
                      strupr( pMemOrg->szName );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    /******************************************************/
                    /* skip it, already stored in pDDEClient structure    */
                    /******************************************************/
                    break;

                  case  BATCH_CURDIR:
                    /******************************************************/
                    /* current directory passed ...                       */
                    /******************************************************/
                    if ( strlen( pStart ) < sizeof(pMemOrg->szCurDir) )
                    {
                      strcpy( pMemOrg->szCurDir, pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
                    /******************************************************/
                    /* batch found is not allowed for current task        */
                    /******************************************************/
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                /********************************************************/
                /* no specification follows after =                     */
                /********************************************************/
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              /**********************************************************/
              /* if slash found and start = pactive, go on              */
              /**********************************************************/
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          /************************************************************/
          /* point to next character until slash found                */
          /************************************************************/
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    /******************************************************************/
    /* check whether all mandatory parameters are provided            */
    /* mandatory are: Folder, file,                                   */
    /******************************************************************/
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pMemOrg->usBatchOccurrence[0]));
      if ( fOK )
      {
        /**************************************************************/
        /* set the TM/2 handler handle where to post our message to...*/
        /**************************************************************/
        pMemOrg->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  /******************************************************************/
  /* free allocated resource                                        */
  /******************************************************************/
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pMemOrg, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEMemOrgParms */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEFolDelParms                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DDEFolDelParms(PDDECLIENT);                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch folder delete     |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for folder delete structure              |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD :                                  |
//|                           copy folder name to DDE data area                |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEFolDelParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ        pStart;                   // pointer to start of current token
  PSZ        pActive;                  // pointer to active position
  CHAR       c;
  BOOL       fOK;
  PDDEFOLDEL pFolDel;                  // folder delete structure
  BATCHCMD   BatchCmd = (BATCHCMD)0;   // active batch command
  PSZ        pData;

  // allocate memory for folder delete structure
  fOK = UtlAlloc( (PVOID *)&pFolDel, 0L, (LONG) sizeof(DDEFOLDEL), NOMSG );

  // get parameter from Cmdline into folder delete data area
  if ( fOK )
  {
    // anchor ida ....
    pDDEClient->pTaskIda = pFolDel;
    pFolDel->hwndErrMsg = pDDEClient->hwndErrMsg;

    // while no error and not end of cmdline
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            // if slash found, pActive points to end of cur token
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...

              // get batch specified after slash
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pFolDel->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
                    fOK = EQFCopyStrip(pDDEClient, pStart,
                                 pFolDel->szFolder, MAX_LONGFILESPEC );
                    break;

                  case  BATCH_TASK:
                    // skip it, already stored in pDDEClient structure
                    break;

                  case  BATCH_CURDIR:
                    // current directory passed, but not used ...
                    break;

                  default :
                    // batch found is not allowed for current task
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                // no specification follows after =
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              // if slash found and start = pactive, go on
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          // point to next character until slash found
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    // check whether all mandatory parameters are provided
    // mandatory are: Folder
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pFolDel->usBatchOccurrence[0]));
      if ( fOK )
      {
        // set the TM/2 handler handle where to post our message to...
        pFolDel->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  // cleanup
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pFolDel, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEFolDelParms */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEDocDelParms                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DDEDocDelParms(PDDECLIENT);                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch document delete   |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for document delete structure            |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_FLD :                                  |
//|                           copy folder name to DDE data area                |
//|                          case BATCH_FILES :                                |
//|                           copy document names to DDE data area             |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEDocDelParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ        pStart;                   // pointer to start of current token
  PSZ        pActive;                  // pointer to active position
  CHAR       c;
  BOOL       fOK;
  PDDEDOCDEL pDocDel;                  // document delete structure
  BATCHCMD   BatchCmd = (BATCHCMD)0;   // active batch command
  PSZ        pData;
  PSZ        *ppListIndex;

  // allocate memory for folder delete structure
  fOK = UtlAlloc( (PVOID *)&pDocDel, 0L, (LONG) sizeof(DDEDOCDEL), NOMSG );

  // get parameter from Cmdline into document delete data area
  if ( fOK )
  {
    // anchor ida ....
    pDDEClient->pTaskIda = pDocDel;
    pDocDel->hwndErrMsg = pDDEClient->hwndErrMsg;

    // while no error and not end of cmdline
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            // if slash found, pActive points to end of cur token
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...

              // get batch specified after slash
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pDocDel->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
                    fOK = EQFCopyStrip(pDDEClient, pStart,
                                 pDocDel->szFldName, MAX_LONGFILESPEC );
                    break;
                  case  BATCH_FILES:
                    if ( *pStart == LISTINDICATOR )
                    {
                      /****************************************************/
                      /* store the list file                              */
                      /****************************************************/
                      fOK = EQFCopyStrip(pDDEClient, pStart+1,
                                   pDocDel->szListName,
                                   sizeof(pDocDel->szListName) );
                    }
                    else
                    {
                      // work against list of files
                      fOK = UtlValidateList( pStart, &ppListIndex,MAX_DDE_FILES );
                      if ( fOK )
                      {
                        USHORT usI;

                        // get number of files given, store ptr to each filename
                        usI = 0;
                        while ( ppListIndex[usI] )
                        {
                          usI++;
                        } /* endwhile */
                        pDocDel->ppFileArray = ppListIndex;
                        pDocDel->usFileNums = usI;
                      }
                      else
                      {
                        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */
                    break;
                  case  BATCH_TASK:
                    // skip it, already stored in pDDEClient structure
                    break;

                  case  BATCH_CURDIR:
                    // current directory passed ...
                    if ( strlen( pStart ) < sizeof(pDocDel->szCurDir) )
                    {
                      strcpy( pDocDel->szCurDir, pStart);
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
                    // batch found is not allowed for current task
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                // no specification follows after =
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              // if slash found and start = pactive, go on
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          // point to next character until slash found
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    // check whether all mandatory parameters are provided
    // mandatory are: Folder
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pDocDel->usBatchOccurrence[0]));
      if ( fOK )
      {
        // set the TM/2 handler handle where to post our message to...
        pDocDel->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  // cleanup
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pDocDel, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEDocDelParms */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEDicExpParms                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DDEDicExpParms(PDDECLIENT);                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch dictionary export |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate memory for dictionary export structure          |
//|                   if ok                                                    |
//|                     set ptr to start                                       |
//|                     while ok and not end of string                         |
//|                       switch (current character)                           |
//|                        case character is Slash:                            |
//|                         validate token                                     |
//|                         switch (token)                                     |
//|                          case BATCH_DICT:                                  |
//|                           copy dict name to DDE data area                  |
//|                          case BATCH_OUT:                                   |
//|                           copy output file name to DDE data area           |
//|                          case BATCH_OVERWRITE:                             |
//|                           if YES specified, set flag = TRUE                |
//|                           if NO specified, set flag= FALSE                 |
//|                           else set return WRONGCMDLINE                     |
//|                          default:                                          |
//|                           set return WRONGCMDLINE                          |
//|                         endswitch                                          |
//|                         increase ptr                                       |
//|                        default: set return WRONGCMDLINE                    |
//|                       endswitch                                            |
//|                     endwhile                                               |
//|                     if ok                                                  |
//|                       if all mandatory parameters available                |
//|                         post a message to import files                     |
//|                       else set return MANDPARAMISSING                      |
//|                       endif                                                |
//|                     endif                                                  |
//|                     if not ok                                              |
//|                      free allocated resource                               |
//+----------------------------------------------------------------------------+
BOOL DDEDicExpParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ        pStart;                   // pointer to start of current token
  PSZ        pActive;                  // pointer to active position
  CHAR       c;
  BOOL       fOK;
  PDDEDICEXP pDicExp;                  // dictionary import export structure
  BATCHCMD   BatchCmd = (BATCHCMD)0;   // active batch command
  PSZ        pData;
  CHAR    chBuffer[MAX_STRINGLEN];
  PSZ   * ppCurDirListIndex;

  // allocate memory for folder delete structure
  fOK = UtlAlloc( (PVOID *)&pDicExp, 0L, (LONG) sizeof(DDEDICEXP), NOMSG );

  // get parameter from Cmdline into document delete data area
  if ( fOK )
  {
    // anchor ida ....
    pDDEClient->pTaskIda = pDicExp;
    pDicExp->hwndErrMsg = pDDEClient->hwndErrMsg;

    // while no error and not end of cmdline
    pActive = pStart = pDDEClient->szCmdLine;
    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            // if slash found, pActive points to end of cur token
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...

              // get batch specified after slash
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pDicExp->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case BATCH_DICT:
                    fOK = EQFCopyStrip(pDDEClient, pStart,
                                 pDicExp->szName, MAX_LONGFILESPEC );
                    break;

                  case  BATCH_OUT:
                    if ( strlen( pStart ) < (sizeof(pDicExp->szOutFile) - 1) )
                    {
                      strcpy( pDicExp->szOutFile, pStart);
                      strupr( pDicExp->szOutFile );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_OVERWRITE:
                    /******************************************************/
                    /* set fOverwrite to yes or no, set error if other     */
                    /* string is specified for Confirm                    */
                    /******************************************************/
                    fOK = EQFCopyStrip(pDDEClient, pStart,
                                 chBuffer, MAX_STRINGLEN );
                    if (fOK )
                    {
                      if ( !(stricmp(chBuffer, YES_STRING)) )
                      {
                        pDicExp->fOverWrite = TRUE;
                      }
                      else if ( (stricmp(chBuffer, NO_STRING)) != 0 )
                      {
                        fOK = FALSE;
                        pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */
                    break;


                  case  BATCH_TASK:
                    // skip it, already stored in pDDEClient structure
                    break;

                  case  BATCH_CURDIR:
                    // current directory passed ...
                    fOK = UtlValidateList( pStart,
                                       &ppCurDirListIndex,MAX_DRIVELIST );
                    if (fOK )
                    {
                      pDicExp->ppCurDirArray = ppCurDirListIndex;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
                    // batch found is not allowed for current task
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                // no specification follows after =
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              // if slash found and start = pactive, go on
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          // point to next character until slash found
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    // check whether all mandatory parameters are provided
    // mandatory are: Folder
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pDicExp->usBatchOccurrence[0]));
      if ( fOK )
      {
        // set the TM/2 handler handle where to post our message to...
        pDicExp->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  // cleanup
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pDicExp, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEDicExpParms */


// function to check if a given handle is the handle of a DDE (=batch process)
// window
BOOL DDEIsBatchHwnd( HWND hwndInQuestion )
{
  CHAR szClass[40];
  BOOL fIsBatchHwnd;

  // get class name of window
  szClass[0] = EOS;
  GetClassName( hwndInQuestion, szClass, sizeof(szClass) );

  // check against our DDE class name
  fIsBatchHwnd = stricmp( szClass, WC_EQF_DDE ) == 0;

  return( fIsBatchHwnd );
} /* end of function DDEIsBatchHwnd */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DDEArchTMParms                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DDEArchTMParms(PDDECLIENT);                              |
//+----------------------------------------------------------------------------+
//|Description:       Prepare/preprocess parameter for batch archive TM        |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT   ptr to client ida                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE          success indicator                          |
//|                   FALSE         error                                      |
//+----------------------------------------------------------------------------+
BOOL DDEArchTMParms
(
  PDDECLIENT pDDEClient
)
{
  PSZ   pStart;                        // pointer to start of current token
  PSZ   pActive;                       // pointer to active position
  CHAR  c;
  BOOL  fOK;
  PDDEARCHTM pArchTM;                  // archive TM structure
  USHORT usI;
  PSZ   * ppListIndex;
  BATCHCMD  BatchCmd = (BATCHCMD)0;    // active batch command
  PSZ pData;
  PSZ   * ppCurDirListIndex;
  int   DeltaBeginStart;               // Delta from pBegin to pStart
  PSZ   pBegin;                        // pointer to begin of command line
  PSZ   pOriginal;                     // pointer to original command line
                                       // (up and downcase)


  // allocate memory for archive TM structure
  fOK = UtlAlloc( (PVOID *)&pArchTM, 0L, (LONG) sizeof(DDEARCHTM), NOMSG );

  // get parameter from Cmdline in folder import data area
  if ( fOK )
  {
    //* anchor IDA ....
    pDDEClient->pTaskIda = pArchTM;
    pArchTM->hwndErrMsg = pDDEClient->hwndErrMsg;

    // while no error and not end of cmdline
    pActive = pStart = pDDEClient->szCmdLine;

    // pointer to Original Command Line
    pBegin        = pStart;
    pOriginal     = pDDEClient->szOrgCmdLine;

    while ( ((c=*pActive)!= NULC) && fOK  )
    {
      switch ( c )
      {
        case DELSLASH:
          if (*(pActive+1)== DELSLASH)
          {
             pActive += 2;              //skip duplicated slash
          }
          else
          {
            // if slash found, pActive points to end of cur token
            if ( pStart != pActive )
            {
              *pActive = EOS;                    //just temporary ...
              // get batch specified after slash
              BatchCmd = ValidateToken( &pStart, pDDEClient->pBatchList );
              if ( pStart != pActive )
              {
                EQFBothStripDuplSlash(pDDEClient,pStart, pActive);
                pArchTM->usBatchOccurrence[ BatchCmd ]++;
                switch ( BatchCmd )
                {
                  case  BATCH_FLD:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pArchTM->szFolder, pStart);
                      strupr( pArchTM->szFolder );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_FILES:
                    if ( *pStart == LISTINDICATOR )
                    {
                      // store the list file
                      if ( strlen( pStart ) < sizeof(pArchTM->chListName) )
                      {
                        strcpy( pArchTM->chListName, pStart+1 );
                      }
                      else
                      {
                        fOK = FALSE;
                        pDDEClient->DDEReturn.usRc = UTL_PARAM_TOO_LONG;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pStart, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    }
                    else
                    {
                      // work against list of files

                      // pBegin and pOriginal are in parallel
                      DeltaBeginStart=pStart-pBegin;
                      pOriginal[strlen(pBegin)]=EOS;
                      // uses lower and upper case characters
                      fOK = UtlValidateList( pOriginal+DeltaBeginStart,
                                          &ppListIndex, MAX_DDE_FILES );

                      if ( fOK )
                      {
                        // get number of files given
                        usI = 0;
                        while ( ppListIndex[usI] )
                        {
                          usI++;
                        } /* endwhile */
                        pArchTM->ppFileArray = ppListIndex;
                        pArchTM->usFileNums = usI;
                      }
                      else
                      {
                        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                        pData = EQFCmdList[BatchCmd].szDesc;
                        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                      } /* endif */
                    } /* endif */
                    break;


                  case  BATCH_MEM:
                    if ( strlen( pStart ) < MAX_LONGFILESPEC )
                    {
                      strcpy( pArchTM->szMemName, pStart);
                      strupr( pArchTM->szMemName );
                    }
                    else
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_TASK:
                    // skip it, already stored in pDDEClient structure
                    break;

                  case  BATCH_CURDIR:
                    // current directory passed ...
                    fOK = UtlValidateList( pStart, &ppCurDirListIndex,MAX_DRIVELIST );
                    if (fOK )
                    {
                      pArchTM->ppCurDirArray = ppCurDirListIndex;
                    }
                    else
                    {
                      pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  case  BATCH_OVERWRITE:
                    // set fOverwrite flag
                    if ( !(stricmp(pStart, YES_STRING)) )
                    {
                      pArchTM->fOverWrite = TRUE;
                    }
                    else if ( (stricmp(pStart, NO_STRING)) != 0 )
                    {
                      fOK = FALSE;
                      pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                      pData = EQFCmdList[BatchCmd].szDesc;
                      UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                    &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    } /* endif */
                    break;

                  default :
                    // batch found is not allowed for current task
                    fOK = FALSE;
                    pDDEClient->DDEReturn.usRc = DDE_WRONGBATCH;
                    pData = pStart;
                    UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                                  &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
                    break;
                } /* endswitch */
              }
              else
              {
                 // no specification follows after =
                 fOK = FALSE;
                 pDDEClient->DDEReturn.usRc = DDE_WRONGCMDLINE;
                 pData = EQFCmdList[BatchCmd].szDesc;
                 UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                               &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
              } /* endif */
              *pActive = DELSLASH;                  //reset EOS to slash
              pStart = pActive;
            }
            else                                    //pStart == pActive
            {
              // if slash found and start = pactive, go on
              pActive++;
            } /* endif */
          } /* endif */
          break;

        default :
          // point to next character until slash found
          pActive++;
          break;
      } /* endswitch */
    } /* endwhile */

    // if list name specified, load the list
    if ( fOK && pArchTM->chListName[0] )
    {
      fOK = UtlListOfFiles( &pArchTM->ppCurDirArray, pArchTM->chListName, &ppListIndex );
      if ( fOK )
      {
        // get number of files given store ptr to each filename
        usI = 0;
        while ( ppListIndex[usI] )
        {
          usI++;
        } /* endwhile */
        pArchTM->ppFileArray = ppListIndex;
        pArchTM->usFileNums = usI;
      }
      else
      {
        pDDEClient->DDEReturn.usRc = DDE_ERRFILELIST;
        pData = EQFCmdList[BatchCmd].szDesc;
        UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 1,
                      &pData, EQF_ERROR, pDDEClient->hwndErrMsg );
      } /* endif */
    } /* endif */

    // check whether all mandatory parameters are provided
    if ( fOK )
    {
      fOK = CheckBatchOccurrence(pDDEClient,
                               &(pArchTM->usBatchOccurrence[0]));
      if ( fOK )
      {
        // set the TM/2 handler handle where to post our message to...
        pArchTM->hwndOwner = pDDEClient->hwndInstance;  //instance handle
      } /* endif */
    } /* endif */
  }
  else
  {
     pDDEClient->DDEReturn.usRc = ERROR_STORAGE;
     UtlErrorHwnd( pDDEClient->DDEReturn.usRc, MB_CANCEL, 0,
                   NULL, EQF_ERROR, pDDEClient->hwndErrMsg );
  } /* endif */

  // free allocated resource
  if ( !fOK )
  {
    UtlAlloc( (PVOID *)&pArchTM, 0L, 0L, NOMSG );
  } /* endif */
  return( fOK );
} /* end of function DDEarchTMParms */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFStripDuplSlash                                        |
//+----------------------------------------------------------------------------+
//|Function call:     EQFStripDuplSlash(PSZ)                                   |
//+----------------------------------------------------------------------------+
//|Description:       strip the 2nd of the duplicated slashes                  |
//|                   Example:parameters should be able to handle /            |
//|                   e.g. "/de=A/B" input here: A//B output: A/B              |
//|                   This is a general function. all duplicated slashes in a  |
//|                   string are stripped ( e.g. A//B////C -> A/B//C )         |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ   ptr to string to be checked                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+

static BOOL
EQFStripDuplSlash
(
   PSZ    pStart
)
{
  PSZ    pIn, pOut;
  CHAR   c;
  BOOL   fStripped = FALSE;

  pIn = pOut = pStart;
  while ( (c=*pOut++ = *pIn++) != NULC )
  {
    if ( (c == DELSLASH) && ((*pIn ) == DELSLASH) )
    {
      pIn++;                           // skip dupl. slash
      fStripped = TRUE;
    } /* endif */
  } /* endwhile */

  return(fStripped);
} /* end of function EQFStripDuplSlash*/


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBothStripDuplSlash                                    |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBothStripDuplSlash(PDDECLIENT, PSZ, PSZ)              |
//+----------------------------------------------------------------------------+
//|Description:       strip the 2nd of the duplicated slashes                  |
//|                   Example:parameters should be able to handle /            |
//|                   e.g. "/de=A/B" input here: A//B output: A/B              |
//|                   This is a general function. all duplicated slashes in a  |
//|                   string are stripped ( e.g. A//B////C -> A/B//C )         |
//+----------------------------------------------------------------------------+
//|Parameters:        PDDECLIENT  ptr to dde struct                            |
//|                   PSZ         ptr to start in szCmdLine                    |
//|                   PSZ         ptr to end of current token in szCmdLine     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+

static VOID
EQFBothStripDuplSlash
(
   PDDECLIENT  pDDEClient,
   PSZ         pStart,
   PSZ         pActive
)
{
   if (EQFStripDuplSlash(pStart))
   {
     PSZ     pszOrgStart = pDDEClient->szOrgCmdLine +
                           (pStart - pDDEClient->szCmdLine);
     PSZ     pszOrgActive = pDDEClient->szOrgCmdLine +
                            (pActive - pDDEClient->szCmdLine);
     CHAR    c = *pszOrgActive;

     *pszOrgActive = EOS;             // temporary
     EQFStripDuplSlash( pszOrgStart);
     *pszOrgActive = c;
   } /* endif */
  return;
} /* end of function EQFBothStripDuplSlash*/
