<?altova_samplexml i:\eqf\reptest.f00\rpt.xml?>
<!--
    Copyright (c) 1999-2011, International Business Machines
    Corporation and others.  All rights reserved.
-->

<!DOCTYPE xsl:stylesheet [
	<!ENTITY nbsp "&#160;">
		<!ENTITY nl "&#xa;"><!--new line-->
]>
<!-- HISTTEXT.XSL

	Author: Gerhard Queck

	Purpose: Convert XML history report into text format
 -->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="html" encoding="utf-8"/>

	<xsl:template match="/">
	
		<xsl:call-template name="reportheader"/>
		<xsl:choose>
			<xsl:when test="/tmhistory/description/report/@layout = 'Detail'">
				<xsl:call-template name="detailedhistory"/>
			</xsl:when>
			<xsl:when test="/tmhistory/description/report/@layout = 'Versionl'">
				<xsl:call-template name="detailedhistory"/>
			</xsl:when>
			<xsl:when test="/tmhistory/description/report/@layout = 'BriefSortByDocument'">
				<xsl:call-template name="summary"/>
			</xsl:when>
			<xsl:when test="/tmhistory/description/report/@layout = 'BriefSortByDate'">
				<xsl:call-template name="summary"/>
			</xsl:when>
		</xsl:choose>
		<xsl:if test="/tmhistory/description/report/@withDocumentList='Yes'">
		  <xsl:call-template name="documentlist"/>
		</xsl:if>
	</xsl:template>
	
	<!-- create report header -->
	<xsl:template name="reportheader">
		<xsl:text>&nl;Report                                  : </xsl:text>
		<xsl:value-of select="/tmhistory/description/report/@type"/> 
		<xsl:text>   </xsl:text>
		<xsl:value-of select="/tmhistory/description/report/@layout"/>
		
		<xsl:text>&nl;TM Version                              : </xsl:text>
		<xsl:value-of select="/tmhistory/description/translationManager/@version"/>
		<xsl:text>.</xsl:text>
		<xsl:value-of select="/tmhistory/description/translationManager/@release"/>
		
		<xsl:text>&nl;Calculation Profile                     : </xsl:text>
		<xsl:value-of select="/tmhistory/description/profile/@name"/>
		
		<xsl:text>&nl;Report Generated at                     : </xsl:text>
		<xsl:value-of select="/tmhistory/description/report/@generatedDate"/>
		<xsl:text>   </xsl:text>
		<xsl:value-of select="/tmhistory/description/report/@generatedTime"/>
		
		<xsl:text>&nl;Folder Name                             : </xsl:text><xsl:value-of select="/tmhistory/description/folder/@nameLong"/>
		<xsl:text>&nl;Documents in Folder                     : </xsl:text><xsl:value-of select="/tmhistory/description/folder/@documentsAmount"/>
		<xsl:text>&nl;Documents selected                      : </xsl:text><xsl:value-of select="/tmhistory/description/report/@documentsSelected"/>
		<xsl:text>&nl;&nl;</xsl:text>
	</xsl:template>

	<!-- create summarized history -->
	<xsl:template name="summary">
		<xsl:text>&nl;=============================================================================================================</xsl:text>
		<xsl:text>&nl;| Task Num | Document                             | Date/Time              | Processed Task                 |</xsl:text>
		<xsl:text>&nl;=============================================================================================================</xsl:text>
		<xsl:for-each select="/tmhistory/summary/histlogRecord">
			<xsl:text>&nl;| </xsl:text>
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="value" select="./header/@num" />
				<xsl:with-param name="width">8</xsl:with-param>
			</xsl:call-template>
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="value" select="./header/@nameLong" />
				<xsl:with-param name="width">36</xsl:with-param>
			</xsl:call-template>
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="value" select="./header/@time" />
				<xsl:with-param name="width">22</xsl:with-param>
			</xsl:call-template>
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="value" select="./header/@task" />
				<xsl:with-param name="width">30</xsl:with-param>
			</xsl:call-template>
		</xsl:for-each>
		<xsl:text>&nl;=============================================================================================================</xsl:text>
	</xsl:template>
	
	
	<!-- create detailed history -->
	<xsl:template name="detailedhistory">
		<xsl:for-each select="/tmhistory/histlogRecord">
			<xsl:call-template name="histlogrecord">
				<xsl:with-param name="header" select="./header"/>
				<xsl:with-param name="data" select="./data"/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:template>
	
	
	<!-- display a single history log record -->
	<xsl:template name="histlogrecord">
		<xsl:param name="header"/>
		<xsl:param name="data"/>
		
		<xsl:text>&nl;Record  </xsl:text>
		<xsl:value-of select="$header/@num"/> 
		<xsl:text>  : </xsl:text>
		<xsl:value-of select="$header/@task"/> 
		
		<xsl:text>&nl;Date                        : </xsl:text>
		<xsl:value-of select="$header/@time"/>
		
		<xsl:text>&nl;Document                    : </xsl:text>
		<xsl:value-of select="$header/@name"/>
		
		<xsl:if test="$header/@path !='' ">
			<xsl:text>&nl;Document path               : </xsl:text>
			<xsl:value-of select="$header/@path"/>
		</xsl:if>
		<xsl:text>&nl;--------------------------------------------------------------------------------------------------------</xsl:text>
		<xsl:choose>
			<xsl:when test="$data/@type = 'DocImport'">
				<xsl:call-template name="import">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'Analysis'">
				<xsl:call-template name="analysis">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'AutomaticSubst3'">
				<xsl:call-template name="autosubst">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'AutomaticSubst'">
				<xsl:call-template name="autosubst">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocExport'">
				<xsl:call-template name="docexport">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocSave3'">
				<xsl:call-template name="docsave">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocSave'">
				<xsl:call-template name="docsave">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocSave2'">
				<xsl:call-template name="docsave">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocImpNewTarget'">
				<xsl:call-template name="docsave">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocImpNewTarget2'">
				<xsl:call-template name="docsave">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocImpNewTarget3'">
				<xsl:call-template name="docsave">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocApi'">
				<xsl:call-template name="docsave">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocApi3'">
				<xsl:call-template name="docsave">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'DocProp'">
				<xsl:call-template name="docprop">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'Version'">
				<xsl:call-template name="version">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'FolProp'">
				<xsl:call-template name="folprop">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'FolPropShipment'">
				<xsl:call-template name="folpropshipment">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$data/@type = 'Version'">
				<xsl:call-template name="version">
					<xsl:with-param name="data" select="$data"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise/>
		</xsl:choose>
		<xsl:text>&nl;&nl;</xsl:text>
		
	</xsl:template>
	
	<!-- display history data of doc prop record -->
	<xsl:template name="docprop">
		<xsl:param name="data"/>
		
		<xsl:if test="$data/markup != '' ">
			<xsl:text>&nl;Markup                      : </xsl:text>
			<xsl:value-of select="$data/markup"/>
		</xsl:if>
		
		<xsl:if test="$data/memory != '' ">
			<xsl:text>&nl;Memory                      : </xsl:text>
			<xsl:value-of select="$data/memory"/>
		</xsl:if>
		
		<xsl:if test="$data/sourceLang != '' ">
			<xsl:text>&nl;Source language             : </xsl:text>
				Source language
				<xsl:value-of select="$data/sourceLang"/>
			
		</xsl:if>
		<xsl:if test="$data/targetLang != '' ">
			<xsl:text>&nl;Target language             : </xsl:text>
				Target language
				<xsl:value-of select="$data/targetLang"/>
			
		</xsl:if>
	</xsl:template>

	
	<!-- display history data of version record -->
	<xsl:template name="version">
		<xsl:param name="data"/>
		
		<xsl:text>&nl;Platform                    : </xsl:text>
		<xsl:value-of select="$data/@platform"/>
	
		<xsl:text>&nl;Product date                : </xsl:text>
		<xsl:value-of select="$data/@productDate"/>
	
		<xsl:text>&nl;Version                     : </xsl:text>
		<xsl:value-of select="$data/@version"/>
		
	</xsl:template>
	
	<!-- display history data of folprop record -->
	<xsl:template name="folprop">
		<xsl:param name="data"/>
		
		<xsl:if test="$data/description != '' ">
			<xsl:text>&nl;Description                 : </xsl:text>
			<xsl:value-of select="$data/description"/>
		</xsl:if>
		
		<xsl:if test="$data/editor != '' ">
			<xsl:text>&nl;Editor                      : </xsl:text>
			<xsl:value-of select="$data/editor"/>
		</xsl:if>
		
		<xsl:if test="$data/markup != '' ">
			<xsl:text>&nl;Markup                      : </xsl:text>
			<xsl:value-of select="$data/markup"/>
		</xsl:if>
		
		<xsl:if test="$data/sourceLang != '' ">
			<xsl:text>&nl;Source language             : </xsl:text>
			<xsl:value-of select="$data/sourceLang"/>
		</xsl:if>

		<xsl:if test="$data/targetLang != '' ">
			<xsl:text>&nl;Target language             : </xsl:text>
			<xsl:value-of select="$data/targetLang"/>
		</xsl:if>
	</xsl:template>

	
	<!-- display history data of folder shipment record -->
	<xsl:template name="folpropshipment">
		<xsl:param name="data"/>
		
		<xsl:text>&nl;Shipment                    : </xsl:text>
		<xsl:value-of select="$data/shipment"/>
		
	</xsl:template>
	
	
	
	<!-- display history data of import record -->
	<xsl:template name="import">
		<xsl:param name="data"/>
		
		<xsl:text>&nl;Format                      : </xsl:text>
		<xsl:value-of select="$data/importType"/>
		
		<xsl:text>&nl;Import path                 : </xsl:text>
		<xsl:value-of select="$data/path"/>
		
		<xsl:text>&nl;Document                    : </xsl:text>
		<xsl:value-of select="$data/document"/>
		
		<xsl:text>&nl;Shipment number             : </xsl:text>
		<xsl:value-of select="$data/shipment"/>
		
		<xsl:text>&nl;Source replaced             : </xsl:text>
		<xsl:value-of select="$data/sourcereplaced"/>
		
		<xsl:text>&nl;Target replaced             : </xsl:text>
		<xsl:value-of select="$data/targetreplaced"/>
		
	</xsl:template>

	
	
	<!-- display history data of analysis record -->
	<xsl:template name="analysis">
		<xsl:param name="data"/>
		
		<xsl:text>&nl;Substitute exact matches    : </xsl:text> 
		<xsl:value-of select="$data/options/@autoReplace"/>
		
		<xsl:text>&nl;Use latest exact match      : </xsl:text> 
		<xsl:value-of select="$data/options/@useLatestMatch"/>
		
		<xsl:text>&nl;Use joined segments         : </xsl:text>
		<xsl:value-of select="$data/options/@autoJoin"/>
		
		<xsl:text>&nl;Add to memory               : </xsl:text>
		<xsl:value-of select="$data/options/@addToMem"/>
		
		<xsl:text>&nl;Translation memory          : </xsl:text>
		<xsl:value-of select="$data/memory"/>
		
		<xsl:text>&nl;Markup                      : </xsl:text>
		<xsl:value-of select="$data/markup"/>
		
	</xsl:template>
	
	
	<!-- display history data of substitution record -->
	<xsl:template name="autosubst">
		<xsl:param name="data"/>
		
		<xsl:call-template name="fuzzylevel">
			<xsl:with-param name="level" select="$data/fuzzyLevel"/>
		</xsl:call-template>
		
		<xsl:call-template name="counttableheader"/>
		
		<xsl:for-each select="$data/wordcount">
			<xsl:call-template name="wordcount">
				<xsl:with-param name="counts" select="."/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:template>
	
	<!-- display doc save record -->
	<xsl:template name="docsave">
		<xsl:param name="data"/>
			
		<xsl:call-template name="fuzzylevel">
			<xsl:with-param name="level" select="$data/fuzzyLevel"/>
		</xsl:call-template>
		
		<xsl:call-template name="counttableheader"/>
		
		<xsl:for-each select="$data/wordcount">
			<xsl:call-template name="wordcount">
				<xsl:with-param name="counts" select="."/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:template>
	
	<!-- display history data of document export -->
	<xsl:template name="docexport">
		<xsl:param name="data"/>
			
		<xsl:text>&nl;Format                      : </xsl:text>
		<xsl:value-of select="$data/exportType"/>
			
		<xsl:if test="$data/exportType != 'Folder' ">
			<xsl:text>&nl;Source exported             : </xsl:text>
			<xsl:value-of select="$data/sourceExported"/>
				
			<xsl:text>&nl;Target exported             : </xsl:text>
			<xsl:value-of select="$data/targetExported"/>
				
			<xsl:text>&nl;With revision marks         : </xsl:text>
			<xsl:value-of select="$data/revisionMarks"/>
				
			<xsl:text>&nl;SNOMATCH file exported      : </xsl:text>
			<xsl:value-of select="$data/snoMatch"/>
				
			<xsl:text>&nl;Source path                 : </xsl:text>
			<xsl:value-of select="$data/sourcePath"/>
				
			<xsl:text>&nl;Target path                 : </xsl:text>
			<xsl:value-of select="$data/targetExported"/>
		</xsl:if>
		
	</xsl:template>

	<!-- show fuzzy levels -->
	<xsl:template name="fuzzylevel">
		<xsl:param name="level"/>
		<xsl:if test="$level/@small !='' ">
			<xsl:text>&nl;Fuzzyness level for segments with 1..4 words        : </xsl:text>
			<xsl:value-of select="$level/@small"/>
			<xsl:text> %</xsl:text>		
			
			<xsl:text>&nl;Fuzzyness level for segments with 5..14 words       : </xsl:text>
			<xsl:value-of select="$level/@medium"/>
			<xsl:text> %</xsl:text>		
			
			<xsl:text>&nl;Fuzzyness level for segments with 15 and more words : </xsl:text>
			<xsl:value-of select="$level/@large"/>
			<xsl:text> %</xsl:text>		
		</xsl:if>
	</xsl:template>
	
	
	<!-- setup count table header -->
	<xsl:template name="counttableheader">
		<xsl:text>&nl;-----------------------------------------------------------------------------</xsl:text>
		<xsl:text>&nl;|              |        | Segments   |  Source    | Modified   | Target     |</xsl:text>
		<xsl:text>&nl;|              |        |            |  Words     | Words      | Words      |</xsl:text>
		<xsl:text>&nl;-----------------------------------------------------------------------------</xsl:text>
	</xsl:template>

	
	<!-- show counts of a single wordcount -->
	<xsl:template name="wordcount">
		<xsl:param name="counts"/>
			<xsl:if test="$counts/@empty = 'No' ">
				<xsl:text>&nl;| </xsl:text>
				<xsl:variable name="typetext1">
					<xsl:choose>
						<xsl:when test="$counts/@type = 'analysisAutosubst'">Analysis</xsl:when>
						<xsl:when test="$counts/@type = 'editAutosubst'">Edit</xsl:when>
						<xsl:when test="$counts/@type = 'exact'">Exact</xsl:when>
						<xsl:when test="$counts/@type = 'replace'">Replace</xsl:when>
						<xsl:when test="$counts/@type = 'fuzzy'">Fuzzy</xsl:when>
						<xsl:when test="$counts/@type = 'machine'">Machine</xsl:when>
						<xsl:when test="$counts/@type = 'nomatch'">None</xsl:when>
						<xsl:when test="$counts/@type = 'notTranslated'">Not</xsl:when>
						<xsl:otherwise></xsl:otherwise>
					</xsl:choose>
				</xsl:variable>
				<xsl:call-template name = "textcolumn" >
					<xsl:with-param name="value" select="$typetext1"/>
					<xsl:with-param name="width">12</xsl:with-param>
				</xsl:call-template>
				
				<xsl:call-template name = "textcolumn" >
					<xsl:with-param name="value">1..4</xsl:with-param>
					<xsl:with-param name="width">6</xsl:with-param>
				</xsl:call-template>

				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='simple' and @type='segments']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='simple' and @type='sourceWords']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='simple' and @type='modWords']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='simple' and @type='targetWords']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
			
				<xsl:text>&nl;| </xsl:text>
				<xsl:variable name="typetext2">
					<xsl:choose>
						<xsl:when test="$counts/@type = 'analysisAutosubst'">Autosubst</xsl:when>
						<xsl:when test="$counts/@type = 'editAutosubst'">Autosubst</xsl:when>
						<xsl:when test="$counts/@type = 'exact'">Matches</xsl:when>
						<xsl:when test="$counts/@type = 'replace'">Matches</xsl:when>
						<xsl:when test="$counts/@type = 'fuzzy'">Matches</xsl:when>
						<xsl:when test="$counts/@type = 'machine'">Matches</xsl:when>
						<xsl:when test="$counts/@type = 'nomatch'">Matches</xsl:when>
						<xsl:when test="$counts/@type = 'notTranslated'">Translated</xsl:when>
						<xsl:otherwise></xsl:otherwise>
					</xsl:choose>
				</xsl:variable>
			
				<xsl:call-template name = "textcolumn" >
					<xsl:with-param name="value" select="$typetext2"/>
					<xsl:with-param name="width">12</xsl:with-param>
				</xsl:call-template>
				
				<xsl:call-template name = "textcolumn" >
					<xsl:with-param name="value">5..14</xsl:with-param>
					<xsl:with-param name="width">6</xsl:with-param>
				</xsl:call-template>

				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='medium' and @type='segments']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='medium' and @type='sourceWords']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='medium' and @type='modWords']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='medium' and @type='targetWords']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
			
				<xsl:text>&nl;| </xsl:text>
				<xsl:call-template name = "textcolumn" >
					<xsl:with-param name="value"></xsl:with-param>
					<xsl:with-param name="width">12</xsl:with-param>
				</xsl:call-template>
				 
				<xsl:call-template name = "textcolumn" >
					<xsl:with-param name="value">15..</xsl:with-param>
					<xsl:with-param name="width">6</xsl:with-param>
				</xsl:call-template>
				 
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='complex' and @type='segments']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='complex' and @type='sourceWords']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='complex' and @type='modWords']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				<xsl:call-template name = "numcolumn" >
					<xsl:with-param name="value" select="$counts/words[@category='complex' and @type='targetWords']/@count"/>
					<xsl:with-param name="width">10</xsl:with-param>
				</xsl:call-template>
				
				<xsl:text>&nl;-----------------------------------------------------------------------------</xsl:text>
				
			</xsl:if>
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
	
	<xsl:variable name="filledcolumn" select="concat($value,'                                       ')"/>
	<xsl:value-of select="substring($filledcolumn,1,$width)"/>
	<xsl:text> | </xsl:text>
</xsl:template>

<xsl:template name = "documentlist" >
    <xsl:text>&nl;&nl;</xsl:text>
	<xsl:text>Report: Document List&nl;&nl;</xsl:text>
    <xsl:for-each select="/tmhistory/documentList/document">
      <xsl:text>         </xsl:text><xsl:value-of select="./@num"/><xsl:text>&nl;</xsl:text>
	  <xsl:if test="./path!=''">
		   <xsl:text>Path   : </xsl:text><xsl:value-of select="./path"/><xsl:text>&nl;</xsl:text>
	  </xsl:if>
      <xsl:text>Name   : </xsl:text><xsl:value-of select="./name"/><xsl:text>&nl;</xsl:text>
      <xsl:text>-------------------------------------------&nl;</xsl:text>
  </xsl:for-each>
</xsl:template>
	
	
</xsl:stylesheet>
