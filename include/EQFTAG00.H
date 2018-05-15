//+----------------------------------------------------------------------------+
//|  EQFTAG00.H - Private header file for the EQF TagTable Handler             |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description:                                                              |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 31 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
// 
// 
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
// 
// 
// $Revision: 1.2 $ ----------- 3 Apr 2002
// --RJ: P014206: add TAG_DESCR_IND to default view
// 
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFTAG00.HV_   1.2   03 May 1999 09:19:44   BUILD  $
 *
 * $Log:   K:\DATA\EQFTAG00.HV_  $
 *
 *    Rev 1.2   03 May 1999 09:19:44   BUILD
 * - re-activated symbolic markup table names
 *
 *    Rev 1.1   12 Oct 1998 10:27:02   BUILD
 * -- MFC support: add end identification for column data
 *
 *    Rev 1.0   09 Jan 1996 09:47:36   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#ifndef INCL_EQFTAG00
#define INCL_EQFTAG00

/**********************************************************************/
/* Index into listbox item data                                       */
/**********************************************************************/
#define TAG_OBJECT_IND       0
#define TAG_NAME_IND         1
#define TAG_SHORTDESCR_IND   2
#define TAG_LONGDESCR_IND    3
#define TAG_VERSION_IND      4
#define TAG_PLUGIN_IND       5
#define TAG_SUPPLIER_IND     6

/* TagTableHandler Instance Data Area */
typedef struct _TAGTTABLEIDA
{
    IDA_HEAD     IdaHead;              // common header of instance area
    HWND         hLBox;                // list box wnd handle
} TAGTABLEIDA, *PTAGTABLEIDA;



__declspec(dllexport) VOID TagTableExport( HWND, PSZ );
__declspec(dllexport) VOID TagTableImport( HWND, PSZ);

// functions implemented in MarkupPluginMapper

typedef enum { MUFILL_NAMES, MUFILL_CLBITEMS } MUFILLTYPES;

__declspec(dllexport) USHORT MUFillLBWithMarkups( HWND hwndLBox, PSZ pszBuffer, int iBufSize, MUFILLTYPES eFillType );
__declspec(dllexport) BOOL isMarkupDeletable( char *pszMarkup, char *pszPlugin );
__declspec(dllexport) BOOL isMarkupExportable( char *pszMarkup, char *pszPlugin );
__declspec(dllexport) BOOL isMarkupImportable( char *pszMarkup, char *pszPlugin );
__declspec(dllexport) BOOL isMarkupProtected( char *pszMarkup, char *pszPlugin );
__declspec(dllexport) BOOL TagMakeListItem( PSZ pszMarkup, PSZ pszPlugin, PSZ pszBuffer, int iBufSize );
__declspec(dllexport) BOOL MULoadMarkupTableFile( char *pszMarkup, char *pszPlugin, PVOID *ppLoadedTable, BOOL fErrorHandling );
__declspec(dllexport) BOOL MUGetMarkupTableFileName( char *pszMarkup, char *pszPlugin, char *pszBuffer, int iBufSize );
__declspec(dllexport) BOOL MUGetMarkupTableFilePath( char *pszMarkup, char *pszPlugin, char *pszBuffer, int iBufSize );
__declspec(dllexport) BOOL MUGetUserExitFileName( char *pszMarkup, char *pszPlugin, char *pszBuffer, int iBufSize );
__declspec(dllexport) BOOL MUUpdateMarkupTableFiles( char *pszMarkup, char *pszPlugin, char *pszShortDescription, char *pszLongDescription, char *pszVersion, char *pszTableFile, char *pszUserExitFile, char *pszMarkupFiles );
__declspec(dllexport) BOOL MUDeleteMarkupTable( char *pszMarkup, char *pszPlugin );

#endif
