//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.test;

import java.beans.PropertyVetoException;
import java.sql.SQLException;

import org.apache.log4j.PropertyConfigurator;

import com.otm.db.Dao;
import com.otm.db.DaoFactory;
import com.otm.util.RunCommand;
import com.otm.util.ServerComponentsControl;

public class DbTest {

	public static void main(String[] args) throws SQLException, PropertyVetoException, InterruptedException, ClassNotFoundException {
PropertyConfigurator.configure("./configure/log4j.properties");	
//		Dao dao= DaoFactory.getInstance();
//		dao.createMemory("test_001", "wlp", "123", "*");
//		dao.addUser("otm_test_dao", "Terr");
//		dao.getUserList("otm_test_dao");
//		dao.removeUser("otm_test_dao", "Terr");
//		dao.listMemories("*");
//		dao.deleteMemory("otm_test_dao");
//		dao.backup("C:\\Users\\IBM_ADMIN\\TmShared\\mydb.sql");
		//dao.restore("C:\\Users\\IBM_ADMIN\\TmShared\\mydb.sql");
//		System.out.println(dao.download("wlp","test_001", 0));
//		dao.createUser("xxx","123","127.0.0.1","3306");
		  
		ServerComponentsControl.startMysql();

		Dao dao= DaoFactory.getInstance();
		//dao.createUser("xxx","123","127.0.0.1","3306");
		//dao.backup("C:\\Users\\IBM_ADMIN\\TmShared\\mydb.sql");
	}
	
}
