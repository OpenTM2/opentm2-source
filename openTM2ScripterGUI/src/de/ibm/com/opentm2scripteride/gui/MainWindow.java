/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui;
import java.awt.BorderLayout;
import java.awt.Insets;
import java.awt.Toolkit;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.JMenuBar;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JSplitPane;
import javax.swing.JTabbedPane;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.UIManager;
import java.awt.Component;
import net.miginfocom.swing.MigLayout;
import javax.swing.JButton;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.InputEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.gui.custom.FileTree;
import de.ibm.com.opentm2scripteride.gui.custom.ScriptOutlineTree;
import de.ibm.com.opentm2scripteride.gui.custom.editor.CodeCompleter;
import de.ibm.com.opentm2scripteride.gui.custom.editor.CodeCompleterList;
import de.ibm.com.opentm2scripteride.gui.custom.editor.TabTextEditor;
import de.ibm.com.opentm2scripteride.utility.Configuration;
import de.ibm.com.opentm2scripteride.utility.Constants;
import javax.swing.JScrollPane;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.ImageIcon;
import java.awt.Dimension;
import java.util.Locale;
import java.awt.event.KeyEvent;


/**
 * The main window of the openTM2Scripter IDE
 */
public class MainWindow extends JFrame {

	private static final long serialVersionUID = 1L;
	private JPanel contentPane;
	private JPanel mEditorPane;
	private JTextField searchField;
	private ScriptOutlineTree mOutlineTree; 
	private FileTree mFileTree;
	private JScrollPane scrollablefiletree;
	private JScrollPane scrollableoutlinetree;
	private JTabbedPane mFileFunctionTabs;
	private JPanel mFunctionsPanel;
	private RunConsolePanel mRunTS;
	private JSplitPane mCreateTSleft;
	private JSplitPane mCreateTSright;
	
	private JToolBar mToolBar;
	
	private JButton btnNewScript;
	private JButton btnOpenFile;
	private JButton btnSave;
	private JButton btnSaveAs;
	private JButton btnConfiguration;
	private JButton btnRun;
	private JButton btnStop;
	private JButton btnOpenLog;
	private JButton btnOptions;
	private JButton btnGenerateReport;
	private JMenu mnHelp;
	private JMenuItem mntmHelpContent;
	private JMenuItem mntmAbout;
	private JToolBar mBottomBar;
	private JTabbedPane mFilesTabPane;
	private JPopupMenu mTabPopMenu;
	private CodeCompleterList mCcl;
	
	/**
	 * Initializes the main window with a title
	 * @param title of the main window
	 */
	public MainWindow(String title) {
		setLocale(Locale.ENGLISH);
		setTitle(title);
		setIconImage(new ImageIcon("resources/icons/apicall.png").getImage());
		
//		try {
//			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
//		} catch (Exception e) {
//		}
		
		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		this.addWindowListener(new WindowAdapter() {
		    public void windowClosing(WindowEvent e) {
		        WindowConnector.getInstance().closeOperation();
		    }
		});
		//setBounds(100, 100, 1000, 500);
		
		/*
		 * Create the menu
		 */

		JMenuBar menuBar = new JMenuBar();
		setJMenuBar(menuBar);
		
		JMenu mnFile = new JMenu("File");
		mnFile.setMnemonic(KeyEvent.VK_F);
		menuBar.add(mnFile);
		
		JMenuItem mntmNewFile = new JMenuItem("New", KeyEvent.VK_N);
		mntmNewFile.setIcon(new ImageIcon("resources/icons/open.png"));
		KeyStroke ctrlNKeyStroke = KeyStroke.getKeyStroke("control N");
		mntmNewFile.setAccelerator(ctrlNKeyStroke);
		mntmNewFile.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().newScript();
			}
		});
		mnFile.add(mntmNewFile);
		
		JMenuItem mntmOpenFile = new JMenuItem("Open File", KeyEvent.VK_O);
		mntmOpenFile.setIcon(new ImageIcon("resources/icons/open.png"));
		KeyStroke ctrlOKeyStroke = KeyStroke.getKeyStroke("control O");
		mntmOpenFile.setAccelerator(ctrlOKeyStroke);
		mntmOpenFile.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().openFileDialog();
			}
		});
		mnFile.add(mntmOpenFile);
		
		
		final JMenuItem mntmClose = new JMenuItem("Close", KeyEvent.VK_W);
		mntmClose.setIcon(new ImageIcon("resources/icons/close_tab.png"));
		KeyStroke ctrlWKeyStroke = KeyStroke.getKeyStroke(KeyEvent.VK_W,ActionEvent.CTRL_MASK);
		mntmClose.setAccelerator(ctrlWKeyStroke);
		mntmClose.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().closeFile(mFilesTabPane,mFilesTabPane.getSelectedIndex());
			}
		});
		mnFile.add(mntmClose);
				
		final JMenuItem mntmCloseAll = new JMenuItem("Close All...");
		KeyStroke ctrlCAKeyStroke = KeyStroke.getKeyStroke(KeyEvent.VK_W,InputEvent.CTRL_MASK|InputEvent.SHIFT_MASK);
		mntmCloseAll.setAccelerator(ctrlCAKeyStroke);
		mntmCloseAll.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().closeAll(mFilesTabPane);
			}
		});
		mnFile.add(mntmCloseAll);
		
		JMenuItem mntmSave = new JMenuItem("Save", KeyEvent.VK_S);
		mntmSave.setIcon(new ImageIcon("resources/icons/save.png"));
		KeyStroke ctrlSKeyStroke = KeyStroke.getKeyStroke("control S");
		mntmSave.setAccelerator(ctrlSKeyStroke);
		mntmSave.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().saveFile();
			}
		});
		mnFile.add(mntmSave);
		
		JMenuItem mntmSaveAs = new JMenuItem("Save As...", new ImageIcon("resources/icons/save_as.png"));
		mntmSaveAs.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().saveFileAs();
			}
		});
		mnFile.add(mntmSaveAs);
		
		JMenuItem mntmConfiguration = new JMenuItem("Configuration", new ImageIcon("resources/icons/settings.png"));
		mntmConfiguration.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().createConfigDialog();
			}
		});
		mnFile.add(mntmConfiguration);
		
		JMenuItem mntmExit = new JMenuItem("Exit", new ImageIcon("resources/icons/close.png"));
		mntmExit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				WindowConnector.getInstance().closeOperation();
			}
		});
		mnFile.add(mntmExit);
		
		mnHelp = new JMenu("Help");
		mnHelp.setMnemonic(KeyEvent.VK_H);
		menuBar.add(mnHelp);
		
		mntmHelpContent = new JMenuItem("Help Content", new ImageIcon("resources/icons/help.png"));
		mntmHelpContent.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().createHelpDialog();
			}
		});
		mnHelp.add(mntmHelpContent);
		/*
		 * TODO a Helpwindow Frame is available but has to be implemented completely
		 * after the implementation this button has to be enabled
		 */
		mntmHelpContent.setEnabled(false);
		
		
		mntmAbout = new JMenuItem("About", new ImageIcon("resources/icons/apicall.png"));
		mntmAbout.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().createAboutDialog();
			}
		});
		mnHelp.add(mntmAbout);
		
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		contentPane.setLayout(new BorderLayout(0, 0));
		setContentPane(contentPane);
		
		
		
		/*
		 * Create the ToolBar
		 */

		mToolBar = new JToolBar();
		contentPane.add(mToolBar, BorderLayout.NORTH);
		
		/*
		 * Create Test Suite ToolBar
		 */

		btnNewScript = new JButton("New");
		btnNewScript.setIcon(new ImageIcon("resources/icons/open.png"));
		mToolBar.add(btnNewScript);
		btnNewScript.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().newScript();
			}
		});
		
		btnOpenFile = new JButton("Open File");
		btnOpenFile.setIcon(new ImageIcon("resources/icons/open.png"));
		mToolBar.add(btnOpenFile);
		btnOpenFile.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().openFileDialog();
			}
		});
		
		btnSave = new JButton("Save");
		btnSave.setIcon(new ImageIcon("resources/icons/save.png"));
		mToolBar.add(btnSave);
		btnSave.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().saveFile();
			}
		});
				
		btnSaveAs = new JButton("Save As...");
		btnSaveAs.setIcon(new ImageIcon("resources/icons/save_as.png"));
		mToolBar.add(btnSaveAs);
		btnSaveAs.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {			
				WindowConnector.getInstance().saveFileAs();
			}
		});
		
		//
		btnGenerateReport = new JButton("Report");
		btnGenerateReport.setIcon(new ImageIcon("resources/icons/report.png"));
		mToolBar.add(btnGenerateReport);
		btnGenerateReport.addActionListener(new ActionListener() {

					@Override
					public void actionPerformed(ActionEvent arg0) {
						WindowConnector.getInstance().generateReport();
					}

		});
		btnGenerateReport.setEnabled(false);
		
		
		btnOpenLog = new JButton("Open Log");
		btnOpenLog.setIcon(new ImageIcon("resources/icons/openlog.png"));
		mToolBar.add(btnOpenLog);
		btnOpenLog.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().openLogWindow();
			}
		});
		
		
		btnConfiguration = new JButton("Configuration");
		btnConfiguration.setIcon(new ImageIcon("resources/icons/settings.png"));
		mToolBar.add(btnConfiguration);
		btnConfiguration.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().createConfigDialog();
			}
		});
		
	
		btnRun = new JButton("Run");
		btnRun.setIcon(new ImageIcon("resources/icons/start.png"));
		mToolBar.add(btnRun);
		btnRun.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				mRunTS.getTextArea().setText("");
				WindowConnector.getInstance().runTS(mRunTS,mFilesTabPane, mFilesTabPane.getSelectedIndex());
			}
		});
		
		btnStop = new JButton("Stop");
		btnStop.setEnabled(false);
		btnStop.setIcon(new ImageIcon("resources/icons/stop.png"));
		mToolBar.add(btnStop);
		btnStop.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().stopProcess();
			}
			
		});
		
		
		btnOptions = new JButton("Options");
		btnOptions.setIcon(new ImageIcon("resources/icons/options.png"));
		mToolBar.add(btnOptions);
		btnOptions.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().runTSOptions(mRunTS);
			}
		});

			
		mEditorPane = new JPanel();
		mEditorPane.setLayout(new BorderLayout(0,0));

		contentPane.add(mEditorPane, BorderLayout.CENTER);
		
		mCreateTSleft = new JSplitPane();
		mCreateTSleft.setContinuousLayout(true);
		mCreateTSleft.setResizeWeight(0.03);
		mCreateTSleft.setDividerSize(3);
		mCreateTSright = new JSplitPane();
		mCreateTSright.setContinuousLayout(true);
		mCreateTSright.setResizeWeight(0.80);
		mCreateTSright.setDividerSize(3);
		//mTabbedPane.addTab("Create Test Suite", new ImageIcon("resources/icons/editor.png"), mCreateTSleft, null);
		mEditorPane.add( mCreateTSleft,BorderLayout.CENTER);
		
		mFileFunctionTabs = new JTabbedPane(JTabbedPane.TOP);
		mFileFunctionTabs.setMinimumSize(new Dimension(70, 7));
		mFileFunctionTabs.setBorder(UIManager.getBorder("FormattedTextField.border"));
		mFileFunctionTabs.setAlignmentX(Component.LEFT_ALIGNMENT);
		mCreateTSleft.setLeftComponent(mFileFunctionTabs);
		mCreateTSleft.setRightComponent(mCreateTSright);
		
		
		/************************* FileTree ************************/
		scrollablefiletree = new JScrollPane();
		scrollablefiletree.setBorder(null);
		String scriptdir = Configuration.getInstance().getValue(Constants.scriptDirectory);
		mFileTree = new FileTree(scriptdir,FileTree.createFileTree);
		scrollablefiletree.setViewportView(mFileTree);
		mFileFunctionTabs.addTab("Scripts", null, scrollablefiletree, null);
		/***********************************************************/

	
		mFunctionsPanel = new JPanel();
		//mFileFunctionTabs.addTab("Functions", null, mFunctionsPanel, null);
		mFunctionsPanel.setLayout(new MigLayout("", "[grow]", "[][grow]"));
		
		JLabel lblSearch = new JLabel("Commands");
		mFunctionsPanel.add(lblSearch, "flowx,cell 0 0");
		
		
		/************** FunctionList integration *******************/
		// create and configure CodeCompleterList instance
		mCcl = new CodeCompleterList();
	    mCcl.setMaximumSize(new Dimension(10000,8000));
		mCcl.setMinimumSize(new Dimension(40,20));
		mCcl.setListContent(CodeCompleter.getInstance().getAllCommands());
		
		JScrollPane functionList = new JScrollPane(mCcl);
		mFunctionsPanel.add(functionList, "cell 0 1,grow");
		/*****************************************************/
		
		searchField = new JTextField();
		searchField.getDocument().addDocumentListener(new DocumentListener(){
			
			public void changedUpdate(DocumentEvent e) {
				WindowConnector.getInstance().setSearchTokens(searchField.getText());
			}

			public void insertUpdate(DocumentEvent e) {
				WindowConnector.getInstance().setSearchTokens(searchField.getText());
			}
			
			public void removeUpdate(DocumentEvent e) {
				WindowConnector.getInstance().setSearchTokens(searchField.getText());	
			}
		});
		mFunctionsPanel.add(searchField, "cell 0 0");
		searchField.setColumns(20);
		
		JButton btnClear = new JButton("");
		btnClear.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				searchField.setText("");
				WindowConnector.getInstance().setSearchTokens(searchField.getText());
			}
		});
		btnClear.setMaximumSize(new Dimension(30, 23));
		btnClear.setIcon(new ImageIcon("resources/icons/clear.png"));
		mFunctionsPanel.add(btnClear, "cell 0 0");
		
		//
		JSplitPane center_panel = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
		center_panel.setContinuousLayout(true);
		center_panel.setBorder(UIManager.getBorder("FormattedTextField.border"));
		center_panel.setResizeWeight(0.65);
		center_panel.setDividerSize(3);
		mCreateTSright.setLeftComponent(center_panel);
		
		//
		/************** Editor integration *******************/
		mFilesTabPane = new JTabbedPane(JTabbedPane.TOP);
		mFilesTabPane.setMinimumSize(new Dimension(200, 22));
		mFilesTabPane.setBorder(UIManager.getBorder("FormattedTextField.border"));
		center_panel.setTopComponent(mFilesTabPane);
		mFilesTabPane.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
		
		mFilesTabPane.addChangeListener(new ChangeListener() {

			@Override
			public void stateChanged(ChangeEvent arg0) {
				// TODO Auto-generated method stub
				MainApp.getInstance().tabFocusChange(
						(TabTextEditor) mFilesTabPane.getSelectedComponent(),
						mFilesTabPane.getSelectedIndex());

			}
		});

		mTabPopMenu = new JPopupMenu();

		final JMenuItem close = new JMenuItem("Close");
		close.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().closeFile(mFilesTabPane,mFilesTabPane.getSelectedIndex());
			}
		});
		
		
		final JMenuItem closeOthers = new JMenuItem("Close Others...");
		closeOthers.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().closeOthers(mFilesTabPane,mFilesTabPane.getSelectedIndex());
			}
		});
		
		final JMenuItem closeAll = new JMenuItem("Close All");
		closeAll.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				WindowConnector.getInstance().closeAll(mFilesTabPane);
			}
		});
		
		mTabPopMenu.add(close);
		mTabPopMenu.add(closeAll);
		mTabPopMenu.add(closeOthers);
		
		mFilesTabPane.addMouseListener(new MouseAdapter(){

			@Override
			public void mouseClicked(MouseEvent e) {
				// TODO Auto-generated method stub
				int tabIndex = mFilesTabPane.indexAtLocation(e.getX(), e.getY());
				if(tabIndex != -1) {
				    // right click, popup menu
					if((e.getModifiers() & InputEvent.BUTTON3_MASK) != 0) {
						mTabPopMenu.show(e.getComponent(),e.getX(),e.getY());
					}
					// double click, close tab page
					else if(e.getClickCount() == 2) {
						// only one New file, can't close it
						WindowConnector.getInstance().closeFile(mFilesTabPane, tabIndex);
					}
				}
			}//end mouseClicked
			
		});
		
		
		//mFilesTabPane.addTab("Create Test Suite", new ImageIcon("resources/icons/editor.png"), EditorConnector.getInstance().createEditor(), null);
		/*****************************************************/
		
		
		/************** Outline integration *******************/
		scrollableoutlinetree = new JScrollPane();
		scrollableoutlinetree.setBorder(UIManager.getBorder("FormattedTextField.border"));
		mOutlineTree = new ScriptOutlineTree(null);
		//mOutlineTree.setMinimumSize(new Dimension(70, 0));
		mOutlineTree.setBorder(null);
		scrollableoutlinetree.setViewportView(mOutlineTree);
		
		//
		JSplitPane outlinePane = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
		outlinePane.setContinuousLayout(true);
		outlinePane.setBorder(UIManager.getBorder("FormattedTextField.border"));
		outlinePane.setResizeWeight(0.30);
		outlinePane.setDividerSize(3);
		//
		outlinePane.setTopComponent(scrollableoutlinetree );
		outlinePane.setBottomComponent( mFunctionsPanel);
		//
		mCreateTSright.setRightComponent(outlinePane); 
		/*****************************************************/
		
		// console window
		JTabbedPane infoTabPane = new JTabbedPane(JTabbedPane.TOP);
		mRunTS = new RunConsolePanel();
		infoTabPane.addTab("Console", new ImageIcon("resources/icons/terminal.png"), mRunTS, null);
		center_panel.setBottomComponent(infoTabPane);
		
		//bottom bar
		mBottomBar = new JToolBar();
		contentPane.add(mBottomBar, BorderLayout.SOUTH);
		mBottomBar.setEnabled(false);
		
		JLabel labelLeft = new JLabel(" ");
		mBottomBar.add(labelLeft);
        
		// do other configures
		selectFilesTab();
		
		// set bottom bar and the whole frame display mode
		if (Configuration.getInstance().getValue("MainWindowWidth") == null
				|| Configuration.getInstance().getValue("MainWindowHeight") == null
				|| "0".equals(Configuration.getInstance().getValue("MainWindowWidth"))
				|| "0".equals(Configuration.getInstance().getValue("MainWindowHeight"))
			)
		{
			Dimension screenSize =Toolkit.getDefaultToolkit().getScreenSize();
			Insets insets = Toolkit.getDefaultToolkit().getScreenInsets(getGraphicsConfiguration());
            int frameWidth = screenSize.width-2;
            int frameHeight = screenSize.height-insets.bottom;
			
			Configuration.getInstance().setValue("MainWindowWidth", Integer.toString(frameWidth));
			Configuration.getInstance().setValue("MainWindowHeight", Integer.toString(frameHeight));
		}
		
		showEditorButtons(true);
		pack();
		
		initUserSettings();
	}
	
	public ScriptOutlineTree getOutlineTree() {
		return mOutlineTree;
	}
	
	public JSplitPane getCreateTSLeftPanel(){
		return mCreateTSleft;
	}
	public JSplitPane getCreateTSRightPanel(){
		return mCreateTSright;
	}
	
	public RunConsolePanel getRTSPanel(){
		return mRunTS;
	}
	
	public FileTree getFileTree()  {
		return mFileTree;
	}
	
	public JTabbedPane getFileFunctionTabs() {
		return mFileFunctionTabs;
	}
	
	public JScrollPane getCreateFileTree(){
		return scrollablefiletree;
	}

	public void selectFilesTab() {
		mFileFunctionTabs.setSelectedComponent(scrollablefiletree);
	}
	
	public void selectFunctionsTab() {
		mFileFunctionTabs.setSelectedComponent(mFunctionsPanel);
	}
	
	public JButton getGenerateReportBtn(){
		return btnGenerateReport;
	}
	
	public JTabbedPane getEditorTabbedPane() {
		return mFilesTabPane;
	}
	
	public JButton getRunBtn() {
		return btnRun;
	}
	
	public JButton getStopBtn() {
		return btnStop;
	}
	
	/**
	 * Shows the correct buttons in the toolbar
	 * @param editor True if the button for the editor, false if those for Run Test Suite
	 */
	public void showEditorButtons(boolean editor){
		
		btnNewScript.setVisible(editor);
		btnOpenFile.setVisible(editor);
		btnSave.setVisible(editor);
		btnSaveAs.setVisible(editor);
		btnConfiguration.setVisible(editor);
		mToolBar.repaint();
	}
	
	/**
	 * Tries to initialise the frame with the old configurations saved in the configurations file
	 * otherwise if nothing is set use standard configurations
	 */
	public void initUserSettings(){
		String[] usrConf = WindowConnector.getInstance().getWindowSettings();
		double windowWidth, windowHeight;
		int createTSTabDivLoc, createTSEditorDivLoc;
		
		if(usrConf[0] != null && usrConf[1] != null){
			windowWidth = Double.valueOf(usrConf[0]);
			windowHeight = Double.valueOf(usrConf[1]);
			setSize(new Dimension((int)windowWidth,(int)windowHeight));
		}
//		if(usrConf[2] != null){
//			mainTab = Boolean.valueOf(usrConf[2]);
//			if(!mainTab){
//				selectrunTSTab();
//			}
//		}
		if(usrConf[3] != null){
			boolean cTSTab = Boolean.valueOf(usrConf[3]);
			if(!cTSTab){
				//selectFunctionsTab();
			}
		}
		if(usrConf[4] != null){
			createTSTabDivLoc = Integer.valueOf(usrConf[4]);
			mCreateTSleft.setDividerLocation(createTSTabDivLoc);
		}
		if(usrConf[5] != null){
			createTSEditorDivLoc = Integer.valueOf(usrConf[5]);
			mCreateTSright.setDividerLocation(createTSEditorDivLoc);
		}

	}
	
	public CodeCompleterList getCodeCompleteList() {
		return mCcl;
	}
	
	public void addOpenedFileTab(String tabName,TabTextEditor editor) {
		mFilesTabPane.addTab(null,editor);
		TabTitlePanel tp = new TabTitlePanel(tabName,mFilesTabPane);
		mFilesTabPane.setTabComponentAt(mFilesTabPane.indexOfComponent(editor), tp);
		mFilesTabPane.setSelectedIndex(mFilesTabPane.getTabCount()-1);
	}
	
	public void setSaveBtnState(boolean bAvtive) {
		btnSave.setEnabled(bAvtive);
	}
	
	public void setActiveTabIndex(int index) {
		mFilesTabPane.setSelectedIndex(index);
	}
	

	
}
