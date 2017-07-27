//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.webservice;

import java.beans.PropertyVetoException;
import java.io.IOException;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import javax.xml.parsers.ParserConfigurationException;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;

import com.otm.db.Dao;
import com.otm.db.DaoFactory;
import com.otm.db.TU;
import com.otm.log.ServiceLogger;
import com.otm.util.SyncParameters;
import com.otm.util.TmxSaxParser;

// this class should be thread safe
// it's a single instance

public class OtmTMServiceSynchronizes {
	
	// an immutable object
	private final Dao mDao = DaoFactory.getInstance();
	//private final Object mLockObj = new Object();
    //private volatile boolean mDbExisted = false;
	private static Logger mLogger = Logger.getLogger(ServiceLogger.class.getName());
	
	/**
	 * 
	 * @param paramHash
	 * @return
	 */
	public HashMap<String, String> create(HashMap<String, String> paramHash) {
		HashMap<String, String> result = new HashMap<String, String>();

		try {
			String memoryname = paramHash.get(SyncParameters.DATASOURCENAME);
			String user = paramHash.get(SyncParameters.USER_ID);
			String password = paramHash.get(SyncParameters.PASSWORD);
            String userList = paramHash.get(SyncParameters.USERIDLIST);
           
            // to compatible for older version "userIdList"
            if(null==userList || userList.isEmpty()) {
            	userList = paramHash.get("user-id-list");
            }
            // to compatible for older version ";" to ","
            if(userList!=null && !userList.isEmpty()) {
            	userList = userList.replaceAll(";",",");
            }
            
			result.put("method",  paramHash.get(SyncParameters.METHOD));
			result.put(SyncParameters.DATASOURCENAME, memoryname);
           
			try {
				mDao.createMemory(memoryname, user,password,userList);
			} catch (PropertyVetoException e) {
				e.printStackTrace();
				result.put(SyncParameters.RESPONSESTATUSMSG, e.getMessage());
				mLogger.error(result);
				return result;
			}

		} catch (SQLException e) {
			result.put(SyncParameters.RESPONSESTATUSMSG, e.getMessage());
			mLogger.error(result);
			return result;
		}

		result.put(SyncParameters.RESPONSESTATUSMSG, "success");
		mLogger.info(result);
		return result;
	}
	
	
	/**
	 * 
	 * @param paramHash
	 * @return
	 */
	public HashMap<String, String> delete(HashMap<String, String> paramHash) {
		HashMap<String, String> result = new HashMap<String, String>();
		String userid = paramHash.get(SyncParameters.USER_ID);
		String memoryName = paramHash.get(SyncParameters.DATASOURCENAME);
		String user = paramHash.get(SyncParameters.USER_ID);
		
		result.put("method",  paramHash.get(SyncParameters.METHOD));
		result.put(SyncParameters.DATASOURCENAME, memoryName);

		try {
			// no user accessed memory, returned directly
			String memories = mDao.listMemories(user);
			if(memories==null || memories.indexOf(memoryName)==-1 ) {
				result.put(SyncParameters.RESPONSESTATUSMSG, "no such memory existed or user can't access to it");
				mLogger.error(result);
				return result;
			}	
						
			String creatorName = mDao.getCreatorName(memoryName);
			//mLogger.info(creatorName);
			if(creatorName!=null && creatorName.equals(userid)) {
				mDao.deleteMemory(memoryName);
			}
		} catch (SQLException e) {
			e.printStackTrace();
			result.put(SyncParameters.RESPONSESTATUSMSG,e.getMessage());
			mLogger.error(result);
			return result;
		}
		
		result.put(SyncParameters.RESPONSESTATUSMSG, "success");
		mLogger.info(result);
		
		return result;
	}
	
	
	/**
	 * 
	 * @param paramHash
	 * @return
	 */
	public HashMap<String, String> listUser(HashMap<String, String> paramHash) {
		HashMap<String, String> result = new HashMap<String, String>();
		
		String memoryName = paramHash.get(SyncParameters.DATASOURCENAME);
		String user = paramHash.get(SyncParameters.USER_ID);
		result.put("method", paramHash.get(SyncParameters.METHOD));
		result.put(SyncParameters.DATASOURCENAME, memoryName);
		
		mLogger.debug(result);
		
		try {
			// no user accessed memory, returned directly
			String memories = mDao.listMemories(user);
			if(memories==null || memories.indexOf(memoryName)==-1 ) {
				result.put(SyncParameters.RESPONSESTATUSMSG, "no such memory existed or user can't access to it");
                mLogger.error(result);
				return result;
			}	
			
			String users = mDao.getUserList(memoryName);
			result.put(SyncParameters.USERIDLIST,users);
			
			// to compatible with old version, also passed this
            result.put("user-id-list",users);          

		} catch (SQLException e) {
		    result.put(SyncParameters.RESPONSESTATUSMSG, e.getMessage());
			e.printStackTrace();
			mLogger.error(result);
			return result;
		}
		
		result.put(SyncParameters.RESPONSESTATUSMSG, "success");
		mLogger.info(result);
		
		return result;
		
	}

	
	/**
	 * 
	 * @param paramHash
	 * @return
	 */
	public HashMap<String, String> addUser(HashMap<String, String> paramHash) {
		HashMap<String, String> result = new HashMap<String, String>();
		
		String memoryName = paramHash.get(SyncParameters.DATASOURCENAME);
		String userToAdd = paramHash.get(SyncParameters.USERTOADD);
		String user = paramHash.get(SyncParameters.USER_ID);
		result.put("method", paramHash.get(SyncParameters.METHOD));
		result.put(SyncParameters.DATASOURCENAME, memoryName);

		try {
			
			// no user accessed memory, returned directly
			String memories = mDao.listMemories(user);
			if(memories==null || memories.indexOf(memoryName)==-1 ) {
				result.put(SyncParameters.RESPONSESTATUSMSG, "no such memory existed or user can't access to it");
                mLogger.error(result);
				return result;
			}	
			
			String res = mDao.addUser(memoryName, userToAdd);
			if(res != null) {
				result.put(SyncParameters.RESPONSESTATUSMSG, res);
				mLogger.error(result);
				return result;
			}
		} catch (SQLException e) {
		    result.put(SyncParameters.RESPONSESTATUSMSG, e.getMessage());
		    mLogger.error(result);
			e.printStackTrace();
			return result;
		}
		
		result.put(SyncParameters.RESPONSESTATUSMSG, "success");
		mLogger.info(result);
		return result;
		
	}
	
	
	/**
	 * 
	 * @param paramHash
	 * @return
	 */
	public HashMap<String, String> removeUser(HashMap<String, String> paramHash) {
		HashMap<String, String> result = new HashMap<String, String>();
		
		String memoryName = paramHash.get(SyncParameters.DATASOURCENAME);
		String userToRemove = paramHash.get(SyncParameters.USERTOREMOVE);
		String user = paramHash.get(SyncParameters.USER_ID);
		result.put("method", paramHash.get(SyncParameters.METHOD));
		result.put(SyncParameters.DATASOURCENAME, memoryName);
		
		try {
			// no user accessed memory, returned directly
			String memories = mDao.listMemories(user);
			if(memories==null || memories.indexOf(memoryName)==-1 ) {
				result.put(SyncParameters.RESPONSESTATUSMSG, "no such memory existed or user can't access to it");
				mLogger.error(result);
				return result;
			}	
						
			String res = mDao.removeUser(memoryName, userToRemove);
			if(res != null) {
				result.put(SyncParameters.RESPONSESTATUSMSG, res);
				mLogger.error(result);
				return result;
			}
		} catch (SQLException e) {
		    result.put(SyncParameters.RESPONSESTATUSMSG, e.getMessage());
		    mLogger.error(result);
			e.printStackTrace();
			return result;
		}
		
		result.put(SyncParameters.RESPONSESTATUSMSG, "success");
		mLogger.info(result);
		return result;
		
	}
	
	
	public HashMap<String, String> listMemories(HashMap<String, String> paramHash) {
		HashMap<String, String> result = new HashMap<String, String>();
		
		String userid = paramHash.get(SyncParameters.USER_ID);
		
		result.put("method", paramHash.get(SyncParameters.METHOD));
		result.put("userid", userid);
  
		try {
			String allMems = mDao.listMemories(userid);
			if(allMems !=null && !allMems.isEmpty()) {
				result.put(SyncParameters.MEMORYLIST, allMems);
			} else {
				result.put(SyncParameters.RESPONSESTATUSMSG, "There is no memory that "+ userid+" could accessed");
				mLogger.error(result);
				return result;
			}
		} catch (SQLException e) {
		    result.put(SyncParameters.RESPONSESTATUSMSG, e.getMessage());
			e.printStackTrace();
			mLogger.error(result);
			return result;
		}
		
		result.put(SyncParameters.RESPONSESTATUSMSG, "success");
		mLogger.info(result);
		return result;
		
	}

	
	public HashMap<String, String> upload(HashMap<String, String> paramHash) {
		HashMap<String, String> result = new HashMap<String, String>();
	
		String memoryname = paramHash.get(SyncParameters.DATASOURCENAME);
		String contents = paramHash.get("tmx-document");
		mLogger.debug(contents);
		String user = paramHash.get(SyncParameters.USER_ID);
		
		result.put("method", paramHash.get(SyncParameters.METHOD));
		result.put(SyncParameters.DATASOURCENAME, memoryname);
        
		// call 
		if(contents.isEmpty()) {
			result.put(SyncParameters.RESPONSESTATUSMSG, "no contents passed");
		} else {
			
			try {
				// no user accessed memory, returned directly
				String memories = mDao.listMemories(user);
//System.out.println(user+"--->"+memories);
				if(memories==null || memories.indexOf(memoryname)==-1 ) {
					result.put(SyncParameters.RESPONSESTATUSMSG, "no such memory existed or user can't access to it");
					mLogger.error(result);
					return result;
				}	
				
				TmxSaxParser tmx = new TmxSaxParser();
			    List<TU> tus = tmx.parseTmxForTU(contents,false);
			    mLogger.info(tus.size());
                long upcounter = mDao.upload(user, memoryname, tus);
                
                if( upcounter == -1)
                	result.put(SyncParameters.LOADEDCOUNTER, "");
                else 
                	result.put(SyncParameters.LOADEDCOUNTER, String.valueOf(upcounter));
			    result.put(SyncParameters.RESPONSESTATUSMSG, "success");
			    
			} catch ( SQLException | ParserConfigurationException |SAXException |IOException e) {
				result.put(SyncParameters.RESPONSESTATUSMSG, e.getMessage());
				e.printStackTrace();
				mLogger.error(result);
			}
		}

		mLogger.info(result);
		return result;
	}
	
	public HashMap<String, String> download(HashMap<String, String> paramHash) {
		HashMap<String, String> result = new HashMap<String, String>();
		
		String userid = paramHash.get(SyncParameters.USER_ID);
		String memoryName = paramHash.get(SyncParameters.DATASOURCENAME);
		String updateCounter = paramHash.get(SyncParameters.UPDATECOUNTER);
		
		result.put("method", paramHash.get(SyncParameters.METHOD));
		result.put("userid", userid);
		result.put(SyncParameters.DATASOURCENAME, memoryName);
		
		// get thread's own uploaded counter list
		HashSet<Long> ownUploadedCounters = new HashSet<Long>();
		String strUploadedCounters = paramHash.get(SyncParameters.OWNUPLOADCOUNTERS);
        if( strUploadedCounters!=null && !strUploadedCounters.isEmpty() ) {
			String []arrs = strUploadedCounters.split(",");
			for(String cp:arrs) {
				if(cp.isEmpty())
					continue;
				ownUploadedCounters.add(Long.valueOf(cp));
			}
        }
		
		
		mLogger.info(result);
		
		try {
			// no user accessed memory, returned directly
			String memories = mDao.listMemories(userid);
			if(memories==null || memories.indexOf(memoryName)==-1 ) {
				result.put(SyncParameters.RESPONSESTATUSMSG, "no such memory existed or user can't access to it");	
				mLogger.error(result);
				return result;
			}	
			
			Map<String,String> tmxStr = mDao.download(userid, memoryName,Long.valueOf(updateCounter),ownUploadedCounters);
			if(!tmxStr.isEmpty()) {

				result.put(SyncParameters.TMXDOCUMENT, tmxStr.get(SyncParameters.TMXDOCUMENT));
				result.put(SyncParameters.UPDATECOUNTER, tmxStr.get(SyncParameters.UPDATECOUNTER));
			} else {
				result.put(SyncParameters.RESPONSESTATUSMSG, "No segments downloaded");
				mLogger.debug(result);
				return result;
			}
		} catch (SQLException e) {
		    result.put(SyncParameters.RESPONSESTATUSMSG, e.getMessage());
		    mLogger.error(result);
			e.printStackTrace();
			return result;
		}
		
		result.put(SyncParameters.RESPONSESTATUSMSG, "success");
		mLogger.debug(result);
		
		return result;
		
	}
}
