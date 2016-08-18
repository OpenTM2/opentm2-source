/*
 * Created on 27.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 * 
 * Changes:
 * 
 * 2009/02/06: - allow single character change strings
 * 
 * 2009/01/28: - XML parsing: preset curData to empty string
 * 
 * 2006/05/11 - Write list in XML format only
 *            - Read list in XML format and plain text format  
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

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParserFactory;

import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.AttributesImpl;
import org.xml.sax.helpers.DefaultHandler;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class ChangeList extends DefaultHandler
{
  // list of delimiters to use when saving external change lists
  private static final char[] delimiterList = 
    {'/', '\\', '\'', '\"', '!', '+', '#', '-', '_', '=', ';', '§', '$', '%', '&', '(', ')',
     '?', '~', '@', ':', '.', ',', '^', '|' };
  
  private Vector changeListVector;
  private Vector changeListComments;

  // header info of replacement list
  private String templateVersion;
  private String projectName;
  private String projectCreationDateDay;
  private String projectCreationDateMonth;
  private String projectCreationDateYear;
  private String creatorName;
  private String creatorNotesId;
  private String creatorEmailId;
  private String chargeToIdAffected;
  private String targetLanguage;
  private String shipmentDateDay;
  private String shipmentDateMonth;
  private String shipmentDateYear;
  private String previousTpmsChargeToId;

  private String curData;
  private int OTHER      = 0;
  private int curGroup = OTHER;
  private int TEMPLATE   = 1;
  private int CREATEDATE = 2;
  private int CREATOR    = 3;
  private int SHIPDATE   = 4;
  private String from;
  private String to;
  private ChangeEntry newEntry;
  private boolean isMultiple;
  private Vector fromList;
  private Vector toList;
  
  
  
  public ChangeList()
  {
    // initialize privat variables
    changeListVector = new Vector();
    changeListComments = new Vector();
    templateVersion = "";
    projectName = "";
    projectCreationDateDay = "";
    projectCreationDateMonth = "";
    projectCreationDateYear = "";
    creatorName = "";
    creatorNotesId = "";
    creatorEmailId = "";
    chargeToIdAffected = "";
    targetLanguage = "";
    shipmentDateDay = "";
    shipmentDateMonth = "";
    shipmentDateYear = "";
    previousTpmsChargeToId = "";
  }
  
  public ChangeEntry getAt( int element )
  {
    ChangeEntry entry = (ChangeEntry)changeListVector.get( element );
    return( entry );
  }
  
  public int getLength()
  {
    return( changeListVector.size() ); 
  }

  public void add( ChangeEntry newEntry )
  {
    // add entry at correct position or replace any existing entry with same changeFrom text
    int size = changeListVector.size();
    int i = 0;
    boolean done = false;
    
    String newChangeFrom = newEntry.getChangeFrom();
    int newLen = newChangeFrom.length();
    
    boolean ignoreEntry = false;
    if ( newLen == 0 )
    {
      // ignore empty entry
      ignoreEntry = true;
    }
    if ( newLen == 1 )
    {
      // ignore entry if it is not '&' and '~'
      if ( (newChangeFrom.charAt(0) != '&') && (newChangeFrom.charAt(0) != '~') )
      {
//        ignoreEntry = true;
      }
    }
    
    if ( !ignoreEntry )                       
    {
      while ( !done && (i < size) )
      {
        ChangeEntry entry = (ChangeEntry)changeListVector.get( i );
        String entryChangeFrom = entry.getChangeFrom();
        int entryLen = entryChangeFrom.length();
        if ( newLen == entryLen )
        {
          if ( entryChangeFrom.equals( newChangeFrom ) )
          {
            // replace entry with new contents
            entry.setChangeTo( newEntry.getChangeTo() );
            done = true;
          }
          else
          {
            // continue with next entry
            i++;
          }
        }
        else if ( newLen > entryLen )
        {
          // insert new entry here
          changeListVector.add( i, newEntry );
          done = true;
        }
        else
        {
          // continue with next entry
          i++;
        }
      }
      if ( !done )
      {
        changeListVector.add( newEntry );
      }
    }
  }
  
  public void add( String from, String to )
  {
    ChangeEntry entry = new ChangeEntry( from, to );
    add( entry );
  }
  
  public void add( String from )
  {
    ChangeEntry entry = new ChangeEntry( from, null );
    add( entry );
  }

    // This method is called when an element is encountered
    public void startElement( String uri, String name, String qName, org.xml.sax.Attributes atts )
    {
      curData = "";
      String curElement = qName.toLowerCase();
      
      if ( curElement.equals( "changeentry" ) )
      {
        newEntry = new ChangeEntry();
        isMultiple = false;
        fromList = null;
        toList = null;
      }
      else if ( curElement.equals( "change" ) )
      {
        isMultiple = true;
        fromList = new Vector();
        toList = new Vector();
      }
      else if ( curElement.equals( "firstshipmentdate" ) )
      {
        curGroup = SHIPDATE;
      }
      else if ( curElement.equals( "creationdate" ) )
      {
        curGroup = CREATEDATE;
      }
      else if ( curElement.equals( "template" ) )
      {
        curGroup = TEMPLATE;
      }
      else if ( curElement.equals( "creator" ) )
      {
        curGroup = CREATOR;
      }
    }

    public void endElement( String namespace, String localName, final String type ) 
    {
      String curElement = type.toLowerCase();
      
      // process data containing elements
      if ( curData != null )
      {
        if ( curElement.equals( "from" ) )
        {
          from = curData;
        }        
        else if ( curElement.equals( "to" ) )
        {
          to = curData;
        }        
        else if ( curElement.equals( "day" ) )
        {
          if ( curGroup == SHIPDATE )
          {
            this.shipmentDateDay = curData;
          }
          else if ( curGroup == CREATEDATE )
          {
            this.projectCreationDateDay = curData;
          }
        }
        else if ( curElement.equals( "month" ) )
        {
          if ( curGroup == SHIPDATE )
          {
            this.shipmentDateMonth = curData;
          }
          else if ( curGroup == CREATEDATE )
          {
            this.projectCreationDateMonth = curData;
          }
        }
        else if ( curElement.equals( "year" ) )
        {
          if ( curGroup == SHIPDATE )
          {
            this.shipmentDateYear = curData;
          }
          else if ( curGroup == CREATEDATE )
          {
            this.projectCreationDateYear = curData;
          }
        }
        else if ( curElement.equals( "version" ) )
        {
          if ( curGroup == TEMPLATE )
          {
            this.templateVersion = curData;
          }
        }
        else if ( curElement.equals( "name" ) )
        {
          if ( curGroup == CREATOR )
          {
            this.creatorName = curData;
          }
          else
          {
            this.projectName = curData;              
          }
        }
        else if ( curElement.equals( "notesid" ) )
        {
          this.creatorNotesId = curData;
        }
        else if ( curElement.equals( "emailid" ) )
        {
          this.creatorEmailId = curData;
        }
        else if ( curElement.equals( "chargetoidaffected" ) )
        {
          this.chargeToIdAffected = curData;
        }
        else if ( curElement.equals( "targetlanguage" ) )
        {
          this.targetLanguage = curData;
        }
        else if ( curElement.equals( "previoustpmschargetoid" ) )
        {
          this.previousTpmsChargeToId = curData;
        }
      }
      
      // process end of change elements
      if ( curElement.equals( "changeentry" ) )
      {
        boolean ok = false;
        
        if ( isMultiple )
        {
          String newFrom = "";
          int size = fromList.size();
          for ( int i = 0; i < size; i++ )
          {
            String temp = (String)fromList.get( i );
            newFrom = newFrom + temp + "\n";
          }
          newEntry.setChangeFrom( newFrom, false );
          newEntry.setMultChangeLists( fromList, toList );
          ok = true;
        }
        else if ( from != null )
        {
          if ( to == null )
          {
            to = "";
          }
          newEntry.setChangeFrom( from, false );
          newEntry.setChangeTo( to, false );
          ok = true;
        }
        
        if ( ok )
        {
          this.add( newEntry );
        }
      }
      else if ( curElement.equals( "change" ) )
      {
        if ( from != null )
        {
          if ( to == null )
          {
            to = "";
          }
         fromList.add( from );
         toList.add( to );
        }
      }
      
    }

    public void characters( char[] ch, int start, int len )  
    { 
      String text = new String( ch, start, len );
      if ( curData == null )
      {
        curData = text;
      }
      else
      {
        curData = curData.concat( text );
      }
    }

  // add changes from an external change list file
  public void add( File file )
  {
    boolean commentLinesFinished = false;
    
    try
    {
      InputStreamReader inReader;
      boolean isUTF16 = Utils.isUTF16Encoded( file );
      FileInputStream inStream = new FileInputStream( file  );
      if ( isUTF16 )
      {
        inReader = new InputStreamReader( inStream, "utf-16") ;
      }
      else
      {
        inReader = new InputStreamReader( inStream, "Cp1250") ;
      }
      BufferedReader in = new BufferedReader( inReader );
      String s;
      s = in.readLine();
      
      // test format of replacement list
      if ( (s != null) && ((s.toLowerCase()).startsWith( "<?xml" )) )
      {
        // load XML based list
        in.close();
        
        try 
        {
          // Create a builder factory
          SAXParserFactory factory = SAXParserFactory.newInstance();
          factory.setValidating(false);

          // Create the builder and parse the file
          factory.newSAXParser().parse( file, this );
	    } 
        catch (SAXException e) 
	    {
          // A parsing error occurred; the xml input is not valid
        } 
        catch (ParserConfigurationException e) 
        {
        } 
        catch (IOException e) 
        {
        } 
      }
      else
      {
        while ( s != null )
        {
          if ( s.length() == 0 )
          {
            // ignore empty lines
          }
          else if ( s.charAt( 0 ) == '*' )
          {
            if ( (s.length() > 1) && (s.charAt( 1 ) == '*') )
            {
              // ignore our own comment lines
            }
            else if ( !commentLinesFinished )
            {
              // add line to the comment vector
              changeListComments.add( s );
            }
          }
          else
          {
            commentLinesFinished = true;  
            int len = s.length();
            if ( len > 3 )
            {
              String changeFrom = new String();
              String changeTo   = new String();
              
              // split line into tokens at delimiters
              Vector tokenVector = Utils.splitIntoTokens( s, s.charAt( 0 ) );
              
              // the first (n+1)/2 tokens are change from strings
              int tokens = tokenVector.size();
              int changeFromStrings = (tokens + 1) / 2;
              int changeToStrings = 0;
              
              for( int i = 0; i < tokens; i++ )
              {
                if ( i < changeFromStrings )
                {
                  if ( changeFrom.length() != 0 )
                  {
                    changeFrom = changeFrom.concat( "\n" ); 
                  }
                  changeFrom = changeFrom.concat( (String)tokenVector.get( i ) ); 
                }
                else
                {
                  if ( changeTo.length() != 0 )
                  {
                    changeTo = changeTo.concat( "\n" ); 
                  }
                  changeTo = changeTo.concat( (String)tokenVector.get( i ) );
                  changeToStrings++;
                } /* endif */
              } /* endfor */
              
              // add missing changeTo strings
              while ( changeToStrings < changeFromStrings )
              {
                changeTo = changeTo.concat( "\n" ); 
                changeToStrings++;
              } /* endwhile */
              
              String newChangeFrom = new String( changeFrom );
              String newChangeTo = new String( changeTo );
              
              add( changeFrom, changeTo );
            }
          }
          s = in.readLine();
        }
        in.close();
      }
      }
        
    catch (IOException ioe )
    {
      // TODO handle exception
    }
  }

  // save changes to an external change list file in XML format
  public void save( File file )
  {
    file.delete();
    try
    {
      FileOutputStream out = new FileOutputStream( file );
      OutputStreamWriter outStream = new OutputStreamWriter( out );
      BufferedWriter outWriter = new BufferedWriter( outStream );
      DataWriter w = new DataWriter( outWriter );
      
      try
      {
        w.setIndentStep(2);
        w.startDocument();
        
        w.startElement("mapList");
        writeHeaderToFile( w );

        w.startElement("ChangeList");
        
        int entries = getLength();
        for ( int i = 0; i < entries; i++ )
        {
          ChangeEntry entry = getAt( i );
          
          if ( entry.isMultipleChange() )
          {
            w.startElement( "ChangeEntry" );
            Vector fromList = entry.getMultChangeFromList();
            Vector toList = entry.getMultChangeToList();
            int iParts = fromList.size();
            for ( int j = 0; j < iParts; j++ )
            {
              w.startElement( "Change" );
              w.dataElement( "From", (String)fromList.elementAt(j) );
              w.dataElement( "To", (String)toList.elementAt(j) );
              w.endElement( "Change" );
            }
            w.endElement("ChangeEntry");
          }
          else
          {
            w.startElement( "ChangeEntry" );
            w.dataElement("From", entry.getChangeFrom() );
            w.dataElement("To", entry.getChangeTo() );
            w.endElement("ChangeEntry");
          }
        }
        
        w.endElement("ChangeList");
        
        w.endElement("mapList");
        w.endDocument();
      }
      catch ( SAXException se )
      {
        
      }
    }
    catch ( IOException ioe )
    {
      
    }
}

  private int writeHeaderToFile( DataWriter w )
  {
    int iResult = 0;
    
    try
    {
      w.startElement( "Header" );
      
      w.startElement( "Template" );
      w.dataElement( "Version", templateVersion );
      w.endElement( "Template" );
      
      w.startElement( "Project" );
      w.dataElement( "Name", projectName );
      
      w.startElement( "CreationDate" );
      w.dataElement( "Day", projectCreationDateDay );
      w.dataElement( "Month", projectCreationDateMonth );
      w.dataElement( "Year", projectCreationDateYear );
      w.endElement( "CreationDate" );
      
      w.startElement( "Creator" );
      w.dataElement( "Name", creatorName );
      w.dataElement( "NotesId", creatorNotesId );
      w.dataElement( "EmailId", creatorEmailId );
      w.endElement( "Creator" );
      
      w.dataElement( "ChargeToIdAffected", chargeToIdAffected );
      w.dataElement( "TargetLanguage", targetLanguage );
      
      w.startElement( "FirstShipmentDate" );
      w.dataElement( "Day", shipmentDateDay );
      w.dataElement( "Month", shipmentDateMonth );
      w.dataElement( "Year", shipmentDateYear );
      w.endElement( "FirstShipmentDate" );
      
      w.dataElement( "PreviousTpmsChargeToId", previousTpmsChargeToId );
      
      w.endElement( "Project" );
      
      w.endElement( "Header" );
    }
    catch ( SAXException se )
    {
      iResult = 18;    
    }
    
    return( iResult );
  }
  
  private String makeChangeString( String changeString, char delimiter )
  {
    String result = new String();
    
    // split string into tokens using LF as delimiter
    Vector tokenVector = Utils.splitIntoTokens( "\n" + changeString + "\n", '\n' );
    int tokens = tokenVector.size();
    for( int i =0; i < tokens; i++ )
    {
      result = result.concat( delimiter + (String)tokenVector.get( i ) );
    } /* endfor */
    return( result );
  }
  
  // add changes from another changelist (thus merging the lists)
  public void add( ChangeList addList )
  {
    int max = addList.getLength();
    int i = 0;
    while ( i < max )
    {
      ChangeEntry entry = addList.getAt( i );      
      add( entry );
      i++;
    }
  }
  
  public void clear()
  {
    changeListVector.clear();
  }

  public void removeAt( int element )
  {
    changeListVector.remove( element );
    return;
  }

}
