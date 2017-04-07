#-------------------------------------------------------------------------------
# OtmProofReadImportPlugin.MAK  - Makefile for the OtmProofReadImportPlugin DLL
# Copyright (c) 2017, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

{$(_SRC)\plugins\tools\OtmProofReadImport}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProofReadImport\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProofReadImport\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProofReadImport\$(*B).cpp

{$(_SRC)\plugins\tools\OtmProofReadImport\filter}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProofReadImport\filter\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\tools\OtmProofReadImport\filter\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\tools\OtmProofReadImport\filter\$(*B).cpp



#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\OtmProofReadImportPlugin.DLL \
        $(_DLL)\ValDocXML.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------


$(_OBJ)\OtmProofReadImportPlugin.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadImportPlugin.cpp \
				$(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadImportPlugin.h \
				$(_SRC)\core\PluginManager\PluginManager.h \
				$(_SRC)\core\PluginManager\OtmPlugin.h \
				$(_SRC)\core\PluginManager\OtmToolPlugin.h


$(_DLL)\OtmProofReadImportPlugin.DLL:	$(_OBJ)\OtmProofReadImportPlugin.OBJ \
				$(_LIB)\PluginManager.lib

$(_OBJ)\ValDocXML.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocXML.cpp 


$(_DLL)\ValDocXML.DLL:	$(_OBJ)\ValDocXML.OBJ 

#------------------------------------------------------------------------------
# Build OtmProofReadImportPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------

#$(_DLL)\OtmProofReadImportPlugin.DLL:
#    @echo ---- Compiling resource $(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadImportPlugin.RC >>$(_ERR)
#    RC /D_WINDOWS /Fo$(_OBJ)\OtmCleanupPlugin.RES $(_SRC)\plugins\tools\OtmCleanupPlugin\OtmCleanupPlugin.RC
#    @echo ---- Converting resource $(_OBJ)\OtmCleanupPlugin.RES >>$(_ERR)
#    CVTRES /NOLOGO /OUT:$(_OBJ)\OtmCleanupPlugin.RBJ $(_OBJ)\OtmCleanupPlugin.RES
#    @echo ---- Linking $(_DLL)\OtmProofReadImportPlugin.DLL
#    @echo ---- Linking $(_DLL)\OtmProofReadImportPlugin.DLL >>$(_ERR)
#    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
#    $(_OBJ)\OtmProofReadImportPlugin.OBJ $(_OBJ)\OtmCleanupPlugin.RBJ
#    $(_OBJ)\OtmProofReadImportPlugin.OBJ $(_OBJ)\OtmCleanupPlugin.RBJ
#/OUT:$(_DLL)\OtmCleanupPlugin.DLL
#/MAP:$(_MAP)\OtmCleanupPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
#$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIB)\OtmDLL.lib
#<<
#    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
#    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport md $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport
#    @copy $(_DLL)\OtmCleanupPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)

$(_DLL)\OtmProofReadImportPlugin.DLL:
    @echo ---- Linking $(_DLL)\OtmProofReadImportPlugin.DLL
    @echo ---- Linking $(_DLL)\OtmProofReadImportPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
    $(_OBJ)\OtmProofReadImportPlugin.OBJ 
/OUT:$(_DLL)\OtmProofReadImportPlugin.DLL
/MAP:$(_MAP)\OtmProofReadImportPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIB)\OtmDLL.lib $(_LIB)\PluginManager.lib 
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport md $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport
    @copy $(_DLL)\OtmProofReadImportPlugin.DLL $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport /Y>$(_ERR)

#------------------------------------------------------------------------------
# Build ValDocXML filter and copy the DLL to the release directory
#------------------------------------------------------------------------------
$(_DLL)\ValDocXML.DLL:
    @echo ---- Linking $(_DLL)\ValDocXML.DLL
    @echo ---- Linking $(_DLL)\ValDocXML.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
    $(_OBJ)\ValDocXML.OBJ 
/OUT:$(_DLL)\ValDocXML.DLL
/MAP:$(_MAP)\ValDocXML.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\PluginManager.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmDLL.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport md $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter md $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter
    @copy $(_DLL)\ValDocXML.DLL $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter /Y>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter\ValDocXML md $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter\ValDocXML
    @copy $(_SRC)\Plugins\tools\OtmProofReadImport\filter\ValDocXML\ValDocXML.XSL $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter\ValDocXML /Y>$(_ERR)


