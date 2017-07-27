# Copyright (c) 1990-2017, International Business Machines
# Corporation and others.  All rights reserved.
#
!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build: $(_DLL)\OTMTMXIE.DLL

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

$(_OBJ)\OTMTMXIE.OBJ: $(_SRC)\plugins\memory\OTMTMXIE.CPP



#------ build OTMTMXIE.DLL --------
$(_DLL)\OTMTMXIE.DLL: $(_OBJ)\OTMTMXIE.OBJ $(_OBJ)\CXMLWriter.OBJ
    @if exist $(_DLL)\OTMTMXIE.DLL  erase $(_DLL)\OTMTMXIE.DLL
    @echo ---- Linking  OTMTMXIE.DLL
    @echo ---- Linking  OTMTMXIE.DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OTMTMXIE.OBJ $(_OBJ)\CXMLWriter.OBJ
/OUT:$(_DLL)\OTMTMXIE.DLL /MAPINFO:EXPORTS
$(_LINK_OPTIONS)
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIB)\OtmDLL.lib $(_LIBOTHER)\xerces-c_3.lib
<<
