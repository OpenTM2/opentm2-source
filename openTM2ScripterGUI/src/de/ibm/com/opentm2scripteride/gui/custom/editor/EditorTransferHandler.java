/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom.editor;

import java.awt.datatransfer.DataFlavor;
import java.util.Scanner;

import javax.swing.TransferHandler;
import javax.swing.text.Element;

import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.gui.custom.FileTree;

/**
 * The editor transfer handler is used for DnD Operations for the editor
 * supports: Dropping of Strings/Files/JTreeElements from the FileTree
 *
 */
@SuppressWarnings("serial")
public class EditorTransferHandler extends TransferHandler {
    int action;
    final DataFlavor fileTreeFlavor=FileTree.getFlavor();
    
    public EditorTransferHandler(int action) {
        this.action = action;
    }
    
    /**
     * Checks if the Object can be dropped
     * Supports Files/Folders and Strings
     */
    public boolean canImport(TransferHandler.TransferSupport support) {
    	
    	//accept files
        if (support.isDataFlavorSupported(DataFlavor.javaFileListFlavor))
        {	setCaretPosition();
        	return true;
        }
               
        //accept strings    
        if (support.isDataFlavorSupported(DataFlavor.stringFlavor)) 
        {	setCaretPosition();
            return true;
        }
       
        //accept JTree Elements
        if(support.isDataFlavorSupported(fileTreeFlavor))
        {   setCaretPosition();
        	return true;
        }
        
        boolean actionSupported = (action & support.getSourceDropActions()) == action;
        if (actionSupported) 
        {
            support.setDropAction(action);
            return true;
        }
        
        //if the element isn't one of the above, don't accept
        return false;
    }

    /**
     * Handles the imported Objects
     * Strings are inserted and Files/Folders are added to a INCLUDE command
     */
    public boolean importData(TransferHandler.TransferSupport support) {
        
    	// try if the drop-action is support
        if (canImport(support)==false) 
        {
            return false;
        }

        // handle dropped Strings
        if(support.isDataFlavorSupported(DataFlavor.stringFlavor))
        {
        String data;
        	try 
        	{
        		data = (String)support.getTransferable().getTransferData(DataFlavor.stringFlavor);
        	} catch (Exception e) {return false;}
        
        	checkInsertNewline();
        	/*EditorConnector.getInstance().editor.*/ MainApp.getInstance().getActiveEditor().setSelectedText(data);
        	/*EditorConnector.getInstance().editor*/  MainApp.getInstance().getActiveEditor().repaint();
        	return true;
        }
        
        //handle dropped files/folders
        if(support.isDataFlavorSupported(DataFlavor.javaFileListFlavor))
        {	
        
        Object file;
        StringBuffer buffer= new StringBuffer();
        Scanner scanner = null;
        
        	try 
        	{
			file= support.getTransferable().getTransferData(DataFlavor.javaFileListFlavor);
			} catch (Exception e1) {return false;} 
			
			buffer.append(file.toString());
			//delete the brackets at the beginning and the end, replace the first bracket with a space 
			buffer.replace(0, 1, " ");
			buffer.delete(buffer.length()-1, buffer.length());
			
			//use the scanner to split up more than one file - delimited by ,
			scanner= extracted(buffer).useDelimiter(",");
			while(scanner.hasNext())
			{	
				//delete the space at the beginning of the string
				buffer.delete(0, buffer.length());
				buffer.append(scanner.next());
				buffer.delete(0, 1);
				
			
				//add to the editor
				checkInsertNewline();
				MainApp.getInstance().getActiveEditor().setSelectedText("INCLUDE '"+buffer.toString()+"'");
				MainApp.getInstance().getActiveEditor().repaint();
			}
			scanner.close();
		    return true;
        }
        
        //handle Files/Folders from the JTree
        if(support.isDataFlavorSupported(fileTreeFlavor))
        {	
        
        Object file;
        StringBuffer buffer= new StringBuffer();
        //Scanner scanner;
        
        	try 
        	{
			file= support.getTransferable().getTransferData(DataFlavor.javaFileListFlavor);
			} catch (Exception e1) {return false;} 
			
			buffer.append(file.toString());
			//delete the brackets at the beginning and the end, replace the first bracket with a space 
			buffer.replace(0, 1, " ");
			buffer.delete(buffer.length()-1, buffer.length());
			
			//use the scanner to split up more than one file - delimited by ,
		//	scanner= new Scanner(buffer.toString()).useDelimiter(",");
			//while(scanner.hasNext())
			/*{	
				//delete the space at the beginning of the string
				buffer.delete(0, buffer.length());
				buffer.append(scanner.next());
				buffer.delete(0, 1);
				
			*/
				//add the include statement to the editor
				checkInsertNewline();
				MainApp.getInstance().getActiveEditor().setSelectedText("INCLUDE '"+buffer.toString()+"'");
				MainApp.getInstance().getActiveEditor().repaint();
			//}
		    return true;
        }
        
        else return false;
    }

	private Scanner extracted(StringBuffer buffer) {
		return new Scanner(buffer.toString());
	}  
 
    /**
     * Changes the caret position to the mouse pointer
     */
    public void setCaretPosition()
    {
        int caretPosition;
        JEditTextArea editor = MainApp.getInstance().getActiveEditor();
        caretPosition=editor.xyToOffset(editor.getMousePosition().x, editor.getMousePosition().y);
		editor.setCaretPosition(caretPosition);

    }
    
    protected void checkInsertNewline() {
    	JEditTextArea editor = MainApp.getInstance().getActiveEditor();
    	int line = editor.getCaretLine();
    	Element el = editor.getDocument().getDefaultRootElement().getElement(line);
    	
        // wlp add, the second parameter is the length
    	String lineContent = editor.getText(el.getStartOffset(), el.getEndOffset()-el.getStartOffset());
    	
    	if (!lineContent.trim().equals("")) {
    		editor.setSelectedText("\n");
    	}
	}
} 