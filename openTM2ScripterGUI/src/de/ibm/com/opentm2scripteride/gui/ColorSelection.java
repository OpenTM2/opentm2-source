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
import net.miginfocom.swing.MigLayout;
 

 
/**
 * ColorSelection is a Panel with a JColorChooser to change 
 * diverse highlighting colors of the editor.
 */
public class ColorSelection extends JPanel {
 
	private static final long serialVersionUID = 1L;
	private JLabel lblChooseOwnColor;
	
    private Color colorChoice;
	private String highlightingtype;
	private JLabel lblActualColor;
	private JPanel colorpanel;
	private JButton btnChoose;
	

    
    public ColorSelection(Color color, String type){
    	setMinimumSize(new Dimension(300, 10));
    	colorChoice = color;
    	highlightingtype = type;
    	init();
    }
    
    public void init() {
    	setLayout(new MigLayout("", "[110px][80px][28px][grow]", "[20px]"));
        
        lblActualColor = new JLabel("actual Color:");
        add(lblActualColor, "flowx,cell 0 0,alignx right,aligny center");
        
        btnChoose = new JButton("choose");
        btnChoose.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Color newcolorChoice = JColorChooser.showDialog(getComponentPopupMenu(), "choose "+ highlightingtype +" Color", colorChoice);
				if(newcolorChoice != null){
					colorChoice = newcolorChoice;
					WindowConnector.getInstance().setHighlightingColor(highlightingtype, colorChoice);
					colorpanel.setBackground(colorChoice);
				}
			}
		});
        
        lblChooseOwnColor = new JLabel("Choose other Color:");
        add(lblChooseOwnColor, "cell 1 0,alignx right,aligny center");
        add(btnChoose, "cell 2 0,alignx left,aligny top");
        
           
       colorpanel = new JPanel();
       colorpanel.setMinimumSize(new Dimension(15, 15));
       colorpanel.setBackground(colorChoice);
       add(colorpanel, "cell 0 0,alignx right,aligny center");
        
    }
}
 
 

