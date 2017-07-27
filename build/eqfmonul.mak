# EQFMONUL.MAK - Makefile for the Dummy Morphologic Functions Module      
# Copyright (c) 2017, International Business Machines
# Corporation and others.  All rights reserved.


!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:	$(_DLL)\EQFMONUL.DLL

$(_OBJ)\EQFMONUL.OBJ:	$(_SRC)\plugins\linguistic\EQFMONUL.CPP \
								$(_SRC)\core\spell\SpellFactory.h \
								$(_SRC)\core\morph\MorphFactory.h

$(_DLL)\EQFMONUL.DLL:	$(_OBJ)\EQFMONUL.OBJ \
								$(_LIB)\OtmBase.lib \
								$(_LIB)\OtmDll.lib \
								$(_LIB)\PluginManager.lib



#------------------------------------------------------------------------------
# Build EQFMONUL.DLL                                                           -
#------------------------------------------------------------------------------
$(_DLL)\EQFMONUL.DLL:
    @echo ---- Linking $(_DLL)\EQFMONUL.DLL
    @echo ---- Linking $(_DLL)\EQFMONUL.DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\EQFMONUL.OBJ
/OUT:$(_DLL)\EQFMONUL.DLL
$(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIB)\OtmDll.lib $(_LIB)\PluginManager.lib 
<<
