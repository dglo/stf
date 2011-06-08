<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">
  <html>
    <head>
      <title>Results: <xsl:value-of select="test-results/name"/></title>
    </head>
    <body>
      <h3><xsl:value-of select="test-results/name"/> (<xsl:value-of select="test-results/description"/>) </h3>
      <table border="1">
	<tbody>
	    <tr>
	      <th>Parameter</th><th>Type</th><th>Value</th>
	    </tr>
	  <xsl:for-each select="test-results/inputParameter">
	    <tr>
	      <td> <xsl:value-of select="name"/> </td>
	      <td>Input</td>
	      <td> <xsl:value-of select="value"/></td>
	    </tr>
	  </xsl:for-each>
	  <xsl:for-each select="test-results/outputParameter">
	    <tr>
	      <td> <xsl:value-of select="name"/> </td>
	      <td>Output</td>
	      <td> <xsl:value-of select="value"/></td>
	    </tr>
	  </xsl:for-each>
	</tbody>
      </table>
    </body>
  </html>
</xsl:template>
</xsl:stylesheet>
