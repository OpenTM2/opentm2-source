#-------------------------------------------------------------------------------
# OtmSpellHSPlugin.MAK    - Makefile for Spell Check Plugin DLL
# Copyright (c) 2014-2017, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\OtmSpellHSPlugin.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------


$(_OBJ)\HunspellObjManager.OBJ:			$(_SRC)\plugins\spell\OtmSpellHSPlugin\HunspellObjManager.cpp \
								$(_SRC)\plugins\spell\OtmSpellHSPlugin\HunspellObjManager.h \
								$(_SRC)\core\PluginManager\OtmSpell.h \
								$(_SRC)\plugins\spell\OtmSpellHSPlugin\AutoLock.h
    @echo ---- Compiling $(_SRC)\$(*B).CPP
    @echo ---- Compiling $(_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\  $(_SRC)\plugins\spell\OtmSpellHSPlugin\$(*B).cpp

$(_OBJ)\OtmSpellHSPlugin.OBJ:	$(_SRC)\plugins\spell\OtmSpellHSPlugin\OtmSpellHSPlugin.cpp \
								$(_SRC)\plugins\spell\OtmSpellHSPlugin\OtmSpellHSPlugin.h \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\OtmPlugin.h \
								$(_SRC)\core\PluginManager\OtmSpellPlugin.h \
								$(_SRC)\core\PluginManager\OtmSpell.h \
								$(_SRC)\plugins\spell\OtmSpellHSPlugin\OtmSpellHS.h
								@$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\  $(_SRC)\plugins\spell\OtmSpellHSPlugin\$(*B).cpp

$(_OBJ)\OtmSpellHS.OBJ:			$(_SRC)\plugins\spell\OtmSpellHSPlugin\OtmSpellHS.cpp \
								$(_SRC)\plugins\spell\OtmSpellHSPlugin\OtmSpellHS.h \
								$(_SRC)\plugins\spell\OtmSpellHSPlugin\HunspellObjManager.h \
								$(_SRC)\plugins\spell\OtmSpellHSPlugin\AutoLock.h \
								$(_SRC)\core\PluginManager\OtmSpell.h
								@$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\  $(_SRC)\plugins\spell\OtmSpellHSPlugin\$(*B).cpp

$(_DLL)\OtmSpellHSPlugin.DLL:	$(_OBJ)\OtmSpellHSPlugin.OBJ \
								$(_OBJ)\OtmSpellHS.OBJ \
								$(_OBJ)\HunspellObjManager.OBJ \
								$(_LIB)\OtmBase.lib \
								$(_LIB)\PluginManager.lib

#------------------------------------------------------------------------------
# Build OtmSpellHSPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------

$(_DLL)\OtmSpellHSPlugin.DLL:
    @echo ---- Linking $(_DLL)\OtmSpellHSPlugin.DLL
    @echo ---- Linking $(_DLL)\OtmSpellHSPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
					$(_OBJ)\OtmSpellHSPlugin.OBJ
					$(_OBJ)\OtmSpellHS.OBJ
					$(_OBJ)\HunspellObjManager.OBJ
/OUT:$(_DLL)\OtmSpellHSPlugin.DLL
/MAP:$(_MAP)\OtmSpellHSPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) 
$(_LIB)\OtmBase.lib 
$(_LIB)\PluginManager.lib 
$(HUNSPELLLIBDIR)\$(HUNSPELLLIB)
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmSpellHSPlugin md $(RELEASE_DIR)\OTM\Plugins\OtmSpellHSPlugin
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmSpellHSPlugin\DICT md $(RELEASE_DIR)\OTM\Plugins\OtmSpellHSPlugin\DICT
    @copy $(_DLL)\OtmSpellHSPlugin.DLL $(RELEASE_DIR)\OTM\Plugins\OtmSpellHSPlugin /Y>$(_ERR)
    @copy $(_SRC)\plugins\spell\OtmSpellHSPlugin\LanguageConfig.lng $(RELEASE_DIR)\OTM\Plugins\OtmSpellHSPlugin /Y>$(_ERR)
    @copy $(_DRIVE)\$(_DEVDIR)\HunSpellDictionaries\*.* $(RELEASE_DIR)\OTM\Plugins\OtmSpellHSPlugin\DICT /Y >>$(_ERR)
