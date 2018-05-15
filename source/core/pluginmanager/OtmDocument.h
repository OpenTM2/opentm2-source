/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMDOCUMENT_H_
#define _OTMDOCUMENT_H_

#ifndef CPPTEST
extern "C" {
#endif
#include "eqf.h"
#include "otmapi.h"
#include "eqftag.h"
#include "eqftp.h"
#include "eqftpi.h"
#include "eqfhlog.h"
#ifndef CPPTEST
}
#endif

class OtmSegment;
class OtmDocumentProperties;

/*! \brief Abstract base-class for document objects */
class __declspec(dllexport) OtmDocument
{

public:

/*! \brief Constructor */
	OtmDocument() {};

/*! \brief Destructor */
	virtual ~OtmDocument() {};

/*! \brief Read a document.
	Reads the associated files.
	\returns true = success, false = one or more files could not be read. */
	virtual bool read() = 0;

/*! \brief Save a document.
	This writes all associated files (incl. properties) to disk.
	\returns true = success, false = failed to save the document. */
	virtual bool save() = 0;

/*! \brief Close a document.
	This closes all associated files (incl. properties). */
	virtual void close() = 0;

/*! \brief Get the document properties.
	\returns Pointer to the document properties. */
	virtual OtmDocumentProperties* getProperties() = 0;

/*! \brief Get the number of segments in the document
	\returns Number of segments. */
	virtual int getNumberOfSegments() = 0;

/*! \brief Get the segment with the given ID
	\returns Pointer to the segment. */
	virtual OtmSegment* getSegment(int iID) = 0;

/*! \brief Add a segment to the end of the list of segments
	\returns true = success, false = failed to add the segment. */
	virtual bool appendSegment(OtmSegment* pSegment) = 0;

/*! \brief Update a segment
	\returns true = success, false = failed to update the segment. */
	virtual bool updateSegment(OtmSegment* pSegment) = 0;

/*! \cond */
	virtual USHORT CPP_EQFBCharType(PTBDOCUMENT pDoc, PTBSEGMENT pSeg, USHORT usOffs) = 0;
	virtual PTBSEGMENT CPP_EQFBGetSegW(PTBDOCUMENT pDoc, ULONG ulSeg) = 0;
	virtual BOOL CPP_EQFBFileExists(PSZ pszFile) = 0;
	virtual BOOL CPP_EQFBDiffTag(PTBDOCUMENT pDoc, ULONG ulSeg, USHORT usOffs) = 0;
	virtual BOOL CPP_EQFBIsLFProtected(PTBSEGMENT pSeg, SHORT sOffs) = 0;
	virtual BOOL CPP_EQFBDiffProtectTag(PTBDOCUMENT pDoc, ULONG ulSeg, USHORT usOffs) = 0;
	virtual VOID CPP_EQFBReparse(PTBDOCUMENT pDoc, PTBSEGMENT pSeg, USHORT usOffs, SHORT sDiff) = 0;
	virtual SHORT CPP_EQFBLineUp(PTBDOCUMENT pDoc) = 0;
	virtual SHORT CPP_EQFBLineDown(PTBDOCUMENT pDoc) = 0;
	virtual SHORT CPP_EQFBDocLoad(PLOADSTRUCT pLoad) = 0;
	virtual PTBDOCUMENT CPP_EQFBDocDelete(PTBDOCUMENT pDoc) = 0;
	virtual SHORT CPP_EQFBDocSave(PTBDOCUMENT pDoc, PSZ pszFileName, BOOL fAskForSave) = 0;
	virtual VOID CPP_EQFBFuncOpenTRNote(PTBDOCUMENT pDoc) = 0;
	virtual VOID CPP_EQFBDocPrint(PTBDOCUMENT pDoc) = 0;
	virtual SHORT CPP_EQFBWordCntPerSeg(PVOID pVoidTable, PTOKENENTRY pTokBuf, PSZ_W pData, SHORT sLanguageId, PULONG pulResult, PULONG pulMarkUp, ULONG ulOemCP) = 0;
	virtual PTBDOCUMENT CPP_EQFBRemoveDoc(PTBDOCUMENT pDoc) = 0;
	virtual BOOL CPP_EQFBCheckNoneTag(PTBDOCUMENT pDoc, PSZ_W pString) = 0;
	virtual VOID CPP_EQFBNormSeg(PTBDOCUMENT pDoc, PSZ_W pData, PSZ_W pOutData) = 0;
	virtual USHORT CPP_EQFBPrepareFileWrite( PTBDOCUMENT pDoc, PVOID  *ppvFileWriteData, PSZ pszFileName, SHORT sLogTaskID, USHORT  usCPConversion, EQF_BOOL fAutoSave ) = 0;
	virtual USHORT CPP_EQFBTerminateFileWrite( PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usRCIn ) = 0;
	virtual USHORT CPP_EQFBWriteNextSegment(PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usCPConversion, ULONG ulCP, PBOOL pfDone) = 0;
	virtual BOOL CPP_EQFBOnTRNote(PTBDOCUMENT pDoc) = 0;
	virtual USHORT CPP_EQFBWriteHistLog(PSZ pszFolObjName, PSZ pszDocName, SHORT TaskId, USHORT usAddInfoLength, PVOID pvAddInfo, BOOL fMsg, HWND hwndErrMsg) = 0;
	virtual USHORT CPP_EQFBWriteHistLog2(PSZ pszFolObjName, PSZ pszDocName, SHORT TaskId, USHORT usAddInfoLength, PVOID pvAddInfo, BOOL fMsg, HWND hwndErrMsg, PSZ pszLongDocName) = 0;
	virtual void CPP_HistLogCorrectRecSizes(PHISTLOGRECORD pRecord) = 0;
	virtual SHORT CPP_EQFBHistDocSave(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID) = 0;
	virtual SHORT CPP_EQFBHistDocSaveEx(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID ) = 0;
	virtual USHORT CPP_EQFBFileRead(PSZ pszFileName, PTBDOCUMENT pDoc) = 0;
	virtual USHORT CPP_EQFBFileReadExW(PSZ pszFileName, PTBDOCUMENT pDoc, LONG lFlags) = 0;
	virtual void CPP_EQFBFreeDoc(PTBDOCUMENT *ppDoc, ULONG ulOptions) = 0;
	virtual USHORT CPP_EQFBFileWrite(PSZ pszFileName, PTBDOCUMENT pDoc) = 0;
	virtual USHORT CPP_EQFBFileWriteEx(PSZ pszFileName, PTBDOCUMENT pDoc, SHORT sLogTaskID, USHORT usCPConversion) = 0;
	virtual PTBSEGMENT CPP_EQFBGetSeg(PTBDOCUMENT pDoc, ULONG ulSeg) = 0;
	virtual PTBSEGMENT CPP_EQFBGetFromBothTables(PTBDOCUMENT pDoc, PULONG pulStandardIndex, PULONG pulAdditionalIndex, PULONG pulLastTable) = 0;
	virtual VOID CPP_EQFBBufRemoveTRNote(PSZ_W pData, PVOID pDocTagTable, PFN pfnUserExit, PFN pfnUserExitW, ULONG ulOemCP) = 0;
	virtual SHORT CPP_EQFBAddSeg(PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg) = 0;
	virtual SHORT CPP_EQFBAddSegW(PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg) = 0;
	virtual USHORT CPP_EQFBBuildCountCheckSum(USHORT usCountFlags, USHORT usSrcWords, USHORT usTgtWords, USHORT usModWords) = 0;
	virtual VOID CPP_EQFBFillWriteAttr(PVOID pQFTagTable, PSZ pszMarkAttr, PSZ pszNoCountAttr, PSZ pszCurrentAttr, PSZ pszJoinAttr, PSZ pszNAttr, PSZ pszStatusAttr, PSZ pszCountAttr) = 0;
	virtual VOID CPP_EQFBFillWriteAttrW(PVOID pQFTagTable, PSZ_W pszMarkAttr, PSZ_W pszNoCountAttr, PSZ_W pszCurrentAttr, PSZ_W pszJoinAttr, PSZ_W pszNAttr, PSZ_W pszStatusAttr, PSZ_W pszCountAttr) = 0;
	virtual BOOL CPP_EQFBGetHexNumberW(PSZ_W pszNumber, PUSHORT pusValue) = 0;
/*! \endcond */

private:

/*! \brief "Usable"-state of the document-object. */
	OtmPlugin::eUsableState usableState;

};

#endif // #ifndef _OTMDOCUMENT_H_
 