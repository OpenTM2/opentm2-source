#-------------------------------------------------------------------------------
# EqfDocumentPlugin.MAK    - Makefile for standard document Plugin DLL
# Copyright (c) 2014-2017, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\EqfDocumentPlugin.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\EQFBFILE.OBJ:			$(_SRC)\core\editor\EQFBFILE.CPP
$(_OBJ)\EQFBDOC.OBJ:			$(_SRC)\core\editor\EQFBDOC.c
$(_OBJ)\EqfDocumentPlugin.OBJ:	$(_SRC)\plugins\document\EqfDocumentPlugin.cpp \
								$(_SRC)\plugins\document\EqfDocumentPlugin.h \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\OtmPlugin.h \
								$(_SRC)\core\PluginManager\OtmDocumentPlugin.h
$(_OBJ)\EqfDocument.OBJ:		$(_SRC)\plugins\document\EqfDocument.cpp \
								$(_SRC)\plugins\document\EqfDocument.h \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\OtmDocument.h

$(_DLL)\EqfDocumentPlugin.DLL:	$(_OBJ)\EQFBFILE.OBJ \
								$(_OBJ)\EQFBDOC.OBJ \
								$(_OBJ)\EqfDocumentPlugin.OBJ \
								$(_OBJ)\EqfDocument.OBJ \
								$(_LIB)\OtmBase.lib \
								$(_LIB)\PluginManager.lib

#------------------------------------------------------------------------------
# Build EqfDocumentPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------

$(_DLL)\EqfDocumentPlugin.DLL:
    @echo ---- Linking $(_DLL)\EqfDocumentPlugin.DLL
    @echo ---- Linking $(_DLL)\EqfDocumentPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
					$(_OBJ)\EQFBFILE.OBJ
					$(_OBJ)\EQFBDOC.OBJ
					$(_OBJ)\EqfDocumentPlugin.OBJ
					$(_OBJ)\EqfDocument.OBJ
/OUT:$(_DLL)\EqfDocumentPlugin.DLL
/MAP:$(_MAP)\EqfDocumentPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) 
$(_LIB)\OtmBase.lib 
$(_LIB)\OtmAPI.lib 
$(_LIB)\OTMEditorFunctions.lib 
$(_LIB)\OTMTagTableFunctions.lib 
$(_LIB)\OtmLinguistic.lib
$(_LIB)\OTMAnalysisFunctions.lib 
$(_LIB)\OtmSegmentedFile.lib  
$(_LIB)\PluginManager.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @copy $(_DLL)\EqfDocumentPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)
