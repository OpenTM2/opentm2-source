<?altova_samplexml i:\eqf\p0233001.f00\rpt.xml?>
<!--
    Copyright (c) 1999-2015, International Business Machines
    Corporation and others.  All rights reserved.
-->

<!--  does not exists in XPath, so defined as the real unicode character 160
     which is the non breaking space character and provides exactly the same result than using  -->
     
<!DOCTYPE xsl:stylesheet 
	[
		<!ENTITY nbsp "&#160;">
		<!ENTITY nl "&#xa;"><!--new line-->
	]>

<!-- calctext.xsl

	Author: Gerhard Queck

	Purpose: Convert an XML calculation report into plain text
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="text"/>

	<xsl:template match="/">
				<xsl:call-template name="reportheader" />
			    <xsl:for-each select="/tmwordcount/description/report/option">
					<xsl:choose>
						<xsl:when test="./@name = 'Base' ">
							<xsl:call-template name="base" />
						</xsl:when>
						<xsl:when test="./@name = 'Summary' ">
							<xsl:call-template name="summary" />
						</xsl:when>
						<xsl:when test="./@name = 'FactSheet' ">
							<xsl:call-template name="factsheet" />
							<xsl:call-template name="finalsummary" />
						</xsl:when>
					</xsl:choose>
			    </xsl:for-each>
			    <xsl:if test="/tmwordcount/description/report/@withDocumentList='Yes'">
			      <xsl:call-template name="documentlist"/>
			    </xsl:if>
	</xsl:template>
	
<!-- create report header --> 
<xsl:template name = "reportheader">
	<xsl:text>&nl;Report                                  : </xsl:text>
	<xsl:value-of select="/tmwordcount/description/report/@type"/> 
	<xsl:text>   </xsl:text>
	<xsl:for-each select="/tmwordcount/description/report/option">
		<xsl:if test="position() != 1">
			<xsl:text>,</xsl:text>
		</xsl:if>
		<xsl:text> </xsl:text>
		<xsl:value-of select="./@name"/>
	</xsl:for-each>
	
	<xsl:text>&nl;OpenTM2 Version                         : </xsl:text>
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
	<xsl:text>&nl;Target Language                         : </xsl:text><xsl:value-of select="/tmwordcount/description/folder/@targetLanguage"/>
    <xsl:if test="/tmwordcount/description/report/@documentsWithRedundantInfo != ''">
		<xsl:text>&nl;Documents prepared for redundany report : </xsl:text>
		<xsl:value-of select="/tmwordcount/description/report/@documentsWithRedundantInfo"/>
	</xsl:if>
	<xsl:text>&nl;&nl;</xsl:text>
</xsl:template>




<!-- create base report --> 
<xsl:template name = "base" >
	<xsl:text>&nl;&nl;Word Count : Base</xsl:text>
	<xsl:text>&nl;Folder     : </xsl:text><xsl:value-of select="/tmwordcount/description/folder/@nameLong"/>
	<xsl:text>&nl;Counted    : Source Words</xsl:text>
	<xsl:for-each select="/tmwordcount/documents/document">
		<xsl:call-template name="base-one-document">
			<xsl:with-param name="document" select="." />
		</xsl:call-template>
</xsl:for-each>

</xsl:template>

<!-- create base report for one document --> 
<xsl:template name = "base-one-document" >
	<xsl:param name="document"></xsl:param>
	<xsl:text>&nl;&nl;Document   : </xsl:text><xsl:value-of select="$document/@nameLong"/>
    <xsl:if test="$document/@path != '' ">
		<xsl:text>&nl;Path       : </xsl:text><xsl:value-of select="$document/@path"/>
    </xsl:if>
	<xsl:text>&nl;Folder     : </xsl:text><xsl:value-of select="/tmwordcount/description/folder/@nameLong"/>
	<xsl:if test="$document/@memory != '' ">
		<xsl:text>&nl;Memory     : </xsl:text>Memory: <xsl:value-of select="$document/@memory"/>
	</xsl:if>
	<xsl:text>&nl;Counted    : Source Words</xsl:text>
	<xsl:call-template name="countsheader">
		<xsl:with-param name="factsheet">No</xsl:with-param>
	</xsl:call-template>
	<xsl:for-each select="$document/wordcount">
		<xsl:if test="./@shipmentFolder = 'All Shipments' ">
			<xsl:call-template name="wordcounts">
				<xsl:with-param name="counts" select="." />
				<xsl:with-param name="name" >Sum</xsl:with-param>
				<xsl:with-param name="id" >0</xsl:with-param>
			</xsl:call-template>
		</xsl:if>
		<xsl:if test="./@shipmentFolder != 'All Shipments' ">
			<xsl:call-template name="wordcounts">
				<xsl:with-param name="counts" select="." />
				<xsl:with-param name="name" ><xsl:value-of select="$document/@nameLong"/></xsl:with-param>
				<xsl:with-param name="id" ><xsl:value-of select="$document/@id"/></xsl:with-param>
			</xsl:call-template>
		</xsl:if>
	</xsl:for-each>
</xsl:template>
	
<!-- create summary --> 
<xsl:template name = "summary" >
	<xsl:text>&nl;&nl;Word Count : Summary</xsl:text>
	<xsl:text>&nl;Folder     : </xsl:text><xsl:value-of select="/tmwordcount/description/folder/@nameLong"/>
	<xsl:text>&nl;Counted    : Source Words</xsl:text>
	<xsl:call-template name="countsheader">
		<xsl:with-param name="factsheet">No</xsl:with-param>
	</xsl:call-template>
	<xsl:for-each select="/tmwordcount/summary/document">
			<xsl:call-template name="wordcounts">
				<xsl:with-param name="counts" select="./wordcount" />
				<xsl:with-param name="name" ><xsl:value-of select="@name"/></xsl:with-param>
				<xsl:with-param name="id" ><xsl:value-of select="@id"/></xsl:with-param>
			</xsl:call-template>
	</xsl:for-each>
</xsl:template>

<!-- setup table column header base on the column list --> 
<xsl:template name = "countsheader" >
	<xsl:param name="factsheet"></xsl:param>

	<xsl:call-template name="seperline"/>

	<!-- first header line --> 
		  
	<xsl:text>&nl;|</xsl:text> 
	
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value">Doc ID</xsl:with-param>
			<xsl:with-param name="width">4</xsl:with-param>
		</xsl:call-template>
		
		<xsl:if test="$factsheet = 'Yes'" >
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="column">SHOW</xsl:with-param>
				<xsl:with-param name="value">Category</xsl:with-param>
				<xsl:with-param name="width">14</xsl:with-param>
			</xsl:call-template>
		</xsl:if>
		 
		<xsl:if test="$factsheet != 'Yes'" >
			<xsl:call-template name="textcolumn">
				<xsl:with-param name="column">SHOW</xsl:with-param>
				<xsl:with-param name="value">Document</xsl:with-param>
				<xsl:with-param name="width">14</xsl:with-param>
			</xsl:call-template>
		</xsl:if>
		 
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value">Ship</xsl:with-param>
			<xsl:with-param name="width">4</xsl:with-param>
		</xsl:call-template>
	 
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value">Category</xsl:with-param>
			<xsl:with-param name="width">6</xsl:with-param>
		</xsl:call-template>
		
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value">Fuzzy</xsl:with-param>
			<xsl:with-param name="width">6</xsl:with-param>
		</xsl:call-template>

		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SumAnalyze</xsl:with-param>
			<xsl:with-param name="value">Analyze</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">AnalyzeAuto</xsl:with-param>
			<xsl:with-param name="value">Analyze</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">AnalyzePost</xsl:with-param>
			<xsl:with-param name="value">Analyze</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SumEdit</xsl:with-param>
			<xsl:with-param name="value">Edit</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>

		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">AnalyzeEdit</xsl:with-param>
			<xsl:with-param name="value">Edit</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">EditExact</xsl:with-param>
			<xsl:with-param name="value">Exact</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">EditReplace</xsl:with-param>
			<xsl:with-param name="value">Replace</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SumFuzzy</xsl:with-param>
			<xsl:with-param name="value">Sum</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">Fuzzy5069</xsl:with-param>
			<xsl:with-param name="value">Fuzzy</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">Fuzzy7089</xsl:with-param>
			<xsl:with-param name="value">Fuzzy</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">Fuzzy9099</xsl:with-param>
			<xsl:with-param name="value">Fuzzy</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">ChangedFuzzy</xsl:with-param>
			<xsl:with-param name="value">Fuzzy</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">MachineMatches</xsl:with-param>
			<xsl:with-param name="value">Machine</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">NoneMatches</xsl:with-param>
			<xsl:with-param name="value">Manually</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">NotTranslated</xsl:with-param>
			<xsl:with-param name="value">Not</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		
	<!-- second header line --> 
		  
	<xsl:text>&nl;|</xsl:text> 
	
	<xsl:call-template name="textcolumn">
		<xsl:with-param name="column">SHOW</xsl:with-param>
		<xsl:with-param name="value">ID</xsl:with-param>
		<xsl:with-param name="width">4</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="textcolumn">
		<xsl:with-param name="column">SHOW</xsl:with-param>
		<xsl:with-param name="value"></xsl:with-param>
		<xsl:with-param name="width">14</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="textcolumn">
		<xsl:with-param name="column">SHOW</xsl:with-param>
		<xsl:with-param name="value">Number</xsl:with-param>
		<xsl:with-param name="width">4</xsl:with-param>
	</xsl:call-template>
	 
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value"></xsl:with-param>
			<xsl:with-param name="width">6</xsl:with-param>
		</xsl:call-template>
		
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value">Level</xsl:with-param>
			<xsl:with-param name="width">6</xsl:with-param>
		</xsl:call-template>

		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SumAnalyze</xsl:with-param>
			<xsl:with-param name="value">Analyze</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">AnalyzeAuto</xsl:with-param>
			<xsl:with-param name="value">Auto</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">AnalyzePost</xsl:with-param>
			<xsl:with-param name="value">Auto2</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SumEdit</xsl:with-param>
			<xsl:with-param name="value">Edit</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>

		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">AnalyzeEdit</xsl:with-param>
			<xsl:with-param name="value">Auto</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">EditExact</xsl:with-param>
			<xsl:with-param name="value">Matches</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">EditReplace</xsl:with-param>
			<xsl:with-param name="value">Matches</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SumFuzzy</xsl:with-param>
			<xsl:with-param name="value">Fuzzy</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">Fuzzy5069</xsl:with-param>
			<xsl:with-param name="value">50-70%</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">Fuzzy7089</xsl:with-param>
			<xsl:with-param name="value">71-90%</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">Fuzzy9099</xsl:with-param>
			<xsl:with-param name="value">&gt;90%</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">ChangedFuzzy</xsl:with-param>
			<xsl:with-param name="value">Changed</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">MachineMatches</xsl:with-param>
			<xsl:with-param name="value">Matches</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">NoneMatches</xsl:with-param>
			<xsl:with-param name="value">Translated</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">NotTranslated</xsl:with-param>
			<xsl:with-param name="value">Translated</xsl:with-param>
			<xsl:with-param name="width">10</xsl:with-param>
		</xsl:call-template>
		
	<xsl:call-template name="seperline"/>

</xsl:template>
	
<!-- create fact sheet --> 
<xsl:template name = "factsheet" >
	<xsl:text>&nl;&nl;Word Count : Fact Sheet</xsl:text>
	<xsl:text>&nl;Folder     : </xsl:text><xsl:value-of select="/tmwordcount/description/folder/@nameLong"/>
	<xsl:text>&nl;Counted    : Source Words</xsl:text>

	<xsl:call-template name="countsheader">
		<xsl:with-param name="factsheet">Yes</xsl:with-param>
	</xsl:call-template>
	<xsl:for-each select="/tmwordcount/factsheet/group">
			<xsl:call-template name="wordcounts">
				<xsl:with-param name="counts" select="./wordcount" />
				<xsl:with-param name="name" ><xsl:value-of select="@name"/></xsl:with-param>
				<xsl:with-param name="id" ></xsl:with-param>
			</xsl:call-template>
	</xsl:for-each>
</xsl:template>

<!-- display all  categories -->      
<xsl:template name = "wordcounts" >
	<xsl:param name="counts"></xsl:param>
	<xsl:param name="name"></xsl:param>
	<xsl:param name="id"></xsl:param>
	
	<xsl:if test="/tmwordcount/description/report/@catDetails='true'" >
		<xsl:call-template name="wordcount">
			<xsl:with-param name="counts" select="$counts" />
			<xsl:with-param name="name" ><xsl:value-of select="$name"/></xsl:with-param>
			<xsl:with-param name="cat" >Simple</xsl:with-param>
			<xsl:with-param name="id" ><xsl:value-of select="$id"/></xsl:with-param>
		</xsl:call-template>
	
		<xsl:call-template name="wordcount">
			<xsl:with-param name="counts" select="$counts" />
			<xsl:with-param name="name" ></xsl:with-param>
			<xsl:with-param name="cat" >Medium</xsl:with-param>
			<xsl:with-param name="id" ></xsl:with-param>
		</xsl:call-template>
	
		<xsl:call-template name="wordcount">
			<xsl:with-param name="counts" select="$counts" />
			<xsl:with-param name="name" ></xsl:with-param>
			<xsl:with-param name="cat" >Complex</xsl:with-param>
			<xsl:with-param name="id" ></xsl:with-param>
		</xsl:call-template>
	</xsl:if>
	<xsl:if test="/tmwordcount/description/report/@catSum='true'" >
		<xsl:choose>
			<xsl:when test="/tmwordcount/description/report/@catDetails='true'"> 
				<xsl:call-template name="seperline2"/>
				<xsl:call-template name="wordcount">
					<xsl:with-param name="counts" select="$counts" />
					<xsl:with-param name="name"></xsl:with-param>
					<xsl:with-param name="cat" >Sum</xsl:with-param>
					<xsl:with-param name="id" ></xsl:with-param>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise> 
			<xsl:call-template name="wordcount">
				<xsl:with-param name="counts" select="$counts" />
				<xsl:with-param name="name" select="$name"/>
				<xsl:with-param name="cat" >Sum</xsl:with-param>
				<xsl:with-param name="id"  select="$id"/>
			</xsl:call-template>
			</xsl:otherwise>
		</xsl:choose> 
	</xsl:if>
	
	<xsl:call-template name="seperline"/>
	
</xsl:template>
      
<!-- display the counts of a single category -->      
<xsl:template name = "wordcount" >
	<xsl:param name="counts"></xsl:param>
	<xsl:param name="name"></xsl:param>
	<xsl:param name="cat"></xsl:param>
	<xsl:param name="id"></xsl:param>

	<!-- start a new line -->
	<xsl:text>&nl;|</xsl:text>
	
	<!-- ID -->
	<xsl:if test="$id !='0'" >
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value" select="$id" />
			<xsl:with-param name="width">4</xsl:with-param>
		</xsl:call-template>
	</xsl:if>
	<xsl:if test="$id ='0'" >
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value"></xsl:with-param>
			<xsl:with-param name="width">4</xsl:with-param>
		</xsl:call-template>
	</xsl:if>
	
	<!-- DocName or Type -->
	<xsl:variable name="rowname">
		<xsl:choose>
			<xsl:when test="$name = 'ActualWords'">Actual Words</xsl:when>
			<xsl:when test="$name = 'ComplexityFactor'">Complexity Factor</xsl:when>
			<xsl:when test="$name = 'WeightedWords'">Complexity Factor x  Actual Words.</xsl:when>
			<xsl:when test="$name = 'PayFactor'">Payment factor</xsl:when>
			<xsl:when test="$name = 'PayWords'">Payable Words</xsl:when>
			<xsl:otherwise><xsl:value-of select="$name"/></xsl:otherwise>
		</xsl:choose>
	</xsl:variable>
	<xsl:call-template name="textcolumn">
		<xsl:with-param name="column">SHOW</xsl:with-param>
		<xsl:with-param name="value" select="$rowname" />
		<xsl:with-param name="width">14</xsl:with-param>
	</xsl:call-template>
	
	<!-- shipment -->
	<xsl:if test="$id !=''" >
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value" select="$counts/@shipmentDocument" />
			<xsl:with-param name="width">4</xsl:with-param>
		</xsl:call-template>
	</xsl:if>   
	<xsl:if test="$id =''" >
		<xsl:call-template name="textcolumn">
			<xsl:with-param name="column">SHOW</xsl:with-param>
			<xsl:with-param name="value"></xsl:with-param>
			<xsl:with-param name="width">4</xsl:with-param>
		</xsl:call-template>
	</xsl:if>   

	
	<!-- category text -->
	<xsl:variable name="catname">
		<xsl:choose>
			<xsl:when test="$cat = 'Simple'">1..4</xsl:when>
			<xsl:when test="$cat = 'Medium'">5..14</xsl:when>
			<xsl:when test="$cat = 'Complex'">&gt;14</xsl:when>
			<xsl:when test="$cat = 'Sum'">
				<xsl:choose>
					<xsl:when test="$name = 'ComplexityFactor'">Mean comp</xsl:when>
					<xsl:when test="$name = 'PayFactor'">Mean pay</xsl:when>
					<xsl:otherwise>Sum</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise></xsl:otherwise>
		</xsl:choose>
	</xsl:variable>	
	<xsl:call-template name="textcolumn">
		<xsl:with-param name="column">SHOW</xsl:with-param>
		<xsl:with-param name="value" select="$catname" />
		<xsl:with-param name="width">6</xsl:with-param>
	</xsl:call-template>

	<!-- fuzzy level -->
	<xsl:variable name="fuzzylevel">
		 <xsl:value-of select="$counts/fuzzy[@category = $cat]/@levelPerCent" />
		 <xsl:text>%</xsl:text>
	</xsl:variable>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">FuzzyLevel</xsl:with-param>
		<xsl:with-param name="value" select="$fuzzylevel" />
		<xsl:with-param name="width">6</xsl:with-param>
	</xsl:call-template>

	<!-- the count values -->
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">SumAnalyze</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='SumAnalyze']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">AnalyzeAuto</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='AnalyzeAuto']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">AnalyzePost</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='AnalyzePost']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">SumEdit</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='SumEdit']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">AnalyzeEdit</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='AnalyzeEdit']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">EditExact</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='EditExact']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">EditReplace</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='EditReplace']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">SumFuzzy</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='SumFuzzy']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">Fuzzy5069</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='Fuzzy5069']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">Fuzzy7089</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='Fuzzy7089']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">Fuzzy9099</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='Fuzzy9099']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">ChangedFuzzy</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='ChangedFuzzy']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">MachineMatches</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='MachineMatches']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">NoneMatches</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='NoneMatches']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
	<xsl:call-template name="numcolumn">
		<xsl:with-param name="column">NotTranslated</xsl:with-param>
		<xsl:with-param name="value" select="$counts/words[@category=$cat and @type='NotTranslated']/@count" />
		<xsl:with-param name="width">10</xsl:with-param>
	</xsl:call-template>
</xsl:template>
      
<!-- show final sumary -->      
<xsl:template name = "finalsummary" >
	<xsl:text>&nl;&nl;Word Count : Final Summary</xsl:text>
	<xsl:text>&nl;Shipments      : </xsl:text><xsl:value-of select="/tmwordcount/finalSummary/@shipment"/>
	<xsl:text>&nl;Payable words  : </xsl:text><xsl:value-of select="/tmwordcount/finalSummary/@payableWords"/>
	<xsl:text>&nl;Local Currency : </xsl:text><xsl:value-of select="/tmwordcount/finalSummary/@localCurrency"/>
	<xsl:text>&nl;Total Payment  : </xsl:text><xsl:value-of select="/tmwordcount/finalSummary/@totalPay"/>
</xsl:template>

<!-- format a numerical column value to a given width  -->      
<xsl:template name = "numcolumn" >
	<xsl:param name="column"></xsl:param>
	<xsl:param name="value"></xsl:param>
	<xsl:param name="width"></xsl:param>
	
	<xsl:if test="$column='SHOW'" >
		<xsl:variable name="filledcolumn" select="concat('               ',$value)"/>
		<xsl:variable name="pos" select="string-length($filledcolumn)-$width"/>
		<xsl:value-of select="substring($filledcolumn,$pos,$width)"/>
		<xsl:text> | </xsl:text>
	</xsl:if>
	
	<xsl:if test="/tmwordcount/columnList/column[@name=$column]/@shrink !='Yes'" >
		<xsl:variable name="filledcolumn" select="concat('               ',$value)"/>
		<xsl:variable name="pos" select="string-length($filledcolumn) - $width + 1"/>
		<xsl:value-of select="substring($filledcolumn,$pos,$width)"/>
		<xsl:text> | </xsl:text>
	</xsl:if>
</xsl:template>

<!-- format a text column value to a given width  -->      
<xsl:template name = "textcolumn" >
	<xsl:param name="column"></xsl:param>
	<xsl:param name="value"></xsl:param>
	<xsl:param name="width"></xsl:param>
	
	<xsl:if test="$column='SHOW'" >
		<xsl:variable name="filledcolumn" select="concat($value,'                             ')"/>
		<xsl:value-of select="substring($filledcolumn,1,$width)"/>
		<xsl:text> | </xsl:text>
	</xsl:if>
	
	<xsl:if test="/tmwordcount/columnList/column[@name=$column]/@shrink !='Yes'" >
		<xsl:variable name="filledcolumn" select="concat($value,'                             ')"/>
		<xsl:value-of select="substring($filledcolumn,1,$width)"/>
		<xsl:text> | </xsl:text>
	</xsl:if>
</xsl:template>

<!-- seperation line -->      
<xsl:template name = "seperline" >

	<!-- seper line for non-variant columns -->      
	<xsl:text>&nl;=================================</xsl:text>
	
	<!-- seper line for active columns -->      
    <xsl:for-each select="/tmwordcount/columnList/column">
		<xsl:if test="./@shrink != 'Yes'">
          <xsl:choose>
			  <xsl:when test="./@name='Shipment'"><xsl:text>=======</xsl:text></xsl:when>
			  <xsl:when test="./@name='FuzzyLevel'"><xsl:text>=========</xsl:text></xsl:when>
			  <xsl:otherwise><xsl:text>=============</xsl:text></xsl:otherwise>
		  </xsl:choose>		
		</xsl:if>
    </xsl:for-each>
</xsl:template>

<!-- inner seperation line -->      
<xsl:template name = "seperline2" >

	<!-- seper line for non-variant columns -->      
	<xsl:text>&nl;---------------------------------</xsl:text>
	
	<!-- seper line for active columns -->      
    <xsl:for-each select="/tmwordcount/columnList/column">
		<xsl:if test="./@shrink != 'Yes'">
          <xsl:choose>
			  <xsl:when test="./@name='Shipment'"><xsl:text>-------</xsl:text></xsl:when>
			  <xsl:when test="./@name='FuzzyLevel'"><xsl:text>---------</xsl:text></xsl:when>
			  <xsl:otherwise><xsl:text>-------------</xsl:text></xsl:otherwise>
		  </xsl:choose>		
		</xsl:if>
    </xsl:for-each>
</xsl:template>

<xsl:template name = "documentlist" >
    <xsl:text>&nl;&nl;</xsl:text>
	<xsl:text>Report: Document List&nl;&nl;</xsl:text>
    <xsl:for-each select="/tmwordcount/documentList/document">
      <xsl:text>         </xsl:text><xsl:value-of select="./@num"/><xsl:text>&nl;</xsl:text>
	  <xsl:if test="./path!=''">
		   <xsl:text>Path   : </xsl:text><xsl:value-of select="./path"/><xsl:text>&nl;</xsl:text>
	  </xsl:if>
      <xsl:text>Name   : </xsl:text><xsl:value-of select="./name"/><xsl:text>&nl;</xsl:text>
      <xsl:text>-------------------------------------------&nl;</xsl:text>
  </xsl:for-each>
</xsl:template>
	

</xsl:stylesheet>
