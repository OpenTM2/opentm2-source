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
        $(_DLL)\ValDocXML.DLL \
        $(_DLL)\ValDocPwb.DLL \
        $(_DLL)\ValDocDocx.DLL

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

$(_OBJ)\OtmProofReadWindow.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadWindow.cpp 

$(_OBJ)\OtmProofReadProcess.OBJ: $(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadProcess.cpp 

$(_OBJ)\OtmProofReadEntry.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadEntry.cpp 

$(_OBJ)\OtmProofReadList.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadList.cpp 

$(_OBJ)\CXMLWriter.OBJ:	$(_SRC)\tools\CXMLWriter.CPP

$(_DLL)\OtmProofReadImportPlugin.DLL:	$(_OBJ)\OtmProofReadImportPlugin.OBJ \
                                $(_OBJ)\OtmProofReadWindow.OBJ \
                                $(_OBJ)\OtmProofReadEntry.OBJ \
                                $(_OBJ)\OtmProofReadList.OBJ \
                                $(_OBJ)\OtmProofReadProcess.OBJ \
                                $(_OBJ)\CXMLWriter.OBJ \
                                $(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadImport.rc \
				$(_LIB)\PluginManager.lib


$(_OBJ)\ValDocXML.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocXML.cpp 


$(_DLL)\ValDocXML.DLL:	$(_OBJ)\ValDocXML.OBJ 



$(_OBJ)\ValDocPwb.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocPwb\ValDocPwb.cpp \
                        $(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadFilter.h

$(_DLL)\ValDocPwb.DLL:	$(_OBJ)\ValDocPwb.OBJ 



$(_OBJ)\ValDocDocx.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocx.cpp \
                       	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocxParse.cpp \
                       	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocx.h \
                        $(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadFilter.h

$(_OBJ)\ValDocDocxParse.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocxParse.cpp \
                       	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocx.h \
                       	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocxUtil.h

$(_OBJ)\ValDocDocxUtil.OBJ:	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocxUtil.cpp \
                       	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocx.h \
                       	$(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocxUtil.h

$(_DLL)\ValDocDocx.DLL:	$(_OBJ)\ValDocDocx.OBJ \
                       	$(_OBJ)\ValDocDocxParse.OBJ \
                       	$(_OBJ)\ValDocDocxUtil.OBJ 

#------------------------------------------------------------------------------
# Build OtmProofReadImportPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------

$(_DLL)\OtmProofReadImportPlugin.DLL:
    @echo ---- Compiling resource $(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadImport.RC >>$(_ERR)
    RC /D_WINDOWS /Fo$(_OBJ)\OtmProofReadImport.RES $(_SRC)\plugins\tools\OtmProofReadImport\OtmProofReadImport.RC
    @echo ---- Converting resource $(_OBJ)\OtmProofReadImport.RES >>$(_ERR)
    CVTRES /NOLOGO /OUT:$(_OBJ)\OtmProofReadImport.RBJ $(_OBJ)\OtmProofReadImport.RES
    @echo ---- Linking $(_DLL)\OtmProofReadImportPlugin.DLL
    @echo ---- Linking $(_DLL)\OtmProofReadImportPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
    $(_OBJ)\OtmProofReadImportPlugin.OBJ $(_OBJ)\OtmProofReadImport.RBJ $(_OBJ)\OtmProofReadProcess.OBJ
    $(_OBJ)\OtmProofReadWindow.OBJ $(_OBJ)\OtmProofReadEntry.OBJ $(_OBJ)\OtmProofReadList.OBJ $(_OBJ)\CXMLWriter.OBJ
/OUT:$(_DLL)\OtmProofReadImportPlugin.DLL
/MAP:$(_MAP)\OtmProofReadImportPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIB)\OtmDLL.lib $(_LIB)\PluginManager.lib $(_LIBOTHER)\xerces-c_3.lib Comctl32.lib
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

#------------------------------------------------------------------------------
# Build ValDocPwb filter and copy the DLL to the release directory
#------------------------------------------------------------------------------

$(_OBJ)\ValDocPwb.OBJ: \
      $(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocPwb\ValDocPwb.cpp
    $(_COMPILER)  $(_CL_CPP_OPTIONS_DLL)  /Fo$(_OBJ)\ValDocPwb $(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocPwb\ValDocPwb.cpp

$(_DLL)\ValDocPwb.DLL:
    @echo ---- Linking $(_DLL)\ValDocPwb.DLL
    @echo ---- Linking $(_DLL)\ValDocPwb.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
    $(_OBJ)\ValDocPwb.OBJ 
/OUT:$(_DLL)\ValDocPwb.DLL
/MAP:$(_MAP)\ValDocPwb.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\PluginManager.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmDLL.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport md $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter md $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter
    @copy $(_DLL)\ValDocPwb.DLL $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter /Y>$(_ERR)

#------------------------------------------------------------------------------
# Build ValDocDocx filter and copy the DLL to the release directory
#------------------------------------------------------------------------------

_ValDocDocx_CPP_OPTIONS = /nologo /c /W2 /MT /EHsc /TP /Zp1 /DWIN32BIT /D_WINDOWS /D_WIN32_WINNT=0x0501 /DUNICODE /wd4229 /Zi
_ValDocDocx_LINK_OPTIONS = /nologo /MACHINE:IX86 /ALIGN:0X1000 /DRIVER /DLL /DEBUG

$(_OBJ)\ValDocDocx.OBJ: \
      $(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocx.cpp
    $(_COMPILER)  $(_ValDocDocx_CPP_OPTIONS) /Fo$(_OBJ)\ValDocDocx $(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocx.cpp
$(_OBJ)\ValDocDocxParse.OBJ: \
      $(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocxParse.cpp
    $(_COMPILER)  $(_ValDocDocx_CPP_OPTIONS) /Fo$(_OBJ)\ValDocDocxParse $(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocxParse.cpp
$(_OBJ)\ValDocDocxUtil.OBJ: \
      $(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocxUtil.cpp
    $(_COMPILER)  $(_ValDocDocx_CPP_OPTIONS) /Fo$(_OBJ)\ValDocDocxUtil $(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\ValDocDocxUtil.cpp

$(_DLL)\ValDocDocx.DLL:
    @echo ---- Linking $(_DLL)\ValDocDocx.DLL
    @echo ---- Linking $(_DLL)\ValDocDocx.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
    $(_OBJ)\ValDocDocx.OBJ 
    $(_OBJ)\ValDocDocxParse.OBJ 
    $(_OBJ)\ValDocDocxUtil.OBJ 
/OUT:$(_DLL)\ValDocDocx.DLL
/MAP:$(_MAP)\ValDocDocx.MAP $(_ValDocDocx_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LIB)\PluginManager.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmDLL.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport md $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport
    @if not exist $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter md $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter
    @copy $(_DLL)\ValDocDocx.DLL $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter /Y>$(_ERR)
    @copy $(_SRC)\plugins\tools\OtmProofReadImport\filter\ValDocDocx\UNZIP.EXE $(RELEASE_DIR)\OTM\Plugins\OtmProofReadImport\filter /Y>$(_ERR)
