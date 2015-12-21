#-------------------------------------------------------------------------------
# EqfSharedMem.MAK    - Makefile for the shared memory plugin
# Copyright (c) 2014, International Business Machines
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

build:	$(_DLL)\MemoryWebServiceClient.DLL  $(_DLL)\EqfSharedMemPlugin.DLL  $(_BIN)\OtmMemReplicator.EXE 

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------


$(_OBJ)\adb_bExistsDataSource.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_bExistsDataSource.c

$(_OBJ)\adb_bExistsDataSourceResponse.obj: 	$(_SRC)\plugins\memory\SharedMem\adb_bExistsDataSourceResponse.c

$(_OBJ)\adb_getDataSources.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_getDataSources.c

$(_OBJ)\adb_getDataSourcesResponse.obj:		$(_SRC)\plugins\memory\SharedMem\adb_getDataSourcesResponse.c

$(_OBJ)\adb_getLanguages.obj: 			$(_SRC)\plugins\memory\SharedMem\adb_getLanguages.c

$(_OBJ)\adb_getLanguagesResponse.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_getLanguagesResponse.c

$(_OBJ)\adb_getLogfile.obj: 			$(_SRC)\plugins\memory\SharedMem\adb_getLogfile.c

$(_OBJ)\adb_getLogfileResponse.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_getLogfileResponse.c

$(_OBJ)\adb_getMonolingualObject.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_getMonolingualObject.c

$(_OBJ)\adb_getMonolingualObjectResponse.obj: 	$(_SRC)\plugins\memory\SharedMem\adb_getMonolingualObjectResponse.c

$(_OBJ)\adb_getMultilingualObject.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_getMultilingualObject.c

$(_OBJ)\adb_getMultilingualObjectResponse.obj: 	$(_SRC)\plugins\memory\SharedMem\adb_getMultilingualObjectResponse.c

$(_OBJ)\adb_setLogfile.obj:		 	$(_SRC)\plugins\memory\SharedMem\adb_setLogfile.c

$(_OBJ)\adb_setLogfileResponse.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_setLogfileResponse.c

$(_OBJ)\adb_shutdown.obj: 			$(_SRC)\plugins\memory\SharedMem\adb_shutdown.c

$(_OBJ)\adb_shutdownResponse.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_shutdownResponse.c

$(_OBJ)\adb_synchronize.obj: 			$(_SRC)\plugins\memory\SharedMem\adb_synchronize.c

$(_OBJ)\adb_synchronizeResponse.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_synchronizeResponse.c

$(_OBJ)\adb_translate.obj: 			$(_SRC)\plugins\memory\SharedMem\adb_translate.c

$(_OBJ)\adb_translateResponse.obj: 		$(_SRC)\plugins\memory\SharedMem\adb_translateResponse.c

$(_OBJ)\axis2_extension_mapper.obj: 		$(_SRC)\plugins\memory\SharedMem\axis2_extension_mapper.c

$(_OBJ)\axis2_stub_OpenTMSWebServiceImplementationService.obj: $(_SRC)\plugins\memory\SharedMem\axis2_stub_OpenTMSWebServiceImplementationService.c

$(_OBJ)\EqfSharedMemory.OBJ:		$(_SRC)\plugins\memory\SharedMem\EqfSharedMemory.CPP $(_SRC)\plugins\memory\SharedMem\EqfSharedMemory.H

$(_OBJ)\MemoryWebServiceClient.OBJ:	$(_SRC)\plugins\memory\SharedMem\MemoryWebServiceClient.CPP

$(_OBJ)\JSONFactory.OBJ:		$(_SRC)\plugins\memory\SharedMem\JSONFactory.CPP $(_SRC)\plugins\memory\SharedMem\JSONFactory.H

$(_OBJ)\FifoQueue.OBJ:		$(_SRC)\plugins\memory\SharedMem\FifoQueue.CPP $(_SRC)\plugins\memory\SharedMem\FifoQueue.H

$(_OBJEXE)\JSONFactory.OBJ:		$(_SRC)\plugins\memory\SharedMem\JSONFactory.CPP $(_SRC)\plugins\memory\SharedMem\JSONFactory.H
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\SharedMem\$(*B).cpp

$(_OBJEXE)\FifoQueue.OBJ:		$(_SRC)\plugins\memory\SharedMem\FifoQueue.CPP $(_SRC)\plugins\memory\SharedMem\FifoQueue.H
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\SharedMem\$(*B).cpp

$(_OBJ)\TMXFactory.OBJ:		$(_SRC)\plugins\memory\SharedMem\TMXFactory.CPP $(_SRC)\plugins\memory\SharedMem\TMXFactory.H

$(_OBJ)\EqfSharedMemoryPlugin.OBJ:  $(_SRC)\plugins\memory\SharedMem\EqfSharedMemoryPlugin.CPP $(_SRC)\plugins\memory\SharedMem\EqfSharedMemoryPlugin.H

$(_OBJ)\EqfSharedMemory.OBJ:  $(_SRC)\plugins\memory\SharedMem\EqfSharedMemory.CPP $(_SRC)\plugins\memory\SharedMem\EqfSharedMemory.H

$(_OBJEXE)\TransportThread.OBJ:  $(_SRC)\plugins\memory\SharedMem\TransportThread.CPP $(_SRC)\plugins\memory\SharedMem\TransportThread.H
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\SharedMem\$(*B).cpp

$(_OBJ)\LogWriter.OBJ:  $(_SRC)\core\utilities\LogWriter.CPP $(_SRC)\core\utilities\LogWriter.H

$(_OBJEXE)\LogWriter.OBJ:  $(_SRC)\core\utilities\LogWriter.CPP $(_SRC)\core\utilities\LogWriter.H
    @echo ---- Compiling $(_SRC)\core\utilities\$(*B).CPP
    @echo ---- Compiling $(_SRC)\core\utilities\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\core\utilities\$(*B).cpp

$(_DLL)\EqfSharedMemPlugin.DLL:		$(_OBJ)\EqfSharedMemoryPlugin.OBJ \
					$(_OBJ)\EqfSharedMemory.OBJ \
					$(_OBJ)\JSONFactory.OBJ \
					$(_OBJ)\FifoQueue.OBJ \
					$(_OBJ)\TMXFactory.OBJ \
					$(_LIB)\OtmAlloc.lib \
					$(_LIB)\OtmBase.lib \
					$(_LIB)\OtmDll.lib \
					$(_LIB)\PluginManager.lib \
					$(_LIB)\MemoryWebServiceClient.lib

#------------------------------------------------------------------------------
# Build TestClient.EXE
#------------------------------------------------------------------------------
$(_BIN)\TestClient.EXE: $(_OBJ)\TestClient.OBJ $(_OBJ)\adb_synchronize.obj $(_OBJ)\adb_synchronizeResponse.obj \
                        $(_OBJ)\axis2_stub_OpenTMSWebServiceImplementationService.obj \
			$(_OBJ)\adb_bExistsDataSource.obj 	$(_OBJ)\adb_bExistsDataSourceResponse.obj $(_OBJ)\adb_getDataSources.obj \
			$(_OBJ)\adb_getDataSourcesResponse.obj 	$(_OBJ)\adb_getLanguages.obj $(_OBJ)\adb_getLanguagesResponse.obj \
			$(_OBJ)\adb_getLogfile.obj $(_OBJ)\adb_getLogfileResponse.obj $(_OBJ)\adb_getMonolingualObject.obj \
			$(_OBJ)\adb_getMonolingualObjectResponse.obj $(_OBJ)\adb_getMultilingualObject.obj $(_OBJ)\adb_getMultilingualObjectResponse.obj \
			$(_OBJ)\adb_setLogfile.obj $(_OBJ)\adb_setLogfileResponse.obj $(_OBJ)\adb_shutdown.obj $(_OBJ)\adb_shutdownResponse.obj \
			$(_OBJ)\adb_translate.obj $(_OBJ)\adb_translateResponse.obj $(_OBJ)\axis2_extension_mapper.obj  
    @echo ---- Linking $(_BIN)\TestClient.EXE
    @echo ---- Linking $(_BIN)\TestClient.EXE >>$(_ERR)
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
$(_OBJ)\TestClient.OBJ
$(_OBJ)\adb_synchronize.obj
$(_OBJ)\adb_synchronizeResponse.obj
$(_OBJ)\axis2_stub_OpenTMSWebServiceImplementationService.obj
$(_OBJ)\adb_bExistsDataSource.obj 	
$(_OBJ)\adb_bExistsDataSourceResponse.obj 
$(_OBJ)\adb_getDataSources.obj 
$(_OBJ)\adb_getDataSourcesResponse.obj 	
$(_OBJ)\adb_getLanguages.obj 
$(_OBJ)\adb_getLanguagesResponse.obj 
$(_OBJ)\adb_getLogfile.obj 
$(_OBJ)\adb_getLogfileResponse.obj 
$(_OBJ)\adb_getMonolingualObject.obj
$(_OBJ)\adb_getMonolingualObjectResponse.obj 
$(_OBJ)\adb_getMultilingualObject.obj 
$(_OBJ)\adb_getMultilingualObjectResponse.obj
$(_OBJ)\adb_setLogfile.obj 
$(_OBJ)\adb_setLogfileResponse.obj 
$(_OBJ)\adb_shutdown.obj $(_OBJ)\adb_shutdownResponse.obj
$(_OBJ)\adb_translate.obj 
$(_OBJ)\adb_translateResponse.obj 
$(_OBJ)\axis2_extension_mapper.obj
$(_LINK_OPTIONS) 
/OUT:$(_BIN)\TestClient.EXE /pdb:"$(_BIN)\EQFD.pdb"
  $(_LINK_LIB_EXE) $(_LIB)\OtmAlloc.LIB $(AXIS2C_HOME)\lib\axutil.lib $(AXIS2C_HOME)\lib\axiom.lib $(AXIS2C_HOME)\lib\axis2_engine.lib
<<


#------------------------------------------------------------------------------
# Build EqfSharedMemPlugin.DLL and copy plugin DLL to release directory
#------------------------------------------------------------------------------
$(_DLL)\EqfSharedMemPlugin.DLL:
    @echo --- Linking $(_DLL)\EqfSharedMemPlugin.DLL
    @echo ---- Linking $(_DLL)\EqfSharedMemPlugin.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
	$(_OBJ)\EqfSharedMemoryPlugin.OBJ
	$(_OBJ)\EqfSharedMemory.OBJ 
	$(_OBJ)\JSONFactory.OBJ 
    $(_OBJ)\FifoQueue.OBJ
	$(_OBJ)\TMXFactory.OBJ 
/OUT:$(_DLL)\EqfSharedMemPlugin.DLL
/MAP:$(_MAP)\EqfSharedMemPlugin.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmAlloc.lib $(_LIB)\OtmBase.lib $(_LIB)\OtmDll.lib $(_LIB)\PluginManager.lib $(_LIBOTHER)\xerces-c_3.lib Shell32.lib $(_LIB)\MemoryWebServiceClient.lib
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
	$(_OBJ)\axis2_stub_OpenTMSWebServiceImplementationService.obj \
	$(_OBJ)\adb_bExistsDataSource.obj 	 \
	$(_OBJ)\adb_bExistsDataSourceResponse.obj  \
	$(_OBJ)\adb_getDataSources.obj  \
	$(_OBJ)\adb_getDataSourcesResponse.obj 	\
	$(_OBJ)\adb_getLanguages.obj \
	$(_OBJ)\adb_getLanguagesResponse.obj \
	$(_OBJ)\adb_getLogfile.obj \
	$(_OBJ)\adb_getLogfileResponse.obj \
	$(_OBJ)\adb_getMonolingualObject.obj \
	$(_OBJ)\adb_getMonolingualObjectResponse.obj \
	$(_OBJ)\adb_getMultilingualObject.obj \
	$(_OBJ)\adb_getMultilingualObjectResponse.obj \
	$(_OBJ)\adb_setLogfile.obj  \
	$(_OBJ)\adb_setLogfileResponse.obj \
	$(_OBJ)\adb_shutdown.obj $(_OBJ)\adb_shutdownResponse.obj \
	$(_OBJ)\adb_translate.obj \
	$(_OBJ)\adb_translateResponse.obj \
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
$(_LINK_LIB_CRT) Shell32.lib $(AXIS2C_HOME)\lib\axutil.lib $(AXIS2C_HOME)\lib\axiom.lib $(AXIS2C_HOME)\lib\axis2_engine.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\WIN\ md $(RELEASE_DIR)\OTM\WIN
    @copy $(_BIN)\MemoryWebServiceClient.DLL $(RELEASE_DIR)\OTM\WIN /Y>$(_ERR)
	
	
#------------------------------------------------------------------------------
# Build Dependencies for OtmMemReplicator.EXE                                 -
#------------------------------------------------------------------------------
$(_OBJEXE)\OtmMemReplicator.obj: 		$(_SRC)\plugins\memory\SharedMem\OtmMemReplicator.cpp
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP
    @echo ---- Compiling $(_SRC)\plugins\memory\SharedMem\$(*B).CPP >>$(_ERR)
    @$(_COMPILER)  $(_CL_CPP_OPTIONS_EXE) /Fo$(_OBJEXE)\ /Fd$(_BIN)\EQFSTART.PDB /Fe$(_BIN)\ $(_SRC)\plugins\memory\SharedMem\$(*B).cpp

OTMMEMREPOBJS= $(_OBJEXE)\OtmMemReplicator.OBJ \
	$(_OBJEXE)\TransportThread.OBJ \
	$(_OBJEXE)\LogWriter.OBJ \
	$(_OBJEXE)\JSONFactory.OBJ \
    $(_OBJEXE)\FifoQueue.OBJ 

$(_BIN)\OtmMemReplicator.EXE:	$(OTMMEMREPOBJS)

#------------------------------------------------------------------------------
# Build OtmMemReplicator.EXE                                                          -
#------------------------------------------------------------------------------
$(_BIN)\OtmMemReplicator.EXE: 
    @echo ---- Linking $(_BIN)\OtmMemReplicator.EXE
    @echo ---- Linking $(_BIN)\OtmMemReplicator.EXE >>$(_ERR)
    @echo ---- _LINK_OPTIONS_EXE=$(_LINK_OPTIONS_EXE)
    $(_LINKER) @<<lnk.rsp >>$(_ERR)
$(OTMMEMREPOBJS) 
$(_LINK_OPTIONS_EXE)
/OUT:$(_BIN)\OtmMemReplicator.EXE /pdb:"$(_BIN)\OtmMemReplicator.pdb"
    $(_LINK_LIB_EXE) $(_LIB)\OtmAlloc.LIB $(_LIB)\OTMBase.LIB  $(_LIB)\MemoryWebServiceClient.LIB Shell32.lib 
<<
    @if not exist $(RELEASE_DIR)\OTM\WIN\ md $(RELEASE_DIR)\OTM\WIN
    @copy $(_BIN)\OtmMemReplicator.EXE $(RELEASE_DIR)\OTM\WIN /Y>$(_ERR)
	