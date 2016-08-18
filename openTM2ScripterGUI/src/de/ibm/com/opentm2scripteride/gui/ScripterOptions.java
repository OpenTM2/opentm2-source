/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

@SuppressWarnings("serial")
public class ScripterOptions extends JPanel{
	
	private RunConsolePanel RTS;
	private JCheckBox cbxDelTmp;
	private JCheckBox cbxHelp;
	private JCheckBox cbxDate;
	private JCheckBox cbxID;
	
	private JLabel lbLogLevel = new JLabel("Log level");
	private JComboBox<String> cbLogs;
	private JLabel lbLogFile = new JLabel("Log file");
	private JTextField tfLogFile = new JTextField(20);
	
	private JLabel lbConst;
	private JLabel lbExample;
	private JTextField tfConst;
	private JButton btnSave;
	private Boolean bolDelTmp;
	private Boolean bolHelp;
	private Boolean bolDate;
	private Boolean bolID;

	private String logLevel;
	
	public final String[] logs = new String[]{"","DEBUG","INFO","WARNING","ERROR","NO"};
	
	public ScripterOptions(RunConsolePanel pRTS){
		super();
		if(this.initPanel()){
			this.RTS = pRTS;
			
			this.cbxHelp.setSelected(this.RTS.getBolShowHelp());
            this.cbxDelTmp.setSelected(this.RTS.getBolDelTmp());
            this.cbxDate.setSelected(this.RTS.getBolDate());
            this.cbxID.setSelected(this.RTS.getBolID());
            this.tfLogFile.setText(this.RTS.getLogFile());
            this.tfConst.setText(this.RTS.getConstants());
            this.cbLogs.setSelectedItem(this.RTS.getLogLevel());
            
    		if(cbxHelp.isSelected()) {
    			this.tfConst.setEnabled(false);
    			this.tfLogFile.setEnabled(false);
    			this.cbLogs.setEnabled(false);
    		}
    			
		}
	}
	
	private boolean initPanel() {
		// Action Panel
		this.setLayout(new GridBagLayout());
		this.setBorder(BorderFactory.createTitledBorder("Scripter Options"));
		this.bolDelTmp = false;
		this.bolHelp = false;
		this.bolDate = false;
		this.bolID = false;
		this.logLevel = null;
		
		this.cbxDelTmp = new JCheckBox("Delete temporary scriptfile");
		this.cbxDelTmp.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent arg0) {
				if(!cbxHelp.isSelected()){
				if(cbxDelTmp.isSelected())
					bolDelTmp = true;
				else
					bolDelTmp = false;
				RTS.setBolDelTmp(bolDelTmp);
				}else{
					cbxDelTmp.setSelected(false);
				}
			}
		});
		
		this.cbxHelp = new JCheckBox("Show help");
		this.cbxHelp.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent e) {
				if(cbxHelp.isSelected()){
					bolHelp = true;
					cbxDate.setSelected(false);
					cbxID.setSelected(false);
					cbxDelTmp.setSelected(false);
					bolDelTmp = false;
					bolDate = false;
					bolID = false;
					logLevel = null;
					
				}else{
					bolHelp = false;
					
				}
				
				RTS.setBolShowHelp(bolHelp);
				tfConst.setEnabled(!bolHelp);
    			tfLogFile.setEnabled(!bolHelp);
    			cbLogs.setEnabled(!bolHelp);
			}
		});
		
		this.cbxDate = new JCheckBox("Show Date and Time");
		this.cbxDate.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent e) {
				if(!cbxHelp.isSelected()){
				if(cbxDate.isSelected()){
					bolDate=true;
				}else{
					bolDate=false;
				}}else{
					cbxDate.setSelected(false);
				}
				RTS.setBolDate(bolDate);
			}
		});
		this.cbxID = new JCheckBox("Show Process ID");
		this.cbxID.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent e) {
				if(!cbxHelp.isSelected()){
				if(cbxID.isSelected()){
					bolID = true;
				}else{
					bolID = false;
				}}else{
					cbxID.setSelected(false);
				}
				RTS.setBolID(bolID);
			}
		});
		
		this.cbLogs = new JComboBox<String>(logs);
		this.cbLogs.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent e) {
				if(!cbxHelp.isSelected()){
					logLevel = (String) cbLogs.getSelectedItem();
					RTS.setLogLevel(logLevel);
				}
			}
		});
		
	   
	    
		this.lbConst = new JLabel( "Constants ");
		this.tfConst = new JTextField(20);
		tfConst.addFocusListener(new FocusAdapter(){
			public void focusLost(FocusEvent e){
				RTS.setConstants(tfConst.getText());
			}
		});
		this.lbExample = new JLabel("Example: \"/constant=Test,15\" ");

		
		tfLogFile.addFocusListener(new FocusAdapter(){
			public void focusLost(FocusEvent e){
				RTS.setLogFile(tfLogFile.getText());
			}
		});
		
		this.btnSave = new JButton("Save");
		this.btnSave.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent e) {
				RTS.setOptions(bolHelp, bolDelTmp, bolDate, bolID, logLevel, tfLogFile.getText(), tfConst.getText());
                
			}
		});
		
		GridBagConstraints gbc = new GridBagConstraints();

        gbc.anchor = GridBagConstraints.WEST;
        gbc.gridwidth=GridBagConstraints.REMAINDER;
        gbc.fill=GridBagConstraints.HORIZONTAL;
        gbc.ipady = 3;
		this.add(this.cbxHelp, gbc);
		
		this.add(this.cbxDelTmp, gbc);
		
		this.add(this.cbxDate, gbc);
		
		this.add(this.cbxID, gbc);
		
		gbc.gridwidth = 1;
		this.add(this.lbLogLevel, gbc);
		gbc.gridwidth=GridBagConstraints.REMAINDER;  
		gbc.weighty = 0.75;
		this.add(this.cbLogs,gbc);
		
		gbc.gridwidth = 1;
		this.add(this.lbLogFile, gbc);
		gbc.gridwidth=GridBagConstraints.REMAINDER;  
		gbc.weighty = 0.75;
		this.add(this.tfLogFile,gbc);

		gbc.gridwidth = 1;
		this.add(this.lbConst,gbc); 
		gbc.gridwidth=GridBagConstraints.REMAINDER;  
		this.add(this.tfConst,gbc);
		
		gbc.weighty = 0.75;
		this.add(this.lbExample,gbc);
		
		//gbc.gridy = 6;
//		gbc.fill = GridBagConstraints.NONE;
//		gbc.anchor = GridBagConstraints.CENTER;
//		gbc.weighty = 0.75;
//		gbc.weighty = 1.0;
//		this.add(this.btnSave, gbc);
		
		return true;
	}

}
