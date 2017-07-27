//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.dom4j.DocumentHelper;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

public class CfgUtil {
    
	private Map<String, String> dbCfg  = new HashMap<String, String>();;
	private String url = null;
	private  int downloadsize= 0;
	private boolean filtDuplicated = false;
	
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
	
	public boolean isDuplicatedFilt() {
		return filtDuplicated;
	}
	
	public String getUrl() {
		String ipAddress = dbCfg.get("server_ip");
		if(null!=ipAddress && !ipAddress.isEmpty()) {
			StringBuilder sbURL = new StringBuilder();
			sbURL.append("http://")
			     .append(ipAddress)
			     .append(":8085/tmservice");
			url = sbURL.toString();
		}
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

		
		//<url name="service_url">http://127.0.0.1:8085/tmservice</url>
		getUrl();
		
		// download_size
		List<?> downloads = root.getChildren("downloadsize");		
		for(Object obj:downloads) {
			Element element = (Element)obj;
			if(element == null)
				continue;
			String dwdsize = element.getValue();
			downloadsize = Integer.valueOf(dwdsize)*1024*1024;	
		}
		
		//filtDuplicated
		List<?> duplicates = root.getChildren("filtduplicated");
		if(!duplicates.isEmpty()) {
			Element element = (Element)duplicates.get(0);
			if(element != null) {
				String temp = element.getValue();
				filtDuplicated = Boolean.valueOf(temp);
			}	
		}
		
	}
	
	public boolean saveToXml() {
		org.dom4j.Document doc = DocumentHelper.createDocument();
		org.dom4j.Element root = doc.addElement("service_cfg");
		
		root.addElement("downloadsize")
			.addAttribute("name", "maxsize_per_time")
			.addText(String.valueOf(downloadsize/(1024*1024)));
		
		root.addElement("filtduplicated")
		    .addAttribute("name", "filter_duplicate_segment")
		    .addText(String.valueOf(filtDuplicated));
		
		org.dom4j.Element dbElement = root.addElement("db");
		
		ArrayList<Entry<String, String>> dbCfgItems = new ArrayList<Entry<String, String>>(dbCfg.entrySet());
		Collections.sort(dbCfgItems, new Comparator<Entry<String, String>>(){
			@Override
			public int compare(Entry<String, String> o1,
					Entry<String, String> o2) {
				return o1.getKey().compareTo(o2.getKey());
			}
			
		});
		Iterator<Entry<String, String>> iter =  dbCfgItems.iterator();
	    while(iter.hasNext()) {
	    	Entry<String,String> entry = iter.next();
	    	dbElement.addElement("property")
                      .addAttribute("name", entry.getKey())
                      .addText(entry.getValue());
	    }
	    
	    
	    try {  
	    	OutputFormat format = OutputFormat.createPrettyPrint();  
            XMLWriter writer = new XMLWriter(new FileWriter("./configure/service_cfg.xml"),format);  
            writer.write(doc);  
            writer.close();  
        } catch (IOException e) {
            e.printStackTrace();  
            return false;
        }  
	    return true;
	}
	
}
