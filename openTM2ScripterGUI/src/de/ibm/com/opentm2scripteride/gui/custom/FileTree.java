/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom;
import java.awt.Component;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Vector;

import javax.swing.ImageIcon;
import javax.swing.JComponent;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.TransferHandler;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;

import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.utility.ErrorHandler;

/**
 * A dynamically changing JTree, which shows the FileSystem
 * under the "root" path
 */
@SuppressWarnings("serial")
public class FileTree extends JTree
{	
	final static DataFlavor flavor =new DataFlavor(TreePath.class, "treePath");
	public FileTreeModel treeModel;
	public static int runFileTree = 0;
	public static int createFileTree = 1;
	private int mFileTreeType;
	protected File mContextFile;
	protected JPopupMenu mPopup;
	
	/**
	 * Returns the used TreeModel
	 * @return	the TreeModel
	 */
	public FileTreeModel getTreeModel() {
		return treeModel;
	}

	/**
	 * Creates a new FileTree and adds a MouseListener for double click events
	 * and a TransferHandler for DnD Events, only dragging is supported
	 * @param root The root dir, from which the file system should be listed
	 */
	public FileTree(String root, int type)
	{	
		mFileTreeType = type;
		File rootFile;
		if(root == null){
			rootFile = new File(System.getProperty("user.dir").toString());
		}
		else{
			rootFile = new File(root);
		}
		
		//set some values for the FileTree
		treeModel=new FileTreeModel(rootFile);
		setModel(treeModel);
		setCellRenderer(new FileCellRenderer());
		setDragEnabled(true);
		getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
		setTransferHandler(new TreeTransferHandler());
		
		createContextMenu();
		addMouseListener(new FileTreeListener());
	}
	
	/**
	 * Returns the data flavor used for dragging
	 * @return the data flavor, usually the static variable flavor
	 */
	public static DataFlavor getFlavor() {
		return flavor;
	}

	/**
	 * Returns the absolute selection path, because JTree isn't able to show it
	 * @return the Absolute Path for the current selection
	 */
	public String getAbsoluteSelectionPath(){
		String path = this.getSelectionPath().toString();
		path = path.substring(1, path.length()-1);
		path = path.replace(", ", "/");
		return path;
	}
	
	/**
	 * Creates the context menu for the FileTree
	 * The context menu has two items: One to rename a file/directoy, the other one to delete it
	 */
	protected void createContextMenu() {
		//first the item to rename sth
		mPopup = new JPopupMenu();
		JMenuItem rename = new JMenuItem("Rename", new ImageIcon("resources/icons/rename.png"));
		rename.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				//prepare a proper output string
				String fn = mContextFile.getName();
				String fileDir = mContextFile.isDirectory() ? "directory" : "file";
				String msg = "Please insert the new name for the " + fileDir + " " + fn + ":";
				//ask the user for a new string
				String newName = JOptionPane.showInputDialog(MainApp.getInstance().getMainWindow(), msg,
						"Rename " + fn, JOptionPane.INFORMATION_MESSAGE);
				//if he aborted, we abort too
				if (newName == null || newName.isEmpty()) {
					return;
				}
				//otherwise create the new file name
				File newFile = new File(mContextFile.getParent(), newName);
				//and rename it
				if (!mContextFile.renameTo(newFile)) {
		 			ErrorHandler.getInstance().handleError(ErrorHandler.ERROR,
		 					"Failed to rename the " + fileDir + " " + fn + " to " + newFile.getName());
				}
				//tell the tree that it has changed
				treeModel.fireNewRoot();
			}
		});
		mPopup.add(rename);

		//now the delete item
		JMenuItem delete = new JMenuItem("Delete", new ImageIcon("resources/icons/delete.png"));
		delete.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				//prepare variables for a proper output string
				int choose = 1;
				String options[] = {"Yes", "No"};
				String fn = mContextFile.getName();
				String fileDir = mContextFile.isDirectory() ? "directory" : "file";
				String question = "Do you really want to delete the " + fileDir + " " + fn + "?\n"
								+ "This cannot be undone!";
				//ask the user if (s)he's sure
		 		choose = JOptionPane.showOptionDialog(MainApp.getInstance().getMainWindow(), question, "Delete " + fn + "?",
		 					JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE, null, options, options[1]);
		 		//if the user aborts, we do it too!
		 		if (choose == 1) {
		 			return;
		 		}
		 		//otherwise delete the file/dir
		 		if (!mContextFile.delete()) {
		 			ErrorHandler.getInstance().handleError(ErrorHandler.ERROR,
		 					"Failed to delete the " + fileDir + " " + fn);
		 		}
		 		//tell the tree, that we updated something
		 		treeModel.fireNewRoot();
			}
		});
		mPopup.add(delete);
		
	}
	
	/**
	 * Mouse listener that either selects or opens the document on double-click (depending on its type)
	 * and opens the context menu on right-click
	 */
	protected class FileTreeListener extends MouseAdapter {
		public void mousePressed(MouseEvent e) {
			//open context menu if we have a right click
			if((e.getModifiers() & InputEvent.BUTTON3_MASK) != 0) {
				TreePath tp = getPathForLocation(e.getX(), e.getY());
				if (tp == null) {
					return;
				}
				Object o = tp.getLastPathComponent();
				if (treeModel.getRoot().equals(o)) {
					return;
				}
				if ( !(o instanceof File) ) {
					return;
				}
				File file = (File) o;
				mContextFile = file;
				mPopup.show(FileTree.this, e.getX(),e.getY());
				return;
			}
			//open file if we are in editor mode
			else if (e.getClickCount() == 2 && mFileTreeType == FileTree.createFileTree)  
		    {
		    	try 
		    	{
		    		TreePath path = getSelectionPath();
		    		File file=(File) path.getLastPathComponent();
		    		MainApp.getInstance().openFile(file);
		    		
		    	}
		    	catch(Exception e1){}
		    }
			//select file if we are in run mode
			else if (e.getClickCount() == 1 && mFileTreeType == FileTree.runFileTree)  
		    {
		    	try 
		    	{
		    		String path = getAbsoluteSelectionPath();
		    		File file= new File(path);
		    		// wlp add the isFile check
                    if(file.isFile())
		    		    MainApp.getInstance().getMainWindow().getRTSPanel().setSelFile(file);
		    		
		    	}
		    	catch(Exception e1){}
		    }
		}
	}

	/**
	 * The TreeModel for the FileTree
	 * some methods are already been filled in the AbstractTreeModel
	 */
	public class FileTreeModel extends AbstractTreeModel 
	{
		  private File rootDir;
		 
		  public FileTreeModel(File rootDir) 
		  {
		    this.rootDir = rootDir;
		  }
		 
		 
		  /**
		   * Gets every child, which should be shown under the current parent path.
		   * Also separates hidden files/folders.
		   */
		  @Override
		  public Object getChild(Object parentDir, int ChildIndex) 
		  {	
			  //Initialize vectors for file/folder separation
			Vector<File> fileChilds = new Vector<File>();
			Vector<File> folderChilds = new Vector<File>();
			
			   //Add files and folders to the right vector
		    File parent = (File) parentDir;
		    for (File child : parent.listFiles()) 
		    {
		    	 if (child != null && child.isDirectory()) {
		    		 folderChilds.add(child);
		    	 }
		    	 else if(child!= null && child.isFile()){
		    		 fileChilds.add(child);
		    	 }
		    }
		    
		    //initialize array and add the folders first
		    String[] children = new String[folderChilds.size()+fileChilds.size()];
		    int i;
		    
		    for(i=0; i<folderChilds.size(); i++)
		    {
		    	children[i]=folderChilds.get(i).getName();
		    }
		    
		    for(i=folderChilds.size(); i<folderChilds.size()+fileChilds.size();i++)
		    {
		    	children[i]=fileChilds.get(i-folderChilds.size()).getName();
		    }
		    
		    //remove the hidden files/folders
		    String[] newChildren=removeHiddenItems(children);
		    		    	
		   return new TreeFile(parent, newChildren[ChildIndex]);		 
		  }
		 
		  /**
		   * Counts how many childs need to be shown below current the parent path.
		   * The separation of hidden files and folders needs to start here
		   */
		  @Override
		  public int getChildCount(Object parent) 
		  {
		    File file = (File) parent;
		    if (file.isDirectory()) 
		    {
		      String[] childArray = file.list();
		      String[] newChildArray=removeHiddenItems(childArray);
		      
		      if (newChildArray != null)
		        return newChildArray.length;
		    }
		    return 0;
		  }
		  
		  	@Override
		    public boolean isLeaf( Object file ) 
		    {
		        return ((File)file).isFile();		   
		    }
		  
		    @Override
		    public Object getRoot() 
		    {
			    return rootDir;
		    }
		    
		    
		    // set new Root Directory and update Filetree
		    public void setnewRootDir(File newRoot){
		    	this.rootDir = newRoot;
		    	this.fireNewRoot();
		    }
		 
		    
		    /**
		     * Is needed when a path, file or folder is changed
		     * -> Not needed in out FileTree yet.
		     */
		    @Override
		    public void valueForPathChanged(TreePath path, Object value) 
		    {
		    	File old = (File) path.getLastPathComponent();
		    	String fileParentPath = old.getParent();
		    	String newFileName = (String) value;
		    	File targetFile = new File(fileParentPath, newFileName);
		    	old.renameTo(targetFile);
		    }
		    
		    /**
		     * Removes the Hidden Files and Folders from the JTree
		     * Implemented for Windows--> $ and for Linux--> .
		     * @param childArray The array with the current ChildList
		     * @return The filtered Array
		     */
		    public String[] removeHiddenItems(String[] childArray)
		    {	
		    	if(childArray!=null)
		    	{
		    		//An ArrayList is needed, because removing items from an array isn't possible
		    		List<String> childList=new ArrayList<String>(Arrays.asList(childArray));
		    		Vector<String> toBeRemoved= new Vector<String>();
		    		
		    		
		    		for(int i=0; i<childList.size(); i++)
		    		{	
		    			//check for the hidden Files/Folders
		    			if(childList.get(i).startsWith(".") || childList.get(i).startsWith("$"))
		    			{	
		    				//save the hidden Files/Folders into a string vector
		    				toBeRemoved.add(childList.get(i));
		    			}
		    		}
		    		
		    		for(int i=0; i<toBeRemoved.size();i++)
		    		{	
		    			//delete them from the ArrayList
		    			String s = toBeRemoved.get(i);
  			  			childList.remove(s);
		    		}
		    		
		    		//store the filtered array list into a new array and return it
		    		String newchildArray[]= new String[childList.size()];
		    		childList.toArray(newchildArray);
		    		
		    		return newchildArray;
		    	}
		    	
		    	else return new String[0];
		    }
		 
		    /**
		     * Is needed, so that the FileTree only shows the names of the files and not the whole path
		     *
		     */
		  private class TreeFile extends File 
		  {
			    public TreeFile(File parent, String child) 
			    {
			      super(parent, child);
			    }
			 
			    public String toString() 
			    {
			      return getName();
			    }
			  }
  
	}
	
	/**
	 * The TransferHandler class for the FileTree
	 * creates a file path object, which can be dragged 
	 *
	 */
	class TreeTransferHandler extends TransferHandler {
		
		final DataFlavor flavor = FileTree.getFlavor();
        DataFlavor[] pathFlavour = new DataFlavor[] {flavor};
       
		 /**
		  * Returns the supported actions, we only need Copy
		  * @return The copy-action constant
		  */
		public int getSourceActions(JComponent comp) {
	    	//only support copy action
	        return COPY;
	    }
		   
		
		/**
		 * Creates a Transferable object, in this case the tree path to 
		 * the dragged object
		 */
		protected Transferable createTransferable(JComponent c) {
            JTree tree = (JTree) c;
            final TreePath path = tree.getSelectionPath();
            Transferable transferable = new Transferable() {
            	

                @Override
                public DataFlavor[] getTransferDataFlavors() {
                    return pathFlavour;
                }

                @Override
                public boolean isDataFlavorSupported(DataFlavor flavor) {
                    return pathFlavour[0].equals(flavor);
                }

                @Override
                public Object getTransferData(DataFlavor flavor)
                        throws UnsupportedFlavorException, IOException {
                    return path;
                }

            };
            return transferable;
        }



	}

	/**
	 * Adds some better looking icons. 
	 *
	 */
  protected class FileCellRenderer extends DefaultTreeCellRenderer {
		ImageIcon mFolderIcon;
		ImageIcon mFileIcon;

		public FileCellRenderer() {
			mFileIcon = new ImageIcon("resources/icons/file.png");
			mFolderIcon = new ImageIcon("resources/icons/block.png");
		}

		public Component getTreeCellRendererComponent(JTree tree, Object value, boolean sel,
													  boolean expanded, boolean leaf, int row,
													  boolean hasFocus) {
			//first set the default stuff
			super.getTreeCellRendererComponent(tree, value, sel, expanded,	leaf, row, hasFocus);
			
			if ( !(value instanceof File) ) {
				return this;
			}
			File f = (File) value;
			
			if (f.isDirectory()) {
				setIcon(mFolderIcon);
			} else {
				setIcon(mFileIcon);
			}

			return this;
		}
	}
}
