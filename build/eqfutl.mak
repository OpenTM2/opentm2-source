#------------------------------------------------------------------------------
#   EQFDLL.MAK    - Makefile for Startup code and resource DLL
#------------------------------------------------------------------------------
#   Copyright Notice:
#
#           Copyright (C) 1990-2012, International Business Machines
#           Corporation and others. All rights reserved
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules  from BUILDER
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------


build:    $(_DLL)\EQFUTL.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

# source\core\utilities

$(_OBJ)\EQFUTCLB.OBJ:	$(_SRC)\core\utilities\EQFUTCLB.C
$(_OBJ)\EQFUTDLG.OBJ:	$(_SRC)\core\utilities\EQFUTDLG.C
$(_OBJ)\EQFUTDOS.OBJ:	$(_SRC)\core\utilities\EQFUTDOS.C
$(_OBJ)\EQFUTERR.OBJ:	$(_SRC)\core\utilities\EQFUTERR.C
$(_OBJ)\EQFUTFIL.OBJ:	$(_SRC)\core\utilities\EQFUTFIL.C
$(_OBJ)\EQFUTILS.OBJ:	$(_SRC)\core\utilities\EQFUTILS.C
$(_OBJ)\EQFUTLNG.OBJ:	$(_SRC)\core\utilities\EQFUTLNG.C
$(_OBJ)\EQFUTPRT.OBJ:	$(_SRC)\core\utilities\EQFUTPRT.C
$(_OBJ)\EQFUTMDI.OBJ:	$(_SRC)\core\utilities\EQFUTMDI.C
$(_OBJ)\EQFPROPS.OBJ:	$(_SRC)\core\utilities\EQFPROPS.C
$(_OBJ)\EQFOBJ00.OBJ:	$(_SRC)\core\utilities\EQFOBJ00.C
$(_OBJ)\EQFPRO00.OBJ:	$(_SRC)\core\utilities\EQFPRO00.C
$(_OBJ)\EQFHASH.OBJ:	$(_SRC)\core\utilities\EQFHASH.C
$(_OBJ)\EQFCMPR.OBJ:	$(_SRC)\core\utilities\EQFCMPR.C
$(_OBJ)\EQFPROGR.OBJ:	$(_SRC)\core\utilities\EQFPROGR.C
$(_OBJ)\EQFAPROF.OBJ:	$(_SRC)\core\utilities\EQFAPROF.C
$(_OBJ)\CSTUB.OBJ:		$(_SRC)\core\utilities\CSTUB.C
$(_OBJ)\EQFCSTUB.OBJ:	$(_SRC)\core\utilities\EQFCSTUB.C
$(_OBJ)\EQFOSWIN.OBJ:	$(_SRC)\core\utilities\EQFOSWIN.C
$(_OBJ)\EQFNOISE.OBJ:	$(_SRC)\core\utilities\EQFNOISE.C
$(_OBJ)\EQFSETUP.OBJ:	$(_SRC)\core\utilities\EQFSETUP.C


$(_DLL)\EQFUTL.DLL:		$(_DEF)\EQFUTL.$(_DEFEXT) \
						$(_OBJ)\EQFUTCLB.OBJ \
						$(_OBJ)\EQFUTDLG.OBJ \
						$(_OBJ)\EQFUTDOS.OBJ \
						$(_OBJ)\EQFUTERR.OBJ \
						$(_OBJ)\EQFUTFIL.OBJ \
						$(_OBJ)\EQFUTILS.OBJ \
						$(_OBJ)\EQFUTLNG.OBJ \
						$(_OBJ)\EQFUTPRT.OBJ \
						$(_OBJ)\EQFUTMDI.OBJ \
						$(_OBJ)\EQFPROPS.OBJ \
						$(_OBJ)\EQFOBJ00.OBJ \
						$(_OBJ)\EQFPRO00.OBJ \
						$(_OBJ)\EQFHASH.OBJ \
						$(_OBJ)\EQFCMPR.OBJ \
						$(_OBJ)\EQFPROGR.OBJ \
						$(_OBJ)\EQFAPROF.OBJ \
						$(_OBJ)\CSTUB.OBJ \
						$(_OBJ)\EQFCSTUB.OBJ \
						$(_OBJ)\EQFOSWIN.OBJ \
						$(_OBJ)\EQFNOISE.OBJ \
						$(_OBJ)\EQFSETUP.OBJ

#------------------------------------------------------------------------------
# Build EQFUTL.DLL                                                          -
#------------------------------------------------------------------------------


$(_DLL)\EQFUTL.DLL:
    @echo ---- Linking $(_DLL)\EQFUTL.DLL
    @echo ---- Linking $(_DLL)\EQFUTL.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
						$(_OBJ)\EQFUTCLB.OBJ 
						$(_OBJ)\EQFUTDLG.OBJ 
						$(_OBJ)\EQFUTDOS.OBJ 
						$(_OBJ)\EQFUTERR.OBJ 
						$(_OBJ)\EQFUTFIL.OBJ 
						$(_OBJ)\EQFUTILS.OBJ 
						$(_OBJ)\EQFUTLNG.OBJ 
						$(_OBJ)\EQFUTPRT.OBJ 
						$(_OBJ)\EQFUTMDI.OBJ 
						$(_OBJ)\EQFPROPS.OBJ 
						$(_OBJ)\EQFOBJ00.OBJ 
						$(_OBJ)\EQFPRO00.OBJ 
						$(_OBJ)\EQFHASH.OBJ 
						$(_OBJ)\EQFCMPR.OBJ 
						$(_OBJ)\EQFPROGR.OBJ 
						$(_OBJ)\EQFAPROF.OBJ 
						$(_OBJ)\CSTUB.OBJ 
						$(_OBJ)\EQFCSTUB.OBJ 
						$(_OBJ)\EQFOSWIN.OBJ 
						$(_OBJ)\EQFNOISE.OBJ 
						$(_OBJ)\EQFSETUP.OBJ
/OUT:$(_DLL)\EQFUTL.DLL
/MAP:$(_MAP)\EQFUTL.MAP $(_LINK_OPTIONS) /DLL
$(_LINK_LIB_CRT)
/DEF:$(_DEF)\EQFUTL.$(_DEFEXT)
<<
