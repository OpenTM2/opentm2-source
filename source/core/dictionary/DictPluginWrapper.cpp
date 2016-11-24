//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2013, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmDictionaryPlugin.h"
#include "core\PluginManager\OtmDictionary.h"
#ifndef CPPTEST
extern "C" {
#endif
#include "OtmDictionaryIF.h"
#ifndef CPPTEST
}
#endif

static OtmDictionaryPlugin* thePlugin = 0;
static OtmDictionary* theDictionary = 0;


__declspec(dllexport)
void InitDictPluginWrapper()
{
	PluginManager* thePluginManager = PluginManager::getInstance();
	thePlugin = (OtmDictionaryPlugin*) thePluginManager->getPlugin(OtmPlugin::eDictionaryType);
	if ( thePlugin != NULL ) theDictionary = thePlugin->CreateDict();
}

USHORT AsdBegin(USHORT usMaxDicts, PHUCB phUCB)
{
	return thePlugin->CPP_AsdBegin(usMaxDicts, phUCB);
}
USHORT AsdEnd(HUCB hUCB)
{
	return thePlugin->CPP_AsdEnd(hUCB);
}
USHORT AsdOpen(HUCB hUCB, USHORT usOpenFlags, USHORT usNumDicts, PSZ *ppszDicts, PHDCB phDCB, PUSHORT pusErrDict)
{
	return thePlugin->CPP_AsdOpen(hUCB, usOpenFlags, usNumDicts, ppszDicts, phDCB, pusErrDict);
}
USHORT AsdClose(HUCB hUCB, HDCB hDCB)
{
	return thePlugin->CPP_AsdClose(hUCB, hDCB);
}
USHORT AsdInsEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PSZ_W pucData, ULONG ulLength, PULONG pulTermNo)
{
	return theDictionary->CPP_AsdInsEntry(hUCB, hDCB, pszTerm, pucData, ulLength, pulTermNo);
}
USHORT AsdInsEntryNonUnicode(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PUCHAR pucData, ULONG ulLength, PULONG pulTermNo)
{
	return theDictionary->CPP_AsdInsEntryNonUnicode(hUCB, hDCB, pszTerm, pucData, ulLength, pulTermNo);
}
USHORT AsdRepEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PSZ_W pucData, ULONG ulLength, PULONG pulTermNo)
{
	return theDictionary->CPP_AsdRepEntry(hUCB, hDCB, pszTerm, pucData, ulLength, pulTermNo);
}
USHORT AsdDelEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm)
{
	return theDictionary->CPP_AsdDelEntry(hUCB, hDCB, pszTerm);
}
USHORT AsdBuild(HUCB hUCB, BOOL fGuarded, PHDCB phDCB, PSZ pszDictName)
{
	return theDictionary->CPP_AsdBuild(hUCB, fGuarded, phDCB, pszDictName);
}
USHORT AsdTranslate(HUCB hUCB, HDCB hDCB,PUCHAR pucSegment, USHORT usOutBufSize, PUCHAR pucOutBuf, USHORT usMode)
{
	return theDictionary->CPP_AsdTranslate(hUCB, hDCB, pucSegment, usOutBufSize, pucOutBuf, usMode);
}
USHORT AsdTranslateW(HUCB hUCB, HDCB hDCB, PSZ_W pucSegment, USHORT usOutBufSize, PSZ_W pucOutBuf, USHORT usMode)
{
	return theDictionary->CPP_AsdTranslateW(hUCB, hDCB, pucSegment, usOutBufSize, pucOutBuf, usMode);
}
USHORT AsdFndBegin(PSZ_W pucSubString, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdFndBegin(pucSubString, hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT AsdFndEquiv(PUCHAR pucTermIn, HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdFndEquiv(pucTermIn, hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT AsdFndEquivW(PSZ_W pucTermIn, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdFndEquivW(pucTermIn, hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT AsdFndMatch(PSZ_W pucTermIn, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdFndMatch(pucTermIn, hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT AsdFndNumber(ULONG ulTermNumber, HDCB hDCB, USHORT usRelocation, HUCB hUCB, PSZ_W pucTerm, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdFndNumber(ulTermNumber, hDCB, usRelocation, hUCB, pucTerm, pulDataLength, phDCB);
}
USHORT AsdNxtTermW(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdNxtTermW(hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT AsdNxtTerm(HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdNxtTerm(hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT AsdPrvTerm(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdPrvTerm(hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT AsdRenumber(HDCB hDCB, HUCB hUCB)
{
	return theDictionary->CPP_AsdRenumber(hDCB, hUCB);
}
USHORT AsdRetEntry(HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PUCHAR pucEntryData, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdRetEntry(hDCB, hUCB, pucTerm, pulTermNumber, pucEntryData, pulDataLength, phDCB);
}
USHORT AsdRetEntryW(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PSZ_W pucEntryData, PULONG pulDataLength, PHDCB phDCB)
{
	return theDictionary->CPP_AsdRetEntryW(hDCB, hUCB, pucTerm, pulTermNumber, pucEntryData, pulDataLength, phDCB);
}
USHORT AsdListIndex(HUCB hUCB, HDCB hDCB, PSZ_W pucTerm, USHORT usTermType, PSZ_W *ppucTermList, PLONG plUsed, PLONG plSize, PUSHORT pusTerms, BOOL fTermInStemForm)
{
	return theDictionary->CPP_AsdListIndex(hUCB, hDCB, pucTerm, usTermType, ppucTermList, plUsed, plSize, pusTerms, fTermInStemForm);
}
USHORT AsdDelete(PSZ pszPropName)
{
	return theDictionary->CPP_AsdDelete(pszPropName);
}
USHORT AsdRename(PSZ pszOldPropName, PSZ pszNewPropName)
{
	return theDictionary->CPP_AsdRename(pszOldPropName, pszNewPropName);
}
USHORT AsdRetPropPtr(HUCB hUCB, HDCB hDCB, PPROPDICTIONARY *ppProp)
{
	return theDictionary->CPP_AsdRetPropPtr(hUCB, hDCB, ppProp);
}
USHORT AsdGetStemForm(HUCB hUCB, HDCB hDCB, PSZ_W pszInTerm, PSZ_W pszOutTerm)
{
	return theDictionary->CPP_AsdGetStemForm(hUCB, hDCB, pszInTerm, pszOutTerm);
}
USHORT AsdEntryChange(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm)
{
	return theDictionary->CPP_AsdEntryChange(hUCB, hDCB, pszTerm);
}
BOOL AsdIsUcbOK(HUCB hUCB)
{
	return theDictionary->CPP_AsdIsUcbOK(hUCB);
}
BOOL AsdIsDcbOK(HDCB hDCB)
{
	return theDictionary->CPP_AsdIsDcbOK(hDCB);
}
BOOL AsdQueryDictName(HDCB hDCB, PSZ pszDictName)
{
	return theDictionary->CPP_AsdQueryDictName(hDCB, pszDictName);
}
BOOL AsdQueryDictShortName(HDCB hDCB, PSZ pszDictName)
{
	return theDictionary->CPP_AsdQueryDictShortName(hDCB, pszDictName);
}
USHORT AsdRetDictList(HDCB hDCB, HDCB *ahDCB, PUSHORT pusNumOfDicts)
{
	return theDictionary->CPP_AsdRetDictList(hDCB, ahDCB, pusNumOfDicts);
}
USHORT AsdRetUserHandle(HUCB hUCB, PUSHORT pusUserHandle)
{
	return theDictionary->CPP_AsdRetUserHandle(hUCB, pusUserHandle);
}
USHORT AsdRetServiceHandle(HUCB hUCB, PUSHORT pusServiceHandle)
{
	return theDictionary->CPP_AsdRetServiceHandle(hUCB, pusServiceHandle);
}
USHORT AsdRetMorphHandle(HDCB hDCB, PUSHORT pusMorphHandle)
{
	return theDictionary->CPP_AsdRetMorphHandle(hDCB, pusMorphHandle);
}
USHORT AsdMergeEntry(HUCB hUCB, HDCB hDCBSource, PSZ_W pucTerm, ULONG ulDataLength, PSZ_W pucData, HDCB hDCBTarget, PUSHORT pusControlFlags)
{
	return theDictionary->CPP_AsdMergeEntry(hUCB, hDCBSource, pucTerm, ulDataLength, pucData, hDCBTarget, pusControlFlags);
}
USHORT AsdNumEntries(HDCB hDCB, PULONG pulNumber)
{
	return theDictionary->CPP_AsdNumEntries(hDCB, pulNumber);
}
USHORT AsdDictVersion(HDCB hDCB, PUSHORT pusVersion)
{
	return theDictionary->CPP_AsdDictVersion(hDCB, pusVersion);
}
USHORT AsdDictSetOldVersion(HDCB hDCB)
{
	return theDictionary->CPP_AsdDictSetOldVersion(hDCB);
}
USHORT AsdLockDict(HDCB hDCB, BOOL fLock)
{
	return theDictionary->CPP_AsdLockDict(hDCB, fLock);
}
USHORT AsdLockEntry(HDCB hDCB, PSZ_W pszTerm, BOOL fLock)
{
	return theDictionary->CPP_AsdLockEntry(hDCB, pszTerm, fLock);
}
USHORT AsdDeleteRemote(PSZ pszPropName)
{
	return theDictionary->CPP_AsdDeleteRemote(pszPropName);
}
USHORT AsdHandleFromDCB(HDCB hDCB, PUSHORT pusDictHandle, PUSHORT pusIndexHandle, PVOID * ppBTreeDict, PVOID * ppBTreeIndex, PLONG pLHandle)
{
	return theDictionary->CPP_AsdHandleFromDCB(hDCB, pusDictHandle, pusIndexHandle, ppBTreeDict, ppBTreeIndex, pLHandle);
}
USHORT AsdCloseOrganize(HUCB hUCB, HDCB hDCB, PSZ pszDictPath, CHAR chPrimDrive, USHORT usRc)
{
	return theDictionary->CPP_AsdCloseOrganize(hUCB, hDCB, pszDictPath, chPrimDrive, usRc);
}
USHORT AsdRCHandling(USHORT usAsdRC, HDCB hDCB, PSZ pszDict, PSZ pszServer, PSZ_W pszTerm, HWND hwnd)
{
	return theDictionary->CPP_AsdRCHandling(usAsdRC, hDCB, pszDict, pszServer, pszTerm, hwnd);
}
USHORT AsdTermList(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, USHORT usNumTerms, USHORT usAction, PSZ_W pucBuffer, USHORT usLen)
{
	return theDictionary->CPP_AsdTermList(hUCB, hDCB, pszTerm, usNumTerms, usAction, pucBuffer, usLen);
}
USHORT AsdWildCardList(HUCB hUCB, HDCB hDCB, PSZ_W pszStartTerm, PSZ_W pszPattern, USHORT usNumTerms, BOOL fCompound, PSZ_W pucBuffer, USHORT usLen)
{
	return theDictionary->CPP_AsdWildCardList(hUCB, hDCB, pszStartTerm, pszPattern, usNumTerms, fCompound, pucBuffer, usLen);
}
USHORT AsdGetTranslation(HUCB hUCB, HDCB hDCB, PUCHAR pucDictData, USHORT usOutBufSize, PUCHAR pucOutBuf, SHORT sListType)
{
	return theDictionary->CPP_AsdGetTranslation(hUCB, hDCB, pucDictData, usOutBufSize, pucOutBuf, sListType);
}
USHORT AsdGetTranslationW(HUCB hUCB, HDCB hDCB, PSZ_W pucDictData, USHORT usOutBufSize, PSZ_W pucOutBuf, SHORT sListType)
{
	return theDictionary->CPP_AsdGetTranslationW(hUCB, hDCB, pucDictData, usOutBufSize, pucOutBuf, sListType);
}
USHORT AsdUpdTime(HDCB hDCB, PLONG plUpdTime)
{
	return theDictionary->CPP_AsdUpdTime(hDCB, plUpdTime);
}
USHORT AsdLoadDictProperties(PSZ pszPropFile, PPROPDICTIONARY pDictProp)
{
	return theDictionary->CPP_AsdLoadDictProperties(pszPropFile, pDictProp);
}
USHORT AsdResynch(HUCB hUCB, HDCB hDCB)
{
	return theDictionary->CPP_AsdResynch(hUCB, hDCB);
}
ULONG GetCPFromDCB(HDCB hDCB)
{
	return theDictionary->CPP_GetCPFromDCB(hDCB);
}
