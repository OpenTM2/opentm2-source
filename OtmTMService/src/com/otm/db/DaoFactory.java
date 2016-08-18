//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.db;


import java.util.Map;



import com.otm.util.CfgUtil;

public class DaoFactory {
    private static Dao instance = null;
    
    public  static synchronized Dao getInstance() {
        if(instance==null) {
        	instance = newDaoInstance();
        }
        return instance;
    }
    
	private static Dao newDaoInstance() {
		
		Map<String, String> dbcfg = CfgUtil.getInstance().getDbCfg();
		
		if( "com.mysql.jdbc.Driver".equals(dbcfg.get("driver_class")) ) {
			return new DaoMysqlImpl(dbcfg);
		}

		return null;
	}
}
