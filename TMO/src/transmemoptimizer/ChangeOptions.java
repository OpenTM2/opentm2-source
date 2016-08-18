/*
 * Created on 16.12.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ChangeOptions
{
  public boolean changeNames;
  public ChangeList nameChangeList;
  public ChangeList markupChangeList;
  public int changeNameMode;
  public boolean logChanges;
  public boolean writeUnchangedSegments;
  public boolean writeOldAndNewSeg;
  public String logFileName;
  public boolean hyphenHandling;
  public boolean markAsMachineMatch;
  
  static int NAME_NAMEMODE = 1;
  static int PATH_NAMEMODE = 2;
  static int BOTH_NAMEMODE = 3;
  
  
  /**
   * 
   */
  public ChangeOptions()
  {
    changeNames = false;
    nameChangeList = null;
    changeNameMode = 0;
    logChanges = false;
    logFileName = null;
    hyphenHandling = false;
    writeUnchangedSegments = false;
    writeOldAndNewSeg = false;
    markAsMachineMatch = false;
  }
}
