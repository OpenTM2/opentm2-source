# ---------------------------------------------------------------------------
#  Copyright (C) 1998-2017, International Business Machines          
#         Corporation and others. All rights reserved 
# ---------------------------------------------------------------------------
SRCPATH = $(_OTMMARKUP_SRC)\otmamri
CMNPATH = $(_OTMMARKUP_SRC)\common  
INCLUDEOPT  = /I$(SRCPATH) /I$(CMNPATH)


#CFLAGS    = $(_OTMMARKUP_CFLAGS) $(INCLUDEOPT)
CFLAGS    = $(_CL_CPP_OPTIONS_DLL) $(INCLUDEOPT)
LINKFLAGS = $(_OTMMARKUP_LINKFLAGS)


.all: \
       $(_OTMMARKUP_OBJ)\otmbmri.obj \
       $(_OBJ)\eqfparse.obj \
       $(_OTMMARKUP_DLL)\otmbmri.dll  
       
#---------------------------------------
#  Compile OBJs              
#---------------------------------------
$(_OTMMARKUP_OBJ)\otmbmri.obj: \
      $(SRCPATH)\otmbmri.c \
      $(_INC)\eqfparse.h \
      $(CMNPATH)\usrcalls.h 
    $(_COMPILER)  $(CFLAGS)  /Fo$(_OTMMARKUP_OBJ)\otmbmri $(SRCPATH)\otmbmri.c  /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\


#---------------------------------------
#  Link DLL               
#---------------------------------------
$(_OTMMARKUP_DLL)\otmbmri.dll:       \
      $(_OTMMARKUP_OBJ)\otmbmri.obj  \
      $(_OTMMARKUP_OBJ)\eqfcalls.obj \
      $(_OTMMARKUP_OBJ)\usrcalls.obj \
      $(_OBJ)\eqfparse.obj 
    @if exist $(_OTMMARKUP_DLL)\otmbmri.dll  erase $(_OTMMARKUP_DLL)\otmbmri.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmbmri.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmbmri.dll >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OTMMARKUP_OBJ)\otmbmri.obj 
$(_OTMMARKUP_OBJ)\eqfcalls.obj 
$(_OTMMARKUP_OBJ)\usrcalls.obj 
$(_OBJ)\eqfparse.obj 
/OUT:$(_OTMMARKUP_DLL)\otmbmri.dll /nologo /MACHINE:IX86 /ALIGN:0X1000 /DRIVER /DLL /NOD
/MAP:$(_OTMMARKUP_MAP)\otmbmri.map
$(_LINK_LIB_CRT) $(_LIB)\otmbase.lib $(_LIB)\OTMLinguistic.lib 
<<
    @copy $(_OTMMARKUP_DLL)\otmbmri.dll $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmamri\otmamri.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmamri\otmamri.chr $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)



