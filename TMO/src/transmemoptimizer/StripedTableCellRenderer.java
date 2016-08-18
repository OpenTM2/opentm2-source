/*
 * Created on 28.10.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.awt.Color;
import java.awt.Component;

import javax.swing.JTable;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;

class StripedTableCellRenderer implements TableCellRenderer {
  public StripedTableCellRenderer(TableCellRenderer targetRenderer,
      Color evenBack, Color evenFore, Color oddBack, Color oddFore) {
    this.targetRenderer = targetRenderer;
    this.evenBack = evenBack;
    this.evenFore = evenFore;this.oddBack = oddBack;
    this.oddFore = oddFore;
  }

  // Implementation of TableCellRenderer interface
  public Component getTableCellRendererComponent( JTable table, Object value, boolean isSelected, boolean hasFocus,
      int row, int column )
  {
    TableCellRenderer renderer = targetRenderer;
    if ( renderer == null )
    {
      // Get default renderer from the table
      renderer = table.getDefaultRenderer( table.getColumnClass( column ) );
    }

    // Let the real renderer create the component
    Component comp = renderer.getTableCellRendererComponent( table, value, isSelected, hasFocus, row, column );

    // Now apply the stripe effect
    if ( isSelected == false && hasFocus == false )
    {
      if ( (row & 1) == 0 )
      {
        comp.setBackground( evenBack != null ? evenBack : table.getBackground() );
        comp.setForeground( evenFore != null ? evenFore : table.getForeground() );
      }
      else
      {
        comp.setBackground( oddBack != null ? oddBack : table.getBackground() );
        comp.setForeground( oddFore != null ? oddFore : table.getForeground() );
      }
    }

    return comp;
  }

  // Convenience method to apply this renderer to single column
  public static void installInColumn(JTable table, int columnIndex,
      Color evenBack, Color evenFore, Color oddBack, Color oddFore) {
    TableColumn tc = table.getColumnModel().getColumn(columnIndex);

    // Get the cell renderer for this column, if any
    TableCellRenderer targetRenderer = tc.getCellRenderer();

    // Create a new StripedTableCellRenderer and install it
    tc.setCellRenderer(new StripedTableCellRenderer(targetRenderer,
        evenBack, evenFore, oddBack, oddFore));
  }

  // Convenience method to apply this renderer to an entire table
  public static void installInTable( JTable table, Color evenBack, Color evenFore, Color oddBack, Color oddFore )
  {
    StripedTableCellRenderer sharedInstance = null;
    int columns = table.getColumnCount();
    for ( int i = 0; i < columns; i++ )
    {
      TableColumn tc = table.getColumnModel().getColumn( i );
      TableCellRenderer targetRenderer = tc.getCellRenderer();
      if ( targetRenderer != null )
      {
        // This column has a specific renderer
        tc.setCellRenderer( new StripedTableCellRenderer( targetRenderer, evenBack, evenFore, oddBack, oddFore ) );
      }
      else
      {
        // This column uses a class renderer - use a shared renderer
        if ( sharedInstance == null )
        {
          sharedInstance = new StripedTableCellRenderer( null, evenBack, evenFore, oddBack, oddFore );
        }
        tc.setCellRenderer( sharedInstance );
      }
    }
  }

  protected TableCellRenderer targetRenderer;

  protected Color evenBack;

  protected Color evenFore;

  protected Color oddBack;

  protected Color oddFore;
}
