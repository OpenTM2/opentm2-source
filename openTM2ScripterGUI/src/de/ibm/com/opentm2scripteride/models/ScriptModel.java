/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.models;

import java.util.ArrayList;

import javax.swing.event.EventListenerList;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

/**
 * This class is the "root" CommandModel that represents a complete script.
 * In contrast to a normal CommandModel it has some extra functionality and implements a TreeModel.
 * So if you want to have an outline of the script you can simply use this object as a model for a JTree
 */
public class ScriptModel extends CommandModel implements TreeModel {
	private static final long serialVersionUID = 4496745711594302424L;
    protected EventListenerList mListeners;
    protected String mName;

    /**
     * The constructor to create a new ScriptModel representing a new script
     */
	public ScriptModel() {
		super(0, CommandCatalog.getInstance().get("unknown"));
		mListeners = new EventListenerList();
		mName = "";
	}
	
	public boolean add(CommandModel cm) {
		boolean ret = super.add(cm);
		fireTreeStructureChanged(new TreePath(this));
		return ret;
	}

	/**
	 * Adds a model after the specified ancestor which might be any child, grand-child, etc
	 * @param ancestor The direct ancestor in the sense of "line before in the script", not parent!
	 * @param insertable The model to be inserted after the ancestor
	 * @return True on success, false otherwise
	 */
	public boolean addAfter(CommandModel ancestor, CommandModel insertable) {
		CommandModel parent = findParent(ancestor);
		if (parent == null) {
			return false;
		}
		int idx = parent.indexOf(ancestor);
		parent.add(idx + 1, insertable);
		fireTreeStructureChanged(getTreePath(parent));
		return true;
	}
	
	/**
	 * Sets the name of the script the model represents
	 * @param name The name of the script
	 */
	public void setName(String name) {
		mName = name;
	}
	
	/**
	 * Returns the name of the script the model represents
	 * @return The name of the script the model represents
	 */
	public String getName() {
		return mName;
	}

	/**
	 * Returns either it's name if set or "Script"
	 * @return Either it's name if set or "Script"
	 */
	public String toString() {
		if (mName.isEmpty()) {
			return "Script";
		}
		return mName;
	}
	
	/**
	 * Checks if the complete script validates (so recursively!)
	 * @return True if the whole script validates, false otherwise.
	 */
	public boolean validates() {
		return validates(true);
	}

	/**
	 * Checks if the script validates
	 * @param recursive If true it checks if the whole script validates. Otherwise it returns true!
	 * @return True if the whole script validates and recursive is true. Otherwise always true
	 */
	public boolean validates(boolean recursive) {
		return recursive ? childrenValidate() : true;

	}
	/*------------------------------------*
	 * Implemented functions of TreeModel *
	 *------------------------------------*/
	/**
	 * Returns the tree parse for the given child, or a path to the root if the child is null
	 * @param el The child we need a TreePath for
	 * @return The TreePath to the seeked model
	 */
	protected TreePath getTreePath(CommandModel el) {
		if (el == null || el == this) {
			return new TreePath(this);
		}
		ArrayList<Object> path = new ArrayList<Object>();
		path.add(el);
		for (CommandModel cur = el; cur != this; cur = findParent(cur)) {
			if (cur == null) {
				return new TreePath(this);
			}
			path.add(el);
		}
		int count = path.size();
		Object[] components = new Object[count];
		for (int i = count; i > 0; i--) {
			components[count - i] = path.get(i - 1);
		}
		return new TreePath(components);
	}
	
    /** 
     * Call when the tree structure below the path has completely changed
     * @param parentPath The path where the changes happend
     */
    protected void fireTreeStructureChanged(TreePath parentPath)
    {
        Object[] pairs = mListeners.getListenerList();
        
        TreeModelEvent e = null;
        
        for (int i = pairs.length - 2; i >= 0; i -= 2)
        {
            if (pairs[i] == TreeModelListener.class)
            {
                if (e == null)
                    e = new TreeModelEvent(this, parentPath, null, null);
                
                ((TreeModelListener)pairs[i + 1]).treeStructureChanged(e);
            }
        }
     }
    

	public Object getChild(Object parent, int idx) {
		//make sure this child exists
		int childcount = getChildCount(parent);
		if (childcount == 0 || childcount < idx + 1) {
			return null;
		}
		//we don't need to check if parent is instanceof CommandModel
		//as getChildCount would have returned 0
		return ((CommandModel) parent).get(idx);
	}

	public int getChildCount(Object parent) {
		if (parent instanceof CommandModel && parent != null) {
			return ((CommandModel) parent).size();
		}
		return 0;
	}

	public int getIndexOfChild(Object parent, Object child) {
		//make sure parent has childs and is a CommandModel
		int childcount = getChildCount(parent);
		if (childcount == 0 || !(child instanceof CommandModel)) {
			return -1;
		}
		return ((CommandModel) parent).indexOf((CommandModel) child);
	}

	public Object getRoot() {
		return this;
	}

	public boolean isLeaf(Object node) {
		return (getChildCount(node) == 0);
	}

    public void addTreeModelListener(TreeModelListener l)
    {
        mListeners.add(TreeModelListener.class, l);
    }

    public void removeTreeModelListener(TreeModelListener l)
    {
        mListeners.remove(TreeModelListener.class, l);
    }

	public void valueForPathChanged(TreePath path, Object newValue) {
		//This code shouldn't be reached at all, as you shouldn't use any editor to modify the model
		fireTreeStructureChanged(path);
	}
}
