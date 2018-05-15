/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _EqfDocument_h_
#define _EqfDocument_h_

#include <string>
#include "core\PluginManager\OtmDocument.h"

class EqfDocument: public OtmDocument
/*! \brief This class implements the standard document (EQF) for OpenTM2. */

{
public:
/*! \brief Constructor */
	EqfDocument();

/*! \brief Destructor */
	~EqfDocument();

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

/*! \brief Read a document.
	Reads the associated files.
	\returns true = success, false = one or more files could not be read. */
	bool read() {return true;};

/*! \brief Save a document.
	This writes all associated files (incl. properties) to disk.
	\returns true = success, false = failed to save the document. */
	bool save() {return true;};

/*! \brief Close a document.
	This closes all associated files (incl. properties). */
	void close() {};

/*! \brief Get the document properties.
	\returns Pointer to the document properties. */
	OtmDocumentProperties* getProperties() {return 0;};

/*! \brief Get the number of segments in the document
	\returns Number of segments. */
	int getNumberOfSegments() {return 0;};

/*! \brief Get the segment with the given ID
	\returns Pointer to the segment. */
	OtmSegment* getSegment(int iID) { iID; return 0;};

/*! \brief Add a segment to the end of the list of segments
	\returns true = success, false = failed to add the segment. */
	bool appendSegment(OtmSegment* pSegment) { pSegment; return true;};

/*! \brief Update a segment
	\returns true = success, false = failed to update the segment. */
	bool updateSegment(OtmSegment* pSegment) { pSegment; return true;};

/*! \cond */
USHORT CPP_EQFBCharType(PTBDOCUMENT pDoc, PTBSEGMENT pSeg, USHORT usOffs);
PTBSEGMENT CPP_EQFBGetSegW(PTBDOCUMENT pDoc, ULONG ulSeg);
BOOL CPP_EQFBFileExists(PSZ pszFile);
BOOL CPP_EQFBDiffTag(PTBDOCUMENT pDoc, ULONG ulSeg, USHORT usOffs);
BOOL CPP_EQFBIsLFProtected(PTBSEGMENT pSeg, SHORT sOffs);
BOOL CPP_EQFBDiffProtectTag(PTBDOCUMENT pDoc, ULONG ulSeg, USHORT usOffs);
VOID CPP_EQFBReparse(PTBDOCUMENT pDoc, PTBSEGMENT pSeg, USHORT usOffs, SHORT sDiff);
SHORT CPP_EQFBLineUp(PTBDOCUMENT pDoc);
SHORT CPP_EQFBLineDown(PTBDOCUMENT pDoc);
SHORT CPP_EQFBDocLoad(PLOADSTRUCT pLoad);
PTBDOCUMENT CPP_EQFBDocDelete(PTBDOCUMENT pDoc);
SHORT CPP_EQFBDocSave(PTBDOCUMENT pDoc, PSZ pszFileName, BOOL fAskForSave);
VOID CPP_EQFBFuncOpenTRNote(PTBDOCUMENT pDoc);
VOID CPP_EQFBDocPrint(PTBDOCUMENT pDoc);
SHORT CPP_EQFBWordCntPerSeg(PVOID pVoidTable, PTOKENENTRY pTokBuf, PSZ_W pData, SHORT sLanguageId, PULONG pulResult, PULONG pulMarkUp, ULONG ulOemCP);
PTBDOCUMENT CPP_EQFBRemoveDoc(PTBDOCUMENT pDoc);
BOOL CPP_EQFBCheckNoneTag(PTBDOCUMENT pDoc, PSZ_W pString);
VOID CPP_EQFBNormSeg(PTBDOCUMENT pDoc, PSZ_W pData, PSZ_W pOutData);
USHORT CPP_EQFBPrepareFileWrite( PTBDOCUMENT pDoc, PVOID  *ppvFileWriteData, PSZ pszFileName, SHORT sLogTaskID, USHORT  usCPConversion, EQF_BOOL fAutoSave );
USHORT CPP_EQFBTerminateFileWrite( PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usRCIn );
USHORT CPP_EQFBWriteNextSegment(PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usCPConversion, ULONG ulCP, PBOOL pfDone);
BOOL CPP_EQFBOnTRNote(PTBDOCUMENT pDoc);
USHORT CPP_EQFBWriteHistLog(PSZ pszFolObjName, PSZ pszDocName, SHORT TaskId, USHORT usAddInfoLength, PVOID pvAddInfo, BOOL fMsg, HWND hwndErrMsg);
USHORT CPP_EQFBWriteHistLog2(PSZ pszFolObjName, PSZ pszDocName, SHORT TaskId, USHORT usAddInfoLength, PVOID pvAddInfo, BOOL fMsg, HWND hwndErrMsg, PSZ pszLongDocName);
void CPP_HistLogCorrectRecSizes(PHISTLOGRECORD pRecord);
SHORT CPP_EQFBHistDocSave(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID);
SHORT CPP_EQFBHistDocSaveEx(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID );
USHORT CPP_EQFBFileRead(PSZ pszFileName, PTBDOCUMENT pDoc);
USHORT CPP_EQFBFileReadExW(PSZ pszFileName, PTBDOCUMENT pDoc, LONG lFlags);
void CPP_EQFBFreeDoc(PTBDOCUMENT *ppDoc, ULONG ulOptions);
USHORT CPP_EQFBFileWrite(PSZ pszFileName, PTBDOCUMENT pDoc);
USHORT CPP_EQFBFileWriteEx(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID, USHORT usCPConversion);
PTBSEGMENT CPP_EQFBGetSeg(PTBDOCUMENT pDoc, ULONG ulSeg);
PTBSEGMENT CPP_EQFBGetFromBothTables(PTBDOCUMENT pDoc, PULONG pulStandardIndex, PULONG pulAdditionalIndex, PULONG pulLastTable);
VOID CPP_EQFBBufRemoveTRNote(PSZ_W pData, PVOID pDocTagTable, PFN pfnUserExit, PFN pfnUserExitW, ULONG ulOemCP);
SHORT CPP_EQFBAddSeg(PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg);
SHORT CPP_EQFBAddSegW(PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg);
USHORT CPP_EQFBBuildCountCheckSum(USHORT usCountFlags, USHORT usSrcWords, USHORT usTgtWords, USHORT usModWords);
VOID CPP_EQFBFillWriteAttr(PVOID pQFTagTable, PSZ pszMarkAttr, PSZ pszNoCountAttr, PSZ pszCurrentAttr, PSZ pszJoinAttr, PSZ pszNAttr, PSZ pszStatusAttr, PSZ pszCountAttr);
VOID CPP_EQFBFillWriteAttrW(PVOID pQFTagTable, PSZ_W pszMarkAttr, PSZ_W pszNoCountAttr, PSZ_W pszCurrentAttr, PSZ_W pszJoinAttr, PSZ_W pszNAttr, PSZ_W pszStatusAttr, PSZ_W pszCountAttr);
BOOL CPP_EQFBGetHexNumberW(PSZ_W pszNumber, PUSHORT pusValue);
/*! \endcond */

private:
	std::string name;
	std::string shortDesc;
	std::string longDesc;
	std::string version;
	std::string supplier;
};

#endif // #ifndef _EqfDocument_h_