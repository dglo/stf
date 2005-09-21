<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:stf="http://glacier.lbl.gov/icecube/daq/stf"
  xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes" method="text"/>
  <xsl:template match="/">
    <xsl:apply-templates select="stf:test"/>
  </xsl:template>
  <xsl:template match="stf:test">
  <xsl:variable name="testName" select="name"/>
static STF_PARAM <xsl:copy-of select="$testName"/>_params[] = {
    <xsl:apply-templates mode="PARAM" select="inputParameter"/>
    <xsl:apply-templates mode="PARAM" select="outputParameter"/>
   { .name = "" }
};

extern void <xsl:copy-of select="$testName"/>Init(STF_DESCRIPTOR *);
extern void <xsl:copy-of select="$testName"/>Entry(STF_DESCRIPTOR *);

static STF_DESCRIPTOR <xsl:copy-of select="$testName"/>_descriptor = {
  .name = "<xsl:copy-of select="$testName"/>",
  .desc = "<xsl:value-of select="description"/>",
  .majorVersion = <xsl:value-of select="version/@major"/>,
  .minorVersion = <xsl:value-of select="version/@minor"/>,
  .testRunnable = 0,
  .nParams = <xsl:value-of select="count(inputParameter) + count(outputParameter)"/>,
  .params = <xsl:copy-of select="$testName"/>_params,
  .initPt = <xsl:copy-of select="$testName"/>Init,
  .entryPt = <xsl:copy-of select="$testName"/>Entry,
  .isInit = 0
};
  </xsl:template>
  <xsl:template match="stf:test/*" mode="PARAM">
    <xsl:apply-templates mode="PARAM" select="boolean|string|unsignedInt|unsignedLong"/>
  </xsl:template>
  <xsl:template match="stf:test/*/boolean" mode="paraType">boolean</xsl:template>
  <xsl:template match="stf:test/*/string" mode="paraType">string</xsl:template>
  <xsl:template match="stf:test/*/unsignedInt" mode="paraType">unsignedInt</xsl:template>
  <xsl:template match="stf:test/*/unsignedLong" mode="paraType">unsignedLong</xsl:template>
  <xsl:template match="stf:test/inputParameter/boolean|stf:test/inputParameter/string" mode="max">NULL</xsl:template>
  <xsl:template match="stf:test/inputParameter/boolean|stf:test/inputParameter/string" mode="min">NULL</xsl:template>
  <xsl:template match="stf:test/inputParameter/boolean|stf:test/inputParameter/string" mode="default">"<xsl:value-of select="@default"/>"</xsl:template>
  <xsl:template match="stf:test/inputParameter/unsignedInt|stf:test/inputParameter/unsignedLong" mode="max">"<xsl:value-of select="@maxValue"/>"</xsl:template>
  <xsl:template match="stf:test/inputParameter/unsignedInt|stf:test/inputParameter/unsignedLong" mode="min">"<xsl:value-of select="@minValue"/>"</xsl:template>
  <xsl:template match="stf:test/inputParameter/unsignedInt|stf:test/inputParameter/unsignedLong" mode="default">"<xsl:value-of select="@default"/>"</xsl:template>
  <xsl:template match="stf:test/inputParameter/*" mode="PARAM">
   {
     .name = "<xsl:copy-of select="../name"/>",
     .class = "input",
     .type = "<xsl:apply-templates mode="paraType" select="."/>",
     .maxValue = <xsl:apply-templates mode="max" select="."/>,
     .minValue = <xsl:apply-templates mode="min" select="."/>,
     .defValue = <xsl:apply-templates mode="default" select="."/>,
     .arraySize = "1",
     .value = {
       .longValue = 0
     },
     .arrayLength = 1
   },
  </xsl:template>
  <xsl:template match="stf:test/outputParameter/*" mode="PARAM">
   {
     .name = "<xsl:copy-of select="../name"/>",
     .class = "output",
     .type = "<xsl:apply-templates mode="paraType" select="."/>",
     .maxValue = NULL,
     .minValue = NULL,
     .defValue = NULL,
     .arraySize = "1",
     .value = {
       .longValue = 0
     },
     .arrayLength = 1
   },
  </xsl:template>
</xsl:stylesheet>
