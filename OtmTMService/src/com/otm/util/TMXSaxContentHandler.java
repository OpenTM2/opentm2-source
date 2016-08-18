//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

import java.util.ArrayList;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import com.otm.db.TU;
import com.otm.db.TUV;


public class TMXSaxContentHandler extends DefaultHandler {
  
	private boolean bCatchData = false;
	private boolean bIsTUVProp = false;
	private TU curTU = null;
	private TUV curTUV = null;
	private String curPropKey = null;
	private StringBuilder curData = new StringBuilder();
	private ArrayList<TU> tus = new ArrayList<TU>();
	
	
	public ArrayList<TU> getTUS() {
		return  tus;
	}
	
	@Override
    public void startElement(String uri, String localName, String qName,
            Attributes attributes) throws SAXException {
    
        // 2 TU
        if("tu".equalsIgnoreCase(qName)) {
        	curTU = new TU();
			for(int i=0; i<attributes.getLength(); i++) {
				curTU.set( attributes.getQName(i), attributes.getValue(i) );
			}

        }
		
        // 3 tuv
        else if("tuv".equalsIgnoreCase(qName)) {
        	curTUV = new TUV();
        	for(int i=0; i<attributes.getLength(); i++) {
        		curTUV.set( attributes.getQName(i), attributes.getValue(i) );
			}
        	bIsTUVProp = true;
        }
        
        // 4 seg
        else if("seg".equalsIgnoreCase(qName)) {
        	bCatchData = true;
        }
         
        // 5 prop
        else if("prop".equals(qName)) {
        	bCatchData = true;
        	curPropKey = attributes.getValue(0);
        } 
        
        // 6 other
        else if(bCatchData) {
        	curData.append("<")
        	       .append(qName)
        	       .append(appendAttributes(attributes))
        	       .append(">");
        }
        
       
    }
    
   
    @Override
    public void characters(char[] ch, int start, int length)
            throws SAXException {
    
         if(bCatchData) {
        	 String segText = new String(ch,start,length).replaceAll("&", "&amp;")
                                                         .replaceAll("<", "&lt;")
                                                         .replaceAll(">", "&gt;")
                                                         .replaceAll("\"","&quot;" )
                                                         .replaceAll("'","&apos;" );
            curData.append(segText);
        	
         }	
    }
    
    @Override
    public void endElement(String uri, String localName, String qName)
            throws SAXException {
    	
    	
    	// 2 tu
    	if("tu".equalsIgnoreCase(qName)) {
    		tus.add(curTU); 	
        	
        }//end TU
    	
    	// 3 tuv
    	else if("tuv".equalsIgnoreCase(qName)) {
    		curTU.addTUV(curTUV);
    		bIsTUVProp = false;
    	}
    	
    	// 4 seg
        else if("seg".equalsIgnoreCase(qName)) {
        	curTUV.set("seg", curData.toString());
        	bCatchData = false;
        	curData.delete(0, curData.length());
        } 
    	
    	// 5 prop
        else if("prop".equals(qName)) {
        	if(bIsTUVProp) {
        		curTUV.set( curPropKey, curData.toString());
        	} else {
        		curTU.set( curPropKey, curData.toString());
        	}
        	bCatchData = false;
        	curData.delete(0, curData.length());
        } 
    	
    	// 6 other
        else if(bCatchData) {
        	curData.append("</").append(qName).append(">");
        } 
       
        
    }
    
    	
    private String appendAttributes(Attributes attributes) {
    	StringBuilder sbAtt = new StringBuilder();
    	if(attributes != null) {
			for(int i=0;i<attributes.getLength();i++){
				sbAtt.append(" ").append(attributes.getQName(i))
				       .append("=\"").append(attributes.getValue(i))
				       .append("\"");
			}
		}
    	return sbAtt.toString();
    }
    

}