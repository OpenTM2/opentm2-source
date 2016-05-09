//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

import java.io.File;
import java.net.URI;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

public class CfgUtil {
    
	private Map<String, String> dbCfg  = new HashMap<String, String>();;
	private String url = null;
	private  int downloadsize= 0;
	
	private static CfgUtil instance = null;
	
	public static synchronized CfgUtil getInstance() {
		if(instance == null) {
			instance = new CfgUtil();
		}
		return instance;
	}
	
	public int getDonwloadSize() {
		return downloadsize;
	}
	
	public String getUrl() {
		return url;
	}
	
	public Map<String,String> getDbCfg() {
		return dbCfg;
	}
	
	private CfgUtil() {
		loadCfg();
	}
	
	private  Document loadXmlFile(File newFile) {
		String filename = newFile.getAbsolutePath();

		SAXBuilder builder = new SAXBuilder(false);
		builder.setValidation(false);
		builder.setFeature("http://xml.org/sax/features/validation", false);
		builder.setFeature(
				"http://apache.org/xml/features/nonvalidating/load-dtd-grammar",
				false);
		builder.setFeature(
				"http://apache.org/xml/features/nonvalidating/load-external-dtd",
				false);
		builder.setExpandEntities(true);

		Document document = null;
		try {
			File file = new File(filename);
			if (file.length() > 0) {
				URI uri = file.toURI();
				document = builder.build(uri.toURL());
			}
		} catch (JDOMException e) {
			e.printStackTrace();
			document = null;
			return document;
		} catch (Exception ex) {
			ex.printStackTrace();
			document = null;
			return document;
		}

		return document;
	}


	private  void loadCfg() {
        
		Document doc = loadXmlFile(new File("./configure/service_cfg.xml"));
		if (doc == null) {
			return;
		}

		Element root = doc.getRootElement();
		
		// db configure
		List<?> dbcfgs = root.getChildren("db");
		if (dbcfgs != null) {
			Element element = (Element) dbcfgs.get(0);

			List<?> children = element.getChildren();
			for (Object child : children) {
				Element ch = (Element) child;
				dbCfg.put(ch.getAttribute("name").getValue(), ch.getValue());
			}
		}
		
		// url
		List<?> urls = root.getChildren("url");		
		for(Object obj:urls) {
			Element element = (Element)obj;
			if(element == null)
				continue;
			System.out.println(element.getValue());
			url =  element.getValue();
//System.out.println(url);				
		}
		
		// download_size
		List<?> downloads = root.getChildren("downloadsize");		
		for(Object obj:downloads) {
			Element element = (Element)obj;
			if(element == null)
				continue;
			String dwdsize = element.getValue();
			downloadsize = Integer.valueOf(dwdsize)*1024*1024;
//System.out.println(dwdsize);	
		}
		
	}
	
}
