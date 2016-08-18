/*! \file
	Copyright Notice:

	Copyright (C) 1990-2012, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _EqfDictionary_h_
#define _EqfDictionary_h_

#include <string>
#include "core\PluginManager\OtmDictionary.h"

class EqfDictionary: public OtmDictionary
/*! \brief This class implements the standard (EQF) dictionary for OpenTM2.
*/

{
public:
/*! \brief Constructor */
	EqfDictionary();
/*! \brief Destructor */
	~EqfDictionary();
/*! \brief Returns the name of the plugin */
	const char* getName();
/*! \brief Returns a short plugin-Description */
	const char* getShortDescription();
/*! \brief Returns a verbose plugin-Description */
	const char* getLongDescription();
/*! \brief Returns the version of the plugin */
	const char* getVersion();
/*! \brief Returns the name of the plugin-supplier */
	const char* getSupplier();

/*! \cond old wrapper stuff */
USHORT CPP_AsdInsEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PSZ_W pucData, ULONG ulLength, PULONG pulTermNo);
USHORT CPP_AsdInsEntryNonUnicode(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PUCHAR pucData, ULONG ulLength, PULONG pulTermNo);
USHORT CPP_AsdRepEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, PSZ_W pucData, ULONG ulLength, PULONG pulTermNo);
USHORT CPP_AsdDelEntry(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm);
USHORT CPP_AsdBuild(HUCB hUCB, BOOL fGuarded, PHDCB phDCB, PSZ pszDictName);
USHORT CPP_AsdTranslate(HUCB hUCB, HDCB hDCB,PUCHAR pucSegment, USHORT usOutBufSize, PUCHAR pucOutBuf, USHORT usMode);
USHORT CPP_AsdTranslateW(HUCB hUCB, HDCB hDCB, PSZ_W pucSegment, USHORT usOutBufSize, PSZ_W pucOutBuf, USHORT usMode);
USHORT CPP_AsdFndBegin(PSZ_W pucSubString, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdFndEquiv(PUCHAR pucTermIn, HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdFndEquivW(PSZ_W pucTermIn, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdFndMatch(PSZ_W pucTermIn, HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdFndNumber(ULONG ulTermNumber, HDCB hDCB, USHORT usRelocation, HUCB hUCB, PSZ_W pucTerm, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdNxtTermW(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdNxtTerm(HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdPrvTerm(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdRenumber(HDCB hDCB, HUCB hUCB);
USHORT CPP_AsdRetEntry(HDCB hDCB, HUCB hUCB, PUCHAR pucTerm, PULONG pulTermNumber, PUCHAR pucEntryData, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdRetEntryW(HDCB hDCB, HUCB hUCB, PSZ_W pucTerm, PULONG pulTermNumber, PSZ_W pucEntryData, PULONG pulDataLength, PHDCB phDCB);
USHORT CPP_AsdListIndex(HUCB hUCB, HDCB hDCB, PSZ_W pucTerm, USHORT usTermType, PSZ_W *ppucTermList, PLONG plUsed, PLONG plSize, PUSHORT pusTerms, BOOL fTermInStemForm);
USHORT CPP_AsdDelete(PSZ pszPropName);
USHORT CPP_AsdRename(PSZ pszOldPropName, PSZ pszNewPropName);
USHORT CPP_AsdRetPropPtr(HUCB hUCB, HDCB hDCB, PPROPDICTIONARY *ppProp);
USHORT CPP_AsdGetStemForm(HUCB hUCB, HDCB hDCB, PSZ_W pszInTerm, PSZ_W pszOutTerm);
USHORT CPP_AsdEntryChange(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm);
BOOL CPP_AsdIsUcbOK(HUCB hUCB);
BOOL CPP_AsdIsDcbOK(HDCB hDCB);
BOOL CPP_AsdQueryDictName(HDCB hDCB, PSZ pszDictName);
BOOL CPP_AsdQueryDictShortName(HDCB hDCB, PSZ pszDictName);
USHORT CPP_AsdRetDictList(HDCB hDCB, HDCB *ahDCB, PUSHORT pusNumOfDicts);
USHORT CPP_AsdRetUserHandle(HUCB hUCB, PUSHORT pusUserHandle);
USHORT CPP_AsdRetServiceHandle(HUCB hUCB, PUSHORT pusServiceHandle);
USHORT CPP_AsdRetMorphHandle(HDCB hDCB, PUSHORT pusMorphHandle);
USHORT CPP_AsdMergeEntry(HUCB hUCB, HDCB hDCBSource, PSZ_W pucTerm, ULONG ulDataLength, PSZ_W pucData, HDCB hDCBTarget, PUSHORT pusControlFlags);
USHORT CPP_AsdNumEntries(HDCB hDCB, PULONG pulNumber);
USHORT CPP_AsdDictVersion(HDCB hDCB, PUSHORT pusVersion);
USHORT CPP_AsdDictSetOldVersion(HDCB hDCB);
USHORT CPP_AsdLockDict(HDCB hDCB, BOOL fLock);
USHORT CPP_AsdLockEntry(HDCB hDCB, PSZ_W pszTerm, BOOL fLock);
USHORT CPP_AsdDeleteRemote(PSZ pszPropName);
USHORT CPP_AsdHandleFromDCB(HDCB hDCB, PUSHORT pusDictHandle, PUSHORT pusIndexHandle, PVOID * ppBTreeDict, PVOID * ppBTreeIndex, PLONG pLHandle);
USHORT CPP_AsdCloseOrganize(HUCB hUCB, HDCB hDCB, PSZ pszDictPath, CHAR chPrimDrive, USHORT usRc);
USHORT CPP_AsdRCHandling(USHORT usAsdRC, HDCB hDCB, PSZ pszDict, PSZ pszServer, PSZ_W pszTerm, HWND hwnd);
USHORT CPP_AsdTermList(HUCB hUCB, HDCB hDCB, PSZ_W pszTerm, USHORT usNumTerms, USHORT usAction, PSZ_W pucBuffer, USHORT usLen);
USHORT CPP_AsdWildCardList(HUCB hUCB, HDCB hDCB, PSZ_W pszStartTerm, PSZ_W pszPattern, USHORT usNumTerms, BOOL fCompound, PSZ_W pucBuffer, USHORT usLen);
USHORT CPP_AsdGetTranslation(HUCB hUCB, HDCB hDCB, PUCHAR pucDictData, USHORT usOutBufSize, PUCHAR pucOutBuf, SHORT sListType);
USHORT CPP_AsdGetTranslationW(HUCB hUCB, HDCB hDCB, PSZ_W pucDictData, USHORT usOutBufSize, PSZ_W pucOutBuf, SHORT sListType);
USHORT CPP_AsdUpdTime(HDCB hDCB, PLONG plUpdTime);
USHORT CPP_AsdLoadDictProperties(PSZ pszPropFile, PPROPDICTIONARY pDictProp);
USHORT CPP_AsdResynch(HUCB hUCB, HDCB hDCB);
ULONG CPP_GetCPFromDCB(HDCB hDCB);
/*! \endcond */

private:
	std::string name;
	std::string shortDesc;
	std::string longDesc;
	std::string version;
	std::string supplier;
};

#endif // #ifndef _EqfDictionary_h_
