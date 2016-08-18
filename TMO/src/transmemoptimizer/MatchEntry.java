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
public class MatchEntry
{
  /** selected flag */
  private boolean selected;
  
  
  /** string containing the segment data */
  private String strSegment;
  
  /** string containing the match */ 
  private String matchSource;
  
  /** string containing the match */ 
  private String matchTarget;
  
  /** string containing the markup table */
  private String strMarkup;
  
  /** string containing the document name */
  private String strDocument;
  
  /**  string containing the match document name */
  private String strMatchDocument;

  /** segment number */
  private int segmentNumber;
  
  /** segment number of match*/
  private int segmentNumberMatch;
  
  /** fuzziness of match */
  private int fuzziness;
  
  /** memory of match */
  private String matchMemory;
  
  /** memory of match */
  private String matchMarkup;
  
  /** list of changes required to convert match to segment string */
  private Vector changes;
  
  public MatchEntry()
  {
    this.selected = false;
    this.segmentNumber = 0;
    this.segmentNumberMatch = 0;
    this.changes = new Vector();
  }
  
  /**
   * @return Returns the strDocument.
   */
  public String getDocument()
  {
    return strDocument;
  }
  /**
   * @param Document The Document to set.
   */
  public void setDocument( String strDocument )
  {
    this.strDocument = strDocument;
  }
  /**
   * @return Returns the Markup.
   */
  public String getMarkup()
  {
    return strMarkup;
  }
  /**
   * @param strMarkup The Markup to set.
   */
  public void setMarkup( String strMarkup )
  {
    this.strMarkup = strMarkup;
  }
  /**
   * @return Returns the document of the match
   */
  public String getMatchDocument()
  {
    return strMatchDocument;
  }
  /**
   * @param strMatchDocument The document of the match.
   */
  public void setMatchDocument( String strMatchDocument )
  {
    this.strMatchDocument = strMatchDocument;
  }
  /**
   * @return Returns the segment.
   */
  public String getSegment()
  {
    return strSegment;
  }
  /**
   * @param strSegment The segment to set.
   */
  public void setSegment( String strSegment )
  {
    this.strSegment = strSegment;
  }
  /**
   * @return Returns the segmentNumber.
   */
  public int getSegmentNumber()
  {
    return segmentNumber;
  }
  /**
   * @param segmentNumber The segmentNumber to set.
   */
  public void setSegmentNumber( int segmentNumber )
  {
    this.segmentNumber = segmentNumber;
  }
  /**
   * @return Returns the segmentNumberMatch.
   */
  public int getSegmentNumberMatch()
  {
    return segmentNumberMatch;
  }
  /**
   * @param segmentNumberMatch The segmentNumberMatch to set.
   */
  public void setSegmentNumberMatch( int segmentNumberMatch )
  {
    this.segmentNumberMatch = segmentNumberMatch;
  }
  /**
   * @return Returns the selected.
   */
  public boolean isSelected()
  {
    return selected;
  }
  /**
   * @param selected The selected to set.
   */
  public void setSelected( boolean selected )
  {
    this.selected = selected;
  }
  /**
   * @return Returns the fuzziness.
   */
  public int getFuzziness()
  {
    return fuzziness;
  }
  /**
   * @param fuzziness The fuzziness to set.
   */
  public void setFuzziness( int fuzziness )
  {
    this.fuzziness = fuzziness;
  }
  /**
   * @return Returns the matchMemory.
   */
  public String getMatchMemory()
  {
    return matchMemory;
  }
  /**
   * @param matchMemory The matchMemory to set.
   */
  public void setMatchMemory( String matchMemory )
  {
    this.matchMemory = matchMemory;
  }
  /**
   * @return Returns the matchMarkup.
   */
  public String getMatchMarkup()
  {
    return matchMarkup;
  }
  /**
   * @param matchMarkup The matchMarkup to set.
   */
  public void setMatchMarkup( String matchMarkup )
  {
    this.matchMarkup = matchMarkup;
  }
  /**
   * @return Returns the matchTarget.
   */
  public String getMatchTarget()
  {
    return matchTarget;
  }
  /**
   * @param matchTarget The matchTarget to set.
   */
  public void setMatchTarget( String matchTarget )
  {
    this.matchTarget = matchTarget;
  }
  /**
   * @return Returns the matchSource.
   */
  public String getMatchSource()
  {
    return matchSource;
  }
  /**
   * @param matchSource The matchSource to set.
   */
  public void setMatchSource( String matchSource )
  {
    this.matchSource = matchSource;
  }
  /**
   * @return Returns the changes.
   */
  public Vector getChanges()
  {
    return changes;
  }
  /**
   * @param changes The changes to set.
   */
  public void setChanges( Vector changes )
  {
    this.changes = changes;
  }
}
