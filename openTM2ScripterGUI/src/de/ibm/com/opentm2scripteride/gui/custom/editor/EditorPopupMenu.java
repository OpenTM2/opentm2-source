/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom.editor;

import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.DataFlavor;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.List;

import javax.swing.ImageIcon;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.gui.custom.ScriptCommandDialog;
import de.ibm.com.opentm2scripteride.models.CommandCatalog;
import de.ibm.com.opentm2scripteride.models.CommandFrame;
import de.ibm.com.opentm2scripteride.models.CommandModel;
import de.ibm.com.opentm2scripteride.models.ScriptModel;

/**
 * A context menu for the editor that allows the user to edit the command in a dialog or cut, copy or paste text
 */
public class EditorPopupMenu extends JPopupMenu {
	
	private static final long serialVersionUID = -7108163007136044240L; //generated
	protected JEditTextArea mTextArea;
	protected CommandModel mModel;

	/**
	 * Creates the context menu
	 * @param textArea The corresponding JEditTextArea (the editor)
	 * @param line The line it refers to
	 */
	public EditorPopupMenu(JEditTextArea textArea, int line) {
		mTextArea = textArea;
		mModel = null;
		//check if we have a command in this line to be edited
		if (line >= 0) {
			ScriptModel sModel = MainApp.getInstance().getMainWindow().getOutlineTree().getModel();
			 mModel = sModel.findByStartingLine(line, true);
			 addEditCommand(line);
			//if (mModel != null) {
			//	addEditCommand();
			//}
		}
		
		//now clipboard actions
		Clipboard clipboard = getToolkit().getSystemClipboard();
		//check if we have selected somethin. If not Cur/Copy should be diabled
		boolean disable = (mTextArea.getSelectedText() == null);
		
		//first cut action
		JMenuItem cut = new JMenuItem("Cut", new ImageIcon("resources/icons/cut.png"));
		cut.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				mTextArea.cut();
			}
		});
		if (disable) {
			cut.setEnabled(false);
		}
		add(cut);
		
		//then copy action
		JMenuItem copy = new JMenuItem("Copy", new ImageIcon("resources/icons/copy.png"));
		copy.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				mTextArea.copy();
			}
		});
		if (disable) {
			copy.setEnabled(false);
		}
		add(copy);

		//last but not least paste action
		JMenuItem paste = new JMenuItem("Paste", new ImageIcon("resources/icons/paste.png"));
		paste.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				mTextArea.paste();
			}
		});
		try {
			//enable the action only if we have valid content on the clipboard
			String selection = (String) clipboard.getContents(this).getTransferData(DataFlavor.stringFlavor);
			if (selection == null) {
				paste.setEnabled(false);
			}
		} catch(Exception e) {
			paste.setEnabled(false);
		}
		add(paste);
		
	}

	/**
	 * a replacement for addEditCommand when mModel is not created 
	 * it will create a new mModel with parameter frame if could then continue
	 * @param line
	 */
	protected void addEditCommand(int iLine) {
		if(mModel==null || mModel.getFrame().getType()==CommandFrame.Type.Unknown) {
		
			if(mTextArea == null || iLine<0)
				return;
			
			String line = mTextArea.getLineText(iLine).trim();
			if(line==null || line.isEmpty())
				return;
			
			// command or control structure all at line start
			String cmd = line.split(" ")[0];
			if(cmd==null || cmd.isEmpty())
				return;
			
			// get all commands
			List<CommandFrame> commands = CommandCatalog.getInstance().toList();
			for(CommandFrame command:commands) {
				if( command.getCommand()!=null && command.getCommand().equalsIgnoreCase(cmd) ) {
					mModel = new CommandModel(iLine, command);
					break;
				}//end if
			}//end for
			
		}//end if
		
		// call the original function continue to do
		addEditCommand();
	}
	
	
	/**
	 * Adds the menu item to edit a command if it has the correct type (not unknown, comment, wrapper)
	 */
	protected void addEditCommand() {
        if(mModel == null)
        	return;
        
		CommandFrame.Type type = mModel.getFrame().getType();
		if (type == CommandFrame.Type.Unknown ||
			type == CommandFrame.Type.Wrapper
			) {
			return;
		}
		String id = mModel.getFrame().getId();
		if(	id.equals("comment") ||
			id.equals("commentblock")
		) {
			return;
		}

		JMenuItem edit = new JMenuItem("Edit Command", new ImageIcon("resources/icons/apicall.png"));
		edit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				if (mModel == null || mModel.getFrame().getParameters().size() < 1) {
					return;
				}
				ScriptCommandDialog dlg = new ScriptCommandDialog(mModel);
				dlg.setVisible(true);
			}
		});
		add(edit);
		addSeparator();
	}
}
