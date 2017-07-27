//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.db;

import java.beans.PropertyVetoException;
import java.sql.SQLException;
import java.util.List;
import java.util.Map;
import java.util.Set;




public interface Dao {
	
/*
	 void createDB(String serverIP, 
			       String port, 
			       String user, 
			       String password, 
			       String driverClass) throws SQLException, PropertyVetoException;
*/	 
	 void createDB() throws SQLException, PropertyVetoException;
	 
	 void createMemory(/*String serverIP, 
			           String port, */
			           String memoryname,
			           String user, 
			           String password,
			           //String driverClass,
			           String userList) throws SQLException, PropertyVetoException;
	 
	 void deleteMemory(String memoryname) throws SQLException;
	 
	 int upload(String user, String memoryname, List<TU> tus) throws SQLException;
	 
	 Map<String,String> download(String user, String memoryname, long updateCounter,Set<Long> ownUploadedCounters) throws SQLException;
	 
	 String listMemories(String userid) throws SQLException;
	 
	 String getDbName();
	 
	 String getCreatorName(String memoryName) throws SQLException;
	 
	 String getUserList(String memoryName) throws SQLException;
	 String addUser(String memoryName,String userToAdd) throws SQLException;
	 String removeUser(String memoryName, String userToRemove) throws SQLException;
	 
	 boolean backup(String outname);
	 boolean restore(String inname);
	 
	 void createUser(String username, String password, String IP,String port) throws SQLException,ClassNotFoundException ;
}
