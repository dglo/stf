<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet version="1.0"
    xmlns:stf="http://glacier.lbl.gov/icecube/daq/stf"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes" method="text"/>
  <xsl:variable name="nl">
<xsl:text>
</xsl:text>
  </xsl:variable>
  <xsl:template match="/">
    <xsl:apply-templates select="xs:schema"/>
<xsl:copy-of select="$nl"/>
  </xsl:template>
  <xsl:template match="xs:schema">
#include "stf/stf.h"
  <xsl:apply-templates mode="include" select="xs:element[@name='setup']/xs:complexType/xs:all/xs:element"/>
STF_DESCRIPTOR *stfDirectory[]={
  <xsl:apply-templates mode="descriptor" select="xs:element[@name='setup']/xs:complexType/xs:all/xs:element"/>
   NULL
};
  </xsl:template>
  <xsl:template match="xs:element/xs:complexType/xs:all/xs:element" mode="include">
#include "stf-apps/<xsl:value-of select="@name"/>.h"
  </xsl:template>
  <xsl:template match="xs:element/xs:complexType/xs:all/xs:element" mode="descriptor">
   &amp;<xsl:value-of select="@name"/>_descriptor,
  </xsl:template>
</xsl:stylesheet>
