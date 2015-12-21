/*! EQFFLL00.H - Internal header file for the EQF FolderListHandler
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/
#ifndef INCL_EQFFLL00
#define INCL_EQFFLL00
#define  HPS_DRIVLIST_cy   30
#define  HPS_HEADLINE_cy   25

#define  FLL_ITEM_FORMAT  "%s\x15L%s\x15L%s\x15D%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15"
typedef enum          //    ^      ^      ^      ^     ^     ^     ^     ^
{                     //
  FLL_OBJECT_IND,     //----+
  FLL_NAME_IND,       //-----------+
  FLL_DESCR_IND,      //------------------+
  FLL_DATE_IND,       //-------------------------+
  FLL_FORMAT_IND,     //------------------------------+
  FLL_MEMORY_IND,     //-------------------------------------+
  FLL_DRIVE_IND,      //-------------------------------------------+
  FLL_DICT_IND,       //-------------------------------------------------+
  FLL_EDIT_IND,       //-------------------------
  FLL_SOURCE_IND,     //-------------------------
  FLL_TARGET_IND      //-------------------------
} FLL_ITEMINDX;


/* FolderListHandler Instance Data Area */
typedef struct _FOLDERLIST
{
    IDA_HEAD     IdaHead;              // common header of instance area
    HWND         hLBox;                // list box wnd handle
    CHAR         szColumnTitles[256];  // column titles
    SHORT  sLastUsedViewList[MAX_VIEW];// last used view list
    SHORT  sDetailsViewList[MAX_VIEW]; // user's details view list
    SHORT  sSortList[MAX_VIEW];        // user's sort criteria list
    OBJNAME     szObjName;             // general use object name buffer
} FOLDERLIST_IDA, *PFOLDERLIST_IDA;


/* Export window proc */
MRESULT APIENTRY FolderListHandlerWP( HWND, WINMSG, WPARAM, LPARAM);
MRESULT APIENTRY FolderListWP( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport)
VOID    FllLoadFolderNames( HWND hlbox, PSZ pszBuffer );
PSZ     QuerySelFolderName( PLISTCOMMAREA, BOOL );
USHORT  MakeDriveButtons( HWND hwnd, PSZ pCurDrvLst);

#define MAX_DUMP_NOTE 4096             // max size of a note attached to a dump

typedef struct _FLLDUMPIDA
{
   CHAR   szObject[_MAX_PATH];         // name of folder object
   CHAR   szFolder[_MAX_PATH];         // name of folder
   CHAR   szSaveram[_MAX_PATH];        // buffer for program used to save data
   CHAR   szTarget[_MAX_PATH];         // name of target file
   CHAR   szSource[_MAX_PATH];         // buffer for name of source file(s)
   CHAR szNote[MAX_DUMP_NOTE+1];       // text of attached note
} FLLDUMPIDA, *PFLLDUMPIDA;

#endif

//   End of EQFFLL00.H
//ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß
