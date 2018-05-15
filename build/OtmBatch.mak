# +----------------------------------------------------------------------------+
# |  OtmBatch.MAK    - Makefile for the OtmBatch tool                      |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2017, International Business Machines              |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:	$(_BIN)\OtmBatch.EXE

$(_BIN)\OtmBatch.EXE: $(_OBJ)\OtmBatch.obj

$(_OBJ)\OtmBatch.obj: $(_SRC)\tools\batch\OtmBatch.C

$(_BIN)\OtmBatch.EXE: $(_OBJ)\OtmBatch.obj
    @echo ---- Linking $(_BIN)\OtmBatch.EXE
    @echo ---- Linking $(_BIN)\OtmBatch.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\OtmBatch.obj
/OUT:$(_BIN)\OtmBatch.EXE
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE) 
$(_LIB)\OtmBase.lib 
$(_LIB)\OTMFUNC.LIB 
$(_LIB)\PluginManager.lib
<<
