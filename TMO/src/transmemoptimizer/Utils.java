/*
 * Created on 28.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Vector;

import javax.swing.JOptionPane;

public class Utils {

    public final static String xml = "xml";
    public final static String exp = "exp";
    
    
    /*
     * Get the extension of a file.
     */  
    public static String getExtension(File f) {
        String ext = null;
        String s = f.getName();
        int i = s.lastIndexOf('.');

        if (i > 0 &&  i < s.length() - 1) {
            ext = s.substring(i+1).toLowerCase();
        }
        return ext;
    }
    
    public static boolean isUTF16Encoded( File inFile )
    {
      boolean isUTF16 = false;
      
      try
      {
        FileInputStream fstream = new FileInputStream( inFile );
        int firstByte = fstream.read();
        int secondByte = fstream.read();
        fstream.close();
        if ( (firstByte == 255) && (secondByte == 254) ) 
        {
          isUTF16 = true;
        }
        
      }
      catch (IOException ioe )
      {
        // do not care ...
      }
      
      
      return( isUTF16 );
    }
 
    /** get the root directory of the TranslationManager installation */
    public static String getTranslationManagerRoot()
    {
      String result = null;
      boolean done = false;
      
      File [] roots =  File.listRoots();
      int drives = roots.length;
      for ( int i = 0; (i < drives) && !done; i++ )
      {
        String rootDir = new String( roots[i].getAbsolutePath() + "OTM" );
        File propFile = new File( rootDir + "\\PROPERTY\\EQFSYSW.PRP" );
        if ( propFile.exists() )
        {
          result = rootDir;
          done = true;
        }
      }
      
      return( result );
    }
    
    public static void showInfoMessage( String text )
    {
      Object[] options = { "OK" };
      JOptionPane.showOptionDialog(null, text, "Info",
          JOptionPane.OK_OPTION, JOptionPane.INFORMATION_MESSAGE,
          null, options, options[0]);  
      return;     
    }
    
    public static void showErrorMessage( String text )
    {
      Object[] options = { "CANCEL" };
      JOptionPane.showOptionDialog(null, text, "Error",
          JOptionPane.CANCEL_OPTION, JOptionPane.ERROR_MESSAGE,
          null, options, options[0]);  
      return;     
    }
    
    public static boolean showConfirmationMessage( String text )
    {
      Object[] options = { "OK", "CANCEL" };
      int iResult = JOptionPane.showOptionDialog(null, text, 
          "Overwrite Confirmation",
          JOptionPane.YES_OPTION, JOptionPane.QUESTION_MESSAGE,
          null, options, options[0]);  
      return( iResult == 0 );
    }
    
    public static Vector splitIntoTokens( String in, char delimiter )
    {
      Vector tokens = new Vector();
      
      int curPos = 0;
      
      // skip first delimiter if any
      if ( (in.length() > 0) && (in.charAt( 0 ) == delimiter) )
      {
        curPos = 1; 
      }
      
      do
      {
        int nextPos = in.indexOf( delimiter, curPos );
        if ( nextPos > 0 )
        {
          // add string up to delimiter
          tokens.add( in.substring( curPos, nextPos ) );
          curPos = nextPos + 1;
        }
        else
        {
          // add remaining data if not empty
          String remaining = in.substring( curPos );
          if ( remaining.length() != 0 )
          {
            tokens.add( remaining );
          }
          curPos = -1;
        }
      } while ( curPos > 0 );
      
      return( tokens );
    }
}