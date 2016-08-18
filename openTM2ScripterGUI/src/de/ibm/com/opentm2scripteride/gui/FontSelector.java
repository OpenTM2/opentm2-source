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
import javax.swing.event.*;

 

 
/**
 * Panel with a Font selector for font changes 
 * of the general font of the editor
 */
public class FontSelector extends JPanel {
 
	private static final long serialVersionUID = 1L;
	
	JComboBox<Object> font;
	JToggleButton tglbtnB,tglbtnI;
    JSpinner sizes;
    
    boolean bold,italic;
    String fontChoice = "";
    int sizeChoice = 12;
 
    public FontSelector(Font actualfont){
    	setMinimumSize(new Dimension(500, 50));
    	setBorder(null);
    	fontChoice = actualfont.getName();
    	bold = actualfont.isBold();
    	italic = actualfont.isItalic();
    	sizeChoice = actualfont.getSize();
    	init();
    }
    
    public void init() {
        add(new JLabel("Font family:"));
 
        GraphicsEnvironment environment =
            GraphicsEnvironment.getLocalGraphicsEnvironment();
        font = new JComboBox<Object>(environment.getAvailableFontFamilyNames());
        font.setSelectedItem(fontChoice);
        font.setMaximumRowCount(5);
        font.addItemListener(new ItemListener() {
			public void itemStateChanged(ItemEvent e) {
				 if (e.getStateChange() != ItemEvent.SELECTED) {
			            return;
			        }
			        fontChoice = (String)font.getSelectedItem();
			        WindowConnector.getInstance().setGeneralFont(fontChoice, bold, italic, sizeChoice);
			}
		});;
        add(font);
 
        
        
        
        
        
        add(new JLabel("Choose Style:"));
        
        tglbtnB = new JToggleButton("b");
        tglbtnB.setSelected(bold);
        tglbtnB.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
        		bold = tglbtnB.isSelected();
        		WindowConnector.getInstance().setGeneralFont(fontChoice, bold, italic, sizeChoice);
        	}
        });
        tglbtnB.setFocusPainted(false);
        add(tglbtnB);
        
        tglbtnI = new JToggleButton("i");
        tglbtnI.setSelected(italic);
        tglbtnI.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent e) {
        		italic = tglbtnI.isSelected();
        		WindowConnector.getInstance().setGeneralFont(fontChoice, bold, italic, sizeChoice);
        	}
        });
        tglbtnI.setFocusPainted(false);
        add(tglbtnI);
 
        
        
        
        
        add(new JLabel("Size:"));
 
        sizes = new JSpinner(new SpinnerNumberModel(sizeChoice, 6, 32, 1));
        sizes.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent e) {
				 try {
			            String size = sizes.getModel().getValue().toString();
			            sizeChoice = Integer.parseInt(size);
			            WindowConnector.getInstance().setGeneralFont(fontChoice, bold, italic, sizeChoice);
			        } catch (NumberFormatException nfe) {
			        }
			}
		});
        add(sizes);
    }	
}
 
 

