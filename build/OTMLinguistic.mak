#------------------------------------------------------------------------------
# OTMLinguistic.MAK - Makefile for the OTMLinguistic DLL
# Copyright (c) 2018 International Business Machines
# Corporation and others.  All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER                                                                  -
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------

build:    $(_DLL)\OTMLinguistic.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFMORPH.OBJ:	$(_SRC)\core\linguistic\EQFMORPH.CPP
$(_OBJ)\SpellFactory.OBJ:	$(_SRC)\core\spell\SpellFactory.cpp
$(_OBJ)\MorphFactory.OBJ:	$(_SRC)\core\morph\MorphFactory.cpp

$(_DLL)\OTMLinguistic.DLL: \
						$(_OBJ)\EQFMORPH.OBJ \
						$(_OBJ)\SpellFactory.OBJ \
						$(_OBJ)\MorphFactory.OBJ 


#------------------------------------------------------------------------------
# Build OTMLinguistic.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMLinguistic.DLL:
    @echo ---- Linking $(_DLL)\OTMLinguistic.DLL
    @echo ---- Linking $(_DLL)\OTMLinguistic.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFMORPH.OBJ 
						$(_OBJ)\SpellFactory.OBJ 
						$(_OBJ)\MorphFactory.OBJ 
/OUT:$(_DLL)\OTMLinguistic.DLL
/MAP:$(_MAP)\OTMLinguistic.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmAPI.lib $(_LIB)\PluginManager.lib 
<<
