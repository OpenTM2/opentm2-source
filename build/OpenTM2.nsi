; Copyright (c) 2016-2017 International Business Machines
; Corporation and others.  All rights reserved.

;NSIS Source for OpenTM2 installer  
;Parts taken from NSIS examples
;Put together by Sasha Maric
;Reworked and extended by Holger Haertling 

AllowRootDirInstall true

Var SIZE
!define APPNAME "OTM"
!define PRODUCT_NAME "OpenTM2"
!define PRODUCT_VERSION "1.4.1"
!define PRODUCT_PUBLISHER "OpenTM2"
!define PRODUCT_WEB_SITE "http://www.opentm2.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\OpenTM2Starter.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
;add for multi user
!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
;add end

!define UninstLog "uninstall.log"
Var UninstLog

!include WinMessages.nsh
!include FileFunc.nsh
!include "MUI2.nsh"
;add for multi user
!include MultiUser.nsh
!include "TextFunc.nsh"
;add end
!include WordFunc.nsh
!include LogicLib.nsh
!include EnvVarUpdate.nsh
!insertmacro GetDrives
!insertmacro DriveSpace

LangString PAGE_TITLE ${LANG_ENGLISH} "Select drive..."
LangString PAGE_SUBTITLE ${LANG_ENGLISH} "OpenTM2 can be installed only in these directories on available drives."

LangString PAGE_TITLE_UPDATE ${LANG_ENGLISH} "Installation Directory..."
LangString PAGE_SUBTITLE_UPDATE ${LANG_ENGLISH} "OpenTM2-updates can only be installed into existing OpenTM2 directories."

Function CustomCreate
   
   ReadRegStr $0 HKCU "SOFTWARE\OpenTM2" ""
   StrCmp $0 '' next1 next2
 next1:
   !insertmacro MUI_HEADER_TEXT $(PAGE_TITLE) $(PAGE_SUBTITLE)
   Goto end1
 next2:
   !insertmacro MUI_HEADER_TEXT $(PAGE_TITLE_UPDATE) $(PAGE_SUBTITLE_UPDATE)

 end1:
   StrCpy $R2 0
   StrCpy $R0 ''
   ${GetDrives} "HDD" GetDrivesCallBack
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Settings' 'NumFields' '6'
   
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Type' 'Label'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Left' '5'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Top' '5'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Right' '-6'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Bottom' '17'
   ReadRegStr $0 HKCU "SOFTWARE\OpenTM2" ""
   StrCmp $0 '' next3 next4
 next3:
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Text' \
   'Select Installation Drive:'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Type' 'DropList'
   Goto end2
 next4:
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 1' 'Text' \
   'The already existing Installation Directory:'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Type' 'Label'
 end2:
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Left' '30'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Top' '26'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Right' '-31'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Bottom' '100'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Flags' 'Notify'
   ReadRegStr $0 HKCU "SOFTWARE\OpenTM2" ""
   StrCmp $0 '' next5 next6
 next5:
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'State' '$R1'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'ListItems' '$R0'
   Goto end3
 next6:
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 2' 'Text' \
   '$R0'
 end3:
   
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Type' 'Label'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Left' '5'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Top' '109'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Right' '59'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Bottom' '119'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 3' 'Text' \
   'Space required:'
   
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Type' 'Label'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Left' '60'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Top' '109'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Right' '-5'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Bottom' '119'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 4' 'Text' \
   '$SIZE Mb'
   
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Type' 'Label'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Left' '5'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Top' '120'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Right' '59'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Bottom' '130'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 5' 'Text' \
   'Space available:'
   
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Type' 'Label'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Left' '60'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Top' '120'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Right' '-5'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Bottom' '130'
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Text' \
   '$R3 Mb'
   
   push $0
   InstallOptions::Dialog '$PLUGINSDIR\custom.ini'
   pop $0
   pop $0
         
FunctionEnd
 
Function CustomLeave
   ReadIniStr $0 '$PLUGINSDIR\custom.ini' 'Settings' 'State'
   StrCmp $0 '2' 0 next
   ReadIniStr $0 '$PLUGINSDIR\custom.ini' 'Field 2' 'State'
   StrCpy $0 $0 3
   ${DriveSpace} "$0" "/D=F /S=M" $R3
   WriteIniStr '$PLUGINSDIR\custom.ini' 'Field 6' 'Text' \
   '$R3 Mb'
   ReadIniStr $0 '$PLUGINSDIR\custom.ini' 'Field 6' 'HWND'
   SendMessage $0 ${WM_SETTEXT} 0 'STR:$R3 Mb'
   Abort
next:
   ReadRegStr $1 HKCU "SOFTWARE\OpenTM2" ""
   StrCmp $1 '' next1 next2
 next1:
   ReadIniStr $0 '$PLUGINSDIR\custom.ini' 'Field 2' 'State'
   StrCpy "$0" "$0${APPNAME}"
   Goto end1
 next2:
   ReadIniStr $0 '$PLUGINSDIR\custom.ini' 'Field 2' 'Text'
 end1:
   StrCpy '$INSTDIR' '$0'
FunctionEnd
 
Function GetDrivesCallBack
   ${DriveSpace} "$9" "/D=F /S=M" $R4
   IntCmp $R4 '$SIZE' end end def
def:
   StrCmp $R2 '0' 0 next
   StrCpy $R3 '$R4'
   ;StrCpy $R1 '$9${APPNAME}'
   StrCpy $R1 '$9'
   IntOp $R2 $R2 + 1
next:
   ;StrCpy $R0 '$R0$9${APPNAME}|'
   StrCpy $R0 '$R0$9|'
end:
   ReadRegStr $1 HKCU "SOFTWARE\OpenTM2" ""
   StrCmp $1 '' next1 next2
next2:
   StrCpy $R0 '$1'
next1:
   Push $0
FunctionEnd
 

; Uninstall log file missing.
LangString UninstLogMissing ${LANG_ENGLISH} "${UninstLog} not found!$\r$\nUninstallation cannot proceed!"
 
; AddItem macro
!macro AddItem Path
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define AddItem "!insertmacro AddItem"
 
; File macro
!macro File FilePath FileName
 IfFileExists "$OUTDIR\${FileName}" +2
  FileWrite $UninstLog "$OUTDIR\${FileName}$\r$\n"
 File "${FilePath}${FileName}"
!macroend
!define File "!insertmacro File"
 
; CreateShortcut macro
!macro CreateShortcut FilePath FilePointer
 FileWrite $UninstLog "${FilePath}$\r$\n"
 CreateShortcut "${FilePath}" "${FilePointer}"
!macroend
!define CreateShortcut "!insertmacro CreateShortcut"
 
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
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define CreateDirectory "!insertmacro CreateDirectory"
 
; SetOutPath macro
!macro SetOutPath Path
 SetOutPath "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define SetOutPath "!insertmacro SetOutPath"
 
; WriteUninstaller macro
!macro WriteUninstaller Path
 WriteUninstaller "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define WriteUninstaller "!insertmacro WriteUninstaller"
 
Section -openlogfile
 CreateDirectory "$INSTDIR"
 IfFileExists "$INSTDIR\${UninstLog}" +3
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
 Goto +4
  SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
  FileSeek $UninstLog 0 END
SectionEnd

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------

;General
  BrandingText " "
  ;Name and file
  !define /date MyTIMESTAMP "%Y%m%d"
 
;   Name "OpenTM2 ${PRODUCT_VERSION}"
   Name "OpenTM2"
   
   Caption "OpenTM2 ${PRODUCT_VERSION} Setup"
 
   ;OutFile "OpenTM2 ${PRODUCT_VERSION} Setup-${MyTIMESTAMP}.exe"
   OutFile "OpenTM2-${PRODUCT_VERSION}-Community-Edition.Setup.exe"  
  
  ;Default installation folder

  InstallDir "C:\OTM"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\OpenTM2" ""
  
  ;Request application privileges for Windows Vista
  ;RequestExecutionLevel user

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------


;--------------------------------
;Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; If you want to see all the languages, regardless of code page, uncomment this...
;!define MUI_LANGDLL_ALLLANGUAGES

; Language Selection Dialog Settings
;!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
;!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
;!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"
;--------------------------------

;Pages


  !insertmacro MUI_PAGE_WELCOME
  !define MUI_LICENSEPAGE_CHECKBOX
  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  ;add for multi user
  !insertmacro MULTIUSER_PAGE_INSTALLMODE
  ;add end
  
  ;!insertmacro MUI_PAGE_DIRECTORY

  Page Custom CustomCreate CustomLeave

  Section -check

         StrCpy '$6' ""
         ReadRegStr $6 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTM2" "UninstallString"
        ${If} $6 S== ""

          GoTo krajif

        ${EndIf}

          ${If} $6 S!= "$INSTDIR\Uninstall.exe"

         ; We are trying to install to different root and there is already version installed elsewhere

               MessageBox MB_OK "You are trying to install OpenTM2 to different drive from where it is currently installed! The Installer will now quit." /SD IDOK

              Abort
       ${EndIf}
       krajif:
  SectionEnd
  
  ;Remove fixpack config file when reinstall
  Section -PluginFixpackChk
    IfFileExists "$INSTDIR\PLUGINS\PluginManagerFixp.conf" 0 NextCheck
	Delete "$INSTDIR\PLUGINS\PluginManagerFixp.conf"
  NextCheck:
	IfFileExists "$INSTDIR\PLUGINS\AutoVerUpFixp.conf" 0 EndCheck
	Delete "$INSTDIR\PLUGINS\AutoVerUpFixp.conf"
  EndCheck:
  SectionEnd

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\OpenTM2" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
   !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  !insertmacro MUI_PAGE_INSTFILES
  !define MUI_FINISHPAGE_RUN "$INSTDIR\WIN\OpenTM2Starter.exe"
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

ShowInstDetails show
ShowUnInstDetails show
;--------------------------------
;Installer Sections

Function FileDiffFound
    StrCpy $R9 "NotEqual"
	StrCpy $0 StopTextCompare

	Push $0
FunctionEnd

Section "OpenTM2" SecOpenTM2
  SectionIn RO

  ${SetOutPath} "$INSTDIR"

  ;ADD YOUR OWN FILES HERE...

${CreateDirectory} "$INSTDIR\LIST"
${CreateDirectory} "$INSTDIR\DICT"
${CreateDirectory} "$INSTDIR\MEM"
${CreateDirectory} "$INSTDIR\API"

${SetOutPath} "$INSTDIR\MSG"
${File} "MSG\" "EQFWE.HLP"

${SetOutPath} "$INSTDIR\DOC"
${File} "DOC\" "PluginManager.chm"
${File} "DOC\" "Opentm2TechnicalReference.pdf"
${File} "DOC\" "Opentm2TranslatorsReference.pdf"
${File} "DOC\" "OpenTM2APICalls.pdf"

SetOverwrite ifnewer
${SetOutPath} "$INSTDIR\PROPERTY"
${File} "PROPERTY\" "STANDARD.EDI"
${File} "PROPERTY\" "RTFEDIT.EDI"

SetOverwrite on
${File} "PROPERTY\" "EqfSharedMemoryCreate.Defaults"

; Start control trigger file
IfFileExists "$INSTDIR\PROPERTY\EQFNFLUENT.TRG" 0 Continue1
CopyFiles "$INSTDIR\PROPERTY\EQFNFLUENT.TRG" "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_old"
SetOverwrite ifdiff
${File} "PROPERTY\" "EQFNFLUENT.TRG"
; first compare the content
StrCpy $R9 "Equal"
${TextCompare} "$INSTDIR\PROPERTY\EQFNFLUENT.TRG" "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_old" "FastDiff" "FileDiffFound"
StrCmp $R9 "NotEqual" Continue2 0
${GetTime} "$INSTDIR\PROPERTY\EQFNFLUENT.TRG" "M" $0 $1 $2 $3 $4 $5 $6
${GetTime} "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_old" "M" $R0 $R1 $R2 $R3 $R4 $R5 $R6
${if} $0 != $R0
${OrIf} $1 != $R1
${OrIf} $2 != $R2
${OrIf} $3 != $R3
${OrIf} $4 != $R4
${OrIf} $5 != $R5
${OrIf} $6 != $R6
Continue2:
IfFileExists "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_new" 0 Continue3
Rename "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_new" "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_new_old"
Continue3:
Rename "$INSTDIR\PROPERTY\EQFNFLUENT.TRG" "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_new"
Rename "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_old" "$INSTDIR\PROPERTY\EQFNFLUENT.TRG"
IfFileExists "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_new_old" 0 Continue5
StrCpy $R9 "Equal"
${TextCompare} "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_new" "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_new_old" "FastDiff" "FileDiffFound"
StrCmp $R9 "NotEqual" 0 Continue4
MessageBox MB_OK "An existing EQFNFLUENT.TRG was found on your computer. The new EQFNFLUENT.TRG was installed with a new name EQFNFLUENT.TRG_new." /SD IDOK
Continue4:
Goto Continue6
${Else}
Goto Continue8
${EndIf}
Continue5:
MessageBox MB_OK "An existing EQFNFLUENT.TRG was found on your computer. The new EQFNFLUENT.TRG was installed with a new name EQFNFLUENT.TRG_new." /SD IDOK
Continue8:
IfFileExists "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_old" 0 Continue6
Delete "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_old"
Continue6:
IfFileExists "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_new_old" 0 Continue7
Delete "$INSTDIR\PROPERTY\EQFNFLUENT.TRG_new_old"
Goto Continue7
Continue1:
SetOverwrite on
${File} "PROPERTY\" "EQFNFLUENT.TRG"
Continue7:
;End control trigger file

${SetOutPath} "$INSTDIR\PRTFORM"
${File} "PRTFORM\" "FORMAT1.FRM"
${File} "PRTFORM\" "FORMAT2.FRM"
${File} "PRTFORM\" "FORMAT3.FRM"
${File} "PRTFORM\" "FORMAT4.FRM"
${File} "PRTFORM\" "LONG.FRM"
${File} "PRTFORM\" "SHORT.FRM"

${SetOutPath} "$INSTDIR\TABLE"
${File} "TABLE\" "CALCHTML.XSL"
${File} "TABLE\" "CALCTEXT.XSL"
${File} "TABLE\" "CNT2HTML.XSL"
${File} "TABLE\" "CNTHTML.XSL"
${File} "TABLE\" "CNTTEXT.XSL"
${File} "TABLE\" "DICFORMT.IBL"
${File} "TABLE\" "EQFLMT.IBL"
${File} "TABLE\" "EQFLOGOS.IBL"
${File} "TABLE\" "FORMAT.IBL"
${File} "TABLE\" "HISTHTML.XSL"
${File} "TABLE\" "HISTTEXT.XSL"
${File} "TABLE\" "LISTFORM.IBL"
${File} "TABLE\" "MEMTABLE.IBL"
${File} "TABLE\" "otmmteval.xsl"
IfFileExists "$INSTDIR\TABLE\MTEVAL.XSL" 0 NoMTEVALXSL
Delete "$INSTDIR\TABLE\MTEVAL.XSL"
NoMTEVALXSL:
${File} "TABLE\" "PANELW.LNG"
${File} "TABLE\" "QDPRTAGS.IBL"
${File} "TABLE\" "QFRTF.IBL"
${File} "TABLE\" "QFSHOW.IBL"
${File} "TABLE\" "QFTAGS.IBL"
${File} "TABLE\" "RESLHTML.XSL"
${File} "TABLE\" "RESLTEXT.XSL"
${File} "TABLE\" "RESLUTF16.XSL"
${File} "TABLE\" "TLIST.IBL"
${File} "TABLE\" "VALDOC2HTML.XSL"
${File} "TABLE\" "VALDOC3HTML.XSL"
${File} "TABLE\" "TMXNAMES.LST"
${File} "TABLE\" "OtmCleanup.lst"
${File} "TABLE\" "languages.xml"
IfFileExists "$INSTDIR\DOC\eqfr5mst.PDF" 0 NoOldDocFile
Delete "$INSTDIR\DOC\eqfr5mst.PDF"
NoOldDocFile:
IfFileExists "$INSTDIR\TABLE\PROPERTY.LNG" 0 NoPropertyLng
Rename "$INSTDIR\TABLE\PROPERTY.LNG" "$INSTDIR\TABLE\PROPERTY.LNG.DONOTUSE"
NoPropertyLng:
IfFileExists "$INSTDIR\TABLE\SOURCE.LNG" 0 NoSourceLng
Rename "$INSTDIR\TABLE\SOURCE.LNG" "$INSTDIR\TABLE\SOURCE.LNG.DONOTUSE"
NoSourceLng:
IfFileExists "$INSTDIR\TABLE\TARGET.LNG" 0 NoTargetLng
Rename "$INSTDIR\TABLE\TARGET.LNG" "$INSTDIR\TABLE\TARGET.LNG.DONOTUSE"
NoTargetLng:

${SetOutPath} "$INSTDIR\WIN"
${File} "WIN\" "EQF_API.DLL"
${File} "WIN\" "OTMAPI.DLL"
${File} "WIN\" "OTMTMXIE.DLL"
${File} "WIN\" "OTMXLFMT.DLL"
${File} "WIN\" "OTMBASE.DLL"
${File} "WIN\" "OTMDLL.DLL"
${File} "WIN\" "PluginManager.DLL"
${File} "WIN\" "PluginMgrAssist.DLL"
${File} "WIN\" "OTMFUNC.DLL"
${File} "WIN\" "OTMGetReportData.EXE"
${File} "WIN\" "OTMLOGOR.DLL"
${File} "WIN\" "OTMITMD.DLL"
${File} "WIN\" "MessagesE.DLL"
${File} "WIN\" "OTMqdam.dll"
${File} "WIN\" "OTMRESWE.DLL"
${File} "WIN\" "msvcr100.dll"
${File} "WIN\" "msvcp100.dll"
${File} "WIN\" "mfc100u.dll"  ;add for PMR402940
${File} "WIN\" "MSVCR71.DLL"
${File} "WIN\" "OpenTM2.EXE"
${File} "WIN\" "OpenTM2Starter.EXE"
${File} "WIN\" "OtmBatch.EXE"
${File} "WIN\" "OpenTM2Scripter.EXE"
${File} "WIN\" "MemoryWebServiceClient.DLL"
${File} "WIN\" "OtmItm.EXE"
${File} "WIN\" "standard.dll"
${File} "WIN\" "rtfedit.dll"
${File} "WIN\" "Xalan-C_1_10.dll"
${File} "WIN\" "XalanMessages_1_10.dll"
${File} "WIN\" "XalanTransform.exe"
${File} "WIN\" "xerces-c_2_7.dll"
${File} "WIN\" "xerces-c_3_1.dll"
${File} "WIN\" "libcurl.dll"
${File} "WIN\" "zlibwapi.dll"
${File} "WIN\" "zlib1.dll"
${File} "WIN\" "libeay32.dll"
${File} "WIN\" "libssh2.dll"
${File} "WIN\" "ssleay32.dll"
${File} "WIN\" "MemoryWebServiceClient.DLL"
${File} "WIN\" "OtmAutoVerUp.exe"
${File} "WIN\" "OpenTM2Starter.exe"
${File} "WIN\" "OtmGetToolInfo.exe"
${File} "WIN\" "OtmSetToolPathEnv.exe"
${File} "WIN\" "OpenTM2ToolsLauncher.exe"
${File} "WIN\" "OTMGlobM.DLL"
${File} "WIN\" "icudt32.DLL"
${File} "WIN\" "icuin32.DLL"
${File} "WIN\" "icuuc32.DLL"
${File} "WIN\" "icuuc50.DLL"
${File} "WIN\" "icudt50.DLL"
${File} "WIN\" "icuuc51.DLL"
${File} "WIN\" "icudt51.DLL"
${File} "WIN\" "OtmTmxSource2Text.EXE"
${File} "WIN\" "OtmTmxSplitSegments.EXE"
${File} "WIN\" "OtmChangeFxp.EXE"
${File} "WIN\" "OtmShowFxp.EXE"
${File} "WIN\" "OtmAdl.EXE"
${File} "WIN\" "OtmMtEval.EXE"
${File} "WIN\" "OtmChkCalc.EXE"
${File} "WIN\" "OtmCreateITMFromMemory.EXE"
${File} "WIN\" "OtmRemoveTags.EXE"
${File} "WIN\" "OtmTmx2Exp.EXE" 
${File} "WIN\" "OtmExp2Tmx.EXE" 
${File} "WIN\" "OtmXliff2Exp.EXE" 
${File} "WIN\" "OtmMemoryTool.EXE" 
${File} "WIN\" "OtmIsOpenTM2FXP.EXE" 
${File} "WIN\" "TM2OTMMigrator.EXE"
${File} "WIN\" "OpenTMSHelper.jar"
${File} "WIN\" "EQFXLIFF.DLL"
${File} "WIN\" "OtmMemoryService.EXE"
${File} "WIN\" "OtmMemoryServiceGUI.EXE"
IfFileExists "$INSTDIR\WIN\OtmMemoryService.conf" OMSConfExists 0
${File} "WIN\" "OtmMemoryService.conf"
OMSConfExists:

${SetOutPath} "$INSTDIR\API"
${File} "API\" "OTMFUNC.LIB"
${File} "API\" "OTMFUNC.H"
${File} "API\" "EQFPAPI.H"

${SetOutPath} "$INSTDIR\PLUGINS"
${File} "PLUGINS\" "EqfDictionaryPlugin.DLL"
${File} "PLUGINS\" "EqfDocumentPlugin.DLL"
${File} "PLUGINS\" "EqfMemoryPlugin.DLL"
${File} "PLUGINS\" "EqfSharedMemPlugin.DLL"
${File} "PLUGINS\" "EqfSharedOnLanMemoryPlugin.DLL"
${File} "PLUGINS\" "OtmMorphICUPlugin.DLL"
${File} "PLUGINS\" "PluginManager.conf.sample"
; merge sample file's value to local config file
IfFileExists "$INSTDIR\PLUGINS\PluginManager.conf" 0 EndCheck1
ReadIniStr $7 '$INSTDIR\PLUGINS\PluginManager.conf.sample' 'Networks' 'URL'
WriteIniStr '$INSTDIR\PLUGINS\PluginManager.conf' 'Networks' 'URL' $7
ReadIniStr $7 '$INSTDIR\PLUGINS\PluginManager.conf.sample' 'BasicPlugins' 'Name'
WriteIniStr '$INSTDIR\PLUGINS\PluginManager.conf' 'BasicPlugins' 'Name' $7
ReadIniStr $7 '$INSTDIR\PLUGINS\PluginManager.conf.sample' 'BasicPlugins' 'MinCnt'
WriteIniStr '$INSTDIR\PLUGINS\PluginManager.conf' 'BasicPlugins' 'MinCnt' $7
ReadIniStr $7 '$INSTDIR\PLUGINS\PluginManager.conf.sample' 'NonRemovablePlugins' 'Name'
WriteIniStr '$INSTDIR\PLUGINS\PluginManager.conf' 'NonRemovablePlugins' 'Name' $7
EndCheck1:
${File} "PLUGINS\" "AutoVersionUp.conf.sample"
; merge sample file's URL to local config file
IfFileExists "$INSTDIR\PLUGINS\AutoVersionUp.conf" 0 EndCheckAutoVersionUp
ReadIniStr $7 '$INSTDIR\PLUGINS\AutoVersionUp.conf.sample' 'Networks' 'URL'
WriteIniStr '$INSTDIR\PLUGINS\AutoVersionUp.conf' 'Networks' 'URL' $7
EndCheckAutoVersionUp:
${File} "PLUGINS\" "PendingUpdates.conf.sample"
${File} "PLUGINS\" "OtmCleanupPlugin.DLL"

${File} "PLUGINS\" "OtmToolsLauncherPlugin.DLL"

${File} "PLUGINS\" "OtmProfileMgrPlugin.Dll"

${SetOutPath} "$INSTDIR\PLUGINS\OtmSpellHSPlugin"
${File} "PLUGINS\OtmSpellHSPlugin\" "OtmSpellHSPlugin.DLL"
${File} "PLUGINS\OtmSpellHSPlugin\" "LanguageConfig.lng"


${SetOutPath} "$INSTDIR\PLUGINS\UserMarkupTablePlugin"
${File} "PLUGINS\UserMarkupTablePlugin\" "UserMarkupTablePlugin.DLL"

!include ..\..\build\OtmMarkupTablePlugin_inc.nsi

${SetOutPath} "$INSTDIR\OtmTMService"
${File} "OtmTMService\" "OtmTMService.jar"
${SetOutPath} "$INSTDIR\OtmTMService\configure"
${File} "OtmTMService\configure\" "service_cfg.xml"
${File} "OtmTMService\configure\" "log4j.properties"
${SetOutPath} "$INSTDIR\OtmTMService\lib"
${File} "OtmTMService\lib\" "c3p0-0.9.2.jar"
${File} "OtmTMService\lib\" "dom4j-1.6.1.jar"
${File} "OtmTMService\lib\" "flexjson-2.1.jar"
${File} "OtmTMService\lib\" "jdom-1.1.3.jar"
${File} "OtmTMService\lib\" "log4j-1.2.15.jar"
${File} "OtmTMService\lib\" "mchange-commons-java-0.2.3.3.jar"
${File} "OtmTMService\lib\" "mysql-connector-java-5.1.7-bin.jar"
${File} "OtmTMService\lib\" "mariadb-java-client-1.5.6.jar"
${SetOutPath} "$INSTDIR\OtmTMService\MANIFEST"
${File} "OtmTMService\MANIFEST\" "MANIFEST.MF"

${SetOutPath} "$INSTDIR\OpenTM2ScripterGUI"
${File} "OpenTM2ScripterGUI\" "OpenTM2ScripterGUI.jar"
${File} "OpenTM2ScripterGUI\" "configuration.conf"
${SetOutPath} "$INSTDIR\OpenTM2ScripterGUI\libs"
${File} "OpenTM2ScripterGUI\libs\" "antlr-3.4-complete.jar"
${File} "OpenTM2ScripterGUI\libs\" "jdom.jar"
${File} "OpenTM2ScripterGUI\libs\" "miglayout15-swing.jar"
${SetOutPath} "$INSTDIR\OpenTM2ScripterGUI\MANIFEST"
${File} "OpenTM2ScripterGUI\MANIFEST\" "MANIFEST.MF"
${SetOutPath} "$INSTDIR\OpenTM2ScripterGUI\resources"
${File} "OpenTM2ScripterGUI\resources\" "commands.xml"
${File} "OpenTM2ScripterGUI\resources\" "OpenTM2Version.info"
${SetOutPath} "$INSTDIR\OpenTM2ScripterGUI\resources\icons"
${File} "OpenTM2ScripterGUI\resources\icons\" "apicall.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "arrow-down.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "arrow-up.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "block.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "checkbox.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "clear.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "close.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "close_tab.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "cog_edit.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "comment.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "copy.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "cut.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "define.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "delete.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "editor.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "editor_style_big.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "error.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "file.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "folder-new.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "folder.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "function.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "help.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "import.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "move.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "new.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "open.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "openlog.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "options.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "paste.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "rename.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "report.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "run.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "save.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "save_as.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "script.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "settings.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "settings_big.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "start.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "stop.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "terminal.png"
${File} "OpenTM2ScripterGUI\resources\icons\" "unknown.png"

${CreateDirectory} "$INSTDIR\OpenTM2ScripterGUI\single_scripts"
${CreateDirectory} "$INSTDIR\OpenTM2ScripterGUI\test_suites"

${SetOutPath} "$INSTDIR\PLUGINS\OtmSpellHSPlugin\DICT"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "af_ZA.dic"
${File} "PLUGINS\\OtmSpellHSPlugin\DICT\" "af_ZA.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "ar.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "ar.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "be_BY.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "be_BY.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "bg_BG.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "bg_BG.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "catalan.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "catalan.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "cs_CZ.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "cs_CZ.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "da_DK.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "da_DK.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "de_DE_frami.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "de_DE_frami.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "el_GR.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "el_GR.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "en-GB.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "en-GB.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "en_AU.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "en_AU.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "en_US.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "en_US.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "es_ES.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "es_ES.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "et_EE.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "et_EE.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "eu.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "eu.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "fr-moderne.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "fr-moderne.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "fr-CA.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "fr-CA.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "he_IL.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "he_IL.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "hr_HR.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "hr_HR.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "hu_HU.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "hu_HU.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "is_IS.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "is_IS.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "it_IT.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "it_IT.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "kk_KZ.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "kk_KZ.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "ko-KR.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "ko-KR.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "lt.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "lt.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "lv_LV.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "lv_LV.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "mk_MK.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "mk_MK.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "nb_NO.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "nb_NO.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "nl_NL.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "nl_NL.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "pl_PL.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "pl_PL.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "pt_BR.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "pt_BR.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "pt_PT.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "pt_PT.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "ro_RO.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "ro_RO.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "ru_RU.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "ru_RU.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "sk_SK.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "sk_SK.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "sr.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "sr.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "sv_SE.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "sv_SE.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "th_TH.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "th_TH.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "tr_TR.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "tr_TR.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "uk_UA.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "uk_UA.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "az-Latn-AZ.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "az-Latn-AZ.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "cy_GB.dic"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "cy_GB.aff"
${File} "PLUGINS\OtmSpellHSPlugin\DICT\" "cy-GB_LICENSE.txt"

${SetOutPath} "$INSTDIR\PLUGINS\OtmMorphICUPlugin\Rules"
${File} "PLUGINS\OtmMorphICUPlugin\Rules\" "english-otm-rules.txt"
${File} "PLUGINS\OtmMorphICUPlugin\Rules\" "english-otm-rules.brk"

${SetOutPath} "$INSTDIR\PLUGINS\OtmMorphICUPlugin\AbbrevLists"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Catalan_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Czech_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Danish_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Dutch(permissive)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Dutch(restrictive)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "English(U.K.)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "English(U.S.)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Finnish_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "French(Canadian)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "French(national)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "German(DPAnat)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "German(reform)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "German(Swiss)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Greek_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Hebrew_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Italian_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Norwegian(Bokmal)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Norwegian(Nynorsk)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Polish_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Portuguese(Br.New)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Portuguese(Brasil)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Portuguese(nat.)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Portuguese(nt.New)_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Russian_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Spanish_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Swedish_abbrev.dic"
${File} "PLUGINS\OtmMorphICUPlugin\AbbrevLists\" "Thai_abbrev.dic"

${SetOutPath} "$INSTDIR\PLUGINS\OtmProofReadImport"
${File} "PLUGINS\OtmProofReadImport\" "OtmProofReadImportPlugin.DLL"

${SetOutPath} "$INSTDIR\PLUGINS\OtmProofReadImport\filter"
${File} "PLUGINS\OtmProofReadImport\filter\" "ValDocXML.DLL"
${File} "PLUGINS\OtmProofReadImport\filter\" "ValDocPwb.DLL"
${File} "PLUGINS\OtmProofReadImport\filter\" "ValDocDocx.DLL"
${File} "PLUGINS\OtmProofReadImport\filter\" "UNZIP.EXE"

${SetOutPath} "$INSTDIR\PLUGINS\OtmProofReadImport\filter\ValDocXML"
${File} "PLUGINS\OtmProofReadImport\filter\ValDocXML\" "ValDocXML.XSL"

;Store installation folder
WriteRegStr HKCU "Software\OpenTM2" "" $INSTDIR

;Create uninstaller
${WriteUninstaller} "$INSTDIR\Uninstall.exe"
  
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
;Create shortcuts
${CreateDirectory} "$SMPROGRAMS\$StartMenuFolder"
${SetOutPath} "$INSTDIR\WIN"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTM2.lnk" "$INSTDIR\WIN\OpenTM2Starter.exe"
${CreateShortCut} "$DESKTOP\OpenTM2.lnk" "$INSTDIR\WIN\OpenTM2Starter.exe"
${CreateShortCut} "$QUICKLAUNCH\OpenTM2.lnk" "$INSTDIR\WIN\OpenTM2Starter.exe"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\Initial Translation Memory.lnk" "$INSTDIR\WIN\OtmItm.exe"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\PluginManager Documentation.lnk" "$INSTDIR\DOC\PluginManager.chm"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTM2 API Call Reference.lnk" "$INSTDIR\DOC\OpenTM2APICalls.PDF"
;Add for P403213 start
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTM2 Translator's Reference.lnk" "$INSTDIR\DOC\Opentm2TranslatorsReference.pdf"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTM2 Technical Reference.lnk" "$INSTDIR\DOC\Opentm2TechnicalReference.pdf"
;Add end
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTM2 Official WebSite.lnk" "https://sites.google.com/site/opentm2/home"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTM2 Wiki.lnk" "http://www.beo-doc.de/opentm2wiki/index.php/Main_Page"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTM2 SVN Repository.lnk" "http://145.253.107.23/svn/opentm2/"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTM2 Trac System.lnk" "http://145.253.107.23:8000/opentm2/"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\Uninstall OpenTM2.lnk" "$INSTDIR\Uninstall.exe"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTMS Helper.lnk" "$INSTDIR\WIN\OpenTMSHelper.jar"
${SetOutPath} "$INSTDIR\OpenTM2ScripterGUI"
${CreateShortCut} "$SMPROGRAMS\$StartMenuFolder\OpenTM2Scripter IDE.lnk" "$INSTDIR\OpenTM2ScripterGUI\OpenTM2ScripterGUI.jar"
${CreateShortCut} "$DESKTOP\Open TM2.lnk" "$SMPROGRAMS\$StartMenuFolder"
  
!insertmacro MUI_STARTMENU_WRITE_END
  
SectionEnd

Var bAddSamples
Section "Samples" SecSamples
  
${SetOutPath} "$INSTDIR"

;ADD YOUR OWN FILES HERE...

${CreateDirectory} "$INSTDIR\EXPORT"
${SetOutPath} "$INSTDIR\EXPORT"
${File} "PROPERTY\" "ShowMeHtml.FXP"

;Copy files for samples start
${SetOutPath} "$INSTDIR\DICT"
${File} "DICT\" "SHOWM000.ASD"
${File} "DICT\" "SHOWM000.ASI"

${SetOutPath} "$INSTDIR\MEM"
${File} "MEM\" "SHOWM000.TMD"
${File} "MEM\" "SHOWM000.TMI"

${SetOutPath} "$INSTDIR\PROPERTY"
${File} "PROPERTY\" "SHOWM000.F00"
${File} "PROPERTY\" "SHOWM000.MEM"
${File} "PROPERTY\" "SHOWM000.PRO"

${CreateDirectory} "$INSTDIR\SHOWM000.F00"
${CreateDirectory} "$INSTDIR\SHOWM000.F00\PROPERTY"
${CreateDirectory} "$INSTDIR\SHOWM000.F00\RTF"
${CreateDirectory} "$INSTDIR\SHOWM000.F00\SOURCE"
${CreateDirectory} "$INSTDIR\SHOWM000.F00\SSOURCE"
${CreateDirectory} "$INSTDIR\SHOWM000.F00\STARGET"
${CreateDirectory} "$INSTDIR\SHOWM000.F00\TARGET"

${SetOutPath} "$INSTDIR\SHOWM000.F00\PROPERTY"
${File} "SHOWM000.F00\PROPERTY\" "HISTLOG.DAT"
${File} "SHOWM000.F00\PROPERTY\" "SHOWMEHT.000"

${SetOutPath} "$INSTDIR\SHOWM000.F00\SOURCE"
${File} "SHOWM000.F00\SOURCE\" "SHOWMEHT.000"

StrCpy $bAddSamples 1

;Copy files for samples end

SectionEnd

Function .onInit
  ; add for multi user
  !insertmacro MULTIUSER_INIT
  StrCpy $bAddSamples 0
  ; add end

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
  FindProcDLL::FindProc "OpenTM2Starter.exe"
  IntCmp $R0 1 0 notRunning
    MessageBox MB_OK|MB_ICONEXCLAMATION "OpenTM2Starter is running. Please close it first" /SD IDOK
    Abort
  notRunning:
  
;Then, if we already have it installed, are we trying to install the same or older version

 ;Get version from registry if available
  ReadRegStr $5 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenTM2" "DisplayVersion"
       ${If} $5 == "0.04"
	     ; this version used a different version string format
	     Goto continueInstallation
       ${EndIf}
       ${If} $5 == ${PRODUCT_VERSION}
	     ; allow re-install over current version
	     Goto continueInstallation
       ${EndIf}
       ${If} $5 S>= ${PRODUCT_VERSION}

         ; this version is OLDER

              MessageBox MB_OK|MB_ICONSTOP "   You are trying to install same or older version of OpenTM2! $\nIf you really want to do this, first uninstall the current version$\n$\n$\tThe Installer will now quit." /SD IDOK

              Abort
       ${EndIf}
       continueInstallation:         
         InitPluginsDir
         GetTempFileName $0
         Rename $0 '$PLUGINSDIR\custom.ini'
         StrCpy $SIZE "28.2"
         !insertmacro MUI_LANGDLL_DISPLAY
         
   
FunctionEnd

Function un.onInit
  !insertmacro MULTIUSER_UNINIT
FunctionEnd

Function .onSelChange

 ; Logic for size calculation if we checked the Samples

   SectionGetFlags ${SecSamples} $R0
    IntOp $R0 $R0 & ${SF_SELECTED}
    IntCmp $R0 ${SF_SELECTED} dodaj
    StrCpy $SIZE "28.0"
    Goto kraj
    dodaj:
    StrCpy $SIZE "28.2"
    kraj:

FunctionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecOpenTM2 ${LANG_ENGLISH} "OpenTM2 Base system"
  LangString DESC_SecSamples ${LANG_ENGLISH} "OpenTM2 Samples"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecOpenTM2} $(DESC_SecOpenTM2)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSamples} $(DESC_SecSamples)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END


Section -Post
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\WIN\OpenTM2Starter.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\WIN\OpenTM2Starter.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  ;modify for multiuser start
  ;${EnvVarUpdate} $0 "PATH" "A" "HKCU" "$INSTDIR\WIN"
  ExecWait '"$INSTDIR\WIN\OtmSetToolPathEnv.exe" /install:1 /mode:$MultiUser.InstallMode /backup /instdir:"$INSTDIR\WIN"'  $0
  
  IntCmp $bAddSamples 1 0 donothing
  ExecWait '"$INSTDIR\win\OtmADL.exe" /DIC=ShowMeDictHtml /MEM=ShowMeMemHtml /FOL=ShowMeHtml' $0
  donothing:

  ;modify for multiuser end
  ;Exec "$INSTDIR\win\OtmADL.exe /DIC=ShowMeDictHtml /MEM=ShowMeMemHtml /FOL=ShowMeHtml" 
SectionEnd

;--------------------------------
;Uninstaller Section

Section -closelogfile
 FileClose $UninstLog
 SetFileAttributes "$INSTDIR\${UninstLog}" READONLY|SYSTEM|HIDDEN
SectionEnd
 
Section Uninstall

  FindProcDLL::FindProc "OpenTM2.exe"
IntCmp $R0 1 0 notRunning
    MessageBox MB_OK|MB_ICONEXCLAMATION "OpenTM2 is running. Please close it first" /SD IDOK
    Abort
  FindProcDLL::FindProc "OpenTM2Starter.exe"
IntCmp $R0 1 0 notRunning
    MessageBox MB_OK|MB_ICONEXCLAMATION "OpenTM2Starter is running. Please close it first" /SD IDOK
    Abort
notRunning:
 
 ; Can't uninstall if uninstall log is missing!
 IfFileExists "$INSTDIR\${UninstLog}" +3
  MessageBox MB_OK|MB_ICONSTOP "$(UninstLogMissing)"
   Abort
 
 ;add for multiuser start
 ;remove the path of OTM from the environment
 ExecWait '"$INSTDIR\WIN\OtmSetToolPathEnv.exe" /install:0 /instdir:"$INSTDIR\WIN"' $0
 ;add for multiuser end
 
 Push $R0
 Push $R1
 Push $R2
 SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
 FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
 StrCpy $R1 -1
 
 GetLineCount:
  ClearErrors
  FileRead $UninstLog $R0
  IntOp $R1 $R1 + 1
  StrCpy $R0 $R0 -2
  Push $R0   
  IfErrors 0 GetLineCount
 
 Pop $R0
 
 LoopRead:
  StrCmp $R1 0 LoopDone
  Pop $R0
 
  IfFileExists "$R0\*.*" 0 +3
   RMDir $R0  #is dir
  Goto +3
  IfFileExists $R0 0 +2
   Delete $R0 #is file
 
  IntOp $R1 $R1 - 1
  Goto LoopRead
 LoopDone:
 FileClose $UninstLog
 Delete "$INSTDIR\${UninstLog}"
 RMDir "$INSTDIR"
 Pop $R2
 Pop $R1
 Pop $R0

  DeleteRegKey /ifempty HKCU "Software\OpenTM2"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  ;modify for multiuser start
  ;${un.EnvVarUpdate} $0 "PATH" "R" "HKCU" "$INSTDIR\WIN"
  ;modify for multiuser end

SectionEnd
