#-------------------------------------------------------------------------------
# OTMMemoryService.mak - Makefile for OTMMemoryService DLL
# Copyright (c) 2016, QSoft GmbH. All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Target                                                                      -
#------------------------------------------------------------------------------
build:    $(_BIN)\OTMMemoryService.EXE $(_BIN)\OTMMemoryServiceGUI.EXE

#$(_BIN)\OtmMemoryServiceTester.EXE 

#------------------------------------------------------------------------------
# Rules                                                                       -
#------------------------------------------------------------------------------
!INCLUDE $(_BLD)\EQFRULES.MAK

{$(_SRC)\plugins\OtmMemoryService}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\OtmMemoryService\$(*B).C
    @echo ---- Compiling $(_SRC)\plugins\OtmMemoryService\$(*B).C >>$(_ERR)
    @$(_COMPILER)  $(_CL_OPTIONS_EXE) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /I$(RESTBED_INCL) /Fe$(_BIN)\ $(_SRC)\plugins\OtmMemoryService\$(*B).c

{$(_SRC)\plugins\OtmMemoryService}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\OtmMemoryService\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\OtmMemoryService\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /I$(RESTBED_INCL) /Fe$(_BIN)\ $(_SRC)\plugins\OtmMemoryService\$(*B).cpp


#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------
$(_OBJ)\OtmMemoryServiceWorker.OBJ:	$(_SRC)\plugins\OtmMemoryService\OtmMemoryServiceWorker.CPP $(_SRC)\plugins\OtmMemoryService\OTMMSJSONFactory.H $(_INC)\OTMFUNC.H
$(_OBJ)\OtmMemoryService.OBJ:	$(_SRC)\plugins\OtmMemoryService\OtmMemoryService.CPP $(_SRC)\plugins\OtmMemoryService\OTMMSJSONFactory.H 
$(_OBJ)\OtmMemoryServiceGUI.OBJ:	$(_SRC)\plugins\OtmMemoryService\OtmMemoryServiceGUI.CPP $(_SRC)\plugins\OtmMemoryService\OTMMemoryService.h
$(_OBJ)\OTMMSJSONFactory.OBJ:	$(_SRC)\plugins\OtmMemoryService\OTMMSJSONFactory.CPP $(_SRC)\plugins\OtmMemoryService\OTMMSJSONFactory.H
$(_BIN)\OTMMemoryServiceGUI.EXE: $(_OBJ)\OtmMemoryService.OBJ $(_OBJ)\OtmMemoryServiceWorker.OBJ $(_OBJ)\OTMMSJSONFactory.OBJ $(_LIB)\OtmFunc.lib $(_OBJ)\OTMMemoryServiceGUI.OBJ 
#$(_OBJ)\OTMMemoryServiceTester.OBJ:	$(_SRC)\plugins\OtmMemoryService\OTMMemoryServiceTester.CPP $(_SRC)\plugins\OtmMemoryService\OtmMemoryServiceWorker.H
#$(_BIN)\OtmMemoryServiceTester.EXE: $(_OBJ)\OtmMemoryServiceTester.OBJ $(_OBJ)\OtmMemoryServiceWorker.obj
$(_OBJ)\WinService.OBJ:	$(_SRC)\plugins\OtmMemoryService\WinService.CPP 
$(_BIN)\OTMMemoryService.EXE: $(_OBJ)\WinService.OBJ $(_OBJ)\OTMMemoryService.OBJ $(_OBJ)\OTMMemoryServiceWorker.OBJ $(_OBJ)\OTMMSJSONFactory.OBJ




#------------------------------------------------------------------------------
# Build                                                                       -
#------------------------------------------------------------------------------
$(_BIN)\OTMMemoryServiceGUI.EXE:
    @echo ---- Linking $(_DLL)\OTMMemoryServiceGUI.EXE
    @echo ---- Linking $(_DLL)\OTMMemoryServiceGUI.EXE >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
	$(_OBJ)\OTMMemoryServiceGUI.OBJ
	$(_OBJ)\OTMMemoryService.OBJ
	$(_OBJ)\OTMMemoryServiceWorker.OBJ
	$(_OBJ)\OTMMSJSONFactory.OBJ
/OUT:$(_BIN)\OTMMemoryServiceGUI.EXE
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE) $(_LIB)\OtmFunc.lib $(_LIB)\OtmBase.lib $(RESTBED_LIB_DIR)\$(RESTBED_LIB) Crypt32.lib
<<

$(_BIN)\OTMMemoryService.EXE:
    @echo ---- Linking $(_DLL)\OTMMemoryService.EXE
    @echo ---- Linking $(_DLL)\OTMMemoryService.EXE >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
	$(_OBJ)\OTMMemoryService.OBJ
	$(_OBJ)\OTMMemoryServiceWorker.OBJ
	$(_OBJ)\OTMMSJSONFactory.OBJ
	$(_OBJ)\WinService.OBJ
/OUT:$(_BIN)\OTMMemoryService.EXE
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE) $(_LIB)\OtmFunc.lib $(_LIB)\OtmBase.lib $(RESTBED_LIB_DIR)\$(RESTBED_LIB) Crypt32.lib
<<


#$(_BIN)\OtmMemoryServiceTester.EXE: 
#    @echo ---- Linking $(_BIN)\$(_BIN)\OtmMemoryServiceTester.EXE
#    @echo ---- Linking $(_BIN)\$(_BIN)\OtmMemoryServiceTester.EXE >>$(_ERR)
#    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
#        $(_OBJ)\OtmMemoryServiceTester.obj
#	$(_OBJ)\OTMMemoryServiceWorker.OBJ
#	$(_OBJ)\OTMMSJSONFactory.OBJ
#/OUT:$(_BIN)\OtmMemoryServiceTester.EXE
#$(_LINK_OPTIONS)
#$(_LINK_LIB_EXE) $(_LIB)\OtmFunc.lib $(_LIB)\OtmBase.lib
#<<


