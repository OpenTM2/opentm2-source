//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.db;

import java.beans.PropertyVetoException;
import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

import org.apache.log4j.Logger;

import com.mchange.v2.c3p0.ComboPooledDataSource;
import com.otm.log.ServiceLogger;
import com.otm.util.CfgUtil;
import com.otm.util.RunCommand;
import com.otm.util.SimpleBase64;
import com.otm.util.SyncParameters;


// this class must be thread safe
public class DaoMysqlImpl implements Dao{

	// ComboPooledDataSource is read-only thread safe
	// Don't expose this to out class, set and get don't allowed
	private ComboPooledDataSource mDs = null;
	
	private static final String DBNAME = "otms_mysql_db";
	private static final String MEMORYINFOTABLE = "otm_memories_info";
	private static final Object USERLISTLOCK = new Object();
	private static final Object UPDATECOUNTERLOCK = new Object();
	private static Logger mLogger = Logger.getLogger(ServiceLogger.class.getName());
	private static final ConcurrentHashMap<String,String> mUserList = new  ConcurrentHashMap<String,String>();
	
	/**
	 * 
	 * @param dbcfg
	 */
	// only call from DaoFactory for once with synchronized
	DaoMysqlImpl(Map<String, String> dbcfg) {
		
		String driverClass = dbcfg.get("driver_class");
		String jdbcUserName = dbcfg.get("user_name");
		String jdbcPassword = dbcfg.get("user_password");
		String serverIP = dbcfg.get("server_ip");
		String port = dbcfg.get("server_port");
		
// create configured user account
// retry until success
boolean bSuccess = false;
while(!bSuccess) {
	bSuccess = true;
	try {
		createUser(jdbcUserName,jdbcPassword,serverIP,port);
	} catch (ClassNotFoundException|SQLException e) {
		e.printStackTrace();
		mLogger.info(e.getMessage());
		bSuccess = false;
		try {
			TimeUnit.MILLISECONDS.sleep(1000);
		} catch (InterruptedException e1) {
			e1.printStackTrace();
		}
	} 	
}

		mDs = new ComboPooledDataSource();
		mDs.setUser(jdbcUserName);
		mDs.setPassword(jdbcPassword);
		try {
			
			mDs.setDriverClass(driverClass);
			StringBuilder url = new StringBuilder();
            if( "com.mysql.jdbc.Driver".equals(driverClass) ) 
                url.append("jdbc:mysql://").append(serverIP).append(":").append(port);
            else if( "org.mariadb.jdbc.Driver".equals(driverClass) )
		        url.append("jdbc:mariadb://").append(serverIP).append(":").append(port);
		    mDs.setJdbcUrl(url.toString());
		    		    
		    // At the same time create DB and INFOTABLE
		    createDB();
	    
		} catch (SQLException e) {
			e.printStackTrace();
		} catch (PropertyVetoException e) {
			e.printStackTrace();
		}
		
		
	}

	/**
	 * 
	 * @param serverIP
	 * @param port
	 * @param user
	 * @param password
	 * @param driverClass
	 * @throws SQLException
	 * @throws PropertyVetoException
	 */
	@Override
	public void createDB() throws SQLException,
			PropertyVetoException {
		
		Connection conn = null; 
		Statement stmt = null;
		try {
			
			conn = mDs.getConnection();
			
			if(conn != null)
				stmt = (Statement) conn.createStatement();
			
			DatabaseMetaData meta = (DatabaseMetaData) conn.getMetaData();
			ResultSet set = meta.getTables(null, null, MEMORYINFOTABLE, null);
			if (set.next()) {
				return;
			}
	            
			// create database
			if(stmt != null) {
				stmt.executeUpdate("CREATE DATABASE IF NOT EXISTS "+DBNAME+" COLLATE = utf8_unicode_ci");
				
				StringBuilder sbMemInfoSql = new StringBuilder();
				sbMemInfoSql.append("CREATE TABLE IF NOT EXISTS ").append(DBNAME).append(".").append(MEMORYINFOTABLE).append("(");
				//sbMemInfoSql.append("ID INT(10) AUTO_INCREMENT,");
				sbMemInfoSql.append("CREATORNAME VARCHAR(20),");
				sbMemInfoSql.append("CREATORPASSWD VARCHAR(20),");
				sbMemInfoSql.append("DRIVERCLASS VARCHAR(50),");
				sbMemInfoSql.append("MEMORYNAME VARCHAR(50),");
				sbMemInfoSql.append("MEMORYUPCOUNTER INT(10),");
				sbMemInfoSql.append("USERLIST VARCHAR(200),PRIMARY KEY(MEMORYNAME))");
				
				stmt.executeUpdate(sbMemInfoSql.toString());
			}
		}
		finally {
			if(stmt != null)
				stmt.close();
			if(conn != null)
				conn.close();
		}
		
	}

	/**
	 * 
	 * @param serverIP
	 * @param port
	 * @param memoryname
	 * @param user
	 * @param password
	 * @param driverClass
	 * @param userList
	 * @throws SQLException
	 * @throws PropertyVetoException
	 */
	@Override
	public void createMemory(String memoryname,String user, String password,String userList) throws SQLException, PropertyVetoException {
		
		Connection conn = null; 
		Statement stmt = null;
		PreparedStatement  pstmt = null;
		
		try {
			conn = mDs.getConnection();
			
			if(conn != null)
				stmt = (Statement) conn.createStatement();
//System.out.println(memoryname);	
			//mLogger.info(memoryname);
			// create database
			if(stmt != null) {
				// create tu table
				StringBuilder sbTuSql = new StringBuilder();
				sbTuSql.append("CREATE TABLE ").append(DBNAME).append(".").append(memoryname).append("_").append("TU(");
				sbTuSql.append("ID INT(10) AUTO_INCREMENT,");
				sbTuSql.append("CREATIONDATE VARCHAR(20),");
				sbTuSql.append("UPDATECOUNTER INT(10),");
				sbTuSql.append("SEGNUM INT(10),");  
				sbTuSql.append("MARKUP VARCHAR(50),");
				sbTuSql.append("DOCNAME VARCHAR(500),");
				sbTuSql.append("SHORTDOCNAME VARCHAR(200),");
				sbTuSql.append("TUHASH INT(64),PRIMARY KEY(ID))");
				
				stmt.executeUpdate(sbTuSql.toString());
				
				//create tuv table
				StringBuilder sbTuvSql = new StringBuilder();
				sbTuvSql.append("CREATE TABLE ").append(DBNAME).append(".").append(memoryname).append("_").append("TUV(");
				sbTuvSql.append("ID INT(10) AUTO_INCREMENT,");
				//sbTuvSql.append("CREATIONDATE VARCHAR(20),");
				sbTuvSql.append("UPDATECOUNTER INT(10),");
				sbTuvSql.append("SEGMENT VARCHAR(2048),");
				sbTuvSql.append("LANG VARCHAR(20),");
				sbTuvSql.append("LANGUAGE VARCHAR(50),");
				sbTuvSql.append("CREATOR VARCHAR(20),");
				sbTuvSql.append("PARENTID INT(10),");
				sbTuvSql.append("PRIMARY KEY(ID))");
				
				stmt.executeUpdate(sbTuvSql.toString());
				
				// insert memory information into MEMORYINFOTABLE				
				StringBuilder sbInsertSql = new StringBuilder();
				sbInsertSql.append("INSERT INTO ").append(DBNAME).append(".").append(MEMORYINFOTABLE).append(" VALUES(?,?,?,?,?,?)");
				pstmt = conn.prepareStatement(sbInsertSql.toString());
				//pstmt.setString(1, null);
				pstmt.setString(1, user);
				pstmt.setString(2, password);
				pstmt.setString(3, "mysql");
				pstmt.setString(4, memoryname);
				pstmt.setInt(5, 1);
				
				if(null==userList||userList.isEmpty())
					userList = "*";
				pstmt.setString(6, userList);
				
				pstmt.executeUpdate();
				
			}
		}
		finally {
			if(stmt != null)
				stmt.close();
			if(pstmt != null)
				pstmt.close();
			if(conn != null)
				conn.close();
		}
	    
	}

	/**
	 * 
	 * @return
	 */
	@Override
	public String getDbName() {
		return DBNAME;
	}

	/**
	 * 
	 * @param memoryName
	 * @return
	 * @throws SQLException
	 */
	@Override
	public String getCreatorName(String memoryName) throws SQLException {
		StringBuilder sbQuery = new StringBuilder("select CREATORNAME from ").append(DBNAME).append(".").append(MEMORYINFOTABLE)
				.append(" where MEMORYNAME='").append(memoryName).append("'");
		
		Connection conn = null;
		Statement stmt = null;
		ResultSet rs = null;
		
	    try {
			conn = mDs.getConnection();
			stmt = (Statement) conn.createStatement();
			rs = stmt.executeQuery(sbQuery.toString());
			rs.next();
			String creatorName = rs.getString("CREATORNAME");
			return creatorName;
			
	    } finally {
			if(rs!=null)
				rs.close();
			if(stmt != null)
			    stmt.close();
			if(conn != null)
				conn.close();	
		}
	}

	/**
	 * 
	 * @param memoryname
	 * @throws SQLException
	 */
	@Override
	public void deleteMemory(String memoryname) throws SQLException {
		StringBuilder sbDrop = new StringBuilder();
		sbDrop.append("DROP TABLE IF EXISTS ").append(DBNAME).append(".").append(memoryname).append("_TU,")
		                        .append(DBNAME).append(".").append(memoryname).append("_TUV");

		StringBuilder sbDelete = new StringBuilder();
		sbDelete.append("DELETE FROM ").append(DBNAME).append(".").append(MEMORYINFOTABLE).append(" WHERE MEMORYNAME='").append(memoryname).append("'");
		
		Connection conn = null;
		Statement stmt = null;
		
		try {
			 conn = mDs.getConnection();
	         stmt = (Statement) conn.createStatement();
             stmt.executeUpdate(sbDrop.toString());
		     stmt.executeUpdate(sbDelete.toString());
		} finally {
			if(stmt!=null)
				stmt.close();
			if(conn!=null)
				conn.close();
		}
	}

	/**
	 * 
	 * @param memoryName
	 * @return
	 * @throws SQLException
	 */
	@Override
	public String getUserList(String memoryName) throws SQLException {
		String users = listMemoryUsers(memoryName);
		
		// cache with no over write
		if(users != null) {
			mUserList.putIfAbsent(memoryName, users);
		}
		
		return users;
	}

	
	/**
	 * 
	 * @param memoryName
	 * @param userToAdd
	 * @return
	 * @throws SQLException
	 */
	@Override
	public String addUser(String memoryName,String userToAdd) throws SQLException {
		synchronized(USERLISTLOCK) {
			String users = listMemoryUsers(memoryName);

			if("*".equals(users))
				return "User list is *, don't allow to add user";
			
			String[] usersList = users.split(",");
			for(String user:usersList) {
				if(user.equals(userToAdd))
					return "User already exist in user list";
			}
			
			StringBuilder newUsers = new StringBuilder(users);
			newUsers.append(",").append(userToAdd);
			
			StringBuilder sbUpdate = new StringBuilder("update ").append(DBNAME).append(".").append(MEMORYINFOTABLE)
					.append(" SET USERLIST='").append(newUsers).append("'")
					.append(" where MEMORYNAME='").append(memoryName).append("'");
			
			Connection conn = null;
			Statement stmt = null;
			try {
				  conn = mDs.getConnection();
				  stmt = (Statement) conn.createStatement();
		          stmt.executeUpdate(sbUpdate.toString());
		          
		          // update cache with over write here
		          mUserList.put(memoryName, newUsers.toString());
			} finally {
				
				if(stmt != null)
				    stmt.close();
				if(conn != null)
					conn.close();	
			}
			
		}//end synchronized
		return null;
	}

	/**
	 * 
	 * @param memoryName
	 * @param userToRemove
	 * @return
	 * @throws SQLException
	 */
	@Override
	public String removeUser(String memoryName, String userToRemove) throws SQLException {
		synchronized(USERLISTLOCK) {
			String users = listMemoryUsers(memoryName);
			if("*".equals(users))
				return "User list is *, don't allow to remove user";
			
			StringBuilder newUsers = new StringBuilder();
			String[] usersList = users.split(",");
		        
			int cnt = 0;
			for(String user: usersList) {
				if(!user.equals(userToRemove)) {
				    newUsers.append(user).append(",");
				    cnt++;
				}
			}
			
			if(cnt==usersList.length) {
				return "User desn't exist in user list"; 
			}
			
			newUsers.deleteCharAt(newUsers.length()-1);
			
			StringBuilder sbUpdate = new StringBuilder("update ").append(DBNAME).append(".").append(MEMORYINFOTABLE)
					.append(" SET USERLIST='").append(newUsers).append("'")
					.append(" where MEMORYNAME='").append(memoryName).append("'");
			
			Connection conn = null;
			Statement stmt = null;
			try {
				  conn = mDs.getConnection();
				  stmt = (Statement) conn.createStatement();
		          stmt.executeUpdate(sbUpdate.toString());
		          
		          // update cache with over write here
		          mUserList.put(memoryName, newUsers.toString());
		          
			} finally {
				
				if(stmt != null)
				    stmt.close();
				if(conn != null)
					conn.close();	
			}
		}
		return null;
	}

	/**
	 * 
	 * @param userid
	 * @return
	 * @throws SQLException
	 */
	@Override
	public String listMemories(String userid) throws SQLException {
		StringBuilder sbQuery = new StringBuilder("select MEMORYNAME,USERLIST from ").append(DBNAME).append(".").append(MEMORYINFOTABLE);
		
		Connection conn = null;
		Statement stmt = null;
		ResultSet rs = null;
		
		try {
			
			conn = mDs.getConnection();
			stmt = (Statement) conn.createStatement();
			rs = stmt.executeQuery(sbQuery.toString());
			
			StringBuilder sbMems = new StringBuilder();
			
			while(rs.next()) {
				String users = rs.getString("USERLIST");
				String memory = rs.getString("MEMORYNAME");
				
				if(isUserAllowed(memory, users, userid)) {
					sbMems.append(memory).append(",");
				}
			}

			if(sbMems.length()>0)
				sbMems.deleteCharAt(sbMems.length()-1);
			return sbMems.toString();
			
		} finally {
			if(rs!=null)
				rs.close();
			if(stmt != null)
			    stmt.close();
			if(conn != null)
				conn.close();	
		}
		 
	}

	/**
	 * 
	 * @param memoryName
	 * @param userList
	 * @param user
	 * @return
	 * @throws SQLException
	 */
	private boolean isUserAllowed(String memoryName,String userList, String user) throws SQLException {

		if(isUserAllowed(userList,user))
			return true;
		
		// is creator?
		String creator = getCreatorName(memoryName);
		if(creator != null && creator.equals(user)) {
			return true;
		}
		
		return false;
			
	}
	
	/**
	 * 
	 * @param userList
	 * @param user
	 * @return
	 * @throws SQLException
	 */
	private boolean isUserAllowed(String userList, String user) throws SQLException {
		// 0. any name
		if("*".equals(userList))
			return true;
	
		// 1. in user-list
		String[] users = userList.split(",");
		for(String u:users) {
			if(u.equals(user))
				return true;
		}
		
		return false;	
	}
	
	/**
	 * 
	 * @param memoryName
	 * @return
	 * @throws SQLException
	 */
	private String listMemoryUsers(String memoryName) throws SQLException {
		
		if(mUserList.containsKey(memoryName)) {
		    String users = mUserList.get(memoryName);
		    if(users!=null && !users.isEmpty()) {
		    	return users;
		    }
		}
		
		StringBuilder sbQuery = new StringBuilder("select USERLIST from ").append(DBNAME).append(".").append(MEMORYINFOTABLE)
				.append(" where MEMORYNAME='").append(memoryName).append("'");
		
		Connection conn = null;
		Statement stmt = null;
		ResultSet rs = null;
		
		
		try {
			conn = mDs.getConnection();
			stmt = (Statement) conn.createStatement();
			rs = stmt.executeQuery(sbQuery.toString());
			if(rs.next()) {
				String users = rs.getString("USERLIST");
				return users;
			}
			
		} finally {
			if(rs!=null)
				rs.close();
			if(stmt != null)
			    stmt.close();
			if(conn != null)
				conn.close();	
		}

		return null;
	}

	/**
	 * 
	 * @param memoryname
	 * @param tus
	 * @throws SQLException
	 */
	@Override
	public int upload(String user, String memoryname, List<TU> tus) throws SQLException {
		if(user==null || tus==null || tus.isEmpty() || memoryname.isEmpty())
			return -1;
		
		// if size bigger than 10, use batch update
		long begtime = System.currentTimeMillis();
	    
		int upcounter = batchUpload(user,memoryname,tus);

		StringBuilder sbInfo = new StringBuilder();
		sbInfo.append("batch insert seconds=").append(TimeUnit.MILLISECONDS.toSeconds(System.currentTimeMillis()-begtime));
		mLogger.info(sbInfo.toString());
		
		return upcounter;
	}
	
	/**
	 * 
	 * @param memoryName
	 * @return
	 * @throws SQLException
	 */
	private int getMemoryUpdateCounter(String memoryName) throws SQLException {
	
		StringBuilder sbQuery = new StringBuilder("select MEMORYUPCOUNTER from ").append(DBNAME).append(".").append(MEMORYINFOTABLE)
				.append(" where MEMORYNAME='").append(memoryName).append("'");
		
		Connection conn = null;
		Statement stmt = null;
		ResultSet rs = null;
		int upcounter = -1;
		
		try {
			conn = mDs.getConnection();
			stmt = (Statement) conn.createStatement();
			
			synchronized(UPDATECOUNTERLOCK) {
				rs = stmt.executeQuery(sbQuery.toString());
				if(rs.next()) {
					upcounter = rs.getInt("MEMORYUPCOUNTER");
					
					// update this counter in the data base
					StringBuilder sbUpdate = new StringBuilder("update ").append(DBNAME).append(".").append(MEMORYINFOTABLE)
							.append(" SET MEMORYUPCOUNTER='").append(upcounter+1).append("'")
							.append(" where MEMORYNAME='").append(memoryName).append("'");
					stmt.executeUpdate(sbUpdate.toString());
				}
			}
			
		} finally {
			if(rs!=null)
				rs.close();
			if(stmt != null)
			    stmt.close();
			if(conn != null)
				conn.close();	
		}

		return upcounter;
	}

	/**
	 * 
	 * @param memoryName
	 * @return
	 * @throws SQLException
	 */
	private int readMaxMemoryUpdateCounter(String memoryName) throws SQLException {
	
		StringBuilder sbQuery = new StringBuilder("select MEMORYUPCOUNTER from ").append(DBNAME).append(".").append(MEMORYINFOTABLE)
				.append(" where MEMORYNAME='").append(memoryName).append("'");
		
		Connection conn = null;
		Statement stmt = null;
		ResultSet rs = null;
		int upcounter = -1;
		
		try {
			conn = mDs.getConnection();
			stmt = (Statement) conn.createStatement();
		
			rs = stmt.executeQuery(sbQuery.toString());
			if(rs.next()) {
				upcounter = rs.getInt("MEMORYUPCOUNTER");
			}
			
		} finally {
			if(rs!=null)
				rs.close();
			if(stmt != null)
			    stmt.close();
			if(conn != null)
				conn.close();	
		}

		return upcounter;
	}
	
	
	/**
	 * 
	 * @param memoryname
	 * @param tus
	 * @throws SQLException
	 */
    private int batchUpload(String user, String memoryname, List<TU> tus)  throws SQLException {

		Connection conn = null; 
		PreparedStatement pstmt = null;
		PreparedStatement ptuvstmt = null;
		Statement stmt = null;
		ResultSet rs = null;
		boolean commitMode = true;
		int upcounter = -1;
		try {
			//TODO: "?useServerPrepStmts=false&rewriteBatchedStatements=true";
			conn = mDs.getConnection();
			commitMode = conn.getAutoCommit();
			conn.setAutoCommit(false);
      
			// filter duplicated enabled?
			// maybe lost some concurrence from other users, but just make this as try best method
			// otherwise program will runs slowly
			if( CfgUtil.getInstance().isDuplicatedFilt() ) {
				tus = filterDuplicatedTUS(memoryname,tus);
				if( tus.size()==0 )
					return -1;
			}
			//
			
			upcounter = getMemoryUpdateCounter(memoryname);
			if(-1 == upcounter)
				return -1;
			
			StringBuilder sbTuInsert = new StringBuilder();
			sbTuInsert.append("INSERT INTO ").append(DBNAME).append(".").append(memoryname).append("_tu").append(" VALUES(?,?,?,?,?,?,?,?)");
			pstmt = conn.prepareStatement(sbTuInsert.toString());

			StringBuilder sbLockTable = new StringBuilder("LOCK TABLE ").
                    append(DBNAME).append(".").append(memoryname).append("_TU").append(" write");
			
			for(TU tu:tus) {
				pstmt.setInt(1, 0);
				pstmt.setString(2, tu.getCreatetionDate());
				pstmt.setInt(3, upcounter);
				pstmt.setInt(4, tu.getSegNum());
				pstmt.setString(5, tu.getMarkup());
				pstmt.setString(6, tu.getDocName());
				pstmt.setString(7, tu.getShortDocName());
				pstmt.setInt(8, tu.hashCode());
				pstmt.addBatch();
			}
			
			//lock tu
			stmt = conn.createStatement();
			stmt.execute(sbLockTable.toString());
			pstmt.executeBatch();
			conn.commit();
			// unlock
			stmt.execute("UNLOCK TABLE");
			stmt.close();
			
			int lastid = -1;
			rs = pstmt.executeQuery("SELECT last_insert_id()");
			while(rs.next()) {
				lastid = rs.getInt(1);
			}
			

			
			// insert tuvs
			StringBuilder sbTuvInsert = new StringBuilder();
			sbTuvInsert.append("INSERT INTO ").append(DBNAME).append(".").append(memoryname).append("_TUV").append(" VALUES(?,?,?,?,?,?,?)");
			ptuvstmt = conn.prepareStatement(sbTuvInsert.toString());
			
			int begTuvId = lastid-tus.size()+1;
			for(TU tu:tus) {
				for(TUV tuv:tu.getTUVS()) {
					ptuvstmt.setInt(1,0);
					//ptuvstmt.setString(2, tuv.getCreatetionDate());
					ptuvstmt.setInt(2,upcounter);
					ptuvstmt.setString(3,tuv.getSegment());
					ptuvstmt.setString(4,tuv.getLang());
					ptuvstmt.setString(5,tuv.getLanguage());
					ptuvstmt.setString(6,user);
					ptuvstmt.setInt(7,begTuvId);
					ptuvstmt.addBatch();
				}
				begTuvId++;
			}
			ptuvstmt.executeBatch();
			conn.commit();
			
		}  finally {
			conn.setAutoCommit(commitMode);
			if(rs!=null)
				rs.close();
			if(pstmt!=null)
				pstmt.close();
			if(ptuvstmt!=null)
				ptuvstmt.close();
			if(conn!=null)
				conn.close();
		}
		
		return upcounter;

	}
    
   
    
    /**
     * 
     * @param memoryname
     * @param tu
     * @return
     * @throws SQLException
     */
    private Map<Integer,ArrayList<Integer>> getTUS(String memoryname) throws SQLException{
		Connection conn = null;
		Statement stmttu = null;
		ResultSet rstu = null;

		try {
			conn = mDs.getConnection();
			stmttu = (Statement) conn.createStatement();
			
			//
			StringBuilder sbQueryTUHash = new StringBuilder("select ID,TUHASH from ").append(DBNAME).append(".")
					                     .append(memoryname).append("_TU");
					                                                      		
            Map<Integer,ArrayList<Integer>> res = new HashMap<Integer,ArrayList<Integer>>();
	        rstu = stmttu.executeQuery(sbQueryTUHash.toString());
	        while( rstu.next() ) {

	        	int key = rstu.getInt(2);
	        	if( res.containsKey(key) ) {
	        		ArrayList<Integer> val = res.get(key);
	        		val.add(rstu.getInt(1));
	        	    res.put(key,val);
	        	} else {
	        		ArrayList<Integer> val = new ArrayList<Integer>();
	        		val.add(rstu.getInt(1));
	        		res.put(key, val);
	        	}
	        }
	        rstu.close();
            
	        return res;

		} finally {
			if(rstu!=null)
				rstu.close();
			if(stmttu != null)
			    stmttu.close();
			if(conn != null)
				conn.close();	
		}
	}
    
    private Map<Integer,ArrayList<DbTUV>> getTUVS(String memoryname) throws SQLException{
		Connection conn = null;
		Statement stmttuv = null;
		ResultSet rstuv = null;

		try {
			conn = mDs.getConnection();
			stmttuv = (Statement) conn.createStatement();
			
			//
			StringBuilder sbQueryTUV = new StringBuilder("select PARENTID,SEGMENT,LANG from ").append(DBNAME).append(".")
					                     .append(memoryname).append("_TUV");
					                                                      		
            Map<Integer,ArrayList<DbTUV>> res = new HashMap<Integer,ArrayList<DbTUV>>();
	        rstuv = stmttuv.executeQuery(sbQueryTUV.toString());
	        while( rstuv.next() ) {

	        	int key = rstuv.getInt(1);
	        	if( res.containsKey(key) ) {
	        		ArrayList<DbTUV> val = res.get(key);
	        		val.add( new DbTUV(rstuv.getString(2),rstuv.getString(3)) );
	        	    res.put(key,val);
	        	} else {
	        		ArrayList<DbTUV> val = new ArrayList<DbTUV>();
	        		val.add(new DbTUV(rstuv.getString(2),rstuv.getString(3)) );
	        		res.put(key, val);
	        	}
	        }
	        rstuv.close();
            
	        return res;

		} finally {
			if(rstuv!=null)
				rstuv.close();
			if(stmttuv != null)
			    stmttuv.close();
			if(conn != null)
				conn.close();	
		}
	}
    
    
    
    private List<TU> filterDuplicatedTUS(String memoryName, List<TU> tus) throws SQLException {
    	if( tus.isEmpty())
    		return tus;
    	
    	Map<Integer,ArrayList<Integer>> tusMap = getTUS(memoryName);
    	if(tusMap.isEmpty())
    		return tus;
        
    	Map<Integer,ArrayList<DbTUV>> tuvMap = getTUVS(memoryName);
    	if(tuvMap.isEmpty())
    		return tus;
    	
    	Iterator<TU> iter = tus.iterator();
    	while(iter.hasNext()) {
    		
    		TU tu = iter.next();
    		if(tu.getTUVS().size()>2)
    			continue;
    		
    		int key = tu.hashCode();
    		
    		if( !tusMap.containsKey(key) )
    			continue;
    		
    		ArrayList<Integer> pids = tusMap.get(key);
    		for(int pid:pids) {
        		TUV srcTUV = tu.getSourceTUV();
        		TUV tgtTUV = tu.getTargetTUV().get(0);
        		
        		boolean isSrcDuplicated = false;
        		boolean isTgtDuplicated = false;
        		ArrayList<DbTUV> tuvs = tuvMap.get(pid);
        		for(DbTUV dbtuv:tuvs) {
        			if( dbtuv.isSourceTUV() ) {
        				if( srcTUV.getSegment().equals(dbtuv.segment) ) {
        					isSrcDuplicated = true;
        				}
        			} else {
        				
        				if( tgtTUV.getLang().equalsIgnoreCase(dbtuv.lang) &&
    							tgtTUV.getSegment().equalsIgnoreCase(dbtuv.segment) ) {
        					isTgtDuplicated = true;
    					}
        					
        			}
        			if(isSrcDuplicated && isTgtDuplicated) {
        				iter.remove();
        				break;
        			}
        		}
        		
        		if(isSrcDuplicated && isTgtDuplicated) 
        			break;
        		
    		}//end for

    	}
    	
    	return tus;
    }
    /**
     * 
     * @param memoryname
     * @param updateCounter
     * @return
     * @throws SQLException
     */
	@Override
	public Map<String,String> download(String user, String memoryname, long updateCounter,Set<Long> ownUploadedCounters)
			throws SQLException {
		Map<String,String> resMap = new HashMap<String,String>();
		boolean hasNext = true;
		long curCounter = updateCounter+1;		
		StringBuilder resTmx = new StringBuilder();
		resTmx.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
		      .append("<tmx version=\"1.4\">")
		      .append("<header datatype=\"plaintext\"/><body>");
		
		long maxCurCounter = readMaxMemoryUpdateCounter(memoryname);
		while(hasNext) {
			if(curCounter > maxCurCounter)
				break;
			
			// skip update counter in own uploaded counter list
			if( ownUploadedCounters.contains(curCounter) ) {
				curCounter++;
				continue;
			}
			
			String temp = downloadOneCounter(user, memoryname,curCounter);
            mLogger.info(temp.length()+" "+CfgUtil.getInstance().getDonwloadSize());
			if(temp!=null && !"".equals(temp)) {
				if( (resTmx.length()+temp.length())<CfgUtil.getInstance().getDonwloadSize()) {
					resTmx.append(temp);
				    curCounter++;
				    continue;
				}
			}
			hasNext = false;
		}
		
		resTmx.append("</body></tmx>");

		resMap.put(SyncParameters.TMXDOCUMENT, resTmx.toString());
		resMap.put(SyncParameters.UPDATECOUNTER,Long.toString(curCounter-1));

		return resMap;
	}
	
	/**
	 * 
	 * @param memoryname
	 * @param updateCounter
	 * @return
	 * @throws SQLException
	 */
	private String downloadOneCounter(String user, String memoryname, long updateCounter) throws SQLException{
		Connection conn = null;
		Statement stmttu = null;
		Statement stmttuv = null;
		ResultSet rstu = null;
		ResultSet rstuv = null;
		try {
			conn = mDs.getConnection();
			stmttuv = (Statement) conn.createStatement();
			stmttu = (Statement) conn.createStatement();
			
			StringBuilder sbQueryTUV = new StringBuilder("select * from ").append(DBNAME).append(".").append(memoryname).append("_TUV")
					.append(" where UPDATECOUNTER=").append(updateCounter);
			
			rstuv = stmttuv.executeQuery(sbQueryTUV.toString());
	
			// cache for TU
			HashMap<Integer,StringBuilder> tuCache= new HashMap<Integer,StringBuilder>();
			
			while(rstuv.next()) {

				StringBuilder tuvs = new StringBuilder();
				tuvs.append("<tuv ").append("xml:lang=\"").append(rstuv.getString(4)).append("\">");
				tuvs.append("<prop type=\"tmgr-language\">").append(rstuv.getString(5)).append("</prop>");
				char[] encodedStr =  SimpleBase64.encode(rstuv.getString(3).getBytes());
				tuvs.append("<seg>").append(encodedStr).append("</seg>");
				tuvs.append("</tuv>");
			
				int parentID = rstuv.getInt(7);
				if(!tuCache.containsKey(parentID)) {
					
					StringBuilder sbQueryTU = new StringBuilder("select * from ").append(DBNAME).append(".").append(memoryname).append("_TU")
							.append(" where ID=").append(parentID);

					rstu = stmttu.executeQuery(sbQueryTU.toString());
					while(rstu.next()) {
						StringBuilder tus = new StringBuilder();
						tus.append("<tu tuid=\"").append(rstu.getInt(1)).append("\" creationdate=\"").append(rstu.getString(2)).append("\">");
						tus.append("<prop type=\"tmgr-segNum\">").append(rstu.getInt(4)).append("</prop>");
						tus.append("<prop type=\"tmgr-markup\">").append(rstu.getString(5)).append("</prop>");
						tus.append("<prop type=\"tmgr-docname\">").append(rstu.getString(6)).append("</prop>");
						tus.append("<prop type=\"tmgr-short-docname\">").append(rstu.getString(7)).append("</prop>");
						
						tuCache.put(parentID, tus);
					}
					rstu.close();
				}
				
				StringBuilder sbTU = tuCache.get(parentID);
				sbTU.append(tuvs);
			}
			
			StringBuilder res = new StringBuilder();
			Iterator<Entry<Integer, StringBuilder>> iter = tuCache.entrySet().iterator();
			while(iter.hasNext()) {
				res.append(iter.next().getValue()).append("</tu>");
				
			}
			
			return res.toString();
			
		} finally {
			if(rstu!=null)
				rstu.close();
			if(stmttu != null)
			    stmttu.close();
			if(rstuv!=null)
				rstuv.close();
			if(stmttuv != null)
			    stmttuv.close();
			if(conn != null)
				conn.close();	
		}	
	}

	/**
	 * back data base
	 * @param dbname
	 * @param outname
	 * @return
	 * @throws SQLException
	 */
	@Override
	public boolean backup(String outname) {
		StringBuilder sbBackup = new StringBuilder();
		sbBackup.append("cmd /C mysqldump.exe  -u root ")
		                    .append("--add-drop-table ")
					        .append("--quick ")
					        .append("--compress ")
					        .append("--extended-insert=false ")
					        .append("--default-character-set=utf8 ")
					        .append(DBNAME)
					        .append(">").append(outname);
		
        RunCommand  rc = new RunCommand();
        int res = rc.backupCmd(sbBackup.toString());
        
        if(res == 1)
        	return false;
        
		return true;
	}

	

	
	@Override
	public boolean restore(String infile) {
		 try {
			createDB();
			
			StringBuilder sbRestore =  new StringBuilder();
			sbRestore.append("cmd /C mysql  -u root ").append(DBNAME).append("<").append(infile);
			
			RunCommand  rc = new RunCommand();
		    int res = rc.restoreCmd(sbRestore.toString());
		        
		    if(res == 1)
		    	return false;
		    
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (PropertyVetoException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		
	        
			return true;
	}

	@Override
	public void createUser(String username, String password, String IP,String port) throws SQLException, ClassNotFoundException {
		
		Connection conn = null; 
		Statement stmt = null;

		//conn = mDs.getConnection();	
		try {
			
// it's called before initialize ConnectionPool
// So use raw connection here
String dbClass = CfgUtil.getInstance().getDbCfg().get("driver_class");
Class.forName(dbClass);

StringBuilder sbUrl = new StringBuilder();
if("org.mariadb.jdbc.Driver".equals(dbClass)) {
	sbUrl.append("jdbc:mariadb://localhost:").append(port);
} else if ("com.mysql.jdbc.Driver".equals(dbClass)) {
	sbUrl.append("jdbc:mysql://localhost:").append(port);
}

String rootPassword =  CfgUtil.getInstance().getDbCfg().get("root_password");
conn = DriverManager.getConnection(sbUrl.toString(), "root", rootPassword);
if(!"localhost".equals(IP) && !"127.0.0.1".equals(IP))
	IP = "%";


			if(conn != null)
				stmt = (Statement) conn.createStatement();
			
			// create database
			if(stmt != null) {
				StringBuilder sbQuery = new StringBuilder();
				// select host,user from mysql.user;
				sbQuery.append("SELECT host,user from mysql.user where user='").append(username).append("' and host='").append(IP).append("'");
				ResultSet rs = stmt.executeQuery(sbQuery.toString());

				if(!rs.next()) {
					
					StringBuilder sbCmd = new StringBuilder();
					sbCmd.append("CREATE USER '").append(username).append("'@'").append(IP).append("' IDENTIFIED BY '").append(password).append("'");
					stmt.execute(sbCmd.toString());
					
					StringBuilder sbGrant = new StringBuilder();
					sbGrant.append("GRANT ALL ON *.* TO '").append(username).append("'@'").append(IP).append("'");
					stmt.execute(sbGrant.toString());	
					
					mLogger.info("create new database account "+username);
				}
				
				if(rs != null)
					rs.close();
			
			}
		}
		finally {
			if(stmt != null)
				stmt.close();
			if(conn != null)
				conn.close();
		}
	}
	
	/**
	 * store important TUV information in DB
	 * @author ADMINIBM
	 *
	 */
	private class DbTUV {
		public String segment;
		public String lang;
		
		public DbTUV(String seg,String lng){
			segment = seg;
			lang = lng;
		}
		
		public boolean isSourceTUV() {
			return  "en-US".equalsIgnoreCase(lang);
		}
	}
		
}
