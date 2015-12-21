# +----------------------------------------------------------------------------+
# |  EQFDLL.MAK    - Makefile for Startup code and resource DLL                |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2013, International Business Machines              |
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


build:    $(_DLL)\EQF_API.DLL

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

$(_DLL)\eqf_api.dll:      $(_OBJ)\eqfx_api.obj $(_OBJ)\eqfx1api.obj

#------------------------------------------------------------------------------
# Build eqf_api.dll                                                           -
#------------------------------------------------------------------------------


$(_DLL)\eqf_api.dll:
    @echo ---- Linking  EQF_API.DLL
    @echo ---- Linking  EQF_API.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp >>$(_ERR)
$(_OBJ)\eqfx_api.obj
$(_OBJ)\eqfx1api.obj
/OUT:$(_DLL)\eqf_api.dll
$(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\otmbase.lib $(_LIB)\otmalloc.lib $(_LIB)\otmdll.lib
<<
