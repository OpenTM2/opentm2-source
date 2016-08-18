/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.utility;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;


public class OpenTM2Datas {
    private ArrayList<String> mMarkupTables    = new ArrayList<String>();
    private ArrayList<String> mSourceLanguages = new ArrayList<String>();
    private ArrayList<String> mTargetLanguages = new ArrayList<String>();
    private ArrayList<String> mEditors         = new ArrayList<String>();
    private ArrayList<String> mDrivers         = new ArrayList<String>();
    private static OpenTM2Datas instance = null;
    
    private OpenTM2Datas() {
    	String otm = getOpenTM2Location();
    	//otm = "C:\\OTM\\";

    	// markup and languages
    	loadMarkups(otm);
    	loadLanguages(otm);
    	
    	// 3 editors now
    	mEditors.add("STANDARD");
    	mEditors.add("XLATE");
    	mEditors.add("RTFEDIT");
    	
    	// Drivers
    	File[] drivers = File.listRoots();
    	for(File drive:drivers) {
    		String disk = drive.getPath();
    		if(disk!=null && !disk.isEmpty())
    			mDrivers.add(disk.substring(0, 1));
    	}
    }
    
    public static OpenTM2Datas getInstance() {
    	if(instance == null) {
    		instance = new OpenTM2Datas();
    	}
    	
    	return instance;
    }
    
    public ArrayList<String> getMarkupTables() {
        return mMarkupTables;	
    }
    
    public ArrayList<String> getSourceLanguages() {
        return mSourceLanguages;	
    }
    
    public ArrayList<String> getTargetLanguages() {
    	return mTargetLanguages;
    }
    
    public ArrayList<String> getEditors() {
    	return mEditors;
    }
    
    public ArrayList<String> getDrivers() {
    	return mDrivers;
    }
    
    /**
     * load markup table names from OpenTM2 Plugins path
     * @param otmpath
     */
    private void loadMarkups(String otmpath) {   	
    	if(otmpath==null)
    		return;
    	
    	StringBuilder pluginsDir = new StringBuilder(otmpath).append("PLUGINS");
    	File plugins = new File(pluginsDir.toString());
    	File[] plginsFiles = plugins.listFiles();
    	for(File fobj: plginsFiles) {
    		
    		if(fobj.isDirectory() && fobj.getName().indexOf("MarkupTablePlugin")!=-1) {
    			String tbName = fobj.getName();
    		    StringBuilder sbxml = new StringBuilder(fobj.getAbsolutePath()).append("\\").append(tbName).append(".xml");
    		    mMarkupTables.addAll( readTablesFromXml(new File(sbxml.toString())) );
    		}
    	}
    	
    	// sort
    	Collections.sort(mMarkupTables);
    }
    
    /**
     * load source language and target language information from OpenTM2 Table path
     * @param otmpath
     */
    private void loadLanguages(String otmpath) {
    	if(otmpath==null)
    		return;
         StringBuilder path = new StringBuilder(otmpath).append("TABLE\\languages.xml"); 
         BufferedReader br = null;
         try {
 			br = new BufferedReader(new FileReader(new File(path.toString())));
 			String line = null;
 			while( (line=br.readLine())!=null ) {
 				if(line.indexOf("<language>")==-1)
 					continue;
 				
 				String nextLine = null;
 				String langName = null;
 				while( (nextLine=br.readLine())!=null ) {
 				    if(nextLine.indexOf("</language>")!=-1)
 				    	break;
 				    int nameBegIdx = nextLine.indexOf("<name>");
 				    if(nameBegIdx!=-1) {
 				    	int nameEndIdx = nextLine.indexOf("</name>");
 				    	if(nameEndIdx!=-1) {
 				    		langName = nextLine.substring(nameBegIdx+6, nameEndIdx);
 				    		//System.out.println(langName);
 				    	}
 				    	continue;
 				    }//end if
 				    
 				    int sourceBegIdx = nextLine.indexOf("<isSourceLanguage>");
 				    if(sourceBegIdx!=-1) {
 				    	int sourceEndIdx = nextLine.indexOf("</isSourceLanguage>");
 				    	if(sourceEndIdx != -1) {
 				    		String yesOrNo = nextLine.substring(sourceBegIdx+"<isSourceLanguage>".length(),sourceEndIdx);
 				    		if("yes".equals(yesOrNo))
 				    			mSourceLanguages.add(langName);
 				    	}
 				    	continue;
 				    }//end if
 				    
 				   int  targetBegIdx = nextLine.indexOf("<isTargetLanguage>");
				   if(targetBegIdx!=-1) {
				    	int targetEndIdx = nextLine.indexOf("</isTargetLanguage>");
				    	if(targetEndIdx != -1) {
				    		String yesOrNo = nextLine.substring(targetBegIdx+"<isTargetLanguage>".length(),targetEndIdx);
				    		if("yes".equals(yesOrNo))
				    			mTargetLanguages.add(langName);
				    	}
				    	break;
				    }//end if
 				}//end while
 			}//end while
         } catch (IOException e) {
 			e.printStackTrace();
 		} finally {
 			//sort
 			if (!mTargetLanguages.isEmpty())
 			    Collections.sort(mTargetLanguages);
 			
            if (!mSourceLanguages.isEmpty())
 			    Collections.sort(mSourceLanguages);

 			try {
 				if(br!=null)
 			        br.close();
 				} catch (IOException e) {
 					e.printStackTrace();
 				}
 		}
         
    }
    
    /**
     * read markup information from XML file
     * @param xmlFile
     * @return
     */
    private ArrayList<String> readTablesFromXml(File xmlFile) {
    	ArrayList<String> res = new ArrayList<String>();
    	BufferedReader br = null;
        try {
			br = new BufferedReader(new FileReader(xmlFile));
			String line = null;
			while( (line=br.readLine())!=null ) {
				if(line.indexOf("<markup>")==-1)
					continue;
				
				String nextLine = null;
				while( (nextLine=br.readLine())!=null ) {
				    if(nextLine.indexOf("</markup>")!=-1)
				    	break;
				    int nameBegIdx = nextLine.indexOf("<name>");
				    if(nameBegIdx!=-1) {
				    	int nameEndIdx = nextLine.indexOf("</name>");
				    	if(nameEndIdx!=-1) {
				    		//System.out.println(nextLine.substring(nameBegIdx+6, nameEndIdx));
				    		res.add(nextLine.substring(nameBegIdx+6, nameEndIdx));
				    	}
				    	break;
				    }//end if
				    
				}//end while
			}//end while
        } catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				if(br!=null)
			        br.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
		}
        return res;
    }
    
    /**
     * parse Xml file to get markup information 
     * @param path
     * @return
     */
    private ArrayList<String> readTablesFromXml(String path) {
        ArrayList<String> res = new ArrayList<String>();
        
        try {
			DocumentBuilder db=DocumentBuilderFactory.newInstance().newDocumentBuilder();			
			Document document=db.parse(new File(path));
			Element root=document.getDocumentElement() ;
			NodeList list = root.getElementsByTagName("markup");
			
			for(int i=0; i<list.getLength(); i++) {
				Node node = list.item(i);
				if(!node.hasChildNodes())
					continue;
			
					NodeList children = node.getChildNodes();
					for(int j=0; j<children.getLength(); j++) {
						Node child = children.item(j);
						if(child instanceof Element && "name".equals(child.getNodeName()) ){
							res.add(child.getTextContent());
							break;
						}
					}//end for
					
			}//end for
	
        }catch (ParserConfigurationException  | SAXException  | IOException e) {
			e.printStackTrace();
		}
        return res;
    }
    
    /**
     * get location where OpenTm2 installed
     * @return
     */
    private String getOpenTM2Location() {
		String location = null;
		File temp = new File("temp.txt");
        String path = temp.getAbsolutePath();
        String searching = File.separator+"OTM"+File.separator;
        int otmIdx = path.lastIndexOf(searching);
        if(otmIdx != -1) {
        	StringBuilder sb = new StringBuilder( path.substring(0, otmIdx+searching.length()) );
        	location = sb.toString();
        }
		return location;
	}

	/**
	 * @param args
	 */
//	public static void main(String[] args) {
//		// TODO Auto-generated method stub
//        OpenTM2Datas otmdatas = OpenTM2Datas.getInstance();
//       
//	}

}
