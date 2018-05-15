#------------------------------------------------------------------------------
# OTMEditorFunctions.MAK - Makefile for the OTMEditorFunctions.DLL
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

build:    $(_DLL)\OTMEditorFunctions.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFBCLIP.OBJ:	$(_SRC)\core\editor\EQFBCLIP.C
$(_OBJ)\EQFBFUNC.OBJ:	$(_SRC)\core\editor\EQFBFUNC.C
$(_OBJ)\EQFBCFND.OBJ:	$(_SRC)\core\editor\EQFBCFND.CPP
$(_OBJ)\EQFBDLG.OBJ:	$(_SRC)\core\editor\EQFBDLG.C
$(_OBJ)\EQFBDLGF.OBJ:	$(_SRC)\core\editor\EQFBDLGF.C
$(_OBJ)\EQFBDLGP.OBJ:	$(_SRC)\core\editor\EQFBDLGP.C
$(_OBJ)\EQFBDLGS.OBJ:	$(_SRC)\core\editor\EQFBDLGS.C
$(_OBJ)\EQFBBIDI.OBJ:	$(_SRC)\core\editor\EQFBBIDI.C
$(_OBJ)\EQFBINIT.OBJ:	$(_SRC)\core\editor\EQFBINIT.C
$(_OBJ)\EQFBKEYB.OBJ:	$(_SRC)\core\editor\EQFBKEYB.C
$(_OBJ)\EQFBMAIN.OBJ:	$(_SRC)\core\editor\EQFBMAIN.C
$(_OBJ)\EQFBMARK.OBJ:	$(_SRC)\core\editor\EQFBMARK.C
$(_OBJ)\EQFBPM.OBJ:		$(_SRC)\core\editor\EQFBPM.C
$(_OBJ)\EQFBSCRN.OBJ:	$(_SRC)\core\editor\EQFBSCRN.C
$(_OBJ)\EQFBTRAN.OBJ:	$(_SRC)\core\editor\EQFBTRAN.C
$(_OBJ)\EQFBTRUT.OBJ:	$(_SRC)\core\editor\EQFBTRUT.C
$(_OBJ)\EQFBUTL.OBJ:	$(_SRC)\core\editor\EQFBUTL.CPP
$(_OBJ)\EQFBWORK.OBJ:	$(_SRC)\core\editor\EQFBWORK.C
$(_OBJ)\EQFBWPRC.OBJ:	$(_SRC)\core\editor\EQFBWPRC.C
$(_OBJ)\EQFBIDI.OBJ:	$(_SRC)\core\editor\EQFBIDI.C
$(_OBJ)\EQFBRTFF.OBJ:	$(_SRC)\core\editor\EQFBRTFF.C
$(_OBJ)\EQFBRTF.OBJ:	$(_SRC)\core\editor\EQFBRTF.C
$(_OBJ)\DocumentPluginMapper.OBJ:	$(_SRC)\core\document\DocumentPluginMapper.cpp
$(_OBJ)\ReImportDoc.OBJ:	$(_SRC)\core\editor\ReImportDoc.CPP
$(_OBJ)\EQFPAPI.OBJ:	$(_SRC)\core\tagtable\EQFPAPI.C
$(_OBJ)\EQFSEGMDGUI.OBJ:	$(_SRC)\core\editor\EQFSEGMDGUI.C
$(_OBJ)\EQF_DA.OBJ:     $(_SRC)\core\services\EQF_DA.C
$(_OBJ)\EQF_MT.OBJ:	$(_SRC)\core\services\EQF_MT.C
$(_OBJ)\EQF_TM.OBJ:	$(_SRC)\core\services\EQF_TM.CPP
$(_OBJ)\EQF_TWBS.OBJ:	$(_SRC)\core\services\EQF_TWBS.C
$(_OBJ)\EQFXDOC.OBJ:	$(_SRC)\core\services\EQFXDOC.C
$(_OBJ)\EQFDDE00.OBJ:	$(_SRC)\core\services\EQFDDE00.C
$(_OBJ)\EQFDOC00.OBJ:	$(_SRC)\core\document\EQFDOC00.C

$(_OBJ)\SpecialCharDlg.OBJ:	$(_SRC)\core\editor\SpecialCharDlg.cpp
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /D UNICODE /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\editor\SpecialCharDlg.cpp



$(_DLL)\OTMEditorFunctions.DLL: \
						$(_OBJ)\EQFBCLIP.OBJ \
						$(_OBJ)\EQFBDLG.OBJ  \
						$(_OBJ)\EQFBDLGF.OBJ \
						$(_OBJ)\EQFBDLGP.OBJ \
						$(_OBJ)\EQFBDLGS.OBJ \
						$(_OBJ)\DocumentPluginMapper.OBJ \
						$(_OBJ)\EQFBFIND.OBJ \
						$(_OBJ)\EQFBCFND.OBJ \
						$(_OBJ)\EQFBFUNC.OBJ \
						$(_OBJ)\EQFBINIT.OBJ \
						$(_OBJ)\EQFBKEYB.OBJ \
						$(_OBJ)\EQFBMAIN.OBJ \
						$(_OBJ)\EQFBMARK.OBJ \
						$(_OBJ)\EQFBPM.OBJ   \
						$(_OBJ)\EQFBSCRN.OBJ \
						$(_OBJ)\EQFBTRAN.OBJ \
						$(_OBJ)\EQFBTRUT.OBJ \
						$(_OBJ)\EQFBUTL.OBJ  \
						$(_OBJ)\EQFBWORK.OBJ \
            $(_OBJ)\ReImportDoc.OBJ \
						$(_OBJ)\EQFBWPRC.OBJ \
						$(_OBJ)\EQFBIDI.OBJ  \
						$(_OBJ)\EQFBRTFF.OBJ \
						$(_OBJ)\EQFBRTF.OBJ  \
            $(_OBJ)\EQFPAPI.OBJ \
            $(_OBJ)\EQFSEGMDGUI.OBJ \
            $(_OBJ)\SpecialCharDlg.OBJ \
						$(_OBJ)\EQFDDE00.OBJ \
						$(_OBJ)\EQF_DA.OBJ   \
						$(_OBJ)\EQF_MT.OBJ   \
						$(_OBJ)\EQF_TM.OBJ   \
						$(_OBJ)\EQF_TWBS.OBJ \
            $(_OBJ)\EQFDOC00.OBJ \
            $(_OBJ)\EQFXDOC.OBJ

#------------------------------------------------------------------------------
# Build OTMEditorFunctions.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMEditorFunctions.DLL:
    @echo ---- Linking $(_DLL)\OTMEditorFunctions.DLL
    @echo ---- Linking $(_DLL)\OTMEditorFunctions.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFBCLIP.OBJ 
						$(_OBJ)\EQFBDLG.OBJ  
						$(_OBJ)\EQFBDLGF.OBJ 
						$(_OBJ)\EQFBDLGP.OBJ 
						$(_OBJ)\EQFBDLGS.OBJ 
						$(_OBJ)\DocumentPluginMapper.OBJ 
						$(_OBJ)\EQFBFIND.OBJ 
						$(_OBJ)\EQFBCFND.OBJ 
						$(_OBJ)\EQFBFUNC.OBJ 
						$(_OBJ)\EQFBINIT.OBJ 
						$(_OBJ)\EQFBKEYB.OBJ 
						$(_OBJ)\EQFBMAIN.OBJ 
						$(_OBJ)\EQFBMARK.OBJ 
						$(_OBJ)\EQFBPM.OBJ   
						$(_OBJ)\EQFBSCRN.OBJ 
						$(_OBJ)\EQFBTRAN.OBJ 
						$(_OBJ)\EQFBTRUT.OBJ 
						$(_OBJ)\EQFBUTL.OBJ  
						$(_OBJ)\EQFBWORK.OBJ 
            $(_OBJ)\ReImportDoc.OBJ 
						$(_OBJ)\EQFBWPRC.OBJ 
						$(_OBJ)\EQFBIDI.OBJ  
						$(_OBJ)\EQFBRTFF.OBJ 
						$(_OBJ)\EQFBRTF.OBJ  
            $(_OBJ)\EQFPAPI.OBJ
						$(_OBJ)\EQFSEGMDGUI.OBJ 
            $(_OBJ)\SpecialCharDlg.OBJ
            $(_OBJ)\EQFDDE00.OBJ 
						$(_OBJ)\EQF_DA.OBJ   
						$(_OBJ)\EQF_MT.OBJ   
						$(_OBJ)\EQF_TM.OBJ   
						$(_OBJ)\EQF_TWBS.OBJ 
            $(_OBJ)\EQFDOC00.OBJ
            $(_OBJ)\EQFXDOC.OBJ
/OUT:$(_DLL)\OTMEditorFunctions.DLL
/MAP:$(_MAP)\OTMEditorFunctions.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS 
            $(_LINK_LIB_CRT) 
            imm32.lib 
            $(_LIB)\OtmBase.lib 
            $(_LIB)\OtmAPI.lib 
            $(_LIB)\PluginManager.lib 
            $(_LIB)\OTMTagTableFunctions.lib 
            $(_LIB)\OTMMemoryFunctions.lib
            $(_LIB)\OTMDictionaryFunctions.lib
            $(_LIB)\OTMLinguistic.lib
            $(_LIB)\OTMAnalysisFunctions.lib
            $(_LIB)\OTMSegmentedFile.lib
            $(_LIB)\OTMListFunctions.lib
            $(_LIB)\OtmFuzzy.lib
            $(_LIB)\OTMFolderFunctions.lib
            $(_LIB)\OTMGLOBM.lib
            imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib 
<<

