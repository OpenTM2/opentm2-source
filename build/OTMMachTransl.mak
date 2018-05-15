#------------------------------------------------------------------------------
# OTMMachTransl.MAK - Makefile for the OTMMachTransl DLL
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

build:    $(_DLL)\OTMMachTransl.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFMT00.OBJ:	$(_SRC)\core\mt\EQFMT00.C
$(_OBJ)\EQFMT01.OBJ:	$(_SRC)\core\mt\EQFMT01.C

$(_DLL)\OTMMachTransl.DLL: \
						$(_OBJ)\EQFMT00.OBJ  \
						$(_OBJ)\EQFMT01.OBJ  

#------------------------------------------------------------------------------
# Build OTMMachTransl.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMMachTransl.DLL:
    @echo ---- Linking $(_DLL)\OTMMachTransl.DLL
    @echo ---- Linking $(_DLL)\OTMMachTransl.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFMT00.OBJ  
						$(_OBJ)\EQFMT01.OBJ  
/OUT:$(_DLL)\OTMMachTransl.DLL
/MAP:$(_MAP)\OTMMachTransl.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmAPI.lib $(_LIB)\PluginManager.lib $(_LIB)\PluginMgrAssist.lib $(_LIB)\OTMGLOBM.lib $(_LIBOTHER)\xerces-c_3.lib
<<
