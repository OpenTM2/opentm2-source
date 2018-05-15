# Copyright (c) 1999-2017, International Business Machines
# Corporation and others.  All rights reserved.
#
# Make file for OpenTM2 commandline tools

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:    $(_BIN)\OtmTmxSplitSegments.EXE \
          $(_BIN)\OtmChangeFxp.EXE \
          $(_BIN)\OtmStampProfile.EXE \
          $(_BIN)\OtmShowFxp.EXE \
          $(_BIN)\OtmAdl.EXE \
          $(_BIN)\OtmMtEval.EXE \
          $(_BIN)\OtmChkCalc.EXE \
          $(_BIN)\OtmCreateITMFromMemory.EXE \
          $(_BIN)\OtmRemoveTags.EXE \
          $(_BIN)\OtmMemoryTool.EXE \
          $(_BIN)\OtmTmx2Exp.EXE \
          $(_BIN)\OtmExp2Tmx.EXE \
          $(_BIN)\OtmXliff2Exp.EXE \
          $(_BIN)\OtmIsOpenTM2FXP.EXE \
          $(_BIN)\TM2OTMMigrator.EXE \
          $(_BIN)\OtmTmxSource2Text.EXE \
          $(_BIN)\OtmCheckRegistry.EXE 

#------------------------------------------------------------------------------
# OtmIsOpenTM2FXP - Test if an FXP package has been created by OpenTM2        -   
#------------------------------------------------------------------------------
$(_OBJ)\OtmIsOpenTM2FXP.obj: $(_SRC)\tools\commandline\OtmIsOpenTM2FXP.c

$(_BIN)\OtmIsOpenTM2FXP.exe: $(_OBJ)\OtmIsOpenTM2FXP.obj
    @echo ---- Linking $(_BIN)\OtmIsOpenTM2FXP.EXE
    @echo ---- Linking $(_BIN)\OtmIsOpenTM2FXP.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OtmIsOpenTM2FXP.obj
/OUT:$(_BIN)\OtmIsOpenTM2FXP.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE)
<<

#------------------------------------------------------------------------------
# OtmCheckRegistry - Check (and correct) registry entries used by OpenTM2     -   
#------------------------------------------------------------------------------
$(_OBJ)\OtmCheckRegistry.obj: $(_SRC)\tools\commandline\OtmCheckRegistry.c

$(_BIN)\OtmCheckRegistry.exe: $(_OBJ)\OtmCheckRegistry.obj
    @echo ---- Linking $(_BIN)\OtmCheckRegistry.EXE
    @echo ---- Linking $(_BIN)\OtmCheckRegistry.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OtmCheckRegistry.obj
/OUT:$(_BIN)\OtmCheckRegistry.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE)
<<

#------------------------------------------------------------------------------
# OtmTmxSplitSegments - Split TMX segments at sentence boundaries                -
#------------------------------------------------------------------------------
$(_OBJ)\TMXSplitSegments.exe: $(_SRC)\tools\commandline\TMXSplitSegments.obj

$(_BIN)\OtmTmxSplitSegments.exe: $(_OBJ)\TMXSplitSegments.obj
    @echo ---- Linking $(_BIN)\OtmTmxSplitSegments.EXE
    @echo ---- Linking $(_BIN)\OtmTmxSplitSegments.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\TMXSplitSegments.obj
/OUT:$(_BIN)\OtmTmxSplitSegments.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE)
<<

#------------------------------------------------------------------------------
# OtmChangeFxp - Change type of exported folders                                 -
#------------------------------------------------------------------------------
$(_OBJ)\OtmChangeFxp.obj: $(_SRC)\tools\commandline\OtmChangeFxp.c

$(_BIN)\OtmChangeFxp.exe: $(_OBJ)\OtmChangeFxp.obj
    @echo ---- Linking $(_BIN)\OtmChangeFxp.EXE
    @echo ---- Linking $(_BIN)\OtmChangeFxp.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OtmChangeFxp.obj
/OUT:$(_BIN)\OtmChangeFxp.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE)
<<

#------------------------------------------------------------------------------
# OtmChkCalc - Check history log for loss of calculation report data             -
#------------------------------------------------------------------------------
$(_OBJ)\OtmChkCalc.obj: $(_SRC)\tools\commandline\OtmChkCalc.c

$(_BIN)\OtmChkCalc.exe: $(_OBJ)\OtmChkCalc.obj
    @echo ---- Linking $(_BIN)\OtmChkCalc.EXE
    @echo ---- Linking $(_BIN)\OtmChkCalc.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OtmChkCalc.obj
/OUT:$(_BIN)\OtmChkCalc.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE)
<<


#------------------------------------------------------------------------------
# OtmAdl - Correct drive letter in property files                             -
#------------------------------------------------------------------------------
$(_OBJ)\OtmAdl.obj: $(_SRC)\tools\commandline\OtmAdl.c

$(_BIN)\OtmAdl.exe: $(_OBJ)\OtmAdl.obj
    @echo ---- Linking $(_BIN)\OtmAdl.EXE
    @echo ---- Linking $(_BIN)\OtmAdl.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OtmAdl.obj
/OUT:$(_BIN)\OtmAdl.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE) 
$(_LIB)\OTMBASE.LIB
$(_LIB)\OTMUtilities.LIB
$(_LIB)\OTMWorkbench.LIB
<<



#------------------------------------------------------------------------------
# OtmCreateITMFromMemory - Create a source-source memory from a NLV memory    -
#------------------------------------------------------------------------------
$(_OBJ)\OtmCreateITMFromMemory.OBJ : $(_SRC)\tools\commandline\OtmCreateITMFromMemory.cpp

$(_BIN)\OtmCreateITMFromMemory.exe: $(_OBJ)\OtmCreateITMFromMemory.obj
    @echo ---- Linking $(_BIN)\OtmCreateITMFromMemory.EXE
    @echo ---- Linking $(_BIN)\OtmCreateITMFromMemory.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OtmCreateITMFromMemory.obj
/OUT:$(_BIN)\OtmCreateITMFromMemory.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE) 
$(_LIB)\OTMFUNC.LIB 
$(_LIB)\OTMBASE.LIB 
$(_LIB)\OTMMemoryFunctions.lib 
$(_LIB)\PluginManager.LIB
<<


#------------------------------------------------------------------------------
# OtmRemoveTags - Remove inline tagging from memory proposals                 -
#------------------------------------------------------------------------------
$(_OBJ)\OTMREMOVETAGS.OBJ : $(_SRC)\tools\commandline\OTMREMOVETAGS.cpp

$(_BIN)\OtmRemoveTags.exe: $(_OBJ)\OTMREMOVETAGS.obj
    @echo ---- Linking $(_BIN)\OtmRemoveTags.EXE
    @echo ---- Linking $(_BIN)\OtmRemoveTags.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OTMREMOVETAGS.obj
/OUT:$(_BIN)\OtmRemoveTags.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE) 
$(_LIB)\OTMFUNC.LIB 
$(_LIB)\OTMBASE.LIB 
$(_LIB)\OTMMemoryFunctions.lib 
$(_LIB)\OTMTagTableFunctions.lib 
$(_LIB)\PluginManager.LIB
<<

#------------------------------------------------------------------------------
# OtmMemoryTool                                                               -
#------------------------------------------------------------------------------
$(_OBJ)\OtmMemoryTool.OBJ : $(_SRC)\tools\commandline\OtmMemoryTool.cpp						   			   
$(_OBJ)\eqfsetex.OBJ :     $(_SRC)\tools\common\eqfsetex.c
$(_OBJ)\InitPlugins.obj:   $(_SRC)\tools\common\InitPlugins.CPP

$(_BIN)\OtmMemoryTool.exe: $(_OBJ)\OtmMemoryTool.obj $(_OBJ)\eqfsetex.obj $(_OBJ)\InitPlugins.obj
    @echo ---- Linking $(_BIN)\OtmMemoryTool.EXE
    @echo ---- Linking $(_BIN)\OtmMemoryTool.EXE >>$(_ERR)
	RC $(_RC_OPT) /Fo$(_RES)\OtmMemoryTool.RES $(_SRC)\tools\commandline\OTMMEMORYTOOL.RC
	CVTRES /NOLOGO /OUT:$(_OBJ)\OtmMemoryTool.RBJ $(_RES)\OtmMemoryTool.RES
     @$(_LINKER) >>$(_ERR) @<<lnk.rsp
 $(_OBJ)\OtmMemoryTool.obj   $(_OBJ)\eqfsetex.obj $(_OBJ)\InitPlugins.obj
 $(_OBJ)\OtmMemoryTool.RBJ
 /OUT:$(_BIN)\OtmMemoryTool.exe
 /MAP:$(_MAP)\OtmMemoryTool.map $(_LINK_OPTIONS)
 $(_LINK_LIB_EXE) 
 $(_LIB)\OTMBASE.LIB 
 $(_LIB)\PluginManager.LIB 
 $(_LIB)\OTMFUNC.LIB 
 $(_LIB)\OTMMemoryFunctions.lib 
 $(_LIB)\OTMWorkbench.LIB 
 $(_LIB)\OTMUtilities.LIB
 $(_LIB)\OTMTagTableFunctions.lib  
 $(_LIB)\OTMEditorFunctions.lib  
 $(_LIB)\OTMDictionaryFunctions.lib  
<<

#------------------------------------------------------------------------------
# TMX2EXP - convert a TMX memory into the EXP format                          -
#------------------------------------------------------------------------------
$(_OBJ)\OtmTmx2Exp.OBJ : $(_SRC)\tools\commandline\TMX2EXP.c

$(_BIN)\OtmTmx2Exp.exe: $(_OBJ)\TMX2EXP.obj
    @echo ---- Linking $(_BIN)\OtmTmx2Exp.EXE
    @echo ---- Linking $(_BIN)\OtmTmx2Exp.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\TMX2EXP.obj
/OUT:$(_BIN)\OtmTmx2Exp.exe
$(_LINK_OPTIONS) 
$(_LINK_LIB_EXE) $(_LIB)\OTMFUNC.LIB $(_LIB)\OTMTMXIE.LIB $(_LIB)\OTMBASE.LIB
<<

#------------------------------------------------------------------------------
# OtmTmxSource2Text - extract source text from a TMX memory                      -
#------------------------------------------------------------------------------
$(_OBJ)\TmxSource2Text.obj: $(_SRC)\tools\commandline\TmxSource2Text.c

$(_BIN)\OtmTmxSource2Text.exe: $(_OBJ)\TmxSource2Text.obj
    @echo ---- Linking $(_BIN)\OtmTmxSource2Text.EXE
    @echo ---- Linking $(_BIN)\OtmTmxSource2Text.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\TmxSource2Text.obj
/OUT:$(_BIN)\OtmTmxSource2Text.exe
$(_LINK_OPTIONS) 
$(_LINK_LIB_EXE)
<<


#------------------------------------------------------------------------------
# EXP2TMX - convert a EXP memory into the TMX format                          -
#------------------------------------------------------------------------------
$(_OBJ)\EXP2TMX.OBJ : $(_SRC)\tools\commandline\EXP2TMX.c

$(_BIN)\OtmExp2Tmx.exe: $(_OBJ)\EXP2TMX.obj
    @echo ---- Linking $(_BIN)\OtmExp2Tmx.EXE
    @echo ---- Linking $(_BIN)\OtmExp2Tmx.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\EXP2TMX.obj
/OUT:$(_BIN)\OtmExp2Tmx.exe
$(_LINK_OPTIONS) 
$(_LINK_LIB_EXE) $(_LIB)\OTMFUNC.LIB $(_LIB)\OTMTMXIE.LIB
<<

#------------------------------------------------------------------------------
# XLIFF2EXP - convert an XLIFF (MT) memory into the EXP format                          -
#------------------------------------------------------------------------------
$(_OBJ)\XLIFF2EXP.OBJ : $(_SRC)\tools\commandline\XLIFF2EXP.c

$(_BIN)\OtmXliff2Exp.exe: $(_OBJ)\XLIFF2EXP.obj
    @echo ---- Linking $(_BIN)\OtmXliff2Exp.EXE
    @echo ---- Linking $(_BIN)\OtmXliff2Exp.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\XLIFF2EXP.obj
/OUT:$(_BIN)\OtmXliff2Exp.exe
$(_LINK_OPTIONS) 
$(_LINK_LIB_EXE) $(_LIB)\OTMFUNC.LIB $(_LIB)\OTMXLFMT.LIB $(_LIB)\OTMBASE.LIB
<<

#------------------------------------------------------------------------------
# OtmMtEval - MT LOG Evaluation TOOL                                             -
#------------------------------------------------------------------------------
$(_OBJ)\MTEVAL.obj: $(_SRC)\tools\commandline\MTEVAL.cpp $(_INC)\cxmlwriter.h $(_SRC)\tools\commandline\OTMCFXP.h

$(_OBJ)\OTMCFXP.obj: $(_SRC)\tools\commandline\OTMCFXP.cpp  $(_SRC)\tools\commandline\OTMCFXP.h

$(_OBJ)\cxmlwriter.obj: $(_SRC)\tools\cxmlwriter.cpp $(_INC)\cxmlwriter.h

$(_BIN)\OtmMtEval.exe: $(_OBJ)\MTEVAL.obj $(_OBJ)\cxmlwriter.obj $(_OBJ)\OTMCFXP.obj
    @echo ---- Linking $(_BIN)\OtmMtEval.EXE
    @echo ---- Linking $(_BIN)\OtmMtEval.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\MTEVAL.obj $(_OBJ)\cxmlwriter.obj $(_OBJ)\OTMCFXP.obj
/OUT:$(_BIN)\OtmMtEval.exe 
$(_LINK_OPTIONS) 
$(_LINK_LIB_EXE)
<<

#------------------------------------------------------------------------------
# OtmShowFxp - Show information of an exported folder                            -
#------------------------------------------------------------------------------
$(_OBJ)\OtmShowFxp.obj: $(_SRC)\tools\commandline\OtmShowFxp.c

$(_BIN)\OtmShowFxp.exe: $(_OBJ)\OtmShowFxp.obj
    @echo ---- Linking $(_BIN)\OtmShowFxp.EXE
    @echo ---- Linking $(_BIN)\OtmShowFxp.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OtmShowFxp.obj
/OUT:$(_BIN)\OtmShowFxp.exe
$(_LINK_OPTIONS) 
$(_LINK_LIB_EXE)
<<

#------------------------------------------------------------------------------
# StampProfile - Stamp and protect calculation profiles                       -
#------------------------------------------------------------------------------
$(_OBJ)\STAMPPROFILE.obj: $(_SRC)\tools\commandline\STAMPPROFILE.c

$(_BIN)\OtmStampProfile.exe: $(_OBJ)\STAMPPROFILE.obj
    @echo ---- Linking $(_BIN)\OtmStampProfile.EXE
    @echo ---- Linking $(_BIN)\OtmStampProfile.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\STAMPPROFILE.obj
/OUT:$(_BIN)\OtmStampProfile.exe
$(_LINK_OPTIONS) 
$(_LINK_LIB_EXE)
<<

#------------------------------------------------------------------------------
# TM2OTMMigrator - Migrate TranslationManager data to OpenTM2                 -
#------------------------------------------------------------------------------
$(_OBJ)\TM2OTMMigrator.obj: $(_SRC)\tools\commandline\TM2OTMMigrator.cpp

$(_BIN)\TM2OTMMigrator.exe: $(_OBJ)\TM2OTMMigrator.obj
    @echo ---- Linking $(_BIN)\TM2OTMMigrator.EXE
    @echo ---- Linking $(_BIN)\TM2OTMMigrator.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\TM2OTMMigrator.obj
/OUT:$(_BIN)\TM2OTMMigrator.exe
$(_LINK_OPTIONS) 
$(_LINK_LIB_EXE)
<<




