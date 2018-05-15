#------------------------------------------------------------------------------
# OTMUtilities.MAK - Makefile for the OtmUtilities DLL
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

build:    $(_DLL)\OTMUtilities.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------


$(_OBJ)\EQFUTDLG.OBJ:	$(_SRC)\core\utilities\EQFUTDLG.CPP
$(_OBJ)\EQFPROPS.OBJ:	$(_SRC)\core\utilities\EQFPROPS.CPP
$(_OBJ)\EQFAPROF.OBJ:	$(_SRC)\core\utilities\EQFAPROF.C
$(_OBJ)\eqfutclb.OBJ:	$(_SRC)\core\utilities\EQFUTCLB.C
$(_OBJ)\EQFNOISE.OBJ:	$(_SRC)\core\utilities\EQFNOISE.C
$(_OBJ)\EQFPROGR.OBJ:	$(_SRC)\core\utilities\EQFPROGR.C
$(_OBJ)\EQFSETUP.OBJ:	$(_SRC)\core\utilities\EQFSETUP.C
$(_OBJ)\EQFTADIT.OBJ:	$(_SRC)\core\utilities\EQFTADIT.cpp
$(_OBJ)\EQFSEGEXPORT.OBJ:	$(_SRC)\core\utilities\EQFSEGEXPORT.C

$(_DLL)\OTMUtilities.DLL: \
						$(_OBJ)\EQFUTDLG.OBJ \
						$(_OBJ)\EQFPROPS.OBJ \
						$(_OBJ)\EQFAPROF.OBJ \
						$(_OBJ)\eqfutclb.OBJ \
						$(_OBJ)\EQFNOISE.OBJ \
						$(_OBJ)\EQFPROGR.OBJ \
						$(_OBJ)\EQFSETUP.OBJ \
						$(_OBJ)\EQFTADIT.OBJ \
						$(_OBJ)\EQFSEGEXPORT.OBJ 

#------------------------------------------------------------------------------
# Build OTMUtilities.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMUtilities.DLL:
    @echo ---- Linking $(_DLL)\OTMUtilities.DLL
    @echo ---- Linking $(_DLL)\OTMUtilities.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFUTDLG.OBJ 
						$(_OBJ)\EQFPROPS.OBJ 
						$(_OBJ)\EQFAPROF.OBJ 
						$(_OBJ)\eqfutclb.OBJ 
						$(_OBJ)\EQFNOISE.OBJ 
						$(_OBJ)\EQFPROGR.OBJ 
						$(_OBJ)\EQFSETUP.OBJ 
						$(_OBJ)\EQFTADIT.OBJ 
						$(_OBJ)\CXMLWriter.OBJ 
						$(_OBJ)\EQFSEGEXPORT.OBJ 
/OUT:$(_DLL)\OTMUtilities.DLL
/MAP:$(_MAP)\OTMUtilities.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
            $(_LINK_LIB_CRT) 
            imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib 
            $(_LIB)\OtmBase.lib 
            $(_LIB)\OtmAPI.lib 
            $(_LIB)\PluginManager.lib 
            $(_LIB)\OTMTagTableFunctions.lib 
            $(_LIBOTHER)\xerces-c_3.lib 
            $(_LIB)\OtmLinguistic.lib
            $(_LIB)\OTMMemoryFunctions.lib
            $(_LIB)\OtmSegmentedFile.lib
            $(_LIB)\OTMDictionaryFunctions.lib
            $(_LIB)\OtmFolderUtils.lib
<<

#$(_LINK_LIB_CRT) imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmAPI.lib $(_LIB)\PluginManager.lib $(_LIB)\PluginMgrAssist.lib $(_LIB)\OTMGLOBM.lib $(_LIBOTHER)\xerces-c_3.lib


