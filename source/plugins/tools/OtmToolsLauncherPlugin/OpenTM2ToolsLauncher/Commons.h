#pragma once
/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#include <string>
#include <vector>
class Commons
{
public:
    Commons(void);
    ~Commons(void);
    static CString searchDialog(const CString filters);
    static CString searchFolder();
    static CString getFileNameFromPath(const CString fileName);
    static std::string getOpenTM2InstallPath(char* resOut, const int length);
    static int executeCommand(const std::string cmd, const std::string output);
    static void runAndFillLog(CRect rect, CWnd* pParentDlg, UINT id, std::string cmd,std::string otmpath);
	static void getSourceLanguage(std::vector<std::string>& srclangs);
	static void getTargetLanguage(std::vector<std::string>& tgtlangs);
	static void getLanguages(std::vector<std::string>& tgtlangs);
	static void getMarkuptables(std::vector<std::string>& markups);
private:
    static CListBox *mpList;
	static std::string mOtmpath;
};

