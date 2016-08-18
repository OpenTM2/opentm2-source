# ---------------------------------------------------------------------------
#  Copyright (C) 1998-2013, International Business Machines          
#         Corporation and others. All rights reserved 
# ---------------------------------------------------------------------------
SRCPATH = $(_OTMMARKUP_SRC)\otmjdk21
CMNPATH = $(_OTMMARKUP_SRC)\common  
INCLUDEOPT  = /I$(SRCPATH)


CFLAGS    = $(_OTMMARKUP_CFLAGS) $(INCLUDEOPT)
LINKFLAGS = $(_OTMMARKUP_LINKFLAGS)


.all: \
       $(_OTMMARKUP_OBJ)\otmjdk11.obj \
       $(_OTMMARKUP_OBJ)\javapro.obj  \
       $(_OTMMARKUP_OBJ)\javaseg.obj  \
       $(_OTMMARKUP_OBJ)\jcontext.obj  \
       $(_OTMMARKUP_DLL)\otmjdk11.dll  
       
#---------------------------------------
#  Compile OBJs              
#---------------------------------------
$(_OTMMARKUP_OBJ)\otmjdk11.obj: \
      $(SRCPATH)\otmjdk11.c \
      $(SRCPATH)\otmjdk11.h \
      $(SRCPATH)\javapro.h  \
      $(SRCPATH)\javaseg.h  \
      $(CMNPATH)\usrcalls.h \
      $(CMNPATH)\reseq.h    
    $(_COMPILER)  $(CFLAGS)  /Fo$(_OTMMARKUP_OBJ)\otmjdk11 $(SRCPATH)\otmjdk11.c

$(_OTMMARKUP_OBJ)\javapro.obj: \
      $(SRCPATH)\javapro.c  \
      $(SRCPATH)\javapro.h  \
      $(SRCPATH)\otmjdk11.h  
    $(_COMPILER)  $(CFLAGS) /Fo$(_OTMMARKUP_OBJ)\javapro $(SRCPATH)\javapro.c

$(_OTMMARKUP_OBJ)\javaseg.obj: \
      $(SRCPATH)\javaseg.c  \
      $(SRCPATH)\javaseg.h  \
      $(SRCPATH)\otmjdk11.h  
    $(_COMPILER)  $(CFLAGS)  /Fo$(_OTMMARKUP_OBJ)\javaseg    $(SRCPATH)\javaseg.c

$(_OTMMARKUP_OBJ)\jcontext.obj: \
      $(SRCPATH)\context.c  \
      $(SRCPATH)\otmjdk11.h  
    $(_COMPILER)  $(CFLAGS) /Fo$(_OTMMARKUP_OBJ)\jcontext $(SRCPATH)\context.c

#---------------------------------------
#  Link DLL               
#---------------------------------------
$(_OTMMARKUP_DLL)\otmjdk11.dll:       \
      $(_OTMMARKUP_OBJ)\otmjdk11.obj  \
      $(_OTMMARKUP_OBJ)\javapro.obj   \
      $(_OTMMARKUP_OBJ)\javaseg.obj   \
      $(_OTMMARKUP_OBJ)\jcontext.obj  \
      $(_OTMMARKUP_OBJ)\usrcalls.obj  \
      $(_OTMMARKUP_OBJ)\reseq.obj     \
      $(_OTMMARKUP_OBJ)\eqfcalls.obj 
    @if exist $(_OTMMARKUP_DLL)\otmjdk11.dll  erase $(_OTMMARKUP_DLL)\otmjdk11.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmjdk11.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmjdk11.dll >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OTMMARKUP_OBJ)\otmjdk11.obj 
$(_OTMMARKUP_OBJ)\javapro.obj 
$(_OTMMARKUP_OBJ)\javaseg.obj 
$(_OTMMARKUP_OBJ)\jcontext.obj 
$(_OTMMARKUP_OBJ)\usrcalls.obj 
$(_OTMMARKUP_OBJ)\reseq.obj 
$(_OTMMARKUP_OBJ)\eqfcalls.obj 
/OUT:$(_OTMMARKUP_DLL)\otmjdk11.dll $(LINKFLAGS)
/MAP:$(_OTMMARKUP_MAP)\otmjdk11.map
user32.lib
<<
    @copy $(_OTMMARKUP_DLL)\otmjdk11.dll $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmjdk21\otmjdk21.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmjdk21\otmajdk2.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmjdk21\otmnjdk2.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmjdk21\otmujdk2.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)

