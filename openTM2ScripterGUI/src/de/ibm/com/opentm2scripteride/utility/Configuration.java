/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
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

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;


public class Configuration {
	static private Configuration mInstance = null;
	private Properties properties;
	private BufferedInputStream stream;

	private Configuration() {
		properties = new Properties();
		try {
			stream = new BufferedInputStream(new FileInputStream(
					"configuration.conf"));
		} catch (FileNotFoundException e) {
			try{
				initConfFile();
			}
			catch(IOException ex){
				ErrorHandler.getInstance().handleError( ErrorHandler.ERROR,
						"Configurations file not found and was\n"+
						"This file is required for the setup of a lot of functions.\n " +
						"openTM2Scripter was also not able to create one because of missing authorization.");
			}
			ErrorHandler.getInstance().handleError( ErrorHandler.ERROR,
							"Configurations file not found \n"+
							"This file is required for the setup of a lot of functions.\n This error could be the reason of following errors.");
		}
		if (stream != null) {
			try {
				properties.load(stream);
				stream.close();
			} catch (IOException e) {
				ErrorHandler.getInstance().handleError( ErrorHandler.ERROR, 
					"Can not access to the configuration file \n"+
					"This file is required for the setup of a lot of functions.\n This error could be the reason of following errors."
					);
			}
		}

	}
	
	static public Configuration getInstance() {
		if (mInstance == null) {
			mInstance = new Configuration();
		}
		return mInstance;
	}
	
	public void initConfFile() throws IOException{
		File conf = new File("configuration.conf");
		FileWriter writer = new FileWriter(conf ,false);
		String actualtime = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss").format(new Date());
		writer.write("#OpenTM2 Configurationfile \n#"+actualtime);
		writer.flush();
		writer.close();
		
		setValue(Constants.commandXMLLocation,"resources\\commands.xml");
	}

	public String getValue(String pDescriptor) {
		String ret = properties.getProperty(pDescriptor);
		return ret;
	}

	public void setValue(String pDescription, String pValue) {
		File confFile = new File("configuration.conf");
		properties.put(pDescription, pValue);
		try {
			properties.store(new FileOutputStream(confFile), "OpenTM2 Configurationfile");
		} catch (FileNotFoundException e) {
			ErrorHandler.getInstance().handleError( 0, 
							"Can not find Configuration File \n"+
							"This file is required for the setup of a lot of functions.\n This error could be the reason of following errors.");
		} catch (IOException e) {
			ErrorHandler.getInstance().handleError( 0,
							"Can not access the configuration file \n"+
							"This file is required for the setup of a lot of functions.\n This error could be the reason of following errors.");
		}
	}
}
