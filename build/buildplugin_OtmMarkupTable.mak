# +----------------------------------------------------------------------------+
# |  BUILDPLUGIN_OtmMarkupTable.MAK - Makefile for OtmMarkupTablePlugin        |
# +----------------------------------------------------------------------------+
# |  Copyright Notice:                                                         |
# |                                                                            |
# |      Copyright (C) 1990-2015 International Business Machines               |
# |      Corporation and others. All rights reserved                           |
# +----------------------------------------------------------------------------+

build:
   @copy $(_BLD)\OtmMarkupTablePlugin.nsi $(RELEASE_DIR)\otm\OtmMarkupTablePlugin.nsi
   @copy $(_BLD)\OtmMarkupTablePlugin_inc.nsi $(RELEASE_DIR)\otm\OtmMarkupTablePlugin_inc.nsi
   
   @$(_NSISMAKE_EXE) /V1 $(RELEASE_DIR)\otm\OtmMarkupTablePlugin.nsi

   @del $(RELEASE_DIR)\otm\OtmMarkupTablePlugin.nsi
   @del $(RELEASE_DIR)\otm\OtmMarkupTablePlugin_inc.nsi
   @del  /q $(RELEASE_DIR)\OpenTM2-OtmMarkups-*Setup.exe 
   @move $(RELEASE_DIR)\otm\OpenTM2-OtmMarkups-*Setup.exe $(RELEASE_DIR)
   @zip -j $(RELEASE_DIR)\OpenTM2-OtmMarkups-Setup.zip $(RELEASE_DIR)\OpenTM2-OtmMarkups-*Setup.exe

