/**
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
*/

package transmemoptimizer;

import javax.swing.JFrame;

import javax.swing.JPanel;
import java.awt.GridBagLayout;

import javax.swing.AbstractCellEditor;
import javax.swing.BorderFactory;
import javax.swing.DefaultCellEditor;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.JViewport;
import javax.swing.ListModel;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JList;
import javax.swing.JTextField;
import javax.swing.UIManager;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.GridBagConstraints;
import java.awt.Point;
import java.awt.Rectangle;

import javax.swing.JLabel;
import javax.swing.border.Border;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;

import java.awt.FlowLayout;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.EventObject;
import java.util.Map;
import java.util.Vector;




public class ApplyChangesDialog extends javax.swing.JDialog implements ActionListener
{
    private ChangeOptions options;
	private JPanel outerPanel;
	private JPanel jButtonPanel;
	private JButton cancelButton;
	private JLabel memoryLabel;
	private JButton loadChangeListButton;
	private JButton loadExternalChangeListButton;
	private JButton clearChangeListButton;
	private JList externalChangeListsList;
	private JLabel externalChangeListLabel;
	private JList changesList;
	private JLabel changeslabel;
	private JButton saveToBrowseButton;
	private JTextField saveToTextField;
	private JLabel saveToLabel;
	private JButton memoryBrowseButton;
	private JTextField memoryTextField;
	private JButton changeButton;
	private JButton saveChangeListButton;
	private JButton addToExternalChangeListButton;
	private JButton removeFromExternalChangeListButton;
	private JButton removeFromChangeListButton;
	private JButton addToChangeListButton;
	private JButton saveExternalChangeListButton;
	private JPanel changesPanel;
	
	private JPanel controlRecordPanel;
	private JCheckBox changeMarkupTableCheckBox;
	private JRadioButton useProjectMarkupTableRadioButton;
	private JLabel projectMarkupLabel;
	private JComboBox projectMarkupComboBox;
	private JRadioButton useMarkupListRadioButton;
	private JButton loadMarkupListButton;
	private JButton clearMarkupListButton;
	private JButton saveMarkupListButton;
	private JButton addToMarkupListButton;
	private JButton removeFromMarkupListButton;
	
	private JPanel additionalChangesPanel;
	private JCheckBox removeLinkEndCheckBox;
	private JCheckBox ignoreTrademarksCheckBox;


	
	protected Vector externalChangeLists;
	protected ApplyChangesDialog dlg;
	protected JFrame parent;
	private MatchListMainApp app;
    private JFileChooser memoryFC;
    private JFileChooser changeListFC;
	private boolean dlgCancelled;
	private ChangeList externalChangeList;
	private ChangeList internalChangeList;
	private ChangeList markupChangeList;
    private ChangeListTableModel internalChangeListModel;
    private ChangeListTableModel externalChangeListModel;
    private TableSorter internalChangeListSorter;
    private TableSorter externalChangeListSorter;
    private JTable changesTable;
    private JTable externalChangesTable;
    private JTable markupChangesTable;
    private ChangeListTableModel markupChangeListModel;

    // fields for file name panel
	private JButton loadNameListButton;
	private JButton clearNameListButton;
	private JButton saveNameListButton;
	private JButton addToNameListButton;
	private JButton removeFromNameListButton;
    private JTable nameChangesTable;
    private ChangeListTableModel nameChangeListModel;
	private ChangeList nameChangeList;
	private JPanel nameChangePanel;
	private JRadioButton changePathRadioButton;
	private JRadioButton changeNameRadioButton;
	private JRadioButton changeBothRadioButton;
	private JCheckBox changeNameCheckBox;
	
	// fields for options panel
	private JPanel optionsPanel;
	private JCheckBox hyphenHandlingCheckBox;
	private JCheckBox logChangesCheckBox;
	private JLabel logFileLabel;
	private JTextField logFileName;
	private JButton logFileBrowseButton;
	private JRadioButton allSegments;
	private JCheckBox includeOldSegs1;
	private JRadioButton changedSegments;
	private JCheckBox includeOldSegs2;
  private JCheckBox markAsMachineMatchCheckBox;
  
	public ApplyChangesDialog(JFrame frame, MatchListMainApp mainapp, ChangeList inChangeList, ChangeOptions inOptions ) 
	{
		super(frame);
		parent = frame;
		dlg = this;
	    app = mainapp;	
	    internalChangeList = inChangeList;
	    options = inOptions;
	    externalChangeList = new ChangeList();
	    if ( markupChangeList == null ) markupChangeList = new ChangeList();  
	    nameChangeList = new ChangeList();
	    
		initGUI();
	    Point pt = new Point( 100, 100 );
	    this.setLocation( pt );
	    memoryFC = new JFileChooser();
	    changeListFC = new JFileChooser();
	    dlgCancelled = true;

	}
	
	private JPanel createChangesPanel()
	{
	  changesPanel = new JPanel();
	  
	  GridBagLayout jPanel1Layout = new GridBagLayout();
	  jPanel1Layout.columnWeights = new double[] {0.2,0.6,0.2};
	  jPanel1Layout.columnWidths = new int[] {10,30,10};
	  jPanel1Layout.rowWeights = new double[] {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
	  jPanel1Layout.rowHeights = new int[] {7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
	  changesPanel.setLayout(jPanel1Layout);
	  
      {
        changeslabel = new JLabel();
        changesPanel.add(changeslabel, new GridBagConstraints(
            0, 1, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
            new Insets( 3, 5, 3, 5), 0, 0));
        changeslabel.setText("TM replacement list");
      }
      
	  {
		  internalChangeListModel = new ChangeListTableModel( internalChangeList );
		  internalChangeListSorter = new TableSorter( internalChangeListModel ); 
		  changesTable = new JTable(internalChangeListSorter);
		  internalChangeListSorter.setTableHeader(changesTable .getTableHeader());
		  
		  changesTable.setDefaultRenderer( Object.class, new MultiLineCellRenderer() );
		  changesTable.setDefaultEditor( Object.class, new MultiLineCellEditor() );
		  
		  // Add the stripe renderer.
		  StripedTableCellRenderer.installInTable( changesTable, new Color( 240, 240, 240 ),
		                                           Color.black, Color.white, Color.black );	
		    
		  TableColumnModel tcm = changesTable.getColumnModel ();
		  
		  JScrollPane changesScrollPane = new JScrollPane( changesTable );
		  Dimension size = new Dimension( 100, 200 );
		  changesScrollPane.setPreferredSize( size );
		  
		  changesPanel.add(changesScrollPane, new GridBagConstraints(
		      0, 2, 2, 15, 0.7, 0.7, GridBagConstraints.CENTER, GridBagConstraints.BOTH,
		      new Insets( 3, 5, 3, 5), 0, 0));
		  changesScrollPane.setToolTipText( "Changes in TM replacement list" );
	  }
	  
	  loadChangeListButton = 
	    addButton( changesPanel, 2, 2, "Load", "Load TM replacement list from external file" );
	  saveChangeListButton = 
	    addButton( changesPanel, 2, 4, "Save", "Save TM replacement list to external file" );
	  addToChangeListButton = 
	    addButton( changesPanel, 2, 6, "Add entry", "Add a new entry to the TM replacement list" );
	  removeFromChangeListButton = 
	    addButton( changesPanel, 2, 8, "Delete", "Delete selected entry from the TM replacement list" );
	  {
	    externalChangeListLabel = new JLabel();
	    changesPanel.add(externalChangeListLabel, new GridBagConstraints(
	        0, 19, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	        new Insets( 3, 5, 3, 5), 0, 0));
	    externalChangeListLabel.setText("User replacement list");
	  }
	  {
	    externalChangeListModel = new ChangeListTableModel( externalChangeList );
	    externalChangeListSorter = new TableSorter( externalChangeListModel ); 
	    externalChangesTable = new JTable(externalChangeListSorter);
	    externalChangeListSorter.setTableHeader(externalChangesTable.getTableHeader());
	    
	    TableColumnModel tcm = externalChangesTable.getColumnModel ();
	    int ncols = tcm.getColumnCount();
	    
	    externalChangesTable.setDefaultRenderer( Object.class, new MultiLineCellRenderer() );
	    externalChangesTable.setDefaultEditor( Object.class, new MultiLineCellEditor() );
		  
	    // Add the stripe renderer.
	    StripedTableCellRenderer.installInTable( externalChangesTable, new Color( 240, 240, 240 ),
	        Color.black, Color.white, Color.black );	
	    
	    JScrollPane externalChangesScrollPane = new JScrollPane( externalChangesTable);
	    Dimension size = new Dimension( 100, 100 );
	    externalChangesScrollPane.setPreferredSize( size );
	    changesPanel.add(externalChangesScrollPane, new GridBagConstraints(
	        0, 20, 2, 15, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.BOTH,
	        new Insets( 3, 5, 3, 5), 0, 0));
	    externalChangesScrollPane.setToolTipText( "Changes from external change lists" );
	  }
	  loadExternalChangeListButton = 
	    addButton( changesPanel, 2, 20, "Load", "Add user replacement list from external file" );
	  saveExternalChangeListButton = 
	    addButton( changesPanel, 2, 21, "Save", "Save user replacement list to external file" );
	  clearChangeListButton = 
	    addButton( changesPanel, 2, 22, "Clear", "Clear user replacement list" );
	  addToExternalChangeListButton = 
	    addButton( changesPanel, 2, 23, "Add entry", "Add a new entry to the user replacement list" );
	  removeFromExternalChangeListButton = 
	    addButton( changesPanel, 2, 24, "Delete", "Delete selected entry from the user replacement list" );
	  
	  return( changesPanel );
	}

	private JPanel createButtonPanel()
	{
	  JPanel jButtonPanel = new JPanel();
	  FlowLayout jButtonPanelLayout = new FlowLayout();
	  jButtonPanel.setLayout(jButtonPanelLayout);
	  {
	    changeButton = new JButton();
	    jButtonPanel.add(changeButton);
	    changeButton.setText("Apply changes");
	    changeButton.setToolTipText("Apply the changes to the input Translation Memory");
	    changeButton.addActionListener( this );
	  }
	  {
	    cancelButton = new JButton();
	    jButtonPanel.add(cancelButton);
	    cancelButton.setText("Cancel");
	    cancelButton.setToolTipText( "Cancel this window" );
	    cancelButton.addActionListener( this );
	  }
	  return( jButtonPanel );
	}

	private JPanel createMemoryPanel()
	{
      JPanel memoryPanel = new JPanel();
	  GridBagLayout jPanel1Layout = new GridBagLayout();
	  jPanel1Layout.columnWeights = new double[] {0.2,0.6,0.2};
	  jPanel1Layout.columnWidths = new int[] {10,30,10};
	  jPanel1Layout.rowWeights = new double[] {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
	  jPanel1Layout.rowHeights = new int[]    {  7,   7,   7,   7,   7,   7,   7,   7,   7};
	  memoryPanel.setLayout(jPanel1Layout);
      
      {
        memoryLabel = new JLabel();
        memoryPanel.add(memoryLabel, new GridBagConstraints(
            0, 2, 1, 1, 0.1, 0.1, GridBagConstraints.WEST, GridBagConstraints.NONE,
            new Insets( 3, 5, 3, 5), 0, 0));
        memoryLabel.setText("External Translation Memory:");
      }
      {
        memoryTextField = new JTextField();
        memoryPanel.add(memoryTextField, new GridBagConstraints(
            1, 2, 1, 1, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets( 3, 5, 3, 5), 0, 0));
        memoryTextField.setText( app.getProperty( "InputMemory", "" ) );
        memoryTextField.setToolTipText( "Name of input Translation Memory");
      }
      {
        memoryBrowseButton = new JButton();
        memoryPanel.add(memoryBrowseButton, new GridBagConstraints(
            2, 2, 1, 1, 0.2, 0.2, GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets( 3, 5, 3, 5), 0, 0));
        memoryBrowseButton.setText("Browse...");
        memoryBrowseButton.setToolTipText( "Select an input Translation Memory" );
        memoryBrowseButton.addActionListener( this );
      }
      {
        saveToLabel = new JLabel();
        memoryPanel.add(saveToLabel, new GridBagConstraints(
            0, 4, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
            new Insets( 3, 5, 3, 5), 0, 0));
        saveToLabel.setText("Save to Translation Memory:");
      }
      {
        saveToTextField = new JTextField();
        memoryPanel.add(saveToTextField, new GridBagConstraints(
            1, 4, 1, 1, 0.0, 0.0,
            GridBagConstraints.CENTER,
            GridBagConstraints.HORIZONTAL,
            new Insets( 3, 5, 3, 5), 0, 0));
        saveToTextField.setText( app.getProperty( "OutputMemory", "" ) );
        saveToTextField.setToolTipText( "Name of the output Translatio Memory receiving the changes");
      }
      {
        saveToBrowseButton = new JButton();
        memoryPanel.add(saveToBrowseButton, new GridBagConstraints(
            2, 4, 1, 1, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets( 3, 5, 3, 5), 0, 0));
        saveToBrowseButton.setText("Browse...");
        saveToBrowseButton.setToolTipText( "Select the output Translation Memory");
        saveToBrowseButton.addActionListener( this );
      }
      return( memoryPanel );
	}
	
	private JPanel createControlRecordPanel()
	{
	  JPanel controlRecordPanel = new JPanel();
	  
	  GridBagLayout gridBagLayout = new GridBagLayout();
	  gridBagLayout.columnWeights = new double[] {0.2,0.6,0.2};
	  gridBagLayout.columnWidths = new int[] {10,30,10};
	  gridBagLayout.rowWeights = new double[] {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
	  gridBagLayout.rowHeights = new int[] {7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
	  controlRecordPanel.setLayout(gridBagLayout);
	  
	  changeMarkupTableCheckBox = new JCheckBox( "Change markup table in Translation Memory" );
	  changeMarkupTableCheckBox.addActionListener( this );
	  controlRecordPanel.add( changeMarkupTableCheckBox, new GridBagConstraints(
	      0, 1, 2, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 3, 5, 3, 5), 0, 0));
	  
	  useProjectMarkupTableRadioButton = new JRadioButton( "Use new project markup table for all segments in Translation Memory" );
	  useProjectMarkupTableRadioButton.addActionListener( this );
	  controlRecordPanel.add( useProjectMarkupTableRadioButton, new GridBagConstraints(
	      0, 2, 2, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 10, 5, 3, 5), 0, 0));
	  
	  projectMarkupLabel = new JLabel( "Project markup:" );
	  controlRecordPanel.add( projectMarkupLabel, new GridBagConstraints(
	      0, 3, 1, 1, 0.0, 0.0, GridBagConstraints.EAST, GridBagConstraints.NONE,
	      new Insets( 10, 5, 3, 5), 0, 0));
	  
	  projectMarkupComboBox = new JComboBox();
	  String selectedMarkup = "";
	  if ( markupChangeList.getLength() != 0 )
	  {
	    ChangeEntry entry = markupChangeList.getAt( 0 );
	    selectedMarkup = entry.getChangeTo();
	  }
	  fillWithMarkups( projectMarkupComboBox, selectedMarkup  );
	  projectMarkupComboBox.setPreferredSize( new Dimension( 120, 20 ) ); 
	  controlRecordPanel.add( projectMarkupComboBox, new GridBagConstraints(
	      1, 3, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 3, 5, 3, 5), 0, 0));
	  
	  useMarkupListRadioButton = new JRadioButton( "Use markup table replacement list");
	  useMarkupListRadioButton.addActionListener( this );
	  controlRecordPanel.add( useMarkupListRadioButton, new GridBagConstraints(
	      0, 6, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 10, 5, 3, 5), 0, 0));
      
	  {
		  markupChangeListModel = new ChangeListTableModel( markupChangeList );
		  TableSorter sorter = new TableSorter( markupChangeListModel ); 
		  markupChangesTable = new JTable(sorter);
		  sorter.setTableHeader(markupChangesTable .getTableHeader());
		  
		  MyTableCellRenderer tr = new MyTableCellRenderer ();
		  TableColumnModel tcm = markupChangesTable.getColumnModel ();
		  int ncols = tcm.getColumnCount();
		  for (int c = 0; c < ncols; c++)
		  {
		    TableColumn tc = tcm.getColumn (c);
		    tc.setCellRenderer (tr);
		  }	        
		  JTextField field = new JTextField();
		  markupChangesTable.setDefaultEditor( Object.class, new DefaultCellEditor( field ) );
		  
		  JScrollPane scrollPane = new JScrollPane( markupChangesTable );
		  Dimension size = new Dimension( 100, 200 );
		  scrollPane.setPreferredSize( size );
		  
		  controlRecordPanel.add(scrollPane, new GridBagConstraints(
		      0, 8, 2, 15, 0.7, 0.7, GridBagConstraints.CENTER, GridBagConstraints.BOTH,
		      new Insets( 10, 5, 3, 5), 0, 0));
		  scrollPane.setToolTipText( "List of markup table changes" );
	  }
	  
	  loadMarkupListButton = 
	    addButton( controlRecordPanel, 2, 8, "Load", "Load markup table replacement list from external file" );
	  saveMarkupListButton = 
	    addButton( controlRecordPanel, 2, 10, "Save", "Save markup table replacement list to external file" );
	  addToMarkupListButton = 
	    addButton( controlRecordPanel, 2, 12, "Add entry", "Add a new entry to the markup table replacement list" );
	  removeFromMarkupListButton = 
	    addButton( controlRecordPanel, 2, 14, "Delete", "Delete selected entry from the markup table list" );
	  clearMarkupListButton = 
	    addButton( controlRecordPanel, 2, 16, "Clear", "Clear markup table replacement list" );
	
	  // set initial state of controls
	  changeMarkupTableCheckBox.setSelected( true ); 
	  useProjectMarkupTableRadioButton.setSelected( false );
	  setProjectMarkupEnabled( false );
	  useMarkupListRadioButton.setSelected( true );
	  setMarkupListEnabled( true );
	  
	  return( controlRecordPanel );
	}
	
	private JPanel createNameChangePanel()
	{
	  JPanel nameChangePanel = new JPanel();
	  
	  GridBagLayout gridBagLayout = new GridBagLayout();
	  gridBagLayout.columnWeights = new double[] {0.2,0.6,0.2};
	  gridBagLayout.columnWidths = new int[] {10,30,10};
	  gridBagLayout.rowWeights = new double[] {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
	  gridBagLayout.rowHeights = new int[] {7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
	  nameChangePanel.setLayout(gridBagLayout);

	  changeNameCheckBox = new JCheckBox( "Change file names in Translation Memory" );
	  changeNameCheckBox.addActionListener( this );
	  nameChangePanel.add( changeNameCheckBox, new GridBagConstraints(
	      0, 1, 2, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 3, 5, 3, 5), 0, 0));
	  
	  
//	  useNameListRadioButton = new JRadioButton( "Use markup table replacement list");
//	  useMarkupListRadioButton.addActionListener( this );
//	  nameChangePanel.add( useMarkupListRadioButton, new GridBagConstraints(
//	      0, 6, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
//	      new Insets( 10, 5, 3, 5), 0, 0));
      
	  {
		  nameChangeListModel = new ChangeListTableModel( nameChangeList );
		  TableSorter sorter = new TableSorter( nameChangeListModel ); 
		  nameChangesTable = new JTable(sorter);
		  sorter.setTableHeader(nameChangesTable .getTableHeader());
		  
		  MyTableCellRenderer tr = new MyTableCellRenderer ();
		  TableColumnModel tcm = nameChangesTable.getColumnModel ();
		  int ncols = tcm.getColumnCount();
		  for (int c = 0; c < ncols; c++)
		  {
		    TableColumn tc = tcm.getColumn (c);
		    tc.setCellRenderer (tr);
		  }	        
		  JTextField field = new JTextField();
		  nameChangesTable.setDefaultEditor( Object.class, new DefaultCellEditor( field ) );
		  
		  JScrollPane scrollPane = new JScrollPane( nameChangesTable );
		  Dimension size = new Dimension( 100, 200 );
		  scrollPane.setPreferredSize( size );
		  
		  nameChangePanel.add(scrollPane, new GridBagConstraints(
		      0, 2, 2, 15, 0.7, 0.7, GridBagConstraints.CENTER, GridBagConstraints.BOTH,
		      new Insets( 10, 5, 3, 5), 0, 0));
		  scrollPane.setToolTipText( "List of path and/or name changes" );
	  }
	  
	  loadNameListButton = 
	    addButton( nameChangePanel, 2, 8, "Load", "Load file name replacement list from external file" );
	  saveNameListButton = 
	    addButton( nameChangePanel, 2, 10, "Save", "Save file name replacement list to external file" );
	  addToNameListButton = 
	    addButton( nameChangePanel, 2, 12, "Add entry", "Add a new entry to the file name replacement list" );
	  removeFromNameListButton = 
	    addButton( nameChangePanel, 2, 14, "Delete", "Delete selected entry from the file name replacement list" );
	  clearNameListButton = 
	    addButton( nameChangePanel, 2, 16, "Clear", "Clear file name replacement list" );
	
      changeNameRadioButton = new JRadioButton( "Apply changes to document name only");
	  changeNameRadioButton.addActionListener( this );
 	  nameChangePanel.add( changeNameRadioButton, new GridBagConstraints(
	      0, 18, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 10, 5, 3, 5), 0, 0));

 	  changePathRadioButton = new JRadioButton( "Apply changes to relative path only");
 	  changePathRadioButton.addActionListener( this );
 	  nameChangePanel.add( changePathRadioButton, new GridBagConstraints(
	      0, 19, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 10, 5, 3, 5), 0, 0));
 	  
      changeBothRadioButton = new JRadioButton( "Apply changes to path and document name");
	  changeBothRadioButton.addActionListener( this );
 	  nameChangePanel.add( changeBothRadioButton, new GridBagConstraints(
	      0, 20, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 10, 5, 3, 5), 0, 0));
 	  
	  // set initial state of controls    
	  changeNameCheckBox.setSelected( app.getIntProperty( "changeDocumentNames", 0 ) == 1 );
	  String changeMode = app.getProperty( "changeDocumentNameMode", "name" );
	  if ( changeMode.equalsIgnoreCase( "both") )
	  {
		  changeNameRadioButton.setSelected( false );
		  changePathRadioButton.setSelected( false );
		  changeBothRadioButton.setSelected( true );
	  }
	  else if ( changeMode.equalsIgnoreCase( "path") )
	  {
		  changeNameRadioButton.setSelected( false );
		  changePathRadioButton.setSelected( true );
		  changeBothRadioButton.setSelected( false );
	  }
	  else
	  {
		  changeNameRadioButton.setSelected( true );
		  changePathRadioButton.setSelected( false );
		  changeBothRadioButton.setSelected( false );
	  }
	  
	  if ( !changeNameCheckBox.isSelected() )
	  {
	    nameChangesTable.setEnabled( false );
		loadNameListButton.setEnabled( false ); 
		saveNameListButton.setEnabled( false );
		addToNameListButton.setEnabled( false );
		removeFromNameListButton.setEnabled( false ); 
		clearNameListButton.setEnabled( false ); 
		changePathRadioButton.setEnabled( false );
		changeNameRadioButton.setEnabled( false );
		changeBothRadioButton.setEnabled( false );
	  }
	  return( nameChangePanel );
	}
	
	private JPanel createAdditionalChangesPanel()
	{
	  JPanel additionalChangesPanel = new JPanel();
	  
	  GridBagLayout gridBagLayout = new GridBagLayout();
	  gridBagLayout.columnWeights = new double[] {0.2,0.6,0.2};
	  gridBagLayout.columnWidths = new int[] {10,30,10};
	  gridBagLayout.rowWeights = new double[] {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
	  gridBagLayout.rowHeights = new int[] {7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
	  additionalChangesPanel.setLayout(gridBagLayout);
	  
	  removeLinkEndCheckBox = new JCheckBox( "Remove all \"<l linkend=...>\" and \"</l>\" tags" );
	  removeLinkEndCheckBox.addActionListener( this );
	  additionalChangesPanel.add( removeLinkEndCheckBox, new GridBagConstraints(
	      0, 1, 2, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 3, 5, 3, 5), 0, 0));
	
	  // set initial state of controls
      String removeLinkEnd = app.getProperty( "RemoveLinkEnd" );
      if ( (removeLinkEnd != null) && removeLinkEnd.equalsIgnoreCase( "true" ) )
      {
        removeLinkEndCheckBox.setSelected( true ); 
      }


	  return( additionalChangesPanel );
	}
	
	private JPanel createOptionsPanel()
	{
	  JPanel optionsPanel = new JPanel();
	  
	  GridBagLayout gridBagLayout = new GridBagLayout();
	  gridBagLayout.columnWeights = new double[] {0.2,0.6,0.2};
	  gridBagLayout.columnWidths = new int[] {10,30,10};
	  gridBagLayout.rowWeights = new double[] {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
	  gridBagLayout.rowHeights = new int[] {7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
	  optionsPanel.setLayout(gridBagLayout);
	  
	  
	  hyphenHandlingCheckBox = new JCheckBox( "Replace even if hyphen is used in target segment" );
	  hyphenHandlingCheckBox.addActionListener( this );
	  optionsPanel.add( hyphenHandlingCheckBox, new GridBagConstraints(
	      0, 1, 2, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 3, 5, 3, 5), 0, 0));
	
	  logChangesCheckBox = new JCheckBox( "Log memory changes" );
	  logChangesCheckBox.addActionListener( this );
	  logChangesCheckBox.setToolTipText( "Activate logging of memory changes" );
	  optionsPanel.add( logChangesCheckBox, new GridBagConstraints(
	      0, 3, 2, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 3, 5, 3, 5), 0, 0));
	  
	  logFileLabel = new JLabel( "Log file name: " );
	  optionsPanel.add( logFileLabel, new GridBagConstraints(
	      0, 5, 1, 1, 0.0, 0.0, GridBagConstraints.EAST, GridBagConstraints.NONE,
	      new Insets( 3, 5, 3, 5), 0, 0));
	
	  logFileName = new JTextField( "" );
	  logFileName.setToolTipText( "Name of log file" );
	  optionsPanel.add( logFileName, new GridBagConstraints(
	      1, 5, 1, 1, 0.0, 0.0, 
          GridBagConstraints.CENTER,
          GridBagConstraints.HORIZONTAL,
	      new Insets( 3, 5, 3, 5), 0, 0));

	  logFileBrowseButton = new JButton();
      optionsPanel.add( logFileBrowseButton, new GridBagConstraints(
          2, 5, 1, 1, 0.2, 0.2, GridBagConstraints.CENTER, GridBagConstraints.NONE,
          new Insets( 3, 5, 3, 5), 0, 0));
      logFileBrowseButton.setText("Browse...");
      logFileBrowseButton.setToolTipText( "Select a log file" );
      logFileBrowseButton.addActionListener( this );
	  
      Border etched = BorderFactory.createEtchedBorder( );
      Border titled = BorderFactory.createTitledBorder( etched, "Memory output mode");
      JPanel outputOptionsPanel = new JPanel();
      
      GridBagLayout gbLayout = new GridBagLayout();
      
      outputOptionsPanel.setLayout( gbLayout );
      outputOptionsPanel.setBorder( titled );
      
      allSegments = new JRadioButton( "Write all segments to translation memory");
      allSegments.setToolTipText( "write changed and unchanged segments to output memory");
	  allSegments.addActionListener( this );
      includeOldSegs1 = new JCheckBox( "Keep old segments in the output memory");
      includeOldSegs1.setToolTipText( "write original and modified segment to output memory");
      changedSegments = new JRadioButton( "Write only changed segments to translation memory");
      changedSegments.setToolTipText( "write only modified segments to the output memory and ignore unchanged segments");
	  changedSegments.addActionListener( this );
      includeOldSegs2 = new JCheckBox( "Keep old segments in the output memory");
      includeOldSegs2.setToolTipText( "write original and modified segment to output memory");
      
      GridBagConstraints constraints1 = new GridBagConstraints( 0, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
          new Insets( 3, 5, 3, 5), 0, 0 );
      GridBagConstraints constraints2 = new GridBagConstraints( 0, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
          new Insets( 3, 20, 3, 5), 0, 0 );
          
      constraints1.gridx = 0;
      constraints1.gridy = 0;
      outputOptionsPanel.add( allSegments, constraints1 );
      
      constraints2.gridx = 0;
      constraints2.gridy = 1;
      outputOptionsPanel.add( includeOldSegs1, constraints2 );
      
      constraints1.gridx = 0;
      constraints1.gridy = 2;
      outputOptionsPanel.add( changedSegments, constraints1 );
      
      constraints2.gridx = 0;
      constraints2.gridy = 3;
      outputOptionsPanel.add( includeOldSegs2, constraints2 );
      
      optionsPanel.add( outputOptionsPanel, new GridBagConstraints(
          0, 6, 3, 3, 0.2, 0.2, GridBagConstraints.WEST, GridBagConstraints.BOTH,
          new Insets( 3, 5, 3, 5), 0, 0));
      
      markAsMachineMatchCheckBox = new JCheckBox( "Set M-flag of changed segments in Translation Memory");
      optionsPanel.add( markAsMachineMatchCheckBox, new GridBagConstraints(
          0, 10, 1, 1, 0.2, 0.2, GridBagConstraints.WEST, GridBagConstraints.NONE,
          new Insets( 3, 5, 3, 5), 0, 0));
      

	  ignoreTrademarksCheckBox = new JCheckBox( "No replace in trademarks (<tm....> ...</tm>)" );
	  ignoreTrademarksCheckBox.addActionListener( this );
	  additionalChangesPanel.add( ignoreTrademarksCheckBox, new GridBagConstraints(
	      0, 11, 1, 1, 0.2, 0.2, GridBagConstraints.WEST, GridBagConstraints.NONE,
	      new Insets( 3, 5, 3, 5), 0, 0));

	  
	  /*----------------------*/
      Border titled2 = BorderFactory.createTitledBorder( etched, "Restrict changes to specific markup tables");
      JPanel restrictMutPanel = new JPanel();
      
      GridBagLayout gbLayout2 = new GridBagLayout();
      
      restrictMutPanel.setLayout( gbLayout );
      restrictMutPanel.setBorder( titled2 );
      
      JRadioButton allMuts = new JRadioButton( "Apply changes to segments of all markup tables");
      JRadioButton selectedMuts = new JRadioButton( "Apply changes to segments with selected markup tables only");
      JList mutList = new JList();
      
      GridBagConstraints constraints21 = new GridBagConstraints( 0, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
          new Insets( 3, 5, 3, 5), 0, 0 );
      GridBagConstraints constraints22 = new GridBagConstraints( 0, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE,
          new Insets( 3, 20, 3, 5), 0, 0 );
          
      constraints21.gridx = 0;
      constraints21.gridy = 0;
      restrictMutPanel.add( allMuts, constraints21 );
      
      constraints21.gridx = 0;
      constraints21.gridy = 1;
      restrictMutPanel.add( selectedMuts, constraints21 );
      
      constraints22.gridx = 0;
      constraints22.gridy = 1;
      restrictMutPanel.add( mutList, constraints22 );
      
      optionsPanel.add( restrictMutPanel, new GridBagConstraints(
          0, 12, 6, 6, 0.2, 0.2, GridBagConstraints.WEST, GridBagConstraints.BOTH,
          new Insets( 3, 5, 3, 5), 0, 0));
/*----------------------*/
	  
	  
	  
	  
	  // set initial state of controls
      hyphenHandlingCheckBox.setSelected( app.getBoolProperty( "HyphenHandling" ) );
      logChangesCheckBox.setSelected( app.getBoolProperty( "Logging" ));
      if ( !logChangesCheckBox.isSelected() ) 
      {
        logFileName.setEnabled( false );
        logFileLabel.setEnabled( false );
        logFileBrowseButton.setEnabled( false );
      }
      
      String logName = app.getProperty( "LogFileName" );
      if ( logName != null )
      {
        logFileName.setText( logName );
      }
      else
      {
        logFileName.setText( "" );
      }

      allSegments.setSelected( app.getBoolProperty( "AllSegments" ) );
      includeOldSegs1.setSelected( app.getBoolProperty( "MergeOldAndNew1" ) );
      changedSegments.setSelected( app.getBoolProperty( "NewSegments" ) );
      includeOldSegs2.setSelected( app.getBoolProperty( "MergeOldAndNew2" ) );
      
      allSegments.setSelected( !changedSegments.isSelected() );
      includeOldSegs1.setEnabled( !changedSegments.isSelected() );
      includeOldSegs2.setEnabled( changedSegments.isSelected() );
      
      markAsMachineMatchCheckBox.setSelected( app.getBoolProperty( "MarkAsMachineMatch" ) );
      ignoreTrademarksCheckBox.setSelected( app.getBoolProperty( "IgnoreTrademarks" ) );
      
	  return( optionsPanel );
	}
	
	private void initGUI() 
	{
	  this.setSize(786, 650 );
	  //	    this.setResizable( false );
	  this.setTitle("Apply changes to external memory");
	  
	  outerPanel = new JPanel();
	  BorderLayout jMainPanelLayout = new BorderLayout();
	  outerPanel.setPreferredSize(new java.awt.Dimension(786, 650) );
	  outerPanel.setLayout(jMainPanelLayout);
	  this.getContentPane().add(outerPanel, BorderLayout.CENTER);
	  
	  JTabbedPane tabbedPane = new JTabbedPane();
	  
	  JPanel buttonPanel = createButtonPanel();
	  outerPanel.add( buttonPanel, BorderLayout.SOUTH );
	  
	  JPanel memoryPanel = createMemoryPanel();
	  outerPanel.add( memoryPanel, BorderLayout.CENTER);
	  
	  changesPanel = createChangesPanel();
	  additionalChangesPanel = createAdditionalChangesPanel();
	  nameChangePanel = createNameChangePanel();
	  optionsPanel = createOptionsPanel();
	  JPanel controlRecordPanel = createControlRecordPanel(); 
	  tabbedPane.addTab( "Segment data changes", changesPanel);
	  tabbedPane.addTab( "Additional changes", additionalChangesPanel );	        
	  tabbedPane.addTab( "Markup table changes", controlRecordPanel );	      
	  tabbedPane.addTab( "File name changes", nameChangePanel );
	  tabbedPane.addTab( "Options", optionsPanel );
	  outerPanel.add( tabbedPane, BorderLayout.NORTH);
	}
	

    private JButton addButton( JPanel panel, int row, int col, String text, String tooltip )
    {
      JButton button = new JButton();
      panel.add(button, new GridBagConstraints(
          row, col, 1, 1, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.NONE,
          new Insets( 3, 5, 3, 5), 0, 0));
      button.setText( text );
      button.setToolTipText( tooltip );
      button.addActionListener( this );
      Dimension dimension = new Dimension( 120, 24 );
      button.setPreferredSize( dimension );
      return( button );
    }
    
     	
	
	/**
	 * @return Returns the input memory.
	 */
	public String getInMemory()
	{
	  return memoryTextField.getText();
	}
	
	/**
	 * @param input memory The inMemory to set.
	 */
	public void setInMemory( String inMemory )
	{
	  memoryTextField.setText( inMemory );
	}
	/**
	 * @return Returns the outMemory.
	 */
	public String getOutMemory()
	{
	  return saveToTextField.getText();
	}
	/**
	 * @param outMemory The outMemory to set.
	 */
	public void setOutMemory( String outMemory )
	{
	  saveToTextField.setText( outMemory );
	}
	
	public boolean isDialogCancelled()
	{ 
	  return ( dlgCancelled );
	}
	
	public ChangeList getExternalChangeList()
	{
	  return externalChangeList;
	}
	
	public ChangeList getInternalChangeList()
	{
	  return internalChangeList;
	}

	public ChangeList getMarkupChangeList()
	{
	  return markupChangeList;
	}

	public void actionPerformed( ActionEvent event )
	{
	  if ( event.getSource() == cancelButton )
	  {
	    dlgCancelled  = true;
	    dispose();
	  }
	  else if ( event.getSource() == saveToBrowseButton )
	  {
	    File file = new File( saveToTextField.getText() );
	    memoryFC.setSelectedFile( file );
	    memoryFC.addChoosableFileFilter(new ExpFileFilter());
	    memoryFC.setFileSelectionMode(JFileChooser.FILES_ONLY);
	    memoryFC.setMultiSelectionEnabled( false );
	    memoryFC.setDialogTitle( "Select an output memory" ); 
	    int returnVal = memoryFC.showSaveDialog( app );
	    
	    if ( returnVal == JFileChooser.APPROVE_OPTION )
	    {
	      file = memoryFC.getSelectedFile();
	      File dir = memoryFC.getCurrentDirectory();
	      saveToTextField.setText( file.getAbsolutePath() );
	    }
	  }
	  else if ( event.getSource() == changeButton )
	  {
	    boolean fOk = true;
	    String inputMemory = memoryTextField.getText();
	    String outputMemory = saveToTextField.getText();
	    File inmem = new File( inputMemory );
	    File outmem = new File( outputMemory  );
	    
	    stopEditing( externalChangesTable );
	    stopEditing( changesTable );
	    
	    if ( inputMemory.length() == 0 )
	    {
	      Utils.showErrorMessage( 
	          "The name of input memory is missing\n" +
	          "Specify the name of an input memory." ); 
	      memoryTextField.requestFocus();
	      fOk = false;
	    }
	    else if ( outputMemory.length() == 0 )
	    {
	      fOk = false;
	      Utils.showErrorMessage( 
	          "The name of output memory is missing\n" +
	          "Specify the name of the output memory." ); 
	      memoryTextField.requestFocus();
	      fOk = false;
	    }
	    else if ( !inmem.exists() )
	    {
	      Utils.showErrorMessage( 
	          "The input memory " + inmem.getAbsoluteFile() + 
	          " does not exist\n" + 
	          "or cannot be accessed.\n" +
	          "Specify the name of an existing memory." );
	      memoryTextField.requestFocus();
	      fOk = false;
	    }
	    else if ( !Utils.isUTF16Encoded( inmem ) )
	    {
	      Utils.showErrorMessage( 
	          "The input memory " + inmem.getAbsoluteFile() + 
	          " is not in Unicode (UTF-16) format.\n" + 
	          "This tool only supports Unicode encoded " +
	          "external memory databases.\n" +
	          "Specify the name of an Unicode encoded memory." ); 
	      memoryTextField.requestFocus();
	      fOk = false;
	    }
	    else if ( inmem.compareTo( outmem ) == 0 )
	    {
	      Utils.showErrorMessage( 
	          "The input memory cannot be used as output memory.\n" +
	          "Please choose a different name for the output memory" ); 
	      saveToTextField.requestFocus();  
	      fOk = false;
	    }
	    else if ( outmem.exists() )
	    {
	      if ( !Utils.showConfirmationMessage(  
	          "The output memory " + outmem.getName() + " exists already.\n" +
	          "Do you want to overwrite the existing file?" ) )
	      {
	        saveToTextField.requestFocus();  
	        fOk = false;
	      }
	    }
	    
	    // update markup change list depending on selected options 
	    if ( fOk )
	    {
	      if ( changeMarkupTableCheckBox.isSelected() )
	      {
            if ( useProjectMarkupTableRadioButton.isSelected() )
            {
              // create dummy change entry with project markup
              markupChangeList.clear();
              String markup = (String)projectMarkupComboBox.getSelectedItem();
              markupChangeList.add( "***", markup  );
              
            }
            else
            {
              // use current markup change list  
            }
	      }
	      else
	      {
	        // clear markup table change list
            markupChangeList.clear();
	      }
	      options.markupChangeList = markupChangeList;
	    }

	    // get file name change options
	    if ( fOk )
	    {
		  options.changeNames = changeNameCheckBox.isSelected();
		  options.nameChangeList = this.nameChangeList;
		  app.setProperty( "changeDocumentNames", options.changeNames ? "1" : "0" );
		  
		  if ( changeNameRadioButton.isSelected() )
		  {
              options.changeNameMode = ChangeOptions.NAME_NAMEMODE;   
			  app.setProperty( "changeDocumentNameMode", "name" );
		  }
		  else if ( changePathRadioButton.isSelected() )
		  {
		    options.changeNameMode = ChangeOptions.PATH_NAMEMODE;   
		    app.setProperty( "changeDocumentNameMode", "path" );
		  }
		  else 
		  {
		    options.changeNameMode = ChangeOptions.BOTH_NAMEMODE;   
		    app.setProperty( "changeDocumentNameMode", "both" );
		  }
	    }
	    
	    // get other options
	    if ( fOk )
	    {
	      options.logChanges = logChangesCheckBox.isSelected();
	      options.logFileName = logFileName.getText();
	      options.hyphenHandling = hyphenHandlingCheckBox.isSelected();
        options.markAsMachineMatch = markAsMachineMatchCheckBox.isSelected();
	      if ( options.logChanges )
	      {
	        File logFile = new File( options.logFileName );
	        
	        if ( options.logFileName.equals( "" ) )
	        {
	          Utils.showErrorMessage( "The log file name is missing" );
	          logFileName.requestFocus();  
	          fOk = false;
	        }
	        else if ( logFile.exists() )
	        {
	          if ( !Utils.showConfirmationMessage(  
	              "The log file " + logFile.getName() + " exists already.\n" +
	          "Do you want to overwrite the existing file?" ) )
	          {
	            logFileName.requestFocus();  
	            fOk = false;
	          }
	        }
		  }
		  
		  if ( allSegments.isSelected() )
		  {
		    options.writeUnchangedSegments = true;
		    options.writeOldAndNewSeg = includeOldSegs1.isSelected();
		  }
		  else
		  {
		    options.writeUnchangedSegments = false;
		    options.writeOldAndNewSeg = includeOldSegs2.isSelected();
		  }
		  

	    }
	    
	    // save memory names and options in properties
	    if ( fOk )
	    {
	      // save input and output memory to properties
	      app.setProperty( "InputMemory", inputMemory );
	      app.setProperty( "OutputMemory", outputMemory );
	      app.setProperty( "RemoveLinkEnd", removeLinkEndCheckBox.isSelected() ? "true" : "false" );
	      app.setProperty( "HyphenHandling", options.hyphenHandling ? "true" : "false" );
	      app.setProperty( "Logging", options.logChanges ? "true" : "false" );
	      app.setProperty( "LogFileName", options.logFileName );

	      app.setProperty( "AllSegments", allSegments.isSelected() );
	      app.setProperty( "MergeOldAndNew1", includeOldSegs1.isSelected()  );
	      app.setProperty( "NewSegments", changedSegments.isSelected()  );
	      app.setProperty( "MergeOldAndNew2", includeOldSegs2.isSelected()  );
        app.setProperty( "MarkAsMachineMatch", markAsMachineMatchCheckBox.isSelected() );
        
	      // end dialog
	      dlg.dlgCancelled  = false;
	      dlg.dispose();
	      
	    }
	  }
	  else if ( event.getSource() == memoryBrowseButton )
	  {
	    File file = new File( memoryTextField.getText() );
	    memoryFC.setSelectedFile( file );
	    memoryFC.addChoosableFileFilter(new ExpFileFilter());
	    memoryFC.setFileSelectionMode(JFileChooser.FILES_ONLY);
	    memoryFC.setMultiSelectionEnabled( false );
	    memoryFC.setDialogTitle( "Select an input memory" ); 
	    int returnVal = memoryFC.showOpenDialog( app );
	    
	    if ( returnVal == JFileChooser.APPROVE_OPTION )
	    {
	      file = memoryFC.getSelectedFile();
	      File dir = memoryFC.getCurrentDirectory();
	      app.setProperty( "InputMemoryDirectory", dir.getPath() );
	      memoryTextField.setText( file.getAbsolutePath() );
	    }
	  }
	  else if ( event.getSource() == loadExternalChangeListButton )
	  {
	    stopEditing( externalChangesTable );
	    loadChangeList( externalChangeList, "user replacement list", false );
	  }
	  else if ( event.getSource() == loadChangeListButton )
	  {
	    stopEditing( changesTable );
	    loadChangeList( internalChangeList, "TM replacement list", true );
	  }
	  else if ( event.getSource() == clearChangeListButton )
	  {
	    stopEditing( externalChangesTable );
	    externalChangeList.clear();
	    externalChangeListModel.fireTableDataChanged();
	  }
	  else if ( event.getSource() == saveExternalChangeListButton )
	  {
	    stopEditing( externalChangesTable );
	    saveChangeList( externalChangeList, "user replacement list" );
	  }
	  else if ( event.getSource() == saveChangeListButton )
	  {
	    stopEditing( changesTable );
	    saveChangeList( internalChangeList, "TM replacement list" );
	  }
	  else if ( event.getSource() == addToChangeListButton )
	  {
	    stopEditing( changesTable );
	    ChangeEntry newEntry = new ChangeEntry();
	    ChangeEntryDialog dlg = new ChangeEntryDialog( this, newEntry );
        dlg.setModal( true );
        dlg.setVisible( true );	    
        if ( !dlg.isCancelled()  )
        {
          internalChangeList.add( newEntry );
  		  internalChangeListModel.fireTableDataChanged();
        }
	  }
	  else if ( event.getSource() == addToExternalChangeListButton )
	  {
	    stopEditing( externalChangesTable );
	    ChangeEntry newEntry = new ChangeEntry();
	    ChangeEntryDialog dlg = new ChangeEntryDialog( this, newEntry );
        dlg.setModal( true );
        dlg.setVisible( true );	    
        if ( !dlg.isCancelled()  )
        {
          externalChangeList.add( newEntry );
  		  externalChangeListModel.fireTableDataChanged();
        }
	  }
	  else if ( event.getSource() == removeFromChangeListButton )
	  {
	    stopEditing( changesTable );
	    int row = changesTable.getSelectedRow();
	    int rowCount = internalChangeListModel.getRowCount(); 
	    if ( (row >= 0) && (row < rowCount ) )
	    {
	      int modelIndex = internalChangeListSorter.modelIndex( row );
	      internalChangeListModel.removeAt( modelIndex );
  		  rowCount--;
  		  if ( row == rowCount )
  		  {
  		    row--;
  		  }
  		  if ( row >= 0 )
  		  {
  		    changesTable.setRowSelectionInterval( row, row );
  		  }
  		  // refresh following rows - does not work ....
    	  // internalChangeListModel.fireTableRowsUpdated( row, rowCount );
        }
	  }
	  else if ( event.getSource() == removeFromExternalChangeListButton )
	  {
	    stopEditing( externalChangesTable );
	    int row = externalChangesTable.getSelectedRow();
	    int rowCount = externalChangeListModel.getRowCount(); 
	    if ( (row >= 0) && (row < rowCount ) )
	    {
	      int modelIndex = externalChangeListSorter.modelIndex( row );
	      externalChangeListModel.removeAt( modelIndex  );
  		  rowCount--;
  		  if ( row == rowCount )
  		  {
  		    row--;
  		  }
  		  if ( row >= 0 )
  		  {
  		    externalChangesTable.setRowSelectionInterval( row, row );
  		  }
  		  // refresh following rows - does not work ....
  		  // externalChangeListModel.fireTableRowsUpdated( row, rowCount );
        }
	  }
	  else if ( event.getSource() == removeFromMarkupListButton )
	  {
	    int row = markupChangesTable.getSelectedRow();
	    if ( (row >= 0) && (row < markupChangeListModel.getRowCount() ) )
	    {
  		  markupChangeListModel.removeAt( row );
        }
	  }
	  else if ( event.getSource() == addToMarkupListButton )
	  {
	    ChangeEntry newEntry = new ChangeEntry();
	    ChangeEntryDialog dlg = new ChangeEntryDialog( this, newEntry );
        dlg.setModal( true );
        dlg.setVisible( true );	    
        if ( !dlg.isCancelled()  )
        {
          markupChangeList.add( newEntry );
  		  markupChangeListModel.fireTableDataChanged();
        }
	  }
	  else if ( event.getSource() == loadMarkupListButton )
	  {
	    loadChangeList( markupChangeList, "markup table replacement list", true );
	    markupChangeListModel.fireTableDataChanged();
	  }
	  else if ( event.getSource() == clearMarkupListButton )
	  {
	    markupChangeList.clear();
	    markupChangeListModel.fireTableDataChanged();
	  }
	  else if ( event.getSource() == saveMarkupListButton )
	  {
	    saveChangeList( markupChangeList, "markup table replacement list" );
	  }
	  else if ( event.getSource() == useProjectMarkupTableRadioButton )
	  {
	    setProjectMarkupEnabled();
	    useMarkupListRadioButton.setSelected( false );
	    setMarkupListEnabled( false );
	  }
	  else if ( event.getSource() == useMarkupListRadioButton )
	  {
	    setMarkupListEnabled();
	    useProjectMarkupTableRadioButton.setSelected( false );
	    setProjectMarkupEnabled( false );
	  }
	  else if ( event.getSource() == changeMarkupTableCheckBox )
	  {
	    boolean enable = changeMarkupTableCheckBox.isSelected();
	    
	    useProjectMarkupTableRadioButton.setEnabled( enable );
	    useMarkupListRadioButton.setEnabled( enable );
	    
	    setProjectMarkupEnabled();
	    setMarkupListEnabled();
	    
	  }
	  else if ( event.getSource() == changeNameCheckBox )
	  {
	    boolean enable = changeNameCheckBox.isSelected();
	    
	    clearNameListButton.setEnabled( enable );
	    loadNameListButton.setEnabled( enable );
	    removeFromNameListButton.setEnabled( enable );
	    saveNameListButton.setEnabled( enable );
	    addToNameListButton.setEnabled( enable );
	    nameChangesTable.setEnabled( enable );
		changePathRadioButton.setEnabled( enable );
		changeNameRadioButton.setEnabled( enable );
		changeBothRadioButton.setEnabled( enable );
	    
	  }
	  else if ( event.getSource() == loadNameListButton )
	  {
	    loadChangeList( nameChangeList, "file name replacement list", true );
	    nameChangeListModel.fireTableDataChanged();
	  }
	  else if ( event.getSource() == clearNameListButton )
	  {
	    nameChangeList.clear();
	    nameChangeListModel.fireTableDataChanged();
	  }
	  else if ( event.getSource() == saveNameListButton )
	  {
	    saveChangeList( nameChangeList, "file name replacement list" );
	  }
	  else if ( event.getSource() == addToNameListButton )
	  {
	    ChangeEntry newEntry = new ChangeEntry();
	    ChangeEntryDialog dlg = new ChangeEntryDialog( this, newEntry );
        dlg.setModal( true );
        dlg.setVisible( true );	    
        if ( !dlg.isCancelled()  )
        {
          nameChangeList.add( newEntry );
  		  nameChangeListModel.fireTableDataChanged();
        }
	  }
	  else if ( event.getSource() == removeFromNameListButton )
	  {
	    int row = nameChangesTable.getSelectedRow();
	    if ( (row >= 0) && (row < nameChangeListModel.getRowCount() ) )
	    {
  		  nameChangeListModel.removeAt( row );
        }
	  }
	  else if ( event.getSource() == changePathRadioButton )
	  {
	    changeNameRadioButton.setSelected( false );
	    changeBothRadioButton.setSelected( false );
	  }
	  else if ( event.getSource() == changeNameRadioButton )
	  {
	    changePathRadioButton.setSelected( false );
	    changeBothRadioButton.setSelected( false );
	  }
	  else if ( event.getSource() == changeBothRadioButton )
	  {
	    changeNameRadioButton.setSelected( false );
	    changePathRadioButton.setSelected( false );
	  }
	  else if ( event.getSource() == logFileBrowseButton )
	  {
	    JFileChooser fileChooser = new JFileChooser();

	    File file = new File( logFileName.getText() );
	    fileChooser.setSelectedFile( file );
//	    fileChooser.addChoosableFileFilter(new ExpFileFilter());
	    fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
	    fileChooser.setMultiSelectionEnabled( false );
	    fileChooser.setDialogTitle( "Select a log file" ); 
	    int returnVal = fileChooser.showSaveDialog( app );
	    
	    if ( returnVal == JFileChooser.APPROVE_OPTION )
	    {
	      file = fileChooser.getSelectedFile();
	      File dir = fileChooser.getCurrentDirectory();
	      logFileName.setText( file.getAbsolutePath() );
	    }
	  }
	  else if ( event.getSource() == logChangesCheckBox )
	  {
	    boolean enabled = logChangesCheckBox.isSelected();
	    
        logFileLabel.setEnabled( enabled );
	    logFileBrowseButton.setEnabled( enabled );
	    logFileName.setEnabled( enabled );
	  }
	  else if ( event.getSource() == allSegments ) 
	  {
	    if ( allSegments.isSelected() )
	    {
	      changedSegments.setSelected( false );
	      includeOldSegs1.setEnabled( true );
	      includeOldSegs2.setEnabled( false );
	    }
	    else if ( !changedSegments.isSelected() )
	    {
	      changedSegments.setSelected( true );
	      includeOldSegs1.setEnabled( false );
	      includeOldSegs2.setEnabled( true );
	    }
	  }
	  else if ( event.getSource() == changedSegments )
	  {
	    if ( changedSegments.isSelected() )
	    {
	      allSegments.setSelected( false );
	      includeOldSegs1.setEnabled( false );
	      includeOldSegs2.setEnabled( true );
	    }
	    else if ( !allSegments.isSelected() )
	    {
	      allSegments.setSelected( true );
	      includeOldSegs1.setEnabled( true );
	      includeOldSegs2.setEnabled( false );
	    }
	  }
	}

  //	 Assumes table is contained in a JScrollPane. Scrolls the
  // cell (rowIndex, vColIndex) so that it is visible within the viewport.
  private void scrollToVisible( JTable table, int rowIndex, int vColIndex )
  {
    if ( !(table.getParent() instanceof JViewport) )
    {
      return;
    }
    JViewport viewport = (JViewport) table.getParent();

    // This rectangle is relative to the table where the
    // northwest corner of cell (0,0) is always (0,0).
    Rectangle rect = table.getCellRect( rowIndex, vColIndex, true );

    // The location of the viewport relative to the table
    Point pt = viewport.getViewPosition();

    // Translate the cell location so that it is relative
    // to the view, assuming the northwest corner of the
    // view is (0,0)
    rect.setLocation( rect.x - pt.x, rect.y - pt.y );

    // Scroll the area into view
    viewport.scrollRectToVisible( rect );
  }
    
 	private void stopEditing ( JTable table )
	{
		if (table.getCellEditor() != null) 
	    {
		  table.getCellEditor().stopCellEditing();
	    }
	}

	private void setMarkupListEnabled( )
	{
	    boolean enable = useMarkupListRadioButton.isSelected() &&
	                     changeMarkupTableCheckBox.isSelected();
	    setMarkupListEnabled( enable );
	}
	private void setMarkupListEnabled( boolean enable )
	{
	    markupChangesTable.setEnabled( enable );
	    loadMarkupListButton.setEnabled( enable );
	    saveMarkupListButton.setEnabled( enable );
	    addToMarkupListButton.setEnabled( enable );
	    removeFromMarkupListButton.setEnabled( enable );
	    clearMarkupListButton.setEnabled( enable );
	}
	
	private void setProjectMarkupEnabled()
	{
	    boolean enable = useProjectMarkupTableRadioButton.isSelected();
	    setProjectMarkupEnabled( enable );
	}
	private void setProjectMarkupEnabled( boolean enable )
	{
	    projectMarkupLabel.setEnabled( enable );
	    projectMarkupComboBox.setEnabled( enable );
	}
	
	private void saveChangeList( ChangeList list, String listName )
	{
	  String lastDir = app.getProperty( "ChangeListDirectory" );
	  if ( lastDir != null )
	  {
	    File dir = new File( lastDir );
	    changeListFC.setCurrentDirectory( dir );
	  }
	  changeListFC.setFileSelectionMode(JFileChooser.FILES_ONLY);
	  changeListFC.setMultiSelectionEnabled( false );
	  changeListFC.setDialogTitle( "Save " + listName ); 
	  int returnVal = changeListFC.showSaveDialog( app );
	  
	  if ( returnVal == JFileChooser.APPROVE_OPTION )
	  {
	    boolean replaceFile = true;
	    File changeListFile = changeListFC.getSelectedFile();
	    app.setProperty( "ChangeListDirectory", changeListFile.getAbsolutePath() );
	    if ( changeListFile.exists() )
	    {
	      if ( !Utils.showConfirmationMessage( "Replacement list " + changeListFile.getName() + 
	                                       " exists already.\nDo you want to overwrite the list?" ) )
	      {
	        replaceFile = false;
	      } 
	    }
	    if ( replaceFile )
	    {
	      list.save( changeListFile );
	      Utils.showInfoMessage( "Replacement list has been saved to " + changeListFile.getName() );
	    }
	  }
	}
	
	private void loadChangeList( ChangeList list, String listName, boolean clearList  )
	{
	  String lastDir = app.getProperty( "ChangeListDirectory" );
	  if ( lastDir != null )
	  {
	    File dir = new File( lastDir );
	    changeListFC.setCurrentDirectory( dir );
	  }
	  changeListFC.setFileSelectionMode(JFileChooser.FILES_ONLY);
	  changeListFC.setMultiSelectionEnabled( false );
	  changeListFC.setDialogTitle( "Load " + listName ); 
	  int returnVal = changeListFC.showOpenDialog( app );
	  
	  if ( returnVal == JFileChooser.APPROVE_OPTION )
	  {
	    File changeList = changeListFC.getSelectedFile();
	    app.setProperty( "ChangeListDirectory", changeList.getAbsolutePath() );
	    if ( clearList )
	    {
	      list.clear();
	    }
	    list.add( changeList );
	    if ( list == externalChangeList )
	    {
	      externalChangeListModel.fireTableDataChanged();
	    }
	    else if ( list == internalChangeList )
	    {
	      internalChangeListModel.fireTableDataChanged();
	    }
	    else if ( list == markupChangeList )
	    {
	      markupChangeListModel.fireTableDataChanged();
	    }
	  }
	}
	
	private void fillWithMarkups( JComboBox comboBox, String selected  )
	{
	  ArrayList list = new ArrayList();
	  String root = app.getTMRootDir();
	  File tableDir = new File( root + "\\TABLE " );
	  File [] markups = tableDir.listFiles();
      int entries = markups.length;
      for ( int i = 0; i < entries; i++ )
      {
        String name = markups[i].getName();
        name = name.toUpperCase();
        if ( name.endsWith( ".TBL") )
        {
          int extensionPos = name.lastIndexOf( '.' );
          if ( extensionPos < 0 ) extensionPos = name.length();
          list.add( name.substring( 0, extensionPos ));
        }
      }
      Collections.sort(list);
      for( int i = 0; i < list.size(); i++ )
      {
        comboBox.addItem( list.get( i ) );
        if ( selected.equals( list.get( i ) ) )
        {
          comboBox.setSelectedIndex( i );
        }
      }

	}
	
  private int getHeight( JTextArea textArea, JTable table, String text, int row, int column )
  {
    FontMetrics fm = getFontMetrics( textArea.getFont() );
    int numberOfTextRows = 0;
    int numberOfTextRows_IncludingNewlines = 0;
    int textWidth = 0;
    int columnWidth = 0;
    if ( fm != null )
    {
      textWidth = fm.stringWidth( text );
    }
    if ( textWidth > 0 )
    {
      TableColumn tableColumn = table.getColumnModel().getColumn( column );
      columnWidth = tableColumn.getWidth() - (textArea.getMargin().left + textArea.getMargin().right);
      numberOfTextRows = (textWidth / columnWidth + 1);
      if ( text.indexOf( '\n' ) > -1 )
      {
        for ( int index = 0; index < text.length(); )
        {
          try
          {
            int index_1 = text.indexOf( '\n', index );
            int index_2 = text.indexOf( '\n', index + 1 );
            String line = text.substring( index_1, index_2 );
            textWidth = fm.stringWidth( line );
            numberOfTextRows_IncludingNewlines += ((textWidth / columnWidth) + 1);
            index = index_2;
          }
          catch ( StringIndexOutOfBoundsException ex )
          {
            break;
          }
        }
        numberOfTextRows = Math.max( numberOfTextRows, numberOfTextRows_IncludingNewlines );
      }
    }
    int rowHeight = fm.getHeight() * (numberOfTextRows + 1);
    if ( rowHeight > table.getRowHeight( row ) )
    {
      return rowHeight;
    }
    return 0;
  }

  public class MultiLineCellRenderer implements TableCellRenderer
  {
    public Component getTableCellRendererComponent( JTable table, Object value, boolean isSelected, boolean hasFocus,
        int row, int column )
    {
      String text = (value == null) ? "" : (String) value;
      textArea.setText( text );
      int height = getHeight( textArea, table, text, row, column );
      int curHeight = table.getRowHeight( row );
      if ( (curHeight != height) && (height > 0) )
      {
        table.setRowHeight( row, height );
      }
      return textArea;
    }

    JTextArea textArea = new JTextArea();
    {
      textArea.setEditable( false );
      textArea.setLineWrap( true );
      textArea.setMargin( new Insets( 0, 5, 0, 5 ) );
    }
  }
    
  public class MultiLineCellEditor extends AbstractCellEditor implements TableCellEditor
  {
    public Component getTableCellEditorComponent( JTable table, Object value, boolean isSelected, int row, int column )
    {
      String text = (value == null) ? "" : (String) value;
      textArea.setText( text );
      int height = getHeight( textArea, table, text, row, column );
      if ( height > 0 )
      {
        table.setRowHeight( row, height );
      }
      return scroll;
    }

    public Object getCellEditorValue()
    {
      return textArea.getText();
    }

    JTextArea   textArea = new JTextArea();
    {
      textArea.setLineWrap( true );
      textArea.setMargin( new Insets( 0, 5, 0, 5 ) );
    }

    JScrollPane scroll   = new JScrollPane();
    {
      scroll.setViewportView( textArea );
      scroll.getVerticalScrollBar().setPreferredSize( new Dimension( 11, 48 ) );
    }
  }
    
  public boolean isCellEditable( EventObject evt )
  {
    if ( evt instanceof MouseEvent )
    {
      // use double-click activation
      int clickCount = 2;
      return ((MouseEvent) evt).getClickCount() >= clickCount;
    }
    return true;
  }
}
