#------------------------------------------------------------------------------
# EQFRPXML.MAK                                                                
#------------------------------------------------------------------------------
# Copyright Notice:                                                          
#                                                                               
#    Copyright (C) 1990-2012, International Business Machines               
#    Corporation and others. All rights reserved                            
#------------------------------------------------------------------------------
#    Description:  XML Calculation Report                                       
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:	$(_DLL)\EQFRPXML.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------
!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\CXMLWriter.OBJ:	$(_SRC)\tools\CXMLWriter.CPP

$(_OBJ)\EQFRPXML.OBJ:	$(_SRC)\core\counting\EQFRPXML.CPP


#------ build EQFRPXML.DLL --------
$(_DLL)\EQFRPXML.DLL:	$(_OBJ)\EQFRPXML.OBJ \
						$(_OBJ)\CXMLWriter.OBJ
    @echo ---- Linking  EQFRPXML.DLL
    @echo ---- Linking  EQFRPXML.DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
						$(_OBJ)\EQFRPXML.OBJ
						$(_OBJ)\CXMLWriter.OBJ
/OUT:$(_DLL)\EQFRPXML.DLL
/MAP:$(_MAP)\EQFRPXML.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmAlloc.LIB $(_LIB)\OTMBase.LIB $(_LIB)\OTMDll.LIB
<<
