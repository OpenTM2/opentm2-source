# OTMFUNC.MAK
#    Description:     --- NON-DDE Batch interface DLL
# Copyright (c) 2017, International Business Machines
# Corporation and others.  All rights reserved.

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------
build:    $(_LIB)\OTMFUNC.DLL \
          $(_BIN)\OTMGetReportData.EXE \
          $(_BIN)\TestAPI.EXE

$(_OBJ)\InitPlugins.obj:        $(_SRC)\tools\common\InitPlugins.CPP

$(_OBJ)\OTMFUNC.OBJ:	$(_SRC)\api\OTMFUNC.C

$(_DLL)\OTMFUNC.DLL:	$(_OBJ)\OTMFUNC.OBJ $(_OBJ)\InitPlugins.obj


$(_DLL)\OTMFUNC.DLL:
    @echo ---- Linking $(_DLL)\OTMFUNC.DLL
    @echo ---- Linking $(_DLL)\OTMFUNC.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
                          $(_OBJ)\OTMFUNC.OBJ
						  $(_OBJ)\InitPlugins.obj
/OUT:$(_DLL)\OTMFUNC.DLL
/MAP:$(_MAP)\OTMFUNC.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) 
$(_LIB)\OTMBase.LIB 
$(_LIB)\OTMITMD.LIB 
$(_LIB)\OTMMemoryFunctions.lib 
$(_LIB)\OTMFolderFunctions.lib 
$(_LIB)\OTMLinguistic.LIB 
$(_LIB)\OTMTagTableFunctions.lib 
$(_LIB)\OTMEditorFunctions.lib 
$(_LIB)\OTMCounting.LIB 
$(_LIB)\OTMDictionaryFunctions.lib 
$(_LIB)\OTMWorkbench.LIB 
$(_LIB)\OTMAnalysisFunctions.lib 
$(_LIB)\OTMUtilities.LIB 
$(_LIB)\PluginManager.LIB
<<
    @if not exist $(RELEASE_DIR)\OTM\API\ md $(RELEASE_DIR)\OTM\API
    @copy $(_LIB)\OtmFUNC.LIB $(RELEASE_DIR)\OTM\API /Y>$(_ERR)
    @copy $(_INC)\OtmFUNC.H $(RELEASE_DIR)\OTM\API /Y>$(_ERR)
    @copy $(_INC)\EQFPAPI.H $(RELEASE_DIR)\OTM\API /Y>$(_ERR)



$(_OBJ)\OTMGetReportData.OBJ:	$(_SRC)\api\OTMGetReportData.C $(_SRC)\api\OTMGetReportData.H

$(_BIN)\OTMGetReportData.EXE:	$(_OBJ)\OTMGetReportData.OBJ


$(_BIN)\OTMGetReportData.EXE:
    @echo ---- Linking $(_DLL)\OTMGetReportData.EXE
    @echo ---- Linking $(_DLL)\OTMGetReportData.EXE >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
                          $(_OBJ)\OTMGetReportData.OBJ
/OUT:$(_BIN)\OTMGetReportData.EXE
/MAP:$(_MAP)\OTMGetReportData.MAP 
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE)
<<

$(_OBJ)\TestAPI.OBJ:	$(_SRC)\api\TestAPI.C

$(_BIN)\TestAPI.EXE:	$(_OBJ)\TestAPI.OBJ $(_LIB)\OtmFunc.LIB


$(_BIN)\TestAPI.EXE:
    @echo ---- Linking $(_DLL)\TestAPI.EXE
    @echo ---- Linking $(_DLL)\TestAPI.EXE >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
                          $(_OBJ)\TestAPI.OBJ
/OUT:$(_BIN)\TestAPI.EXE
/MAP:$(_MAP)\TestAPI.MAP 
$(_LINK_OPTIONS) $(_LIB)\OtmFUNC.LIB
$(_LINK_LIB_EXE)
<<
