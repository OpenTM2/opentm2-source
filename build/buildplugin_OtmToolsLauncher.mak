# +----------------------------------------------------------------------------+
# |  BUILDPLUGIN_OtmToolsLauncher.MAK - Makefile for OtmToolsLauncherPlugin    |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2015 International Business Machines               |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+

build:
   @del  /q $(RELEASE_DIR)\OpenTM2-OtmToolsLauncher-*Setup.exe 
   @zip -j $(RELEASE_DIR)\OpenTM2-OtmToolsLauncher-Setup.zip $(RELEASE_DIR)\otm\plugins\OtmToolsLauncherPlugin.DLL

