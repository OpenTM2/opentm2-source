#-------------------------------------------------------------------------------
# OTMQdam.MAK - Makefile for database access
# Copyright (c) 2013, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\OTMQDAM.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\EQFDAM.OBJ:					$(_SRC)\core\dictionary\EQFDAM.C
$(_OBJ)\EQFQDAM.OBJ:				$(_SRC)\core\dictionary\EQFQDAM.C
$(_OBJ)\EQFQDAMI.OBJ:				$(_SRC)\core\dictionary\EQFQDAMI.C
$(_OBJ)\EQFQDAMU.OBJ:				$(_SRC)\core\dictionary\EQFQDAMU.C
$(_OBJ)\EQFQDAMW.OBJ:				$(_SRC)\core\dictionary\EQFQDAMW.C
$(_OBJ)\EQFQDSRV.OBJ:				$(_SRC)\core\dictionary\EQFQDSRV.C

$(_DLL)\OTMQDAM.DLL:	$(_OBJ)\EQFDAM.OBJ \
						$(_OBJ)\EQFQDAM.OBJ \
						$(_OBJ)\EQFQDAMI.OBJ \
						$(_OBJ)\EQFQDAMU.OBJ \
						$(_OBJ)\EQFQDAMW.OBJ \
						$(_OBJ)\EQFQDSRV.OBJ

#------------------------------------------------------------------------------
# Build OTMQDAM.DLL
#------------------------------------------------------------------------------

$(_DLL)\OTMQDAM.DLL:
    @echo ---- Linking $(_DLL)\OTMQDAM.DLL
    @echo ---- Linking $(_DLL)\OTMQDAM.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
			$(_OBJ)\EQFDAM.OBJ
			$(_OBJ)\EQFQDAM.OBJ
			$(_OBJ)\EQFQDAMI.OBJ
			$(_OBJ)\EQFQDAMU.OBJ
			$(_OBJ)\EQFQDAMW.OBJ
			$(_OBJ)\EQFQDSRV.OBJ
/OUT:$(_DLL)\OTMQDAM.DLL
/MAP:$(_MAP)\EqfQdam.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmAlloc.lib $(_LIB)\OTMBase.lib $(_LIB)\OTMDll.lib
<<
