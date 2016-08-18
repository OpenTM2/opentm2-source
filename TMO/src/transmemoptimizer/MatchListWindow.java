/*
 * Created on 03.10.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JInternalFrame;
import javax.swing.JLabel;
import javax.swing.JLayeredPane;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.ProgressMonitor;
import javax.swing.ProgressMonitorInputStream;
import javax.swing.event.InternalFrameAdapter;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;
import javax.swing.Timer;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MatchListWindow extends JInternalFrame
{
  protected MatchListMainApp app;
  protected JTable table;
  protected JInternalFrame frame;
  private JButton applyButton;
  private JButton selectAllButton;
  private JButton deselectAllButton;
  private JButton deleteButton;
  private MatchListTableModel model;
  private MatchListWindow matchlistwindow;
  private MatchList matchList;
  private File currentMatchListFile;
  private TableSorter sorter;

  
  private final int defColumnWidth[] = { 40, 60, 100, 100, 100, 80, 80, 100, 120, 100 };
  private final int defColumnMinWidth[] = { 20, 20, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60 };
  private final int defColumnMaxWidth[] = { 100, 100, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600 };
  
  public MatchListWindow( MatchListMainApp shell, File matchListFile )
  {
    app = shell;
    frame = this;
    matchlistwindow = this;
    
    app.status.setText("Opening segment list " + matchListFile.getName() ); 
    
    // load the match list into memory
    matchList = new MatchList();
    matchList.load( matchListFile.getAbsolutePath() );
    currentMatchListFile = matchListFile;
    
    // Create a new JInternalFrame and add it to the desktop
    this.setTitle( "Segment list " + matchListFile.getName() );
    
    // restore last used size/position
    setLastUsedSize( "MatchListWindow" );
    
    this.getContentPane().setLayout(new BorderLayout());
    this.setResizable( true );
    this.setClosable( true );
    this.setIconifiable( true );
    this.setMaximizable( true );
 
    TableModelListener valueListenener = new TableModelListener()
    {
      public void tableChanged(TableModelEvent e) 
      {
        app.secondStatusBox.setText( " " + model.getCheckedElements() + " of " + model.getRowCount() + " selected ");
      }
    };
    
    
    model = new MatchListTableModel( matchList );
    model.addTableModelListener( valueListenener );
    
    sorter = new TableSorter( model ); 

    // Create a JTable and tell it to display our model
    table = new JTable(sorter);
    sorter.setTableHeader(table.getTableHeader());
    
    TableColumnModel columnModel = table.getColumnModel();
    int iColumns = columnModel.getColumnCount();
    
    MyTableCellRenderer tr = new MyTableCellRenderer ();
    for (int c = 0; c < iColumns; c++) // skip checkbox column!
    {
      TableColumn tc = columnModel.getColumn (c);
      switch ( c )
      {
        case 0: break; // no specific renderer for column one
        case 1: tc.setCellRenderer (tr);  break; 
        case 2: tc.setCellRenderer (new MultiLineCellRenderer() );  break; 
        case 3: tc.setCellRenderer (new MultiLineCellRenderer() );  break; 
        case 4: tc.setCellRenderer (tr);  break; 
        case 5: tc.setCellRenderer (tr);  break; 
        case 6: tc.setCellRenderer (tr);  break; 
        case 7: tc.setCellRenderer (tr);  break; 
        case 8: tc.setCellRenderer (new MultiLineCellRenderer() );  break; 
        case 9: tc.setCellRenderer (tr);  break; 
      }
    }	        
   
    table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF); 
    
    for ( int i = 0; i < iColumns; i++ )
    {
      TableColumn column = columnModel.getColumn( i );
      
      int width = app.getIntProperty( "columnWidth" + i, defColumnWidth[i] );
      column.setMaxWidth( defColumnMaxWidth[i] );
      column.setMinWidth( defColumnMinWidth[i] );
      column.setPreferredWidth( width );
      column.setWidth( width );
    }
    
//    int lines = 3;
//    table.setRowHeight( table.getRowHeight() * lines);
    
    
//    for ( int i = 2; i <= 4; i++ )
//    {
//      column = columnModel.getColumn( i );
//      column.setCellRenderer( new MultiLineCellRenderer() );
//    }
    
    
    // Display it all in a scrolling window and make the window appear
    this.getContentPane().add(new JScrollPane(table), "Center");
    
    // add buttons
    BL listener = new BL();
    
    JPanel bottomPanel = new JPanel();
    bottomPanel.setLayout( new BorderLayout() );
    
    JPanel buttonPanel = new JPanel();
    buttonPanel.setLayout( new FlowLayout() );
    
    selectAllButton = new JButton( "Select All");
    selectAllButton.addActionListener( listener );
    selectAllButton.setToolTipText( "Select all entries in segment list" );
    buttonPanel.add( selectAllButton );
    
    deselectAllButton = new JButton( "Deselect All");
    deselectAllButton.addActionListener( listener );
    deselectAllButton.setToolTipText( "Deselect all entries in the segment list" );
    buttonPanel.add( deselectAllButton  );
    
    deleteButton = new JButton( "Delete selected entries");
    deleteButton.addActionListener( listener );
    deleteButton.setToolTipText( "Delete all selected entries in the segment list" );
    buttonPanel.add( deleteButton );
    
    applyButton = new JButton( "Apply selected changes to memory");
    applyButton.addActionListener( listener );
    applyButton.setToolTipText( "Apply the changes of the selected entries to a memory" );
    buttonPanel.add( applyButton );
    
    String labelText = "<html>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" +
                       "(*) in Fuzziness column: the replacement will create " +
                       "a fuzzy match only as <font color=\"#009900\">inserted</font> " +
                       "strings cannot be changed " +
                       "automatically<<html>";
    bottomPanel.add( new JLabel( labelText ), BorderLayout.NORTH );
    
    bottomPanel.add( buttonPanel, BorderLayout.SOUTH );
    this.getContentPane().add( bottomPanel, "South");

    // window close adapter
    this.addInternalFrameListener(new InternalFrameAdapter()
    {
      public void internalFrameClosing( InternalFrameEvent we)
      {
        // store column widths in properties
        app.setProperty( "column1", "5" );
        TableColumnModel columnModel = table.getColumnModel();
        int iColumns = columnModel.getColumnCount();
        for ( int i = 0; i < iColumns; i++ )
        {
          TableColumn column = columnModel.getColumn( i );
          app.setProperty( "columnWidth" + i, column.getWidth() );
        }
        saveLastUsedSize( "MatchListWindow" );
      }
    });

    
    // By default, internal frames are not visible; make it visible
    this.setVisible(true);
    
    app.desktop.add(this, JLayeredPane.DEFAULT_LAYER);
    app.status.setText("Segment list loaded.");
    app.secondStatusBox.setText( " " + matchList.getLength() + " entries ");

    app.desktop.repaint();
   

    
  }

  /** our button listener */
  class BL implements ActionListener
  {
    public void actionPerformed( ActionEvent event )
    {
      if ( event.getSource() == applyButton )
      {
        applySelectedChanges();        
      }
      else if ( event.getSource() == selectAllButton )
      {
        sorter.setUpdateInhibited( true );
        int iMax = model.getRowCount();
        for( int i = 0; i < iMax; i++ )
        {
          model.setValueAt( Boolean.TRUE, i, 0 );
        }
        sorter.setUpdateInhibited( false );
      }
      else if ( event.getSource() == deleteButton )
      {
        model.deleteSelected();
      }
      else if ( event.getSource() == deselectAllButton )
      {
        sorter.setUpdateInhibited( true );
        int iMax = model.getRowCount();
        for( int i = 0; i < iMax; i++ )
        {
          model.setValueAt( Boolean.FALSE, i, 0 );
        }
        sorter.setUpdateInhibited( false );
      }
    }
  }
  
  protected static void changeMemory( MatchListMainApp mainapp, ChangeList changeList, ChangeOptions changeOptions ) 
  {
    mainapp.status.setText( "Changing memory..." );
    
    String inMemory = mainapp.getProperty( "InputMemory");
    String outMemory = mainapp.getProperty( "OutputMemory");
    String removeLinkEndString = mainapp.getProperty( "RemoveLinkEnd" );
    boolean removeLinkEnds = false;
    if ( removeLinkEndString != null )
    {
      removeLinkEnds = removeLinkEndString.equals( "true"); 
    }
    
    ChangeMemoryJob job = new ChangeMemoryJob( inMemory, outMemory, changeList, changeOptions  );
    job.setRemoveLinkEnd( removeLinkEnds );
    new ProgressDialog( mainapp, job );
    
    mainapp.status.setText( "Change complete" );

    Object[] options = { "OK" };
    File mem = new File( outMemory );
    int iResult = JOptionPane.showOptionDialog(null, 
        "The change of output memory " + mem.getName() + " is complete",
        "Info",
        JOptionPane.OK_OPTION, JOptionPane.INFORMATION_MESSAGE,
        null, options, options[0]);  
  }
  
  /**  restore last used size/position */
  protected void setLastUsedSize( String windowName )
  {
    Rectangle rect = new Rectangle();
    rect.height = app.getIntProperty( windowName + "Height", 200 );
    rect.width = app.getIntProperty( windowName + "Width", 600 );
    rect.x = app.getIntProperty( windowName + "PosX", 50 );
    rect.y = app.getIntProperty( windowName + "PosY", 50 );
    this.setBounds( rect );
  }

  /**  apply last used size/position */
  protected void saveLastUsedSize( String windowName )
  {
    Rectangle rect = frame.getBounds();
    app.setProperty( windowName + "Height", rect.height );
    app.setProperty( windowName + "Width", rect.width );
    app.setProperty( windowName + "PosX", rect.x );
    app.setProperty( windowName + "PosY", rect.y );
  }

  protected void addToChangeList( ChangeList changeList, MatchEntry entry )
  {
    Vector changes = entry.getChanges();
    int changeEntries = changes.size();
    int j = 0;
    boolean insertChangesInList = false;
    
    String allChangeFrom = new String();
    String allChangeTo = new String();
    
    while ( j < changeEntries )
    {
      String changeCommand = (String)changes.get( j );
      if ( changeCommand.equals( "#delete#" ) )
      {
        j++;
        if ( j < changeEntries  )
        {
          String changeText = (String)changes.get( j );
          if ( allChangeFrom.length() != 0 )
          {
            allChangeFrom = allChangeFrom.concat( "\n" );
            allChangeTo = allChangeTo.concat( "\n" );
          }
          allChangeFrom = allChangeFrom.concat( changeText );
          j++;
        }
      }
      else if ( changeCommand.equals( "#insert#" ) )
      {
        j++;
        if ( j < changeEntries  )
        {
          insertChangesInList = true;
          j++;
        }
      }
      else if ( changeCommand.equals( "#modify#" ) )
      {
        j++;
        if ( j < changeEntries  )
        {
          String changeFrom = (String)changes.get( j );
          j++;
          if ( j < changeEntries  )
          {
            String changeTo = (String)changes.get( j );
            if ( allChangeFrom.length() != 0 )
            {
              allChangeFrom = allChangeFrom.concat( "\n" );
              allChangeTo = allChangeTo.concat( "\n" );
            }
            allChangeFrom = allChangeFrom.concat( changeFrom );
            allChangeTo = allChangeTo.concat( changeTo );
            j++;
          }
        }
      }
      else
      {
        j++;
      }
    } /* end while */
    
    // GQ: add replacements strings even if there is inserted text in the segment
//    if ( !insertChangesInList && (allChangeFrom.length() != 0) )
    if ( allChangeFrom.length() != 0 )
    {
      changeList.add( allChangeFrom, allChangeTo );
    }
  }

  public void saveMatchList( File file )
  {
    matchList.save( file );
    JOptionPane.showMessageDialog(null, "Segment list " + file.getName() + " has been saved." );
    
    currentMatchListFile = file;
    this.setTitle( "Segment list  " + file.getName() );
  }
  
  public void saveMatchList()
  {
    saveMatchList( currentMatchListFile );  
  }

  public File getCurrentMatchListFile()
  {
    return ( currentMatchListFile );
  }
  
  public static void applyChanges( MatchListMainApp mainapp, ChangeList changeList, ChangeOptions options )
  {
    // show apply dialog
    ApplyChangesDialog dlg = new ApplyChangesDialog( mainapp, mainapp, changeList, options  );
    dlg.setModal( true );
    dlg.setVisible( true );

    // start memory change
    if ( !dlg.isDialogCancelled() )
    {
      // merge any external change list into our internal change list
      changeList = dlg.getInternalChangeList();
      changeList.add( dlg.getExternalChangeList() );
      
      // change the memory
      MatchListWindow.changeMemory( mainapp, changeList, options );
    }
  }

  public void applySelectedChanges()
  {
    ChangeOptions options = new ChangeOptions();
    // setup list of selected changes
    ChangeList changeList = new ChangeList();
    options.markupChangeList = new ChangeList();
    
    int numOfEntries = matchList.getLength();
    for( int i = 0; i < numOfEntries; i++ )
    {
      MatchEntry entry = matchList.getAt( i );
      if ( entry.isSelected() )
      {
        addToChangeList( changeList, entry );
        String matchMarkup = entry.getMatchMarkup();
        String segMarkup = entry.getMarkup();
        if ( !matchMarkup.equals( segMarkup ))
        {
          options.markupChangeList.add( matchMarkup, segMarkup );
        }
      }
    }
    
    applyChanges( app, changeList, options );
  }
}
