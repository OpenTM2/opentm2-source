/*
 * Created on 11.10.2005
 *
 * 2008/01/31 - avoid rekursive replacing by using a mask string for the search which has
 *              blanks instead of the replaced string
 * 
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.Date;
import java.util.Vector;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ChangeMemoryJob extends Job
{
  private String inputMemory;
  private String outputMemory;
  private ChangeList changeList;
  private boolean removeLinkEnd;
  private String sourceLines;
  private String targetLines;
  private String sourceMask;
  private String targetMask;
  private ChangeOptions options;
  private String blankString;
  
  public ChangeMemoryJob( String inMem, String outMem, ChangeList changes, ChangeOptions inOptions )
  {
    inputMemory = inMem;
    outputMemory = outMem;
    changeList = changes;
    removeLinkEnd = false;
    options = inOptions;
  }
  
  public void run()
  {
    String source = new String();
    blankString = new String("                                                                                                                                                                                               ");
    
    try
    {
      // open input memory
      File inmem = new File( inputMemory );
      FileInputStream inStream = new FileInputStream( inmem );
      InputStreamReader inReader = new InputStreamReader( inStream,"utf-16") ;
      BufferedReader in = new BufferedReader( inReader );
      size = inStream.available();
      currentTask = "Changing memory";
      
      // open output memory
      File outmem = new File( outputMemory );
      outmem.delete();
      FileOutputStream outStream = new FileOutputStream( outmem );
      BufferedWriter out = new BufferedWriter(new OutputStreamWriter( outStream,"UnicodeLittle"));
      
      // open logfile
      BufferedWriter log = null;
      boolean logFileUsed = false;
      
      if ( options.logChanges )
      {
        File logFile = new File( options.logFileName );
        logFile.delete();
        FileOutputStream logStream = new FileOutputStream( logFile );
        log = new BufferedWriter(new OutputStreamWriter( logStream ) );
        log.write( "*** TMO Log " + new Date() + "***\r\n" );
      }

      String orgControlString = null;
      String newControlString = null;
      String segStart = null;
      String controlStart = null;
      String controlEnd = null;
      String segEnd = null;
      
      while ( (source = in.readLine()) != null )
      {
        String upperCasedString = source.toUpperCase();
        
        if ( upperCasedString.startsWith( "<SEGMENT>" ) )
        {
          segStart = source;
        }
        else if ( upperCasedString.startsWith( "</SEGMENT>" ) )
        {
          segEnd = source;
        }
        else if ( upperCasedString.startsWith( "</CONTROL>" ) )
        {
          controlEnd = source;
        }
        else if ( upperCasedString.startsWith( "<CONTROL>" ) )
        {
          controlStart = source;
          source = in.readLine();
          orgControlString = new String( source );
          newControlString = changeControlString( source );
        }
        else if ( upperCasedString.startsWith( "<SOURCE>" ) )
        {
          boolean dataChanged = false;
          
          sourceLines = null;
          targetLines = null;
          
          // get all source strings
          while ( (source != null) && !upperCasedString.startsWith( "<TARGET>") )
          {
            if ( sourceLines != null )
            {
              sourceLines = sourceLines.concat( "\r\n" + source );
            }
            else
            {
              sourceLines = new String( source );
            }
            source = in.readLine();
            if ( source != null ) upperCasedString = source.toUpperCase();
          } 

          // get all target lines
          while ( (source != null) && !upperCasedString.startsWith( "</SEGMENT>") )
          {
            if ( targetLines != null )
            {
              targetLines = targetLines.concat( "\r\n" + source );
            }
            else
            {
              targetLines = new String( source );
            }
            source = in.readLine();
            if ( source != null ) upperCasedString = source.toUpperCase();
          } 
          segEnd = source;
          
          // apply any changes on source and target of segment
          String orgSourceLines = new String( sourceLines );
          String orgTargetLines = new String( targetLines );
          
          dataChanged = changeSourceAndTarget();
          
          // check length of segments (may exceed segment size limit due to changes)
          if ( dataChanged )
          {
            // the max length 2047 + length of tags (<source>/</source>)
            if ( sourceLines.length() >= 2065 )
            {
              // show error message
              String segNum = segStart.substring( 9 );
              int seg = Integer.parseInt( segNum );
              String error = "Change of segment " + seg + " not performed as the changed segment data " +
                             "exceeds segment size limit (2047 characters)"; 
              Utils.showErrorMessage( error );
              
              // add error message to log file
              if ( options.logChanges && (log != null) )
              {
                log.write( "Error: Data could not be changed, segment size limit exceeded by change" );
                log.write( "Control record\r\n" );
                log.write( "  " + orgControlString + "\r\n" );
                log.write( "Data before change:\r\n" );
                log.write( "  " + orgSourceLines + "\r\n" );
                log.write( "  " + orgTargetLines + "\r\n" );
                log.write( "Data after change:\r\n" );
                log.write( "  " + sourceLines + "\r\n" );
                log.write( "  " + targetLines + "\r\n" );
              }
              
              // undo change
              dataChanged = false;
              sourceLines = orgSourceLines;
              targetLines = orgTargetLines;
              
            }
          }
          
          boolean controlChanged = !orgControlString.equals(newControlString );
          
          boolean writeOldSeg = false;
          
          // check if old segment has to be written to output memory
          if ( dataChanged || controlChanged )
          {
            writeOldSeg = options.writeOldAndNewSeg;
          }
          else
          {
            writeOldSeg = options.writeUnchangedSegments;
          }
          
          // write old segment
          if ( writeOldSeg )
          {
            // adjust segment number if new segment is written as well
            if ( dataChanged || controlChanged )
            {
              int delimiterPos = orgControlString.indexOf( 0x15 );    
              if ( delimiterPos >= 0 )
              {
                String segNum = orgControlString.substring( 0, delimiterPos );
                String rest = orgControlString.substring( delimiterPos );
                int seg = Integer.parseInt( segNum );
                seg = seg + 1;
                String newSegNum = Integer.toString( seg );
                int len = newSegNum.length();
                orgControlString = "000000000".substring( 0, 6 - len ) + newSegNum + rest;
              }
            }
            
            out.write( segStart + "\r\n" );
            out.write( controlStart + "\r\n" );
            out.write( orgControlString + "\r\n" );
            out.write( controlEnd + "\r\n" );
            out.write( orgSourceLines + "\r\n" );
            out.write( orgTargetLines + "\r\n" );
            out.write( segEnd + "\r\n" );
          }
          
          // write changed segment
          if ( dataChanged || controlChanged )
          {
            if ( options.markAsMachineMatch )
            {
              // set machine match flag in control record
              int delimiterPos = newControlString.indexOf( 0x15 );    
              if ( delimiterPos >= 0 )
              {
                String segNum = newControlString.substring( 0, delimiterPos + 1  );
                String rest = newControlString.substring( delimiterPos + 2 );
                newControlString = segNum + "1" + rest;
              }
            }
            
            out.write( segStart + "\r\n" );
            out.write( controlStart + "\r\n" );
            out.write( newControlString + "\r\n" );
            out.write( controlEnd + "\r\n" );
            out.write( sourceLines + "\r\n" );
            out.write( targetLines + "\r\n" );
            out.write( segEnd + "\r\n" );
          }
          
          // do any logging
          if ( options.logChanges && (log != null) )
          {
            if ( controlChanged || dataChanged )
            {
              logFileUsed = true;
              if ( controlChanged )
              {
                log.write( "Control record has been changed\r\n" );
                log.write( "Old value: " + orgControlString + "\r\n" );
                log.write( "New value: " + newControlString + "\r\n" );
              }
              else
              {
                log.write( "Control record\r\n" );
                log.write( "  " + orgControlString + "\r\n" );
              }
              
              if ( dataChanged )
              {
                log.write( "Data has been changed\r\n" );
                log.write( "Data before change:\r\n" );
                log.write( "  " + orgSourceLines + "\r\n" );
                log.write( "  " + orgTargetLines + "\r\n" );
                log.write( "Data after change:\r\n" );
                log.write( "  " + sourceLines + "\r\n" );
                log.write( "  " + targetLines + "\r\n" );
              }
              log.write( "\r\n" );
            }
          }
        }
        else
        {
          // write line to output
          out.write( source + "\r\n" );
        }
        status = size - inStream.available();
      }
      in.close();
      out.close();
      if ( options.logChanges && (log != null))
      {
        if ( !logFileUsed )
        {
          log.write( "no changes were applicable\r\n" );
        }
        log.close();
      }
      status = size;
    }
    catch ( IOException ioe )
    {
      System.err.println( "MLV memory change: file handling error");
    }
  }

  private boolean changeSourceAndTarget()
  {
    int numOfChanges = changeList.getLength();
    boolean done = false;
    boolean dataChanged = false;
    
    String orgSource = new String( sourceLines );
    String orgTarget = new String( targetLines );
    sourceMask = new String( orgSource );
    targetMask = new String( orgTarget );
    String tempSourceMask = null;
    String tempTargetMask = null;
        
    for( int i = 0; (i < numOfChanges) && !done; i++ )
    {
      ChangeEntry change = changeList.getAt( i );
      if ( change == null )
      {
        // nothing to do
      }
      else if ( change.isMultipleChange() )
      {
        
        // verify that change to strings are contained in source and target
        Vector changeFromList = change.getMultChangeFromList();
        Vector changeToList = change.getMultChangeToList();
        
        if ( containsAllChangeStrings( changeFromList, sourceMask ) &&
             containsAllChangeStrings( changeFromList, targetMask ) ) 
        {
          boolean changeOK = true;
          boolean somethingChanged = false;
          int iNumOfChanges = changeFromList.size();
          tempSourceMask = new String( sourceMask );
          tempTargetMask = new String( targetMask );
          
          for ( int j = 0; (j < iNumOfChanges) && changeOK; j++ )
          { 
            String from = (String)changeFromList.get( j );
            String to = (String)changeToList.get( j );
            int sourceChanges = doChange( from, to, true, options.hyphenHandling ); 
            int targetChanges = doChange( from, to, false, options.hyphenHandling ); 
            if ( sourceChanges != targetChanges )
            {
              // different number of changes in source and target 
              changeOK = false;
            }
            else if ( sourceChanges != 0 )
            {
              // data has been changed
              somethingChanged = true;
            }
            else
            {
              // nothing changed, we have to skip this chnage entry
              changeOK = false;
            }
          } /* endfor */
          
          // keep changes or restore original data
          if ( somethingChanged )
          {
            if ( changeOK )
            {
              // use changed string as new org strings
              orgSource = new String( sourceLines );
              orgTarget = new String( targetLines );
              
              
              // data has been changed
              dataChanged = true;
            }
            else
            {
              // discard changes made
              sourceLines = orgSource;
              targetLines = orgTarget;
              sourceMask = tempSourceMask;
              targetMask = tempTargetMask;
            }
          }
        } /* endif */
      }
      else if ( countainedInString( change.getChangeFrom(), sourceLines ) &&
                countainedInString( change.getChangeFrom(), targetLines ) ) 
      {
        tempSourceMask = new String( sourceMask );
        tempTargetMask = new String( targetMask );
        
        // apply single change on source and target lines
        int sourceChanges = doChange( change.getChangeFrom(), change.getChangeTo(), true, options.hyphenHandling ); 
        int targetChanges = doChange( change.getChangeFrom(), change.getChangeTo(), false, options.hyphenHandling  );
        
        if ( sourceChanges != targetChanges )
        {
          // different number of changes in source and target, restore original strings 
          sourceLines = orgSource;
          targetLines = orgTarget;
          sourceMask = tempSourceMask;
          targetMask = tempTargetMask;
        }
        else if ( sourceChanges != 0 )
        {
          // data has been changed
          dataChanged = true;
          
          // use changed string as new org strings
          orgSource = new String( sourceLines );
          orgTarget = new String( targetLines );
        }
      }
    } /* endfor */
    
    // do any additional changes
    if ( removeLinkEnd )
    {
      String linkEndStart = "<l linkend";
      String linkEndEnd   = "</l>";
//      String linkEndRegEx = "<l linkend=\"\\w*\"> ?";  // regex when blank is to be included...
      String linkEndRegEx = "<l linkend=\"\\w*\">";
      
      if ( countainedInString( linkEndStart, sourceLines ) &&
           countainedInString( linkEndEnd, sourceLines ) &&
           countainedInString( linkEndStart, targetLines ) &&
           countainedInString( linkEndEnd, targetLines ) )
      {
        String oldSource = sourceLines;
        String oldTarget = targetLines;
        sourceLines = sourceLines.replaceAll( linkEndRegEx, "" );
        sourceLines = sourceLines.replace( linkEndEnd, "" );
        targetLines = targetLines.replaceAll( linkEndRegEx, "" );
        targetLines = targetLines.replace( linkEndEnd, "" );
        if ( (oldSource != sourceLines) || (oldTarget != targetLines) )
        {
          dataChanged = true;
        }
      }
    }
    return( dataChanged );
  }

  private boolean containsAllChangeStrings( Vector strings, String lines )
  {
    boolean allFound = true;
    
    int iNumOfStrings = strings.size();
    
    for ( int i = 0; (i < iNumOfStrings) && allFound; i++ )
    { 
      if ( !countainedInString( (String)strings.get( i ), lines ) )
      {
        allFound = false;  
      }
    } /* endfor */
    return( allFound );
  }

  private boolean countainedInString( String search, String lines )
  {
    boolean found = lines.indexOf( search ) >= 0;
    
    if ( !found && options.hyphenHandling && search.endsWith( " " ))
    {
      String newSearch = search.substring( 0, search.length() - 1 ) + "-";
      found = lines.indexOf( newSearch ) >= 0;
    }
    return( found );
  }
  
  private int doChange( String changeFrom, String changeTo , boolean changeSource, boolean hyphenHandling  )
  {
    int iChanges = 0;
    
    String segmentData = changeSource ? sourceLines : targetLines;
    String mask = changeSource ? sourceMask : targetMask;
    
    if ( changeFrom == null )
    {
      // internal error: change from should always be set
      System.err.println( "TMO internal error, entry with null changefrom in change list");
    }
    else 
    {
      if ( changeTo == null )
      {
        changeTo = "";
      }
      
      int pos = 0;
      do
      {
        int newPos = mask.indexOf( changeFrom, pos );
        if ( newPos >= 0 )
        {
          int len = changeFrom.length();
          mask = mask.substring( 0, newPos ) + blankString.substring( 0, changeTo.length()) + mask.substring( newPos + len );
          segmentData = segmentData.substring( 0, newPos ) + changeTo + segmentData.substring( newPos + len );
          newPos += changeTo.length();
          iChanges++;
        }
        else if ( hyphenHandling )
        {
          if ( changeFrom.endsWith( " " ) && changeTo.endsWith( " ") )
          {
            String newChangeFrom = changeFrom.substring( 0, changeFrom.length() - 1 ) + "-";
            String newChangeTo = changeTo.substring( 0, changeTo.length() - 1 ) + "-";
            newPos = mask.indexOf( newChangeFrom, pos );
            if ( newPos >= 0 )
            {
              int len = newChangeFrom.length();
              mask = mask.substring( 0, newPos ) + blankString.substring( 0, newChangeTo.length()) + mask.substring( newPos + len );
              segmentData = segmentData.substring( 0, newPos ) + newChangeTo + segmentData.substring( newPos + len );
              newPos += newChangeTo.length();
              iChanges++;
            }
          }
        }
        pos = newPos;
      } while ( pos >= 0 );
      
      if ( iChanges != 0 )
      {
        if ( changeSource )
        {
          sourceLines = segmentData;
          sourceMask = mask;
        }
        else
        {
          targetLines = segmentData;
          targetMask = mask;
        }
      }
    }
    return( iChanges );
  }
  
  private String changeControlString( String source )
  {
    String target = new String( source );
    
    Vector tokens = new Vector();
    boolean tokensChanged = false;
    
    // split control string into tokens
    int x15Pos = 0;
    int iNum = 0;
    while ( x15Pos >= 0 ) 
    {
      iNum++;
      int nextX15Pos = target.indexOf( 0x15, x15Pos + 1 );
      int startPos = ( x15Pos == 0 ) ? 0 : x15Pos + 1;
      if ( nextX15Pos > 0 )
      {
        String token = target.substring( startPos, nextX15Pos );
        tokens.add( token );
      }
      else
      {
        String token = target.substring( startPos );
        tokens.add( token );
      }
      x15Pos = nextX15Pos;
    }

    // fill vector if not all elements have been read
    while ( iNum < 8 )
    {
      tokens.add( new String("") );
      iNum++;        
    }
    
    // change markup if found
    String markup = (String)tokens.get( 6 );
    if ( markup != null )
    {
      if ( options.markupChangeList.getLength() != 0 )
      {
        ChangeEntry entry = options.markupChangeList.getAt( 0 );
        if ( entry.getChangeFrom().equals( "***" ) )
        {
          // apply change to all markups
          markup = entry.getChangeTo();
          tokens.set( 5, markup );
          tokensChanged = true;
        }
        else
        {
          // use change list to check if markup table has to be changed
          int entries = options.markupChangeList.getLength();
          boolean done = false;
          for( int i = 0; (i < entries) && !done ; i++ )
          {
            ChangeEntry change = options.markupChangeList.getAt( i );
            if ( change != null )
            {
              String changeFrom = change.getChangeFrom();
              String changeTo = change.getChangeTo();
              if ( (changeFrom != null ) && (changeTo != null) )
              {
                if ( changeFrom.equals( markup ) )
                {
                  tokens.set( 5, changeTo ); 
                  done = true;
                  tokensChanged = true;
                }
              }
            }
            else
            {
              // internal error: entry should always be set
              System.err.println( "MLV internal error, null entry in change list");
            }
          }
        }
      }
    }
    
    // look and change document name if file name change has been selected
    if ( options.changeNames && (options.nameChangeList.getLength() != 0) )
    {
      String shortName = (String)tokens.get( 7 );
      String longName = (String)tokens.get( 8 );
      
      if ( (shortName != null) && (longName != null) )
      {
        // split name into path and name part
        String namePart = "";
        String pathPart = "";
        
        int pathDelimiter = longName.lastIndexOf( '\\' );
        if ( pathDelimiter > 0 )
        {
          namePart = longName.substring( pathDelimiter + 1 );  
          pathPart = longName.substring( 0, pathDelimiter + 1 );  
        }
        else
        {
          namePart = longName;
          pathPart = null;
        }
        
        if ( options.changeNameMode == ChangeOptions.BOTH_NAMEMODE )
        {
          int entries = options.nameChangeList.getLength();
          boolean done = false;
          for( int i = 0; (i < entries) && !done ; i++ )
          {
            ChangeEntry change = options.nameChangeList.getAt( i );
            if ( change != null )
            {
              String changeFrom = change.getChangeFrom();
              String changeTo = change.getChangeTo();
              if ( (changeFrom != null ) && (changeTo != null) )
              {
                String newName = longName.replace( changeFrom, changeTo ); 
                if ( newName != longName )
                {
                  tokens.set( 8, newName ); 
                  done = true;
                  tokensChanged = true;
                }
              }
            }
          }
        }
        else if ( options.changeNameMode == ChangeOptions.NAME_NAMEMODE )
        {
          int entries = options.nameChangeList.getLength();
          boolean done = false;
          for( int i = 0; (i < entries) && !done ; i++ )
          {
            ChangeEntry change = options.nameChangeList.getAt( i );
            if ( change != null )
            {
              String changeFrom = change.getChangeFrom();
              String changeTo = change.getChangeTo();
              if ( (changeFrom != null ) && (changeTo != null) )
              {
                String newName = namePart.replace( changeFrom, changeTo ); 
                if ( newName != namePart )
                {
                  if ( pathPart != null )
                  {
                    tokens.set( 8, pathPart + newName ); 
                  }
                  else
                  {
                    tokens.set( 8, newName ); 
                  }
                  done = true;
                  tokensChanged = true;
                }
              }
            }
          }
        }
        else if ( pathPart != null )   // change only if path is available
        {
          int entries = options.nameChangeList.getLength();
          boolean done = false;
          for( int i = 0; (i < entries) && !done ; i++ )
          {
            ChangeEntry change = options.nameChangeList.getAt( i );
            if ( change != null )
            {
              String changeFrom = change.getChangeFrom();
              String changeTo = change.getChangeTo();
              if ( (changeFrom != null ) && (changeTo != null) )
              {
                String newName = pathPart.replace( changeFrom, changeTo ); 
                if ( newName != pathPart )
                {
                  tokens.set( 8, newName + namePart ); 
                  done = true;
                  tokensChanged = true;
                }
              }
            }
          }
        }
      }
    }
    
    // build new control record if tokens have been changed
    if ( tokensChanged )
    {
      target = new String(  (String)tokens.get( 0 ) + "\u0015" +
           					(String)tokens.get( 1 ) + "\u0015" +
           					(String)tokens.get( 2 ) + "\u0015" +
           					(String)tokens.get( 3 ) + "\u0015" +
           					(String)tokens.get( 4 ) + "\u0015" +
           					(String)tokens.get( 5 ) + "\u0015" +
           					(String)tokens.get( 6 ) + "\u0015" +
           					(String)tokens.get( 7 ) + "\u0015" +
           					(String)tokens.get( 8 ) );
    }
    
    return( target );
  }
 
  /**
   * @param removeLinkEnd The removeLinkEnd to set.
   */
  public void setRemoveLinkEnd( boolean removeLinkEnd )
  {
    this.removeLinkEnd = removeLinkEnd;
  }
}
