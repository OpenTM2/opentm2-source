#------------------------------------------------------------------------------
# OTMTagTableFunctions.MAK - Makefile for the OTMTagTableFunctions.DLL
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

build:    $(_DLL)\OTMTagTableFunctions.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFTEXP.OBJ:	$(_SRC)\core\tagtable\EQFTEXP.C
$(_OBJ)\EQFTIMP.OBJ:	$(_SRC)\core\tagtable\EQFTIMP.C
$(_OBJ)\EQFPARSE.OBJ:	$(_SRC)\core\tagtable\EQFPARSE.C
$(_OBJ)\EQFTOKEN.OBJ:	$(_SRC)\core\tagtable\EQFTOKEN.C
$(_OBJ)\EQFTATAG.OBJ:	$(_SRC)\core\analysis\EQFTATAG.C
$(_OBJ)\MarkupPluginMapper.OBJ:	$(_SRC)\core\tagtable\MarkupPluginMapper.CPP


$(_DLL)\OTMTagTableFunctions.DLL: $(_OBJ)\EQFPARSE.OBJ \
						$(_OBJ)\EQFTEXP.OBJ  \
						$(_OBJ)\EQFTIMP.OBJ  \
						$(_OBJ)\EQFTOKEN.OBJ \
            $(_OBJ)\EQFTATAG.OBJ \
						$(_OBJ)\MarkupPluginMapper.OBJ 

 
#------------------------------------------------------------------------------
# Build OTMTagTableFunctions.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMTagTableFunctions.DLL:
    @echo ---- Linking $(_DLL)\OTMTagTableFunctions.DLL
    @echo ---- Linking $(_DLL)\OTMTagTableFunctions.DLL >>$(_ERR)
    $(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFPARSE.OBJ 
						$(_OBJ)\EQFTEXP.OBJ  
						$(_OBJ)\EQFTIMP.OBJ  
						$(_OBJ)\EQFTOKEN.OBJ 
            $(_OBJ)\EQFTATAG.OBJ
						$(_OBJ)\MarkupPluginMapper.OBJ 
/OUT:$(_DLL)\OTMTagTableFunctions.DLL
/MAP:$(_MAP)\OTMTagTableFunctions.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmAPI.lib $(_LIB)\PluginManager.lib $(_LIB)\OtmLinguistic.lib
<<
