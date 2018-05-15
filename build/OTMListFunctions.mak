#------------------------------------------------------------------------------
# OTMListFunctions.MAK - Makefile for the OTMListFunctions.DLL
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

build:    $(_DLL)\OTMListFunctions.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFLIST.OBJ:	$(_SRC)\core\lists\EQFLIST.C
$(_OBJ)\EQFLSTIE.OBJ:	$(_SRC)\core\lists\EQFLSTIE.C
$(_OBJ)\EQFLSTLP.OBJ:	$(_SRC)\core\lists\EQFLSTLP.C
$(_OBJ)\EQFLSTUT.OBJ:	$(_SRC)\core\lists\EQFLSTUT.C

$(_DLL)\OTMListFunctions.DLL: \
						$(_OBJ)\EQFLIST.OBJ  \
						$(_OBJ)\EQFLSTIE.OBJ \
						$(_OBJ)\EQFLSTLP.OBJ \
						$(_OBJ)\EQFLSTUT.OBJ \

#------------------------------------------------------------------------------
# Build OTMListFunctions.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMListFunctions.DLL:
    @echo ---- Linking $(_DLL)\OTMListFunctions.DLL
    @echo ---- Linking $(_DLL)\OTMListFunctions.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFLIST.OBJ  
						$(_OBJ)\EQFLSTIE.OBJ 
						$(_OBJ)\EQFLSTLP.OBJ 
						$(_OBJ)\EQFLSTUT.OBJ 
/OUT:$(_DLL)\OTMListFunctions.DLL
/MAP:$(_MAP)\OTMListFunctions.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
            $(_LINK_LIB_CRT) 
            imm32.lib SHELL32.LIB COMCTL32.LIB oleaut32.lib uuid.lib ole32.lib 
            $(_LIB)\OtmBase.lib 
            $(_LIB)\OtmAPI.lib 
            $(_LIB)\OtmLinguistic.lib 
            $(_LIB)\OTMTagTableFunctions.lib
            $(_LIB)\OTMDictionaryFunctions.lib
            $(_LIB)\OtmUtilities.lib
<<
