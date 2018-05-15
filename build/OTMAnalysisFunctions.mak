#------------------------------------------------------------------------------
# OTMAnalysisFunctions.MAK - Makefile for the OTMAnalysisFunctions.DLL
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

build:    $(_DLL)\OTMAnalysisFunctions.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFTAML.OBJ:	$(_SRC)\core\analysis\EQFTAML.CPP
$(_OBJ)\EQFANA00.OBJ:	$(_SRC)\core\analysis\EQFANA00.CPP
$(_OBJ)\EQFTAARC.OBJ:	$(_SRC)\core\analysis\EQFTAARC.CPP
$(_OBJ)\EQFTAFUN.OBJ:	$(_SRC)\core\analysis\EQFTAFUN.C
$(_OBJ)\EQFTAG00.OBJ:	$(_SRC)\core\analysis\EQFTAG00.C
$(_OBJ)\EQFTALP0.OBJ:	$(_SRC)\core\analysis\EQFTALP0.C
$(_OBJ)\EQFTALP1.OBJ:	$(_SRC)\core\analysis\EQFTALP1.C
$(_OBJ)\EQFTALP2.OBJ:	$(_SRC)\core\analysis\EQFTALP2.C
$(_OBJ)\EQFTAPH2.OBJ:	$(_SRC)\core\analysis\EQFTAPH2.CPP
$(_OBJ)\EQFPRSNO.OBJ:	$(_SRC)\core\analysis\EQFPRSNO.CPP
$(_OBJ)\EQFTSEGM.OBJ:	$(_SRC)\core\analysis\EQFTSEGM.CPP
$(_OBJ)\EQFIANA1.OBJ:	$(_SRC)\core\analysis\EQFIANA1.C
$(_OBJ)\EQFENTITY.OBJ:	$(_SRC)\core\analysis\EQFENTITY.C
$(_OBJ)\LoadBalancerList.OBJ:	$(_SRC)\core\analysis\LoadBalancerList.CPP

$(_DLL)\OTMAnalysisFunctions.DLL: \
						$(_OBJ)\EQFANA00.OBJ \
						$(_OBJ)\EQFTAARC.OBJ \
						$(_OBJ)\EQFTAFUN.OBJ \
						$(_OBJ)\EQFTAG00.OBJ \
						$(_OBJ)\EQFTALP0.OBJ \
						$(_OBJ)\EQFTALP1.OBJ \
						$(_OBJ)\EQFTALP2.OBJ \
						$(_OBJ)\EQFTAPH2.OBJ \
						$(_OBJ)\EQFPRSNO.OBJ \
						$(_OBJ)\EQFTSEGM.OBJ \
						$(_OBJ)\EQFIANA1.OBJ \
						$(_OBJ)\EQFTAML.OBJ \
						$(_OBJ)\EQFENTITY.OBJ \
            $(_OBJ)\LoadBalancerList.OBJ


#------------------------------------------------------------------------------
# Build OTMAnalysisFunctions.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMAnalysisFunctions.DLL:
    @echo ---- Linking $(_DLL)\OTMAnalysisFunctions.DLL
    @echo ---- Linking $(_DLL)\OTMAnalysisFunctions.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFANA00.OBJ 
						$(_OBJ)\EQFTAML.OBJ 
						$(_OBJ)\EQFTAARC.OBJ 
						$(_OBJ)\EQFTAFUN.OBJ 
						$(_OBJ)\EQFTAG00.OBJ 
						$(_OBJ)\EQFTALP0.OBJ 
						$(_OBJ)\EQFTALP1.OBJ 
						$(_OBJ)\EQFTALP2.OBJ 
						$(_OBJ)\EQFTAPH2.OBJ 
						$(_OBJ)\EQFPRSNO.OBJ 
						$(_OBJ)\EQFTSEGM.OBJ 
						$(_OBJ)\EQFIANA1.OBJ 
						$(_OBJ)\EQFENTITY.OBJ 
            $(_OBJ)\LoadBalancerList.OBJ
/OUT:$(_DLL)\OTMAnalysisFunctions.DLL
/MAP:$(_MAP)\OTMAnalysisFunctions.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
						$(_LINK_LIB_CRT) 
            $(_LIB)\OtmBase.lib 
            $(_LIB)\OtmAPI.lib 
            $(_LIB)\OtmLinguistic.lib 
            $(_LIB)\OTMMemoryFunctions.lib 
            $(_LIB)\OtmSegmentedFile.lib 
            $(_LIB)\OTMDictionaryFunctions.lib 
            $(_LIB)\OtmFuzzy.lib 
            $(_LIB)\OTMTagTableFunctions.lib 
            $(_LIB)\OTMUtilities.lib 
            $(_LIB)\OTMFolderUtils.lib 
            $(_LIB)\OTMCounting.lib 
            $(_LIB)\PluginManager.lib $(_LIB)\OTMTagTableFunctions.lib 
            imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib 
            $(_LIB)\OTMGLOBM.lib 
<<

#            $(_LIBOTHER)\xerces-c_3.lib

#$(_LINK_LIB_CRT) imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmAPI.lib $(_LIB)\PluginManager.lib $(_LIB)\PluginMgrAssist.lib $(_LIB)\OTMGLOBM.lib $(_LIBOTHER)\xerces-c_3.lib
