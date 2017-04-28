<?xml version="1.0"?>
<!-- we only care about rest temperatures
     thus this stylesheet discards the first value in beerXML -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text"/>

  <xsl:template match="/">
    <xsl:for-each select="/RECIPES/RECIPE/MASH/MASH_STEPS/MASH_STEP">
      <xsl:if test="position()>'1'">
        <xsl:value-of select="STEP_TEMP" />
        <xsl:text> </xsl:text>
        <xsl:value-of select="STEP_TIME" />
        <xsl:if test="not(position()=last())">
          <xsl:text> </xsl:text>
        </xsl:if>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

</xsl:stylesheet>
