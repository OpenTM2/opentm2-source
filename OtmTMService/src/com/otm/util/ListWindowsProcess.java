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

  
public class ListWindowsProcess {
   
 
 public static boolean isMysqlRunning() throws IOException {
	 int runningCnts = 0;
	 ProcessBuilder pb = new ProcessBuilder("tasklist");
	 Process p = pb.start();
	 BufferedReader out = new BufferedReader(new InputStreamReader(new BufferedInputStream(p.getInputStream()), Charset.forName("utf-8")));
	 BufferedReader err = new BufferedReader(new InputStreamReader(new BufferedInputStream(p.getErrorStream())));
	  
	 String ostr;
     while ((ostr = out.readLine()) != null) {
    	if(ostr.indexOf("mysqld.exe")!=-1) {
    		runningCnts++;
    		break;
    	}
     }
     if(out != null)
    	 out.close();
     
	 String estr = err.readLine();
	 if (estr != null) {
	     System.out.println(estr);
	 }
     if(err != null)
    	 err.close();
     
	 return runningCnts==1;
 }

  
}