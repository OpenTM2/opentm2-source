/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride;

import java.awt.Dimension;
import java.awt.EventQueue;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.Timer;
import java.util.TimerTask;

import javax.swing.JOptionPane;
import javax.swing.JTabbedPane;
import javax.swing.UIManager;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import de.ibm.com.opentm2scripteride.gui.MainWindow;
import de.ibm.com.opentm2scripteride.gui.TabTitlePanel;
import de.ibm.com.opentm2scripteride.gui.custom.ReportWindow;
import de.ibm.com.opentm2scripteride.gui.custom.editor.EditorConfigure;
import de.ibm.com.opentm2scripteride.gui.custom.editor.EditorTransferHandler;
import de.ibm.com.opentm2scripteride.gui.custom.editor.OpenTM2TokenMarker;
import de.ibm.com.opentm2scripteride.gui.custom.editor.TabTextEditor;
import de.ibm.com.opentm2scripteride.parser.ParserThread;
import de.ibm.com.opentm2scripteride.utility.Configuration;
import de.ibm.com.opentm2scripteride.utility.Constants;
import de.ibm.com.opentm2scripteride.utility.ErrorHandler;
import de.ibm.com.opentm2scripteride.utility.OneAppInstance;

/**
 * This class cares about opening, saving and parsing of files
 * It also includes the static main method as entry point for the application.
 */
public class MainApp {
	
	public static final int MAXTABSALLOWED = 20;
	
	static MainApp mInstance = null;
	static private String mAppName = "OpenTM2Scripter GUI";
	static private String mVersion = "1.3.0";
	
	
	protected MainWindow mMainWindow;
	protected File mScriptTemplate;
	protected TabTextEditor mActiveEditor;
	protected ParserThread mParserThread;
	protected Timer mReparseTimer;
	protected boolean mKnownUnsaved;
	private int mNewFileIndex = 0;
	
	
	/**
	 * Entry point for application, simply creates an instance of MainApp
	 * @param args The arguments we got from the environment or command line
	 */
	public static void main(String[] args) {
		// set windows look feel for report window
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {
			
		}
		
		// start from command line
		if(args.length==2 && "-report".equals(args[0]) && !args[1].isEmpty()) {
			   ReportWindow.createReportWindow(args[1],true);
			   return;
		}
		
		// only one instance allowed when start from GUI
		String lockResult = OneAppInstance.lock();
        if(lockResult != null) {
        	if(lockResult.equals(OneAppInstance.ONEINSTNCERUNNING)) {
        		JOptionPane.showMessageDialog(null, OneAppInstance.ONEINSTNCERUNNING, "OpenTM2ScripterGUI", JOptionPane.ERROR_MESSAGE);
        	} else {
        		JOptionPane.showMessageDialog(null, "OpenTM2ScripterGUI can't be started.\n"+lockResult, "OpenTM2ScripterGUI", JOptionPane.ERROR_MESSAGE);
        	}
        	return;
        }
        
        // start GUI
		getInstance(); 
	
	}
	
	/**
	 * The constructor. Initializes the variables and commissions the EventQueue with creating the MainWindow
	 */
	protected MainApp() {
		mScriptTemplate = new File(Constants.scriptTemplate);
		mParserThread = null;
		mReparseTimer = null;
		mKnownUnsaved = false;
		
		//copy the history configuration if have getOpenTM2Properpty()
		copyConfig(getOpenTM2Properpty()+"configuration.conf.data", "configuration.conf");
		
		//let the event queue create the main window
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					mMainWindow = new MainWindow(mAppName);
					mMainWindow.setVisible(true);

					//open opened files
					openFilesFromConfiguration();

					//updateWindowTitle();
				} catch (Exception e) {
					e.printStackTrace();
					ErrorHandler.getInstance().handleError(ErrorHandler.FATAL_ERROR, "Could not initialize the main window:", e);
				}
			}
		});	
	}
	
	/**
	 * This is the only way to construct and access the MainApp class.
	 * It makes sure that only one instance exists and everybody can globally access it
	 * @return The instance of the MainApp object
	 */
	public static MainApp getInstance() {
		if (mInstance == null) {
			mInstance = new MainApp();
		}
		return mInstance;
	}
	
	public void tabFocusChange(TabTextEditor editor, int index) {
		MainApp.getInstance().setActiveEditor(editor);
		if(index == -1) {
			MainApp.getInstance().newFile();
		}

		if(MainApp.getInstance().getActiveEditor().getFile() == null)
			mMainWindow.setSaveBtnState(false);
		else
			mMainWindow.setSaveBtnState(true);
		
		updateWindowTitle();
		startParsing();
	}
	
	/**
	 * Opens of file in the editor
	 * @param file The file to be opened
	 * @return True on success, false otherwise
	 */
	public boolean openFile(File file) {    
		TabTextEditor editor = null;
		// add a file tab
		try {
			editor = openTabFile(file, file.getName());
		} catch (IOException e) {
			ErrorHandler.getInstance().handleError(ErrorHandler.ERROR,
					"Failed to open file \"" + file.toString() + "\": " + e.toString());
			return false;
		}
		
		//reset values about its state
		mKnownUnsaved = false;	
		mMainWindow.setSaveBtnState(true);
		//mMainWindow.selectFunctionsTab();

		// make sure it's smaller than change time at here
		if(editor != null)
			editor.setDocSavedTime(System.currentTimeMillis());
		
		updateWindowTitle();
		
		//start to parse that file
		startParsing();
		
		return true;
	}
	
	/**
	 * Saves the content of the editor to the given filename
	 * @param filename The name of the file to write the editor's content in. Note that any existing file will be overwritten
	 * @return True on success, false otherwise
	 */
	public boolean saveFile(File filename) {
		//try to write the editors content to the specified file
		try{
//			FileWriter writer = new FileWriter(filename ,false);
//			String editorText = mActiveEditor.getText();
//			writer.write(convertLFtoOSLF(editorText));
//	 		writer.flush();
//	 		writer.close();
	 		
			BufferedWriter out = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(filename), "UTF-8"));
	 			try {
	 				String editorText = mActiveEditor.getText();
	 			    out.write(convertLFtoOSLF(editorText));
	 			    out.flush();
	 			} finally {
	 			    out.close();
	 			}
		}
		catch(IOException e){
			ErrorHandler.getInstance().handleError(ErrorHandler.ERROR, 
					"Failed writing to file " + filename.getName() + ": \n" + e.getMessage());
			return false;
		}
		
		mActiveEditor.setDocSavedTime( System.currentTimeMillis());
		
		//make sure we reparse
		reparse();
		return true;
	}
	
	/**
	 * Opens the template file in the editor, so the user can create a new file.
	 */
	public void newFile(){
		Configuration.getInstance().setValue(Constants.openedFile, "");
		
		String filename = "New "+Integer.toString(mNewFileIndex++);
		TabTextEditor editor = null;
		try {
			editor = openTabFile(null,filename);
		} catch (IOException e) {
			e.printStackTrace();
		}
			
		mMainWindow.setSaveBtnState(false);
		mKnownUnsaved = false;
		updateWindowTitle();
		
		// make sure it's smaller than change time at here
		if(editor != null)
		    editor.setDocSavedTime(System.currentTimeMillis());
				
		//make sure we reparse now
		reparse();
	}
	
	
	public TabTextEditor openTabFile(File file, String tabName) throws IOException {
		if(mMainWindow.getEditorTabbedPane().getTabCount() >= MAXTABSALLOWED) {
			JOptionPane.showMessageDialog(mMainWindow, 
					 "You have opened files more than "+Integer.toString(MAXTABSALLOWED)+", please close some before open or create new file",
                      "Infor", JOptionPane.INFORMATION_MESSAGE);
			return null;
		}
		
		//byte[] buffer = null;
		StringBuilder sbText = new StringBuilder();
	   	if(file != null)
	   	{	  		
	   		//check if the file already opened
			JTabbedPane tp = mMainWindow.getEditorTabbedPane();
			for(int i=0; i<tp.getTabCount(); i++) {
			    TabTextEditor tte = (TabTextEditor)tp.getSelectedComponent();
			    File editorFile = tte.getFile();
			    if( editorFile!=null && file.getAbsolutePath().equals( editorFile.getAbsolutePath() ) )
			    	return null;
			}
			
			//
		    //buffer = new byte[(int) file.length()];
		    //BufferedInputStream f = null;
            BufferedReader br = null;
           
		    try {
		        //f = new BufferedInputStream(new FileInputStream(file));	     
		        br = new BufferedReader( new InputStreamReader(new FileInputStream(file),"UTF-8") );
		        String tempLine = null;
		        while( (tempLine=br.readLine())!=null ) {
		        	sbText.append(tempLine);
		        	sbText.append(System.lineSeparator());
		        }
		    	//f.read(buffer);
		    } finally {
		        if (br != null) try { br.close(); } catch (IOException ignored) {}
		    }
	   	}
	   	
        // new an editor
	    TabTextEditor editor = new TabTextEditor();
		editor.getDocument().addDocumentListener(new DocumentChangeListener());
	    mMainWindow.addOpenedFileTab(tabName,editor);
	    editor.initFinished();
	    
	    // config editor
	    editor.configUndoManagerLimit(-1);
		editor.setTokenMarker(new OpenTM2TokenMarker());
	    editor.setMinimumSize(new Dimension(80,40));
		editor.setTransferHandler(new EditorTransferHandler(EditorTransferHandler.COPY_OR_MOVE));
		editor.setFile(file);
		if(file == null)
			editor.setFileName(tabName);
		//editor.setDocSavedTime(System.currentTimeMillis());
		
	    // config app editor
	    setActiveEditor(editor);
	    
        // repaint
		if(sbText.length()!=0) {
	        editor.setText(convertCRLFtoLF(sbText.toString()));
		}
	    editor.repaint();
	    editor.discardAllEdits();
	    editor.addDocUndoableEditListener();
	    editor.setCaretPosition(0);
	    
	    return editor;
	}
	
	/**
	 * Returns the main window object
	 * @return The main window
	 */
	public MainWindow getMainWindow(){
		return mMainWindow;
	}
	
	/**
	 * Returns the version of the application
	 * @return The current version as a string
	 */
	public String getVersion() throws IOException{
		BufferedReader br =null;
		try {
			br = new BufferedReader(new FileReader("resources/OpenTM2Version.info"));
			String line = br.readLine();
			if(line!=null && !line.isEmpty()){
				mVersion = line;
			}
		} finally {
			if(br != null)
		        br.close();
		}
		return mVersion;
	}

    /**
     * is the active editor document saved?
     * @return
     */
	public boolean isActSaved() {
		return mActiveEditor.isDocSaved();
	}
	
	/**
	 * Returns the name of the application
	 * @return The name of the application
	 */	
	public String getAppName() {
		return mAppName;
	}
	
	/**
	 * Starts a timer for permanent parsing of the editors content
	 */
	public void startParsing() {
		//update every 1.5 seconds if we have a change
		final int interval = 1500;
		
		//make sure we reparse now
		reparse();
		
		//kill the old timer if there is one
		if (mReparseTimer != null) {
			mReparseTimer.cancel();
			mReparseTimer = null;
		}
		
		//start a new timer
		mReparseTimer = new Timer();
		mReparseTimer.schedule(
			new TimerTask() {
				//check if change since last time
				public void run() {
					//re-parse if there were changes in the last interval
					if (mActiveEditor!=null && mActiveEditor.getDocChangedTime() > System.currentTimeMillis() - interval) {
						reparse();
					}
				}
			},
			interval,
			interval
		);
	}

	/**
	 * Updates the window title, regarding the state of the document
	 */
	public void updateWindowTitle() {
		if( mActiveEditor == null)
			return;
		//first the application name
		StringBuffer sb = new StringBuffer(mAppName);
		sb.append(" - ");
		
		//then the file if there is an opened file
		String tabName = mActiveEditor.getFileName();
		StringBuffer sbTab = new StringBuffer();
		
		if (!isActSaved()) {
		    sb.append("*");
		    sbTab.append("*");
	    }
		
		if (mActiveEditor.getFileName() != null) {
			sb.append(tabName);
			// also update tab name
			sbTab.append(tabName);	    
			int index = mMainWindow.getEditorTabbedPane().indexOfComponent(mActiveEditor);
			TabTitlePanel ttp = (TabTitlePanel)mMainWindow.getEditorTabbedPane().getTabComponentAt(index);
		    ttp.setTitle(sbTab.toString(),index);
		}

		mMainWindow.setTitle(sb.toString());
	}
	
    /**
     * open files opened last time	
     */
	protected void openFilesFromConfiguration() {
		boolean hasOneAtleast = false;
		
		String value =Configuration.getInstance().getValue(Constants.openedFile);
		String[] filenames = value.split(",");
		if(filenames.length == 0)
			hasOneAtleast = false;

		for(int i=0; i<filenames.length; i++) {
			
			File file = new File(filenames[i]);
			if (file.canRead()) {
				openFile(file);
				hasOneAtleast = true;
			}
		}
		
		// add a New file
		if(!hasOneAtleast)
			newFile();
	}
	
	/**
	 * Reparses the document. It can be called manually if you have to reparse the editors content NOW.
	 * Otherwise a timer calls this functions in regular intervals as soon startParsing() was called
	 */
	protected void reparse() {
		//check if we have a parser thread and create one if we didn't
		if (mParserThread == null) {
			mParserThread = new ParserThread(mActiveEditor.getFile());
			mParserThread.start();
		}
		//get the editor's content
		StringBuffer buf = new StringBuffer(mActiveEditor.getText()).append("\n");
		//now tell the parser thread to parse it
		mParserThread.parse(mActiveEditor.getFile(), new ByteArrayInputStream(buf.toString().getBytes()));
	}
		
	/**
	 * A simple DocumentListener that notifies the MainApp if anything in the document was changed
	 */
	protected class DocumentChangeListener implements DocumentListener {

		public void changedUpdate(DocumentEvent e) { 
			documentChanged();
		}

		public void insertUpdate(DocumentEvent e) { 
			documentChanged(); 
		}

		public void removeUpdate(DocumentEvent e) { 
			documentChanged(); 
		}
		
		private void documentChanged() {
			//check for a new document to parse
			if ( mActiveEditor.getDocSavedTime()==0L &&  mActiveEditor.getDocChangedTime()==0L ) {
				startParsing();
				//mMainWindow.selectFunctionsTab();
			}

			mActiveEditor.setDocChangedTime(System.currentTimeMillis());
			updateWindowTitle();
		}
	}//end class DocumentChangeListener
	
	
	public TabTextEditor getActiveEditor() {
		return mActiveEditor;
	}
	
	public void setActiveEditor(TabTextEditor editor) {
		mActiveEditor = editor;
		EditorConfigure.getInstance().setEditor(editor);
	}

	/**
	 * get the property path of OpenTM2 installed
	 * @return
	 */
	public String getOpenTM2Properpty() {
		String location = null;
		File temp = new File("temp.txt");
        String path = temp.getAbsolutePath();
        String searching = File.separator+"OTM"+File.separator;
        int otmIdx = path.lastIndexOf(searching);
        if(otmIdx != -1) {
        	StringBuilder sbPropertyPath = new StringBuilder( path.substring(0, otmIdx+searching.length()) );
        	sbPropertyPath.append("PROPERTY");
        	sbPropertyPath.append(File.separator);
        	location = sbPropertyPath.toString();
        }
		return location;
	}
	
	/**
	 * copy OpenTM2ScripterGUI configuration to save as history
	 * @param fromWhere
	 * @param toWhere
	 * @return
	 */
	public boolean copyConfig(String fromWhere, String toWhere) {
		
		if(toWhere==null || toWhere.isEmpty() || fromWhere==null || fromWhere.isEmpty())
			return false;
		
		File fromWhereFile = new File(fromWhere);
		if(!fromWhereFile.exists())
			return false;
		
		File confFile = new File(toWhere);
		if(confFile.exists())
			confFile.delete();
		
		FileInputStream fins = null;
		FileOutputStream fouts = null;
		boolean bSuc = false;
		try {
			
			bSuc = confFile.createNewFile();
			if(bSuc) {
				fins = new FileInputStream(fromWhere);
				fouts = new FileOutputStream(confFile);
				byte[] cnts = new byte[1024*10];
				int len = -1;
				while( (len=fins.read(cnts)) != -1 ) {
					fouts.write(cnts,0,len);
				}
				fouts.flush();
			}
			
		} catch (FileNotFoundException e1) {
			
			e1.printStackTrace();
			
		} catch (IOException e1) {
			
			e1.printStackTrace();
			
		}finally{
			
			try {	
				
				if(fins != null)
				    fins.close();
				
				if(fouts != null)
					fouts.close();
			} catch (IOException e1){
				e1.printStackTrace();
			}
	   }//end finally
	
	   return bSuc;
    }
	
	private String convertCRLFtoLF(String input) {
		return input.replaceAll("\r\n", "\n").replaceAll("\r", "\n");
	}
	
	private String convertLFtoOSLF(String input) {
		String lineSep = System.getProperty("line.separator");
		
		if(!lineSep.equals("\n"))
		    return input.replaceAll("\n", lineSep);
		
		return input;
	}
}