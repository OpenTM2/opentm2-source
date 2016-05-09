//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.webservice;

import java.util.HashMap;

import javax.jws.WebParam;
import javax.jws.WebService;
import javax.jws.soap.SOAPBinding;

import org.apache.log4j.Logger;

import com.otm.log.ServiceLogger;
import com.otm.util.SyncParameters;

import flexjson.JSONDeserializer;
import flexjson.JSONSerializer;


@WebService(endpointInterface = "com.otm.webservice.OtmTMService")
@SOAPBinding(style = SOAPBinding.Style.RPC)
public class OtmTMServiceImpl implements OtmTMService {

	// synchronize called by many clients, maybe at the same time
	// it delegates works to OtmTMServiceSynchronizes
	// must be thread safe object
	private final OtmTMServiceSynchronizes mSync = new OtmTMServiceSynchronizes();
	private static Logger mLogger = Logger.getLogger(ServiceLogger.class.getName());
    
	@Override
	public String synchronize(@WebParam(name = "parameters") String parameters) {
		HashMap<String, String> paramHash = new HashMap<String, String>();
		HashMap<String, String> resultHash = new HashMap<String, String>();
		
//System.out.println(parameters);	
		mLogger.debug(parameters);
		
        if(null==parameters) {
//System.out.println("null parameters");
            mLogger.error("received null parameters");
			resultHash.put(SyncParameters.RESPONSESTATUSMSG, "empty parameters send");
			return serialize(resultHash);
        }
        
		paramHash = new JSONDeserializer<HashMap<String, String>>()
				.deserialize(parameters);

		String method = paramHash.get("method");
		if (method == null || method.isEmpty()) {
			resultHash.put(SyncParameters.RESPONSESTATUSMSG, "empty method name");
			return serialize(resultHash);
		}

		// begin to call detail method
		if("upload".equals(method)) {
			resultHash = mSync.upload(paramHash);
			
		} else if("download".equals(method)) {
			resultHash = mSync.download(paramHash);
			
		}else if("create".equals(method)) {
			resultHash = mSync.create(paramHash);

			
		} else if("delete".equals(method)) {
			resultHash = mSync.delete(paramHash);
			
		} else if("listuser".equals(method)) {
			resultHash = mSync.listUser(paramHash);
			
		} else if("adduser".equals(method)) {
			resultHash = mSync.addUser(paramHash);
			
		}  else if("removeuser".equals(method)) {
			resultHash = mSync.removeUser(paramHash);
			
		}  else if("listallmemories".equals(method)) {
			resultHash = mSync.listMemories(paramHash);
			
		}
		
		return serialize(resultHash);

	}

	private String serialize(HashMap<String, String> map) {
		JSONSerializer serializer = new JSONSerializer();
		String result = serializer.deepSerialize(map);
		return result;
	}

}