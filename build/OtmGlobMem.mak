# Copyright (c) 1999-2017, International Business Machines
# Corporation and others.  All rights reserved.
#
!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build: $(_DLL)\OTMGLOBM.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------
!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\OTMGLOBM.OBJ: $(_SRC)\core\utilities\OTMGLOBM.CPP $(_INC)\OTMGLOBMEM.H

$(_OBJ)\CXMLWriter.OBJ: $(_SRC)\tools\CXMLWriter.CPP $(_INC)\CXMLWriter.H

$(_DLL)\OTMGLOBM.dll:      $(_OBJ)\OTMGLOBM.OBJ $(_OBJ)\CXMLWriter.OBJ

$(_DLL)\OTMGLOBM.dll:
    @if exist $(_DLL)\OTMGLOBM.dll  erase $(_DLL)\OTMGLOBM.dll
    @echo ---- Linking  OTMGLOBM.DLL
    @echo ---- Linking  OTMGLOBM.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp >>$(_ERR)
$(_OBJ)\OTMGLOBM.OBJ $(_OBJ)\CXMLWriter.OBJ
/OUT:$(_DLL)\OTMGLOBM.dll /MAPINFO:EXPORTS
$(_LINK_OPTIONS) 
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIBOTHER)\xerces-c_3.lib
<<
