//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.db;

import java.util.ArrayList;
import java.util.List;

public class TU {
	
    @Override
	public int hashCode() {
		int result = 1;
        result = result*31 + createtionDate.hashCode();
        result = result*31 + Integer.valueOf(segNum);
        result = result*31 + markup.hashCode();
        result = result*31 + docName.hashCode();
        result = result*31 + shortDocName.hashCode();
        
        for(TUV tuv:tuvs){
        	result = result*31 + tuv.hashCode();
        }
        
		return result;
	}

	@Override
	public String toString() {
		return "TU [createtionDate=" + createtionDate + ", lastAccessTime="
				+ updateCounter + ", segNum=" + segNum + ", markup=" + markup
				+ ", docName=" + docName + ", shortDocName=" + shortDocName
				+ ", tuvs=" + tuvs.toString() + "]";
	}

	private String createtionDate;
    private String updateCounter;
    private int segNum;
    private String markup;
    private String docName;
    private String shortDocName;
    
    private ArrayList<TUV> tuvs = new ArrayList<TUV>();
    
    public void set(String name, String value) {
    	if("creationdate".equals(name)) {
			setCreatetionDate(value);
		} else if("tmgr-segNum".equals(name) || "tmgr:segNum".equals(name)) {
			setSegNum(Integer.valueOf(value));
		} else if("tmgr-markup".equals(name) || "tmgr:markup".equals(name)) {
			setMarkup(value);
		} else if("tmgr-docname".equals(name) || "tmgr:docname".equals(name)) {
			setDocName(value);
		} else if("tmgr-short-docname".equals(name)) {
			setShortDocName(value);
		}
    }
    
    public void addTUV(TUV tuv) {
    	tuvs.add(tuv);
    }

    public List<TUV> getTUVS() {
    	return tuvs;
    }
    
	public String getCreatetionDate() {
		return createtionDate;
	}

	public void setCreatetionDate(String createtionDate) {
		this.createtionDate = createtionDate;
	}

	public String getLastAccessTime() {
		return updateCounter;
	}

	public void setLastAccessTime(String lastAccessTime) {
		this.updateCounter = lastAccessTime;
	}

	public int getSegNum() {
		return segNum;
	}

	public void setSegNum(int segNum) {
		this.segNum = segNum;
	}

	public String getMarkup() {
		return markup;
	}

	public void setMarkup(String markup) {
		this.markup = markup;
	}

	public String getDocName() {
		return docName;
	}

	public void setDocName(String docName) {
		this.docName = docName;
	}

	public String getShortDocName() {
		return shortDocName;
	}

	public void setShortDocName(String shortDocName) {
		this.shortDocName = shortDocName;
	}
    
    public TUV getSourceTUV() {
    	for(TUV tuv:tuvs){
    		if( "en-US".equalsIgnoreCase(tuv.getLang()) )
    			return tuv;
    	}
    	return null;
    }
    
    public ArrayList<TUV> getTargetTUV() {
    	ArrayList<TUV> tgtTUVS = new ArrayList<TUV>();
    	for(TUV tuv:tuvs){
    		if( !"en-US".equalsIgnoreCase(tuv.getLang()) )
    			tgtTUVS.add(tuv);
    	}
    	return tgtTUVS;
    }
}
