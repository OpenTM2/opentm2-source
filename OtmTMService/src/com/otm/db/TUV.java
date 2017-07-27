//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.db;

public class TUV {
	@Override
	public String toString() {
		return "TUV [createtionDate=" + createtionDate + ", lastAccessTime="
				+ updateCounter + ", segment=" + segment + ", lang=" + lang
				+ ", language=" + language + "]";
	}

	@Override
	public int hashCode() {
        int result = 1;
        result = result*31 + segment.hashCode();
        //result = result*31 + lang.hashCode();
        //result = result*31 + language.hashCode();
		return result;
	}
	
	private String createtionDate;
	private String updateCounter;
	private String segment;
	private String lang;
	private String language;
	private String creator;
	  
	

	public void set(String name, String value) {
		if ("xml:lang".equals(name) || "tmgr-lang".equals(name)) {
			setLang(value);
		} else if ("tmgr-language".equals(name) || "tmgr:language".equals(name)) {
			setLanguage(value);
		} else if ("seg".equals(name)) {
			setSegment(value);
		}
	}

	public String getCreatetionDate() {
		return createtionDate;
	}

	public void setCreatetionDate(String createtionDate) {
		this.createtionDate = createtionDate;
	}

	public String getSegment() {
		return segment;
	}

	public void setSegment(String segment) {
		this.segment = segment;
	}

	public String getLastAccessTime() {
		return updateCounter;
	}

	public void setLastAccessTime(String lastAccessTime) {
		this.updateCounter = lastAccessTime;
	}

	public String getLang() {
		return lang;
	}

	public void setLang(String lang) {
		this.lang = lang;
	}

	public String getLanguage() {
		return language;
	}

	public void setLanguage(String language) {
		this.language = language;
	}

	public String getCreator() {
		return creator;
	}

	public void setCreator(String creator) {
		this.creator = creator;
	}
}
