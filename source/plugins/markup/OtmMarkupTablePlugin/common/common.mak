# ---------------------------------------------------------------------------
#  Copyright (C) 1998-2016, International Business Machines          
#         Corporation and others. All rights reserved 
# ---------------------------------------------------------------------------
SRCPATH = $(_OTMMARKUP_SRC)\common
INCLUDEOPT = /I$(SRCPATH)

CFLAGS    = $(_OTMMARKUP_CFLAGS) $(INCLUDEOPT)
CPPFLAGS  = $(_OTMMARKUP_CPPFLAGS) $(INCLUDEOPT)


.all: \
       $(_OTMMARKUP_OBJ)\USRCALLS.OBJ    \
       $(_OTMMARKUP_OBJ)\USRCALLS_W.OBJ  \
       $(_OTMMARKUP_OBJ)\UNICODE.OBJ     \
       $(_OTMMARKUP_OBJ)\RESEQ.OBJ       \
       $(_OTMMARKUP_OBJ)\RESEQ_W.OBJ     \
       $(_OTMMARKUP_OBJ)\EQFCALLS.OBJ    \
       $(_OTMMARKUP_OBJ)\EQFCALLS_W.OBJ
       
#---------------------------------------
#  Compile OBJs              
#---------------------------------------
$(_OTMMARKUP_OBJ)\reseq.obj: \
      $(SRCPATH)\reseq.c \
      $(SRCPATH)\reseq.h 
    $(_COMPILER)  $(CFLAGS)  /Fo$(_OTMMARKUP_OBJ)\reseq  $(SRCPATH)\reseq.c

$(_OTMMARKUP_OBJ)\usrcalls.obj: \
      $(SRCPATH)\usrcalls.c \
      $(SRCPATH)\usrcalls.h \
      $(_INC)\otmapi.h 
    $(_COMPILER)  $(CFLAGS)  /Fo$(_OTMMARKUP_OBJ)\usrcalls   $(SRCPATH)\usrcalls.c

$(_OTMMARKUP_OBJ)\eqfcalls.obj: \
       $(SRCPATH)\eqfcalls.c    \
      $(_INC)\otmapi.h 
    $(_COMPILER)  $(CFLAGS)  /Fo$(_OTMMARKUP_OBJ)\eqfcalls   $(SRCPATH)\eqfcalls.c



$(_OTMMARKUP_OBJ)\reseq_w.obj: \
      $(SRCPATH)\reseq.c \
      $(SRCPATH)\reseq.h 
    $(_COMPILER)  $(CPPFLAGS) /Fo$(_OTMMARKUP_OBJ)\reseq_w $(SRCPATH)\reseq.c

$(_OTMMARKUP_OBJ)\usrcalls_w.obj: \
      $(SRCPATH)\usrcalls.c \
      $(SRCPATH)\usrcalls.h \
      $(_INC)\otmapi.h 
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\usrcalls_w $(SRCPATH)\usrcalls.c

$(_OTMMARKUP_OBJ)\eqfcalls_w.obj: \
       $(SRCPATH)\eqfcalls.c    \
      $(_INC)\otmapi.h 
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\eqfcalls_w $(SRCPATH)\eqfcalls.c

$(_OTMMARKUP_OBJ)\unicode.obj: \
      $(SRCPATH)\unicode.cpp \
      $(SRCPATH)\unicode.h
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\unicode $(SRCPATH)\unicode.cpp

