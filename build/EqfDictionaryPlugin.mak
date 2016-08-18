#-------------------------------------------------------------------------------
# EqfDictionaryPlugin.MAK - Makefile for standard Dictionary Plugin DLL
# Copyright (c) 2013, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\EqfDictionaryPlugin.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\EQFDASD.OBJ:				$(_SRC)\core\dictionary\EQFDASD.C
$(_OBJ)\EQFDASDM.OBJ:				$(_SRC)\core\dictionary\EQFDASDM.C
$(_OBJ)\EQFDASDT.OBJ:				$(_SRC)\core\dictionary\EQFDASDT.C
$(_OBJ)\EqfDictionaryPlugin.OBJ:	$(_SRC)\plugins\dictionary\EqfDictionaryPlugin.cpp \
									$(_SRC)\plugins\dictionary\EqfDictionaryPlugin.h \
									$(_SRC)\core\PluginManager\PluginManager.h \
									$(_SRC)\core\PluginManager\OtmPlugin.h \
									$(_SRC)\core\PluginManager\OtmDictionaryPlugin.h
$(_OBJ)\EqfDictionary.OBJ:	$(_SRC)\plugins\dictionary\EqfDictionary.cpp \
							$(_SRC)\plugins\dictionary\EqfDictionary.h \
							$(_SRC)\core\PluginManager\PluginManager.h \
							$(_SRC)\core\PluginManager\OtmDictionary.h


$(_DLL)\EqfDictionaryPlugin.DLL:	$(_OBJ)\EQFDASD.OBJ \
									$(_OBJ)\EQFDASDM.OBJ \
									$(_OBJ)\EQFDASDT.OBJ \
									$(_OBJ)\EqfDictionaryPlugin.OBJ \
									$(_OBJ)\EqfDictionary.OBJ \
									$(_LIB)\OtmAlloc.lib \
									$(_LIB)\OtmBase.lib \
									$(_LIB)\OtmDll.lib \
									$(_LIB)\PluginManager.lib

#------------------------------------------------------------------------------
# Build EqfDictionaryPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------

$(_DLL)\EqfDictionaryPlugin.DLL:
    @echo ---- Linking $(_DLL)\EqfDictionaryPlugin.DLL
    @echo ---- Linking $(_DLL)\EqfDictionaryPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
			$(_OBJ)\EQFDASD.OBJ
			$(_OBJ)\EQFDASDM.OBJ
			$(_OBJ)\EQFDASDT.OBJ
			$(_OBJ)\EQFQDSRV.OBJ
			$(_OBJ)\EQFDICRC.OBJ
			$(_OBJ)\EqfDictionaryPlugin.OBJ
			$(_OBJ)\EqfDictionary.OBJ
/OUT:$(_DLL)\EqfDictionaryPlugin.DLL
/MAP:$(_MAP)\EqfDictionaryPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_DLL)\OtmAlloc.lib $(_LIB)\OTMBase.lib $(_LIB)\OTMDll.lib $(_LIB)\OTMQDAM.LIB $(_LIB)\PluginManager.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @copy $(_DLL)\EqfDictionaryPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)
