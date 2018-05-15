#-------------------------------------------------------------------------------
# OtmCleanupPlugin.MAK  - Makefile for cleanup tool plugin DLL
# Copyright (c) 2014, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\OtmCleanupPlugin.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------


$(_OBJ)\OtmCleanupPlugin.OBJ:	$(_SRC)\plugins\tools\OtmCleanupPlugin\OtmCleanupPlugin.cpp \
				$(_SRC)\plugins\tools\OtmCleanupPlugin\OtmCleanupPlugin.h \
				$(_SRC)\core\PluginManager\PluginManager.h \
				$(_SRC)\core\PluginManager\OtmPlugin.h \
				$(_SRC)\core\PluginManager\OtmToolPlugin.h


$(_DLL)\OtmCleanupPlugin.DLL:	$(_OBJ)\OtmCleanupPlugin.OBJ \
				$(_LIB)\PluginManager.lib

#------------------------------------------------------------------------------
# Build OtmCleanupPlugin and copy plugin DLL to release directory
#------------------------------------------------------------------------------

$(_DLL)\OtmCleanupPlugin.DLL:
    @echo ---- Compiling resource $(_SRC)\plugins\tools\OtmCleanupPlugin\OtmCleanupPlugin.RC >>$(_ERR)
    RC /D_WINDOWS /Fo$(_OBJ)\OtmCleanupPlugin.RES $(_SRC)\plugins\tools\OtmCleanupPlugin\OtmCleanupPlugin.RC
    @echo ---- Converting resource $(_OBJ)\OtmCleanupPlugin.RES >>$(_ERR)
    CVTRES /NOLOGO /OUT:$(_OBJ)\OtmCleanupPlugin.RBJ $(_OBJ)\OtmCleanupPlugin.RES
    @echo ---- Linking $(_DLL)\OtmCleanupPlugin.DLL
    @echo ---- Linking $(_DLL)\OtmCleanupPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
    $(_OBJ)\OtmCleanupPlugin.OBJ $(_OBJ)\OtmCleanupPlugin.RBJ
/OUT:$(_DLL)\OtmCleanupPlugin.DLL
/MAP:$(_MAP)\OtmCleanupPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\PluginManager.lib $(_LIB)\OtmBase.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @copy $(_DLL)\OtmCleanupPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)