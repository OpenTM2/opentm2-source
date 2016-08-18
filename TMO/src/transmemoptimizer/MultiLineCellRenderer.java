/*
 * Created on 29.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import javax.swing.*;
import javax.swing.table.*;
import javax.swing.border.*;
import java.awt.*;
 
/**
 * @version 1.0 11/09/98
 */
 

//public class MultiLineCellRenderer implements TableCellRenderer
public class MultiLineCellRenderer extends DefaultTableCellRenderer
{
  private int  fontSize = 9;

  private Font textFont = new Font( "Dialog", 0, fontSize );

  public Component getTableCellRendererComponent( JTable table, Object value, boolean isSelected, boolean hasFocus,
      int row, int column )
  {
    JEditorPane textArea;
    
    textArea = new JEditorPane();
    textArea.setEditable( false );
    textArea.setContentType( "text/html" );
    textArea.setMargin( new Insets( 0, 5, 0, 5 ) );
    
    String text = (value == null) ? "" : (String) value;
    textArea.setText( text );
    textArea.setFont( textFont );
    if (row % 2 == 0)
    {
      textArea.setBackground( new Color (235, 235, 235) );
    }
    int height = getHeight( textArea, table, text, row, column );
    if ( height > 0 )
    {
      table.setRowHeight( row, height );
    }
    return textArea;
  }

  private int row, col;
  
  public MultiLineCellRenderer()
  {
  }

  private int getHeight( JEditorPane textArea, JTable table, String text, int row, int column )
  {
    FontMetrics fm = textArea.getFontMetrics( textArea.getFont() );
    int numberOfTextRows = 2;
    int numberOfTextRows_IncludingNewlines = 0;
    int textWidth = 0;
    int columnWidth = 0;
    if ( fm != null )
    {
      textWidth = fm.stringWidth( text );
    }
    if ( textWidth > 0 )
    {
      TableColumn tableColumn = table.getColumnModel().getColumn( column );
      columnWidth = tableColumn.getWidth() - (textArea.getMargin().left + textArea.getMargin().right);
      numberOfTextRows = (textWidth / columnWidth + 1);
      if ( text.indexOf( '\n' ) > -1 )
      {
        for ( int index = 0; index < text.length(); )
        {
          try
          {
            int index_1 = text.indexOf( '\n', index );
            int index_2 = text.indexOf( '\n', index + 1 );
            String line = text.substring( index_1, index_2 );
            textWidth = fm.stringWidth( line );
            numberOfTextRows_IncludingNewlines += ((textWidth / columnWidth) + 1);
            index = index_2;
          }
          catch ( StringIndexOutOfBoundsException ex )
          {
            break;
          }
        }
        numberOfTextRows = Math.max( numberOfTextRows, numberOfTextRows_IncludingNewlines ) + 2;
      }
    }
    int rowHeight = fm.getHeight() * (numberOfTextRows + 1);
    if ( rowHeight > table.getRowHeight( row ) )
    {
      return rowHeight;
    }
    return 0;
  }
  
} 
  