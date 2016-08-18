# Copyright (c) 2012, International Business Machines
# Corporation and others.  All rights reserved.

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:    $(_MRI)\EQFMSG$(_NLSCHAR).STR \
          $(_DLL)\Messages$(_NLSCHAR).DLL  

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------
!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#################### creating the message and help files #######################
$(_MRI)\EQFMSG$(_NLSCHAR).STR: $(_INC)\EQFMSG.H
   @echo ---- Building EQFMSG$(_NLSCHAR).STR file
   @echo ---- Building EQFMSG$(_NLSCHAR).STR file >>$(_ERR)
   @attrib $(_INC)\EQFMSG.H -r >>$(_ERR)
   @$(_BIN)\eqfmsg $(_INC)\EQFMSG.H    \
                   $(_MRI)\EQFMSG$(_NLSCHAR).STR  \
                   $(_IPF)\EQFMSG$(_NLSCHAR).IPF  \
                   $(_MRI)\EQFMSG.HTB  \
                   $(_LANGUAGE)        1>>$(_ERR)


$(_DLL)\Messages$(_NLSCHAR).DLL: $(_MRI)\EQFMSG$(_NLSCHAR).STR
   @echo ---- Building $(_DLL)\Messages$(_NLSCHAR).DLL
   @echo ---- Building $(_DLL)\Messages$(_NLSCHAR).DLL >>$(_ERR)
   @$(_RC_COMPILER) $(_RC_OPT) -fo$(_RES)\EQFMSG$(_NLSCHAR).RES  $(_MRI)\EQFMSG$(_NLSCHAR).STR >>$(_ERR)
   @CVTRES /NOLOGO /OUT:$(_OBJ)\Messages.RBJ $(_RES)\EQFMSG$(_NLSCHAR).RES >>$(_ERR)
   @$(_LINKER) >>$(_ERR) @<<
$(_OBJ)\Messages.RBJ
/OUT:$(_DLL)\Messages$(_NLSCHAR).DLL $(_LINK_OPT_BASE) $(_LINK_LIB_CRT)
<<
