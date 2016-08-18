/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui;


import java.awt.BorderLayout;
import javax.swing.JDialog;
import javax.swing.JFrame;

import java.awt.Dimension;


/**
 * The Help Dialog, with help content for users.
 */
public class HelpDialog extends JDialog {

	private static final long serialVersionUID = 1L;

	/**
	 * Create the dialog.
	 */
	public HelpDialog(JFrame owner, String name, boolean modal) {
		super(owner, name, modal);
		WindowConnector.getInstance().saveActualSettings();
	
		setPreferredSize(new Dimension(710, 450));

		setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);

		
		setMinimumSize(new Dimension(400, 300));
		setBounds(200, 200, 656, 450);
		getContentPane().setLayout(new BorderLayout(0, 0));
		
		// TODO here should be some content for the Help Dialog
		
	}
	

}
