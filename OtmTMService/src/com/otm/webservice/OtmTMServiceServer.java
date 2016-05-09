//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.webservice;
import java.net.MalformedURLException;
import java.net.URL;

import javax.swing.JOptionPane;
import javax.xml.namespace.QName;
import javax.xml.ws.Endpoint;
import javax.xml.ws.Service;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import com.otm.log.ServiceLogger;
import com.otm.util.CfgUtil;
import com.otm.util.ServerComponentsControl;


public class OtmTMServiceServer {

	private Endpoint mEndpoint = null;
	private final String mStrUrl = CfgUtil.getInstance().getUrl();
    private static Logger mLogger = Logger.getLogger(ServiceLogger.class.getName());
	public String getUrl() {
		return mStrUrl;
	}
	
	public OtmTMService start() throws MalformedURLException {
		// start mysql and apache firstly
		//System.out.println("Starting Apache server and Mysql...");
		mLogger.info("Starting Apache server and Mysql...");
		ServerComponentsControl.startApacheMysql();
		
	    mEndpoint = Endpoint.publish(mStrUrl, new OtmTMServiceImpl());
          
        QName qname = new QName("http://webservice.otm.com/","OtmTMServiceImplService");  
        Service service = Service.create(new URL(mStrUrl+"?wsdl"), qname);
        return service.getPort(OtmTMService.class);  
	}
	
    
	public void stop() {			
		if(mEndpoint != null)
		    mEndpoint.stop();
	}
	


   public static void main(String[] args) {
	   PropertyConfigurator.configure("./configure/log4j.properties");	   
	   // start point
	   OtmTMServiceServer server = null;
	   try {
		   
		   // after this, webservice begin to work
		   // synchronize function receive messages to process
			server = new OtmTMServiceServer();
			server.start();
			JOptionPane.showMessageDialog(null, "Server running...\n"+server.getUrl(), "OtmTMService", JOptionPane.INFORMATION_MESSAGE);
			server.stop();
			//System.out.println("Server running, input any character and press 'Enter' to stop the server");
			/*try {
				System.in.read(new byte[1], 0, 1);
				server.stop();
			} catch (IOException e) {
				e.printStackTrace();
			}
			*/
			
	   }catch (MalformedURLException e) {
		   e.printStackTrace();
	   }

   }
   
}
