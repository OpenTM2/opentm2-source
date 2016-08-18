//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;


public class RunCommand {
    
	private String lastErrorMsg = "";
	/**
	 * 
	 * @param cmd
	 * @return 1 failed, 0 success
	 */
	public int  backupCmd(String cmd) {
        Runtime run = Runtime.getRuntime();
        
        try {
            Process p = run.exec( cmd );
            return cmdIO( null,null,p.getErrorStream());
        } catch(Exception e) {
        	e.printStackTrace();
        }
        
        
       
        return 1;
    }

	/**
	 * 
	 * @param cmd
	 * @param outpath
	 * @return
	 */
	public int  restoreCmd(String cmd) {
        Runtime run = Runtime.getRuntime();
        
        try {
            Process p = run.exec(cmd );
            return cmdIO(null,null,p.getErrorStream());
        } catch(Exception e) {
        	e.printStackTrace();
        }
        
		return 1;
    }
	

	private int cmdIO(InputStream ins, OutputStream outs, InputStream errors) {
		try {  
           
			if(ins!=null && outs!=null) {
				 BufferedReader inBr = new BufferedReader(new InputStreamReader( new BufferedInputStream(ins) ));  
		            
		            // output dbs to output file
		            PrintWriter pw = new PrintWriter(new OutputStreamWriter(outs, "utf8"));
		            StringBuilder sbOut = new StringBuilder();
		            String tempLine;
		     	    while ((tempLine = inBr.readLine()) != null) { 
		     	    	sbOut.append(tempLine);
		            } 
   	   
		     	    pw.write(sbOut.toString());
		            inBr.close(); 
		            pw.close();
		            
			}
           
            String lineStr;  
            BufferedReader errorBr = new BufferedReader(new InputStreamReader(new BufferedInputStream(errors)));  
           
           // check if there are errors
           lineStr = errorBr.readLine();
           StringBuilder sbError = new StringBuilder();
           while(lineStr!=null) {
        	   System.out.println(lineStr);
        	   sbError.append(lineStr);
        	   lineStr = errorBr.readLine();
           }
        
           errorBr.close();
           
           
           if(sbError.length()==0) 
        	   return 0;
           else
        	   lastErrorMsg = sbError.toString();

        } catch (Exception e) {  
            e.printStackTrace();  
        } 
		return 1;
	}
	
	
	public String getLastErrorMsg() {
		return lastErrorMsg;
	}

}
