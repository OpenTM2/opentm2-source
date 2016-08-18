/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui;

import java.awt.BorderLayout;
import java.io.File;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.gui.custom.FileTree;
import javax.swing.JTextField;

public class RunConsolePanel extends JPanel {

	private static final long serialVersionUID = 1L;
	
	private JTextArea textArea;
	private JPanel runTS_splitPane;
	
	private Boolean bolShowHelp;
	private Boolean bolDelTmp;
	private Boolean bolDate;
	private Boolean bolID;
	private String  logLevel;
	private String  logFile;
	private String  constants;
	
	private Boolean fileIsSelected = false;
	private File selectedFile;

	private FileTree mFileTree;
	private JTextField textField;
	
	public RunConsolePanel() {
		bolShowHelp =false;
		bolDelTmp = true;
		bolDate = false;
		bolID = false;
		setLogLevel(null);
		setLogFile(null);
		constants ="";
		
		setLayout(new BorderLayout(0, 0));
		
		textArea = new JTextArea();
		//textArea.setBackground(Color.BLACK);
		//textArea.setForeground(Color.WHITE);
		textArea.setEditable(false);
		//textArea.setFont(new Font("Verdana", Font.PLAIN/*Font.BOLD*/, 10));
		
		JScrollPane scrollPane = new JScrollPane(textArea);
		scrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
		scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
	    
		add(scrollPane,BorderLayout.CENTER);  
		
	}
	
	// Function called by OpenTm2Process to enable the GenerateReport Button
	// after the output is printed to the TextArea
	public void enableReport() {
		WindowConnector.getInstance().enableReport();
	}
	
	
	// Function called by OpenTm2Process to append the process output to the
	// Output TextArea taOutput
	public void printInputToTF(String tmp) {
		this.textArea.append(tmp + "\n");
	}
	
	//sets the Scripteroptions
		//called from ScripterOptions
	public void setOptions(boolean pHelp, boolean pDelTmp, boolean pDate, boolean pID, String logLevel, String logFile, String pConstants){
		this.bolShowHelp = pHelp;
		this.bolDelTmp = pDelTmp;
		this.bolDate = pDate;
		this.bolID = pID;
		this.setLogLevel(logLevel);
		this.setLogFile(logFile);
		this.constants = pConstants;
	}
	
	public FileTree getFileTree(){
		return mFileTree;
	}

	public File getSelFile(){
		//if(selectedFile==null)
			selectedFile = MainApp.getInstance().getActiveEditor().getFile();
		
		return selectedFile;
	}
	
	public void setSelFile(File selFile){
		if(selFile.isFile()){
			selectedFile = selFile;
			textField.setText(selFile.toString());
			textField.setColumns(selFile.toString().length());
		}
		else{
			textField.setText("~no file selected~");
		}
		if(selectedFile != null){
			fileIsSelected = true;
		}
		else{
			fileIsSelected = false;
		}		
	}
	
	public Boolean getBolShowHelp() {
		return bolShowHelp;
	}

	public void setBolShowHelp(Boolean bShowHelp) {
		bolShowHelp = bShowHelp;
	}
	
	public Boolean getFileIsSelected() {
		return fileIsSelected;
	}

	public void setFileIsSelected(Boolean fileIsSelected) {
		this.fileIsSelected = fileIsSelected;
	}

	public JTextArea getTextArea() {
		return textArea;
	}

	public Boolean getBolDelTmp() {
		return bolDelTmp;
	}

	public void setBolDelTmp(Boolean bDel) {
		bolDelTmp = bDel;
	}
	
	public Boolean getBolDate() {
		return bolDate;
	}
    
	public void setBolDate(Boolean bDate) {
		bolDate = bDate;
	}
	
	public Boolean getBolID() {
		return bolID;
	}

	public void setBolID(Boolean bID) {
		bolID = bID;
	}
	
	public String getConstants() {
		return constants;
	}
	
	public void setConstants(String strConstants) {
	   constants = strConstants;
	}
	
	public JPanel getSplitPane(){
		return runTS_splitPane; 
	}

	public String getLogLevel() {
		return logLevel;
	}

	public void setLogLevel(String logLevel) {
		this.logLevel = logLevel;
	}

	public String getLogFile() {
		return logFile;
	}

	public void setLogFile(String logFile) {
		this.logFile = logFile;
	}
    
}
