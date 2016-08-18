#-------------------------------------------------------------------------------
# OTMDialog.mak - Makefile for OTMDialog DLL
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

build:    $(_DLL)\OTMDialog.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\eqfutdlg.OBJ:	$(_SRC)\core\utilities\eqfutdlg.c
$(_OBJ)\eqfutmdi.OBJ:	$(_SRC)\core\utilities\eqfutmdi.c

$(_DLL)\OTMDialog.DLL:	$(_OBJ)\eqfutdlg.OBJ \
						$(_OBJ)\eqfutmdi.OBJ

#------------------------------------------------------------------------------
# Build OTMDialog.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMDialog.DLL:
    @echo ---- Linking $(_DLL)\OTMDialog.DLL
    @echo ---- Linking $(_DLL)\OTMDialog.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
					$(_OBJ)\eqfutdlg.OBJ
					$(_OBJ)\eqfutmdi.OBJ
/OUT:$(_DLL)\OTMDialog.DLL
/MAP:$(_MAP)\OTMDialog.MAP $(_LINK_OPTIONS) /MAPINFO:EXPORTS /DLL
$(_LINK_LIB_CRT) $(_LIB)\OTMBase.lib $(_LIB)\OtmAlloc.lib
<<
