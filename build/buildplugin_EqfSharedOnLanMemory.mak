# +-------------------------------------------------------------------------------------+
# |  BUILDPLUGIN_EqfSharedOnLanMemory.MAK - Makefile for EqwfSharedOnLanMemoryPlugin    |
# +-------------------------------------------------------------------------------------+
# |  Copyright Notice:                                                                  |
# |                                                                                     |
# |      Copyright (C) 1990-2015 International Business Machines                        |
# |      Corporation and others. All rights reserved                                    |
# +-------------------------------------------------------------------------------------+

build:
   @del  /q $(RELEASE_DIR)\OpenTM2-EqfSharedOnLanMemory-*Setup.exe 
   @zip -j $(RELEASE_DIR)\OpenTM2-EqfSharedOnLanMemory-Setup.zip $(RELEASE_DIR)\otm\plugins\EqfSharedOnLanMemoryPlugin.DLL

