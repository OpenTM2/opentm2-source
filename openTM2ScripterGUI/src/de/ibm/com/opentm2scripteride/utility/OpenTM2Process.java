/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
/**************************************
 * ################################## *
 *                                    *
 * 		Created by:			 	      *
 * 							 	      *
 * 		Jonas Traub			 	      *
 * 		Filip Haase			 	      *
 * 		Tobias Sch�neberger	 	      *
 * 		Kai Weller			 	      *
 * 		Martin Meinke		 	      *
 * 							 	      *
 * 		04/2011				 	      *
 *                                    *
 * ################################## *
 **************************************/

package de.ibm.com.opentm2scripteride.utility;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.lang.ProcessBuilder;
import java.util.Scanner;
import de.ibm.com.opentm2scripteride.gui.RunConsolePanel;
import de.ibm.com.opentm2scripteride.gui.WindowConnector;


public class OpenTM2Process extends Thread {

	// Variable Declaration
	private ProcessBuilder builder;
	private File workingdir;
	private Process workingprocess;
	private String PathToScript;
	private RunConsolePanel RTS;
	private Scanner scan;
  
	public void run() {
		try {
			Thread.sleep(2);
			this.setRTSText();
			
		} catch (InterruptedException e) {
		}
		
	}

	// Constructor
	// RTS is a Reference to RunTestSuitePanel
	public OpenTM2Process(String pPathToScript, RunConsolePanel pRTS, boolean showHelp, boolean delTmp, boolean bolDate, boolean bolID, String logLevel, String logFile,String constants) {
		this.RTS = pRTS;
		
		this.PathToScript = pPathToScript;
		Configuration conf = Configuration.getInstance();
		if(showHelp == true){
			builder = new ProcessBuilder(
					conf.getValue("OPENTM2SCRIPTER"), "/HELP");
		}else{
			StringBuilder sbParamters = new StringBuilder( this.PathToScript);

			if(logLevel!=null && !"".equals(logLevel)) {
				sbParamters.append(" \" \" ");
				sbParamters.append("/loglevel="+logLevel);
				
			}
			
			if(logFile!=null && !"".equals(logFile)) {
				sbParamters.append(" \" \" ");
				sbParamters.append("/log="+logFile);
			}
			
			String infor = this.getParameters(delTmp, bolDate, bolID);
			if(infor!=null && !"".equals(infor)) {
				sbParamters.append(infor);
			}
			
			if(constants!=null && !"".equals(constants)){
				sbParamters.append(" \" \" ");
				sbParamters.append(constants);
			}
			System.out.println(sbParamters.toString());
			builder = new ProcessBuilder(conf.getValue("OPENTM2SCRIPTER"), sbParamters.toString());

		}
		this.setWorkingDir(this.PathToScript);

		try {
			workingprocess = builder.start();
		}catch (IOException e) {
			ErrorHandler.getInstance().handleError(ErrorHandler.ERROR,
							"Commandline Error \n"+
							"Can not start the Commandline.\nSee the logfile for more Information.");
			ErrorHandler.getInstance().logErrorDetails(e);
		}
	}

	// Sets the active working directory
	public void setWorkingDir(String dir) {
		int i = dir.length();

		while (i > 1) {
			if (dir.charAt(i - 1) == '\\') {
				dir = dir.substring(0, i);
				i = 0;
			}
			i--;
		}

		this.workingdir = new File(dir);
		this.builder.directory(this.workingdir);
	}

	// Returns a scanner on the process input stream
	// Input stream is used to get data from the process
	public Scanner getProcessInputScanner() {
		Scanner s = new Scanner(this.workingprocess.getInputStream());
		return s;
	}

	// Returns a scanner on the process error stream
	// InputStream returns the process error outputdata
	public Scanner getProcessErrorScanner() {
		Scanner s = new Scanner(this.workingprocess.getErrorStream());
		return s;
	}

	// Returns the process output stream
	// Output stream is used to send data to the Process
	public OutputStream getProcessOutputStream() {
		return this.workingprocess.getOutputStream();
	}

	//Prints the Process Output to the TextArea in RunTestSuite
	public void setRTSText() throws InterruptedException {
		this.scan = new Scanner(this.workingprocess.getInputStream());
		
		while (this.scan.hasNext()) {
			this.RTS.printInputToTF(scan.nextLine());
            Thread.sleep(5);
		}
		this.scan.close();
		
		WindowConnector.getInstance().enableRunBtn(true);
		WindowConnector.getInstance().enableStopBtn(false);
		this.RTS.enableReport();
	}
	
	public void stopProcess(){
		this.workingprocess.destroy();
	}
	
	private String getParameters( boolean delTmp, boolean date, boolean ID){

		    StringBuilder sb = new StringBuilder();

			if(delTmp){
				sb.append(" \" \" ");
				sb.append("/delTemp");
			}

			if(date) {
				sb.append(" \" \" ");
				sb.append("/INFO=d");
			}
				
			if(ID) {
				sb.append(" \" \" ");
				sb.append("/INFO=p");
			}
			
			return sb.toString();
	}
}
