#-------------------------------------------------------------------------------
# OtmCleanupPlugin.MAK  - Makefile for cleanup tool plugin DLL
# Copyright (c) 2015, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\OtmToolsLauncherPlugin.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------


$(_OBJ)\OtmToolsLauncherPlugin.OBJ:	$(_SRC)\plugins\tools\OtmToolsLauncherPlugin\OtmToolsLauncherPlugin.cpp \
				$(_SRC)\plugins\tools\OtmToolsLauncherPlugin\OtmToolsLauncherPlugin.h \
				$(_SRC)\core\PluginManager\PluginManager.h \
				$(_SRC)\core\PluginManager\OtmPlugin.h \
				$(_SRC)\core\PluginManager\OtmToolPlugin.h


$(_DLL)\OtmToolsLauncherPlugin.DLL:	$(_OBJ)\OtmToolsLauncherPlugin.OBJ \
				$(_LIB)\PluginManager.lib

#------------------------------------------------------------------------------
# Build OtmToolsLauncherPlugin and copy plugin DLL to release directory
#------------------------------------------------------------------------------
$(_DLL)\OtmToolsLauncherPlugin.DLL:
    @echo ---- Linking $(_DLL)\OtmToolsLauncherPlugin.DLL
    @echo ---- Linking $(_DLL)\OtmToolsLauncherPlugin.DLL >>$(_ERR)
    $(_LINKER) @<<lnk.rsp>>$(_ERR)
    $(_OBJ)\OtmToolsLauncherPlugin.OBJ 
/OUT:$(_DLL)\OtmToolsLauncherPlugin.DLL
/MAP:$(_MAP)\OtmToolsLauncherPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\PluginManager.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmDLL.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @copy $(_DLL)\OtmToolsLauncherPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)
