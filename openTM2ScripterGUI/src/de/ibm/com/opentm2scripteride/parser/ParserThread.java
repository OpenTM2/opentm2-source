/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.parser;

import java.io.File;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import javax.swing.SwingUtilities;
import org.antlr.runtime.ANTLRInputStream;
import org.antlr.runtime.CommonTokenStream;
import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.models.CommandModel;
import de.ibm.com.opentm2scripteride.models.ScriptModel;
import de.ibm.com.opentm2scripteride.utility.ErrorHandler;

public class ParserThread extends Thread {
	protected InputStream mInputStream;
	protected InputStream mNext;
	protected Lock mStreamLock;
	protected Lock mStatusLock;
	protected File mScriptName;
	protected boolean mParse;
	protected boolean mAbort;
	
	public ParserThread(File scriptName) {
		super("Parser");
		mInputStream = null;
		mNext = null;
		mStreamLock = new ReentrantLock();
		mStatusLock = new ReentrantLock();
		mScriptName = scriptName;
		mParse = false;
		mAbort = false;
	}
	
	public void parse(File scriptName, InputStream inputStream) {
		mStreamLock.lock();
		mNext = inputStream;
		mStreamLock.unlock();
		
		mStatusLock.lock();
		mScriptName = scriptName;
		mParse = true;
		mAbort = true;
		mStatusLock.unlock();
	}
	
	public void run() {
		while (true) {
			boolean doParse;
			mStatusLock.lock();
			doParse = mParse;
			if (mAbort) {
				mAbort = false;
			}
			mStatusLock.unlock();
			if (!doParse) {
				yield();
				try {
					sleep(500);
				} catch (InterruptedException e) {}
				continue;
			}
			
			mStreamLock.lock();
			if (mNext != mInputStream) {
				mInputStream = mNext;
				if (mInputStream == null) {
					mStatusLock.lock();
					mParse = false;
					mStatusLock.unlock();
					mStreamLock.unlock();
					continue;
				}
			}
			mStreamLock.unlock();
			try {
				//open file
				mStreamLock.lock();
				ANTLRInputStream ais = new ANTLRInputStream(mInputStream);
				mStreamLock.unlock();
				ScriptLexer lexer = new ScriptLexer(ais);
				CommonTokenStream cts = new CommonTokenStream(lexer);
				ScriptParser parser = new ScriptParser(cts);
				
				if (checkAbort()) {
					continue;
				}
				
				//parse it and get the model
				ScriptModel sm = parser.script();
				if (mScriptName != null) {
					mStatusLock.lock();
					sm.setName(mScriptName.getName());
					mStatusLock.unlock();
				}
				
				//close resources
				parser = null;
				cts = null;
				lexer = null;
				ais = null;
				mStreamLock.lock();
				mInputStream.close();
				mStreamLock.unlock();
		
				if (checkAbort()) {
					continue;
				}
				mStatusLock.lock();
				mParse = false; //we parsed successfully
				mStatusLock.unlock();				
				
				//check for invalid lines
				ArrayList<Integer> l = new ArrayList<Integer>();
				for (CommandModel cm : sm) {
					if (!cm.getBody().validates()) {
						l.add(new Integer(cm.getStartLine()));
					}
				}
				int inv[] = new int[l.size()];
				for (int i = 0; i < inv.length; i++) {
					inv[i] = l.get(i).intValue();
				}
				
				if (checkAbort()) {
					continue;
				}
				
				//update GUI
				SwingUtilities.invokeLater(new ParserGuiConnection(sm, inv));
			} catch (Exception e) {
				ErrorHandler.getInstance().logErrorDetails(e);
			}
		}
	}
	
	protected boolean checkAbort() {
		boolean ret;
		mStatusLock.lock();
		ret = mAbort;
		mStatusLock.unlock();
		return ret;
	}
}

class ParserGuiConnection implements Runnable {
	protected int[] mInvLines;
	protected ScriptModel mScriptModel;
	
	public ParserGuiConnection(ScriptModel sm, int[] inv) {
		mScriptModel = sm;
		mInvLines = inv;
	}
	
	public void run() {
		MainApp.getInstance().getMainWindow().getOutlineTree().setModel(mScriptModel);
		MainApp.getInstance().getActiveEditor().getPainter().setInvalidLines(mInvLines, mInvLines.length);	
	}
}

