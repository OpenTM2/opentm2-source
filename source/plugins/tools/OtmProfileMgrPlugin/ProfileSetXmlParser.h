//+----------------------------------------------------------------------------+
//|OtmProfileSetXmlParser.h     Parse profile set xml file                     |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:             Flora Lee                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:        This is module contains some functions which are used   |
//|                    during parse profile set xml file                       |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#pragma once

// the Win32 Xerces build requires the default structure packing
#include "xercesc/framework/XMLPScanToken.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/OutOfMemoryException.hpp"
#include "xercesc/util/XMLUni.hpp"
#include "xercesc/dom/DOM.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/framework/LocalFileFormatTarget.hpp"

#include "LastUsedVal.h"
#include "OtmLogWriterLoc.h"

XERCES_CPP_NAMESPACE_USE;

// error code define
#define PLUGIN_PARSER_ERR                                       10

// len define
#define COMP_SHORT_DSCP_LEN                                     256

// Backup folder name
#define BAK_FLD_NAME                                            "Bak"
#define PROFILE_BAK_NAME                                        "ProfileBak"

// Action for unknown markup: 0=Abort, 1=Skip, 2=Use default value
#define KEY_VALUE                                               "Value"
#define VALUE_ABORT                                             "Abort"
#define VALUE_SKIP                                              "Skip"
#define VALUE_USE                                               "Use"

// keyword of xml
#define KEY_PROFILE_SET                                         "profileSettings"
#define KEY_TRANS_EDITOR                                        "translationEditor"
#define KEY_WORKBENCH                                           "workbench"
#define KEY_FOLDER_LIST                                         "folderList"
#define KEY_LAST_USED_VALUES                                    "lastUsedValues"

// key for font size
#define KEY_NAME                                                "Name"
#define KEY_CAPTION                                             "Caption"
#define KEY_INDEX                                               "Index"
#define KEY_FUNC_ID                                             "FuncID"
#define KEY_FUNC_NAME                                           "FuncName"
#define KEY_UCODE                                               "UCode"
#define KEY_USTATE                                              "UState"

// key for position
#define KEY_POS_CX                                              "cx"
#define KEY_POS_CY                                              "cy"
#define KEY_POS_X                                               "x"
#define KEY_POS_Y                                               "y"
#define KEY_POS_FS                                              "fs"

// key for translation editor
// key for colors
#define KEY_FONT_TYPE                                           "FontType"
#define KEY_FONT_COLOR                                          "FontColor"
#define KEY_FOREGROUND_COLOR                                    "ForegroundColor"
#define KEY_BACKGROUND_COLOR                                    "BackgroundColor"
#define KEY_FONT_SYMB_NAME                                      "FontSymbolicName"
#define KEY_UNDERSCORE_FLAG                                     "UnderscoreFlag"
#define KEY_REVERSE_FLAG                                        "ReverseFlag"

// key for fonts
#define KEY_FONTS                                               "Fonts"
#define KEY_WINDOW                                              "Window"
#define KEY_COLORS                                              "Colors"
#define KEY_FONT                                                "Font"
#define KEY_X                                                   "X"
#define KEY_Y                                                   "Y"

// key for keys
#define KEY_KEYS                                                "Keys"
#define KEY_SPEC_CHAR                                           "SpecChar"
#define KEY_KEY                                                 "Key"
#define KEY_FUNCTION                                            "Function"
#define KEY_DISP_STR                                            "DisplayStr"

// key for system properties
#define KEY_GENERAL                                             "General"
#define KEY_MARKUP                                              "MarkupRelated"
#define KEY_IF_REPLACE_GENERIC_INLINE                           "IfReplaceGenericInline"
#define KEY_LOGO_TIME                                           "LogoTime"
#define KEY_WEB_BROWSER                                         "WebBrowser"
#define KEY_IF_USE_IELIKE_TREE_VIEW                             "IfUseIELikeTreeView" 
#define KEY_SMALL_LKUP_FUZZ_LEVEL                               "SmallLkupFuzzLevel"
#define KEY_MEDIUM_LKUP_FUZZ_LEVEL                              "MediumLkupFuzzLevel"
#define KEY_LARGE_LKUP_FUZZ_LEVEL                               "LargeLkupFuzzLevel"
#define KEY_SMALL_FUZZ_LEVEL                                    "SmallFuzzLevel"
#define KEY_MEDIUM_FUZZ_LEVEL                                   "MediumFuzzLevel"
#define KEY_LARGE_FUZZ_LEVEL                                    "LargeFuzzLevel"
#define KEY_DEF_TAR_LANG                                        "DefTarLang"
#define KEY_ENABLE_PROCESSING_IDDOC                             "EnableProcessingIDDOC"
#define KEY_DISPLAY_ENTITIES_VALUE                              "DispalyEntitiesValue"
#define KEY_ACTION_FOR_UNKNOW_MARKUP                            "ActionForUnknowMarkup"
#define KEY_OTM_WINDOW_POS                                      "OTMWindowPos"
#define KEY_LST_USD_VIEW_WIDTH                                  "LastUsedViewWidth"

// key for folder list
#define KEY_EXPORT_PATH                                         "ExportPath"
#define KEY_IMPORT_PATH                                         "ImportPath"
#define KEY_FOLDER_LIST_POS                                     "FolderListPos"
#define KEY_EXP_WITH_DICT                                       "ExportWithDict"
#define KEY_EXP_DATA_ONLY                                       "ExportDataOnly"
#define KEY_EXP_WITH_MEM                                        "ExportWithMem"
#define KEY_EXP_WITH_MEM_DATABASE                               "ExportWithMemDatabase"
#define KEY_EXP_ADD_NOTE                                        "ExportAddNote"
#define KEY_EXP_DEL_FOLDER                                      "ExportDeleteFolder"
#define KEY_IMP_WITH_DICT                                       "ImportWithDict"
#define KEY_IMP_WITH_MEM                                        "ImportWithMem"
#define KEY_EXP_ORGININATOR                                     "ExpOriginator"
#define KEY_EXP_EMAIL                                           "ExpEmail"
#define KEY_IMP_DRIVE                                           "ImpToDrive"

// key for last used values
#define KEY_GLOB_FIND_LAST_VAL                                  "GlobFindLastUsedValues"
#define KEY_SHARE_MEM_ACCESS_LAST_VAL                           "EqfSharedMemoryAccessLastUsed"
#define KEY_SHARE_MEM_CREATE_LAST_VAL                           "EqfSharedMemoryCreateLastUsed"
#define KEY_BATCH_LIST_LAST_VAL                                 "BatchListLastUsed"
#define KEY_EQF_NFLUENT_TRG                                     "EQFNFLUENTTRG"
#define KEY_EQF_LIST_LAST_VAL                                   "ListLastUsedValue"

// key for global find last used value
#define KEY_FIND_STR                                            "FindStr"
#define KEY_REPLACE_STR                                         "ReplaceStr"
#define KEY_IF_FIND_STR_IN_SRC                                  "IfFindStrInSrc"
#define KEY_IF_APPLY_BAT_LST                                    "IfApplyBatchList"
#define KEY_FIND_STR_IN_SRC                                     "FindStrInSrc"
#define KEY_WILDCHAR_FOR_SINGLE                                 "WildcharForSingle"
#define KEY_WILDCHAR_FOR_MULTI                                  "WildcharForMultiple"
#define KEY_IF_UPDATE_TM                                        "IfUpdateTM"
#define KEY_IF_CONFIRM_ON_REPLACE                               "IfConfirmOnReplace"
#define KEY_IF_FIND_TRANSABLE_ONLY                              "IfFindTansbleOnly"
#define KEY_IF_WHOLE_WORDS_ONLY                                 "IfWholeWordsOnly"
#define KEY_IF_CASE_RESPECT                                     "IfCaseRespect"
#define KEY_SEARCH_IN                                           "SearchIn"
#define KEY_IF_SHOW_SRC                                         "ShowSource"
#define KEY_IF_SHOW_TAR                                         "ShowTarget"
#define KEY_IF_RESPECT_LB                                       "IfRespectLineBreaks"
#define KEY_SHOW_BEFORE_AFTER                                   "ShowBeforeAfter"
#define KEY_GF_WIN_POS                                          "GlobalFindWinPos"
#define KEY_WEB_SEV_URL                                         "WebSevUrl"
#define KEY_USER_ID                                             "UserId"
#define KEY_PASSWORD                                            "Password"
#define KEY_DS_GENERIC_TYPE                                     "DSGenericType"
#define KEY_DS_TYPE                                             "DSType"
#define KEY_DS_SERVER                                           "DSServer"
#define KEY_DS_PORT                                             "DSPort"
#define KEY_DS_USER                                             "DSUser"
#define KEY_DS_PWD                                              "DSPassword"
#define KEY_DS_USER_LIST                                        "UserList"
#define VAL_TARGET                                              "target"
#define VAL_SOURCE                                              "source"
#define VAL_BOTH                                                "both"

// batch list
#define KEY_BATCH_ITM                                           "BatchItem"
#define KEY_FIND_IN_TARGET                                      "FindInTarget"
#define KEY_FIND_IN_SOURCE                                      "FindInSource"
#define KEY_REPLACE_IN_TARGET                                   "ReplaceInTarget"

// key for NFLUENT
#define KEY_MTLOGGING                                           "MTLOGGING"
#define KEY_NOMATCH                                             "NOMATCH"
#define KEY_ALLSEGS                                             "ALLSEGS"
#define KEY_ALLWMATCH                                           "ALLWMATCH"
#define KEY_NOMATCHEXP                                          "NOMATCHEXP"
#define KEY_ALLSEGSEXP                                          "ALLSEGSEXP"
#define KEY_ALLWMATCHSOURCE                                     "ALLWMATCHSOURCE"
#define KEY_NOPROPOSAL                                          "NOPROPOSAL"
#define KEY_NOPROPOSALEXP                                       "NOPROPOSALEXP"
#define KEY_XLIFF                                               "XLIFF"
#define KEY_INCLUDEWORDCOUNT                                    "INCLUDEWORDCOUNT"
#define KEY_NOMATCH_NODUPLICATE                                 "NOMATCH_NODUPLICATE"
#define KEY_NOMATCHEXP_NODUPLICATE                              "NOMATCHEXP_NODUPLICATE"
#define KEY_ALLSEGS_NODUPLICATE                                 "ALLSEGS_NODUPLICATE"
#define KEY_ALLSEGSEXP_NODUPLICATE                              "ALLSEGSEXP_NODUPLICATE"
#define KEY_ALLWMATCH_NODUPLICATE                               "ALLWMATCH_NODUPLICATE"
#define KEY_ALLWMATCHSOURCE_NODUPLICATE                         "ALLWMATCHSOURCE_NODUPLICATE"
#define KEY_NOPROPOSAL_NODUPLICATE                              "NOPROPOSAL_NODUPLICATE"
#define KEY_NOPROPOSALEXP_NODUPLICATE                           "NOPROPOSALEXP_NODUPLICATE"

class CProfileSetXmlParser
{
private:
    char        m_strErrMsg[MAX_BUF_SIZE];

private:
    int DoImportFromRoot(DOMNode* elementRoot, POPTIONSET pOptionSet);
    int DoTransEditorImport(DOMNode* pTransEditorEle);
    int FontInfoImport(DOMNode* pFontInfoEle);
    int ColorsInfoImport(DOMNode* pColorsInfoEle);
    int ColorInfoImport(DOMNode* pColorInfoEle, PTEXTTYPETABLE pTextTable, int nColorInx);
    int KeyInfoImport(DOMNode* pKeyInfoEle);
    int DoWorkbenchImport(DOMNode* pWorkbenchEle);
    int WorkbenchGeneralImport(DOMNode* pGeneralEle, PPROPSYSTEM  & pSystemProp);
    int WorkbenchMarkupImport(DOMNode* pMarkupEle, PPROPSYSTEM  & pSystemProp);
    int DoFolderListImport(DOMNode* pFolderListEle);
    int DoLastUsedValuesImport(DOMNode* pLastValEle);
    int GlobFindLastValImp(DOMNode* pGlobFindEle);
    int SharedMemAccessLastValImp(DOMNode* pGlobFindEle);
    int SharedMemCreateLastValImp(DOMNode* pGlobFindEle);
    int BatchListLastValImp(DOMNode* pBatLstLastValEle);
    int NFluentTrgImp(DOMNode* pNFluentEle);
    int ListLastValImp(DOMNode* pLstLastValEle);
    void SubListLastValImp(DOMNode* pSubLstLastValEle, const char * strLstName);
    void WindowPosImport(xercesc::DOMNode* pDomNode, EQF_SWP * pSwp);
    int BackupProfiles();

    int DoTransEditorExport(xercesc::DOMDocument* pExportXmlDoc);
    int FontInfoExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement*  pEditorEle);
    int ColorInfoExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement*  pEditorEle);
    int KeyInfoExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement*  pEditorEle);
    int DoWorkbenchExport(xercesc::DOMDocument * pExportXmlDoc);
    int WorkbenchGeneralExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement*  pWorkbenchEle, PPROPSYSTEM  & pSystemProp);
    int WorkbenchMarkupExport(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pWorkbenchEle, PPROPSYSTEM & pSystemProp);
    int DoFolderListExport(xercesc::DOMDocument * pExportXmlDoc);
    int DoLastUsedValuesExport(xercesc::DOMDocument* pExportXmlDoc);
    int GlobFindLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle);
    int SharedMemAccessLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle);
    int SharedMemCreateLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle);
    int BatchListLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle);
    int NFluentTrgExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle);
    int ListLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLastUsedValEle);
    void SubListLastValExp(xercesc::DOMDocument* pExportXmlDoc, DOMElement* pLstLastValEle, const char * strLstName);
    int WindowPosExport(DOMElement* pElement, EQF_SWP * pSwp);

    const char * GetColorIndex(DOMNode* pColorInxEle);

public:
    CProfileSetXmlParser(void);
    ~CProfileSetXmlParser(void);
    int DoProfileExport(POPTIONSET pOptionSet);
    int DoProfileImport(POPTIONSET pOptionSet);
    const char * GetParserErrMsg();
};
