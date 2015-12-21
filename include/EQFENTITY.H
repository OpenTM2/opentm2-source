// Copyright (c) 2009-2012, International Business Machines
// Corporation and others.  All rights reserved.
//
//
//+-----------------------------------------------------------------------------+
//| EQFENTITY.H       External header file entity functions                     |
//+-----------------------------------------------------------------------------+

__declspec(dllexport)
LONG LoadEntities( PSZ pszFolObject );

__declspec(dllexport)
BOOL ScanForEntities( PTBDOCUMENT pDoc );

__declspec(dllexport)
BOOL isEntityMarkup( PSZ pszMarkup );

BOOL SetEntity( LONG hEntity, PSZ_W pszID, ULONG ulSeg, PSZ pszDocument, PSZ_W pszValue );

BOOL isEntity( PSZ_W pszWord );

BOOL GetEntityValue( LONG hEntity, PSZ_W pszID, ULONG ulSeg, PSZ pszDocument, PSZ_W pszValue );

BOOL SaveEntitites( LONG hEntities, PSZ pszFolObject );

__declspec(dllexport)
BOOL FreeEntitites( LONG hEntity );
