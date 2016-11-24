/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui;


import java.awt.BorderLayout;
import java.awt.Color;

import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JTextPane;

import java.awt.Dimension;

import de.ibm.com.opentm2scripteride.MainApp;

import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.text.StyleContext;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.Component;
import java.awt.FlowLayout;
import java.io.IOException;

import javax.swing.text.StyleConstants;



/**
 * About Window of the openTM2ScripterIDE
 */
public class AboutWindow extends JDialog {

	private static final long serialVersionUID = 1L;

	/**
	 * Create the dialog.
	 */
	public AboutWindow(JFrame owner, String name, boolean modal) {
		super(owner, name, modal);
		WindowConnector.getInstance().saveActualSettings();
	
		setPreferredSize(new Dimension(710, 450));
			
		setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);

		
		setMinimumSize(new Dimension(300, 270));
		setBounds(200, 200, 450, 250);
		getContentPane().setLayout(new BorderLayout(0, 0));
		
		
		StyleContext.NamedStyle centerStyle = StyleContext.getDefaultStyleContext().new NamedStyle();
		StyleConstants.setAlignment(centerStyle,StyleConstants.ALIGN_LEFT);

		JTextPane textArea = new JTextPane();
		textArea.setLogicalStyle(centerStyle);
		getContentPane().add(textArea, BorderLayout.CENTER);
		textArea.setText(getAboutText());
		textArea.setBackground(new Color(238,238,238));
		textArea.setEditable(false);
		
		JPanel panel = new JPanel();
		panel.setBackground(new Color(238,238,238));
		getContentPane().add(panel, BorderLayout.SOUTH);
		
		JButton btnOK = new JButton("OK");
		btnOK.setAlignmentY(Component.TOP_ALIGNMENT);
		btnOK.setAlignmentX(Component.RIGHT_ALIGNMENT);
		btnOK.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				dispose();
			}
		});
		panel.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		panel.add(btnOK);
		
		
	}
	
	
	/** Gets the String which shall be displayed in the about window
	 * @return String to display
	 */
	private String getAboutText(){
		/*String about = 	MainApp.getInstance().getAppName()+" v"+MainApp.getInstance().getVersion()+"\n\n"+
						"A graphical user interface to create and run openTM2 scripts\n\n"+
						"This product includes icons from:\n"+
						"famfamfam.com/lab/icons/silk/\n\n"+
						"(c)Copyright IBM Corporation 2012, IBM Translation Technical Services\n"+
						"IBM Authorized Use Only";*/

		try {
			String version = MainApp.getInstance().getVersion();
			
			StringBuilder sbAbout = new StringBuilder();
			sbAbout.append("\n     OpenTM2Scripter GUI, a graphical user interface to\n")
			       .append("     create and run OpenTM2 scripts.\n\n")
			       .append("     Copyright(C) 2012-2016,\n")
			       .append("     International Business Machines Corporation and others.\n")
			       .append("     All rights reserved.\n")
			       .append("     Version ").append(version).append("\n\n")
			       .append("     This product includes icons from:\n")
			       .append("     famfam.com/lab/icons.silk/");
			return sbAbout.toString();			
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		return "";
	}
	

}

