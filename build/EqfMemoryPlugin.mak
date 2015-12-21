#-------------------------------------------------------------------------------
# EqfMemoryPlugin.MAK    - Makefile for standard Translation Memory Plugin DLL
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

build:	$(_DLL)\EqfMemoryPlugin.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\EQFNTM.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTM.C
$(_OBJ)\EQFMEMSD.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFMEMSD.C
$(_OBJ)\EQFMEMUT.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFMEMUT.C
$(_OBJ)\EQFNTCL.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTCL.C
$(_OBJ)\EQFNTCR.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTCR.C
$(_OBJ)\EQFNTDEL.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTDEL.C
$(_OBJ)\EQFNTEXT.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTEXT.C
$(_OBJ)\EQFNTGET.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTGET.CPP
$(_OBJ)\EQFNTMDB.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTMDB.C
$(_OBJ)\EQFNTOP.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTOP.C
$(_OBJ)\EQFNTPUT.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTPUT.C
$(_OBJ)\EQFNTTMT.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTTMT.C
$(_OBJ)\EQFNTUTL.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFNTUTL.C
$(_OBJ)\EQFTMRTV.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFTMRTV.C
$(_OBJ)\EQFTMSTR.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFTMSTR.C
$(_OBJ)\EQFTMUPD.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFTMUPD.C
$(_OBJ)\EQFTMUTL.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFTMUTL.C
$(_OBJ)\EQFTMEXT.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EQFTMEXT.C

$(_OBJ)\EqfMemoryPlugin.OBJ:	$(_SRC)\plugins\memory\EqfMemoryPlugin\EqfMemoryPlugin.cpp \
								$(_SRC)\plugins\memory\EqfMemoryPlugin\EqfMemoryPlugin.h \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\OtmPlugin.h \
								$(_SRC)\core\PluginManager\OtmMemoryPlugin.h

$(_OBJ)\EqfMemory.OBJ:			$(_SRC)\plugins\memory\EqfMemoryPlugin\EqfMemory.cpp \
								$(_SRC)\plugins\memory\EqfMemoryPlugin\EqfMemory.h \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\OtmMemory.h

$(_DLL)\EqfMemoryPlugin.DLL:	$(_OBJ)\EQFNTM.OBJ \
								$(_OBJ)\EQFMEMSD.OBJ \
								$(_OBJ)\EQFMEMUT.OBJ \
								$(_OBJ)\EQFNTCL.OBJ \
								$(_OBJ)\EQFNTCR.OBJ \
								$(_OBJ)\EQFNTDEL.OBJ \
								$(_OBJ)\EQFNTEXT.OBJ \
								$(_OBJ)\EQFNTGET.OBJ \
								$(_OBJ)\EQFNTMDB.OBJ \
								$(_OBJ)\EQFNTOP.OBJ \
								$(_OBJ)\EQFNTPUT.OBJ \
								$(_OBJ)\EQFNTTMT.OBJ \
								$(_OBJ)\EQFNTUTL.OBJ \
								$(_OBJ)\EQFTMRTV.OBJ \
								$(_OBJ)\EQFTMSTR.OBJ \
								$(_OBJ)\EQFTMUPD.OBJ \
								$(_OBJ)\EQFTMUTL.OBJ \
								$(_OBJ)\EQFTMEXT.OBJ \
								$(_OBJ)\EqfMemoryPlugin.OBJ \
								$(_OBJ)\EqfMemory.OBJ \
								$(_LIB)\OtmAlloc.lib \
								$(_LIB)\OtmBase.lib \
								$(_LIB)\OtmDll.lib \
								$(_LIB)\PluginManager.lib

#------------------------------------------------------------------------------
# Build EqfMemoryPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------

$(_DLL)\EqfMemoryPlugin.DLL:
    @echo ---- Linking $(_DLL)\EqfMemoryPlugin.DLL
    @echo ---- Linking $(_DLL)\EqfMemoryPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
					$(_OBJ)\EQFNTM.OBJ
					$(_OBJ)\EQFMEMSD.OBJ
					$(_OBJ)\EQFMEMUT.OBJ
					$(_OBJ)\EQFNTCL.OBJ
					$(_OBJ)\EQFNTCR.OBJ
					$(_OBJ)\EQFNTDEL.OBJ
					$(_OBJ)\EQFNTEXT.OBJ
					$(_OBJ)\EQFNTGET.OBJ
					$(_OBJ)\EQFNTMDB.OBJ
					$(_OBJ)\EQFNTOP.OBJ
					$(_OBJ)\EQFNTPUT.OBJ
					$(_OBJ)\EQFNTTMT.OBJ
					$(_OBJ)\EQFNTUTL.OBJ
					$(_OBJ)\EQFTMRTV.OBJ
					$(_OBJ)\EQFTMSTR.OBJ
					$(_OBJ)\EQFTMUPD.OBJ
					$(_OBJ)\EQFTMUTL.OBJ
					$(_OBJ)\EQFTMEXT.OBJ
					$(_OBJ)\EqfMemoryPlugin.OBJ
					$(_OBJ)\EqfMemory.OBJ
/OUT:$(_DLL)\EqfMemoryPlugin.DLL
/MAP:$(_MAP)\EqfMemoryPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmAlloc.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmDll.lib $(_LIB)\OTMQDAM.LIB $(_LIB)\PluginManager.lib $(_LIB)\OTMGLOBM.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @copy $(_DLL)\EqfMemoryPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)