# EQFMSG.MAK
# Copyright (c) 2012, International Business Machines
# Corporation and others.  All rights reserved.
#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:	$(_BIN)\EQFMSG.EXE

!INCLUDE $(_BLD)\EQFRULES.MAK


$(_OBJ)\eqfmsg.obj:	$(_SRC)\tools\eqfmsg.c

$(_BIN)\eqfmsg.exe:	$(_OBJ)\eqfmsg.obj
    @echo ---- Linking $(_BIN)\eqfmsg.EXE
    @echo ---- Linking $(_BIN)\eqfmsg.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\eqfmsg.obj
/OUT:$(_BIN)\eqfmsg.exe
$(_LINK_OPTIONS)
$(_LINK_LIB_EXE)
<<
