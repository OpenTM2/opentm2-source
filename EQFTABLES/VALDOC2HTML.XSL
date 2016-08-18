<!--
    Copyright (c) 1999-2014, International Business Machines
    Corporation and others.  All rights reserved.
-->

<!-- &nbsp; does not exists in XPath, so defined as the real unicode character 160
     which is the non breaking space character and provides exactly the same result than using &nbsp; -->
     
<!DOCTYPE xsl:stylesheet 
	[
		<!ENTITY nbsp "&#160;">
	]>

<!-- valdoc2html.xsl

	Author: Gerhard Queck

	Purpose: Convert an XML validation document to HTML
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- =========================================================================================== --> 	
<!-- == Date of Stylesheet (original): January 01, 2005                                       == -->
<!-- == Created by                   : Gerhard Queck                                          == -->
<!-- == Date of Stylesheet (update)  : April, 18, 2014                                        == -->
<!-- == Updated by                   : Gerhard Queck                                          == -->
<!-- == Purpose                      : Convert a validation document into the HTML format     == -->
<!-- == Version                      : 3.0                                                    == -->
<!-- =========================================================================================== -->

	<xsl:output method="html" encoding="utf-8"/>

	<xsl:template match="/">
		<html>
     		<head></head>
			<body>
			    <xsl:if test="/Proof/@Type='Validation'">
					<xsl:call-template name="instructions"/>
			    </xsl:if>
			    <xsl:for-each select="/Proof/Document">
					<xsl:call-template name="document">
						<xsl:with-param name="doc" select="." />
						<xsl:with-param name="type" select="/Proof/@Type" />
					</xsl:call-template>
			    </xsl:for-each>
			</body>
		</html>
	</xsl:template>

<xsl:template name = "instructions" >
	<P><BR/></P>
	<TABLE border="1" cellpadding="6" width="100%">
	  <TR>
		 <TD>
			<xsl:for-each select="/Proof/Instructions/Line">
			   <xsl:value-of select="."/>&nbsp;<br/>
			</xsl:for-each>
		 </TD>
	  </TR>
    </TABLE>  
</xsl:template>

	
<!-- convert a single document --> 
<xsl:template name = "document" >
	<xsl:param name="doc"/>
	<xsl:param name="type"/>
	<P><BR/></P>
	<TABLE border="1" cellpadding="6" width="100%">
	   <TBODY>
		  <TR>
			 <TD width="15%" bgcolor="#d3d3d3">Document:</TD>
			 <TD><xsl:value-of select="$doc/Header/DocumentName"/>&nbsp;</TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Folder:</TD>
			 <TD><xsl:value-of select="$doc/Header/Folder"/>&nbsp;</TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Date created:</TD>
			 <TD><xsl:value-of select="$doc/Header/CreationDate"/>&nbsp;</TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Project:</TD>
			 <TD><xsl:value-of select="$doc/Header/Project"/>&nbsp;</TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Project manager:</TD>
			 <TD><xsl:value-of select="$doc/Header/IBMPL"/>&nbsp;</TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Translator:</TD>
			 <TD><xsl:value-of select="$doc/Header/Translator"/>&nbsp;</TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Validator:</TD>
			 <TD>&nbsp;</TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Comments:</TD>
			 <TD>&nbsp;</TD>
		  </TR>
	   </TBODY>
	</TABLE>
	<TABLE border="1" cellpadding="6" width="100%">
	   <tr>
		<xsl:if test="$type='Validation'">
			<th bgcolor="#d3d3d3">Seg<br/>#</th>
			<th bgcolor="#d3d3d3">%<br/>Match<br/>Ratio</th>
 		        <xsl:if test="$doc/SegmentList/Segment[1]/@TaxoCode!=''">
				<th bgcolor="#d3d3d3">Taxonomy<br/>Code</th>
	                </xsl:if>
			<th bgcolor="#d3d3d3">Source</th>
			<th bgcolor="#d3d3d3">Translation</th>
		</xsl:if>
		<xsl:if test="$type!='Validation'">
			<th bgcolor="#d3d3d3">Seg<br/>#</th>
	        <xsl:if test="$doc/SegmentList/Segment[1]/@ExistMatch!=''">
				<th bgcolor="#d3d3d3">Existing<br/>Match</th>
            </xsl:if>
            <th bgcolor="#d3d3d3">
				<xsl:if test="$doc/SegmentList/Segment[1]/@ExistMatch!=''">
					Used<br/>
				</xsl:if>
			Match
			</th>
			<th bgcolor="#d3d3d3">%<br/>Match<br/>Ratio</th>
	        <xsl:if test="$doc/SegmentList/Segment[1]/@WordCount!=''">
				<th bgcolor="#d3d3d3">Word<br/>Count</th>
            </xsl:if>
	        <xsl:if test="$doc/SegmentList/Segment[1]/@TaxoCode!=''">
				<th bgcolor="#d3d3d3">Taxonomy<br/>Code</th>
            </xsl:if>
			<th bgcolor="#d3d3d3">Source</th>
			<th bgcolor="#d3d3d3">Translation</th>
		</xsl:if>
	   </tr>	   
	   <TBODY>
		<xsl:if test="$type='Validation'">
			<xsl:for-each select="$doc/SegmentList/Segment">
				<xsl:call-template name="valsegment">
					<xsl:with-param name="seg" select="." />
				</xsl:call-template>
			</xsl:for-each>
		</xsl:if>
		<xsl:if test="$type!='Validation'">
			<xsl:for-each select="$doc/SegmentList/Segment">
				<xsl:call-template name="proofsegment">
					<xsl:with-param name="seg" select="." />
				</xsl:call-template>
			</xsl:for-each>
		</xsl:if>
	   </TBODY>
	</TABLE>
</xsl:template>

<!-- handle a single segment --> 
<xsl:template name = "valsegment" >
	<xsl:param name="seg"/>
    <TR>
	  <TD align="center"><xsl:value-of select="$seg/@Number"/></TD>
	      <xsl:choose>
			<xsl:when test="$seg/@Match='NotTranslated'">
        	  <TD align="center"><xsl:value-of select="$seg/@Fuzziness"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='ModifiedAutoSubst'">
        	  <TD align="center" bgcolor="Gold"><xsl:value-of select="$seg/@Fuzziness"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='ExactFromManual'">
        	  <TD align="center" bgcolor="Gold"><xsl:value-of select="$seg/@Fuzziness"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='ModifiedExact'">
        	  <TD align="center" bgcolor="Gold"><xsl:value-of select="$seg/@Fuzziness"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Fuzziness='100'">
        	  <TD align="center" bgcolor="Lime"><xsl:value-of select="$seg/@Fuzziness"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Fuzziness='0'">
        	  <TD align="center" bgcolor="red"><xsl:value-of select="$seg/@Fuzziness"/></TD>
			</xsl:when>
			<xsl:otherwise>
        	  <TD align="center" bgcolor="Gold"><xsl:value-of select="$seg/@Fuzziness"/></TD>
			</xsl:otherwise>
		  </xsl:choose>
	  <xsl:if test="$seg/@TaxoCode!=''">
		  <TD align="center" ><xsl:value-of select="$seg/@TaxoCode"/>&nbsp;</TD>
	  </xsl:if>
	  <TD><xsl:value-of select="$seg/Source"/></TD>
	  <TD><xsl:value-of select="$seg/Target"/></TD>
    </TR>
</xsl:template>

<!-- handle a single segment --> 
<xsl:template name = "proofsegment" >
	<xsl:param name="seg"/>
    <TR>
	  <TD align="center"><xsl:value-of select="$seg/@Number"/></TD>
	  <xsl:if test="$seg/@ExistMatch!=''">
	      <xsl:choose>
			<xsl:when test="$seg/@ExistMatch='No proposal'">
        	  <TD align="center" bgcolor="red"><xsl:value-of select="$seg/@ExistMatch"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@ExistMatch='Fuzzy'">
        	  <TD align="center" bgcolor="Gold"><xsl:value-of select="$seg/@ExistMatch"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@ExistMatch='Exact'">
    	      <xsl:choose>
        	    <xsl:when test="$seg/@Match='ExactFromManual'">
            	  <TD align="center" bgcolor="Gold">Exact<br/>From New</TD>
				</xsl:when>
				<xsl:otherwise>
            	  <TD align="center" bgcolor="Lime"><xsl:value-of select="$seg/@ExistMatch"/></TD>
				</xsl:otherwise>
    		  </xsl:choose>
			</xsl:when>
			<xsl:when test="$seg/@ExistMatch='GlobalMemory'">
        	  <TD align="center" bgcolor="Lime"><xsl:value-of select="$seg/@ExistMatch"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@ExistMatch='Machine'">
        	  <TD align="center" bgcolor="Orange"><xsl:value-of select="$seg/@ExistMatch"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@ExistMatch='Exist'">
        	  <TD align="center" bgcolor="Lime"><xsl:value-of select="$seg/@ExistMatch"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@ExistMatch='Replace'">
        	  <TD align="center" bgcolor="Darkorange"><xsl:value-of select="$seg/@ExistMatch"/></TD>
			</xsl:when>
			<xsl:otherwise>
        	  <TD><xsl:value-of select="$seg/@ExistMatch"/></TD>
			</xsl:otherwise>
		  </xsl:choose>
	  </xsl:if>
	      <xsl:choose>
			<xsl:when test="$seg/@Match='New'">
        	  <TD align="center" bgcolor="red"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='Fuzzy'">
        	  <TD align="center" bgcolor="Gold"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='ExactFromManual'">
        	  <TD align="center" bgcolor="Gold">Exact<br/>From New</TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='Exact'">
        	  <TD align="center" bgcolor="Lime"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='GlobalMemoryModified'">
        	  <TD align="center" bgcolor="Gold"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='GlobalMemory'">
        	  <TD align="center" bgcolor="Lime"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='ModifiedExact'">
        	  <TD align="center" bgcolor="Gold">Modified<br/>Exact</TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='ModifiedAutoSubst'">
        	  <TD align="center" bgcolor="Gold">Modified<br/>AutoSubst</TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='Machine'">
        	  <TD align="center" bgcolor="Orange"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='Protected'">
        	  <TD align="center" bgcolor="LightGrey"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='AutoSubst'">
        	  <TD align="center" bgcolor="Lime"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='NotTranslated'">
        	  <TD align="center" bgcolor="DeepSkyBlue"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:when test="$seg/@Match='Replace'">
        	  <TD align="center" bgcolor="Darkorange"><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:when>
			<xsl:otherwise>
        	  <TD><xsl:value-of select="$seg/@Match"/></TD>
			</xsl:otherwise>
		  </xsl:choose>
	  <TD align="center" ><xsl:value-of select="$seg/@Fuzziness"/>&nbsp;</TD>
	  <xsl:if test="$seg/@WordCount!=''">
       	  <TD align="center" > <xsl:value-of select="$seg/@WordCount"/></TD>
	  </xsl:if>
	  <xsl:if test="$seg/@TaxoCode!=''">
		  <TD align="center" ><xsl:value-of select="$seg/@TaxoCode"/>&nbsp;</TD>
	  </xsl:if>
	  <TD><xsl:value-of select="$seg/Source"/></TD>
	  <TD><xsl:value-of select="$seg/Target"/></TD>
    </TR>
</xsl:template>



</xsl:stylesheet>
      
