#    OTMRES.MAK    - Makefile for General Resource DLL                          
# Copyright (c) 2015, International Business Machines
# Corporation and others.  All rights reserved.

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list                                                                 -
#------------------------------------------------------------------------------
build: $(_DLL)\OTMRES$(PL)$(_NLSCHAR).DLL \
       $(_DLL)\OTMLOGOR.DLL 


#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------
#!IF "$(MAKEDEP)" == "Y"
#!INCLUDE $(DEPFILE)
#!ENDIF

#------------------------------------------------------------------------------
# Dependencies                                                                -
#------------------------------------------------------------------------------
$(_RES)\EQFRES.RES :  $(_RC)\EQFRES.RC   $(_ID)\EQF.ID        \
       $(_ID)\EQFTWB.ID     $(_INC)\EQFSERNO.H   $(_MRI)\EQFSTART.MRI \
       $(_MRI)\EQFFOL.MRI   $(_ID)\EQFFOL.ID     $(_ID)\EQFFLL.ID     \
       $(_ID)\EQFUTCLB.ID   $(_ID)\EQFDPROP.ID   $(_ID)\EQFDOC01.ID   \
       $(_MRI)\EQFDOC01.MRI $(_MRI)\EQFUTCLB.MRI $(_ICO)\EQFTWBW.ICO  \
       $(_ICO)\EQFFLLW.ICO  $(_ICO)\EQFFOLW.ICO  $(_ICO)\EQFDOCW.ICO  \
       $(_ICO)\EQFANAW.ICO  $(_ICO)\EQFMEMW.ICO  $(_ICO)\EQFDIMPW.ICO \
       $(_ICO)\EQFDEXPW.ICO $(_ICO)\EQFFIMPW.ICO $(_ICO)\EQFFEXPW.ICO \
       $(_ICO)\EQFDORGW.ICO $(_ICO)\EQFTAGW.ICO  $(_ICO)\EQFDICW.ICO  \
       $(_ICO)\EQFCNTW.ICO  $(_ICO)\EQFMIMPW.ICO $(_ICO)\EQFMEXPW.ICO \
       $(_ICO)\EQFMMRGW.ICO $(_ICO)\EQFMORGW.ICO $(_ICO)\EQFMSRVW.ICO \
       $(_ICO)\EQFDICLW.ICO $(_ICO)\EQFDICDW.ICO $(_ICO)\EQFDICEW.ICO \
       $(_ICO)\EQFTIMPW.ICO $(_ICO)\EQFTEXPW.ICO $(_ICO)\EQFLISTW.ICO \
       $(_ICO)\EQFFDW.BMP   $(_ICO)\EQFHDW.BMP   $(_ICO)\EQFCDW.BMP   \
       $(_ICO)\EQFRAMW.BMP  $(_ICO)\EQFNETW.BMP  $(_ICO)\EQFNETW.BMP  \
       $(_ICO)\EQFTOOLW.BMP $(_MEN)\EQFSTART.MEN $(_STR)\EQFSTART.STR \
       $(_DLG)\EQFFOL.DLG   $(_STR)\EQFFOL.STR   $(_DLG)\EQFUTCLB.DLG \
       $(_DLG)\EQFTWB.DLG   $(_DLG)\EQFDOC01.DLG $(_STR)\EQFDOC01.STR \
       $(_DLG)\EQFLOGO.DLG  $(_ICO)\EQFBACKW.BMP $(_DLG)\EQFUTILS.DLG \
       $(_ICO)\EQFQDPRW.ICO $(_ICO)\EQFLISTW.ICO $(_ID)\EQFSTART.ID \
       $(_DLG)\EQFMUPRP.DLG $(_DLG)\EQFBPROP.DLG


$(_RES)\EQFRES.RES: $(_MRI)\EQFDICT.MRI  $(_ID)\EQFDIMP.ID    \
       $(_ID)\EQFDEX.ID     $(_ID)\EQFDPROP.ID   $(_ID)\EQFRDICS.ID   \
       $(_ID)\EQFDIC00.ID   $(_DLG)\EQFDIMP.DLG  $(_DLG)\EQFDEX.DLG   \
       $(_DLG)\EQFDPROP.DLG $(_DLG)\EQFRDICS.DLG $(_MRI)\EQFCOLW.MRI

$(_RES)\EQFRES.RES: $(_ID)\EQFMEM.ID $(_DLG)\EQFMEMD.DLG  $(_STR)\EQFMEM.STR \
       $(_MRI)\EQFMEM.MRI $(_DLG)\EQFMTP.DLG

$(_RES)\EQFRES.RES: $(_ID)\EQFDDLG.ID $(_MRI)\EQFDICTL.MRI $(_ID)\EQFQDPR.ID \
       $(_MRI)\EQFQDPRR.MRI $(_DLG)\EQFDDLG.DLG

$(_RES)\EQFRES.RES: $(_ID)\EQFFILT.ID \
       $(_MRI)\EQFFILT.MRI  $(_STR)\EQFFILT.STR  $(_DLG)\EQFFILT.DLG  \
       $(_DLG)\EQFFILTC.DLG

$(_RES)\EQFRES.RES:  $(_DLG)\EQFLIST.DLG  \
       $(_ID)\EQFLIST.ID    $(_STR)\EQFLIST.STR  $(_MRI)\EQFLIST.MRI

$(_RES)\EQFRES.RES: $(_MRI)\EQFTMM.MRI   \
       $(_MRI)\EQFTMM.MEN   $(_MRI)\EQFTMFUN.MRI $(_MRI)\EQFTMFUN.DLG $(_RC)\EQFTMF99.DLG \
       $(_ID)\EQFTMFUN.ID   $(_ID)\EQFTMM.ID

$(_RES)\EQFRES.RES: $(_ID)\EQFB.ID $(_MRI)\EQFB.MRI \
       $(_MRI)\EQFB.MEN     $(_MRI)\EQFB.STR     $(_MRI)\EQFB.ACC \
       $(_ID)\EQFBDLG.ID    $(_MRI)\\EQFBDLG.DLG $(_ID)\EQFITMD.ID \
       $(_MRI)\EQFITM.MEN   $(_MRI)\EQFITMD.DLG

$(_RES)\EQFRES.RES: $(_RC)\EQFIANA1.RC \
       $(_MRI)\EQFIANA1.MRI $(_ID)\EQFIANA1.ID $(_MRI)\EQFIANA1.DLG \
       $(_ID)\EQFCNT01.ID   $(_MRI)\EQFCNT01.MRI $(_MRI)\EQFCNT01.STR \
       $(_MRI)\EQFCNT01.DLG $(_MRI)\EQFLNG00.MRI \
       $(_ID)\EQFLNG00.ID   $(_MRI)\EQFLNG00.STR

$(_RES)\EQFRES.RES: $(_ID)\EQFMEM.ID $(_MRI)\EQFMEM.MRI $(_MRI)\EQFMEM.STR \
       $(_MRI)\EQFMEMD.DLG $(_MRI)\EQFTEXP.MRI $(_ID)\EQFTEXP.ID $(_MRI)\EQFTEXP.DLG

$(_RES)\EQFRES.RES: $(_MRI)\EQFTIMP.MRI  $(_ID)\EQFTIMP.ID $(_ID)\EQFTAG00.ID \
       $(_MRI)\EQFTIMP.DLG $(_MRI)\EQFRPT.MRI   $(_ID)\EQFRPT.ID  $(_MRI)\EQFRPT1.DLG \
       $(_MRI)\EQFRPT2.DLG $(_MRI)\EQFRPT.DLG $(_MRI)\EQFRPT3.DLG \
       $(_MRI)\EQFRPT.STR \
       $(_MRI)\EQFMT.STR $(_MRI)\EQFMT.DLG $(_ID)\EQFMT.ID $(_MRI)\EQFMT.MRI

$(_RES)\EQFRES.RES: $(_RC)\EQFANA.DLG  $(_RC)\EQFSPROP.DLG   $(_RC)\EQFRPT99.DLG  \
       $(_RC)\EQFFOL99.DLG $(_RC)\EQFSORT.DLG $(_RC)\EQFFILT2.DLG   $(_RC)\EQFDIC99.DLG \
       $(_RC)\EQFSUBF.DLG  $(_RC)\EQFDOC99.DLG $(_RC)\EQFDOC.DLG



$(_OBJ)\EQFRES.RBJ:   $(_RES)\EQFRES.RES

$(_DLL)\OTMRES$(PL)$(_NLSCHAR).DLL: $(_OBJ)\EQFRES.RBJ

#------------------------------------------------------------------------------
# Build resource DLL                                                          -
#------------------------------------------------------------------------------
$(_DLL)\OTMRES$(PL)$(_NLSCHAR).DLL:
    @if exist $(_DLL)\OTMRES$(PL)$(_NLSCHAR).DLL erase $(_DLL)\OTMRES$(PL)$(_NLSCHAR).DLL
    @echo ---- Linking $(_DLL)\OTMRES$(PL)$(_NLSCHAR).DLL
    @echo ---- Linking $(_DLL)\OTMRES$(PL)$(_NLSCHAR).DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OBJ)\EQFRES.RBJ
/OUT:$(_DLL)\OTMRES$(PL)$(_NLSCHAR).DLL
$(_LINK_OPT_BASE)
$(_LINK_LIB_CRT)
<<

$(_RES)\EQFLOGOR.RES: $(_RC)\EQFLOGOR.RC    \
                      $(_ID)\EQF.ID

$(_OBJ)\EQFLOGOR.RBJ: $(_RES)\EQFLOGOR.RES

$(_DLL)\OTMLOGOR.DLL: $(_OBJ)\EQFLOGOR.RBJ
    @echo ---- Linking $(_DLL)\OTMLOGOR.DLL
    @echo ---- Linking $(_DLL)\OTMLOGOR.DLL >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<lnk.rsp
$(_OBJ)\EQFLOGOR.RBJ
/OUT:$(_DLL)\OTMLOGOR.DLL
/MAP:$(_MAP)\EQFLOGOR.MAP $(_LINK_OPTIONS) /NOENTRY
$(_LINK_LIB_CRT)
<<

