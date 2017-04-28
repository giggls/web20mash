<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text"/>

  <xsl:template match="/">
    <xsl:if test="/xsud/Sud/Rasten/Rast_1">
    <xsl:value-of select="/xsud/Sud/Rasten/Rast_1/RastTemp" />
      <xsl:text> </xsl:text>
      <xsl:value-of select="/xsud/Sud/Rasten/Rast_1/RastDauer" />
    </xsl:if>
    
    <xsl:if test="/xsud/Sud/Rasten/Rast_2">
    <xsl:text> </xsl:text>
    <xsl:value-of select="/xsud/Sud/Rasten/Rast_2/RastTemp" />
      <xsl:text> </xsl:text>
      <xsl:value-of select="/xsud/Sud/Rasten/Rast_2/RastDauer" />
    </xsl:if>

    <xsl:if test="/xsud/Sud/Rasten/Rast_3">
    <xsl:text> </xsl:text>
    <xsl:value-of select="/xsud/Sud/Rasten/Rast_3/RastTemp" />
      <xsl:text> </xsl:text>
      <xsl:value-of select="/xsud/Sud/Rasten/Rast_3/RastDauer" />
    </xsl:if>
    
    <xsl:if test="/xsud/Sud/Rasten/Rast_4">
    <xsl:text> </xsl:text>
    <xsl:value-of select="/xsud/Sud/Rasten/Rast_4/RastTemp" />
      <xsl:text> </xsl:text>
      <xsl:value-of select="/xsud/Sud/Rasten/Rast_4/RastDauer" />
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>
