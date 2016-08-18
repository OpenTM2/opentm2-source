# +----------------------------------------------------------------------------+
# |  OpenTM2Scripter.MAK    - Makefile for the OpenTM2scripter tool            |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2013, International Business Machines              |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+
#

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:	$(_BIN)\OpenTM2Scripter.EXE

$(_BIN)\OpenTM2Scripter.EXE: $(_OBJ)\OpenTM2Scripter.obj

$(_OBJ)\OpenTM2Scripter.obj: $(_SRC)\tools\batch\OpenTM2Scripter.c

$(_BIN)\OpenTM2Scripter.exe: $(_OBJ)\OpenTM2Scripter.obj
    @echo ---- Linking $(_BIN)\OpenTM2Scripter.EXE
    @echo ---- Linking $(_BIN)\OpenTM2Scripter.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OpenTM2Scripter.obj
/OUT:$(_BIN)\OpenTM2Scripter.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE) $(_LIB)\OTMFUNC.LIB $(_LIB)\OtmBase.lib $(_LIB)\OTMDLL.lib $(_LIB)\PluginManager.lib
<<
