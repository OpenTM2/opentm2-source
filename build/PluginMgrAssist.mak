# ------------------------------------------------------------------------------
# PluginManager.MAK    - Makefile for the PluginManager
# Copyright (c) 2015, International Business Machines
# Corporation and others.  All rights reserved.
# ------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------
# eqfrules from BUILDER
#------------------------------------------------------------------------------

!INCLUDE $(_BLD)\EQFRULES.MAK

#------------------------------------------------------------------------------
# target list
#------------------------------------------------------------------------------

build:	$(_DLL)\PluginMgrAssist.DLL

#------------------------------------------------------------------------------
# include dependency list for eqf*.c programs
#------------------------------------------------------------------------------

!IF "$(MAKEDEP)" == "Y"
!INCLUDE $(DEPFILE)
!ENDIF

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

$(_OBJ)\OtmPlgMgGUIComm.obj:	$(_SRC)\core\PluginManager\OtmPlgMgGUIComm.cpp \
                                $(_SRC)\core\PluginManager\OtmPlgMgGUIComm.h \
								$(_SRC)\core\PluginManager\OtmPlgMgGUIStr.h

$(_OBJ)\OtmComm.obj:	        $(_SRC)\core\PluginManager\OtmComm.cpp \
                                $(_SRC)\core\PluginManager\OtmComm.h

$(_OBJ)\OtmHttp.obj:	        $(_SRC)\core\PluginManager\OtmHttp.cpp \
								$(_SRC)\core\PluginManager\OtmHttp.h

$(_OBJ)\OtmHttps.obj:	        $(_SRC)\core\PluginManager\OtmHttps.cpp \
								$(_SRC)\core\PluginManager\OtmHttps.h

$(_OBJ)\OtmSftp.obj:	        $(_SRC)\core\PluginManager\OtmSftp.cpp \
								$(_SRC)\core\PluginManager\OtmSftp.h


$(_OBJ)\OtmSftpConfig.obj:      $(_SRC)\core\PluginManager\OtmSftpConfig.cpp \
								$(_SRC)\core\PluginManager\OtmSftpConfig.h

$(_OBJ)\PlgMgXmlParser.obj:	    $(_SRC)\core\PluginManager\PlgMgXmlParser.cpp \
								$(_SRC)\core\PluginManager\PlgMgXmlParser.h

$(_OBJ)\HistoryWriter.obj:	    $(_SRC)\core\PluginManager\HistoryWriter.cpp \
								$(_SRC)\core\PluginManager\HistoryWriter.h

$(_OBJ)\TimeManager.obj:	    $(_SRC)\core\PluginManager\TimeManager.cpp \
								$(_SRC)\core\PluginManager\TimeManager.h

$(_DLL)\PluginMgrAssist.DLL:	$(_OBJ)\OtmPlgMgGUIComm.obj \
                                $(_OBJ)\OtmComm.obj \
							    $(_OBJ)\OtmHttp.obj \
							    $(_OBJ)\OtmHttps.obj \
							    $(_OBJ)\OtmSftp.obj \
							    $(_OBJ)\OtmSftpConfig.obj \
							    $(_OBJ)\PlgMgXmlParser.obj \
								$(_OBJ)\HistoryWriter.obj \
								$(_OBJ)\TimeManager.obj

#------------------------------------------------------------------------------
# Build PluginMgrAssist.dll
#------------------------------------------------------------------------------

$(_DLL)\PluginMgrAssist.DLL:
    @echo ---- Linking $(_DLL)\PluginMgrAssist.DLL
    @echo ---- Linking $(_DLL)\PluginMgrAssist.DLL >>$(_ERR)
    @$(_LINKER) @<<lnk.rsp>>$(_ERR)
			$(_OBJ)\OtmPlgMgGUIComm.OBJ
			$(_OBJ)\OtmComm.obj
			$(_OBJ)\OtmHttp.obj
			$(_OBJ)\OtmHttps.obj
			$(_OBJ)\OtmSftp.obj
			$(_OBJ)\OtmSftpConfig.obj
			$(_OBJ)\PlgMgXmlParser.obj
			$(_OBJ)\HistoryWriter.obj
			$(_OBJ)\TimeManager.obj

/OUT:$(_DLL)\PluginMgrAssist.DLL
/MAP:$(_MAP)\PluginMgrAssist.MAP $(_LINK_OPTIONS) /DLL /MAPINFO:EXPORTS
$(_LINK_LIB_CRT) $(_LIB)\OtmBase.lib $(_LIB)\PluginManager.lib $(_LIBOTHER)\xerces-c_3.lib $(_LIBOTHER)\zlibwapi.lib comctl32.lib shell32.lib
<<
    @if not exist $(RELEASE_DIR)\OTM\PLUGINS\ md $(RELEASE_DIR)\OTM\PLUGINS
    @copy /y $(_SRC)\core\PluginManager\PluginManager.conf.sample $(RELEASE_DIR)\OTM\PLUGINS /Y>$(_ERR)
