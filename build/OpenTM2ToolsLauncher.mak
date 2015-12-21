#------------------------------------------------------------------------------
# OpenTM2ToolsLauncher.MAK - Makefile for Startup code and resource DLL
# Copyright (c) 2015, International Business Machines
# Corporation and others.  All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER                                                                  -
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# Variable Defination                                                                 -
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------

build:    $(_OBJEXE)\OpenTM2ToolsLauncher.exe

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------
$(_OBJEXE)\Commons.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\Commons.cpp \
						   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\Commons.cpp
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp

$(_OBJEXE)\TabCtrlOwn.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\TabCtrlOwn.cpp \
							  $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\TabCtrlOwn.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp


$(_OBJEXE)\OpenTM2ToolsLauncher.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\OpenTM2ToolsLauncher.cpp \
							            $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\OpenTM2ToolsLauncher.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	

$(_OBJEXE)\OpenTM2ToolsLauncherDlg.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\OpenTM2ToolsLauncherDlg.cpp \
							              $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\OpenTM2ToolsLauncherDlg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp


$(_OBJEXE)\DlgAdl.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgAdl.cpp \
						  $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgAdl.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgExp2Tmx.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgExp2Tmx.cpp \
							  $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgExp2Tmx.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgChangeFxp.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgChangeFxp.cpp \
							  $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgChangeFxp.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgChkCalc.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgChkCalc.cpp \
							  $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgChkCalc.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgItmFm.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgItmFm.cpp \
							$(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgItmFm.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgMtEval.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgMtEval.cpp \
							 $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgMtEval.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgTmxSplitSeg.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgTmxSplitSeg.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgTmxSplitSeg.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgTmx2Exp.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgTmx2Exp.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgTmx2Exp.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgRemoveTags.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgRemoveTags.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgRemoveTags.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	

$(_OBJEXE)\DlgShowFxp.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgShowFxp.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgShowFxp.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp


$(_OBJEXE)\DlgTmx2Text.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgTmx2Text.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgTmx2Text.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp


$(_OBJEXE)\DlgXliff2Exp.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgXliff2Exp.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgXliff2Exp.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp

	
$(_OBJEXE)\DlgMigrator.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgMigrator.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgMigrator.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	

$(_OBJEXE)\DlgGetToolInfo.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgGetToolInfo.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgGetToolInfo.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp

$(_OBJEXE)\DlgGetReportData.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgGetReportData.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgGetReportData.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgMemoryTool.obj: $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgMemoryTool.cpp \
							   $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgMemoryTool.h 
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\DlgToolLauncher.obj:    $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgToolLauncher.cpp \
							       $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\DlgToolLauncher.h
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE)  /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\$(*B).cpp
	
$(_OBJEXE)\OpenTM2ToolsLauncher.RES:	$(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\OpenTM2ToolsLauncher.rc \
						                $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\res\OpenTM2ToolsLauncher.rc2 \
						                $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\resource.h 
    @$(_RC_COMPILER)  /n /Fo $(_OBJEXE)\OpenTM2ToolsLauncher.RES  $(_SRC)\plugins\Tools\OtmToolsLauncherPlugin\OpenTM2ToolsLauncher\OpenTM2ToolsLauncher.rc >>$(_ERR)

OpenTM2ToolsLauncherOBJS = $(_OBJEXE)\Commons.obj \
			               $(_OBJEXE)\TabCtrlOwn.obj \
			               $(_OBJEXE)\OpenTM2ToolsLauncher.obj \
			               $(_OBJEXE)\OpenTM2ToolsLauncherDlg.obj \
			               $(_OBJEXE)\DlgAdl.obj \
			               $(_OBJEXE)\DlgExp2Tmx.obj \
						   $(_OBJEXE)\DlgChangeFxp.obj \
						   $(_OBJEXE)\DlgChkCalc.obj \
						   $(_OBJEXE)\DlgItmFm.obj \
						   $(_OBJEXE)\DlgMtEval.obj \
						   $(_OBJEXE)\DlgTmxSplitSeg.obj \
						   $(_OBJEXE)\DlgRemoveTags.obj \
						   $(_OBJEXE)\DlgTmx2Exp.obj \
						   $(_OBJEXE)\DlgShowFxp.obj \
						   $(_OBJEXE)\DlgTmx2Text.obj \
						   $(_OBJEXE)\DlgXliff2Exp.obj \
						   $(_OBJEXE)\DlgMigrator.obj \
						   $(_OBJEXE)\DlgGetToolInfo.obj \
						   $(_OBJEXE)\DlgGetReportData.obj \
						   $(_OBJEXE)\DlgMemoryTool.obj \
			               $(_OBJEXE)\DlgToolLauncher.obj \
	                       $(_OBJEXE)\OpenTM2ToolsLauncher.RES 

$(_OBJEXE)\OpenTM2ToolsLauncher.exe:	$(OpenTM2ToolsLauncherOBJS)


#------------------------------------------------------------------------------
# Build OpenTM2ToolsLauncher.exe
#------------------------------------------------------------------------------
$(_OBJEXE)\OpenTM2ToolsLauncher.exe:
    @echo ---- Linking $(_OBJEXE)\OpenTM2ToolsLauncher.exe
    @echo ---- Linking $(_OBJEXE)\OpenTM2ToolsLauncher.exe >>$(_ERR)
    @echo ---- _LINK_OPTIONS_EXE=$(_LINK_CPP_OPTIONS)
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
    $(OpenTM2ToolsLauncherOBJS) 
	$(_LINK_CPP_OPTIONS)  /entry:WinMainCRTStartup 
	
/OUT:$(_OBJEXE)\OpenTM2ToolsLauncher.exe
<<
    @if not exist $(RELEASE_DIR)\OTM\WIN\ md $(RELEASE_DIR)\OTM\WIN
    @copy $(_OBJEXE)\OpenTM2ToolsLauncher.EXE $(RELEASE_DIR)\OTM\WIN /Y>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\PLUGINS\ md $(RELEASE_DIR)\OTM\PLUGINS
