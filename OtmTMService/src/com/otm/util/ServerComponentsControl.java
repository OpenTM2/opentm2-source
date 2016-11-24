//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

import java.io.File;
import java.io.IOException;
import java.util.concurrent.TimeUnit;

import javax.swing.JOptionPane;

import org.apache.log4j.Logger;

import com.otm.log.ServiceLogger;

public class ServerComponentsControl {
	private static Logger mLogger = Logger.getLogger(ServiceLogger.class
			.getName());

	public static boolean startMysql() {
		try {
			if (ListWindowsProcess.isMysqlRunning()) {
				mLogger.info("Mysql already start");
				return true;
			}
			
			String dbDir = CfgUtil.getInstance().getDbCfg().get("db_installed_dir");
			File xampp = new File(dbDir);
			if (!xampp.exists()) {
				JOptionPane.showMessageDialog(null,
						"Please install MariaDB into "+dbDir);
				return false;
			}

			new ProcessBuilder(dbDir+"/bin/mysqld.exe","-u root").start();
	        
			TimeUnit.MILLISECONDS.sleep(1000);
			boolean bRuning = ListWindowsProcess.isMysqlRunning();
			int retryCnts = 0;
			while (!bRuning) {
				TimeUnit.MILLISECONDS.sleep(1000);
				bRuning = ListWindowsProcess.isMysqlRunning();
				mLogger.info("Checking Mysql status...");
				retryCnts++;
				if(retryCnts == 10) {
					mLogger.info("MySql can't be started...");
					return false;
				}
			}
		
			if(bRuning) {
				mLogger.info("Mysql start");
			}
		} catch (InterruptedException | IOException e) {
			e.printStackTrace();
		}
        
		return true;
	}

}