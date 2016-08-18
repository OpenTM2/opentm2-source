/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom;

import java.awt.Color;
import java.awt.Component;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.DropMode;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JTree;
import javax.swing.ToolTipManager;
import javax.swing.TransferHandler;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreePath;
import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.models.CommandFrame;
import de.ibm.com.opentm2scripteride.models.CommandModel;
import de.ibm.com.opentm2scripteride.models.ScriptModel;

/**
 * A tree that renders a ScriptModel. It includes drag&drop support.
 */
public class ScriptOutlineTree extends JTree {
	private static final long serialVersionUID = 1826389177764909363L; //generated
	protected ScriptModel mModel;
	
	/**
	 * Constructs an empty tree
	 */
	public ScriptOutlineTree() {
		this(null);
	}
	
	/**
	 * Constructs a tree to render the given ScriptModel
	 * @param sm The ScriptModel to be rendered
	 */
	public ScriptOutlineTree(ScriptModel sm) {
		super(sm);
		setTransferHandler(new ScriptOutlineTransferHandler(this, TransferHandler.COPY));
		setDropMode(DropMode.ON);
		ToolTipManager.sharedInstance().registerComponent(this);
		addMouseListener(new OutlineMouseListener());
		setCellRenderer(new OutlineCellRenderer());
	}
	
	/**
	 * Returns the model the tree is currently rendering
	 * @return The model the tree is currently rendering
	 */
	public ScriptModel getModel() {
		return mModel;
	}

	/**
	 * Sets another model the tree should render
	 * @param model The new model to be rendered by the tree
	 */
	public void setModel(ScriptModel model) {
		//this line is necessary as there will be misbehavior otherwise 
		super.setModel(null);
		super.setModel(model);
		mModel = model;
		repaint();
	}

	/**
	 * Opens a ScriptCommandDialog on doubleclick or sets the caret of the editor to the corresponding line on single click
	 */
	protected class OutlineMouseListener extends MouseAdapter {
		public void mousePressed(MouseEvent e) {
			//find out which node was clicked and check if it's a real CommandModel that we need
			TreePath tp = getPathForLocation(e.getX(), e.getY());
			if (tp == null) {
				return;
			}
			Object o = tp.getLastPathComponent();
			if ( !(o instanceof CommandModel) ) {
				return;
			}
			CommandModel cm = (CommandModel) o;
			switch (e.getClickCount()) {
				case 1:
					//jump to line
					MainApp.getInstance().getActiveEditor().setCaretPosition(cm.getStartLine(), 0);
					break;
				
				case 2:
					//open dialog
					if (cm.getFrame().getParameters().size() < 1) {
						return;
					}
					ScriptCommandDialog dlg = new ScriptCommandDialog(cm);
					dlg.setVisible(true);
					break;
					
				default:
					//nothing
					break;
			}
		}
	}


	/**
	 * Set proper icons and tooltips for the different CommandModels in the rendered ScriptModel
	 */
	protected class OutlineCellRenderer extends DefaultTreeCellRenderer {
		private static final long serialVersionUID = -1038539313231297541L; //generated
		ImageIcon mApiCallIcon;
		ImageIcon mBlockIcon;
		ImageIcon mCommentIcon;
		ImageIcon mDefineIcon;
		ImageIcon mFunctionIcon;
		ImageIcon mScriptModelIcon;
		ImageIcon mUnknownIcon;
		
		public OutlineCellRenderer() {
			//get all needed icons
			mApiCallIcon = new ImageIcon("resources/icons/apicall.png");
			mBlockIcon = new ImageIcon("resources/icons/block.png");
			mCommentIcon = new ImageIcon("resources/icons/comment.png");
			mDefineIcon = new ImageIcon("resources/icons/define.png");
			mFunctionIcon = new ImageIcon("resources/icons/function.png");
			mScriptModelIcon = new ImageIcon("resources/icons/script.png"); 
			mUnknownIcon =  new ImageIcon("resources/icons/unknown.png");
		}

		public Component getTreeCellRendererComponent(JTree tree, Object value, boolean sel,
													  boolean expanded, boolean leaf, int row,
													  boolean hasFocus) {
			//first set the default stuff
			super.getTreeCellRendererComponent(tree, value, sel, expanded,	leaf, row, hasFocus);
			
			//now check for highlighting
			this.setBorder(null);
			
			//now be sure that we have a CommandModel object and sepcify the rest;
			if ( !(value instanceof CommandModel) ) {
				return this;
			}
			CommandModel cmdModel = (CommandModel) value;
			boolean valid = cmdModel.validates();
			if (!valid) {
				setForeground(Color.RED);
			}
			
			//set the correct icon and tooltip
			setIcon(chooseIcon(cmdModel));
			
			//set tooltip
			String tooltip = null;
			if (cmdModel == mModel) { //check if root element
				//if the root element isn't valid there are errors in the script
				tooltip = valid ? "There are no errors in the script" : "There are errors existing in the script";
			} else { //otherwise get an idea of the line
				tooltip = cmdModel.getBody().toHTML();
			}
			setToolTipText(tooltip.isEmpty() ? null : tooltip);

			return this;
		}
		
		protected Icon chooseIcon(CommandModel cm) {
			CommandFrame frame = cm.getFrame();
			String id = frame.getId();
			
			//specials first
			if (id.equalsIgnoreCase("define")) {
				return mDefineIcon;
			} else if (id.equalsIgnoreCase("commentblock")) {
				return mCommentIcon;
			} else if (id.equalsIgnoreCase("comment")) {
				return mCommentIcon;
			}
				
			//category
			switch (frame.getType()) {
				case Block:
				case Wrapper:
					return mBlockIcon;
					
				case ApiCall:
					return mApiCallIcon;
					
				case Function:
					return mFunctionIcon;
					
				case Unknown:
					if (cm instanceof ScriptModel) {
						return mScriptModelIcon;
					}
					return mUnknownIcon;
					
				default:
					if (cm.size() > 0) {
						return mBlockIcon;
					}
					return getIcon();
			}
		}
	}
}
