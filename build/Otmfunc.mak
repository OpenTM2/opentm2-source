# OTMFUNC.MAK
#    Description:     --- NON-DDE Batch interface DLL
# Copyright (c) 2014, International Business Machines
# Corporation and others.  All rights reserved.

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------
build:    $(_LIB)\OTMFUNC.DLL \
          $(_BIN)\OTMGetReportData.EXE

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
$(_LINK_LIB_CRT) $(_LIB)\OtmAlloc.LIB $(_LIB)\OTMBase.LIB $(_LIB)\OTMITMD.LIB $(_LIB)\OTMDll.LIB $(_LIB)\PluginManager.LIB
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
