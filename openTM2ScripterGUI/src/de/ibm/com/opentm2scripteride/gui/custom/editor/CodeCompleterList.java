/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
 */
package de.ibm.com.opentm2scripteride.gui.custom.editor;

import java.awt.datatransfer.StringSelection;
import java.awt.datatransfer.Transferable;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JComponent;
import javax.swing.JList;
import javax.swing.ListSelectionModel;
import javax.swing.TransferHandler;
import javax.swing.text.BadLocationException;

import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.models.CommandFrame;
import de.ibm.com.opentm2scripteride.utility.ErrorHandler;

/**
 * The CodeCompleterList is a modified JList, which handles Enter/MouseClick
 * events, inserts the selected value into the text area and supports Drag and
 * Drop
 * 
 * @author Tim Reppenhagen 2011/12/14
 * @version 1.0 2011/12/14
 */
@SuppressWarnings({ "serial" })
public class CodeCompleterList extends JList<Object> {

	static Object[] empty = new Object[0];

	/**
	 * Creates a new CodeCompleterList and adds ActionListeners for pressing
	 * Enter and double clicking. Also it adds a TransferHandler to support
	 * dragging commands from the List
	 */
	public CodeCompleterList() {

		setSelectionMode(ListSelectionModel.SINGLE_INTERVAL_SELECTION);
		setLayoutOrientation(JList.VERTICAL);
		setDragEnabled(true);

		// double click to insert command into editor window
		addMouseListener(new MouseAdapter() {

			public void mouseClicked(MouseEvent e) {
				if (e.getClickCount() == 2) {
					try {
						insertListValue();
					} catch (Exception e1) {
						ErrorHandler.getInstance().handleError(0,
								"Could not insert the value to the editor.");
					}
				}
			}
		});
	}

	/**
	 * Inserts the selected value into the text area, deletes the typed word and
	 * deletes the contents in the list
	 */
	public void insertListValue() {
		CommandFrame cmd = (CommandFrame) getSelectedValue();
		insertListValue(cmd.getCommand());
	}

	/**
	 * Inserts the given String into the TextArea
	 * 
	 * @param stringToInsert
	 *            The String that should be inserted
	 */
	public void insertListValue(String stringToInsert) {
		JEditTextArea editor = MainApp.getInstance().getActiveEditor();
		int offset = editor.getCaretPosition();

		// set selection start and selection end
		editor.select(offset, offset);

		// if not at new line insert, insert a newline
		//int line = editor.getDocument().getDefaultRootElement()
		//		.getElementIndex(offset);
		//String lineText = editor.getLineText(line);
		// relOffset==0 means it's at the line start
		//int relOffset = offset
		//		- editor.getDocument().getDefaultRootElement().getElement(line)
		//				.getStartOffset();

        // fix where to insert the reminder command
		// firstly remove around text then insert the reminder command
		int[] beginEnd = editor.wordAroundPosition(offset, "", true);
		if (beginEnd != null) {
			try {
				String aroundText = editor.getText(beginEnd[0], beginEnd[1]-beginEnd[0]).trim();
				if(!aroundText.isEmpty())
				   editor.getDocument().remove(beginEnd[1]-aroundText.length(), aroundText.length());
			} catch (BadLocationException e) {
				e.printStackTrace();
			}
		}
     
		// insert the string
		editor.setSelectedText(stringToInsert);
		MainApp.getInstance().getActiveEditor().requestFocus();
	}

	/**
	 * Inserts the values of "content" to the list
	 * 
	 * @param content
	 *            The values, which should be inserted
	 */
	public void setListContent(Object[] content) {
		this.setListData(content);
	}

	/**
	 * Clear the List
	 */
	public void emptyList() {
		this.setListData(empty);

	}

	/**
	 * Reports if the List is currently empty
	 * 
	 * @return true if the list is empty, false otherwise
	 */
	public boolean isListEmpty() {
		if (getModel().getSize() == 0)
			return true;

		else
			return false;
	}

	/**
	 * Reports if the List is currently filled with all imported commands
	 * 
	 * @return true if the list is full, false otherwise
	 */
	public boolean isListFull() {
		if (getModel().getSize() == CodeCompleter.getInstance()
				.getNumberOfAllCommand())
			return true;

		else
			return false;
	}

	/**
	 * The TransferHandler for the CodeCompleterList
	 * 
	 * @author Tim Reppenhagen
	 * 
	 */
	class ListTransferHandler extends TransferHandler {
		private CommandFrame cmd;
		private String string;

		/**
		 * Returns the supported actions, we only need Copy
		 * 
		 * @return The copy-action constant
		 */
		public int getSourceActions(JComponent comp) {
			// only support copy action
			return COPY;
		}

		/**
		 * Creates a transferable string from the selected CommandFrame
		 * 
		 * @return the selected command frame as string
		 */
		public Transferable createTransferable(JComponent comp) {
			cmd = (CommandFrame) getSelectedValue();
			string = cmd.getCommand();

			return new StringSelection(string);
		}
	}

}
