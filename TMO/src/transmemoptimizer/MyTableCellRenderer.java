/*
 * Created on 13.10.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.awt.Color;
import java.awt.Component;

import javax.swing.JTable;
import javax.swing.UIManager;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableColumnModel;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class MyTableCellRenderer extends DefaultTableCellRenderer
{
  private int row, col;
  
  public Component getTableCellRendererComponent (JTable table,
      Object value,
      boolean isSelected,
      boolean hasFocus,
      int row,
      int column)
  {
    // Save row and column information for use in setValue().
    this.row = row;
    this.col = column;
    
    // Allow superclass to return rendering component.
    Component c = super.getTableCellRendererComponent (table, value,
        isSelected, hasFocus, row, column);
//    TableColumnModel columnModel = table.getColumnModel();
//    setSize(columnModel.getColumn(column).getWidth(), 100000);
//    int height = c.getPreferredSize ().height;
//    if (height > table.getRowHeight (row))
//    {
//      table.setRowHeight( row, height );
//    }
    return c;
  }
  
  protected void setValue (Object v)
  {
    // Allow superclass to set the value.
    super.setValue (v);
    
    // If in names column, color cell with even row number white on
    // dark green, and cell with odd row number black on white.
    
    if (row % 2 == 0)
    {
      setForeground (UIManager.getColor ("Table.foreground"));
      setBackground (new Color (235, 235, 235));
    }
    else
    {
      setForeground (UIManager.getColor ("Table.foreground"));
      setBackground (UIManager.getColor ("Table.background"));
    }
    
  }
}
