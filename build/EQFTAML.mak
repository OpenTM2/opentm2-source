#------------------------------------------------------------------------------
# EQFTAML.MAK   - Makefile for EQFTAML.DLL                                  
# Copyright (c) 2012, International Business Machines
# Corporation and others.  All rights reserved.
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

# target list                                                                 

build:   $(_DLL)\EQFTAML.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\CXMLWriter.OBJ: $(_SRC)\tools\CXMLWriter.CPP
$(_OBJ)\EQFTAML.OBJ: $(_SRC)\core\analysis\EQFTAML.CPP

#------ build EQFTAML.DLL --------
$(_DLL)\EQFTAML.DLL: $(_OBJ)\EQFTAML.OBJ $(_OBJ)\CXMLWriter.OBJ
    @echo ---- Linking  EQFTAML.DLL
    @echo ---- Linking  EQFTAML.DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\EQFTAML.OBJ $(_OBJ)\CXMLWriter.OBJ
/OUT:$(_DLL)\EQFTAML.DLL /MAPINFO:EXPORTS
/MAP:$(_MAP)\EQFTAML.MAP $(_LINK_OPTIONS)
$(_LINK_LIB_CRT) $(_LIB)\OtmAlloc.LIB $(_LIB)\OTMBASE.LIB $(_LIB)\OTMDLL.LIB oleaut32.lib uuid.lib ole32.lib
<<
