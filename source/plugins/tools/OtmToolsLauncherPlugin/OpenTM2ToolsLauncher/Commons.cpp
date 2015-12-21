/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include "StdAfx.h"
#include "Commons.h"
#include <fstream>
#include <algorithm>


CListBox *Commons::mpList = NULL;
std::string Commons::mOtmpath = "";

Commons::Commons(void)
{  
}


Commons::~Commons(void)
{
}

CString Commons::searchDialog(const CString filters)
{

      CFileDialog dlg(TRUE,NULL,NULL,OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, filters);

      if (dlg.DoModal() == IDOK )
      {
         CString strPath ;
         strPath  = dlg.GetPathName();
         if(strPath.IsEmpty())
         { 
             return CString("");
         }
         return strPath;
      }
      return CString("");
}

CString Commons::searchFolder()
{
      CFolderPickerDialog dlg;
      if (dlg.DoModal() == IDOK )
      {
         CString strPath  = dlg.GetFolderPath();
         return strPath;
      }
      return "";
}

CString Commons::getFileNameFromPath(const CString fileName)
{
    if(fileName.IsEmpty())
        return fileName;

    int idx = fileName.ReverseFind('\\');
    if(idx != -1)
    {
        CString fname = fileName.Right(fileName.GetLength()-idx-1);
        if(fname.Find(L'.') != -1)
        {
            fname = fname.Left(fname.Find('.'));
        }
        if(!fname.IsEmpty())
            return fname;
    }
    return fileName;
}

std::string Commons::getOpenTM2InstallPath(char* resOut, const int length)
{
    if(resOut==NULL || length<=0)
        return "";

	if(mOtmpath != "")
	{
		strncpy(resOut,mOtmpath.c_str(),length);
		return mOtmpath;
	}

    HKEY hKey = NULL;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        HKEY hSubKey = NULL;
        if (RegOpenKeyExA(hKey, "OpenTM2" , 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
        {
            DWORD dwType = REG_SZ;
            DWORD iSize = length*sizeof(resOut[0]);
            int iSuccess = RegQueryValueExA(hSubKey, "", 0, &dwType, (LPBYTE)&(resOut[0]), &iSize);
            if (iSuccess == ERROR_SUCCESS)
            {
                dwType = REG_SZ;
                //strcat(pResOut, "\\");
                //resOut.append(L"\\");
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    if (resOut[0]=='\0')
    {
        return "";
    }
    
	mOtmpath = resOut;
    return resOut;
}

int Commons::executeCommand(const std::string cmd, const std::string output)
{
    DWORD nRC = NO_ERROR;

    STARTUPINFOA si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    if(!output.empty())
    {
        SECURITY_ATTRIBUTES StdErrSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
        si.hStdOutput = CreateFileA(output.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, 
                                   &StdErrSA, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    else
        si.hStdOutput = NULL;
    si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi;

    if (CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);

        if (!GetExitCodeProcess(pi.hProcess, &nRC))
        {
            nRC = GetLastError();
        }
    }
    else
    {
        nRC = GetLastError();
    }
    
    return nRC;
}

void Commons::runAndFillLog(CRect rect, CWnd* pParentDlg, UINT id, std::string cmd,std::string otmpath)
{    
    if(mpList!=NULL)
    {
        delete mpList;
        mpList = NULL;
    }

    if(mpList==NULL)
    {
        mpList = new CListBox();
        if(mpList == NULL)
            return;
    }

    // dynamically create list box
    mpList->Create(LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_HSCROLL|WS_VISIBLE|WS_CHILD|WS_TABSTOP,
                      rect,pParentDlg,id);

    // set font
    CFont* pFont = pParentDlg->GetFont();
    mpList->SetFont(pFont);

    // set window rect
    CRect mainRect;
    pParentDlg->GetDlgItem(id)->GetWindowRect(&mainRect);
    mpList->ScreenToClient(&mainRect);
    mpList->MoveWindow( mainRect.left+rect.left,
                       mainRect.top+rect.top,
                       mainRect.Width(),
                       mainRect.Height());

	mpList->InsertString(0,"Processing started ...");
	mpList->RedrawWindow();
	
    // run command
    std::string logpath = otmpath+"\\LOGS\\OpenTM2ToolsLauncher.log";
    int nRC = Commons::executeCommand(cmd,logpath);

    std::ifstream fin(logpath.c_str());
    std::string line;
    int idx = 0;
 
    //while(mpList->GetCount()>0)
    mpList->DeleteString(0);

    // determin horizontal scroll bar
    CDC *pDC = mpList->GetDC();
    if ( NULL == pDC )
    {
       return;
    }

    int nMaxExtent = 0;
    CString szText;

    while(getline(fin,line))
    {
        mpList->InsertString(idx,(LPCTSTR)line.c_str());
        mpList->GetText( idx, szText );
        CSize &cs = pDC->GetTextExtent( szText );
        if ( cs.cx > nMaxExtent )
        {
            nMaxExtent = cs.cx;
        }
        idx++;
    }
    fin.close();
    mpList->ReleaseDC(pDC);

    if ( mpList->GetCount() < 1 )
    {
       mpList->SetHorizontalExtent( 0 );
       return;
    }

    mpList->SetHorizontalExtent( nMaxExtent );

}


void  Commons::getSourceLanguage(std::vector<std::string>& srclangs)
{
	char szBuf[512+1] = {'\0'};
	std::string otmpath = getOpenTM2InstallPath(szBuf,512);
	if(otmpath.length()==0)
		return;

	std::string langFile = otmpath+"\\Table\\languages.xml";
	std::fstream fin(langFile);
	std::string line;
	
	while(getline(fin,line))
	{
		std::string::size_type sNameIdx = line.find("<name>");
		if(sNameIdx==std::string::npos)
			continue;

	    std::string::size_type eNameIdx = line.find("</name>");
		if(eNameIdx==std::string::npos)
			continue;

		std::string nameLine = line;

		while(getline(fin,line)) 
		{
		    std::string::size_type sTypeIdx = line.find("<isSourceLanguage>");
			if(sTypeIdx==std::string::npos)
			    continue;

			std::string::size_type eTypeIdx = line.find("</isSourceLanguage>");
			if(eTypeIdx==std::string::npos)
				continue;

			std::string isSource = line.substr(sTypeIdx+strlen("<isSourceLanguage>"), eTypeIdx-sTypeIdx-strlen("<isSourceLanguage>"));
			if(isSource=="yes")
			{
			    std::string name = nameLine.substr(sNameIdx+strlen("<name>"),eNameIdx-sNameIdx-strlen("<name>"));
				srclangs.push_back(name);
			}
			break;
		}
	
	}//end while

	fin.close();
	std::sort(srclangs.begin(),srclangs.end());
}


void  Commons::getTargetLanguage(std::vector<std::string>& tgtlangs)
{
	char szBuf[512+1] = {'\0'};
	std::string otmpath = getOpenTM2InstallPath(szBuf,512);
	if(otmpath.length()==0)
		return;

	std::string langFile = otmpath+"\\Table\\languages.xml";
	std::fstream fin(langFile);
	std::string line;

	while(getline(fin,line))
	{
		std::string::size_type sNameIdx = line.find("<name>");
		if(sNameIdx==std::string::npos)
			continue;

	    std::string::size_type eNameIdx = line.find("</name>");
		if(eNameIdx==std::string::npos)
			continue;

		std::string nameLine = line;

		while(getline(fin,line)) 
		{
		    std::string::size_type sTypeIdx = line.find("<isTargetLanguage>");
			if(sTypeIdx==std::string::npos)
			    continue;

			std::string::size_type eTypeIdx = line.find("</isTargetLanguage>");
			if(eTypeIdx==std::string::npos)
				continue;

			std::string isSource = line.substr(sTypeIdx+strlen("<isTargetLanguage>"), eTypeIdx-sTypeIdx-strlen("<isTargetLanguage>"));
			if(isSource=="yes")
			{
			    std::string name = nameLine.substr(sNameIdx+strlen("<name>"),eNameIdx-sNameIdx-strlen("<name>"));
				tgtlangs.push_back(name);
			}
			break;
		}
	
	}//end while

	fin.close();
	std::sort(tgtlangs.begin(),tgtlangs.end());
}


void Commons::getLanguages(std::vector<std::string>& langs)
{
	std::vector<std::string> srclangs;
	std::vector<std::string> tgtlangs;

	getSourceLanguage(srclangs);
	getTargetLanguage(tgtlangs);

	
	for(std::size_t i=0; i<tgtlangs.size(); i++)
	{
		langs.push_back(tgtlangs[i]);
	}
	
	for(std::size_t i=0; i<srclangs.size(); i++)
	{
		std::size_t j=0;
		for(; j<langs.size(); j++)
			if(langs[j] == srclangs[i])
				break;
		if(j==langs.size())
			langs.push_back(srclangs[i]);
	
	}
	std::sort(langs.begin(),langs.end());
}


void Commons::getMarkuptables(std::vector<std::string>& markups)
{
	char szBuf[512+1] = {'\0'};
	std::string otmpath = getOpenTM2InstallPath(szBuf,512);
	if(otmpath.length()==0)
		return;

	std::vector<std::string> names;

	std::string otmMarkuptableFile = otmpath+"\\PLUGINS\\OtmMarkupTablePlugin\\OtmMarkupTablePlugin.xml";
	names.push_back(otmMarkuptableFile);

	for(int i=0; i<names.size(); i++)
	{
		std::fstream fin(names[i]);
		std::string line;
		while(getline(fin,line))
		{
			if(line.find("<markup>")!=std::string::npos)
			{
				getline(fin,line);
			    
				std::string::size_type begIdx = line.find("<name>");
				std::string::size_type endIdx = line.find("</name>");
				if(begIdx!=std::string::npos && endIdx!=std::string::npos)
				{
					std::string table = line.substr(begIdx+6,endIdx-begIdx-6);
					markups.push_back(table);
				}
			}
		}
		
		fin.close();
	}
	std::sort(markups.begin(),markups.end());
}


