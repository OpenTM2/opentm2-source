#-------------------------------------------------------------------------------
# OtmMarkupTablePlugin.mak    - Makefile for the OTM markup table plugin DLL
#                               and the related markup table user exits 
# Copyright (c) 2013,2017 International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------
 
build:    $(_OTMMARKUP_DLL)\OtmMarkupTablePlugin.DLL \
          $(_OTMMARKUP_RELEASE_DIR)\TABLE\OtmMarkupTablePlugin.XML \
          $(_OTMMARKUP_DLL)\OTMBMRI.DLL     \
          $(_OTMMARKUP_DLL)\OTMRTF.DLL      \
          $(_OTMMARKUP_DLL)\OTMQUOTE.DLL    \
          $(_OTMMARKUP_DLL)\OTMHTM32.DLL    \
          $(_OTMMARKUP_DLL)\OTMJDK11.DLL    \
          $(_OTMMARKUP_DLL)\OTMMSOFC.DLL    \
          $(_OTMMARKUP_DLL)\OTMXML.DLL      \
          $(_OTMMARKUP_DLL)\OTMXMODC.DLL 


#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Build OtmMarkupPlugin.DLL
# and copy table files and plugin DLL to release directory
#------------------------------------------------------------------------------
$(_OTMMARKUP_OBJ)\OtmMarkupTablePlugin.OBJ:   $(_OTMMARKUP_SRC)\OtmMarkupTablePlugin.cpp \
                                              $(_OTMMARKUP_SRC)\OtmMarkupTablePlugin.h \
                                              $(_SRC)\core\pluginmanager\PluginManager.h \
                                              $(_SRC)\core\pluginmanager\OtmPlugin.h \
                                              $(_SRC)\core\pluginmanager\OtmMarkupPlugin.h
$(_OTMMARKUP_OBJ)\OtmMarkupTable.OBJ:	      $(_OTMMARKUP_SRC)\OtmMarkupTable.cpp \
                                              $(_OTMMARKUP_SRC)\OtmMarkupTable.h \
                                              $(_SRC)\core\pluginmanager\PluginManager.h \
                                              $(_SRC)\core\pluginmanager\OtmMarkup.h
$(_OTMMARKUP_DLL)\OtmMarkupTablePlugin.DLL:   $(_OTMMARKUP_OBJ)\OtmMarkupTablePlugin.OBJ \
                                              $(_OTMMARKUP_OBJ)\OtmMarkupTable.OBJ \
                                              $(_LIB)\OtmBase.lib \
                                              $(_LIB)\OtmDll.lib \
                                              $(_LIB)\PluginManager.lib
$(_OTMMARKUP_RELEASE_DIR)\TABLE\OtmMarkupTablePlugin.XML:   $(_OTMMARKUP_SRC)\OtmMarkupTablePlugin.XML

{$(_OTMMARKUP_SRC)\}.cpp{$(_OTMMARKUP_OBJ)}.obj:
    @echo ---- Compiling $(_OTMMARKUP_SRC)\$(*B).CPP
    @echo ---- Compiling $(_OTMMARKUP_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /I$(_OTMMARKUP_SRC) /Fo$(_OTMMARKUP_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_OTMMARKUP_SRC)\$(*B).cpp

$(_OTMMARKUP_DLL)\OtmMarkupTablePlugin.DLL:
    @if exist $(_OTMMARKUP_DLL)\OtmMarkupTablePlugin.dll  erase $(_OTMMARKUP_DLL)\OtmMarkupTablePlugin.dll
    @echo ---- Linking $(_OTMMARKUP_DLL)\OtmMarkupTablePlugin.DLL
    @echo ---- Linking $(_OTMMARKUP_DLL)\OtmMarkupTablePlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
        $(_OTMMARKUP_OBJ)\OtmMarkupTablePlugin.OBJ
        $(_OTMMARKUP_OBJ)\OtmMarkupTable.OBJ
/OUT:$(_OTMMARKUP_DLL)\OtmMarkupTablePlugin.DLL
/MAP:$(_OTMMARKUP_MAP)\OtmMarkupTablePlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS      
    $(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib  $(_LIB)\OtmDll.lib $(_LIB)\PluginManager.lib
<<

$(_OTMMARKUP_RELEASE_DIR)\TABLE\OtmMarkupTablePlugin.XML:
    @copy $(_OTMMARKUP_SRC)\OtmMarkupTablePlugin.XML $(_OTMMARKUP_RELEASE_DIR)\ /Y>$(_ERR)

    @copy $(_OTMMARKUP_DLL)\OtmMarkupTablePlugin.DLL $(_OTMMARKUP_RELEASE_DIR)\ /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\OtmMarkupTablePlugin.XML $(_OTMMARKUP_RELEASE_DIR)\ /Y>$(_ERR)


#------------------------------------------------------------------------------
# OTMASCII.   Build markup table.
#------------------------------------------------------------------------------
    @copy $(_OTMMARKUP_SRC)\otmascii\otmascii.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmascii\otmansi.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmascii\otmutf8.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmascii\otmutf16.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)


#------------------------------------------------------------------------------
# COMMON.    Build common routines. 
#------------------------------------------------------------------------------
#   @nmake $(_MAKEOPT) $(_OTMMARKUP_SRC)\common\common.mak
!INCLUDE $(_OTMMARKUP_SRC)\common\common.mak


#------------------------------------------------------------------------------
# OTMAMRI.    Build markup table.
#------------------------------------------------------------------------------
#   @nmake $(_MAKEOPT) $(_OTMMARKUP_SRC)\otmamri\otmamri.mak
!INCLUDE $(_OTMMARKUP_SRC)\otmamri\otmamri.mak


#------------------------------------------------------------------------------
# OTMDQUOT.  Build markup table.
#------------------------------------------------------------------------------
    @copy $(_OTMMARKUP_SRC)\otmdquot\otmdquot.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmdquot\otmadquo.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmdquot\otmadquo.chr $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmdquot\otmudquo.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmdquot\otmudquo.chr $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)


#------------------------------------------------------------------------------
# OTMHTM32.  Build markup table.
#------------------------------------------------------------------------------
#   @nmake $(_MAKEOPT) $(_OTMMARKUP_SRC)\otmhtm32\otmhtm32.mak
!INCLUDE $(_OTMMARKUP_SRC)\otmhtm32\otmhtm32.mak


#------------------------------------------------------------------------------
# OTMJDK21.  Build markup table.
#------------------------------------------------------------------------------
#   @nmake $(_MAKEOPT) $(_OTMMARKUP_SRC)\otmjdk21\otmjdk21.mak
!INCLUDE $(_OTMMARKUP_SRC)\otmjdk21\otmjdk21.mak


#------------------------------------------------------------------------------
# OTMMSOFC.  Build markup table.
#------------------------------------------------------------------------------
#   @nmake $(_MAKEOPT) $(_OTMMARKUP_SRC)\otmmsofc\otmmsofc.mak
!INCLUDE $(_OTMMARKUP_SRC)\otmmsofc\otmmsofc.mak


#------------------------------------------------------------------------------
# OTMQUOTE.  Build markup table.
#------------------------------------------------------------------------------
#   @nmake $(_MAKEOPT) $(_OTMMARKUP_SRC)\otmquote\otmquote.mak
!INCLUDE $(_OTMMARKUP_SRC)\otmquote\otmquote.mak


#------------------------------------------------------------------------------
# OTMRTF.  Build markup table.
#------------------------------------------------------------------------------
#   @nmake $(_MAKEOPT) $(_OTMMARKUP_SRC)\otmrtf\otmrtf.mak
!INCLUDE $(_OTMMARKUP_SRC)\otmrtf\otmrtf.mak


#------------------------------------------------------------------------------
# OTMXHTML.   Build markup table.
#------------------------------------------------------------------------------
    @copy $(_OTMMARKUP_SRC)\otmxhtml\otmxahtm.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxhtml\otmxuhtm.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxhtml\otmxahtm.xml $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxhtml\otmxuhtm.xml $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)


#------------------------------------------------------------------------------
# OTMXML.  Build markup table.
#------------------------------------------------------------------------------
#   @nmake $(_MAKEOPT) $(_OTMMARKUP_SRC)\otmxml\otmxml.mak
!INCLUDE $(_OTMMARKUP_SRC)\otmxml\otmxml.mak


#------------------------------------------------------------------------------
# OTMXMODC.  Build markup table.
#------------------------------------------------------------------------------
#   @nmake $(_MAKEOPT) $(_OTMMARKUP_SRC)\otmxmodc\otmxmodc.mak
!INCLUDE $(_OTMMARKUP_SRC)\otmxmodc\otmxmodc.mak


#------------------------------------------------------------------------------
# OTMXMXLF.   Build markup table.
#------------------------------------------------------------------------------
    @copy $(_OTMMARKUP_SRC)\otmxmxlf\otmxmxlf.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxmxlf\otmxaxlf.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxmxlf\otmxuxlf.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxmxlf\otmxliff.xml $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    
#------------------------------------------------------------------------------
# OTMXMSDL.   Build markup table.
#------------------------------------------------------------------------------
    @copy $(_OTMMARKUP_SRC)\otmxmxlf\otmxusdl.tbl $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
    @copy $(_OTMMARKUP_SRC)\otmxmxlf\otmsdlxlf.xml $(_OTMMARKUP_RELEASE_DIR)\TABLE /Y>$(_ERR)
