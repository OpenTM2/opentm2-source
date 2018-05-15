#------------------------------------------------------------------------------
# OTMWorkbench.MAK - Makefile for the OTMWorkbench DLL
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

build:    $(_DLL)\OTMWorkbench.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFFLL00.OBJ:	$(_SRC)\core\workbench\EQFFLL00.CPP
$(_OBJ)\EQFDIC00.OBJ:	$(_SRC)\core\dictionary\EQFDIC00.C
$(_OBJ)\EQFPLGMG.OBJ:	$(_SRC)\core\workbench\EQFPLGMG.CPP
$(_OBJ)\EQFDRVEX.OBJ:	$(_SRC)\core\workbench\EQFDRVEX.CPP
$(_OBJ)\EQFCOPYR.OBJ:	$(_SRC)\core\workbench\EQFCOPYR.C
$(_OBJ)\EQFLOGO.OBJ:	$(_SRC)\core\workbench\EQFLOGO.C
$(_OBJ)\EQFFOLLI.OBJ:	$(_SRC)\core\workbench\EQFFOLLI.C
$(_OBJ)\EQFLNG00.OBJ:	$(_SRC)\core\workbench\EQFLNG00.C
$(_OBJ)\EQFMAIN.OBJ:	$(_SRC)\core\workbench\EQFMAIN.C
$(_OBJ)\EQFHNDLR.OBJ:	$(_SRC)\core\workbench\EQFHNDLR.C
$(_OBJ)\EQFTMM.OBJ:		$(_SRC)\core\memory\EQFTMM.CPP
$(_OBJ)\EQFTMMV.OBJ:	$(_SRC)\core\memory\EQFTMMV.CPP
$(_OBJ)\EQFMEM00.OBJ:	$(_SRC)\core\memory\EQFMEM00.CPP
$(_OBJ)\EQFDOC00.OBJ:	$(_SRC)\core\document\EQFDOC00.C  
$(_OBJ)\EQFCNT00.OBJ:	$(_SRC)\core\counting\EQFCNT00.C

$(_DLL)\OTMWorkbench.DLL: \
						$(_OBJ)\EQFFLL00.OBJ \
						$(_OBJ)\EQFFOLLI.OBJ \
            $(_OBJ)\EQFCOPYR.OBJ \
						$(_OBJ)\EQFLOGO.OBJ  \
						$(_OBJ)\EQFPLGMG.OBJ \
						$(_OBJ)\EQFDRVEX.OBJ \
						$(_OBJ)\EQFLNG00.OBJ \
            $(_OBJ)\EQFHNDLR.OBJ \
            $(_OBJ)\EQFDIC00.OBJ \
						$(_OBJ)\EQFTMM.OBJ \
						$(_OBJ)\EQFTMMV.OBJ \
						$(_OBJ)\EQFMEM00.OBJ \
            $(_OBJ)\EQFDOC00.OBJ \
						$(_OBJ)\EQFMAIN.OBJ \
            $(_OBJ)\EQFCNT00.OBJ 


#------------------------------------------------------------------------------
# Build OTMWorkbench.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMWorkbench.DLL:
    @echo ---- Linking $(_DLL)\OTMWorkbench.DLL
    @echo ---- Linking $(_DLL)\OTMWorkbench.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFFLL00.OBJ 
            $(_OBJ)\EQFDIC00.OBJ
						$(_OBJ)\EQFFOLLI.OBJ 
            $(_OBJ)\EQFCOPYR.OBJ 
						$(_OBJ)\EQFLOGO.OBJ  
						$(_OBJ)\EQFPLGMG.OBJ 
						$(_OBJ)\EQFDRVEX.OBJ 
						$(_OBJ)\EQFLNG00.OBJ 
            $(_OBJ)\EQFHNDLR.OBJ
            $(_OBJ)\EQFTMM.OBJ
            $(_OBJ)\EQFTMMV.OBJ 
 						$(_OBJ)\EQFMEM00.OBJ 
            $(_OBJ)\EQFDOC00.OBJ 
						$(_OBJ)\EQFMAIN.OBJ  
            $(_OBJ)\EQFCNT00.OBJ
/OUT:$(_DLL)\OTMWorkbench.DLL
/MAP:$(_MAP)\OTMWorkbench.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
            $(_LINK_LIB_CRT) 
            $(_LIB)\OtmBase.lib 
            $(_LIB)\OtmAPI.lib 
            $(_LIB)\PluginManager.lib 
            $(_LIB)\PluginMgrAssist.lib
            $(_LIB)\OtmFolderUtils.lib 
            $(_LIB)\OTMFolderFunctions.lib 
            $(_LIB)\OTMMemoryFunctions.lib 
            $(_LIB)\OTMDictionaryFunctions.lib 
            $(_LIB)\OTMListFunctions.lib 
            $(_LIB)\OTMTagTableFunctions.lib 
            $(_LIB)\OTMAnalysisFunctions.lib 
            $(_LIB)\OTMEditorFunctions.lib 
            $(_LIB)\OTMGLOBM.lib
            $(_LIB)\OTMCounting.lib
            $(_LIB)\OTMUtilities.lib
            $(_LIB)\OTMLinguistic.lib
            $(_LIB)\OTMMachTransl.lib
            $(_LIB)\OTMSegmentedFile.lib
            SHELL32.LIB
            COMCTL32.LIB
<<

