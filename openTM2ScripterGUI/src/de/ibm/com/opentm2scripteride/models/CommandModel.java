/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.models;

import java.util.ArrayList;
import java.util.List;

/**
 * A class that represents a (partial) tree of the document.
 * It's basically an array. If it has members it means that this CommandModel represents a whole block (for, if, ...).
 * Otherwise it will only contain information about a CommandBody at a specific line in the script
 */
public class CommandModel extends ArrayList<CommandModel> {
	private static final long serialVersionUID = -3625526700046284122L;
	CommandBody mBody;
	int mLine;

	/**
	 * Constructs a model at the given line about the given command without any arguments
	 * @param line The line this command corresponds to
	 * @param frame The command this model corresponds to
	 */
	public CommandModel(int line, CommandFrame frame) {
		mBody = new CommandBody(frame);
		//mLine = 0;
		mLine = line;
	}
	
	/**
	 * Constructs a model at the given line about the command with the given string without any arguments
	 * @param line The line this command corresponds to
	 * @param command The command-string we will search for in the CommandCatalog
	 */
	public CommandModel(int line, String command) {
		init(line, command, new ArrayList<String>());
	}
	
	/**
	 * Constructs a model at the given line about the command with the given string with one argument
	 * @param line The line this command corresponds to
	 * @param command The command-string we will search for in the CommandCatalog
	 * @param arg The one and only argument of this command
	 */
	public CommandModel(int line, String command, String arg) {
		ArrayList<String> args = new ArrayList<String>();
		args.add(arg);
		init(line, command, args);
	}
	
	/**
	 * Constructs a model at the given line about the command with the given string and the given arguments
	 * @param line The line this command corresponds to
	 * @param command The command-string we will search for in the CommandCatalog
	 * @param args The list of arguments corresponding to this command
	 */
	public CommandModel(int line, String command, List<String> args) {
		init(line, command, args);
	}

	/**
	 * The internally used constructor for all visible constructors
	 * @param line The line this command corresponds to
	 * @param command The command-string we will search for in the CommandCatalog
	 * @param args The list of arguments corresponding to this command
	 */
	private void init(int line, String command, List<String> args) {
		args = (args == null) ? new ArrayList<String>() : args;
		mLine = line;
		CommandFrame cf = CommandCatalog.getInstance().getByCommand(command);
		if (cf.getId() == "unknown" && args.size() > 0) {
			args.set(0, command + " " + args.get(0));
		}
		mBody = new CommandBody(cf, args);
	}

	/**
	 * Creates a CommandModel by the command's id instead of it's command-string or frame
	 * @param line The line this command corresponds to
	 * @param id The id of the Command we should look for in the CommandCatalog
	 * @return The new CommandModel object
	 */
	public static CommandModel newById(int line, String id) {
		CommandFrame cf = CommandCatalog.getInstance().get(id);
		return new CommandModel(line, cf);
	}
	
	/**
	 * Returns the corresponding CommandFrame
	 * @return The corresponding CommandFrame
	 */
	public CommandFrame getFrame() {
		return mBody.getFrame();
	}
	
	/**
	 * Returns the corresponding CommandBody object
	 * @return The corresponding CommandBody
	 */
	public CommandBody getBody() {
		return mBody;
	}
	
	/**
	 * Returns the line this command/block starts
	 * @return The line this command/block starts
	 */
	public int getStartLine() {
		return mLine;
	}

	/**
	 * Sets the line this command/block starts
	 * @param line The line this command/block starts
	 */
	public void setStartLine(int line) {
		mLine = line;
	}
	
	/**
	 * Returns the line this command/block ends.
	 * @return The line this command/block ends It's the startLine if it hasn't any children.
	 */
	public int getEndLine() {
		int sz = size();
		int end = (sz > 0) ? get(sz - 1).getEndLine() : mLine;
		if ( !mBody.getFrame().getEnd().isEmpty() ) {
			end++;
		}
		return end;
	}
	
	/**
	 * Searches for the parent object of the given children in this block
	 * @param child The children searching its parent
	 * @return The parent object or null if it wasn't found
	 */
	public CommandModel findParent(CommandModel child) {
		for (CommandModel cur : this) {
			if (cur == child) {
				return this;
			}
			CommandModel found = cur.findParent(child);
			if (found != null) {
				return found;
			}
		}
		return null;
	}
	
	/**
	 * Finds the object corresponding to the given line if it's part of this block or the block itself if it's starting here
	 * @param line The line the block/object should correspond to
	 * @param ignoreWrapper If true and the object has a child with the same line number, the child will be return. Otherwise this object directly
	 * @return The object corresponding to the given line number or null if we didn't find any.
	 */
	public CommandModel findByStartingLine(int line, boolean ignoreWrapper) {
		if (mLine == line) {
			if (!ignoreWrapper || size() < 1) {
				return this;
			}
			CommandModel first = get(0);
			first = first.findByStartingLine(line, ignoreWrapper);
			return (first == null) ? this : first;
		}
		for (CommandModel cur : this ) {
			CommandModel find = cur.findByStartingLine(line, ignoreWrapper);
			if (find != null) {
				return find;
			}
		}
		return null;
	}
	
	/**
	 * Checks if this model validates. The children are ignored.
	 * @return True if it validates, false otherwise.
	 */
	public boolean validates() {
		return validates(false);
	}
	
	/**
	 * Checks if this model validates
	 * @param recursive If true it's regarded whether the children also validate
	 * @return True if it's valid, false otherwise
	 */
	public boolean validates(boolean recursive) {
		boolean recVal = true;
		if (recursive) {
			recVal = childrenValidate();
		}
		return recVal && mBody.validates();
	}
	
	/**
	 * Checks if all children validate and ignore if the model itself is valid
	 * @return True if all children validate, false otherwise
	 */
	public boolean childrenValidate() {
		for (CommandModel cur : this) {
			if (!cur.validates(true)) {
				return false;
			}
		}
		return true;
	}
	
	/**
	 * Adds a CommandModel as a child of this one.
	 * @param el The child to be added. If it's null it won't be added.
	 * @return True on success, false otherwise (e.g. it was null)
	 */
	public boolean add(CommandModel el) {
		if (el == null) {
			return false;
		}
		return super.add(el);
	}
	
	/**
	 * Adds a CommandModel as a child of this one at the specified position.
	 * All following children will be changed as their corresponding lines will be incremented
	 * @param index The index the model should be added at
	 * @param element The child to be added. If it's null it won't be added.
	 */	
	public void add(int index, CommandModel element) {
		//check if null, we don't need null!
		if (element == null) {
			return;
		}
		//first add it
		super.add(index, element);
		int lines = element.getEndLine() - element.getStartLine() + 1;		
		//now update line numbers!
		for (int i = index + 1; i < size(); i++) {
			CommandModel cur = get(i);
			cur.incrementStartLines(lines);
		}
	}
	
	/**
	 * Increments recursively all corresponding lines by the given value
	 * @param inc The value to increment this and all children
	 */
	public void incrementStartLines(int inc) {
		mLine += inc;
		for (CommandModel cur : this ) {
			cur.incrementStartLines(inc);
		}
	}
	
	/**
	 * Represents this model (with all children) in text as used in a script
	 * @return The string representing this model
	 */
	public String toText() {
		return toText(0);
	}
	
	/**
	 * Represents this model (with all children) in text as used in a script
	 * @param indent The indention level used (1 equals two spaces)
	 * @return The string representing this model
	 */
	public String toText(int indent) {
		StringBuilder sb = new StringBuilder();
		String body = mBody.toText();
		if (!body.isEmpty()) {
			for (int i = 0; i < indent; i++)
				sb.append("  ");
			sb.append(body);
		}
		for (CommandModel cm : this) {
			sb.append(cm.toText(indent + 1));
			sb.append("\n");
		}
		return sb.toString();
	}
	
	/**
	 * Represents this model in text as used in a script neglecting all children
	 * @return The string representing this model wothout any children
	 */
	public String toString() {
		return mBody.toString();
	}
	
	/**
	 * Used to check if another model is completely equal to this one
	 */
	public boolean equals(Object o) {
		if (!(o instanceof CommandModel)) {
			return false;
		}
		CommandModel cm = (CommandModel) o;
		if (cm.getStartLine() != mLine) {
			return false;
		}
		if (cm.getEndLine() != getEndLine()) {
			return false;
		}
		if (cm.size() != size()) {
			return false;
		}
		if (mBody.compareTo(cm.getBody()) != 0) {
			return false;
		}
		return true;
	}

}
