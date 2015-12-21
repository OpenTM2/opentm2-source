#------------------------------------------------------------------------------
# OTMDLL.MAK - Makefile for Startup code and resource DLL
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

build:    $(_OBJEXE)\OtmAutoVerUp.exe

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------								
$(_OBJEXE)\OtmAutoVerUp.obj:    $(_SRC)\plugins\OtmAutoVerUp\OtmAutoVerUp.cpp \
								$(_SRC)\plugins\OtmAutoVerUp\OtmAutoVerUp.h \
								$(_SRC)\plugins\OtmAutoVerUp\OtmXmlParser.h
    @echo ---- Compiling $(_SRC)\plugins\OtmAutoVerUp\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OtmAutoVerUp\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OtmAutoVerUp\$(*B).cpp
								
$(_OBJEXE)\EQFAUTOVERUP.obj:	$(_SRC)\plugins\OtmAutoVerUp\EQFAUTOVERUP.cpp \
								$(_SRC)\plugins\OtmAutoVerUp\EQFAUTOVERUP.h \
								$(_SRC)\plugins\OtmAutoVerUp\OtmXmlParser.h
    @echo ---- Compiling $(_SRC)\plugins\OtmAutoVerUp\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OtmAutoVerUp\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OtmAutoVerUp\$(*B).cpp

$(_OBJEXE)\OtmXmlParser.obj:	$(_SRC)\plugins\OtmAutoVerUp\OtmXmlParser.cpp \
								$(_SRC)\plugins\OtmAutoVerUp\OtmXmlParser.h
    @echo ---- Compiling $(_SRC)\plugins\OtmAutoVerUp\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OtmAutoVerUp\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OtmAutoVerUp\$(*B).cpp
								
$(_OBJEXE)\OtmHttp.obj:	        $(_SRC)\core\PluginManager\OtmHttp.cpp \
								$(_SRC)\core\PluginManager\OtmHttp.h
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\core\PluginManager\$(*B).cpp

$(_OBJEXE)\OtmHttps.obj:	    $(_SRC)\core\PluginManager\OtmHttps.cpp \
								$(_SRC)\core\PluginManager\OtmHttps.h
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\core\PluginManager\$(*B).cpp

$(_OBJEXE)\OtmSftp.obj:	        $(_SRC)\core\PluginManager\OtmSftp.cpp \
								$(_SRC)\core\PluginManager\OtmSftp.h
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\core\PluginManager\$(*B).cpp

$(_OBJEXE)\OtmAutoVerUpComm.obj:$(_SRC)\plugins\OtmAutoVerUp\OtmAutoVerUpComm.cpp \
								$(_SRC)\plugins\OtmAutoVerUp\OtmAutoVerUpComm.h
    @echo ---- Compiling $(_SRC)\plugins\OtmAutoVerUp\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OtmAutoVerUp\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OtmAutoVerUp\$(*B).cpp

$(_OBJEXE)\OtmComm.obj:         $(_SRC)\core\PluginManager\OtmComm.cpp \
								$(_SRC)\core\PluginManager\OtmComm.h
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\core\PluginManager\$(*B).cpp


$(_OBJEXE)\OtmSftpConfig.obj:   $(_SRC)\core\PluginManager\OtmSftpConfig.cpp \
								$(_SRC)\core\PluginManager\OtmSftpConfig.h
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\core\PluginManager\$(*B).cpp

$(_OBJEXE)\OtmLogWriter.obj:    $(_SRC)\plugins\OtmAutoVerUp\OtmLogWriter.cpp \
								$(_SRC)\plugins\OtmAutoVerUp\OtmLogWriter.h
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /D ZLIB_WINAPI /Fo$(_OBJEXE)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\OtmAutoVerUp\$(*B).cpp

$(_OBJEXE)\OtmAutoVerUp.RES:	$(_SRC)\plugins\OtmAutoVerUp\OtmAutoVerUp.rc \
								$(_SRC)\plugins\OtmAutoVerUp\resource.h \
								$(_SRC)\plugins\OtmAutoVerUp\OtmAutoVerUp.ico  \
								$(_SRC)\plugins\OtmAutoVerUp\small.ico
	@echo ---- Compiling $(_RC_COMPILER)  /n /Fo $(_OBJEXE)\OtmAutoVerUp.RES $(_SRC)\plugins\OtmAutoVerUp\OtmAutoVerUp.rc
    @$(_RC_COMPILER)  /n /Fo $(_OBJEXE)\OtmAutoVerUp.RES $(_SRC)\plugins\OtmAutoVerUp\OtmAutoVerUp.rc >>$(_ERR)
								
OTMAUTOVERUPOBJS = $(_OBJEXE)\OtmAutoVerUp.obj \
	               $(_OBJEXE)\EQFAUTOVERUP.obj \
	               $(_OBJEXE)\OtmXmlParser.obj \
	               $(_OBJEXE)\OtmHttp.obj \
	               $(_OBJEXE)\OtmHttps.obj \
	               $(_OBJEXE)\OtmSftp.obj \
				   $(_OBJEXE)\OtmAutoVerUpComm.obj \
				   $(_OBJEXE)\OtmComm.obj \
	               $(_OBJEXE)\OtmSftpConfig.obj \
				   $(_OBJEXE)\OtmLogWriter.obj \
				   $(_OBJEXE)\OtmAutoVerUp.RES

$(_OBJEXE)\OtmAutoVerUp.exe:	$(OTMAUTOVERUPOBJS)

#------------------------------------------------------------------------------
# Build OtmAutoVerUp.exe
#------------------------------------------------------------------------------
$(_OBJEXE)\OtmAutoVerUp.exe:
    @echo ---- Linking $(_OBJEXE)\OtmAutoVerUp.exe
    @echo ---- Linking $(_OBJEXE)\OtmAutoVerUp.exe >>$(_ERR)
    @echo ---- _LINK_OPTIONS_EXE=$(_LINK_OPTIONS_EXE)
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
    $(OTMAUTOVERUPOBJS) 
	$(_LINK_OPTIONS_EXE)
/OUT:$(_OBJEXE)\OtmAutoVerUp.exe
    $(_LINK_LIB_EXE) $(_LIBOTHER)\xerces-c_3.lib $(_LIBOTHER)\libcurl.lib $(_LIBOTHER)\zlibwapi.lib comctl32.lib shell32.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\WIN\ md $(RELEASE_DIR)\OTM\WIN
    @copy $(_OBJEXE)\OtmAutoVerUp.EXE $(RELEASE_DIR)\OTM\WIN /Y>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\PLUGINS\ md $(RELEASE_DIR)\OTM\PLUGINS
    @copy /y $(_SRC)\plugins\OtmAutoVerUp\AutoVersionUp.conf.sample $(RELEASE_DIR)\OTM\PLUGINS /Y>$(_ERR)

