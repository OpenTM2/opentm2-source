#-------------------------------------------------------------------------------
# OtmMorphICUPlugin.MAK    - Makefile for Morphologic Plugin DLL using ICU
# Copyright (c) 2013-2017, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\OtmMorphICUPlugin.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\OtmMorphICUPlugin.OBJ:	$(_SRC)\plugins\morph\OtmMorphICUPlugin\OtmMorphICUPlugin.cpp \
								$(_SRC)\plugins\morph\OtmMorphICUPlugin\OtmMorphICUPlugin.h \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\OtmPlugin.h \
								$(_SRC)\core\PluginManager\OtmMorphPlugin.h \
								$(_SRC)\core\PluginManager\OtmMorph.h \
								$(_SRC)\plugins\morph\OtmMorphICUPlugin\OtmMorphICU.h
								@$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\  $(_SRC)\plugins\morph\OtmMorphICUPlugin\$(*B).cpp

$(_OBJ)\OtmMorphICU.OBJ:		$(_SRC)\plugins\morph\OtmMorphICUPlugin\OtmMorphICU.cpp \
								$(_SRC)\plugins\morph\OtmMorphICUPlugin\OtmMorphICU.h \
								$(_SRC)\plugins\spell\OtmSpellHSPlugin\HunspellObjManager.h \
								$(_SRC)\plugins\spell\OtmSpellHSPlugin\AutoLock.h \
								$(_SRC)\core\PluginManager\OtmMorph.h
								@$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\  $(_SRC)\plugins\morph\OtmMorphICUPlugin\$(*B).cpp

$(_DLL)\OtmMorphICUPlugin.DLL:	$(_OBJ)\OtmMorphICUPlugin.OBJ \
								$(_OBJ)\OtmMorphICU.OBJ \
								$(_OBJ)\HunspellObjManager.OBJ \
								$(_LIB)\OtmBase.lib \
								$(_LIB)\PluginManager.lib

#------------------------------------------------------------------------------
# Build OtmMorphICUPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------

$(_DLL)\OtmMorphICUPlugin.DLL:
    @echo ---- Linking $(_DLL)\OtmMorphICUPlugin.DLL
    @echo ---- Linking $(_DLL)\OtmMorphICUPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
					$(_OBJ)\OtmMorphICUPlugin.OBJ
					$(_OBJ)\OtmMorphICU.OBJ
					$(_OBJ)\HunspellObjManager.OBJ
					$(_OBJ)\OtmSpellHS.OBJ
/OUT:$(_DLL)\OtmMorphICUPlugin.DLL
/MAP:$(_MAP)\OtmMorphICUPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) 
$(_LIB)\OtmBase.lib 
$(_LIB)\PluginManager.lib 
$(HUNSPELLLIBDIR)\$(HUNSPELLLIB) $(ICULIB)\icuuc.lib $(ICULIB)\icudt.lib $(ICULIB)\icuin.lib $(ICULIB)\icuio.lib $(ICULIB)\icule.lib $(ICULIB)\iculx.lib $(ICULIB)\icutu.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmMorphICUPlugin md $(RELEASE_DIR)\OTM\Plugins\OtmMorphICUPlugin
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmMorphICUPlugin\Rules md $(RELEASE_DIR)\OTM\Plugins\OtmMorphICUPlugin\Rules
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmMorphICUPlugin\AbbrevLists md $(RELEASE_DIR)\OTM\Plugins\OtmMorphICUPlugin\AbbrevLists
    @copy $(_DLL)\OtmMorphICUPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)
    @copy $(_DRIVE)\$(_DEVDIR)\ICUMorphData\Rules\*.* $(RELEASE_DIR)\OTM\Plugins\OtmMorphICUPlugin\Rules /Y >>$(_ERR)
    @copy $(_DRIVE)\$(_DEVDIR)\ICUMorphData\AbbrevLists\*.* $(RELEASE_DIR)\OTM\Plugins\OtmMorphICUPlugin\AbbrevLists /Y >>$(_ERR)
