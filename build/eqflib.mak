#   Copyright Notice:                                                         
#                                                                             
#           Copyright (C) 1990-2012, International Business Machines          
#           Corporation and others. All rights reserved                       
#                                                                             
#                                                                             

!INCLUDE $(_BLD)\EQFRULES.MAK

#------ target list -----
build:  $(_LIB)\EQF_API.LIB  \
        $(_LIB)\EQFRPXML.LIB \
        $(_LIB)\EQFTAML.LIB

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------
!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------ building  libraries --------

$(_LIB)\EQF_API.LIB:   $(_DEF)\EQF_API.$(_DEFEXT)
    @echo ---- Creating import library $(_LIB)\$(*B).LIB
    @echo ---- Creating import library $(_LIB)\$(*B).LIB >>$(_ERR)
    @copy $(_DEF)\eqf_api1.$(_DEFEXT) $(_DEF)\eqf_api.$(_DEFEXT) /Y
    @$(_IMPLIBER) /DEF:$(_DEF)\$(*B).$(_DEFEXT) /MACHINE:IX86 /NOLOGO /OUT:$(_LIB)\$(*B).LIB >>$(_ERR)


$(_LIB)\EQFRPXML.LIB:   $(_DEF)\EQFRPXML.$(_DEFEXT)


$(_LIB)\EQFTAML.LIB:   $(_DEF)\EQFTAML.$(_DEFEXT)


