//+----------------------------------------------------------------------------+
//|OtmComm.CPP     OTM  Plugin Manager Parser function                         |
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
//|                    during plugin manager parser                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "OtmComm.h"

//static LogWriter m_gLogComm;
//static BOOL m_bLogOpened = FALSE;

int OtmUnCompress(const char * strSrcDir, PCOTMCOPIES pDefPathCopies, PCOTMCOPIES pOtmCopies, const char * strDefPath, POTMGRPSTING pstrFiles)
{
    // define variable
    int nRC = UNZ_ERRNO;

    if (NULL == pstrFiles)
    {
        return nRC;
    }

    unz_global_info  unzGlobalInfo;
    unz_file_info    unzFileInfo;

    // open unzip file
    unzFile unzipFile = unzOpen(strSrcDir);
    if (NULL == unzipFile)
    {
        nRC = ERROR_OPEN_UNZIP_FILE_A;
        return nRC;
    }

    // get the info of unzip file
    nRC = unzGetGlobalInfo(unzipFile, &unzGlobalInfo);
    if (UNZ_OK != nRC)
    {
        nRC = ERROR_UNZIP_FILE_A;
        return nRC;
    }

    // enum the first file of unzip file
    nRC = unzGoToFirstFile(unzipFile);
    if (UNZ_OK != nRC)
    {
        nRC = ERROR_UNZIP_FILE_A;
        return nRC;
    }

    for (int iInx = 0; iInx < (int) unzGlobalInfo.number_entry; iInx++)
    {
        char strFileName[MAX_PATH];
        char strEF[MAX_BUF_SIZE];
        char strComment[MAX_BUF_SIZE];
        memset(strFileName,  0x00, sizeof(strFileName));
        memset(strEF,        0x00, sizeof(strEF));
        memset(strComment,   0x00, sizeof(strComment));

        // get the info of unzip file
        nRC = unzGetCurrentFileInfo(unzipFile,       &unzFileInfo,
                                    strFileName,     sizeof(strFileName),
                                    (void*)strEF,    sizeof(strEF),
                                    strComment,      sizeof(strComment));

        if (UNZ_OK != nRC)
        {
            nRC = ERROR_UNZIP_FILE_A;
            break;
        }

        // open the unzip file
        char strDestFile[MAX_PATH];
        memset(strDestFile, 0x00, sizeof(strDestFile));
        GetRealDestDir(strFileName, pDefPathCopies, pOtmCopies, strDefPath, strDestFile);
        nRC = OtmExtractOneFile(unzipFile, strDestFile);
        if (UNZ_OK != nRC)
        {
            break;
        }
        pstrFiles->push_back(strDestFile);

        // enum next unzip file
        nRC = unzGoToNextFile(unzipFile);
        if (UNZ_END_OF_LIST_OF_FILE == nRC)
        {
            nRC = UNZ_OK;
            break;
        }
        else if (UNZ_OK != nRC)
        {
            nRC = ERROR_UNZIP_FILE_A;
            break;
        }
    }

    // close the unzip file
    if (UNZ_OK == nRC)
    {
        nRC = unzClose(unzipFile);
        if (UNZ_OK != nRC)
        {
            nRC = ERROR_UNZIP_FILE_A;
        }
    }
    return nRC;
}

int OtmExtractOneFile(unzFile unzipFile, char * strDestFile)
{
    // define variable
    int nRC = UNZ_OK;

    nRC = unzOpenCurrentFile(unzipFile);
    if (UNZ_OK != nRC)
    {
        nRC = ERROR_UNZIP_FILE_A;
        return nRC;
    }

    // check whether need to create folder
//    m_gLogComm.writef("create folder=%s", strDestFile);
    nRC = OtmCreateFolder(strDestFile);
    // unzip file
    if (IS_FOLDER == nRC)
    {
        nRC = UNZ_OK;
        return nRC;
    }

//    m_gLogComm.writef("create file=%s", strDestFile);
    FILE * fileUnzip = fopen(strDestFile, "wb");
    if (NULL == fileUnzip)
    {
        nRC = ERROR_UNZIP_FILE_A;
        return nRC;
    }

    // read the content of the unzip file and write into the new file
    wchar_t wstrBuf[MAX_BUF_SIZE];
    memset(wstrBuf, 0x00, sizeof(wstrBuf));
    int unzResult;
    while (UNZ_EOF != (unzResult = unzReadCurrentFile(unzipFile, wstrBuf, sizeof(wstrBuf))))
    {
        int nWritten = fwrite(wstrBuf, 1, unzResult, fileUnzip);
        if (nWritten != unzResult)
        {
            nRC = ERROR_WRITE_UNZIP_FILE_A;
            unzCloseCurrentFile(unzipFile);
            break;
        }
    }

    fclose(fileUnzip);
    // close the current unzip file handle
    if (UNZ_OK == nRC)
    {
        nRC = unzCloseCurrentFile(unzipFile);
        if (UNZ_OK != nRC)
        {
            nRC = ERROR_UNZIP_FILE_A;
        }
    }

    return nRC;
}

int OtmCreateFolder(char * strDestFile)
{
    int nRC = NO_ERROR;

    if ((NULL == strDestFile) || (0 == strlen(strDestFile)))
    {
        nRC = ERROR_OTM_CREATE_FOLDER_A;
        return nRC;
    }

    OtmRemoveExtraSlash(strDestFile);

    int nLen = strlen(strDestFile);
    if ('/' == strDestFile[nLen-1])
    {
        nRC = IS_FOLDER;
    }
    else if ('\\' == strDestFile[nLen-1])
    {
        nRC = IS_FOLDER;
    }

    OtmConvertToBackSlash(strDestFile);

    // create sub folder if necessary
    // 1. find "\"
    string strCheck(strDestFile);
    unsigned nStart = 0;
    unsigned nPos = strCheck.find("\\", nStart);
    while (string::npos != nPos)
    {
        string strFolder = strCheck.substr(0, nPos);

        nStart = nPos + 1;
        // check whehter the second character is still "\\"
        nPos = strCheck.find("\\", nStart);
        // if folder already exists, continue
        if (-1 != access(strFolder.c_str(), 0))
        {
            continue;
        }

        // otherwise create the folder
        if (!CreateDirectory(strFolder.c_str(), NULL))
        {
            nRC = ERROR_OTM_CREATE_FOLDER_A;
        }
    }

    // 2. find "/"
    nPos = strCheck.find("/", nStart);
    while (string::npos != nPos)
    {
        string strFolder = strCheck.substr(0, nPos);

        nStart = nPos + 1;
        // check whehter the second character is still "\\"
        nPos = strCheck.find("/", nStart);
        // if folder already exists, continue
        if (OTM_NOT_FOUND != access(strFolder.c_str(), 0))
        {
            continue;
        }

        // otherwise create the folder
        if (!CreateDirectory(strFolder.c_str(), NULL))
        {
            nRC = ERROR_OTM_CREATE_FOLDER_A;
        }
    }

    return nRC;
}

void OtmConvertToBackSlash(char * strFileName)
{
    if (NULL == strFileName)
    {
        return;
    }

    // replace all '/' to '\\'
    for (int iInx = 0; iInx < (int) strlen(strFileName); iInx++)
    {
        if ('/' == strFileName[iInx])
        {
            strFileName[iInx] = '\\';
        }
    }

    OtmRemoveExtraSlash(strFileName);
}

void GetRealDestDir(const char * strFileFullName, PCOTMCOPIES pDefPathCopies, PCOTMCOPIES pOtmCopies, const char * strDefPath, char * strDestFile)
{
    if ((NULL == strFileFullName) || (0 == strlen(strFileFullName)))
    {
        return;
    }

    char strFilePath[MAX_BUF_SIZE];
    char strFileName[MAX_BUF_SIZE];

    memset(strFilePath, 0x00, sizeof(strFilePath));
    memset(strFileName, 0x00, sizeof(strFileName));

    OtmSplitPath(strFileFullName, strFilePath, strFileName);

    //m_bLogOpened = FALSE;
    //if (!m_gLogComm.isOpen())
    //{
    //    m_gLogComm.open(LOG_NAME_OTM_COMM);
    //    m_bLogOpened = TRUE;
    //}

    BOOL bBreak = FALSE;
    for (int iInx = 0; (pOtmCopies != NULL) && (iInx < (int) pOtmCopies->size()); iInx++)
    {
        // if the file is copied, just break
        if (!OtmComparePath(strFileFullName, (*pOtmCopies)[iInx].strFrom))
        {
            bBreak = TRUE;
        }

        // 1. replace the default path
        char strNewTo[MAX_PATH];
        memset(strNewTo, 0x00, sizeof(strNewTo));

        for (int jInx = 0; (pDefPathCopies != NULL) && (jInx < (int) pDefPathCopies->size()); jInx++)
        {
            char strTemp[MAX_PATH];
            memset(strTemp, 0x00, sizeof(strTemp));

            if (0 == jInx)
            {
                sprintf(strTemp, "%s", (*pOtmCopies)[iInx].strTo);
            }
            else
            {
                sprintf(strTemp, "%s", strNewTo);
            }
            ReplaceDefaultDir(strTemp, (*pDefPathCopies)[jInx].strFrom, (*pDefPathCopies)[jInx].strTo, strNewTo);
        }

        // 2. replace the original path
        char strNewTo2[MAX_PATH];
        memset(strNewTo2, 0x00, sizeof(strNewTo2));

        ReplaceOriginalDir(strNewTo, strFilePath, strNewTo2);

        // 3. Join the path
        OtmJointPath(strDestFile, strNewTo2, strFileName);

        if (bBreak)
        {
            break;
        }
    }

    if ((NULL == strDestFile) || (0 == strlen(strDestFile)))
    {
        sprintf(strDestFile, "%s\\%s", strDefPath, strFileName);
    }

    /*if (m_bLogOpened)
    {
        m_gLogComm.close();
    }*/
}

BOOL ReplaceDefaultDir(const char * strOriPath, const char * strDefPath, const char * strReplacePath, char * strNewPath)
{
    BOOL bReplaced = FALSE;
    // first check strTo
    if ((NULL == strOriPath) || (0 == strlen(strOriPath)))
    {
        strncpy(strNewPath, strReplacePath, strlen(strReplacePath));
        return bReplaced;
    }
    else if ((NULL == strReplacePath) || (0 == strlen(strReplacePath)))
    {
        strncpy(strNewPath, strOriPath, strlen(strOriPath));
        return bReplaced;
    }

    // then locate the position of default dir
    string strTarget(strOriPath);
    bReplaced  = OtmStringReplace(strTarget, strDefPath, strReplacePath);
    strcpy(strNewPath, strTarget.c_str());

    return bReplaced;
}

BOOL ReplaceOriginalDir(const char * strOriPath, const char * strReplacePath, char * strNewPath)
{
    BOOL bReplaced = FALSE;
    // first check the parameter
    if ((NULL == strOriPath) || (0 == strlen(strOriPath)))
    {
        strncpy(strNewPath, strReplacePath, strlen(strReplacePath));
        return bReplaced;
    }
    else if ((NULL == strReplacePath) || (0 == strlen(strReplacePath)))
    {
        strncpy(strNewPath, strOriPath, strlen(strOriPath));
        return bReplaced;
    }

    // then locate the position of default dir
    string strTarget(strOriPath);
    bReplaced = OtmStringReplace(strTarget, ORIGINAL_PATH_KEY, strReplacePath);

    char strNewReplacePath[MAX_PATH];
    memset(strNewReplacePath, 0x00, sizeof(strNewReplacePath));
    if (!bReplaced)
    {
        OtmMergePath(strTarget.c_str(), strReplacePath, strNewReplacePath);
        strcpy(strNewPath, strNewReplacePath);
    }
    else
    {
        strcpy(strNewPath, strTarget.c_str());
    }

    return bReplaced;
}

void OtmSplitPath(const char * strFullName, char * strPath, char * strName)
{
    if ((NULL == strFullName) || (strlen(strFullName) == 0))
    {
        return;
    }
    string strFullNameStr(strFullName);

    string::size_type nPos = strFullNameStr.rfind("/");
    if (string::npos != nPos)
    {
        nPos++;
        strcpy(strPath, strFullNameStr.substr(0, nPos).c_str());
        if (nPos < strFullNameStr.length())
        {
            strcpy(strName, strFullNameStr.substr(nPos).c_str());
        }
        return;
    }

    nPos = strFullNameStr.rfind("\\");
    if (string::npos != nPos)
    {
        nPos++;
        strcpy(strPath, strFullNameStr.substr(0, nPos).c_str());
        if (nPos < strFullNameStr.length())
        {
            strcpy(strName, strFullNameStr.substr(nPos).c_str());
        }
        return;
    }

    strcpy(strName, strFullName);
}

long GetFileSize(const char * strFile)
{
    // obtain file size:
    long lSize = 0;

    FILE * fileObj = fopen(strFile, "rb");
    if (!fileObj)
    {
        return lSize;
    }

    fseek(fileObj , 0, SEEK_END);
    lSize = ftell(fileObj);
    fclose(fileObj);

    return lSize;
}

void OtmGetKeyValue(const char * strParam, char * strKey, char * strVal, char cBreak, BOOL bKeepBreak)
{
    if ((NULL == strParam) || (strlen(strParam) == 0))
    {
        return;
    }

    int nLen = (int) strlen(strParam);
    int nKeyInx = 0;
    int nValInx = 0;

    BOOL bValStart = FALSE;
    for (int iInx = 0; iInx <= nLen; iInx++)
    {
        if ((cBreak == strParam[iInx]) && !bValStart)
        {
            bValStart = TRUE;
            if (bKeepBreak)
            {
                strKey[nKeyInx] = strParam[iInx];
                nKeyInx++;
            }
            continue;
        }
        else if (!bValStart)
        {
            strKey[nKeyInx] = strParam[iInx];
            nKeyInx++;
            continue;
        }
        else
        {
            strVal[nValInx] = strParam[iInx];
            nValInx++;
            continue;
        }
    }

    strKey[nKeyInx] = '\0';
    strVal[nValInx] = '\0';
}

BOOL OtmStringReplace(string & strTarget, const string & strOld, const string & strNew)
{
    BOOL bReplaced = FALSE;
    string::size_type nPos = 0;
    while((nPos = strTarget.find(strOld, nPos)) != string::npos)
    {
        bReplaced = TRUE;
        strTarget.replace(nPos, strOld.length(), strNew);
        nPos += strNew.length();
    }
    return bReplaced;
}

void OtmJointPath(char * strDestPath, const char * strPath1, const char * strPath2)
{
    // remove the last "\" of strPath1
    if ((NULL == strPath1) || (strlen(strPath1) == 0))
    {
        return;
    }

    if ((NULL == strPath2) || (strlen(strPath2) == 0))
    {
        strcpy(strDestPath, strPath1);
        return;
    }

    char strTempPath1[MAX_PATH];
    memset(strTempPath1, 0x00, sizeof(strTempPath1));
    strcpy(strTempPath1, strPath1);

    int nLastPos = strlen(strTempPath1) - 1;
    if ('\\' == strTempPath1[nLastPos])
    {
        strTempPath1[nLastPos] = '\0';
    }

    if ('\\' == strPath2[0])
    {
        sprintf(strDestPath, "%s%s", strTempPath1, strPath2);
    }
    else
    {
        sprintf(strDestPath, "%s\\%s", strTempPath1, strPath2);
    }
}

void OtmRemoveExtraSlash(char * strPath)
{
    if ((NULL == strPath) || (strlen(strPath) == 0))
    {
        return;
    }

    char strTemp[MAX_PATH];
    memset(strTemp, 0x00, sizeof(strTemp));

    int iInx, nLen, iRealInx;
    nLen = strlen(strPath);
    iRealInx = 0;
    for (iInx = 0; iInx < nLen; iInx++)
    {
        strTemp[iRealInx] = strPath[iInx];
        iRealInx++;
        while ('\\' == strPath[iInx])
        {
            if (((iInx + 1) < nLen) && ('\\' == strPath[iInx+1]))
            {
                if (iInx == (nLen - 1))
                {
                    strTemp[iInx] = '\0';;
                }
                iInx++;
            }
            else
            {
                break;
            }
        }
    }
    if (strTemp[iInx] != '\0')
    {
        strTemp[iInx] = '\0';
    }
    strcpy(strPath, strTemp);
}

int OtmComparePath(const char * strPath1, const char * strPath2)
{
    char strTempPath1[MAX_PATH];
    char strTempPath2[MAX_PATH];

    memset(strTempPath1, 0x00, sizeof(strTempPath1));
    memset(strTempPath2, 0x00, sizeof(strTempPath2));

    strcpy(strTempPath1, strPath1);
    strcpy(strTempPath2, strPath2);

    OtmConvertToBackSlash(strTempPath1);
    OtmConvertToBackSlash(strTempPath2);

    OtmRemoveExtraSlash(strTempPath1);
    OtmRemoveExtraSlash(strTempPath2);

    return stricmp(strTempPath1, strTempPath2);
}

int OtmIsDirectory(const char * strPath)
{
    struct _stat statBuf;
    // Remove the last backflash
    char strTempPath[MAX_PATH];
    memset(strTempPath, 0x00, sizeof(strTempPath));

    int nRC = UNDEFINED_ERROR;
    if ((NULL == strPath) || (0 >= strlen(strPath)))
    {
        return nRC;
    }

    strcpy(strTempPath, strPath);
    if ('\\' == strTempPath[strlen(strTempPath)-1])
    {
        strTempPath[strlen(strTempPath)-1] = '\0';
    }

    nRC = _stat(strTempPath, &statBuf);
    if (nRC == 0)
    {
        if (statBuf.st_mode & _S_IFDIR)
        {
            nRC = IS_FOLDER;
        }
        else
        {
            nRC = IS_FILE;
        }
    }
    else
    {
        if (errno == ENOENT)
        {
            nRC = NOT_EXIST;
        }
        else
        {
            nRC = UNDEFINED_ERROR;
        }
    }

    return nRC;
}

int OtmDeleteAllInDir(const char * strTargetDir)
{
    int nRC = NO_ERROR;

    // delete all in the target directory
    WIN32_FIND_DATA otmFindData;
    ZeroMemory(&otmFindData, sizeof(WIN32_FIND_DATA));

    HANDLE hFindHandle;
    BOOL bFinished = FALSE;

    char strSearchPath[MAX_PATH];
    char strTmpTgtDir[MAX_PATH];
    memset(strSearchPath, 0x00, sizeof(strSearchPath));
    memset(strTmpTgtDir,  0x00, sizeof(strTmpTgtDir));

    strcpy(strSearchPath, strTargetDir);
    if (strSearchPath[strlen(strSearchPath) - 1] != '\\')
    {
       strcat(strSearchPath, "\\");
    }
    strcpy(strTmpTgtDir, strSearchPath);
    strcat(strSearchPath, "*");

    hFindHandle = FindFirstFile((LPCTSTR)strSearchPath, &otmFindData);
    if (INVALID_HANDLE_VALUE == hFindHandle)
    {
        bFinished = TRUE;           // this is nothing in the target directory
    }

    while (!bFinished)
    {
        sprintf(strSearchPath, "%s%s", strTmpTgtDir, otmFindData.cFileName);
        // if directory is searched
        if (otmFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (otmFindData.cFileName[0] != '.')
            {
                OtmDeleteAllInDir(strSearchPath);
            }
        }
        else
        {
            // delete file
            remove(strSearchPath);
        }

        if (!FindNextFile(hFindHandle, &otmFindData))
        {
            bFinished = TRUE;
        }
    }
    FindClose(hFindHandle);
    // delete empty directory
    RemoveDirectory(strTargetDir);
    return nRC;
}

void OtmMergePath(const char * strOriPath, const char * strReplacePath, char * strMergedPath)
{
    /*m_bLogOpened = FALSE;
    if (!m_gLogComm.isOpen())
    {
        m_gLogComm.open("PluginManagerGUIComm");
        m_bLogOpened = TRUE;
    }*/

    // 1. parameter check
    if ((NULL == strOriPath) || (0 == strlen(strOriPath)) ||
        (NULL == strReplacePath) || (0 == strlen(strReplacePath)))
    {
        return;
    }

    // 2. find the place to merge
    OTMGRPSTING vecOriSegments;
    OtmSplitPathToParts(strOriPath, &vecOriSegments);

    OTMGRPSTING vecRepaceSegments;
    OtmSplitPathToParts(strReplacePath, &vecRepaceSegments);

    int nPos = OtmFindReplacePos(&vecOriSegments, &vecRepaceSegments);

    // 3. do the merge
    int nReplaceLen = (int) vecRepaceSegments.size();
    if (OTM_NOT_FOUND != nPos)
    {
        nPos++;
        if (nPos < nReplaceLen)
        {
            // need merge
            sprintf(strMergedPath, "%s\\%s", strOriPath, vecRepaceSegments[nPos].c_str());
            nPos++;
            for (; nPos < nReplaceLen; nPos++)
            {
                sprintf(strMergedPath, "%s\\%s", strMergedPath, vecRepaceSegments[nPos].c_str());
            }
        }
    }

    // 4. extra check
    if ((NULL == strMergedPath) || (strlen(strMergedPath) == 0))
    {
        strcpy(strMergedPath, strOriPath);
//        m_gLogComm.writef("merged path=%s", strMergedPath);
        return;
    }

    int nMergedLen    = strlen(strMergedPath);
    int nReplacedLen  = strlen(strReplacePath);
//    m_gLogComm.writef("final check=%s,%s", strMergedPath, strReplacePath);
    if (((strMergedPath[nMergedLen-1] != '\\')    || (strMergedPath[nMergedLen-1] != '/')) && 
        ((strReplacePath[nReplacedLen-1] == '\\') || (strReplacePath[nReplacedLen-1] == '/')))
    {
        sprintf(strMergedPath, "%s%c", strMergedPath, strReplacePath[nReplacedLen-1]);
    }
    /*m_gLogComm.writef("merged path=%s", strMergedPath);

    if (m_bLogOpened)
    {
        m_gLogComm.close();
    }*/
}

void OtmSplitPathToParts(const char * strPath, POTMGRPSTING pvecSegments)
{
    if ((NULL == strPath) || (0 == strlen(strPath)))
    {
        return;
    }

    string strTempPath(strPath);
//    m_gLogComm.writef("original=%s", strTempPath.c_str());
    size_t nPos = strTempPath.find("\\");
    if (string::npos == nPos)
    {
        nPos = strTempPath.find("/");
    }

    string strSegment;
    size_t nStart = 0;
    while (string::npos != nPos)
    {
        strSegment =strTempPath.substr(nStart, (nPos-nStart));
//        m_gLogComm.writef("segment=%s", strSegment.c_str());
        pvecSegments->push_back(strSegment.c_str());
        nPos++;
        nStart = nPos;
        if (nPos == strTempPath.length())
        {
            break;
        }
        nPos = strTempPath.find("\\", nPos);
        if (string::npos == nPos)
        {
            nPos = strTempPath.find("/", nPos);
        }
    }

    if (nStart < strTempPath.length())
    {
        strSegment =strTempPath.substr(nStart);
//        m_gLogComm.writef("segment=%s", strSegment.c_str());
        pvecSegments->push_back(strSegment.c_str());
    }
}

int OtmFindReplacePos(POTMGRPSTING pvecOriSegments, POTMGRPSTING pvecReplaceSegments)
{
    /*m_bLogOpened = FALSE;
    if (!m_gLogComm.isOpen())
    {
        m_gLogComm.open("PluginManagerGUIComm");
        m_bLogOpened = TRUE;
    }*/

    int nPos = OTM_NOT_FOUND;
    int nOriLen     = (int) (*pvecOriSegments).size();
    int nReplaceLen = (int) (*pvecReplaceSegments).size();

    int iInx;
    for (iInx = 0; iInx < nOriLen; iInx++)
    {
//        m_gLogComm.writef("Compare=%s,%s", (*pvecOriSegments)[iInx].c_str(), (*pvecReplaceSegments)[0].c_str());
        if (!stricmp((*pvecOriSegments)[iInx].c_str(), (*pvecReplaceSegments)[0].c_str()))
        {
            nPos = 0;
            break;
        }
    }

    if (iInx == (nOriLen - 1))
    {
        return nPos;
    }

    for (; iInx < nOriLen; iInx++)
    {
//        m_gLogComm.writef("Compare2=%s,%s", (*pvecOriSegments)[iInx].c_str(), (*pvecReplaceSegments)[nPos].c_str());
        if (!stricmp((*pvecOriSegments)[iInx].c_str(), (*pvecReplaceSegments)[nPos].c_str()))
        {
            nPos++;
        }
        else
        {
            nPos = OTM_NOT_FOUND;
            break;
        }
//        m_gLogComm.writef("Compare2=%s,%s,%d,%d", (*pvecOriSegments)[iInx].c_str(), (*pvecReplaceSegments)[nPos-1].c_str(), nPos, nReplaceLen);

        if (nPos >= nReplaceLen)
        {
            nPos = OTM_NOT_FOUND;
            break;
        }
    }

    /*if (m_bLogOpened)
    {
        m_gLogComm.close();
    }*/

    return nPos;
}

/*
int OtmExecuteCommand(char * strCmd, char * strOutput, BOOL bNeedWait)
{
    DWORD nRC = NO_ERROR;

    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    if ((strOutput != NULL) && (strlen(strOutput) != 0))
    {
        SECURITY_ATTRIBUTES StdErrSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
        si.hStdOutput = CreateFile(strOutput, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, 
            &StdErrSA, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;
    } else {
        si.hStdOutput = NULL;
        si.dwFlags = STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;
    }

    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, strCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        if (bNeedWait)
        {
            WaitForSingleObject(pi.hProcess, INFINITE);

            if (!GetExitCodeProcess(pi.hProcess, &nRC))
            {
                nRC = GetLastError();
            }
        }
    }
    else
    {
        nRC = GetLastError();
    }

    if ((strOutput != NULL) && (strlen(strOutput) != 0)) {
        CloseHandle(si.hStdOutput);
    }

    return nRC;
}
*/

int OtmExecuteCommand(char * strCmd, char * strOutput, BOOL bNeedWait)
{
    DWORD nRC = NO_ERROR;

    // if requires elevation, use ShellExecute instead of
    SHELLEXECUTEINFO otmShellInfo;

    memset(&otmShellInfo, 0x00, sizeof(otmShellInfo));
    otmShellInfo.cbSize = sizeof(otmShellInfo);
    otmShellInfo.hwnd = NULL;
    otmShellInfo.lpVerb = VERB_OPEN;
    otmShellInfo.lpFile = strCmd;
    otmShellInfo.nShow = SW_SHOWNORMAL;
    otmShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;

    if (ShellExecuteEx(&otmShellInfo))
    {
        if (bNeedWait)
        {
            WaitForSingleObject(otmShellInfo.hProcess, INFINITE);

            if (!GetExitCodeProcess(otmShellInfo.hProcess, &nRC))
            {
                nRC = GetLastError();
            }
        }
    }
    else
    {
        nRC = GetLastError();
    }

    return nRC;
}

BOOL IsProgramRunning(const char * strProgram)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 ProcessEntry;
        BOOL fMore = FALSE;

        ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
        fMore = Process32First(hSnapshot, &ProcessEntry);
        while (fMore)
        {
            if (stricmp(ProcessEntry.szExeFile, strProgram) == 0)
            {
                return(TRUE);
            }
            fMore = Process32Next(hSnapshot, &ProcessEntry);
        }
        CloseHandle(hSnapshot);
    }

    return(FALSE);
}

void RemoveDoubleQuotationMark(char * strParam)
{
    char strTemp[MAX_BUF_SIZE];
    memset(strTemp, 0x00, sizeof(strTemp));

    if ((NULL == strParam) || (strlen(strParam) == 0))
    {
        return;
    }

    if ('\"' != strParam[0])
    {
        return;
    }

    int iInx, jInx;
    jInx = 0;
    for (iInx = 1; iInx < ((int)strlen(strParam) - 1); iInx++)
    {
        strTemp[jInx] = strParam[iInx];
        jInx++;
    }

   if ('\"' == strParam[iInx])
   {
       strTemp[jInx] = '\0';
   }
   else
   {
       strTemp[jInx] = strParam[iInx];
       jInx++;
       strTemp[jInx] = '\0';
   }

   strcpy(strParam, strTemp);
}

void OtmCenterWindow(HWND hWnd)
{
    HWND hParentOrOwner;
    RECT rcParent, rcWnd;
    int x, y;
    if ((hParentOrOwner = GetParent(hWnd)) == NULL)
    {
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcParent, 0);
    }
    else
    {
        GetWindowRect(hParentOrOwner, &rcParent);
    }
    GetWindowRect(hWnd, &rcWnd);
    x = ((rcParent.right-rcParent.left) - (rcWnd.right-rcWnd.left)) / 2 + rcParent.left;
    y = ((rcParent.bottom-rcParent.top) - (rcWnd.bottom-rcWnd.top)) / 2 + rcParent.top;
    SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

char * OtmCtime(const time_t * timeValue)
{
    char * strTime = asctime(gmtime(timeValue));
    /*int nLen = strlen(strTime) - 2;
    strTime[nLen] = '\0';*/
    return strTime;
}

int IsNewVersion(const char * strVerOld, const char * strVerNew)
{
    BOOL bNew = VER_OLD_VAL;

    if (NULL == strVerOld)
    {
        bNew = VER_NEW_VAL;
        return bNew;
    }
    else if (NULL == strVerNew)
    {
        bNew = VER_OLD_VAL;
        return bNew;
    }

    OTMGRPSTING vecOldSegments;
    OTMGRPSTING vecNewSegments;

    OtmSplitDotToParts(strVerOld, &vecOldSegments);
    OtmSplitDotToParts(strVerNew, &vecNewSegments);

    int iNewInx = 0;
    for (int iOldInx = 0; iOldInx < (int) vecOldSegments.size(); iOldInx++)
    {
        int nOld = atoi(vecOldSegments[iOldInx].c_str());
        int nNew = atoi(vecNewSegments[iNewInx].c_str());
        if (nOld < nNew)
        {
            bNew = VER_NEW_VAL;
            return bNew;
        }
        else if (nOld == nNew)
        {
            const char * strOld = vecOldSegments[iOldInx].c_str();
            const char * strNew = vecNewSegments[iNewInx].c_str();
            if (stricmp(strOld, strNew) < 0)
            {
                bNew = VER_NEW_VAL;
                return bNew;
            }
            else if (stricmp(strOld, strNew) > 0)
            {
                bNew = VER_OLD_VAL;
                return bNew;
            }
            else
            {
                bNew = VER_SAM_VAL;
                iNewInx++;
                continue;
            }
        }
        else
        {
            bNew = VER_OLD_VAL;
            return bNew;
        }
    }

    while (iNewInx < (int) vecNewSegments.size())
    {
        if (stricmp(vecNewSegments[iNewInx].c_str(), "0"))
        {
            bNew = VER_NEW_VAL;
            return bNew;
        }

        iNewInx++;
    }

    return bNew;
}

int GetDateFromTime(int nTimeVal)
{
    int nDate;
    if (nTimeVal > MAX_DATE_VAL)
    {
        // if the value is large than max date, it means it not date value but time_t value
        // just compare date, so convert to tm structure first, as we just need compare date
        time_t timeVal(nTimeVal);
        struct tm * tmVal = localtime(&timeVal);

        // convert to int date value
        char strDate[MAX_DATE_LEN];
        memset(strDate, 0x00, sizeof(strDate));
        sprintf(strDate, DATE_FORMAT, (tmVal->tm_year + 1900), (tmVal->tm_mon + 1), tmVal->tm_mday);

        nDate = atoi(strDate);
    }
    else
    {
        nDate = nTimeVal;
    }

    return nDate;
}

void OtmSplitDotToParts(const char * strToSplit, POTMGRPSTING pvecSegments)
{
    if ((NULL == strToSplit) || (0 == strlen(strToSplit)))
    {
        return;
    }

    string strTempToSplit(strToSplit);
    int nInx = 0;

    // set the start index
    if ('v' == strToSplit[0])
    {
        nInx++;
    }
    else if ('V' == strToSplit[0])
    {
        nInx++;
    }

    int nLen = (int) strlen(strToSplit);
    int nStart = nInx;
    string strSegment;

    for (; nInx < nLen; nInx++)
    {
        // if not digit or alpha, it means it is just a delimiter
        if (!isdigit(strToSplit[nInx]) && !isalpha(strToSplit[nInx]))
        {
            // add content before the delimiter
            strSegment = strTempToSplit.substr(nStart, (nInx - nStart));
            pvecSegments->push_back(strSegment.c_str());

            // add the delimiter
            strSegment = strTempToSplit.substr(nInx, 1);
            pvecSegments->push_back(strSegment.c_str());

            nStart = nInx + 1;
        }
        else if (isdigit(strToSplit[nInx]))
        {
            if (isalpha(strToSplit[nInx+1]))
            {
                strSegment = strTempToSplit.substr(nStart, (nInx + 1 - nStart));
                pvecSegments->push_back(strSegment.c_str());
                nStart = nInx + 1;
            }
        }
        else if (isalpha(strToSplit[nInx]))
        {
            if (isdigit(strToSplit[nInx+1]))
            {
                strSegment = strTempToSplit.substr(nStart, (nInx + 1 - nStart));
                pvecSegments->push_back(strSegment.c_str());
                nStart = nInx + 1;
            }
        }
    }

    if (nStart < (int) strTempToSplit.length())
    {
        strSegment =strTempToSplit.substr(nStart);
        pvecSegments->push_back(strSegment.c_str());
    }
}

void GetNameFromUrl(char * strName, const char * strUml)
{
    string strTempUml(strUml);
    size_t nPos = strTempUml.rfind("/");
    strcpy(strName, strTempUml.substr(nPos+1).c_str());
}

BOOL IsFixpack(const char * strName)
{
    BOOL bFixpack = FALSE;
    if (!strncmp(strName, PREFIX_FIXPACK, strlen(PREFIX_FIXPACK)))
    {
        bFixpack = TRUE;
    }

    return bFixpack;
}

void SplitFixpackName(const char * strFixpkName, char * strName, char * strVer, char * strId)
{
    string strTempFixpkName(strFixpkName);

    size_t nPos1 = strTempFixpkName.find("_");

    if (string::npos != nPos1)
    {
        nPos1++;
        size_t nPos2 = strTempFixpkName.find("_", nPos1);
        if (string::npos != nPos2)
        {
            string strValue = strTempFixpkName.substr(nPos1, (nPos2-nPos1));
            strncpy(strName, strValue.c_str(), strValue.length());

            nPos2++;
            size_t nPos3 = strTempFixpkName.find("_", nPos2);
            if (string::npos != nPos3)
            {
                strValue = strTempFixpkName.substr(nPos2, nPos3-nPos2).c_str();
                strncpy(strVer, strValue.c_str(), strValue.length());

                strValue = strTempFixpkName.substr(nPos3+1).c_str();
                strncpy(strId, strValue.c_str(), strValue.length());
            }
        }
    }
}

int IsFixpackParentChked(HWND hwndDlgLst, const char * strName, int nNameInx, int nVerInx)
{
    int nChked = PARENT_NOT_FOUND;

    char strId[MAX_LEN];
    char strParentName[MAX_LEN];
    char strParentVer[MAX_VER_LEN];
    memset(strId,          0x00, sizeof(strId));
    memset(strParentName,  0x00, sizeof(strParentName));
    memset(strParentVer,   0x00, sizeof(strParentVer));

    SplitFixpackName(strName, strParentName, strParentVer, strId);

    for (int iInx = 0; iInx < ListView_GetItemCount(hwndDlgLst); iInx++)
    {
        // get the name of the plugin
        char strLstName[MAX_LEN];
        char strLstVer[MAX_VER_LEN];

        memset(strLstName, 0x00, sizeof(strLstName));
        memset(strLstVer,  0x00, sizeof(strLstVer));

        ListView_GetItemText(hwndDlgLst, iInx, nNameInx, strLstName, sizeof(strLstName));
        ListView_GetItemText(hwndDlgLst, iInx, nVerInx, strLstVer, sizeof(strLstVer));

        if (IsFixpack(strLstName))
        {
            continue;
        }

        BOOL bItmChked = ListView_GetCheckState(hwndDlgLst, iInx);

        if (!stricmp(strParentName, strLstName) && !stricmp(strParentVer, strLstVer))
        {
            if (bItmChked)
            {
                nChked = PARENT_CHECKED;
            }
            else
            {
                nChked = PARENT_NOT_CHECKED;
            }
            return nChked;
        }
    }

    return nChked;
}

void InitOtmCopy(POTMCOPY pOtmCopy)
{
    memset(pOtmCopy->strFrom, 0x00, sizeof(pOtmCopy->strFrom));
    memset(pOtmCopy->strTo,   0x00, sizeof(pOtmCopy->strTo));
}

void InitFixpack(PFIXPACK pFixpack)
{
    pFixpack->bRestart   = FALSE;
    pFixpack->bNeedWait  = TRUE;
    memset(pFixpack->strId,         0x00, sizeof(pFixpack->strId));
    memset(pFixpack->strDLUrl,      0x00, sizeof(pFixpack->strDLUrl));
    memset(pFixpack->strDLType,     0x00, sizeof(pFixpack->strDLType));
    memset(pFixpack->strMethod,     0x00, sizeof(pFixpack->strMethod));
    memset(pFixpack->strShortDscp,  0x00, sizeof(pFixpack->strShortDscp));
    memset(pFixpack->strLongDscp,   0x00, sizeof(pFixpack->strLongDscp));
    memset(pFixpack->strDate,       0x00, sizeof(pFixpack->strDate));
}

void InitMainThreadInfo(PMAINTHREADINFO pMainTdInfo)
{
    pMainTdInfo->bFixpack = FALSE;

    memset(pMainTdInfo->strName,     0x00, sizeof(pMainTdInfo->strName));
    memset(pMainTdInfo->strVer,      0x00, sizeof(pMainTdInfo->strVer));
    memset(pMainTdInfo->strFixpkId,  0x00, sizeof(pMainTdInfo->strFixpkId));

    pMainTdInfo->iInx = 0;
    pMainTdInfo->jInx = 0;
    pMainTdInfo->kInx = 0;
}

void InitNetworkParam(PNETWORKPARAM pNetworkParam)
{
    memset(pNetworkParam->strProxyAddress,  0x00, sizeof(pNetworkParam->strProxyAddress));
    memset(pNetworkParam->strProxyPort,     0x00, sizeof(pNetworkParam->strProxyPort));
    memset(pNetworkParam->strError,         0x00, sizeof(pNetworkParam->strError));
    pNetworkParam->nTimeout = DEF_CONNECT_TIMEOUT;
}

void GetLibcurlPath(char * strPath)
{
    char strAppPath[MAX_PATH];
    memset(strAppPath, 0x00, sizeof(strAppPath));
    GetToolAppPath(APP_NAME, strAppPath);

    char strLibcurlPath[MAX_PATH];
    memset(strLibcurlPath, 0x00, sizeof(strLibcurlPath));
    sprintf(strLibcurlPath, "%s\\%s\\%s", strAppPath, WIN_FOLDER_KEY, LIBCURL_DLL);

    strcpy(strPath, strLibcurlPath);
}

void GetToolAppPath(const char * strAppName, char * strAppPath)
{
    // get OpenTM2 properties path
    HKEY hKey = NULL;

    REGSAM regOption = NULL;
    char strSubKey[MAX_PATH];
    memset(strSubKey, 0x00, sizeof(strSubKey));

    if (OtmIsWow64())
    {
        regOption = KEY_READ | KEY_WOW64_64KEY;
        sprintf(strSubKey, "%s\\%s", APP_SOFTWARE_STR, REG_KEY_WOW_6432_NODE_STR);
    }
    else
    {
        regOption = KEY_READ | KEY_WOW64_32KEY;
        sprintf(strSubKey, "%s", APP_SOFTWARE_STR);
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, regOption, &hKey) == ERROR_SUCCESS)
    {
        HKEY hSubKey = NULL;
        if (RegOpenKeyEx(hKey, strAppName, 0, regOption, &hSubKey) == ERROR_SUCCESS)
        {
            DWORD dwType = REG_SZ;
            DWORD iSize = MAX_PATH;
            int iSuccess = RegQueryValueEx(hSubKey, KEY_DRIVE, 0, &dwType, (LPBYTE)strAppPath, &iSize);
            if (iSuccess == ERROR_SUCCESS)
            {
                dwType = REG_SZ;
                iSize = MAX_PATH;
                strcat(strAppPath, "\\");
                iSuccess = RegQueryValueEx(hSubKey, KEY_PATH, 0, &dwType, (LPBYTE)(strAppPath + strlen(strAppPath)), &iSize);
                if (iSuccess != ERROR_SUCCESS)
                {
                    strAppPath[0] = 0;
                }
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    if ((NULL == strAppPath) || (strAppPath[0] == 0))
    {
        char strModule[MAX_PATH];
        memset(strModule, 0x00, sizeof(strModule));

        GetModuleFileName(NULL, strModule, sizeof(strModule));
        GetModuleAppPath(strModule, strAppPath);
    }

}

void GetModuleAppPath(const char * strModule, char * strAppPath)
{
    char strDrive[_MAX_DRIVE];
    char strDir[_MAX_DIR];
    char strFname[_MAX_FNAME];
    char strExt[_MAX_EXT];
    _splitpath(strModule, strDrive, strDir, strFname, strExt);

    // remove last path delimiter
    int nLen = strlen(strDir);
    strDir[nLen-1] = '\0';

    // find the last path delimiter
    char * strPos = strrchr(strDir, '\\');
    nLen = strPos - strDir;

    if (nLen <= 0)
    {
        return;
    }

    strDir[nLen] = '\0';
    sprintf(strAppPath, "%s\\%s", strDrive, strDir);
}

BOOL OtmIsWow64()
{
    BOOL bWow64 = FALSE; 

    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");
    if (NULL != fnIsWow64Process)
    {
        fnIsWow64Process(GetCurrentProcess(), &bWow64);
    }

    return bWow64;
}
