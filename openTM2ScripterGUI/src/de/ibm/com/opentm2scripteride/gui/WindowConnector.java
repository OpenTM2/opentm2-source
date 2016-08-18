/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui;

import java.awt.Color;
import java.awt.Font;
import java.io.File;

import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.WindowConstants;

import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.gui.custom.FileTree;
import de.ibm.com.opentm2scripteride.gui.custom.ReportWindow;
import de.ibm.com.opentm2scripteride.gui.custom.editor.CodeCompleter;
import de.ibm.com.opentm2scripteride.gui.custom.editor.EditorConfigure;
import de.ibm.com.opentm2scripteride.gui.custom.editor.SyntaxStyle;
import de.ibm.com.opentm2scripteride.gui.custom.editor.TabTextEditor;
import de.ibm.com.opentm2scripteride.utility.Configuration;
import de.ibm.com.opentm2scripteride.utility.Constants;
import de.ibm.com.opentm2scripteride.utility.ErrorHandler;
import de.ibm.com.opentm2scripteride.utility.OneAppInstance;
import de.ibm.com.opentm2scripteride.utility.OpenTM2Process;


/**
 * Responsible for the execution of all actions which will be performed after an event
 * in the main class occured
 */
public class WindowConnector {
	
	private static WindowConnector instance = null;
	private String notset = "~has not been set~";
	private OpenTM2Process process;
	
	private SyntaxStyle apicall, comment,controlstructs, functions,variables;
	private Color selCol, line;
	private Font generalFont;
	private boolean eolMarker;
	
	private WindowConnector(){
		
	}
	
	
	/**
	 * Creates the config dialog
	 */
	public void createConfigDialog(){
		try {
			MainApp mainApp = MainApp.getInstance();
			ConfigDialog dialog = new ConfigDialog(mainApp.getMainWindow(),
					mainApp.getAppName() + " - Configuration", true);
			dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
			
		}
	}
	
	/**
	 *  Creates the help dialog
	 */
	public void createHelpDialog(){
		try {
			MainApp mainApp = MainApp.getInstance();
			HelpDialog dialog = new HelpDialog(mainApp.getMainWindow(),
					mainApp.getAppName() + " - Help", true);
			dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	/**
	 *  Creates the about dialog
	 */
	public void createAboutDialog(){
		try {
			MainApp mainApp = MainApp.getInstance();
			AboutWindow dialog = new AboutWindow(mainApp.getMainWindow(),
					 "About - "+mainApp.getAppName(), true);
			dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
			dialog.setVisible(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	/**
	 *  Creates the JFileChooser dialog and opens a file via 
	 *  the openFile method of the MainApp class
	 */
	public void openFileDialog() {
		Configuration conf = Configuration.getInstance();
		
		String lastOpenDir = conf.getValue(Constants.lastOpenDir);
		if(lastOpenDir==null || lastOpenDir.isEmpty())
			lastOpenDir = conf.getValue(Constants.scriptDirectory);
		
		File scriptdir = new File(lastOpenDir);
		JFileChooser fc = new JFileChooser(scriptdir);
		if (fc.showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {
			// save it as history
			conf.setValue(Constants.lastOpenDir, fc.getCurrentDirectory().toString());
			
			MainApp.getInstance().openFile(fc.getSelectedFile());
		}
	}
	
	
	public void openLogWindow() {
		String histDir = Configuration.getInstance().getValue(Constants.lastOpenDir);
		if(histDir==null || histDir.isEmpty()) {
				histDir = System.getProperty("user.dir");
		}
			
		File scriptdir = new File(histDir);
		JFileChooser fc = new JFileChooser(scriptdir);
		if (fc.showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {
			Configuration.getInstance().setValue(Constants.lastOpenDir,fc.getCurrentDirectory().toString());
			ReportWindow.createReportWindow(fc.getSelectedFile().toString(),false);
		}
	}
	/**
	 *  Clears the Editor and calls proceedWithUnsavedFile method
	 *  if the editor has unsaved changes
	 */
	public void newScript(){
		MainApp.getInstance().newFile();
	}
	
	
	/**
	 * Generates the JFileChooser save dialog to select/create a File
	 * and saves the File via the saveFile method of the MainApp class
	 */
	public boolean saveFileAs() {
		Configuration conf = Configuration.getInstance();
		File scriptdir = new File(conf.getValue(Constants.scriptDirectory));
		JFileChooser saver = new JFileChooser(scriptdir);
		if (saver.showSaveDialog(null) != JFileChooser.APPROVE_OPTION) {
			return true;
		}
		File toSave = saver.getSelectedFile();
		if (toSave.exists()){
			//overwrite existing file?
		 	Object[] options = {"Yes", "No"}; 
		 	int auswahl = JOptionPane.showOptionDialog(null, toSave.getName()+" aready exist, Do you want to replace it?",
		 			"SaveAs", JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE,
		 			null, options, options[1]);
		 	if(auswahl== 1) {
		 			return true;
		 	}
		}
		
		boolean ret = MainApp.getInstance().saveFile(toSave);
		
		// update TabTitleName
		if(ret){
			MainApp.getInstance().getMainWindow().setSaveBtnState(true);
			MainApp.getInstance().getActiveEditor().setFile(toSave);
			// rename tab tile
			JTabbedPane fileTabPane  = MainApp.getInstance().getMainWindow().getEditorTabbedPane();
			TabTitlePanel tp = (TabTitlePanel)fileTabPane.getTabComponentAt(fileTabPane.getSelectedIndex());
			tp.setTitle(toSave.getName(),fileTabPane.getSelectedIndex());
			fileTabPane.setTabComponentAt(fileTabPane.getSelectedIndex(),tp);
			//
			MainApp.getInstance().updateWindowTitle();
		}
		
		//
		repaintTree();
		return ret;
	}
	
	/**
	 * close an opened file
	 * @param tabIndex
	 */
	public void closeFile(JTabbedPane tp, int tabIndex) {
		// check if need to save
		//tp.setSelectedIndex(tabIndex);
		boolean bRes = proceedWithUnsavedFile(tp,tabIndex);
		if(!bRes)
			return;
		
		// only one New file, can't close it
		if(tp.getTabCount()==1) {
			TabTextEditor editor = (TabTextEditor)tp.getComponentAt(0);
			if ( editor.getFile()==null && editor.getText().isEmpty() )
			    return;
		}
		   
		tp.remove(tabIndex);
	}
	
	/**
	 * close other tabs except specified by tabIndex
	 * @param tp
	 * @param tabIndex
	 */
	public void closeOthers(JTabbedPane tp, int tabIndex) {
		int totalTabs = tp.getTabCount();
		for(int i=totalTabs-1; i>=0; i--) {
			if(i!=tabIndex)
			    closeFile(tp,i);
		}
	}
	
	/**
	 * close all opened file tab
	 * @param tp
	 */
	public void closeAll(JTabbedPane tp) {
		int totalTabs = tp.getTabCount();
		for(int i=totalTabs-1; i>=0; i--) 
			closeFile(tp,i);
	}
	
	/**
	 *  Saves a script if a file of this script already exists
	 *  otherwise the method saveFileAs is called
	 */
	public boolean saveFile(){
		MainApp app = MainApp.getInstance();
		File toSave = app.getActiveEditor().getFile() ;
		if ( toSave == null ) {
			return saveFileAs();
		}
		
		boolean bRes = app.saveFile(toSave);
		if(bRes)
			app.updateWindowTitle();
		
		return bRes;
	}
	
	/****************** General Settings Methods*****************/
	
	/**
	 * Saves the general settings and writes it into the config file
	 * @param location location of the log file
	 * @param commandXML location of the commandXML file
	 * @param scriptsource location of the script directory
	 * @param exe location of the openTM2 Scripter.exe
	 */
	public void changeSettings(String location, String commandXML, String scriptsource, String exe){

		Configuration conf = Configuration.getInstance();
		if(!location.equalsIgnoreCase(notset)){
			conf.setValue(Constants.logFileLocation,location);
		}
		if(!commandXML.equalsIgnoreCase(notset)){
			conf.setValue(Constants.commandXMLLocation,commandXML);
		}
		if(!scriptsource.equalsIgnoreCase(notset)){
			conf.setValue(Constants.scriptDirectory,scriptsource);
			repaintTree();
		}
		if(!exe.equalsIgnoreCase(notset)){
			conf.setValue(Constants.openTM2scripterLocation,exe);
		}
	}
	
	
	/**
	 * Repaints two Filetrees of createTS and runTS
	 */
	public void repaintTree(){
		FileTree main = MainApp.getInstance().getMainWindow().getFileTree();	
		File newRoot = new File(Configuration.getInstance().getValue(Constants.scriptDirectory));
		main.getTreeModel().setnewRootDir(newRoot);
	}
	
	
	/**
	 * Opens a JFileChooser dialog and writes the path of the selected file/folder
	 * into the given textfield
	 * @param textfield file path is written in this textfield
	 * @param choosefolder true if only choose a folder false if only choose files
	 */
	public void getPath(JTextField textfield, boolean choosefolder){
		File tempfile;
		Configuration conf = Configuration.getInstance();
		File scriptdir = new File(conf.getValue(Constants.scriptDirectory));
		JFileChooser fc = new JFileChooser(scriptdir);
		if(choosefolder){
			fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		}
		if (JFileChooser.APPROVE_OPTION == fc.showOpenDialog(null)) {
			tempfile = fc.getSelectedFile();
			if (tempfile != null) {
				// Generates the relative Path from the selected file to the directory of this programm.
				// I delete the URL conversion, just use path instead of
//				URI progPath = new File(System.getProperty("user.dir").toString()).toURI();
//				URI temp = tempfile.toURI();
//				String relPath = temp.getPath();
//				if(!progPath.getPath().equalsIgnoreCase(relPath) &&
//					progPath.getPath().substring(0, 1).equalsIgnoreCase(relPath.substring(0,1))){	
//					relPath = progPath.relativize(temp).getPath();
//				}
//				else{
//					relPath = relPath.substring(1);
//				}
				textfield.setText(tempfile.getPath());
			}
		}
		
	}
	
	/**
	 * Returns saved parameters from the config file
	 * @param conf the name of the parameter which will be returned
	 * @return the value of the config parameter
	 */
	public String getConf(String conf){
		String confSetting = Configuration.getInstance().getValue(conf);

		if(confSetting != null){
			return confSetting; 
		}
		else{
			return notset;
		}
	}
	
	/******************Styling Methods******************/
	
	/**
	 * Saves the actual styling settings
	 */
	public void saveActualSettings(){
		EditorConfigure guicon = EditorConfigure.getInstance();
		generalFont = guicon.getFont();
		
		selCol = guicon.getSelectionColor();
		line = guicon.getLineHighlightColor();
		
		apicall = guicon.getStyleApiCalls();
		comment = guicon.getStyleComments();
		controlstructs = guicon.getStyleControlStructures();
		functions = guicon.getStyleFunctions();
		variables = guicon.getStyleVariable();
		
		eolMarker = guicon.getEOLChar();
	}
	
	/**
	 * Restores the actual styling settings
	 */
	public void restoreSettings(){
		EditorConfigure editorCfg = EditorConfigure.getInstance();
		editorCfg.setFont(generalFont);
		editorCfg.setSelectionColor(selCol);
		editorCfg.setLineHighlightColor(line);
		editorCfg.setStyleApiCalls(apicall.getColor(), apicall.isItalic(), apicall.isBold());
		editorCfg.setStyleComments(comment.getColor(), comment.isItalic(), comment.isBold());
		editorCfg.setStyleControlStructures(controlstructs.getColor(), controlstructs.isItalic(), controlstructs.isBold());
		editorCfg.setStyleFunctions(functions.getColor(), functions.isItalic(), functions.isBold());
		editorCfg.setStyleVariable(variables.getColor(), variables.isItalic(), variables.isBold());
		editorCfg.setEOLChar(eolMarker);
	}
	
	
	/**
	 * This method sets the general font of the editor
	 * @param fontname name of the font
	 * @param bold true if font is bold
	 * @param italic true if font is italic
	 * @param size the fontsize
	 */
	public void setGeneralFont(String fontname, boolean bold, boolean italic, int size){
		int stylechoice ;
		if(bold == false && italic == false){
			stylechoice = 0;
		}
		else if(bold == true && italic == false){
			stylechoice = 1;
		}
		else if(bold == false && italic == true){
			stylechoice = 2;
		}
		else {
			stylechoice = 3;
		}
		
		EditorConfigure.getInstance().setFont(new Font(fontname,stylechoice,size));
	}
	
	
	/**
	 * Sets the highlighing of a special highlighting type of the editor
	 * @param type describes the highlightingtype which shall be changed
	 * @param bold true if highlighting is bold
	 * @param italic true if highlighting is italic
	 */
	public void setHighlightingtype(String type, boolean bold, boolean italic){	
		Color color;
		if(type.equalsIgnoreCase(Constants.highlightingconstants[0])){
			color = EditorConfigure.getInstance().getStyleVariable().getColor();
			EditorConfigure.getInstance().setStyleVariable(color, italic, bold);
		}
		else if(type.equalsIgnoreCase(Constants.highlightingconstants[1])){
			color = EditorConfigure.getInstance().getStyleComments().getColor();
			EditorConfigure.getInstance().setStyleComments(color, italic, bold);
		}
		else if(type.equalsIgnoreCase(Constants.highlightingconstants[2])){
			color = EditorConfigure.getInstance().getStyleApiCalls().getColor();
			EditorConfigure.getInstance().setStyleApiCalls(color, italic, bold);
		}
		else if(type.equalsIgnoreCase(Constants.highlightingconstants[3])){
			color = EditorConfigure.getInstance().getStyleControlStructures().getColor();
			EditorConfigure.getInstance().setStyleControlStructures(color, italic, bold);
		}
		else {
			color = EditorConfigure.getInstance().getStyleFunctions().getColor();
			EditorConfigure.getInstance().setStyleFunctions(color, italic, bold);
		}
	}
	
	/**
	 * Sets the highlighing color of a special highlighting type of the editor
	 * @param type describes the highlightingtype which shall be changed
	 * @param color the new color of the highlightingtype
	 */
	public void setHighlightingColor(String type, Color color){		
		boolean italic, bold;
		if(type.equalsIgnoreCase(Constants.highlightingconstants[0])){
			bold = EditorConfigure.getInstance().getStyleVariable().isBold();
			italic = EditorConfigure.getInstance().getStyleVariable().isItalic();
			EditorConfigure.getInstance().setStyleVariable(color, italic, bold);
		}
		else if(type.equalsIgnoreCase(Constants.highlightingconstants[1])){
			bold = EditorConfigure.getInstance().getStyleComments().isBold();
			italic = EditorConfigure.getInstance().getStyleComments().isItalic();
			EditorConfigure.getInstance().setStyleComments(color, italic, bold);
		}
		else if(type.equalsIgnoreCase(Constants.highlightingconstants[2])){
			bold = EditorConfigure.getInstance().getStyleApiCalls().isBold();
			italic = EditorConfigure.getInstance().getStyleApiCalls().isItalic();
			EditorConfigure.getInstance().setStyleApiCalls(color, italic, bold);
		}
		else if(type.equalsIgnoreCase(Constants.highlightingconstants[3])){
			bold = EditorConfigure.getInstance().getStyleControlStructures().isBold();
			italic = EditorConfigure.getInstance().getStyleControlStructures().isItalic();
			EditorConfigure.getInstance().setStyleControlStructures(color, italic, bold);
		}
		else if (type.equalsIgnoreCase(Constants.highlightingconstants[4])){
			bold = EditorConfigure.getInstance().getStyleFunctions().isBold();
			italic = EditorConfigure.getInstance().getStyleFunctions().isItalic();
			EditorConfigure.getInstance().setStyleFunctions(color, italic, bold);
		}
		else if (type.equalsIgnoreCase(Constants.highlightingconstants[5])){
			EditorConfigure.getInstance().setSelectionColor(color);
		}
		else{
			EditorConfigure.getInstance().setLineHighlightColor(color);
		}
		
	}
	
	/****************** Code Completion Methods*****************/
	
	/**
	 * Searches for a command or apicall with the String s in it
	 * @param s search String
	 */
	public void setSearchTokens(String s){
		CodeCompleter.getInstance().setSearch(s);
	}
	
	/****************** Run TS Methods*****************/
	
	/**
	 * Selects the script file which shall be executed in the runTS Panel
	 * @param runTS the runTS Panel
	 */
	public void selectFile(RunConsolePanel runTS){
		File scriptdir = new File(Configuration.getInstance().getValue(Constants.scriptDirectory));
		JFileChooser fc = new JFileChooser(scriptdir);
		fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
	
		if (fc.showOpenDialog(null) == 0) {
			runTS.setSelFile(fc.getSelectedFile());
		} else {
			runTS.setSelFile(null);
		}
	}
	
	/**
	 * Runs the selected script
	 * @param runTS the runTS Panel
	 */
	public void runTS(RunConsolePanel runTS, JTabbedPane tp, int index ){
		if(!proceedWithUnsavedFile(tp, index))
			return;
		
		File scripterExe;
		if(Configuration.getInstance().getValue(Constants.openTM2scripterLocation) == null){
			ErrorHandler.getInstance().handleError(ErrorHandler.ERROR, "Before you run a script you have to specify the openTM2Scripter.exe Location in the Configurations Dialog");
			createConfigDialog();
			return;
		}
		else{
			scripterExe = new File(Configuration.getInstance().getValue(Constants.openTM2scripterLocation));
			if(!scripterExe.exists()){
				ErrorHandler.getInstance().handleError(ErrorHandler.ERROR , "The specified openTM2Scripter.exe does not exist. Please update the Location in the Configurations Dialog");
				return;
			}
		}
		JTextArea textArea = runTS.getTextArea();
		File selectedFile = runTS.getSelFile();
		//boolean fileIsSelected = runTS.getFileIsSelected();
		boolean bolShowHelp = runTS.getBolShowHelp();
		boolean bolDelTmp = runTS.getBolDelTmp();
		boolean bolDate = runTS.getBolDate();
		boolean bolID = runTS.getBolID();
		String constants = runTS.getConstants();
		// check if a file was selected and if the selectedFile is not
		// null
		if (selectedFile != null && selectedFile.exists()) {
			    //if truly run, disable the report button
			    this.disableReport();
			    WindowConnector.getInstance().enableRunBtn(false);
			    WindowConnector.getInstance().enableStopBtn(true);
			    
				// starts Process with selected File if selected File is
				// not XML
				textArea.setText("");
				textArea.append("Start Time : " + new java.util.Date() + "\n");
				process = new OpenTM2Process(selectedFile.getAbsolutePath().toString(), runTS ,bolShowHelp, bolDelTmp, bolDate, bolID, runTS.getLogLevel(), runTS.getLogFile(), constants);
				process.start();
				
				//fileIsSelected = false;
		} else {
			ErrorHandler.getInstance().handleError(ErrorHandler.ERROR , "No File selected");
		}
	}
	
	/**
	 * Stops the process which runs the script
	 */
	public void stopProcess(){
		if(process != null) {
		    process.stopProcess();
		    process = null;
		    enableRunBtn(true);
		}
	}
	
	/**
	 * Sets the Options for running the script to the runTS Panel
	 * @param runTS the runTS Panel
	 */
	public void runTSOptions(RunConsolePanel runTS){
		JFrame frame = new JFrame("Scripter Options");
		frame.setSize(300, 300);
		ScripterOptions scriptOpt = new ScripterOptions(runTS);
		frame.getContentPane().add(scriptOpt);
		frame.setVisible(true);  
	}
	
	/**
	 * 
	 */
	public void generateReport() {
		ReportWindow rw = new ReportWindow();
		String cons = MainApp.getInstance().getMainWindow().getRTSPanel().getTextArea().getText();
		if(!cons.isEmpty())
		    rw.generateReport(cons);
       
	}
	/**
	 * Enables the report button
	 */
	public void enableReport(){
		MainApp.getInstance().getMainWindow().getGenerateReportBtn().setEnabled(true);
	}
	
	public void disableReport() {
		MainApp.getInstance().getMainWindow().getGenerateReportBtn().setEnabled(false);
	}

	public void enableRunBtn(boolean bEnabled) {
		MainApp.getInstance().getMainWindow().getRunBtn().setEnabled(bEnabled);
	}
	
	public void enableStopBtn(boolean bEnabled) {
		MainApp.getInstance().getMainWindow().getStopBtn().setEnabled(bEnabled);
	}
	/****************** Close Operation*****************/

	/**
	 * Checks if the opened file has unsaved changes and asks the user to change them if so
	 * @return True if the user saved oder decided against, false if the user wants to abort the action
	 */
	private boolean proceedWithUnsavedFile(JTabbedPane tp, int tabIndex) {
		int decision = 1;
		TabTextEditor editor = (TabTextEditor)tp.getComponentAt(tabIndex);
		boolean newFile = (editor.getFile() == null && !editor.getText().isEmpty());
		if ( newFile || !editor.isDocSaved()){
			tp.setSelectedIndex(tabIndex);
			Object[] options = {"Yes", "No", "Cancel"}; 
	 		decision = JOptionPane.showOptionDialog(null, 
	 				                    "Save file "+editor.getFileName()+"?",
	 				                    "Save",
	 				                    JOptionPane.YES_NO_CANCEL_OPTION,
	 									JOptionPane.QUESTION_MESSAGE, null, options, options[2]);
		}
		boolean ret = true;
		if(decision == 0){
			if (newFile) {
				ret = saveFileAs();
			} else {
				ret = saveFile();
			}
		} else if (decision == 2) {
			return false;
		}
		return ret;
	}
	
	/**
	 * ask user if want to save all the unsaved files
	 * @return
	 */
	public boolean proceedWithUnsavedFiles() {
		boolean bRes = true;
		//
		JTabbedPane tp = MainApp.getInstance().getMainWindow().getEditorTabbedPane();
		for(int i=0; i<tp.getTabCount(); i++) {
			TabTextEditor editor = (TabTextEditor)tp.getComponentAt(i);
			if(!editor.isDocSaved() && !editor.getText().isEmpty()) {
				tp.setSelectedIndex(i);
				bRes = proceedWithUnsavedFile(tp,i);
				if(!bRes)
					break;
			}
		}

		return bRes;
	}
	
	/**
	 * get all opened file names , and separate them with "," in string
	 * @return
	 */
	private String getAllOpenedFileNames() {
		StringBuilder sb = new StringBuilder();
		JTabbedPane tp = MainApp.getInstance().getMainWindow().getEditorTabbedPane();
		
		for(int i=0; i<tp.getTabCount(); i++){
			TabTextEditor tempEditor= (TabTextEditor)tp.getComponentAt(i);
			if(tempEditor.getFile() == null)
				continue;
			
		    sb.append(tempEditor.getFile().getAbsolutePath());
		    if(i!=tp.getTabCount()-1)
			    sb.append(",");
		}
		return sb.toString();
	}
	
	/**
	 * Closes the program an checks if an unsaved file is opened 
	 */
	public void closeOperation() {
		if (proceedWithUnsavedFiles()) {
			saveFrameSettings();
			try{
				
				Configuration.getInstance().setValue(Constants.openedFile,getAllOpenedFileNames() );
				MainApp.getInstance().copyConfig("configuration.conf",MainApp.getInstance().getOpenTM2Properpty()+"configuration.conf.data");
			}
			catch(Exception e){	
				// No opened script to save
			} finally{
				OneAppInstance.unlock();
			}
			System.exit(0);
		}
	}
	
	/**
	 * Saves the actual configuration of the frame, so the user can open the program
	 * like he had closed it
	 */
	public void saveFrameSettings(){
		MainApp mainApp = MainApp.getInstance();
		MainWindow mw = mainApp.getMainWindow();
		Configuration conf = Configuration.getInstance();
		int confCount = Constants.windowSettingsCount;
		String settings[] = new String[confCount];
		settings[0] = Double.toString(mw.getWidth());
		settings[1] = Double.toString(mw.getHeight());
		settings[2] = Boolean.toString(mw.getCreateTSLeftPanel().isShowing());
		settings[3] = Boolean.toString(mw.getCreateFileTree().isShowing());		
		settings[4] = Integer.toString(mw.getCreateTSLeftPanel().getDividerLocation());
		settings[5] = Integer.toString(mw.getCreateTSRightPanel().getDividerLocation());
//		settings[6] = Integer.toString(mw.getRTSPanel().getSplitPane().getDividerLocation());
		
		for (int i = 0; i < confCount; i++) {
			conf.setValue(Constants.windowSettings[i], settings[i]);	
		}
	}
	
	
	/**
	 * Gets the configuration of the frame from the config file
	 * @return frame configuration
	 */
	public String[] getWindowSettings(){
		Configuration conf = Configuration.getInstance();
		int confLen = Constants.windowSettingsCount;
		String[] configuration = new String[confLen];
		for (int i = 0; i < confLen; i++) {
			configuration[i] = conf.getValue(Constants.windowSettings[i]);
		}
		return configuration;
	}
	
	
	
	public static WindowConnector getInstance(){
		if(instance == null){
			instance = new WindowConnector();
		}
		return instance;
	}

}
