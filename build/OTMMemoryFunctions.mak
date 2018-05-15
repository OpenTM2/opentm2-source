#------------------------------------------------------------------------------
# OTMMemoryFunctions.MAK - Makefile for the OTMMemoryFunctions.DLL
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

build:    $(_DLL)\OTMMemoryFunctions.DLL

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\EQFMECLM.OBJ:	$(_SRC)\core\memory\EQFMECLM.CPP
$(_OBJ)\EqfFilterNoMatchFile.OBJ:	$(_SRC)\core\memory\EqfFilterNoMatchFile.CPP
$(_OBJ)\EQFMEMCD.OBJ:	$(_SRC)\core\memory\EQFMEMCD.CPP
$(_OBJ)\EQFMEMED.OBJ:	$(_SRC)\core\memory\EQFMEMED.CPP
$(_OBJ)\EQFMEMEP.OBJ:	$(_SRC)\core\memory\EQFMEMEP.CPP
$(_OBJ)\EQFMEMIP.OBJ:	$(_SRC)\core\memory\EQFMEMIP.CPP
$(_OBJ)\EQFMEMLD.OBJ:	$(_SRC)\core\memory\EQFMEMLD.CPP
$(_OBJ)\EQFMEMLP.OBJ:	$(_SRC)\core\memory\EQFMEMLP.CPP
$(_OBJ)\EQFMEMMD.OBJ:	$(_SRC)\core\memory\EQFMEMMD.CPP
$(_OBJ)\EQFMEMMP.OBJ:	$(_SRC)\core\memory\EQFMEMMP.CPP
$(_OBJ)\EQFMEMRP.OBJ:	$(_SRC)\core\memory\EQFMEMRP.CPP
$(_OBJ)\EQFMEMAddMatchSegID :	$(_SRC)\core\memory\EQFMEMAddMatchSegID.CPP
$(_OBJ)\EQFTMFUN.OBJ:		$(_SRC)\core\memory\EQFTMFUN.CPP
$(_OBJ)\GenericTagReplace.C:	$(_SRC)\core\memory\GenericTagReplace.C
$(_OBJ)\TMPluginWrapper.OBJ:	$(_SRC)\core\memory\TMPluginWrapper.cpp
$(_OBJ)\MemoryFactory.OBJ:	$(_SRC)\core\memory\MemoryFactory.cpp

$(_DLL)\OTMMemoryFunctions.DLL:		\
						$(_OBJ)\EQFMECLM.OBJ \
						$(_OBJ)\EQFMEMCD.OBJ \
						$(_OBJ)\EQFMEMED.OBJ \
						$(_OBJ)\EQFMEMEP.OBJ \
						$(_OBJ)\EQFMEMIP.OBJ \
						$(_OBJ)\EQFMEMLD.OBJ \
						$(_OBJ)\EQFMEMLP.OBJ \
						$(_OBJ)\EQFMEMMD.OBJ \
						$(_OBJ)\EQFMEMMP.OBJ \
						$(_OBJ)\EQFMEMAddMatchSegID.OBJ \
						$(_OBJ)\EQFMEMRP.OBJ \
						$(_OBJ)\EQFTMFUN.OBJ \
						$(_OBJ)\MemoryFactory.OBJ \
            $(_OBJ)\EqfFilterNoMatchFile.OBJ \
						$(_OBJ)\GenericTagReplace.OBJ \
						$(_OBJ)\TMPluginWrapper.OBJ 


#------------------------------------------------------------------------------
# Build OTMMemoryFunctions.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMMemoryFunctions.DLL:
    @echo ---- Linking $(_DLL)\OTMMemoryFunctions.DLL
    @echo ---- Linking $(_DLL)\OTMMemoryFunctions.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFMECLM.OBJ 
						$(_OBJ)\EQFMEMCD.OBJ 
						$(_OBJ)\EQFMEMED.OBJ 
						$(_OBJ)\EQFMEMEP.OBJ 
						$(_OBJ)\EQFMEMIP.OBJ 
						$(_OBJ)\EQFMEMLD.OBJ 
						$(_OBJ)\EQFMEMLP.OBJ 
						$(_OBJ)\EQFMEMMD.OBJ 
						$(_OBJ)\EQFMEMMP.OBJ 
						$(_OBJ)\EQFMEMAddMatchSegID.OBJ 
						$(_OBJ)\EQFMEMRP.OBJ 
						$(_OBJ)\EQFTMFUN.OBJ 
						$(_OBJ)\MemoryFactory.OBJ 
            $(_OBJ)\EqfFilterNoMatchFile.OBJ 
						$(_OBJ)\GenericTagReplace.OBJ 
						$(_OBJ)\TMPluginWrapper.OBJ 
/OUT:$(_DLL)\OTMMemoryFunctions.DLL
/MAP:$(_MAP)\OTMMemoryFunctions.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) imm32.lib 
     SHELL32.LIB 
     COMCTL32.LIB 
     oleaut32.lib 
     uuid.lib 
     ole32.lib 
     $(_LIB)\OtmBase.lib 
     $(_LIB)\OtmAPI.lib 
     $(_LIB)\PluginManager.lib 
     $(_LIB)\OTMGLOBM.lib 
     $(_LIB)\OtmLinguistic.lib 
     $(_LIB)\OTMTagTableFunctions.lib 
     $(_LIB)\OTMSegmentedFile.lib 
     $(_LIBOTHER)\xerces-c_3.lib
     $(_LIB)\OtmFuzzy.lib 
<<

