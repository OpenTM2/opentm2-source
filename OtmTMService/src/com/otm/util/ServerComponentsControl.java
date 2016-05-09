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

	public static void stopApacheMysql() {
		File xampp = new File("C:/xampp");
		if (!xampp.exists()) {
			JOptionPane.showMessageDialog(null,
					"Please install XAMPP into C:/xampp directory.");
			return;
		}

		try {

			new ProcessBuilder("C:/xampp/mysql_stop.bat").start();
			new ProcessBuilder("C:/xampp/apache_stop.bat").start();
			Thread.sleep(500);

		} catch (InterruptedException | IOException e) {
			e.printStackTrace();
		}
	}

	public static void startApacheMysql() {
		try {
			if (ListWindowsProcess.isMysqlApacheRunning()) {
				mLogger.info("Apache and Mysql already start");
				return;
			}
			
			File xampp = new File("C:/xampp");
			if (!xampp.exists()) {
				JOptionPane.showMessageDialog(null,
						"Please install XAMPP into C:/xampp directory.");
				return;
			}

			new ProcessBuilder("C:/xampp/mysql_start.bat").start();
			new ProcessBuilder("C:/xampp/apache_start.bat").start();

			TimeUnit.MILLISECONDS.sleep(1000);
			boolean bRuning = ListWindowsProcess.isMysqlApacheRunning();
			while (!bRuning) {
				TimeUnit.MILLISECONDS.sleep(1000);
				bRuning = ListWindowsProcess.isMysqlApacheRunning();
				mLogger.info("Checking Apache and Mysql status...");
			}
		
			if(bRuning) {
				mLogger.info("Apache and Mysql start");
			}
		} catch (InterruptedException | IOException e) {
			e.printStackTrace();
		}

	}

	public static void main(String[] args) {
		ServerComponentsControl.startApacheMysql();
		// ServerComponentsControl.stopApacheMysql();
	}

}