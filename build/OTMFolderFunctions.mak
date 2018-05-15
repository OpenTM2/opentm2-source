#------------------------------------------------------------------------------
# OTMFolderFunctions.MAK - Makefile for the OTMFolderFunctions DLL
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

build:    $(_DLL)\OTMFolderFunctions.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFFOL00.OBJ:	$(_SRC)\core\folder\EQFFOL00.C
$(_OBJ)\EQFFOL01.OBJ:	$(_SRC)\core\folder\EQFFOL01.C
$(_OBJ)\EQFFOL02.OBJ:	$(_SRC)\core\folder\EQFFOL02.CPP
$(_OBJ)\EQFFOL03.OBJ:	$(_SRC)\core\folder\EQFFOL03.CPP
$(_OBJ)\EQFFOL04.OBJ:	$(_SRC)\core\folder\EQFFOL04.CPP $(_SRC)\core\folder\EQFGFR.H 
$(_OBJ)\EQFGFRBATCH.OBJ: $(_SRC)\core\folder\EQFGFRBATCH.CPP $(_SRC)\core\folder\EQFGFR.H
$(_OBJ)\EQFGFRPAINT.OBJ: $(_SRC)\core\folder\EQFGFRPAINT.CPP $(_SRC)\core\folder\EQFGFR.H
$(_OBJ)\EQFFOL05.OBJ:	$(_SRC)\core\folder\EQFFOL05.C
$(_OBJ)\EQFFOL06.OBJ:	$(_SRC)\core\folder\EQFFOL06.CPP
$(_OBJ)\EQFFSRCH.OBJ:	$(_SRC)\core\folder\EQFFSRCH.CPP
$(_OBJ)\EQFDOC01.OBJ:	$(_SRC)\core\document\EQFDOC01.C  
$(_OBJ)\EQFDOC02.OBJ:	$(_SRC)\core\document\EQFDOC02.C  
$(_OBJ)\ValDocExp.OBJ:	$(_SRC)\core\document\ValDocExp.CPP
$(_OBJ)\EQFDDED.OBJ:	$(_SRC)\core\services\EQFDDED.C
$(_OBJ)\EQFBAdjustCountInfo.OBJ:	$(_SRC)\core\editor\EQFBAdjustCountInfo.CPP


$(_DLL)\OTMFolderFunctions.DLL: \
						$(_OBJ)\EQFFOL00.OBJ \
            $(_OBJ)\EQFFOL01.OBJ \
						$(_OBJ)\EQFFOL02.OBJ \
						$(_OBJ)\EQFFOL03.OBJ \
						$(_OBJ)\EQFFOL04.OBJ \
						$(_OBJ)\EQFGFRBATCH.OBJ \
						$(_OBJ)\EQFGFRPAINT.OBJ \
						$(_OBJ)\EQFFOL05.OBJ \
						$(_OBJ)\EQFFOL06.OBJ \
						$(_OBJ)\EQFFSRCH.OBJ \
						$(_OBJ)\EQFDOC01.OBJ \
						$(_OBJ)\EQFDOC02.OBJ \
            $(_OBJ)\ValDocExp.OBJ \
            $(_OBJ)\EQFDDED.OBJ \
            $(_OBJ)\EQFBAdjustCountInfo.OBJ

#------------------------------------------------------------------------------
# Build OTMFolderFunctions.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMFolderFunctions.DLL:
    @echo ---- Linking $(_DLL)\OTMFolderFunctions.DLL
    @echo ---- Linking $(_DLL)\OTMFolderFunctions.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFFOL00.OBJ
            $(_OBJ)\EQFFOL01.OBJ 
						$(_OBJ)\EQFFOL02.OBJ 
						$(_OBJ)\EQFFOL03.OBJ 
						$(_OBJ)\EQFFOL04.OBJ 
						$(_OBJ)\EQFGFRBATCH.OBJ 
						$(_OBJ)\EQFGFRPAINT.OBJ 
						$(_OBJ)\EQFFOL05.OBJ 
						$(_OBJ)\EQFFOL06.OBJ 
						$(_OBJ)\EQFFSRCH.OBJ 
						$(_OBJ)\EQFDOC01.OBJ 
						$(_OBJ)\EQFDOC02.OBJ 
            $(_OBJ)\ValDocExp.OBJ
            $(_OBJ)\EQFDDED.OBJ
            $(_OBJ)\EQFBAdjustCountInfo.OBJ
/OUT:$(_DLL)\OTMFolderFunctions.DLL
/MAP:$(_MAP)\OTMFolderFunctions.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
						$(_LINK_LIB_CRT) 
            imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib 
            $(_LIB)\OtmBase.lib 
            $(_LIB)\OtmAPI.lib 
            $(_LIB)\PluginManager.lib 
            $(_LIB)\OTMDictionaryFunctions.lib 
						$(_LIB)\OTMTagTableFunctions.lib 
            $(_LIB)\OTMGLOBM.lib 
            $(_LIB)\OTMLinguistic.lib 
            $(_LIB)\OTMSegmentedFile.lib 
            $(_LIB)\OTMMemoryFunctions.lib 
            $(_LIB)\OTMFuzzy.lib 
            $(_LIB)\OTMFolderUtils.lib 
            $(_LIB)\OTMDialog.lib 
            $(_LIB)\OTMUtilities.lib 
            $(_LIB)\OTMAnalysisFunctions.lib 
            $(_LIB)\OTMCounting.lib 
<<

