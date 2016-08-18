/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui;

import java.awt.FlowLayout;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;

public class TabTitlePanel extends JPanel{
    
	private static final long serialVersionUID = 1L;
	private JLabel mTitle;
	//private JLabel mIcon;
    private CloseButton mBtnClose;
    private final JTabbedPane mPane;
    
    public TabTitlePanel(String s,JTabbedPane pane){
        super(new FlowLayout(FlowLayout.LEFT, 2, 0));
        mTitle=new JLabel(s);
        this.mPane=pane;
        mBtnClose=new CloseButton();
        //mIcon = new JLabel(new ImageIcon("resources/icons/editor.png"));
        
        //add(mIcon);
        add(mTitle);
        add(mBtnClose);
        
        mTitle.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 5));
        setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
        setOpaque(false);
    }
 
    public void setTitle(String title, int tabIndex) {
    	mTitle.setText(title);
    	mPane.setTabComponentAt(tabIndex, this);
    }
    
    private class CloseButton extends JButton {
		private static final long serialVersionUID = 1L;
		private ImageIcon icon;
        public CloseButton(){
            icon=new ImageIcon("resources/icons/close_tab.png");
            setSize(icon.getImage().getWidth(null),icon.getImage().getHeight(null));
            setIcon(icon);
            setBorder(null);
            setBorderPainted(false);
            setFocusPainted(false);
            addMouseListener(new MouseAdapter(){
                public void mouseClicked(MouseEvent e){
                	int tabIndex = mPane.indexOfTabComponent(TabTitlePanel.this);
    				if(tabIndex != -1) {
                	    WindowConnector.getInstance().closeFile(mPane, tabIndex);
    				}
                }
            });
        }
    }
     
}