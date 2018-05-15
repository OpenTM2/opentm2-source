//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmDocumentPlugin.h"
#include "core\PluginManager\OtmDocument.h"

static OtmDocumentPlugin* thePlugin = 0;
static OtmDocument* theDocument = 0;


__declspec(dllexport)
void InitDocumentPluginMapper()
{
	PluginManager* thePluginManager = PluginManager::getInstance();
	thePlugin = (OtmDocumentPlugin*) thePluginManager->getPlugin(OtmPlugin::eDocumentType);
  if ( thePlugin != NULL )
  {
 	  theDocument = thePlugin->createDocument();
  }
#ifdef _DEBUG
  else
  {
    MessageBox( NULL, "No document plugin available, could not initialize document plugin mapper","Debug message",  MB_OK );
  }
#endif
}

#ifndef CPPTEST
extern "C" {
#endif

USHORT EQFBCharType(PTBDOCUMENT pDoc, PTBSEGMENT pSeg, USHORT usOffs)
{
	return theDocument->CPP_EQFBCharType(pDoc, pSeg, usOffs);
}
PTBSEGMENT EQFBGetSegW(PTBDOCUMENT pDoc, ULONG ulSeg)
{
	return theDocument->CPP_EQFBGetSegW(pDoc, ulSeg);
}
BOOL EQFBFileExists(PSZ pszFile)
{
	return theDocument->CPP_EQFBFileExists(pszFile);
}
BOOL EQFBDiffTag(PTBDOCUMENT pDoc, ULONG ulSeg, USHORT usOffs)
{
	return theDocument->CPP_EQFBDiffTag(pDoc, ulSeg, usOffs);
}
BOOL EQFBIsLFProtected(PTBSEGMENT pSeg, SHORT sOffs)
{
	return theDocument->CPP_EQFBIsLFProtected(pSeg, sOffs);
}
BOOL EQFBDiffProtectTag(PTBDOCUMENT pDoc, ULONG ulSeg, USHORT usOffs)
{
	return theDocument->CPP_EQFBDiffProtectTag(pDoc, ulSeg, usOffs);
}
VOID EQFBReparse(PTBDOCUMENT pDoc, PTBSEGMENT pSeg, USHORT usOffs, SHORT sDiff)
{
	return theDocument->CPP_EQFBReparse(pDoc, pSeg, usOffs, sDiff);
}
SHORT EQFBLineUp(PTBDOCUMENT pDoc)
{
	return theDocument->CPP_EQFBLineUp(pDoc);
}
SHORT EQFBLineDown(PTBDOCUMENT pDoc)
{
	return theDocument->CPP_EQFBLineDown(pDoc);
}
SHORT EQFBDocLoad(PLOADSTRUCT pLoad)
{
	return theDocument->CPP_EQFBDocLoad(pLoad);
}
PTBDOCUMENT EQFBDocDelete(PTBDOCUMENT pDoc)
{
	return theDocument->CPP_EQFBDocDelete(pDoc);
}
SHORT EQFBDocSave(PTBDOCUMENT pDoc, PSZ pszFileName, BOOL fAskForSave)
{
	return theDocument->CPP_EQFBDocSave(pDoc, pszFileName, fAskForSave);
}
VOID EQFBFuncOpenTRNote(PTBDOCUMENT pDoc)
{
	return theDocument->CPP_EQFBFuncOpenTRNote(pDoc);
}
VOID EQFBDocPrint(PTBDOCUMENT pDoc)
{
	return theDocument->CPP_EQFBDocPrint(pDoc);
}
SHORT EQFBWordCntPerSeg(PVOID pVoidTable, PTOKENENTRY pTokBuf, PSZ_W pData, SHORT sLanguageId, PULONG pulResult, PULONG pulMarkUp, ULONG ulOemCP)
{
	return theDocument->CPP_EQFBWordCntPerSeg(pVoidTable, pTokBuf, pData, sLanguageId, pulResult, pulMarkUp, ulOemCP);
}
PTBDOCUMENT EQFBRemoveDoc(PTBDOCUMENT pDoc)
{
	return theDocument->CPP_EQFBRemoveDoc(pDoc);
}
BOOL EQFBCheckNoneTag(PTBDOCUMENT pDoc, PSZ_W pString)
{
	return theDocument->CPP_EQFBCheckNoneTag(pDoc, pString);
}
VOID EQFBNormSeg(PTBDOCUMENT pDoc, PSZ_W pData, PSZ_W pOutData)
{
	return theDocument->CPP_EQFBNormSeg(pDoc, pData, pOutData);
}
USHORT EQFBWriteNextSegment(PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usConversion, ULONG ulCP, PBOOL pfDone )
{
	return theDocument->CPP_EQFBWriteNextSegment(pDoc, pvFileWriteData, usConversion, ulCP, pfDone );
}
USHORT EQFBPrepareFileWrite( PTBDOCUMENT pDoc, PVOID  *ppvFileWriteData, PSZ pszFileName, SHORT sLogTaskID, USHORT  usCPConversion, EQF_BOOL fAutoSave )
{
	return theDocument->CPP_EQFBPrepareFileWrite( pDoc, ppvFileWriteData, pszFileName, sLogTaskID, usCPConversion, fAutoSave);
}
USHORT EQFBTerminateFileWrite( PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usRCIn )
{
	return theDocument->CPP_EQFBTerminateFileWrite( pDoc, pvFileWriteData, usRCIn );
}
BOOL EQFBOnTRNote(PTBDOCUMENT pDoc)
{
	return theDocument->CPP_EQFBOnTRNote(pDoc);
}
USHORT EQFBWriteHistLog(PSZ pszFolObjName, PSZ pszDocName, SHORT TaskId, USHORT usAddInfoLength, PVOID pvAddInfo, BOOL fMsg, HWND hwndErrMsg)
{
	return theDocument->CPP_EQFBWriteHistLog(pszFolObjName, pszDocName, TaskId, usAddInfoLength, pvAddInfo, fMsg, hwndErrMsg);
}
USHORT EQFBWriteHistLog2(PSZ pszFolObjName, PSZ pszDocName, SHORT TaskId, USHORT usAddInfoLength, PVOID pvAddInfo, BOOL fMsg, HWND hwndErrMsg, PSZ pszLongDocName)
{
	return theDocument->CPP_EQFBWriteHistLog2(pszFolObjName, pszDocName, TaskId, usAddInfoLength, pvAddInfo, fMsg, hwndErrMsg, pszLongDocName);
}
void HistLogCorrectRecSizes(PHISTLOGRECORD pRecord)
{
	return theDocument->CPP_HistLogCorrectRecSizes(pRecord);
}
SHORT EQFBHistDocSave(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID)
{
	return theDocument->CPP_EQFBHistDocSave(pszFileName, pDoc, sLogTaskID);
}
SHORT EQFBHistDocSaveEx(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID )
{
	return theDocument->CPP_EQFBHistDocSaveEx(pszFileName, pDoc, sLogTaskID );
}
USHORT EQFBFileRead(PSZ pszFileName, PTBDOCUMENT pDoc)
{
	return theDocument->CPP_EQFBFileRead(pszFileName, pDoc);
}
USHORT EQFBFileReadExW(PSZ pszFileName, PTBDOCUMENT pDoc, LONG lFlags)
{
	return theDocument->CPP_EQFBFileReadExW(pszFileName, pDoc, lFlags);
}
void EQFBFreeDoc(PTBDOCUMENT *ppDoc, ULONG ulOptions)
{
	return theDocument->CPP_EQFBFreeDoc(ppDoc, ulOptions);
}
USHORT EQFBFileWrite(PSZ pszFileName, PTBDOCUMENT pDoc)
{
	return theDocument->CPP_EQFBFileWrite(pszFileName, pDoc);
}
USHORT EQFBFileWriteEx(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID, USHORT usCPConversion)
{
	return theDocument->CPP_EQFBFileWriteEx(pszFileName, pDoc, sLogTaskID, usCPConversion);
}
PTBSEGMENT EQFBGetSeg(PTBDOCUMENT pDoc, ULONG ulSeg)
{
	return theDocument->CPP_EQFBGetSeg(pDoc, ulSeg);
}
PTBSEGMENT EQFBGetFromBothTables(PTBDOCUMENT pDoc, PULONG pulStandardIndex, PULONG pulAdditionalIndex, PULONG pulLastTable)
{
	return theDocument->CPP_EQFBGetFromBothTables(pDoc, pulStandardIndex, pulAdditionalIndex, pulLastTable);
}
VOID EQFBBufRemoveTRNote(PSZ_W pData, PVOID pDocTagTable, PFN pfnUserExit, PFN pfnUserExitW, ULONG ulOemCP)
{
	return theDocument->CPP_EQFBBufRemoveTRNote(pData, pDocTagTable, pfnUserExit, pfnUserExitW, ulOemCP);
}
SHORT EQFBAddSeg(PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg)
{
	return theDocument->CPP_EQFBAddSeg(pDoc, pNewSeg);
}
SHORT EQFBAddSegW(PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg)
{
	return theDocument->CPP_EQFBAddSegW(pDoc, pNewSeg);
}
USHORT EQFBBuildCountCheckSum(USHORT usCountFlags, USHORT usSrcWords, USHORT usTgtWords, USHORT usModWords)
{
	return theDocument->CPP_EQFBBuildCountCheckSum(usCountFlags, usSrcWords, usTgtWords, usModWords);
}
VOID EQFBFillWriteAttr(PVOID pQFTagTable, PSZ pszMarkAttr, PSZ pszNoCountAttr, PSZ pszCurrentAttr, PSZ pszJoinAttr, PSZ pszNAttr, PSZ pszStatusAttr, PSZ pszCountAttr)
{
	return theDocument->CPP_EQFBFillWriteAttr(pQFTagTable, pszMarkAttr, pszNoCountAttr, pszCurrentAttr, pszJoinAttr, pszNAttr, pszStatusAttr, pszCountAttr);
}
VOID EQFBFillWriteAttrW(PVOID pQFTagTable, PSZ_W pszMarkAttr, PSZ_W pszNoCountAttr, PSZ_W pszCurrentAttr, PSZ_W pszJoinAttr, PSZ_W pszNAttr, PSZ_W pszStatusAttr, PSZ_W pszCountAttr)
{
	return theDocument->CPP_EQFBFillWriteAttrW(pQFTagTable, pszMarkAttr, pszNoCountAttr, pszCurrentAttr, pszJoinAttr, pszNAttr, pszStatusAttr, pszCountAttr);
}
BOOL EQFBGetHexNumberW(PSZ_W pszNumber, PUSHORT pusValue)
{
	return theDocument->CPP_EQFBGetHexNumberW(pszNumber, pusValue);
}

#ifndef CPPTEST
}
#endif
