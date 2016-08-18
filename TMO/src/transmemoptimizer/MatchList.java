/*
 * Created on 27.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 * 
 * Changes:
 * 
 * 2006/05/16: - Use SAX parser instead of DOM parser in load method (old method renamed to loadUsingDOM )
 */
package transmemoptimizer;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
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
public class MatchList extends DefaultHandler
{
  private DocumentBuilderFactory factory;
  private DocumentBuilder builder;
  private Document document;
  private Vector matchListVector;
  
  // variables for SAX parsing
  private String parseCurData;
  private MatchEntry parseNewEntry;
  private Vector parseChangeList;
  
  public MatchList()
  {
    matchListVector = new Vector();
  }
  
  public MatchEntry getAt( int element )
  {
    MatchEntry entry = (MatchEntry)matchListVector.get( element );
    return( entry );
  }
  
  public void removeAt( int element )
  {
    matchListVector.remove( element );
    return;
  }
  
  
  public int getLength()
  {
    return( matchListVector.size() ); 
  }
  
  private MatchEntry convertNodeToMatchEntry( Node entryNode )
  {
    boolean ok = true;
    
    MatchEntry entry = new MatchEntry();

    // loop entry node and collect entry data
    Node nodeChild = entryNode.getFirstChild();
    while ( ok && (nodeChild != null) )
    {
      String strName = nodeChild.getNodeName();
      if ( "segment".equals( strName ))
      {
        Node dataNode = nodeChild.getFirstChild();        // get text node
        entry.setSegment( dataNode.getNodeValue() );
        
        NamedNodeMap attr = nodeChild.getAttributes();
        
        Node attrNode = attr.getNamedItem( "document" );
        entry.setDocument( attrNode.getNodeValue() );
        
        attrNode = attr.getNamedItem( "segnumber" );
        entry.setSegmentNumber( Integer.parseInt( attrNode.getNodeValue() ) );
        
        attrNode = attr.getNamedItem( "markup" );
        entry.setMarkup( attrNode.getNodeValue() );
        
      }
      else if ( "match".equals( strName ))
      {
        NamedNodeMap attr = nodeChild.getAttributes();
        
        Node attrNode = attr.getNamedItem( "document" );
        entry.setMatchDocument( attrNode.getNodeValue() );
        
        attrNode = attr.getNamedItem( "fuzziness" );
        entry.setFuzziness( Integer.parseInt( attrNode.getNodeValue() ) );
        
        attrNode = attr.getNamedItem( "segnumber" );
        entry.setSegmentNumberMatch( Integer.parseInt( attrNode.getNodeValue() ) );
        
        attrNode = attr.getNamedItem( "memory" );
        entry.setMatchMemory( attrNode.getNodeValue() );
        
        attrNode = attr.getNamedItem( "markup" );
        entry.setMatchMarkup( attrNode.getNodeValue() );
        
        // loop child nodes to get source and target of match
        Node matchChildNode = nodeChild.getFirstChild();        // get text node
        while ( matchChildNode != null )
        {
          String strChildName = matchChildNode.getNodeName();
          if ( "source".equals( strChildName  ) )
          {
            Node dataNode = matchChildNode.getFirstChild();        // get text node
            entry.setMatchSource( dataNode.getNodeValue() );
          }
          else if ( "target".equals( strChildName  ) )
          {
            Node dataNode = matchChildNode.getFirstChild();        // get text node
            entry.setMatchTarget( dataNode.getNodeValue() );
          }
          matchChildNode = matchChildNode.getNextSibling();
        }
      }
      else if ( "changelist".equals( strName ))
      {
        // add changes to change list
        Vector changes = entry.getChanges();
        Node changeNode = nodeChild.getFirstChild();        // get text node
        while ( changeNode != null )
        {
          String changeName = changeNode.getNodeName();
          if ( "change".equals( changeName  ) )
          {
            NamedNodeMap attr = changeNode.getAttributes();
            Node attrNode = attr.getNamedItem( "type" );
            String nodeValue = attrNode.getNodeValue();
            // no identifier for changeto string
            if ( ! "changeto".equals( nodeValue ) )
            {
              changes.add( "#" + nodeValue + "#" );
            }
            else
            {
              int iyyyy = 5;
            }
            Node dataNode = changeNode.getFirstChild(); 
            if ( dataNode != null )
            {  
	            String text = dataNode.getNodeValue();
	            changes.add( text  );
            }
            else
            {
	            ok = false;
            }
          }
          changeNode = changeNode.getNextSibling();
        }
        
      }
      
      // continue with next sibling
      nodeChild = nodeChild.getNextSibling();
    }
    
    return( ok ? entry : null );
  }

  /**
   * @param absoluteFile
   */
  public boolean load( String matchList )
  {
    try 
    {
      matchListVector.clear();

      // Create a builder factory
      SAXParserFactory factory = SAXParserFactory.newInstance();
      factory.setValidating(false);

      // Create the builder and parse the file
      factory.newSAXParser().parse( matchList, this );
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
    
    return( true );
  }
  
  /**
   * @param absoluteFile
   */
  public boolean loadUsingDOM( String matchList )
  {
    int successfulEntries = 0;
    int failedEntries = 0;


    try
    {
      factory = DocumentBuilderFactory.newInstance();
      builder = factory.newDocumentBuilder();
      document = builder.parse( matchList );
      
      matchListVector.clear();
  
      NodeList entryNodeList = document.getElementsByTagName("entry");
      if ( entryNodeList != null )
      {
        int iLength = entryNodeList.getLength();
        int iCurrent = 0;
        while ( iCurrent < iLength )
        {
          Node entryNode = entryNodeList.item( iCurrent );
          if ( entryNode != null )
          {
	          MatchEntry entry = convertNodeToMatchEntry( entryNode );
	          if ( entry != null )
	          {
		          NamedNodeMap attr = entryNode.getAttributes();
		          Node attrNode = attr.getNamedItem( "selected" );
		          if ( attrNode != null )
		          {
		            String attrValue = attrNode.getNodeValue();
		            if ( attrValue.equalsIgnoreCase( "true" ) )
		            {
		              entry.setSelected( true );
		            }
		          }
		          
		          matchListVector.add( entry );
		          successfulEntries++;
	          }
	          else
	          {
		          failedEntries++;
	          }
          }
          iCurrent = iCurrent + 1;
        }
      }
      
    } 
    catch (Exception ex) 
    {
      ex.printStackTrace();
    }
    return( failedEntries == 0 );
  }
  
  public void save( File outFile )
  {
    outFile.delete();
    try
    {
      FileOutputStream out = new FileOutputStream( outFile );
      OutputStreamWriter outStream = new OutputStreamWriter( out );
      BufferedWriter outWriter = new BufferedWriter( outStream );
      DataWriter w = new DataWriter( outWriter );
      
      try
      {
        w.setIndentStep(2);
        w.startDocument();
        w.startElement("entrylist");
        
        AttributesImpl attr = new AttributesImpl();
        
        int entries = matchListVector.size();
        for ( int i = 0; i < entries; i++ )
        {
          MatchEntry entry = (MatchEntry)matchListVector.get( i );
          attr.clear();
          attr.addAttribute( "", "selected", "", "char", Boolean.toString( entry.isSelected() ) );
          w.startElement( "", "entry", "", attr );
          
          attr.clear();
          attr.addAttribute( "", "document", "", "char", entry.getDocument() );
          attr.addAttribute( "", "segnumber", "", "char", Integer.toString( entry.getSegmentNumber() ) );
          attr.addAttribute( "", "markup", "", "char", entry.getMarkup() );
          w.dataElement( "", "segment", "", attr, entry.getSegment() );
          
          attr.clear();
          attr.addAttribute( "", "fuzziness", "", "char", Integer.toString( entry.getFuzziness() ) );
          attr.addAttribute( "", "document", "", "char", entry.getMatchDocument() );
          attr.addAttribute( "", "segnumber", "", "char", Integer.toString( entry.getSegmentNumberMatch() ) );
          attr.addAttribute( "", "markup", "", "char", entry.getMatchMarkup() );
          attr.addAttribute( "", "memory", "", "char", entry.getMatchMemory() );
          w.startElement( "", "match", "", attr );
          w.dataElement("source", entry.getMatchSource() );
          w.dataElement("target", entry.getMatchTarget() );
          w.endElement("match");
          w.startElement("changelist");
          Vector changesVector = entry.getChanges();
          int changes = changesVector.size();
          for ( int j = 0; j < changes; j++  )
          {
            attr.clear();
            String changeCmd = (String)changesVector.get( j );
            j++;
            if ( j < changes )
            {
              String changeString = (String)changesVector.get( j );
              if ( "#delete#".equals( changeCmd ))
              {
                attr.clear();
                attr.addAttribute( "", "type", "", "char", "delete" );
                w.dataElement( "", "change", "", attr, changeString );
              }
              else if ( "#modify#".equals( changeCmd ))
              {
                attr.clear();
                attr.addAttribute( "", "type", "", "char", "modify" );
                w.dataElement( "", "change", "", attr, changeString );
                j++;
                if ( j < changes )
                {
                  changeString = (String)changesVector.get( j );
                  attr.clear();
                  attr.addAttribute( "", "type", "", "char", "changeto" );
                  w.dataElement( "", "change", "", attr, changeString );
                }
              }
              else if ( "#insert#".equals( changeCmd ))
              {
                attr.clear();
                attr.addAttribute( "", "type", "", "char", "insert" );
                w.dataElement( "", "change", "", attr, changeString );
              }
              else if ( "#equal#".equals( changeCmd ))
              {
                attr.clear();
                attr.addAttribute( "", "type", "", "char", "equal" );
                w.dataElement( "", "change", "", attr, changeString );
              }
            }
            
          }
          w.endElement("changelist");
          w.endElement("entry");
          
        }
        
        w.endElement("entrylist");
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
  
  
  // methods for parsing XML file using SAX

  // this method is called when an element is encountered
  public void startElement( String uri, String name, String localName, org.xml.sax.Attributes atts )
  {
    String curElement = localName.toLowerCase();
    
    parseCurData = null;
    
    if ( curElement.equals( "entry" ) )
    {
      parseNewEntry = new MatchEntry();
      
      String selected = atts.getValue( "selected" );
      if ( selected != null )
      {
        parseNewEntry.setSelected( selected.equalsIgnoreCase( "true" ) );        
      }
    }
    else if ( curElement.equals( "segment" ) )
    {
      // attributes: "document", "segnumber", "markup",
      
      String doc = atts.getValue( "document" );
      if ( doc != null )
      {
        parseNewEntry.setDocument( doc );        
      }
     
      String seg = atts.getValue( "segnumber" );
      if ( seg != null )
      {
        parseNewEntry.setSegmentNumber( Integer.parseInt( seg ) );        
      }
     
      String markup = atts.getValue( "markup" );
      if ( markup != null )
      {
        parseNewEntry.setMarkup( markup );        
      }
     
    }
    else if ( curElement.equals( "match" ) )
    {
      // attributes: "fuzziness", "document", "segnumber", "markup", "memory" 
      
      String fuzzy = atts.getValue( "fuzziness" );
      if ( fuzzy != null )
      {
        parseNewEntry.setFuzziness( Integer.parseInt( fuzzy ) );        
      }
     
      String doc = atts.getValue( "document" );
      if ( doc != null )
      {
        parseNewEntry.setMatchDocument( doc );        
      }
     
      String seg = atts.getValue( "segnumber" );
      if ( seg != null )
      {
        parseNewEntry.setSegmentNumberMatch( Integer.parseInt( seg ) );        
      }
     
      String markup = atts.getValue( "markup" );
      if ( markup != null )
      {
        parseNewEntry.setMatchMarkup( markup );        
      }
      
      String memory = atts.getValue( "memory" );
      if ( memory != null )
      {
        parseNewEntry.setMatchMemory( memory );        
      }
    }
    else if ( curElement.equals( "changelist" ) )
    {
      // start a new change list
      parseChangeList = new Vector();
    }
    else if ( curElement.equals( "change" ) )
    {
      // attributes: "type"
      String type = atts.getValue( "type" );
      if ( type == null ) type = "unknown";
      if ( type.equalsIgnoreCase( "changeto") )
      {
        // nothing to do, this type is implicetely set by previous change
      }
      else
      {
        parseChangeList.add( "#" + type + "#" );
      }
    }
  }

  
  // this method is called for characters
  public void characters( char[] ch, int start, int len )  
  { 
    String text = new String( ch, start, len );
    if ( parseCurData == null )
    {
      parseCurData = text;
    }
    else
    {
      parseCurData = parseCurData.concat( text );
    }
  }

  // this method is called when an element ends
  public void endElement( String namespace, String localName, final String type ) 
  {
    String curElement = type.toLowerCase();
    
    // process data containing elements
    if ( curElement.equals( "entry" ) )
    {
      if ( parseNewEntry != null )
      {
        matchListVector.add( parseNewEntry );
        parseNewEntry = null;
      }
    }        
    else if ( curElement.equals( "segment" ) )
    {
      if ( parseCurData != null )
      {
        parseNewEntry.setSegment( parseCurData );
      }
    }        
    else if ( curElement.equals( "source" ) )
    {
      if ( parseCurData != null )
      {
        parseNewEntry.setMatchSource( parseCurData );
      }
    }        
    else if ( curElement.equals( "target" ) )
    {
      if ( parseCurData != null )
      {
        parseNewEntry.setMatchTarget( parseCurData );
      }
    }        
    else if ( curElement.equals( "changelist" ) )
    {
      if ( parseChangeList != null )
      {
        parseNewEntry.setChanges( parseChangeList );
      }
    }        
    else if ( curElement.equals( "change" ) )
    {
      if ( (parseCurData != null ) && (parseChangeList != null ) )
      {
        parseChangeList.add( parseCurData );
      }
    }        
  }


}
