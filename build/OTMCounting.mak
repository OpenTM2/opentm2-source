#------------------------------------------------------------------------------
# OTMCounting.MAK - Makefile for the OTMCounting DLL
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

build:    $(_DLL)\OTMCounting.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFCNT01.OBJ:	$(_SRC)\core\counting\EQFCNT01.C
$(_OBJ)\EQFRPT.OBJ:		$(_SRC)\core\counting\EQFRPT.C
$(_OBJ)\EQFWCNT.OBJ: 	$(_SRC)\core\counting\EQFWCNT.CPP
$(_OBJ)\EQFRPXML.OBJ:	$(_SRC)\core\counting\EQFRPXML.CPP
$(_OBJ)\EQFRPT00.OBJ:	$(_SRC)\core\counting\EQFRPT00.C

$(_DLL)\OTMCounting.DLL: \
						$(_OBJ)\EQFCNT01.OBJ \
						$(_OBJ)\EQFRPT.OBJ   \
						$(_OBJ)\EQFWCNT.OBJ  \
						$(_OBJ)\EQFRPXML.OBJ \
            $(_OBJ)\EQFRPT00.OBJ

#------------------------------------------------------------------------------
# Build OTMCounting.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMCounting.DLL:
    @echo ---- Linking $(_DLL)\OTMCounting.DLL
    @echo ---- Linking $(_DLL)\OTMCounting.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFCNT01.OBJ 
						$(_OBJ)\EQFWCNT.OBJ  
						$(_OBJ)\EQFRPT.OBJ   
						$(_OBJ)\EQFRPXML.OBJ 
            $(_OBJ)\EQFRPT00.OBJ
/OUT:$(_DLL)\OTMCounting.DLL
/MAP:$(_MAP)\OTMCounting.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
            $(_LINK_LIB_CRT) 
            $(_LIB)\OtmBase.lib 
            $(_LIB)\OtmAPI.lib 
            $(_LIB)\PluginManager.lib 
            $(_LIB)\OTMTagTableFunctions.lib 
            $(_LIB)\OtmSegmentedFile.lib 
            $(_LIB)\OtmLinguistic.lib 
            $(_LIB)\OTMMemoryFunctions.lib 
            $(_LIB)\OtmFolderUtils.lib 
            $(_LIB)\OtmFuzzy.lib 
<<

#$(_LINK_LIB_CRT) imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmAPI.lib $(_LIB)\PluginManager.lib $(_LIB)\PluginMgrAssist.lib $(_LIB)\OTMGLOBM.lib $(_LIBOTHER)\xerces-c_3.lib
