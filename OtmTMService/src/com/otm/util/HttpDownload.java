//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

public class HttpDownload {

    public static void  httpDownloadFromUrl(String urlStr,String savePath) {  
       
    	HttpURLConnection conn = null;
        InputStream inputStream = null;
        ByteArrayOutputStream bos = null;
        FileOutputStream fos = null;
        
        try {
        	 URL url = new URL(urlStr);    
             conn = (HttpURLConnection)url.openConnection();      
             conn.setConnectTimeout(200*1000);  
             conn.setRequestProperty("User-Agent", "Mozilla/4.0 (compatible; MSIE 5.0; Windows NT; DigExt)");  
             
	        inputStream = conn.getInputStream();    
	        byte[] buffer = new byte[1024];    
	        int len = 0;    
	 System.out.println("Downloading...");
	        bos = new ByteArrayOutputStream();    
	        while((len = inputStream.read(buffer)) != -1) {    
	            bos.write(buffer, 0, len);    
	        }    
	        if(bos != null)
	            bos.close();   
	        
	System.out.println("Saving...");  
	        byte[] dataDownloaded  = bos.toByteArray();    
	        File savedFile = new File(savePath);      
	        fos = new FileOutputStream(savedFile);       
	        fos.write(dataDownloaded); 
	        
        } catch (Exception e){
        	e.printStackTrace();
        } finally {
        	try {
        	if(fos!=null){  
                fos.close();    
            }  
            
            if(inputStream!=null){  
                inputStream.close();  
            }
            
            
            if(conn != null)
            	conn.disconnect();
            
            if(fos != null)
            	fos.close();
            
        	} catch(Exception e) {
        		e.printStackTrace();
        	}
        }
        
    }  
  
  
    public static void main(String[] args) {  
    	HttpDownload.httpDownloadFromUrl("https://downloadsapachefriends.global.ssl.fastly.net/xampp-files/5.6.15/xampp-win32-5.6.15-2-VC11-installer.exe",  
                "C:/xampp.exe");  
    }  
}
