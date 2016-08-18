# Copyright (c) 2012-2015, International Business Machines
# Corporation and others.  All rights reserved.
build: $(RELEASE_DIR)\OpenTM2ScripterGUI.jar

$(RELEASE_DIR)\OpenTM2ScripterGUI.jar:
    echo Compiling all java files...
	if not exist $(SCRIPTERGUI_DIR)\bin md $(SCRIPTERGUI_DIR)\bin
    javac -cp $(SCRIPTERGUI_DIR)\libs\*;$(SCRIPTERGUI_DIR)\resources\*; -d $(SCRIPTERGUI_DIR)\bin  -Xlint:deprecation @$(SCRIPTERGUI_DIR)\AllJavaFiles.txt
	echo Compiling complete...
    if exist $(RELEASE_DIR)\OpenTM2ScripterGUI.jar del $(RELEASE_DIR)\OpenTM2ScripterGUI.jar
    echo Copying files...
    xcopy $(SCRIPTERGUI_DIR)\resources   $(SCRIPTERGUI_DIR)\bin\resources /S /I /Y
	xcopy $(SCRIPTERGUI_DIR)\libs        $(SCRIPTERGUI_DIR)\bin\libs /S /I /Y
	xcopy $(SCRIPTERGUI_DIR)\MANIFEST    $(SCRIPTERGUI_DIR)\bin\MANIFEST /S /I /Y
    echo Creating jars....
    jar cfm $(SCRIPTERGUI_DIR)\OpenTM2ScripterGUI.jar  $(SCRIPTERGUI_DIR)\MANIFEST\MANIFEST.MF  -C $(SCRIPTERGUI_DIR)/bin/ .
	echo Automatically build commands.xml...
	python  $(SCRIPTERGUI_DIR)\resources\parseApiCalls.py  $(SCRIPTERGUI_DIR)\resources\commands.xml $(_INC)\OtmFUNC.H
	echo OpenTM2ScripterGUI.jar created successfully...
	
	


