/*
 * Created on 20.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import javax.swing.table.*;


/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ChangeListTableModel extends AbstractTableModel {

  protected String[] columnNames = new String[] { "Change from", "Change to" };
  protected Class[] columnClasses = new Class[] { String.class, String.class };

  protected ChangeList changeList;
  
    public ChangeListTableModel( ChangeList changeListIn ) 
    {
      changeList = changeListIn;
    }

  public int getColumnCount() 
  { 
    return 2;  // A constant for this model 
  }  
  
  public int getRowCount() 
  { 
    return changeList.getLength(); 
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
    ChangeEntry entry = changeList.getAt( row );
    
    switch(col) 
    {
      case 0: return entry.getChangeFrom();
      case 1: return entry.getChangeTo();
      default: return null;
    }
  }
  
  public boolean isCellEditable(int row, int col) 
  {
    return true;
  }
 
  public void setValueAt(Object value, int row, int col) 
  {
    ChangeEntry entry = changeList.getAt( row );
    
    switch(col) 
    {
      case 0: entry.setChangeFrom( (String)value ); break;
      case 1: entry.setChangeTo( (String)value );   break;
      default: 
    }
  	fireTableCellUpdated(row, col);
  }
  
  public void removeAt( int row ) 
  {
    changeList.removeAt( row );
    //fireTableDataChanged();
    fireTableRowsDeleted( row, row );
  }

  
  
}