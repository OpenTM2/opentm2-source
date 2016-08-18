/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom.editor;

import java.io.File;

public class TabTextEditor extends JEditTextArea {

	private static final long serialVersionUID = 1L;
	
	private File mFile = null;
    private String mFileName = null;
    
	public File getFile() {
		return mFile;
	}

	public void setFile(File mFile) {
		this.mFile = mFile;
		if(mFile != null)
			setFileName(mFile.getName());
	}

	public String getFileName() {
		return mFileName;
	}

	public void setFileName(String mFileName) {
		this.mFileName = mFileName;
	}
	
	
}
