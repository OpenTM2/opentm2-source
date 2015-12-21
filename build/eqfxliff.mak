# +----------------------------------------------------------------------------+
# |  EQFXLIFF.MAK   - Makefile for XLIFF folder import/export module           |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |          Copyright (C) 1990-2014, International Business Machines          |
# |          Corporation and others. All rights reserved                       |
# |                                                                            |
# +----------------------------------------------------------------------------+

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build: $(_DLL)\EQFXLIFF.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------
!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\CXMLWriter.OBJ: $(_SRC)\tools\CXMLWriter.CPP $(_INC)\CXMLWriter.H $(_INC)\EQFXLIFI.H

$(_OBJ)\EQFXLIFP.OBJ: $(_SRC)\core\folder\EQFXLIFP.CPP $(_INC)\CXMLWriter.H $(_INC)\EQFXLIFI.H

$(_OBJ)\EQFXPARS.OBJ: $(_SRC)\core\folder\EQFXPARS.CPP $(_INC)\CXMLWriter.H $(_INC)\EQFXLIFI.H

$(_OBJ)\EQFXPARI.OBJ: $(_SRC)\core\folder\EQFXPARI.CPP $(_INC)\CXMLWriter.H $(_INC)\EQFXLIFI.H

$(_OBJ)\EQFXNAME.OBJ: $(_SRC)\core\folder\EQFXNAME.CPP $(_INC)\EQFXLIFI.H

$(_OBJ)\EQFXUTIL.OBJ: $(_SRC)\core\folder\EQFXUTIL.CPP $(_INC)\CXMLWriter.H $(_INC)\EQFXLIFI.H

#------ build EQFXLIFF.DLL --------
$(_DLL)\EQFXLIFF.DLL: $(_OBJ)\EQFXLIFP.OBJ $(_OBJ)\CXMLWriter.OBJ $(_OBJ)\EQFXUTIL.OBJ $(_OBJ)\EQFXNAME.OBJ  $(_OBJ)\EQFXPARS.OBJ $(_OBJ)\EQFXPARI.OBJ
    @if exist $(_DLL)\EQFXLIFF.DLL  erase $(_DLL)\EQFXLIFF.DLL
    @echo ---- Linking  EQFXLIFF.DLL
    @echo ---- Linking  EQFXLIFF.DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\EQFXLIFP.OBJ $(_OBJ)\CXMLWriter.OBJ $(_OBJ)\EQFXUTIL.OBJ $(_OBJ)\EQFXNAME.OBJ  $(_OBJ)\EQFXPARS.OBJ $(_OBJ)\EQFXPARI.OBJ
/OUT:$(_DLL)\EQFXLIFF.DLL
/MAP:$(_MAP)\EQFXLIFF.MAP $(_LINK_OPTIONS)
$(_LINK_LIB_CRT) $(_LIB)\OtmAlloc.lib $(_LIB)\OTMBASE.LIB $(_LIB)\OTMDLL.LIB $(PACKAGESLIB)\xerces-c_3.lib
<<
