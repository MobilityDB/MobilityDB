<?xml version='1.0' encoding="iso-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>

<xsl:import href="@DOCBOOK_XSL@/epub/docbook.xsl"/> 

<!-- Set parameters to uniformize manual across all formats -->

<!-- Make section numbers to include the chapter number. This assumes chapter numbering is turned on -->
<xsl:param name="section.autolabel" select="1" />
<xsl:param name="section.label.includes.component.label" select="1" />
<!-- Generated text for chapters, sections, figures... is only the number and not the tile -->
<xsl:param name="xref.with.number.and.title" select="0" />
<!-- Placement for the legend for figures and tables is after -->
<xsl:param name="formal.title.placement">
figure after
example before
equation before
table after
procedure before
task before
</xsl:param>
<!-- Allow controlling individual cell borders -->
<xsl:param name="table.borders.with.css" select="1" />

</xsl:stylesheet>

