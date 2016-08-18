//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.test;

import java.beans.PropertyVetoException;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.net.MalformedURLException;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.otm.util.SyncParameters;
import com.otm.webservice.OtmTMService;
import com.otm.webservice.OtmTMServiceServer;

import flexjson.JSONSerializer;

public class SyncTest {
    
	public final ExecutorService mEs = Executors.newCachedThreadPool();
	
	public void create(OtmTMService tms, HashMap<String, String> paramHash) {
		
		JSONSerializer serializer = new JSONSerializer();
		String params = serializer.deepSerialize(paramHash);
        String res = tms.synchronize(params);
        
        System.out.println(res);
	}
	
    public void delete(OtmTMService tms, HashMap<String, String> paramHash) {
		
		JSONSerializer serializer = new JSONSerializer();
		String params = serializer.deepSerialize(paramHash);
        String res = tms.synchronize(params);
        
        System.out.println(res);
	}

    public void download(OtmTMService tms) {
    	final HashMap<String, String> paramHash = new HashMap<String, String>();
    	paramHash.put("method", "download");
		paramHash.put(SyncParameters.USER_ID, "wlp");
		paramHash.put("dataSourcePassword", "456");
		paramHash.put("update-counter", "0");
		paramHash.put(SyncParameters.DATASOURCENAME,"mytest7");
	
		JSONSerializer serializer = new JSONSerializer();
		String params = serializer.deepSerialize(paramHash);
		String res = tms.synchronize(params);
		
		System.out.println(res);
    }
    
    
	public void create_multithread(final OtmTMService tms) throws InterruptedException {
		
		for(int i=0; i<10; i++) {
			final HashMap<String, String> paramHash = new HashMap<String, String>();
	    	paramHash.put("method", "create");
			paramHash.put(SyncParameters.USER_ID, "wliping");
			paramHash.put("dataSourcePassword", "456");
			paramHash.put(SyncParameters.DATASOURCENAME,"mytest");
			paramHash.put("dataSourceType","com.mysql.jdbc.Driver");
			paramHash.put("dataSourceServer","localhost");
			paramHash.put("dataSourcePort","3306");
			paramHash.put("user-id-list", "*");
			
			paramHash.put(SyncParameters.DATASOURCENAME, "mytest"+Integer.toString(i));

			Runnable thread = new Runnable() {
				public void run() {
					create(tms, paramHash);
				}
			};
			
			mEs.submit(thread);
		}

	}
	
    public void delete_multithread(final OtmTMService tms) throws InterruptedException {
		
		for(int i=0; i<10; i++) {
			final HashMap<String, String> paramHash = new HashMap<String, String>();
	    	paramHash.put("method", "delete");
			paramHash.put("user-id", "wliping");
			paramHash.put("password", "456");

//			paramHash.put("dataSourceName","mytest");
//			paramHash.put("dataSourceType","com.mysql.jdbc.Driver");
//			paramHash.put("dataSourceServer","localhost");
//			paramHash.put("dataSourcePort","3306");
//			paramHash.put("user-id-list", "*");
			
			paramHash.put(SyncParameters.DATASOURCENAME, "mytest"+Integer.toString(i));

			Runnable thread = new Runnable() {
				public void run() {
					create(tms, paramHash);
				}
			};
			
			mEs.submit(thread);
		}

	}

	
    public void users(OtmTMService tms) {
    	final HashMap<String, String> paramHash = new HashMap<String, String>();
    	
    	paramHash.put("method", "adduser");
		paramHash.put("userid", "wlp");
		paramHash.put(SyncParameters.USERTOADD, "Tee");
		paramHash.put(SyncParameters.DATASOURCENAME,"mytest4");
		
		JSONSerializer serializer = new JSONSerializer();
		String params = serializer.deepSerialize(paramHash);
        String res = tms.synchronize(params);
        System.out.println(res);
        
        paramHash.put("method", "removeuser");
		paramHash.put("userid", "wlp");
		paramHash.put(SyncParameters.USERTOREMOVE, "Tee");
		params = serializer.deepSerialize(paramHash);
        res = tms.synchronize(params);
        System.out.println(res);
        
        paramHash.put("method", "listuser");
		paramHash.put("userid", "wlp");
		params = serializer.deepSerialize(paramHash);
        res = tms.synchronize(params);
        System.out.println(res);
        
        paramHash.put("method", "listallmemories");
		paramHash.put(SyncParameters.USER_ID, "wlp");
		params = serializer.deepSerialize(paramHash);
        res = tms.synchronize(params);
        System.out.println(res);
	}
 
    public void users_multithread(final OtmTMService tms) throws InterruptedException {
	    final HashMap<String, String> paramHash_add = new HashMap<String, String>();
	    paramHash_add.put("method", "adduser");
	    paramHash_add.put("userid", "wlp");
	    paramHash_add.put(SyncParameters.USERTOADD, "Tele");
	    paramHash_add.put(SyncParameters.DATASOURCENAME,"mytest4");
		
		Runnable thread_add = new Runnable() {
			public void run() {
				//create(tms, paramHash);
			    JSONSerializer serializer = new JSONSerializer();
				String params = serializer.deepSerialize(paramHash_add);
		        String res = tms.synchronize(params);
		        System.out.println(res);
			}
		};
		mEs.submit(thread_add);
		
		Thread.sleep(10);
		
		final HashMap<String, String> paramHash_list = new HashMap<String, String>();
		paramHash_list.put("method", "listuser");
		paramHash_list.put("userid", "wlp");
		paramHash_list.put(SyncParameters.DATASOURCENAME,"mytest4");
		
		Runnable thread_list = new Runnable() {
			public void run() {
				//create(tms, paramHash);
			    JSONSerializer serializer = new JSONSerializer();
				String params = serializer.deepSerialize(paramHash_list);
		        String res = tms.synchronize(params);
		        System.out.println(res);
			}
		};
		
		
		mEs.submit(thread_list);
    }
    

    @SuppressWarnings("resource")
	public HashMap<String,String> fillUploadParameter(String name, String fname) {
		BufferedReader br = null;
		StringBuilder sbTmx = new StringBuilder();
		try {
			br = new BufferedReader(new FileReader(new File(fname)));
			String tempLine = null;
			while((tempLine=br.readLine())!=null) {
				sbTmx.append(tempLine);
			}
		} catch (FileNotFoundException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

	
		HashMap<String, String> param = new HashMap<String, String>();
		param.put("method", "upload");
		param.put("user-id", "wliping");
		param.put("password", "123");
		param.put(SyncParameters.DATASOURCENAME, name);
		param.put("tmx-document", sbTmx.toString());
		param.put("encoding", "UTF-8");
		//param.put("update-counter", "");
		
		return param;
	}
    
    public void multithread_upload(final String mname,final OtmTMService tms) {

		for (int i = 1; i <= 1; i++) {
			final String fname = "./testdata/IAM.tmx";// + Integer.toString(i) + ".tmx";

			Runnable thread = new Runnable() {
				public void run() {
					HashMap<String, String> param = fillUploadParameter(mname, fname);
					JSONSerializer serializer = new JSONSerializer();
					String params = serializer.deepSerialize(param);
					//System.out.println(params);
					String result = tms.synchronize(params);
					System.out.println("result: " + result);
					try {
						Thread.sleep(1000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			};

			mEs.submit(thread);
		}
        try {
			mEs.awaitTermination(10000, TimeUnit.SECONDS);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
    
	public static void main(String[] args) throws MalformedURLException, InterruptedException, SQLException, PropertyVetoException {
		SyncTest synctest = new SyncTest();
    	OtmTMServiceServer service = new OtmTMServiceServer();
    	OtmTMService tms = service.start();
 	
  	//synctest.create_multithread(tms);
    	//synctest.delete_multithread(tms);	
    	//long cur = System.currentTimeMillis();
synctest.multithread_upload("mytest0", tms);
    	//synctest.users(tms);
    	//synctest.users_multithread(tms);
//        synctest.download(tms);
       // System.out.println(System.currentTimeMillis()-cur);
    	//Thread.sleep(100000);
        //service.stop();
        
	}

}
