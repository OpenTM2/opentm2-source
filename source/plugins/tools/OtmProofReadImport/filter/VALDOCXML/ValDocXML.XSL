<!--
    Copyright (c) 1999-2017, International Business Machines
    Corporation and others.  All rights reserved.
-->

<!-- &nbsp; does not exists in XPath, so defined as the real unicode character 160
     which is the non breaking space character and provides exactly the same result than using &nbsp; -->
     
<!DOCTYPE xsl:stylesheet 
	[
		<!ENTITY nbsp "&#160;">
	]>

<!-- valdocxml.xsl

	Author: Gerhard Queck

	Purpose: Convert an XML validation document into the ProofRead XML format
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- =========================================================================================== --> 	
<!-- == Date of Stylesheet (original): February 15, 2017                                      == -->
<!-- == Created by                   : Gerhard Queck                                          == -->
<!-- == Date of Stylesheet (update)  : February 15, 2017                                      == -->
<!-- == Updated by                   : Gerhard Queck                                          == -->
<!-- == Purpose                      : Convert a validation document into proofread import XML== -->
<!-- == Version                      : 1.0                                                    == -->
<!-- =========================================================================================== -->

	<xsl:output method="xml" encoding="utf-8"/>

	<xsl:template match="/">
		<ProofReadingResults>
     		<Header>
				<Folder><xsl:value-of select="/Proof/Document/Header/Folder"/></Folder>
				<CreationDate><xsl:value-of select="/Proof/Document/Header/CreationDate"/></CreationDate>
				<ProofReadDate></ProofReadDate>
				<Translator><xsl:value-of select="/Proof/Document/Header/Translator"/></Translator>
				<ProofReader></ProofReader>
			</Header>
		    <xsl:for-each select="/Proof/Document">
				<xsl:call-template name="document">
					<xsl:with-param name="doc" select="." />
				</xsl:call-template>
		    </xsl:for-each>
		</ProofReadingResults>
	</xsl:template>

	
<!-- convert a single document --> 
<xsl:template name = "document" >
	<xsl:param name="doc"/>
		<Document name="{$doc/Header/DocumentName}" SourceLang="{$doc/Header/SourceLanguage}" TargetLang="{$doc/Header/TargetLanguage}" Markup="{$doc/Header/Markup}">
			<SegmentList>
				<xsl:for-each select="$doc/SegmentList/Segment">
					<xsl:call-template name="segment">
						<xsl:with-param name="seg" select="." />
					</xsl:call-template>
				</xsl:for-each>
			</SegmentList>
		</Document>
</xsl:template>

<!-- handle a single segment --> 
<xsl:template name = "segment" >
	<xsl:param name="seg"/>
    <Segment Number="{$seg/@Number}" Selected="no" Processed="no">
	  <Source><xsl:value-of select="$seg/Source"/></Source>
	  <Target><xsl:value-of select="$seg/Target"/></Target>
	  <ModTarget><xsl:value-of select="$seg/ModTarget"/></ModTarget>
	  <NewTarget></NewTarget>
	  <Comment></Comment>
    </Segment>
</xsl:template>


</xsl:stylesheet>
      
