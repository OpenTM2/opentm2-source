/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#include "core\PluginManager\PluginManager.h"
#include "EqfDocument.h"

EqfDocument::EqfDocument()
{
	name = "EqfDocument";
	shortDesc = "DocumentPlugin";
	longDesc = "This is the standard document implementation";
	version = "1.0";
	supplier = "International Business Machines Corporation";
}

EqfDocument::~EqfDocument()
{
}

const char* EqfDocument::getName()
{
	return name.c_str();
}

const char* EqfDocument::getShortDescription()
{
	return shortDesc.c_str();
}

const char* EqfDocument::getLongDescription()
{
	return longDesc.c_str();
}

const char* EqfDocument::getVersion()
{
	return version.c_str();
}

const char* EqfDocument::getSupplier()
{
	return supplier.c_str();
}

USHORT EqfDocument::CPP_EQFBCharType(PTBDOCUMENT pDoc, PTBSEGMENT pSeg, USHORT usOffs)
{
	return EQFBCharType(pDoc, pSeg, usOffs);
}
PTBSEGMENT EqfDocument::CPP_EQFBGetSegW(PTBDOCUMENT pDoc, ULONG ulSeg)
{
	return EQFBGetSegW(pDoc, ulSeg);
}
BOOL EqfDocument::CPP_EQFBFileExists(PSZ pszFile)
{
	return EQFBFileExists(pszFile);
}
BOOL EqfDocument::CPP_EQFBDiffTag(PTBDOCUMENT pDoc, ULONG ulSeg, USHORT usOffs)
{
	return EQFBDiffTag(pDoc, ulSeg, usOffs);
}
BOOL EqfDocument::CPP_EQFBIsLFProtected(PTBSEGMENT pSeg, SHORT sOffs)
{
	return EQFBIsLFProtected(pSeg, sOffs);
}
BOOL EqfDocument::CPP_EQFBDiffProtectTag(PTBDOCUMENT pDoc, ULONG ulSeg, USHORT usOffs)
{
	return EQFBDiffProtectTag(pDoc, ulSeg, usOffs);
}
VOID EqfDocument::CPP_EQFBReparse(PTBDOCUMENT pDoc, PTBSEGMENT pSeg, USHORT usOffs, SHORT sDiff)
{
	return EQFBReparse(pDoc, pSeg, usOffs, sDiff);
}
SHORT EqfDocument::CPP_EQFBLineUp(PTBDOCUMENT pDoc)
{
	return EQFBLineUp(pDoc);
}
SHORT EqfDocument::CPP_EQFBLineDown(PTBDOCUMENT pDoc)
{
	return EQFBLineDown(pDoc);
}
SHORT EqfDocument::CPP_EQFBDocLoad(PLOADSTRUCT pLoad)
{
	return EQFBDocLoad(pLoad);
}
PTBDOCUMENT EqfDocument::CPP_EQFBDocDelete(PTBDOCUMENT pDoc)
{
	return EQFBDocDelete(pDoc);
}
SHORT EqfDocument::CPP_EQFBDocSave(PTBDOCUMENT pDoc, PSZ pszFileName, BOOL fAskForSave)
{
	return EQFBDocSave(pDoc, pszFileName, fAskForSave);
}
VOID EqfDocument::CPP_EQFBFuncOpenTRNote(PTBDOCUMENT pDoc)
{
	return EQFBFuncOpenTRNote(pDoc);
}
VOID EqfDocument::CPP_EQFBDocPrint(PTBDOCUMENT pDoc)
{
	return EQFBDocPrint(pDoc);
}
SHORT EqfDocument::CPP_EQFBWordCntPerSeg(PVOID pVoidTable, PTOKENENTRY pTokBuf, PSZ_W pData, SHORT sLanguageId, PULONG pulResult, PULONG pulMarkUp, ULONG ulOemCP)
{
	return EQFBWordCntPerSeg(pVoidTable, pTokBuf, pData, sLanguageId, pulResult, pulMarkUp, ulOemCP);
}
PTBDOCUMENT EqfDocument::CPP_EQFBRemoveDoc(PTBDOCUMENT pDoc)
{
	return EQFBRemoveDoc(pDoc);
}
BOOL EqfDocument::CPP_EQFBCheckNoneTag(PTBDOCUMENT pDoc, PSZ_W pString)
{
	return EQFBCheckNoneTag(pDoc, pString);
}
VOID EqfDocument::CPP_EQFBNormSeg(PTBDOCUMENT pDoc, PSZ_W pData, PSZ_W pOutData)
{
	return EQFBNormSeg(pDoc, pData, pOutData);
}
USHORT EqfDocument::CPP_EQFBWriteNextSegment(PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usConversion, ULONG ulCP, PBOOL pfDone )
{
	return EQFBWriteNextSegment(pDoc, pvFileWriteData, usConversion, ulCP, pfDone );
}
USHORT EqfDocument::CPP_EQFBPrepareFileWrite( PTBDOCUMENT pDoc, PVOID  *ppvFileWriteData, PSZ pszFileName, SHORT sLogTaskID, USHORT  usCPConversion, EQF_BOOL fAutoSave )
{
	return EQFBPrepareFileWrite( pDoc, ppvFileWriteData, pszFileName, sLogTaskID, usCPConversion, fAutoSave);
}
USHORT EqfDocument::CPP_EQFBTerminateFileWrite( PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usRCIn )
{
	return EQFBTerminateFileWrite( pDoc, pvFileWriteData, usRCIn );
}

BOOL EqfDocument::CPP_EQFBOnTRNote(PTBDOCUMENT pDoc)
{
	return EQFBOnTRNote(pDoc);
}
USHORT EqfDocument::CPP_EQFBWriteHistLog(PSZ pszFolObjName, PSZ pszDocName, SHORT TaskId, USHORT usAddInfoLength, PVOID pvAddInfo, BOOL fMsg, HWND hwndErrMsg)
{
	return EQFBWriteHistLog(pszFolObjName, pszDocName, TaskId, usAddInfoLength, pvAddInfo, fMsg, hwndErrMsg);
}
USHORT EqfDocument::CPP_EQFBWriteHistLog2(PSZ pszFolObjName, PSZ pszDocName, SHORT TaskId, USHORT usAddInfoLength, PVOID pvAddInfo, BOOL fMsg, HWND hwndErrMsg, PSZ pszLongDocName)
{
	return EQFBWriteHistLog2(pszFolObjName, pszDocName, TaskId, usAddInfoLength, pvAddInfo, fMsg, hwndErrMsg, pszLongDocName);
}
void EqfDocument::CPP_HistLogCorrectRecSizes(PHISTLOGRECORD pRecord)
{
	return HistLogCorrectRecSizes(pRecord);
}
SHORT EqfDocument::CPP_EQFBHistDocSave(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID)
{
	return EQFBHistDocSave(pszFileName, pDoc, sLogTaskID);
}
SHORT EqfDocument::CPP_EQFBHistDocSaveEx(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID, BOOL fAdjustCountInfo )
{
	return EQFBHistDocSaveEx(pszFileName, pDoc, sLogTaskID, fAdjustCountInfo );
}
USHORT EqfDocument::CPP_EQFBFileRead(PSZ pszFileName, PTBDOCUMENT pDoc)
{
	return EQFBFileRead(pszFileName, pDoc);
}
USHORT EqfDocument::CPP_EQFBFileReadExW(PSZ pszFileName, PTBDOCUMENT pDoc, LONG lFlags)
{
	return EQFBFileReadExW(pszFileName, pDoc, lFlags);
}
void EqfDocument::CPP_EQFBFreeDoc(PTBDOCUMENT *ppDoc, ULONG ulOptions)
{
	return EQFBFreeDoc(ppDoc, ulOptions);
}
USHORT EqfDocument::CPP_EQFBFileWrite(PSZ pszFileName, PTBDOCUMENT pDoc)
{
	return EQFBFileWrite(pszFileName, pDoc);
}
USHORT EqfDocument::CPP_EQFBFileWriteEx(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID, USHORT usCPConversion)
{
	return EQFBFileWriteEx(pszFileName, pDoc, sLogTaskID, usCPConversion);
}
PTBSEGMENT EqfDocument::CPP_EQFBGetSeg(PTBDOCUMENT pDoc, ULONG ulSeg)
{
	return EQFBGetSeg(pDoc, ulSeg);
}
PTBSEGMENT EqfDocument::CPP_EQFBGetFromBothTables(PTBDOCUMENT pDoc, PULONG pulStandardIndex, PULONG pulAdditionalIndex, PULONG pulLastTable)
{
	return EQFBGetFromBothTables(pDoc, pulStandardIndex, pulAdditionalIndex, pulLastTable);
}
VOID EqfDocument::CPP_EQFBBufRemoveTRNote(PSZ_W pData, PVOID pDocTagTable, PFN pfnUserExit, PFN pfnUserExitW, ULONG ulOemCP)
{
	return EQFBBufRemoveTRNote(pData, pDocTagTable, pfnUserExit, pfnUserExitW, ulOemCP);
}
SHORT EqfDocument::CPP_EQFBAddSeg(PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg)
{
	return EQFBAddSeg(pDoc, pNewSeg);
}
SHORT EqfDocument::CPP_EQFBAddSegW(PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg)
{
	return EQFBAddSegW(pDoc, pNewSeg);
}
USHORT EqfDocument::CPP_EQFBBuildCountCheckSum(USHORT usCountFlags, USHORT usSrcWords, USHORT usTgtWords, USHORT usModWords)
{
	return EQFBBuildCountCheckSum(usCountFlags, usSrcWords, usTgtWords, usModWords);
}
VOID EqfDocument::CPP_EQFBFillWriteAttr(PVOID pQFTagTable, PSZ pszMarkAttr, PSZ pszNoCountAttr, PSZ pszCurrentAttr, PSZ pszJoinAttr, PSZ pszNAttr, PSZ pszStatusAttr, PSZ pszCountAttr)
{
	return EQFBFillWriteAttr(pQFTagTable, pszMarkAttr, pszNoCountAttr, pszCurrentAttr, pszJoinAttr, pszNAttr, pszStatusAttr, pszCountAttr);
}
VOID EqfDocument::CPP_EQFBFillWriteAttrW(PVOID pQFTagTable, PSZ_W pszMarkAttr, PSZ_W pszNoCountAttr, PSZ_W pszCurrentAttr, PSZ_W pszJoinAttr, PSZ_W pszNAttr, PSZ_W pszStatusAttr, PSZ_W pszCountAttr)
{
	return EQFBFillWriteAttrW(pQFTagTable, pszMarkAttr, pszNoCountAttr, pszCurrentAttr, pszJoinAttr, pszNAttr, pszStatusAttr, pszCountAttr);
}
BOOL EqfDocument::CPP_EQFBGetHexNumberW(PSZ_W pszNumber, PUSHORT pusValue)
{
	return EQFBGetHexNumberW(pszNumber, pusValue);
}
