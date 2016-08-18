/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeModel;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.EventListenerList;


/** Support for generic dynamically changing TreeModels.
 * Used for the unimplemented methods in FileTree
*/

public abstract class AbstractTreeModel
    implements TreeModel
{
    protected EventListenerList listeners;
    

    protected AbstractTreeModel()
    {
        listeners = new EventListenerList();
    }

    /**
     * Counts how many childs will be shown under the parent path
     */
    public int getIndexOfChild(Object parent, Object child)
    {
        for (int count = getChildCount(parent), i = 0; i < count; i++)
            if (getChild(parent, i).equals(child))
                return i;

        return -1;
    }


    /** Call when there is a new root, which may be null, i.e. not existent. */
    protected void fireNewRoot()
    {
        Object[] pairs = listeners.getListenerList();

        Object root = getRoot();

        TreePath path = (root != null) ? new TreePath(root) : null;
        
        TreeModelEvent e = null;
        
        for (int i = pairs.length - 2; i >= 0; i -= 2)
        {
            if (pairs[i] == TreeModelListener.class)
            {
                if (e == null)
                    e = new TreeModelEvent(this, path, null, null);
                
                ((TreeModelListener)pairs[i + 1]).treeStructureChanged(e);
            }
        }
    }
    

    /**
     * Adds a Tree model Listener
     */
    public void addTreeModelListener(TreeModelListener l)
    {
        listeners.add(TreeModelListener.class, l);
    }
    
    /**
     * Removes a Tree model Listener
     */
    public void removeTreeModelListener(TreeModelListener l)
    {
        listeners.remove(TreeModelListener.class, l);
    }
}