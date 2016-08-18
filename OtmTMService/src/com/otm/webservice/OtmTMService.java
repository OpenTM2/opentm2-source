//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.webservice;

import javax.jws.WebParam;
import javax.jws.WebResult;
import javax.jws.WebService;  
import javax.jws.WebMethod;  
import javax.jws.soap.SOAPBinding;  



@WebService
@SOAPBinding(style=SOAPBinding.Style.RPC)  
public interface OtmTMService {

	@WebMethod
	@WebResult(partName = "return")
	public String synchronize(@WebParam(name = "parameters") String parameters);
}
