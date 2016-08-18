#------------------------------------------------------------------------------
# OpenTM2Starter.MAK - Makefile for Startup code and resource DLL
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

build:    $(_OBJEXE)\OpenTM2Starter.exe

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------								
$(_OBJEXE)\PlginPendLst.obj:    	$(_SRC)\plugins\OpenTM2Starter\PlginPendLst.cpp \
									$(_SRC)\plugins\OpenTM2Starter\PlginPendLst.h
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp

$(_OBJEXE)\AvuPendLst.obj:        	$(_SRC)\plugins\OpenTM2Starter\AvuPendLst.cpp \
									$(_SRC)\plugins\OpenTM2Starter\AvuPendLst.h
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp

$(_OBJEXE)\HistoryWriter.obj:	    $(_SRC)\plugins\OpenTM2Starter\HistoryWriter.cpp \
									$(_SRC)\plugins\OpenTM2Starter\HistoryWriter.h
	@echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp

$(_OBJEXE)\TimeManager.obj:	        $(_SRC)\plugins\OpenTM2Starter\TimeManager.cpp \
									$(_SRC)\plugins\OpenTM2Starter\TimeManager.h
	@echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp

$(_OBJEXE)\PlgMgXmlLocParser.obj:	$(_SRC)\plugins\OpenTM2Starter\PlgMgXmlLocParser.cpp \
									$(_SRC)\plugins\OpenTM2Starter\PlgMgXmlLocParser.h
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp

$(_OBJEXE)\OtmXmlLocParser.obj:		$(_SRC)\plugins\OpenTM2Starter\OtmXmlLocParser.cpp \
									$(_SRC)\plugins\OpenTM2Starter\OtmXmlLocParser.h
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp

$(_OBJEXE)\OtmLogWriterLoc.obj:		$(_SRC)\plugins\OpenTM2Starter\OtmLogWriterLoc.cpp \
									$(_SRC)\plugins\OpenTM2Starter\OtmLogWriterLoc.h
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp
								
$(_OBJEXE)\OpenTM2StarterComm.obj:	$(_SRC)\plugins\OpenTM2Starter\OpenTM2StarterComm.cpp \
									$(_SRC)\plugins\OpenTM2Starter\OpenTM2StarterComm.h
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp

$(_OBJEXE)\OpenTM2Starter.obj:	    $(_SRC)\plugins\OpenTM2Starter\OpenTM2Starter.cpp \
									$(_SRC)\plugins\OpenTM2Starter\OpenTM2Starter.h
	@echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp

$(_OBJEXE)\OpenTM2StarterApp.obj:	$(_SRC)\plugins\OpenTM2Starter\OpenTM2StarterApp.cpp \
									$(_SRC)\plugins\OpenTM2Starter\OpenTM2StarterApp.h
	@echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OpenTM2Starter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OpenTM2Starter\$(*B).cpp

$(_OBJEXE)\OpenTM2Starter.res:		$(_SRC)\plugins\OpenTM2Starter\OpenTM2Starter.rc \
									$(_SRC)\plugins\OpenTM2Starter\resource.h \
									$(_SRC)\plugins\OpenTM2Starter\OpenTM2Starter.ico
	@echo ---- Compiling $(_RC_COMPILER)  /n /Fo $(_OBJEXE)\OpenTM2Starter.RES $(_SRC)\plugins\OpenTM2Starter\OpenTM2Starter.rc
    @$(_RC_COMPILER)  /n /Fo $(_OBJEXE)\OpenTM2Starter.RES $(_SRC)\plugins\OpenTM2Starter\OpenTM2Starter.rc >>$(_ERR)
								
OPENTM2STARTEROBJS = $(_OBJEXE)\OpenTM2StarterApp.obj \
                     $(_OBJEXE)\HistoryWriter.obj \
					 $(_OBJEXE)\TimeManager.obj \
					 $(_OBJEXE)\OpenTM2Starter.obj \
					 $(_OBJEXE)\OpenTM2StarterComm.obj \
	                 $(_OBJEXE)\OtmLogWriterLoc.obj \
	                 $(_OBJEXE)\PlgMgXmlLocParser.obj \
	                 $(_OBJEXE)\PlginPendLst.obj \
					 $(_OBJEXE)\AvuPendLst.obj \
					 $(_OBJEXE)\OtmXmlLocParser.obj \
				     $(_OBJEXE)\OpenTM2Starter.RES

$(_OBJEXE)\OpenTM2Starter.exe:	$(OPENTM2STARTEROBJS)

#------------------------------------------------------------------------------
# Build OtmAutoVerUp.exe
#------------------------------------------------------------------------------
$(_OBJEXE)\OpenTM2Starter.exe:
    @echo ---- Linking $(_OBJEXE)\OpenTM2Starter.exe
    @echo ---- Linking $(_OBJEXE)\OpenTM2Starter.exe >>$(_ERR)
    @echo ---- _LINK_OPTIONS_EXE=$(_LINK_OPTIONS_EXE)
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
    $(OPENTM2STARTEROBJS) 
	$(_LINK_OPTIONS_EXE)
/OUT:$(_OBJEXE)\OpenTM2Starter.exe
    $(_LINK_LIB_EXE) $(_LIBOTHER)\xerces-c_3.lib comctl32.lib /STACK:"2000000"
<<
    @if not exist $(RELEASE_DIR)\OTM\WIN\ md $(RELEASE_DIR)\OTM\WIN
    @copy $(_OBJEXE)\OpenTM2Starter.EXE $(RELEASE_DIR)\OTM\WIN /Y>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\PLUGINS\ md $(RELEASE_DIR)\OTM\PLUGINS
    @copy /y $(_SRC)\plugins\OpenTM2Starter\PendingUpdates.conf.sample $(RELEASE_DIR)\OTM\PLUGINS /Y>$(_ERR)

