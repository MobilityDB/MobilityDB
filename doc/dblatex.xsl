<?xml version='1.0' encoding="UTF-8"?>
<!--
   ****************************************************************************
    MobilityDB Manual
    Copyright(c) MobilityDB Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>

<!-- A signature block states the syntax of an entry, and the examples that follow it state SQL.
     Both are verbatim, and only the examples are set in the shaded frame that marks a code
     block, so that the syntax reads as part of the entry it opens and stays separated from the
     examples. -->
<xsl:template match="programlisting[@role='syntax']">
  <xsl:text>&#10;\begingroup\lstset{backgroundcolor={},frame=none,xleftmargin=0pt,%&#10;</xsl:text>
  <xsl:text>  aboveskip=\smallskipamount,belowskip=\smallskipamount}%&#10;</xsl:text>
  <xsl:apply-imports/>
  <xsl:text>\endgroup&#10;</xsl:text>
</xsl:template>

<!-- Monospaced inline text states an apostrophe upright, as a listing already does, and not as
     the right single quotation mark that the typewriter text font gives it -->
<xsl:template name="inline.monoseq">
  <xsl:param name="content">
    <xsl:apply-templates/>
  </xsl:param>
  <xsl:text>\texttt{</xsl:text>
  <xsl:if test="not($monoseq.small = '0')">
    <xsl:text>\small{</xsl:text>
  </xsl:if>
  <xsl:call-template name="inline.hyphenate">
    <xsl:with-param name="format" select="'monoseq'"/>
    <xsl:with-param name="string">
      <xsl:call-template name="scape-replace">
        <xsl:with-param name="from">'</xsl:with-param>
        <xsl:with-param name="to">\textquotesingle{}</xsl:with-param>
        <xsl:with-param name="string" select="$content"/>
      </xsl:call-template>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:if test="not($monoseq.small = '0')">
    <xsl:text>}</xsl:text>
  </xsl:if>
  <xsl:text>}</xsl:text>
</xsl:template>

</xsl:stylesheet>
