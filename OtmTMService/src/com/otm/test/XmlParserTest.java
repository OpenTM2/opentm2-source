//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.test;
import java.io.IOException;
import java.sql.SQLException;
import java.util.List;
import java.util.concurrent.TimeUnit;

import javax.xml.parsers.ParserConfigurationException;

import org.dom4j.DocumentException;
import org.xml.sax.SAXException;

import com.otm.db.Dao;
import com.otm.db.DaoFactory;
import com.otm.db.TU;
import com.otm.util.TmxSaxParser;

public class XmlParserTest {

	public static void main(String[] args) throws DocumentException, ParserConfigurationException, SAXException, IOException {
        TmxSaxParser tmx = new TmxSaxParser();
long begTime = System.currentTimeMillis();		 

		List<TU> tus = tmx.parseTmxForTU("testdata/1.tmx",true);
		
for(TU tu:tus) {
	System.out.println(tu.toString());
}
		Dao dao= DaoFactory.getInstance();
		try {
			dao.upload("wliping","mytest2", tus);
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
System.out.println(TimeUnit.MILLISECONDS.toSeconds(System.currentTimeMillis()-begTime));
	}
	
}
