<?xml version='1.0' encoding="iso-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>

<xsl:import href="@DOCBOOK_XSL@/epub/docbook.xsl"/> 

<!-- Set parameters to uniformize manual across all formats -->

<xsl:param name="section.autolabel" select="1"/>
<xsl:param name="section.label.includes.component.label" select="1"/>
<xsl:param name="xref.with.number.and.title" select="0"/>

</xsl:stylesheet>

