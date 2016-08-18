/*
 * Created on 20.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import javax.swing.table.*;
import java.io.File;
import java.util.Date;
import java.util.Vector;


/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MatchListTableModel extends AbstractTableModel {

  protected String[] columnNames = new String[] 
  {
    "Sel", 
    "Num", 
    "Segment in new source document", 
    "Translation Memory Proposal", 
    "Document", 
    "Proposal document", 
    "Markup",
    "Fuzziness",
    "Changes"
  };
  
  protected int checkedElements;
  
  /** constants for html marking of JLabel text */
  protected static final String deleteStart = "<font color=\"#FF0000\"><strike>";
  protected static final String deleteEnd   = "</strike></font>";
  protected static final String insertStart = "<font color=\"#009900\">";
  protected static final String insertEnd   = "</font>";
  protected static final String modifyStart = "<font color=\"#0000FF\">";
  protected static final String modifyEnd   = "</font>";
  protected static final String htmlStart   = "<html>";
  protected static final String htmlEnd     = "</html>";
  
  protected Class[] columnClasses = new Class[] 
  { 
      Boolean.class, 
      Long.class, 
      String.class, 
      String.class, 
      String.class,  
      String.class, 
      String.class,
      Integer.class,
      String.class
  };

    protected MatchList matchList;
  
    public MatchListTableModel( MatchList matchListIn ) 
    {
      matchList = matchListIn;
      checkedElements = 0;
    }

  public int getCheckedElements() 
  {
    return( checkedElements );
  }

  public int getColumnCount() 
  { 
    return 9;  // A constant for this model 
  }  
  
  public int getRowCount() 
  { 
    return matchList.getLength(); 
  }  

  // Information about each column
  public String getColumnName(int col) 
  { 
    return columnNames[col]; 
  }
  
  public Class getColumnClass(int col) 
  { 
    return columnClasses[col]; 
  }

  // The method that must actually return the value of each cell
  public Object getValueAt(int row, int col) 
  {
    MatchEntry matchEntry = matchList.getAt( row );
    
    switch(col) 
    {
      case 0: return new Boolean(matchEntry.isSelected());
      case 1: return new Long( row );
      case 2: 
        return htmlStart + maskTagging( matchEntry.getSegment() ) + htmlEnd;
      case 3: 
      {
        String changesHTML = new String( htmlStart );
        Vector changes = matchEntry.getChanges();
        int entries = changes.size();
        int i = 0;
        while ( i < entries )
        {
          String changeCommand = (String)changes.get( i );
          if ( changeCommand.equals( "#delete#" ) )
          {
            i++;
            if ( i < entries  )
            {
              String changeText = (String)changes.get( i );
              changesHTML = changesHTML.concat( deleteStart + maskTagging( changeText ) + deleteEnd );
              i++;
            }
          }
          else if ( changeCommand.equals( "#equal#" ) )
          {
            i++;
            if ( i < entries  )
            {
              String changeText = (String)changes.get( i );
              changesHTML = changesHTML.concat( maskTagging( changeText ) );
              i++;
            }
          }
          else if ( changeCommand.equals( "#modify#" ) )
          {
            i++;
            if ( i < entries  )
            {
              String changeText = (String)changes.get( i );
              changesHTML = changesHTML.concat( modifyStart + maskTagging( changeText ) + modifyEnd );
              i++;
            }
            if ( i < entries  )
            {
              String changeText = (String)changes.get( i );
              changesHTML = changesHTML.concat( insertStart + maskTagging( changeText ) + insertEnd );
              i++;
            }
          }
          else if ( changeCommand.equals( "#insert#" ) )
          {
            i++;
            if ( i < entries  )
            {
              String changeText = (String)changes.get( i );
              changesHTML = changesHTML.concat( insertStart + maskTagging( changeText ) + insertEnd );
              i++;
            }
          }
          else
          {
            changesHTML = changesHTML.concat( changeCommand );
            i++;
          }
        }  
        changesHTML = changesHTML.concat( htmlEnd );
//        System.out.println( changesHTML );
        return changesHTML;
      }
      case 4: 
        return matchEntry.getDocument() + "(" + matchEntry.getSegmentNumber() + ")";
      case 5: 
        return matchEntry.getMatchDocument() + "(" + matchEntry.getSegmentNumberMatch() + ")"; 
      case 6: return matchEntry.getMarkup();
      case 7: 
      {
        Vector changes = matchEntry.getChanges();
        String result = "" + matchEntry.getFuzziness();
        
        int entries = changes.size();
        int i = 0;
        boolean containsInsertedStrings = false;
        while ( !containsInsertedStrings  && (i < entries) )
        {
          String changeCommand = (String)changes.get( i );
          if ( changeCommand.equals( "#insert#" ) )
          {
            containsInsertedStrings = true;
          }
          i++;
        }  
        if ( containsInsertedStrings )
        {
          result = result.concat( " (*)" );
        }
        return result;
      }
      case 8:
      {
        String changesList = new String( htmlStart );
        Vector changes = matchEntry.getChanges();
        int entries = changes.size();
        int i = 0;
        while ( i < entries )
        {
          String changeCommand = (String)changes.get( i );
          if ( changeCommand.equals( "#delete#" ) )
          {
            i++;
            if ( i < entries  )
            {
              String changeText = (String)changes.get( i );
              changesList = changesList.concat( "\"" + maskTagging( changeText ) + "\" ==&gt; \"\"<br>" );
              i++;
            }
          }
          else if ( changeCommand.equals( "#modify#" ) )
          {
            i++;
            if ( i < entries  )
            {
              String changeText = (String)changes.get( i );
              changesList = changesList.concat( "\"" + maskTagging( changeText ) + "\" ==&gt; " );
              i++;
            }
            if ( i < entries  )
            {
              String changeText = (String)changes.get( i );
              changesList = changesList.concat( "\"" + maskTagging( changeText ) + "\"<br>" );
              i++;
            }
          }
          else
          {
            i++;
          }
        }  
        changesList = changesList.concat( htmlEnd );
        return changesList;
      }
      default: return null;
    }
  }
  
  public boolean isCellEditable(int row, int col) 
  {
    //Note that the data/cell address is constant,
    //no matter where the cell appears onscreen.
    if (col == 0) 
    {
      return true;
    } 
    else 
    {
      return false;
    }
  }

 
  public void setValueAt(Object value, int row, int col) 
  {
    
    switch(col) 
    {
      case 0:
      {
        MatchEntry matchEntry = matchList.getAt( row );
        Boolean bValue = (Boolean)value; 
        boolean wasSelected = matchEntry.isSelected();
        matchEntry.setSelected( bValue.booleanValue() );
        if ( wasSelected != bValue.booleanValue() )
        {
          if ( wasSelected )
          {
            checkedElements--;
          }
          else
          {
            checkedElements++;
          }
        }
        // the fireTableCellUpdate has been moved to the TableSorter class
        // as only this class knows which cell has been modified
      }
      default: 
    }
    
  }
  
  public void deleteSelected() 
  {
    int entries = matchList.getLength();
    int i = 0;
    while ( i < entries )
    {
      MatchEntry matchEntry = matchList.getAt( i );
      if ( matchEntry.isSelected() )
      {
        matchList.removeAt( i );
        checkedElements--;
        entries--;  
      }
      else
      {
        i++;
      }
    }
    fireTableDataChanged();
  }
  private String maskTagging( String source )
  {
    String result = source.replace( "<", "&lt;" );    
    result = result.replace( ">", "&gt;" );    
    return( result );
  }
}