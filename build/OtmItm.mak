#-------------------------------------------------------------------------------
# EQFITM.MAK
#-------------------------------------------------------------------------------
# Copyright Notice:
#
#     Copyright (C) 1990-2017, International Business Machines
#     Corporation and others. All rights reserved
#-------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#-------------------------------------------------------------------------------
# target list
#-------------------------------------------------------------------------------
build:    $(_DLL)\OTMITMD.DLL \
          $(_BIN)\OtmItm.EXE

#-------------------------------------------------------------------------------
# dependencies
#-------------------------------------------------------------------------------

$(_OBJ)\EQFITM.obj:             $(_SRC)\tools\itm\EQFITM.C

$(_OBJ)\eqfitmt.obj:            $(_SRC)\tools\itm\eqfitmt.C

$(_OBJ)\eqfitmd.obj:            $(_SRC)\tools\itm\eqfitmd.C

$(_OBJ)\eqfitmf.obj:            $(_SRC)\tools\itm\eqfitmf.C

$(_OBJ)\eqfitmv.obj:            $(_SRC)\tools\itm\eqfitmv.C

$(_OBJ)\eqfitmv2.obj:           $(_SRC)\tools\itm\eqfitmv2.C

$(_OBJ)\eqfitml.obj:            $(_SRC)\tools\itm\eqfitml.C

$(_OBJ)\eqfitmc.obj:            $(_SRC)\tools\itm\eqfitmc.C

$(_OBJ)\eqfitms.obj:            $(_SRC)\tools\itm\eqfitms.C

#$(_OBJ)\InitPlugins.obj:        $(_SRC)\tools\itm\InitPlugins.CPP

$(_OBJ)\eqfsetex.obj:           $(_SRC)\tools\common\eqfsetex.C


OBJSITM=$(_OBJ)\EQFITM.obj  $(_OBJ)\eqfitmt.obj $(_OBJ)\eqfitmd.obj  \
        $(_OBJ)\eqfitmf.obj $(_OBJ)\eqfitmv.obj $(_OBJ)\eqfitmv2.obj \
        $(_OBJ)\eqfitml.obj $(_OBJ)\eqfitmc.obj 

#------ build EqfITM.exe --------
$(_BIN)\OtmItm.exe : $(_OBJ)\eqfitms.obj $(_RC)\EQFITM.RC $(_OBJ)\eqfsetex.obj $(_DLL)\OTMITMD.DLL 
    @echo ---- Linking  OtmItm.EXE
    @echo ---- Linking  OtmItm.EXE >>$(_ERR)
    RC /D_WINDOWS /Fo$(_OBJ)\EQFITM.RES $(_RC)\EQFITM.RC 
    CVTRES /MACHINE:IX86 /NOLOGO /OUT:$(_OBJ)\EQFITM.RBJ $(_OBJ)\EQFITM.RES
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\eqfitms.obj
$(_OBJ)\eqfsetex.obj

$(_OBJ)\eqfitm.rbj
/OUT:$(_BIN)\OtmItm.exe $(_LINK_OPTIONS)
$(_LIB)\OtmBase.lib 
$(_LIB)\OtmWorkbench.lib 
$(_LINK_LIB_EXE) 
$(_LIB)\PluginManager.lib 
$(_LIB)\OTMITMD.LIB
<<

#------ build OTMITMD.DLL --------
$(_DLL)\OTMITMD.DLL : $(OBJSITM) $(_OBJ)\eqflogo.obj  $(_OBJ)\eqfsetex.obj
    @echo ---- Linking  OTMITMD.DLL
    @echo ---- Linking  OTMITMD.DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(OBJSITM)
$(_OBJ)\eqflogo.obj
$(_OBJ)\eqfsetex.obj
/OUT:$(_DLL)\OTMITMD.DLL $(_LINK_OPTIONS) /MAPINFO:EXPORTS
$(_LINK_LIB_CRT)
$(_LIB)\OtmBase.lib 
$(_LIB)\OTMEditorFunctions.lib 
$(_LIB)\OtmWorkbench.lib 
$(_LIB)\OTMDictionaryFunctions.lib 
$(_LIB)\OTMMemoryFunctions.lib 
$(_LIB)\OtmLinguistic.lib 
$(_LIB)\OTMAnalysisFunctions.lib 
$(_LIB)\OTMTagTableFunctions.lib 
$(_LIB)\OtmUtilities.lib 
$(_LIB)\OTMFolderFunctions.lib 
$(_LIB)\OtmSegmentedFile.lib 
SHELL32.LIB 
$(_LIB)\PluginManager.lib
<<
