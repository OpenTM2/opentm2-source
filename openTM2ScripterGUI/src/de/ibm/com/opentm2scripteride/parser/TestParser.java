/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
/*
package de.ibm.com.opentm2scripteride.parser;

import java.io.FileInputStream;
import java.io.InputStream;

import org.antlr.runtime.ANTLRInputStream;
import org.antlr.runtime.CommonTokenStream;

import de.ibm.com.opentm2scripteride.models.ScriptModel;

public class TestParser {

	public static void main(String[] args) throws Exception {
		String filename = args[0];
		InputStream is = new FileInputStream(filename);
		ANTLRInputStream ais = new ANTLRInputStream(is);
		ScriptLexer lexer = new ScriptLexer(ais);
		CommonTokenStream cts = new CommonTokenStream(lexer);
		ScriptParser parser = new ScriptParser(cts);
		ScriptModel sp = parser.script();
		System.out.println(sp.toText());
		System.out.println();
		System.out.println(sp.getEndLine());
	}
}
*/