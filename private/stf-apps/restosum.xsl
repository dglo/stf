<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output indent="yes" method="text"/>
<xsl:template match="/">
 <xsl:value-of select="test-results/name"/> 
 <xsl:text> </xsl:text>
 <xsl:apply-templates select="test-results/outputParameter" >
   <xsl:with-param name="prt">boardID</xsl:with-param>
 </xsl:apply-templates>
 <xsl:apply-templates select="test-results/outputParameter" >
   <xsl:with-param name="prt">passed</xsl:with-param>
 </xsl:apply-templates>
</xsl:template>

<xsl:template match="outputParameter">
 <xsl:param name="prt">all</xsl:param>
 <xsl:variable name="nm" select="name"/>
 <xsl:if test="$prt=$nm">
   <xsl:value-of select="value"/>
   <xsl:text> </xsl:text>
  </xsl:if>
</xsl:template>

</xsl:stylesheet>
