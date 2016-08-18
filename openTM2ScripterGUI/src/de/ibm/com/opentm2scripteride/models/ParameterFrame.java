/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.models;

import java.io.Serializable;

import org.jdom.Element;


/**
 * This class represents a paramater with all its attributes (type, description, name, optional,..)
 */
public class ParameterFrame implements Serializable {
	
	private static final long serialVersionUID = -5823913709774419071L; //generated

	public enum Type {
		Default, /* Any string, integer, variable, ... */
		Variable, /* A variable */
		Constant, /* A constant */
		UInt, /* An unsigned integer */
		Condition, /* A condition (boolean expression) */
		Action /* Any action */
	}
	
	private String mId;
	private String mName;
	private Type mType;
	private String mDescription;
	private boolean mOptional;
	
	/**
	 * Constructs an object that represents a parameter with a name, a type and a description
	 * @param id The id of the parameter
	 * @param name The readable name of the parameter
	 * @param type The type of the parameter
	 * @param optional Describes if this parameter is optional
	 * @param description The description of the parameter, if null it will be an empty string
	 */
	public ParameterFrame(String id, String name, Type type, boolean optional, String description) {
		mName = (name != null) ? name : "";
		mId = (id != null) ? id : "";
		mType = type;
		mDescription = (description != null) ? description : "";
		mOptional = optional;
	}


	/**
	 * Constructs a CommandParameter from an XML node
	 * @param node The xml node
	 * @return The new CommandParameter object
	 */
	public static ParameterFrame fromXml(Element node) {
		String id = node.getAttributeValue("id");
		String name = node.getAttributeValue("name");
		String desc = node.getAttributeValue("description");
		String t = node.getAttributeValue("type");
		String optional = node.getAttributeValue("optional");
		t = (t == null) ? "" : t.toLowerCase();
		Type type;
		if (t.equals("variable")) {
			type = Type.Variable;
		}
		else if (t.equals("constant")) {
			type = Type.Constant;
		}
		else if (t.equals("uint")) {
			type = Type.UInt;
		}
		else if (t.equals("condition")) {
			type = Type.Condition;
		}
		else if (t.equals("action")) {
			type = Type.Action;
		}
		else {
			type = Type.Default;
		}
		boolean isOptional = (optional != null && optional.equalsIgnoreCase("true"));
		return new ParameterFrame(id, name, type, isOptional, desc);
	}

	/**
	 * Checks if the given string could be used as a value for this parameter (depends on type)
	 * @param text The string to be validated
	 * @return True if it fits to the type, false otherwise
	 */
	public boolean validates(String text) {
		if (
			 (text == null || text.isEmpty()) &&
			 mOptional
		   ) {
			return true;
		}
		switch ( mType ) {
			case Variable:
				return ParameterTypeChecker.checkVariableName(text);
				//break; java would complain if it's not commented out
				
			case Constant:
				return ParameterTypeChecker.checkConstantName(text);
				//break; java would complain if it's not commented out
				
			case UInt:
				return ParameterTypeChecker.checkPositiveIntegerAttr(text);
				//break; java would complain if it's not commented out
				
			case Condition:
				return ParameterTypeChecker.checkConditionIf(text);
				//break; java would complain if it's not commented out
				
			case Action:
				return ParameterTypeChecker.checkForAction(text);
				
			default:
				return ParameterTypeChecker.checkSimpleAttr(text);
				
		}
	}

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
	
	public String getName() {
		return mName;
	}

	public void setName(String name) {
		mName = name;
	}
	
	public boolean isOptional() {
		return mOptional;
	}
	
	public void setOptional(boolean isOptional) {
		mOptional = isOptional;
	}
}
