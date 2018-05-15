# Copyright (c) 2017, International Business Machines
# Corporation and others.  All rights reserved.
build: $(RELEASE_DIR)\TMO.jar

source=$(_DRIVE)\$(_DEVDIR)\tmo\src\transmemoptimizer

$(RELEASE_DIR)\TMO.jar:
    echo Compiling all java files...
	if not exist $(source)\bin md $(source)\bin
    javac -cp $(source)\bin\*; -d $(source)\bin  -Xlint:deprecation @$(source)\TmoJavaFiles.txt
	echo Compiling complete...
    if exist $(RELEASE_DIR)\TMO.jar del $(RELEASE_DIR)\TMO.jar
    echo Creating jar....
    jar cfm $(source)\TMO.jar  $(source)\MANIFEST\MANIFEST.MF  -C $(source)/bin/ .
	echo TMO.jar created successfully...
	
	


