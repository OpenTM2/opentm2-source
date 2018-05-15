#-------------------------------------------------------------------------------
# UserMarkupTablePlugin.MAK    - Makefile for the EQF markup table plugin DLL
# Copyright (c) 2013-2017 International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK


#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:    $(_USERMARKUP_DLL)\UserMarkupTablePlugin.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF


#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_USERMARKUP_OBJ)\UserMarkupTablePlugin.OBJ:	$(_USERMARKUP_SRC)\UserMarkupTablePlugin.cpp \
                 				$(_USERMARKUP_SRC)\UserMarkupTablePlugin.h \
                 				$(_SRC)\core\pluginmanager\PluginManager.h \
                 				$(_SRC)\core\pluginmanager\OtmPlugin.h \
                 				$(_SRC)\core\pluginmanager\OtmMarkupPlugin.h
$(_USERMARKUP_OBJ)\UserMarkupTable.OBJ:	        $(_USERMARKUP_SRC)\UserMarkupTable.cpp \
		           		        $(_USERMARKUP_SRC)\UserMarkupTable.h \
		           		        $(_SRC)\core\pluginmanager\PluginManager.h \
		           		        $(_SRC)\core\pluginmanager\OtmMarkup.h
$(_USERMARKUP_DLL)\UserMarkupTablePlugin.DLL:	$(_USERMARKUP_OBJ)\UserMarkupTablePlugin.OBJ \
				                $(_USERMARKUP_OBJ)\UserMarkupTable.OBJ \
				                $(_LIB)\OtmBase.lib \
				                $(_LIB)\PluginManager.lib

#------------------------------------------------------------------------------
# Build uUserMarkupPlugin.DLL
#------------------------------------------------------------------------------

{$(_USERMARKUP_SRC)\}.cpp{$(_USERMARKUP_OBJ)}.obj:
    @echo ---- Compiling $(_USERMARKUP_SRC)\$(*B).CPP
    @echo ---- Compiling $(_USERMARKUP_SRC)\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /I$(_USERMARKUP_SRC) /Fo$(_USERMARKUP_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_USERMARKUP_SRC)\$(*B).cpp


$(_USERMARKUP_DLL)\UserMarkupTablePlugin.DLL:
    @if exist $(_USERMARKUP_DLL)\UserMarkupTablePlugin.dll  erase $(_USERMARKUP_DLL)\UserMarkupTablePlugin.dll
    @echo ---- Linking $(_USERMARKUP_DLL)\UserMarkupTablePlugin.DLL
    @echo ---- Linking $(_USERMARKUP_DLL)\UserMarkupTablePlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
	 $(_USERMARKUP_OBJ)\UserMarkupTablePlugin.OBJ
	 $(_USERMARKUP_OBJ)\UserMarkupTable.OBJ
/OUT:$(_USERMARKUP_DLL)\UserMarkupTablePlugin.DLL
/MAP:$(_USERMARKUP_MAP)\UserMarkupTablePlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OTMBase.lib  $(_LIB)\PluginManager.lib
<<


    @copy $(_USERMARKUP_DLL)\UserMarkupTablePlugin.DLL $(_USERMARKUP_RELEASE_DIR)\ /Y>$(_ERR)
