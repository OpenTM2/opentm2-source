/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.utility;

public class Constants {
	
	final public static String logFileLocation = "LOGFILE";
	final public static String commandXMLLocation = "COMMANDS_XML";
	final public static String scriptDirectory = "SCRIPTDIR";
	final public static String openTM2scripterLocation= "OPENTM2SCRIPTER";
	final public static String openedFile = "OPENED_FILE";
	final public static String scriptTemplate = "default_temp_script.script";
	final public static String genFont ="general_Font";
	final public static String lastOpenDir="LASTOPENDIR";
	
	final public static String[] highlightingconstants = {
		"Variables",
		"Comments",
		"Apicalls",
		"Structures",
		"Functions",
		"Selection",
		"Linehighlighting",
		"EOL_Character"
	};
	
	final public static String[] windowSettings = {
		"MainWindowWidth",
		"MainWindowHeight",
		"SelMainTab",
		"CreateTSTab",
		"CreateTSTabDivLoc",
		"CreateTSEditorDivLoc"//,
		//"runTSTreeDivLoc"
	};
	final public static int windowSettingsCount = windowSettings.length;	
}