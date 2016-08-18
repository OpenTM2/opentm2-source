/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
 */
package de.ibm.com.opentm2scripteride.gui.custom.editor;

import java.util.List;

import javax.swing.ListModel;

import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.models.CommandCatalog;
import de.ibm.com.opentm2scripteride.models.CommandFrame;

/**
 * The code-completion backend class Merges chars to a string, checks them if
 * the length is >2 and sets the found words to the CodeCompleterList
 * 
 * @author Tim Reppenhagen 2011/12/14
 * @version 1.0 2011/12/14
 * 
 */
public class CodeCompleter {
	private static CodeCompleter instance = null;

	private Object[] allCommands;

	private CodeCompleter() {
		allCommands = (Object[]) CommandCatalog.getInstance().toList()
				.toArray();
	}


	/**
	 * Returns how many commands are currently imported
	 * 
	 * @return The number of all commands imported
	 */
	public int getNumberOfAllCommand() {
		return allCommands.length;
	}

	public void setSearch(String mask) {
		setFilter(mask, true);
	}

	public void setFilter(String filter, boolean masksearch) {
		if (filter == null || filter.isEmpty()) {
			//allcommands();
			MainApp.getInstance().getMainWindow().getCodeCompleteList().setListContent(CodeCompleter.getInstance().getAllCommands());
			return;
		}
		List<CommandFrame> found = null;
		if (masksearch) { // search
			found = CommandCatalog.getInstance().searchCommand(filter);
		} else { // just the words beginning with
			found = CommandCatalog.getInstance().commandsStartingWith(filter);
		}
		//EditorConnector.getInstance().setListContent(found.toArray());
		MainApp.getInstance().getMainWindow().getCodeCompleteList().setListContent(found.toArray());
	}

	/**
	 * Returns the first suggestions out of the ArrayList
	 * 
	 * @return the first suggestion
	 */
	public String getFirstSuggestion() {
		int idx = MainApp.getInstance().getMainWindow().getCodeCompleteList().getFirstVisibleIndex();
		ListModel<?> model = MainApp.getInstance().getMainWindow().getCodeCompleteList().getModel();
		if (idx < 0 || idx > model.getSize() - 1) {
			return null;
		}
		Object value = model.getElementAt(idx);
		if (!(value instanceof CommandFrame)) {
			return null;
		}
		return ((CommandFrame) value).getCommand();
	}

	public Object[] getAllCommands() {
		return allCommands;
	}

	/**********************************************************/

	public static CodeCompleter getInstance() {
		if (instance == null) {
			instance = new CodeCompleter();
		}
		return instance;
	}
}