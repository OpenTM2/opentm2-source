# ------------------------------------------------------------------------------
# PluginManager.MAK    - Makefile for the PluginManager
# Copyright (c) 2014, International Business Machines
# Corporation and others.  All rights reserved.
# ------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\PluginManager.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\PluginManager.obj:		$(_SRC)\core\PluginManager\PluginManager.cpp \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\PluginManagerImpl.h \
								$(_SRC)\core\PluginManager\OtmPlugin.h

$(_OBJ)\PluginManagerImpl.obj:	$(_SRC)\core\PluginManager\PluginManagerImpl.cpp \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\PluginManagerImpl.h \
								$(_SRC)\core\PluginManager\OtmPlugin.h

$(_DLL)\PluginManager.DLL:	$(_OBJ)\PluginManager.obj \
							$(_OBJ)\PluginManagerImpl.obj							

#------------------------------------------------------------------------------
# Build PluginManager.dll
#------------------------------------------------------------------------------

$(_DLL)\PluginManager.DLL:
    @echo ---- Linking $(_DLL)\PluginManager.DLL
    @echo ---- Linking $(_DLL)\PluginManager.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
			$(_OBJ)\PluginManager.obj
			$(_OBJ)\PluginManagerImpl.obj
/OUT:$(_DLL)\PluginManager.DLL
/MAP:$(_MAP)\PluginManager.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib
<<
