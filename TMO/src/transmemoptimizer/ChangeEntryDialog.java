/*
 * Created on 14.10.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ChangeEntryDialog extends javax.swing.JDialog implements ActionListener
{
  private boolean dlgCancelled;
  private JButton addButton;
  private JButton cancelButton;
  private JTextField changeFromField;
  private JTextField changeToField;
  private ChangeEntry changeEntry;
  
	public ChangeEntryDialog (JDialog parent, ChangeEntry entry  ) 
	{
		super(parent);
	    
		initGUI();
	    Point pt = new Point( 200, 130 );
	    this.setLocation( pt );
		changeEntry = entry;
	    dlgCancelled = true;

	}

	private void initGUI() 
	{
	  try 
	  {
	    this.setSize(300, 200 );
	    this.setResizable( false );
	    this.setTitle("Create a new replacement entry");
	    {
	      JPanel outerPanel = new JPanel();
	      BorderLayout jMainPanelLayout = new BorderLayout();
	      outerPanel.setPreferredSize(new java.awt.Dimension(778, 494));
	      outerPanel.setLayout(jMainPanelLayout);
	      this.getContentPane().add(outerPanel, BorderLayout.CENTER);
          JPanel mainPanel = new JPanel();
          JPanel jButtonPanel = new JPanel();
          
	      {
	        outerPanel.add(mainPanel, BorderLayout.NORTH);
	        GridLayout jPanel1Layout = new GridLayout( 4, 1, 10, 10 );
	        mainPanel.setLayout(jPanel1Layout);
	      }
	      {
	        outerPanel.add(jButtonPanel, BorderLayout.SOUTH);
	        FlowLayout jButtonPanelLayout = new FlowLayout();
	        jButtonPanel.setLayout(jButtonPanelLayout);
	        {
	          addButton = new JButton( "OK" );
	          jButtonPanel.add(addButton);
	          addButton.setToolTipText("Add the new entry to the replacement list");
	          addButton.addActionListener( this );
	        }
	        {
	          cancelButton = new JButton( "Cancel");
	          jButtonPanel.add(cancelButton);
	          cancelButton.setToolTipText( "Cancel this window" );
	          cancelButton.addActionListener( this );
	        }
	      }
	      {
	        mainPanel.add( new JLabel( "Change from") );
	        changeFromField = new JTextField( "" );
	        mainPanel.add( changeFromField );
	        mainPanel.add( new JLabel( "Change to") );
	        changeToField = new JTextField( "" );
	        mainPanel.add( changeToField );
	        outerPanel.add( new JLabel( "Use | as delimiter for conditional change commands"), BorderLayout.CENTER );
	      }
	    }
	  } 
	  catch (Exception e) 
	  {
	    e.printStackTrace();
	  }
	}
	


  public void actionPerformed( ActionEvent event )
  {
    if ( event.getSource() == cancelButton )
    {
      setVisible( false );
      dispose();     
    }
    else if ( event.getSource() == addButton )
    {
      String fromString = changeFromField.getText();
      String toString = changeToField.getText();
      changeEntry.setChangeFrom( fromString.replace( '|', '\n' )  );
      changeEntry.setChangeTo( toString.replace( '|', '\n' ) );
      if ( changeEntry.getChangeFrom().length() == 0 )
      {
        Utils.showErrorMessage( "change from text is required" );
        changeFromField.requestFocus();
      }
      else
      {
        dlgCancelled = false;
        setVisible( false );
        dispose();
      }
    }
  }

  public boolean isCancelled()
  {
    return( dlgCancelled );
  }
}
