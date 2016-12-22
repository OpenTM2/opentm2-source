#-------------------------------------------------------------------------------
# OTMBase.mak - Makefile for OTMMemoryService DLL
# Copyright (c) 2016, QSoft GmbH. All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Target                                                                      -
#------------------------------------------------------------------------------
build:    $(_DLL)\OTMMemoryService.DLL

#------------------------------------------------------------------------------
# Rules                                                                       -
#------------------------------------------------------------------------------
!INCLUDE $(_BLD)\EQFRULES.MAK

{$(_SRC)\plugins\OtmMemoryService}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\OtmMemoryService\$(*B).C
    @echo ---- Compiling $(_SRC)\plugins\OtmMemoryService\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\OtmMemoryService\$(*B).c

{$(_SRC)\plugins\OtmMemoryService}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\OtmMemoryService\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OtmMemoryService\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\OtmMemoryService\$(*B).cpp

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------
$(_OBJ)\OtmMemoryService.OBJ:	$(_SRC)\plugins\OtmMemoryService\OtmMemoryService.CPP $(_SRC)\plugins\OtmMemoryService\JSONFactory.H $(_INC)\OTMFUNC.H
$(_OBJ)\JSONFactory.OBJ:	$(_SRC)\plugins\OtmMemoryService\JSONFactory.CPP $(_SRC)\plugins\OtmMemoryService\JSONFactory.H
$(_DLL)\OTMMemoryService.DLL: $(_OBJ)\OtmMemoryService.OBJ $(_OBJ)\JSONFactory.OBJ $(_LIB)\OtmFunc.lib



#------------------------------------------------------------------------------
# Build                                                                       -
#------------------------------------------------------------------------------
$(_DLL)\OTMMemoryService.DLL:
    @echo ---- Linking $(_DLL)\OTMMemoryService.DLL
    @echo ---- Linking $(_DLL)\OTMMemoryService.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
	$(_OBJ)\OTMMemoryService.OBJ
	$(_OBJ)\JSONFactory.OBJ
/OUT:$(_DLL)\OTMMemoryService.DLL
/MAP:$(_MAP)\OTMMemoryService.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmFunc.lib
<<

