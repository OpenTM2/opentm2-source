#-------------------------------------------------------------------------------
# OTMAllok.mak - Makefile for memory allocation DLL
# Copyright (c) 2012, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:    $(_DLL)\OTMAlloc.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\UtlAlloc.OBJ:	$(_SRC)\core\utilities\UtlAlloc.c \
						$(_SRC)\core\utilities\Utility.h

$(_DLL)\OTMAlloc.DLL:	$(_OBJ)\UtlAlloc.OBJ

#------------------------------------------------------------------------------
# Build OtmAlloc.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMAlloc.DLL:
    @echo ---- Linking $(_DLL)\OTMAlloc.DLL
    @echo ---- Linking $(_DLL)\OTMAlloc.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
			$(_OBJ)\UtlAlloc.OBJ
/OUT:$(_DLL)\OTMAlloc.DLL
/MAP:$(_MAP)\OTMAlloc.MAP $(_LINK_OPTIONS) /MAPINFO:EXPORTS /DLL
$(_LINK_LIB_CRT)
<<
