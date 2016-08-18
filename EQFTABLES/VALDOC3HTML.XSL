<!--
    Copyright (c) 1999-2015 International Business Machines
    Corporation and others.  All rights reserved.
-->

<!-- &nbsp; does not exists in XPath, so defined as the real unicode character 160
     which is the non breaking space character and provides exactly the same result than using &nbsp; -->
     
<!DOCTYPE xsl:stylesheet 
	[
		<!ENTITY nbsp "&#160;">
	]>

<!-- valdoc3tml.xsl

	Author: David Walters

	Purpose: Convert an XML validation document (translatable text only) to HTML
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- =========================================================================================== --> 	
<!-- == Date of Stylesheet (original): February 25, 2015                                      == -->
<!-- == Created by                   : David Walters                                          == -->
<!-- == Date of Stylesheet (update)  : April, 18, 2014                                        == -->
<!-- == Updated by                   : David Walters                                          == -->
<!-- == Purpose                      : Convert a validation document into the HTML format     == -->
<!-- == Version                      : 1.0                                                    == -->
<!-- =========================================================================================== -->

	<xsl:output method="html" encoding="utf-8"/>

	<xsl:template match="/">
		<html>
     		<head></head>
			<body>
			    <xsl:for-each select="/Proof/Document">
					<xsl:call-template name="document">
						<xsl:with-param name="doc" select="." />
						<xsl:with-param name="type" select="/Proof/@Type" />
					</xsl:call-template>
			    </xsl:for-each>
			</body>
		</html>
	</xsl:template>
	
<!-- convert a single document --> 
<xsl:template name = "document" >
	<xsl:param name="doc"/>
	<xsl:param name="type"/>
	<p><br/></p>
	<table border="0" cellpadding="0" width="100%">
	   <tbody>
		  <tr>
			 <td width="10%">Document:</td>
			 <td><xsl:value-of select="$doc/Header/DocumentName"/></td>
		  </tr>
		  <tr>
			 <td>Folder:</td>
			 <td><xsl:value-of select="$doc/Header/Folder"/></td>
		  </tr>
		  <tr>
			 <td>Date created:</td>
			 <td><xsl:value-of select="$doc/Header/CreationDate"/></td>
		  </tr>
		  <tr>
			 <td>Project:</td>
			 <td><xsl:value-of select="$doc/Header/Project"/></td>
		  </tr>
		  <tr>
			 <td>Project manager:</td>
			 <td><xsl:value-of select="$doc/Header/IBMPL"/></td>
		  </tr>
		  <tr>
			 <td>Translator:</td>
			 <td><xsl:value-of select="$doc/Header/Translator"/></td>
		  </tr>
		  <tr>
			 <td>Comments:</td>
			 <td>Translated text only</td>
		  </tr>
	   </tbody>
	</table>
    <br/>
	<table border="0" cellpadding="0" width="100%">
	   <tbody>
			<xsl:for-each select="$doc/SegmentList/Segment">
				<xsl:call-template name="proofsegment">
					<xsl:with-param name="seg" select="." />
				</xsl:call-template>
			</xsl:for-each>
	   </tbody>
	</table>
</xsl:template>

<!-- handle a single segment --> 
<xsl:template name = "proofsegment" >
	<xsl:param name="seg"/>
    <xsl:if test="$seg/@Match!='Protected'">
    <tr>
	  <td>[<xsl:value-of select="$seg/@Number"/>]</td>
	  <td><xsl:value-of select="$seg/Target"/></td>
    </tr>
    </xsl:if>
</xsl:template>



</xsl:stylesheet>
      
