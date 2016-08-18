/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom;

import java.awt.datatransfer.DataFlavor;
import javax.swing.TransferHandler;
import javax.swing.tree.TreePath;
import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.gui.custom.editor.JEditTextArea;
import de.ibm.com.opentm2scripteride.models.CommandModel;

/**
 * The Transfer Handler for the Outline Tree.
 * Needed for DnD operations. In this case only dropping is supported,
 *
 */
@SuppressWarnings("serial")
public class ScriptOutlineTransferHandler extends TransferHandler {
    protected int action;
    protected ScriptOutlineTree outlineTree;
    
    public ScriptOutlineTransferHandler(ScriptOutlineTree tree, int action) {
        this.action = action;
        outlineTree = tree;
    }
    
    /**
     * Check what kind of data and DnD operation is supported. In this case: only strings // dropping
     */
    public boolean canImport(TransferHandler.TransferSupport support) {
    	//accept strings    
        if (support.isDataFlavorSupported(DataFlavor.stringFlavor)) 
        {
            return true;
        }
       
        //only support dropping
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
     * Check via "canImport" if the action/data is supported, then import the data.
     * First get the commandModel from the dropped string, then add it to the OutlineTree
     * and then import the command to the editor
     */
    public boolean importData(TransferHandler.TransferSupport support) {
        
    	// try if the drop-action is supported
        // get string, transfer to a command model and add it to the tree
        if (!canImport(support) || !support.isDataFlavorSupported(DataFlavor.stringFlavor)) {
        	return false;
        }
        String droppedString;
        CommandModel insertableModel;
        CommandModel ancestorModel;
        Object o;
        TreePath ancestorPath;
       
    	try {
    		droppedString = (String)support.getTransferable().getTransferData(DataFlavor.stringFlavor);
    	} catch (Exception e) {
    		return false;
    	}
    	
    	//get the selected command
    	ancestorPath= outlineTree.getPathForLocation(outlineTree.getMousePosition().x, outlineTree.getMousePosition().y);
    	o = ancestorPath.getLastPathComponent();

    	if (!(o instanceof CommandModel)) {
    		return false; //shouldn't happen
    	}
    	ancestorModel= (CommandModel) o;
    	//get the command, which should be inserted
    	insertableModel= new CommandModel(ancestorModel.getEndLine() + 1, droppedString);
        
    	//add the new command after the selected command
    	outlineTree.getModel().addAfter(ancestorModel, insertableModel);
    	
    	//add to the editor
    	//EditorConnector connector = EditorConnector.getInstance();
    	JEditTextArea editor = MainApp.getInstance().getActiveEditor();
    	if (editor.getLineCount() <= insertableModel.getStartLine()) {
    		editor.setCaretPosition(editor.getDocumentLength());
    		editor.setSelectedText("\n");
    	}
    	editor.setCaretPosition(insertableModel.getStartLine(), 0);
    	editor.setSelectedText(droppedString+"\n");
    	editor.repaint();
    	outlineTree.repaint();
    	return true;
    }  
} 
