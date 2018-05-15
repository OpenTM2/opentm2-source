# Copyright (c) 2014-2017, International Business Machines
# Corporation and others.  All rights reserved.

#------------------------------------------------------------------------------
# eqfrules  from BUILDER                                                                  -
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------


build:    $(_DLL)\STANDARD.DLL 

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
# Build standard.dll                                                           -
#------------------------------------------------------------------------------

$(_DLL)\standard.DLL: $(_OBJ)\eqfbmain.obj
    @echo ---- Linking  STANDARD.DLL
    @echo ---- Linking  STANDARD.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp >>$(_ERR)
$(_OBJ)\eqfbmain.obj
/OUT:$(_DLL)\standard.dll
$(_LINK_OPTIONS) /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) 
$(_LIB)\OtmBase.LIB 
$(_LIB)\OtmAPI.LIB
$(_LIB)\OTMEditorFunctions.lib 
<<

