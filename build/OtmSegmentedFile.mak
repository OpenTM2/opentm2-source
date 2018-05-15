#-------------------------------------------------------------------------------
# OTMSegmentedFile.mak - Makefile for OTMSegmentedFile DLL
# Containsall functions dealing with segmented document files (SSOURCE and STARGET)
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

build:    $(_DLL)\OTMSegmentedFile.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------
$(_OBJ)\OTMSegFile.OBJ:	$(_SRC)\core\editor\OTMSegFile.c
$(_OBJ)\EQFBFILE.OBJ:			$(_SRC)\core\editor\EQFBFILE.CPP
$(_OBJ)\EQFSEGMD.OBJ:	$(_SRC)\core\editor\EQFSEGMD.C

$(_DLL)\OTMSegmentedFile.DLL:	\
            $(_OBJ)\OTMSegFile.OBJ \
            $(_OBJ)\EQFSEGMD.OBJ \
            $(_OBJ)\EQFBFILE.OBJ


#------------------------------------------------------------------------------
# Build OTMBase.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMSegmentedFile.DLL:
    @echo ---- Linking $(_DLL)\OTMSegmentedFile.DLL
    @echo ---- Linking $(_DLL)\OTMSegmentedFile >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
          $(_OBJ)\OTMSegFile.OBJ
          $(_OBJ)\EQFSEGMD.OBJ
          $(_OBJ)\EQFBFILE.OBJ
/OUT:$(_DLL)\OTMSegmentedFile.DLL
/MAP:$(_MAP)\OTMSegmentedFile.MAP $(_LINK_OPTIONS) /MAPINFO:EXPORTS /DLL
          $(_LINK_LIB_CRT) 
          imm32.lib 
          $(_LIBOTHER)\xerces-c_3.lib
          $(_LIB)\OTMBase.lib
          $(_LIB)\OTMTagTableFunctions.lib
          $(_LIB)\OTMLinguistic.lib
<<
