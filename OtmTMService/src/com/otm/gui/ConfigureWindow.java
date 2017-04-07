//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.gui;

import java.awt.Component;

import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;
import javax.swing.JComboBox;
import javax.swing.JButton;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Map;

import javax.swing.JPasswordField;

import com.otm.util.CfgUtil;

public class ConfigureWindow {

	//private JFrame frmConfigureWindow;
	public final JDialog frmConfigureWindow = new JDialog(new JFrame(), true); 
	private JTextField textDbInstalledDir;
	private JPasswordField pfRootPassword;
	private JTextField tfServerPort;
	
	/**
	 * Launch the application.
	 */
	public static void show() {
		try {
			ConfigureWindow window = new ConfigureWindow();
			window.frmConfigureWindow.setVisible(true);
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
   
	/**
	 * Create the application.
	 */
	public ConfigureWindow() {
		initialize();
	}

	/**
	 * Initialize the contents of the frame.
	 */
	private void initialize() {
		//frmConfigureWindow = new JFrame();
		frmConfigureWindow.setResizable(false);
		frmConfigureWindow.setTitle("Configure Window");
		frmConfigureWindow.setBounds(100, 100, 670, 368);
		//frmConfigureWindow.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frmConfigureWindow.getContentPane().setLayout(null);
		
		JLabel lblDbDir = new JLabel("DB Installed Directory");
		lblDbDir.setBounds(37, 39, 154, 15);
		frmConfigureWindow.getContentPane().add(lblDbDir);
		
		JLabel lblRootPassword = new JLabel("DB Root password");
		lblRootPassword.setBounds(37, 77, 139, 15);
		frmConfigureWindow.getContentPane().add(lblRootPassword);
		
		JLabel lblServerIP = new JLabel("Server IP");
		lblServerIP.setBounds(37, 118, 54, 15);
		frmConfigureWindow.getContentPane().add(lblServerIP);
		
		textDbInstalledDir = new JTextField();
		textDbInstalledDir.setBounds(201, 36, 317, 21);
		frmConfigureWindow.getContentPane().add(textDbInstalledDir);
		textDbInstalledDir.setColumns(10);
		
		if(CfgUtil.getInstance().getDbCfg().get("db_installed_dir") != null)
			textDbInstalledDir.setText( CfgUtil.getInstance().getDbCfg().get("db_installed_dir") );
		
		final JComboBox<String> cbServerIP = new JComboBox<String>();
		cbServerIP.setBounds(201, 115, 317, 21);
		frmConfigureWindow.getContentPane().add(cbServerIP);
		{
			cbServerIP.addItem("localhost");
			
			InetAddress addr = null;
			try {
				addr = InetAddress.getLocalHost();
				String ip=addr.getHostAddress().toString();
				cbServerIP.addItem(ip);
			} catch (UnknownHostException e) {
				e.printStackTrace();
			}
			
			if(CfgUtil.getInstance().getDbCfg().get("server_ip") != null) {
				cbServerIP.setSelectedItem(CfgUtil.getInstance().getDbCfg().get("server_ip"));
			}
		}
		
		JButton btnSave = new JButton("Save");
		btnSave.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				Component[] compotents = frmConfigureWindow.getContentPane().getComponents();
				for(Component cm:compotents){
					if( cm instanceof JTextField ){
						String txt = ((JTextField) cm).getText();
						if(txt == null || txt.isEmpty()) {
							JOptionPane.showMessageDialog(null, "All input text box shouldn't be empty!", "OtmTMService", JOptionPane.INFORMATION_MESSAGE);
							return;
						}
					}
				}//
				
				Map<String, String> cfgs = CfgUtil.getInstance().getDbCfg();
				cfgs.put("db_installed_dir", textDbInstalledDir.getText());
				
				if( cfgs.get("db_installed_dir").toLowerCase().indexOf("mariadb") == -1 ){
				    cfgs.put("driver_class", "com.mysql.jdbc.Driver");	
				} else {
				    cfgs.put("driver_class", "org.mariadb.jdbc.Driver");	
				}
				
				cfgs.put("root_password",String.valueOf(pfRootPassword.getPassword()));
				cfgs.put("server_ip", (String) cbServerIP.getSelectedItem());
				cfgs.put("server_port", (String) tfServerPort.getText());
				if( CfgUtil.getInstance().saveToXml() ) {
					frmConfigureWindow.dispose();
				}
			}
		});
		btnSave.setBounds(201, 204, 70, 23);
		frmConfigureWindow.getContentPane().add(btnSave);
		
		JButton btnClose = new JButton("Exit");
		btnClose.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				frmConfigureWindow.dispose();
			}
		});
		btnClose.setBounds(358, 204, 70, 23);
		frmConfigureWindow.getContentPane().add(btnClose);
		
		JButton btnBrowser = new JButton("Browse");
		btnBrowser.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				 JFileChooser chooser = new JFileChooser();
			     chooser.setFileSelectionMode( JFileChooser.DIRECTORIES_ONLY);
			     int returnVal = chooser.showOpenDialog(null);
			     if(returnVal == JFileChooser.APPROVE_OPTION) {
			    	 textDbInstalledDir.setText(chooser.getSelectedFile().getPath());
			    }
			}
		});
		btnBrowser.setBounds(541, 35, 83, 23);
		frmConfigureWindow.getContentPane().add(btnBrowser);
		
		pfRootPassword = new JPasswordField();
		pfRootPassword.setBounds(201, 74, 317, 21);
		frmConfigureWindow.getContentPane().add(pfRootPassword);
		if( CfgUtil.getInstance().getDbCfg().get("root_password")!=null )
			pfRootPassword.setText( CfgUtil.getInstance().getDbCfg().get("root_password") );
		
		
		JLabel lblNewLabel = new JLabel("Server Port");
		lblNewLabel.setBounds(37, 162, 139, 15);
		frmConfigureWindow.getContentPane().add(lblNewLabel);
		
		tfServerPort = new JTextField();
		tfServerPort.setBounds(201, 159, 317, 21);
		frmConfigureWindow.getContentPane().add(tfServerPort);
		tfServerPort.setColumns(10);
		if( CfgUtil.getInstance().getDbCfg().get("server_port")!=null )
			tfServerPort.setText( CfgUtil.getInstance().getDbCfg().get("server_port") );
		
	}
}
