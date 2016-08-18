/*! \file
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMDICTIONARY_H_
#define _OTMDICTIONARY_H_

#ifndef CPPTEST
extern "C" {
#endif
#include "eqf.h"
#ifndef CPPTEST
}
#endif

/*! \brief Abstract base-class for dictionary objects */
class __declspec(dllexport) OtmDictionary
{

public:

/*! \brief Constructor */
	OtmDictionary() {};

/*! \brief Destructor */
	virtual ~OtmDictionary() {};

/*! \cond old wrapper stuff */
virtual USHORT CPP_AsdInsEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PSZ_W pucData, ULONG ulLength, PULONG pulTermNo) = 0;
virtual USHORT CPP_AsdInsEntryNonUnicode(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PUCHAR pucData, ULONG ulLength, PULONG pulTermNo) = 0;
virtual USHORT CPP_AsdRepEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PSZ_W pucData, ULONG ulLength, PULONG pulTermNo) = 0;
virtual USHORT CPP_AsdDelEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm) = 0;
virtual USHORT CPP_AsdBuild(HUCB hUCB, BOOL fGuarded, PHDCB phDCB, PSZ pszDictName) = 0;
virtual USHORT CPP_AsdTranslate(HUCB hUCB, HDCB hDCB,PUCHAR pucSegment, USHORT usOutBufSize, PUCHAR pucOutBuf, USHORT usMode) = 0;
virtual USHORT CPP_AsdTranslateW(HUCB hUCB, HDCB hDCB, PSZ_W pucSegment, USHORT usOutBufSize, PSZ_W pucOutBuf, USHORT usMode) = 0;
virtual USHORT CPP_AsdFndBegin(PSZ_W pucSubString, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdFndEquiv(PUCHAR pucTermIn, HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdFndEquivW(PSZ_W pucTermIn, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdFndMatch(PSZ_W pucTermIn, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdFndNumber(ULONG ulTermNumber, HDCB hDCB, USHORT usRelocation, HUCB hUCB, PSZ_W pucTerm, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdNxtTermW(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdNxtTerm(HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdPrvTerm(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdRenumber(HDCB hDCB, HUCB hUCB) = 0;
virtual USHORT CPP_AsdRetEntry(HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PUCHAR pucEntryData, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdRetEntryW(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PSZ_W pucEntryData, PULONG pulDataLength, PHDCB phDCB) = 0;
virtual USHORT CPP_AsdListIndex(HUCB hUCB, HDCB hDCB, PSZ_W pucTerm, USHORT usTermType, PSZ_W *ppucTermList, PLONG plUsed, PLONG plSize, PUSHORT pusTerms, BOOL fTermInStemForm) = 0;
virtual USHORT CPP_AsdDelete(PSZ pszPropName) = 0;
virtual USHORT CPP_AsdRename(PSZ pszOldPropName, PSZ pszNewPropName) = 0;
virtual USHORT CPP_AsdRetPropPtr(HUCB hUCB, HDCB hDCB, PPROPDICTIONARY *ppProp) = 0;
virtual USHORT CPP_AsdGetStemForm(HUCB hUCB, HDCB hDCB, PSZ_W pszInTerm, PSZ_W pszOutTerm) = 0;
virtual USHORT CPP_AsdEntryChange(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm) = 0;
virtual BOOL CPP_AsdIsUcbOK(HUCB hUCB) = 0;
virtual BOOL CPP_AsdIsDcbOK(HDCB hDCB) = 0;
virtual BOOL CPP_AsdQueryDictName(HDCB hDCB, PSZ pszDictName) = 0;
virtual BOOL CPP_AsdQueryDictShortName(HDCB hDCB, PSZ pszDictName) = 0;
virtual USHORT CPP_AsdRetDictList(HDCB hDCB, HDCB *ahDCB, PUSHORT pusNumOfDicts) = 0;
virtual USHORT CPP_AsdRetUserHandle(HUCB hUCB, PUSHORT pusUserHandle) = 0;
virtual USHORT CPP_AsdRetServiceHandle(HUCB hUCB, PUSHORT pusServiceHandle) = 0;
virtual USHORT CPP_AsdRetMorphHandle(HDCB hDCB, PUSHORT pusMorphHandle) = 0;
virtual USHORT CPP_AsdMergeEntry(HUCB hUCB, HDCB hDCBSource, PSZ_W pucTerm, ULONG ulDataLength, PSZ_W pucData, HDCB hDCBTarget, PUSHORT pusControlFlags) = 0;
virtual USHORT CPP_AsdNumEntries(HDCB hDCB, PULONG pulNumber) = 0;
virtual USHORT CPP_AsdDictVersion(HDCB hDCB, PUSHORT pusVersion) = 0;
virtual USHORT CPP_AsdDictSetOldVersion(HDCB hDCB) = 0;
virtual USHORT CPP_AsdLockDict(HDCB hDCB, BOOL fLock) = 0;
virtual USHORT CPP_AsdLockEntry(HDCB hDCB, PSZ_W pszTerm, BOOL fLock) = 0;
virtual USHORT CPP_AsdDeleteRemote(PSZ pszPropName) = 0;
virtual USHORT CPP_AsdHandleFromDCB(HDCB hDCB, PUSHORT pusDictHandle, PUSHORT pusIndexHandle, PVOID * ppBTreeDict, PVOID * ppBTreeIndex, PLONG pLHandle) = 0;
virtual USHORT CPP_AsdCloseOrganize(HUCB hUCB, HDCB hDCB, PSZ pszDictPath, CHAR chPrimDrive, USHORT usRc) = 0;
virtual USHORT CPP_AsdRCHandling(USHORT usAsdRC, HDCB hDCB, PSZ pszDict, PSZ pszServer, PSZ_W pszTerm, HWND hwnd) = 0;
virtual USHORT CPP_AsdTermList(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, USHORT usNumTerms, USHORT usAction, PSZ_W pucBuffer, USHORT usLen) = 0;
virtual USHORT CPP_AsdWildCardList(HUCB hUCB, HDCB hDCB, PSZ_W pszStartTerm, PSZ_W pszPattern, USHORT usNumTerms, BOOL fCompound, PSZ_W pucBuffer, USHORT usLen) = 0;
virtual USHORT CPP_AsdGetTranslation(HUCB hUCB, HDCB hDCB, PUCHAR pucDictData, USHORT usOutBufSize, PUCHAR pucOutBuf, SHORT sListType) = 0;
virtual USHORT CPP_AsdGetTranslationW(HUCB hUCB, HDCB hDCB, PSZ_W pucDictData, USHORT usOutBufSize, PSZ_W pucOutBuf, SHORT sListType) = 0;
virtual USHORT CPP_AsdUpdTime(HDCB hDCB, PLONG plUpdTime) = 0;
virtual USHORT CPP_AsdLoadDictProperties(PSZ pszPropFile, PPROPDICTIONARY pDictProp) = 0;
virtual USHORT CPP_AsdResynch(HUCB hUCB, HDCB hDCB) = 0;
virtual ULONG CPP_GetCPFromDCB(HDCB hDCB) = 0;
/*! \endcond */

private:

/*! \brief "Usable"-state of the dictionary-object. */
	OtmPlugin::eUsableState usableState;

};

#endif // #ifndef _OTMDICTIONARY_H_
 
