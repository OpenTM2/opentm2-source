#------------------------------------------------------------------------------
# OtmGetToolInfo.MAK - Makefile for Startup code and resource DLL
# Copyright (c) 2014, International Business Machines
# Corporation and others.  All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER                                                                  -
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------

build:    $(_OBJEXE)\OtmGetToolInfo.exe

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------								
$(_OBJEXE)\GetToolInfoApp.obj:    	$(_SRC)\plugins\tools\OtmGetToolInfoPlugin\GetToolInfoApp.cpp \
									$(_SRC)\plugins\tools\OtmGetToolInfoPlugin\GetToolInfoApp.h
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmGetToolInfoPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmGetToolInfoPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmGetToolInfoPlugin\$(*B).cpp
								
$(_OBJEXE)\GetToolInfo.obj:	$(_SRC)\plugins\tools\OtmGetToolInfoPlugin\GetToolInfo.cpp \
									$(_SRC)\plugins\tools\OtmGetToolInfoPlugin\GetToolInfo.h
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmGetToolInfoPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmGetToolInfoPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmGetToolInfoPlugin\$(*B).cpp

$(_OBJEXE)\GetToolInfo.res:		$(_SRC)\plugins\tools\OtmGetToolInfoPlugin\GetToolInfo.rc \
									$(_SRC)\plugins\tools\OtmGetToolInfoPlugin\resource.h
	@echo ---- Compiling $(_RC_COMPILER)  /n /Fo $(_OBJEXE)\GetToolInfo.RES $(_SRC)\plugins\tools\OtmGetToolInfoPlugin\GetToolInfo.rc
    @$(_RC_COMPILER)  /n /Fo $(_OBJEXE)\GetToolInfo.RES $(_SRC)\plugins\tools\OtmGetToolInfoPlugin\GetToolInfo.rc >>$(_ERR)
								
GETTOOLINFOROBJS = $(_OBJEXE)\GetToolInfoApp.obj \
					 $(_OBJEXE)\GetToolInfo.obj \
				     $(_OBJEXE)\GetToolInfo.RES

$(_OBJEXE)\OtmGetToolInfo.exe:	$(GETTOOLINFOROBJS)

#------------------------------------------------------------------------------
# Build OtmGetToolInfo.exe
#------------------------------------------------------------------------------
$(_OBJEXE)\OtmGetToolInfo.exe:
    @echo ---- Linking $(_OBJEXE)\OtmGetToolInfo.exe
    @echo ---- Linking $(_OBJEXE)\OtmGetToolInfo.exe >>$(_ERR)
    @echo ---- _LINK_OPTIONS_EXE=$(_LINK_OPTIONS_EXE)
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
    $(GETTOOLINFOROBJS) 
	$(_LINK_OPTIONS_EXE) /subsystem:console /entry:WinMainCRTStartup /STACK:"2000000"
/OUT:$(_OBJEXE)\OtmGetToolInfo.exe
    $(_LINK_LIB_EXE) $(_LIB)\OtmBase.lib $(_LIB)\PluginManager.lib $(_LIB)\PluginMgrAssist.lib version.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\WIN\ md $(RELEASE_DIR)\OTM\WIN
    @copy $(_OBJEXE)\OtmGetToolInfo.exe $(RELEASE_DIR)\OTM\WIN /Y>$(_ERR)


