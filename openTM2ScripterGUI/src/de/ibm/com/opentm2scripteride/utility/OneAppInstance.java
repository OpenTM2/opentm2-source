/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.utility;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;

public class OneAppInstance {

	private static File mFile;
	private static FileChannel mFileChanel;
	private static FileLock mFileLock;
	private final static String LOCKFILENAME="OpenTM2Scripter_OneAPPLocker.lock";
	
	public final static String ONEINSTNCERUNNING = "One instance of OpenTM2ScripterGUI is running";
	/**
	 * lock the file
	 * @return
	 */
	public static String lock(){
		// create the file
		mFile = new File(LOCKFILENAME);
		if(mFile.exists()) {
			mFile.delete();
		}
		
		String message = null;
		// try to get the lock
		try {
			mFileChanel = new RandomAccessFile(mFile, "rw").getChannel();
			mFileLock = mFileChanel.tryLock();
			if(mFileLock == null) {
				// lock failed, means has already locked by others
				mFileChanel.close();
				message = ONEINSTNCERUNNING;
			}
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			message = e.getMessage();
		} catch (IOException e) {
			e.printStackTrace();
			message = e.getMessage();
		}
		
        return message;
	}
	
	/**
	 * unlock file lock
	 */
	public static void unlock() {
        try {
        	
        	if(mFileLock != null) {
            	mFileLock.release();
            	mFileChanel.close();
            	mFile.delete();
            }
        } catch(IOException e) {
        	e.printStackTrace();
        }   
	}
	
}
