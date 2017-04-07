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
import javax.swing.UIManager;
import javax.xml.namespace.QName;
import javax.xml.ws.Endpoint;
import javax.xml.ws.Service;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import com.otm.gui.ConfigureWindow;
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
		// start mysql
		mLogger.info("Starting Mysql...");
		boolean bMsqlStarted = ServerComponentsControl.startMysql();
		if(!bMsqlStarted) {
			return null;
		}
		
	    mEndpoint = Endpoint.publish(mStrUrl, new OtmTMServiceImpl());
          
        QName qname = new QName("http://webservice.otm.com/","OtmTMServiceImplService");  
        Service service = Service.create(new URL(mStrUrl+"?wsdl"), qname);
        return service.getPort(OtmTMService.class);  
	}
	
    
	public void stop() {			
		if(mEndpoint != null)
		    mEndpoint.stop();
		    System.exit(0);
	}
	


   public static void main(String[] args) {
	   PropertyConfigurator.configure("./configure/log4j.properties");	   
	   // start point
	   OtmTMServiceServer server = null;
	   try {
		   UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		   
		   // set configure for the first time
		   if( CfgUtil.getInstance().getDbCfg().get("root_password")==null ||
			   CfgUtil.getInstance().getDbCfg().get("db_installed_dir")==null){
			   ConfigureWindow.show();
		   } else {
			   // if not the first time, ask the user whether want to pop up configure dialog
			   int n = JOptionPane.showConfirmDialog(null, "Do you want to change the configuration now ?", "OtmTmService", JOptionPane.YES_NO_OPTION);
			   if(n == 0) {
				   ConfigureWindow.show();
			   }
		   }  
		   
		   // after this, web service begin to work
		   // synchronize function receive messages to process
			server = new OtmTMServiceServer();
			if( server.start() != null ) {
				System.out.println(server.getUrl());
			    //JOptionPane.showMessageDialog(null, "Server running...\n"+server.getUrl(), "OtmTMService", JOptionPane.INFORMATION_MESSAGE);
			    Object[] options = {"Stop"};
				JOptionPane.showOptionDialog(null,
						                            "Server running...\n"+server.getUrl(),
													"OtmTMService",
													JOptionPane.OK_OPTION,
													JOptionPane.INFORMATION_MESSAGE,
													null,
													options,
													options[0]);
			    server.stop();
			} else {
				 JOptionPane.showMessageDialog(null, "MySql can't be started!", "OtmTMService", JOptionPane.INFORMATION_MESSAGE);
			}
						
	   }catch (Exception e) {
		   e.printStackTrace();
	   }

   }
   
}
