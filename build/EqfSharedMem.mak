#-------------------------------------------------------------------------------
# EqfSharedMem.MAK    - Makefile for the shared memory plugin
# Copyright (c) 2017, International Business Machines
# Corporation and others.  All rights reserved.
#-------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------


!INCLUDE $(_BLD)\EQFRULES.MAK

{$(_SRC)\plugins\memory\SharedMem}.cpp{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_DLL) /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\SharedMem\$(*B).cpp

#  rule adjusted for Axis2C (files cannot be compiled using /TP switch)
{$(_SRC)\plugins\memory\SharedMem}.c{$(_OBJ)}.obj:
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).C
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).C >>$(_ERR)
   @$(_COMPILER) /nologo /c /Zp1 /EHsc /D_WINDOWS /DWIN32 /DAXIS2_DECLARE_EXPORT /DWIN32BIT /D_WIN32 /D_CRT_SECURE_NO_WARNINGS /D_USE_32BIT_TIME_T /Zi /Od /Ob2 /FR /GA -Od -Zi /D_DEBUG  /Fo$(_OBJ)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\SharedMem\$(*B).c

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\MemoryWebServiceClient.DLL  $(_DLL)\EqfSharedMemPlugin.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\adb_synchronize.obj: 			$(_SRC)\plugins\memory\SharedMem\adb_synchronize.c

$(_OBJ)\adb_synchronizeResponse.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_synchronizeResponse.c

$(_OBJ)\axis2_extension_mapper.obj: 		$(_SRC)\plugins\memory\SharedMem\axis2_extension_mapper.c

$(_OBJ)\axis2_stub_OtmTMServiceImplService.obj: $(_SRC)\plugins\memory\SharedMem\axis2_stub_OtmTMServiceImplService.c

$(_OBJ)\EqfSharedMemory.OBJ:		$(_SRC)\plugins\memory\SharedMem\EqfSharedMemory.CPP $(_SRC)\plugins\memory\SharedMem\EqfSharedMemory.H

$(_OBJ)\MemoryWebServiceClient.OBJ:	$(_SRC)\plugins\memory\SharedMem\MemoryWebServiceClient.CPP

$(_OBJ)\JSONFactory.OBJ:		$(_SRC)\plugins\memory\SharedMem\JSONFactory.CPP $(_SRC)\plugins\memory\SharedMem\JSONFactory.H

$(_OBJ)\FifoQueue.OBJ:		$(_SRC)\plugins\memory\SharedMem\FifoQueue.CPP $(_SRC)\plugins\memory\SharedMem\FifoQueue.H

$(_OBJ)\TMXFactory.OBJ:		$(_SRC)\plugins\memory\SharedMem\TMXFactory.CPP $(_SRC)\plugins\memory\SharedMem\TMXFactory.H

$(_OBJ)\EqfSharedMemoryPlugin.OBJ:  $(_SRC)\plugins\memory\SharedMem\EqfSharedMemoryPlugin.CPP $(_SRC)\plugins\memory\SharedMem\EqfSharedMemoryPlugin.H

$(_OBJ)\EqfSharedMemory.OBJ:  $(_SRC)\plugins\memory\SharedMem\EqfSharedMemory.CPP $(_SRC)\plugins\memory\SharedMem\EqfSharedMemory.H

$(_OBJ)\ReplicateThread.OBJ:  $(_SRC)\plugins\memory\SharedMem\ReplicateThread.CPP $(_SRC)\plugins\memory\SharedMem\ReplicateThread.H

$(_OBJ)\LogWriter.OBJ:  $(_SRC)\core\utilities\LogWriter.CPP $(_SRC)\core\utilities\LogWriter.H

$(_DLL)\EqfSharedMemPlugin.DLL:		$(_OBJ)\EqfSharedMemoryPlugin.OBJ \
					$(_OBJ)\EqfSharedMemory.OBJ \
					$(_OBJ)\ReplicateThread.OBJ \
					$(_OBJ)\JSONFactory.OBJ \
					$(_OBJ)\FifoQueue.OBJ \
					$(_OBJ)\TMXFactory.OBJ \
					$(_LIB)\OtmBase.lib \
					$(_LIB)\PluginManager.lib \
					$(_LIB)\MemoryWebServiceClient.lib


#------------------------------------------------------------------------------
# Build EqfSharedMemPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------
$(_DLL)\EqfSharedMemPlugin.DLL:
    @echo --- Linking $(_DLL)\EqfSharedMemPlugin.DLL
    @echo ---- Linking $(_DLL)\EqfSharedMemPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
	$(_OBJ)\ReplicateThread.OBJ 
	$(_OBJ)\EqfSharedMemoryPlugin.OBJ
	$(_OBJ)\EqfSharedMemory.OBJ 
	$(_OBJ)\JSONFactory.OBJ 
    $(_OBJ)\FifoQueue.OBJ
	$(_OBJ)\TMXFactory.OBJ 
/OUT:$(_DLL)\EqfSharedMemPlugin.DLL
/MAP:$(_MAP)\EqfSharedMemPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIB)\PluginManager.lib $(_LIBOTHER)\xerces-c_3.lib Shell32.lib $(_LIB)\MemoryWebServiceClient.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\Plugins\ md $(RELEASE_DIR)\OTM\Plugins
    @echo ************ Info *****************  
    @echo $(_DLL)\EqfSharedMemPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y
    @copy $(_DLL)\EqfSharedMemPlugin.DLL $(RELEASE_DIR)\OTM\Plugins /Y>$(_ERR)

MEMWEBSERVICEOBJS= $(_OBJ)\MemoryWebServiceClient.OBJ \
	$(_OBJ)\JSONFactory.OBJ \
    $(_OBJ)\FifoQueue.OBJ \
    $(_OBJ)\LogWriter.OBJ \
	$(_OBJ)\adb_synchronize.obj \
	$(_OBJ)\adb_synchronizeResponse.obj \
	$(_OBJ)\axis2_stub_OtmTMServiceImplService.obj \
	$(_OBJ)\axis2_extension_mapper.obj	
	
$(_DLL)\MemoryWebServiceClient.DLL: $(MEMWEBSERVICEOBJS)

#------------------------------------------------------------------------------
# Build MemoryWebServiceClient.DLL and copy  DLL to release directory
#------------------------------------------------------------------------------
$(_BIN)\MemoryWebServiceClient.DLL:
    @echo ---- Linking $(_DLL)\MemoryWebServiceClient.DLL
    @echo ---- Linking $(_DLL)\MemoryWebServiceClient.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
	$(MEMWEBSERVICEOBJS)
/OUT:$(_DLL)\MemoryWebServiceClient.DLL
/MAP:$(_MAP)\MemoryWebServiceClient.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) 
Shell32.lib 
$(AXIS2C_HOME)\lib\axutil.lib $(AXIS2C_HOME)\lib\axiom.lib $(AXIS2C_HOME)\lib\axis2_engine.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\WIN\ md $(RELEASE_DIR)\OTM\WIN
    @copy $(_BIN)\MemoryWebServiceClient.DLL $(RELEASE_DIR)\OTM\WIN /Y>$(_ERR)
