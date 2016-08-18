# +----------------------------------------------------------------------------+
# |  OPENTM2.MAK - Makefile for OpenTM2 Startup code                           |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2013, International Business Machines              |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build:    $(_BIN)\OpenTM2.EXE

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

$(_OBJ)\eqfcfrm.OBJ:	$(_SRC)\core\mfc\eqfcfrm.CPP
$(_OBJ)\eqfmfrm.OBJ:	$(_SRC)\core\mfc\eqfmfrm.CPP
$(_OBJ)\eqfclbox.OBJ:	$(_SRC)\core\mfc\eqfclbox.CPP
$(_OBJ)\eqfdoc99.OBJ:	$(_SRC)\core\mfc\eqfdoc99.CPP
$(_OBJ)\eqfprc99.OBJ:	$(_SRC)\core\mfc\eqfprc99.CPP
$(_OBJ)\eqfrpt99.OBJ:	$(_SRC)\core\mfc\eqfrpt99.CPP
$(_OBJ)\eqftmm99.OBJ:	$(_SRC)\core\mfc\eqftmm99.CPP
$(_OBJ)\eqfdic99.OBJ:	$(_SRC)\core\mfc\eqfdic99.CPP
$(_OBJ)\eqffll99.OBJ:	$(_SRC)\core\mfc\eqffll99.CPP
$(_OBJ)\eqffnt99.OBJ:	$(_SRC)\core\mfc\eqffnt99.CPP
$(_OBJ)\eqfweb.OBJ:		$(_SRC)\core\mfc\eqfweb.CPP
$(_OBJ)\eqfmuprp.OBJ:	$(_SRC)\core\mfc\eqfmuprp.CPP
$(_OBJ)\eqfgen99.OBJ:	$(_SRC)\core\mfc\eqfgen99.CPP
$(_OBJ)\eqfmfc.OBJ:		$(_SRC)\core\mfc\eqfmfc.CPP
$(_OBJ)\eqfd.OBJ:		$(_SRC)\core\mfc\eqfd.CPP


#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------
OBJS= $(_OBJ)\EQFD.OBJ \
      $(_OBJ)\eqfcfrm.OBJ \
      $(_OBJ)\eqfmfrm.OBJ \
      $(_OBJ)\eqfclbox.OBJ \
      $(_OBJ)\eqfdoc99.OBJ \
      $(_OBJ)\eqfprc99.OBJ \
      $(_OBJ)\eqfrpt99.OBJ \
      $(_OBJ)\eqftmm99.OBJ \
      $(_OBJ)\eqfdic99.OBJ \
      $(_OBJ)\eqffll99.OBJ \
      $(_OBJ)\eqffnt99.OBJ \
      $(_OBJ)\eqfweb.OBJ \
      $(_OBJ)\eqfmuprp.OBJ \
      $(_OBJ)\eqfgen99.OBJ \
      $(_OBJ)\eqfmfc.OBJ

$(_BIN)\OpenTM2.EXE:	$(OBJS)
#------------------------------------------------------------------------------
# Build OpenTM2.EXE                                                          -
#------------------------------------------------------------------------------
$(_BIN)\OpenTM2.EXE:
    @echo ---- Linking $(_BIN)\OpenTM2.EXE
    @echo ---- Linking $(_BIN)\OpenTM2.EXE >>$(_ERR)
    RC /D_WINDOWS /Fo$(_OBJ)\EQFSTART.RES $(_RC)\EQFSTART.RC
    CVTRES /NOLOGO /OUT:$(_OBJ)\EQFD.RBJ $(_OBJ)\EQFSTART.RES
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
$(OBJS) $(_OBJ)\EQFD.RBJ
$(_LINK_CPP_OPTIONS)
/OUT:$(_BIN)\OpenTM2.EXE /pdb:"$(_BIN)\EQFD.pdb"
   $(_LIB)\OtmAlloc.LIB $(_LIB)\OTMBase.LIB $(_LIB)\OTMDll.LIB version.lib $(_LIB)\OTMQDAM.LIB $(_LIB)\PluginManager.lib
<<
