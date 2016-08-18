/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.models;

import java.io.Serializable;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeMap;

/**
 * This class represents an "instance" of a command including parameters
 */
public class CommandBody implements Comparable<CommandBody>, Serializable {

	private static final long serialVersionUID = -4316272678890313103L; //generated
	
	CommandFrame mFrame;
	Map<String, String> mParameters;

	/**
	 * Create it for the given command with the mapped parameters
	 * @param frame The Command it corresponds to ("is an instance of")
	 * @param parameters The parameters this "instance" has
	 */
	public CommandBody(CommandFrame frame, Map<String, String> parameters) {
		init(frame, parameters);
	}
	
	/**
	 * Create it for the given command with a list of parameters
	 * @param frame The Command it corresponds to ("is an instance of") 
	 * @param parameters The parameters in the order they'd be written
	 */
	public CommandBody(CommandFrame frame, List<String> parameters) {
		frame = validFrameObject(frame);
		Map<String, String> tMap = new TreeMap<String, String>();
		List <ParameterFrame> existingParams = frame.getParameters();
		//create the parameter map of the list
		int paramNum = existingParams.size();
		int paramAvailable = parameters.size();
		int max = paramAvailable < paramNum ? paramAvailable : paramNum; 
		for (int i = 0; i < max; i++) {
			ParameterFrame pf = existingParams.get(i);
			tMap.put(pf.getId(), parameters.get(i));
		}
		if ( paramAvailable > paramNum ) { //save additional parameters nevertheless
			for (int i = 0; i < paramAvailable - paramNum; i++) {
				String id = "_p_unknown_" + i;
				tMap.put(id, parameters.get(paramNum + i));
			}
		}
		init(frame, tMap);
	}
	
	/**
	 * Creates an empty "instance" of the Command
	 * @param frame The Command it corresponds to ("is an instance of")
	 */
	public CommandBody(CommandFrame frame) {
		init(frame, null);
	}
	
	/**
	 * Makes sure we have a valid frame and sets parameters
	 * @param frame The Command it corresponds to ("is an instance of")
	 * @param parameters The parameters for this "instance"
	 */
	private void init(CommandFrame frame, Map<String, String> parameters) {
		mFrame = validFrameObject(frame);;
		mParameters = ( parameters == null ) ? new TreeMap<String, String>() : parameters;
	}
	
	/**
	 * Returns an HTML string that represents the command and the assigned parameters with their name
	 * @return The HTML string that represents the command and the assigned parameters with their name
	 */
	public String toHTML() {
		if (mFrame == null) {
			return "";
		}
		if (mFrame.getId() == "unknown") {
			String text = mParameters.get("text");
			return (text == null) ? "" : text;
		}
		StringBuilder sb = new StringBuilder();
		sb.append("<html>");
		//title: the command name
		sb.append("<b>");
		sb.append(mFrame.getName());
		sb.append("</b>");
		for (ParameterFrame f : mFrame.getParameters()) {
			sb.append("<br><i>");
			sb.append(f.getName());
			sb.append("</i>: ");
			sb.append(mParameters.get(f.getId()));
		}
		sb.append("</html>");
		return sb.toString();
	}
	
	/**
	 * Returns a string that represents this instance as a command line in the form of
	 * "COMMAND [ARG1] [, ARG2, ...]"
	 * @return This instance as a command line
	 */
	public String toText() {
		if (mFrame == null) {
			return "";
		}
		//"Unknown" commands are simply printed
		if (mFrame.getId() == "unknown") {
			String text = mParameters.get("text");
			return (text == null) ? "" : text;
		}
		//usual commands are created
		StringBuilder sb = new StringBuilder(mFrame.getCommand());
		//append all parameters
		List<ParameterFrame> paras = mFrame.getParameters();
		if (paras.size() > 0) {
			sb.append(" ");
			sb.append(mParameters.get(paras.get(0).getId()));
			for (int i = 1; i < paras.size(); i++) {
				sb.append(", ");
				sb.append(mParameters.get(paras.get(i).getId()));
			}
		}
		//check if we have an end
		String end = mFrame.getEnd();
		if (!end.isEmpty()) {
			sb.append("\n");
			sb.append(end);
		}

		return sb.toString();
	}
	
	/**
	 * The name of the command. If it's "unknown" we guess it.
	 * @return The name of the command
	 */
	public String toString() {
		if (mFrame.getId() == "unknown") {
			String text = mParameters.get("text");
			return guessUnknownCommand(text);
		}
		return mFrame.getName();
	}

	/**
	 * Compares if this instance is equal to another one (same command, same assigned parameters)
	 */
	public int compareTo(CommandBody b) {
		//check the command itself first
		int c = mFrame.compareTo(b.getFrame());
		if ( c != 0 ) {
			return c;
		}
		//then check the number of parameters
		Map<String, String> bParas = b.getParameters();
		if ( mParameters.size() < bParas.size() ) {
			return -1;
		}
		else if ( mParameters.size() > bParas.size() ) {
			return 1;
		}
		//then check each parameter
		Set<Entry<String, String>> set = mParameters.entrySet();
		for ( Entry<String, String> e : set ) {
			String key = e.getKey();
			if (!bParas.containsKey(key)) {
				return 1;
			}
			String valString = bParas.get(key);
			if (valString == null) {
				return 1;
			}
			String curVal = e.getValue();
			if (curVal == null) {
				return -1;
			}
			c = curVal.compareTo(valString);
			if ( c != 0 ) {
				return c;
			}
		}
		
		return 0;
	}

	/**
	 * Checks if the body is valid, which means that all parameters validate
	 * @return True if all parameters are valid, false otherwise
	 */
	public boolean validates() {
		//chek if unknown, unknown never validates!
		if (mFrame.getType() == CommandFrame.Type.Unknown) {
			return false;
		}
		//simply check if all parameters validate :)
		List<ParameterFrame> availParas = mFrame.getParameters();
		for (ParameterFrame param : availParas) {
			String id = param.getId();
			if (!mParameters.containsKey(id) && !param.isOptional()) {
				return false;
			}
			if (!param.validates(getParameter(param.getId()))) {
				return false;
			}
		}
		return mParameters.size() <= availParas.size();
	}
	
	/**
	 * Returns a map of all parameters that are assigned
	 * @return A map of all parameters that are assigned
	 */
	public Map<String, String> getParameters() {
		return mParameters;
	}
	
	/**
	 * Returns the assigned parameter for the given id
	 * @param id The id we want to have the parameter for
	 * @return The assigned value or null if it wasn't assigned
	 */
	public String getParameter(String id)
	{
		String s = mParameters.get(id);
		return (s == null) ? "" : s;
	}
	
	public String getParameterSkipQuote(String id)
	{
		String res = getParameter(id);
		if(!res.isEmpty())
		{
			if(res.startsWith("\"") && res.endsWith("\""))
				return res.substring(1, res.length()-1);
		}
		return res;
	}
	
	/**
	 * Assigns a parameter for this "instance"
	 * @param id The id of the parameter to be assigned 
	 * @param value The value to be assigned
	 */
	public void setParameter(String id, String value)
	{
		mParameters.put(id, value);
	}
	
	/**
	 * Return the command this object is an "instance" of
	 * @return The command this object is an "instance" of
	 */
	public CommandFrame getFrame() {
		return mFrame;
	}
	
	/**
	 * Checks if the given object is a valid command frame
	 * @param cf The command frame
	 * @return The command frame given if it's valid or an "unknown" command frame
	 */
	protected CommandFrame validFrameObject(CommandFrame cf) {
		if (cf != null) {
			return cf;
		}
		return CommandCatalog.getInstance().get("unknown");
	}

	/**
	 * Guesses the command of a string
	 * @param text The text it should guess the command of
	 * @return Either the first word of the string or "Unknown"
	 */
	protected String guessUnknownCommand(String text) {
		if(text == null || text.isEmpty()){
			return "Unknown";
		}
		String t = text.trim();
		int space = t.indexOf(' ');
		int tab = t.indexOf('\t');
		int sep = space;
		if (tab > 0) {
			if (space > 0) {
				sep = Math.min(space, tab);
			} else {
				sep = tab;
			}
		}
		if (sep < 0) {
			sep = t.length();
		}
		return (sep < 0 || sep > 29) ? "Unknown" : text.substring(0, sep);
	}
}
