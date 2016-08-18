# ---------------------------------------------------------------------------
#  Copyright (C) 1998-2016, International Business Machines          
#         Corporation and others. All rights reserved 
# ---------------------------------------------------------------------------
SRCPATH = $(_OTMMARKUP_SRC)\otmmsofc
CMNPATH = $(_OTMMARKUP_SRC)\common  
INCLUDEOPT  = /I$(SRCPATH) /I$(CMNPATH)

CPPFLAGS  = $(_OTMMARKUP_CPPFLAGS) $(INCLUDEOPT)
LINKFLAGS = $(_OTMMARKUP_LINKFLAGS)


.all: \
       $(_OTMMARKUP_OBJ)\otmmsofc.obj  \
       $(_OTMMARKUP_OBJ)\ofcparse.obj  \
       $(_OTMMARKUP_OBJ)\ofcexprt.obj  \
       $(_OTMMARKUP_OBJ)\ofccheck.obj  \
       $(_OTMMARKUP_DLL)\otmmsofc.dll  
       
#---------------------------------------
#  Compile OBJs              
#---------------------------------------
$(_OTMMARKUP_OBJ)\otmmsofc.obj: \
      $(SRCPATH)\otmmsofc.c     \
      $(SRCPATH)\otmmsofc.h     \
      $(CMNPATH)\unicode.h
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\otmmsofc $(SRCPATH)\otmmsofc.c

$(_OTMMARKUP_OBJ)\ofcparse.obj: \
      $(SRCPATH)\parse.cpp      \
      $(SRCPATH)\otmmsofc.h
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\ofcparse $(SRCPATH)\parse.cpp 

$(_OTMMARKUP_OBJ)\ofcexprt.obj: \
      $(SRCPATH)\export.cpp      \
      $(SRCPATH)\otmmsofc.h
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\ofcexprt $(SRCPATH)\export.cpp 

$(_OTMMARKUP_OBJ)\ofccheck.obj: \
      $(SRCPATH)\check.cpp      \
      $(SRCPATH)\otmmsofc.h
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\ofccheck $(SRCPATH)\check.cpp 


#---------------------------------------
#  Linking DLL               
#---------------------------------------
$(_OTMMARKUP_DLL)\otmmsofc.dll:          \
      $(_OTMMARKUP_OBJ)\otmmsofc.obj     \
      $(_OTMMARKUP_OBJ)\ofcparse.obj     \
      $(_OTMMARKUP_OBJ)\ofcexprt.obj     \
      $(_OTMMARKUP_OBJ)\ofccheck.obj     \
      $(_OTMMARKUP_OBJ)\usrcalls_w.obj   \
      $(_OTMMARKUP_OBJ)\eqfcalls_w.obj   \
      $(_OTMMARKUP_OBJ)\unicode.obj       
    @if exist $(_OTMMARKUP_DLL)\otmmsofc.dll  erase $(_OTMMARKUP_DLL)\otmmsofc.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmmsofc.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmmsofc.dll >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OTMMARKUP_OBJ)\otmmsofc.obj
$(_OTMMARKUP_OBJ)\ofcparse.obj
$(_OTMMARKUP_OBJ)\ofcexprt.obj
$(_OTMMARKUP_OBJ)\ofccheck.obj
$(_OTMMARKUP_OBJ)\usrcalls_w.obj
$(_OTMMARKUP_OBJ)\eqfcalls_w.obj
$(_OTMMARKUP_OBJ)\unicode.obj
/OUT:$(_OTMMARKUP_DLL)\otmmsofc.DLL   $(LINKFLAGS)
/MAP:$(_OTMMARKUP_MAP)\otmmsofc.map
<<

    @copy $(_OTMMARKUP_DLL)\otmmsofc.dll $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmmsofc\otmmsofc.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)

