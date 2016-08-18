/** MatchListMainApp.java 
 * 
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 * 
 * Changes
 * 
 * 2009/02/06: - modified version string to 1.1.7
 * 
 * 2009/01/28: - modified version string to 1.1.6
 * 
 * 2008/01/31: - modified version string to 1.1.5
 * 
 * 2006/10/06: - use system look and feel
 *             - modified version string to 1.1.4
 * 
 * 2006/05/16: - modified version string to 1.1.3
 * */
package transmemoptimizer;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.Hashtable;
import java.util.Properties;
import java.util.Vector;

import javax.swing.Action;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDesktopPane;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JSeparator;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.UIManager;

/** A reusable application shell that provides the basic
    framework common to all graphical applications.  Specifically
    provides a menu, menubar, status panel and mdi interface. */
public class MatchListMainApp extends JFrame implements ActionListener
{

    /** Where internal frames are added to. */
    JDesktopPane desktop;
    /** Menu bar. */
    JMenuBar mBar;
    /** Tool bar. */
    JToolBar tBar;
    /** Used to map command names to actions. */
    Hashtable commands = new Hashtable();
    /** A class which displays a status panel. */
    StatusPanel status;
    /** Text field in the second box in the status panel. */
    JTextField secondStatusBox;

    /** Our properties and last used values */
    Properties props;
    
    /** our frame object */
    MatchListMainApp app;

    /* menu items */
    JMenu fileMenu;
    JMenuItem openMenuItem;
    JMenuItem saveMenuItem;
    JMenuItem exitMenuItem;
    JMenuItem saveAsMenuItem;
    JMenuItem applyMenuItem;
    JMenu helpMenu;
    JMenuItem aboutMenuItem;
    
    /** root directory of TranslationManager */ 
    String rootDirOfTM;
    
    /** our matchlist window */
    MatchListWindow mlw;

    /** accessor. */
    public JDesktopPane getDesktopPane() { return desktop; }

    /** Method to open the given match list in a window */
    public boolean openMatchListWindow( File matchListFile )
    {
      // close any open match list window
      if ( mlw != null )
      {
        mlw.dispose();
      }
      
      mlw = new MatchListWindow( this, matchListFile );
      
      if ( mlw != null )
      {
        saveMenuItem.setEnabled( true );
        saveAsMenuItem.setEnabled( true );
      }
      
      return( true );
    }

    /** No-arg constructor. */
    public MatchListMainApp( String inFile ) 
    {
        // call parent constructor
        super("Translation Memory Optimizer V 1.1.7");
    
        app = this;
        
        rootDirOfTM = Utils.getTranslationManagerRoot();
        
        
        setLookAndFeel();
        
        // load properties from file
        props = new Properties();
        
        try
        {
          props.load( new FileInputStream( "matchlist.properties" ) );
        }
        catch ( IOException ioe ) 
        {
          // load failed
        }
        
        // build menus
        mBar = buildMenus();
        this.setJMenuBar(mBar);
    
        // initial menu item states
        saveMenuItem.setEnabled( false );
        saveAsMenuItem.setEnabled( false );
        
        // build tool bar
        Container contentPane = this.getContentPane();
        contentPane.setLayout(new BorderLayout(2,2));
//        tBar = buildToolBar(commands);
//        contentPane.add("North", tBar);
    
        // build status panel
        secondStatusBox = new JTextField(20);
        secondStatusBox.setEditable(false);
        secondStatusBox.setText("  ");
        status = new StatusPanel(secondStatusBox);
        contentPane.add("South", status);
        status.setText(" ");
    
        // add Desktop pane (for MDI)
        desktop = new JDesktopPane();
        contentPane.add("Center", desktop);
        
        
        setDesktopSize(this);
        this.setVisible(true);
    
        // shutdown adapter
        this.addWindowListener(new WindowAdapter()
        {
          public void windowClosing(WindowEvent we)
          {
            // store window size and position
            Rectangle rect = app.getBounds();
            app.setProperty( "desktopX", rect.x );
            app.setProperty( "desktopY", rect.y );
            app.setProperty( "desktopWidth", rect.width );
            app.setProperty( "desktopHeight", rect.height );
            
            // save our properties
            try
            {
              props.save( new FileOutputStream( "matchlist.properties"  ), "Matchlist properties" );
            }
            catch ( IOException ioe ) 
            {
              // save failed
            }
            
            System.exit(0); 
          }
        });
        
        // open our start file
        if ( inFile != null )
        {
	        File matchListFile = new File( inFile );
	        openMatchListWindow( matchListFile );
        }
        else
        {
          openMatchList();          
        }
    }
    
    private void setLookAndFeel()
    {
      try
      {
        UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      }
      catch (Exception e)
      {
        try
        {
          UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
        }
        catch (Exception e1)
        {
          e1.printStackTrace();
        }
      }
    }

    /** Method to size a window. */
    public static void setDesktopSize(MatchListMainApp frame)
    {
//        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
        
        Rectangle rect = new Rectangle();
        rect.x = frame.getIntProperty( "desktopX", 50 );
        rect.y = frame.getIntProperty( "desktopY", 50 );
        rect.width = frame.getIntProperty( "desktopWidth", 600 );
        rect.height = frame.getIntProperty( "desktopHeight", 400 );
        frame.setBounds( rect );
    }

    /** Method to center the window. */
    public static void centerOnScreen(JFrame frame)
    {
        Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension window = frame.getSize();
        int iCenterX = screen.width / 2;
        int iCenterY = screen.height / 2;
        frame.setLocation(iCenterX - (window.width / 2),
                          iCenterY - (window.height / 2));
    }

    /** Method to add Menus & Menuitems to the MenuBar. */
    private JMenuBar buildMenus()
    {
        JMenuBar bar = new JMenuBar();

        // Create File Menu
        fileMenu = new JMenu("File");
        openMenuItem = addMenuItem(fileMenu,"Open...");
        saveMenuItem = addMenuItem(fileMenu,"Save");
        saveAsMenuItem = addMenuItem(fileMenu,"Save as...");
        
        fileMenu.add(new JSeparator());
        applyMenuItem = addMenuItem(fileMenu,"Apply changes...");

        fileMenu.add(new JSeparator());
        exitMenuItem = addMenuItem(fileMenu,"Exit");

        // Create Edit Menu
//        JMenu editMenu = new JMenu("Edit");
//        addMenuItem(editMenu,"Copy",commands);
//        addMenuItem(editMenu,"Cut", commands);
//        addMenuItem(editMenu,"Paste", commands);
//
        // Create Help Menu
        JMenu helpMenu = new JMenu("Help");
        aboutMenuItem = addMenuItem( helpMenu, "About...");
        
//        addMenuItem(helpMenu, "Help", commands);

        bar.add(fileMenu);
//        bar.add(editMenu);
        bar.add(helpMenu);
        return bar;
    }

    /** Method to add menuitems and connect the actions to the
        menubar. */
    private JMenuItem addMenuItem(JMenu m, String cmd )
    {
        JMenuItem anItem = new JMenuItem(cmd);
        anItem.addActionListener(this);
        m.add(anItem);
        return( anItem );
    }


//    /** Method to construct a toolbar. */
//    private JToolBar buildToolBar(Hashtable commands)
//    {
//        JToolBar bar = new JToolBar();
//
//        // lock it in
//        bar.setFloatable(false);
//        bar.setBorderPainted(true);
//
//        // New, Open, Save, Copy, Cut, Paste
//        // addButton(bar, "New", commands);
//        addButton(bar, "Open", commands);
//        addButton(bar, "Save", commands);
//        addButton(bar, "Copy", commands);
//        addButton(bar, "Cut", commands);
//        addButton(bar, "Paste", commands);
//
//        return bar;
//    }

    /** Method to add a button to a toolbar. */
    private void addButton(JToolBar tBar, String cmd, Hashtable commands)
    {
//        URL loc = this.getClass().getResource("Images" + File.separator + (cmd.toUpperCase()) + ".GIF");
//        URL loc = this.getClass().getResource("Images." + (cmd.toUpperCase()) + ".GIF");
        URL loc = this.getClass().getResource("matchlistviewer.Images" + File.separator + (cmd.toUpperCase()) + ".GIF");
        ImageIcon ic = new ImageIcon(loc);
        JButton aButton = new JButton(ic);
        aButton.addActionListener((Action)commands.get(cmd));
        tBar.add(aButton);
    }

    /** Main method to invoke from the command line. */
    public static void main(String args[])
    {
      String startFile = null;
      if ( args.length >= 1 )
      {
        startFile = args[0];
      }
      new MatchListMainApp( startFile );
    }
    
    /** Method to set a property value */
    public void setProperty( String key, String value)
    {
      props.setProperty( key, value );
    }
    
    /** Method to set an property value */
    public void setProperty( String key, int  value)
    {
      props.setProperty( key, Integer.toString( value ) );
    }

    public void setProperty( String key, boolean  value)
    {
      props.setProperty( key, value ? "true" : "false" );
    }

    /** Method to get a property value */
    public String getProperty(String key)
    {
      return( props.getProperty( key ) );
    }
    
    /** Method to get a integer property value */
    public int getIntProperty(String key)
    {
      return( this.getIntProperty( key, 0 ) );
    }
    public int getIntProperty(String key, int iDefault )
    {
      String value = props.getProperty( key, Integer.toString( iDefault ) );
      return( Integer.parseInt( value ));
    }
    
    public String getProperty(String key, String defaultValue )
    {
      return( props.getProperty( key, defaultValue ) );
    }

    public boolean getBoolProperty( String key )
    {
      String value = props.getProperty( key, "false" );
      return( "true".equalsIgnoreCase( value ));
    }

    public void removeProperty(String key)
    {
      props.remove( key );
      return;
    }

    public void actionPerformed( ActionEvent ae )
    {
      if ( ae.getSource() == exitMenuItem )
      {
        System.exit(0);
      }
      else if ( ae.getSource() == applyMenuItem )
      {
        if ( mlw != null )
        {
          mlw.applySelectedChanges();
        }
        else
        {
          ChangeOptions options = new ChangeOptions();
          
          ChangeList changeList = new ChangeList();
          options.markupChangeList = new ChangeList();
          MatchListWindow.applyChanges( this, changeList, options );
        }
      }
      else if ( ae.getSource() == saveMenuItem )
      {
        if ( mlw != null )
        {
          mlw.saveMatchList();
        }
      }
      else if ( ae.getSource() == saveAsMenuItem )
      {
        if ( mlw != null )
        {
          File file = mlw.getCurrentMatchListFile();
          JFileChooser fc = new JFileChooser();
          
          fc.addChoosableFileFilter(new XmlFileFilter());
          
          fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
          fc.setMultiSelectionEnabled( false );
          fc.setDialogTitle( "Save segment list" ); 
  	      fc.setCurrentDirectory( file );
          
          int returnVal = fc.showSaveDialog( app );
          
          if ( returnVal == JFileChooser.APPROVE_OPTION )
          {
            file = fc.getSelectedFile();
            if ( file.exists() )
            {
              Object[] options = { "OK", "CANCEL" };
              int iResult = JOptionPane.showOptionDialog(null, 
                  "Segment list " + file.getName() + " exists already\n" +
                  "Do you want to overwrite the list?", 
                  "Overwrite Confirmation",
                  JOptionPane.YES_OPTION, JOptionPane.QUESTION_MESSAGE,
                  null, options, options[0]);  
              if ( iResult == 0 )
              {
                mlw.saveMatchList( file );
              }
            }
            else
            {
              mlw.saveMatchList( file );
            }
          }
        }
      }
      else if ( ae.getSource() == openMenuItem )
      {
        openMatchList();
      }
      else if ( ae.getSource() == aboutMenuItem )
      {
        AboutDialog dlg = new AboutDialog();
        dlg.setModal( true );
        dlg.setVisible( true );
      }
    }


    private void openMatchList()
    {
      JFileChooser fc = new JFileChooser();
      
      fc.addChoosableFileFilter(new XmlFileFilter());
      
      // apply last used value
      String lastUsedDir;
      if ( rootDirOfTM != null )
      {
        lastUsedDir = new String( rootDirOfTM + "\\MATCHLIST" );
      }
      else
      {
        lastUsedDir = app.getProperty( "lastopendir" );
      }
      
      fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
      fc.setMultiSelectionEnabled( false );
      fc.setDialogTitle( "Open a match list" ); 
      if ( lastUsedDir != null )
      {
	      File fileLastUsedDir = new File( lastUsedDir );
	      fc.setCurrentDirectory( fileLastUsedDir );
      }
      
      int returnVal = fc.showOpenDialog( app );
      
      if ( returnVal == JFileChooser.APPROVE_OPTION )
      {
        File file = fc.getSelectedFile();
        File dir = fc.getCurrentDirectory();
        app.setProperty( "lastopendir", dir.getPath() ); 

        openMatchListWindow( file );
      }
      
    }
    
    public String getTMRootDir()
    {
      return( rootDirOfTM );
    }
}













