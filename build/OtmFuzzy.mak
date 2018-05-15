#-------------------------------------------------------------------------------
# OTMFuzzy.mak - Makefile for OTMFuzzy DLL
# Copyright (c) 2018, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:    $(_DLL)\OTMFuzzy.DLL

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------
$(_OBJ)\EQFBFUZZ.OBJ:	$(_SRC)\core\editor\EQFBFUZZ.C

$(_DLL)\OTMFuzzy.DLL:	$(_OBJ)\EQFBFUZZ.OBJ

#------------------------------------------------------------------------------
# Build OTMFuzzy.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMFuzzy.DLL:
    @echo ---- Linking $(_DLL)\OTMFuzzy.DLL
    @echo ---- Linking $(_DLL)\OTMFuzzy.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
          $(_OBJ)\EQFBFUZZ.OBJ 
/OUT:$(_DLL)\OTMFuzzy.DLL
/MAP:$(_MAP)\OTMFuzzy.MAP $(_LINK_OPTIONS) /MAPINFO:EXPORTS /DLL
          $(_LINK_LIB_CRT) 
          $(_LIB)\OTMTagTableFunctions.lib
          $(_LIB)\OtmBase.lib
          $(_LIB)\OtmLinguistic.lib
<<
