/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui;

import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

import de.ibm.com.opentm2scripteride.gui.custom.editor.SyntaxStyle;
import net.miginfocom.swing.MigLayout;
 

 
/**
 * Highlighting class is responsible for syntax changes (bold/italic)
 * for the diverse highlightingtypes of the editor.
 */
public class Highlighting extends JPanel {
 
	private static final long serialVersionUID = 1L;
	
	private JLabel lblStyle;
	
	private JToggleButton tglbtnB;
	private JToggleButton tglbtnI;
	
	private boolean bold, italic;
	private String highlightingtype;
	
	private ColorSelection colSelector;

    
    public Highlighting(SyntaxStyle actualStyle, String type){
    	setMinimumSize(new Dimension(520, 10));
    	setBorder(BorderFactory.createTitledBorder("For " + type));
    	bold = actualStyle.isBold();
    	italic = actualStyle.isItalic();
    	highlightingtype = type;
    	colSelector = new ColorSelection(actualStyle.getColor(),type);
    	init();
    }
    
    public void init() {
    	
    	setLayout(new MigLayout("", "[110px,grow][80px,grow][28px,grow]", "0px []"));
       
        add(colSelector,"cell 0 1");
        
        
        lblStyle = new JLabel("  Choose Style:");
        add(lblStyle, "flowx,cell 1 1,alignx right,aligny center");
        
        tglbtnB = new JToggleButton("b");
        tglbtnB.setSelected(bold);
        tglbtnB.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
        		bold = tglbtnB.isSelected();
        		WindowConnector.getInstance().setHighlightingtype(highlightingtype, bold, italic);
        	}
        });
        add(tglbtnB, "cell 2 1");
        
        tglbtnI = new JToggleButton("i");
        tglbtnI.setSelected(italic);
        tglbtnI.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
        		italic = tglbtnI.isSelected();
        		WindowConnector.getInstance().setHighlightingtype(highlightingtype, bold, italic);
        	}
        });
        add(tglbtnI, "cell 2 1");

        
    }
}
 
 

