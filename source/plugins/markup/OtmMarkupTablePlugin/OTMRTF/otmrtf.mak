# ---------------------------------------------------------------------------
#  Copyright (C) 1998-2017, International Business Machines          
#         Corporation and others. All rights reserved 
# ---------------------------------------------------------------------------
SRCPATH = $(_OTMMARKUP_SRC)\otmrtf
CMNPATH = $(_OTMMARKUP_SRC)\common  
INCLUDEOPT  = /I$(SRCPATH) /I$(CMNPATH)


#CFLAGS    = $(_OTMMARKUP_CFLAGS) $(INCLUDEOPT)
CFLAGS    = $(_CL_OPTIONS_DLL) $(INCLUDEOPT)
LINKFLAGS = $(_OTMMARKUP_LINKFLAGS)


.all: \
       $(_OTMMARKUP_OBJ)\otmrtf.obj \
       $(_OTMMARKUP_DLL)\otmrtf.dll  
       
#---------------------------------------
#  Compile OBJs              
#---------------------------------------
$(_OTMMARKUP_OBJ)\otmrtf.obj: \
      $(SRCPATH)\otmrtf.c \
      $(SRCPATH)\otmrtf.h \
      $(CMNPATH)\usrcalls.h 
    $(_COMPILER)  $(CFLAGS)  /Fo$(_OTMMARKUP_OBJ)\otmrtf $(SRCPATH)\otmrtf.c  /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\


#---------------------------------------
#  Link DLL               
#---------------------------------------
$(_OTMMARKUP_DLL)\otmrtf.dll:         \
      $(_OTMMARKUP_OBJ)\otmrtf.obj    \
      $(_OTMMARKUP_OBJ)\usrcalls.obj  \
      $(_OTMMARKUP_OBJ)\eqfcalls.obj 
    @if exist $(_OTMMARKUP_DLL)\otmrtf.dll  erase $(_OTMMARKUP_DLL)\otmrtf.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmrtf.dll
    @echo ---- Linking  $(_OTMMARKUP_DLL)\otmrtf.dll >>$(_ERR)
    @$(_LINKER) >>$(_ERR) @<<
$(_OTMMARKUP_OBJ)\otmrtf.obj 
$(_OTMMARKUP_OBJ)\usrcalls.obj 
$(_OTMMARKUP_OBJ)\eqfcalls.obj 
/OUT:$(_OTMMARKUP_DLL)\otmrtf.dll $(LINKFLAGS)
/MAP:$(_OTMMARKUP_MAP)\otmrtf.map
$(_LINK_LIB_CRT) 
$(_LIB)\OtmBase.lib
$(_LIB)\OtmLinguistic.lib
$(_LIB)\OtmSegmentedFile.lib
$(_LIB)\OtmTagTableFunctions.lib
$(_LIB)\OtmAnalysisFunctions.lib
<<
    @copy $(_OTMMARKUP_DLL)\otmrtf.dll $(_OTMMARKUP_RELEASE_DIR)\BIN /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmrtf\otmrtf.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmrtf\otmrtf.chr $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)


