/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom.editor;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;

import javax.swing.text.Element;


/**
 * This class inherits TextAreaPainter of the JEditSyntax project and let you draw
 * line numbers in it, support anti aliasing and draw lines invalid as in spell correction.
 * @author Stefan Burnicki (stefan.burnicki@de.ibm.com)
 */
public class AdvancedTextAreaPainter extends TextAreaPainter {

	private static final long serialVersionUID = -6601324249186453822L; //generated
	protected boolean mDrawLineNumbers;
	protected boolean mAntiAliasing;
	protected int[] mInvalidLines;
	protected int mInvalidLinesCount;
	
	/**
	 * Inherited constructor of TextAreaPainter
	 * @param textArea The JEditTextArea its content should be drawn
	 * @param defaults The default settings
	 */
	public AdvancedTextAreaPainter(JEditTextArea textArea, TextAreaDefaults defaults) {
		super(textArea, defaults);
		mDrawLineNumbers = true;
		mAntiAliasing = true;
		mInvalidLines = new int[0];
		mInvalidLinesCount = 0;
	}
	
	/**
	 * Sets the lines to be drawn invalid
	 * @param invalid The array of line numbers corresponding to the invalid lines
	 * @param count The number of invalid lines in this array
	 */
	public void setInvalidLines(int[] invalid, int count) {
		mInvalidLines = invalid;
		mInvalidLinesCount = count;
		repaint();
	}
	
	/**
	 * Return all lines that will be painted invalid
	 * @return An array of line numbers corresponding to the invalid lines
	 */
	public int[] getInvalidLines() {
		return mInvalidLines;
	}
	
	/**
	 * Return the number of invalid lines
	 * @return The number of invalid lines
	 */
	public int getInvalidLinesCount() {
		return mInvalidLinesCount;
	}
	
	/**
	 * All lines will be painted valid
	 */
	public void clearInvalidLines() {
		mInvalidLinesCount = 0;
	}
	
	/**
	 * Sets whether to use anti aliasing when painting or not
	 * @param use True if it should use anti aliasing, false otherwise
	 */
	public void setAntiAliasing(boolean use) {
		mAntiAliasing = use;
		repaint();
	}
	
	/**
	 * Returns whether anti aliasing is used for painting
	 * @return True if anti aliasing is used, false otherwise
	 */
	public boolean getAntiAliasing() {
		return mAntiAliasing;
	}
	
	/**
	 * Check if the painter draws line numbers
	 * @return True if it does, false otherwise
	 */
	public boolean isDrawingLineNumbers() {
		return mDrawLineNumbers;
	}
	
	/**
	 * Set if the painter should draw line numbers
	 * @param draw True if it should, false otherwise
	 */
	public void drawLineNumbers(boolean draw) {
		mDrawLineNumbers = draw;
		repaint();
	}
	
	/**
	 * Returns the preferred size regarding the line numbering
	 */
	public Dimension getPreferredSize()
	{
		Dimension dim = super.getPreferredSize();
		if (!mDrawLineNumbers) {
			return dim;
		}
		dim.width += getLineNumberingWidth();
		return dim;
	}

	/**
	 * Paints the whole editor
	 */
	public void paint(Graphics gfx) {
		//check if we draw with anti-aliasing and set it up
		if (mAntiAliasing) {
			Graphics2D gfx2d = (Graphics2D) gfx;
			gfx2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING , RenderingHints.VALUE_ANTIALIAS_ON);
			gfx = gfx2d;
		}
		
		//check if we should draw line number
		if (mDrawLineNumbers) {
			paintLineNumbers(gfx);
			//translate the coordinate system so other paint methods don't have to care
			gfx.translate(getLineNumberingWidth(), 0);
		}
		
		//now draw the main area
		super.paint(gfx);
		
		//draw the invalid lines
		if (mInvalidLinesCount > 0) {
			paintInvalidLines(gfx);
		}
	}
	
	/**
	 * Returns the width of the line numbering on the left side
	 * @return The width of the line numbering
	 */
	public int getLineNumberingWidth() {
		if (!mDrawLineNumbers) {
			return 0;
		}
		int totalLines = possibleMaxLineNo();
		//check how many decimal places the int has and multiply it with max char width + margin
		int numbering = getDecPlaces(totalLines) * fm.charWidth('0') + 3;
		return numbering;
	}
	
	/**
	 * Paints the line numbering on the left
	 * @param gfx The Graphics object to draw on
	 */
	protected void paintLineNumbers(Graphics gfx) {
		int numberingWidth = getLineNumberingWidth();
		Rectangle clipRect = gfx.getClipBounds();
		
		if ( clipRect.x > numberingWidth ) {
			//just draw a region when no numbering is affected
			//wlp:no need to translate it here
			//gfx.translate(numberingWidth, 0);
			//wlp:no need to call super paint, because it will be called later
			//wlp:such behavior cause paint twice.
			//super.paint(gfx);
			return;
		}
		
		//TODO: make color configurable
		gfx.setColor(Color.LIGHT_GRAY);
		int firstLine = textArea.getFirstLine();
		int height = fm.getHeight();
		int firstInvalid = getFirstClipLine(firstLine, height, clipRect);
		int lastInvalid = getLastClipLine(firstLine, height, clipRect);
		
		int clipNumberWidth = numberingWidth - clipRect.x;
		clipNumberWidth = (clipNumberWidth > clipRect.width) ? clipRect.width : clipNumberWidth;
		gfx.fillRect(clipRect.x, clipRect.y, clipNumberWidth, clipRect.height);
		

		
		//TODO: make color configurable
		gfx.setColor(Color.DARK_GRAY);
		gfx.setFont(getFont());
		int maxLines = lastInvalid;
		int existingLines = textArea.getLineCount() - 1;
		maxLines = (maxLines > existingLines) ? existingLines : maxLines;

		// both +1 because we paint natural numbering and begin with 1, not 0
		for(int line = firstInvalid + 1; line <= maxLines + 1; line++)
		{
			String number = Integer.toString(line);
			int numwidth = fm.stringWidth(number);
			//x = u - 2 and y = v - 3 specify the padding
			gfx.drawString(number, numberingWidth - numwidth - 2, (line - firstLine) * height - 3);
		}
		//update clipping region, we took care of the line numbers
		gfx.setClip(numberingWidth, clipRect.y, clipRect.width - clipNumberWidth - 1, clipRect.height);
	}
	
	/**
	 * Paints a red zig-zag stroke under the invalid lines
	 * @param gfx The Graphics object to paint on
	 */
	protected void paintInvalidLines(Graphics gfx) {
		int charHeight = fm.getHeight();
		Element root = textArea.getDocument().getDefaultRootElement();
		int firstLine = textArea.getFirstLine();
		Rectangle clip = gfx.getClipBounds();
		int firstClipLine = getFirstClipLine(firstLine, charHeight, clip);
		int lastClipLine = getLastClipLine(firstLine, charHeight, clip);
		for (int idx = 0; idx < mInvalidLinesCount; idx++) {
			int line = mInvalidLines[idx];
			//check if we are in the clipping region
			if (line < firstClipLine || line > lastClipLine) {
				continue;
			}
			
			//bottom of the line, +3 because the height of the zig-zag is 3
			int baseY = charHeight * (line - firstLine + 1) - 4;
			
			//get length of line in pixels
			Element curLine = root.getElement(line);
			if (curLine == null) {
				//this shouldn't happen, but we don't like crashes
				continue;
			}
			int offset = curLine.getEndOffset() - curLine.getStartOffset();
			int length = textArea._offsetToX(line, offset) + 1; //+1 as we also draw the last pixel
			
			//now we build the zig-zag line as two arrays of x- and y-positions
			if (length < 0) {
				return;
			}
			int xPoints[] = new int[length];
			int yPoints[] = new int[length];
			for (int i = 0; i < length; i++ ) {
				xPoints[i] = i;
				yPoints[i] = baseY + (i % 3);
			}
			gfx.setColor(Color.RED);
			gfx.setXORMode(getBackground());
			gfx.drawPolyline(xPoints, yPoints, length);
		}
	}

	/**
	 * Returns the first line in the clipping region
	 * @param firstLine The first line on the viewport
	 * @param height The height of the viewport
	 * @param clipRect The clipping region
	 * @return The first line in the clipping region that needs to be repainted
	 */
	protected int getFirstClipLine(int firstLine, int height, Rectangle clipRect) {
		// We don't use yToLine() here because that method doesn't
		// return lines past the end of the document
		return firstLine + clipRect.y / height;
	}

	/**
	 * Returns the last line in the clipping region
	 * @param firstLine The first line on the viewport
	 * @param height The height of the viewport
	 * @param clipRect The clipping region
	 * @return The last line in the clipping region that needs to be repainted
	 */
	protected int getLastClipLine(int firstLine, int height, Rectangle clipRect) {
		// Because the clipRect's height is usually an even multiple
		// of the font height, we subtract 1 from it, otherwise one
		// too many lines will always be painted.
		return firstLine + (clipRect.y + clipRect.height - 1) / height;
	}
	
	/**
	 * Returns the number of lines that may be in the area (if you would fill up the whitespace)
	 * @return The number of possible lines (inluding whitespace)
	 */
	protected int possibleMaxLineNo() {
		int totalLines = textArea.getLineCount();
		int possibleLines = textArea.firstLine + textArea.getHeight() / fm.getHeight();
		return (totalLines > possibleLines) ? totalLines : possibleLines;
	}
	
	/**
	 * Returns the number of decimal places the int has. (0-9 have one, 10-99 have 2, ...)
	 * @param i The integer you want to know the decimal places of
	 * @return The number of decimal places i has
	 */
	protected int getDecPlaces(int i) {
		if ( i == 0 )
			return 1;
		int l = 0;
		for(; i > 0; i /= 10) {
			l++;
		}
		return l;
	}
}
