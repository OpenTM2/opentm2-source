<?altova_samplexml i:\eqf\reptest.f00\rpt.xml?>
<!--
    Copyright (c) 1999-2011, International Business Machines
    Corporation and others.  All rights reserved.
-->

<!DOCTYPE xsl:stylesheet 
	[
		<!ENTITY nbsp "&#160;">
		<!ENTITY nl "&#xa;"><!--new line-->
	]>

<!-- CNTTEXT.XSL

	Author: Gerhard Queck

	Purpose: Convert an XML counting report into plain text format
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="text"/>

	<xsl:template match="/">
		<xsl:call-template name="reportheader" />
		<xsl:for-each select="/tmwordcount/documents/document">
			<xsl:call-template name="document">
				<xsl:with-param name="document" select="."/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:template>
	
<!-- create report header --> 
<xsl:template name = "reportheader" >
	<xsl:text>&nl;Report                                  : </xsl:text>
	<xsl:value-of select="/tmwordcount/description/report/@type"/> 
	<xsl:text>   </xsl:text>
	<xsl:value-of select="/tmwordcount/description/report/@layout"/>
	
	<xsl:text>&nl;TM Version                              : </xsl:text>
	<xsl:value-of select="/tmwordcount/description/translationManager/@version"/>
	<xsl:text>.</xsl:text>
	<xsl:value-of select="/tmwordcount/description/translationManager/@release"/>
	
	<xsl:text>&nl;Calculation Profile                     : </xsl:text>
	<xsl:value-of select="/tmwordcount/description/profile/@name"/>
	
	<xsl:text>&nl;Report Generated at                     : </xsl:text>
	<xsl:value-of select="/tmwordcount/description/report/@generatedDate"/>
	<xsl:text>   </xsl:text>
	<xsl:value-of select="/tmwordcount/description/report/@generatedTime"/>
	
	<xsl:text>&nl;Folder Name                             : </xsl:text><xsl:value-of select="/tmwordcount/description/folder/@nameLong"/>
	<xsl:text>&nl;Documents in Folder                     : </xsl:text><xsl:value-of select="/tmwordcount/description/folder/@documentsAmount"/>
	<xsl:text>&nl;Documents selected                      : </xsl:text><xsl:value-of select="/tmwordcount/description/report/@documentsSelected"/>
	<xsl:text>&nl;&nl;</xsl:text>
</xsl:template>

<!-- create report for one document --> 
<xsl:template name = "document" >
	<xsl:param name="document"></xsl:param>
	<xsl:text>&nl;&nl;&nl;Document      : </xsl:text>
	<xsl:value-of select="$document/@nameLong"/>
		  
	<xsl:if test="$document/@nameLong != 'Summary'">
		<xsl:text>&nl;Date          : </xsl:text>
		<xsl:value-of select="$document/@date"/>
		<xsl:text>&nl;Last task     : </xsl:text>
		<xsl:value-of select="$document/@lastProcess"/>
	</xsl:if>
	
	<!-- header -->
	<xsl:text>&nl;===========================================================================</xsl:text>
	<xsl:text>&nl;|            |        | Segments   | Source     | Modified   | Target     |</xsl:text>
	<xsl:text>&nl;|            |        |            | Words      | Words      | Words      |</xsl:text>
	<xsl:text>&nl;===========================================================================</xsl:text>

	<!-- counts -->
	<xsl:for-each select="$document/wordcount">
			<xsl:call-template name="wordcounts">
				<xsl:with-param name="counts" select="." />
			</xsl:call-template>
	</xsl:for-each>
</xsl:template>

<!-- display all  count values of a group -->      
<xsl:template name = "wordcounts" >
	<xsl:param name="counts"></xsl:param>
	<xsl:param name="name"></xsl:param>
	<xsl:param name="id"></xsl:param>

    <xsl:variable name="text1">
	  <xsl:choose>
		  <xsl:when test="$counts/@type = 'analysisAutosubst'">Analysis</xsl:when>
		  <xsl:when test="$counts/@type = 'editAutosubst'">Edit</xsl:when>
		  <xsl:when test="$counts/@type = 'exact'">Exact</xsl:when>
		  <xsl:when test="$counts/@type = 'replace'">Replace</xsl:when>
		  <xsl:when test="$counts/@type = 'fuzzy'">Fuzzy</xsl:when>
		  <xsl:when test="$counts/@type = 'machine'">Machine</xsl:when>
		  <xsl:when test="$counts/@type = 'nomatch'">None</xsl:when>
		  <xsl:when test="$counts/@type = 'notTranslated'">Not</xsl:when>
		  <xsl:when test="$counts/@type = 'summary'"></xsl:when>
		  <xsl:otherwise><xsl:value-of select="$counts/@type"/></xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>	
    
	<xsl:call-template name="wordcount">
		<xsl:with-param name="counts" select="$counts" />
		<xsl:with-param name="name" ><xsl:value-of select="$text1"/></xsl:with-param>
		<xsl:with-param name="cat" >Simple</xsl:with-param>
	</xsl:call-template>
		
    <xsl:variable name="text2">
	  <xsl:choose>
		  <xsl:when test="$counts/@type = 'analysisAutosubst'">Autosubst</xsl:when>
		  <xsl:when test="$counts/@type = 'editAutosubst'">Autosubst</xsl:when>
		  <xsl:when test="$counts/@type = 'exact'">Matches</xsl:when>
		  <xsl:when test="$counts/@type = 'replace'">Matches</xsl:when>
		  <xsl:when test="$counts/@type = 'fuzzy'">Matches</xsl:when>
		  <xsl:when test="$counts/@type = 'machine'">Matches</xsl:when>
		  <xsl:when test="$counts/@type = 'nomatch'">Matches</xsl:when>
		  <xsl:when test="$counts/@type = 'notTranslated'">Translated</xsl:when>
		  <xsl:when test="$counts/@type = 'summary'">Summary</xsl:when>
		  <xsl:otherwise><xsl:value-of select="$counts/@type"/></xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>	
    
		
	<xsl:call-template name="wordcount">
		<xsl:with-param name="counts" select="$counts" />
		<xsl:with-param name="name" ><xsl:value-of select="$text2"/></xsl:with-param>
		<xsl:with-param name="cat" >Medium</xsl:with-param>
		<xsl:with-param name="id" ></xsl:with-param>
	</xsl:call-template>
		
		
	<xsl:call-template name="wordcount">
		<xsl:with-param name="counts" select="$counts" />
		<xsl:with-param name="name" ></xsl:with-param>
		<xsl:with-param name="cat" >Complex</xsl:with-param>
	</xsl:call-template>

	<xsl:text>&nl;===========================================================================</xsl:text>
		
</xsl:template>
      
<!-- display the counts of a single category -->      
<xsl:template name = "wordcount" >
	<xsl:param name="counts"></xsl:param>
	<xsl:param name="name"></xsl:param>
	<xsl:param name="cat"></xsl:param>
	
    <xsl:text>&nl;| </xsl:text>
    
	<!-- name -->
	<xsl:call-template name="textcolumn">
		<xsl:with-param name="value" select="$name" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
    
	<!-- category text -->
	<xsl:choose>
		<xsl:when test="$cat = 'Simple'">
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="value">1..4</xsl:with-param>
				<xsl:with-param name="width">6</xsl:with-param>
			</xsl:call-template>
		</xsl:when>
		<xsl:when test="$cat = 'Medium'">
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="value">5..14</xsl:with-param>
				<xsl:with-param name="width">6</xsl:with-param>
			</xsl:call-template>
		</xsl:when>
		<xsl:when test="$cat = 'Complex'">
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="value">14..</xsl:with-param>
				<xsl:with-param name="width">6</xsl:with-param>
			</xsl:call-template>
		</xsl:when>
		<xsl:otherwise>
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="value"></xsl:with-param>
				<xsl:with-param name="width">6</xsl:with-param>
			</xsl:call-template>
		</xsl:otherwise>
	</xsl:choose>
	

	<!-- the count values -->
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='segments']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='sourceWords']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='modWords']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='targetWords']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	
</xsl:template>

<!-- format a numerical column value to a given width  -->      
<xsl:template name = "numcolumn" >
	<xsl:param name="value"></xsl:param>
	<xsl:param name="width"></xsl:param>
	
	<xsl:variable name="filledcolumn" select="concat('               ',$value)"/>
	<xsl:variable name="pos" select="string-length($filledcolumn) - $width + 1"/>
	<xsl:value-of select="substring($filledcolumn,$pos,$width)"/>
	<xsl:text> | </xsl:text>
</xsl:template>

<!-- format a text column value to a given width  -->      
<xsl:template name = "textcolumn" >
	<xsl:param name="value"></xsl:param>
	<xsl:param name="width"></xsl:param>
	
	<xsl:variable name="filledcolumn" select="concat($value,'                             ')"/>
	<xsl:value-of select="substring($filledcolumn,1,$width)"/>
	<xsl:text> | </xsl:text>
</xsl:template>



</xsl:stylesheet>
