/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom.editor;

/*
 * TextAreaDefaults.java - Encapsulates default values for various settings
 * Copyright (C) 1999 Slava Pestov, modified version by Tim Reppenhagen, 2011
 *
 * You may use and modify this package for any purpose. Redistribution is
 * permitted, in both source and binary form, provided that this notice
 * remains intact in all source distributions of this package.
 */

import javax.swing.JPopupMenu;
import de.ibm.com.opentm2scripteride.utility.Configuration;
import de.ibm.com.opentm2scripteride.utility.Constants;
import java.awt.Color;
import java.awt.Font;
import java.util.Scanner;

/**
 * Encapsulates default settings for a text area. This can be passed
 * to the constructor once the necessary fields have been filled out.
 * The advantage of doing this over calling lots of set() methods after
 * creating the text area is that this method is faster.
 */
public class TextAreaDefaults
{
	
	public InputHandler inputHandler;
	public SyntaxDocument document;
	public boolean editable;

	public boolean caretVisible;
	public boolean caretBlinks;
	public boolean blockCaret;
	public int electricScroll;

	public int cols;
	public int rows;
	public SyntaxStyle[] styles;
	public Color caretColor;
	public Color selectionColor;
	public Color lineHighlightColor;
	public boolean lineHighlight;
	public Color bracketHighlightColor;
	public boolean bracketHighlight;
	public Color eolMarkerColor;
	public boolean eolMarkers;
	public boolean paintInvalid;

	public Font generalFont;
	
	public JPopupMenu popup;

	/**
	 * Returns a new TextAreaDefaults object with the default values filled
	 * in.
	 */
	public  TextAreaDefaults()
	{
			inputHandler = new DefaultInputHandler();
			inputHandler.addDefaultKeyBindings();
		    document = new SyntaxDocument();
			editable = true;

			caretVisible = true;
			caretBlinks = true;
			electricScroll = 3;

			cols = 10;
			rows = 10;
			styles = SyntaxUtilities.getDefaultSyntaxStyles();
			caretColor = Color.black;
			selectionColor = new Color(80,160,246);
			lineHighlightColor = new Color(0xfdffaf);
			lineHighlight = true;
			bracketHighlightColor = Color.black;
			bracketHighlight = true;
			eolMarkerColor = new Color(0xBBBBBB);
			if(Configuration.getInstance().getValue(Constants.highlightingconstants[7]) != null){
				try{
					eolMarkers = Boolean.valueOf(Configuration.getInstance().getValue(Constants.highlightingconstants[7]));
				}
				catch(NumberFormatException e){
					eolMarkers = true;
				}
			}
			else{
				eolMarkers = true;
			}
			paintInvalid = false;
			
			if(Configuration.getInstance().getValue(Constants.highlightingconstants[5]) != null){
				 Scanner sc = new Scanner(Configuration.getInstance().getValue(Constants.highlightingconstants[5]));
				 sc.useDelimiter("\\D+");
				 try{
					 selectionColor = new Color(sc.nextInt(), sc.nextInt(), sc.nextInt());
				 }
				 catch(NumberFormatException e){
					 selectionColor = new Color(30,120,226);
				}
				 finally {
				 sc.close();
				 }
			}
			else{
			selectionColor = new Color(30,120,226);
			}
			
			if(Configuration.getInstance().getValue(Constants.highlightingconstants[6]) != null){
				Scanner sc = new Scanner(Configuration.getInstance().getValue(Constants.highlightingconstants[6]));
				sc.useDelimiter("\\D+");
				try{
					lineHighlightColor = new Color(sc.nextInt(), sc.nextInt(), sc.nextInt());
				}
				 catch(NumberFormatException e){
					lineHighlightColor = new Color(0xfdffaf);
				}
				finally {
					sc.close();
				}
			}
			else{
			lineHighlightColor = new Color(0xfdffaf);
			}
			
			if(Configuration.getInstance().getValue(Constants.genFont) != null){
				String name = "",styleString="";
				int style,size;
				Scanner sc = null;
				try{
					sc = new Scanner(Configuration.getInstance().getValue(Constants.genFont));
					sc.useDelimiter(",");
					sc.next();
					sc.useDelimiter("=");
					sc.next();
					sc.useDelimiter(",");
					name =sc.next().substring(1);
					sc.useDelimiter("=");
					sc.next();
					sc.useDelimiter(",");
					styleString = sc.next().substring(1);
					sc.useDelimiter("\\D+");
					size = sc.nextInt();
					if(styleString.equalsIgnoreCase("plain")){
						style = Font.PLAIN;
					}
					else if(styleString.equalsIgnoreCase("bold")){
						style = Font.BOLD;
					}
					else if(styleString.equalsIgnoreCase("italic")){
						style = Font.ITALIC;
					}
					else{
						style = Font.BOLD | Font.ITALIC;
					}
					generalFont = new Font(name,style,size);
				}
				catch(Exception e){
					generalFont =new Font("Monospaced",Font.PLAIN,14);
				}
				finally {
					if(sc != null)
						sc.close();
				}
			}
			else{
				generalFont =new Font("Monospaced",Font.PLAIN,14);
			}

	}
}
