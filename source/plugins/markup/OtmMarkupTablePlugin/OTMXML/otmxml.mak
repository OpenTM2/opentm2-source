# ---------------------------------------------------------------------------
#  Copyright (C) 1998-2013, International Business Machines          
#         Corporation and others. All rights reserved 
# ---------------------------------------------------------------------------
SRCPATH = $(_OTMMARKUP_SRC)\otmxml
CMNPATH = $(_OTMMARKUP_SRC)\common  
INCLUDEOPT  = /I$(SRCPATH)

CPPFLAGS  = $(_OTMMARKUP_CPPFLAGS) $(INCLUDEOPT)
LINKFLAGS = $(_OTMMARKUP_LINKFLAGS)


.all: \
       $(_OTMMARKUP_OBJ)\otmxml.obj \
       $(_OTMMARKUP_OBJ)\xmlparse.obj \
       $(_OTMMARKUP_OBJ)\xmlstate.obj \
       $(_OTMMARKUP_DLL)\otmxml.dll
       
#---------------------------------------
#  Compile OBJs              
#---------------------------------------
$(_OTMMARKUP_OBJ)\otmxml.obj: \
      $(SRCPATH)\otmxml.c     \
      $(SRCPATH)\parse.h      \
      $(CMNPATH)\usrcalls.h
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\otmxml $(SRCPATH)\otmxml.c

$(_OTMMARKUP_OBJ)\xmlparse.obj: \
      $(SRCPATH)\parse.cpp      \
      $(SRCPATH)\parse.h        \
      $(SRCPATH)\state.h        \
      $(CMNPATH)\unicode.h      \
      $(CMNPATH)\usrcalls.h     \
      $(CMNPATH)\reseq.h
    $(_COMPILER)  $(CPPFLAGS) /Fo$(_OTMMARKUP_OBJ)\xmlparse  $(SRCPATH)\parse.cpp

$(_OTMMARKUP_OBJ)\xmlstate.obj: \
      $(SRCPATH)\state.cpp      \
      $(SRCPATH)\state.h        \
      $(CMNPATH)\unicode.h      \
      $(CMNPATH)\usrcalls.h      
    $(_COMPILER)  $(CPPFLAGS) /Fo$(_OTMMARKUP_OBJ)\xmlstate $(SRCPATH)\state.cpp

#---------------------------------------
#  Linking DLL               
#---------------------------------------
$(_OTMMARKUP_DLL)\otmxml.dll:          \
      $(_OTMMARKUP_OBJ)\otmxml.obj     \
      $(_OTMMARKUP_OBJ)\xmlparse.obj   \
      $(_OTMMARKUP_OBJ)\xmlstate.obj   \
      $(_OTMMARKUP_OBJ)\usrcalls_w.obj \
      $(_OTMMARKUP_OBJ)\reseq_w.obj    \
      $(_OTMMARKUP_OBJ)\unicode.obj    \
      $(_OTMMARKUP_OBJ)\eqfcalls.obj
    @if exist $(_OTMMARKUP_DLL)\otmxml.dll  erase $(_OTMMARKUP_DLL)\otmxml.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmxml.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmxml.dll >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OTMMARKUP_OBJ)\otmxml.obj
$(_OTMMARKUP_OBJ)\xmlparse.obj
$(_OTMMARKUP_OBJ)\xmlstate.obj
$(_OTMMARKUP_OBJ)\usrcalls_w.obj
$(_OTMMARKUP_OBJ)\reseq_w.obj
$(_OTMMARKUP_OBJ)\unicode.obj
$(_OTMMARKUP_OBJ)\eqfcalls.obj
/OUT:$(_OTMMARKUP_DLL)\otmxml.dll   $(LINKFLAGS)
/MAP:$(_OTMMARKUP_MAP)\otmxml.map
<<
    @copy $(_OTMMARKUP_DLL)\otmxml.dll $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxml\otmxml.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxml\otmxaxml.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxml\otmxuxml.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxml\otmxml.ctl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxml\otmxml.lcl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)

