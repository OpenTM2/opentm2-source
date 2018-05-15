# ---------------------------------------------------------------------------
#  Copyright (C) 1998-2017, International Business Machines          
#         Corporation and others. All rights reserved 
# ---------------------------------------------------------------------------
SRCPATH = $(_OTMMARKUP_SRC)\otmquote
CMNPATH = $(_OTMMARKUP_SRC)\common  
INCLUDEOPT  = /I$(SRCPATH) /I$(CMNPATH)


#CFLAGS    = $(_OTMMARKUP_CFLAGS) $(INCLUDEOPT)
CFLAGS    = $(_CL_CPP_OPTIONS_DLL) $(INCLUDEOPT)
LINKFLAGS = $(_OTMMARKUP_LINKFLAGS)


.all: \
       $(_OTMMARKUP_OBJ)\otmquote.obj \
       $(_OBJ)\eqfparse.obj \
       $(_OTMMARKUP_DLL)\otmquote.dll  
       
#---------------------------------------
#  Compile OBJs              
#---------------------------------------
$(_OTMMARKUP_OBJ)\otmquote.obj: \
      $(SRCPATH)\otmquote.c \
      $(_INC)\eqfparse.h \
      $(CMNPATH)\usrcalls.h 
    $(_COMPILER)  $(CFLAGS)  /Fo$(_OTMMARKUP_OBJ)\otmquote $(SRCPATH)\otmquote.c  /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\


#---------------------------------------
#  Link DLL               
#---------------------------------------
$(_OTMMARKUP_DLL)\otmquote.dll:       \
      $(_OTMMARKUP_OBJ)\otmquote.obj  \
      $(_OTMMARKUP_OBJ)\usrcalls.obj  \
      $(_OTMMARKUP_OBJ)\eqfcalls.obj  \
      $(_OBJ)\eqfparse.obj 
    @if exist $(_OTMMARKUP_DLL)\otmquote.dll  erase $(_OTMMARKUP_DLL)\otmquote.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmquote.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmquote.dll >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OTMMARKUP_OBJ)\otmquote.obj 
$(_OTMMARKUP_OBJ)\usrcalls.obj 
$(_OTMMARKUP_OBJ)\eqfcalls.obj 
$(_OBJ)\eqfparse.obj 
/OUT:$(_OTMMARKUP_DLL)\otmquote.dll /nologo /MACHINE:IX86 /ALIGN:0X1000 /DRIVER /DLL /NOD
/MAP:$(_OTMMARKUP_MAP)\otmquote.map
$(_LINK_LIB_CRT) 
$(_LIB)\OtmBase.lib 
$(_LIB)\OTMLinguistic.lib 
$(_LIB)\OTMTagTableFunctions.lib 
<<
    @copy $(_OTMMARKUP_DLL)\otmquote.dll $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmquote\otmquote.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmquote\otmquote.chr $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmquote\otmaquot.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmquote\otmaquot.chr $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmquote\otmuquot.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmquote\otmuquot.chr $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)


