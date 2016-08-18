/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom.editor;
/*
 * SyntaxUtilities.java - Utility functions used by syntax colorizing
 * Copyright (C) 1999 Slava Pestov
 *
 * You may use and modify this package for any purpose. Redistribution is
 * permitted, in both source and binary form, provided that this notice
 * remains intact in all source distributions of this package.
 */

import javax.swing.text.*;

import de.ibm.com.opentm2scripteride.gui.custom.editor.TokenMarker.Token;
import de.ibm.com.opentm2scripteride.utility.Configuration;
import de.ibm.com.opentm2scripteride.utility.Constants;

import java.awt.*;
import java.util.Scanner;

/**
 * Class with several utility functions used by jEdit's syntax colorizing
 * subsystem.
 *
 * @author Slava Pestov, modified by Tim Reppenhagen, 2011
 * @version $Id: SyntaxUtilities.java,v 1.9 1999/12/13 03:40:30 sp Exp $
 */
public class SyntaxUtilities
{
	// variables
	private SyntaxUtilities() {}
	
	/**
	 * Checks if a subregion of a <code>Segment</code> is equal to a
	 * string.
	 * @param ignoreCase True if case should be ignored, false otherwise
	 * @param text The segment
	 * @param offset The offset into the segment
	 * @param match The string to match
	 */
	public static boolean regionMatches(boolean ignoreCase, Segment text,
					    int offset, String match)
	{
		int length = offset + match.length();
		char[] textArray = text.array;
		if(length > text.offset + text.count)
			return false;
		for(int i = offset, j = 0; i < length; i++, j++)
		{
			char c1 = textArray[i];
			char c2 = match.charAt(j);
			if(ignoreCase)
			{
				c1 = Character.toUpperCase(c1);
				c2 = Character.toUpperCase(c2);
			}
			if(c1 != c2)
				return false;
		}
		return true;
	}
	
	/**
	 * Checks if a subregion of a <code>Segment</code> is equal to a
	 * character array.
	 * @param ignoreCase True if case should be ignored, false otherwise
	 * @param text The segment
	 * @param offset The offset into the segment
	 * @param match The character array to match
	 */
	public static boolean regionMatches(boolean ignoreCase, Segment text,
					    int offset, char[] match)
	{
		int length = offset + match.length;
		char[] textArray = text.array;
		if(length > text.offset + text.count)
			return false;
		for(int i = offset, j = 0; i < length; i++, j++)
		{
			char c1 = textArray[i];
			char c2 = match[j];
			if(ignoreCase)
			{
				c1 = Character.toUpperCase(c1);
				c2 = Character.toUpperCase(c2);
			}
			if(c1 != c2)
				return false;
		}
		return true;
	}

	/**
	 * Returns the default style table. This can be passed to the
	 * <code>setStyles()</code> method of <code>SyntaxDocument</code>
	 * to use the default syntax styles.
	 */
	public static SyntaxStyle[] getDefaultSyntaxStyles()
	{
		SyntaxStyle[] styles = new SyntaxStyle[Token.ID_COUNT];
		
		styles[Token.VARIABLE] = getUserSyntaxStlye(0) ;
		if(styles[Token.VARIABLE] == null){
			styles[Token.VARIABLE] = new SyntaxStyle(new Color(0x2B1C8B),false,false);
		}
		
		styles[Token.COMMENT] = getUserSyntaxStlye(1);
		if(styles[Token.COMMENT] == null){
			styles[Token.COMMENT] = new SyntaxStyle(new Color(0x888888),true,false);
		}
		
		styles[Token.APICALLS] = getUserSyntaxStlye(2);
		if(styles[Token.APICALLS] == null){
			styles[Token.APICALLS] = new SyntaxStyle(new Color(0,100,0),false,true);
		}
		
		styles[Token.BLOCK] = getUserSyntaxStlye(3);
		if(styles[Token.BLOCK] == null){
			styles[Token.BLOCK] = new SyntaxStyle(Color.blue,false,true);
		}
		
		styles[Token.FUNCTIONS] = getUserSyntaxStlye(4);
		if(styles[Token.FUNCTIONS] == null){
			styles[Token.FUNCTIONS] = new SyntaxStyle(Color.black,false,true);
		}
		
		styles[Token.INVALID] = new SyntaxStyle(Color.red,false,true);

		return styles;
	}
	
	private static SyntaxStyle getUserSyntaxStlye(int highlightingtype){
		String saved,substring;
		Color userColor;
		boolean bold,italic;
		saved = Configuration.getInstance().getValue(Constants.highlightingconstants[highlightingtype]);
		
		if(saved != null){
			
			saved = saved.substring(20);
			Scanner sc = extracted(saved).useDelimiter("\\D+");
		    userColor = new Color(sc.nextInt(), sc.nextInt(), sc.nextInt());
		    sc.useDelimiter(",");
		    sc.next();
		    if(sc.hasNext()){
		    	 substring = sc.next();
		    	 if(substring.startsWith("italic")){
		    		 italic = true;
		    		 if(sc.hasNext()){
		    			 substring = sc.next();
		    			 if(substring.startsWith("bold")){
			    			 bold = true;
			    		 }
			    		 else{
			    			 bold = false;
			    		 }
		    		 }
		    		 else{
		    			 bold = false;
		    		 }
		    		 
		    	 }
		    	 else{
		    		 bold = true;
		    		 if(sc.hasNext()){
		    			 substring = sc.next();
		    			 if(substring.startsWith("italic")){
			    			 italic = true;
			    		 }
			    		 else{
			    			 italic = false;
			    		 }
		    		 }
		    		 else{
		    			 italic = false;
		    		 }
		    	 }
		    }
		    else{
		    	bold = false;
		    	italic = false;
		    }
		    sc.close();
		    return new SyntaxStyle(userColor,italic,bold);
		}
		else{
			return null;
		}
	
	}

	private static Scanner extracted(String saved) {
		return new Scanner(saved);
	}
	

	/**
	 * Paints the specified line onto the graphics context. Note that this
	 * method munges the offset and count values of the segment.
	 * @param line The line segment
	 * @param tokens The token list for the line
	 * @param styles The syntax style list
	 * @param expander The tab expander used to determine tab stops. May
	 * be null
	 * @param gfx The graphics context
	 * @param x The x co-ordinate
	 * @param y The y co-ordinate
	 * @return The x co-ordinate, plus the width of the painted string
	 */
	public static int paintSyntaxLine(Segment line, Token tokens,
		SyntaxStyle[] styles, TabExpander expander, Graphics gfx,
		int x, int y)
	{
		Font defaultFont = gfx.getFont();
		Color defaultColor = gfx.getColor();
		for(;;)
		{
			byte id = tokens.id;
			if(id == Token.END)
				break;

			int length = tokens.length;
			if(id == Token.NULL)
			{
				if(!defaultColor.equals(gfx.getColor()))
					gfx.setColor(defaultColor);
				if(!defaultFont.equals(gfx.getFont()))
					gfx.setFont(defaultFont);
			}
			else
				styles[id].setGraphicsFlags(gfx,defaultFont);

			line.count = length;
			x = Utilities.drawTabbedText(line,x,y,gfx,expander,0);
			line.offset += length;

			tokens = tokens.next;
		}
		return x;
	}


}
