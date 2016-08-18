/*
 * Created on 27.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.util.Vector;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ChangeEntry
{
  /** string containing the chnange from text */
  private String changeFrom;
  
  /** string containing the change to text  */ 
  private String changeTo;
  
  /** flag for multiple change commands */
  private boolean isMultipleChange;
  
  /** list of single changes for multiple change commands */
  private Vector multChangeFromList;
  private Vector multChangeToList;
  
  public ChangeEntry()
  {
    this.changeFrom = null;
    this.changeTo = null;
    this.isMultipleChange = false;
    this.multChangeFromList = null;
    this.multChangeToList = null;
  }
  
  public ChangeEntry( String from )
  {
    this.changeFrom = from;
    this.changeTo = null;
    this.isMultipleChange = checkForMultipleChanges( from );
  }
  
  public ChangeEntry( String from, String to )
  {
    this.changeFrom = from;
    this.changeTo = to;
    this.isMultipleChange = checkForMultipleChanges( from ) || checkForMultipleChanges( to );
  }
  
  /**
   * @return Returns the changeFrom text.
   */
  public String getChangeFrom()
  {
    return changeFrom;
  }
  /**
   * @param changeFrom The Document to set.
   */
  public void setChangeFrom( String s )
  {
    this.changeFrom = s;
    this.isMultipleChange = this.isMultipleChange || checkForMultipleChanges( this.changeFrom );
    this.multChangeFromList = null;
  }
  public void setChangeFrom( String s, boolean multipleCheck )
  {
    this.changeFrom = s;
    if ( multipleCheck )
    {
      this.isMultipleChange = this.isMultipleChange || checkForMultipleChanges( this.changeFrom );
    }
    else
    {
      this.isMultipleChange = false;
    }
    this.multChangeFromList = null;
  }
  /**
   * @return Returns the changeTo string or null if none.
   */
  public String getChangeTo()
  {
    return changeTo;
  }
  /**
   * @param changeTo The change to text to set.
   */
  public void setChangeTo( String s )
  {
    this.changeTo = s;
    this.isMultipleChange = this.isMultipleChange || checkForMultipleChanges( this.changeTo );
    this.multChangeToList = null;
  }
  public void setChangeTo( String s, boolean multipleCheck  )
  {
    this.changeTo = s;
    if ( multipleCheck )
    {
      this.isMultipleChange = this.isMultipleChange || checkForMultipleChanges( this.changeTo );
    }
    else
    {
      this.isMultipleChange = false;
    }
    this.multChangeToList = null;
  }
  
  
  public boolean isMultipleChange()
  {
    return( this.isMultipleChange );
  }
  
  public static boolean checkForMultipleChanges( String s )
  {
    int lfPos = s.indexOf( '\n' );
    return( lfPos >= 0 );
  }

  public void setMultChangeLists( Vector fromList, Vector toList )
  {
    this.isMultipleChange = true;
    this.multChangeFromList = fromList;
    this.multChangeToList = toList;
  }
  
  /**
   * @return Returns the multiple changeTo list or null if none.
   */
  public Vector getMultChangeToList()
  {
    if ( this.isMultipleChange )
    {
      if ( this.multChangeToList == null )
      {
        this.multChangeToList = Utils.splitIntoTokens( this.changeTo, '\n' ); 
      }
      else
      {
        this.multChangeToList = new Vector();
      }
      
      // ensure that changeTo list has same size as changeFrom list
      if ( this.multChangeFromList == null )
      {
        getMultChangeFromList();
      }
      
      int toLen = this.multChangeToList.size();
      int fromLen = this.multChangeFromList.size();
      while ( toLen < fromLen )
      {
        this.multChangeToList.add( new String() );
        toLen++;
      } /* endwhile */
      
      return ( this.multChangeToList );
    }
    else
    {
      return null;
    }
  }
  
  /**
   * @return Returns the multiple changeFrom list or null if none.
   */
  public Vector getMultChangeFromList()
  {
    if ( this.isMultipleChange )
    {
      if ( this.multChangeFromList == null )
      {
        this.multChangeFromList = Utils.splitIntoTokens( this.changeFrom, '\n' ); 
      }
      return ( this.multChangeFromList );
    }
    else
    {
      return null;
    }
  }

}
