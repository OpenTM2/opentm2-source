/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui;

import java.awt.BorderLayout;
import java.awt.Color;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import net.miginfocom.swing.MigLayout;

import javax.swing.BorderFactory;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JTextField;
import java.awt.Dimension;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;


import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.gui.custom.editor.EditorConfigure;
import de.ibm.com.opentm2scripteride.gui.custom.editor.SyntaxStyle;
import de.ibm.com.opentm2scripteride.utility.Constants;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;

/**
 *Configuration Dialog where general settings 
 *and settings for the editor can be set.
 */
public class ConfigDialog extends JDialog {

	private static final long serialVersionUID = 1L;
	private JTextField txtLog;
	private JTextField txtCommand;
	private JTextField txtSource;
	private JTextField txtExe;
	private JCheckBox eol;


	/**
	 * Create the dialog.
	 */
	public ConfigDialog(JFrame owner, String name, boolean modal) {
		super(owner, name, modal);
		WindowConnector.getInstance().saveActualSettings();
	
		setPreferredSize(new Dimension(710, 450));
		
		setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
		this.addWindowListener(new WindowAdapter() {
		    public void windowClosing(WindowEvent e) {
		        WindowConnector.getInstance().restoreSettings();
		        dispose();
		    }
		});
		
		setMinimumSize(new Dimension(710, 450));
		setBounds(200, 200, 656, 450);
		getContentPane().setLayout(new BorderLayout(0, 0));
		
		JTabbedPane tabbedPane = new JTabbedPane(JTabbedPane.LEFT);
		getContentPane().add(tabbedPane, BorderLayout.CENTER);
		
		//add panel for general settings
		tabbedPane.addTab("General Settings", new ImageIcon("resources/icons/settings_big.png"),
				createGeneralSettings(), "Customice the Script Editor");
		
		//add panel for editor settings
		tabbedPane.addTab("Editor Settings", new ImageIcon("resources/icons/editor_style_big.png"),
				createEditorSettings(), "Configure the OpenTM2 Scripter");
		
		//add save and close button
		getContentPane().add(createSaveCloseButtons(), BorderLayout.SOUTH);		
	}
	
	JPanel createColorSelection(Color color, String type) {
		JPanel panel = new JPanel();
		panel.setMinimumSize(new Dimension(520, 10));
		panel.setBorder(BorderFactory.createTitledBorder("For " + type));   	
    	panel.setLayout(new MigLayout("", "[110px,grow]", "0px []"));
    	panel.add(new ColorSelection(color, type),"cell 0 0");
        return panel;
	}
	
	JPanel createSaveCloseButtons() {
		JPanel panel = new JPanel();
		JButton btnSaveChanges = new JButton("Save Changes and Close");
		panel.add(btnSaveChanges);
		
		JButton btnDiscardChanges = new JButton("Discard Changes and Close");
		panel.add(btnDiscardChanges);
		btnDiscardChanges.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				WindowConnector.getInstance().restoreSettings();
				dispose();
			}
		});
		btnSaveChanges.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
					WindowConnector.getInstance().changeSettings(txtLog.getText(), txtCommand.getText(), txtSource.getText(), txtExe.getText());
					dispose();
					MainApp.getInstance().copyConfig("configuration.conf",MainApp.getInstance().getOpenTM2Properpty()+"configuration.conf.data");
			}
		});
		return panel;
	}
	
	JPanel createGeneralSettings() {
		JPanel gs = new JPanel();
		gs.setLayout(new MigLayout("", "[right][100px:n,grow][50px:n]", "[][][][][][50px:n][][][][]"));
		
		JLabel lblLocationOfLogfile = new JLabel("Location of Logfile:");
		gs.add(lblLocationOfLogfile, "cell 0 0,alignx trailing");
		
		txtLog = new JTextField();
		txtLog.setText(WindowConnector.getInstance().getConf(Constants.logFileLocation));
		gs.add(txtLog, "cell 1 0,growx");
		txtLog.setColumns(10);
		
		JButton btnSelectFile = new JButton("Select File");
		btnSelectFile.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				WindowConnector.getInstance().getPath(txtLog,false);
			}	
		});
		gs.add(btnSelectFile, "cell 2 0,growx");
		
		JLabel lblLocationOfCommandXML = new JLabel("Location of Commands XML:");
		gs.add(lblLocationOfCommandXML, "cell 0 1,alignx trailing");
		
		txtCommand = new JTextField();
		txtCommand.setText(WindowConnector.getInstance().getConf(Constants.commandXMLLocation));
		gs.add(txtCommand, "cell 1 1,growx");
		txtCommand.setColumns(10);
		
		JButton btnSelectFile_1 = new JButton("Select File");
		btnSelectFile_1.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				WindowConnector.getInstance().getPath(txtCommand,false);
			}
		});
		gs.add(btnSelectFile_1, "cell 2 1,growx");
		
		JLabel lblDirectoryForSource = new JLabel("Directory for Scripts");
		gs.add(lblDirectoryForSource, "cell 0 2,alignx trailing");
		
		txtSource = new JTextField();
		txtSource.setText(WindowConnector.getInstance().getConf(Constants.scriptDirectory));
		gs.add(txtSource, "cell 1 2,growx");
		txtSource.setColumns(10);
		
		JButton btnSelectFolder_1 = new JButton("Select Folder");
		btnSelectFolder_1.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				WindowConnector.getInstance().getPath(txtSource,true);
			}
		});
		gs.add(btnSelectFolder_1, "cell 2 2,growx");
		
		JLabel lblLocationOfOpentmscripterexe = new JLabel("Location of OpenTm2Scripter.exe");
		gs.add(lblLocationOfOpentmscripterexe, "cell 0 3,alignx trailing");
		
		txtExe = new JTextField();
		txtExe.setText(WindowConnector.getInstance().getConf(Constants.openTM2scripterLocation));
		gs.add(txtExe, "cell 1 3,growx");
		txtExe.setColumns(10);
		
		JButton btnSelectFile_2 = new JButton("Select File");
		btnSelectFile_2.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				WindowConnector.getInstance().getPath(txtExe,false);
			}
		});
		gs.add(btnSelectFile_2, "cell 2 3,growx");
		return gs;
	}
	
	public JScrollPane createEditorSettings() {
		EditorConfigure gc = EditorConfigure.getInstance();
		
		
		JScrollPane scroll = new JScrollPane();
		scroll.setBorder(null);
		
		JPanel es = new JPanel();
		es.setBorder(null);
		scroll.setViewportView(es);
		
		es.setLayout(new MigLayout("", "[grow][]", "[]"));
		
		JLabel lblMakeTheGeneral = new JLabel(" Make the general Font Setttings for the Editor:");
		es.add(lblMakeTheGeneral, "cell 0 0");
		
		FontSelector generalFont = new FontSelector(gc.getFont());
		es.add(generalFont, "cell 0 1 1 2");
		
		int row = 3;
		JLabel lblPersionalizeTheHighlighting = new JLabel("Persionalize the Syntax-Highlighting of the Editor:");
		es.add(lblPersionalizeTheHighlighting, "cell 0 " + row++);
		
		SyntaxStyle[] highlighting = { gc.getStyleVariable(), gc.getStyleComments(), gc.getStyleApiCalls(), gc.getStyleControlStructures(),
				gc.getStyleFunctions() };
		int i = 0;
		for (; i < highlighting.length; i++) {
			es.add(new Highlighting(highlighting[i], Constants.highlightingconstants[i]), "cell 0 " + row++);
		}
		
		es.add(createColorSelection(gc.getSelectionColor(),Constants.highlightingconstants[i++]), "cell 0 " + row++);
		es.add(createColorSelection(gc.getLineHighlightColor(),Constants.highlightingconstants[i++]), "cell 0 " + row++);
		
		JLabel checkBoxEOL = new JLabel("enable End-Of-Line character: ");
		es.add(checkBoxEOL, "cell 0 11");
		
		eol = new JCheckBox();
		eol.setSelected(gc.getEOLChar());
		eol.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent arg0) {
				EditorConfigure.getInstance().setEOLChar(eol.isSelected());
			}
		});
		es.add(eol, "cell 0 11");
		return scroll;
	}

}
