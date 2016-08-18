//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParserFactory;
import org.xml.sax.SAXException;

import com.otm.db.TU;


public class TmxSaxParser {

	public ArrayList<TU> parseTmxForTU(String fileOrString, boolean isFile) throws ParserConfigurationException, SAXException, UnsupportedEncodingException, IOException {
		
        SAXParserFactory parserFactory=SAXParserFactory.newInstance();
        javax.xml.parsers.SAXParser parser=parserFactory.newSAXParser();
       
        TMXSaxContentHandler myhandler=new TMXSaxContentHandler();
        if(isFile)
        	parser.parse(new FileInputStream(new File(fileOrString)), myhandler);
        else
		    parser.parse(new ByteArrayInputStream(fileOrString.getBytes("UTF-8")), myhandler);
		
		return myhandler.getTUS();
	}
	

}
