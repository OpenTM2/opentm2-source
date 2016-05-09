/*! \file
Description: EQF Folder Handler Global Find and Replace (GFR) function
Batch list maintenance tab related code

Copyright Notice:

Copyright (C) 1990-2016, International Business Machines
Corporation and others. All rights reserved
*/

#include "LastUsedVal.h"

int OtmGetSysProp(PPROPSYSTEM  & pSystemProp)
{
    int nRC = NO_ERROR;

    char strSysPropPath[MAX_PATH];
    memset(strSysPropPath, 0x00, sizeof(strSysPropPath));
    GetStringFromRegistry(APPL_Name, KEY_SysProp, strSysPropPath, sizeof(strSysPropPath), "");

    ULONG ulBytesRead;             // number of bytes read by UtlLoadFile

    pSystemProp = NULL;
    BOOL bOK = UtlLoadFileL(strSysPropPath, (PVOID *)&pSystemProp, &ulBytesRead, FALSE, TRUE);

    if (!bOK && (0 == ulBytesRead))
    {
        nRC = ERROR_READ_SYS_PROP_A;
    }

    return nRC;
}

int OtmSaveSysProp(PPROPSYSTEM  & pSystemProp)
{
    int nRC = NO_ERROR;

    // update system properties
    HPROP hPropSys;            // system properties handle
    hPropSys = EqfQuerySystemPropHnd();
    if (!SetPropAccess(hPropSys, PROP_ACCESS_WRITE))
    {
        UtlError(ERROR_ACCESS_SYSTEMPROPERTIES, MB_CANCEL, 0, (PSZ *) NULP, EQF_ERROR);
        nRC = ERROR_SAVE_SYS_PROP_A;
    }
    else
    {
        EQFINFO ErrorInfo;
        PPROPSYSTEM pPropSys = GetSystemPropPtr();
        memcpy(pPropSys, pSystemProp, sizeof(*pSystemProp));
        if (SaveProperties(EqfQuerySystemPropHnd(), &ErrorInfo))
        {
            nRC = ERROR_SAVE_SYS_PROP_A;
        }
        ResetPropAccess(EqfQuerySystemPropHnd(), PROP_ACCESS_WRITE);
    }

    return nRC;
}

// clear the batch list table
BOOL GFR_ClearBatchList(PFOLFINDDATA pIda)
{
    for(int i = 0; i < pIda->iBatchListUsed; i++)
    {
        PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[i]; 
        UtlAlloc((PVOID *)&pEntry, 0, 0, NOMSG);
        pIda->ppBatchList[i] = NULL;
    } /* endfor */
    pIda->iBatchListUsed = 0;
    return(TRUE);
}

// create new batch list entry with data from batch list entry data structure
PFOLFINDBATCHLISTENTRY GFR_CreateBatchListEntry(PBATCHLISTENTRYDATA pData)
{
    PFOLFINDBATCHLISTENTRY pNewEntry = NULL;

    // compute size of newor updated entry
    int iTargetFindLen = (wcslen(pData->szTargetFind) + 1) * sizeof(CHAR_W); 
    int iSourceFindLen = (wcslen(pData->szSourceFind) + 1) * sizeof(CHAR_W); 
    int iTargetChangeLen = (wcslen(pData->szTargetChange) + 1) * sizeof(CHAR_W); 
    int iNewLen = sizeof(FOLFINDBATCHLISTENTRY) + iTargetFindLen + iSourceFindLen + iTargetChangeLen;

    // allocate new entry
    if (!UtlAlloc((PVOID *)&pNewEntry, 0, iNewLen, ERROR_STORAGE)) return(NULL);

    // fill new entry
    pNewEntry->iSize = iNewLen;
    pNewEntry->iTargetFindLen = iTargetFindLen;
    pNewEntry->iTargetFindOffs = sizeof(FOLFINDBATCHLISTENTRY);
    wcscpy((PSZ_W)(((PBYTE)pNewEntry) + pNewEntry->iTargetFindOffs), pData->szTargetFind);
    pNewEntry->iSourceFindLen = iSourceFindLen;
    pNewEntry->iSourceFindOffs = pNewEntry->iTargetFindOffs + pNewEntry->iTargetFindLen;
    wcscpy((PSZ_W)(((PBYTE)pNewEntry) + pNewEntry->iSourceFindOffs), pData->szSourceFind);
    pNewEntry->iTargetChangeLen = iTargetChangeLen;
    pNewEntry->iTargetChangeOffs = pNewEntry->iSourceFindOffs + pNewEntry->iSourceFindLen;
    wcscpy((PSZ_W)(((PBYTE)pNewEntry) + pNewEntry->iTargetChangeOffs), pData->szTargetChange);

    return(pNewEntry);
}

// add a batch list entry to the batch list table
BOOL GFR_AddBatchListEntry(PFOLFINDDATA pIda, PFOLFINDBATCHLISTENTRY pEntry, int iPos)
{
    // enlarge pointer array of batch list when necessary
    if ((pIda->iBatchListUsed + 1) > pIda->iBatchListSize)
    {
        int iNewSize = pIda->iBatchListSize + 50;
        int iOldLen = pIda->iBatchListSize * sizeof(PFOLFINDBATCHLISTENTRY);
        int iNewLen = iNewSize * sizeof(PFOLFINDBATCHLISTENTRY);
        if (!UtlAlloc((PVOID *)&(pIda->ppBatchList), iOldLen, iNewLen, ERROR_STORAGE)) return(FALSE);
        pIda->iBatchListSize = iNewSize;
    }

    // verify given position
    if ((iPos < 0) || (iPos > pIda->iBatchListUsed))iPos = pIda->iBatchListUsed;

    // make room at given position
    for(int i = (pIda->iBatchListUsed - 1); i >= iPos; i--)
    {
        pIda->ppBatchList[i+1] = pIda->ppBatchList[i];
    }

    // add pointer of new entry at given position
    pIda->ppBatchList[iPos] = pEntry;

    pIda->iBatchListUsed += 1;

    return(TRUE);
}

// process a single line from an imported batch list
BOOL ProcessImportedBatchListLine(PFOLFINDDATA pIda, PSZ_W pszLine, PBATCHLISTENTRYDATA pData)
{
    memset(pData, 0, sizeof(BATCHLISTENTRYDATA));
    int iColumn = 0;

    while((iColumn < 3) && (*pszLine != 0))
    {
        while(iswspace(*pszLine)) pszLine++; // skip any whitespace

        PSZ_W pszStart = pszLine;

        if (*pszLine == L'\"')
        {
            // process text enclosed in quotes
            pszLine++; pszStart++;
            while((*pszLine != 0) && (*pszLine != L'\"') && (*pszLine != L'\n') && (*pszLine != L'\r')) pszLine++;

            if (*pszLine == L'\"') 
            {
                *pszLine = 0;
                pszLine++;
                while(iswspace(*pszLine)) pszLine++; // skip any whitespace
                if (*pszLine == L',') pszLine++;
            }
        }
        else
        {
            // find next comma
            while((*pszLine != 0) && (*pszLine != L',') && (*pszLine != L'\n') && (*pszLine != L'\r')) pszLine++;
            if (*pszLine != 0) 
            {
                *pszLine = 0;
                pszLine++;
            } /* endif */
        } /* endif */

        // handle any data found
        switch(iColumn)
        {
        case 0: wcscpy(pData->szTargetFind, pszStart); break;
        case 1: wcscpy(pData->szSourceFind, pszStart); break;
        case 2: wcscpy(pData->szTargetChange, pszStart); break;
        } /* endswitch */
        iColumn++;
    } /* endwhile */

    // add entry to batch list if ok
    if ((pData->szTargetFind[0] != 0) || (pData->szSourceFind[0] != 0))
    {
        PFOLFINDBATCHLISTENTRY pNewEntry = GFR_CreateBatchListEntry(pData);
        if (pNewEntry)
        {
            GFR_AddBatchListEntry(pIda, pNewEntry, pIda->iBatchListUsed);
        } /* endif */
    } /* endif */
    return(TRUE);
} /* end of ProcessImportedBatchListLine */

// import a batch list
BOOL GFR_ImportBatchList(PFOLFINDDATA pIda, PSZ pszListFile)
{

    FILE *hfList = fopen(pszListFile, "rb");
    if (hfList == NULL)
    {
        UtlError(FILE_NOT_EXISTS, MB_CANCEL, 1, &pszListFile, EQF_ERROR);
        return(FALSE);
    } /* endif */

    // clear existing batch list
    GFR_ClearBatchList(pIda);

    // read first two bytes and check for UTF16 BOM
    fread(pIda->szNameBuffer, 2, 1, hfList);

    if (memcmp(pIda->szNameBuffer, UNICODEFILEPREFIX, 2) == 0)
    {
        // import unicode list
        while(!feof(hfList))
        {
            pIda->szBuffer[0] = 0;
            fgetws(pIda->szBuffer, sizeof(pIda->szBuffer)/ sizeof(CHAR_W), hfList);
            ProcessImportedBatchListLine(pIda, pIda->szBuffer, &pIda->BatchEntryData);
        } /* endwhile */
    }
    else
    {
        // import ANSI list
        fclose(hfList);
        hfList = fopen(pszListFile, "r");
        if (hfList == NULL)
        {
            UtlError(FILE_NOT_EXISTS, MB_CANCEL, 1, &pszListFile, EQF_ERROR);
            return(FALSE);
        } /* endif */

        while(!feof(hfList))
        {
            pIda->szNameBuffer[0] = EOS;
            fgets(pIda->szNameBuffer, sizeof(pIda->szNameBuffer), hfList);
            int iChars = MultiByteToWideChar(CP_ACP, 0, pIda->szNameBuffer, strlen(pIda->szNameBuffer), pIda->szBuffer, sizeof(pIda->szBuffer) / sizeof(CHAR_W));
            pIda->szBuffer[iChars] = 0;
            ProcessImportedBatchListLine(pIda, pIda->szBuffer, &pIda->BatchEntryData);
        } /* endwhile */
    }
    fclose(hfList);

    return(TRUE);
}

// write a string to the output file, enclose string in quotes if it contains a comma
BOOL WriteStringToExportedBatchList(FILE *hfList, PSZ_W pszString)
{
    PSZ_W pszComma = wcschr(pszString, L',');

    if (pszComma != NULL) fwrite(L"\"", sizeof(CHAR_W), 1, hfList);

    // strip any linefeed from string
    int iLen = wcslen(pszString);
    if (iLen && pszString[iLen-1] == L'\n') iLen--;
    if (iLen && pszString[iLen-1] == L'\r') iLen--;
    fwrite(pszString, sizeof(CHAR_W), iLen, hfList);

    if (pszComma != NULL) fwrite(L"\"", sizeof(CHAR_W), 1, hfList);

    return(TRUE);
}

// export a batch list
BOOL GFR_ExportBatchList(PFOLFINDDATA pIda, PSZ pszListFile)
{
    FILE *hfList = fopen(pszListFile, "wb");
    if (hfList == NULL)
    {
        UtlError(ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszListFile, EQF_ERROR);
        return(FALSE);
    } /* endif */

    // write BOM 
    fwrite(UNICODEFILEPREFIX, strlen(UNICODEFILEPREFIX), 1, hfList);

    // write batch list entries
    for(int i = 0; i < pIda->iBatchListUsed; i++)
    {
        PFOLFINDBATCHLISTENTRY pEntry = pIda->ppBatchList[i];

        WriteStringToExportedBatchList(hfList, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetFindOffs));
        fwrite(L",", sizeof(CHAR_W), 1, hfList);
        WriteStringToExportedBatchList(hfList, (PSZ_W)(((PBYTE)pEntry) + pEntry->iSourceFindOffs));
        fwrite(L",", sizeof(CHAR_W), 1, hfList);
        WriteStringToExportedBatchList(hfList, (PSZ_W)(((PBYTE)pEntry) + pEntry->iTargetChangeOffs));
        fwrite(L"\r\n", sizeof(CHAR_W), 2, hfList);
    }

    fclose(hfList);

    return(TRUE);
}

BOOL GFR_GetLastUsedValues(PFOLFINDDATA pIda)
{
    PFOLFINDLASTUSED pLastUsed = NULL;
    BOOL fOK = TRUE;

    // build name of our file containing the last used values
    CHAR szLastUsedValues[MAX_LONGPATH];
    UtlMakeEQFPath(szLastUsedValues, NULC, PROPERTY_PATH, NULL);
    strcat(szLastUsedValues, "\\GlobFindLastUsedValues.DAT");

    // allocate last values area
    fOK = UtlAlloc((PVOID *)&pLastUsed, 0, sizeof(FOLFINDLASTUSED), ERROR_STORAGE);
    if (!fOK) return(FALSE);

    // load existing file, use values from folder list properties if load fails
    FILE *hfLastUsed = fopen(szLastUsedValues, "rb");

    if (hfLastUsed == NULL)
    {
        // get last used values from folder list property file
        EQFINFO     ErrorInfo;       // error code of property handler calls
        PPROPFOLDERLIST pFllProp = NULL;// ptr to folder list properties
        PVOID       hFllProp;        // handle of folder list properties
        OBJNAME     szFllObjName;    // buffer for folder list object name

        UtlMakeEQFPath(szFllObjName, NULC, SYSTEM_PATH, NULL);
        strcat(szFllObjName, BACKSLASH_STR);
        strcat(szFllObjName, DEFAULT_FOLDERLIST_NAME);
        hFllProp = OpenProperties(szFllObjName, NULL, PROP_ACCESS_READ, &ErrorInfo);
        if (hFllProp) pFllProp = (PPROPFOLDERLIST)MakePropPtrFromHnd(hFllProp);
        if (pFllProp)
        {
            int i=0;

            pLastUsed->fFolFindCaseRespect = pFllProp->fFolFindCaseRespect;
            pLastUsed->fFolFindUpdateTM = pFllProp->fFolFindUpdateTM;
            pLastUsed->fFolFindTranslTextOnly = pFllProp->fFolFindTranslTextOnly;
            pLastUsed->fFolFindWholeWordsOnly = pFllProp->fFolFindWholeWordsOnly;
            pLastUsed->fFolFindConfirm = pFllProp->fFolFindConfirm;
            pLastUsed->chWildCardSingleChar = pFllProp->chWildCardSingleChar;
            pLastUsed->chWildCardMultChar = pFllProp->chWildCardMultChar;
            wcscpy(pLastUsed->szFolFind, pFllProp->szFolFind);
            wcscpy(pLastUsed->szFolChangeTo, pFllProp->szFolChangeTo);
            while (i < MAX_SEARCH_HIST && pFllProp->szFindList[i][0] != EOS)
            {
                wcscpy(pLastUsed->szFindList[i], pFllProp->szFindList[i]);
                i++;
            }// end while
            i=0;
            while (i < MAX_SEARCH_HIST && pFllProp->szReplaceList[i][0] != EOS)
            {
                wcscpy(pLastUsed->szReplaceList[i], pFllProp->szReplaceList[i]);
                i++;
                i++;
            }// end while
            memcpy(&(pLastUsed->swpFolFindSizePos), &(pFllProp->swpFolFindSizePos), sizeof(pLastUsed->swpFolFindSizePos));
        }
    }
    else
    {
        // load last used values (don't care if loaded data is shorter than our last used values area)
        fread(pLastUsed, 1, sizeof(FOLFINDLASTUSED), hfLastUsed);
        fclose(hfLastUsed);
    }

    // anchor last used values in IDA
    pIda->pLastUsed = pLastUsed;

    // add by Flora Lee: file should be close?
    if (NULL != hfLastUsed)
    {
        fclose(hfLastUsed);
    }

    return(TRUE);
}

BOOL GFR_SaveLastUsedValues(PFOLFINDDATA pIda)
{
    // build name of our file containing the last used values
    CHAR szLastUsedValues[MAX_LONGPATH];
    UtlMakeEQFPath(szLastUsedValues, NULC, PROPERTY_PATH, NULL);
    strcat(szLastUsedValues, "\\GlobFindLastUsedValues.DAT");

    UtlWriteFile(szLastUsedValues, sizeof(FOLFINDLASTUSED), (PVOID)pIda->pLastUsed, FALSE);

    return(TRUE);
}

int loadPropFile(
    PSZ pszName,
    PSZ pszPassword,
    void **ppvProp,
    int iSize
   )
{
    int iRC = 0;
    char szPath[512];
    unsigned short usSize = 0;

    UtlMakeEQFPath(szPath, NULC, PROPERTY_PATH, NULL);
    strcat(szPath, "\\");
    strcat(szPath, pszName);

    *ppvProp = NULL;
    if (!UtlLoadFile(szPath, ppvProp, &usSize, FALSE, FALSE))
    {
        return(GetLastError());
    } /* endif */

    if (usSize != (USHORT)iSize)
    {
        UtlAlloc(ppvProp, 0, 0, NOMSG);
        return(ERROR_READ_FAULT);  
    } /* endif */     


    // decrypt the data
    if (pszPassword != NULL)
    {
        encrypt((PBYTE)*ppvProp, (int)usSize, pszPassword, FALSE);
    } /* endif */

    return(iRC);
}

/*! \brief Write memory properties file 
\param pszName name of the property file
\param pszPassword properties encryption password
\param pvProp pointer to the properties file
\param iPropSize size o fthe properties in number of bytes
\returns 0 if successful or error return code
*/
int writePropFile(
    PSZ pszName,
    PSZ pszPassword,
    void *pvProp,
    int iPropSize
   )
{
    int iRC = 0;
    char szPath[512];

    UtlMakeEQFPath(szPath, NULC, PROPERTY_PATH, NULL);
    strcat(szPath, "\\");
    strcat(szPath, pszName);

    // encrypt the the data
    if (pszPassword != NULL)
    {
        encrypt((PBYTE)pvProp, iPropSize, pszPassword, TRUE);
    } /* endif */     

    if (!UtlWriteFile(szPath, (USHORT)iPropSize, pvProp, FALSE))
    {
        return(GetLastError());
    } /* endif */

    return(iRC);
}

/*! \brief Simple data decrypter/encrypter
\param pbData pointer to data area being encrypter/decrypted
\param iSize number of bytes in data area
\param pszPassword password to be used for decryption/encryption
\param fEncrypt true = encrypt, false = decrypt
\returns 0
*/
int encrypt(PBYTE pbData, int iSize, PSZ pszPassword, BOOL fEncrypt)
{
    if (fEncrypt)
    {
        BYTE bLastValue = 0;
        int iLen = strlen(pszPassword);
        int iPW = 0;
        for (int i = 0; i < iSize; i++)
        {
            BYTE bValue = (BYTE)(pbData[i] + bLastValue);
            bValue = (BYTE)(bValue + (pszPassword[iPW] * (iPW+1) * 3));
            bLastValue = bValue;
            pbData[i] = bValue;
            iPW++;
            if (iPW == iLen) iPW = 0;
        } /* endfor */
    }
    else
    {
        BYTE bLastValue = 0;
        int iLen = strlen(pszPassword);
        int iPW = 0;
        for (int i = 0; i < iSize; i++)
        {
            BYTE bValue = pbData[i];
            bValue = (BYTE)(bValue - (pszPassword[iPW] * (iPW+1) * 3));
            bValue = (BYTE)(bValue - bLastValue);
            bLastValue = pbData[i];
            pbData[i] = bValue;
            iPW++;
            if (iPW == iLen) iPW = 0;
        } /*endfor */
    } /* endif */
    return(0);
}

// get settings from trigger file
BOOL GetTriggerFileSettings(PNFLUENTDATA pData)
{
    FILE *hfTrigger = NULL;

    // setup trigger file name
    UtlMakeEQFPath(pData->szFileName, NULC, PROPERTY_PATH, NULL);
    strcat(pData->szFileName, BACKSLASH_STR);
    strcat(pData->szFileName, NFLUENT_FILE_NAME);

    // loop over trigger file lines
    hfTrigger = fopen(pData->szFileName, "r");
    if (hfTrigger)
    {
        fgets((PSZ)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer), hfTrigger);
        while (!feof(hfTrigger))
        {
            PSZ pszLine = (PSZ)pData->bUTF8Buffer;
            if (*pszLine != '*')
            {
                if (_strnicmp(pszLine, "MTLOGGING", 9) == 0)
                {
                    pData->bMTLogging = TRUE;
                }
                else if (_strnicmp(pszLine, "INCLUDEWORDCOUNT", 16) == 0)
                {
                    pData->bIncWrdCnt = TRUE;
                }
                else if (_strnicmp(pszLine, "NOMATCHEXP_NODUPLICATE", 22) == 0)
                {
                    pData->bNoMatchExpNoDuplic = TRUE;
                }
                else if (_strnicmp(pszLine, "NOMATCHEXP", 10) == 0)
                {
                    pData->bNoMatchExp = TRUE;
                }
                else if (_strnicmp(pszLine, "NOMATCH_NODUPLICATE", 19) == 0)
                {
                    pData->bNoMatchNoDuplic = TRUE;
                }
                else if (_strnicmp(pszLine, "NOMATCH", 7) == 0)
                {
                    pData->bNoMatch = TRUE;
                }
                else if (_strnicmp(pszLine, "ALLSEGSEXP_NODUPLICATE", 22) == 0)
                {
                    pData->bAllSegsExpNoDuplic = TRUE;
                }
                else if (_strnicmp(pszLine, "ALLSEGSEXP", 10) == 0)
                {
                    pData->bAllSegsExp = TRUE;
                }
                else if (_strnicmp(pszLine, "ALLSEGS_NODUPLICATE", 19) == 0)
                {
                    pData->bAllSegsNoDuplic = TRUE;
                }
                else if (_strnicmp(pszLine, "ALLSEGS", 7) == 0)
                {
                    pData->bAllSegs = TRUE;
                }
                else if (_strnicmp(pszLine, "ALLWMATCHSOURCE_NODUPLICATE", 27) == 0)
                {
                    pData->bAllWMatchSrcNoDuplic = FALSE;
                }
                else if (_strnicmp(pszLine, "ALLWMATCHSOURCE", 15) == 0)
                {
                    pData->bAllWMatchSrc = TRUE;
                }
                else if (_strnicmp(pszLine, "ALLWMATCH_NODUPLICATE", 21) == 0)
                {
                    pData->bAllWMatchNoDuplic = TRUE;
                }
                else if (_strnicmp(pszLine, "ALLWMATCH", 9) == 0)
                {
                    pData->bAllWMatch = TRUE;
                }
                else if (_strnicmp(pszLine, "NOPROPOSALEXP_NODUPLICATE", 25) == 0)
                {
                    pData->bNoProposalExpNoDuplic = TRUE;
                }
                else if (_strnicmp(pszLine, "NOPROPOSALEXP", 13) == 0)
                {
                    pData->bNoProposalExp = TRUE;
                }
                else if (_strnicmp(pszLine, "NOPROPOSAL_NODUPLICATE", 22) == 0)
                {
                    pData->bNoProposalNoDuplic = TRUE;
                }
                else if (_strnicmp(pszLine, "NOPROPOSAL", 9) == 0)
                {
                    pData->bNoProposal = TRUE;
                }
                else if (_strnicmp(pszLine, "XLIFF", 5) == 0)
                {
                    pData->bXliff = TRUE;
                }
                else
                {
                    // ignore unknown switch
                }

            }
            fgets((PSZ)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer), hfTrigger);
        }
        fclose(hfTrigger);
    }
    return(TRUE);
}

BOOL SaveTriggerFileSettings(PNFLUENTDATA pData)
{
    FILE * fileTrigger = NULL;
    FILE * fileTemp = NULL;

    char strTempFile[MAX_BUF_SIZE];
    memset(strTempFile, 0x00, sizeof(strTempFile));

    // setup trigger file name
    UtlMakeEQFPath(pData->szFileName, NULC, PROPERTY_PATH, NULL);
    strcat(pData->szFileName, BACKSLASH_STR);
    strcat(pData->szFileName, NFLUENT_FILE_NAME);

    UtlMakeEQFPath(strTempFile, NULC, PROPERTY_PATH, NULL);
    strcat(strTempFile, BACKSLASH_STR);
    strcat(strTempFile, NFLUENT_FILE_NAME_TMP);

    fileTemp = fopen(strTempFile, "wb");
    if (NULL == fileTemp)
    {
        return FALSE;
    }

    // loop over trigger file lines
    fileTrigger = fopen(pData->szFileName, "r");
    if (fileTrigger)
    {
        fgets((PSZ)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer), fileTrigger);
        while (!feof(fileTrigger))
        {
            PSZ pszLine = (PSZ)pData->bUTF8Buffer;
            if (*pszLine == '*')
            {
                pszLine++;
            }

            if (_strnicmp(pszLine, "MTLOGGING", 9) == 0)
            {
                if (pData->bMTLogging)
                {
                    fprintf(fileTemp, "%s\r\n", "MTLOGGING");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "MTLOGGING");
                }
            }
            else if (_strnicmp(pszLine, "INCLUDEWORDCOUNT", 16) == 0)
            {
                if (pData->bIncWrdCnt)
                {
                    fprintf(fileTemp, "%s\r\n", "INCLUDEWORDCOUNT");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "INCLUDEWORDCOUNT");
                }
            }
            else if (_strnicmp(pszLine, "NOMATCHEXP_NODUPLICATE", 22) == 0)
            {
                if (pData->bNoMatchExpNoDuplic)
                {
                    fprintf(fileTemp, "%s\r\n", "NOMATCHEXP_NODUPLICATE");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "NOMATCHEXP_NODUPLICATE");
                }
            }
            else if (_strnicmp(pszLine, "NOMATCHEXP", 10) == 0)
            {
                if (pData->bNoMatchExp)
                {
                    fprintf(fileTemp, "%s\r\n", "NOMATCHEXP");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "NOMATCHEXP");
                }
            }
            else if (_strnicmp(pszLine, "NOMATCH_NODUPLICATE", 19) == 0)
            {
                if (pData->bNoMatchNoDuplic)
                {
                    fprintf(fileTemp, "%s\r\n", "NOMATCH_NODUPLICATE");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "NOMATCH_NODUPLICATE");
                }
            }
            else if (_strnicmp(pszLine, "NOMATCH", 7) == 0)
            {
                if (pData->bNoMatch)
                {
                    fprintf(fileTemp, "%s\r\n", "NOMATCH");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "NOMATCH");
                }
            }
            else if (_strnicmp(pszLine, "ALLSEGSEXP_NODUPLICATE", 22) == 0)
            {
                if (pData->bAllSegsExpNoDuplic)
                {
                    fprintf(fileTemp, "%s\r\n", "ALLSEGSEXP_NODUPLICATE");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "ALLSEGSEXP_NODUPLICATE");
                }
            }
            else if (_strnicmp(pszLine, "ALLSEGSEXP", 10) == 0)
            {
                if (pData->bAllSegsExp)
                {
                    fprintf(fileTemp, "%s\r\n", "ALLSEGSEXP");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "ALLSEGSEXP");
                }
            }
            else if (_strnicmp(pszLine, "ALLSEGS_NODUPLICATE", 19) == 0)
            {
                if (pData->bAllSegsNoDuplic)
                {
                    fprintf(fileTemp, "%s\r\n", "ALLSEGS_NODUPLICATE");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "ALLSEGS_NODUPLICATE");
                }
            }
            else if (_strnicmp(pszLine, "ALLSEGS", 7) == 0)
            {
                if (pData->bAllSegs)
                {
                    fprintf(fileTemp, "%s\r\n", "ALLSEGS");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "ALLSEGS");
                }
            }
            else if (_strnicmp(pszLine, "ALLWMATCHSOURCE_NODUPLICATE", 27) == 0)
            {
                if (pData->bAllWMatchSrcNoDuplic)
                {
                    fprintf(fileTemp, "%s\r\n", "ALLWMATCHSOURCE_NODUPLICATE");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "ALLWMATCHSOURCE_NODUPLICATE");
                }
            }
            else if (_strnicmp(pszLine, "ALLWMATCHSOURCE", 15) == 0)
            {
                if (pData->bAllWMatchSrc)
                {
                    fprintf(fileTemp, "%s\r\n", "ALLWMATCHSOURCE");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "ALLWMATCHSOURCE");
                }
            }
            else if (_strnicmp(pszLine, "ALLWMATCH_NODUPLICATE", 21) == 0)
            {
                if (pData->bAllWMatchNoDuplic)
                {
                    fprintf(fileTemp, "%s\r\n", "ALLWMATCH_NODUPLICATE");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "ALLWMATCH_NODUPLICATE");
                }
            }
            else if (_strnicmp(pszLine, "ALLWMATCH", 9) == 0)
            {
                if (pData->bAllWMatch)
                {
                    fprintf(fileTemp, "%s\r\n", "ALLWMATCH");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "ALLWMATCH");
                }
            }
            else if (_strnicmp(pszLine, "NOPROPOSALEXP_NODUPLICATE", 25) == 0)
            {
                if (pData->bNoProposalExpNoDuplic)
                {
                    fprintf(fileTemp, "%s\r\n", "NOPROPOSALEXP_NODUPLICATE");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "NOPROPOSALEXP_NODUPLICATE");
                }
            }
            else if (_strnicmp(pszLine, "NOPROPOSALEXP", 13) == 0)
            {
                if (pData->bNoProposalExp)
                {
                    fprintf(fileTemp, "%s\r\n", "NOPROPOSALEXP");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "NOPROPOSALEXP");
                }
            }
            else if (_strnicmp(pszLine, "NOPROPOSAL_NODUPLICATE", 22) == 0)
            {
                if (pData->bNoProposalNoDuplic)
                {
                    fprintf(fileTemp, "%s\r\n", "NOPROPOSAL_NODUPLICATE");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "NOPROPOSAL_NODUPLICATE");
                }
            }
            else if (_strnicmp(pszLine, "NOPROPOSAL", 9) == 0)
            {
                if (pData->bNoProposal)
                {
                    fprintf(fileTemp, "%s\r\n", "NOPROPOSAL");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "NOPROPOSAL");
                }
            }
            else if (_strnicmp(pszLine, "XLIFF", 5) == 0)
            {
                if (pData->bXliff)
                {
                    fprintf(fileTemp, "%s\r\n", "XLIFF");
                }
                else
                {
                    fprintf(fileTemp, "*%s\r\n", "XLIFF");
                }
            }
            else
            {
                int nLstPos = strlen((PSZ)pData->bUTF8Buffer) - 1;
                // remove \n
                if (pData->bUTF8Buffer[nLstPos] == '\n')
                {
                    pData->bUTF8Buffer[nLstPos] = '\0';
                }
                fprintf(fileTemp, "%s\r\n", (PSZ)pData->bUTF8Buffer);
            }

            fgets((PSZ)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer), fileTrigger);
        }
        fclose(fileTrigger);
        fclose(fileTemp);
    }

    CopyFile(strTempFile, pData->szFileName, FALSE);
    DeleteFile(strTempFile);

    return(TRUE);
}

void UtlMakeLUVFileName(const char * strFileName, char * strFilePath)
{
    UtlMakeEQFPath(strFilePath, NULC, LIST_PATH, NULL);
    strcat(strFilePath, "\\" );
    strcat(strFilePath, strFileName);
    strcat(strFilePath, ".LUV");
    return;
}
