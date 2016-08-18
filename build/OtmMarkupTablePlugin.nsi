; Copyright (c) 2015, International Business Machines
; Corporation and others.  All rights reserved.
;                                                  Updated:  1-14-15

;NSIS Source for OpenTM2 OtmMarkupTablePlugin installer
;Parts taken from NSIS examples

AllowRootDirInstall true

Var SIZE
!define APPNAME "OTM"
!define PRODUCT_NAME "OpenTM2-OtmMarkupTablePlugin"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "IBM"
!define PRODUCT_WEB_SITE "http://www.opentm2.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\OpenTM2Starter.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define UninstLog "uninstall.log"
Var UninstLog

!include WinMessages.nsh
!include FileFunc.nsh
!include "MUI2.nsh"
!include "TextFunc.nsh"
!include WordFunc.nsh
!insertmacro GetDrives
!insertmacro DriveSpace

LangString PAGE_TITLE ${LANG_ENGLISH} "Select drive..."
LangString PAGE_SUBTITLE ${LANG_ENGLISH} "OpenTM2 can be installed only in these directories on available drives"

 

 
!define AddItem "!insertmacro AddItem"
 
; File macro
!macro File FilePath FileName
 IfFileExists "$OUTDIR\${FileName}" +2
  FileWrite $UninstLog "$OUTDIR\${FileName}$\r$\n"
 File "${FilePath}${FileName}"
!macroend
!define File "!insertmacro File"
 
; Copy files macro
!macro CopyFiles SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 CopyFiles "${SourcePath}" "${DestPath}"
!macroend
!define CopyFiles "!insertmacro CopyFiles"
 
; Rename macro
!macro Rename SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 Rename "${SourcePath}" "${DestPath}"
!macroend
!define Rename "!insertmacro Rename"
 
; CreateDirectory macro
!macro CreateDirectory Path
 CreateDirectory "${Path}"
!macroend
!define CreateDirectory "!insertmacro CreateDirectory"
 
; SetOutPath macro
!macro SetOutPath Path
 SetOutPath "${Path}"
!macroend
!define SetOutPath "!insertmacro SetOutPath"
 
 
Section -openlogfile
SectionEnd

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------

;General
  BrandingText " "
  ;Name and file
  !define /date MyTIMESTAMP "%Y%m%d"
 
   Name "OpenTM2 OTM Markups"
   
   Caption "OpenTM2 OTM Markups ${PRODUCT_VERSION} Setup"
 
   OutFile "OpenTM2-OTMMarkups-${PRODUCT_VERSION}-Setup.exe"

  InstallDir "C:\OTM"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\OpenTM2" ""
  
  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------


;--------------------------------
;Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

;--------------------------------

;Pages


  !insertmacro MUI_PAGE_WELCOME

  Section -check
  SectionEnd

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\OpenTM2" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

ShowInstDetails show
ShowUnInstDetails show
;--------------------------------
;Installer Sections

Section "OpenTM2" SecOpenTM2
  SectionIn RO

  ${SetOutPath} "$INSTDIR"



!include ..\..\build\OtmMarkupTablePlugin_inc.nsi


SectionEnd


Function .onInit

; This part checks whether the Installer is already running

             System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex") i .r1 ?e'
             Pop $R0
             StrCmp $R0 0 +3
             MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
             Abort


; We want to check few things before we actually start installing, first, if OpenTM2 is running

  FindProcDLL::FindProc "OpenTM2.exe"
  IntCmp $R0 1 0 notRunning
    MessageBox MB_OK|MB_ICONEXCLAMATION "OpenTM2 is running. Please close it first" /SD IDOK
    Abort
  notRunning:
  
  
;Then, if we already have it installed, are we trying to install the same or older version

 ;Get version from registry if available
  ReadRegStr $5 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTM2" "DisplayVersion"
       continueInstallation:         
         InitPluginsDir
         GetTempFileName $0
         StrCpy $SIZE "28.2"
         
   
FunctionEnd

Function .onSelChange
FunctionEnd

;--------------------------------
;Descriptions



Section -Post
SectionEnd

;--------------------------------
;Uninstaller Section

Section -closelogfile
 FileClose $UninstLog
 SetFileAttributes "$INSTDIR\${UninstLog}" READONLY|SYSTEM|HIDDEN
SectionEnd
 
Section Uninstall
SectionEnd