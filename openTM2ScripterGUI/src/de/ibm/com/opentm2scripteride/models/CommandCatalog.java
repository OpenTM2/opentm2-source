/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.models;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeMap;
import java.util.regex.Pattern;

import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.xml.sax.InputSource;
import org.jdom.Document;
import org.jdom.Element;

import de.ibm.com.opentm2scripteride.utility.Configuration;
import de.ibm.com.opentm2scripteride.utility.Constants;
import de.ibm.com.opentm2scripteride.utility.ErrorHandler;

/**
 * A catalog that provides all available commands
 */
public class CommandCatalog extends TreeMap<String, CommandFrame> {
	
	private static final long serialVersionUID = 2289102627735667221L; //generated
	private static CommandCatalog mInstance = null;

	/**
	 * Constructs the catalog
	 */
	private CommandCatalog() {
		super();
		List<ParameterFrame> list = new ArrayList<ParameterFrame>();
		list.add(new ParameterFrame("text", "Text", ParameterFrame.Type.Default, true, ""));
		
		//add a "unknown" spec by default
		CommandFrame unknown = new CommandFrame("unknown", "Unknown", list, CommandFrame.Type.Unknown, null, null, null);
		put("unknown", unknown);
	}
	
	/**
	 * Used to access the catalog. As it's global and should exist only once, this method should be used 
	 * @return The CommandCatalog object
	 */
	public static CommandCatalog getInstance() {
		if (mInstance == null) {
			Configuration conf = Configuration.getInstance();
			mInstance = CommandCatalog.fromXmlFile(conf.getValue(Constants.commandXMLLocation));
		}
		return mInstance;
	}
	
	/**
	 * Constructs an object by parsing an XML file
	 * @param filename The filename of the XML file to be parsed
	 * @return The newly created CommandCatalog object
	 */
	public static CommandCatalog fromXmlFile(String filename)
	{
    	SAXBuilder sxbuild = new SAXBuilder();
    	InputSource is = new InputSource(filename);
    	Document doc;
    	try {
			doc = sxbuild.build(is);
		} catch (JDOMException e) {
			ErrorHandler.getInstance().logErrorDetails(e);
			return null;
		} catch (IOException e) {
			ErrorHandler.getInstance().logErrorDetails(e);
			return null;
		}
    	Element root = doc.getRootElement();
    	CommandCatalog cat = new CommandCatalog();
    	cat.fromXml(root.getChild("wrapper"), CommandFrame.Type.Wrapper);
    	cat.fromXml(root.getChild("blocks"), CommandFrame.Type.Block);
    	cat.fromXml(root.getChild("api"), CommandFrame.Type.ApiCall);
    	cat.fromXml(root.getChild("functions"), CommandFrame.Type.Function);
		return cat;
	}
	
	/**
	 * Extends the map with commands from the XML node
	 * @param node The XML node with "command" children
	 * @param type The type of the command (Block or Function)
	 */
	@SuppressWarnings("unchecked")
	public void fromXml(Element node, CommandFrame.Type type) {
		//the JDOM API seems to be old as it just returns a List though it's a list of Elements
		//so we suppress the warning for the operation in the loop
    	for ( Element curCmd : (List<Element>) node.getChildren("command")) {
    		CommandFrame cmd = CommandFrame.fromXml(curCmd, type);
    		if (cmd != null)
    			put(cmd.getId(), cmd);
    	}
	}
	
	/**
	 * Return a CommandFrame which's command equals the given string or an "unknown" CommandFrame
	 * It will be ignored if the characters are upper or lower case
	 * @param cmd The command string
	 * @return The CommandFrame which's command equals the given string or an "unknown" CommandFrame
	 */
	public CommandFrame getByCommand(String cmd) {
		for (CommandFrame c : values()) {
			if (c.getCommand().equalsIgnoreCase(cmd)) {
				return c;
			}
		}
		return get("unknown");
	}
	
	/**
	 * Returns a list of CommandFrames of commands beginning with a specific string
	 * It will be ignored if the characters are upper or lower case
	 * @param start The start of the command
	 * @return The List of CommandFrames which have commands beginning with the given string
	 */
	public List<CommandFrame> commandsStartingWith(String start) {
		ArrayList<CommandFrame> cmdList = new ArrayList<CommandFrame>();
		for (CommandFrame cmd : values()) {
			if (cmd.getCommand().toLowerCase().startsWith(start.toLowerCase())) {
				cmdList.add(cmd);
			}
		}
		return cmdList;
	}
	
	/**
	 * Returns a list of CommandFrames with commands of the specified type
	 * @param type The type the commands should have
	 * @return The List of CommandFrames with the given type
	 */
	public List<CommandFrame> commandsByType(CommandFrame.Type type) {
		ArrayList<CommandFrame> cmdList = new ArrayList<CommandFrame>();
		for (CommandFrame cmd : values()) {
			if (cmd.getType() == type) {
				cmdList.add(cmd);
			}
		}
		return cmdList;
	}
	
	/**
	 * Returns a list of all CommandFrames 
	 * @return The List of all CommandFrames
	 */
	public List<CommandFrame> toList() {
		ArrayList<CommandFrame> cmdList = new ArrayList<CommandFrame>();
		for (CommandFrame cmd : values()) {
				cmdList.add(cmd);
		}
		return cmdList;
	}
	
	/**
	 * Searches a list for CommandFrames
	 * @param mask The search mask, * can be used placeholder
	 * @return List of the found CommandFrames
	 */
	public List<CommandFrame> searchCommand(String mask) {
		StringBuilder newmask = new StringBuilder(".*");
		newmask.append(mask.replace("*", ".*"));
		newmask.append(".*");
		Pattern pattern = Pattern.compile(newmask.toString(), Pattern.CASE_INSENSITIVE);
		ArrayList<CommandFrame> cmdList = new ArrayList<CommandFrame>();
		for (CommandFrame cmd : values()) {
			if (pattern.matcher(cmd.getCommand()).matches()) {
				cmdList.add(cmd);
			}
		}
		return cmdList;
	}
}
