#------------------------------------------------------------------------------
# OTMDLL.MAK - Makefile for Startup code and resource DLL
# Copyright (c) 2017, International Business Machines
# Corporation and others.  All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER                                                                  -
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------

build:    $(_DLL)\OtmProfileMgrPlugin.Dll

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\OtmProfileMgrPlugin.obj:	$(_SRC)\plugins\tools\OtmProfileMgrPlugin\OtmProfileMgrPlugin.cpp
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D ZLIB_WINAPI /Fo$(_OBJ)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).cpp

$(_OBJ)\OtmProfileMgrDlg.obj:	$(_SRC)\plugins\tools\OtmProfileMgrPlugin\OtmProfileMgrDlg.cpp
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D ZLIB_WINAPI /Fo$(_OBJ)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).cpp

$(_OBJ)\OtmProfileMgrComm.obj:	$(_SRC)\plugins\tools\OtmProfileMgrPlugin\OtmProfileMgrComm.cpp
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D ZLIB_WINAPI /Fo$(_OBJ)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).cpp

$(_OBJ)\ProfileConfXmlParser.obj:	        $(_SRC)\plugins\tools\OtmProfileMgrPlugin\ProfileConfXmlParser.cpp
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D ZLIB_WINAPI /Fo$(_OBJ)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).cpp

$(_OBJ)\ProfileSetXmlParser.obj:	    $(_SRC)\plugins\tools\OtmProfileMgrPlugin\ProfileSetXmlParser.cpp
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D ZLIB_WINAPI /Fo$(_OBJ)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).cpp

$(_OBJ)\LastUsedVal.obj:	        $(_SRC)\plugins\tools\OtmProfileMgrPlugin\LastUsedVal.cpp
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D ZLIB_WINAPI /Fo$(_OBJ)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).cpp

$(_OBJ)\OtmLogWriterLoc.obj:$(_SRC)\plugins\tools\OtmProfileMgrPlugin\OtmLogWriterLoc.cpp
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D ZLIB_WINAPI /Fo$(_OBJ)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).cpp

$(_OBJ)\OtmFileEncryptSet.obj:$(_SRC)\plugins\tools\OtmProfileMgrPlugin\OtmFileEncryptSet.cpp
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D ZLIB_WINAPI /Fo$(_OBJ)\ /Fd /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProfileMgrPlugin\$(*B).cpp

$(_OBJ)\OtmProfileMgrPlugin.RES:	$(_SRC)\plugins\tools\OtmProfileMgrPlugin\OtmProfileMgrPlugin.rc \
								$(_SRC)\plugins\tools\OtmProfileMgrPlugin\resource.h \
								$(_SRC)\plugins\tools\OtmProfileMgrPlugin\small.ico
	@echo ---- Compiling $(_RC_COMPILER)  /n /Fo $(_OBJ)\OtmProfileMgrPlugin.RES $(_SRC)\plugins\tools\OtmProfileMgrPlugin\OtmProfileMgrPlugin.rc
    @$(_RC_COMPILER)  /n /Fo $(_OBJ)\OtmProfileMgrPlugin.RES $(_SRC)\plugins\tools\OtmProfileMgrPlugin\OtmProfileMgrPlugin.rc >>$(_ERR)

$(_DLL)\OtmProfileMgrPlugin.Dll:	$(_OBJ)\OtmProfileMgrPlugin.obj \
	               $(_OBJ)\OtmProfileMgrDlg.obj \
	               $(_OBJ)\OtmProfileMgrComm.obj \
	               $(_OBJ)\ProfileConfXmlParser.obj \
	               $(_OBJ)\ProfileSetXmlParser.obj \
	               $(_OBJ)\LastUsedVal.obj \
	               $(_OBJ)\OtmLogWriterLoc.obj \
				   $(_OBJ)\OtmFileEncryptSet.obj \
				   $(_OBJ)\OtmProfileMgrPlugin.RES

#------------------------------------------------------------------------------
# Build OtmProfileMgrPlugin and copy plugin DLL to release directory
#------------------------------------------------------------------------------
$(_DLL)\OtmProfileMgrPlugin.Dll:
    @echo ---- Linking $(_OBJ)\OtmProfileMgrPlugin.Dll
    @echo ---- Linking $(_OBJ)\OtmProfileMgrPlugin.Dll >>$(_ERR)
    $(_LINKER) @<<lnk.rsp>>$(_ERR)
    $(_OBJ)\OtmProfileMgrPlugin.OBJ 
    $(_OBJ)\OtmProfileMgrDlg.OBJ
    $(_OBJ)\OtmProfileMgrComm
    $(_OBJ)\ProfileConfXmlParser.OBJ 
    $(_OBJ)\ProfileSetXmlParser.OBJ 
    $(_OBJ)\LastUsedVal.OBJ 
    $(_OBJ)\OtmLogWriterLoc.OBJ
	$(_OBJ)\OtmFileEncryptSet.obj
    $(_OBJ)\OtmProfileMgrPlugin.RES
/OUT:$(_DLL)\OtmProfileMgrPlugin.DLL
/MAP:$(_MAP)\OtmProfileMgrPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) 
$(_LIB)\OtmBase.lib 
$(_LIB)\OTMEditorFunctions.lib 
$(_LIB)\PluginManager.lib 
$(_LIBOTHER)\xerces-c_3.lib $(_LIBOTHER)\zlibwapi.lib comctl32.lib shell32.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @copy $(_DLL)\OtmProfileMgrPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)

