# ---------------------------------------------------------------------------
#  Copyright (C) 1998-2013, International Business Machines          
#         Corporation and others. All rights reserved 
# ---------------------------------------------------------------------------
SRCPATH = $(_OTMMARKUP_SRC)\otmhtm32
CMNPATH = $(_OTMMARKUP_SRC)\common  
INCLUDEOPT  = /I$(SRCPATH)

CPPFLAGS  = $(_OTMMARKUP_CPPFLAGS) $(INCLUDEOPT)
LINKFLAGS = $(_OTMMARKUP_LINKFLAGS)


.all: \
       $(_OTMMARKUP_OBJ)\otmhtm32.obj \
       $(_OTMMARKUP_OBJ)\entity.obj   \
       $(_OTMMARKUP_OBJ)\htmlscri.obj \
       $(_OTMMARKUP_OBJ)\scrptseg.obj \
       $(_OTMMARKUP_DLL)\otmhtm32.dll
       
#---------------------------------------
#  Compile OBJs              
#---------------------------------------
$(_OTMMARKUP_OBJ)\otmhtm32.obj: \
      $(SRCPATH)\otmhtm32.c \
      $(SRCPATH)\otmhtm32.h \
      $(CMNPATH)\usrcalls.h \
      $(CMNPATH)\unicode.h  \
      $(CMNPATH)\reseq.h    
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\otmhtm32 $(SRCPATH)\otmhtm32.c

$(_OTMMARKUP_OBJ)\entity.obj: \
      $(SRCPATH)\entity.c  \
      $(SRCPATH)\entity.h  \
      $(SRCPATH)\otmhtm32.h 
    $(_COMPILER)  $(CPPFLAGS) /Fo$(_OTMMARKUP_OBJ)\entity  $(SRCPATH)\entity.c

$(_OTMMARKUP_OBJ)\htmlscri.obj: \
      $(SRCPATH)\htmlscri.c  \
      $(SRCPATH)\otmhtm32.h  \
      $(CMNPATH)\usrcalls.h  
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\htmlscri   $(SRCPATH)\htmlscri.c

$(_OTMMARKUP_OBJ)\scrptseg.obj: \
      $(SRCPATH)\scrptseg.c  \
      $(SRCPATH)\scrptseg.h  \
      $(SRCPATH)\otmhtm32.h  \
      $(CMNPATH)\usrcalls.h  
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\scrptseg   $(SRCPATH)\scrptseg.c

#---------------------------------------
#  Linking DLL               
#---------------------------------------
$(_OTMMARKUP_DLL)\otmhtm32.dll:        \
      $(_OTMMARKUP_OBJ)\otmhtm32.obj   \
      $(_OTMMARKUP_OBJ)\entity.obj     \
      $(_OTMMARKUP_OBJ)\htmlscri.obj   \
      $(_OTMMARKUP_OBJ)\scrptseg.obj   \
      $(_OTMMARKUP_OBJ)\usrcalls_w.obj \
      $(_OTMMARKUP_OBJ)\reseq_w.obj    \
      $(_OTMMARKUP_OBJ)\unicode.obj    \
      $(_OTMMARKUP_OBJ)\eqfcalls.obj
    @if exist $(_OTMMARKUP_DLL)\otmhtm32.dll  erase $(_OTMMARKUP_DLL)\otmhtm32.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmhtm32.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmhtm32.dll >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OTMMARKUP_OBJ)\otmhtm32.obj
$(_OTMMARKUP_OBJ)\entity.obj
$(_OTMMARKUP_OBJ)\htmlscri.obj
$(_OTMMARKUP_OBJ)\scrptseg.obj
$(_OTMMARKUP_OBJ)\usrcalls_w.obj
$(_OTMMARKUP_OBJ)\reseq_w.obj
$(_OTMMARKUP_OBJ)\unicode.obj    
$(_OTMMARKUP_OBJ)\eqfcalls.obj
/OUT:$(_OTMMARKUP_DLL)\otmhtm32.dll $(LINKFLAGS)
/MAP:$(_OTMMARKUP_MAP)\otmhtm32.map
<<
    @copy $(_OTMMARKUP_DLL)\otmhtm32.dll $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmhtm32\otmhtm32.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmhtm32\otmuhtm3.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)

