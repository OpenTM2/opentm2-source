# ---------------------------------------------------------------------------
#  Copyright (C) 1998-2013, International Business Machines          
#         Corporation and others. All rights reserved 
# ---------------------------------------------------------------------------
SRCPATH = $(_OTMMARKUP_SRC)\otmxmodc
CMNPATH = $(_OTMMARKUP_SRC)\common  
INCLUDEOPT  = /I$(SRCPATH) 

CPPFLAGS  = $(_OTMMARKUP_CPPFLAGS) $(INCLUDEOPT)
LINKFLAGS = $(_OTMMARKUP_LINKFLAGS)


.all: \
       $(_OTMMARKUP_OBJ)\otmxmodc.obj \
       $(_OTMMARKUP_OBJ)\odcparse.obj \
       $(_OTMMARKUP_OBJ)\odcexprt.obj \
       $(_OTMMARKUP_DLL)\otmxmodc.dll
       
#---------------------------------------
#  Compile OBJs              
#---------------------------------------
$(_OTMMARKUP_OBJ)\otmxmodc.obj: \
      $(SRCPATH)\otmxmodc.c \
      $(SRCPATH)\otmxmodc.h \
      $(SRCPATH)\parse.h    \
      $(SRCPATH)\export.h   \
      $(CMNPATH)\unicode.h  \
      $(CMNPATH)\reseq.h    
    $(_COMPILER)  $(CPPFLAGS)  /Fo$(_OTMMARKUP_OBJ)\otmxmodc $(SRCPATH)\otmxmodc.c

$(_OTMMARKUP_OBJ)\odcparse.obj: \
      $(SRCPATH)\parse.cpp  \
      $(SRCPATH)\parse.h    \
      $(SRCPATH)\otmxmodc.h  
    $(_COMPILER)  $(CPPFLAGS) /Fo$(_OTMMARKUP_OBJ)\odcparse  $(SRCPATH)\parse.cpp

$(_OTMMARKUP_OBJ)\odcexprt.obj: \
      $(SRCPATH)\export.cpp  \
      $(SRCPATH)\export.h    \
      $(SRCPATH)\otmxmodc.h  
    $(_COMPILER)  $(CPPFLAGS) /Fo$(_OTMMARKUP_OBJ)\odcexprt $(SRCPATH)\export.cpp

#---------------------------------------
#  Linking DLL               
#---------------------------------------
$(_OTMMARKUP_DLL)\otmxmodc.dll: \
      $(_OTMMARKUP_OBJ)\otmxmodc.obj    \
      $(_OTMMARKUP_OBJ)\odcparse.obj    \
      $(_OTMMARKUP_OBJ)\odcexprt.obj    \
      $(_OTMMARKUP_OBJ)\usrcalls_w.obj  \
      $(_OTMMARKUP_OBJ)\reseq_w.obj     \
      $(_OTMMARKUP_OBJ)\unicode.obj     \
      $(_OTMMARKUP_OBJ)\eqfcalls.obj
    @if exist $(_OTMMARKUP_DLL)\otmxmodc.dll  erase $(_OTMMARKUP_DLL)\otmxmodc.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmxmodc.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmxmodc.dll >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OTMMARKUP_OBJ)\otmxmodc.obj
$(_OTMMARKUP_OBJ)\odcparse.obj
$(_OTMMARKUP_OBJ)\odcexprt.obj
$(_OTMMARKUP_OBJ)\usrcalls_w.obj
$(_OTMMARKUP_OBJ)\reseq_w.obj
$(_OTMMARKUP_OBJ)\unicode.obj
$(_OTMMARKUP_OBJ)\eqfcalls.obj
/OUT:$(_OTMMARKUP_DLL)\otmxmodc.dll   $(LINKFLAGS)
/MAP:$(_OTMMARKUP_MAP)\otmxmodc.map
<<
    @copy $(_OTMMARKUP_DLL)\otmxmodc.dll $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxmodc\otmxmodc.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxmodc\otmopndc.xml $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxmodc\unzip.exe $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxmodc\zip.exe $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)

