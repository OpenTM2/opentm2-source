# +----------------------------------------------------------------------------+
# |  OTMAPIDUMMY.MAK  - Makefile for editor API functions LIB                  |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2017, International Business Machines              |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+
# |  Description:                                                              |
# +----------------------------------------------------------------------------+
#

#------------------------------------------------------------------------------
# eqfrules  from BUILDER                                                                  -
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------


build:    $(_DLL)\OTMAPI.LIB

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------

$(_OBJ)\eqf_api.obj:      $(_SRC)\api\eqfx_api.c

$(_OBJ)\eqfx1api.obj:     $(_SRC)\api\eqfx1api.c

$(_DLL)\OTMAPI.LIB:      $(_OBJ)\eqfx_api.obj $(_OBJ)\eqfx1api.obj

#------------------------------------------------------------------------------
# Build OTMAPI.LIB                                                       -
#------------------------------------------------------------------------------


!CMDSWITCHES +I
$(_DLL)\OTMAPI.LIB:
    @echo ---- Creating  OTMAPI.LIB
    @echo ---- Creating  OTMAPI.LIB >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp > nul 
$(_OBJ)\eqfx_api.obj
$(_OBJ)\eqfx1api.obj
/OUT:$(_DLL)\OTMAPI.dll
$(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\otmbase.lib 
<<
!CMDSWITCHES -I
