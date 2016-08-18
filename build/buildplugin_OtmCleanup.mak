# +----------------------------------------------------------------------------+
# |  BUILDPLUGIN_OtmCleanup.MAK - Makefile for OtmCleanupPlugin                |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2015 International Business Machines               |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+

build:
   @del  /q $(RELEASE_DIR)\OpenTM2-OtmCleanup-*Setup.exe 
   @zip -j $(RELEASE_DIR)\OpenTM2-OtmCleanup-Setup.zip $(RELEASE_DIR)\otm\plugins\OtmCleanupPlugin.DLL

