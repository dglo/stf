<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet version="1.0" xmlns:stf="http://glacier.lbl.gov/icecube/daq/stf"
  xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes" method="text"/>
  <xsl:template match="/">
    <xsl:apply-templates select="xs:schema"/>
  </xsl:template>
  <xsl:template match="xs:schema">
#include "stf/stf.h"
  <xsl:apply-templates select="xs:element[@name='setup']/xs:complexType/xs:all/xs:element"/>
STF_DESCRIPTOR *stfDirectory[]={
   &amp;StfEg_descriptor,
   &amp;memory_descriptor,
   NULL
};
  </xsl:template>
  <xsl:template match="xs:element/xs:complexType/xs:all/xs:element">
#include "stf-apps/<xsl:value-of select="@name"/>.h"
  </xsl:template>
</xsl:stylesheet>
