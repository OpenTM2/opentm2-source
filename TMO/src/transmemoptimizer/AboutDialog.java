/*
 * Created on 14.10.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 * 
 * 
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
public class AboutDialog extends javax.swing.JDialog implements ActionListener
{
  private JButton okButton;
  
	public AboutDialog () 
	{
		initGUI();
	    Point pt = new Point( 200, 130 );
	    this.setLocation( pt );
	}

	private void initGUI() 
	{
	  try 
	  {
	    this.setSize(620, 350 );
//	    this.setResizable( false );
	    this.setTitle("About Transation Memory Optimizer");
	    {
	      JPanel outerPanel = new JPanel();
	      BorderLayout jMainPanelLayout = new BorderLayout();
	      outerPanel.setPreferredSize(new java.awt.Dimension(300, 200));
	      outerPanel.setLayout(jMainPanelLayout);
	      this.getContentPane().add(outerPanel, BorderLayout.CENTER);
          JPanel mainPanel = new JPanel();
          JPanel jButtonPanel = new JPanel();
          
	      {
	        outerPanel.add(mainPanel, BorderLayout.NORTH);
	        FlowLayout jPanel1Layout = new FlowLayout();
	        mainPanel.setLayout(jPanel1Layout);
	      }
	      {
	        FlowLayout jButtonPanelLayout = new FlowLayout();
	        jButtonPanel.setLayout(jButtonPanelLayout);
	        {
	          okButton = new JButton( "OK" );
	          jButtonPanel.add(okButton);
	          okButton.setToolTipText("Close about window");
	          okButton.addActionListener( this );
	        }
	        outerPanel.add(jButtonPanel, BorderLayout.SOUTH);
	      }
	      {
	        mainPanel.add( new JLabel( "<html><br>" + 
	                                   "<center><h1>Translation Memory Optimizer Version 1.1.8</h1></center><br><br>" + 
	                                   "<center>Copyright (C) IBM Corporation 2005, 2015</center><br>" + 
	                                   "<center>All rights reserved</center><br>" +
	                                   "<br>" +
	        		                   "<center>This tool may only be used for IBM internal translation purposes.</center><br>" +
	        				           "<center>Do not distribute this tool or the information about it to " +
	        				           "anybody outside the IBM translation community</center><br>" +
	        				           "<center>For any questions regarding this tool please contact <u>tmemea@de.ibm.com</u></center></html>" ) );
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
    if ( event.getSource() == okButton )
    {
      setVisible( false );
      dispose();
    }
  }
}
