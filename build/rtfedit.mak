# Copyright (c) 2014, International Business Machines
# Corporation and others.  All rights reserved.

#------------------------------------------------------------------------------
# eqfrules  from BUILDER                                                                  -
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------


build:    $(_DLL)\RTFEDIT.DLL 

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# Build rtfedit.dll                                                           -
#------------------------------------------------------------------------------

$(_DLL)\rtfedit.DLL: $(_OBJ)\eqfbrtf0.obj
    @echo ---- Linking  RTFEDIT.DLL
    @echo ---- Linking  RTFEDIT.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp >>$(_ERR)
$(_OBJ)\eqfbrtf0.obj
/OUT:$(_DLL)\rtfedit.dll
$(_LINK_OPTIONS) /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.LIB $(_LIB)\OtmAlloc.LIB $(_LIB)\OtmDll.LIB $(_LIB)\OtmAPI.LIB
<<

