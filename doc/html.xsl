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

<xsl:import href="@DOCBOOK_XSL@/html/chunk.xsl"/>

<!-- State the role of a verbatim block as a second CSS class, so that a signature block is
     styled apart from the examples that follow it -->
<xsl:template match="programlisting[@role]|screen[@role]|synopsis[@role]" mode="class.value">
	<xsl:value-of select="concat(local-name(.), ' ', @role)"/>
</xsl:template>

</xsl:stylesheet>
