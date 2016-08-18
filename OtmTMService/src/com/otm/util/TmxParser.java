//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;

import com.otm.db.TU;
import com.otm.db.TUV;

public class TmxParser {

	public ArrayList<TU> parseTmxForTU(String fileOrString, boolean isFile) throws DocumentException, UnsupportedEncodingException {
		ArrayList<TU> tus = new ArrayList<TU>();
		
		SAXReader saxReader = new SAXReader();
		Document doc = null;
		if(isFile)
		    doc = saxReader.read(new File(fileOrString));
		else
			doc = saxReader.read( new ByteArrayInputStream(fileOrString.getBytes("UTF-8")) );
		Element root = doc.getRootElement();

		List<?> body = root.elements("body");
		if (body.isEmpty())
			return tus;

		Element e = (Element) body.get(0);

		// tus
		for (Iterator<?> tuIter = e.elementIterator(); tuIter.hasNext();) {

			Element tu = (Element) tuIter.next();
            
			TU tuc = new TU();
			
			// attributes
			Iterator<?> attIter = tu.attributeIterator();
			while (attIter.hasNext()) {
				Attribute att = (Attribute) attIter.next();
				tuc.set(att.getName(),att.getValue());
			}

			// child(prop, tuvs)
			Iterator<?> child = tu.elementIterator();
			while (child.hasNext()) {
				Element ch = (Element) child.next();

				if ("prop".equals(ch.getName())) {
//System.out.println(ch.attribute(0).getText()+" "+ch.getText());
					// prop
					tuc.set(ch.attribute(0).getText(),ch.getText());
					
					//above here is TU
				} else if ("tuv".equals(ch.getName())) {
					TUV tuv = new TUV();
					// tuv prop
					Iterator<?> attiter =  ch.attributes().iterator();
					while(attiter.hasNext()) {
						Attribute atttuv = (Attribute) attiter.next();
						if("lang".equals(atttuv.getName())) {
							tuv.set("xml:lang",atttuv.getValue());
						    break;
						}
					}
					
					Iterator<?> propIter = ch.elementIterator("prop");
					while(propIter.hasNext()) {
						Element prop = (Element) propIter.next();
						tuv.set(prop.attribute(0).getText() ,prop.getText());
//System.out.println(prop.attribute(0).getText()+" "+prop.getText());
					}
					
					// seg
					Element seg = ch.element("seg");
					tuv.set("seg" ,seg.getText() );
System.out.println(seg.getText());					
					tuc.addTUV(tuv);
				}

			}//end while

			tus.add(tuc);
			
		}//end for
		
		return tus;

	}
}
