//+----------------------------------------------------------------------------+
//|ProfileConfXmlParser.h     Parse profile set xml file                       |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:             Flora Lee                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:        This is module contains some functions which are used   |
//|                    during parse profile config xml file                    |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#pragma once

#include "xercesc/framework/XMLPScanToken.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/TransService.hpp"
#include "xercesc/dom/DOM.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/framework/LocalFileFormatTarget.hpp"

#include "OtmProfileMgrComm.h"
#include "OtmLogWriterLoc.h"
#include "ProfileSetXmlParser.h"

XERCES_CPP_NAMESPACE_USE;

#define KEY_PROFILE_CONFIG                                           "ProfileConfig"

#define KEY_MODE                                                     "Mode"
#define KEY_TAR_FILE                                                 "TargetFile"
#define KEY_DIRS                                                     "Dirtories"
#define KEY_DIR                                                      "Dir"
#define KEY_NAMES                                                    "Names"
#define KEY_NAME                                                     "Name"
#define KEY_MAX_HIST_CNT                                             "MaxHistCnt"
#define KEY_CHK_ALL                                                  "CheckAll"
#define KEY_CHK_TRANS_EDITOR                                         "CheckTranslationEditor"
#define KEY_CHK_WORKBENCH                                            "CheckWorkbench"
#define KEY_CHK_FOLDER_LIST                                          "CheckFolderList"
#define KEY_CHK_LAST_USED_VAL                                        "CheckLastUsedVal"
#define KEY_IF_ENCRYPT                                               "IfEncrypt"
#define KEY_IF_KEEP_ORI_FILE                                         "IfKeepOriFile"

#define MODE_EXPORT                                                  "Export"
#define MODE_IMPORT                                                  "Import"

class CProfileConfXmlParser
{
private:
    char        m_strErrMsg[MAX_BUF_SIZE];
    char        m_strConfPath[MAX_PATH];
    LogWriter   m_logProfileMgr;

private:
    int OtmParseFromRoot(DOMNode* elementConfRoot, POPTIONSET pSetOption);

public:
    int LoadProfileSetConfig(POPTIONSET pSetOption);
    int SaveProfileSetConfig(POPTIONSET pSetOption);

public:
    CProfileConfXmlParser(const char * strStartDir = NULL);
    ~CProfileConfXmlParser(void);
};

