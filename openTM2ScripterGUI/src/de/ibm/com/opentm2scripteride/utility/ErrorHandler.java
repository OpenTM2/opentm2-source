/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
/**************************************
 Error Handler Class,
 originally created by Jonas Traub, Filip Haase, Tobias Sch�neberger, Kai Weller, Martin Meinke in 04/2011
  modified by Georg Volk, Stefan Bruniki, Tim Reppenhagen in 01/2012 
 **************************************/

package de.ibm.com.opentm2scripteride.utility;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Calendar;

import javax.swing.JOptionPane;

import java.text.SimpleDateFormat;

/**
 * Handles errors by showing an appropriate Dialogs and writing them to the log file
 */
public class ErrorHandler {

	public static final String DATE_FORMAT_NOW = "yyyy-MM-dd HH:mm:ss";

	private BufferedWriter logWriter;
	private static ErrorHandler instance = null;
	
	enum ErrorType {
		Warning("Warning", JOptionPane.WARNING_MESSAGE),
		Error("Error", JOptionPane.ERROR_MESSAGE),
		FatalError("Fatal Error", JOptionPane.ERROR_MESSAGE);
		
		String type;
		int optionPaneIcon;
		
		ErrorType(String t, int paneIcon) {
			type = t;
			optionPaneIcon = paneIcon;
		}
		public String getType() {
			return type;
		}
		public int getOptionPaneIcon() {
			return optionPaneIcon;
		}
	}
	
	public static final int WARNING = 0;
	public static final int ERROR = 1;
	public static final int FATAL_ERROR = 2;
	
	/**
	 * The constructor
	 * @param logfile The logfile we should write to
	 */
	protected ErrorHandler(File logfile){
		this.logWriter=openLogFile(logfile);
		
	}
	
	/**
	 * Handles an error
	 * @param type The final ints of this class should be used (ERROR, FATAL_ERROR or WARNING)
 	 * @param message The error message 
	 */
	public void handleError(int type, String message) {
		ErrorType err = getErrorType(type);
		writeLog(err.getType() + ": Time: "+getTime() + "\n  " + message + "\n");
		JOptionPane.showMessageDialog(null, message + "\n\nNote: This " + err.getType()
				+ " was written to logfile", err.getType(), err.getOptionPaneIcon());
	}

	/**
	 * Handles an error and adds the message of the exception
	 * @param type The final ints of this class should be used (ERROR, FATAL_ERROR or WARNING)
 	 * @param message The error message
 	 * @param e The exception to be added
	 */
	public void handleError(int type, String message, Exception e) {
		handleError(type, message + "\n" + e.getMessage());
	}
	
	/**
	 * Handles an error from an exception
 	 * @param e The exception to be handled
	 */
	public void handleError(Exception e) {
		handleError(ERROR, e.getMessage());
	}
	
	/**
	 * Returns the ErrorType corresponding to the int
	 * @param t The final int (ERROR, FATAL_ERROR or WARNING)
	 * @return The corresponding ErrorType (ErrorType.Error as default)
	 */
	protected ErrorType getErrorType(int t) {
		if (t == 0) {
			return ErrorType.Warning;
		} else if (t == 2) {
			return ErrorType.FatalError;
		}
		return ErrorType.Error;
	}
	
	/**
	 * Returns the current time as a string
	 * @return The current time as a string
	 */
	public static String getTime() {
		Calendar cal = Calendar.getInstance();
	    SimpleDateFormat sdf = new SimpleDateFormat(DATE_FORMAT_NOW);
	    return sdf.format(cal.getTime());
	}
	
	/**
	 * Simply logs the message of the exception
	 * @param e The exception to be logged
	 */
	public void logErrorDetails(Exception e){
		if (logWriter!=null){
			try {
				logWriter.write(e.getMessage());
			} catch (IOException e1) {}
		}
	}
	
	/**
	 * Writes a message to the logfile
	 * @param msg The message to be written
	 */
	public void writeLog(String msg) {
		if (logWriter == null) {
			return;
		}
		try {
			logWriter.write(msg);
			logWriter.flush();
		} catch (IOException e) {
			plainError("ERROR: Failed to write to log file:\n" + e.getMessage());
		}
	}

	/**
	 * Opens the file as a BufferedWriter to log
	 * @param logfile The file to be opened
	 * @return The BufferedWriter object or null
	 */
	private BufferedWriter openLogFile(File logfile){
		try {
			return new BufferedWriter(new FileWriter(logfile,true));
		} catch (Exception e) {
			plainError("ERROR: Unable to open logfile:\n" + e.getMessage() + "\nNote: This error could be a reason for following errors.");
		}
		return null;
	}
	
	/**
	 * Simply shows an error without logging it
	 * @param msg The error message to be shown
	 */
	protected static void plainError(String msg) {
		JOptionPane.showMessageDialog(null,msg,"ERROR!", JOptionPane.ERROR_MESSAGE);
	}
	
	/**
	 * Used to access the one and only ErrorHandler object globally.
	 * @return The existing ErrorHandler object
	 */
	public static ErrorHandler getInstance(){
		if ( instance == null ) {
			String logDir = Configuration.getInstance().getValue(Constants.logFileLocation);
			File logFile = null;
			try{
				logFile= new File(logDir);
			}
			catch(NullPointerException e){
				try{
					logFile = new File("log.txt");
					FileWriter writer = new FileWriter(logFile ,false);
					writer.close();
				}
				catch(IOException ex){
					plainError("ERROR: Unable to create logfile. \n "
							+ "openTM2Scripter was also not able to create the logfile because of missing authorization.");
				}
			}
			
			instance = new ErrorHandler(logFile);
		}
		return instance;
	}
}
