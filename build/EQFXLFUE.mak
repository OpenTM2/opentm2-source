#   EQFXLIFP.MAK   - Makefile for the EQFXLIFF parser                         
#
#  Copyright Notice:                                                         
#                                                                            
#      Copyright (C) 1990-2012, International Business Machines              
#      Corporation and others. All rights reserved                           

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:    $(_DLL)\EQFXLFUE.DLL

#------------------------------------------------------------------------------
# dependencies                                                                -
#------------------------------------------------------------------------------
$(_DLL)\EQFXLFUE.DLL:   $(_DEF)\EQFXLFUE.$(_DEFEXT)  \
                       $(_OBJ)\EQFXLFUE.OBJ

$(_OBJ)\EQFXLFUE.OBJ:   $(_SRC)\plugins\markup\EQFXLFUE.C

#------------------------------------------------------------------------------
# build of object files                                                       -
#------------------------------------------------------------------------------
$(_OBJ)\EQFXLFUE.OBJ:  $(_SRC)\plugins\markup\EQFXLFUE.C

#------------------------------------------------------------------------------
# Build EQFXLFUE.DLL                                                           -
#------------------------------------------------------------------------------

$(_DLL)\EQFXLFUE.DLL:
    @if exist $(_DLL)\EQFXLFUE.DLL  erase $(_DLL)\EQFXLFUE.DLL
    @echo ---- Linking $(_DLL)\EQFXLFUE.DLL
    @echo ---- Linking $(_DLL)\EQFXLFUE.DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\EQFXLFUE.OBJ
/OUT:$(_DLL)\EQFXLFUE.DLL
$(_LINK_OPTIONS)
$(_LINK_LIB_CRT) $(_LIB)\EQFDLL.LIB
/DEF:$(_DEF)\EQFXLFUE.$(_DEFEXT)
<<

