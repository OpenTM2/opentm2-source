#------------------------------------------------------------------------------
# OTMDictionaryFunctions.MAK - Makefile for the OTMDictionaryFunctions DLL
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

build:    $(_DLL)\OTMDictionaryFunctions.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFDORG.OBJ:	$(_SRC)\core\dictionary\EQFDORG.C
$(_OBJ)\EQFFILTC.OBJ:	$(_SRC)\core\dictionary\EQFFILTC.C
$(_OBJ)\EQFFILTD.OBJ:	$(_SRC)\core\dictionary\EQFFILTD.C
$(_OBJ)\EQFFILTH.OBJ:	$(_SRC)\core\dictionary\EQFFILTH.C
$(_OBJ)\EQFFILTU.OBJ:	$(_SRC)\core\dictionary\EQFFILTU.C
$(_OBJ)\EQFFILTW.OBJ:	$(_SRC)\core\dictionary\EQFFILTW.C
$(_OBJ)\EQFDDISP.OBJ:	$(_SRC)\core\dictionary\EQFDDISP.C
$(_OBJ)\EQFDEDIT.OBJ:	$(_SRC)\core\dictionary\EQFDEDIT.C
$(_OBJ)\EQFDEX.OBJ:		$(_SRC)\core\dictionary\EQFDEX.C
$(_OBJ)\EQFDIC01.OBJ:	$(_SRC)\core\dictionary\EQFDIC01.C
$(_OBJ)\EQFDIC02.OBJ:	$(_SRC)\core\dictionary\EQFDIC02.C
$(_OBJ)\EQFDIC03.OBJ:	$(_SRC)\core\dictionary\EQFDIC03.C
$(_OBJ)\EQFDIMP.OBJ:	$(_SRC)\core\dictionary\EQFDIMP.C
$(_OBJ)\EQFDLOOK.OBJ:	$(_SRC)\core\dictionary\EQFDLOOK.C
$(_OBJ)\EQFDLUP.OBJ:	$(_SRC)\core\dictionary\EQFDLUP.C
$(_OBJ)\EQFQDPR.OBJ:	$(_SRC)\core\dictionary\EQFQDPR.C
$(_OBJ)\EQFQDPRA.OBJ:	$(_SRC)\core\dictionary\EQFQDPRA.C
$(_OBJ)\EQFQDPRD.OBJ:	$(_SRC)\core\dictionary\EQFQDPRD.C
$(_OBJ)\EQFQDPRP.OBJ:	$(_SRC)\core\dictionary\EQFQDPRP.C
$(_OBJ)\EQFQDPRU.OBJ:	$(_SRC)\core\dictionary\EQFQDPRU.C
$(_OBJ)\EQFRDICS.OBJ:	$(_SRC)\core\dictionary\EQFRDICS.C
$(_OBJ)\EQFSDICS.OBJ:	$(_SRC)\core\dictionary\EQFSDICS.C
$(_OBJ)\EQFQLDB.OBJ:	$(_SRC)\core\dictionary\EQFQLDB.C
$(_OBJ)\EQFQLDBI.OBJ:	$(_SRC)\core\dictionary\EQFQLDBI.C
$(_OBJ)\EQFDICRC.OBJ:	$(_SRC)\core\dictionary\EQFDICRC.C
$(_OBJ)\DictPluginWrapper.OBJ:	$(_SRC)\core\dictionary\DictPluginWrapper.cpp
$(_OBJ)\EQFMORPW.OBJ:	$(_SRC)\core\linguistic\EQFMORPW.C


$(_DLL)\OTMDictionaryFunctions.DLL: \
						$(_OBJ)\EQFDDISP.OBJ \
						$(_OBJ)\EQFDEDIT.OBJ \
						$(_OBJ)\EQFDEX.OBJ   \
						$(_OBJ)\EQFDIC01.OBJ \
						$(_OBJ)\EQFDIC02.OBJ \
						$(_OBJ)\EQFDIC03.OBJ \
						$(_OBJ)\EQFDIMP.OBJ  \
						$(_OBJ)\EQFDLOOK.OBJ \
						$(_OBJ)\EQFDLUP.OBJ  \
 						$(_OBJ)\EQFQDPR.OBJ  \
						$(_OBJ)\EQFQDPRA.OBJ \
						$(_OBJ)\EQFQDPRD.OBJ \
						$(_OBJ)\EQFQDPRP.OBJ \
						$(_OBJ)\EQFQDPRU.OBJ \
						$(_OBJ)\EQFRDICS.OBJ \
						$(_OBJ)\EQFSDICS.OBJ \
						$(_OBJ)\EQFQLDB.OBJ \
						$(_OBJ)\EQFQLDBI.OBJ \
						$(_OBJ)\EQFDICRC.OBJ \
            $(_OBJ)\EQFDORG.OBJ \
            $(_OBJ)\EQFFILTC.OBJ \
            $(_OBJ)\EQFFILTD.OBJ \
            $(_OBJ)\EQFFILTH.OBJ \
            $(_OBJ)\EQFFILTU.OBJ \
            $(_OBJ)\EQFFILTW.OBJ \
            $(_OBJ)\EQFMORPW.OBJ \
						$(_OBJ)\DictPluginWrapper.OBJ 


#------------------------------------------------------------------------------
# Build OTMDictionaryFunctions.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMDictionaryFunctions.DLL:
    @echo ---- Linking $(_DLL)\OTMDictionaryFunctions.DLL
    @echo ---- Linking $(_DLL)\OTMDictionaryFunctions.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFDDISP.OBJ 
						$(_OBJ)\EQFDEDIT.OBJ 
						$(_OBJ)\EQFDEX.OBJ   
						$(_OBJ)\EQFDIC01.OBJ 
						$(_OBJ)\EQFDIC02.OBJ 
						$(_OBJ)\EQFDIC03.OBJ 
						$(_OBJ)\EQFDIMP.OBJ  
						$(_OBJ)\EQFDLOOK.OBJ 
						$(_OBJ)\EQFDLUP.OBJ  
 						$(_OBJ)\EQFQDPR.OBJ  
						$(_OBJ)\EQFQDPRA.OBJ 
						$(_OBJ)\EQFQDPRD.OBJ 
						$(_OBJ)\EQFQDPRP.OBJ 
						$(_OBJ)\EQFQDPRU.OBJ 
						$(_OBJ)\EQFRDICS.OBJ 
						$(_OBJ)\EQFSDICS.OBJ 
						$(_OBJ)\EQFQLDB.OBJ 
            $(_OBJ)\EQFQLDBI.OBJ 
						$(_OBJ)\EQFDICRC.OBJ 
            $(_OBJ)\EQFDORG.OBJ 
            $(_OBJ)\EQFFILTC.OBJ 
            $(_OBJ)\EQFFILTD.OBJ 
            $(_OBJ)\EQFFILTH.OBJ 
            $(_OBJ)\EQFFILTU.OBJ 
            $(_OBJ)\EQFFILTW.OBJ 
            $(_OBJ)\EQFMORPW.OBJ
						$(_OBJ)\DictPluginWrapper.OBJ 
/OUT:$(_DLL)\OTMDictionaryFunctions.DLL
/MAP:$(_MAP)\OTMDictionaryFunctions.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmAPI.lib $(_LIB)\PluginManager.lib $(_LIB)\OtmLinguistic.lib $(_LIB)\OTMTagTableFunctions.lib 
<<
