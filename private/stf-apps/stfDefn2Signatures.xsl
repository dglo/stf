<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:stf="http://glacier.lbl.gov/icecube/daq/stf"
  xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes" method="text"/>
  <xsl:variable name="nl">
<xsl:text>
</xsl:text>
  </xsl:variable>
  <xsl:template match="/">
    <xsl:apply-templates select="stf:test"/>
  </xsl:template>
  <xsl:template match="stf:test">
  <xsl:variable name="testName" select="name"/>
extern BOOLEAN <xsl:copy-of select="$testName"/>Init(STF_DESCRIPTOR *);
extern BOOLEAN <xsl:copy-of select="$testName"/>Entry(STF_DESCRIPTOR *,
<xsl:apply-templates mode="Entry" select="inputParameter"/>
<xsl:apply-templates mode="Entry" select="outputParameter"/>
  </xsl:template>
  <xsl:template match="stf:test/*/*" mode="Entry">
    <xsl:text>                   </xsl:text>
    <xsl:apply-templates mode="signature" select="."/>
    <xsl:apply-templates mode="entryModifier" select="."/>
    <xsl:text> </xsl:text>
    <xsl:copy-of select="../name"/>
  </xsl:template>
  <xsl:template match="stf:test/*" mode="Entry">
    <xsl:apply-templates mode="Entry" select="boolean|string|unsignedInt|unsignedLong"/>
    <xsl:choose>
      <xsl:when test='((0=count(../outputParameter))or("outputParameter"=local-name()))and(last()=position())'>);</xsl:when>
      <xsl:otherwise>,</xsl:otherwise>
    </xsl:choose>
    <xsl:copy-of select="$nl"/>
  </xsl:template>
  <xsl:template match="stf:test/*/boolean" mode="signature">BOOLEAN</xsl:template>
  <xsl:template match="stf:test/inputParameter/string" mode="signature">const char*</xsl:template>
  <xsl:template match="stf:test/outputParameter/string" mode="signature">char*</xsl:template>
  <xsl:template match="stf:test/*/unsignedInt" mode="signature">unsigned int</xsl:template>
  <xsl:template match="stf:test/*/unsignedLong" mode="signature">unsigned long</xsl:template>
  <xsl:template match="stf:test/outputParameter/*" mode="entryModifier">* </xsl:template>
  <xsl:template match="stf:test/*/*" mode="EntryLocal">
    <xsl:text>                   </xsl:text>
    <xsl:apply-templates mode="entryLocalModifier" select="."/>
    <xsl:text>getParamByName(d, "</xsl:text>
    <xsl:copy-of select="../name"/><xsl:text>")->value.</xsl:text>
    <xsl:apply-templates mode="value" select="."/>
    <xsl:text>Value</xsl:text>
  </xsl:template>
  <xsl:template match="stf:test/*" mode="EntryLocal">
    <xsl:apply-templates mode="EntryLocal" select="boolean|string|unsignedInt|unsignedLong"/>
    <xsl:choose>
      <xsl:when test='((0=count(../outputParameter))or("outputParameter"=local-name()))and(last()=position())'>);</xsl:when>
      <xsl:otherwise>,</xsl:otherwise>
    </xsl:choose>
    <xsl:copy-of select="$nl"/>
  </xsl:template>
  <xsl:template match="stf:test/outputParameter/*" mode="entryLocalModifier">&amp;</xsl:template>
</xsl:stylesheet>
