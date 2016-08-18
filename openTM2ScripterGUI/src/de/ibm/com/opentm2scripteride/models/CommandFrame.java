/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.models;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.jdom.Element;


public class CommandFrame implements Comparable<CommandFrame>, Serializable {
	
	private static final long serialVersionUID = -1289323992578396418L; //generated

	public enum Type {
		Function,
		Block,
		ApiCall,
		Wrapper,
		Unknown
	}
	
	protected String mId;
	protected List<ParameterFrame> mParameters;
	protected Type mType;
	
	protected String mName;	
	protected String mCommand;
	protected String mEnd;
	protected String mDescription;

	/**
	 * Constructs a ScriptCommand object with the passed fields
	 * @param name The unique name of the Script, used as identifier
	 * @param parameters The List of parameters
	 * @param type The type of the command (Block or Function)
	 * @param command The command 
	 * @param end The end of the command (optional)
	 * @param description The description of the command
	 */
	public CommandFrame(String id, String name, List<ParameterFrame> parameters,
			Type type, String command, String end, String description) {
		mId = id;
		mName = (name == null) ? mId : name;
		mParameters = (parameters == null) ? new ArrayList<ParameterFrame>() : parameters ;
		mType = type;
		mCommand = (command == null) ? "" : command;
		mEnd = (end == null) ? "" : end;
		mDescription = (description == null) ? "" : description;
	}
	
	/**
	 * Parses a CommandFrame from a given XML node
	 * @param node The node to parse from
	 * @param type The type this Command has
	 * @return The CommandFrame parsed from the given node or null if it failed
	 */
	public static CommandFrame fromXml(Element node, Type type) {
		String id = node.getAttributeValue("id");
		if (id.isEmpty()) {
			return null;
		}
		String name = node.getAttributeValue("name");
		String command = node.getAttributeValue("command");
		String end = node.getAttributeValue("endcommand");
		String description = node.getAttributeValue("description");
		List<ParameterFrame> paras = CommandFrame.parseParametersFromXml(node);
		return new CommandFrame(id, name, paras,type, command, end, description);
	}
	
	/**
	 * Parses parameters from an XML node
	 * @param node The XML node that might has "parameter"-children
	 * @return A list of the parameters
	 */
	@SuppressWarnings("unchecked")
	public static List<ParameterFrame> parseParametersFromXml(Element node) {
    	//the JDOM API seems to be old as it just returns a List though it's a list of Elements
		//so we suppress the warning for the operation in the loop
		
		ArrayList<ParameterFrame> paras = new ArrayList<ParameterFrame>();
		for (Element el : (List<Element>) node.getChildren("parameter")) {
			ParameterFrame p = ParameterFrame.fromXml(el);
			if (p != null)
				paras.add(p);
		}
		return paras;
	}

	/**
	 * Checks if another CommandFrame is equal to this one
	 */
	public int compareTo(CommandFrame b) {
		int r = mName.compareTo(b.getName());
		if (r != 0) {
			return r;
		}
		Integer sizeA = mParameters.size();
		return sizeA.compareTo(b.getParameters().size());
	}
	
	/**
	 * Simply the name of the Command this frame represents
	 */
	public String toString() {
		return mName;
	}

	
	
	/* ONLY Getters and Setters here */


	public String getId() {
		return mId;
	}

	public void setId(String id) {
		mId = id;
	}	
	
	public String getName() {
		return mName;
	}

	public void setName(String name) {
		mName = name;
	}

	public List<ParameterFrame> getParameters() {
		return mParameters;
	}

	public void setParameters(List<ParameterFrame> parameters) {
		mParameters = parameters;
	}

	public String getCommand() {
		return mCommand;
	}

	public void setCommand(String command) {
		mCommand = command;
	}

	public String getEnd() {
		return mEnd;
	}

	public void setEnd(String end) {
		mEnd = end;
	}

	public String getDescription() {
		return mDescription;
	}

	public void setDescription(String description) {
		mDescription = description;
	}

	public Type getType() {
		return mType;
	}

	public void setType(Type type) {
		mType = type;
	}

}
