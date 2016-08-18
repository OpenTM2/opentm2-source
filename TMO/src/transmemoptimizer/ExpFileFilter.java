/*
 * Created on 28.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 *
 */
package transmemoptimizer;

import java.io.File;

import javax.swing.filechooser.FileFilter;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ExpFileFilter extends FileFilter
{
  public ExpFileFilter()
  {
    
  }
  public boolean accept(File f) 
  {
    if (f.isDirectory()) 
    {
      return true;
    }
    
    String extension = Utils.getExtension(f);
    if (extension != null) 
    {
      if (extension.equals(Utils.exp) )
      {
        return true;
      }
      else 
      {
        return false;
      }
    }
    
    return false;
  }
  
  public String getDescription()
  {
    return "exported memory files (*.EXP)";
  }
}
