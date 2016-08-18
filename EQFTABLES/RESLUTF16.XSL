<?altova_samplexml i:\eqf\reptest.f00\rpt.xml?>
<!--
    Copyright (c) 1999-2014, International Business Machines
    Corporation and others.  All rights reserved.
-->

<!DOCTYPE xsl:stylesheet 
	[
		<!ENTITY nbsp "&#160;">
		<!ENTITY nl "&#xa;"><!--new line-->
	]>

<!-- CNTTEXT.XSL

	Author: Gerhard Queck

	Purpose: Convert an XML redundant segment list into text format
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="text" encoding="UTF-16" />

	<xsl:template match="/">
		<xsl:call-template name="reportheader" />
		<xsl:for-each select="/tmredsegments/redundantEntry">
			<xsl:call-template name="redentry">
				<xsl:with-param name="entry" select="."/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:template>
	
<!-- create report header --> 
<xsl:template name = "reportheader">
	<xsl:text>&nl;Report: </xsl:text><xsl:value-of select="/tmredsegments/description/report/@type"/> <xsl:value-of select="/tmredsegments/description/report/@layout"/>
	<xsl:text>&nl;TM Version: </xsl:text><xsl:value-of select="/tmredsegments/description/translationManager/@version"/>.<xsl:value-of select="/tmredsegments/description/translationManager/@release"/>
	<xsl:text>&nl;Calculation Profile: </xsl:text><xsl:value-of select="/tmredsegments/description/profile/@name"/>
	<xsl:text>&nl;Report Generated at: </xsl:text><xsl:value-of select="/tmredsegments/description/report/@generatedDate"/><xsl:value-of select="/tmredsegments/description/report/@generatedTime"/>
	<xsl:text>&nl;Folder Name: </xsl:text><xsl:value-of select="/tmredsegments/description/folder/@nameLong"/>
	<xsl:text>&nl;Documents in Folder: </xsl:text><xsl:value-of select="/tmredsegments/description/folder/@documentsAmount"/>
	<xsl:text>&nl;Documents selected: </xsl:text><xsl:value-of select="/tmredsegments/description/report/@documentsSelected"/>
	<xsl:text>&nl;&nl;</xsl:text>
</xsl:template>

<!-- create output for on eentry --> 
<xsl:template name = "redentry" >
	<xsl:param name="entry"></xsl:param>
			<xsl:text>&nl;Entry </xsl:text><xsl:value-of select="$entry/@num"/><xsl:text>: Frequency: </xsl:text><xsl:value-of select="$entry/@frequency"/><xsl:text>&nl;</xsl:text>
		  <xsl:for-each select="$entry/document">
				<xsl:text>[</xsl:text><xsl:value-of select="position()"/><xsl:text>] </xsl:text>
				<xsl:value-of select="./@folderobjname"/><xsl:text>\</xsl:text><xsl:value-of select="./@docname"/>
				<xsl:text> : #</xsl:text>
				<xsl:value-of select="./@segnum"/>
				<xsl:text>&nl;</xsl:text>
		  </xsl:for-each>
		<xsl:text>Segment: </xsl:text>
		<xsl:value-of select="$entry/segment"/>
		<xsl:text>&nl;&nl;====================================================================================================================</xsl:text>
</xsl:template>

</xsl:stylesheet>
