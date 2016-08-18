# Copyright (c) 2012-2016, International Business Machines
# Corporation and others.  All rights reserved.
build: $(RELEASE_DIR)\OtmTMService.jar

$(RELEASE_DIR)\OtmTMService.jar:
    echo Compiling all java files...
	if not exist $(OTMTMSERVICE_DIR)\bin md $(OTMTMSERVICE_DIR)\bin
    javac -cp $(OTMTMSERVICE_DIR)\lib\*;$(OTMTMSERVICE_DIR)\configure\*; -d $(OTMTMSERVICE_DIR)\bin  -Xlint:deprecation @$(OTMTMSERVICE_DIR)\AllJavaFiles.txt
	echo Compiling complete...
    if exist $(RELEASE_DIR)\OtmTMService.jar del $(RELEASE_DIR)\OtmTMService.jar
    echo Copying files...
    xcopy $(OTMTMSERVICE_DIR)\configure   $(OTMTMSERVICE_DIR)\bin\configure /S /I /Y
	xcopy $(OTMTMSERVICE_DIR)\lib        $(OTMTMSERVICE_DIR)\bin\lib /S /I /Y
	xcopy $(OTMTMSERVICE_DIR)\MANIFEST    $(OTMTMSERVICE_DIR)\bin\MANIFEST /S /I /Y
    echo Creating jars....
    jar cfm $(OTMTMSERVICE_DIR)\OtmTMService.jar  $(OTMTMSERVICE_DIR)\MANIFEST\MANIFEST.MF  -C $(OTMTMSERVICE_DIR)/bin/ .
	echo OtmTMService.jar created successfully...
	
	


