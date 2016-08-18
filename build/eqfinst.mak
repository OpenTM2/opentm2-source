# +----------------------------------------------------------------------------+
# |  EQFINST.MAK    - Make for installation program(s)                         |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2012, International Business Machines              |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:                 $(_BIN)\EQFPRPUP.EXE \
                       $(_BIN)\EQFINST.EXE

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------
!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# dependencies and build commands                                             -
#------------------------------------------------------------------------------

$(_OBJEXE)\EQFSETUP.OBJ : $(_SRC)\EQFSETUP.C $(_INC)\EQFSETUP.H

$(_OBJEXE)\INSTALL.OBJ : $(_SRC)\INSTALL.C   $(_INC)\INSTALL.H \
                      $(_INC)\EQFSETUP.H  $(_MRI)\INSTALL.MRI \
                      $(_INC)\EQF.H

$(_OBJEXE)\EQFINST.OBJ:  $(_SRC)\EQFINST.C   $(_INC)\EQF.H   \
                         $(_INC)\EQFSETUP.H

$(_BIN)\EQFINST.EXE:  $(_OBJEXE)\EQFINST.OBJ  \
                      $(_OBJEXE)\EQFUPPRP.OBJ \
                      $(_OBJEXE)\EQFSETUP.OBJ
    @echo ---- Linking $(_BIN)\EQFINST.EXE
    @echo ---- Linking $(_BIN)\EQFINST.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OBJEXE)\EQFINST.OBJ
$(_OBJEXE)\EQFUPPRP.OBJ
$(_OBJEXE)\EQFSETUP.OBJ
/OUT:$(_BIN)\EQFINST.EXE EQFDLL.LIB
/MAP:$(_MAP)\EQFINST.MAP $(_LINK_OPTIONS)
$(_LINK_LIB_EXE)
/DEF:$(_DEF)\EQFINST.$(_DEFEXT)
<<

$(_BIN)\EQFPRPUP.EXE: $(_OBJEXE)\EQFPRPUP.OBJ \
                      $(_OBJEXE)\EQFUPPRP.OBJ \
                      $(_OBJEXE)\EQFSETUP.OBJ
    @echo ---- Linking $(_BIN)\EQFPRPUP.EXE
    @echo ---- Linking $(_BIN)\EQFPRPUP.EXE >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OBJEXE)\EQFPRPUP.OBJ
$(_OBJEXE)\EQFUPPRP.OBJ
$(_OBJEXE)\EQFSETUP.OBJ
/OUT:$(_BIN)\EQFPRPUP.EXE
/MAP:$(_MAP)\EQFPRPUP.MAP $(_LINK_OPTIONS)
$(_LINK_LIB_EXE) EQFDLL.LIB
/DEF:$(_DEF)\EQFPRPUP.$(_DEFEXT)
<<

