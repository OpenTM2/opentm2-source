<?altova_samplexml i:\eqf\atest.f00\rpt.xml?>
<!--
    Copyright (c) 1999-2011, International Business Machines
    Corporation and others.  All rights reserved.
-->

<!-- &nbsp; does not exists in XPath, so defined as the real unicode character 160
     which is the non breaking space character and provides exactly the same result than using &nbsp; -->
     
<!DOCTYPE xsl:stylesheet 
	[
		<!ENTITY nbsp "&#160;">
	]>

<!-- CNTHTML.XSL

	Author: Gerhard Queck

	Purpose: Convert an XML counting report to HTML
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="html" encoding="utf-8"/>

	<xsl:template match="/">
		<html>
<!-- <head> required to put <META http-equiv="Content-Type" content="text/html; charset=utf-8">
     generated by output encoding attribute -->
     		<head></head>
			<body>
				<xsl:call-template name="reportheader" />
			    <xsl:for-each select="/tmwordcount/documents/document">
					<xsl:call-template name="document">
						<xsl:with-param name="document" select="."/>
					</xsl:call-template>
			    </xsl:for-each>
			</body>
		</html>
	</xsl:template>
	
<!-- create report header --> 
<xsl:template name = "reportheader" >
	<P><BR/></P>
	<CENTER>
	<TABLE border="1">
	   <TBODY>
		  <TR>
			 <TD bgcolor="#d3d3d3">Report:</TD>
			 <TD bgcolor="#f7f7f7"><xsl:value-of select="/tmwordcount/description/report/@type"/>&nbsp;<xsl:value-of select="/tmwordcount/description/report/@layout"/></TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">TM Version:</TD>
			 <TD bgcolor="#f7f7f7"><xsl:value-of select="/tmwordcount/description/translationManager/@version"/>.<xsl:value-of select="/tmwordcount/description/translationManager/@release"/></TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Calculation Profile:</TD>
			 <TD bgcolor="#f7f7f7"><xsl:value-of select="/tmwordcount/description/profile/@name"/></TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Report Generated at:</TD>
			 <TD bgcolor="#f7f7f7">
				 <xsl:value-of select="/tmwordcount/description/report/@generatedDate"/>
				 &nbsp;
				 <xsl:value-of select="/tmwordcount/description/report/@generatedTime"/>
			</TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Folder Name:</TD>
			 <TD bgcolor="#f7f7f7"><xsl:value-of select="/tmwordcount/description/folder/@nameLong"/></TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Documents in Folder:</TD>
			 <TD bgcolor="#f7f7f7"><xsl:value-of select="/tmwordcount/description/folder/@documentsAmount"/></TD>
		  </TR>
		  <TR>
			 <TD bgcolor="#d3d3d3">Documents selected:</TD>
			 <TD bgcolor="#f7f7f7"><xsl:value-of select="/tmwordcount/description/report/@documentsSelected"/></TD>
		  </TR>
	   </TBODY>
	</TABLE>
	</CENTER>
</xsl:template>

<!-- create report for one document --> 
<xsl:template name = "document" >
	<xsl:param name="document"></xsl:param>
	<P>
	<TABLE border="1">
	   <TBODY>
		  <TR>
			 <TD bgcolor="#d3d3d3">Document: </TD>
			 <TD bgcolor="#f7f7f7"><xsl:value-of select="$document/@nameLong"/></TD>
		  </TR>
		  <xsl:if test="$document/@nameLong != 'Summary'">
			  <TR>
				 <TD bgcolor="#d3d3d3">Date:</TD>
				 <TD bgcolor="#f7f7f7"><xsl:value-of select="$document/@date"/></TD>
			  </TR>
			  <TR>
				 <TD bgcolor="#d3d3d3">Last task:</TD>
				 <TD bgcolor="#f7f7f7"><xsl:value-of select="$document/@lastProcess"/></TD>
			  </TR>
		  </xsl:if>
	   </TBODY>
	</TABLE>
	<TABLE border="1" cellspacing="3">
	   <TBODY>
		  <TR>
			 <TD bgcolor="#d3d3d3" align="center">&nbsp;</TD>
			 <TD bgcolor="#d3d3d3" align="center">&nbsp;</TD>
			 <TD bgcolor="#d3d3d3" align="center">Segments</TD>
			 <TD bgcolor="#d3d3d3" align="center">Source<BR/>Words</TD>
			 <TD bgcolor="#d3d3d3" align="center">Modified<BR/>Words</TD>
			 <TD bgcolor="#d3d3d3" align="center">Target<BR/>Words</TD>
		  </TR>
			<xsl:for-each select="$document/wordcount">
					<xsl:call-template name="wordcounts">
						<xsl:with-param name="counts" select="." />
					</xsl:call-template>
			</xsl:for-each>
	   </TBODY>
	</TABLE>
	</P>
</xsl:template>

<!-- display all  count values of a group -->      
<xsl:template name = "wordcounts" >
	<xsl:param name="counts"></xsl:param>
	<xsl:param name="name"></xsl:param>
	<xsl:param name="id"></xsl:param>
	
		<tr align="right">
			<xsl:call-template name="wordcount">
				<xsl:with-param name="counts" select="$counts" />
				<xsl:with-param name="name" ><xsl:value-of select="$counts/@type"/></xsl:with-param>
				<xsl:with-param name="cat" >Simple</xsl:with-param>
			</xsl:call-template>
		</tr>
		
		<tr align="right">
			<xsl:call-template name="wordcount">
				<xsl:with-param name="counts" select="$counts" />
				<xsl:with-param name="name" ></xsl:with-param>
				<xsl:with-param name="cat" >Medium</xsl:with-param>
				<xsl:with-param name="id" ></xsl:with-param>
			</xsl:call-template>
		</tr>
		
		<tr align="right">
			<xsl:call-template name="wordcount">
				<xsl:with-param name="counts" select="$counts" />
				<xsl:with-param name="name" ></xsl:with-param>
				<xsl:with-param name="cat" >Complex</xsl:with-param>
			</xsl:call-template>
		</tr>
</xsl:template>
      
<!-- display the counts of a single category -->      
<xsl:template name = "wordcount" >
	<xsl:param name="counts"></xsl:param>
	<xsl:param name="name"></xsl:param>
	<xsl:param name="cat"></xsl:param>
	
    <xsl:if test="$name != ''">
      <td rowspan="3" align="left">
		  <xsl:choose>
		      <xsl:when test="$name = 'analysisAutosubst'">Analysis<br/>Autosubst</xsl:when>
		      <xsl:when test="$name = 'editAutosubst'">Edit<br/>Autosubst</xsl:when>
		      <xsl:when test="$name = 'exact'">Exact<br/>Matches</xsl:when>
		      <xsl:when test="$name = 'replace'">Replace<br/>Matches</xsl:when>
		      <xsl:when test="$name = 'fuzzy'">Fuzzy<br/>Matches</xsl:when>
		      <xsl:when test="$name = 'machine'">Machine<br/>Matches</xsl:when>
		      <xsl:when test="$name = 'nomatch'">None<br/>Matches</xsl:when>
		      <xsl:when test="$name = 'notTranslated'">Not<br/>Translated</xsl:when>
		      <xsl:when test="$name = 'summary'">Summary</xsl:when>
			  <xsl:otherwise><xsl:value-of select="$name"/></xsl:otherwise>
		  </xsl:choose>
      
      </td>
    </xsl:if>	
    
	<!-- category text -->
	<td>		
		<xsl:choose>
			<xsl:when test="$cat = 'Simple'">1..4</xsl:when>
			<xsl:when test="$cat = 'Medium'">5..14</xsl:when>
			<xsl:when test="$cat = 'Complex'">&gt;14</xsl:when>
			<xsl:otherwise></xsl:otherwise>
		</xsl:choose>
		&nbsp;
	</td>

	<!-- the count values -->
	<td><xsl:value-of select="$counts/words[@category=$cat and @type='segments']/@count" />&nbsp;</td>
	<td><xsl:value-of select="$counts/words[@category=$cat and @type='sourceWords']/@count" />&nbsp;</td>
	<td><xsl:value-of select="$counts/words[@category=$cat and @type='modWords']/@count" />&nbsp;</td>
	<td><xsl:value-of select="$counts/words[@category=$cat and @type='targetWords']/@count" />&nbsp;</td>
	
</xsl:template>

</xsl:stylesheet>
