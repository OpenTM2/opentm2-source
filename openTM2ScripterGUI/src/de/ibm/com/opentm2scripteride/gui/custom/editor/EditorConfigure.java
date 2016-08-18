/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom.editor;

import java.awt.Color;
import java.awt.Font;
import de.ibm.com.opentm2scripteride.gui.custom.editor.TokenMarker.Token;
import de.ibm.com.opentm2scripteride.utility.Configuration;
import de.ibm.com.opentm2scripteride.utility.Constants;

/**
 * Provides the editors' methods for the GUI 
 * @author Tim Reppenhagen 2011/12/14
 * @version 1.0 2011/12/14
 *
 */
public class EditorConfigure{
	
	private static EditorConfigure instance = null;
	private JEditTextArea editor = null;

	/**
	 * Sets the selection color.
	 * @param selectionColor The selection color
	 */
	public void setSelectionColor(Color selectionColor)
	{
		
		editor.painter.setSelectionColor(selectionColor);
		Configuration.getInstance().setValue(Constants.highlightingconstants[5],selectionColor.toString());
	}
	
	/**
	 * Returns the color for selections
	 * @return The color for selections
	 */
	public Color getSelectionColor(){
		return editor.painter.getSelectionColor();
	}
	
	/**
	 * Sets the line highlight color.
	 * @param lineHighlightColor The line highlight color
	 */
	public void setLineHighlightColor(Color lineHighlightColor)
	{
		editor.painter.setLineHighlightColor(lineHighlightColor);
		Configuration.getInstance().setValue(Constants.highlightingconstants[6],lineHighlightColor.toString());
	}
	
	/**
	 * Returns the highlighting color
	 * @return The highlighting color
	 */
	public Color getLineHighlightColor(){
		return editor.painter.getLineHighlightColor();
	}
	
	
	/**
	 * Sets the font for this component. This is overridden to update the
	 * cached font metrics and to recalculate which lines are visible.
	 * @param font The font
	 */
	public void setFont(Font font)
	{
		editor.painter.setFont(font);
		Configuration.getInstance().setValue(Constants.genFont, font.toString());
	}
	
	/**
	 * Returns the Font from the TextArea Painter
	 * @return The Font
	 */
	public Font getFont(){
		return editor.painter.getFont();
	}
	
	
	/**
	 * Sets the EOL character
	 * @param eol true if the EOL char should be painted,
	 * false otherwise
	 */
	public void setEOLChar(boolean eol){
		editor.painter.setEOLMarkersPainted(eol);
		Configuration.getInstance().setValue(Constants.highlightingconstants[7], new Boolean(eol).toString());
	}
	
	
	/**
	 * Returns true if the EOL character is painted
	 * @return boolean
	 */
	public boolean getEOLChar(){
		return editor.painter.getEOLMarkersPainted();
	}
	
	
	/**
	 * Sets the Style of the variable
	 * @param color the new color for variables
	 * @param italic should the variable be shown italic
	 * @param bold should the variable be shown bold
	 */
	public void setStyleVariable(Color color, boolean italic, boolean bold)
	{
		
		SyntaxStyle newStyle = new SyntaxStyle(color ,italic, bold);
		SyntaxStyle[] styles=editor.painter.getStyles();
		styles[Token.VARIABLE] = newStyle;
		editor.painter.setStyles(styles);
		Configuration.getInstance().setValue(Constants.highlightingconstants[0],newStyle.toString().substring(48));
	}
	
	/**
	 * Returns the style for the variable
	 * @return the style for the variable
	 */
	public SyntaxStyle getStyleVariable(){
		SyntaxStyle[] styles=editor.painter.getStyles();
		return styles[Token.VARIABLE];
	}
	
	/**
	 * Sets the Style of the Comments
	 * @param color the new color for the Functions
	 * @param italic should the Comments be shown italic
	 * @param bold should the Comments Structures be shown bold
	 */
	public void setStyleComments(Color color, boolean italic, boolean bold){
		SyntaxStyle newStyle = new SyntaxStyle(color ,italic, bold);
		SyntaxStyle[] styles=editor.painter.getStyles();
		styles[Token.COMMENT] = newStyle;
		editor.painter.setStyles(styles);
		Configuration.getInstance().setValue(Constants.highlightingconstants[1],newStyle.toString().substring(48));
	}
	
	/**
	 * Returns the style for the comments
	 * @return the style for the comments
	 */
	public SyntaxStyle getStyleComments(){
		SyntaxStyle[] styles=editor.painter.getStyles();
		return styles[Token.COMMENT];
	}
	
	/**
	 * Sets the Style of the ApiCalls
	 * @param color the new color for ApiCalls
	 * @param italic should the ApiCalls be shown italic
	 * @param bold should the ApiCalls be shown bold
	 */
	public void setStyleApiCalls(Color color, boolean italic, boolean bold){
		SyntaxStyle newStyle = new SyntaxStyle(color ,italic, bold);
		SyntaxStyle[] styles=editor.painter.getStyles();
		styles[Token.APICALLS] = newStyle;
		editor.painter.setStyles(styles);
		Configuration.getInstance().setValue(Constants.highlightingconstants[2],newStyle.toString().substring(48));
	}
	
	/**
	 * Returns the style for the APICalls 
	 * @return the style for the APICalls 
	 */
	public SyntaxStyle getStyleApiCalls(){
		SyntaxStyle[] styles=editor.painter.getStyles();
		return styles[Token.APICALLS];
	}
	
	/**
	 * Sets the Style of the Control Structures//Blocks
	 * @param color the new color for Control Structures//Blocks
	 * @param italic should the Control Structures be shown italic
	 * @param bold should the Control Structures be shown bold
	 */
	public void setStyleControlStructures(Color color, boolean italic, boolean bold){
		SyntaxStyle newStyle = new SyntaxStyle(color ,italic, bold);
		SyntaxStyle[] styles=editor.painter.getStyles();
		styles[Token.BLOCK] = newStyle;
		editor.painter.setStyles(styles);
		Configuration.getInstance().setValue(Constants.highlightingconstants[3],newStyle.toString().substring(48));
	}
	
	/**
	 * Returns the style for the ControlStructures//Blocks
	 * @return Returns the style for the ControlStructures//Blocks
	 */
	public SyntaxStyle getStyleControlStructures(){
		SyntaxStyle[] styles=editor.painter.getStyles();
		return styles[Token.BLOCK];
	}
	
	/**
	 * Sets the Style of the Functions
	 * @param color the new color for the Functions
	 * @param italic should the Functions be shown italic
	 * @param bold should the Functions Structures be shown bold
	 */
	public void setStyleFunctions(Color color, boolean italic, boolean bold){
		SyntaxStyle newStyle = new SyntaxStyle(color ,italic, bold);
		SyntaxStyle[] styles=editor.painter.getStyles();
		styles[Token.FUNCTIONS] = newStyle;
		editor.painter.setStyles(styles);
		Configuration.getInstance().setValue(Constants.highlightingconstants[4],newStyle.toString().substring(48));
	}
	
	/**
	 * Returns the style for the functions
	 * @return the style for the functions
	 */
	public SyntaxStyle getStyleFunctions(){
		SyntaxStyle[] styles=editor.painter.getStyles();
		return styles[Token.FUNCTIONS];
	}
	
	
	public void setEditor(JEditTextArea editor) {
		this.editor = editor;
	}
	/**********************************************************/
	
	public static EditorConfigure getInstance(){
		if(instance == null){
			instance = new EditorConfigure();
		}
		return instance;
	}
}


