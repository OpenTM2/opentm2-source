#-------------------------------------------------------------------------------
# EqfMemoryPlugin.MAK    - Makefile for Tmgr Shared and LAN-based Translation Memory Plugin DLL
# Copyright (c) 2017, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

{$(_SRC)\plugins\memory\EqfSharedOnLanMemoryPlugin}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\memory\EqfSharedOnLanMemoryPlugin\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\memory\EqfSharedOnLanMemoryPlugin\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\EqfSharedOnLanMemoryPlugin\$(*B).cpp


#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\EqfSharedOnLanMemoryPlugin.DLL

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

$(_OBJ)\EqfSharedOnLanMemoryPlugin.OBJ:	$(_SRC)\plugins\memory\EqfSharedOnLanMemoryPlugin\EqfSharedOnLanMemoryPlugin.cpp \
								$(_SRC)\plugins\memory\EqfSharedOnLanMemoryPlugin\EqfSharedOnLanMemoryPlugin.h \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\OtmPlugin.h \
								$(_SRC)\core\PluginManager\OtmMemoryPlugin.h

$(_OBJ)\EqfSharedOnLanMemory.OBJ:			$(_SRC)\plugins\memory\EqfSharedOnLanMemoryPlugin\EqfSharedOnLanMemory.cpp \
								$(_SRC)\plugins\memory\EqfSharedOnLanMemoryPlugin\EqfSharedOnLanMemory.h \
								$(_SRC)\core\PluginManager\PluginManager.h \
								$(_SRC)\core\PluginManager\OtmMemory.h

$(_DLL)\EqfSharedOnLanMemoryPlugin.DLL:	$(_OBJ)\EQFNTM.OBJ \
								$(_OBJ)\EqfSharedOnLanMemoryPlugin.OBJ \
								$(_OBJ)\EqfSharedOnLanMemory.OBJ \
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
								$(_LIB)\OtmBase.lib \
								$(_LIB)\OtmDll.lib \
								$(_LIB)\PluginManager.lib

#------------------------------------------------------------------------------
# Build EqfSharedOnLanMemoryPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------

$(_DLL)\EqfSharedOnLanMemoryPlugin.DLL:
    @echo ---- Linking $(_DLL)\EqfSharedOnLanMemoryPlugin.DLL
    @echo ---- Linking $(_DLL)\EqfSharedOnLanMemoryPlugin.DLL >>$(_ERR)
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
					$(_OBJ)\EqfSharedOnLanMemoryPlugin.OBJ
					$(_OBJ)\EqfSharedOnLanMemory.OBJ
/OUT:$(_DLL)\EqfSharedOnLanMemoryPlugin.DLL
/MAP:$(_MAP)\EqfSharedOnLanMemoryPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIB)\OtmDll.lib $(_LIB)\OTMQDAM.LIB $(_LIB)\PluginManager.lib $(_LIB)\OTMGLOBM.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @copy $(_DLL)\EqfSharedOnLanMemoryPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)