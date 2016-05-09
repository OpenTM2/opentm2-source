//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.List;

  
public class ListWindowsProcess {
   
 private static List<String> listWindowsRunningProcess() throws IOException {
	 ArrayList<String> running = new ArrayList<String>();
	 
	 ProcessBuilder pb = new ProcessBuilder("tasklist");
	 Process p = pb.start();
	 BufferedReader out = new BufferedReader(new InputStreamReader(new BufferedInputStream(p.getInputStream()), Charset.forName("utf-8")));
	 BufferedReader err = new BufferedReader(new InputStreamReader(new BufferedInputStream(p.getErrorStream())));
	  
	 String ostr;
     while ((ostr = out.readLine()) != null) {
    	 String[] arrs = ostr.split("  ");
    	 running.add(arrs[0]);
     }
     if(out != null)
    	 out.close();
     
	 String estr = err.readLine();
	 if (estr != null) {
	     System.out.println(estr);
	 }
     if(err != null)
    	 err.close();
     
	 return running;
 }
 

 public static boolean isProcessRunning(String processName) {
	 boolean isRunning = false;
	 try {
		List<String> process = listWindowsRunningProcess();
		if(process.indexOf(processName) != -1)
			isRunning = true;
	} catch (IOException e) {
		e.printStackTrace();
	} 
	return isRunning;
 }
 
 public static boolean isMysqlApacheRunning() throws IOException {
	 int runningCnts = 0;
	 ProcessBuilder pb = new ProcessBuilder("tasklist");
	 Process p = pb.start();
	 BufferedReader out = new BufferedReader(new InputStreamReader(new BufferedInputStream(p.getInputStream()), Charset.forName("utf-8")));
	 BufferedReader err = new BufferedReader(new InputStreamReader(new BufferedInputStream(p.getErrorStream())));
	  
	 String ostr;
     while ((ostr = out.readLine()) != null) {
    	if(ostr.indexOf("mysqld.exe")!=-1 || ostr.indexOf("httpd.exe")!=-1)
    		runningCnts++;
    	if(runningCnts == 2)
    		break;
     }
     if(out != null)
    	 out.close();
     
	 String estr = err.readLine();
	 if (estr != null) {
	     System.out.println(estr);
	 }
     if(err != null)
    	 err.close();
     
	 return runningCnts==2;
 }
 
// public static void main(String[] args) throws Exception {
//    
//	 boolean res = ListWindowsProcess.isProcessRunning("httpd.exe");
//	 System.out.println(res);
// }
  
}