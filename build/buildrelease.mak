# +----------------------------------------------------------------------------+
# |  BUILDRELEASE.MAK - Makefile for OpenTM2 releases                          |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2016 International Business Machines              |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+

# creates the binary distribution using the binaries created by the other make files
# plus the sample files and the table files

build:
    @if not exist $(RELEASE_DIR) md $(RELEASE_DIR)
    @if exist $(RELEASE_DIR)\OpenTM2.ZIP erase $(RELEASE_DIR)\OpenTM2.ZIP
    @if not exist $(RELEASE_DIR)\OTM md $(RELEASE_DIR)\OTM
    @echo ---- copying sample data from $(SAMPLEDATADIR) ----  
    @echo ---- copying sample data from $(SAMPLEDATADIR) ---- >$(_ERR)
    @xcopy $(SAMPLEDATADIR)\*.* $(RELEASE_DIR)\OTM /s /E /Y >>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\TABLE md $(RELEASE_DIR)\OTM\TABLE
    @echo ---- copying table files from $(EQFTABLES) ---- 
    @echo ---- copying table files from $(EQFTABLES) ---- >>$(_ERR)
    @copy $(EQFTABLES)\*.* $(RELEASE_DIR)\OTM\TABLE /Y >>$(_ERR)
    @del $(RELEASE_DIR)\OTM\TABLE\EQFADQUO.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\EQFAMRI.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\EQFANSI.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\EQFAQUOT.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\EQFASCII.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\EQFDQUOT.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\eqfhtml.chr
    @del $(RELEASE_DIR)\OTM\TABLE\eqfhtml.tbl
    @del $(RELEASE_DIR)\OTM\TABLE\EQFMRI.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\EQFRTF.CHR
    @del $(RELEASE_DIR)\OTM\TABLE\EQFRTF.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\EQFUDQUO.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\EQFUQUOT.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\EQFUTF8.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\IBMXUHTM.TBL
    @del $(RELEASE_DIR)\OTM\TABLE\UNICODE.TBL
    @echo ---- copying EXEs+DLLs from $(_DLL) ---- 
    @echo ---- copying EXEs+DLLs from $(_DLL) ---- >>$(_ERR)
    @copy $(_DLL)\*.DLL $(RELEASE_DIR)\OTM\WIN /Y >>$(_ERR)
    @copy $(_BIN)\*.EXE $(RELEASE_DIR)\OTM\WIN /Y >>$(_ERR)
    @echo ---- copying OpenTM2 Scripter IDE ---- 
    @echo ---- copying OpenTM2 Scripter IDE ---- >>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI md $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI
    @copy $(_DRIVE)\$(_DEVDIR)\OpenTM2ScripterGUI\OpenTM2ScripterGUI.jar $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI /Y >>$(_ERR)
    @copy $(_DRIVE)\$(_DEVDIR)\OpenTM2ScripterGUI\configuration.conf $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI /Y >>$(_ERR)
    @echo ---- copying libs ---- 
    @if not exist $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\libs md $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\libs
    @copy $(_DRIVE)\$(_DEVDIR)\OpenTM2ScripterGUI\libs\*.jar $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\libs /Y >>$(_ERR)
    @echo ---- copying manifest---- 
    @if not exist $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\MANIFEST md $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\MANIFEST
    @copy $(_DRIVE)\$(_DEVDIR)\OpenTM2ScripterGUI\MANIFEST\*.mf $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\MANIFEST /Y >>$(_ERR)
    @echo ---- copying resources---- 
    @if not exist $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\resources md $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\resources
    @copy $(_DRIVE)\$(_DEVDIR)\OpenTM2ScripterGUI\resources\* $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\resources /Y >>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\resources\icons md $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\resources\icons
    @copy $(_DRIVE)\$(_DEVDIR)\OpenTM2ScripterGUI\resources\icons\*.png $(RELEASE_DIR)\OTM\OpenTM2ScripterGUI\resources\icons /Y >>$(_ERR)
    @echo ---- copying OtmTMService ---- 
    @echo ---- OtmTMService ---- >>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\OtmTMService md $(RELEASE_DIR)\OTM\OtmTMService
    @copy $(_DRIVE)\$(_DEVDIR)\OtmTMService\OtmTMService.jar $(RELEASE_DIR)\OTM\OtmTMService /Y >>$(_ERR)
    @echo ---- copying libs ---- 
    @if not exist $(RELEASE_DIR)\OTM\OtmTMService\lib md $(RELEASE_DIR)\OTM\OtmTMService\lib
    @copy $(_DRIVE)\$(_DEVDIR)\OtmTMService\lib\*.jar $(RELEASE_DIR)\OTM\OtmTMService\lib /Y >>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\OtmTMService\configure md $(RELEASE_DIR)\OTM\OtmTMService\configure
    @copy $(_DRIVE)\$(_DEVDIR)\OtmTMService\configure\* $(RELEASE_DIR)\OTM\OtmTMService\configure /Y >>$(_ERR)
    @echo ---- copying manifest---- 
    @if not exist $(RELEASE_DIR)\OTM\OtmTMService\MANIFEST md $(RELEASE_DIR)\OTM\OtmTMService\MANIFEST
    @copy $(_DRIVE)\$(_DEVDIR)\OtmTMService\MANIFEST\*.mf $(RELEASE_DIR)\OTM\OtmTMService\MANIFEST /Y >>$(_ERR)
    @echo ---- copying binaries from $(PACKAGESBIN) ---- 
    @echo ---- copying binaries from $(PACKAGESBIN) ---- >>$(_ERR)
    @copy $(PACKAGESBIN)\*.* $(RELEASE_DIR)\OTM\WIN /Y >>$(_ERR)
    @echo ---- copying OpenTMSHelper from $(_DRIVE)\$(_DEVDIR)\OpenTMSHelper ---- 
    @echo ---- copying OpenTMSHelper from $(_DRIVE)\$(_DEVDIR)\OpenTMSHelper ----  >>$(_ERR)
    @copy $(_DRIVE)\$(_DEVDIR)\OpenTMSHelper\OpenTMSHelper.jar $(RELEASE_DIR)\OTM\WIN /Y >>$(_ERR)
    @echo ---- copying MSG file ---- 
    @echo ---- copying MSG file ---- >>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\MSG md $(RELEASE_DIR)\OTM\MSG
    @echo ---- copying compiled help file ---- 
    @echo ---- copying compiled help file ---- >>$(_ERR)
    @if not exist $(RELEASE_DIR)\OTM\DOC md $(RELEASE_DIR)\OTM\DOC
	@copy $(_DOC)\PluginManager\PluginManager.chm $(RELEASE_DIR)\OTM\DOC /Y >>$(_ERR)
	@copy $(_DOC)\Opentm2TechnicalReference.pdf $(RELEASE_DIR)\OTM\DOC /Y >>$(_ERR)
	@copy $(_DOC)\Opentm2TranslatorsReference.pdf $(RELEASE_DIR)\OTM\DOC /Y >>$(_ERR)
	@copy $(_DOC)\OpenTM2APICalls.pdf $(RELEASE_DIR)\OTM\DOC /Y >>$(_ERR)


    @echo ---- Creating install package ---- 
    @echo ---- Creating install package ---- >>$(_ERR)
    @copy $(_BLD)\OpenTM2.nsi $(RELEASE_DIR)\OTM /Y >>$(_ERR)
    @copy $(_BLD)\License.txt $(RELEASE_DIR)\OTM /Y >>$(_ERR)
    @$(_NSISMAKE_EXE) /V1 $(RELEASE_DIR)\OTM\OpenTM2.nsi
    @move $(RELEASE_DIR)\OTM\OpenTM2*.EXE $(RELEASE_DIR)>>$(_ERR)

