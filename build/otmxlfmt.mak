# Copyright (c) 1990-2017, International Business Machines
# Corporation and others.  All rights reserved.
#
!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build: $(_DLL)\OTMXLFMT.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------
!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\CXMLWriter.OBJ: $(_SRC)\tools\CXMLWriter.CPP

$(_OBJ)\OTMXLFMT.OBJ: $(_SRC)\plugins\memory\OTMXLFMT.CPP



#------ build OTMXLFMT.DLL --------
$(_DLL)\OTMXLFMT.DLL: $(_OBJ)\OTMXLFMT.OBJ $(_OBJ)\CXMLWriter.OBJ
    @if exist $(_DLL)\OTMXLFMT.DLL  erase $(_DLL)\OTMXLFMT.DLL
    @echo ---- Linking  OTMXLFMT.DLL
    @echo ---- Linking  OTMXLFMT.DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OTMXLFMT.OBJ $(_OBJ)\CXMLWriter.OBJ
/OUT:$(_DLL)\OTMXLFMT.DLL /MAPINFO:EXPORTS
$(_LINK_OPTIONS)
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIB)\OtmDLL.lib $(_LIBOTHER)\xerces-c_3.lib
<<
