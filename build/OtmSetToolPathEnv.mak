#------------------------------------------------------------------------------
# OtmGetToolInfo.MAK - Makefile for Startup code and resource DLL
# Copyright (c) 2015, International Business Machines
# Corporation and others.  All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER                                                                  -
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------

build:    $(_OBJEXE)\OtmSetToolPathEnv.exe

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------								
$(_OBJEXE)\SetToolPathEnvApp.obj:    	$(_SRC)\plugins\tools\OtmSetToolPathEnv\SetToolPathEnvApp.cpp \
									    $(_SRC)\plugins\tools\OtmSetToolPathEnv\SetToolPathEnvApp.h
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmSetToolPathEnv\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmSetToolPathEnv\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmSetToolPathEnv\$(*B).cpp
								
$(_OBJEXE)\SetToolPathEnv.obj:	$(_SRC)\plugins\tools\OtmSetToolPathEnv\SetToolPathEnv.cpp \
						        $(_SRC)\plugins\tools\OtmSetToolPathEnv\SetToolPathEnv.h
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmSetToolPathEnv\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmSetToolPathEnv\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmSetToolPathEnv\$(*B).cpp

$(_OBJEXE)\OtmLogWriterEnv.obj:	$(_SRC)\plugins\tools\OtmSetToolPathEnv\OtmLogWriterEnv.cpp \
						        $(_SRC)\plugins\tools\OtmSetToolPathEnv\OtmLogWriterEnv.h
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmSetToolPathEnv\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmSetToolPathEnv\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmSetToolPathEnv\$(*B).cpp

$(_OBJEXE)\SetToolPathEnv.res:		$(_SRC)\plugins\tools\OtmSetToolPathEnv\SetToolPathEnv.rc \
									$(_SRC)\plugins\tools\OtmSetToolPathEnv\resource.h
	@echo ---- Compiling $(_RC_COMPILER)  /n /Fo $(_OBJEXE)\SetToolPathEnv.RES $(_SRC)\plugins\tools\OtmSetToolPathEnv\SetToolPathEnv.rc
    @$(_RC_COMPILER)  /n /Fo $(_OBJEXE)\SetToolPathEnv.RES $(_SRC)\plugins\tools\OtmSetToolPathEnv\SetToolPathEnv.rc >>$(_ERR)
								
SETTOOLPATHENVOBJS = $(_OBJEXE)\SetToolPathEnvApp.obj \
					 $(_OBJEXE)\SetToolPathEnv.obj \
					 $(_OBJEXE)\OtmLogWriterEnv.obj \
				     $(_OBJEXE)\SetToolPathEnv.RES

$(_OBJEXE)\OtmSetToolPathEnv.exe:	$(SETTOOLPATHENVOBJS)

#------------------------------------------------------------------------------
# Build OtmSetToolPathEnv.exe
#------------------------------------------------------------------------------
$(_OBJEXE)\OtmSetToolPathEnv.exe:
    @echo ---- Linking $(_OBJEXE)\OtmSetToolPathEnv.exe
    @echo ---- Linking $(_OBJEXE)\OtmSetToolPathEnv.exe >>$(_ERR)
    @echo ---- _LINK_OPTIONS_EXE=$(_LINK_OPTIONS_EXE)
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
    $(SETTOOLPATHENVOBJS) 
	$(_LINK_OPTIONS_EXE) /subsystem:console /entry:WinMainCRTStartup /STACK:"2000000"
/OUT:$(_OBJEXE)\OtmSetToolPathEnv.exe
    $(_LINK_LIB_EXE) $(_LIB)\OtmBase.lib comctl32.lib shell32.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\WIN\ md $(RELEASE_DIR)\OTM\WIN
    @copy $(_OBJEXE)\OtmSetToolPathEnv.exe $(RELEASE_DIR)\OTM\WIN /Y>$(_ERR)


