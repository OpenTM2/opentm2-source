/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#include "core\PluginManager\PluginManager.h"
#include "EqfDictionary.h"
#ifndef CPPTEST
extern "C" {
#endif
#include "OtmDictionaryIF.h"
#ifndef CPPTEST
}
#endif

EqfDictionary::EqfDictionary(
)
{
	name = "EqfDictionary";
	shortDesc = "EqfDictionary";
	longDesc = "This is the standard (EQF) dictionary implementation";
	version = "1.0";
	supplier = "International Business Machines Corporation";
}

EqfDictionary::~EqfDictionary()
{
}

const char* EqfDictionary::getName()
{
	return name.c_str();
}

const char* EqfDictionary::getShortDescription()
{
	return shortDesc.c_str();
}

const char* EqfDictionary::getLongDescription()
{
	return longDesc.c_str();
}

const char* EqfDictionary::getVersion()
{
	return version.c_str();
}

const char* EqfDictionary::getSupplier()
{
	return supplier.c_str();
}

USHORT  EqfDictionary::CPP_AsdInsEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PSZ_W pucData, ULONG ulLength, PULONG pulTermNo)
{
	return AsdInsEntry(hUCB, hDCB, pszTerm, pucData, ulLength, pulTermNo);
}
USHORT  EqfDictionary::CPP_AsdInsEntryNonUnicode(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PUCHAR pucData, ULONG ulLength, PULONG pulTermNo)
{
	return AsdInsEntryNonUnicode(hUCB, hDCB, pszTerm, pucData, ulLength, pulTermNo);
}
USHORT  EqfDictionary::CPP_AsdRepEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PSZ_W pucData, ULONG ulLength, PULONG pulTermNo)
{
	return AsdRepEntry(hUCB, hDCB, pszTerm, pucData, ulLength, pulTermNo);
}
USHORT  EqfDictionary::CPP_AsdDelEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm)
{
	return AsdDelEntry(hUCB, hDCB, pszTerm);
}
USHORT  EqfDictionary::CPP_AsdBuild(HUCB hUCB, BOOL fGuarded, PHDCB phDCB, PSZ pszDictName)
{
	return AsdBuild(hUCB, fGuarded, phDCB, pszDictName);
}
USHORT  EqfDictionary::CPP_AsdTranslate(HUCB hUCB, HDCB hDCB,PUCHAR pucSegment, USHORT usOutBufSize, PUCHAR pucOutBuf, USHORT usMode)
{
	return AsdTranslate(hUCB, hDCB, pucSegment, usOutBufSize, pucOutBuf, usMode);
}
USHORT  EqfDictionary::CPP_AsdTranslateW(HUCB hUCB, HDCB hDCB, PSZ_W pucSegment, USHORT usOutBufSize, PSZ_W pucOutBuf, USHORT usMode)
{
	return AsdTranslateW(hUCB, hDCB, pucSegment, usOutBufSize, pucOutBuf, usMode);
}
USHORT  EqfDictionary::CPP_AsdFndBegin(PSZ_W pucSubString, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdFndBegin(pucSubString, hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdFndEquiv(PUCHAR pucTermIn, HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdFndEquiv(pucTermIn, hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdFndEquivW(PSZ_W pucTermIn, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdFndEquivW(pucTermIn, hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdFndMatch(PSZ_W pucTermIn, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdFndMatch(pucTermIn, hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdFndNumber(ULONG ulTermNumber, HDCB hDCB, USHORT usRelocation, HUCB hUCB, PSZ_W pucTerm, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdFndNumber(ulTermNumber, hDCB, usRelocation, hUCB, pucTerm, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdNxtTermW(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdNxtTermW(hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdNxtTerm(HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdNxtTerm(hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdPrvTerm(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdPrvTerm(hDCB, hUCB, pucTerm, pulTermNumber, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdRenumber(HDCB hDCB, HUCB hUCB)
{
	return AsdRenumber(hDCB, hUCB);
}
USHORT  EqfDictionary::CPP_AsdRetEntry(HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PUCHAR pucEntryData, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdRetEntry(hDCB, hUCB, pucTerm, pulTermNumber, pucEntryData, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdRetEntryW(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PSZ_W pucEntryData, PULONG pulDataLength, PHDCB phDCB)
{
	return AsdRetEntryW(hDCB, hUCB, pucTerm, pulTermNumber, pucEntryData, pulDataLength, phDCB);
}
USHORT  EqfDictionary::CPP_AsdListIndex(HUCB hUCB, HDCB hDCB, PSZ_W pucTerm, USHORT usTermType, PSZ_W *ppucTermList, PLONG plUsed, PLONG plSize, PUSHORT pusTerms, BOOL fTermInStemForm)
{
	return AsdListIndex(hUCB, hDCB, pucTerm, usTermType, ppucTermList, plUsed, plSize, pusTerms, fTermInStemForm);
}
USHORT  EqfDictionary::CPP_AsdDelete(PSZ pszPropName)
{
	return AsdDelete(pszPropName);
}
USHORT  EqfDictionary::CPP_AsdRename(PSZ pszOldPropName, PSZ pszNewPropName)
{
	return AsdRename(pszOldPropName, pszNewPropName);
}
USHORT  EqfDictionary::CPP_AsdRetPropPtr(HUCB hUCB, HDCB hDCB, PPROPDICTIONARY *ppProp)
{
	return AsdRetPropPtr(hUCB, hDCB, ppProp);
}
USHORT  EqfDictionary::CPP_AsdGetStemForm(HUCB hUCB, HDCB hDCB, PSZ_W pszInTerm, PSZ_W pszOutTerm)
{
	return AsdGetStemForm(hUCB, hDCB, pszInTerm, pszOutTerm);
}
USHORT  EqfDictionary::CPP_AsdEntryChange(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm)
{
	return AsdEntryChange(hUCB, hDCB, pszTerm);
}
BOOL  EqfDictionary::CPP_AsdIsUcbOK(HUCB hUCB)
{
	return AsdIsUcbOK(hUCB);
}
BOOL  EqfDictionary::CPP_AsdIsDcbOK(HDCB hDCB)
{
	return AsdIsDcbOK(hDCB);
}
BOOL  EqfDictionary::CPP_AsdQueryDictName(HDCB hDCB, PSZ pszDictName)
{
	return AsdQueryDictName(hDCB, pszDictName);
}
BOOL  EqfDictionary::CPP_AsdQueryDictShortName(HDCB hDCB, PSZ pszDictName)
{
	return AsdQueryDictShortName(hDCB, pszDictName);
}
USHORT  EqfDictionary::CPP_AsdRetDictList(HDCB hDCB, HDCB *ahDCB, PUSHORT pusNumOfDicts)
{
	return AsdRetDictList(hDCB, ahDCB, pusNumOfDicts);
}
USHORT  EqfDictionary::CPP_AsdRetUserHandle(HUCB hUCB, PUSHORT pusUserHandle)
{
	return AsdRetUserHandle(hUCB, pusUserHandle);
}
USHORT  EqfDictionary::CPP_AsdRetServiceHandle(HUCB hUCB, PUSHORT pusServiceHandle)
{
	return AsdRetServiceHandle(hUCB, pusServiceHandle);
}
USHORT  EqfDictionary::CPP_AsdRetMorphHandle(HDCB hDCB, PUSHORT pusMorphHandle)
{
	return AsdRetMorphHandle(hDCB, pusMorphHandle);
}
USHORT  EqfDictionary::CPP_AsdMergeEntry(HUCB hUCB, HDCB hDCBSource, PSZ_W pucTerm, ULONG ulDataLength, PSZ_W pucData, HDCB hDCBTarget, PUSHORT pusControlFlags)
{
	return AsdMergeEntry(hUCB, hDCBSource, pucTerm, ulDataLength, pucData, hDCBTarget, pusControlFlags);
}
USHORT  EqfDictionary::CPP_AsdNumEntries(HDCB hDCB, PULONG pulNumber)
{
	return AsdNumEntries(hDCB, pulNumber);
}
USHORT  EqfDictionary::CPP_AsdDictVersion(HDCB hDCB, PUSHORT pusVersion)
{
	return AsdDictVersion(hDCB, pusVersion);
}
USHORT  EqfDictionary::CPP_AsdDictSetOldVersion(HDCB hDCB)
{
	return AsdDictSetOldVersion(hDCB);
}
USHORT  EqfDictionary::CPP_AsdLockDict(HDCB hDCB, BOOL fLock)
{
	return AsdLockDict(hDCB, fLock);
}
USHORT  EqfDictionary::CPP_AsdLockEntry(HDCB hDCB, PSZ_W pszTerm, BOOL fLock)
{
	return AsdLockEntry(hDCB, pszTerm, fLock);
}
USHORT  EqfDictionary::CPP_AsdDeleteRemote(PSZ pszPropName)
{
	return AsdDeleteRemote(pszPropName);
}
USHORT  EqfDictionary::CPP_AsdHandleFromDCB(HDCB hDCB, PUSHORT pusDictHandle, PUSHORT pusIndexHandle, PVOID * ppBTreeDict, PVOID * ppBTreeIndex, PLONG pLHandle)
{
	return AsdHandleFromDCB(hDCB, pusDictHandle, pusIndexHandle, ppBTreeDict, ppBTreeIndex, pLHandle);
}
USHORT  EqfDictionary::CPP_AsdCloseOrganize(HUCB hUCB, HDCB hDCB, PSZ pszDictPath, CHAR chPrimDrive, USHORT usRc)
{
	return AsdCloseOrganize(hUCB, hDCB, pszDictPath, chPrimDrive, usRc);
}
USHORT  EqfDictionary::CPP_AsdRCHandling(USHORT usAsdRC, HDCB hDCB, PSZ pszDict, PSZ pszServer, PSZ_W pszTerm, HWND hwnd)
{
	return AsdRCHandling(usAsdRC, hDCB, pszDict, pszServer, pszTerm, hwnd);
}
USHORT  EqfDictionary::CPP_AsdTermList(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, USHORT usNumTerms, USHORT usAction, PSZ_W pucBuffer, USHORT usLen)
{
	return AsdTermList(hUCB, hDCB, pszTerm, usNumTerms, usAction, pucBuffer, usLen);
}
USHORT  EqfDictionary::CPP_AsdWildCardList(HUCB hUCB, HDCB hDCB, PSZ_W pszStartTerm, PSZ_W pszPattern, USHORT usNumTerms, BOOL fCompound, PSZ_W pucBuffer, USHORT usLen)
{
	return AsdWildCardList(hUCB, hDCB, pszStartTerm, pszPattern, usNumTerms, fCompound, pucBuffer, usLen);
}
USHORT  EqfDictionary::CPP_AsdGetTranslation(HUCB hUCB, HDCB hDCB, PUCHAR pucDictData, USHORT usOutBufSize, PUCHAR pucOutBuf, SHORT sListType)
{
	return AsdGetTranslation(hUCB, hDCB, pucDictData, usOutBufSize, pucOutBuf, sListType);
}
USHORT  EqfDictionary::CPP_AsdGetTranslationW(HUCB hUCB, HDCB hDCB, PSZ_W pucDictData, USHORT usOutBufSize, PSZ_W pucOutBuf, SHORT sListType)
{
	return AsdGetTranslationW(hUCB, hDCB, pucDictData, usOutBufSize, pucOutBuf, sListType);
}
USHORT  EqfDictionary::CPP_AsdUpdTime(HDCB hDCB, PLONG plUpdTime)
{
	return AsdUpdTime(hDCB, plUpdTime);
}
USHORT  EqfDictionary::CPP_AsdLoadDictProperties(PSZ pszPropFile, PPROPDICTIONARY pDictProp)
{
	return AsdLoadDictProperties(pszPropFile, pDictProp);
}
USHORT  EqfDictionary::CPP_AsdResynch(HUCB hUCB, HDCB hDCB)
{
	return AsdResynch(hUCB, hDCB);
}
ULONG EqfDictionary::CPP_GetCPFromDCB(HDCB hDCB)
{
	return GetCPFromDCB(hDCB);
}
