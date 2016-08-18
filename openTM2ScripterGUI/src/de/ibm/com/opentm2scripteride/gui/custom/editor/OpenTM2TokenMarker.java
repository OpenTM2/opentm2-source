/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom.editor;
/**
 * Modified CTokenMarker 
 * Original CTokenMarker Copyright text:
 * Copyright (C) 1998, 1999 Slava Pestov, 
 *
 * You may use and modify this package for any purpose. Redistribution is
 * permitted, in both source and binary form, provided that this notice
 * remains intact in all source distributions of this package.
 */

import java.util.List;

import javax.swing.text.Segment;
import de.ibm.com.opentm2scripteride.models.CommandCatalog;
import de.ibm.com.opentm2scripteride.models.CommandFrame;

/**
 * OpenTM2TokenMarker
 *
 * @author Tim Reppenhagen, 2011/12/10
 * @version 1.0 2011/12/10
 *  modified CTokenMarker by Slava Pestov
 */
public class OpenTM2TokenMarker extends TokenMarker
{
	public OpenTM2TokenMarker()
	{
		this(true,getKeywords());
	}
	
	//Variables
	private static KeywordMap keywords;
	
	private boolean cpp;
	private int lastOffset;
	private int lastKeyword;
	
	@SuppressWarnings("static-access")
	public OpenTM2TokenMarker(boolean cpp, KeywordMap keywords)
	{
		this.cpp = cpp;
		this.keywords = keywords;
	}
	
	/**
	 * Fills in the Keywords into a Hashtable and returns it
	 * @return the filled Keyword-Hashtable
	 */
	public static KeywordMap getKeywords()
	{
		if(keywords != null) {
			return keywords;
		}
		
		keywords = new KeywordMap(true);
		CommandCatalog cc = CommandCatalog.getInstance();
		for (int i = 0; i < 4;  i++) {
			CommandFrame.Type type = CommandFrame.Type.Unknown;
			byte token = 0;
			switch (i) {
				case 0:
					type = CommandFrame.Type.ApiCall;
					token = Token.APICALLS;
					break;
				case 1:
					type = CommandFrame.Type.Function;
					token = Token.FUNCTIONS;
					break;
				case 2:
					type = CommandFrame.Type.Block;
					token = Token.BLOCK;
					break;
				case 3:
					type = CommandFrame.Type.Wrapper;
					token = Token.BLOCK;
					break;
			}
			if (type == CommandFrame.Type.Unknown) {
				break; //should never ever happen!
			}
			//get api calls and fill command and endcommand in map
			List<CommandFrame> list = cc.commandsByType(type);
			for (CommandFrame cur : list) {
				String cmd = cur.getCommand();
				if (!cmd.isEmpty()) {
					keywords.add(cmd, token);
				}
				cmd = cur.getEnd();
				if (!cmd.isEmpty()) {
					keywords.add(cmd, token);
				}
			}
			

			//Comment (*)- and variable(% // $)-operators are defined in the loop later	
		}

		return keywords;
	}
	

	/**
	 * Marks the tokens in the color, which is specified in the class "Token"
	 */
	public byte markTokensImpl(byte token, Segment line, int lineIndex)
	{
		char[] array = line.array;
		int offset = line.offset;
		lastOffset = offset;
		lastKeyword = offset;
		int length = line.count + offset;
		boolean backslash = false;

loop:		for(int i = offset; i < length; i++)
		{	
			int i1 = (i+1);

			char c = array[i];

			switch(token)
			{
			case Token.NULL:
				switch(c)
				{
				case '*':
					// how to process such cases?
					if(cpp && (i==line.offset||isAllSpaceBefore(array,line.offset,i)) )
					{
						if(doKeyword(line,i,c))
							break;
						addToken(i - lastOffset,token);
						addToken(length - i,Token.COMMENT);
						lastOffset = lastKeyword = length;
						break loop;
					}
					break;
				case '%':
					doKeyword(line,i,c);

						addToken(i - lastOffset,token);
						token = Token.VARIABLE;
						lastOffset = lastKeyword = i;
					
					break;
				case '$':
					doKeyword(line,i,c);

						addToken(i - lastOffset,token);
						token = Token.VARIABLE;
						lastOffset = lastKeyword = i;
					
					break;

				default:
					backslash = false;
					if(!Character.isLetterOrDigit(c)
						&& c != '_')
						doKeyword(line,i,c);
					break;
				}
				break;
			
			case Token.COMMENT:
				backslash = false;
				if(c == '*' && length - i > 1)
				{
					if(array[i1] == '/')
					{
						i++;
						addToken((i+1) - lastOffset,token);
						token = Token.COMMENT;
						lastOffset = lastKeyword = i+1;
					}
				}
				break;
			case Token.VARIABLE:
				if(c == '%' || c=='$')
				{
					addToken(i1 - lastOffset,Token.VARIABLE);
					token = Token.NULL;
					lastOffset = lastKeyword = i1;
				}
				break;
			default:
				throw new InternalError("Invalid state: "
					+ token);
			}
		}

		if(token == Token.NULL)
			doKeyword(line,length,'\0');

		switch(token)
		{
		case Token.VARIABLE:
			addToken(length - lastOffset,Token.INVALID);
			token = Token.NULL;
			break;
		case Token.BLOCK:
			addToken(length - lastOffset,token);
			if(!backslash)
				token = Token.NULL;
		default:
			addToken(length - lastOffset,token);
			break;
		}

		return token;
	}

	/**
	 * Jumps to the next Keyword
	 */
	private boolean doKeyword(Segment line, int i, char c)
	{
		int i1 = i+1;

		int len = i - lastKeyword;
		byte id = keywords.lookup(line,lastKeyword,len);
		if(id != Token.NULL)
		{
			if(lastKeyword != lastOffset)
				addToken(lastKeyword - lastOffset,Token.NULL);
			addToken(len,id);
			lastOffset = i;
		}
		lastKeyword = i1;
		return false;
	}
	
	/**
	 * 
	 * @param line
	 * @param start
	 * @param current
	 * @return
	 */
	private boolean isAllSpaceBefore(char[] line, int start, int current)
	{
		if(current == start)
			return true;
		
		for(int i=current; i>start; i--)
		{
			if(!Character.isSpaceChar(line[i]))
				return false;
		}
		
		return false;
	}
	
}
